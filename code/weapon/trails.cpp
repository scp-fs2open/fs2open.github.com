/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "weapon/trails.h"
#include "globalincs/systemvars.h"
#include "render/3d.h" 
#include "io/timer.h"
#include "ship/ship.h"


int Num_trails;
trail Trails;

// Reset everything between levels
void trail_level_init()
{
	Num_trails = 0;
	Trails.next = &Trails;
}

void trail_level_close()
{
	trail *nextp;
	for(trail *trailp = Trails.next; trailp != &Trails; trailp = nextp)
	{
		nextp = trailp->next;

		//Now we can delete it
		delete trailp;
	}

	Num_trails=0;
}

//returns the number of a free trail
//returns -1 if no free trails
trail *trail_create(trail_info *info)
{
	// standalone server should never create trails
	// No trails at slot 0
	if((Game_mode & GM_STANDALONE_SERVER) || !Detail.weapon_extras)
		return NULL;

	// Make a new trail
	trail *trailp = new trail;

	// increment counter
	Num_trails++;

	// Init the trail data
	trailp->info = *info;
	trailp->tail = 0;
	trailp->head = 0;	
	trailp->object_died = false;		
	trailp->trail_stamp = timestamp(trailp->info.stamp);

	//Add it to the front of the list
	//This is quickest since there are no prev vars
	trailp->next = Trails.next;
	Trails.next = trailp;

	return trailp;
}

// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void trail_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w )
{
	vec3d uvec, rvec;

	vm_vec_sub( &rvec, &Eye_position, pos );
	if (!IS_VEC_NULL(&rvec))
		vm_vec_normalize( &rvec );

	vm_vec_crossprod(&uvec,fvec,&rvec);
	if (!IS_VEC_NULL(&uvec))
		vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, pos, &uvec, w * 0.5f );
	vm_vec_scale_add( bot, pos, &uvec, -w * 0.5f );
}

// trail is on ship
int trail_is_on_ship(trail *trailp, ship *shipp)
{
	if(trailp == NULL)
		return 0;

	for(int idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->trail_ptr[idx] == trailp){
			return 1;
		}
	}

	// nope
	return 0;
}

// Render the trail behind a missile.
// Basically a queue of points that face the viewer
extern int Cmdline_nohtl;

static vertex **Trail_vlist = NULL;
static vertex *Trail_v_list = NULL;
static int Trail_verts_allocated = 0;

static void deallocate_trail_verts()
{
	if (Trail_vlist != NULL) {
		vm_free(Trail_vlist);
		Trail_vlist = NULL;
	}

	if (Trail_v_list != NULL) {
		vm_free(Trail_v_list);
		Trail_v_list = NULL;
	}
}

static void allocate_trail_verts(int num_verts)
{
	if (num_verts <= 0)
		return;

	if (num_verts <= Trail_verts_allocated)
		return;

	if (Trail_vlist != NULL) {
		vm_free(Trail_vlist);
		Trail_vlist = NULL;
	}

	if (Trail_v_list != NULL) {
		vm_free(Trail_v_list);
		Trail_v_list = NULL;
	}

	Trail_vlist = (vertex**) vm_malloc( num_verts * sizeof(vertex*) );
	Trail_v_list = (vertex*) vm_malloc( num_verts * sizeof(vertex) );

	memset( Trail_v_list, 0, sizeof(vertex) * Trail_verts_allocated );

	Trail_verts_allocated = num_verts;


	static bool will_free_at_exit = false;

	if ( !will_free_at_exit ) {
		atexit(deallocate_trail_verts);
		will_free_at_exit = true;
	}
}

void trail_render( trail * trailp )
{
	int sections[NUM_TRAIL_SECTIONS];
	int num_sections = 0;
	int i;
	vec3d topv, botv, *fvec, last_pos, tmp_fvec;
	vertex  top, bot;
	int nv = 0;
	float w;
	ubyte l;
	vec3d centerv;

	if (trailp->tail == trailp->head)
		return;

	// if this trail is on the player ship, and he's in any padlock view except rear view, don't draw	
	if ( (Player_ship != NULL) && trail_is_on_ship(trailp, Player_ship) &&
		(Viewer_mode & (VM_PADLOCK_UP | VM_PADLOCK_LEFT | VM_PADLOCK_RIGHT)) )
	{
		return;
	}

	trail_info *ti	= &trailp->info;

	int n = trailp->tail;

	do	{
		n--;

		if (n < 0)
			n = NUM_TRAIL_SECTIONS-1;

		if (trailp->val[n] > 1.0f)
			break;

		sections[num_sections++] = n;
	} while ( n != trailp->head );

	if (num_sections <= 0)
		return;

	Assertion(ti->texture.bitmap_id != -1, "Weapon trail %s could not be loaded", ti->texture.filename); // We can leave this as an assert, but tell them how to fix it. --Chief

	memset( &top, 0, sizeof(vertex) );
	memset( &bot, 0, sizeof(vertex) );

	// it's a tristrip, so allocate for 2+1
	allocate_trail_verts((num_sections * 2) + 1);

	float w_size = (ti->w_end - ti->w_start);
	float a_size = (ti->a_end - ti->a_start);
	int num_faded_sections = ti->n_fade_out_sections;

	for (i = 0; i < num_sections; i++) {
		n = sections[i];
		float init_fade_out = 1.0f;

		if ((num_faded_sections > 0) && (i < num_faded_sections)) {
			init_fade_out = ((float) i) / (float) num_faded_sections;
		}

		w = trailp->val[n] * w_size + ti->w_start;
		if (init_fade_out != 1.0f) {
			l = (ubyte)fl2i((trailp->val[n] * a_size + ti->a_start) * 255.0f * init_fade_out * init_fade_out);
		} else {
			l = (ubyte)fl2i((trailp->val[n] * a_size + ti->a_start) * 255.0f);
		}

		if ( i == 0 )	{
			if ( num_sections > 1 )	{
				vm_vec_sub(&tmp_fvec, &trailp->pos[n], &trailp->pos[sections[i+1]] );
				vm_vec_normalize_safe(&tmp_fvec);
				fvec = &tmp_fvec;
			} else {
				fvec = &tmp_fvec;
				fvec->xyz.x = 0.0f;
				fvec->xyz.y = 0.0f;
				fvec->xyz.z = 1.0f;
			}
		} else {
			vm_vec_sub(&tmp_fvec, &last_pos, &trailp->pos[n] );
			vm_vec_normalize_safe(&tmp_fvec);
			fvec = &tmp_fvec;
		}

		trail_calc_facing_pts( &topv, &botv, fvec, &trailp->pos[n], w );

		if ( !Cmdline_nohtl ) {
			g3_transfer_vertex( &top, &topv );
			g3_transfer_vertex( &bot, &botv );
		} else {
			g3_rotate_vertex( &top, &topv );
			g3_rotate_vertex( &bot, &botv );
		}

		top.a = bot.a = l;	

		if (i > 0) {
			float U = i2fl(i);

			if (i == num_sections-1) {
				// Last one...
				vm_vec_avg( &centerv, &topv, &botv );

				if ( !Cmdline_nohtl )
					g3_transfer_vertex( &Trail_v_list[nv+2], &centerv );
				else
					g3_rotate_vertex( &Trail_v_list[nv+2], &centerv );

				Trail_v_list[nv].a = l;	

				Trail_vlist[nv] = &Trail_v_list[nv];
				Trail_vlist[nv]->u = U;
				Trail_vlist[nv]->v = 1.0f; 
				Trail_vlist[nv]->r = Trail_vlist[nv]->g = Trail_vlist[nv]->b = l;
				nv++;

				Trail_vlist[nv] = &Trail_v_list[nv];
				Trail_vlist[nv]->u = U;
				Trail_vlist[nv]->v = 0.0f; 
				Trail_vlist[nv]->r = Trail_vlist[nv]->g = Trail_vlist[nv]->b = l;
				nv++;

				Trail_vlist[nv] = &Trail_v_list[nv];
				Trail_vlist[nv]->u = U + 1.0f;
				Trail_vlist[nv]->v = 0.5f;
				Trail_vlist[nv]->r = Trail_vlist[nv]->g = Trail_vlist[nv]->b = 0;
				nv++;
			} else {
				Trail_vlist[nv] = &Trail_v_list[nv];
				Trail_vlist[nv]->u = U;
				Trail_vlist[nv]->v = 1.0f; 
				Trail_vlist[nv]->r = Trail_vlist[nv]->g = Trail_vlist[nv]->b = l;
				nv++;

				Trail_vlist[nv] = &Trail_v_list[nv];
				Trail_vlist[nv]->u = U;
				Trail_vlist[nv]->v = 0.0f; 
				Trail_vlist[nv]->r = Trail_vlist[nv]->g = Trail_vlist[nv]->b = l;
				nv++;
			}
		}

		last_pos = trailp->pos[n];
		Trail_v_list[nv] = top;
		Trail_v_list[nv+1] = bot;
	}


	if ( !nv )
		return;

	if (nv < 3)
		Error( LOCATION, "too few verts in trail render\n" );

	// there should always be three verts in the last section and 2 everyware else, therefore there should always be an odd number of verts
	if ( (nv % 2) != 1 )
		Warning( LOCATION, "even number of verts in trail render\n" );


	gr_set_bitmap( ti->texture.bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
	g3_draw_poly( nv, Trail_vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_ALPHA | TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT | TMAP_FLAG_TRISTRIP );
}



void trail_add_segment( trail *trailp, vec3d *pos )
{
	int next = trailp->tail;
	trailp->tail++;
	if ( trailp->tail >= NUM_TRAIL_SECTIONS )
		trailp->tail = 0;

	if ( trailp->head == trailp->tail )	{
		// wrapped!!
		trailp->head++;
		if ( trailp->head >= NUM_TRAIL_SECTIONS )
			trailp->head = 0;
	}
	
	trailp->pos[next] = *pos;
	trailp->val[next] = 0.0f;
}		

void trail_set_segment( trail *trailp, vec3d *pos )
{
	int next = trailp->tail-1;
	if ( next < 0 )	{
		next = NUM_TRAIL_SECTIONS-1;
	}
	
	trailp->pos[next] = *pos;
}

void trail_move_all(float frametime)
{
	int num_alive_segments,n;
	float time_delta;
	trail *next_trail;
	trail *prev_trail = &Trails;

	for (trail *trailp = Trails.next; trailp != &Trails; trailp = next_trail) {
		next_trail = trailp->next;

		num_alive_segments = 0;

		if ( trailp->tail != trailp->head )	{
			n = trailp->tail;			
			time_delta = frametime / trailp->info.max_life;
			do	{
				n--;
				if ( n < 0 ) n = NUM_TRAIL_SECTIONS-1;

				trailp->val[n] += time_delta;

				if ( trailp->val[n] <= 1.0f ) {
					num_alive_segments++;	// Record how many still alive.
				}

			} while ( n != trailp->head );
		}		
	
		if ( (num_alive_segments < 1) && trailp->object_died)
		{
			prev_trail->next = trailp->next;
			delete trailp;

			// decrement counter
			Num_trails--;
		}
		else
		{
			prev_trail = trailp;
		}
	}
}

void trail_object_died( trail *trailp )
{
	trailp->object_died = true;
}

void trail_render_all()
{
	// No trails at slot 0
	if ( !Detail.weapon_extras )
		return;

	for(trail *trailp = Trails.next; trailp!=&Trails; trailp = trailp->next )
	{
		trail_render(trailp);
	}

}
int trail_stamp_elapsed(trail *trailp)
{
	return timestamp_elapsed(trailp->trail_stamp);
}

void trail_set_stamp(trail *trailp)
{
	trailp->trail_stamp = timestamp(trailp->info.stamp);
}

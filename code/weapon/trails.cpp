/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Trails.cpp $
 * $Revision: 2.27.2.4 $
 * $Date: 2007-05-11 03:12:28 $
 * $Author: taylor $
 *
 * Code for missile trails
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.27.2.3  2007/04/14 23:42:02  taylor
 * little cleanup
 * fix memory size fubar on trail vlist ptr
 *
 * Revision 2.27.2.2  2007/02/12 00:48:43  taylor
 * oops, forgot this bit
 *
 * Revision 2.27.2.1  2007/02/12 00:45:24  taylor
 * bit of cleanup and minor performance tweaks
 * sync up with new generic_anim/bitmap and weapon delayed loading changes
 * with generic_anim, use Goober's animation timing for beam section and glow animations
 * make trail render list dynamic (as well as it can be)
 *
 * Revision 2.27  2006/05/27 16:45:11  taylor
 * some minor cleanup
 * remove -nobeampierce
 * update for geometry batcher changes
 *
 * Revision 2.26  2006/03/18 10:28:58  taylor
 * meh.
 *
 * Revision 2.25  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.24  2005/02/20 23:11:51  wmcoolmon
 * Fix0r3d trails
 *
 * Revision 2.23  2005/02/20 08:24:19  wmcoolmon
 * More trails updating goodness
 *
 * Revision 2.22  2005/02/20 07:39:14  wmcoolmon
 * Trails update: Better, faster, stronger, but not much more reliable
 *
 * Revision 2.21  2005/02/19 07:54:33  wmcoolmon
 * Removed trails limit
 *
 * Revision 2.20  2004/10/31 22:02:47  taylor
 * little cleanup
 *
 * Revision 2.19  2004/07/26 20:47:56  Kazan
 * remove MCD complete
 *
 * Revision 2.18  2004/07/12 16:33:09  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.17  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.16  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.15  2004/02/20 04:29:57  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.14  2004/02/16 11:47:34  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.13  2004/02/14 00:18:37  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.12  2004/02/04 09:02:42  Goober5000
 * got rid of unnecessary double semicolons
 * --Goober5000
 *
 * Revision 2.11  2003/11/19 20:37:25  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 * Revision 2.10  2003/11/17 04:25:58  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.9  2003/11/16 04:09:20  Goober5000
 * language
 *
 * Revision 2.8  2003/11/11 03:56:13  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.7  2003/11/09 06:31:40  Kazan
 * a couple of htl functions being called in nonhtl (ie NULL functions) problems fixed
 * conflicts in cmdline and timerbar.h log entries
 * cvs stopped acting like it was on crack obviously
 *
 * Revision 2.6  2003/11/02 05:50:08  bobboau
 * modified trails to render with tristrips now rather than with stinky old trifans,
 * MUCH faster now, at least one order of magnatude.
 *
 * Revision 2.5  2003/10/23 18:03:25  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.4  2003/08/21 15:04:17  phreak
 * zeroed out the specular fields since they caused some flickering
 *
 * Revision 2.3  2003/07/15 16:07:12  phreak
 * trails now properly alphablend in ogl as it does in d3d
 *
 * Revision 2.2  2003/05/04 20:50:06  phreak
 * bumped MAX_TRAILS to 500 because of afterburner trails
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     6/23/99 2:23p Mattk
 * Fixed detail level trail rendering problem.
 * 
 * 6     6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 5     2/23/99 8:11p Dave
 * Tidied up dogfight mode. Fixed TvT ship type problems for alpha wing.
 * Small pass over todolist items.
 * 
 * 4     2/17/99 2:11p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 3     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 9     5/13/98 3:10p John
 * made detail slider for weapon rendering change the distance that lasers
 * become non-textured.  The lowest setting turns off missile trail
 * rendering.
 * 
 * 8     5/08/98 7:09p Dave
 * Lots of UI tweaking.
 * 
 * 7     4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 6     3/31/98 5:19p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 5     3/23/98 5:00p John
 * Improved missile trails.  Made smooth alpha under hardware.  Made end
 * taper.  Made trail touch weapon.
 * 
 * 4     1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 3     1/15/98 11:13a John
 * Added code for specifying weapon trail bitmaps in weapons.tbl
 * 
 * 2     12/21/97 6:15p John
 * Made a seperate system for missile trails
 * 
 * 1     12/21/97 5:30p John
 * Initial version
 *
 * $NoKeywords: $
 */

#include "weapon/trails.h"
#include "globalincs/systemvars.h"
#include "render/3d.h" 
#include "io/timer.h"
#include "ship/ship.h"

#include <vector>

typedef struct section_data {
	vec3d pos;		// positions of trail points
	float fade;		// for each point, a value that tells how much to fade out
} section_data;

typedef struct trail {
	int head, tail;					// pointers into the queue for the trail points
	bool object_died;				// set to zero as long as object	
	int trail_stamp;				// trail timestamp	
	int	handle;						// unique id for this trail

	section_data section[NUM_TRAIL_SECTIONS];

	trail_info info;				// trail info - this is passed when creating a trail

	// these two are used for rendering only...
	int num_active_sections;
	section_data active_sections[NUM_TRAIL_SECTIONS];
} trail;

std::vector<trail> Trails;

static const int Min_trail_bump = 100;
static int Next_trail_handle = 0;

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
	if (num_verts <= 0) {
		return;
	}

	if (num_verts <= Trail_verts_allocated) {
		return;
	}

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

// Reset everything between levels
void trail_level_init()
{
	Trails.clear();
	Trails.reserve(Min_trail_bump);

	Next_trail_handle = 0;

	// go ahead and allocate enough for 100 verts
	allocate_trail_verts(201);
}

void trail_level_close()
{
	Trails.clear();
}

static trail *get_trail_from_id(int trail_id)
{
	for (std::vector<trail>::iterator trailp = Trails.begin(); trailp != Trails.end(); ++trailp) {
		if (trailp->handle == trail_id) {
			return &Trails[trailp - Trails.begin()];
		}
	}

	return NULL;
}

//returns the number of a free trail
//returns -1 if no free trails
int trail_create(trail_info *info)
{
	trail new_trail;

	// standalone server should never create trails
	// No trails at slot 0
	if ( (Game_mode & GM_STANDALONE_SERVER) || !Detail.weapon_extras ) {
		return -1;
	}

	// Init the trail data
	memcpy(&new_trail.info, info, sizeof(trail_info));
	new_trail.tail = 0;
	new_trail.head = 0;	
	new_trail.object_died = false;		
	new_trail.trail_stamp = timestamp(new_trail.info.stamp);

	new_trail.handle = Next_trail_handle++;

	if ( Trails.size() == Trails.capacity() ) {
		Trails.reserve( Trails.size() + Min_trail_bump );
	}

	Trails.push_back( new_trail );

	return new_trail.handle;
}

// trail is on ship
int trail_is_on_ship(int trail_id, ship *shipp)
{
	if ( (trail_id < 0) || (trail_id > Next_trail_handle) ) {
		Int3();
		return 0;
	}

	for (int idx = 0; idx < MAX_SHIP_CONTRAILS; idx++) {
		if (shipp->trail_id[idx] == trail_id) {
			return 1;
		}
	}

	// nope
	return 0;
}

// Render the trail behind a missile.
// Basically a queue of points that face the viewer
extern int Cmdline_nohtl;

void trail_render(trail *trailp)
{
	int i;
	vec3d topv, botv, fvec, uvec, rvec, last_pos;
	vertex  top, bot;
	int nv = 0;
	float w;
	ubyte l;
	vec3d centerv;

	if (trailp->tail == trailp->head) {
		return;
	}

	// if this trail is on the player ship, and he's in any padlock view except rear view, don't draw	
	if ( (Player_ship != NULL) && trail_is_on_ship(trailp->handle, Player_ship) &&
		(Viewer_mode & (VM_PADLOCK_UP | VM_PADLOCK_LEFT | VM_PADLOCK_RIGHT)) )
	{
		return;
	}

	trail_info *ti = &trailp->info;

	Assert(ti->texture.bitmap_id != -1);

	memset( &top, 0, sizeof(vertex) );
	memset( &bot, 0, sizeof(vertex) );

	int num_sections = trailp->num_active_sections;

	// it's a tristrip, so allocate for 2+1
	allocate_trail_verts((num_sections * 2) + 1);

	float w_size = (ti->w_end - ti->w_start);
	float a_size = (ti->a_end - ti->a_start);

	for (i = 0; i < num_sections; i++) {
		section_data *sdp = &trailp->active_sections[i];

		w = sdp->fade * w_size + ti->w_start;
		l = (ubyte)fl2i((sdp->fade * a_size + ti->a_start) * 255.0f);

		if (i == 0) {
			if (num_sections > 1) {
				vm_vec_sub(&fvec, &sdp->pos, &trailp->active_sections[i+1].pos);
				vm_vec_normalize_safe(&fvec);
			} else {
				fvec = vmd_z_vector;
			}
		} else {
			vm_vec_sub(&fvec, &last_pos, &sdp->pos);
			vm_vec_normalize_safe(&fvec);
		}

		vm_vec_sub(&rvec, &Eye_position, &sdp->pos);
		vm_vec_normalize(&rvec);
	
		vm_vec_crossprod(&uvec, &fvec, &rvec);
		vm_vec_normalize(&uvec);
	
		vm_vec_scale_add(&topv, &sdp->pos, &uvec, w * 0.5f);
		vm_vec_scale_add(&botv, &sdp->pos, &uvec, -w * 0.5f);

		if ( !Cmdline_nohtl ) {
			g3_transfer_vertex(&top, &topv);
			g3_transfer_vertex(&bot, &botv);
		} else {
			g3_rotate_vertex(&top, &topv);
			g3_rotate_vertex(&bot, &botv);
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

		last_pos = sdp->pos;
		Trail_v_list[nv] = top;
		Trail_v_list[nv+1] = bot;

		sdp++;
	}


	if ( !nv ) {
		return;
	}

	if (nv < 3) {
		Error( LOCATION, "too few verts in trail render\n" );
	}

	// there should always be three verts in the last section and 2 everyware else, therefore there should always be an odd number of verts
	if ( (nv % 2) != 1 ) {
		Warning( LOCATION, "even number of verts in trail render\n" );
	}


	gr_set_bitmap( ti->texture.bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
	g3_draw_poly( nv, Trail_vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_ALPHA | TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_HTL_3D_UNLIT | TMAP_FLAG_TRISTRIP );
}



void trail_add_segment(int trail_id, vec3d *pos)
{
	trail *trailp = get_trail_from_id(trail_id);

	if (trailp == NULL) {
		return;
	}

	int next = trailp->tail;

	trailp->tail++;

	if (trailp->tail >= NUM_TRAIL_SECTIONS) {
		trailp->tail = 0;
	}

	if (trailp->head == trailp->tail) {
		// wrapped!!
		trailp->head++;

		if (trailp->head >= NUM_TRAIL_SECTIONS) {
			trailp->head = 0;
		}
	}

	trailp->section[next].pos = *pos;
	trailp->section[next].fade = 0.0f;
}		

void trail_set_segment(int trail_id, vec3d *pos)
{
	trail *trailp = get_trail_from_id(trail_id);

	if (trailp == NULL) {
		return;
	}

	int next = trailp->tail-1;

	if (next < 0) {
		next = NUM_TRAIL_SECTIONS-1;
	}
	
	trailp->section[next].pos = *pos;
}

void trail_move_all(float frametime)
{
	int alive_segments[NUM_TRAIL_SECTIONS];
	int num_alive_segments, n;
	float time_delta;
	section_data *sdp;

	if ( Trails.empty() ) {
		return;
	}

	for (std::vector<trail>::iterator trailp = Trails.begin(); trailp != Trails.end(); ) {
		num_alive_segments = 0;

		if (trailp->tail != trailp->head) {
			n = trailp->tail;			
			time_delta = frametime * trailp->info.i_max_life;

			do	{
				n--;

				if (n < 0) {
					n = NUM_TRAIL_SECTIONS-1;
				}

				trailp->section[n].fade += time_delta;

				if (trailp->section[n].fade <= 1.0f) {
					alive_segments[num_alive_segments++] = n;	// Record how many still alive.
				}
			} while (n != trailp->head);
		}		
	
		if ( (num_alive_segments < 1) && trailp->object_died ) {
			*trailp = Trails.back();
			Trails.pop_back();
			continue;
		}

		if (num_alive_segments < 0) {
			continue;
		}

		sdp = &trailp->active_sections[0];
		trailp->num_active_sections = num_alive_segments;

		for (n = 0; n < num_alive_segments; n++) {
			sdp->pos = trailp->section[alive_segments[n]].pos;
			sdp->fade = trailp->section[alive_segments[n]].fade;
			sdp++;
		}

		++trailp;
	}
}

void trail_object_died(int trail_id)
{
	trail *trailp = get_trail_from_id(trail_id);

	if (trailp == NULL) {
		return;
	}

	trailp->object_died = true;
}

void trail_render_all()
{
	// No trails at slot 0
	if ( !Detail.weapon_extras ) {
		return;
	}

	if ( Trails.empty() ) {
		return;
	}

	uint end = Trails.size();

	for (uint i = 0; i < end; i++) {
		trail_render(&Trails[i]);
	}
}

int trail_stamp_elapsed(int trail_id)
{
	trail *trailp = get_trail_from_id(trail_id);

	if (trailp == NULL) {
		return 0;
	}

	return timestamp_elapsed(trailp->trail_stamp);
}

void trail_set_stamp(int trail_id)
{
	trail *trailp = get_trail_from_id(trail_id);

	if (trailp == NULL) {
		return;
	}

	trailp->trail_stamp = timestamp(trailp->info.stamp);
}

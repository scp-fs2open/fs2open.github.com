/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "render/3d.h" 
#include "bmpman/bmpman.h"
#include "particle/particle.h"
#include "object/object.h"
#include "cmdline/cmdline.h"
#include "graphics/grbatch.h"

#ifndef NDEBUG
#include "io/timer.h"
#endif

int Num_particles = 0;
static SCP_vector<particle*> Particles;

int Anim_bitmap_id_fire = -1;
int Anim_num_frames_fire = -1;

int Anim_bitmap_id_smoke = -1;
int Anim_num_frames_smoke = -1;

int Anim_bitmap_id_smoke2 = -1;
int Anim_num_frames_smoke2 = -1;

static int Particles_enabled = 1;

uint lastSignature = 0; // 0 is an invalid signature!


// Reset everything between levels
void particle_init()
{
	int fps;

	Particles_enabled = (Detail.num_particles > 0);

	Num_particles = 0;

	Particles.clear();

	// FIRE!!!
	if ( Anim_bitmap_id_fire == -1 )	{
		Anim_bitmap_id_fire = bm_load_animation( "particleexp01", &Anim_num_frames_fire, &fps, NULL, 0 );
	}

	// Cough, cough
	if ( Anim_bitmap_id_smoke == -1 )	{
		Anim_bitmap_id_smoke = bm_load_animation( "particlesmoke01", &Anim_num_frames_smoke, &fps, NULL, 0 );
	}

	// wheeze
	if ( Anim_bitmap_id_smoke2 == -1 )	{
		Anim_bitmap_id_smoke2 = bm_load_animation( "particlesmoke02", &Anim_num_frames_smoke2, &fps, NULL, 0 );
	}
}

// only call from game_shutdown()!!!
void particle_close()
{
	while (!Particles.empty())
	{		
		particle* part = Particles.back();
		part->signature = 0;
		delete part;

		Particles.pop_back();
	}
}

void particle_page_in()
{
	bm_page_in_texture( Anim_bitmap_id_fire );
	bm_page_in_texture( Anim_bitmap_id_smoke );
	bm_page_in_texture( Anim_bitmap_id_smoke2 );
}

DCF(particles,"Turns particles on/off")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Particles_enabled = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Particles_enabled = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Particles_enabled ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: particles [bool]\nTurns particle system on/off.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "particles are %s\n", (Particles_enabled?"ON":"OFF") );	
}


int Num_particles_hwm = 0;

// Creates a single particle. See the PARTICLE_?? defines for types.
particle *particle_create( particle_info *pinfo )
{
	if ( !Particles_enabled )
	{
		return NULL;
	}

	particle* new_particle = new particle();
	int fps = 1;
	
	new_particle->pos = pinfo->pos;
	new_particle->velocity = pinfo->vel;
	new_particle->age = 0.0f;
	new_particle->max_life = pinfo->lifetime;
	new_particle->radius = pinfo->rad;
	new_particle->type = pinfo->type;
	new_particle->optional_data = pinfo->optional_data;
	new_particle->tracer_length = pinfo->tracer_length;
	new_particle->attached_objnum = pinfo->attached_objnum;
	new_particle->attached_sig = pinfo->attached_sig;
	new_particle->reverse = pinfo->reverse;
	new_particle->particle_index = (int)Particles.size();

	switch (pinfo->type) {
		case PARTICLE_BITMAP:
		case PARTICLE_BITMAP_PERSISTENT: {
			if (pinfo->optional_data < 0) {
				Int3();
				delete new_particle;
				return NULL;
			}

			bm_get_info( pinfo->optional_data, NULL, NULL, NULL, &new_particle->nframes, &fps );

			if ( new_particle->nframes > 1 )	{
				// Recalculate max life for ani's
				new_particle->max_life = i2fl(new_particle->nframes) / i2fl(fps);
			}

			break;
		}

		case PARTICLE_FIRE: {
			if (Anim_bitmap_id_fire < 0) {
				delete new_particle;
				return NULL;
			}

			new_particle->optional_data = Anim_bitmap_id_fire;
			new_particle->nframes = Anim_num_frames_fire;

			break;
		}

		case PARTICLE_SMOKE: {
			if (Anim_bitmap_id_smoke < 0) {
				delete new_particle;
				return NULL;
			}

			new_particle->optional_data = Anim_bitmap_id_smoke;
			new_particle->nframes = Anim_num_frames_smoke;

			break;
		}

		case PARTICLE_SMOKE2: {
			if (Anim_bitmap_id_smoke2 < 0) {
				delete new_particle;
				return NULL;
			}

			new_particle->optional_data = Anim_bitmap_id_smoke2;
			new_particle->nframes = Anim_num_frames_smoke2;

			break;
		}

		default:
			new_particle->nframes = 1;
			break;
	}
	
	new_particle->signature = ++lastSignature;
	Particles.push_back( new_particle );

#ifndef NDEBUG
	if (Particles.size() > (uint)Num_particles_hwm) {
		Num_particles_hwm = (int)Particles.size();

		nprintf(("Particles", "Num_particles high water mark = %i\n", Num_particles_hwm));
	}
#endif

	return Particles.back();
}

particle *particle_create( vec3d *pos, vec3d *vel, float lifetime, float rad, int type, int optional_data, float tracer_length, object *objp, bool reverse )
{
	particle_info pinfo;

	if ( (type < 0) || (type >= NUM_PARTICLE_TYPES) ) {
		Int3();
		return NULL;
	}

	// setup old data
	pinfo.pos = *pos;
	pinfo.vel = *vel;
	pinfo.lifetime = lifetime;
	pinfo.rad = rad;
	pinfo.type = type;
	pinfo.optional_data = optional_data;	

	// setup new data
	pinfo.tracer_length = -1.0f;
	if(objp == NULL)
	{
		pinfo.attached_objnum = -1;
		pinfo.attached_sig = -1;
	}
	else
	{
		pinfo.attached_objnum = OBJ_INDEX(objp);
		pinfo.attached_sig = objp->signature;
	}
	pinfo.reverse = reverse? 1 : 0;

	// lower level function
	return particle_create(&pinfo);
}

MONITOR( NumParticles )

void particle_move_all(float frametime)
{
	MONITOR_INC( NumParticles, Num_particles );	

	if ( !Particles_enabled )
		return;

	if ( Particles.empty() )
		return;

	for (SCP_vector<particle*>::iterator p = Particles.begin(); p != Particles.end(); ) {	
		particle* part = *p;
		if (part->age == 0.0f) {
			part->age = 0.00001f;
		} else {
			part->age += frametime;
		}

		// if it's time expired, remove it
		if (part->age > part->max_life) {
			// special case, if max_life is 0 then we want it to render at least once
			if ( (part->age > frametime) || (part->max_life > 0.0f) ) {
				part->signature = 0;
				delete *p;
				*p = NULL;

				*p = Particles.back();
				Particles.pop_back();
				continue;
			}
		}

		// if the particle is attached to an object which has become invalid, kill it
		if (part->attached_objnum >= 0) {
			// if the signature has changed, or it's bogus, kill it
			if ( (part->attached_objnum >= MAX_OBJECTS)
				|| (part->attached_sig != Objects[part->attached_objnum].signature) )
			{
				part->signature = 0;
				delete *p;
				*p = NULL;

				*p = Particles.back();
				Particles.pop_back();
				continue;
			}
		}
		// move as a regular particle
		else {
			vm_vec_scale_add2( &part->pos, &part->velocity, frametime );
		}

		// next particle
		++p;
	}
}

// kill all active particles
void particle_kill_all()
{
	// kill all active particles
	Num_particles = 0;
	Num_particles_hwm = 0;

	while (!Particles.empty())
	{
		particle* part = Particles.back();
		part->signature = 0;
		delete part;
		Particles.pop_back();
	}
}

MONITOR( NumParticlesRend )

static float get_current_alpha(vec3d *pos)
{
	float dist;
	float alpha;

	const float inner_radius = 30.0f;
	const float magic_num = 2.75f;

	// determine what alpha to draw this bitmap with
	// higher alpha the closer the bitmap gets to the eye
	dist = vm_vec_dist_quick(&Eye_position, pos);

	// if the point is inside the inner radius, alpha is based on distance to the player's eye,
	// becoming more transparent as it gets close
	if (dist <= inner_radius) {
		// alpha per meter between the magic # and the inner radius
		alpha = 0.99999f / (inner_radius - magic_num);

		// above value times the # of meters away we are
		alpha *= (dist - magic_num);
		return (alpha < 0.05f) ? 0.0f : alpha;
	}

	return 0.99999f;
}

void particle_render_all()
{
	ubyte flags;
	float pct_complete;
	float alpha;
	vertex pos;
	vec3d ts, te, temp;
	int rotate = 1;
	int framenum, cur_frame;
	bool render_batch = false;
	int tmap_flags = TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD;

	if ( !Particles_enabled )
		return;

	MONITOR_INC( NumParticlesRend, Num_particles );	

	if ( Particles.empty() )
		return;

	for (SCP_vector<particle*>::iterator p = Particles.begin(); p != Particles.end(); ++p) {
		particle* part = *p;
		// skip back-facing particles (ripped from fullneb code)
		// Wanderer - add support for attached particles
		vec3d p_pos;
		if (part->attached_objnum >= 0) {
			vm_vec_unrotate(&p_pos, &part->pos, &Objects[part->attached_objnum].orient);
			vm_vec_add2(&p_pos, &Objects[part->attached_objnum].pos);
		} else {
			p_pos = part->pos;
		}

		if ( vm_vec_dot_to_point(&Eye_matrix.vec.fvec, &Eye_position, &p_pos) <= 0.0f ) {
			continue;
		}

		// calculate the alpha to draw at
		alpha = get_current_alpha(&p_pos);

		// if it's transparent then just skip it
		if (alpha <= 0.0f) {
			continue;
		}

		// make sure "rotate" is enabled for this particle
		rotate = 1;

		// if this is a tracer style particle, calculate tracer vectors
		if (part->tracer_length > 0.0f) {			
			ts = p_pos;
			temp = part->velocity;
			vm_vec_normalize_quick(&temp);
			vm_vec_scale_add(&te, &ts, &temp, part->tracer_length);

			// don't bother rotating
			rotate = 0;
		}

		// rotate the vertex
		if (rotate) {
			flags = g3_rotate_vertex( &pos, &p_pos );

			if ( flags ) {
				continue;
			}

			if (!Cmdline_nohtl)
				g3_transfer_vertex(&pos, &p_pos);
		}

		// pct complete for the particle
		pct_complete = part->age / part->max_life;

		// figure out which frame we should be using
		if (part->nframes > 1) {
			framenum = fl2i(pct_complete * part->nframes + 0.5);
			CLAMP(framenum, 0, part->nframes-1);

			cur_frame = part->reverse ? (part->nframes - framenum - 1) : framenum;
		} else {
			cur_frame = 0;
		}

		if (part->type == PARTICLE_DEBUG) {
			gr_set_color( 255, 0, 0 );
			g3_draw_sphere_ez( &p_pos, part->radius );
		} else {
			framenum = part->optional_data;

			Assert( cur_frame < part->nframes );

			// if this is a tracer style particle
			if (part->tracer_length > 0.0f) {
				batch_add_laser( framenum + cur_frame, &ts, part->radius, &te, part->radius );
			}
			// draw as a regular bitmap
			else {
				batch_add_bitmap( framenum + cur_frame, tmap_flags, &pos, part->particle_index % 8, part->radius, alpha );
			}

			render_batch = true;
		}
	}

	if (render_batch) {
		batch_render_all();
	}
}




//============================================================================
//============== HIGH-LEVEL PARTICLE SYSTEM CREATION CODE ====================
//============================================================================

// Use a structure rather than pass a ton of parameters to particle_emit
/*
typedef struct particle_emitter {
	int		num_low;			// Lowest number of particles to create
	int		num_high;			// Highest number of particles to create
	vec3d	pos;				// Where the particles emit from
	vec3d	vel;				// Initial velocity of all the particles
	float	lifetime;			// How long the particles live
	vec3d	normal;				// What normal the particle emit arond
	float	normal_variance;	// How close they stick to that normal 0=good, 1=360 degree
	float	min_vel;			// How fast the slowest particle can move
	float	max_vel;			// How fast the fastest particle can move
	float	min_rad;			// Min radius
	float	max_rad;			// Max radius
} particle_emitter;
*/

static inline int get_percent(int count)
{
	if (count == 0)
		return 0;

	// this should basically return a scale like:
	//  50, 75, 100, 125, 150, ...
	// based on value of 'count' (detail level)
	return ( 50 + (25 * (count-1)) );
}

// Creates a bunch of particles. You pass a structure
// rather than a bunch of parameters.
void particle_emit( particle_emitter *pe, int type, int optional_data, float range )
{
	int i, n;

	if ( !Particles_enabled )
		return;

	int n1, n2;

	// Account for detail
	int percent = get_percent(Detail.num_particles);

	//Particle rendering drops out too soon.  Seems to be around 150 m.  Is it detail level controllable?  I'd like it to be 500-1000 
	float min_dist = 125.0f;
	float dist = vm_vec_dist_quick( &pe->pos, &Eye_position ) / range;
	if ( dist > min_dist )	{
		percent = fl2i( i2fl(percent)*min_dist / dist );
		if ( percent < 1 ) {
			return;
		}
	}
	//mprintf(( "Dist = %.1f, percent = %d%%\n", dist, percent ));

	n1 = (pe->num_low*percent)/100;
	n2 = (pe->num_high*percent)/100;

	// How many to emit?
	n = (rand() % (n2-n1+1)) + n1;
	
	if ( n < 1 ) return;


	for (i=0; i<n; i++ )	{
		// Create a particle
		vec3d tmp_vel;
		vec3d normal;				// What normal the particle emit arond

		float radius = (( pe->max_rad - pe->min_rad ) * frand()) + pe->min_rad;

		float speed = (( pe->max_vel - pe->min_vel ) * frand()) + pe->min_vel;

		float life = (( pe->max_life - pe->min_life ) * frand()) + pe->min_life;

		normal.xyz.x = pe->normal.xyz.x + (frand()*2.0f - 1.0f)*pe->normal_variance;
		normal.xyz.y = pe->normal.xyz.y + (frand()*2.0f - 1.0f)*pe->normal_variance;
		normal.xyz.z = pe->normal.xyz.z + (frand()*2.0f - 1.0f)*pe->normal_variance;
		vm_vec_normalize_safe( &normal );
		vm_vec_scale_add( &tmp_vel, &pe->vel, &normal, speed );

		particle_create( &pe->pos, &tmp_vel, life, radius, type, optional_data );
	}
}


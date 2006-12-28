/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Particle/Particle.cpp $
 * $Revision: 2.23 $
 * $Date: 2006-12-28 00:59:48 $
 * $Author: wmcoolmon $
 *
 * Code for particle system
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.22  2006/12/01 04:50:59  Goober5000
 * reverted the buggy commit that caused Mantis bug #1101
 *
 * Revision 2.21  2006/09/11 06:45:40  taylor
 * various small compiler warning and strict compiling fixes
 *
 * Revision 2.20  2006/09/08 06:18:29  taylor
 * small optimization
 *
 * Revision 2.18  2006/05/27 16:52:50  taylor
 * lots of little cleanup and minor fixage
 * make Particles[] dynamic (appears to improve processing speed of two particle functions by about 30%)
 * move to completely using Bobboau's geometry batcher instead of the stale one that RandomTiger did
 * little change to PARTICLE_SMOKE to flip UV orient differently, may add some realism, or just look kinda neat
 * get rid of particle_emit() stuff what was hardcoded for MAX_DETAIL_LEVEL value
 *
 * Revision 2.17  2006/01/21 09:36:58  wmcoolmon
 * Texture replacement stuff
 *
 * Revision 2.16  2005/09/24 02:43:33  Goober5000
 * whoops
 * --Goober5000
 *
 * Revision 2.14  2005/06/19 09:00:09  taylor
 * minor sanity checking for geometry_batcher
 * make particle batchers allocate dynamically
 * handle cases where a particle graphic couldn't be loaded
 *
 * Revision 2.13  2005/04/05 05:53:23  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.12  2005/03/24 23:37:25  taylor
 * fix GCC building
 *
 * Revision 2.11  2005/03/16 01:35:59  bobboau
 * added a geometry batcher and implemented it in sevral places
 * namely: lasers, thrusters, and particles,
 * these have been the primary botle necks for some time,
 * and this seems to have smoothed them out quite a bit.
 *
 * Revision 2.10  2005/02/04 20:06:06  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.9  2004/11/21 11:33:23  taylor
 * page in all frames at once rather than separately (old code that wasn't updated)
 *
 * Revision 2.8  2004/07/26 20:47:47  Kazan
 * remove MCD complete
 *
 * Revision 2.7  2004/07/12 16:33:02  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.6  2004/03/21 09:41:54  randomtiger
 * Fixed a bug that was causing windowed movie playback and a crash.
 * Added some batching redirection.
 *
 * Revision 2.5  2004/03/20 14:47:13  randomtiger
 * Added base for a general dynamic batching solution.
 * Fixed NO_DSHOW_CODE code path bug.
 *
 * Revision 2.4  2004/03/05 09:02:09  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2004/01/24 14:31:27  randomtiger
 * Added the D3D particle code, its not bugfree but works perfectly on my card and helps with the framerate.
 * Its optional and off by default, use -d3d_particle to activiate.
 * Also bumped up D3D ambient light setting, it was way too dark.
 * Its now set to something similar to the original game.
 *
 * Revision 2.2  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 11    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 10    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 9     7/07/99 3:11p Dave
 * Fix for uninitialized particle system data.
 * 
 * 8     4/22/99 11:06p Dave
 * Final pass at beam weapons. Solidified a lot of stuff. All that remains
 * now is to tweak and fix bugs as they come up. No new beam weapon
 * features.
 * 
 * 7     1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 6     1/28/99 9:10a Andsager
 * Particles increased in width, life, number.  Max particles increased
 * 
 * 5     1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 4     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 3     1/21/99 2:06p Dave
 * Final checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 28    5/13/98 3:25p John
 * Added code to make explosion impacts not get removed by other
 * particles.
 * 
 * 27    5/11/98 10:06a John
 * Added new particle for Adam
 * 
 * 26    4/30/98 11:31a Andsager
 * Added particles to big ship explosions.  Modified particle_emit() to
 * take optional range to increase range at which pariticles are created.
 * 
 * 25    4/17/98 1:42p Allender
 * fixed NDEBUG build problems
 * 
 * 24    4/17/98 6:58a John
 * Made particles not reduce in low mem conditions.
 * 
 * 23    4/15/98 4:21p John
 * Made particles drop off with distance smoothly.  Made old particles get
 * deleted by new ones.
 * 
 * 22    4/15/98 11:15a Adam
 * upped MAX_PARTICLES to 1500 (from 800).  Primary weapon hit sparks were
 * not finding slots.
 * 
 * 21    4/12/98 10:24a John
 * Made particle drop off distance larger for larger detail levels.
 * 
 * 20    4/09/98 7:58p John
 * Cleaned up tmapper code a bit.   Put NDEBUG around some ndebug stuff.
 * Took out XPARENT flag settings in all the alpha-blended texture stuff.
 * 
 * 19    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 18    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 17    4/01/98 10:57a Mike
 * Reduce array sizes to save memory.
 * 
 * 16    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 15    3/30/98 4:02p John
 * Made machines with < 32 MB of RAM use every other frame of certain
 * bitmaps.   Put in code to keep track of how much RAM we've malloc'd.
 * 
 * 14    3/26/98 5:26p John
 * added new paging code. nonfunctional.
 * 
 * 13    3/22/98 11:02a John
 * Made a bunch of the detail levels actually do something
 * 
 * 12    2/13/98 5:01p John
 * Made particles behind you not render
 * 
 * 11    2/06/98 7:28p John
 * Made debris and particles not get created if > 200 m from eye.   Added
 * max_velocity to asteroid's physics info for aiding in throwing out
 * collision pairs.
 * 
 * 10    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 9     1/29/98 11:48a John
 * Added new counter measure rendering as model code.   Made weapons be
 * able to have impact explosion.
 * 
 * 8     1/26/98 5:10p John
 * Took particle use out of registry.
 * 
 * 7     1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 6     1/13/98 8:09p John
 * Removed the old collision system that checked all pairs.   Added code
 * to disable collisions and particles.
 * 
 * 5     1/02/98 5:04p John
 * Several explosion related changes.  Made fireballs not be used as
 * ani's.  Made ship spark system expell particles.  Took away impact
 * explosion for weapon hitting ship... this needs to get added to weapon
 * info and makes shield hit more obvious.  Only make sparks when hit
 * hull, not shields.
 * 
 * 4     12/30/97 6:44p John
 * Made g3_Draw_bitmap functions account for aspect of bitmap.
 * 
 * 3     12/26/97 5:42p Adam
 * 
 * 2     12/23/97 3:58p John
 * Second rev of particles
 * 
 * 1     12/23/97 8:26a John
 *
 * $NoKeywords: $
 */

#include <vector>

#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "render/3d.h" 
#include "bmpman/bmpman.h"
#include "particle/particle.h"
#include "object/object.h"
#include "cmdline/cmdline.h"
#include "graphics/grbatch.h"
#include "parse/parselo.h"

#ifndef NDEBUG
#include "io/timer.h"
#endif


typedef struct particle {
	// old style data
	vec3d	pos;				// position
	vec3d	velocity;			// velocity
	float	age;				// How long it's been alive
	float	max_life;			// How much life we had
	float	radius;				// radius
	int		type;				// type										// -1 = None
	uint	optional_data;		// depends on type
	int		nframes;			// If an ani, how many frames?	

	// new style data
	float	tracer_length;		// if this is set, draw as a rod to simulate a "tracer" effect
	int		attached_objnum;	// if this is set, pos is relative to the attached object. velocity is ignored
	int		attached_sig;		// to check for dead/nonexistent objects
	ubyte	reverse;			// play any animations in reverse
	int		particle_index;		// used to keep particle offset in dynamic array for orient usage

	//WMC data
	int batcher;
} particle;

struct particle_batcher {
private:
	int bmap_id;
	int bmap_nframes;
	geometry_batcher *bmap_batchers;
	void clone(const particle_batcher &other)
	{
		bmap_id = other.bmap_id;
		bmap_nframes = other.bmap_nframes;

		if(other.bmap_batchers != NULL)
		{
			bmap_batchers = new geometry_batcher[bmap_nframes];
			for(int i = 0; i < bmap_nframes; i++) bmap_batchers[i] = other.bmap_batchers[i];
		}
		else
		{
			bmap_batchers = NULL;
		}
	}

public:
	particle_batcher(const particle_batcher &pb) { clone(pb); }
	particle_batcher(){bmap_id=-1;bmap_nframes=0;bmap_batchers=NULL;}
	int Set(char *fname)
	{
		bmap_id = bm_load_animation(fname, &bmap_nframes, NULL, 0);
		if(bmap_id < 0)
		{
			bmap_id = bm_load(fname);
			if(bmap_id > -1)
				bmap_nframes = 1;
			else
				bmap_nframes = 0;
		}
		else
		{
			bm_get_info(bmap_id, NULL, NULL, NULL, &bmap_nframes, NULL);
		}

		if(bmap_batchers != NULL)
		{
			delete[] bmap_batchers;
			bmap_batchers = NULL;
		}
		if(bmap_id > -1)
		{
			bmap_batchers =	new geometry_batcher[bmap_nframes];
		}

		return bmap_id;
	}

	int Set(int n_bmap_id)
	{
		if(bmap_batchers != NULL)
		{
			delete[] bmap_batchers;
			bmap_batchers = NULL;
		}
		if(!bm_is_valid(n_bmap_id))
		{
			bmap_id = -1;
			bmap_nframes = 0;
		}
		else
		{
			bmap_id = n_bmap_id;
			bmap_nframes = 1;
			bm_get_info(bmap_id, NULL, NULL, NULL, &bmap_nframes, NULL);
			bmap_batchers = new geometry_batcher[bmap_nframes];
		}

		return bmap_id;
	}

	~particle_batcher()
	{
		if(bmap_batchers != NULL)
		{
			delete[] bmap_batchers;
			bmap_batchers = NULL;
		}
	}

	const particle_batcher &operator =(const particle_batcher &other)
	{
		if (this != &other) {
			clone(other);
		}
		return *this;
	}

	//Funcs
	int GetBitmapId(){return bmap_id;}
	
	bool PageIn()
	{
		if(bmap_id > 0)
		{
			bm_page_in_texture(bmap_id, bmap_nframes);
			return true;
		}
		else
			return false;
	}

	void AddBitmap(int frame_id, vertex *pnt, int orient, float rad, float angle = 1.0f, float depth = 0.0f)
	{
		if(bmap_batchers == NULL)
			return;

		int idx = frame_id - bmap_id;
		bmap_batchers[idx].add_allocate(1);
		bmap_batchers[idx].draw_bitmap(pnt, orient, rad, angle, depth);
	}
	float AddLaser(int frame_id, vec3d *p0, float width1, vec3d *p1, float width2, int r = 255, int g = 255, int b = 255)
	{
		if(bmap_batchers == NULL)
			return 0.0f;

		int idx = frame_id - bmap_id;
		bmap_batchers[idx].add_allocate(1);
		return bmap_batchers[idx].draw_laser(p0, width1, p1, width2, r, g, b);
	}

	void Render()
	{
		if(bmap_batchers == NULL)
			return;

		int i;
		for(i = 0; i < bmap_nframes; i++)
		{
			const float alpha = 1.0f;
			const int tmap_flags = TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT;
			if ( bmap_batchers[i].need_to_render() ) {
				gr_set_bitmap(bmap_id + i, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha );
				bmap_batchers[i].render(tmap_flags);
			}
		}
	}
};

static std::vector<particle> Particles;
static int Particles_enabled = 1;
static std::vector<particle_batcher> Batchers;


// Reset everything between levels
void particle_init()
{
//	Particles_enabled = os_config_read_uint( NULL, "UseParticles", 0 );

	Particles.clear();
	Batchers.clear();
}

static int Particle_fire = -1;
int particle_get_fire_id()
{
	if(Particle_fire > -1)
		return Particle_fire;

	Particle_fire = bm_load_animation("particleexp01");
	if(Particle_fire < 0)
		Particle_fire = bm_load("particleexp01");

	return Particle_fire;
}

static int Particle_smoke = -1;
int particle_get_smoke_id()
{
	if(Particle_smoke > -1)
		return Particle_smoke;

	Particle_smoke = bm_load_animation("particlesmoke01");
	if(Particle_smoke < 0)
		Particle_smoke = bm_load("particlesmoke02");

	return Particle_smoke;
}

static int Particle_smoke2 = -1;
int particle_get_smoke2_id()
{
	if(Particle_smoke2 > -1)
		return Particle_smoke2;

	Particle_smoke2 = bm_load_animation("particlesmoke02");
	if(Particle_smoke2 < 0)
		Particle_smoke2 = bm_load("particlesmoke02");

	return Particle_smoke2;
}

// only call from game_shutdown()!!!
void particle_close()
{
	Particles.clear();
	Batchers.clear();
}

void particle_page_in()
{
	for(uint i = 0; i < Batchers.size(); i++)
	{
		Batchers[i].PageIn();
	}
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

//	os_config_write_uint( NULL, "UseParticles", Particles_enabled );
}


int Num_particles_hwm = 0;

int particle_load_batcher(int bmap_id)
{
	//WMC - make sure we get the right batcher
	int first_frame = bm_get_info(bmap_id);
	if(first_frame > -1)
	{
		for(uint i = 0; i < Batchers.size(); i++)
		{
			if(Batchers[i].GetBitmapId() == first_frame)
				return i;
		}

		Batchers.push_back(particle_batcher());
		int nidx = Batchers.size()-1;
		if(Batchers[nidx].Set(first_frame) == first_frame)
			return nidx;
	}

	return -1;
}

// Creates a single particle. See the PARTICLE_?? defines for types.
void particle_create( particle_info *pinfo )
{
	particle new_particle;

	if ( !Particles_enabled )
		return;

#ifdef NDEBUG
	//WMC - No texture to emit.
	if(pinfo->optional_data < 0)
		return;
#endif


	if (Particles.size() > (uint)Num_particles_hwm) {
		Num_particles_hwm = (int)Particles.size();

		nprintf(("Particles", "Num_particles high water mark = %i\n", Num_particles_hwm));
	}

	// Init the particle data
	memset( &new_particle, 0, sizeof(particle) );
	new_particle.pos = pinfo->pos;
	new_particle.velocity = pinfo->vel;
	new_particle.age = 0.0f;
	new_particle.max_life = pinfo->lifetime;
	new_particle.radius = pinfo->rad;
	new_particle.optional_data = pinfo->optional_data;
	new_particle.type = pinfo->type;
#ifndef NDEBUG
	if(new_particle.optional_data < 0)
		new_particle.type = PARTICLE_DEBUG;
#endif
	new_particle.tracer_length = pinfo->tracer_length;
	new_particle.attached_objnum = pinfo->attached_objnum;
	new_particle.attached_sig = pinfo->attached_sig;
	new_particle.reverse = pinfo->reverse;
	new_particle.particle_index = (int)Particles.size();

	if ( (new_particle.type == PARTICLE_BITMAP)/* || (new_particle.type == PARTICLE_BITMAP_PERSISTENT)*/ )
	{
		int fps;
		int first_frame = bm_get_info( new_particle.optional_data, NULL, NULL, NULL, &new_particle.nframes, &fps );
		if ( new_particle.nframes > 1 )
		{
			// Recalculate max life for ani's
			new_particle.max_life = i2fl(new_particle.nframes) / i2fl(fps);
			new_particle.batcher = particle_load_batcher(first_frame);
		}
#ifndef NDEBUG
		else
		{
			new_particle.type = PARTICLE_DEBUG;
		}
#endif
	}
	else
	{
		new_particle.nframes = 1;
	}

	Particles.push_back( new_particle );
}

void particle_create( vec3d *pos, vec3d *vel, float lifetime, float rad, int type, uint optional_data, float tracer_length, object *objp, bool reverse )
{
	particle_info pinfo;

	if ( (type < 0) || (type >= NUM_PARTICLE_TYPES) ) {
		Int3();
		return;
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
	particle_create(&pinfo);
}

MONITOR( NumParticles )

void particle_move_all(float frametime)
{
	particle *p;

	if ( !Particles_enabled )
		return;

	MONITOR_INC(NumParticles, Particles.size());

	for (uint i = 0; i < Particles.size(); i++) {
		p = &Particles[i];

		// bogus attached objnum
		if (p->attached_objnum >= MAX_OBJECTS) {
			Particles.erase( Particles.begin() + i );
			continue;
		}

		p->age += frametime;
	
		if ( p->age > p->max_life )	{
			// If it's time expired remove it
			Particles.erase( Particles.begin() + i );
			continue;
		}

		// if the vector is attached to an object which has become invalid, kill if
		if (p->attached_objnum >= 0) {
			// if the signature has changed, kill it
			if (p->attached_sig != Objects[p->attached_objnum].signature) {
				Particles.erase( Particles.begin() + i );
				continue;
			}
		}
		// move as a regular particle
		else {
			// Move the particle
			vm_vec_scale_add2( &p->pos, &p->velocity, frametime );		
		}
	}
}

// kill all active particles
void particle_kill_all()
{
//	int idx;

	// kill all active particles
	Num_particles_hwm = 0;

	Particles.clear();
	Batchers.clear();
}

MONITOR( NumParticlesRend )

void particle_render_all()
{
	particle *p;
	ubyte flags;
	float pct_complete;
	float alpha = 1.0f;
	vertex pos, pos_htl;
	vec3d ts, te, temp;
	int rotate = 1;
	uint i;
	int framenum;
//	int cur_frame;

	if ( !Particles_enabled )
		return;

	MONITOR_INC( NumParticlesRend, Particles.size() );	

//	int n = 0;
	int nclipped = 0;


	for (i = 0; i < Particles.size(); i++) {
		p = &Particles[i];

	//	n++;

		// make sure "rotate" is enabled for this particle
		rotate = 1;

		// pct complete for the particle
		pct_complete = p->age / p->max_life;

		// calculate the alpha to draw at
		alpha = 1.0f;	

		// if this is a tracer style particle, calculate tracer vectors
		if (p->tracer_length > 0.0f) {			
			ts = p->pos;
			temp = p->velocity;
			vm_vec_normalize_quick(&temp);
			vm_vec_scale_add(&te, &ts, &temp, p->tracer_length);

			// don't bother rotating
			rotate = 0;
		}
		// if this is an "attached" particle. move it
		else if (p->attached_objnum >= 0) {
			// offset the vector, and transform to view coords
			// vm_vec_add(&te, &Objects[p->attached_objnum].pos, &p->pos);

			vm_vec_unrotate(&temp, &p->pos, &Objects[p->attached_objnum].orient);
			vm_vec_add2(&temp, &Objects[p->attached_objnum].pos);

			flags = g3_rotate_vertex(&pos, &temp);

			if (flags) {				
				nclipped++;
				continue;
			}

			if (!Cmdline_nohtl)
				g3_transfer_vertex(&pos_htl, &temp);

			// don't bother rotating again
			rotate = 0;
		}

		// rotate the vertex
		if (rotate) {
			flags = g3_rotate_vertex( &pos, &p->pos );

			if ( flags ) {
				nclipped++;
				continue;
			}

			if (!Cmdline_nohtl)
				g3_transfer_vertex(&pos_htl, &p->pos);
		}

		switch (p->type) 
		{
			case PARTICLE_DEBUG:				// A red sphere, no optional data required
				gr_set_color( 255, 0, 0 );
				g3_draw_sphere_ez( &p->pos, p->radius );
				break;

			case PARTICLE_BITMAP:		
			{	// A bitmap, optional data is the bitmap number	
				framenum = p->optional_data;

				if ( p->nframes > 1 )	{
					int n = fl2i(pct_complete * p->nframes + 0.5);

					if ( n < 0 )
						n = 0;
					else if ( n > p->nframes-1 )
						n = p->nframes-1;

					framenum += n;
				}

				if(p->batcher < 0)
					break;

				// if this is a tracer style particle
				if (p->tracer_length > 0.0f) {					
					//batch_add_laser( framenum, &ts, p->radius, &te, p->radius );
					Batchers[p->batcher].AddLaser(framenum, &ts, p->radius, &te, p->radius);
				}
				// draw as a regular bitmap
				else {
					Batchers[p->batcher].AddBitmap(framenum, (Cmdline_nohtl) ? &pos : &pos_htl, p->particle_index % 8, p->radius, alpha);
					//batch_add_bitmap( framenum, tmap_flags, (Cmdline_nohtl) ? &pos : &pos_htl, p->particle_index % 8, p->radius, alpha );
				}

				break;
			}
		}
	}
	for(i = 0; i < Batchers.size(); i++)
	{
		Batchers[i].Render();
	}

//	mprintf(( "NP=%d, NCP=%d\n", n, nclipped ));
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
void particle_emit( particle_emitter *pe )
{
	int i, n;

	if ( !Particles_enabled )
		return;

#ifdef NDEBUG
	//WMC - No texture to emit.
	if(pe->texture_id < 0)
		return;
#endif

	int n1, n2;

	// Account for detail
	int percent = get_percent(Detail.num_particles);

	//Particle rendering drops out too soon.  Seems to be around 150 m.  Is it detail level controllable?  I'd like it to be 500-1000 
	float min_dist = 125.0f;
	float dist = vm_vec_dist_quick( &pe->pos, &Eye_position ) / pe->range;
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
#ifndef NDEBUG
		if(pe->texture_id < 0)
			particle_create( &pe->pos, &tmp_vel, life, radius, PARTICLE_DEBUG);
#else
		particle_create( &pe->pos, &tmp_vel, life, radius, PARTICLE_BITMAP, pe->texture_id );
#endif
	}
}

void parse_particle_emitter(particle_emitter *pe)
{
	Assert(pe != NULL);

	if(optional_string("+Texture:"))
	{
		char buf[MAX_FILENAME_LEN];
		stuff_string(buf, F_NAME, sizeof(buf));

		int th = bm_load_animation(buf);
		if(th < 0)
			th = bm_load(buf);

		if(th > -1)
		{
			if(pe->texture_id > -1)
				bm_unload(pe->texture_id);

			pe->texture_id = th;
		}
	}
	if(optional_string("+Relative Position:"))
		stuff_vector(&pe->pos);

	if(optional_string("+Relative Velocity:"))
		stuff_vector(&pe->vel);

	if(optional_string("+Relative Normal:"))
		stuff_vector(&pe->normal);

	if(optional_string("+Normal Variance:"))
		stuff_float(&pe->normal_variance);

	if(optional_string("+Min Radius:"))
		stuff_float(&pe->min_rad);

	if(optional_string("+Max Radius:"))
		stuff_float(&pe->max_rad);

	if(optional_string("+Min Speed:"))
		stuff_float(&pe->min_vel);

	if(optional_string("+Max Speed:"))
		stuff_float(&pe->max_vel);

	if(optional_string("+Min Lifetime:"))
		stuff_float(&pe->min_life);

	if(optional_string("+Max Lifetime:"))
		stuff_float(&pe->max_life);

	if(optional_string("+Min Particles:"))
		stuff_int(&pe->num_low);

	if(optional_string("+Max Particles:"))
		stuff_int(&pe->num_high);
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Shockwave.cpp $
 * $Revision: 2.14 $
 * $Date: 2005-07-24 06:01:37 $
 * $Author: wmcoolmon $
 *
 * C file for creating and managing shockwaves
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.13  2005/07/24 00:32:45  wmcoolmon
 * Synced 3D shockwaves' glowmaps with the model, tossed in some medals.tbl
 * support for the demo/FS1
 *
 * Revision 2.12  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.11  2005/03/10 08:00:17  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.10  2004/07/26 20:47:56  Kazan
 * remove MCD complete
 *
 * Revision 2.9  2004/07/12 16:33:09  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.8  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.7  2004/01/30 07:39:09  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.6  2003/10/23 18:03:25  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.5  2003/09/26 14:37:16  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.4  2003/08/31 06:00:42  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.3  2003/08/22 07:35:09  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.2  2003/03/18 08:44:06  Goober5000
 * added explosion-effect sexp and did some other minor housekeeping
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     9/01/99 10:15a Dave
 * 
 * 6     7/18/99 12:32p Dave
 * Randomly oriented shockwaves.
 * 
 * 5     3/23/99 2:29p Andsager
 * Fix shockwaves for kamikazi and Fred defined.  Collect together
 * shockwave_create_info struct.
 * 
 * 4     2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 49    5/18/98 3:04p Lawrance
 * Play shockwave impact sound
 * 
 * 48    5/18/98 12:59a Lawrance
 * Replace shockwave impact sound with a new "whoosh" sound that
 * originates from the shockwave center
 * 
 * 47    4/15/98 10:17p Mike
 * Training mission #5.
 * Fix application of subsystem damage.
 * 
 * 46    4/09/98 7:58p John
 * Cleaned up tmapper code a bit.   Put NDEBUG around some ndebug stuff.
 * Took out XPARENT flag settings in all the alpha-blended texture stuff.
 * 
 * 45    3/31/98 5:19p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 44    3/30/98 4:02p John
 * Made machines with < 32 MB of RAM use every other frame of certain
 * bitmaps.   Put in code to keep track of how much RAM we've malloc'd.
 * 
 * 43    3/26/98 5:26p John
 * added new paging code. nonfunctional.
 * 
 * 42    3/04/98 4:11p Lawrance
 * Have area effects affect asteroids, have asteroids cast an area effect,
 * fix ship shockwaves
 * 
 * 41    2/26/98 10:08p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 40    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 39    2/20/98 8:32p Lawrance
 * Make shockwaves affect subsystems more realistically.
 * 
 * 38    2/16/98 11:26a Lawrance
 * ensure shockwave lasts full duration
 * 
 * 37    2/16/98 10:04a Lawrance
 * Fix broken shockwave damage.
 * 
 * 36    2/14/98 4:42p Lawrance
 * pass shockwave object (not parent) to ship_apply_global_damage()
 * 
 * 35    2/12/98 11:54p Lawrance
 * restructure rendering code to use an animation
 * 
 * 34    2/11/98 5:38p Dave
 * Put in a global inited flag for shockwaves.
 * 
 * 33    2/02/98 8:47a Andsager
 * Ship death area damage applied as instantaneous damage for small ships
 * and shockwaves for large (>50 radius) ships.
 * 
 * 32    1/26/98 11:54a Lawrance
 * Don't allow Navbuoys to be affected by area-effect damage and blasts.
 * 
 * 31    1/22/98 11:43p Lawrance
 * Play sound effect when player is hit by shockwave.
 * 
 * 30    1/14/98 4:31p Dave
 * Made shockwaves apply damage correctly.
 * 
 * 29    1/14/98 2:59p Allender
 * if shockwave came from a weapon, make the shockwave's parent be the
 * weapons parent.
 * 
 * 28    12/30/97 6:44p John
 * Made g3_Draw_bitmap functions account for aspect of bitmap.
 * 
 * 27    12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 26    11/29/97 2:06p John
 * made g3_draw_bitmap and g3_draw_rotated bitmap take w&h, not w/2 & h/2,
 * like they used to incorrectly assume.   Added code to model to read in
 * thruster radius's.
 * 
 * 25    11/19/97 10:20p Lawrance
 * remove ship_shockwave from ship struct, handled in physics now
 * 
 * 24    11/16/97 8:52p Andsager
 * For shockwaves, update physics damping info in physics code.
 * 
 * 23    9/18/97 10:48p Lawrance
 * comment out unused struct member
 * 
 * 22    9/18/97 4:08p John
 * Cleaned up & restructured ship damage stuff.
 * 
 * 21    9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 20    9/04/97 5:10p Andsager
 * implement physics using moment of inertia and mass (from BSPgen).
 * Added to phys_info struct.  Updated ship_info, polymodel structs.
 * Updated weapon ($Mass and $Force) and ship ($Mass -> $Density) tables
 * 
 * 19    9/03/97 4:33p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 18    8/26/97 3:31p Andsager
 * scaled shockwave shake duration according to damage
 * 
 * 17    8/05/97 1:25a Mike
 * Make ship death roll be shortened by more damage.
 * 
 * 16    7/31/97 5:55p John
 * made so you pass flags to obj_create.
 * Added new collision code that ignores any pairs that will never
 * collide.
 * 
 * 15    7/25/97 4:30p Andsager
 * Save shockwave info
 * 
 * 14    7/25/97 1:04p Andsager
 * Modified physics flag PF_REDUCED_DAMP for damping when object is hit.
 * Flag is now set in physics_apply_whack/shock and turned off in
 * physics_sim_vel.  Weapons should not directly set this flag.
 * 
 * 13    7/22/97 2:40p Andsager
 * shockwaves now cause proper rotation of ships
 * 
 * 12    7/20/97 7:01p Lawrance
 * changed names of anim_ files to be more consistent
 * 
 * 11    7/18/97 10:52a Lawrance
 * let player have some control when shockwave hits
 * 
 * 10    7/17/97 8:02p Lawrance
 * tweaking shockwave effect
 * 
 * 9     7/16/97 5:51p Lawrance
 * make shockwaves translucent
 * 
 * 8     7/16/97 4:00p Lawrance
 * render shockwaves by default
 * 
 * 7     7/16/97 3:50p Lawrance
 * render shockwaves first, to fake transparency
 * 
 * 6     7/16/97 2:52p Lawrance
 * make shockwaves objects
 * 
 * 5     7/15/97 7:26p Lawrance
 * make shockwave blast persist over time
 * 
 * 4     7/09/97 1:56p Lawrance
 * add savegame support for shockwaves
 * 
 * 3     7/09/97 10:33a Lawrance
 * make area-effect spheres translucent
 * 
 * 2     7/08/97 6:00p Lawrance
 * implementing shockwaves
 * 
 * 1     7/08/97 1:30p Lawrance
 *
 * $NoKeywords: $
 */

#include "weapon/shockwave.h"
#include "render/3d.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "ship/shiphit.h"
#include "gamesnd/gamesnd.h"
#include "asteroid/asteroid.h"
#include "object/object.h"



// -----------------------------------------------------------
// Data structures
// -----------------------------------------------------------

// -----------------------------------------------------------
// Module-wide globals
// -----------------------------------------------------------

static char Shockwave_filenames[MAX_SHOCKWAVE_TYPES][NAME_LENGTH] = 
{
//XSTR:OFF
	"shockwave01",
	"\0",
	"\0",
	"\0",
//XSTR:ON
};

shockwave			Shockwaves[MAX_SHOCKWAVES];
shockwave_info	Shockwave_info[MAX_SHOCKWAVE_TYPES];

shockwave			Shockwave_list;
int					Shockwave_inited = 0;

int defalt_shockwave_model = -1;
// -----------------------------------------------------------
// Function macros
// -----------------------------------------------------------
#define SW_INDEX(sw) (sw-Shockwaves)
	
// -----------------------------------------------------------
// Externals
// -----------------------------------------------------------
extern int Show_area_effect;

// ------------------------------------------------------------------------------------
// shockwave_create()
//
// Call to create a shockwave
//
//	input:	parent_objnum	=> object number of object spawning the shockwave
//				pos				=>	vector specifing global position of shockwave center
//				speed				=>	speed at which shockwave expands (m/s)
//				inner_radius	=>	radius at which damage applied is at maximum
//				outer_radius	=> damage decreases linearly to zero from inner_radius to
//										outer_radius.  Outside outer_radius, damage is 0.
//				damage			=>	the maximum damage (ie within inner_radius)
//				blast				=> the maximux blast (within inner_radius)
//				sw_flag			=> indicates whether shockwave is from weapon or ship explosion
//				delay          => delay in ms before the shockwave actually starts
//
//	return:	success			=>	object number of shockwave
//				failure			=>	-1
//
// Goober5000 - now parent_objnum can be allowed to be -1
int shockwave_create(int parent_objnum, vec3d *pos, shockwave_create_info *sci, int flag, int delay, int model, int info_index)
{
	if(model == -1)
		model = defalt_shockwave_model;
	if(info_index < 0 || info_index > MAX_SHOCKWAVE_TYPES)
		info_index = 0;

	int				i, objnum, real_parent;
	shockwave		*sw;
	shockwave_info	*si;
	matrix			orient;

 	for ( i = 0; i < MAX_SHOCKWAVES; i++ ) {
		if ( !(Shockwaves[i].flags & SW_USED) ){
			break;
		}
	}

	if ( i == MAX_SHOCKWAVES ) {
		return -1;
	}

	// real_parent is the guy who caused this shockwave to happen
	if (parent_objnum == -1) {	// Goober5000
		real_parent = -1;
	} else if ( Objects[parent_objnum].type == OBJ_WEAPON ){
		real_parent = Objects[parent_objnum].parent;
	} else {
		real_parent = parent_objnum;
	}

	sw = &Shockwaves[i];

	sw->model = model;

	sw->flags = (SW_USED | flag);
	sw->speed = sci->speed;
	sw->inner_radius = sci->inner_rad;
	sw->outer_radius = sci->outer_rad;
	sw->damage = sci->damage;
	sw->blast = sci->blast;
	sw->radius = 1.0f;
	sw->pos = *pos;
	sw->num_objs_hit = 0;
	sw->shockwave_info_index=info_index;		// only one type for now... type could be passed is as a parameter
	sw->current_bitmap=-1;

	sw->time_elapsed=0.0f;
	sw->delay_stamp = delay;

	sw->rot_angles = sci->rot_angles;

	si = &Shockwave_info[sw->shockwave_info_index];
//	sw->total_time = i2fl(si->num_frames) / si->fps;	// in seconds
	sw->total_time = sw->outer_radius / sw->speed;

	if ( (parent_objnum != -1) && Objects[parent_objnum].type == OBJ_WEAPON ) {		// Goober5000: allow -1
		sw->weapon_info_index = Weapons[Objects[parent_objnum].instance].weapon_info_index;
	}
	else {		
		sw->weapon_info_index = -1;
	}

	orient = vmd_identity_matrix;
	vm_angles_2_matrix(&orient, &sw->rot_angles);
//	angles a;
//	a.p = sw->rot_angle*(PI/180);
//	a.b = frand_range(0.0f, 2.0f*PI);
//	a.h = frand_range(0.0f, 2.0f*PI);
//	vm_angles_2_matrix(&orient, &a);
	objnum = obj_create( OBJ_SHOCKWAVE, real_parent, i, &orient, &sw->pos, sw->outer_radius, OF_RENDERS );

	if ( objnum == -1 ){
		Int3();
	}

	sw->objnum = objnum;

	list_append(&Shockwave_list, sw);

	return objnum;
}

// ------------------------------------------------------------------------------------
// shockwave_delete()
//
// Delete a shockwave
//
//	input:	object *objp	=>		pointer to shockwave object
//
void shockwave_delete(object *objp)
{
	Assert(objp->type == OBJ_SHOCKWAVE);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHOCKWAVES);

	Shockwaves[objp->instance].flags = 0;
	Shockwaves[objp->instance].objnum = -1;	
	list_remove(&Shockwave_list, &Shockwaves[objp->instance]);
}

// ------------------------------------------------------------------------------------
// shockwave_delete_all()
//
//
void shockwave_delete_all()
{
	shockwave	*sw, *next;
	
	sw = GET_FIRST(&Shockwave_list);
	while ( sw != &Shockwave_list ) {
		next = sw->next;
		Assert(sw->objnum != -1);
		Objects[sw->objnum].flags |= OF_SHOULD_BE_DEAD;
		sw = next;
	}
}

// Set the correct frame of animation for the shockwave
void shockwave_set_framenum(int index)
{
	int				framenum;
	shockwave		*sw;
	shockwave_info	*si;

	sw = &Shockwaves[index];
	si = &Shockwave_info[sw->shockwave_info_index];

	framenum = fl2i(sw->time_elapsed / sw->total_time * si->num_frames + 0.5);

	// ensure we don't go past the number of frames of animation
	if ( framenum > (si->num_frames-1) ) {
		framenum = (si->num_frames-1);
		Objects[sw->objnum].flags |= OF_SHOULD_BE_DEAD;
	}

	if ( framenum < 0 ) {
		framenum = 0;
	}

	sw->current_bitmap = si->bitmap_id + framenum;
}

// ------------------------------------------------------------------------------------
// shockwave_move()
//
//	Simulate a single shockwave.  If the shockwave radius exceeds outer_radius, then
// delete the shockwave.
//
//	input:		ojbp			=>		object pointer that points to shockwave object
//					frametime	=>		time to simulate shockwave
//
void shockwave_move(object *shockwave_objp, float frametime)
{
	shockwave	*sw;
	object		*objp;
	float			blast,damage;
	int			i;
	
	Assert(shockwave_objp->type == OBJ_SHOCKWAVE);
	Assert(shockwave_objp->instance  >= 0 && shockwave_objp->instance < MAX_SHOCKWAVES);
	sw = &Shockwaves[shockwave_objp->instance];

	// if the shockwave has a delay on it
	if(sw->delay_stamp != -1){
		if(timestamp_elapsed(sw->delay_stamp)){
			sw->delay_stamp = -1;
		} else {
			return;
		}
	}

	sw->time_elapsed += frametime;
/*
	if ( sw->time_elapsed > sw->total_time ) {
		shockwave_objp->flags |= OF_SHOULD_BE_DEAD;
	}
*/

	shockwave_set_framenum(shockwave_objp->instance);
		
	sw->radius += (frametime * sw->speed);
	if ( sw->radius > sw->outer_radius ) {
		sw->radius = sw->outer_radius;
		shockwave_objp->flags |= OF_SHOULD_BE_DEAD;
		return;
	}

	// blast ships and asteroids
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) ) {
			continue;
		}
	
		if ( objp->type == OBJ_SHIP ) {
			// don't blast navbuoys
			if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY ) {
				continue;
			}
		}

		// only apply damage to a ship once from a shockwave
		for ( i = 0; i < sw->num_objs_hit; i++ ) {
			if ( objp->signature == sw->obj_sig_hitlist[i] ){
				break;
			}
		}
		if ( i < sw->num_objs_hit ){
			continue;
		}

		if ( weapon_area_calc_damage(objp, &sw->pos, sw->inner_radius, sw->outer_radius, sw->blast, sw->damage, &blast, &damage, sw->radius) == -1 ){
			continue;
		}

		// okay, we have damage applied, record the object signature so we don't repeatedly apply damage
		Assert(sw->num_objs_hit < SW_MAX_OBJS_HIT);
		if ( sw->num_objs_hit >= SW_MAX_OBJS_HIT) {
			sw->num_objs_hit--;
		}

		switch(objp->type) {
		case OBJ_SHIP:
			sw->obj_sig_hitlist[sw->num_objs_hit++] = objp->signature;
			ship_apply_global_damage(objp, shockwave_objp, &sw->pos, damage );
			weapon_area_apply_blast(NULL, objp, &sw->pos, blast, 1);
			break;
		case OBJ_ASTEROID:
			asteroid_hit(objp, NULL, NULL, damage);
			break;
		default:
			Int3();
			break;
		}


		// If this shockwave hit the player, play shockwave impact sound
		if ( objp == Player_obj ) {
			snd_play( &Snds[SND_SHOCKWAVE_IMPACT], 0.0f, MAX(0.4f, damage/Weapon_info[sw->weapon_info_index].damage) );
		}

	}	// end for
}

// ------------------------------------------------------------------------------------
// shockwave_render()
//
//	Draw the shockwave identified by handle
//
//	input:	objp	=>		pointer to shockwave object
//
extern int Cmdline_nohtl;
bool rendering_shockwave = false;
void shockwave_render(object *objp)
{
	shockwave		*sw;
	shockwave_info	*si;
	vertex			p;

	Assert(objp->type == OBJ_SHOCKWAVE);
	Assert(objp->instance >= 0 && objp->instance < MAX_SHOCKWAVES);

	sw = &Shockwaves[objp->instance];
	si = &Shockwave_info[sw->shockwave_info_index];
	int shockwave_model = sw->model;

	if( (sw->delay_stamp != -1) && !timestamp_elapsed(sw->delay_stamp)){
		return;
	}

	if ( sw->current_bitmap < 0 ){
		return;
	}

	// turn off fogging
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	if(shockwave_model > -1){
		float model_Interp_scale_x = sw->radius /40;
		float model_Interp_scale_y = sw->radius /40;
		float model_Interp_scale_z = sw->radius /40;

		set_warp_globals(model_Interp_scale_x, model_Interp_scale_y, model_Interp_scale_z, -1, 1.0f - (sw->radius/sw->outer_radius) );
		
		float dist = vm_vec_dist_quick( &sw->pos, &Eye_position );

		model_set_detail_level((int)(dist / (sw->radius * 10.0f)));
		gr_set_cull(0);
		rendering_shockwave = true;
		model_render( shockwave_model, &Objects[sw->objnum].orient, &sw->pos, MR_NO_LIGHTING | MR_NORMAL | MR_CENTER_ALPHA | MR_NO_CULL,sw->objnum);
		rendering_shockwave = false;
		gr_set_cull(1);

		model_Interp_scale_x = 1.0f;
		model_Interp_scale_y = 1.0f;
		model_Interp_scale_z = 1.0f;



		set_warp_globals(model_Interp_scale_x, model_Interp_scale_y, model_Interp_scale_z, -1, -1.0f);
	}else{
		if(!Cmdline_nohtl)g3_transfer_vertex(&p, &sw->pos );
		else g3_rotate_vertex(&p, &sw->pos );
	
		gr_set_bitmap(sw->current_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.3f );
		if(Cmdline_nohtl)g3_draw_rotated_bitmap(&p, fl_radian(sw->rot_angles.p), sw->radius, TMAP_FLAG_TEXTURED);	
		else g3_draw_rotated_bitmap(&p, fl_radian(sw->rot_angles.p), sw->radius, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);	
	}
}

// ------------------------------------------------------------------------------------
// shockwave_load()
//
// Call to load a Shockwave_type once its name is in Shockwave_filenames
//
int shockwave_load(int info_index)
{
	Assert(info_index > 0);
	Assert(info_index < MAX_SHOCKWAVE_TYPES);
	shockwave_info *si = &Shockwave_info[info_index];
	if(si->bitmap_id > 0)
	{
		Warning(LOCATION, "Attempted to load a shockwave twice!");
		return 1;
	}
	
	si->bitmap_id = bm_load_animation( Shockwave_filenames[info_index], &si->num_frames, &si->fps, 1 );
	if ( si->bitmap_id < 0 ) {
		Error(LOCATION, "Could not load shockwave '%s'\n", Shockwave_filenames[info_index]);
		return 0;
	}
	
	return 1;
}

// ------------------------------------------------------------------------------------
// shockwave_init()
//
// Call once at the start of each level (mission)
//
void shockwave_level_init()
{
	int i;	
	shockwave_info	*si;

	// load in shockwaves
	for ( i=0; i<NUM_DEFAULT_SHOCKWAVE_TYPES; i++ ) {
		shockwave_load(i);
	}
	
	for( i= NUM_DEFAULT_SHOCKWAVE_TYPES; i < MAX_SHOCKWAVE_TYPES; i++) {
		si = &Shockwave_info[i];
		si->bitmap_id = -1;
		si->num_frames = 0;
		si->fps = 0;
	}
	
	list_init(&Shockwave_list);

	for ( i = 0; i < MAX_SHOCKWAVES; i++ ) {
		Shockwaves[i].flags = 0;
		Shockwaves[i].objnum = -1;
	}

	mprintf(("loading defalt shockwave model"));
	defalt_shockwave_model = -1;
	defalt_shockwave_model = model_load("shockwave.pof", 0, NULL, 0);
	mprintf((" %d\n", defalt_shockwave_model));

	Shockwave_inited = 1;
}

// ------------------------------------------------------------------------------------
// shockwave_level_close()
//
// Call at the close of each level (mission)
void shockwave_level_close()
{
	if(Shockwave_inited){
		shockwave_delete_all();		
	}
	
	//Unload all non-default shockwaves
	int i,j;
	shockwave_info *si;
	for(i = NUM_DEFAULT_SHOCKWAVE_TYPES; i < MAX_SHOCKWAVE_TYPES; i++)
	{
		si = &Shockwave_info[i];
		for(j = 0; j < si->num_frames; j++)
			bm_unload(si->bitmap_id + j);
		si->bitmap_id = -1;
		si->num_frames = 0;
		si->fps = 0;
		Shockwave_filenames[i][0] = '\0';
	}
	Shockwave_inited = 0;
}

// ------------------------------------------------------------------------------------
// shockwave_close()
//
//	Called at game-shutdown to 
//
void shockwave_close()
{
}

// ------------------------------------------------------------------------------------
// shockwave_move_all()
//
//	Simulate all shockwaves in Shockwave_list
//
//	input:	frametime	=>		time for last frame in ms
//
void shockwave_move_all(float frametime)
{
	shockwave	*sw, *next;
	
	sw = GET_FIRST(&Shockwave_list);
	while ( sw != &Shockwave_list ) {
		next = sw->next;
		Assert(sw->objnum != -1);
		shockwave_move(&Objects[sw->objnum], frametime);
		sw = next;
	}
}

// ------------------------------------------------------------------------------------
// shockwave_render_all()
//
//
void shockwave_render_all()
{
	shockwave	*sw, *next;
	
	sw = GET_FIRST(&Shockwave_list);
	while ( sw != &Shockwave_list ) {
		next = sw->next;
		Assert(sw->objnum != -1);
		shockwave_render(&Objects[sw->objnum]);
		sw = next;
	}
}

// return the weapon_info_index field for a shockwave
int shockwave_weapon_index(int index)
{
	return Shockwaves[index].weapon_info_index;
}

// return the maximum radius for specified shockwave
float	shockwave_max_radius(int index)
{
	return Shockwaves[index].outer_radius;
}

void shockwave_page_in()
{
	int i;
	shockwave_info	*si;

	// load in shockwaves
	for ( i=0; i<MAX_SHOCKWAVE_TYPES; i++ ) {
		si = &Shockwave_info[i];
		bm_page_in_texture( si->bitmap_id, si->num_frames );
	}

}

int shockwave_add(char *bm_name)
{
	for(int i = 0; i < MAX_SHOCKWAVE_TYPES; i++)
	{
		if(!stricmp(Shockwave_filenames[i], bm_name))
		{
			return i;
		}
		else if(!strlen(Shockwave_filenames[i]))
		{
			strcpy(Shockwave_filenames[i], bm_name);
			
			shockwave_load(i);
			
			return i;
		}
	}
	
	Warning(LOCATION, "Could not add shockwave type %s, as MAX_SHOCKWAVE_TYPES has been reached. Contact an fs2_open coder if you REALLY think you need more than %d shockwaves", bm_name, MAX_SHOCKWAVE_TYPES-NUM_DEFAULT_SHOCKWAVE_TYPES);
	return -1;
}

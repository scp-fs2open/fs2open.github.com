/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/AiBig.cpp $
 * $Revision: 1.8 $
 * $Date: 2005-10-22 22:22:41 $
 * $Author: Goober5000 $
 *
 * C module for AI code related to large ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/09/01 04:14:03  taylor
 * various weapon_range cap fixes for primary, secondary weapons and hud targetting info
 *
 * Revision 1.5  2005/07/27 18:27:49  Goober5000
 * verified that submode_start_time is updated whenever submode is changed
 * --Goober5000
 *
 * Revision 1.4  2005/07/25 05:23:33  Goober5000
 * whoops
 * --Goober5000
 *
 * Revision 1.3  2005/07/25 03:13:23  Goober5000
 * various code cleanups, tweaks, and fixes; most notably the MISSION_FLAG_USE_NEW_AI
 * should now be added to all places where it is needed (except the turret code, which I still
 * have to to review)
 * --Goober5000
 *
 * Revision 1.2  2005/04/05 05:53:13  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 1.1  2005/03/25 06:45:12  wmcoolmon
 * Initial AI code move commit - note that aigoals.cpp has some escape characters in it, I'm not sure if this is really a problem.
 *
 * Revision 2.12  2005/03/10 08:00:14  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.11  2004/11/27 10:49:14  taylor
 * make sure the old behavior still applies to everything that's not a beam
 *
 * Revision 2.10  2004/07/26 20:47:50  Kazan
 * remove MCD complete
 *
 * Revision 2.9  2004/07/12 16:33:04  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.8  2004/03/05 09:01:51  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.7  2003/11/11 02:15:40  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.6  2003/06/25 03:16:32  phreak
 * changed around ai code to take into account a limited lock range for local ssms
 *
 * Revision 2.5  2003/06/11 03:03:16  phreak
 * the ai can now use local ssms. they are able to use them at *very* long range
 *
 * Revision 2.4  2003/03/20 08:24:45  unknownplayer
 * Modified the command line options so they are all in lower-case characters.
 * Made a slight AI adjustment to how ships choose to attack turrets (they now have a 25% chance of attacking a beam turret which has fired at them from another ship)
 *
 * Revision 2.3  2003/01/15 07:09:09  Goober5000
 * changed most references to modelnum to use ship instead of ship_info --
 * this will help with the change-model sexp and any other instances of model
 * changing
 * --Goober5000
 *
 * Revision 2.2  2002/10/19 19:29:28  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:28  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 13    8/31/99 4:24p Andsager
 * Reduce collisions when attacking big ships.
 * 
 * 12    7/19/99 2:18p Mikeb
 * DA:  DOH!!
 * 
 * 11    7/19/99 2:11p Mikeb
 * DA: sometimes big_ship_attack_normal is not known
 * 
 * 10    6/14/99 3:21p Andsager
 * Allow collisions between ship and its debris.  Fix up collision pairs
 * when large ship is warping out.
 * 
 * 9     6/02/99 5:41p Andsager
 * Reduce range of secondary weapons not fired from turrets in nebula.
 * Reduce range of beams fired from turrrets in nebula
 * 
 * 8     5/13/99 2:31p Jamesa
 * DA:  Temp fix when attacking subsys of big ship and BS is blown up.
 * 
 * 7     5/06/99 11:46a Andsager
 * Bug fixes.  Don't get into illegal strafe submode.  Don't choose turret
 * enemy objnum for beam protected.
 * 
 * 6     4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 5     4/20/99 3:40p Andsager
 * Changes to big ship ai.  Uses bounding box as limit where to fly to
 * when flying away.
 * 
 * 4     4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 77    5/25/98 12:12a Mike
 * Improve big ship avoidance to avoid big ships when in a nearby dock
 * mode.  Fixes very dumb looking behavior in sm2-01a.  Verified sm1-08a
 * not too dumb at start.
 * 
 * 76    5/23/98 2:38a Mike
 * Improve balance of cheat-firing Synaptics.
 * 
 * 75    5/23/98 12:40a Mike
 * Hook for Delta wing to fire Synaptic bombs in sm3-09a.
 * 
 * 74    5/22/98 12:08p Mike
 * Force reset of aspect locking when AI ships switch banks due to
 * good-time-to-unload-secondaries
 * 
 * 73    5/21/98 1:25p Lawrance
 * Fix exception in ai_bpap() when no verts are in closest octant
 * 
 * 72    5/21/98 9:57a Mike
 * Massively improve firing of bombs at big ships.  Improve willingness to
 * take incoming fire to deliver bomb.  Make a little easier to gain
 * aspect lock.
 * 
 * 71    5/12/98 10:06a Mike
 * Fix a jillion bugs preventing ships from firing bombs at Lucifer in
 * sm3-09.
 * 
 * 70    5/10/98 11:30p Mike
 * Better firing of bombs, less likely to go into strafe mode.
 * 
 * 69    5/10/98 3:41a Lawrance
 * Fix bug that was preventing ships from leaving strafe mode
 * 
 * 68    5/08/98 4:39p Mike
 * Make ships match bank when attacking large ships.
 * 
 * 67    5/07/98 11:59p Mike
 * Remove unneeded code, computation of num_verts.
 * 
 * 66    4/29/98 5:01p Mike
 * Large overhaul in how turrets fire.
 * 
 * 65    4/27/98 11:59p Mike
 * Intermediate checkin.  Getting big ship turrets firing at big ships to
 * use pick_big_attack_point.
 * 
 * 64    4/23/98 12:32a Mike
 * Fix bogus math which looked at (1-fov) instead of fov.
 * 
 * 63    4/13/98 4:52p Allender
 * remove AI_frametime and us flFrametime instead.  Make lock warning work
 * in multiplayer for aspect seeking missiles.  Debris fixups
 * 
 * 62    4/13/98 3:27p Lawrance
 * Fix bug that caused ships to leave strafe mode right away since they
 * hadn't taken damage for a while
 * 
 * 61    4/12/98 2:02p Mike
 * Make small ships avoid big ships.
 * Turn on Collide_friendly flag.
 * 
 * 60    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 59    3/25/98 12:00a Allender
 * MK: make sure that output from function gets a valid valud (ai_pbap I
 * think)
 * 
 * 58    3/24/98 10:07p Adam
 * AL: Add another Assert() to try and catch bogus wp->big_attack_point
 * 
 * 57    3/24/98 9:56p Adam
 * AL: Assert that local_attack_point is valid in ai_bpap
 * 
 * 56    3/21/98 3:36p Mike
 * Fix/optimize attacking of big ships.
 * 
 * 55    3/16/98 5:17p Mike
 * Make ships not get stuck on big ships when attacking.
 * 
 * 54    3/16/98 12:03a Mike
 * Add support for kamikaze mode.  Add AIF_NO_DYNAMIC which means
 * relentlessly pursue current goal.
 * 
 * 53    3/12/98 4:04p Lawrance
 * Leave strafe mode if haven't been hit for a while and no enemy
 * fighter/bombers are close by.
 * 
 * 52    3/09/98 5:12p Mike
 * Make sure Pl_objp uses are valid.
 * Throw asteroids at ships in asteroid field, not "Gal"
 * Support ships warp out if no goals after 30 seconds.
 * 
 * 51    3/03/98 11:51p Lawrance
 * Ensure target is a ship before looking at SIF flags
 * 
 * 50    3/02/98 11:35a Mike
 * Fix logic with ai_do_default_behavior -- return after call.
 * 
 * 49    2/19/98 7:37p Lawrance
 * Fix bug that was causing problems with subsystem path following.
 * 
 * 48    2/14/98 3:38p Mike
 * Make bombs have arm time, drop for 1/2 second, have hull_strength.
 * Make them not fired until closer to target.
 * 
 * 47    2/13/98 3:56p Mike
 * Make ships more likely to stick to player orders, not get so distracted
 * by getting hit by someone else.
 * 
 * 46    2/11/98 4:30p Lawrance
 * Improve disarm/disable behavior.
 * 
 * 45    2/11/98 1:16a Mike
 * Mods to AI behavior to make it harder to stay on a ship's tail.  Not
 * too much improvement.
 * 
 * 44    2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 43    1/30/98 11:28a Comet
 * Fix indexing problem into verts in ai_bpap()
 * 
 * 42    1/29/98 10:46p Mike
 * In ai_big_pick_attack_point, handle case of octant with no points.
 * 
 * 41    1/29/98 1:39p Mike
 * Better heat seeking homing on big ships.
 * 
 * 40    1/27/98 4:18p Mike
 * Not fire weapons too early when attacking large ships.  Aspect lock
 * take proper amount of time for AI ships.
 * 
 * 39    1/22/98 5:14p Lawrance
 * clean up ai_big code, clear path info when stop attacking a subsystem
 * 
 * 38    1/20/98 11:41p Mike
 * Ships use new system of preferred big weapons to fire.  Also, support
 * AIF_UNLOAD_SECONDARIES which means fire as fast as possible.
 * 
 * 37    1/20/98 9:47a Mike
 * Suppress optimized compiler warnings.
 * Some secondary weapon work.
 * 
 * 36    1/17/98 4:45p Mike
 * Better support for AI selection of secondary weapons.
 * 
 * 35    1/16/98 3:57p Mike
 * Clean up some bugs with attacking of protected ships.  Make turrets
 * only fire at targets that are within range.
 * 
 * 34    1/16/98 11:32a Mike
 * Fix bug when a ship is attacking a dead subsystem on a protected ship.
 * 
 * 33    1/07/98 1:32p Lawrance
 * Remove some nprintf output
 * 
 * 32    1/07/98 1:21p Sandeep
 * Fix bug where accessing uninited pointer
 * 
 * 31    1/06/98 6:58p Lawrance
 * Attack turrets (sometimes) when fired upon while attacking a ship.
 * 
 * 30    12/31/97 6:28p Lawrance
 * Improve attacking subsystems on moving ships.
 * 
 * 29    12/31/97 4:16p Lawrance
 * Use different fov's for attacking subsystems depending if ship is
 * moving
 * 
 * 28    12/31/97 4:05p Mike
 * Support for all teams optionally attacking all teams.
 * Lead big ships when attacking their hull.
 * 
 * 27    12/31/97 12:26p Lawrance
 * Don't enter strafe mode for transports, fix bug that prevented strafe
 * mode from getting entered.
 * 
 * 26    12/30/97 5:20p Mike
 * Don't fire secondaries at a protected ship if its hull is very weak.
 * 
 * 25    12/30/97 4:26p Lawrance
 * Take out some debug statements
 * 
 * 24    12/15/97 7:16p Lawrance
 * improving subsystem attacking
 * 
 * 23    12/02/97 11:58a Lawrance
 * avoid during strafe mode only when endangered by a weapon
 * 
 * 22    12/01/97 5:11p Lawrance
 * make strafe mode more effective... slow down when approaching and use
 * afterburner in avoids
 * 
 * 21    11/27/97 4:21p Lawrance
 * set submode to -1 when switching to AIM_NONE from AIM_STRAFE
 * 
 * 20    11/24/97 2:27a Lawrance
 * if attacking a big ship and moving very slow, enter strafe mode if
 * enemy fighter/bombers are close by
 * 
 * 19    11/14/97 11:27a Johnson
 * if goal_objnum is -1, and in STRAFE, return 1 from
 * ai_maybe_follow_subsys_path(), so break out of STRAFE code
 * 
 * 18    11/13/97 11:51a Johnson
 * ALAN: comment out Int3() in AiBig... need to test on my own machine
 * 
 * 17    11/12/97 11:51p Lawrance
 * only check target_objnum == goal_objnum when following path
 * 
 * 16    11/12/97 11:01p Lawrance
 * change call to ai_find_path()
 * ensure goal_objnum == target_objnum in STRAFE
 * 
 * 15    11/12/97 11:18a Lawrance
 * fix bug in path following of subsystems
 * 
 * 14    11/11/97 5:22p Mike
 * Don't allow ships to attack navbuoys.
 * Make ships lead big ships with dumbfire.
 * Make ships smarter about leaving evade weapon mode.
 * Fix a couple Assert() bugs.
 * 
 * 13    11/06/97 6:27p Lawrance
 * fix bug that was messing up retreat point in STRAFE mode
 * 
 * 12    11/06/97 4:53p Mike
 * Limit number of ships that can simultaneously attack another.  Make
 * turrets inherit AI class from parent ship.
 * 
 * 11    11/06/97 12:27a Mike
 * Better avoid behavior.
 * Modify ai_turn_towards_vector() to take a flag parameter.
 * 
 * 10    11/02/97 10:55p Lawrance
 * removed some unused code, added some comments
 * 
 * 9     11/01/97 4:01p Mike
 * Adapt to new ai_find_path().
 * 
 * 8     10/31/97 11:24a Lawrance
 * If taget changes from a big ship to small ship while in AIM_STRAFE,
 * switch to AIM_CHASE
 * 
 * 7     10/30/97 10:06p Lawrance
 * chg some timing values so ship will reach retreat point
 * 
 * 6     10/30/97 9:17p Lawrance
 * work on getting AIM_STRAFE working well with disable/disarm, try to
 * balance
 * 
 * 5     10/30/97 12:32a Lawrance
 * further work on AIM_STRAFE
 * 
 * 4     10/29/97 6:25p Lawrance
 * add AIM_STRAFE functionality
 * 
 * 3     10/26/97 4:19p Lawrance
 * added ai_get_weapon_dist()
 * 
 * 2     10/26/97 3:24p Lawrance
 * split off large ship ai code into AiBig.cpp
 *
 * $NoKeywords: $
 */

#include "ai/aibig.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "ship/ship.h"
#include "ship/afterburner.h"
#include "freespace2/freespace.h"
#include "weapon/weapon.h"
#include "io/timer.h"
#include "mission/missionparse.h"



#pragma optimize("", off)
#pragma auto_inline(off)

#define SCAN_FIGHTERS_INTERVAL	2000		// how often an AI fighter/bomber should scan for enemy fighter/bombers
														// if sitting still and pounding on a big ship.  If enemy fighters are
														// close ( < ENTER_STRAFE_THREAT_DIST ), then enter AIM_STRAFE

#define ENTER_STRAFE_THREAT_DIST_SQUARED	360000	// use squared distance, instead of 600

#define MIN_DOT_TO_ATTACK_SUBSYS				0.7f
#define MIN_DOT_TO_ATTACK_MOVING_SUBSYS	0.97f

// AI BIG MAGIC NUMBERS
#define	STRAFE_RETREAT_COLLIDE_TIME	2.0		// when anticipated collision time is less than this, begin retreat
#define	STRAFE_RETREAT_COLLIDE_DIST	100		// when perpendicular distance to *surface* is less than this, begin retreat
#define	STRAFE_RETREAT_BOX_DIST			300		// distance beyond the bounding box to retreat

#define	EVADE_BOX_BASE_DISTANCE			300		// standard distance to end evade submode
#define	EVADE_BOX_MIN_DISTANCE			200		// minimun distance to end evade submode, after long time

#define	ATTACK_STOP_DISTANCE				150		// when distance to target is less than this, put on brakes

#define	ATTACK_COLLIDE_BASE_DIST		300		// absolute distance at which to begin checking for possible collision
#define	ATTACK_COLLIDE_AVOID_DIST		60			// perpendicular distance to attack surface at which begin avoiding
#define	ATTACK_COLLIDE_AVOID_TIME		1.0		// anticipated collision time at which to begin evade
#define	ATTACK_COLLIDE_SLOW_DIST		150		// perpendicular distance to attack surface at which begin slowing down
#define	ATTACK_COLLIDE_SLOW_TIME		1.5		// anticipated collision time at which to begin slowing down


// forward declarations
void	ai_big_evade_ship();
void	ai_big_chase_attack(ai_info *aip, ship_info *sip, vec3d *enemy_pos, float dist_to_enemy, int modelnum);
void	ai_big_avoid_ship();
int	ai_big_maybe_follow_subsys_path(int do_dot_check=1);

extern int model_which_octant_distant_many( vec3d *pnt, int model_num,matrix *model_orient, vec3d * model_pos, polymodel **pm, int *octs);
extern void compute_desired_rvec(vec3d *rvec, vec3d *goal_pos, vec3d *cur_pos);
extern void big_ship_collide_recover_start(object *objp, object *big_objp, vec3d *collide_pos, vec3d *collision_normal);


//	Called by ai_big_pick_attack_point.
//	Generates a random attack point.
//	If truly_random flag set (haha), then generate a pretty random number.  Otherwise, generate a static rand which
//	tends to not change from frame to frame.
//	Try four times and choose nearest point to increase chance of getting a good point.
void ai_bpap(object *objp, vec3d *attacker_objp_pos, vec3d *attacker_objp_fvec, vec3d *attack_point, vec3d *local_attack_point, float fov, float weapon_travel_dist, vec3d *surface_normal)
{
	float		nearest_dist;
	vec3d	result_point, best_point;
	vec3d	rel_point;
	int		num_tries;
	model_octant	*octp;
	polymodel	*pm;
	int		i, q, octs[4];	

	best_point = objp->pos;
	nearest_dist = weapon_travel_dist;

	model_which_octant_distant_many(attacker_objp_pos, Ships[objp->instance].modelnum, &objp->orient, &objp->pos, &pm, octs);

	num_tries = (int) (vm_vec_dist(&objp->pos, attacker_objp_pos)/objp->radius);

	if (num_tries >= 4)
		num_tries = 1;
	else
		num_tries = 4 - num_tries;

	//	Index #0 is best one.
	if ( pm->octants[octs[0]].verts ) {
		*local_attack_point = *pm->octants[octs[0]].verts[0];	//	Set just in case it doesn't get set below.
	} else {
		vm_vec_zero(local_attack_point);
	}

	for (q=0; q<4; q++) {
		octp = &pm->octants[octs[q]];
		if (octp->nverts > 0) {

			if (num_tries > octp->nverts)
				num_tries = octp->nverts;

			if (num_tries > octp->nverts)
				num_tries = octp->nverts;

			int	best_index = -1;

			for (i=0; i<num_tries; i++) {
				int	index;
				float	dist, dot;
				vec3d	v2p;

				index = (int) (frand() * (octp->nverts));

				rel_point = *octp->verts[index];
				vm_vec_unrotate(&result_point, &rel_point, &objp->orient);
				vm_vec_add2(&result_point, &objp->pos);

				dist = vm_vec_normalized_dir(&v2p, &result_point, attacker_objp_pos);
				dot = vm_vec_dot(&v2p, attacker_objp_fvec);

				if (dot > fov) {
					if (dist < nearest_dist) {
						best_index = index;
						nearest_dist = dist;
						best_point = result_point;
						*local_attack_point = rel_point;
						Assert( !vm_is_vec_nan(local_attack_point) );
						if (dot > (1.0f + fov)/2.0f)	//	If this point is quite good, quit searching for a better one.
							goto done_1;
					}
				}
			}
		}
	}
done_1:

	*attack_point = best_point;

	// Cast from attack_objp_pos to local_attack_pos and check for nearest collision.
	// If no collision, cast to (0,0,0) [center of big ship]**  [best_point initialized to 000]

	// do in world coords to get attack point, then translate to local for local_attack_point
	vec3d attack_dir, end_point, temp;
	float dist;
	dist = vm_vec_normalized_dir(&attack_dir, attack_point, attacker_objp_pos);

	if (dist > 0.1) {
		vm_vec_scale_add(&end_point, attack_point, &attack_dir, 30.0f);
	} else {
		vm_vec_scale_add(&end_point, attack_point, attacker_objp_fvec, 30.0f);
	}
	
	mc_info mc;
	mc.model_num = Ships[objp->instance].modelnum;
	mc.orient = &objp->orient;
	mc.pos = &objp->pos;
	mc.p0 = attacker_objp_pos;
	mc.p1 = &end_point;
	mc.flags = MC_CHECK_MODEL;
	mc.radius = 0.0f;
	model_collide(&mc);

	if (mc.num_hits > 0) {
		*attack_point = mc.hit_point_world;
		vm_vec_sub(&temp, attack_point, &objp->pos);
		vm_vec_rotate(local_attack_point, &temp, &objp->orient);
		if (surface_normal) {
			vm_vec_unrotate(surface_normal, &mc.hit_normal, &objp->orient);
		}
	} else {
		vm_vec_zero(local_attack_point);
		*attack_point = objp->pos;
		if (surface_normal) {
			vm_vec_zero(surface_normal);
		}
	}
}

//	Stuff a point to attack based on nearest octant.
//	If no points in that octant, leave attack_point unmodified.
//
//	Note: Default value for fov is 1.0f  1.0f means don't use fov parameter.
//	If fov != 1.0f, try up to four times to find a point that's in the field of view.
void ai_big_pick_attack_point_turret(object *objp, ship_subsys *ssp, vec3d *gpos, vec3d *gvec, vec3d *attack_point, float fov, float weapon_travel_dist)
{
	if (!timestamp_elapsed(ssp->turret_pick_big_attack_point_timestamp)) {
		vec3d	result_point;
		vm_vec_unrotate(&result_point, &ssp->turret_big_attack_point, &objp->orient);
		vm_vec_add(attack_point, &result_point, &objp->pos);
	} else {
		vec3d	local_attack_point;
		ssp->turret_pick_big_attack_point_timestamp = timestamp(2000 + (int) (frand()*500.0f));
		ai_bpap(objp, gpos, gvec, attack_point, &local_attack_point, fov, weapon_travel_dist, NULL);
		ssp->turret_big_attack_point = local_attack_point;
	}
}

//	Stuff a point to attack based on nearest octant.
//	If no points in that octant, leave attack_point unmodified.
//
//	Note: Default value for fov is 1.0f  1.0f means don't use fov parameter.
//	If fov != 1.0f, try up to four times to find a point that's in the field of view.
//	Note, attacker_objp can be a ship or a weapon.
void ai_big_pick_attack_point(object *objp, object *attacker_objp, vec3d *attack_point, float fov)
{
	Assert(objp->instance > -1);
	Assert(objp->type == OBJ_SHIP);

	vec3d	local_attack_point;

	switch (attacker_objp->type) {
	case OBJ_SHIP: {
		ai_info	*attacker_aip;
		attacker_aip = &Ai_info[Ships[attacker_objp->instance].ai_index];
		if (!timestamp_elapsed(attacker_aip->pick_big_attack_point_timestamp)) {
			vec3d	result_point;

			vm_vec_unrotate(&result_point, &attacker_aip->big_attack_point, &objp->orient);
			vm_vec_add(attack_point, &result_point, &objp->pos);

			return;
		}
	
		attacker_aip->pick_big_attack_point_timestamp = timestamp(2000 + (int) (frand()*500.0f));
		break;
						}
	case OBJ_WEAPON: {
		weapon	*wp = &Weapons[attacker_objp->instance];

		if (!timestamp_elapsed(wp->pick_big_attack_point_timestamp)) {
			vec3d	result_point;

			vm_vec_unrotate(&result_point, &wp->big_attack_point, &objp->orient);
			vm_vec_add(attack_point, &result_point, &objp->pos);

			return;
		}
		wp->pick_big_attack_point_timestamp = timestamp(2000 + (int) (frand()*500.0f));
	
		break;
						  }
	}

	// checks valid line to target
	vec3d surface_normal;
	ai_bpap(objp, &attacker_objp->pos, &attacker_objp->orient.vec.fvec, attack_point, &local_attack_point, fov, 99999.9f, &surface_normal);

	switch (attacker_objp->type) {
	case OBJ_SHIP: {
		ai_info	*attacker_aip;
		// if we can't find a new local_attack_point don't change local_attack_point
		if (vm_vec_mag_squared(&local_attack_point) < 1) {
			return;
		}

		attacker_aip = &Ai_info[Ships[attacker_objp->instance].ai_index];
		attacker_aip->big_attack_point = local_attack_point;
		attacker_aip->big_attack_surface_normal = surface_normal;
		break;
						}
	case OBJ_WEAPON: {
		weapon	*wp = &Weapons[attacker_objp->instance];
		wp->big_attack_point = local_attack_point;
		Assert( !vm_is_vec_nan(&wp->big_attack_point) );
		break;
						  }
	}

	return;
}

// Handler for SM_EVADE submode ( called from ai_big_chase() )
void ai_big_evade_ship()
{
	vec3d	player_pos, enemy_pos;
	float		dist;
	ship		*shipp = &Ships[Pl_objp->instance];
	ai_info	*aip = &Ai_info[shipp->ai_index];
	vec3d	randvec, semi_enemy_pos;

	ai_set_positions(Pl_objp, En_objp, aip, &player_pos, &enemy_pos);

	dist = vm_vec_dist_quick(&player_pos, &enemy_pos);
	vm_vec_rand_vec_quick(&randvec);
	if ((Missiontime>>14) & 1) {
		vm_vec_scale_add(&semi_enemy_pos, &enemy_pos, &randvec, dist/2.0f);
		aip->prev_goal_point = semi_enemy_pos;
	} else {
		semi_enemy_pos = aip->prev_goal_point;
	}
	
	accelerate_ship(aip, 1.0f - ((Missiontime>>8) & 0x3f)/128.0f );
	turn_away_from_point(Pl_objp, &semi_enemy_pos, 0.0f);

	float box_dist;
	vec3d box_vec;
	int is_inside;
	box_dist = get_world_closest_box_point_with_delta(&box_vec, En_objp, &player_pos, &is_inside, EVADE_BOX_BASE_DISTANCE);
	if (box_dist > EVADE_BOX_BASE_DISTANCE) {
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
	} else if ((box_dist > EVADE_BOX_MIN_DISTANCE) && (Missiontime - aip->submode_start_time > i2f(5)) ) {
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
	}

	//TODO TEST

/*	if (dist > 4*En_objp->radius) {
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
	} else if (dist > En_objp->radius) {
		if (Missiontime - aip->submode_start_time > i2f(5)) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
		}
	} */
}

// Handler for SM_AVOID submode ( called from ai_big_chase() )
void ai_big_avoid_ship()
{
	ai_big_evade_ship();
}

// reset path following information
void ai_big_subsys_path_cleanup(ai_info *aip)
{
	if ( aip->ai_flags & AIF_ON_SUBSYS_PATH ) {
		aip->ai_flags &= ~AIF_ON_SUBSYS_PATH;
		aip->path_goal_dist = -1;
		aip->path_start = -1;
		aip->path_cur = -1;
		aip->path_length = 0;
	}
}

// Maybe Pl_objp needs to follow a path to get in line-of-sight to a subsystem
// input:	do_dot_check	=>	default value 0, flag to indicate whether check should be done to ensure
//										subsystem is within certain field of view.  We don't want to check fov when
//										strafing, since ship is weaving to avoid turret fire
int ai_big_maybe_follow_subsys_path(int do_dot_check)
{
	ai_info	*aip;
	float		dot = 1.0f, min_dot;
	object	*target_objp;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];
	target_objp = &Objects[aip->target_objnum];

	if ( (aip->targeted_subsys != NULL) && (aip->target_objnum >= 0) && (aip->targeted_subsys->system_info->path_num >= 0) ) {
		polymodel	*pm;
		int			subsys_path_num, subsys_in_sight, checked_sight;
		float			dist;

		pm = model_get( Ships[Pl_objp->instance].modelnum );
	
		// If attacking a subsystem, ensure that we have an unobstructed line of sight... if not, then move
		// towards path linked to subsystem
		subsys_in_sight = 0;	// assume Pl_objp doesn't have line of sight to subys

		// only check for subsystem sight every N milliseconds
		checked_sight = 0;
		if ( timestamp_elapsed(aip->path_subsystem_next_check) ) {
			vec3d		geye, gsubpos;
			eye			*ep;

			aip->path_subsystem_next_check = timestamp(1500);

			// get world pos of eye (stored in geye)
			ep = &(pm->view_positions[0] );
			model_find_world_point( &geye, &ep->pnt, Ships[Pl_objp->instance].modelnum, 0, &Pl_objp->orient, &Pl_objp->pos );
			
			// get world pos of subsystem
			vm_vec_unrotate(&gsubpos, &aip->targeted_subsys->system_info->pnt, &En_objp->orient);
			vm_vec_add2(&gsubpos, &En_objp->pos);

			checked_sight = 1;

			// Calc dot between subsys normal (based on using path info), and subsys_to_eye vector.  This is
			// useful later when we want to decide whether we have a realistic line-of-sight to the subsystem.
			vec3d subsys_normal, subsys_to_eye;
			if ( do_dot_check ) {
				if ( !ship_return_subsys_path_normal(&Ships[target_objp->instance], aip->targeted_subsys, &gsubpos, &subsys_normal) ) {
					vm_vec_normalized_dir(&subsys_to_eye, &geye, &gsubpos);
					dot = vm_vec_dot(&subsys_normal, &subsys_to_eye);
				}
			}

			if ( ship_subsystem_in_sight(En_objp, aip->targeted_subsys, &geye, &gsubpos, 1) ) {
				subsys_in_sight = 1;
			}
		}

		// check if subsystem not in sight
		min_dot = (target_objp->phys_info.fspeed > 5.0f?MIN_DOT_TO_ATTACK_MOVING_SUBSYS:MIN_DOT_TO_ATTACK_SUBSYS);
		if ( (checked_sight && ((!subsys_in_sight) || (dot < min_dot)) ) )  {

			aip->path_goal_dist = 5;
			subsys_path_num = aip->targeted_subsys->system_info->path_num;
			if ( (aip->path_start) == -1 || (aip->mp_index != subsys_path_num) ) {
				// maybe create a new path
				if ( subsys_path_num >= 0 ) {
					Assert(aip->target_objnum >= 0);
					ai_find_path(Pl_objp, aip->target_objnum, subsys_path_num, 0, 1);
					if ( aip->path_start >= 0 ) {
						aip->ai_flags |= AIF_ON_SUBSYS_PATH;
					}
				}
			}
		}

		if ( checked_sight && subsys_in_sight && (dot > min_dot) ) {
			// we've got a clear shot, stay here for a bit
			aip->path_subsystem_next_check = timestamp_rand(5000,8000);
		}

		// If there is a path that we are tracking, see if ship has gotten to within
		// aip->path_goal_dist units.  If so, then ship can stop following the path.  This
		// is required since we don't want to follow the path all the way to the subsystem,
		// and we want ships to stop different distances from their path destination
		if ( aip->path_length > 0 ) {
			int path_done=0;
			int in_view=0;

			Assert(aip->path_length >= 2);
			dist = vm_vec_dist_quick(&Path_points[aip->path_start+aip->path_length-2].pos, &Pl_objp->pos);

			if ( aip->path_cur >= (aip->path_start+aip->path_length-1) ) {
				path_done=1;
			}

			min_dot = (target_objp->phys_info.fspeed > 5.0f?MIN_DOT_TO_ATTACK_MOVING_SUBSYS:MIN_DOT_TO_ATTACK_SUBSYS);
			if ( (checked_sight && subsys_in_sight) && (dot > min_dot) ) {
				in_view=1;
			}

			// if we've reached the destination, then we can stop following the path
			if ( path_done || ( dist < aip->path_goal_dist ) || in_view ) {
				ai_big_subsys_path_cleanup(aip);
			} else if ( dist > aip->path_goal_dist ) {
				// If we have a path to follow, follow it and return			
				if ( aip->path_start != -1 ) {
					// for now, only follow the path to the first point
					if ( aip->path_cur < (aip->path_start+aip->path_length-1) ) {
						if ( aip->goal_objnum != aip->target_objnum ) {
							//Int3();	// what is going on here? - Get Alan
							aip->previous_mode = aip->mode;
							aip->mode = AIM_NONE;
							aip->submode = -1;
							aip->submode_start_time = Missiontime;
							return 1;
						}
						ai_path();
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

// This function is only called from ai_big_chase_attack() when a ship is flying very slowly and
// attacking a big ship.  The ship should scan for enemy fighter/bombers... if any are close, then
// return 1, otherwise return 0;
//
// input: aip	=>	ai_info pointer for Pl_objp
//			 sip	=>	ship_info pointer for Pl_objp
//
// exit:		1	=>	ship should enter strafe mode
//				0	=> ship should not change ai mode, no fighter/bomber threats are near
//
// NOTE: uses SCAN_FIGHTERS_INTERVAL and ENTER_STRAFE_THREAT_DIST_SQUARED which are defined in AiBig.h
int ai_big_maybe_start_strafe(ai_info *aip, ship_info *sip)
{
	// if moving slowly (or stopped), and SIF_SMALL_SHIP, then enter STRAFE mode if enemy fighter/bombers
	// are near
	if ( sip->flags & SIF_SMALL_SHIP ) {
		if ( timestamp_elapsed(aip->scan_for_enemy_timestamp) ) {
			ship_obj	*so;
			object	*test_objp;
			ship		*test_sp;
			float		dist_squared;
			
			aip->scan_for_enemy_timestamp = timestamp(SCAN_FIGHTERS_INTERVAL);
			// iterate through ships, and see if any fighter/bomber from opposite team are near
			so = GET_FIRST(&Ship_obj_list);
			while( so != END_OF_LIST(&Ship_obj_list) ) {
				test_objp = &Objects[so->objnum];
				test_sp = &Ships[test_objp->instance];

				if ( test_sp->team != Ships[Pl_objp->instance].team ) {
					if ( Ship_info[test_sp->ship_info_index].flags & SIF_SMALL_SHIP ) {
						dist_squared = vm_vec_dist_squared(&Pl_objp->pos, &test_objp->pos);
						if ( dist_squared < ENTER_STRAFE_THREAT_DIST_SQUARED ) {
							return 1;
						}
					}
				}
				so = GET_NEXT(so);
			}
		}
	}
	// If we've reached here, there are no enemy fighter/bombers near
	return 0;
}

//	ATTACK submode handler for chase mode.
void ai_big_chase_attack(ai_info *aip, ship_info *sip, vec3d *enemy_pos, float dist_to_enemy, int modelnum)
{
	int		start_bank;
	float		dot_to_enemy, time_to_hit;
	polymodel *po = model_get( modelnum );

	//	Maybe evade an incoming weapon.
	if (((time_to_hit = ai_endangered_by_weapon(aip)) < 4.0f) && (time_to_hit >= 0.0f)) {
		aip->submode = SM_EVADE_WEAPON;
		aip->submode_start_time = Missiontime;
		aip->prev_goal_point = En_objp->pos;
	} else {
		//	If moving slowly, maybe evade incoming fire.
		if (Pl_objp->phys_info.speed < 3.0f) {
			object *objp;
			for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
				if ((objp->type == OBJ_WEAPON) && (Weapons[objp->instance].team != Ships[Pl_objp->instance].team))
					if (Weapon_info[objp->instance].subtype == WP_LASER) {
						vec3d	in_vec;
						float		dist;

						vm_vec_sub(&in_vec, &objp->pos, &Pl_objp->pos);
						if (vm_vec_dot(&in_vec, &objp->orient.vec.fvec) > 0.0f) {
							dist = vm_vec_normalize(&in_vec);
							if ((dist < 200.0f) && (vm_vec_dot(&in_vec, &objp->orient.vec.fvec) > 0.95f)) {
								if ((Objects[objp->parent].signature == objp->parent_sig) && (vm_vec_dist_quick(&objp->pos, &Objects[objp->parent].pos) < 300.0f)) {
									set_target_objnum(aip, objp->parent);
									aip->submode = SM_ATTACK;
									aip->submode_start_time = Missiontime;
								} else {
									aip->submode = SM_EVADE;
									aip->submode_start_time = Missiontime;
									aip->prev_goal_point = En_objp->pos;
								}
							}
						}
					}
			}
			
			// Since ship is moving slowly and attacking a large ship, scan if enemy fighters are near, if so
			// then enter strafe mode
			if ( ai_big_maybe_start_strafe(aip, sip) ) {
				aip->previous_mode = aip->mode;
				aip->mode = AIM_STRAFE;
				aip->submode_parm0 = Missiontime;	// use parm0 as time strafe mode entered
				aip->submode = AIS_STRAFE_ATTACK;
				aip->submode_start_time = Missiontime;
				return;
			}

		} // end if ( Pl_objp->phys_info.speed < 3.0f ) 

		// see if Pl_objp needs to reposition to get a good shot at subsystem which is being attacked
		if ( ai_big_maybe_follow_subsys_path() ) {
			return;
		}

		vec3d	*rel_pos, vec_to_enemy;
		float		weapon_travel_dist;

		start_bank = Ships[aip->shipnum].weapons.current_primary_bank;

		if ((po->n_guns) && (start_bank != -1)) {
			rel_pos = &po->gun_banks[start_bank].pnt[0];
		} else
			rel_pos = NULL;

		dist_to_enemy = vm_vec_normalized_dir(&vec_to_enemy, enemy_pos, &Pl_objp->pos);
		dot_to_enemy = vm_vec_dot(&vec_to_enemy, &Pl_objp->orient.vec.fvec);

		vec3d	rvec_vec, *rvec = &rvec_vec;

		if (dist_to_enemy > 500.0f)
			compute_desired_rvec(rvec, enemy_pos, &Pl_objp->pos);
		else
			rvec = NULL;

		ai_turn_towards_vector(enemy_pos, Pl_objp, flFrametime, sip->srotation_time, NULL, rel_pos, 0.0f, 0, rvec);

		// calc range of primary weapon
		weapon_travel_dist = ai_get_weapon_dist(&Ships[Pl_objp->instance].weapons);

		if ( aip->targeted_subsys != NULL ) {
			if (dist_to_enemy > (weapon_travel_dist-20))
				accelerate_ship(aip, 1.0f);
			else {
				// AL 12-31-97: Move at least as quickly as your target is moving...
				accelerate_ship(aip, MAX(1.0f - dot_to_enemy, Objects[aip->target_objnum].phys_info.fspeed/sip->max_speed));
			}

		} else {
			float accel;
			if (dot_to_enemy < 0.0f) {
				accel = 0.5f;
			} else if (dot_to_enemy < 0.75f) {
				accel = 0.75f;
			} else {
				if (dist_to_enemy > weapon_travel_dist/2.0f) {
					accel = 1.0f;
				} else {
					accel = 1.0f - dot_to_enemy;
				}
			}

			// use dist normal to enemy here (dont break 50 barrier)
			if (dist_to_enemy < ATTACK_STOP_DISTANCE) {
//				accelerate_ship(aip, accel * 0.5f);
				accelerate_ship(aip, -1.0f);
			} else {
				accelerate_ship(aip, accel);
			}

		}
	}
}

// Handler for submode SM_CONTINUOUS_TURN
void ai_big_chase_ct()
{
	ai_chase_ct();
}

extern void ai_select_secondary_weapon(object *objp, ship_weapon *swp, int priority1 = -1, int priority2 = -1);
extern float set_secondary_fire_delay(ai_info *aip, ship *shipp, weapon_info *swip);
extern void ai_choose_secondary_weapon(object *objp, ai_info *aip, object *en_objp);
extern int maybe_avoid_big_ship(object *objp, object *ignore_objp, ai_info *aip, vec3d *goal_point, float delta_time);

extern void maybe_cheat_fire_synaptic(object *objp, ai_info *aip);

// Determine if Pl_objp should fire weapons at current target, based on input parameters
//
// dist_to_enemy	=>		distance (in m) to attack point on current target
// dot_to_enemy	=>		dot product between fvec of Pl_objp and vector from Pl_objp to attack point
//
void ai_big_maybe_fire_weapons(float dist_to_enemy, float dot_to_enemy, vec3d *firing_pos, vec3d *enemy_pos, vec3d *enemy_vel)
{
	ai_info		*aip;
	ship_weapon	*swp;
	int has_fired = -1;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];
	swp = &Ships[Pl_objp->instance].weapons;

	if (dot_to_enemy > 0.95f - 0.5f * En_objp->radius/MAX(1.0f, En_objp->radius + dist_to_enemy)) {
		aip->time_enemy_in_range += flFrametime;
		
		//	Chance of hitting ship is based on dot product of firing ship's forward vector with vector to ship
		//	and also the size of the target relative to distance to target.
		if (dot_to_enemy > MAX(0.5f, 0.90f + aip->ai_accuracy/10.0f - En_objp->radius/MAX(1.0f,dist_to_enemy))) {

			ship *temp_shipp;
			temp_shipp = &Ships[Pl_objp->instance];
			ship_weapon *tswp = &temp_shipp->weapons;

			if ( tswp->num_primary_banks > 0 ) {
				Assert(tswp->current_primary_bank < tswp->num_primary_banks);
				weapon_info	*wip = &Weapon_info[tswp->primary_bank_weapons[tswp->current_primary_bank]];

				if (dist_to_enemy < MIN((wip->max_speed * wip->lifetime), wip->weapon_range)){
					has_fired = 1;
					if(! ai_fire_primary_weapon(Pl_objp)){
						has_fired = -1;
					//	ship_stop_fire_primary(Pl_objp);
					}
				}

				int	priority1, priority2;

				priority1 = -1;
				priority2 = -1;

				//	Maybe favor selecting a bomb.
				//	Note, if you're firing a bomb, if it's aspect seeking, the firing conditions can be looser.
				if (Ship_info[Ships[En_objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))
					if (En_objp->phys_info.speed * dist_to_enemy < 5000.0f)		//	Don't select a bomb if enemy moving fast relative to distance
						priority1 = WIF_BOMB;

				if (!(En_objp->flags & OF_PROTECTED)) {
					//ai_select_secondary_weapon(Pl_objp, tswp, priority1, priority2);	//	Note, need to select to get weapon speed and lifetime

					ai_choose_secondary_weapon(Pl_objp, aip, En_objp);
					int current_bank = tswp->current_secondary_bank;
					weapon_info	*swip = &Weapon_info[tswp->secondary_bank_weapons[current_bank]];

					//	If ship is protected and very low on hits, don't fire missiles.
					if ((current_bank > -1) &&  (!(En_objp->flags & OF_PROTECTED) || (En_objp->hull_strength > 10*swip->damage))) {
						if (aip->ai_flags & AIF_UNLOAD_SECONDARIES) {
							if (timestamp_until(swp->next_secondary_fire_stamp[current_bank]) > swip->fire_wait*1000.0f) {
								swp->next_secondary_fire_stamp[current_bank] = timestamp((int) (swip->fire_wait*1000.0f));
							}
						}

						if (timestamp_elapsed(swp->next_secondary_fire_stamp[current_bank])) {
							float firing_range;
							if (swip->wi_flags2 & WIF2_LOCAL_SSM)
								firing_range=swip->lssm_lock_range;
							else
								firing_range = MIN((swip->max_speed * swip->lifetime), swip->weapon_range);
							// reduce firing range of secondaries in nebula
							extern int Nebula_sec_range;
							if ((The_mission.flags & MISSION_FLAG_FULLNEB) && Nebula_sec_range) {
								firing_range *= 0.8f;
							}

							float t = 0.25f;	//	default delay in seconds until next fire.

							if (dist_to_enemy < firing_range*1.0f) {


								//vm_vec_scale_add(&future_enemy_pos, enemy_pos, enemy_vel, dist_to_enemy/swip->max_speed);
								//if (vm_vec_dist_quick(&future_enemy_pos, firing_pos) < firing_range * 0.8f) {
									if (ai_fire_secondary_weapon(Pl_objp)) {
										if (aip->ai_flags & AIF_UNLOAD_SECONDARIES) {
											t = swip->fire_wait;
										} else {
											t = set_secondary_fire_delay(aip, temp_shipp, swip);
										}

										swp->next_secondary_fire_stamp[current_bank] = timestamp((int) (t*1000.0f));
									}
								//}
							}
							swp->next_secondary_fire_stamp[current_bank] = timestamp((int) (t*1000.0f));
						}
					}
				}
			}
		}
	} else {
		aip->time_enemy_in_range *= (1.0f - flFrametime);
	}

	if(has_fired == -1){	//stuff that hapens when the ship stops fireing
		ship_stop_fire_primary(Pl_objp);
	}

}

// switch ai ship into chase mode
void ai_big_switch_to_chase_mode(ai_info *aip)
{
	aip->previous_mode = aip->mode;
	aip->mode = AIM_CHASE;
	aip->submode = SM_ATTACK;
	aip->submode_start_time = Missiontime;
}

extern int ai_big_strafe_maybe_retreat(float dist, vec3d *target_pos);

// Make object Pl_objp chase object En_objp, which is a big ship, not a small ship.
void ai_big_chase()
{
	float			dist_to_enemy, dot_to_enemy;
	vec3d		player_pos, enemy_pos, vec_to_enemy;
	ship_info	*sip = &Ship_info[Ships[Pl_objp->instance].ship_info_index];
	ship			*shipp = &Ships[Pl_objp->instance];	
	ai_info		*aip = &Ai_info[shipp->ai_index];
	int			enemy_ship_type;
	vec3d		predicted_enemy_pos;

	Assert(aip->mode == AIM_CHASE);

	maybe_cheat_fire_synaptic(Pl_objp, aip);

	enemy_ship_type = Ship_info[Ships[En_objp->instance].ship_info_index].flags;

	ai_set_positions(Pl_objp, En_objp, aip, &player_pos, &enemy_pos);

	player_pos = Pl_objp->pos;
	ai_big_pick_attack_point(En_objp, Pl_objp, &enemy_pos, 0.8f);

	//	Compute the predicted position of the center of the ship, then add the delta to the goal pos.
	if (En_objp->phys_info.speed > 3.0f) {
		set_predicted_enemy_pos(&predicted_enemy_pos, Pl_objp, En_objp, aip);
		vm_vec_add2(&enemy_pos, &predicted_enemy_pos);
		vm_vec_sub2(&enemy_pos, &En_objp->pos);
	}	else
		predicted_enemy_pos = En_objp->pos;

	if (aip->targeted_subsys != NULL) {
		get_subsystem_pos(&enemy_pos, En_objp, aip->targeted_subsys);
	}

	dist_to_enemy = vm_vec_normalized_dir(&vec_to_enemy, &enemy_pos, &player_pos); // - En_objp->radius;
	dot_to_enemy = vm_vec_dot(&vec_to_enemy, &Pl_objp->orient.vec.fvec);

	if (aip->ai_flags & AIF_TARGET_COLLISION) {
		if ( ai_big_strafe_maybe_retreat(dist_to_enemy, &enemy_pos) ) {
			aip->mode = AIM_STRAFE;
			aip->submode_parm0 = Missiontime;	// use parm0 as time strafe mode entered
			aip->submode = AIS_STRAFE_AVOID;
			aip->submode_start_time = Missiontime;
			return;
		}
	}

	if (aip->ai_flags & AIF_KAMIKAZE) {
		//nprintf(("AI", "Kamikaze: %7.3f %7.3f\n", dot_to_enemy, dist_to_enemy));
		accelerate_ship(aip, 1.0f);
		if ((dist_to_enemy < 400.0f) && ai_maybe_fire_afterburner(Pl_objp, aip)) {
			afterburners_start(Pl_objp);
			aip->afterburner_stop_time = Missiontime + 3*F1_0;
		}
	}

	//	If just acquired target, or target is not in reasonable cone, don't refine believed enemy position.
	if ((dot_to_enemy < 0.25f) || (aip->target_time < 1.0f) || (aip->ai_flags & AIF_SEEK_LOCK)) {
		update_aspect_lock_information(aip, &vec_to_enemy, dist_to_enemy - En_objp->radius, En_objp->radius);
	} else if (aip->targeted_subsys != NULL) {		
		Assert(aip->targeted_subsys != NULL);
		get_subsystem_pos(&enemy_pos, En_objp, aip->targeted_subsys);
		vm_vec_add2(&enemy_pos, &predicted_enemy_pos);
		vm_vec_sub2(&enemy_pos, &En_objp->pos);
		dist_to_enemy = vm_vec_normalized_dir(&vec_to_enemy, &enemy_pos, &player_pos); // - En_objp->radius;
		dot_to_enemy = vm_vec_dot(&vec_to_enemy, &Pl_objp->orient.vec.fvec);
		update_aspect_lock_information(aip, &vec_to_enemy, dist_to_enemy, En_objp->radius);
	} else if (En_objp->flags & OF_PROTECTED) {	//	If protected and we're not attacking a subsystem, stop attacking!
		update_aspect_lock_information(aip, &vec_to_enemy, dist_to_enemy - En_objp->radius, En_objp->radius);
		aip->target_objnum = -1;
		if (find_enemy(Pl_objp-Objects, MAX_ENEMY_DISTANCE, Skill_level_max_attackers[Game_skill_level]) == -1) {
			ai_do_default_behavior(Pl_objp);
			return;
		}
	} else {
		update_aspect_lock_information(aip, &vec_to_enemy, dist_to_enemy - En_objp->radius, En_objp->radius);
	}

	//	If recently acquired target and not looking at target, just turn a bit.
	switch (aip->submode) {
	case SM_ATTACK:
	case SM_SUPER_ATTACK:
		if (vm_vec_dist_quick(&Pl_objp->pos, &predicted_enemy_pos) > 100.0f + En_objp->radius * 2.0f) {
			if (maybe_avoid_big_ship(Pl_objp, En_objp, aip, &predicted_enemy_pos, 10.0f))
				return;
		}

		if (aip->target_time < 2.0f)
			if ((dot_to_enemy < 0.9f) || (dist_to_enemy > 300.0f)) {
				aip->submode = SM_CONTINUOUS_TURN;
				aip->submode_start_time = Missiontime - fl2f(2.75f);	//	This backdated start time allows immediate switchout.
				if (dot_to_enemy > 0.0f)
					aip->target_time += flFrametime * dot_to_enemy;
			}
		break;
	}

	//
	//	Set turn and acceleration based on submode.
	//
	switch (aip->submode) {
	case SM_CONTINUOUS_TURN:
		ai_big_chase_ct();
		break;

	case SM_ATTACK:
	case SM_SUPER_ATTACK:
	case SM_ATTACK_FOREVER:
		ai_big_chase_attack(aip, sip, &enemy_pos, dist_to_enemy, shipp->modelnum);
		break;

	case SM_EVADE:
		ai_big_evade_ship();
		break;

	case SM_AVOID:
		ai_big_avoid_ship();
		break;

	case SM_EVADE_WEAPON:
		evade_weapon();
		break;

	default:
		aip->last_attack_time = Missiontime;
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
		break;
	}

	// maybe Pl_objp was forced into strafe mode, if so return now
	if ( aip->mode == AIM_STRAFE )
		return;

	//	Maybe choose a new submode.
	if (aip->submode != SM_AVOID && aip->submode != SM_EVADE ) {
		//	If a very long time since attacked, attack no matter what!
		if (aip->submode != SM_SUPER_ATTACK) {
			if (Missiontime - aip->last_attack_time > i2f(6)) {
				aip->submode = SM_SUPER_ATTACK;
				aip->submode_start_time = Missiontime;
				aip->last_attack_time = Missiontime;
			}
		}

		//	If a collision is expected, pull out!
		float dist_normal_to_enemy;

		if (vm_vec_mag_squared(&aip->big_attack_surface_normal) > 0.9) {
			dist_normal_to_enemy = fl_abs((dist_to_enemy * vm_vec_dot(&vec_to_enemy, &aip->big_attack_surface_normal)));
		} else {
			// don;t have normal so use a conservative value here
			dist_normal_to_enemy = 0.3f * dist_to_enemy;
		}

//		float time_to_enemy = dist_normal_to_enemy / Pl_objp->phys_info.speed * fl_abs(vm_vec_dot(&Pl_objp->phys_info.vel, &aip->big_attack_surface_normal));
//		if (Framecount % 30 == 1) {
//			mprintf(("normal dist; %.1f, time: %.1f\n", dist_normal_to_enemy, time_to_enemy));
//		}
		
		// since we're not in strafe and we may get a bad normal, cap dist_normal_to_enemy as MIN(0.3*dist_to_enemy, self)
		// this will allow us to get closer on a bad normal
		dist_normal_to_enemy = MAX(0.3f*dist_to_enemy, dist_normal_to_enemy);

		if (dist_to_enemy < ATTACK_COLLIDE_BASE_DIST) {
			// within 50m or 1sec
			float time_to_enemy;
			if (vm_vec_mag_squared(&aip->big_attack_surface_normal) > 0.9) {
				if (Pl_objp->phys_info.speed > 0.1) {
					time_to_enemy = dist_normal_to_enemy / fl_abs(vm_vec_dot(&Pl_objp->phys_info.vel, &aip->big_attack_surface_normal));
				} else {
					// a big time
					time_to_enemy = 100.0f;
				}
			} else {
				if (Pl_objp->phys_info.speed > 0.1) {
					time_to_enemy = dist_normal_to_enemy / Pl_objp->phys_info.speed;
				} else {
					// a big time
					time_to_enemy = 100.0f;
				}
			}

			float speed_dist = MAX(0.0f, (Pl_objp->phys_info.speed-50) * 2);
			if ((dist_normal_to_enemy < ATTACK_COLLIDE_AVOID_DIST + speed_dist) || (time_to_enemy < ATTACK_COLLIDE_AVOID_TIME) ) {
				// get away, simulate crsh recovery (don't use avoid)
//				accelerate_ship(aip, -1.0f);
				big_ship_collide_recover_start(Pl_objp, En_objp, &Pl_objp->pos, NULL);
//				aip->submode = SM_AVOID;
//				aip->submode_start_time = Missiontime;
			} else if ((dist_normal_to_enemy < ATTACK_COLLIDE_SLOW_DIST) || (time_to_enemy < ATTACK_COLLIDE_SLOW_TIME) ) {
				// slow down
				accelerate_ship(aip, -1.0f);
			}
		}

		/*
		if ((dot_to_enemy > 1.0f - 0.1f * En_objp->radius/(dist_to_enemy + 1.0f)) && (Pl_objp->phys_info.speed > dist_to_enemy/5.0f)) {
			if (might_collide_with_ship(Pl_objp, En_objp, dot_to_enemy, dist_to_enemy, (aip->targeted_subsys == NULL)*2.0f + 1.5f)) {
				if ((Missiontime - aip->last_hit_time > F1_0*4) && (dist_to_enemy < Pl_objp->radius*2 + En_objp->radius*2)) {
					accelerate_ship(aip, -1.0f);
				} else {
					aip->submode = SM_AVOID;
					aip->submode_start_time = Missiontime;
				}
			}
		} */
	}

	switch (aip->submode) {
	case SM_CONTINUOUS_TURN:
		if (Missiontime - aip->submode_start_time > i2f(3)) {
			aip->last_attack_time = Missiontime;
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
		}
		break;

	case SM_ATTACK:
	case SM_ATTACK_FOREVER:
		break;
		
	case SM_EVADE:
		if (dist_to_enemy > 4*En_objp->radius) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		break;

	case SM_SUPER_ATTACK:
		break;

	case SM_AVOID:
		// if outside box by > 300 m
		// DA 4/20/98  can never get here attacking a big ship
		{
		int is_inside;
		float box_dist = get_world_closest_box_point_with_delta(NULL, En_objp, &Pl_objp->pos, &is_inside, 0.0f);
		
		if (box_dist > 300) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		}
		break;

/*		if (dot_to_enemy > -0.2f) {
			aip->submode_start_time = Missiontime;
		} else if (Missiontime - aip->submode_start_time > i2f(1)/2) {
			if (might_collide_with_ship(Pl_objp, En_objp, dot_to_enemy, dist_to_enemy, 3.0f)) {
				aip->submode_start_time = Missiontime;
			} else {
				aip->submode = SM_ATTACK;
				aip->submode_start_time = Missiontime;
				aip->last_attack_time = Missiontime;
			}
		}

		break;*/

	case SM_EVADE_WEAPON:
		if (aip->danger_weapon_objnum == -1) {
			aip->submode = SM_ATTACK;
			aip->submode_start_time = Missiontime;
			aip->last_attack_time = Missiontime;
		}
		break;

	default:
		aip->submode = SM_ATTACK;
		aip->submode_start_time = Missiontime;
		aip->last_attack_time = Missiontime;
	}

	//
	//	Maybe fire primary weapon and update time_enemy_in_range
	//
	//nprintf(("AI", "time_enemy_in_range = %7.3f, dot = %7.3f\n", aip->time_enemy_in_range, dot_to_enemy));

	// AL: add condition that Pl_objp must not be following a path to fire.  This may be too extreme, but
	//     I noticed AI ships firing inappropriately when following a path near a big ship.
	//		 TODO: investigate why ships fire (and aren't close to hitting ship) when following a path near
	//				 a big ship
	if (aip->mode != AIM_EVADE && aip->path_start == -1 ) {
		ai_big_maybe_fire_weapons(dist_to_enemy, dot_to_enemy, &player_pos, &predicted_enemy_pos, &En_objp->phys_info.vel);
	} else
		aip->time_enemy_in_range *= (1.0f - flFrametime);
}

void ai_big_ship(object *objp)
{
	// do nothing
}

// all three parms are output parameters
//	Enemy object is En_objp
// pos	=>		world pos of attack point
// dist	=>		distance from Pl_objp front to attack point
// dot	=>		dot of Pl_objp fvec and vector to attack point
//	fire_pos =>	world pos from which firing
void ai_big_attack_get_data(vec3d *enemy_pos, float *dist_to_enemy, float *dot_to_enemy)
{
	vec3d		player_pos, vec_to_enemy, predicted_enemy_pos;	
	ship			*shipp = &Ships[Pl_objp->instance];	
	ai_info		*aip = &Ai_info[shipp->ai_index];
	ship_info	*esip = &Ship_info[Ships[En_objp->instance].ship_info_index];

	Assert(aip->mode == AIM_STRAFE);

	// ensure that Pl_objp is still targeting a big ship
	if ( !(esip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
		ai_big_switch_to_chase_mode(aip);
		return;
	}

	ai_set_positions(Pl_objp, En_objp, aip, &player_pos, enemy_pos);

	player_pos = Pl_objp->pos;

	if (aip->targeted_subsys != NULL) {
		Assert(aip->targeted_subsys != NULL);
		get_subsystem_pos(enemy_pos, En_objp, aip->targeted_subsys);
	} else {
		// checks valid line to target
		ai_big_pick_attack_point(En_objp, Pl_objp, enemy_pos, 0.8f);
	}

	// Take player pos to be center of ship + ship_radius
	vm_vec_scale_add2(&player_pos, &Pl_objp->orient.vec.fvec, Pl_objp->radius); 

	//	If seeking lock, try to point directly at ship, else predict position so lasers can hit it.
	//	If just acquired target, or target is not in reasonable cone, don't refine believed enemy position.
	vm_vec_normalized_dir(&vec_to_enemy, enemy_pos, &player_pos);
	*dot_to_enemy=vm_vec_dot(&vec_to_enemy, &Pl_objp->orient.vec.fvec);
	if ((*dot_to_enemy < 0.25f) || (aip->target_time < 1.0f) || (aip->ai_flags & AIF_SEEK_LOCK)) {
		predicted_enemy_pos=*enemy_pos;
	} else {
		vec3d	gun_pos, pnt;
		polymodel *po = model_get( shipp->modelnum );
		float		weapon_speed;

		//	Compute position of gun in absolute space and use that as fire position.
		pnt = po->gun_banks[0].pnt[0];
		vm_vec_unrotate(&gun_pos, &pnt, &Pl_objp->orient);
		vm_vec_add2(&gun_pos, &Pl_objp->pos);
		weapon_speed = ai_get_weapon_speed(&shipp->weapons);
		
		set_predicted_enemy_pos_turret(&predicted_enemy_pos, &gun_pos, Pl_objp, enemy_pos, &En_objp->phys_info.vel, weapon_speed, aip->time_enemy_in_range);
	}

	*dist_to_enemy = vm_vec_normalized_dir(&vec_to_enemy, &predicted_enemy_pos, &player_pos);
	*dot_to_enemy = vm_vec_dot(&vec_to_enemy, &Pl_objp->orient.vec.fvec);
	update_aspect_lock_information(aip, &vec_to_enemy, *dist_to_enemy, En_objp->radius);

	*enemy_pos = predicted_enemy_pos;
}

// check to see if Pl_objp has gotten too close to attacking point.. if so, break off by entering
// AIS_STRAFE_RETREAT
int ai_big_strafe_maybe_retreat(float dist, vec3d *target_pos)
{
	ai_info	*aip;
	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	vec3d vec_to_target;
	vm_vec_sub(&vec_to_target, target_pos, &Pl_objp->pos);

	float dist_to_target, dist_normal_to_target, time_to_target;
	dist_to_target = vm_vec_mag_quick(&vec_to_target);
	if (vm_vec_mag_quick(&aip->big_attack_surface_normal) > 0.9) {
		dist_normal_to_target = -vm_vec_dotprod(&vec_to_target, &aip->big_attack_surface_normal);
	} else {
		dist_normal_to_target = 0.2f * vm_vec_mag_quick(&vec_to_target);
	}

	dist_normal_to_target = MAX(0.2f*dist_to_target, dist_normal_to_target);
	time_to_target = dist_normal_to_target / Pl_objp->phys_info.speed;

	// add distance penalty for going too fast
	float speed_to_dist_penalty = MAX(0.0f, (Pl_objp->phys_info.speed-50));

	//if ((dot_to_enemy > 1.0f - 0.1f * En_objp->radius/(dist_to_enemy + 1.0f)) && (Pl_objp->phys_info.speed > dist_to_enemy/5.0f))

	// Inside 2 sec retreat, setting goal point to box point + 300m
	// If collision, use std collision resolution.
	if ( !(aip->ai_flags & AIF_KAMIKAZE) && ((aip->ai_flags & AIF_TARGET_COLLISION) || (time_to_target < STRAFE_RETREAT_COLLIDE_TIME) || (dist_normal_to_target < STRAFE_RETREAT_COLLIDE_DIST + speed_to_dist_penalty)) ) {
		if (aip->ai_flags & AIF_TARGET_COLLISION) {
			// use standard collision resolution
			aip->ai_flags &= ~AIF_TARGET_COLLISION;
			big_ship_collide_recover_start(Pl_objp, En_objp, &Pl_objp->pos, NULL);
		} else {
			// too close for comfort so fly to box point + 300
			aip->submode = AIS_STRAFE_RETREAT1;
			aip->submode_start_time = Missiontime;

			float box_dist;
			int is_inside;
			vec3d goal_point;
			box_dist = get_world_closest_box_point_with_delta(&goal_point, En_objp, &Pl_objp->pos, &is_inside, STRAFE_RETREAT_BOX_DIST);

			// set goal point
			aip->goal_point = goal_point;

			if (ai_maybe_fire_afterburner(Pl_objp, aip)) {
				afterburners_start(Pl_objp);
				aip->afterburner_stop_time = Missiontime + 3*F1_0;
			}
		}

		return 1;
	} else {
		return 0;
	}
}

// attack directly to the turret and fire weapons 
void ai_big_strafe_attack()
{
	ai_info	*aip;
	vec3d	target_pos;
	vec3d	rand_vec;
	float		target_dist, target_dot, accel, t;
	object	*target_objp;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	if ( ai_big_maybe_follow_subsys_path(0) ) {
		return;
	}

	ai_big_attack_get_data(&target_pos, &target_dist, &target_dot);
	if ( ai_big_strafe_maybe_retreat(target_dist, &target_pos) )
		return;

	target_objp = &Objects[aip->target_objnum];

	if (aip->ai_flags & AIF_KAMIKAZE) {
		if (target_dist < 1200.0f) {
			//nprintf(("AI", "Kamikaze: %7.3f %7.3f\n", target_dot, target_dist));
			ai_turn_towards_vector(&target_pos, Pl_objp, flFrametime, Ship_info[Ships[Pl_objp->instance].ship_info_index].srotation_time, NULL, NULL, 0.0f, 0);
			accelerate_ship(aip, 1.0f);
			if ((target_dist < 400.0f) && ai_maybe_fire_afterburner(Pl_objp, aip)) {
				afterburners_start(Pl_objp);
				aip->afterburner_stop_time = Missiontime + 3*F1_0;
			}
			return;
		}
	}

	if (!(aip->ai_flags & AIF_SEEK_LOCK) || (aip->aspect_locked_time < 2.0f)) {
		t  = ai_endangered_by_weapon(aip);
		if ( t > 0.0f && t < 1.5f ) {
			// set up goal_point for avoid path to turn towards
			aip->goal_point = Pl_objp->pos;
			switch(rand()%4) {
			case 0:
				vm_vec_scale_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.uvec, 2.0f);
				break;
			case 1:
				vm_vec_scale_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.uvec, -2.0f);
				break;
			case 2:
				vm_vec_scale_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.rvec, 2.0f);
				break;
			case 3:
				vm_vec_scale_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.rvec, -2.0f);
				break;
			} // end switch

			vm_vec_scale(&rand_vec, 1000.0f);
			vm_vec_add2(&aip->goal_point, &rand_vec);

			aip->submode = AIS_STRAFE_AVOID;
			aip->submode_start_time = Missiontime;
	//		nprintf(("Alan","Ship %s entering AIS_STRAFE_AVOID at frame %d\n", Ships[aip->shipnum].ship_name, Framecount));

			if (ai_maybe_fire_afterburner(Pl_objp, aip)) {
				afterburners_start(Pl_objp);
				aip->afterburner_stop_time = Missiontime + fl2f(0.5f);
			}
		}
	}

	// maybe fire weapons at target
	ai_big_maybe_fire_weapons(target_dist, target_dot, &Pl_objp->pos, &En_objp->pos, &En_objp->phys_info.vel);
	turn_towards_point(Pl_objp, &target_pos, NULL, 0.0f);

	// Slow down if we've not been hit for a while
	fix last_hit = Missiontime - aip->last_hit_time;
	if ( target_dist > 1200 || last_hit < F1_0*6) {
		accel = 1.0f;
	} else {
		float attack_time;
		attack_time = f2fl(Missiontime - aip->submode_start_time);
		if ( attack_time > 15 ) {
			accel = 0.2f;
		} else if ( attack_time > 10 ) {
			accel = 0.4f;
		} else if ( attack_time > 8 ) {
			accel = 0.6f;
		} else if ( attack_time > 5 ) {
			accel = 0.8f;
		} else {
			accel = 1.0f;
		}
	}

	accel = 1.0f;
	accelerate_ship(aip, accel);

	// if haven't been hit in quite a while, leave strafe mode
	fix long_enough;
	long_enough = F1_0*20;
	if ( (last_hit > long_enough) && ( (Missiontime - aip->submode_parm0) > long_enough) ) {
		ai_big_switch_to_chase_mode(aip);
	}
}

// pick a new attack point when entering this state, and keep using it
void ai_big_strafe_avoid()
{
	ai_info	*aip;
	vec3d	target_pos;
	float		target_dist, target_dot;
	fix		mode_time;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	mode_time = Missiontime - aip->submode_start_time;

	ai_big_attack_get_data(&target_pos, &target_dist, &target_dot);
	if ( ai_big_strafe_maybe_retreat(target_dist, &target_pos) )
		return;

	if ( mode_time > fl2f(0.5)) {
		aip->submode = AIS_STRAFE_ATTACK;
		aip->submode_start_time = Missiontime;
//		nprintf(("Alan","Ship %s entering AIS_STRAFE_ATTACK at frame %d\n", Ships[aip->shipnum].ship_name, Framecount));
	}

	turn_towards_point(Pl_objp, &aip->goal_point, NULL, 0.0f);
	accelerate_ship(aip, 1.0f);
}

// move towards aip->goal_point in an evasive manner
void ai_big_strafe_retreat1()
{
	float dist;
	ai_info	*aip;
	vec3d	rand_vec;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	dist = vm_vec_dist_quick(&Pl_objp->pos, &aip->goal_point);
	if ( dist < 70 ) {
		aip->submode = AIS_STRAFE_POSITION;
		aip->submode_start_time = Missiontime;
//		nprintf(("Alan","Ship %s entering AIS_STRAFE_POSITION\n", Ships[aip->shipnum].ship_name));
		return;
	}

	if (Missiontime - aip->submode_start_time > fl2f(1.50f)) {
		// set up goal_point for avoid path to turn towards
		aip->prev_goal_point = Pl_objp->pos;
		switch(rand()%4) {
		case 0:
			vm_vec_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.uvec);
			break;
		case 1:
			vm_vec_scale_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.uvec, -1.0f);
			break;
		case 2:
			vm_vec_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.rvec);
			break;
		case 3:
			vm_vec_scale_add(&rand_vec, &Pl_objp->orient.vec.fvec, &Pl_objp->orient.vec.rvec, -1.0f);
			break;
		} // end switch

		//vm_vec_scale(&rand_vec, 200.0f);
		//vm_vec_add2(&aip->prev_goal_point, &rand_vec);
		vm_vec_scale_add(&aip->prev_goal_point, &aip->goal_point, &rand_vec, 200.0f);

		aip->submode = AIS_STRAFE_RETREAT2;
		aip->submode_start_time = Missiontime;
	}

	turn_towards_point(Pl_objp, &aip->goal_point, NULL, 0.0f);
	accelerate_ship(aip, 1.0f);
}

void ai_big_strafe_retreat2()
{
	float dist;
	ai_info	*aip;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	if (Pl_objp->phys_info.flags & PF_AFTERBURNER_ON ) {
		if (Missiontime > aip->afterburner_stop_time) {
			//nprintf(("AI", "Frame %i, turning off afterburner.\n", AI_FrameCount));
			afterburners_stop(Pl_objp);
		}
	}

	dist = vm_vec_dist_quick(&Pl_objp->pos, &aip->goal_point);
	if ( dist < 70 ) {
		aip->submode = AIS_STRAFE_POSITION;
		aip->submode_start_time = Missiontime;
//		nprintf(("Alan","Ship %s entering AIS_STRAFE_POSITION\n", Ships[aip->shipnum].ship_name));
		return;
	}

	if (Missiontime - aip->submode_start_time > fl2f(0.70f)) {
		aip->submode = AIS_STRAFE_RETREAT1;
		aip->submode_start_time = Missiontime;

		if ( (Missiontime - aip->last_hit_time) < F1_0*5 ) {
			if (ai_maybe_fire_afterburner(Pl_objp, aip)) {
				afterburners_start(Pl_objp);
				aip->afterburner_stop_time = Missiontime + F1_0;
			}
		}
	}

	turn_towards_point(Pl_objp, &aip->prev_goal_point, NULL, 0.0f);
	accelerate_ship(aip, 1.0f);
}

// reposition self to begin another strafing run
void ai_big_strafe_position()
{
	ai_info	*aip;
	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];
	// TODO: loop around or something!!

	aip->submode = AIS_STRAFE_ATTACK;
	aip->submode_start_time = Missiontime;
//	nprintf(("Alan","Ship %s entering AIS_STRAFE_ATTACK\n", Ships[aip->shipnum].ship_name));
}

//	--------------------------------------------------------------------------
// #define	AIS_STRAFE_ATTACK		201	// fly towards target and attack
// #define	AIS_STRAFE_AVOID		202	// fly evasive vector to avoid incoming fire
// #define	AIS_STRAFE_RETREAT1	203	// fly away from attack point (directly)
// #define	AIS_STRAFE_RETREAT1	204	// fly away from attack point (on an avoid vector)
// #define	AIS_STRAFE_POSITION	205	// re-position to resume strafing attack
//
void ai_big_strafe()
{
	ai_info	*aip;

	aip = &Ai_info[Ships[Pl_objp->instance].ai_index];

	Assert(aip->mode == AIM_STRAFE);

/*
	if ( aip->goal_objnum != aip->target_objnum ) {
		Int3();	// what is going on here? - Get Alan
		aip->mode = AIM_NONE;
		return;
	}
*/

	// check if target is still a big ship... if not enter chase mode
	if ( !(Ship_info[Ships[En_objp->instance].ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) ) {
		ai_big_switch_to_chase_mode(aip);
		return;
	}
	
	switch (aip->submode) {
	case AIS_STRAFE_ATTACK:
		ai_big_strafe_attack();
		break;
	case AIS_STRAFE_AVOID:
		ai_big_strafe_avoid();
		break;
	case AIS_STRAFE_RETREAT1:
		ai_big_strafe_retreat1();
		break;
	case AIS_STRAFE_RETREAT2:
		ai_big_strafe_retreat2();
		break;
	case AIS_STRAFE_POSITION:
		ai_big_strafe_position();
		break;
	default:

		Int3();		//	Illegal submode for AIM_STRAFE
		break;
	}
}

// See if Pl_objp should enter strafe mode (This is called from maybe_evade_dumbfire_weapon(), and the
// weapon_objnum is for a weapon that is about to collide with Pl_objp
//
// Check if weapon_objnum was fired by Pl_objp's target, and whether Pl_objp's target is a big ship, if
// so, enter AIM_STRAFE
int ai_big_maybe_enter_strafe_mode(object *pl_objp, int weapon_objnum, int consider_target_only)
{
	ai_info		*aip;
	ship_info	*sip;
	object		*weapon_objp, *parent_objp;

	aip = &Ai_info[Ships[pl_objp->instance].ai_index];
	Assert(aip->mode != AIM_STRAFE);		// can't happen

	// if Pl_objp has no target, then we can't enter strafe mode
	if ( aip->target_objnum < 0 ) {
		return 0;
	}

	// if target is not a ship, stafe mode is not possible
	if ( Objects[aip->target_objnum].type != OBJ_SHIP ) {
		return 0;
	}

	sip = &Ship_info[Ships[Objects[aip->target_objnum].instance].ship_info_index];

	// if Pl_objp's target is not a big/capital ship, then cannot enter strafe mode
	// AL 12-31-97: Even though transports are considered big ships, don't enter strafe mode on them
	if ( !(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) || (sip->flags & SIF_TRANSPORT) ) {
		return 0;
	}

	//	If Pl_objp not a fighter or bomber, don't enter strafe mode. -- MK, 11/11/97.
	if ( !(Ship_info[Ships[pl_objp->instance].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER)) ) {
		return 0;
	}

	Assert(weapon_objnum >= 0 && weapon_objnum < MAX_OBJECTS);
	weapon_objp = &Objects[weapon_objnum];
	Assert(weapon_objp->type == OBJ_WEAPON);

	Assert(weapon_objp->parent >= 0 && weapon_objp->parent < MAX_OBJECTS);
	parent_objp = &Objects[weapon_objp->parent];
	if ( (parent_objp->signature != weapon_objp->parent_sig) || (parent_objp->type != OBJ_SHIP) ) {
		return 0;
	}

	// Maybe the ship which fired the weapon isn't the current target
	if ( OBJ_INDEX(parent_objp) != aip->target_objnum ) {

//JAS IMPOSSIBLE		if (1) { // consider_target_only ) {
//JAS IMPOSSIBLE			return 0;
//JAS IMPOSSIBLE		} else {
			// switch targets
			sip = &Ship_info[Ships[parent_objp->instance].ship_info_index];
			if ( !(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) || (sip->flags & SIF_TRANSPORT) ) {
				return 0;
			}
			set_target_objnum(aip, OBJ_INDEX(parent_objp));
//JAS IMPOSSIBLE		}
	}

	ai_big_strafe_maybe_attack_turret(pl_objp, weapon_objp);

	// if we've got this far, the weapon must have come from the player's target, and it is a 
	// big/capital ship... so enter strafe mode
	aip->previous_mode = aip->mode;
	aip->mode = AIM_STRAFE;
	aip->submode_parm0 = Missiontime;	// use parm0 as time strafe mode entered
	aip->submode = AIS_STRAFE_AVOID;
	aip->submode_start_time = Missiontime;
//	nprintf(("Alan","%s Accepted strafe mode\n", Ships[pl_objp->instance].ship_name));

	return 1;
}

// Consider attacking a turret, if a turret actually fired the weapon
// input:	ship_objp	=>	ship that will attack the turret
//				weapon_objp	=>	
void ai_big_strafe_maybe_attack_turret(object *ship_objp, object *weapon_objp)
{
	ai_info	*aip;
	object	*parent_objp;

	Assert(ship_objp->type == OBJ_SHIP);
	aip = &Ai_info[Ships[ship_objp->instance].ai_index];

	// Make decision to attack turret based on AI class.  The better AI ships will realize that
	// it is better to take out the turrets first on a big ship.
	if ( (frand()*100) > (aip->ai_courage-15) )
		return;

	// If ship is already attacking a subsystem, don't switch
	if ( aip->targeted_subsys != NULL ) {
		return;
	}

	// If the weapon is not from a turret, return
	if ( Weapons[weapon_objp->instance].turret_subsys == NULL ) {
		return;
	}

	Assert(weapon_objp->parent >= 0 && weapon_objp->parent < MAX_OBJECTS);
	parent_objp = &Objects[weapon_objp->parent];
	
	// UnknownPlayer : Decide whether or not this weapon was a beam, in which case it might be a good
	// idea to go after it even if its not on the current target. This is randomized so
	// the ai will not always go after different ships firing beams at them.
	// Approx 1/4 chance we'll go after the other ship's beam.

	bool attack_turret_on_different_ship = (The_mission.flags & MISSION_FLAG_USE_NEW_AI)
		&& (Weapon_info[weapon_objp->instance].wi_flags & WIF_BEAM) && (frand()*100 < 25.0f);

	// unless we're making an exception, we should only attack a turret if it sits on the current target
	if ( !attack_turret_on_different_ship )
	{
		if ( (parent_objp->signature != weapon_objp->parent_sig) || (parent_objp->type != OBJ_SHIP) ) {
			return;
		}
		
		if ( aip->target_objnum != OBJ_INDEX(parent_objp) ) {
			return;
		}
	}

	// attack the turret
	set_targeted_subsys(aip, Weapons[weapon_objp->instance].turret_subsys, OBJ_INDEX(parent_objp));
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/CollideDebrisShip.cpp $
 * $Revision: 2.5 $
 * $Date: 2004-07-12 16:32:59 $
 * $Author: Kazan $
 *
 * Routines to detect collisions and do physics, damage, etc for ships and debris
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/11/11 02:15:42  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.2  2003/04/29 01:03:22  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    9/14/99 2:59a Andsager
 * Limit amount of damage done by debris colliding with ship for SUPERCAP
 * 
 * 11    8/31/99 11:43p Andsager
 * No ship:debris damage to ship if debris' parent is ship itself
 * 
 * 10    8/10/99 5:49p Andsager
 * Fix bug - no damage collision against debris.
 * 
 * 9     8/09/99 4:45p Andsager
 * Add HUD "collision " waring when colliding with asteroids and debris.
 * 
 * 8     7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 7     6/14/99 3:21p Andsager
 * Allow collisions between ship and its debris.  Fix up collision pairs
 * when large ship is warping out.
 * 
 * 6     1/12/99 10:24a Andsager
 * Fix asteroid-ship collision bug, with negative collision
 * 
 * 5     10/23/98 1:11p Andsager
 * Make ship sparks emit correctly from rotating structures.
 * 
 * 4     10/20/98 1:39p Andsager
 * Make so sparks follow animated ship submodels.  Modify
 * ship_weapon_do_hit_stuff() and ship_apply_local_damage() to add
 * submodel_num.  Add submodel_num to multiplayer hit packet.
 * 
 * 3     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 27    4/24/98 5:35p Andsager
 * Fix sparks sometimes drawing not on model.  If ship is sphere in
 * collision, don't draw sparks.  Modify ship_apply_local_damage() to take
 * parameter no_spark.
 * 
 * 26    4/02/98 6:29p Lawrance
 * compile out asteroid references for demo
 * 
 * 25    4/02/98 5:10p Mike
 * Decrease damage yet more (by 3x) when an asteroid collides with a ship
 * during warp out.
 * 
 * 24    4/02/98 4:25p Andsager
 * fix bug calculating max_ship_impulse
 * 
 * 23    3/30/98 10:35a Andsager
 * Attempt to optimize ship:asteroid collisions.  Maybe return if time.
 * 
 * 22    3/25/98 2:37p Andsager
 * Modify maximum damage done by asteroids, depends on ship's max_vel, not
 * vel when warping out.
 * 
 * 21    3/25/98 2:15p Andsager
 * 
 * 20    3/25/98 1:36p Mike
 * Balancing of sm1-06a, Galatea mission.
 * 
 * 19    3/23/98 11:14a Andsager
 * Modified collide_asteroid_ship to calculate next possible collision
 * time based on ship afterburner speed.
 * 
 * 18    3/20/98 12:02a Mike
 * Cap damage done by an asteroid on a huge ship to 1/8 ship strength if a
 * large ship.  If <4000 hull, can do more than 1/8.
 * 
 * 17    3/05/98 3:12p Mike
 * Fix bugs in asteroid collisions.  Throw asteroids at Galataea (needs to
 * be made general).  Add skill level support for asteroid throwing.
 * 
 * 16    3/04/98 11:21p Mike
 * Less rotation on huge ships in death roll.
 * Less damage done to huge ships when hit an asteroid at high speed.
 * 
 * 15    3/02/98 2:58p Mike
 * Make "asteroids" in debug console turn asteroids on/off.
 * 
 * 14    2/19/98 12:46a Lawrance
 * Further work on asteroids.
 * 
 * 13    2/07/98 2:14p Mike
 * Improve asteroid splitting.  Add ship:asteroid collisions.  Timestamp
 * ship:debris collisions.
 * 
 * 12    2/06/98 7:29p John
 * Added code to cull out some future asteroid-ship collisions.
 * 
 * 11    2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 10    2/04/98 8:57p Lawrance
 * remove unused variable
 * 
 * 9     2/04/98 6:08p Lawrance
 * Add a light collision sound, overlay a shield collide sound if
 * applicable.
 * 
 * 8     1/05/98 9:07p Andsager
 * Changed ship_shipor_debris_hit_info struct to more meaninful names
 * 
 * 7     1/02/98 9:08a Andsager
 * Changed ship:ship and ship:debris collision detection to ignore shields
 * and collide only against hull.  Also, significantly reduced radius of
 * sphere.
 * 
 * 6     12/22/97 9:56p Andsager
 * Implement ship:debris collisions.  Generalize and move
 * ship_ship_or_debris_hit struct from CollideShipShip to ObjCollide.h
 * 
 * 5     11/03/97 11:08p Lawrance
 * play correct collision sounds.
 * 
 * 4     10/27/97 8:35a John
 * code for new player warpout sequence
 * 
 * 3     10/01/97 5:55p Lawrance
 * change call to snd_play_3d() to allow for arbitrary listening position
 * 
 * 2     9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 1     9/17/97 2:14p John
 * Initial revision
 *
 * $NoKeywords: $
 */


#include "object/objcollide.h"
#include "ship/ship.h"
#include "debris/debris.h"
#include "playerman/player.h"
#include "ship/shiphit.h"
#include "io/timer.h"
#include "asteroid/asteroid.h"
#include "hud/hud.h"
#include "object/object.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

void calculate_ship_ship_collision_physics(collision_info_struct *ship_ship_hit_info);

/*
extern int Framecount;
int Debris_ship_count = 0;
*/

// Checks debris-ship collisions.  pair->a is debris and pair->b is ship.
// Returns 1 if all future collisions between these can be ignored
int collide_debris_ship( obj_pair * pair )
{
	float dist;
	object *pdebris = pair->a;
	object *pship = pair->b;

		// Don't check collisions for warping out player
	if ( Player->control_mode != PCM_NORMAL )	{
		if ( pship == Player_obj )
			return 0;
	}

	Assert( pdebris->type == OBJ_DEBRIS );
	Assert( pship->type == OBJ_SHIP );

/*	Debris_ship_count++;
	if (Debris_ship_count % 100 == 0)
		nprintf(("AI", "Done %i debris:ship checks in %i frames = %.2f checks/frame\n", Debris_ship_count, Framecount, (float) Debris_ship_count/Framecount));
*/
	dist = vm_vec_dist( &pdebris->pos, &pship->pos );
	if ( dist < pdebris->radius + pship->radius )	{
		int hit;
		vector	hitpos;
		// create and initialize ship_ship_hit_info struct
		collision_info_struct debris_hit_info;
		memset( &debris_hit_info, -1, sizeof(collision_info_struct) );

		if ( pdebris->phys_info.mass > pship->phys_info.mass ) {
			debris_hit_info.heavy = pdebris;
			debris_hit_info.light = pship;
		} else {
			debris_hit_info.heavy = pship;
			debris_hit_info.light = pdebris;
		}

		hit = debris_check_collision(pdebris, pship, &hitpos, &debris_hit_info );
		if ( hit ) {
			float		ship_damage;	
			float		debris_damage;

			// do collision physics
			calculate_ship_ship_collision_physics( &debris_hit_info );

			if ( debris_hit_info.impulse < 0.5f )
				return 0;

			// calculate ship damage
			ship_damage = 0.005f * debris_hit_info.impulse;	//	Cut collision-based damage in half.
			//	Decrease heavy damage by 2x.
			if (ship_damage > 5.0f)
				ship_damage = 5.0f + (ship_damage - 5.0f)/2.0f;

			// calculate debris damage and set debris damage to greater or debris and ship
			// debris damage is needed since we can really whack some small debris with afterburner and not do
			// significant damage to ship but the debris goes off faster than afterburner speed.
			debris_damage = debris_hit_info.impulse/pdebris->phys_info.mass;	// ie, delta velocity of debris
			debris_damage = (debris_damage > ship_damage) ? debris_damage : ship_damage;

			// supercaps cap damage at 10-20% max hull ship damage
			if (Ship_info[Ships[pship->instance].ship_info_index].flags & SIF_SUPERCAP) {
				float cap_percent_damage = frand_range(0.1f, 0.2f);
				ship_damage = min(ship_damage, cap_percent_damage * Ships[pship->instance].ship_initial_hull_strength);
			}

			// apply damage to debris
			debris_hit( pdebris, pship, &hitpos, debris_damage);		// speed => damage
			int quadrant_num, apply_ship_damage;

			// apply damage to ship unless 1) debris is from ship
			// apply_ship_damage = !((pship->signature == pdebris->parent_sig) && ship_is_beginning_warpout_speedup(pship));
			apply_ship_damage = !(pship->signature == pdebris->parent_sig);

			if ( debris_hit_info.heavy == pship ) {
				quadrant_num = get_ship_quadrant_from_global(&hitpos, pship);
				if ((pship->flags & OF_NO_SHIELDS) || !ship_is_shield_up(pship, quadrant_num) ) {
					quadrant_num = -1;
				}
				if (apply_ship_damage) {
					ship_apply_local_damage(debris_hit_info.heavy, debris_hit_info.light, &hitpos, ship_damage, quadrant_num, CREATE_SPARKS, debris_hit_info.submodel_num);
				}
			} else {
				// don't draw sparks using sphere hit position
				if (apply_ship_damage) {
					ship_apply_local_damage(debris_hit_info.light, debris_hit_info.heavy, &hitpos, ship_damage, MISS_SHIELDS, NO_SPARKS);
				}
			}

			// maybe print Collision on HUD
			if ( pship == Player_obj ) {					
				hud_start_text_flash(XSTR("Collision", 1431), 2000);
			}

			collide_ship_ship_do_sound(&hitpos, pship, pdebris, pship==Player_obj);

			return 0;
		}
	} else {	//	Bounding spheres don't intersect, set timestamp for next collision check.
		float	ship_max_speed, debris_speed;
		float	time;
		ship *shipp;

		shipp = &Ships[pship->instance];

		if (ship_is_beginning_warpout_speedup(pship)) {
			ship_max_speed = max(ship_get_max_speed(shipp), ship_get_warp_speed(pship));
		} else {
			ship_max_speed = ship_get_max_speed(shipp);
		}
		ship_max_speed = max(ship_max_speed, 10.0f);
		ship_max_speed = max(ship_max_speed, pship->phys_info.vel.xyz.z);

		debris_speed = pdebris->phys_info.speed;

		time = 1000.0f * (dist - pship->radius - pdebris->radius - 10.0f) / (ship_max_speed + debris_speed);		// 10.0f is a safety factor
		time -= 200.0f;		// allow one frame slow frame at ~5 fps

		if (time > 100) {
			//nprintf(("AI", "Ship %s debris #%i delay time = %.1f seconds\n", Ships[pship->instance].ship_name, pdebris-Objects, time/1000.0f));
			pair->next_check_time = timestamp( fl2i(time) );
		} else {
			pair->next_check_time = timestamp(0);	// check next time
		}
	}

	return 0;
}

// Checks asteroid-ship collisions.  pair->a is asteroid and pair->b is ship.
// Returns 1 if all future collisions between these can be ignored
int collide_asteroid_ship( obj_pair * pair )
{
#ifndef FS2_DEMO

	if (!Asteroids_enabled)
		return 0;

	float		dist;
	object	*pasteroid = pair->a;
	object	*pship = pair->b;

		// Don't check collisions for warping out player
	if ( Player->control_mode != PCM_NORMAL )	{
		if ( pship == Player_obj ) return 0;
	}

	if (pasteroid->hull_strength < 0.0f)
		return 0;

	Assert( pasteroid->type == OBJ_ASTEROID );
	Assert( pship->type == OBJ_SHIP );

	dist = vm_vec_dist( &pasteroid->pos, &pship->pos );

	if ( dist < pasteroid->radius + pship->radius )	{
		int hit;
		vector	hitpos;
		// create and initialize ship_ship_hit_info struct
		collision_info_struct asteroid_hit_info;
		memset( &asteroid_hit_info, -1, sizeof(collision_info_struct) );

		if ( pasteroid->phys_info.mass > pship->phys_info.mass ) {
			asteroid_hit_info.heavy = pasteroid;
			asteroid_hit_info.light = pship;
		} else {
			asteroid_hit_info.heavy = pship;
			asteroid_hit_info.light = pasteroid;
		}

		hit = asteroid_check_collision(pasteroid, pship, &hitpos, &asteroid_hit_info );
		if ( hit ) {
			float		ship_damage;	
			float		asteroid_damage;

			vector asteroid_vel = pasteroid->phys_info.vel;

			// do collision physics
			calculate_ship_ship_collision_physics( &asteroid_hit_info );

			if ( asteroid_hit_info.impulse < 0.5f )
				return 0;

			// limit damage from impulse by making max impulse (for damage) 2*m*v_max_relative
			float max_ship_impulse = (2.0f*pship->phys_info.max_vel.xyz.z+vm_vec_mag_quick(&asteroid_vel)) * 
				(pship->phys_info.mass*pasteroid->phys_info.mass) / (pship->phys_info.mass + pasteroid->phys_info.mass);

			if (asteroid_hit_info.impulse > max_ship_impulse) {
				ship_damage = 0.001f * max_ship_impulse;
			} else {
				ship_damage = 0.001f * asteroid_hit_info.impulse;	//	Cut collision-based damage in half.
			}

			//	Decrease heavy damage by 2x.
			if (ship_damage > 5.0f)
				ship_damage = 5.0f + (ship_damage - 5.0f)/2.0f;

			if ((ship_damage > 500.0f) && (ship_damage > Ships[pship->instance].ship_initial_hull_strength/8.0f)) {
				ship_damage = Ships[pship->instance].ship_initial_hull_strength/8.0f;
				nprintf(("AI", "Pinning damage to %s from asteroid at %7.3f (%7.3f percent)\n", Ships[pship->instance].ship_name, ship_damage, 100.0f * ship_damage/Ships[pship->instance].ship_initial_hull_strength));
			}

			//	Decrease damage during warp out because it's annoying when your escoree dies during warp out.
			if (Ai_info[Ships[pship->instance].ai_index].mode == AIM_WARP_OUT)
				ship_damage /= 3.0f;

			//nprintf(("AI", "Asteroid damage on %s = %7.3f (%6.2f percent)\n", Ships[pship->instance].ship_name, ship_damage, 100.0f * ship_damage/Ships[pship->instance].ship_initial_hull_strength));

			// calculate asteroid damage and set asteroid damage to greater or asteroid and ship
			// asteroid damage is needed since we can really whack some small asteroid with afterburner and not do
			// significant damage to ship but the asteroid goes off faster than afterburner speed.
			asteroid_damage = asteroid_hit_info.impulse/pasteroid->phys_info.mass;	// ie, delta velocity of asteroid
			asteroid_damage = (asteroid_damage > ship_damage) ? asteroid_damage : ship_damage;

			// apply damage to asteroid
			asteroid_hit( pasteroid, pship, &hitpos, asteroid_damage);		// speed => damage

			//extern fix Missiontime;

			int quadrant_num;
			if ( asteroid_hit_info.heavy == pship ) {
				quadrant_num = get_ship_quadrant_from_global(&hitpos, pship);
				if ((pship->flags & OF_NO_SHIELDS) || !ship_is_shield_up(pship, quadrant_num) ) {
					quadrant_num = -1;
				}
				ship_apply_local_damage(asteroid_hit_info.heavy, asteroid_hit_info.light, &hitpos, ship_damage, quadrant_num, CREATE_SPARKS, asteroid_hit_info.submodel_num);
				//if (asteroid_hit_info.heavy->type == OBJ_SHIP) {
				//	nprintf(("AI", "Time = %7.3f, asteroid #%i applying %7.3f damage to ship %s\n", f2fl(Missiontime), pasteroid-Objects, ship_damage, Ships[asteroid_hit_info.heavy->instance].ship_name));
				//}
			} else {
				// dont draw sparks (using sphere hitpos)
				ship_apply_local_damage(asteroid_hit_info.light, asteroid_hit_info.heavy, &hitpos, ship_damage, MISS_SHIELDS, NO_SPARKS);
				//if (asteroid_hit_info.light->type == OBJ_SHIP) {
				//	nprintf(("AI", "Time = %7.3f, asteroid #%i applying %7.3f damage to ship %s\n", f2fl(Missiontime), pasteroid-Objects, ship_damage, Ships[asteroid_hit_info.light->instance].ship_name));
				//}
			}

			// maybe print Collision on HUD
			if ( pship == Player_obj ) {					
				hud_start_text_flash(XSTR("Collision", 1431), 2000);
			}

			collide_ship_ship_do_sound(&hitpos, pship, pasteroid, pship==Player_obj);

			return 0;
		}

		return 0;
	} else {
		// estimate earliest time at which pair can hit
		float asteroid_max_speed, ship_max_speed, time;
		ship *shipp = &Ships[pship->instance];

		asteroid_max_speed = vm_vec_mag(&pasteroid->phys_info.vel);		// Asteroid... vel gets reset, not max vel.z
		asteroid_max_speed = max(asteroid_max_speed, 10.0f);

		if (ship_is_beginning_warpout_speedup(pship)) {
			ship_max_speed = max(ship_get_max_speed(shipp), ship_get_warp_speed(pship));
		} else {
			ship_max_speed = ship_get_max_speed(shipp);
		}
		ship_max_speed = max(ship_max_speed, 10.0f);
		ship_max_speed = max(ship_max_speed, pship->phys_info.vel.xyz.z);


		time = 1000.0f * (dist - pship->radius - pasteroid->radius - 10.0f) / (asteroid_max_speed + ship_max_speed);		// 10.0f is a safety factor
		time -= 200.0f;		// allow one frame slow frame at ~5 fps

		if (time > 100) {
			pair->next_check_time = timestamp( fl2i(time) );
		} else {
			pair->next_check_time = timestamp(0);	// check next time
		}
		return 0;
	}
#else
	return 0;	// no asteroids in demo version
#endif
}

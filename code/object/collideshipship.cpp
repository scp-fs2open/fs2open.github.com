/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/CollideShipShip.cpp $
 * $Revision: 2.26 $
 * $Date: 2007-11-23 23:49:33 $
 * $Author: wmcoolmon $
 *
 * Routines to detect collisions and do physics, damage, etc for ships and ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.25  2007/02/20 04:20:27  Goober5000
 * the great big duplicate model removal commit
 *
 * Revision 2.24  2007/02/19 07:24:51  wmcoolmon
 * WMCoolmon experiences a duh moment. Move scripting collision variable declarations in front of overrides, to give
 * them access to these (somewhat useful) variables
 *
 * Revision 2.23  2007/02/11 21:26:35  Goober5000
 * massive shield infrastructure commit
 *
 * Revision 2.22  2007/02/11 06:19:05  Goober5000
 * invert the do-collision flag into a don't-do-collision flag, plus fixed a wee lab bug
 *
 * Revision 2.21  2007/01/08 00:50:58  Goober5000
 * remove WMC's limbo code, per our discussion a few months ago
 * this will later be handled by copying ship stats using sexps or scripts
 *
 * Revision 2.20  2006/12/28 00:59:39  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 2.19  2006/07/09 01:55:41  Goober5000
 * consolidate the "for reals" crap into a proper ship flag; also move the limbo flags over to SF2_*; etc.
 * this should fix Mantis #977
 * --Goober5000
 *
 * Revision 2.18  2006/06/07 04:42:22  wmcoolmon
 * Limbo flag support; further scripting 3.6.9 update
 *
 * Revision 2.17  2005/12/29 08:08:39  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.16  2005/10/09 09:13:29  wmcoolmon
 * Added warpin/warpout speed override values to ships.tbl
 *
 * Revision 2.15  2005/09/25 08:25:14  Goober5000
 * Okay, everything should now work again. :p Still have to do a little more with the asteroids.
 * --Goober5000
 *
 * Revision 2.14  2005/07/12 21:10:57  Goober5000
 * fixed "warpout sequence aborted" bug
 * --Goober5000
 *
 * Revision 2.13  2005/04/05 05:53:21  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.12  2005/03/27 12:28:32  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.11  2005/03/10 08:00:11  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.10  2005/01/26 05:42:37  Goober5000
 * fixed a nitpicky thing
 * --Goober5000
 *
 * Revision 2.9  2005/01/11 21:38:49  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.8  2004/07/26 20:47:45  Kazan
 * remove MCD complete
 *
 * Revision 2.7  2004/07/12 16:32:59  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.6  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.5  2003/04/29 01:03:22  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.4  2003/01/24 03:48:11  Goober5000
 * aw, nuts - fixed a dumb bug with my new don't-collide-invisible code :p
 * --Goober5000
 *
 * Revision 2.3  2003/01/18 09:25:42  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.2  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/26 16:17:46  penguin
 * renamed 'big' and 'small' (conflict w/ MS include file)
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
 * 31    9/01/99 5:40p Andsager
 * Collision resolution between small and CAP during warp
 * 
 * 30    8/24/99 8:55p Dave
 * Make sure nondimming pixels work properly in tech menu.
 * 
 * 29    7/29/99 12:11a Andsager
 * comment fix
 * 
 * 28    7/28/99 11:23p Andsager
 * Try a different strategy to resolve collisions between ships on same
 * team.
 * 
 * 27    7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 26    7/15/99 5:41p Andsager
 * Clean up demo build
 * 
 * 25    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 24    7/12/99 11:49a Andsager
 * Really fix collision warp-in bug.
 * 
 * 23    7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 22    7/08/99 5:49p Andsager
 * Fixed bug colliding with just warped in Cap ship
 * 
 * 21    6/14/99 3:21p Andsager
 * Allow collisions between ship and its debris.  Fix up collision pairs
 * when large ship is warping out.
 * 
 * 20    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 19    4/20/99 3:45p Andsager
 * Modify ship_apply_local_damage to take a collision normal
 * 
 * 18    4/19/99 12:21p Johnson
 * Allow ships with invisible polygons which do not collide
 * 
 * 17    3/20/99 2:54p Andsager
 * Fix collision for cap ships warping in - speed is much greater than
 * expected.
 * 
 * 16    2/05/99 11:07a Andsager
 * Make cap ships not get shoved around with asteroid collisions
 * 
 * 15    2/02/99 1:18p Andsager
 * Modify asteroid/cruiser collisions so cruisers don't get bashed so
 * much.
 * 
 * 14    1/12/99 5:45p Dave
 * Moved weapon pipeline in multiplayer to almost exclusively client side.
 * Very good results. Bandwidth goes down, playability goes up for crappy
 * connections. Fixed object update problem for ship subsystems.
 * 
 * 13    1/11/99 12:42p Andsager
 * Add live debris - debris which is created from a destroyed subsystem,
 * when the ship is still alive
 * 
 * 12    12/03/98 3:14p Andsager
 * Check in code that checks rotating submodel actually has ship subsystem
 * 
 * 11    11/20/98 2:22p Andsager
 * Change collision separation h2l_vec
 * 
 * 10    11/19/98 11:47p Andsager
 * Fix possible divide by zero bug.
 * 
 * 9     11/19/98 11:08p Andsager
 * Check in of physics and collision detection of rotating submodels
 * 
 * 8     11/13/98 5:06p Johnson
 * Fix Kulas collision bug
 * 
 * 7     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 6     10/23/98 1:11p Andsager
 * Make ship sparks emit correctly from rotating structures.
 * 
 * 5     10/20/98 1:39p Andsager
 * Make so sparks follow animated ship submodels.  Modify
 * ship_weapon_do_hit_stuff() and ship_apply_local_damage() to add
 * submodel_num.  Add submodel_num to multiplayer hit packet.
 * 
 * 4     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 105   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 104   5/24/98 11:36p Mike
 * Comment out no-optimize pragmas.
 * 
 * 103   5/24/98 10:50p Mike
 * Fix problem with ships with propagating explosions not being able to
 * kamikaze.
 * 
 * 102   5/21/98 3:48p Lawrance
 * prevent player from entering friendly ship docking bays
 * 
 * 101   5/19/98 2:19p Mike
 * Don't do collision detection between small ship emerging or departing
 * and its parent.
 * 
 * 100   5/18/98 4:53p Hoffoss
 * Some force feedback tweaks and pilot initializations there should have
 * been happening, but weren't, and not are!
 * 
 * 99    5/13/98 11:34p Mike
 * Model caching system.
 * 
 * 98    5/10/98 11:11p Lawrance
 * Allow ships to collide if in second stage of arrival
 * 
 * 97    5/08/98 5:25p Lawrance
 * Don't allow collision sounds too play over each so much
 * 
 * 96    5/08/98 3:51p Allender
 * temporary fix for support ships on clients in multiplayer
 * 
 * 95    5/08/98 11:22a Allender
 * fix ingame join trouble.  Small messaging fix.  Enable collisions for
 * friendlies again
 * 
 * 94    5/07/98 12:24a Hoffoss
 * Finished up sidewinder force feedback support.
 * 
 * 93    5/03/98 5:40p Mike
 * Debug info for trapping player collisions.
 * 
 * 92    4/28/98 2:28p Allender
 * temporarily put back in collision out for multiplayers for ships on the
 * same team since that broke rearm/repair
 * 
 * 91    4/28/98 1:00a Andsager
 * Add collision sanity check
 * 
 * 90    4/28/98 12:23a Chad
 * removed call which prevented same team coliisions from happening in
 * multiplayer
 * 
 * 89    4/24/98 5:35p Andsager
 * Fix sparks sometimes drawing not on model.  If ship is sphere in
 * collision, don't draw sparks.  Modify ship_apply_local_damage() to take
 * parameter no_spark.
 * 
 * 88    4/23/98 4:42p Mike
 * Support invisible polygons that only enemy ships bump into.
 * 
 * 87    4/20/98 12:36a Mike
 * Make team vs. team work when player is hostile.  Several targeting
 * problems.
 * 
 * 86    4/12/98 2:02p Mike
 * Make small ships avoid big ships.
 * Turn on Collide_friendly flag.
 * 
 * 85    4/08/98 4:01p Andsager
 * Removed assert in calculate_ship_ship_collisions_physics()
 * 
 * 84    4/06/98 1:39a Mike
 * NDEBUG out some debugt code.
 * Make ships find a new target if their target is on the same team.
 * 
 * 83    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 82    3/25/98 2:43p Andsager
 * comment out asserts
 * 
 * 81    3/25/98 1:19p Mike
 * Comment out unimportant assert.
 * 
 * 80    3/25/98 10:43a Andsager
 * Hack for ship_asteroid collisions when erroneous relative_vel >150
 * 
 * 79    3/25/98 12:05a Mike
 * Comment out line to make sm1-06a playable.   Reported to DaveA.
 * 
 * 78    3/23/98 9:20a Andsager
 * Remove all velocity updates in object code.
 * 
 * 77    3/17/98 12:49a Mike
 * Improved kamikaze behavior.
 * 
 * 76    3/16/98 12:02a Mike
 * Add support for kamikaze mode.  Add AIF_NO_DYNAMIC which means
 * relentlessly pursue current goal.
 * 
 * 75    3/13/98 12:57p Mike
 * Remove an Assert that was easy to trip with time compressed 8x.
 * 
 * 74    3/09/98 12:58a Andsager
 * Don't check asteroids very large displacements in collisions, since
 * they may wrap.
 * 
 * 73    3/09/98 12:16a Andsager
 * 
 * 72    2/22/98 2:48p John
 * More String Externalization Classification
 * 
 * 71    2/20/98 8:32p Lawrance
 * Add radius parm to sound_play_3d()
 * 
 * 70    2/12/98 2:16p Andsager
 * Better ship:ship collision pair next check time estimate
 * 
 * 69    2/09/98 1:45p Andsager
 * Fix bug in finding earliest possilble ship:ship collision.  Used
 * max_vel, not afterburner_max_vel.
 * 
 * 68    2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 67    2/04/98 6:08p Lawrance
 * Add a light collision sound, overlay a shield collide sound if
 * applicable.
 * 
 * 66    2/02/98 4:36p Mike
 * Prevent damage from occurring between two ships during very last frame
 * of warpout if docked and on opposite sides.
 * 
 * 65    1/28/98 2:15p Mike
 * Make collision damage affect shield, not just hull.
 * 
 * 64    1/28/98 11:06a Andsager
 * Remove some collision pairs.  Add debug code for displaying collision
 * pair info.  
 * 
 * 63    1/23/98 5:08p Andsager
 * Collision from rotation is turned on for all ship:ship colliisons.
 * 
 * 62    1/20/98 9:47a Mike
 * Suppress optimized compiler warnings.
 * Some secondary weapon work.
 * 
 * 61    1/19/98 11:56a Sandeep
 * DA:  remove warning in calculate ship_ship_physics from ship_debris
 * collision when debris is spawned next to chasing ship.
 * 
 * 60    1/14/98 2:30p Andsager
 * Change warning for bad relative velocity
 * 
 * 59    1/12/98 9:26p Andsager
 * Implement collisions from rotation.
 * 
 * 58    1/09/98 9:29a Mike
 * Enable docked ships to warp out.  Make damage done to a ship not
 * proportional to its own mass.
 * 
 * 57    1/08/98 12:12a Mike
 * Make ships turn before warping out, if necessary, to avoid a collision.
 * Warn player if his warpout will collide.  Abort if in stage1.
 * 
 * 56    1/05/98 9:08p Andsager
 * Changed ship_shipor_debris_hit_info struct to more meaninful names.
 * Begin implementation of collision from rotation.
 * 
 * 55    1/02/98 9:08a Andsager
 * Changed ship:ship and ship:debris collision detection to ignore shields
 * and collide only against hull.  Also, significantly reduced radius of
 * sphere.
 * 
 * 54    12/23/97 5:34p Andsager
 * Fixed bug colliding against edge of ships without shields.
 * 
 * 53    12/22/97 9:56p Andsager
 * Implement ship:debris collisions.  Generalize and move
 * ship_ship_or_debris_hit struct from CollideShipShip to ObjCollide.h
 * 
 * 52    12/17/97 9:39p Lawrance
 * Always play collide sound when player collides with another ship.
 * 
 * 51    12/17/97 3:55p Andsager
 * Added separation velocity in ship:ship collision when both ships on
 * same team.
 * 
 * 50    12/16/97 5:24p Andsager
 * Modify collision detection criterion.  Somewhat of a hack, but it keeps
 * ships from getting stuci on each other.  Comment out debug info.
 * 
 * 49    12/08/97 6:23p Lawrance
 * fix collision sounds (broken since hit pos was changed to local coords)
 * 
 * 48    12/04/97 5:34p Lawrance
 * let player collide with friendly ships (no damage though) by default
 * 
 * 47    12/04/97 4:05p Allender
 * comment out hud printf for ship ship collisions
 * 
 * 46    12/03/97 5:44p Andsager
 * Implement relative velocity collisions in the reference frame of the
 * heavier object.
 * 
 * 45    12/03/97 12:04p Hoffoss
 * Made changes so the 8 %'s aren't needed anymore.  Can just use 2 again
 * now.
 * 
 * 44    12/03/97 11:35a Hoffoss
 * Made changes to HUD messages send throughout the game.
 * 
 * 43    11/28/97 3:51p Mike
 * Get blasted % symbol to display through HUD_printf.  Had to enter
 * %%%%%%%% (yes, that's 8x %)
 * 
 * 42    11/26/97 3:25p Mike
 * Decrease large quantities of damage.  If > 5.0f, cut out half of amount
 * over 5.0f.
 * 
 * 41    11/16/97 8:45p Mike
 * Add SM_ATTACK_FOREVER submode (of AIM_CHASE) and ships better dealing
 * with their engines being blown out.
 * 
 * 40    11/14/97 9:27a Andsager
 * Changed debug print statements
 * 
 * 39    11/13/97 6:12p Lawrance
 * uncomment code that was commented out for build reasons
 * 
 * 38    11/13/97 6:11p Lawrance
 * call hud_start_collision_flash() when player ship hits another ship
 * 
 * 37    11/13/97 4:59p Mike
 * Add new chase submode: SM_FLY_AWAY.  Deals with a ship colliding with
 * its target.  Ships were getting hung up on each other because the avoid
 * code was used to deal with collisions.  It was very bad.
 * 
 * 36    11/12/97 11:53p Mike
 * Fix code that shows damage taken due to collision to only work for
 * player.
 * 
 * 35    11/12/97 12:14p Mike
 * Cut ship:ship collision damage by half again and put in a HUD message
 * for large damage.
 * 
 * 34    11/12/97 10:03a Mike
 * Cut damage done due to ship:ship collisions by half.
 * 
 * 33    11/10/97 10:50p Mike
 * Fix bug preventing ships in sm1-03a from warping out together as a
 * docked pair.  Only worked for support ships and cargo, not a pair of
 * transports.
 * 
 * 32    11/09/97 11:24p Andsager
 * Set small bounce in ship-ship collision.  coeffic restitution 0.2
 * 
 * 31    11/09/97 4:39p Lawrance
 * make 'Collide_friendly' make friendly collisions behave the same way as
 * hostile collisions
 * 
 * 30    11/07/97 4:36p Mike
 * Change how ships determine they're under attack by dumbfire weapons.
 * 
 * 29    11/06/97 12:27a Mike
 * Better avoid behavior.
 * Modify ai_turn_towards_vector() to take a flag parameter.
 * 
 * 28    11/05/97 10:32p Mike
 * Convert Assert() to nprintf when point of collisions is farther apart
 * than sum of object radii.
 * 
 * 27    11/05/97 9:28p Mike
 * Add ships_are_docking() to allow ships of different teams to dock.
 * 
 * 26    11/05/97 5:50p Andsager
 * Added hit_time to ship_ship_hit_info to prevent hit_time from getting
 * overwritten.
 * 
 * 25    11/03/97 11:21p Andsager
 * Fixed bug getting shield quad.  Reduced damage in ship-ship.  Collision
 * normal from sphere center to hit_pos
 * 
 * 24    11/03/97 11:08p Lawrance
 * play correct collision sounds.
 * 
 * 23    11/03/97 2:07p Lawrance
 * add ship-to-ship collision sound
 * 
 * 22    11/02/97 10:54p Lawrance
 * add Collide_friendly, which is changed through debug console to
 * enable/disable friend-friend collisions
 * 
 * 21    11/01/97 3:58p Mike
 * Zero damage caused by ship:ship collisions until it gets balanced.
 * 
 * 20    10/29/97 5:19p Dave
 * More debugging of server transfer. Put in debrief/brief 
 * transition for multiplayer (w/standalone)
 * 
 * 19    10/29/97 4:56p Andsager
 * Fixed bugs in collision physics involving normals.  Collided objects
 * now back up in the direction they came in.
 * 
 * 18    10/28/97 4:57p John
 * Put Andsager's new sphereline collide code officially into the code
 * base and did a little restructuring.  Fixed a few little bugs with it
 * and added some simple bounding box elimination and did some timings.
 * 
 * 
 * 17    10/27/97 6:12p Dave
 * Changed host/server transfer around. Added some multiplayer data to
 * state save/restore. Made multiplayer quitting more intelligent.
 * 
 * 16    10/27/97 8:35a John
 * code for new player warpout sequence
 * 
 * 15    10/25/97 10:12a Andsager
 * Cleaned up ship_ship_check_collision. Moved SHIP_SPHERE_CHECK to
 * objCollide.h.  Added some debug code for shield/hull collisions
 * 
 * 14    10/22/97 10:29p Andsager
 * modify ship_ship_check_collision to allow sphere-polygon collisions
 * 
 * 13    10/19/97 11:45p Mike
 * Hacked in damage due to gravity well of a planet.
 * 
 * 12    10/19/97 9:41p Andsager
 * undefine SPHERE_POLY_CHECK
 * 
 * 11    10/19/97 9:34p Andsager
 * Changed model_collide to take 2nd parameter radius with (default = 0)
 * 
 * 10    10/17/97 1:32a Andsager
 * add sphere-polygon collision detection
 * 
 * 9     10/01/97 5:55p Lawrance
 * change call to snd_play_3d() to allow for arbitrary listening position
 * 
 * 8     9/30/97 5:06p Andsager
 * rename vm_project_point_onto_surface() -> vm_project_name_onto_plane()
 * 
 * 7     9/28/97 2:19p Andsager
 * fixed bug in getting shield point in ray model collisions
 * 
 * 6     9/25/97 2:54p Andsager
 * added small bounce to collisions
 * 
 * 5     9/19/97 5:00p Andsager
 * modify collisions so that damage is first applied to shield and then to
 * hull
 * 
 * 4     9/18/97 4:08p John
 * Cleaned up & restructured ship damage stuff.
 * 
 * 3     9/18/97 3:58p Andsager
 * fix bugs in sphere_sphere collision (sets r and hit_pos)
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
#include "object/object.h"
#include "ship/ship.h"
#include "freespace2/freespace.h"
#include "ship/shiphit.h"
#include "gamesnd/gamesnd.h"
#include "render/3d.h"			// needed for View_position, which is used when playing 3d sound
#include "gamesequence/gamesequence.h"
#include "hud/hudshield.h"
#include "hud/hudmessage.h"
#include "io/joy_ff.h"
#include "io/timer.h"
#include "asteroid/asteroid.h"
#include "playerman/player.h"
#include "object/objectdock.h"
#include "parse/lua.h"
#include "parse/scripting.h"



//#pragma optimize("", off)
//#pragma auto_inline(off)

#define COLLISION_FRICTION_FACTOR	0.0
#define COLLISION_ROTATION_FACTOR	0.2
#define SEP_VEL	5.0f		// separation velocity between two ships that collide on same team.

#define COLLIDE_DEBUG
#undef  COLLIDE_DEBUG

// GENERAL COLLISIONS FUNCTIONS
// calculates the inverse moment of inertia matrix in world coordinates
void get_I_inv (matrix* I_inv, matrix* I_inv_body, matrix* orient);

// calculate the physics of extended two body collisions
void calculate_ship_ship_collision_physics(collision_info_struct *ship_ship_hit_info);

int ship_hit_shield(object *obj, mc_info *mc, collision_info_struct *sshs);
void collect_ship_ship_physics_info(object *heavy, object *light, mc_info *mc_info, collision_info_struct *ship_ship_hit_info);

#ifndef NDEBUG
static int Collide_friendly = 1;
DCF_BOOL( collide_friendly, Collide_friendly )
#endif

static int Player_collide_sound, AI_collide_sound;
static int Player_collide_shield_sound, AI_collide_shield_sound;

//	Return true if two ships are docking.
int ships_are_docking(object *objp1, object *objp2)
{
	ai_info	*aip1, *aip2;
	ship		*shipp1, *shipp2;

	shipp1 = &Ships[objp1->instance];
	shipp2 = &Ships[objp2->instance];

	aip1 = &Ai_info[shipp1->ai_index];
	aip2 = &Ai_info[shipp2->ai_index];

	// for multiplayer clients -- disable the collision stuff for support ships.
	/*
	if ( MULTIPLAYER_CLIENT ) {
		if ( (Ship_info[shipp1->ship_info_index].flags & SIF_SUPPORT) || (Ship_info[shipp2->ship_info_index].flags & SIF_SUPPORT) ) {
			return 1;
		}
	}
	*/

	if (dock_check_find_direct_docked_object(objp1, objp2)) {
		return 1;
	}

	if (aip1->mode == AIM_DOCK) {
		if (aip1->goal_objnum == OBJ_INDEX(objp2)){
			return 1;
		}
	} else if (aip2->mode == AIM_DOCK) {
		if (aip2->goal_objnum == OBJ_INDEX(objp1)){
			return 1;
		}
	}

	return 0;

}

//	If light_obj emerging from or departing to dock bay in heavy_obj, no collision detection.
int bay_emerge_or_depart(object *heavy_objp, object *light_objp)
{
	if (light_objp->type != OBJ_SHIP)
		return 0;

	ai_info	*aip = &Ai_info[Ships[light_objp->instance].ai_index];

	if ((aip->mode == AIM_BAY_EMERGE) || (aip->mode == AIM_BAY_DEPART)) {
		if (aip->goal_objnum == OBJ_INDEX(heavy_objp))
			return 1;
	}

	return 0;
}

int ship_ship_check_collision(collision_info_struct *ship_ship_hit_info, vec3d *hitpos)
{
	object *heavy_obj	= ship_ship_hit_info->heavy;
	object *light_obj = ship_ship_hit_info->light;
	int	player_involved;	// flag to indicate that A or B is the Player_obj

	Assert( heavy_obj->type == OBJ_SHIP );
	Assert( light_obj->type == OBJ_SHIP );

	ship *heavy_shipp = &Ships[heavy_obj->instance];
	ship *light_shipp = &Ships[light_obj->instance];

	// AL 12-4-97: we use the player_involved flag to ensure collisions are always
	//             done with the player, regardless of team.
	if ( heavy_obj == Player_obj || light_obj == Player_obj ) {
		player_involved = 1;
	} else {
		player_involved = 0;
	}

	// Make ships that are warping in not get collision detection done
//	if ( heavy_shipp->flags & SF_ARRIVING ) {
	if ( heavy_shipp->flags & SF_ARRIVING_STAGE_1 ) { 
  		return 0;
  	}

	// Don't do collision detection for docking ships, since they will always collide while trying to dock
	if ( ships_are_docking(heavy_obj, light_obj) ) {
		return 0;
	}

	//	If light_obj emerging from or departing to dock bay in heavy_obj, no collision detection.
	if (bay_emerge_or_depart(heavy_obj, light_obj)) {
		return 0;
	}

	//	Ships which are dying should not do collision detection.
	//	Also, this is the only clean way I could figure to get ships to not do damage to each other for one frame
	//	when they are docked and departing.  Due to sequencing, they would not show up as docked, yet they
	//	would still come through here, so they would harm each other, if on opposing teams. -- MK, 2/2/98
	if ((heavy_obj->flags & OF_SHOULD_BE_DEAD) || (light_obj->flags & OF_SHOULD_BE_DEAD)) {
		return 0;
	}

	//nprintf(("AI", "Frame %i: Collision between %s and %s\n", Framecount, heavy_shipp->ship_name, light_shipp->ship_name));

#ifndef NDEBUG
	//	Don't do collision detection on a pair of ships on the same team.
	//	Change this someday, but for now, it's a problem.
	if ( !Collide_friendly ) {		// Collide_friendly is a global value changed via debug console
		if ( (!player_involved) && (heavy_shipp->team == light_shipp->team) ) {
			return 0;
		}
	}
#endif

	//	Apparently we're doing same team collisions.
	//	But, if both are offscreen, ignore the collision
	if (heavy_shipp->team == light_shipp->team) {
//		if ((Game_mode & GM_MULTIPLAYER) || (!(heavy_obj->flags & OF_WAS_RENDERED) && !(light_obj->flags & OF_WAS_RENDERED)))
		// mwa 4/28/98 -- don't understand why GM_MULTIPLAYER was included in this line.  All clients
		// need to do all collisions for their own ship. removing the multiplayer part of next if statement.

		if ( (!(heavy_obj->flags & OF_WAS_RENDERED) && !(light_obj->flags & OF_WAS_RENDERED)) ) {
			return 0;
		}
	}

	//	If either of these objects doesn't get collision checks, abort.
	if (Ship_info[heavy_shipp->ship_info_index].flags & SIF_NO_COLLIDE) {
		return 0;
	}

	if (Ship_info[light_shipp->ship_info_index].flags & SIF_NO_COLLIDE) {
		return 0;
	}

	// Set up model_collide info
	mc_info mc;
	memset(&mc, -1, sizeof(mc_info));

//	vec3d submodel_hit;

	// Do in heavy object RF
	mc.model_num = Ship_info[heavy_shipp->ship_info_index].model_num;	// Fill in the model to check
	mc.orient = &heavy_obj->orient;		// The object's orient

	vec3d zero, p0, p1;
	vm_vec_zero(&zero);		// we need the physical vector and can not set its value to zero
	vm_vec_sub(&p0, &light_obj->last_pos, &heavy_obj->last_pos);
	vm_vec_sub(&p1, &light_obj->pos, &heavy_obj->pos);

	// find the light object's position in the heavy object's reference frame at last frame and also in this frame.
	vec3d p0_temp, p0_rotated;
	
	// Collision detection from rotation enabled if at max rotaional velocity and 5fps, rotation is less than PI/2
	// This should account for all ships
	if ( (vm_vec_mag_squared( &heavy_obj->phys_info.max_rotvel ) * .04) < (PI*PI/4) ) {
		// collide_rotate calculate (1) start position and (2) relative velocity
		ship_ship_hit_info->collide_rotate = 1;
		vm_vec_rotate(&p0_temp, &p0, &heavy_obj->last_orient);
		vm_vec_unrotate(&p0_rotated, &p0_temp, &heavy_obj->orient);
		mc.p0 = &p0_rotated;				// Point 1 of ray to check
		vm_vec_sub(&ship_ship_hit_info->light_rel_vel, &p1, &p0_rotated);
		vm_vec_scale(&ship_ship_hit_info->light_rel_vel, 1/flFrametime);
	} else {
		// should be no ships that can rotate this fast
		Int3();
		ship_ship_hit_info->collide_rotate = 0;
		mc.p0 = &p0;							// Point 1 of ray to check
		vm_vec_sub(&ship_ship_hit_info->light_rel_vel, &light_obj->phys_info.vel, &heavy_obj->phys_info.vel);
	}
	
	// Set up collision info
	mc.pos = &zero;						// The object's position
	mc.p1 = &p1;							// Point 2 of ray to check
	mc.radius = model_get_core_radius(Ship_info[light_shipp->ship_info_index].model_num);
	mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);			// flags

	//	Only check invisible face polygons for ship:ship of different teams.
	if ( !(heavy_shipp->flags2 & SF2_DONT_COLLIDE_INVIS) ) {
		if ((heavy_obj->flags & OF_PLAYER_SHIP) || (light_obj->flags & OF_PLAYER_SHIP) || (heavy_shipp->team != light_shipp->team) ) {
			mc.flags |= MC_CHECK_INVISIBLE_FACES;
		}
	}
	
	// copy important data
	int copy_flags = mc.flags;  // make a copy of start end positions of sphere in  big ship RF
	vec3d copy_p0, copy_p1;
	copy_p0 = *mc.p0;
	copy_p1 = *mc.p1;

	// first test against the sphere - if this fails then don't do any submodel tests
	mc.flags = MC_ONLY_SPHERE | MC_CHECK_SPHERELINE;

	int submodel_list[MAX_ROTATING_SUBMODELS];
	int num_rotating_submodels = 0;
	int valid_hit_occured = 0;
	polymodel *pm;

	ship_model_start(heavy_obj);

	if (model_collide(&mc)) {

		// Set earliest hit time
		ship_ship_hit_info->hit_time = FLT_MAX;

		// Do collision the cool new way
		if ( ship_ship_hit_info->collide_rotate ) {

			model_get_rotating_submodel_list(submodel_list, &num_rotating_submodels, heavy_obj);

			pm = model_get(Ship_info[heavy_shipp->ship_info_index].model_num);

			// turn off all rotating submodels and test for collision
			int i;
			for (i=0; i<num_rotating_submodels; i++) {
				pm->submodel[submodel_list[i]].blown_off = 1;
			}

			// reset flags to check MC_CHECK_MODEL | MC_CHECK_SPHERELINE and maybe MC_CHECK_INVISIBLE_FACES and MC_SUBMODEL_INSTANCE
			mc.flags = copy_flags | MC_SUBMODEL_INSTANCE;

			// check each submodel in turn
			for (i=0; i<num_rotating_submodels; i++) {
				// turn on submodel for collision test
				pm->submodel[submodel_list[i]].blown_off = 0;

				// set angles for last frame
				angles copy_angles = pm->submodel[submodel_list[i]].angs;

				// find the start and end positions of the sphere in submodel RF
				pm->submodel[submodel_list[i]].angs = pm->submodel[submodel_list[i]].sii->prev_angs;
				world_find_model_point(&p0, &light_obj->last_pos, pm, submodel_list[i], &heavy_obj->last_orient, &heavy_obj->last_pos);

				pm->submodel[submodel_list[i]].angs = copy_angles;
				world_find_model_point(&p1, &light_obj->pos, pm, submodel_list[i], &heavy_obj->orient, &heavy_obj->pos);

				mc.p0 = &p0;
				mc.p1 = &p1;
				// mc.pos = zero	// in submodel RF

				mc.orient = &vmd_identity_matrix;
				mc.submodel_num = submodel_list[i];

				if ( model_collide(&mc) ) {
					if (mc.hit_dist < ship_ship_hit_info->hit_time ) {
						valid_hit_occured = 1;

						// set up ship_ship_hit_info common
						set_hit_struct_info(ship_ship_hit_info, &mc, SUBMODEL_ROT_HIT);
						model_find_world_point(&ship_ship_hit_info->hit_pos, &mc.hit_point, mc.model_num, mc.hit_submodel, &heavy_obj->orient, &zero);

						// set up ship_ship_hit_info for rotating submodel
						if (ship_ship_hit_info->edge_hit == 0) {
							model_find_obj_dir(&ship_ship_hit_info->collision_normal, &mc.hit_normal, heavy_obj, mc.hit_submodel);
						}

						// find position in submodel RF of light object at collison
						vec3d int_light_pos, diff;
						vm_vec_sub(&diff, mc.p1, mc.p0);
						vm_vec_scale_add(&int_light_pos, mc.p0, &diff, mc.hit_dist);
						model_find_world_point(&ship_ship_hit_info->light_collision_cm_pos, &int_light_pos, mc.model_num, mc.hit_submodel, &heavy_obj->orient, &zero);

//						submodel_hit = mc.hit_point;

						/*
						// find position in CM RF of the heavy object at collision
						vm_vec_sub(&diff, &heavy_obj->pos, &heavy_obj->last_pos);
						vm_vec_scale_add(&int_heavy_pos, &heavy_obj->last_pos, &diff, mc.hit_dist);

						// Find orientation of heavy at time of collision.  Use last_orientation * delta_orientation.
						// heavy last orient * (delta_orient * time)
						matrix m_temp, rot_matrix;
						float theta;
						vec3d rot_axis;

						vm_copy_transpose_matrix(&m_temp, &heavy_obj->last_orient);			// Mtemp1 = curr ^-1
						vm_matrix_x_matrix(&rot_matrix, &m_temp, &heavy_obj->orient);		// R = goal * Mtemp1
						vm_matrix_to_rot_axis_and_angle(&rot_matrix, &theta, &rot_axis);	// determines angle and rotation axis from curr to goal
						vm_quaternion_rotate(&m_temp, theta * mc.hit_dist, &rot_axis);
						Assert(is_valid_matrix(&m_temp));
						vm_matrix_x_matrix(&int_heavy_orient, &heavy_obj->last_orient, &m_temp);

						// set submodel angle at time of collision
						// TODO: generalize... what happens when angle passes 0 or 2PI
						angles temp_angs;
						vm_vec_sub(&diff, (vec3d*)&pm->submodel[submodel_list[i]].angs, (vec3d*)&pm->submodel[submodel_list[i]].sii->prev_angs);
						vm_vec_scale_add((vec3d*)&temp_angs, (vec3d *)&pm->submodel[submodel_list[i]].sii->prev_angs, &diff, mc.hit_dist);
						pm->submodel[submodel_list[i]].angs = temp_angs;

						// find intersection point in submodel RF - THEN advance to end of frametime.
						vec3d temp = int_light_pos;
						world_find_model_point(&int_submodel_pos, &int_light_pos, pm, submodel_list[i], &int_heavy_orient, &int_heavy_pos);
						vec3d temp2;

						// Advance to end of frametime
						pm->submodel[submodel_list[i]].angs = copy_angles;
						model_find_world_point(&ship_ship_hit_info->light_collision_cm_pos, &int_submodel_pos, mc.model_num, mc.hit_submodel, mc.orient, &zero);
						vm_vec_sub(&temp2, &ship_ship_hit_info->light_collision_cm_pos, &ship_ship_hit_info->hit_pos);
		*/
		//					vec3d temp2;
		//					vm_vec_sub(&temp2, &ship_ship_hit_info->light_collision_cm_pos, &ship_ship_hit_info->hit_pos);

					}
				}
				// Don't look at this submodel again
				pm->submodel[submodel_list[i]].blown_off = 1;
			}

		}

		// Recover and do usual ship_ship collision, but without rotating submodels
		mc.flags = copy_flags;
		*mc.p0 = copy_p0;
		*mc.p1 = copy_p1;
		mc.orient = &heavy_obj->orient;

		// usual ship_ship collision test
		if ( model_collide(&mc) )	{
			// check if this is the earliest hit
			if (mc.hit_dist < ship_ship_hit_info->hit_time) {
				valid_hit_occured = 1;

				set_hit_struct_info(ship_ship_hit_info, &mc, SUBMODEL_NO_ROT_HIT);

				// get hitpos - heavy_pos
//				if ( ship_ship_hit_info->collide_rotate ) {
//					model_find_world_point(&ship_ship_hit_info->hit_pos, &mc.hit_point, mc.model_num, mc.hit_submodel, &heavy_obj->orient, &zero);
//				}

				// get collision normal if not edge hit
				if (ship_ship_hit_info->edge_hit == 0) {
					model_find_obj_dir(&ship_ship_hit_info->collision_normal, &mc.hit_normal, heavy_obj, mc.hit_submodel);
				}

				// find position in submodel RF of light object at collison
				vec3d diff;
				vm_vec_sub(&diff, mc.p1, mc.p0);
				vm_vec_scale_add(&ship_ship_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);

//				submodel_hit = mc.hit_point;
			}
		}

		ship_model_stop( heavy_obj );
	}

	if (valid_hit_occured) {

		// Collision debug stuff
#ifdef DEBUG
		object *collide_obj = NULL;
		if (heavy_obj == Player_obj) {
			collide_obj = light_obj;
		} else if (light_obj == Player_obj) {
			collide_obj = heavy_obj;
		}
		if ((collide_obj != NULL) && (Ship_info[Ships[collide_obj->instance].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))) {
			char	*submode_string = "";
			ai_info	*aip;

			extern char *Mode_text[];
			aip = &Ai_info[Ships[collide_obj->instance].ai_index];

			if (aip->mode == AIM_CHASE)
				submode_string = Submode_text[aip->submode];

			nprintf(("AI", "Player collided with ship %s, AI mode = %s, submode = %s\n", Ships[collide_obj->instance].ship_name, Mode_text[aip->mode], submode_string));
		}
#endif

		// Update ai to deal with collisions
		if (heavy_obj-Objects == Ai_info[light_shipp->ai_index].target_objnum) {
			Ai_info[light_shipp->ai_index].ai_flags |= AIF_TARGET_COLLISION;
		}
		if (light_obj-Objects == Ai_info[heavy_shipp->ai_index].target_objnum) {
			Ai_info[heavy_shipp->ai_index].ai_flags |= AIF_TARGET_COLLISION;
		}

		// SET PHYSICS PARAMETERS
		// already have (hitpos - heavy) and light_cm_pos
		// get heavy cm pos - already have light_cm_pos
		ship_ship_hit_info->heavy_collision_cm_pos = zero;

		// get r_heavy and r_light
		ship_ship_hit_info->r_heavy = ship_ship_hit_info->hit_pos;
		vm_vec_sub(&ship_ship_hit_info->r_light, &ship_ship_hit_info->hit_pos, &ship_ship_hit_info->light_collision_cm_pos);

		// set normal for edge hit
		if ( ship_ship_hit_info->edge_hit ) {
			vm_vec_copy_normalize(&ship_ship_hit_info->collision_normal, &ship_ship_hit_info->r_light);
			vm_vec_negate(&ship_ship_hit_info->collision_normal);
		}

		// get world hitpos
		vm_vec_add(hitpos, &ship_ship_hit_info->heavy->pos, &ship_ship_hit_info->r_heavy);

		/*
		vec3d temp1, temp2, temp3, diff;
		vm_vec_add(&temp1, &ship_ship_hit_info->light_collision_cm_pos, &ship_ship_hit_info->r_light);
		vm_vec_add(&temp2, &ship_ship_hit_info->heavy_collision_cm_pos, &ship_ship_hit_info->r_heavy);
		vm_vec_sub(&diff, &temp2, &temp1);

		ship_model_start( heavy_obj );
		pm = model_get(heavy_shipp->modelnum);
		world_find_model_point(&temp3, hitpos, pm, ship_ship_hit_info->submodel_num, &heavy_obj->orient, &heavy_obj->pos);
		ship_model_stop( heavy_obj );

		vm_vec_sub(&diff, &submodel_hit, &temp3);

		if (vm_vec_mag(&diff) > 0.1) {
			Int3();
		}	*/


		// do physics
		calculate_ship_ship_collision_physics(ship_ship_hit_info);

		// Provide some separation for the case of same team
		if (heavy_shipp->team == light_shipp->team) {
			//	If a couple of small ships, just move them apart.

			if ((Ship_info[heavy_shipp->ship_info_index].flags & SIF_SMALL_SHIP) && (Ship_info[light_shipp->ship_info_index].flags & SIF_SMALL_SHIP)) {
				if ((heavy_obj->flags & OF_PLAYER_SHIP) || (light_obj->flags & OF_PLAYER_SHIP)) {
					/*
					vec3d	h2l_vec;										
					float		mass_sum = heavy_obj->phys_info.mass + light_obj->phys_info.mass; 
					float		lh_ratio;

					lh_ratio = light_obj->phys_info.mass/mass_sum;
					if (lh_ratio < 0.2f) {
						lh_ratio = 0.2f;
					}

					// actually initialize h2l_vec
					vm_vec_sub(&h2l_vec, &light_obj->pos, &heavy_obj->pos);
					
					//	Choose best direction to move objects.  Want to move away from collision point.
					//	Hmm, maybe this is needlessly complex.  Maybe should use collision point and slide them
					//	away from that? -- MK, 4/5/98
					
					if (vm_vec_dot(&light_obj->phys_info.vel, &h2l_vec) > 0.0f) {
						vm_vec_scale_add2(&light_obj->phys_info.vel, &h2l_vec, 10.0f * (1.0f - lh_ratio));
					} else {
						if (vm_vec_dot(&light_obj->orient.rvec, &h2l_vec) < 0.0f) {
							vm_vec_scale_add2(&light_obj->phys_info.vel, &light_obj->orient.rvec, -10.0f * (1.0f - lh_ratio));
						} else {
							vm_vec_scale_add2(&light_obj->phys_info.vel, &light_obj->orient.rvec, +10.0f * (1.0f - lh_ratio));
						}
					}

					if (vm_vec_dot(&heavy_obj->phys_info.vel, &h2l_vec) < 0.0f) {
						vm_vec_scale_add2(&heavy_obj->phys_info.vel, &h2l_vec, 10.0f * (1.0f - lh_ratio));
					} else {
						if (vm_vec_dot(&heavy_obj->orient.rvec, &h2l_vec) < 0.0f) {
							vm_vec_scale_add2(&light_obj->phys_info.vel, &light_obj->orient.rvec, +10.0f * (1.0f - lh_ratio));
						} else {
							vm_vec_scale_add2(&light_obj->phys_info.vel, &light_obj->orient.rvec, -10.0f * (1.0f - lh_ratio));
						}
					}*/

					vec3d h_to_l_vec;
					vec3d rel_vel_h;
					vec3d perp_rel_vel;

					vm_vec_sub(&h_to_l_vec, &heavy_obj->pos, &light_obj->pos);
					vm_vec_sub(&rel_vel_h, &heavy_obj->phys_info.vel, &light_obj->phys_info.vel);
					float mass_sum = light_obj->phys_info.mass + heavy_obj->phys_info.mass;

					// get comp of rel_vel perp to h_to_l_vec;
					float mag = vm_vec_dotprod(&h_to_l_vec, &rel_vel_h) / vm_vec_mag_squared(&h_to_l_vec);
					vm_vec_scale_add(&perp_rel_vel, &rel_vel_h, &h_to_l_vec, -mag);
					vm_vec_normalize(&perp_rel_vel);

					vm_vec_scale_add2(&heavy_obj->phys_info.vel, &perp_rel_vel, SEP_VEL * light_obj->phys_info.mass / mass_sum);
					vm_vec_scale_add2(&light_obj->phys_info.vel, &perp_rel_vel, -SEP_VEL * heavy_obj->phys_info.mass / mass_sum);

					vm_vec_rotate( &heavy_obj->phys_info.prev_ramp_vel, &heavy_obj->phys_info.vel, &heavy_obj->orient );
					vm_vec_rotate( &light_obj->phys_info.prev_ramp_vel, &light_obj->phys_info.vel, &light_obj->orient );
				}
			} else {
				// add extra velocity to separate the two objects, backing up the direction we came in.
				// TODO: add effect of velocity from rotating submodel
				float rel_vel = vm_vec_mag_quick( &ship_ship_hit_info->light_rel_vel);
				if (rel_vel < 1) {
					rel_vel = 1.0f;
				}
				float		mass_sum = heavy_obj->phys_info.mass + light_obj->phys_info.mass; 
				vm_vec_scale_add2( &heavy_obj->phys_info.vel, &ship_ship_hit_info->light_rel_vel, SEP_VEL*light_obj->phys_info.mass/(mass_sum*rel_vel) );
				vm_vec_rotate( &heavy_obj->phys_info.prev_ramp_vel, &heavy_obj->phys_info.vel, &heavy_obj->orient );
				vm_vec_scale_add2( &light_obj->phys_info.vel, &ship_ship_hit_info->light_rel_vel, -SEP_VEL*heavy_obj->phys_info.mass/(mass_sum*rel_vel) );
				vm_vec_rotate( &light_obj->phys_info.prev_ramp_vel, &light_obj->phys_info.vel, &light_obj->orient );
			}
		}
	}


	return valid_hit_occured;
}

// gets modified mass of cruiser in cruiser/asteroid collision so cruisers dont get bumped so hard.
// modified mass is 10x, 4x, or 2x larger than asteroid mass
// returns 1 if modified mass is larger than given mass, 0 otherwise 
int check_special_cruiser_asteroid_collision(object *heavy, object *light, float *cruiser_mass, int *cruiser_light)
{
#ifndef FS2_DEMO
	int asteroid_type;

	if (heavy->type == OBJ_ASTEROID) {
		Assert(light->type == OBJ_SHIP);
		if (Ship_info[Ships[light->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {

			asteroid_type = Asteroids[heavy->instance].asteroid_type;
			if (asteroid_type == 0) {
				*cruiser_mass = 10.0f * heavy->phys_info.mass;
			} else if (asteroid_type == 1) {
				*cruiser_mass = 4.0f * heavy->phys_info.mass;
			} else {
				*cruiser_mass = 2.0f * heavy->phys_info.mass;
			}

			if (*cruiser_mass > light->phys_info.mass) {
				*cruiser_light = 1;
				return 1;
			}
		}
	} else if (light->type == OBJ_ASTEROID) {
		Assert(heavy->type == OBJ_SHIP);
		if (Ship_info[Ships[heavy->instance].ship_info_index].flags & SIF_BIG_SHIP) {

			asteroid_type = Asteroids[light->instance].asteroid_type;
			if (asteroid_type == 0) {
				*cruiser_mass = 10.0f * light->phys_info.mass;
			} else if (asteroid_type == 1) {
				*cruiser_mass = 4.0f * light->phys_info.mass;
			} else {
				*cruiser_mass = 2.0f * light->phys_info.mass;
			}

			if (*cruiser_mass > heavy->phys_info.mass) {
				*cruiser_light = 0;
				return 1;
			}
		}
	}
#endif
	return 0;
}

// ------------------------------------------------------------------------------------------------
//		input:		ship_ship_hit		=>		structure containing ship_ship hit info
//		(includes)	A, B					=>		objects colliding
//						r_A, r_B				=>		position to collision from center of mass
//						collision_normal	=>		collision_normal (outward from B)			
//
//		output:	velocity, angular velocity, impulse
//
// ------------------------------------------------------------------------------------------------
//
// calculates correct physics response to collision between two objects given
//		masses, moments of inertia, velocities, angular velocities, 
//		relative collision positions, and the impulse direction
//
void calculate_ship_ship_collision_physics(collision_info_struct *ship_ship_hit_info)
{
	// important parameters passed thru ship_ship_or_debris_hit
	// calculate the whack applied to each ship from collision

	// make local copies of hit_struct parameters
	object *heavy = ship_ship_hit_info->heavy;
	object *light = ship_ship_hit_info->light;

	// make cruiser/asteroid collision softer on cruisers.
	int special_cruiser_asteroid_collision;
	int cruiser_light = 0;
	float cruiser_mass = 0.0f, copy_mass = 0.0f;
	special_cruiser_asteroid_collision = check_special_cruiser_asteroid_collision(heavy, light, &cruiser_mass, &cruiser_light);

	if (special_cruiser_asteroid_collision) {
		if (cruiser_light) {
			Assert(light->phys_info.mass < cruiser_mass);
			copy_mass = light->phys_info.mass;
			light->phys_info.mass = cruiser_mass;
		} else {
			Assert(heavy->phys_info.mass < cruiser_mass);
			copy_mass = heavy->phys_info.mass;
			heavy->phys_info.mass = cruiser_mass;
		}
	}

	float		coeff_restitution;	// parameter controls amount of bounce
	float		v_rel_normal_m;		// relative collision velocity in the direction of the collision normal
	vec3d	v_rel_parallel_m;		// normalized v_rel (Va-Vb) projected onto collision surface
	vec3d	world_rotvel_heavy_m, world_rotvel_light_m, vel_from_rotvel_heavy_m, vel_from_rotvel_light_m, v_rel_m, vel_heavy_m, vel_light_m;

	coeff_restitution = 0.1f;		// relative velocity wrt normal is zero after the collision ( range 0-1 )

	// find velocity of each obj at collision point

	// heavy object is in cm reference frame so we don't get a v_heavy term.
	if ( ship_ship_hit_info->collide_rotate ) {
		// if we have collisions from rotation, the effective velocity from rotation of the large body is alreay taken account
		vm_vec_zero( &vel_heavy_m );
	} else {
		// take account the effective velocity from rotation
		vm_vec_unrotate(&world_rotvel_heavy_m, &heavy->phys_info.rotvel, &heavy->orient);	// heavy's world rotvel before collision
		vm_vec_crossprod(&vel_from_rotvel_heavy_m, &world_rotvel_heavy_m, &ship_ship_hit_info->r_heavy);	// heavy's velocity from rotvel before collision
		vel_heavy_m = vel_from_rotvel_heavy_m;
	}

	// if collision from rotating submodel of heavy obj, add in vel from rotvel of submodel
	vec3d local_vel_from_submodel;

	if (ship_ship_hit_info->submodel_rot_hit == 1) {
		bool set_model = false;

		polymodel *pm = model_get(Ship_info[Ships[heavy->instance].ship_info_index].model_num);

		// be sure model is set
		if (pm->submodel[ship_ship_hit_info->submodel_num].sii == NULL) {
			set_model = true;
			ship_model_start(heavy);
		}

		// set point on axis of rotating submodel if not already set.
		if (!pm->submodel[ship_ship_hit_info->submodel_num].sii->axis_set) {
			model_init_submodel_axis_pt(pm->submodel[ship_ship_hit_info->submodel_num].sii, pm->id, ship_ship_hit_info->submodel_num);
		}

		vec3d omega, axis, r_rot;
		if (pm->submodel[ship_ship_hit_info->submodel_num].movement_axis == MOVEMENT_AXIS_X) {
			axis = vmd_x_vector;
		} else if (pm->submodel[ship_ship_hit_info->submodel_num].movement_axis == MOVEMENT_AXIS_Y) {
			axis = vmd_y_vector;
		} else if (pm->submodel[ship_ship_hit_info->submodel_num].movement_axis == MOVEMENT_AXIS_Z) {
			axis = vmd_z_vector;
		} else {
			// must be one of these axes or submodel_rot_hit is incorrectly set
			Int3();
		}

		// get world rotational velocity of rotating submodel
		model_find_obj_dir(&omega, &axis, heavy, ship_ship_hit_info->submodel_num);
		vm_vec_scale(&omega, pm->submodel[ship_ship_hit_info->submodel_num].sii->cur_turn_rate);

		// world coords for r_rot
		vec3d temp;
		vm_vec_unrotate(&temp, &pm->submodel[ship_ship_hit_info->submodel_num].sii->pt_on_axis, &heavy->orient);
		vm_vec_sub(&r_rot, &ship_ship_hit_info->hit_pos, &temp);
//		vm_vec_rotate(&temp, &r_rot, &heavy->orient);	// to ship coords

		vm_vec_crossprod(&local_vel_from_submodel, &omega, &r_rot);
//		vm_vec_rotate(&temp, &local_vel_from_submodel, &heavy->orient); // to ship coords

//		if (vm_vec_dotprod(&local_vel_from_submodel, &ship_ship_hit_info->collision_normal) > 0) {
//			nprintf(("Physics", "Rotating submodel collision - got whacked\n"));
//		} else {
//			nprintf(("Physics", "Rotating submodel collision - sub got whacked from behind\n"));
//		}
		if (set_model) {
			ship_model_stop(heavy);
		}
	} else {
		// didn't collide with submodel
		vm_vec_zero(&local_vel_from_submodel);
	}

	vm_vec_unrotate(&world_rotvel_light_m, &light->phys_info.rotvel, &light->orient);		// light's world rotvel before collision
	vm_vec_crossprod(&vel_from_rotvel_light_m, &world_rotvel_light_m, &ship_ship_hit_info->r_light);	// light's velocity from rotvel before collision
	vm_vec_add(&vel_light_m, &vel_from_rotvel_light_m, &ship_ship_hit_info->light_rel_vel);
	vm_vec_sub(&v_rel_m, &vel_light_m, &vel_heavy_m);

	// Add in effect of rotating submodel
	vm_vec_sub2(&v_rel_m, &local_vel_from_submodel);

	v_rel_normal_m = vm_vec_dotprod(&v_rel_m, &ship_ship_hit_info->collision_normal);// if less than zero, colliding contact taking place
																									// (v_slow - v_fast) dot (n_fast)

	if (v_rel_normal_m > 0) {
	//	This can happen in 2 situations.
	// (1) The rotational velocity is large enough to cause ships to miss.  In this case, there would most likely
	// have been a collision, but at a later time, so reset v_rel_normal_m 

	//	(2) We could also have just gotten a slightly incorrect hitpos, where r dot v_rel is nearly zero.  
	//	In this case, we know there was a collision, but slight collision and the normal is correct, so reset v_rel_normal_m
	//	need a normal direction.  We can just take the -v_light normalized.		v_rel_normal_m = -v_rel_normal_m;
		nprintf(("Physics", "Frame %i reset v_rel_normal_m %f Edge %i\n", Framecount, v_rel_normal_m, ship_ship_hit_info->edge_hit));
		// if (v_rel_normal_m > 5)
		//	Warning(LOCATION, "v_rel_normal_m > 5 %f  Get Dave A.\n", -v_rel_normal_m);
		v_rel_normal_m = -v_rel_normal_m;
	}

	vec3d	rotational_impulse_heavy, rotational_impulse_light, delta_rotvel_heavy, delta_rotvel_light;
	vec3d	delta_vel_from_delta_rotvel_heavy, delta_vel_from_delta_rotvel_light, impulse;
	float		impulse_mag, heavy_denom, light_denom;
	matrix	heavy_I_inv, light_I_inv;

	// include a frictional collision impulse F parallel to the collision plane
	// F = I * sin (collision_normal, normalized v_rel_m)  [sin is ratio of v_rel_parallel_m to v_rel_m]
	// note:  (-) sign is needed to account for the direction of the v_rel_parallel_m
	float collision_speed_parallel;
	float parallel_mag;
	impulse = ship_ship_hit_info->collision_normal;
	vm_vec_projection_onto_plane(&v_rel_parallel_m, &v_rel_m, &ship_ship_hit_info->collision_normal);
	collision_speed_parallel = vm_vec_normalize_safe(&v_rel_parallel_m);
	parallel_mag = float(-COLLISION_FRICTION_FACTOR) * collision_speed_parallel / vm_vec_mag(&v_rel_m);
	vm_vec_scale_add2(&impulse, &v_rel_parallel_m, parallel_mag);
	
	// calculate the effect on the velocity of the collison point per unit impulse
	// first find the effect thru change in rotvel
	// then find the change in the cm vel
	if (heavy == Player_obj) {
		vm_vec_zero( &delta_rotvel_heavy );
		heavy_denom = 1.0f / heavy->phys_info.mass;
	} else {
		vm_vec_crossprod(&rotational_impulse_heavy, &ship_ship_hit_info->r_heavy, &impulse);
		get_I_inv(&heavy_I_inv, &heavy->phys_info.I_body_inv, &heavy->orient);
		vm_vec_rotate(&delta_rotvel_heavy, &rotational_impulse_heavy, &heavy_I_inv);
		vm_vec_scale(&delta_rotvel_heavy, float(COLLISION_ROTATION_FACTOR));		// hack decrease rotation (delta_rotvel)
		vm_vec_crossprod(&delta_vel_from_delta_rotvel_heavy, &delta_rotvel_heavy , &ship_ship_hit_info->r_heavy);
		heavy_denom = vm_vec_dotprod(&delta_vel_from_delta_rotvel_heavy, &ship_ship_hit_info->collision_normal);
		if (heavy_denom < 0) {
			// sanity check
			heavy_denom = 0.0f;
		}
		heavy_denom += 1.0f / heavy->phys_info.mass;
	} 

	// calculate the effect on the velocity of the collison point per unit impulse
	// first find the effect thru change in rotvel
	// then find the change in the cm vel
	if (light == Player_obj) {
		vm_vec_zero( &delta_rotvel_light );
		light_denom = 1.0f / light->phys_info.mass;
	} else {
		vm_vec_crossprod(&rotational_impulse_light, &ship_ship_hit_info->r_light, &impulse);
		get_I_inv(&light_I_inv, &light->phys_info.I_body_inv, &light->orient);
		vm_vec_rotate(&delta_rotvel_light, &rotational_impulse_light, &light_I_inv);
		vm_vec_scale(&delta_rotvel_light, float(COLLISION_ROTATION_FACTOR));		// hack decrease rotation (delta_rotvel)
		vm_vec_crossprod(&delta_vel_from_delta_rotvel_light, &delta_rotvel_light, &ship_ship_hit_info->r_light);
		light_denom = vm_vec_dotprod(&delta_vel_from_delta_rotvel_light, &ship_ship_hit_info->collision_normal);
		if (light_denom < 0) {
			// sanity check
			light_denom = 0.0f;
		}
		light_denom += 1.0f / light->phys_info.mass;
	} 

	// calculate the necessary impulse to achieved the desired relative velocity after the collision
	// update damage info in mc
	impulse_mag = -(1.0f + coeff_restitution)*v_rel_normal_m / (heavy_denom + light_denom);
	ship_ship_hit_info->impulse = impulse_mag;
	if (impulse_mag < 0) {
		nprintf(("Physics", "negative impulse mag -- Get Dave A if not Descent Physics\n"));
		impulse_mag = -impulse_mag;
	}
	
	// update the physics info structs for heavy and light objects
	// since we have already calculated delta rotvel for heavy and light in world coords
	// physics should not have to recalculate this, just change into body coords (done in collide_whack)
	vm_vec_scale(&impulse, impulse_mag);
	//Assert(impulse_mag < 20e6);
	vm_vec_scale(&delta_rotvel_light, impulse_mag);	
	physics_collide_whack(&impulse, &delta_rotvel_light, &light->phys_info, &light->orient);
	vm_vec_negate(&impulse);
	vm_vec_scale(&delta_rotvel_heavy, -impulse_mag);
	physics_collide_whack(&impulse, &delta_rotvel_heavy, &heavy->phys_info, &heavy->orient);

	// Find final positions
	// We will try not to worry about the left over time in the frame
	// heavy's position unchanged by collision
	// light's position is heavy's position plus relative position from heavy
	vm_vec_add(&light->pos, &heavy->pos, &ship_ship_hit_info->light_collision_cm_pos);

	// Try to move each body back to its position just before collision occured to prevent interpenetration
	// Move away in direction of light and away in direction of normal
	vec3d direction_light;	// direction light is moving relative to heavy
	vm_vec_sub(&direction_light, &ship_ship_hit_info->light_rel_vel, &local_vel_from_submodel);
	vm_vec_normalize_safe(&direction_light);

	Assert( !vm_is_vec_nan(&direction_light) );
	vm_vec_scale_add2(&heavy->pos, &direction_light,  0.2f * light->phys_info.mass / (heavy->phys_info.mass + light->phys_info.mass));
	vm_vec_scale_add2(&light->pos, &direction_light, -0.2f * heavy->phys_info.mass / (heavy->phys_info.mass + light->phys_info.mass));
	vm_vec_scale_add2(&heavy->pos, &ship_ship_hit_info->collision_normal, -0.1f * light->phys_info.mass / (heavy->phys_info.mass + light->phys_info.mass));
	vm_vec_scale_add2(&light->pos, &ship_ship_hit_info->collision_normal,  0.1f * heavy->phys_info.mass / (heavy->phys_info.mass + light->phys_info.mass));

	// restore mass in case of special cruuiser / asteroid collision
	if (special_cruiser_asteroid_collision) {
		if (cruiser_light) {
			light->phys_info.mass = copy_mass;
		} else {
			heavy->phys_info.mass = copy_mass;
		}
	}
}


// ------------------------------------------------------------------------------------------------
//	get_I_inv()
//
//		input:	I_inv_body	=>		inverse moment of inertia matrix in body coordinates
//					orient		=>		orientation matrix
//
//		output:	I_inv			=>		inverse moment of inertia matrix in world coordinates
// ------------------------------------------------------------------------------------------------
//
// calculates the inverse moment of inertia matrix from the body matrix and oreint matrix
//
void get_I_inv (matrix* I_inv, matrix* I_inv_body, matrix* orient)
{
	matrix Mtemp1, Mtemp2;
	// I_inv = (Rt)(I_inv_body)(R)
	// This is opposite to what is commonly seen in books since we are rotating coordianates axes 
	// which is equivalent to rotating in the opposite direction (or transpose)

	vm_matrix_x_matrix(&Mtemp1, I_inv_body, orient);
	vm_copy_transpose_matrix(&Mtemp2, orient);
	vm_matrix_x_matrix(I_inv, &Mtemp2, &Mtemp1);
}

#define	PLANET_DAMAGE_SCALE	4.0f
#define	PLANET_DAMAGE_RANGE	3		//	If within this factor of radius, apply damage.

fix	Last_planet_damage_time = 0;
extern void hud_start_text_flash(char *txt, int t);

//	Procss player_ship:planet damage.
//	If within range of planet, apply damage to ship.
void mcp_1(object *player_objp, object *planet_objp)
{
	float	planet_radius;
	float	dist;

	planet_radius = planet_objp->radius;
	dist = vm_vec_dist_quick(&player_objp->pos, &planet_objp->pos);

	if (dist > planet_radius*PLANET_DAMAGE_RANGE)
		return;

	ship_apply_global_damage( player_objp, planet_objp, NULL, PLANET_DAMAGE_SCALE * flFrametime * (float)pow((planet_radius*PLANET_DAMAGE_RANGE)/dist, 3.0f) );

	if ((Missiontime - Last_planet_damage_time > F1_0) || (Missiontime < Last_planet_damage_time)) {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Too close to planet.  Taking damage!", 465));
		Last_planet_damage_time = Missiontime;
		snd_play_3d( &Snds[SND_ABURN_ENGAGE], &player_objp->pos, &View_position );
	}

}

//	Return true if *objp is a planet, else return false.
//	Hack: Just checking first six letters of name.
int is_planet(object *objp)
{
	return (strnicmp(Ships[objp->instance].ship_name, NOX("planet"), 6) == 0);
}


//	If exactly one of these is a planet and the other is a player ship, do something special.
//	Return true if this was a ship:planet (or planet_ship) collision and we processed it.
//	Else return false.
int maybe_collide_planet (object *obj1, object *obj2)
{
	ship_info	*sip1, *sip2;

	sip1 = &Ship_info[Ships[obj1->instance].ship_info_index];
	sip2 = &Ship_info[Ships[obj2->instance].ship_info_index];

	if (sip1->flags & SIF_PLAYER_SHIP) {
		if (is_planet(obj2)) {
			mcp_1(obj1, obj2);
			return 1;
		}
	} else if (is_planet(obj1)) {
		if (sip2->flags & SIF_PLAYER_SHIP) {
			mcp_1(obj2, obj1);
			return 1;
		}
	}

	return 0;
}

#define	MIN_REL_SPEED_FOR_LOUD_COLLISION		50		// relative speed of two colliding objects at which we play the "loud" collide sound

void collide_ship_ship_sounds_init()
{
	Player_collide_sound = -1;
	AI_collide_sound = -1;
	Player_collide_shield_sound = -1;
	AI_collide_shield_sound = -1;
}

// determine what sound to play when two ships collide
void collide_ship_ship_do_sound(vec3d *world_hit_pos, object *A, object *B, int player_involved)
{
	vec3d	rel_vel;
	float		rel_speed;
	int		light_collision=0;
			
	vm_vec_sub(&rel_vel, &A->phys_info.desired_vel, &B->phys_info.desired_vel);
	rel_speed = vm_vec_mag_quick(&rel_vel);

	if ( rel_speed > MIN_REL_SPEED_FOR_LOUD_COLLISION ) {
		snd_play_3d( &Snds[SND_SHIP_SHIP_HEAVY], world_hit_pos, &View_position );
	} else {
		light_collision=1;
		if ( player_involved ) {
			if ( !snd_is_playing(Player_collide_sound) ) {
				Player_collide_sound = snd_play_3d( &Snds[SND_SHIP_SHIP_LIGHT], world_hit_pos, &View_position );
			}
		} else {
			if ( !snd_is_playing(AI_collide_sound) ) {
				AI_collide_sound = snd_play_3d( &Snds[SND_SHIP_SHIP_LIGHT], world_hit_pos, &View_position );
			}
		}
	}

	// maybe play a "shield" collision sound overlay if appropriate
	if ( (shield_get_strength(A) > 5) || (shield_get_strength(B) > 5) ) {
		float vol_scale=1.0f;
		if ( light_collision ) {
			vol_scale=0.7f;
		}

		if ( player_involved ) {
			if ( !snd_is_playing(Player_collide_sound) ) {
				Player_collide_shield_sound = snd_play_3d( &Snds[SND_SHIP_SHIP_SHIELD], world_hit_pos, &View_position );
			}
		} else {
			if ( !snd_is_playing(Player_collide_sound) ) {
				AI_collide_shield_sound = snd_play_3d( &Snds[SND_SHIP_SHIP_SHIELD], world_hit_pos, &View_position );
			}
		}
	}
}

//	obj1 and obj2 collided.
//	If different teams, kamikaze bit set and other ship is large, auto-explode!
void do_kamikaze_crash(object *obj1, object *obj2)
{
	ai_info	*aip1, *aip2;
	ship		*ship1, *ship2;

	ship1 = &Ships[obj1->instance];
	ship2 = &Ships[obj2->instance];

	aip1 = &Ai_info[ship1->ai_index];
	aip2 = &Ai_info[ship2->ai_index];

	if (ship1->team != ship2->team) {
		if (aip1->ai_flags & AIF_KAMIKAZE) {
			if (Ship_info[ship2->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
				obj1->hull_strength = KAMIKAZE_HULL_ON_DEATH;
				shield_set_strength(obj1, 0.0f);
			}
		} if (aip2->ai_flags & AIF_KAMIKAZE) {
			if (Ship_info[ship1->ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) {
				obj2->hull_strength = KAMIKAZE_HULL_ON_DEATH;
				shield_set_strength(obj2, 0.0f);
			}
		}
	}
}

// response when hit by fast moving cap ship
void maybe_push_little_ship_from_fast_big_ship(object *big_obj, object *small_obj, float impulse, vec3d *normal)

{
	// Move player out of the way of a BIG|HUGE ship warping in or out
	int big_class = Ship_info[Ships[big_obj->instance].ship_info_index].class_type;
	int small_class = Ship_info[Ships[small_obj->instance].ship_info_index].class_type;
	if (big_class > -1 && Ship_types[big_class].ship_bools & STI_SHIP_WARP_PUSHES) {
		if (small_class > -1 && Ship_types[small_class].ship_bools & STI_SHIP_WARP_PUSHABLE) {
			float big_speed = vm_vec_mag_quick(&big_obj->phys_info.vel);
			if (big_speed > 3*big_obj->phys_info.max_vel.xyz.z) {
				// push player away in direction perp to forward of big ship
				// get perp vec
				vec3d temp, perp;
				vm_vec_sub(&temp, &small_obj->pos, &big_obj->pos);
				vm_vec_scale_add(&perp, &temp, &big_obj->orient.vec.fvec, -vm_vec_dotprod(&temp, &big_obj->orient.vec.fvec));
				vm_vec_normalize_quick(&perp);

				// don't drive into sfc we just collided with
				if (vm_vec_dotprod(&perp, normal) < 0) {
					vm_vec_negate(&perp);
				}

				// get magnitude of added perp vel
				float added_perp_vel_mag = impulse / small_obj->phys_info.mass;

				// add to vel and ramp vel
				vm_vec_scale_add2(&small_obj->phys_info.vel, &perp, added_perp_vel_mag);
				vm_vec_rotate(&small_obj->phys_info.prev_ramp_vel, &small_obj->phys_info.vel, &small_obj->orient);
			}
		}
	}
}

// Checks ship-ship collisions.  pair->a and pair->b are ships.
// Returns 1 if all future collisions between these can be ignored
// Always returns 0, since two ships can always collide unless one (1) dies or (2) warps out.
int collide_ship_ship( obj_pair * pair )
{
	int	player_involved;
	float dist;
	object *A = pair->a;
	object *B = pair->b;

	if ( A->type == OBJ_WAYPOINT ) return 1;
	if ( B->type == OBJ_WAYPOINT ) return 1;
	
	Assert( A->type == OBJ_SHIP );
	Assert( B->type == OBJ_SHIP );

	// If the player is one of the two colliding ships, flag this... it is used in
	// several places this function.
	if ( A == Player_obj || B == Player_obj ) {
		player_involved = 1;
	} else {
		player_involved = 0;
	}

	// Don't check collisions for warping out player if past stage 1.
	if ( player_involved && (Player->control_mode > PCM_WARPOUT_STAGE1) )	{
		return 0;
	}

	dist = vm_vec_dist( &A->pos, &B->pos );

	//	If one of these is a planet, do special stuff.
	if (maybe_collide_planet(A, B))
		return 0;

	if ( dist < A->radius + B->radius )	{
		int		hit;

		object	*HeavyOne, *LightOne;
		// if two objects have the same mass, make the one with the larger pointer address the HeavyOne.
		if ( fl_abs(A->phys_info.mass - B->phys_info.mass) < 1 ) {
			if (A > B) {
				HeavyOne = A;
				LightOne = B;
			} else {
				HeavyOne = B;
				LightOne = A;
			}
		} else {
			if (A->phys_info.mass > B->phys_info.mass) {
				HeavyOne = A;
				LightOne = B;
			} else {
				HeavyOne = B;
				LightOne = A;
			}
		}

		// create ship_ship_or_debris_hit
		// inputs	obj A, obj B
		// outputs	hitpos, impulse (for damage), shield hit tri (for quadrant)
		collision_info_struct ship_ship_hit_info;
		memset(&ship_ship_hit_info, -1, sizeof(collision_info_struct));

		ship_ship_hit_info.heavy = HeavyOne;		// heavy object, generally slower moving
		ship_ship_hit_info.light = LightOne;		// light object, generally faster moving

		vec3d world_hit_pos;

		hit = ship_ship_check_collision(&ship_ship_hit_info, &world_hit_pos);

/*		if ((hitpos.x == FastOne->pos.x) && (hitpos.y == FastOne->pos.y) && (hitpos.z == FastOne->pos.z))
			Int3();
		if ((hitpos.x == SlowOne->pos.x) && (hitpos.y == SlowOne->pos.y) && (hitpos.z == SlowOne->pos.z))
			Int3();
		if ((A == FastOne) && (hitpos.x == FastOne->last_pos.x) && (hitpos.y == FastOne->last_pos.y) && (hitpos.z == FastOne->last_pos.z))
			Int3();
*/
		if ( hit )
		{
			Script_system.SetHookObjects(4, "Ship", A, "ShipB", B, "Self", A, "Object", B);
			bool a_override = Script_system.IsConditionOverride(CHA_COLLIDESHIP, A);
			
			//Yes this should be reversed.
			Script_system.SetHookObjects(4, "Ship", B, "ShipB", A, "Self", B, "Object", A);
			bool b_override = Script_system.IsConditionOverride(CHA_COLLIDESHIP, B);
			if(!a_override && !b_override)
			{
				float		damage;

				if ( player_involved && (Player->control_mode == PCM_WARPOUT_STAGE1) )
				{
					gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
					HUD_printf(XSTR( "Warpout sequence aborted.", 466));
				}

	//			vec3d	rel_vec;

				//	Hack, following line would cause a null vector in vm_vec_normalized_dir below.  This should prevent it.
				// FastOne->pos = FastOne->last_pos;
	//			vm_vec_scale_add2(&FastOne->pos, &FastOne->last_pos, 0.01f);
	//			vm_vec_scale(&FastOne->pos, 1.0f/1.01f);

				//	Amount of damage done by a collision changed by MK, 11/19/96.
				//	Now uses relative velocity and ignores shield of objects.  No reason
				//	smacking into a capital ship should damage you 1000 times as much as
				//	smacking into a fighter.  Depends on your velocity and whether you
				//	smack headon or barely glance.

				// Amount of damage done by a collision changed by DA 08/26/97.
				// Amount of damage now depends on impulse imparted by a collision,
				// scaled by max momentum of a ship, so ramming full speed head on into an
				// immovable object should kill you.
	//			vm_vec_sub(&rel_vec, &B->phys_info.vel, &A->phys_info.vel);
	//			damage = vm_vec_mag_quick(&rel_vec);

	//			float impulse = 0.0f;		// HACK!!! Should be something, right?
				damage = 0.005f * ship_ship_hit_info.impulse;	//	Cut collision-based damage in half.
				//	Decrease heavy damage by 2x.
				if (damage > 5.0f){
					damage = 5.0f + (damage - 5.0f)/2.0f;
				}

				do_kamikaze_crash(A, B);

				if (ship_ship_hit_info.impulse > 0) {
					float	q;

					q = vm_vec_dist_quick(&A->pos, &B->pos) / (A->radius + B->radius);

	#ifndef NDEBUG
	//				//nprintf(("AI", "Frame %i: %s and %s, dam=%7.2f.  dist/rad=%5.2f. Zeroing.\n", Framecount, Ships[A->instance].ship_name, Ships[B->instance].ship_name, damage, q));
	//				if (damage > 5.0f) {
	//					if ( player_involved ) {
	//						object	*other_objp;
	//						float		dot;
	//						vec3d	v2h;
	//
	//						if (A == Player_obj)
	//							other_objp = B;
	//						else
	//							other_objp = A;
	//
	//						vm_vec_normalized_dir(&v2h, &ship_ship_hit_info.hit_pos, &Player_obj->pos);
	//						dot = vm_vec_dot(&Player_obj->orient.fvec, &v2h);
	//					//	HUD_printf("Collision %s: %i%%. (dot=%5.2f), dist ratio=%5.2f", Ships[other_objp->instance].ship_name, (int) (100.0f * damage/Ships[Player_obj->instance].ship_max_hull_strength), dot,
	//					//		vm_vec_dist_quick(&Player_obj->pos, &other_objp->pos) / (Player_obj->radius + other_objp->radius));
	//					}
	//				}
	#endif
					if ( player_involved ) {					
						hud_start_text_flash(XSTR("Collision", 1431), 2000);
					}
				}
				//	damage *= (max_shields of fastest) / (max_impulse_of_fastest)
				// possibly calculate damage both ways and use largest/smallest/avg?

	//			vm_vec_add(&world_hit_pos, &ship_ship_hit_info.heavy->pos, &ship_ship_hit_info.hit_pos);

				collide_ship_ship_do_sound(&world_hit_pos, A, B, player_involved);

				// check if we should do force feedback stuff
				if (player_involved && (ship_ship_hit_info.impulse > 0)) {
					float scaler;
					vec3d v;

					scaler = -ship_ship_hit_info.impulse / Player_obj->phys_info.mass * 300;
					vm_vec_copy_normalize(&v, &world_hit_pos);
					joy_ff_play_vector_effect(&v, scaler);
				}

				//mprintf(("Ship:Ship damage = %7.3f\n", speed));
				#ifndef NDEBUG
				if ( !Collide_friendly ) {
					if ( Ships[A->instance].team == Ships[B->instance].team ) {
						vec3d	collision_vec, right_angle_vec;
						vm_vec_normalized_dir(&collision_vec, &ship_ship_hit_info.hit_pos, &A->pos);
						if (vm_vec_dot(&collision_vec, &A->orient.vec.fvec) > 0.999f){
							right_angle_vec = A->orient.vec.rvec;
						} else {
							vm_vec_cross(&right_angle_vec, &A->orient.vec.uvec, &collision_vec);
						}

						vm_vec_scale_add2( &A->phys_info.vel, &right_angle_vec, +2.0f);
						vm_vec_scale_add2( &B->phys_info.vel, &right_angle_vec, -2.0f);
						//nprintf(("AI", "A: [%6.3f %6.3f %6.3f] B: [%6.3f %6.3f %6.3f]\n", A->phys_info.vel.x, A->phys_info.vel.y, A->phys_info.vel.z, B->phys_info.vel.x, B->phys_info.vel.y, B->phys_info.vel.z));

						return 0;
					}
				}
				#endif

				// nprintf(("AI", "Ship:ship collision: %s and %s.\n", Ships[A->instance].ship_name, Ships[B->instance].ship_name));

				//	Scale damage based on skill level for player.
				if ((LightOne->flags & OF_PLAYER_SHIP) || (HeavyOne->flags & OF_PLAYER_SHIP)) {
					damage *= (float) (Game_skill_level*Game_skill_level+1)/(NUM_SKILL_LEVELS+1);
				} else if (Ships[LightOne->instance].team == Ships[HeavyOne->instance].team) {
					//	Decrease damage if non-player ships and not large.
					//	Looks dumb when fighters are taking damage from bumping into each other.
					if ((LightOne->radius < 50.0f) && (HeavyOne->radius <50.0f)) {
						damage /= 4.0f;
					}
				}
				
				float dam2 = (100.0f * damage/LightOne->phys_info.mass);

				int	quadrant_num = shield_get_quadrant_global(ship_ship_hit_info.heavy, &world_hit_pos);
				//nprintf(("AI", "Ship %s hit in quad #%i\n", Ships[ship_ship_hit_info.heavy->instance].ship_name, quadrant_num));
				if ((ship_ship_hit_info.heavy->flags & OF_NO_SHIELDS) || !shield_is_up(ship_ship_hit_info.heavy, quadrant_num) ) {
					quadrant_num = -1;
				}

				ship_apply_local_damage(ship_ship_hit_info.heavy, ship_ship_hit_info.light, &world_hit_pos, 100.0f * damage/HeavyOne->phys_info.mass, quadrant_num, CREATE_SPARKS, ship_ship_hit_info.submodel_num, &ship_ship_hit_info.collision_normal);
				hud_shield_quadrant_hit(ship_ship_hit_info.heavy, quadrant_num);

				//nprintf(("AI", "Ship %s hit in quad #%i\n", Ships[ship_ship_hit_info.light->instance].ship_name, quadrant_num));
				// don't draw sparks (using sphere hitpos)
				ship_apply_local_damage(ship_ship_hit_info.light, ship_ship_hit_info.heavy, &world_hit_pos, dam2, MISS_SHIELDS, NO_SPARKS, -1, &ship_ship_hit_info.collision_normal);
				hud_shield_quadrant_hit(ship_ship_hit_info.light, quadrant_num);

				maybe_push_little_ship_from_fast_big_ship(ship_ship_hit_info.heavy, ship_ship_hit_info.light, ship_ship_hit_info.impulse, &ship_ship_hit_info.collision_normal);
				//nprintf(("AI", "Damage to %s = %7.3f\n", Ships[LightOne->instance].ship_name, dam2));
			}

			if(!(b_override && !a_override))
			{
				Script_system.SetHookObjects(4, "Ship", A, "ShipB", B, "Self", A, "Object", B);
				Script_system.RunCondition(CHA_COLLIDESHIP, '\0', NULL, A);
			}
			if((b_override && !a_override) || (!b_override && !a_override))
			{
				//Yes this should be reversed.
				Script_system.SetHookObjects(4, "Ship", B, "ShipB", A, "Self", B, "Object", A);
				Script_system.RunCondition(CHA_COLLIDESHIP, '\0', NULL, B);
			}

			Script_system.RemHookVars(4, "Ship", "ShipB", "Self", "Object");

			return 0;
		}		
	}
	else
	{
		// estimate earliest time at which pair can hit

		// cap ships warping in/out can exceed ship's expected velocity
		// if ship is warping in, in stage 1, its velocity is 0, so make ship try to collide next frame
		int sif_a_flags, sif_b_flags;
		sif_a_flags = Ship_info[Ships[A->instance].ship_info_index].flags;
		sif_b_flags = Ship_info[Ships[B->instance].ship_info_index].flags;

		// if ship is huge and warping in
		if ( ((Ships[A->instance].flags & SF_ARRIVING_STAGE_1) && (sif_a_flags & SIF_HUGE_SHIP))
			|| ((Ships[B->instance].flags & SF_ARRIVING_STAGE_1) && (sif_b_flags & SIF_HUGE_SHIP)) ) {
			pair->next_check_time = timestamp(0);	// check next time
			return 0;
		}

		// get max of (1) max_vel.z, (2) 10, (3) afterburner_max_vel.z, (4) vel.z (for warping in ships exceeding expected max vel)
		float shipA_max_speed, shipB_max_speed, time;

		// get shipA max speed
		if (ship_is_beginning_warpout_speedup(A)) {
			shipA_max_speed = MAX(ship_get_max_speed(&Ships[A->instance]), ship_get_warpout_speed(A));
		} else {
			shipA_max_speed = ship_get_max_speed(&Ships[A->instance]);
		}

		// Maybe warping in or finished warping in with excessive speed
		shipA_max_speed = MAX(shipA_max_speed, vm_vec_mag(&A->phys_info.vel));
		shipA_max_speed = MAX(shipA_max_speed, 10.0f);

		// get shipB max speed
		if (ship_is_beginning_warpout_speedup(B)) {
			shipB_max_speed = MAX(ship_get_max_speed(&Ships[B->instance]), ship_get_warpout_speed(B));
		} else {
			shipB_max_speed = ship_get_max_speed(&Ships[B->instance]);
		}

		// Maybe warping in or finished warping in with excessive speed
		shipB_max_speed = MAX(shipB_max_speed, vm_vec_mag(&B->phys_info.vel));
		shipB_max_speed = MAX(shipB_max_speed, 10.0f);

		time = 1000.0f * (dist - A->radius - B->radius) / (shipA_max_speed + shipB_max_speed);
		time -= 200.0f;		// allow one frame slow frame at ~5 fps

		if (time > 0) {
			pair->next_check_time = timestamp( fl2i(time) );
		} else {
			pair->next_check_time = timestamp(0);	// check next time
		}
	}
	
	return 0;
}

void collect_ship_ship_physics_info(object *heavy, object *light, mc_info *mc_info, collision_info_struct *ship_ship_hit_info)
{
	// slower moving object [A] is checked at its final position (polygon and position is found on obj)
	// faster moving object [B] is reduced to a point and a ray is drawn from its last_pos to pos
	// collision code returns hit position and normal on [A]

	// estimate location on B that contacts A
	// first find orientation of B relative to the normal it collides against.
	// then find an approx hit location using the position hit on the bounding box

	vec3d *r_heavy = &ship_ship_hit_info->r_heavy;
	vec3d *r_light = &ship_ship_hit_info->r_light;
	vec3d *heavy_collide_cm_pos = &ship_ship_hit_info->heavy_collision_cm_pos;
	vec3d *light_collide_cm_pos = &ship_ship_hit_info->light_collision_cm_pos;

	float core_rad = model_get_core_radius(Ship_info[Ships[light->instance].ship_info_index].model_num);

	// get info needed for ship_ship_collision_physics
	Assert(mc_info->hit_dist > 0);

	// get light_collide_cm_pos
	if ( !ship_ship_hit_info->submodel_rot_hit ) {
		vec3d displacement;
		vm_vec_sub(&displacement, mc_info->p1, mc_info->p0);

		*light_collide_cm_pos = *mc_info->p0;
		vm_vec_scale_add2(light_collide_cm_pos, &displacement, ship_ship_hit_info->hit_time);
	}
	
	// get r_light
	vm_vec_sub(r_light, &ship_ship_hit_info->hit_pos, light_collide_cm_pos);

//	Assert(vm_vec_mag(&r_light) > core_rad - 0.1);
	float mag = float(fabs(vm_vec_mag(r_light) - core_rad));
	if (mag > 0.1) {
		nprintf(("Physics", "Framecount: %i |r_light - core_rad| > 0.1)\n", Framecount));
	}

	if (ship_ship_hit_info->edge_hit) {
	// For an edge hit, just take the closest valid plane normal as the collision normal
		vm_vec_copy_normalize(&ship_ship_hit_info->collision_normal, r_light);
		vm_vec_negate(&ship_ship_hit_info->collision_normal);
	}

	// r dot n may not be negative if hit by moving model parts.
	float dot = vm_vec_dotprod( r_light, &ship_ship_hit_info->collision_normal );
	if ( dot > 0 )
	{
		nprintf(("Physics", "Framecount: %i r dot normal  > 0\n", Framecount, dot));
	}

	vm_vec_zero(heavy_collide_cm_pos);

	float q = vm_vec_dist(heavy_collide_cm_pos, light_collide_cm_pos) / (heavy->radius + core_rad);
	if (q > 1.0f) {
		nprintf(("Physics", "Warning: q = %f.  Supposed to be <= 1.0.\n", q));
	}

	*r_heavy = ship_ship_hit_info->hit_pos;

	// fill in ship_ship_hit_info
//	ship_ship_hit_info->heavy_collision_cm_pos = heavy_collide_cm_pos;
//	ship_ship_hit_info->light_collision_cm_pos = light_collide_cm_pos;
//	ship_ship_hit_info->r_heavy = r_heavy;
//	ship_ship_hit_info->r_light = r_light;

// sphere_sphere_case_handled separately
#ifdef COLLIDE_DEBUG
nprintf(("Physics", "Frame: %i %s info: last_pos: [%4.1f, %4.1f, %4.1f], collide_pos: [%4.1f, %4.1f %4.1f] vel: [%4.1f, %4.1f %4.1f]\n",
	Framecount, Ships[heavy->instance].ship_name, heavy->last_pos.x, heavy->last_pos.y, heavy->last_pos.z,
	heavy_collide_cm_pos.x, heavy_collide_cm_pos.y, heavy_collide_cm_pos.z,
	heavy->phys_info.vel.x, heavy->phys_info.vel.y, heavy->phys_info.vel.z));

nprintf(("Physics", "Frame: %i %s info: last_pos: [%4.1f, %4.1f, %4.1f], collide_pos: [%4.1f, %4.1f, %4.1f] vel: [%4.1f, %4.1f, %4.1f]\n",
	Framecount, Ships[light->instance].ship_name, light->last_pos.x, light->last_pos.y, light->last_pos.z,
	light_collide_cm_pos.x, light_collide_cm_pos.y, light_collide_cm_pos.z,
	light->phys_info.vel.x, light->phys_info.vel.y, light->phys_info.vel.z));
#endif

}	

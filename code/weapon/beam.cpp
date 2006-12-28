/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Beam.cpp $
 * $Revision: 2.75 $
 * $Date: 2006-12-28 00:59:53 $
 * $Author: wmcoolmon $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.74  2006/11/06 06:36:44  taylor
 * updated/fixed modelanim code
 *
 * Revision 2.73  2006/10/08 02:05:38  Goober5000
 * fix forum links
 *
 * Revision 2.72  2006/09/11 05:38:16  taylor
 * don't try to draw beam sections if there is no valid texture for it, or if there is no width
 *
 * Revision 2.71  2006/07/24 07:36:50  taylor
 * minor cleanup/optimization to beam warmup glow rendering function
 * various lighting code cleanups
 *  - try to always make sure beam origin lights occur outside of model
 *  - make Static_lights[] dynamic
 *  - be sure to reset to first 8 lights when moving on to render spec related texture passes
 *  - add ambient color to point lights (helps warp effects)
 *  - sort lights to try and get more important and/or visible lights to always happen in initial render pass
 *
 * Revision 2.70  2006/07/05 23:36:07  Goober5000
 * cvs comment tweaks
 *
 * Revision 2.69  2006/06/10 21:31:14  wmcoolmon
 * Oh wait, I can just do this
 *
 * Revision 2.68  2006/06/10 21:22:57  wmcoolmon
 * Fix attempt #2 for mantis bug 0000900
 *
 * Revision 2.67  2006/05/27 16:45:11  taylor
 * some minor cleanup
 * remove -nobeampierce
 * update for geometry batcher changes
 *
 * Revision 2.66  2006/02/25 21:47:19  Goober5000
 * spelling
 *
 * Revision 2.65  2006/01/15 18:56:45  taylor
 * another NULL VEC3D warning fix
 *
 * Revision 2.64  2005/12/29 08:08:42  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.63  2005/12/14 08:07:33  phreak
 * Handling of exit explosions for beams when a ship is about to get destroyed, or according to the code: "tooled".
 * Bumped beam frame collisions to 10 to compensate.
 *
 * Revision 2.62  2005/11/21 02:43:30  Goober5000
 * change from "setting" to "profile"; this way makes more sense
 * --Goober5000
 *
 * Revision 2.61  2005/11/21 00:46:05  Goober5000
 * add ai_settings.tbl
 * --Goober5000
 *
 * Revision 2.60  2005/11/08 01:04:02  wmcoolmon
 * More warnings instead of Int3s/Asserts, better Lua scripting, weapons_expl.tbl is no longer needed nor read, added "$Disarmed ImpactSnd:", fire-beam fix
 *
 * Revision 2.59  2005/10/10 17:21:11  taylor
 * remove NO_NETWORK
 *
 * Revision 2.58  2005/09/25 08:25:14  Goober5000
 * Okay, everything should now work again. :p Still have to do a little more with the asteroids.
 * --Goober5000
 *
 * Revision 2.57  2005/09/03 17:48:21  phreak
 * don't wrap the U_offset variable since it causes the beam to jump once it gets reset.
 *
 * Revision 2.56  2005/07/24 18:35:44  taylor
 * proper multi support for fighter beams, already has code to break protocol commented out if we need it,
 *   decided to skip and beam type checks and let beam_fire() reassign it (should work ok)
 *
 * Revision 2.55  2005/07/22 10:18:37  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.54  2005/07/22 03:54:46  taylor
 * better error checking/handling for when you fire primary/beam weapons without a target selected
 *
 * Revision 2.53  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.52  2005/07/12 21:01:40  Goober5000
 * properly fixed beam_apply_whack()
 * --Goober5000
 *
 * Revision 2.50  2005/04/28 05:29:30  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 2.49  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.48  2005/03/08 03:50:17  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 2.47  2005/03/03 16:40:28  taylor
 * animated beam muzzle glows
 *
 * Revision 2.46  2005/03/02 21:24:48  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.45  2005/01/17 23:35:45  argv
 * Surface shields.
 *
 * See forum thread:
 * http://www.hard-light.net/forums/index.php/topic,29643.0.html
 *
 * -- _argv[-1]
 *
 * Revision 2.44  2005/01/14 23:08:17  argv
 * Revision 2.42 (Goober's fixes) broke "no pierce shields". Fixed.
 *
 * -- _argv[-1]
 *
 * Revision 2.43  2005/01/11 21:38:49  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.42  2005/01/02 23:22:22  Goober5000
 * got rid of a nasty nasty bug in the beam collision function, and cleaned up some of Bobboau's old comments
 * --Goober5000
 *
 * Revision 2.41  2004/11/21 11:38:17  taylor
 * support for animated beam sections
 * various weapon-only-used fixes
 * remove the -1 frame fix since it was fixed elsewhere
 *
 * Revision 2.40  2004/07/26 20:47:56  Kazan
 * remove MCD complete
 *
 * Revision 2.39  2004/07/12 16:33:09  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.38  2004/04/06 05:42:49  Goober5000
 * ugh - added back in something that Bobboau removed
 * -Goober5000
 *
 * Revision 2.37  2004/04/03 02:55:50  bobboau
 * commiting recent minor bug fixes
 *
 * Revision 2.36  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.35  2004/03/05 09:01:53  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.34  2004/02/07 00:48:53  Goober5000
 * made FS2 able to account for subsystem mismatches between ships.tbl and the
 * model file - e.g. communication vs. communications
 * --Goober5000
 *
 * Revision 2.33  2003/11/17 04:25:58  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.32  2003/11/16 04:09:20  Goober5000
 * language
 *
 * Revision 2.31  2003/11/11 03:56:13  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.30  2003/11/11 02:15:41  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.29  2003/11/09 07:36:52  Goober5000
 * fixed spelling
 * --Goober5000
 *
 * Revision 2.28  2003/10/23 18:03:25  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.27  2003/10/22 23:10:14  phreak
 * commented out some mprintfs that kazan missed
 *
 * Revision 2.26  2003/10/13 05:57:50  Kazan
 * Removed a bunch of Useless *_printf()s in the rendering pipeline that were just slowing stuff down
 * Commented out the "warning null vector in vector normalize" crap
 * Added "beam no whack" flag for beams - said beams NEVER whack
 * Some reliability updates in FS2NetD
 *
 * Revision 2.25  2003/09/13 06:02:04  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.23  2003/08/21 15:04:17  phreak
 * zeroed out the specular fields since they caused some flickering
 *
 * Revision 2.22  2003/07/16 23:12:44  phreak
 * fixed a dumb bug
 *
 * Revision 2.21  2003/07/15 02:39:59  phreak
 * fixed problem where beams that ignore shields do not fully damage the target
 * disabled attenuation for beams, pending rework
 *
 * Revision 2.20  2003/05/04 20:21:00  phreak
 * beams can't fire when disrupted.
 * cycle ends if the beam is firing and shooting ship is disrupted
 *
 * Revision 2.19  2003/04/29 01:03:22  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.18  2003/03/29 09:42:05  Goober5000
 * made beams default shield piercing again
 * also added a beam no pierce command line flag
 * and fixed something else which I forgot :P
 * --Goober5000
 *
 * Revision 2.17  2003/03/20 23:20:46  Goober5000
 * comments
 * --Goober500
 *
 * Revision 2.16  2003/03/02 17:19:45  Goober5000
 * commented an unneeded variable
 * -- Goober5000
 *
 * Revision 2.15  2003/02/26 02:56:55  bobboau
 * fixed the bug with fighter beams not giveing you kills
 *
 * Revision 2.14  2003/02/25 07:15:07  bobboau
 * doh... forgot something in the beam rendering code
 *
 * Revision 2.13  2003/02/25 06:22:50  bobboau
 * fixed a bunch of fighter beam bugs,
 * most notabley the sound now works corectly,
 * and they have limeted range with atenuated damage (table option)
 * added bank specific compatabilities
 *
 * Revision 2.12  2003/02/16 18:55:59  phreak
 * fixed typecasting warnings
 *
 * Revision 2.11  2003/02/16 05:14:29  bobboau
 * added glow map nebula bug fix for d3d, someone should add a fix for glide too
 * more importantly I (think I) have fixed all major bugs with fighter beams, and added a bit of new functionality
 *
 * Revision 2.10  2003/01/19 01:07:43  bobboau
 * redid the way glow maps are handled; you now must set a global variable before you render a poly that uses a glow map, then set it to -1 when you're done with it
 * fixed a few other misc bugs too
 *
 * Revision 2.9  2003/01/13 23:20:01  Goober5000
 * bug hunting; fixed the beam whack effect bug
 * --Goober5000
 *
 * Revision 2.8  2002/12/27 17:58:11  Goober5000
 * the insidious beam bug is now squashed AND committed!
 * --Goober5000
 *
 * Revision 2.7  2002/12/20 00:50:41  DTP
 * DTP FIX, fast fix to the beam/shield bug problem. will maybe commit later when i find out why this is getting triggered when it should not
 *
 * Revision 2.6  2002/12/07 01:37:43  bobboau
 * initial decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.5  2002/11/14 04:18:17  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * Revision 2.4  2002/10/19 19:29:29  bobboau
 * initial commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.3  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/26 03:11:24  wmcoolmon
 * Added Bobboau's beam tiling code
 *
 * Revision 2.1  2002/07/25 04:50:48  wmcoolmon
 * Added Bobboau's fighter-beam code.
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/13 21:09:29  mharris
 * I think the last of the networking code has ifndef NO_NETWORK...
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 68    9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 67    9/09/99 2:36p Mikek
 * Put back in the "1.0f +" in BEAM_TYPE_A aiming.  Not the best way to
 * correct this.
 * 
 * 65    9/08/99 10:29p Dave
 * Make beam sound pausing and unpausing much safer.
 * 
 * 64    9/06/99 12:46a Andsager
 * Add weapon_explosion_ani LOD
 * 
 * 63    9/03/99 5:12p Mikek
 * Change miss_factor code, making it a lot more likely for beams to miss
 * and also making them increasingly likely to miss with each subsequent
 * shot in the multi-shot burst.
 * 
 * 62    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 61    8/28/99 7:29p Dave
 * Fixed wingmen persona messaging. Make sure locked turrets don't count
 * towards the # attacking a player.
 * 
 * 60    8/27/99 9:07p Dave
 * LOD explosions. Improved beam weapon accuracy.
 * 
 * 59    8/26/99 10:15a Dave
 * Don't apply beam whacks to docked ships.
 * 
 * 58    7/31/99 1:16p Dave
 * Use larger font for 1024 HUD flash text box. Make beam weapons aware of
 * weapon subsystem damage on firing ship.
 * 
 * 57    7/22/99 4:00p Dave
 * Fixed beam weapon muzzle glow rendering. Externalized hud shield info.
 * 
 * 56    7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 55    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 54    7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 53    7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 52    7/02/99 10:51p Dave
 * Limit friendly beam fire damage. :(
 * 
 * 51    7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 50    6/29/99 7:39p Dave
 * Lots of small bug fixes.
 * 
 * 49    6/29/99 2:53p Dave
 * Re-enabled beam lighting.
 * 
 * 48    6/25/99 3:04p Dave
 * Removed Int3().
 * 
 * 47    6/24/99 3:00p Dave
 * Stupid bug.
 * 
 * 46    6/23/99 4:49p Dave
 * Temporarily removed beam lighting.
 * 
 * 45    6/21/99 7:25p Dave
 * netplayer pain packet. Added type E unmoving beams.
 * 
 * 44    6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 43    6/14/99 10:45a Dave
 * Made beam weapons specify accuracy by skill level in the weapons.tbl
 * 
 * 42    6/09/99 2:55p Andsager
 * Allow multiple asteroid subtypes (of large, medium, small) and follow
 * family.
 * 
 * 41    6/08/99 6:02p Jamesa
 * Handle case where type B beam start and end directions are really
 * close.
 * 
 * 40    6/04/99 2:16p Dave
 * Put in shrink effect for beam weapons.
 * 
 * 39    5/14/99 11:47a Andsager
 * Added  beam_get_weapon_info_index(object *bm)
 * 
 * 38    5/12/99 10:43a Andsager
 * Increase max beam length
 * 
 * 37    5/08/99 8:25p Dave
 * Upped object pairs. First run of nebula lightning.
 * 
 * 36    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 35    5/03/99 9:07a Dave
 * Pirate Bob. Changed beam test code a bit.
 * 
 * 34    4/27/99 12:16a Dave
 * Fixed beam weapon muzzle glow problem. Fixed premature timeout on the
 * pxo server list screen. Fixed secondary firing for hosts on a
 * standalone. Fixed wacky multiplayer weapon "shuddering" problem.
 * 
 * 33    4/25/99 6:12p Johnson
 * Fixed bug where multi-shot beams were getting the shot count from the
 * wrong place.
 * 
 * 32    4/25/99 3:36p Dave
 * Fixed nebula table code. Tweaked beam weapon explosion stuff.
 * 
 * 31    4/23/99 2:33p Johnson
 * Allow beams to shoot at missiles properly.
 * 
 * 30    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 29    4/22/99 11:06p Dave
 * Final pass at beam weapons. Solidified a lot of stuff. All that remains
 * now is to tweak and fix bugs as they come up. No new beam weapon
 * features.
 * 
 * 28    4/21/99 6:15p Dave
 * Did some serious housecleaning in the beam code. Made it ready to go
 * for anti-fighter "pulse" weapons. Fixed collision pair creation. Added
 * a handy macro for recalculating collision pairs for a given object.
 * 
 * 27    4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 26    4/19/99 11:01p Dave
 * More sophisticated targeting laser support. Temporary checkin.
 * 
 * 25    4/19/99 4:54p Johnson
 * Removed infinite loop from beam_render_all()   oops :)
 * 
 * 24    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 23    4/04/99 2:13p Dave
 * Put in area effect beam weapons. May be a bit too expensive though.
 * Dave
 * 22    4/02/99 9:55a Dave
 * Added a few more options in the weapons.tbl for beam weapons. Attempt
 * at putting "pain" packets into multiplayer.
 * 
 * 21    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 20    3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 19    3/04/99 6:09p Dave
 * Added in sexpressions for firing beams and checking for if a ship is
 * tagged.
 * 
 * 18    3/02/99 9:25p Dave
 * Added a bunch of model rendering debug code. Started work on fixing
 * beam weapon wacky firing.
 * 
 * 17    2/21/99 1:48p Dave
 * Some code for monitoring datarate for multiplayer in detail.
 * 
 * 16    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 15    2/05/99 3:23p Mattf
 * Made beams a little more forgiving when checking object types.
 * 
 * 14    2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 13    2/04/99 6:29p Dave
 * First full working rev of FS2 PXO support.  Fixed Glide lighting
 * problems.
 * 
 * 12    1/30/99 9:02p Dave
 * Fixed neb stuff.
 * 
 * 11    1/30/99 1:29a Dave
 * Fixed nebula thumbnail problem. Full support for 1024x768 choose pilot
 * screen.  Fixed beam weapon death messages.
 * 
 * 10    1/29/99 7:08p Johnson
 * Put in fix for beam weapons which are on dying ships.
 * 
 * 9     1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 8     1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 7     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 6     1/21/99 2:06p Dave
 * Final checkin for multiplayer testing.
 * 
 * 5     1/21/99 10:45a Dave
 * More beam weapon stuff. Put in warmdown time.
 * 
 * 4     1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 3     1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 2     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 
 * $NoKeywords: $
 */


#include "weapon/beam.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "object/objcollide.h"
#include "ship/ship.h"
#include "freespace2/freespace.h"
#include "render/3d.h"
#include "io/timer.h"
#include "debris/debris.h"
#include "asteroid/asteroid.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "particle/particle.h"
#include "ship/shiphit.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudmessage.h"
#include "lighting/lighting.h"
#include "hud/hudshield.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "parse/parselo.h"



extern int Cmdline_nohtl;
// ------------------------------------------------------------------------------------------------
// BEAM WEAPON DEFINES/VARS
//

// this is the constant which defines when a beam is an "area" beam. meaning, when we switch on sphereline checking and when 
// a beam gets "stopped" by an object. It is a percentage of the object radius which the beam must be wider than
#define BEAM_AREA_PERCENT			0.4f

// randomness factor - all beam weapon aiming is adjusted by +/- some factor within this range
#define BEAM_RANDOM_FACTOR			0.4f

#define MAX_BEAMS					500
#define BEAM_DAMAGE_TIME			170			// apply damage 
#define MAX_SHOT_POINTS				30
#define SHOT_POINT_TIME				200			// 5 arcs a second

#define TOOLTIME						1500.0f

// max # of collisions we'll allow per frame
#define MAX_FRAME_COLLISIONS		10

// beam flag defines
#define BF_SAFETY						(1<<0)		// if this is set, don't collide or render for this frame. lifetime still increases though
#define BF_SHRINK						(1<<1)		// if this is set, the beam is in the warmdown phase

// beam struct (the actual weapon/object)
typedef struct beam {
	// low-level data
	int		objnum;					// our own objnum
	int		weapon_info_index;
	int		sig;						// signature for the shooting object
	object	*objp;					// the shooting object (who owns the turret that I am being fired from)
	object	*target;					// target object
	ship_subsys *target_subsys;	// targeted subsys
	int		target_sig;				// target sig
	ship_subsys *subsys;				// subsys its being fired from
	beam		*next, *prev;			// link list stuff
	vec3d	targeting_laser_offset;
	int		framecount;				// how many frames the beam has been active
	int		flags;					// see BF_* defines 	
	float		shrink;					// shrink factor	

	// beam info	
	int		warmup_stamp;			// timestamp for "warming up"
	int		warmdown_stamp;		// timestamp for "warming down"
	int		type;						// see BEAM_TYPE_* defines in beam.h
	float		life_left;				// in seconds
	float		life_total;				// total life	
	// this vector has very special meaning. BEFORE performing collision checks, it basically implies a "direction". meaning
	// the vector between it and last_start is where the beam will be aiming. AFTER performing collision checks, it is the
	// literal world collision point on the object (or meaningless, if we hit nothing). The function beam_move_all_pre() is
	// responsible for then setting it up pre-collision time
	vec3d	last_shot;				
	vec3d	last_start;				
	int		shot_index;				// for type D beam weapons

	// recent collisions
	beam_collision r_collisions[MAX_FRAME_COLLISIONS];					// recent collisions
	int r_collision_count;														// # of recent collisions

	// collision info for this frame
	beam_collision f_collisions[MAX_FRAME_COLLISIONS];					// collisions for the current frame
	int f_collision_count;														// # of collisions we recorded this frame

	// looping sound info, HANDLE
	int		beam_sound_loop;		// -1 if none

	// team 
	char		team;

	float range;
	float damage_threshold;

	// exactly how the beam will behave. by passing this is multiplayer from server to client, we can ensure that
	// everything looks the same
	beam_info binfo;
	bool fighter_beam;
	int bank;

	int Beam_muzzle_stamp;
	vec3d local_pnt;
} beam;

beam Beams[MAX_BEAMS];				// all beams
beam Beam_free_list;					// free beams
beam Beam_used_list;					// used beams
int Beam_count = 0;					// how many beams are in use

// octant indices. These are "good" pairs of octants to use for beam target
#define BEAM_NUM_GOOD_OCTANTS			8
int Beam_good_slash_octants[BEAM_NUM_GOOD_OCTANTS][4] = {		
	{ 2, 5, 1, 0 },					// octant, octant, min/max pt, min/max pt
	{ 7, 0, 1, 0 },
	{ 1, 6, 1, 0 },					
	{ 6, 1, 0, 1 },
	{ 5, 2, 0, 1 },	
	{ 0, 7, 0, 1 },		
	{ 7, 1, 1, 0 },
	{ 6, 0, 1, 0 },
};
int Beam_good_shot_octants[BEAM_NUM_GOOD_OCTANTS][4] = {		
	{ 5, 0, 1, 0 },					// octant, octant, min/max pt, min/max pt
	{ 7, 2, 1, 0 },
	{ 7, 1, 1, 0 },					
	{ 6, 0, 1, 0 },
	{ 7, 3, 1, 0 },	
	{ 6, 2, 1, 0 },
	{ 5, 1, 1, 0 },
	{ 4, 0, 1, 0 },
};

// beam lighting effects
int Beam_lighting = 1;

// debug stuff - keep track of how many collision tests we perform a second and how many we toss a second
#define BEAM_TEST_STAMP_TIME		4000	// every 4 seconds
int Beam_test_stamp = -1;
int Beam_test_ints = 0;
int Beam_test_ship = 0;
int Beam_test_ast = 0;
int Beam_test_framecount = 0;

// beam warmup completion %
#define BEAM_WARMUP_PCT(b)			( ((float)Weapon_info[b->weapon_info_index].b_info.beam_warmup - (float)timestamp_until(b->warmup_stamp)) / (float)Weapon_info[b->weapon_info_index].b_info.beam_warmup ) 

// beam warmdown completion %		
#define BEAM_WARMDOWN_PCT(b)		( ((float)Weapon_info[b->weapon_info_index].b_info.beam_warmdown - (float)timestamp_until(b->warmdown_stamp)) / (float)Weapon_info[b->weapon_info_index].b_info.beam_warmdown ) 

// timestamp for spewing muzzle particles
//int Beam_muzzle_stamp = -1;

// link into the physics paused system
extern int physics_paused;

// beam lighting info
#define MAX_BEAM_LIGHT_INFO		100
typedef struct beam_light_info {
	beam *bm;					// beam casting the light
	int objnum;					// object getting light cast on it
	ubyte source;				// 0 to light the shooter, 1 for lighting any ship the beam passes, 2 to light the collision ship
	vec3d c_point;			// collision point for type 2 lights
} beam_light_info;

beam_light_info Beam_lights[MAX_BEAM_LIGHT_INFO];
int Beam_light_count = 0;

float b_whack_small = 500.0f;
float b_whack_big = 1500.0f;
float b_whack_damage = 150.0f;
DCF(b_whack_small, "")
{
	dc_get_arg(ARG_FLOAT);
	b_whack_small = Dc_arg_float;
}
DCF(b_whack_big, "")
{
	dc_get_arg(ARG_FLOAT);
	b_whack_big = Dc_arg_float;
}
DCF(b_whack_damage, "")
{
	dc_get_arg(ARG_FLOAT);
	b_whack_damage = Dc_arg_float;
}


// ------------------------------------------------------------------------------------------------
// BEAM WEAPON FORWARD DECLARATIONS
//

// delete a beam
void beam_delete(beam *b);

// handle a hit on a specific object
void beam_handle_collisions(beam *b);

// fills in binfo
void beam_get_binfo(beam *b, float accuracy, int num_shots);

// aim the beam (setup last_start and last_shot - the endpoints). also recalculates object collision info
void beam_aim(beam *b);

// type A functions
void beam_type_a_move(beam *b);

// type B functions
void beam_type_b_move(beam *b);

// type C functions
void beam_type_c_move(beam *b);

// type D functions
void beam_type_d_move(beam *b);
void beam_type_d_get_status(beam *b, int *shot_index, int *fire_wait);

// type e functions
void beam_type_e_move(beam *b);

// given a model #, and an object, stuff 2 good world coord points
void beam_get_octant_points(int modelnum, object *objp, int oct_index, int oct_array[BEAM_NUM_GOOD_OCTANTS][4], vec3d *v1, vec3d *v2);

// given an object, return its model num
int beam_get_model(object *objp);

// for a given object, and a firing beam, determine its critical dot product and range
void beam_get_cull_vals(object *objp, beam *b, float *cull_dot, float *cull_dist);

// get the total possible cone for a given beam in radians
float beam_get_cone_dot(beam *b);

// for rendering the beam effect
// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void beam_calc_facing_pts(vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add);

// render the muzzle glow for a beam weapon
void beam_render_muzzle_glow(beam *b);

// generate particles for the muzzle glow
void beam_generate_muzzle_particles(beam *b);

// throw some jitter into the aim - based upon shot_aim
void beam_jitter_aim(beam *b, float aim);

// if it is legal for the beam to continue firing
// returns -1 if the beam should stop firing immediately
// returns 0 if the beam should go to warmdown
// returns 1 if the beam can continue along its way
int beam_ok_to_fire(beam *b);

// start the warmup phase for the beam
void beam_start_warmup(beam *b);

// start the firing phase for the beam, return 0 if the beam failed to start, and should be deleted altogether
int beam_start_firing(beam *b);

// start the warmdown phase for the beam
void beam_start_warmdown(beam *b);

// add a collision to the beam for this frame (to be evaluated later)
void beam_add_collision(beam *b, object *hit_object, mc_info *cinfo, int quad = -1, int exit_flag = 0);

// sort collisions for the frame
int beam_sort_collisions_func(const void *e1, const void *e2);

// get the width of the widest section of the beam
float beam_get_widest(beam *b);

// mark an object as being lit
void beam_add_light(beam *b, int objnum, int source, vec3d *c_point);

// apply lighting from any beams
void beam_apply_lighting();

// recalculate beam sounds (looping sounds relative to the player)
void beam_recalc_sounds(beam *b);

// apply a whack to a ship
void beam_apply_whack(beam *b, object *objp, vec3d *hit_point);

// return the amount of damage which should be applied to a ship. basically, filters friendly fire damage 
float beam_get_ship_damage(beam *b, object *objp);

// if the beam is likely to tool a given target before its lifetime expires
int beam_will_tool_target(beam *b, object *objp);


// ------------------------------------------------------------------------------------------------
// BEAM WEAPON FUNCTIONS
//

// init at game startup
void beam_init()
{
	// clear the beams
	list_init( &Beam_free_list );
	list_init( &Beam_used_list );
}

// initialize beam weapons for this level
void beam_level_init()
{
	// intialize beams
	int idx;

	Beam_count = 0;
	list_init( &Beam_free_list );
	list_init( &Beam_used_list );
	memset(Beams, 0, sizeof(beam) * MAX_BEAMS);

	// Link all object slots into the free list
	for (idx=0; idx<MAX_BEAMS; idx++)	{
		Beams[idx].objnum = -1;
		list_append(&Beam_free_list, &Beams[idx] );
	}

	// reset muzzle particle spew timestamp
}

// shutdown beam weapons for this level
void beam_level_close()
{
	// clear the beams
	list_init( &Beam_free_list );
	list_init( &Beam_used_list );
}

// fire a beam, returns nonzero on success. the innards of the code handle all the rest, foo
int beam_fire(beam_fire_info *fire_info)
{
	beam *new_item;
	weapon_info *wip;
	ship *firing_ship;
	int objnum;		

	// sanity check
	if(fire_info == NULL){
		Int3();
		return -1;
	}

	// if we're out of beams, bail
	if(Beam_count >= MAX_BEAMS){
		return -1;
	}

	// for now, only allow ship targets
	if (!fire_info->fighter_beam) {
		if((fire_info->target == NULL) || ((fire_info->target->type != OBJ_SHIP) && (fire_info->target->type != OBJ_ASTEROID) && (fire_info->target->type != OBJ_DEBRIS) && (fire_info->target->type != OBJ_WEAPON))){
			return -1;
		}
	}

	// make sure the beam_info_index is valid
	Assert((fire_info->beam_info_index >= 0) && (fire_info->beam_info_index < MAX_WEAPON_TYPES) && (Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM));
	if((fire_info->beam_info_index < 0) || (fire_info->beam_info_index >= MAX_WEAPON_TYPES) || !(Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM)){
		return -1;
	}

	wip = &Weapon_info[fire_info->beam_info_index];	
	// make sure a ship is firing this
	Assert((fire_info->shooter->type == OBJ_SHIP) && (fire_info->shooter->instance >= 0) && (fire_info->shooter->instance < MAX_SHIPS));
	if((fire_info->shooter->type != OBJ_SHIP) || (fire_info->shooter->instance < 0) && (fire_info->shooter->instance >= MAX_SHIPS)){
		return -1;
	}
	firing_ship = &Ships[fire_info->shooter->instance];

	// get a free beam
	new_item = GET_FIRST(&Beam_free_list);
	Assert( new_item != &Beam_free_list );		// shouldn't have the dummy element
	if(new_item == &Beam_free_list){
		return -1;
	}

	// remove from the free list
	list_remove( &Beam_free_list, new_item );
	
	// insert onto the end of used list
	list_append( &Beam_used_list, new_item );

	// increment counter
	Beam_count++;	

	// fill in some values
	new_item->warmup_stamp = -1;
	new_item->warmdown_stamp = -1;
	new_item->weapon_info_index = fire_info->beam_info_index;	
	new_item->objp = fire_info->shooter;
	new_item->sig = fire_info->shooter->signature;
	new_item->subsys = fire_info->turret;	
	new_item->local_pnt = fire_info->turret->system_info->pnt;
	new_item->life_left = wip->b_info.beam_life;	
	new_item->life_total = wip->b_info.beam_life;
	new_item->r_collision_count = 0;
	new_item->f_collision_count = 0;
	new_item->target = fire_info->target;
	new_item->target_subsys = fire_info->target_subsys;
	new_item->target_sig = (fire_info->target != NULL) ? fire_info->target->signature : 0;
	new_item->beam_sound_loop = -1;
	new_item->type = wip->b_info.beam_type;
	new_item->targeting_laser_offset = fire_info->targeting_laser_offset;
	new_item->framecount = 0;
	new_item->flags = 0;
	new_item->shot_index = 0;
	new_item->shrink = 1.0f;	
	new_item->team = (char)firing_ship->team;
	new_item->range = wip->b_info.range;
	new_item->damage_threshold = wip->b_info.damage_threshold;
	new_item->fighter_beam = fire_info->fighter_beam;
	new_item->bank = fire_info->bank;
	new_item->Beam_muzzle_stamp = -1;
	
	if(fire_info->fighter_beam){
		new_item->type = BEAM_TYPE_C;
	}

	// if the targeted subsystem is not NULL, force it to be a type A beam
	if(new_item->target_subsys != NULL && new_item->type != BEAM_TYPE_C){
		new_item->type = BEAM_TYPE_A;
	}

	// type D weapons can only fire at small ships and missiles
	if(new_item->type == BEAM_TYPE_D){
		// if its a targeted ship, get the target ship
		if((fire_info->target != NULL) && (fire_info->target->type == OBJ_SHIP) && (fire_info->target->instance >= 0)){		
			ship *target_ship = &Ships[fire_info->target->instance];
			
			// maybe force to be a type A
			if(Ship_info[target_ship->ship_info_index].class_type > -1 && (Ship_types[Ship_info[target_ship->ship_info_index].class_type].weapon_bools & STI_WEAP_BEAMS_EASILY_HIT)){
				new_item->type = BEAM_TYPE_A;
			}
		}
	}
	
	// ----------------------------------------------------------------------
	// THIS IS THE CRITICAL POINT FOR MULTIPLAYER
	// beam_get_binfo(...) determines exactly how the beam will behave over the course of its life
	// it fills in binfo, which we can pass to clients in multiplayer	
	if(fire_info->beam_info_override != NULL){
		new_item->binfo = *fire_info->beam_info_override;
	} else {
		beam_get_binfo(new_item, fire_info->accuracy, wip->b_info.beam_shots);			// to fill in b_info	- the set of directional aim vectors
	}	

	// create the associated object
	objnum = obj_create(OBJ_BEAM, -1, new_item - Beams, &vmd_identity_matrix, &vmd_zero_vector, 1.0f, OF_COLLIDES);
	if(objnum < 0){
		beam_delete(new_item);
		nprintf(("General", "obj_create() failed for beam weapon! bah!\n"));
		Int3();
		return -1;
	}
	new_item->objnum = objnum;

	// this sets up all info for the first frame the beam fires
	beam_aim(new_item);						// to fill in shot_point, etc.	

	// check to see if its legal to fire at this guy
	if(beam_ok_to_fire(new_item) != 1){
		beam_delete(new_item);
		mprintf(("Killing beam at initial fire because of illegal targeting!!!\n"));
		return -1;
	}

	// if we're a multiplayer master - send a packet
	if(MULTIPLAYER_MASTER){
		send_beam_fired_packet(fire_info->shooter, fire_info->turret, fire_info->target, fire_info->beam_info_index, &new_item->binfo, (ubyte)fire_info->fighter_beam);
	}

	// start the warmup phase
	beam_start_warmup(new_item);		

	return objnum;
}

// fire a targeting beam, returns objnum on success. a much much simplified version of a beam weapon
// targeting lasers last _one_ frame. For a continuous stream - they must be created every frame.
// this allows it to work smoothly in multiplayer (detect "trigger down". every frame just create a targeting laser firing straight out of the
// object. this way you get all the advantages of nice rendering and collisions).
// NOTE : only references beam_info_index and shooter
int beam_fire_targeting(fighter_beam_fire_info *fire_info)
{
	beam *new_item;
	weapon_info *wip;
	int objnum;	
	ship *firing_ship;

	// sanity check
	if(fire_info == NULL){
		Int3();
		return -1;
	}

	// if we're out of beams, bail
	if(Beam_count >= MAX_BEAMS){
		return -1;
	}
	
	// make sure the beam_info_index is valid
	Assert((fire_info->beam_info_index >= 0) && (fire_info->beam_info_index < MAX_WEAPON_TYPES) && (Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM));
	if((fire_info->beam_info_index < 0) || (fire_info->beam_info_index >= MAX_WEAPON_TYPES) || !(Weapon_info[fire_info->beam_info_index].wi_flags & WIF_BEAM)){
		return -1;
	}
	wip = &Weapon_info[fire_info->beam_info_index];	

	// make sure a ship is firing this
	Assert((fire_info->shooter->type == OBJ_SHIP) && (fire_info->shooter->instance >= 0) && (fire_info->shooter->instance < MAX_SHIPS));
	if((fire_info->shooter->type != OBJ_SHIP) || (fire_info->shooter->instance < 0) && (fire_info->shooter->instance >= MAX_SHIPS)){
		return -1;
	}
	firing_ship = &Ships[fire_info->shooter->instance];


	// get a free beam
	new_item = GET_FIRST(&Beam_free_list);
	Assert( new_item != &Beam_free_list );		// shouldn't have the dummy element

	// remove from the free list
	list_remove( &Beam_free_list, new_item );
	
	// insert onto the end of used list
	list_append( &Beam_used_list, new_item );

	// increment counter
	Beam_count++;

	// maybe allocate some extra data based on the beam type
	Assert(wip->b_info.beam_type == BEAM_TYPE_C);
	if(wip->b_info.beam_type != BEAM_TYPE_C){
		return -1;
	}

	// fill in some values
	new_item->warmup_stamp = fire_info->warmup_stamp;
	new_item->warmdown_stamp = fire_info->warmdown_stamp;
	new_item->weapon_info_index = fire_info->beam_info_index;	
	new_item->objp = fire_info->shooter;
	new_item->sig = fire_info->shooter->signature;
	new_item->subsys = NULL;
	new_item->life_left = fire_info->life_left;	
	new_item->life_total = fire_info->life_total;
	new_item->r_collision_count = 0;
	new_item->f_collision_count = 0;
	new_item->target = NULL;
	new_item->target_subsys = NULL;
	new_item->target_sig = 0;	
	new_item->beam_sound_loop = -1;
	new_item->type = BEAM_TYPE_C;	
	new_item->targeting_laser_offset = fire_info->targeting_laser_offset;
	new_item->framecount = 0;
	new_item->flags = 0;
	new_item->shot_index = 0;
	new_item->shrink = 1.0f;	
	new_item->team = (char)firing_ship->team;
	new_item->range = wip->b_info.range;
	new_item->damage_threshold = wip->b_info.damage_threshold;

	// type c is a very special weapon type - binfo has no meaning

	// create the associated object
	objnum = obj_create(OBJ_BEAM, OBJ_INDEX(fire_info->shooter), new_item - Beams, &vmd_identity_matrix, &vmd_zero_vector, 1.0f, OF_COLLIDES);

	if(objnum < 0){
		beam_delete(new_item);
		nprintf(("General", "obj_create() failed for beam weapon! bah!\n"));
		Int3();
		return -1;
	}
	new_item->objnum = objnum;	

	// this sets up all info for the first frame the beam fires
	beam_aim(new_item);						// to fill in shot_point, etc.		

	if(Beams[Objects[objnum].instance].objnum != objnum){
		Int3();
		return -1;
	}

//	Objects[objnum].instance = objnum
	
	return objnum;
}

// return an object index of the guy who's firing this beam
int beam_get_parent(object *bm)
{
	beam *b;

	// get a handle to the beam
	Assert(bm->type == OBJ_BEAM);
	Assert(bm->instance >= 0);	
	if(bm->type != OBJ_BEAM){
		return -1;
	}
	if(bm->instance < 0){
		return -1;
	}
	b = &Beams[bm->instance];

	Assert(b->objp != NULL);
	if(b->objp == NULL){
		return -1;
	}

	// if the object handle is invalid
	if(b->objp->signature != b->sig){
		return -1;
	}
 //comented out to see if this is the weak link in the fighter beam hit recording -Bobboau

	// return the handle
	return OBJ_INDEX(b->objp);
}

// return weapon_info_index of beam
int beam_get_weapon_info_index(object *bm)
{
	Assert(bm->type == OBJ_BEAM);
	if (bm->type != OBJ_BEAM) {
		return -1;
	}

	Assert(bm->instance >= 0 && bm->instance < MAX_BEAMS);
	if (bm->instance < 0) {
		return -1;
	}
//make sure it's returning a valid info index
	Assert((Beams[bm->instance].weapon_info_index > -1) && (Beams[bm->instance].weapon_info_index < Num_weapon_types));

	// return weapon_info_index
	return Beams[bm->instance].weapon_info_index;
}



// given a beam object, get the # of collisions which happened during the last collision check (typically, last frame)
int beam_get_num_collisions(int objnum)
{	
	// sanity checks
	if((objnum < 0) || (objnum >= MAX_OBJECTS)){
		Int3();
		return -1;
	}
	if((Objects[objnum].instance < 0) || (Objects[objnum].instance >= MAX_BEAMS)){
		Int3();
		return -1;
	}
	if(Beams[Objects[objnum].instance].objnum != objnum){
		Int3();
		return -1;
	}

	if(Beams[Objects[objnum].instance].objnum < 0){
		Int3();
		return -1;
	}

	// return the # of recent collisions
	return Beams[Objects[objnum].instance].r_collision_count;
}

// stuff collision info, returns 1 on success
int beam_get_collision(int objnum, int num, int *collision_objnum, mc_info **cinfo)
{
	// sanity checks
	if((objnum < 0) || (objnum >= MAX_OBJECTS)){
		Int3();
		return 0;
	}
	if((Objects[objnum].instance < 0) || (Objects[objnum].instance >= MAX_BEAMS)){
		Int3();
		return 0;
	}
	if((Beams[Objects[objnum].instance].objnum != objnum) || (Beams[Objects[objnum].instance].objnum < 0)){
		Int3();
		return 0;
	}
	if(num >= Beams[Objects[objnum].instance].r_collision_count){
		Int3();
		return 0;
	}

	// return - success
	*cinfo = &Beams[Objects[objnum].instance].r_collisions[num].cinfo;
	*collision_objnum = Beams[Objects[objnum].instance].r_collisions[num].c_objnum;
	return 1;
}

// pause all looping beam sounds
void beam_pause_sounds()
{
	beam *moveup = NULL;

	// set all beam volumes to 0	
	moveup = GET_FIRST(&Beam_used_list);
	if(moveup == NULL){
		return;
	}
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		// set the volume to 0, if he has a looping beam sound
		if(moveup->beam_sound_loop >= 0){
			snd_set_volume(moveup->beam_sound_loop, 0.0f);
		}

		// next beam
		moveup = GET_NEXT(moveup);
	}
}

// unpause looping beam sounds
void beam_unpause_sounds()
{
	beam *moveup = NULL;

	// recalc all beam sounds
	moveup = GET_FIRST(&Beam_used_list);
	if(moveup == NULL){
		return;
	}
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		beam_recalc_sounds(moveup);

		// next beam
		moveup = GET_NEXT(moveup);
	}
}

void beam_get_global_turret_gun_info(object *objp, ship_subsys *ssp, vec3d *gpos, vec3d *gvec, int use_angles, vec3d *targetp, bool fighter_beam){
		ship_get_global_turret_gun_info(objp, ssp, gpos, gvec, use_angles, targetp);
	if(fighter_beam)*gvec = objp->orient.vec.fvec;
}

// -----------------------------===========================------------------------------
// BEAM MOVEMENT FUNCTIONS
// -----------------------------===========================------------------------------

// move a type A beam weapon
void beam_type_a_move(beam *b)
{
	vec3d dir;
	vec3d temp, temp2;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &temp2, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}

	// put the "last_shot" point arbitrarily far away
	vm_vec_sub(&dir, &b->last_shot, &b->last_start);
	vm_vec_normalize_quick(&dir);
	vm_vec_scale_add(&b->last_shot, &b->last_start, &dir, b->range);
	Assert(is_valid_vec(&b->last_shot));
}

// move a type B beam weapon
#define BEAM_T(b)						( ((b->binfo.delta_ang / b->life_total) * (b->life_total - b->life_left)) / b->binfo.delta_ang )
void beam_type_b_move(beam *b)
{		
	vec3d actual_dir;
	vec3d temp, temp2;
	float dot_save;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &temp2, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}	

	// if the two direction vectors are _really_ close together, just use the original direction
	dot_save = vm_vec_dot(&b->binfo.dir_a, &b->binfo.dir_b);
	if((double)dot_save >= 0.999999999){
		actual_dir = b->binfo.dir_a;
	} 
	// otherwise move towards the dir	we calculated when firing this beam	
	else {
		vm_vec_interp_constant(&actual_dir, &b->binfo.dir_a, &b->binfo.dir_b, BEAM_T(b));
	}

	// now recalculate shot_point to be shooting through our new point
	vm_vec_scale_add(&b->last_shot, &b->last_start, &actual_dir, b->range);
	int is_valid = is_valid_vec(&b->last_shot);
	Assert(is_valid);
	if(!is_valid){
		actual_dir = b->binfo.dir_a;
		vm_vec_scale_add(&b->last_shot, &b->last_start, &actual_dir, b->range);
	}
}

// type C functions
void beam_type_c_move(beam *b)
{	
	vec3d temp;

	// ugh
	if(b->objp == NULL){
		Int3();
		return;
	}

	// type c beams only last one frame so we never have to "move" them.			
	temp = b->targeting_laser_offset;
	vm_vec_unrotate(&b->last_start, &temp, &b->objp->orient);
	vm_vec_add2(&b->last_start, &b->objp->pos);	
	vm_vec_scale_add(&b->last_shot, &b->last_start, &b->objp->orient.vec.fvec, b->range);

	Ships[b->objp->instance].weapon_energy -= Weapon_info[b->weapon_info_index].energy_consumed * flFrametime;
}

// type D functions
void beam_type_d_move(beam *b)
{
	int shot_index, fire_wait;
	vec3d temp, temp2, dir;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &temp2, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}	

	// determine what stage of the beam we're in
	beam_type_d_get_status(b, &shot_index, &fire_wait);

	// if we've changed shot index
	if(shot_index != b->shot_index){
		// set the new index
		b->shot_index = shot_index;

		// re-aim
		beam_aim(b);
	}

	// if we're in the fire wait stage
	b->flags &= ~BF_SAFETY;
	if(fire_wait){
		b->flags |= BF_SAFETY;
	}

	// put the "last_shot" point arbitrarily far away
	vm_vec_sub(&dir, &b->last_shot, &b->last_start);
	vm_vec_normalize_quick(&dir);
	vm_vec_scale_add(&b->last_shot, &b->last_start, &dir, b->range);
	Assert(is_valid_vec(&b->last_shot));
}
void beam_type_d_get_status(beam *b, int *shot_index, int *fire_wait)
{	
	float shot_time = b->life_total / (float)b->binfo.shot_count;
	float beam_time = b->life_total - b->life_left;

	// determine what "shot" we're on	
	*shot_index = (int)(beam_time / shot_time);
	Assert(*shot_index < b->binfo.shot_count);
	if(*shot_index >= b->binfo.shot_count){
		*shot_index = b->binfo.shot_count - 1;
	}	

	// determine if its the firing or waiting section of the shot (fire happens first, THEN wait)	
	*fire_wait = 0;
	if(beam_time > ((shot_time * (*shot_index)) + (shot_time * 0.5f))){
		*fire_wait = 1;
	} 	
}

// type e functions
void beam_type_e_move(beam *b)
{
	vec3d temp, turret_norm;	

	// LEAVE THIS HERE OTHERWISE MUZZLE GLOWS DRAW INCORRECTLY WHEN WARMING UP OR DOWN
	// get the "originating point" of the beam for this frame. essentially bashes last_start
	beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &turret_norm, 1, &temp, b->fighter_beam);

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return;
	}	

	// put the "last_shot" point arbitrarily far away
	vm_vec_scale_add(&b->last_shot, &b->last_start, &turret_norm, b->range);	
	Assert(is_valid_vec(&b->last_shot));
}

// pre-move (before collision checking - but AFTER ALL OTHER OBJECTS HAVE BEEN MOVED)
void beam_move_all_pre()
{	
	beam *b;	
	beam *moveup;		

	// zero lights for this frame yet
	Beam_light_count = 0;

	// traverse through all active beams
	moveup = GET_FIRST(&Beam_used_list);
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		// get the beam
		b = moveup;

		// unset collision info
		b->f_collision_count = 0;
		
		if(!physics_paused){
			//WMC - cull beams if the firing ship is dead,
			//and the beam needs the firing ship
			if(b->type != BEAM_TYPE_C && b->sig != b->objp->signature)
			{
					moveup = GET_NEXT(moveup);
					beam_delete(b);
					continue;
			}
			// move the beam
			switch(b->type){
			// type A beam weapons don't move
			case BEAM_TYPE_A :			
				beam_type_a_move(b);
				break;

			// type B beam weapons move across the target somewhat randomly
			case BEAM_TYPE_B :
				beam_type_b_move(b);
				break;				

			// type C beam weapons are attached to a fighter - pointing forward
			case BEAM_TYPE_C:
				beam_type_c_move(b);
				break;

			// type D
			case BEAM_TYPE_D:
				beam_type_d_move(b);
				break;

			// type E
			case BEAM_TYPE_E:
				beam_type_e_move(b);
				break;

			// illegal beam type
			default :
				Int3();
			}
		}

		// next
		moveup = GET_NEXT(moveup);
	}
}

// post-collision time processing for beams
void beam_move_all_post()
{
	beam *moveup;	
	beam *next_one;	
	int bf_status;	
	beam_weapon_info *bwi;

	// traverse through all active beams
	moveup = GET_FIRST(&Beam_used_list);
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		 bwi = &Weapon_info[moveup->weapon_info_index].b_info;

//mprintf(("moveing beam with weapon info index %d, post\n", moveup->weapon_info_index));

		 // check the status of the beam
		bf_status = beam_ok_to_fire(moveup);

		// if we're warming up
		if(moveup->warmup_stamp != -1){
			next_one = GET_NEXT(moveup);			

			// should we be stopping?
			if(bf_status < 0){
//				mprintf(("killing beam becase it isn't ok to be fireing\n"));
				beam_delete(moveup);
			} else {
				// if the warming up timestamp has expired, start firing
				if(timestamp_elapsed(moveup->warmup_stamp)){							
					// start firing
					if(!beam_start_firing(moveup)){
//						mprintf(("killing beam becase it shouldn't have started fireing yet\n"));
						beam_delete(moveup);												
					} 			
					
					// add a muzzle light for the shooter
					beam_add_light(moveup, OBJ_INDEX(moveup->objp), 0, NULL);
				} 
			}

			// next
//			mprintf(("beam is warming up, moveing to next\n"));
			moveup = next_one;
			continue;
		} 
		// if we're warming down
		else if(moveup->warmdown_stamp != -1){			
			next_one = GET_NEXT(moveup);

			// should we be stopping?
			if(bf_status < 0){
//				mprintf(("killing beam becase it isn't ok to fire\n"));
				beam_delete(moveup);
			} else {
				// if we're done warming down, the beam is finished
				if(timestamp_elapsed(moveup->warmdown_stamp)){	
//					mprintf(("euthaniseing beam\n"));
					beam_delete(moveup);				
				}			
			}

			// next
//			mprintf(("beam is warming down, moveing to next\n"));
			moveup = next_one;
			continue;
		}
//		mprintf(("beam is fireing\n"));
		// otherwise, we're firing away.........		

		// add a muzzle light for the shooter
		beam_add_light(moveup, OBJ_INDEX(moveup->objp), 0, NULL);

		// subtract out the life left for the beam
		if(!physics_paused){
			moveup->life_left -= flFrametime;						
		}

		// if we're past the shrink point, start shrinking the beam
		if(moveup->life_left <= (moveup->life_total * bwi->beam_shrink_factor)){
			moveup->flags |= BF_SHRINK;
		}

		// if we're shrinking the beam
		if(moveup->flags & BF_SHRINK){
			moveup->shrink -= bwi->beam_shrink_pct * flFrametime;
			if(moveup->shrink < 0.1f){
				moveup->shrink = 0.1f;
			}
		}		

		// stop shooting?
		if(bf_status <= 0){
			next_one = GET_NEXT(moveup);

			// if beam should abruptly stop
			if(bf_status == -1){
//				mprintf(("beam stoping abruptly\n"));
				beam_delete(moveup);							
			}
			// if the beam should just power down
			else {			
				beam_start_warmdown(moveup);
			}
			
			// next beam
			moveup = next_one;
//			mprintf(("beam stopping\n"));
			continue;
		}				

		// increment framecount
		moveup->framecount++;		
//		mprintf(("frame %d\n", moveup->framecount));
		// type c weapons live for one frame only
/*		if(moveup->type == BEAM_TYPE_C){
			if(moveup->framecount > 1){
				next_one = GET_NEXT(moveup);
				beam_delete(moveup);							
//			mprintf(("type c beams only live for one frame\n"));
				moveup = next_one;
				continue;
			}
		}
		// done firing, so go into the warmdown phase
		else*/ {
			if((moveup->life_left <= 0.0f) && (moveup->warmdown_stamp == -1)){
				beam_start_warmdown(moveup);
				
				moveup = GET_NEXT(moveup);	
//				mprintf(("warming beam down\n"));
				continue;
			}				
		}	
//		mprintf(("starting collisions\n"));

		// handle any collisions which occured collision (will take care of applying damage to all objects which got hit)
		beam_handle_collisions(moveup);						

//		mprintf(("recalcing sounds\n"));
		// recalculate beam sounds
		beam_recalc_sounds(moveup);

		// next item
		moveup = GET_NEXT(moveup);
//		mprintf(("moved, getting next\n"));
	}

	// apply all beam lighting
//	mprintf(("applying light\n"));
	beam_apply_lighting();

	// process beam culling info
#ifndef NDEBUG
	/*
	if(Beam_test_stamp == -1){
		Beam_test_stamp = timestamp(BEAM_TEST_STAMP_TIME);
		Beam_test_ints = 0;
		Beam_test_framecount = 0;
	} else {
		if(timestamp_elapsed(Beam_test_stamp)){			
			// report the results
			nprintf(("General", "Performed %f beam ints/frame (%d, %d, %d, %d), over %f seconds\n", (float)Beam_test_ints/(float)Beam_test_framecount, Beam_test_ints, Beam_test_framecount, Beam_test_ship, Beam_test_ast, (float)BEAM_TEST_STAMP_TIME / 1000.0f));

			// reset vars
			Beam_test_stamp = timestamp(BEAM_TEST_STAMP_TIME);
			Beam_test_ints = 0;
			Beam_test_ship = 0;
			Beam_test_ast = 0;
			Beam_test_framecount = 0;
		} else {
			Beam_test_framecount++;
		}
	}
	*/
#endif
//	mprintf(("done beam_move_all_post\n"));
}

// -----------------------------===========================------------------------------
// BEAM RENDERING FUNCTIONS
// -----------------------------===========================------------------------------

// render a beam weapon
#define STUFF_VERTICES()	do { verts[0]->u = 0.0f; verts[0]->v = 0.0f;	verts[1]->u = 1.0f; verts[1]->v = 0.0f; verts[2]->u = 1.0f;	verts[2]->v = 1.0f; verts[3]->u = 0.0f; verts[3]->v = 1.0f; } while(0);
#define P_VERTICES()		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);
int poly_beam = 0;
float U_offset =0.0f; // beam texture offset -Bobboau
void beam_render(beam_weapon_info *bwi, vec3d *start, vec3d *shot, float shrink)
{	
//	mprintf(("about to render a beam\n"));
	int idx, s_idx;
	vertex h1[4];				// halves of a beam section	
	vertex *verts[4] = { &h1[0], &h1[1], &h1[2], &h1[3] };	
	vec3d fvec, top1, bottom1, top2, bottom2;
	float scale;	
	float u_scale;	// beam tileing -Bobboau
	float length;	// beam tileing -Bobboau

	memset(h1,0,sizeof(vertex)*4);

	// bogus weapon info index
	if(bwi == NULL){
		return;
	}

	// if the beam start and endpoints are the same
	if(vm_vec_same(start, shot)){
		return;
	}

	// get beam direction
	vm_vec_sub(&fvec, shot, start);
	vm_vec_normalize_quick(&fvec);		

	// turn off backface culling
	gr_set_cull(0);
	
	// draw all sections	
	for(s_idx=0; s_idx<bwi->beam_num_sections; s_idx++){
		if ( (bwi->sections[s_idx].texture < 0) || (bwi->sections[s_idx].width <= 0.0f) )
			continue;

		// calculate the beam points
		scale = frand_range(1.0f - bwi->sections[s_idx].flicker, 1.0f + bwi->sections[s_idx].flicker);
		beam_calc_facing_pts(&top1, &bottom1, &fvec, start, bwi->sections[s_idx].width * scale * shrink, bwi->sections[s_idx].z_add);	
		beam_calc_facing_pts(&top2, &bottom2, &fvec, shot, bwi->sections[s_idx].width * scale * scale * shrink, bwi->sections[s_idx].z_add);				
		if(Cmdline_nohtl){
			g3_rotate_vertex(verts[0], &bottom1); 
			g3_rotate_vertex(verts[1], &bottom2);	
			g3_rotate_vertex(verts[2], &top2); 
			g3_rotate_vertex(verts[3], &top1);
		}else{
			g3_transfer_vertex(verts[0], &bottom1); 
			g3_transfer_vertex(verts[1], &bottom2);	
			g3_transfer_vertex(verts[2], &top2); 
			g3_transfer_vertex(verts[3], &top1);
		}
		P_VERTICES();						
		STUFF_VERTICES();		// stuff the beam with creamy goodness (texture coords)

		//U_offset = ( ( ((float)timestamp() * bwi->sections[s_idx].translation)/1000.0f) - (float)(timestamp() * (int)bwi->sections[s_idx].translation /1000));

		length = vm_vec_dist(start, shot);					// beam tileing -Bobboau
		
		if (bwi->sections[s_idx].tile_type == 1){
		u_scale = length / (bwi->sections[s_idx].width /2) / bwi->sections[s_idx].tile_factor;	// beam tileing, might make a tileing factor in beam index later -Bobboau
		}else{
		u_scale = bwi->sections[s_idx].tile_factor;
		}

		verts[1]->u = (u_scale + (U_offset * bwi->sections[s_idx].translation));				// beam tileing -Bobboau
		verts[2]->u = (u_scale + (U_offset * bwi->sections[s_idx].translation));				// beam tileing -Bobboau
		verts[3]->u = (0 + (U_offset * bwi->sections[s_idx].translation));
		verts[0]->u = (0 + (U_offset * bwi->sections[s_idx].translation));

		float per = 1.0f;
		if(bwi->range)per -= length / bwi->range;

		//this should never happen but, just to be safe
		if(per > 1.0f)per = 1.0f;
		if(per < 0.0f)per = 0.0f;

		verts[1]->r = (ubyte)(255 * per);
		verts[2]->r = (ubyte)(255 * per);
		verts[1]->g = (ubyte)(255 * per);
		verts[2]->g = (ubyte)(255 * per);
		verts[1]->b = (ubyte)(255 * per);
		verts[2]->b = (ubyte)(255 * per);
		verts[1]->a = (ubyte)(255 * per);
		verts[2]->a = (ubyte)(255 * per);

		verts[0]->r = 255;
		verts[3]->r = 255;
		verts[0]->g = 255;
		verts[3]->g = 255;
		verts[0]->b = 255;
		verts[3]->b = 255;
		verts[0]->a = 255;
		verts[3]->a = 255;

		// set the right texture with additive alpha, and draw the poly
		int cur_frame = 0;
		if (bwi->sections[s_idx].nframes > 1) {
			cur_frame = ((timestamp() / bwi->sections[s_idx].fps) % bwi->sections[s_idx].nframes);
		}

		gr_set_bitmap(bwi->sections[s_idx].texture + cur_frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);

		g3_draw_poly( 4, verts, TMAP_FLAG_TEXTURED | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_TILED | TMAP_FLAG_CORRECT | TMAP_HTL_3D_UNLIT); 
		// added TMAP_FLAG_TILED flag for beam texture tileing -Bobboau			
		// added TMAP_FLAG_RGB and TMAP_FLAG_GOURAUD so the beam would apear to fade along it's length-Bobboau
		
	}		
	
	// turn backface culling back on
	gr_set_cull(1);	
//	mprintf(("it should have rendered\n"));
}

// generate particles for the muzzle glow
int hack_time = 100;
DCF(h_time, "")
{
	dc_get_arg(ARG_INT);
	hack_time = Dc_arg_int;
}
void beam_generate_muzzle_particles(beam *b)
{
	int particle_count;
	int idx;
	weapon_info *wip;
	vec3d turret_norm, turret_pos, particle_pos, particle_dir, p_temp;
	matrix m;
	particle_info pinfo;

	// if our hack stamp has expired
	if(!((b->Beam_muzzle_stamp == -1) || timestamp_elapsed(b->Beam_muzzle_stamp))){
		return;
	}

	// never generate anything past about 1/5 of the beam fire time	
	if(b->warmup_stamp == -1){
		return;
	}

	// get weapon info
	wip = &Weapon_info[b->weapon_info_index];

	// no specified particle for this beam weapon
	if(wip->b_info.beam_particle_ani < 0){
		return;
	}
	
	// reset the hack stamp
	b->Beam_muzzle_stamp = timestamp(hack_time);

	// randomly generate 10 to 20 particles
	particle_count = (int)frand_range(0.0f, (float)wip->b_info.beam_particle_count);

	// get turret info - position and normal	
//	turret_pos = b->last_start;
//	vm_vec_sub(&turret_norm, &b->last_start,&b->last_shot);
//	vm_vec_normalize(&turret_norm);

	//turret_pos  = b->subsys->system_info->turret_firing_point[b->subsys->turret_next_fire_pos % b->subsys->system_info->turret_num_firing_points];
//	turret_pos = b->subsys->system_info->pnt;
	turret_pos = b->local_pnt;
	turret_norm = b->subsys->system_info->turret_norm;	

	// randomly perturb a vector within a cone around the normal
	vm_vector_2_matrix(&m, &turret_norm, NULL, NULL);
	for(idx=0; idx<particle_count; idx++){
		// get a random point in the cone
		vm_vec_random_cone(&particle_dir, &turret_norm, wip->b_info.beam_particle_angle, &m);
		p_temp = turret_pos;
		vm_vec_scale_add(&p_temp, &turret_pos, &particle_dir, wip->b_info.beam_muzzle_radius * frand_range(0.75f, 0.9f));

		// transform into world coords		
		vm_vec_unrotate(&particle_pos, &p_temp, &b->objp->orient);
		vm_vec_add2(&particle_pos, &b->objp->pos);
		p_temp = particle_dir;
		vm_vec_unrotate(&particle_dir, &p_temp, &b->objp->orient);

		// now generate some interesting values for the particle
		float p_time_ref = wip->b_info.beam_life + ((float)wip->b_info.beam_warmup / 1000.0f);		
		float p_life = frand_range(p_time_ref * 0.5f, p_time_ref * 0.7f);
		float p_vel = (wip->b_info.beam_muzzle_radius / p_life) * frand_range(0.85f, 1.2f);
		vm_vec_scale(&particle_dir, -p_vel);

		memset(&pinfo, 0, sizeof(particle_info));
		pinfo.pos = particle_pos;
		pinfo.vel = particle_dir;
		pinfo.lifetime = p_life;
		pinfo.attached_objnum = -1;
		pinfo.attached_sig = 0;
		pinfo.rad = wip->b_info.beam_particle_radius;
		pinfo.reverse = 1;
		pinfo.type = PARTICLE_BITMAP;
		pinfo.optional_data = wip->b_info.beam_particle_ani;
		pinfo.tracer_length = -1.0f;		
		particle_create(&pinfo);
	}
}

// render the muzzle glow for a beam weapon
void beam_render_muzzle_glow(beam *b)
{
	vertex pt;
	weapon_info *wip = &Weapon_info[b->weapon_info_index];
	beam_weapon_info *bwi = &Weapon_info[b->weapon_info_index].b_info;
	float rad, pct, rand_val;
	int tmap_flags = TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT;

	// if we don't have a glow bitmap
	if (bwi->beam_glow_bitmap < 0)
		return;

	// if the beam is warming up, scale the glow
	if (b->warmup_stamp != -1) {		
		// get warmup pct
		pct = BEAM_WARMUP_PCT(b);
		rand_val = 1.0f;
	} else
	// if the beam is warming down
	if (b->warmdown_stamp != -1) {
		// get warmup pct
		pct = 1.0f - BEAM_WARMDOWN_PCT(b);
		rand_val = 1.0f;
	} 
	// otherwise the beam is really firing
	else {
		pct = 1.0f;
		rand_val = frand_range(0.90f, 1.0f);
	}

	rad = wip->b_info.beam_muzzle_radius * pct * rand_val;

	// don't bother trying to draw if there is no radius
	if (rad <= 0.0f)
		return;

	// draw the bitmap
	if (Cmdline_nohtl)
		g3_rotate_vertex(&pt, &b->last_start);
	else
		g3_transfer_vertex(&pt, &b->last_start);


	int frame = 0;

	if (bwi->beam_glow_nframes > 1)
		frame = (timestamp() / bwi->beam_glow_fps) % bwi->beam_glow_nframes;

	gr_set_bitmap(bwi->beam_glow_bitmap + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.8f * pct);

	// draw 1 bitmap
	g3_draw_bitmap(&pt, 0, rad, tmap_flags);
	
	// maybe draw more
	if (pct > 0.3f)
		g3_draw_bitmap(&pt, 0, rad * 0.75f, tmap_flags, rad * 0.25f);

	if (pct > 0.5f)
		g3_draw_bitmap(&pt, 0, rad * 0.45f, tmap_flags, rad * 0.55f);

	if (pct > 0.7f)
		g3_draw_bitmap(&pt, 0, rad * 0.25f, tmap_flags, rad * 0.75f);
}

// render all beam weapons
void beam_render_all()
{
	beam *moveup;	
	
	U_offset = U_offset + flFrametime;	//moves the U value of texture coods in beams if desired-Bobboau

	//don't wrap since it causes the beam to jump	
//	if(U_offset > 1.0f){
//		U_offset = U_offset - 1.0f;	//keeps it below 1.0-Bobboau
//	}


	// traverse through all active beams
	moveup = GET_FIRST(&Beam_used_list);
	while(moveup != END_OF_LIST(&Beam_used_list)){				
		// each beam type renders a little bit differently
		if((moveup->warmup_stamp == -1) && (moveup->warmdown_stamp == -1) && !(moveup->flags & BF_SAFETY)){
			// HACK -  if this is the first frame the beam is firing, don't render it
			if(moveup->life_left >= moveup->life_total - 0.0001f){

				moveup = GET_NEXT(moveup);
				continue;
			}			

			// render the beam itself
			Assert(moveup->weapon_info_index >= 0);
			if(moveup->weapon_info_index < 0){
				moveup = GET_NEXT(moveup);
				continue;
			}
			beam_render(&Weapon_info[moveup->weapon_info_index].b_info, &moveup->last_start, &moveup->last_shot, moveup->shrink);
		}

		// render the muzzle glow
		beam_render_muzzle_glow(moveup);		

		// maybe generate some muzzle particles
		beam_generate_muzzle_particles(moveup);

		// next item
		moveup = GET_NEXT(moveup);
	}	
}

// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void beam_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add )
{
	vec3d uvec, rvec;
	vec3d temp;

	temp = *pos;

	vm_vec_sub( &rvec, &Eye_position, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_crossprod(&uvec,fvec,&rvec);
	// VECMAT-ERROR: NULL VEC3D (value of, fvec == rvec)
	vm_vec_normalize_safe(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	
}

// light scale factor
float blight = 25.5f;
DCF(blight, "")
{
	dc_get_arg(ARG_FLOAT);
	blight = Dc_arg_float;
}

// call to add a light source to a small object
void beam_add_light_small(beam *bm, object *objp, vec3d *pt_override = NULL)
{
	weapon_info *wip;
	beam_weapon_info *bwi;
	float noise;

	// no lighting 
	if(!Beam_lighting){
		return;
	}

	// sanity
	Assert(bm != NULL);
	if(bm == NULL){
		return;
	}
	Assert(objp != NULL);
	if(objp == NULL){
		return;
	}
	Assert(bm->weapon_info_index >= 0);
	wip = &Weapon_info[bm->weapon_info_index];
	bwi = &wip->b_info;

	// some noise
	noise = frand_range(1.0f - bwi->sections[0].flicker, 1.0f + bwi->sections[0].flicker);

	// widest part of the beam
	float light_rad = beam_get_widest(bm) * blight * noise;	

	// nearest point on the beam, and its distance to the ship
	vec3d near_pt;
	if(pt_override == NULL){
		float dist;
		vm_vec_dist_to_line(&objp->pos, &bm->last_start, &bm->last_shot, &near_pt, &dist);
		if(dist > light_rad){
			return;
		}
	} else {
		near_pt = *pt_override;
	}

	// average rgb of the beam	
	float fr = (float)wip->laser_color_1.red / 255.0f;
	float fg = (float)wip->laser_color_1.green / 255.0f;
	float fb = (float)wip->laser_color_1.blue / 255.0f;

	// add a unique light
	// noise *= 0.1f;			// a little less noise here, since we want the beam to generally cast a bright light
	light_add_point_unique(&near_pt, light_rad * 0.0001f, light_rad, 1.0f, fr, fg, fb, OBJ_INDEX(objp));
}

// call to add a light source to a large object
void beam_add_light_large(beam *bm, object *objp, vec3d *pt0, vec3d *pt1)
{
	weapon_info *wip;
	beam_weapon_info *bwi;
	float noise;

	// no lighting 
	if(!Beam_lighting){
		return;
	}

	// sanity
	Assert(bm != NULL);
	if(bm == NULL){
		return;
	}
	Assert(objp != NULL);
	if(objp == NULL){
		return;
	}
	Assert(bm->weapon_info_index >= 0);
	wip = &Weapon_info[bm->weapon_info_index];
	bwi = &wip->b_info;

	// some noise
	noise = frand_range(1.0f - bwi->sections[0].flicker, 1.0f + bwi->sections[0].flicker);

	// widest part of the beam
	float light_rad = beam_get_widest(bm) * blight * noise;		

	// average rgb of the beam	
	float fr = (float)wip->laser_color_1.red / 255.0f;
	float fg = (float)wip->laser_color_1.green / 255.0f;
	float fb = (float)wip->laser_color_1.blue / 255.0f;

	// add a unique light
	noise *= 0.1f;			// a little less noise here, since we want the beam to generally cast a bright light
	light_add_tube(pt0, pt1, 1.0f, light_rad, 1.0f * noise, fr, fg, fb, OBJ_INDEX(objp));
}

// mark an object as being lit
void beam_add_light(beam *b, int objnum, int source, vec3d *c_point)
{
	beam_light_info *l;

	// if we're out of light slots!
	if(Beam_light_count >= MAX_BEAM_LIGHT_INFO){
		// Int3();
		return;
	}

	// otherwise add it
	l = &Beam_lights[Beam_light_count++];
	l->bm = b;
	l->objnum = objnum;
	l->source = (ubyte)source;
	
	// only type 2 lights (from collisions) need a collision point
	if(c_point != NULL){
		l->c_point = *c_point;
	} else {
		Assert(source != 2);
		if(source == 2){
			Beam_light_count--;
		}
	}
}

// apply lighting from any beams
void beam_apply_lighting()
{
	int idx;
	beam_light_info *l;
	vec3d pt, dir;
	beam_weapon_info *bwi;

	// convert all beam lights into real lights
	for(idx=0; idx<Beam_light_count; idx++){
		// get the light
		l = &Beam_lights[idx];		

		// bad object
		if((l->objnum < 0) || (l->objnum >= MAX_OBJECTS) || (l->bm == NULL)){
			continue;
		}

		bwi = &Weapon_info[l->bm->weapon_info_index].b_info;

		// different light types
		switch(l->source){
		// from the muzzle of the gun
		case 0:
			// a few meters in from the of muzzle			
			vm_vec_sub(&dir, &l->bm->last_start, &l->bm->last_shot);
			vm_vec_normalize_quick(&dir);
			vm_vec_scale(&dir, -0.8f);  // TODO: This probably needs to *not* be stupid. -taylor
			vm_vec_scale_add(&pt, &l->bm->last_start, &dir, bwi->beam_muzzle_radius * 5.0f);

			beam_add_light_small(l->bm, &Objects[l->objnum], &pt);
			break;

		// from the beam passing by
		case 1:
			// object type
			switch(Objects[l->objnum].type){
			case OBJ_SHIP:
				Assert(Objects[l->objnum].instance >= 0);

				// large ships
				if(Ship_info[Ships[Objects[l->objnum].instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)){
					beam_add_light_large(l->bm, &Objects[l->objnum], &l->bm->last_start, &l->bm->last_shot);
				}
				// small ships
				else {
					beam_add_light_small(l->bm, &Objects[l->objnum]);
				}
				break;

			// asteroids get small lights
			case OBJ_ASTEROID:
				beam_add_light_small(l->bm, &Objects[l->objnum]);
				break;

			// debris gets small lights
			case OBJ_DEBRIS:
				beam_add_light_small(l->bm, &Objects[l->objnum]);
				break;
			}
			break;

		// from a collision
		case 2:
			// a few meters from the collision point			
			vm_vec_sub(&dir, &l->bm->last_start, &l->c_point);
			vm_vec_normalize_quick(&dir);
			vm_vec_scale_add(&pt, &l->c_point, &dir, bwi->beam_muzzle_radius * 5.0f);

			beam_add_light_small(l->bm, &Objects[l->objnum], &pt);
			break;
		}
	}	
}

// -----------------------------===========================------------------------------
// BEAM BOOKKEEPING FUNCTIONS
// -----------------------------===========================------------------------------

// delete a beam
void beam_delete(beam *b)
{
	// remove from active list and put on free list
	list_remove(&Beam_used_list, b);
	list_append(&Beam_free_list, b);

	// delete our associated object
	// Assert(b->objnum >= 0);
	if(b->objnum >= 0){
		obj_delete(b->objnum);
	}
	b->objnum = -1;

	// kill the beam looping sound
	if(b->beam_sound_loop != -1){
		snd_stop(b->beam_sound_loop);
		b->beam_sound_loop = -1;
	}	

	// handle model animation reversal (closing)
	// (beam animations should end pretty much immediately - taylor)
	if (b->subsys->turret_animation_position == MA_POS_READY)
		b->subsys->turret_animation_done_time = timestamp(50);

	// subtract one
	Beam_count--;
	Assert(Beam_count >= 0);
	nprintf(("General", "Recycled beam (%d beams remaining)\n", Beam_count));
}

// given an object, return its model num
int beam_get_model(object *objp)
{
	int pof;

	if (objp == NULL) {
		return -1;
	}

	Assert(objp->instance >= 0);
	if(objp->instance < 0){
		return -1;
	}

	switch(objp->type){
	case OBJ_SHIP:		
		return Ships[objp->instance].modelnum;

	case OBJ_WEAPON:
		Assert(Weapons[objp->instance].weapon_info_index >= 0);
		if(Weapons[objp->instance].weapon_info_index < 0){
			return -1;
		}
		return Weapon_info[Weapons[objp->instance].weapon_info_index].model_num;

	case OBJ_DEBRIS:
		Assert(Debris[objp->instance].is_hull);
		if(!Debris[objp->instance].is_hull){
			return -1;
		}
		return Debris[objp->instance].model_num;		

	case OBJ_ASTEROID:
		pof = Asteroids[objp->instance].asteroid_subtype;
		Assert(Asteroids[objp->instance].asteroid_type >= 0);
		if(Asteroids[objp->instance].asteroid_type < 0){
			return -1;
		}
		return Asteroid_info[Asteroids[objp->instance].asteroid_type].model_num[pof];

	default:
		// this shouldn't happen too often
		mprintf(("Beam couldn't find a good find a good object model/type!! (%d)", objp->type));
		return -1;
	}

	// can't happen
	Int3();
	return -1;
}

// start the warmup phase for the beam
void beam_start_warmup(beam *b)
{
	// set the warmup stamp
	b->warmup_stamp = timestamp(Weapon_info[b->weapon_info_index].b_info.beam_warmup);

	// start playing warmup sound
	if(!(Game_mode & GM_STANDALONE_SERVER) && (Weapon_info[b->weapon_info_index].b_info.beam_warmup_sound >= 0)){		
		snd_play_3d(&Snds[Weapon_info[b->weapon_info_index].b_info.beam_warmup_sound], &b->last_start, &View_position);
	}
}

// start the firing phase for the beam, return 0 if the beam failed to start, and should be deleted altogether
int beam_start_firing(beam *b)
{
	// kill the warmup stamp so the rest of the code knows its firing
	b->warmup_stamp = -1;	

	// any special stuff for each weapon type
	switch(b->type){
	// re-aim type A and D beam weapons here, otherwise they tend to miss		
	case BEAM_TYPE_A:
	case BEAM_TYPE_D:
		beam_aim(b);
		break;
	
	case BEAM_TYPE_B:
		break;

	case BEAM_TYPE_C:
		break;	

	case BEAM_TYPE_E:
		break;

	default:
		Int3();
	}

	// determine if we can legitimately start firing, or if we need to take other action
	switch(beam_ok_to_fire(b)){
	case -1 :
		return 0;

	case 0 :			
		beam_start_warmdown(b);
		return 1;
	}				
			
	// start the beam firing sound now, if we haven't already		
	if((b->beam_sound_loop == -1) && (Weapon_info[b->weapon_info_index].b_info.beam_loop_sound >= 0)){				
		b->beam_sound_loop = snd_play_3d(&Snds[Weapon_info[b->weapon_info_index].b_info.beam_loop_sound], &b->last_start, &View_position, 0.0f, NULL, 1, 1.0, SND_PRIORITY_SINGLE_INSTANCE, NULL, 1.0f, 1);

		// "shot" sound
		snd_play_3d(&Snds[SND_BEAM_SHOT], &b->last_start, &View_position); //I'm sorry this thing has always pissed me off -Bobboau
		// GAH - Bobboau, for goodness sake don't delete things flippantly.  If you want to change this kind of thing, add
		// command-line behavior like Phreak did for his targeting stuff.  The code is not yours to tromp all over like this. -- Goober5000
	}	

	// success
	return 1;
}

// start the warmdown phase for the beam
void beam_start_warmdown(beam *b)
{
	// timestamp
	b->warmdown_stamp = timestamp(Weapon_info[b->weapon_info_index].b_info.beam_warmdown);			

	// start the warmdown sound
	if(Weapon_info[b->weapon_info_index].b_info.beam_warmdown_sound >= 0){				
		snd_play_3d(&Snds[Weapon_info[b->weapon_info_index].b_info.beam_warmdown_sound], &b->last_start, &View_position);
	}

	// kill the beam looping sound 
	if(b->beam_sound_loop != -1){
		snd_stop(b->beam_sound_loop);
		b->beam_sound_loop = -1;
	}						
}

// recalculate beam sounds (looping sounds relative to the player)
void beam_recalc_sounds(beam *b)
{
	beam_weapon_info *bwi;
	vec3d pos;	

	Assert(b->weapon_info_index >= 0);
	if(b->weapon_info_index < 0){
		return;
	}
	bwi = &Weapon_info[b->weapon_info_index].b_info;

	// update the sound position relative to the player
	if(b->beam_sound_loop != -1){
		// get the point closest to the player's viewing position
		switch(vm_vec_dist_to_line(&View_position, &b->last_start, &b->last_shot, &pos, NULL)){
		// behind the beam, so use the start pos
		case -1:
			pos = b->last_start;
			break;

		// use the closest point
		case 0:
			// already calculated in vm_vec_dist_to_line(...)
			break;

		// past the beam, so use the shot pos
		case 1:
			pos = b->last_shot;
			break;
		}

		snd_update_3d_pos(b->beam_sound_loop, &Snds[bwi->beam_loop_sound], &pos);
	}
}


// -----------------------------===========================------------------------------
// BEAM AIMING FUNCTIONS
// -----------------------------===========================------------------------------

// fills in binfo
void beam_get_binfo(beam *b, float accuracy, int num_shots)
{
	vec3d p2;
	int model_num, idx;	
	vec3d oct1, oct2;
	vec3d turret_point, turret_norm;
	beam_weapon_info *bwi;
	int skill_level;

	// where the shot is originating from (b->last_start gets filled in)
	beam_get_global_turret_gun_info(b->objp, b->subsys, &turret_point, &turret_norm, 1, &p2, b->fighter_beam);

	// get a model # to work with
	model_num = beam_get_model(b->target);	
	if(model_num < 0){
		return;
	}	

	// get beam weapon info
	Assert(b->weapon_info_index >= 0);
	if(b->weapon_info_index < 0){
		return;
	}
	bwi = &Weapon_info[b->weapon_info_index].b_info;

	// skill level
	skill_level = Game_skill_level;
	if(Game_skill_level >= NUM_SKILL_LEVELS){
		skill_level = NUM_SKILL_LEVELS - 1;
	}
	if(Game_skill_level < 0){
		skill_level = 0;
	}

	// stuff num shots even though its only used for type D weapons
	b->binfo.shot_count = (ubyte)num_shots;
	if(b->binfo.shot_count > MAX_BEAM_SHOTS){
		b->binfo.shot_count = MAX_BEAM_SHOTS;
	}

	// generate the proper amount of directional vectors	
	switch(b->type){	
	// pick an accuracy. beam will be properly aimed at actual fire time
	case BEAM_TYPE_A:		
		// all we will do is decide whether or not we will hit - type A beam weapons are re-aimed immediately before firing
		b->binfo.shot_aim[0] = frand_range(0.0f, 1.0f + bwi->beam_miss_factor[skill_level] * accuracy);
		b->binfo.shot_count = 1;

		// get random model points, this is useful for big ships, because we never miss when shooting at them			
		submodel_get_two_random_points(model_num, 0, &b->binfo.dir_a, &b->binfo.dir_b);
		break;	

	// just 2 points in the "slash"
	case BEAM_TYPE_B:							
		beam_get_octant_points(model_num, b->target, (int)frand_range(0.0f, BEAM_NUM_GOOD_OCTANTS), Beam_good_slash_octants, &oct1, &oct2);		

		// point 1
		vm_vec_sub(&b->binfo.dir_a, &oct1, &turret_point);
		vm_vec_normalize(&b->binfo.dir_a);

		// point 2
		vm_vec_sub(&b->binfo.dir_b, &oct2, &turret_point);
		vm_vec_normalize(&b->binfo.dir_b);	
		
		// delta angle
		b->binfo.delta_ang = fl_abs(vm_vec_delta_ang_norm(&b->binfo.dir_a, &b->binfo.dir_b, NULL));
		break;

	// nothing for this beam - its very special case
	case BEAM_TYPE_C:
		break;

	// type D beams fire at small ship multiple times
	case BEAM_TYPE_D:
		// get a bunch of shot aims
		for(idx=0; idx<b->binfo.shot_count; idx++){
			//	MK, 9/3/99: Added pow() function to make increasingly likely to miss with subsequent shots.  30% more likely with each shot.
			float r = ((float) pow(1.3f, idx)) * bwi->beam_miss_factor[skill_level] * accuracy;
			b->binfo.shot_aim[idx] = frand_range(0.0f, 1.0f + r);
		}
		break;

	// type e beams just fire straight
	case BEAM_TYPE_E:
		b->binfo.shot_aim[0] = 0.0000001f;
		b->binfo.shot_count = 1;
		b->binfo.dir_a = turret_norm;
		b->binfo.dir_b = turret_norm;
		break;

	default:
		break;
	}
}

// aim the beam (setup last_start and last_shot - the endpoints). also recalculates collision pairs
void beam_aim(beam *b)
{
	vec3d temp, p2;
	int model_num;	
	
	// type C beam weapons have no target
	if(b->target == NULL){
		Assert(b->type == BEAM_TYPE_C);
		if(b->type != BEAM_TYPE_C){
			return;
		}
	}
	// get a model # to work with
	else {
		model_num = beam_get_model(b->target);	
		if(model_num < 0){
			return;
		}	
	}

	// setup our initial shot point and aim direction
	switch(b->type){
	case BEAM_TYPE_A:	
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);

		// if we're targeting a subsystem - shoot directly at it
		if(b->target_subsys != NULL){			
			// unrotate the center of the subsystem
//			vm_vec_unrotate(&b->last_shot, &b->local_pnt, &b->target->orient);
			vm_vec_unrotate(&b->last_shot, &b->target_subsys->system_info->pnt, &b->target->orient);
			vm_vec_add2(&b->last_shot, &b->target->pos);		 
			vm_vec_sub(&temp, &b->last_shot, &b->last_start);
			
			vm_vec_scale_add(&b->last_shot, &b->last_start, &temp, 2.0f);
			break;
		}

		// if we're shooting at a big ship - shoot directly at the model
		if((b->target->type == OBJ_SHIP) && (Ship_info[Ships[b->target->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP))){
			// rotate into world coords
			vm_vec_unrotate(&temp, &b->binfo.dir_a, &b->target->orient);
			vm_vec_add2(&temp, &b->target->pos);

			// get the shot point
			vm_vec_sub(&p2, &temp, &b->last_start);
			vm_vec_scale_add(&b->last_shot, &b->last_start, &p2, 2.0f);
			break;
		}
		
		// point at the center of the target, then jitter based on shot_aim
		b->last_shot = b->target->pos;		
		beam_jitter_aim(b, b->binfo.shot_aim[0]);
		break;	

	case BEAM_TYPE_B:		
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);				

		// set the shot point
		vm_vec_scale_add(&b->last_shot, &b->last_start, &b->binfo.dir_a, b->range);
		Assert(is_valid_vec(&b->last_shot));		
		break;

	case BEAM_TYPE_C:
		// start point
		temp = b->targeting_laser_offset;	
		vm_vec_unrotate(&b->last_start, &temp, &b->objp->orient);
		vm_vec_add2(&b->last_start, &b->objp->pos);
		vm_vec_scale_add(&b->last_shot, &b->last_start, &b->objp->orient.vec.fvec, b->range);		
		break;

	case BEAM_TYPE_D:				
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);		
		
		// point at the center of the target, then jitter based on shot_aim
		b->last_shot = b->target->pos;		
		beam_jitter_aim(b, b->binfo.shot_aim[b->shot_index]);
		nprintf(("AI", "Frame %i: FIRING\n", Framecount));
		break;

	case BEAM_TYPE_E:
		// where the shot is originating from (b->last_start gets filled in)
		beam_get_global_turret_gun_info(b->objp, b->subsys, &b->last_start, &temp, 1, &p2, b->fighter_beam);		

		// point directly in the direction of the turret
		vm_vec_scale_add(&b->last_shot, &b->last_start, &temp, b->range);
		break;

	default:
		Int3();
	}		

	// recalculate object pairs
	OBJ_RECALC_PAIRS((&Objects[b->objnum]));
}

// given a model #, and an object, stuff 2 good world coord points
void beam_get_octant_points(int modelnum, object *objp, int oct_index, int oct_array[BEAM_NUM_GOOD_OCTANTS][4], vec3d *v1, vec3d *v2)
{	
	vec3d t1, t2, temp;
	polymodel *m = model_get(modelnum);

	// bad bad bad bad bad bad
	if(m == NULL){
		Int3();
		return;
	}

	Assert((oct_index >= 0) && (oct_index < BEAM_NUM_GOOD_OCTANTS));

	// randomly pick octants	
	t1 = oct_array[oct_index][2] ? m->octants[oct_array[oct_index][0]].max : m->octants[oct_array[oct_index][0]].min;
	t2 = oct_array[oct_index][3] ? m->octants[oct_array[oct_index][1]].max : m->octants[oct_array[oct_index][1]].min;
	Assert(!vm_vec_same(&t1, &t2));

	// get them in world coords
	vm_vec_unrotate(&temp, &t1, &objp->orient);
	vm_vec_add(v1, &temp, &objp->pos);
	vm_vec_unrotate(&temp, &t2, &objp->orient);
	vm_vec_add(v2, &temp, &objp->pos);
}

// throw some jitter into the aim - based upon shot_aim
void beam_jitter_aim(beam *b, float aim)
{
	vec3d forward, circle;
	matrix m;
	float subsys_strength;

	// if the weapons subsystem is damaged or destroyed
	if((b->objp != NULL) && (b->objp->signature == b->sig) && (b->objp->type == OBJ_SHIP) && (b->objp->instance >= 0) && (b->objp->instance < MAX_SHIPS)){
		// get subsytem strength
		subsys_strength = ship_get_subsystem_strength(&Ships[b->objp->instance], SUBSYSTEM_WEAPONS);
		
		// when subsytem strength is 0, double the aim error factor
		aim += aim * (1.0f - subsys_strength);
	}

	// shot aim is a direct linear factor of the target model's radius.
	// so, pick a random point on the circle
	vm_vec_sub(&forward, &b->last_shot, &b->last_start);
	vm_vec_normalize_quick(&forward);
	
	// vector
	vm_vector_2_matrix(&m, &forward, NULL, NULL);

	// get a vector on the circle - this should appear to be pretty random
	// vm_vec_scale_add(&circle, &b->last_shot, &m.rvec, aim * b->target->radius);
	vm_vec_random_in_circle(&circle, &b->last_shot, &m, aim * b->target->radius, 0);
	
	// get the vector pointing to the circle point
	vm_vec_sub(&forward, &circle, &b->last_start);	
	vm_vec_scale_add(&b->last_shot, &b->last_start, &forward, 2.0f);
}


// -----------------------------===========================------------------------------
// BEAM COLLISION FUNCTIONS
// -----------------------------===========================------------------------------

// collide a beam with a ship, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_ship(obj_pair *pair)
{
	beam *b;		
	ship *shipp;
	ship_info *sip;
	mc_info test_collide;		
	int model_num, quad;
	float widest;
	weapon_info *bwi;
	vec3d beam_dir, neg_beam_dir,tmp_vec;

	// bogus
	if(pair == NULL){
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// Don't check collisions for warping out player if past stage 1.
	if ( Player->control_mode >= PCM_WARPOUT_STAGE1)	{
		if ( pair->a == Player_obj ) return 0;
		if ( pair->b == Player_obj ) return 0;
	}

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){		
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if(b->flags & BF_SAFETY){
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if(pair->b == b->objp){
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if(model_num < 0){
		// Int3();
		return 1;
	}
	
#ifndef NDEBUG
	Beam_test_ints++;
	Beam_test_ship++;
#endif

	// bad
	Assert(pair->b->type == OBJ_SHIP);
	Assert(pair->b->instance >= 0);
	if((pair->b->type != OBJ_SHIP) || ((pair->b->instance) < 0)){
		return 1;
	}

	shipp = &Ships[pair->b->instance];
	sip = &Ship_info[shipp->ship_info_index];

	// get the widest portion of the beam
	widest = beam_get_widest(b);

	// do the collision		
	test_collide.model_num = model_num;
	test_collide.submodel_num = -1;
	test_collide.orient = &pair->b->orient;
	test_collide.pos = &pair->b->pos;
	test_collide.p0 = &b->last_start;
	test_collide.p1 = &b->last_shot;

	bwi = &Weapon_info[b->weapon_info_index];

	polymodel *pm = model_get(model_num);

	int exit_flag = 0;
	int collisions_handled = 0;
	vm_vec_sub(&beam_dir, &b->last_shot, &b->last_start);
	vm_vec_normalize(&beam_dir);
	vm_vec_copy_scale(&neg_beam_dir, &beam_dir, -1);

	do
	{
		// maybe do a sphere line
		if(widest > pair->b->radius * BEAM_AREA_PERCENT){
			test_collide.radius = beam_get_widest(b) * 0.5f;
			//if the shields have any juice check them otherwise check the model
			if ( !(bwi->wi_flags2 & WIF2_PIERCE_SHIELDS) && (get_shield_strength(&Objects[shipp->objnum])) && (bwi->shield_factor >= 0) && ((pm->shield.ntris > 0) && (pm->shield.nverts > 0)) )
			{
				//check shields for beams wich have a positive shield factor -Bobboau
				test_collide.flags = MC_CHECK_SHIELD | MC_CHECK_SPHERELINE;
			}	
			else
			{	
				test_collide.flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE;
			}
		}
		else
		{	
			if ( !(bwi->wi_flags2 & WIF2_PIERCE_SHIELDS) && (get_shield_strength(&Objects[shipp->objnum])) && (bwi->shield_factor >= 0) && ((pm->shield.ntris > 0) && (pm->shield.nverts > 0)) )
			{
				//check shields for type c beams -Bobboau
				test_collide.flags = MC_CHECK_SHIELD | MC_CHECK_RAY;	
			}
			else
			{	
				test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;	
			}
		}

		model_collide(&test_collide);

		quad = -1;
		if((test_collide.flags & MC_CHECK_SHIELD) || (!(Objects[shipp->objnum].flags & OF_NO_SHIELDS) && !(bwi->wi_flags2 & WIF2_PIERCE_SHIELDS) && (Ship_info[shipp->ship_info_index].flags2 & SIF2_SURFACE_SHIELDS)))	//if we're checking shields
		{
			quad = get_quadrant(&test_collide.hit_point);//find which quadrant we hit
			//mprintf(("the thing I hit was hit in quadrant %d\n", quad));
			//then if the beam does more damage than that quadrant can take
			if(Objects[shipp->objnum].shield_quadrant[quad] < (bwi->damage * bwi->shield_factor * 2.0f))
				//if(!(ship_is_shield_up(&Objects[shipp->objnum], get_quadrant(&test_collide.hit_point))))
			{
				// _argv[-1], 16 Jan 2005: Don't do another model_collide for surface shields, since we just did this. Just a performance optimization.
				if (test_collide.flags & MC_CHECK_SHIELD)
				{
					//go through the shield and hit the hull -Bobboau
					if(widest > pair->b->radius * BEAM_AREA_PERCENT)
					{
						test_collide.radius = beam_get_widest(b) * 0.5f;
						test_collide.flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE;
					}
					else
					{	
						test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;	
					}
					model_collide(&test_collide);
				}
			}
		}


		// if we got a hit
		if(test_collide.num_hits)
		{
			
			// add to the collision list
			beam_add_collision(b, pair->b, &test_collide, quad, exit_flag);
			collisions_handled++;
			
	
			//check collisions heading from the origin of the beam to its endpoint
			//this handles the beam entering the model
			if (!exit_flag)
			{
				test_collide.p0 = &tmp_vec;
				vm_vec_add(&tmp_vec, &test_collide.hit_point_world, &beam_dir);
			}
			//check collisions heading from the endpoint of the beam to its origin
			//this handles the beam exiting the model
			else
			{
				test_collide.p0 = &tmp_vec;
				vm_vec_add(&tmp_vec, &test_collide.hit_point_world, &neg_beam_dir);
			}
			

		}
		else
		{
			//break out of the loop if we couldn't find any more exit holes left
			if (exit_flag) break;

			//didn't find any new enter holes, start finding exit holes.
			else
			{
				exit_flag = 1;
				
				test_collide.p0 = &b->last_shot;
				test_collide.p1 = &b->last_start;
			}
		}

	//even though this this properly terminate, keep the collisions handled low so that the collisions with other ships aren't bumped out
	}while(beam_will_tool_target(b,pair->b) && collisions_handled < 6);

	// add this guy to the lighting list
	beam_add_light(b, OBJ_INDEX(pair->b), 1, NULL);

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);

/*	decal_point dec;
	dec.orient.vec = test_collide.orient->vec;
	dec.pnt = test_collide.hit_point;
	dec.radius = bwi->impact_explosion_radius*2;

	decal_create_simple(&Objects[shipp->objnum], &dec, bwi->b_info.beam_glow_bitmap);
*/
//	mprintf(("a decal should have been made at %0.2f %0.2f %0.2f", dec.pnt.xyz.x, dec.pnt.xyz.y, dec.pnt.xyz.z));
		
	return 0;
}


// collide a beam with an asteroid, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_asteroid(obj_pair *pair)
{
	beam *b;		
	mc_info test_collide;		
	int model_num;

	// bogus
	if(pair == NULL){
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if(b->flags & BF_SAFETY){
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if(pair->b == b->objp){
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if(model_num < 0){
		Int3();
		return 1;
	}	

#ifndef NDEBUG
	Beam_test_ints++;
	Beam_test_ast++;
#endif

	// do the collision		
	test_collide.model_num = model_num;
	test_collide.submodel_num = -1;
	test_collide.orient = &pair->b->orient;
	test_collide.pos = &pair->b->pos;
	test_collide.p0 = &b->last_start;
	test_collide.p1 = &b->last_shot;	
	test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;
	model_collide(&test_collide);

	// if we got a hit
	if(test_collide.num_hits){
		// add to the collision list
		beam_add_collision(b, pair->b, &test_collide);
	}	

	// add this guy to the lighting list
	beam_add_light(b, OBJ_INDEX(pair->b), 1, NULL);

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);
		
	return 0;	
}

// collide a beam with a missile, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_missile(obj_pair *pair)
{
	beam *b;	
	mc_info test_collide;		
	int model_num;

	// bogus
	if(pair == NULL){
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if(b->flags & BF_SAFETY){
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if(pair->b == b->objp){
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if(model_num < 0){
		//Int3();
		return 1;
	}

#ifndef NDEBUG
	Beam_test_ints++;
#endif

	// do the collision		
	test_collide.model_num = model_num;
	test_collide.submodel_num = -1;
	test_collide.orient = &pair->b->orient;
	test_collide.pos = &pair->b->pos;
	test_collide.p0 = &b->last_start;
	test_collide.p1 = &b->last_shot;
	test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;
	model_collide(&test_collide);

	// if we got a hit
	if(test_collide.num_hits){
		// add to the collision list
		beam_add_collision(b, pair->b, &test_collide);
	}

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);

	return 0;
}

// collide a beam with debris, returns 1 if we can ignore all future collisions between the 2 objects
int beam_collide_debris(obj_pair *pair)
{	
	beam *b;	
	mc_info test_collide;		
	int model_num;

	// bogus
	if(pair == NULL){
		return 0;
	}

	// get the beam
	Assert(pair->a->instance >= 0);
	Assert(pair->a->type == OBJ_BEAM);
	Assert(Beams[pair->a->instance].objnum == OBJ_INDEX(pair->a));
	b = &Beams[pair->a->instance];

	// if the "warming up" timestamp has not expired
	if((b->warmup_stamp != -1) || (b->warmdown_stamp != -1)){
		return 0;
	}

	// if the beam is on "safety", don't collide with anything
	if(b->flags & BF_SAFETY){
		return 0;
	}
	
	// if the colliding object is the shooting object, return 1 so this is culled
	if(pair->b == b->objp){
		return 1;
	}	

	// try and get a model
	model_num = beam_get_model(pair->b);
	if(model_num < 0){
		// Int3();
		return 1;
	}	

#ifndef NDEBUG
	Beam_test_ints++;
#endif

	// do the collision		
	test_collide.model_num = model_num;
	test_collide.submodel_num = -1;
	test_collide.orient = &pair->b->orient;
	test_collide.pos = &pair->b->pos;
	test_collide.p0 = &b->last_start;
	test_collide.p1 = &b->last_shot;
	test_collide.flags = MC_CHECK_MODEL | MC_CHECK_RAY;
	model_collide(&test_collide);

	// if we got a hit
	if(test_collide.num_hits){
		// add to the collision list
		beam_add_collision(b, pair->b, &test_collide);
	}	

	// add this guy to the lighting list
	beam_add_light(b, OBJ_INDEX(pair->b), 1, NULL);

	// reset timestamp to timeout immediately
	pair->next_check_time = timestamp(0);

	return 0;
}

// early-out function for when adding object collision pairs, return 1 if the pair should be ignored
int beam_collide_early_out(object *a, object *b)
{
	beam *bm;
	weapon_info *bwi;
	float cull_dist, cull_dot;
	vec3d dot_test, dot_test2, dist_test;	
		
	// get the beam
	Assert(a->instance >= 0);
	if(a->instance < 0){
		return 1;
	}
	Assert(a->type == OBJ_BEAM);
	if(a->type != OBJ_BEAM){
		return 1;
	}
	Assert(Beams[a->instance].objnum == OBJ_INDEX(a));
	if(Beams[a->instance].objnum != OBJ_INDEX(a)){
		return 1;
	}	
	bm = &Beams[a->instance];
	Assert(bm->weapon_info_index >= 0);
	if(bm->weapon_info_index < 0){
		return 1;
	}
	bwi = &Weapon_info[bm->weapon_info_index];

	// if the second object has an invalid instance, bail
	if(b->instance < 0){
		return 1;
	}

	if((vm_vec_dist(&bm->last_start, &b->pos)-b->radius) > bwi->b_info.range){
		return 1;
	}//if the object is too far away, don't bother trying to colide with it-Bobboau

	// baseline bails
	switch(b->type){
	case OBJ_SHIP:
		break;
	case OBJ_ASTEROID:
		// targeting lasers only hit ships
/*		if(bwi->b_info.beam_type == BEAM_TYPE_C){
			return 1;
		}*/
		break;
	case OBJ_DEBRIS:
		// targeting lasers only hit ships
/*		if(bwi->b_info.beam_type == BEAM_TYPE_C){
			return 1;
		}*/
		// don't ever collide with non hull pieces
		if(!Debris[b->instance].is_hull){
			return 1;
		}
		break;
	case OBJ_WEAPON:
		// targeting lasers only hit ships
/*		if(bwi->b_info.beam_type == BEAM_TYPE_C){
			return 1;
		}*/
		// don't ever collide against laser weapons - duh
		if(Weapon_info[Weapons[b->instance].weapon_info_index].subtype == WP_LASER){
			return 1;
		}
		break;
	}

	// get full cull value
	beam_get_cull_vals(b, bm, &cull_dot, &cull_dist);

	// if the object fails these conditions, bail
	vm_vec_sub(&dist_test, &b->pos, &bm->last_start);
	dot_test = dist_test;
	vm_vec_sub(&dot_test2, &bm->last_shot, &bm->last_start);
	vm_vec_normalize_quick(&dot_test);
	vm_vec_normalize_quick(&dot_test2);
	// cull_dist == DIST SQUARED FOO!
	if((vm_vec_dotprod(&dot_test, &dot_test2) < cull_dot) && (vm_vec_mag_squared(&dist_test) > cull_dist)){
		return 1;
	}
	
	// don't cull
	return 0;
}

// add a collision to the beam for this frame (to be evaluated later)
void beam_add_collision(beam *b, object *hit_object, mc_info *cinfo, int quad, int exit_flag)
{
	beam_collision *bc;
	int idx;
	int	quadrant_num = -1;
//	weapon_info *bwi = &Weapon_info[b->weapon_info_index];

	// if we haven't reached the limit for beam collisions, just add
	if(b->f_collision_count < MAX_FRAME_COLLISIONS){
		bc = &b->f_collisions[b->f_collision_count++];
		bc->c_objnum = OBJ_INDEX(hit_object);
		bc->cinfo = *cinfo;

/*		decal_point dec;
		vec3d bfvec;
		vm_vec_sub(&bfvec, &b->last_shot, &b->last_start);
		dec.orient.vec.fvec = bfvec;//cinfo->hit_normal;
		//get a good orientation matrix baised on the normal of the hit face and the orientation of the ship it hit-Bobboau
		vm_vec_normalize( &dec.orient.vec.fvec );
		vm_vec_crossprod( &dec.orient.vec.uvec, &cinfo->hit_normal, &dec.orient.vec.fvec);
		vm_vec_normalize( &dec.orient.vec.uvec );
		vm_vec_crossprod( &dec.orient.vec.rvec, &dec.orient.vec.uvec, &dec.orient.vec.fvec);
		vm_vec_normalize( &dec.orient.vec.rvec );

		dec.pnt = cinfo->hit_point;
		dec.radius = bwi->decal_rad;

//		decal_create_simple(hit_object, &dec, bwi->b_info.beam_glow_bitmap);//this is the old decals, not the new/better ones
		decal_create(hit_object, &dec, bc->cinfo.hit_submodel, bwi->decal_texture, bwi->decal_backface_texture);
*/

		// _argv[-1], 16 Jan 2005: Surface shield support.
		if( ((cinfo->flags & MC_CHECK_SHIELD) && cinfo->num_hits) || quad >= 0 ){ //beam shield hit code -Bobboau
			bc->quadrant= quadrant_num = quad == -1 ? get_quadrant(&cinfo->hit_point) : quad;
			if (!(hit_object->flags & SF_DYING) ) {
				if (cinfo->flags & MC_CHECK_SHIELD) {
					// Don't do this for surface shields.
					add_shield_point(hit_object-Objects, cinfo->shield_hit_tri, &cinfo->hit_point);
				}
				hud_shield_quadrant_hit(hit_object, quadrant_num);
			}
		}
		else
		{
			bc->quadrant=-1;
		}

		// done
		
		return;
	}		

	// otherwise, we've got to do some checking, ick. 
	// I guess we can always just remove the farthest item
	bc = NULL;
	for(idx=0; idx<MAX_FRAME_COLLISIONS; idx++){
		if((bc == NULL) || (b->f_collisions[idx].cinfo.hit_dist > bc->cinfo.hit_dist)){
			bc = &b->f_collisions[idx];
		}
	}

	// copy in
	Assert(bc != NULL);
	if(bc == NULL){
		return;
	}
	bc->c_objnum = OBJ_INDEX(hit_object);
	bc->cinfo = *cinfo;
	bc->is_exit_collision = exit_flag;
}

// sort collisions for the frame
int beam_sort_collisions_func(const void *e1, const void *e2)
{
	beam_collision *b1 = (beam_collision*)e1;
	beam_collision *b2 = (beam_collision*)e2;

	return b1->cinfo.hit_dist < b2->cinfo.hit_dist ? -1 : 1;
}

// handle a hit on a specific object
void beam_handle_collisions(beam *b)
{	
	int idx, s_idx;
	beam_collision r_coll[MAX_FRAME_COLLISIONS];
	int r_coll_count = 0;
	beam_weapon_info *bwi;
	weapon_info *wi;
	float widest;	

	// early out if we had no collisions
	if(b->f_collision_count <= 0){
		return;
	}

	// get beam weapon info
	if((b->weapon_info_index < 0) || (b->weapon_info_index >= Num_weapon_types)){
		Int3();
		return;
	}
	bwi = &Weapon_info[b->weapon_info_index].b_info;
	wi = &Weapon_info[b->weapon_info_index];

	// get the widest part of the beam
	widest = beam_get_widest(b);

	// the first thing we need to do is sort the collisions, from closest to farthest
	qsort(b->f_collisions, b->f_collision_count, sizeof(beam_collision), beam_sort_collisions_func);

	// now apply all collisions until we reach a ship which "stops" the beam or we reach the end of the list
	for(idx=0; idx<b->f_collision_count; idx++){	
		int model_num = -1;
		int do_damage = 0;
		int first_hit = 1;
		int target = b->f_collisions[idx].c_objnum;

		// if we have an invalid object
		if((target < 0) || (target >= MAX_OBJECTS)){
			continue;
		}

		// try and get a model to deal with		
		model_num = beam_get_model(&Objects[target]);
		if(model_num < 0){
			continue;
		}

		// add lighting
		beam_add_light(b, target, 2, &b->f_collisions[idx].cinfo.hit_point_world);

		// add to the recent collision list
		r_coll[r_coll_count].c_objnum = target;
		r_coll[r_coll_count].c_sig = Objects[target].signature;
		r_coll[r_coll_count].c_stamp = -1;
		r_coll[r_coll_count].cinfo = b->f_collisions[idx].cinfo;
		
		// if he was already on the recent collision list, copy his timestamp
		// also, be sure not to play the impact sound again.
		for(s_idx=0; s_idx<b->r_collision_count; s_idx++){
			if((r_coll[r_coll_count].c_objnum == b->r_collisions[s_idx].c_objnum) && (r_coll[r_coll_count].c_sig == b->r_collisions[s_idx].c_sig)){
				// timestamp
				r_coll[r_coll_count].c_stamp = b->r_collisions[s_idx].c_stamp;

				// don't play the impact sound again
				first_hit = 0;
			}
		}

		// if the damage timestamp has expired or is not set yet, apply damage
		if((r_coll[r_coll_count].c_stamp == -1) || timestamp_elapsed(r_coll[r_coll_count].c_stamp)){
			do_damage = 1;
			r_coll[r_coll_count].c_stamp = timestamp(BEAM_DAMAGE_TIME);
		}

		// if no damage - don't even indicate it has been hit
		if(wi->damage <= 0){
			do_damage = 0;
		}

		// increment collision count
		r_coll_count++;		

		// play the impact sound
		if(first_hit){
			snd_play_3d( &Snds[wi->impact_snd], &b->f_collisions[idx].cinfo.hit_point_world, &Eye_position );
		}

		// do damage
		if(do_damage && !physics_paused){
			// maybe draw an explosion
			if(wi->impact_weapon_expl_index >= 0){
				int ani_handle = Weapon_explosions.GetAnim(wi->impact_weapon_expl_index, &b->f_collisions[idx].cinfo.hit_point_world, wi->impact_explosion_radius);
				particle_create( &b->f_collisions[idx].cinfo.hit_point_world, &vmd_zero_vector, 0.0f, wi->impact_explosion_radius, PARTICLE_BITMAP, ani_handle );
			}

			switch(Objects[target].type){
			case OBJ_DEBRIS:
				// hit the debris - the debris hit code takes care of checking for MULTIPLAYER_CLIENT, etc
				debris_hit(&Objects[target], &Objects[b->objnum], &b->f_collisions[idx].cinfo.hit_point_world, Weapon_info[b->weapon_info_index].damage);
				break;

			case OBJ_WEAPON:
				// detonate the missile
				Assert(Weapon_info[Weapons[Objects[target].instance].weapon_info_index].subtype == WP_MISSILE);

				if (!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER) {
					weapon_hit(&Objects[target], NULL, &Objects[target].pos);
				}
				break;

			case OBJ_ASTEROID:
				// hit the asteroid
				if (!(Game_mode & GM_MULTIPLAYER) || MULTIPLAYER_MASTER) {
					asteroid_hit(&Objects[target], &Objects[b->objnum], &b->f_collisions[idx].cinfo.hit_point_world, Weapon_info[b->weapon_info_index].damage);
				}
				break;
			case OBJ_SHIP:	
				// hit the ship - again, the innards of this code handle multiplayer cases
				// maybe vaporize ship.
				//only apply damage if the collision is not an exit collision.  this prevents twice the damage from being done, although it probably be more realistic since two holes are being punched in the ship instead of one.
				if (!b->f_collisions[idx].is_exit_collision) ship_apply_local_damage(&Objects[target], &Objects[b->objnum], &b->f_collisions[idx].cinfo.hit_point_world, beam_get_ship_damage(b, &Objects[target]), b->f_collisions[idx].quadrant);

				// if this is the first hit on the player ship. whack him
				if(do_damage)
				{
					// Goober5000 - AGH!  BAD BAD BAD BAD BAD BAD BAD BAD bug!  The whack's hit point is in *local*
					// coordinates, NOT world coordinates!
					beam_apply_whack(b, &Objects[target], &b->f_collisions[idx].cinfo.hit_point);
				}
				break;
			}		
		}				

		// if the radius of the target is somewhat close to the radius of the beam, "stop" the beam here
		// for now : if its smaller than about 1/3 the radius of the ship
		if(widest <= (Objects[target].radius * BEAM_AREA_PERCENT) && !beam_will_tool_target(b, &Objects[target])){	
			// set last_shot so we know where to properly draw the beam		
			b->last_shot = b->f_collisions[idx].cinfo.hit_point_world;
			Assert(is_valid_vec(&b->last_shot));		

			// done wif the beam
			break;
		}
	}

	// store the new recent collisions
	for(idx=0; idx<r_coll_count; idx++){
		b->r_collisions[idx] = r_coll[idx];
	}
	b->r_collision_count = r_coll_count;
}

// for a given object, and a firing beam, determine its critical dot product and range
void beam_get_cull_vals(object *objp, beam *b, float *cull_dot, float *cull_dist)
{
	switch(objp->type){
	// debris and asteroids are classified as slow moving small objects
	// use cull_dot == potential cone of beam + 10% and 50.0 meters
	case OBJ_DEBRIS:
	case OBJ_ASTEROID:
		*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.10f);
		*cull_dist = 50.0f * 50.0f;
		return;

	// treat missiles as fast-moving small objects
	case OBJ_WEAPON:
		*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.5f);
		*cull_dist = 300.0f * 300.0f;
		return;

	case OBJ_SHIP:
		// for cap ships, only cull for 90deg or better
		/*
		if(Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_CAPITAL){
			*cull_dot = 0.0f;
			*cull_dist = 0.0f;
			return;
		}
		*/

		// for large ships, cull at some multiple of the radius
		if(Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)){
			*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.25f);
			
			*cull_dist = (objp->radius * 1.3f) * (objp->radius * 1.3f);
			return;
		}

		// for everthing else, cull the same as missiles
		*cull_dot = 1.0f - ((1.0f - beam_get_cone_dot(b)) * 1.5f);
		*cull_dist = 300.0f * 300.0f;
		return;
	}

	// BAD BAD BAD - but this code will cause everything to cull properly
	Int3();
	*cull_dot = 1.0f;
	*cull_dist = 0.0f;
	return;
}

// FIXME - make sure we are truthfull representing the "cone" for all beam types
// get the total possible cone for a given beam in radians
float beam_get_cone_dot(beam *b)
{
	switch(b->type){
	case BEAM_TYPE_A:
	case BEAM_TYPE_C:
	case BEAM_TYPE_D:
	case BEAM_TYPE_E:
		// even though these beams don't move, return a _very_ small value
		return (float)cos(fl_radian(50.5f));	
		
	case BEAM_TYPE_B:
		return vm_vec_dotprod(&b->binfo.dir_a, &b->binfo.dir_b);

	default:
		Int3();
	}

	Int3();
	return 0.0f;
}

// if it is legal for the beam to fire, or continue firing
int beam_ok_to_fire(beam *b)
{
	// if my own object is invalid, stop firing
	if(b->objp->signature != b->sig){
		mprintf(("BEAM : killing beam because of invalid parent object SIGNATURE!\n"));
		return -1;
	}

	// if my own object is a ghost
	if(b->objp->type != OBJ_SHIP){
		mprintf(("BEAM : killing beam because of invalid parent object TYPE!\n"));
		return -1;
	}	

	// type C beams are ok to fire all the time
	if(b->type == BEAM_TYPE_C){
		ship *shipp = &Ships[b->objp->instance];
		if(shipp->weapon_energy < 0.0){
//			shipp->weapons.next_primary_fire_stamp[b->bank] = timestamp(Weapon_info[shipp->weapons.primary_bank_weapons[b->bank]].b_info.beam_warmdown*2);
			shipp->weapons.next_primary_fire_stamp[b->bank] = timestamp(2000);
			int ship_maybe_play_primary_fail_sound();
			if ( &Objects[shipp->objnum] == Player_obj )ship_maybe_play_primary_fail_sound();
			mprintf(("killing fighter beam becase it ran out of energy\n"));
			return 0;
		}else return 1;
	}

	// if the shooting turret is destroyed	
	if(b->subsys->current_hits <= 0.0f){		
		mprintf(("BEAM : killing beam because turret has been destroyed!\n"));
		return -1;
	}
	
	//kill it if its disrupted
	if (ship_subsys_disrupted(b->subsys))
	{
		return -1;
	}

	// if the beam will be firing out of its FOV, power it down
	vec3d aim_dir, temp;
	vec3d turret_dir, turret_pos;
	vm_vec_sub(&aim_dir, &b->last_shot, &b->last_start);
	vm_vec_normalize(&aim_dir);
	beam_get_global_turret_gun_info(b->objp, b->subsys, &turret_pos, &turret_dir, 1, &temp, b->fighter_beam);
	if(vm_vec_dotprod(&aim_dir, &turret_dir) < b->subsys->system_info->turret_fov){
		mprintf(("BEAM : powering beam down because of FOV condition!\n"));
		return 0;
	}

	// ok to fire/continue firing
	return 1;
}

// get the width of the widest section of the beam
float beam_get_widest(beam *b)
{
	int idx;
	float widest = -1.0f;

	// sanity
	Assert(b->weapon_info_index >= 0);
	if(b->weapon_info_index < 0){
		return -1.0f;
	}

	// lookup
	for(idx=0; idx<Weapon_info[b->weapon_info_index].b_info.beam_num_sections; idx++){
		if(Weapon_info[b->weapon_info_index].b_info.sections[idx].width > widest){
			widest = Weapon_info[b->weapon_info_index].b_info.sections[idx].width;
		}
	}

	// return	
	return widest * b->shrink;
}  

// apply a whack to a ship
void beam_apply_whack(beam *b, object *objp, vec3d *hit_point)
{
	weapon_info *wip;	
	ship *shipp;

	// sanity
	Assert((b != NULL) && (objp != NULL) && (hit_point != NULL));
	if((b == NULL) || (objp == NULL) || (hit_point == NULL)){
		return;
	}	
	Assert(b->weapon_info_index >= 0);
	wip = &Weapon_info[b->weapon_info_index];	
	Assert((objp != NULL) && (objp->type == OBJ_SHIP) && (objp->instance >= 0) && (objp->instance < MAX_SHIPS));
	if((objp == NULL) || (objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return;
	}
	shipp = &Ships[objp->instance];
	if((shipp->ai_index < 0) || (shipp->ai_index >= MAX_AI_INFO)){
		return;
	}

	// don't whack docked ships
	// Goober5000 - whacking docked ships should work now, so whack them
	// Goober5000 - weapons with no mass don't whack (bypass the calculations)
	if(/*object_is_docked(objp) ||*/ (wip->mass == 0.0f)) {
		return;
	}

	// determine how big of a whack to apply
	float whack;
	float dist;

	// this if block was added by Bobboau to make beams whack properly while preserving reverse compatibility
	if(wip->mass == 100.0f){
		if(wip->damage < b_whack_damage){
			whack = b_whack_small;
		} else {
			whack = b_whack_big;
		}
	}else{
		whack = wip->mass;
	}

	// whack direction
	vec3d whack_dir, temp;
	vm_vec_dist_to_line(&objp->pos, &b->last_start, &b->last_shot, &temp, &dist);
	vm_vec_sub(&whack_dir, &objp->pos, &temp);
	vm_vec_normalize(&whack_dir);
	vm_vec_scale(&whack_dir, whack);

	// apply the whack
	ship_apply_whack(&whack_dir, hit_point, objp);
}

// return the amount of damage which should be applied to a ship. basically, filters friendly fire damage 
float beam_get_ship_damage(beam *b, object *objp)
{	
	// if the beam is on the same team as the object
	Assert((objp != NULL) && (b != NULL));
	if((objp == NULL) || (b == NULL)){
		return 0.0f;
	}
	Assert((objp->type == OBJ_SHIP) && (objp->instance >= 0) && (objp->instance < MAX_SHIPS));
	if((objp->type != OBJ_SHIP) || (objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return 0.0f;
	}

	// Bobboau, if you're going to re-implement attenuation, this is how you should do it.
	//float dist = vm_vec_dist_quick(b->objp->pos, objp->pos);
	float attenuation = 1.0f; // or scaled by dist
	//-- Goober5000

	// same team. yikes
	if((b->team == Ships[objp->instance].team) && (Weapon_info[b->weapon_info_index].damage > The_mission.ai_profile->beam_friendly_damage_cap[Game_skill_level])){
		return The_mission.ai_profile->beam_friendly_damage_cap[Game_skill_level] * attenuation;
	}

	// normal damage
	return Weapon_info[b->weapon_info_index].damage * attenuation;
}

// if the beam is likely to tool a given target before its lifetime expires
int beam_will_tool_target(beam *b, object *objp)
{
	weapon_info *wip = &Weapon_info[b->weapon_info_index];
	float damage_in_a_few_seconds;

	// sanity
	if(objp == NULL){
		return 0;
	}

	// if the object is not a ship, bail
	if(objp->type != OBJ_SHIP){
		return 0;
	}
	if((objp->instance < 0) || (objp->instance >= MAX_SHIPS)){
		return 0;
	}

	// if the beam is going to apply more damage in about 1 and a half than the hull of the ship can take
	damage_in_a_few_seconds = (TOOLTIME / (float)BEAM_DAMAGE_TIME) * wip->damage;
	if(objp->hull_strength < damage_in_a_few_seconds){

		// tooled
		return 1;
	}

	return 0;
}

float beam_accuracy = 1.0f;
DCF(b_aim, "")
{
	dc_get_arg(ARG_FLOAT);
	beam_accuracy = Dc_arg_float;
}
DCF(beam_list, "")
{
	int idx;
	int b_count = 0;

	for(idx=0; idx<Num_weapon_types; idx++){
		if(Weapon_info[idx].wi_flags & WIF_BEAM){			
			b_count++;
			dc_printf("Beam %d : %s\n", b_count, Weapon_info[idx].name);
		}
	}
}
void beam_test(int whee)
{
	int s1, s2;
	object *orion, *fenris;
	ship_subsys *orion_turret, *fenris_turret, *fenris_radar, *orion_radar, *lookup;
	beam_fire_info f;

	nprintf(("General", "Running beam test\n"));

	// lookup some stuff 
	s1 = ship_name_lookup("GTD Orion 1");
	Assert(s1 >= 0);
	orion = &Objects[Ships[s1].objnum];
	s2 = ship_name_lookup("GTC Fenris 2");
	Assert(s2 >= 0);
	fenris = &Objects[Ships[s2].objnum];		

	// get beam weapons
	lookup = GET_FIRST(&Ships[s1].subsys_list);
	orion_turret = NULL;
	orion_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s1].subsys_list)){
		// turret		
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret07")){
			orion_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			orion_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(orion_turret != NULL);
	Assert(orion_radar != NULL);
	lookup = GET_FIRST(&Ships[s2].subsys_list);
	fenris_turret = NULL;
	fenris_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s2].subsys_list)){
		// turret
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret07")){
			fenris_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			fenris_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(fenris_turret != NULL);	
	Assert(fenris_radar != NULL);

	memset(&f, 0, sizeof(beam_fire_info));
	f.accuracy = beam_accuracy;
	f.beam_info_index = -1;
	f.beam_info_override = NULL;
	f.shooter = orion;
	f.target = fenris;
	f.target_subsys = fenris_turret;
	f.turret = orion_turret;

	// find the first beam
	int idx;	
	int beam_first = -1;
	int beam_count = 0;

	for(idx=0; idx<Num_weapon_types; idx++){
		if(Weapon_info[idx].wi_flags & WIF_BEAM){			
			beam_count++;
			if(beam_count > 1){
				beam_first = idx;
				break;
			}
		}
	}	
	if(beam_first < 0){
		return;
	}
	
	// maybe fire it, if its valid
	f.beam_info_index = beam_first + whee - 1;
	if(Weapon_info[f.beam_info_index].wi_flags & WIF_BEAM){
		HUD_printf("Firing %s\n", Weapon_info[f.beam_info_index].name);

		beam_fire(&f);
	}
}

void beam_test_new(int whee)
{
	int s1, s2, s3;
	object *orion, *fenris, *herc2, *herc3, *herc6, *alpha;
	ship_subsys *orion_turret, *fenris_turret, *fenris_radar, *orion_radar, *lookup;
	beam_fire_info f;

	nprintf(("General", "Running beam test\n"));

	// lookup some stuff 
	s1 = ship_name_lookup("GTD Orion 1");
	Assert(s1 >= 0);
	orion = &Objects[Ships[s1].objnum];
	s2 = ship_name_lookup("GTC Fenris 2");
	Assert(s2 >= 0);
	fenris = &Objects[Ships[s2].objnum];	
	s3 = ship_name_lookup("GTF Hercules 2");
	Assert(s3 >= 0);
	herc2 = &Objects[Ships[s3].objnum];
	s3 = ship_name_lookup("GTF Hercules 3");
	Assert(s3 >= 0);
	herc3 = &Objects[Ships[s3].objnum];
	s3 = ship_name_lookup("GTF Hercules 6");
	Assert(s3 >= 0);
	herc6 = &Objects[Ships[s3].objnum];
	s3 = ship_name_lookup("Alpha 1");
	Assert(s3 >= 0);
	alpha = &Objects[Ships[s3].objnum];	

	// get beam weapons
	lookup = GET_FIRST(&Ships[s1].subsys_list);
	orion_turret = NULL;
	orion_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s1].subsys_list)){
		// turret		
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret07")){
			orion_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			orion_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(orion_turret != NULL);
	Assert(orion_radar != NULL);
	lookup = GET_FIRST(&Ships[s2].subsys_list);
	fenris_turret = NULL;
	fenris_radar = NULL;
	while(lookup != END_OF_LIST(&Ships[s2].subsys_list)){
		// turret
		if((lookup->system_info->type == SUBSYSTEM_TURRET) && !subsystem_stricmp(lookup->system_info->subobj_name, "turret03")){
			fenris_turret = lookup;			
		}

		// radar
		if(lookup->system_info->type == SUBSYSTEM_RADAR){
			fenris_radar = lookup;
		}

		lookup = GET_NEXT(lookup);
	}
	Assert(fenris_turret != NULL);	
	Assert(fenris_radar != NULL);

	memset(&f, 0, sizeof(beam_fire_info));
	f.accuracy = beam_accuracy;	
	f.beam_info_override = NULL;
	f.shooter = fenris;
	f.target = alpha;
	f.target_subsys = NULL;
	f.turret = fenris_turret;
	f.num_shots = 3;

	// find the first beam
	int idx;	
	int beam_first = -1;
	int beam_count = 0;

	for(idx=0; idx<Num_weapon_types; idx++){
		if(Weapon_info[idx].wi_flags & WIF_BEAM){
			beam_count++;
			if(beam_count > 1){
				beam_first = idx;
				break;
			}			
		}
	}	
	if(beam_first < 0){
		return;
	}
	
	// maybe fire it, if its valid
	f.beam_info_index = beam_first + whee - 1;
	if(Weapon_info[f.beam_info_index].wi_flags & WIF_BEAM){
		HUD_printf("Firing %s\n", Weapon_info[f.beam_info_index].name);
		beam_fire(&f);
	}
}

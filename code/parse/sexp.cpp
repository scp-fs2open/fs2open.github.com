/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/parse/SEXP.CPP $
 * $Revision: 2.47 $
 * $Date: 2003-03-20 04:27:10 $
 * $Author: Goober5000 $
 *
 * main sexpression generator
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.46  2003/03/20 00:08:08  Goober5000
 * making sexps better
 * --Goober5000
 *
 * Revision 2.45  2003/03/19 06:23:27  Goober5000
 * added warp-effect sexp
 * --Goober5000
 *
 * Revision 2.44  2003/03/18 13:38:07  unknownplayer
 * Fixed up a bug in the code which was causing all mission loads to crash.
 * As of this update, the DX merge is officially functional and complete within the CVS main branch. Tell Inquisitor we won't need to roll back at all, but Goober5000 needs to go and look at his SEXP code since he messed up a bunch of checking functions with it
 *
 * Revision 2.43  2003/03/18 08:44:04  Goober5000
 * added explosion-effect sexp and did some other minor housekeeping
 * --Goober5000
 *
 * Revision 2.42  2003/03/03 04:28:37  Goober5000
 * fixed the tech room bug!  yay!
 * --Goober5000
 *
 * Revision 2.41  2003/03/01 01:15:38  Goober5000
 * fixed the initial status bug
 *
 * Revision 2.40  2003/01/26 18:37:19  Goober5000
 * changed change-music to change-soundtrack
 * --Goober5000
 *
 * Revision 2.39  2003/01/25 04:54:36  Goober5000
 * added preloader to change-ship-model and change-ship-class so that the game
 * doesn't hesitate when switching models in the middle of missions
 * --Goober5000
 *
 * Revision 2.38  2003/01/25 04:17:39  Goober5000
 * added change-music sexp and bumped MAX_SOUNDTRACKS from 10 to 25
 * --Ian Warfield
 *
 * Revision 2.37  2003/01/21 17:24:16  Goober5000
 * fixed a few bugs in Bobboau's implementation of the glow sexps; also added
 * help for the sexps in sexp_tree
 * --Goober5000
 *
 * Revision 2.36  2003/01/20 05:40:50  bobboau
 * added several sExps for turning glow points and glow maps on and off
 *
 * Revision 2.35  2003/01/19 22:20:22  Goober5000
 * fixed a bunch of bugs -- the support ship sexp, the "no-subspace-drive" flag,
 * and departure into hangars should now all work properly
 * --Goober5000
 *
 * Revision 2.34  2003/01/19 07:45:38  Goober5000
 * actually added the set-support-ship sexp; much of the other commit was
 * groundwork (data types and stuff)
 * --Goober5000
 *
 * Revision 2.33  2003/01/19 07:02:16  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 2.32  2003/01/18 09:25:41  Goober5000
 * fixed bug I inadvertently introduced by modifying SIF_ flags with sexps rather
 * than SF_ flags
 * --Goober5000
 *
 * Revision 2.31  2003/01/15 21:29:05  anonymous
 * fixed the demo compilation. Define FS2_DEMO globally to compile as a demo. Make sure warp.pof is in your data/models directory.
 *
 * Revision 2.30  2003/01/13 23:20:00  Goober5000
 * bug hunting; fixed the beam whack effect bug
 * --Goober5000
 *
 * Revision 2.29  2003/01/10 04:14:19  Goober5000
 * I found these two beautiful functions in ship.cpp - ship_change_model
 * and change_ship_type - so I made them into sexps :)
 * --Goober5000
 *
 * Revision 2.28  2003/01/07 20:06:44  Goober5000
 * added ai-chase-any-except sexp
 * --Goober5000
 *
 * Revision 2.27  2003/01/05 01:26:35  Goober5000
 * added capability of is-iff and change-iff to have wings as well as ships
 * as their arguments; also allowed a bunch of sexps to accept the player
 * as an argument where they would previously display a parse error
 * --Goober5000
 *
 * Revision 2.26  2003/01/04 23:15:39  Goober5000
 * fixed the order sexp
 * --Goober5000
 *
 * Revision 2.25  2003/01/03 21:58:07  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.24  2003/01/02 03:09:01  Goober5000
 * this is the way we squash the bugs, squash the bugs, squash the bugs
 * this is the way we squash the bugs, so early in the morning :p
 * --Goober5000
 *
 * Revision 2.23  2003/01/02 00:35:20  Goober5000
 * added don't-collide-invisible and collide-invisible sexps
 * --Goober5000
 *
 * Revision 2.22  2003/01/01 23:33:33  Goober5000
 * added ship-vaporize and ship-no-vaporize sexps
 * --Goober5000
 *
 * Revision 2.21  2002/12/31 18:59:43  Goober5000
 * if it ain't broke, don't fix it
 * --Goober5000
 *
 * Revision 2.20  2002/12/31 07:26:40  Goober5000
 * added damaged-escort-priority-all sexp
 * --Goober5000
 *
 * Revision 2.19  2002/12/27 20:16:18  phreak
 * added damage-escort-list as a new sexp
 *
 * Revision 2.18  2002/12/27 02:57:51  Goober5000
 * removed the existing stealth sexps and replaced them with the following...
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 2.17  2002/12/25 01:22:23  Goober5000
 * meh - changed is-cargo-x to is-cargo
 * --Goober5000
 *
 * Revision 2.16  2002/12/24 07:42:29  Goober5000
 * added change-ai-class and is-ai-class, and I think I may also have nailed the
 * is-iff bug; did some other bug hunting as well
 * --Goober5000
 *
 * Revision 2.15  2002/12/23 23:01:27  Goober5000
 * added set-cargo and is-cargo-x sexps
 * --Goober5000
 *
 * Revision 2.14  2002/12/23 05:41:08  Goober5000
 * Bah - Andsager is a moron. :) I see no reason why rand shouldn't work
 * multiple times.  It's been fixed thus.
 * --Goober5000
 *
 * Revision 2.13  2002/12/23 05:18:52  Goober5000
 * Squashed some Volition bugs! :O Some of the sexps for dealing with more than
 * one ship would return after only dealing with the first ship.
 *
 * Also added the following sexps:
 * is-ship-stealthed
 * ship-force-stealth
 * ship-force-nostealth
 * ship-remove-stealth-forcing
 *
 * They toggle the stealth flag on and off.  If a ship is forced stealthy, it won't even
 * show up for friendly ships.
 * --Goober5000
 *
 * Revision 2.12  2002/12/22 21:12:22  Goober5000
 * added primaries-depleted and primary-ammo-pct sexps -- useful for ships with
 * ballistic primaries
 * --Goober5000
 *
 * Revision 2.11  2002/12/22 17:22:47  Goober5000
 * Subcategories implemented. :) So far all that's been done is the Change menu, but other
 * subcategorizations are possible.  Here are the instructions from sexp.h...
 * "Adding more subcategories is possible with the new code.  All that needs to be done is
 * to add a #define here (a number from 0x0000 to 0x00ff ORred with the category that it
 * goes under), some appropriate case statements in get_subcategory() (in sexp.cpp) that
 * will return the subcategory for each sexp that uses it, and the submenu name in the
 * op_submenu[] array in sexp_tree.cpp."
 *
 * Please note that I rearranged a whole bunch of sexps in the Operators[] array in sexp.cpp
 * in order to make the subcategories work better, so if you get a whole bunch of differences
 * or even conflicts, just ignore them. :)
 * --Goober5000
 *
 * Revision 2.10  2002/12/21 17:58:11  Goober5000
 * rearranged the sexp list and got the preliminary subcategories working - still need to work on the actual submenu
 * --Goober5000
 *
 * Revision 2.9  2002/12/20 07:17:23  Goober5000
 * Updated the cargo-known-delay and cap-subsys-cargo-known-delay sexps to work correctly if set-scanned and set-unscanned are used repeatedly with the same ship or ship subsystem.  In most cases, this functionality will never be needed, but it's nice to know that it's here. :) However, I should point out that cap-subsys-cargo-known-delay will recognize only the first instance of a subsystem being revealed once the ship is no longer in the mission.  Here is the relevant bit I put into the sexp handling routine...
 * "Since there is no way to keep track of subsystem status once a ship has departed or has been destroyed, check the mission log.  This will work in 99.9999999% of all cases; however, if the mission designer repeatedly sets and resets the scanned status of the subsystem, the mission log will only return the first occurrence of the subsystem cargo being revealed (regardless of whether it was first hidden using set-unscanned).  Normally, ships keep track of cargo data in the subsystem struct, but once/ the ship has left the mission, the subsystem linked list is purged, causing the loss of this information.  I judged the significant rework of the subsystem code not worth the rare instance that this sexp may be required to work in this way, especially since this problem only occurs after the ship departs.  If the mission designer really needs this functionality, he or she can achieve the same result with creative combinations of event chaining and is-event-true."
 *
 * Please note that this problem only occurs with cap-subsys-cargo-known-delay, only if the subsystem scanned status is revealed more than once, and then only after the ship is no longer in the mission.  Sufficiently rare, I figured, that I could afford to not worry about it.
 * --Goober5000
 *
 * Revision 2.8  2002/12/17 03:25:30  Goober5000
 * added set-scanned and set-unscanned sexps
 *
 * PLEASE NOTE: These sexps will cause cargo-known-delay and cap-subsys-cargo-known-delay to not work correctly if set-scanned and set-unscanned are used more than once.  I'll fix this later.
 *
 * --Goober5000
 *
 * Revision 2.7  2002/12/12 08:01:57  Goober5000
 * added distance-ship-subsystem sexp
 * ~Goober5000~
 *
 * Revision 2.6  2002/11/28 00:00:37  sesquipedalian
 * end-mission sexp added
 *
 * Revision 2.5  2002/11/20 21:22:31  DTP
 * DTP; fix at line 6785 in parse/SEXP.cpp, a forgotten "{", when the is tagged thing was Fixed
 *
 * Revision 2.4  2002/11/19 02:15:50  sesquipedalian
 * Adding in EdrickV's fix to the is-tagged sexp.
 *
 * Revision 2.3  2002/10/29 22:41:48  sesquipedalian
 * no message
 *
 * Revision 2.2  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/13 09:16:18  wmcoolmon
 * Added initial code for "ship-lights-on" and "ship-lights-off" SEXPs
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/10 20:42:44  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 66    9/13/99 11:38a Andsager
 * Stupid switch for sexp_team_score.  Don't forget *break*
 * 
 * 65    9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 64    9/07/99 1:05a Andsager
 * Added team-score sexp for multi team vs team missions
 * 
 * 63    9/06/99 9:46p Jefff
 * skip mission support
 * 
 * 62    8/29/99 2:33p Andsager
 * Fix bug in sexp_is_ship_visible()
 * 
 * 61    8/27/99 9:07p Dave
 * LOD explosions. Improved beam weapon accuracy.
 * 
 * 60    8/27/99 4:07p Andsager
 * Add is-ship-visible sexp.  Make ship-vanish sexp SINGLE player only
 * 
 * 59    8/26/99 4:21p Mikek
 * Add new special-check for JimB, zeroing front shield quadrant.
 * 
 * 58    8/24/99 4:25p Andsager
 * Add ship-vanish sexp
 * 
 * 57    8/23/99 6:05p Johnson
 * Oops. Self destruct log should also go into the is-destroyed
 * sexpression.
 * 
 * 56    8/23/99 3:23p Johnson
 * Fix bug in sexp_facing2 when Player_obj does not yet existl
 * 
 * 55    8/19/99 3:30p Andsager
 * Make sexp_special_warp_dist check if ship if facing knossos device
 * 
 * 54    8/19/99 9:20a Andsager
 * Enable flashing for all guages
 * 
 * 53    8/19/99 12:31a Andsager
 * Add support guage as one that can flash
 * 
 * 52    8/18/99 12:09p Andsager
 * Add debug if message has no anim for message.  Make messages come from
 * wing leader.
 * 
 * 51    8/16/99 10:04p Andsager
 * Add special-warp-dist and special-warpout-name sexp for Knossos device
 * warpout.
 * 
 * 50    8/09/99 3:32p Andsager
 * Make has_time_elapsed work with arithmatic args.
 * 
 * 49    8/09/99 2:00p Dave
 * 2 new sexpressions.
 * 
 * 48    8/02/99 4:26p Dave
 * Added 2 new sexpressions.
 * 
 * 47    7/31/99 2:30p Dave
 * Added nifty mission message debug viewing keys.
 * 
 * 46    7/28/99 1:36p Andsager
 * Modify cargo1 to include flag CARGO_NO_DEPLETE.  Add sexp
 * cargo-no-deplete (only for BIG / HUGE).  Modify ship struct to pack
 * better.
 * 
 * 45    7/24/99 4:56p Dave
 * Added 3 new sexpressions.
 * 
 * 44    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 43    7/20/99 9:19p Andsager
 * Added facing waypoint sexp
 * 
 * 42    7/20/99 9:54a Andsager
 * Add subsys-set-random sexp
 * 
 * 41    7/19/99 12:02p Andsager
 * Allow AWACS on any ship subsystem. Fix sexp_set_subsystem_strength to
 * only blow up subsystem if its strength is > 0
 * 
 * 40    7/15/99 9:22p Andsager
 * modify secondaries_depleted sexp to report true only when all
 * secondaries gone
 * 
 * 39    7/13/99 3:37p Andsager
 * Add secondaries-depleted sexp
 * 
 * 38    7/08/99 12:06p Andsager
 * Add turret-tagged-only and turret-tagged-clear sexp.
 * 
 * 37    6/28/99 4:51p Andsager
 * Add ship-guardian sexp (does not allow ship to be killed)
 * 
 * 36    6/25/99 2:51p Andsager
 * Make event_delay, goal_delay, and subsys strength work with sexp_rand
 * 
 * 35    6/23/99 5:51p Andsager
 * Add waypoint-cap-speed.  Checkin stealth ai - inactive.
 * 
 * 34    6/16/99 10:21a Dave
 * Added send-message-list sexpression.
 * 
 * 33    6/15/99 2:53p Andsager
 * Fixed subsystem checking for AWACS type subsystems.
 * 
 * 32    6/01/99 8:35p Dave
 * Finished lockarm weapons. Added proper supercap weapons/damage. Added
 * awacs-set-radius sexpression.
 * 
 * 31    5/27/99 12:14p Andsager
 * Some fixes for live debris when more than one subsys on ship with live
 * debris.  Set subsys strength (when 0) blows off subsystem.
 * sexp_hits_left_subsystem works for SUBSYSTEM_UNKNOWN.
 * 
 * 30    5/24/99 11:28a Dave
 * Sexpression for adding/removing ships from the hud escort list.
 * 
 * 29    4/28/99 9:33a Andsager
 * Add turret-free and turret-lock (and -all) sexp.  Stargger start time
 * of beam weapons beam-free and beam-free-all.
 * 
 * 28    4/26/99 2:14p Andsager
 * Add beam-protect-ship and beam-unprotect-ship sexp.
 * 
 * 27    4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 26    4/23/99 9:45a Andsager
 * Modify rand_sexp to return the same value each time it is called on a
 * particular node.  This prevents random time delay defaulting to min.
 * 
 * 25    4/21/99 6:15p Dave
 * Did some serious housecleaning in the beam code. Made it ready to go
 * for anti-fighter "pulse" weapons. Fixed collision pair creation. Added
 * a handy macro for recalculating collision pairs for a given object.
 * 
 * 24    4/19/99 2:28p Johnson
 * DA:  Use ship_info flags, not ship_flags for ship type.  DOH!!
 * 
 * 23    4/19/99 11:46a Johnson
 * DA: DOH!
 * 
 * 22    4/19/99 11:36a Johnson
 * Don't display multiple warning messges when large ship gets more than 1
 * cargo transfer.
 * 
 * 21    4/02/99 9:55a Dave
 * Added a few more options in the weapons.tbl for beam weapons. Attempt
 * at putting "pain" packets into multiplayer.
 * 
 * 20    3/31/99 2:22p Johnson
 * Fix sexp_repair_subsystem() to accept sexp increments
 * 
 * 19    3/28/99 5:58p Dave
 * Added early demo code. Make objects move. Nice and framerate
 * independant, but not much else. Don't use yet unless you're me :)
 * 
 * 18    3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 17    3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 16    3/04/99 6:09p Dave
 * Added in sexpressions for firing beams and checking for if a ship is
 * tagged.
 * 
 * 15    3/04/99 9:22a Andsager
 * Make escort list work with ship-is-visible.  When not visible, dump,
 * when becoming visible, maybe add.
 * 
 * 14    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 13    2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 12    2/11/99 5:22p Andsager
 * Fixed bugs, generalized block Sexp_variables
 * 
 * 11    2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
 * 
 * 10    1/25/99 4:29p Andsager
 * Modify sexp_modify_variable() to handle type string. added quick out
 * for multiplayer_client
 * 
 * 9     1/25/99 8:11a Andsager
 * Add sexp_modify_variable().  Changed syntax checking to allow, adding
 * operator return type ambiguous
 * 
 * 8     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 7     1/20/99 9:37a Andsager
 * 
 * 6     1/19/99 3:57p Andsager
 * Round 2 of variables
 * 
 * 5     1/07/99 1:52p Andsager
 * Initial check in of Sexp_variables
 * 
 * 4     11/05/98 5:55p Dave
 * Big pass at reducing #includes
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
 * 321   9/20/98 7:20p Dave
 * Added CHANGE_IFF packet. 
 * 
 * 320   9/16/98 10:42a Hoffoss
 * Added sexp node counting to fsm files for end user designers.
 * 
 * 319   7/06/98 4:31p Hoffoss
 * Fixed bug in sexp parsing when formula is ( ).
 * 
 * 318   6/09/98 5:15p Lawrance
 * French/German localization
 * 
 * 317   5/26/98 11:54a Allender
 * fix multiplayer problems and sexpression crash
 * 
 * 316   5/23/98 3:16p Allender
 * work on object update packet optimizations (a new updating system).
 * Don't allow medals/promotions/badges when playing singple player
 * missions through the simulator
 * 
 * 315   5/20/98 1:34a Allender
 * fixed observer problems.  Removed some code offending BoundsCheccker.
 * Fixed medals for multiplayer
 * 
 * 314   5/17/98 12:46p Mike
 * Change special-check functions for cueing rearm suggestion.
 * 
 * 313   5/14/98 6:29p Hoffoss
 * Fixed some warnings a release rebuild all turned up.
 * 
 * 312   5/14/98 10:15a Allender
 * add optional argument to prevous-goal/event operators to specify what
 * sexpression should return when being played as a single mission
 * 
 * 311   5/08/98 10:16a Lawrance
 * Add new "ship attacking count" gauge
 * 
 * 310   4/29/98 11:39p Allender
 * fixed nasty problem with is-event-false always returning true right
 * away
 * 
 * 
 */

//	Parse a symbolic expression.
//	These are identical to Lisp functions.
//	It uses a very baggy format, allocating 16 characters per token, regardless
//	of how many are used.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

#include "parse/parselo.h"
#include "parse/sexp.h"
#include "ship/ship.h"
#include "freespace2/freespace.h"
#include "math/fix.h"
#include "weapon/weapon.h"
#include "mission/missionlog.h"
#include "mission/missionparse.h"		// for p_object definition
#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "math/vecmat.h"
#include "globalincs/linklist.h"
#include "model/model.h"				// for subsystem types
#include "ship/aigoals.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "controlconfig/controlsconfig.h"
#include "io/timer.h"
#include "ship/shiphit.h"
#include "gamesequence/gamesequence.h"
#include "stats/scoring.h"
#include "stats/medals.h"
#include "playerman/player.h"
#include "hud/hudmessage.h"
#include "hud/hud.h"
#include "missionui/redalert.h"
#include "jumpnode/jumpnode.h"
#include "hud/hudshield.h"
#include "hud/hudescort.h"
#include "weapon/beam.h"
#include "starfield/supernova.h"
#include "hud/hudets.h"
#include "math/fvi.h"
#include "ship/awacs.h"
#include "hud/hudsquadmsg.h"		// for the order sexp
#include "gamesnd/eventmusic.h"		// for change-soundtrack
#include "menuui/techmenu.h"		// for intel stuff
#include "fireball/fireballs.h"	// for explosion stuff
#include "gamesnd/gamesnd.h"
#include "render/3d.h"
#include "asteroid/asteroid.h"
#include "ship/shipfx.h"
#include "weapon/shockwave.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multi_team.h"
#endif

#define TRUE	1
#define FALSE	0


sexp_oper Operators[] = {
//   Operator, Identity, Min / Max arguments
	{ "+",					OP_PLUS,			2,	INT_MAX	},
	{ "-",					OP_MINUS,			2,	INT_MAX	},
	{ "*",					OP_MUL,				2,	INT_MAX	},
	{ "/",					OP_DIV,				2,	INT_MAX	},
	{ "mod",				OP_MOD,				2,	INT_MAX	},
	{ "rand",				OP_RAND,			2,	2	},

	{ "true",							OP_TRUE,							0,	0,			},
	{ "false",							OP_FALSE,						0,	0,			},
	{ "and",								OP_AND,							2,	INT_MAX,	},
	{ "and-in-sequence",				OP_AND_IN_SEQUENCE,			2, INT_MAX, },
	{ "or",								OP_OR,							2,	INT_MAX,	},
	{ "not",								OP_NOT,							1, 1,			},
	{ "=",								OP_EQUALS,						2,	INT_MAX,	},
	{ ">",								OP_GREATER_THAN,				2,	2,			},
	{ "<",								OP_LESS_THAN,					2,	2,			},
	{ "is-iff",							OP_IS_IFF,						2, INT_MAX,	},
	{ "is-ai-class",					OP_IS_AI_CLASS,					2, INT_MAX,	},
	{ "has-time-elapsed",			OP_HAS_TIME_ELAPSED,			1,	1,			},

	{ "is-goal-true-delay",					OP_GOAL_TRUE_DELAY,				2, 2,	},
	{ "is-goal-false-delay",				OP_GOAL_FALSE_DELAY,				2, 2,	},
	{ "is-goal-incomplete",					OP_GOAL_INCOMPLETE,				1, 1,	},
	{ "is-event-true",						OP_EVENT_TRUE,							1, 1,			},
	{ "is-event-true-delay",				OP_EVENT_TRUE_DELAY,				2, 2,	},
	{ "is-event-false",						OP_EVENT_FALSE,						1, 1,			},
	{ "is-event-false-delay",				OP_EVENT_FALSE_DELAY,			2, 2,	},
	{ "is-event-incomplete",				OP_EVENT_INCOMPLETE,				1, 1,	},
	{ "is-previous-goal-true",				OP_PREVIOUS_GOAL_TRUE,			2, 3,	},
	{ "is-previous-goal-false",			OP_PREVIOUS_GOAL_FALSE,			2, 3,	},
	{ "is-previous-goal-incomplete",		OP_PREVIOUS_GOAL_INCOMPLETE,	2, 3,	},
	{ "is-previous-event-true",			OP_PREVIOUS_EVENT_TRUE,			2, 3,	},
	{ "is-previous-event-false",			OP_PREVIOUS_EVENT_FALSE,		2, 3,	},
	{ "is-previous-event-incomplete",	OP_PREVIOUS_EVENT_INCOMPLETE,	2, 3,	},

	{ "is-destroyed",							OP_IS_DESTROYED,						1,	INT_MAX,	},
	{ "is-destroyed-delay",					OP_IS_DESTROYED_DELAY,				2,	INT_MAX,	},
	{ "is-subsystem-destroyed",			OP_IS_SUBSYSTEM_DESTROYED,			2, 2,			},
	{ "is-subsystem-destroyed-delay",	OP_IS_SUBSYSTEM_DESTROYED_DELAY,	3, 3,			},
	{ "is-disabled",							OP_IS_DISABLED,						1, INT_MAX,	},
	{ "is-disabled-delay",					OP_IS_DISABLED_DELAY,				2, INT_MAX,	},
	{ "is-disarmed",							OP_IS_DISARMED,						1, INT_MAX,	},
	{ "is-disarmed-delay",					OP_IS_DISARMED_DELAY,				2, INT_MAX,	},
	{ "has-docked",							OP_HAS_DOCKED,							3, 3,			},
	{ "has-docked-delay",					OP_HAS_DOCKED_DELAY,					4, 4,			},
	{ "has-undocked",							OP_HAS_UNDOCKED,						3, 3,			},
	{ "has-undocked-delay",					OP_HAS_UNDOCKED_DELAY,				4, 4,			},
	{ "has-arrived",							OP_HAS_ARRIVED,						1, INT_MAX,	},
	{ "has-arrived-delay",					OP_HAS_ARRIVED_DELAY,				2, INT_MAX,	},
	{ "has-departed",							OP_HAS_DEPARTED,						1, INT_MAX,	},
	{ "has-departed-delay",					OP_HAS_DEPARTED_DELAY,				2, INT_MAX,	},
	{ "are-waypoints-done",					OP_WAYPOINTS_DONE,					2, 2,			},
	{ "are-waypoints-done-delay",			OP_WAYPOINTS_DONE_DELAY,			3, 3,			},
	{ "ship-type-destroyed",				OP_SHIP_TYPE_DESTROYED,				2, 2,			},
	{ "percent-ships-destroyed",			OP_PERCENT_SHIPS_DESTROYED,		2, INT_MAX,	},
	{ "percent-ships-departed",			OP_PERCENT_SHIPS_DEPARTED,			2, INT_MAX,	},
	{ "depart-node-delay",					OP_DEPART_NODE_DELAY,				3, INT_MAX, },	
	{ "destroyed-or-departed-delay",		OP_DESTROYED_DEPARTED_DELAY,		2, INT_MAX, },	

	{ "is-cargo-known",						OP_IS_CARGO_KNOWN,					1, INT_MAX,	},
	{ "is-cargo-known-delay",				OP_CARGO_KNOWN_DELAY,				2, INT_MAX,	},
	{ "cap-subsys-cargo-known-delay",	OP_CAP_SUBSYS_CARGO_KNOWN_DELAY,	3, INT_MAX,	},
	{ "is-cargo",						OP_IS_CARGO,						2, 3 },
	{ "is-ship-visible",				OP_IS_SHIP_VISIBLE,			1, 1, },
	{ "is-ship-stealthy",				OP_IS_SHIP_STEALTHY,	1, 1, },
	{ "is-friendly-stealth-visible",		OP_IS_FRIENDLY_STEALTH_VISIBLE,	1, 1, },
	{ "is_tagged",								OP_IS_TAGGED,							1, 1			},
	{ "has-been-tagged-delay",				OP_HAS_BEEN_TAGGED_DELAY,			2, INT_MAX,	},
	{ "shield-recharge-pct",				OP_SHIELD_RECHARGE_PCT,				1, 1			},
	{ "engine-recharge-pct",				OP_ENGINE_RECHARGE_PCT,				1, 1			},
	{ "weapon-recharge-pct",				OP_WEAPON_RECHARGE_PCT,				1, 1			},
	{ "shield-quad-low",						OP_SHIELD_QUAD_LOW,					2,	2			},
	{ "primary-ammo-pct",					OP_PRIMARY_AMMO_PCT,				2,	2			},
	{ "secondary-ammo-pct",					OP_SECONDARY_AMMO_PCT,				2,	2			},
	{ "is-primary-selected",				OP_IS_PRIMARY_SELECTED,				2,	2			},
	{ "is-secondary-selected",				OP_IS_SECONDARY_SELECTED,			2,	2			},
	{ "shields-left",					OP_SHIELDS_LEFT,				1, 1, },
	{ "hits-left",						OP_HITS_LEFT,					1, 1, },
	{ "hits-left-subsystem",		OP_HITS_LEFT_SUBSYSTEM,		2, 2, },
	{ "distance",						OP_DISTANCE,					2, 2, },
	{ "distance-ship-subsystem",	OP_DISTANCE_SUBSYSTEM,	3, 3 },					// Goober5000
	{ "special-warp-dist",			OP_SPECIAL_WARP_DISTANCE,	1, 1,	},
	{ "get-object-x",				OP_GET_OBJECT_X,				1,	2	},	// Goober5000
	{ "get-object-y",				OP_GET_OBJECT_Y,				1,	2	},	// Goober5000
	{ "get-object-z",				OP_GET_OBJECT_Z,				1,	2	},	// Goober5000
	{ "time-elapsed-last-order",	OP_LAST_ORDER_TIME,			2, 2, /*INT_MAX*/ },
	{ "skill-level-at-least",		OP_SKILL_LEVEL_AT_LEAST,	1, 1, },
	{ "num-players",					OP_NUM_PLAYERS,				0, 0, },
	{ "num_kills",								OP_NUM_KILLS,							1, 1			},
	{ "num_type_kills",						OP_NUM_TYPE_KILLS,					2,	2			},
	{ "num_class_kills",						OP_NUM_CLASS_KILLS,					2,	2			},
	{ "team-score",					OP_TEAM_SCORE,					1,	1,	}, 
	{ "was-promotion-granted",				OP_WAS_PROMOTION_GRANTED,			0, 1,			},
	{ "was-medal-granted",					OP_WAS_MEDAL_GRANTED,				0, 1,			},

	{ "time-ship-destroyed",	OP_TIME_SHIP_DESTROYED,	1,	1,	},
	{ "time-ship-arrived",		OP_TIME_SHIP_ARRIVED,	1,	1,	},
	{ "time-ship-departed",		OP_TIME_SHIP_DEPARTED,	1,	1,	},
	{ "time-wing-destroyed",	OP_TIME_WING_DESTROYED,	1,	1,	},
	{ "time-wing-arrived",		OP_TIME_WING_ARRIVED,	1,	1,	},
	{ "time-wing-departed",		OP_TIME_WING_DEPARTED,	1,	1,	},
	{ "mission-time",				OP_MISSION_TIME,			0, 0,	},
	{ "time-docked",				OP_TIME_DOCKED,			3, 3, },
	{ "time-undocked",			OP_TIME_UNDOCKED,			3, 3, },

	{ "when",			OP_WHEN,	2, INT_MAX, },
	{ "cond",			OP_COND, 1, INT_MAX, },

	{ "send-message-list",			OP_SEND_MESSAGE_LIST,			4, INT_MAX	},
	{ "send-message",					OP_SEND_MESSAGE,					3, 3,			},
	{ "send-random-message",		OP_SEND_RANDOM_MESSAGE,			3, INT_MAX,	},
	{ "invalidate-goal",				OP_INVALIDATE_GOAL,				1, INT_MAX,	},
	{ "validate-goal",				OP_VALIDATE_GOAL,					1, INT_MAX,	},

	{ "add-goal",						OP_ADD_GOAL,						2, 2,			},
	{ "add-ship-goal",				OP_ADD_SHIP_GOAL,					2, 2,			},
	{ "add-wing-goal",				OP_ADD_WING_GOAL,					2, 2,			},
	{ "clear-goals",					OP_CLEAR_GOALS,					1, INT_MAX,	},
	{ "clear-ship-goals",			OP_CLEAR_SHIP_GOALS,				1, 1,			},
	{ "clear-wing-goals",			OP_CLEAR_WING_GOALS,				1, 1,			},
	{ "good-rearm-time",				OP_GOOD_REARM_TIME,				2,	2,			},
	{ "good-secondary-time",		OP_GOOD_SECONDARY_TIME,			4, 4,			},
	{ "change-iff",					OP_CHANGE_IFF,						2,	INT_MAX,	},
	{ "change-ai-class",			OP_CHANGE_AI_CLASS,				2,	INT_MAX,	},
	{ "protect-ship",				OP_PROTECT_SHIP,					1, INT_MAX,	},
	{ "unprotect-ship",				OP_UNPROTECT_SHIP,				1, INT_MAX,	},
	{ "beam-protect-ship",			OP_BEAM_PROTECT_SHIP,			1, INT_MAX,	},
	{ "beam-unprotect-ship",		OP_BEAM_UNPROTECT_SHIP,			1, INT_MAX,	},

	{ "sabotage-subsystem",			OP_SABOTAGE_SUBSYSTEM,			3, 3,			},
	{ "repair-subsystem",			OP_REPAIR_SUBSYSTEM,				3, 3,			},
	{ "set-subsystem-strength",	OP_SET_SUBSYSTEM_STRNGTH,		3, 3,			},
	{ "subsys-set-random",			OP_SUBSYS_SET_RANDOM,			3, INT_MAX	},
	{ "self-destruct",				OP_SELF_DESTRUCT,					1, INT_MAX,	},
	{ "transfer-cargo",				OP_TRANSFER_CARGO,				2, 2,			},
	{ "exchange-cargo",				OP_EXCHANGE_CARGO,				2, 2,			},
	{ "set-cargo",					OP_SET_CARGO,					2, 3,			},
	{ "jettison-cargo-delay",		OP_JETTISON_CARGO,				2, 2			},
	{ "cargo-no-deplete",			OP_CARGO_NO_DEPLETE,				1,	2			},
	{ "set-scanned",				OP_SET_SCANNED,					1, 2 },
	{ "set-unscanned",				OP_SET_UNSCANNED,					1, 2 },

	{ "ship-invulnerable",			OP_SHIP_INVULNERABLE,			1, INT_MAX	},
	{ "ship-vulnerable",				OP_SHIP_VULNERABLE,				1, INT_MAX	},
	{ "ship-guardian",				OP_SHIP_GUARDIAN,					1, INT_MAX	},
	{ "ship-no-guardian",			OP_SHIP_NO_GUARDIAN,				1, INT_MAX	},
	{ "ship-invisible",				OP_SHIP_INVISIBLE,				1, INT_MAX	},
	{ "ship-visible",				OP_SHIP_VISIBLE,					1, INT_MAX	},
	{ "ship-stealthy",				OP_SHIP_STEALTHY,			1, INT_MAX },
	{ "ship-unstealthy",			OP_SHIP_UNSTEALTHY,		1, INT_MAX },			// Goober5000
	{ "friendly-stealth-invisible",	OP_FRIENDLY_STEALTH_INVISIBLE,	1, INT_MAX },	// Goober5000
	{ "friendly-stealth-visible",	OP_FRIENDLY_STEALTH_VISIBLE,	1, INT_MAX },	// Goober5000
	{ "ship-vaporize",				OP_SHIP_VAPORIZE,				1, INT_MAX },	// Goober5000
	{ "ship-no-vaporize",			OP_SHIP_NO_VAPORIZE,			1, INT_MAX },	// Goober5000
	{ "break-warp",					OP_WARP_BROKEN,					1, INT_MAX,	},
	{ "fix-warp",						OP_WARP_NOT_BROKEN,				1, INT_MAX,	},
	{ "never-warp",					OP_WARP_NEVER,						1, INT_MAX, },
	{ "allow-warp",					OP_WARP_ALLOWED,					1, INT_MAX, },

	{ "fire-beam",						OP_BEAM_FIRE,						3, 4			},
	{ "beam-free",						OP_BEAM_FREE,						2, INT_MAX	},
	{ "beam-free-all",				OP_BEAM_FREE_ALL,					1, 1			},
	{ "beam-lock",						OP_BEAM_LOCK,						2, INT_MAX	},
	{ "beam-lock-all",				OP_BEAM_LOCK_ALL,					1, 1			},
	{ "turret-free",					OP_TURRET_FREE,					2, INT_MAX	},
	{ "turret-free-all",				OP_TURRET_FREE_ALL,				1, 1			},
	{ "turret-lock",					OP_TURRET_LOCK,					2, INT_MAX	},
	{ "turret-lock-all",				OP_TURRET_LOCK_ALL,				1, 1			},
	{ "turret-tagged-only",			OP_TURRET_TAGGED_ONLY_ALL,		1,	1			},
	{ "turret-tagged-clear",		OP_TURRET_TAGGED_CLEAR_ALL,	1,	1			},

	{ "red-alert",						OP_RED_ALERT,						0, 0			},
	{ "end-mission",			OP_END_MISSION,			0, 0,			}, //-Sesquipedalian
	{ "next-mission",					OP_NEXT_MISSION,					1, 1,			},
	{ "end-campaign",					OP_END_CAMPAIGN,					0, 0,			},
	{ "end-of-campaign",				OP_END_OF_CAMPAIGN,				0, 0,			},
	{ "grant-promotion",				OP_GRANT_PROMOTION,				0, 0,			},
	{ "grant-medal",					OP_GRANT_MEDAL,					1, 1,			},
	{ "allow-ship",					OP_ALLOW_SHIP,						1, 1,			},
	{ "allow-weapon",					OP_ALLOW_WEAPON,					1, 1,			},
	{ "tech-add-ships",				OP_TECH_ADD_SHIP,					1, INT_MAX	},
	{ "tech-add-weapons",			OP_TECH_ADD_WEAPON,				1, INT_MAX	},
	{ "tech-add-intel",				OP_TECH_ADD_INTEL,				1, INT_MAX	},	// Goober5000
	{ "tech-reset-to-default",		OP_TECH_RESET_TO_DEFAULT,		0, 0 },	// Goober5000

	{ "don't-collide-invisible",	OP_DONT_COLLIDE_INVISIBLE,		1, INT_MAX },	// Goober5000
	{ "collide-invisible",			OP_COLLIDE_INVISIBLE,			1, INT_MAX },	// Goober5000
	{ "change-ship-model",			OP_CHANGE_SHIP_MODEL,			2, INT_MAX },	// Goober5000
	{ "change-ship-class",			OP_CHANGE_SHIP_CLASS,			2, INT_MAX },	// Goober5000
	{ "deactivate-glow-points",		OP_DEACTIVATE_GLOW_POINTS,		1, INT_MAX },	//-Bobboau
	{ "activate-glow-points",		OP_ACTIVATE_GLOW_POINTS,		1, INT_MAX },	//-Bobboau
	{ "deactivate-glow-maps",		OP_DEACTIVATE_GLOW_MAPS,		1, INT_MAX },	//-Bobboau
	{ "activate-glow-maps",			OP_ACTIVATE_GLOW_MAPS,			1, INT_MAX },	//-Bobboau
	{ "deactivate-glow-point-bank",	OP_DEACTIVATE_GLOW_POINT_BANK,	2, 1+MAX_GLOW_POINTS },	//-Bobboau
	{ "activate-glow-point-bank",	OP_ACTIVATE_GLOW_POINT_BANK,	2, 1+MAX_GLOW_POINTS },	//-Bobboau

	{ "set-ship-position",			OP_SET_SHIP_POSITION,			4,	4	},	// Goober5000
	{ "set-ship-facing",			OP_SET_SHIP_FACING,				4,	4	},	// Goober5000
	{ "set-ship-facing-object",		OP_SET_SHIP_FACING_OBJECT,		2,	2	},	// Goober5000

	{ "modify-variable",				OP_MODIFY_VARIABLE,			2,	2,			},
	{ "add-remove-escort",			OP_ADD_REMOVE_ESCORT,			2, 2			},
	{ "set-support-ship",			OP_SET_SUPPORT_SHIP,			6, 6 },	// Goober5000
	{ "damaged-escort-priority",		OP_DAMAGED_ESCORT_LIST,		3, INT_MAX },					//phreak
	{ "damaged-escort-priority-all",	OP_DAMAGED_ESCORT_LIST_ALL,	1, MAX_COMPLETE_ESCORT_LIST },					// Goober5000
	{ "awacs-set-radius",			OP_AWACS_SET_RADIUS,				3,	3			},
	{ "primitive-sensors-set-range",OP_PRIMITIVE_SENSORS_SET_RANGE,	2,	2 },	// Goober5000
	{ "cap-waypoint-speed",			OP_CAP_WAYPOINT_SPEED,			2, 2			},
	{ "special-warpout-name",		OP_SET_SPECIAL_WARPOUT_NAME,	2, 2 },
	{ "ship-vanish",					OP_SHIP_VANISH,					1, INT_MAX	},
	{ "supernova-start",				OP_SUPERNOVA_START,				1,	1			},
	{ "ship-lights-on",					OP_SHIP_LIGHTS_ON,					1, 1			}, //-WMCoolmon
	{ "ship-lights-off",					OP_SHIP_LIGHTS_OFF,					1, 1			}, //-WMCoolmon
	{ "shields-on",					OP_SHIELDS_ON,					1, INT_MAX			}, //-Sesquipedalian
	{ "shields-off",					OP_SHIELDS_OFF,					1, INT_MAX			}, //-Sesquipedalian
	{ "change-soundtrack",				OP_CHANGE_SOUNDTRACK,				1, 1 },		// Goober5000	
	{ "explosion-effect",			OP_EXPLOSION_EFFECT,			11, 11 },			// Goober5000
	{ "warp-effect",				OP_WARP_EFFECT,					12, 12 },		// Goober5000
	{ "emp-effect",					OP_EMP_EFFECT,					1, 1 },			// Goober5000

	{ "error",	OP_INT3,	0, 0 },

	{ "ai-chase",					OP_AI_CHASE,					2, 2, },
	{ "ai-chase-wing",			OP_AI_CHASE_WING,				2, 2, },
	{ "ai-chase-any",				OP_AI_CHASE_ANY,				1, 1, },
	{ "ai-chase-any-except",	OP_AI_CHASE_ANY_EXCEPT,		1, 1+MAX_SPECIAL_OBJECTS, },
	{ "ai-guard",					OP_AI_GUARD,					2, 2, },
	{ "ai-guard-wing",			OP_AI_GUARD_WING,				2, 2, },
	{ "ai-destroy-subsystem",	OP_AI_DESTROY_SUBSYS,		3, 3, },
	{ "ai-disable-ship",			OP_AI_DISABLE_SHIP,			2, 2, },
	{ "ai-disarm-ship",			OP_AI_DISARM_SHIP,			2, 2, },
	{ "ai-warp",					OP_AI_WARP,						2, 2, },
	{ "ai-warp-out",				OP_AI_WARP_OUT,				1, 1, },
	{ "ai-dock",					OP_AI_DOCK,						4, 4, },
	{ "ai-undock",					OP_AI_UNDOCK,					1, 1, },
	{ "ai-waypoints",				OP_AI_WAYPOINTS,				2, 2, },
	{ "ai-waypoints-once",		OP_AI_WAYPOINTS_ONCE,		2, 2, },
	{ "ai-ignore",					OP_AI_IGNORE,					2, 2, },
	{ "ai-stay-near-ship",		OP_AI_STAY_NEAR_SHIP,		2, 2, },
	{ "ai-evade-ship",			OP_AI_EVADE_SHIP,				2, 2,	},
	{ "ai-keep-safe-distance",	OP_AI_KEEP_SAFE_DISTANCE,	1, 1, },
	{ "ai-stay-still",			OP_AI_STAY_STILL,				2, 2, },
	{ "ai-play-dead",				OP_AI_PLAY_DEAD,				1, 1, },

	{ "goals",	OP_GOALS_ID,	1, INT_MAX, },

	{ "key-pressed",				OP_KEY_PRESSED,				1,	2,			},
	{ "key-reset",					OP_KEY_RESET,					1, INT_MAX,	},
	{ "targeted",					OP_TARGETED,					1, 3,			},
	{ "speed",						OP_SPEED,						1, 1,			},
	{ "facing",						OP_FACING,						2, 2,			},
	{ "facing-waypoint",			OP_FACING2,						2, 2,			},
	{ "order",						OP_ORDER,						2, 3,			},
	{ "waypoint-missed",			OP_WAYPOINT_MISSED,			0, 0,			},
	{ "waypoint-twice",			OP_WAYPOINT_TWICE,			0, 0,			},
	{ "path-flown",				OP_PATH_FLOWN,					0, 0,			},
	{ "training-msg",				OP_TRAINING_MSG,				1, 4,			},
	{ "flash-hud-gauge",			OP_FLASH_HUD_GAUGE,			1, 1,			},
	{ "primaries-depleted",		OP_PRIMARIES_DEPLETED,		1, 1,			},
	{ "secondaries-depleted",	OP_SECONDARIES_DEPLETED,	1, 1,			},
	{ "special-check",			OP_SPECIAL_CHECK,				1, 1,			},

	{ "set-training-context-fly-path",	OP_SET_TRAINING_CONTEXT_FLY_PATH,	2, 2, },
	{ "set-training-context-speed",		OP_SET_TRAINING_CONTEXT_SPEED,		2, 2, },

	{ "do-nothing",	OP_NOP,	0, 0,			},
};

sexp_ai_goal_link Sexp_ai_goal_links[] = {
	{ AI_GOAL_CHASE, OP_AI_CHASE },
	{ AI_GOAL_CHASE_WING, OP_AI_CHASE_WING },
	{ AI_GOAL_DOCK, OP_AI_DOCK },
	{ AI_GOAL_UNDOCK, OP_AI_UNDOCK },
	{ AI_GOAL_WARP, OP_AI_WARP_OUT },
	{ AI_GOAL_WARP, OP_AI_WARP },
	{ AI_GOAL_WAYPOINTS, OP_AI_WAYPOINTS },
	{ AI_GOAL_WAYPOINTS_ONCE, OP_AI_WAYPOINTS_ONCE },
	{ AI_GOAL_DESTROY_SUBSYSTEM, OP_AI_DESTROY_SUBSYS },
	{ AI_GOAL_DISABLE_SHIP, OP_AI_DISABLE_SHIP },
	{ AI_GOAL_DISARM_SHIP, OP_AI_DISARM_SHIP },
	{ AI_GOAL_GUARD, OP_AI_GUARD },
	{ AI_GOAL_CHASE_ANY, OP_AI_CHASE_ANY },
	{ AI_GOAL_CHASE_ANY_EXCEPT, OP_AI_CHASE_ANY_EXCEPT },
	{ AI_GOAL_GUARD_WING, OP_AI_GUARD_WING },
	{ AI_GOAL_EVADE_SHIP, OP_AI_EVADE_SHIP },
	{ AI_GOAL_STAY_NEAR_SHIP, OP_AI_STAY_NEAR_SHIP },
	{ AI_GOAL_KEEP_SAFE_DISTANCE, OP_AI_KEEP_SAFE_DISTANCE },
	{ AI_GOAL_IGNORE, OP_AI_IGNORE },
	{ AI_GOAL_STAY_STILL, OP_AI_STAY_STILL },
	{ AI_GOAL_PLAY_DEAD, OP_AI_PLAY_DEAD },
};

char *HUD_gauge_text[NUM_HUD_GAUGES] = 
{
	"LEAD_INDICATOR",
	"ORIENTATION_TEE",
	"HOSTILE_TRIANGLE",
	"TARGET_TRIANGLE",
	"MISSION_TIME",
	"RETICLE_CIRCLE",
	"THROTTLE_GAUGE",
	"RADAR",
	"TARGET_MONITOR",
	"CENTER_RETICLE",
	"TARGET_MONITOR_EXTRA_DATA",
	"TARGET_SHIELD_ICON",
	"PLAYER_SHIELD_ICON",
	"ETS_GAUGE",
	"AUTO_TARGET",
	"AUTO_SPEED",
	"WEAPONS_GAUGE",
	"ESCORT_VIEW",
	"DIRECTIVES_VIEW",
	"THREAT_GAUGE",
	"AFTERBURNER_ENERGY",
	"WEAPONS_ENERGY",
	"WEAPON_LINKING_GAUGE",
	"TARGER_MINI_ICON",
	"OFFSCREEN_INDICATOR",
	"TALKING_HEAD",
	"DAMAGE_GAUGE",
	"MESSAGE_LINES",
	"MISSILE_WARNING_ARROW",
	"CMEASURE_GAUGE",
	"OBJECTIVES_NOTIFY_GAUGE",
	"WINGMEN_STATUS",
	"OFFSCREEN RANGE",
	"KILLS GAUGE",
	"ATTACKING TARGET COUNT",
	"TEXT FLASH",
	"MESSAGE BOX",
#ifndef FS2_DEMO
	"SUPPORT GUAGE",
	"LAG GUAGE"
#endif
};


int	Directive_count;
int	Sexp_useful_number;  // a variable to pass useful info in from external modules
int	Locked_sexp_true, Locked_sexp_false;
int	Num_operators = sizeof(Operators) / sizeof(sexp_oper);
int	Num_sexp_ai_goal_links = sizeof(Sexp_ai_goal_links) / sizeof(sexp_ai_goal_link);
int	Sexp_build_flag;
int	Sexp_clipboard = -1;  // used by Fred
int	Training_context = 0;
int	Training_context_speed_set;
int	Training_context_speed_min;
int	Training_context_speed_max;
int	Training_context_speed_timestamp;
int	Training_context_path;
int	Training_context_goal_waypoint;
int	Training_context_at_waypoint;
float	Training_context_distance;
char	Sexp_error_text[MAX_SEXP_TEXT];
char	*Sexp_string; //[1024] = {0};
sexp_node Sexp_nodes[MAX_SEXP_NODES];
sexp_variable Sexp_variables[MAX_SEXP_VARIABLES];

int Players_target = UNINITIALIZED;
ship_subsys *Players_targeted_subsys;
int Players_target_timestamp;

int get_sexp(char *token);
int eval_sexp(int cur_node);
void build_extended_sexp_string(int cur_node, int level, int mode);
void update_sexp_references(char *old_name, char *new_name, int format, int node);
int sexp_determine_team(char *subj);
int sexp_distance2(int obj1, char *subj);
int sexp_distance3(int obj1, int obj2);
int extract_sexp_variable_index(int node);
void init_sexp_vars();
int num_eval(int node);

void init_sexp()
{
	int i;

	for (i=0; i<MAX_SEXP_NODES; i++) {
		if ( !(Sexp_nodes[i].type & SEXP_FLAG_PERSISTENT) ){
			Sexp_nodes[i].type = SEXP_NOT_USED;
		}
	}

	init_sexp_vars();

	Locked_sexp_false = Locked_sexp_true = -1;
	Locked_sexp_false = alloc_sexp("false", SEXP_LIST, SEXP_ATOM_OPERATOR, -1, -1);
	Assert(Locked_sexp_false != -1);
	Sexp_nodes[Locked_sexp_false].type = SEXP_ATOM;  // fix bypassing value
	Locked_sexp_true = alloc_sexp("true", SEXP_LIST, SEXP_ATOM_OPERATOR, -1, -1);
	Assert(Locked_sexp_true != -1);
	Sexp_nodes[Locked_sexp_true].type = SEXP_ATOM;  // fix bypassing value
}

// allocates an sexp node.
int alloc_sexp(char *text, int type, int subtype, int first, int rest)
{
	int i;

	i = find_operator(text);
	if ((i == OP_TRUE) && (type == SEXP_ATOM) &&	(subtype == SEXP_ATOM_OPERATOR)) {
		return Locked_sexp_true;
	} else if ((i == OP_FALSE) && (type == SEXP_ATOM) && (subtype == SEXP_ATOM_OPERATOR)) {
		return Locked_sexp_false;
	}

	i = find_free_sexp();
	Assert(i != Locked_sexp_true);
	Assert(i != Locked_sexp_false);
	if (i == MAX_SEXP_NODES){
		return -1;
	}

	Assert(strlen(text) < TOKEN_LENGTH);
	strcpy(Sexp_nodes[i].text, text);
	Assert(type >= 0);
	Sexp_nodes[i].type = type;
	Sexp_nodes[i].subtype = subtype;
	Sexp_nodes[i].first = first;
	Sexp_nodes[i].rest = rest;
	Sexp_nodes[i].value = SEXP_UNKNOWN;
	return i;
}

int Sexp_hwm = 0;

int count_free_sexp_nodes()
{
	int i, f = 0, p = 0;

	for (i=0; i<MAX_SEXP_NODES; i++) {
		if (Sexp_nodes[i].type == SEXP_NOT_USED){
			f++;
		} else if (Sexp_nodes[i].type & SEXP_FLAG_PERSISTENT){
			p++;
		}
	}

	if (MAX_SEXP_NODES - f > Sexp_hwm) {
		nprintf(("Sexp", "Sexp nodes: Free=%d, Used=%d, Persistent=%d\n", f, MAX_SEXP_NODES - f, p));
		Sexp_hwm = MAX_SEXP_NODES - f;
	}

	return f;
}

// find the next free sexp and return it's index.
int find_free_sexp()
{
	int i;

	for (i=0; i<MAX_SEXP_NODES; i++){
		if (Sexp_nodes[i].type == SEXP_NOT_USED){
			break;
		}
	}

#ifndef NDEBUG
	//count_free_sexp_nodes();
#endif

	Assert(i != MAX_SEXP_NODES);  // time to raise the limit..
	if (i == MAX_SEXP_NODES){
		return -1;
	}

	return i;
}

// sexp_mark_persistent() marks a whole sexp tree with the persistent flag so that it won't
// get re-used between missions
void sexp_mark_persistent( int n )
{
	if (n == -1){
		return;
	}

	// total hack because of the true/false locked sexps -- we should make those persistent as well
	if ( (n == Locked_sexp_true) || (n == Locked_sexp_false) ){
		return;
	}

	Assert( !(Sexp_nodes[n].type & SEXP_FLAG_PERSISTENT) );
	Sexp_nodes[n].type |= SEXP_FLAG_PERSISTENT;

	sexp_mark_persistent(Sexp_nodes[n].first);
	sexp_mark_persistent(Sexp_nodes[n].rest);

}

// sexp_unmark_persistent() removes the persistent flag from all nodes in the tree
void sexp_unmark_persistent( int n )
{
	if (n == -1){
		return;
	}

	if ( (n == Locked_sexp_true) || (n == Locked_sexp_false) ){
		return;
	}

	Assert( Sexp_nodes[n].type & SEXP_FLAG_PERSISTENT );
	Sexp_nodes[n].type &= ~SEXP_FLAG_PERSISTENT;

	sexp_unmark_persistent(Sexp_nodes[n].first);
	sexp_unmark_persistent(Sexp_nodes[n].rest);
}

// just frees up the specified sexp node,  Leaves link chains untouched.
int free_one_sexp(int num)
{
	Assert(Sexp_nodes[num].type != SEXP_NOT_USED);  // make sure it is actually used
	Assert( !(Sexp_nodes[num].type & SEXP_FLAG_PERSISTENT) );

	if ((num == Locked_sexp_true) || (num == Locked_sexp_false)){
		return 0;
	}

	Sexp_nodes[num].type = SEXP_NOT_USED;
	return 1;
}

// frees a used sexp node, so it can be reused later.  Should only be called on
// an atom or a list, and not an operator.  If on a list, the list and everything
// in it will be freed (including the operator).
int free_sexp(int num)
{
	int i, rest, count = 0;

	Assert(Sexp_nodes[num].type != SEXP_NOT_USED);  // make sure it is actually used
	Assert( !(Sexp_nodes[num].type & SEXP_FLAG_PERSISTENT) );

	if ((num == Locked_sexp_true) || (num == Locked_sexp_false) || (num == -1) ){
		return 0;
	}

	Sexp_nodes[num].type = SEXP_NOT_USED;
	count++;

	i = Sexp_nodes[num].first;
	while (i != -1){
		count += free_sexp(i);
		i = Sexp_nodes[i].rest;
	}

	rest = Sexp_nodes[num].rest;
	for (i=0; i<MAX_SEXP_NODES; i++) {
		if (Sexp_nodes[i].first == num){
			Sexp_nodes[i].first = rest;
		}

		if (Sexp_nodes[i].rest == num){
			Sexp_nodes[i].rest = rest;
		}
	}

	return count;  // total elements freed up.
}

// used to free up an entire sexp tree.  Because the root node is an operator, instead of
// a list, we can't simply call free_sexp().  This function should only be called on the
// root node of an sexp, otherwise the linking will get screwed up.
int free_sexp2(int num)
{	
	int i, count = 0;

	if ((num == -1) || (num == Locked_sexp_true) || (num == Locked_sexp_false)){
		return 0;
	}

	i = Sexp_nodes[num].rest;
	while (i != -1) {
		count += free_sexp(i);
		i = Sexp_nodes[i].rest;
	}

	count += free_sexp(num);
	return count;
}

// This function resets the status of all the nodes in a tree, forcing them to all be
// evaulated again.
void flush_sexp_tree(int node)
{
	if (node < 0){
		return;
	}

	Sexp_nodes[node].value = SEXP_UNKNOWN;
	flush_sexp_tree(Sexp_nodes[node].first);
	flush_sexp_tree(Sexp_nodes[node].rest);
}

int verify_sexp_tree(int node)
{
	if (node == -1){
		return 0;
	}

	if ((Sexp_nodes[node].type == SEXP_NOT_USED) ||
		(Sexp_nodes[node].first == node) ||
		(Sexp_nodes[node].rest == node)) {
		Error(LOCATION, "Sexp node is corrupt");
		return -1;
	}

	if (Sexp_nodes[node].first != -1){
		verify_sexp_tree(Sexp_nodes[node].first);
	}
	if (Sexp_nodes[node].rest != -1){
		verify_sexp_tree(Sexp_nodes[node].rest);
	}

	return 0;
}

int dup_sexp_chain(int node)
{
	int cur, first, rest;

	if (node == -1){
		return -1;
	}

	// TODO - CASE OF SEXP VARIABLES - ONLY 1 COPY OF VARIABLE
	first = dup_sexp_chain(Sexp_nodes[node].first);
	rest = dup_sexp_chain(Sexp_nodes[node].rest);
	cur = alloc_sexp(Sexp_nodes[node].text, Sexp_nodes[node].type, Sexp_nodes[node].subtype, first, rest);

	if (cur == -1) {
		if (first != -1){
			free_sexp(first);
		}
		if (rest != -1){
			free_sexp(rest);
		}
	}

	return cur;
}

// returns 1 if they are the same, 0 if different
int cmp_sexp_chains(int node1, int node2)
{
	if ((node1 == -1) && (node2 == -1)){
		return 1;
	}

	if ((node1 == -1) || (node2 == -1)){
		return 0;
	}

	// DA: 1/7/99 Need to check the actual Sexp_node.text, not possible variable, which can be equal
	if (stricmp(Sexp_nodes[node1].text, Sexp_nodes[node2].text)){
		return 0;
	}

	if (!cmp_sexp_chains(Sexp_nodes[node1].first, Sexp_nodes[node2].first)){
		return 0;
	}

	if (!cmp_sexp_chains(Sexp_nodes[node1].rest, Sexp_nodes[node2].rest)){
		return 0;
	}

	return 1;
}

// determine if an sexp node is within the given sexp chain.
int query_node_in_sexp(int node, int sexp)
{
	if (sexp == -1){
		return 0;
	}
	if (node == sexp){
		return 1;
	}

	if (query_node_in_sexp(node, Sexp_nodes[sexp].first)){
		return 1;
	}
	if (query_node_in_sexp(node, Sexp_nodes[sexp].rest)){
		return 1;
	}

	return 0;
}

// find the index of the list associated with an operator
int find_sexp_list(int num)
{
	int i;

	for (i=0; i<MAX_SEXP_NODES; i++){
		if (Sexp_nodes[i].first == num){
			return i;
		}
	}

	return -1;
}

// find index of operator that item is an argument of.
int find_parent_operator(int num)
{
	int i;

	if (Sexp_nodes[num].subtype == SEXP_ATOM_OPERATOR){
		num = find_sexp_list(num);
	}

	while (Sexp_nodes[num].subtype != SEXP_ATOM_OPERATOR) {
		for (i=0; i<MAX_SEXP_NODES; i++){
			if (Sexp_nodes[i].rest == num){
				break;
			}
		}

		if (i == MAX_SEXP_NODES){
			return -1;  // not found, probably at top node already.
		}

		num = i;
	}

	return num;
}

// function to determine if an sexpression node is the top level node of an sexpression tree.  Top
// level nodes do not have their node id in anyone elses first or rest index
int is_sexp_top_level( int node )
{
	int i;

	if ( Sexp_nodes[node].type == SEXP_NOT_USED ){
		return 0;
	}

	for (i = 0; i < MAX_SEXP_NODES; i++ ) {
		if ( (Sexp_nodes[i].type == SEXP_NOT_USED) || ( i == node ) ){				// don't check myself or unused nodes
			continue;
		}
		if ( (Sexp_nodes[i].first == node) || (Sexp_nodes[i].rest == node) ){
			return 0;
		}
	}

	return 1;
}

int identify_operator(char *token)
{
	int	i;

	for (i=0; i<Num_operators; i++){
		if (!stricmp(token, Operators[i].text)){
			return i;
		}
	}

	return -1;
}

int find_operator(char *token)
{
	int	i;

	for (i=0; i<Num_operators; i++){
		if (!stricmp(token, Operators[i].text)){
			return Operators[i].value;
		}
	}

	return 0;
}

int query_sexp_args_count(int index)
{
	int count = 0;

	while (Sexp_nodes[index].rest != -1){
		count++;
		index = Sexp_nodes[index].rest;
	}

	return count;
}

// returns 0 if ok, negative if there's an error in expression..
// See the returns types in sexp.h

int check_sexp_syntax(int index, int return_type, int recursive, int *bad_index, int mode)
{
	int i = 0, z, t, type, argnum = 0, count, op, type2 = 0, op2;
	int op_index;

	Assert(index >= 0 && index < MAX_SEXP_NODES);
	Assert(Sexp_nodes[index].type != SEXP_NOT_USED);
	if (Sexp_nodes[index].subtype == SEXP_ATOM_NUMBER && return_type == OPR_BOOL) {
		// special case Mark seems to want supported
		Assert(Sexp_nodes[index].first == -1);  // only lists should have a first pointer
		if (Sexp_nodes[index].rest != -1)  // anything after the number?
			return SEXP_CHECK_NONOP_ARGS; // if so, it's a syntax error

		return 0;
	}

	op_index = index;		// save the index of the operator since we need to get to other args.
	if (bad_index)
		*bad_index = op_index;

	if (Sexp_nodes[op_index].subtype != SEXP_ATOM_OPERATOR)
		return SEXP_CHECK_OP_EXPECTED;  // not an operator, which it should always be

	op = identify_operator(CTEXT(op_index));
	if (op == -1)
		return SEXP_CHECK_UNKNOWN_OP;  // unrecognized operator

	//special case - OPR_AMBIGUOUS matches all
	if (return_type != OPR_AMBIGUOUS) {
		if (query_operator_return_type(op) != return_type) {
			return SEXP_CHECK_TYPE_MISMATCH;
		}
	} 

	count = query_sexp_args_count(op_index);
	if (count < Operators[op].min || count > Operators[op].max)
		return SEXP_CHECK_BAD_ARG_COUNT;  // incorrect number of arguments

	index = Sexp_nodes[op_index].rest;
	while (index != -1) {
		type = query_operator_argument_type(op, argnum);
		Assert(Sexp_nodes[index].type != SEXP_NOT_USED);
		if (bad_index)
			*bad_index = index;

		if (Sexp_nodes[index].subtype == SEXP_ATOM_LIST) {
			i = Sexp_nodes[index].first;
			if (bad_index)
				*bad_index = i;
			
			// be sure to check to see if this node is a list of stuff and not an actual operator type
			// thing.  (i.e. in the case of a cond statement, the conditional will fall into this if
			// statement.  MORE TO DO HERE!!!!
			if (Sexp_nodes[i].subtype == SEXP_ATOM_LIST)
				return 0;

			op2 = identify_operator(CTEXT(i));
			if (op2 == -1)
				return SEXP_CHECK_UNKNOWN_OP;

			type2 = query_operator_return_type(op2);
			if (recursive) {
				switch (type) {
					case OPF_NUMBER:
						t = OPR_NUMBER;
						break;

					case OPF_POSITIVE:
						t = OPR_POSITIVE;
						break;

					case OPF_BOOL:
						t = OPR_BOOL;
						break;

					case OPF_NULL:
						t = OPR_NULL;
						break;

					case OPF_AI_GOAL:
						t = OPR_AI_GOAL;
						break;

					// special case for modify-variable"
					case OPF_AMBIGUOUS:
						t = OPR_AMBIGUOUS;
						break;

					default:
						return SEXP_CHECK_UNKNOWN_TYPE;  // no other return types available
				}

				if ((z = check_sexp_syntax(i, t, recursive, bad_index)) != 0) {
					return z;
				}
			}

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_NUMBER) {
			char *ptr;

			type2 = OPR_POSITIVE;
			ptr = CTEXT(index);
			if (*ptr == '-') {
				type2 = OPR_NUMBER;
				ptr++;
			}

			if (type == OPF_BOOL)  // allow numbers to be used where boolean is required.
				type2 = OPR_BOOL;

			while (*ptr) {
				if (!isdigit(*ptr))
					return SEXP_CHECK_INVALID_NUM;  // not a valid number

				ptr++;
			}

			i = atoi(CTEXT(index));
			z = find_operator(CTEXT(op_index));
			if ( (z == OP_HAS_DOCKED_DELAY) || (z == OP_HAS_UNDOCKED_DELAY) )
				if ( (argnum == 2) && (i < 1) )
					return SEXP_CHECK_NUM_RANGE_INVALID;

			z = identify_operator(CTEXT(op_index));
			if ( (query_operator_return_type(z) == OPR_AI_GOAL) && (argnum == Operators[op].min - 1) )
				if ( (i < 0) || (i > 89) )
					return SEXP_CHECK_NUM_RANGE_INVALID;

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_STRING) {
			type2 = SEXP_ATOM_STRING;

		} else {
			Assert(0);
		}

		switch (type) {
			case OPF_NUMBER:
				if ((type2 != OPR_NUMBER) && (type2 != OPR_POSITIVE)){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_POSITIVE:
				if (type2 == OPR_NUMBER){
					return SEXP_CHECK_NEGATIVE_NUM;
				}

				if (type2 != OPR_POSITIVE){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_SHIP_NOT_PLAYER:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (ship_name_lookup(CTEXT(index), 0) < 0) {
					if (Fred_running || mission_parse_ship_arrived(CTEXT(index))){  // == 0 when still on arrival list
						return SEXP_CHECK_INVALID_SHIP;
					}
				}

				break;

			case OPF_SHIP:
			case OPF_SHIP_POINT:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (ship_name_lookup(CTEXT(index), 1) < 0) {
					if (Fred_running || mission_parse_ship_arrived(CTEXT(index))) {		// == 0 when still on arrival list
						if (type == OPF_SHIP){													// return invalid ship if not also looking for point
							return SEXP_CHECK_INVALID_SHIP;
						}

						if (waypoint_lookup(CTEXT(index)) < 0){
							if (verify_vector(CTEXT(index))){						// verify return non-zero on invalid point
								return SEXP_CHECK_INVALID_POINT;
							}
						}
					}
				}

				break;

			case OPF_WING:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (wing_name_lookup(CTEXT(index), 1) < 0){
					return SEXP_CHECK_INVALID_WING;
				}

				break;

			case OPF_SHIP_WING:
			case OPF_SHIP_WING_POINT:
				if ( type2 != SEXP_ATOM_STRING ){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if ((ship_name_lookup(CTEXT(index), 1) < 0) && (wing_name_lookup(CTEXT(index), 1) < 0)) {
					if (Fred_running || mission_parse_ship_arrived(CTEXT(index))) {		// == 0 when still on arrival list
						if (type != OPF_SHIP_WING_POINT){									// return invalid if not also looking for point
							return SEXP_CHECK_INVALID_SHIP_WING;
						}

						if (waypoint_lookup(CTEXT(index)) < 0){
							if (verify_vector(CTEXT(index))){  // non-zero on verify vector mean invalid!
								if (!sexp_determine_team(CTEXT(index))){
									return SEXP_CHECK_INVALID_POINT;
								}
							}
						}
					}
				}

				break;

			case OPF_AWACS_SUBSYSTEM:
			case OPF_SUBSYSTEM:
			{
				char *shipname;
				int shipnum,ship_class, i;
				int ship_index;				

				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				// we must get the model of the ship that is part of this sexpression and find a subsystem
				// with that name.  This code assumes by default that the ship is *always* the first name
				// in the sexpression.  If this is ever not the case, the code here must be changed to
				// get the correct ship name.
				switch(Operators[identify_operator(CTEXT(op_index))].value)
				{
					case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
					case OP_DISTANCE_SUBSYSTEM:
					case OP_SET_CARGO:
					case OP_IS_CARGO:
					case OP_CHANGE_AI_CLASS:
					case OP_IS_AI_CLASS:
						ship_index = Sexp_nodes[Sexp_nodes[op_index].rest].rest;
						break;

					case OP_BEAM_FIRE:
						if(argnum == 1){
							ship_index = Sexp_nodes[op_index].rest;
						} else {
							ship_index = Sexp_nodes[Sexp_nodes[Sexp_nodes[op_index].rest].rest].rest;
						}
						break;
	
					default :
						ship_index = Sexp_nodes[op_index].rest;
						break;
				}

				shipname = CTEXT(ship_index);
				shipnum = ship_name_lookup(shipname, 1);
				if (shipnum >= 0){
					ship_class = Ships[shipnum].ship_info_index;
				} else {
					// must try to find the ship in the ship_arrival_list
					p_object *parse_obj;

					parse_obj = mission_parse_get_arrival_ship( shipname );
					if ( parse_obj == NULL ){
						return SEXP_CHECK_INVALID_SHIP;
					}

					ship_class = parse_obj->ship_class;
				}

				// check for the special "hull" value
				if ( (Operators[op].value == OP_SABOTAGE_SUBSYSTEM) || (Operators[op].value == OP_REPAIR_SUBSYSTEM) || (Operators[op].value == OP_SET_SUBSYSTEM_STRNGTH) ) {
					if ( !stricmp( CTEXT(index), SEXP_HULL_STRING) ){
						break;
					}
				}

				for (i=0; i<Ship_info[ship_class].n_subsystems; i++)
				{
					if (!stricmp(Ship_info[ship_class].subsystems[i].subobj_name, CTEXT(index)))
					{
						break;
					}
				}

				if (i == Ship_info[ship_class].n_subsystems)
				{
					return SEXP_CHECK_INVALID_SUBSYS;
				}

				// if we're checking for an AWACS subsystem and this is not an awacs subsystem
				if(Fred_running)
				{
					if((type == OPF_AWACS_SUBSYSTEM) && !(Ship_info[ship_class].subsystems[i].flags & MSS_FLAG_AWACS))
					{
						return SEXP_CHECK_INVALID_SUBSYS;
					}
				}

				break;
			}
			return SEXP_CHECK_UNKNOWN_ERROR;	// just in case of something going wrong - it won't trickle down

			case OPF_POINT:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (waypoint_lookup(CTEXT(index)) < 0)
				{
					if (verify_vector(CTEXT(index)))
					{
						return SEXP_CHECK_INVALID_POINT;
					}
				}

				break;

			case OPF_IFF:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<Num_team_names; i++)
				{
					if (!stricmp(Team_names[i], CTEXT(index)))
					{
						break;
					}
				}

				if (i == Num_team_names)
				{
					return SEXP_CHECK_INVALID_IFF;
				}

				break;

			case OPF_AI_CLASS:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<Num_ai_classes; i++)
				{
					if (!stricmp(Ai_class_names[i], CTEXT(index)))
					{
						break;
					}
				}

				if (i == Num_ai_classes)
				{
					return SEXP_CHECK_INVALID_AI_CLASS;
				}

				break;

			case OPF_ARRIVAL_LOCATION:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<MAX_ARRIVAL_NAMES; i++)
				{
					if (!stricmp(Arrival_location_names[i], CTEXT(index)))
					{
						break;
					}
				}

				if (i == MAX_ARRIVAL_NAMES)
				{
					return SEXP_CHECK_INVALID_ARRIVAL_LOCATION;
				}

				break;

			case OPF_DEPARTURE_LOCATION:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<MAX_DEPARTURE_NAMES; i++)
				{
					if (!stricmp(Departure_location_names[i], CTEXT(index)))
					{
						break;
					}
				}

				if (i == MAX_DEPARTURE_NAMES)
				{
					return SEXP_CHECK_INVALID_DEPARTURE_LOCATION;
				}

				break;

			case OPF_ARRIVAL_ANCHOR_ALL:
				if (type2 != SEXP_ATOM_STRING)
				{
					return SEXP_CHECK_TYPE_MISMATCH;
				}
				else
				{
					int valid = 0;

					for (i=0; i<MAX_SPECIAL_ARRIVAL_ANCHORS; i++)
					{
						if (!stricmp(Special_arrival_anchor_names[i], CTEXT(index)))
						{
							valid = 1;
							break;
						}
					}

					if (ship_name_lookup(CTEXT(index), 1) >= 0)
					{
						valid = 1;
					}

					if (!Fred_running && !mission_parse_ship_arrived(CTEXT(index)))	// 0 when on arrival list
					{
						valid = 1;
					}

					if (!valid)
					{
						return SEXP_CHECK_INVALID_ARRIVAL_ANCHOR_ALL;
					}
				}

				break;

			case OPF_SOUNDTRACK_NAME:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (!stricmp(CTEXT(index), "<No Music>"))
					break;

				for (i=0; i<Num_soundtracks; i++)
				{
					if (!stricmp(CTEXT(index), Soundtracks[i].name))
					{
						break;
					}
				}

				if (i == Num_soundtracks)
					return SEXP_CHECK_INVALID_SOUNDTRACK_NAME;

				break;

			case OPF_SHIP_WITH_BAY:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (ship_name_lookup(CTEXT(index), 1) < 0)
				{
					if (Fred_running || mission_parse_ship_arrived(CTEXT(index)))	// == 0 when still on arrival list
					{
						return SEXP_CHECK_INVALID_SHIP;
					}
				}

				// ship exists at this point

				// now determine if this ship has a docking bay
				if (!ship_has_dock_bay(ship_name_lookup_absolute(CTEXT(index))))
				{
					return SEXP_CHECK_INVALID_SHIP_WITH_BAY;
				}
				break;

			case OPF_SUPPORT_SHIP_CLASS:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (!stricmp(CTEXT(index), "<any support ship class>"))
					break;

				for (i = 0; i < Num_ship_types; i++ ) {
					if ( !stricmp(CTEXT(index), Ship_info[i].name) )
					{
						if (Ship_info[i].flags & SIF_SUPPORT)
						{
							break;
						}
					}
				}

				if ( i == Num_ship_types )
					return SEXP_CHECK_INVALID_SUPPORT_SHIP_CLASS;

				break;

			case OPF_BOOL:
				if (type2 != OPR_BOOL){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_NULL:
				if (type2 != OPR_NULL){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_AI_ORDER:
				if ( type2 != SEXP_ATOM_STRING ){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				break;

			case OPF_AI_GOAL:
				if (type2 != OPR_AI_GOAL){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				if (Fred_running) {
					int ship_num, ship2, i, w = 0, z;

					ship_num = ship_name_lookup(CTEXT(Sexp_nodes[op_index].rest));
					if (ship_num < 0) {
						w = wing_name_lookup(CTEXT(Sexp_nodes[op_index].rest));
						if (w < 0) {
							if (bad_index){
								*bad_index = Sexp_nodes[op_index].rest;
							}

							return SEXP_CHECK_INVALID_SHIP;  // should have already been caught earlier, but just in case..
						}
					}

					Assert(Sexp_nodes[index].subtype == SEXP_ATOM_LIST);
					z = Sexp_nodes[index].first;
					Assert(Sexp_nodes[z].subtype != SEXP_ATOM_LIST);
					z = find_operator(CTEXT(z));
					if (ship_num >= 0) {
						if (!query_sexp_ai_goal_valid(z, ship_num)){
							return SEXP_CHECK_ORDER_NOT_ALLOWED;
						}

					} else {
						for (i=0; i<Wings[w].wave_count; i++){
							if (!query_sexp_ai_goal_valid(z, Wings[w].ship_index[i])){
								return SEXP_CHECK_ORDER_NOT_ALLOWED;
							}
						}
					}

					if ((z == OP_AI_DOCK) && (Sexp_nodes[index].rest >= 0)) {
						ship2 = ship_name_lookup(CTEXT(Sexp_nodes[index].rest));
						if ((ship_num < 0) || !ship_docking_valid(ship_num, ship2)){
							return SEXP_CHECK_DOCKING_NOT_ALLOWED;
						}
					}
				}

				// we should check the syntax of the actual goal!!!!
				z = Sexp_nodes[index].first;
				if ((z = check_sexp_syntax(z, OPR_AI_GOAL, recursive, bad_index)) != 0){
					return z;
				}

				break;

			case OPF_SHIP_TYPE:
				if (type2 != SEXP_ATOM_STRING){
					return SEXP_CHECK_TYPE_MISMATCH;
				}

				for (i=0; i<MAX_SHIP_TYPE_COUNTS; i++){
					if (!stricmp( Ship_type_names[i], CTEXT(index))){
						break;
					}
				}

				if (i == MAX_SHIP_TYPE_COUNTS){
					return SEXP_CHECK_INVALID_SHIP_TYPE;
				}

				break;

			case OPF_WAYPOINT_PATH:
				for (i=0; i<Num_waypoint_lists; i++){
					if (!stricmp(Waypoint_lists[i].name, CTEXT(index))){
						break;
					}
				}

				if (i == Num_waypoint_lists){
					return SEXP_CHECK_TYPE_MISMATCH;
				}
				break;

			case OPF_MESSAGE:
				// code commented out because of order reversing of arguments to messaging code.  Maybe
				// I'll comment it back in someday when older missions get fixed.
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {
					for (i=0; i<Num_messages; i++)
						if (!stricmp(Messages[i].name, CTEXT(index)))
							break;

					if (i == Num_messages)
						return SEXP_CHECK_UNKNOWN_MESSAGE;
				}
				
				break;

			case OPF_PRIORITY: {
				// following code must be removed since I changed the order of the operators in the send-message
				// function to take priority second.  Maybe someday, I'll require that all designers fix the
				// old missions.
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {  // should still check in Fred though..
					char *name;

					name = CTEXT(index);
					if (!stricmp(name, "low") || !stricmp(name, "normal") || !stricmp(name, "high"))
						break;

					return SEXP_CHECK_INVALID_PRIORITY;
				}

				break;
			}

			case OPF_MISSION_NAME:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {
					if (mode == SEXP_MODE_CAMPAIGN) {
						for (i=0; i<Campaign.num_missions; i++)
							if (!stricmp(CTEXT(index), Campaign.missions[i].name)) {
								if ((i != Sexp_useful_number) && (Campaign.missions[i].level >= Campaign.missions[Sexp_useful_number].level))
									return SEXP_CHECK_INVALID_LEVEL;

								break;
							}

						if (i == Campaign.num_missions)
							return SEXP_CHECK_INVALID_MISSION_NAME;

					} else {
						// mwa -- put the following if statement to prevent Fred errors for possibly valid
						// conditions.  We should do something else here!!!
						if ( (Operators[op].value == OP_PREVIOUS_EVENT_TRUE) || (Operators[op].value == OP_PREVIOUS_EVENT_FALSE) || (Operators[op].value == OP_PREVIOUS_EVENT_INCOMPLETE)
							|| (Operators[op].value == OP_PREVIOUS_GOAL_TRUE) || (Operators[op].value == OP_PREVIOUS_GOAL_FALSE) || (Operators[op].value == OP_PREVIOUS_GOAL_INCOMPLETE) )
							break;

						if (!(*Mission_filename) || stricmp(Mission_filename, CTEXT(index)))
							return SEXP_CHECK_INVALID_MISSION_NAME;
					}
				}

				break;

			case OPF_GOAL_NAME:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				// we only need to check the campaign list if running in Fred and are in campaign mode.
				// otherwise, check the set of current goals
				if ( Fred_running && (mode == SEXP_MODE_CAMPAIGN) ) {
					z = find_parent_operator(index);
					Assert(z >= 0);
					z = Sexp_nodes[z].rest;  // first argument of operator should be mission name
					Assert(z >= 0);
					for (i=0; i<Campaign.num_missions; i++)
						if (!stricmp(CTEXT(z), Campaign.missions[i].name))
							break;

					// read the goal/event list from the mission file if both num_goals and num_events
					// are < 0
					if ((Campaign.missions[i].num_goals <= 0) && (Campaign.missions[i].num_events <= 0) )
						read_mission_goal_list(i);
					
					if (i < Campaign.num_missions) {
						for (t=0; t<Campaign.missions[i].num_goals; t++)
							if (!stricmp(CTEXT(index), Campaign.missions[i].goals[t].name))
								break;

						if (t == Campaign.missions[i].num_goals)
							return SEXP_CHECK_INVALID_GOAL_NAME;
					}
				} else {
					// MWA -- short circuit evaluation of these things for now.
					if ( (Operators[op].value == OP_PREVIOUS_GOAL_TRUE) || (Operators[op].value == OP_PREVIOUS_GOAL_FALSE) || (Operators[op].value == OP_PREVIOUS_GOAL_INCOMPLETE) )
						break;

					for (i=0; i<Num_goals; i++)
						if (!stricmp(CTEXT(index), Mission_goals[i].name))
							break;

					if (i == Num_goals)
						return SEXP_CHECK_INVALID_GOAL_NAME;
				}

				break;

			case OPF_EVENT_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				// like above checking for goals, check events in the campaign only if in Fred
				// and only if in campaign mode.  Otherwise, check the current set of events
				if ( Fred_running && (mode == SEXP_MODE_CAMPAIGN) ) {
					z = find_parent_operator(index);
					Assert(z >= 0);
					z = Sexp_nodes[z].rest;  // first argument of operator should be mission name
					Assert(z >= 0);
					for (i=0; i<Campaign.num_missions; i++)
						if (!stricmp(CTEXT(z), Campaign.missions[i].name))
							break;

					// read the goal/event list from the mission file if both num_goals and num_events
					// are < 0
					if ((Campaign.missions[i].num_goals <= 0) && (Campaign.missions[i].num_events <= 0) )
						read_mission_goal_list(i);
					
					if (i < Campaign.num_missions) {
						for (t=0; t<Campaign.missions[i].num_events; t++)
							if (!stricmp(CTEXT(index), Campaign.missions[i].events[t].name))
								break;

						if (t == Campaign.missions[i].num_events)
							return SEXP_CHECK_INVALID_EVENT_NAME;
					}
				} else {
					// MWA -- short circuit evaluation of these things for now.
					if ( (Operators[op].value == OP_PREVIOUS_EVENT_TRUE) || (Operators[op].value == OP_PREVIOUS_EVENT_FALSE) || (Operators[op].value == OP_PREVIOUS_EVENT_INCOMPLETE) )
						break;

					for ( i = 0; i < Num_mission_events; i++ ) {
						if ( !stricmp(CTEXT(index), Mission_events[i].name) )
							break;
					}
					if ( i == Num_mission_events )
						return SEXP_CHECK_INVALID_EVENT_NAME;
				}
				break;

			case OPF_DOCKER_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {
					int ship_num, model, i, z;

					z = find_parent_operator(op_index);
					ship_num = ship_name_lookup(CTEXT(Sexp_nodes[z].rest), 1);
					if (ship_num < 0) {
						if (bad_index)
							*bad_index = Sexp_nodes[z].rest;

						return SEXP_CHECK_INVALID_SHIP;  // should have already been caught earlier, but just in case..
					}

					model = Ships[ship_num].modelnum;
					z = model_get_num_dock_points(model);
					for (i=0; i<z; i++)
						if (!stricmp(CTEXT(index), model_get_dock_name(model, i)))
							break;

					if (i == z)
						return SEXP_CHECK_INVALID_DOCKER_POINT;
				}

				break;

			case OPF_DOCKEE_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (Fred_running) {
					int ship_num, model, i, z;

					ship_num = ship_name_lookup(CTEXT(Sexp_nodes[op_index].rest), 1);
					if (ship_num < 0) {
						if (bad_index)
							*bad_index = Sexp_nodes[op_index].rest;

						return SEXP_CHECK_INVALID_SHIP;  // should have already been caught earlier, but just in case..
					}

					model = Ships[ship_num].modelnum;
					z = model_get_num_dock_points(model);
					for (i=0; i<z; i++)
						if (!stricmp(CTEXT(index), model_get_dock_name(model, i)))
							break;

					if (i == z)
						return SEXP_CHECK_INVALID_DOCKEE_POINT;
				}

				break;

			case OPF_WHO_FROM:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				if (*CTEXT(index) != '#') {  // not a manual source?
					//if ( !stricmp(CTEXT(index), "<any allied>") )
					//	return SEXP_CHECK_INVALID_MSG_SOURCE;

					if ( stricmp(CTEXT(index), "<any wingman>"))  // not a special token?
						if ((ship_name_lookup(CTEXT(index)) < 0) && (wing_name_lookup(CTEXT(index), 1) < 0))  // is it in the mission?
							if (Fred_running || mission_parse_ship_arrived(CTEXT(index)))  // == 0 when still on arrival list
								return SEXP_CHECK_INVALID_MSG_SOURCE;
				}

				break;
				
			case OPF_KEYPRESS:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				break;

			case OPF_CARGO:
				if (type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;
				break;

			case OPF_SKILL_LEVEL:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < NUM_SKILL_LEVELS; i++) {
					if ( !stricmp(CTEXT(index), Skill_level_names(i, 0)) )
						break;
				}
				if ( i == NUM_SKILL_LEVELS )
					return SEXP_CHECK_INVALID_SKILL_LEVEL;
				break;

			case OPF_MEDAL_NAME:
				if ( type2 != SEXP_ATOM_STRING)
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < NUM_MEDALS; i++) {
					if ( !stricmp(CTEXT(index), Medals[i].name) )
						break;
				}

				if ( i == NUM_MEDALS )
					return SEXP_CHECK_INVALID_MEDAL_NAME;
				break;

			case OPF_HUGE_WEAPON:
			case OPF_WEAPON_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < Num_weapon_types; i++ ) {
					if ( !stricmp(CTEXT(index), Weapon_info[i].name) )
						break;
				}

				if ( i == Num_weapon_types )
					return SEXP_CHECK_INVALID_WEAPON_NAME;

				// we need to be sure that for huge weapons, the WIF_HUGE flag is set
				if ( type == OPF_HUGE_WEAPON ) {
					if ( !(Weapon_info[i].wi_flags & WIF_HUGE) )
						return SEXP_CHECK_INVALID_WEAPON_NAME;
				}

				break;

			case OPF_INTEL_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < Intel_info_size; i++ ) {
					if ( !stricmp(CTEXT(index), Intel_info[i].name) )
						break;
				}

				if ( i == Intel_info_size )
					return SEXP_CHECK_INVALID_INTEL_NAME;
				
				break;
	
			case OPF_SHIP_CLASS_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for (i = 0; i < Num_ship_types; i++ ) {
					if ( !stricmp(CTEXT(index), Ship_info[i].name) )
						break;
				}

				if ( i == Num_ship_types )
					return SEXP_CHECK_INVALID_SHIP_CLASS_NAME;

				// preload the model if needed, just in case there is no other ship of this class
				// in the mission: this eliminates the slight pause during a mission when loading
				// a model that hasn't been loaded yet
				if (!Fred_running)
				{
					z = find_parent_operator(index);
					if (z == OP_CHANGE_SHIP_MODEL || z == OP_CHANGE_SHIP_CLASS)
					{
						ship_info *sip = &Ship_info[ship_info_lookup(CTEXT(index))];	// we already know this class exists
	
						sip->modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);		// use the highest detail level
					}
				}

				break;

			case OPF_HUD_GAUGE_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for ( i = 0; i < NUM_HUD_GAUGES; i++ ) {
					if ( !stricmp(CTEXT(index), HUD_gauge_text[i]) )
						break;
				}

				// if we reached the end of the list, then the name is invalid
				if ( i == NUM_HUD_GAUGES )
					return SEXP_CHECK_INVALID_GAUGE_NAME;
				
				break;

			case OPF_JUMP_NODE_NAME:
				if ( type2 != SEXP_ATOM_STRING )
					return SEXP_CHECK_TYPE_MISMATCH;

				for ( i = 0; i < Num_jump_nodes; i++ ) {
					if ( !stricmp(Jump_nodes[i].name, CTEXT(index)) )
						break;
				}

				if ( i == Num_jump_nodes )
					return SEXP_CHECK_INVALID_JUMP_NODE;

				break;


			case OPF_VARIABLE_NAME:
				if ( Fred_running ) {
					if ( get_index_sexp_variable_name(Sexp_nodes[index].text)  == -1) {
						return SEXP_CHECK_INVALID_VARIABLE;
					}
				}
				// if Fred not running anything goes
				break;

			case OPF_AMBIGUOUS:
				// type checking for modify-variable
				// string or number -- anything goes
				break;						

			default:
				Int3();  // currently unhandled argument format (so add it now)
		}

		index = Sexp_nodes[index].rest;
		argnum++;
	}

	return 0;
}

//	Stuff a string (" chars ") in *str, return length.
//	Updates Mp.
int get_string(char *str)
{
	int	len;

	len = strcspn(Mp + 1, "\"");
	strncpy(str, Mp + 1, len);
	str[len] = 0;

	Mp += len + 2;
	return len;
}


// get text to stuff into Sexp_node in case of variable
// if Fred_running - stuff Sexp_variables[].variable_name
// otherwise - stuff index into Sexp_variables array.
void get_sexp_text_for_variable(char *text, char *token)
{
	int end_index;
	int sexp_var_index;
	
	// get variable name (up to '['
	end_index = strcspn(token, "[");
	Assert( (end_index != 0) && (end_index < TOKEN_LENGTH-1) );
	strncpy(text, token, end_index);
	text[end_index] = '\0';

	if ( !Fred_running ) {
		// freespace - get index into Sexp_variables array
		sexp_var_index = get_index_sexp_variable_name(text);
		Assert(sexp_var_index != -1);
		sprintf(text, "%d", sexp_var_index);
	}
}


// returns the first sexp index of data this function allocates. (start of this sexp)
// recursive function - always sets first and then rest
int get_sexp(char *token)
{
	int start, node, last, len, op, count;
	char variable_text[TOKEN_LENGTH];

	// start - the node allocated in first instance of fuction
	// node - the node allocated in current instance of function
	// count - number of nodes allocated this instance of function [do we set last.rest or first]
	// variable - whether string or number is a variable referencing Sexp_variables

	// initialization
	start = last = -1;
	count = 0;

	ignore_white_space();
	while (*Mp != ')') {
		Assert(*Mp != EOF_CHAR);
		if (*Mp == '(') {
			// Sexp list
			Mp++;
			node = alloc_sexp("", SEXP_LIST, SEXP_ATOM_LIST, get_sexp(token), -1);

		} else if (*Mp == '\"')	{
			// Sexp string
			len = strcspn(Mp + 1, "\"");
			
			Assert(Mp[len + 1] == '\"');    // hit EOF first (unterminated string)
			Assert(len < TOKEN_LENGTH);  // token is too long.

			// check if string variable
			if ( *(Mp + 1) == SEXP_VARIABLE_CHAR ) {

				// reduce length by 1 for end \"
				int length = len - 1;
				Assert(length < 2*TOKEN_LENGTH+2);

				// start copying after skipping 1st char
				strncpy(token, Mp + 2, length);
				token[length] = 0;

				get_sexp_text_for_variable(variable_text, token);
				node = alloc_sexp(variable_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_STRING, -1, -1);
			} else {
				strncpy(token, Mp + 1, len);
				token[len] = 0;
				node = alloc_sexp(token, SEXP_ATOM, SEXP_ATOM_STRING, -1, -1);
			}

			// bump past closing \" by 1 char
			Mp += len + 2;

		} else {
			// Sexp operator or number
			len = 0;
			bool variable = false;
			while (*Mp != ')' && !is_white_space(*Mp)) {
				if ( (len == 0) && (*Mp == SEXP_VARIABLE_CHAR) ) {
					variable = true;
					Mp++;
					continue;
				}
				Assert(*Mp != EOF_CHAR);
				Assert(len < TOKEN_LENGTH - 1);
				token[len++] = *Mp++;
			}

			token[len] = 0;
			len = 0;
			op = identify_operator(token);
			if (op != -1) {
				node = alloc_sexp(token, SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, -1);
			} else {
				if ( variable ) {
					// convert token text for variable
					get_sexp_text_for_variable(variable_text, token);

					node = alloc_sexp(variable_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_NUMBER, -1, -1);
				} else {
					node = alloc_sexp(token, SEXP_ATOM, SEXP_ATOM_NUMBER, -1, -1);
				}
			}
		}

		// update links
		if (count++) {
			Assert(last != -1);
			Sexp_nodes[last].rest = node;
		} else {
			start = node;
		}

		Assert(node != -1);  // ran out of nodes.  Time to raise the MAX!
		last = node;
		ignore_white_space();
	}

	Mp++;  // skip past the ')'
	return start;
}


// Stuffs a list of sexp variables
int stuff_sexp_variable_list()
{	
	int count;
	char var_name[TOKEN_LENGTH];
	char default_value[TOKEN_LENGTH];
	char str_type[TOKEN_LENGTH];
	int index;
	int type;

	count = 0;
	required_string("$Variables:");
	ignore_white_space();

	// check for start of list
	if (*Mp != '(') {
		error_display(1, "Reading sexp variable list.  Found [%c].  Expecting '('.\n", *Mp);
		longjmp(parse_abort, 6);
	}

	Mp++;
	ignore_white_space();

	while (*Mp != ')') {
		Assert(count < MAX_SEXP_VARIABLES);

		// get index - for debug
		stuff_int(&index);
		ignore_gray_space();

		// get var_name
		get_string(var_name);
		ignore_gray_space();

		// get default_value;
		get_string(default_value);
		ignore_gray_space();

		// get type
		get_string(str_type);
		ignore_white_space();


		if (!stricmp(str_type, "number")) {
			type = SEXP_VARIABLE_NUMBER;
		} else if (!stricmp(str_type, "string")) {
			type = SEXP_VARIABLE_STRING;
		} else if (!stricmp(str_type, "block")) {
			type = SEXP_VARIABLE_BLOCK | SEXP_VARIABLE_BLOCK_EXP;
		} else {
			type = SEXP_VARIABLE_UNKNOWN;
			Int3();
		}

		count++;

		// check if variable name already exists
		if ( (type == SEXP_VARIABLE_NUMBER) || (type == SEXP_VARIABLE_STRING) ) {
			Assert(get_index_sexp_variable_name(var_name) == -1);
		}

		sexp_add_variable(default_value, var_name, type, index);
	}

	Mp++;

	return count;
}

//
void build_sexp_text_string(char *buffer, int node, int mode)
{
	if (Sexp_nodes[node].type & SEXP_FLAG_VARIABLE) {

		int sexp_variables_index = get_index_sexp_variable_name(Sexp_nodes[node].text);
		Assert(sexp_variables_index != -1);
		Assert( (Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_NUMBER) || (Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_STRING) );

		// number
		if (Sexp_nodes[node].subtype == SEXP_ATOM_NUMBER) {
			Assert(Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_NUMBER);
		
			// Error check - can be Fred or Freespace
			if (mode == SEXP_ERROR_CHECK_MODE) {
				if ( Fred_running ) {
					sprintf(buffer, "%s[%s] ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
				} else {
					sprintf(buffer, "%s[%s] ", Sexp_variables[sexp_variables_index].variable_name, Sexp_variables[sexp_variables_index].text);
				}
			} else {
				// Save as string - only  Fred
				Assert(mode == SEXP_SAVE_MODE);
				sprintf(buffer, "@%s[%s] ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
			}
		} else {
			// string
			Assert(Sexp_nodes[node].subtype == SEXP_ATOM_STRING);
			Assert(Sexp_variables[sexp_variables_index].type & SEXP_VARIABLE_STRING);

			// Error check - can be Fred or Freespace
			if (mode == SEXP_ERROR_CHECK_MODE) {
				if ( Fred_running ) {
					sprintf(buffer, "%s[%s] ", Sexp_variables[sexp_variables_index].variable_name, Sexp_variables[sexp_variables_index].text);
				} else {
					sprintf(buffer, "%s[%s] ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
				}
			} else {
				// Save as string - only Fred
				Assert(mode == SEXP_SAVE_MODE);
				sprintf(buffer, "\"@%s[%s]\" ", Sexp_nodes[node].text, Sexp_variables[sexp_variables_index].text);
			}
		}
	} else {
		// not a variable
		if (Sexp_nodes[node].subtype == SEXP_ATOM_STRING) {
			sprintf(buffer, "\"%s\" ", CTEXT(node));
		} else {
			sprintf(buffer, "%s ", CTEXT(node));
		}
	}

}


int build_sexp_string(int cur_node, int level, int mode)
{
	char	pstr[128];
	int len, offset, node;

	Sexp_build_flag = 0;
	offset = strlen(Sexp_string);
	strcat(Sexp_string, "( ");
	node = cur_node;
	while (node != -1) {
		Assert(node >= 0 && node < MAX_SEXP_NODES);
		if (Sexp_nodes[node].first == -1) {
			// build text to string
			build_sexp_text_string(pstr, node, mode);
			strcat(Sexp_string, pstr);

		} else {
			build_sexp_string(Sexp_nodes[node].first, level + 1, mode);
		}

		node = Sexp_nodes[node].rest;
	}

	strcat(Sexp_string, ") ");
	len = strlen(Sexp_string) - offset;
	if (len > 40) {
		Sexp_string[offset] = 0;
		build_extended_sexp_string(cur_node, level, mode);
		return 1;
	}

	return 0;
}

void build_extended_sexp_string(int cur_node, int level, int mode)
{
	char pstr[128];
	int i, flag = 0, node;

	strcat(Sexp_string, "( ");
	node = cur_node;
	while (node != -1) {
		if (flag)  // not the first line?
			for (i=0; i<level + 1; i++)
				strcat(Sexp_string, "   ");

		flag = 1;
		Assert(node >= 0 && node < MAX_SEXP_NODES);
		if (Sexp_nodes[node].first == -1) {
			build_sexp_text_string(pstr,node, mode);
			strcat(Sexp_string, pstr);

		} else {
			build_sexp_string(Sexp_nodes[node].first, level + 1, mode);
		}

		strcat(Sexp_string, "\n");
		node = Sexp_nodes[node].rest;
	}

	for (i=0; i<level; i++)
		strcat(Sexp_string, "   ");

	strcat(Sexp_string, ")");
}

void convert_sexp_to_string(int cur_node, char *outstr, int mode)
{
	Sexp_string = outstr;
	*outstr = 0;
	if (cur_node >= 0)
		build_sexp_string(cur_node, 0, mode);
	else
		strcpy(Sexp_string, "( )");
}

// determine if the named ship or wing hasn't arrived yet (wing or ship must be on arrival list)
int sexp_query_has_yet_to_arrive(char *name)
{
	int i;

	if (ship_query_state(name) < 0)
		return 1;

	i = wing_name_lookup(name, 1);

	// has not arrived yet, and never will arrive
	if ((i >= 0) && (Wings[i].num_waves >= 0) && (Wings[i].flags & WF_NEVER_EXISTED)){
		return 1;
	}

	// has not arrived yet
	if ((i >= 0) && (Wings[i].num_waves >= 0) && !Wings[i].total_arrived_count){
		return 1;
	}

	return 0;
}

// arithmetic functions
int add_sexps(int n)
{
	int	sum = 0, val;

	if (n != -1) {
		if ( CAR(n) != -1)
			sum = eval_sexp( CAR(n) );
		else
			sum = atoi( CTEXT(n) );

		// be sure to check for the NAN value when doing arithmetic -- this value should
		// get propagated to the next highest function.
		if ( Sexp_nodes[CAR(n)].value == SEXP_NAN )
			return SEXP_NAN;
		else if ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER )
			return SEXP_NAN_FOREVER;

		while (CDR(n) != -1) {
			val = eval_sexp( CDR(n) );
			// be sure to check for the NAN value when doing arithmetic -- this value should
			// get propagated to the next highest function.
			if ( Sexp_nodes[CDR(n)].value == SEXP_NAN )
				return SEXP_NAN;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER )
				return SEXP_NAN_FOREVER;
			sum += val;
			n = CDR(n);
		}
	}

	return sum;
}

int sub_sexps(int n)
{
	int	sum = 0;

	if (n != -1) { 
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(CAR(n));
		else
			sum = atoi(CTEXT(n));

		while (CDR(n) != -1) {
			sum -= eval_sexp(CDR(n));
			n = CDR(n);
		}
	}

	return sum;
}

int mul_sexps(int n)
{
	int	sum = 0;

	if (n != -1) {
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(Sexp_nodes[n].first);
		else
			sum = atoi(CTEXT(n));

		while (Sexp_nodes[n].rest != -1) {
			sum *= eval_sexp(Sexp_nodes[n].rest);
			n = Sexp_nodes[n].rest;
		}
	}

	return sum;
}

int div_sexps(int n)
{
	int	sum = 0;

	if (n != -1) {
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(Sexp_nodes[n].first);
		else
			sum = atoi(CTEXT(n));

		while (Sexp_nodes[n].rest != -1) {
			sum /= eval_sexp(Sexp_nodes[n].rest);
			n = Sexp_nodes[n].rest;
		}
	}

	return sum;
}

int mod_sexps(int n)
{
	int	sum = 0;

	if (n != -1) {
		if (Sexp_nodes[n].first != -1)
			sum = eval_sexp(Sexp_nodes[n].first);
		else
			sum = atoi(CTEXT(n));

		while (Sexp_nodes[n].rest != -1) {
			sum = sum % eval_sexp(Sexp_nodes[n].rest);
			n = Sexp_nodes[n].rest;
		}
	}

	return sum;
}

int rand_internal(int low, int high)
{
	int diff;

	// get diff - don't allow negative or zero
	diff = high - low;
	if (diff < 0) {
		diff = 0;
	}

	return (low + rand() % (diff + 1));
}


int rand_sexp(int n, int multiple = 1)	// was 0 - changed to 1 by Goober5000
{
	int low = 0;
	int high = 0;
	int rand_num = 0;

	if (n != -1) {
		if (Sexp_nodes[n].value == SEXP_NUM_EVAL) {
			// don't regenerate new random number
			rand_num = atoi(CTEXT(n));
		} else {
			// get low
//			if (Sexp_nodes[n].first != -1) {
//				low = eval_sexp(Sexp_nodes[n].first);
//			} else {
//				low = atoi(CTEXT(n));
//			}
			low = num_eval(n);

			// get high
			high = num_eval(CDR(n));

			// get the random number
			rand_num = rand_internal(low, high);

			if (!multiple) {
				// set .value and .text so random number is generated only once.
				Sexp_nodes[n].value = SEXP_NUM_EVAL;
				sprintf(Sexp_nodes[n].text, "%d", rand_num);
			}
		}
	}

	return rand_num;
}

// boolean evaluation functions.  Evaluate all sexpressions in the 'or' operator.  Needed to mark
// entries in the mission log as essential so when pruning the log, we know which entries we might
// need to keep.
int sexp_or(int n)
{
	int all_false, result;

	all_false = 1;
	result = 0;
	if (n != -1) {
		if (Sexp_nodes[n].first != -1) {
			result |= eval_sexp(Sexp_nodes[n].first);
			if ( Sexp_nodes[Sexp_nodes[n].first].value == SEXP_KNOWN_TRUE )
				return SEXP_KNOWN_TRUE;															// if one of the OR clauses is TRUE, whole clause is true
			if ( Sexp_nodes[Sexp_nodes[n].first].value != SEXP_KNOWN_FALSE )		// if the value is still unknown, they all can't be false
				all_false = 0;
		} else
			result |= atoi(CTEXT(n));

		// don't return on true value -- keep evaluating for mission log purposes
		//if ( result )
		//	return result;

		while (Sexp_nodes[n].rest != -1) {
			result |= eval_sexp(Sexp_nodes[n].rest);
			if ( Sexp_nodes[Sexp_nodes[n].rest].value == SEXP_KNOWN_TRUE )
				return SEXP_KNOWN_TRUE;															// if one of the OR clauses is TRUE, whole clause is true
			if ( Sexp_nodes[Sexp_nodes[n].rest].value != SEXP_KNOWN_FALSE )		// if the value is still unknown, they all can't be false
				all_false = 0;
			n = Sexp_nodes[n].rest;
			// don't return on true value -- keep evaluating for mission log purposes
			//if ( result )
			//	return result;
		}
	}

	if ( all_false )
		return SEXP_KNOWN_FALSE;

	return result;
}

// this function does the 'and' operator.  It will short circuit evaluation  *but* it will still
// evaluate other members of the and construct.  I do this because I need events in the mission log
// to get marked as essential for goal purposes, and evaluation is pretty much the only way
int sexp_and(int n)
{
	int all_true, result;

	result = -1;
	all_true = 1;
	if (n != -1) {
		if (Sexp_nodes[n].first != -1) {
			result &= eval_sexp( CAR(n) );
			if ( Sexp_nodes[Sexp_nodes[n].first].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;														// if one of the AND clauses is FALSE, whole clause is false
			if ( Sexp_nodes[Sexp_nodes[n].first].value != SEXP_KNOWN_TRUE )		// if the value is still unknown, they all can't be true
				all_true = 0;
		} else
			result &= atoi(CTEXT(n));

		// don't short circuit -- evaluate everything for purposes of marking mission log
		//if ( !result )
		//	return result;

		while (Sexp_nodes[n].rest != -1) {
			int new_result;

			new_result = eval_sexp( CDR(n) );
			result &= new_result;
			if ( Sexp_nodes[Sexp_nodes[n].rest].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;															// if one of the OR clauses is TRUE, whole clause is true
			if ( Sexp_nodes[Sexp_nodes[n].rest].value != SEXP_KNOWN_TRUE )				// if the value is still unknown, they all can't be false
				all_true = 0;
			// don't short circuit -- evaluate everything for purposes of marking mission log
			//if ( !result )
				//return result;

			n = Sexp_nodes[n].rest;
		}
	}

	if ( all_true )
		return SEXP_KNOWN_TRUE;

	return result;
}

// this version of the 'and' operator determines whether or not it's arguments become true
// in the order in which they are specified in the when statement.  Should be a simple matter of 
// seeing if anything evaluates to true later than something that evalueated to false
int sexp_and_in_sequence(int n)
{
	int result = -1;
	int all_true;

	all_true = 1;											// represents whether or not all nodes we have seen so far are true
	if (n != -1) {
		if (Sexp_nodes[n].first != -1) {
			result &= eval_sexp( CAR(n) );
			if ( Sexp_nodes[Sexp_nodes[n].first].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;														// if one of the AND clauses is FALSE, whole clause is false
			if ( Sexp_nodes[Sexp_nodes[n].first].value != SEXP_KNOWN_TRUE )		// if value is true, mark our all_true variable for later checking
				all_true = 0;
		} else
			result &= atoi(CTEXT(n));

		// a little test -- if the previous sexpressions was true, then mark the node itself as always
		// true.  I did this because of the distance function.  It might become true, then when waiting for
		// the second evalation, it might become false, rendering this function false.  So, when one becomes
		// true -- mark it true forever.
		if ( result )
			Sexp_nodes[Sexp_nodes[n].first].value = SEXP_KNOWN_TRUE;

		while (Sexp_nodes[n].rest != -1) {
			int next_result;

			next_result = eval_sexp( CDR(n) );
			if ( next_result && !result )				// if current result is true, and our running result is false, thngs didn't become true in order
				return SEXP_KNOWN_FALSE;
			result &= next_result;
			if ( Sexp_nodes[Sexp_nodes[n].rest].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_FALSE;															// if one of the OR clauses is TRUE, whole clause is true
			if ( Sexp_nodes[Sexp_nodes[n].rest].value != SEXP_KNOWN_TRUE )				// if the value is still unknown, they all can't be false
				all_true = 0;
			// see comment above for explanation of next lines
			if ( result )
				Sexp_nodes[Sexp_nodes[n].rest].value = SEXP_KNOWN_TRUE;
			n = Sexp_nodes[n].rest;
		}
	}

	if ( all_true )
		return SEXP_KNOWN_TRUE;

	return result;
}

// for these four basic boolean operations (not, <, >, and =), we have special cases that we must deal
// with.  We have sexpressions operators that might return a NAN type return value (such as the distance
// between two ships when one of the ships is destroyed or departed).  These operations need to check for
// this special NAN value and adjust their return types accordingly.  NAN values represent false return values
int sexp_not( int n )
{
	int result = 0;

	if (n != -1) {
		if (Sexp_nodes[n].first != -1) {
			result = eval_sexp( CAR(n) );
			if ( Sexp_nodes[CAR(n)].value == SEXP_KNOWN_FALSE )
				return SEXP_KNOWN_TRUE;												// not KNOWN_FALSE == KNOWN_TRUE;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_KNOWN_TRUE )		// not KNOWN_TRUE == KNOWN_FALSE
				return SEXP_KNOWN_FALSE;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_NAN )				// not NAN == TRUE (I think)
				return 1;
			else if ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER )
				return 1;
		} else
			result = atoi(CTEXT(n));
	}

	return !result;
}

int sexp_gt(int n)
{
	int exp1, exp2;

	exp1 = eval_sexp( n );
	exp2 = eval_sexp( CDR(n) );
	
	// check for the NAN value
	if ( ( Sexp_nodes[CAR(n)].value == SEXP_NAN ) || ( Sexp_nodes[CDR(n)].value == SEXP_NAN ) )
		return 0;
	else if ( ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER ) || ( Sexp_nodes[CDR(n)].value == SEXP_NAN_FOREVER ) )
		return SEXP_KNOWN_FALSE;

	if ( exp1 > exp2 )
		return 1;
	return 0;
}

int sexp_lt(int n)
{
	int exp1, exp2;

	exp1 = eval_sexp( n );
	exp2 = eval_sexp( CDR(n) );

		// check for the NAN value
	if ( ( Sexp_nodes[CAR(n)].value == SEXP_NAN ) || ( Sexp_nodes[CDR(n)].value == SEXP_NAN ) )
		return 0;
	else if ( ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER ) || ( Sexp_nodes[CDR(n)].value == SEXP_NAN_FOREVER ) )
		return SEXP_KNOWN_FALSE;

	if ( exp1 < exp2 )
		return 1;
	return 0;
}

int sexp_equal(int n)
{
	int exp1, exp2;

	exp1 = eval_sexp( n );
	exp2 = eval_sexp( CDR(n) );

		// check for the NAN value
	if ( ( Sexp_nodes[CAR(n)].value == SEXP_NAN ) || ( Sexp_nodes[CDR(n)].value == SEXP_NAN ) )
		return 0;
	else if ( ( Sexp_nodes[CAR(n)].value == SEXP_NAN_FOREVER ) || ( Sexp_nodes[CDR(n)].value == SEXP_NAN_FOREVER ) )
		return SEXP_KNOWN_FALSE;

	if ( exp1 == exp2 )
		return 1;
	return 0;
}

// Evaluate if given ship is destroyed.
//	Return true if the ship in the expression has been destroyed.
int sexp_is_destroyed(int n, fix *latest_time)
{
	char	*name;
	int	count, num_destroyed, wing_index;
	fix	time;

	Assert ( n != -1 );

	count = 0;
	num_destroyed = 0;
	wing_index = -1;
	while (n != -1) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// check to see if this ship/wing has departed.  If so, then function is known false
		if ( mission_log_get_time (LOG_SHIP_DEPART, name, NULL, NULL) || mission_log_get_time (LOG_WING_DEPART, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;

		// check the mission log.  If ship/wing not destroyed, immediately return 0.
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time) || mission_log_get_time(LOG_WING_DESTROYED, name, NULL, &time) || mission_log_get_time(LOG_SELF_DESTRUCT, name, NULL, &time)) {
			num_destroyed++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		} else {
			// ship or wing isn't destroyed -- add to directive count
			if ( (wing_index = wing_name_lookup( name, 1 )) >= 0 ) {
				Directive_count += Wings[wing_index].current_count;
			} else
				Directive_count++;
		}

		// move to next ship/wing in list
		n = CDR(n);
	}

	// special case to mark a directive for destroy wing objectives true after a short amount
	// of time when there are more waves for this wing.
	if ( (count == 1) && (wing_index >= 0) && (Directive_count == 0) ) {
		if ( Wings[wing_index].current_wave < Wings[wing_index].num_waves )
			Directive_count =	DIRECTIVE_WING_ZERO;
	}

	if ( count == num_destroyed )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}


// return true if the subsystem of the given ship has been destroyed or not
int sexp_is_subsystem_destroyed(int n)
{
	char *ship_name, *subsys_name;

	Assert( n != -1 );
	
	ship_name = CTEXT(n);
	subsys_name = CTEXT(CDR(n));

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// if the ship has departed, no way to destroy it's subsystem.
	if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL ))
		return SEXP_KNOWN_FALSE;
	
	if ( mission_log_get_time(LOG_SHIP_SUBSYS_DESTROYED, ship_name, subsys_name, NULL) )
		return SEXP_KNOWN_TRUE;

	return 0;

}

// determines if a ship has docked
int sexp_has_docked(int n)
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = atoi(CTEXT(CDR(CDR(n))));		// count of times that we should look for

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	Assert ( count > 0 );
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
		return SEXP_KNOWN_FALSE;

	if ( !mission_log_get_time_indexed(LOG_SHIP_DOCK, docker, dockee, count, NULL) )
		return 0;

	return SEXP_KNOWN_TRUE;
}

// determines if a ship has undocked
int sexp_has_undocked(int n)
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = atoi(CTEXT(CDR(CDR(n))));

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	Assert ( count > 0 );
	if ( !mission_log_get_time_indexed(LOG_SHIP_UNDOCK, docker, dockee, count, NULL) ) {
		// if either ship destroyed before they dock, then sexp is known false
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else
			return 0;
	}

	return SEXP_KNOWN_TRUE;
}

// determines if a ship has arrived onto the scene
int sexp_has_arrived(int n, fix *latest_time)
{
	char *name;
	int	count, num_arrived;
	fix	time;

	count = 0;
	num_arrived = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);
		// if there is no log entry for this ship/wing for arrival, sexpression is false
		if ( mission_log_get_time(LOG_SHIP_ARRIVE, name, NULL, &time) || mission_log_get_time(LOG_WING_ARRIVE, name, NULL, &time) ) {
			num_arrived++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_arrived )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

// determines if a ship/wing has departed
int sexp_has_departed(int n, fix *latest_time)
{
	char *name;
	int count, num_departed;
	fix time;

	count = 0;
	num_departed = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no departure log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) || mission_log_get_time(LOG_WING_DESTROYED, name, NULL, NULL))
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DEPART, name, NULL, &time) || mission_log_get_time(LOG_WING_DEPART, name, NULL, &time) ) {
			num_departed++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_departed )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

// determines if ships are disabled
int sexp_is_disabled( int n, fix *latest_time )
{
	char *name;
	int count, num_disabled;
	fix time;

	count = 0;
	num_disabled = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no disable log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DEPART, name, NULL, &time) || mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DISABLED, name, NULL, &time) ) {
			num_disabled++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_disabled )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

// function to determine if a ship is done flying waypoints
int sexp_are_waypoints_done( int n )
{
	char *ship_name, *waypoint_name;

	ship_name = CTEXT(n);
	waypoint_name = CTEXT(CDR(n));

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// a destroyed or departed ship will never reach their goal -- return known false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
		return SEXP_KNOWN_FALSE;
	else if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) )
		return SEXP_KNOWN_FALSE;

	// now check the log for the waypoints done entry
	if ( mission_log_get_time(LOG_WAYPOINTS_DONE, ship_name, waypoint_name, NULL) )
		return SEXP_KNOWN_TRUE;

	return 0;
}


// determines if ships are disarmed
int sexp_is_disarmed( int n, fix *latest_time )
{
	char *name;
	int count, num_disarmed;
	fix time;

	count = 0;
	num_disarmed = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no disarm log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DEPART, name, NULL, &time) || mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DISARMED, name, NULL, &time) ) {
			num_disarmed++;
			if ( latest_time && (time > *latest_time) )
				*latest_time = time;
		}
		n = CDR(n);
	}

	if ( count == num_disarmed )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

// the following functions are similar to the above objective functions but return true/false
// if N seconds have elasped after the corresponding function is true.
int sexp_is_destroyed_delay(int n)
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;

	delay = i2f(num_eval(n));

	// check value of is_destroyed function.  KNOWN_FALSE should be returned immediately
	val = sexp_is_destroyed( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {

		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return 0;
}

int sexp_is_subsystem_destroyed_delay( int n )
{
	char *ship_name, *subsys_name;
	fix delay, time;

	Assert( n != -1 );
	
	ship_name = CTEXT(n);
	subsys_name = CTEXT(CDR(n));
	delay = i2f(atoi(CTEXT(CDR(CDR(n)))));

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// if the ship has departed, no way to destroy it's subsystem.
	if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL ))
		return SEXP_KNOWN_FALSE;

	if ( mission_log_get_time(LOG_SHIP_SUBSYS_DESTROYED, ship_name, subsys_name, &time) ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return 0;
}

int sexp_is_disabled_delay( int n )
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(atoi(CTEXT(n)));

	// check value of is_disable for known false and return immediately if it is.
	val = sexp_is_disabled( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return 0;
}

int sexp_is_disarmed_delay( int n )
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(atoi(CTEXT(n)));
	
	// check value of is_disarmed for a known false value and return that immediately if it is
	val = sexp_is_disarmed( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return 0;
}

int sexp_has_docked_delay( int n )
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = atoi(CTEXT(CDR(CDR(n))));		// count of times that we should look for
	fix delay = i2f(atoi(CTEXT(CDR(CDR(CDR(n))))));
	fix time;

	Assert ( count > 0 );
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
		return SEXP_KNOWN_FALSE;

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	if ( !mission_log_get_time_indexed(LOG_SHIP_DOCK, docker, dockee, count, &time) )
		return 0;

	if ( (Missiontime - time) >= delay )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

int sexp_has_undocked_delay( int n )
{
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = atoi(CTEXT(CDR(CDR(n))));
	fix delay = i2f(atoi(CTEXT(CDR(CDR(CDR(n))))));
	fix time;

	if (sexp_query_has_yet_to_arrive(docker))
		return SEXP_CANT_EVAL;

	if (sexp_query_has_yet_to_arrive(dockee))
		return SEXP_CANT_EVAL;

	Assert ( count > 0 );
	if ( !mission_log_get_time_indexed(LOG_SHIP_UNDOCK, docker, dockee, count, &time) ) {
		// if either ship destroyed before they dock, then sexp is known false
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, docker, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, dockee, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else
			return 0;
	}

	if ( (Missiontime - time) >= delay )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

int sexp_has_arrived_delay( int n )
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(atoi(CTEXT(n)));

	// check return value from arrived function.  if can never arrive, then return that value here as well
	val = sexp_has_arrived( CDR(n), &time );
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return 0;
}

int sexp_has_departed_delay( int n )
{
	fix delay, time;
	int val;

	Assert ( n >= 0 );

	time = 0;
	delay = i2f(atoi(CTEXT(n)));

	// must first check to see if the departed function could ever be true/false or is true or false.
	// if it can never be true, return that value
	val = sexp_has_departed( CDR(n), &time);
	if ( val == SEXP_KNOWN_FALSE )
		return val;

	if ( val == SEXP_CANT_EVAL )
		return SEXP_CANT_EVAL;

	if ( val ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	}

	return 0;
}

// function to determine if a ship is done flying waypoints after N seconds
int sexp_are_waypoints_done_delay( int n )
{
	char *ship_name, *waypoint_name;
	fix time, delay;

	ship_name = CTEXT(n);
	waypoint_name = CTEXT(CDR(n));
	delay = i2f(atoi(CTEXT(CDR(CDR(n)))));

	if (sexp_query_has_yet_to_arrive(ship_name))
		return SEXP_CANT_EVAL;

	// a destroyed or departed ship will never reach their goal -- return known false
	// 
	// Not checking the entries below.  Ships which warp out after reaching their goal (or getting
	// destroyed after their goal), but after reaching their waypoints, may have this goal incorrectly
	// marked false!!!!

	// now check the log for the waypoints done entry
	if ( mission_log_get_time(LOG_WAYPOINTS_DONE, ship_name, waypoint_name, &time) ) {
		if ( (Missiontime - time) >= delay )
			return SEXP_KNOWN_TRUE;
	} else {
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
	}

	return 0;
}

// function to determine is all of a given ship type are destroyed
int sexp_ship_type_destroyed( int n )
{
	int percent, type;
	char *shiptype;

	percent = atoi(CTEXT(n));
	shiptype = CTEXT(CDR(n));

	for ( type = 0; type < MAX_SHIP_TYPE_COUNTS; type++ ) {
		if ( !stricmp( Ship_type_names[type], shiptype) )
			break;
	}

	// bogus if we reach the end of this array!!!!
	if ( type == MAX_SHIP_TYPE_COUNTS ) {
		Int3();
		return 0;
	}

	if ( Ship_counts[type].total == 0 )
		return 0;

	// determine if the percentage of killed/total is >= percentage given in the expression
	if ( (Ship_counts[type].killed * 100 / Ship_counts[type].total) >= percent)
		return SEXP_KNOWN_TRUE;
	
	return 0;
}


// following are time based functions
int sexp_has_time_elapsed(int n)
{
	int time = num_eval(n);

	if ( f2i(Missiontime) >= time )
		return SEXP_KNOWN_TRUE;

	return 0;
}

// next function returns the time into the mission
int sexp_mission_time()
{
	return f2i(Missiontime);
}

// returns percent of length of distance to special warpout plane
int sexp_special_warp_dist( int n)
{
	char *ship_name;
	int shipnum;

	// get shipname
	ship_name = CTEXT(n);

	// check to see if either ship was destroyed or departed.  If so, then make this node known
	// false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, ship_name, NULL, NULL) ) {
		return SEXP_NAN_FOREVER;
	}

	// get ship name
	shipnum = ship_name_lookup(ship_name);
	if (shipnum < 0) {
		return SEXP_NAN;
	}

	// check that ship has warpout_objnum
	if (Ships[shipnum].special_warp_objnum == -1) {
		return SEXP_NAN;
	}
	
	Assert( (Ships[shipnum].special_warp_objnum >= 0) && (Ships[shipnum].special_warp_objnum < MAX_OBJECTS));
	if ( (Ships[shipnum].special_warp_objnum < 0) && (Ships[shipnum].special_warp_objnum >= MAX_OBJECTS) ) {
		return SEXP_NAN;
	}

	// check the special warpout device is valid
	int valid = FALSE;
	object *ship_objp = &Objects[Ships[shipnum].objnum];
	object *warp_objp = &Objects[Ships[shipnum].special_warp_objnum];
	if (warp_objp->type == OBJ_SHIP) {
		if (Ship_info[Ships[warp_objp->instance].ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
			valid = TRUE;
		}
	}

	if (!valid) {
		return SEXP_NAN;
	}

	// check if within 45 degree half-angle cone of facing 
	float dot = fl_abs(vm_vec_dotprod(&warp_objp->orient.vec.fvec, &ship_objp->orient.vec.fvec));
	if (dot < 0.707f) {
		return SEXP_NAN;
	}

	// get distance
	vector hit_pt;
	float dist = fvi_ray_plane(&hit_pt, &warp_objp->pos, &warp_objp->orient.vec.fvec, &ship_objp->pos, &ship_objp->orient.vec.fvec, 0.0f);
	polymodel *pm = model_get(Ships[shipnum].modelnum);
	dist += pm->mins.xyz.z;

	// return as a percent of length
	return (int) (100.0f * dist / ship_get_length(&Ships[shipnum]));
}


int sexp_time_destroyed(int n)
{
	fix time;

	if ( !mission_log_get_time( LOG_SHIP_DESTROYED, CTEXT(n), NULL, &time) ){		// returns 0 when not found
		return SEXP_NAN;
	}
	
	return f2i(time);
}

int sexp_time_wing_destroyed(int n)
{
	fix time;

	if ( !mission_log_get_time( LOG_WING_DESTROYED, CTEXT(n), NULL, &time) ){
		return SEXP_NAN;
	}
	
	return f2i(time);
}

int sexp_time_docked(int n)
{
	fix time;
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = atoi(CTEXT(CDR(CDR(n))));

	Assert ( count > 0 );
	if ( !mission_log_get_time_indexed(LOG_SHIP_DOCK, docker, dockee, count, &time) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_undocked(int n)
{
	fix time;
	char *docker = CTEXT(n);
	char *dockee = CTEXT(CDR(n));
	int count = atoi(CTEXT(CDR(CDR(n))));

	Assert ( count > 0 );
	if ( !mission_log_get_time_indexed(LOG_SHIP_UNDOCK, docker, dockee, count, &time) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_ship_arrived(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time( LOG_SHIP_ARRIVE, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_wing_arrived(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time( LOG_WING_ARRIVE, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_ship_departed(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time( LOG_SHIP_DEPART, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

int sexp_time_wing_departed(int n)
{
	fix time;

	Assert( n != -1 );
	if ( !mission_log_get_time( LOG_WING_DEPART, CTEXT(n), NULL, &time ) ){
		return SEXP_NAN;
	}

	return f2i(time);
}

// function to return the remaining shields as a percentage of the given ship.
int sexp_shields_left(int n)
{
	int shipnum, percent;
	char *shipname;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	// now return the amount of shields left as a percentage of the whole.
	percent = (int)(get_shield_strength(&Objects[Ships[shipnum].objnum]) / Ship_info[Ships[shipnum].ship_info_index].shields * 100.0f);
	return percent;
}

// function to return the remaining hits left as a percentage of the whole.  This hit amount counts for
// all hits on the ship (hull + subsystems).  Use hits_left_hull to find hull hits remaining.
int sexp_hits_left(int n)
{
	int shipnum, percent;
	char *shipname;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	// now return the amount of hits left as a percentage of the whole.  Subtract the percentage from 100
	// since we are working with total hit points taken, not total remaining.
	ship		*shipp = &Ships[shipnum];
	ship_info *sip  = &Ship_info[shipp->ship_info_index];
	object	*objp = &Objects[shipp->objnum];
	percent = (int) (100.0f * objp->hull_strength / sip->initial_hull_strength);
	return percent;
}

// is ship visible on radar
// returns 0 - not visible
// returns 1 - marginally targetable (jiggly on radar)
// returns 2 - fully targetable
int sexp_is_ship_visible(int n)
{
	char *shipname;
	int shipnum;
	int ship_is_visible = 0;

	// if multiplayer, bail
	if (Game_mode & GM_MULTIPLAYER) {
		return SEXP_NAN_FOREVER;
	}

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	// get ship's *radar* visiblity
	if (Player_ship != NULL)
	{
		if (ship_is_visible_by_team_new(&Objects[Ships[shipnum].objnum], Player_ship))
		{
			ship_is_visible = 2;
		}
	}

	// only check awacs level if ship is not visible by team
	if (Player_ship != NULL && !ship_is_visible) {
		float awacs_level = awacs_get_level(&Objects[Ships[shipnum].objnum], Player_ship);
		if (awacs_level >= 1.0f) {
			ship_is_visible = 2;
		} else if (awacs_level > 0) {
			ship_is_visible = 1;
		}
	}

	return ship_is_visible;
}

// is the stealth flag set on this ship
int sexp_is_ship_stealthy(int n)
{
	char *shipname;
	int shipnum;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ) {					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	if (Ships[shipnum].flags2 & SF2_STEALTH)
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// is the friendly stealth ship visible
int sexp_is_friendly_stealth_visible(int n)
{
	char *shipname;
	int shipnum;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ) {					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	if (Ships[shipnum].flags2 & SF2_FRIENDLY_STEALTH_INVIS)
		return SEXP_FALSE;
	else
		return SEXP_TRUE;
}

// get multi team v team score
// if not multi team v team return 0
// if invalid team return 0
int sexp_team_score(int node)
{
#ifndef NO_NETWORK
	// if multi t vs t
	if (Game_mode & GM_MULTIPLAYER) {
		if (Netgame.type_flags & NG_TYPE_TEAM) {

			int team = atoi(CTEXT(node));

			if (team == 1) {
				return Multi_team0_score;
			} else if (team == 2) {
				return Multi_team1_score;
			} else {
				// invalid team index
				Int3();
				return 0;
			}
		}
	}
#endif

	return 0;
}


// function to return the remaining hits left on a subsystem as a percentage of thw whole.
int sexp_hits_left_subsystem(int n)
{
	int shipnum, percent, type;
	char *shipname;
	char *subsys_name;

	shipname = CTEXT(n);
	
	// if ship is gone or departed, cannot ever evaluate properly.  Return NAN_FOREVER
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, shipname, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, shipname, NULL, NULL) ){
		return SEXP_NAN_FOREVER;
	}

	shipnum = ship_name_lookup( shipname );
	if ( shipnum == -1 ){					// hmm.. if true, must not have arrived yet
		return SEXP_NAN;
	}

	subsys_name = CTEXT(CDR(n));
	type = ai_get_subsystem_type( subsys_name );
	if ( (type >= 0) && (type < SUBSYSTEM_MAX) ) {
		// return as a percentage the hits remaining on the subsystem as a whole (i.e. for 3 engines,
		// we are returning the sum of the hits on the 3 engines)
		if (type == SUBSYSTEM_UNKNOWN) {
			// find the ship subsystem by searching ship's subsys_list
			ship_subsys *ss;
			ss = GET_FIRST( &Ships[shipnum].subsys_list );
			while ( ss != END_OF_LIST( &Ships[shipnum].subsys_list ) ) {

				if ( !stricmp(ss->system_info->subobj_name, subsys_name)) {
					percent = (int) (ss->current_hits / ss->system_info->max_hits * 100.0f);
					return percent;
				}

				ss = GET_NEXT( ss );
			}
			// we reached end of ship subsys list without finding subsys_name
			Int3();

		} else {
			percent = (int)(ship_get_subsystem_strength(&Ships[shipnum],type) * 100.0f);
			return percent;
		}
	}
	return SEXP_NAN;			// if for some strange reason, the type field of the subsystem is bogus
}

int sexp_determine_team(char *subj)
{
	int team = 0;

	if (!stricmp(subj, "<any friendly>")){
		team = TEAM_FRIENDLY;
	} else if (!stricmp(subj, "<any hostile>")){
		team = TEAM_HOSTILE;
	} else if (!stricmp(subj, "<any neutral>")){
		team = TEAM_NEUTRAL;
	} else if (!stricmp(subj, "<any unknown>")){
		team = TEAM_UNKNOWN;
	} else if (!stricmp(subj, "<any traitor>")){
		team = TEAM_TRAITOR;
	}

	return team;
}

// returns the distance between two objects.  If a wing is specified as one (or both) of the arguments
// to this function, we are looking for the closest distance
int sexp_distance(int n)
{
	int i, team, obj, dist, dist_min = 0, inited = 0;
	char *sname1, *sname2;
	wing *wingp;
	ship_obj *so;

	sname1 = CTEXT(n);
	sname2 = CTEXT(CDR(n));

	// check to see if either ship was destroyed or departed.  If so, then make this node known
	// false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, sname1, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, sname1, NULL, NULL) ||
		  mission_log_get_time(LOG_SHIP_DESTROYED, sname2, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, sname2, NULL, NULL) ) 
		return SEXP_NAN_FOREVER;

	// one of the names might be the name of a wing.  Check to see if the wing is detroyed or departed
	if ( mission_log_get_time(LOG_WING_DESTROYED, sname1, NULL, NULL) || mission_log_get_time( LOG_WING_DEPART, sname1, NULL, NULL) ||
		  mission_log_get_time(LOG_WING_DESTROYED, sname2, NULL, NULL) || mission_log_get_time( LOG_WING_DEPART, sname2, NULL, NULL) ) 
		return SEXP_NAN_FOREVER;

	team = sexp_determine_team(sname1);
	if (team) {  // we have a team type, so check all ships of that type
		so = GET_FIRST(&Ship_obj_list);
		while (so != END_OF_LIST(&Ship_obj_list)) {
			if (Ships[Objects[so->objnum].instance].team == team) {
				obj = so->objnum;
				dist = sexp_distance2(obj, sname2);
				if (dist != SEXP_NAN) {
					if (!inited || (dist < dist_min)) {
						dist_min = dist;
						inited = 1;
					}
				}
			}

			so = GET_NEXT(so);
		}

		if (!inited)  // no objects were checked
			return SEXP_NAN;

		return dist_min;
	}

	// at this point, we must have a wing, ship or point for a subj
	obj = ship_name_lookup(sname1);
	if (obj >= 0)
		return sexp_distance2(Ships[obj].objnum, sname2);

	// at this point, we must have a wing or point for a subj
	obj = waypoint_lookup(sname1);
	if (obj >= 0)
		return sexp_distance2(obj, sname2);
		
	// at this point, we must have a wing for a subj
	obj = wing_name_lookup(sname1);
	if (obj < 0)
		return SEXP_NAN;  // we apparently don't have anything legal

	wingp = &Wings[obj];
	for (i=0; i<wingp->current_count; i++) {
		obj = Ships[wingp->ship_index[i]].objnum;
		dist = sexp_distance2(obj, sname2);
		if (dist != SEXP_NAN) {
			if (!inited || (dist < dist_min)) {
				dist_min = dist;
				inited = 1;
			}
		}
	}

	if (!inited)  // no objects were checked
		return SEXP_NAN;

	return dist_min;
}

// check distance between a given ship and a given subject (ship, wing, any <team>).
int sexp_distance2(int obj1, char *subj)
{
	int i, team, obj2, dist, dist_min = 0, inited = 0;
	wing *wingp;
	ship_obj	*so;
	
	team = sexp_determine_team(subj);
	if (team) {  // we have a team type, so check all ships of that type
		so = GET_FIRST(&Ship_obj_list);
		while (so != END_OF_LIST(&Ship_obj_list)) {
			if (Ships[Objects[so->objnum].instance].team == team) {
				obj2 = so->objnum;
				dist = sexp_distance3(obj1, obj2);
				if (dist != SEXP_NAN) {
					if (!inited || (dist < dist_min)) {
						dist_min = dist;
						inited = 1;
					}
				}
			}

			so = GET_NEXT(so);
		}

		if (!inited)  // no objects were checked
			return SEXP_NAN;

		return dist_min;
	}

	// at this point, we must have a wing, ship or point for a subj
	obj2 = ship_name_lookup(subj);
	if (obj2 >= 0)
		return sexp_distance3(obj1, Ships[obj2].objnum);

	// at this point, we must have a wing or point for a subj
	obj2 = waypoint_lookup(subj);
	if (obj2 >= 0)
		return sexp_distance3(obj1, obj2);
		
	// at this point, we must have a wing for a subj
	obj2 = wing_name_lookup(subj);
	if (obj2 < 0)
		return SEXP_NAN;  // we apparently don't have anything legal

	wingp = &Wings[obj2];
	for (i=0; i<wingp->current_count; i++) {
		obj2 = Ships[wingp->ship_index[i]].objnum;
		dist = sexp_distance3(obj1, obj2);
		if (dist != SEXP_NAN) {
			if (!inited || (dist < dist_min)) {
				dist_min = dist;
				inited = 1;
			}
		}
	}

	if (!inited)  // no objects were checked
		return SEXP_NAN;

	return dist_min;
}

// check distance between two given objects
int sexp_distance3(int obj1, int obj2)
{
	if ( (obj1 == -1) || (obj2 == -1) )				// if either object isn't present in the mission now
		return SEXP_NAN;									// return a really small number

	if ( (Objects[obj1].type == OBJ_SHIP) && (Objects[obj2].type == OBJ_SHIP) ) {
		if (OBJ_INDEX(Player_obj) == obj1)
			return (int) hud_find_target_distance( &Objects[obj2], &Objects[obj1] );
		else
			return (int) hud_find_target_distance( &Objects[obj1], &Objects[obj2] );

	} else {
		return (int) vm_vec_dist_quick( &Objects[obj1].pos, &Objects[obj2].pos );
	}
}

// locate the subsystem on a ship - Goober5000
void sexp_get_subsystem_pos(int shipnum, char *subsys_name, vector *subsys_world_pos)
{
	ship_subsys *ss;

	Assert(shipnum >= 0);

	// find the ship subsystem by searching ship's subsys_list
	ss = GET_FIRST( &Ships[shipnum].subsys_list );
	while ( ss != END_OF_LIST( &Ships[shipnum].subsys_list ) )
	{
		// if we found the subsystem
		if ( !stricmp(ss->system_info->subobj_name, subsys_name))
		{
			// find world position of subsystem on this object (the ship)
			get_subsystem_world_pos(&Objects[Ships[shipnum].objnum], ss, subsys_world_pos);
			return;
		}

		ss = GET_NEXT( ss );
	}
	// we reached end of ship subsys list without finding subsys_name
	Int3();
}

// returns the distance between an object and a ship subsystem.  If a wing is specified as the object argument
// to this function, we are looking for the closest distance
int sexp_distance_subsystem(int n)	// Goober5000
{
	int i, team, dist, obj, dist_min = 0, inited = 0;
	char *obj_name, *ship_with_subsys, *subsys_name;
	vector subsys_pos;
	wing *wingp;
	ship_obj *so;

	obj_name = CTEXT(n);
	ship_with_subsys = CTEXT(CDR(n));
	subsys_name = CTEXT(CDR(CDR(n)));

	// check to see if either ship was destroyed or departed.  If so, then make this node known
	// false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, obj_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, obj_name, NULL, NULL) ||
		  mission_log_get_time(LOG_SHIP_DESTROYED, ship_with_subsys, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, ship_with_subsys, NULL, NULL) ) 
		return SEXP_NAN_FOREVER;

	// the object might a wing.  Check to see if the wing is detroyed or departed
	if ( mission_log_get_time(LOG_WING_DESTROYED, obj_name, NULL, NULL) || mission_log_get_time( LOG_WING_DEPART, obj_name, NULL, NULL) ) 
		return SEXP_NAN_FOREVER;

	// find the ship with the subsystem
	obj = ship_name_lookup( ship_with_subsys );
	if ( obj < 0 )		// hmm... must not have arrived yet
	{
		return SEXP_NAN;
	}

	// get the subsystem's coordinates
	sexp_get_subsystem_pos(obj, subsys_name, &subsys_pos);

	// check if the first object is on a team
	team = sexp_determine_team(obj_name);
	if (team)	// we have a team type, so check all ships of that type
	{
		so = GET_FIRST(&Ship_obj_list);
		while (so != END_OF_LIST(&Ship_obj_list))
		{
			// do we check this ship?
			if (Ships[Objects[so->objnum].instance].team == team)
			{
				// compute distance
				dist = (int) vm_vec_dist_quick( &Objects[so->objnum].pos, &subsys_pos );
				
				// check for minimum
				if (!inited || (dist < dist_min))
				{
					dist_min = dist;
					inited = 1;
				}
			}

			so = GET_NEXT(so);
		}

		if (!inited)  // no objects were checked
			return SEXP_NAN;

		return dist_min;
	}

	// at this point, we must have a wing, ship or point for a subj
	obj = ship_name_lookup(obj_name);
	if ( !(obj < 0) )	// do we have a ship?
	{
		return (int) vm_vec_dist_quick( &Objects[obj].pos, &subsys_pos );
	}

	// at this point, we must have a wing or point for a subj
	obj = waypoint_lookup(obj_name);
	if ( !(obj < 0) )	// do we have a point?
	{
		return (int) vm_vec_dist_quick( &Objects[obj].pos, &subsys_pos );
	}
		
	// at this point, we must have a wing for a subj
	obj = wing_name_lookup(obj_name);
	if (obj < 0)
	{
		return SEXP_NAN;  // we apparently don't have anything legal
	}

	// check all ships in the wing
	wingp = &Wings[obj];
	for (i=0; i<wingp->current_count; i++)
	{
		// compute distance
		dist = (int) vm_vec_dist_quick( &Objects[Ships[wingp->ship_index[i]].objnum].pos, &subsys_pos );
				
		// check for minimum
		if (!inited || (dist < dist_min))
		{
			dist_min = dist;
			inited = 1;
		}
	}

	if (!inited)  // no objects were checked
	{
		return SEXP_NAN;
	}

	return dist_min;
}

// Goober5000
int sexp_vec_coordinate(vector *v, int index)
{
	// return result:
	switch(index)
	{
		case 0:
			return (int)v->xyz.x;
		case 1:
			return (int)v->xyz.y;
		case 2:
			return (int)v->xyz.z;
		default:
			Int3();
			return 0;
		}
}

// Goober5000
int sexp_get_object_coordinates(int n, int index) 
{
	ship_obj *so;
	char *object_name = CTEXT(n);
	n = CDR(n);
	int team, obj;

	// check to see if ship destroyed or departed.  In either case, do nothing.
	if ( mission_log_get_time(LOG_SHIP_DEPART, object_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, object_name, NULL, NULL) )
		return SEXP_NAN;

	// the object might be the name of a wing.  Check to see if the wing is detroyed or departed
	if ( mission_log_get_time(LOG_WING_DESTROYED, object_name, NULL, NULL) || mission_log_get_time( LOG_WING_DEPART, object_name, NULL, NULL) ) 
		return SEXP_NAN;

	team = sexp_determine_team(object_name);
	if (team)	// we have a team type, so pick the first ship of that type
	{
		so = GET_FIRST(&Ship_obj_list);
		while (so != END_OF_LIST(&Ship_obj_list))
		{
			if (Ships[Objects[so->objnum].instance].team == team)
			{
				return sexp_vec_coordinate(&Objects[so->objnum].pos, index);
			}

			so = GET_NEXT(so);
		}

		return SEXP_NAN;	// if no match
	}

	// at this point, we must have a wing, ship or point for a target
	obj = ship_name_lookup(object_name);
	if (obj >= 0)
	{
		// see if we have a subsys
		if (n != -1)
		{
			vector subsys_pos = ZERO_VECTOR;	// to avoid warning :-/
			sexp_get_subsystem_pos(obj, CTEXT(n), &subsys_pos);
			return sexp_vec_coordinate(&subsys_pos, index);
		}

		return sexp_vec_coordinate(&Objects[Ships[obj].objnum].pos, index);
	}

	// at this point, we must have a wing or point for a target
	obj = waypoint_lookup(object_name);
	if (obj >= 0)
	{
		return sexp_vec_coordinate(&Objects[obj].pos, index);
	}
		
	// at this point, we must have a wing for a target
	obj = wing_name_lookup(object_name);
	if (obj < 0)
		return SEXP_NAN;  // we apparently don't have anything legal

	// make sure at least one ship exists
	if (!Wings[obj].current_count)
		return SEXP_NAN;

	// point to wing leader
	return sexp_vec_coordinate(&Objects[Ships[Wings[obj].ship_index[0]].objnum].pos, index);
}

// Goober5000
void sexp_set_ship_position(int n)
{
	char *ship_name;
	int ship_num, x, y, z;
	
	ship_name = CTEXT(n);
	n = CDR(n);

	x = atoi(CTEXT(n));
	n = CDR(n);
	y = atoi(CTEXT(n));
	n = CDR(n);
	z = atoi(CTEXT(n));

	// check to see if the ship was destroyed or departed.  If so, then make this node known false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, ship_name, NULL, NULL) ) 
		return;

	// get ship number
	ship_num = ship_name_lookup(ship_name);

	// if the ship isn't in the mission, do nothing
	if (ship_num < 0)
		return;

	// set position
	Objects[Ships[ship_num].objnum].pos.xyz.x = (float)x;
	Objects[Ships[ship_num].objnum].pos.xyz.y = (float)y;
	Objects[Ships[ship_num].objnum].pos.xyz.z = (float)z;
}

// Goober5000
void sexp_set_ship_orient(int ship_num, vector *location)
{
	vector v_orient;
	matrix m_orient;
	object *obj;

	obj = &Objects[Ships[ship_num].objnum];

	// calculate orientation matrix ----------------
	memset(&v_orient, 0, sizeof(vector));

	vm_vec_sub(&v_orient, location, &obj->pos);

	if (IS_VEC_NULL(&v_orient))
	{
		Warning(LOCATION, "error in warp-effect: warp can't point to itself; quitting the warp...\n");
		return;
	}

	vm_vector_2_matrix(&m_orient, &v_orient, NULL, NULL);


	// set orientation -----------------------------
	obj->orient = m_orient;
}

// Goober5000
void sexp_set_ship_facing(int n)
{
	vector location;
	char *ship_name;
	int ship_num;
	
	ship_name = CTEXT(n);
	n = CDR(n);

	location.xyz.x = (float)atoi(CTEXT(n));
	n = CDR(n);
	location.xyz.y = (float)atoi(CTEXT(n));
	n = CDR(n);
	location.xyz.z = (float)atoi(CTEXT(n));

	// check to see if the ship was destroyed or departed.  If so, then make this node known false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, ship_name, NULL, NULL) ) 
		return;

	// get ship number
	ship_num = ship_name_lookup(ship_name);

	// if the ship isn't in the mission, do nothing
	if (ship_num < 0)
		return;

	sexp_set_ship_orient(ship_num, &location);
}

// Goober5000
void sexp_set_ship_facing_object(int n)
{
	ship_obj *so;
	char *ship_name;
	char *target_name;
	int ship_num, team, obj;

	ship_name = CTEXT(n);
	target_name = CTEXT(CDR(n));

	// check to see if either ship was destroyed or departed.  If so, then make this node known false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, ship_name, NULL, NULL) ||
		  mission_log_get_time(LOG_SHIP_DESTROYED, target_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, target_name, NULL, NULL) ) 
		return;

	// get ship number
	ship_num = ship_name_lookup(ship_name);

	// if the ship isn't in the mission, do nothing
	if (ship_num < 0)
		return;

	// the second name might be the name of a wing.  Check to see if the wing is detroyed or departed
	if ( mission_log_get_time(LOG_WING_DESTROYED, target_name, NULL, NULL) || mission_log_get_time( LOG_WING_DEPART, target_name, NULL, NULL) ) 
		return;

	team = sexp_determine_team(target_name);
	if (team)	// we have a team type, so pick the first ship of that type
	{
		so = GET_FIRST(&Ship_obj_list);
		while (so != END_OF_LIST(&Ship_obj_list))
		{
			if (Ships[Objects[so->objnum].instance].team == team)
			{
				sexp_set_ship_orient(ship_num, &Objects[so->objnum].pos);
				return;
			}

			so = GET_NEXT(so);
		}

		return;	// if no match
	}

	// at this point, we must have a wing, ship or point for a target
	obj = ship_name_lookup(target_name);
	if (obj >= 0)
	{
		sexp_set_ship_orient(ship_num, &Objects[Ships[obj].objnum].pos);
		return;
	}

	// at this point, we must have a wing or point for a target
	obj = waypoint_lookup(target_name);
	if (obj >= 0)
	{
		sexp_set_ship_orient(ship_num, &Objects[obj].pos);
		return;
	}
		
	// at this point, we must have a wing for a target
	obj = wing_name_lookup(target_name);
	if (obj < 0)
		return;  // we apparently don't have anything legal

	// make sure at least one ship exists
	if (!Wings[obj].current_count)
		return;

	// point to wing leader
	sexp_set_ship_orient(ship_num, &Objects[Ships[Wings[obj].ship_index[0]].objnum].pos);
}

// funciton to determine when the last meaningful order was given to one or more ships.  Returns
// true or false depending on whether or not a meaningful order was received
int sexp_last_order_time( int n )
{
	int instance, i;
	fix time;
	char *name;
	ai_goals *aigp;

	time = i2f(atoi(CTEXT(n)));
	Assert ( time >= 0 );

	n = CDR(n);
	while ( n != -1 ) {
		name = CTEXT(n);
		instance = ship_name_lookup(name);
		if ( instance != -1 ) {
			aigp = Ai_info[Ships[instance].ai_index].goals;
		} else {
			instance = wing_name_lookup(name);
			if ( instance == -1 )						// if we cannot find ship or wing, return 0
				return 0;
			aigp = Wings[instance].ai_goals;
		}

		// with the ship, check the ai_goals structure for this ship and determine if there are any
		// orders which are < time seconds since current mission time
		for ( i = 0; i < MAX_AI_GOALS; i++ ) {
			int mode;

			mode = aigp->ai_mode;
			if ( (mode  != AI_GOAL_NONE) && (mode != AI_GOAL_WARP) )
				if ( (aigp->time + time) > Missiontime )
					break;
			aigp++;
		}
		if ( i == MAX_AI_GOALS )
			return 1;

		n = CDR(n);
	}

	return 0;
}

// sexpression to return the number of players in the mission
int sexp_num_players()
{
	int count;
	object *objp;

	count = 0;
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type == OBJ_SHIP) && (objp->flags & OF_PLAYER_SHIP) )
			count++;
	}

	return count;
}

// expression to determine if the current skill level of the game is at least
// the skill level given in the sexpression
int sexp_skill_level_at_least( int n )
{
	int i;
	char *level_name;

	level_name = CTEXT(n);
	for (i = 0; i < NUM_SKILL_LEVELS; i++ ) {
		if ( !stricmp(level_name, Skill_level_names(i, 0)) ) {
			if ( Game_skill_level >= i ){
				return 1;
			} else {
				return 0;
			}
		}
	}

	// return 0 if not found!!!
	return 0;
}

int sexp_was_promotion_granted(int n)
{
	if (Player->flags & PLAYER_FLAGS_PROMOTED)
		return 1;

	return 0;
}

int sexp_was_medal_granted(int n)
{
	int i;
	char *medal_name;

	if (n < 0) {
		if (Player->stats.m_medal_earned >= 0)
			return 1;

		return 0;
	}

	medal_name = CTEXT(n);
	for (i=0; i<NUM_MEDALS; i++) {
		if (!stricmp(medal_name, Medals[i].name))
			break;
	}

	if ( (i < NUM_MEDALS) && (Player->stats.m_medal_earned == i) )
		return 1;

	return 0;
}

// function which returns true if the percentage of ships (and ships in wings) departed is at
// least the percentage given.  what determine if we should check destroyed or departed status
int sexp_percent_ships_depart_destroy(int n, int what)
{
	int percent;
	int total, count;
	char *name;

	percent = atoi(CTEXT(n));

	total = 0;
	count = 0;
	// iterate through the rest of the ships/wings in the list and tally the departures and the
	// total
	for ( n = CDR(n); n != -1; n = CDR(n) ) {
		int wingnum;

		name = CTEXT(n);

		wingnum = wing_name_lookup( name, 1 );
		if ( wingnum != -1 ) {
			// for wings, we can increment the total by the total number of ships that we expect for
			// this wing, and the departures by the number of departures stored for this wing
			total += (Wings[wingnum].wave_count * Wings[wingnum].num_waves);
			if ( what == OP_PERCENT_SHIPS_DEPARTED )
				count += Wings[wingnum].total_departed;
			else if ( what == OP_PERCENT_SHIPS_DESTROYED )
				count += Wings[wingnum].total_destroyed;
			else
				Int3();			// this would be very bogus!
		} else {
			// must be a ship, so increment the total by 1, then determine if this ship has departed
			total++;
			if ( what == OP_PERCENT_SHIPS_DEPARTED ) {
				if ( mission_log_get_time(LOG_SHIP_DEPART, name, NULL, NULL) )
					count++;
			} else if ( what == OP_PERCENT_SHIPS_DESTROYED ) {
				if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) )
					count++;
			} else
				Int3();			// this would be very bogus as well.

		}
	}

	// now, look at the percentage
	if ( ((count * 100) / total) >= percent )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

// function to tell is a list of ships has departed from within a radius of a given jump node.
// returns true N seconds after the list of ships have departed
int sexp_depart_node_delay( int n )
{
	int delay, count, num_departed;
	char *jump_node_name, *name;
	fix latest_time, this_time;

	delay = atoi( CTEXT(n) );
	n = CDR(n);
	jump_node_name = CTEXT(n);

	// iterate through the list of ships
	n = CDR(n);
	latest_time = 0;
	count = 0;
	num_departed = 0;
	while ( n != -1 ) {
		count++;
		name = CTEXT(n);

		if (sexp_query_has_yet_to_arrive(name))
			return SEXP_CANT_EVAL;

		// if ship/wing destroyed, sexpression is known false.  Also, if there is no departure log entry, then
		// the sexpression is not true.
		if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_SHIP_DEPART, name, jump_node_name, &this_time) ) {
			num_departed++;
			if ( this_time > latest_time )
				latest_time = this_time;
		}
		n = CDR(n);
	}

	if ( (count == num_departed) && ((Missiontime - latest_time) >= delay) )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

// sexpression which returns true when the listed ships/wings have all been destroyed or
// have departed.
int sexp_destroyed_departed_delay( int n )
{
	int count, total;
	fix delay, latest_time;
	char *name;

	// get the delay
	delay = i2f(atoi(CTEXT(n)));
	n = CDR(n);

	count = 0;					// number destroyed or departed
	total = 0;					// total number of ships/wings to check
	latest_time = 0;
	while ( n != -1 ) {
		int wingnum;
		fix time_gone = 0;

		total++;
		name = CTEXT(n);

		// for wings, check the WF_GONE flag to see if there are no more ships in this wing to arrive.
		wingnum = wing_name_lookup(name, 1);
		if ( wingnum != -1 ) {
			if ( Wings[wingnum].flags & WF_WING_GONE ) {
				// be sure to get the latest time of one of these 
				if ( Wings[wingnum].time_gone > latest_time ){
					time_gone = Wings[wingnum].time_gone;
				}
				count++;
			}
		} else if ( mission_log_get_time(LOG_SHIP_DEPART, name, NULL, &time_gone) ) {
			count++;
		} else if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, &time_gone) ) {
			count++;
		}

		// check our latest time
		if ( time_gone > latest_time ){
			latest_time = time_gone;
		}

		n = CDR(n);
	}

	if ( (count == total) && (Missiontime > (latest_time + delay)) )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

int sexp_special_warpout_name( int node )
{
	int shipnum, knossos_num;
	char *ship_name, *knossos;

	ship_name = CTEXT(node);
	knossos = CTEXT(CDR(node));

	// check to see if either ship was destroyed or departed.  If so, then make this node known
	// false
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, ship_name, NULL, NULL) ||
		  mission_log_get_time(LOG_SHIP_DESTROYED, knossos, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, knossos, NULL, NULL) ) 
		return SEXP_NAN_FOREVER;

	// get ship name
	shipnum = ship_name_lookup(ship_name);
	if (shipnum < 0) {
		return SEXP_NAN;
	}

	// get knossos ship
	knossos_num = ship_name_lookup(knossos);
	if (knossos_num < 0) {
		return SEXP_NAN;
	}

	// set special warpout objnum
	Ships[shipnum].special_warp_objnum = knossos_num;
	return 0;
}

// function which determines if N seconds have elapsed since all discovery of all cargo
// of given ships
// Goober5000 - I reworked this function to allow for the set-scanned and set-unscanned sexps
// to work multiple times in a row and also to fix the potential bug where exited ships are
// checked against their departure time, not against their cargo known time
int sexp_is_cargo_known( int n, int check_delay )
{
	int count, ship_num, num_known, delay;

	char *name;

	Assert ( n >= 0 );

	count = 0;
	num_known = 0;

	// get the delay value (if there is one)
	delay = 0;
	if ( check_delay )
	{
		delay = atoi(CTEXT(n) );
		n = CDR(n);
	}

	while ( n != -1 )
	{
		fix time_known;
		int is_known;

		is_known = 0;

		count++;

		// see if we have already checked this entry
		if ( Sexp_nodes[n].value == SEXP_KNOWN_TRUE )
		{
			num_known++;
		}
		else
		{
			name = CTEXT(n);

			// find the index in the ship array
			ship_num = ship_name_lookup_absolute(name);
			Assert(ship_num != -1);	// make sure that the ship actually is valid

			// see if the ship has already exited the mission (either through departure or destruction)
			if (ship_find_exited_ship_by_name(name) != -1)
			{
				// if not known, the whole thing is known false
				if ( !(Ships[ship_num].flags & SF_CARGO_REVEALED) )
					return SEXP_KNOWN_FALSE;

				// check the delay of when we found out
				time_known = Missiontime - Ships[ship_num].time_cargo_revealed;
				if ( f2i(time_known) >= delay )
				{
					is_known = 1;

					// here is the only place in the new sexp that this can be known true
					Sexp_nodes[n].value = SEXP_KNOWN_TRUE;
				}
			}
			// ship either in mission or not arrived yet
			else
			{
				// if ship_name_lookup returns -1, then ship is either exited or yet to arrive,
				// and we've already checked exited
				if ( ship_name_lookup(name) != -1 )
				{
					if ( Ships[ship_num].flags & SF_CARGO_REVEALED )
					{
						time_known = Missiontime - Ships[ship_num].time_cargo_revealed;
						if ( f2i(time_known) >= delay )
						{
							is_known = 1;
						}
					}
				}
			}
		}

		// if cargo is known, mark our variable, but not the sexp, because it may change later
		if ( is_known )
		{
			num_known++;
		}

		n = CDR(n);
	}

	Directive_count += count - num_known;
	if ( count == num_known )
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

void get_cap_subsys_cargo_flags(int shipnum, char *subsys_name, int *known, fix *time_revealed)
{
	int subsys_set = 0;
	ship_subsys *ss;

	// find the ship subsystem by searching ship's subsys_list
	ss = GET_FIRST( &Ships[shipnum].subsys_list );
	while ( ss != END_OF_LIST( &Ships[shipnum].subsys_list ) )
	{
		// if we found the subsystem
		if ( !stricmp(ss->system_info->subobj_name, subsys_name))
		{
			// set the flags
			*known = ss->subsys_cargo_revealed;
			*time_revealed = ss->time_subsys_cargo_revealed;

			subsys_set = 1;
		}

		ss = GET_NEXT( ss );
	}

	// if we didn't find the subsystem, the ship hasn't arrived yet
	if (!subsys_set)
	{
		*known = -1;
		*time_revealed = 0;
	}
}

// reworked by Goober5000 to allow for set-scanned and set-unscanned to be used more than once
int sexp_cap_subsys_cargo_known_delay(int n)
{
	int delay, count, num_known, ship_num, cargo_revealed;
	fix time_revealed;
	char *ship_name, *subsys_name;

	num_known = 0;
	count = 0;
	cargo_revealed = 0;
	time_revealed = 0;

	// get delay
	delay = atoi(CTEXT(n));
	n = CDR(n);

	// get ship name
	ship_name = CTEXT(n);
	n = CDR(n);

	// find the index in the ship array
	ship_num = ship_name_lookup_absolute(ship_name);
	Assert(ship_num != -1);	// make sure that the ship actually is valid

	while ( n != -1 )
	{
		fix time_known;
		int is_known;

		is_known = 0;
		count++;

		// see if we have already checked this entry
		if ( Sexp_nodes[n].value == SEXP_KNOWN_TRUE )
		{
			num_known++;
		}
		else
		{
			// get subsys name
			subsys_name = CTEXT(n);

			// get flags
			get_cap_subsys_cargo_flags(ship_num, subsys_name, &cargo_revealed, &time_revealed);

			// see if the ship has already exited the mission (either through departure or destruction)
			if (ship_find_exited_ship_by_name(ship_name) != -1)
			{
				// if not known, the whole thing is known false
				if (!cargo_revealed)
					return SEXP_KNOWN_FALSE;

				// check the delay of when we found out...
				// Since there is no way to keep track of subsystem status once a ship has departed
				// or has been destroyed, check the mission log.  This will work in 99.9999999% of
				// all cases; however, if the mission designer repeatedly sets and resets the scanned
				// status of the subsystem, the mission log will only return the first occurrence of the
				// subsystem cargo being revealed (regardless of whether it was first hidden using
				// set-unscanned).  Normally, ships keep track of cargo data in the subsystem struct,
				// but once/ the ship has left the mission, the subsystem linked list is purged,
				// causing the loss of this information.  I judged the significant rework of the
				// subsystem code not worth the rare instance that this sexp may be required to work
				// in this way, especially since this problem only occurs after the ship departs.  If
				// the mission designer really needs this functionality, he or she can achieve the
				// same result with creative combinations of event chaining and is-event-true.
				mission_log_get_time(LOG_CAP_SUBSYS_CARGO_REVEALED, ship_name, subsys_name, &time_known);

				if (f2i(Missiontime - time_known) >= delay)
				{
					is_known = 1;

					// here is the only place in the new sexp that this can be known true
					Sexp_nodes[n].value = SEXP_KNOWN_TRUE;
				}
			}
			// ship either in mission or not arrived yet
			else
			{
				// if ship_name_lookup returns -1, then ship is either exited or yet to arrive,
				// and we've already checked exited
				if ( ship_name_lookup(ship_name) != -1 )
				{
					if (cargo_revealed)
					{
						time_known = Missiontime - time_revealed;
						if ( f2i(time_known) >= delay )
						{
							is_known = 1;
						}
					}
				}
			}
		}

		// if cargo is known, mark our variable, but not the sexp, because it may change later
		if (is_known)
		{
			num_known++;
		}

		n = CDR(n);
	}

	Directive_count += count - num_known;
	if ( count == num_known )
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// Goober5000
void sexp_set_scanned_unscanned(int n, int flag)
{
	char *ship_name, *subsys_name;
	int shipnum, subsys_set;
	ship_subsys *ss;

	// get ship name
	ship_name = CTEXT(n);

	// check to see the ship was destroyed or departed - if so, do nothing
	if ( mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) || mission_log_get_time( LOG_SHIP_DEPART, ship_name, NULL, NULL) )
	{
		return;
	}

	// get ship number
	shipnum = ship_name_lookup(ship_name);

	// if the ship isn't in the mission, do nothing
	if (shipnum == -1)
	{
		return;
	}

	// check for possible next optional argument: subsystem
	n = CDR(n);

	// if no subsystem specified, just do it for the ship and exit
	if (n == -1)
	{
		if (flag)
			ship_do_cargo_revealed(&Ships[shipnum]);
		else
			ship_do_cargo_hidden(&Ships[shipnum]);

		return;
	}

	// iterate through all subsystems
	while (n != -1)
	{
		subsys_name = CTEXT(n);
		subsys_set = 0;

		// find the ship subsystem by searching ship's subsys_list
		ss = GET_FIRST( &Ships[shipnum].subsys_list );
		while ( ss != END_OF_LIST( &Ships[shipnum].subsys_list ) )
		{
			// if we found the subsystem
			if ( !stricmp(ss->system_info->subobj_name, subsys_name))
			{
				// do it for the subsystem
				if (flag)
					ship_do_cap_subsys_cargo_revealed(&Ships[shipnum], ss);
				else
					ship_do_cap_subsys_cargo_hidden(&Ships[shipnum], ss);

				subsys_set = 1;
			}

			ss = GET_NEXT( ss );
		}

		// if we didn't find the subsystem -- bad
		if (!subsys_set)
		{
			Int3();
		}

		// but if it did, loop again
		n = CDR(n);
	}
}

int sexp_has_been_tagged_delay(int n)
{
	int count, shipnum, num_known, delay;
	char *name;

	Assert ( n >= 0 );

	count = 0;
	num_known = 0;

	// get the delay value
	delay = atoi(CTEXT(n) );

	n = CDR(n);

	while ( n != -1 ) {
		fix time_known;
		int is_known;

		is_known = 0;

		count++;

		// see if we have already checked this entry
		if ( Sexp_nodes[n].value == SEXP_KNOWN_TRUE ) {
			num_known++;
		} else {
			int exited_index;

			name = CTEXT(n);

			// see if the ship has already exited the mission (either through departure or destruction).  If so,
			// grab the status of whether the cargo is known from this list
			exited_index = ship_find_exited_ship_by_name( name );
			if (exited_index != -1 ) {
				if ( !(Ships_exited[exited_index].flags & SEF_BEEN_TAGGED) )
					return SEXP_KNOWN_FALSE;

				// check the delay of when we found out.  We use the ship died time which isn't entirely accurate
				// but won't cause huge delays.
				time_known = Missiontime - Ships_exited[exited_index].time;
				if ( f2i(time_known) >= delay )
					is_known = 1;
			} else {

				// otherwise, ship should still be in the mission.  If ship_name_lookup returns -1, then ship
				// is yet to arrive.
				shipnum = ship_name_lookup( name );
				if ( shipnum != -1 ) {
					if ( Ships[shipnum].time_first_tagged != 0 ) {
						time_known = Missiontime - Ships[shipnum].time_first_tagged;
						if ( f2i(time_known) >= delay )
							is_known = 1;
					}
				}
			}
		}

		// if cargo is known, mark our variable and this sexpression.
		if ( is_known ) {
			num_known++;
			Sexp_nodes[n].value = SEXP_KNOWN_TRUE;
		}

		n = CDR(n);
	}

	Directive_count += count - num_known;
	if ( count == num_known )
		return SEXP_KNOWN_TRUE;
	else
		return 0;
}

// return object index of waypoint or -1 if no such waypoint
int waypoint_lookup(char *name)
{
	char buf[128];
	int i;
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_WAYPOINT) {
			i = ptr->instance;
			sprintf(buf, "%s:%d", Waypoint_lists[i / 65536].name, (i & 0xffff) + 1);
			if ( !stricmp(buf, name) )
				return OBJ_INDEX(ptr);
		}

		ptr = GET_NEXT(ptr);
	}

	return -1;
}

// conditional sexpressions follow
	
// eval_when evaluates the when conditional
int eval_when(int n)
{
	int cond, val;

	Assert( n >= 0 );				// must have valid sexp index

	cond = CAR(n);
	val = eval_sexp(cond);		// get the value of the the conditional
	if ( val ) {					// if the value is true, perform the actions is the 'then' part of the if
		int actions, exp;

		actions = CDR(n);
		while ( actions != -1 ) {
			exp = CAR(actions);
			if ( exp != -1 )
				val = eval_sexp(exp);								// these sexp evaled only for side effects
			actions = CDR(actions);
		}
	}

	if (Sexp_nodes[cond].value == SEXP_KNOWN_FALSE)
		return SEXP_KNOWN_FALSE;  // no need to waste time on this anymore

	if (val == SEXP_KNOWN_FALSE)
		return 0;  // can't return known false, as this would bypass future actions under the when

	return val;
}

// eval_cond() evaluates the cond conditional
int eval_cond( int n )
{
	int cond = 0, node, val = 0;

	Assert (n >= 0);
	while (n >= 0) {
		node = CAR(n);
		cond = CAR(node);
		val = eval_sexp(cond);

		// if the conditional evaluated to true, then we must evaluate the rest of the expression returning
		// the value of this evaluation
		if (val) {
			int actions, exp;

			val = 0;
			actions = CDR(node);
			while (actions >= 0) {
				exp = CAR(actions);
				if (exp >= -1)
					val = eval_sexp(exp);								// these sexp evaled only for side effects

				actions = CDR(actions);
			}

			break;
		}

		// move onto the next cond clause
		n = CDR(n);
	}

	return val;
}

// Goober5000 - added wing capability
int sexp_is_iff( int n )
{
	char *ship_or_wing_name, *iff;
	int i, ship_num, wing_num, team;
	wing *wingp;

	Assert ( n >= 0 );

	// iff value is the first parameter, second is a list of one or more ships to check to see if the
	// iff value matches
	iff = CTEXT(n);
	if ( !stricmp(iff, "friendly") )
		team = TEAM_FRIENDLY;
	else if ( !stricmp(iff, "hostile") )
		team = TEAM_HOSTILE;
	else if ( !stricmp(iff, "neutral") )
		team = TEAM_NEUTRAL;
	else if ( !stricmp(iff, "unknown") )
		team = TEAM_UNKNOWN;
	else if ( !stricmp(iff, "traitor") )
		team = TEAM_TRAITOR;
	else {
		Int3();
		mprintf(("Warning: Team %s no longer supported.  Just Friendly and Hostile.\n", iff));
		team = TEAM_HOSTILE;
	}

	n = CDR(n);
	for ( ; n != -1; n = CDR(n) )
	{
		ship_or_wing_name = CTEXT(n);

		ship_num = ship_name_lookup(ship_or_wing_name);
		wing_num = wing_name_lookup(ship_or_wing_name);

		if (ship_num != -1)
		{
			// if the team doesn't match the team specified, return false immediately
			if (!ship_is_iff(ship_num, team))
			{
				return SEXP_FALSE;
			}
		}

		if (wing_num != -1)
		{
			wingp = &Wings[wing_num];
			
			for (i=0; i<wingp->current_count; i++)
			{
				// if the team doesn't match the team specified, return false immediately
				if (!ship_is_iff(wingp->ship_index[i], team))
				{
					return SEXP_FALSE;
				}
			}
		}
	}

	// got this far: we must be okay for all ships
	return SEXP_TRUE;
}

// Goober5000 - added wing capability
void sexp_change_iff( int n )
{
	char *ship_or_wing_name, *new_iff;
	int i, ship_num, wing_num, new_team;
	wing *wingp;

	Assert ( n >= 0 );
	new_iff = CTEXT(n);
	if ( !stricmp(new_iff, "friendly") )
		new_team = TEAM_FRIENDLY;
	else if ( !stricmp(new_iff, "hostile") )
		new_team = TEAM_HOSTILE;
	else if ( !stricmp(new_iff, "neutral") )
		new_team = TEAM_NEUTRAL;
	else if ( !stricmp(new_iff, "unknown") )
		new_team = TEAM_UNKNOWN;
	else if ( !stricmp(new_iff, "traitor") )
		new_team = TEAM_TRAITOR;
	else {
		mprintf(("Warning: Team %s no longer supported.  Just Friendly and Hostile.\n", new_iff));
		new_team = TEAM_HOSTILE;
		Int3();
	}

	n = CDR(n);
	for ( ; n != -1; n = CDR(n) )
	{
		ship_or_wing_name = CTEXT(n);

		// see if we can find anything
		ship_num = ship_name_lookup(ship_or_wing_name);
		wing_num = wing_name_lookup(ship_or_wing_name);

		// change ship
		if (ship_num != -1)
		{
			ship_change_iff(ship_num, new_team);
		}

		// change wing
		if (wing_num != -1)
		{
			wingp = &Wings[wing_num];
			
			for (i=0; i<wingp->current_count; i++)
			{
				ship_change_iff(wingp->ship_index[i], new_team);
			}
		}
	}
}

// Goober5000
// ai class value is the first parameter, second is a ship, rest are subsystems to check
int sexp_is_ai_class( int n )
{
	char *ship_name, *subsystem;
	int i, ship_num, ai_class, class_to_test;

	Assert ( n >= 0 );

	// find ai class
	ai_class = -1;
	for (i=0; i<Num_ai_classes; i++)
	{
		if (!stricmp(Ai_class_names[i], CTEXT(n)))
			ai_class = i;
	}

	Assert(ai_class >= 0);

	n = CDR(n);
	ship_name = CTEXT(n);
	n = CDR(n);

	// find ship
	ship_num = ship_name_lookup(ship_name);

	// we can't do anything with ships that aren't present
	if (ship_num != -1)
		return SEXP_CANT_EVAL;

	// subsys?
	if (n != 1)
	{
		ship_subsys *ss;

		// loopity-loop
		for ( ; n != -1; n = CDR(n) )
		{
			subsystem = CTEXT(n);
			class_to_test = -1;

			// find the ship subsystem by searching ship's subsys_list
			ss = GET_FIRST( &Ships[ship_num].subsys_list );
			while ( ss != END_OF_LIST( &Ships[ship_num].subsys_list ) )
			{
				// if we found the subsystem
				if ( !stricmp(ss->system_info->subobj_name, subsystem))
				{
					// get ai class
					class_to_test = ss->weapons.ai_class;
					break;
				}

				ss = GET_NEXT( ss );
			}
			Assert(class_to_test >= 0);

			// if no match, return false immediately
			if (ai_class != class_to_test)
				return SEXP_FALSE;
		}

		// we've come this far; it must all be true
		return SEXP_TRUE;
	}
	// just the ship
	else
	{
		if (Ship_info[Ships[ship_num].ship_info_index].ai_class == ai_class)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
	}
}

// Goober5000
void sexp_change_ai_class( int n )
{
	int i, ship_num, new_ai_class;

	Assert ( n >= 0 );

	// find ai class
	new_ai_class = -1;
	for (i=0; i<Num_ai_classes; i++)
	{
		if (!stricmp(Ai_class_names[i], CTEXT(n)))
			new_ai_class = i;
	}

	Assert(new_ai_class >= 0);

	// find ship
	n = CDR(n);
	ship_num = ship_name_lookup(CTEXT(n));
	n = CDR(n);

	// we can't do anything with ships that aren't present
	if (ship_num != -1)
		return;

	// subsys?
	if (n != 1)
	{
		// loopity-loop
		for ( ; n != -1; n = CDR(n) )
		{
			ship_subsystem_set_new_ai_class(ship_num, CTEXT(n), new_ai_class);

#ifndef NO_NETWORK
			// send a network packet if we need to
			if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Ships[ship_num].objnum >= 0))
			{
				send_change_ai_class_packet(Objects[Ships[ship_num].objnum].net_signature, CTEXT(n), new_ai_class);
			}
#endif
		}
	}
	// just the one ship
	else
	{
		ship_set_new_ai_class(ship_num, new_ai_class);

#ifndef NO_NETWORK
		// send a network packet if we need to
		if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Ships[ship_num].objnum >= 0))
		{
			send_change_ai_class_packet(Objects[Ships[ship_num].objnum].net_signature, NULL, new_ai_class);
		}
#endif
	}
}

// following routine adds an ai goal to a ship structure.  The sexpression index
// passed in should be an ai-goal of the proper form.  The code in MissionGoal should
// check the syntax.

void sexp_add_ship_goal( int n )
{
	int num, sindex;
	char *ship_name;

	Assert ( n >= 0 );
	ship_name = CTEXT(n);
	num = ship_name_lookup(ship_name);
	if ( num < 0 )									// ship not around anymore???? then forget it!
		return;

	sindex = CDR(n);
	ai_add_ship_goal_sexp( sindex, AIG_TYPE_EVENT_SHIP, &(Ai_info[Ships[num].ai_index]) );
}

// identical to above, except add a wing
void sexp_add_wing_goal( int n )
{
	int num, sindex;
	char *wing_name;

	Assert ( n >= 0 );
	wing_name = CTEXT(n);
	num = wing_name_lookup(wing_name);
	if ( num < 0 )									// ship not around anymore???? then forget it!
		return;

	sindex = CDR(n);
	ai_add_wing_goal_sexp( sindex, AIG_TYPE_EVENT_WING, num );
}

// sexp_add_goal adds a goal to the specified entiry (ships and wings have unique names between
// the two sets).
void sexp_add_goal( int n )
{
	int num, sindex;
	char *name;

	Assert ( n >= 0 );
	name = CTEXT(n);
	sindex = CDR(n);

	// first, look for ship name -- if found, then add ship goal.  else look for wing name -- if
	// found, add wing goal
	if ( (num = ship_name_lookup(name)) != -1 )
		ai_add_ship_goal_sexp( sindex, AIG_TYPE_EVENT_SHIP, &(Ai_info[Ships[num].ai_index]) );
	else if ( (num = wing_name_lookup(name)) != -1 )
		ai_add_wing_goal_sexp( sindex, AIG_TYPE_EVENT_WING, num );
}

// clears out all ai goals for a ship
void sexp_clear_ship_goals( int n )
{
	int num;
	char *ship_name;

	Assert ( n >= 0 );
	ship_name = CTEXT(n);
	num = ship_name_lookup(ship_name);
	ai_clear_ship_goals( &(Ai_info[Ships[num].ai_index]) );
}

// clears out ai goals for a wing
void sexp_clear_wing_goals( int n )
{
	int num;
	char *wing_name;

	Assert ( n >= 0 );
	wing_name = CTEXT(n);
	num = wing_name_lookup(wing_name);
	if ( num < 0 )
		return;
	ai_clear_wing_goals( num );
}

// this function clears all ai goals for the given ship or wing
void sexp_clear_goals( int n )
{
	int num;
	char *name;

	Assert ( n >= 0 );
	while ( n != -1 ) {
		name = CTEXT(n);
		if ( (num = ship_name_lookup(name)) != -1 )
			ai_clear_ship_goals( &(Ai_info[Ships[num].ai_index]) );
		else if ( (num = wing_name_lookup(name)) != -1 )
			ai_clear_wing_goals( num );

		n = CDR(n);
	}
}

// this function get called by send-message or send-message random with the name of the message, sender,
// and priority.
void sexp_send_one_message( char *name, char *who_from, char *priority, int group, int delay )
{
	int ipriority, num, ship_index, source;
	ship *shipp;

	if(physics_paused){
		return;
	}

	// determine the priority of the message
	if ( !stricmp(priority, "low") )
		ipriority = MESSAGE_PRIORITY_LOW;
	else if ( !stricmp(priority, "normal") )
		ipriority = MESSAGE_PRIORITY_NORMAL;
	else if ( !stricmp(priority, "high") )
		ipriority = MESSAGE_PRIORITY_HIGH;
	else {
		Int3();
		ipriority = MESSAGE_PRIORITY_NORMAL;
	}

	// check to see if the 'who_from' string is a ship that had been destroyed or departed.  If so,
	// then don't send the message.  We must look at 'who_from' to determine what to look for.  who_from
	// may be any allied person, any wingman, a wingman from a specific wing, or a specific ship
	ship_index = -1;
	shipp = NULL;
	source = MESSAGE_SOURCE_COMMAND;
	if ( who_from[0] == '#' ) {
		message_send_unique_to_player( name, &(who_from[1]), MESSAGE_SOURCE_SPECIAL, ipriority, group, delay );
		return;
	} else if (!stricmp(who_from, "<any allied>")) {
		//Int3();			// no longer supported
		return;
	} else if ( (num = wing_name_lookup(who_from)) != -1 ) {
		// message from a wing
		// this will be an invalid case soon
		// Int3();
		// choose wing leader to speak for wing (hence "1" at end of ship_get_random_ship_in_wing)
		ship_index = ship_get_random_ship_in_wing( num, SHIP_GET_NO_PLAYERS, 1 );
		if ( ship_index == -1 ) {
			if ( ipriority != MESSAGE_PRIORITY_HIGH )
				return;
		}

	} else if ( mission_log_get_time(LOG_SHIP_DESTROYED, who_from, NULL, NULL) || mission_log_get_time(LOG_SHIP_DEPART, who_from, NULL, NULL) 
		|| mission_log_get_time(LOG_WING_DESTROYED, who_from, NULL, NULL) || mission_log_get_time(LOG_WING_DEPART, who_from, NULL, NULL) ) {
		// getting into this if statement means that the ship or wing (sender) is no longer in the mission
		// if message is high priority, make it come from Terran Command
		if ( ipriority != MESSAGE_PRIORITY_HIGH )
			return;
		
		source = MESSAGE_SOURCE_COMMAND;

	} else if ( !stricmp(who_from, "<any wingman>") || (wing_name_lookup(who_from) != -1) ) {
		source = MESSAGE_SOURCE_WINGMAN;
	} else {
		// Message from a apecific ship
		// bail if not high priority, otherwise reroute to command
		source = MESSAGE_SOURCE_SHIP;
		ship_index = ship_name_lookup(who_from);
		if ( ship_index == -1 ) {
			if ( ipriority != MESSAGE_PRIORITY_HIGH )
				return;
			source = MESSAGE_SOURCE_COMMAND;
		}
	}

	if ( ship_index == -1 ){
		shipp = NULL;
	} else {
		shipp = &Ships[ship_index];
	}

	message_send_unique_to_player( name, shipp, source, ipriority, group, delay );
}

// Goober5000
void sexp_change_music(int n)
{
	event_sexp_change_music(CTEXT(n));
}

// Goober5000
void sexp_explosion_effect(int n)
/* From the SEXP help...
	{ OP_EXPLOSION_EFFECT, "explosion-effect\r\n"
		"\tCauses an explosion at a given origin, with the given parameters.  The explosion goes off immediately.\r\n"
		"Takes 10 arguments...\r\n"
		"\t1:  Origin X\r\n"
		"\t2:  Origin Y\r\n"
		"\t3:  Origin Z\r\n"
		"\t4:  Damage\r\n"
		"\t5:  Blast force\r\n"
		"\t6:  Size of explosion\r\n"
		"\t7:  Inner radius to apply damage\r\n"
		"\t8:  Outer radius to apply damage\r\n"
		"\t9:  Shockwave speed (if 0, there will be no shockwave)\r\n"
		"\t10: Type (0 = medium, 1 = large1, 2 = large2)\r\n"
		"\t11: Sound (index into sounds.tbl)" },
*/
// Basically, this function pretends that there's a ship at the origin that's blowing up, and
// it does stuff accordingly.  In some places, it has to tiptoe around a little because the
// code often expects a parent object when in fact there is none. <.<  >.>
{
	vector origin;
	int max_damage, max_blast, explosion_size, inner_radius, outer_radius, shockwave_speed, fireball_type, sound_index;
	shockwave_create_info sci;

	// read in data --------------------------------
	origin.xyz.x = (float)atoi(CTEXT(n));
	n = CDR(n);
	origin.xyz.y = (float)atoi(CTEXT(n));
	n = CDR(n);
	origin.xyz.z = (float)atoi(CTEXT(n));
	n = CDR(n);

	max_damage = atoi(CTEXT(n));
	n = CDR(n);
	max_blast = atoi(CTEXT(n));
	n = CDR(n);

	explosion_size = atoi(CTEXT(n));
	n = CDR(n);
	inner_radius = atoi(CTEXT(n));
	n = CDR(n);
	outer_radius = atoi(CTEXT(n));
	n = CDR(n);

	shockwave_speed = atoi(CTEXT(n));
	n = CDR(n);

	// fireball type
	if (atoi(CTEXT(n)) == 0)
	{
		fireball_type = FIREBALL_EXPLOSION_MEDIUM;
	}
	else if (atoi(CTEXT(n)) == 1)
	{
		fireball_type = FIREBALL_EXPLOSION_LARGE1;
	}
	else if (atoi(CTEXT(n)) == 2)
	{
		fireball_type = FIREBALL_EXPLOSION_LARGE2;
	}
	else
	{
		Warning(LOCATION, "explosion-effect type is out of range; quitting the explosion...\n");
		return;
	}
	n = CDR(n);

	sound_index = atoi(CTEXT(n));


	// play sound effect ---------------------------
	snd_play_3d( &Snds[sound_index], &origin, &View_position, 0.0f, NULL, 0, 1.0f, SND_PRIORITY_MUST_PLAY  );


	// create the fireball -------------------------
	fireball_create( &origin, fireball_type, -1, (float)explosion_size );


	// apply area affect damage --------------------
	if (!max_damage && !max_blast)
		return;

	if ( shockwave_speed > 0 )
	{
		sci.inner_rad = (float)inner_radius;
		sci.outer_rad = (float)outer_radius;
		sci.blast = (float)max_blast;
		sci.damage = (float)max_damage;
		sci.speed = (float)shockwave_speed;
		sci.rot_angle = frand_range(0.0f, 359.0f);
		shockwave_create(-1, &origin, &sci, SW_SHIP_DEATH);
	}
	else
	{
		object *objp;
		float t_blast = 0.0f;
		float t_damage = 0.0f;
		for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
		{
			if ( (objp->type != OBJ_SHIP) && (objp->type != OBJ_ASTEROID) )
			{
				continue;
			}
		
			// don't blast navbuoys
			if ( objp->type == OBJ_SHIP )
			{
				if ( ship_get_SIF(objp->instance) & SIF_NAVBUOY )
				{
					continue;
				}
			}

			if ( ship_explode_area_calc_damage( &origin, &objp->pos, (float)inner_radius, (float)outer_radius, (float)max_damage, (float)max_blast, &t_damage, &t_blast ) == -1 )
			{
				continue;
			}

			switch ( objp->type )
			{
				case OBJ_SHIP:
					ship_apply_global_damage( objp, NULL, &origin, t_damage );
					vector force, vec_ship_to_impact;
					vm_vec_sub( &vec_ship_to_impact, &objp->pos, &origin );
					vm_vec_copy_normalize( &force, &vec_ship_to_impact );
					vm_vec_scale( &force, (float)max_blast );
					ship_apply_whack( &force, &vec_ship_to_impact, objp );
					break;

				case OBJ_ASTEROID:
					asteroid_hit(objp, NULL, NULL, t_damage);
					break;

				default:
					Int3();
					break;
			}
		}	// end for
	}
}

// Goober5000
void sexp_warp_effect(int n)
/* From the SEXP help...
	{ OP_WARP_EFFECT, "warp-effect\r\n"
		"\tCauses a subspace warp effect at a given origin, facing toward a given location, with the given parameters.\r\n"
		"Takes 12 arguments...\r\n"
		"\t1:  Origin X\r\n"
		"\t2:  Origin Y\r\n"
		"\t3:  Origin Z\r\n"
		"\t4:  Location X\r\n"
		"\t5:  Location Y\r\n"
		"\t6:  Location Z\r\n"
		"\t7:  Radius\r\n"
		"\t8:  Duration in seconds\r\n"
		"\t9:  Warp opening sound (index into sounds.tbl)\r\n"
		"\t10: Warp closing sound (index into sounds.tbl)\r\n"
		"\t11: Type (0 for standard blue [default], 1 for Knossos green)\r\n"
		"\t12: Effect (0 for Bobboau's new effect [default], 1 for original FS2 effect)" },
*/
{
	vector origin, location, v_orient;
	matrix m_orient;
	int radius, duration, warp_open_sound_index, warp_close_sound_index, fireball_type, extra_flags;
	extra_flags = FBF_WARP_VIA_SEXP;

	// read in data --------------------------------
	origin.xyz.x = (float)atoi(CTEXT(n));
	n = CDR(n);
	origin.xyz.y = (float)atoi(CTEXT(n));
	n = CDR(n);
	origin.xyz.z = (float)atoi(CTEXT(n));
	n = CDR(n);

	location.xyz.x = (float)atoi(CTEXT(n));
	n = CDR(n);
	location.xyz.y = (float)atoi(CTEXT(n));
	n = CDR(n);
	location.xyz.z = (float)atoi(CTEXT(n));
	n = CDR(n);

	radius = atoi(CTEXT(n));
	n = CDR(n);
	duration = atoi(CTEXT(n));
	if (duration < 4) duration = 4;
	n = CDR(n);

	warp_open_sound_index = atoi(CTEXT(n));
	n = CDR(n);
	warp_close_sound_index = atoi(CTEXT(n));
	n = CDR(n);

	// fireball type
	if (atoi(CTEXT(n)) == 0)
	{
		fireball_type = FIREBALL_WARP_EFFECT;
	}
	else if (atoi(CTEXT(n)) == 1)
	{
		fireball_type = FIREBALL_KNOSSOS_EFFECT;
	}
	else
	{
		Warning(LOCATION, "warp-effect type is out of range; quitting the warp...\n");
		return;
	}
	n = CDR(n);

	// effect type
	if (atoi(CTEXT(n)) == 0)
	{
		// do nothing; this is standard
	}
	else if (atoi(CTEXT(n)) == 1)
	{
		extra_flags |= FBF_WARP_FORCE_OLD;
	}
	else
	{
		Warning(LOCATION, "warp-effect effect is out of range; quitting the warp...\n");
		return;
	}


	// calculate orientation matrix ----------------
	memset(&v_orient, 0, sizeof(vector));

	vm_vec_sub(&v_orient, &location, &origin);

	if (IS_VEC_NULL(&v_orient))
	{
		Warning(LOCATION, "error in warp-effect: warp can't point to itself; quitting the warp...\n");
		return;
	}

	vm_vector_2_matrix(&m_orient, &v_orient, NULL, NULL);

	// create fireball -----------------------------
	fireball_create(&origin, fireball_type, -1, (float)radius, 0, NULL, (float)duration, -1, &m_orient, 0, extra_flags, warp_open_sound_index, warp_close_sound_index);
}

// Goober5000
void sexp_emp_effect(int n)
/* From the SEXP help...
	{ OP_EMP_EFFECT, "emp-effect\r\n"
		"\tCauses an emp blast at a given origin, with the given parameters.\r\n"
		"Takes 5 arguments...\r\n"
		"\t1: Origin X\r\n"
		"\t2: Origin Y\r\n"
		"\t3: Origin Z\r\n"
		"\t4: Intensity\r\n"
		"\t5: Duration in seconds" },*/
{
	vector origin;
	int intensity, duration;

	// read in data --------------------------------
	origin.xyz.x = (float)atoi(CTEXT(n));
	n = CDR(n);
	origin.xyz.y = (float)atoi(CTEXT(n));
	n = CDR(n);
	origin.xyz.z = (float)atoi(CTEXT(n));
	n = CDR(n);

	intensity = atoi(CTEXT(n));
	n = CDR(n);
	duration = atoi(CTEXT(n));


	// start emp effect
}

void sexp_send_message( int n )
{
	char *name, *who_from, *priority, *tmp;

	if(physics_paused){
		return;
	}

	Assert ( n != -1 );
	who_from = CTEXT(n);
	priority = CTEXT(CDR(n));
	name = CTEXT(CDR(CDR(n)));

	// a temporary check to see if the name field matched a priority since I am in the process
	// of reordering the arguments
	if ( !stricmp(name, "low") || !stricmp(name, "normal") || !stricmp(name, "high") ) {
		tmp = name;
		name = priority;
		priority = tmp;
	}

	sexp_send_one_message( name, who_from, priority, 0, 0 );
}

void sexp_send_message_list( int n )
{
	char *name, *who_from, *priority;
	int delay;

	if(physics_paused){
		return;
	}

	// send a bunch of messages
	delay = 0;
	while(n != -1){
		who_from = CTEXT(n);

		// next node
		n = CDR(n);
		if(n == -1){
			Warning(LOCATION, "Detected incomplete parameter list in sexp-send-message-list");
			return;
		}
		priority = CTEXT(n);

		// next node
		n = CDR(n);
		if(n == -1){
			Warning(LOCATION, "Detected incomplete parameter list in sexp-send-message-list");
			return;
		}
		name = CTEXT(n);

		// next node
		n = CDR(n);
		if(n == -1){
			Warning(LOCATION, "Detected incomplete parameter list in sexp-send-message-list");
			return;
		}
		delay += atoi(CTEXT(n));

		// send the message
		sexp_send_one_message(name, who_from, priority, 1, delay);

		// next node
		n = CDR(n);
	}
}

void sexp_send_random_message( int n )
{
	char *name, *who_from, *priority;
	int temp, num_messages, message_num;

	Assert ( n != -1 );
	who_from = CTEXT(n);
	priority = CTEXT(CDR(n));

	if(physics_paused){
		return;
	}

	// count the number of messages that we have
	n = CDR(CDR(n));
	temp = n;
	num_messages = 0;
	while ( n != -1 ) {
		n = CDR(n);
		num_messages++;
	}
	Assert ( num_messages >= 1 );
	
	// get a random message, and pass the parameters to send_one_message
	message_num = myrand() % num_messages;
	n = temp;
	while ( n != -1 ) {
		if ( message_num == 0 )
			break;
		message_num--;
		n = CDR(n);
	}
	Assert (n != -1);		// should have found the message!!!
	name = CTEXT(n);

	sexp_send_one_message( name, who_from, priority, 0, 0 );
}

void sexp_self_destruct( int n )
{
	char *ship_name;
	int shipnum;

	while ( n != -1 ) {
		// get the ship name and be sure that it is still in the mission.  Destroy it if we find it
		ship_name = CTEXT(n);
		shipnum = ship_name_lookup( ship_name );
		if ( shipnum == -1 )
			return;
		ship_self_destruct( &Objects[Ships[shipnum].objnum] );

		n = CDR(n);
	}
}

void sexp_next_mission( int n )
{
	char *mission_name;
	int i;

	mission_name = CTEXT(n);
	for (i = 0; i < Campaign.num_missions; i++) {
		if ( !stricmp(Campaign.missions[i].name, mission_name) ) {
			Campaign.next_mission = i;
			return;
		}
	}
	Error(LOCATION, "Mission name %s not found in campaign file for next-mission command", mission_name);
}

// function to deal with the end-of-campaign sexpression.  
void sexp_end_of_campaign( int n )
{
	// this is really a do-nothing sexpression.  It is pretty much a placeholder to allow
	// campaigns to have repeat-mission branches at the end of the campaign.  By not setting
	// anything in this function, the higher level campaign code will see this as end-of-campaign
	// since next_mission isn't set to anything.  (To be safe, we'll set to -1).
	Campaign.next_mission = -1;	
}

// sexpression to end everything.  One parameter is the movie to play when this is over.
void sexp_end_campaign( int n )
{
	// post and event to move us to the end-of-campaign state.  There we will play a movie, then
	// go to debriefing.
	// gameseq_post_event( GS_EVENT_END_CAMPAIGN );

	// in FS2 our ending is a bit wacky. we'll just flag the mission as having ended the campaign	
	Campaign_ended_in_mission = 1;
}

// sabotage subsystem reduces the strength of a subsystem by the given percentage.  If it is reduced to
// below 0%, then the hits of the subsystem are set to 0
void sexp_sabotage_subsystem( int n )
{
	char *shipname, *subsystem;
	int	percentage, shipnum, index;
	float sabotage_hits;
	ship	*shipp;
	ship_subsys *ss;

	shipname = CTEXT(n);
	subsystem = CTEXT(CDR(n));
	percentage = atoi(CTEXT(CDR(CDR(n))));

	shipnum = ship_name_lookup(shipname);
	
	// if no ship, then return immediately.
	if ( shipnum == -1 )
		return;
	shipp = &Ships[shipnum];

	// see if we are dealing with the HULL
	if ( !stricmp( subsystem, SEXP_HULL_STRING) ) {
		float ihs;
		object *objp;

		ihs = Ship_info[shipp->ship_info_index].initial_hull_strength;
		sabotage_hits = ihs * ((float)percentage / 100.0f);
		objp = &Objects[shipp->objnum];
		objp->hull_strength -= sabotage_hits;

		// self destruct the ship if <= 0.
		if ( objp->hull_strength <= 0.0f )
			ship_self_destruct( objp );
		return;
	}

	// now find the given subsystem on the ship.  The subsystem should be an actual subsystem name
	// and not a generic type (generic type meaning SUBSYSTEM_ENGINE, etc).

	index = ship_get_subsys_index(shipp, subsystem);
	if ( index == -1 ) {
		nprintf(("Warning", "Couldn't find subsystem %s on ship %s for sabotage subsystem\n", subsystem, shipp->ship_name));
		return;
	}

	// get the pointer to the subsystem.  Check it's current hits against it's max hits, and
	// set the strength to the given percentage if current strength is > given percentage
	ss = ship_get_indexed_subsys( shipp, index );
	sabotage_hits = ss->system_info->max_hits * ((float)percentage / 100.0f);
	ss->current_hits -= sabotage_hits;
	if ( ss->current_hits < 0.0f )
		ss->current_hits = 0.0f;
	ship_recalc_subsys_strength( shipp );
}

// repair_subsystem adds some percentage of hits to a subsystem.  Anything repaired about 100% is
// set to max hits
void sexp_repair_subsystem( int n )
{
	char *shipname, *subsystem;
	int	percentage, shipnum, index;
	float repair_hits;
	ship *shipp;
	ship_subsys *ss;

	shipname = CTEXT(n);
	subsystem = CTEXT(CDR(n));
	shipnum = ship_name_lookup(shipname);
	
	// if no ship, then return immediately.
	if ( shipnum == -1 ) {
		return;
	}
	shipp = &Ships[shipnum];
	
	// check if we've got a number or an op
	if ( CAR(CDR(CDR(n))) != -1) {
		percentage = eval_sexp( CAR(CDR(CDR(n))) );
	} else {
		percentage = atoi(CTEXT(CDR(CDR(n))));
	}

	// see if we are dealing with the HULL
	if ( !stricmp( subsystem, SEXP_HULL_STRING) ) {
		float ihs;
		object *objp;

		ihs = Ship_info[shipp->ship_info_index].initial_hull_strength;
		repair_hits = ihs * ((float)percentage / 100.0f);
		objp = &Objects[shipp->objnum];
		objp->hull_strength += repair_hits;
		if ( objp->hull_strength > ihs )
			objp->hull_strength = ihs;
		return;
	}

	// now find the given subsystem on the ship.  The subsystem should be an actual subsystem name
	// and not a generic type (generic type meaning SUBSYSTEM_ENGINE, etc).

	index = ship_get_subsys_index(shipp, subsystem);
	if ( index == -1 ) {
		nprintf(("Warning", "Couldn't find subsystem %s on ship %s for repair subsystem\n", subsystem, shipp->ship_name));
		return;
	}

	// get the pointer to the subsystem.  Check it's current hits against it's max hits, and
	// set the strength to the given percentage if current strength is < given percentage
	ss = ship_get_indexed_subsys( shipp, index );
	repair_hits = ss->system_info->max_hits * ((float)percentage / 100.0f);
	ss->current_hits += repair_hits;
	if ( ss->current_hits > ss->system_info->max_hits )
		ss->current_hits = ss->system_info->max_hits;
	ship_recalc_subsys_strength( shipp );
}

// sexpression code to set a subsystem of a ship at a specific percentage
void sexp_set_subsystem_strength( int n )
{
	char *shipname, *subsystem;
	int	percentage, shipnum, index;
	ship *shipp;
	ship_subsys *ss;

	shipname = CTEXT(n);
	subsystem = CTEXT(CDR(n));
	percentage = num_eval(CDR(CDR(n)));

	shipnum = ship_name_lookup(shipname);
	
	// if no ship, then return immediately.
	if ( shipnum == -1 )
		return;
	shipp = &Ships[shipnum];

	if ( percentage > 100 ) {
		nprintf(("Warning", "percentage for set_subsystem_strength > 100 -- setting to 100\n"));
		percentage = 100;
	} else if ( percentage < 0 ) {
		nprintf(("Werning", "percantage for set_subsystem_strength < 0 -- setting to 0\n"));
		percentage = 0;
	}

	// see if we are dealing with the HULL
	if ( !stricmp( subsystem, SEXP_HULL_STRING) ) {
		float ihs;
		object *objp;

		objp = &Objects[shipp->objnum];

		// destroy the ship if percentage is 0
		if ( percentage == 0 ) {
			ship_self_destruct( objp );
		} else {
			ihs = Ship_info[shipp->ship_info_index].initial_hull_strength;
			objp->hull_strength = ihs * ((float)percentage / 100.0f);
		}

		return;
	}

	// now find the given subsystem on the ship.  The subsystem should be an actual subsystem name
	// and not a generic type (generic type meaning SUBSYSTEM_ENGINE, etc).

	index = ship_get_subsys_index(shipp, subsystem);
	if ( index == -1 ) {
		nprintf(("Warning", "Couldn't find subsystem %s on ship %s for repair subsystem\n", subsystem, shipp->ship_name));
		return;
	}

	// get the pointer to the subsystem.  Check it's current hits against it's max hits, and
	// set the strength to the given percentage if current strength is < given percentage
	ss = ship_get_indexed_subsys( shipp, index );

	// maybe blow up subsys
	if (ss->current_hits > 0) {
		if (percentage < 1) {
			do_subobj_destroyed_stuff(shipp, ss, NULL);
		}
	}

	// set hit points
	ss->current_hits = ss->system_info->max_hits * ((float)percentage / 100.0f);

	ship_recalc_subsys_strength( shipp );
}

// function which changes the validity of a goal.  The flag paramater tells us whether to mark the goals
// as valid or invalid
void sexp_change_goal_validity( int n, int flag )
{
	char *name;

	while ( n != -1 ) {
		name = CTEXT(n);
		if ( flag )
			mission_goal_mark_valid( name );
		else
			mission_goal_mark_invalid( name );

		n = CDR(n);
	}
}

// Goober5000
// yeesh - be careful of the cargo-no-deplete flag :p
int sexp_is_cargo(int n)
{
	char *cargo, *ship, *subsystem;
	int ship_num, cargo_index;

	cargo = CTEXT(n);
	ship = CTEXT(CDR(n));
	if (CDR(CDR(n)) != -1)
		subsystem = CTEXT(CDR(CDR(n)));
	else
		subsystem = NULL;

	cargo_index = -1;

	// find ship
	ship_num = ship_name_lookup_absolute(ship);

	// in-mission?
	if (ship_name_lookup(ship) != -1)
	{
		if (subsystem)
		{
			ship_subsys *ss;

			// find the ship subsystem by searching ship's subsys_list
			ss = GET_FIRST( &Ships[ship_num].subsys_list );
			while ( ss != END_OF_LIST( &Ships[ship_num].subsys_list ) )
			{
				// if we found the subsystem
				if ( !stricmp(ss->system_info->subobj_name, subsystem))
				{
					// set cargo
					cargo_index = ss->subsys_cargo_name;
					break;
				}

				ss = GET_NEXT( ss );
			}
		}
		else
		{
			cargo_index = Ships[ship_num].cargo1;
		}
	}
	else
	{
		// can't check subsys of ships not in mission
		if (subsystem)
		{
			return SEXP_CANT_EVAL;
		}

		// departed?
		if (ship_find_exited_ship_by_name(ship) != -1)
		{
			cargo_index = Ships[ship_num].cargo1;
		}
		// not arrived yet
		else
		{
			p_object *parse_obj;

			// find cargo for the parse object
			parse_obj = mission_parse_get_arrival_ship( ship );
			Assert ( parse_obj );
			cargo_index = int(parse_obj->cargo1);
		}
	}

	// did we get any cargo
	if (cargo_index < 0)
		return SEXP_FALSE;

	// check cargo
	if (!stricmp(Cargo_names[cargo_index & CARGO_INDEX_MASK], cargo))
		return SEXP_TRUE;
	else
		return SEXP_FALSE;
}

// Goober5000
// yeesh - be careful of the cargo-no-deplete flag :p
void sexp_set_cargo(int n)
{
	char *cargo, *ship, *subsystem;
	int ship_num, i, cargo_index;

	cargo = CTEXT(n);
	ship = CTEXT(CDR(n));
	if (CDR(CDR(n)) != -1)
		subsystem = CTEXT(CDR(CDR(n)));
	else
		subsystem = NULL;

	// check to see if ship destroyed or departed.  In either case, do nothing.
	if ( mission_log_get_time(LOG_SHIP_DEPART, ship, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship, NULL, NULL) )
		return;

	cargo_index = -1;
	// find this cargo in the cargo list
	for (i = 0; i < Num_cargo; i++)
	{
		// found it?
		if (!stricmp(cargo, Cargo_names[i]))
		{
			cargo_index = i;
			break;
		}
	}

	// not found
	if (cargo_index == -1)
	{
		// make new entry if possible
		if (Num_cargo + 1 >= MAX_CARGO)
			return;

		Assert(strlen(cargo) <= NAME_LENGTH - 1);

		cargo_index = Num_cargo;
		Num_cargo++;

		strcpy(Cargo_names[cargo_index], cargo);
	}

	// get the ship
	ship_num = ship_name_lookup(ship);

	// we can only set subsystems if the ship is in the mission
	if (ship_num != -1)
	{
		if (subsystem)
		{
			ship_subsys *ss;

			// find the ship subsystem by searching ship's subsys_list
			ss = GET_FIRST( &Ships[ship_num].subsys_list );
			while ( ss != END_OF_LIST( &Ships[ship_num].subsys_list ) )
			{
				// if we found the subsystem
				if ( !stricmp(ss->system_info->subobj_name, subsystem))
				{
					// set cargo
					ss->subsys_cargo_name = cargo_index | (ss->subsys_cargo_name & CARGO_NO_DEPLETE);
					return;
				}

				ss = GET_NEXT( ss );
			}
			// we reached end of ship subsys list without finding subsys_name
			Int3();
		}
		else
		{
			// simply set the ship cargo
			Ships[ship_num].cargo1 = char(cargo_index | (Ships[ship_num].cargo1 & CARGO_NO_DEPLETE));
		}
	}
	else
	{
		if (!subsystem)
		{
			p_object *parse_obj;

			// set cargo for the parse object
			parse_obj = mission_parse_get_arrival_ship( ship );
			Assert ( parse_obj );
			parse_obj->cargo1 = char(cargo_index | (parse_obj->cargo1 & CARGO_NO_DEPLETE));
		}
	}
}

// function to transfer cargo from one ship to another
void sexp_transfer_cargo( int n )
{
	char *shipname1, *shipname2;
	int shipnum1, shipnum2, i;
	object *objp;

	shipname1 = CTEXT(n);
	shipname2 = CTEXT(CDR(n));

	// find the ships -- if neither in the mission, the abort
	shipnum1 = ship_name_lookup(shipname1);
	shipnum2 = ship_name_lookup(shipname2);
	if ( (shipnum1 == -1) || (shipnum2 == -1) )
		return;

	// we must be sure that these two objects are indeed docked
	objp = ai_find_docked_object( &Objects[Ships[shipnum1].objnum] );
	if ( objp != &Objects[Ships[shipnum2].objnum] ) {
		Int3();			// you are trying to transfer cargo between two ships not docked
		return;
	}

	if ( !stricmp(Cargo_names[Ships[shipnum1].cargo1 & CARGO_INDEX_MASK], "nothing") ) {
		Int3();			// you are transfering no cargo!!!!
		return;
	}

	// transfer cargo from ship1 to ship2
#ifndef NDEBUG
	// Don't give warning for large ships (cruiser on up) 
	if (! (Ship_info[Ships[shipnum2].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
		if ( stricmp(Cargo_names[Ships[shipnum2].cargo1 & CARGO_INDEX_MASK], "nothing") ) {
			Warning(LOCATION, "Transfering cargo to %s which already\nhas cargo %s.\nCargo will be replaced", Ships[shipnum2].ship_name, Cargo_names[Ships[shipnum2].cargo1 & CARGO_INDEX_MASK] );
		}
	}
#endif
	Ships[shipnum2].cargo1 = char(Ships[shipnum1].cargo1 & CARGO_INDEX_MASK);

	if ( !(Ships[shipnum1].cargo1 & CARGO_NO_DEPLETE) ) {
		// need to set ship1's cargo to nothing.  scan the cargo_names array looking for the string nothing.
		// add it if not found
		for (i = 0; i < Num_cargo; i++ ) {
			if ( !stricmp(Cargo_names[i], "nothing") ) {
				Ships[shipnum1].cargo1 = char(i);
				return;
			}
		}
		strcpy(Cargo_names[i], "Nothing");
		Num_cargo++;
	}
}

// this function exchanges cargo between two ships
void sexp_exchange_cargo( int n )
{
	char *shipname1, *shipname2;
	int shipnum1, shipnum2, temp;
	object *objp;

	shipname1 = CTEXT(n);
	shipname2 = CTEXT(CDR(n));

	// find the ships -- if neither in the mission, the abort
	shipnum1 = ship_name_lookup(shipname1);
	shipnum2 = ship_name_lookup(shipname2);
	if ( (shipnum1 == -1) || (shipnum2 == -1) )
		return;

	// we must be sure that these two objects are indeed docked
	objp = ai_find_docked_object( &Objects[Ships[shipnum1].objnum] );
	if ( objp != &Objects[Ships[shipnum2].objnum] ) {
		Int3();			// you are trying to transfer cargo between two ships not docked
		return;
	}

	temp = (Ships[shipnum1].cargo1 & CARGO_INDEX_MASK);
	Ships[shipnum1].cargo1 = char(Ships[shipnum2].cargo1 & CARGO_INDEX_MASK);
	Ships[shipnum2].cargo1 = char(temp);
}

void sexp_cap_waypont_speed(int n)
{
	char *shipname;
	int shipnum;
	int speed;

	shipname = CTEXT(n);
	speed = atoi(CTEXT(CDR(n)));

	shipnum = ship_name_lookup(shipname);

	if (shipnum == -1) {
		Int3();	// trying to set waypoint speed of ship not already in game
		return;
	}

	// cap speed to range (-1, 32767) to store within int
	if (speed < 0) {
		speed = -1;
	}

	if (speed > 32767) {
		speed = 32767;
	}

	Ai_info[Ships[shipnum].ai_index].waypoint_speed_cap = speed;
}

// this function causes a ship to jettison its cargo
void sexp_jettison_cargo( int n )
{
	char *shipname;
	int jettison_delay, ship_index;	

	// get some data
	shipname = CTEXT(n);
	jettison_delay = atoi(CTEXT(CDR(n)));

	// lookup the ship
	ship_index = ship_name_lookup(shipname);
	if(ship_index < 0){
		return;
	}
	
	// jettison cargo
	ship_jettison_cargo(&Ships[ship_index]);
}

void sexp_cargo_no_deplete( int n )
{
	char *shipname;
	int ship_index, no_deplete = 1;

	// get some data
	shipname = CTEXT(n);

	// lookup the ship
	ship_index = ship_name_lookup(shipname);
	if(ship_index < 0){
		return;
	}

	if ( !(Ship_info[Ships[ship_index].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
		Warning(LOCATION, "Trying to make non BIG or HUGE ship %s with non-depletable cargo.\n", Ships[ship_index].ship_name);
		return;
	}

	if (CDR(n) != -1) {
		no_deplete = atoi(CTEXT(CDR(n)));
		Assert((no_deplete == 0) || (no_deplete == 1));
		if ( (no_deplete != 0) && (no_deplete != 1) ) {
			no_deplete = 1;
		}
	}

	if (no_deplete) {
		Ships[ship_index].cargo1 |= CARGO_NO_DEPLETE;
	} else {
		Ships[ship_index].cargo1 &= (~CARGO_NO_DEPLETE);
	}

}

// sexpression to end the mission!  Fixed by EdrickV, implemented by Sesquipedalian
void sexp_end_mission( int n )
{
		// we have a special debriefing screen for multiplayer furballs
	if((Game_mode & GM_MULTIPLAYER) && (The_mission.game_type & MISSION_TYPE_MULTI_DOGFIGHT)){
		gameseq_post_event( GS_EVENT_MULTI_DOGFIGHT_DEBRIEF);
	}
	// do the normal debriefing for all other situations
	else {
		gameseq_post_event(GS_EVENT_DEBRIEF);
	}
}

// funciton to toggle the status bit for the AI code which tells the AI if it is a good time
// to rearm.  The status being set means good time.  Status not being set (unset), means bad time.
// designers must implement this.
void sexp_good_time_to_rearm( int n )
{
	int i, time;
	char *team_name;

	team_name = CTEXT(n);
	time = atoi(CTEXT(CDR(n)));						// this is the time for how long a good rearm is active -- in seconds
	for ( i = 0; i < Num_team_names; i++ ) {
		if ( !stricmp(team_name, Team_names[i]) ) {
			int team;

			team = 1 << i;
			ai_set_rearm_status( team, time );
		}
	}
}

// function which grants promotion to the player
void sexp_grant_promotion()
{
	// short circuit multiplayer for now until we figure out what to do.
	if ( Game_mode & GM_MULTIPLAYER )
		return;

	// set a bit to tell player should get promoted at the end of the mission.  I suppose the other
	// thing that we could do would be to set the players score to at least the amount of
	// points for the next level, but this way is better I think.
	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		Player->flags |= PLAYER_FLAGS_PROMOTED;
	}
}

// function which gives the named medal to the players in the mission
void sexp_grant_medal( int n )
{
	int i, j;
	char *medal_name;

	// don't give medals in normal gameplay when not in campaign mode
	if ( (Game_mode & GM_NORMAL) && !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	Assert(Player->stats.m_medal_earned < 0);  // Mission has problems.  Tried to grant 2 medals in 1 mission.
	medal_name = CTEXT(n);
	for (i = 0; i < NUM_MEDALS; i++ ) {
		if ( !stricmp(medal_name, Medals[i].name) )
			break;
	}

	if ( i < NUM_MEDALS ) {
		Player->stats.m_medal_earned = i;
#ifndef NO_NETWORK
		if ( Game_mode & GM_MULTIPLAYER ) {
			for ( j = 0; j < MAX_PLAYERS; j++ ) {
				if ( MULTI_CONNECTED(Net_players[j]) ) {
					Net_players[j].player->stats.m_medal_earned = i;
				}
			}
		}
#endif
	}
}

void sexp_tech_add_ship(int node)
{
	int i;
	char *name;

	Assert(node >= 0);
	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	while (node >= 0) {
		name = CTEXT(node);
		i = ship_info_lookup(name);
		if (i >= 0)
			Ship_info[i].flags |= SIF_IN_TECH_DATABASE;
		else
			Error(LOCATION, "Ship class \"%s\" invalid", name);

		node = CDR(node);
	}
}

void sexp_tech_add_weapon(int node)
{
	int i;
	char *name;

	Assert(node >= 0);
	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	while (node >= 0) {
		name = CTEXT(node);
		i = weapon_info_lookup(name);
		if (i >= 0)
			Weapon_info[i].wi_flags |= WIF_IN_TECH_DATABASE;
		else
			Error(LOCATION, "Weapon class \"%s\" invalid", name);

		node = CDR(node);
	}
}

void sexp_tech_add_intel(int node)
{
	int i;
	char *name;

	Assert(node >= 0);
	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	while (node >= 0) {
		name = CTEXT(node);
		i = intel_info_lookup(name);
		if (i >= 0)
			Intel_info[i].flags |= IIF_IN_TECH_DATABASE;
		else
			Error(LOCATION, "Intel name \"%s\" invalid", name);

		node = CDR(node);
	}
}

// Goober5000 - reset all the tech entries to their default states
void sexp_tech_reset_to_default()
{
	tech_reset_to_default();
}

// function to set variables needed to grant a new ship/weapon to the player during the course
// of a mission
void sexp_allow_ship( int n )
{
	int sindex;
	char *name;

	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	// get the name of the ship and lookup up the ship_info index for it
	name = CTEXT(n);
	sindex = ship_info_lookup( name );
	if ( sindex == -1 )
		return;

	// now we have a valid index --
	mission_campaign_save_persistent( CAMPAIGN_PERSISTENT_SHIP, sindex );
}

void sexp_allow_weapon( int n )
{
	int sindex;
	char *name;

	// this function doesn't mean anything when not in campaign mode
	if ( !(Game_mode & GM_CAMPAIGN_MODE) )
		return;

	// get the name of the weapon and lookup up the weapon_info index for it
	name = CTEXT(n);
	sindex = weapon_info_lookup( name );
	if ( sindex == -1 )
		return;

	// now we have a valid index --
	mission_campaign_save_persistent( CAMPAIGN_PERSISTENT_WEAPON, sindex );
}

// functions to deal with breaking/fixing the warp engines on ships/wings.  should_break is true when
// we are breaking the warp drive (can be repaired). The parameter is 0 when is getting broken (i.e.
// can be fixed by repair).  The repair parameter tells us whether we are clearing the destroyed or broken
// flag (1), or setting them (0).
void sexp_deal_with_warp( int n, int should_break, int nix )
{
	int index, flag;
	char *name;

	for ( ; n != -1; n = CDR(n) ) {
		name = CTEXT(n);
		index = ship_name_lookup(name);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) )
			continue;

		// we can only operate on ships which are in the mission
		if ( index != -1 ) {

			// set the flag value accoring to whether we are destroying the warp or just breaking it
			if ( should_break )
				flag = SF_WARP_BROKEN;
			else
				flag = SF_WARP_NEVER;

			if ( nix )
				Ships[index].flags |= flag;
			else
				Ships[index].flags &= ~flag;

		} else {
			// maybe this ship has yet to arrive.  Get a possible parse object and set the flag if found
			p_object *pobjp;

			pobjp = mission_parse_get_arrival_ship( name );
#ifndef NDEBUG
			if ( pobjp == NULL ) {
				Int3();				// warning, find allender -- should be impossible
				continue;
			}
#endif
			if ( should_break )
				flag = P_SF_WARP_BROKEN;
			else
				flag = P_SF_WARP_BROKEN;

			if ( nix )
				pobjp->flags |= flag;
			else
				pobjp->flags &= ~flag;

		}
	}
}

// function which is used to tell the AI when it is okay to fire certain secondary
// weapons at other ships.
void sexp_good_secondary_time( int n )
{
	char *team_name, *weapon_name, *ship_name;
	int num_weapons, weapon_index, team, i;

	team_name = CTEXT(n);
	num_weapons = atoi(CTEXT(CDR(n)));
	weapon_name = CTEXT(CDR(CDR(n)));
	ship_name = CTEXT(CDR(CDR(CDR(n))));

	weapon_index = weapon_info_lookup(weapon_name);
	if ( weapon_index == -1 ) {
		nprintf(("Warning", "couldn't find weapon %s for good-secondary-time\n", weapon_name));
		return;
	}

	// get the team type from the team_name
	for ( i = 0; i < Num_team_names; i++ ) {
		if ( !stricmp(Team_names[i], team_name) )
			break;
	}
	if ( i == Num_team_names ) {
		nprintf(("Warning", "couldn't find team %s for good-secondary-time\n", team_name ));
		return;
	}
	team = (1<<i);			// this is the magic formula to get to a team type.

	// see if the ship has departed or has been destroyed.  If so, then we don't need to set up the
	// AI stuff
	if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
		return;

	ai_good_secondary_time( team, weapon_index, num_weapons, ship_name );
}


// function to deal with getting status of goals for previous missions (in the current campaign).
// the status parameter is used to tell this function if we are looking for a goal_satisfied, goal_failed,
// or goal incomplete event
int sexp_previous_goal_status( int n, int status )
{
	char rval = 0, *mission_name;
	char *goal_name;
	int i, mission_num, default_value = 0, use_defaults = 1;

	mission_name = CTEXT(n);
	goal_name = CTEXT(CDR(n));

	// check for possible next optional argument
	n = CDR(CDR(n));
	if ( n != -1 ) {
		default_value = eval_sexp(n);
	}

	// try to find the given mission name in the current list of missions in the campaign.
	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		i = mission_campaign_find_mission( mission_name );

		if ( i == -1 ) {
			// if mission not found, assume that goal was false (so previous-goal-false returns true)
			nprintf(("General", "Couldn't find mission name %s in current campaign's list of missions.\nReturning %s for goal-status function.", mission_name, (status==GOAL_COMPLETE)?"false":"true"));
			if ( status == GOAL_COMPLETE )
				rval = SEXP_KNOWN_FALSE;
			else
				rval = SEXP_KNOWN_TRUE;

			use_defaults = 0;
		} else if (Campaign.missions[i].flags & CMISSION_FLAG_SKIPPED) {
			use_defaults = 1;
		} else {
			// now try and find the goal this mission
			mission_num = i;
			for (i = 0; i < Campaign.missions[mission_num].num_goals; i++) {
				if ( !stricmp(Campaign.missions[mission_num].goals[i].name, goal_name) )
					break;
			}

			if ( i == Campaign.missions[mission_num].num_goals ) {
				Warning(LOCATION, "Couldn't find goal name %s in mission %s.\nReturning %s for goal-true function.", goal_name, mission_name, (status==GOAL_COMPLETE)?"false":"true");
				if ( status == GOAL_COMPLETE )
					rval = SEXP_KNOWN_FALSE;
				else
					rval = SEXP_KNOWN_TRUE;

			} else {
				// now return KNOWN_TRUE or KNOWN_FALSE based on the status field in the goal structure
				if ( Campaign.missions[mission_num].goals[i].status == status )
					rval = SEXP_KNOWN_TRUE;
				else
					rval = SEXP_KNOWN_FALSE;
			}

			use_defaults = 0;
		}
	}

	if (use_defaults) {
		// when not in campaign mode, always return KNOWN_TRUE when looking for goal complete, and KNOWN_FALSE
		// otherwise
		if ( n != -1 ) {
			if ( default_value )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		} else {
			if ( status == GOAL_COMPLETE )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		}
	}

	return rval;
}

// sexpression which gets the status of an event from a previous mission.  Like the above function but
// dealing with events instead of goals.  Again, the status parameter tells the code if we are looking
// for an event_true, event_false, or event_incomplete status
int sexp_previous_event_status( int n, int status )
{
	char rval = 0, *mission_name;
	char *name;
	int i, mission_num, default_value = 0, use_defaults = 1;

	mission_name = CTEXT(n);
	name = CTEXT(CDR(n));

	// check for possible optional parameter
	n = CDR(CDR(n));
	if ( n != -1 ){
		default_value = eval_sexp(n);
	}

	if ( Game_mode & GM_CAMPAIGN_MODE ) {
		// following function returns -1 when mission isn't found.
		i = mission_campaign_find_mission( mission_name );

		// if the mission name wasn't found -- make this return FALSE for the event status.
		if ( i == -1 ) {
			nprintf(("General", "Couldn't find mission name %s in current campaign's list of missions.\nReturning %s for event-status function.", mission_name, (status==EVENT_SATISFIED)?"false":"true"));
			if ( status == EVENT_SATISFIED ) {
				rval = SEXP_KNOWN_FALSE;
			} else {
				rval = SEXP_KNOWN_TRUE;
			}

			use_defaults = 0;
		} else if (Campaign.missions[i].flags & CMISSION_FLAG_SKIPPED) {
			use_defaults = 1;
		} else {
			// now try and find the goal this mission
			mission_num = i;
			for (i = 0; i < Campaign.missions[mission_num].num_events; i++) {
				if ( !stricmp(Campaign.missions[mission_num].events[i].name, name) )
					break;
			}

			if ( i == Campaign.missions[mission_num].num_events ) {
				Warning(LOCATION, "Couldn't find event name %s in mission %s.\nReturning %s for event_status function.", name, mission_name, (status==EVENT_SATISFIED)?"false":"true");
				if ( status == EVENT_SATISFIED )
					rval = SEXP_KNOWN_FALSE;
				else
					rval = SEXP_KNOWN_TRUE;

			} else {
				// now return KNOWN_TRUE or KNOWN_FALSE based on the status field in the goal structure
				if ( Campaign.missions[mission_num].events[i].status == status )
					rval = SEXP_KNOWN_TRUE;
				else
					rval = SEXP_KNOWN_FALSE;
			}

			use_defaults = 0;
		}
	} 

	if (use_defaults) {
		if ( n != -1 ) {
			if ( default_value )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		} else {
			if ( status == EVENT_SATISFIED )
				rval = SEXP_KNOWN_TRUE;
			else
				rval = SEXP_KNOWN_FALSE;
		}
	}

	return rval;
}

// function to return the status of an event in the current mission.  The passed parameter indicates
// if we are checking whether the event is true or the event is false.
int sexp_event_status( int n, int want_true )
{
	char *name;
	int i, result;

	name = CTEXT(n);
	for (i = 0; i < Num_mission_events; i++ ) {
		// look for the event name, check it's status.  If formula is gone, we know the state won't ever change.
		if ( !stricmp(Mission_events[i].name, name) ) {
			result = Mission_events[i].result;
			if (Mission_events[i].formula < 0) {
				if ( (want_true && result) || (!want_true && !result) )
					return SEXP_KNOWN_TRUE;
				else
					return SEXP_KNOWN_FALSE;

			} else {
				if ( (want_true && result) || (!want_true && !result) )
					return SEXP_TRUE;
				else
					return SEXP_FALSE;
			}
		}
	}

	return 0;
}

// function to return the status of an event N seconds after the event is true or false.  Similar
// to above function but waits N seconds before returning true
int sexp_event_delay_status( int n, int want_true )
{
	char *name;
	int i, result;
	fix delay;

	name = CTEXT(n);
	delay = i2f(num_eval(CDR(n)));
	for (i = 0; i < Num_mission_events; i++ ) {
		// look for the event name, check it's status.  If formula is gone, we know the state won't ever change.
		if ( !stricmp(Mission_events[i].name, name) ) {
			if ( (fix) Mission_events[i].timestamp + delay >= Missiontime )
				return SEXP_FALSE;

			result = Mission_events[i].result;
			if (Mission_events[i].formula < 0) {
				if ( (want_true && result) || (!want_true && !result) )
					return SEXP_KNOWN_TRUE;
				else
					return SEXP_KNOWN_FALSE;

			} else {
				if ( want_true && result )  //) || (!want_true && !result) )
					return SEXP_TRUE;
				else
					return SEXP_FALSE;
			}
		}
	}

	return 0;
}

// function which returns true if the given event is still incomplete
int sexp_event_incomplete( int n )
{
	char *name;
	int i;

	name = CTEXT(n);

	for (i = 0; i < Num_mission_events; i++ ) {
		if ( !stricmp(Mission_events[i].name, name ) ) {
			// if the formula is still >= 0 (meaning it is still getting eval'ed), then
			// the event is incomplete
			if ( Mission_events[i].formula != -1 )
				return SEXP_TRUE;
			else
				return SEXP_KNOWN_FALSE;
		}
	}

	return 0;
}

// function to return the status of an goal N seconds after the goal is true or false.  Similar
// to above function but operates on goals instead of events
int sexp_goal_delay_status( int n, int want_true )
{
	char *name;
	fix delay, time;

	name = CTEXT(n);
	delay = i2f(num_eval(CDR(n)));
	
	if ( want_true ) {
		// if we are looking for a goal true entry and we find a false, then return known false here
		if ( mission_log_get_time(LOG_GOAL_FAILED, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_GOAL_SATISFIED, name, NULL, &time) ) {
			if ( (Missiontime - time) >= delay )
				return SEXP_KNOWN_TRUE;
		}
	} else {
		// if we are looking for a goal false entry and we find a true, then return known false here
		if ( mission_log_get_time(LOG_GOAL_SATISFIED, name, NULL, NULL) )
			return SEXP_KNOWN_FALSE;
		else if ( mission_log_get_time(LOG_GOAL_FAILED, name, NULL, &time) ) {
			if ( (Missiontime - time) >= delay )
				return SEXP_KNOWN_TRUE;
		}
	}

	return 0;
}

// function which returns true if the given goal is still incomplete
int sexp_goal_incomplete( int n )
{
	char *name;

	name = CTEXT(n);

	if ( mission_log_get_time( LOG_GOAL_SATISFIED, name, NULL, NULL) || mission_log_get_time( LOG_GOAL_FAILED, name, NULL, NULL) )
		return SEXP_KNOWN_FALSE;
	else
		return SEXP_TRUE;
}

// protects/unprotects a ship.  The flag tells us whether or not the protect bit should be set (flag==1)
// or cleared (flag==0)
void sexp_protect_ships( int n, int flag )
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num >= 0 ) {
			if ( flag )
				Objects[Ships[num].objnum].flags |= OF_PROTECTED;
			else
				Objects[Ships[num].objnum].flags &= ~OF_PROTECTED;
		} else {
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj ) {
				if ( flag )
					parse_obj->flags |= P_OF_PROTECTED;
				else
					parse_obj->flags &= ~P_OF_PROTECTED;

	#ifndef NDEBUG
			} else {
				Int3();	// get allender -- could be a potential problem here
	#endif
			}
		}
	}
}

// protects/unprotects a ship.  The flag tells us whether or not the protect bit should be set (flag==1)
// or cleared (flag==0)
void sexp_beam_protect_ships( int n, int flag )
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num >= 0 ) {
			if ( flag )
				Objects[Ships[num].objnum].flags |= OF_BEAM_PROTECTED;
			else
				Objects[Ships[num].objnum].flags &= ~OF_BEAM_PROTECTED;
		} else {
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj ) {
				if ( flag )
					parse_obj->flags |= P_OF_BEAM_PROTECTED;
				else
					parse_obj->flags &= ~P_OF_BEAM_PROTECTED;

	#ifndef NDEBUG
			} else {
				Int3();	// get allender -- could be a potential problem here
	#endif
			}
		}
	}
}



// sets the "dont collide invisible" flag on a list of ships - Goober5000
void sexp_dont_collide_invisible( int n, int dont_collide )
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) )
	{
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 )
		{
			if ( dont_collide )
			{
				Ships[num].flags2 |= SF2_DONT_COLLIDE_INVIS;
			}
			else
			{
				Ships[num].flags2 &= ~SF2_DONT_COLLIDE_INVIS;
			}

		}
		else
		{
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj )
			{
				if ( dont_collide )
					parse_obj->flags |= P_SF2_DONT_COLLIDE_INVIS;
				else
					parse_obj->flags &= ~P_SF2_DONT_COLLIDE_INVIS;

	#ifndef NDEBUG
			}
			else
			{
				Int3();	// could be a potential problem here
	#endif
			}
		}
	}
}

// sets the vaporize flag on a list of ships - Goober5000
void sexp_ships_vaporize( int n, int vaporize )
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) )
	{
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 )
		{
			if ( vaporize )
			{
				Ships[num].flags |= SF_VAPORIZE;
			}
			else
			{
				Ships[num].flags &= ~SF_VAPORIZE;
			}

		}
		else
		{
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj )
			{
				if ( vaporize )
					parse_obj->flags |= P_SF_VAPORIZE;
				else
					parse_obj->flags &= ~P_SF_VAPORIZE;

	#ifndef NDEBUG
			}
			else
			{
				Int3();	// could be a potential problem here
	#endif
			}
		}
	}
}

// sexpression to make ships "visible" and "invisible" to sensors.  The visible parameter is true
// when making ships visible, false otherwise
void sexp_ships_visible( int n, int visible )
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 ) {
			if ( !visible ) {
				Ships[num].flags |= SF_HIDDEN_FROM_SENSORS;
			} else {
				Ships[num].flags &= ~SF_HIDDEN_FROM_SENSORS;
				if (Ships[num].flags & SF_ESCORT) {
					// SEND add escort request
					hud_add_ship_to_escort(Ships[num].objnum, 1);
				}
			}

		} else {
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj ) {
				if ( !visible )
					parse_obj->flags |= P_SF_HIDDEN_FROM_SENSORS;
				else
					parse_obj->flags &= ~P_SF_HIDDEN_FROM_SENSORS;

	#ifndef NDEBUG
			} else {
				Int3();	// get allender -- could be a potential problem here
	#endif
			}
		}
	}
}

// Goober5000
void sexp_ships_stealthy(int n, int stealthy)
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) )
	{
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 )
		{
			if ( stealthy )
			{
				Ships[num].flags2 |= SF2_STEALTH;
			}
			else
			{
				Ships[num].flags2 &= ~SF2_STEALTH;

				// add to escort list because we became visible
				if (Ships[num].flags & SF_ESCORT)
				{
					// SEND add escort request
					hud_add_ship_to_escort(Ships[num].objnum, 1);
				}
			}
		}
		else
		{
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj )
			{
				if ( stealthy )
				{
					parse_obj->flags |= P_SF2_STEALTH;
				}
				else
				{
					parse_obj->flags &= ~P_SF2_STEALTH;
				}

	#ifndef NDEBUG
			} else {
				Int3();	// could be a potential problem here
	#endif
			}
		}
	}
}

// Goober5000
void sexp_friendly_stealth_invisible(int n, int invisible)
{
	char *ship_name;
	int num;

	for ( ; n != -1; n = CDR(n) )
	{
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 )
		{
			if ( invisible )
			{
				Ships[num].flags2 |= SF2_FRIENDLY_STEALTH_INVIS;
			}
			else
			{
				Ships[num].flags2 &= ~SF2_FRIENDLY_STEALTH_INVIS;

				// add to escort list because we became visible
				if (Ships[num].flags & SF_ESCORT)
				{
					// SEND add escort request
					hud_add_ship_to_escort(Ships[num].objnum, 1);
				}
			}
		}
		else
		{
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj )
			{
				if ( invisible )
				{
					parse_obj->flags |= P_SF2_FRIENDLY_STEALTH_INVIS;
				}
				else
				{
					parse_obj->flags &= ~P_SF2_FRIENDLY_STEALTH_INVIS;
				}

	#ifndef NDEBUG
			} else {
				Int3();	// could be a potential problem here
	#endif
			}
		}
	}
}

// sexpression to toggle invulnerability flag of ships.
void sexp_ships_invulnerable( int n, int invulnerable )
{
	char *ship_name;
	object *objp;
	int num;

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 ) {
			objp = &Objects[Ships[num].objnum];
			if ( invulnerable )
				objp->flags |= OF_INVULNERABLE;
			else
				objp->flags &= ~OF_INVULNERABLE;
		} else {
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj ) {
				if ( invulnerable )
					parse_obj->flags |= P_SF_INVULNERABLE;
				else
					parse_obj->flags &= ~P_SF_INVULNERABLE;

	#ifndef NDEBUG
			} else {
				Int3();	// get allender -- could be a potential problem here
	#endif
			}
		}
	}
}

// sexpression to toggle KEEP ALIVE flag of ship object
void sexp_ships_guardian( int n, int guardian )
{
	char *ship_name;
	object *objp;
	int num;

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 ) {
			objp = &Objects[Ships[num].objnum];
			if ( guardian )
				objp->flags |= OF_GUARDIAN;
			else
				objp->flags &= ~OF_GUARDIAN;
		} else {
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj ) {
				if ( guardian )
					parse_obj->flags |= P_SF_GUARDIAN;
				else
					parse_obj->flags &= ~P_SF_GUARDIAN;

	#ifndef NDEBUG
			} else {
				Int3();	// get allender -- could be a potential problem here
	#endif
			}
		}
	}
}

// make ship vanish without a trace (and what its docked to)
void ship_vanished(int);
void sexp_ship_vanish( int n )
{
	char *ship_name;
	object *objp, *docked_objp;
	int num;

	// if MULTIPLAYER bail
	if (Game_mode & GM_MULTIPLAYER) {
		return;
	}

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 ) {
			objp = &Objects[Ships[num].objnum];

			// check if docked
			docked_objp = ai_find_docked_object( objp );

			// kill
			objp->flags |= OF_SHOULD_BE_DEAD;

			// make vanish
			ship_vanished(num);

			if (docked_objp) {
				// kill
				docked_objp->flags |= OF_SHOULD_BE_DEAD;

				// vanish
				ship_vanished(docked_objp->instance);
			}
		}
	}
}


void sexp_ship_lights_on(int node)	//-WMCoolmon
{
	//Insert sexp here
}

void sexp_ship_lights_off(int node) //-WMCoolmon
{
	//Insert sexp here
}

void sexp_shields_off(int n, int shields_off ) //-Sesquipedalian
{
	char *ship_name;
	object *objp;
	int num;

	for ( ; n != -1; n = CDR(n) ) {
		ship_name = CTEXT(n);

		// check to see if ship destroyed or departed.  In either case, do nothing.
		if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
			continue;

		// get the ship num.  If we get a -1 for the number here, ship has yet to arrive.  Store this ship
		// in a list until created
		num = ship_name_lookup(ship_name);
		if ( num != -1 ) {
			objp = &Objects[Ships[num].objnum];
			if ( shields_off )
				objp->flags |= OF_NO_SHIELDS; 
			else
				objp->flags &= ~OF_NO_SHIELDS; 
		} else {
			p_object *parse_obj;

			parse_obj = mission_parse_get_arrival_ship( ship_name );
			if ( parse_obj ) {
				if ( shields_off )
					parse_obj->flags |= P_OF_NO_SHIELDS;
				else
					parse_obj->flags &= ~P_OF_NO_SHIELDS;

	#ifndef NDEBUG
			} else {
				Int3();	// get allender -- could be a potential problem here
	#endif
			}
		}
	}
}




int sexp_key_pressed(int node)
{
	int z, t;

	Assert(node != -1);
	z = translate_key_to_index(CTEXT(node));
	if (z < 0){
		return 0;
	}

	if (!Control_config[z].used){
		return 0;
	}

	if (CDR(node) < 0){
		return 1;
	}

	t = atoi(CTEXT(CDR(node)));
	return timestamp_has_time_elapsed(Control_config[z].used, t * 1000);
}

void sexp_key_reset(int node)
{
	int z;

	Assert(node != -1);
	z = translate_key_to_index(CTEXT(node));
	if (z >= 0)
		Control_config[z].used = 0;
}

int sexp_targeted(int node)
{
	int z;
	ship_subsys *ptr;

	z = ship_query_state(CTEXT(node));
	if (z == 1){
		return SEXP_KNOWN_FALSE;  // ship isn't around, nor will it ever be
	} else if (z == -1) {
		return SEXP_CANT_EVAL;
	}

	z = ship_name_lookup(CTEXT(node), 1);
	if ((z < 0) || !Player_ai || (Ships[z].objnum != Player_ai->target_objnum)){
		return 0;
	}

	if (CDR(node) >= 0) {
		z = atoi(CTEXT(CDR(node))) * 1000;
		if (!timestamp_has_time_elapsed(Players_target_timestamp, z)){
			return 0;
		}

		if (CDR(CDR(node)) >= 0) {
			ptr = Player_ai->targeted_subsys;
			if (!ptr || stricmp(ptr->system_info->subobj_name, CTEXT(CDR(CDR(node))))){
				return 0;
			}
		}
	}

	return 1;
}

int sexp_speed(int node)
{
	if (Training_context & TRAINING_CONTEXT_SPEED) {
		if (Training_context_speed_set){
			if (timestamp_has_time_elapsed(Training_context_speed_timestamp, atoi(CTEXT(node)) * 1000)){
				return SEXP_KNOWN_TRUE;
			}
		}
	}

	return 0;
}

// Goober5000
int sexp_primaries_depleted(int node)
{
	int sindex, num_banks, num_depleted_banks;
	ship *shipp;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) {
		return 0;
	}

	shipp = &Ships[sindex];
	if (shipp->objnum < 0) {
		return 0;
	}

	// see if ship has ballistic primary weapons   
	if (!(Ship_info[shipp->ship_info_index].flags & SIF_BALLISTIC_PRIMARIES))   
		return 0; 

	// get num primary banks
	num_banks = shipp->weapons.num_primary_banks;
	num_depleted_banks = 0;

	// get number of depleted banks
	for (int idx=0; idx<num_banks; idx++)
	{
		// is this a ballistic bank?
		if (Weapon_info[shipp->weapons.primary_bank_weapons[idx]].wi_flags2 & WIF2_BALLISTIC)
		{
			// is this bank out of ammo?
			if (shipp->weapons.primary_bank_ammo[idx] == 0)
			{
				num_depleted_banks++;
			}
		}
	}

	// are they all depleted?
	return (num_depleted_banks == num_banks);
}

int sexp_secondaries_depleted(int node)
{
	int sindex, num_banks, num_depleted_banks;
	ship *shipp;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if (sindex < 0) {
		return 0;
	}

	shipp = &Ships[sindex];
	if (shipp->objnum < 0) {
		return 0;
	}

	// get num secondary banks
	num_banks = shipp->weapons.num_secondary_banks;
	num_depleted_banks = 0;

	// get number of depleted banks
	for (int idx=0; idx<num_banks; idx++) {
		if (shipp->weapons.secondary_bank_ammo[idx] == 0) {
			num_depleted_banks++;
		}
	}

	// are they all depleted?
	return (num_depleted_banks == num_banks);
}

int sexp_facing(int node)
{
	int obj, sh;
	float a1, a2;
	vector v1, v2;

	if (ship_query_state(CTEXT(node)) < 0){
		return SEXP_KNOWN_FALSE;
	}

	sh = ship_name_lookup(CTEXT(node));
	if ((sh < 0) || !Player_obj){
		return 0;
	}

	obj = Ships[sh].objnum;
	v1 = Player_obj->orient.vec.fvec;
	vm_vec_normalize(&v1);
	vm_vec_sub(&v2, &Objects[obj].pos, &Player_obj->pos);
	vm_vec_normalize(&v2);
	a1 = vm_vec_dotprod(&v1, &v2);
	a2 = (float) cos(ANG_TO_RAD(atof(CTEXT(CDR(node)))));
	if (a1 >= a2){
		return 1;
	}

	return 0;
}

// is ship facing first waypoint in waypoint path
int sexp_facing2(int node)
{
	int i;
	float a1, a2;
	vector v1, v2;

	// bail if Player_obj is not good
	if (!Player_obj) {
		return SEXP_CANT_EVAL;
	}

	// get player fvec
	v1 = Player_obj->orient.vec.fvec;
	vm_vec_normalize(&v1);

	// get waypoint name
	char *waypoint_name = CTEXT(node);

	// get position of first waypoint
	int wp_index = -1;
	for (i=0; i<Num_waypoint_lists; i++) {
		if (!stricmp(waypoint_name, Waypoint_lists[i].name)) {
			wp_index = i;
			break;
		}
	}

	if (wp_index == -1) {
		return SEXP_CANT_EVAL;
	}

	// Waypoint_lists[wp_index].waypoints[0]

	vm_vec_sub(&v2, &Waypoint_lists[wp_index].waypoints[0], &Player_obj->pos);
	vm_vec_normalize(&v2);
	a1 = vm_vec_dotprod(&v1, &v2);
	a2 = (float) cos(ANG_TO_RAD(atof(CTEXT(CDR(node)))));
	if (a1 >= a2){
		return 1;
	}

	return 0;
}

// implemented by Goober5000
int sexp_order(int n)
{
	char *ship_or_wing = CTEXT(n);
	char *order = CTEXT(CDR(n));
	char *target = NULL;

	if (CDR(CDR(n)) != -1)
		target = CTEXT(CDR(CDR(n)));

	return hud_query_order_issued(ship_or_wing, order, target);
}

int sexp_waypoint_missed()
{
	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		if (Training_context_at_waypoint > Training_context_goal_waypoint){
			return 1;
		}
	}

	return 0;
}

int sexp_waypoint_twice()
{
	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		if (Training_context_at_waypoint < Training_context_goal_waypoint - 1){
			return 1;
		}
	}

	return 0;
}

int sexp_path_flown()
{
	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		if (Training_context_goal_waypoint == Waypoint_lists[Training_context_path].count){
			return 1;
		}
	}

	return 0;
}

void sexp_send_training_message(int node)
{
	int t = -1, delay = 0;

	if(physics_paused){
		return;
	}

	Assert(node >= 0);
	Assert(Event_index >= 0);

	if ((CDR(node) >= 0) && (CDR(CDR(node)) >= 0)) {
		delay = atoi(CTEXT(CDR(CDR(node)))) * 1000;
		t = CDR(CDR(CDR(node)));
		if (t >= 0){
			t = atoi(CTEXT(t));
		}
	}

	if ((Mission_events[Event_index].repeat_count > 1) || (CDR(node) < 0)){
		message_training_que(CTEXT(node), timestamp(delay), t);
	} else {
		message_training_que(CTEXT(CDR(node)), timestamp(delay), t);
	}

//	if (Training_msg_method)
//		gameseq_post_event(GS_EVENT_TRAINING_PAUSE);
}

int sexp_shield_recharge_pct(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}

	// shield recharge pct
	return (int)(100.0f * Energy_levels[Ships[sindex].shield_recharge_index]);
}

int sexp_engine_recharge_pct(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}

	// shield recharge pct
	return (int)(100.0f * Energy_levels[Ships[sindex].engine_recharge_index]);
}

int sexp_weapon_recharge_pct(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}

	// shield recharge pct
	return (int)(100.0f * Energy_levels[Ships[sindex].weapon_recharge_index]);
}

int sexp_shield_quad_low(int node)
{
	int sindex, idx;	
	float max_quad, check;
	ship_info *sip;
	object *objp;

	// get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS)){
		return 0;
	}
	if((Ships[sindex].ship_info_index < 0) || (Ships[sindex].ship_info_index >= Num_ship_types)){
		return 0;
	}
	objp = &Objects[Ships[sindex].objnum];
	sip = &Ship_info[Ships[sindex].ship_info_index];
	if(!(sip->flags & SIF_SMALL_SHIP)){
		return 0;
	}
	max_quad = sip->shields / (float)MAX_SHIELD_SECTIONS;	

	// shield pct
	check = (float)atoi(CTEXT(CDR(node)));

	// check his quadrants
	for(idx=0; idx<MAX_SHIELD_SECTIONS; idx++){
		if( ((objp->shields[idx] / max_quad) * 100.0f) <= check ){
			return 1;
		}
	}

	// all good
	return 0;	
}

// Goober5000
int sexp_primary_ammo_pct(int node)
{
	ship *shipp;
	int sindex;
	int check, idx;
	int ret_sum[MAX_SUPPORTED_PRIMARY_BANKS];
	int ret = 0;

	// get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0)
	{
		return 0;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS))
	{
		return 0;
	}
	shipp = &Ships[sindex];
	
	// bank to check
	check = atoi(CTEXT(CDR(node)));

	// bogus check? (MAX_SUPPORTED_PRIMARY_BANKS == cumulative sum of all banks)
	if((check != MAX_SUPPORTED_PRIMARY_BANKS) && (check > shipp->weapons.num_primary_banks)){
		return 0;
	}

	// cumulative sum?
	if(check == MAX_SUPPORTED_PRIMARY_BANKS)
	{
		for(idx=0; idx<shipp->weapons.num_primary_banks; idx++)
		{
			// ballistic
			if (Weapon_info[shipp->weapons.primary_bank_weapons[idx]].wi_flags2 & WIF2_BALLISTIC)
			{
				ret_sum[idx] = (int)(((float)shipp->weapons.primary_bank_ammo[idx] / (float)shipp->weapons.primary_bank_start_ammo[idx]) * 100.0f);
			}
			// non-ballistic
			else
			{
				ret_sum[idx] = 100;
			}
		}

		// add it up
		ret = 0;
		for(idx=0; idx<shipp->weapons.num_primary_banks; idx++)
		{
			ret += ret_sum[idx];
		}
		ret = (int)((float)ret / (float)shipp->weapons.num_primary_banks);
	}
	else
	{
		// ballistic
		if (Weapon_info[shipp->weapons.primary_bank_weapons[check]].wi_flags2 & WIF2_BALLISTIC)
		{
			ret = (int)(((float)shipp->weapons.primary_bank_ammo[check] / (float)shipp->weapons.primary_bank_start_ammo[check]) * 100.0f);
		}
		// non-ballistic
		else
		{
			ret = 100;
		}
	}	

	// return
	return ret;
}

int sexp_secondary_ammo_pct(int node)
{
	ship *shipp;
	int sindex;
	int check, idx;
	int ret_sum[MAX_SUPPORTED_SECONDARY_BANKS];
	int ret = 0;

	// get the ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if((Ships[sindex].objnum < 0) || (Ships[sindex].objnum >= MAX_OBJECTS)){
		return 0;
	}
	shipp = &Ships[sindex];
	
	// bank to check
	check = atoi(CTEXT(CDR(node)));

	// bogus check? (MAX_SUPPORTED_SECONDARY_BANKS == cumulative sum of all banks)
	if((check != MAX_SUPPORTED_SECONDARY_BANKS) && (check > shipp->weapons.num_secondary_banks)){
		return 0;
	}

	// cumulative sum?
	if(check == MAX_SUPPORTED_SECONDARY_BANKS){
		for(idx=0; idx<shipp->weapons.num_secondary_banks; idx++){
			ret_sum[idx] = (int)(((float)shipp->weapons.secondary_bank_ammo[idx] / (float)shipp->weapons.secondary_bank_start_ammo[idx]) * 100.0f);
		}

		// add it up
		ret = 0;
		for(idx=0; idx<shipp->weapons.num_secondary_banks; idx++){
			ret += ret_sum[idx];
		}
		ret = (int)((float)ret / (float)shipp->weapons.num_secondary_banks);
	} else {
		ret = (int)(((float)shipp->weapons.secondary_bank_ammo[check] / (float)shipp->weapons.secondary_bank_start_ammo[check]) * 100.0f);
	}	

	// return
	return ret;
}

// Goober5000
void sexp_change_ship_model(int n)
{
	int ship_num, class_num = ship_info_lookup(CTEXT(n));
	Assert(class_num != -1);

	n = CDR(n);
	// all ships in the sexp
	for ( ; n != -1; n = CDR(n))
	{
		ship_num = ship_name_lookup(CTEXT(n), 1);

		// don't change unless it's currently in the mission
		if (ship_num != -1)
		{
			// don't mess with a ship that's occupied
			if (!(Ships[ship_num].flags & (SF_DYING | SF_ARRIVING | SF_DEPARTING)))
			{
				ship_model_change(ship_num, class_num);
			}
		}
	}
}

// Goober5000
void sexp_change_ship_class(int n)
{
	int ship_num, class_num = ship_info_lookup(CTEXT(n));
	Assert(class_num != -1);

	n = CDR(n);
	// all ships in the sexp
	for ( ; n != -1; n = CDR(n))
	{
		ship_num = ship_name_lookup(CTEXT(n), 1);

		// don't change unless it's currently in the mission
		if (ship_num != -1)
		{
			// don't mess with a ship that's occupied
			if (!(Ships[ship_num].flags & (SF_DYING | SF_ARRIVING | SF_DEPARTING)))
			{
				change_ship_type(ship_num, class_num, 1);
			}
		}
	}
}


//-Bobboau
void sexp_activate_deactivate_glow_points(int n, int activate)
{
	int sindex;
	ship *shipp;

	// every ship specified
	for ( ; n != -1; n = CDR(n))
	{
		sindex = ship_name_lookup(CTEXT(n), 1);

		// ensure ship exists
		if (sindex >= 0)
		{
			shipp = &Ships[sindex];

#ifndef NDEBUG
/*
			if (activate)
				HUD_printf("activating glows on %s", shipp->ship_name);
			else
				HUD_printf("deactivating glows on %s", shipp->ship_name);
*/
#endif

			if (activate)
				shipp->glows_active = GLOW_POINTS_ALL_ON;
			else
				shipp->glows_active = GLOW_POINTS_ALL_OFF;
		}
	}
}

//-Bobboau
void sexp_activate_deactivate_glow_maps(int n, int activate)
{
	int sindex;
	ship *shipp;

	for ( ; n != -1; n = CDR(n))
	{
		sindex = ship_name_lookup(CTEXT(n), 1);
		if (sindex >= 0)
		{
			shipp = &Ships[sindex];

#ifndef NDEBUG
/*
			if (activate)
				HUD_printf("activating glow maps on %s", shipp->ship_name);
			else
				HUD_printf("deactivating glow maps on %s", shipp->ship_name);
*/
#endif

			shipp->glowmaps_active = activate;	
		}
	}
}

//-Bobboau
void sexp_activate_deactivate_glow_point_bank(int n, int activate)
{
	int sindex, i;
	ship *shipp;

	sindex = ship_name_lookup(CTEXT(n), 1);
	if (sindex < 0)
		return;
	shipp = &Ships[sindex];

	n = CDR(n);
	for ( ; n != -1; n = CDR(n))
	{
		i = (int)atoi(CTEXT(n));
		if (i < MAX_GLOW_POINTS)
		{

#ifndef NDEBUG
			if (activate)
				HUD_printf("activating glow %d on %s", i, shipp->ship_name);
			else
				HUD_printf("deactivating glow %d on %s", i, shipp->ship_name);
#endif

			if (activate)
				shipp->glows_active	|= (1 << i);
			else
				shipp->glows_active	&= ~(1 << i);
		}
	}

}

void sexp_beam_fire(int node)
{
	int sindex;
	beam_fire_info fire_info;		
	int idx;	

	// zero stuff out
	memset(&fire_info, 0, sizeof(beam_fire_info));
	fire_info.accuracy = 0.000001f;							// this will guarantee a hit

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}
	fire_info.shooter = &Objects[Ships[sindex].objnum];

	// get the subsystem
	fire_info.turret = ship_get_subsys(&Ships[sindex], CTEXT(CDR(node)));
	if(fire_info.turret == NULL){
		return;
	}

	// get the target
	sindex = ship_name_lookup(CTEXT(CDR(CDR(node))));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}
	fire_info.target = &Objects[Ships[sindex].objnum];

	// see if the optional subsystem can be found	
	fire_info.target_subsys = NULL;
	fire_info.target_subsys = ship_get_subsys(&Ships[sindex], CTEXT(CDR(CDR(CDR(node)))));	

	// if it has no primary weapons
	if(fire_info.turret->weapons.num_primary_banks <= 0){
		Int3();
		return;
	}

	// if the turret is destroyed
	if(fire_info.turret->current_hits <= 0.0f){
		return;
	}

	// hmm, this could be wacky. Let's just simply select the first beam weapon in the turret
	fire_info.beam_info_index = -1;	
	for(idx=0; idx<fire_info.turret->weapons.num_primary_banks; idx++){
		// store the weapon info index
		if(Weapon_info[fire_info.turret->weapons.primary_bank_weapons[idx]].wi_flags & WIF_BEAM){
			fire_info.beam_info_index = fire_info.turret->weapons.primary_bank_weapons[idx];
		}
	}

	// fire the beam
	if(fire_info.beam_info_index != -1){		
		beam_fire(&fire_info);
	} else {
		// it would appear the turret doesn't have any beam weapons, dumbass
		Int3();
	}
}	

void sexp_beam_free(int node)
{
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	while(node != -1){
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			return;
		}

		// flag it as beam free :)
		turret->weapons.flags |= SW_FLAG_BEAM_FREE;
		turret->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));

		// next
		node = CDR(node);
	}
}

void sexp_beam_free_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// free all beam weapons
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as beam free
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags |= SW_FLAG_BEAM_FREE;
			subsys->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_beam_lock(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	while(node != -1){
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			return;
		}

		// flag it as not beam free
		turret->weapons.flags &= ~(SW_FLAG_BEAM_FREE);

		// next
		node = CDR(node);
	}
}

void sexp_beam_lock_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// free all beam weapons
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as not beam free
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags &= ~(SW_FLAG_BEAM_FREE);
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_turret_free(int node)
{
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	while(node != -1){
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			return;
		}

		// flag turret as no longer locked :)
		turret->weapons.flags &= (~SW_FLAG_TURRET_LOCK);
		turret->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));

		// next
		node = CDR(node);
	}
}

void sexp_turret_free_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// free all turrets
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as free
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags &= (~SW_FLAG_TURRET_LOCK);
			subsys->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_turret_lock(int node)
{	
	int sindex;
	ship_subsys *turret = NULL;	

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	node = CDR(node);
	while(node != -1){
		// get the subsystem
		turret = ship_get_subsys(&Ships[sindex], CTEXT(node));
		if(turret == NULL){
			return;
		}

		// flag turret as locked
		turret->weapons.flags |= SW_FLAG_TURRET_LOCK;

		// next
		node = CDR(node);
	}
}

void sexp_turret_lock_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// lcck all turrets
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as locked
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags |= SW_FLAG_TURRET_LOCK;
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_turret_tagged_only_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// mark all turrets to only target tagged ships
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as locked
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags |= SW_FLAG_TAGGED_ONLY;
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_turret_tagged_clear_all(int node)
{
	ship_subsys *subsys;
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// mark all turrets so not restricted to only tagged ships
	subsys = GET_FIRST(&Ships[sindex].subsys_list);
	while(subsys != END_OF_LIST(&Ships[sindex].subsys_list)){
		// just mark all turrets as locked
		if(subsys->system_info->type == SUBSYSTEM_TURRET){
			subsys->weapons.flags &= (~SW_FLAG_TAGGED_ONLY);
		}

		// next item
		subsys = GET_NEXT(subsys);
	}
}

void sexp_add_remove_escort(int node)
{
	int sindex;
	int flag;
	char *whee;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}

	// determine whether to add or remove it
	whee = CTEXT(CDR(node));
	flag = atoi(CTEXT(CDR(node)));

	// add/remove
	if(flag){	
		hud_add_ship_to_escort(Ships[sindex].objnum, 1);
	} else {
		hud_remove_ship_from_escort(Ships[sindex].objnum);
	}
}

//given: two escort priorities and a list of ships
//do:    sets the most damaged one to the first priority and the rest to the second.
void sexp_damage_escort_list(int node)
{
	int n;
	int priority1;		//escort priority to set the most damaged ship
	int priority2;		//""         ""   to set the other ships

	ship* shipp;
	float smallest_hull_pct=1;		//smallest hull pct found
	int small_shipnum=-1;		//index in Ships[] of the above
	float current_hull_pct;			//hull pct of current ship we are evaluating
	int shipnum=-1;				//index in Ships[] of the above

	priority1=atoi(CTEXT(node));
	priority2=atoi(CTEXT(CDR(node)));

	//go to start of ship list
	n=CDR(CDR(node));
	
	//loop through the ships
	for ( ; n != -1; n = CDR(n) )
	{
		shipnum=ship_name_lookup(CTEXT(n));
		
		//it may be dead
		if (shipnum < 0)
			continue;

		if (Ships[shipnum].objnum < 0)
			continue;

		shipp=&Ships[shipnum];

		//calc hull integrity and compare
		current_hull_pct=Objects[shipp->objnum].hull_strength/Ship_info[shipp->ship_info_index].initial_hull_strength;

		if (current_hull_pct < smallest_hull_pct)
		{
			Ships[small_shipnum].escort_priority=priority2;		//give the previous smallest the lower priority
			
			smallest_hull_pct=current_hull_pct;
			small_shipnum=shipnum;
	
			shipp->escort_priority=priority1;					//give the new smallest the higher priority
			hud_add_ship_to_escort(Ships[shipnum].objnum,1);
		}
		else														//if its bigger to begin with give it lower priority
		{
			shipp->escort_priority=priority2;
			hud_add_ship_to_escort(Ships[shipnum].objnum,1);
		}
	}
}

// Goober5000 - set stuff for mission support ship
void sexp_set_support_ship(int n)
{
	int i, temp_val;

	// get arrival location
	temp_val = -1;
	for (i=0; i<MAX_ARRIVAL_NAMES; i++)
	{
		if (!stricmp(CTEXT(n), Arrival_location_names[i]))
			temp_val = i;
	}
	if (temp_val < 0)
	{
		Warning(LOCATION, "Support ship arrival location '%s' not found.\n", CTEXT(n));
		return;
	}
	The_mission.support_ships.arrival_location = temp_val;

	// get arrival anchor
	n = CDR(n);
	temp_val = -1;
	for (i=0; i<Num_parse_names; i++)
	{
		if (!stricmp(CTEXT(n), Parse_names[i]))
			temp_val = i;
	}
	// if not found, make a new entry
	if (temp_val < 0)
	{
		strcpy(Parse_names[Num_parse_names], CTEXT(n));
		temp_val = Num_parse_names;
		Num_parse_names++;
	}
	The_mission.support_ships.arrival_anchor = temp_val;

	// get departure location
	n = CDR(n);
	temp_val = -1;
	for (i=0; i<MAX_DEPARTURE_NAMES; i++)
	{
		if (!stricmp(CTEXT(n), Departure_location_names[i]))
			temp_val = i;
	}
	if (temp_val < 0)
	{
		Warning(LOCATION, "Support ship departure location '%s' not found.\n", CTEXT(n));
		return;
	}
	The_mission.support_ships.departure_location = temp_val;

	// get departure anchor
	n = CDR(n);
	temp_val = -1;
	for (i=0; i<Num_parse_names; i++)
	{
		if (!stricmp(CTEXT(n), Parse_names[i]))
			temp_val = i;
	}
	// if not found, make a new entry
	if (temp_val < 0)
	{
		strcpy(Parse_names[Num_parse_names], CTEXT(n));
		temp_val = Num_parse_names;
		Num_parse_names++;
	}
	The_mission.support_ships.departure_anchor = temp_val;

	// get ship class
	n = CDR(n);
	temp_val = ship_info_lookup(CTEXT(n));
	if ((temp_val < 0) && (stricmp(CTEXT(n), "<any support ship class>")))
	{
		Warning(LOCATION, "Support ship class '%s' not found.\n", CTEXT(n));
		return;
	}
	The_mission.support_ships.ship_class = temp_val;

	// get max number of ships allowed
	n = CDR(n);
	The_mission.support_ships.max_support_ships = atoi(CTEXT(n));
}

// Goober5000
// set *all* the escort priorities of ships in escort list as follows: most damaged ship gets
// first priority in the argument list, next damaged gets next priority, etc.; if there are more
// ships than priorities, all remaining ships get the final priority on the list
// -- As indicated in the argument specification, there must be at least one argument but no more
// than MAX_COMPLETE_ESCORT_LIST arguments
void sexp_damage_escort_list_all(int n)
{
	typedef struct my_escort_ship
	{
		int index;
		float hull;
	} my_escort_ship;

	int priority[MAX_COMPLETE_ESCORT_LIST];
	my_escort_ship escort_ship[MAX_COMPLETE_ESCORT_LIST];
	int i, j, num_escort_ships, num_priorities, temp_i;
	float temp_f;
	ship *shipp;

	// build list of priorities
	num_priorities = 0;
	for ( ; n != 1; n = CDR(n) )
	{
		priority[num_priorities] = atoi(CTEXT(n));
		num_priorities++;
	}

	// build custom list of escort ships
	num_escort_ships = 0;
	for (i = 0; i < MAX_SHIPS; i++)
	{
		shipp = &Ships[i];

		// make sure it exists
		if ( shipp->objnum < 0 )
			continue;

		// make sure it's on the escort list
		if ( !(shipp->flags & SF_ESCORT) )
			continue;

		// set index
		escort_ship[num_escort_ships].index = i;

		// calc and set hull integrity
		escort_ship[num_escort_ships].hull = Objects[shipp->objnum].hull_strength / Ship_info[shipp->ship_info_index].initial_hull_strength;

		num_escort_ships++;
	}

	// sort it bubbly, lowest hull to highest hull
	for (i = 0; i < num_escort_ships; i++)
	{
		for (j = 0; j < i; j++)
		{
			if (escort_ship[i].hull < escort_ship[j].hull)
			{
				// swap
				temp_i = escort_ship[i].index;
				temp_f = escort_ship[i].hull;
				escort_ship[i].index = escort_ship[j].index;
				escort_ship[i].hull = escort_ship[j].hull;
				escort_ship[j].index = temp_i;
				escort_ship[j].hull = temp_f;
			}
		}
	}

	// loop through and assign priorities
	for (i = 0; i < num_escort_ships; i++)
	{
		if (i >= num_priorities)
			Ships[escort_ship[i].index].escort_priority = priority[num_priorities - 1];
		else
			Ships[escort_ship[i].index].escort_priority = priority[i];
	}

	// redo the escort list
	hud_setup_escort_list();
}

void sexp_awacs_set_radius(int node)
{
	int sindex;		
	ship_subsys *awacs;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}	

	// get the awacs subsystem
	awacs = ship_get_subsys(&Ships[sindex], CTEXT(CDR(node)));
	if(awacs == NULL){
		return;
	}

	// make sure this _is_ an awacs subsystem
	Assert(awacs->system_info->flags & MSS_FLAG_AWACS);
	if(awacs->system_info->flags & MSS_FLAG_AWACS){
		return;
	}

	// set the new awacs radius
	awacs->awacs_radius = (float)atoi(CTEXT(CDR(CDR(node))));	
}

// Goober5000
void sexp_primitive_sensors_set_range(int n)
{
	char *ship_name = CTEXT(n);
	int ship_num, range = atoi(CTEXT(CDR(n)));

	// check to see if ship destroyed or departed.  In either case, do nothing.
	if ( mission_log_get_time(LOG_SHIP_DEPART, ship_name, NULL, NULL) || mission_log_get_time(LOG_SHIP_DESTROYED, ship_name, NULL, NULL) )
		return;

	// get the ship - doesn't matter if it hasn't arrived yet
	ship_num = ship_name_lookup_absolute(ship_name);

	// set the new range
	Ships[ship_num].primitive_sensor_range = range;
}

int sexp_is_tagged(int node)
{
	int sindex;

	// get the firing ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	object *caller = &Objects[Ships[sindex].objnum];
	if(ship_is_tagged(caller)) { // This line and the one above were added.
	// if(Ships[sindex].tag_left> 0.0f) The broken code. Changed by EdrickV@HLP

		return 1;
	}

	// not tagged
	return 0;
}

int sexp_num_kills(int node)
{
	int sindex, np_index;
	player *p = NULL;

	// get the ship we're interested in
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	
#ifndef NO_NETWORK
	// in multiplayer, search through all players
	if(Game_mode & GM_MULTIPLAYER){
		// try and find the player
		np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			p = Net_players[np_index].player;
		}
	}
	// if we're in single player, we're only concerned with ourself
	else 
#endif
	{
		// me
		if(Player_obj == &Objects[Ships[sindex].objnum]){
			p = Player;
		}
	}

	// now, if we have a valid player, return his kills
	if(p != NULL){
		return p->stats.m_kill_count_ok;
	}

	// bad
	return 0;
}

int sexp_num_type_kills(int node)
{
	int sindex, np_index, st_index;
	int idx, total;
	player *p = NULL;

	// get the ship we're interested in
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	
#ifndef NO_NETWORK
	// in multiplayer, search through all players
	if(Game_mode & GM_MULTIPLAYER){
		// try and find the player
		np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			p = Net_players[np_index].player;
		}
	}
	// if we're in single player, we're only concerned with ourself
	else
#endif
	{
		// me
		if(Player_obj == &Objects[Ships[sindex].objnum]){
			p = Player;
		}
	}

	// bad
	if(p == NULL){
		return 0;
	}

	// lookup ship type name
	st_index = ship_type_name_lookup(CTEXT(CDR(node)));
	if(st_index < 0){
		return 0;
	}

	// look stuff up	
	total = 0;
	for(idx=0; idx<Num_ship_types; idx++){
		if((p->stats.m_okKills[idx] > 0) && (Ship_info[idx].flags & Ship_type_flags[st_index])){
			total += p->stats.m_okKills[idx];
		}
	}

	// total
	return total;
}

int sexp_num_class_kills(int node)
{
	int sindex, np_index, si_index;
	player *p = NULL;

	// get the ship we're interested in
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	
#ifndef NO_NETWORK
	// in multiplayer, search through all players
	if(Game_mode & GM_MULTIPLAYER){
		// try and find the player
		np_index = multi_find_player_by_object(&Objects[Ships[sindex].objnum]);
		if((np_index >= 0) && (np_index < MAX_PLAYERS)){
			p = Net_players[np_index].player;
		}
	}
	// if we're in single player, we're only concerned with ourself
	else
#endif
	{
		// me
		if(Player_obj == &Objects[Ships[sindex].objnum]){
			p = Player;
		}
	}

	// bad
	if(p == NULL){
		return 0;
	}

	// get the ship type we're looking for
	si_index = ship_info_lookup(CTEXT(CDR(node)));
	if((si_index < 0) || (si_index > Num_ship_types)){
		return 0;
	}

	// return the count	
	return p->stats.m_okKills[si_index];
}

void sexp_subsys_set_random(int node)
{
	int sindex, low, high, n, idx, rand, exclusion_list[MAX_MODEL_SUBSYSTEMS];		
	ship_subsys *subsys;
	ship *shipp;

	// get ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return;
	}
	if(Ships[sindex].objnum < 0){
		return;
	}
	shipp = &Ships[sindex];

	// get low
	low = num_eval(CDR(node));
	if (low < 0) {
		low = 0;
	}

	// get high
	high = num_eval(CDR(CDR(node)));
	if (high > 100) {
		high = 100;
	}

	if (low > high) {
		Int3();
		return;
	}

	n = CDR(CDR(CDR(node)));

	// init exclusion list
	memset(exclusion_list, 0, sizeof(int) * Ship_info[shipp->ship_info_index].n_subsystems);

	// get exclusion list
	while( n != -1) {
		int exclude_index = ship_get_subsys_index(shipp, CTEXT(n), 0);
		if (exclude_index >= 0) {
			exclusion_list[exclude_index] = 1;
		}

		n = CDR(n);
	}

	// apply to all others
	for (idx=0; idx<Ship_info[shipp->ship_info_index].n_subsystems; idx++) {
		if ( exclusion_list[idx] == 0 ) {
			// get non excluded subsystem
			subsys = ship_get_indexed_subsys(shipp, idx, NULL);

			// randomize its hit points
			rand = rand_internal(low, high);
			subsys->current_hits = 0.01f * rand * subsys->system_info->max_hits;
		}
	}
}

void sexp_supernova_start(int node)
{
	supernova_start(atoi(CTEXT(node)));
}

int sexp_is_secondary_selected(int node)
{
	int sindex;
	int bank;
	ship *shipp;

	// lookup ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	shipp = &Ships[sindex];

	// bogus value?
	bank = atoi(CTEXT(CDR(node)));
	if(bank >= shipp->weapons.num_secondary_banks){
		return 0;
	}

	// is this the bank currently selected
	if(bank == shipp->weapons.current_secondary_bank){
		return 1;
	}

	// nope
	return 0;
}

int sexp_is_primary_selected(int node)
{
	int sindex;
	int bank;
	ship *shipp;

	// lookup ship
	sindex = ship_name_lookup(CTEXT(node));
	if(sindex < 0){
		return 0;
	}
	if(Ships[sindex].objnum < 0){
		return 0;
	}
	shipp = &Ships[sindex];

	// bogus value?
	bank = atoi(CTEXT(CDR(node)));
	if(bank >= shipp->weapons.num_primary_banks){
		return 0;
	}

	// is this the bank currently selected
	if(bank == shipp->weapons.current_primary_bank){
		return 1;
	}

	// nope
	return 0;
}

#define	RIGHT_QUAD	0
#define	FRONT_QUAD	1
#define	LEFT_QUAD	3
#define	REAR_QUAD	2

//	Return SEXP_TRUE if quadrant quadnum is near max.
int shield_quad_near_max(int quadnum)
{
	float	remaining = 0.0f;
	for (int i=0; i<MAX_SHIELD_SECTIONS; i++) {
		if (i == quadnum){
			continue;
		}
		remaining += Player_obj->shields[i];
	}

	if ((remaining < 2.0f) || (Player_obj->shields[quadnum] > Ship_info[Player_ship->ship_info_index].shields/MAX_SHIELD_SECTIONS - 5.0f)) {
		return SEXP_TRUE;
	} else {
		return SEXP_FALSE;
	}
}

//	Return truth value for special SEXP.
//	Used in training#5, perhaps in other missions.
int process_special_sexps(int index)
{
	switch (index) {
	case 0:	//	Ship "Freighter 1" is aspect locked by player.
		if (Player_ai->target_objnum != -1) {
			if (!(stricmp(Ships[Objects[Player_ai->target_objnum].instance].ship_name, "Freighter 1"))) {
				if (Player_ai->current_target_is_locked)
					return SEXP_TRUE;
			}
		}
		return SEXP_FALSE;
		break;
	case 1:	//	Fired Interceptors
		object	*objp;
		for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (objp->type == OBJ_WEAPON) {
				if (!stricmp(Weapon_info[Weapons[objp->instance].weapon_info_index].name, "Interceptor#weak")) {
					int target = Weapons[objp->instance].target_num;
					if (target != -1) {
						if (Objects[target].type == OBJ_SHIP) {
							if (!(stricmp(Ships[Objects[target].instance].ship_name, "Freighter 1")))
								return SEXP_TRUE;
						}
					}
				}
			}
		}
		return SEXP_FALSE;
	case 2:	//	Ship "Freighter 1", subsystem "Weapons" is aspect locked by player.
		if (Player_ai->target_objnum != -1) {
			if (!(stricmp(Ships[Objects[Player_ai->target_objnum].instance].ship_name, "Freighter 1"))) {
				if (!(stricmp(Player_ai->targeted_subsys->system_info->name, "Weapons"))) {
					if (Player_ai->current_target_is_locked){
						return SEXP_TRUE;
					}
				}
			}
		}
		return SEXP_FALSE;
		break;
	case 3:	//	Player ship suffering shield damage on front.
		apply_damage_to_shield(Player_obj, FRONT_QUAD, 10.0f);
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		return SEXP_TRUE;
		break;
	case 4:	//	Player ship suffering much damage.
		nprintf(("AI", "Frame %i\n", Framecount));
		apply_damage_to_shield(Player_obj, FRONT_QUAD, 10.0f);
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		if (Player_obj->shields[FRONT_QUAD] < 2.0f)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;
	case 5:	//	Player's shield is quick repaired
		nprintf(("AI", "Frame %i, recharged to %7.3f\n", Framecount, Player_obj->shields[FRONT_QUAD]));

		apply_damage_to_shield(Player_obj, FRONT_QUAD, -flFrametime*200.0f);

		if (Player_obj->shields[FRONT_QUAD] > Ship_info[Player_ship->ship_info_index].shields/4.0f)
			Player_obj->shields[FRONT_QUAD] = Ship_info[Player_ship->ship_info_index].shields/4.0f;

		//hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		if (Player_obj->shields[FRONT_QUAD] > Player_obj->shields[(FRONT_QUAD+1)%4] - 2.0f)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;
	case 6:	//	3 of player's shield quadrants are reduced to 0.
		Player_obj->shields[1] = 1.0f;
		Player_obj->shields[2] = 1.0f;
		Player_obj->shields[3] = 1.0f;
		//apply_damage_to_shield(Player_obj, FRONT_QUAD, 1.0f);
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		return SEXP_TRUE;
	case 7:	//	Make sure front quadrant has been maximized, or close to it.
		if (shield_quad_near_max(FRONT_QUAD)) return SEXP_TRUE; else return SEXP_FALSE;
		break;

	case 8:	//	Make sure rear quadrant has been maximized, or close to it.
		if (shield_quad_near_max(REAR_QUAD)) return SEXP_TRUE; else return SEXP_FALSE;
		break;
	
	case 9:	//	Zero left and right quadrants in preparation for maximizing rear quadrant.
		Player_obj->shields[LEFT_QUAD] = 0.0f;
		Player_obj->shields[RIGHT_QUAD] = 0.0f;
		hud_shield_quadrant_hit(Player_obj, LEFT_QUAD);
		return SEXP_TRUE;
		break;

	case 10:	//	Return true if player is low on Interceptors.
		if (Player_ship->weapons.secondary_bank_ammo[0] + Player_ship->weapons.secondary_bank_ammo[1] < 8)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 11:	//	Return true if player has plenty of Interceptors.
		if (Player_ship->weapons.secondary_bank_ammo[0] + Player_ship->weapons.secondary_bank_ammo[1] >= 8)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 12:	//	Return true if player is low on Interceptors.
		if (Player_ship->weapons.secondary_bank_ammo[0] + Player_ship->weapons.secondary_bank_ammo[1] < 4)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;
		break;

	case 13:	// Zero front shield quadrant.  Added for Jim Boone on August 26, 1999 by MK.
		Player_obj->shields[FRONT_QUAD] = 0.0f;
		hud_shield_quadrant_hit(Player_obj, FRONT_QUAD);
		return SEXP_TRUE;
		break;

	case 100:	//	Return true if player is out of countermeasures.
		if (Player_ship->cmeasure_count <= 0)
			return SEXP_TRUE;
		else
			return SEXP_FALSE;

	default:
		Int3();	//	Unsupported node type.
	}

	return SEXP_FALSE;
}

// custom sexp operator for handling misc training stuff
int sexp_special_training_check(int node)
{
	int num, rtn;

	num = atoi(CTEXT(node));
	if (num == SPECIAL_CHECK_TRAINING_FAILURE)
		return Training_failure ? SEXP_TRUE : SEXP_FALSE;

	// To MK: do whatever you want with this number here.
	rtn = process_special_sexps(atoi(CTEXT(node)));

	return rtn;
}

// sexpression to flash a hud gauge.  gauge name is text valud of node
void sexp_flash_hud_gauge( int node )
{
	char *name;
	int i;

	name = CTEXT(node);
	for (i = 0; i < NUM_HUD_GAUGES; i++ ) {
		if ( !stricmp(HUD_gauge_text[i], name) ) {
			hud_gauge_start_flash(i);	// call HUD function to flash gauge
			break;
		}
	}
}

void sexp_set_training_context_fly_path(int node)
{
	int i;

	for (i=0; i<Num_waypoint_lists; i++)
		if (!stricmp(CTEXT(node), Waypoint_lists[i].name))
			break;

	if (i < Num_waypoint_lists) {
		Training_context |= TRAINING_CONTEXT_FLY_PATH;
		Training_context_path = i;
		Training_context_distance = (float) atof(CTEXT(CDR(node)));
		Training_context_goal_waypoint = 0;
		Training_context_at_waypoint = -1;
	}
}

void sexp_set_training_context_speed(int node)
{
	Training_context |= TRAINING_CONTEXT_SPEED;
	Training_context_speed_min = atoi(CTEXT(node));
	Training_context_speed_max = atoi(CTEXT(CDR(node)));
	Training_context_speed_set = 0;
}

// high-level sexpression evaluator
int eval_sexp(int cur_node)
{
	int node, type, sexp_val = UNINITIALIZED;
	if (cur_node == -1)  // empty list, i.e. sexp: ( )
		return FALSE;

	Assert(cur_node >= 0);			// we have special sexp nodes <= -1!!!  MWA
									// which should be intercepted before we get here.  HOFFOSS
	type = SEXP_NODE_TYPE(cur_node);
	Assert( (type == SEXP_LIST) || (type == SEXP_ATOM) );

	// trap known true and known false sexpressions.  We don't trap on SEXP_NAN sexpressions since
	// they may yet evaluate to true or false.

	if (Sexp_nodes[cur_node].value == SEXP_KNOWN_TRUE)
		return 1;
	else if (Sexp_nodes[cur_node].value == SEXP_KNOWN_FALSE)
		return 0;

	if (Sexp_nodes[cur_node].first != -1) {
		node = CAR(cur_node);
		sexp_val = eval_sexp(node);
		Sexp_nodes[cur_node].value = Sexp_nodes[node].value;	// higher level node gets node value
		return sexp_val;

	} else {
		int op_num;

		node = CDR(cur_node);		// makes reading the next bit of code a little easier.

		op_num = find_operator(CTEXT(cur_node)); 
		switch ( op_num ) {
		// arithmetic operators will always return just their value
			case OP_PLUS:
				sexp_val = add_sexps( node );
				break;

			case OP_MINUS:
				sexp_val = sub_sexps( node );
				break;

			case OP_MUL:
				sexp_val = mul_sexps( node );
				break;

			case OP_MOD:
				sexp_val = mod_sexps( node );
				break;

			case OP_DIV:
				sexp_val = div_sexps( node );
				break;

			case OP_RAND:
				sexp_val = rand_sexp( node );
				break;

		// boolean operators can have one of the special sexp values (known true, known false, unknown)
			case OP_TRUE:
				sexp_val = SEXP_KNOWN_TRUE;
				break;

			case OP_FALSE:
				sexp_val = SEXP_KNOWN_FALSE;
				break;

			case OP_OR:
				sexp_val = sexp_or( node );
				break;

			case OP_AND:
				sexp_val = sexp_and( node );
				break;

			case OP_AND_IN_SEQUENCE:
				sexp_val = sexp_and_in_sequence( node );
				break;

			case OP_GREATER_THAN:
				sexp_val = sexp_gt( node );
				break;
				
			case OP_LESS_THAN:
				sexp_val = sexp_lt( node );
				break;

			case OP_EQUALS:
				sexp_val = sexp_equal( node );
				break;

			case OP_IS_IFF:
				sexp_val = sexp_is_iff( node );
				break;

			case OP_NOT:
				sexp_val = sexp_not( node );
				break;

			case OP_PREVIOUS_GOAL_TRUE:
				sexp_val = sexp_previous_goal_status( node, GOAL_COMPLETE );
				break;

			case OP_PREVIOUS_GOAL_FALSE:
				sexp_val = sexp_previous_goal_status( node, GOAL_FAILED );
				break;

			case OP_PREVIOUS_GOAL_INCOMPLETE:
				sexp_val = sexp_previous_goal_status( node, GOAL_INCOMPLETE );
				break;

			case OP_PREVIOUS_EVENT_TRUE:
				sexp_val = sexp_previous_event_status( node, EVENT_SATISFIED );
				break;

			case OP_PREVIOUS_EVENT_FALSE:
				sexp_val = sexp_previous_event_status( node, EVENT_FAILED );
				break;

			case OP_PREVIOUS_EVENT_INCOMPLETE:
				sexp_val = sexp_previous_event_status( node, EVENT_INCOMPLETE );
				break;

			case OP_EVENT_TRUE:
			case OP_EVENT_FALSE:
				sexp_val = sexp_event_status( node, (op_num == OP_EVENT_TRUE?1:0) );
				if ((sexp_val != SEXP_TRUE) && (sexp_val != SEXP_KNOWN_TRUE))
					Sexp_useful_number = 0;  // indicate sexp isn't current yet

				break;

			case OP_EVENT_TRUE_DELAY:
			case OP_EVENT_FALSE_DELAY:
				sexp_val = sexp_event_delay_status( node, (op_num == OP_EVENT_TRUE_DELAY?1:0) );
				if ((sexp_val != SEXP_TRUE) && (sexp_val != SEXP_KNOWN_TRUE))
					Sexp_useful_number = 0;  // indicate sexp isn't current yet
				break;

			case OP_GOAL_TRUE_DELAY:
			case OP_GOAL_FALSE_DELAY:
				sexp_val = sexp_goal_delay_status( node, (op_num == OP_GOAL_TRUE_DELAY?1:0) );
				break;

			case OP_EVENT_INCOMPLETE:
				sexp_val = sexp_event_incomplete( node );
				if ((sexp_val != SEXP_TRUE) && (sexp_val != SEXP_KNOWN_TRUE))
					Sexp_useful_number = 0;  // indicate sexp isn't current yet
				break;

			case OP_GOAL_INCOMPLETE:
				sexp_val = sexp_goal_incomplete( node );
				break;

			// destroy type sexpressions
			case OP_IS_DESTROYED:
				sexp_val = sexp_is_destroyed( node, NULL );
				break;

			case OP_IS_SUBSYSTEM_DESTROYED:
				sexp_val = sexp_is_subsystem_destroyed( node );
				break;

			case OP_HAS_DOCKED:
				sexp_val = sexp_has_docked( node );
				break;

			case OP_HAS_ARRIVED:
				sexp_val = sexp_has_arrived( node, NULL );
				break;

			case OP_HAS_DEPARTED:
				sexp_val = sexp_has_departed( node, NULL );
				break;

			case OP_HAS_UNDOCKED:
				sexp_val = sexp_has_undocked( node );
				break;

			case OP_IS_DISABLED:
				sexp_val = sexp_is_disabled( node, NULL );
				break;

			case OP_IS_DISARMED:
				sexp_val = sexp_is_disarmed( node, NULL );
				break;

			case OP_WAYPOINTS_DONE:
				sexp_val = sexp_are_waypoints_done( node );
				break;

			// objective operators that use a delay
			case OP_IS_DESTROYED_DELAY:
				sexp_val = sexp_is_destroyed_delay( node );
				break;

			case OP_IS_SUBSYSTEM_DESTROYED_DELAY:
				sexp_val = sexp_is_subsystem_destroyed_delay( node );
				break;

			case OP_HAS_DOCKED_DELAY:
				sexp_val = sexp_has_docked_delay( node );
				break;

			case OP_HAS_ARRIVED_DELAY:
				sexp_val = sexp_has_arrived_delay( node );
				break;

			case OP_HAS_DEPARTED_DELAY:
				sexp_val = sexp_has_departed_delay( node );
				break;

			case OP_HAS_UNDOCKED_DELAY:
				sexp_val = sexp_has_undocked_delay( node );
				break;

			case OP_IS_DISABLED_DELAY:
				sexp_val = sexp_is_disabled_delay( node );
				break;

			case OP_IS_DISARMED_DELAY:
				sexp_val = sexp_is_disarmed_delay( node );
				break;

			case OP_WAYPOINTS_DONE_DELAY:
				sexp_val = sexp_are_waypoints_done_delay( node );
				break;

			case OP_SHIP_TYPE_DESTROYED:
				sexp_val = sexp_ship_type_destroyed( node );
				break;

			// time based sexpressions
			case OP_HAS_TIME_ELAPSED:
				sexp_val = sexp_has_time_elapsed( node );
				break;

			case OP_MODIFY_VARIABLE:
				sexp_modify_variable( node );
				sexp_val = 1;	// 1 means only do once.
				break;

			case OP_TIME_SHIP_DESTROYED:
				sexp_val = sexp_time_destroyed( node );
				break;
			
			case OP_TIME_WING_DESTROYED:
				sexp_val = sexp_time_wing_destroyed( node );
				break;
			
			case OP_TIME_SHIP_ARRIVED:
				sexp_val = sexp_time_ship_arrived( node );
				break;
			
			case OP_TIME_WING_ARRIVED:
				sexp_val = sexp_time_wing_arrived( node );
				break;
			
			case OP_TIME_SHIP_DEPARTED:
				sexp_val = sexp_time_ship_departed( node );
				break;
			
			case OP_TIME_WING_DEPARTED:
				sexp_val = sexp_time_wing_departed( node );
				break;

			case OP_MISSION_TIME:
				sexp_val = sexp_mission_time();
				break;

			case OP_TIME_DOCKED:
				sexp_val = sexp_time_docked( node );
				break;

			case OP_TIME_UNDOCKED:
				sexp_val = sexp_time_undocked( node );
				break;

			// info based sexpressions (like shields, hits
			case OP_SHIELDS_LEFT:
				sexp_val = sexp_shields_left( node );
				break;

			case OP_HITS_LEFT:
				sexp_val = sexp_hits_left( node );
				break;

			case OP_HITS_LEFT_SUBSYSTEM:
				sexp_val = sexp_hits_left_subsystem( node );
				break;

			case OP_SPECIAL_WARP_DISTANCE:
				sexp_val = sexp_special_warp_dist( node );
				break;

			case OP_DISTANCE:
				sexp_val = sexp_distance( node );
				break;

			case OP_DISTANCE_SUBSYSTEM:
				sexp_val = sexp_distance_subsystem( node );
				break;

			case OP_IS_SHIP_VISIBLE:
				sexp_val = sexp_is_ship_visible( node );
				break;

			case OP_IS_SHIP_STEALTHY:
				sexp_val = sexp_is_ship_stealthy( node );
				break;

			case OP_IS_FRIENDLY_STEALTH_VISIBLE:
				sexp_val = sexp_is_friendly_stealth_visible( node );
				break;

			case OP_TEAM_SCORE:
				sexp_val = sexp_team_score( node );
				break;

			case OP_LAST_ORDER_TIME:
				sexp_val = sexp_last_order_time( node );
				break;

			case OP_NUM_PLAYERS:
				sexp_val = sexp_num_players();
				break;

			case OP_SKILL_LEVEL_AT_LEAST:
				sexp_val = sexp_skill_level_at_least( node );
				break;

			case OP_IS_CARGO_KNOWN:
			case OP_CARGO_KNOWN_DELAY:
				sexp_val = sexp_is_cargo_known( node, (op_num==OP_IS_CARGO_KNOWN)?0:1 );
				break;

			case OP_HAS_BEEN_TAGGED_DELAY:
				sexp_val = sexp_has_been_tagged_delay(node);
				break;

			case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
				sexp_val = sexp_cap_subsys_cargo_known_delay(node);
				break;

			case OP_WAS_PROMOTION_GRANTED:
				sexp_val = sexp_was_promotion_granted(node);
				break;

			case OP_WAS_MEDAL_GRANTED:
				sexp_val = sexp_was_medal_granted(node);
				break;

			case OP_PERCENT_SHIPS_DEPARTED:
			case OP_PERCENT_SHIPS_DESTROYED:
				sexp_val = sexp_percent_ships_depart_destroy(node, op_num);
				break;

			case OP_DEPART_NODE_DELAY:
				sexp_val = sexp_depart_node_delay( node );
				break;

			case OP_DESTROYED_DEPARTED_DELAY:
				sexp_val = sexp_destroyed_departed_delay( node );
				break;

			// conditional sexpressions
			case OP_WHEN:
				sexp_val = eval_when( node );
				break;

			case OP_COND:
				sexp_val = eval_cond( node );
				break;

			// sexpressions with side effects
			case OP_CHANGE_IFF:
				sexp_change_iff( node );
				sexp_val = 1;
				break;

			case OP_ADD_SHIP_GOAL:
				sexp_add_ship_goal( node );
				sexp_val = 1;
				break;

			case OP_ADD_WING_GOAL:
				sexp_add_wing_goal( node );
				sexp_val = 1;
				break;

			case OP_ADD_GOAL:
				sexp_add_goal( node );
				sexp_val = 1;
				break;

			case OP_CLEAR_SHIP_GOALS:
				sexp_clear_ship_goals( node );
				sexp_val = 1;
				break;

			case OP_CLEAR_WING_GOALS:
				sexp_clear_wing_goals( node );
				sexp_val = 1;
				break;

			case OP_CLEAR_GOALS:
				sexp_clear_goals( node );
				sexp_val = 1;
				break;

			case OP_PROTECT_SHIP:
			case OP_UNPROTECT_SHIP:
				sexp_protect_ships( node, (op_num==OP_PROTECT_SHIP?1:0) );
				sexp_val = 1;
				break;

			case OP_BEAM_PROTECT_SHIP:
			case OP_BEAM_UNPROTECT_SHIP:
				sexp_beam_protect_ships( node, (op_num==OP_BEAM_PROTECT_SHIP?1:0) );
				sexp_val = 1;
				break;

			case OP_SHIP_STEALTHY:
			case OP_SHIP_UNSTEALTHY:
				sexp_ships_stealthy( node, (op_num == OP_SHIP_STEALTHY) );
				sexp_val = 1;
				break;

			case OP_FRIENDLY_STEALTH_INVISIBLE:
			case OP_FRIENDLY_STEALTH_VISIBLE:
				sexp_friendly_stealth_invisible( node, (op_num == OP_FRIENDLY_STEALTH_INVISIBLE) );
				sexp_val = 1;
				break;

			case OP_SHIP_INVISIBLE:
			case OP_SHIP_VISIBLE:
				sexp_ships_visible( node, (op_num==OP_SHIP_VISIBLE?1:0) );
				sexp_val = 1;
				break;

			case OP_SHIP_VULNERABLE:
			case OP_SHIP_INVULNERABLE:
				sexp_ships_invulnerable( node, (op_num==OP_SHIP_INVULNERABLE?1:0) );
				sexp_val = 1;
				break;

			case OP_SHIP_GUARDIAN:
			case OP_SHIP_NO_GUARDIAN:
				sexp_ships_guardian( node, (op_num==OP_SHIP_GUARDIAN?1:0) );
				sexp_val = 1;
				break;

			case OP_SHIP_VANISH:
				sexp_ship_vanish( node );
				sexp_val = 1;
				break;

			case OP_SHIP_LIGHTS_ON: //-WMCoolmon
				sexp_ship_lights_on(node);
				sexp_val = 1;
				break;

			case OP_SHIP_LIGHTS_OFF: //-WMCoolmon
				sexp_ship_lights_off(node);
				sexp_val = 1;
				break;

			//-Sesquipedalian
			case OP_SHIELDS_ON: 
			case OP_SHIELDS_OFF:
				sexp_shields_off( node, (op_num==OP_SHIELDS_OFF?1:0) );
				sexp_val = 1;
				break;

			case OP_SET_CARGO:
				sexp_set_cargo(node);
				sexp_val = 1;
				break;

			case OP_IS_CARGO:
				sexp_val = sexp_is_cargo(node);
				break;

			case OP_CHANGE_AI_CLASS:
				sexp_change_ai_class(node);
				sexp_val = 1;
				break;

			case OP_IS_AI_CLASS:
				sexp_val = sexp_is_ai_class(node);
				break;
				
			case OP_CHANGE_SOUNDTRACK:
				sexp_change_music(node);
				sexp_val = 1;
				break;

			case OP_EXPLOSION_EFFECT:
				sexp_explosion_effect(node);
				sexp_val = 1;
				break;

			case OP_WARP_EFFECT:
				sexp_warp_effect(node);
				sexp_val = 1;
				break;

			case OP_EMP_EFFECT:
				sexp_emp_effect(node);
				sexp_val = 1;
				break;

			case OP_SEND_MESSAGE:
				sexp_send_message( node );
				sexp_val = 1;
				break;

			case OP_SEND_MESSAGE_LIST:
				sexp_send_message_list( node );
				sexp_val = 1;
				break;

			case OP_SEND_RANDOM_MESSAGE:
				sexp_send_random_message( node );
				sexp_val = 1;
				break;

			case OP_SELF_DESTRUCT:
				sexp_self_destruct( node );
				sexp_val = 1;
				break;

			case OP_NEXT_MISSION:
				sexp_next_mission( node );
				sexp_val = 1;
				break;
				
			case OP_END_OF_CAMPAIGN:
				sexp_end_of_campaign( node );
				sexp_val = 1;
				break;

			case OP_END_CAMPAIGN:
				sexp_end_campaign( node );
				sexp_val = 1;
				break;

			case OP_SABOTAGE_SUBSYSTEM:
				sexp_sabotage_subsystem( node );
				sexp_val = 1;
				break;

			case OP_REPAIR_SUBSYSTEM:
				sexp_repair_subsystem( node );
				sexp_val = 1;
				break;

			case OP_SET_SUBSYSTEM_STRNGTH:
				sexp_set_subsystem_strength( node );
				sexp_val = 1;
				break;

			case OP_INVALIDATE_GOAL:
			case OP_VALIDATE_GOAL:
				sexp_change_goal_validity( node, (op_num==OP_INVALIDATE_GOAL?0:1) );
				sexp_val = 1;
				break;

			case OP_TRANSFER_CARGO:
				sexp_transfer_cargo( node );
				sexp_val = 1;
				break;

			case OP_EXCHANGE_CARGO:
				sexp_exchange_cargo( node );
				sexp_val = 1;
				break;


			case OP_JETTISON_CARGO:
				sexp_jettison_cargo( node );
				sexp_val = 1;
				break;

			case OP_CARGO_NO_DEPLETE:
					sexp_cargo_no_deplete( node );
					sexp_val = 1;
					break;

			case OP_SET_SCANNED:	// Goober5000
			case OP_SET_UNSCANNED:
				sexp_set_scanned_unscanned(node, op_num == OP_SET_SCANNED);
				sexp_val = 1;
				break;

			case OP_SET_SPECIAL_WARPOUT_NAME:
				sexp_special_warpout_name( node );
				sexp_val = 1;
				break;

			//-Sesquipedalian
			case OP_END_MISSION: 
                        sexp_end_mission( node );
                        sexp_val = 1;
				break;

				// sexpressions for setting flag for good/bad time for someone to reasm
			case OP_GOOD_REARM_TIME:
				sexp_good_time_to_rearm( node );
				sexp_val = 1;
				break;

			case OP_GRANT_PROMOTION:
				sexp_grant_promotion();
				sexp_val = 1;
				break;

			case OP_GRANT_MEDAL:
				sexp_grant_medal( node );
				sexp_val = 1;
				break;

			case OP_SHIP_VAPORIZE:
			case OP_SHIP_NO_VAPORIZE:
				sexp_ships_vaporize( node, (op_num == OP_SHIP_VAPORIZE) );
				sexp_val = 1;
				break;

			case OP_DONT_COLLIDE_INVISIBLE:
			case OP_COLLIDE_INVISIBLE:
				sexp_dont_collide_invisible( node, (op_num == OP_DONT_COLLIDE_INVISIBLE) );
				sexp_val = 1;
				break;

			case OP_WARP_BROKEN:
			case OP_WARP_NOT_BROKEN:
				sexp_deal_with_warp( node, 0, op_num==OP_WARP_BROKEN?1:0 );
				sexp_val = 1;
				break;

			case OP_WARP_NEVER:
			case OP_WARP_ALLOWED:
				sexp_deal_with_warp( node, 1, op_num==OP_WARP_NEVER?1:0 );
				sexp_val = 1;
				break;

			case OP_GOOD_SECONDARY_TIME:
				sexp_good_secondary_time( node );
				sexp_val = 1;
				break;

			// sexpressions to allow shpis/weapons during the course of a mission
			case OP_ALLOW_SHIP:
				sexp_allow_ship( node );
				sexp_val = 1;
				break;

			case OP_ALLOW_WEAPON:
				sexp_allow_weapon( node );
				sexp_val = 1;
				break;

			case OP_TECH_ADD_SHIP:
				sexp_tech_add_ship(node);
				sexp_val = 1;
				break;

			case OP_TECH_ADD_WEAPON:
				sexp_tech_add_weapon(node);
				sexp_val = 1;
				break;

			case OP_TECH_ADD_INTEL:
				sexp_tech_add_intel(node);
				sexp_val = 1;
				break;

			case OP_TECH_RESET_TO_DEFAULT:
				sexp_tech_reset_to_default();
				sexp_val = 1;
				break;

				// in the case of a red_alert mission, simply call the red alert function to close
				// the current campaign's mission and move forward to the next mission
			case OP_RED_ALERT:
				red_alert_start_mission();
				sexp_val = 1;
				break;

			case OP_GET_OBJECT_X:
			case OP_GET_OBJECT_Y:
			case OP_GET_OBJECT_Z:
				sexp_val = sexp_get_object_coordinates(node, (op_num==OP_GET_OBJECT_X)?0:((op_num==OP_GET_OBJECT_Y)?1:2));
				break;

			case OP_SET_SHIP_POSITION:
				sexp_set_ship_position(node);
				sexp_val = 1;
				break;

			case OP_SET_SHIP_FACING:
				sexp_set_ship_facing(node);
				sexp_val = 1;
				break;

			case OP_SET_SHIP_FACING_OBJECT:
				sexp_set_ship_facing_object(node);
				sexp_val = 1;
				break;

			// training operators
			case OP_KEY_PRESSED:
				sexp_val = sexp_key_pressed(node);
				break;

			case OP_SPECIAL_CHECK:
				sexp_val = sexp_special_training_check(node);
				break;

			case OP_KEY_RESET:
				sexp_key_reset(node);
				sexp_val = SEXP_KNOWN_TRUE;  // only do it first time in repeating events.
				break;

			case OP_TARGETED:
				sexp_val = sexp_targeted(node);
				break;

			case OP_SPEED:
				sexp_val = sexp_speed(node);
				break;

			case OP_PRIMARIES_DEPLETED:
				sexp_val = sexp_primaries_depleted(node);
				break;

			case OP_SECONDARIES_DEPLETED:
				sexp_val = sexp_secondaries_depleted(node);
				break;

			case OP_FACING:
				sexp_val = sexp_facing(node);
				break;

			case OP_FACING2:
				sexp_val = sexp_facing2(node);
				break;

			case OP_ORDER:
				sexp_val = sexp_order(node);
				break;

			case OP_WAYPOINT_MISSED:
				sexp_val = sexp_waypoint_missed();
				break;

			case OP_WAYPOINT_TWICE:
				sexp_val = sexp_waypoint_twice();
				break;

			case OP_PATH_FLOWN:
				sexp_val = sexp_path_flown();
				break;

			case OP_TRAINING_MSG:
				sexp_send_training_message(node);
				sexp_val = 1;
				break;

			case OP_FLASH_HUD_GAUGE:
				sexp_flash_hud_gauge(node);
				sexp_val = 1;
				break;

			case OP_SET_TRAINING_CONTEXT_FLY_PATH:
				sexp_set_training_context_fly_path(node);
				sexp_val = 1;
				break;

			case OP_SET_TRAINING_CONTEXT_SPEED:
				sexp_set_training_context_speed(node);
				sexp_val = 1;
				break;

			// debugging operators
			case OP_INT3:
				Int3();
				sexp_val = 0;
				break;

			case 0: // zero represents a non-operator
				return atoi(CTEXT(cur_node));

			case OP_NOP:
				sexp_val = 1;
				break;

			case OP_BEAM_FIRE:
				sexp_beam_fire(node);
				sexp_val = 1;
				break;

			case OP_IS_TAGGED:
				sexp_val = sexp_is_tagged(node);
				break;

			case OP_NUM_KILLS:
				sexp_val = sexp_num_kills(node);
				break;

			case OP_NUM_TYPE_KILLS:
				sexp_val = sexp_num_type_kills(node);
				break;

			case OP_NUM_CLASS_KILLS:
				sexp_val = sexp_num_class_kills(node);
				break;

			case OP_BEAM_FREE:
				sexp_val = 1;
				sexp_beam_free(node);
				break;

			case OP_BEAM_FREE_ALL:
				sexp_val = 1;
				sexp_beam_free_all(node);
				break;

			case OP_BEAM_LOCK:
				sexp_val = 1;
				sexp_beam_lock(node);
				break;

			case OP_BEAM_LOCK_ALL:
				sexp_val = 1;
				sexp_beam_lock_all(node);
				break;

			case OP_TURRET_FREE:
				sexp_val = 1;
				sexp_turret_free(node);
				break;

			case OP_TURRET_FREE_ALL:
				sexp_val = 1;
				sexp_turret_free_all(node);
				break;

			case OP_TURRET_LOCK:
				sexp_val = 1;
				sexp_turret_lock(node);
				break;

			case OP_TURRET_LOCK_ALL:
				sexp_val = 1;
				sexp_turret_lock_all(node);
				break;

			case OP_ADD_REMOVE_ESCORT:
				sexp_val = 1;
				sexp_add_remove_escort(node);
				break;
			
			case OP_DAMAGED_ESCORT_LIST:
				sexp_val = 1;
				sexp_damage_escort_list(node);
				break;

			case OP_DAMAGED_ESCORT_LIST_ALL:
				sexp_val = 1;
				sexp_damage_escort_list_all(node);
				break;

			case OP_AWACS_SET_RADIUS:
				sexp_val = 1;
				sexp_awacs_set_radius(node);
				break;

			case OP_PRIMITIVE_SENSORS_SET_RANGE:
				sexp_primitive_sensors_set_range(node);
				sexp_val = 1;
				break;

			case OP_CAP_WAYPOINT_SPEED:
				sexp_val = 1;
				sexp_cap_waypont_speed(node);
				break;

			case OP_TURRET_TAGGED_ONLY_ALL:
				sexp_val = 1;
				sexp_turret_tagged_only_all(node);
				break;

			case OP_TURRET_TAGGED_CLEAR_ALL:
				sexp_val = 1;
				sexp_turret_tagged_clear_all(node);
				break;

			case OP_SUBSYS_SET_RANDOM:
				sexp_val = 1;
				sexp_subsys_set_random(node);
				break;

			case OP_SUPERNOVA_START:
				sexp_val = 1;
				sexp_supernova_start(node);
				break;

			case OP_SHIELD_RECHARGE_PCT:
				sexp_val = sexp_shield_recharge_pct(node);
				break;

			case OP_ENGINE_RECHARGE_PCT:
				sexp_val = sexp_engine_recharge_pct(node);
				break;

			case OP_WEAPON_RECHARGE_PCT:
				sexp_val = sexp_weapon_recharge_pct(node);
				break;

			case OP_SHIELD_QUAD_LOW:
				sexp_val = sexp_shield_quad_low(node);
				break;

			case OP_PRIMARY_AMMO_PCT:
				sexp_val = sexp_primary_ammo_pct(node);
				break;

			case OP_SECONDARY_AMMO_PCT:
				sexp_val = sexp_secondary_ammo_pct(node);
				break;

			case OP_IS_SECONDARY_SELECTED:
				sexp_val = sexp_is_secondary_selected(node);
				break;

			case OP_IS_PRIMARY_SELECTED:
				sexp_val = sexp_is_primary_selected(node);
				break;

			// Goober5000
			case OP_SET_SUPPORT_SHIP:
				sexp_set_support_ship(node);
				sexp_val = 1;
				break;

			// Goober5000
			case OP_CHANGE_SHIP_MODEL:
				sexp_change_ship_model(node);
				sexp_val = 1;
				break;

			// Goober5000
			case OP_CHANGE_SHIP_CLASS:
				sexp_change_ship_class(node);
				sexp_val = 1;
				break;

			//-Bobboau
			case OP_ACTIVATE_GLOW_POINTS:
			case OP_DEACTIVATE_GLOW_POINTS:
				sexp_activate_deactivate_glow_points(node, op_num == OP_ACTIVATE_GLOW_POINTS);
				sexp_val = 1;
				break;

			//-Bobboau
			case OP_ACTIVATE_GLOW_MAPS:
			case OP_DEACTIVATE_GLOW_MAPS:
				sexp_activate_deactivate_glow_maps(node, op_num == OP_ACTIVATE_GLOW_MAPS);
				sexp_val = 1;
				break;

			//-Bobboau
			case OP_ACTIVATE_GLOW_POINT_BANK:
			case OP_DEACTIVATE_GLOW_POINT_BANK:
				sexp_activate_deactivate_glow_point_bank(node, op_num == OP_ACTIVATE_GLOW_POINT_BANK);
				sexp_val = 1;
				break;

			default:
				Error(LOCATION, "Looking for SEXP operator, found '%s'.\n", CTEXT(cur_node));
				break;
		}

		Assert(sexp_val != UNINITIALIZED);

		// if we haven't returned, check the sexp value of the sexpression evaluation.  A special
		// value of known true or known false means that we should set the sexp.value field for
		// short circuit eval (and return that special value as well).
		if (sexp_val == SEXP_KNOWN_TRUE) {
			Sexp_nodes[cur_node].value = SEXP_KNOWN_TRUE;
			return 1;
		}

		if (sexp_val == SEXP_KNOWN_FALSE) {
			Sexp_nodes[cur_node].value = SEXP_KNOWN_FALSE;
			return 0;
		}

		if ( sexp_val == SEXP_NAN ) {
			Sexp_nodes[cur_node].value = SEXP_NAN;			// not a number values are false I would suspect
			return 0;
		}

		if ( sexp_val == SEXP_NAN_FOREVER ) {
			Sexp_nodes[cur_node].value = SEXP_NAN_FOREVER;
			return sexp_val;
		}

		if ( sexp_val == SEXP_CANT_EVAL ) {
			Sexp_nodes[cur_node].value = SEXP_CANT_EVAL;
			Sexp_useful_number = 0;  // indicate sexp isn't current yet
			return 0;
		}

		if ( Sexp_nodes[cur_node].value == SEXP_NAN ) {	// if we had a nan, but now don't, reset the value
			Sexp_nodes[cur_node].value = SEXP_UNKNOWN;
			return sexp_val;
		}

		if ( sexp_val ){
			Sexp_nodes[cur_node].value = SEXP_TRUE;
		} else {
			Sexp_nodes[cur_node].value = SEXP_FALSE;
		}

		return sexp_val;
	}
}

//	Still a debug-level system.
//	get_sexp_main reads and builds the internal representation for a
//	symbolic expression.
//	On entry:
//		Mp points at first character in expression.
//	The symbolic expression is built in Sexp_nodes beginning at node 0.
int get_sexp_main()
{
	int	start_node, op;
	char	token[TOKEN_LENGTH];
	char  *savep, ch;

	ignore_white_space();

	savep = Mp;
	if (!strncmp(Mp, "( )", 3))
		savep++;

	Assert(*Mp == '(');
	Mp++;
	start_node = get_sexp(token);
	// only need to check syntax if we have a operator
	if ( /*Sexp_nodes[start_node].subtype != SEXP_ATOM_OPERATOR  ||*/ Fred_running || (start_node == -1))
		return start_node;

	ch = *Mp;
	*Mp = '\0';

	op = identify_operator(CTEXT(start_node));
	if (op == -1)
		Error (LOCATION, "Can't find operator %s in operator list\n.", CTEXT(start_node) );

	*Mp = ch;

	return start_node;
}

void test_sexps()
{
	Mp = Mission_text;
	while (*Mp != '#') {
		get_sexp_main();
		diag_printf("\n----------------\n");
		ignore_white_space();
	}
	exit(0);
}

// returns the data type returned by an operator
int query_operator_return_type(int op)
{
	if (op < FIRST_OP)
	{
		Assert(op >= 0 && op < Num_operators);
		op = Operators[op].value;
	}

	switch (op)
	{
		case OP_TRUE:
		case OP_FALSE:
		case OP_AND:
		case OP_AND_IN_SEQUENCE:
		case OP_OR:
		case OP_NOT:
		case OP_EQUALS:
		case OP_GREATER_THAN:
		case OP_LESS_THAN:
		case OP_IS_DESTROYED:
		case OP_IS_SUBSYSTEM_DESTROYED:
		case OP_IS_DISABLED:
		case OP_IS_DISARMED:
		case OP_HAS_DOCKED:
		case OP_HAS_UNDOCKED:
		case OP_HAS_ARRIVED:
		case OP_HAS_DEPARTED:
		case OP_IS_DESTROYED_DELAY:
		case OP_IS_SUBSYSTEM_DESTROYED_DELAY:
		case OP_IS_DISABLED_DELAY:
		case OP_IS_DISARMED_DELAY:
		case OP_HAS_DOCKED_DELAY:
		case OP_HAS_UNDOCKED_DELAY:
		case OP_HAS_ARRIVED_DELAY:
		case OP_HAS_DEPARTED_DELAY:
		case OP_IS_IFF:
		case OP_IS_AI_CLASS:
		case OP_HAS_TIME_ELAPSED:
		case OP_GOAL_INCOMPLETE:
		case OP_GOAL_TRUE_DELAY:
		case OP_GOAL_FALSE_DELAY:
		case OP_EVENT_INCOMPLETE:
		case OP_EVENT_TRUE_DELAY:
		case OP_EVENT_FALSE_DELAY:
		case OP_PREVIOUS_EVENT_TRUE:
		case OP_PREVIOUS_EVENT_FALSE:
		case OP_PREVIOUS_EVENT_INCOMPLETE:
		case OP_PREVIOUS_GOAL_TRUE:
		case OP_PREVIOUS_GOAL_FALSE:
		case OP_PREVIOUS_GOAL_INCOMPLETE:
		case OP_WAYPOINTS_DONE:
		case OP_WAYPOINTS_DONE_DELAY:
		case OP_SHIP_TYPE_DESTROYED:
		case OP_LAST_ORDER_TIME:
		case OP_KEY_PRESSED:
		case OP_TARGETED:
		case OP_SPEED:
		case OP_FACING:
		case OP_FACING2:
		case OP_ORDER:
		case OP_WAYPOINT_MISSED:
		case OP_WAYPOINT_TWICE:
		case OP_PATH_FLOWN:
		case OP_EVENT_TRUE:
		case OP_EVENT_FALSE:
		case OP_SKILL_LEVEL_AT_LEAST:
		case OP_IS_CARGO_KNOWN:
		case OP_HAS_BEEN_TAGGED_DELAY:
		case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
		case OP_CARGO_KNOWN_DELAY:
		case OP_WAS_PROMOTION_GRANTED:
		case OP_WAS_MEDAL_GRANTED:
		case OP_PERCENT_SHIPS_DEPARTED:
		case OP_PERCENT_SHIPS_DESTROYED:
		case OP_DEPART_NODE_DELAY:
		case OP_DESTROYED_DEPARTED_DELAY:
		case OP_SPECIAL_CHECK:
		case OP_IS_TAGGED:
		case OP_PRIMARIES_DEPLETED:
		case OP_SECONDARIES_DEPLETED:
		case OP_SHIELD_QUAD_LOW:
		case OP_IS_SECONDARY_SELECTED:
		case OP_IS_PRIMARY_SELECTED:
		case OP_IS_SHIP_STEALTHY:
		case OP_IS_FRIENDLY_STEALTH_VISIBLE:
		case OP_IS_CARGO:
			return OPR_BOOL;

		case OP_PLUS:
		case OP_MINUS:
		case OP_MOD:
		case OP_MUL:
		case OP_DIV:
		case OP_RAND:
		case OP_TIME_SHIP_DESTROYED:
		case OP_TIME_SHIP_ARRIVED:
		case OP_TIME_SHIP_DEPARTED:
		case OP_TIME_WING_DESTROYED:
		case OP_TIME_WING_ARRIVED:
		case OP_TIME_WING_DEPARTED:
		case OP_MISSION_TIME:
		case OP_TIME_DOCKED:
		case OP_TIME_UNDOCKED:
		case OP_SHIELDS_LEFT:
		case OP_HITS_LEFT:
		case OP_HITS_LEFT_SUBSYSTEM:
		case OP_DISTANCE:
		case OP_DISTANCE_SUBSYSTEM:
		case OP_NUM_PLAYERS:
		case OP_NUM_KILLS:
		case OP_NUM_TYPE_KILLS:
		case OP_NUM_CLASS_KILLS:
		case OP_SHIELD_RECHARGE_PCT:
		case OP_ENGINE_RECHARGE_PCT:
		case OP_WEAPON_RECHARGE_PCT:
		case OP_PRIMARY_AMMO_PCT:
		case OP_SECONDARY_AMMO_PCT:
		case OP_SPECIAL_WARP_DISTANCE:
		case OP_IS_SHIP_VISIBLE:
		case OP_TEAM_SCORE:
			return OPR_POSITIVE;

		case OP_COND:
		case OP_WHEN:
		case OP_CHANGE_IFF:
		case OP_CHANGE_AI_CLASS:
		case OP_CLEAR_SHIP_GOALS:
		case OP_CLEAR_WING_GOALS:
		case OP_CLEAR_GOALS:
		case OP_ADD_SHIP_GOAL:
		case OP_ADD_WING_GOAL:
		case OP_ADD_GOAL:
		case OP_PROTECT_SHIP:
		case OP_UNPROTECT_SHIP:
		case OP_BEAM_PROTECT_SHIP:
		case OP_BEAM_UNPROTECT_SHIP:
		case OP_INT3:
		case OP_NOP:
		case OP_GOALS_ID:
		case OP_SEND_MESSAGE:
		case OP_SELF_DESTRUCT:
		case OP_NEXT_MISSION:
		case OP_END_CAMPAIGN:
		case OP_END_OF_CAMPAIGN:
		case OP_SABOTAGE_SUBSYSTEM:
		case OP_REPAIR_SUBSYSTEM:
		case OP_INVALIDATE_GOAL:
		case OP_VALIDATE_GOAL:
		case OP_SEND_RANDOM_MESSAGE:
		case OP_TRANSFER_CARGO:
		case OP_EXCHANGE_CARGO:
		case OP_SET_CARGO:
		case OP_JETTISON_CARGO:
		case OP_CARGO_NO_DEPLETE:
		case OP_SET_SCANNED:
		case OP_SET_UNSCANNED:
		case OP_KEY_RESET:
		case OP_TRAINING_MSG:
		case OP_SET_TRAINING_CONTEXT_FLY_PATH:
		case OP_SET_TRAINING_CONTEXT_SPEED:
		case OP_END_MISSION:
		case OP_SET_SUBSYSTEM_STRNGTH:
		case OP_GOOD_REARM_TIME:
		case OP_GRANT_PROMOTION:
		case OP_GRANT_MEDAL:
		case OP_ALLOW_SHIP:
		case OP_ALLOW_WEAPON:
		case OP_TECH_ADD_SHIP:
		case OP_TECH_ADD_WEAPON:
		case OP_TECH_ADD_INTEL:
		case OP_TECH_RESET_TO_DEFAULT:
		case OP_WARP_BROKEN:
		case OP_WARP_NOT_BROKEN:
		case OP_WARP_NEVER:
		case OP_WARP_ALLOWED:
		case OP_FLASH_HUD_GAUGE:
		case OP_GOOD_SECONDARY_TIME:
		case OP_SHIP_VISIBLE:
		case OP_SHIP_INVISIBLE:
		case OP_SHIP_VULNERABLE:
		case OP_SHIP_INVULNERABLE:
		case OP_SHIP_GUARDIAN:
		case OP_SHIP_VANISH:
		case OP_SHIP_LIGHTS_ON:
		case OP_SHIP_LIGHTS_OFF:
		case OP_SHIP_NO_GUARDIAN:
		case OP_SHIELDS_ON:
		case OP_SHIELDS_OFF:
		case OP_SHIP_STEALTHY:
		case OP_SHIP_UNSTEALTHY:
		case OP_FRIENDLY_STEALTH_INVISIBLE:
		case OP_FRIENDLY_STEALTH_VISIBLE:
		case OP_RED_ALERT:
		case OP_MODIFY_VARIABLE:
		case OP_BEAM_FIRE:
		case OP_BEAM_FREE:
		case OP_BEAM_FREE_ALL:
		case OP_BEAM_LOCK:
		case OP_BEAM_LOCK_ALL:
		case OP_TURRET_FREE:
		case OP_TURRET_FREE_ALL:
		case OP_TURRET_LOCK:
		case OP_TURRET_LOCK_ALL:
		case OP_ADD_REMOVE_ESCORT:
		case OP_DAMAGED_ESCORT_LIST:
		case OP_DAMAGED_ESCORT_LIST_ALL:
		case OP_AWACS_SET_RADIUS:
		case OP_PRIMITIVE_SENSORS_SET_RANGE:
		case OP_SEND_MESSAGE_LIST:
		case OP_CAP_WAYPOINT_SPEED:
		case OP_TURRET_TAGGED_ONLY_ALL:
		case OP_TURRET_TAGGED_CLEAR_ALL:
		case OP_SUBSYS_SET_RANDOM:
		case OP_SUPERNOVA_START:
		case OP_SET_SPECIAL_WARPOUT_NAME:
		case OP_SHIP_VAPORIZE:
		case OP_SHIP_NO_VAPORIZE:
		case OP_DONT_COLLIDE_INVISIBLE:
		case OP_COLLIDE_INVISIBLE:
		case OP_CHANGE_SHIP_MODEL:
		case OP_CHANGE_SHIP_CLASS:
		case OP_DEACTIVATE_GLOW_POINTS:
		case OP_ACTIVATE_GLOW_POINTS:
		case OP_DEACTIVATE_GLOW_MAPS:
		case OP_ACTIVATE_GLOW_MAPS:
		case OP_DEACTIVATE_GLOW_POINT_BANK:
		case OP_ACTIVATE_GLOW_POINT_BANK:
		case OP_SET_SUPPORT_SHIP:
		case OP_CHANGE_SOUNDTRACK:
		case OP_EXPLOSION_EFFECT:
		case OP_WARP_EFFECT:
		case OP_EMP_EFFECT:
		case OP_SET_SHIP_POSITION:
		case OP_SET_SHIP_FACING:
		case OP_SET_SHIP_FACING_OBJECT:
			return OPR_NULL;

		case OP_GET_OBJECT_X:
		case OP_GET_OBJECT_Y:
		case OP_GET_OBJECT_Z:
			return OPR_NUMBER;

		case OP_AI_CHASE:
		case OP_AI_DOCK:
		case OP_AI_UNDOCK:
		case OP_AI_WARP:						// this particular operator is obsolete
		case OP_AI_WARP_OUT:
		case OP_AI_WAYPOINTS:
		case OP_AI_WAYPOINTS_ONCE:
		case OP_AI_DESTROY_SUBSYS:
		case OP_AI_CHASE_WING:
		case OP_AI_DISABLE_SHIP:
		case OP_AI_DISARM_SHIP:
		case OP_AI_GUARD:
		case OP_AI_GUARD_WING:
		case OP_AI_CHASE_ANY:
		case OP_AI_CHASE_ANY_EXCEPT:
		case OP_AI_EVADE_SHIP:
		case OP_AI_STAY_NEAR_SHIP:
		case OP_AI_KEEP_SAFE_DISTANCE:
		case OP_AI_IGNORE:
		case OP_AI_STAY_STILL:
		case OP_AI_PLAY_DEAD:
			return OPR_AI_GOAL;

		default:
			Int3();
	}

	return 0;
}

// returns the data type of a specified argument to an operator.  Argnum is 0 indexed.
int query_operator_argument_type(int op, int argnum)
{
	int index = op;

	if (op < FIRST_OP)
	{
		Assert(index >= 0 && index < Num_operators);
		op = Operators[index].value;

	} else {
		for (index=0; index<Num_operators; index++)
			if (Operators[index].value == op)
				break;

		Assert(index < Num_operators);
	}

	if (argnum >= Operators[index].max)
		return OPF_NONE;

	switch (op) {
		case OP_TRUE:
		case OP_FALSE:
		case OP_MISSION_TIME:
		case OP_INT3:
		case OP_NOP:
		case OP_WAYPOINT_MISSED:
		case OP_WAYPOINT_TWICE:
		case OP_PATH_FLOWN:
		case OP_GRANT_PROMOTION:
		case OP_WAS_PROMOTION_GRANTED:
		case OP_RED_ALERT:
		case OP_END_MISSION:
			return OPF_NONE;

		case OP_AND:
		case OP_AND_IN_SEQUENCE:
		case OP_OR:
		case OP_NOT:
			return OPF_BOOL;

		case OP_PLUS:
		case OP_MINUS:
		case OP_MOD:
		case OP_MUL:
		case OP_DIV:
		case OP_EQUALS:
		case OP_GREATER_THAN:
		case OP_LESS_THAN:
		case OP_RAND:
		case OP_HAS_TIME_ELAPSED:
		case OP_SPEED:
		case OP_SET_TRAINING_CONTEXT_SPEED:
		case OP_SPECIAL_CHECK:
		case OP_AI_WARP_OUT:
		case OP_TEAM_SCORE:
			return OPF_POSITIVE;

		case OP_AI_WARP:								// this operator is obsolete
		case OP_SET_TRAINING_CONTEXT_FLY_PATH:
			if ( !argnum )
				return OPF_WAYPOINT_PATH;
			else
				return OPF_NUMBER;
		
		case OP_AI_WAYPOINTS:
		case OP_AI_WAYPOINTS_ONCE:
			if ( argnum == 0 )
				return OPF_WAYPOINT_PATH;
			else
				return OPF_POSITIVE;

		case OP_IS_DISABLED:
		case OP_IS_DISARMED:
		case OP_TIME_SHIP_DESTROYED:
		case OP_TIME_SHIP_ARRIVED:
		case OP_TIME_SHIP_DEPARTED:
		case OP_SHIELDS_LEFT:
		case OP_HITS_LEFT:
		case OP_CLEAR_SHIP_GOALS:
		case OP_PROTECT_SHIP:
		case OP_UNPROTECT_SHIP:
		case OP_BEAM_PROTECT_SHIP:
		case OP_BEAM_UNPROTECT_SHIP:
		case OP_TRANSFER_CARGO:
		case OP_EXCHANGE_CARGO:
		case OP_SHIP_INVISIBLE:
		case OP_SHIP_VISIBLE:
		case OP_SHIP_INVULNERABLE:
		case OP_SHIP_VULNERABLE:
		case OP_SHIP_GUARDIAN:
		case OP_SHIP_NO_GUARDIAN:
		case OP_SHIP_VANISH:
		case OP_SHIP_LIGHTS_ON:
		case OP_SHIP_LIGHTS_OFF:
		case OP_SHIELDS_ON:
		case OP_SHIELDS_OFF:
		case OP_SHIP_STEALTHY:
		case OP_SHIP_UNSTEALTHY:
		case OP_FRIENDLY_STEALTH_INVISIBLE:
		case OP_FRIENDLY_STEALTH_VISIBLE:
		case OP_PRIMARIES_DEPLETED:
		case OP_SECONDARIES_DEPLETED:
		case OP_SPECIAL_WARP_DISTANCE:
		case OP_SET_SPECIAL_WARPOUT_NAME:
		case OP_IS_SHIP_VISIBLE:
		case OP_IS_SHIP_STEALTHY:
		case OP_IS_FRIENDLY_STEALTH_VISIBLE:
			return OPF_SHIP;

		case OP_IS_DESTROYED:
		case OP_HAS_ARRIVED:
		case OP_HAS_DEPARTED:
		case OP_CLEAR_GOALS:
			return OPF_SHIP_WING;

		case OP_IS_DISABLED_DELAY:
		case OP_IS_DISARMED_DELAY:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP;

		case OP_FACING:
			if (argnum == 0)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_FACING2:
			if (argnum == 0) {
				return OPF_WAYPOINT_PATH;
			} else {
				return OPF_POSITIVE;
			}

		case OP_ORDER:
			if (argnum == 1)
				return OPF_AI_ORDER;
			else
				return OPF_SHIP_WING;	// arg 0 or 2

		case OP_IS_DESTROYED_DELAY:
		case OP_HAS_ARRIVED_DELAY:
		case OP_HAS_DEPARTED_DELAY:
		case OP_LAST_ORDER_TIME:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP_WING;

		case OP_DISTANCE:
			return OPF_SHIP_WING_POINT;

		case OP_GET_OBJECT_X:
		case OP_GET_OBJECT_Y:
		case OP_GET_OBJECT_Z:
			if (argnum==0)
				return OPF_SHIP_WING_POINT;
			else
				return OPF_SUBSYSTEM;

		case OP_SET_SHIP_POSITION:
			if (argnum==0)
				return OPF_SHIP;
			else
				return OPF_NUMBER;

		case OP_SET_SHIP_FACING:
			return OPF_NUMBER;

		case OP_SET_SHIP_FACING_OBJECT:
			return OPF_SHIP_WING_POINT;

		case OP_MODIFY_VARIABLE:
			if (argnum == 0) {
				return OPF_VARIABLE_NAME;
			} else {
				return OPF_AMBIGUOUS; 
			}

		case OP_HAS_DOCKED:
		case OP_HAS_UNDOCKED:
		case OP_HAS_DOCKED_DELAY:
		case OP_HAS_UNDOCKED_DELAY:
		case OP_TIME_DOCKED:
		case OP_TIME_UNDOCKED:
			if ( argnum < 2 )
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_TIME_WING_DESTROYED:
		case OP_TIME_WING_ARRIVED:
		case OP_TIME_WING_DEPARTED:
		case OP_CLEAR_WING_GOALS:
			return OPF_WING;

		case OP_SET_SCANNED:
		case OP_SET_UNSCANNED:
		case OP_IS_SUBSYSTEM_DESTROYED:
		case OP_HITS_LEFT_SUBSYSTEM:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM;

		case OP_DISTANCE_SUBSYSTEM:
			if (argnum == 0)
				return OPF_SHIP_WING_POINT;
			else if (argnum == 1)
				return OPF_SHIP;
			else if (argnum == 2)
				return OPF_SUBSYSTEM;
			else
				Int3();		// shouldn't happen

		case OP_TARGETED:
			if (!argnum)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_POSITIVE;
			else
				return OPF_SUBSYSTEM;

		case OP_IS_SUBSYSTEM_DESTROYED_DELAY:
			if ( argnum == 0 )
				return OPF_SHIP;
			else if ( argnum == 1 )
				return OPF_SUBSYSTEM;
			else
				return OPF_POSITIVE;

		case OP_IS_IFF:
		case OP_CHANGE_IFF:
			if (!argnum)
				return OPF_IFF;
			else
				return OPF_SHIP_WING;

		case OP_ADD_SHIP_GOAL:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_AI_GOAL;

		case OP_ADD_WING_GOAL:
			if (!argnum)
				return OPF_WING;
			else
				return OPF_AI_GOAL;

		case OP_ADD_GOAL:
			if ( argnum == 0 )
				return OPF_SHIP_WING;
			else
				return OPF_AI_GOAL;

		case OP_WHEN:
		case OP_COND:
			if (!argnum)
				return OPF_BOOL;
			else
				return OPF_NULL;

		case OP_AI_DISABLE_SHIP:
		case OP_AI_DISARM_SHIP:
		case OP_AI_EVADE_SHIP:
		case OP_AI_STAY_NEAR_SHIP:
		case OP_AI_IGNORE:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_POSITIVE;

		case OP_AI_CHASE:
		case OP_AI_GUARD:
			if (!argnum)
				return OPF_SHIP_WING;
			else
				return OPF_POSITIVE;

		case OP_AI_UNDOCK:
		case OP_AI_KEEP_SAFE_DISTANCE:
			return OPF_POSITIVE;

		case OP_AI_DOCK:
			if (!argnum)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_DOCKER_POINT;
			else if (argnum == 2)
				return OPF_DOCKEE_POINT;
			else
				return OPF_POSITIVE;

		case OP_AI_CHASE_WING:
		case OP_AI_GUARD_WING:
			if (!argnum)
				return OPF_WING;
			else
				return OPF_POSITIVE;

		case OP_AI_DESTROY_SUBSYS:
			if (!argnum)
				return OPF_SHIP;
			else if (argnum == 1)
				return OPF_SUBSYSTEM;
			else
				return OPF_POSITIVE;
			
		case OP_GOALS_ID:
			return OPF_AI_GOAL;

		case OP_SET_CARGO:
		case OP_IS_CARGO:
			if (argnum == 0)
				return OPF_CARGO;
			else if (argnum == 1)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM;

		case OP_CHANGE_AI_CLASS:
		case OP_IS_AI_CLASS:
			if (argnum == 0)
				return OPF_AI_CLASS;
			else if (argnum == 1)
				return OPF_SHIP;
			else
				return OPF_SUBSYSTEM;

		case OP_CHANGE_SOUNDTRACK:
			return OPF_SOUNDTRACK_NAME;

		case OP_EXPLOSION_EFFECT:
			if (argnum <= 2)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_WARP_EFFECT:
			if (argnum <= 5)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_EMP_EFFECT:
			if (argnum <= 2)
				return OPF_NUMBER;
			else
				return OPF_POSITIVE;

		case OP_SEND_MESSAGE:
		case OP_SEND_RANDOM_MESSAGE:
			if ( argnum == 0 )
				return OPF_WHO_FROM;
			else if ( argnum == 1 )
				return OPF_PRIORITY;
			else
				return OPF_MESSAGE;

		case OP_SEND_MESSAGE_LIST:
			int a_mod;

			// every four, the value repeats
			a_mod = argnum % 4;			

			// who from
			if(a_mod == 0)
				return OPF_WHO_FROM;
			else if(a_mod == 1)
				return OPF_PRIORITY;
			else if(a_mod == 2)
				return OPF_MESSAGE;
			else if(a_mod == 3)
				return OPF_POSITIVE;

		case OP_TRAINING_MSG:
			if (argnum < 2)
				return OPF_MESSAGE;
			else
				return OPF_POSITIVE;

		case OP_SELF_DESTRUCT:
			return OPF_SHIP;

		case OP_NEXT_MISSION:
			return OPF_MISSION_NAME;

		case OP_END_CAMPAIGN:
		case OP_END_OF_CAMPAIGN:
			return OPF_NONE;

		case OP_PREVIOUS_GOAL_TRUE:
		case OP_PREVIOUS_GOAL_FALSE:
			if ( argnum == 0 )
				return OPF_MISSION_NAME;
			else if (argnum == 1 )
				return OPF_GOAL_NAME;
			else
				return OPF_BOOL;

		case OP_PREVIOUS_GOAL_INCOMPLETE:
			return OPF_GOAL_NAME;

		case OP_PREVIOUS_EVENT_TRUE:
		case OP_PREVIOUS_EVENT_FALSE:
		case OP_PREVIOUS_EVENT_INCOMPLETE:
			if (!argnum)
				return OPF_MISSION_NAME;
			else if ( argnum == 1 )
				return OPF_EVENT_NAME;
			else
				return OPF_BOOL;

		case OP_SABOTAGE_SUBSYSTEM:
		case OP_REPAIR_SUBSYSTEM:
		case OP_SET_SUBSYSTEM_STRNGTH:
			if (!argnum)
				return OPF_SHIP;		// changed from OPF_SHIP_NOT_PLAYER by Goober5000: now it can be the player ship also
			else if (argnum == 1 )
				return OPF_SUBSYSTEM;
			else
				return OPF_POSITIVE;

		case OP_WAYPOINTS_DONE:
			if ( argnum == 0 )
				return OPF_SHIP_WING;
			else
				return OPF_WAYPOINT_PATH;

		case OP_WAYPOINTS_DONE_DELAY:
			if ( argnum == 0 )
				return OPF_SHIP_WING;
			else if ( argnum == 1 )
				return OPF_WAYPOINT_PATH;
			else
				return OPF_POSITIVE;

		case OP_INVALIDATE_GOAL:
		case OP_VALIDATE_GOAL:
			return OPF_GOAL_NAME;

		case OP_SHIP_TYPE_DESTROYED:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP_TYPE;

		case OP_KEY_PRESSED:
			if (!argnum)
				return OPF_KEYPRESS;
			else
				return OPF_POSITIVE;

		case OP_KEY_RESET:
			return OPF_KEYPRESS;

		case OP_EVENT_TRUE:
		case OP_EVENT_FALSE:
			return OPF_EVENT_NAME;

		case OP_EVENT_INCOMPLETE:
		case OP_EVENT_TRUE_DELAY:
		case OP_EVENT_FALSE_DELAY:
			if (!argnum)
				return OPF_EVENT_NAME;
			else
				return OPF_POSITIVE;

		case OP_GOAL_INCOMPLETE:
		case OP_GOAL_TRUE_DELAY:
		case OP_GOAL_FALSE_DELAY:
			if (!argnum)
				return OPF_GOAL_NAME;
			else
				return OPF_POSITIVE;

		case OP_AI_PLAY_DEAD:
		case OP_AI_CHASE_ANY:
			return OPF_POSITIVE;

		case OP_AI_CHASE_ANY_EXCEPT:
			if (argnum == 0)
				return OPF_POSITIVE;
			else
				return OPF_SHIP_WING;

		case OP_AI_STAY_STILL:
			if (!argnum)
				return OPF_SHIP_POINT;
			else
				return OPF_POSITIVE;

		case OP_GOOD_REARM_TIME:
			if ( argnum == 0 )
				return OPF_IFF;
			else
				return OPF_POSITIVE;

		case OP_NUM_PLAYERS:
			return OPF_POSITIVE;

		case OP_SKILL_LEVEL_AT_LEAST:
			return OPF_SKILL_LEVEL;

		case OP_GRANT_MEDAL:
		case OP_WAS_MEDAL_GRANTED:
			return OPF_MEDAL_NAME;

		case OP_IS_CARGO_KNOWN:
			return OPF_SHIP;

		case OP_CARGO_KNOWN_DELAY:
			if ( argnum == 0 )
				return OPF_POSITIVE;
			else
				return OPF_SHIP;

		case OP_HAS_BEEN_TAGGED_DELAY:
			if ( argnum == 0 ) {
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP;
			}

		case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
			if ( argnum == 0 ) {
				return OPF_POSITIVE;
			} else if ( argnum == 1 ) {
				return OPF_SHIP;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_ALLOW_SHIP:
		case OP_TECH_ADD_SHIP:
			return OPF_SHIP_CLASS_NAME;

		case OP_ALLOW_WEAPON:
		case OP_TECH_ADD_WEAPON:
			return OPF_WEAPON_NAME;

		case OP_TECH_ADD_INTEL:
			return OPF_INTEL_NAME;

		case OP_TECH_RESET_TO_DEFAULT:
			return OPF_NONE;

		case OP_SHIP_VAPORIZE:
		case OP_SHIP_NO_VAPORIZE:
			return OPF_SHIP;

		case OP_DONT_COLLIDE_INVISIBLE:
		case OP_COLLIDE_INVISIBLE:
			return OPF_SHIP;

		case OP_WARP_BROKEN:
		case OP_WARP_NOT_BROKEN:
		case OP_WARP_NEVER:
		case OP_WARP_ALLOWED:
			return OPF_SHIP;

		case OP_FLASH_HUD_GAUGE:
			return OPF_HUD_GAUGE_NAME;

		case OP_GOOD_SECONDARY_TIME:
			if ( argnum == 0 )
				return OPF_IFF;
			else if ( argnum == 1 )
				return OPF_POSITIVE;
			else if ( argnum == 2 )
				return OPF_HUGE_WEAPON;
			else
				return OPF_SHIP;

		case OP_PERCENT_SHIPS_DEPARTED:
		case OP_PERCENT_SHIPS_DESTROYED:
			if ( argnum == 0 ){
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP_WING;
			}
			break;

		case OP_DEPART_NODE_DELAY:	
			if ( argnum == 0 ){
				return OPF_POSITIVE;
			} else if ( argnum == 1 ){
				return OPF_JUMP_NODE_NAME;
			} else {
				return OPF_SHIP;
			}

		case OP_DESTROYED_DEPARTED_DELAY:
			if ( argnum == 0 ){
				return OPF_POSITIVE;
			} else {
				return OPF_SHIP_WING;
			}

		case OP_JETTISON_CARGO:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_POSITIVE;
			}

		case OP_CARGO_NO_DEPLETE:
			if (argnum == 0) {
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_BEAM_FIRE:
			switch(argnum){
			case 0:
				return OPF_SHIP;
			case 1:
				return OPF_SUBSYSTEM;
			case 2:
				return OPF_SHIP;
			case 3:
				return OPF_SUBSYSTEM;
			}

		case OP_IS_TAGGED:
			return OPF_SHIP;

		case OP_NUM_KILLS:
			return OPF_SHIP;

		case OP_NUM_TYPE_KILLS:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_SHIP_TYPE;
			}

		case OP_NUM_CLASS_KILLS:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_SHIP_CLASS_NAME;
			}

		case OP_BEAM_FREE:
		case OP_BEAM_LOCK:
		case OP_TURRET_FREE:
		case OP_TURRET_LOCK:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_BEAM_FREE_ALL:
		case OP_BEAM_LOCK_ALL:
		case OP_TURRET_FREE_ALL:
		case OP_TURRET_LOCK_ALL:
		case OP_TURRET_TAGGED_ONLY_ALL:
		case OP_TURRET_TAGGED_CLEAR_ALL:
			return OPF_SHIP;

		case OP_ADD_REMOVE_ESCORT:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_AWACS_SET_RADIUS:
			if(argnum == 0){
				return OPF_SHIP;
			} else if(argnum == 1){
				return OPF_AWACS_SUBSYSTEM;
			} else {
				return OPF_NUMBER;
			}

		case OP_PRIMITIVE_SENSORS_SET_RANGE:
			if (!argnum)
				return OPF_SHIP;
			else
				return OPF_NUMBER;

		case OP_CAP_WAYPOINT_SPEED:
			if (argnum == 0) {
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_SUBSYS_SET_RANDOM:
			if (argnum == 0) {
				return OPF_SHIP;
			} else if (argnum == 1 || argnum == 2) {
				return OPF_NUMBER;
			} else {
				return OPF_SUBSYSTEM;
			}

		case OP_SUPERNOVA_START:
			return OPF_NUMBER;

		case OP_SHIELD_RECHARGE_PCT:
		case OP_WEAPON_RECHARGE_PCT:
		case OP_ENGINE_RECHARGE_PCT:
			return OPF_SHIP;			

		case OP_SHIELD_QUAD_LOW:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_PRIMARY_AMMO_PCT:
		case OP_SECONDARY_AMMO_PCT:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_IS_SECONDARY_SELECTED:
		case OP_IS_PRIMARY_SELECTED:
			if(argnum == 0){
				return OPF_SHIP;
			} else {
				return OPF_NUMBER;
			}

		case OP_DAMAGED_ESCORT_LIST:
			if (argnum < 2)
			{
				return OPF_NUMBER;
			}
			else
			{
				return OPF_SHIP;
			}

		case OP_DAMAGED_ESCORT_LIST_ALL:
			return OPF_POSITIVE;

		case OP_CHANGE_SHIP_MODEL:
		case OP_CHANGE_SHIP_CLASS:
			if (!argnum)
				return OPF_SHIP_CLASS_NAME;
			else
				return OPF_SHIP;

		case OP_DEACTIVATE_GLOW_POINTS:	//-Bobboau
		case OP_ACTIVATE_GLOW_POINTS:	//-Bobboau
		case OP_DEACTIVATE_GLOW_MAPS:	//-Bobboau
		case OP_ACTIVATE_GLOW_MAPS:		//-Bobboau
			return OPF_SHIP;	//a list of ships that are to be activated/deactivated
		case OP_DEACTIVATE_GLOW_POINT_BANK:
		case OP_ACTIVATE_GLOW_POINT_BANK:
			if (!argnum)
				return OPF_SHIP;		//the ship
			else
				return OPF_POSITIVE;		//the glow bank to disable

		// Goober5000 - this is complicated :)
		case OP_SET_SUPPORT_SHIP:
			if ((argnum == 0) || (argnum == 2))
				return OPF_DEPARTURE_LOCATION;	// use this for both because we don't want Near Ship or In Front of Ship
			if ((argnum == 1) || (argnum == 3))
				return OPF_SHIP_WITH_BAY;		// same - we don't want to anchor around anything without a docking bay
			if (argnum == 4)
				return OPF_SUPPORT_SHIP_CLASS;
			if (argnum == 5)
				return OPF_NUMBER;

		default:
			Int3();
	}

	return 0;
}

void update_block_names(const char *old_name, const char *new_name)
{
	int i;

	for (i=0; i<MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_BLOCK) {
			if ( !stricmp(old_name, Sexp_variables[i].variable_name) ) {
				strcpy(Sexp_variables[i].variable_name, new_name);
			}
		}
	}
}

// DA: 1/7/99  Used to rename ships and waypoints, not variables
// Strictly used in FRED
void update_sexp_references(char *old_name, char *new_name)
{
	int i;

	// update_block_names
	update_block_names(old_name, new_name);

	Assert(strlen(new_name) < TOKEN_LENGTH);
	for (i=0; i<MAX_SEXP_NODES; i++){
		if ((SEXP_NODE_TYPE(i) == SEXP_ATOM) && (Sexp_nodes[i].subtype == SEXP_ATOM_STRING)){
			if (!stricmp(CTEXT(i), old_name)){
				strcpy(CTEXT(i), new_name);
			}
		}
	}
}

// DA: 1/7/99  Used to rename event names, goal names, not variables
// Strictly used in FRED
void update_sexp_references(char *old_name, char *new_name, int format)
{
	int i;

	Assert(strlen(new_name) < TOKEN_LENGTH);
	for (i=0; i<MAX_SEXP_NODES; i++){
		if (is_sexp_top_level(i)){
			update_sexp_references(old_name, new_name, format, i);
		}
	}
}

// DA: 1/7/99  Used to rename event names, goal names, not variables
// recursive function to update references to a certain type of data
void update_sexp_references(char *old_name, char *new_name, int format, int node)
{
	int i, n, op;

	if (node < 0){
		return;
	}

	if ((SEXP_NODE_TYPE(node) == SEXP_LIST) && (Sexp_nodes[node].subtype == SEXP_ATOM_LIST)) {
		if (Sexp_nodes[node].first){
			update_sexp_references(old_name, new_name, format, Sexp_nodes[node].first);
		}
		if (Sexp_nodes[node].rest){
			update_sexp_references(old_name, new_name, format, Sexp_nodes[node].rest);
		}

		return;
	}

	if (SEXP_NODE_TYPE(node) != SEXP_ATOM){
		return;
	}

	if (Sexp_nodes[node].subtype != SEXP_ATOM_OPERATOR){
		return;
	}

	op = identify_operator(CTEXT(node));
	Assert(Sexp_nodes[node].first < 0);
	n = Sexp_nodes[node].rest;
	i = 0;
	while (n >= 0) {
		if (SEXP_NODE_TYPE(n) == SEXP_LIST){
			update_sexp_references(old_name, new_name, format, Sexp_nodes[n].first);
		} else {
			Assert((SEXP_NODE_TYPE(n) == SEXP_ATOM) && ((Sexp_nodes[n].subtype == SEXP_ATOM_NUMBER) || (Sexp_nodes[n].subtype == SEXP_ATOM_STRING)));
			if (query_operator_argument_type(op, i) == format) {
				if (!stricmp(CTEXT(n), old_name)){
					strcpy(CTEXT(n), new_name);
				}
			}
		}

		n = Sexp_nodes[n].rest;
		i++;
	}
}

int query_referenced_in_sexp(int mode, char *name, int *node)
{
	int i, n, j;

	for (n=0; n<MAX_SEXP_NODES; n++){
		if ((SEXP_NODE_TYPE(n) == SEXP_ATOM) && (Sexp_nodes[n].subtype == SEXP_ATOM_STRING)){
			if (!stricmp(CTEXT(n), name)){
				break;
			}
		}
	}

	if (n == MAX_SEXP_NODES){
		return 0;
	}

	if (node){
		*node = n;
	}

	// so we know it's being used somewhere..  Time to find out where..
	for (i=0; i<MAX_SHIPS; i++)
		if (Ships[i].objnum >= 0) {
			if (query_node_in_sexp(n, Ships[i].arrival_cue)){
				return i | SRC_SHIP_ARRIVAL;
			}
			if (query_node_in_sexp(n, Ships[i].departure_cue)){
				return i | SRC_SHIP_DEPARTURE;
			}
		}

	for (i=0; i<MAX_WINGS; i++){
		if (Wings[i].wave_count) {
			if (query_node_in_sexp(n, Wings[i].arrival_cue)){
				return i | SRC_WING_ARRIVAL;
			}
			if (query_node_in_sexp(n, Wings[i].departure_cue)){
				return i | SRC_WING_DEPARTURE;
			}
		}
	}

	for (i=0; i<Num_mission_events; i++){
		if (query_node_in_sexp(n, Mission_events[i].formula)){
			return i | SRC_EVENT;
		}
	}

	for (i=0; i<Num_goals; i++){
		if (query_node_in_sexp(n, Mission_goals[i].formula)){
			return i | SRC_MISSION_GOAL;
		}
	}

	for (j=0; j<Num_teams; j++) {
		for (i=0; i<Debriefings[j].num_stages; i++) {
			if (query_node_in_sexp(n, Debriefings[j].stages[i].formula)){
				return i | SRC_DEBRIEFING;
			}
		}
	}

	for (j=0; j<Num_teams; j++) {
		for (i=0; i<Briefings[j].num_stages; i++) {
			if (query_node_in_sexp(n, Briefings[j].stages[i].formula)){
				return i | SRC_BRIEFING;
			}
		}
	}

	return SRC_UNKNOWN;
}

int verify_vector(char *text)
{
	char *str;
	int i, z, len = 0;

	for (i=0; i<Num_waypoint_lists; i++) {
		len = strlen(str = Waypoint_lists[i].name);
		if (!strnicmp(str, text, len)){
			if (!text[len] || text[len] == ':'){
				break;
			}
		}
	}

	if (i < Num_waypoint_lists) {
		if (!text[len]){
			return 0;  // a valid waypoint path
		}

		str = &text[len + 1];
		while (*str){
			if (!isdigit(*str++)){
				return -1;  // not a valid number
			}
		}

		z = atoi(&text[len + 1]);
		if (z < 1 || z > Waypoint_lists[i].count){
			return -1;  // out of range
		}

		return 0;  // a valid waypoint
	}

	len = strlen(text);
	if (text[0] != '(' || text[len - 1] != ')'){
		return -1;
	}

	str = &text[0];
	skip_white(&str);
	if (*str != '('){
		return -1;
	}

	str++;
	skip_white(&str);
	if (validate_float(&str)){
		return -1;
	}

	skip_white(&str);
	if (validate_float(&str)){
		return -1;
	}

	skip_white(&str);
	if (validate_float(&str)){
		return -1;
	}

	skip_white(&str);
	if (*str != ')'){
		return -1;
	}

	str++;
	skip_white(&str);
	if (*str){
		return -1;
	}

	return 0;
}

void skip_white(char **str)
{
	if ((**str == ' ') || (**str == '\t')){
		(*str)++;
	}
}

int validate_float(char **str)
{
	int count = 0, dot = 0;

	while (isdigit(**str) || **str == '.') {
		if (**str == '.') {
			if (dot){
				return -1;
			}

			dot = 1;
		}

		(*str)++;
		count++;
	}

	if (!count){
		return -1;
	}

	return 0;
}

// check if operator return type opr is a valid match for operator argument type opf
int sexp_query_type_match(int opf, int opr)
{
	switch (opf) {
		case OPF_NUMBER:
			return ((opr == OPR_NUMBER) || (opr == OPR_POSITIVE));

		case OPF_POSITIVE:
			return (opr == OPR_POSITIVE);

		case OPF_BOOL:
			return (opr == OPR_BOOL);

		case OPF_NULL:
			return (opr == OPR_NULL);

		case OPF_AI_GOAL:
			return (opr == OPR_AI_GOAL);
	}

	return 0;
}

char *sexp_error_message(int num)
{
	switch (num) {
		case SEXP_CHECK_NONOP_ARGS:
			return "Data shouldn't have arguments";

		case SEXP_CHECK_OP_EXPECTED:
			return "Operator expected instead of data";

		case SEXP_CHECK_UNKNOWN_OP:
			return "Unrecognized operator";

		case SEXP_CHECK_TYPE_MISMATCH:
			return "Argument type mismatch";

		case SEXP_CHECK_BAD_ARG_COUNT:
			return "Argument count is illegal";

		case SEXP_CHECK_UNKNOWN_TYPE:
			return "Unknown operator argument type";

		case SEXP_CHECK_INVALID_NUM:
			return "Not a number";

		case SEXP_CHECK_INVALID_SHIP:
			return "Invalid ship name";

		case SEXP_CHECK_INVALID_WING:
			return "Invalid wing name";

		case SEXP_CHECK_INVALID_SUBSYS:
			return "Invalid subsystem name";

		case SEXP_CHECK_INVALID_IFF:
			return "Invalid team name";

		case SEXP_CHECK_INVALID_AI_CLASS:
			return "Invalid AI class name";

		case SEXP_CHECK_INVALID_POINT:
			return "Invalid point";

		case SEXP_CHECK_NEGATIVE_NUM:
			return "Negative number not allowed";

		case SEXP_CHECK_INVALID_SHIP_WING:
			return "Invalid ship/wing name";

		case SEXP_CHECK_INVALID_SHIP_TYPE:
			return "Invalid ship type";

		case SEXP_CHECK_UNKNOWN_MESSAGE:
			return "Invalid message name";

		case SEXP_CHECK_INVALID_PRIORITY:
			return "Invalid priority";

		case SEXP_CHECK_INVALID_MISSION_NAME:
			return "Invalid mission filename";

		case SEXP_CHECK_INVALID_GOAL_NAME:
			return "Invalid goal name";

		case SEXP_CHECK_INVALID_LEVEL:
			return "Mission level too low in tree";

		case SEXP_CHECK_INVALID_MSG_SOURCE:
			return "Invalid message source";

		case SEXP_CHECK_INVALID_DOCKER_POINT:
			return "Invalid docker point";

		case SEXP_CHECK_INVALID_DOCKEE_POINT:
			return "Invalid dockee point";

		case SEXP_CHECK_ORDER_NOT_ALLOWED:
			return "Ship not allowed to have this order";

		case SEXP_CHECK_DOCKING_NOT_ALLOWED:
			return "Ship can't dock with target ship";

		case SEXP_CHECK_NUM_RANGE_INVALID:
			return "Number is out of range";

		case SEXP_CHECK_INVALID_EVENT_NAME:
			return "Event name is invalid (not known)";

		case SEXP_CHECK_INVALID_SKILL_LEVEL:
			return "Skill level name is invalid (not known)";

		case SEXP_CHECK_INVALID_MEDAL_NAME:
			return "Invalid medal name";

		case SEXP_CHECK_INVALID_WEAPON_NAME:
			return "Invalid weapon name";

		case SEXP_CHECK_INVALID_INTEL_NAME:
			return "Invalid intel name";

		case SEXP_CHECK_INVALID_SHIP_CLASS_NAME:
			return "Invalid ship class name";

		case SEXP_CHECK_INVALID_GAUGE_NAME:
			return "Invalid HUD gauges name";

		case SEXP_CHECK_INVALID_JUMP_NODE:
			return "Invalid Jump Node name";

		case SEXP_CHECK_UNKNOWN_ERROR:
			return "Unknown error";

		case SEXP_CHECK_INVALID_SUPPORT_SHIP_CLASS:
			return "Invalid support ship class";

		case SEXP_CHECK_INVALID_SHIP_WITH_BAY:
			return "Ship does not have a fighter bay";

		case SEXP_CHECK_INVALID_ARRIVAL_LOCATION:
			return "Invalid arrival location";

		case SEXP_CHECK_INVALID_DEPARTURE_LOCATION:
			return "Invalid departure location";

		case SEXP_CHECK_INVALID_ARRIVAL_ANCHOR_ALL:
			return "Invalid universal arrival anchor";

		case SEXP_CHECK_INVALID_SOUNDTRACK_NAME:
			return "Invalid soundtrack name";
	}

	sprintf(Sexp_error_text, "Sexp error code %d", num);
	return Sexp_error_text;
}

int query_sexp_ai_goal_valid(int sexp_ai_goal, int ship)
{
	int i, op;

	for (op=0; op<Num_operators; op++)
		if (Operators[op].value == sexp_ai_goal)
			break;

	Assert(op < Num_operators);
	for (i=0; i<Num_sexp_ai_goal_links; i++)
		if (Sexp_ai_goal_links[i].op_code == sexp_ai_goal)
			break;

	Assert(i < Num_sexp_ai_goal_links);
	return ai_query_goal_valid(ship, Sexp_ai_goal_links[i].ai_goal);
}

// Takes an Sexp_node.text pointing to a variable (of form "Sexp_variables[xx]=string" or "Sexp_variables[xx]=number")
// and returns the index into the Sexp_variables array of the actual value 
int extract_sexp_variable_index(int node)
{
	char *text = Sexp_nodes[node].text;
	char char_index[8];
	char *start_index;
	int variable_index;

	// get past the '['
	start_index = text + 15;
	Assert(isdigit(*start_index));

	int len = 0;

	while ( *start_index != ']' ) {
		char_index[len++] = *(start_index++);
		Assert(len < 3);
	}

	Assert(len > 0);
	char_index[len] = 0;	// append null termination to string

	variable_index = atoi(char_index);
	Assert( (variable_index >= 0) && (variable_index < MAX_SEXP_VARIABLES) );

	return variable_index;
}


// wrapper around Sexp_node[xx].text for normal and variable
char *CTEXT(int n)
{
	if (Sexp_nodes[n].type & SEXP_FLAG_VARIABLE) {
		int sexp_variable_index;
		if (Fred_running) {
			sexp_variable_index = get_index_sexp_variable_name(Sexp_nodes[n].text);
			Assert(sexp_variable_index != -1);
		} else {
//			sexp_variable_index = extract_sexp_variable_index(n);
			sexp_variable_index = atoi(Sexp_nodes[n].text);
		}
		// Reference a Sexp_variable
		// string format -- "Sexp_variables[xx]=number" or "Sexp_variables[xx]=string", where xx is the index

		Assert( !(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NOT_USED) );
		Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

		return Sexp_variables[sexp_variable_index].text;
	} else {
		return Sexp_nodes[n].text;
	}
}


// Set all Sexp_variables to type uninitialized
void init_sexp_vars()
{
	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
//		if ( !(Sexp_nodes[i].type & SEXP_FLAG_PERSISTENT) )
//			Sexp_nodes[i].type = SEXP_NOT_USED;
		Sexp_variables[i].type = SEXP_VARIABLE_NOT_USED;
	}
}


// Adds an Sexp_variable to be used in a mission.
// This should be called from within mission parse.
int sexp_add_variable(const char *text, const char *var_name, int type, int index)
{
	// if index == -1, find next open slot
	if (index == -1) {
		for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
			if (Sexp_variables[i].type == SEXP_VARIABLE_NOT_USED) {
				index = i;
				break;
			}
		}
	} else {
		Assert( (index >= 0) && (index < MAX_SEXP_VARIABLES) );
	}

	if (index >= 0) {
		strcpy(Sexp_variables[index].text, text);
		strcpy(Sexp_variables[index].variable_name, var_name);
		Sexp_variables[index].type &= ~SEXP_VARIABLE_NOT_USED;
		Sexp_variables[index].type = (type | SEXP_VARIABLE_SET);
	}

	return index;
}


// Modifies and Sexp_variable to be used in a mission
// This should be called in mission when an sexp_variable is to be modified
void sexp_modify_variable(char *text, int index)
{
	Assert(index >= 0 && index < MAX_SEXP_VARIABLES);
	Assert(Sexp_variables[index].type & SEXP_VARIABLE_SET);
#ifndef NO_NETWORK
	Assert( !MULTIPLAYER_CLIENT );
#endif

	strcpy(Sexp_variables[index].text, text);
	Sexp_variables[index].type |= SEXP_VARIABLE_MODIFIED;

	// do multi_callback_here
}

void sexp_modify_variable(int n)
{
	int sexp_variable_index;
	int new_number;
//	char *new_char_var;
	char number_as_str[TOKEN_LENGTH];

#ifndef NO_NETWORK
	// Only do single player of multi host
	if ( MULTIPLAYER_CLIENT ) {
		return;
	}
#endif

	if (n != -1) {
		// get sexp_variable index
		Assert(Sexp_nodes[n].first == -1);
		sexp_variable_index = atoi(Sexp_nodes[n].text);

		// verify variable set
		Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_SET);

		if (Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NUMBER) {
			// get new numerical value
			new_number = eval_sexp(Sexp_nodes[n].rest);

			sprintf(number_as_str, "%d", new_number);
			sexp_modify_variable(number_as_str, sexp_variable_index);
		} else {
			// get new string
			Assert(Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING);

			char *new_text = Sexp_nodes[Sexp_nodes[n].rest].text;
			sexp_modify_variable(new_text, sexp_variable_index);
		}
	}
}
	


// Different type needed for Fred (1) allow modification of type (2) no callback required
void sexp_fred_modify_variable(const char *text, const char *var_name, int index, int type)
{
	Assert(index >= 0 && index < MAX_SEXP_VARIABLES);
	Assert(Sexp_variables[index].type & SEXP_VARIABLE_SET);
	Assert( (type & SEXP_VARIABLE_NUMBER) || (type & SEXP_VARIABLE_STRING) );

	strcpy(Sexp_variables[index].text, text);
	strcpy(Sexp_variables[index].variable_name, var_name);
	Sexp_variables[index].type = (SEXP_VARIABLE_SET | SEXP_VARIABLE_MODIFIED | type);
}

// return index of sexp_variable_name, -1 if not found
int get_index_sexp_variable_name(const char* temp_name)
{
	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			// check case sensitive
			if ( !strcmp(Sexp_variables[i].variable_name, temp_name) ) {
				return i;
			}
		}
	}

	// not found
	return -1;
}


// counts number of sexp_variables that are set
int sexp_variable_count()
{
	int count = 0;

	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if ( (Sexp_variables[i].type & SEXP_VARIABLE_SET)  && !(Sexp_variables[i].type & SEXP_VARIABLE_BLOCK) ) {
			count++;
		}
	}

	return count;
}


// deletes sexp_variable from active
void sexp_variable_delete(int index)
{
	Assert(Sexp_variables[index].type & SEXP_VARIABLE_SET);

	Sexp_variables[index].type = SEXP_VARIABLE_NOT_USED;
}

int sexp_var_compare(const void *var1, const void *var2)
{
	int set1, set2;
	sexp_variable *sexp_var1, *sexp_var2;

	sexp_var1 = (sexp_variable*) var1;
	sexp_var2 = (sexp_variable*) var2;

	set1 = sexp_var1->type & SEXP_VARIABLE_SET;
	set2 = sexp_var2->type & SEXP_VARIABLE_SET;

	if (!set1 && !set2) {
		return 0;
	} else if (set1 && !set2) {
		return -1;
	} else if (!set1 && set2) {
		return 1;
	} else {
		return stricmp( sexp_var1->variable_name, sexp_var2->variable_name);
	}
}

// Count number of variables in block
int sexp_variable_block_count()
{
	int count = 0;
	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_BLOCK) {
			count++;
		}
	}

	return count;
}

// Sort sexp_variable list lexigraphically, with set before unset
void sexp_variable_sort()
{
	qsort( (void *)Sexp_variables, (size_t)(MAX_SEXP_VARIABLES - sexp_variable_block_count()), sizeof(sexp_variable), sexp_var_compare );
}

int sexp_variable_allocate_block(const char* block_name, int block_type)
{
	int num_blocks, block_count, var_count, start;
	block_count = sexp_variable_block_count();
	var_count = sexp_variable_count();

	if (block_type & SEXP_VARIABLE_BLOCK_EXP) {
		num_blocks = BLOCK_EXP_SIZE;
	} else {
		Int3();	// add new block type here with size
		return -1;
	}

	if (block_count + var_count > (MAX_SEXP_VARIABLES - num_blocks)) {
		// not enough free space
		return -1;
	}

	// squeeze all variables to front of array
	sexp_variable_sort();

	// squeeze all block to end of array
	sexp_variable_condense_block();

	start = MAX_SEXP_VARIABLES - block_count - num_blocks;

	for (int idx=start; idx<start+num_blocks; idx++) {
		Assert(Sexp_variables[idx].type == SEXP_VARIABLE_NOT_USED);
		Sexp_variables[idx].type = SEXP_VARIABLE_BLOCK | block_type;
		strcpy(Sexp_variables[idx].variable_name, block_name);
	}

	return start;
}

// squeeze all blocks to top of array
void sexp_variable_condense_block()
{
	int temp_idx, idx, var_count;

	var_count = sexp_variable_count();
	temp_idx = MAX_SEXP_VARIABLES-1;

	for (idx=MAX_SEXP_VARIABLES-1; idx>var_count-1; idx--) {
		if (Sexp_variables[idx].type & SEXP_VARIABLE_BLOCK) {
			if (temp_idx > idx) {
				Sexp_variables[temp_idx] = Sexp_variables[idx];
				Sexp_variables[idx].type = SEXP_VARIABLE_NOT_USED;
			}
			temp_idx--;
		}
	}
}


void sexp_variable_block_free(const char *ship_name, int start_index, int block_type)
{
	int num_blocks;

	if (block_type & SEXP_VARIABLE_BLOCK_EXP) {
		num_blocks = BLOCK_EXP_SIZE;
	} else {
		Int3();  // new type of block
		return;
	}

	for (int i=start_index; i<(start_index + num_blocks); i++) {
		Assert(!stricmp(Sexp_variables[i].variable_name, ship_name));

		Assert(Sexp_variables[i].type & block_type);

		Sexp_variables[i].type = SEXP_VARIABLE_NOT_USED;
	}

	sexp_variable_condense_block();
}

// evaluate number which may result from an operator or may be text
// only useful for first operand
int num_eval(int node)
{
	if ( CAR(node) != -1 ) {
		return eval_sexp(CAR(node));
	} else {
		return atoi(CTEXT(node));
	}
}

// Goober5000 - for FRED2 menu subcategories
int get_subcategory(int sexp_id)
{
	switch(sexp_id)
	{
		case OP_SEND_MESSAGE_LIST:
		case OP_SEND_MESSAGE:
		case OP_SEND_RANDOM_MESSAGE:
		case OP_INVALIDATE_GOAL:
		case OP_VALIDATE_GOAL:
			return CHANGE_SUBCATEGORY_MESSAGING_AND_MISSION_GOALS;
			
		case OP_ADD_GOAL:
		case OP_CLEAR_GOALS:
		case OP_GOOD_REARM_TIME:
		case OP_GOOD_SECONDARY_TIME:
		case OP_CHANGE_IFF:
		case OP_CHANGE_AI_CLASS:
		case OP_PROTECT_SHIP:
		case OP_UNPROTECT_SHIP:
		case OP_BEAM_PROTECT_SHIP:
		case OP_BEAM_UNPROTECT_SHIP:
			return CHANGE_SUBCATEGORY_AI_AND_IFF;
			
		case OP_SABOTAGE_SUBSYSTEM:
		case OP_REPAIR_SUBSYSTEM:
		case OP_SET_SUBSYSTEM_STRNGTH:
		case OP_SUBSYS_SET_RANDOM:
		case OP_SELF_DESTRUCT:
		case OP_TRANSFER_CARGO:
		case OP_EXCHANGE_CARGO:
		case OP_SET_CARGO:
		case OP_JETTISON_CARGO:
		case OP_CARGO_NO_DEPLETE:
		case OP_SET_SCANNED:
		case OP_SET_UNSCANNED:
			return CHANGE_SUBCATEGORY_SUBSYSTEMS_AND_CARGO;
			
		case OP_SHIP_INVULNERABLE:
		case OP_SHIP_VULNERABLE:
		case OP_SHIP_GUARDIAN:
		case OP_SHIP_NO_GUARDIAN:
		case OP_SHIP_INVISIBLE:
		case OP_SHIP_VISIBLE:
		case OP_SHIP_STEALTHY:
		case OP_SHIP_UNSTEALTHY:
		case OP_FRIENDLY_STEALTH_INVISIBLE:
		case OP_FRIENDLY_STEALTH_VISIBLE:
		case OP_WARP_BROKEN:
		case OP_WARP_NOT_BROKEN:
		case OP_WARP_NEVER:
		case OP_WARP_ALLOWED:
			return CHANGE_SUBCATEGORY_SHIP_STATUS;
			
		case OP_BEAM_FIRE:
		case OP_BEAM_FREE:
		case OP_BEAM_FREE_ALL:
		case OP_BEAM_LOCK:
		case OP_BEAM_LOCK_ALL:
		case OP_TURRET_FREE:
		case OP_TURRET_FREE_ALL:
		case OP_TURRET_LOCK:
		case OP_TURRET_LOCK_ALL:
		case OP_TURRET_TAGGED_ONLY_ALL:
		case OP_TURRET_TAGGED_CLEAR_ALL:
			return CHANGE_SUBCATEGORY_BEAMS_AND_TURRETS;

		case OP_RED_ALERT:
		case OP_END_MISSION:
		case OP_END_CAMPAIGN:
		case OP_GRANT_PROMOTION:
		case OP_GRANT_MEDAL:
		case OP_ALLOW_SHIP:
		case OP_ALLOW_WEAPON:
		case OP_TECH_ADD_SHIP:
		case OP_TECH_ADD_WEAPON:
		case OP_TECH_ADD_INTEL:
		case OP_TECH_RESET_TO_DEFAULT:
			return CHANGE_SUBCATEGORY_MISSION_AND_CAMPAIGN;

		case OP_DONT_COLLIDE_INVISIBLE:
		case OP_COLLIDE_INVISIBLE:
		case OP_CHANGE_SHIP_MODEL:
		case OP_CHANGE_SHIP_CLASS:
		case OP_DEACTIVATE_GLOW_POINTS:
		case OP_ACTIVATE_GLOW_POINTS:
		case OP_DEACTIVATE_GLOW_MAPS:
		case OP_ACTIVATE_GLOW_MAPS:
		case OP_DEACTIVATE_GLOW_POINT_BANK:
		case OP_ACTIVATE_GLOW_POINT_BANK:
			return CHANGE_SUBCATEGORY_MODELS_AND_TEXTURES;

		case OP_SET_SHIP_POSITION:
		case OP_SET_SHIP_FACING:
		case OP_SET_SHIP_FACING_OBJECT:
			return CHANGE_SUBCATEGORY_COORDINATE_MANIPULATION;

		case OP_MODIFY_VARIABLE:
		case OP_ADD_REMOVE_ESCORT:
		case OP_AWACS_SET_RADIUS:
		case OP_PRIMITIVE_SENSORS_SET_RANGE:
		case OP_CAP_WAYPOINT_SPEED:
		case OP_SET_SPECIAL_WARPOUT_NAME:
		case OP_SHIP_VANISH:
		case OP_SUPERNOVA_START:
		case OP_SHIP_VAPORIZE:
		case OP_SHIP_NO_VAPORIZE:
		case OP_SHIP_LIGHTS_ON:
		case OP_SHIP_LIGHTS_OFF:
		case OP_SHIELDS_ON:
		case OP_SHIELDS_OFF:
		case OP_DAMAGED_ESCORT_LIST:
		case OP_DAMAGED_ESCORT_LIST_ALL:
		case OP_SET_SUPPORT_SHIP:
		case OP_CHANGE_SOUNDTRACK:
		case OP_EXPLOSION_EFFECT:
		case OP_WARP_EFFECT:
		case OP_EMP_EFFECT:
			return CHANGE_SUBCATEGORY_SPECIAL;
		
		default:
			return -1;		// sexp doesn't have a subcategory
	}
}
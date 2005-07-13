/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Io/KeyControl.cpp $
 * $Revision: 2.52 $
 * $Date: 2005-07-13 00:44:22 $
 * $Author: Goober5000 $
 *
 * Routines to read and deal with keyboard input.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.51  2005/04/25 00:23:23  wmcoolmon
 * MAX_SHIP_TYPES > Num_ship_types
 *
 * Revision 2.50  2005/04/20 04:34:01  phreak
 * get the cheat keys for "force weapon" working again
 *
 * Revision 2.49  2005/04/05 05:53:18  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.48  2005/03/27 13:23:05  Goober5000
 * rewrote the weapon cheat cycle functions to avoid recursion
 * --Goober5000
 *
 * Revision 2.47  2005/03/27 12:28:33  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.46  2005/03/27 04:16:19  wmcoolmon
 * Fixed possible infinite loop problem
 * ----------------------------------------------------------------------
 *
 * Revision 2.45  2005/03/25 06:57:34  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.44  2005/03/03 06:05:28  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.43  2005/03/02 21:24:41  taylor
 * more NO_NETWORK/INF_BUILD goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.42  2005/02/21 05:40:16  Goober5000
 * restored release version of some cheat codes; removed a cheat for cloaking
 * --Goober5000
 *
 * Revision 2.41  2005/02/20 05:22:44  Goober5000
 * restored original behavior for cheat code time dilation
 * --Goober5000
 *
 * Revision 2.40  2005/02/04 10:12:30  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.39  2005/01/16 22:39:09  wmcoolmon
 * Added VM_TOPDOWN view; Added 2D mission mode, add 16384 to mission +Flags to use.
 *
 * Revision 2.38  2004/09/28 19:54:32  Kazan
 * | is binary or, || is boolean or - please use the right one
 * autopilot velocity ramping biasing
 * made debugged+k kill guardianed ships
 *
 * Revision 2.37  2004/08/26 18:19:36  Kazan
 * small standalone-related multibug fix
 * prohibited execution of process_debug_keys(int) while Game_mode & GM_MULTIPLAYER
 *
 * Revision 2.36  2004/07/29 19:37:50  Kazan
 * unbug the JS bug i caused --- kazan
 *
 * Revision 2.35  2004/07/26 20:47:33  Kazan
 * remove MCD complete
 *
 * Revision 2.34  2004/07/25 00:31:29  Kazan
 * i have absolutely nothing to say about that subject
 *
 * Revision 2.33  2004/07/12 16:32:51  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.32  2004/07/12 03:19:16  Kazan
 * removed a couple pointless useless messages from the debug console
 *
 * Revision 2.31  2004/05/10 10:51:53  Goober5000
 * made primary and secondary banks quite a bit more friendly... added error-checking
 * and reorganized a bunch of code
 * --Goober5000
 *
 * Revision 2.30  2004/05/10 06:11:47  Goober5000
 * fixed warp decision
 * --Goober5000
 *
 * Revision 2.29  2004/05/03 21:22:21  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.28  2004/04/26 19:50:04  Goober5000
 * added back in Kazan's line: now you can slow down in cheat mode
 * whether you press tilde or not
 * --Goober5000
 *
 * Revision 2.27  2004/04/26 19:43:07  Goober5000
 * rolled back Kazan's change
 * --Goober5000
 *
 * Revision 2.25  2004/04/25 06:31:53  Goober5000
 * made time dilation only available in cheat mode; also fixed an obscure CTD
 * --Goober5000
 *
 * Revision 2.24  2004/04/24 15:44:21  Kazan
 * Fixed Lobby screen showing up while exiting from non-tracker games
 * Time dialiation up to 1/4
 *
 * Revision 2.23  2004/04/06 03:09:26  phreak
 * added a control config option for the wireframe hud targetbox i enabled ages ago
 *
 * Revision 2.22  2004/04/06 01:11:41  Kazan
 * make custom build work again
 *
 * Revision 2.21  2004/03/05 09:02:04  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.20  2004/01/30 07:39:07  Goober5000
 * whew - I just went through all the code I ever added (or at least, that I could
 * find that I commented with a Goober5000 tag) and added a bunch of Asserts
 * and error-checking
 * --Goober5000
 *
 * Revision 2.19  2004/01/14 06:38:41  Goober5000
 * fixx0red a bug
 * --Goober5000
 *
 * Revision 2.18  2003/10/15 22:03:25  Kazan
 * Da Species Update :D
 *
 * Revision 2.17  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.16  2003/09/13 06:02:06  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.14  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.13  2003/08/05 23:45:18  bobboau
 * glow maps, for some reason they wern't in here, they should be now,
 * also there is some debug code for changeing the FOV in game,
 * and I have some changes to the init code to try and get a 32 or 24 bit back buffer
 * if posable, this may cause problems for people
 * the changes were all marked and if needed can be undone
 *
 * Revision 2.12  2003/07/15 02:35:59  phreak
 * added a debug key to cloak targeted ship ~+shift+x
 *
 * Revision 2.11  2003/07/04 02:26:15  phreak
 * tilde + x cloaks a ship
 *
 * Revision 2.10  2003/04/29 01:03:23  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.9  2003/01/18 10:00:43  Goober5000
 * added "no-subspace-drive" flag for ships
 * --Goober5000
 *
 * Revision 2.8  2003/01/03 21:58:08  Goober5000
 * Fixed some minor bugs, and added a primitive-sensors flag, where if a ship
 * has primitive sensors it can't target anything and objects don't appear
 * on radar if they're outside a certain range.  This range can be modified
 * via the sexp primitive-sensors-set-range.
 * --Goober5000
 *
 * Revision 2.7  2002/12/31 18:59:43  Goober5000
 * if it ain't broke, don't fix it
 * --Goober5000
 *
 * Revision 2.5  2002/12/10 05:43:34  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.4  2002/10/19 03:50:29  randomtiger
 * Added special pause mode for easier action screenshots.
 * Added new command line parameter for accessing all single missions in tech room. - RT
 *
 * Revision 2.3  2002/10/17 20:40:51  randomtiger
 * Added ability to remove HUD ingame on keypress shift O
 * So I've added a new key to the bind list and made use of already existing hud removal code.
 *
 * Revision 2.2  2002/08/06 16:49:22  phreak
 * added keybing for wireframe hud,
 * fixed previous missile cheat
 *
 *
 * Revision 2.2  2002/08/04 23:15:40  PhReAk
 * Fixed previous missile cheat, commented out beam_test(x) lines
 * in process_debug_keys().  beam_test(x) caused assertion failure in
 * beam.cpp apprx line 3147.  Also added toggle for wire_hud in process_player_ship_keys()
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.5  2002/05/26 14:13:19  mharris
 * Debugging
 *
 * Revision 1.4  2002/05/24 16:45:45  mharris
 * Added some debug mprintfs
 *
 * Revision 1.3  2002/05/17 03:03:14  mharris
 * Porting fixes: changes to matrix and vector structs; added
 * NO_NETWORK and NO_SOUND where appropriate.
 *
 * Revision 1.2  2002/05/13 21:43:38  mharris
 * A little more network and sound cleanup
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 47    9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 46    9/07/99 11:26p Andsager
 * Fix "r" targeting key, making evaluate_ship_as_closest_target() and
 * hud_target_live_turret() consider if turret is targeting player
 * 
 * 45    9/03/99 1:31a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 44    9/01/99 10:09a Dave
 * Pirate bob.
 * 
 * 43    8/27/99 10:36a Dave
 * Impose a 2% penalty for hitting the shield balance key.
 * 
 * 42    8/27/99 9:57a Dave
 * Enabled standard cheat codes. Allow player to continue in a campaing
 * after using cheat codes.
 * 
 * 41    8/26/99 9:45a Dave
 * First pass at easter eggs and cheats.
 * 
 * 40    8/24/99 1:49a Dave
 * Fixed client-side afterburner stuttering. Added checkbox for no version
 * checking on PXO join. Made button info passing more friendly between
 * client and server.
 * 
 * 39    8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 38    8/19/99 10:59a Dave
 * Packet loss detection.
 * 
 * 37    8/18/99 12:09p Andsager
 * Add debug if message has no anim for message.  Make messages come from
 * wing leader.
 * 
 * 36    8/05/99 2:05a Dave
 * Whee.
 * 
 * 35    8/01/99 12:39p Dave
 * Added HUD contrast control key (for nebula).
 * 
 * 34    7/31/99 2:30p Dave
 * Added nifty mission message debug viewing keys.
 * 
 * 33    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 32    7/15/99 4:09p Andsager
 * Disable cheats for FS2_DEMO
 * 
 * 31    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 30    7/07/99 3:32p Dave
 * Oops. Forgot to remove this.
 * 
 * 29    7/02/99 4:31p Dave
 * Much more sophisticated lightning support.
 * 
 * 28    6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 27    6/09/99 2:55p Andsager
 * Allow multiple asteroid subtypes (of large, medium, small) and follow
 * family.
 * 
 * 26    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 25    5/08/99 8:25p Dave
 * Upped object pairs. First run of nebula lightning.
 * 
 * 24    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 23    5/03/99 9:07a Dave
 * Pirate Bob. Changed beam test code a bit.
 * 
 * 22    4/21/99 6:15p Dave
 * Did some serious housecleaning in the beam code. Made it ready to go
 * for anti-fighter "pulse" weapons. Fixed collision pair creation. Added
 * a handy macro for recalculating collision pairs for a given object.
 * 
 * 21    4/16/99 5:54p Dave
 * Support for on/off style "stream" weapons. Real early support for
 * target-painting lasers.
 * 
 * 20    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 19    3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 18    3/28/99 5:58p Dave
 * Added early demo code. Make objects move. Nice and framerate
 * independant, but not much else. Don't use yet unless you're me :)
 * 
 * 17    3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 * 
 * 16    2/21/99 6:01p Dave
 * Fixed standalone WSS packets. 
 * 
 * 15    2/21/99 1:48p Dave
 * Some code for monitoring datarate for multiplayer in detail.
 * 
 * 14    1/21/99 2:06p Dave
 * Final checkin for multiplayer testing.
 * 
 * 13    1/19/99 3:57p Andsager
 * Round 2 of variables
 * 
 * 12    1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 11    1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 10    12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 9     12/04/98 3:37p Andsager
 * Added comment out asteroid launcher
 * 
 * 8     11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 7     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 6     10/26/98 9:42a Dave
 * Early flak gun support.
 * 
 * 5     10/20/98 1:39p Andsager
 * Make so sparks follow animated ship submodels.  Modify
 * ship_weapon_do_hit_stuff() and ship_apply_local_damage() to add
 * submodel_num.  Add submodel_num to multiplayer hit packet.
 * 
 * 4     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 387   6/17/98 11:02a Lawrance
 * show what cheat code is in the comment
 * 
 * 386   6/09/98 5:15p Lawrance
 * French/German localization
 * 
 * 385   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 384   6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 383   5/24/98 1:46p Mike
 * Final cheat codes.
 * 
 * 382   5/19/98 2:20p Mike
 * Comment out nprintf().
 * 
 * 381   5/19/98 12:19p Mike
 * Cheat codes!
 * 
 * 380   5/19/98 11:11a Lawrance
 * Make 'G' only target hostiles
 * 
 * 379   5/18/98 11:00p Mike
 * Adding support for cheat system.
 * 
 * 378   5/18/98 12:41a Allender
 * fixed subsystem problems on clients (i.e. not reporting properly on
 * damage indicator).  Fixed ingame join problem with respawns.  minor
 * comm menu stuff
 * 
 * 377   5/17/98 1:43a Dave
 * Eradicated chatbox problems. Remove speed match for observers. Put in
 * help screens for PXO. Fix messaging and end mission privelges. Fixed
 * team select screen bugs. Misc UI fixes.
 * 
 * 376   5/15/98 8:36p Lawrance
 * Add 'target ship that last sent transmission' target key
 * 
 * 375   5/14/98 11:07a Lawrance
 * Ensure looped sounds get stopped before stopping all channels
 * 
 * 374   5/12/98 11:59p Dave
 * Put in some more functionality for Parallax Online.
 * 
 * 373   5/11/98 5:29p Hoffoss
 * Added mouse button mapped to joystick button support.
 * 
 * 372   5/08/98 10:14a Lawrance
 * Play sound when auto-targeting gets toggled.
 * 
 * 371   5/04/98 9:25a Allender
 * don't allow time compression in multiplayer
 * 
 * 370   4/30/98 4:43p Allender
 * trap player obj when changing dual fire status on ship
 * 
 * 369   4/30/98 4:16p Peter
 * fixes for critical button functions when a player (client) ship is dead
 * or dying
 * 
 * 368   4/27/98 9:03a Dave
 * Fixed a multiplayer sequencing bug where paused players who were in the
 * options screen got an Assert when unpausing. Removed an optimiized
 * build warning in keycontrol. 
 * 
 * 367   4/26/98 4:29p Lawrance
 * Put back time compression keys... somehow they were trashed by a later
 * checkin.
 * 
 * 366   4/25/98 5:36p Mike
 * Prevent player warpout if engine < 10%.
 * 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "globalincs/linklist.h"
#include "io/key.h"
#include "io/joy.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "hud/hud.h"
#include "gamesequence/gamesequence.h"
#include "mission/missiongoals.h"
#include "hud/hudets.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudsquadmsg.h"
#include "gamesnd/eventmusic.h"
#include "freespace2/freespace.h"
#include "mission/missionhotkey.h"
#include "hud/hudescort.h"
#include "hud/hudshield.h"
#include "io/keycontrol.h"
#include "ship/shiphit.h"
#include "ship/shipfx.h"
#include "mission/missionlog.h"
#include "hud/hudtargetbox.h"
#include "popup/popup.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "hud/hudconfig.h"
#include "hud/hudmessage.h"
#include "network/multi_pmsg.h"
#include "globalincs/crypt.h"
#include "starfield/supernova.h"
#include "mission/missionmessage.h"
#include "menuui/mainhallmenu.h"
#include "missionui/missionpause.h"
#include "hud/hudgauges.h"
#include "freespace2/freespace.h"	//For time compression stuff

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_pause.h"
#include "network/multi_observer.h"
#include "network/multi_endgame.h"
#endif

#if defined(ENABLE_AUTO_PILOT)
#include "autopilot/autopilot.h"
#endif

// --------------------------------------------------------------
// Global to file 
// --------------------------------------------------------------

// --------------------------------------------------------------
// Externals
// --------------------------------------------------------------
typedef	struct asteroid_field {
	vec3d	min_bound;						//	Minimum range of field.
	vec3d	max_bound;						//	Maximum range of field.
	vec3d	vel;								//	Average asteroid moves at this velocity.
	float		speed;							// Average speed of field
	int		num_initial_asteroids;		//	Number of asteroids at creation.
} asteroid_field;

// time compression/dilation values - Goober5000
// (Volition sez "can't compress below 0.25"... not sure if
// this is arbitrary or dictated by code)
#define MAX_TIME_MULTIPLIER		64
#define MAX_TIME_DIVIDER		4

#define CHEAT_BUFFER_LEN	20
#define CHEATSPOT				(CHEAT_BUFFER_LEN - 1)

char CheatBuffer[CHEAT_BUFFER_LEN+1];

#ifdef FS2_DEMO
	char *Cheat_code_demo = NOX("33BE^(8]C01(:=BHt");
#else
	char *Cheat_code = NOX("33BE^(8]C01(:=BHt");					// www.freespace2.com
	char *Cheat_code_fish = NOX("bDc9y+$;#AIDRoouM");			// vasudanswuvfishes
	char *Cheat_code_headz = NOX("!;:::@>F7L?@@2:@A");			// humanheadsinside.
	char *Cheat_code_tooled = NOX("sipp-^rM@L!U^usjX");		// tooledworkedowned
	char *Cheat_code_pirate = NOX("MAP4YP[4=-2uC(yJ^");		// arrrrwalktheplank	
	char *Cheat_code_skip = NOX("7!ICkSI\"(8n3JesBP");			// skipmemymissionyo
#endif
									  // 666)6=N79+Z45=BE0e
int Tool_enabled = 0;
bool Perspective_locked=false;

	/*
#else 
	// list of the cheat codes
	//#ifdef INTERPLAYQA
	// "DavidPerry" NOX("0!XZQ*K.pu");
	// NOX("&BvWJe=a?$VP*=@2W,2Y");	// Super-secret 20 character string!
	//NOX("STs`nHqW\\lv#KD_aCSWN");	//	solveditonceandforall (note double \\ as string contains \.
	//XSTR:OFF
	char *Cheat_code_in_game = NOX("///FES)P<A5=7CCB!n10");	//	www.volition-inc.com
	char *Cheat_code_movies =  NOX("&BvWJe=a?$VP*=@2W,2Y");	// freespacestandsalone
	char *Cheat_code_pirate = NOX("%,sPzoE>\\+_(Qs#+h-8o");				// arrwalktheplankmatey
	//XSTR:ON
#endif
	*/

//int All_movies_enabled = 0;

//int Debug_allowed = 0;

//#endif

extern int AI_watch_object;
extern int Countermeasures_enabled;

extern float do_subobj_hit_stuff(object *ship_obj, object *other_obj, vec3d *hitpos, float damage);

extern void mission_goal_mark_all_true( int type );

int Normal_key_set[] = {
	TARGET_NEXT,
	TARGET_PREV,
	TARGET_NEXT_CLOSEST_HOSTILE,
	TARGET_PREV_CLOSEST_HOSTILE,
	TARGET_NEXT_CLOSEST_FRIENDLY,
	TARGET_PREV_CLOSEST_FRIENDLY,
	TARGET_TARGETS_TARGET,
	TARGET_SHIP_IN_RETICLE,
	TARGET_LAST_TRANMISSION_SENDER,
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,
	TARGET_CLOSEST_SHIP_ATTACKING_SELF,
	STOP_TARGETING_SHIP,
	TOGGLE_AUTO_TARGETING,
	TARGET_SUBOBJECT_IN_RETICLE,
	TARGET_PREV_SUBOBJECT,
	TARGET_NEXT_SUBOBJECT,
	STOP_TARGETING_SUBSYSTEM,

	TARGET_NEXT_UNINSPECTED_CARGO,
	TARGET_PREV_UNINSPECTED_CARGO,
	TARGET_NEWEST_SHIP,
	TARGET_NEXT_LIVE_TURRET,
	TARGET_PREV_LIVE_TURRET,
	TARGET_NEXT_BOMB,
	TARGET_PREV_BOMB,

	ATTACK_MESSAGE,
	DISARM_MESSAGE,
	DISABLE_MESSAGE,
	ATTACK_SUBSYSTEM_MESSAGE,
	CAPTURE_MESSAGE,
	ENGAGE_MESSAGE,
	FORM_MESSAGE,
	PROTECT_MESSAGE,
	COVER_MESSAGE,
	WARP_MESSAGE,
	REARM_MESSAGE,
	IGNORE_MESSAGE,
	SQUADMSG_MENU,

	VIEW_CHASE,
	VIEW_OTHER_SHIP,
	VIEW_TOPDOWN,

	SHOW_GOALS,
	END_MISSION,

	ADD_REMOVE_ESCORT,
	ESCORT_CLEAR,
	TARGET_NEXT_ESCORT_SHIP,

	XFER_SHIELD,
	XFER_LASER,
	INCREASE_SHIELD,
	INCREASE_WEAPON,
	INCREASE_ENGINE,
	DECREASE_SHIELD,
	DECREASE_WEAPON,
	DECREASE_ENGINE,
	ETS_EQUALIZE,
	SHIELD_EQUALIZE,
	SHIELD_XFER_TOP,
	SHIELD_XFER_BOTTOM,
	SHIELD_XFER_RIGHT,
	SHIELD_XFER_LEFT,

	CYCLE_NEXT_PRIMARY,
	CYCLE_PREV_PRIMARY,
	CYCLE_SECONDARY,
	CYCLE_NUM_MISSLES,
	RADAR_RANGE_CYCLE,

	MATCH_TARGET_SPEED,
	TOGGLE_AUTO_MATCH_TARGET_SPEED,

	VIEW_EXTERNAL,
	VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,
	LAUNCH_COUNTERMEASURE,
	ONE_THIRD_THROTTLE,
	TWO_THIRDS_THROTTLE,
	PLUS_5_PERCENT_THROTTLE,
	MINUS_5_PERCENT_THROTTLE,
	ZERO_THROTTLE,
	MAX_THROTTLE,

	TARGET_CLOSEST_REPAIR_SHIP,

	MULTI_MESSAGE_ALL,
	MULTI_MESSAGE_FRIENDLY,
	MULTI_MESSAGE_HOSTILE,
	MULTI_MESSAGE_TARGET,
	MULTI_OBSERVER_ZOOM_TO,

	TIME_SPEED_UP,
	TIME_SLOW_DOWN,

	TOGGLE_HUD_CONTRAST,

	MULTI_TOGGLE_NETINFO,
	MULTI_SELF_DESTRUCT,

	TOGGLE_HUD,
#if defined(ENABLE_AUTO_PILOT)
	HUD_TARGETBOX_TOGGLE_WIREFRAME,
	AUTO_PILOT_TOGGLE,
	NAV_CYCLE
#else
	HUD_TARGETBOX_TOGGLE_WIREFRAME
#endif

};

int Dead_key_set[] = {
	TARGET_NEXT,
	TARGET_PREV,
	TARGET_NEXT_CLOSEST_HOSTILE,
	TARGET_PREV_CLOSEST_HOSTILE,
	TARGET_NEXT_CLOSEST_FRIENDLY,
	TARGET_PREV_CLOSEST_FRIENDLY,
	TARGET_TARGETS_TARGET,
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,
	STOP_TARGETING_SHIP,
	TOGGLE_AUTO_TARGETING,
	TARGET_SUBOBJECT_IN_RETICLE,
	TARGET_PREV_SUBOBJECT,
	TARGET_NEXT_SUBOBJECT,
	STOP_TARGETING_SUBSYSTEM,
	TARGET_NEWEST_SHIP,
	TARGET_NEXT_LIVE_TURRET,
	TARGET_PREV_LIVE_TURRET,
	TARGET_NEXT_BOMB,
	TARGET_PREV_BOMB,

	VIEW_CHASE,
	VIEW_OTHER_SHIP,
	VIEW_TOPDOWN,

	SHOW_GOALS,

	ADD_REMOVE_ESCORT,
	ESCORT_CLEAR,
	TARGET_NEXT_ESCORT_SHIP,
	TARGET_CLOSEST_REPAIR_SHIP,	

	MULTI_MESSAGE_ALL,
	MULTI_MESSAGE_FRIENDLY,
	MULTI_MESSAGE_HOSTILE,
	MULTI_MESSAGE_TARGET,
	MULTI_OBSERVER_ZOOM_TO,

	TIME_SPEED_UP,
	TIME_SLOW_DOWN
};

int Critical_key_set[] = {		
	CYCLE_SECONDARY,			
	CYCLE_NUM_MISSLES,			
	INCREASE_WEAPON,			
	DECREASE_WEAPON,	
	INCREASE_SHIELD,			
	DECREASE_SHIELD,			
	INCREASE_ENGINE,			
	DECREASE_ENGINE,			
	ETS_EQUALIZE,
	SHIELD_EQUALIZE,			
	SHIELD_XFER_TOP,			
	SHIELD_XFER_BOTTOM,			
	SHIELD_XFER_LEFT,			
	SHIELD_XFER_RIGHT,			
	XFER_SHIELD,			
	XFER_LASER,			
};

int Non_critical_key_set[] = {
	CYCLE_NEXT_PRIMARY,		
	CYCLE_PREV_PRIMARY,			
	MATCH_TARGET_SPEED,			
	TOGGLE_AUTO_MATCH_TARGET_SPEED,			
	TARGET_NEXT,	
	TARGET_PREV,			
	TARGET_NEXT_CLOSEST_HOSTILE,			
	TARGET_PREV_CLOSEST_HOSTILE,			
	TOGGLE_AUTO_TARGETING,			
	TARGET_NEXT_CLOSEST_FRIENDLY,			
	TARGET_PREV_CLOSEST_FRIENDLY,			
	TARGET_SHIP_IN_RETICLE,			
	TARGET_LAST_TRANMISSION_SENDER,
	TARGET_CLOSEST_REPAIR_SHIP,			
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,			
	STOP_TARGETING_SHIP,			
	TARGET_CLOSEST_SHIP_ATTACKING_SELF,			
	TARGET_TARGETS_TARGET,			
	TARGET_SUBOBJECT_IN_RETICLE,			
	TARGET_PREV_SUBOBJECT,			
	TARGET_NEXT_SUBOBJECT,			
	STOP_TARGETING_SUBSYSTEM,
	TARGET_NEXT_BOMB,
	TARGET_PREV_BOMB,
	TARGET_NEXT_UNINSPECTED_CARGO,
	TARGET_PREV_UNINSPECTED_CARGO,
	TARGET_NEWEST_SHIP,
	TARGET_NEXT_LIVE_TURRET,
	TARGET_PREV_LIVE_TURRET,
	ATTACK_MESSAGE,
	DISARM_MESSAGE,
	DISABLE_MESSAGE,
	ATTACK_SUBSYSTEM_MESSAGE,
	CAPTURE_MESSAGE,
	ENGAGE_MESSAGE,
   FORM_MESSAGE,
	PROTECT_MESSAGE,
	COVER_MESSAGE,
	WARP_MESSAGE,
	IGNORE_MESSAGE,
	REARM_MESSAGE,
	VIEW_CHASE,
	VIEW_EXTERNAL,
	VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,
	VIEW_OTHER_SHIP,
	VIEW_TOPDOWN,
	RADAR_RANGE_CYCLE,
	SQUADMSG_MENU,
	SHOW_GOALS,
	END_MISSION,
	ADD_REMOVE_ESCORT,
	ESCORT_CLEAR,
	TARGET_NEXT_ESCORT_SHIP,	
	MULTI_MESSAGE_ALL,
	MULTI_MESSAGE_FRIENDLY,
	MULTI_MESSAGE_HOSTILE,
	MULTI_MESSAGE_TARGET,
	MULTI_OBSERVER_ZOOM_TO,			
	TOGGLE_HUD_CONTRAST,

	MULTI_TOGGLE_NETINFO,
	MULTI_SELF_DESTRUCT,

	TOGGLE_HUD,
#if defined(ENABLE_AUTO_PILOT)
	HUD_TARGETBOX_TOGGLE_WIREFRAME,
	AUTO_PILOT_TOGGLE,
	NAV_CYCLE
#else
	HUD_TARGETBOX_TOGGLE_WIREFRAME
#endif

};


// set sizes of the key sets automatically
int Normal_key_set_size = sizeof(Normal_key_set) / sizeof(int);
int Dead_key_set_size = sizeof(Dead_key_set) / sizeof(int);
int Critical_key_set_size = sizeof(Critical_key_set) / sizeof(int);
int Non_critical_key_set_size = sizeof(Non_critical_key_set) / sizeof(int);

// --------------------------------------------------------------
// routine to process keys used only for debugging
// --------------------------------------------------------------
//#ifndef NDEBUG

void debug_cycle_player_ship(int delta)
{
	if ( Player_obj == NULL )
		return;

	int si_index = Ships[Player_obj->instance].ship_info_index;
	int sanity = 0;
	ship_info	*sip;
	while ( TRUE ) {
		si_index += delta;
		if ( si_index > Num_ship_types ){
			si_index = 0;
		}
		if ( si_index < 0 ){
			si_index = Num_ship_types - 1;
		}
		sip = &Ship_info[si_index];
		if ( sip->flags & SIF_PLAYER_SHIP ){
			break;
		}

		// just in case
		sanity++;
		if ( sanity > Num_ship_types ){
			break;
		}
	}

	change_ship_type(Player_obj->instance, si_index);
	HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Player ship changed to %s", 0), Ship_info[si_index].name);			
}

// cycle targeted ship to next ship in that species
void debug_cycle_targeted_ship(int delta)
{
	object		*objp;
	ship_info	*sip;
	int			si_index, species;
	char			name[NAME_LENGTH];

	if ( Player_ai->target_objnum == -1 )
		return;

	objp = &Objects[Player_ai->target_objnum];
	if ( objp->type != OBJ_SHIP )
		return;

	si_index = Ships[objp->instance].ship_info_index;
	Assert(si_index != -1 );
	species = Ship_info[si_index].species;

	int sanity = 0;

	while ( TRUE ) {
		si_index += delta;
		if ( si_index > Num_ship_types )
			si_index = 0;
		if ( si_index < 0 )
			si_index = Num_ship_types-1;

	
		sip = &Ship_info[si_index];
	
		// if it has test in the name, jump over it
		strcpy(name, sip->name);
		_strlwr(name);
		if ( strstr(name,NOX("test")) != NULL )
			continue;

		if ( sip->species == species && (sip->flags & (SIF_FIGHTER | SIF_BOMBER | SIF_TRANSPORT) ) )
			break;

		// just in case
		sanity++;
		if ( sanity > Num_ship_types )
			break;
	}

	change_ship_type(objp->instance, si_index);
	HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Changed player target to %s", 1), Ship_info[si_index].name);			
}

void debug_max_secondary_weapons(object *objp)
{
	int index;
	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];
	ship_weapon *swp = &shipp->weapons;

	for ( index = 0; index < MAX_SHIP_SECONDARY_BANKS; index++ )
	{
		swp->secondary_bank_ammo[index] = sip->secondary_bank_ammo_capacity[index];
	}
}

void debug_max_primary_weapons(object *objp)	// Goober5000
{
	Assert(objp);	// Goober5000

	int index;
	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];
	ship_weapon *swp = &shipp->weapons;
	weapon_info *wip;

	if (sip->flags & SIF_BALLISTIC_PRIMARIES)
	{
		for ( index = 0; index < MAX_SHIP_PRIMARY_BANKS; index++ )
		{
			wip = &Weapon_info[swp->primary_bank_weapons[index]];
			if (wip->wi_flags2 & WIF2_BALLISTIC)
			{
				swp->primary_bank_ammo[index] = sip->primary_bank_ammo_capacity[index];
			}
		}
	}
}

void debug_change_song(int delta)
{
#ifndef NO_SOUND
	char buf[256];
	if ( event_music_next_soundtrack(delta) != -1 ) {
		event_music_get_soundtrack_name(buf);
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Soundtrack changed to: %s", 2), buf);

	} else {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Event music is not playing", 3));
	}
#endif
}

//extern void set_global_ignore_object(int objnum);

extern void hud_target_asteroid();
extern int Framerate_delay;

extern void snd_stop_any_sound();

extern float Viewer_zoom;
extern vec3d Eye_position;
extern matrix Eye_matrix;
extern void g3_set_view_matrix(vec3d *view_pos,matrix *view_matrix,float zoom);

extern int Show_cpu;

int get_prev_weapon_looped(int current_weapon, int subtype)
{
	int i, new_index;

	for (i = 1; i < Num_weapon_types; i++)
	{
		new_index = (Num_weapon_types + current_weapon - i) % Num_weapon_types;

		if(Weapon_info[new_index].subtype == subtype)
		{
			return new_index;
		}
	}

	return current_weapon;
}

int get_next_weapon_looped(int current_weapon, int subtype)
{
	int i, new_index;

	for (i = 1; i < Num_weapon_types; i++)
	{
		new_index = (current_weapon + i) % Num_weapon_types;

		if(Weapon_info[new_index].subtype == subtype)
		{
			return new_index;
		}
	}

	return current_weapon;
}

void process_debug_keys(int k)
{
#ifdef INTERPLAYQA
	if ( !Debug_allowed )
		return;
#endif

	// Kazan -- NO CHEATS IN MULTIb
	if (Game_mode & GM_MULTIPLAYER)
	{
		return;
	}

	// if ( (k & KEY_DEBUGGED) && (Game_mode & GM_RECORDING_DEMO) )
		// return;

	switch (k) {
		case KEY_DEBUGGED + KEY_H:
			hud_target_toggle_hidden_from_sensors();
			break;

		case KEY_DEBUGGED + KEY_F: 
			/*
			int i;
			for (i=0; i<NUM_HUD_GAUGES; i++) {
				hud_gauge_start_flash(i);
			}
			*/
			extern int wacky_scheme;
			if(wacky_scheme == 3){
				wacky_scheme = 0;
			} else {
				wacky_scheme++;
			}
			break;
		
		case KEY_DEBUGGED + KEY_ALTED + KEY_F:
			Framerate_delay += 10;
			HUD_printf(XSTR( "Framerate delay increased to %i milliseconds per frame.", 4), Framerate_delay);
			break;

		case KEY_DEBUGGED + KEY_ALTED + KEY_SHIFTED + KEY_F:
			Framerate_delay -= 10;
			if (Framerate_delay < 0)
				Framerate_delay = 0;

			HUD_printf(XSTR( "Framerate delay decreased to %i milliseconds per frame.", 5), Framerate_delay);
			break;

		case KEY_DEBUGGED + KEY_X:
		case KEY_DEBUGGED1 + KEY_X:
			HUD_printf("Cloaking has been disabled, thank you for playing fs2_open, %s", Player->callsign);
			break;

		case KEY_DEBUGGED + KEY_C:
		case KEY_DEBUGGED1 + KEY_C:
			// hud_enemymsg_toggle();
			if(Player_obj->flags & OF_COLLIDES){
				obj_set_flags(Player_obj, Player_obj->flags & ~(OF_COLLIDES));
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Player no longer collides");
			} else {
				obj_set_flags(Player_obj, Player_obj->flags | OF_COLLIDES);
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Player collides");
			}
			break;

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_C:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_C:
			Countermeasures_enabled = !Countermeasures_enabled;
			HUD_printf(XSTR( "Countermeasure firing: %s", 6), Countermeasures_enabled ? XSTR( "ENABLED", 7) : XSTR( "DISABLED", 8));
			break;

// Goober5000: this should only be compiled in debug builds, since it crashes release
#ifndef NDEBUG
		case KEY_DEBUGGED + KEY_E:
			gameseq_post_event(GS_EVENT_EVENT_DEBUG);
			break;
#endif

		// Goober5000: handle time dilation in cheat section
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_COMMA:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_COMMA:
			if ( Game_mode & GM_NORMAL ) {
				if ( Game_time_compression > (F1_0/MAX_TIME_DIVIDER) ) {
					change_time_compression(0.5);
				} else {
					gamesnd_play_error_beep();
				}
			} else {
				gamesnd_play_error_beep();
			}
			break;

		// Goober5000: handle as normal here
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_PERIOD:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_PERIOD:
			if ( Game_mode & GM_NORMAL ) {
				if ( Game_time_compression < (F1_0*MAX_TIME_MULTIPLIER) ) {
					change_time_compression(2);
				} else {
					gamesnd_play_error_beep();
				}
			} else {
				gamesnd_play_error_beep();
			}
			break;

		//	Kill! the currently targeted ship.
		case KEY_DEBUGGED + KEY_K:
		case KEY_DEBUGGED1 + KEY_K:
			if (Player_ai->target_objnum != -1) {
				object	*objp = &Objects[Player_ai->target_objnum];

				switch (objp->type) {
				case OBJ_SHIP:
					
					// remove guardian flag -- kazan
					Ships[objp->instance].ship_guardian_threshold = 0;
					
					ship_apply_local_damage( objp, Player_obj, &objp->pos, 100000.0f, MISS_SHIELDS, CREATE_SPARKS);
					ship_apply_local_damage( objp, Player_obj, &objp->pos, 1.0f, MISS_SHIELDS, CREATE_SPARKS);
					break;
				case OBJ_WEAPON:
					Weapons[objp->instance].lifeleft = 0.01f;
					break;
				}
			}

			break;
		
		// play the next mission message
		case KEY_DEBUGGED + KEY_V:		
			extern int Message_debug_index;
			extern int Num_messages_playing;
			// stop any other messages
			if(Num_messages_playing){
				message_kill_all(1);
			}

			// next message
			if(Message_debug_index >= Num_messages - 1){
				Message_debug_index = Num_builtin_messages;
			} else {
				Message_debug_index++;
			}
			
			// play the message
			message_send_unique_to_player( Messages[Message_debug_index].name, Message_waves[Messages[Message_debug_index].wave_info.index].name, MESSAGE_SOURCE_SPECIAL, MESSAGE_PRIORITY_HIGH, 0, 0 );			
			if (Messages[Message_debug_index].avi_info.index == -1) {
				HUD_printf("No anim set for message \"%s\"; None will play!", Messages[Message_debug_index].name);
			}
			break;

		// play the previous mission message
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_V:
			extern int Message_debug_index;
			extern int Num_messages_playing;
			// stop any other messages
			if(Num_messages_playing){
				message_kill_all(1);
			}

			// go maybe go down one
			if(Message_debug_index == Num_builtin_messages - 1){
				Message_debug_index = Num_builtin_messages;
			} else if(Message_debug_index > Num_builtin_messages){
				Message_debug_index--;
			}
			
			// play the message
			message_send_unique_to_player( Messages[Message_debug_index].name, Message_waves[Messages[Message_debug_index].wave_info.index].name, MESSAGE_SOURCE_SPECIAL, MESSAGE_PRIORITY_HIGH, 0, 0 );
			if (Messages[Message_debug_index].avi_info.index == -1) {
				HUD_printf("No avi associated with this message; None will play!");
			}
			break;

		// reset to the beginning of mission messages
		case KEY_DEBUGGED + KEY_ALTED + KEY_V:
			extern int Message_debug_index;
			Message_debug_index = Num_builtin_messages - 1;
			HUD_printf("Resetting to first mission message");
			break;

		//	Kill! the currently targeted ship.
		case KEY_DEBUGGED + KEY_ALTED + KEY_SHIFTED + KEY_K:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_SHIFTED + KEY_K:
			if (Player_ai->target_objnum != -1) {
				object	*objp = &Objects[Player_ai->target_objnum];

				if (objp->type == OBJ_SHIP) {
					ship_apply_local_damage( objp, Player_obj, &objp->pos, Ships[objp->instance].ship_max_hull_strength * 0.1f + 10.0f, MISS_SHIELDS, CREATE_SPARKS);
				}
			}
			break;

			//	Kill the currently targeted subsystem.
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_K:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_K:
			if ((Player_ai->target_objnum != -1) && (Player_ai->targeted_subsys != NULL)) {
				object	*objp = &Objects[Player_ai->target_objnum];
				if ( objp->type == OBJ_SHIP ) {
					ship		*sp = &Ships[objp->instance];
					vec3d	g_subobj_pos;

					get_subsystem_world_pos(objp, Player_ai->targeted_subsys, &g_subobj_pos);

					do_subobj_hit_stuff(objp, Player_obj, &g_subobj_pos, (float) -Player_ai->targeted_subsys->system_info->type); //100.0f);

					if ( sp->subsys_info[SUBSYSTEM_ENGINE].current_hits <= 0.0f ) {
						mission_log_add_entry(LOG_SHIP_DISABLED, sp->ship_name, NULL );
						sp->flags |= SF_DISABLED;				// add the disabled flag
					}

					if ( sp->subsys_info[SUBSYSTEM_TURRET].current_hits <= 0.0f ) {
						mission_log_add_entry(LOG_SHIP_DISARMED, sp->ship_name, NULL );
						// sp->flags |= SF_DISARMED;				// add the disarmed flag
					}
				}
			}
			break;

		case KEY_DEBUGGED + KEY_ALTED + KEY_K:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_K:
			{
				float	shield, integrity;
				vec3d	pos, randvec;

				vm_vec_rand_vec_quick(&randvec);
				vm_vec_scale_add(&pos, &Player_obj->pos, &randvec, Player_obj->radius);
			ship_apply_local_damage(Player_obj, Player_obj, &pos, 25.0f, MISS_SHIELDS, CREATE_SPARKS);
			hud_get_target_strength(Player_obj, &shield, &integrity);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "You knocked yourself down to %7.3f percent hull.\n", 9), 100.0f * integrity);
			break;
			}
			
		//	Whack down the player's shield and hull by a little more than 50%
		//	Select next object to be viewed by AI.
		case KEY_DEBUGGED + KEY_I:
		case KEY_DEBUGGED1 + KEY_I:
			Player_obj->flags ^= OF_INVULNERABLE;
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "You are %s", 10), Player_obj->flags & OF_INVULNERABLE ? XSTR( "now INVULNERABLE!", 11) : XSTR( "no longer invulnerable...", 12));
			break;

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_I:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_I:
			if (Player_ai->target_objnum != -1) {
				object	*objp = &Objects[Player_ai->target_objnum];

				objp->flags ^= OF_INVULNERABLE;
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Player's target [%s] is %s", 13), Ships[objp->instance].ship_name, objp->flags & OF_INVULNERABLE ? XSTR( "now INVULNERABLE!", 11) : XSTR( "no longer invulnerable...", 12));
			}
			break;
/*
		case KEY_DEBUGGED + KEY_ALTED + KEY_I:
			if (Player_ai->target_objnum != -1)
				set_global_ignore_object(Player_ai->target_objnum);
			break;
*/

		case KEY_DEBUGGED + KEY_N:
			AI_watch_object++;
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Spewing debug info about object #%d", 14), AI_watch_object);
			break;

		case KEY_DEBUGGED + KEY_O:
		case KEY_DEBUGGED1 + KEY_O:
			toggle_player_object();
			break;				

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_O:
			extern int Debug_octant;
			if(Debug_octant == 7){
				Debug_octant = -1;
			} else {
				Debug_octant++;
			}
			nprintf(("General", "Debug_octant == %d\n", Debug_octant));
			break;

		case KEY_DEBUGGED + KEY_P:
			supernova_start(20);
			break;

		case KEY_DEBUGGED + KEY_W:
		case KEY_DEBUGGED1 + KEY_W:
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_W:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_W:
			// temp code for testing purposes, toggles weapon energy cheat
			Weapon_energy_cheat = !Weapon_energy_cheat;
			if (Weapon_energy_cheat) {
				if (k & KEY_SHIFTED)
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Weapon energy and missile count will always be at full ALL SHIPS!", 15));
				else
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Weapon energy and missile count will always be at full for player", 16));

				debug_max_secondary_weapons(Player_obj);
				debug_max_primary_weapons(Player_obj);
				if (k & KEY_SHIFTED) {
					object	*objp;

					for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
						if (objp->type == OBJ_SHIP)
							debug_max_secondary_weapons(objp);
							debug_max_primary_weapons(objp);
				}

			} else
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Normal weapon energy system / missile count restored", 17));

			break;

		case KEY_DEBUGGED + KEY_G:
		case KEY_DEBUGGED1 + KEY_G:
			mission_goal_mark_all_true( PRIMARY_GOAL );
			break;

		case KEY_DEBUGGED + KEY_G + KEY_SHIFTED:
		case KEY_DEBUGGED1 + KEY_G + KEY_SHIFTED:
			mission_goal_mark_all_true( SECONDARY_GOAL );
			break;

		case KEY_DEBUGGED + KEY_G + KEY_ALTED:
		case KEY_DEBUGGED1 + KEY_G + KEY_ALTED:
			mission_goal_mark_all_true( BONUS_GOAL );
			break;

		case KEY_DEBUGGED + KEY_9: {
		case KEY_DEBUGGED1 + KEY_9:
			ship* shipp;
			
			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank];
			*weap = get_next_weapon_looped(*weap, WP_MISSILE);

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary Weapon forced to %s", 18), Weapon_info[*weap].name);
			break;
		}

			
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_9: {
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_9:
			ship* shipp;

			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank];
			*weap = get_prev_weapon_looped(*weap, WP_MISSILE);

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary Weapon forced to %s", 18), Weapon_info[*weap].name);
			break;
		}
		

#ifndef FS2_DEMO
		case KEY_DEBUGGED + KEY_U: {
		case KEY_DEBUGGED1 + KEY_U:
			// launch asteroid
			extern asteroid_field Asteroid_field;
			object *asteroid_create(asteroid_field *asfieldp, int asteroid_type, int subtype);
			object *objp = asteroid_create(&Asteroid_field, 0, 0);
			vec3d vel;
			vm_vec_copy_scale(&vel, &Player_obj->orient.vec.fvec, 50.0f);
			objp->phys_info.vel = vel;
			objp->phys_info.desired_vel = vel;
			objp->pos = Player_obj->pos;
			//mission_goal_mark_all_true( PRIMARY_GOAL );
			break;
		}
#endif

		case KEY_DEBUGGED + KEY_0: {
		case KEY_DEBUGGED1 + KEY_0:
			ship* shipp;

			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank];
			*weap = get_next_weapon_looped(*weap, WP_LASER);
			
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Primary Weapon forced to %s", 19), Weapon_info[*weap].name);
			break;
		}

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_0: {
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_0:
			ship* shipp;

			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank];
			*weap = get_prev_weapon_looped(*weap, WP_LASER);
		
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Primary Weapon forced to %s", 19), Weapon_info[*weap].name);
			break;
		}

#ifndef NO_SOUND
		case KEY_DEBUGGED + KEY_J: {
			int new_pattern = event_music_return_current_pattern();

			new_pattern++;
			if ( new_pattern >= MAX_PATTERNS )
				new_pattern = 0;

			event_music_change_pattern(new_pattern);
			break;
		}

		case KEY_DEBUGGED + KEY_M: {
			if ( Event_music_enabled ) {
				event_music_disable();
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Event music disabled", 20));

			} else {
				event_music_enable();
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Event music enabled", 21));
			}

			break;
		}
#endif

		case KEY_DEBUGGED + KEY_R: {
		case KEY_DEBUGGED1 + KEY_R:
			if (Player_ai->target_objnum != -1)
				ai_issue_rearm_request(&Objects[Player_ai->target_objnum]);
			else
				ai_issue_rearm_request(Player_obj);

			break;
		}

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_UP:
			Game_detail_level++;
			HUD_printf( XSTR( "Detail level set to %+d\n", 22), Game_detail_level );
			break;

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_DOWN:
			Game_detail_level--;
			HUD_printf( XSTR( "Detail level set to %+d\n", 22), Game_detail_level );
			break;

#ifndef NDEBUG
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_T:	{
			extern int Test_begin;

			if ( Test_begin == 1 )
				break;

			Test_begin = 1;
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Frame Rate test started", 23));

			break;
		}
#endif
		case KEY_DEBUGGED  + KEY_A:	{

			HUD_printf("frame rate currently is %0.2 FPS", 1/flFrametime);

			break;
		}


#ifndef NO_NETWORK
		case KEY_DEBUGGED + KEY_D:
			extern int OO_update_index;			

			if(MULTIPLAYER_MASTER){
				do {
					OO_update_index++;
				} while((OO_update_index < (MAX_PLAYERS-1)) && !MULTI_CONNECTED(Net_players[OO_update_index]));
				if(OO_update_index >= MAX_PLAYERS-1){
					OO_update_index = -1;
				}			
			} else {
				if(OO_update_index < 0){
					OO_update_index = MY_NET_PLAYER_NUM;
				} else {
					OO_update_index = -1;
				}
			}
			break;
#endif

		// change player ship to next flyable type
		case KEY_DEBUGGED + KEY_RIGHT:
			debug_cycle_player_ship(1);
			break;

		// change player ship to previous flyable ship
		case KEY_DEBUGGED + KEY_LEFT:
			debug_cycle_player_ship(-1);
			break;
		
		// cycle target to ship
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_RIGHT:
			debug_cycle_targeted_ship(1);
			break;

		// cycle target to previous ship
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_LEFT:
			debug_cycle_targeted_ship(-1);
			break;

		// change species of the targeted ship
		case KEY_DEBUGGED + KEY_S: {
			if ( Player_ai->target_objnum < 0 )
				break;

			object		*objp;
			ship_info	*sip;

			objp = &Objects[Player_ai->target_objnum];
			if ( objp->type != OBJ_SHIP )
				return;

			sip = &Ship_info[Ships[objp->instance].ship_info_index];
			sip->species++;

			if (sip->species >= True_NumSpecies)
				sip->species = 0;

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Species of target changed to: %s", 24), Species_names[sip->species]);
			break;
		}
			
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_S:
			game_increase_skill_level();
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Skill level set to %s.", 25), Skill_level_names(Game_skill_level));
			break;

		/*// kill all missiles
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_1:
			beam_test(1);
			break;				
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_2:
			beam_test(2);
			break;		
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_3:
			beam_test(3);
			break;				
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_4:
			beam_test(4);
			break;		
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_5:
			beam_test(5);
			break;				
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_6:
			beam_test(6);
			break;		
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_7:
			beam_test(7);
			break;				
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_8:
			beam_test(8);
			break;		
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_9:
			beam_test(9);
			break;				

		case KEY_DEBUGGED + KEY_CTRLED + KEY_1:
			beam_test_new(1);
			break;				
		case KEY_DEBUGGED + KEY_CTRLED + KEY_2:
			beam_test_new(2);
			break;		
		case KEY_DEBUGGED + KEY_CTRLED + KEY_3:
			beam_test_new(3);
			break;*/
					
#ifndef NO_SOUND
		case KEY_DEBUGGED + KEY_T: {
			char buf[256];
			event_music_get_info(buf);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, buf);
			break;
		}

		case KEY_DEBUGGED + KEY_UP:
		case KEY_DEBUGGED1 + KEY_UP:
			debug_change_song(1);
			break;

		case KEY_DEBUGGED + KEY_DOWN:
		case KEY_DEBUGGED1 + KEY_DOWN:
			debug_change_song(-1);
			break;
#endif

		case KEY_PADMINUS: {
			int init_flag = 0;

			if ( keyd_pressed[KEY_1] )	{
				init_flag = 1;
				HUD_color_red -= 4;
			} 

			if ( keyd_pressed[KEY_2] )	{
				init_flag = 1;
				HUD_color_green -= 4;
			} 

			if ( keyd_pressed[KEY_3] )	{
				init_flag = 1;
				HUD_color_blue -= 4;
			} 

			if (init_flag)
				HUD_init_colors();

			break;
		}
		
		case KEY_DEBUGGED + KEY_Y:
			/*
			// blast a debug lightning bolt in front of the player
			vec3d start, strike;
			
			vm_vec_scale_add(&start, &Player_obj->pos, &Player_obj->orient.fvec, 300.0f);
			vm_vec_scale_add2(&start, &Player_obj->orient.rvec, -300.0f);
			vm_vec_scale_add(&strike, &start, &Player_obj->orient.rvec, 600.0f);
			nebl_bolt(DEBUG_BOLT, &start, &strike);
			*/
			extern int tst;
			tst = 2;
			break;

		case KEY_PADPLUS: {
			int init_flag = 0;

			if ( keyd_pressed[KEY_1] )	{
				init_flag = 1;
				HUD_color_red += 4;
			} 

			if ( keyd_pressed[KEY_2] )	{
				init_flag = 1;
				HUD_color_green += 4;
			} 

			if ( keyd_pressed[KEY_3] )	{
				init_flag = 1;
				HUD_color_blue += 4;
			} 

			if (init_flag)
				HUD_init_colors();

			break;
		}
		case KEY_DEBUGGED + KEY_ALTED + KEY_EQUAL:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_EQUAL:
			{
			Viewer_zoom += 0.1f;
			g3_set_view_matrix(&Eye_position, &Eye_matrix, Viewer_zoom);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, "viewer zoom raised to %0.2f" , Viewer_zoom);
			}
			break;

		case KEY_DEBUGGED + KEY_ALTED + KEY_MINUS:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_MINUS:
			{
			Viewer_zoom -= 0.1f;
			g3_set_view_matrix(&Eye_position, &Eye_matrix, Viewer_zoom);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, "viewer zoom lowered to %0.2f" , Viewer_zoom);
			}
			break;
		case KEY_DEBUGGED + KEY_Z:
		case KEY_DEBUGGED1 + KEY_Z:
			{
				Show_cpu = !Show_cpu;
			}
			break;

	}	// end switch

}
//#endif

void ppsk_hotkeys(int k)
{

#ifndef FS2_DEMO

	// use k to check for keys that can have Shift,Ctrl,Alt,Del status
	int hotkey_set;

#ifndef NDEBUG
	k &= ~KEY_DEBUGGED;			// since hitting F11 will set this bit
#endif

	switch (k) {
		case KEY_F5:
		case KEY_F6:
		case KEY_F7:
		case KEY_F8:
		case KEY_F9:
		case KEY_F10:
		case KEY_F11:
		case KEY_F12:
			hotkey_set = mission_hotkey_get_set_num(k);
			if ( !(Players[Player_num].flags & PLAYER_FLAGS_MSG_MODE) )
				hud_target_hotkey_select( hotkey_set );
			else
				hud_squadmsg_hotkey_select( hotkey_set );

			break;

		case KEY_F5 + KEY_SHIFTED:
		case KEY_F6 + KEY_SHIFTED:
		case KEY_F7 + KEY_SHIFTED:
		case KEY_F8 + KEY_SHIFTED:
		case KEY_F9 + KEY_SHIFTED:
		case KEY_F10 + KEY_SHIFTED:
		case KEY_F11 + KEY_SHIFTED:
		case KEY_F12 + KEY_SHIFTED:
			hotkey_set = mission_hotkey_get_set_num(k&(~KEY_SHIFTED));
			mprintf(("Adding to set %d\n", hotkey_set+1));
			if ( Player_ai->target_objnum == -1)
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No target to add/remove from set %d.", 26), hotkey_set+1);
			else  {
				hud_target_hotkey_add_remove( hotkey_set, &Objects[Player_ai->target_objnum], HOTKEY_USER_ADDED);
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "%s added to set %d. (F%d)", 27), Ships[Objects[Player_ai->target_objnum].instance].ship_name, hotkey_set, 4+hotkey_set+1);
			}

			break;

		case KEY_F5 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F6 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F7 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F8 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F9 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F10 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F11 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F12 + KEY_SHIFTED + KEY_ALTED:
			hotkey_set = mission_hotkey_get_set_num(k & ~KEY_SHIFTED+KEY_ALTED);
			hud_target_hotkey_clear( hotkey_set );
			break;

		case KEY_SHIFTED + KEY_MINUS:
			if ( HUD_color_alpha > HUD_COLOR_ALPHA_USER_MIN )	{
				HUD_color_alpha--;
				HUD_init_colors();
			}
			break;
/*		case KEY_SHIFTED + KEY_U:
			{
			object *debris_create(object *source_obj, int model_num, int submodel_num, vec3d *pos, vec3d *exp_center, int hull_flag, float exp_force);

			object *temp = debris_create(Player_obj, Ships[0].modelnum, model_get(Ships[0].modelnum)->debris_objects[0], &Player_obj->pos, &Player_obj->pos, 1, 1.0f);
			if (temp) {
				temp->hull_strength = 5000.0f;
				int objnum = temp - Objects;
				vm_vec_copy_scale(&Objects[objnum].phys_info.vel, &Player_obj->orient.fvec, 30.0f);
			}
			}
			break;
*/

		case KEY_SHIFTED + KEY_EQUAL:
			if ( HUD_color_alpha < HUD_COLOR_ALPHA_USER_MAX ) {
				HUD_color_alpha++;
				HUD_init_colors();
			}
			break;
	}	// end switch

#endif

}

// check keypress 'key' against a set of valid controls and mark the match in the
// player's button info bitfield.  Also checks joystick controls in the set.
//
// key = scancode (plus modifiers).
// count = total size of the list
// list = list of Control_config struct action indices to check for
void process_set_of_keys(int key, int count, int *list)
{
	int i;

	for (i=0; i<count; i++)
		if (check_control(list[i], key))
			button_info_set(&Player->bi, list[i]);
}

// routine to process keys used for player ship stuff (*not* ship movement).
void process_player_ship_keys(int k)
{
	int masked_k;

	masked_k = k & ~KEY_CTRLED;	// take out CTRL modifier only	

	// moved this line to beginning of function since hotkeys now encompass
	// F5 - F12.  We can return after using F11 as a hotkey.
	ppsk_hotkeys(masked_k);
	if (keyd_pressed[KEY_DEBUG_KEY]){
		return;
	}

	// if we're in supernova mode. do nothing
	if(Player->control_mode == PCM_SUPERNOVA){
		return;
	}

	// pass the key to the squadmate messaging code.  If the messaging code took the key, then return
	// from here immediately since we don't want to do further key processing.
	if ( hud_squadmsg_read_key(k) )
		return;

	if ( Player->control_mode == PCM_NORMAL )	{
		//	The following things are not legal to do while dead.
		if ( !(Game_mode & GM_DEAD) ) {
			process_set_of_keys(masked_k, Normal_key_set_size, Normal_key_set);
		} else	{
			process_set_of_keys(masked_k, Dead_key_set_size, Dead_key_set);
		}
	} else {

	}
}

// Handler for when player hits 'ESC' during the game
void game_do_end_mission_popup()
{
	int	pf_flags, choice;
//	char	savegame_filename[_MAX_FNAME];

#ifndef NO_NETWORK
	// do the multiplayer version of this
	if(Game_mode & GM_MULTIPLAYER){
		multi_quit_game(PROMPT_ALL);
	}
	else
#endif
	{

		// single player version....
		// do housekeeping things.
		game_stop_time();
#ifndef NO_SOUND
		game_stop_looped_sounds();
		snd_stop_all();
#endif

		pf_flags = PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON;
		choice = popup(pf_flags, 3, POPUP_NO, XSTR( "&Yes, Quit", 28), XSTR( "Yes, &Restart", 29), XSTR( "Do you really want to end the mission?", 30));

		switch (choice) {
		case 1:
			// save the game before quitting if in campaign mode
			// MWA -- 3/26/98 -- no more save/restore!!!!
/*
			if ( Game_mode & GM_CAMPAIGN_MODE ) {
				memset(savegame_filename, 0, _MAX_FNAME);
				mission_campaign_savefile_generate_root(savegame_filename);
				strcat(savegame_filename, NOX("svg"));
				if ( state_save_all(savegame_filename) ) {
					Int3();	// could not save this game
				}
			}
*/
			gameseq_post_event(GS_EVENT_END_GAME);
			break;

		case 2:
			gameseq_post_event(GS_EVENT_ENTER_GAME);
			break;

		default:
			break;  // do nothing
		}

		game_start_time();
		game_flush();
	}
}

// handle pause keypress
void game_process_pause_key()
{
#ifndef NO_NETWORK
	// special processing for multiplayer
	if (Game_mode & GM_MULTIPLAYER) {							
		if(Multi_pause_status){
			multi_pause_request(0);
		} else {
			multi_pause_request(1);
		}		
	}
	else
#endif
	{
		gameseq_post_event( GS_EVENT_PAUSE_GAME );
	}
}

#define WITHIN_BBOX()	do { \
	float scale = 2.0f; \
	polymodel *pm = model_get(s_check->modelnum); \
	collided = 0; \
	if(pm != NULL){ \
		vec3d temp = new_obj->pos; \
		vec3d gpos; \
		vm_vec_sub2(&temp, &hit_check->pos); \
		vm_vec_rotate(&gpos, &temp, &hit_check->orient); \
		if((gpos.xyz.x >= pm->mins.xyz.x * scale) && (gpos.xyz.y >= pm->mins.xyz.y * scale) && (gpos.xyz.z >= pm->mins.xyz.z * scale) && (gpos.xyz.x <= pm->maxs.xyz.x * scale) && (gpos.xyz.y <= pm->maxs.xyz.y * scale) && (gpos.xyz.z <= pm->maxs.xyz.z * scale)) { \
			collided = 1; \
		} \
	} \
} while(0)

#define MOVE_AWAY_BBOX() do { \
	polymodel *pm = model_get(s_check->modelnum); \
	if(pm != NULL){ \
		switch((int)frand_range(0.0f, 3.9f)){ \
		case 0: \
			new_obj->pos.xyz.x += 200.0f; \
			break; \
		case 1: \
			new_obj->pos.xyz.x -= 200.0f; \
			break; \
		case 2: \
			new_obj->pos.xyz.y += 200.0f; \
			break; \
		case 3: \
			new_obj->pos.xyz.y -= 200.0f; \
			break; \
		default : \
			new_obj->pos.xyz.z -= 200.0f; \
			break; \
		} \
	} \
} while(0)

// process cheat codes
void game_process_cheats(int k)
{
	int i;
	char *cryptstring;

	if ( k == 0 ){
		return;
	}

#ifndef NO_NETWORK
	// no cheats in multiplayer, ever
	if(Game_mode & GM_MULTIPLAYER){
		Cheats_enabled = 0;
		return;
	}
#endif

	k = key_to_ascii(k);

	for (i = 0; i < CHEAT_BUFFER_LEN; i++){
		CheatBuffer[i]=CheatBuffer[i+1];
	}

	CheatBuffer[CHEATSPOT]=(char)k;

	cryptstring=jcrypt(&CheatBuffer[CHEAT_BUFFER_LEN - CRYPT_STRING_LENGTH]);		

#ifdef FS2_DEMO	
	if ( !strcmp(Cheat_code_demo, cryptstring) ) {
		HUD_printf(XSTR( "Cheats enabled.", 31));
		Cheats_enabled = 1;
		if (Player->flags & PLAYER_FLAGS_MSG_MODE){
			hud_squadmsg_toggle();
		}
	}
	
#else
	if( !strcmp(Cheat_code, cryptstring) && !(Game_mode & GM_MULTIPLAYER)){
		Cheats_enabled = 1;
		HUD_printf("Cheats enabled");
	}
	if( !strcmp(Cheat_code_fish, cryptstring) ){
		// only enable in the main hall
		if((gameseq_get_state() == GS_STATE_MAIN_MENU) && (main_hall_id() == 1)){
			extern void fishtank_start();
			fishtank_start();
		}
	}
	if( !strcmp(Cheat_code_headz, cryptstring) ){
		main_hall_vasudan_funny();
	}
	if( !strcmp(Cheat_code_skip, cryptstring) && (gameseq_get_state() == GS_STATE_MAIN_MENU)){
		extern void main_hall_campaign_cheat();
		main_hall_campaign_cheat();
	}
	if( !strcmp(Cheat_code_tooled, cryptstring) && (Game_mode & GM_IN_MISSION)){
		Tool_enabled = 1;
		HUD_printf("Prepare to be taken to school");
	}
	if( !strcmp(Cheat_code_pirate, cryptstring) && (Game_mode & GM_IN_MISSION) && (Player_obj != NULL)){
		HUD_printf(NOX("Walk the plank"));
		
		for(int idx=0; idx<1; idx++){
			vec3d add = Player_obj->pos;
			add.xyz.x += frand_range(-700.0f, 700.0f);
			add.xyz.y += frand_range(-700.0f, 700.0f);
			add.xyz.z += frand_range(-700.0f, 700.0f);

			int objnum = ship_create(&vmd_identity_matrix, &add, Num_ship_types - 1);

			if(objnum >= 0){
				int collided;
				ship_obj *moveup;
				object *hit_check;
				ship *s_check;
				object *new_obj = &Objects[objnum];

				// place him
				// now make sure we're not colliding with anyone		
				do {
					collided = 0;
					moveup = GET_FIRST(&Ship_obj_list);
					while(moveup!=END_OF_LIST(&Ship_obj_list)){
						// don't check the new_obj itself!!
						if(moveup->objnum != objnum){
							hit_check = &Objects[moveup->objnum];
							Assert(hit_check->type == OBJ_SHIP);
							Assert(hit_check->instance >= 0);
							if((hit_check->type != OBJ_SHIP) || (hit_check->instance < 0)){
								continue;
							}
							s_check = &Ships[hit_check->instance];
							
							// just to make sure we don't get any strange magnitude errors
							if(vm_vec_same(&hit_check->pos, &Objects[objnum].pos)){
								Objects[objnum].pos.xyz.x += 1.0f;
							}
							
							WITHIN_BBOX();				
							if(collided){						
								MOVE_AWAY_BBOX();
								break;
							} 
							collided = 0;
						}
						moveup = GET_NEXT(moveup);
					}
				} while(collided);   					
			
				// warpin
				shipfx_warpin_start(&Objects[objnum]);

				// tell him to attack				
				// ai_add_ship_goal_player( AIG_TYPE_PLAYER_SHIP, AI_GOAL_CHASE_ANY, SM_ATTACK, NULL, &Ai_info[Ships[Objects[objnum].instance].ai_index] );
			}
		}
	}
#endif
	/*
//#ifdef INTERPLAYQA
	if ( !strcmp(Cheat_code_in_game, cryptstring) ) {
		HUD_printf(XSTR( "Cheats enabled.", 31));
		Cheats_enabled = 1;
		if (Player->flags & PLAYER_FLAGS_MSG_MODE){
			hud_squadmsg_toggle();
		}
	} else if ( !strcmp(Cheat_code_movies, cryptstring) ) {
		HUD_printf(XSTR( "All movies available in Tech Room", 32));
		All_movies_enabled = 1;
		if (Player->flags & PLAYER_FLAGS_MSG_MODE){
			hud_squadmsg_toggle();
		}
	} else if( !strcmp(Cheat_code_pirate, cryptstring) ){
		HUD_printf(NOX("Walk the plank"));
		
		for(int idx=0; idx<1; idx++){
			vec3d add;
			add.xyz.x = frand_range(-1000.0f, 1000.0f);
			add.xyz.y = frand_range(-1000.0f, 1000.0f);
			add.xyz.z = frand_range(-1000.0f, 1000.0f);

			int objnum = ship_create(&vmd_identity_matrix, &add, Num_ship_types - 1);			

			if(objnum >= 0){
				shipfx_warpin_start(&Objects[objnum]);
			}
		}
	}
#endif
	*/
}

void game_process_keys()
{
	int k;

	button_info_clear(&Player->bi);	// clear out the button info struct for the player
    do
	{		
		k = game_poll();

		//if (k)
		//	mprintf(("got key %d at %s:%d\n", k, __FILE__, __LINE__));
	
		// AL 12-10-97: Scan for keys used to leave the dead state	(don't process any)
		// DB 1-13-98 : New popup code will run the game do state, so we must skip 
		//              all key processing in this function, since everything should be run through the popup dialog
		if ( Game_mode & GM_DEAD_BLEW_UP ) {
			continue;
		}

		game_process_cheats( k );

		// mwa -- 4/5/97 Moved these two function calls before the switch statement.  I don't think
		// that this has adverse affect on anything and is acutally desireable because of the
		// ESC key being used to quit any HUD message/input mode that might be currently in use
		process_player_ship_keys(k);

//		#ifndef NDEBUG
		process_debug_keys(k);		//	Note, also processed for cheats.
//		#endif		

		switch (k) {
			case 0:
				// No key
				break;
			
			case KEY_ESC:
				if ( Player->control_mode != PCM_NORMAL )	{
					if ( Player->control_mode == PCM_WARPOUT_STAGE1 )	{
						gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
					} else {
						// too late to abort warp out!
					}
				} else {
					// let the ESC key break out of messaging mode
					if ( Players[Player_num].flags & PLAYER_FLAGS_MSG_MODE ) {
						hud_squadmsg_toggle();
						break;
					}

					//If topdown view in non-2D mission, go back to cockpit view.
					if ( (Viewer_mode & VM_TOPDOWN) && !(The_mission.flags & MISSION_FLAG_2D_MISSION)) {
						Viewer_mode &= ~VM_TOPDOWN;
						break;
					}

					// if in external view or chase view, go back to cockpit view
					if ( Viewer_mode & (VM_EXTERNAL|VM_CHASE|VM_OTHER_SHIP) ) {
						Viewer_mode &= ~(VM_EXTERNAL|VM_CHASE|VM_OTHER_SHIP);
						break;
					}

					if (!(Game_mode & GM_DEAD_DIED))
						game_do_end_mission_popup();

				}
				break;

			case KEY_Y:								
				break;

			case KEY_N:
				break;			

			case KEY_ALTED + KEY_SHIFTED+KEY_J:
				// treat the current joystick position as the center position
				joy_set_cen();
				break;

			case KEY_DEBUGGED | KEY_PAUSE:
				gameseq_post_event( GS_EVENT_DEBUG_PAUSE_GAME );
				break;

			case KEY_ALTED + KEY_PAUSE:
				if( Game_mode & GM_DEAD_BLEW_UP || 
					Game_mode & GM_DEAD_DIED) {
					break;
				}

				pause_set_type(PAUSE_TYPE_VIEWER);
				game_process_pause_key();
				break;
			case KEY_PAUSE:
				if( Game_mode & GM_DEAD_BLEW_UP || 
					Game_mode & GM_DEAD_DIED) {
					break;
				}

				pause_set_type(PAUSE_TYPE_NORMAL);
				game_process_pause_key();
				break;

		} // end switch

		
	}
	while (k);

	button_info_do(&Player->bi);	// call functions based on status of button_info bit vectors
}

int button_function_critical(int n, net_player *p = NULL)
{
	object *objp;
	player *pl;
	net_player *npl;
	int at_self;    // flag indicating the object is local (for hud messages, etc)

	Assert(n >= 0);
   
#ifndef NO_NETWORK
	// multiplayer clients should leave critical button bits alone and pass them to the server instead
	if ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)) {
		// if this flag is set, we should apply the button itself (came from the server)
		if (!Multi_button_info_ok){
			return 0;
		}
	}
#endif

	// in single player mode make sure we're using the player object and the player himself, otherwise use the object and
	// player pertaining to the passed net_player
	npl = NULL;
	if (p == NULL) {
		objp = Player_obj;
		pl = Player;
#ifndef NO_NETWORK
		if(Game_mode & GM_MULTIPLAYER){
			npl = Net_player;

			// if we're the server in multiplayer and we're an observer, don't process our own critical button functions
			if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
				return 0;
			}
		}
#endif
		at_self = 1;
	} else {
		objp = &Objects[p->m_player->objnum];
		pl = p->m_player;
		npl = p;
		at_self = 0;

		if ( NETPLAYER_IS_DEAD(npl) || (Ships[Objects[pl->objnum].instance].flags & SF_DYING) )
			return 0;
	}
	
	switch (n) {				
		// cycle to next secondary weapon
		case CYCLE_SECONDARY:
			if(at_self)
				control_used(CYCLE_SECONDARY);
			
			hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
			if (ship_select_next_secondary(objp)) {
				ship* shipp = &Ships[objp->instance];
				if ( timestamp_elapsed(shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank]) ) {
					shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank] = timestamp(250);	//	1/4 second delay until can fire
				}

#ifndef NO_NETWORK
				// multiplayer server should maintain bank/link status here
				if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
					Assert(npl != NULL);
					multi_server_update_player_weapons(npl,shipp);										
				}					
#endif
			}			
			break;

		// cycle number of missiles
		case CYCLE_NUM_MISSLES:
			if(at_self)
				control_used(CYCLE_NUM_MISSLES);

			if ( objp == Player_obj ) {
				if ( Player_ship->weapons.num_secondary_banks <= 0 ) {
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has no secondary weapons", 33));
					gamesnd_play_iface(SND_GENERAL_FAIL);
					break;
				}
			}
					
			if ( Ships[objp->instance].flags & SF_SECONDARY_DUAL_FIRE ) {		
				Ships[objp->instance].flags &= ~SF_SECONDARY_DUAL_FIRE;
				if(at_self) {
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary weapon set to normal fire mode", 34));
					snd_play( &Snds[SND_SECONDARY_CYCLE] );
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
				}
			} else {
				Ships[objp->instance].flags |= SF_SECONDARY_DUAL_FIRE;
				if(at_self) {
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary weapon set to dual fire mode", 35));
					snd_play( &Snds[SND_SECONDARY_CYCLE] );
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
				}
			}

#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}					
#endif
			break;

		// increase weapon recharge rate
		case INCREASE_WEAPON:
			if(at_self)
				control_used(INCREASE_WEAPON);
			increase_recharge_rate(objp, WEAPONS);

#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}					
#endif
			break;

		// decrease weapon recharge rate
		case DECREASE_WEAPON:
			if(at_self)
				control_used(DECREASE_WEAPON);
			decrease_recharge_rate(objp, WEAPONS);

#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}								
#endif
			break;

		// increase shield recharge rate
		case INCREASE_SHIELD:
			if(at_self)
				control_used(INCREASE_SHIELD);
			increase_recharge_rate(objp, SHIELDS);

#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}								
#endif
			break;

		// decrease shield recharge rate
		case DECREASE_SHIELD:
			if(at_self)
				control_used(DECREASE_SHIELD);
			decrease_recharge_rate(objp, SHIELDS);

#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}								
#endif
			break;

		// increase energy to engines
		case INCREASE_ENGINE:
			if(at_self)
				control_used(INCREASE_ENGINE);
			increase_recharge_rate(objp, ENGINES);

#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}							
#endif
			break;

		// decrease energy to engines
		case DECREASE_ENGINE:
			if(at_self)
   			control_used(DECREASE_ENGINE);
			decrease_recharge_rate(objp, ENGINES);

#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}										
#endif
			break;

		// equalize recharge rates
		case ETS_EQUALIZE:
			if (at_self) {
   			control_used(ETS_EQUALIZE);
			}

			set_default_recharge_rates(objp);
			snd_play( &Snds[SND_ENERGY_TRANS] );
#ifndef NO_NETWORK
			// multiplayer server should maintain bank/link status here
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}										
#endif
			break;

		// equalize shield energy to all quadrants
		case SHIELD_EQUALIZE:
			if(at_self){
				control_used(SHIELD_EQUALIZE);
			}
			hud_shield_equalize(objp, pl);
			break;

		// transfer shield energy to front
		case SHIELD_XFER_TOP:
			if(at_self){
   			control_used(SHIELD_XFER_TOP);
			}
			hud_augment_shield_quadrant(objp, 1);
			break;

		// transfer shield energy to rear
		case SHIELD_XFER_BOTTOM:
			if(at_self)
				control_used(SHIELD_XFER_BOTTOM);
			hud_augment_shield_quadrant(objp, 2);
			break;

		// transfer shield energy to left
		case SHIELD_XFER_LEFT:
			if(at_self)
				control_used(SHIELD_XFER_LEFT);
			hud_augment_shield_quadrant(objp, 3);
			break;
			
		// transfer shield energy to right
		case SHIELD_XFER_RIGHT:
			if(at_self)
				control_used(SHIELD_XFER_RIGHT);
			hud_augment_shield_quadrant(objp, 0);
			break;

		// transfer energy to shield from weapons
		case XFER_SHIELD:
			if(at_self)
				control_used(XFER_SHIELD);
			transfer_energy_to_shields(objp);
			break;

		// transfer energy to weapons from shield
		case XFER_LASER:
			if(at_self)
				control_used(XFER_LASER);
			transfer_energy_to_weapons(objp);
			break;

		// following are not handled here, but we need to bypass the Int3()
		case LAUNCH_COUNTERMEASURE:
		case VIEW_SLEW:
		case VIEW_EXTERNAL:
		case VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK:
		case VIEW_TRACK_TARGET:
		case ONE_THIRD_THROTTLE:
		case TWO_THIRDS_THROTTLE:
		case MINUS_5_PERCENT_THROTTLE:
		case PLUS_5_PERCENT_THROTTLE:
		case ZERO_THROTTLE:
		case MAX_THROTTLE:
			return 0;

		default :
			Int3(); // bad bad bad
			break;
	}

	return 1;
}

// return !0 if the action is allowed, otherwise return 0
int button_allowed(int n)
{
	// RT Commented this out cos it was disabling use of keys when HUD is off
#if 0
	if ( hud_disabled() ) {
		switch (n) {
		case SHOW_GOALS:
		case END_MISSION:
		case CYCLE_NEXT_PRIMARY:
		case CYCLE_PREV_PRIMARY:
		case CYCLE_SECONDARY:
		case ONE_THIRD_THROTTLE:
		case TWO_THIRDS_THROTTLE:
		case PLUS_5_PERCENT_THROTTLE:
		case MINUS_5_PERCENT_THROTTLE:
		case ZERO_THROTTLE:
		case MAX_THROTTLE:
			return 1;
		default:
			return 0;
		}
	}
#endif

	return 1;
}

// execute function corresponding to action n
// basically, these are actions which don't affect demo playback at all
int button_function_demo_valid(int n)
{
	// by default, we'll return "not processed". ret will get set to 1, if this is one of the keys which is always allowed, even in demo
	// playback.
	int ret = 0;

	//	No keys, not even targeting keys, when player in death roll.  He can press keys after he blows up.
	if (Game_mode & GM_DEAD_DIED){
		return 0;
	}

	// any of these buttons are valid
	switch(n){
	case VIEW_CHASE:
		control_used(VIEW_CHASE);
		if(!Perspective_locked)
		{
			Viewer_mode ^= VM_CHASE;
			if ( Viewer_mode & VM_CHASE ) {
				Viewer_mode &= ~VM_EXTERNAL;
			}
		}
		else
		{
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		ret = 1;
		break;

	case VIEW_EXTERNAL:
		control_used(VIEW_EXTERNAL);
		if(!Perspective_locked)
		{
			Viewer_mode ^= VM_EXTERNAL;
			Viewer_mode &= ~VM_EXTERNAL_CAMERA_LOCKED;	// reset camera lock when leave/entering external view
			if ( Viewer_mode & VM_EXTERNAL ) {
				Viewer_mode &= ~VM_CHASE;
			}
		}
		else
		{
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		ret = 1;
		break;

	case VIEW_TOPDOWN:
		control_used(VIEW_TOPDOWN);
		if(!Perspective_locked)
		{
			Viewer_mode ^= VM_TOPDOWN;
			if(Viewer_mode & VM_TOPDOWN) {
				Viewer_mode &= ~VM_TOPDOWN;
			}
		}
		else
		{
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		ret = 1;
		break;

	case VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK:
		control_used(VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK);
		if ( Viewer_mode & VM_EXTERNAL ) {
		Viewer_mode ^= VM_EXTERNAL_CAMERA_LOCKED;
		if ( Viewer_mode & VM_EXTERNAL_CAMERA_LOCKED ) {
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "External camera is locked, controls will move ship", 36));
			} else {
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "External camera is free, controls will move the camera, not the ship", 37));
			}
		}
		ret = 1;
		break;

	case VIEW_OTHER_SHIP:
		control_used(VIEW_OTHER_SHIP);
		if ( Player_ai->target_objnum < 0 || Perspective_locked) {
			snd_play( &Snds[SND_TARGET_FAIL] );
		} else {
			if ( Objects[Player_ai->target_objnum].type != OBJ_SHIP )  {
				snd_play( &Snds[SND_TARGET_FAIL] );
			} else {
				Viewer_mode ^= VM_OTHER_SHIP;
			}
		}
		ret = 1;
		break;

	case TIME_SLOW_DOWN:
		if ( Game_mode & GM_NORMAL ) {
			// Goober5000 - time dilation only available in cheat mode (see above);
			// now you can do it with or without pressing the tilde, per Kazan's request
			if ( ((Game_time_compression > F1_0) || (Cheats_enabled && (Game_time_compression > (F1_0/MAX_TIME_DIVIDER)))) && !Time_compression_locked) {
				change_time_compression(0.5f);
			} else {
				gamesnd_play_error_beep();
			}
		} else {
			gamesnd_play_error_beep();
		}
		ret = 1;
		break;

	case TIME_SPEED_UP:
		if ( Game_mode & GM_NORMAL ) {
			if ( (Game_time_compression < (F1_0*MAX_TIME_MULTIPLIER)) && !Time_compression_locked ) {
				change_time_compression(2.0f);
			} else {
				gamesnd_play_error_beep();
			}
		} else {
			gamesnd_play_error_beep();
		}
		ret = 1;
		break;
	}

	// done
	return ret;
}

// execute function corresponding to action n (BUTTON_ #define from KeyControl.h)
// Goober5000: function returns 1 when action was taken
int button_function(int n)
{
	Assert(n >= 0);

	//mprintf(("got button %d at %s:%d\n", n, __FILE__, __LINE__));

	if ( !button_allowed(n) ) {
		return 0;
	}

	//	No keys, not even targeting keys, when player in death roll.  He can press keys after he blows up.
	if (Game_mode & GM_DEAD_DIED){
		return 0;
	}


	// Goober5000 - if the ship doesn't have subspace drive, jump key doesn't work: so test and exit early
	if (Player_ship->flags2 & SF2_NO_SUBSPACE_DRIVE)
	{
		switch(n)
		{
			case END_MISSION:
				control_used(n);	// set the timestamp for when we used the control, in case we need it
				return 1;			// pretend we took the action: if we return 0, strange stuff may happen
		}
	}

	// Goober5000 - if we have primitive sensors, some keys don't work: so test and exit early
	if (Player_ship->flags2 & SF2_PRIMITIVE_SENSORS)
	{
		switch (n)
		{
			case MATCH_TARGET_SPEED:
			case TOGGLE_AUTO_MATCH_TARGET_SPEED:
			case TOGGLE_AUTO_TARGETING:
			case TARGET_NEXT:
			case TARGET_PREV:
			case TARGET_NEXT_CLOSEST_HOSTILE:
			case TARGET_PREV_CLOSEST_HOSTILE:
			case TARGET_NEXT_CLOSEST_FRIENDLY:
			case TARGET_PREV_CLOSEST_FRIENDLY:
			case TARGET_SHIP_IN_RETICLE:
			case TARGET_LAST_TRANMISSION_SENDER:
			case TARGET_CLOSEST_REPAIR_SHIP:
			case TARGET_CLOSEST_SHIP_ATTACKING_TARGET:
			case STOP_TARGETING_SHIP:
			case TARGET_CLOSEST_SHIP_ATTACKING_SELF:
			case TARGET_TARGETS_TARGET:
			case TARGET_SUBOBJECT_IN_RETICLE:
			case TARGET_NEXT_SUBOBJECT:
			case TARGET_PREV_SUBOBJECT:
			case STOP_TARGETING_SUBSYSTEM:
			case TARGET_NEXT_BOMB:
			case TARGET_PREV_BOMB:
			case TARGET_NEXT_UNINSPECTED_CARGO:
			case TARGET_PREV_UNINSPECTED_CARGO:
			case TARGET_NEWEST_SHIP:
			case TARGET_NEXT_LIVE_TURRET:
			case TARGET_PREV_LIVE_TURRET:
			case TARGET_NEXT_ESCORT_SHIP:
				control_used(n);	// set the timestamp for when we used the control, in case we need it
				return 1;			// pretend we took the action: if we return 0, strange stuff may happen
		}
	}

	// now handle keys the regular way
	switch (n) {
		// cycle to next primary weapon
		case CYCLE_NEXT_PRIMARY:			
			// bogus?
			if((Player_obj == NULL) || (Player_ship == NULL)){
				break;
			}

			hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
			if (ship_select_next_primary(Player_obj, CYCLE_PRIMARY_NEXT)) {
				ship* shipp = Player_ship;
				shipp->weapons.next_primary_fire_stamp[shipp->weapons.current_primary_bank] = timestamp(250);	//	1/4 second delay until can fire				
				// multiplayer server should maintain bank/link status here
				// if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
//					Assert(npl != NULL);
//					multi_server_update_player_weapons(npl,shipp);										
//				}					
			}			
			break;

		// cycle to previous primary weapon
		case CYCLE_PREV_PRIMARY:			
			// bogus?
			if((Player_obj == NULL) || (Player_ship == NULL)){
				break;
			}

			hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
			if (ship_select_next_primary(Player_obj, CYCLE_PRIMARY_PREV)) {
				ship* shipp = Player_ship;
				shipp->weapons.next_primary_fire_stamp[shipp->weapons.current_primary_bank] = timestamp(250);	//	1/4 second delay until can fire

				// multiplayer server should maintain bank/link status here
				// if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
					// Assert(npl != NULL);
					// multi_server_update_player_weapons(npl,shipp);										
				// }					
			}			
			break;

		// cycle to next secondary weapon
		case CYCLE_SECONDARY:
			return button_function_critical(CYCLE_SECONDARY);
			break;

		// cycle number of missiles fired from secondary bank
		case CYCLE_NUM_MISSLES:
         return button_function_critical(CYCLE_NUM_MISSLES);			
			break;

		// undefined in multiplayer for clients right now
		// match target speed
		case MATCH_TARGET_SPEED:
			control_used(MATCH_TARGET_SPEED);
			// If player is auto-matching, break auto-match speed
			if ( Player->flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
				Player->flags &= ~PLAYER_FLAGS_AUTO_MATCH_SPEED;
			}
			player_match_target_speed();						
			break;

		// undefined in multiplayer for clients right now
		// toggle auto-match target speed
		case TOGGLE_AUTO_MATCH_TARGET_SPEED:
			// multiplayer observers can't match target speed
#ifndef NO_NETWORK
			if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)) ){
				break;
			}
#endif	
			Player->flags ^= PLAYER_FLAGS_AUTO_MATCH_SPEED;			
			control_used(TOGGLE_AUTO_MATCH_TARGET_SPEED);
			hud_gauge_popup_start(HUD_AUTO_SPEED);
			if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
				snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
//				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Auto match target speed activated", -1));
				if ( !Player->flags & PLAYER_FLAGS_MATCH_TARGET ) {
					player_match_target_speed();
				}
			}
			else {
//				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Auto match target deactivated", -1));
				snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
				player_match_target_speed();
			}			
			break;

		// target next
		case TARGET_NEXT:
			control_used(TARGET_NEXT);			
			if ( hud_sensors_ok(Player_ship) ) {
				hud_target_next();
			}
			break;

		// target previous
		case TARGET_PREV:
			control_used(TARGET_PREV);			
			if ( hud_sensors_ok(Player_ship) ) {
				hud_target_prev();			
			}
			break;

		// target the next hostile target
		case TARGET_NEXT_CLOSEST_HOSTILE:
			control_used(TARGET_NEXT_CLOSEST_HOSTILE);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_list();
			}			
			break;

		// target the previous closest hostile 
		case TARGET_PREV_CLOSEST_HOSTILE:
			control_used(TARGET_PREV_CLOSEST_HOSTILE);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_list(1,0);
			}
			break;

		// toggle auto-targeting
		case TOGGLE_AUTO_TARGETING:
			control_used(TOGGLE_AUTO_TARGETING);
			hud_gauge_popup_start(HUD_AUTO_TARGET);
			Players[Player_num].flags ^= PLAYER_FLAGS_AUTO_TARGETING;
			if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_TARGETING ) {
				if (hud_sensors_ok(Player_ship)) {
					hud_target_closest(opposing_team_mask(Player_ship->team), -1, FALSE, TRUE );
					snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
//					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Auto targeting activated", -1));
				} else {
					Players[Player_num].flags ^= PLAYER_FLAGS_AUTO_TARGETING;
				}
			} else {
				snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
//				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Auto targeting deactivated", -1));
			}
			break;

		// target the next friendly ship
		case TARGET_NEXT_CLOSEST_FRIENDLY:
			control_used(TARGET_NEXT_CLOSEST_FRIENDLY);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_list(0);
			}
			break;

		// target the closest friendly ship
		case TARGET_PREV_CLOSEST_FRIENDLY:
			control_used(TARGET_PREV_CLOSEST_FRIENDLY);			
			if (hud_sensors_ok(Player_ship)) {
				hud_target_next_list(0,0);
			}
			break;

		// target ship closest to center of reticle
		case TARGET_SHIP_IN_RETICLE:
			control_used(TARGET_SHIP_IN_RETICLE);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_in_reticle_new();
			}
			break;

		case TARGET_LAST_TRANMISSION_SENDER:
			control_used(TARGET_LAST_TRANMISSION_SENDER);
			if ( hud_sensors_ok(Player_ship)) {
				hud_target_last_transmit();
			}
			break;

		// target the closest repair ship
		case TARGET_CLOSEST_REPAIR_SHIP:
			control_used(TARGET_CLOSEST_REPAIR_SHIP);
			// AL: Try to find the closest repair ship coming to repair the player... if no support
			//		 ships are coming to rearm the player, just try for the closest repair ship
			if ( hud_target_closest_repair_ship(OBJ_INDEX(Player_obj)) == 0 ) {
				if ( hud_target_closest_repair_ship() == 0 ) {
					snd_play(&Snds[SND_TARGET_FAIL]);
				}
			}
			break;

		// target the closest ship attacking current target
		case TARGET_CLOSEST_SHIP_ATTACKING_TARGET:
			control_used(TARGET_CLOSEST_SHIP_ATTACKING_TARGET);
			if (hud_sensors_ok(Player_ship)){
				hud_target_closest(opposing_team_mask(Player_ship->team), Player_ai->target_objnum);
			}
			break;

		// stop targeting ship
		case STOP_TARGETING_SHIP:
			control_used(STOP_TARGETING_SHIP);
			hud_cease_targeting();
			break;

		// target closest ship that is attacking player
		case TARGET_CLOSEST_SHIP_ATTACKING_SELF:
			control_used(TARGET_CLOSEST_SHIP_ATTACKING_SELF);
			if (hud_sensors_ok(Player_ship)){
				hud_target_closest(opposing_team_mask(Player_ship->team), OBJ_INDEX(Player_obj), TRUE, 0, 1);
			}
			break;

		// target your target's target
		case TARGET_TARGETS_TARGET:
			control_used(TARGET_TARGETS_TARGET);
			if (hud_sensors_ok(Player_ship)){
				hud_target_targets_target();
			}
			break;

		// target ships subsystem in reticle
		case TARGET_SUBOBJECT_IN_RETICLE:
			control_used(TARGET_SUBOBJECT_IN_RETICLE);
			if (hud_sensors_ok(Player_ship)){
				hud_target_subsystem_in_reticle();
			}
			break;

		case TARGET_PREV_SUBOBJECT:
			control_used(TARGET_PREV_SUBOBJECT);
			if (hud_sensors_ok(Player_ship)){
				hud_target_prev_subobject();
			}
			break;

		// target next subsystem on current target
		case TARGET_NEXT_SUBOBJECT:
			control_used(TARGET_NEXT_SUBOBJECT);
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_subobject();
			}
			break;

		// stop targeting subsystems on ship
		case STOP_TARGETING_SUBSYSTEM:
			control_used(STOP_TARGETING_SUBSYSTEM);
			hud_cease_subsystem_targeting();
			break;
			
		case TARGET_NEXT_BOMB:
			control_used(TARGET_NEXT_BOMB);
			hud_target_missile(Player_obj, 1);
			break;

		case TARGET_PREV_BOMB:
			control_used(TARGET_PREV_BOMB);
			hud_target_missile(Player_obj, 0);
			break;

		case TARGET_NEXT_UNINSPECTED_CARGO:
			hud_target_uninspected_object(1);
			break;

		case TARGET_PREV_UNINSPECTED_CARGO:
			hud_target_uninspected_object(0);
			break;

		case TARGET_NEWEST_SHIP:
			hud_target_newest_ship();
			break;

		case TARGET_NEXT_LIVE_TURRET:
			hud_target_live_turret(1);
			break;

		case TARGET_PREV_LIVE_TURRET:
			hud_target_live_turret(0);
			break;

		// wingman message: attack current target
		case ATTACK_MESSAGE:
			control_used(ATTACK_MESSAGE);
			hud_squadmsg_shortcut( ATTACK_TARGET_ITEM );
			break;

		// wingman message: disarm current target
		case DISARM_MESSAGE:
			control_used(DISARM_MESSAGE);
			hud_squadmsg_shortcut( DISARM_TARGET_ITEM );
			break;

		// wingman message: disable current target
		case DISABLE_MESSAGE:
			control_used(DISABLE_MESSAGE);
			hud_squadmsg_shortcut( DISABLE_TARGET_ITEM );
			break;

		// wingman message: disable current target
		case ATTACK_SUBSYSTEM_MESSAGE:
			control_used(ATTACK_SUBSYSTEM_MESSAGE);
			hud_squadmsg_shortcut( DISABLE_SUBSYSTEM_ITEM );
			break;

		// wingman message: capture current target
		case CAPTURE_MESSAGE:
			control_used(CAPTURE_MESSAGE);
			hud_squadmsg_shortcut( CAPTURE_TARGET_ITEM );
			break;

		// wingman message: engage enemy
		case ENGAGE_MESSAGE:
    		control_used(ENGAGE_MESSAGE);
			hud_squadmsg_shortcut( ENGAGE_ENEMY_ITEM );
			break;

		// wingman message: form on my wing
		case FORM_MESSAGE:
			control_used(FORM_MESSAGE);
			hud_squadmsg_shortcut( FORMATION_ITEM );
			break;

		// wingman message: protect current target
		case PROTECT_MESSAGE:
			control_used(PROTECT_MESSAGE);
			hud_squadmsg_shortcut( PROTECT_TARGET_ITEM );
			break;

		// wingman message: cover me
		case COVER_MESSAGE:
			control_used(COVER_MESSAGE);
			hud_squadmsg_shortcut( COVER_ME_ITEM );
			break;
		
		// wingman message: warp out
		case WARP_MESSAGE:
			control_used(WARP_MESSAGE);
			hud_squadmsg_shortcut( DEPART_ITEM );
			break;

		case IGNORE_MESSAGE:
			control_used(IGNORE_MESSAGE);
			hud_squadmsg_shortcut( IGNORE_TARGET_ITEM );
			break;

		// rearm message
		case REARM_MESSAGE:
			control_used(REARM_MESSAGE);
			hud_squadmsg_rearm_shortcut();
			break;

		// cycle to next radar range
		case RADAR_RANGE_CYCLE:
			control_used(RADAR_RANGE_CYCLE);
			HUD_config.rp_dist++;
			if ( HUD_config.rp_dist >= RR_MAX_RANGES )
				HUD_config.rp_dist = 0;

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Radar range set to %s", 38), Radar_range_text(HUD_config.rp_dist));
			break;

		// toggle the squadmate messaging menu
		case SQUADMSG_MENU:
			control_used(SQUADMSG_MENU);
			hud_squadmsg_toggle();				// leave the details to the messaging code!!!
			break;

		// show the mission goals screen
		case SHOW_GOALS:
			control_used(SHOW_GOALS);
			gameseq_post_event( GS_EVENT_SHOW_GOALS );
			break;

		// end the mission
		case END_MISSION:
#ifndef NO_NETWORK
			// in multiplayer, all end mission requests should go through the server
			if(Game_mode & GM_MULTIPLAYER){				
				multi_handle_end_mission_request();
				break;
			}
#endif
			control_used(END_MISSION);
			
			if (collide_predict_large_ship(Player_obj, 200.0f)) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				HUD_printf(XSTR( "** WARNING ** Collision danger.  Warpout not activated.", 39));
			} else if (!ship_can_warp(Player_ship)  /*ship_get_subsystem_strength( Player_ship, SUBSYSTEM_ENGINE ) < 0.1f*/) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				HUD_printf(XSTR( "Engine failure.  Cannot engage warp drive.", 40));
			} else {
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_START );
			}			
			break;

		case ADD_REMOVE_ESCORT:
			if ( Player_ai->target_objnum >= 0 ) {
				control_used(ADD_REMOVE_ESCORT);
				hud_add_remove_ship_escort(Player_ai->target_objnum);
			}
			break;

		case ESCORT_CLEAR:
			control_used(ESCORT_CLEAR);
			hud_escort_clear_all();
			break;

		case TARGET_NEXT_ESCORT_SHIP:
			control_used(TARGET_NEXT_ESCORT_SHIP);
			hud_escort_target_next();
			break;

		// increase weapon recharge rate
		case INCREASE_WEAPON:
			hud_gauge_popup_start(HUD_ETS_GAUGE);
			return button_function_critical(INCREASE_WEAPON);
			break;

		// decrease weapon recharge rate
		case DECREASE_WEAPON:
			hud_gauge_popup_start(HUD_ETS_GAUGE);
			return button_function_critical(DECREASE_WEAPON);
			break;

		// increase shield recharge rate
		case INCREASE_SHIELD:
			hud_gauge_popup_start(HUD_ETS_GAUGE);
			return button_function_critical(INCREASE_SHIELD);
			break;

		// decrease shield recharge rate
		case DECREASE_SHIELD:
			hud_gauge_popup_start(HUD_ETS_GAUGE);
			return button_function_critical(DECREASE_SHIELD);
			break;

		// increase energy to engines
		case INCREASE_ENGINE:
			hud_gauge_popup_start(HUD_ETS_GAUGE);
			return button_function_critical(INCREASE_ENGINE);
			break;

		// decrease energy to engines
		case DECREASE_ENGINE:
			hud_gauge_popup_start(HUD_ETS_GAUGE);
			return button_function_critical(DECREASE_ENGINE);
			break;

		case ETS_EQUALIZE:
			hud_gauge_popup_start(HUD_ETS_GAUGE);
			return button_function_critical(ETS_EQUALIZE);
			break;

		// equalize shield energy to all quadrants
		case SHIELD_EQUALIZE:
			return button_function_critical(SHIELD_EQUALIZE);
			break;

		// transfer shield energy to front
		case SHIELD_XFER_TOP:
         return button_function_critical(SHIELD_XFER_TOP);
			break;

		// transfer shield energy to rear
		case SHIELD_XFER_BOTTOM:
			return button_function_critical(SHIELD_XFER_BOTTOM);
			break;

		// transfer shield energy to left
		case SHIELD_XFER_LEFT:
			return button_function_critical(SHIELD_XFER_LEFT);
			break;
		
		// transfer shield energy to right
		case SHIELD_XFER_RIGHT:
			return button_function_critical(SHIELD_XFER_RIGHT);
			break;

		// transfer energy to shield from weapons
		case XFER_SHIELD:
			return button_function_critical(XFER_SHIELD);
			break;

		// transfer energy to weapons from shield
		case XFER_LASER:
			return button_function_critical(XFER_LASER);
			break;

#ifndef NO_NETWORK
		// message all netplayers button
		case MULTI_MESSAGE_ALL:
			multi_msg_key_down(MULTI_MSG_ALL);
			break;

		// message all friendlies button
		case MULTI_MESSAGE_FRIENDLY:
			multi_msg_key_down(MULTI_MSG_FRIENDLY);
			break;

		// message all hostiles button
		case MULTI_MESSAGE_HOSTILE:
			multi_msg_key_down(MULTI_MSG_HOSTILE);
			break;

		// message targeted ship (if player)
		case MULTI_MESSAGE_TARGET:
			multi_msg_key_down(MULTI_MSG_TARGET);
			break;

		// if i'm an observer, zoom to my targeted object
		case MULTI_OBSERVER_ZOOM_TO:
			multi_obs_zoom_to_target();
			break;		
#endif

		// toggle between high and low HUD contrast
		case TOGGLE_HUD_CONTRAST:
			gamesnd_play_iface(SND_USER_SELECT);
			hud_toggle_contrast();
			break;

#ifndef NO_NETWORK
		// toggle network info
		case MULTI_TOGGLE_NETINFO:
			extern int Multi_display_netinfo;
			Multi_display_netinfo = !Multi_display_netinfo;
			break;

		// self destruct (multiplayer only)
		case MULTI_SELF_DESTRUCT:
			if(!(Game_mode & GM_MULTIPLAYER)){
				break;
			}

			// bogus netplayer
			if((Net_player == NULL) || (Net_player->m_player == NULL)){
				break;
			}

			// blow myself up, if I'm the server
			if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
				if((Net_player->m_player->objnum >= 0) && (Net_player->m_player->objnum < MAX_OBJECTS) && 
					(Objects[Net_player->m_player->objnum].type == OBJ_SHIP) && (Objects[Net_player->m_player->objnum].instance >= 0) && (Objects[Net_player->m_player->objnum].instance < MAX_SHIPS)){

					ship_self_destruct(&Objects[Net_player->m_player->objnum]);
				}
			}
			// otherwise send a packet to the server
			else {
				send_self_destruct_packet();
			}
			break;
#endif
		case TOGGLE_HUD:
			gamesnd_play_iface(SND_USER_SELECT);
			hud_toggle_draw();
			break;

		case HUD_TARGETBOX_TOGGLE_WIREFRAME:
			gamesnd_play_iface(SND_USER_SELECT);
			hud_targetbox_switch_wireframe_mode();
			break;	


#if defined(ENABLE_AUTO_PILOT)
		// Autopilot key control
		case AUTO_PILOT_TOGGLE:
			if (AutoPilotEngaged)
			{
				EndAutoPilot();
			}
			else
			{
				if (CurrentNav == -1)
				{
					gamesnd_play_iface(SND_GENERAL_FAIL);
				}
				else
				{
					if (CanAutopilot())
					{
						StartAutopilot();
					}
					else
						gamesnd_play_iface(SND_GENERAL_FAIL);
				}
			}
			break;

		case NAV_CYCLE:
			if (!Sel_NextNav())
				gamesnd_play_iface(SND_GENERAL_FAIL);
			break;
#endif

		// following are not handled here, but we need to bypass the Int3()
		case LAUNCH_COUNTERMEASURE:
		case VIEW_SLEW:
		case VIEW_TRACK_TARGET:
		case ONE_THIRD_THROTTLE:
		case TWO_THIRDS_THROTTLE:
		case MINUS_5_PERCENT_THROTTLE:
		case PLUS_5_PERCENT_THROTTLE:
		case ZERO_THROTTLE:
		case MAX_THROTTLE:
			return 0;

		default:
			mprintf(("Unknown key %d at %s:%u\n", n, __FILE__, __LINE__));
//  			Int3();
			break;
	} // end switch

	return 1;
}

// Call functions for when buttons are pressed
void button_info_do(button_info *bi)
{
	int i, j;

	for (i=0; i<NUM_BUTTON_FIELDS; i++) {
		if ( bi->status[i] == 0 ){
			continue;
		}

		// at least one bit is set in the status integer
		for (j=0; j<32; j++) {

			// check if the bit is set. If button_function returns 1 (implying the action was taken), then unset the bit
			if ( bi->status[i] & (1 << j) ) {
				// always process buttons which are valid for demo playback
				if(button_function_demo_valid(32 * i + j)){
					bi->status[i] &= ~(1 << j);
				}
				// other buttons
				else {
					// if we're in demo playback, always clear the bits
					if(Game_mode & GM_DEMO_PLAYBACK){
						bi->status[i] &= ~(1 << j);
					}
					// otherwise check as normal
					else if (button_function(32 * i + j)) {
						bi->status[i] &= ~(1 << j);					
					}
				}
			}
		}
	}
}


// set the bit for the corresponding action n (BUTTON_ #define from KeyControl.h)
void button_info_set(button_info *bi, int n)
{
	int field_num, bit_num;
	
	field_num = n / 32;
	bit_num = n % 32;

	bi->status[field_num] |= (1 << bit_num);	
}

// unset the bit for the corresponding action n (BUTTON_ #define from KeyControl.h)
void button_info_unset(button_info *bi, int n)
{
	int field_num, bit_num;
	
	field_num = n / 32;
	bit_num = n % 32;

	bi->status[field_num] &= ~(1 << bit_num);	
}

int button_info_query(button_info *bi, int n)
{
	return bi->status[n / 32] & (1 << (n % 32));
}

// clear out the button_info struct
void button_info_clear(button_info *bi)
{
	int i;

	for (i=0; i<NUM_BUTTON_FIELDS; i++) {
		bi->status[i] = 0;
	}
}

// strip out all noncritical keys from the button info struct
void button_strip_noncritical_keys(button_info *bi)
{
	int idx;

	// clear out all noncritical keys
	for(idx=0;idx<Non_critical_key_set_size;idx++){
		button_info_unset(bi,Non_critical_key_set[idx]);
	}
} 



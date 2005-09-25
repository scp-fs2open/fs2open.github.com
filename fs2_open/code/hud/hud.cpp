/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUD.cpp $
 * $Revision: 2.54 $
 * $Date: 2005-09-25 22:24:22 $
 * $Author: Goober5000 $
 *
 * C module that contains all the HUD functions at a high level
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.53  2005/08/31 07:17:43  Goober5000
 * removed two unused functions
 * --Goober5000
 *
 * Revision 2.52  2005/07/22 10:18:38  Goober5000
 * CVS header tweaks
 * --Goober5000
 *
 * Revision 2.51  2005/07/18 03:44:01  taylor
 * cleanup hudtargetbox rendering from that total hack job that had been done on it (fixes wireframe view as well)
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.50  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.49  2005/07/13 02:30:53  Goober5000
 * removed autopilot #define
 * --Goober5000
 *
 * Revision 2.48  2005/07/13 00:44:22  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.47  2005/07/02 19:42:15  taylor
 * ton of non-standard resolution fixes
 *
 * Revision 2.46  2005/05/30 05:31:19  taylor
 * make sure we don't show various offscreen indicators and info when hud-disabled-except-messages is used
 *
 * Revision 2.45  2005/05/12 03:50:10  Goober5000
 * repeating messages with variables should work properly now
 * --Goober5000
 *
 * Revision 2.44  2005/04/05 05:53:17  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.43  2005/03/27 12:28:33  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.42  2005/03/25 06:57:34  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.41  2005/03/10 08:00:06  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.40  2005/03/02 21:24:44  taylor
 * more network/inferno goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.39  2005/02/27 07:07:47  wmcoolmon
 * nonstandard res HUD stuff
 *
 * Revision 2.38  2005/02/14 23:54:11  taylor
 * make loading screen shader a bit taller
 * add i.o to credits for Linux and OSX code
 * add libjpeg and ogg stuff to credits for license compliance
 * replace an Int3() with a debug message in the hud code
 *
 * Revision 2.37  2005/02/13 08:38:54  wmcoolmon
 * nonstandard resolution-friendly function updates
 *
 * Revision 2.36  2005/01/30 03:26:11  wmcoolmon
 * HUD updates
 *
 * Revision 2.35  2005/01/17 06:35:35  wmcoolmon
 * Attempt to fix the crash when a player ship's subsystems receives damage, and it has a lot (ie player ship is a capital ship).This seems to be controlled by SUBSYSTEM_MAX, which also controls the number of subsystems saved in red alert missions.
 *
 * Revision 2.34  2005/01/12 00:17:09  phreak
 * fixed hud offsets bug
 *
 * Revision 2.33  2005/01/11 21:38:49  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.32  2005/01/01 07:18:47  wmcoolmon
 * NEW_HUD stuff, turned off this time. :) It's in a state of disrepair at the moment, doesn't show anything.
 *
 * Revision 2.31  2004/12/26 22:45:58  taylor
 * fix some hud stuff getting drawn multiple times, don't show target data if HUD_disabled_except_messages is set
 *
 * Revision 2.30  2004/12/26 20:46:34  Goober5000
 * hud-disable-except-messages now resets after each mission
 * --Goober5000
 *
 * Revision 2.29  2004/12/25 09:27:41  wmcoolmon
 * Fix to modular tables workaround with Fs2NetD + Sync to current NEW_HUD code
 *
 * Revision 2.28  2004/12/24 05:07:05  wmcoolmon
 * NEW_HUD compiles now. :)
 *
 * Revision 2.27  2004/12/23 23:08:21  wmcoolmon
 * Proposed HUD system stuffs - within NEW_HUD defines.
 *
 * Revision 2.26  2004/12/14 14:46:12  Goober5000
 * allow different wing names than ABGDEZ
 * --Goober5000
 *
 * Revision 2.25  2004/11/21 11:29:53  taylor
 * make hud_disabled_except_messages() do everything at once, add it to hud_show_radar() and hud_show_target_model() so those don't show up (bug #271)
 *
 * Revision 2.24  2004/09/17 00:18:17  Goober5000
 * changed toggle-hud to hud-disable; added hud-disable-except-messages
 * --Goober5000
 *
 * Revision 2.23  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 2.22  2004/07/26 17:54:04  Kazan
 * Autopilot system completed -- i am dropping plans for GUI nav map
 * Fixed FPS counter during time compression
 *
 * Revision 2.21  2004/07/12 16:32:49  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.20  2004/07/01 01:53:27  phreak
 * function pointer radar update.
 * will enable us to make different radar styles that we can switch between
 *
 * Revision 2.19  2004/06/08 00:35:43  wmcoolmon
 * Added +Color: option for custom gauges.
 *
 * Revision 2.18  2004/06/01 07:31:56  wmcoolmon
 * Lotsa stuff. Custom gauges w/ ANIs support added, SEXPs to set gauge text, gauge image frames, and gauge coords. These SEXPs and toggle-hud reside in the Hud/change category.
 *
 * Revision 2.17  2004/05/31 08:54:13  wmcoolmon
 * Update to make images (not animations) work
 *
 * Revision 2.16  2004/05/31 08:32:25  wmcoolmon
 * Custom HUD support, better loading, etc etc.
 *
 * Revision 2.15  2004/05/03 21:22:21  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.14  2004/04/25 06:31:52  Goober5000
 * made time dilation only available in cheat mode; also fixed an obscure CTD
 * --Goober5000
 *
 * Revision 2.13  2004/04/24 15:44:21  Kazan
 * Fixed Lobby screen showing up while exiting from non-tracker games
 * Time dialiation up to 1/4
 *
 * Revision 2.12  2004/03/28 17:49:54  taylor
 * runtime language selection, mantis:0000133
 *
 * Revision 2.11  2004/03/05 09:02:03  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.10  2003/09/13 20:59:54  Goober5000
 * fixed case-sensitivity bugs and possibly that Zeta wing bug
 * --Goober5000
 *
 * Revision 2.9  2003/08/21 08:31:24  Goober5000
 * fixed turret text display for non-laser weapons
 * --Goober5000
 *
 * Revision 2.8  2003/06/11 02:59:48  phreak
 * local ssm stuff for hud.
 * they are always in lock range due to the subspace drive on them
 * they also can't be targeted when in stage 3.
 *
 * Revision 2.7  2003/05/21 20:27:07  phreak
 * hud is drawn when in chase camera for player
 *
 * Revision 2.6  2003/04/29 01:03:23  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.5  2003/03/18 10:07:03  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.4  2003/01/19 07:02:15  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 2.3  2003/01/17 07:59:09  Goober5000
 * fixed some really strange behavior with strings not being truncated at the
 * # symbol
 * --Goober5000
 *
 * Revision 2.2  2002/10/17 20:40:50  randomtiger
 * Added ability to remove HUD ingame on keypress shift O
 * So I've added a new key to the bind list and made use of already existing hud removal code.
 *
 * Revision 2.1.2.1  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.6  2002/05/26 14:12:03  mharris
 * Changed alphacolors from automatic to static
 *
 * Revision 1.5  2002/05/13 15:11:03  mharris
 * More NO_NETWORK ifndefs added
 *
 * Revision 1.4  2002/05/10 20:42:43  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.3  2002/05/10 06:08:08  mharris
 * Porting... added ifndef NO_SOUND
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 58    10/28/99 2:04a Jefff
 * some german specific coords.  
 * 
 * 57    10/25/99 5:43p Jefff
 * added (and subsequently commented) some scoring debug code.  checked in
 * to work on later
 * 
 * 56    9/09/99 3:55a Andsager
 * Reset Hud_support_objnum to -1 when guage stops displaying
 * 
 * 55    9/01/99 11:16a Andsager
 * Fix bug where support ship guage would not show up if second support
 * ship called in whlile 1st one dying.
 * 
 * 54    8/23/99 1:49p Dave
 * Fixed damage popup (hopefully)
 * 
 * 53    8/23/99 11:34a Dave
 * Fixed shield intensity rendering problems.
 * 
 * 52    8/19/99 6:16p Jefff
 * 
 * 51    8/17/99 7:15p Jefff
 * auto-target & auto-speed text drawn in code
 * 
 * 50    8/16/99 4:04p Dave
 * Big honking checkin.
 * 
 * 49    8/09/99 3:47p Dave
 * Fixed incorrect nebula regeneration. Default HUD to low-contrast in
 * non-nebula missions.
 * 
 * 48    8/09/99 3:14p Dave
 * Make "launch" warning gauge draw in code.
 * 
 * 47    8/05/99 2:05a Dave
 * Fixes. Optimized detail level stuff.
 * 
 * 46    8/04/99 2:56p Jefff
 * fixed black box behind pilot head in hi-res
 * 
 * 45    8/04/99 9:54a Andsager
 * Auto target turrets on big ships.
 * 
 * 44    8/01/99 12:39p Dave
 * Added HUD contrast control key (for nebula).
 * 
 * 43    7/31/99 4:15p Dave
 * Fixed supernova particle velocities. Handle OBJ_NONE in target
 * monitoring view. Properly use objectives notify gauge colors.
 * 
 * 42    7/31/99 1:16p Dave
 * Use larger font for 1024 HUD flash text box. Make beam weapons aware of
 * weapon subsystem damage on firing ship.
 * 
 * 41    7/26/99 10:41a Jefff
 * added call to hud_maybe_show_damage() in hud_render_2d().  not sure how
 * this got out in the 1st place.
 * 
 * 40    7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 39    7/22/99 4:00p Dave
 * Fixed beam weapon muzzle glow rendering. Externalized hud shield info.
 * 
 * 38    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 37    7/21/99 3:19p Jefff
 * adjusted subspace and red alert popup text coords
 * 
 * 36    7/19/99 2:13p Dave
 * Added some new strings for Heiko.
 * 
 * 35    7/19/99 11:48a Jefff
 * Countermeasure success sound added
 * 
 * 34    7/16/99 12:22p Jefff
 * Added sound FX to objective popups
 * 
 * 33    7/15/99 7:16p Jefff
 * Red Alert box is now red
 * 
 * 32    7/09/99 12:00a Andsager
 * Added target box with distance for remote detonate weapons
 * 
 * 31    6/28/99 4:33p Jasenw
 * Fixed coords for hi res engine wash gauge
 * 
 * 30    6/11/99 11:13a Dave
 * last minute changes before press tour build.
 * 
 * 29    6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 28    6/08/99 1:14a Dave
 * Multi colored hud test.
 * 
 * 27    6/07/99 4:20p Andsager
 * Add HUD color for tagged object.  Apply to target and radar.
 * 
 * 26    5/28/99 5:36p Andsager
 * stupid comment
 * 
 * 25    5/28/99 10:00a Andsager
 * Make player hud target affected by Nebula range
 * 
 * 24    5/22/99 5:35p Dave
 * Debrief and chatbox screens. Fixed small hi-res HUD bug.
 * 
 * 23    5/21/99 5:36p Andsager
 * Put in high res engine wash gauge and approx coords
 * 
 * 22    5/21/99 1:44p Andsager
 * Add engine wash gauge
 * 
 * 21    4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 20    2/25/99 4:19p Dave
 * Added multiplayer_beta defines. Added cd_check define. Fixed a few
 * release build warnings. Added more data to the squad war request and
 * response packets.
 * 
 * 19    2/24/99 4:02p Dave
 * Fixed weapon locking and homing problems for multiplayer dogfight mode.
 * 
 * 18    2/17/99 2:10p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 17    2/03/99 8:37a Jasen
 * Fixed dock in coords
 * 
 * 16    2/01/99 9:24a Jasen
 * Fixed subspace and objectives displays for hi res.
 * 
 * 15    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 14    1/21/99 9:28p Dave
 * Fixed damage gauge coords.
 * 
 * 13    1/07/99 9:05a Jasen
 * coords, coords, coords
 * 
 * 12    1/06/99 3:24p Dave
 * Fixed stupid code.
 * 
 * 11    1/06/99 3:14p Jasen
 * more new coords
 * 
 * 10    1/06/99 2:33p Jasen
 * updated coords
 * 
 * 9     1/06/99 1:27p Dave
 * Removed duplicate global var.
 * 
 * 8     1/06/99 1:26p Dave
 * Put in seperate X coords for "dock in" and the associated time value
 * for the support ship gauge.
 * 
 * 7     12/28/98 3:17p Dave
 * Support for multiple hud bitmap filenames for hi-res mode.
 * 
 * 6     12/21/98 5:02p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 5     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 4     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 223   8/28/98 3:28p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 222   8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 221   8/09/98 4:45p Lawrance
 * center various HUD text - fixes problems in the German version
 * 
 * 220   6/18/98 10:10a Allender
 * fixed compiler warnings
 * 
 * 219   6/17/98 11:03a Lawrance
 * position subspace notify correctly for german version
 * 
 * 218   6/13/98 10:48p Lawrance
 * Changed code to utilize proper fixed-space 1 character.
 * 
 * 217   6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 216   6/12/98 2:49p Dave
 * Patch 1.02 changes.
 * 
 * 215   6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 214   6/01/98 11:43a John
 * JAS & MK:  Classified all strings for localization.
 * 
 * 213   5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 212   5/17/98 3:32p Lawrance
 * Allow red alert orders to get downloaded when in an out-of-cockpit view
 * 
 * 211   5/15/98 8:36p Lawrance
 * Add 'target ship that last sent transmission' target key
 * 
 * 210   5/10/98 5:28p Lawrance
 * Ensure hud messages and talking heads show up when viewing from another
 * ship
 * 
 * 209   5/10/98 12:11a Lawrance
 * Fix a couple of problems with 2D gauges showing up in external views
 * 
 * 208   5/09/98 4:52p Lawrance
 * Implement padlock view (up/rear/left/right)
 * 
 * 207   5/09/98 12:20a Lawrance
 * Show hud messages in all views
 * 
 * 206   5/08/98 5:32p Lawrance
 * Allow cargo scanning even if target gauge is disabled
 * 
 * 205   5/08/98 10:13a Lawrance
 * Don't allow targeting of ships that have SF_EXPLODED flag set
 * 
 * 204   5/07/98 1:01a Chad
 * Yet another hud gauage which shouldn't be rendered as a multiplayer
 * observer.
 * 
 * 203   5/04/98 12:08p Ed
 * from allender:  move hud_target_change_check() after code which does
 * possible auto target change.  Fixed multiplayer problem where locking
 * subsys does not match ship currently targeted
 * 
 * 202   5/04/98 6:12p Lawrance
 * Write generic function hud_end_string_at_first_hash_symbol(), to use in
 * various spots on the HUD
 * 
 * 201   4/30/98 3:32p Lawrance
 * Cull dead/departed ships from escort ship in hud_update_frame()
 * 
 * 200   4/23/98 10:24p Mike
 * Int3(), then recover gracefully from some error in which ship to be
 * repaired is killed. 
 * 
 * $NoKeywords: $
 *
*/


#include "hud/hud.h"
#include "asteroid/asteroid.h"
#include "freespace2/freespace.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "graphics/font.h"
#include "hud/hudconfig.h"
#include "hud/hudescort.h"
#include "hud/hudets.h"
#include "hud/hudlock.h"
#include "hud/hudmessage.h"
#include "hud/hudobserver.h"
#include "hud/hudreticle.h"
#include "hud/hudshield.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudtarget.h"
#include "hud/hudtargetbox.h"
#include "hud/hudwingmanstatus.h"
#include "hud/hudparse.h"
#include "object/objectdock.h"
#include "render/3dinternal.h"
#include "hud/hudnavigation.h"	//kazan

#include "io/timer.h"
#include "localization/localize.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "mission/missiontraining.h"
#include "missionui/redalert.h"
#include "model/model.h"
#include "object/object.h"
#include "playerman/player.h"
#include "radar/radar.h"
#include "render/3d.h"
#include "ai/aigoals.h"
#include "ship/ship.h"
#include "starfield/supernova.h"
#include "weapon/emp.h"
#include "weapon/weapon.h"
#include "radar/radarsetup.h"

#ifndef NO_NETWORK
#include "network/multiutil.h"
#include "network/multi_voice.h"
#include "network/multi_pmsg.h"
#endif



// new values for HUD alpha
#define HUD_NEW_ALPHA_DIM				80	
#define HUD_NEW_ALPHA_NORMAL			120
#define HUD_NEW_ALPHA_BRIGHT			220

// high contrast
#define HUD_NEW_ALPHA_DIM_HI			130
#define HUD_NEW_ALPHA_NORMAL_HI		190
#define HUD_NEW_ALPHA_BRIGHT_HI		255

// This is used to prevent drawing of hud (and pause popup)

// globals that will control the color of the HUD gauges
int HUD_color_red = 0;
int HUD_color_green = 255;
int HUD_color_blue = 0;
int HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;		// 1 -> HUD_COLOR_ALPHA_USER_MAX

int HUD_draw     = 1;
int HUD_contrast = 0;										// high or lo contrast (for nebula, etc)

// Goober5000
int HUD_disable_except_messages = 0;

color HUD_color_defaults[HUD_NUM_COLOR_LEVELS];		// array of colors with different alpha blending
color HUD_color_debug;										// grey debug text shown on HUD

static int Player_engine_snd_loop = -1;

// animations for damages gauges
hud_anim Target_static;
hud_anim	Radar_static;

// HUD render frame offsets
float HUD_offset_x = 0.0f;
float HUD_offset_y = 0.0f;

// Global: integrity of player's target
float Pl_target_integrity;

static int Hud_last_can_target;	// whether Player is able to target in the last frame
static int Hud_can_target_timer;	// timestamp to allow target gauge to draw static once targeting functions are not allowed

// centered text message gauges (collision, emp, etc)
char Hud_text_flash[512] = "";
int Hud_text_flash_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		-1, 172
	},
	{ // GR_1024
		-1, 275
	}
};
void hud_init_text_flash_gauge();
void hud_start_text_flash(char *txt, int t);
void hud_maybe_show_text_flash_icon();


// multiplayer messaging text
int Multi_msg_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		5, 150
	},
	{ // GR_1024
		8, 240
	}
};

// multiplayer voice stuff
int Voice_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		5, 165
	},
	{ // GR_1024
		8, 255
	}
};

// redalert downloading new orders text
int Red_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		-1, 116
	},
	{ // GR_1024
		-1, 186
	}
};
int Red_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		-1, 124
	},
	{ // GR_1024
		-1, 194
	}
};

// subspace popup
int Subspace_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		-1, 116
	},
	{ // GR_1024
		-1, 186
	}
};
int Subspace_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		100, 124
	},
	{ // GR_1024
		140, 194
	}
};

// message text coords
int Head_message_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		7, 37
	},
	{ // GR_1024
		11, 57
	}
};

// ping text coords
int Ping_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		560, 3
	},
	{ // GR_1024
		896, 5
	}
};

// supernova coords
int Supernova_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		100, 100
	},
	{ // GR_1024
		170, 170
	}
};
	
// used to draw the netlag icon on the HUD
hud_frames Netlag_icon;
int Netlag_icon_loaded=0;
int Netlag_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		386, 331
	},
	{ // GR_1024
		627, 529
	}
};
char Netlag_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"netlag1",
	"netlag1"
};

// used to draw the kills gauge
hud_frames Kills_gauge;
int Kills_gauge_loaded = 0;
int Kills_gauge_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		497, 361
	},
	{ // GR_1024
		880, 624
	}
};
int Kills_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		503, 365
	},
	{ // GR_1024
		886, 628
	}
};

int Kills_text_val_coords_gr[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		615, 365
	},
	{ // GR_1024
		984, 628
	}
};

int Kills_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		571, 365
	},
	{ // GR_1024
		954, 628
	}
};

char Kills_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"kills1",
	"kills1"
};

// used to draw border around a talking head
static hud_frames Head_frame_gauge;
static int Head_frame_gauge_loaded = 0;
int Head_frame_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		5, 35
	},
	{ // GR_1024
		5, 56
	}
};
char Head_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"head1",
	"head1"
};

// mission time frame
static hud_frames Mission_time_gauge;
static int Mission_time_gauge_loaded = 0;
int Mission_time_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		587, 448
	},
	{ // GR_1024
		969, 716
	}
};
int Mission_time_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		591, 452
	},
	{ // GR_1024
		973, 720
	}
};
int Mission_time_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		613, 460
	},
	{ // GR_640
		995, 728
	}
};
char Mission_time_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"time1",
	"time1"
};

// used to draw the hud support view
static hud_frames Support_view_gauge;
static int Support_view_gauge_loaded = 0;
static int Hud_support_view_active;
static int Hud_support_view_abort;		// active when we need to display abort message
static int Hud_support_view_fade;		// timer
static int Hud_support_obj_sig, Hud_support_objnum, Hud_support_target_sig;
int Support_view_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		265, 334
	},
	{ // GR_1024
		459, 534
	}
};
int Support_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		267, 335
	},
	{ // GR_1024
		462, 536
	}
};
int Support_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		-1, 348
	},
	{ // GR_1024
		-1, 546
	}
};
int Support_text_dock_coords[GR_NUM_RESOLUTIONS][2] = {			// "dock in" x coord
	{ // GR_640
		270, -1
	},
	{ // GR_1024
		465, -1
	}
};
int Support_text_dock_val_coords[GR_NUM_RESOLUTIONS][2] = {		// time value for "dock in" x coord
	{ // GR_640
		328, -1
	},
	{ // GR_1024
		524, -1
	}
};
char Support_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"support1",
	"support1"
};

// damage gauge stuff
#define NUM_DAMAGE_GAUGES	3
static hud_frames Damage_gauges[NUM_DAMAGE_GAUGES];
static int Damage_gauges_loaded = 0;
char *Damage_gauge_fnames[GR_NUM_RESOLUTIONS][NUM_DAMAGE_GAUGES] = 
{
	//XSTR:OFF
	{ // GR_640
		"damage1",
		"damage2",
		"damage3",
	},
	{ // GR_1024
		"damage1",
		"damage2",
		"damage3",
	}
//XSTR:ON
};
int Damage_gauge_line_h[GR_NUM_RESOLUTIONS] = { 
	9, 
	9
};
int Damage_gauge_coords[GR_NUM_RESOLUTIONS][2][2] = {
	{ // GR_640
		{245, 38},
		{245, 63}
	},
	{ // GR_1024
		{440, 61},
		{440, 86}
	}
	// These #'s seem to work, although I really don't know why. Frankly, it frightens me,
	// because it means the 640 coords _shouldn't_. This may be due to D3D strangeness, so
	// we'll have to investigate when we get hi-res Glide in.
};
int Damage_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		248, 40
	},
	{ // GR_1024
		443, 63
	}
};
int Hull_integ_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		249, 53
	},
	{ // GR_1024
		444, 76
	}
};
int Hull_integ_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		387, 53
	},
	{ // GR_1024
		582, 76
	},
};
int Damage_subsys_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		249, 65
	},
	{ // GR_1024
		444, 88
	}
};


// flashing gauges
#define HUD_GAUGE_FLASH_DURATION		5000
#define HUD_GAUGE_FLASH_INTERVAL		200
int HUD_gauge_flash_duration[NUM_HUD_GAUGES];
int HUD_gauge_flash_next[NUM_HUD_GAUGES];
int HUD_gauge_bright;

// Objective display
typedef struct objective_display_info
{
	int display_timer;
	int goal_type;
	int goal_status;
	int goal_ntotal;
	int goal_nresolved;

} objective_display_info;

static objective_display_info Objective_display;

static int			Objective_display_gauge_inited=0;
static hud_frames	Objective_display_gauge;
int Objective_display_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		245, 114
	},
	{ // GR_1024
		436, 184
	}
};
int Objective_text_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		-1, 116
	},
	{ // GR_1024
		-1, 186
	}
};
int Objective_text_val_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		-1, 125
	},
	{ // GR_1024
		-1, 195
	}
};
char Objective_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"objective1",
	"objective1"
};

// Subspace notify display
static int Subspace_notify_active;
static int Objective_notify_active;
static int HUD_abort_subspace_timer = 1;

// used to track how player subsystems are getting damaged
typedef struct hud_subsys_info
{
	float	last_str;
	int	flash_duration_timestamp;
} hud_subsys_info;

static hud_subsys_info	Pl_hud_subsys_info[SUBSYSTEM_MAX];
static int					Pl_hud_next_flash_timestamp;
static int					Pl_hud_is_bright;

#define SUBSYS_DAMAGE_FLASH_DURATION	1800
#define SUBSYS_DAMAGE_FLASH_INTERVAL	100

// timers used for popup gauges
int HUD_popup_timers[NUM_HUD_GAUGES];

// forward declarations
void update_throttle_sound();
void hud_show_damage_popup();
void hud_damage_popup_init();
void hud_support_view_init();
void hud_gauge_flash_init();
void hud_objective_message_init();
void hud_maybe_display_objective_message();
void hud_stop_subspace_notify();
void hud_start_subspace_notify();
void hud_stop_objective_notify();
void hud_start_objective_notify();
int hud_subspace_notify_active();
int hud_objective_notify_active();
void hud_subspace_notify_abort();
void hud_maybe_display_subspace_notify();
void hud_init_netlag_icon();
void hud_maybe_show_netlag_icon();
void hud_maybe_display_red_alert();
void hud_init_kills_gauge();
void hud_show_kills_gauge();
int hud_maybe_render_emp_icon();
void hud_init_emp_icon();
void hud_maybe_blit_head_border();

//	Saturate a value in minv..maxv.
void saturate(int *i, int minv, int maxv)
{
	if (*i < minv)
		*i = minv;
	else if (*i > maxv)
		*i = maxv;
}

// init the colors used for the different shades of the HUD
void HUD_init_hud_color_array()
{
	int i;

	for ( i = 0; i < HUD_NUM_COLOR_LEVELS; i++ ) {
		gr_init_alphacolor( &HUD_color_defaults[i], HUD_color_red, HUD_color_green, HUD_color_blue, (i+1)*16 );
	}
}

// HUD_init will call all the various HUD gauge init functions.  This function is called at the
// start of each mission (level)
void HUD_init_colors()
{
	saturate(&HUD_color_red, 0, 255);
	saturate(&HUD_color_green, 0, 255);
	saturate(&HUD_color_blue, 0, 255);
	saturate(&HUD_color_alpha, 0, HUD_COLOR_ALPHA_USER_MAX);

	gr_init_alphacolor( &HUD_color_debug, 128, 255, 128, HUD_color_alpha*16 );
	HUD_init_hud_color_array();

	hud_init_targeting_colors();
	hud_gauge_flash_init();
}

// The following global data is used to determine if we should change the engine sound.
// We only check if the throttle has changed every THROTTLE_SOUND_CHECK_INTERVAL ms, and
// then we make sure that the throttle has actually changed.  If it has changed, we start
// a new sound and/or adjust the volume.  This occurs in update_throttle_sound()
//
static float last_percent_throttle;
#define THROTTLE_SOUND_CHECK_INTERVAL	50	// in ms
static int throttle_sound_check_id;

// used for the display of damaged subsystems
typedef struct hud_subsys_damage
{
	int	str;
	int	type;
	char	*name;
} hud_subsys_damage;

#define DAMAGE_FLASH_TIME 150
static int Damage_flash_bright;
static int Damage_flash_timer;

// initialize the timers used for popup gauges
void hud_init_popup_timers()
{
	int i;
	for (i=0; i<NUM_HUD_GAUGES; i++) {
		HUD_popup_timers[i] = timestamp(0);
	}
}

// Load in the bitmap for the talking head gauge if required
void hud_init_talking_head_gauge()
{
	// ensure the talking head border is loaded
	if ( !Head_frame_gauge_loaded ) {
		Head_frame_gauge.first_frame = bm_load_animation(Head_fname[gr_screen.res], &Head_frame_gauge.num_frames);
		if ( Head_frame_gauge.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in ani: Head_fname[gr_screen.res]\n");
		}
		Head_frame_gauge_loaded = 1;
	}
}

// Load in the bitmap for the mission time gauge if required
void hud_init_mission_time_gauge()
{
	// ensure the talking head border is loaded
	if ( !Mission_time_gauge_loaded ) {
		Mission_time_gauge.first_frame = bm_load_animation(Mission_time_fname[gr_screen.res], &Mission_time_gauge.num_frames);
		if ( Mission_time_gauge.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in ani: Mission_time_fname[gr_screen.res]\n");
		}
		Mission_time_gauge_loaded = 1;
	}
}

// ----------------------------------------------------------------------
// HUD_init()
//
// Called each level to initalize HUD systems
//
void HUD_init()
{
	HUD_init_colors();
	hud_init_msg_window();
	hud_init_targeting();
	hud_init_reticle();
	hud_shield_level_init();
	hud_init_ets();
	hud_targetbox_init();
	hud_escort_init();
	hud_damage_popup_init();
	hud_support_view_init();
	hud_init_squadmsg();		// initialize the vars needed for squadmate messaging
	hud_init_popup_timers();
	hud_objective_message_init();
	hud_init_wingman_status_gauge();
	hud_anim_init(&Target_static, Target_window_coords[gr_screen.res][0], Target_window_coords[gr_screen.res][1], NOX("TargetStatic"));
	hud_targetbox_static_init();
	hud_init_text_flash_gauge();
	hud_init_netlag_icon();	
	hud_init_talking_head_gauge();
	hud_init_mission_time_gauge();
	hud_init_kills_gauge();
	hud_stop_subspace_notify();
	hud_stop_objective_notify();
	hud_target_last_transmit_level_init();

	throttle_sound_check_id = timestamp(THROTTLE_SOUND_CHECK_INTERVAL);
	HUD_abort_subspace_timer = 1;
	Hud_last_can_target = 1;
	Hud_can_target_timer = 1;
	last_percent_throttle = 0.0f;

	// default to high contrast in the nebula
	HUD_contrast = 0;
	HUD_draw     = 1;
	HUD_disable_except_messages = 0;

	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		HUD_contrast = 1;
	} 
}

void hud_toggle_draw()
{
	HUD_draw = !HUD_draw;
}

// Goober5000
void hud_set_draw(int draw)
{
	HUD_draw = draw;
}

// Goober5000
void hud_disable_except_messages(int disable)
{
	HUD_disable_except_messages = disable;
}

// Goober5000
// like hud_disabled, except messages are still drawn
int hud_disabled_except_messages()
{
	return HUD_disable_except_messages;
}

// return !0 if HUD is disabled (ie no gauges are shown/usable), otherwise return 0
int hud_disabled()
{
	return !HUD_draw;
}

// Determine if we should popup the weapons gauge on the HUD.
void hud_maybe_popup_weapons_gauge()
{
	if ( hud_gauge_is_popup(HUD_WEAPONS_GAUGE) ) {
		ship_weapon *swp = &Player_ship->weapons;
		int			i;

		for ( i = 0; i < swp->num_secondary_banks; i++ ) {
			if ( swp->secondary_bank_ammo[i] > 0 ) {
				int ms_till_fire = timestamp_until(swp->next_secondary_fire_stamp[i]);
				if ( ms_till_fire >= 1000 ) {
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE, 2500);
				}
			}
		}
	}
}

// hud_update_frame() will update hud systems
//
// This function updates those parts of the hud that are not dependant on the
// rendering of the hud.
void hud_update_frame()
{
	object	*targetp;
	int		can_target;

	update_throttle_sound();
	hud_check_reticle_list();
	hud_wingman_status_update();

	// Check hotkey selections to see if any ships need to be removed
	hud_prune_hotkeys();

	// Remove dead/departed ships from the escort list
	hud_escort_cull_list();

	hud_update_reticle( Player );
	hud_shield_hit_update();
	hud_maybe_popup_weapons_gauge();	

	// if emp is active we have to allow targeting by the "random emp" system
	// we will intercept player targeting requests in hud_sensors_ok() when checking key commands
	// DB - 8/24/98
	can_target = hud_sensors_ok(Player_ship, 0);
	if(emp_active_local()){
		can_target = 1;
	}
	if ( !can_target && Hud_last_can_target ) {
		Hud_can_target_timer = timestamp(1200);		
	}
	Hud_last_can_target = can_target;

	if ( timestamp_elapsed(Hud_can_target_timer) ) {
		if ( (Player_ai->target_objnum != -1) && !can_target ){
			Player_ai->target_objnum = -1;
		}
	}

	// if there is no target, check if auto-targeting is enabled, and select new target
	int retarget = 0;
	int retarget_turret = 0;

	if (Player_ai->target_objnum == -1){
		retarget = 1;
	} else if (Objects[Player_ai->target_objnum].type == OBJ_SHIP) {
		if (Ships[Objects[Player_ai->target_objnum].instance].flags & SF_DYING){
			if (timestamp_elapsed(Ships[Objects[Player_ai->target_objnum].instance].final_death_time)) {
				retarget = 1;
			}
		}
	}

	// check if big ship and currently selected subsys is turret and turret is dead
	// only do this is not retargeting
	if ((!retarget) && (Player_ai->target_objnum != -1)) {
		if (Objects[Player_ai->target_objnum].type == OBJ_SHIP) {
			if ( !(Ships[Objects[Player_ai->target_objnum].instance].flags & SF_DYING) ) {
				if ( Ship_info[Ships[Objects[Player_ai->target_objnum].instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP) ) {
					ship_subsys *ss = Player_ai->targeted_subsys;
					if (ss != NULL) {
						if ((ss->system_info->type == SUBSYSTEM_TURRET) && (ss->current_hits == 0)) {
							retarget_turret = 1;
						}
					}
				}
			}
		}
	}

	if ( retarget && can_target ) {
		Player_ai->current_target_is_locked = 0;
		if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_TARGETING ) {
			Player_ai->target_objnum = -1;
			hud_target_auto_target_next();
		}
	}

	if (retarget_turret && can_target) {
		Assert(!retarget);
		// get closest weighted live turret
		// hud_target_closest(OBJ_INDEX(Player_obj), FALSE, FALSE);
		void hud_update_closest_turret();
		hud_update_closest_turret();
	}

	hud_target_change_check();

	if (Player_ai->target_objnum == -1) {
		if ( Target_static_looping != -1 ) {
			snd_stop(Target_static_looping);
		}
		return;
	}

	targetp = &Objects[Player_ai->target_objnum];


	int stop_targetting_this_thing = 0;

	// check to see if the target is still alive
	if ( targetp->flags&OF_SHOULD_BE_DEAD ) {
		stop_targetting_this_thing = 1;
	}

	Player->target_is_dying = FALSE;
	ship	*target_shipp = NULL;
	
	if ( targetp->type == OBJ_SHIP ) {
		Assert ( targetp->instance >=0 && targetp->instance < MAX_SHIPS );
		target_shipp = &Ships[targetp->instance];
		Player->target_is_dying = target_shipp->flags & SF_DYING;

		// If it is warping out (or exploded), turn off targeting
		if ( target_shipp->flags & (SF_DEPART_WARP|SF_EXPLODED) ) {
			stop_targetting_this_thing = 1;
		}
	}

	// Check if can still be seen in Nebula
	if ( hud_target_invalid_awacs(targetp) ) {
		stop_targetting_this_thing = 1;
	}

	// If this was found to be something we shouldn't
	// target anymore, just remove it
	if ( stop_targetting_this_thing )	{
		Player_ai->target_objnum = -1;
		Player_ai->targeted_subsys = NULL;
		hud_stop_looped_locking_sounds();
	}
	
	if (Player->target_is_dying) {
		hud_stop_looped_locking_sounds();
		if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_TARGETING ) {
			hud_target_auto_target_next();
		}
	}

	#ifndef NO_SOUND
	// Switch to battle track when a targeted ship is hostile and within BATTLE_START_MIN_TARGET_DIST
	if (targetp->type == OBJ_SHIP && Event_Music_battle_started == 0 ) {
		Assert( target_shipp != NULL );

		if (opposing_team_mask(Player_ship->team)) {
			float	dist_to_target;

			dist_to_target = vm_vec_dist_quick(&targetp->pos, &Player_obj->pos);
			if (dist_to_target < BATTLE_START_MIN_TARGET_DIST) {

				// If the target has an AI class of none, it is a Cargo, NavBuoy or other non-aggressive
				// ship, so don't start the battle music	
				if (stricmp(Ai_class_names[Ai_info[target_shipp->ai_index].ai_class], NOX("none")))
					event_music_battle_start();
			}
		}
	}
	#endif

	// Since we need to reference the player's target integrity in several places this upcoming 
	// frame, only calculate once here
	if ( target_shipp ) {
		Pl_target_integrity = get_hull_pct(targetp);
	}

	hud_update_cargo_scan_sound();

}

void HUD_render_forward_icon(object *objp)
{
	vertex	v0;
	vec3d	p0;

	vm_vec_scale_add(&p0, &objp->pos, &objp->orient.vec.fvec, 100.0f);
	g3_rotate_vertex(&v0, &p0);

	gr_set_color(255, 0, 0);
	if ((!(v0.flags & PF_OVERFLOW)) && (v0.codes == 0)) // make sure point projected
		g3_draw_sphere(&v0, 1.25f);
	else if (v0.codes != 0) { // target center is not on screen
		// draw the offscreen indicator at the edge of the screen where the target is closest to
		hud_draw_offscreen_indicator(&v0, &p0);
	}
}

// Draw white brackets around asteroids which has the AF_DRAW_BRACKETS flag set
void hud_show_asteroid_brackets()
{
	if ( hud_sensors_ok(Player_ship, 0) ) {
		asteroid_show_brackets();
	}
}

// Draw radar gauge on the HUD
void hud_show_radar()
{
	if ( hud_disabled_except_messages() ) {
		return;
	}

	if ( hud_disabled() ) {
		return;
	}

	if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED | /*VM_CHASE |*/ VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY ))) {
		if ( Game_detail_flags & DETAIL_FLAG_HUD )	{
			if ( hud_gauge_active(HUD_RADAR) ) {
				HUD_reset_clip();
				radar_frame_render(flFrametime);
			}
		}
	}
}

// Render model of target in the target view box
void hud_show_target_model()
{
	if ( hud_disabled_except_messages() ) {
		return;
	}

	if ( hud_disabled() ) {
		return;
	}

	// display the miniature model of the target in the target box and shade
	// RT Might be faster to use full detail model
	if ( Game_detail_flags & DETAIL_FLAG_HUD )	{
		if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED | /*VM_CHASE |*/ VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY)))
			hud_render_target_model();
	}
}

void hud_show_common_3d_gauges(float frametime, int in_cockpit)
{
	// draw the targeting data around any message sender
	hud_show_message_sender();

	// if messages are disabled then skip everything else
	if ( hud_disabled_except_messages() ) {
		return;
	}

	// Draw Navigation stuff
	HUD_Draw_Navigation();

	// draw boxes around current selection set, if any
	hud_show_selection_set();

	// draw brackets around asteroids is necessary
	hud_show_asteroid_brackets();


	// draw targetting data around the current target
	hud_show_targeting_gauges(frametime, in_cockpit);



	// draw brackets and distance to remote detonate missile
	hud_show_remote_detonate_missile();
}

// Render gauges that need to be between a g3_start_frame() and a g3_end_frame()
void HUD_render_3d(float frametime)
{
	Player->subsys_in_view = -1;

	if ( hud_disabled() ) {
		return;
	}

	if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED |/* VM_CHASE |*/ VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY))) {

		hud_show_common_3d_gauges(frametime, 1);

		//	Show all homing missiles locked onto the player.
		//	Currently not supporting a way to toggle this as I'm not sure we'll stick wtih this gauge. -- MK, 3/17/97.
		if ( hud_gauge_active(HUD_MISSILE_WARNING_ARROW) && !hud_disabled_except_messages() ) {
			hud_show_homing_missiles();
		}

	} else if ( Viewer_mode & (/*VM_CHASE |*/ VM_EXTERNAL | VM_WARP_CHASE | VM_PADLOCK_ANY ) ) {
		// If the player is warping out, don't draw the targeting gauges
		Assert(Player != NULL);
		if ( Player->control_mode != PCM_NORMAL ) {
			return;
		}

		hud_show_common_3d_gauges(frametime, 0);
	}

	if (Viewer_mode & VM_SLEWED) {
		HUD_render_forward_icon(Player_obj);
	}

}


// call from HUD_render_2d() when in gameplay, and call when in navmap
void hud_show_messages()
{
	// draw the message window
	hud_show_msg_window();
	hud_show_fixed_text();
}

// decide if we want to blit damage status to the screen
void hud_maybe_show_damage()
{
	if ( !hud_gauge_active(HUD_DAMAGE_GAUGE) ) {
		return;
	}

	// display the current weapon info for the player ship, with ammo/energy counts
	if ( hud_gauge_active(HUD_DAMAGE_GAUGE) ) {
		int show_gauge_flag;

		if ( (Player_ship->ship_max_hull_strength - Player_obj->hull_strength) > 1.0f ) {
			show_gauge_flag = 1;
		} else {
			show_gauge_flag = 0;
		}

		// is gauge configured as a popup?
		if ( hud_gauge_is_popup(HUD_DAMAGE_GAUGE) ) {
			if ( !hud_gauge_popup_active(HUD_DAMAGE_GAUGE) ) {
				show_gauge_flag=0;
			}
		}
			
		if ( show_gauge_flag ) {
			hud_show_damage_popup();
		}
	}
}

// The damage toggle button was pressed, so change state
void hud_damage_popup_toggle()
{
	snd_play(&Snds[SND_SQUADMSGING_ON]);
	
	// If gague is disabled (off), make it on all the time
	if ( !hud_gauge_active(HUD_DAMAGE_GAUGE) ) {
		hud_config_set_gauge_flags(HUD_DAMAGE_GAUGE,1,0);		
		return;
	}

	// if gauge is popup, turn it off if it is current up, otherwise force it to be up
	if ( hud_gauge_is_popup(HUD_DAMAGE_GAUGE) ) {
		if ( Player_obj->hull_strength == Player_ship->ship_max_hull_strength ) {
			hud_config_set_gauge_flags(HUD_DAMAGE_GAUGE,1,0);		
		} else {
			hud_config_set_gauge_flags(HUD_DAMAGE_GAUGE,0,0);		
		}
		return;
	}

	// gauge is on, without any popup... so force it to be off
	hud_config_set_gauge_flags(HUD_DAMAGE_GAUGE,0,0);		
}


// Display the current mission time in MM:SS format
void hud_show_mission_time()
{
	float mission_time, time_comp;
	int minutes=0;
	int seconds=0;
	
	mission_time = f2fl(Missiontime);  // convert to seconds

	minutes=(int)(mission_time/60);
	seconds=(int)mission_time%60;

	hud_set_gauge_color(HUD_MISSION_TIME);

	// blit background frame
	if ( Mission_time_gauge.first_frame >= 0 ) {
		GR_AABITMAP(Mission_time_gauge.first_frame, Mission_time_coords[gr_screen.res][0], Mission_time_coords[gr_screen.res][1]);				
	}

	// print out mission time in MM:SS format
	gr_printf(Mission_time_text_coords[gr_screen.res][0], Mission_time_text_coords[gr_screen.res][1], NOX("%02d:%02d"), minutes, seconds);

	// display time compression as xN
	time_comp = f2fl(Game_time_compression);
	if ( time_comp < 1 ) {
		gr_printf(Mission_time_text_val_coords[gr_screen.res][0], Mission_time_text_val_coords[gr_screen.res][1], /*XSTR( "x%.1f", 215), time_comp)*/ NOX("%.2f"), time_comp);
	} else {
		gr_printf(Mission_time_text_val_coords[gr_screen.res][0], Mission_time_text_val_coords[gr_screen.res][1], XSTR( "x%.2f", 216), time_comp);
	}
}

// If a head animation is playing, then blit a border around it
void hud_maybe_blit_head_border()
{
	if ( Head_frame_gauge.first_frame == -1 ){
		return;
	}

	if ( message_anim_is_playing() ) {
		// draw frame
		// hud_set_default_color();
		hud_set_gauge_color(HUD_TALKING_HEAD);

		GR_AABITMAP(Head_frame_gauge.first_frame, Head_frame_coords[gr_screen.res][0], Head_frame_coords[gr_screen.res][1]);		

		// draw title
		gr_string(Head_message_coords[gr_screen.res][0], Head_message_coords[gr_screen.res][1], XSTR("message", 217));
	}
}

// Black out area behind head animation
void hud_maybe_clear_head_area()
{
	if ( Head_frame_gauge.first_frame == -1 ) {
		return;
	}

	if ( message_anim_is_playing() ) {
		// clear
		if (gr_screen.res == GR_640) {
			HUD_set_clip(7, 45, 160, 120);		// these coords are set in MissionMessage.cpp
		} else {
			HUD_set_clip(7, 66, 160, 120);
		}
		gr_clear();
		HUD_reset_clip();
	}
}

void hud_maybe_display_supernova()
{
	float time_left;

	// if there's a supernova coming
	time_left = supernova_time_left();
	if(time_left < 0.0f){
		return;
	}

	gr_set_color_fast(&Color_bright_red);
	gr_printf(Supernova_coords[gr_screen.res][0], Supernova_coords[gr_screen.res][1], "Supernova Warning: %.2f s", time_left);
}

#ifndef NO_NETWORK
// render multiplayer ping time to the server if appropriate
void hud_render_multi_ping()
{
	// if we shouldn't be displaying a ping time, return here
	if(!multi_show_ingame_ping()){
		return;
	}
	
	// if we're in multiplayer mode, display our ping time to the server
	if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		char ping_str[50];
		memset(ping_str,0,50);

		// if our ping is positive, display it
		if((Netgame.server != NULL) && (Netgame.server->s_info.ping.ping_avg > 0)){
			// get the string
			if(Netgame.server->s_info.ping.ping_avg >= 1000){
				sprintf(ping_str,XSTR("> 1 sec",628));
			} else {
				sprintf(ping_str,XSTR("%d ms",629),Netgame.server->s_info.ping.ping_avg);
			}

			// blit the string out
			hud_set_default_color();
			gr_string(Ping_coords[gr_screen.res][0], Ping_coords[gr_screen.res][1], ping_str);
		}
	}
}
#endif  // ifndef NO_NETWORK

// render all the 2D gauges on the HUD
void HUD_render_2d(float frametime)
{
	int show_gauge_flag;

	HUD_reset_clip();

	//make sure that the player isn't targeting a 3rd stage local ssm
	if (Player_ai->target_objnum > -1)
	{
		if (Objects[Player_ai->target_objnum].type == OBJ_WEAPON)
		{
			if (Weapons[Objects[Player_ai->target_objnum].instance].lssm_stage==3)
			{
				set_target_objnum( Player_ai, -1 );
				hud_lock_reset();
			}
		}
	}

/*
	// show some scoring debug stuff
	{
		extern char Scoring_debug_text[];
		gr_string( 10, 40, Scoring_debug_text );
	}
*/

	// Goober5000 - special case... hud is off, but we're still displaying messages
	if ( hud_disabled_except_messages() )
	{
		if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED |/* VM_CHASE |*/ VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY ))) {
			// draw a border around a talking head if it is playing
			hud_maybe_blit_head_border();

			// show the directives popup and/or training popup
			message_training_display();
		}

		hud_show_messages();

		return;
	}

	if ( hud_disabled() )
	{
		return;
	}

	//Custom hud stuff
#ifdef NEW_HUD
	//Player_ship->ship_hud.show();
	//	int i;
	//	gauge_data* cg;
/*	for(i = 0; i < current_hud->num_gauges; i++)
	{
		if(current_hud->gauges[i].type == HG_MAINGAUGE)
		{
			current_hud->gauges[i].update(current_hud->owner);
		}
	}*/
#else
	int i;
	static bool image_ids_set = false;
	static hud_frames image_ids[MAX_CUSTOM_HUD_GAUGES];
	if(!image_ids_set)
	{
		for(i = 0; i < Num_custom_gauges; i++)
		{
			if(strlen(current_hud->custom_gauge_images[i]))
			{
				image_ids[i].first_frame = bm_load_animation(current_hud->custom_gauge_images[i], &image_ids[i].num_frames);
				if(image_ids[i].first_frame != -1)
				{
					bm_page_in_aabitmap( image_ids[i].first_frame, image_ids[i].num_frames );
				}
				else
				{
					image_ids[i].first_frame = bm_load(current_hud->custom_gauge_images[i]);
				}
			}
			else
			{
				image_ids[i].first_frame = -1;
			}
		}
		image_ids_set = true;
	}

	//Display the gauges
	for(i = 0; i < Num_custom_gauges; i++)
	{
		if(current_hud->custom_gauge_colors[i].red != 0 || current_hud->custom_gauge_colors[i].green != 0 || current_hud->custom_gauge_colors[i].blue != 0)
		{
			//No custom alpha gauge color...
			gr_init_alphacolor(&current_hud->custom_gauge_colors[i], current_hud->custom_gauge_colors[i].red, current_hud->custom_gauge_colors[i].green, current_hud->custom_gauge_colors[i].blue, (HUD_color_alpha+1)*16);

			gr_set_color_fast(&current_hud->custom_gauge_colors[i]);
		}
		if(strlen(current_hud->custom_gauge_text[i]))
		{
			hud_num_make_mono(current_hud->custom_gauge_text[i]);
			gr_string(current_hud->custom_gauge_coords[i][0], current_hud->custom_gauge_coords[i][1], current_hud->custom_gauge_text[i]);
		}
		if(image_ids[i].first_frame != -1)
		{
			GR_AABITMAP(image_ids[i].first_frame + current_hud->custom_gauge_frames[i], current_hud->custom_gauge_coords[i][0], current_hud->custom_gauge_coords[i][1]);
		}

		//So we're back to normal
		hud_set_default_color();
	}
#endif

	if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED |/* VM_CHASE |*/ VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY )))
	{
		// display Energy Transfer System gauges
		if ( hud_gauge_active(HUD_ETS_GAUGE) ) {
			show_gauge_flag=1;
			// is gauge configured as a popup?
			if ( hud_gauge_is_popup(HUD_ETS_GAUGE) ) {
				if ( !hud_gauge_popup_active(HUD_ETS_GAUGE) ) {
					show_gauge_flag=0;
				}
			}
			
			if ( show_gauge_flag ) {
				hud_show_ets();
			}
		}

		// display afterburner fuel gauge
		if ( hud_gauge_active(HUD_AFTERBURNER_ENERGY) ) {
			hud_set_gauge_color(HUD_AFTERBURNER_ENERGY);
			hud_show_afterburner_gauge();
		}		

		// text flash gauge
		hud_maybe_show_text_flash_icon();

#ifndef NO_NETWORK
		// maybe show the netlag icon
		if(Game_mode & GM_MULTIPLAYER){
			hud_maybe_show_netlag_icon();

			if(Net_player->flags & NETINFO_FLAG_OBSERVER){
				hud_render_observer();					
			}
		}
#endif

		// draw the reticle
		hud_show_reticle();


		// display info on the ships in the escort list
		if ( hud_gauge_active(HUD_ESCORT_VIEW) ) {
			show_gauge_flag=1;
			// is gauge configured as a popup?
			if ( hud_gauge_is_popup(HUD_ESCORT_VIEW) ) {
				if ( !hud_gauge_popup_active(HUD_ESCORT_VIEW) ) {
					show_gauge_flag=0;
				}
			}
			
			if ( show_gauge_flag ) {
				hud_set_gauge_color(HUD_ESCORT_VIEW);
				hud_display_escort();
			}
		}

		// display the current weapon info for the player ship, with ammo/energy counts
		if ( hud_gauge_active(HUD_WEAPONS_GAUGE) ) {
			show_gauge_flag=1;
			// is gauge configured as a popup?
			if ( hud_gauge_is_popup(HUD_WEAPONS_GAUGE) ) {
				if ( !hud_gauge_popup_active(HUD_WEAPONS_GAUGE) ) {
					show_gauge_flag=0;
				}
			}
			
			if ( show_gauge_flag ) {
				hud_show_weapons();
			}
		}

		// display player countermeasures count
		if ( hud_gauge_active(HUD_CMEASURE_GAUGE) ) {
			show_gauge_flag=1;
			// is gauge configured as a popup?
			if ( hud_gauge_is_popup(HUD_CMEASURE_GAUGE) ) {
				if ( !hud_gauge_popup_active(HUD_CMEASURE_GAUGE) ) {
					show_gauge_flag=0;
				}
			}
			
			if ( show_gauge_flag ) {
				hud_show_cmeasure_gague();
			}
		}

		if ( hud_gauge_active(HUD_WEAPONS_ENERGY) ) {
			hud_show_weapon_energy_gauge();
		}

		// show the auto-target icons
		hud_show_auto_icons();				

		// draw a border around a talking head if it is playing
		hud_maybe_blit_head_border();

		// draw the status of support ship servicing the player
		hud_support_view_blit();

		// draw the damage status
		hud_maybe_show_damage();

		// show mission time 
		if ( hud_gauge_active(HUD_MISSION_TIME) ) {
			hud_show_mission_time();
		}

		// show subspace notify gauge
		hud_maybe_display_subspace_notify();

		// show objective status gauge
		if ( hud_gauge_active(HUD_OBJECTIVES_NOTIFY_GAUGE) ) {
			hud_maybe_display_objective_message();
		}

		if ( hud_gauge_active(HUD_WINGMEN_STATUS) ) {
			hud_wingman_status_render();
		}

		if ( hud_gauge_active(HUD_KILLS_GAUGE) ) {
			show_gauge_flag=1;
			// is gauge configured as a popup?
			if ( hud_gauge_is_popup(HUD_KILLS_GAUGE) ) {
				if ( !hud_gauge_popup_active(HUD_KILLS_GAUGE) ) {
					show_gauge_flag=0;
				}
			}
			
			if ( show_gauge_flag ) {
				hud_show_kills_gauge();
			}
		}

		// show the player shields
		if ( hud_gauge_active(HUD_PLAYER_SHIELD_ICON) ) {
			hud_shield_show(Player_obj);
		}

		// show the directives popup and/or training popup
		message_training_display();

#ifndef NO_NETWORK
		// if this is a multiplayer game, blit any icons/bitmaps indicating voice recording or playback
		if(Game_mode & GM_MULTIPLAYER){
			hud_show_voice_status();
		}
#endif
	}

	hud_show_messages();

#ifndef NO_NETWORK
	// maybe render any necessary multiplayer text messaging strings being entered
	hud_maybe_render_multi_text();
#endif

	// show red alert notify gauge when moving to red alert
	hud_maybe_display_red_alert();	

	// display supernova warning
	hud_maybe_display_supernova();

	// check to see if we are in messaging mode.  If so, send the key to the code
	// to deal with the message.  hud_sqaudmsg_do_frame will return 0 if the key
	// wasn't used in messaging mode, otherwise 1.  In the event the key was used,
	// return immediately out of this function.
	if ( Players->flags & PLAYER_FLAGS_MSG_MODE ) {
		if ( hud_squadmsg_do_frame() ){
			return;
		}
	}

#ifndef NO_NETWORK
	hud_render_multi_ping();	
#endif
}


// hud_stop_looped_engine_sounds()
//
// This function will set the loop id's for the engine noises to -1, this will force any
// looping engine sounds to stop.  This should only be called when the game decides to
// stop all looping sounds
//

void hud_stop_looped_engine_sounds()
{
	if ( Player_engine_snd_loop > -1 )	{
		snd_stop(Player_engine_snd_loop);
		//snd_chg_loop_status(Player_engine_snd_loop, 0);
		Player_engine_snd_loop = -1;
	}
}

#define ZERO_PERCENT			0.01f
#define ENGINE_MAX_VOL		1.0f
#define ENGINE_MAX_PITCH	44100

void update_throttle_sound()
{
	// determine what engine sound to play
	float percent_throttle;
//	int	throttle_pitch;

#ifndef NO_NETWORK
	// if we're a multiplayer observer, stop any engine sounds from playing and return
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
		// stop engine sound if it is playing
		if(Player_engine_snd_loop != -1){
			snd_stop(Player_engine_snd_loop);
			Player_engine_snd_loop = -1;
		}

		// return
		return;
	}
#endif

	if ( timestamp_elapsed(throttle_sound_check_id) ) {

		throttle_sound_check_id = timestamp(THROTTLE_SOUND_CHECK_INTERVAL);
	
		if ( Ships[Player_obj->instance].current_max_speed == 0 ) {
			percent_throttle = Player_obj->phys_info.fspeed / Ship_info[Ships[Player_obj->instance].ship_info_index].max_speed;
		} else {
			percent_throttle = Player_obj->phys_info.fspeed / Ships[Player_obj->instance].current_max_speed;
		}

		// If the throttle has changed, modify the sound
		if ( percent_throttle != last_percent_throttle || Player_engine_snd_loop == -1 ) {

			if ( percent_throttle < ZERO_PERCENT ) {
				if ( Player_engine_snd_loop > -1 )	{
					snd_chg_loop_status(Player_engine_snd_loop, 0);
					Player_engine_snd_loop = -1;
				}
			}
			else {
				if ( Player_engine_snd_loop == -1 ){
					Player_engine_snd_loop = snd_play_looping( &Snds[SND_ENGINE], 0.0f , -1, -1, percent_throttle * ENGINE_MAX_VOL );
				} else {
					// The sound may have been trashed at the low-level if sound channel overflow.
					// TODO: implement system where certain sounds cannot be interrupted (priority?)
					if ( snd_is_playing(Player_engine_snd_loop) ) {
						snd_set_volume(Player_engine_snd_loop, percent_throttle * ENGINE_MAX_VOL);
					}
					else {
						Player_engine_snd_loop = -1;
					}
				}
			}

//			throttle_pitch = snd_get_pitch(Player_engine_snd_loop);
//			if ( percent_throttle > 0.5 ) {
//				snd_set_pitch(Player_engine_snd_loop, fl2i(22050 + (percent_throttle-0.5f)*1000));
//			}

		}	// end if (percent_throttle != last_percent_throttle)

		last_percent_throttle = percent_throttle;

	}	// end if ( timestamp_elapsed(throttle_sound_check_id) )
}

// called at the beginning of each level.  Loads frame data in once, and initializes any damage
// gauge specific data
void hud_damage_popup_init()
{
	int i;

	if ( !Damage_gauges_loaded ) {
		for ( i = 0; i < NUM_DAMAGE_GAUGES; i++ ) {
			Damage_gauges[i].first_frame = bm_load_animation(Damage_gauge_fnames[gr_screen.res][i], &Damage_gauges[i].num_frames);
			if ( Damage_gauges[i].first_frame == -1 ) {
				Warning(LOCATION, "Could not load in the ani: %s\n", Damage_gauge_fnames[gr_screen.res][i]);
				return;
			}
		}
		Damage_gauges_loaded = 1;
	}

	Damage_flash_bright = 0;
	Damage_flash_timer =	1;

	for ( i = 0; i < SUBSYSTEM_MAX; i++ ) {
		Pl_hud_subsys_info[i].last_str = 1000.0f;
		Pl_hud_subsys_info[i].flash_duration_timestamp = 1;
		Pl_hud_next_flash_timestamp = 1;
		Pl_hud_is_bright = 0;
	}
}

// ---------------------------------------------------------
// show player damage status via a popup window

void hud_show_damage_popup()
{
	model_subsystem	*psub;
	ship_subsys			*pss;
	ship_info			*sip;
	int					sx, sy, bx, by, w, h, screen_integrity, num, best_str, best_index;
	float					strength, shield, integrity;
	char					buf[128];
	hud_subsys_damage	hud_subsys_list[SUBSYSTEM_MAX];	

	if ( Damage_gauges[0].first_frame == -1 ) {
		return;
	}

	if ( (The_mission.game_type & MISSION_TYPE_TRAINING) && Training_message_visible ){
		return;
	}
		
	sip = &Ship_info[Player_ship->ship_info_index];
	hud_get_target_strength(Player_obj, &shield, &integrity);
	screen_integrity = fl2i(integrity*100);

	if ( hud_gauge_is_popup(HUD_DAMAGE_GAUGE) ) {
		if ( screen_integrity >= 100 ) {
			return;
		}
	}

	if ( timestamp_elapsed(Damage_flash_timer) ) {
		Damage_flash_timer = timestamp(DAMAGE_FLASH_TIME);
		Damage_flash_bright ^= 1;
	}

	hud_set_gauge_color(HUD_DAMAGE_GAUGE);

	// draw the top of the damage pop-up
	GR_AABITMAP(Damage_gauges[0].first_frame, Damage_gauge_coords[gr_screen.res][0][0], Damage_gauge_coords[gr_screen.res][0][1]);	
	gr_string(Damage_text_coords[gr_screen.res][0], Damage_text_coords[gr_screen.res][1], XSTR( "damage", 218));

	// show hull integrity
	if ( screen_integrity < 100 ) {		
		if ( screen_integrity == 0 ) {
			screen_integrity = 1;
		}
		sprintf(buf, XSTR( "%d%%", 219), screen_integrity);
		hud_num_make_mono(buf);
		gr_get_string_size(&w, &h, buf);
		if ( screen_integrity < 30 ) {
			gr_set_color_fast(&Color_red);
		}
		gr_string(Hull_integ_coords[gr_screen.res][0], Hull_integ_coords[gr_screen.res][1], XSTR( "Hull Integrity", 220));
		gr_string(Hull_integ_val_coords[gr_screen.res][0] - w, Hull_integ_val_coords[gr_screen.res][1], buf);
	} 

	// show damaged subsystems
	sx = Damage_subsys_text_coords[gr_screen.res][0];
	sy = Damage_subsys_text_coords[gr_screen.res][1];
	bx = Damage_gauge_coords[gr_screen.res][1][0];
	by = Damage_gauge_coords[gr_screen.res][1][1];

	num = 0;
	for ( pss = GET_FIRST(&Player_ship->subsys_list); pss !=END_OF_LIST(&Player_ship->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		strength = ship_get_subsystem_strength(Player_ship, psub->type);
		if ( strength < 1 ) {
			screen_integrity = fl2i(strength*100);
			if ( screen_integrity == 0 ) {
				if ( strength > 0 ) {
					screen_integrity = 1;
				}
			}
			hud_subsys_list[num].name = psub->name;
			hud_subsys_list[num].str  = screen_integrity;
			hud_subsys_list[num].type = psub->type;
			num++;

			if ( strength < Pl_hud_subsys_info[psub->type].last_str ) {
				Pl_hud_subsys_info[psub->type].flash_duration_timestamp = timestamp(SUBSYS_DAMAGE_FLASH_DURATION);
			}
			Pl_hud_subsys_info[psub->type].last_str = strength;

			//Don't display more than 12 damaged subsystems.
			if(num >= SUBSYSTEM_MAX)
			{
				break;
			}
		}
	}

	int type;
	for ( int i = 0; i < num; i++ ) {
		best_str = 1000;
		best_index = -1;
		for ( int j = 0; j < num-i; j++ ) {
			if ( hud_subsys_list[j].str < best_str ) {
				best_str = hud_subsys_list[j].str;
				best_index = j;
			}
		}

		Assert(best_index >= 0);
		Assert(best_str >= 0);

		// display strongest subsystem left in list
		// draw the bitmap
		// hud_set_default_color();
		hud_set_gauge_color(HUD_DAMAGE_GAUGE);

		GR_AABITMAP(Damage_gauges[1].first_frame, bx, by);
		by += Damage_gauge_line_h[gr_screen.res];

		type = hud_subsys_list[best_index].type;
		if ( !timestamp_elapsed( Pl_hud_subsys_info[type].flash_duration_timestamp ) ) {
			if ( timestamp_elapsed( Pl_hud_next_flash_timestamp ) ) {
				Pl_hud_is_bright ^= 1;
				Pl_hud_next_flash_timestamp = timestamp(SUBSYS_DAMAGE_FLASH_INTERVAL);
			}
			
			if ( Pl_hud_is_bright ) {
				int alpha_color;
				alpha_color = MIN(HUD_COLOR_ALPHA_MAX,HUD_color_alpha+HUD_BRIGHT_DELTA);
				// gr_set_color_fast(&HUD_color_defaults[alpha_color]);

				hud_set_gauge_color(HUD_DAMAGE_GAUGE, alpha_color);
			} else {				
				hud_set_gauge_color(HUD_DAMAGE_GAUGE);
			}
		}

		// draw the text
		if ( best_str < 30 ) {
			if ( best_str <= 0 ) {
				if ( Damage_flash_bright ) {
					gr_set_color_fast(&Color_bright_red);
				} else {
					gr_set_color_fast(&Color_red);
				}

			} else {
				gr_set_color_fast(&Color_red);
			}
		} else {
			hud_set_gauge_color(HUD_DAMAGE_GAUGE);
		}		

		gr_string(sx, sy, hud_targetbox_truncate_subsys_name(hud_subsys_list[best_index].name));
		sprintf(buf, XSTR( "%d%%", 219), best_str);
		hud_num_make_mono(buf);
		gr_get_string_size(&w, &h, buf);
		gr_string(Hull_integ_val_coords[gr_screen.res][0] - w, sy, buf);
		sy += Damage_gauge_line_h[gr_screen.res];

		// remove it from hud_subsys_list
		if ( best_index < (num-i-1) ) {
			hud_subsys_list[best_index] = hud_subsys_list[num-i-1];
		}
	}

	// draw the bottom of the gauge
	// hud_set_default_color();
	hud_set_gauge_color(HUD_DAMAGE_GAUGE);

	GR_AABITMAP(Damage_gauges[2].first_frame, bx, by);		
}

// init the members of the hud_anim struct to default values
void hud_anim_init(hud_anim *ha, int sx, int sy, char *filename)
{
	ha->first_frame		= -1;
	ha->num_frames		= 0;
	ha->total_time		= 0.0f;
	ha->time_elapsed	= 0.0f;
	ha->sx				= sx;
	ha->sy				= sy;
	strcpy(ha->name, filename);
}

// init the members of the hud_frames struct to default values
void hud_frames_init(hud_frames *hf)
{
	hf->first_frame		= -1;
	hf->num_frames		= 0;
}

// call to unload the targetbox static animation
void hud_anim_release(hud_anim *ha)
{
	int i;
	for ( i = 0; i < ha->num_frames; i++ ) {
		bm_unload(ha->first_frame + i);
	}
}

// load a hud_anim
// return 0 is successful, otherwise return -1
int hud_anim_load(hud_anim *ha)
{
	int		fps;

	ha->first_frame = bm_load_animation(ha->name, &ha->num_frames, &fps);
	if ( ha->first_frame == -1 ) {
		Int3();	// couldn't load animation file in
		return -1;
	}
	Assert(fps != 0);
	ha->total_time = i2fl(ha->num_frames)/fps;
	return 0;
}

// render out a frame of the targetbox static animation, based on how much time has
// elapsed
// input:	ha				=>	pointer to hud anim info
//				frametime	=>	seconds elapsed since last frame
//				draw_alpha	=>	draw bitmap as alpha-bitmap (default 0)
//				loop			=>	anim should loop (default 1)
//				hold_last	=>	should last frame be held (default 0)
//				reverse		=>	play animation in reverse (default 0)
int hud_anim_render(hud_anim *ha, float frametime, int draw_alpha, int loop, int hold_last, int reverse,bool resize)
{
	int framenum;

	if ( ha->num_frames <= 0 ) {
		if ( hud_anim_load(ha) == -1 )
			return 0;
	}

	ha->time_elapsed += frametime;
	if ( ha->time_elapsed > ha->total_time ) {
		if ( loop ) {
			ha->time_elapsed = 0.0f;
		} else {
			if ( !hold_last ) {
				return 0;
			}
		}
	}

	// draw the correct frame of animation
	framenum = fl2i( (ha->time_elapsed * ha->num_frames) / ha->total_time );
	if (reverse) {
		framenum = (ha->num_frames-1) - framenum;
	}

	if ( framenum < 0 )
		framenum = 0;
	if ( framenum >= ha->num_frames )
		framenum = ha->num_frames-1;

	// Blit the bitmap for this frame
	if(emp_should_blit_gauge()){
		gr_set_bitmap(ha->first_frame + framenum);
		if ( draw_alpha ){
			gr_aabitmap(ha->sx, ha->sy,resize);
		} else {
			gr_bitmap(ha->sx, ha->sy,resize);
		}
	}

	return 1;
}

// convert a number string to use mono-spaced 1 character
void hud_num_make_mono(char *num_str)
{
	int len, i, sc;
	len = strlen(num_str);

	sc = Lcl_special_chars;
	for ( i = 0; i < len; i++ ) {
		if ( num_str[i] == '1' ) {
			num_str[i] = (char)(sc + 1);
		}
	}
}

// flashing text gauge
void hud_init_text_flash_gauge()
{	
}

void hud_start_text_flash(char *txt, int t)
{
	// bogus
	if(txt == NULL){
		strcpy(Hud_text_flash, "");
		return;
	}

	// HACK. don't override EMP if its still going    :)
	if(!stricmp(Hud_text_flash, NOX("Emp")) && !hud_targetbox_flash_expired(TBOX_FLASH_CMEASURE)){
		return;
	}

	strncpy(Hud_text_flash, txt, 500);
	hud_targetbox_start_flash(TBOX_FLASH_CMEASURE, t);	
}

void hud_maybe_show_text_flash_icon()
{		
	int bright;

	if ( hud_targetbox_flash_expired(TBOX_FLASH_CMEASURE) ) {
		return;
	}

	hud_targetbox_maybe_flash(TBOX_FLASH_CMEASURE);		

	// bright?
	bright = hud_targetbox_is_bright(TBOX_FLASH_CMEASURE);

	// draw
	hud_show_text_flash_icon(Hud_text_flash, Hud_text_flash_coords[gr_screen.res][1], bright);
}

void hud_show_text_flash_icon(char *txt, int y, int bright)
{
	int w, h;

	// different font size in hi-res
	if(gr_screen.res != GR_640){
		gr_set_font(FONT3);
	}

	// set color
	if(bright){
		hud_set_gauge_color(HUD_TEXT_FLASH, HUD_C_DIM);
	} else {
		gr_set_color_fast(&Color_black);
	}

	// string size
	gr_get_string_size(&w, &h, txt);

	// draw the box	
	gr_rect( (int)((((float)gr_screen.max_w_unscaled / 2.0f) - ((float)w / 2.0f)) - 1.0f), (int)((float)y - 1.0f), w + 2, h + 1);

	// string
	hud_set_gauge_color(HUD_TEXT_FLASH, HUD_C_BRIGHT);	
	gr_string(fl2i((gr_screen.max_w_unscaled - w) / 2.0f), y, txt);

	// go back to normal font
	gr_set_font(FONT1);
}

// maybe display the kills gauge on the HUD
void hud_show_kills_gauge()
{
	if ( Kills_gauge.first_frame < 0 ) {
		return;
	}

	// hud_set_default_color();
	hud_set_gauge_color(HUD_KILLS_GAUGE);

	// draw background
	GR_AABITMAP(Kills_gauge.first_frame, Kills_gauge_coords[gr_screen.res][0], Kills_gauge_coords[gr_screen.res][1]);	

	gr_string(Kills_text_coords[gr_screen.res][0], Kills_text_coords[gr_screen.res][1], XSTR( "kills:", 223));

	// display how many kills the player has so far
	char	num_kills_string[32];
	int	w,h;

	if ( !Player ) {
		Int3();
		return;
	}

	sprintf(num_kills_string, "%d", Player->stats.m_kill_count_ok);

	gr_get_string_size(&w, &h, num_kills_string);
	if (Lcl_gr) {
		gr_string(Kills_text_val_coords_gr[gr_screen.res][0]-w, Kills_text_val_coords_gr[gr_screen.res][1], num_kills_string);
	} else {
		gr_string(Kills_text_val_coords[gr_screen.res][0]-w, Kills_text_val_coords[gr_screen.res][1], num_kills_string);
	}
}

#ifndef NO_NETWORK
// maybe show the netlag icon on the hud
void hud_maybe_show_netlag_icon()
{
	int lag_status;

	if ( Netlag_icon.first_frame == -1 ) {
		Int3();
		return;
	}

	lag_status = multi_query_lag_status();	

	switch(lag_status) {
	case 0:
		// draw the net lag icon flashing
		hud_targetbox_start_flash(TBOX_FLASH_NETLAG);
		if(hud_targetbox_maybe_flash(TBOX_FLASH_NETLAG)){
			hud_set_gauge_color(HUD_LAG_GAUGE, HUD_C_BRIGHT);
		} else {
			hud_set_gauge_color(HUD_LAG_GAUGE);
		}
		gr_set_bitmap(Netlag_icon.first_frame);
		break;
	case 1:
		// draw the disconnected icon flashing fast
		if(hud_targetbox_maybe_flash(TBOX_FLASH_NETLAG,1)){
			hud_set_gauge_color(HUD_LAG_GAUGE, HUD_C_BRIGHT);
		} else {
			hud_set_gauge_color(HUD_LAG_GAUGE);
		}
		gr_set_bitmap(Netlag_icon.first_frame+1);
		break;
	default:
		// nothing to draw
		return;
	}
	
	if(emp_should_blit_gauge()){
		gr_aabitmap(Netlag_coords[gr_screen.res][0], Netlag_coords[gr_screen.res][1]);
	}
}
#endif  // ifndef NO_NETWORK

// load in kills gauge if required
void hud_init_kills_gauge()
{
	if ( !Kills_gauge_loaded ) {
		Kills_gauge.first_frame = bm_load_animation(Kills_fname[gr_screen.res], &Kills_gauge.num_frames);
		if ( Kills_gauge.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in the kills ani: Kills_fname[gr_screen.res]\n");
			return;
		}
		Kills_gauge_loaded = 1;
	}
}

// load in netlag icon if required
void hud_init_netlag_icon()
{
	if ( !Netlag_icon_loaded ) {
		Netlag_icon.first_frame = bm_load_animation(Netlag_fname[gr_screen.res], &Netlag_icon.num_frames);
		if ( Netlag_icon.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in the netlag ani: Netlag_fname[gr_screen.res]\n");
			return;
		}
		Netlag_icon_loaded = 1;
	}
}

// called at mission start to init data, and load support view bitmap if required
void hud_support_view_init()
{
	Hud_support_view_fade = 1;
	Hud_support_obj_sig = -1;
	Hud_support_target_sig = -1;
	Hud_support_objnum = -1;
	Hud_support_view_active = 0;
	Hud_support_view_abort = 0;

	// ensure the talking head border is loaded
	if ( !Support_view_gauge_loaded ) {
		Support_view_gauge.first_frame = bm_load_animation(Support_fname[gr_screen.res], &Support_view_gauge.num_frames);
		if ( Support_view_gauge.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in ani: Support_fname[gr_screen.res]\n");
		}
		Support_view_gauge_loaded = 1;
	}
}

// start displaying the support view pop-up.  This will remain up until hud_support_view_stop is called.
// input:	objnum	=>		object number for the support ship
void hud_support_view_start()
{
	Hud_support_view_active = 1;
	Hud_support_view_fade = 1;
}

// stop displaying the support view pop-up
void hud_support_view_stop(int stop_now)
{
	if ( stop_now ) {
		Hud_support_view_active = 0;
		Hud_support_view_fade = 1;
		Hud_support_view_abort = 0;
	} else {
		Hud_support_view_fade = timestamp(2000);
	}

	Hud_support_obj_sig = -1;
	Hud_support_target_sig = -1;
	Hud_support_objnum = -1;
}

void hud_support_view_abort()
{
	hud_support_view_stop(0);
	Hud_support_view_abort = 1;
}

// return the number of seconds until repair ship will dock with player, return -1 if error
// 
// mwa made this function more general purpose
// Goober5000 made clearer
//
// NOTE: This function is pretty stupid now.  It just assumes the dockee is sitting still, and
//		   the docker is moving directly to the dockee.
int hud_get_dock_time( object *docker_objp )
{
	ai_info	*aip;
	object	*dockee_objp;
	float		dist, rel_speed, docker_speed;
	vec3d	rel_vel;

	aip = &Ai_info[Ships[docker_objp->instance].ai_index];

	// get the dockee object pointer
	if (aip->goal_objnum == -1) {
		// this can happen when you target a support ship as it warps in
		// just give a debug warning instead of a fault - taylor
	//	Int3();	//	Shouldn't happen, but let's recover gracefully.
		mprintf(("'aip->goal_objnum == -1' in hud_get_dock_time(), line %i\n", __LINE__));
		return 0;
	}

	dockee_objp = &Objects[aip->goal_objnum];

	// if the ship is docked, return 0
	if ( dock_check_find_direct_docked_object(docker_objp, dockee_objp) )
		return 0;

	vm_vec_sub(&rel_vel, &docker_objp->phys_info.vel, &dockee_objp->phys_info.vel);
	rel_speed = vm_vec_mag_quick(&rel_vel);

	dist = vm_vec_dist_quick(&dockee_objp->pos, &docker_objp->pos);

	docker_speed = docker_objp->phys_info.speed;

	if ( rel_speed <= docker_speed/2.0f) {	//	This means the player is moving away fast from the support ship.
		return (int) (dist/docker_speed);
	} else {
		float	d1;
		float	d = dist;
		float	time = 0.0f;
		
		if (rel_speed < 20.0f)
			rel_speed = 20.0f;

		//	When faraway, use max speed, not current speed.  Might not have sped up yet.
		if (d > 100.0f) {
			time += (d - 100.0f)/docker_objp->phys_info.max_vel.xyz.z;
		}

		//	For mid-range, use current speed.
		if (d > 60.0f) {
			d1 = MIN(d, 100.0f);

			time += (d1 - 60.0f)/rel_speed;
		}

		//	For nearby, ship will have to slow down a bit for docking maneuver.
		if (d > 30.0f) {
			d1 = MIN(d, 60.0f);

			time += (d1 - 30.0f)/5.0f;
		}

		//	For very nearby, ship moves quite slowly.
		d1 = MIN(d, 30.0f);
		time += d1/7.5f;

		return fl2i(time);
	}
}

// Locate the closest support ship which is trying to dock with player, return -1 if there is no support
// ship currently trying to dock with the player
// MA:  4/22/98 -- pass in objp to find support ship trying to dock with objp
int hud_support_find_closest( int objnum )
{
	ship_obj		*sop;
	ai_info		*aip;
	object		*objp;
	int i;

	objp = &Objects[objnum];

	sop = GET_FIRST(&Ship_obj_list);
	while(sop != END_OF_LIST(&Ship_obj_list)){
		if ( Ship_info[Ships[Objects[sop->objnum].instance].ship_info_index].flags & SIF_SUPPORT ) {
			int pship_index, sindex;

			// make sure support ship is not dying
			if ( !(Ships[Objects[sop->objnum].instance].flags & (SF_DYING|SF_EXPLODED)) ) {

				Assert( objp->type == OBJ_SHIP );
				aip = &Ai_info[Ships[Objects[sop->objnum].instance].ai_index];
				pship_index = objp->instance;

				// we must check all goals for this support ship -- not just the first one
				for ( i = 0; i < MAX_AI_GOALS; i++ ) {

					// we can use == in the next statement (and should) since a ship will only ever be
					// following one order at a time.
					if ( aip->goals[i].ai_mode == AI_GOAL_REARM_REPAIR ) {
						Assert( aip->goals[i].ship_name );
						sindex = ship_name_lookup( aip->goals[i].ship_name );
						if ( sindex == pship_index )
							return sop->objnum;
					}
				}
			}
		}
		sop = GET_NEXT(sop);
	}

	return -1;
}

// dipaly the hud_support view popup
void hud_support_view_blit()
{
	int	show_time;
	char	outstr[64];
	
	if ( !Hud_support_view_active ) {
		return;
	}

#ifndef NO_NETWORK
	// don't render this gauge for multiplayer observers
	if((Game_mode & GM_MULTIPLAYER) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER))){
		return;
	}
#endif

	// If we haven't determined yet who the rearm ship is, try to!
	if (Hud_support_objnum == -1) {
		Hud_support_objnum = hud_support_find_closest( OBJ_INDEX(Player_obj) );
		if ( Hud_support_objnum >= 0 ) {
			Hud_support_obj_sig = Objects[Hud_support_objnum].signature;
			Hud_support_target_sig = Player_obj->signature;
		}
	} else {
		// check to see if support ship is still alive
		if ( (Objects[Hud_support_objnum].signature != Hud_support_obj_sig) || (Hud_support_target_sig != Player_obj->signature) ) {
			hud_support_view_stop(1);
			return;
		}
	}

	// set hud color
	hud_set_gauge_color(HUD_SUPPORT_GAUGE);

	GR_AABITMAP(Support_view_gauge.first_frame, Support_view_coords[gr_screen.res][0], Support_view_coords[gr_screen.res][1]);	

	gr_string(Support_text_coords[gr_screen.res][0], Support_text_coords[gr_screen.res][1], XSTR( "support", 224));

	if ( Hud_support_view_fade > 1 ) {
		if ( !timestamp_elapsed(Hud_support_view_fade) ) {
			if ( Hud_support_view_abort){
				gr_string(0x8000, Support_text_val_coords[gr_screen.res][1], XSTR( "aborted", 225));
			} else {
				gr_string(0x8000, Support_text_val_coords[gr_screen.res][1], XSTR( "complete", 1407));
			}
			return;
		} else {
			Hud_support_view_abort = 0;
			Hud_support_view_active = 0;
			Hud_support_view_fade = 1;
			Hud_support_objnum = -1;
			return;
		}
	}

	show_time = 0;
	if ( Player_ai->ai_flags & AIF_BEING_REPAIRED ) {
		Assert(Player_ship->ship_max_hull_strength > 0);
		if (  (ship_get_subsystem_strength(Player_ship, SUBSYSTEM_ENGINE) < 1.0 ) ||
				(ship_get_subsystem_strength(Player_ship, SUBSYSTEM_SENSORS) < 1.0 ) ||
				(ship_get_subsystem_strength(Player_ship, SUBSYSTEM_WEAPONS) < 1.0 ) ||
				(ship_get_subsystem_strength(Player_ship, SUBSYSTEM_COMMUNICATION) < 1.0 ) ) {
			sprintf(outstr, XSTR( "repairing", 227));
		} else {
			sprintf(outstr, XSTR( "rearming", 228));
		}
		gr_string(0x8000, Support_text_val_coords[gr_screen.res][1], outstr);
	} else if (Player_ai->ai_flags & AIF_REPAIR_OBSTRUCTED) {
		sprintf(outstr, XSTR( "obstructed", 229));
		gr_string(0x8000, Support_text_val_coords[gr_screen.res][1], outstr);
	} else {
		if ( Hud_support_objnum == -1 ) {
			if (The_mission.support_ships.arrival_location == ARRIVE_FROM_DOCK_BAY)
			{
				sprintf(outstr, XSTR( "exiting hangar", -1));
			}
			else
			{
				sprintf(outstr, XSTR( "warping in", 230));
			}
			gr_string(0x8000, Support_text_val_coords[gr_screen.res][1], outstr);
		} else {
			ai_info *aip;

			// display "busy" when support ship isn't actually enroute to me
			aip = &Ai_info[Ships[Objects[Hud_support_objnum].instance].ai_index];
			if ( aip->goal_objnum != OBJ_INDEX(Player_obj) ) {
				sprintf(outstr, XSTR( "busy", 231));
				show_time = 0;

			} else {
				sprintf(outstr, XSTR( "dock in:", 232));
				show_time = 1;
			}		

			if (!show_time) {
				gr_string(Support_text_dock_coords[gr_screen.res][0], Support_text_val_coords[gr_screen.res][1], outstr);
			} else {			
				gr_string(Support_text_dock_coords[gr_screen.res][0], Support_text_val_coords[gr_screen.res][1], outstr);
			}
		}
	}

	if ( show_time ) {
		int seconds, minutes;

		Assert( Hud_support_objnum != -1 );

		// ensure support ship is still alive
		if ( (Objects[Hud_support_objnum].signature != Hud_support_obj_sig) || (Hud_support_target_sig != Player_obj->signature) ) {
			hud_support_view_stop(1);
			seconds = 0;
		} else {
			seconds = hud_get_dock_time( &Objects[Hud_support_objnum] );
		}

		if ( seconds >= 0 ) {
			minutes = seconds/60;
			seconds = seconds%60;
			if ( minutes > 99 ) {
				minutes = 99;
				seconds = 99;
			}
		} else {
			minutes = 99;
			seconds = 99;
		}
		gr_printf(Support_text_dock_val_coords[gr_screen.res][0], Support_text_val_coords[gr_screen.res][1], NOX("%02d:%02d"), minutes, seconds);
	}
}

// Set the current color to the default HUD color (with default alpha)
void hud_set_default_color()
{
	Assert(HUD_color_alpha >= 0 && HUD_color_alpha < HUD_NUM_COLOR_LEVELS);
	gr_set_color_fast(&HUD_color_defaults[HUD_color_alpha]);
}

// Set the current color to a bright HUD color (ie high alpha)
void hud_set_bright_color()
{
	int alpha_color;
	alpha_color = MIN(HUD_COLOR_ALPHA_MAX,HUD_color_alpha+HUD_BRIGHT_DELTA);
	gr_set_color_fast(&HUD_color_defaults[alpha_color]);
}

// Set the current color to a dim HUD color (ie low alpha)
void hud_set_dim_color()
{
	if ( HUD_color_alpha > 2 ) {
		gr_set_color_fast(&HUD_color_defaults[2]);
	}
}

// hud_set_iff_color() will set the color to the IFF color based on the team
//
// input:	team			=>		team to base color on
//				is_bright	=>		default parameter (value 0) which uses bright version of IFF color
void hud_set_iff_color(object *objp, int is_bright)
{
	// AL 12-26-97:	it seems IFF color needs to be set relative to the player team.  If
	//						the team in question is the same as the player, then it should be 
	//						drawn friendly.  If the team is different than the players, then draw the
	//						appropriate IFF.          
	int team;
	team = obj_team(objp);

	if ( ship_is_tagged(objp) ) {
		gr_set_color_fast(&IFF_colors[IFF_COLOR_TAGGED][is_bright]);
	} else if ( (team == Player_ship->team) && (Player_ship->team != TEAM_TRAITOR) ) {
		gr_set_color_fast(&IFF_colors[IFF_COLOR_FRIENDLY][is_bright]);
	} else {
		switch (team) {
		case TEAM_NEUTRAL:
			gr_set_color_fast(&IFF_colors[IFF_COLOR_NEUTRAL][is_bright]);
			break;
		case TEAM_UNKNOWN:
			gr_set_color_fast(&IFF_colors[IFF_COLOR_UNKNOWN][is_bright]);
			break;
		case TEAM_HOSTILE:
		case TEAM_FRIENDLY:
		case TEAM_TRAITOR:
			gr_set_color_fast(&IFF_colors[IFF_COLOR_HOSTILE][is_bright]);
			break;
		default:
			Int3();
			gr_set_color_fast(&IFF_colors[IFF_COLOR_UNKNOWN][is_bright]);
			break;
		}
	}
}

// Determine if ship team should be ignored, based on
// team filter
// input:	team_filter	=>	team mask used to select friendly or hostile ships
//				ship_team	=>	team of the ship in question
// exit:		1				=>	ship_team matches filter from player perspective
//				0				=>	ship_team does match team filter
int hud_team_matches_filter(int team_filter, int ship_team)
{
	return team_filter & ship_team;
}


// reset gauge flashing data
void hud_gauge_flash_init()
{
	int i;
	for ( i=0; i<NUM_HUD_GAUGES; i++ ) {
		HUD_gauge_flash_duration[i]=timestamp(0);
		HUD_gauge_flash_next[i]=timestamp(0);
	}
	HUD_gauge_bright=0;
}

#define NUM_VM_OTHER_SHIP_GAUGES 5
static int Vm_other_ship_gauges[NUM_VM_OTHER_SHIP_GAUGES] = 
{
	HUD_CENTER_RETICLE,
	HUD_TARGET_MONITOR,
	HUD_TARGET_MONITOR_EXTRA_DATA,
	HUD_MESSAGE_LINES,
	HUD_TALKING_HEAD
};

// determine if the specified HUD gauge should be displayed
int hud_gauge_active(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);

	// AL: Special code: Only show two gauges when not viewing from own ship
	if ( Viewer_mode & VM_OTHER_SHIP ) {
		for ( int i = 0; i < NUM_VM_OTHER_SHIP_GAUGES; i++ ) {
			if ( gauge_index == Vm_other_ship_gauges[i] ) {
				return 1;
			}
		}
		return 0;
	}

	return hud_config_show_flag_is_set(gauge_index);
}

// determine if gauge is in pop-up mode or not
int hud_gauge_is_popup(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	return hud_config_popup_flag_is_set(gauge_index);
}

// determine if a popup gauge should be drawn
int hud_gauge_popup_active(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	if ( !hud_gauge_is_popup(gauge_index) ) {
		return 0;
	}

	if ( !timestamp_elapsed(HUD_popup_timers[gauge_index]) ) {
		return 1;
	} else {
		return 0;
	}
}

// start a gauge to popup
void hud_gauge_popup_start(int gauge_index, int time) 
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	if ( !hud_gauge_is_popup(gauge_index) ) {
		return;
	}

	HUD_popup_timers[gauge_index] = timestamp(time);

}

// call HUD function to flash gauge
void hud_gauge_start_flash(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	HUD_gauge_flash_duration[gauge_index] = timestamp(HUD_GAUGE_FLASH_DURATION);
	HUD_gauge_flash_next[gauge_index] = 1;
}

// Set the HUD color for the gauge, based on whether it is flashing or not
void hud_set_gauge_color(int gauge_index, int bright_index)
{
//	color use_color;
	int flash_status = hud_gauge_maybe_flash(gauge_index);
	color *use_color = &HUD_config.clr[gauge_index];
	int alpha;

	// if we're drawing it as bright
	if(bright_index != HUD_C_NONE){
		switch(bright_index){
		case HUD_C_DIM:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_DIM_HI : HUD_NEW_ALPHA_DIM;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;

		case HUD_C_NORMAL:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_NORMAL_HI : HUD_NEW_ALPHA_NORMAL;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;

		case HUD_C_BRIGHT:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_BRIGHT_HI : HUD_NEW_ALPHA_BRIGHT;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;

		// intensity
		default: 
			Assert((bright_index >= 0) && (bright_index < HUD_NUM_COLOR_LEVELS));
			if(bright_index < 0){
				bright_index = 0;
			}
			if(bright_index >= HUD_NUM_COLOR_LEVELS){
				bright_index = HUD_NUM_COLOR_LEVELS - 1;
			}

			// alpha = 255 - (255 / (bright_index + 1));
			// alpha = (int)((float)alpha * 1.5f);
			int level = 255 / (HUD_NUM_COLOR_LEVELS);
			alpha = level * bright_index;
			if(alpha > 255){
				alpha = 255;
			}
			if(alpha < 0){
				alpha = 0;
			}
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		}
	} else {
		switch(flash_status) {
		case 0:
			alpha = HUD_contrast ? HUD_NEW_ALPHA_DIM_HI : HUD_NEW_ALPHA_DIM;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		case 1:			
			alpha = HUD_contrast ? HUD_NEW_ALPHA_BRIGHT_HI : HUD_NEW_ALPHA_BRIGHT;
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		default:			
			alpha = HUD_contrast ? HUD_NEW_ALPHA_NORMAL_HI : HUD_NEW_ALPHA_NORMAL;	
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			break;
		}
	}

	gr_set_color_fast(use_color);	
}

// set the color for a gauge that may be flashing
// exit:	-1	=>	gauge is not flashing
//			0	=>	gauge is flashing, draw dim
//			1	=>	gauge is flashing, draw bright
int hud_gauge_maybe_flash(int gauge_index)
{
	Assert(gauge_index >=0 && gauge_index < NUM_HUD_GAUGES);
	int flash_status=-1;
	if ( !timestamp_elapsed(HUD_gauge_flash_duration[gauge_index]) ) {
		if ( timestamp_elapsed(HUD_gauge_flash_next[gauge_index]) ) {
			HUD_gauge_flash_next[gauge_index] = timestamp(HUD_GAUGE_FLASH_INTERVAL);
			HUD_gauge_bright ^= (1<<gauge_index);	// toggle between default and bright frames
		}

		if ( HUD_gauge_bright & (1<<gauge_index) ) {
			flash_status=1;
		} else {
			flash_status=0;
		}
	}
	return flash_status;
}

// Init the objective message display data
void hud_objective_message_init()
{
	// ensure the talking head border is loaded
	if ( !Objective_display_gauge_inited ) {
		Objective_display_gauge.first_frame = bm_load_animation(Objective_fname[gr_screen.res], &Objective_display_gauge.num_frames);
		if ( Objective_display_gauge.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in ani: Objective_fname[gr_screen.res]\n");
		}
		Objective_display_gauge_inited = 1;
	}

	Objective_display.display_timer=timestamp(0);
}

// Display objective status on the HUD
// input:	type			=>	type of goal, one of:	PRIMARY_GOAL
//																	SECONDARY_GOAL
//																	BONUS_GOAL
//
//				status		=> status of goal, one of:	GOAL_FAILED
//																	GOAL_COMPLETE
//																	GOAL_INCOMPLETE
//
void hud_add_objective_messsage(int type, int status)
{
	Objective_display.display_timer=timestamp(7000);
	Objective_display.goal_type=type;
	Objective_display.goal_status=status;

#ifndef NO_NETWORK
	// if this is a multiplayer tvt game
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM) && (Net_player != NULL)){
		mission_goal_fetch_num_resolved(type, &Objective_display.goal_nresolved, &Objective_display.goal_ntotal, Net_player->p_info.team);
	}
	else
#endif
	{
		mission_goal_fetch_num_resolved(type, &Objective_display.goal_nresolved, &Objective_display.goal_ntotal);
	}

	// TODO: play a sound?
}

// maybe display the 'subspace drive engaged' message
void hud_maybe_display_subspace_notify()
{
	int warp_aborted = 0;
	// maybe make gauge active
	if ( (Player->control_mode == PCM_WARPOUT_STAGE1) || (Player->control_mode == PCM_WARPOUT_STAGE2) || (Player->control_mode == PCM_WARPOUT_STAGE3) ) {
		if (!hud_subspace_notify_active()) {
			// keep sound from being played 1e06 times
			hud_start_subspace_notify();
		}
	} else {
		if ( !timestamp_elapsed(HUD_abort_subspace_timer) ) {
			warp_aborted = 1;
		} else {
			hud_stop_subspace_notify();
		}
	}

	if ( !hud_subspace_notify_active() ) {
		return;
	}

	if ( Objective_display_gauge.first_frame < 0 ) {
		return;
	}

	// blit the background	
	hud_set_gauge_color(HUD_OBJECTIVES_NOTIFY_GAUGE);
	GR_AABITMAP(Objective_display_gauge.first_frame, Objective_display_coords[gr_screen.res][0], Objective_display_coords[gr_screen.res][1]);	

	hud_targetbox_start_flash(TBOX_FLASH_OBJECTIVE);
	if(hud_targetbox_maybe_flash(TBOX_FLASH_OBJECTIVE)){
		hud_set_gauge_color(HUD_OBJECTIVES_NOTIFY_GAUGE, HUD_C_BRIGHT);
	} else {
		hud_set_gauge_color(HUD_OBJECTIVES_NOTIFY_GAUGE);
	}


	gr_string(0x8000, Subspace_text_coords[gr_screen.res][1],XSTR( "subspace drive", 233));
	if ( warp_aborted ) {
		gr_string(0x8000, Subspace_text_val_coords[gr_screen.res][1],XSTR( "aborted", 225));
	} else {
		gr_string(0x8000, Subspace_text_val_coords[gr_screen.res][1],XSTR( "engaged", 234));
	}
}

// maybe display the 'Downloading new orders' message
void hud_maybe_display_red_alert()
{
	if ( !red_alert_check_status() ) {
		return;
	}

	if ( Objective_display_gauge.first_frame < 0 ) {
		return;
	}

	if ( hud_subspace_notify_active() ) {
		return;
	}

	if ( hud_objective_notify_active() ) {
		return;
	}

	// blit the background
	gr_set_color_fast(&Color_red);		// color box red, cuz its an emergency for cryin out loud

	GR_AABITMAP(Objective_display_gauge.first_frame, Objective_display_coords[gr_screen.res][0], Objective_display_coords[gr_screen.res][1]);	

	hud_targetbox_start_flash(TBOX_FLASH_OBJECTIVE);
	if(hud_targetbox_maybe_flash(TBOX_FLASH_OBJECTIVE)) {
		gr_set_color_fast(&Color_red);
	} else {
		gr_set_color_fast(&Color_bright_red);
	}

	gr_string(0x8000, Red_text_coords[gr_screen.res][1], XSTR( "downloading new", 235));
	gr_string(0x8000, Red_text_val_coords[gr_screen.res][1], XSTR( "orders...", 236));

	// TODO: play a sound?
}

// Maybe show an objective status update on the HUD
void hud_maybe_display_objective_message()
{
	char buf[128];

	if ( timestamp_elapsed(Objective_display.display_timer) ) {
		hud_stop_objective_notify();
		return;
	}

	if ( Objective_display_gauge.first_frame < 0 ) {
		return;
	}

	if ( hud_subspace_notify_active() ) {
		return;
	}

	if (!hud_objective_notify_active()) {
		hud_start_objective_notify();
	}
	
	// blit the background
	hud_set_gauge_color(HUD_OBJECTIVES_NOTIFY_GAUGE);
	GR_AABITMAP(Objective_display_gauge.first_frame, Objective_display_coords[gr_screen.res][0], Objective_display_coords[gr_screen.res][1]);	

	hud_targetbox_start_flash(TBOX_FLASH_OBJECTIVE);
	if(hud_targetbox_maybe_flash(TBOX_FLASH_OBJECTIVE)){
		hud_set_gauge_color(HUD_OBJECTIVES_NOTIFY_GAUGE, HUD_C_BRIGHT);
	} else {
		hud_set_gauge_color(HUD_OBJECTIVES_NOTIFY_GAUGE);
	}

	// draw the correct goal type
	switch(Objective_display.goal_type) {
	case PRIMARY_GOAL:
		gr_string(0x8000, Objective_text_coords[gr_screen.res][1],XSTR( "primary objective", 237));
		break;
	case SECONDARY_GOAL:
		gr_string(0x8000, Objective_text_coords[gr_screen.res][1],XSTR( "secondary objective", 238));
		break;
	case BONUS_GOAL:
		gr_string(0x8000, Objective_text_coords[gr_screen.res][1],XSTR( "bonus objective", 239));
		break;
	}

	// show the status
	switch(Objective_display.goal_type) {
	case PRIMARY_GOAL:
	case SECONDARY_GOAL:
		switch(Objective_display.goal_status) {
		case GOAL_FAILED:
			sprintf(buf, XSTR( "failed (%d/%d)", 240), Objective_display.goal_nresolved, Objective_display.goal_ntotal);
			gr_string(0x8000, Objective_text_val_coords[gr_screen.res][1], buf);
			break;
		default:
			sprintf(buf, XSTR( "complete (%d/%d)", 241), Objective_display.goal_nresolved, Objective_display.goal_ntotal);
			gr_string(0x8000, Objective_text_val_coords[gr_screen.res][1], buf);
			break;
		}		
		break;
	case BONUS_GOAL:
		switch(Objective_display.goal_status) {
		case GOAL_FAILED:
			gr_string(0x8000, Objective_text_val_coords[gr_screen.res][1], XSTR( "failed", 242));
			break;
		default:
			gr_string(0x8000, Objective_text_val_coords[gr_screen.res][1], XSTR( "complete", 226));
			break;
		}		
		break;
	}
}

#ifndef NO_NETWORK
void hud_show_voice_status()
{
	char play_callsign[CALLSIGN_LEN+5];
	
	// if we are currently playing a rtvoice sound stream from another player back
	memset(play_callsign,0,CALLSIGN_LEN+5);
	switch(multi_voice_status()){
	// the player has been denied the voice token
	case MULTI_VOICE_STATUS_DENIED:
		// show a red indicator or something
		gr_string(Voice_coords[gr_screen.res][0], Voice_coords[gr_screen.res][1], XSTR( "[voice denied]", 243));
		break;

	// the player is currently recording
	case MULTI_VOICE_STATUS_RECORDING:
		gr_string(Voice_coords[gr_screen.res][0], Voice_coords[gr_screen.res][1], XSTR( "[recording voice]", 244));
		break;
		
	// the player is current playing back voice from someone
	case MULTI_VOICE_STATUS_PLAYING:
		gr_string(Voice_coords[gr_screen.res][0], Voice_coords[gr_screen.res][1], XSTR( "[playing voice]", 245));
		break;

	// nothing voice related is happening on my machine
	case MULTI_VOICE_STATUS_IDLE:
		// probably shouldn't be displaying anything
		break;
	}	
}
#endif  // ifndef NO_NETWORK

void hud_subspace_notify_abort()
{
	HUD_abort_subspace_timer = timestamp(1500);
}

void hud_stop_subspace_notify()
{
	Subspace_notify_active=0;
}

void hud_start_subspace_notify()
{

	Subspace_notify_active=1;
}

int hud_subspace_notify_active()
{
	return Subspace_notify_active;
}

void hud_stop_objective_notify()
{
	Objective_notify_active = 0;
}

void hud_start_objective_notify()
{
	snd_play(&(Snds[SND_DIRECTIVE_COMPLETE]));
	Objective_notify_active = 1;
}

int hud_objective_notify_active()
{
	return Objective_notify_active;
}

#ifndef NO_NETWORK
// render multiplayer text message currently being entered if any
void hud_maybe_render_multi_text()
{
	char txt[MULTI_MSG_MAX_TEXT_LEN+20];

	// clear the text
	memset(txt,0,MULTI_MSG_MAX_TEXT_LEN+1);

	// if there is valid multiplayer message text to be displayed
	if(multi_msg_message_text(txt)){
		gr_set_color_fast(&Color_normal);
		gr_string(Multi_msg_coords[gr_screen.res][0], Multi_msg_coords[gr_screen.res][1], txt);
	}
}
#endif

// set the offset values for this render frame
void HUD_set_offsets(object *viewer_obj, int wiggedy_wack)
{
	if ( (viewer_obj == Player_obj) && wiggedy_wack ){		
		vec3d tmp;
		vertex pt;
		ubyte flags;		

		HUD_offset_x = 0.0f;
		HUD_offset_y = 0.0f;

		vm_vec_scale_add( &tmp, &Eye_position, &Viewer_obj->orient.vec.fvec, 100.0f );
		
		flags = g3_rotate_vertex(&pt,&tmp);

		if (flags == 0) {

			g3_project_vertex(&pt);

			if (!(pt.flags & PF_OVERFLOW))	{
				gr_unsize_screen_posf( &pt.sx, &pt.sy );
				HUD_offset_x -= 0.45f * (i2fl(gr_screen.clip_width_unscaled)*0.5f - pt.sx);
				HUD_offset_y -= 0.45f * (i2fl(gr_screen.clip_height_unscaled)*0.5f - pt.sy);
			}
		}

		if ( HUD_offset_x > 100.0f )	{
			HUD_offset_x = 100.0f;
		} else if ( HUD_offset_x < -100.0f )	{
			HUD_offset_x += 100.0f;
		}

		if ( HUD_offset_y > 100.0f )	{
			HUD_offset_y = 100.0f;
		} else if ( HUD_offset_y < -100.0f )	{
			HUD_offset_y += 100.0f;
		}

	} else {
		HUD_offset_x = 0.0f;
		HUD_offset_y = 0.0f;
	}
}

// Basically like gr_reset_clip only it accounts for hud jittering
void HUD_reset_clip()
{
	int hx = fl2i(HUD_offset_x);
	int hy = fl2i(HUD_offset_y);

	gr_set_clip(hx, hy, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled);
}

// Basically like gr_set_clip only it accounts for hud jittering
void HUD_set_clip(int x, int y, int w, int h)
{
	int hx = fl2i(HUD_offset_x);
	int hy = fl2i(HUD_offset_y);

	gr_set_clip(hx+x, hy+y, w, h);
}

// -------------------------------------------------------------------------------------
// hud_save_restore_camera_data()
//
//	Called to save and restore the 3D camera settings.
//
void hud_save_restore_camera_data(int save)
{
	static vec3d	save_view_position;
	static float	save_view_zoom;
	static matrix	save_view_matrix;
	static matrix	save_eye_matrix;
	static vec3d	save_eye_position;

	// save global view variables, so we can restore them
	if ( save ) {
		save_view_position	= View_position;
		save_view_zoom			= View_zoom;
		save_view_matrix		= View_matrix;
		save_eye_matrix		= Eye_matrix;
		save_eye_position		= Eye_position;
	}
	else {
		// restore global view variables
		View_position	= save_view_position;
		View_zoom		= save_view_zoom;
		View_matrix		= save_view_matrix;
		Eye_matrix		= save_eye_matrix;
		Eye_position	= save_eye_position;
	}
}


void hud_toggle_contrast()
{
	HUD_contrast = !HUD_contrast;
}

void hud_set_contrast(int high)
{
	HUD_contrast = high;
}

// Paging functions for the rest of the hud code
extern void hudwingmanstatus_page_in();
extern void hudescort_page_in();
extern void hudets_page_in();
extern void hudlock_page_in();
extern void hudreticle_page_in();
extern void hudshield_page_in();
extern void hudsquadmsg_page_in();
extern void hudtarget_page_in();
extern void hudtargetbox_page_in();

// Page in all hud bitmaps
void hud_page_in()
{
	int i;
#ifdef NEW_HUD
	/*
	//Page in default hud stuff
	for(i = 0; i < default_hud.num_gauges; i++)
	{
		if(default_hud.gauges[i].type != HG_UNUSED)
		{
			default_hud.gauges[i].reset();
			default_hud.gauges[i].page_in();
		}
	}*/
#endif

	bm_page_in_aabitmap( Kills_gauge.first_frame, Kills_gauge.num_frames );
	bm_page_in_aabitmap( Head_frame_gauge.first_frame, Head_frame_gauge.num_frames );
	bm_page_in_aabitmap( Mission_time_gauge.first_frame, Mission_time_gauge.num_frames );
	for ( i = 0; i < NUM_DAMAGE_GAUGES; i++ ) {
		bm_page_in_aabitmap( Damage_gauges[i].first_frame, Damage_gauges[i].num_frames);
	}

	bm_page_in_aabitmap( Netlag_icon.first_frame, Netlag_icon.num_frames);			
	bm_page_in_aabitmap( Support_view_gauge.first_frame, Support_view_gauge.num_frames);
	bm_page_in_aabitmap( Objective_display_gauge.first_frame, Objective_display_gauge.num_frames);		

	// Paging functions for the rest of the hud code
	hudwingmanstatus_page_in();
	hudescort_page_in();
	hudets_page_in();
	hudlock_page_in();
	hudreticle_page_in();
	hudshield_page_in();
	hudsquadmsg_page_in();
	hudtarget_page_in();
	hudtargetbox_page_in();
	//CUSTOM gauges
//	hudcustom_page_in();
}

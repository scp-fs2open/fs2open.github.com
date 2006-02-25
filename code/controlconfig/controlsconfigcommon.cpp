/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/ControlConfig/ControlsConfigCommon.cpp $
 * $Revision: 2.14 $
 * $Date: 2006-02-25 21:46:59 $
 * $Author: Goober5000 $
 *
 * C module for keyboard, joystick and mouse configuration common stuff (between Fred and FreeSpace)
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.13  2005/10/11 05:24:33  wmcoolmon
 * Gliding
 *
 * Revision 2.12  2005/07/13 02:30:52  Goober5000
 * removed autopilot #define
 * --Goober5000
 *
 * Revision 2.11  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.10  2005/01/16 22:39:08  wmcoolmon
 * Added VM_TOPDOWN view; Added 2D mission mode, add 16384 to mission +Flags to use.
 *
 * Revision 2.9  2004/07/26 20:47:26  Kazan
 * remove MCD complete
 *
 * Revision 2.8  2004/07/25 00:31:28  Kazan
 * i have absolutely nothing to say about that subject
 *
 * Revision 2.7  2004/07/12 16:32:43  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.6  2004/05/03 21:22:19  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.5  2004/04/06 03:09:01  phreak
 * added a control config option for the wireframe hud targetbox i enabled ages ago
 *
 * Revision 2.4  2003/11/11 02:15:43  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.3  2002/10/19 19:29:27  bobboau
 * initial commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam shield hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.2  2002/10/17 20:40:50  randomtiger
 * Added ability to remove HUD ingame on keypress shift O
 * So I've added a new key to the bind list and made use of already existing hud removal code.
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    11/01/99 3:36p Jefff
 * had scan code texts for y and z in german swapped
 * 
 * 13    11/01/99 2:16p Jefff
 * minor key name change in german
 * 
 * 12    10/29/99 6:10p Jefff
 * squashed the y/z german issues once and for all
 * 
 * 11    10/28/99 11:16p Jefff
 * Changed some german key names.  Made key translations always use the
 * english table.
 * 
 * 10    10/28/99 2:05a Jefff
 * revised some german key names.  changed some y/z switch stuff.
 * 
 * 9     10/25/99 5:39p Jefff
 * swap init binding for y and z keys in German builds
 * 
 * 8     9/01/99 2:56p Jefff
 * a few control key description changes
 * 
 * 7     8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 6     8/19/99 10:59a Dave
 * Packet loss detection.
 * 
 * 5     8/01/99 12:39p Dave
 * Added HUD contrast control key (for nebula).
 * 
 * 4     7/23/99 2:57p Andsager
 * fix translate_key_to_index to work with localization
 * 
 * 3     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 41    6/19/98 3:51p Lawrance
 * localize control text
 * 
 * 40    6/17/98 11:05a Lawrance
 * localize the control config strings
 * 
 * 39    5/24/98 2:28p Hoffoss
 * Because we never really care about if the left or the right shift or
 * alt key was used, but rather than either shift or alt was used, made
 * both map to the left one.  Solves some problems, causes none.
 * 
 * 38    5/22/98 11:21a Hoffoss
 * Added bank when pressed back in.
 * 
 * 37    5/15/98 8:37p Lawrance
 * Add 'equalize recharge rates' and 'target ship that sent last
 * transmission' key bindings
 * 
 * 36    5/13/98 7:15p Hoffoss
 * Fixed remaining bugs with axis binding.
 * 
 * 35    5/13/98 1:17a Hoffoss
 * Added joystick axes configurability.
 * 
 * 34    5/12/98 11:25a Hoffoss
 * Disabled bank when pressed action.
 * 
 * 33    5/09/98 10:48p Lawrance
 * change text for view bindings
 * 
 * 32    5/09/98 4:52p Lawrance
 * Implement padlock view (up/rear/left/right)
 * 
 * 31    4/30/98 9:43p Hoffoss
 * Added new actions for free look movement which are default bounded to
 * the hat.
 * 
 * 30    4/25/98 1:25p Lawrance
 * Add time compression keys to key config
 * 
 * 29    4/25/98 12:43p Allender
 * added new shortcut key for attack my subsystem
 * 
 * 28    4/18/98 5:00p Dave
 * Put in observer zoom key. Made mission sync screen more informative.
 * 
 * 27    4/17/98 1:24a Lawrance
 * fix spelling error
 * 
 * 26    4/15/98 11:06a Lawrance
 * fix bug with a multi key showing up in demo, remove obsolete bindings
 * from demo and full version
 * 
 * 25    4/10/98 12:47p Allender
 * changed working on replay popup.  Don't reference repair in comm menu.
 * Added Shift-R for repair me
 * 
 * 24    4/08/98 11:11a Hoffoss
 * Fixed some bugs that showed up due to fixing other bugs the other day
 * with controls.
 * 
 * 23    4/07/98 4:06p Lawrance
 * Change binding next for bomb targeting
 * 
 * 22    4/07/98 1:52p Lawrance
 * fix error in binding text for energy keys 
 * 
 * 21    4/06/98 11:17a Hoffoss
 * Fixed num lock/pause interplay bug.
 * 
 * 20    4/01/98 6:46p Lawrance
 * change text for Alt+J
 * 
 * 19    3/31/98 4:15p Hoffoss
 * Disabled the show objectives action.
 * 
 * 18    3/25/98 2:16p Dave
 * Select random default image for newly created pilots. Fixed several
 * multi-pause messaging bugs. Begin work on online help for multiplayer
 * keys.
 * 
 * 17    3/19/98 5:04p Dave
 * Put in support for targeted multiplayer text and voice messaging (all,
 * friendly, hostile, individual).
 * 
 * 16    3/17/98 11:25a Hoffoss
 * Fixed some of the action's names.
 * 
 * 15    3/17/98 10:48a Hoffoss
 * Allowed a special hack for "bank while pressed" action to use alt and
 * shift keys standalone.
 * 
 * 14    2/28/98 9:47p Lawrance
 * change some binding text
 * 
 * 13    2/28/98 7:02p Lawrance
 * overhaul on-line help
 * 
 * 12    2/26/98 12:33a Lawrance
 * Added back slew mode,  lots of changes to external and chase views.
 * 
 * 11    2/23/98 11:25a Sandeep
 * added default keys
 * 
 * 10    2/22/98 6:27p Lawrance
 * External mode takes precedence over chase mode.  Fix bug with repair
 * ships docking in chase mode.
 * 
 * 9     2/22/98 12:19p John
 * Externalized some strings
 * 
 * 8     2/02/98 6:59p Lawrance
 * Adding new targeting keys (bomb, uninspected cargo, new ship, live
 * turrets).
 * 
 * 7     1/27/98 4:24p Allender
 * moved hotkey selection to F3 instead of X.  Made palette flash
 * disappear in multiplayer pause menu
 * 
 * 6     1/22/98 4:53p Hoffoss
 * Made training messages/directives display a joystick button in place of
 * a keypress if there is no keypress bound to the action.
 * 
 * 5     1/22/98 4:15p Hoffoss
 * Added code to allow popup to tell player he needs to bind a key for the
 * training mission.
 * 
 * 4     1/02/98 4:42p Allender
 * removed unused key bindings from control config list
 * 
 * 3     12/30/97 4:47p Allender
 * work with ignore my target command.  Added new keyboard hotkey.  Made
 * it work globally
 * 
 * 2     12/24/97 3:37p Hoffoss
 * Moved control config stuff to seperate library to Fred can access it as
 * well.
 * 
 * 1     12/24/97 3:27p Hoffoss
 * 
 * 1     12/24/97 11:58a Hoffoss
 * 
 * 98    12/22/97 2:15p Hoffoss
 * Fixed bug where joystick axis lines weren't being displayed.
 * 
 * 97    12/16/97 2:44p Hoffoss
 * Added clear button to control config screen.
 * 
 * 96    12/12/97 3:07p Hoffoss
 * Changed how deleting bindings work.  Each control of an action can be
 * deleted independently or both at once.
 * 
 * 95    12/07/97 2:36p John
 * Made warp out be Alt+J instead of J
 * 
 * 94    12/03/97 4:59p Hoffoss
 * Added reset sound and change control config sounds around.
 * 
 * 93    12/03/97 4:16p Hoffoss
 * Changed sound stuff used in interface screens for interface purposes.
 * 
 * 92    12/01/97 5:25p Hoffoss
 * Routed interface sound playing through a special function that will
 * only allow one instance of the sound to play at a time, avoiding
 * over-mixing problems.
 * 
 * 91    12/01/97 3:38p Hoffoss
 * Changed placement of 'More' indicator.
 * 
 * 90    11/25/97 3:49p Hoffoss
 * Changed subhilighting to happen when mouse is over line, rather than
 * mouse button down but not up again yet.
 * 
 * 89    11/24/97 10:20p Lawrance
 * Add key 'KEY_N' to target next ship on monitoring view
 * 
 * 88    11/24/97 6:15p Lawrance
 * fix button scroll problem
 * 
 * 87    11/23/97 6:15p Hoffoss
 * Make exiting the controls config screen save the pilot, so changes keep
 * even if programs crashes or something.
 * 
 * 86    11/21/97 5:57p Hoffoss
 * Fixed bug where timef type controls weren't checking joystick buttons.
 * 
 * 85    11/21/97 4:06p Hoffoss
 * Fixed bug with enabling of axis are reversed.
 * 
 * 84    11/20/97 4:47p Hoffoss
 * Swapped throttle and rudder, which apparently were backwards.
 * 
 * 83    11/20/97 4:00p Lawrance
 * set z to a valid value
 * 
 * 82    11/20/97 3:52p Hoffoss
 * Made "None" appear if no control is bound to function.
 * 
 * 81    11/20/97 2:10p Hoffoss
 * Added defaults for  joystick buttons 2 and 3.
 * 
 * 80    11/20/97 1:08a Lawrance
 * add support for 'R' key - target closest friendly repair ship
 * 
 * 79    11/19/97 8:33p Hoffoss
 * New controls config screen baby!
 * 
 * 78    11/17/97 7:09p Hoffoss
 * Made chase view control a trigger type.
 * 
 * 77    11/17/97 6:40p Lawrance
 * Changed 'I' key to toggle of extended target info, moved target closest
 * locked missile to 'L'
 * 
 * 76    11/17/97 3:26p Jasen
 * Adjusted coordinates for the help button .ani location
 * 
 * 75    11/16/97 3:51p Hoffoss
 * Added more button to config screen when scrolling allowed.
 * 
 * 74    11/16/97 3:20p Hoffoss
 * Added various suggestions from the Todolist.
 * 
 * 73    11/14/97 4:33p Mike
 * Change Debug key to backquote (from F11).
 * Balance a ton of subsystems in ships.tbl.
 * Change "Heavy Laser" to "Disruptor".
 * 
 * 72    11/12/97 2:54p Hoffoss
 * Made modifiers non-stand alone, added support for red tabs on
 * conflicts.
 * 
 * 71    10/30/97 6:01p Hoffoss
 * Changed screen to utilize the new text color standards.
 * 
 * 70    10/29/97 7:25p Hoffoss
 * Added crude support for UI button double click checking.
 * 
 * 69    10/29/97 6:32p Hoffoss
 * Changed some interface apperances.
 * 
 * 68    10/29/97 4:54p Hoffoss
 * Changed scan_code_text element for Enter to 'Enter' instead of 'a' with
 * a caret over it (what's that all about?)
 * 
 * 67    10/28/97 10:42a Hoffoss
 * Fixed handling of continuous controls that have modifiers to the key.
 * 
 * 66    10/28/97 10:03a Hoffoss
 * Fixed bug with continuous type controls not registering if pressed and
 * released too quickly.  Also some other little changes.
 * 
 * 65    10/28/97 12:12a Lawrance
 * remove unused keys (Alt-H and Alt-F)
 * 
 * 64    10/27/97 12:23p Hoffoss
 * Improved warning system and fixed coloring bug.
 * 
 * 63    10/27/97 11:39a Hoffoss
 * Added control conflicts checking.
 * 
 * 62    10/26/97 3:20p Hoffoss
 * Added many missing features to control config screen.
 * 
 * 61    10/25/97 5:41p Hoffoss
 * More functionality added to the controls config screen.
 * 
 * 60    10/24/97 11:00p Hoffoss
 * Controls config screen much improved.
 * 
 * 59    10/22/97 4:50p Hoffoss
 * Disabled throttle by default.
 * 
 * 58    10/22/97 11:00a Hoffoss
 * Changed VIEW_SLEW and VIEW_EXTERNAL to be continuous rather than
 * trigger actions.
 * 
 * 57    10/21/97 7:05p Hoffoss
 * Overhauled the key/joystick control structure and usage throughout the
 * entire FreeSpace code.  The whole system is very different now.
 * 
 * 56    10/18/97 7:19p Hoffoss
 * Added timestamp recording when a key is pressed.
 * 
 * 55    10/13/97 4:33p Hoffoss
 * Made training messages go away after time.
 * 
 * 54    10/13/97 3:24p Hoffoss
 * Made it so training message text can be arbitrarily bolded.
 * 
 * 53    9/24/97 4:52p Hoffoss
 * Changed training message key token handling, and implemented a new
 * training message system method to test out for a while.
 * 
 * 52    9/14/97 10:24p Lawrance
 * add damage screen popup window
 * 
 * 51    9/10/97 6:02p Hoffoss
 * Added code to check for key-pressed sexp operator in FreeSpace as part
 * of training mission stuff.
 * 
 * 50    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * $NoKeywords: $
 *
*/

#include "controlconfig/controlsconfig.h"
#include "io/key.h"
#include "io/joy.h"
#include "localization/localize.h"


#define TARGET_TAB			0
#define SHIP_TAB				1
#define WEAPON_TAB			2
#define COMPUTER_TAB			3

int Failed_key_index;

// assume control keys are used as modifiers until we find out 
int Shift_is_modifier;
int Ctrl_is_modifier;
int Alt_is_modifier;

int Axis_enabled[JOY_NUM_AXES] = { 1, 1, 1, 0, 0, 0 };
int Axis_enabled_defaults[JOY_NUM_AXES] = { 1, 1, 1, 0, 0, 0 };
int Invert_axis[JOY_NUM_AXES] = { 0, 0, 0, 0, 0, 0 };
int Invert_axis_defaults[JOY_NUM_AXES] = { 0, 0, 0, 0, 0, 0 };

// arrays which hold the key mappings.  The array index represents a key-independent action.
//
//XSTR:OFF
config_item Control_config[CCFG_MAX + 1] = {
	// targeting a ship
	{                           KEY_T,				-1, TARGET_TAB,	true, "Target Next Ship" },
	{             KEY_SHIFTED | KEY_T,				-1, TARGET_TAB,	true, "Target Previous Ship" },
	{                           KEY_H,				2,  TARGET_TAB,	true, "Target Next Closest Hostile Ship" },
	{	           KEY_SHIFTED | KEY_H,				-1, TARGET_TAB,	true, "Target Previous Closest Hostile Ship" },
	{ KEY_ALTED |               KEY_H,				-1, TARGET_TAB,	true, "Toggle Auto Targeting" },
	{                           KEY_F,				-1, TARGET_TAB,	true, "Target Next Closest Friendly Ship" },
	{             KEY_SHIFTED | KEY_F,				-1, TARGET_TAB,	true, "Target Previous Closest Friendly Ship" },
	{                           KEY_Y,				4,  TARGET_TAB,	true, "Target Ship in Reticle" },
	{                           KEY_G,				-1, TARGET_TAB,	true, "Target Target's Nearest Attacker" },
	{ KEY_ALTED	|					 KEY_Y,			-1, TARGET_TAB,	true, "Target Last Ship to Send Transmission" },
	{ KEY_ALTED |               KEY_T,				-1, TARGET_TAB,	true, "Turn Off Auto-Targeting" },

	// targeting a ship's subsystem
	{                           KEY_V,				-1, TARGET_TAB,	true, "Target Subsystem in Reticle" },
	{                           KEY_S,				-1, TARGET_TAB,	true, "Target Next Subsystem" },
	{             KEY_SHIFTED | KEY_S,				-1, TARGET_TAB,	true, "Target Previous Subsystem" },
	{ KEY_ALTED |               KEY_S,				-1, TARGET_TAB,	true, "Turn Off Auto-Targeting of Subsystems" },

	// matching speed
	{                           KEY_M,				-1, COMPUTER_TAB,	true, "Match Target Speed" },
	{ KEY_ALTED |               KEY_M,				-1, COMPUTER_TAB,	true, "Toggle Auto Speed Matching" },

	// weapons
	{                           KEY_LCTRL,			0,	 WEAPON_TAB,	true, "Fire Primary Weapon", CC_TYPE_CONTINUOUS },
	{                           KEY_SPACEBAR,		1,  WEAPON_TAB,	true, "Fire Secondary Weapon", CC_TYPE_CONTINUOUS },
	{                           KEY_PERIOD,			-1, WEAPON_TAB,	true, "Cycle Forward Primary Weapon" },
	{                           KEY_COMMA,			-1, WEAPON_TAB,	true, "Cycle Backward Primary Weapon" },
	{                           KEY_DIVIDE,			-1, WEAPON_TAB,	true, "Cycle Secondary Weapon Bank" },
	{             KEY_SHIFTED | KEY_DIVIDE,			-1, WEAPON_TAB,	true, "Cycle Secondary Weapon Firing Rate" },
	{                           KEY_X,				3,	 WEAPON_TAB,	true, "Launch Countermeasure" },

	// controls
	{                           KEY_A,				-1, SHIP_TAB,		true, "Forward Thrust", CC_TYPE_CONTINUOUS },
	{                           KEY_Z,				-1, SHIP_TAB,		true, "Reverse Thrust", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD7,			-1, SHIP_TAB,		true, "Bank Left", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD9,			-1, SHIP_TAB,		true, "Bank Right", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD8,			-1, SHIP_TAB,		true, "Pitch Forward", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD2,			-1, SHIP_TAB,		true, "Pitch Backward", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD4,			-1, SHIP_TAB,		true, "Turn Left", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD6,			-1, SHIP_TAB,		true, "Turn Right", CC_TYPE_CONTINUOUS },

	// throttle controls
	{                           KEY_BACKSP,			-1, SHIP_TAB,		true, "Set Throttle to Zero" },
	{                           KEY_SLASH,			-1, SHIP_TAB,		true, "Set Throttle to Max" },
	{                           KEY_LBRACKET,		-1, SHIP_TAB,		true, "Set Throttle to One-Third" },
	{                           KEY_RBRACKET,		-1, SHIP_TAB,		true, "Set Throttle to Two-Thirds" },
	{                           KEY_EQUAL,			-1, SHIP_TAB,		true, "Increase Throttle 5 Percent" },
	{                           KEY_MINUS,			-1, SHIP_TAB,		true, "Decrease Throttle 5 Percent" },

	// squadmate messaging
	{             KEY_SHIFTED | KEY_A,				-1, COMPUTER_TAB,	true, "Attack My Target" },
	{             KEY_SHIFTED | KEY_Z,				-1, COMPUTER_TAB,	true, "Disarm My Target" },
	{             KEY_SHIFTED | KEY_D,				-1, COMPUTER_TAB,	true, "Disable My Target" },
	{             KEY_SHIFTED | KEY_V,				-1, COMPUTER_TAB,	true, "Attack my Subsystem" },
	{             KEY_SHIFTED | KEY_X,				-1, COMPUTER_TAB,	true, "Capture My Target" },
	{             KEY_SHIFTED | KEY_E,				-1, COMPUTER_TAB,	true, "Engage Enemy" },
	{             KEY_SHIFTED | KEY_W,				-1, COMPUTER_TAB,	true, "Form on my Wing" },
	{             KEY_SHIFTED | KEY_I,				-1, COMPUTER_TAB,	true, "Ignore my Target" },
	{             KEY_SHIFTED | KEY_P,				-1, COMPUTER_TAB,	true, "Protect my Target" },
	{             KEY_SHIFTED | KEY_C,				-1, COMPUTER_TAB,	true, "Cover me" },
	{             KEY_SHIFTED | KEY_J,				-1, COMPUTER_TAB,	true, "Return to base" },
	{				  KEY_SHIFTED | KEY_R,			-1, COMPUTER_TAB, true, "Rearm me" },

	{									 KEY_R,		6,  TARGET_TAB,	true, "Target Closest Attacking Ship" },

	// Views
	{                           KEY_PADMULTIPLY,	-1, COMPUTER_TAB,	true, "Chase View" },
	{                           KEY_PADPERIOD,		-1, COMPUTER_TAB,	true, "External View"},
	{                           KEY_PADENTER,		-1, COMPUTER_TAB,	true, "Toggle External Camera Lock"},
	{                           KEY_PAD0,			-1, COMPUTER_TAB,	true, "Free Look View", CC_TYPE_CONTINUOUS },
	{                           KEY_PADDIVIDE,		-1, COMPUTER_TAB,	true, "Current Target View" },
	{                           KEY_PADPLUS,		-1, COMPUTER_TAB,	true, "Increase View Distance", CC_TYPE_CONTINUOUS },
	{                           KEY_PADMINUS,		-1, COMPUTER_TAB,	true, "Decrease View Distance", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD5,			-1, COMPUTER_TAB,	true, "Center View", CC_TYPE_CONTINUOUS },
	{							-1,					33, COMPUTER_TAB, true, "View Up", CC_TYPE_CONTINUOUS },
	{							-1,					32, COMPUTER_TAB, true, "View Rear", CC_TYPE_CONTINUOUS },
	{							-1,					34, COMPUTER_TAB, true, "View Left", CC_TYPE_CONTINUOUS },
	{							-1,					35, COMPUTER_TAB, true, "View Right", CC_TYPE_CONTINUOUS },

	{                           KEY_RAPOSTRO,		-1, COMPUTER_TAB,	true, "Cycle Radar Range" },
	{                           KEY_C,				-1, COMPUTER_TAB, true, "Communications Menu" },
	{                           -1,					-1, -1,				true, "Show Objectives" },
	{ KEY_ALTED |               KEY_J,				-1, COMPUTER_TAB,	true, "Enter Subspace (End Mission)" },
	{                           KEY_J,				-1, TARGET_TAB,	true, "Target Target's Target" },
	{                           KEY_TAB,			5,  SHIP_TAB,		true, "Afterburner", CC_TYPE_CONTINUOUS },
	
	{                           KEY_INSERT,		-1, COMPUTER_TAB,	true, "Increase Weapon Energy" },
	{                           KEY_DELETE,		-1, COMPUTER_TAB,	true, "Decrease Weapon Energy" },
	{                           KEY_HOME,			-1, COMPUTER_TAB,	true, "Increase Shield Energy" },
	{                           KEY_END,			-1, COMPUTER_TAB,	true, "Decrease Shield Energy" },
	{                           KEY_PAGEUP,		-1, COMPUTER_TAB,	true, "Increase Engine Energy" },
	{                           KEY_PAGEDOWN,		-1, COMPUTER_TAB,	true, "Decrease Engine Energy" },
	{ KEY_ALTED |               KEY_D,				-1, COMPUTER_TAB, true, "Equalize Energy Settings" },

	{                           KEY_Q,				7,  COMPUTER_TAB,	true, "Equalize Shield" },
	{                           KEY_UP,				-1, COMPUTER_TAB,	true, "Augment Forward Shield" },
	{                           KEY_DOWN,			-1, COMPUTER_TAB,	true, "Augment Rear Shield" },
	{                           KEY_LEFT,			-1, COMPUTER_TAB,	true, "Augment Left Shield" },
	{                           KEY_RIGHT,			-1, COMPUTER_TAB,	true, "Augment Right Shield" },
	{                           KEY_SCROLLOCK,	-1, COMPUTER_TAB,	true, "Transfer Energy Laser->Shield" },
	{             KEY_SHIFTED | KEY_SCROLLOCK,	-1, COMPUTER_TAB,	true, "Transfer Energy Shield->Laser" },
	{                           -1,					-1, -1,				true, "Show Damage Popup Window" },	

	{                           -1,					-1, SHIP_TAB,		true, "Bank When Pressed", CC_TYPE_CONTINUOUS },
	{									 -1,					-1, -1,				true, "Show NavMap" },
	{ KEY_ALTED |	             KEY_E,				-1, COMPUTER_TAB,	true, "Add or Remove Escort" },
	{ KEY_ALTED | KEY_SHIFTED | KEY_E,				-1, COMPUTER_TAB,	true, "Clear Escort List" },
	{					             KEY_E,				-1, TARGET_TAB,	true, "Target Next Escort Ship" },
	{ KEY_ALTED	|					 KEY_R,				-1, TARGET_TAB,	true, "Target Closest Repair Ship" },

	{                           KEY_U,				-1, TARGET_TAB,	true, "Target Next Uninspected Cargo" },
	{             KEY_SHIFTED | KEY_U,				-1, TARGET_TAB,	true, "Target Previous Uninspected Cargo" },
	{									 KEY_N,		-1, TARGET_TAB,	true, "Target Newest Ship In Area" },
	{                           KEY_K,				-1, TARGET_TAB,	true, "Target Next Live Turret" },
	{             KEY_SHIFTED | KEY_K,				-1, TARGET_TAB,	true, "Target Previous Live Turret" },

	{									 KEY_B,		-1, TARGET_TAB,	true, "Target Next Hostile Bomb or Bomber" },
	{             KEY_SHIFTED | KEY_B,				-1, TARGET_TAB,	true, "Target Previous Hostile Bomb or Bomber" },

	// multiplayer messaging keys
	{									 KEY_1,				-1, COMPUTER_TAB, true, "(Multiplayer) Message All", CC_TYPE_CONTINUOUS },
	{									 KEY_2,				-1, COMPUTER_TAB, true, "(Multiplayer) Message Friendly", CC_TYPE_CONTINUOUS },
	{									 KEY_3,				-1, COMPUTER_TAB, true, "(Multiplayer) Message Hostile", CC_TYPE_CONTINUOUS },
	{									 KEY_4,				-1, COMPUTER_TAB, true, "(Multiplayer) Message Target", CC_TYPE_CONTINUOUS },
	{ KEY_ALTED	|					 KEY_X,				-1, COMPUTER_TAB, true, "(Multiplayer) Observer zoom to target"},	

	{             KEY_SHIFTED | KEY_PERIOD,		-1, COMPUTER_TAB,	true, "Increase time compression" },
	{             KEY_SHIFTED | KEY_COMMA,			-1, COMPUTER_TAB,	true, "Decrease time compression" },

	{									 KEY_L,				-1, COMPUTER_TAB, true, "Toggle high HUD contrast" },	

	{				  KEY_SHIFTED | KEY_N,				-1, COMPUTER_TAB, true, "(Multiplayer) Toggle network info"},
	{				  KEY_SHIFTED | KEY_END,			-1, COMPUTER_TAB, true, "(Multiplayer) Self destruct"},

	// Misc
	{				  KEY_SHIFTED | KEY_O,			-1, COMPUTER_TAB, true, "Toggle HUD"},
	{				  KEY_SHIFTED | KEY_3,			-1, SHIP_TAB, true, "Right Thrust", CC_TYPE_CONTINUOUS},
	{				  KEY_SHIFTED | KEY_1,			-1, SHIP_TAB, true, "Left Thrust", CC_TYPE_CONTINUOUS},
	{				  KEY_SHIFTED | KEY_PADPLUS,	-1, SHIP_TAB, true, "Up Thrust", CC_TYPE_CONTINUOUS},
	{				  KEY_SHIFTED | KEY_PADENTER,	-1, SHIP_TAB, true, "Down Thrust", CC_TYPE_CONTINUOUS},
	{ KEY_ALTED |     KEY_SHIFTED | KEY_Q,			-1, COMPUTER_TAB, true, "Toggle HUD Wireframe Targetbox"},
	{							-1,					-1,	COMPUTER_TAB, false, "Top-down View"},
	{							-1,					-1, COMPUTER_TAB, false, "Track targeted object", CC_TYPE_CONTINUOUS},

	// Auto Navigation Systen
	{ KEY_ALTED |					KEY_A,			-1, COMPUTER_TAB, false, "Toggle Auto Pilot"},
	{ KEY_ALTED |					KEY_N,			-1, COMPUTER_TAB, false, "Cycle Nav Points"},
	
	{ KEY_ALTED |					KEY_G,			-1, SHIP_TAB, false, "Toggle gliding"},

	{                           -1,					-1, -1,			 false,	"" }
};

char *Scan_code_text_german[] = {
	"",				"Esc",				"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"Akzent '",				"\xE1",				"R\x81""cktaste",		"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Z",				"U",				"I",
	"O",				"P",				"\x9A",				"+",				"Eingabe",			"Strg Links",			"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				"\x99",
	"\xAE",				"`",				"Shift",			"#",				"Y",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"-",				"Shift",			"Num *",
	"Alt",				"Leertaste",			"Hochstell",			"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",				"Pause",			"Rollen",			"Num 7",
	"Num 8",			"Num 9",			"Num -",			"Num 4",			"Num 5",			"Num 6",			"Num +",			"Num 1",
	"Num 2",			"Num 3",			"Num 0",			"Num ,",			"",				"",				"",				"F11",
	"F12",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Num Eingabe",			"Strg Rechts",			"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Num /",			"",				"Druck",
	"Alt",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",			"",				"Pos 1",
	"Pfeil Hoch",			"Bild Hoch",			"",				"Pfeil Links",			"",				"Pfeil Rechts",			"",				"Ende",
	"Pfeil Runter", 			"Bild Runter",			"Einfg",			"Entf",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

char *Joy_button_text_german[] = {
	"Knopf 1",		"Knopf 2",		"Knopf 3",		"Knopf 4",		"Knopf 5",		"Knopf 6",
	"Knopf 7",		"Knopf 8",		"Knopf 9",		"Knopf 10",		"Knopf 11",		"Knopf 12",
	"Knopf 13",		"Knopf 14",		"Knopf 15",		"Knopf 16",		"Knopf 17",		"Knopf 18",
	"Knopf 19",		"Knopf 20",		"Knopf 21",		"Knopf 22",		"Knopf 23",		"Knopf 24",
	"Knopf 25",		"Knopf 26",		"Knopf 27",		"Knopf 28",		"Knopf 29",		"Knopf 30",
	"Knopf 31",		"Knopf 32",		"Hut Hinten",	"Hut Vorne",	"Hut Links",	"Hut Rechts"
};

char *Scan_code_text_french[] = {
	"",				"\x90""chap",			"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"-",				"=",				"Fl\x82""che Ret.",			"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Entr\x82""e",			"Ctrl Gauche",			"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"Maj.",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"Maj.",			"Pav\x82 *",
	"Alt",				"Espace",			"Verr. Maj.",			"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",				"Pause",			"Arret defil",		"Pav\x82 7",
	"Pav\x82 8",			"Pav\x82 9",			"Pav\x82 -",			"Pav\x82 4",			"Pav\x82 5",			"Pav\x82 6",			"Pav\x82 +",			"Pav\x82 1",
	"Pav\x82 2",			"Pav\x82 3",			"Pav\x82 0",			"Pav\x82 .",			"",				"",				"",				"F11",
	"F12",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Pav\x82 Entr",			"Ctrl Droite",		"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Pav\x82 /",			"",				"Impr \x82""cran",
	"Alt",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Verr num",			"",				"Orig.",
	"Fl\x82""che Haut",			"Page Haut",			"",				"Fl\x82""che Gauche",			"",				"Fl\x82""che Droite",			"",			"Fin",
	"Fl\x82""che Bas", 			"Page Bas",			"Inser",			"Suppr",			"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

char *Joy_button_text_french[] = {
	"Bouton 1",		"Bouton 2",		"Bouton 3",		"Bouton 4",		"Bouton 5",		"Bouton 6",
	"Bouton 7",		"Bouton 8",		"Bouton 9",		"Bouton 10",		"Bouton 11",		"Bouton 12",
	"Bouton 13",		"Bouton 14",		"Bouton 15",		"Bouton 16",		"Bouton 17",		"Bouton 18",
	"Bouton 19",		"Bouton 20",		"Bouton 21",		"Bouton 22",		"Bouton 23",		"Bouton 24",
	"Bouton 25",		"Bouton 26",		"Bouton 27",		"Bouton 28",		"Bouton 29",		"Bouton 30",
	"Bouton 31",		"Bouton 32",		"Chapeau Arrière",		"Chapeau Avant",		"Chapeau Gauche",		"Chapeau Droite"
};

//	This is the text that is displayed on the screen for the keys a player selects
char *Scan_code_text_english[] = {
	"",				"Esc",			"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"-",				"=",				"Backspace",	"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Enter",			"Left Ctrl",	"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"Shift",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"Shift",			"Pad *",
	"Alt",			"Spacebar",		"Caps Lock",	"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",			"Pause",			"Scroll Lock",	"Pad 7",
	"Pad 8",			"Pad 9",			"Pad -",			"Pad 4",			"Pad 5",			"Pad 6",			"Pad +",			"Pad 1",
	"Pad 2",			"Pad 3",			"Pad 0",			"Pad .",			"",				"",				"",				"F11",
	"F12",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Pad Enter",	"Right Ctrl",	"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Pad /",			"",				"Print Scrn",
	"Alt",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",		"",				"Home",
	"Up Arrow",		"Page Up",		"",				"Left Arrow",	"",				"Right Arrow",	"",				"End",
	"Down Arrow",  "Page Down",	"Insert",		"Delete",		"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

char *Joy_button_text_english[] = {
	"Button 1",		"Button 2",		"Button 3",		"Button 4",		"Button 5",		"Button 6",
	"Button 7",		"Button 8",		"Button 9",		"Button 10",	"Button 11",	"Button 12",
	"Button 13",	"Button 14",	"Button 15",	"Button 16",	"Button 17",	"Button 18",
	"Button 19",	"Button 20",	"Button 21",	"Button 22",	"Button 23",	"Button 24",
	"Button 25",	"Button 26",	"Button 27",	"Button 28",	"Button 29",	"Button 30",
	"Button 31",	"Button 32",	"Hat Back",		"Hat Forward",	"Hat Left",		"Hat Right"
};

char **Scan_code_text = Scan_code_text_english;
char **Joy_button_text = Joy_button_text_english;

void set_modifier_status()
{
	int i;

	Alt_is_modifier = 0;
	Shift_is_modifier = 0;
	Ctrl_is_modifier = 0;

	for (i=0; i<CCFG_MAX; i++) {
		if (Control_config[i].key_id < 0)
			continue;

		if (Control_config[i].key_id & KEY_ALTED)
			Alt_is_modifier = 1;

		if (Control_config[i].key_id & KEY_SHIFTED)
			Shift_is_modifier = 1;

		if (Control_config[i].key_id & KEY_CTRLED) {
			Assert(0);  // get Alan
			Ctrl_is_modifier = 1;
		}
	}
}

int translate_key_to_index(char *key)
{
	int i, index = -1, alt = 0, shift = 0, max_scan_codes;

	if (Lcl_gr) {
		max_scan_codes = sizeof(Scan_code_text_german) / sizeof(char *);
	} else if (Lcl_fr) {
		max_scan_codes = sizeof(Scan_code_text_french) / sizeof(char *);
	} else {
		max_scan_codes = sizeof(Scan_code_text_english) / sizeof(char *);
	}

	// look for modifiers
	Assert(key);
	if (!strnicmp(key, "Alt", 3)) {
		alt = 1;
		key += 3;
		if (*key)
			key++;
	}

	char *translated_shift;
	
	if(Lcl_gr){
		translated_shift = "Shift";
	} else if(Lcl_fr){	
		translated_shift = "Maj.";
	} else {	
		translated_shift = "Shift";
	}

	if (!strnicmp(key, translated_shift, 5)) {
		shift = 1;
		key += 5;
		if (*key)
			key++;
	}

	// look up index for default key
	if (*key) {
		for (i=0; i<max_scan_codes; i++)
			if (!stricmp(key, Scan_code_text_english[i])) {
				index = i;
				break;
			}

		if (i == max_scan_codes)
			return -1;

		if (shift)
			index |= KEY_SHIFTED;
		if (alt)
			index |= KEY_ALTED;

		// convert scancode to Control_config index
		for (i=0; i<CCFG_MAX; i++) {
			if (Control_config[i].key_default == index) {
				index = i;
				break;
			}
		}

		if (i == CCFG_MAX)
			return -1;

		return index;
	}

	return -1;
}

// Given the system default key 'key', return the current key that is bound to the function
// Both are 'key' and the return value are descriptive strings that can be displayed
// directly to the user.  If 'key' isn't a real key or not normally bound to anything,
// or there is no key current bound to the function, NULL is returned.
char *translate_key(char *key)
{
	int index = -1, code = -1;

	index = translate_key_to_index(key);
	if (index < 0)
		return NULL;

	code = Control_config[index].key_id;
	Failed_key_index = index;
	if (code < 0) {
		code = Control_config[index].joy_id;
		if (code >= 0)
			return Joy_button_text[code];
	}

	return textify_scancode(code);
}

char *textify_scancode(int code)
{
	static char text[40];

	if (code < 0)
		return "None";

	*text = 0;
	if (code & KEY_ALTED) {
		if(Lcl_gr){		
			strcat(text, "Alt-");
		} else if(Lcl_fr){		
			strcat(text, "Alt-");
		} else {		
			strcat(text, "Alt-");
		}		
	}

	if (code & KEY_SHIFTED) {		
		if(Lcl_gr){
			strcat(text, "Shift-");
		} else if(Lcl_fr){		
			strcat(text, "Maj.-");
		} else {		
			strcat(text, "Shift-");
		}
	}

	strcat(text, Scan_code_text[code & KEY_MASK]);
	return text;
}
//XSTR:ON

// initialize common control config stuff - call at game startup after localization has been initialized
void control_config_common_init()
{
	if(Lcl_gr){
		Scan_code_text = Scan_code_text_german;
		Joy_button_text = Joy_button_text_german;
		
		// swap init bindings for y and z keys
		Control_config[TARGET_SHIP_IN_RETICLE].key_default = KEY_Z;
		Control_config[TARGET_LAST_TRANMISSION_SENDER].key_default = KEY_ALTED | KEY_Z;
		Control_config[REVERSE_THRUST].key_default = KEY_Y;
		Control_config[DISARM_MESSAGE].key_default = KEY_SHIFTED | KEY_Y;		
	} else if(Lcl_fr){
		Scan_code_text = Scan_code_text_french;
		Joy_button_text = Joy_button_text_french;
	} else {
		Scan_code_text = Scan_code_text_english;
		Joy_button_text = Joy_button_text_english;
	}
}

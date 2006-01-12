/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GameSequence/GameSequence.h $
 * $Revision: 2.9 $
 * $Date: 2006-01-12 17:42:56 $
 * $Author: wmcoolmon $
 *
 * Header file for Game Sequencing items
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2005/07/13 02:50:53  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.7  2005/03/25 06:57:33  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.6  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.5  2004/08/11 05:06:23  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.4  2004/05/03 21:22:20  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
 *
 * Revision 2.3  2004/03/08 22:02:38  Kazan
 * Lobby GUI screen restored
 *
 * Revision 2.2  2004/03/05 09:02:03  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     9/03/99 1:31a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 7     8/27/99 12:04a Dave
 * Campaign loop screen.
 * 
 * 6     8/04/99 5:36p Andsager
 * Show upsell screens at end of demo campaign before returning to main
 * hall.
 * 
 * 5     2/23/99 2:29p Dave
 * First run of oldschool dogfight mode. 
 * 
 * 4     11/05/98 5:55p Dave
 * Big pass at reducing #includes
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
 * 83    5/15/98 12:09a Dave
 * New tracker api code. New game tracker code. Finished up first run of
 * the PXO screen. Fixed a few game server list exceptions.
 * 
 * 82    5/12/98 2:46a Dave
 * Rudimentary communication between Parallax Online and freespace. Can
 * get and store channel lists.
 * 
 * 81    4/25/98 7:39p Allender
 * fixd some small hotkey stuff.  Worked on turret orientation being
 * correct for multiplayer.  new sexpression called end-campaign will will
 * end the main campaign
 * 
 * 80    4/23/98 7:08p John
 * Removed some obsoleted states.
 * 
 * 79    4/16/98 4:31p Hoffoss
 * Changed demo screen referenced to view cutscenes screen, which is now
 * what it's called.
 * 
 * 78    4/02/98 5:40p Hoffoss
 * Added the Load Mission screen to FreeSpace.
 * 
 * 77    3/11/98 5:32p Lawrance
 * Fix up text arrays for events/states
 * 
 * 76    3/09/98 12:13a Lawrance
 * Add support for Red Alert missions
 * 
 * 75    3/07/98 5:44p Dave
 * Finished player info popup. Ironed out a few todo bugs.
 * 
 * 74    3/05/98 4:12p John
 * Made Debug+F4 switch Glide and windowed.
 * 
 * 73    3/03/98 1:00p Hoffoss
 * Added new command briefing event and state.
 * 
 * 72    3/02/98 3:44p Hoffoss
 * Added new Campaign Room state and event.
 * 
 * 71    2/19/98 6:26p Dave
 * Fixed a few file xfer bugs. Tweaked mp team select screen. Put in
 * initial support for player data uploading.
 * 
 * 70    2/18/98 10:21p Dave
 * Ripped out old file xfer system. Put in brand new xfer system.
 * 
 * 69    2/08/98 5:07p Dave
 * Put in support for multiplayer furball mode.
 * 
 * 68    1/28/98 6:22p Dave
 * Made standalone use ~8 megs less memory. Fixed multiplayer submenu
 * sequencing bug.
 * 
 * 67    1/23/98 5:43p Dave
 * Finished bringing standalone up to speed. Coded in new host options
 * screen.
 * 
 * 66    1/22/98 5:25p Dave
 * Modified some pregame sequencing packets. Starting to repair broken
 * standalone stuff.
 * 
 * 65    1/20/98 5:42p Dave
 * Moved ingame join to its own module. Improved it a bit.
 * 
 * 64    1/15/98 6:12p Dave
 * Fixed weapons loadout bugs with multiplayer respawning. Added
 * multiplayer start screen. Fixed a few chatbox bugs.
 * 
 * 63    1/15/98 6:00p Hoffoss
 * Added option to quit menu (in game) to restart the mission.  Doesn't
 * seem to quite work, though.  Checking code in so someone else can look
 * into it.
 * 
 * 62    1/05/98 10:05a Dave
 * Big re-sequencing of server transfer. Centralized _all_ server transfer
 * code to one module.
 * 
 * 61    12/30/97 4:28p Lawrance
 * Give text descriptions for events, change debug output to give text
 * desciption of event/state
 * 
 * 60    12/24/97 8:56p Lawrance
 * took out obsolete state used for non-existant sound config screen
 * 
 * 59    12/17/97 8:44p John
 * added code to warp the player out of mission no matter what; no
 * cancelling, no max speed check.
 * 
 * 58    11/15/97 2:37p Dave
 * More multiplayer campaign support.
 * 
 * 57    11/13/97 7:01p Hoffoss
 * Fixed GS_state_text[], which didn't match the current states we have
 * available.
 * 
 * 56    11/11/97 4:57p Dave
 * Put in support for single vs. multiplayer pilots. Began work on
 * multiplayer campaign saving. Put in initial player select screen
 * 
 * 55    11/10/97 6:02p Hoffoss
 * Added new debug paused state.
 * 
 * 54    11/03/97 10:12p Hoffoss
 * Finished up work on the hud message/mission log scrollback screen.
 * 
 * 53    10/27/97 6:11p Dave
 * Changed host/server transfer around. Added some multiplayer data to
 * state save/restore. Made multiplayer quitting more intelligent.
 * 
 * 52    10/27/97 8:33a John
 * added code for new player warpout sequence
 * 
 * 51    10/24/97 6:19p Dave
 * More standalone testing/fixing. Added reliable endgame sequencing.
 * Added reliable ingame joining. Added reliable stats transfer (endgame).
 * Added support for dropping players in debriefing. Removed a lot of old
 * unused code.
 * 
 * 50    10/18/97 7:47p Hoffoss
 * Changed state workings for controls config screen.
 * 
 * 49    10/02/97 9:53p Hoffoss
 * Added event evaluation analysis debug screen so we can determine the
 * state of events and their sexp trees to track down logic problems and
 * such.
 * 
 * 48    10/02/97 4:52p Dave
 * Added event to move to the multi wait state with a post_event
 * 
 * 47    10/01/97 4:52p Dave
 * Got ingame join and observer mode to correctly work under the new
 * player_obj system.
 * 
 * 46    9/30/97 5:08p Dave
 * Began work on ingame join ship selection screen/state.
 * 
 * 45    9/23/97 11:53p Lawrance
 * add state do perform multiplayer on-line help
 * 
 * 44    9/22/97 4:55p Hoffoss
 * Added a training message window display thingy.
 * 
 * 43    9/19/97 4:24p Allender
 * added team selection state -- initialze player* variable in
 * player_level_init
 * 
 * 42    9/18/97 10:17p Lawrance
 * add help state for briefing
 * 
 * 41    9/18/97 9:21a Dave
 * Added view medals state. Changed pilot scoring struct to reflect.
 * 
 * 40    8/29/97 4:51p Dave
 * Added a state and even for multiplayer pausing.
 * 
 * 39    8/15/97 5:15p Dave
 * Added a file xfer state.
 * 
 * 38    8/15/97 9:29a Dave
 * Removed standalone server briefing wait state.
 * 
 * 37    8/04/97 4:39p Dave
 * Added first 3 standalone state handlers
 * 
 * 36    7/30/97 5:24p Dave
 * Added in demo system state.
 * 
 * 35    7/24/97 2:17p Dave
 * Added show statistics state.
 * 
 * 34    7/23/97 4:51p Dave
 * Added join tracker state
 * 
 * 33    7/14/97 12:03a Lawrance
 * added navmap state
 * 
 * 32    6/13/97 2:30p Lawrance
 * Added debriefings
 * 
 * 31    6/12/97 9:13a Allender
 * added sequencing state to the end of ship selection.  Changed some
 * packet names and host sequencing
 * 
 * 30    6/10/97 9:56p Allender
 * get multiplayer mission selection working.  Host can select mission and
 * have himself and clients load the mission -- no sequencing past this
 * point however
 * 
 * 29    6/06/97 10:40a Allender
 * added 'type' to mission (single/multi/etc).  Added a couple of new game
 * states for allowing to choose mission for multiplayer game
 * 
 * 28    5/20/97 10:02a Lawrance
 * added view medals screen
 * 
 * 27    4/28/97 2:17p Lawrance
 * added help state for hotkey assignment screen
 * 
 * 26    4/25/97 3:41p Lawrance
 * added support for hotkey assignment screen
 * 
 * 25    4/23/97 9:54a Lawrance
 * made show goals screen a separate state
 * 
 * 24    4/22/97 11:06a Lawrance
 * added credits state
 * 
 * 23    4/17/97 9:01p Allender
 * start of campaign stuff.  Campaigns now stored in external file (no
 * filenames in code).  Continuing campaign won't work at this point
 * 
 * 22    4/03/97 8:40p Lawrance
 * give GS_STATE's for player death correct numbering
 * 
 * 21    4/02/97 6:03p Mike
 * Make dying work through event driven code.
 * 
 * 20    3/05/97 5:04p Lawrance
 * added new states for different context help
 * 
 * 19    3/03/97 1:21p Allender
 * mission log stuff -- display the log during/after game.  Enhanced
 * structure
 * 
 * 18    3/02/97 4:43p Lawrance
 * Added in state for sound/music sound config
 * 
 * 17    2/25/97 11:07a Lawrance
 * Added support for weapon loadout state
 * 
 * 16    1/09/97 12:57p Lawrance
 * supporting a new state where the player picks to either save or restore
 * 
 * 15    1/09/97 12:41a Lawrance
 * added function to pop a state without restoring that state
 * 
 * 14    1/06/97 10:44p Lawrance
 * Changes to make save/restore functional
 * 
 * 13    1/01/97 6:44p Lawrance
 * added new state for protocol choice in a net game
 * 
 * 12    12/03/96 3:45p Lawrance
 * supporting control configuration
 * 
 * 11    12/01/96 3:49a Lawrance
 * adding support for various multiplayer states
 * 
 * 10    11/29/96 6:09p Lawrance
 * added a state for HUD configuration
 * 
 * 9     11/27/96 3:21p Lawrance
 * added state for when player is examining message scroll-back
 * 
 * 8     11/19/96 1:22p Lawrance
 * added event to start a briefing
 * 
 * 7     11/18/96 8:12p John
 * Changed some briefing and sequencing stuff around.
 * 
 * 6     11/18/96 5:07p John
 * Changed sequencing code to call entry,leave functions for each state
 * change.   Added Shift+Pause debug pause thing.
 * 
 * 5     11/13/96 4:02p Lawrance
 * complete over-haul of the menu system and the states associated with
 * them
 * 
 * 4     10/23/96 9:08a Allender
 * Removed primary and secondary goal complete states -- to be implemented
 * later.
 *
*/


// defines for game sequencing

#ifndef __GAMESEQUENCE_H__
#define __GAMESEQUENCE_H__

// defines for game sequencing events
//

#define GS_EVENT_MAIN_MENU						0		// first event to move to first state
#define GS_EVENT_START_GAME						1		// start a new game (Loads a mission then goes to briefing state)
#define GS_EVENT_ENTER_GAME						2		// switches into game state, probably after mission briefing or ship selection.
#define GS_EVENT_START_GAME_QUICK				3		// start a new game (Loads a mission then goes to directly to game state)
#define GS_EVENT_END_GAME						4		// end the current game (i.e. back to main menu)
#define GS_EVENT_QUIT_GAME						5		// quit the entire game
#define GS_EVENT_PAUSE_GAME						6		// pause the current game
#define GS_EVENT_PREVIOUS_STATE					7		// return to the previous state
#define GS_EVENT_OPTIONS_MENU					8		// go to the options menu
#define GS_EVENT_BARRACKS_MENU					9		// go to the barracks menu
#define GS_EVENT_TRAINING_MENU					10		// go to the training menu
#define GS_EVENT_TECH_MENU						11		// go to the tech room menu
#define GS_EVENT_LOAD_MISSION_MENU				12		// go to the load mission menu
#define GS_EVENT_SHIP_SELECTION					13		// Show ship selection menu
#define GS_EVENT_TOGGLE_FULLSCREEN				14		//	toggle fullscreen mode
#define GS_EVENT_START_BRIEFING					15		// go to the briefing for the current mission
#define GS_EVENT_DEBUG_PAUSE_GAME				16
#define GS_EVENT_HUD_CONFIG						17		// start the HUD configuration screen
#define GS_EVENT_MULTI_JOIN_GAME				18		// start multiplayer join game screen
#define GS_EVENT_CONTROL_CONFIG					19		// get user to choose what type of controller to config
#define GS_EVENT_EVENT_DEBUG					20		 // an event debug trace scroll list display screen
#define GS_EVENT_WEAPON_SELECTION				21		// Do weapon loadout 
#define GS_EVENT_MISSION_LOG_SCROLLBACK			22		// scrollback screen for message log entries
#define GS_EVENT_GAMEPLAY_HELP					23		// show help for the gameplay
#define GS_EVENT_DEATH_DIED						24		//	Player just died
#define GS_EVENT_DEATH_BLEW_UP					25		//	Saw ship explode.
#define GS_EVENT_NEW_CAMPAIGN					26
#define GS_EVENT_CREDITS						27		// Got to the credits
#define GS_EVENT_SHOW_GOALS						28		// Show the goal status screen
#define GS_EVENT_HOTKEY_SCREEN					29		// Show the hotkey assignment screen
#define GS_EVENT_VIEW_MEDALS					30		// Go to the View Medals screen
#define GS_EVENT_MULTI_HOST_SETUP				31		// host setup for multiplayer
#define GS_EVENT_MULTI_CLIENT_SETUP				32		// client setup for multiplayer
#define GS_EVENT_DEBRIEF						33		// go to debriefing
#define GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN  	34    // go to the demo management screen
#define GS_EVENT_MULTI_STD_WAIT					35    // standalone wait state
#define GS_EVENT_STANDALONE_MAIN				36    // the main do-nothing state of the standalone
#define GS_EVENT_MULTI_PAUSE				    37    // pause your multiplayer game
#define GS_EVENT_TEAM_SELECT					38		// team selection for multiplayer
#define GS_EVENT_TRAINING_PAUSE					39		// pause game while training message is displayed
#define GS_EVENT_INGAME_PRE_JOIN				40    // go to ship selection screen for ingame join
#define GS_EVENT_PLAYER_WARPOUT_START			41		// player hit 'j' to warp out
#define GS_EVENT_PLAYER_WARPOUT_START_FORCED	42		// player is being forced out of mission no matter what
#define GS_EVENT_PLAYER_WARPOUT_STOP			43		// player hit 'esc' or something to cancel warp out
#define GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1		44		// player ship got up to speed
#define GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2		45		// player ship got through the warp effect
#define GS_EVENT_PLAYER_WARPOUT_DONE			46		// warp effect went away
#define GS_EVENT_STANDALONE_POSTGAME			47	   // debriefing, etc
#define GS_EVENT_INITIAL_PLAYER_SELECT			48		// initial screen where player selects from multi/single player pilots
#define GS_EVENT_GAME_INIT						49
#define GS_EVENT_MULTI_MISSION_SYNC				50    // sychronize/transfer/load any mission specific data in multiplayer
#define GS_EVENT_MULTI_START_GAME				51		// immediately before the create game screen for the host to set the game variables
#define GS_EVENT_MULTI_HOST_OPTIONS				52		// options the host can set while in the create game scree
#define GS_EVENT_MULTI_DOGFIGHT_DEBRIEF			53		// multiplayer furball debriefing screen (replaces normal debriefing)
#define GS_EVENT_CAMPAIGN_ROOM					54
#define GS_EVENT_CMD_BRIEF						55		// switch to command briefing screen
#define GS_EVENT_TOGGLE_GLIDE					56		//	GS_EVENT_TOGGLE_GLIDE
#define GS_EVENT_RED_ALERT						57		// go to red alert screen
#define GS_EVENT_SIMULATOR_ROOM					58
#define GS_EVENT_END_CAMPAIGN					59		// end of the whole thang.
#define GS_EVENT_END_DEMO						60		// end of demo campaign
#define GS_EVENT_LOOP_BRIEF						61		// campaign loop brief
#define GS_EVENT_CAMPAIGN_CHEAT					62		// skip to a mission in a campaign
#define GS_EVENT_NET_CHAT						63		// #Kazan# - Go to net chat for Fs2NetD
#define GS_EVENT_LAB							64		// WMC - I-FRED concept
#define GS_EVENT_STORYBOOK						65		// WMC - the storybook

// IMPORTANT:  When you add a new event, update the initialization for GS_event_text[]
//             which is done in GameSequence.cpp
//
extern char *GS_event_text[];		// text description for the GS_EVENT_* #defines above


// defines for game sequencing states
//
// IMPORTANT:  When you add a new state, update the initialization for GS_state_text[]
//             which is done in GameSequence.cpp
#define GS_STATE_MAIN_MENU							1
#define GS_STATE_GAME_PLAY							2
#define GS_STATE_GAME_PAUSED						3
#define GS_STATE_QUIT_GAME							4
#define GS_STATE_OPTIONS_MENU						5
#define GS_STATE_BARRACKS_MENU					7
#define GS_STATE_TECH_MENU							8
#define GS_STATE_TRAINING_MENU					9
#define GS_STATE_LOAD_MISSION_MENU				10
#define GS_STATE_BRIEFING							11
#define GS_STATE_SHIP_SELECT						12
#define GS_STATE_DEBUG_PAUSED						13
#define GS_STATE_HUD_CONFIG						14
#define GS_STATE_MULTI_JOIN_GAME					15
#define GS_STATE_CONTROL_CONFIG					16
#define GS_STATE_WEAPON_SELECT					17
#define GS_STATE_MISSION_LOG_SCROLLBACK		18
#define GS_STATE_DEATH_DIED						19		//	Player just died
#define GS_STATE_DEATH_BLEW_UP					20		//	Saw ship explode.
#define GS_STATE_SIMULATOR_ROOM					21
#define GS_STATE_CREDITS							22
#define GS_STATE_SHOW_GOALS						23
#define GS_STATE_HOTKEY_SCREEN					24
#define GS_STATE_VIEW_MEDALS						25		// Go to the View Medals screen
#define GS_STATE_MULTI_HOST_SETUP				26		// state where host sets up multiplayer game
#define GS_STATE_MULTI_CLIENT_SETUP				27		// client setup for multiplayer game
#define GS_STATE_DEBRIEF							28
#define GS_STATE_VIEW_CUTSCENES		         29
#define GS_STATE_MULTI_STD_WAIT					30
#define GS_STATE_STANDALONE_MAIN				   31
#define GS_STATE_MULTI_PAUSED				      32
#define GS_STATE_TEAM_SELECT						33
#define GS_STATE_TRAINING_PAUSED					34		 // game is paused while training msg is being read.
#define GS_STATE_INGAME_PRE_JOIN				   35		 // go to ship selection screen for ingame join
#define GS_STATE_EVENT_DEBUG						36		 // an event debug trace scroll list display screen
#define GS_STATE_STANDALONE_POSTGAME			37		 // debriefing, etc.
#define GS_STATE_INITIAL_PLAYER_SELECT			38
#define GS_STATE_MULTI_MISSION_SYNC				39
#define GS_STATE_MULTI_START_GAME				40
#define GS_STATE_MULTI_HOST_OPTIONS				41
#define GS_STATE_MULTI_DOGFIGHT_DEBRIEF		42
#define GS_STATE_CAMPAIGN_ROOM					43
#define GS_STATE_CMD_BRIEF							44		// command briefing screen
#define GS_STATE_RED_ALERT							45		// red alert screen
#define GS_STATE_END_OF_CAMPAIGN					46		// end of main campaign -- only applicable in single player
#define GS_STATE_GAMEPLAY_HELP					47
#define GS_STATE_END_DEMO							48		// end of demo campaign (upsell then main menu)
#define GS_STATE_LOOP_BRIEF						49
#define GS_STATE_NET_CHAT						50			// #Kazan# - state for the pxo chat readded by penguin
#define GS_STATE_LAB							51
#define GS_STATE_STORYBOOK						52

#define GS_NUM_STATES							53			//Last one++

// IMPORTANT:  When you add a new state, update the initialization for GS_state_text[]
//             which is done in GameSequence.cpp
//
extern struct script_hook GS_state_hooks[];	//WMC-for scripting
extern char *GS_state_text[];		// text description for the GS_STATE_* #defines above
extern int Num_gs_event_text;
extern int Num_gs_state_text;		//WMC - for scripting


// function prototypes
//
void gameseq_init();
int gameseq_process_events( void );		// returns current game state
int gameseq_get_state( int depth = 0 );
void gameseq_post_event( int event );
int gameseq_get_event( void );

void gameseq_set_state(int new_state, int override = 0);
void gameseq_push_state( int new_state );
void gameseq_pop_state( void );
int gameseq_get_pushed_state();
int gameseq_get_depth();
void gameseq_pop_and_discard_state(void);


// Called by the sequencing code when things happen.
void game_process_event(int current_state, int event);
void game_leave_state(int old_state,int new_state);
void game_enter_state(int old_state,int new_state);
void game_do_state(int current_state);

// Kazan
bool GameState_Stack_Valid();

//WMC
int gameseq_get_state_idx(char *s);

#endif /* __GAMESEQUENCE_H__ */

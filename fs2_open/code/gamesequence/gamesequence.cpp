/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GameSequence/GameSequence.cpp $
 * $Revision: 2.7 $
 * $Date: 2005-03-25 06:57:33 $
 * $Author: wmcoolmon $
 *
 * File to control Game Sequencing
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.5  2004/07/26 20:47:30  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:32:47  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.3  2004/05/03 21:22:20  Kazan
 * Abandon strdup() usage for mod list processing - it was acting odd and causing crashing on free()
 * Fix condition where alt_tab_pause() would flipout and trigger failed assert if game minimizes during startup (like it does a lot during debug)
 * Nav Point / Auto Pilot code (All disabled via #ifdefs)
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
 * 3     2/23/99 2:29p Dave
 * First run of oldschool dogfight mode. 
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 65    5/15/98 12:09a Dave
 * New tracker api code. New game tracker code. Finished up first run of
 * the PXO screen. Fixed a few game server list exceptions.
 * 
 * 64    5/12/98 2:46a Dave
 * Rudimentary communication between Parallax Online and freespace. Can
 * get and store channel lists.
 * 
 * 63    5/06/98 1:12a Allender
 * fix sequencing names, added nprintf to help respawn debugging
 * 
 * 62    4/25/98 7:39p Allender
 * fixd some small hotkey stuff.  Worked on turret orientation being
 * correct for multiplayer.  new sexpression called end-campaign will will
 * end the main campaign
 * 
 * 61    4/23/98 7:08p John
 * Removed some obsoleted states.
 * 
 * 60    4/16/98 4:31p Hoffoss
 * Changed demo screen referenced to view cutscenes screen, which is now
 * what it's called.
 * 
 * 59    4/02/98 5:40p Hoffoss
 * Added the Load Mission screen to FreeSpace.
 * 
 * 58    3/11/98 5:32p Lawrance
 * Fix up text arrays for events/states
 * 
 * 57    3/09/98 12:13a Lawrance
 * Add support for Red Alert missions
 * 
 * 56    3/05/98 4:12p John
 * Made Debug+F4 switch Glide and windowed.
 * 
 * 55    3/03/98 1:00p Hoffoss
 * Added new command briefing event and state.
 * 
 * 54    3/02/98 4:23p Hoffoss
 * Forgot to add state label.
 * 
 * 53    3/02/98 3:44p Hoffoss
 * Added new Campaign Room state and event.
 * 
 * 52    2/21/98 11:58a John
 * Put in some stuff to externalize strings
 * 
 * 51    2/19/98 6:26p Dave
 * Fixed a few file xfer bugs. Tweaked mp team select screen. Put in
 * initial support for player data uploading.
 * 
 * 50    2/18/98 10:21p Dave
 * Ripped out old file xfer system. Put in brand new xfer system.
 * 
 * 49    2/08/98 5:07p Dave
 * Put in support for multiplayer furball mode.
 * 
 * 48    1/28/98 6:21p Dave
 * Made the standalone use ~8 megs less memory. Fixed multiplayer submenu
 * endgame problem.
 * 
 * 47    1/23/98 5:43p Dave
 * Finished bringing standalone up to speed. Coded in new host options
 * screen.
 * 
 * 46    1/22/98 5:25p Dave
 * Modified some pregame sequencing packets. Starting to repair broken
 * standalone stuff.
 * 
 * 45    1/22/98 4:15p Hoffoss
 * Added code to allow popup to tell player he needs to bind a key for the
 * training mission.
 * 
 * 44    1/20/98 5:42p Dave
 * Moved ingame join to its own module. Improved it a bit.
 * 
 * 43    1/19/98 12:57p Allender
 * removed confusing comment
 * 
 * 42    1/19/98 12:56p Allender
 * fix problem of freespace_start_misison possibly failing due to mission
 * not properly loading (for single player only right now).
 * 
 * 41    1/15/98 6:12p Dave
 * Fixed weapons loadout bugs with multiplayer respawning. Added
 * multiplayer start screen. Fixed a few chatbox bugs.
 * 
 * 40    1/15/98 6:00p Hoffoss
 * Added option to quit menu (in game) to restart the mission.  Doesn't
 * seem to quite work, though.  Checking code in so someone else can look
 * into it.
 * 
 * 39    1/05/98 10:05a Dave
 * Big re-sequencing of server transfer. Centralized _all_ server transfer
 * code to one module.
 * 
 * 38    12/30/97 4:28p Lawrance
 * Give text descriptions for events, change debug output to give text
 * desciption of event/state
 * 
 * 37    12/24/97 8:56p Lawrance
 * took out obsolete state used for non-existant sound config screen
 * 
 * 36    11/19/97 8:28p Dave
 * Hooked in Main Hall screen. Put in Anim support for ping ponging
 * animations as well as general reversal of anim direction.
 * 
 * 35    11/15/97 2:36p Dave
 * Added more multiplayer campaign support.
 * 
 * 34    11/13/97 7:01p Hoffoss
 * Fixed GS_state_text[], which didn't match the current states we have
 * available.
 * 
 * 33    11/10/97 6:02p Hoffoss
 * Added new debug paused state.
 * 
 * 32    11/03/97 10:12p Hoffoss
 * Finished up work on the hud message/mission log scrollback screen.
 * 
 * 31    10/22/97 11:00p Lawrance
 * modify pop_and_discard() to allow discarding of all states on the stack
 * 
 * 30    10/22/97 5:08p John
 * fixed a whole slew of bugs and clean up a bunch of stuff dealing with
 * end of mission stuff.
 * 
 * 29    10/02/97 9:49p Hoffoss
 * Added event evaluation analysis debug screen so we can determine the
 * state of events and their sexp trees to track down logic problems and
 * such.
 * 
 * 28    9/23/97 11:53p Lawrance
 * add state do perform multiplayer on-line help
 * 
 * 27    9/22/97 4:55p Hoffoss
 * Added a training message window display thingy.
 * 
 * 26    9/19/97 4:24p Allender
 * added team selection state -- initialze player* variable in
 * player_level_init
 * 
 * 25    9/18/97 10:17p Lawrance
 * add help state for briefing
 * 
 * 24    9/18/97 10:15p Lawrance
 * add help state for briefing
 * 
 * 23    7/14/97 12:03a Lawrance
 * added navmap state
 * 
 * 22    6/13/97 2:30p Lawrance
 * Added debriefings
 * 
 * 21    5/20/97 10:02a Lawrance
 * added view medals screen
 * 
 * 20    4/28/97 2:17p Lawrance
 * added help state for hotkey assignment screen
 * 
 * 19    4/25/97 3:41p Lawrance
 * added support for hotkey assignment screen
 * 
 * 18    4/23/97 9:54a Lawrance
 * made show goals screen a separate state
 * 
 * 17    4/22/97 11:06a Lawrance
 * added credits state
 * 
 * 16    4/17/97 9:01p Allender
 * start of campaign stuff.  Campaigns now stored in external file (no
 * filenames in code).  Continuing campaign won't work at this point
 * 
 * 15    4/03/97 8:40p Lawrance
 * add new player death states to GS_state_text[]
 * 
 * 14    3/05/97 5:04p Lawrance
 * added new states for different context help
 * 
 * 13    1/09/97 12:41a Lawrance
 * added function to pop a state without restoring that state
 * 
 * 12    12/22/96 3:41p Lawrance
 * integrating energy transfer system
 * 
 * 11    12/09/96 2:35p Allender
 * modifed game sequencing so that game_leave_state and game_enter_state
 * are *always* called.
 * 
 * 10    12/08/96 1:54a Lawrance
 * put check in to see if a state change request is invalid (ie already in
 * that state)
 * 
 * 9     11/18/96 5:07p John
 * Changed sequencing code to call entry,leave functions for each state
 * change.   Added Shift+Pause debug pause thing.
 * 
 * 8     11/13/96 4:02p Lawrance
 * complete over-haul of the menu system and the states associated with
 * them
 * 
 * 7     10/23/96 9:08a Allender
 * Removed primary and secondary goal complete states -- to be implemented
 * later.
 *
*/

/*
 *  All states for game sequencing are defined in GameSequence.h.
 *  States should always be referred to using the macros.
*/

#include "gamesequence/gamesequence.h"
#include "globalincs/pstypes.h"



// local defines
#define MAX_GAMESEQ_EVENTS		20		// maximum number of events on the game sequencing queue
#define GS_STACK_SIZE			10		// maximum number of stacked states

// local variables
typedef struct state_stack {
	int	current_state;
	int	event_queue[MAX_GAMESEQ_EVENTS];
	int	queue_tail, queue_head;
} state_stack;

// DO NOT MAKE THIS NON-STATIC!!!!
LOCAL state_stack gs[GS_STACK_SIZE];
LOCAL int gs_current_stack = -1;						// index of top state on stack.

static int state_reentry = 0;  // set if we are already in state processing
static int state_processing_event_post = 0;  // set if we are already processing an event to switch states
static int state_in_event_processer = 0;

// Text of state, corresponding to #define values for GS_STATE_*
//XSTR:OFF
char *GS_event_text[] =
{
	"GS_EVENT_MAIN_MENU",
	"GS_EVENT_START_GAME",
	"GS_EVENT_ENTER_GAME",
	"GS_EVENT_START_GAME_QUICK",
	"GS_EVENT_END_GAME",									
	"GS_EVENT_QUIT_GAME",								// 5
	"GS_EVENT_PAUSE_GAME",
	"GS_EVENT_PREVIOUS_STATE",
	"GS_EVENT_OPTIONS_MENU",
	"GS_EVENT_BARRACKS_MENU",							
	"GS_EVENT_TRAINING_MENU",							// 10
	"GS_EVENT_TECH_MENU",
	"GS_EVENT_LOAD_MISSION_MENU",
	"GS_EVENT_SHIP_SELECTION",
	"GS_EVENT_TOGGLE_FULLSCREEN",						
	"GS_EVENT_WEAPON_SELECT_HELP",					// 15
	"GS_EVENT_START_BRIEFING",
	"GS_EVENT_DEBUG_PAUSE_GAME",
	"GS_EVENT_HUD_CONFIG",
	"GS_EVENT_MULTI_SETUP",								
	"GS_EVENT_MULTI_JOIN_GAME",						// 20
	"GS_EVENT_CONTROL_CONFIG",
	"GS_EVENT_EVENT_DEBUG",
	"GS_EVENT_MULTI_PROTO_CHOICE",
	"GS_EVENT_SAVE_RESTORE",							
	"GS_EVENT_CHOOSE_SAVE_OR_RESTORE",				// 25
	"GS_EVENT_WEAPON_SELECTION",
	"GS_EVENT_MISSION_LOG_SCROLLBACK",
	"GS_EVENT_MAIN_MENU_HELP",							
	"GS_EVENT_GAMEPLAY_HELP",
	"GS_EVENT_SHIP_SELECT_HELP",						// 30
	"GS_EVENT_DEATH_DIED",
	"GS_EVENT_DEATH_BLEW_UP",
	"GS_EVENT_NEW_CAMPAIGN",
	"GS_EVENT_CREDITS",
	"GS_EVENT_SHOW_GOALS",								// 35
	"GS_EVENT_HOTKEY_SCREEN",
	"GS_EVENT_HOTKEY_SCREEN_HELP",					
	"GS_EVENT_VIEW_MEDALS",
	"GS_EVENT_MULTI_HOST_SETUP",
	"GS_EVENT_MULTI_CLIENT_SETUP",					// 40
	"GS_EVENT_DEBRIEF",
	"GS_EVENT_NAVMAP",									
	"GS_EVENT_MULTI_JOIN_TRACKER",
	"GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN",
	"GS_EVENT_MULTI_STD_WAIT",							// 45
	"GS_EVENT_STANDALONE_MAIN",
	"GS_EVENT_MULTI_PAUSE",
	"GS_EVENT_BRIEFING_HELP",
	"GS_EVENT_TEAM_SELECT",
	"GS_EVENT_TRAINING_PAUSE",							// 50	
	"GS_EVENT_MULTI_HELP",								
	"GS_EVENT_INGAME_PRE_JOIN",
	"GS_EVENT_PLAYER_WARPOUT_START",
	"GS_EVENT_PLAYER_WARPOUT_START_FORCED",
	"GS_EVENT_PLAYER_WARPOUT_STOP",					// 55
	"GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1",			
	"GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2",
	"GS_EVENT_PLAYER_WARPOUT_DONE",
	"GS_EVENT_STANDALONE_POSTGAME",
	"GS_EVENT_INITIAL_PLAYER_SELECT",				// 60
	"GS_EVENT_GAME_INIT",								
	"GS_EVENT_MULTI_MISSION_SYNC",
	"GS_EVENT_MULTI_CAMPAIGN_SELECT",
	"GS_EVENT_MULTI_SERVER_TRANSFER",
	"GS_EVENT_MULTI_START_GAME",						// 65
	"GS_EVENT_MULTI_HOST_OPTIONS",					
	"GS_EVENT_MULTI_DOGFIGHT_DEBRIEF",
	"GS_EVENT_CAMPAIGN_ROOM",
	"GS_EVENT_CMD_BRIEF",
	"GS_EVENT_TOGGLE_GLIDE",							// 70
	"GS_EVENT_RED_ALERT",								
	"GS_EVENT_SIMULATOR_ROOM",
	"GS_EVENT_EMD_CAMPAIGN",	
	"GS_EVENT_LAB",
	"GS_EVENT_STORYBOOK",								//75
};
//XSTR:ON

// Text of state, corresponding to #define values for GS_STATE_*
//XSTR:OFF
char *GS_state_text[] =
{
	"NOT A VALID STATE",
	"GS_STATE_MAIN_MENU",								// 1
	"GS_STATE_GAME_PLAY",
	"GS_STATE_GAME_PAUSED",
	"GS_STATE_QUIT_GAME",
	"GS_STATE_OPTIONS_MENU",							// 5
	"GS_EVENT_WEAPON_SELECT_HELP",
	"GS_STATE_BARRACKS_MENU",
	"GS_STATE_TECH_MENU",
	"GS_STATE_TRAINING_MENU",
	"GS_STATE_LOAD_MISSION_MENU",						// 10
	"GS_STATE_BRIEFING",
	"GS_STATE_SHIP_SELECT",
	"GS_STATE_DEBUG_PAUSED",
	"GS_STATE_HUD_CONFIG",
	"GS_STATE_MULTI_SETUP",								// 15
	"GS_STATE_MULTI_JOIN_GAME",
	"GS_STATE_CONTROL_CONFIG",
	"GS_STATE_MULTI_PROTO_CHOICE",
	"GS_STATE_SAVE_RESTORE",
	"GS_STATE_WEAPON_SELECT",							// 20
	"GS_STATE_MISSION_LOG_SCROLLBACK",
	"GS_STATE_MAIN_MENU_HELP",
	"GS_STATE_GAMEPLAY_HELP",
	"GS_STATE_SHIP_SELECT_HELP",
	"GS_STATE_DEATH_DIED",								// 25
	"GS_STATE_DEATH_BLEW_UP",
	"GS_STATE_SIMULATOR_ROOM",
	"GS_STATE_CREDITS",
	"GS_STATE_SHOW_GOALS",
	"GS_STATE_HOTKEY_SCREEN",							// 30
	"GS_STATE_HOTKEY_SCREEN_HELP",
	"GS_STATE_VIEW_MEDALS",
	"GS_STATE_MULTI_HOST_SETUP",
	"GS_STATE_MULTI_CLIENT_SETUP",
	"GS_STATE_DEBRIEF",									// 35
	"GS_STATE_NAVMAP",
	"GS_STATE_MULTI_JOIN_TRACKER",  
	"GS_STATE_VIEW_CUTSCENES",
	"GS_STATE_MULTI_STD_WAIT",
	"GS_STATE_STANDALONE_MAIN",						// 40
	"GS_STATE_MULTI_PAUSED",  	
	"GS_STATE_BRIEFING_HELP",
	"GS_STATE_TEAM_SELECT",
	"GS_STATE_TRAINING_PAUSED",
	"GS_STATE_MULTI_HELP",  							// 45
	"GS_STATE_INGAME_PRE_JOIN",						
	"GS_STATE_EVENT_DEBUG",
	"GS_STATE_STANDALONE_POSTGAME",
	"GS_STATE_INITIAL_PLAYER_SELECT",
	"GS_STATE_MULTI_MISSION_SYNC",					// 50
	"GS_STATE_MULTI_SERVER_TRANSFER",				
	"GS_STATE_MULTI_START_GAME",
	"GS_STATE_MULTI_HOST_OPTIONS",
	"GS_STATE_MULTI_DOGFIGHT_DEBRIEF",				
	"GS_STATE_CAMPAIGN_ROOM",							// 55
	"GS_STATE_CMD_BRIEF",
	"GS_STATE_RED_ALERT",
	"GS_STATE_END_OF_CAMPAIGN",
	"GS_STATE_FRED_CONCEPT",
	"GS_STATE_STORYBOOK",
};
//XSTR:ON

void gameseq_init()
{
	int i;

	for (i=0; i<GS_STACK_SIZE; i++ )	{
		// gs[i].current_state = GS_STATE_MAIN_MENU;
		gs[i].current_state = 0;
		gs[i].queue_tail=0;
		gs[i].queue_head=0;
	}

	gs_current_stack = 0;
	state_reentry = 0;
	state_processing_event_post = 0;
	state_in_event_processer = 0;
}

// gameseq_post_event posts a new game sequencing event onto the gameseq
// event queue

void gameseq_post_event( int event )
{
	if (state_processing_event_post) {
		nprintf(("Warning", "Received post for event %s during state transtition. Find Allender if you are unsure if this is bad.\n", GS_event_text[event] ));
	}

	Assert(gs[gs_current_stack].queue_tail < MAX_GAMESEQ_EVENTS);
	gs[gs_current_stack].event_queue[gs[gs_current_stack].queue_tail++] = event;
	if ( gs[gs_current_stack].queue_tail == MAX_GAMESEQ_EVENTS )
		gs[gs_current_stack].queue_tail = 0;
}

// returns one of the GS_EVENT_ id's on the game sequencing queue

int gameseq_get_event()
{
	int event;

	if ( gs[gs_current_stack].queue_head == gs[gs_current_stack].queue_tail )
		return -1;
	event = gs[gs_current_stack].event_queue[gs[gs_current_stack].queue_head++];
	if ( gs[gs_current_stack].queue_head == MAX_GAMESEQ_EVENTS )
		gs[gs_current_stack].queue_head = 0;

	return event;
}	   

// Is our state stack valid
bool GameState_Stack_Valid()
{
	return (gs_current_stack != -1);
}

// returns one of the GS_STATE_ macros
int gameseq_get_state(int depth)
{	
	Assert(depth <= gs_current_stack);
			
	return gs[gs_current_stack - depth].current_state;
}

int gameseq_get_depth()
{
	return gs_current_stack;
}

void gameseq_set_state(int new_state, int override)
{
	int event, old_state;

	if ( (new_state == gs[gs_current_stack].current_state) && !override )
		return;

	old_state = gs[gs_current_stack].current_state;

	// Flush all events!!
	while ( (event = gameseq_get_event()) != -1 ) {
		mprintf(( "Throwing out event %d because of state set from %d to %d\n", event, old_state, new_state ));
	}

	Assert( state_reentry == 1 );		// Get John! (Invalid state sequencing!)
	Assert( state_in_event_processer == 1 );		// can only call from game_process_event

	state_processing_event_post++;
	state_reentry++;
	game_leave_state(gs[gs_current_stack].current_state,new_state);

	gs[gs_current_stack].current_state = new_state;

	game_enter_state(old_state,gs[gs_current_stack].current_state);
	state_reentry--;
	state_processing_event_post--;
}
	
void gameseq_push_state( int new_state )
{
	if ( new_state == gs[gs_current_stack].current_state )
		return;

	int old_state = gs[gs_current_stack].current_state;

	// Flush all events!!
// I commented out because I'm not sure if we should throw out events when pushing or not.
//	int event;
//	while( (event = gameseq_get_event()) != -1 )	{
//		mprintf(( "Throwing out event %d because of state push from %d to %d\n", event, old_state, new_state ));
//	}

	Assert( state_reentry == 1 );		// Get John! (Invalid state sequencing!)
	Assert( state_in_event_processer == 1 );		// can only call from game_process_event

	gs_current_stack++;
	Assert(gs_current_stack < GS_STACK_SIZE);

	state_processing_event_post++;
	state_reentry++;
	game_leave_state(old_state,new_state);

	gs[gs_current_stack].current_state = new_state;
	gs[gs_current_stack].queue_tail = 0;
	gs[gs_current_stack].queue_head = 0;

	game_enter_state(old_state,gs[gs_current_stack].current_state);
	state_reentry--;
	state_processing_event_post--;
}

void gameseq_pop_state()
{
	int popped_state = 0;

	Assert(state_reentry == 1);		// Get John! (Invalid state sequencing!)

	if (gs_current_stack >= 1) {
		int old_state;

		// set the old state to be the state which is about to be popped off the queue
		old_state = gs[gs_current_stack].current_state;

		// set the popped_state to be the state which is going to be moved into
		popped_state = gs[gs_current_stack-1].current_state;

		// leave the current state
		state_reentry++;
		game_leave_state(gs[gs_current_stack].current_state,popped_state);

		// set the popped_state to be the one we moved into
		gs_current_stack--;
		popped_state = gs[gs_current_stack].current_state;

		// swap all remaining events from the state which just got popped to this new state
		while(gs[gs_current_stack+1].queue_head != gs[gs_current_stack+1].queue_tail){
			gameseq_post_event(gs[gs_current_stack+1].event_queue[gs[gs_current_stack+1].queue_head++]);
		}

		game_enter_state(old_state, gs[gs_current_stack].current_state);
		state_reentry--;

	}

}

// gameseq_pop_and_discard_state() is used to remove a state that was pushed onto the stack, but
// will never need to be popped.  An example of this is entering a state that may require returning
// to the previous state (then you would call gameseq_pop_state).  Or you may simply continue to
// another part of the game, to avoid filling up the stack with states that may never be popped, you
// call this function to discard the top of the gs.
//

void gameseq_pop_and_discard_state()
{
	if (gs_current_stack > 0 ) {
		gs_current_stack--;
	}
}

// Returns the last state pushed on stack
int gameseq_get_pushed_state()
{
	if (gs_current_stack >= 1) {
		return gs[gs_current_stack-1].current_state;
	} else	
		return -1;
}

// gameseq_process_events gets called every time through high level loops
// (i.e. game loops, main menu loop).  Function is responsible for pulling
// game sequence events off the queue and changing the state when necessary.
// Returns the current state.
		// pull events game sequence events off of the queue.  Process one at a time
		// based on the current state and the new event.

int gameseq_process_events()	
{
	int event, old_state;
	old_state = gs[gs_current_stack].current_state;

	Assert(state_reentry == 0);		// Get John! (Invalid state sequencing!)

	while ( (event = gameseq_get_event()) != -1 ) {
		state_reentry++;
		state_in_event_processer++;
		game_process_event(gs[gs_current_stack].current_state, event);
		state_in_event_processer--;
		state_reentry--;
		// break when state changes so that code will get called at
		// least one frame for each state.
		if (old_state != gs[gs_current_stack].current_state)
			break;	
	}

	state_reentry++;
	game_do_state(gs[gs_current_stack].current_state);
	state_reentry--;

	return gs[gs_current_stack].current_state;
} 


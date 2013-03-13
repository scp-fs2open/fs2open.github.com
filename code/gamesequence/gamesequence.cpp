/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



/*
 *  All states for game sequencing are defined in GameSequence.h.
 *  States should always be referred to using the macros.
*/

#include "gamesequence/gamesequence.h"
#include "globalincs/pstypes.h"
#include "parse/scripting.h"



// local defines
#define MAX_GAMESEQ_EVENTS		20		// maximum number of events on the game sequencing queue
#define GS_STACK_SIZE			10		// maximum number of stacked states

// local variables
typedef struct state_stack {
	int	current_state;
	int previous_state;
	int	event_queue[MAX_GAMESEQ_EVENTS];
	int	queue_tail, queue_head;
} state_stack;

// DO NOT MAKE THIS NON-STATIC!!!!
LOCAL state_stack gs[GS_STACK_SIZE];
LOCAL int gs_current_stack = -1;						// index of top state on stack.

static int state_reentry = 0;  // set if we are already in state processing
static int state_processing_event_post = 0;  // set if we are already processing an event to switch states
static int state_in_event_processer = 0;

script_hook GS_state_hooks[GS_NUM_STATES];

// Text of state, corresponding to #define values for GS_STATE_*
//XSTR:OFF
char *GS_event_text[] =
{
	"GS_EVENT_MAIN_MENU",							// 0
	"GS_EVENT_START_GAME",
	"GS_EVENT_ENTER_GAME",
	"GS_EVENT_START_GAME_QUICK",
	"GS_EVENT_END_GAME",
	"GS_EVENT_QUIT_GAME",							// 5
	"GS_EVENT_PAUSE_GAME",
	"GS_EVENT_PREVIOUS_STATE",
	"GS_EVENT_OPTIONS_MENU",
	"GS_EVENT_BARRACKS_MENU",
	"GS_EVENT_TRAINING_MENU",						// 10
	"GS_EVENT_TECH_MENU",
	"GS_EVENT_LOAD_MISSION_MENU",
	"GS_EVENT_SHIP_SELECTION",
	"GS_EVENT_TOGGLE_FULLSCREEN",
	"GS_EVENT_START_BRIEFING",						// 15
	"GS_EVENT_DEBUG_PAUSE_GAME",
	"GS_EVENT_HUD_CONFIG",
	"GS_EVENT_MULTI_JOIN_GAME",
	"GS_EVENT_CONTROL_CONFIG",
	"GS_EVENT_EVENT_DEBUG",							// 20
	"GS_EVENT_WEAPON_SELECTION",
	"GS_EVENT_MISSION_LOG_SCROLLBACK",
	"GS_EVENT_GAMEPLAY_HELP",
	"GS_EVENT_DEATH_DIED",
	"GS_EVENT_DEATH_BLEW_UP",						// 25
	"GS_EVENT_NEW_CAMPAIGN",
	"GS_EVENT_CREDITS",
	"GS_EVENT_SHOW_GOALS",
	"GS_EVENT_HOTKEY_SCREEN",
	"GS_EVENT_VIEW_MEDALS",							// 30
	"GS_EVENT_MULTI_HOST_SETUP",
	"GS_EVENT_MULTI_CLIENT_SETUP",
	"GS_EVENT_DEBRIEF",
	"GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN",
	"GS_EVENT_MULTI_STD_WAIT",						// 35
	"GS_EVENT_STANDALONE_MAIN",
	"GS_EVENT_MULTI_PAUSE",
	"GS_EVENT_TEAM_SELECT",
	"GS_EVENT_TRAINING_PAUSE",
	"GS_EVENT_INGAME_PRE_JOIN",						// 40
	"GS_EVENT_PLAYER_WARPOUT_START",
	"GS_EVENT_PLAYER_WARPOUT_START_FORCED",
	"GS_EVENT_PLAYER_WARPOUT_STOP",
	"GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1",
	"GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2",			// 45
	"GS_EVENT_PLAYER_WARPOUT_DONE",
	"GS_EVENT_STANDALONE_POSTGAME",
	"GS_EVENT_INITIAL_PLAYER_SELECT",
	"GS_EVENT_GAME_INIT",
	"GS_EVENT_MULTI_MISSION_SYNC",					// 50
	"GS_EVENT_MULTI_START_GAME",
	"GS_EVENT_MULTI_HOST_OPTIONS",
	"GS_EVENT_MULTI_DOGFIGHT_DEBRIEF",
	"GS_EVENT_CAMPAIGN_ROOM",
	"GS_EVENT_CMD_BRIEF",							// 55
	"GS_EVENT_TOGGLE_GLIDE",
	"GS_EVENT_RED_ALERT",
	"GS_EVENT_SIMULATOR_ROOM",
	"GS_EVENT_END_CAMPAIGN",
	"GS_EVENT_LOOP_BRIEF",
	"GS_EVENT_CAMPAIGN_CHEAT",
	"GS_EVENT_PXO",
	"GS_EVENT_LAB",
	"GS_EVENT_PXO_HELP",							// 65
	"GS_EVENT_FICTION_VIEWER",
	"GS_EVENT_SCRIPTING"
};
//XSTR:ON

int Num_gs_event_text = sizeof(GS_event_text)/sizeof(char*);

// Text of state, corresponding to #define values for GS_STATE_*
//XSTR:OFF
char *GS_state_text[] =
{
	"NOT A VALID STATE",							// 0
	"GS_STATE_MAIN_MENU",
	"GS_STATE_GAME_PLAY",
	"GS_STATE_GAME_PAUSED",
	"GS_STATE_QUIT_GAME",
	"GS_STATE_OPTIONS_MENU",						// 5
	"GS_STATE_BARRACKS_MENU",
	"GS_STATE_TECH_MENU",
	"GS_STATE_TRAINING_MENU",
	"GS_STATE_LOAD_MISSION_MENU",
	"GS_STATE_BRIEFING",							// 10
	"GS_STATE_SHIP_SELECT",
	"GS_STATE_DEBUG_PAUSED",
	"GS_STATE_HUD_CONFIG",
	"GS_STATE_MULTI_JOIN_GAME",
	"GS_STATE_CONTROL_CONFIG",						// 15
	"GS_STATE_WEAPON_SELECT",
	"GS_STATE_MISSION_LOG_SCROLLBACK",
	"GS_STATE_DEATH_DIED",
	"GS_STATE_DEATH_BLEW_UP",
	"GS_STATE_SIMULATOR_ROOM",						// 20
	"GS_STATE_CREDITS",
	"GS_STATE_SHOW_GOALS",
	"GS_STATE_HOTKEY_SCREEN",
	"GS_STATE_VIEW_MEDALS",
	"GS_STATE_MULTI_HOST_SETUP",					// 25
	"GS_STATE_MULTI_CLIENT_SETUP",
	"GS_STATE_DEBRIEF",
	"GS_STATE_VIEW_CUTSCENES",
	"GS_STATE_MULTI_STD_WAIT",
	"GS_STATE_STANDALONE_MAIN",						// 30
	"GS_STATE_MULTI_PAUSED",
	"GS_STATE_TEAM_SELECT",
	"GS_STATE_TRAINING_PAUSED",
	"GS_STATE_INGAME_PRE_JOIN",
	"GS_STATE_EVENT_DEBUG",							// 35
	"GS_STATE_STANDALONE_POSTGAME",
	"GS_STATE_INITIAL_PLAYER_SELECT",
	"GS_STATE_MULTI_MISSION_SYNC",
	"GS_STATE_MULTI_START_GAME",
	"GS_STATE_MULTI_HOST_OPTIONS",					// 40
	"GS_STATE_MULTI_DOGFIGHT_DEBRIEF",
	"GS_STATE_CAMPAIGN_ROOM",
	"GS_STATE_CMD_BRIEF",
	"GS_STATE_RED_ALERT",
	"GS_STATE_END_OF_CAMPAIGN",						// 45
	"GS_STATE_GAMEPLAY_HELP",
	"GS_STATE_LOOP_BRIEF",
	"GS_STATE_PXO",
	"GS_STATE_LAB",									// 50
	"GS_STATE_PXO_HELP",
	"GS_STATE_START_GAME",
	"GS_STATE_FICTION_VIEWER",
	"GS_STATE_SCRIPTING"
};
//XSTR:ON

int Num_gs_state_text = sizeof(GS_state_text)/sizeof(char*);

void gameseq_init()
{
	int i;

	for (i=0; i<GS_STACK_SIZE; i++ )	{
		// gs[i].current_state = GS_STATE_MAIN_MENU;
		gs[i].current_state = 0;
		gs[i].previous_state = 0;
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

int gameseq_get_previous_state()
{
	return gs[gs_current_stack].previous_state;
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
	gs[gs_current_stack].previous_state = old_state;

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
	gs[gs_current_stack].previous_state = old_state;
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
		gs[gs_current_stack].previous_state = old_state;

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

int gameseq_get_event_idx(char *s)
{
	for(int i = 0; i < Num_gs_event_text; i++)
	{
		if(!stricmp(s, GS_event_text[i])) {
			return i;
		}
	}

	return -1;
}

int gameseq_get_state_idx(char *s)
{
	for(int i = 0; i < Num_gs_state_text; i++)
	{
		if(!stricmp(s, GS_state_text[i])) {
			return i;
		}
	}

	return -1;
}

// If the given state exists in the stack then return the index, -1 if not
int gameseq_get_state_idx(int state)
{
	for(int i = 0; i <= gs_current_stack; i++)
	{
		if (gs[i].current_state == state) {
			return i;
		}
	}

	return -1;
}

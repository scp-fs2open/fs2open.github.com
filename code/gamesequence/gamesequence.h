/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




// defines for game sequencing

#ifndef __GAMESEQUENCE_H__
#define __GAMESEQUENCE_H__

/**
 *  @brief Enum's for Game Sequence Events.
 *  
 *  @details IMPORTANT: When you add a new event, update the initialization for GS_event_text[] which is done in
 *      gamesequence.cpp. Otherwise, the fs2_open.log string "Got event..." will not display properly.
 */
enum GS_EVENT {
	GS_EVENT_MAIN_MENU                  = 0,    // first event to move to first state
	GS_EVENT_START_GAME,                        // start a new game (Loads a mission then goes to briefing state)
	GS_EVENT_ENTER_GAME,                        // switches into game state, probably after mission briefing or ship selection.
	GS_EVENT_START_GAME_QUICK,                  // start a new game (Loads a mission then goes to directly to game state)
	GS_EVENT_END_GAME,                          // end the current game (i.e. back to main menu)
	GS_EVENT_QUIT_GAME,                         // quit the entire game
	GS_EVENT_PAUSE_GAME,                        // pause the current game
	GS_EVENT_PREVIOUS_STATE,                    // return to the previous state
	GS_EVENT_OPTIONS_MENU,                      // go to the options menu
	GS_EVENT_BARRACKS_MENU,                     // go to the barracks menu
	GS_EVENT_TRAINING_MENU,                     // go to the training menu
	GS_EVENT_TECH_MENU,                         // go to the tech room menu
	GS_EVENT_LOAD_MISSION_MENU,                 // go to the load mission menu
	GS_EVENT_SHIP_SELECTION,                    // Show ship selection menu
	GS_EVENT_TOGGLE_FULLSCREEN,                 // toggle fullscreen mode
	GS_EVENT_START_BRIEFING,                    // go to the briefing for the current mission
	GS_EVENT_DEBUG_PAUSE_GAME,
	GS_EVENT_HUD_CONFIG,                        // start the HUD configuration screen
	GS_EVENT_MULTI_JOIN_GAME,                   // start multiplayer join game screen
	GS_EVENT_CONTROL_CONFIG,                    // get user to choose what type of controller to config
	GS_EVENT_EVENT_DEBUG,                       // an event debug trace scroll list display screen
	GS_EVENT_WEAPON_SELECTION,                  // Do weapon loadout 
	GS_EVENT_MISSION_LOG_SCROLLBACK,            // scrollback screen for message log entries
	GS_EVENT_GAMEPLAY_HELP,                     // show help for the gameplay
	GS_EVENT_DEATH_DIED,                        // Player just died
	GS_EVENT_DEATH_BLEW_UP,                     // Saw ship explode.
	GS_EVENT_NEW_CAMPAIGN,
	GS_EVENT_CREDITS,                           // Got to the credits
	GS_EVENT_SHOW_GOALS,                        // Show the goal status screen
	GS_EVENT_HOTKEY_SCREEN,                     // Show the hotkey assignment screen
	GS_EVENT_VIEW_MEDALS,                       // Go to the View Medals screen
	GS_EVENT_MULTI_HOST_SETUP,                  // host setup for multiplayer
	GS_EVENT_MULTI_CLIENT_SETUP,                // client setup for multiplayer
	GS_EVENT_DEBRIEF,                           // go to debriefing
	GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN,        // go to the management screen
	GS_EVENT_MULTI_STD_WAIT,                    // standalone wait state
	GS_EVENT_STANDALONE_MAIN,                   // the main do-nothing state of the standalone
	GS_EVENT_MULTI_PAUSE,                       // pause your multiplayer game
	GS_EVENT_TEAM_SELECT,                       // team selection for multiplayer
	GS_EVENT_TRAINING_PAUSE,                    // pause game while training message is displayed
	GS_EVENT_INGAME_PRE_JOIN,                   // go to ship selection screen for ingame join
	GS_EVENT_PLAYER_WARPOUT_START,              // player hit 'j' to warp out
	GS_EVENT_PLAYER_WARPOUT_START_FORCED,       // player is being forced out of mission no matter what
	GS_EVENT_PLAYER_WARPOUT_STOP,               // player hit 'esc' or something to cancel warp out
	GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1,        // player ship got up to speed
	GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2,        // player ship got through the warp effect
	GS_EVENT_PLAYER_WARPOUT_DONE,               // warp effect went away
	GS_EVENT_STANDALONE_POSTGAME,               // debriefing, etc
	GS_EVENT_INITIAL_PLAYER_SELECT,             // initial screen where player selects from multi/single player pilots
	GS_EVENT_GAME_INIT,
	GS_EVENT_MULTI_MISSION_SYNC,                // sychronize/transfer/load any mission specific data in multiplayer
	GS_EVENT_MULTI_START_GAME,                  // immediately before the create game screen for the host to set the game variables
	GS_EVENT_MULTI_HOST_OPTIONS,                // options the host can set while in the create game scree
	GS_EVENT_MULTI_DOGFIGHT_DEBRIEF,            // multiplayer furball debriefing screen (replaces normal debriefing)
	GS_EVENT_CAMPAIGN_ROOM,
	GS_EVENT_CMD_BRIEF,                         // switch to command briefing screen
	GS_EVENT_TOGGLE_GLIDE,                      // GS_EVENT_TOGGLE_GLIDE
	GS_EVENT_RED_ALERT,                         // go to red alert screen
	GS_EVENT_SIMULATOR_ROOM,
	GS_EVENT_END_CAMPAIGN,                      // end of the whole thang.
	GS_EVENT_LOOP_BRIEF,                        // campaign loop brief
	GS_EVENT_CAMPAIGN_CHEAT,                    // skip to a mission in a campaign
	GS_EVENT_PXO,
	GS_EVENT_LAB,                               // WMC - I-FRED concept
	GS_EVENT_PXO_HELP,
	GS_EVENT_FICTION_VIEWER,
	GS_EVENT_SCRIPTING,

	GS_NUM_EVENTS    // Last one++
};
// IMPORTANT:  When you add a new event, update the initialization for GS_event_text[]
//             which is done in gamesequence.cpp
//
extern char *GS_event_text[];		// text description for the GS_EVENT_* #defines above


/**
 *  @brief Enum's for game sequencing states
 *  
 *  @details IMPORTANT: When you add a new state, you must update the initialization for GS_state_text[] in
 *    gamesequence.cpp. Otherwise, the fs2_open.log string "Got event..." will not display properly.
 */
enum GS_STATE {
	GS_STATE_INVALID           = 0,    // This state should never be reached
	GS_STATE_MAIN_MENU,
	GS_STATE_GAME_PLAY,
	GS_STATE_GAME_PAUSED,
	GS_STATE_QUIT_GAME,
	GS_STATE_OPTIONS_MENU,
	GS_STATE_BARRACKS_MENU,
	GS_STATE_TECH_MENU,
	GS_STATE_TRAINING_MENU,
	GS_STATE_LOAD_MISSION_MENU,
	GS_STATE_BRIEFING,
	GS_STATE_SHIP_SELECT,
	GS_STATE_DEBUG_PAUSED,
	GS_STATE_HUD_CONFIG,
	GS_STATE_MULTI_JOIN_GAME,
	GS_STATE_CONTROL_CONFIG,
	GS_STATE_WEAPON_SELECT,
	GS_STATE_MISSION_LOG_SCROLLBACK,
	GS_STATE_DEATH_DIED,               // Player just died
	GS_STATE_DEATH_BLEW_UP,            // Saw ship explode.
	GS_STATE_SIMULATOR_ROOM,
	GS_STATE_CREDITS,
	GS_STATE_SHOW_GOALS,
	GS_STATE_HOTKEY_SCREEN,
	GS_STATE_VIEW_MEDALS,              // Go to the View Medals screen
	GS_STATE_MULTI_HOST_SETUP,         // state where host sets up multiplayer game
	GS_STATE_MULTI_CLIENT_SETUP,       // client setup for multiplayer game
	GS_STATE_DEBRIEF,
	GS_STATE_VIEW_CUTSCENES,
	GS_STATE_MULTI_STD_WAIT,
	GS_STATE_STANDALONE_MAIN,
	GS_STATE_MULTI_PAUSED,
	GS_STATE_TEAM_SELECT,
	GS_STATE_TRAINING_PAUSED,          // game is paused while training msg is being read.
	GS_STATE_INGAME_PRE_JOIN,          // go to ship selection screen for ingame join
	GS_STATE_EVENT_DEBUG,              // an event debug trace scroll list display screen
	GS_STATE_STANDALONE_POSTGAME,      // debriefing, etc.
	GS_STATE_INITIAL_PLAYER_SELECT,
	GS_STATE_MULTI_MISSION_SYNC,
	GS_STATE_MULTI_START_GAME,
	GS_STATE_MULTI_HOST_OPTIONS,
	GS_STATE_MULTI_DOGFIGHT_DEBRIEF,
	GS_STATE_CAMPAIGN_ROOM,
	GS_STATE_CMD_BRIEF,                // command briefing screen
	GS_STATE_RED_ALERT,                // red alert screen
	GS_STATE_END_OF_CAMPAIGN,          // end of main campaign -- only applicable in single player
	GS_STATE_GAMEPLAY_HELP,
	GS_STATE_LOOP_BRIEF,
	GS_STATE_PXO,
	GS_STATE_LAB,
	GS_STATE_PXO_HELP,
	GS_STATE_START_GAME,
	GS_STATE_FICTION_VIEWER,
	GS_STATE_SCRIPTING,

	GS_NUM_STATES    // Last one++
};
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
int gameseq_get_previous_state();
void gameseq_pop_and_discard_state(void);


// Called by the sequencing code when things happen.
void game_process_event(int current_state, int event);
void game_leave_state(int old_state,int new_state);
void game_enter_state(int old_state,int new_state);
void game_do_state(int current_state);

// Kazan
bool GameState_Stack_Valid();

//WMC
int gameseq_get_event_idx(char *s);
int gameseq_get_state_idx(char *s);

//zookeeper
int gameseq_get_state_idx(int state);

#endif /* __GAMESEQUENCE_H__ */

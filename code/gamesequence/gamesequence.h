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
#define GS_EVENT_PXO							63
#define GS_EVENT_LAB							64		// WMC - I-FRED concept
#define GS_EVENT_PXO_HELP						65
#define GS_EVENT_FICTION_VIEWER					66
#define GS_EVENT_SCRIPTING						67

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
#define GS_STATE_BARRACKS_MENU						6
#define GS_STATE_TECH_MENU							7
#define GS_STATE_TRAINING_MENU						8
#define GS_STATE_LOAD_MISSION_MENU					9
#define GS_STATE_BRIEFING							10
#define GS_STATE_SHIP_SELECT						11
#define GS_STATE_DEBUG_PAUSED						12
#define GS_STATE_HUD_CONFIG							13
#define GS_STATE_MULTI_JOIN_GAME					14
#define GS_STATE_CONTROL_CONFIG						15
#define GS_STATE_WEAPON_SELECT						16
#define GS_STATE_MISSION_LOG_SCROLLBACK				17
#define GS_STATE_DEATH_DIED							18		//	Player just died
#define GS_STATE_DEATH_BLEW_UP						19		//	Saw ship explode.
#define GS_STATE_SIMULATOR_ROOM						20
#define GS_STATE_CREDITS							21
#define GS_STATE_SHOW_GOALS							22
#define GS_STATE_HOTKEY_SCREEN						23
#define GS_STATE_VIEW_MEDALS						24		// Go to the View Medals screen
#define GS_STATE_MULTI_HOST_SETUP					25		// state where host sets up multiplayer game
#define GS_STATE_MULTI_CLIENT_SETUP					26		// client setup for multiplayer game
#define GS_STATE_DEBRIEF							27
#define GS_STATE_VIEW_CUTSCENES						28
#define GS_STATE_MULTI_STD_WAIT						29
#define GS_STATE_STANDALONE_MAIN					30
#define GS_STATE_MULTI_PAUSED						31
#define GS_STATE_TEAM_SELECT						32
#define GS_STATE_TRAINING_PAUSED					33		 // game is paused while training msg is being read.
#define GS_STATE_INGAME_PRE_JOIN					34		 // go to ship selection screen for ingame join
#define GS_STATE_EVENT_DEBUG						35		 // an event debug trace scroll list display screen
#define GS_STATE_STANDALONE_POSTGAME				36		 // debriefing, etc.
#define GS_STATE_INITIAL_PLAYER_SELECT				37
#define GS_STATE_MULTI_MISSION_SYNC					38
#define GS_STATE_MULTI_START_GAME					39
#define GS_STATE_MULTI_HOST_OPTIONS					40
#define GS_STATE_MULTI_DOGFIGHT_DEBRIEF				41
#define GS_STATE_CAMPAIGN_ROOM						42
#define GS_STATE_CMD_BRIEF							43		// command briefing screen
#define GS_STATE_RED_ALERT							44		// red alert screen
#define GS_STATE_END_OF_CAMPAIGN					45		// end of main campaign -- only applicable in single player
#define GS_STATE_GAMEPLAY_HELP						46
#define GS_STATE_END_DEMO							47		// end of demo campaign (upsell then main menu)
#define GS_STATE_LOOP_BRIEF							48
#define GS_STATE_PXO								49
#define GS_STATE_LAB								50
#define GS_STATE_PXO_HELP							51
#define GS_STATE_START_GAME							52
#define GS_STATE_FICTION_VIEWER						53
#define GS_STATE_SCRIPTING							54

#define GS_NUM_STATES							55			//Last one++

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

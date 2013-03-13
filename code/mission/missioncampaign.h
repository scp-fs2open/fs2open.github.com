/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSION_CAMPAIGN_H
#define _MISSION_CAMPAIGN_H

#include "stats/scoring.h"

struct sexp_variable;

// name of the builtin campaign.
#define BUILTIN_CAMPAIGN		"FreeSpace2"

#define MAX_CAMPAIGN_MISSIONS	100			// maximum number of missions in a campaign

#define CAMPAIGN_ERROR_CORRUPT			-1
#define CAMPAIGN_ERROR_SEXP_EXHAUSTED	-2
#define CAMPAIGN_ERROR_MISSING			-3
#define CAMPAIGN_ERROR_SAVEFILE			-4
#define CAMPAIGN_ERROR_IGNORED			-5

// types of campaigns -- these defines match the string literals listed below which
// are found in the campaign files.  I don't think that we need campaigns for furball
// missions.
#define CAMPAIGN_TYPE_SINGLE			0
#define CAMPAIGN_TYPE_MULTI_COOP		1
#define CAMPAIGN_TYPE_MULTI_TEAMS		2

#define MAX_CAMPAIGN_TYPES				3

// type of movies we may be able to play
#define CAMPAIGN_MOVIE_PRE_MISSION		1
#define CMAPAIGN_MOVIE_POST_MISSION		2

#define CAMPAIGN_SINGLE_PLAYER_SIG     0xddddeeee
#define CAMPAIGN_MULTI_PLAYER_SIG      0xeeeeffff

// defines for possibly persistent information
#define CAMPAIGN_PERSISTENT_SHIP			1
#define CAMPAIGN_PERSISTENT_WEAPON		2

// Goober5000 - Bastion flag is not needed anymore; there can now be more than two main halls;
// but the flag is kept in order to maintain compatibility with older campaigns
#define CMISSION_FLAG_BASTION	(1<<0)	// set if stationed on Bastion, else Galatea

#define CMISSION_FLAG_SKIPPED	(1<<1)	// set if skipped, else not
#define CMISSION_FLAG_HAS_LOOP	(1<<2)	// mission loop, e.g. FS2 SOC loops
#define CMISSION_FLAG_HAS_FORK	(1<<3)	// campaign fork, e.g. Scroll or BWO (mutually exclusive with loop)

#define CAMPAIGN_LOOP_MISSION_UNINITIALIZED	-2

extern char *campaign_types[MAX_CAMPAIGN_TYPES];


// campaign flags - Goober5000
#define CF_DEFAULT_VALUE			0
#define CF_CUSTOM_TECH_DATABASE		(1 << 0)	// Goober5000

// structure for a campaign definition.  It contains the mission names and other interesting
// information about a campaign and the mission strucuture within.

typedef struct mgoal {
	char	name[NAME_LENGTH];		// name of the goal (same as name in the mission_goal structure
	char	status;						// failed, satisfied, or incomplete (same as goal completion);
} mgoal;

typedef struct mevent {
	char	name[NAME_LENGTH];
	char	status;
} mevent;

typedef struct cmission {
	char				*name;					// name of the mission
	char				*notes;					// mission notes for mission (used by Fred)
	char				briefing_cutscene[NAME_LENGTH];	// name of the cutscene to be played before this mission
	int				formula;					// sexpression used to determine mission branching.
	int				completed;				// has the player completed this mission
	int				num_goals;				// number of goals this mission had
	mgoal			*goals;					// malloced array of mgoals (of num_goals size) which has the goal completion status
	int				num_events;				// number of events this mission had
	mevent			*events;				// malloced array of mevents (of num_events size) which has event completion status
	int				num_variables;			// number of variables this mission had - Goober5000
	sexp_variable	*variables;				// malloced array of sexp_variables (of num_variables size) containing mission-persistent variables - Goober5000
	int				mission_loop_formula;	// formula to determine whether to allow a side loop
	char			*mission_branch_desc;	// message in popup
	char			*mission_branch_brief_anim;
	char			*mission_branch_brief_sound;
	int				level;					// what level of the tree it's on (Fred)
	int				pos;					// what x position on level it's on (Fred)
	int				flags;
	SCP_string		main_hall;				// which main hall the player is in - converted to SCP_string by CommanderDJ
	ubyte			debrief_persona_index;	// which persona is used for ranks/badges - Goober5000
	scoring_struct	stats;
} cmission;

typedef struct campaign {
	char	name[NAME_LENGTH];						// name of the campaign
	char	filename[MAX_FILENAME_LEN];				// filename the campaign info is in
	char	*desc;									// description of campaign
	int		type;									// type of campaign
	int		flags;									// flags - Goober5000
	int		num_missions;							// number of missions in the campaign
	int		num_missions_completed;					// number of missions in the campaign that have been flown
	int		current_mission;						// the current mission that the player is playing.  Only valid during the mission
	int		next_mission;							// number of the next mission to fly when comtinuing the campaign.  Always valid
	int		prev_mission;							// mission that we just came from.  Always valid
	int		loop_enabled;							// whether mission loop is chosen - true during a loop, false otherwise
	int		loop_mission;							// mission number of misssion loop (if any)
	int		loop_reentry;							// mission number to return to after loop is finished
	int		realign_required;						// are any missions missing alignment info? (Fred)
	int		num_players;							// valid in multiplayer campaigns -- number of players campaign supports.
	ubyte	ships_allowed[MAX_SHIP_CLASSES];		// which ships the player can use
	ubyte	weapons_allowed[MAX_WEAPON_TYPES];		// which weapons the player can use
	cmission	missions[MAX_CAMPAIGN_MISSIONS];	// decription of the missions
	int				num_variables;					// number of variables this campaign had - Goober5000
	sexp_variable	*variables;						// malloced array of sexp_variables (of num_variables size) containing campaign-persistent variables - Goober5000
} campaign;

extern campaign Campaign;

// campaign wasn't ended
extern int Campaign_ending_via_supernova;

// structure for players.  Holds the campaign name, number of missions flown in the campaign, and result
// of the missions flown.  This structure is stored in the player file and thus is persistent across
// games
typedef struct campaign_info
{
	int		num_missions_completed;
	char	filename[NAME_LENGTH];
	ubyte	missions_completed[MAX_CAMPAIGN_MISSIONS];
} campaign_info;

// extern'ed so the mission loading can get a list of campains.  Only use this
// data after mission_campaign_build_list() is called
#define MAX_CAMPAIGNS	128
extern char *Campaign_names[MAX_CAMPAIGNS];
extern char *Campaign_file_names[MAX_CAMPAIGNS];
extern char *Campaign_descs[MAX_CAMPAIGNS];
extern int	Num_campaigns;
extern int	Campaign_names_inited;
extern SCP_vector<SCP_string> Ignored_campaigns;

extern char Default_campaign_file_name[MAX_FILENAME_LEN - 4];

// if the campaign file is missing this will get set for us to check against
extern int Campaign_file_missing;

// called at game startup time to load the default single player campaign
void mission_campaign_init( void );

// called to reload the default campaign
int mission_campaign_load_by_name( char *filename );
int mission_campaign_load_by_name_csfe( char *filename, char *callsign );


// load up and initialize a new campaign
int mission_campaign_load( char *filename, player *pl = NULL, int load_savefile = 1 );

// function to save the state of the campaign between missions or to load a campaign save file
extern int mission_campaign_save( void );

// declaration for local campaign save game load function
extern void mission_campaign_savefile_delete( char *cfilename );
extern void mission_campaign_delete_all_savefiles( char *pilot_name );

// if a given campaign is a multiplayer campaign, we can load and save the multiplayer info portion with these functions
extern int mission_campaign_parse_is_multi(char *filename, char *name);

// function which sets up internal variable for player to play next mission in the campaign
extern int mission_campaign_next_mission( void );

// function which is called with the current mission in this campaign is over
extern void mission_campaign_mission_over( bool do_next_mission = true );

// frees all memory at game close time
extern void mission_campaign_clear( void );

// read in a campaign file.  Used by Fred.
int mission_campaign_load_fred(char *filename, char *name_verify = NULL);

// used by Fred to get a mission's list of goals.
void read_mission_goal_list(int num);

void mission_campaign_build_list(bool desc = false, bool sort = true, bool multiplayer = false);
void mission_campaign_free_list();

// returns index of mission with passed name
extern int mission_campaign_find_mission( char *name );

// maybe play a movie.  type indicates before or after mission
extern void mission_campaign_maybe_play_movie(int type);

// save persistent information
extern void mission_campaign_save_persistent( int type, int index );

void mission_campaign_savefile_generate_root(char *filename, player *pl = NULL);

int mission_campaign_savefile_save();

// The following are functions I added to set up the globals and then
// execute the corresponding mission_campaign_savefile functions.

// Saves the campaign camp under the player name pname
int campaign_savefile_save(char *pname);
// Deletes the campaign save camp under the player name pname
void campaign_delete_save( char *cfn, char *pname);
// Loads campaign camp from fname under player name pname
void campaign_savefile_load(char *fname, char *pname);

// get name and type of specified campaign file
int mission_campaign_get_info(char *filename, char *name, int *type, int *max_players, char **desc = NULL);

// get a listing of missions in a campaign
int mission_campaign_get_mission_list(char *filename, char **list, int max);

// load up a campaign for the current player.
int mission_load_up_campaign( player *p = NULL );

// stores mission goals and events in Campaign struct
void mission_campaign_store_goals_and_events_and_variables();

// evaluates next mission and possible loop mission
void mission_campaign_eval_next_mission();

// returns to the beginning of the previous mission
int mission_campaign_previous_mission();

// proceeds to next mission in campaign
void mission_campaign_skip_to_next(int start_game = 1);

// break out of loop
void mission_campaign_exit_loop();

// jump to specified mission
void mission_campaign_jump_to_mission(char *name);

// stuff for the end of the campaign of the single player game
void mission_campaign_end_init();
void mission_campaign_end_close();
void mission_campaign_end_do();

// Goober5000 - save persistent variables
extern void mission_campaign_save_player_persistent_variables();

extern void mission_campaign_load_failure_popup();

// End CSFE stuff
#endif

#ifndef __WXFRED_MISSION__
#define __WXFRED_MISSION__

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "globalincs/globals.h"
#include "graphics/2d.h"
#include "sound/sound.h"
#include "ai/ai_profiles.h"
#include "mission/missionparse.h"

//wxWidgets stuff

#include <wx/datetime.h>

//WMC - This should be here
#define FS_MISSION_FILE_EXT				NOX(".fs2")

struct wing;
struct p_dock_instance;

#define NUM_NEBULAS			3				// how many background nebulas we have altogether
#define NUM_NEBULA_COLORS	9

// arrival anchor types
// mask should be high enough to avoid conflicting with ship anchors
#define SPECIAL_ARRIVAL_ANCHOR_FLAG				0x1000
#define SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG		0x0100

// update version when mission file format changes, and add approprate code
// to check loaded mission version numbers in the parse code.  Also, be sure
// to update both MissionParse and MissionSave (FRED) when changing the
// mission file format!
#define	MISSION_VERSION 0.10f
#define	FRED_MISSION_VERSION 0.10f

#define WING_PLAYER_BASE	0x80000  // used by Fred to tell ship_index in a wing points to a player

// mission parse flags used for parse_mission() to tell what kind of information to get from the mission file
#define MPF_ONLY_MISSION_INFO	(1 << 0)
#define MPF_IMPORT_FSM			(1 << 1)

// bitfield definitions for missions game types
#define OLD_MAX_GAME_TYPES				4					// needed for compatibility
#define OLD_GAME_TYPE_SINGLE_ONLY	0
#define OLD_GAME_TYPE_MULTI_ONLY		1
#define OLD_GAME_TYPE_SINGLE_MULTI	2
#define OLD_GAME_TYPE_TRAINING		3

#define MAX_MISSION_TYPES				5
#define MISSION_TYPE_SINGLE			(1<<0)
#define MISSION_TYPE_MULTI				(1<<1)
#define MISSION_TYPE_TRAINING			(1<<2)
#define MISSION_TYPE_MULTI_COOP		(1<<3)
#define MISSION_TYPE_MULTI_TEAMS		(1<<4)
#define MISSION_TYPE_MULTI_DOGFIGHT	(1<<5)

#define MISSION_FLAG_SUBSPACE					(1<<0)	// mission takes place in subspace
#define MISSION_FLAG_NO_PROMOTION				(1<<1)	// cannot get promoted or badges in this mission
#define MISSION_FLAG_FULLNEB					(1<<2)	// mission is a full nebula mission
#define MISSION_FLAG_NO_BUILTIN_MSGS			(1<<3)	// disables builtin msgs
#define MISSION_FLAG_NO_TRAITOR					(1<<4)	// player cannot become a traitor
#define MISSION_FLAG_TOGGLE_SHIP_TRAILS			(1<<5)	// toggles ship trails (off in nebula, on outside nebula)
#define MISSION_FLAG_SUPPORT_REPAIRS_HULL		(1<<6)	// Toggles support ship repair of ship hulls
#define MISSION_FLAG_BEAM_FREE_ALL_BY_DEFAULT	(1<<7)	// Beam-free-all by default - Goober5000
#define MISSION_FLAG_CURRENTLY_UNUSED_1			(1<<8)
#define MISSION_FLAG_CURRENTLY_UNUSED_2			(1<<9)
#define MISSION_FLAG_NO_BRIEFING				(1<<10)	// no briefing, jump right into mission - Goober5000
#define MISSION_FLAG_TOGGLE_DEBRIEFING			(1<<11)	// Turn on debriefing for dogfight. Off for everything else - Goober5000
#define MISSION_FLAG_CURRENTLY_UNUSED_3			(1<<12)
#define MISSION_FLAG_ALLOW_DOCK_TREES			(1<<13)	// toggle between hub and tree model for ship docking (see objectdock.cpp) - Gooober5000
#define MISSION_FLAG_2D_MISSION					(1<<14) // Mission is meant to be played top-down style; 2D physics and movement.
#define MISSION_FLAG_CURRENTLY_UNUSED_4			(1<<15)
#define MISSION_FLAG_RED_ALERT					(1<<16)	// a red-alert mission - Goober5000
#define MISSION_FLAG_SCRAMBLE					(1<<17)	// a scramble mission - Goober5000
#define MISSION_FLAG_NO_BUILTIN_COMMAND			(1<<18)	// turns off Command without turning off pilots - Karajorma
#define MISSION_FLAG_PLAYER_START_AI			(1<<19) // Player Starts mission under AI Control (NOT MULTI COMPATABLE) - Kazan
#define MISSION_FLAG_ALL_ATTACK					(1<<20)	// all teams at war - Goober5000
#define MISSION_FLAG_USE_AP_CINEMATICS			(1<<21) // Kazan - use autopilot cinematics
#define MISSION_FLAG_DEACTIVATE_AP         	    (1<<22) // KeldorKatarn - deactivate autopilot (patch approved by Kazan)

// some mice macros for mission type
#define IS_MISSION_MULTI_COOP			(The_mission.game_type & MISSION_TYPE_MULTI_COOP)
#define IS_MISSION_MULTI_TEAMS		(The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)
#define IS_MISSION_MULTI_DOGFIGHT	(The_mission.game_type & MISSION_TYPE_MULTI_DOGFIGHT)

// movie type defines
#define	MOVIE_PRE_FICTION		0
#define	MOVIE_PRE_CMD_BRIEF		1
#define	MOVIE_PRE_BRIEF			2
#define	MOVIE_PRE_GAME			3
#define	MOVIE_PRE_DEBRIEF		4
#define MOVIE_END_CAMPAIGN		5

class wxFREDMissionHeader {
public:
	wxString	name;
	wxString	author;
	float	version;
	wxString	created;
	wxString	modified;
	wxString	notes;
	wxString	mission_desc;
	int	game_type;
	int	flags;
	int	num_players;									// valid in multiplayer missions -- number of players supported
	uint	num_respawns;									// valid in multiplayer missions -- number of respawns allowed
	int		max_respawn_delay;									// valid in multiplayer missions -- number of respawns allowed
	support_ship_info	support_ships;		// Goober5000
	wxString	squad_filename;			// if the player has been reassigned to a squadron, this is the filename of the logo, otherwise empty string
	wxString	squad_name;				// if the player has been reassigned to a squadron, this is the name of the squadron, otherwise empty string
	char	loading_screen[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN];
	char	skybox_model[MAX_FILENAME_LEN];
	char	envmap_name[MAX_FILENAME_LEN];
	int		skybox_flags;
	int		contrail_threshold;
	int		ambient_light_level;
	sound_env	sound_environment;

	// Goober5000
	int	command_persona;
	wxString command_sender;

	// Goober5000
	wxString event_music_name;
	wxString briefing_music_name;
	wxString substitute_event_music_name;
	wxString substitute_briefing_music_name;

	// Goober5000
	ai_profile_t *ai_profile;

	//SCP_vector<mission_cutscene> cutscenes;

	void Reset_header( )
	{
		name = "Untitled";
		author = "Someone";
		version = 0.;
		
		wxDateTime* now = new wxDateTime();
		created = now->Now().Format("%c");
		modified = now->Now().Format("%c");
		notes = "";
		mission_desc = "";
		game_type = 0;
		flags = 0;
		num_players = 0;
		num_respawns = 0;
		max_respawn_delay = 0;
		memset( &support_ships, 0, sizeof( support_ships ) );

		support_ships.arrival_location = ARRIVE_AT_LOCATION;
		support_ships.arrival_anchor = -1;
		support_ships.departure_location = DEPART_AT_LOCATION;
		support_ships.departure_anchor = -1;
		support_ships.max_hull_repair_val = 0.0f;
		support_ships.max_subsys_repair_val = 100.0f;	//ASSUMPTION: full repair capabilities
		support_ships.max_support_ships = -1;	// infinite
		support_ships.ship_class = -1;
		support_ships.tally = 0;
		support_ships.support_available_for_species = 0;

		squad_filename = "";
		squad_name = "";
		for ( int i = 0; i < GR_NUM_RESOLUTIONS; i++ )
			loading_screen[ i ][ 0 ] = '\0';
		skybox_model[ 0 ] = '\0';
		envmap_name[ 0 ] = '\0';
		skybox_flags = 0;
		contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
		ambient_light_level = 0;
		sound_environment.id = -1;
		command_persona = 0;
		command_sender = "";
		event_music_name = "";
		briefing_music_name = "";
		substitute_event_music_name = "";
		substitute_briefing_music_name = "";
		ai_profile = NULL;
		//cutscenes.clear( );
	}

	wxFREDMissionHeader( )
	{
		Reset_header( );
	}

};

class wxFREDMission : public wxFREDMissionHeader {
public:
	wxFREDMission() {
		Reset_header();
	}
};

#endif
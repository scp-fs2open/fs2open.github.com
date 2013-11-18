/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _PARSE_H
#define _PARSE_H

#include <setjmp.h>
#include "ai/ai.h"
#include "ai/ai_profiles.h"
#include "model/model.h"
#include "object/object.h"
#include "graphics/2d.h"
#include "sound/sound.h"
#include "parse/sexp.h"
#include "io/keycontrol.h"

//WMC - This should be here
#define FS_MISSION_FILE_EXT				NOX(".fs2")

struct wing;
struct p_dock_instance;

#define NUM_NEBULAS			3				// how many background nebulas we have altogether
#define NUM_NEBULA_COLORS	9

#define DEFAULT_AMBIENT_LIGHT_LEVEL			0x00787878

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


// Goober5000
typedef struct support_ship_info {
	int		arrival_location;				// arrival location
	int		arrival_anchor;					// arrival anchor
	int		departure_location;				// departure location
	int		departure_anchor;				// departure anchor
	float	max_hull_repair_val;			// % of a ship's hull that can be repaired -C
	float	max_subsys_repair_val;			// same thing, except for subsystems -C
	int		max_support_ships;				// max number of consecutive support ships
	int		max_concurrent_ships;			// max number of concurrent support ships in mission per team
	int		ship_class;						// ship class of support ship
	int		tally;							// number of support ships so far
	int		support_available_for_species;	// whether support is available for a given species (this is a bitfield)
} support_ship_info;

// movie type defines
#define	MOVIE_PRE_FICTION		0
#define	MOVIE_PRE_CMD_BRIEF		1
#define	MOVIE_PRE_BRIEF			2
#define	MOVIE_PRE_GAME			3
#define	MOVIE_PRE_DEBRIEF		4
#define MOVIE_END_CAMPAIGN		5

// defines a mission cutscene.
typedef struct mission_cutscene {
	int type; 
	char cutscene_name[NAME_LENGTH];	
	int formula; 

	mission_cutscene( ) 
		: type( 0 ), formula( -1 )
	{ 
		cutscene_name[ 0 ] = 0;
	}
} mission_cutscene;

typedef struct mission {
	char	name[NAME_LENGTH];
	char	author[NAME_LENGTH];
	float	version;
	char	created[DATE_TIME_LENGTH];
	char	modified[DATE_TIME_LENGTH];
	char	notes[NOTES_LENGTH];
	char	mission_desc[MISSION_DESC_LENGTH];
	int	game_type;
	int	flags;
	int	num_players;									// valid in multiplayer missions -- number of players supported
	uint	num_respawns;									// valid in multiplayer missions -- number of respawns allowed
	int		max_respawn_delay;									// valid in multiplayer missions -- number of respawns allowed
	support_ship_info	support_ships;		// Goober5000
	char	squad_filename[MAX_FILENAME_LEN];		// if the player has been reassigned to a squadron, this is the filename of the logo, otherwise empty string
	char	squad_name[NAME_LENGTH];				// if the player has been reassigned to a squadron, this is the name of the squadron, otherwise empty string
	char	loading_screen[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN];
	char	skybox_model[MAX_FILENAME_LEN];
	matrix	skybox_orientation;
	char	envmap_name[MAX_FILENAME_LEN];
	int		skybox_flags;
	int		contrail_threshold;
	int		ambient_light_level;
	float	neb_far_multi;
	float	neb_near_multi;
	sound_env	sound_environment;

	// Goober5000
	int	command_persona;
	char command_sender[NAME_LENGTH];

	// Goober5000
	char event_music_name[NAME_LENGTH];
	char briefing_music_name[NAME_LENGTH];
	char substitute_event_music_name[NAME_LENGTH];
	char substitute_briefing_music_name[NAME_LENGTH];

	// Goober5000
	ai_profile_t *ai_profile;

	SCP_vector<mission_cutscene> cutscenes;

	void Reset( )
	{
		int i = 0;
		name[ 0 ] = '\0';
		author[ 0 ] = '\0';
		version = 0.;
		created[ 0 ] = '\0';
		modified[ 0 ] = '\0';
		notes[ 0 ] = '\0';
		mission_desc[ 0 ] = '\0';
		game_type = 0;
		flags = 0;
		num_players = 0;
		num_respawns = 0;
		max_respawn_delay = 0;
		memset(&Ignored_keys, 0, sizeof(int)*CCFG_MAX);
		memset( &support_ships, 0, sizeof( support_ships ) );
		squad_filename[ 0 ] = '\0';
		squad_name[ 0 ] = '\0';
		for ( i = 0; i < GR_NUM_RESOLUTIONS; i++ )
			loading_screen[ i ][ 0 ] = '\0';
		skybox_model[ 0 ] = '\0';
		vm_set_identity(&skybox_orientation);
		skybox_flags = 0;
		envmap_name[ 0 ] = '\0';
		contrail_threshold = 0;
		ambient_light_level = DEFAULT_AMBIENT_LIGHT_LEVEL;
		sound_environment.id = -1;
		command_persona = 0;
		command_sender[ 0 ] = '\0';
		event_music_name[ 0 ] = '\0';
		briefing_music_name[ 0 ] = '\0';
		substitute_event_music_name[ 0 ] = '\0';
		substitute_briefing_music_name[ 0 ] = '\0';
		ai_profile = NULL;
		cutscenes.clear( );
	}

	mission( )
	{
		Reset( );
	}
} mission;

// cargo defines
// NOTE: MAX_CARGO MUST REMAIN <= 64 (CARGO_NO_DEPLETE) for NO_DEPLETE to work.
// FURTHER NOTE (Goober5000): If a new flag is added here, the code (particularly in sexp.cpp)
// must be reworked so that all the flags are maintained from function to function
#define CARGO_INDEX_MASK	0xBF
#define CARGO_NO_DEPLETE	0x40		// CARGO_NO_DEPLETE + CARGO_INDEX_MASK must == FF
#define MAX_CARGO				30


// Goober5000 - contrail threshold (previously defined in ShipContrails.cpp)
#define CONTRAIL_THRESHOLD_DEFAULT	45

extern mission The_mission;
extern char Mission_filename[80];  // filename of mission in The_mission (Fred only)

#define	MAX_FORMATION_NAMES	3
#define	MAX_STATUS_NAMES		3

// defines for arrival locations.  These defines should match their counterparts in the arrival location
// array
#define	MAX_ARRIVAL_NAMES				4
#define	ARRIVE_AT_LOCATION			0
#define	ARRIVE_NEAR_SHIP				1
#define	ARRIVE_IN_FRONT_OF_SHIP		2
#define	ARRIVE_FROM_DOCK_BAY			3

// defines for departure locations.  These defines should match their counterparts in the departure location
// array
#define MAX_DEPARTURE_NAMES			2
#define DEPART_AT_LOCATION				0
#define DEPART_AT_DOCK_BAY				1

#define	MAX_GOAL_TYPE_NAMES	3

// alternate ship type names
#define MAX_ALT_TYPE_NAMES				100
extern char Mission_alt_types[MAX_ALT_TYPE_NAMES][NAME_LENGTH];
extern int Mission_alt_type_count;

// callsign
#define MAX_CALLSIGNS					100
extern char Mission_callsigns[MAX_CALLSIGNS][NAME_LENGTH];
extern int Mission_callsign_count;

// path restrictions
#define MAX_PATH_RESTRICTIONS		10
typedef struct path_restriction_t {
	int num_paths;
	int cached_mask;
	char path_names[MAX_SHIP_BAY_PATHS][MAX_NAME_LEN];
} path_restriction_t;

extern char *Ship_class_names[MAX_SHIP_CLASSES];
extern char *Ai_behavior_names[MAX_AI_BEHAVIORS];
extern char *Formation_names[MAX_FORMATION_NAMES];
extern char *Status_desc_names[MAX_STATUS_NAMES];
extern char *Status_type_names[MAX_STATUS_NAMES];
extern char *Status_target_names[MAX_STATUS_NAMES];
extern char *Arrival_location_names[MAX_ARRIVAL_NAMES];
extern char *Departure_location_names[MAX_DEPARTURE_NAMES];
extern char *Goal_type_names[MAX_GOAL_TYPE_NAMES];

extern char *Reinforcement_type_names[];
extern char *Object_flags[];
extern char *Parse_object_flags[];
extern char *Parse_object_flags_2[];
extern char *Icon_names[];
extern char *Mission_event_log_flags[];

extern char *Cargo_names[MAX_CARGO];
extern char Cargo_names_buf[MAX_CARGO][NAME_LENGTH];

extern char Mission_parse_storm_name[NAME_LENGTH];

extern int	Num_iff;
extern int	Num_ai_behaviors;
extern int	Num_ai_classes;
extern int	Num_cargo;
extern int	Num_status_names;
extern int	Num_arrival_names;
extern int	Num_formation_names;
extern int	Num_goal_type_names;
extern int	Num_reinforcement_type_names;
extern int	Player_starts;
extern fix	Entry_delay_time;
extern int	Loading_screen_bm_index;

extern int Num_unknown_ship_classes;
extern int Num_unknown_weapon_classes;
extern int Num_unknown_loadout_classes;

extern ushort Current_file_checksum;
extern int    Current_file_length;

#define SUBSYS_STATUS_NO_CHANGE	-999

typedef struct subsys_status {
	char	name[NAME_LENGTH];
	float	percent;  // percent damaged
	int	primary_banks[MAX_SHIP_PRIMARY_BANKS];
	int primary_ammo[MAX_SHIP_PRIMARY_BANKS];
	int	secondary_banks[MAX_SHIP_SECONDARY_BANKS];
	int	secondary_ammo[MAX_SHIP_SECONDARY_BANKS];
	int	ai_class;
	int	subsys_cargo_name;
} subsys_status;

typedef struct texture_replace {
	char ship_name[NAME_LENGTH];
	char old_texture[MAX_FILENAME_LEN];
	char new_texture[MAX_FILENAME_LEN];
	int new_texture_id;
} texture_replace;

extern SCP_vector<texture_replace> Fred_texture_replacements;

typedef struct alt_class {
	int ship_class;				
	int variable_index;			// if set allows the class to be set by a variable
	bool default_to_this_class;
}alt_class;

#define MAX_OBJECT_STATUS	10

//	a parse object
//	information from a $OBJECT: definition is read into this struct to
// be copied into the real object, ship, etc. structs
class p_object
{
public:
	char	name[NAME_LENGTH];
	p_object *next, *prev;

	vec3d	pos;
	matrix	orient;
	int	ship_class;
	int	team;
	int	behavior;							// ai_class;
	int	ai_goals;							// sexp of lists of goals that this ship should try and do
	char	cargo1;
	SCP_string team_color_setting;

	int	status_count;
	int	status_type[MAX_OBJECT_STATUS];
	int	status[MAX_OBJECT_STATUS];
	int	target[MAX_OBJECT_STATUS];

	int	subsys_index;						// index into subsys_status array
	int	subsys_count;						// number of elements used in subsys_status array
	int	initial_velocity;
	int	initial_hull;
	int	initial_shields;

	int	arrival_location;
	int	arrival_distance;					// used when arrival location is near or in front of some ship
	int	arrival_anchor;						// ship used for anchoring an arrival point
	int arrival_path_mask;					// Goober5000
	int	arrival_cue;						//	Index in Sexp_nodes of this sexp.
	int	arrival_delay;

	int	departure_location;
	int	departure_anchor;
	int departure_path_mask;				// Goober5000
	int	departure_cue;						//	Index in Sexp_nodes of this sexp.
	int	departure_delay;

	char	misc[NAME_LENGTH];

	int	wingnum;							// set to -1 if not in a wing -- Wing array index otherwise
	int pos_in_wing;						// Goober5000 - needed for FRED with the new way things work

	int	flags;								// mission savable flags
	int flags2;								// Goober5000
	int	escort_priority;					// priority in escort list
	int	ai_class;
	int	hotkey;								// hotkey number (between 0 and 9) -1 means no hotkey
	int	score;
	float assist_score_pct;					// percentage of the score which players who gain an assist will get when this ship is killed
	int	orders_accepted;					// which orders this ship will accept from the player
	p_dock_instance	*dock_list;				// Goober5000 - parse objects this parse object is docked to
	object *created_object;					// Goober5000
	int	group;								// group object is within or -1 if none.
	int	persona_index;
	int	kamikaze_damage;					// base damage for a kamikaze attack

	bool use_special_explosion;				// new special explosion/hitpoints system 
	int special_exp_damage;
	int special_exp_blast;
	int special_exp_inner;
	int special_exp_outer;
	bool use_shockwave;
	int special_exp_shockwave_speed;
	int special_exp_deathroll_time;

	int	special_hitpoints;
	int	special_shield;

	ushort net_signature;					// network signature this object can have
	int destroy_before_mission_time;

	char	wing_status_wing_index;			// wing index (0-4) in wingman status gauge
	char	wing_status_wing_pos;			// wing position (0-5) in wingman status gauge

	uint	respawn_count;						// number of respawns for this object.  Applies only to player wing ships in multiplayer
	int	respawn_priority;					// priority this ship has for controlling respawn points

	int		alt_type_index;					// optional alt type index
	int		callsign_index;					// optional callsign index

	float ship_max_hull_strength;			// Needed to deal with special hitpoints
	float ship_max_shield_strength;

	// Goober5000
	int num_texture_replacements;
	texture_replace replacement_textures[MAX_REPLACEMENT_TEXTURES];	// replacement textures - Goober5000

	SCP_vector<alt_class> alt_classes;	

	int alt_iff_color[MAX_IFFS][MAX_IFFS];

	p_object();
	~p_object();
};

// defines for flags used for p_objects when they are created.  Used to help create special
// circumstances for those ships.  This list of bitfield indicators MUST correspond EXACTLY
// (i.e., order and position must be the same) to its counterpart in MissionParse.cpp!!!!

#define MAX_PARSE_OBJECT_FLAGS	27

#define P_SF_CARGO_KNOWN				(1<<0)
#define P_SF_IGNORE_COUNT				(1<<1)
#define P_OF_PROTECTED					(1<<2)
#define P_SF_REINFORCEMENT				(1<<3)
#define P_OF_NO_SHIELDS					(1<<4)
#define P_SF_ESCORT						(1<<5)
#define P_OF_PLAYER_START				(1<<6)
#define P_SF_NO_ARRIVAL_MUSIC			(1<<7)
#define P_SF_NO_ARRIVAL_WARP			(1<<8)
#define P_SF_NO_DEPARTURE_WARP			(1<<9)
#define P_SF_LOCKED						(1<<10)
#define P_OF_INVULNERABLE				(1<<11)
#define P_SF_HIDDEN_FROM_SENSORS		(1<<12)
#define P_SF_SCANNABLE					(1<<13)	// ship is a "scannable" ship
#define P_AIF_KAMIKAZE					(1<<14)
#define P_AIF_NO_DYNAMIC				(1<<15)
#define P_SF_RED_ALERT_STORE_STATUS		(1<<16)
#define P_OF_BEAM_PROTECTED				(1<<17)
#define P_OF_FLAK_PROTECTED				(1<<18)
#define P_OF_LASER_PROTECTED			(1<<19)
#define P_OF_MISSILE_PROTECTED			(1<<20)
#define P_SF_GUARDIAN					(1<<21)
#define P_KNOSSOS_WARP_IN				(1<<22)
#define P_SF_VAPORIZE					(1<<23)
#define P_SF2_STEALTH					(1<<24)
#define P_SF2_FRIENDLY_STEALTH_INVIS	(1<<25)
#define P_SF2_DONT_COLLIDE_INVIS		(1<<26)

// the following parse object flags are used internally by FreeSpace
#define P_SF_USE_UNIQUE_ORDERS		(1<<27)	// tells a newly created ship to use the default orders for that ship
#define P_SF_DOCK_LEADER			(1<<28)	// Goober5000 - a docked parse object that is the leader of its group
#define P_SF_CANNOT_ARRIVE			(1<<29)	// used to indicate that this ship's arrival cue will never be true
#define P_SF_WARP_BROKEN			(1<<30)	// warp engine should be broken for this ship
#define P_SF_WARP_NEVER				(1<<31)	// warp drive is destroyed

// more parse flags! -- Goober5000
// same caveat: This list of bitfield indicators MUST correspond EXACTLY
// (i.e., order and position must be the same) to its counterpart in MissionParse.cpp!!!!

#define MAX_PARSE_OBJECT_FLAGS_2	22

#define P2_SF2_PRIMITIVE_SENSORS			(1<<0)
#define P2_SF2_NO_SUBSPACE_DRIVE			(1<<1)
#define P2_SF2_NAV_CARRY_STATUS				(1<<2)
#define P2_SF2_AFFECTED_BY_GRAVITY			(1<<3)
#define P2_SF2_TOGGLE_SUBSYSTEM_SCANNING	(1<<4)
#define P2_OF_TARGETABLE_AS_BOMB			(1<<5)
#define P2_SF2_NO_BUILTIN_MESSAGES			(1<<6)
#define P2_SF2_PRIMARIES_LOCKED				(1<<7)
#define P2_SF2_SECONDARIES_LOCKED			(1<<8)
#define P2_SF2_NO_DEATH_SCREAM				(1<<9)
#define P2_SF2_ALWAYS_DEATH_SCREAM			(1<<10)
#define P2_SF2_NAV_NEEDSLINK				(1<<11)
#define P2_SF2_HIDE_SHIP_NAME				(1<<12)
#define P2_SF2_SET_CLASS_DYNAMICALLY		(1<<13)
#define P2_SF2_LOCK_ALL_TURRETS_INITIALLY	(1<<14)		
#define P2_SF2_AFTERBURNER_LOCKED			(1<<15)	
#define P2_OF_FORCE_SHIELDS_ON				(1<<16)
#define P2_OF_IMMOBILE						(1<<17)
#define P2_SF2_NO_ETS						(1<<18)
#define P2_SF2_CLOAKED						(1<<19)
#define P2_SF2_SHIP_LOCKED					(1<<20)
#define P2_SF2_WEAPONS_LOCKED				(1<<21)

// and again: these flags do not appear in the array
//#define blah							(1<<29)
//#define blah							(1<<30)
#define P2_ALREADY_HANDLED				(1<<31)	// Goober5000 - used for docking currently, but could be used generically


// Goober5000 - this is now dynamic
extern SCP_vector<p_object> Parse_objects;
#define POBJ_INDEX(pobjp) (pobjp - &Parse_objects[0])	// yes, this arithmetic is valid :D

extern p_object Support_ship_pobj, *Arriving_support_ship;
extern p_object Ship_arrival_list;

typedef struct {
	// ships
	int		default_ship;  // default ship type for player start point (recommended choice)
	int		num_ship_choices; // number of ship choices inside ship_list 
	int		loadout_total;	// Total number of ships available of all classes 
	int		ship_list[MAX_SHIP_CLASSES];
	char	ship_list_variables[MAX_SHIP_CLASSES][TOKEN_LENGTH];
	int		ship_count[MAX_SHIP_CLASSES];
	char	ship_count_variables[MAX_SHIP_CLASSES][TOKEN_LENGTH];

	// weapons
	int		num_weapon_choices;
	int		weaponry_pool[MAX_WEAPON_TYPES];
	int		weaponry_count[MAX_WEAPON_TYPES];
	char	weaponry_pool_variable[MAX_WEAPON_TYPES][TOKEN_LENGTH];
	char	weaponry_amount_variable[MAX_WEAPON_TYPES][TOKEN_LENGTH];
	bool	weapon_required[MAX_WEAPON_TYPES];
} team_data;

#define MAX_P_WINGS		16
#define MAX_SHIP_LIST	16

extern team_data Team_data[MAX_TVT_TEAMS];
extern subsys_status *Subsys_status;
extern int Subsys_index;

extern vec3d Parse_viewer_pos;
extern matrix Parse_viewer_orient;

extern int Mission_arrival_timestamp;
extern int Mission_departure_timestamp;
extern fix Mission_end_time;

extern char Parse_names[MAX_SHIPS + MAX_WINGS][NAME_LENGTH];
extern int Num_parse_names;
extern int Num_teams;

extern char			Player_start_shipname[NAME_LENGTH];
extern int			Player_start_shipnum;
extern p_object	*Player_start_pobject;

extern int Mission_palette;  // index of palette file to use for mission
extern int Nebula_index;  // index into Nebula_filenames[] of nebula to use in mission.
extern char *Nebula_filenames[NUM_NEBULAS];
extern char *Nebula_colors[NUM_NEBULA_COLORS];
extern p_object *Arriving_support_ship;

extern char Neb2_texture_name[MAX_FILENAME_LEN];


int parse_main(const char *mission_name, int flags = 0);
p_object *mission_parse_get_arrival_ship(ushort net_signature);
p_object *mission_parse_get_arrival_ship(const char *name);
p_object *mission_parse_get_parse_object(ushort net_signature);
p_object *mission_parse_get_parse_object(const char *name);
int parse_create_object(p_object *objp);
void resolve_parse_flags(object *objp, int parse_flags, int parse_flags2);

void mission_parse_close();

// used in squadmate messaging stuff to create wings from reinforcements.
int parse_wing_create_ships(wing *wingp, int num_to_create, int force = 0, int specific_instance = -1 );

// function for getting basic mission data without loading whole mission
int mission_parse_is_multi(const char *filename, char *mission_name );
int mission_parse_get_multi_mission_info(const char *filename);

// called externally from multiplayer code
int mission_do_departure(object *objp, bool goal_is_to_warp = false);

// called externally from freespace.cpp
void mission_parse_fixup_players(void);

// get a index to a perminently kept around name of a ship or wing
int get_parse_name_index(const char *name);

// called from freespace game level loop
void mission_parse_eval_stuff();

// function to set the ramaing time left in the mission
void mission_parse_set_end_time( int seconds );

// code to bring in a repair ship.
void mission_bring_in_support_ship( object *requester_objp );
int mission_is_support_ship_arriving( void );
void mission_add_to_arriving_support( object *requester_objp );
int mission_is_repair_scheduled( object *objp );
int mission_remove_scheduled_repair( object *objp );
void mission_parse_support_arrived( int objnum );

// alternate name stuff
int mission_parse_lookup_alt(const char *name);
void mission_parse_lookup_alt_index(int index, char *out);
int mission_parse_add_alt(const char *name);
void mission_parse_remove_alt(const char *name);
void mission_parse_reset_alt();

// callsign stuff
int mission_parse_lookup_callsign(const char *name);
void mission_parse_lookup_callsign_index(int index, char *out);
int mission_parse_add_callsign(const char *name);
void mission_parse_remove_callsign(const char *name);
void mission_parse_reset_callsign();

// is training mission
int is_training_mission();

// code to save/restore mission parse stuff
int get_mission_info(const char *filename, mission *missionp = NULL, bool basic = true);

// Goober5000
void parse_dock_one_docked_object(p_object *pobjp, p_object *parent_pobjp);

// Goober5000
extern int Knossos_warp_ani_used;

// Karajorma
void swap_parse_object(p_object *p_obj, int ship_class);
void clear_texture_replacements();

// Goober5000
subsys_status *parse_get_subsys_status(p_object *pobjp, char *subsys_name);


#endif


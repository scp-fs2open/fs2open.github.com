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

#include <csetjmp>
#include <set>

#include "ai/ai.h"
#include "ai/ai_profiles.h"
#include "globalincs/version.h"
#include "graphics/2d.h"
#include "io/keycontrol.h"
#include "model/model.h"
#include "model/animation/modelanimation.h"
#include "object/object.h"
#include "parse/sexp.h"
#include "sound/sound.h"
#include "mission/mission_flags.h"
#include "nebula/volumetrics.h"
#include "stats/scoring.h"

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

int get_special_anchor(const char *name);

// MISSION_VERSION should be the earliest version of FSO that can load the current mission format without
// requiring version-specific comments.  It should be updated whenever the format changes, but it should
// not be updated simply because the engine's version changed.
extern const gameversion::version MISSION_VERSION;
extern const gameversion::version LEGACY_MISSION_VERSION;

// This checks to see if a mission has data that requires saving in a newer format.  This would warrant
// a "soft version bump" rather than a hard bump because not all missions are affected.
extern bool check_for_23_3_data();

#define WING_PLAYER_BASE	0x80000  // used by Fred to tell ship_index in a wing points to a player

// mission parse flags used for parse_mission() to tell what kind of information to get from the mission file
#define MPF_ONLY_MISSION_INFO	(1 << 0)
#define MPF_IMPORT_FSM			(1 << 1)
#define MPF_FAST_RELOAD			(1 << 2)	// skip clearing some stuff so we can load the mission faster (usually since it's the same mission)

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
// If you add one here, you must also add a description to missioncutscenedlg.cpp for FRED
// and update the dropdown list in fred.rc for the mission cutscene dialog editor.
enum : int {
	MOVIE_PRE_FICTION,
	MOVIE_PRE_CMD_BRIEF,
	MOVIE_PRE_BRIEF,
	MOVIE_PRE_GAME,
	MOVIE_PRE_DEBRIEF,
	MOVIE_POST_DEBRIEF,
	MOVIE_END_CAMPAIGN,

	// Must always be at the end of the list
	Num_movie_types
};

// defines a mission cutscene.
typedef struct mission_cutscene {
	int type; 
	char filename[MAX_FILENAME_LEN];
	int formula;
} mission_cutscene;

typedef struct mission_default_custom_data {
	SCP_string key;
	SCP_string value;
	SCP_string description;
} mission_default_custom_data;

typedef struct mission_custom_string {
	SCP_string name;
	SCP_string value;
	SCP_string text;
} mission_custom_string;

// descriptions of flags for FRED
template <class T>
struct parse_object_flag_description {
	T def;
	SCP_string flag_desc;
};

typedef struct mission {
	char	name[NAME_LENGTH];
	SCP_string	author;
	gameversion::version	required_fso_version;
	char	created[DATE_TIME_LENGTH];
	char	modified[DATE_TIME_LENGTH];
	char	notes[NOTES_LENGTH];
	char	mission_desc[MISSION_DESC_LENGTH];
	int	game_type;
    flagset<Mission::Mission_Flags> flags;
	int	num_players;									// valid in multiplayer missions -- number of players supported
	uint	num_respawns;									// valid in multiplayer missions -- number of respawns allowed
	int		max_respawn_delay;									// valid in multiplayer missions -- number of respawns allowed
	support_ship_info	support_ships;		// Goober5000
	char	squad_filename[MAX_FILENAME_LEN];		// if the player has been reassigned to a squadron, this is the filename of the logo, otherwise empty string
	char	squad_name[NAME_LENGTH];				// if the player has been reassigned to a squadron, this is the name of the squadron, otherwise empty string
	char	loading_screen[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN];
	char	skybox_model[MAX_FILENAME_LEN];
	animation::ModelAnimationSet skybox_model_animations;
	matrix	skybox_orientation;
	char	envmap_name[MAX_FILENAME_LEN];
	int		skybox_flags;
	int		contrail_threshold;
	int		ambient_light_level;
	float	neb_far_multi;
	float	neb_near_multi;
	tl::optional<volumetric_nebula> volumetrics;
	sound_env	sound_environment;
	vec3d   gravity;
	int     HUD_timer_padding;

	// Goober5000
	int	command_persona;
	char command_sender[NAME_LENGTH];
	int debriefing_persona;
	traitor_override_t* traitor_override;

	// Goober5000
	char event_music_name[NAME_LENGTH];
	char briefing_music_name[NAME_LENGTH];
	char substitute_event_music_name[NAME_LENGTH];
	char substitute_briefing_music_name[NAME_LENGTH];

	// Goober5000
	ai_profile_t *ai_profile;

	SCP_string lighting_profile_name;

	SCP_vector<mission_cutscene> cutscenes;

	SCP_map<SCP_string, SCP_string> custom_data;

	SCP_vector<mission_custom_string> custom_strings;

	void Reset( );

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
#define MAX_CARGO				60


// Goober5000 - contrail threshold (previously defined in ShipContrails.cpp)
#define CONTRAIL_THRESHOLD_DEFAULT	45

extern mission The_mission;
extern char Mission_filename[80];  // filename of mission in The_mission (Fred only)

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

extern const char *Ship_class_names[MAX_SHIP_CLASSES];
extern const char *Ai_behavior_names[MAX_AI_BEHAVIORS];
extern const char *Arrival_location_names[MAX_ARRIVAL_NAMES];
extern const char *Departure_location_names[MAX_DEPARTURE_NAMES];
extern const char *Goal_type_names[MAX_GOAL_TYPE_NAMES];

extern const char *Reinforcement_type_names[];
extern char *Object_flags[];
extern flag_def_list_new<Mission::Parse_Object_Flags> Parse_object_flags[];
extern parse_object_flag_description<Mission::Parse_Object_Flags> Parse_object_flag_descriptions[];
extern const size_t Num_parse_object_flags;
extern const char *Icon_names[];
extern const char *Mission_event_log_flags[];

extern int Num_mission_event_flags;
extern flag_def_list Mission_event_flags[];

extern char *Cargo_names[MAX_CARGO];
extern char Cargo_names_buf[MAX_CARGO][NAME_LENGTH];

extern char Mission_parse_storm_name[NAME_LENGTH];

extern int	Num_iff;
extern int	Num_ai_behaviors;
extern int	Num_ai_classes;
extern int	Num_cargo;
extern int	Num_arrival_names;
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

extern SCP_vector<mission_default_custom_data> Default_custom_data;

#define SUBSYS_STATUS_NO_CHANGE	-999

// Squadron Default Name
#define NO_SQUAD	"<none>"

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
	bool from_table;
} texture_replace;

extern SCP_vector<texture_replace> Fred_texture_replacements;

typedef struct alt_class {
	int ship_class;				
	int variable_index;			// if set allows the class to be set by a variable
	bool default_to_this_class;
}alt_class;

//	a parse object
//	information from a $OBJECT: definition is read into this struct to
// be copied into the real object, ship, etc. structs
class p_object
{
public:
	char	name[NAME_LENGTH] = "";
	SCP_string display_name;
	p_object *next = nullptr, *prev = nullptr;

	vec3d	pos = vmd_zero_vector;
	matrix	orient = vmd_identity_matrix;
	int	ship_class = -1;
	int	team = -1;
	int loadout_team = -1;						// original team, should never be changed after being set!!
	int	ai_goals = -1;							// sexp of lists of goals that this ship should try and do
	char	cargo1 = '\0';
	SCP_string team_color_setting;

	int	subsys_index = -1;						// index into subsys_status array
	int	subsys_count = 0;						// number of elements used in subsys_status array
	int	initial_velocity = 0;
	int	initial_hull = 100;
	int	initial_shields = 100;

	int	arrival_location = ARRIVE_AT_LOCATION;
	int	arrival_distance = 0;					// used when arrival location is near or in front of some ship
	int	arrival_anchor = -1;						// ship used for anchoring an arrival point
	int arrival_path_mask = 0;					// Goober5000
	int	arrival_cue = -1;				//	Index in Sexp_nodes of this sexp.
	int	arrival_delay = 0;

	int	departure_location = DEPART_AT_LOCATION;
	int	departure_anchor = -1;
	int departure_path_mask = 0;				// Goober5000
	int	departure_cue = -1;			//	Index in Sexp_nodes of this sexp.
	int	departure_delay = 0;

	int warpin_params_index = -1;
	int warpout_params_index = -1;

	int	wingnum = -1;							// set to -1 if not in a wing -- Wing array index otherwise
	int pos_in_wing = -1;						// Goober5000 - needed for FRED with the new way things work

	flagset<Mission::Parse_Object_Flags>	flags;								// mission savable flags
	int	escort_priority = 0;					// priority in escort list
	int	ai_class = -1;
	int	hotkey = -1;								// hotkey number (between 0 and 9) -1 means no hotkey
	int	score = 0;
	float assist_score_pct = 0.0f;					// percentage of the score which players who gain an assist will get when this ship is killed
	SCP_set<size_t> orders_accepted;		// which orders this ship will accept from the player
	p_dock_instance	*dock_list = nullptr;				// Goober5000 - parse objects this parse object is docked to
	object *created_object = nullptr;					// Goober5000
	int collision_group_id = 0;							// Goober5000
	int	group = -1;								// group object is within or -1 if none.
	int	persona_index = -1;
	int	kamikaze_damage = 0;					// base damage for a kamikaze attack

	bool use_special_explosion = false;				// new special explosion/hitpoints system 
	int special_exp_damage = -1;
	int special_exp_blast = -1;
	int special_exp_inner = -1;
	int special_exp_outer = -1;
	bool use_shockwave = false;
	int special_exp_shockwave_speed = 0;
	int special_exp_deathroll_time = 0;

	int	special_hitpoints = 0;
	int	special_shield = -1;

	ushort net_signature = 0;					// network signature this object can have
	int destroy_before_mission_time = -1;

	char	wing_status_wing_index = -1;			// wing index (0-4) in wingman status gauge
	char	wing_status_wing_pos = -1;			// wing position (0-5) in wingman status gauge

	uint	respawn_count = 0;						// number of respawns for this object.  Applies only to player wing ships in multiplayer
	int	respawn_priority = 0;					// priority this ship has for controlling respawn points

	int		alt_type_index = -1;					// optional alt type index
	int		callsign_index = -1;					// optional callsign index

	float ship_max_hull_strength = 0.0f;			// Needed to deal with special hitpoints
	float ship_max_shield_strength = 0.0f;
	float max_shield_recharge = 0.0f;

	// Goober5000
	SCP_vector<texture_replace> replacement_textures;

	SCP_vector<alt_class> alt_classes;	
	SCP_map<std::pair<int, int>, int> alt_iff_color;

	~p_object();

	const char* get_display_name();
	bool has_display_name();
};

// Goober5000 - this is now dynamic
extern SCP_vector<p_object> Parse_objects;
#define POBJ_INDEX(pobjp) (int)(pobjp - &Parse_objects[0])	// yes, this arithmetic is valid :D

extern p_object Support_ship_pobj, *Arriving_support_ship;
extern p_object Ship_arrival_list;

typedef struct team_data {
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
	bool    do_not_validate;
	int		weaponry_pool[MAX_WEAPON_TYPES];
	int		weaponry_count[MAX_WEAPON_TYPES];
	char	weaponry_pool_variable[MAX_WEAPON_TYPES][TOKEN_LENGTH];
	char	weaponry_amount_variable[MAX_WEAPON_TYPES][TOKEN_LENGTH];
	bool	weapon_required[MAX_WEAPON_TYPES];
} team_data;

#define MAX_P_WINGS		16
#define MAX_SHIP_LIST	16

extern team_data Team_data[MAX_TVT_TEAMS];
extern int Num_teams;

extern subsys_status *Subsys_status;
extern int Subsys_index;

extern vec3d Parse_viewer_pos;
extern matrix Parse_viewer_orient;

extern fix Mission_end_time;

extern SCP_vector<SCP_string> Parse_names;

extern char			Player_start_shipname[NAME_LENGTH];
extern int			Player_start_shipnum;
extern p_object	*Player_start_pobject;

extern int Mission_palette;  // index of palette file to use for mission
extern int Nebula_index;  // index into Nebula_filenames[] of nebula to use in mission.
extern const char *Nebula_filenames[NUM_NEBULAS];
extern const char *Nebula_colors[NUM_NEBULA_COLORS];
extern p_object *Arriving_support_ship;

extern char Neb2_texture_name[MAX_FILENAME_LEN];


void mission_init(mission *pm);
bool parse_main(const char *mission_name, int flags = 0);
p_object *mission_parse_get_arrival_ship(ushort net_signature);
p_object *mission_parse_get_arrival_ship(const char *name);
bool mission_check_ship_yet_to_arrive(const char *name);
p_object *mission_parse_find_parse_object(const char *name);
int parse_create_object(p_object *objp, bool standalone_ship = false);
void resolve_parse_flags(object *objp, flagset<Mission::Parse_Object_Flags> &parse_flags);

void mission_parse_close();

// used in fred management.cpp when creating a new mission
void apply_default_custom_data(mission* pm);

bool mission_maybe_make_ship_arrive(p_object *p_objp, bool force_arrival = false);
bool mission_maybe_make_wing_arrive(int wingnum, bool force_arrival = false);

// used in squadmate messaging stuff to create wings from reinforcements.
int parse_wing_create_ships(wing *wingp, int num_to_create, bool force_create = false, bool force_arrival = false, int specific_instance = -1 );

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

// code to bring in a repair ship.
void mission_bring_in_support_ship( object *requester_objp );
int mission_is_support_ship_arriving( void );
void mission_add_to_arriving_support( object *requester_objp );
int mission_is_repair_scheduled( object *objp );
int mission_remove_scheduled_repair( object *objp );
void mission_parse_support_arrived( int objnum );

// alternate name stuff
int mission_parse_lookup_alt(const char *name);
const char *mission_parse_lookup_alt_index(int index);
int mission_parse_add_alt(const char *name);
void mission_parse_remove_alt(const char *name);
void mission_parse_reset_alt();
void mission_process_alt_types();

// callsign stuff
int mission_parse_lookup_callsign(const char *name);
const char *mission_parse_lookup_callsign_index(int index);
int mission_parse_add_callsign(const char *name);
void mission_parse_remove_callsign(const char *name);
void mission_parse_reset_callsign();

// is training mission
int is_training_mission();

// code to save/restore mission parse stuff
int get_mission_info(const char *filename, mission *missionp = nullptr, bool basic = true, bool filename_is_full_path = false);

// Goober5000
void parse_dock_one_docked_object(p_object *pobjp, p_object *parent_pobjp);

// Karajorma
void swap_parse_object(p_object *p_obj, int ship_class);
void clear_texture_replacements();

// Goober5000
subsys_status *parse_get_subsys_status(p_object *pobjp, const char *subsys_name);

// MjnMixael
mission_custom_string* get_custom_string_by_name(SCP_string name);

#endif


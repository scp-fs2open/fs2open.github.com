/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <csetjmp>


#include "ai/aigoals.h"
#include "ai/ailua.h"
#include "asteroid/asteroid.h"
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "debris/debris.h"
#include "gamesnd/eventmusic.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "hud/hud.h"
#include "hud/hudescort.h"
#include "hud/hudets.h"
#include "hud/hudsquadmsg.h"
#include "hud/hudwingmanstatus.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "localization/localize.h"
#include "math/bitarray.h"
#include "math/fvi.h"
#include "math/staticrand.h"
#include "mission/missionbriefcommon.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionhotkey.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "missionui/fictionviewer.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/redalert.h"
#include "mod_table/mod_table.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "network/multi.h"
#include "network/multi_endgame.h"
#include "network/multi_respawn.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "object/objectdock.h"
#include "object/parseobjectdock.h"
#include "object/objectshield.h"
#include "object/waypoint.h"
#include "parse/generic_log.h"
#include "parse/parselo.h"
#include "parse/sexp_container.h"
#include "scripting/global_hooks.h"
#include "scripting/hook_api.h"
#include "scripting/hook_conditions.h"
#include "scripting/scripting.h"
#include "species_defs/species_defs.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "popup/popupdead.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "sound/ds.h"
#include "starfield/nebula.h"
#include "starfield/starfield.h"
#include "weapon/weapon.h"
#include "tracing/Monitor.h"
#include "missionparse.h"


// MISSION_VERSION should be the earliest version of FSO that can load the current mission format without
// requiring version-specific comments.  It should be updated whenever the format changes, but it should
// not be updated simply because the engine's version changed.
// NOTE: The version can only have two numbers because old FRED builds expect the version to be a float.
const gameversion::version MISSION_VERSION = gameversion::version(23, 1);
const gameversion::version LEGACY_MISSION_VERSION = gameversion::version(0, 10);

LOCAL struct {
	char docker[NAME_LENGTH];
	char dockee[NAME_LENGTH];
	char docker_point[NAME_LENGTH];
	char dockee_point[NAME_LENGTH];
} Initially_docked[MAX_SHIPS];

int Total_initially_docked;

mission	The_mission;
char Mission_filename[80];

int Mission_palette;  // index into Nebula_palette_filenames[] of palette file to use for mission
int Nebula_index;  // index into Nebula_filenames[] of nebula to use in mission.
int Num_ai_behaviors = MAX_AI_BEHAVIORS;
int Num_cargo = 0;
int Num_arrival_names = MAX_ARRIVAL_NAMES;
int Num_goal_type_names = MAX_GOAL_TYPE_NAMES;
int Num_parse_goals;
int Player_starts = 1;
int Num_teams;
fix Entry_delay_time = 0;

int Num_unknown_ship_classes;
int Num_unknown_weapon_classes;
int Num_unknown_loadout_classes;

ushort Current_file_checksum = 0;
ushort Last_file_checksum = 0;
int    Current_file_length   = 0;

SCP_vector<mission_default_custom_data> Default_custom_data;

// alternate ship type names
char Mission_alt_types[MAX_ALT_TYPE_NAMES][NAME_LENGTH];
int Mission_alt_type_count = 0;

// callsigns
char Mission_callsigns[MAX_CALLSIGNS][NAME_LENGTH];
int Mission_callsign_count = 0;

#define SHIP_WARP_TIME 5.0f		// how many seconds it takes for ship to warp in

// the ship arrival list will contain a list of ships that are yet to arrive.  This
// list could also include ships that are part of wings!
p_object Ship_arrival_list;	// for linked list of ships to arrive later

// all the ships that we parse
SCP_vector<p_object> Parse_objects;


// list for arriving support ship
p_object	Support_ship_pobj;
p_object *Arriving_support_ship;
char Arriving_repair_targets[MAX_AI_GOALS][NAME_LENGTH];
int Num_arriving_repair_targets;

#define MIN_SUBSYS_STATUS_SIZE		25
subsys_status *Subsys_status = NULL;
int Subsys_index;
int Subsys_status_size;

char Mission_parse_storm_name[NAME_LENGTH] = "none";

team_data Team_data[MAX_TVT_TEAMS];

// variables for player start in single player
char		Player_start_shipname[NAME_LENGTH];
int		Player_start_shipnum;
p_object *Player_start_pobject;

// name of all ships to use while parsing a mission (since a ship might be referenced by
// something before that ship has even been loaded yet)
SCP_vector<SCP_string> Parse_names;

SCP_vector<texture_replace> Fred_texture_replacements;

SCP_unordered_set<int> Fred_migrated_immobile_ships;

int Num_path_restrictions;
path_restriction_t Path_restrictions[MAX_PATH_RESTRICTIONS];

extern int debrief_find_persona_index();

//XSTR:OFF

const char *Nebula_filenames[NUM_NEBULAS] = {
	"Nebula01",
	"Nebula02",
	"Nebula03"	
};

const char *Neb2_filenames[NUM_NEBULAS] = {
	"Nebfull01",
	"Nebfull02",
	"Nebfull03"
};

// Note: Nebula_colors[] and Nebula_palette_filenames are linked via index numbers
const char *Nebula_colors[NUM_NEBULA_COLORS] = {
	"Red",
	"Blue",
	"Gold",
	"Purple",
	"Maroon",
	"Green",
	"Grey blue",
	"Violet",
	"Grey Green",
};

const char *Ai_behavior_names[MAX_AI_BEHAVIORS] = {
	"Chase",
	"Evade",
	"Get behind",
	"Stay Near",
	"Still",
	"Guard",
	"Avoid",
	"Waypoints",
	"Dock",
	"None",
	"Big Ship",
	"Path",
	"Be Rearmed",
	"Safety",
	"Evade Weapon",
	"Strafe",
	"Play Dead",
	"Bay Emerge",
	"Bay Depart",
	"Sentry Gun",
	"Warp Out",
	"Fly To Ship",
	"Lua AI",
};

char *Cargo_names[MAX_CARGO];
char Cargo_names_buf[MAX_CARGO][NAME_LENGTH];

const char *Ship_class_names[MAX_SHIP_CLASSES];		// to be filled in from Ship_info array

const char *Icon_names[MIN_BRIEF_ICONS] = {
	"Fighter", "Fighter Wing", "Cargo", "Cargo Wing", "Largeship",
	"Largeship Wing", "Capital", "Planet", "Asteroid Field", "Waypoint",
	"Support Ship", "Freighter(no cargo)", "Freighter(has cargo)",
	"Freighter Wing(no cargo)", "Freighter Wing(has cargo)", "Installation",
	"Bomber", "Bomber Wing", "Cruiser", "Cruiser Wing", "Unknown", "Unknown Wing",
	"Player Fighter", "Player Fighter Wing", "Player Bomber", "Player Bomber Wing",
	"Knossos Device", "Transport Wing", "Corvette", "Gas Miner", "Awacs", "Supercap", "Sentry Gun", "Jump Node", "Transport"
};

// definitions for arrival locations for ships/wings
const char *Arrival_location_names[MAX_ARRIVAL_NAMES] = {
	"Hyperspace", "Near Ship", "In front of ship", "In back of ship", "Above ship", "Below ship", "To left of ship", "To right of ship", "Docking Bay",
};

const char *Departure_location_names[MAX_DEPARTURE_NAMES] = {
	"Hyperspace", "Docking Bay",
};

const char *Goal_type_names[MAX_GOAL_TYPE_NAMES] = {
	"Primary", "Secondary", "Bonus",
};

const char *Reinforcement_type_names[] = {
	"Attack/Protect",
	"Repair/Rearm",
};

const char *Old_game_types[OLD_MAX_GAME_TYPES] = {
	"Single Player Only",	
	"Multiplayer Only",
	"Single/Multi Player",
	"Training mission"
};

flag_def_list_new<Mission::Parse_Object_Flags> Parse_object_flags[] = {
    { "cargo-known",					Mission::Parse_Object_Flags::SF_Cargo_known,			true, false },
    { "ignore-count",					Mission::Parse_Object_Flags::SF_Ignore_count,			true, false },
    { "protect-ship",					Mission::Parse_Object_Flags::OF_Protected,				true, false },
    { "reinforcement",					Mission::Parse_Object_Flags::SF_Reinforcement,			true, false },
    { "no-shields",						Mission::Parse_Object_Flags::OF_No_shields,				true, false },
    { "escort",							Mission::Parse_Object_Flags::SF_Escort,					true, false },
    { "player-start",					Mission::Parse_Object_Flags::OF_Player_start,			true, false },
    { "no-arrival-music",				Mission::Parse_Object_Flags::SF_No_arrival_music,		true, false },
    { "no-arrival-warp",				Mission::Parse_Object_Flags::SF_No_arrival_warp,		true, false },
    { "no-departure-warp",				Mission::Parse_Object_Flags::SF_No_departure_warp,		true, false },
    { "locked",							Mission::Parse_Object_Flags::SF_Locked,					true, false },
    { "invulnerable",					Mission::Parse_Object_Flags::OF_Invulnerable,			true, false },
    { "hidden-from-sensors",			Mission::Parse_Object_Flags::SF_Hidden_from_sensors,	true, false },
    { "scannable",						Mission::Parse_Object_Flags::SF_Scannable,				true, false },
    { "kamikaze",						Mission::Parse_Object_Flags::AIF_Kamikaze,				true, false },
    { "no-dynamic",						Mission::Parse_Object_Flags::AIF_No_dynamic,			true, false },
    { "red-alert-carry",				Mission::Parse_Object_Flags::SF_Red_alert_store_status, true, false },
    { "beam-protect-ship",				Mission::Parse_Object_Flags::OF_Beam_protected,			true, false },
    { "flak-protect-ship",				Mission::Parse_Object_Flags::OF_Flak_protected,			true, false },
    { "laser-protect-ship",				Mission::Parse_Object_Flags::OF_Laser_protected,		true, false },
    { "missile-protect-ship",			Mission::Parse_Object_Flags::OF_Missile_protected,		true, false },
    { "guardian",						Mission::Parse_Object_Flags::SF_Guardian,				true, false },
    { "special-warp",					Mission::Parse_Object_Flags::Knossos_warp_in,			true, false },
    { "vaporize",						Mission::Parse_Object_Flags::SF_Vaporize,				true, false },
    { "stealth",						Mission::Parse_Object_Flags::SF_Stealth,				true, false },
    { "friendly-stealth-invisible",		Mission::Parse_Object_Flags::SF_Friendly_stealth_invis, true, false },
    { "don't-collide-invisible",		Mission::Parse_Object_Flags::SF_Dont_collide_invis,		true, false },
    { "primitive-sensors",				Mission::Parse_Object_Flags::SF_Primitive_sensors,		true, false },
    { "no-subspace-drive",				Mission::Parse_Object_Flags::SF_No_subspace_drive,		true, false },
    { "nav-carry-status",				Mission::Parse_Object_Flags::SF_Nav_carry_status,		true, false },
    { "affected-by-gravity",			Mission::Parse_Object_Flags::SF_Affected_by_gravity,	true, false },
    { "toggle-subsystem-scanning",		Mission::Parse_Object_Flags::SF_Toggle_subsystem_scanning, true, false },
    { "targetable-as-bomb",				Mission::Parse_Object_Flags::OF_Targetable_as_bomb,		true, false },
    { "no-builtin-messages",			Mission::Parse_Object_Flags::SF_No_builtin_messages,	true, false },
    { "primaries-locked",				Mission::Parse_Object_Flags::SF_Primaries_locked,		true, false },
    { "secondaries-locked",				Mission::Parse_Object_Flags::SF_Secondaries_locked,		true, false },
    { "no-death-scream",				Mission::Parse_Object_Flags::SF_No_death_scream,		true, false },
    { "always-death-scream",			Mission::Parse_Object_Flags::SF_Always_death_scream,	true, false },
    { "nav-needslink",					Mission::Parse_Object_Flags::SF_Nav_needslink,			true, false },
    { "hide-ship-name",					Mission::Parse_Object_Flags::SF_Hide_ship_name,			true, false },
    { "set-class-dynamically",			Mission::Parse_Object_Flags::SF_Set_class_dynamically,	true, false },
    { "lock-all-turrets",				Mission::Parse_Object_Flags::SF_Lock_all_turrets_initially, true, false },
    { "afterburners-locked",			Mission::Parse_Object_Flags::SF_Afterburner_locked,		true, false },
    { "force-shields-on",				Mission::Parse_Object_Flags::OF_Force_shields_on,		true, false },
    { "immobile",						Mission::Parse_Object_Flags::OF_Immobile,				true, false },
    { "don't-change-position",			Mission::Parse_Object_Flags::OF_Dont_change_position,	true, false },
    { "don't-change-orientation",		Mission::Parse_Object_Flags::OF_Dont_change_orientation,	true, false },
    { "no-ets",							Mission::Parse_Object_Flags::SF_No_ets,					true, false },
    { "cloaked",						Mission::Parse_Object_Flags::SF_Cloaked,				true, false },
    { "ship-locked",					Mission::Parse_Object_Flags::SF_Ship_locked,			true, false },
    { "weapons-locked",					Mission::Parse_Object_Flags::SF_Weapons_locked,			true, false },
    { "scramble-messages",				Mission::Parse_Object_Flags::SF_Scramble_messages,		true, false },
    { "no_collide",						Mission::Parse_Object_Flags::OF_No_collide,				true, false },
    { "no-disabled-self-destruct",		Mission::Parse_Object_Flags::SF_No_disabled_self_destruct, true, false },
    { "hide-in-mission-log",			Mission::Parse_Object_Flags::SF_Hide_mission_log,		true, false },
    { "same-arrival-warp-when-docked",		Mission::Parse_Object_Flags::SF_Same_arrival_warp_when_docked,		true, false },
    { "same-departure-warp-when-docked",	Mission::Parse_Object_Flags::SF_Same_departure_warp_when_docked,	true, false },
    { "ai-attackable-if-no-collide",		Mission::Parse_Object_Flags::OF_Attackable_if_no_collide, true, false },
    { "fail-sound-locked-primary",			Mission::Parse_Object_Flags::SF_Fail_sound_locked_primary, true, false },
    { "fail-sound-locked-secondary",		Mission::Parse_Object_Flags::SF_Fail_sound_locked_secondary, true, false },
    { "aspect-immune",						Mission::Parse_Object_Flags::SF_Aspect_immune, true, false },
	{ "cannot-perform-scan",			Mission::Parse_Object_Flags::SF_Cannot_perform_scan,	true, false },
	{ "no-targeting-limits",				Mission::Parse_Object_Flags::SF_No_targeting_limits, true, false},
};

parse_object_flag_description<Mission::Parse_Object_Flags> Parse_object_flag_descriptions[] = {
    { Mission::Parse_Object_Flags::SF_Cargo_known,					"If set, the ship's cargo can be seen without scanning the ship."},
    { Mission::Parse_Object_Flags::SF_Ignore_count,					"Ignore this ship when counting ship types for goals."},
    { Mission::Parse_Object_Flags::OF_Protected,					"Ship and Turret AI will ignore and not attack ship."},
    { Mission::Parse_Object_Flags::SF_Reinforcement,				"This ship is a reinforcement ship."},
    { Mission::Parse_Object_Flags::OF_No_shields,					"Ship will have no shields (ETS will be rebalanced if shields were off and are enabled)."},
    { Mission::Parse_Object_Flags::SF_Escort,						"This ship is an escort ship."},
    { Mission::Parse_Object_Flags::OF_Player_start,					"Ship is a player ship."},
    { Mission::Parse_Object_Flags::SF_No_arrival_music,				"Don't play arrival music when ship arrives."},
    { Mission::Parse_Object_Flags::SF_No_arrival_warp,				"No arrival warp-in effect."},
    { Mission::Parse_Object_Flags::SF_No_departure_warp,			"No departure warp-in effect."},
    { Mission::Parse_Object_Flags::SF_Locked,						"Ship cannot be changed in the Ship Loadout."},
    { Mission::Parse_Object_Flags::OF_Invulnerable,					"Stops ship from taking any damage."},
    { Mission::Parse_Object_Flags::SF_Hidden_from_sensors,			"If set, the ship can't be targeted and appears on radar as a blinking dot."},
    { Mission::Parse_Object_Flags::SF_Scannable,					"Whether or not the ship can be scanned."},
    { Mission::Parse_Object_Flags::AIF_Kamikaze,					"Ship will attack big ships by colliding with them and exploding."},
    { Mission::Parse_Object_Flags::AIF_No_dynamic,					"Will stop allowing the AI to pursue dynamic goals (eg: chasing ships it was not ordered to)."},
    { Mission::Parse_Object_Flags::SF_Red_alert_store_status,		"Ship status should be stored/restored if red alert mission."},
    { Mission::Parse_Object_Flags::OF_Beam_protected,				"Turrets with beam weapons will ignore and not attack ship."},
    { Mission::Parse_Object_Flags::OF_Flak_protected,				"Turrets with flak weapons will ignore and not attack ship."},
    { Mission::Parse_Object_Flags::OF_Laser_protected,				"Turrets with laser weapons will ignore and not attack ship."},
    { Mission::Parse_Object_Flags::OF_Missile_protected,			"Turrets with missile weapons will ignore and not attack ship."},
    { Mission::Parse_Object_Flags::SF_Guardian,						"Ship health cannot be reduced below 1."},
    { Mission::Parse_Object_Flags::Knossos_warp_in,					"Ship uses the special Knossos warp-in animation."},
    { Mission::Parse_Object_Flags::SF_Vaporize,						"Causes a ship to vanish (no deathroll, no debris, no explosion) when destroyed."},
    { Mission::Parse_Object_Flags::SF_Stealth,						"If set, the ship can't be targeted, is invisible on radar, and is ignored by AI unless firing."},
    { Mission::Parse_Object_Flags::SF_Friendly_stealth_invis,		"If set and the ship is also flagged with stealth then the ship can't be targeted even by ships on the same team."},
    { Mission::Parse_Object_Flags::SF_Dont_collide_invis,			"Will cause polygons with an invisible texture to stop colliding with objects."},
    { Mission::Parse_Object_Flags::SF_Primitive_sensors,			"Targets will only be a blip on the radar. Ships cannot targeted and aspect lock cannot be used."},
    { Mission::Parse_Object_Flags::SF_No_subspace_drive,			"Will not allow a ship to jump into subspace."},
    { Mission::Parse_Object_Flags::SF_Nav_carry_status,				"This ship autopilots with the player."},
    { Mission::Parse_Object_Flags::SF_Affected_by_gravity,			"Deprecated. Does nothing."},
    { Mission::Parse_Object_Flags::SF_Toggle_subsystem_scanning,	"Switches between being able to scan a whole ship or individual subsystems."},
    { Mission::Parse_Object_Flags::OF_Targetable_as_bomb,			"Allows ship to be targeted with the bomb targeting key."},
    { Mission::Parse_Object_Flags::SF_No_builtin_messages,			"Ship will not send built-in messages."},
    { Mission::Parse_Object_Flags::SF_Primaries_locked,				"Will stop a ship from firing their primary weapons."},
    { Mission::Parse_Object_Flags::SF_Secondaries_locked,			"Will stop a ship from firing their secondary weapons."},
    { Mission::Parse_Object_Flags::SF_No_death_scream,				"Ship will not send a death message."},
    { Mission::Parse_Object_Flags::SF_Always_death_scream,			"Ship will always send a death message."},
    { Mission::Parse_Object_Flags::SF_Nav_needslink,				"This ship requires \"linking\" for autopilot ."},
    { Mission::Parse_Object_Flags::SF_Hide_ship_name,				"If set, the ship name can't be seen when the ship is targeted."},
    { Mission::Parse_Object_Flags::SF_Set_class_dynamically,		"This ship should have its class assigned rather than simply read from the mission file."},
    { Mission::Parse_Object_Flags::SF_Lock_all_turrets_initially,	"Lock all turrets on this ship at mission start or on arrival."},
    { Mission::Parse_Object_Flags::SF_Afterburner_locked,			"Will stop a ship from firing their afterburner."},
    { Mission::Parse_Object_Flags::OF_Force_shields_on,				"Shields will be activated regardless of other flags."},
    { Mission::Parse_Object_Flags::OF_Immobile,						"Will not let a ship change position or orientation. Upon destruction the ship will still do the death roll and explosion."},
    { Mission::Parse_Object_Flags::OF_Dont_change_position,			"Will not let a ship change position. Upon destruction the ship will still do the death roll and explosion."},
    { Mission::Parse_Object_Flags::OF_Dont_change_orientation,		"Will not let a ship change orientation. Upon destruction the ship will still do the death roll and explosion."},
    { Mission::Parse_Object_Flags::SF_No_ets,						"Will not allow a ship to alter its ETS system."},
    { Mission::Parse_Object_Flags::SF_Cloaked,						"This ship will not be rendered."},
    { Mission::Parse_Object_Flags::SF_Ship_locked,					"Prevents the player from changing the ship class on loadout screen."},
    { Mission::Parse_Object_Flags::SF_Weapons_locked,				"Prevents the player from changing the weapons on the ship on the loadout screen."},
    { Mission::Parse_Object_Flags::SF_Scramble_messages,			"All messages sent from this ship appear scrambled."},
    { Mission::Parse_Object_Flags::OF_No_collide,					"Ship cannot be collided with."},
    { Mission::Parse_Object_Flags::SF_No_disabled_self_destruct,	"Ship will not self-destruct after 90 seconds if engines or weapons destroyed."},
    { Mission::Parse_Object_Flags::SF_Hide_mission_log,				"Mission log events generated for this ship will not be viewable."},
    { Mission::Parse_Object_Flags::SF_Same_arrival_warp_when_docked,"Docked ships use the same warp effect size upon arrival as if they were not docked instead of the enlarged aggregate size."},
    { Mission::Parse_Object_Flags::SF_Same_departure_warp_when_docked,"Docked ship use the same warp effect size upon departure as if they were not docked instead of the enlarged aggregate size."},
    { Mission::Parse_Object_Flags::OF_Attackable_if_no_collide,		"Allows the AI to attack this object, even if no-collide is set."},
    { Mission::Parse_Object_Flags::SF_Fail_sound_locked_primary,	"Play the firing fail sound when the weapon is locked."},
    { Mission::Parse_Object_Flags::SF_Fail_sound_locked_secondary,	"Play the firing fail sound when the weapon is locked."},
    { Mission::Parse_Object_Flags::SF_Aspect_immune,				"Ship cannot be targeted by Aspect Seekers."},
	{ Mission::Parse_Object_Flags::SF_Cannot_perform_scan,			"Ship cannot scan other ships."},
	{ Mission::Parse_Object_Flags::SF_No_targeting_limits,			"Ship is always targetable regardless of AWACS or targeting range limits."},
};

const size_t Num_parse_object_flags = sizeof(Parse_object_flags) / sizeof(flag_def_list_new<Mission::Parse_Object_Flags>);

// These are only the flags that are saved to the mission file.  See the MEF_ #defines.
flag_def_list Mission_event_flags[] = {
	{ "interval & delay use msecs", MEF_USE_MSECS, 0 },
};
int Num_mission_event_flags = sizeof(Mission_event_flags) / sizeof(flag_def_list);

const char *Mission_event_log_flags[MAX_MISSION_EVENT_LOG_FLAGS] = {
	"true",
	"false",
	"always true",			// disabled
	"always false",
	"first repeat",
	"last repeat", 
	"first trigger",
	"last trigger",
	"state change",
};

//XSTR:ON

int Num_reinforcement_type_names = sizeof(Reinforcement_type_names) / sizeof(char *);

vec3d Parse_viewer_pos;
matrix Parse_viewer_orient;

int Loading_screen_bm_index=-1;

fix Mission_end_time;

// calculates a "unique" file signature as a ushort (checksum) and an int (file length)
// the amount of The_mission we're going to checksum
// WARNING : do NOT call this function on the server - it will overwrite goals, etc
#define MISSION_CHECKSUM_SIZE (NAME_LENGTH + NAME_LENGTH + 4 + DATE_TIME_LENGTH + DATE_TIME_LENGTH)

// a timer used to limit arrival music
#define ARRIVAL_MUSIC_MIN_SEPARATION	60000
// a random delay before announcing enemy arrivals
#define ARRIVAL_MESSAGE_DELAY_MIN		2000
#define ARRIVAL_MESSAGE_DELAY_MAX		3000

static int Allow_arrival_music_timestamp;
static int Allow_arrival_message_timestamp;
static int Arrival_message_delay_timestamp;
static int Arrival_message_subject;

static int Allow_backup_message_timestamp;

// multi TvT
static int Allow_arrival_music_timestamp_m[2];
static int Allow_arrival_message_timestamp_m[2];
static int Arrival_message_delay_timestamp_m[2];

// local prototypes
void parse_player_info2(mission *pm);
bool post_process_mission(mission *pm);
int allocate_subsys_status();
void parse_common_object_data(p_object	*objp);
void parse_asteroid_fields(mission *pm);
int mission_set_arrival_location(int anchor, ArrivalLocation location, int distance, int objnum, int path_mask, vec3d *new_pos, matrix *new_orient);
int get_anchor(const char *name);
void mission_parse_set_up_initial_docks();
void mission_parse_set_arrival_locations();
void mission_set_wing_arrival_location( wing *wingp, int num_to_set );
void parse_object_set_handled_flag_helper(p_object *pobjp, p_dock_function_info *infop);
void parse_object_clear_all_handled_flags();
int parse_object_on_arrival_list(p_object *pobjp);
int add_path_restriction();

static bool Warned_about_team_out_of_range;

// Goober5000
void mission_parse_mark_non_arrival(p_object *p_objp);
void mission_parse_mark_non_arrival(wing *wingp);
void mission_parse_mark_non_arrivals();

// Goober5000 - FRED import
void convertFSMtoFS2();


MONITOR(NumShipArrivals)
MONITOR(NumShipDepartures)

const std::shared_ptr<scripting::Hook<scripting::hooks::ShipDepartConditions>> OnDepartureStartedHook = scripting::Hook<scripting::hooks::ShipDepartConditions>::Factory(
	"On Departure Started", "Called when a ship starts the departure process.",
	{
		{"Self", "ship", "An alias for Ship."},
		{"Ship", "ship", "The ship that has begun the departure process."},
	});

 const std::shared_ptr<scripting::Hook<>> OnLoadoutAboutToParseHook = scripting::Hook<>::Factory("On Loadout About To Parse",
	"Called during mission load just before parsing the team loadout.",{});

custom_string* get_custom_string_by_name(SCP_string name)
 {
	for (size_t i = 0; i < The_mission.custom_strings.size(); i++) {
		if (The_mission.custom_strings[i].name == name) {
			return &The_mission.custom_strings[i];
		}
	}

	return nullptr;
}

// Goober5000
void parse_custom_bitmap(const char *expected_string_640, const char *expected_string_1024, char *string_field_640, char *string_field_1024)
{
	bool found640 = false, found1024 = false;

	// custom mission loading background, or whatever
	if (optional_string(expected_string_640))
	{
		found640 = true;
		stuff_string(string_field_640, F_NAME, MAX_FILENAME_LEN);
	}
	if (optional_string(expected_string_1024))
	{
		found1024 = true;
		stuff_string(string_field_1024, F_NAME, MAX_FILENAME_LEN);
	}

	// error testing
	if (Fred_running && (found640) && !(found1024))
	{
		mprintf(("Mission: found an entry for %s but not a corresponding entry for %s!\n", expected_string_640, expected_string_1024));
	}
	if (Fred_running && !(found640) && (found1024))
	{
		mprintf(("Mission: found an entry for %s but not a corresponding entry for %s!\n", expected_string_1024, expected_string_640));
	}
}

void parse_mission_info(mission *pm, bool basic = false)
{
	char game_string[NAME_LENGTH];

	// Goober5000
	skip_to_start_of_string("#Mission Info");

	required_string("#Mission Info");
	
	required_string("$Version:");
	pm->required_fso_version = gameversion::parse_version_inline();

	if (!gameversion::check_at_least(pm->required_fso_version))
		throw parse::VersionException("Mission requires version " + gameversion::format_version(pm->required_fso_version), pm->required_fso_version);

	required_string("$Name:");
	stuff_string(pm->name, F_NAME, NAME_LENGTH);

	required_string("$Author:");
	stuff_string(pm->author, F_NAME);

	required_string("$Created:");
	stuff_string(pm->created, F_DATE, DATE_TIME_LENGTH);

	required_string("$Modified:");
	stuff_string(pm->modified, F_DATE, DATE_TIME_LENGTH);

	required_string("$Notes:");
	stuff_string(pm->notes, F_NOTES, NOTES_LENGTH);

	if (optional_string("$Mission Desc:"))
		stuff_string(pm->mission_desc, F_MULTITEXT, MISSION_DESC_LENGTH);		

	if ( optional_string("+Game Type:")) {
		// HACK HACK HACK -- stuff_string was changed to *not* ignore carriage returns.  Since the
		// old style missions may have carriage returns, deal with it here.
		ignore_white_space();
		stuff_string(game_string, F_NAME, NAME_LENGTH);
		for ( int i = 0; i < OLD_MAX_GAME_TYPES; i++ ) {
			if ( !stricmp(game_string, Old_game_types[i]) ) {

				// this block of code is now old mission compatibility code.  We specify game
				// type in a different manner than before.
				if ( i == OLD_GAME_TYPE_SINGLE_ONLY )
					pm->game_type = MISSION_TYPE_SINGLE;
				else if ( i == OLD_GAME_TYPE_MULTI_ONLY )
					pm->game_type = MISSION_TYPE_MULTI;
				else if ( i == OLD_GAME_TYPE_SINGLE_MULTI )
					pm->game_type = (MISSION_TYPE_SINGLE | MISSION_TYPE_MULTI );
				else if ( i == OLD_GAME_TYPE_TRAINING )
					pm->game_type = MISSION_TYPE_TRAINING;
				else
					Int3();

				if ( pm->game_type & MISSION_TYPE_MULTI )
					pm->game_type |= MISSION_TYPE_MULTI_COOP;

				break;
			}
		}
	}

	if ( optional_string("+Game Type Flags:") ) {
		stuff_int(&pm->game_type);
	}

	// multiplayer team v. team games have two teams.  If we have three teams, we need to use
	// a new mission mode!
	if ( (pm->game_type & MISSION_TYPE_MULTI) && (pm->game_type & MISSION_TYPE_MULTI_TEAMS) ){
		Num_teams = 2;
	}

	if (optional_string("+Flags:")){
        stuff_flagset(&pm->flags);
	}

	// nebula mission stuff
	if(optional_string("+NebAwacs:")){
		stuff_float(&Neb2_awacs);
	}
	if(optional_string("+Storm:")){
		stuff_string(Mission_parse_storm_name, F_NAME, NAME_LENGTH);

		if (!basic)
			nebl_set_storm(Mission_parse_storm_name);
	}
	if(optional_string("+Fog Near Mult:")){
		stuff_float(&Neb2_fog_near_mult);
	}
	if(optional_string("+Fog Far Mult:")){
		stuff_float(&Neb2_fog_far_mult);
	}

	if (optional_string("+Volumetric Nebula:")) {
		pm->volumetrics.emplace().parse_volumetric_nebula();
	}

	// Goober5000 - ship contrail speed threshold
	if (optional_string("$Contrail Speed Threshold:")){
		stuff_int(&pm->contrail_threshold);
	}

	// get the number of players if in a multiplayer mission
	if ( pm->game_type & MISSION_TYPE_MULTI ) {
		if ( optional_string("+Num Players:") ) {
			stuff_int( &(pm->num_players) );
		}
	}

	// get the number of respawns
	if ( pm->game_type & MISSION_TYPE_MULTI ) {
		if ( optional_string("+Num Respawns:") ){
			stuff_int( (int*)&(pm->num_respawns) );
		}
	}

	if ( pm->game_type & MISSION_TYPE_MULTI ) {
		if ( optional_string("+Max Respawn Time:") ){
			stuff_int( &The_mission.max_respawn_delay );
		}
	}

	if ( optional_string("+Red Alert:")) {
		int temp;
		stuff_int(&temp);

        pm->flags.set(Mission::Mission_Flags::Red_alert, temp != 0);    
    } 

	if ( optional_string("+Scramble:")) {
		int temp;
		stuff_int(&temp);

        pm->flags.set(Mission::Mission_Flags::Scramble, temp != 0);
	}

	// if we are just requesting basic info then skip everything else.  the reason
	// for this is to make sure that we don't modify things outside of the mission struct
	// that might not get reset afterwards (like what can happen in the techroom) - taylor
	//
	// NOTE: this can be dangerous so be sure that any get_mission_info() call (defaults to basic info) will
	//       only reference data parsed before this point!! (like current FRED2 and game code does)
	if (basic)
		return;

	if ( optional_string("+Disallow Support:"))
	{
		int temp;
		stuff_int(&temp);

		pm->support_ships.max_support_ships = (temp > 0) ? 0 : -1;
	}

	if ( optional_string("+Hull Repair Ceiling:"))
	{
		float temp;
		stuff_float(&temp);

		//ONLY set max_hull_repair_val if the value given is valid -C
		if (temp <= 100.0f && temp >= 0.0f) {
			pm->support_ships.max_hull_repair_val = temp;
		}
	}

	if ( optional_string("+Subsystem Repair Ceiling:"))
	{
		float temp;
		stuff_float(&temp);

		//ONLY set max_subsys_repair_val if the value given is valid -C
		if (temp <= 100.0f && temp >= 0.0f) {
			pm->support_ships.max_subsys_repair_val = temp;
		}
	}

	if (optional_string("+All Teams Attack")){
		Mission_all_attack = 1;
	}

	//	Maybe delay the player's entry.
	if (optional_string("+Player Entry Delay:")) {
		float	temp;
		
		stuff_float(&temp);
		Assert(temp >= 0.0f);
		Entry_delay_time = fl2f(temp);
	}

	if (optional_string("+Viewer pos:")){
		stuff_vec3d(&Parse_viewer_pos);
	}

	if (optional_string("+Viewer orient:")){
		stuff_matrix(&Parse_viewer_orient);
	}

	// possible squadron reassignment (only in single player)
	if(optional_string("+SquadReassignName:")){
		char buf[std::max(NAME_LENGTH, MAX_FILENAME_LEN)];

		stuff_string(buf, F_NAME, NAME_LENGTH);
		if (!(Game_mode & GM_MULTIPLAYER))
			strcpy_s(pm->squad_name, buf);

		if(optional_string("+SquadReassignLogo:")){
			stuff_string(buf, F_NAME, MAX_FILENAME_LEN);
			if (!(Game_mode & GM_MULTIPLAYER))
				strcpy_s(pm->squad_filename, buf);
		}
	}

	// wing stuff by Goober5000 ------------------------------------------
	// the wing name arrays are initialized in ship_level_init
	if (optional_string("$Starting wing names:"))
	{
		stuff_string_list(Starting_wing_names, MAX_STARTING_WINGS);
	}

	if (optional_string("$Squadron wing names:"))
	{
		stuff_string_list(Squadron_wing_names, MAX_SQUADRON_WINGS);
	}

	if (optional_string("$Team-versus-team wing names:"))
	{
		stuff_string_list(TVT_wing_names, MAX_TVT_WINGS);
	}
	// end of wing stuff -------------------------------------------------


	// Goober5000 - made this into a function since we use much the same technique for the briefing background
	parse_custom_bitmap("$Load Screen 640:", "$Load Screen 1024:", pm->loading_screen[GR_640], pm->loading_screen[GR_1024]);

	if (optional_string("$Skybox Model:"))
	{
		stuff_string(pm->skybox_model, F_NAME, MAX_FILENAME_LEN);

		if (optional_string("$Skybox Model Animations:")) {
			animation::ModelAnimationParseHelper::parseAnimsetInfo(pm->skybox_model_animations, 'b', pm->name);
		}
		if (optional_string("$Skybox Model Moveables:")) {
			animation::ModelAnimationParseHelper::parseMoveablesetInfo(pm->skybox_model_animations);
		}
	}

	if (optional_string("+Skybox Orientation:"))
	{
		stuff_matrix(&pm->skybox_orientation);
	}

	if (optional_string("+Skybox Flags:")){
		pm->skybox_flags = 0;
		stuff_int(&pm->skybox_flags); 
	}

	// Goober5000 - AI on a per-mission basis
	if (optional_string("$AI Profile:"))
	{
		int index;
		char temp[NAME_LENGTH];

		stuff_string(temp, F_NAME, NAME_LENGTH);
		index = ai_profile_lookup(temp);

		if (index >= 0)
			The_mission.ai_profile = &Ai_profiles[index];
		else
			WarningEx(LOCATION, "Mission: %s\nUnknown AI profile %s!", pm->name, temp );
	}

	if (optional_string("$Lighting Profile:"))
	{
		stuff_string(The_mission.lighting_profile_name, F_NAME);
	}
	else
		The_mission.lighting_profile_name = lighting_profiles::default_name();
	lighting_profiles::switch_to(The_mission.lighting_profile_name);

	if (optional_string("$Sound Environment:")) {
		char preset[65] = { '\0' };
		stuff_string(preset, F_NAME, sizeof(preset)-1);

		int preset_id = ds_eax_get_preset_id(preset);

		if (preset_id >= 0) {
			sound_env_get(&pm->sound_environment, preset_id);
		}

		// NOTE: values will be clamped properly when the effect is actually set

		if (optional_string("+Volume:")) {
			stuff_float(&pm->sound_environment.volume);
		}

		if (optional_string("+Damping:")) {
			stuff_float(&pm->sound_environment.damping);
		}

		if (optional_string("+Decay Time:")) {
			stuff_float(&pm->sound_environment.decay);
		}
	}

	float gravity_accel = 0.0f;
	if (optional_string("$Gravity Acceleration:")) {
		stuff_float(&gravity_accel);
		pm->gravity = vm_vec_new(0.0f, -gravity_accel, 0.0f);
	}
}

void parse_player_info(mission *pm)
{
	char temp[NAME_LENGTH];
	Assert(pm != NULL);

	// alternate type names begin here
	if(optional_string("#Alternate Types:")){		
		// read them all in
		while(!optional_string("#end")){
			required_string("$Alt:");
			stuff_string(temp, F_NAME, NAME_LENGTH);

			// maybe store it
			mission_parse_add_alt(temp);			
		}
	}
	
	// callsigns begin here
	if(optional_string("#Callsigns:")){		
		// read them all in
		while(!optional_string("#end")){
			required_string("$Callsign:");
			stuff_string(temp, F_NAME, NAME_LENGTH);

			// maybe store it
			mission_parse_add_callsign(temp);			
		}
	}
	
	required_string("#Players");

	// starting general orders go here
	ai_lua_reset_general_orders();

	if (optional_string("+General Orders Enabled:")) {
		SCP_vector<SCP_string> accepted_flags;
		stuff_string_list(accepted_flags);

		for (const SCP_string& accepted : accepted_flags) {
			int lua_order_id = ai_lua_find_general_order_id(accepted);

			if (lua_order_id >= 0) {
				ai_lua_enable_general_order(lua_order_id, true);
			}
		}
	}
	if (optional_string("+General Orders Valid:")) {
		SCP_vector<SCP_string> accepted_flags;
		stuff_string_list(accepted_flags);

		for (const SCP_string& accepted : accepted_flags) {
			int lua_order_id = ai_lua_find_general_order_id(accepted);

			if (lua_order_id >= 0) {
				ai_lua_validate_general_order(lua_order_id, true);
			}
		}
	}

	while (required_string_either("#Objects", "$")){
		parse_player_info2(pm);
	}
}

void parse_player_info2(mission *pm)
{
	int nt, i;
	SCP_vector<loadout_row> list, list2;
	team_data *ptr;

	if (OnLoadoutAboutToParseHook->isActive()) {
		OnLoadoutAboutToParseHook->run();
	}

	// read in a ship/weapon pool for each team.
	for ( nt = 0; nt < Num_teams; nt++ ) {
		int num_choices;

		ptr = &Team_data[nt];
		// get the shipname for single player missions
		// MWA -- make this required later!!!!
		if ( optional_string("$Starting Shipname:") )
			stuff_string( Player_start_shipname, F_NAME, NAME_LENGTH );

		if (optional_string("+Do Not Validate Loadout")) {
			ptr->do_not_validate = true;
		} else {
			ptr->do_not_validate = false;
		}

		required_string("$Ship Choices:");
		stuff_loadout_list(list, MISSION_LOADOUT_SHIP_LIST);

		num_choices = 0;

		// check ship class loadout entries
		for (auto &sc : list) {
			// in a campaign, see if the player is allowed the ships or not.  Remove them from the
			// pool if they are not allowed
			if (Game_mode & GM_CAMPAIGN_MODE || (MULTIPLAYER_CLIENT)) {
				if ( !Campaign.ships_allowed[sc.index] )
					continue;
			}
			if (sc.index < 0 || sc.index >= ship_info_size())
				continue;

			ptr->ship_list[num_choices] = sc.index;

			// if the list isn't set by a variable leave the variable name empty
			if (sc.index_sexp_var == NOT_SET_BY_SEXP_VARIABLE) {
				strcpy_s(ptr->ship_list_variables[num_choices], "") ;
			}
			else {
				strcpy_s(ptr->ship_list_variables[num_choices], Sexp_variables[sc.index_sexp_var].variable_name);
			}

			ptr->ship_count[num_choices] = sc.count;
			ptr->loadout_total += sc.count;

			// if the list isn't set by a variable leave the variable name empty
			if (sc.count_sexp_var == NOT_SET_BY_SEXP_VARIABLE) {
				strcpy_s(ptr->ship_count_variables[num_choices], "");
			}
			else {
				strcpy_s(ptr->ship_count_variables[num_choices], Sexp_variables[sc.count_sexp_var].variable_name);
			}

			num_choices++;
		}
		ptr->num_ship_choices = num_choices;

		ptr->default_ship = -1;
		if (optional_string("+Default_ship:")) {
			char str[NAME_LENGTH];
			stuff_string(str, F_NAME, NAME_LENGTH);
			ptr->default_ship = ship_info_lookup(str);
			if (-1 == ptr->default_ship) {
				WarningEx(LOCATION, "Mission: %s\nUnknown default ship %s!  Defaulting to %s.", pm->name, str, Ship_info[ptr->ship_list[0]].name );
				ptr->default_ship = ptr->ship_list[0]; // default to 1st in list
			}
			// see if the player's default ship is an allowable ship (campaign only). If not, then what
			// do we do?  choose the first allowable one?
			if (Game_mode & GM_CAMPAIGN_MODE || (MULTIPLAYER_CLIENT)) {
				if ( !(Campaign.ships_allowed[ptr->default_ship]) ) {
					for (i = 0; i < ship_info_size(); i++ ) {
						if ( Campaign.ships_allowed[i] ) {
							ptr->default_ship = i;
							break;
						}
					}
					Assertion( i < ship_info_size(), "Mission: %s: Could not find a valid default ship.\n", pm->name );
				}
			}
		}

		if (ptr->default_ship == -1)  // invalid or not specified, make first in list
			ptr->default_ship = ptr->ship_list[0];

		required_string("+Weaponry Pool:");
		stuff_loadout_list(list2, MISSION_LOADOUT_WEAPON_LIST);

		num_choices = 0;

		// check weapon class loadout entries
		for (auto &wc : list2) {
			// in a campaign, see if the player is allowed the weapons or not.  Remove them from the
			// pool if they are not allowed
			if (Game_mode & GM_CAMPAIGN_MODE || (MULTIPLAYER_CLIENT)) {
				if ( !Campaign.weapons_allowed[wc.index] ) {
					continue;
				}
			}
			if (wc.index < 0 || wc.index >= weapon_info_size())
				continue;

			// always allow the pool to be added in FRED, it is a verbal warning
			// to let the mission dev know about the problem
			if ( !(Weapon_info[wc.index].wi_flags[Weapon::Info_Flags::Player_allowed]) && !Fred_running ) {
				WarningEx(LOCATION, "Weapon '%s' in weapon pool isn't allowed on player loadout! Ignoring it ...\n", Weapon_info[wc.index].name);
				continue;
			}

			ptr->weaponry_pool[num_choices] = wc.index; 
			ptr->weaponry_count[num_choices] = wc.count;

			// if the list isn't set by a variable leave the variable name empty
			if (wc.index_sexp_var == NOT_SET_BY_SEXP_VARIABLE) {
				strcpy_s(ptr->weaponry_pool_variable[num_choices], "");
			}
			else {
				strcpy_s(ptr->weaponry_pool_variable[num_choices], Sexp_variables[wc.index_sexp_var].variable_name);
			}

			// if the list isn't set by a variable leave the variable name empty
			if (wc.count_sexp_var == NOT_SET_BY_SEXP_VARIABLE) {
				strcpy_s(ptr->weaponry_amount_variable[num_choices], "");
			}
			else {
				strcpy_s(ptr->weaponry_amount_variable[num_choices], Sexp_variables[wc.count_sexp_var].variable_name);
			}

			num_choices++; 
		}
		ptr->num_weapon_choices = num_choices;

		memset(ptr->weapon_required, 0, MAX_WEAPON_TYPES * sizeof(bool));
		if (optional_string("+Required for mission:"))
		{
			int num_weapons;
			int weapon_list_buf[MAX_WEAPON_TYPES];
			num_weapons = (int)stuff_int_list(weapon_list_buf, MAX_WEAPON_TYPES, WEAPON_LIST_TYPE);

			for (i = 0; i < num_weapons; i++)
				ptr->weapon_required[weapon_list_buf[i]] = true;
		}
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough ship/weapon pools for mission.  There are %d teams and only %d pools.", Num_teams, nt);
}

void parse_cutscenes(mission *pm) 
{
	if (optional_string("#Cutscenes"))
	{
		mission_cutscene scene;

		while (!optional_string("#end"))
		{
			// this list should correspond to the MOVIE_* enums
			scene.type = optional_string_one_of(7,
				"$Fiction Viewer Cutscene:",
				"$Command Brief Cutscene:",
				"$Briefing Cutscene:",
				"$Pre-game Cutscene:",
				"$Debriefing Cutscene:",
				"$Post-debriefing Cutscene:",
				"$Campaign End Cutscene:");

			// Didn't find one of the valid cutscene types
			if (scene.type < 0) {

				// $Post-briefing cutscene was added as an alias of $Pre-game cutscene when
				// this section was only editable by hand. Now that there's a dialog that explains
				// what each option is, we can drop the alias and quietly convert it if it's found
				// for backwards compatibility. Log print just in case.
				if (optional_string("$Post-briefing Cutscene:")) {
					scene.type = MOVIE_PRE_GAME;
					mprintf(("Found cutscene defined as '$Post-briefing Cutscene' and converted it to '$Pre-game cutscene'\n"));
				} else {
					// no more cutscenes specified?
					break;
				}
			}

			// get the cutscene file
			stuff_string(scene.filename, F_NAME, NAME_LENGTH);

			// get the sexp if we have one
			if (optional_string("+formula:"))
				scene.formula = get_sexp_main();
			else
				scene.formula = Locked_sexp_true;

			// add it
			pm->cutscenes.push_back(scene);
		}
	}
}

void parse_plot_info(mission * /*pm*/)
{
	if (optional_string("#Plot Info"))
	{
		char dummy_filespec[FILESPEC_LENGTH];
		char dummy_name[NAME_LENGTH];

		required_string("$Tour:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);

		required_string("$Pre-Briefing Cutscene:");
		stuff_string(dummy_filespec, F_FILESPEC, FILESPEC_LENGTH);

		required_string("$Pre-Mission Cutscene:");
		stuff_string(dummy_filespec, F_FILESPEC, FILESPEC_LENGTH);

		required_string("$Next Mission Success:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);

		required_string("$Next Mission Partial:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);

		required_string("$Next Mission Failure:");
		stuff_string(dummy_name, F_NAME, NAME_LENGTH);
	}
}

void parse_briefing_info(mission * /*pm*/)
{
	char junk[4096];

	if ( !optional_string("#Briefing Info") )
		return;

	required_string("$Briefing Voice 1:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Briefing Text 1:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Briefing Voice 2:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Briefing Text 2:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Briefing Voice 3:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Briefing Text 3:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Debriefing Voice 1:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Debriefing Text 1:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Debriefing Voice 2:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Debriefing Text 2:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));

	required_string("$Debriefing Voice 3:");
	stuff_string(junk, F_FILESPEC, sizeof(junk));

	required_string("$Debriefing Text 3:");
	stuff_string(junk, F_MULTITEXTOLD, sizeof(junk));
}

/**
 * Parse the event music and briefing music for the mission
 */
void parse_music(mission *pm, int flags)
{
	int index;
	char *ch;
	char temp[NAME_LENGTH];

	if ( !optional_string("#Music") ) {
		return;
	}

	required_string("$Event Music:");
	stuff_string(pm->event_music_name, F_NAME, NAME_LENGTH);

	// Goober5000
	if (optional_string("$Substitute Event Music:"))
		stuff_string(pm->substitute_event_music_name, F_NAME, NAME_LENGTH);

	required_string("$Briefing Music:");
	stuff_string(pm->briefing_music_name, F_NAME, NAME_LENGTH);

	// Goober5000
	if (optional_string("$Substitute Briefing Music:"))
		stuff_string(pm->substitute_briefing_music_name, F_NAME, NAME_LENGTH);

	// old stuff, apparently
	if (optional_string("$Debriefing Success Music:"))
	{
		stuff_string(temp, F_NAME, NAME_LENGTH);
		index = event_music_get_spooled_music_index(temp);
		if ((index >= 0) && ((Spooled_music[index].flags & SMF_VALID) || Fred_running)) {
			event_music_set_score(SCORE_DEBRIEFING_SUCCESS, temp);
		}
	}

	// not old, just added since it makes sense
	if (optional_string("$Debriefing Average Music:"))
	{
		stuff_string(temp, F_NAME, NAME_LENGTH);
		index = event_music_get_spooled_music_index(temp);
		if ((index >= 0) && ((Spooled_music[index].flags & SMF_VALID) || Fred_running)) {
			event_music_set_score(SCORE_DEBRIEFING_AVERAGE, temp);
		}
	}

	// old stuff
	if (optional_string("$Debriefing Fail Music:") || optional_string("$Debriefing Failure Music:"))
	{
		stuff_string(temp, F_NAME, NAME_LENGTH);
		index = event_music_get_spooled_music_index(temp);
		if ((index >= 0) && ((Spooled_music[index].flags & SMF_VALID) || Fred_running)) {
			event_music_set_score(SCORE_DEBRIEFING_FAILURE, temp);
		}
	}

	// new stuff
	if (optional_string("$Fiction Viewer Music:"))
	{
		stuff_string(temp, F_NAME, NAME_LENGTH);
		event_music_set_score(SCORE_FICTION_VIEWER, temp);
	}

	// Goober5000 - if briefing not specified in import, default to BRIEF1
	if (!stricmp(pm->briefing_music_name, "none") && (flags & MPF_IMPORT_FSM))
		strcpy_s(pm->briefing_music_name, "BRIEF1");

	// Goober5000 - old way of grabbing substitute music, but here for reverse compatibility
	if (optional_string("$Substitute Music:"))
	{
		stuff_string(pm->substitute_event_music_name, F_NAME, NAME_LENGTH, "," );
		Mp++;
		stuff_string(pm->substitute_briefing_music_name, F_NAME, NAME_LENGTH);
	}

	// Goober5000 - if the mission is being imported, the substitutes are the specified tracks
	// (with FS1 prefixes) and we generate new specified tracks
	if (flags & MPF_IMPORT_FSM)
	{
		// no specified music?
		if (!stricmp(pm->event_music_name, "none"))
			goto done_event_music;

		// set the FS1 equivalent as the substitute
		strcpy_s(pm->substitute_event_music_name, "FS1-");
		strcat_s(pm->substitute_event_music_name, pm->event_music_name);

		// if we have Marauder, it's in FS2 as Deuteronomy, so we're done
		if (!stricmp(pm->event_music_name, "7: Marauder") && event_music_get_soundtrack_index("5: Deuteronomy") >= 0)
		{
			strcpy_s(pm->event_music_name, "5: Deuteronomy");
			goto done_event_music;
		}

		// search for something with the same track number
		strcpy_s(temp, pm->event_music_name);
		ch = strchr(temp, ':');
		if (ch != NULL)
		{
			*(ch + 1) = '\0';
				
			for (auto &st: Soundtracks)
			{
				if (!strncmp(temp, st.name, strlen(temp)))
				{
					strcpy_s(pm->event_music_name, st.name);
					goto done_event_music;
				}
			}
		}

		if (!Soundtracks.empty())
		{
			// last resort: pick a random track out of the 7 FS2 soundtracks
			int num = std::max((int)Soundtracks.size(), 7);
			strcpy_s(pm->event_music_name, Soundtracks[Random::next(num)].name);
		}


done_event_music:


		// no specified music?
		if (!stricmp(pm->briefing_music_name, "none"))
			goto done_briefing_music;

		// set the FS1 equivalent as the substitute
		strcpy_s(pm->substitute_briefing_music_name, "FS1-");
		strcat_s(pm->substitute_briefing_music_name, pm->briefing_music_name);

		// Choco Mousse is the FS1 title soundtrack, so use Aquitaine in FS2
		if (!stricmp(pm->briefing_music_name, "Choco Mousse") && event_music_get_spooled_music_index("Aquitaine") >= 0)
		{
			strcpy_s(pm->briefing_music_name, "Aquitaine");
			goto done_briefing_music;
		}

		// we might have a match with the same track number
		if (event_music_get_spooled_music_index(pm->briefing_music_name) >= 0)
			goto done_briefing_music;

		if (!Spooled_music.empty())
		{
			// last resort: pick a random track out of the first 7 FS2 briefings (the regular ones)...
			int num = std::max((int)Spooled_music.size(), 7);
			strcpy_s(pm->briefing_music_name, Spooled_music[Random::next(num)].name);
		}


done_briefing_music:

		/* NO-OP */ ;
	}


	// set the soundtrack, preferring the substitute in FS2 (not FRED!)
	index = event_music_get_soundtrack_index(pm->substitute_event_music_name);
	if ((index >= 0) && (Soundtracks[index].flags & EMF_VALID) && !Fred_running)
	{
		event_music_set_soundtrack(pm->substitute_event_music_name);
	}
	else
	{
		event_music_set_soundtrack(pm->event_music_name);
	}

	// set the briefing, preferring the substitute in FS2 (not FRED!)
	index = event_music_get_spooled_music_index(pm->substitute_briefing_music_name);
	if ((index >= 0) && (Spooled_music[index].flags & SMF_VALID) && !Fred_running)
	{
		event_music_set_score(SCORE_BRIEFING, pm->substitute_briefing_music_name);
	}
	else
	{
		event_music_set_score(SCORE_BRIEFING, pm->briefing_music_name);
	}
}

/**
 * Parse fiction viewer
 */
void parse_fiction(mission * /*pm*/)
{
	if (optional_string("#Fiction Viewer"))
	{
		bool fiction_viewer_loaded = false;

		while (check_for_string("$File:"))
		{
			fiction_viewer_stage stage;
			memset(&stage, 0, sizeof(fiction_viewer_stage));
			stage.formula = Locked_sexp_true;

			required_string("$File:");
			stuff_string(stage.story_filename, F_FILESPEC, MAX_FILENAME_LEN);

			if (optional_string("$Font:"))
				stuff_string(stage.font_filename, F_FILESPEC, MAX_FILENAME_LEN);

			if (optional_string("$Voice:"))
				stuff_string(stage.voice_filename, F_FILESPEC, MAX_FILENAME_LEN);

			if (optional_string("$UI:"))
				stuff_string(stage.ui_name, F_NAME, NAME_LENGTH);

			parse_custom_bitmap("$Background 640:", "$Background 1024:", stage.background[GR_640], stage.background[GR_1024]);

			// get the sexp if we have one
			if (optional_string("$Formula:"))
				stage.formula = get_sexp_main();

			if (strlen(stage.story_filename) > 0)
			{
				// now, store this stage
				Fiction_viewer_stages.push_back(stage);

				// see if this is the stage we want to display, then display it
				if (!Fred_running && !fiction_viewer_loaded && is_sexp_true(stage.formula))
				{
					fiction_viewer_load((int)(Fiction_viewer_stages.size() - 1));
					fiction_viewer_loaded = true;
				}
			}
			else
			{
				error_display(0, "Fiction viewer story filename may not be empty!");
			}
		}
	}
}

/**
 * Parse command briefing
 */
void parse_cmd_brief(mission * /*pm*/)
{
	int stage;

	Assert(!Cur_cmd_brief->num_stages);
	stage = 0;

	required_string("#Command Briefing");

	// Yarn - use the same code as for mission loading screens
	parse_custom_bitmap("$Background 640:", "$Background 1024:", Cur_cmd_brief->background[GR_640], Cur_cmd_brief->background[GR_1024]);

	while (optional_string("$Stage Text:")) {
		Assert(stage < CMD_BRIEF_STAGES_MAX);
		stuff_string(Cur_cmd_brief->stage[stage].text, F_MULTITEXT, NULL);

		required_string("$Ani Filename:");
		stuff_string(Cur_cmd_brief->stage[stage].ani_filename, F_FILESPEC, MAX_FILENAME_LEN);
		if (optional_string("+Wave Filename:"))
			stuff_string(Cur_cmd_brief->stage[stage].wave_filename, F_FILESPEC, MAX_FILENAME_LEN);
		else
			Cur_cmd_brief->stage[stage].wave_filename[0] = 0;

		stage++;
	}

	Cur_cmd_brief->num_stages = stage;
}

void parse_cmd_briefs(mission *pm)
{
	int i;

	// a hack follows because old missions don't have a command briefing
	if (required_string_either("#Command Briefing", "#Briefing"))
		return;

	for (i=0; i<Num_teams; i++) {
		Cur_cmd_brief = &Cmd_briefs[i];
		parse_cmd_brief(pm);
	}
}

/**
 * Parse the data required for the mission briefing
 *
 * NOTE: This updates the global Briefing struct with all the data necessary to drive the briefing
 */
void parse_briefing(mission * /*pm*/, int flags)
{
	int nt, i, j, stage_num = 0, icon_num = 0;
	brief_stage *bs;
	brief_icon *bi;
	briefing *bp;

	char not_used_text[MAX_ICON_TEXT_LEN];
	
	// MWA -- 2/3/98.  we can now have multiple briefing and debriefings in a mission
	for ( nt = 0; nt < Num_teams; nt++ ) {
		if ( !optional_string("#Briefing") )
			break;

		bp = &Briefings[nt];
		required_string("$start_briefing");

		// Goober5000 - use the same code as for mission loading screens
		parse_custom_bitmap("$briefing_background_640:", "$briefing_background_1024:", bp->background[GR_640], bp->background[GR_1024]);
		parse_custom_bitmap("$ship_select_background_640:", "$ship_select_background_1024:", bp->ship_select_background[GR_640], bp->ship_select_background[GR_1024]);
		parse_custom_bitmap("$weapon_select_background_640:", "$weapon_select_background_1024:", bp->weapon_select_background[GR_640], bp->weapon_select_background[GR_1024]);

		required_string("$num_stages:");
		stuff_int(&bp->num_stages);
		Assert(bp->num_stages <= MAX_BRIEF_STAGES);

		stage_num = 0;
		while (required_string_either("$end_briefing", "$start_stage")) {
			required_string("$start_stage");
			Assert(stage_num < MAX_BRIEF_STAGES);

			if (stage_num >= bp->num_stages) {
				error_display(1,
							  "$num_stages did not match the number of specified stages! %d stages were specified but there is at least one more.",
							  bp->num_stages);
			}

			bs = &bp->stages[stage_num++];
			required_string("$multi_text");
			stuff_string(bs->text, F_MULTITEXT, NULL);
			required_string("$voice:");
			stuff_string(bs->voice, F_FILESPEC, MAX_FILENAME_LEN);
			required_string("$camera_pos:");
			stuff_vec3d(&bs->camera_pos);
			required_string("$camera_orient:");
			stuff_matrix(&bs->camera_orient);
			required_string("$camera_time:");
			stuff_int(&bs->camera_time);

			if (optional_string("$no_grid"))
				bs->draw_grid = false;

			if (optional_string("$grid_color:")) {
				int rgba[4] = {0, 0, 0, 0};
				stuff_int_list(rgba, 4, RAW_INTEGER_TYPE);
				gr_init_alphacolor(&bs->grid_color, rgba[0], rgba[1], rgba[2], rgba[3]);
			} else {
				bs->grid_color = Color_briefing_grid;
			}

			if ( optional_string("$num_lines:") ) {
				stuff_int(&bs->num_lines);

				if ( Fred_running )	{
					Assert(bs->lines!=NULL);
				} else {
					if ( bs->num_lines > 0 )	{
						bs->lines = (brief_line *)vm_malloc(sizeof(brief_line)*bs->num_lines);
						Assert(bs->lines!=NULL);
					}
				}

				for (i=0; i<bs->num_lines; i++) {
					required_string("$line_start:");
					stuff_int(&bs->lines[i].start_icon);
					required_string("$line_end:");
					stuff_int(&bs->lines[i].end_icon);
				}
			}
			else {
				bs->num_lines = 0;
			}

			required_string("$num_icons:");
			stuff_int(&bs->num_icons);

			if ( Fred_running )	{
				Assert(bs->icons!=NULL);
			} else {
				if ( bs->num_icons > 0 )	{
					bs->icons = (brief_icon *)vm_malloc(sizeof(brief_icon)*bs->num_icons);
					Assert(bs->icons!=NULL);
				}
			}

			if ( optional_string("$flags:") )
				stuff_int(&bs->flags);
			else
				bs->flags = 0;

			if ( optional_string("$formula:") )
				bs->formula = get_sexp_main();
			else
				bs->formula = Locked_sexp_true;

			Assert(bs->num_icons <= MAX_STAGE_ICONS );

			// static alias stuff - stupid, but it seems to be necessary
			auto temp_team_names = std::unique_ptr<const char* []>(new const char*[Iff_info.size()]);
			for (i = 0; i < (int)Iff_info.size(); i++)
				temp_team_names[i] = Iff_info[i].iff_name;

			while (required_string_either("$end_stage", "$start_icon"))
			{
				required_string("$start_icon");
				Assert(icon_num < MAX_STAGE_ICONS);
				// Make sure we don't cause a buffer overflow if $num_icons is wrong
				if (icon_num >= bs->num_icons) {
					error_display(1,
								  "$num_icons did not match the number of specified icons! %d icons were specified but there is at least one more.",
								  bs->num_icons);
				}
				bi = &bs->icons[icon_num++];

				required_string("$type:");
				stuff_int(&bi->type);

				// Goober5000 - import
				if (flags & MPF_IMPORT_FSM)
				{
					// someone changed the jump node icon to a Knossos, so change it back
					if (bi->type == ICON_KNOSSOS_DEVICE)
						bi->type = ICON_JUMP_NODE;

					// change largeship to transport
					else if (bi->type == ICON_LARGESHIP)
						bi->type = ICON_TRANSPORT;

					// ditto
					else if (bi->type == ICON_LARGESHIP_WING)
						bi->type = ICON_TRANSPORT_WING;
				}

				find_and_stuff("$team:", &bi->team, F_NAME, temp_team_names.get(), Iff_info.size(), "team name");

				find_and_stuff("$class:", &bi->ship_class, F_NAME, Ship_class_names, Ship_info.size(), "ship class");
				bi->modelnum = -1;
				bi->model_instance_num = -1;

				// Goober5000 - import
				if (flags & MPF_IMPORT_FSM)
				{
					// the Faustus is a largeship
					if (!strnicmp(Ship_info[bi->ship_class].name, "GTSC Faustus", 12))
					{
						if (bi->type == ICON_CRUISER)
							bi->type = ICON_LARGESHIP;

						else if (bi->type == ICON_CRUISER_WING)
							bi->type = ICON_LARGESHIP_WING;
					}
					// the Hades is a supercap
					else if (!strnicmp(Ship_info[bi->ship_class].name, "GTD Hades", 9))
					{
						bi->type = ICON_SUPERCAP;
					}
				}

				required_string("$pos:");
				stuff_vec3d(&bi->pos);

				bi->label[0] = 0;
				if (optional_string("$label:"))
					stuff_string(bi->label, F_MESSAGE, MAX_LABEL_LEN);
				bi->closeup_label[0] = 0;
				if (optional_string("$closeup label:")) {
					stuff_string(bi->closeup_label, F_MESSAGE, MAX_LABEL_LEN);
				}

				bi->scale_factor = 1.0f;
				if (optional_string("$icon scale:")) {
					int scale;
					stuff_int(&scale);
					bi->scale_factor = scale / 100.0f;
				}

				if (optional_string("+id:")) {
					stuff_int(&bi->id);
					if (bi->id >= Cur_brief_id)
						Cur_brief_id = bi->id + 1;

				} else {
					bi->id = -1;
					for (i=0; i<stage_num-1; i++)
						for (j=0; j < bp->stages[i].num_icons; j++)
						{
							if (!stricmp(bp->stages[i].icons[j].label, bi->label))
								bi->id = bp->stages[i].icons[j].id;
						}

					if (bi->id < 0)
						bi->id = Cur_brief_id++;
				}

				bi->flags=0;
				int val;
				required_string("$hlight:");	
				stuff_int(&val);
				if ( val>0 ) {
					bi->flags |= BI_HIGHLIGHT;
				}

				if (optional_string("$mirror:"))
				{
					stuff_int(&val);
					if ( val>0 ) {
						bi->flags |= BI_MIRROR_ICON;
					}	
				}

				if (optional_string("$use wing icon:"))
				{
					stuff_int(&val);
					if ( val>0 ) {
						bi->flags |= BI_USE_WING_ICON;
					}
				}

				if (optional_string("$use cargo icon:")) {
					stuff_int(&val);
					if (val > 0) {
						bi->flags |= BI_USE_CARGO_ICON;
					}
				}

				required_string("$multi_text");
				stuff_string(not_used_text, F_MULTITEXT, MAX_ICON_TEXT_LEN);
				required_string("$end_icon");
			} // end while

			if (icon_num != bs->num_icons) {
				error_display(1,
							  "$num_icons did not match the number of specified icons! %d icons were specified but only %d were parsed.",
							  bs->num_icons,
							  icon_num);
			}
			icon_num = 0;
			required_string("$end_stage");
		}	// end while
		if (stage_num != bp->num_stages) {
			error_display(1,
						  "$num_stages did not match the number of specified icons! %d stages were specified but only %d were parsed.",
						  bp->num_stages,
						  stage_num);
		}
		required_string("$end_briefing");
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough briefings in mission file.  There are %d teams and only %d briefings.", Num_teams, nt );
}

/**
 * Parse the data required for the mission debriefings
 */
void parse_debriefing_new(mission * /*pm*/)
{
	int				stage_num, nt;
	debriefing		*db;
	debrief_stage	*dbs;
	
	// 2/3/98 -- MWA.  We can now have multiple briefings and debriefings on a team
	for ( nt = 0; nt < Num_teams; nt++ ) {

		if ( !optional_string("#Debriefing_info") )
			break;

		stage_num = 0;

		db = &Debriefings[nt];

		// Yarn - use the same code as for mission loading screens
		parse_custom_bitmap("$Background 640:", "$Background 1024:", db->background[GR_640], db->background[GR_1024]);

		required_string("$Num stages:");
		stuff_int(&db->num_stages);
		Assert(db->num_stages <= MAX_DEBRIEF_STAGES);

		while (required_string_either("#", "$Formula")) {
			Assert(stage_num < MAX_DEBRIEF_STAGES);
			dbs = &db->stages[stage_num++];
			required_string("$Formula:");
			dbs->formula = get_sexp_main();
			required_string("$multi text");
			stuff_string(dbs->text, F_MULTITEXT, NULL);
			required_string("$Voice:");
			stuff_string(dbs->voice, F_FILESPEC, MAX_FILENAME_LEN);
			required_string("$Recommendation text:");
			stuff_string(dbs->recommendation_text, F_MULTITEXT, NULL);
		} // end while

		Assert(db->num_stages == stage_num);
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough debriefings for mission.  There are %d teams and only %d debriefings;\n", Num_teams, nt );
}

void position_ship_for_knossos_warpin(object *objp)
{
	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];
	object *knossos_objp = nullptr;
	float half_length, min_dist = -1.0f;
	vec3d center_pos, actual_local_center;
	vec3d new_point, new_center_pos, offset;

	// Assume no valid knossos device
	shipp->special_warpin_objnum = -1;

	// find closest knossos device (allow multiple knossoses)
	for (ship_obj *so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so))
	{
		object *ship_objp = &Objects[so->objnum];
		if (ship_objp->flags[Object::Object_Flags::Should_be_dead])
			continue;

		if (Ship_info[Ships[ship_objp->instance].ship_info_index].flags[Ship::Info_Flags::Knossos_device])
		{
			// is this the closest?  (can use dist_squared since we're only comparing)
			float dist = vm_vec_dist_squared(&ship_objp->pos, &objp->pos);
			if (min_dist < 0.0f || dist < min_dist)
			{
				knossos_objp = ship_objp;
				min_dist = dist;
			}
		}
	}

	if (knossos_objp == nullptr)
		return;

	// set ship special_warpin_objnum
	shipp->special_warpin_objnum = OBJ_INDEX(knossos_objp);

	// determine the correct center of the model (which may not be the model's origin)
	if (object_is_docked(objp))
		dock_calc_docked_actual_center(&actual_local_center, objp);
	else
		ship_class_get_actual_center(sip, &actual_local_center);

	// find world position of the center of the ship assembly
	vm_vec_unrotate(&center_pos, &actual_local_center, &objp->orient);
	vm_vec_add2(&center_pos, &objp->pos);

	// determine the half-length
	if (object_is_docked(objp))
	{
		// we need to get the longitudinal radius of our ship, so find the semilatus rectum along the Z-axis
		half_length = dock_calc_max_semilatus_rectum_parallel_to_axis(objp, Z_AXIS);
	}
	else
		half_length = 0.5f * ship_class_get_length(sip);

	// position self for warp on plane of device
	float dist = fvi_ray_plane(&new_point, &knossos_objp->pos, &knossos_objp->orient.vec.fvec, &center_pos, &objp->orient.vec.fvec, 0.0f);
	vm_vec_scale_add(&new_center_pos, &center_pos, &objp->orient.vec.fvec, (dist - half_length));

	// now move the actual ship based on how we moved the center
	vm_vec_sub(&offset, &new_center_pos, &center_pos);
	vm_vec_add2(&objp->pos, &offset);
	
	// if ship is HUGE, make it go through the center of the knossos
	if (sip->is_huge_ship())
	{
		vm_vec_sub(&offset, &knossos_objp->pos, &new_point);
		vm_vec_add2(&objp->pos, &offset);
	}
}

/**
 * This is conceptually almost the same as ::obj_move_one_docked_object and is used in the same way.
 */
void parse_dock_one_docked_object(p_object *pobjp, p_object *parent_pobjp)
{
	int dockpoint, parent_dockpoint;
	char *dockpoint_name, *parent_dockpoint_name;
	object *objp, *parent_objp;

	// get the actual in-game objects that will be docked
	objp = pobjp->created_object;
	parent_objp = parent_pobjp->created_object;

	// check valid
	if (!objp || !parent_objp)
	{
		Int3();
		return;
	}

	// get dockpoint names
	dockpoint_name = dock_find_dockpoint_used_by_object(pobjp, parent_pobjp);
	parent_dockpoint_name = dock_find_dockpoint_used_by_object(parent_pobjp, pobjp);

	// check valid
	if (!dockpoint_name || !parent_dockpoint_name)
	{
		Int3();
		return;
	}

	// resolve names to dockpoints
	dockpoint = model_find_dock_name_index(Ship_info[Ships[objp->instance].ship_info_index].model_num, dockpoint_name);
	parent_dockpoint = model_find_dock_name_index(Ship_info[Ships[parent_objp->instance].ship_info_index].model_num, parent_dockpoint_name);

	// check valid
	if ((dockpoint < 0) || (parent_dockpoint < 0))
	{
		if (dockpoint < 0)
			ReleaseWarning(LOCATION, "Dockpoint %s could not be found on model %s", dockpoint_name, model_get(Ship_info[Ships[objp->instance].ship_info_index].model_num)->filename);
		if (parent_dockpoint < 0)
			ReleaseWarning(LOCATION, "Dockpoint %s could not be found on model %s", parent_dockpoint_name, model_get(Ship_info[Ships[parent_objp->instance].ship_info_index].model_num)->filename);

		return;
	}

	// dock them
	nprintf(("AI", "Initially docked: %s to parent %s\n", Ships[objp->instance].ship_name, Ships[parent_objp->instance].ship_name));
	ai_dock_with_object(objp, dockpoint, parent_objp, parent_dockpoint, AIDO_DOCK_NOW);
}

int parse_create_object_sub(p_object *objp, bool standalone_ship = false);

// Goober5000
void parse_create_docked_object_helper(p_object *pobjp, p_dock_function_info *infop)
{
	// create the object
	parse_create_object_sub(pobjp);

	// remove the object from the arrival list
	// (don't remove the leader, because he'll be removed the usual way)
	if (pobjp != infop->parameter_variables.objp_value)
	{
		// If an object is present at the beginning of the mission, it may not have been put on the arrival
		// list.  This is totally dependent on the order the objects are parsed in the mission file because
		// that is the order in which they are created.  All the objects in a docked group are created
		// simultaneously when and only when the leader is created.

		// if it's on the list, remove it
		if (parse_object_on_arrival_list(pobjp))
			list_remove(&Ship_arrival_list, pobjp);
	}
}

/**
 * This is a bit tricky because of the way initial docking is now handled.
 * Docking groups require special treatment.
 */
int parse_create_object(p_object *pobjp, bool standalone_ship)
{
	object *objp;

	// if this guy is part of a dock group, create the entire group, starting with the leader
	if (object_is_docked(pobjp))
	{
		p_dock_function_info dfi;

		// we should only be calling this for dock leaders, because the dock leader
		// governs the creation of his entire group
		Assert((pobjp->flags[Mission::Parse_Object_Flags::SF_Dock_leader]));

		// if the leader will be destroyed before the mission starts, then *only* create the leader;
		// don't create the rest of the group (this is what retail did)
		if (pobjp->destroy_before_mission_time >= 0)
			return parse_create_object_sub(pobjp);

		// store the leader as a parameter
		dfi.parameter_variables.objp_value = pobjp;

		// Just as we couldn't create the parse dock trees until we had parsed them all,
		// we can't dock the in-game objects until we have created them all. :p

		// create all the objects
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_create_docked_object_helper);

		// dock all the objects
		dock_dock_docked_objects(pobjp);

		// now clear the handled flags
		parse_object_clear_all_handled_flags();
	}
	// create normally
	else
	{
		parse_create_object_sub(pobjp, standalone_ship);
	}

	// get the main object
	objp = pobjp->created_object;

	// if arriving through knossos, adjust objp->pos to plane of knossos and set object reference
	// special warp is single player only
	if ((pobjp->flags[Mission::Parse_Object_Flags::Knossos_warp_in]) && !(Game_mode & GM_MULTIPLAYER))
	{
		if (!Fred_running)
			position_ship_for_knossos_warpin(objp);
	}

	// warp it in (moved from parse_create_object_sub)
	if ((Game_mode & GM_IN_MISSION) && (!Fred_running) && (!Game_restoring))
	{
		if ((Ships[objp->instance].wingnum < 0) && (pobjp->arrival_location != ArrivalLocation::FROM_DOCK_BAY))
		{
			shipfx_warpin_start(objp);
		}
	}

	// return the main object's objnum
	return OBJ_INDEX(objp);
}

void parse_bring_in_docked_wing(p_object *p_objp, int wingnum, int shipnum);

/**
 * Given a stuffed p_object struct, create an object and fill in the necessary fields.
 * @return object number.
 */
int parse_create_object_sub(p_object *p_objp, bool standalone_ship)
{
	int	i, j, k, objnum, shipnum;
	int anchor_objnum = -1;
	bool brought_in_docked_wing = false;
	ai_info *aip;
	ship_subsys *ptr;
	ship *shipp;
	ship_info *sip;
	subsys_status *sssp;
	ship_weapon *wp;

	MONITOR_INC(NumShipArrivals, 1);

	// base level creation - need ship name in case of duplicate textures
	objnum = ship_create(&p_objp->orient, &p_objp->pos, p_objp->ship_class, p_objp->name, standalone_ship);
	Assert(objnum != -1);
	shipnum = Objects[objnum].instance;

	shipp = &Ships[shipnum];
	sip = &Ship_info[shipp->ship_info_index];

	// Goober5000 - make the parse object aware of what it was created as
	p_objp->created_object = &Objects[objnum];

	// Goober5000 - set the collision group if one was provided
	Objects[objnum].collision_group_id = p_objp->collision_group_id;

	// Goober5000 - if this object is being created because he's docked to something,
	// and he's in a wing, then mark the wing as having arrived
	if (object_is_docked(p_objp) && !(p_objp->flags[Mission::Parse_Object_Flags::SF_Dock_leader]) && (p_objp->wingnum >= 0))
	{
		if (!Fred_running)
		{
			parse_bring_in_docked_wing(p_objp, p_objp->wingnum, shipnum);
			brought_in_docked_wing = true;
		}
	}

	shipp->group = p_objp->group;
	shipp->team = p_objp->team;
	shipp->display_name = p_objp->display_name;
	shipp->escort_priority = p_objp->escort_priority;
	shipp->use_special_explosion = p_objp->use_special_explosion;
	shipp->special_exp_damage = p_objp->special_exp_damage;
	shipp->special_exp_blast = p_objp->special_exp_blast;
	shipp->special_exp_inner = p_objp->special_exp_inner;
	shipp->special_exp_outer = p_objp->special_exp_outer;
	shipp->use_shockwave = p_objp->use_shockwave;
	shipp->special_exp_shockwave_speed = p_objp->special_exp_shockwave_speed;
	shipp->special_exp_deathroll_time = p_objp->special_exp_deathroll_time;

	shipp->special_hitpoints = p_objp->special_hitpoints;
	shipp->special_shield = p_objp->special_shield;

	shipp->ship_iff_color = p_objp->alt_iff_color;

	shipp->ship_max_shield_strength = p_objp->ship_max_shield_strength;
	shipp->ship_max_hull_strength =  p_objp->ship_max_hull_strength;
	shipp->max_shield_recharge = p_objp->max_shield_recharge;

	// Goober5000 - ugh, this is really stupid having to do this here; if the
	// ship creation code was better organized this wouldn't be necessary
	if (shipp->special_hitpoints > 0)
	{
		float hull_factor = shipp->ship_max_hull_strength / sip->max_hull_strength;
		ship_subsys *ss;

		for (ss = GET_FIRST(&shipp->subsys_list); ss != END_OF_LIST(&shipp->subsys_list) && ss != NULL; ss = GET_NEXT(ss))
		{
			ss->max_hits *= hull_factor;

			if (Fred_running)
				ss->current_hits = 0.0f;
			else
				ss->current_hits = ss->max_hits;
		}

		ship_recalc_subsys_strength(shipp);
	}

	// Goober5000 - this is also stupid; this is in ship_set but it needs to be done here because of the special hitpoints mod
	Objects[objnum].hull_strength = shipp->ship_max_hull_strength;

	shipp->respawn_priority = p_objp->respawn_priority;

	// if this is a multiplayer dogfight game, and its from a player wing, make it team traitor
	if (MULTI_DOGFIGHT && (p_objp->wingnum >= 0) && p_objp->flags[Mission::Parse_Object_Flags::SF_From_player_wing])
		shipp->team = Iff_traitor;

	// alternate stuff
	shipp->alt_type_index = p_objp->alt_type_index;
	shipp->callsign_index = p_objp->callsign_index;

	// AI stuff.  Note a lot of the AI was already initialized in ship_create.
	aip = &(Ai_info[shipp->ai_index]);

	aip->ai_class = p_objp->ai_class;
	shipp->weapons.ai_class = p_objp->ai_class;  // Fred uses this instead of above.
	//Fixes a bug where the AI class attributes were not copied if the AI class was set in the mission.
	if (The_mission.ai_profile->flags[AI::Profile_Flags::Fix_ai_class_bug])
		ship_set_new_ai_class(shipp, p_objp->ai_class);

	aip->mode = AIM_NONE;

	// make sure aim_safety has its submode defined
	if (aip->mode == AIM_SAFETY) {
		aip->submode = AISS_1;
	}

	shipp->cargo1 = p_objp->cargo1;
	strcpy_s(shipp->cargo_title, p_objp->cargo_title);

	shipp->arrival_location = p_objp->arrival_location;
	shipp->arrival_distance = p_objp->arrival_distance;
	shipp->arrival_anchor = p_objp->arrival_anchor;
	shipp->arrival_path_mask = p_objp->arrival_path_mask;
	shipp->arrival_cue = p_objp->arrival_cue;
	shipp->arrival_delay = p_objp->arrival_delay;
	shipp->departure_location = p_objp->departure_location;
	shipp->departure_anchor = p_objp->departure_anchor;
	shipp->departure_path_mask = p_objp->departure_path_mask;
	shipp->departure_cue = p_objp->departure_cue;
	shipp->departure_delay = p_objp->departure_delay;
	shipp->wingnum = p_objp->wingnum;
	shipp->hotkey = p_objp->hotkey;
	shipp->score = p_objp->score;
	shipp->assist_score_pct = p_objp->assist_score_pct;
	shipp->persona_index = p_objp->persona_index;
	if (Ship_info[shipp->ship_info_index].uses_team_colors && !p_objp->team_color_setting.empty())
		shipp->team_name = p_objp->team_color_setting;

	if (p_objp->warpin_params_index >= 0)
		shipp->warpin_params_index = p_objp->warpin_params_index;
	if (p_objp->warpout_params_index >= 0)
		shipp->warpout_params_index = p_objp->warpout_params_index;

	// now that we have our correct warpout params, set the warp effects
	if (!Fred_running) {
		ship_set_warp_effects(&Objects[objnum]);
	}

	// reset texture animations
	shipp->base_texture_anim_timestamp = _timestamp();

	// handle the replacement textures
	shipp->apply_replacement_textures(p_objp->replacement_textures);

	// Copy across the alt classes (if any) for FRED
	if (Fred_running) {
		shipp->s_alt_classes = p_objp->alt_classes; 
	}

	// check the parse object's flags for possible things to set on this newly created ship
	resolve_parse_flags(&Objects[objnum], p_objp->flags);


	// other flag checks
////////////////////////

	// forcing the shields on or off depending on flags -- but only if shield strength supports it

	// no strength means we can't have shields, period
    if (p_objp->ship_max_shield_strength == 0.0f)
        Objects[objnum].flags.set(Object::Object_Flags::No_shields);
    // force shields on means we have them regardless of other flags; per r5332 this ranks above the next check
    else if (p_objp->flags[Mission::Parse_Object_Flags::OF_Force_shields_on])
        Objects[objnum].flags.remove(Object::Object_Flags::No_shields);
    // intrinsic no-shields means we have them off in-game
    else if (!Fred_running && (sip->flags[Ship::Info_Flags::Intrinsic_no_shields]))
        Objects[objnum].flags.set(Object::Object_Flags::No_shields);

	// don't set the flag if the mission is ongoing in a multiplayer situation. This will be set by the players in the
	// game only before the game or during respawning.
	// MWA -- changed the next line to remove the !(Game_mode & GM_MULTIPLAYER).  We shouldn't be setting
	// this flag in single player mode -- it gets set in post process mission.
    if ((p_objp->flags[Mission::Parse_Object_Flags::OF_Player_start]) && (Fred_running || ((Game_mode & GM_MULTIPLAYER) && !(Game_mode & GM_IN_MISSION)))) {
		Objects[objnum].flags.set(Object::Object_Flags::Player_ship);
	}

	// a couple of ai_info flags.  Also, do a reasonable default for the kamikaze damage regardless of
	// whether this flag is set or not
	if (p_objp->flags[Mission::Parse_Object_Flags::AIF_Kamikaze])
	{
		Ai_info[shipp->ai_index].ai_flags.set(AI::AI_Flags::Kamikaze);
		Ai_info[shipp->ai_index].kamikaze_damage = p_objp->kamikaze_damage;
	}

	if (p_objp->flags[Mission::Parse_Object_Flags::AIF_No_dynamic])
		Ai_info[shipp->ai_index].ai_flags.set(AI::AI_Flags::No_dynamic);

	if (p_objp->flags[Mission::Parse_Object_Flags::SF_Red_alert_store_status])
	{
		if (!(Game_mode & GM_MULTIPLAYER)) {
			shipp->flags.set(Ship::Ship_Flags::Red_alert_store_status);
		}
	}

	if (p_objp->flags[Mission::Parse_Object_Flags::Knossos_warp_in])
	{
        Objects[objnum].flags.set(Object::Object_Flags::Special_warpin);
		Knossos_warp_ani_used = true;
	}

	// set the orders that this ship will accept.  It will have already been set to default from the
	// ship create code, so only set them if the parse object flags say they are unique
	if (p_objp->flags[Mission::Parse_Object_Flags::SF_Use_unique_orders])
	{
		shipp->orders_accepted = p_objp->orders_accepted;

		// MWA  5/15/98 -- Added the following debug code because some orders that ships
		// will accept were apparently written out incorrectly with Fred.  This Int3() should
		// trap these instances.
#ifndef NDEBUG
		if (Fred_running)
		{
			std::set<size_t> default_orders, remaining_orders;
			
			default_orders = ship_get_default_orders_accepted(&Ship_info[shipp->ship_info_index]);
			std::set_difference(p_objp->orders_accepted.begin(), p_objp->orders_accepted.end(), default_orders.begin(), default_orders.end(),
								  std::inserter(remaining_orders, remaining_orders.begin()));
			if (!remaining_orders.empty())
			{
				Warning(LOCATION, "Ship %s has orders which it will accept that are\nnot part of default orders accepted.\n\nPlease reedit this ship and change the orders again\n", shipp->ship_name);
			}
		}
#endif
	}

////////////////////////


	// if ship is in a wing, set some equivalent flags on the ship
	if (shipp->wingnum >= 0)
	{
		auto wingp = &Wings[shipp->wingnum];

		if (wingp->flags[Ship::Wing_Flags::No_arrival_warp])
			shipp->flags.set(Ship::Ship_Flags::No_arrival_warp);

		if (wingp->flags[Ship::Wing_Flags::No_departure_warp])
			shipp->flags.set(Ship::Ship_Flags::No_departure_warp);

		if (wingp->flags[Ship::Wing_Flags::Same_arrival_warp_when_docked])
			shipp->flags.set(Ship::Ship_Flags::Same_arrival_warp_when_docked);

		if (wingp->flags[Ship::Wing_Flags::Same_departure_warp_when_docked])
			shipp->flags.set(Ship::Ship_Flags::Same_departure_warp_when_docked);

		if (wingp->flags[Ship::Wing_Flags::Nav_carry])
			shipp->flags.set(Ship::Ship_Flags::Navpoint_carry);

		// if it's wing leader, take the opportunity to set the wing leader's info index in the wing struct
		if (!Fred_running && p_objp->pos_in_wing == 0 && wingp->special_ship_ship_info_index != -1) {
			wingp->special_ship_ship_info_index = p_objp->ship_class;
		}
	}

	// if the wing index and wing pos are set for this parse object, set them for the ship.  This
	// is useful in multiplayer when ships respawn
	shipp->wing_status_wing_index = p_objp->wing_status_wing_index;
	shipp->wing_status_wing_pos = p_objp->wing_status_wing_pos;


	// set up the ai_goals for this object -- all ships created here are AI controlled.
	if (p_objp->ai_goals != -1)
	{
		int sexp;

		// set up the ai goals for this object.
		for (sexp = CDR(p_objp->ai_goals); sexp != -1; sexp = CDR(sexp))
			ai_add_ship_goal_sexp(sexp, ai_goal_type::EVENT_SHIP, aip);

		// free the sexpression nodes only for non-wing ships.  wing code will handle its own case
		if (p_objp->wingnum < 0)
		{
			free_sexp2(p_objp->ai_goals);	// free up sexp nodes for reuse, since they aren't needed anymore.
			p_objp->ai_goals = -1;
		}
	}

	Assert(sip->model_num != -1);

	// initialize subsystem statii here.  The subsystems are given a percentage damaged.  So a percent value
	// of 20% means that the subsystem is 20% damaged (*not* 20% of max hits).  This is opposite the way
	// that the initial velocity/hull strength/shields work
	i = p_objp->subsys_count;
	while (i--)
	{
		sssp = &Subsys_status[p_objp->subsys_index + i];
		if (!stricmp(sssp->name, NOX("Pilot")))
		{
			ptr = nullptr;
			wp = &shipp->weapons;
		}
		else
		{
			ptr = ship_get_subsys(shipp, sssp->name);	// find the subsystem in the ship list that corresponds to the parsed subsystem
			if (!ptr)
			{
				Warning(LOCATION, "Unable to find '%s' in the ship %s (class %s) subsys_list!", sssp->name, shipp->ship_name, sip->name);
				continue;
			}
			wp = &ptr->weapons;
		}

		if (sssp->primary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
		{
			for (j = k = 0; j < MAX_SHIP_PRIMARY_BANKS; ++j)
			{
				// skip over any empty primary banks unless we are in FRED
				if ((sssp->primary_banks[j] >= 0) || Fred_running)
				{
					wp->primary_bank_weapons[k] = sssp->primary_banks[j];
					++k;
				}
			}

			// double check the Pilot subsystem...
			if (!ptr && !Fred_running)
			{
				// if we somehow ended up with more banks than this ship actually has, clear the excess
				// (this can happen if the mission was edited by hand)
				while (k > sip->num_primary_banks)
					wp->primary_bank_weapons[--k] = -1;
			}

			if (Fred_running)
			{
				// only do this for the Pilot subsystem
				if (!ptr)
					wp->num_primary_banks = sip->num_primary_banks;
			}
			else
				wp->num_primary_banks = k;
		}

		for (j = 0; j < wp->num_primary_banks; ++j)
		{
			if (Fred_running)
			{
				wp->primary_bank_ammo[j] = sssp->primary_ammo[j];
			}
			else if (wp->primary_bank_weapons[j] >= 0 && Weapon_info[wp->primary_bank_weapons[j]].wi_flags[Weapon::Info_Flags::Ballistic])
			{
				Assertion(Weapon_info[wp->primary_bank_weapons[j]].cargo_size > 0.0f,
					"Primary weapon cargo size <= 0. Ship (%s) Subsystem (%s) Bank (%i) Weapon (%s)",
					shipp->ship_name, sssp->name,j,Weapon_info[wp->primary_bank_weapons[j]].name);
				// Pilot subsystem ammo depends on ship ammo capacity
				// in contrast non pilot subsystems depend on the bank capacity of the subsystem
				int ammo_cap;
				if (!ptr) {
					ammo_cap = sip->primary_bank_ammo_capacity[j];
				} else {
					ammo_cap = wp->primary_bank_capacity[j];
				}
				int capacity = (int)std::lround(sssp->primary_ammo[j] / 100.0f * ammo_cap);
				wp->primary_bank_ammo[j] = (int)std::lround(capacity / Weapon_info[wp->primary_bank_weapons[j]].cargo_size);
			}
		}

		if (sssp->secondary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
		{
			for (j = k = 0; j < MAX_SHIP_SECONDARY_BANKS; ++j)
			{
				// skip over any empty secondary banks unless we are in FRED
				if ((sssp->secondary_banks[j] >= 0) || Fred_running)
				{
					wp->secondary_bank_weapons[k] = sssp->secondary_banks[j];
					++k;
				}
			}

			// double check the Pilot subsystem...
			if (!ptr && !Fred_running)
			{
				// if we somehow ended up with more banks than this ship actually has, clear the excess
				// (this can happen if the mission was edited by hand)
				while (k > sip->num_secondary_banks)
					wp->secondary_bank_weapons[--k] = -1;
			}

			if (Fred_running)
			{
				// only do this for the Pilot subsystem
				if (!ptr)
					wp->num_secondary_banks = sip->num_secondary_banks;
			}
			else
				wp->num_secondary_banks = k;
		}

		for (j = 0; j < wp->num_secondary_banks; ++j)
		{
			if (Fred_running)
			{
				wp->secondary_bank_ammo[j] = sssp->secondary_ammo[j];
			}
			else if (wp->secondary_bank_weapons[j] >= 0)
			{
				if (Weapon_info[wp->secondary_bank_weapons[j]].wi_flags[Weapon::Info_Flags::SecondaryNoAmmo]) {
					wp->secondary_bank_ammo[j] = 0;
					continue;
				}

				Assertion(Weapon_info[wp->secondary_bank_weapons[j]].cargo_size > 0.0f,
					"Secondary weapon cargo size <= 0. Ship (%s) Subsystem (%s) Bank (%i) Weapon (%s)",
					shipp->ship_name, sssp->name, j, Weapon_info[wp->secondary_bank_weapons[j]].name);
				// Pilot subsystem ammo depends on ship ammo capacity
				// in contrast non pilot subsystems depend on the bank capacity of the subsystem
				int ammo_cap;
				if (!ptr) {
					ammo_cap = sip->secondary_bank_ammo_capacity[j];
				} else {
					ammo_cap = wp->secondary_bank_capacity[j];
				}
				int capacity = (int)std::lround(sssp->secondary_ammo[j] / 100.0f * ammo_cap);
				wp->secondary_bank_ammo[j] = (int)std::lround(capacity / Weapon_info[wp->secondary_bank_weapons[j]].cargo_size);
			}
		}

		// if we are parsing a Pilot subsystem, skip the rest
		if (!ptr)
			continue;

		if (shipp->flags[Ship::Ship_Flags::Lock_all_turrets_initially])
		{
			// mark all turrets as locked
			if(ptr->system_info->type == SUBSYSTEM_TURRET)
			{
				ptr->weapons.flags.set(Ship::Weapon_Flags::Turret_Lock);
			}
		}

		if (Fred_running)
		{
			ptr->current_hits = sssp->percent;
			ptr->max_hits = 100.0f;
		}
		else
		{
			ptr->max_hits = ptr->system_info->max_subsys_strength * (shipp->ship_max_hull_strength / sip->max_hull_strength);

			float new_hits = ptr->max_hits * ((100.0f - sssp->percent) / 100.f);
			if (!(ptr->flags[Ship::Subsystem_Flags::No_aggregate])) {
				shipp->subsys_info[ptr->system_info->type].aggregate_current_hits -= (ptr->max_hits - new_hits);
			}

			if ((100.0f - sssp->percent) < 0.5)
			{
				ptr->current_hits = 0.0f;
				if (ptr->submodel_instance_1 != nullptr)
					ptr->submodel_instance_1->blown_off = true;
			}
			else
			{
				ptr->current_hits = new_hits;
			}
		}

		ptr->subsys_cargo_name = sssp->subsys_cargo_name;
		strcpy_s(ptr->subsys_cargo_title, sssp->subsys_cargo_title);

		if (sssp->ai_class != SUBSYS_STATUS_NO_CHANGE)
			ptr->weapons.ai_class = sssp->ai_class;

		ptr->turret_animation_position = MA_POS_NOT_SET;	// model animation position is not set
		ptr->turret_animation_done_time = 0;
	}
	
	// initial hull strength, shields, and velocity are all expressed as a percentage of the max value/
	// so a initial_hull value of 90% means 90% of max.  This way is opposite of how subsystems are dealt
	// with
	if (Fred_running)
	{
		Objects[objnum].phys_info.speed = i2fl(p_objp->initial_velocity);
		Objects[objnum].hull_strength = i2fl(p_objp->initial_hull);
		Objects[objnum].shield_quadrant[0] = i2fl(p_objp->initial_shields);

	}
	else
	{
		int max_allowed_sparks, num_sparks, iLoop;

		Objects[objnum].hull_strength = p_objp->initial_hull * shipp->ship_max_hull_strength / 100.0f;
		float quad_strength = shipp->max_shield_recharge * p_objp->initial_shields * shield_get_max_quad(&Objects[objnum]) / 100.0f;
		for (auto &quadrant : Objects[objnum].shield_quadrant)
			quadrant = quad_strength;

		// initial velocities now do not apply to ships which warp in after mission starts
		// WMC - Make it apply for ships with IN_PLACE_ANIM type
		// zookeeper - Also make it apply for hyperspace warps
		if (!(Game_mode & GM_IN_MISSION) || (Warp_params[shipp->warpin_params_index].warp_type == WT_IN_PLACE_ANIM || Warp_params[shipp->warpin_params_index].warp_type == WT_HYPERSPACE))
		{
			Objects[objnum].phys_info.speed = (float) p_objp->initial_velocity * sip->max_speed / 100.0f;
			// prev_ramp_vel needs to be in local coordinates
			// set z of prev_ramp_vel to initial velocity
			vm_vec_zero(&Objects[objnum].phys_info.prev_ramp_vel);
			Objects[objnum].phys_info.prev_ramp_vel.xyz.z = Objects[objnum].phys_info.speed;
			// convert to global coordinates and set to ship velocity and desired velocity
			vm_vec_unrotate(&Objects[objnum].phys_info.vel, &Objects[objnum].phys_info.prev_ramp_vel, &Objects[objnum].orient);
			Objects[objnum].phys_info.desired_vel = Objects[objnum].phys_info.vel;
		}

		// recalculate damage of subsystems
		ship_recalc_subsys_strength(&Ships[shipnum]);

		// create sparks on a ship whose hull is damaged.  We will create two sparks for every 20%
		// of hull damage done.  100 means no sparks.  between 80 and 100 do two sparks.  60 and 80 is
		// four, etc.
		auto pm = model_get(sip->model_num);
		max_allowed_sparks = get_max_sparks(&Objects[objnum]);
		num_sparks = (int)((100.0f - p_objp->initial_hull) / 5.0f);
		if (num_sparks > max_allowed_sparks)
			num_sparks = max_allowed_sparks;

		for (iLoop = 0; iLoop < num_sparks; iLoop++)
		{
			// DA 10/20/98 - sparks must be chosen on the hull and not any submodel
			vec3d v1 = submodel_get_random_point(sip->model_num, pm->detail[0]);
			ship_hit_sparks_no_rotate(&Objects[objnum], &v1);
		}
	}

	// in mission, we add a log entry -- set ship positions for ships not in wings, and then do
	// warpin effect
	if ((Game_mode & GM_IN_MISSION) && (!Fred_running))
	{
		mission_log_add_entry(LOG_SHIP_ARRIVED, shipp->ship_name, NULL);

		if (!Game_restoring)
		{
			// if this ship isn't in a wing, determine its arrival location
			if (shipp->wingnum == -1)
			{
				// multiplayer clients set the arrival location of ships to be at location since their
				// position has already been determined.  Don't actually set the variable since we
				// don't want the warp effect to show if coming from a dock bay.
				auto location = p_objp->arrival_location;

				if (MULTIPLAYER_CLIENT)
					location = ArrivalLocation::AT_LOCATION;

				anchor_objnum = mission_set_arrival_location(p_objp->arrival_anchor, location, p_objp->arrival_distance, objnum, p_objp->arrival_path_mask, NULL, NULL);

				// Goober5000 - warpin start moved to parse_create_object
			}
		}
		
		// possibly add this ship to a hotkey set
		// Ships can now have both a ship-hotkey and a wing-hotkey -- FSF
		if (shipp->hotkey != -1)
			mission_hotkey_mf_add(shipp->hotkey, shipp->objnum, HOTKEY_MISSION_FILE_ADDED);
		if ((shipp->wingnum != -1) && (Wings[shipp->wingnum].hotkey != -1))
			mission_hotkey_mf_add(Wings[shipp->wingnum].hotkey, shipp->objnum, HOTKEY_MISSION_FILE_ADDED);

		// possibly add this ship to the hud escort list
		if (shipp->flags[Ship::Ship_Flags::Escort])
			hud_add_remove_ship_escort(objnum, 1);
	}

	// for multiplayer games, make a call to the network code to assign the object signature
	// of the newly created object.  The network host of the netgame will always assign a signature
	// to a newly created object.  The network signature will get to the clients of the game in
	// different manners depending on whether or not an individual ship or a wing was created.
	if (Game_mode & GM_MULTIPLAYER)
	{
		Objects[objnum].net_signature = p_objp->net_signature;

		// Goober5000 - for an initially docked group, only send the packet for the dock leader... this is necessary so that the
		// docked hierarchy of objects can be created in the right order on the client side
		if (!object_is_docked(p_objp) || (p_objp->flags[Mission::Parse_Object_Flags::SF_Dock_leader]))
		{
			if ((Game_mode & GM_IN_MISSION) && MULTIPLAYER_MASTER && (p_objp->wingnum == -1))
				send_ship_create_packet(&Objects[objnum], (p_objp == Arriving_support_ship));
		}
		// also add this ship to the multi ship tracking and interpolation struct
		multi_rollback_ship_record_add_ship(objnum);
	}

	// If the ship is in a wing, this will be done in mission_set_wing_arrival_location() instead
	// If the ship is in a wing, but the wing is docked then addition of bool brought_in_docked_wing accounts for that status --wookieejedi
	if (Game_mode & GM_IN_MISSION && ((shipp->wingnum == -1) || (brought_in_docked_wing))) {
		object *anchor_objp = (anchor_objnum >= 0) ? &Objects[anchor_objnum] : nullptr;

		if (scripting::hooks::OnShipArrive->isActive()) {
			scripting::hooks::OnShipArrive->run(scripting::hooks::ShipArriveConditions{ shipp, p_objp->arrival_location, anchor_objp },
				scripting::hook_param_list(
					scripting::hook_param("Ship", 'o', &Objects[objnum]),
					scripting::hook_param("Parent", 'o', anchor_objp, anchor_objp != nullptr)
				));
		}
	}

	if (!Fred_running) {
		// if this is an asteroid target, add it to the list
		for (SCP_string& name : Asteroid_field.target_names) {
			if (stricmp(name.c_str(), shipp->ship_name) == 0) {
				asteroid_add_target(&Objects[objnum]);
				break;
			}
		}
	}

	// assign/update parse object in ship registry entry if needed
	// (this is unrelated to ship registry state management and is only here because apparently in-game joining needs it;
	// in the normal course of ship creation, the pointers and status are updated elsewhere)
	auto ship_it = Ship_registry_map.find(shipp->ship_name);
	if (ship_it != Ship_registry_map.end()) {
		auto entry = &Ship_registry[ship_it->second];

		if (entry->status == ShipStatus::INVALID) {
			Warning(LOCATION, "Potential bug: ship registry status for %s is INVALID", shipp->ship_name);
		}

		// arriving support ships have unique housekeeping and are not in Parse_objects
		if (p_objp == Arriving_support_ship) {
			if (entry->pobj_num < -1 || entry->pobj_num >= 0) {
				Warning(LOCATION, "Potential bug: an arriving support ship %s has a bogus pobj_num index %d", shipp->ship_name, entry->pobj_num);
				entry->pobj_num = -1;
			}
		}
		else {
			int pobj_num = POBJ_INDEX(p_objp);
			if (!SCP_vector_inbounds(Parse_objects, pobj_num)) {
				Warning(LOCATION, "Potential bug: ship registry parse object for %s is not listed in Parse_objects", shipp->ship_name);
				entry->pobj_num = -1;
			}
			else if (entry->pobj_num == -1) {
				Warning(LOCATION, "Potential bug: ship registry parse object for %s is not assigned", shipp->ship_name);
				entry->pobj_num = pobj_num;
			}
			else if (!SCP_vector_inbounds(Parse_objects, entry->pobj_num)) {
				Warning(LOCATION, "Potential bug: ship registry parse object for %s is out of bounds", shipp->ship_name);
				entry->pobj_num = pobj_num;
			}
			else if (entry->pobj_num != pobj_num) {
				Warning(LOCATION, "Potential bug: ship registry parse object index for %s (%d) is different from its expected value (%d)", shipp->ship_name, entry->pobj_num, pobj_num);
				entry->pobj_num = pobj_num;
			}
		}
	}

	// now that everything has been set up, do some sanity checks
#ifndef NDEBUG
	{
		auto pm = model_get(sip->model_num);
		Assertion(pm->n_guns == sip->num_primary_banks, "For %s (%s), the number of primary banks in the ship class does not match the number of gun banks in the model!", sip->name, pm->filename);
		Assertion(shipp->weapons.num_primary_banks <= pm->n_guns, "For %s (%s), the number of primary banks in the ship is greater than the number of gun banks in the model!", shipp->ship_name, pm->filename);
		Assertion(pm->n_missiles == sip->num_secondary_banks, "For %s (%s), the number of secondary banks in the ship class does not match the number of missile banks in the model!", sip->name, pm->filename);
		Assertion(shipp->weapons.num_secondary_banks <= pm->n_missiles, "For %s (%s), the number of secondary banks in the ship is greater than the number of missile banks in the model!", shipp->ship_name, pm->filename);
	}
#endif

	return objnum;
}

/**
 * There are a bunch of assumptions in the code that, in FS2, the wing will be created first, and
 * then it will create its component ships.  If a wing arrives because all its ships were docked
 * to something else, these assumptions are turned inside out.  So we have to sort of bootstrap
 * the creation of the wing by running a subset of the code from parse_wing_create_ships().
 */
void parse_bring_in_docked_wing(p_object *p_objp, int wingnum, int shipnum)
{
	Assert(!Fred_running);
	Assert(p_objp != NULL);
	Assert(wingnum >= 0);
	Assert(shipnum >= 0);
	wing *wingp = &Wings[wingnum];

	// link ship and wing together
	// (do this first because mission log relies on ship_index)
	wingp->ship_index[p_objp->pos_in_wing] = shipnum;
	Ships[shipnum].wingnum = wingnum;

	// has this wing arrived at all yet?
	if (wingp->current_wave == 0)
	{
		wingp->current_wave++;
		mission_log_add_entry( LOG_WING_ARRIVED, wingp->name, NULL, wingp->current_wave );
		wingp->wave_delay_timestamp = TIMESTAMP::invalid();
	}
	// how did we get more than one wave here?
	else if (wingp->current_wave > 1)
		Error(LOCATION, "Wing %s was created from docked ships but somehow has more than one wave!", wingp->name);

	// increment tallies
	wingp->total_arrived_count++;
	wingp->current_count++;
	// make sure we haven't created too many ships
	Assert(wingp->current_count <= MAX_SHIPS_PER_WING);

	// at this point the wing has arrived, so handle the stuff for this particular ship

	// set up wingman status index
	hud_wingman_status_set_index(wingp, &Ships[shipnum], p_objp);

	// copy to parse object
	p_objp->wing_status_wing_index = Ships[shipnum].wing_status_wing_index;
	p_objp->wing_status_wing_pos = Ships[shipnum].wing_status_wing_pos;

	// handle AI
	ai_info *aip = &Ai_info[Ships[shipnum].ai_index];

	if (wingp->flags[Ship::Wing_Flags::No_dynamic])
		aip->ai_flags.set(AI::AI_Flags::No_dynamic);

	// copy any goals from the wing to the newly created ship
	for (int index = 0; index < MAX_AI_GOALS; index++)
	{
		if (wingp->ai_goals[index].ai_mode != AI_GOAL_NONE)
			ai_copy_mission_wing_goal(&wingp->ai_goals[index], aip);
	}
}

// Goober5000
void resolve_parse_flags(object *objp, flagset<Mission::Parse_Object_Flags> &parse_flags)
{
    Assert(objp != NULL);
    ship *shipp = &Ships[objp->instance];

    if (parse_flags[Mission::Parse_Object_Flags::SF_Cargo_known])
        shipp->flags.set(Ship::Ship_Flags::Cargo_revealed);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Ignore_count])
        shipp->flags.set(Ship::Ship_Flags::Ignore_count);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Protected])
        objp->flags.set(Object::Object_Flags::Protected);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Reinforcement])
    {
        //Individual ships in wings can't be reinforcements - FUBAR
        if (shipp->wingnum >= 0)
        {
            Warning(LOCATION, "Ship %s is a reinforcement unit but is a member of a wing. Ignoring reinforcement flag.", shipp->ship_name);
        }
        else
        {
            shipp->flags.set(Ship::Ship_Flags::Reinforcement);
        }
    }

    if ((parse_flags[Mission::Parse_Object_Flags::OF_No_shields]) && (parse_flags[Mission::Parse_Object_Flags::OF_Force_shields_on]))
    {
        Warning(LOCATION, "The parser found a ship with both the \"force-shields-on\" and \"no-shields\" flags; this is inconsistent!");
    }
    if (parse_flags[Mission::Parse_Object_Flags::OF_No_shields])
        objp->flags.set(Object::Object_Flags::No_shields);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Escort])
        shipp->flags.set(Ship::Ship_Flags::Escort);

    // P_OF_PLAYER_START is handled in parse_create_object_sub

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_arrival_music])
        shipp->flags.set(Ship::Ship_Flags::No_arrival_music);

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_arrival_warp])
        shipp->flags.set(Ship::Ship_Flags::No_arrival_warp);

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_departure_warp])
        shipp->flags.set(Ship::Ship_Flags::No_departure_warp);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Ship_locked])
        shipp->flags.set(Ship::Ship_Flags::Ship_locked);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Weapons_locked])
        shipp->flags.set(Ship::Ship_Flags::Weapons_locked);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Locked]) {
        shipp->flags.set(Ship::Ship_Flags::Ship_locked);
        shipp->flags.set(Ship::Ship_Flags::Weapons_locked);
    }

    if (parse_flags[Mission::Parse_Object_Flags::OF_Invulnerable])
        objp->flags.set(Object::Object_Flags::Invulnerable);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Hidden_from_sensors])
        shipp->flags.set(Ship::Ship_Flags::Hidden_from_sensors);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Scannable])
        shipp->flags.set(Ship::Ship_Flags::Scannable);

    // P_AIF_KAMIKAZE, P_AIF_NO_DYNAMIC, and P_SF_RED_ALERT_CARRY are handled in parse_create_object_sub

    if (parse_flags[Mission::Parse_Object_Flags::OF_Beam_protected])
        objp->flags.set(Object::Object_Flags::Beam_protected);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Flak_protected])
        objp->flags.set(Object::Object_Flags::Flak_protected);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Laser_protected])
        objp->flags.set(Object::Object_Flags::Laser_protected);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Missile_protected])
        objp->flags.set(Object::Object_Flags::Missile_protected);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Guardian])
        shipp->ship_guardian_threshold = SHIP_GUARDIAN_THRESHOLD_DEFAULT;

    if (parse_flags[Mission::Parse_Object_Flags::SF_Vaporize])
        shipp->flags.set(Ship::Ship_Flags::Vaporize);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Stealth])
        shipp->flags.set(Ship::Ship_Flags::Stealth);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Friendly_stealth_invis])
        shipp->flags.set(Ship::Ship_Flags::Friendly_stealth_invis);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Dont_collide_invis])
        shipp->flags.set(Ship::Ship_Flags::Dont_collide_invis);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Primitive_sensors])
        shipp->flags.set(Ship::Ship_Flags::Primitive_sensors);

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_subspace_drive])
        shipp->flags.set(Ship::Ship_Flags::No_subspace_drive);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Nav_carry_status])
        shipp->flags.set(Ship::Ship_Flags::Navpoint_carry);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Affected_by_gravity])
        shipp->flags.set(Ship::Ship_Flags::Affected_by_gravity);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Toggle_subsystem_scanning])
        shipp->flags.set(Ship::Ship_Flags::Toggle_subsystem_scanning);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Targetable_as_bomb])
        objp->flags.set(Object::Object_Flags::Targetable_as_bomb);

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_builtin_messages])
        shipp->flags.set(Ship::Ship_Flags::No_builtin_messages);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Primaries_locked])
        shipp->flags.set(Ship::Ship_Flags::Primaries_locked);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Secondaries_locked])
        shipp->flags.set(Ship::Ship_Flags::Secondaries_locked);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Set_class_dynamically])
        shipp->flags.set(Ship::Ship_Flags::Set_class_dynamically);

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_death_scream])
        shipp->flags.set(Ship::Ship_Flags::No_death_scream);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Always_death_scream])
        shipp->flags.set(Ship::Ship_Flags::Always_death_scream);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Nav_needslink])
        shipp->flags.set(Ship::Ship_Flags::Navpoint_needslink);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Hide_ship_name])
        shipp->flags.set(Ship::Ship_Flags::Hide_ship_name);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Lock_all_turrets_initially])
        shipp->flags.set(Ship::Ship_Flags::Lock_all_turrets_initially);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Afterburner_locked])
        shipp->flags.set(Ship::Ship_Flags::Afterburner_locked);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Force_shields_on])
        shipp->flags.set(Ship::Ship_Flags::Force_shields_on);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Dont_change_position])
        objp->flags.set(Object::Object_Flags::Dont_change_position);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Dont_change_orientation])
        objp->flags.set(Object::Object_Flags::Dont_change_orientation);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Immobile])
    {
        // handle "soft deprecation" of Immobile by setting the two half-flags, but only in FRED
        // (FRED has dialog support for the two half-flags but not the legacy Immobile flag)
        if (Fred_running)
        {
            objp->flags.set(Object::Object_Flags::Dont_change_position);
            objp->flags.set(Object::Object_Flags::Dont_change_orientation);

            // keep track of migrated ships
            Fred_migrated_immobile_ships.insert(objp->instance);
        }
        else
            objp->flags.set(Object::Object_Flags::Immobile);
    }

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_ets])
        shipp->flags.set(Ship::Ship_Flags::No_ets);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Cloaked])
        shipp->flags.set(Ship::Ship_Flags::Cloaked);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Scramble_messages])
        shipp->flags.set(Ship::Ship_Flags::Scramble_messages);

    if (parse_flags[Mission::Parse_Object_Flags::OF_No_collide])
        objp->flags.remove(Object::Object_Flags::Collides);

    if (parse_flags[Mission::Parse_Object_Flags::SF_No_disabled_self_destruct])
        shipp->flags.set(Ship::Ship_Flags::No_disabled_self_destruct);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Dock_leader])
        shipp->flags.set(Ship::Ship_Flags::Dock_leader);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Warp_broken])
        shipp->flags.set(Ship::Ship_Flags::Warp_broken);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Warp_never])
        shipp->flags.set(Ship::Ship_Flags::Warp_never);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Has_display_name])
        shipp->flags.set(Ship::Ship_Flags::Has_display_name);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Hide_mission_log])
        shipp->flags.set(Ship::Ship_Flags::Hide_mission_log);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Same_arrival_warp_when_docked])
        shipp->flags.set(Ship::Ship_Flags::Same_arrival_warp_when_docked);

    if (parse_flags[Mission::Parse_Object_Flags::SF_Same_departure_warp_when_docked])
		shipp->flags.set(Ship::Ship_Flags::Same_departure_warp_when_docked);

    if (parse_flags[Mission::Parse_Object_Flags::OF_Attackable_if_no_collide])
		objp->flags.set(Object::Object_Flags::Attackable_if_no_collide);

	if (parse_flags[Mission::Parse_Object_Flags::SF_Fail_sound_locked_primary])
		shipp->flags.set(Ship::Ship_Flags::Fail_sound_locked_primary);

	if (parse_flags[Mission::Parse_Object_Flags::SF_Fail_sound_locked_secondary])
		shipp->flags.set(Ship::Ship_Flags::Fail_sound_locked_secondary);

	if (parse_flags[Mission::Parse_Object_Flags::SF_Aspect_immune])
		shipp->flags.set(Ship::Ship_Flags::Aspect_immune);

	if (parse_flags[Mission::Parse_Object_Flags::SF_Cannot_perform_scan])
		shipp->flags.set(Ship::Ship_Flags::Cannot_perform_scan);

	if (parse_flags[Mission::Parse_Object_Flags::SF_No_targeting_limits])
		shipp->flags.set(Ship::Ship_Flags::No_targeting_limits);

	if (parse_flags[Mission::Parse_Object_Flags::SF_From_player_wing])
		shipp->flags.set(Ship::Ship_Flags::From_player_wing);
}

void fix_old_special_explosions(p_object *p_objp, int variable_index) 
{
	int i;

	Assertion(!(p_objp->use_special_explosion), "Mission appears to be using both the new and old method of special explosions for %s. Old method values used", p_objp->name); 
	
	// check all the variables are valid
	for ( i = variable_index; i < (variable_index + BLOCK_EXP_SIZE); i++ ) {
		if (!( Block_variables[i].type & SEXP_VARIABLE_BLOCK )) {
			Warning (LOCATION, "%s is using the old special explosions method but does not appear to have variables for all the values", p_objp->name);
			return;
		}
	}

	p_objp->use_special_explosion = true;

	p_objp->special_exp_damage = atoi(Block_variables[variable_index+DAMAGE].text);
	p_objp->special_exp_blast = atoi(Block_variables[variable_index+BLAST].text);
	p_objp->special_exp_inner = atoi(Block_variables[variable_index+INNER_RAD].text);
	p_objp->special_exp_outer = atoi(Block_variables[variable_index+OUTER_RAD].text);
	p_objp->use_shockwave = (atoi(Block_variables[variable_index+PROPAGATE].text) ? 1:0);
	p_objp->special_exp_shockwave_speed = atoi(Block_variables[variable_index+SHOCK_SPEED].text);
	p_objp->special_exp_deathroll_time = 0;
}

void fix_old_special_hits(p_object *p_objp, int variable_index)
{
	int i; 

	Assertion( ((p_objp->special_hitpoints == 0) && (p_objp->special_shield == -1)),"Mission appears to be using both the new and old method of special hitpoints for %s", p_objp->name);  
	
	// check all the variables are valid
	for ( i = variable_index; i < (variable_index + BLOCK_HIT_SIZE); i++ ) {
		if (!( Block_variables[i].type & SEXP_VARIABLE_BLOCK )) {
			Warning (LOCATION, "%s is using the old special hitpoints method but does not appear to have variables for all the values", p_objp->name);
			return;
		}
	}

	p_objp->special_hitpoints = atoi(Block_variables[variable_index+HULL_STRENGTH].text);
	p_objp->special_shield = atoi(Block_variables[variable_index+SHIELD_STRENGTH].text);
}

// this will be called when Parse_objects is cleared between missions and upon shutdown
p_object::~p_object()
{
	dock_free_dock_list(this);
}

const char* p_object::get_display_name() {
	if (has_display_name()) {
		return display_name.c_str();
	} else {
		return name;
	}
}
bool p_object::has_display_name() {
	return flags[Mission::Parse_Object_Flags::SF_Has_display_name];
}

extern int parse_warp_params(const WarpParams *inherit_from, WarpDirection direction, const char *info_type_name, const char *sip_name, bool set_supercap_warp_physics = false);

/**
 * Mp points at the text of an object, which begins with the "$Name:" field.
 * Snags all object information.  Creating the ship now only happens after everything has been parsed.
 *
 * @param pm Mission
 * @param flag is parameter that is used to tell what kind information we are retrieving from the mission.
 * if we are just getting player starts, then don't create the objects
 * @param p_objp Object
 */
int parse_object(mission *pm, int  /*flag*/, p_object *p_objp)
{
	int	i;
    char name[NAME_LENGTH];
	ship_info *sip;

	Assert(pm != NULL);

	required_string("$Name:");
	stuff_string(p_objp->name, F_NAME, NAME_LENGTH);
	if (mission_parse_find_parse_object(p_objp->name))
		error_display(0, NOX("Redundant ship name: %s\n"), p_objp->name);

	// if this name has a hash, create a default display name
	if (get_pointer_to_first_hash_symbol(p_objp->name)) {
		p_objp->display_name = p_objp->name;
		end_string_at_first_hash_symbol(p_objp->display_name);
		p_objp->flags.set(Mission::Parse_Object_Flags::SF_Has_display_name);
	}

	if (optional_string("$Display Name:")) {
		stuff_string(p_objp->display_name, F_NAME);
		p_objp->flags.set(Mission::Parse_Object_Flags::SF_Has_display_name);
	}

	find_and_stuff("$Class:", &p_objp->ship_class, F_NAME, Ship_class_names, Ship_info.size(), "ship class");
	if (p_objp->ship_class < 0)
	{
		if (Fred_running) {
			Warning(LOCATION, "Ship \"%s\" has an invalid ship type (ships.tbl probably changed).  Making it type 0\n", p_objp->name);
		} 
		else {
			mprintf(("MISSIONS: Ship \"%s\" has an invalid ship type (ships.tbl probably changed).  Making it type 0\n", p_objp->name));
		}

		p_objp->ship_class = 0;
		Num_unknown_ship_classes++;
	}
	sip = &Ship_info[p_objp->ship_class];

	// initialize class-specific fields
	p_objp->ai_class = sip->ai_class;
	p_objp->warpin_params_index = sip->warpin_params_index;
	p_objp->warpout_params_index = sip->warpout_params_index;
	p_objp->ship_max_shield_strength = sip->max_shield_strength;
	p_objp->ship_max_hull_strength = sip->max_hull_strength;
	Assert(p_objp->ship_max_hull_strength > 0.0f);	// Goober5000: div-0 check (not shield because we might not have one)
	p_objp->max_shield_recharge = sip->max_shield_recharge;
	p_objp->replacement_textures = sip->replacement_textures;	// initialize our set with the ship class set, which may be empty

	// Karajorma - See if there are any alternate classes specified for this ship. 
	// The alt class can either be a variable or a ship class name
	char alt_ship_class[TOKEN_LENGTH > NAME_LENGTH ? TOKEN_LENGTH : NAME_LENGTH];
	int is_variable; 

	while (optional_string("$Alt Ship Class:")) {	
		alt_class new_alt_class; 

		is_variable = get_string_or_variable(alt_ship_class); 

		if (is_variable) {
			new_alt_class.variable_index = get_index_sexp_variable_name(alt_ship_class);
			if(new_alt_class.variable_index >= 0) {
				new_alt_class.ship_class = ship_info_lookup(Sexp_variables[new_alt_class.variable_index].text);
			}
			else {
				new_alt_class.ship_class = -1;
			}
		}
		else {
			new_alt_class.variable_index = -1;
			new_alt_class.ship_class = ship_info_lookup(alt_ship_class); 
		}

		if (new_alt_class.ship_class < 0 ) {
			if (!Fred_running) {
				Warning(LOCATION, "Ship \"%s\" has an invalid Alternate Ship Class type (ships.tbl probably changed). Skipping this entry", p_objp->name); 
				continue; 
			}
			else {
				// incorrect initial values for a variable can be fixed in FRED
				if (new_alt_class.variable_index != -1) {
					Warning(LOCATION, "Ship \"%s\" has an invalid Alternate Ship Class type.", p_objp->name); 
				}
				// but there is little we can do if someone spelled a ship class incorrectly
				else {
					Warning(LOCATION, "Ship \"%s\" has an invalid Alternate Ship Class type. Skipping this entry", p_objp->name); 
					continue; 			
				}
			}
		}

		if (optional_string("+Default Class:")) {
			new_alt_class.default_to_this_class = true; 
		}
		else {
			new_alt_class.default_to_this_class = false;
		}

		p_objp->alt_classes.push_back(new_alt_class); 
	}

	// if this is a multiplayer dogfight mission, skip support ships
	if(MULTI_DOGFIGHT && (sip->flags[Ship::Info_Flags::Support]))
		return 0;

	// optional alternate name type
	if(optional_string("$Alt:"))
	{
		// alternate name
		stuff_string(name, F_NAME, NAME_LENGTH);

		// try and find the alternate name
		p_objp->alt_type_index = mission_parse_lookup_alt(name);
		if(p_objp->alt_type_index < 0)
			WarningEx(LOCATION, "Mission %s\nError looking up alternate ship type name %s!\n", pm->name, name);
		else
			mprintf(("Using alternate ship type name: %s\n", name));
	}

	// optional callsign
	if(optional_string("$Callsign:"))
	{
		// alternate callsign
		stuff_string(name, F_NAME, NAME_LENGTH);

		// try and find the callsign
		p_objp->callsign_index = mission_parse_lookup_callsign(name);
		if(p_objp->callsign_index < 0)
			WarningEx(LOCATION, "Mission %s\nError looking up callsign %s!\n", pm->name, name);
		else
			mprintf(("Using callsign: %s\n", name));
	}

	auto temp_team_names = std::unique_ptr<const char*[]>(new const char*[Iff_info.size()]);
	for (i = 0; i < (int)Iff_info.size(); i++)
		temp_team_names[i] = Iff_info[i].iff_name;

	find_and_stuff("$Team:", &p_objp->team, F_NAME, temp_team_names.get(), Iff_info.size(), "team name");

	// save current team for loadout purposes, so that in multi we always respawn
	// from the original loadout slot even if the team changes
	p_objp->loadout_team = p_objp->team;

	if (optional_string("$Team Color Setting:")) {
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);
		p_objp->team_color_setting = temp;

		//If team color is not default then verify
		if (stricmp(p_objp->team_color_setting.c_str(), sip->default_team_name.c_str()) != 0) {
			if (Team_Colors.find(p_objp->team_color_setting) == Team_Colors.end()) {
				mprintf(("Invalid team color specified in mission file for ship %s, resetting to default\n", p_objp->name));
				p_objp->team_color_setting = sip->default_team_name;
			}
		}
	}

	required_string("$Location:");
	stuff_vec3d(&p_objp->pos);

	required_string("$Orientation:");
	stuff_matrix(&p_objp->orient);

	// legacy code, not even used in FS1
	if (optional_string("$IFF:"))
	{
		stuff_string(name, F_NAME, NAME_LENGTH);
	}
	if (optional_string("$AI Behavior:"))
	{
		stuff_string(name, F_NAME, NAME_LENGTH);
	}

	if (optional_string("+AI Class:")) 
	{
		p_objp->ai_class = match_and_stuff(F_NAME, Ai_class_names, Num_ai_classes, "AI class");

		if (p_objp->ai_class < 0) 
		{
			Warning(LOCATION, "AI Class for ship %s does not exist in ai.tbl. Setting to first available class.\n", p_objp->name);
			p_objp->ai_class = 0;
		}		
	}

	if (optional_string("$AI Goals:"))
		p_objp->ai_goals = get_sexp_main();

	if (!required_string_either("$AI Goals:", "$Cargo 1:"))
	{
		required_string("$AI Goals:");
		p_objp->ai_goals = get_sexp_main();
	}

	int temp;
	find_and_stuff_or_add("$Cargo 1:", &temp, F_NAME, Cargo_names, &Num_cargo, MAX_CARGO, "cargo");
	p_objp->cargo1 = char(temp);
	if (optional_string("$Cargo 2:"))
	{
		stuff_string(name, F_NAME, NAME_LENGTH);
	}

	if (optional_string("$Cargo Title:"))
	{
		stuff_string(p_objp->cargo_title, F_NAME, NAME_LENGTH);
	}

	parse_common_object_data(p_objp);  // get initial conditions and subsys status

	find_and_stuff("$Arrival Location:", &temp, F_NAME, Arrival_location_names, Num_arrival_names, "Arrival Location");
	if (temp >= 0)
		p_objp->arrival_location = static_cast<ArrivalLocation>(temp);

	if (optional_string("+Arrival Distance:"))
	{
		stuff_int(&p_objp->arrival_distance);

		// Goober5000
		if ((p_objp->arrival_distance <= 0) && (
			   (p_objp->arrival_location == ArrivalLocation::NEAR_SHIP)
			|| (p_objp->arrival_location == ArrivalLocation::IN_FRONT_OF_SHIP) || (p_objp->arrival_location == ArrivalLocation::IN_BACK_OF_SHIP)
			|| (p_objp->arrival_location == ArrivalLocation::ABOVE_SHIP) || (p_objp->arrival_location == ArrivalLocation::BELOW_SHIP)
			|| (p_objp->arrival_location == ArrivalLocation::TO_LEFT_OF_SHIP) || (p_objp->arrival_location == ArrivalLocation::TO_RIGHT_OF_SHIP) ))
		{
			Warning(LOCATION, "Arrival distance for ship %s cannot be %d.  Setting to 1.\n", p_objp->name, p_objp->arrival_distance);
			p_objp->arrival_distance = 1;
		}
	}

	if (p_objp->arrival_location != ArrivalLocation::AT_LOCATION)
	{
		required_string("$Arrival Anchor:");
		stuff_string(name, F_NAME, NAME_LENGTH);
		p_objp->arrival_anchor = get_anchor(name);
	}

	p_objp->arrival_path_mask = -1;		// -1 only until resolved
	if (optional_string("+Arrival Paths:"))
	{
		// temporarily use mask to point to the restriction index
		p_objp->arrival_path_mask = add_path_restriction();
	}

	if (optional_string("+Arrival Delay:"))
	{
		int delay;
		stuff_int(&delay);
		if (delay < 0)
		{
			Warning(LOCATION, "Cannot have arrival delay < 0 on ship %s", p_objp->name);
			delay = 0;
		}

		if (!Fred_running)
			p_objp->arrival_delay = -delay;			// use negative numbers to mean we haven't set up a timer yet
		else
			p_objp->arrival_delay = delay;
	}

	required_string("$Arrival Cue:");
	p_objp->arrival_cue = get_sexp_main();

	find_and_stuff("$Departure Location:", &temp, F_NAME, Departure_location_names, Num_arrival_names, "Departure Location");
	if (temp >= 0)
		p_objp->departure_location = static_cast<DepartureLocation>(temp);

	if (p_objp->departure_location != DepartureLocation::AT_LOCATION)
	{
		required_string("$Departure Anchor:");
		stuff_string(name, F_NAME, NAME_LENGTH);
		p_objp->departure_anchor = get_anchor(name);
	}

	p_objp->departure_path_mask = -1;		// -1 only until resolved
	if (optional_string("+Departure Paths:"))
	{
		// temporarily use mask to point to the restriction index
		p_objp->departure_path_mask = add_path_restriction();
	}

	if (optional_string("+Departure Delay:"))
	{
		int delay;
		stuff_int(&delay);
		if (delay < 0)
		{
			Warning(LOCATION, "Cannot have departure delay < 0 (ship %s)", p_objp->name);
			delay = 0;
		}

		if (!Fred_running)
			p_objp->departure_delay = -delay;		// use negative numbers to mean that delay timer not yet set
		else
			p_objp->departure_delay = delay;
	}

	required_string("$Departure Cue:");
	p_objp->departure_cue = get_sexp_main();

	// look for warp parameters
	p_objp->warpin_params_index = parse_warp_params(&Warp_params[p_objp->warpin_params_index], WarpDirection::WARP_IN, "Ship", p_objp->name);
	p_objp->warpout_params_index = parse_warp_params(&Warp_params[p_objp->warpout_params_index], WarpDirection::WARP_OUT, "Ship", p_objp->name);

	// dummy string; not used
	if (optional_string("$Misc Properties:"))
		stuff_string(name, F_NAME, NAME_LENGTH);

	required_string("$Determination:");
	int dummy; 
	stuff_int(&dummy);

    // set flags
    if (optional_string("+Flags:"))
    {
        SCP_vector<SCP_string> unparsed;
        parse_string_flag_list(p_objp->flags, Parse_object_flags, Num_parse_object_flags, &unparsed);
        if (!unparsed.empty()) {
            for (size_t k = 0; k < unparsed.size(); ++k) {
                WarningEx(LOCATION, "Unknown flag in parse object flags: %s", unparsed[k].c_str());
            }
        }
    }

    // second set - Goober5000
    if (optional_string("+Flags2:"))
    {
        SCP_vector<SCP_string> unparsed;
        parse_string_flag_list(p_objp->flags, Parse_object_flags, Num_parse_object_flags, &unparsed);
        if (!unparsed.empty()) {
            for (size_t k = 0; k < unparsed.size(); ++k) {
				// catch typos or deprecations
				if (!stricmp(unparsed[k].c_str(), "no-collide") || !stricmp(unparsed[k].c_str(), "no_collide")) {
					p_objp->flags.set(Mission::Parse_Object_Flags::OF_No_collide);
				}
				else {
					WarningEx(LOCATION, "Unknown flag in parse object flags: %s", unparsed[k].c_str());
				}
            }
        }
    }


	// always store respawn priority, just for ease of implementation
	if(optional_string("+Respawn Priority:"))
		stuff_int(&p_objp->respawn_priority);	

    if (optional_string("+Escort Priority:"))
    {
        stuff_int(&p_objp->escort_priority);
    }

    if (p_objp->flags[Mission::Parse_Object_Flags::OF_Player_start])
    {
        p_objp->flags.set(Mission::Parse_Object_Flags::SF_Cargo_known);	// make cargo known for players
        Player_starts++;
    }

	if (optional_string("$Special Explosion:")) {
		p_objp->use_special_explosion = true;
		bool period_detected = false;

		if (required_string("+Special Exp Damage:")) {
			stuff_int(&p_objp->special_exp_damage);

			if (*Mp == '.') {
				period_detected = true;
				advance_to_eoln(NULL);
			}
		}

		if (required_string("+Special Exp Blast:")) {
			stuff_int(&p_objp->special_exp_blast);

			if (*Mp == '.') {
				period_detected = true;
				advance_to_eoln(NULL);
			}
		}

		if (required_string("+Special Exp Inner Radius:")) {
			stuff_int(&p_objp->special_exp_inner);

			if (*Mp == '.') {
				period_detected = true;
				advance_to_eoln(NULL);
			}
		}

		if (required_string("+Special Exp Outer Radius:")) {
			stuff_int(&p_objp->special_exp_outer);

			if (*Mp == '.') {
				period_detected = true;
				advance_to_eoln(NULL);
			}
		}

		if (optional_string("+Special Exp Shockwave Speed:")) {
			stuff_int(&p_objp->special_exp_shockwave_speed);
			p_objp->use_shockwave = true;

			if (*Mp == '.') {
				period_detected = true;
				advance_to_eoln(NULL);
			}
		}

		if (optional_string("+Special Exp Death Roll Time:")) {
			stuff_int(&p_objp->special_exp_deathroll_time);
		}

		if (period_detected) {
			nprintf(("Warning", "Special explosion attributes have been returned to integer format\n"));
		}
	}

	if (optional_string("+Special Hitpoints:")) {
		stuff_int(&p_objp->special_hitpoints);
	}

	if (optional_string("+Special Shield Points:")) {
		stuff_int(&p_objp->special_shield);
	}

	if (optional_string("+Special Exp index:")) {
		int variable_index; 
		stuff_int(&variable_index);
		fix_old_special_explosions(p_objp, variable_index);
	}

	if (optional_string("+Special Hitpoint index:")) {
		int variable_index; 
		stuff_int(&variable_index);
		fix_old_special_hits(p_objp, variable_index);
	}

	// set custom shield value
	if (p_objp->special_shield != -1) {
		p_objp->ship_max_shield_strength = (float) p_objp->special_shield; 
	}
	
	// set custom hitpoint value
	if (p_objp->special_hitpoints > 0) {
		p_objp->ship_max_hull_strength = (float) p_objp->special_hitpoints; 
	}

	// if the kamikaze flag is set, we should have the next flag
	if (optional_string("+Kamikaze Damage:"))
	{
		int damage;

		stuff_int(&damage);
		p_objp->kamikaze_damage = damage;
	}

	if (optional_string("+Hotkey:"))
	{
		stuff_int(&p_objp->hotkey);
		Assert((p_objp->hotkey >= 0) && (p_objp->hotkey < 10));
	}

	// Goober5000
	while (optional_string("+Docked With:"))
	{
		char docked_with[NAME_LENGTH];
		char docker_point[NAME_LENGTH];
		char dockee_point[NAME_LENGTH];

		// grab docking information
		// (whoever designed the original docking system
		// reversed the dockpoints in the mission file)
		stuff_string(docked_with, F_NAME, NAME_LENGTH);
		required_string("$Docker Point:");
		stuff_string(dockee_point, F_NAME, NAME_LENGTH);
		required_string("$Dockee Point:");
		stuff_string(docker_point, F_NAME, NAME_LENGTH);	

		// make sure we don't overflow the limit
		if (Total_initially_docked >= MAX_SHIPS)
		{
			mprintf(("Too many initially docked instances; skipping...\n"));
			continue;
		}

		// put this information into the Initially_docked array
		strcpy_s(Initially_docked[Total_initially_docked].docker, p_objp->name);
		strcpy_s(Initially_docked[Total_initially_docked].dockee, docked_with);
		strcpy_s(Initially_docked[Total_initially_docked].docker_point, docker_point);
		strcpy_s(Initially_docked[Total_initially_docked].dockee_point, dockee_point);
		Total_initially_docked++;
	}

	// check the optional parameter for destroying the ship before the mission starts.  If this parameter is
	// here, then we need to destroy the ship N seconds before the mission starts (for debris purposes).
	// store the time value here.  We want to create this object for sure.  Set the arrival cue and arrival
	// delay to bogus values
	if (optional_string("+Destroy At:"))
	{
		stuff_int(&p_objp->destroy_before_mission_time);
		if (p_objp->destroy_before_mission_time < 0)
		{
			Warning(LOCATION, "Cannot set a negative 'destroy before mission' value (ship %s)", p_objp->name);
			p_objp->destroy_before_mission_time = 0;
		}

		p_objp->arrival_cue = Locked_sexp_true;
		p_objp->arrival_delay = timestamp(0);
	}

	// check for the optional "orders accepted" string which contains the orders from the default
    // set that this ship will actually listen to
	// Obsolete and only for backwards compatibility
    if (optional_string("+Orders Accepted:"))
    {
		int tmp_orders;
        stuff_int(&tmp_orders);
		
        if (tmp_orders != -1) {
			p_objp->flags.set(Mission::Parse_Object_Flags::SF_Use_unique_orders);

			//Iterate over all player orders for the bitfield, but restrict to those that the integer could actually provide
			for(size_t j = 0; j < Player_orders.size() && j < std::numeric_limits<decltype(tmp_orders)>::digits; j++) {
				if((1 << j) & tmp_orders)
					p_objp->orders_accepted.insert(j + 1); //The first "true" order starts at idx 1, since 0 can be "no order"
			}
		}
    }

	if (optional_string("+Orders Accepted List:"))
	{
		SCP_vector<SCP_string> accepted_flags;
		stuff_string_list(accepted_flags);

		for (const SCP_string& accepted : accepted_flags) {
			for (size_t j = 0; j < Player_orders.size(); j++) {
				if (Player_orders[j].parse_name == accepted)
					p_objp->orders_accepted.insert(j);
			}
		}

		p_objp->flags.set(Mission::Parse_Object_Flags::SF_Use_unique_orders);
	}

	if (optional_string("+Group:"))
		stuff_int(&p_objp->group);

	bool table_score = false; 
	if (optional_string("+Use Table Score:")) {
		table_score = true; 
	}

	if (optional_string("+Score:")) {
		if (!table_score) {
			stuff_int(&p_objp->score);
		}
		// throw away the value the mission file has and use the table value.
		else {
			int temp_score; 
			stuff_int(&temp_score); 
		}
	}
	else {
		table_score = true;
	}
	
	if (table_score) {
		p_objp->score = sip->score;
	}

	if (optional_string("+Assist Score Percentage:")) {
		stuff_float(&p_objp->assist_score_pct);
		// value must be a percentage
		if (p_objp->assist_score_pct < 0) {
			p_objp->assist_score_pct = 0;
		} 
		else if (p_objp->assist_score_pct > 1) {
			p_objp->assist_score_pct = 1;
		}
	}

	// parse the persona index if present
	// For backwards compatbility only
	if (optional_string("+Persona Index:")) {
		stuff_int(&p_objp->persona_index);
		if (p_objp->persona_index < -1 || p_objp->persona_index >= (int)Personas.size()) {
			Warning(LOCATION, "Persona index %d for %s is out of range!  Setting to -1.", p_objp->persona_index, p_objp->name);
			p_objp->persona_index = -1;
		}
	}

	if (optional_string("+Persona Name:")) {
		SCP_string persona;
		stuff_string(persona, F_NAME);
		p_objp->persona_index = message_persona_name_lookup(persona.c_str());
	}

	// texture replacement - Goober5000
	if (optional_string("$Texture Replace:") || optional_string("$Duplicate Model Texture Replace:"))
	{
		texture_replace tr;
		char *p;

		tr.from_table = false;

		while (optional_string("+old:"))
		{
			strcpy_s(tr.ship_name, p_objp->name);
			tr.new_texture_id = -1;

			stuff_string(tr.old_texture, F_NAME, MAX_FILENAME_LEN);
			required_string("+new:");
			stuff_string(tr.new_texture, F_NAME, MAX_FILENAME_LEN);

			// get rid of extensions
			p = strchr(tr.old_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", tr.old_texture));
				*p = 0;
			}
			p = strchr(tr.new_texture, '.');
			if (p)
			{
				mprintf(("Extraneous extension found on replacement texture %s!\n", tr.new_texture));
				*p = 0;
			}

			// add it if we aren't over the limit
			if (p_objp->replacement_textures.size() < MAX_MODEL_TEXTURES)
				p_objp->replacement_textures.push_back(tr);
			else
				mprintf(("Too many replacement textures specified for ship '%s'!\n", p_objp->name));
		}
	}

	// for multiplayer, assign a network signature to this parse object.  Doing this here will
	// allow servers to use the signature with clients when creating new ships, instead of having
	// to pass ship names all the time
	if (Game_mode & GM_MULTIPLAYER)
		p_objp->net_signature = multi_assign_network_signature(MULTI_SIG_SHIP);

	// this is a valid/legal ship to create
	return 1;
}

void mission_parse_handle_late_arrivals(p_object *p_objp)
{
	ship_info *sip = NULL;

	// only for objects which show up after the start of a mission
	if (p_objp->created_object != NULL)
		return;

	Assert( p_objp->ship_class >= 0 );

	sip = &Ship_info[p_objp->ship_class];

	// we need the model to process the texture set, so go ahead and load it now
	sip->model_num = model_load(sip->pof_file, sip);
}

// Goober5000 - I split this because 1) it's clearer; and 2) initially multiple docked ships would have been
// insanely difficult otherwise
//
// Maybe create the object.
// Don't create the new ship blindly.  First, check the sexp for the arrival cue
// to determine when this ship should arrive.  If not right away, stick this ship
// onto the ship arrival list to be looked at later.  Also check to see if it should use the
// wings arrival cue.  The ship may get created later depending on whether or not the wing
// is created.
// Always create ships when FRED is running.
void mission_parse_maybe_create_parse_object(p_object *pobjp)
{
	// Bail if it was already created.  This should only happen when we previously
	// created all the objects in a docked group simultaneously.
	if (pobjp->created_object != NULL)
	{
		Assert(object_is_docked(pobjp));
		return;
	}

	// Goober5000
	// shunt this guy to the arrival list if he meets one of the following conditions:
	// 1) he's docked but not the dock leader
	// 2) this is FS2 (i.e. not FRED2) AND he meets one of the following conditions:
	//    a) he's not cued to arrive yet
	//    b) his arrival delay hasn't elapsed
	//    c) he's reinforcement
    if ((object_is_docked(pobjp) && !(pobjp->flags[Mission::Parse_Object_Flags::SF_Dock_leader])) || 
        (!Fred_running && (!eval_sexp(pobjp->arrival_cue) || 
            !timestamp_elapsed(pobjp->arrival_delay) || 
            (pobjp->flags[Mission::Parse_Object_Flags::SF_Reinforcement]))))
	{
		// we can't add ships getting destroyed to the arrival list!!!
		Assert (pobjp->destroy_before_mission_time < 0);

		// add to arrival list
		list_append(&Ship_arrival_list, pobjp);

		// we need to deal with replacement textures now, so that texture page-in will work properly
		mission_parse_handle_late_arrivals(pobjp);
	}
	// ingame joiners bail here.
	else if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN))
	{
		return;
	}
	// the ship is present at the beginning of the mission, so we create it
	else
	{
		int	real_objnum = parse_create_object(pobjp);	// this object may later get destroyed depending on wing status!!!!

		// if the ship is supposed to be destroyed before the mission, then blow up the ship and mark the pieces
		// as last forever.  Only call this stuff when you are blowing up the ship
		if (pobjp->destroy_before_mission_time >= 0)
		{
			object *objp = &Objects[real_objnum];

			// FreeSpace
			if (!Fred_running)
			{
				shipfx_blow_up_model(objp, 0, 0, &objp->pos);
				objp->flags.set(Object::Object_Flags::Should_be_dead);

				// Make sure that the ship is marked as destroyed so the AI doesn't freak out later
				ship_add_exited_ship(&Ships[objp->instance], Ship::Exit_Flags::Destroyed);

				// Same with the ship registry so that SEXPs don't refer to phantom ships
				auto entry = &Ship_registry[Ship_registry_map[pobjp->name]];
				entry->status = ShipStatus::EXITED;
				entry->objnum = -1;
				entry->shipnum = -1;
				entry->cleanup_mode = SHIP_DESTROYED;

				// once the ship is exploded, find the debris pieces belonging to this object, mark them
				// as not to expire, and move them forward in time N seconds
				for (auto &db: Debris)
				{
					if (!(db.flags[Debris_Flags::Used]))		// not used, move onto the next one.
						continue;
					if (db.source_objnum != real_objnum)		// not from this ship, move to next one
						continue;
					
					debris_remove_from_hull_list(&db);
					db.flags.set(Debris_Flags::DoNotExpire);   // mark as don't expire
					db.lifeleft = -1.0f;						// be sure that lifeleft == -1.0 so that it really doesn't expire!

					// now move the debris along its path for N seconds
					objp = &Objects[db.objnum];
					physics_sim(&objp->pos, &objp->orient, &objp->phys_info, &The_mission.gravity, (float) pobjp->destroy_before_mission_time);
				}
			}
			// FRED
			else
			{
				// be sure to set the variable in the ships structure for the final death time!!!
				Ships[objp->instance].final_death_time = pobjp->destroy_before_mission_time;
				Ships[objp->instance].flags.set(Ship::Ship_Flags::Kill_before_mission);
			}
		}
	}
}

void parse_common_object_data(p_object *p_objp)
{
	int i;

	// Genghis: used later for subsystem checking
	auto sip = &Ship_info[p_objp->ship_class];

	// now change defaults if present
	if (optional_string("+Initial Velocity:"))
		stuff_int(&p_objp->initial_velocity);
	if (optional_string("+Initial Hull:"))
		stuff_int(&p_objp->initial_hull);
	if (optional_string("+Initial Shields:"))
		stuff_int(&p_objp->initial_shields);

	p_objp->subsys_index = Subsys_index;
	while (optional_string("+Subsystem:")) {
		i = allocate_subsys_status();

		p_objp->subsys_count++;
		stuff_string(Subsys_status[i].name, F_NAME, NAME_LENGTH);
		
		// Genghis: check that the subsystem name makes sense for this ship type
		if (subsystem_stricmp(Subsys_status[i].name, NOX("pilot")))
		{
			int j;
			for (j=0; j < sip->n_subsystems; ++j)
				if (!subsystem_stricmp(sip->subsystems[j].subobj_name, Subsys_status[i].name))
					break;
			//if (j == sip->n_subsystems)
				//Warning(LOCATION, "Ship \"%s\", class \"%s\"\nUnknown subsystem \"%s\" found in mission!", objp->name, sip->name, Subsys_status[i].name);
		}

		if (optional_string("$Damage:"))
			stuff_float(&Subsys_status[i].percent);

		if (optional_string("+Cargo Name:")) {
			char cargo_name[NAME_LENGTH];
			stuff_string(cargo_name, F_NAME, NAME_LENGTH);
			int index = string_lookup(cargo_name, Cargo_names, Num_cargo, "cargo", false);
			if (index == -1) {
				if (Num_cargo < MAX_CARGO) {
					index = Num_cargo;
					strcpy(Cargo_names[Num_cargo++], cargo_name);
				}
				else {
					WarningEx(LOCATION, "Maximum number of cargo names (%d) exceeded, defaulting to Nothing!", MAX_CARGO);
					index = 0;
				}
			}
			Subsys_status[i].subsys_cargo_name = index;
		}

		if (optional_string("+Cargo Title:")) {
			stuff_string(Subsys_status[i].subsys_cargo_title, F_NAME, NAME_LENGTH);
		}

		if (optional_string("+AI Class:"))
		{
			Subsys_status[i].ai_class = match_and_stuff(F_NAME, Ai_class_names, Num_ai_classes, "AI class");

			if (Subsys_status[i].ai_class < 0)
			{
				Warning(LOCATION, "AI Class for ship %s and subsystem %s does not exist in ai.tbl. Setting to first available class.\n", p_objp->name, Subsys_status[i].name);
				Subsys_status[i].ai_class = 0;
			}
		}

		if (optional_string("+Primary Banks:"))
			stuff_int_list(Subsys_status[i].primary_banks, MAX_SHIP_PRIMARY_BANKS, WEAPON_LIST_TYPE);

		// Goober5000
		if (optional_string("+Pbank Ammo:"))
			stuff_int_list(Subsys_status[i].primary_ammo, MAX_SHIP_PRIMARY_BANKS, RAW_INTEGER_TYPE);

		if (optional_string("+Secondary Banks:"))
			stuff_int_list(Subsys_status[i].secondary_banks, MAX_SHIP_SECONDARY_BANKS, WEAPON_LIST_TYPE);

		if (optional_string("+Sbank Ammo:"))
			stuff_int_list(Subsys_status[i].secondary_ammo, MAX_SHIP_SECONDARY_BANKS, RAW_INTEGER_TYPE);
	}
}


/**
 * Checks if any ships of a certain ship class are still available in the team loadout
 * @return The index of the ship in team_data->ship_list if found or -1 if it isn't
 */
int get_reassigned_index(team_data *current_team, int ship_class) 
{
	// Search through the available ships to see if there is a matching ship class in the loadout
	for (int i=0; i < current_team->num_ship_choices; i++)
	{
		if (ship_class == current_team->ship_list[i])
		{
			if (current_team->ship_count[i] > 0) {
				return i;
			}
			else {
				return -1;
			}
		}
	}

	return -1;
}

/**
 * Updates the loadout quanities for a ship class.
 */
void update_loadout_totals(team_data *current_team, int loadout_index)
{
	// Fix the loadout variables to show that the class has less available if there are still ships available
	if (current_team->ship_count[loadout_index] > 0)
	{
		Assert (current_team->loadout_total > 0); 

		current_team->ship_count[loadout_index]--;
		current_team->loadout_total--;
	}
}

/**
 * Attempts to set the class of this ship based which ship classes still remain unassigned in the ship loadout
 * The ship class specified by the mission file itself is tested first. Followed by the list of alt classes. 
 * If an alt class flagged as default_to_this_class is reached the ship will be assigned to that class.
 * If the class can't be assigned because no ships of that class remain the function returns false.  
 */
bool is_ship_assignable(p_object *p_objp)
{
	int loadout_index = -1;

	team_data *data_for_team = &Team_data[p_objp->team];

	// First lets check if the ship specified in the mission file is of an assignable class
	loadout_index = get_reassigned_index(data_for_team, p_objp->ship_class);
	if (loadout_index != -1 )
	{
		Assert (data_for_team->loadout_total > 0);

		update_loadout_totals(data_for_team, loadout_index);
			
		// Since the ship in the mission file matched one available in the loadout we need go no further
		return true;
	}

	// Now we check the alt_classes (if there are any)
	for (SCP_vector<alt_class>::iterator pac = p_objp->alt_classes.begin(); pac != p_objp->alt_classes.end(); ++pac) {
		// we don't check availability unless we are asked to
		if (pac->default_to_this_class == false) {
			loadout_index = pac->ship_class;
			break;
		}
		else {
			loadout_index = get_reassigned_index(data_for_team, pac->ship_class);
			if (loadout_index != -1 ) {
				update_loadout_totals(data_for_team, loadout_index);
				break;
			}
		}
	}

	// If we managed to assign a class we'd may need to actually swap to it
	if (loadout_index != -1 ) {
		if (p_objp->ship_class != data_for_team->ship_list[loadout_index])
		{
			swap_parse_object(p_objp, data_for_team->ship_list[loadout_index]);
		}
		return true;
	}

	return false;
}

/**
 * Checks the list of Parse_objects to see if any of them should be reassigned based on the 
 * number of ships of that class that were present in the loadout.
 */
void process_loadout_objects() 
{	
	SCP_vector<size_t> reassignments;
	
	// Loop through all the Parse_objects looking for ships that should be affected by the loadout code.
	for (size_t i=0; i < Parse_objects.size(); i++)
	{
		p_object *p_objp = &Parse_objects[i];
        if (p_objp->flags[Mission::Parse_Object_Flags::SF_Set_class_dynamically])
		{
			if (!(is_ship_assignable(p_objp)))
			{
				// store the ship so we can come back to it later.
				reassignments.push_back(i);
			}
		}
	}
		
	// Now we go though the ships we were unable to assign earlier and reassign them on a first come first 
	// served basis.
	for (size_t m=0; m < reassignments.size(); m++)
	{
		p_object *p_objp = &Parse_objects[reassignments[m]];
		team_data *current_team = &Team_data[p_objp->team];
		bool loadout_assigned = false;
        Assert(p_objp->flags[Mission::Parse_Object_Flags::SF_Set_class_dynamically]);

		// First thing to check is whether we actually have any ships left to assign
		if (current_team->loadout_total == 0)
		{
			// If there is nothing left to assign we should use the ship in the mission file
			loadout_assigned = true;
		}
		// We do have ships left in the team loadout that we can assign
		else
		{
			// Go through the loadout until we find an unassigned ship
			for (int j=0; j < current_team->num_ship_choices; j++)
			{
				if (current_team->ship_count[j] > 0)
				{
					update_loadout_totals(current_team, j);
					// We will need to assign a new class too (if a p_object the same class was available
					// it should have been assigned by attempt_loadout_assignation_from_defaults()
					Assert (p_objp->ship_class != current_team->ship_list[j]);
					swap_parse_object(p_objp, current_team->ship_list[j]);

					loadout_assigned = true;
					break ;
				}
			}
		}
			
		// We should never reach here with an unassigned loadout
		Assert (loadout_assigned);
	}
}

extern UI_TIMESTAMP Multi_ping_timestamp;
void parse_objects(mission *pm, int flag)
{	
	Assert(pm != NULL);

	required_string("#Objects");	

	// parse in objects
	while (required_string_either("#Wings", "$Name:"))
	{
		p_object pobj;

		// parse a single object
		int valid = parse_object(pm, flag, &pobj);

		// not all objects are always valid or legal
		if (!valid)
			continue;

		// add it
		Parse_objects.push_back(pobj);

		// send out a ping if we are multi so that psnet2 doesn't kill us off for a long load
		// NOTE that we can't use the timestamp*() functions here since they won't increment
		//      during this loading process
		if (Game_mode & GM_MULTIPLAYER)
		{
			if (!Multi_ping_timestamp.isValid() || ui_timestamp_elapsed(Multi_ping_timestamp))
			{
				multi_ping_send_all();
				Multi_ping_timestamp = ui_timestamp(10000); // timeout is 10 seconds between pings
			}
		}
	}

	// Goober5000 - I moved the docking stuff to post_process_ships_wings because of interdependencies
	// between ships and wings.  Neither docking stuff nor ship stuff (for ships present at mission start)
	// will be valid until after post_process_ships_wings is run.

	// Karajorma - Now that we've parsed all the objects we can set the class of those which were flagged 
	// to be set based on the number of ships available in the loadout. 
	if (!Fred_running)
	{
		process_loadout_objects();
	}	
}

/**
 * Replaces a p_object with a new one based on a Ship_info index.
 */
void swap_parse_object(p_object *p_obj, int new_ship_class)
{
	ship_info *new_ship_info = &Ship_info[new_ship_class];
	ship_info *old_ship_info = &Ship_info[p_obj->ship_class];
	int subsys_ind = p_obj->subsys_index;
	subsys_status *ship_subsystems = &Subsys_status[subsys_ind];

	// Class
	// First things first. Change the class of the p_object
	p_obj->ship_class = new_ship_class;

	if (p_obj->wingnum > -1 && p_obj->pos_in_wing == 0) {
		Wings[p_obj->wingnum].special_ship_ship_info_index = new_ship_class;
	}

	// Hitpoints
	// We need to take into account that the ship might have been assigned special hitpoints so we can't 
	// simply swap old for new. 
	Assert (p_obj->ship_max_hull_strength > 0);
	Assert (old_ship_info->max_hull_strength > 0);
	
	float hp_multiplier = p_obj->ship_max_hull_strength / old_ship_info->max_hull_strength;
	p_obj->ship_max_hull_strength = new_ship_info->max_hull_strength * hp_multiplier;

	// Shields
	// Again we have to watch out for special hitpoints but this time we can't assume that there will be a 
	// shield. So first lets see if there is one. 
	if ((p_obj->ship_max_shield_strength != old_ship_info->max_shield_strength) && 
		(p_obj->ship_max_shield_strength > 0) &&
		(new_ship_info->max_shield_strength > 0))
	{
		// This ship is using special hitpoints to alter the shield strength
		float shield_multiplier = p_obj->ship_max_shield_strength / i2fl(old_ship_info->max_shield_strength);
		p_obj->ship_max_shield_strength = new_ship_info->max_shield_strength * shield_multiplier;
	}
	// Not using special hitpoints or a class which has a shield strength of zero
	else
	{
		p_obj->ship_max_shield_strength = new_ship_info->max_shield_strength;
	}
	
	// Primary weapons
	// First find out what is the correct number for a ship of this class
	int num_pbanks = new_ship_info->num_primary_banks;
	// Now cycle through the primary banks looking for banks that were added or removed
	for (int i=0; i < MAX_SHIP_PRIMARY_BANKS; i++)
	{
		// If we're dealing with a primary bank that actually should exist on this ship
		if ( i < num_pbanks )
		{
			// We only care if a weapon hasn't been parsed in for this bank
			if (ship_subsystems->primary_banks[i] == -1)
			{
				// Give the ship the default weapon for this bank. 
				ship_subsystems->primary_banks[i] = new_ship_info->primary_bank_weapons[i];
			}
		}		
		// Any primary banks the ship doesn't have should be set to -1
		else
		{
			ship_subsystems->primary_banks[i] = -1;
		}
	}

	// Secondary weapons 
	// Again we first have to find out how many we should have
	int num_sbanks = new_ship_info->num_secondary_banks;
	// Now cycle through the secondary banks looking for banks that were added or removed
	for (int j=0; j < MAX_SHIP_SECONDARY_BANKS; j++)
	{
		// If we're dealing with a primary bank that actually should exist on this ship
		if ( j < num_sbanks )
		{
			// We only care if a weapon hasn't been parsed in for this bank
			if (ship_subsystems->secondary_banks[j] == -1){
				// Give the ship the default weapon for this bank. 
				ship_subsystems->secondary_banks[j] = new_ship_info->secondary_bank_weapons[j];
			}
		}		
		// Any secondary banks the ship doesn't have should be set to -1
		else
		{
			ship_subsystems->secondary_banks[j] = -1;
		}
	}
}

// Goober5000 - this needs to search the vector instead of using the registry, because it can be used during parsing before the registry is populated
p_object *mission_parse_find_parse_object(const char *name)
{
	SCP_vector<p_object>::iterator ii;

	// look for original ships
	for (ii = Parse_objects.begin(); ii != Parse_objects.end(); ++ii)
		if (!stricmp(ii->name, name))
			return &(*ii);

	// boo
	return NULL;
}

int find_wing_name(char *name)
{
	int	i;

	for (i = 0; i < Num_wings; i++)
	{
		if (!stricmp(name, Wings[i].name))
			return i;
	}

	return -1;
}

/**
* @brief						Tries to create a wing of ships
* @param[inout]	wingp			Pointer to the wing structure of the wing to be created
* @param[in] num_to_create		Number of ships to create
* @param[in] force_create		If true, the wing will be created regardless of whether or not the arrival conditions
*								have been met yet.
* @param[in] force_arrival		If true, the wing will assume its arrival cue is true
* @param[in] specific_instance	Set this to create a specific ship from this wing
* @returns						Number of ships created
*/
int parse_wing_create_ships( wing *wingp, int num_to_create, bool force_create, bool force_arrival, int specific_instance )
{
	int wingnum, objnum, num_create_save;
	int time_to_arrive;
	int pre_create_count;
	int i, j;

	// we need to send this in multiplayer
	pre_create_count = wingp->total_arrived_count;

	// force_create is used to force creation of the wing -- used for multiplayer
	if ( !force_create ) {
		// we only want to evaluate the arrival cue of the wing if:
		// 1) single player
		// 2) multiplayer and I am the host of the game
		// can't create any ships if the arrival cue is false or the timestamp has not elapsed.

		if ( !force_arrival && !eval_sexp(wingp->arrival_cue) )
			return 0;

		// once the sexpressions becomes true, then check the arrival delay on the wing.  The first time, the
		// arrival delay will be <= 0 meaning that no timer object has been set yet.  Set up the timestamp
		// which should always give a number >= 0;
		if ( wingp->arrival_delay <= 0 ) {
			wingp->arrival_delay = timestamp( -wingp->arrival_delay * 1000 );
			Assert ( wingp->arrival_delay >= 0 );
		}

		if ( !timestamp_elapsed( wingp->arrival_delay ) )
			return 0;

		// if wing is coming from docking bay, then be sure that ship we are arriving from actually exists
		// (or will exist).
		if ( wingp->arrival_location == ArrivalLocation::FROM_DOCK_BAY ) {
			Assert( wingp->arrival_anchor >= 0 );
			auto anchor_ship_entry = ship_registry_get(Parse_names[wingp->arrival_anchor]);

			// see if ship is yet to arrive.  If so, then return 0 so we can evaluate again later.
			if (!anchor_ship_entry || anchor_ship_entry->status == ShipStatus::NOT_YET_PRESENT)
				return 0;

			// see if ship is in mission.
			if (!anchor_ship_entry->has_shipp()) {
				int num_remaining;

				// since this wing cannot arrive from this place, we need to mark the wing as destroyed and
				// set the wing variables appropriately.  Good for directives.

				// set the gone flag
                wingp->flags.set(Ship::Wing_Flags::Gone);

				// mark the number of waves and number of ships destroyed equal to the last wave and the number
				// of ships yet to arrive
				num_remaining = ( (wingp->num_waves - wingp->current_wave) * wingp->wave_count);
				wingp->total_arrived_count += num_remaining;
				wingp->current_wave = wingp->num_waves;

				if ( mission_log_get_time(LOG_SHIP_DESTROYED, anchor_ship_entry->name, nullptr, nullptr) || mission_log_get_time(LOG_SELF_DESTRUCTED, anchor_ship_entry->name, nullptr, nullptr) ) {
					wingp->total_destroyed += num_remaining;
				} else if ( mission_log_get_time(LOG_SHIP_DEPARTED, anchor_ship_entry->name, nullptr, nullptr) ) {
					wingp->total_departed += num_remaining;
				} else {
					wingp->total_vanished += num_remaining;
				}

				mission_parse_mark_non_arrival(wingp);	// Goober5000
				WarningEx(LOCATION, "Warning: Wing %s cannot arrive from docking bay of destroyed or departed %s.\n", wingp->name, anchor_ship_entry->name);
				return 0;
			}

			// Goober5000 - check status of fighterbays - if they're destroyed, we can't launch - but we want to reeval later
			if (ship_fighterbays_all_destroyed(anchor_ship_entry->shipp())) {
				WarningEx(LOCATION, "Warning: Wing %s cannot arrive from destroyed docking bay of %s.\n", wingp->name, anchor_ship_entry->name);
				return 0;
			}
		}

		if ( num_to_create == 0 )
			return 0;

		// check the wave_delay_timestamp field.  If it is not valid, make it valid (based on wave delay min
		// and max values).  If it is valid, and not elapsed, then return.  If it is valid and elasped, then
		// continue on.
		if ( !wingp->wave_delay_timestamp.isValid() ) {

			// if at least one of these is valid, then reset the timestamp.  If they are both zero, we will create the
			// wave
			if ( (wingp->wave_delay_min > 0) || (wingp->wave_delay_max > 0) ) {
				Assert ( wingp->wave_delay_min <= wingp->wave_delay_max );
				time_to_arrive = wingp->wave_delay_min + (int)(frand() * (wingp->wave_delay_max - wingp->wave_delay_min));

				// MWA -- 5/18/98
				// HACK HACK -- in the presense of Mike Comet and Mitri, I have introduced one of the most
				// serious breaches of coding standards.  I'm to lazy to fix this the correct way.  Insert
				// a delay before the next wave of the wing can arrive to that clients in the game have ample
				// time to kill off any ships in the wing before the next wave arrives.
				if ( Game_mode & GM_MULTIPLAYER ){
					time_to_arrive += 7;
				}
				wingp->wave_delay_timestamp = _timestamp(time_to_arrive * MILLISECONDS_PER_SECOND);
				return 0;
			}

			// if we get here, both min and max values are 0;  See comments above for a most serious hack
			time_to_arrive = 0;
			if ( Game_mode & GM_MULTIPLAYER )
				time_to_arrive += 7;
			wingp->wave_delay_timestamp = _timestamp(time_to_arrive * MILLISECONDS_PER_SECOND);
		}

		// now check to see if the wave_delay_timestamp is elapsed or not
		if ( !timestamp_elapsed(wingp->wave_delay_timestamp) )
			return 0;
	}

	// finally we can create the wing.

	num_create_save = num_to_create;

	wingnum = WING_INDEX(wingp);					// get the wing number

	// if there are no ships to create, then all ships must be player start ships -- do nothing in this case.
	if ( num_to_create == 0 ){
		return 0;
	}

	wingp->current_wave++;						// we are creating new ships
	// we need to create num_to_create ships.  Since the arrival cues for ships in a wing
	// are ignored, then *all* ships must be in the Ship_arrival_list.

	objnum = -1;

	// Goober5000 - we have to do this via the array because we have no guarantee we'll be able to iterate along the list
	// (since created objects plus anything they're docked to will be removed from it)
	for (SCP_vector<p_object>::iterator ii = Parse_objects.begin(); ii != Parse_objects.end(); ++ii)
	{
		int index;
		ai_info *aip;
		p_object *p_objp = &(*ii);

		// compare the wingnums.  When they are equal, we can create the ship.  In the case of
		// wings that have multiple waves, this code implies that we essentially creating clones
		// of the ships that were created in Fred for the wing when more ships for a new wave
		// arrive.  The threshold value of a wing can also make one of the ships in a wing be "cloned"
		// more often than other ships in the wing.  I don't think this matters much.
		if (p_objp->wingnum != wingnum)
			continue;

		Assert((p_objp->pos_in_wing >= 0) && (p_objp->pos_in_wing < MAX_SHIPS_PER_WING));

		// skip ships destroyed before mission in a similar way that red-alert-deleted ships are skipped
		if (p_objp->destroy_before_mission_time >= 0)
		{
			num_to_create--;
			num_create_save--;
			ship_add_ship_type_count(p_objp->ship_class, -1);
			wingp->red_alert_skipped_ships++;	// even though it isn't red-alert, this is needed for keeping indexes correct

			// skip over this parse object
			if (num_to_create == 0)
				break;
			else
				continue;
		}

		// ensure on arrival list
		if (!parse_object_on_arrival_list(p_objp))
			continue;
	
		// when ingame joining, we need to create a specific ship out of the list of ships for a
		// wing.  specific_instance is a 0 based integer which specified which ship in the wing
		// to create.  So, only create the ship we actually need to.
		if ((Game_mode & GM_MULTIPLAYER) && (specific_instance > 0))
		{
			specific_instance--;
			continue;
		}
		// when not creating a specific ship, we should skip over any ships that weren't carried along in the red-alert
        else if (p_objp->flags[Mission::Parse_Object_Flags::Red_alert_deleted])
		{
			num_to_create--;
			num_create_save--;
			ship_add_ship_type_count(p_objp->ship_class, -1);
			wingp->red_alert_skipped_ships++;

			// clear the flag so that this parse object can be used for the next wave
            p_objp->flags.remove(Mission::Parse_Object_Flags::Red_alert_deleted);

			// skip over this parse object
			if (num_to_create == 0)
				break;
			else
				continue;
		}

        Assert(!(p_objp->flags[Mission::Parse_Object_Flags::SF_Cannot_arrive]));		// get allender

		// if we have the maximum number of ships in the wing, we must bail as well
		if (wingp->current_count >= MAX_SHIPS_PER_WING)
		{
			Int3();					// this is bogus -- we should always allow all ships to be created
			num_to_create = 0;
			break;
		}

		// bash the ship name to be the name of the wing + some number if there is > 1 wave in this wing
		wingp->total_arrived_count++;
		if (wingp->num_waves > 1)
		{
			bool needs_display_name;
			wing_bash_ship_name(p_objp->name, wingp->name, wingp->total_arrived_count + wingp->red_alert_skipped_ships, &needs_display_name);

			// set up display name if we need to
			// (In the unlikely edge case where the ship already has a display name for some reason, it will be overwritten.
			// This is unavoidable, because if we didn't overwrite display names, all waves would have the display name from the first wave.)
			if (needs_display_name)
			{
				p_objp->display_name = p_objp->name;
				end_string_at_first_hash_symbol(p_objp->display_name);
				p_objp->flags.set(Mission::Parse_Object_Flags::SF_Has_display_name);
			}

			// subsequent waves of ships will not be in the ship registry, so add them
			if (!ship_registry_exists(p_objp->name))
			{
				ship_registry_entry entry(p_objp->name);
				entry.status = ShipStatus::NOT_YET_PRESENT;
				entry.pobj_num = POBJ_INDEX(p_objp);

				Ship_registry.push_back(entry);
				Ship_registry_map[p_objp->name] = static_cast<int>(Ship_registry.size() - 1);
			}
		}

		// also, if multiplayer, set the parse object's net signature to be wing's net signature
		// base + total_arrived_count (before adding 1)
		// Cyborg -- The original ships in the wave have their net_signature set at mission parse
		// so only do this if this is a subsequent wave.
		if (Game_mode & GM_MULTIPLAYER && wingp->current_wave > 1)
		{
			// Cyborg -- Also, then we need to subtract the original wave's number of fighters 
			// and also subtract 1 to use the wing's starting signature
			p_objp->net_signature = (ushort) (wingp->net_signature + wingp->total_arrived_count - (wingp->wave_count + 1));
		}


		objnum = parse_create_object(p_objp);
		aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];

		// copy any goals from the wing to the newly created ship
		for (index = 0; index < MAX_AI_GOALS; index++)
		{
			if (wingp->ai_goals[index].ai_mode != AI_GOAL_NONE)
				ai_copy_mission_wing_goal(&wingp->ai_goals[index], aip);
		}

		if (wingp->flags[Ship::Wing_Flags::No_dynamic])
			aip->ai_flags.set(AI::AI_Flags::No_dynamic);

		// update housekeeping variables
		// NOTE:  for the initial wing setup we use actual position to get around
		//        object order isses, but ships in all following waves just get
		//        tacked onto the end of the list
		if (wingp->current_wave == 1) {
			wingp->ship_index[p_objp->pos_in_wing] = Objects[objnum].instance;
		} else {
			wingp->ship_index[wingp->current_count] = Objects[objnum].instance;
		}

		// set up wingman status index
		hud_wingman_status_set_index(wingp, &Ships[Objects[objnum].instance], p_objp);

		p_objp->wing_status_wing_index = Ships[Objects[objnum].instance].wing_status_wing_index;
		p_objp->wing_status_wing_pos = Ships[Objects[objnum].instance].wing_status_wing_pos;

		wingp->current_count++;

		// keep any player ship on the parse object list -- used for respawns
		// 5/8/98 -- MWA -- don't remove ships from the list when you are ingame joining
		if (!(p_objp->flags[Mission::Parse_Object_Flags::OF_Player_start]))
		{
			if ((Game_mode & GM_NORMAL) || !(Net_player->flags & NETINFO_FLAG_INGAME_JOIN))
			{
				// only remove ship if one wave in wing
				if (wingp->num_waves == wingp->current_wave)
				{
					// remove p_objp from the list
					list_remove(&Ship_arrival_list, p_objp);
					
					// free up sexp nodes for reuse
					if (p_objp->ai_goals != -1)
					{
						free_sexp2(p_objp->ai_goals);
						p_objp->ai_goals = -1;
					}
				}
			}
		}

		// keep track of how many ships to create.  Stop when we have done all that we are supposed to do.
		num_to_create--;
		if (num_to_create == 0)
			break;
	}

	// we should always have enough ships in the list!!!
	Assert (num_to_create == 0);

	// wing current_count needs to match the end of the ship_index[] list, but there
	// is a very off chance it could have holes in it (especially if it's a red-alert
	// wing that arrives late), so make sure to compact the list
	int length = MAX_SHIPS_PER_WING;
	for (i = 0; i < length; i++)
	{
		if (wingp->ship_index[i] == -1)
		{
			// shift actual values downward
			for (j = i; j < length - 1; j++)
			{
				wingp->ship_index[j] = wingp->ship_index[j+1];

				// update "special" ship too
				if (wingp->special_ship == j+1)
					wingp->special_ship--;
			}

			// last value becomes -1
			wingp->ship_index[j] = -1;
			length--;

			// stay on the current index in case we still have a -1
			i--;
		}
	}
	
	// possibly play some event driven music here.  Send a network packet indicating the wing was
	// created.  Only do this stuff if actually in the mission.
	if ( (objnum != -1) && (Game_mode & GM_IN_MISSION) ) {		// if true, we have created at least one new ship.
		int it, ship_num;

		// see if this wing is a player starting wing, and if so, call the maybe_add_form_goal
		// function to possibly make the wing form on the player
		for (it = 0; it < MAX_STARTING_WINGS; it++ ) {
			if ( Starting_wings[it] == wingnum ){
				break;
			}
		}
		if ( it < MAX_STARTING_WINGS ){
			ai_maybe_add_form_goal( wingp );
		}

		mission_log_add_entry( LOG_WING_ARRIVED, wingp->name, NULL, wingp->current_wave );
		ship_num = wingp->ship_index[0];

		if ( !(Ships[ship_num].flags[Ship::Ship_Flags::No_arrival_music]) && !(wingp->flags[Ship::Wing_Flags::No_arrival_music]) ) {
			if ( timestamp_elapsed(Allow_arrival_music_timestamp) ) {
				Allow_arrival_music_timestamp = timestamp(ARRIVAL_MUSIC_MIN_SEPARATION);
				event_music_arrival(Ships[ship_num].team);	
			}
		}

		// possibly change the location where these ships arrive based on the wings arrival location
		mission_set_wing_arrival_location( wingp, num_create_save );

		// if in multiplayer (and I am the host) and in the mission, send a wing create command to all
		// other players
		if ( MULTIPLAYER_MASTER ){
			send_wing_create_packet( wingp, num_create_save, pre_create_count );
		}

#ifndef NDEBUG
		// test code to check to be sure that all ships in the wing are ignoring the same types
		// of orders from the leader
		if ( Fred_running ) {
			Assert( wingp->ship_index[wingp->special_ship] != -1 );
			const std::set<size_t>& orders = Ships[wingp->ship_index[0]].orders_accepted;
			for (it = 0; it < wingp->current_count; it++ ) {
				if (it == wingp->special_ship)
					continue;

				if ( orders != Ships[wingp->ship_index[it]].orders_accepted ) {
					Warning(LOCATION, "ships in wing %s are ignoring different player orders.  Please find Mark A\nto talk to him about this.", wingp->name );
					break;
				}
			}
		}
#endif

	}

	wingp->wave_delay_timestamp = TIMESTAMP::invalid();		// we will need to set this up properly for the next wave
	return num_create_save;
}

void parse_wing(mission *pm)
{
	int wingnum, i, wing_goals;
	char name[NAME_LENGTH], ship_names[MAX_SHIPS_PER_WING][NAME_LENGTH];
    char wing_flag_strings[PARSEABLE_WING_FLAGS][NAME_LENGTH];
	wing *wingp;

	Assert(pm != NULL);
	wingp = &Wings[Num_wings];

	required_string("$Name:");
	stuff_string(wingp->name, F_NAME, NAME_LENGTH);

	wingnum = find_wing_name(wingp->name);
	if (wingnum != -1)
		error_display(0, NOX("Redundant wing name: %s\n"), wingp->name);
	wingnum = Num_wings;

	// squad logo - Goober5000
	if (optional_string("+Squad Logo:"))
	{
		int flag = -1;

		stuff_string(wingp->wing_squad_filename, F_NAME, MAX_FILENAME_LEN);

		// load it only if FRED isn't running
		if (!Fred_running)
		{
			// check all previous wings to see if we already loaded it (we want to save memory)
			for (i = 0; i < Num_wings; i++)
			{
				// do we have a previous texture?
				if (Wings[i].wing_insignia_texture != -1)
				{
					// if we have a match
					if (!stricmp(Wings[i].wing_squad_filename, wingp->wing_squad_filename))
					{
						flag = i;
						break;
					}
				}
			}

			// if we have loaded it already, just use the old bitmap index
			if (flag != -1)
			{
				wingp->wing_insignia_texture = Wings[flag].wing_insignia_texture;
			}
			else
			{
				wing_load_squad_bitmap(wingp);
			}
		}
	}

	required_string("$Waves:");
	stuff_int(&wingp->num_waves);
	Assert ( wingp->num_waves >= 1 );		// there must be at least 1 wave

	required_string("$Wave Threshold:");
	stuff_int(&wingp->threshold);

	required_string("$Special Ship:");
	stuff_int(&wingp->special_ship);

	// Use a custom formation if specified
	if (optional_string("+Formation:")) {
		char f[NAME_LENGTH];
		stuff_string(f, F_NAME, NAME_LENGTH);

		wingp->formation = wing_formation_lookup(f);
		if (wingp->formation < 0) {
			Warning(LOCATION, "Invalid Formation %s.", f);
		}
	}
	if (optional_string("+Formation Scale:")) {
		stuff_float(&wingp->formation_scale);
	}

	int temp;
	find_and_stuff("$Arrival Location:", &temp, F_NAME, Arrival_location_names, Num_arrival_names, "Arrival Location");
	if (temp >= 0)
		wingp->arrival_location = static_cast<ArrivalLocation>(temp);

	if ( optional_string("+Arrival Distance:") )
	{
		stuff_int( &wingp->arrival_distance );

		// Goober5000
		if ((wingp->arrival_distance <= 0) && (
			   (wingp->arrival_location == ArrivalLocation::NEAR_SHIP)
			|| (wingp->arrival_location == ArrivalLocation::IN_FRONT_OF_SHIP) || (wingp->arrival_location == ArrivalLocation::IN_BACK_OF_SHIP)
			|| (wingp->arrival_location == ArrivalLocation::ABOVE_SHIP) || (wingp->arrival_location == ArrivalLocation::BELOW_SHIP)
			|| (wingp->arrival_location == ArrivalLocation::TO_LEFT_OF_SHIP) || (wingp->arrival_location == ArrivalLocation::TO_RIGHT_OF_SHIP) ))
		{
			Warning(LOCATION, "Arrival distance for wing %s cannot be %d.  Setting to 1.\n", wingp->name, wingp->arrival_distance);
			wingp->arrival_distance = 1;
		}
	}

	if ( wingp->arrival_location != ArrivalLocation::AT_LOCATION )
	{
		required_string("$Arrival Anchor:");
		stuff_string(name, F_NAME, NAME_LENGTH);
		wingp->arrival_anchor = get_anchor(name);
	}

	wingp->arrival_path_mask = -1;		// -1 only until resolved
	if (optional_string("+Arrival Paths:"))
	{
		// temporarily use mask to point to the restriction index
		wingp->arrival_path_mask = add_path_restriction();
	}

	if (optional_string("+Arrival delay:"))
	{
		int delay;
		stuff_int(&delay);
		if (delay < 0)
		{
			Warning(LOCATION, "Cannot have arrival delay < 0 on wing %s", wingp->name);
			delay = 0;
		}

		if (!Fred_running)
			wingp->arrival_delay = -delay;			// use negative numbers to mean we haven't set up a timer yet
		else
			wingp->arrival_delay = delay;
	}

	required_string("$Arrival Cue:");
	wingp->arrival_cue = get_sexp_main();
	
	find_and_stuff("$Departure Location:", &temp, F_NAME, Departure_location_names, Num_arrival_names, "Departure Location");
	if (temp >= 0)
		wingp->departure_location = static_cast<DepartureLocation>(temp);

	if ( wingp->departure_location != DepartureLocation::AT_LOCATION )
	{
		required_string("$Departure Anchor:");
		stuff_string( name, F_NAME, NAME_LENGTH );
		wingp->departure_anchor = get_anchor(name);
	}

	wingp->departure_path_mask = -1;		// -1 only until resolved
	if (optional_string("+Departure Paths:"))
	{
		// temporarily use mask to point to the restriction index
		wingp->departure_path_mask = add_path_restriction();
	}

	if (optional_string("+Departure delay:"))
	{
		int delay;
		stuff_int(&delay);
		if (delay < 0)
		{
			Warning(LOCATION, "Cannot have departure delay < 0 on wing %s", wingp->name);
			delay = 0;
		}

		if (!Fred_running)
			wingp->departure_delay = -delay;		// use negative numbers to mean that delay timer not yet set
		else
			wingp->departure_delay = delay;
	}

	required_string("$Departure Cue:");
	wingp->departure_cue = get_sexp_main();

	// stores a list of all names of ships in the wing
	required_string("$Ships:");
	wingp->wave_count = (int)stuff_string_list( ship_names, MAX_SHIPS_PER_WING );

	// get the wings goals, if any
	wing_goals = -1;
	if ( optional_string("$AI Goals:") )
		wing_goals = get_sexp_main();

	if (optional_string("+Hotkey:")) {
		stuff_int(&wingp->hotkey);
		Assert((wingp->hotkey >= 0) && (wingp->hotkey < 10));
	}

	if (optional_string("+Flags:")) {
		auto count = (int) stuff_string_list(wing_flag_strings, PARSEABLE_WING_FLAGS);

		for (i = 0; i < count; i++) {
			if (!stricmp(wing_flag_strings[i], NOX("ignore-count")))
				wingp->flags.set(Ship::Wing_Flags::Ignore_count);
			else if (!stricmp(wing_flag_strings[i], NOX("reinforcement")))
				wingp->flags.set(Ship::Wing_Flags::Reinforcement);
			else if (!stricmp(wing_flag_strings[i], NOX("no-arrival-music")))
				wingp->flags.set(Ship::Wing_Flags::No_arrival_music);
			else if (!stricmp(wing_flag_strings[i], NOX("no-arrival-message")))
				wingp->flags.set(Ship::Wing_Flags::No_arrival_message);
			else if (!stricmp(wing_flag_strings[i], NOX("no-first-wave-message")))
				wingp->flags.set(Ship::Wing_Flags::No_first_wave_message);
			else if (!stricmp(wing_flag_strings[i], NOX("no-arrival-warp")))
				wingp->flags.set(Ship::Wing_Flags::No_arrival_warp);
			else if (!stricmp(wing_flag_strings[i], NOX("no-departure-warp")))
				wingp->flags.set(Ship::Wing_Flags::No_departure_warp);
			else if (!stricmp(wing_flag_strings[i], NOX("no-dynamic")))
				wingp->flags.set(Ship::Wing_Flags::No_dynamic);
			else if (!stricmp(wing_flag_strings[i], NOX("nav-carry-status")))
				wingp->flags.set(Ship::Wing_Flags::Nav_carry);
			else if (!stricmp(wing_flag_strings[i], NOX("same-arrival-warp-when-docked")))
				wingp->flags.set(Ship::Wing_Flags::Same_arrival_warp_when_docked);
			else if (!stricmp(wing_flag_strings[i], NOX("same-departure-warp-when-docked")))
				wingp->flags.set(Ship::Wing_Flags::Same_departure_warp_when_docked);
			else
				Warning(LOCATION, "unknown wing flag\n%s\n\nSkipping.", wing_flag_strings[i]);
		}
	}

	// get the wave arrival delay bounds (if present).  Used as lower and upper bounds (in seconds)
	// which determine when new waves of a wing should arrive.
	if ( optional_string("+Wave Delay Min:") )
		stuff_int( &(wingp->wave_delay_min) );
	if ( optional_string("+Wave Delay Max:") )
		stuff_int( &(wingp->wave_delay_max) );

	// 7/13/98 -- MWA
	// error checking against the player ship wings (i.e. starting & tvt) to be sure that wave count doesn't exceed one for
	// these wings.
	if ( MULTI_NOT_TEAM ) {
		for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
			if ( !stricmp(Starting_wing_names[i], wingp->name) ) {
				if ( wingp->num_waves > 1 ) {
					// only end the game if we're the server - clients will eventually find out :)
					if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
						multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_WAVE_COUNT);																
					}
				}
			}
		}
	}
	else if (MULTI_TEAM) {
		for (i = 0; i < MAX_TVT_WINGS; i++ ) {
			if ( !stricmp(TVT_wing_names[i], wingp->name) ) {
				if ( wingp->num_waves > 1 ) {
					// only end the game if we're the server - clients will eventually find out :)
					if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
						multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_WAVE_COUNT);																
					}
				}
			}
		}
	}

	// Get the next starting signature for this in this wing.  We want to reserve wave_count * num_waves
	// of signature.  These can be used to construct wings for ingame joiners.
	if ( Game_mode & GM_MULTIPLAYER ) {
		int next_signature;

		wingp->net_signature = multi_assign_network_signature( MULTI_SIG_SHIP );
		// Cyborg -- Subtract one because the original wave already has its signatures set.
		next_signature = wingp->net_signature + (wingp->wave_count * (wingp->num_waves - 1));
		if ( next_signature > SHIP_SIG_MAX )
			Error(LOCATION, "Too many total ships in mission (%d) for network signature assignment", SHIP_SIG_MAX);
		multi_set_network_signature( (ushort)next_signature, MULTI_SIG_SHIP );
	}

	// set up the ai_goals for this wing -- all ships created from this wing will inherit these goals
	// goals for the wing are stored slightly differently than for ships.  We simply store the index
	// into the sexpression array of each goal (max 10).  When a ship in this wing is created, each
	// goal in the wings goal array is given to the ship.
	if ( wing_goals != -1 ) {
		int sexp;

		// this will assign the goals to the wings as well as to any ships in the wing that have been
		// already created.
		for ( sexp = CDR(wing_goals); sexp != -1; sexp = CDR(sexp) )
			ai_add_wing_goal_sexp(sexp, ai_goal_type::EVENT_WING, wingp);  // used by Fred

		free_sexp2(wing_goals);  // free up sexp nodes for reuse, since they aren't needed anymore.
	}

	// make a temporary map since we want to keep this separate from the ship registry
	SCP_unordered_map<SCP_string, int> parse_object_indexes;
	i = 0;
	for (const auto& pobj : Parse_objects)
		parse_object_indexes.emplace(pobj.name, i++);

	// Goober5000 - to avoid confusing mismatches in the ship registry and hotkey list, and possibly other places,
	// make sure the order of parse objects matches their order in the wing
	auto prev_iter = parse_object_indexes.end();
	for (i = 0; i < wingp->wave_count; i++) {
		auto this_iter = parse_object_indexes.find(ship_names[i]);
		if (this_iter == parse_object_indexes.end()) {
			Error(LOCATION, "Cannot load mission -- for wing %s, ship %s is not present in #Objects section.\n", wingp->name, ship_names[i]);
			break;
		}

		if (i > 0) {
			// compare the parse object indexes of the previous ship and this ship
			if (this_iter->second < prev_iter->second) {
				// swap the parse objects
				std::swap(Parse_objects[this_iter->second], Parse_objects[prev_iter->second]);

				// swap the swapped net signatures so they are in their original order
				std::swap(Parse_objects[this_iter->second].net_signature, Parse_objects[prev_iter->second].net_signature);

				// swap the indexes in our temporary map
				std::swap(this_iter->second, prev_iter->second);

				// start over from the beginning of the wing
				i = -1;
				this_iter = parse_object_indexes.end();
			}
		}

		prev_iter = this_iter;
	}

	// set the wing number for all ships in the wing
	for (i = 0; i < wingp->wave_count; i++ ) {
		char *ship_name = ship_names[i];

		// Goober5000 - since the ship/wing creation stuff is reordered to accommodate multiple docking,
		// everything is still only in the parse array at this point (in both FRED and FS2)

		// find the parse object and assign it the wing number
		auto iter = parse_object_indexes.find(ship_name);
		if (iter != parse_object_indexes.end()) {
			auto p_objp = &Parse_objects[iter->second];

			// get Allender -- ship appears to be in multiple wings
			// (or appears multiple times in the same wing)
			if (p_objp->wingnum >= 0)
				Error(LOCATION, "Cannot load mission -- tried to assign ship %s to wing %s but it was already assigned to wing %s.\n", ship_name, wingp->name, p_objp->wingnum < Num_wings ? Wings[p_objp->wingnum].name : "<out of range wingnum>");

			// assign wingnum
			p_objp->wingnum = wingnum;
			p_objp->pos_in_wing = i;

			// we have found our "special ship" (our wing leader)
			if (!Fred_running && i == 0){
				wingp->special_ship_ship_info_index = p_objp->ship_class;
			}

			// Goober5000 - if this is a player start object, there shouldn't be a wing arrival delay (Mantis #2678)
			if ((p_objp->flags[Mission::Parse_Object_Flags::OF_Player_start]) && (wingp->arrival_delay != 0)) {
				Warning(LOCATION, "Wing %s specifies an arrival delay of %ds, but it also contains a player.  The arrival delay will be reset to 0.", wingp->name, abs(wingp->arrival_delay));
				if (!Fred_running && wingp->arrival_delay > 0) {
					// timestamp has been set, so set it again
					wingp->arrival_delay = timestamp(0);
				} else {
					// no timestamp, or timestamp invalid
					wingp->arrival_delay = 0;
				}
			}
		}
	}

	// Goober5000 - wing creation stuff moved to post_process_ships_wings
}

void parse_wings(mission* pm)
{
	required_string("#Wings");
	while (required_string_either("#Events", "$Name:"))
	{
		Assert(Num_wings < MAX_WINGS);
		parse_wing(pm);
		Num_wings++;
	}
}

// Goober5000
void resolve_path_masks(int anchor, int *path_mask)
{
	path_restriction_t *prp;

	// if we have no restrictions, do a quick out
	if (*path_mask < 0)
	{
		*path_mask = 0;
		return;
	}

	// get path restriction info
	prp = &Path_restrictions[*path_mask];

	// uninitialized; compute the mask from scratch
	if (prp->cached_mask & (1 << MAX_SHIP_BAY_PATHS))
	{
		int j, bay_path, modelnum;
		p_object *parent_pobjp;

		// get anchor ship
		Assert(!(anchor & SPECIAL_ARRIVAL_ANCHOR_FLAG));
		auto parent_ship_entry = ship_registry_get(Parse_names[anchor]);
		parent_pobjp = parent_ship_entry->p_objp();

		// Load the anchor ship model with subsystems and all; it'll need to be done for this mission anyway
		ship_info *sip = &Ship_info[parent_pobjp->ship_class];
		modelnum = model_load(sip->pof_file, sip);

		// resolve names to indexes
		*path_mask = 0;
		for	(j = 0; j < prp->num_paths; j++)
		{
			bay_path = model_find_bay_path(modelnum, prp->path_names[j]);
			if (bay_path < 0)
				continue;

			*path_mask |= (1 << bay_path);
		}

		// cache the result
		prp->cached_mask = *path_mask;
	}
	// already computed; so reuse it
	else
	{
		*path_mask = prp->cached_mask;
	}
}

/**
 * Resolve arrival/departure path masks
 * NB: between parsing and the time this function is run, the path_mask variables store the index of the path info;
 * at all other times, they store the masks of the bay paths as expected
 */
void post_process_path_stuff()
{
	int i;
	wing *wingp;

	// take care of parse objects (ships)
	for (SCP_vector<p_object>::iterator pobjp = Parse_objects.begin(); pobjp != Parse_objects.end(); ++pobjp)
	{
		resolve_path_masks(pobjp->arrival_anchor, &pobjp->arrival_path_mask);
		resolve_path_masks(pobjp->departure_anchor, &pobjp->departure_path_mask);
	}

	// take care of wings
	for (i = 0; i < Num_wings; i++)
	{
		wingp = &Wings[i];

		resolve_path_masks(wingp->arrival_anchor, &wingp->arrival_path_mask);
		resolve_path_masks(wingp->departure_anchor, &wingp->departure_path_mask);
	}
}

// Goober5000
void post_process_ships_wings()
{
	// error checking for custom wings
	if (strcmp(Starting_wing_names[0], TVT_wing_names[0]) != 0)
	{
		Error(LOCATION, "The first starting wing and the first team-versus-team wing must have the same wing name.\n");
	}

	// set up wing indexes
	for (int i = 0; i < MAX_STARTING_WINGS; i++)
		Starting_wings[i] = wing_name_lookup(Starting_wing_names[i], 1);
	for (int i = 0; i < MAX_SQUADRON_WINGS; i++)
		Squadron_wings[i] = wing_name_lookup(Squadron_wing_names[i], 1);
	for (int i = 0; i < MAX_TVT_WINGS; i++)
		TVT_wings[i] = wing_name_lookup(TVT_wing_names[i], 1);

	// when TVT, hack starting wings to be team wings
	if (MULTI_TEAM)
	{
		Assert(MAX_TVT_WINGS <= MAX_STARTING_WINGS);
		for (int i = 0; i < MAX_STARTING_WINGS; i++)
		{
			if (i < MAX_TVT_WINGS)
				Starting_wings[i] = TVT_wings[i];
			else
				Starting_wings[i] = -1;
		}
	}

	// Goober5000 - add all the parse objects to the ship registry.  This must be done before any other parse object processing.
	for (auto &p_obj : Parse_objects)
	{
		ship_registry_entry entry(p_obj.name);
		entry.status = ShipStatus::NOT_YET_PRESENT;
		entry.pobj_num = POBJ_INDEX(&p_obj);

		Ship_registry.push_back(entry);
		Ship_registry_map[p_obj.name] = static_cast<int>(Ship_registry.size() - 1);
	}

	// Goober5000 - resolve the path masks.  Needs to be done early because
	// mission_parse_maybe_create_parse_object relies on it.
	post_process_path_stuff();

	// Goober5000 - now that we've parsed all the objects, resolve the initially docked references.
	// This must be done before anything that relies on the dock references but can't be done until
	// both ships and wings have been parsed.
	mission_parse_set_up_initial_docks();

	// Goober5000 - Now do any post-processing for parse objects.
	for (auto &p_obj : Parse_objects)
	{
		// set a flag if this parse object is in a starting wing
		if (p_obj.wingnum >= 0)
		{
			for (int i = 0; i < MAX_STARTING_WINGS; i++)
				if (p_obj.wingnum == Starting_wings[i])
					p_obj.flags.set(Mission::Parse_Object_Flags::SF_From_player_wing);
		}

		// also load any replacement textures (do this outside the parse loop because we may have ship class replacements too)
		for (SCP_vector<texture_replace>::iterator tr = p_obj.replacement_textures.begin(); tr != p_obj.replacement_textures.end(); ++tr)
		{
			// load the texture
			if (!stricmp(tr->new_texture, "invisible"))
			{
				// invisible is a special case
				tr->new_texture_id = REPLACE_WITH_INVISIBLE;
			}
			else
			{
				// try to load texture or anim as normal
				tr->new_texture_id = bm_load_either(tr->new_texture);
			}

			// not found?
			if (tr->new_texture_id < 0)
				mprintf(("Could not load replacement texture %s for ship %s\n", tr->new_texture, p_obj.name));

			// account for FRED
			if (Fred_running)
			{
				Fred_texture_replacements.push_back(*tr);
				Fred_texture_replacements.back().new_texture_id = -1;
			}
		}

		// Goober5000 - preload stuff for certain object flags
		// (done after parsing object, but before creating it)
		if (p_obj.flags[Mission::Parse_Object_Flags::Knossos_warp_in])
			Knossos_warp_ani_used = true;

		// check the warp parameters too
		if (p_obj.warpin_params_index >= 0 && (Warp_params[p_obj.warpin_params_index].warp_type == WT_KNOSSOS || Warp_params[p_obj.warpin_params_index].warp_type == WT_DEFAULT_THEN_KNOSSOS))
			Knossos_warp_ani_used = true;
		if (p_obj.warpout_params_index >= 0 && (Warp_params[p_obj.warpout_params_index].warp_type == WT_KNOSSOS || Warp_params[p_obj.warpout_params_index].warp_type == WT_DEFAULT_THEN_KNOSSOS))
			Knossos_warp_ani_used = true;
	}

	// Goober5000 - now create all objects that we can.  This must be done before any ship stuff
	// but can't be done until the dock references are resolved.  This was originally done
	// in parse_object().
	for (auto &p_obj : Parse_objects)
	{
		// Evaluate the arrival cue and maybe set up the arrival delay.  This can't be done until the ship registry is populated
		// (because SEXPs now require a complete ship registry) but must be done before the arrival list check inside
		// mission_parse_maybe_create_parse_object.  That check is, in fact, the only reason this is needed.  We don't need to
		// pre-emptively set up the delay for wings because there is no equivalent wing arrival list check.  In any case, the
		// arrival_delay is always validated in mission_did_ship_arrive (for ships) and parse_wing_create_ships (for wings).
		// Addendum: Don't mess with any arrival delays which are strictly positive, meaning they have already been set.
		// (This is the case for ships destroyed before mission.  In the retail codebase, the destroy-before-mission chunk was
		// parsed after the arrival cue and delay were parsed and checked, so it overwrote them.)
		if (!Fred_running && (p_obj.arrival_cue >= 0) && (p_obj.arrival_delay <= 0))
		{
			// eval the arrival cue.  if the cue is true, set up the timestamp for the arrival delay
			if (eval_sexp(p_obj.arrival_cue))
				p_obj.arrival_delay = timestamp(-p_obj.arrival_delay * 1000);
		}

		// create as usual
		mission_parse_maybe_create_parse_object(&p_obj);
	}


	// ----------------- at this point the ships have been created -----------------
	// Now set up the wings.  This must be done after both dock stuff and ship stuff.

	// Goober5000 - for FRED, the ships are initialized after the wings, so we must now tell the wings
	// where their ships are
	if (Fred_running)
	{
		// even though the ships are already created, only the parse objects know the wing info
		for (SCP_vector<p_object>::iterator p_objp = Parse_objects.begin(); p_objp != Parse_objects.end(); ++p_objp)
		{
			// link the ship into the wing's array of ship indices (previously done in parse_wing
			// in a rather less backwards way)
			if (p_objp->wingnum >= 0)
			{
				int shipnum = ship_name_lookup(p_objp->name);

				Assert(shipnum >= 0 && shipnum < MAX_SHIPS);
				Assert(p_objp->pos_in_wing >= 0 && p_objp->pos_in_wing < MAX_SHIPS_PER_WING);

				Wings[p_objp->wingnum].ship_index[p_objp->pos_in_wing] = shipnum;
			}
		}
	}
	// Goober5000 - for FS2, the wings are initialized first, so all we have to do is create their ships
	else
	{
		for (int i = 0; i < Num_wings; i++)
		{
			wing *wingp = &Wings[i];

			// create the wing if is isn't a reinforcement.
			if (!(wingp->flags[Ship::Wing_Flags::Reinforcement]))
				parse_wing_create_ships(wingp, wingp->wave_count);
		}
	}

	
	// ----------------- at this point wings and ships should both be valid -----------------
	// Now do some error checking for multi

	static_assert(MAX_TVT_WINGS_PER_TEAM == 1, "Unless you also update the section of code below or redo the loadout code, for TvT, there should be just one player wing, otherwise, wings may start disappearing in game.");

	// now see if we found the missing wing.  We're looking for a wing that is there after a wing that is not.
	// for now TvT mission do not have enough player wings to be affected by this bug.
	if (The_mission.game_type & (MISSION_TYPE_MULTI | MISSION_TYPE_MULTI_COOP)) {
		bool Squadron_wing_names_found[MAX_SQUADRON_WINGS];

		// quickly look through the squadron wing names to see if we have to warn the modder about this mission
		// to avoid them avoid the multi missing wing bug.
		for (int i = 0; i < MAX_SQUADRON_WINGS; i++) {
			Squadron_wing_names_found[i] = false;

			for (int j = 0; j < Num_wings; j++) {
				if (!strcmp(Wings[j].name, Squadron_wing_names[i])) {
					Squadron_wing_names_found[i] = true;
				}
			}
		}

		bool found = false;
		do {
			found = false;
			// we only search up to the MAX_STARTING_WINGS because non-starting wings should not be in starting wing indices (0-2)
			for (int i = 1; i < MAX_STARTING_WINGS; i++) {
				// If there was a wing for this squadron entry, check the last one. If it's empty, we found a mistake, so move the wing names over.
				if (Squadron_wing_names_found[i] && !Squadron_wing_names_found[i - 1]) {
					Warning(LOCATION, "Squadron wings are not in the correct order and may cause wings to disappear in multi.\n\nEither wing %s should exist or the %s entry needs to come before it in the list.\n\nPlease go back and fix the mission.", Squadron_wing_names[i - 1], Squadron_wing_names[i]);
					char temp_chars[NAME_LENGTH];
					strcpy_s(temp_chars, Squadron_wing_names[i - 1]);
					strcpy_s(Squadron_wing_names[i - 1], Squadron_wing_names[i]);
					strcpy_s(Squadron_wing_names[i], temp_chars);
					Squadron_wing_names_found[i] = !Squadron_wing_names_found[i];
					Squadron_wing_names_found[i - 1] = !Squadron_wing_names_found[i - 1];
					found = true;
				}
			}
		} while (found);
	}
}

// mission events are sexpressions which cause things to happen based on the outcome
// of other events in a mission.  Essentially scripting the different things that can happen
// in a mission

void parse_event(mission *pm)
{
	SCP_UNUSED(pm);

	Mission_events.emplace_back();
	auto event = &Mission_events.back();

	required_string( "$Formula:" );
	event->formula = get_sexp_main();

	if (optional_string("+Name:")){
		stuff_string(event->name, F_NAME);
	}

	if ( optional_string("+Repeat Count:")){
		stuff_int( &(event->repeat_count) );

		// sanity check on the repeat count variable
		// _argv[-1] - negative repeat count is now legal; means repeat indefinitely.
		if ( event->repeat_count == 0 ){
			Warning(LOCATION, "Repeat count for mission event %s is 0.\nMust be >= 1 or negative!  Setting to 1.", event->name.c_str() );
			event->repeat_count = 1;
		}
	}

	if ( optional_string("+Trigger Count:")){
		stuff_int( &(event->trigger_count) );
		event->flags |= MEF_USING_TRIGGER_COUNT; 

		// if we have a trigger count but no repeat count, we want the event to loop until it has triggered enough times
		if (event->repeat_count == 1) {
			event->repeat_count = -1;
		}

		// sanity check on the trigger count variable
		// negative trigger count is also legal
		if ( event->trigger_count == 0 ){
			Warning(LOCATION, "Trigger count for mission event %s is 0.\nMust be >= 1 or negative!  Setting to 1.", event->name.c_str() );
			event->trigger_count = 1;
		}
	}

	if ( optional_string("+Interval:")){
		stuff_int( &(event->interval) );
	}

	if ( optional_string("+Score:") ){
		stuff_int(&event->score);
	}

	if ( optional_string("+Chained:") ){
		stuff_int(&event->chain_delay);
	}

	if ( optional_string("+Objective:") ) {
		stuff_string(event->objective_text, F_NAME);
	}

	if ( optional_string("+Objective key:") ) {
		stuff_string(event->objective_key_text, F_NAME);
	}

	if( optional_string("+Team:") ) {
		stuff_int(&event->team);

		// sanity check
		if (event->team < -1 || event->team >= MAX_TVT_TEAMS) {
			if (Fred_running && !Warned_about_team_out_of_range) {
				Warning(LOCATION, "+Team: value was out of range in the mission file!  This was probably caused by a bug in an older version of FRED.  Using -1 for now.");
				Warned_about_team_out_of_range = true;
			}
			event->team = -1;
		}
	}

	if (optional_string("+Event Flags:")) {
		parse_string_flag_list(event->flags, Mission_event_flags, Num_mission_event_flags);
	}

	if( optional_string("+Event Log Flags:") ) {
		SCP_vector<SCP_string> buffer;
		
		stuff_string_list(buffer); 
		for (int i = 0; i < (int)buffer.size(); i++) {

			for (int j = 0; j < MAX_MISSION_EVENT_LOG_FLAGS; j++) {
				if (!stricmp(buffer[i].c_str(), Mission_event_log_flags[j])) {
					// add the flag to the variable, bitshifted by the index used in Mission_event_log_flags[]
					event->mission_log_flags |= 1 << j; 
					break;
				}
			}
		}
	}

	if (optional_string("$Annotations Start")) {
		// annotations are only used in FRED
		if (Fred_running) {
			while (check_for_string("+Comment:") || check_for_string("+Background Color:") || check_for_string("+Path:")) {
				event_annotation ea;
				ea.path.push_back((int)(event - &Mission_events[0]));

				if (optional_string("+Comment:")) {
					stuff_string(ea.comment, F_MULTITEXT);
					lcl_replace_stuff(ea.comment, true);
				}

				if (optional_string("+Background Color:")) {
					stuff_ubyte(&ea.r);
					check_first_non_grayspace_char(Mp, ',', &Mp);
					stuff_ubyte(&ea.g);
					check_first_non_grayspace_char(Mp, ',', &Mp);
					stuff_ubyte(&ea.b);
				}

				if (optional_string("+Path:")) {
					int num;
					while (true) {
						ignore_gray_space();
						if (stuff_int_optional(&num) != 2) {
							break;
						}
						ea.path.push_back(num);
					}
				}

				Event_annotations.push_back(std::move(ea));
			}
			required_string("$Annotations End");
		} else {
			skip_to_string("$Annotations End");
		}
	}
}

void parse_events(mission *pm)
{
	required_string("#Events");

	while (required_string_either( "#Goals", "$Formula:")) {
		parse_event(pm);
	}
}

void parse_goal(mission *pm)
{
	SCP_UNUSED(pm);
	int dummy;

	Mission_goals.emplace_back();
	auto goalp = &Mission_goals.back();

	find_and_stuff("$Type:", &goalp->type, F_NAME, Goal_type_names, Num_goal_type_names, "goal type");

	required_string("+Name:");
	stuff_string(goalp->name, F_NAME);

	// backwards compatibility for old Fred missions - all new missions should use $MessageNew
	if(optional_string("$Message:")){
		stuff_string(goalp->message, F_NAME);
	} else {
		required_string("$MessageNew:");
		stuff_string(goalp->message, F_MULTITEXT);
	}

	if (optional_string("$Rating:")){
		stuff_int(&dummy);  // not used
	}

	required_string("$Formula:");
	goalp->formula = get_sexp_main();

	if ( optional_string("+Invalid:") )
		goalp->type |= INVALID_GOAL;
	if ( optional_string("+Invalid") )
		goalp->type |= INVALID_GOAL;
	if ( optional_string("+No music") )
		goalp->flags |= MGF_NO_MUSIC;

	if ( optional_string("+Score:") ){
		stuff_int(&goalp->score);
	}

	if ( optional_string("+Team:") ){
		stuff_int( &goalp->team );

		// sanity check
		if (goalp->team < -1 || goalp->team >= (int)Iff_info.size()) {
			if (Fred_running && !Warned_about_team_out_of_range) {
				Warning(LOCATION, "+Team: value was out of range in the mission file!  This was probably caused by a bug in an older version of FRED.  Using -1 for now.");
				Warned_about_team_out_of_range = true;
			}
			goalp->team = -1;
		}
	}
}

void parse_goals(mission *pm)
{
	required_string("#Goals");

	while (required_string_either("#Waypoints", "$Type:")){
		parse_goal(pm);
	}

	if ((pm->game_type & MISSION_TYPE_MULTI) && Mission_goals.size() > UINT8_MAX)
		throw parse::ParseException("Number of goals is too high and breaks multi!");
}

void parse_waypoint_list(mission *pm)
{
	Assert(pm != NULL);

	char name_buf[NAME_LENGTH];
	required_string("$Name:");
	stuff_string(name_buf, F_NAME, NAME_LENGTH);

	SCP_vector<vec3d> vec_list;
	required_string("$List:");
	stuff_vec3d_list(vec_list);

	waypoint_add_list(name_buf, vec_list);
}

void parse_waypoints_and_jumpnodes(mission *pm)
{
	vec3d pos;

	required_string("#Waypoints");

	char file_name[MAX_FILENAME_LEN] = { 0 };
	char jump_name[NAME_LENGTH] = { 0 };
	char jump_display_name[NAME_LENGTH] = {0};

	while (optional_string("$Jump Node:")) {
		stuff_vec3d(&pos);
		CJumpNode jnp(&pos);

		if (optional_string("$Jump Node Name:") || optional_string("+Jump Node Name:")) {
			stuff_string(jump_name, F_NAME, NAME_LENGTH);
			jnp.SetName(jump_name);
		}

		if (optional_string("+Display Name:")) {
			stuff_string(jump_display_name, F_NAME, NAME_LENGTH);
			jnp.SetDisplayName(jump_display_name);
		}

		if(optional_string("+Model File:")){
			stuff_string(file_name, F_NAME, MAX_FILENAME_LEN);
			jnp.SetModel(file_name);
		}

		if(optional_string("+Alphacolor:")) {
			ubyte r,g,b,a;
			stuff_ubyte(&r);
			stuff_ubyte(&g);
			stuff_ubyte(&b);
			stuff_ubyte(&a);
			jnp.SetAlphaColor(r, g, b, a);
		}

		if(optional_string("+Hidden:")) {
			int hide;
			stuff_boolean(&hide);
			jnp.SetVisibility(!hide);
		}

		Jump_nodes.push_back(std::move(jnp));
	}

	while (required_string_either("#Messages", "$Name:"))
		parse_waypoint_list(pm);
}

void parse_messages(mission *pm, int flags)
{
	required_string("#Messages");

	// command stuff by Goober5000 ---------------------------------------
	if (optional_string("$Command Sender:"))
	{
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);

		if (*temp == '#')
			strcpy_s(pm->command_sender, &temp[1]);
		else
			strcpy_s(pm->command_sender, temp);
	}

	if (optional_string("$Command Persona:"))
	{
		int idx;
		char temp[NAME_LENGTH];
		stuff_string(temp, F_NAME, NAME_LENGTH);

		idx = message_persona_name_lookup(temp);
		if (idx >= 0)
			pm->command_persona = idx;
		else
			Warning(LOCATION, "Supplied Command Persona is invalid!  Defaulting to %s.", Personas[Default_command_persona].name);
	}
	// end of command stuff ----------------------------------------------


	mprintf(("Starting mission message count : %d\n", (int)Message_waves.size()));

	while (required_string_either("#Reinforcements", "$Name")) {
		MessageFormat format = (flags & MPF_IMPORT_FSM) ? MessageFormat::FS1_MISSION : MessageFormat::FS2_MISSION;
		message_parse(format);
	}

	mprintf(("Ending mission message count : %d\n", (int)Message_waves.size()));
}

void parse_reinforcement(mission *pm)
{
	reinforcements *ptr;
	p_object *rforce_obj = NULL;
	int instance = -1;

	Assert(Num_reinforcements < MAX_REINFORCEMENTS);
	Assert(pm != NULL);
	ptr = &Reinforcements[Num_reinforcements];

	required_string("$Name:");
	stuff_string(ptr->name, F_NAME, NAME_LENGTH);	

	find_and_stuff("$Type:", &ptr->type, F_NAME, Reinforcement_type_names, Num_reinforcement_type_names, "reinforcement type");

	required_string("$Num times:");
	stuff_int(&ptr->uses);
	ptr->num_uses = 0;

	// reset the flags to 0
	ptr->flags = 0;

	if ( optional_string("+Arrival delay:") )
	{
		int delay;
		stuff_int(&delay);
		if (delay < 0)
		{
			Warning(LOCATION, "Cannot have arrival delay < 0 on reinforcement %s", ptr->name);
			delay = 0;
		}

		ptr->arrival_delay = delay;
	}

	if ( optional_string("+No Messages:") ){
		stuff_string_list( ptr->no_messages, MAX_REINFORCEMENT_MESSAGES );
	}

	if ( optional_string("+Yes Messages:") ){
		stuff_string_list( ptr->yes_messages, MAX_REINFORCEMENT_MESSAGES );
	}	

	// sanity check on the names of reinforcements
	rforce_obj = mission_parse_find_parse_object(ptr->name);

	if (rforce_obj == NULL) {
		if ((instance = wing_name_lookup(ptr->name, 1)) == -1) {
			Warning(LOCATION, "Reinforcement %s not found as ship or wing", ptr->name);
			return;
		}
	} else {
		// Individual ships in wings can't be reinforcements - FUBAR
		if (rforce_obj->wingnum >= 0)
		{
			Warning(LOCATION, "Reinforcement %s is part of a wing - Ignoring reinforcement declaration", ptr->name);
			return;
		}
		else
		{
			instance = rforce_obj->wingnum;
		}
	}

	// now, if the reinforcement is a wing, then set the number of waves of the wing == number of
	// uses of the reinforcement
	if (instance >= 0) {
		Wings[instance].num_waves = ptr->uses;
	}

	Num_reinforcements++;
}

void parse_reinforcements(mission *pm)
{
	required_string("#Reinforcements");

	while (required_string_either("#Background bitmaps", "$Name:"))
		parse_reinforcement(pm);
}

void parse_one_background(background_t *background)
{
	// clear here too because this function can be called from more than one place
	background->flags.reset();
	background->suns.clear();
	background->bitmaps.clear();

	// we might have some flags
	if (optional_string("+Flags:"))
	{
		// we'll assume the list will contain no more than 4 distinct tokens
		char flag_strings[4][NAME_LENGTH];
		int num_strings = (int)stuff_string_list(flag_strings, 4);

		for (auto i = 0; i < num_strings; ++i)
		{
			// if this flag is found, this background was saved with correctly calculated angles, so it should be loaded as such
			if (!stricmp(flag_strings[i], "corrected angles"))
				background->flags.set(Starfield::Background_Flags::Corrected_angles_in_mission_file);
		}
	}

	// parse suns
	while (optional_string("$Sun:"))
	{
		starfield_list_entry sle;

		// filename
		stuff_string(sle.filename, F_NAME, MAX_FILENAME_LEN);

		// angles
		required_string("+Angles:");
		stuff_float(&sle.ang.p);
		stuff_float(&sle.ang.b);
		stuff_float(&sle.ang.h);

		// correct legacy bitmap angles which used incorrect math in older versions
		if (!background->flags[Starfield::Background_Flags::Corrected_angles_in_mission_file])
		{
			stars_correct_background_sun_angles(&sle.ang);
			sle.ang.b = 0.0f;	// stars do not bank
		}

		// scale
		required_string("+Scale:");
		stuff_float(&sle.scale_x);
		sle.scale_y = sle.scale_x;
		sle.div_x = 1;
		sle.div_y = 1;

		// add it
		background->suns.push_back(sle);
	}

	// parse starfields
	while (optional_string("$Starbitmap:"))
	{
		starfield_list_entry sle;

		// filename
		stuff_string(sle.filename, F_NAME, MAX_FILENAME_LEN);

		// angles
		required_string("+Angles:");
		stuff_float(&sle.ang.p);
		stuff_float(&sle.ang.b);
		stuff_float(&sle.ang.h);

		// correct legacy bitmap angles which used incorrect math in older versions
		if (!background->flags[Starfield::Background_Flags::Corrected_angles_in_mission_file])
			stars_correct_background_bitmap_angles(&sle.ang);

		// scale
		if (optional_string("+Scale:"))
		{
			stuff_float(&sle.scale_x);
			sle.scale_y = sle.scale_x;
			sle.div_x = 1;
			sle.div_y = 1;
		}
		else
		{
			required_string("+ScaleX:");
			stuff_float(&sle.scale_x);

			required_string("+ScaleY:");
			stuff_float(&sle.scale_y);

			required_string("+DivX:");
			stuff_int(&sle.div_x);

			required_string("+DivY:");
			stuff_int(&sle.div_y);
		}

		// add it
		background->bitmaps.push_back(sle);
	}

	if (Fred_running)
	{
		// the angles are now stored correctly, so by default we also want to save them correctly
		background->flags.set(Starfield::Background_Flags::Corrected_angles_in_mission_file);
	}
}

void parse_bitmaps(mission *pm)
{
	char str[MAX_FILENAME_LEN];
	int z;

	required_string("#Background bitmaps");

	required_string("$Num stars:");
	stuff_int(&Num_stars);
	if (Num_stars >= MAX_STARS)
		Num_stars = MAX_STARS;

	required_string("$Ambient light level:");
	stuff_int(&pm->ambient_light_level);

	if (pm->ambient_light_level == 0)
	{
		pm->ambient_light_level = DEFAULT_AMBIENT_LIGHT_LEVEL;
	}

	// This should call light_set_ambient() to
	// set the ambient light

	// neb2 info

	// all poofs on by default
	for (size_t i = 0; i < Poof_info.size(); i++)
		set_bit(Neb2_poof_flags.get(), i);
	bool nebula = false;
	if (optional_string("+Neb2:")) {
		nebula = true;
		stuff_string(Neb2_texture_name, F_NAME, MAX_FILENAME_LEN);
	}
	if (optional_string("+Neb2Color:")) {
		nebula = true;
		int neb_colors[3];
		stuff_int_list(neb_colors, 3, RAW_INTEGER_TYPE);
		Neb2_fog_color[0] = (ubyte)neb_colors[0];
		Neb2_fog_color[1] = (ubyte)neb_colors[1];
		Neb2_fog_color[2] = (ubyte)neb_colors[2];
		pm->flags |= Mission::Mission_Flags::Neb2_fog_color_override;
	}
	if (nebula) {
		// Obsolete and only for backwards compatibility
		if (optional_string("+Neb2Flags:")) {
			int temp;
			stuff_int(&temp);
			bit_array_set_from_int(Neb2_poof_flags.get(), Poof_info.size(), temp);
		}

		// Get poofs by name
		if (optional_string("+Neb2 Poofs List:")) {
			SCP_vector<SCP_string> poofs_list;
			stuff_string_list(poofs_list);
			neb2_set_poof_bits(poofs_list);
		}

		// initialize neb effect. its gross to do this here, but Fred is dumb so I have no choice ... :(
		if(Fred_running && (pm->flags[Mission::Mission_Flags::Fullneb])){
			neb2_post_level_init(pm->flags[Mission::Mission_Flags::Neb2_fog_color_override]);
		}
	}

	if(pm->flags[Mission::Mission_Flags::Fullneb]){
		// no regular nebula stuff
		nebula_close();
	} else {
		if (optional_string("+Nebula:")) {
			stuff_string(str, F_NAME, MAX_FILENAME_LEN);
			
			// parse the proper nebula type (full or not)	
			for (z=0; z<NUM_NEBULAS; z++){
				if(pm->flags[Mission::Mission_Flags::Fullneb]){
					if (!stricmp(str, Neb2_filenames[z])) {
						Nebula_index = z;
						break;
					}
				} else {
					if (!stricmp(str, Nebula_filenames[z])) {
						Nebula_index = z;
						break;
					}
				}
			}

			if (z == NUM_NEBULAS)
				WarningEx(LOCATION, "Mission %s\nUnknown nebula %s!", pm->name, str);

			if (optional_string("+Color:")) {
				stuff_string(str, F_NAME, MAX_FILENAME_LEN);
				for (z=0; z<NUM_NEBULA_COLORS; z++){
					if (!stricmp(str, Nebula_colors[z])) {
						Mission_palette = z;
						break;
					}
				}
			}

			if (z == NUM_NEBULA_COLORS)
				WarningEx(LOCATION, "Mission %s\nUnknown nebula color %s!", pm->name, str);

			if (optional_string("+Pitch:")){
				stuff_int(&Nebula_pitch);
			} else {
				Nebula_pitch = 0;
			}

			if (optional_string("+Bank:")){
				stuff_int(&Nebula_bank);
			} else {
				Nebula_bank = 0;
			}

			if (optional_string("+Heading:")){
				stuff_int(&Nebula_heading);
			} else {
				Nebula_heading = 0;
			}						
		}

		nebula_init(Nebula_index, Nebula_pitch, Nebula_bank, Nebula_heading);
	}	

	// Goober5000
	while (optional_string("$Bitmap List:") || check_for_string("$Sun:") || check_for_string("$Starbitmap:"))
	{
		stars_add_blank_background(false);
		parse_one_background(&Backgrounds.back());
	}

	if (optional_string("$Environment Map:")) {
		stuff_string(pm->envmap_name, F_NAME, MAX_FILENAME_LEN);
	}

	// bypass spurious stuff from e.g. FS1 missions
	skip_to_start_of_string("#");
}

void parse_asteroid_fields(mission *pm)
{
	int i;

	Assert(pm != NULL);

	Asteroid_field.num_initial_asteroids = 0;

	i = 0;

	if (!optional_string("#Asteroid Fields"))
		return;

	while (required_string_either("#", "$density:")) {
		float speed, density;
		int type;

		Assert(i < 1);
		required_string("$Density:");
		stuff_float(&density);

		Asteroid_field.num_initial_asteroids = (int) density;

		Asteroid_field.field_type = FT_ACTIVE;
		if (optional_string("+Field Type:")) {
			stuff_int( &type );
			Asteroid_field.field_type = (field_type_t)type;
		}

		Asteroid_field.debris_genre = DG_ASTEROID;
		if (optional_string("+Debris Genre:")) {
			stuff_int( &type );
			Asteroid_field.debris_genre = (debris_genre_t)type;
		}

		Asteroid_field.field_debris_type.clear();
		Asteroid_field.field_asteroid_type.clear();

		// Debris types
		if (Asteroid_field.debris_genre == DG_DEBRIS) {

			// Obsolete and only for backwards compatibility
			for (int j = 0; j < MAX_RETAIL_DEBRIS_TYPES; j++) {
				if (optional_string("+Field Debris Type:")) {
					int subtype;
					stuff_int(&subtype);
					Asteroid_field.field_debris_type.push_back(subtype);
				}
			}

			// Get asteroids by name
			while (optional_string("+Field Debris Type Name:")) {
				SCP_string ast_name;
				stuff_string(ast_name, F_NAME);
				int subtype;
				subtype = get_asteroid_index(ast_name.c_str());
				if (subtype >= 0) {
					Asteroid_field.field_debris_type.push_back(subtype);
				} else {
					WarningEx(LOCATION, "Mission %s\n Invalid asteroid debris %s!", pm->name, ast_name.c_str());
				}
			}

		// Asteroid types
		} else {

			// Retail asteroid subtypes
			SCP_string colors[NUM_ASTEROID_SIZES] = {"Brown", "Blue", "Orange"};

			// Obsolete and only for backwards compatibility
			for (int j = 0; j < NUM_ASTEROID_SIZES; j++) {
				if (optional_string("+Field Debris Type:")) {
					int subtype;
					stuff_int(&subtype);
					Asteroid_field.field_asteroid_type.push_back(colors[subtype]);
				}
			}

			// Get asteroids by name
			while (optional_string("+Field Debris Type Name:")) {
				SCP_string ast_name;
				stuff_string(ast_name, F_NAME);

				// Old saving for asteroids was bugged and saved the asteroid size rather than the subtype color
				// so we'll compensate for that here
				for (size_t k = 0; k < NUM_ASTEROID_SIZES; k++) {
					// if we get the name for small/medium/large asteroid then convert it to retail colors
					if (stricmp(ast_name.c_str(), Asteroid_info[k].name) == 0) {
						ast_name = colors[k];
						break;
					}
				}

				auto list = get_list_valid_asteroid_subtypes();

				//validate the asteroid subtype name
				bool valid = false;
				for (const auto& entry : list) {
					if (ast_name == entry) {
						valid = true;
						break;
					}
				}

				if (valid){
					Asteroid_field.field_asteroid_type.push_back(ast_name);
				} else {
					WarningEx(LOCATION, "Mission %s\n Invalid asteroid %s!", pm->name, ast_name.c_str());
				}
			}
		}

		bool invalid_asteroids = false;
		for (int& ast_type : Asteroid_field.field_debris_type) {
			if (ast_type >= (int)Asteroid_info.size()) {
				invalid_asteroids = true;
				ast_type = -1;
			}
		}

		if (invalid_asteroids)
			Warning(LOCATION, "The Asteroid field contains invalid entries!");

		// backward compatibility
		// Is this a good idea? This doesn't seem like a good idea. What is this compatibility actually for??
		// If you've defined an asteroid field but didn't define any 'roids, then tough luck. Fix your mission.
		// If this is for a retail mission then this needs to be hardcoded for that specific mission file probably. - Mjn
		if ((Asteroid_field.debris_genre == DG_ASTEROID) && (Asteroid_field.field_asteroid_type.empty())) {
			Asteroid_field.field_asteroid_type.push_back("Brown");
		}

		required_string("$Average Speed:");
		stuff_float(&speed);

		vm_vec_rand_vec_quick(&Asteroid_field.vel);
		vm_vec_scale(&Asteroid_field.vel, speed);

		Asteroid_field.speed = speed;

		required_string("$Minimum:");
		stuff_vec3d(&Asteroid_field.min_bound);

		required_string("$Maximum:");
		stuff_vec3d(&Asteroid_field.max_bound);

		vec3d a_rad;
		vm_vec_sub(&a_rad, &Asteroid_field.max_bound, &Asteroid_field.min_bound);
		vm_vec_scale(&a_rad, 0.5f);
		float b_rad = vm_vec_mag(&a_rad);

		Asteroid_field.bound_rad = MAX(3000.0f, b_rad);

		if (optional_string("+Inner Bound:")) {
			Asteroid_field.has_inner_bound = true;

			required_string("$Minimum:");
			stuff_vec3d(&Asteroid_field.inner_min_bound);

			required_string("$Maximum:");
			stuff_vec3d(&Asteroid_field.inner_max_bound);
		} else {
			Asteroid_field.has_inner_bound = false;
		}

		if (optional_string("+Use Enhanced Checks")) {
			Asteroid_field.enhanced_visibility_checks = true;
		}

		if (optional_string("$Asteroid Targets:")) {
			stuff_string_list(Asteroid_field.target_names);
		}
		i++;
	}
}

void parse_variables()
{
	int i, j, num_variables = 0;

	if (! optional_string("#Sexp_variables") ) {
		return;
	} else {
		num_variables = stuff_sexp_variable_list();
	}

	// yeesh - none of this should be done in FRED :)
	// It shouldn't be done for missions in the tecroom either. They should default to whatever FRED set them to
	if ( Fred_running || !(Game_mode & GM_CAMPAIGN_MODE) ) {
		return;
	}

	// Goober5000 - now set the default value, if it's a variable saved on mission progress
	// loop through the current mission's variables
	for (j = 0; j < num_variables; j++) {
		// check against existing variables
		for (auto& current_pv : Campaign.persistent_variables) {
			// if the active mission has a variable with the same name as a variable saved to the campaign file override its initial value with the previous mission's value
			if ( !stricmp(Sexp_variables[j].variable_name, current_pv.variable_name) ) {
				// if this is an eternal that shares the same name as a non-eternal warn but do nothing
				if (Sexp_variables[j].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) {
					error_display(0, "Variable %s is marked eternal but has the same name as another persistent variable. One of these should be renamed to avoid confusion", Sexp_variables[j].text);
				}
				else if (Sexp_variables[j].type  & SEXP_VARIABLE_IS_PERSISTENT) {
					Sexp_variables[j].type = current_pv.type;
					strcpy_s(Sexp_variables[j].text, current_pv.text);
					break;
				} else {
					error_display(0, "Variable %s has the same name as another persistent variable. One of these should be renamed to avoid confusion", Sexp_variables[j].text);
				}
			}
		}
	}

	// next, see if any eternal variables are set loop through the current mission's variables
	for (j = 0; j < num_variables; j++) {
		// check against existing variables
		for (i = 0; i < (int)Player->variables.size(); i++) {
			// if the active mission has a variable with the same name as a variable saved to the player file override its initial value with the previous mission's value
			if ( !stricmp(Sexp_variables[j].variable_name, Player->variables[i].variable_name) ) {
				if (Sexp_variables[j].type & SEXP_VARIABLE_IS_PERSISTENT) {
					// if the variable in the player file is marked as eternal but the version in the mission file is not, we assume that the player file one is rogue
					// and use the one in the mission file instead.
					if ((Player->variables[i].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) && !(Sexp_variables[j].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE)) {
						break;
					}
					// replace the default values with the ones saved to the player file
					Sexp_variables[j].type = Player->variables[i].type;
					strcpy_s(Sexp_variables[j].text, Player->variables[i].text);

					/*
					// check that the eternal flag has been set. Players using a player file from before the eternal flag was added may have old player-persistent variables
					// these should be converted to non-eternals
					if (!(Player->variables[i].type & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE)) {
						Sexp_variables[j].type &= ~SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
					}
					*/

					break;
				} else {
					error_display(0, "Variable %s has the same name as an eternal variable. One of these should be renamed to avoid confusion", Sexp_variables[j].variable_name);
				}
			}
		}
	}
}

void parse_sexp_containers()
{
	if (!optional_string("#Sexp_containers")) {
		return;
	}

	if (optional_string("$Lists")) {
		stuff_sexp_list_containers();
		required_string("$End Lists");
	}

	if (optional_string("$Maps")) {
		stuff_sexp_map_containers();
		required_string("$End Maps");
	}

	// jg18 - persistence-related checking
	// adapted from parse_variables()

	// do this stuff only when playing through a campaign
	if (Fred_running || !(Game_mode & GM_CAMPAIGN_MODE)) {
		return;
	}

	// first update this mission's containers from campaign-persistent containers
	for (const auto &current_pc : Campaign.persistent_containers) {
		auto *p_container = get_sexp_container(current_pc.container_name.c_str());
		if (p_container != nullptr) {
			auto &container = *p_container;

			// if this is an eternal container that shares the same name as a non-eternal, warn but do nothing
			if (container.is_eternal()) {
				error_display(0,
					"SEXP container %s is marked eternal but has the same name as another persistent container. One of "
					"these should be renamed to avoid confusion",
					container.container_name.c_str());
			} else if (container.is_persistent()) {
				if (container.type_matches(current_pc)) {
					// TODO: when network containers are supported, review whether replacement should occur
					// if one container is marked for network use and the other isn't

					// replace!
					container = current_pc;
				} else {
					error_display(0,
						"SEXP container %s is marked persistent but its type (%x) doesn't match a similarly named "
						"persistent container's type (%x). One of "
						"these should be renamed to avoid confusion",
						container.container_name.c_str(),
						(int)container.get_non_persistent_type(),
						(int)current_pc.get_non_persistent_type());
				}
			} else {
				error_display(0,
					"SEXP container %s has the same name as another persistent container. One of these should be "
					"renamed to avoid confusion",
					container.container_name.c_str());
			}
		}
	}

	// then update this mission's containers from player-persistent containers
	for (const auto& player_container : Player->containers) {
		auto *p_container = get_sexp_container(player_container.container_name.c_str());
		if (p_container != nullptr) {
			auto &container = *p_container;

			if (container.is_persistent()) {
				if (player_container.is_eternal() && !container.is_eternal()) {
					// use the mission's non-eternal container over the player-persistent eternal container
					continue;
				} else {
					if (container.type_matches(player_container)) {
						// TODO: when network containers are supported, review whether replacement should occur
						// if one container is marked for network use and the other isn't

						// replace!
						container = player_container;
					} else {
						error_display(0,
							"SEXP container %s is marked persistent but its type (%x) doesn't match a similarly named "
							"eternal container's type (%x). One of "
							"these should be renamed to avoid confusion",
							container.container_name.c_str(),
							(int)container.get_non_persistent_type(),
							(int)player_container.get_non_persistent_type());
					}
				}
			} else {
				error_display(0,
					"SEXP container %s has the same name as an eternal container. One of these should be renamed "
					"to avoid confusion",
					container.container_name.c_str());
			}
		}
	}
}

void parse_custom_data(mission* pm)
{
	if (!optional_string("#Custom Data"))
		return;

	if (optional_string("$begin_data_map")) {

		parse_string_map(pm->custom_data, "$end_data_map", "+Val:");
	}

	if (optional_string("$begin_custom_strings")) {
		while (optional_string("$Name:")) {
			custom_string cs;

			// The name of the string
			stuff_string(cs.name, F_NAME);

			// Arbitrary string value used for grouping strings together
			required_string("+Value:");
			stuff_string(cs.value, F_NAME);

			// The string text itself
			required_string("+String:");
			stuff_string(cs.text, F_MULTITEXT);

			pm->custom_strings.push_back(cs);
		}

		required_string("$end_custom_strings");
	}
}

void apply_default_custom_data(mission* pm)
{
	for (const auto& def : Default_custom_data) {
		size_t count = 0;
		for (const auto& listed : pm->custom_data) {
			if (listed.first != def.key) {
				count++;
			}
		}
		if (count == pm->custom_data.size()) {
			pm->custom_data.emplace(def.key, def.value);
		}
	}
}

bool parse_mission(mission *pm, int flags)
{
	int saved_warning_count = Global_warning_count;
	int saved_error_count = Global_error_count;

	// reset parse error stuff
	Num_unknown_ship_classes = 0;
	Num_unknown_weapon_classes = 0;
	Num_unknown_loadout_classes = 0;

	Warned_about_team_out_of_range = false;

	reset_parse();
	mission_init(pm);

	parse_mission_info(pm);

	Current_file_checksum = netmisc_calc_checksum(pm,MISSION_CHECKSUM_SIZE);

	if (flags & MPF_ONLY_MISSION_INFO)
		return true;

	parse_plot_info(pm);
	parse_variables();
	parse_sexp_containers();
	parse_briefing_info(pm);	// TODO: obsolete code, keeping so we don't obsolete existing mission files
	parse_cutscenes(pm);
	parse_fiction(pm);
	parse_cmd_briefs(pm);
	parse_briefing(pm, flags);
	parse_debriefing_new(pm);
	parse_player_info(pm);
	parse_objects(pm, flags);
	parse_wings(pm);
	parse_events(pm);
	parse_goals(pm);
	parse_waypoints_and_jumpnodes(pm);
	parse_messages(pm, flags);
	parse_reinforcements(pm);
	parse_bitmaps(pm);
	parse_asteroid_fields(pm);
	parse_music(pm, flags);
	parse_custom_data(pm);

	// if we couldn't load some mod data
	if ((Num_unknown_ship_classes > 0) || ( Num_unknown_loadout_classes > 0 )) {
		// if running on standalone server, just print to the log
		if (Game_mode & GM_STANDALONE_SERVER) {
			mprintf(("Warning!  Could not load %d ship classes!\n", Num_unknown_ship_classes));
			return false;
		}
		// don't do this in FRED; we will display a separate popup
		else if (!Fred_running) {
			// build up the prompt...
			SCP_string text;

			if (Num_unknown_ship_classes > 0) {
				sprintf(text, "Warning!\n\nFreeSpace was unable to find %d ship class%s while loading this mission.  This can happen if you try to play a %s that is incompatible with the current mod.\n\n", Num_unknown_ship_classes, (Num_unknown_ship_classes > 1) ? "es" : "", (Game_mode & GM_CAMPAIGN_MODE) ? "campaign" : "mission");
			}
			else {
				sprintf(text, "Warning!\n\nFreeSpace was unable to find %d weapon class%s while loading this mission.  This can happen if you try to play a %s that is incompatible with the current mod.\n\n", Num_unknown_loadout_classes, (Num_unknown_loadout_classes > 1) ? "es" : "", (Game_mode & GM_CAMPAIGN_MODE) ? "campaign" : "mission");
			}

			if (Game_mode & GM_CAMPAIGN_MODE) {
				text += "(The current campaign is \"";
				text += Campaign.name;
			} else {
				text += "(The current mission is \"";
				text += pm->name;
			}

			text += "\", and the current mod is \"";

			if (Cmdline_mod == NULL || *Cmdline_mod == 0) {
				text += "<retail default> ";
			} else {
				for (char *mod_token = Cmdline_mod; *mod_token != '\0'; mod_token += strlen(mod_token) + 1) {
					text += mod_token;
					text += " ";
				}
			}

			text.erase(text.length() - 1);	// trim last space
			text += "\".)\n\n  You can continue to load the mission, but it is quite likely that you will encounter a large number of mysterious errors.  It is recommended that you either select a ";
			text += (Game_mode & GM_CAMPAIGN_MODE) ? "campaign" : "mission";
			text += " that is compatible with your current mod, or else exit FreeSpace and select a different mod.\n\n";

			text += "Do you want to continue to load the mission?";

			// now display the popup
			int popup_rval = popup(PF_TITLE_BIG | PF_TITLE_RED, 2, POPUP_NO, POPUP_YES, text.c_str());
			if (popup_rval == 0) {
				return false;
			}
		}
	}

	if (!post_process_mission(pm))
		return false;

	if ((saved_warning_count - Global_warning_count) > 10 || (saved_error_count - Global_error_count) > 0) {
		char text[512];
		sprintf(text, "Warning!\n\nThe current mission has generated %d warnings and/or errors during load.  These are usually caused by corrupted ship models or syntax errors in the mission file.  While FreeSpace Open will attempt to compensate for these issues, it cannot guarantee a trouble-free gameplay experience.  Source Code Project staff cannot provide assistance or support for these problems, as they are caused by the mission's data files, not FreeSpace Open's source code.", (saved_warning_count - Global_warning_count) + (saved_error_count - Global_error_count));
		popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, text);
	}

	log_printf(LOGFILE_EVENT_LOG, "Mission %s loaded.\n", pm->name); 

	// success
	return true;
}

bool post_process_mission(mission *pm)
{
	int			i;
	int			indices[MAX_SHIPS], objnum;
	ship_weapon	*swp;
	ship_obj *so;

	// Goober5000 - this must be done even before post_process_ships_wings because it is a prerequisite
	ship_clear_ship_type_counts();

	// Goober5000 - must be done before all other post processing
	post_process_ships_wings();

	// the player_start_shipname had better exist at this point!
	auto player_start_entry = ship_registry_get(Player_start_shipname);
	if (!player_start_entry) {
		Warning(LOCATION, "Player start ship '%s' does not exist!", Player_start_shipname);		// maybe the mission was hand-edited :P
		return false;
	}
	Player_start_shipnum = player_start_entry->shipnum;
	Assert( Player_start_shipnum != -1 );
	Player_start_pobject = player_start_entry->p_objp();
	Assert( Player_start_pobject != NULL );

	// Assign objnum, shipnum, etc. to the player structure
	objnum = Ships[Player_start_shipnum].objnum;
	Player_obj = &Objects[objnum];
	if (!Fred_running){
		Player->objnum = objnum;
	}

	Player_obj->flags.set(Object::Object_Flags::Player_ship);			// make this object a player controlled ship.
	Player_ship = &Ships[Player_start_shipnum];
	Player_ai = &Ai_info[Player_ship->ai_index];

	Player_ai->targeted_subsys = NULL;
	Player_ai->targeted_subsys_parent = -1;

	// determine if player start has initial velocity and set forward cruise percent to relect this
	// this should check prev_ramp_vel because that is in local coordinates --wookieejedi
	if ( Player_obj->phys_info.prev_ramp_vel.xyz.z > 0.0f )
		Player->ci.forward_cruise_percent = Player_obj->phys_info.prev_ramp_vel.xyz.z / Player_obj->phys_info.max_vel.xyz.z * 100.0f;

	// Kazan - player use AI at start?
	if (pm->flags[Mission::Mission_Flags::Player_start_ai])
		Player_use_ai = true;

	// Assign squadron information
	if (!Fred_running && (Player != nullptr) && (pm->squad_name[0] != '\0') && (Game_mode & GM_CAMPAIGN_MODE) && !(Game_mode & GM_MULTIPLAYER)) {
		mprintf(("Reassigning player to squadron %s\n", pm->squad_name));
		player_set_squad(Player, pm->squad_name);
		player_set_squad_bitmap(Player, pm->squad_filename, false);
	}

	init_ai_system();

	waypoint_create_game_objects();

	// Goober5000 - this needs to be called only once after parsing of objects and wings is complete
	// (for individual invalidation, see mission_parse_mark_non_arrival)
	mission_parse_mark_non_arrivals();

	// deal with setting up arrival location for all ships.  Must do this now after all ships are created
	mission_parse_set_arrival_locations();

	// clear out information about arriving support ships
	Arriving_support_ship = nullptr;
	Num_arriving_repair_targets = 0;

	// convert all ship name indices to ship indices now that mission has been loaded
	if (Fred_running) {
		i = 0;
		for (const auto &parse_name: Parse_names) {
			auto ship_entry = ship_registry_get(parse_name);
			indices[i] = ship_entry ? ship_entry->shipnum : -1;
			if (indices[i] < 0)
				Warning(LOCATION, "Ship name \"%s\" referenced, but this ship doesn't exist", parse_name.c_str());
			i++;
		}

		for (i=0; i<MAX_SHIPS; i++) {
			if ((Ships[i].objnum >= 0) && (Ships[i].arrival_anchor >= 0) && (Ships[i].arrival_anchor < SPECIAL_ARRIVAL_ANCHOR_FLAG))
				Ships[i].arrival_anchor = indices[Ships[i].arrival_anchor];

			if ( (Ships[i].objnum >= 0) && (Ships[i].departure_anchor >= 0) )
				Ships[i].departure_anchor = indices[Ships[i].departure_anchor];
		}

		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count  && (Wings[i].arrival_anchor >= 0) && (Wings[i].arrival_anchor < SPECIAL_ARRIVAL_ANCHOR_FLAG))
				Wings[i].arrival_anchor = indices[Wings[i].arrival_anchor];

			if (Wings[i].wave_count  && (Wings[i].departure_anchor >= 0) )
				Wings[i].departure_anchor = indices[Wings[i].departure_anchor];
		}

	}

	// before doing anything else, we must validate all of the sexpressions that were loaded into the mission.
	// Loop through the Sexp_nodes array and send the top level functions to the check_sexp_syntax parser
	// Cyborg -- If you are a ingame joiner, your sexps will be taken care of by the server, and checking will 
	// actually crash mission load, since a list of objects present are also given to them only after this point.

	if (!((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN))) {
		for (i = 0; i < Num_sexp_nodes; i++) {
			if (is_sexp_top_level(i) && (!Fred_running || (i != Sexp_clipboard))) {
				int result, bad_node, op;

				op = get_operator_index(i);

				// need to make sure it is an operator before we treat it like one..
				if (op < 0)
				{
					result = SEXP_CHECK_UNKNOWN_OP;
					bad_node = i;
				}
				else
					result = check_sexp_syntax(i, query_operator_return_type(op), 1, &bad_node);

				// entering this if statement will result in program termination!!!!!
				// print out an error based on the return value from check_sexp_syntax()
				// G5K: now entering this statement simply aborts the mission load
				if ( result ) {
					SCP_string location_str;
					SCP_string sexp_str;
					SCP_string error_msg;
					SCP_string bad_node_str;

					// it's helpful to point out the goal/event, so do that if we can
					int index;
					if ((index = mission_event_find_sexp_tree(i)) >= 0) {
						location_str = "Error in mission event: \"";
						location_str += Mission_events[index].name;
						location_str += "\": ";
					} else if ((index = mission_goal_find_sexp_tree(i)) >= 0) {
						location_str = "Error in mission goal: \"";
						location_str += Mission_goals[index].name;
						location_str += "\": ";
					}

					convert_sexp_to_string(sexp_str, i, SEXP_ERROR_CHECK_MODE);
					truncate_message_lines(sexp_str, 30);

					stuff_sexp_text_string(bad_node_str, bad_node, SEXP_ERROR_CHECK_MODE);
					if (!bad_node_str.empty()) {	// the previous function adds a space at the end
						bad_node_str.pop_back();
					}
				
					error_msg = location_str + sexp_error_message(result);
					error_msg += ".\n\nIn sexpression: ";
					error_msg += sexp_str;
					error_msg += "\n\n(Bad node appears to be: ";
					error_msg += bad_node_str;
					error_msg += ")\n";
					Warning(LOCATION, "%s", error_msg.c_str());

					// syntax errors are recoverable in Fred but not FS
					if (!Fred_running && !sexp_recoverable_error(result)) {
						return false;
					}
				}
			}
		}
	}

	// multiplayer missions are handled just before mission start
	if (!(Game_mode & GM_MULTIPLAYER) ){	
		ai_post_process_mission();
	}

	// [ship_clear_ship_type_counts() moved to top of function]

	// we must count all of the ships of particular types.  We count all of the ships that do not have
	// their SF_IGNORE_COUNT flag set.  We don't count ships in wings when the equivalent wing flag is set.
	// in counting ships in wings, we increment the count by the wing's wave count to account for everyone.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		// do not skip over should-be-dead ships (probably destroy-before-mission ships)
		// a) because previous versions didn't, and changing this would change percentages,
		// b) because it does no harm to access should-be-dead ships here

		int shipnum = Objects[so->objnum].instance;
		// pass over non-ship objects and player ship objects
		if ( Ships[shipnum].objnum == -1 || (Objects[Ships[shipnum].objnum].flags[Object::Object_Flags::Player_ship]) )
			continue;
		if ( Ships[shipnum].flags[Ship::Ship_Flags::Ignore_count] )
			continue;
		if ( (Ships[shipnum].wingnum != -1) && (Wings[Ships[shipnum].wingnum].flags[Ship::Wing_Flags::Ignore_count]) )
			continue;

		int num = 1;

		ship_add_ship_type_count( Ships[shipnum].ship_info_index, num );
	}

	// now go through the list of ships yet to arrive
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		int num;

		// go through similar motions as above
		if ( p_objp->flags[Mission::Parse_Object_Flags::SF_Ignore_count] )
			continue;
		if ( (p_objp->wingnum != -1) && (Wings[p_objp->wingnum].flags[Ship::Wing_Flags::Ignore_count]) )
			continue;

		if ( p_objp->wingnum == -1 )
			num = 1;
		else
			num = Wings[p_objp->wingnum].num_waves - 1;			// subtract one since we already counted the first wave
		
		ship_add_ship_type_count( p_objp->ship_class, num );
	}

	// set player weapons that are selected by default
	// AL 09/17/97: I added this code to select the first primary/secondary weapons, 
	// since I noticed the player ship sometimes doesn't get default weapons selected		

	// DB: modified 4/23/98 to take multiplayer into account. Under certain circumstances, multiplayer netplayer ships
	//     had their current_primary_bank and current_secondary_bank set to -1 (from ship_set()) and left there since
	//     Player_ship is not the only one we need to need about.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		if (Objects[so->objnum].flags[Object::Object_Flags::Should_be_dead])
			continue;
		ship *shipp = &Ships[Objects[so->objnum].instance];

		// don't process non player wing ships
		if ( !(shipp->flags[Ship::Ship_Flags::From_player_wing] ) )
			continue;			

		swp = &shipp->weapons;
	
		if ( swp->num_primary_banks > 0 ) {
			swp->current_primary_bank = 0;			// currently selected primary bank
		}

		if ( swp->num_secondary_banks > 0 ) {
			swp->current_secondary_bank = 0;			// currently selected secondary bank
		}
	}

	ets_init_ship(Player_obj);	// init ETS data for the player

	// Goober5000
	stars_load_first_valid_background();

	// put the timestamp stuff here for now
	Mission_end_time = -1;

	Allow_arrival_music_timestamp=timestamp(0);
	Allow_arrival_message_timestamp=timestamp(0);
	Allow_backup_message_timestamp=timestamp(0);
	Arrival_message_delay_timestamp = timestamp(-1);
	Arrival_message_subject = -1;

	int idx;
	for(idx=0; idx<2; idx++){
		Allow_arrival_music_timestamp_m[idx]=timestamp(0);
		Allow_arrival_message_timestamp_m[idx]=timestamp(0);
		Arrival_message_delay_timestamp_m[idx] = timestamp(-1);
	}	

	if(Game_mode & GM_MULTIPLAYER){ 
		multi_respawn_build_points();
	}	

	// maybe reset hotkey defaults when loading new mission
	if ( Last_file_checksum != Current_file_checksum ){
		mission_hotkey_reset_saved();
	}
	Last_file_checksum = Current_file_checksum;

	if (pm->volumetrics)
		pm->volumetrics->renderVolumeBitmap();

	apply_default_custom_data(pm);

	if (Preload_briefing_icon_models) {
		int team = 0;
		if (MULTI_TEAM) {
			team = Net_player->p_info.team;
		}
		const auto &br = Briefings[team].stages;
		for (i = 0; i < Briefings[team].num_stages; i++) {
			const auto &stage = br[i];
			for (int j = 0; j < stage.num_icons; j++) {
				ship_info *sip = &Ship_info[stage.icons[j].ship_class];
				stage.icons[j].modelnum = model_load(sip->pof_file, sip);
			}
		}
	}

	// success
	return true;
}

int get_mission_info(const char *filename, mission *mission_p, bool basic, bool filename_is_full_path)
{
	static SCP_string real_fname_buf;
	const char *real_fname = nullptr;

	if ( !filename || !strlen(filename) ) {
		return -1;
	}

	if (filename_is_full_path) {
		real_fname = filename;
	} else {
		// replace any extension with the standard one
		auto p = strrchr(filename, '.');
		if (p == nullptr) {
			real_fname_buf = filename;
			real_fname_buf += FS_MISSION_FILE_EXT;
			real_fname = real_fname_buf.c_str();
		} else if (stricmp(p, FS_MISSION_FILE_EXT) == 0) {
			real_fname = filename;
		} else {
			real_fname_buf.assign(filename, p - filename);
			real_fname_buf += FS_MISSION_FILE_EXT;
			real_fname = real_fname_buf.c_str();
		}
	}

	// if mission_p is NULL, make it point to The_mission
	if ( mission_p == NULL )
		mission_p = &The_mission;

	CFILE *ftemp = cfopen(real_fname, "rt");
	if (!ftemp) {
		return -1;
	}

	// 7/9/98 -- MWA -- check for 0 length file.
	auto filelength = cfilelength(ftemp);
	cfclose(ftemp);
	if (filelength == 0) {
		return -1;
	}

	try
	{
		read_file_text(real_fname, CF_TYPE_MISSIONS);
		mission_p->Reset();
		reset_parse();
		parse_mission_info(mission_p, basic);
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("MISSIONS: Unable to parse '%s'!  Error message = %s.\n", real_fname, e.what()));
		return -1;
	}
	catch (const parse::VersionException& e)
	{
		mprintf(("MISSIONS: Unable to parse '%s'!  %s", real_fname, e.what()));
		return -1;
	}

	return 0;
}

void mission::Reset()
{
	name[ 0 ] = '\0';
	author = "";
	required_fso_version = LEGACY_MISSION_VERSION;
	created[ 0 ] = '\0';
	modified[ 0 ] = '\0';
	notes[ 0 ] = '\0';
	mission_desc[ 0 ] = '\0';
	game_type = MISSION_TYPE_SINGLE;				// default to single player only
	flags.reset();

	num_players = 1;
	num_respawns = 3;
	max_respawn_delay = -1;
	memset(&Ignored_keys, 0, sizeof(int)*CCFG_MAX);

	memset( &support_ships, 0, sizeof( support_ships ) );
	support_ships.arrival_anchor = -1;
	support_ships.departure_anchor = -1;
	support_ships.max_subsys_repair_val = 100.0f;	//ASSUMPTION: full repair capabilities
	support_ships.max_support_ships = -1;	// infinite
	support_ships.max_concurrent_ships = 1;
	support_ships.ship_class = -1;

	// for each species, store whether support is available
	for (int species = 0; species < (int)Species_info.size(); species++) {
		if (Species_info[species].support_ship_index >= 0) {
			support_ships.support_available_for_species |= (1 << species);
		}
	}

	squad_filename[ 0 ] = '\0';
	squad_name[ 0 ] = '\0';
	for ( int i = 0; i < GR_NUM_RESOLUTIONS; i++ )
		loading_screen[ i ][ 0 ] = '\0';

	skybox_model[ 0 ] = '\0';
	vm_set_identity(&skybox_orientation);
	skybox_flags = DEFAULT_NMODEL_FLAGS;

	envmap_name[ 0 ] = '\0';
	contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	ambient_light_level = DEFAULT_AMBIENT_LIGHT_LEVEL;
	sound_environment.id = -1;

	command_persona = Default_command_persona;
	strcpy_s(command_sender, DEFAULT_COMMAND);
	debriefing_persona = debrief_find_persona_index();
	traitor_override = nullptr;

	event_music_name[ 0 ] = '\0';
	briefing_music_name[ 0 ] = '\0';
	substitute_event_music_name[ 0 ] = '\0';
	substitute_briefing_music_name[ 0 ] = '\0';

	ai_profile = &Ai_profiles[Default_ai_profile];
	lighting_profile_name = lighting_profiles::default_name();

	cutscenes.clear( );

	gravity = vmd_zero_vector;
	HUD_timer_padding = 0;
	volumetrics.reset();

	custom_data.clear();
	custom_strings.clear();
}

/**
 * Initialize the mission and related data structures.
 */
void mission_init(mission *pm)
{
	pm->Reset();

	Player_starts = 0;
	Player_start_shipnum = -1;
	*Player_start_shipname = 0;		// make the string 0 length for checking later
	Entry_delay_time = 0;

	Mission_all_attack = 0;
	Num_teams = 1;				// assume 1

	init_sexp();
	mission_goals_and_events_init();

	fiction_viewer_reset();
	cmd_brief_reset();
	brief_reset();
	debrief_reset();
	messages_init();

	mission_parse_reset_alt();
	mission_parse_reset_callsign();
	ai_lua_reset_general_orders();

	Parse_names.clear();
	Num_path_restrictions = 0;
	Num_ai_dock_names = 0;
	ai_clear_goal_target_names();

	for (int i = 0; i < MAX_CARGO; i++)
		Cargo_names[i] = Cargo_names_buf[i]; // make a pointer array for compatibility
	strcpy(Cargo_names[0], "Nothing");
	Num_cargo = 1;

	jumpnode_level_close();
	waypoint_level_close();

	red_alert_invalidate_timestamp();
	event_music_reset_choices();
	clear_texture_replacements();

	// initialize the initially_docked array.
	for (int i = 0; i < MAX_SHIPS; i++) {
		Initially_docked[i].docker[0] = '\0';
		Initially_docked[i].dockee[0] = '\0';
		Initially_docked[i].docker_point[0] = '\0';
		Initially_docked[i].dockee_point[0] = '\0';
	}
	Total_initially_docked = 0;

	Parse_objects.clear();
	list_init(&Ship_arrival_list);	// init list for arrival ships

	Subsys_index = 0;
	Subsys_status_size = 0;

	if (Subsys_status != nullptr) {
		vm_free( Subsys_status );
		Subsys_status = nullptr;
	}

	Num_wings = 0;
	for (int i = 0; i < MAX_WINGS; i++)
		Wings[i].clear();
	
	Num_reinforcements = 0;

	Asteroid_field.num_initial_asteroids = 0;

	// This could be set with a sexp, so we should reset it here
	Motion_debris_override = false;

	// reset background bitmaps and suns
	stars_pre_level_init();
	neb2_pre_level_init();

	ENVMAP = -1;

	// FS1 nebula
	nebula_close();
	Nebula_pitch = (int)((float)(Random::next() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_bank = (int)((float)(Random::next() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_heading = (int)((float)(Random::next() & 0x0fff) * 360.0f / 4096.0f);
	Nebula_index = -1;
	Mission_palette = 1;
}

// Main parse routine for parsing a mission.  The default parameter flags tells us which information
// to get when parsing the mission.  0 means get everything (default).  Other flags just gets us basic
// info such as game type, number of players etc. or whether we are importing from a different format.
bool parse_main(const char *mission_name, int flags)
{
	int i;
	bool rval;

	Assert(Ship_info.size() <= MAX_SHIP_CLASSES);

	// fill in Ship_class_names array with the names from the ship_info struct
	i = 0;
	for (auto it = Ship_info.begin(); it != Ship_info.end(); i++, ++it)
		Ship_class_names[i] = it->name;
	
	do {
		// don't do this for imports
		if (!(flags & MPF_IMPORT_FSM)) {
			CFILE *ftemp = cfopen(mission_name, "rt", CF_TYPE_MISSIONS);

			// fail situation.
			if (!ftemp) {
				if (!Fred_running)
					Error( LOCATION, "Couldn't open mission '%s'\n", mission_name );

				Current_file_length = -1;
				Current_file_checksum = 0;
	
				rval = false;
				break;
			}

			Current_file_length = cfilelength(ftemp);
			cfclose(ftemp);
		}

		try
		{
			// import FS1 mission
			if (flags & MPF_IMPORT_FSM) {
				read_file_text(mission_name, CF_TYPE_ANY);
				convertFSMtoFS2();
				rval = parse_mission(&The_mission, flags);
			}
			// regular mission load
			else {
				read_file_text(mission_name, CF_TYPE_MISSIONS);
				rval = parse_mission(&The_mission, flags);
			}

			display_parse_diagnostics();
		}
		catch (const parse::ParseException& e)
		{
			mprintf(("MISSIONS: Unable to parse '%s'!  Error message = %s.\n", mission_name, e.what()));
			rval = false;
			break;
		}
		catch (const parse::VersionException& e)
		{
			mprintf(("MISSIONS: Unable to parse '%s'!  %s", mission_name, e.what()));
			rval = false;
			break;
		}
	} while (0);

	if (!Fred_running)
		strcpy_s(Mission_filename, mission_name);

	return rval;
}

// Note, this is currently only called from game_shutdown()
void mission_parse_close()
{
	// free subsystems
	if (Subsys_status != NULL)
	{
		vm_free(Subsys_status);
		Subsys_status = NULL;
	}

	// the destructor for each p_object will clear its dock list
	Parse_objects.clear();
}

/**
 * Sets the arrival location of the ships in wingp.  
 *
 * @param wingp Pointer to wing
 * @param num_to_set The threshold value for wings may have us create more ships in the wing when there are still some remaining
 */
void mission_set_wing_arrival_location( wing *wingp, int num_to_set )
{
	int index;
	int anchor_objnum = -1;

	// get the starting index into the ship_index array of the first ship whose location we need set.

	index = wingp->current_count - num_to_set;
	if ( (wingp->arrival_location == ArrivalLocation::FROM_DOCK_BAY) || (wingp->arrival_location == ArrivalLocation::AT_LOCATION) ) {
		while ( index < wingp->current_count ) {
			object *objp;

			objp = &Objects[Ships[wingp->ship_index[index]].objnum];
			anchor_objnum = mission_set_arrival_location(wingp->arrival_anchor, wingp->arrival_location, wingp->arrival_distance, OBJ_INDEX(objp), wingp->arrival_path_mask, NULL, NULL);

			index++;
		}
	} else {
		object *leader_objp;
		vec3d pos;
		matrix orient;
		int wing_index;

		// wing is not arriving from a docking bay -- possibly move them based on arriving near
		// or in front of some other ship.
		index = wingp->current_count - num_to_set;
		leader_objp = &Objects[Ships[wingp->ship_index[index]].objnum];
		anchor_objnum = mission_set_arrival_location(wingp->arrival_anchor, wingp->arrival_location, wingp->arrival_distance, OBJ_INDEX(leader_objp), wingp->arrival_path_mask, &pos, &orient);
		if (anchor_objnum != -1) {
			// modify the remaining ships created
			index++;
			wing_index = 1;
			while ( index < wingp->current_count ) {
				object *objp;

				objp = &Objects[Ships[wingp->ship_index[index]].objnum];

				// change the position of the next ships in the wing.  Use the cool function in AiCode.cpp which
				// Mike K wrote to give new positions to the wing members.
				get_absolute_wing_pos( &objp->pos, leader_objp, WING_INDEX(wingp), wing_index++, false);
				memcpy( &objp->orient, &orient, sizeof(matrix) );

				index++;
			}
		}
	}

	if (Game_mode & GM_IN_MISSION) {
		for ( index = wingp->current_count - num_to_set; index < wingp->current_count; index ++ ) {
			object *objp = &Objects[Ships[wingp->ship_index[index]].objnum];
			object *anchor_objp = (anchor_objnum >= 0) ? &Objects[anchor_objnum] : nullptr;

			if (scripting::hooks::OnShipArrive->isActive()) {
				scripting::hooks::OnShipArrive->run(scripting::hooks::ShipArriveConditions{ &Ships[wingp->ship_index[index]], wingp->arrival_location, anchor_objp },
					scripting::hook_param_list(
						scripting::hook_param("Ship", 'o', objp),
						scripting::hook_param("Parent", 'o', anchor_objp, anchor_objp != nullptr)
					));
			}

			if (wingp->arrival_location != ArrivalLocation::FROM_DOCK_BAY) {
				shipfx_warpin_start(objp);
			}
		}
	}
}

/**
 * Called after a mission is parsed to set the arrival locations of all ships in the
 * mission to the apprioriate spot.  Mainly needed because ships might be in dock bays to start
 * the mission, so their AI mode must be set appropriately.
 */
void mission_parse_set_arrival_locations()
{
	int i;

	if ( Fred_running )
		return;

	obj_merge_created_list();
	for (auto so: list_range(&Ship_obj_list)) {
		// do not skip over should-be-dead ships (probably destroy-before-mission ships)
		// a) because previous versions didn't, and changing this could affect the location of should-be-dead ships
		// b) because it does no harm to access should-be-dead ships here

		auto shipp = &Ships[Objects[so->objnum].instance];
		// if the ship is in a wing -- ignore the info and let the wing info handle it
		if ( shipp->wingnum != -1 )
			continue;

		// call function to set arrival location for this ship.
		mission_set_arrival_location( shipp->arrival_anchor, shipp->arrival_location, shipp->arrival_distance, so->objnum, shipp->arrival_path_mask, NULL, NULL);
	}

	// do the wings
	for ( i = 0; i < Num_wings; i++ ) {

		// if wing has no ships, then don't process it.
		if ( Wings[i].current_count == 0 )
			continue;

		mission_set_wing_arrival_location( &Wings[i], Wings[i].current_count );
	}
}

// Goober5000
bool sexp_is_locked_false(int node)
{
	// dunno why these are different, but they are
	if (Fred_running)
		return (node == Locked_sexp_false);
	else
		return (Sexp_nodes[node].value == SEXP_KNOWN_FALSE);
}

void set_cue_to_false(int *cue)
{
	free_sexp2(*cue);
	*cue = Locked_sexp_false;
}

// function to set the arrival cue of a ship to false
void reset_arrival_to_false(p_object *pobjp, bool reset_wing)
{
	// falsify the ship cue
	mprintf(("Setting arrival cue of ship %s to false for initial docking purposes.\n", pobjp->name));
	set_cue_to_false(&pobjp->arrival_cue);

	// falsify the wing cue and all ships in that wing
	if (reset_wing && pobjp->wingnum >= 0)
	{
		wing *wingp = &Wings[pobjp->wingnum];
		mprintf(("Setting arrival cue of wing %s to false for initial docking purposes.\n", wingp->name));
		set_cue_to_false(&wingp->arrival_cue);

		for (SCP_vector<p_object>::iterator ii = Parse_objects.begin(); ii != Parse_objects.end(); ++ii)
		{
			if ((&(*ii) != pobjp) && (ii->wingnum == pobjp->wingnum))
				reset_arrival_to_false(&(*ii), false);
		}
	}
}

/**
 * In both retail and SCP, the dock "leader" is defined as the only guy in his
 * group with a non-false arrival cue.
 *
 * If we are forcing a leader, that means *none* of the ships in this dock group
 * have a non-false arrival cue, so we just need to pick one
 */
void parse_object_mark_dock_leader_sub(p_object *pobjp, p_dock_function_info *infop, bool force_a_leader)
{
	int cue_to_check;

	// if this guy is part of a wing, he uses his wing's arrival cue
	if (pobjp->wingnum >= 0)
	{
		cue_to_check = Wings[pobjp->wingnum].arrival_cue;
	}
	// check the object's arrival cue
	else
	{
		cue_to_check = pobjp->arrival_cue;
	}

	// is he a leader (using the definition above)?
	if (!sexp_is_locked_false(cue_to_check) || force_a_leader)
	{
		p_object *existing_leader;

		// increment number of leaders found
		infop->maintained_variables.int_value++;

		// see if we already found a leader
		existing_leader = infop->maintained_variables.objp_value;
		if (existing_leader != NULL)
		{
			// keep existing leader if he has a higher priority than us
			if (ship_class_compare(pobjp->ship_class, existing_leader->ship_class) >= 0)
			{
				// set my arrival cue to false
				reset_arrival_to_false(pobjp, true);
				return;
			}

			// otherwise, unmark the existing leader and set his arrival cue to false
            existing_leader->flags.remove(Mission::Parse_Object_Flags::SF_Dock_leader);
			reset_arrival_to_false(existing_leader, true);
		}

		// mark and save me as the leader
        pobjp->flags.set(Mission::Parse_Object_Flags::SF_Dock_leader);
		infop->maintained_variables.objp_value = pobjp;
	}
}

void parse_object_mark_dock_leader_helper(p_object *pobjp, p_dock_function_info *infop)
{
	parse_object_mark_dock_leader_sub(pobjp, infop, false);
}

void parse_object_choose_arbitrary_dock_leader_helper(p_object *pobjp, p_dock_function_info *infop)
{
	parse_object_mark_dock_leader_sub(pobjp, infop, true);
}

// Goober5000
void parse_object_set_handled_flag_helper(p_object *pobjp, p_dock_function_info * /*infop*/)
{
    pobjp->flags.set(Mission::Parse_Object_Flags::Already_handled);
}

// Goober5000
void parse_object_clear_handled_flag_helper(p_object *pobjp, p_dock_function_info * /*infop*/)
{
    pobjp->flags.remove(Mission::Parse_Object_Flags::Already_handled);
}

// Goober5000
void parse_object_clear_all_handled_flags()
{
	// clear flag for all ships
	for (SCP_vector<p_object>::iterator ii = Parse_objects.begin(); ii != Parse_objects.end(); ++ii)
	{
		p_object *pobjp = &(*ii);
		p_dock_function_info dfi;

		// since we're going through all objects, this object may not be docked
		if (!object_is_docked(pobjp))
			continue;

		// has this object (by extension, this group of docked objects) been cleared already?
		if (!(pobjp->flags[Mission::Parse_Object_Flags::Already_handled]))
			continue;

		// clear the handled flag for this group
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_object_clear_handled_flag_helper);
	}
}

// Goober5000
// This function iterates through the Initially_docked array and builds the dock trees
// for each parse object.  This can only be done after all objects and wings have been parsed.
void mission_parse_set_up_initial_docks()
{
	int i;

	// build trees
	for (i = 0; i < Total_initially_docked; i++)
	{
		// resolve the docker and dockee
		auto docker_entry = ship_registry_get(Initially_docked[i].docker);
		auto docker = docker_entry ? docker_entry->p_objp_or_null() : nullptr;
		if (docker == nullptr)
		{
			Warning(LOCATION, "Could not resolve initially docked object '%s'!", Initially_docked[i].docker);
			continue;
		}
		auto dockee_entry = ship_registry_get(Initially_docked[i].dockee);
		auto dockee = dockee_entry ? dockee_entry->p_objp_or_null() : nullptr;
		if (dockee == nullptr)
		{
			Warning(LOCATION, "Could not resolve docking target '%s' of initially docked object '%s'!", Initially_docked[i].dockee, Initially_docked[i].docker);
			continue;
		}

		// skip docking if they're already docked
		// (in FSO, we list all initially docked pairs for all ships,
		// so we end up with twice as many docking entries as we need)
		if (dock_check_find_direct_docked_object(docker, dockee))
			continue;

		// resolve the dockpoints
		auto docker_point = Initially_docked[i].docker_point;
		auto dockee_point = Initially_docked[i].dockee_point;

		// docker point in use?
		if (dock_find_object_at_dockpoint(docker, docker_point) != NULL)
		{
			Warning(LOCATION, "Trying to initially dock '%s' and '%s', but the former's dockpoint is already in use!", Initially_docked[i].docker, Initially_docked[i].dockee);
			continue;
		}

		// dockee point in use?
		if (dock_find_object_at_dockpoint(dockee, dockee_point) != NULL)
		{
			Warning(LOCATION, "Trying to initially dock '%s' and '%s', but the latter's dockpoint is already in use!", Initially_docked[i].docker, Initially_docked[i].dockee);
			continue;
		}

		dock_dock_objects(docker, docker_point, dockee, dockee_point);
	}

	// now resolve the leader of each tree
	for (SCP_vector<p_object>::iterator ii = Parse_objects.begin(); ii != Parse_objects.end(); ++ii)
	{
		p_object *pobjp = &(*ii);
		p_dock_function_info dfi;

		// since we're going through all objects, this object may not be docked
		if (!object_is_docked(pobjp))
			continue;

		// has this object (by extension, this group of docked objects) been handled already?
		if (pobjp->flags[Mission::Parse_Object_Flags::Already_handled])
			continue;

		// find the dock leader(s)
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_object_mark_dock_leader_helper);

		// display an error if necessary
		if (dfi.maintained_variables.int_value == 0)
		{
			Warning(LOCATION, "In the docking group containing %s, every ship has an arrival cue set to false.  The group will not appear in-mission!\n", pobjp->name);

			// for FRED, we must arbitrarily choose a dock leader, otherwise the entire docked group will not be loaded
			if (Fred_running)
				dock_evaluate_all_docked_objects(pobjp, &dfi, parse_object_choose_arbitrary_dock_leader_helper);
		}
		else if (dfi.maintained_variables.int_value > 1)
		{
			Warning(LOCATION, "In the docking group containing %s, there is more than one ship with a non-false arrival cue!  There can only be one such ship.  Setting all arrival cues except %s to false...\n", dfi.maintained_variables.objp_value->name, dfi.maintained_variables.objp_value->name);
		}

		// clear dfi stuff
		dfi.maintained_variables.int_value = 0;
		dfi.maintained_variables.objp_value = NULL;

		// set the handled flag for this group
		dock_evaluate_all_docked_objects(pobjp, &dfi, parse_object_set_handled_flag_helper);
	}

	// now clear the handled flags
	parse_object_clear_all_handled_flags();
}

/**
 * Returns true or false if the given mission support multiplayers
 */
int mission_parse_is_multi(const char *filename, char *mission_name)
{
	int game_type;
	int filelength;
	CFILE *ftemp;

	// new way of getting information.  Open the file, and just get the name and the game_type flags.
	// return the flags if a multiplayer mission

	ftemp = cfopen(filename, "rt");
	if (!ftemp)
		return 0;

	// 7/9/98 -- MWA -- check for 0 length file.
	filelength = cfilelength(ftemp);
	cfclose(ftemp);
	if ( filelength == 0 )
		return 0;

	game_type = 0;
	do {
		try
		{
			read_file_text(filename, CF_TYPE_MISSIONS);
			reset_parse();

			if (skip_to_string("$Name:") != 1) {
				nprintf(("Network", "Unable to process %s because we couldn't find $Name:", filename));
				break;
			}
			stuff_string(mission_name, F_NAME, NAME_LENGTH);

			if (skip_to_string("+Game Type Flags:") != 1) {
				nprintf(("Network", "Unable to process %s because we couldn't find +Game Type Flags:\n", filename));
				break;
			}
			stuff_int(&game_type);
		}
		catch (const parse::ParseException& e)
		{
			mprintf(("MISSIONS: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
			break;
		}
		catch (const parse::VersionException& e)
		{
			mprintf(("MISSIONS: Unable to parse '%s'!  %s", filename, e.what()));
			break;
		}
	} while (0);

	return (game_type & MISSION_TYPE_MULTI) ? game_type : 0;
}

/**
 * Called to retrieve useful information about a mission.  
 *
 * We will get the name, description, and number of players for a mission. Probably used for multiplayer only?
 * The calling function can use the information in The_mission to get the name/description of the mission
 * if needed.
 */
int mission_parse_get_multi_mission_info( const char *filename )
{
	if ( get_mission_info(filename, &The_mission) )
		return -1;

	Assert( The_mission.game_type & MISSION_TYPE_MULTI );		// assume multiplayer only for now?

	// return the number of parse_players.  later, we might want to include (optionally?) the number
	// of other ships in the main players wing (usually wing 'alpha') for inclusion of number of
	// players allowed.

	return The_mission.num_players;
}

/**
 * @brief				Returns the parse object on the ship arrival list associated with the given name.
 * @param[in] name		The name of the object
 * @returns				The parse object, or NULL if no object with the given name is on the arrival list
 * @remarks				This function is used to determine whether a ship has arrived. Ships on the arrival list
 *						are considered to not be in the game; In order to make respawns work in multiplayer,
 *						player ships (those marked with the P_OF_PLAYER_START flag) are never removed from it.
 */
p_object *mission_parse_get_arrival_ship(const char *name)
{
	p_object *p_objp;

	if (name == nullptr)
		return nullptr;

	for (p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		if (!stricmp(p_objp->name, name)) 
		{
			return p_objp;	// still on the arrival list
		}
	}

	return nullptr;
}

/**
 * @brief					Returns the parse object on the ship arrival list associated with the given net signature.
 * @param[in] net_signature	The net signature of the object
 * @returns					The parse object, or NULL if no object with the given signature is on the arrival list
 * @remarks					This function is used to determine whether a ship has arrived. Ships on the arrival list
 *							are considered to not be in the game; In order to make respawns work in multiplayer,
 *							player ships (those marked with the P_OF_PLAYER_START flag) are never removed from it.
 */
p_object *mission_parse_get_arrival_ship(ushort net_signature)
{
	p_object *p_objp;

	for (p_objp = GET_FIRST(&Ship_arrival_list); p_objp !=END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		if (p_objp->net_signature == net_signature) 
		{
			return p_objp;	// still on the arrival list
		}
	}

	return NULL;
}

/**
 * Because player ships remain on the arrival list (see parse_wing_create_ships), testing for yet-to-arrive by merely
 * checking the list will produce false positives for player ships.  So this function also checks whether the object was created.
 */
bool mission_check_ship_yet_to_arrive(const char *name)
{
	p_object *p_objp = mission_parse_get_arrival_ship(name);
	if (p_objp == nullptr)
		return false;

	if (p_objp->created_object != nullptr)
		return false;

	return true;
}

/**
 * Sets the arrival location of a parse object according to the arrival location of the object.
 * @return objnum of anchor ship if there is one, -1 otherwise.
 */
int mission_set_arrival_location(int anchor, ArrivalLocation location, int dist, int objnum, int path_mask, vec3d *new_pos, matrix *new_orient)
{
	int shipnum, anchor_objnum;
	vec3d anchor_pos, rand_vec, new_fvec;
	matrix orient;

	if ( location == ArrivalLocation::AT_LOCATION )
		return -1;

	Assert(anchor >= 0);

	// this ship might possibly arrive at another location.  The location is based on the
	// proximity of some ship (and some other special tokens)
	if (anchor & SPECIAL_ARRIVAL_ANCHOR_FLAG)
	{
		bool get_players = (anchor & SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG) > 0;

		// filter out iff
		int iff_index = anchor;
		iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_FLAG;
		iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG;

		// get ship
		shipnum = ship_get_random_team_ship(iff_get_mask(iff_index), get_players ? SHIP_GET_ONLY_PLAYERS : SHIP_GET_ANY_SHIP);
	}
	// if we didn't find the arrival anchor in the list of special nodes, then do a
	// ship name lookup on the anchor
	else
	{
		auto anchor_entry = ship_registry_get(Parse_names[anchor]);
		shipnum = anchor_entry ? anchor_entry->shipnum : -1;
	}

	// if we didn't get an object from one of the above functions, then make the object
	// arrive at its placed location
	if (shipnum < 0)
	{
		Assert ( location != ArrivalLocation::FROM_DOCK_BAY );		// bogus data somewhere!!!  get mwa
		nprintf (("allender", "couldn't find ship for arrival anchor -- using location ship created at"));
		return -1;
	}

	// take the shipnum and get the position.  once we have positions, we can determine where
	// to make this ship appear
	Assert ( shipnum != -1 );
	anchor_objnum = Ships[shipnum].objnum;
	anchor_pos = Objects[anchor_objnum].pos;

	// if arriving from docking bay, then set ai mode and call function as per AL's instructions.
	if ( location == ArrivalLocation::FROM_DOCK_BAY ) {
		// if we get an error, just let the ship arrive(?)
		if ( ai_acquire_emerge_path(&Objects[objnum], anchor_objnum, path_mask) == -1 ) {
			// get MWA or AL -- not sure what to do here when we cannot acquire a path
			mprintf(("Unable to acquire arrival path on anchor ship %s\n", Ships[shipnum].ship_name));
			return -1;
		}
	} else {

		// AL: ensure dist > 0 (otherwise get errors in vecmat)
		// TODO: maybe set distance to 2x ship radius of ship appearing in front of?
		if ( dist <= 0 )
		{
			// Goober5000 - default to 100
			Warning(LOCATION, "Distance of %d is invalid in mission_set_arrival_location.  Defaulting to 100.\n", dist);
			dist = 100;
		}
		
		// get a vector which is the ships arrival position based on the type of arrival
		// this ship should have.  Arriving near a ship we use a random normalized vector
		// scaled by the distance given by the designer.  Arriving in front of a ship means
		// entering the battle in the view cone.
		if ( location == ArrivalLocation::NEAR_SHIP ) {
			// get a random vector -- use static randvec if in multiplayer
			if ( Game_mode & GM_NORMAL )
				vm_vec_rand_vec_quick(&rand_vec);
			else
				static_randvec( Objects[objnum].net_signature, &rand_vec );
		} else if ( location == ArrivalLocation::IN_FRONT_OF_SHIP || location == ArrivalLocation::IN_BACK_OF_SHIP
			     || location == ArrivalLocation::ABOVE_SHIP || location == ArrivalLocation::BELOW_SHIP
			     || location == ArrivalLocation::TO_LEFT_OF_SHIP || location == ArrivalLocation::TO_RIGHT_OF_SHIP ) {
			vec3d t1, t2, t3;
			int r1, r2;
			float x;

			// cool function by MK to give a reasonable random vector "in front" of a ship
			// rvec and uvec are the right and up vectors.
			// If these are not available, this would be an expensive method.
			x = cosf(fl_radians(45.0f));
			if ( Game_mode & GM_NORMAL ) {
				r1 = Random::flip_coin() ? -1 : 1;
				r2 = Random::flip_coin() ? -1 : 1;
			} else {
				// in multiplayer, use the static rand functions so that all clients can get the
				// same information.
				r1 = static_rand(Objects[objnum].net_signature) < STATIC_RAND_MAX / 2 ? -1 : 1;
				r2 = static_rand(Objects[objnum].net_signature+1) < STATIC_RAND_MAX / 2 ? -1 : 1;
			}

			vm_vec_copy_scale(&t1, &(Objects[anchor_objnum].orient.vec.fvec), x);
			vm_vec_copy_scale(&t2, &(Objects[anchor_objnum].orient.vec.rvec), (1.0f - x) * r1);
			vm_vec_copy_scale(&t3, &(Objects[anchor_objnum].orient.vec.uvec), (1.0f - x) * r2);

			vm_vec_add(&rand_vec, &t1, &t2);
			vm_vec_add2(&rand_vec, &t3);
			vm_vec_normalize(&rand_vec);

			// vertical axis: rotate to up (around X axis)
			if (location == ArrivalLocation::ABOVE_SHIP || location == ArrivalLocation::BELOW_SHIP)
				vm_rot_point_around_line(&rand_vec, &rand_vec, PI_2, &vmd_zero_vector, &vmd_x_vector);
			// lateral axis: rotate to right (backwards around Y axis)
			else if (location == ArrivalLocation::TO_LEFT_OF_SHIP || location == ArrivalLocation::TO_RIGHT_OF_SHIP)
				vm_rot_point_around_line(&rand_vec, &rand_vec, -PI_2, &vmd_zero_vector, &vmd_y_vector);

			// for the opposite directions, just flip the vector around
			if (location == ArrivalLocation::IN_BACK_OF_SHIP || location == ArrivalLocation::BELOW_SHIP || location == ArrivalLocation::TO_LEFT_OF_SHIP)
				vm_vec_negate(&rand_vec);
		} else {
			UNREACHABLE("Unknown location type discovered when trying to parse %s -- Please let an SCP coder know!", Ships[shipnum].ship_name);
		}

		// add in the radius of the two ships involved.  This will make the ship arrive further than
		// specified, but will appear more accurate since we are pushing the edge of the model to the
		// specified distance.  large objects appears to be a lot closer without the following line because
		// the object centers were at the correct distance, but the model itself was much closer to the
		// target ship.
		dist += (int)Objects[objnum].radius + (int)Objects[anchor_objnum].radius;
		vm_vec_scale_add(&Objects[objnum].pos, &anchor_pos, &rand_vec, (float)dist);

		// I think that we will always want to orient the ship that is arriving to face towards
		// the ship it is arriving near/in front of.  The effect will be cool!
		//
		// calculate the new fvec of the ship arriving and use only that to get the matrix.  isn't a big
		// deal not getting bank.
		vm_vec_normalized_dir(&new_fvec, &anchor_pos, &Objects[objnum].pos );
		vm_vector_2_matrix_norm( &orient, &new_fvec, nullptr, nullptr );
		Objects[objnum].orient = orient;
	}

	// set the new_pos parameter since it might be used outside the function (i.e. when dealing with wings).
	if ( new_pos )
		memcpy(new_pos, &Objects[objnum].pos, sizeof(vec3d) );

	if ( new_orient )
		memcpy( new_orient, &Objects[objnum].orient, sizeof(matrix) );

	return anchor_objnum;
}

/**
 * Mark a reinforcement as available
 */
void mission_parse_mark_reinforcement_available(char *name)
{
	int i;
	reinforcements *rp;

	for (i = 0; i < Num_reinforcements; i++) {
		rp = &Reinforcements[i];
		if ( !stricmp(rp->name, name) ) {
			if ( !(rp->flags & RF_IS_AVAILABLE) ) {
				rp->flags |= RF_IS_AVAILABLE;

				// tell all of the clients.
				if ( MULTIPLAYER_MASTER ) {
					send_reinforcement_avail( i );
				}
			}
			return;
		}
	}

	Assert ( i < Num_reinforcements );
}

/**
 * Takes a parse object and checks the arrival cue, delay and destruction of object it is arriving from then creates the object if necessary.  
 *
 * @return -1 if not created.  
 * @return objnum of created ship otherwise
 */
int mission_did_ship_arrive(p_object *objp, bool force_arrival)
{
	// find out if the arrival cue became true
	bool should_arrive = force_arrival || eval_sexp(objp->arrival_cue);

	// we must first check to see if this ship is a reinforcement or not.  If so, then don't
	// process
	if ( objp->flags[Mission::Parse_Object_Flags::SF_Reinforcement] ) {

		// if this ship did arrive, mark the reinforcement as available, and tell clients if in multiplayer
		// mode
		if ( should_arrive ) {
			mission_parse_mark_reinforcement_available(objp->name);
		}

		// if we're forcing the arrival, then "use" the reinforcement; otherwise don't process anything else
		if (force_arrival) {
			for (int i = 0; i < Num_reinforcements; i++) {
				auto rp = &Reinforcements[i];
				if (!stricmp(rp->name, objp->name)) {
					rp->num_uses++;
					break;
				}
			}
		} else {
			return -1;
		}
	}

	if ( should_arrive ) { 		// has the arrival criteria been met?
		// check to see if the delay field <= 0.  if so, then create a timestamp and then maybe
		// create the object
		if ( objp->arrival_delay <= 0 ) {
			objp->arrival_delay = timestamp( -objp->arrival_delay * 1000 );

			// make sure we have a valid timestamp
			Assert( objp->arrival_delay > 0 );
		}
		
		// if the timestamp hasn't elapsed, move onto the next ship.
		if ( !timestamp_elapsed(objp->arrival_delay) )
			return -1;

		// check to see if this ship is to arrive via a docking bay.  If so, and the ship to arrive from
		// doesn't exist, don't create.
		if ( objp->arrival_location == ArrivalLocation::FROM_DOCK_BAY ) {
			Assert( objp->arrival_anchor >= 0 );
			auto anchor_ship_entry = ship_registry_get(Parse_names[objp->arrival_anchor]);

			// see if ship is yet to arrive.  If so, then return -1 so we can evaluate again later.
			if (!anchor_ship_entry || anchor_ship_entry->status == ShipStatus::NOT_YET_PRESENT)
				return -1;

			// see if ship is in mission.
			if (!anchor_ship_entry->has_shipp()) {
				mission_parse_mark_non_arrival(objp);	// Goober5000
				WarningEx(LOCATION, "Warning: Ship %s cannot arrive from docking bay of destroyed or departed %s.\n", objp->name, anchor_ship_entry->name);
				return -1;
			}

			// Goober5000 - check status of fighterbays - if they're destroyed, we can't launch - but we want to reeval later
			if (ship_fighterbays_all_destroyed(anchor_ship_entry->shipp())) {
				WarningEx(LOCATION, "Warning: Ship %s cannot arrive from destroyed docking bay of %s.\n", objp->name, anchor_ship_entry->name);
				return -1;
			}
		}

		if ( objp->flags[Mission::Parse_Object_Flags::SF_Cannot_arrive] ) {
			WarningEx(LOCATION, "Warning: Ship %s cannot arrive. Ship not created.\n", objp->name);
			return -1;
		}

		// create the ship
		int object_num = parse_create_object(objp);
		Assert(object_num >= 0 && object_num < MAX_OBJECTS);
		
		// Play the music track for an arrival
		if ( !(Ships[Objects[object_num].instance].flags[Ship::Ship_Flags::No_arrival_music]) )
			if ( timestamp_elapsed(Allow_arrival_music_timestamp) ) {
				Allow_arrival_music_timestamp = timestamp(ARRIVAL_MUSIC_MIN_SEPARATION);
				event_music_arrival(Ships[Objects[object_num].instance].team);
			}
		return object_num;
	}

	return -1;
}

// Goober5000
bool mission_maybe_make_ship_arrive(p_object *p_objp, bool force_arrival)
{
	if (p_objp->wingnum >= 0)
	{
		Warning(LOCATION, "Parse objects (%s) belonging to wings must arrive through the wing code!", p_objp->name);
		return false;
	}

	if (p_objp->created_object != nullptr)
	{
		Warning(LOCATION, "Cannot create a parse object (%s) more than once!", p_objp->name);
		return false;
	}

	// try to create ship
	int objnum = mission_did_ship_arrive(p_objp, force_arrival);
	if (objnum < 0)
		return false;

	if (p_objp == Arriving_support_ship)
		mission_parse_support_arrived(objnum);		// support ships have some unique housekeeping and are never on the arrival list
	else if (parse_object_on_arrival_list(p_objp))
		list_remove(&Ship_arrival_list, p_objp);	// remove from arrival list

	return true;
}

// Goober5000
void mission_parse_mark_non_arrival(p_object *p_objp)
{
	// mark the flag
    p_objp->flags.set(Mission::Parse_Object_Flags::SF_Cannot_arrive);
}

// Goober5000
void mission_parse_mark_non_arrival(wing *wingp)
{
	int wingnum = WING_INDEX(wingp);

	// look through all ships yet to arrive...
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		// ...and mark the ones in this wing
        if (p_objp->wingnum == wingnum)
            p_objp->flags.set(Mission::Parse_Object_Flags::SF_Cannot_arrive);
	}	
}

/**
 * Set a flag on all parse objects on ship arrival list which cannot arrive in the mission
 */
void mission_parse_mark_non_arrivals()
{
	for (p_object *p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp))
	{
		if (p_objp->wingnum != -1)
		{
			if (!object_is_docked(p_objp) && (Sexp_nodes[Wings[p_objp->wingnum].arrival_cue].value == SEXP_KNOWN_FALSE))
				p_objp->flags.set(Mission::Parse_Object_Flags::SF_Cannot_arrive);
		}
		else
		{
			if (Sexp_nodes[p_objp->arrival_cue].value == SEXP_KNOWN_FALSE)
				p_objp->flags.set(Mission::Parse_Object_Flags::SF_Cannot_arrive);
		}
	}
}

/**
 * Deal with support ship arrival. This function can get called from either single or multiplayer.  Needed to that clients
 * can know when to abort rearm.
 *
 * @param objnum is the object number of the arriving support ship
 */
void mission_parse_support_arrived( int objnum )
{
	int i;

	// when the support ship arrives, the shipname it is supposed to repair is in the 'misc'
	// field of the parse object.  If the ship still exists, call ai function which actually
	// issues the goal for the repair
	for ( i = 0; i < Num_arriving_repair_targets; i++ ) {
		int shipnum;

		shipnum = ship_name_lookup( Arriving_repair_targets[i] );

		if ( shipnum != -1 ) {
			object *requester_objp, *support_objp;

			support_objp = &Objects[objnum];
			requester_objp = &Objects[Ships[shipnum].objnum];
			ai_add_rearm_goal( requester_objp, support_objp );
		}
	}

	Ships[Objects[objnum].instance].flags.set(Ship::Ship_Flags::Warped_support);

	Arriving_support_ship = NULL;
	Num_arriving_repair_targets = 0;
}

// Goober5000
int parse_object_on_arrival_list(p_object *pobjp)
{
	return (pobjp->next != NULL) && (pobjp->prev != NULL);
}

/**
 * Check the lists of arriving ships and wings, creating new ships/wings if the arrival criteria have been met
 */
void mission_eval_arrivals() {
	int rship = -1;

	// before checking arrivals, check to see if we should play a message concerning arrivals
	// of other wings.  We use the timestamps to delay the arrival message slightly for
	// better effect
	if (timestamp_valid(Arrival_message_delay_timestamp) && timestamp_elapsed(Arrival_message_delay_timestamp) && !MULTI_TEAM) {
		bool use_terran_cmd = !The_mission.flags[Mission::Mission_Flags::No_builtin_command] && (Command_announces_enemy_arrival_chance >= 0) && (frand() < Command_announces_enemy_arrival_chance);

		rship = ship_get_random_player_wing_ship(SHIP_GET_UNSILENCED);
		ship* subject = (Arrival_message_subject < 0) ? nullptr : &Ships[Arrival_message_subject];
		if ((rship < 0) || use_terran_cmd) {
			message_send_builtin(MESSAGE_ARRIVE_ENEMY, nullptr, subject, -1, -1);
		} else if (rship >= 0) {
			message_send_builtin(MESSAGE_ARRIVE_ENEMY, &Ships[rship], subject, -1, -1);
		}

		Arrival_message_delay_timestamp = timestamp(-1);		// make the stamp invalid
		Arrival_message_subject = -1;
	}

	// check the arrival list
	// Goober5000 - we can't run through the list the usual way because we might
	// remove a bunch of objects and completely screw up the list linkage
	for (SCP_vector<p_object>::iterator ii = Parse_objects.begin(); ii != Parse_objects.end(); ++ii) {
		p_object* pobjp = &(*ii);

		// make sure we're on the arrival list
		if (!parse_object_on_arrival_list(pobjp)) {
			continue;
		}

		// if this object has a wing, don't create it -- let code for wings determine if it should be created
		if (pobjp->wingnum >= 0) {
			continue;
		}

		// make it arrive
		mission_maybe_make_ship_arrive(pobjp);
	}

	// check the support ship arrival list
	if (Arriving_support_ship) {
		// make it arrive (support ships are not put on the arrival list)
		mission_maybe_make_ship_arrive(Arriving_support_ship);
	}

	// we must also check to see if there are waves of a wing that must
	// reappear if all the ships of the current wing have been destroyed or
	// have departed. If this is the case, then create the next wave.
	for (int i = 0; i < Num_wings; i++) {
		// make it arrive
		mission_maybe_make_wing_arrive(i);
	}
}

bool mission_maybe_make_wing_arrive(int wingnum, bool force_arrival)
{
	int rship = -1;
	auto wingp = &Wings[wingnum];

	// should we process this wing anymore
	if (wingp->flags[Ship::Wing_Flags::Gone])
		return false;

	// if we have a reinforcement wing, then don't try to create new ships automatically.
	if (wingp->flags[Ship::Wing_Flags::Reinforcement])
	{
		// check to see in the wings arrival cue is true, and if so, then mark the reinforcement
		// as available
		if (force_arrival || eval_sexp(wingp->arrival_cue))
			mission_parse_mark_reinforcement_available(wingp->name);

		// if we're forcing the arrival, then "use" the reinforcement; otherwise don't process anything else
		if (force_arrival && wingp->current_count == 0) {
			for (int i = 0; i < Num_reinforcements; i++) {
				auto rp = &Reinforcements[i];
				if (!stricmp(rp->name, wingp->name)) {
					rp->num_uses++;
					break;
				}
			}
		} else {
			// reinforcement wings skip the rest of the function
			return false;
		}
	}
		
	// don't do evaluations for departing wings
	if (wingp->flags[Ship::Wing_Flags::Departing])
		return false;

	// must check to see if we are at the last wave.  Code above to determine when a wing is gone only
	// gets run when a ship is destroyed (not every N seconds like it used to).  Do a quick check here.
	if (wingp->current_wave == wingp->num_waves)
		return false;

	// If the current wave of this wing is 0, then we haven't created the ships in the wing yet.
	// If the threshold of the wing has been reached, then we need to create more ships.
	bool is_first_wave = wingp->current_wave == 0;
	if (is_first_wave || (wingp->current_count <= wingp->threshold))
	{
		// Call parse_wing_create_ships to try and create it.  That function will eval the arrival
		// cue of the wing and create the ships if necessary.
		int created = parse_wing_create_ships(wingp, wingp->wave_count, false, force_arrival);

		// if we didn't create any ships, nothing more to do for this wing
		if (created <= 0)
			return false;

		// If this wing was a reinforcement wing, then we need to reset the reinforcement flag for the wing
		// so the user can call in another set if need be.
		if (wingp->flags[Ship::Wing_Flags::Reset_reinforcement])
		{
            wingp->flags.remove(Ship::Wing_Flags::Reset_reinforcement);
            wingp->flags.set(Ship::Wing_Flags::Reinforcement);
		}

		// probably send a message to the player when this wing arrives.
		// if no message, nothing more to do for this wing
		if (wingp->flags[Ship::Wing_Flags::No_arrival_message])
			return true;
		if (wingp->flags[Ship::Wing_Flags::No_first_wave_message] && is_first_wave)
			return true;

		// multiplayer team vs. team
		if(MULTI_TEAM)
		{
			// send a hostile wing arrived message
			rship = wingp->ship_index[wingp->special_ship];

			int multi_team_filter = Ships[rship].team;

			// there are two timestamps at work here.  One to control how often the player receives
			// messages about incoming hostile waves, and the other to control how long after
			// the wing arrives does the player actually get the message.
			if (timestamp_elapsed(Allow_arrival_message_timestamp_m[multi_team_filter]))
			{
				if (!timestamp_valid(Arrival_message_delay_timestamp_m[multi_team_filter]))
				{
					Arrival_message_delay_timestamp_m[multi_team_filter] = timestamp_rand(ARRIVAL_MESSAGE_DELAY_MIN, ARRIVAL_MESSAGE_DELAY_MAX);
				}
				Allow_arrival_message_timestamp_m[multi_team_filter] = timestamp(Builtin_messages[MESSAGE_ARRIVE_ENEMY].min_delay);
						
				// send to the proper team
				message_send_builtin(MESSAGE_ARRIVE_ENEMY, nullptr, nullptr, -1, multi_team_filter);
			}
		}
		// does the player attack this ship?
		else if (iff_x_attacks_y(Player_ship->team, Ships[wingp->ship_index[0]].team))
		{
			// there are two timestamps at work here.  One to control how often the player receives
			// messages about incoming hostile waves, and the other to control how long after
			// the wing arrives does the player actually get the message.
			if (timestamp_elapsed(Allow_arrival_message_timestamp))
			{
				if (!timestamp_valid(Arrival_message_delay_timestamp))
				{
					Arrival_message_delay_timestamp = timestamp_rand(ARRIVAL_MESSAGE_DELAY_MIN, ARRIVAL_MESSAGE_DELAY_MAX);
					Arrival_message_subject = wingp->ship_index[0];
				}
				Allow_arrival_message_timestamp = timestamp(Builtin_messages[MESSAGE_ARRIVE_ENEMY].min_delay);
			}
		}
		// everything else
		else if (timestamp_elapsed(Allow_backup_message_timestamp)) {
			rship = ship_get_random_ship_in_wing(wingnum, SHIP_GET_UNSILENCED);
			if (rship >= 0) {
				auto sent = message_send_builtin(MESSAGE_BACKUP, &Ships[rship], nullptr, -1, -1);
				if (sent) {
					Allow_backup_message_timestamp = timestamp(Builtin_messages[MESSAGE_BACKUP].min_delay);
				}
			}
		}

		return true;
	}

	return false;
}


/**
 * Called to make object objp depart.  Rewritten and expanded by Goober5000.
 */
int mission_do_departure(object *objp, bool goal_is_to_warp)
{
	Assert(objp->type == OBJ_SHIP);
	bool beginning_departure;
	DepartureLocation location;
	int anchor, path_mask;
	ship *shipp = &Ships[objp->instance];
	ai_info *aip = &Ai_info[shipp->ai_index];

	// this function is often called many times in a row for ships that are in the first stage of departing,
	// but some things should only be done when departure first starts
	if (aip->mode == AIM_WARP_OUT || shipp->is_departing())
	{
		beginning_departure = false;
	}
	else
	{
		beginning_departure = true;
		mprintf(("Entered mission_do_departure() for %s\n", shipp->ship_name));

	if (OnDepartureStartedHook->isActive())
	{
		// add scripting hook for 'On Departure Started' --wookieejedi
		// hook is placed at the beginning of this function to allow the scripter to
		// actually have access to the ship's departure decisions before they are all executed
		OnDepartureStartedHook->run(scripting::hooks::ShipDepartConditions{ shipp },
			scripting::hook_param_list(scripting::hook_param("Self", 'o', objp), scripting::hook_param("Ship", 'o', objp)));
	}

		// abort rearm, because if we entered this function we're either going to depart via hyperspace, depart via bay,
		// or revert to our default behavior
		ai_abort_rearm_request(objp);
	}

	// if our current goal is to warp, then we won't consider departing to a bay, because the goal explicitly says to warp out
	// (this sort of goal can be assigned in FRED, either in the ship's initial orders or as the ai-warp-out goal)
	if (goal_is_to_warp)
	{
		// aha, but not if we were ORDERED to depart, because the comms menu ALSO uses the goal code, and yet the comms menu means any departure method!
		if ((shipp->flags[Ship::Ship_Flags::Departure_ordered]) || ((shipp->wingnum >= 0) && (Wings[shipp->wingnum].flags[Ship::Wing_Flags::Departure_ordered])))
		{
			mprintf(("Looks like we were ordered to depart; initiating the standard departure logic\n"));
		}
		// since our goal is to warp, then if we can warp, jump directly to the warping part
		else if (ship_can_warp_full_check(shipp))
		{
			mprintf(("Our current goal is to warp!  Trying to warp...\n"));
			goto try_to_warp;
		}
		// otherwise, since we can't warp, we'll do the standard bay departure check, etc.
	}

	// if this ship belongs to a wing, then use the wing departure information
	if (beginning_departure && shipp->wingnum >= 0)
	{
		wing *wingp = &Wings[shipp->wingnum];

		// copy the wing's departure information to the ship
		// (needed because the bay departure code will check the ship's information again later on)
		shipp->departure_location = wingp->departure_location;
		shipp->departure_anchor = wingp->departure_anchor;
		shipp->departure_path_mask = wingp->departure_path_mask;
	}
	
	location = shipp->departure_location;
	anchor = shipp->departure_anchor;
	path_mask = shipp->departure_path_mask;

	// if departing to a docking bay, try to find the anchor ship to depart to.  If not found, then
	// just make it warp out like anything else.
	if (location == DepartureLocation::TO_DOCK_BAY)
	{
		Assert(anchor >= 0);
		auto anchor_ship_entry = ship_registry_get(Parse_names[anchor]);

		// see if ship is yet to arrive.  If so, then warp.
		if (!anchor_ship_entry || anchor_ship_entry->status == ShipStatus::NOT_YET_PRESENT)
		{
			mprintf(("Anchor ship %s hasn't arrived yet!  Trying to warp...\n", Parse_names[anchor].c_str()));
			goto try_to_warp;
		}

		// see if ship is in mission.  If not, then we can assume it was destroyed or departed since
		// it is not on the arrival list (as shown by above if statement).
		if (!anchor_ship_entry->has_shipp())
		{
			mprintf(("Anchor ship %s not found!  Trying to warp...\n", anchor_ship_entry->name));
			goto try_to_warp;
		}

		// see if we can actually depart to the ship
		if (!ship_useful_for_departure(anchor_ship_entry->shipnum, shipp->departure_path_mask))
		{
			mprintf(("Anchor ship %s not suitable for departure (dying, departing, bays destroyed, etc.).  Trying to warp...\n", anchor_ship_entry->name));
			goto try_to_warp;
		}

		// find a path
		if (ai_acquire_depart_path(objp, anchor_ship_entry->objnum, path_mask) >= 0)
		{
			MONITOR_INC(NumShipDepartures,1);

			mprintf(("Acquired departure path\n"));
			return 1;
		}
	}

try_to_warp:

	// make sure we can actually warp
	if (ship_can_warp_full_check(shipp))
	{
		if (aip->mode != AIM_WARP_OUT)
		{
			mprintf(("Setting mode to warpout\n"));

			ai_set_mode_warp_out(objp, aip);
			MONITOR_INC(NumShipDepartures, 1);
		}

		return 1;
	}
	// find something else to do
	else
	{
		// NOTE: this point should no longer be reached in the standard goal code, since the goal or comm order must be achievable
		// for this function to be called.  This point should only be reached if the ship has no subspace drive, AND either has no
		// mothership assigned (or departs to hyperspace) or the mothership was destroyed, AND falls into one of the following cases:
		// 1) The ship's departure cue evaluates to true in the mission
		// 2) A support ship has had its hull fall to 25% when it has no repair targets
		// 3) A fighter or bomber with an IFF that doesn't allow support ships has its warp_out_timestamp elapse (but this seems to not be a possibility anymore)
		// 4) An instructor in a training mission has been fired upon
		mprintf(("Can't warp!  Doing something else instead.\n"));

        shipp->flags.remove(Ship::Ship_Flags::Depart_dockbay);
        shipp->flags.remove(Ship::Ship_Flags::Depart_warp);
		ai_do_default_behavior(objp);

		return 0;
	}
}

/**
 * Put here because mission_eval_arrivals is here.
 * @todo Might move these to a better location later -- MWA
 */
void mission_eval_departures()
{
	int i, j;
	object *objp;
	wing *wingp;

	// scan through the active ships an evaluate their departure cues.  For those
	// ships whose time has come, set their departing flag.

	for (auto so: list_range(&Ship_obj_list)) {
		objp = &Objects[so->objnum];
		if (objp->flags[Object::Object_Flags::Should_be_dead])
			continue;
		if (objp->type == OBJ_SHIP) {
			ship	*shipp;

			Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

			shipp = &Ships[objp->instance];
			
			// don't process a ship that is already departing or dying or disabled
			// AL 12-30-97: Added SF_CANNOT_WARP to check
			// Goober5000 - fixed so that it WILL eval when SF_CANNOT_WARP if departing to dockbay
			// wookieejedi - fixed so it accounts for break and never warp too
			if ( shipp->is_dying_or_departing() || ( !(ship_can_warp_full_check(shipp)) && (shipp->departure_location != DepartureLocation::TO_DOCK_BAY)) || ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE) ) {
				continue;
			}

			// don't process ships that are part of a wing -- handled in seperate case
			if ( shipp->wingnum != -1 )
				continue;

			// when the departure cue becomes true, set off the departure delay timer.  We store the
			// timer as -seconds in FreeSpace which indicates that the timer has not been set.  If the timer
			// is not set, then turn it into a valid timer and keep evaluating the timer until it is elapsed
			if ( eval_sexp(shipp->departure_cue) ) {
				if ( shipp->departure_delay <= 0 )
					shipp->departure_delay = timestamp(-shipp->departure_delay * 1000 );
				if ( timestamp_elapsed(shipp->departure_delay) )
					mission_do_departure( objp );
			}
		}
	}

	// now scan through the list of wings and check their departure cues.  For wings with
	// that cue being true, we must update internal variables to indicate that the wing is
	// departed and that no further waves of this wing will appear

	for ( i = 0; i < Num_wings; i++ ) {
		wingp = &Wings[i];

		// should we process this wing anymore
		if ( wingp->flags[Ship::Wing_Flags::Departing] )
			continue;

		// evaluate the sexpression.  If true, mark all the ships in this wing as departing and increment
		// the num departed in the wing structure.  Then add number of remaining waves * ships/wave to
		// departed count to get total count of ships in the wing which departed.  (We are counting ships
		// that have not yet arrived as departed if they never arrive -- this may be bad, but for some reason
		// seems like the right thing to do).

		if ( eval_sexp(wingp->departure_cue) ) {
			// if we haven't set up the departure timer yet (would be <= 0) setup the timer to pop N seconds
			// later
			if ( wingp->departure_delay <= 0 )
				wingp->departure_delay = timestamp( -wingp->departure_delay * 1000 );
			if ( !timestamp_elapsed(wingp->departure_delay) )
				continue;

            wingp->flags.set(Ship::Wing_Flags::Departing);
			for ( j = 0; j < wingp->current_count; j++ ) {
				ship *shipp;

				shipp = &Ships[wingp->ship_index[j]];
				if ( (shipp->is_departing()) || (shipp->flags[Ship::Ship_Flags::Dying]) )
					continue;

				Assert ( shipp->objnum != -1 );
				objp = &Objects[shipp->objnum];

				mission_do_departure( objp );
				// don't add to wingp->total_departed here -- this is taken care of in ship code.
			}
		}
	}
}

/**
 * Called from high level game loop to do mission evaluation stuff
 */
void mission_parse_eval_stuff()
{
	mission_eval_arrivals();
	mission_eval_departures();
}

int allocate_subsys_status()
{
	int i;
	// set primary weapon ammunition here, but does it actually matter? - Goober5000

	Assert(Subsys_index >= 0);

	// we allocate in blocks of MIN_SUBSYS_STATUS_SIZE so if we need more then make more
	if ( (Subsys_status == NULL) || (Subsys_index >= (Subsys_status_size - 1)) ) {
		Assert( MIN_SUBSYS_STATUS_SIZE > 0 );

		Subsys_status_size += MIN_SUBSYS_STATUS_SIZE;
		Subsys_status = (subsys_status*)vm_realloc(Subsys_status, sizeof(subsys_status) * Subsys_status_size );
	}

	Verify( Subsys_status != NULL );

	// the memset is redundant to the below assignments
	//memset( &Subsys_status[Subsys_index], 0, sizeof(subsys_status) );

	Subsys_status[Subsys_index].name[0] = '\0';

	Subsys_status[Subsys_index].percent = 0.0f;

	Subsys_status[Subsys_index].primary_banks[0] = SUBSYS_STATUS_NO_CHANGE;
	Subsys_status[Subsys_index].primary_ammo[0] = 100; // *
	
	for (i=1; i<MAX_SHIP_PRIMARY_BANKS; i++)
	{
		Subsys_status[Subsys_index].primary_banks[i] = -1;  // none
		Subsys_status[Subsys_index].primary_ammo[i] = 100;	// *
	}

	Subsys_status[Subsys_index].secondary_banks[0] = SUBSYS_STATUS_NO_CHANGE;
	Subsys_status[Subsys_index].secondary_ammo[0] = 100;
	
	for (i=1; i<MAX_SHIP_SECONDARY_BANKS; i++)
	{
		Subsys_status[Subsys_index].secondary_banks[i] = -1;
		Subsys_status[Subsys_index].secondary_ammo[i] = 100;
	}

	Subsys_status[Subsys_index].ai_class = SUBSYS_STATUS_NO_CHANGE;

	Subsys_status[Subsys_index].subsys_cargo_name = 0;	// "Nothing"
	Subsys_status[Subsys_index].subsys_cargo_title[0] = '\0';

	return Subsys_index++;
}

// Goober5000
int insert_subsys_status(p_object *pobjp)
{
	int i, new_index;

	// this is not good; we have to allocate another slot, but then bump all the
	// slots upward so that this particular parse object's subsystems are contiguous
	new_index = allocate_subsys_status();

	// only bump the subsystems if this isn't the very last subsystem
	if (new_index != pobjp->subsys_index + pobjp->subsys_count)
	{
		// copy the new blank entry for future reference
		subsys_status temp_entry;
		memcpy(&temp_entry, &Subsys_status[new_index], sizeof(subsys_status));

		// shift elements upward
		for (i = Subsys_index - 1; i > (pobjp->subsys_index + pobjp->subsys_count); i--)
		{
			memcpy(&Subsys_status[i], &Subsys_status[i-1], sizeof(subsys_status));
		}

		// correct the index so that the new subsystem belongs to the proper p_object
		new_index = pobjp->subsys_index + pobjp->subsys_count;

		// put the blank entry in the p_object
		memcpy(&Subsys_status[new_index], &temp_entry, sizeof(subsys_status));
	}

	// make the p_object aware of its new subsystem
	pobjp->subsys_count++;

	// we also have to adjust all the indexes in existing parse objects
	// (each p_object's subsys_index points to subsystem 0 in its list)
	for (SCP_vector<p_object>::iterator ii = Parse_objects.begin(); ii != Parse_objects.end(); ++ii)
	{
		// bump up base index to accommodate inserted subsystem
		if (ii->subsys_index >= new_index)
			ii->subsys_index++;
	}

	return new_index;
}

// Goober5000
subsys_status *parse_get_subsys_status(p_object *pobjp, const char *subsys_name)
{
	int i;
	subsys_status *sssp;

	for (i = 0; i < pobjp->subsys_count; i++)
	{
		sssp = &Subsys_status[pobjp->subsys_index + i];

		if (!subsystem_stricmp(sssp->name, subsys_name))
			return sssp;
	}

	return NULL;
}

// find (or add) the name in the list and return an index to it.
int get_parse_name_index(const char *name)
{
	int i, count = static_cast<int>(Parse_names.size());

	for (i=0; i<count; ++i)
		if (!stricmp(name, Parse_names[i].c_str()))
			return i;

	Assert(strlen(name) < NAME_LENGTH);
	Parse_names.push_back(name);
	return i;
}

// Goober5000
int add_path_restriction()
{
	int i, j;

	// parse it
	path_restriction_t temp;
	temp.cached_mask = (1 << MAX_SHIP_BAY_PATHS);	// uninitialized value (too high)
	temp.num_paths = (int)stuff_string_list(temp.path_names, MAX_SHIP_BAY_PATHS);

	// no restriction?
	if (temp.num_paths == 0)
		return -1;

	// first, see if it's duplicated anywhere
	for (i = 0; i < Num_path_restrictions; i++)
	{
		// must have same number of allowed paths
		if (temp.num_paths != Path_restrictions[i].num_paths)
			continue;

		// see if path names match
		for (j = 0; j < temp.num_paths; j++)
		{
			// no match, so skip this
			if (stricmp(temp.path_names[j], Path_restrictions[i].path_names[j]) != 0)
				goto continue_outer_loop;
		}

		// match!
		return i;

continue_outer_loop:
		;
	}

	// no match, so add a new restriction

	// check limit
	if (Num_path_restrictions >= MAX_PATH_RESTRICTIONS)
	{
		Warning(LOCATION, "Maximum number of path restrictions reached");
		return -1;
	}

	// add this restriction at the new index
	int index = Num_path_restrictions++;
	Path_restrictions[index] = temp;

	return index;
}

/**
 * Look for \<any friendly\>, \<any hostile player\>, etc.
 */
int get_special_anchor(const char *name)
{
	char tmp[NAME_LENGTH + 15];
	const char *iff_name;
	int iff_index;
	
	if (strnicmp(name, "<any ", 5) != 0)
		return -1;

	strcpy_s(tmp, name+5);
	iff_name = strtok(tmp, " >");
	if (iff_name == nullptr)
		return -1;

	// hack substitute "hostile" for "enemy"
	if (!stricmp(iff_name, "enemy"))
		iff_name = "hostile";

	iff_index = iff_lookup(iff_name);
	if (iff_index < 0)
		return -1;

	// restrict to players?
	if (stristr(name+5, "player") != NULL)
		return (iff_index | SPECIAL_ARRIVAL_ANCHOR_FLAG | SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG);
	else
		return (iff_index | SPECIAL_ARRIVAL_ANCHOR_FLAG);
}

int get_anchor(const char *name)
{
	int special_anchor = get_special_anchor(name);

	if (special_anchor >= 0)
		return special_anchor;

	return get_parse_name_index(name);
}

/**
 * Fixup the goals/ai references for player objects in the mission
 */
void mission_parse_fixup_players()
{
	for (auto so: list_range(&Ship_obj_list)) {
		auto objp = &Objects[so->objnum];
		if (objp->flags[Object::Object_Flags::Should_be_dead])
			continue;

		if ( (objp->type == OBJ_SHIP) && (objp->flags[Object::Object_Flags::Player_ship]) ) {
			game_busy( NOX("** fixing up player/ai stuff **") );	// animate the loading screen, doesn't nothing if the screen is not active
			ai_clear_ship_goals( &Ai_info[Ships[objp->instance].ai_index] );
			init_ai_object( OBJ_INDEX(objp) );
		}
	}
}

// code to warp in a new support ship.  It works by finding the average position of all ships
// in the mission, creating a vector from that position to the player, and scaling out behind the
// player some distance.  Should be sufficient.

#define WARP_IN_MIN_DISTANCE	1000.0f
#define WARP_IN_TIME_MIN		3000				// warps in min 3 seconds later
#define WARP_IN_TIME_MAX		6000				// warps in max 6 seconds later

/**
 * Adds requester_objp onto the queue of ships for the arriving support ship to service
 */
void mission_add_to_arriving_support( object *requester_objp )
{
	int i;
	ship *shipp;

	Assert ( Arriving_support_ship );

	if ( Num_arriving_repair_targets == MAX_AI_GOALS ) {
		mprintf(("Reached MAX_AI_GOALS trying to add repair request!\n"));
		return;
	}

	shipp = &Ships[requester_objp->instance];
	// check for duplicates before adding
	for (i = 0; i < Num_arriving_repair_targets; i++ ) {
		if ( !stricmp(Arriving_repair_targets[i], shipp->ship_name) ){
			break;
		}
	}
	if ( i != Num_arriving_repair_targets ){		// found the ship before reaching the end -- ignore it!
		return;
	}

	strcpy_s( Arriving_repair_targets[Num_arriving_repair_targets], Ships[requester_objp->instance].ship_name );
	Num_arriving_repair_targets++;

	if ( MULTIPLAYER_MASTER ){
		multi_maybe_send_repair_info( requester_objp, NULL, REPAIR_INFO_WARP_ADD );
	}	
}

extern int pp_collide_any(vec3d *curpos, vec3d *goalpos, float radius, object *ignore_objp1, object *ignore_objp2, int big_only_flag);

/**
 * Set the warp in position for a support ship relative to an object.
 * Caller tries several positions, passing vector in x, y, z.
 */
int get_warp_in_pos(vec3d *pos, object *objp, float x, float y, float z)
{
	float	rand_val;

	if ( Game_mode & GM_NORMAL )
		rand_val = frand();
	else
		rand_val = static_randf(objp->net_signature);

	rand_val = 1.0f + (rand_val - 0.5f)*0.2f;

	*pos = objp->pos;

	vm_vec_scale_add2( pos, &objp->orient.vec.rvec, x*rand_val*800.0f);
	vm_vec_scale_add2( pos, &objp->orient.vec.uvec, y*rand_val*800.0f);
	vm_vec_scale_add2( pos, &objp->orient.vec.fvec, z*rand_val*800.0f);

	return pp_collide_any(&objp->pos, pos, objp->radius, objp, NULL, 1);
}

/**
 * Modified by Goober5000 to allow more flexibility in support ships
 */
void mission_bring_in_support_ship( object *requester_objp )
{
	vec3d center, warp_in_pos;
	p_object *pobj;
	ship *requester_shipp;
	int i;

	Assert ( requester_objp->type == OBJ_SHIP );
	requester_shipp = &Ships[requester_objp->instance];	//	MK, 10/23/97, used to be ->type, bogus, no?

	// if the support ship is already arriving, add the requester to the list
	if ( Arriving_support_ship ) {
		mission_add_to_arriving_support( requester_objp );
		return;
	}

	// If the support ship class was set via SEXP, use it. Otherwise, look it up by the requester's species.
	int ship_class = The_mission.support_ships.ship_class;
	if (ship_class < 0) {
		int requester_species = Ship_info[requester_shipp->ship_info_index].species;
		ship_class = Species_info[requester_species].support_ship_index;
		if ( ship_class < 0 ) {
			Warning(LOCATION, "Couldn't determine the support ship class to bring in\n");
			return;
		}
	}
	
	// create a parse object, and put it onto the ship arrival list.  This whole thing kind of stinks.
	// I want to put it into a parse object since it needs to arrive just a little later than
	// this function is called.  I have to make some assumptions in the code about values for the parse
	// object since I'm no longer working with a mission file.  These exceptions will be noted with
	// comments

	Support_ship_pobj = p_object();		// get a fresh p_object with default fields
	Arriving_support_ship = &Support_ship_pobj;
	pobj = Arriving_support_ship;
	pobj->ship_class = ship_class;
	auto sip = &Ship_info[ship_class];

	// initialize class-specific fields
	pobj->ai_class = sip->ai_class;
	pobj->warpin_params_index = sip->warpin_params_index;
	pobj->warpout_params_index = sip->warpout_params_index;
	pobj->ship_max_shield_strength = sip->max_shield_strength;
	pobj->ship_max_hull_strength = sip->max_hull_strength;
	Assert(pobj->ship_max_hull_strength > 0.0f);	// Goober5000: div-0 check (not shield because we might not have one)
	pobj->max_shield_recharge = sip->max_shield_recharge;
	pobj->replacement_textures = sip->replacement_textures;	// initialize our set with the ship class set, which may be empty
	pobj->score = sip->score;

	// get average position of all ships
	obj_get_average_ship_pos( &center );
	vm_vec_sub( &warp_in_pos, &center, &(requester_objp->pos) );

	//	Choose position to warp in ship.
	//	Temporary, but changed by MK because it used to be exactly behind the player.
	//	This could cause an Assert if the player immediately targeted it (before moving).
	//	Tend to put in front of the player to aid him in flying towards the ship.

	if (!get_warp_in_pos(&warp_in_pos, requester_objp, 1.0f, 0.1f, 1.0f))
		if (!get_warp_in_pos(&warp_in_pos, requester_objp, 1.0f, 0.2f, -1.0f))
			if (!get_warp_in_pos(&warp_in_pos, requester_objp, -1.0f, -0.2f, -1.0f))
				if (!get_warp_in_pos(&warp_in_pos, requester_objp, -1.0f, -0.1f, 1.0f))
					get_warp_in_pos(&warp_in_pos, requester_objp, 0.1f, 1.0f, 0.2f);

	// position for ship if it warps in
	pobj->pos = warp_in_pos;

	// tally the ship
	The_mission.support_ships.tally++;

	// create a name for the ship.  use "Support #".  look for collisions until one isn't found anymore
	i = 1;
	do {
		sprintf(pobj->name, NOX("Support %d"), i);
		if ( (ship_name_lookup(pobj->name) == -1) && (ship_find_exited_ship_by_name(pobj->name) == -1) )
			break;
		i++;
	} while(true);

	// create a ship registry entry for the support ship
	{
		ship_registry_entry entry(pobj->name);
		entry.status = ShipStatus::NOT_YET_PRESENT;
		entry.pobj_num = -1;	// since it's not in Parse_objects

		Ship_registry.push_back(entry);
		Ship_registry_map[pobj->name] = static_cast<int>(Ship_registry.size() - 1);
	}

	pobj->team = requester_shipp->team;

	// We will put the requester object shipname in repair target array and then take
	// care of setting up the goal when creating the ship!!!!
	Num_arriving_repair_targets = 0;
	mission_add_to_arriving_support( requester_objp );

	// Goober5000 - take some stuff from mission flags
	pobj->arrival_location = The_mission.support_ships.arrival_location;
	pobj->arrival_anchor = The_mission.support_ships.arrival_anchor;
	pobj->departure_location = The_mission.support_ships.departure_location;
	pobj->departure_anchor = The_mission.support_ships.departure_anchor;

	pobj->arrival_delay = timestamp_rand(WARP_IN_TIME_MIN, WARP_IN_TIME_MAX);
	pobj->arrival_cue = Locked_sexp_true;
	pobj->departure_cue = Locked_sexp_false;

	pobj->initial_velocity = 100;		// start at 100% velocity

    if (Player_obj->flags[Object::Object_Flags::No_shields])  {
		pobj->flags.set(Mission::Parse_Object_Flags::OF_No_shields);	// support ships have no shields when player has not shields
	}

	pobj->net_signature = multi_assign_network_signature(MULTI_SIG_SHIP);
}

/**
 * Returns true if a support ship is currently in the process of warping in.
 */
int mission_is_support_ship_arriving()
{
	if ( Arriving_support_ship )
		return 1;
	else
		return 0;
}

/**
 * Returns true if the given ship is scheduled to be repaired by the arriving support ship
 */
int mission_is_repair_scheduled( object *objp )
{
	char *name;
	int i;

	if ( !Arriving_support_ship )
		return 0;

	Assert ( objp->type == OBJ_SHIP );
	name = Ships[objp->instance].ship_name;
	for (i = 0; i < Num_arriving_repair_targets; i++ ) {
		if ( !strcmp( name, Arriving_repair_targets[i]) )
			return 1;
	}

	return 0;
}

/**
 * Removed the given ship from the list of ships that are to get repair by arriving support ship
 */
int mission_remove_scheduled_repair( object *objp )
{
	char *name;
	int i, index;

	if ( !Arriving_support_ship )
		return 0;

	// itereate through the target list looking for this ship name.  If not found, we
	// can simply return.
	Assert ( objp->type == OBJ_SHIP );
	name = Ships[objp->instance].ship_name;
	for (index = 0; index < Num_arriving_repair_targets; index++ ) {
		if ( !strcmp( name, Arriving_repair_targets[index]) )
			break;
	}
	if ( index == Num_arriving_repair_targets )
		return 0;

	// ship is found -- compress the array
	for ( i = index; i < Num_arriving_repair_targets - 1; i++ )
		strcpy_s( Arriving_repair_targets[i], Arriving_repair_targets[i+1] );

	Num_arriving_repair_targets--;

	if ( MULTIPLAYER_MASTER )
		multi_maybe_send_repair_info( objp, NULL, REPAIR_INFO_WARP_REMOVE );

	return 1;
}

/**
 * Alternate name stuff
 */
int mission_parse_lookup_alt(const char *name)
{
	int idx;

	// sanity
	if(name == NULL)
		return -1;

	// lookup
	for(idx=0; idx<Mission_alt_type_count; idx++)
	{
		if(!strcmp(Mission_alt_types[idx], name))
			return idx;
	}

	// could not find
	return -1;
}

static int mission_parse_lookup_alt_index_warn = 1;
const char *mission_parse_lookup_alt_index(int index)
{
	if((index < 0) || (index >= Mission_alt_type_count))
	{
		if (mission_parse_lookup_alt_index_warn)
		{
			Warning(LOCATION, "Ship with invalid alt_name.  Get a programmer");
			mission_parse_lookup_alt_index_warn = 0;
		}
		return "";
	}

	// get it
	return Mission_alt_types[index];
}

int mission_parse_add_alt(const char *name)
{
	// sanity
	if(name == NULL)
		return -1;

	// maybe add
	if(Mission_alt_type_count < MAX_ALT_TYPE_NAMES)
	{
		// stuff the name
		strcpy_s(Mission_alt_types[Mission_alt_type_count++], name);

		// done
		return Mission_alt_type_count - 1;
	}

	return -1;
}

void mission_parse_remove_alt(const char *name)
{
	// sanity
	if(name == NULL)
		return;

	// maybe remove
	for (int i = 0; i < Mission_alt_type_count; ++i)
	{
		if (!strcmp(Mission_alt_types[i], name))
		{
			// remove this name by overwriting it with the last name
			if (i < Mission_alt_type_count - 1)
				strcpy_s(Mission_alt_types[i], Mission_alt_types[Mission_alt_type_count - 1]);

			Mission_alt_type_count--;
			break;
		}
	}
}

void mission_parse_reset_alt()
{
	Mission_alt_type_count = 0;
}

// For compatibility purposes some mods want alt names to truncate at the hash symbol.  But we can't actually do that at mission load,
// since we have to save them again in FRED.  So this function processes the names just before the mission starts.
// To further complicate things, some mods actually want the hash to be displayed.  So this function uses the double-hash as an escape sequence for a single hash.
void mission_process_alt_types()
{
	for (int i = 0; i < Mission_alt_type_count; ++i)
	{
		// truncate at a single hash
		end_string_at_first_hash_symbol(Mission_alt_types[i], true);

		// ## -> #
		consolidate_double_characters(Mission_alt_types[i], '#');
	}
}

/**
 * Callsign stuff
 */
int mission_parse_lookup_callsign(const char *name)
{
	int idx;

	// sanity
	if(name == NULL)
		return -1;

	// lookup
	for(idx=0; idx<Mission_callsign_count; idx++)
	{
		if(!strcmp(Mission_callsigns[idx], name))
			return idx;
	}

	// could not find
	return -1;
}

static int mission_parse_lookup_callsign_index_warn = 1;
const char *mission_parse_lookup_callsign_index(int index)
{
	if((index < 0) || (index >= Mission_callsign_count))
	{
		if (mission_parse_lookup_callsign_index_warn)
		{
			Warning(LOCATION, "Ship with invalid callsign.  Get a programmer");
			mission_parse_lookup_callsign_index_warn = 0;
		}
		return "";
	}

	// get it
	return Mission_callsigns[index];
}

int mission_parse_add_callsign(const char *name)
{
	// sanity
	if(name == NULL)
		return -1;

	// maybe add
	if(Mission_callsign_count < MAX_CALLSIGNS)
	{
		// stuff the name
		strcpy_s(Mission_callsigns[Mission_callsign_count++], name);

		// done
		return Mission_callsign_count - 1;
	}

	return -1;
}

void mission_parse_remove_callsign(const char *name)
{
	// sanity
	if(name == NULL)
		return;

	// maybe remove
	for (int i = 0; i < Mission_callsign_count; ++i)
	{
		if (!strcmp(Mission_callsigns[i], name))
		{
			// remove this callsign by overwriting it with the last callsign
			if (i < Mission_callsign_count - 1)
				strcpy_s(Mission_callsigns[i], Mission_callsigns[Mission_callsign_count - 1]);

			Mission_callsign_count--;
			break;
		}
	}
}

void mission_parse_reset_callsign()
{
	Mission_callsign_count = 0;
}

int is_training_mission()
{
	return (The_mission.game_type & MISSION_TYPE_TRAINING);
}

/**
 * Go through all the displayed text in one section and fix the section and text delimiters should all be different
 */
void conv_fix_punctuation_section(char *str, const char *section_start, const char *section_end, const char *text_start,
								  const char *text_end)
{
	char *s1, *s2, *t1, *t2;

	s1 = strstr(str, section_start);
	s2 = strstr(s1, section_end);

	t1 = s1;
	
	while (1)
	{
		t1 = strstr(t1+1, text_start);
		if (!t1 || t1 > s2) return;

		t2 = strstr(t1, text_end);
		if (!t2 || t2 > s2) return;

		replace_all(t1, "\"", "$quote", PARSE_TEXT_SIZE - (str - Parse_text) - 1, (t2 - t1));
	}	
}
	
// Goober5000
void conv_fix_punctuation()
{
	// command briefings
	conv_fix_punctuation_section(Parse_text, "#Command Briefing", "#Briefing", "$Stage Text:", "$end_multi_text");

	// briefings
	conv_fix_punctuation_section(Parse_text, "#Briefing", "#Debriefing_info", "$multi_text", "$end_multi_text");

	// debriefings
	conv_fix_punctuation_section(Parse_text, "#Debriefing_info", "#Players", "$Multi text", "$end_multi_text");

	// messages
	conv_fix_punctuation_section(Parse_text, "#Messages", "#Reinforcements", "$Message:", "\n");
}

// Goober5000
void convertFSMtoFS2()
{
	// fix punctuation
	conv_fix_punctuation();
}

void clear_texture_replacements() 
{
	Fred_texture_replacements.clear();
}

bool check_for_23_3_data()
{
	auto e_list = ai_lua_get_general_orders(true);
	if (!e_list.empty())
		return true;
	auto v_list = ai_lua_get_general_orders(false, true);
	if (!v_list.empty())
		return true;

	for (const auto& msg : Messages)
	{
		if (!msg.note.empty())
			return true;
	}

	if (!The_mission.custom_data.empty()) {
		return true;
	}

	if (!The_mission.custom_strings.empty()) {
		return true;
	}

	for (const auto& so : list_range(&Ship_obj_list))
	{
		auto shipp = &Ships[Objects[so->objnum].instance];

		auto shipp_params = &Warp_params[shipp->warpin_params_index];
		auto sip_params = &Warp_params[Ship_info[shipp->ship_info_index].warpin_params_index];
		if (shipp_params->supercap_warp_physics != sip_params->supercap_warp_physics)
			return true;

		shipp_params = &Warp_params[shipp->warpout_params_index];
		sip_params = &Warp_params[Ship_info[shipp->ship_info_index].warpout_params_index];
		if (shipp_params->supercap_warp_physics != sip_params->supercap_warp_physics)
			return true;
	}

	for (int t = 0; t < Num_teams; t++) {
		if (Team_data[t].do_not_validate) {
			return true;
		}
	}

	return false;
}

bool check_for_24_1_data()
{
	for (int wingnum = 0; wingnum < Num_wings; wingnum++)
	{
		if (Wings[wingnum].arrival_location == ArrivalLocation::IN_BACK_OF_SHIP || Wings[wingnum].arrival_location == ArrivalLocation::ABOVE_SHIP || Wings[wingnum].arrival_location == ArrivalLocation::BELOW_SHIP
			|| Wings[wingnum].arrival_location == ArrivalLocation::TO_LEFT_OF_SHIP || Wings[wingnum].arrival_location == ArrivalLocation::TO_RIGHT_OF_SHIP)
			return true;
	}

	for (const auto& so : list_range(&Ship_obj_list))
	{
		auto shipp = &Ships[Objects[so->objnum].instance];

		if (shipp->arrival_location == ArrivalLocation::IN_BACK_OF_SHIP || shipp->arrival_location == ArrivalLocation::ABOVE_SHIP || shipp->arrival_location == ArrivalLocation::BELOW_SHIP
			|| shipp->arrival_location == ArrivalLocation::TO_LEFT_OF_SHIP || shipp->arrival_location == ArrivalLocation::TO_RIGHT_OF_SHIP)
			return true;

		if (shipp->cargo_title[0] != '\0')
			return true;
		for (const auto& ss : list_range(&shipp->subsys_list))
		{
			if (ss->subsys_cargo_title[0] != '\0')
				return true;
		}
	}

	return (Asteroid_field.debris_genre == DG_DEBRIS && !Asteroid_field.field_debris_type.empty()) ||
		   (Asteroid_field.debris_genre == DG_ASTEROID && !Asteroid_field.field_asteroid_type.empty());
}

bool check_for_24_3_data()
{
	for (int i = 0; i < Num_teams; i++) {
		for (int j = 0; j < Briefings[i].num_stages; j++) {
			if (!gr_compare_color_values(Briefings[i].stages[j].grid_color, Color_briefing_grid)) {
				return true;
			}
		}
	}
	return false;
}

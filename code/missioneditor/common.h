#pragma once
#include "globalincs/globals.h"
#include "mission/missionmessage.h"
#include "ship/anchor_t.h"

// Default AWACS range applied when the nebula intensity is unset or invalid
constexpr float DEFAULT_NEBULA_RANGE = 3000.0f;

// Smallest meaningful change in an orientation input field (degrees)
constexpr float ORIENT_INPUT_THRESHOLD = 0.01f;

// Normalize a degree value into the range [-180, 180]
float normalize_degrees(float deg);

// Voice acting manager
#define INVALID_MESSAGE ((MMessage*)SIZE_MAX) // was originally SIZE_T_MAX but that wasn't available outside fred. May need more research.

enum class PersonaSyncIndex : int {
	Wingman = 0,      // <Wingman Personas>
	NonWingman = 1,   // <Non-Wingman Personas>
	PersonasStart = 2 // indices >= 2 map to specific persona
};

extern char Voice_abbrev_briefing[NAME_LENGTH];
extern char Voice_abbrev_campaign[NAME_LENGTH];
extern char Voice_abbrev_command_briefing[NAME_LENGTH];
extern char Voice_abbrev_debriefing[NAME_LENGTH];
extern char Voice_abbrev_message[NAME_LENGTH];
extern char Voice_abbrev_mission[NAME_LENGTH];
extern bool Voice_no_replace_filenames;
extern char Voice_script_entry_format[NOTES_LENGTH];
extern int Voice_export_selection;
extern bool Voice_group_messages;

extern SCP_string Voice_script_default_string;
extern SCP_string Voice_script_instructions_string;

void time_to_mission_info_string(const std::tm* src, char* dest, size_t dest_max_len);

void stuff_special_arrival_anchor_name(char* buf, int iff_index, int restrict_to_players, bool retail_format);

void stuff_special_arrival_anchor_name(char* buf, int anchor_num, bool retail_format);

int anchor_to_target(anchor_t anchor);

anchor_t target_to_anchor(int target);

// Rebuild Starting_wings[], Squadron_wings[], TVT_wings[] from their parallel
// name arrays via wing_name_lookup.  Consolidated from FRED's and qtFRED's
// previously-duplicated copies.
void update_custom_wing_indexes();

void generate_weaponry_usage_list_team(int team, int* arr);

void generate_weaponry_usage_list_wing(int wing_num, int* arr);

// If Player_start_shipnum no longer refers to a valid player start ship, repoint it to the
// first remaining player start in the mission (or -1 if there are none).  Call this after
// changing a ship to or from an OBJ_START via demotion, deletion, etc.
void ensure_valid_player_start_shipnum();

// Make the given object the sole player start, per single-player semantics: promote it to
// OBJ_START, demote every other player start to OBJ_SHIP, adjust Player_starts to match,
// and repoint Player_start_shipnum.  Returns true if anything changed.
bool set_single_player_start(int objnum);

struct FredShipSlotConfig
{
	char (*fred_alt_names)[NAME_LENGTH + 1] = nullptr;
	char (*fred_callsigns)[NAME_LENGTH + 1] = nullptr;

	int *cur_ship = nullptr;
};

// Move the ship currently in Ships[from] into Ships[to], updating every
// back-reference (Objects, Ai_info, Wings, Player_start_shipnum, Ship_registry,
// and editor-side fields supplied via cfg).  Leaves Ships[from] empty.
// Preconditions: from != to, Ships[from].objnum >= 0, Ships[to].objnum < 0.
// No caller may hold a ship* to either slot across this call.
// Fields in cfg whose pointers are nullptr are skipped.
void reassign_ship_slot(int from, int to, const FredShipSlotConfig& cfg, bool resort_obj_list = true);

// Swap the contents of two slots.  Both must be valid (Ships[a].objnum >= 0
// and Ships[b].objnum >= 0).  Implemented as three calls to reassign_ship_slot
// via a temporary empty slot.
void swap_ship_slots(int a, int b, const FredShipSlotConfig& cfg);

// Move the item at position from_pos in slots to position to_pos, shifting the
// items in between by one position.
void rotate_ship_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredShipSlotConfig& cfg);

struct FredWingSlotConfig
{
	int (*wing_objects)[MAX_SHIPS_PER_WING] = nullptr;
	int *cur_wing = nullptr;
};

// Move the wing currently in Wings[from] into Wings[to], updating every
// back-reference (Ships[i].wingnum, Starting/Squadron/TVT_wings caches, and
// editor-side fields supplied via cfg).  Leaves Wings[from] empty.
// Preconditions: from != to, Wings[from].wave_count > 0, Wings[to].wave_count == 0.
// No caller may hold a wing* to either slot across this call.
// Fields in cfg whose pointers are nullptr are skipped.
void reassign_wing_slot(int from, int to, const FredWingSlotConfig& cfg, bool update_wing_indexes = true);

// Swap the contents of two slots.  Both must be valid (Wings[a].wave_count > 0
// and Wings[b].wave_count > 0).  Implemented as three calls to
// reassign_wing_slot via a temporary empty slot.
void swap_wing_slots(int a, int b, const FredWingSlotConfig& cfg);

// Move the item at position from_pos in slots to position to_pos, shifting the
// items in between by one position.
void rotate_wing_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredWingSlotConfig& cfg);

// Restore the obj_used_list invariant for the OBJ_SHIP/OBJ_START subset:
// among ship-type entries, list order matches Ships[] index order.
void resort_ships_in_obj_used_list();

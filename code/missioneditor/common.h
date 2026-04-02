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

void generate_weaponry_usage_list_team(int team, int* arr);

void generate_weaponry_usage_list_wing(int wing_num, int* arr);

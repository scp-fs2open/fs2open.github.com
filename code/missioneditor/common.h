#pragma once
#include "globalincs/globals.h"
#include "mission/missionmessage.h"

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

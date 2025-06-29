/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "mission/missionmessage.h"

#include "anim/animplay.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/utility.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "hud/hudgauges.h"
#include "hud/hudmessage.h"
#include "hud/hudtarget.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "localization/localize.h"
#include "math/vecmat.h"
#include "mission/missiontraining.h"
#include "mission/missiongoals.h"
#include "mod_table/mod_table.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "parse/parselo.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "scripting/api/objs/message.h"
#include "scripting/hook_api.h"
#include "scripting/scripting.h"
#include "ship/ship.h"
#include "ship/subsysdamage.h"
#include "sound/audiostr.h"
#include "sound/fsspeech.h"
#include "species_defs/species_defs.h"
#include "utils/Random.h"

bool Allow_generic_backup_messages = false;
bool Always_loop_head_anis = false;
bool Use_newer_head_ani_suffix = false;
float Command_announces_enemy_arrival_chance = 0.25;

#define DEFAULT_MOOD 0
SCP_vector<SCP_string> Builtin_moods;
int Current_mission_mood;

builtin_message::builtin_message(const char* _name, int _occurrence_chance, int _max_count, int _min_delay, int _priority, int _timing, int _fallback, bool _used_strdup)
	: name(_name), occurrence_chance(_occurrence_chance), max_count(_max_count), min_delay(_min_delay), priority(_priority), timing(_timing), fallback(_fallback), used_strdup(_used_strdup)
{}

builtin_message::~builtin_message()
{
	if (used_strdup)
	{
		vm_free(const_cast<char*>(name));
		name = nullptr;
	}
}

builtin_message::builtin_message(const builtin_message& other)
	: name(other.name), occurrence_chance(other.occurrence_chance), max_count(other.max_count), min_delay(other.min_delay), priority(other.priority), timing(other.timing), fallback(other.fallback), used_strdup(other.used_strdup)
{
	if (other.used_strdup)
		name = vm_strdup(other.name);
}

builtin_message& builtin_message::operator=(const builtin_message& other)
{
	name = other.used_strdup ? vm_strdup(other.name) : other.name;
	occurrence_chance = other.occurrence_chance;
	max_count = other.max_count;
	min_delay = other.min_delay;
	priority = other.priority;
	timing = other.timing;
	fallback = other.fallback;
	used_strdup = other.used_strdup;
	return *this;
}

SCP_vector<builtin_message> Builtin_messages = {
  #define X(_, NAME, CHANCE, COUNT, DELAY, PRIORITY, TIME, FALLBACK) { \
    NAME,                                                              \
    CHANCE,                                                            \
    COUNT,                                                             \
    DELAY,                                                             \
    MESSAGE_PRIORITY_ ## PRIORITY,                                     \
    MESSAGE_TIME_ ## TIME,                                             \
    MESSAGE_ ## FALLBACK,                                              \
    false                                                              \
  }
  BUILTIN_MESSAGE_TYPES
  #undef X
};

constexpr int BUILTIN_BOOST_LEVEL_ONE =  1;
constexpr int BUILTIN_BOOST_LEVEL_TWO =  2;
constexpr int BUILTIN_MATCHES_TYPE    =  4;
constexpr int BUILTIN_MATCHES_FILTER  =  8;
constexpr int BUILTIN_MATCHES_MOOD    = 16;
constexpr int BUILTIN_MATCHES_SPECIES = 32;
constexpr int BUILTIN_MATCHES_PERSONA = 64;

constexpr int BUILTIN_BOOST_LEVEL_THREE = (BUILTIN_BOOST_LEVEL_ONE | BUILTIN_BOOST_LEVEL_TWO);

int get_builtin_message_type(const char* name) {
	size_t count = Builtin_messages.size();
	for (unsigned int i = 0; i < count; i++) {
		if (!stricmp(Builtin_messages[i].name, name)) {
			return i;
		}
	}
	return MESSAGE_NONE;
}

SCP_vector<MMessage> Messages;

int Num_messages, Num_message_avis, Num_message_waves;
int Num_builtin_messages, Num_builtin_avis, Num_builtin_waves;

int Message_debug_index = -1;

SCP_vector<message_extra> Message_avis;
SCP_vector<message_extra> Message_waves;

#define MAX_PLAYING_MESSAGES		2

#define MAX_WINGMAN_HEADS			2
#define MAX_COMMAND_HEADS			3

//XSTR:OFF
#define HEAD_PREFIX_STRING			"head-"
#define COMMAND_HEAD_PREFIX		"head-cm1"
#define COMMAND_WAVE_PREFIX		"TC_"
#define SUPPORT_NAME					"Support"
//XSTR:ON

// variables to keep track of messages that are currently playing
int Num_messages_playing;						// number of is a message currently playing?

pmessage Playing_messages[MAX_PLAYING_MESSAGES];

int Message_shipnum;						// ship number of who is sending message to player -- used outside this module
static TIMESTAMP Message_expire;			// timestamp to extend the duration of message brackets when not using voice files

// variables to control message queuing.  All new messages to the player are queued.  The array
// will be ordered by priority, then time submitted.

#define MQF_CONVERT_TO_COMMAND		(1<<0)			// convert this queued message to terran command
#define MQF_CHECK_ALIVE					(1<<1)			// check for the existence of who_from before sending

typedef struct message_q {
	fix	time_added;					// time at which this entry was added
	TIMESTAMP window_timestamp;		// timestamp which will tell us how long we have to play the message
	int	priority;					// priority of the message
	int	message_num;				// index into the Messages[] array
	SCP_vm_unique_ptr<char> special_message;	// Goober5000 - message to play if we've replaced stuff (like variables)
	char who_from[NAME_LENGTH];		// who this message is from
	int	source;						// who the source of the message is (HUD_SOURCE_* type)
	int	builtin_type;				// type of builtin message (-1 if mission message)
	int	flags;						// should this message entry be converted to Terran Command head/wave file
	TIMESTAMP min_delay_stamp;		// minimum delay before this message will start playing
	int	group;						// message is part of a group, don't time it out
	int event_num_to_cancel;		// Goober5000 - if this event is true, the message will not be played
} message_q;

#define DEFAULT_MESSAGE_LENGTH	3000			// default number of milliseconds to display message indicator on hud
SCP_vector<message_q>	MessageQ;
int MessageQ_num;			// keeps track of number of entries on the queue.

#define MESSAGE_IMMEDIATE_TIMESTAMP		1000		// immediate messages must play within 1 second
#define MESSAGE_SOON_TIMESTAMP			5000		// "soon" messages must play within 5 seconds

// Persona information
SCP_vector<Persona> Personas;

const char *Persona_type_names[MAX_PERSONA_TYPES] =
{
//XSTR:OFF
	"wingman",
	"support",
	"large", 
	"command",
//XSTR:ON
};

int Default_command_persona, Default_support_persona;

// Goober5000
// NOTE - these are truncated filenames, i.e. without extensions
SCP_vector<SCP_string> Generic_message_filenames;

///////////////////////////////////////////////////////////////////
// used to distort incoming messages when comms are damaged
///////////////////////////////////////////////////////////////////
static int Message_wave_muted;
static int Message_wave_duration;
static int Next_mute_time;

#define MAX_DISTORT_PATTERNS	2
#define MAX_DISTORT_LEVELS		6
static float Distort_patterns[MAX_DISTORT_PATTERNS][MAX_DISTORT_LEVELS] = 
{
	{0.20f, 0.20f, 0.20f, 0.20f, 0.20f, 0.20f},
	{0.10f, 0.20f, 0.25f, 0.25f, 0.05f, 0.15f}
};

static int Distort_num;		// which distort pattern is being used
static int Distort_next;	// which section of distort pattern is next

int Head_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		7, 45
	},
	{ // GR_1024
		7, 66
	}
};

const auto OnMessageReceivedHook = scripting::Hook<>::Factory(
	"On Message Received",
	"Invoked when a mission sends a message.",
	{
		{"Name", "string", "The name of the message in the mission"},
		{"Message",
		 "string",
		 "The text of the sent message. This will have any placeholder expanded (e.g. SEXP "
		 "variables) and will be what the player sees on the HUD."},
		{"SenderString", "string", "The source of the message as a string. Same as used by the engine on the HUD."},
		{"Builtin", "boolean", "true if this is a builtin message, false of this is a mission message"},
		{"Sender",
		 "ship",
		 "If sent from an object, the object that has sent the message. Invalid if not sent from an object"},
		{"MessageHandle", "message", "The scripting handle of the message being sent."},
	});

// forward declarations
void message_maybe_distort_text(SCP_string &text, int shipnum, bool for_death_scream);
int comm_between_player_and_ship(int other_shipnum, bool for_death_scream);

// following functions to parse messages.tbl -- code pretty much ripped from weapon/ship table parsing code

// functions to deal with parsing personas.  Personas are just a list of names that give someone
// sending a message an identity which spans the life of the mission
void persona_parse()
{
	Persona this_persona;
	this_persona.flags = 0;
	this_persona.species_bitfield = 0;

	required_string("$Persona:");
	stuff_string(this_persona.name, F_NAME, NAME_LENGTH);
	
	bool dup = false;
	if (message_persona_name_lookup(this_persona.name) >= 0) {
		Warning(LOCATION, "Duplicate Persona %s found, ignoring!", this_persona.name);
		dup = true;
	}

	// get the type name and set the appropriate flag
	required_string("$Type:");
	char type[NAME_LENGTH];
	stuff_string( type, F_NAME, NAME_LENGTH );

	int i;
	for ( i = 0; i < MAX_PERSONA_TYPES; i++ ) {
		if ( !stricmp( type, Persona_type_names[i]) ) {

			this_persona.flags |= (1 << i);

			break;
		}
	}

	if ( i == MAX_PERSONA_TYPES )
		WarningEx(LOCATION, "Unknown persona type in messages.tbl -- %s\n", type );

	char cstrtemp[NAME_LENGTH];
	while ( optional_string("+") )
	{
		stuff_string(cstrtemp, F_NAME, NAME_LENGTH);
		int j = species_info_lookup(cstrtemp);

		if (j >= 0)
		{
			if (j < 32)
				this_persona.species_bitfield |= (1 << j);
			else
				Warning(LOCATION, "Species %s is index 32 or higher and therefore cannot be assigned to the persona species bitfield of %s",
					Species_info[j].species_name, this_persona.name);
		}
		else
			WarningEx(LOCATION, "Unknown species in messages.tbl -- %s\n", cstrtemp );
	}

	// if no species were assigned, the persona should default to the first species, since that's how retail did it
	if (this_persona.species_bitfield == 0)
		this_persona.species_bitfield = (1 << 0);

	if (optional_string("$Allow substitution of missing messages:"))
	{
		bool temp;
		stuff_boolean(&temp);
		if (temp)
			this_persona.flags |= PERSONA_FLAG_SUBSTITUTE_MISSING_MESSAGES;
	}
	else 
	{
		this_persona.flags |= PERSONA_FLAG_SUBSTITUTE_MISSING_MESSAGES;
	}

	if (optional_string("$No automatic assignment:"))
	{
		bool temp;
		stuff_boolean(&temp);
		if (temp)
			this_persona.flags |= PERSONA_FLAG_NO_AUTOMATIC_ASSIGNMENT;
	}

	if (!dup) {
		int persona_index = (int) Personas.size();
		Personas.push_back(this_persona);

		// Save some important personae for later
		if (this_persona.flags & PERSONA_FLAG_COMMAND) {
			// Always use the most recent Command persona found, since that's how retail does it
			Default_command_persona = persona_index;
		}
		if ((this_persona.flags & PERSONA_FLAG_SUPPORT) && (Default_support_persona == -1)) {
			Default_support_persona = persona_index;
		}
	}
}

// two functions to add avi/wave names into a table
int add_avi( char *avi_name )
{
	int i;
	message_extra extra; 

	Assert (strlen(avi_name) < MAX_FILENAME_LEN );

	// check to see if there is an existing avi being used here
	for ( i = 0; i < (int)Message_avis.size(); i++ ) {
		if ( !stricmp(Message_avis[i].name, avi_name) )
			return i;
	}

	// would have returned if a slot existed.
	generic_anim_init( &extra.anim_data, avi_name );
	strcpy_s( extra.name, avi_name );
	extra.num    = sound_load_id::invalid();
	extra.exists = (generic_anim_load(&extra.anim_data) == 0); // load only to validate the anim
	generic_anim_unload(&extra.anim_data); // unload to not waste bmpman slots
	Message_avis.push_back(extra); 
	Num_message_avis++;
	return ((int)Message_avis.size() - 1);
}

int add_wave( const char *wave_name )
{
	int i;
	message_extra extra; 

	Assert (strlen(wave_name) < MAX_FILENAME_LEN );

	// check to see if there is an existing wave being used here
	for ( i = 0; i < (int)Message_waves.size(); i++ ) {
		if ( !stricmp(Message_waves[i].name, wave_name) )
			return i;
	}

	generic_anim_init( &extra.anim_data );
	strcpy_s( extra.name, wave_name );
	extra.num = sound_load_id::invalid();
	Message_waves.push_back(extra);
	Num_message_waves++;
	return ((int)Message_waves.size() - 1);
}

void message_filter_clear(MessageFilter& filter) {
	filter.species_bitfield = 0;
	filter.type_bitfield = 0;
	filter.team_bitfield = 0;
}

void message_filter_parse(MessageFilter& filter) {
	SCP_string buf;
	message_filter_clear(filter);
	while (optional_string("+Ship name:")) {
		stuff_string(buf, F_NAME);
		filter.ship_name.push_back(buf);
	}
	while (optional_string("+Callsign:")) {
		stuff_string(buf, F_NAME);
		filter.callsign.push_back(buf);
	}
	while (optional_string("+Class name:")) {
		stuff_string(buf, F_NAME);
		filter.class_name.push_back(buf);
	}
	while (optional_string("+Wing name:")) {
		stuff_string(buf, F_NAME);
		filter.wing_name.push_back(buf);
	}
	while (optional_string("+Species:")) {
		stuff_string(buf, F_NAME);
		int species = species_info_lookup(buf.c_str());
		if (species < 0) {
			Warning(LOCATION, "Unknown species %s in messages.tbl", buf.c_str());
		} else if (species >= 32) {
			Warning(LOCATION, "Species %s is index 32 or higher and therefore cannot be used in a message filter", buf.c_str());
		} else {
			filter.species_bitfield |= (1 << species);
		}
	}
	while (optional_string("+Type:")) {
		stuff_string(buf, F_NAME);
		int type = ship_type_name_lookup(buf.c_str());
		if (type < 0) {
			Warning(LOCATION, "Unknown ship type %s in messages.tbl", buf.c_str());
		} else if (type >= 32) {
			Warning(LOCATION, "Type %s is index 32 or higher and therefore cannot be used in a message filter", buf.c_str());
		} else {
			filter.type_bitfield |= (1 << type);
		}
	}
	while (optional_string("+Team:")) {
		stuff_string(buf, F_NAME);
		int team = iff_lookup(buf.c_str());
		if (team < 0) {
			Warning(LOCATION, "Unknown team %s in messages.tbl", buf.c_str());
		} else if (team >= 32) {
			Warning(LOCATION, "Team %s is index 32 or higher and therefore cannot be used in a message filter", buf.c_str());
		} else {
			filter.team_bitfield |= (1 << team);
		}
	}
}

void handle_legacy_backup_message(MissionMessage& msg, SCP_string wing_name) {
	static bool warned = false;
	if (!warned) {
		WarningEx(LOCATION,
			"Converting legacy '%s Arrived' message. Consult the documentation on message filters for more information. "
			"A complete list will be printed to the log.",
			wing_name.c_str());
		warned = true;
	} 
	
	mprintf(("Converting legacy '%s Arrived' message for message %s with persona %s.\n",
			wing_name.c_str(),
			msg.name,
			Personas[msg.persona_index].name));

	static const char* backup = Builtin_messages[MESSAGE_BACKUP].name;
	msg.sender_filter.wing_name.push_back(wing_name);
	strcpy(msg.name, backup);
}

int lookup_mood(SCP_string const& name) {
	for (auto i = Builtin_moods.begin(); i != Builtin_moods.end(); ++i) {
		if (lcase_equal(*i, name)) {
			return (int) std::distance(Builtin_moods.begin(), i);
		}
	}
	Warning(LOCATION, "Message.tbl has an entry for mood type %s, but this mood is not in the #Moods section of the table.", name.c_str());
	return -1;
}

// parses an individual message
void message_parse(MessageFormat format) {
	MissionMessage msg;
	char persona_name[NAME_LENGTH];

	required_string("$Name:");
	stuff_string(msg.name, F_NAME, NAME_LENGTH);

	// team
	msg.multi_team = -1;
	if (optional_string("$Team:")) {
		int mt;
		stuff_int(&mt);

		// keep it real
		if ((mt < 0) || (mt >= MAX_TVT_TEAMS)) {
			mt = -1;
		}

		// only bother with filters in-game if multiplayer and TvT
		if (Fred_running || (MULTI_TEAM)) {
			msg.multi_team = mt;
		}
	}

	// backwards compatibility for old fred missions - all new ones should use $MessageNew
	if (optional_string("$Message:")) {
		stuff_string(msg.message, F_MESSAGE, MESSAGE_LENGTH);
	} else {
		required_string("$MessageNew:");
		stuff_string(msg.message, F_MULTITEXT, MESSAGE_LENGTH);
	}

	msg.persona_index = -1;
	if (optional_string("+Persona:")) {
		stuff_string(persona_name, F_NAME, NAME_LENGTH);
		msg.persona_index = message_persona_name_lookup(persona_name);

		if (msg.persona_index == -1) {
			WarningEx(LOCATION, "Unknown persona in message %s in messages.tbl -- %s\n", msg.name, persona_name );
		}
	}

	if (!Fred_running) {
		msg.avi_info.index = -1;
	} else {
		msg.avi_info.name = NULL;
	}

	if (optional_string("+AVI Name:")) {
		char avi_name[MAX_FILENAME_LEN];

		stuff_string(avi_name, F_NAME, MAX_FILENAME_LEN);

		// Goober5000 - for some reason :V: swapped Head-TP1 and Head-TP4 in FS2
		if (format == MessageFormat::FS1_MISSION && !strnicmp(avi_name, "Head-TP1", 8)) {
			avi_name[7] = '4';
		}

		if (!Fred_running) {
			msg.avi_info.index = add_avi(avi_name);
		} else {
			msg.avi_info.name = vm_strdup(avi_name);
		}
	}

	if (!Fred_running) {
		msg.wave_info.index = -1;
	} else {
		msg.wave_info.name = NULL;
	}

	if (optional_string("+Wave Name:")) {
		char wave_name[MAX_FILENAME_LEN];

		stuff_string(wave_name, F_NAME, MAX_FILENAME_LEN);
		if (!Fred_running) {
			msg.wave_info.index = add_wave(wave_name);
		} else {
			msg.wave_info.name = vm_strdup(wave_name);
		}
	}

	if (optional_string("+Note:")) {
		if (Fred_running) { // Msg stage notes do nothing in FSO, so let's not even waste a few bytes
			stuff_string(msg.note, F_MULTITEXT);
			lcl_replace_stuff(msg.note, true);
		} else {
			SCP_string junk;
			stuff_string(junk, F_MULTITEXT);
		}
	}

	bool require_exact_mood_match;
	if (optional_string("$Mood:")) {
		SCP_string buf;
		stuff_string(buf, F_NAME);
		msg.mood = lookup_mood(buf);
		require_exact_mood_match = optional_string("+Require exact match");
	} else {
		msg.mood = DEFAULT_MOOD;
		require_exact_mood_match = false;
	}

	if (require_exact_mood_match) {
		for (auto i = Builtin_moods.begin(); i != Builtin_moods.end(); ++i) {
			int mood = (int) std::distance(Builtin_moods.begin(), i);
			if (mood != msg.mood) {
				msg.excluded_moods.push_back(mood);
			}
		}
	} else if (optional_string("$Exclude Mood:")) {
		SCP_vector<SCP_string> buf;
		stuff_string_list(buf);
		for (auto i = buf.begin(); i != buf.end(); ++i) {
			int mood = lookup_mood(*i);
			if (mood >= 0) {
				msg.excluded_moods.push_back(mood);
			}
		}
	}

	if (optional_string("$Filter by sender:")) {
		message_filter_parse(msg.sender_filter);
	} else {
		message_filter_clear(msg.sender_filter);
	}

	if (optional_string("$Filter by subject:")) {
		message_filter_parse(msg.subject_filter);
	} else {
		message_filter_clear(msg.subject_filter);
	}

	msg.outer_filter_radius = -1;
	if (optional_string("$Filter by other ship:")) {
		if (optional_string("+Within range of sender:")) {
			stuff_int(&msg.outer_filter_radius);
		}
		message_filter_parse(msg.outer_filter);
	} else {
		message_filter_clear(msg.outer_filter);
	}

	if (optional_string("$Prefer this message very highly")) {
		msg.boost_level = BUILTIN_BOOST_LEVEL_THREE;
	} else if (optional_string("$Prefer this message highly")) {
		msg.boost_level = BUILTIN_BOOST_LEVEL_TWO;
	} else if (optional_string("$Prefer this message")) {
		msg.boost_level = BUILTIN_BOOST_LEVEL_ONE;
	} else {
		msg.boost_level = 0;
	}

	if (format == MessageFormat::TABLED) {
		if (!stricmp(msg.name, "Beta Arrived")) {
			handle_legacy_backup_message(msg, "Beta");
		} else if (!stricmp(msg.name, "Gamma Arrived")) {
			handle_legacy_backup_message(msg, "Gamma");
		} else if (!stricmp(msg.name, "Delta Arrived")) {
			handle_legacy_backup_message(msg, "Delta");
		} else if (!stricmp(msg.name, "Epsilon Arrived")) {
			handle_legacy_backup_message(msg, "Epsilon");
		} else if (get_builtin_message_type(msg.name) == MESSAGE_NONE) {
			Warning(LOCATION, "Unknown builtin message type %s in messages.tbl", msg.name);
		}
	}

	Num_messages++;
	Messages.push_back(msg); 
}

void message_frequency_parse()
{
	char name[32];
	int max_count, min_delay, occurrence_chance;

	required_string("$Name:");
	stuff_string(name, F_NAME, NAME_LENGTH);
	int builtin_type = get_builtin_message_type(name);
	if (builtin_type == MESSAGE_NONE) {
		Warning(LOCATION, "Unknown Builtin Message Type Detected. Type : %s not supported", name);
		return;
	}

	if (optional_string("+Occurrence Chance:")) {
		stuff_int(&occurrence_chance); 
		if ((occurrence_chance >= 0) && (occurrence_chance <= 100)) {
			Builtin_messages[builtin_type].occurrence_chance = occurrence_chance;
		}
	}

	if (optional_string("+Maximum Count:")) {
		stuff_int(&max_count); 
		if (max_count > -2) {
			Builtin_messages[builtin_type].max_count = max_count;
		}
	}

	if (optional_string("+Minimum Delay:")) {
		stuff_int(&min_delay); 
		if (min_delay > -1) {
			Builtin_messages[builtin_type].min_delay = min_delay;
		}
	}	
}

bool message_moods_check_existing(const SCP_string& mood)
{
	for (int i = 0; i < (int)Builtin_moods.size(); i++) {
		if (mood == Builtin_moods[i]) {
			return true;
		}
	}

	return false;
}

void message_moods_parse()
{	

	while (required_string_either("#End", "$Mood:")){
		SCP_string buf; 

		required_string("$Mood:");
		stuff_string(buf, F_NAME);

		if (!message_moods_check_existing(buf)) {
			Builtin_moods.push_back(buf);
		} else {
			mprintf(("Message mood %s already exists. Skipping!", buf.c_str()));
		}
	}

	required_string("#End");
}

int parse_existing_message_type() {
	char name[NAME_LENGTH];
	stuff_string(name, F_NAME, NAME_LENGTH);
	int type = get_builtin_message_type(name);
	if (type == MESSAGE_NONE && stricmp(name, "None")) {
		Warning(LOCATION, "Unknown message type %s", name);
	}
	return type;
}

int parse_message_priority() {
	// TODO: Convert this to required_string_one_of.
	if (optional_string("High")) {
		return MESSAGE_PRIORITY_HIGH;
	} else if (optional_string("Low")) {
		return MESSAGE_PRIORITY_LOW;
	} else {
		required_string("Normal");
		return MESSAGE_PRIORITY_NORMAL;
	}
}

void parse_custom_message_types(bool live = true) {
	if (optional_string("#Custom Message Types")) {
		while (optional_string("$Custom Message Type:")) {
			char name[NAME_LENGTH];
			stuff_string(name, F_NAME, NAME_LENGTH);
			required_string("+Fallback:");
			int fallback = parse_existing_message_type();
			required_string("+Priority:");
			int priority = parse_message_priority();
			if (live) {
				if (get_builtin_message_type(name) == MESSAGE_NONE) {
					Builtin_messages.emplace_back(vm_strdup(name), 100, -1, 0, priority, MESSAGE_TIME_SOON, fallback, true);
				} else {
					Warning(LOCATION, "Custom message type %s is already defined", name);
				}
			}
		}
		required_string("#End");
	}
}

void parse_custom_message_table(const char* filename) {
	read_file_text(filename, CF_TYPE_TABLES);
	reset_parse();
	parse_custom_message_types();
}

void message_types_init() {
	static bool table_read = false;
	if (!table_read) {
		table_read = true;
		parse_custom_message_table("messages.tbl");
		parse_modular_table("*-msg.tbm", parse_custom_message_table);
	}
}

void parse_msgtbl(const char* filename)
{
	try {
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// Goober5000 - ugh, ugly hack to fix the FS2 retail tables
		if (!Parsing_modular_table) {
			char* pVawacs25 = strstr(Mp, "Vawacs25.wav");
			if (pVawacs25) {
				char* pAwacs75 = strstr(pVawacs25, "Awacs75.wav");
				if (pAwacs75) {
					// move the 'V' from the first filename to the second, and adjust the 'A' case
					*pVawacs25 = 'A';
					for (int i = 1; i < (pAwacs75 - pVawacs25) - 1; i++)
						pVawacs25[i] = pVawacs25[i + 1];
					pAwacs75[-1] = 'V';
					pAwacs75[0] = 'a';
				}
			}
		}

		// now we can start parsing
		parse_custom_message_types(false); // Already parsed, so skip it
		if (optional_string("#Message Settings")) {
			if (optional_string("$Always loop head anis:")) {
				stuff_boolean(&Always_loop_head_anis);
			}
			if (optional_string("$Use newer head ani suffix features:")) {
				stuff_boolean(&Use_newer_head_ani_suffix);
			}
			if (optional_string("$Allow Any Ship To Send Backup Messages:")) {
				stuff_boolean(&Allow_generic_backup_messages);
			}
			if (optional_string("$Chance for Command to announce enemy arrival:")) {
				int scratch;
				stuff_int(&scratch);
				if (scratch < 0) {
					Warning(LOCATION, "$Chance for Command to announce enemy arrival: is negative; assuming 0");
					Command_announces_enemy_arrival_chance = 0;
				} else if (scratch > 100) {
					Warning(LOCATION, "$Chance for Command to announce enemy arrival: is over 100; assuming 100");
					Command_announces_enemy_arrival_chance = 1;
				} else {
					Command_announces_enemy_arrival_chance = static_cast<float>(scratch) / 100;
				}
			}
		}

		if (optional_string("#Message Frequencies")) {
			while (!required_string_one_of(3, "$Name:", "#Personas", "#Moods" )) {
				message_frequency_parse();
			}
		}

		Builtin_moods.push_back("Default");
		if (optional_string("#Moods")) {
			message_moods_parse();
		}

		if (optional_string("#Personas")) {
			while (required_string_one_of(3, "#Messages", "$Persona:", "#End")) {
				persona_parse();
			}
		}

		if (optional_string("#Messages")) {
			while (required_string_either("#End", "$Name:")) {
				message_parse(MessageFormat::TABLED);
			}
		}

		required_string("#End");

		if (optional_string("#Simulated Speech Overrides"))
		{
			char file[MAX_FILENAME_LEN];

			while (required_string_either("#End", "$File Name:"))
			{
				required_string("$File Name:");
				stuff_string(file, F_NAME, MAX_FILENAME_LEN);

				// get extension
				char* ptr = strchr(file, '.');
				if (ptr == NULL)
				{
					Warning(LOCATION, "Simulated speech override file '%s' was provided with no extension!", file);
					continue;
				}

				// test extension
				if (stricmp(ptr, ".ogg") != 0 && stricmp(ptr, ".wav") != 0)
				{
					Warning(LOCATION, "Simulated speech override file '%s' was provided with an extension other than .wav or .ogg!", file);
					continue;
				}

				// truncate extension
				*ptr = '\0';

				// add truncated file name
				Generic_message_filenames.push_back(file);
			}

			required_string("#End");
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

// this is called at the start of each level
void messages_init()
{
	int i;
	static int table_read = 0;

	if ( !table_read ) {
		message_types_init(); // To be safe, but in practice this should have been called already

		Default_command_persona = -1;
		Default_support_persona = -1;

		// speed things up a little by setting the capacities for the message vectors to roughly the FS2 amounts
		Messages.reserve(500);
		Message_waves.reserve(300);
		Message_avis.reserve(30);

		Num_messages = 0;

		// Add built-in generic filenames
		Generic_message_filenames.clear();
		Generic_message_filenames.push_back("none");
		Generic_message_filenames.push_back("cuevoice");
		Generic_message_filenames.push_back("cue_voice");
		Generic_message_filenames.push_back("emptymsg");
		Generic_message_filenames.push_back("generic");
		Generic_message_filenames.push_back("msgstart");

		// first parse the default table
		parse_msgtbl("messages.tbl");

		// parse any modular tables
		parse_modular_table("*-msg.tbm", parse_msgtbl);

		// save the number of builtin message things -- make initing between missions easier
		Num_builtin_messages = Num_messages;
		Num_builtin_avis = Num_message_avis;
		Num_builtin_waves = Num_message_waves;

		table_read = 1;
	}

	Current_mission_mood = 0;

	// reset the number of messages that we have for this mission
	Num_messages = Num_builtin_messages;
	Num_message_avis = Num_builtin_avis;
	Num_message_waves = Num_builtin_waves;
	Message_debug_index = Num_builtin_messages - 1;

	// initialize the stuff for the linked lists of messages
	MessageQ_num = 0;
	MessageQ.clear();
	
	// this forces a reload of the AVI's and waves for builtin messages.  Needed because the flic and
	// sound system also get reset between missions!
	for (i = 0; i < Num_builtin_avis; i++ ) {
		generic_anim_unload(&Message_avis[i].anim_data);
	}

	for (i = 0; i < Num_builtin_waves; i++ ){
		Message_waves[i].num = sound_load_id::invalid();
	}

	Message_shipnum = -1;
	Num_messages_playing = 0;
	for ( i = 0; i < MAX_PLAYING_MESSAGES; i++ ) {
		//Playing_messages[i].anim = NULL;
		Playing_messages[i].anim_data = NULL;
		Playing_messages[i].start_frame = -1;
		Playing_messages[i].play_anim = false;
		Playing_messages[i].wave         = sound_handle::invalid();
		Playing_messages[i].id = -1;
		Playing_messages[i].priority = -1;
		Playing_messages[i].shipnum = -1;
		Playing_messages[i].builtin_type = -1;
	}

	// reinitialize the personas.  mark them all as not used
	for (i = 0; i < (int)Personas.size(); i++) {
		Personas[i].flags &= ~PERSONA_FLAG_USED;
	}

	Message_wave_muted = 0;
	Next_mute_time = 1;

	//wipe all the non-builtin messages
	Messages.erase((Messages.begin()+Num_builtin_messages), Messages.end()); 
	Message_avis.erase((Message_avis.begin()+Num_builtin_avis), Message_avis.end()); 
	Message_waves.erase((Message_waves.begin()+Num_builtin_waves), Message_waves.end());
}

// free a loaded avi
void message_mission_free_avi(int m_index)
{
	// check for bogus index
	if ( (m_index < 0) || (m_index >= Num_message_avis) )
		return;

	generic_anim_unload(&Message_avis[m_index].anim_data);
}

// called to do cleanup when leaving a mission
void message_mission_shutdown()
{
	int i;

	mprintf(("Unloading in mission messages\n"));

	training_mission_shutdown();

	// kill/stop all playing messages sounds and animations if we need to
	message_kill_all(true);

	// remove the wave sounds from memory
	for (i = 0; i < Num_message_waves; i++ ) {
		if (Message_waves[i].num.isValid()) {
			snd_unload( Message_waves[i].num );
		}
	}

	fsspeech_stop();

	// free up remaining anim data - taylor
	for (i=0; i<Num_message_avis; i++) {
		message_mission_free_avi(i);
	}

	// Goober5000 - free up special messages (done automatically via unique_ptr)
	MessageQ.clear();
}

// functions to deal with queuing messages to the message system.

//	Compare function for sorting message queue entries based on priority.
//	Return values set to sort array in _decreasing_ order.  If priorities equal, sort based
// on time added into queue
int message_queue_priority_compare(const message_q *ma, const message_q *mb)
{
	if (ma->priority > mb->priority) {
		return -1;
	} else if (ma->priority < mb->priority) {
		return 1;
	} else if (ma->time_added < mb->time_added) {
		return -1;
	} else if (ma->time_added > mb->time_added) {
		return 1;
	} else {
		return 0;
	}
}

//	Pauses all currently playing messages in the message queue
void message_pause_all()
{
	for (int i = 0; i < Num_messages_playing; i++) {
		if ((Playing_messages[i].wave.isValid()) && snd_is_playing(Playing_messages[i].wave)) {
			snd_pause(Playing_messages[i].wave);
		}
	}
}

//	Resumes playing any paused messages in the message queue
void message_resume_all()
{
	for (int i = 0; i < Num_messages_playing; i++) {
		if ((Playing_messages[i].wave.isValid()) && snd_is_paused(Playing_messages[i].wave)) {
			snd_resume(Playing_messages[i].wave);
		}
	}
}

// function to kill all currently playing messages.  kill_all parameter tells us to
// kill only the animations that are playing, or wave files too
void message_kill_all( bool kill_all )
{
	if (Num_messages_playing <= 0) {
		return;
	}

	// kill sounds for all voices currently playing
	for (int i = 0; i < Num_messages_playing; i++ ) {
		/*if ( (Playing_messages[i].anim != NULL) && anim_playing(Playing_messages[i].anim) ) {
			anim_stop_playing( Playing_messages[i].anim );
			Playing_messages[i].anim=NULL;
		}*/
		if ( Playing_messages[i].play_anim) {
			Playing_messages[i].play_anim = false;
		}

		if ( kill_all ) {
			if ((Playing_messages[i].wave.isValid()) && snd_is_playing(Playing_messages[i].wave)) {
				snd_stop( Playing_messages[i].wave );
			}

			Playing_messages[i].shipnum = -1;
		}
	}

	if ( kill_all ) {
		Num_messages_playing = 0;
		fsspeech_stop();
	}
}

// function to kill nth playing message
void message_kill_playing( int message_num )
{
	Assert( message_num < Num_messages_playing );

	/*if ( (Playing_messages[message_num].anim != NULL) && anim_playing(Playing_messages[message_num].anim) ) {
		anim_stop_playing( Playing_messages[message_num].anim );
		Playing_messages[message_num].anim=NULL;
	}*/
	if ( Playing_messages[message_num].play_anim) {
		Playing_messages[message_num].play_anim = false;
	}

	if ((Playing_messages[message_num].wave.isValid()) && snd_is_playing(Playing_messages[message_num].wave))
		snd_stop( Playing_messages[message_num].wave );

	Playing_messages[message_num].shipnum = -1;

	fsspeech_stop();
}


// returns true if all messages currently playing are builtin messages
int message_playing_builtin()
{
	int i;

	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( Playing_messages[i].id >= Num_builtin_messages ){
			break;
		}
	}

	// if we got through the list without breaking, all playing messages are builtin messages
	if ( i == Num_messages_playing ){
		return 1;
	} else {
		return 0;
	}
}

// returns true in any playing message is of the specific builtin type
int message_playing_specific_builtin( int builtin_type )
{
	int i;

	for (i = 0; i < Num_messages_playing; i++ ) {
		if ( (Playing_messages[i].id < Num_builtin_messages) && (Playing_messages[i].builtin_type == builtin_type) ){
			return 1;
		}
	}

	return 0;
}

// returns true if all messages current playing are unique messages
int message_playing_unique()
{
	int i;

	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( Playing_messages[i].id < Num_builtin_messages ){
			break;
		}
	}

	// if we got through the list without breaking, all playing messages are builtin messages
	if ( i == Num_messages_playing ){
		return 1;
	} else {
		return 0;
	}
}


// returns the highest priority of the currently playing messages
#define MESSAGE_GET_HIGHEST		1
#define MESSAGE_GET_LOWEST		2
int message_get_priority(int which)
{
	int priority;

	if ( which == MESSAGE_GET_HIGHEST ){
		priority = MESSAGE_PRIORITY_LOW;
	} else {
		priority = MESSAGE_PRIORITY_HIGH;
	}

	for (int i = 0; i < Num_messages_playing; i++ ) {
		if ( (which == MESSAGE_GET_HIGHEST) && (Playing_messages[i].priority > priority) ){
			priority = Playing_messages[i].priority;
		} else if ( (which == MESSAGE_GET_LOWEST) && (Playing_messages[i].priority < priority) ){
			priority = Playing_messages[i].priority;
		}
	}

	return priority;
}


// removes current message from the queue
void message_remove_from_queue(message_q *q)
{	
	// quick out if nothing to do.
	if ( MessageQ_num <= 0 ) {
		return;
	}	

	q->priority = -1;
	q->time_added = -1;
	q->message_num = -1;
	q->builtin_type = -1;
	q->min_delay_stamp = TIMESTAMP::invalid();
	q->group = 0;

	// Goober5000
	q->special_message.reset();
	q->event_num_to_cancel = -1;

	if ( MessageQ_num > 1 ) {
		insertion_sort(MessageQ, MessageQ_num, message_queue_priority_compare);
	}

	MessageQ_num--;
}

// Load in the sound data for a message.
//
// index - index into the Message_waves[] array
//
void message_load_wave(int index, const char *filename)
{
	Assertion(index >= 0, "Invalid index passed!");

	if (Message_waves[index].num.isValid()) {
		return;
	}

	if ( !Sound_enabled ) {
		Message_waves[index].num = sound_load_id::invalid();
		return;
	}

	game_snd_entry tmp_gse;
	strcpy_s(tmp_gse.filename, filename);
	Message_waves[index].num = snd_load(&tmp_gse, nullptr, 0);

	if (!Message_waves[index].num.isValid())
		nprintf(("messaging", "Cannot load message wave: %s.  Will not play\n", Message_waves[index].name));
}

// Goober5000
bool message_filename_is_generic(const char *filename)
{
	char truncated_filename[MAX_FILENAME_LEN];

	// truncate any file extension
	strcpy_s(truncated_filename, filename);
	char *ptr = strchr(truncated_filename, '.');

	// extension must be a recognized sound file
	if ((ptr == NULL) || (stricmp(ptr, ".ogg") != 0 && stricmp(ptr, ".wav") != 0))
		return false;

	// truncate it
	*ptr = '\0';

	// test against the list
	for (unsigned int i = 0; i < Generic_message_filenames.size(); i++)
	{
		if (!stricmp(Generic_message_filenames[i].c_str(), truncated_filename))
			return true;
	}

	return false;
}

bool message_filename_has_fs1_wingman_prefix(const char *filename)
{
	Assert(filename != nullptr);
	return (filename[0] >= '1' && filename[0] <= '6'
		&& filename[1] == '_'
		&& filename[2] != '\0');
}

void message_filename_convert_to_command(char *buf, const char *filename)
{
	// prepend the command name, and then the rest of the filename.
	strcpy(buf, COMMAND_WAVE_PREFIX);
	strcat(buf, &filename[2]);
}

// Play wave file associated with message
// input: m		=>		pointer to message description
//
// note: changes Message_wave_duration, Playing_messages[].wave, and Message_waves[].num
bool message_play_wave( message_q *q )
{
	int index;
	MissionMessage *m;
	char filename[MAX_FILENAME_LEN];

	m = &Messages[q->message_num];

	if ( m->wave_info.index >= 0 ) {
		index = m->wave_info.index;
		strcpy_s( filename, Message_waves[index].name );

		// Goober5000 - if we're using simulated speech, it should pre-empt the generic beeps
		if (fsspeech_play_from(FSSPEECH_FROM_INGAME) && message_filename_is_generic(filename))
			return false;

		// if we need to bash the wave name because of "conversion" to terran command, do it here
		// Look for "[1-6]_" at the front of the message.  If found, then convert to TC_*
		if ( q->flags & MQF_CONVERT_TO_COMMAND && message_filename_has_fs1_wingman_prefix(filename) ) {
			char temp[MAX_FILENAME_LEN];
			message_filename_convert_to_command(temp, filename);
			strcpy_s(filename, temp);

			Message_waves[index].num = sound_load_id::invalid(); // forces us to reload the message
		}

		// load the sound file into memory
		message_load_wave(index, filename);
		if (!Message_waves[index].num.isValid()) {
			m->wave_info.index = -1;
		}

		if ( m->wave_info.index >= 0 ) {
			// this call relies on the fact that snd_play returns -1 if the sound cannot be played
			Message_wave_duration = snd_get_duration(Message_waves[index].num);
			Playing_messages[Num_messages_playing].wave = snd_play_raw( Message_waves[index].num, 0.0f );

			return Playing_messages[Num_messages_playing].wave.isValid();
		}
	}

	return false;
}

// Determine the starting frame for the animation
// input:	time	=>		time of voice clip, in ms
//				ani	=>		pointer to anim data
//				reverse	=>	flag to indicate that the start should be time ms from the end (used for death screams)
void message_calc_anim_start_frame(int time, generic_anim *ani, int reverse)
{
	float	wave_time, anim_time;
	int	start_frame;

	start_frame=0;

	// If no voice clip exists, start from beginning of anim
	if ( time <= 0 ) {
		return;
	}

	// convert time to seconds
	wave_time = time/1000.0f;
	anim_time = ani->total_time;

	// If voice clip is longer than anim, start from beginning of anim
	if ( wave_time >= (anim_time) ) {
		return;
	}

	float fps = ani->num_frames / ani->total_time;
	if ( reverse ) {
		start_frame = (ani->num_frames-1) - (int)std::lround(fps * wave_time);
	} else {
		int num_frames_extra;
		num_frames_extra = (int)std::lround(fps * (anim_time - wave_time));
		if ( num_frames_extra > 0 ) {
			start_frame= Random::next(num_frames_extra);
		}
	}

	if ( start_frame < 0 ) {
		mprintf(("Calculated start frame for animation %s was less than 0, setting to 0.\n", ani->filename));
		start_frame=0;
	}

	ani->current_frame = start_frame;
	ani->anim_time = start_frame / fps;
}

// Play animation associated with message
// input:	m		=>		pointer to message description
//				q		=>		message queue data
//
// note: changes Messave_wave_duration, Playing_messages[].wave, and Message_waves[].num
void message_play_anim( message_q *q )
{
	message_extra	*anim_info;
	int				is_death_scream=0, persona_index=-1, rand_index=0;
	char				ani_name[MAX_FILENAME_LEN], temp[MAX_FILENAME_LEN], *p;
	MissionMessage	*m;

	// don't even bother with this stuff if the gauge is disabled - taylor
	if ( !hud_gauge_active(HUD_TALKING_HEAD) ) {
		return;
	}

	m = &Messages[q->message_num];

	// check to see if the avi_index is valid -- try and load/play the avi if so.
	if ( m->avi_info.index < 0 ) {
		return;
	}

	anim_info = &Message_avis[m->avi_info.index];

	// get the filename.  Strip off the extension since we won't need it anyway
	strcpy_s(ani_name, anim_info->name);
	p = strchr(ani_name, '.');			// gets us to the extension
	if ( p ) {
		*p = '\0';
	}

	// builtin messages are given a base ani which we should add a suffix on before trying
	// to load the animation.  See if this message is a builtin message which has a persona
	// attached to it.  Deal with munging the name

	// support ships use a wingman head.
	// terran command uses its own set of heads.
	if ( (!anim_info->exists) &&	// if the base animation doesn't exist, then a, b, or c needs to be appended
		((q->message_num < Num_builtin_messages) || !(strnicmp(HEAD_PREFIX_STRING, ani_name, strlen(HEAD_PREFIX_STRING)-1))) ) {
		int subhead_selected = FALSE;
		persona_index = m->persona_index;
		
		// if this ani should be converted to a terran command, set the persona to the command persona
		// so the correct head plays.
		if ( q->flags & MQF_CONVERT_TO_COMMAND ) {
			persona_index = The_mission.command_persona;

			// if this is a FS1 message, swap the head
			if (m->wave_info.index >= 0) {
				if (message_filename_has_fs1_wingman_prefix(Message_waves[m->wave_info.index].name)) {
					strcpy_s(ani_name, COMMAND_HEAD_PREFIX);
				}
			}
		}

		// Goober5000 - guard against negative array indexing; this way, if no persona was
		// assigned, the logic will drop down below like it's supposed to
		if (persona_index >= 0)
		{
			if (!Use_newer_head_ani_suffix) {
				if (Personas[persona_index].flags & (PERSONA_FLAG_WINGMAN | PERSONA_FLAG_SUPPORT)) {
					// get a random head
					if (q->builtin_type == MESSAGE_WINGMAN_SCREAM) {
						rand_index = MAX_WINGMAN_HEADS; // [0,MAX) are regular heads; MAX is always death head
						is_death_scream = 1;
					} else {
						rand_index = ((int)Missiontime % MAX_WINGMAN_HEADS);
					}
					strcpy_s(temp, ani_name);
					sprintf_safe(ani_name, "%s%c", temp, 'a' + rand_index);
					subhead_selected = TRUE;
				} else if (Personas[persona_index].flags & (PERSONA_FLAG_COMMAND | PERSONA_FLAG_LARGE)) {
					// get a random head
					// Goober5000 - *sigh*... if mission designers assign a command persona
					// to a wingman head, they risk having the death ani play
					if (!strnicmp(ani_name, "Head-TP", 7) || !strnicmp(ani_name, "Head-VP", 7)) {
						mprintf(("message '%s' incorrectly assigns a command/largeship persona to a wingman animation!\n", m->name));
						rand_index = ((int)Missiontime % MAX_WINGMAN_HEADS);
					} else {
						rand_index = ((int)Missiontime % MAX_COMMAND_HEADS);
					}

					strcpy_s(temp, ani_name);
					sprintf_safe(ani_name, "%s%c", temp, 'a' + rand_index);
					subhead_selected = TRUE;
				} else {
					mprintf(("message '%s' uses an unrecognized persona type\n", m->name));
				}
			} else {
				// Explicitely allow death anims for large ships now. Only command can't have a death message.
				if (!(Personas[persona_index].flags & PERSONA_FLAG_COMMAND) && q->builtin_type == MESSAGE_WINGMAN_SCREAM) {
					strcpy_s(temp, ani_name);
					sprintf_safe(ani_name, "%s-death", temp);
					subhead_selected = TRUE;
				} else {
					strcpy_s(temp, ani_name);
					sprintf_safe(ani_name, "%s-reg", temp);
					subhead_selected = TRUE;
				}
			}
		} else {
			// In suffix mode if we don't have a persona AND the anim doesn't exist then append -reg
			if (Use_newer_head_ani_suffix) {
				strcpy_s(temp, ani_name);
				sprintf_safe(ani_name, "%s-reg", temp);
				subhead_selected = TRUE;
			}
		}

		if (!subhead_selected) {
			if (!Use_newer_head_ani_suffix) {
				// choose between a and b
				rand_index = ((int)Missiontime % MAX_WINGMAN_HEADS);
				strcpy_s(temp, ani_name);
				sprintf_safe(ani_name, "%s%c", temp, 'a' + rand_index);
			}
			mprintf(("message '%s' with invalid head.  Fix by assigning persona to the message.\n", m->name));
		}
		nprintf(("Messaging", "playing head %s for %s\n", ani_name, q->who_from));
	}

	// check to see if the avi has been loaded.  If not, then load the AVI.  On an error loading
	// the avi, set the top level index to -1 to avoid multiple tries at loading the flick.

	// if there is something already here that's not this same file then go ahead a let go of it - taylor
	if ( !strstr(anim_info->anim_data.filename, ani_name) ) {
		nprintf(("Messaging", "clearing headani data due to name mismatch: (%s) (%s)\n",
					anim_info->anim_data.filename, ani_name));
		message_mission_free_avi( m->avi_info.index );
	}

	strcpy_s( anim_info->anim_data.filename, ani_name );
	if(!Full_color_head_anis)
			anim_info->anim_data.use_hud_color = true;

	if ( generic_anim_stream(&anim_info->anim_data, false) < 0 ) {
		nprintf (("messaging", "Cannot load message avi %s.  Will not play.\n", ani_name));
		m->avi_info.index = -1;			// if cannot load the avi -- set this index to -1 to avoid trying to load multiple times
	}

	if ( m->avi_info.index >= 0 ) {
		// This call relies on the fact that AVI_play will return -1 if the AVI cannot be played
		// if any messages are already playing, kill off any head anims that are currently playing.  We will
		// only play a head anim of the newest messages being played
		nprintf(("messaging", "killing off any currently playing head animations\n"));
		message_kill_all(false);

		if ( hud_disabled() ) {
			return;
		}
		
		if (!Always_loop_head_anis) {
			anim_info->anim_data.direction = GENERIC_ANIM_DIRECTION_NOLOOP;
		}
		Playing_messages[Num_messages_playing].anim_data = &anim_info->anim_data;
		message_calc_anim_start_frame(Message_wave_duration, &anim_info->anim_data, is_death_scream);
		Playing_messages[Num_messages_playing].play_anim = true;
	}
}

/** 
 * process the message queue -- called once a frame
 */
void message_queue_process()
{	
	SCP_string buf;
	SCP_string who_from;
	message_q *q;
	int i;
	MissionMessage *m;
	bool builtinMessage = false; // gcc doesn't like var decls crossed by goto's
	object* sender = NULL;

	// Don't play messages until first frame has been rendered
	if ( Framecount < 2 ) {
		return;
	}

	// determine if all playing messages (if any) are done playing.  If any are done, remove their
	// entries collapsing the Playing_messages array if necessary
	if ( Num_messages_playing > 0 ) {

		// for each message playing, determine if it is done.
		i = 0;
		while ( i < Num_messages_playing ) {
			int ani_done, wave_done, j;

			ani_done = 1;
			if ( Playing_messages[i].play_anim )
				ani_done = 0;

			wave_done = 1;

//			if ( (Playing_messages[i].wave != -1) && snd_is_playing(Playing_messages[i].wave) )
			if ((Playing_messages[i].wave.isValid()) && (snd_time_remaining(Playing_messages[i].wave) > 250))
				wave_done = 0;

			// Don't kill paused messages
			if ((Playing_messages[i].wave.isValid()) && snd_is_paused(Playing_messages[i].wave)) {
				wave_done = 0;
			}

			// Goober5000
			if (fsspeech_playing())
				wave_done = 0;

			// AL 1-20-98: If voice message is done, kill the animation early
			if ((Playing_messages[i].wave.isValid()) && wave_done) {
				/*if ( !ani_done ) {
					anim_stop_playing( Playing_messages[i].anim );
				}*/
				Playing_messages[i].play_anim = false;
			}

			//if player is a traitor remove all messages that aren't traitor related
			if ((Playing_messages[i].builtin_type != MESSAGE_OOPS) && (Playing_messages[i].builtin_type != MESSAGE_HAMMER_SWINE)) {
				if ( (Player_ship->team == Iff_traitor) && ( !(Game_mode & GM_MULTIPLAYER) || !(Netgame.type_flags & NG_TYPE_DOGFIGHT) ) ) {
					message_kill_playing(i);
					Message_shipnum = -1;
					i++;
					continue;
				}
			}

			// see if the ship sending this message is dying.  If do, kill wave and anim
			if ( Playing_messages[i].shipnum != -1 ) {
				if ( (Ships[Playing_messages[i].shipnum].flags[Ship::Ship_Flags::Dying]) && (Playing_messages[i].builtin_type != MESSAGE_WINGMAN_SCREAM) ) {
					int shipnum;

					shipnum = Playing_messages[i].shipnum;
					message_kill_playing( i );
					// force this guy to scream
					// AL 22-2-98: Ensure don't use -1 to index into ships array.  Mark, something is incorrect 
					//             here, since message_kill_playing() seems to always set Playing_messages[i].shipnum to -1
					// MWA 3/24/98 -- save shipnum before killing message
					// 
					Assert( shipnum >= 0 );
					if ( !(Ships[shipnum].flags[Ship::Ship_Flags::Ship_has_screamed]) && !(Ships[shipnum].flags[Ship::Ship_Flags::No_death_scream]) ) {
						ship_scream( &Ships[shipnum] );
					}
					continue;							// this should keep us in the while() loop with same value of i.														
				}											// we should enter the next 'if' statement during next pass
			}

			// if both ani and wave are done, mark internal variable so we can do next message on queue, and
			// global variable to clear voice brackets on hud
			if (wave_done && ani_done &&
			    (timestamp_elapsed(Message_expire) || (Playing_messages[i].wave.isValid()) ||
			     (Playing_messages[i].shipnum == -1))) {
				nprintf(("messaging", "Message %d is done playing\n", i));
				Message_shipnum = -1;
				Num_messages_playing--;
				if ( Num_messages_playing == 0 )
					break;

				// there is still another message playing.  Collapse the playing_message array
				nprintf(("messaging", "Collapsing playing message stack\n"));
				for ( j = i+1; j < Num_messages_playing + 1; j++ ) {
					Playing_messages[j-1] = Playing_messages[j];
				}
			} else {
				// messages is not done playing -- move to next message
				i++;
			}
		}
	}

	// preprocess message queue and remove anything on the queue that is too old.  If next message on
	// the queue can be played, then break out of the loop.  Otherwise, loop until nothing on the queue
	while ( MessageQ_num > 0 ) {
		q = &MessageQ[0];
		// message is outside its time window
		if ( q->window_timestamp.isValid() && timestamp_elapsed(q->window_timestamp) && !q->group) {
			// remove message from queue and see if more to remove
			nprintf(("messaging", "Message %s didn't play because it didn't fit into time window.\n", Messages[q->message_num].name));
			if ( q->message_num < Num_builtin_messages ){			// we should only ever remove builtin messages this way
				message_remove_from_queue(q);
			} else {
				break;
			}
		}
		// message can't be played after a certain event
		else if (q->event_num_to_cancel >= 0 && Mission_events[q->event_num_to_cancel].result) {
			message_remove_from_queue(q);
		}
		// message isn't too old
		else {
			break;
		}
	}

	// no need to process anything if there isn't anything on the queue
	if ( MessageQ_num <= 0 ){
		return;
	}

	// get a pointer to an item on the queue
	int found = -1;
	int idx = 0;
	while((found == -1) && (idx < MessageQ_num)){
		// if this guy has no min delay timestamp, or it has expired, select him
		if (!MessageQ[idx].min_delay_stamp.isValid() || timestamp_elapsed(MessageQ[idx].min_delay_stamp)) {
			found = idx;
			break;
		}

		// next
		idx++;
	}

	// if we didn't find anything, bail
	if(found == -1){
		return;
	}
	// if this is not the first item on the queue, make it the first item
	if(found != 0){
		// store the entry
		message_q temp = std::move(MessageQ[found]);

		// move all other entries up
		for(idx=found; idx>0; idx--){
			MessageQ[idx] = std::move(MessageQ[idx-1]);
		}

		// plop the entry down as being first
		MessageQ[0] = std::move(temp);
	}

	q = &MessageQ[0];
	Assert ( q->message_num != -1 );
	Assert ( q->priority != -1 );
	Assert ( q->time_added != -1 );

	int provisional_message_shipnum = -1;

	// Do some checks to see if we are actually going to play this message at all.  These checks have been moved above the
	// "if ( Num_messages_playing )" block because we don't want to cancel an existing message if we have nothing to replace it with.

	// Goober5000 - argh, don't conflate special sources with ships!
	// NOTA BENE: don't check for != HUD_SOURCE_TERRAN_CMD, because with the new command persona code, Command could be a ship
	if ( q->source != HUD_SOURCE_IMPORTANT ) {
		provisional_message_shipnum = ship_name_lookup( q->who_from );

		// see if we need to check if sending ship is alive
		if ( (provisional_message_shipnum < 0) && (q->flags & MQF_CHECK_ALIVE) ) {
			goto all_done;
		}
	}

	// AL: added 07/14/97.. only play avi/sound if in gameplay
	if ( gameseq_get_state() != GS_STATE_GAME_PLAY )
		goto all_done;

	// AL 4-7-98: Can't receive messages if comm is destroyed
	if ( hud_communications_state(Player_ship) == COMM_DESTROYED ) {
		goto all_done;
	}
	// G5K 4-26-20: Can't send messages if comm is destroyed
	if ( The_mission.ai_profile->flags[AI::Profile_Flags::Check_comms_for_non_player_ships] && (provisional_message_shipnum >= 0) && hud_communications_state(&Ships[provisional_message_shipnum], (q->builtin_type == MESSAGE_WINGMAN_SCREAM)) == COMM_DESTROYED ) {
		goto all_done;
	}

	//	Don't play death scream unless a small ship.
	if ( q->builtin_type == MESSAGE_WINGMAN_SCREAM ) {
		if (provisional_message_shipnum < 0) {
			goto all_done;
		}
		if (!(Ship_info[Ships[provisional_message_shipnum].ship_info_index].is_small_ship() || (Ships[provisional_message_shipnum].flags[Ship::Ship_Flags::Always_death_scream])) ) {
			goto all_done;
		}
	}

	// At this point we think we're going to play the queued message.

	if ( Num_messages_playing ) {
		// peek at the first message on the queue to see if it should interrupt, or overlap a currently
		// playing message.  Mission specific messages will always interrupt builtin messages.  They
		// will never interrupt other mission specific messages.
		//
		//  Builtin message might interrupt other builtin messages, or overlap them, all depending on
		// message priority.

		if ( q->builtin_type == MESSAGE_HAMMER_SWINE ) {
			message_kill_all(true);
		} else if ( message_playing_specific_builtin(MESSAGE_HAMMER_SWINE) ) {
			MessageQ_num = 0;
			return;
		} else if ( message_playing_builtin() && ( q->message_num >= Num_builtin_messages) && (q->priority > MESSAGE_PRIORITY_LOW) ) {
			// builtin is playing and we have a unique message to play.  Kill currently playing message
			// so unique can play uninterrupted.  Only unique messages higher than low priority will interrupt
			// other messages.
			message_kill_all(true);
			nprintf(("messaging", "Killing all currently playing messages to play unique message\n"));
		} else if ( message_playing_builtin() && (q->message_num < Num_builtin_messages) ) {
			// when a builtin message is queued, we might either overlap or interrupt the currently
			// playing message.
			//
			// we have to check for num_messages_playing (again), since code for death scream might
			// kill all messages.
			if ( Num_messages_playing ) {
				if ( message_get_priority(MESSAGE_GET_HIGHEST) < q->priority ) {
					// lower priority message playing -- kill it.
					message_kill_all(true);
					nprintf(("messaging", "Killing all currently playing messages to play high priority builtin\n"));
				} else if ( message_get_priority(MESSAGE_GET_LOWEST) > q->priority ) {
					// queued message is a lower priority, so wait it out
					return;
				} else {
					// if we get here, then queued messages is a builtin message with the same priority
					// as the currently playing messages.  This state will cause messages to overlap.
					nprintf(("messaging", "playing builtin message (overlap) because priorities match\n"));
				}
			}
		} else if ( message_playing_unique() && (q->message_num < Num_builtin_messages) ) {
			// code messages can kill any low priority mission specific messages
			if ( Num_messages_playing ) {
				if ( message_get_priority(MESSAGE_GET_HIGHEST) == MESSAGE_PRIORITY_LOW ) {
					message_kill_all(true);
					nprintf(("messaging", "Killing low priority unique messages to play code message\n"));
				} else {
					return;			// do nothing.
				}
			}
		} else {
			return;
		}
	}

	// if we are playing the maximum number of voices, then return.  Make the check here since the above
	// code might kill off currently playing messages
	if ( Num_messages_playing == MAX_PLAYING_MESSAGES )
		return;

	// if this is a ship, then don't play anything if this ship is already talking
	if ( provisional_message_shipnum != -1 ) {
		for ( i = 0; i < Num_messages_playing; i++ ) {
			if ( (Playing_messages[i].shipnum != -1) && (Playing_messages[i].shipnum == provisional_message_shipnum) ){
				return;
			}
		}
	}

	// At this point we are definitely going to play the queued message.

	Message_shipnum = provisional_message_shipnum;

	// set up module globals for this message
	m = &Messages[q->message_num];
	Playing_messages[Num_messages_playing].anim_data = NULL;
	Playing_messages[Num_messages_playing].wave = sound_handle::invalid();
	Playing_messages[Num_messages_playing].id  = q->message_num;
	Playing_messages[Num_messages_playing].priority = q->priority;
	Playing_messages[Num_messages_playing].shipnum = Message_shipnum;
	Playing_messages[Num_messages_playing].builtin_type = q->builtin_type;

	Message_wave_duration = 0;

	// translate tokens in message to the real things
	buf = message_translate_tokens(q->special_message ? q->special_message.get() : m->message);

	Message_expire = _timestamp(static_cast<int>(42 * buf.size()));

	// play wave first, since need to know duration for picking anim start frame
	if(message_play_wave(q) == false) {
		fsspeech_play(FSSPEECH_FROM_INGAME, buf.c_str());
	}

	// play animation for head
	message_play_anim(q);
	
	// distort the message if comms system is damaged
	message_maybe_distort_text(buf, Message_shipnum, (q->builtin_type == MESSAGE_WINGMAN_SCREAM));

#ifndef NDEBUG
	// debug only -- if the message is a builtin message, put in parens whether or not the voice played
	if (Sound_enabled && !Playing_messages[Num_messages_playing].wave.isValid()) {
		buf += NOX("..(no wavefile for voice)");
		snd_play(gamesnd_get_game_sound(GameSounds::CUE_VOICE));
	}
#endif
	
	// if this is a ship, do we use name or callsign or ship class?
	if ( Message_shipnum >= 0 ) {
		ship *shipp = &Ships[Message_shipnum];
		if ( shipp->callsign_index >= 0 ) {
			who_from = hud_get_ship_callsign( shipp );
		} else if ( ((Iff_info[shipp->team].flags & IFFF_WING_NAME_HIDDEN) && (shipp->wingnum != -1)) || (shipp->flags[Ship::Ship_Flags::Hide_ship_name]) ) {
			who_from = hud_get_ship_class( shipp );
		} else {
			who_from = shipp->get_display_name();
		}
	} else {
		who_from = q->who_from;
	}

	if ( !stricmp(who_from.c_str(), "<none>") ) {
		HUD_sourced_printf( q->source, NOX("%s"), buf.c_str() );
	} else {
		HUD_sourced_printf( q->source, NOX("%s: %s"), who_from.c_str(), buf.c_str() );
	}

	if ( Message_shipnum >= 0 ) {
		hud_target_last_transmit_add(Message_shipnum);
	}

	builtinMessage = q->builtin_type != -1;

	if (Message_shipnum >= 0) {
		sender = &Objects[Ships[Message_shipnum].objnum];
	}
	if (OnMessageReceivedHook->isActive()) {
		OnMessageReceivedHook->run(scripting::hook_param_list(
			scripting::hook_param("Name", 's', m->name),
			scripting::hook_param("MessageHandle", 'o', scripting::api::l_Message.Set(q->message_num)),
			scripting::hook_param("Message", 's', buf),
			scripting::hook_param("SenderString", 's', who_from),
			scripting::hook_param("Builtin", 'b', builtinMessage),
			scripting::hook_param("Sender", 'o', sender)));
	}

	Num_messages_playing++;		// this has to be done at the end because some sound functions use it to index into the array
all_done:
	message_remove_from_queue(q);
}

// queues up a message to display to the player
void message_queue_message( int message_num, int priority, int timing, const char *who_from, int source, int group, int delay, int builtin_type, int event_num_to_cancel )
{
	int i, m_persona;
	char temp_buf[MESSAGE_LENGTH];

	if ( message_num < 0 ) return;

	// some messages can get queued quickly.  Try to filter out certain types of messages before
	// they get queued if there are other messages of the same type already queued
	if ( (builtin_type == MESSAGE_ALREADY_ON_WAY) || (builtin_type == MESSAGE_OOPS) ) {
		// if it is already playing, then don't play it
		if ( message_playing_specific_builtin(builtin_type) ) 
			return;

		for ( i = 0; i < MessageQ_num; i++ ) {
			// if one of these messages is already queued, then don't play
			if ( (MessageQ[i].message_num == message_num) && (MessageQ[i].builtin_type == builtin_type) )
				return;

		}
	}

	// check to be sure that we have room to queue this message
	if ( MessageQ_num == (int)MessageQ.size() ) {
		MessageQ.emplace_back();
	}

	// if player is a traitor, no messages for him!!!
	// unless those messages are traitor related
	// Goober5000 - allow messages during multiplayer dogfight (Mantis #1436)
	if ( (Player_ship->team == Iff_traitor) && ( !(Game_mode & GM_MULTIPLAYER) || !(Netgame.type_flags & NG_TYPE_DOGFIGHT) ) ) {
		if ((builtin_type != MESSAGE_OOPS) && (builtin_type != MESSAGE_HAMMER_SWINE)) {
			return;
		}
	}

	m_persona = Messages[message_num].persona_index;

	// put the message into a slot
	i = MessageQ_num++;
	MessageQ[i].time_added = Missiontime;
	MessageQ[i].priority = priority;
	MessageQ[i].message_num = message_num;
	MessageQ[i].source = source;
	MessageQ[i].builtin_type = builtin_type;
	MessageQ[i].min_delay_stamp = _timestamp(delay);
	MessageQ[i].group = group;
	strcpy_s(MessageQ[i].who_from, who_from);
	MessageQ[i].special_message.reset();
	MessageQ[i].event_num_to_cancel = event_num_to_cancel;

	// Goober5000 - replace variables if necessary
	// karajorma/jg18 - replace container references if necessary
	strcpy_s(temp_buf, Messages[message_num].message);
	const bool replace_var = sexp_replace_variable_names_with_values(temp_buf, MESSAGE_LENGTH - 1);
	const bool replace_con = sexp_container_replace_refs_with_values(temp_buf, MESSAGE_LENGTH - 1);
	if (replace_var || replace_con)
		MessageQ[i].special_message.reset(vm_strdup(temp_buf));

	MessageQ[i].flags = 0;

	// wingman personas have their alive status checked
	if ( (m_persona != -1) && (Personas[m_persona].flags & PERSONA_FLAG_WINGMAN) ) {
		bool convert_to_command = false;

		// SPECIAL HACK -- if the who_from is terran command, and there is a wingman persona attached
		// to this message, then set a bit to tell the wave/anim playing code to play the command version
		// of the wave and head
		// ADDENDUM -- Since the special hack is specifically for mission-unique messages, don't
		// convert built-in messages to Command
		if ( builtin_type < 0
			&& (!stricmp(who_from, The_mission.command_sender)
				|| (The_mission.flags[Mission::Mission_Flags::Override_hashcommand] && !stricmp(who_from, DEFAULT_COMMAND))
				) ) {
			// ADDENDUM 2 -- perform an additional check: only convert this message if a WAV exists for it
			auto m = &Messages[message_num];
			if (m->wave_info.index >= 0) {
				auto filename = Message_waves[m->wave_info.index].name;
				if (message_filename_has_fs1_wingman_prefix(filename)) {
					char converted[MAX_FILENAME_LEN];
					message_filename_convert_to_command(converted, filename);
					if (cf_exists_full_ext(converted, CF_TYPE_VOICE_SPECIAL, NUM_AUDIO_EXT, audio_ext_list)) {
						convert_to_command = true;
					}
				}
			}
		}

		if (convert_to_command) {
			MessageQ[i].flags |= MQF_CONVERT_TO_COMMAND;
			MessageQ[i].source = HUD_SOURCE_TERRAN_CMD;
		} else {
			MessageQ[i].flags |= MQF_CHECK_ALIVE;
		}
	}

	// set the timestamp of when to play this message based on the 'timing' value
	if ( timing == MESSAGE_TIME_IMMEDIATE )
		MessageQ[i].window_timestamp = _timestamp(MESSAGE_IMMEDIATE_TIMESTAMP);
	else if ( timing == MESSAGE_TIME_SOON )
		MessageQ[i].window_timestamp = _timestamp(MESSAGE_SOON_TIMESTAMP);
	else
		MessageQ[i].window_timestamp = TIMESTAMP::invalid();		// make invalid

	insertion_sort(MessageQ, MessageQ_num, message_queue_priority_compare);
}

// given a message id#, should it be filtered for me?
int message_filter_multi(int id)
{
	// not multiplayer
	if(!(Game_mode & GM_MULTIPLAYER)){
		return 0;
	}

	// bogus
	if((id < 0) || (id >= Num_messages)){
		mprintf(("Filtering bogus mission message!\n"));
		return 1;
	}

	// builtin messages
	if(id < Num_builtin_messages){
	}
	// mission-specific messages
	else {
		// not team filtered
		if(Messages[id].multi_team < 0){
			return 0;
		}

		// not TvT
		if(!(Netgame.type_flags & NG_TYPE_TEAM)){
			return 0;
		}

		// is this for my team?
		if((Net_player != NULL) && (Net_player->p_info.team != Messages[id].multi_team)){
			mprintf(("Filtering team-based mission message!\n"));
			return 1;
		}
	}		
	
	return 0;
}

// send_unique_to_player sends a mission unique (specific) message to the player (possibly a multiplayer
// person).  These messages are *not* the builtin messages
void message_send_unique( const char *id, const void *data, int m_source, int priority, int group, int delay, int event_num_to_cancel )
{
	int i, source;
	const char *who_from;

	source = 0;
	who_from = NULL;
	for (i=0; i<Num_messages; i++) {
		// find the message
		if ( !stricmp(id, Messages[i].name) ) {

			// if the ship pointer and special_who are both NULL then this is from generic "Terran Command"
			// if the ship is NULL and special_who is not NULL, then this is from special_who
			// otherwise, message is from ship.
			if ( m_source == MESSAGE_SOURCE_COMMAND ) {
				who_from = The_mission.command_sender;
				source = HUD_SOURCE_TERRAN_CMD;
			} else if ( m_source == MESSAGE_SOURCE_SPECIAL ) {
				who_from = (const char *)data;
				source = HUD_SOURCE_IMPORTANT;
			} else if ( m_source == MESSAGE_SOURCE_WINGMAN ) {
				int m_persona, ship_index;

				// find a wingman with the same persona as this message.  If the message's persona doesn't
				// exist, we will use Terran command
				m_persona = Messages[i].persona_index;
				if ( m_persona == -1 ) {
					mprintf(("Warning:  Message %s has no persona assigned.\n", Messages[i].name));
				}

				// get a ship. we allow silenced ships since this is a unique messange and therefore the mission designer 
				// should have taken into account that the ship may have been silenced.
				ship_index = ship_get_random_player_wing_ship( SHIP_GET_NO_PLAYERS, 0.0f, m_persona, 1, Messages[i].multi_team);

				// if the ship_index is -1, then make the message come from Terran command
				if ( ship_index == -1 ) {
					who_from = The_mission.command_sender;
					source = HUD_SOURCE_TERRAN_CMD;
				} else {
					who_from = Ships[ship_index].ship_name;
					source = HUD_team_get_source(Ships[ship_index].team);
				}

			} else if ( m_source == MESSAGE_SOURCE_SHIP ) {
				auto shipp = (const ship *)data;
				who_from = shipp->ship_name;
				source = HUD_team_get_source(shipp->team);
			} else if ( m_source == MESSAGE_SOURCE_NONE ) {
				who_from = "<none>";
			}

			// not multiplayer or this message is for me, then queue it
			// if ( !(Game_mode & GM_MULTIPLAYER) || ((multi_target == -1) || (multi_target == MY_NET_PLAYER_NUM)) ){

			// maybe filter it out altogether
			if (!message_filter_multi(i)) {
				message_queue_message( i, priority, MESSAGE_TIME_ANYTIME, who_from, source, group, delay, -1, event_num_to_cancel );
			}

			// send a message packet to a player if destined for everyone or only a specific person
			if ( MULTIPLAYER_MASTER ){
				send_mission_message_packet( i, who_from, priority, MESSAGE_TIME_SOON, source, -1, -1, -1, delay, event_num_to_cancel );
			}			

			return;		// all done with displaying		
		}
	}
	nprintf (("messaging", "Couldn't find message id %s to send to player!\n", id ));
}

bool should_skip_builtin_message(int type, ship* shipp) {
	if (type == MESSAGE_NONE) {
		return true;
	}

	// If we aren't showing builtin msgs, bail.
	if (The_mission.flags[Mission::Mission_Flags::No_builtin_msgs]) {
		return true;
	}

	// Karajorma - If we aren't showing builtin msgs from command and this is not a ship, bail.
	if ((shipp == NULL) && (The_mission.flags[Mission::Mission_Flags::No_builtin_command])) {
		return true;
	}

	// Respect the tabled occurrence chance.
	int occurrence_chance = Builtin_messages[type].occurrence_chance;
	return (occurrence_chance < 100 && (int)(frand()*100) > occurrence_chance);
}

int get_persona_type(ship_info* sip) {
	if (sip->is_fighter_bomber()) {
		return PERSONA_FLAG_WINGMAN;
	} else if (sip->flags[Ship::Info_Flags::Support]) {
		return PERSONA_FLAG_SUPPORT;
	} else {
		return PERSONA_FLAG_LARGE;
	}
}

int pick_persona(ship* shipp) {
	SCP_vector<int> candidates;
	ship_info* sip = &Ship_info[shipp->ship_info_index];
	int species = sip->species;
	int persona_type = get_persona_type(sip);
	for (int i = 0; i < (int)Personas.size(); i++) {
		if (species >= 32 || (Personas[i].species_bitfield & (1 << species)) == 0) {
			// The persona's species is incompatible with the ship's
			continue;
		}
		if (Personas[i].flags & PERSONA_FLAG_NO_AUTOMATIC_ASSIGNMENT)
			// The persona does not allow us to pick it
			continue;
		if ((Personas[i].flags & persona_type) == 0) {
			// The persona is the wrong type
			continue;
		}
		if (Personas[i].flags & PERSONA_FLAG_USED) {
			// This persona has been used before, but note it in case we need it
			candidates.push_back(i);
		} else {
			// We haven't used this persona yet, so do!
			Personas[i].flags |= PERSONA_FLAG_USED;
			return i;
		}
	}
	int count = (int) candidates.size();
	if (count == 1) {
		return candidates[0];
	} else if (count > 1) {
		return candidates[Random::next(count)];
	} else if (persona_type & PERSONA_FLAG_SUPPORT) {
		// Species without a support persona (e.g. the UEF) historically used the
		// first support persona; retain that behavior
		return Default_support_persona;
	} else {
		return -1;
	} 
}

bool can_auto_assign_persona(ship* shipp) {
	// If the Auto_assign_personas flag is on, we can assign them for any ship
	// Otherwise, we can only assign them for support
	return Auto_assign_personas || Ship_info[shipp->ship_info_index].flags[Ship::Info_Flags::Support];
}

int get_persona(ship* shipp) {
	if (shipp == NULL) {
		return The_mission.command_persona;
	} else if (shipp->persona_index != -1) {
		return shipp->persona_index;
	} else if (can_auto_assign_persona(shipp)) {
		return shipp->persona_index = pick_persona(shipp);
	} else {
		return -1;
	}
}

bool persona_allows_substitution(int persona) {
	return (persona >= 0) && (Personas[persona].flags & PERSONA_FLAG_SUBSTITUTE_MISSING_MESSAGES);
}

bool has_filters(MessageFilter& filter) {
	return !filter.ship_name.empty()
	    || !filter.callsign.empty()
			|| !filter.class_name.empty()
			|| !filter.wing_name.empty()
			||  filter.species_bitfield
			||  filter.type_bitfield
			||  filter.team_bitfield;
}

bool filter_matches(SCP_string value, SCP_vector<SCP_string>& filter) {
	for (SCP_vector<SCP_string>::iterator iter = filter.begin(); iter != filter.end(); ++iter) {
		if (value == *iter) {
			return true;
		}
	}
	return filter.empty();
}

bool filter_matches(int value, int filter) {
	return (filter == 0) || ((1 << value) & filter);
}

bool filters_match(MessageFilter& filter, ship* it) {
	Assert (has_filters(filter));
	if (it == nullptr) {
		return false;
	} else {
		int wing_index = it->wingnum;
		SCP_string wing_name = (wing_index < 0) ? "" : Wings[wing_index].name;
		return filter_matches(it->ship_name, filter.ship_name)
	      && filter_matches(hud_get_ship_callsign(it), filter.callsign)
		    && filter_matches(hud_get_ship_class(it), filter.class_name)
		    && filter_matches(wing_name, filter.wing_name)
		    && filter_matches(Ship_info[it->ship_info_index].species, filter.species_bitfield)
		    && (Ship_info[it->ship_info_index].class_type < 0 || filter_matches(Ship_info[it->ship_info_index].class_type, filter.type_bitfield))
		    && filter_matches(it->team, filter.team_bitfield);
	}
}

bool outer_filters_match(MessageFilter& filter, int range, ship* sender) {
	for (auto i: list_range(&Ship_obj_list)) {
		auto obj = &Objects[i->objnum];
		// Ignore dead/dying ships
		if (obj->flags[Object::Object_Flags::Should_be_dead] || Ships[obj->instance].flags[Ship::Ship_Flags::Dying]) {
			continue;
		}
		// Ignore the sender itself
		if (sender->objnum == i->objnum) {
			continue;
		}
		// If a range was specified, ignore anything out of range
		if ((range > 0) && (vm_vec_dist(&obj->pos, &Objects[sender->objnum].pos) > range)) {
			continue;
		}
		// If we got this far, we can check our filters
		if (filters_match(filter, &Ships[obj->instance])) {
			return true;
		}
	}
	return false;
}

bool excludes_current_mood(int message) {
	for (SCP_vector<int>::iterator iter = Messages[message].excluded_moods.begin(); iter != Messages[message].excluded_moods.end(); ++iter) {
		if (*iter == Current_mission_mood) {
			return true;
		}
	}
	return false;
}

int get_builtin_message_inner(int type, int persona, ship* sender, ship* subject, bool require_exact_persona_match) {
	const char* name = Builtin_messages[type].name;
	SCP_vector<int> matching_builtins;
	int match_level, best_match_level = 0;

	for (int i = 0; i < Num_builtin_messages; i++) {
		if (stricmp(Messages[i].name, name)) {
			continue;
		} else if (Messages[i].persona_index == persona) {
			match_level = BUILTIN_MATCHES_PERSONA;
		} else if (require_exact_persona_match) {
			continue;
		} else if (Personas[Messages[i].persona_index].flags & PERSONA_FLAG_NO_AUTOMATIC_ASSIGNMENT) {
			continue;
		} else if ((persona >= 0) && ((Personas[Messages[i].persona_index].species_bitfield & Personas[persona].species_bitfield) != 0)) {
			match_level = BUILTIN_MATCHES_SPECIES;
		} else {
			match_level = BUILTIN_MATCHES_TYPE;
		}

		// Apply "Prefer this message" flags
		match_level |= Messages[i].boost_level;

		if (Current_mission_mood == Messages[i].mood) {
			// Boost messages that match the current mood
			match_level |= BUILTIN_MATCHES_MOOD;
		} else if (excludes_current_mood(i)) {
			// Ignore messages that are incompatible with the current mood
			continue;
		}

		// Apply sender filters
		if (has_filters(Messages[i].sender_filter)) {
			if (filters_match(Messages[i].sender_filter, sender)) {
				// Boost messages that have at least one filter
				match_level |= BUILTIN_MATCHES_FILTER;
			} else {
				// Ignore messages where any filter doesn't match
				continue;
			}
		} else if (type == MESSAGE_BACKUP && !Allow_generic_backup_messages) {
			// Historically, only certain wings were allowed to send Backup messages.
			// The hardcoded requirement has been moved to message filters, but it
			// would break backwards compatibility if we suddenly start sending Backup
			// messages for wings that previously couldn't.
			continue;
		}

		// Ditto with subject filters
		if (has_filters(Messages[i].subject_filter)) {
			if (filters_match(Messages[i].subject_filter, subject)) {
				// Boost messages that have at least one filter
				match_level |= BUILTIN_MATCHES_FILTER;
			} else {
				// Ignore messages where any filter doesn't match
				continue;
			}
		}

		// Ditto with outer filters, although they're more complicated
		if (has_filters(Messages[i].outer_filter)) {
			if (outer_filters_match(Messages[i].outer_filter, Messages[i].outer_filter_radius, sender)) {
				// Boost messages that have at least one filter
				match_level |= BUILTIN_MATCHES_FILTER;
			} else {
				// Ignore messages where any filter doesn't match
				continue;
			}
		}

		if (match_level == best_match_level) {
			matching_builtins.push_back(i);
		} else if (match_level > best_match_level) {
			best_match_level = match_level;
			matching_builtins.clear();
			matching_builtins.push_back(i);
		}
	}

	int matches = (int) matching_builtins.size();
	if (matches == 1) {
		return matching_builtins[0];
	} else if (matches > 0) {
		return matching_builtins[Random::next(matches)];
	}

	// We're still here, so look for a fallback message
	int fallback = Builtin_messages[type].fallback;
	if (fallback != MESSAGE_NONE) {
		return get_builtin_message_inner(fallback, persona, sender, subject, require_exact_persona_match);
	} 

	// Still here? Guess we're staying silent
	return MESSAGE_NONE;
}

int get_builtin_message(int type, int persona, ship* sender, ship* subject) {
	int result = get_builtin_message_inner(type, persona, sender, subject, true);
	if (result != MESSAGE_NONE) {
		return result;
	} else if (persona_allows_substitution(persona)) {
		// Only borrow messages from other personae as an absolute last-ditch effort
		return get_builtin_message_inner(type, persona, sender, subject, false);
	} else {
		return MESSAGE_NONE;
	}
}

// send builtin_to_player sends a message (from messages.tbl) to the player. These messages are
// the generic informational type messages. The have priorities like misison specific messages,
// and use a timing to tell how long we should wait before playing this message
bool message_send_builtin(int type, ship* sender, ship* subject, int multi_target, int multi_team_filter) {
	if (should_skip_builtin_message(type, sender)) {
		return false;
	}

	int persona_index = get_persona(sender);
	int message_index = get_builtin_message(type, persona_index, sender, subject);
	if (message_index == MESSAGE_NONE) {
		return false;
	}

	// Get the message's source for HUD purposes
	int source;
	const char *who_from;
	if (sender) {
		who_from = sender->ship_name;
		source = HUD_team_get_source(sender->team);
	} else if (type == MESSAGE_ALREADY_ON_WAY) {
		// If the player called for support while a support ship is arriving, have it acknowledge the request
		who_from = SUPPORT_NAME;
		source = HUD_SOURCE_TERRAN_CMD;
	} else {
		who_from = The_mission.command_sender;

		// Goober5000 - if Command is a ship that is present, change the source accordingly
		int shipnum = ship_name_lookup(who_from);
		if (shipnum >= 0) {
			source = HUD_team_get_source(Ships[shipnum].team);
		} else {
			source = HUD_SOURCE_TERRAN_CMD;
		}
	}

	// Queue this message, handling multiplayer routing if needed
	int priority = Builtin_messages[type].priority;
	int timing = Builtin_messages[type].timing;

	// Not multiplayer or this message is for me, then queue it
	if (!(Game_mode & GM_MULTIPLAYER) || ((multi_target == -1) || (multi_target == MY_NET_PLAYER_NUM))) {
		// if this filter matches mine
		if ((multi_team_filter < 0) || !(Netgame.type_flags & NG_TYPE_TEAM) || ((Net_player != NULL) && (Net_player->p_info.team == multi_team_filter))) {
			message_queue_message(message_index, priority, timing, who_from, source, 0, 0, type, -1);
		}
	}

	// Send a message packet to a player if destined for everyone or only a specific person
	if (MULTIPLAYER_MASTER) {
		// only send a message if it is of a particular type
		if (multi_target == -1) {
			if (multi_message_should_broadcast(type)) {
				send_mission_message_packet(message_index, who_from, priority, timing, source, type, -1, multi_team_filter);
			}
		} else {
			send_mission_message_packet(message_index, who_from, priority, timing, source, type, multi_target, multi_team_filter);
		}
	}
	return true;
}

// message_is_playing()
//
// Return the Message_playing flag. Message_playing is local to MissionMessage.cpp, but
// this info is needed by code in HUDsquadmsg.cpp
//
int message_is_playing()
{
	return Num_messages_playing?1:0;
}

// Functions below pertain only to personas!!!!

// given a character string, try to find the persona index
int message_persona_name_lookup(const char* name)
{
	int i;

	for (i = 0; i < (int)Personas.size(); i++) {
		if ( !stricmp(Personas[i].name, name) )
			return i;
	}

	return -1;
}

// Blank out portions of the audio playback for the sound identified by Message_wave
// This works by using the same Distort_pattern[][] that was used to distort the associated text
void message_maybe_distort()
{
	int i;
	int was_muted;

	if ( Num_messages_playing == 0 )
		return;
	
	for ( i = 0; i < Num_messages_playing; i++ ) {
		if ( !snd_is_playing(Playing_messages[i].wave) )
			return;
	}

	// distort the number of voices currently playing
	for ( i = 0; i < Num_messages_playing; i++ ) {
		Assert(Playing_messages[i].wave.isValid());

		was_muted = 0;

		if ( comm_between_player_and_ship(Playing_messages[i].shipnum, Playing_messages[i].builtin_type == MESSAGE_WINGMAN_SCREAM) != COMM_OK ) {
			was_muted = Message_wave_muted;
			if ( timestamp_elapsed(Next_mute_time) ) {
				Next_mute_time = fl2i(Distort_patterns[Distort_num][Distort_next++] * Message_wave_duration);
				if ( Distort_next >= MAX_DISTORT_LEVELS )
					Distort_next = 0;

				Message_wave_muted ^= 1;
			}
		
			if ( Message_wave_muted ) {
				if ( !was_muted )
					snd_set_volume(Playing_messages[i].wave, 0.0f, true);
			} else {
				if ( was_muted )
					snd_set_volume(Playing_messages[i].wave, (Master_voice_volume * aav_voice_volume), true);
			}
		}
	}
}


// if the player communications systems are heavily damaged, distort incoming messages.
//
// first case: Message_wave_duration == 0 (this occurs when there is no associated voice playback)
//					Blank out random runs of characters in the message
//
// second case: Message_wave_duration > 0 (occurs when voice playback accompainies message)
//					 Blank out portions of the sound based on Distort_num, this this is that same
//					 data that will be used to blank out portions of the audio playback
//
void message_maybe_distort_text(SCP_string &text, int shipnum, bool for_death_scream)
{
	int voice_duration;

	if (comm_between_player_and_ship(shipnum, for_death_scream) == COMM_OK) {
		return;
	}

	auto len         = unicode::num_codepoints(text.begin(), text.end());
	if (Message_wave_duration == 0) {
		SCP_string result_str;

		size_t next_distort = Random::next(5, 9);
		size_t i            = 0;
		size_t run = 0;
		for (auto cp : unicode::codepoint_range(text.c_str())) {
			if (i == next_distort) {
				run = Random::next(3, 7);
				if (i + run > len)
					run = len - i;
			}

			if (run > 0) {
				unicode::encode(UNICODE_CHAR('-'), std::back_inserter(result_str));
				--run;

				if (run <= 0) {
					next_distort = i + Random::next(5, 9);
				}
			} else {
				unicode::encode(cp, std::back_inserter(result_str));
			}

			++i;
		}
		text = std::move(result_str);
		return;
	}

	voice_duration = Message_wave_duration;

	// distort text
	Distort_num = Random::next(MAX_DISTORT_PATTERNS);
	Distort_next = 0;
	unicode::codepoint_range range(text.c_str());
	auto curr_iter = range.begin();
	size_t curr_offset = 0;
	SCP_string result_str;
	while (voice_duration > 0) {
		size_t run = fl2i(Distort_patterns[Distort_num][Distort_next] * len);
		auto upper_limit = std::min(len, curr_offset + run);
		auto num_chars = upper_limit - curr_offset;
		if (Distort_next & 1) {
			for (size_t i = 0; i < num_chars; ++i, ++curr_iter) {
				if (*curr_iter != UNICODE_CHAR(' ')) {
					unicode::encode(UNICODE_CHAR('-'), std::back_inserter(result_str));
				} else {
					unicode::encode(UNICODE_CHAR(' '), std::back_inserter(result_str));
				}
			}

			curr_offset += num_chars;
			if ( upper_limit >= len )
				break;
		} else {
			for (size_t i = 0; i < num_chars; ++i) {
				unicode::encode(*curr_iter, std::back_inserter(result_str));
				++curr_iter;
			}
			curr_offset += run;
		}

		voice_duration -= fl2i(Distort_patterns[Distort_num][Distort_next]*Message_wave_duration);
		Distort_next++;
		if ( Distort_next >= MAX_DISTORT_LEVELS )
			Distort_next = 0;
	}
	text = std::move(result_str);
	
	Distort_next = 0;
}

// Load mission messages (this is called by the level paging code when running with low memory)
void message_pagein_mission_messages()
{
	int i;
	
	mprintf(("Paging in mission messages\n"));

	if (Num_messages <= Num_builtin_messages) {
		return;
	}

	char *sound_filename;

	for (i=Num_builtin_messages; i<Num_messages; i++) {
		if (Messages[i].wave_info.index != -1) {
			sound_filename = Message_waves[Messages[i].wave_info.index].name;
			message_load_wave(Messages[i].wave_info.index, sound_filename);
		}
	}
}

// ---------------------------------------------------
// Add and remove messages - used by autopilot code now, but useful elswhere

bool add_message(const char* name, const char* message, int persona_index, int multi_team)
{
	MissionMessage msg; 
	strcpy_s(msg.name, name);
	strcpy_s(msg.message, message);
	msg.persona_index = persona_index;
	msg.multi_team = multi_team;
	msg.avi_info.index = -1;
	msg.wave_info.index = -1;
	Messages.push_back(msg);
	Num_messages++;

	return true;
}

bool change_message(const char* name, const char* message, int persona_index, int multi_team)
{
	for (int i = Num_builtin_messages; i < Num_messages; i++) 
	{
		if (!strcmp(Messages[i].name, name)) 
		{
			strcpy_s(Messages[i].message, message);
			Messages[i].persona_index = persona_index;
			Messages[i].multi_team = multi_team;

			Messages[i].avi_info.index = -1;
			Messages[i].wave_info.index = -1;
			return true;
		}
	}

	// not found.. fall through
	return add_message(name, message, persona_index, multi_team);
}

/**
 * This returns the minimum of the comm state between the player and the other ship.  In practice, retail has no checks whatsoever on a ship's ability
 * to send messages unless that ship is the player, so such a change requires an AI profiles option and we must default to the player's state.  However, we
 * have a bit of wiggle room with COMM_SCRAMBLED, because EMP effects are either transient or set by the scramble-messages SEXP.
 */
int comm_between_player_and_ship(int other_shipnum, bool for_death_scream)
{
	int player_comm_state = hud_communications_state(Player_ship);

	if (other_shipnum < 0)
		return player_comm_state;

	int other_comm_state = hud_communications_state(&Ships[other_shipnum], for_death_scream);

	if (The_mission.ai_profile->flags[AI::Profile_Flags::Check_comms_for_non_player_ships])
	{
		return MIN(player_comm_state, other_comm_state);
	}
	else
	{
		if (player_comm_state == COMM_OK && other_comm_state == COMM_OK)
			return COMM_OK;
		else if (player_comm_state == COMM_OK && other_comm_state == COMM_SCRAMBLED)
			return COMM_SCRAMBLED;
		else
			return player_comm_state;
	}
}

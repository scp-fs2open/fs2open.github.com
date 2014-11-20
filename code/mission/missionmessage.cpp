/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "hud/hudmessage.h"
#include "hud/hudgauges.h"
#include "hud/hudtarget.h"
#include "io/timer.h"
#include "parse/parselo.h"
#include "gamesnd/gamesnd.h"
#include "gamesequence/gamesequence.h"
#include "anim/animplay.h"
#include "hud/hud.h"
#include "ship/ship.h"
#include "ship/subsysdamage.h"
#include "weapon/emp.h"
#include "localization/localize.h"
#include "hud/hudconfig.h"
#include "sound/fsspeech.h"
#include "species_defs/species_defs.h"
#include "parse/sexp.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "mod_table/mod_table.h"
#include "parse/scripting.h"

SCP_vector<SCP_string> Builtin_moods;
int Current_mission_mood;

int Valid_builtin_message_types[MAX_BUILTIN_MESSAGE_TYPES]; 
// here is the list of the builtin message names and the settings which control how frequently
// they are heard.  These names are used to match against names read in for builtin message 
// radio bits to see what message to play.  
// These are generic names, meaning that there will be the same message type for a 
// number of different personas
builtin_message Builtin_messages[] =
{
//XSTR:OFF
	{"Arrive Enemy",			100,	-1,		0}, 
	{"Attack Target",			100,	-1,		0}, 
	{"Beta Arrived",			100,	-1,		0}, 
	{"Check 6",					100,	2,		6000}, 
	{"Engage",					100,	-1,		0}, 
	{"Gamma Arrived",			100,	-1,		0}, 
	{"Help",					100,	10,		60000}, 
	{"Praise",					100,	10,		60000}, 
	{"Backup",					100,	-1,		0}, 
	{"Ignore Target",			100,	-1,		0}, 
	{"No",						100,	-1,		0}, 
	{"Oops 1",					100,	-1,		0}, 
	{"Permission",				100,	-1,		0}, 		// AL: no code support yet
	{"Stray",					100,	-1,		0}, 			// DA: no code support
	{"Depart",					100,	-1,		0}, 
	{"yes",						100,	-1,		0}, 
	{"Rearm on Way",			100,	-1,		0}, 
	{"On way",					100,	-1,		0}, 
	{"Rearm warping in",		100,	-1,		0}, 
	{"No Target",				100,	-1,		0}, 
	{"Docking Start",			100,	-1,		0}, 		// AL: no message seems to exist for this
	{"Repair Done",				100,	-1,		0}, 
	{"Repair Aborted",			100,	-1,		0}, 
	{"Traitor",					100,	-1,		0}, 
	{"Rearm",					100,	-1,		0}, 
	{"Disable Target",			100,	-1,		0}, 
	{"Disarm Target",			100,	-1,		0}, 
	{"Player Dead",				100,	-1,		0}, 
	{"Death",					50,		10,		60000}, 
	{"Support Killed",			100,	-1,		0}, 
	{"All Clear",				100,	-1,		0}, 			// DA: no code support
	{"All Alone",				100,	-1,		0}, 
	{"Repair",					100,	-1,		0}, 
	{"Delta Arrived",			100,	-1,		0}, 
	{"Epsilon Arrived",			100,	-1,		0}, 
	{"Instructor Hit",			100,	-1,		0}, 
	{"Instructor Attack",		100,	-1,		0}, 
	{"Stray Warning",			100,	-1,		0}, 
	{"Stray Warning Final",		100,	-1,		0}, 
	{"AWACS at 75",				100,	-1,		0}, 
	{"AWACS at 25",				100,	-1,		0}, 
	{"Praise Self",				10,		4,		60000}, 
	{"High Praise",				100,	-1,		0}, 
	{"Rearm Primaries",			100,	-1,		0}, 
	{"Primaries Low",			100,	-1,		0}, 
	//XSTR:ON
};

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

/*typedef struct pmessage {
	//anim_instance *anim;		// handle of anim currently playing
	anim *anim_data;			// animation data to be used by the talking head HUD gauge handler
	int start_frame;			// the start frame needed to play the animation
	bool play_anim;			// used to tell HUD gauges if they should be playing or not
	int wave;					// handle of wave currently playing
	int id;						// id of message currently playing
	int priority;				// priority of message currently playing
	int shipnum;				// shipnum of ship sending this message,  -1 if from Terran command
	int builtin_type;			// if a builtin message, type of the message
} pmessage;*/

pmessage Playing_messages[MAX_PLAYING_MESSAGES];

int Message_shipnum;						// ship number of who is sending message to player -- used outside this module
int Message_expire;							// timestamp to extend the duration of message brackets when not using voice files

// variables to control message queuing.  All new messages to the player are queued.  The array
// will be ordered by priority, then time submitted.

#define MQF_CONVERT_TO_COMMAND		(1<<0)			// convert this queued message to terran command
#define MQF_CHECK_ALIVE					(1<<1)			// check for the existence of who_from before sending

typedef struct message_q {
	fix	time_added;					// time at which this entry was added
	int	window_timestamp;			// timestamp which will tell us how long we have to play the message
	int	priority;					// priority of the message
	int	message_num;				// index into the Messages[] array
	char *special_message;			// Goober5000 - message to play if we've replaced stuff (like variables)
	char who_from[NAME_LENGTH];		// who this message is from
	int	source;						// who the source of the message is (HUD_SOURCE_* type)
	int	builtin_type;				// type of builtin message (-1 if mission message)
	int	flags;						// should this message entry be converted to Terran Command head/wave file
	int	min_delay_stamp;			// minimum delay before this message will start playing
	int	group;						// message is part of a group, don't time it out
} message_q;

#define MAX_MESSAGE_Q				30
#define MAX_MESSAGE_LIFE			F1_0*30		// After being queued for 30 seconds, don't play it
#define DEFAULT_MESSAGE_LENGTH	3000			// default number of milliseconds to display message indicator on hud
message_q	MessageQ[MAX_MESSAGE_Q];
int MessageQ_num;			// keeps track of number of entries on the queue.

#define MESSAGE_IMMEDIATE_TIMESTAMP		1000		// immediate messages must play within 1 second
#define MESSAGE_SOON_TIMESTAMP			5000		// "soon" messages must play within 5 seconds
#define MESSAGE_ANYTIME_TIMESTAMP		-1			// anytime timestamps are invalid

// Persona information
int Num_personas;
Persona *Personas = NULL;

char *Persona_type_names[MAX_PERSONA_TYPES] = 
{
//XSTR:OFF
	"wingman",
	"support",
	"large", 
	"command",
//XSTR:ON
};

int Default_command_persona;

// Goober5000
// NOTE - these are truncated filenames, i.e. without extensions
SCP_vector<SCP_string> generic_message_filenames;

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

// forward declarations
void message_maybe_distort_text(char *text, int shipnum);
int comm_between_player_and_ship(int other_shipnum);

// following functions to parse messages.tbl -- code pretty much ripped from weapon/ship table parsing code

static void persona_parse_close()
{
	if (Personas != NULL) {
		vm_free(Personas);
		Personas = NULL;
	}
}

// functions to deal with parsing personas.  Personas are just a list of names that give someone
// sending a message an identity which spans the life of the mission
void persona_parse()
{
	int i;
	char type[NAME_LENGTH];

	static bool done_at_exit = false;
	if ( !done_at_exit ) {
		atexit( persona_parse_close );
		done_at_exit = true;
	}

	// this way should cause the least amount of problems on the various platforms - taylor
	Personas = (Persona*)vm_realloc( Personas, sizeof(Persona) * (Num_personas + 1) );

	if (Personas == NULL)
		Error(LOCATION, "Not enough memory to allocate Personas!" );

	memset(&Personas[Num_personas], 0, sizeof(Persona));

	required_string("$Persona:");
	stuff_string(Personas[Num_personas].name, F_NAME, NAME_LENGTH);

	// get the type name and set the appropriate flag
	required_string("$Type:");
	stuff_string( type, F_NAME, NAME_LENGTH );
	for ( i = 0; i < MAX_PERSONA_TYPES; i++ ) {
		if ( !stricmp( type, Persona_type_names[i]) ) {

			Personas[Num_personas].flags |= (1<<i);

			// save the Command persona in a global
			if ( Personas[Num_personas].flags & PERSONA_FLAG_COMMAND ) {
				// always use the most recent Command persona
				// found, since that's how retail does it
				Default_command_persona = Num_personas;
			}

			break;
		}
	}

	if ( i == MAX_PERSONA_TYPES )
		WarningEx(LOCATION, "Unknown persona type in messages.tbl -- %s\n", type );

	char cstrtemp[NAME_LENGTH];
	if ( optional_string("+") )
	{
		int j;
		stuff_string(cstrtemp, F_NAME, NAME_LENGTH);

		for (j = 0; j < (int)Species_info.size(); j++)
		{
			if (!strcmp(cstrtemp, Species_info[j].species_name))
			{
				Personas[Num_personas].species = j;
				break;
			}
		}

		if ( j == (int)Species_info.size() )
			WarningEx(LOCATION, "Unknown species in messages.tbl -- %s\n", cstrtemp );
	}

	Num_personas++;
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
	generic_anim_init( &extra.anim_data );
	strcpy_s( extra.name, avi_name );
	strcpy_s( extra.anim_data.filename, avi_name);
	extra.num = -1;
	generic_anim_load(&extra.anim_data);
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
	extra.num = -1;
	Message_waves.push_back(extra);
	Num_message_waves++;
	return ((int)Message_waves.size() - 1);
}

// parses an individual message
void message_parse(bool importing_from_fsm)
{
	MissionMessage msg;
	char persona_name[NAME_LENGTH];

	required_string("$Name:");
	stuff_string(msg.name, F_NAME, NAME_LENGTH);

	// team
	msg.multi_team = -1;
	if(optional_string("$Team:")){
		int mt;
		stuff_int(&mt);

		// keep it real
		if((mt < 0) || (mt >= MAX_TVT_TEAMS)){
			mt = -1;
		}

		// only bother with filters in-game if multiplayer and TvT
		if(Fred_running || (MULTI_TEAM) ){
			msg.multi_team = mt;
		}
	}

	// backwards compatibility for old fred missions - all new ones should use $MessageNew
	if(optional_string("$Message:")){
		stuff_string(msg.message, F_MESSAGE, MESSAGE_LENGTH);
	} else {
		required_string("$MessageNew:");
		stuff_string(msg.message, F_MULTITEXT, MESSAGE_LENGTH);
	}

	msg.persona_index = -1;
	if ( optional_string("+Persona:") ) {
		stuff_string(persona_name, F_NAME, NAME_LENGTH);
		msg.persona_index = message_persona_name_lookup( persona_name );

		if ( msg.persona_index == -1 )
			WarningEx(LOCATION, "Unknown persona in message %s in messages.tbl -- %s\n", msg.name, persona_name );
	}

	if ( !Fred_running)
		msg.avi_info.index = -1;
	else
		msg.avi_info.name = NULL;

	if ( optional_string("+AVI Name:") ) {
		char avi_name[MAX_FILENAME_LEN];

		stuff_string(avi_name, F_NAME, MAX_FILENAME_LEN);

		// Goober5000 - for some reason :V: swapped Head-TP1
		// and Head-TP4 in FS2
		if (importing_from_fsm && !strnicmp(avi_name, "Head-TP1", 8))
			avi_name[7] = '4';

		if ( !Fred_running ) {
			msg.avi_info.index = add_avi(avi_name);
		} else {
			msg.avi_info.name = vm_strdup(avi_name);
		}
	}

	if ( !Fred_running )
		msg.wave_info.index = -1;
	else
		msg.wave_info.name = NULL;

	if ( optional_string("+Wave Name:") ) {
		char wave_name[MAX_FILENAME_LEN];

		stuff_string(wave_name, F_NAME, MAX_FILENAME_LEN);
		if ( !Fred_running ) {
			msg.wave_info.index = add_wave(wave_name);
		} else {
			msg.wave_info.name = vm_strdup(wave_name);
		}
	}

	if ( optional_string("$Mood:")) {
		SCP_string buf; 
		bool found = false;

		stuff_string(buf, F_NAME); 
		for (SCP_vector<SCP_string>::iterator iter = Builtin_moods.begin(); iter != Builtin_moods.end(); ++iter) {
			if (iter->compare(buf) == 0) {
				msg.mood = iter - Builtin_moods.begin();
				found = true;
				break;
			}
		}

		if (!found) {
			// found a mood, but it's not in the list of moods at the start of the table
			Warning(LOCATION, "Message.tbl has an entry for mood type %s, but this mood is not in the #Moods section of the table.", buf.c_str()); 
		}
	}
	else {
		msg.mood = 0;
	}

	if ( optional_string("$Exclude Mood:")) {
		SCP_vector<SCP_string> buff;
		bool found = false;

		stuff_string_list(buff); 
		for (SCP_vector<SCP_string>::iterator parsed_moods = buff.begin(); parsed_moods != buff.end(); ++parsed_moods) {
			for (SCP_vector<SCP_string>::iterator iter = Builtin_moods.begin(); iter != Builtin_moods.end(); ++iter) {
				if (!stricmp(iter->c_str(), parsed_moods->c_str())) {
					msg.excluded_moods.push_back(iter - Builtin_moods.begin());
					found = true;
					break;
				}
			}

			if (!found) {
				// found a mood, but it's not in the list of moods at the start of the table
				Warning(LOCATION, "Message.tbl has an entry for exclude mood type %s, but this mood is not in the #Moods section of the table.", parsed_moods->c_str()); 
			}
		}
	}

	Num_messages++;
	Messages.push_back(msg); 
}

void message_frequency_parse()
{
	char name[32];
	int i, max_count, min_delay, occurrence_chance;  
	int builtin_type = -1; 

	required_string("$Name:");
	stuff_string(name, F_NAME, NAME_LENGTH);

	for (i = 0; i < MAX_BUILTIN_MESSAGE_TYPES; i++) {
		if (!strcmp(name, Builtin_messages[i].name)) {
			builtin_type = i;
			break;
		}
	}

	if (builtin_type == -1) {
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

void message_moods_parse()
{	

	while (required_string_either("#End", "$Mood:")){
		SCP_string buf; 

		required_string("$Mood:");
		stuff_string(buf, F_NAME);

		Builtin_moods.push_back(buf);
	}

	required_string("#End");
}

void parse_msgtbl()
{
	int i, j;

	//speed things up a little by setting the capacities for the message vectors to roughly the FS2 amounts
	Messages.reserve(500);
	Message_waves.reserve(300);
	Message_avis.reserve(30);

	read_file_text("messages.tbl", CF_TYPE_TABLES);
	reset_parse();
	Num_messages = 0;
	Num_personas = 0;

	// Goober5000 - ugh, ugly hack to fix the FS2 retail tables
	char *pVawacs25 = strstr(Mp, "Vawacs25.wav");
	if (pVawacs25)
	{
		char *pAwacs75 = strstr(pVawacs25, "Awacs75.wav");
		if (pAwacs75)
		{
			// move the 'V' from the first filename to the second, and adjust the 'A' case
			*pVawacs25 = 'A';
			for (i = 1; i < (pAwacs75 - pVawacs25) - 1; i++)
				pVawacs25[i] = pVawacs25[i+1];
			pAwacs75[-1] = 'V';
			pAwacs75[0] = 'a';
		}
	}

	// now we can start parsing
	if (optional_string("#Message Frequencies")) {
		while (!required_string_3("$Name:", "#Personas", "#Moods" )) {
			message_frequency_parse();
		}
	}	

	Builtin_moods.push_back("Default");
	if (optional_string("#Moods")) {
		message_moods_parse();
	}	


	required_string("#Personas");
	while ( required_string_either("#Messages", "$Persona:")){
		persona_parse();
	}

	required_string("#Messages");
	while (required_string_either("#End", "$Name:")){
		message_parse();
	}

	required_string("#End");

	// save the number of builtin message things -- make initing between missions easier
	Num_builtin_messages = Num_messages;
	Num_builtin_avis = Num_message_avis;
	Num_builtin_waves = Num_message_waves;

	
	memset(Valid_builtin_message_types, 0, sizeof(int)*MAX_BUILTIN_MESSAGE_TYPES); 
	// now cycle through the messages to determine which type of builtins we have messages for
	for (i = 0; i < Num_builtin_messages; i++) {
		for (j = 0; j < MAX_BUILTIN_MESSAGE_TYPES; j++) {
			if (!(stricmp(Messages[i].name, Builtin_messages[j].name))) {
				Valid_builtin_message_types[j] = 1; 
				break;
			}
		}
	}


	// additional table part!
	generic_message_filenames.clear();
	generic_message_filenames.push_back("none");
	generic_message_filenames.push_back("cuevoice");
	generic_message_filenames.push_back("emptymsg");
	generic_message_filenames.push_back("generic");
	generic_message_filenames.push_back("msgstart");

	if (optional_string("#Simulated Speech Overrides"))
	{
		char filename[MAX_FILENAME_LEN];

		while (required_string_either("#End", "$File Name:"))
		{
			required_string("$File Name:");
			stuff_string(filename, F_NAME, MAX_FILENAME_LEN);

			// get extension
			char *ptr = strchr(filename, '.');
			if (ptr == NULL)
			{
				Warning(LOCATION, "Simulated speech override file '%s' was provided with no extension!", filename);
				continue;
			}

			// test extension
			if (stricmp(ptr, ".ogg") && stricmp(ptr, ".wav"))
			{
				Warning(LOCATION, "Simulated speech override file '%s' was provided with an extension other than .wav or .ogg!", filename);
				continue;
			}

			// truncate extension
			*ptr = '\0';

			// add truncated file name
			generic_message_filenames.push_back(filename);
		}

		required_string("#End");
	}
}

// this is called at the start of each level
void messages_init()
{
	int rval, i;
	static int table_read = 0;

	if ( !table_read ) {
		Default_command_persona = -1;

		if ((rval = setjmp(parse_abort)) != 0) {
			mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "messages.tbl", rval));
			return;
		}

		parse_msgtbl();
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
	for (i = 0; i < MAX_MESSAGE_Q; i++) {
		MessageQ[i].priority = -1;
		MessageQ[i].time_added = -1;
		MessageQ[i].message_num = -1;
		MessageQ[i].builtin_type = -1;
		MessageQ[i].min_delay_stamp = -1;
		MessageQ[i].group = 0;

		// Goober5000
		MessageQ[i].special_message = NULL;
	}
	
	// this forces a reload of the AVI's and waves for builtin messages.  Needed because the flic and
	// sound system also get reset between missions!
	for (i = 0; i < Num_builtin_avis; i++ ) {
		generic_anim_unload(&Message_avis[i].anim_data);
	}

	for (i = 0; i < Num_builtin_waves; i++ ){
		Message_waves[i].num = -1;
	}

	Message_shipnum = -1;
	Num_messages_playing = 0;
	for ( i = 0; i < MAX_PLAYING_MESSAGES; i++ ) {
		//Playing_messages[i].anim = NULL;
		Playing_messages[i].anim_data = NULL;
		Playing_messages[i].start_frame = -1;
		Playing_messages[i].play_anim = false;
		Playing_messages[i].wave = -1;
		Playing_messages[i].id = -1;
		Playing_messages[i].priority = -1;
		Playing_messages[i].shipnum = -1;
		Playing_messages[i].builtin_type = -1;
	}

	// reinitialize the personas.  mark them all as not used
	for ( i = 0; i < Num_personas; i++ ){
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

	// Make sure this code doesn't get run if the talking head guage is off
	// helps prevent a crash on jump out if this code doesn't work right
	if ( !hud_gauge_active(HUD_TALKING_HEAD) )
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
	if (Num_messages_playing) {
		message_kill_all(1);
	}

	// remove the wave sounds from memory
	for (i = 0; i < Num_message_waves; i++ ) {
		if ( Message_waves[i].num != -1 ){
			snd_unload( Message_waves[i].num );
		}
	}

	fsspeech_stop();

	// free up remaining anim data - taylor
	for (i=0; i<Num_message_avis; i++) {
		message_mission_free_avi(i);
	}

	// Goober5000 - free up special messages
	for (i = 0; i < MAX_MESSAGE_Q; i++)
	{
		if (MessageQ[i].special_message != NULL)
		{
			vm_free(MessageQ[i].special_message);
			MessageQ[i].special_message = NULL;
		}
	}
}

// call from game_shutdown() ONLY!!!
void message_mission_close()
{
	// free the persona data
	if (Personas != NULL) {
		vm_free( Personas );
		Personas = NULL;
	}
}

// functions to deal with queuing messages to the message system.

//	Compare function for sorting message queue entries based on priority.
//	Return values set to sort array in _decreasing_ order.  If priorities equal, sort based
// on time added into queue
int message_queue_priority_compare(const void *a, const void *b)
{
	message_q *ma, *mb;

	ma = (message_q *) a;
	mb = (message_q *) b;

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

// function to kill all currently playing messages.  kill_all parameter tells us to
// kill only the animations that are playing, or wave files too
void message_kill_all( int kill_all )
{
	int i;

	Assert( Num_messages_playing );

	// kill sounds for all voices currently playing
	for ( i = 0; i < Num_messages_playing; i++ ) {
		/*if ( (Playing_messages[i].anim != NULL) && anim_playing(Playing_messages[i].anim) ) {
			anim_stop_playing( Playing_messages[i].anim );
			Playing_messages[i].anim=NULL;
		}*/
		if ( Playing_messages[i].play_anim) {
			Playing_messages[i].play_anim = false;
		}

		if ( kill_all ) {
			if ( (Playing_messages[i].wave != -1 ) && snd_is_playing(Playing_messages[i].wave) ){
				snd_stop( Playing_messages[i].wave );
			}

			Playing_messages[i].shipnum = -1;
		}
	}

	if ( kill_all ) {
		Num_messages_playing = 0;
	}

	fsspeech_stop();
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

	if ( (Playing_messages[message_num].wave != -1 ) && snd_is_playing(Playing_messages[message_num].wave) )
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
#define MESSAGE_GET_LOWEST			2
int message_get_priority(int which)
{
	int i;
	int priority;

	if ( which == MESSAGE_GET_HIGHEST ){
		priority = MESSAGE_PRIORITY_LOW;
	} else {
		priority = MESSAGE_PRIORITY_HIGH;
	}

	for ( i = 0; i < Num_messages_playing; i++ ) {
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

	MessageQ_num--;
	q->priority = -1;
	q->time_added = -1;
	q->message_num = -1;
	q->builtin_type = -1;
	q->min_delay_stamp = -1;
	q->group = 0;

	// Goober5000
	if (q->special_message != NULL)
	{
		vm_free(q->special_message);
		q->special_message = NULL;
	}

	if ( MessageQ_num > 0 ) {
		insertion_sort(MessageQ, MAX_MESSAGE_Q, sizeof(message_q), message_queue_priority_compare);
	}
}

// Load in the sound data for a message.
//
// index - index into the Message_waves[] array
//
void message_load_wave(int index, const char *filename)
{
	Assertion(index >= 0, "Invalid index passed!");

	if ( Message_waves[index].num >= 0) {
		return;
	}

	if ( !Sound_enabled ) {
		Message_waves[index].num = -1;
		return;
	}

	game_snd tmp_gs;
	strcpy_s( tmp_gs.filename, filename );
	Message_waves[index].num = snd_load( &tmp_gs, 0 );

	if (Message_waves[index].num == -1)
		nprintf(("messaging", "Cannot load message wave: %s.  Will not play\n", Message_waves[index].name));
}

// Goober5000
bool message_filename_is_generic(char *filename)
{
	char truncated_filename[MAX_FILENAME_LEN];

	// truncate any file extension
	strcpy_s(truncated_filename, filename);
	char *ptr = strchr(truncated_filename, '.');

	// extension must be a recognized sound file
	if ((ptr == NULL) || (stricmp(ptr, ".ogg") && stricmp(ptr, ".wav")))
		return false;

	// truncate it
	*ptr = '\0';

	// test against the list
	for (unsigned int i = 0; i < generic_message_filenames.size(); i++)
	{
		if (!stricmp(generic_message_filenames[i].c_str(), truncated_filename))
			return true;
	}

	return false;
}

// Play wave file associated with message
// input: m		=>		pointer to message description
//
// note: changes Messave_wave_duration, Playing_messages[].wave, and Message_waves[].num
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
		if ( q->flags & MQF_CONVERT_TO_COMMAND ) {
			char *p, new_filename[MAX_FILENAME_LEN];

			Message_waves[index].num = -1;					// forces us to reload the message

			// bash the filename here. Look for "[1-6]_" at the front of the message.  If found, then
			// convert to TC_*
			p = strchr(filename, '_' );
			if ( p == NULL ) {
				mprintf(("Cannot convert %s to terran command wave -- find Sandeep or Allender\n", Message_waves[index].name));
				return false;
			}

			// prepend the command name, and then the rest of the filename.
			p++;
			strcpy_s( new_filename, COMMAND_WAVE_PREFIX );
			strcat_s( new_filename, p );
			strcpy_s( filename, new_filename );
		}

		// load the sound file into memory
		message_load_wave(index, filename);
		if ( Message_waves[index].num == -1 ) {
			m->wave_info.index = -1;
		}

		if ( m->wave_info.index >= 0 ) {
			// this call relies on the fact that snd_play returns -1 if the sound cannot be played
			Message_wave_duration = snd_get_duration(Message_waves[index].num);
			Playing_messages[Num_messages_playing].wave = snd_play_raw( Message_waves[index].num, 0.0f );

			return (Playing_messages[Num_messages_playing].wave != -1);
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
		start_frame = (ani->num_frames-1) - fl2i(fps * wave_time + 0.5f);
	} else {
		int num_frames_extra;
		num_frames_extra = fl2i(fps * (anim_time - wave_time) + 0.5f);
		if ( num_frames_extra > 0 ) {
			start_frame=rand()%num_frames_extra;
		}
	}

	if ( start_frame < 0 ) {
		Int3();
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
	int subhead_selected = FALSE;
	if ( (q->message_num < Num_builtin_messages) || !(_strnicmp(HEAD_PREFIX_STRING, ani_name, strlen(HEAD_PREFIX_STRING)-1)) ) {
		persona_index = m->persona_index;
		
		// if this ani should be converted to a terran command, set the persona to the command persona
		// so the correct head plays.
		if ( q->flags & MQF_CONVERT_TO_COMMAND ) {
			persona_index = The_mission.command_persona;
			strcpy_s( ani_name, COMMAND_HEAD_PREFIX );
		}

		// Goober5000 - guard against negative array indexing; this way, if no persona was
		// assigned, the logic will drop down below like it's supposed to
		if (persona_index >= 0)
		{
			if ( Personas[persona_index].flags & (PERSONA_FLAG_WINGMAN | PERSONA_FLAG_SUPPORT) ) {
				// get a random head
				if ( q->builtin_type == MESSAGE_WINGMAN_SCREAM ) {
					rand_index = MAX_WINGMAN_HEADS;		// [0,MAX) are regular heads; MAX is always death head
					is_death_scream = 1;
				} else {
					rand_index = ((int) Missiontime % MAX_WINGMAN_HEADS);
				}
				strcpy_s(temp, ani_name);
				sprintf(ani_name, "%s%c", temp, 'a'+rand_index);
				subhead_selected = TRUE;
			} else if ( Personas[persona_index].flags & (PERSONA_FLAG_COMMAND | PERSONA_FLAG_LARGE) ) {
				// get a random head
				// Goober5000 - *sigh*... if mission designers assign a command persona
				// to a wingman head, they risk having the death ani play
				if ( !strnicmp(ani_name, "Head-TP", 7) || !strnicmp(ani_name, "Head-VP", 7) ) {
					mprintf(("message '%s' incorrectly assigns a command/largeship persona to a wingman animation!\n", m->name));
					rand_index = ((int) Missiontime % MAX_WINGMAN_HEADS);
				} else {
					rand_index = ((int) Missiontime % MAX_COMMAND_HEADS);
				}

				strcpy_s(temp, ani_name);
				sprintf(ani_name, "%s%c", temp, 'a'+rand_index);
				subhead_selected = TRUE;
			} else {
				mprintf(("message '%s' uses an unrecognized persona type\n", m->name));
			}
		}

		if (!subhead_selected) {
			// choose between a and b
			rand_index = ((int) Missiontime % MAX_WINGMAN_HEADS);
			strcpy_s(temp, ani_name);
			sprintf(ani_name, "%s%c", temp, 'a'+rand_index);
			mprintf(("message '%s' with invalid head.  Fix by assigning persona to the message.\n", m->name));
		}
		nprintf(("Messaging", "playing head %s for %s\n", ani_name, q->who_from));
	}

	// check to see if the avi has been loaded.  If not, then load the AVI.  On an error loading
	// the avi, set the top level index to -1 to avoid multiple tries at loading the flick.

	// if there is something already here that's not this same file then go ahead a let go of it - taylor
	if ( !strstr(anim_info->anim_data.filename, ani_name) ) {
		message_mission_free_avi( m->avi_info.index );
	}

	generic_anim_init(&anim_info->anim_data, ani_name);
	if(!Full_color_head_anis)
			anim_info->anim_data.use_hud_color = true;

	if ( generic_anim_stream(&anim_info->anim_data) < 0 ) {
		nprintf (("messaging", "Cannot load message avi %s.  Will not play.\n", ani_name));
		m->avi_info.index = -1;			// if cannot load the avi -- set this index to -1 to avoid trying to load multiple times
	}

	if ( m->avi_info.index >= 0 ) {
		// This call relies on the fact that AVI_play will return -1 if the AVI cannot be played
		// if any messages are already playing, kill off any head anims that are currently playing.  We will
		// only play a head anim of the newest messages being played
		if ( Num_messages_playing > 0 ) {
			nprintf(("messaging", "killing off any currently playing head animations\n"));
			message_kill_all( 0 );
		}

		if ( hud_disabled() ) {
			return;
		}
		
		anim_info->anim_data.direction = GENERIC_ANIM_DIRECTION_NOLOOP;
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
	char	buf[MESSAGE_LENGTH];
	char who_from[NAME_LENGTH];	
	message_q *q;
	int i;
	MissionMessage *m;
	bool builtinMessage = false; // gcc doesn't like var decls crossed by goto's

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
			if ( (Playing_messages[i].wave != -1) && (snd_time_remaining(Playing_messages[i].wave) > 250) )
				wave_done = 0;

			// Goober5000
			if (fsspeech_playing())
				wave_done = 0;

			// AL 1-20-98: If voice message is done, kill the animation early
			if ( (Playing_messages[i].wave != -1) && wave_done ) {
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
				if ( (Ships[Playing_messages[i].shipnum].flags & SF_DYING) && (Playing_messages[i].builtin_type != MESSAGE_WINGMAN_SCREAM) ) {
					int shipnum;

					shipnum = Playing_messages[i].shipnum;
					message_kill_playing( i );
					// force this guy to scream
					// AL 22-2-98: Ensure don't use -1 to index into ships array.  Mark, something is incorrect 
					//             here, since message_kill_playing() seems to always set Playing_messages[i].shipnum to -1
					// MWA 3/24/98 -- save shipnum before killing message
					// 
					Assert( shipnum >= 0 );
					if ( !(Ships[shipnum].flags & SF_SHIP_HAS_SCREAMED) && !(Ships[shipnum].flags2 & SF2_NO_DEATH_SCREAM) ) {
						ship_scream( &Ships[shipnum] );
					}
					continue;							// this should keep us in the while() loop with same value of i.														
				}											// we should enter the next 'if' statement during next pass
			}

			// if both ani and wave are done, mark internal variable so we can do next message on queue, and
			// global variable to clear voice brackets on hud
			if ( wave_done && ani_done && ( timestamp_elapsed(Message_expire) || (Playing_messages[i].wave != -1) || (Playing_messages[i].shipnum == -1) ) ) {
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
		if ( timestamp_valid(q->window_timestamp) && timestamp_elapsed(q->window_timestamp) && !q->group) {
			// remove message from queue and see if more to remove
			nprintf(("messaging", "Message %s didn't play because it didn't fit into time window.\n", Messages[q->message_num].name));
			if ( q->message_num < Num_builtin_messages ){			// we should only ever remove builtin messages this way
				message_remove_from_queue(q);
			} else {
				break;
			}
		} else {
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
		if((MessageQ[idx].min_delay_stamp == -1) || timestamp_elapsed(MessageQ[idx].min_delay_stamp)){
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
		message_q temp;

		// store the entry
		memcpy(&temp, &MessageQ[found], sizeof(message_q));

		// move all other entries up
		for(idx=found; idx>0; idx--){
			memcpy(&MessageQ[idx], &MessageQ[idx-1], sizeof(message_q));
		}

		// plop the entry down as being first
		memcpy(&MessageQ[0], &temp, sizeof(message_q));
	}

	q = &MessageQ[0];
	Assert ( q->message_num != -1 );
	Assert ( q->priority != -1 );
	Assert ( q->time_added != -1 );

	if ( Num_messages_playing ) {
		// peek at the first message on the queue to see if it should interrupt, or overlap a currently
		// playing message.  Mission specific messages will always interrupt builtin messages.  They
		// will never interrupt other mission specific messages.
		//
		//  Builtin message might interrupt other builtin messages, or overlap them, all depending on
		// message priority.

		if ( q->builtin_type == MESSAGE_HAMMER_SWINE ) {
			message_kill_all(1);
		} else if ( message_playing_specific_builtin(MESSAGE_HAMMER_SWINE) ) {
			MessageQ_num = 0;
			return;
		} else if ( message_playing_builtin() && ( q->message_num >= Num_builtin_messages) && (q->priority > MESSAGE_PRIORITY_LOW) ) {
			// builtin is playing and we have a unique message to play.  Kill currently playing message
			// so unique can play uninterrupted.  Only unique messages higher than low priority will interrupt
			// other messages.
			message_kill_all(1);
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
					message_kill_all(1);
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
					message_kill_all(1);
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

	// Goober5000 - argh, don't conflate special sources with ships!
	// NOTA BENE: don't check for != MESSAGE_SOURCE_COMMAND, because with the new command persona code, Command could be a ship
	if ( q->source != MESSAGE_SOURCE_SPECIAL ) {
		Message_shipnum = ship_name_lookup( q->who_from );

		// see if we need to check if sending ship is alive
		if ( (Message_shipnum < 0) && (q->flags & MQF_CHECK_ALIVE) ) {
			goto all_done;
		}
	}

	// if this is a ship, then don't play anything if this ship is already talking
	if ( Message_shipnum != -1 ) {
		for ( i = 0; i < Num_messages_playing; i++ ) {
			if ( (Playing_messages[i].shipnum != -1) && (Playing_messages[i].shipnum == Message_shipnum) ){
				return;
			}
		}
	}

	// set up module globals for this message
	m = &Messages[q->message_num];
	Playing_messages[Num_messages_playing].anim_data = NULL;
	Playing_messages[Num_messages_playing].wave  = -1;
	Playing_messages[Num_messages_playing].id  = q->message_num;
	Playing_messages[Num_messages_playing].priority = q->priority;
	Playing_messages[Num_messages_playing].shipnum = Message_shipnum;
	Playing_messages[Num_messages_playing].builtin_type = q->builtin_type;

	Message_wave_duration = 0;

	// translate tokens in message to the real things
	if (q->special_message == NULL)
		message_translate_tokens(buf, m->message);
	else
		message_translate_tokens(buf, q->special_message);

	Message_expire = timestamp(42 * strlen(buf));
	// AL: added 07/14/97.. only play avi/sound if in gameplay
	if ( gameseq_get_state() != GS_STATE_GAME_PLAY )
		goto all_done;

	// AL 4-7-98: Can't receive messages if comm is destroyed
	if ( hud_communications_state(Player_ship) == COMM_DESTROYED ) {
		goto all_done;
	}

	//	Don't play death scream unless a small ship.
	if ( q->builtin_type == MESSAGE_WINGMAN_SCREAM ) {
		if (!((Ship_info[Ships[Message_shipnum].ship_info_index].flags & SIF_SMALL_SHIP) || (Ships[Message_shipnum].flags2 & SF2_ALWAYS_DEATH_SCREAM)) ) {
			goto all_done;
		}
	}

	// play wave first, since need to know duration for picking anim start frame
	if(message_play_wave(q) == false) {
		fsspeech_play(FSSPEECH_FROM_INGAME, buf);
	}

	// play animation for head
	message_play_anim(q);
	
	// distort the message if comms system is damaged
	message_maybe_distort_text(buf, Message_shipnum);

#ifndef NDEBUG
	// debug only -- if the message is a builtin message, put in parens whether or not the voice played
	if ( Sound_enabled && (Playing_messages[Num_messages_playing].wave == -1) ) {
		strcat_s( buf, NOX("..(no wavefile for voice)"));
		snd_play(&Snds[SND_CUE_VOICE]);
	}
#endif
	
	strcpy_s (who_from, q->who_from);

	// if this is a ship, do we use name or callsign or ship class?
	if ( Message_shipnum >= 0 ) {
		ship *shipp = &Ships[Message_shipnum];
		if ( shipp->callsign_index >= 0 ) {
			hud_stuff_ship_callsign( who_from, shipp );
		} else if ( ((Iff_info[shipp->team].flags & IFFF_WING_NAME_HIDDEN) && (shipp->wingnum != -1)) || (shipp->flags2 & SF2_HIDE_SHIP_NAME) ) {
			hud_stuff_ship_class( who_from, shipp );
		} else {
			end_string_at_first_hash_symbol(who_from);
		}
	}

	if ( !stricmp(who_from, "<none>") ) {
		HUD_sourced_printf( q->source, NOX("%s"), buf );
	} else HUD_sourced_printf( q->source, NOX("%s: %s"), who_from, buf );

	if ( Message_shipnum >= 0 ) {
		hud_target_last_transmit_add(Message_shipnum);
	}

	Script_system.SetHookVar("Name", 's', m->name);
	Script_system.SetHookVar("Message", 's', buf);
	Script_system.SetHookVar("SenderString", 's', who_from);

	builtinMessage = q->builtin_type != -1;
	Script_system.SetHookVar("Builtin", 'b', &builtinMessage);
	if (Message_shipnum >= 0)
	{
		object* sender = &Objects[Ships[Message_shipnum].objnum];

		Script_system.SetHookObject("Sender", sender);

		Script_system.RunCondition(CHA_MSGRECEIVED, 0, NULL, sender);

		Script_system.RemHookVar("Sender");
	}
	else
	{
		Script_system.RunCondition(CHA_MSGRECEIVED);
	}

	Script_system.RemHookVars(4, "Name", "Message", "SenderString", "Builtin");

all_done:
	Num_messages_playing++;
	message_remove_from_queue( q );
}

// queues up a message to display to the player
void message_queue_message( int message_num, int priority, int timing, char *who_from, int source, int group, int delay, int builtin_type )
{
	int i, m_persona;
	char temp_buf[MESSAGE_LENGTH];

	if ( message_num < 0 ) return;

	// some messages can get queued quickly.  Try to filter out certain types of messages before
	// they get queued if there are other messages of the same type already queued
	if ( (builtin_type == MESSAGE_REARM_ON_WAY) || (builtin_type == MESSAGE_OOPS) ) {
		// if it is already playing, then don't play it
		if ( message_playing_specific_builtin(builtin_type) ) 
			return;

		for ( i = 0; i < MessageQ_num; i++ ) {
			// if one of these messages is already queued, then don't play
			if ( (MessageQ[i].message_num == message_num) && (MessageQ[i].builtin_type == builtin_type) )
				return;

		}
	}

	// check to be sure that we haven't reached our max limit on these messages yet.
	if ( MessageQ_num == MAX_MESSAGE_Q ) {
		mprintf(("Message queue already full. Message will not be added!\n"));										
		return;
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
	i = MessageQ_num;
	MessageQ[i].time_added = Missiontime;
	MessageQ[i].priority = priority;
	MessageQ[i].message_num = message_num;
	MessageQ[i].source = source;
	MessageQ[i].builtin_type = builtin_type;
	MessageQ[i].min_delay_stamp = timestamp(delay);
	MessageQ[i].group = group;
	strcpy_s(MessageQ[i].who_from, who_from);

	// Goober5000 - this shouldn't happen, but let's be safe
	if (MessageQ[i].special_message != NULL)
	{
		Int3();
		vm_free(MessageQ[i].special_message);
		MessageQ[i].special_message = NULL;
	}

	// Goober5000 - replace variables if necessary
	strcpy_s(temp_buf, Messages[message_num].message);
	if (sexp_replace_variable_names_with_values(temp_buf, MESSAGE_LENGTH))
		MessageQ[i].special_message = vm_strdup(temp_buf);

	// SPECIAL HACK -- if the who_from is terran command, and there is a wingman persona attached
	// to this message, then set a bit to tell the wave/anim playing code to play the command version
	// of the wave and head
	MessageQ[i].flags = 0;
	if ( !stricmp(who_from, The_mission.command_sender) && (m_persona != -1) && (Personas[m_persona].flags & PERSONA_FLAG_WINGMAN) ) {
		MessageQ[i].flags |= MQF_CONVERT_TO_COMMAND;
		MessageQ[i].source = HUD_SOURCE_TERRAN_CMD;
	}

	if ( (m_persona != -1) && (Personas[m_persona].flags & PERSONA_FLAG_WINGMAN) ) {
		if ( !strstr(who_from, ".wav") ) {
			MessageQ[i].flags |= MQF_CHECK_ALIVE;
		}
	}

	// set the timestamp of when to play this message based on the 'timing' value
	if ( timing == MESSAGE_TIME_IMMEDIATE )
		MessageQ[i].window_timestamp = timestamp(MESSAGE_IMMEDIATE_TIMESTAMP);
	else if ( timing == MESSAGE_TIME_SOON )
		MessageQ[i].window_timestamp = timestamp(MESSAGE_SOON_TIMESTAMP);
	else
		MessageQ[i].window_timestamp = timestamp(MESSAGE_ANYTIME_TIMESTAMP);		// make invalid

	MessageQ_num++;
	insertion_sort(MessageQ, MAX_MESSAGE_Q, sizeof(message_q), message_queue_priority_compare);

	// Try to start it!
	// MWA -- called every frame from game loop
	//message_queue_process();
}

// function to return the persona index of the given ship.  If it isn't assigned, it will be
// in this function.  persona_type could be a wingman, Terran Command, or other generic ship
// type personas.  ship is the ship we should assign a persona to
int message_get_persona( ship *shipp )
{
	int i = 0, ship_type, count;
	int *slist = new int[Num_personas];
	memset( slist, 0, sizeof(int) * Num_personas );

	if ( shipp != NULL ) {
		// see if this ship has a persona
		if ( shipp->persona_index != -1 ) {
		//	return shipp->persona_index;
			i = shipp->persona_index;
			goto I_Done;
		}

		// get the type of ship (i.e. support, fighter/bomber, etc)
		ship_type = Ship_info[shipp->ship_info_index].flags;

		int persona_needed;
		count = 0;

		if ( ship_type & (SIF_FIGHTER|SIF_BOMBER) )
		{
			persona_needed = PERSONA_FLAG_WINGMAN;
		} else if ( ship_type & SIF_SUPPORT ) 
		{
			persona_needed = PERSONA_FLAG_SUPPORT;
		}
		else 
		{
			persona_needed = PERSONA_FLAG_LARGE;
		}

		// first try to go for an unused persona
		for (i = 0; i < Num_personas; i++)
		{
			// this Persona is not our species - skip it
			if (Personas[i].species != Ship_info[shipp->ship_info_index].species)
				continue;

			// check the ship types, and don't try to assign those which don't type match
			if ( Personas[i].flags & persona_needed)
			{
				if (!(Personas[i].flags & PERSONA_FLAG_USED))
				{
					// if it hasn't been used - USE IT!
					Personas[i].flags |= PERSONA_FLAG_USED;
				//	return i;
					goto I_Done;
				}
				else
				{
					// otherwise add it to our list of valid options to randomly select from
					slist[count] = i;
					count++;
				}
			}
		}

		// we didn't find an unused one - so we randomly select one
		if(count != 0)
		{
			i = (rand() % count);
			i = slist[i];
		}
		// RT Protect against count being zero
		else
			i = slist[0];

		//return i;
		goto I_Done;
	}

	// for now -- we don't support other types of personas (non-wingman personas)
	Int3();
//	return 0;

I_Done:
	delete[] slist;

	return i;
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
void message_send_unique_to_player( char *id, void *data, int m_source, int priority, int group, int delay )
{
	int i, source;
	char *who_from;

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
				who_from = (char *)data;
				source = HUD_SOURCE_TERRAN_CMD;
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
				ship *shipp;

				shipp = (ship *)data;
				who_from = shipp->ship_name;
				source = HUD_team_get_source(shipp->team);
			} else if ( m_source == MESSAGE_SOURCE_NONE ) {
				who_from = "<none>";
			}

			// not multiplayer or this message is for me, then queue it
			// if ( !(Game_mode & GM_MULTIPLAYER) || ((multi_target == -1) || (multi_target == MY_NET_PLAYER_NUM)) ){

			// maybe filter it out altogether
			if (!message_filter_multi(i)) {
				message_queue_message( i, priority, MESSAGE_TIME_ANYTIME, who_from, source, group, delay );
			}

			// send a message packet to a player if destined for everyone or only a specific person
			if ( MULTIPLAYER_MASTER ){
				send_mission_message_packet( i, who_from, priority, MESSAGE_TIME_SOON, source, -1, -1, -1, delay);
			}			

			return;		// all done with displaying		
		}
	}
	nprintf (("messaging", "Couldn't find message id %s to send to player!\n", id ));
}

#define BUILTIN_MATCHES_TYPE					0
#define BUILTIN_MATCHES_SPECIES					1
#define BUILTIN_MATCHES_PERSONA_CHECK_MOOD		2
#define BUILTIN_MATCHES_PERSONA_EXCLUDED		3
#define BUILTIN_MATCHES_PERSONA					4
#define	BUILTIN_MATCHES_PERSONA_MOOD			5

typedef	struct matching_builtin {
		int type_of_match;
		int message_index;
}matching_builtin;

// send builtin_to_player sends a message (from messages.tbl) to the player.  These messages are
// the generic informational type messages.  The have priorities like misison specific messages,
// and use a timing to tell how long we should wait before playing this message
void message_send_builtin_to_player( int type, ship *shipp, int priority, int timing, int group, int delay, int multi_target, int multi_team_filter )
{
	int i, persona_index = -1, persona_species = -1, message_index = -1, random_selection = -1;
	int source;
	int num_matching_builtins = 0;
	char *who_from;
	int best_match = -1;

	matching_builtin *current_builtin = new matching_builtin(); 
	SCP_vector <matching_builtin> matching_builtins; 


	// if we aren't showing builtin msgs, bail
	if (The_mission.flags & MISSION_FLAG_NO_BUILTIN_MSGS)
		return;

	// Karajorma - If we aren't showing builtin msgs from command and this is not a ship, bail
	if ( (shipp == NULL) && (The_mission.flags & MISSION_FLAG_NO_BUILTIN_COMMAND) ) 
		return;

	// builtin type isn't supported by this version of the table
	if (!Valid_builtin_message_types[type]) {
		// downgrade certain message types to more generic ones more likely to be supported
		if (type == MESSAGE_HIGH_PRAISE ) {
			type = MESSAGE_PRAISE; 
		}
		else if ( type == MESSAGE_REARM_PRIMARIES ) {
			type = MESSAGE_REARM_REQUEST; 
		}
		else {
			return;
		}

		// check if the downgraded type is also invalid
		if (!Valid_builtin_message_types[type]) {
			return;
		}
	}

	// see if there is a persona assigned to this ship.  If not, then try to assign one!!!
	if ( shipp ) {
		// Karajorma - the game should assert if a silenced ship gets this far
		Assert( !(shipp->flags2 & SF2_NO_BUILTIN_MESSAGES) );

		if ( shipp->persona_index == -1 )
			shipp->persona_index = message_get_persona( shipp );

		persona_index = shipp->persona_index;

		if ( persona_index == -1 )
			nprintf(("messaging", "Couldn't find persona for %s\n", shipp->ship_name ));	

		// be sure that this ship can actually send a message!!! (i.e. not-not-flyable -- get it!)
		Assert( !(Ship_info[shipp->ship_info_index].flags & SIF_NOT_FLYABLE) );		// get allender or alan
	} else {
		persona_index = The_mission.command_persona;				// use the terran command persona
	}

	char *name = Builtin_messages[type].name;

	if (persona_index >= 0) {
		persona_species = Personas[persona_index].species;
	}

	// try to find a builtin message with the given type for the given persona
	// we may try to play a message in the wrong persona if we can't find the right message for the given persona
	for ( i = 0; i < Num_builtin_messages; i++ ) {
		// check the type of message
		if ( !stricmp(Messages[i].name, name) ) {
			// condition 1: we have a type match
			current_builtin->message_index = i;
			current_builtin->type_of_match =  BUILTIN_MATCHES_TYPE; 

			// check the species of this persona (if required)
			if ( (persona_species >= 0) && (Personas[Messages[i].persona_index].species == persona_species) ) {
				// condition 2: we have a type + species match
				current_builtin->type_of_match =  BUILTIN_MATCHES_SPECIES; 
			}

			// check the exact persona (if required)
			// NOTE: doesn't need to be nested under the species condition above
			if ( (persona_index >= 0) && (Messages[i].persona_index == persona_index) ) {
				// condition 3: type + species + persona index match	
				current_builtin->type_of_match =  BUILTIN_MATCHES_PERSONA_CHECK_MOOD; 
			}

			// check if the personas mood suits this particular message, first check if it is excluded
			if (!Messages[i].excluded_moods.empty() && (current_builtin->type_of_match ==  BUILTIN_MATCHES_PERSONA_CHECK_MOOD)) {
				for (SCP_vector<int>::iterator iter = Messages[i].excluded_moods.begin(); iter != Messages[i].excluded_moods.end(); ++iter) {
					if (*iter == Current_mission_mood) {
						current_builtin->type_of_match =  BUILTIN_MATCHES_PERSONA_EXCLUDED; 
						break; 
					}
				}
			}

			if (current_builtin->type_of_match ==  BUILTIN_MATCHES_PERSONA_CHECK_MOOD) {
				if (Current_mission_mood == Messages[i].mood) {
					current_builtin->type_of_match =  BUILTIN_MATCHES_PERSONA_MOOD; 
				}
				else {
					current_builtin->type_of_match =  BUILTIN_MATCHES_PERSONA; 
				}
			}			

			if (current_builtin->type_of_match == best_match) {
				num_matching_builtins++;
			}
			// otherwise check to see if the this is the best kind of match we've found so far
			else if (current_builtin->type_of_match > best_match) {
				best_match = current_builtin->type_of_match; 
				num_matching_builtins = 1;
			}

			// add the match to our list
			matching_builtins.push_back(*current_builtin); 
		}
	}

	if (best_match == BUILTIN_MATCHES_PERSONA_EXCLUDED) {
		mprintf(("MESSAGING", "Couldn't find builtin message %s for persona %d with a none excluded mood\n", Builtin_messages[type].name, persona_index ));
		mprintf(("MESSAGING", "using an excluded message for this persona\n"));
	}else if (best_match == BUILTIN_MATCHES_SPECIES) {
		mprintf(("MESSAGING", "Couldn't find builtin message %s for persona %d\n", Builtin_messages[type].name, persona_index ));
		mprintf(("MESSAGING", "using a message for any persona of that species\n"));
	} else if (best_match == BUILTIN_MATCHES_TYPE) {
		mprintf(("MESSAGING", "Couldn't find builtin message %s for persona %d\n", Builtin_messages[type].name, persona_index ));
		mprintf(("MESSAGING", "looking for message for any persona of any species\n"));
	} else if (best_match < 0) {
		mprintf(("MESSAGING", "Couldn't find any builtin message of type %d\n", type ));
		Int3();
		return; 
	}
	
	// since we may have multiple builtins we need to pick one at random
	random_selection = (int)(rand32() % num_matching_builtins) + 1; 

	// loop through the vector until we have found enough elements of the correct matching type
	for (i = 0; i < (int)matching_builtins.size(); i++) {
		if (matching_builtins[i].type_of_match == best_match) {
			random_selection--; 
			if (random_selection == 0) {
				message_index = matching_builtins[i].message_index;
				break;
			}
		}
	}

	Assertion (random_selection == 0, "unable to randomly select built in message correctly, still have %d selections left", random_selection); 

	// get who this message is from -- kind of a hack since we assume Terran Command in the
	// absence of a ship.  This will be fixed later
	if ( shipp ) {
		who_from = shipp->ship_name;
		source = HUD_team_get_source( shipp->team );
	} else {
		who_from = The_mission.command_sender;

		// Goober5000 - if Command is a ship that is present, change the source accordingly
		int shipnum = ship_name_lookup(who_from);
		if (shipnum >= 0)
			source = HUD_team_get_source( Ships[shipnum].team );
		else
			source = HUD_SOURCE_TERRAN_CMD;
	}

	// maybe change the who from here for special rearm cases (always seems like that is the case :-) )
	if ( !stricmp(who_from, The_mission.command_sender) && (type == MESSAGE_REARM_ON_WAY) ){
		who_from = SUPPORT_NAME;
	}

	// determine what we should actually do with this dang message.  In multiplayer, we must
	// deal with the fact that this message might not get played on my machine if I am a server

	// not multiplayer or this message is for me, then queue it
	if ( !(Game_mode & GM_MULTIPLAYER) || ((multi_target == -1) || (multi_target == MY_NET_PLAYER_NUM)) ){

		// if this filter matches mine
		if( (multi_team_filter < 0) || !(Netgame.type_flags & NG_TYPE_TEAM) || ((Net_player != NULL) && (Net_player->p_info.team == multi_team_filter)) ){
			message_queue_message( message_index, priority, timing, who_from, source, group, delay, type );
		}
	}

	// send a message packet to a player if destined for everyone or only a specific person
	if ( MULTIPLAYER_MASTER ) {
		// only send a message if it is of a particular type
		if(multi_target == -1){
			if(multi_message_should_broadcast(type)){				
				send_mission_message_packet( message_index, who_from, priority, timing, source, type, -1, multi_team_filter );
			}
		} else {
			send_mission_message_packet( message_index, who_from, priority, timing, source, type, multi_target, multi_team_filter );
		}
	}
}

// message_is_playing()
//
// Return the Message_playing flag.  Message_playing is local to MissionMessage.cpp, but
// this info is needed by code in HUDsquadmsg.cpp
//
int message_is_playing()
{
	return Num_messages_playing?1:0;
}

// Functions below pertain only to personas!!!!

// given a character string, try to find the persona index
int message_persona_name_lookup( char *name )
{
	int i;

	for (i = 0; i < Num_personas; i++ ) {
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
		Assert(Playing_messages[i].wave >= 0 );

		was_muted = 0;

		if ( comm_between_player_and_ship(Playing_messages[i].shipnum) != COMM_OK) {
			was_muted = Message_wave_muted;
			if ( timestamp_elapsed(Next_mute_time) ) {
				Next_mute_time = fl2i(Distort_patterns[Distort_num][Distort_next++] * Message_wave_duration);
				if ( Distort_next >= MAX_DISTORT_LEVELS )
					Distort_next = 0;

				Message_wave_muted ^= 1;
			}
		
			if ( Message_wave_muted ) {
				if ( !was_muted )
					snd_set_volume(Playing_messages[i].wave, 0.0f);
			} else {
				if ( was_muted )
					snd_set_volume(Playing_messages[i].wave, (Master_sound_volume * aav_voice_volume));
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
void message_maybe_distort_text(char *text, int shipnum)
{
	int i, j, len, run, curr_offset, voice_duration, next_distort;

	if ( comm_between_player_and_ship(shipnum) == COMM_OK ) { 
		return;
	}

	len = strlen(text);
	if ( Message_wave_duration == 0 ) {
		next_distort = 5+myrand()%5;
		for ( i = 0; i < len; i++ ) {
			if ( i == next_distort ) {
				run = 3+myrand()%5;
				if ( i+run > len )
					run = len-i;
				for ( j = 0; j < run; j++) {
					text[i++] = '-';
					if ( i >= len )
						break;
				}
				next_distort = i + (5+myrand()%5);
			}
		}
		return;
	}

	voice_duration = Message_wave_duration;

	// distort text
	Distort_num = myrand()%MAX_DISTORT_PATTERNS;
	Distort_next = 0;
	curr_offset = 0;
	while (voice_duration > 0) {
		run = fl2i(Distort_patterns[Distort_num][Distort_next] * len);
		if (Distort_next & 1) {
			for ( i = curr_offset; i < MIN(len, curr_offset+run); i++ ) {
				if ( text[i] != ' ' ) 
					text[i] = '-';
			}
			curr_offset = i;
			if ( i >= len )
				break;
		} else {
			curr_offset += run;
		}

		voice_duration -= fl2i(Distort_patterns[Distort_num][Distort_next]*Message_wave_duration);
		Distort_next++;
		if ( Distort_next >= MAX_DISTORT_LEVELS )
			Distort_next = 0;
	};
	
	Distort_next = 0;
}

// return 1 if a talking head animation is playing, otherwise return 0
int message_anim_is_playing()
{
	int i;

	for (i = 0; i < Num_messages_playing; i++ ) {
		//if ( (Playing_messages[i].anim != NULL) && anim_playing(Playing_messages[i].anim) )
		if(Playing_messages[i].play_anim)
			return 1;
	}

	return 0;
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

bool add_message(char *name, char *message, int persona_index, int multi_team)
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

bool change_message(char *name, char *message, int persona_index, int multi_team)
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
 * Ideally, this would return the minimum of the comm state between the player and the other ship.  In practice, retail has no checks whatsoever on a ship's ability
 * to send messages unless that ship is the player, so such a change would require an AI profiles option and we must default to the player's state.  However, we
 * have a bit of wiggle room with COMM_SCRAMBLED, because EMP effects are either transient or set by the newly enhanced scramble-messages SEXP.  Thus any comm
 * dropout does not cause an unanticipated deviation in the mission design.
 */
int comm_between_player_and_ship(int other_shipnum)
{
	int player_comm_state = hud_communications_state(Player_ship);

	if (other_shipnum < 0)
		return player_comm_state;

	int other_comm_state = hud_communications_state(&Ships[other_shipnum]);

	/* here is where you would check the flag
	if (hypothetical_ai_profiles_flag)
	{
		return MIN(player_comm_state, other_comm_state);
	}
	else
	*/
	{
		if (player_comm_state == COMM_OK && other_comm_state == COMM_OK)
			return COMM_OK;
		else if (player_comm_state == COMM_SCRAMBLED || other_comm_state == COMM_SCRAMBLED)
			return COMM_SCRAMBLED;
		else
			return player_comm_state;
	}
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "missionui/missiondebrief.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionpause.h"
#include "mission/missionbriefcommon.h"
#include "mission/missiongoals.h"
#include "mission/missioncampaign.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "ui/uidefs.h"
#include "gamesnd/gamesnd.h"
#include "parse/parselo.h"
#include "sound/audiostr.h"
#include "io/timer.h"
#include "gamehelp/contexthelp.h"
#include "stats/stats.h"
#include "playerman/player.h"
#include "gamesnd/eventmusic.h"
#include "graphics/font.h"
#include "popup/popup.h"
#include "stats/medals.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "osapi/osapi.h"
#include "sound/fsspeech.h"
#include "globalincs/globals.h"
#include "ship/ship.h"
#include "cfile/cfile.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multiui.h"
#include "network/multi_pinfo.h"
#include "network/multi_kick.h"
#include "network/multi_campaign.h"
#include "network/multi_endgame.h"
#include "missionui/chatbox.h"


#define MAX_TOTAL_DEBRIEF_LINES	200

#define TEXT_TYPE_NORMAL			1
#define TEXT_TYPE_RECOMMENDATION	2

#define DEBRIEF_NUM_STATS_PAGES	4
#define DEBRIEF_MISSION_STATS		0
#define DEBRIEF_MISSION_KILLS		1
#define DEBRIEF_ALLTIME_STATS		2
#define DEBRIEF_ALLTIME_KILLS		3

// 3rd coord is max width in pixels
int Debrief_title_coords[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		18, 118, 174
	},
	{ // GR_1024
		28, 193, 280
	}
};

int Debrief_text_wnd_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		43, 140, 339, 303			
	},	
	{	// GR_1024
		69, 224, 535, 485		
	}
};

int Debrief_text_x2[GR_NUM_RESOLUTIONS] = {
	276,	// GR_640
	450	// GR_1024
};
	
int Debrief_stage_info_coords[GR_NUM_RESOLUTIONS][2] = {
	{	// GR_640
		379, 137		
	},	
	{	// GR_1024
		578, 224
	}
};

int Debrief_more_coords[GR_NUM_RESOLUTIONS][2] = {
	{	// GR_640
		323, 453	
	},
	{	// GR_1024
		323, 453	
	}
};

#define MULTI_LIST_TEAM_OFFSET					16		

int Debrief_multi_list_team_max_display[GR_NUM_RESOLUTIONS] = {
	9,	// GR_640
	12	// GR_1024
};

int Debrief_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		416, 280, 195, 101
	},
	{	// GR_1024
		666, 448, 312, 162
	}
};

int Debrief_award_wnd_coords[GR_NUM_RESOLUTIONS][2] = {
	{	// GR_640
		411, 126
	},
	{	// GR_1024
		658, 203	
	}
};


int Debrief_award_coords[GR_NUM_RESOLUTIONS][2] = {
	{	// GR_640
		416, 140
	},
	{	// GR_1024
		666, 224	
	}
};

// 0=x, 1=y, 2=width of the field
int Debrief_medal_text_coords[GR_NUM_RESOLUTIONS][3] = {
	{	// GR_640
		423, 247, 189
	},
	{	// GR_1024
		666, 333, 67
	}
};

// 0=x, 1=y, 2=height of the field
int Debrief_award_text_coords[GR_NUM_RESOLUTIONS][3] = {
	{	// GR_640
		416, 210, 42
	},
	{	// GR_1024
		666, 333, 67
	}
};

// 0 = with medal
// 1 = without medal (text will use medal space)
#define DB_WITH_MEDAL		0
#define DB_WITHOUT_MEDAL	1
int Debrief_award_text_width[GR_NUM_RESOLUTIONS][2] = {
	{	// GR_640
		123, 203
	},
	{	// GR_1024
		196, 312	
	}
};

char *Debrief_single_name[GR_NUM_RESOLUTIONS] = {
	"DebriefSingle",		// GR_640
	"2_DebriefSingle"		// GR_1024
};
char *Debrief_multi_name[GR_NUM_RESOLUTIONS] = {
	"DebriefMulti",		// GR_640
	"2_DebriefMulti"		// GR_1024
};
char *Debrief_mask_name[GR_NUM_RESOLUTIONS] = {
	"Debrief-m",			// GR_640
	"2_Debrief-m"			// GR_1024
};

#define NUM_BUTTONS	18
#define NUM_TABS		2

#define DEBRIEF_TAB				0
#define STATS_TAB					1
#define TEXT_SCROLL_UP			2
#define TEXT_SCROLL_DOWN		3
#define REPLAY_MISSION			4
#define RECOMMENDATIONS			5
#define FIRST_STAGE				6
#define PREV_STAGE				7
#define NEXT_STAGE				8
#define LAST_STAGE				9
#define MULTI_PINFO_POPUP		10
#define MULTI_KICK				11
#define MEDALS_BUTTON			12
#define PLAYER_SCROLL_UP		13
#define PLAYER_SCROLL_DOWN		14
#define HELP_BUTTON				15
#define OPTIONS_BUTTON			16
#define ACCEPT_BUTTON			17

#define REPEAT	1

//XSTR:OFF
char* Debrief_loading_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"PleaseWait",		// GR_640
	"2_PleaseWait"		// GR_1024
};

//XSTR:ON

typedef struct {
	char	text[NAME_LENGTH+1];	// name of ship type with a colon
	int	num;						// how many ships of this type player has killed
} debrief_stats_kill_info;

typedef struct {
	int net_player_index;	// index into Net_players[] array
	int rank_bitmap;			// bitmap id for rank
	char callsign[CALLSIGN_LEN];
} debrief_multi_list_info;

static ui_button_info Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("DB_00",		6,		1,		37,	7,		0),		// debriefing
		ui_button_info("DB_01",		6,		21,	37,	23,	1),		// statistics
		ui_button_info("DB_02",		1,		195,	-1,	-1,	2),		// scroll stats up
		ui_button_info("DB_03",		1,		236,	-1,	-1,	3),		// scroll stats down
		ui_button_info("DB_04",		1,		428,	49,	447,	4),		// replay mission
		ui_button_info("DB_05",		17,	459,	49,	464,	5),		// recommendations
		ui_button_info("DB_06",		323,	454,	-1,	-1,	6),		// first page
		ui_button_info("DB_07",		348,	454,	-1,	-1,	7),		// prev page
		ui_button_info("DB_08",		372,	454,	-1,	-1,	8),		// next page
		ui_button_info("DB_09",		396,	454,	-1,	-1,	9),		// last page
		ui_button_info("DB_10",		441,	384,	433,	413,	10),		// pilot info
		ui_button_info("DB_11",		511,	384,	510,	413,	11),		// kick
		ui_button_info("DB_12",		613,	226,	-1,	-1,	12),		// medals
		ui_button_info("DB_13",		615,	329,	-1,	-1,	13),		// scroll pilots up
		ui_button_info("DB_14",		615,	371,	-1,	-1,	14),		// scroll pilots down
		ui_button_info("DB_15",		538,	431,	500,	440,	15),		// help
		ui_button_info("DB_16",		538,	455,	479,	464,	16),		// options
		ui_button_info("DB_17",		573,	432,	572,	413,	17),		// accept
	},
	{ // GR_1024
		ui_button_info("2_DB_00",		10,	1,		59,	12,	0),		// debriefing
		ui_button_info("2_DB_01",		10,	33,	59,	37,	1),		// statistics
		ui_button_info("2_DB_02",		1,		312,	-1,	-1,	2),		// scroll stats up
		ui_button_info("2_DB_03",		1,		378,	-1,	-1,	3),		// scroll stats down
		ui_button_info("2_DB_04",		1,		685,	79,	715,	4),		// replay mission
		ui_button_info("2_DB_05",		28,	735,	79,	743,	5),		// recommendations
		ui_button_info("2_DB_06",		517,	726,	-1,	-1,	6),		// first page
		ui_button_info("2_DB_07",		556,	726,	-1,	-1,	7),		// prev page
		ui_button_info("2_DB_08",		595,	726,	-1,	-1,	8),		// next page
		ui_button_info("2_DB_09",		633,	726,	-1,	-1,	9),		// last page
		ui_button_info("2_DB_10",		706,	615,	700,	661,	10),		// pilot info
		ui_button_info("2_DB_11",		817,	615,	816,	661,	11),		// kick
		ui_button_info("2_DB_12",		981,	362,	-1,	-1,	12),		// medals
		ui_button_info("2_DB_13",		984,	526,	-1,	-1,	13),		// scroll pilots up
		ui_button_info("2_DB_14",		984,	594,	-1,	-1,	14),		// scroll pilots down
		ui_button_info("2_DB_15",		861,	689,	801,	705,	15),		// help
		ui_button_info("2_DB_16",		861,	728,	777,	744,	16),		// options
		ui_button_info("2_DB_17",		917,	692,	917,	692,	17),		// accept
	}
};

// text
#define NUM_DEBRIEF_TEXT				10
#define MP_TEXT_INDEX_1					4
#define MP_TEXT_INDEX_2					5
#define MP_TEXT_INDEX_3					6
UI_XSTR Debrief_strings[GR_NUM_RESOLUTIONS][NUM_DEBRIEF_TEXT] = {
	{ // GR_640
		{ "Debriefing",		804,		37,	7,		UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][DEBRIEF_TAB].button },
		{ "Statistics",		1333,		37,	26,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][STATS_TAB].button },
		{ "Replay Mission",	444,		49,	447,	UI_XSTR_COLOR_PINK,	-1,	&Buttons[0][REPLAY_MISSION].button },
		{ "Recommendations",	1334,		49,	464,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][RECOMMENDATIONS].button },
		{ "Pilot",				1310,		433,	413,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][MULTI_PINFO_POPUP].button },
		{ "Info",				1311,		433,	423,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][MULTI_PINFO_POPUP].button },
		{ "Kick",				1266,		510,	413,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][MULTI_KICK].button },
		{ "Help",				928,		500,	440,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][HELP_BUTTON].button },
		{ "Options",			1036,		479,	464,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[0][OPTIONS_BUTTON].button },
		{ "Accept",				1035,		572,	413,	UI_XSTR_COLOR_PINK,	-1,	&Buttons[0][ACCEPT_BUTTON].button },
	},
	{ // GR_1024
		{ "Debriefing",		804,		59,	12,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][DEBRIEF_TAB].button },
		{ "Statistics",		1333,		59,	47,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][STATS_TAB].button },
		{ "Replay Mission",	444,		79,	715,	UI_XSTR_COLOR_PINK,	-1,	&Buttons[1][REPLAY_MISSION].button },
		{ "Recommendations",	1334,		79,	743,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][RECOMMENDATIONS].button },
		{ "Pilot",				1310,		700,	661,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][MULTI_PINFO_POPUP].button },
		{ "Info",				1311,		700,	679,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][MULTI_PINFO_POPUP].button },
		{ "Kick",				1266,		816,	661,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][MULTI_KICK].button },
		{ "Help",				928,		801,	705,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][HELP_BUTTON].button },
		{ "Options",			1036,		780,	744,	UI_XSTR_COLOR_GREEN,	-1,	&Buttons[1][OPTIONS_BUTTON].button },
		{ "Accept",				1035,		917,	672,	UI_XSTR_COLOR_PINK,	-1,	&Buttons[1][ACCEPT_BUTTON].button },
	}
};


char Debrief_current_callsign[CALLSIGN_LEN+10];
player *Debrief_player;

static UI_WINDOW Debrief_ui_window;
static UI_BUTTON List_region;
static int Background_bitmap;					// bitmap for the background of the debriefing
static int Award_bg_bitmap;
static int Debrief_multi_loading_bitmap;
static int Rank_bitmap;
static int Medal_bitmap;
static int Badge_bitmap;

static int Promoted;
static int Debrief_accepted;
static int Turned_traitor;
static int Must_replay_mission;

static int Current_mode;
static int New_mode;
static int Recommend_active;
static int Award_active;
static int Text_offset;
static int Num_text_lines = 0;
static int Num_debrief_lines = 0;
//static int Num_normal_debrief_lines = 0;
static int Text_type[MAX_TOTAL_DEBRIEF_LINES];
static char *Text[MAX_TOTAL_DEBRIEF_LINES];

static int Debrief_inited = 0;
static int New_stage;
static int Current_stage;
static int Num_stages;
static int Num_debrief_stages;
static int Stage_voice = -1;
static int Debrief_music_timeout = 0;

static int Multi_list_size;
static int Multi_list_offset;

int Debrief_multi_stages_loaded = 0;
int Debrief_multi_voice_loaded = 0;

// static int Debrief_voice_ask_for_cd;

// voice id's for debriefing text
static int Debrief_voices[MAX_DEBRIEF_STAGES];

#define DEBRIEF_VOICE_DELAY 400				// time to delay voice playback when a new stage starts
static int Debrief_cue_voice;					// timestamp to cue the playback of the voice
static int Debrief_first_voice_flag = 1;	// used to delay the first voice playback extra long

static int Debriefing_paused = 0;

int Max_debrief_Lines;

// pointer used for getting to debriefing information
debriefing	Traitor_debriefing;				// used when player is a traitor

// pointers to the active stages for this debriefing
static debrief_stage *Debrief_stages[MAX_DEBRIEF_STAGES];
static debrief_stage Promotion_stage, Badge_stage;
static debrief_stats_kill_info *Debrief_stats_kills = NULL;
static debrief_multi_list_info Multi_list[MAX_PLAYERS];
int Multi_list_select;

// flag indicating if we should display info for the given player (in multiplayer)
int Debrief_should_show_popup = 1;

// already shown skip mission popup?
static int Debrief_skip_popup_already_shown = 0;

void debrief_text_init();
void debrief_accept(int ok_to_post_start_game_event = 1);
void debrief_kick_selected_player();


// promotion voice selection stuff
#define NUM_VOLITION_CAMPAIGNS	1
typedef struct {
	char  campaign_name[32];
	int	num_missions;
} v_campaign;

v_campaign Volition_campaigns[NUM_VOLITION_CAMPAIGNS] = {
	{
		BUILTIN_CAMPAIGN,		// the only campaign for now, but this leaves room for a mission pack
		35						// make sure this is equal to the  number of missions you gave in the corresponding Debrief_promotion_voice_mapping
	}
};


// data for which voice goes w/ which mission
typedef struct voice_map {
	char  mission_file[32];
	int	persona_index;
} voice_map;

voice_map Debrief_promotion_voice_mapping[NUM_VOLITION_CAMPAIGNS][MAX_CAMPAIGN_MISSIONS] = {
	{		// FreeSpace2 campaign 
		{ "SM1-01.fs2",			1 },
		{ "SM1-02.fs2",			1 },
		{ "SM1-03.fs2",			1 },
		{ "SM1-04.fs2",			2 },
		{ "SM1-05.fs2",			2 },
		{ "SM1-06.fs2",			2 },
		{ "SM1-07.fs2",			2 },
		{ "SM1-08.fs2",			3 },
		{ "SM1-09.fs2",			3 },
		{ "SM1-10.fs2",			3 },

		{ "SM2-01.fs2",			6 },
		{ "SM2-02.fs2",			6 },
		{ "SM2-03.fs2",			6 },
		{ "SM2-04.fs2",			7 },
		{ "SM2-05.fs2",			7 },
		{ "SM2-06.fs2",			7 },
		{ "SM2-07.fs2",			8 },
		{ "SM2-08.fs2",			8 },
		{ "SM2-09.fs2",			8 },
		{ "SM2-10.fs2",			8 },

		{ "SM3-01.fs2",			8 },
		{ "SM3-02.fs2",			8 },
		{ "SM3-03.fs2",			8 },
		{ "SM3-04.fs2",			8 },
		{ "SM3-05.fs2",			8 },
		{ "SM3-06.fs2",			9 },
		{ "SM3-07.fs2",			9 },
		{ "SM3-08.fs2",			9 },
		{ "SM3-09.fs2",			9 },
		{ "SM3-10.fs2",			9 },			// no debriefing for 3-10
		
		{ "loop1-1.fs2",			4 },
		{ "loop1-2.fs2",			4 },
		{ "loop1-3.fs2",			5 },
		{ "loop2-1.fs2",			4 },
		{ "loop2-2.fs2",			4 }
	}
};

static char* Debrief_award_background[GR_NUM_RESOLUTIONS] = {
	"DebriefAward",
	"2_DebriefAward"
};

#define AWARD_TEXT_MAX_LINES				5
#define AWARD_TEXT_MAX_LINE_LENGTH		128
char Debrief_award_text[AWARD_TEXT_MAX_LINES][AWARD_TEXT_MAX_LINE_LENGTH];
int Debrief_award_text_num_lines = 0;



// prototypes, you know you love 'em
void debrief_add_award_text(char *str);
void debrief_award_text_clear();



// functions
const char *debrief_tooltip_handler(const char *str)
{
	if (!stricmp(str, NOX("@.Medal"))) {
		if (Award_active){
			return XSTR( "Medal", 435);
		}

	} else if (!stricmp(str, NOX("@.Rank"))) {
		if (Award_active){
			return XSTR( "Rank", 436);
		}

	} else if (!stricmp(str, NOX("@.Badge"))) {
		if (Award_active){
			return XSTR( "Badge", 437);
		}

	} else if (!stricmp(str, NOX("@Medal"))) {
		if (Medal_bitmap >= 0){
			return Medals[Player->stats.m_medal_earned].name;
		}

	} else if (!stricmp(str, NOX("@Rank"))) {
		if (Rank_bitmap >= 0){
			return Ranks[Promoted].name;
		}

	} else if (!stricmp(str, NOX("@Badge"))) {
		if (Badge_bitmap >= 0){
			return Medals[Player->stats.m_badge_earned].name;
		}
	}

	return NULL;
}

// initialize the array of handles to the different voice streams
void debrief_voice_init()
{
	int i;

	for (i=0; i<MAX_DEBRIEF_STAGES; i++) {
		Debrief_voices[i] = -1;
	}
}

void debrief_load_voice_file(int voice_num, char *name)
{
	int load_attempts = 0;
	while(1) {

		if ( load_attempts++ > 5 ) {
			break;
		}

		Debrief_voices[voice_num] = audiostream_open( name, ASF_VOICE );
		if ( Debrief_voices[voice_num] >= 0 ) {
			break;
		}

		// Don't bother to ask for the CD in multiplayer
		if ( Game_mode & GM_MULTIPLAYER ) {
			break;
		}

		// couldn't load voice, ask user to insert CD (if necessary)

		// if ( Debrief_voice_ask_for_cd ) {
			// if ( game_do_cd_check() == 0 ) {
				// Debrief_voice_ask_for_cd = 0;
				// break;
			// }
		// }
	}
}

// open and pre-load the stream buffers for the different voice streams
void debrief_voice_load_all()
{
	int i;

	// Debrief_voice_ask_for_cd = 1;

	for ( i=0; i<Num_debrief_stages; i++ ) {
		if ( strlen(Debrief_stages[i]->voice) <= 0 ) {
			continue;
		}
		if ( strnicmp(Debrief_stages[i]->voice, NOX("none"), 4) ) {
			debrief_load_voice_file(i, Debrief_stages[i]->voice);
//			Debrief_voices[i] = audiostream_open(Debrief_stages[i]->voice, ASF_VOICE);
		}
	}
}

// close all the briefing voice streams
void debrief_voice_unload_all()
{
	int i;

	for ( i=0; i<MAX_DEBRIEF_STAGES; i++ ) {
		if ( Debrief_voices[i] != -1 ) {
			audiostream_close_file(Debrief_voices[i], 0);
			Debrief_voices[i] = -1;
		}
	}
}

// start playback of the voice for a particular briefing stage
void debrief_voice_play()
{
	if (!Briefing_voice_enabled || (Current_mode != DEBRIEF_TAB)){
		return;
	}

	// no more stages?  We are done then.
	if (Stage_voice >= Num_debrief_stages){
		return;
	}

	// if in delayed start, see if delay has elapsed and start voice if so
	if (Debrief_cue_voice) {
		if (!timestamp_elapsed(Debrief_cue_voice)){
			return;
		}

		Stage_voice++;  // move up to next voice
		if ((Stage_voice < Num_debrief_stages) && (Debrief_voices[Stage_voice] >= 0)) {
			audiostream_play(Debrief_voices[Stage_voice], Master_voice_volume, 0);
			Debrief_cue_voice = 0;  // indicate no longer in delayed start checking
		}

		return;
	}

	// see if voice is still playing.  If so, do nothing yet.
	if ((Stage_voice >= 0) && audiostream_is_playing(Debrief_voices[Stage_voice])){
		return;
	}

	// set voice to play in a little while from now.
	Debrief_cue_voice = timestamp(DEBRIEF_VOICE_DELAY);
}

// stop playback of the voice for a particular briefing stage
void debrief_voice_stop()
{
	if ((Stage_voice < 0) || (Stage_voice > Num_debrief_stages) || (Debrief_voices[Stage_voice] < 0))
		return;

	audiostream_stop(Debrief_voices[Stage_voice], 1, 0);  // stream is automatically rewound
	Stage_voice = -1;

	fsspeech_stop();
}

extern int Briefing_music_handle;

void debrief_pause()
{
	if (Debriefing_paused)
		return;

	Debriefing_paused = 1;

	if (Briefing_music_handle >= 0) {
		audiostream_pause(Briefing_music_handle);
	}

	if ((Stage_voice < 0) || (Stage_voice > Num_debrief_stages) || (Debrief_voices[Stage_voice] < 0))
		return;

	audiostream_pause(Debrief_voices[Stage_voice]);

	fsspeech_pause(true);
}

void debrief_unpause()
{
	if (!Debriefing_paused)
		return;

	Debriefing_paused = 1;

	if (Briefing_music_handle >= 0) {
		audiostream_unpause(Briefing_music_handle);
	}

	if ((Stage_voice < 0) || (Stage_voice > Num_debrief_stages) || (Debrief_voices[Stage_voice] < 0))
		return;

	audiostream_unpause(Debrief_voices[Stage_voice]);

	fsspeech_pause(false);
}

// function to deal with inserting possible promition and badge stages into the debriefing
// on the clients
void debrief_multi_fixup_stages()
{
	int i;

	// possibly insert the badge stage first, them the promotion stage since they are
	// inserted at the front of the debrief stages.
	if ( Badge_bitmap >= 0 ) {
		// move all stages forward one.  Don't 
		for ( i = Num_debrief_stages; i > 0; i-- ) {
			Debrief_stages[i] = Debrief_stages[i-1];
		}
		Debrief_stages[0] = &Badge_stage;
		Num_debrief_stages++;
	}

	if ( Promoted >= 0) {
		// move all stages forward one
		for ( i = Num_debrief_stages; i > 0; i-- ) {
			Debrief_stages[i] = Debrief_stages[i-1];
		}
		Debrief_stages[0] = &Promotion_stage;
		Num_debrief_stages++;
	}
}


// function called from multiplayer clients to set up the debriefing information for them
// (sent from the server).  
void debrief_set_multi_clients( int stage_count, int active_stages[] )
{
	int i;

	// set up the right briefing for this guy
	if(MULTI_TEAM){
		Debriefing = &Debriefings[Net_player->p_info.team];
	} else {
		Debriefing = &Debriefings[0];			
	}

	// see if this client was promoted -- if so, then add the first stage.
	Num_debrief_stages = 0;

	// set the pointers to the debriefings for this client
	for (i = 0; i < stage_count; i++) {
		Debrief_stages[Num_debrief_stages++] = &Debriefing->stages[active_stages[i]];
	}

	Debrief_multi_stages_loaded = 1;
}

// evaluate all stages for all teams.  Server of a multiplayer game will have to send that
// information to all clients after leaving this screen.
void debrief_multi_server_stuff()
{
	debriefing *debriefp;

	int stage_active[MAX_TVT_TEAMS][MAX_DEBRIEF_STAGES], *stages[MAX_TVT_TEAMS];
	int i, j, num_stages, stage_count[MAX_TVT_TEAMS];

	memset( stage_active, 0, sizeof(stage_active) );

	for (i=0; i<Num_teams; i++) {
		debriefp = &Debriefings[i];
		num_stages = 0;
		stages[i] = stage_active[i];
		for (j=0; j<debriefp->num_stages; j++) {
			if ( eval_sexp(debriefp->stages[j].formula) ) {
				stage_active[i][num_stages] = j;
				num_stages++;
			}
		}

		stage_count[i] = num_stages;
	}

	// if we're in campaign mode, evaluate campaign stuff
	if (Netgame.campaign_mode == MP_CAMPAIGN) {
		multi_campaign_eval_debrief();
	}

	// send the information to all clients.
	send_debrief_info( stage_count, stages );
}


// --------------------------------------------------------------------------------------
//	debrief_set_stages_and_multi_stuff()
//
// Set up the active stages for this debriefing
//
// returns:		number of active debriefing stages
//
int debrief_set_stages_and_multi_stuff()
{
	int i;
	debriefing	*debriefp;

	if ( MULTIPLAYER_CLIENT ) {
		return 0;
	}

	Num_debrief_stages = 0;

	if ( Game_mode & GM_MULTIPLAYER ) {
		debrief_multi_server_stuff();
	}

	// check to see if player is a traitor (looking at his team).  If so, use the special
	// traitor debriefing.  Only done in single player
	debriefp = Debriefing;
	if ( !(Game_mode & GM_MULTIPLAYER) ) {
		// although the no traitor flag check seems redundant it allows the mission designer to turn the traitor
		// debriefing off before the mission ends and supply one themselves 
		if ((Player_ship->team == Iff_traitor) && !(The_mission.flags & MISSION_FLAG_NO_TRAITOR))
			debriefp = &Traitor_debriefing;
	}

	Num_debrief_stages = 0;
	if (Promoted >= 0) {
		Debrief_stages[Num_debrief_stages++] = &Promotion_stage;
	}

	if (Badge_bitmap >= 0) {
		Debrief_stages[Num_debrief_stages++] = &Badge_stage;
	}

	for (i=0; i<debriefp->num_stages; i++) {
		if (eval_sexp(debriefp->stages[i].formula) == 1) {
			Debrief_stages[Num_debrief_stages++] = &debriefp->stages[i];
		}
	}

	return Num_debrief_stages;
}

// init the buttons that are specific to the debriefing screen
void debrief_buttons_init()
{
	ui_button_info *b;
	int i;

	for ( i=0; i<NUM_BUTTONS; i++ ) {
		b = &Buttons[gr_screen.res][i];
		b->button.create( &Debrief_ui_window, "", b->x, b->y, 60, 30, 0 /*b->flags & REPEAT*/, 1 );
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action( common_play_highlight_sound );
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add all xstrs
	for(i=0; i<NUM_DEBRIEF_TEXT; i++){
		// multiplayer specific text
		if((i == MP_TEXT_INDEX_1) || (i == MP_TEXT_INDEX_2) || (i == MP_TEXT_INDEX_3)){
			// only add if in multiplayer mode
			if(Game_mode & GM_MULTIPLAYER){
				Debrief_ui_window.add_XSTR(&Debrief_strings[gr_screen.res][i]);
			}
		} 
		// all other text
		else {
			Debrief_ui_window.add_XSTR(&Debrief_strings[gr_screen.res][i]);
		}
	}
	
	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	Buttons[gr_screen.res][NEXT_STAGE].button.set_hotkey(KEY_RIGHT);
	Buttons[gr_screen.res][PREV_STAGE].button.set_hotkey(KEY_LEFT);
	Buttons[gr_screen.res][LAST_STAGE].button.set_hotkey(KEY_SHIFTED | KEY_RIGHT);
	Buttons[gr_screen.res][FIRST_STAGE].button.set_hotkey(KEY_SHIFTED | KEY_LEFT);
	Buttons[gr_screen.res][TEXT_SCROLL_UP].button.set_hotkey(KEY_UP);
	Buttons[gr_screen.res][TEXT_SCROLL_DOWN].button.set_hotkey(KEY_DOWN);
	Buttons[gr_screen.res][ACCEPT_BUTTON].button.set_hotkey(KEY_CTRLED+KEY_ENTER);

	// if in multiplayer, disable the button for all players except the host
	// also disable for squad war matches
	if(Game_mode & GM_MULTIPLAYER){
		if((Netgame.type_flags & NG_TYPE_SW) || !(Net_player->flags & NETINFO_FLAG_GAME_HOST)){
			Buttons[gr_screen.res][REPLAY_MISSION].button.disable();
		}
	}
}

// --------------------------------------------------------------------------------------
//	debrief_ui_init()
//
void debrief_ui_init()
{
	// init ship selection masks and buttons
	common_set_interface_palette("DebriefPalette");		// set the interface palette
	Debrief_ui_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );
	Debrief_ui_window.set_mask_bmap(Debrief_mask_name[gr_screen.res]);
	Debrief_ui_window.tooltip_handler = debrief_tooltip_handler;
	debrief_buttons_init();

	// load in help overlay bitmap	
	help_overlay_load(DEBRIEFING_OVERLAY);
	help_overlay_set_state(DEBRIEFING_OVERLAY,0);

	if ( Game_mode & GM_MULTIPLAYER ) {
		// close down any old instances of the chatbox
		chatbox_close();

		// create the new one
		chatbox_create();
		Background_bitmap = bm_load(Debrief_multi_name[gr_screen.res]);
		List_region.create(&Debrief_ui_window, "", Debrief_list_coords[gr_screen.res][0], Debrief_list_coords[gr_screen.res][1], Debrief_list_coords[gr_screen.res][2], Debrief_list_coords[gr_screen.res][3], 0, 1);
		List_region.hide();

	} else {
		Background_bitmap = bm_load(Debrief_single_name[gr_screen.res]);
	}

	if ( Background_bitmap < 0 ) {
		Warning(LOCATION, "Could not load the background bitmap for debrief screen");
	}

	Award_bg_bitmap = bm_load(Debrief_award_background[gr_screen.res]);
	Debrief_multi_loading_bitmap = bm_load(Debrief_loading_bitmap_fname[gr_screen.res]);
}

// Goober5000
int debrief_find_persona_index()
{
	int i, j;

	if ((Campaign.current_mission >= 0) && (Campaign.missions[Campaign.current_mission].name) && (Campaign.filename))
	{
		// Goober5000 - first see if the campaign supplied a persona index
		// (0 means use the Volition default)
		if (Campaign.missions[Campaign.current_mission].debrief_persona_index != 0)
			return Campaign.missions[Campaign.current_mission].debrief_persona_index;

		// search through all official campaigns for our current campaign
		for (i = 0; i < NUM_VOLITION_CAMPAIGNS; i++)
		{
			if (!stricmp(Campaign.filename, Volition_campaigns[i].campaign_name))
			{
				// now search through the mission filenames
				for (j = 0; j < Volition_campaigns[i].num_missions; j++)
				{
					// found it!
					if (!stricmp(Campaign.missions[Campaign.current_mission].name, Debrief_promotion_voice_mapping[i][j].mission_file))
					{
						return Debrief_promotion_voice_mapping[i][j].persona_index;
					}
				}
			}
		}
	}

	// not found
	return -1;
}

// Goober5000
// V sez: "defaults to number 9 (Petrarch) for non-volition missions
// this is an ugly, nasty, hateful way of doing this, but it saves us changing the missions at this point"
void debrief_choose_voice(char *voice_dest, char *voice_base, int persona_index, int default_to_base = 0)
{
	if (persona_index >= 0)
	{
		// get voice file
		sprintf(voice_dest, NOX("%d_%s"), persona_index, voice_base);

		// if it exists, we're done
		if (cf_exists_full(voice_dest, CF_TYPE_VOICE_DEBRIEFINGS))
			return;
	}

	// that didn't work, so use the default

	// use the base file; don't prepend anything to it
	if (default_to_base)
	{
		strcpy(voice_dest, voice_base);
	}
	// default to Petrarch
	else
	{
		strcpy(voice_dest, "9_");
		strcpy(voice_dest + 2, voice_base);
	}
}

void debrief_choose_medal_variant(char *buf, int medal_earned, int zero_based_version_index)
{
	Assert(buf != NULL && medal_earned >= 0 && zero_based_version_index >= 0);

	// start with the regular file name, adapted for resolution
	sprintf(buf, NOX("%s%s"), Resolution_prefixes[gr_screen.res], Medals[medal_earned].debrief_bitmap);

	// if the medal has multiple versions, we may want to choose a specific bitmap
	char *p = strstr(buf, "##");
	if (p != NULL)
	{
		int version;
		char number[8];

		// possible to earn more than we have variants for
		if (zero_based_version_index >= Medals[medal_earned].num_versions)
			version = Medals[medal_earned].num_versions - 1;
		else
			version = zero_based_version_index;

		// also, this is dumb
		if (Medals[medal_earned].version_starts_at_1)
			version++;

		sprintf(number, NOX("%.2d"), version);
		Assert(strlen(number) == 2);
		strncpy(p, number, 2);
	}
}

void debrief_award_init()
{
	char buf[80];

	Rank_bitmap = -1; 
	Medal_bitmap = -1;
	Badge_bitmap = -1;
	Promoted = -1;

	// be sure there are no old award texts floating around
	debrief_award_text_clear();

	// handle medal earned
	if ( Player->stats.m_medal_earned != -1 ) {
		debrief_choose_medal_variant(buf, Player->stats.m_medal_earned, Player->stats.medal_counts[Player->stats.m_medal_earned] - 1);
		Medal_bitmap = bm_load(buf);

		debrief_add_award_text(Medals[Player->stats.m_medal_earned].name);
	}
	
	// handle promotions
	if ( Player->stats.m_promotion_earned != -1 ) {
		Promoted = Player->stats.m_promotion_earned;
		debrief_choose_medal_variant(buf, Rank_medal_index, Promoted);
		Rank_bitmap = bm_load(buf);

		// see if we have a persona
		int persona_index = debrief_find_persona_index();

		// use persona-specific promotion text if it exists; otherwise, use default
		if (Ranks[Promoted].promotion_text.find(persona_index) != Ranks[Promoted].promotion_text.end()) {
			Promotion_stage.text = Ranks[Promoted].promotion_text[persona_index];
		} else {
			Promotion_stage.text = Ranks[Promoted].promotion_text[-1];
		}
		Promotion_stage.recommendation_text = "";

		// choose appropriate promotion voice for this mission
		debrief_choose_voice(Promotion_stage.voice, Ranks[Promoted].promotion_voice_base, persona_index);

		debrief_add_award_text(Ranks[Promoted].name);
	}

	// handle badge earned
	// only grant badge if earned and allowed.  (no_promotion really means no promotion and no badges)
	if ( Player->stats.m_badge_earned != -1 ) {
		debrief_choose_medal_variant(buf, Player->stats.m_badge_earned, Player->stats.medal_counts[Player->stats.m_badge_earned] - 1);
		Badge_bitmap = bm_load(buf);

		// see if we have a persona
		int persona_index = debrief_find_persona_index();

		// use persona-specific badge text if it exists; otherwise, use default
		if (Medals[Player->stats.m_badge_earned].promotion_text.find(persona_index) != Medals[Player->stats.m_badge_earned].promotion_text.end()) {
			Badge_stage.text = Medals[Player->stats.m_badge_earned].promotion_text[persona_index];
		} else {
			Badge_stage.text = Medals[Player->stats.m_badge_earned].promotion_text[-1];
		}
		Badge_stage.recommendation_text = "";

		// choose appropriate badge voice for this mission
		debrief_choose_voice(Badge_stage.voice, Medals[Player->stats.m_badge_earned].voice_base, persona_index);

		debrief_add_award_text(Medals[Player->stats.m_badge_earned].name);
	}

	if ((Rank_bitmap >= 0) || (Medal_bitmap >= 0) || (Badge_bitmap >= 0)) {
		Award_active = 1;
	} else {
		Award_active = 0;
	}
}

// debrief_traitor_init() initializes local data which could be used if the player leaves the 
// mission a traitor.  The same debriefing always gets played
void debrief_traitor_init()
{
	static int inited = 0;

	if ( !inited ) {
		debriefing		*debrief;
		debrief_stage	*stagep;
		int rval;
		int stage_num;

		// open localization
		lcl_ext_open();

		if ((rval = setjmp(parse_abort)) != 0) {
			mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "traitor.tbl", rval));
			lcl_ext_close();
			return;
		}

		read_file_text("traitor.tbl", CF_TYPE_TABLES);
		reset_parse();		

		// simplied form of the debriefing stuff.
		debrief = &Traitor_debriefing;
		required_string("#Debriefing_info");

		required_string("$Num stages:");
		stuff_int(&debrief->num_stages);
		Assert(debrief->num_stages == 1);

		stage_num = 0;
		stagep = &debrief->stages[stage_num++];
		required_string("$Formula:");
		stagep->formula = get_sexp_main();
		required_string("$multi text");
		stuff_string( stagep->text, F_MULTITEXT, NULL);
		required_string("$Voice:");
		char traitor_voice_file[MAX_FILENAME_LEN];
		stuff_string(traitor_voice_file, F_FILESPEC, MAX_FILENAME_LEN);

// DKA 9/13/99	Only 1 traitor msg for FS2
//		if ( Player->main_hall ) {
//			strcpy_s(stagep->voice, NOX("3_"));
//		} else {
//			strcpy_s(stagep->voice, NOX("1_"));
//		}

		// Goober5000
		debrief_choose_voice(stagep->voice, traitor_voice_file, debrief_find_persona_index(), 1);

		required_string("$Recommendation text:");
		stuff_string( stagep->recommendation_text, F_MULTITEXT, NULL);

		inited = 1;

		// close localization
		lcl_ext_close();
	}

	// disable the accept button if in single player and I am a traitor
	Debrief_accepted = 0;
	Turned_traitor = Must_replay_mission = 0;
	if (!(Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_CAMPAIGN_MODE)) {
		if (Player_ship->team == Iff_traitor){
			Turned_traitor = 1;
		}

		if (Campaign.next_mission == Campaign.current_mission){
			Must_replay_mission = 1;
		}
	}

	if (Turned_traitor || Must_replay_mission) {
		Buttons[gr_screen.res][ACCEPT_BUTTON].button.hide();

		// kill off any stats
		Player->flags &= ~PLAYER_FLAGS_PROMOTED;
		scoring_level_init( &Player->stats );
	}
}

// initialization for listing of players in game
void debrief_multi_list_init()
{
	Multi_list_size = 0;  // number of net players to choose from
	Multi_list_offset = 0;

	Multi_list_select = -1;

	if ( !(Game_mode & GM_MULTIPLAYER) ) 
		return;

	debrief_rebuild_player_list();

	// switch stats display to this newly selected player
	set_player_stats(Multi_list[0].net_player_index);
	strcpy_s(Debrief_current_callsign, Multi_list[0].callsign);	
	Debrief_player = Player;
}

void debrief_multi_list_scroll_up()
{
	// if we're at the beginning of the list, don't do anything
	if(Multi_list_offset == 0){
		gamesnd_play_iface(SND_GENERAL_FAIL);
		return;
	}

	// otherwise scroll up
	Multi_list_offset--;
	gamesnd_play_iface(SND_USER_SELECT);
}

void debrief_multi_list_scroll_down()
{		
	// if we can scroll down no further
	if(Multi_list_size < Debrief_multi_list_team_max_display[gr_screen.res]){
		gamesnd_play_iface(SND_GENERAL_FAIL);
		return;
	}
	if((Multi_list_offset + Debrief_multi_list_team_max_display[gr_screen.res]) >= Multi_list_size){
		gamesnd_play_iface(SND_GENERAL_FAIL);
		return;
	}

	// otherwise scroll down
	Multi_list_offset++;
	gamesnd_play_iface(SND_USER_SELECT);
}

// draw the connected net players
void debrief_multi_list_draw()
{
	int y, z, font_height,idx;
	char str[CALLSIGN_LEN+5];
	net_player *np;
	
	font_height = gr_get_font_height();	

	// if we currently have no item picked, pick a reasonable one
	if((Multi_list_size >= 0) && (Multi_list_select == -1)){
		// select the entry which corresponds to the local player
		Multi_list_select = 0;				
		for(idx=0;idx<Multi_list_size;idx++){
			if(Multi_list[idx].net_player_index == MY_NET_PLAYER_NUM){
				Multi_list_select = idx;

				// switch stats display to this newly selected player
				set_player_stats(Multi_list[idx].net_player_index);
				strcpy_s(Debrief_current_callsign, Multi_list[idx].callsign);	
				Debrief_player = Net_players[Multi_list[idx].net_player_index].m_player;				
				break;
			}
		}
	}

	// draw the list itself
	y = 0;
	z = Multi_list_offset;
	while (y + font_height <= Debrief_list_coords[gr_screen.res][3]){
		np = &Net_players[Multi_list[z].net_player_index];

		if (z >= Multi_list_size){
			break;
		}
		// set the proper text color for the highlight
		if(np->flags & NETINFO_FLAG_GAME_HOST){
			if(Multi_list_select == z){
				gr_set_color_fast(&Color_text_active_hi);
			} else {
				gr_set_color_fast(&Color_bright);
			}
		} else {
			if(Multi_list_select == z){
				gr_set_color_fast(&Color_text_active);
			} else {
				gr_set_color_fast(&Color_text_normal);
			}
		}

		// blit the proper indicator - skipping observers
		if(!((np->flags & NETINFO_FLAG_OBSERVER) && !(np->flags & NETINFO_FLAG_OBS_PLAYER))){
			if(Netgame.type_flags & NG_TYPE_TEAM){
				// team 0
				if(np->p_info.team == 0){
					// draw his "selected" icon
					if(((np->state == NETPLAYER_STATE_DEBRIEF_ACCEPT) || (np->state == NETPLAYER_STATE_DEBRIEF_REPLAY)) && (Multi_common_icons[MICON_TEAM0_SELECT] != -1)){
						gr_set_bitmap(Multi_common_icons[MICON_TEAM0_SELECT]);
						gr_bitmap(Debrief_list_coords[gr_screen.res][0], Debrief_list_coords[gr_screen.res][1] + y - 2);
					} 
					// draw his "normal" icon
					else if(Multi_common_icons[MICON_TEAM0] != -1){
						gr_set_bitmap(Multi_common_icons[MICON_TEAM0]);
						gr_bitmap(Debrief_list_coords[gr_screen.res][0], Debrief_list_coords[gr_screen.res][1] + y - 2);
					}					
				} else if(np->p_info.team == 1){
					// draw his "selected" icon
					if(((np->state == NETPLAYER_STATE_DEBRIEF_ACCEPT) || (np->state == NETPLAYER_STATE_DEBRIEF_REPLAY)) && (Multi_common_icons[MICON_TEAM1_SELECT] != -1)){						
						gr_set_bitmap(Multi_common_icons[MICON_TEAM1_SELECT]);
						gr_bitmap(Debrief_list_coords[gr_screen.res][0], Debrief_list_coords[gr_screen.res][1] + y - 2);
					} 
					// draw his "normal" icon
					else if(Multi_common_icons[MICON_TEAM1] != -1){
						gr_set_bitmap(Multi_common_icons[MICON_TEAM1]);
						gr_bitmap(Debrief_list_coords[gr_screen.res][0], Debrief_list_coords[gr_screen.res][1] + y - 2);
					}					
				}
			} else {
				// draw the team 0 selected icon
				if(((np->state == NETPLAYER_STATE_DEBRIEF_ACCEPT) || (np->state == NETPLAYER_STATE_DEBRIEF_REPLAY)) && (Multi_common_icons[MICON_TEAM0_SELECT] != -1)){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM0_SELECT]);
					gr_bitmap(Debrief_list_coords[gr_screen.res][0], Debrief_list_coords[gr_screen.res][1] + y - 2);
				}
			}
		}

		strcpy_s(str,Multi_list[z].callsign);
		if(Net_players[Multi_list[z].net_player_index].flags & NETINFO_FLAG_OBSERVER && !(Net_players[Multi_list[z].net_player_index].flags & NETINFO_FLAG_OBS_PLAYER)){
			strcat_s(str,XSTR( "(O)", 438));
		}		

		// bli
		gr_string(Debrief_list_coords[gr_screen.res][0] + MULTI_LIST_TEAM_OFFSET, Debrief_list_coords[gr_screen.res][1] + y, str);

		y += font_height;
		z++;
	}
}

void debrief_kick_selected_player()
{
	if(Multi_list_select >= 0){
		Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);
		multi_kick_player(Multi_list[Multi_list_select].net_player_index);
	}
}


// get optional mission popup text 
void debrief_assemble_optional_mission_popup_text(char *buffer, char *mission_loop_desc)
{
	Assert(buffer != NULL);
	// base message

	if (mission_loop_desc == NULL) {
		strcpy(buffer, XSTR("<No Mission Loop Description Available>", 1490));
		mprintf(("No mission loop description available\n"));
	} else {
		strcpy(buffer, mission_loop_desc);
	}

	strcat(buffer, XSTR("\n\n\nDo you want to play the optional mission?", 1491));
}

// what to do when the accept button is hit
void debrief_accept(int ok_to_post_start_game_event)
{
	extern int Weapon_energy_cheat;
	int go_loop = 0;
	Weapon_energy_cheat=0;

	if ( (/*Cheats_enabled ||*/ Turned_traitor || Must_replay_mission) && (Game_mode & GM_CAMPAIGN_MODE) ) {
		const char *str;
		int z;

		if (Game_mode & GM_MULTIPLAYER) {
			return;
		}

		if (Player_ship->team == Iff_traitor){
			str = XSTR( "Your career is over, Traitor!  You can't accept new missions!", 439);
		}/* else if (Cheats_enabled) {
			str = XSTR( "You are a cheater.  You cannot accept this mission!", 440);
		}*/ else {
			str = XSTR( "You have failed this mission and cannot accept.  What do you you wish to do instead?", 441);
		}

		z = popup(0, 3, XSTR( "Return to &Debriefing", 442), XSTR( "Go to &Flight Deck", 443), XSTR( "&Replay Mission", 444), str);
		if (z == 2){
			gameseq_post_event(GS_EVENT_START_GAME);  // restart mission
		} else if ( z == 1 ) {
			gameseq_post_event(GS_EVENT_END_GAME);  // return to main hall, tossing stats
		}

		return;
	}

	Debrief_accepted = 1;
	// save mission stats
	if (Game_mode & GM_MULTIPLAYER) {
		// note that multi_debrief_accept_hit() will handle all mission_campaign_* calls on its own
		// as well as doing stats transfers, etc.
		multi_debrief_accept_hit();
	} else {
		int play_commit_sound = 1;
		// only write the player's stats if he's accepted

		// if we are just playing a single mission, then don't do many of the things
		// that need to be done.  Nothing much should happen when just playing a single
		// mission that isn't in a campaign.
		if ( Game_mode & GM_CAMPAIGN_MODE ) {

			// check for possible mission loop
			// check for (1) mission loop available, (2) don't have to repeat last mission
			if(!(Game_mode & GM_MULTIPLAYER)){
				int cur = Campaign.current_mission;
				bool require_repeat_mission = (Campaign.current_mission == Campaign.next_mission);
				if (Campaign.missions[cur].flags & CMISSION_FLAG_HAS_LOOP) {
					Assert(Campaign.loop_mission != CAMPAIGN_LOOP_MISSION_UNINITIALIZED);
				}

				if ( (Campaign.missions[cur].flags & CMISSION_FLAG_HAS_LOOP) && (Campaign.loop_mission != -1) && !require_repeat_mission ) {
					/*
					char buffer[512];
					debrief_assemble_optional_mission_popup_text(buffer, Campaign.missions[cur].mission_loop_desc);

					int choice = popup(0 , 2, POPUP_NO, POPUP_YES, buffer);
					if (choice == 1) {
						Campaign.loop_enabled = 1;
						Campaign.next_mission = Campaign.loop_mission;
					}
					*/
					go_loop = 1;
				}
			}			

			// loopy loopy time
			if (go_loop && ok_to_post_start_game_event) {
				gameseq_post_event( GS_EVENT_LOOP_BRIEF );
			}
			// continue as normal
			else {
				// end the mission
				// if we can loop, but don't want to right now, then setup so that we can later
				mission_campaign_mission_over( (go_loop) ? false : true );

				// check to see if we are out of the loop now
				if ( Campaign.next_mission == Campaign.loop_reentry ) {
					Campaign.loop_enabled = 0;
				}

				// check if campaign is over
				if ( Campaign.next_mission == -1 ) {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				} else {
					if ( ok_to_post_start_game_event ) {
						gameseq_post_event(GS_EVENT_START_GAME);
					} else {
						play_commit_sound = 0;
					}
				}
			}
		} else {
			gameseq_post_event(GS_EVENT_MAIN_MENU);
		}

		// Goober5000
		if ( play_commit_sound && !(The_mission.flags & MISSION_FLAG_TOGGLE_DEBRIEFING)) {
			gamesnd_play_iface(SND_COMMIT_PRESSED);
		}

		game_flush();
	}
}

void debrief_next_tab()
{
	New_mode = Current_mode + 1;
	if (New_mode >= NUM_TABS)
		New_mode = 0;
}

void debrief_prev_tab()
{
	New_mode = Current_mode - 1;
	if (New_mode < 0)
		New_mode = NUM_TABS - 1;
}

// --------------------------------------------------------------------------------------
//	debrief_next_stage()
//
void debrief_next_stage()
{
	if (Current_stage < Num_stages - 1) {
		New_stage = Current_stage + 1;
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG);

	} else
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG_FAIL);
}

// --------------------------------------------------------------------------------------
//	debrief_prev_stage()
//
void debrief_prev_stage()
{
	if (Current_stage) {
		New_stage = Current_stage - 1;
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG);

	} else
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG_FAIL);
}

// --------------------------------------------------------------------------------------
//	debrief_first_stage()
void debrief_first_stage()
{
	if (Current_stage) {
		New_stage = 0;
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG);

	} else
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG_FAIL);
}

// --------------------------------------------------------------------------------------
//	debrief_last_stage()
void debrief_last_stage()
{
	if (Current_stage != Num_stages - 1) {
		New_stage = Num_stages - 1;
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG);

	} else
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG_FAIL);
}

// draw what stage number the debriefing is on
void debrief_render_stagenum()
{
	int w;
	char buf[64];
	
	if (Num_stages < 2)
		return;
		
	sprintf(buf, XSTR( "%d of %d", 445), Current_stage + 1, Num_stages);
	gr_get_string_size(&w, NULL, buf);
	gr_set_color_fast(&Color_bright_blue);
	gr_string(Debrief_stage_info_coords[gr_screen.res][0] - w, Debrief_stage_info_coords[gr_screen.res][1], buf);
	gr_set_color_fast(&Color_white);
}

// render the mission difficulty at the specified y location
void debrief_render_mission_difficulty(int y_loc)
{	
	gr_string(0, y_loc, XSTR( "Skill Level", 1509));
	gr_string(Debrief_text_x2[gr_screen.res], y_loc, Skill_level_names(Game_skill_level));	
}

// render the mission time at the specified y location
void debrief_render_mission_time(int y_loc)
{
	char time_str[30];
	
	game_format_time(Missiontime, time_str);
	gr_string(0, y_loc, XSTR( "Mission Time", 446));
	gr_string(Debrief_text_x2[gr_screen.res], y_loc, time_str);	
}

// render out the debriefing text to the scroll window
void debrief_render()
{
	int y, z, font_height;

	if ( Num_stages <= 0 )
		return;

	font_height = gr_get_font_height();

	gr_set_clip(Debrief_text_wnd_coords[gr_screen.res][0], Debrief_text_wnd_coords[gr_screen.res][1], Debrief_text_wnd_coords[gr_screen.res][2], Debrief_text_wnd_coords[gr_screen.res][3]);
	y = 0;
	z = Text_offset;
	while (y + font_height <= Debrief_text_wnd_coords[gr_screen.res][3]) {
		if (z >= Num_text_lines)
			break;

		if (Text_type[z] == TEXT_TYPE_NORMAL)
			gr_set_color_fast(&Color_white);
		else
			gr_set_color_fast(&Color_bright_red);

		if (Text[z])
			gr_string(0, y, Text[z]);

		y += font_height;
		z++;
	}

	gr_reset_clip();
}

// render out the stats info to the scroll window
//
void debrief_stats_render()
{	
	int i, y, font_height;	

	gr_set_color_fast(&Color_blue);
	gr_set_clip(Debrief_text_wnd_coords[gr_screen.res][0], Debrief_text_wnd_coords[gr_screen.res][1], Debrief_text_wnd_coords[gr_screen.res][2], Debrief_text_wnd_coords[gr_screen.res][3]);
	gr_string(0, 0, Debrief_current_callsign);
	font_height = gr_get_font_height();
	y = 30;
	
	gr_set_color_fast(&Color_white);
	
	debrief_render_mission_difficulty(y);
	y += 2*font_height;
	
	switch ( Current_stage ) {
		case DEBRIEF_MISSION_STATS:
			i = Current_stage - 1;
			if ( i < 0 )
				i = 0;

			// display mission completion time
			debrief_render_mission_time(y);

			y += 2*font_height;
			show_stats_label(i, 0, y, font_height);
			show_stats_numbers(i, Debrief_text_x2[gr_screen.res], y, font_height);
			break;
		case DEBRIEF_ALLTIME_STATS:
			i = Current_stage - 1;
			if ( i < 0 )
				i = 0;

			show_stats_label(i, 0, y, font_height);
			show_stats_numbers(i, Debrief_text_x2[gr_screen.res], y, font_height);
			break;

		case DEBRIEF_ALLTIME_KILLS:
		case DEBRIEF_MISSION_KILLS:
			i = Text_offset;
			while (y + font_height <= Debrief_text_wnd_coords[gr_screen.res][3]) {
				if (i >= Num_text_lines)
					break;

				if (!i) {
					if ( Current_stage == DEBRIEF_MISSION_KILLS )
						gr_printf(0, y, XSTR( "Mission Kills by Ship Type", 447));
					else
						gr_printf(0, y, XSTR( "All-time Kills by Ship Type", 448));

				} else if (i > 1) {
					//Assert: Was debrief_setup_ship_kill_stats called?
					Assert(Debrief_stats_kills != NULL);

					gr_printf(0, y, "%s", Debrief_stats_kills[i - 2].text);
					gr_printf(Debrief_text_x2[gr_screen.res], y, "%d", Debrief_stats_kills[i - 2].num);
				}

				y += font_height;
				i++;
			}

			if (Num_text_lines == 2) {
				if ( Current_stage == DEBRIEF_MISSION_KILLS )
					gr_printf(0, y, XSTR( "(No ship kills this mission)", 449));
				else
					gr_printf(0, y, XSTR( "(No ship kills)", 450));
			}

			break;

		default:
			Int3();
			break;
	} 

	gr_reset_clip();
}

// do action for when the replay button is pressed
void debrief_replay_pressed()
{	
	if (!Turned_traitor && !Must_replay_mission && (Game_mode & GM_CAMPAIGN_MODE)) {
		int choice;
		choice = popup(0, 2, POPUP_CANCEL, XSTR( "&Replay", 451), XSTR( "If you choose to replay this mission, you will be required to complete it again before proceeding to future missions.\n\nIn addition, any statistics gathered during this mission will be discarded if you choose to replay.", 452));

		if (choice != 1){
			return;
		}
	}

	gameseq_post_event(GS_EVENT_START_GAME);	// restart mission
	gamesnd_play_iface(SND_COMMIT_PRESSED);
}

// -------------------------------------------------------------------
// debrief_redraw_pressed_buttons()
//
// Redraw any debriefing buttons that are pressed down.  This function is needed
// since we sometimes need to draw pressed buttons last to ensure the entire
// button gets drawn (and not overlapped by other buttons)
//
void debrief_redraw_pressed_buttons()
{
	int i;
	UI_BUTTON *b;
	
	for ( i=0; i<NUM_BUTTONS; i++ ) {
		b = &Buttons[gr_screen.res][i].button;
		// don't draw the recommendations button if we're in stats mode
		if ( b->button_down() ) {
			b->draw_forced(2);
		}
	}
}

// debrief specific button with hotspot 'i' has been pressed, so perform the associated action
//
void debrief_button_pressed(int num)
{
	switch (num) {
		case DEBRIEF_TAB:
			Buttons[gr_screen.res][RECOMMENDATIONS].button.enable();			
			// Debrief_ui_window.use_hack_to_get_around_stupid_problem_flag = 0;
			if (num != Current_mode){
				gamesnd_play_iface(SND_SCREEN_MODE_PRESSED);
			}
			New_mode = num;
			break;
		case STATS_TAB:
			// Debrief_ui_window.use_hack_to_get_around_stupid_problem_flag = 1;			// allows failure sound to be played
			Buttons[gr_screen.res][RECOMMENDATIONS].button.disable();			
			if (num != Current_mode){
				gamesnd_play_iface(SND_SCREEN_MODE_PRESSED);
			}
			New_mode = num;
			break;

		case TEXT_SCROLL_UP:
			if (Text_offset) {
				Text_offset--;
				gamesnd_play_iface(SND_SCROLL);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
			break;

		case TEXT_SCROLL_DOWN:
			if (Max_debrief_Lines < Num_text_lines) {
				Text_offset++;
				gamesnd_play_iface(SND_SCROLL);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
			break;

		case REPLAY_MISSION:
			if(Game_mode & GM_MULTIPLAYER){
				multi_debrief_replay_hit();
			} else {			
				debrief_replay_pressed();	
			}
			break;

		case RECOMMENDATIONS:
			gamesnd_play_iface(SND_USER_SELECT);
			Recommend_active = !Recommend_active;
			debrief_text_init();
			break;

		case FIRST_STAGE:
			debrief_first_stage();
			break;

		case PREV_STAGE:
			debrief_prev_stage();
			break;

		case NEXT_STAGE:
			debrief_next_stage();
			break;

		case LAST_STAGE:
			debrief_last_stage();
			break;

		case HELP_BUTTON:
			gamesnd_play_iface(SND_HELP_PRESSED);
			launch_context_help();
			break;

		case OPTIONS_BUTTON:
			gamesnd_play_iface(SND_SWITCH_SCREENS);
			gameseq_post_event( GS_EVENT_OPTIONS_MENU );
			break;

		case ACCEPT_BUTTON:
			debrief_accept();
			break;

		case MEDALS_BUTTON:
			gamesnd_play_iface(SND_SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_VIEW_MEDALS);
			break;

		case PLAYER_SCROLL_UP:
			debrief_multi_list_scroll_up();
			break;

		case PLAYER_SCROLL_DOWN:
			debrief_multi_list_scroll_down();
			break;

		case MULTI_PINFO_POPUP:
			Debrief_should_show_popup = 1;
			break;

		case MULTI_KICK:
			debrief_kick_selected_player();
			break;
	} // end swtich
}

void debrief_setup_ship_kill_stats(int stage_num)
{
	int i;
	//ushort *kill_arr;
	int *kill_arr;	//DTP max ships
	debrief_stats_kill_info	*kill_info;

	Assert(Current_stage < DEBRIEF_NUM_STATS_PAGES);
	if ( Current_stage == DEBRIEF_MISSION_STATS || Current_stage == DEBRIEF_ALLTIME_STATS )
		return;

	if(Debrief_stats_kills == NULL)
	{
		Debrief_stats_kills = new debrief_stats_kill_info[Num_ship_classes];
	}

	Assert(Debrief_player != NULL);

	// kill_ar points to an array of MAX_SHIP_TYPE ints
	if ( Current_stage == DEBRIEF_MISSION_KILLS ) {
		kill_arr = Debrief_player->stats.m_okKills;
	} else {		
		kill_arr = Debrief_player->stats.kills;
	}

	Num_text_lines = 0;
	for ( i=0; i<MAX_SHIP_CLASSES; i++ ) {

		// code used to add in mission kills, but the new system assumes that the player will accept, so
		// all time stats already have mission stats added in.
		if ( kill_arr[i] <= 0 ){
			continue;
		}


		kill_info = &Debrief_stats_kills[Num_text_lines++];

		kill_info->num = kill_arr[i];

		strcpy_s(kill_info->text, Ship_info[i].name);
		strcat_s(kill_info->text, NOX(":"));
	}

	Num_text_lines += 2;
}

// Iterate through the debriefing buttons, checking if they are pressed
void debrief_check_buttons()
{
	int i;

	for ( i=0; i<NUM_BUTTONS; i++ ) {
		if ( Buttons[gr_screen.res][i].button.pressed() ) {
			debrief_button_pressed(i);
		}
	}

	int y, z;

	if ( !(Game_mode & GM_MULTIPLAYER) ) 
		return;

	if (List_region.pressed()) {
		List_region.get_mouse_pos(NULL, &y);
		z = Multi_list_offset + y / gr_get_font_height();
		if ((z >= 0) && (z < Multi_list_size)) {
			// switch stats display to this newly selected player
			set_player_stats(Multi_list[z].net_player_index);
			strcpy_s(Debrief_current_callsign, Multi_list[z].callsign);
			Debrief_player = Net_players[Multi_list[z].net_player_index].m_player;
			Multi_list_select = z;
			debrief_setup_ship_kill_stats(Current_stage);
			gamesnd_play_iface(SND_USER_SELECT);			
		}
	}	

	// if the player was double clicked on - we should popup a player info popup
	/*
	if (List_region.double_clicked()) {
		Debrief_should_show_popup = 1;
	}
	*/
}

void debrief_text_stage_init(const char *src, int type)
{
	int i, n_lines, n_chars[MAX_DEBRIEF_LINES];
	char line[MAX_DEBRIEF_LINE_LEN];
	const char *p_str[MAX_DEBRIEF_LINES];

	n_lines = split_str(src, Debrief_text_wnd_coords[gr_screen.res][2], n_chars, p_str, MAX_DEBRIEF_LINES);
	Assert(n_lines >= 0);

	// if you hit this, you proba	
	if(n_lines >= MAX_DEBRIEF_LINES){
		Warning(LOCATION, "You have come close to the limit of debriefing lines, try adding more stages");	
	}

	for ( i=0; i<n_lines; i++ ) {
		Assert(n_chars[i] < MAX_DEBRIEF_LINE_LEN);
		Assert(Num_text_lines < MAX_TOTAL_DEBRIEF_LINES);
		strncpy(line, p_str[i], n_chars[i]);
		line[n_chars[i]] = 0;
		drop_white_space(line);
		Text_type[Num_text_lines] = type;
		Text[Num_text_lines++] = vm_strdup(line);
	}

	return;
}

void debrief_free_text()
{
	int i;

	for (i=0; i<Num_debrief_lines; i++)
		if (Text[i])
			vm_free(Text[i]);

	Num_debrief_lines = 0;
}

// setup the debriefing text lines for rendering
void debrief_text_init()
{
	int r_count = 0;
	const char *src;
	int i;

	// If no wav files are being used use speech simulation
	bool use_sim_speech = true;
	for (i = 0; i < MAX_DEBRIEF_STAGES; i++) {
		if(Debrief_voices[i] != -1) {
		 	use_sim_speech = false;
			break;
		}
	}

	// release old text lines first
	debrief_free_text();
	Num_text_lines = Text_offset = 0;

	fsspeech_start_buffer();

	if (Current_mode == DEBRIEF_TAB) {
		for (i=0; i<Num_debrief_stages; i++) {
			if (i)
				Text[Num_text_lines++] = NULL;  // add a blank line between stages

			src = Debrief_stages[i]->text.c_str();

			if (*src) {
				debrief_text_stage_init(src, TEXT_TYPE_NORMAL);

				if (use_sim_speech && !Recommend_active) {
					fsspeech_stuff_buffer(src);
					fsspeech_stuff_buffer("\n");
				}
			}

			if (Recommend_active) {
				src = Debrief_stages[i]->recommendation_text.c_str();

				if ((i == Num_debrief_stages - 1) && !r_count && !*src)
					src = XSTR( "We have no recommendations for you.", 1054);

				if (*src) {
					Text[Num_text_lines++] = NULL;
					debrief_text_stage_init(src, TEXT_TYPE_RECOMMENDATION);
					r_count++;

					if (use_sim_speech) {
						fsspeech_stuff_buffer(src);
						fsspeech_stuff_buffer("\n");
					}
				}
			}
		}

		Num_debrief_lines = Num_text_lines;
		if(use_sim_speech) {
			fsspeech_play_buffer(FSSPEECH_FROM_BRIEFING);
		}
		return;
	}

	// not in debriefing mode, must be in stats mode
	Num_text_lines = 0;
	debrief_setup_ship_kill_stats(Current_stage);
}


// --------------------------------------------------------------------------------------
//

// start up the appropriate music
static void debrief_init_music()
{
	int score = SCORE_DEBRIEF_AVERAGE;

	Debrief_music_timeout = 0;

	if ( (Game_mode & GM_CAMPAIGN_MODE) && (Campaign.next_mission == Campaign.current_mission) ) {
		// you failed the mission, so you get the fail music
		score = SCORE_DEBRIEF_FAIL;
	} else if ( mission_goals_met() ) {
		// you completed all primaries and secondaries, so you get the win music
		score = SCORE_DEBRIEF_SUCCESS;
	} else {
		// you somehow passed the mission, so you get a little something for your efforts.
		score = SCORE_DEBRIEF_AVERAGE;
	}

	// if multi client then give a slight delay before playing average music
	// since we'd like to eval the goals once more for slow clients
	if ( MULTIPLAYER_CLIENT && (score == SCORE_DEBRIEF_AVERAGE) ) {
		Debrief_music_timeout = timestamp(2000);
		return;
	}

	common_music_init(score);
}

void debrief_init()
{
	int i;

	Assert(!Debrief_inited);
//	Campaign.loop_enabled = 0;
	Campaign.loop_mission = CAMPAIGN_LOOP_MISSION_UNINITIALIZED;

	// set up the right briefing for this guy
	if(MULTI_TEAM){
		Debriefing = &Debriefings[Net_player->p_info.team];
	} else {
		Debriefing = &Debriefings[0];			
	}

	// Goober5000 - replace any variables with their values
	for (i = 0; i < Debriefing->num_stages; i++) {
		sexp_replace_variable_names_with_values(Debriefing->stages[i].text);
		sexp_replace_variable_names_with_values(Debriefing->stages[i].recommendation_text);
	}

	// no longer is mission
	Game_mode &= ~(GM_IN_MISSION);	

	game_flush();
	Current_mode = -1;
	New_mode = DEBRIEF_TAB;
	Recommend_active = Award_active = 0;

	Current_stage = -1;
	New_stage = 0;
	Debrief_cue_voice = 0;
	Num_text_lines = Num_debrief_lines = 0;
	Debrief_first_voice_flag = 1;

	Debrief_multi_voice_loaded = 0;

	if ( (Game_mode & GM_CAMPAIGN_MODE) && ( !MULTIPLAYER_CLIENT )	) {
		// MUST store goals and events first - may be used to evaluate next mission
		// store goals and events
		mission_campaign_store_goals_and_events_and_variables();

		// evaluate next mission
		mission_campaign_eval_next_mission();
	}

	// call traitor init before calling scoring_level_close.  traitor init will essentially nullify
	// any stats
	if ( !(Game_mode & GM_MULTIPLAYER) ) {	// only do for single player
		debrief_traitor_init();					// initialize data needed if player becomes traitor.
	}

	// call scoring level close for my stats.  Needed for award_init.  The stats will
	// be backed out if used chooses to replace them.
	scoring_level_close();

	debrief_ui_init();  // init UI items
	debrief_award_init();
	show_stats_init();
	debrief_voice_init();

	debrief_multi_list_init();

//	rank_bitmaps_clear();
//	rank_bitmaps_load();

	strcpy_s(Debrief_current_callsign, Player->callsign);
	Debrief_player = Player;
//	Debrief_current_net_player_index = debrief_multi_list[0].net_player_index;

	// set up the Debrief_stages[] and Recommendations[] arrays.  Only do the following stuff
	// for non-clients (i.e. single and game server).  Multiplayer clients will get their debriefing
	// info directly from the server.
	if ( !MULTIPLAYER_CLIENT ) {
		debrief_set_stages_and_multi_stuff();

		if ( Num_debrief_stages <= 0 ) {
			Num_debrief_stages = 0;
		} else {
			debrief_voice_load_all();
		}
	} else {
		// multiplayer client may have already received their debriefing info.  If they have not,
		// then set the num debrief stages to 0
		if ( !Debrief_multi_stages_loaded ) {
			Num_debrief_stages = 0;
		}
	}

	/*
	if (mission_evaluate_primary_goals() == PRIMARY_GOALS_COMPLETE) {
		common_music_init(SCORE_DEBRIEF_SUCCESS);
	} else {
		common_music_init(SCORE_DEBRIEF_FAIL);
	}
	*/

	// start up the appropriate music
	debrief_init_music();

	if (Game_mode & GM_MULTIPLAYER) {
		multi_debrief_init();

		// if i'm not the host of the game, disable the multi kick button
		if (!(Net_player->flags & NETINFO_FLAG_GAME_HOST)) {
			Buttons[gr_screen.res][MULTI_KICK].button.disable();
		}
	} else {
		Buttons[gr_screen.res][PLAYER_SCROLL_UP].button.disable();
		Buttons[gr_screen.res][PLAYER_SCROLL_DOWN].button.disable();
		Buttons[gr_screen.res][MULTI_PINFO_POPUP].button.disable();
		Buttons[gr_screen.res][MULTI_KICK].button.disable();
		Buttons[gr_screen.res][PLAYER_SCROLL_UP].button.hide();
		Buttons[gr_screen.res][PLAYER_SCROLL_DOWN].button.hide();
		Buttons[gr_screen.res][MULTI_PINFO_POPUP].button.hide();		
		Buttons[gr_screen.res][MULTI_KICK].button.hide();
	}

	if (!Award_active) {
		Buttons[gr_screen.res][MEDALS_BUTTON].button.disable();
		Buttons[gr_screen.res][MEDALS_BUTTON].button.hide();
	}

	Debrief_skip_popup_already_shown = 0;

	Debrief_inited = 1;
}

// --------------------------------------------------------------------------------------
//	debrief_close()
void debrief_close()
{
	int i;

	Assert(Debrief_inited);

	// if the mission wasn't accepted, clear out my stats
	// we need to evaluate a little differently for multiplayer since the conditions for "accepting" 
	// are a little bit different
	if (Game_mode & GM_MULTIPLAYER) {
		// if stats weren't accepted, backout my own stats
		if (multi_debrief_stats_accept_code() != 1) {
			if(MULTIPLAYER_MASTER){
				for(i=0; i<MAX_PLAYERS; i++){
					if(MULTI_CONNECTED(Net_players[i]) && !MULTI_STANDALONE(Net_players[i]) && !MULTI_PERM_OBSERVER(Net_players[i]) && (Net_players[i].m_player != NULL)){
						scoring_backout_accept(&Net_players[i].m_player->stats);
					}
				}
			} else {
				scoring_backout_accept( &Player->stats );
			}
		}
	} else {
		// single player
		if( !Debrief_accepted || !(Game_mode & GM_CAMPAIGN_MODE) ){
			// Make sure we don't skip any missions in a campaign.
			if ( Game_mode & GM_CAMPAIGN_MODE ) {
				Campaign.next_mission = Campaign.current_mission;
			}
			scoring_backout_accept( &Player->stats );
		}
	}

	// if dude passed the misson and accepted, reset his show skip popup flag
	if (Debrief_accepted) {
		Player->show_skip_popup = 1;
	}

	if (Num_debrief_lines) {
		for (i=0; i<Num_debrief_lines; i++){
			if (Text[i]){
				vm_free(Text[i]);
			}
		}
	}

	// clear out debrief info parsed from mission file - taylor
	mission_debrief_common_reset();

	// unload the overlay bitmap
//	help_overlay_unload(DEBRIEFING_OVERLAY);

	// clear out award text 
	Debrief_award_text_num_lines = 0;

	debrief_voice_unload_all();
	common_music_close();
	chatbox_close();

	// unload bitmaps
	if (Background_bitmap >= 0){
		bm_release(Background_bitmap);
	}

	if (Award_bg_bitmap >= 0){
		bm_release(Award_bg_bitmap);
	}

	if (Rank_bitmap >= 0){
		bm_release(Rank_bitmap);
	}

	if (Medal_bitmap >= 0){
		bm_release(Medal_bitmap);
	}

	if (Badge_bitmap >= 0){
		bm_release(Badge_bitmap);
	}

	Debrief_ui_window.destroy();
	common_free_interface_palette();		// restore game palette
	show_stats_close();

	if (Game_mode & GM_MULTIPLAYER){
		multi_debrief_close();
	}

	if(Debrief_stats_kills != NULL)
	{
		delete[] Debrief_stats_kills;
		Debrief_stats_kills = NULL;
	}
	game_flush();

	Stage_voice = -1;

	Debriefing_paused = 0;

	Debrief_inited = 0;
}

// handle keypresses in debriefing
void debrief_do_keys(int new_k)
{
	switch (new_k) {
		case KEY_TAB:
			debrief_next_tab();
			break;

		case KEY_SHIFTED | KEY_TAB:
			debrief_prev_tab();
			break;

		case KEY_ESC: {
			int pf_flags;
			int choice;

			// multiplayer accept popup is a little bit different
			if (Game_mode & GM_MULTIPLAYER) {		
				multi_debrief_esc_hit();
			} else {
				// display the normal debrief popup
				if (!Turned_traitor && !Must_replay_mission && (Game_mode & GM_CAMPAIGN_MODE)) {
					pf_flags = PF_BODY_BIG; // | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON;
					choice = popup(pf_flags, 3, POPUP_CANCEL, XSTR( "&Yes", 454), XSTR( "&No, retry later", 455), XSTR( "Accept this mission outcome?", 456));
					if (choice == 1) {  // accept and continue on
						debrief_accept(0);
						gameseq_post_event(GS_EVENT_MAIN_MENU);
					}

					if (choice < 1)
						break;

				} else if ( ( Turned_traitor || Must_replay_mission ) && (Game_mode & GM_CAMPAIGN_MODE)) {
					// need to popup saying that mission was a failure and must be replayed
					choice = popup(0, 2, POPUP_NO, POPUP_YES, XSTR( "Because this mission was a failure, you must replay this mission when you continue your campaign.\n\nReturn to the Flight Deck?", 457));
					if (choice <= 0) {
						break;
					}
				}

				// Return to Main Hall
				gameseq_post_event(GS_EVENT_END_GAME);
			}
		}

		default:
			break;
	}	// end switch
}

// uuuuuugly
void debrief_draw_award_text()
{
	int start_y, curr_y, i, x, sw;
	int fh = gr_get_font_height();
	int field_width = (Medal_bitmap > 0) ? Debrief_award_text_width[gr_screen.res][DB_WITH_MEDAL] : Debrief_award_text_width[gr_screen.res][DB_WITHOUT_MEDAL];

	// vertically centered within field
	start_y = Debrief_award_text_coords[gr_screen.res][1] + ((Debrief_award_text_coords[gr_screen.res][2] - (fh * Debrief_award_text_num_lines)) / 2);
	curr_y = start_y;

	// draw the strings
	for (i=0; i<Debrief_award_text_num_lines; i++) {
		gr_get_string_size(&sw, NULL, Debrief_award_text[i]);
		x = (Medal_bitmap < 0) ? (Debrief_award_text_coords[gr_screen.res][0] + (field_width - sw) / 2) : Debrief_award_text_coords[gr_screen.res][0];
		if (i==AWARD_TEXT_MAX_LINES-1) x += 7;				// hack because of the shape of the box
		gr_set_color_fast(&Color_white);
		gr_string(x, curr_y, Debrief_award_text[i]);

		// adjust y pos, including a little extra between the "pairs"
		curr_y += fh;
		if ((i == 1) || (i == 3)) { 
			curr_y += ((gr_screen.res == GR_640) ? 2 : 6);
		}
	}
}

// clears out text array so we don't have old award text showing up on new awards.
void debrief_award_text_clear() {
	int i;
	
	Debrief_award_text_num_lines = 0;
	for (i=0; i<AWARD_TEXT_MAX_LINES; i++) {
		//Debrief_award_text[i][0] = 0;
		memset(Debrief_award_text[i], 0, sizeof(char)*AWARD_TEXT_MAX_LINE_LENGTH);
	}
}

// this is the nastiest code I have ever written.  if you are modifying this, i feel bad for you.
void debrief_add_award_text(char *str)
{
	Assert(Debrief_award_text_num_lines < AWARD_TEXT_MAX_LINES);
	if (Debrief_award_text_num_lines >= AWARD_TEXT_MAX_LINES) {
		return;
	}

	char *line2;
	int field_width = (Medal_bitmap > 0) ? Debrief_award_text_width[gr_screen.res][DB_WITH_MEDAL] : Debrief_award_text_width[gr_screen.res][DB_WITHOUT_MEDAL];

	// copy in the line
	strcpy_s(Debrief_award_text[Debrief_award_text_num_lines], str);	

	// maybe translate for displaying
	if (Lcl_gr) {
		lcl_translate_medal_name_gr(Debrief_award_text[Debrief_award_text_num_lines]);
	}

	Debrief_award_text_num_lines++;

	// if its too long, split once ONLY
	// assumes text isnt > 2 lines, but this is a safe assumption due to the char limits of the ranks/badges/etc
	if (Debrief_award_text_num_lines < AWARD_TEXT_MAX_LINES) {
		line2 = split_str_once(Debrief_award_text[Debrief_award_text_num_lines-1], field_width);
		if (line2 != NULL) {
			sprintf(Debrief_award_text[Debrief_award_text_num_lines], " %s", line2);  // indent a space
		}
		Debrief_award_text_num_lines++;		// leave blank line even if it all fits into 1
	}
}

//	called once per frame to drive all the input reading and rendering
void debrief_do_frame(float frametime)
{
	int k=0, new_k=0;
	const char *please_wait_str = XSTR("Please Wait", 1242);
	char buf[256];

	Assert(Debrief_inited);	

	// Goober5000 - accept immediately if skipping debriefing
	if (The_mission.flags & MISSION_FLAG_TOGGLE_DEBRIEFING)
	{
		// make sure that we can actually advance - we don't want an endless loop!!!
		if ( !((/*Cheats_enabled ||*/ Turned_traitor || Must_replay_mission) && (Game_mode & GM_CAMPAIGN_MODE)) )
		{
			debrief_accept();
			return;
		}
	}

	int str_w, str_h;

	// first thing is to load the files
	if ( MULTIPLAYER_CLIENT && !Debrief_multi_stages_loaded ) {
		// draw the background, etc
		GR_MAYBE_CLEAR_RES(Background_bitmap);
		if (Background_bitmap >= 0) {
			gr_set_bitmap(Background_bitmap);
			gr_bitmap(0, 0);
		}

		Debrief_ui_window.draw();
		chatbox_render();
		if ( Debrief_multi_loading_bitmap > -1 ){
			gr_set_bitmap(Debrief_multi_loading_bitmap);		
			gr_bitmap( Please_wait_coords[gr_screen.res][0], Please_wait_coords[gr_screen.res][1] );
		}

		// draw "Please Wait"		
		gr_set_color_fast(&Color_normal);
		gr_set_font(FONT2);
		gr_get_string_size(&str_w, &str_h, please_wait_str);
		gr_string((gr_screen.max_w - str_w) / 2, (gr_screen.max_h - str_h) / 2, please_wait_str);
		gr_set_font(FONT1);

		gr_flip();

		// make sure we run the debrief do frame
		if (Game_mode & GM_MULTIPLAYER) {
			multi_debrief_do_frame();
		}

		// esc pressed?		
		os_poll();	
		int keypress = game_check_key();	
		if(keypress == KEY_ESC){
			// popup to leave
			multi_quit_game(PROMPT_CLIENT);
		}

		return;
	}

	// if multiplayer client, and not loaded voice, then load it
	if ( MULTIPLAYER_CLIENT && !Debrief_multi_voice_loaded ) {
		debrief_multi_fixup_stages();
		debrief_voice_load_all();
		Debrief_multi_voice_loaded = 1;
	}

	if ( help_overlay_active(DEBRIEFING_OVERLAY) ) {
		Buttons[gr_screen.res][HELP_BUTTON].button.reset_status();
		Debrief_ui_window.set_ignore_gadgets(1);
	}


	k = chatbox_process();

	if ( Game_mode & GM_NORMAL ) {
		new_k = Debrief_ui_window.process(k);
	} else {
		new_k = Debrief_ui_window.process(k, 0);
	}

	if ( (k > 0) || (new_k > 0) || B1_JUST_RELEASED ) {
		if ( help_overlay_active(DEBRIEFING_OVERLAY) ) {
			help_overlay_set_state(DEBRIEFING_OVERLAY, 0);
			Debrief_ui_window.set_ignore_gadgets(0);
			k = 0;
			new_k = 0;
		}
	}

	if ( !help_overlay_active(DEBRIEFING_OVERLAY) ) {
		Debrief_ui_window.set_ignore_gadgets(0);
	}

	// don't show pilot info popup by default
	Debrief_should_show_popup = 0;

	// see if the mode has changed and handle it if so.
	if ( Current_mode != New_mode ) {
		debrief_voice_stop();
		Current_mode = New_mode;
		Current_stage = -1;
		New_stage = 0;
		if (New_mode == DEBRIEF_TAB) {
			Num_stages = 1;
			Debrief_cue_voice = 0;
			Stage_voice = -1;
			if (Debrief_first_voice_flag) {
				Debrief_cue_voice = timestamp(DEBRIEF_VOICE_DELAY * 3);
				Debrief_first_voice_flag = 0;
			}
		} else {
			Num_stages = DEBRIEF_NUM_STATS_PAGES;
		}
	}

	if ((Num_stages > 0) &&  (New_stage != Current_stage)) {
		Current_stage = New_stage;
		debrief_text_init();
	}

	debrief_voice_play();

	// multi clients get a slight delay before music start to check goals again
	if ( timestamp_valid(Debrief_music_timeout) ) {
		if ( timestamp_elapsed(Debrief_music_timeout) ) {
			Debrief_music_timeout = 0;

			if ( mission_goals_met() ) {
				common_music_init(SCORE_DEBRIEF_SUCCESS);
			} else {
				common_music_init(SCORE_DEBRIEF_AVERAGE);
			}

			common_music_do();
		}
	} else {	
		common_music_do();
	}

	if (Game_mode & GM_MULTIPLAYER) {
		multi_debrief_do_frame();
	}

	// Now do all the rendering for the frame
	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0);
	} 

	// draw the awarded stuff, G
	if ( Award_active && (Award_bg_bitmap >= 0) ) {
		gr_set_bitmap(Award_bg_bitmap);
		gr_bitmap(Debrief_award_wnd_coords[gr_screen.res][0], Debrief_award_wnd_coords[gr_screen.res][1]);
		if (Rank_bitmap >= 0) {
			gr_set_bitmap(Rank_bitmap);
			gr_bitmap(Debrief_award_coords[gr_screen.res][0], Debrief_award_coords[gr_screen.res][1]);
		}

		if (Medal_bitmap >= 0) {
			gr_set_bitmap(Medal_bitmap);
			gr_bitmap(Debrief_award_coords[gr_screen.res][0], Debrief_award_coords[gr_screen.res][1]);
		}

		if (Badge_bitmap >= 0) {
			gr_set_bitmap(Badge_bitmap);
			gr_bitmap(Debrief_award_coords[gr_screen.res][0], Debrief_award_coords[gr_screen.res][1]);
		}

		//  draw medal/badge/rank labels
		debrief_draw_award_text();
	}
	
	Debrief_ui_window.draw();
	debrief_redraw_pressed_buttons();
	Buttons[gr_screen.res][Current_mode].button.draw_forced(2);
	if (Recommend_active && (Current_mode != STATS_TAB)) {
		Buttons[gr_screen.res][RECOMMENDATIONS].button.draw_forced(2);
	}

	// draw the title of the mission
	gr_set_color_fast(&Color_bright_white);
	strcpy_s(buf, The_mission.name);
	gr_force_fit_string(buf, 255, Debrief_title_coords[gr_screen.res][2]);
	gr_string(Debrief_title_coords[gr_screen.res][0], Debrief_title_coords[gr_screen.res][1], buf);	

#if !defined(NDEBUG)
	gr_set_color_fast(&Color_normal);
	gr_printf(Debrief_title_coords[gr_screen.res][0], Debrief_title_coords[gr_screen.res][1] - 10, NOX("[name: %s, mod: %s]"), Mission_filename, The_mission.modified);
#endif

	// draw the screen-specific text
	switch (Current_mode) {
		case DEBRIEF_TAB:
			if ( Num_debrief_stages <= 0 ) {
				gr_set_color_fast(&Color_white);
				Assert( Game_current_mission_filename != NULL );
				gr_printf(Debrief_text_wnd_coords[gr_screen.res][0], Debrief_text_wnd_coords[gr_screen.res][1], XSTR( "No Debriefing for mission: %s", 458), Game_current_mission_filename);

			} else {
				debrief_render();
			}

			break;

		case STATS_TAB:
			debrief_stats_render();
			break;
	} // end switch

	if (gr_screen.res == 1) {
		Max_debrief_Lines = 450/gr_get_font_height(); //Make the max number of lines dependent on the font height. 225 and 85 are magic numbers, based on the window size in retail. 
	} else {
		Max_debrief_Lines = 340/gr_get_font_height();
	}

	if (Max_debrief_Lines < Num_text_lines) {
		int w;

		gr_set_color_fast(&Color_red);
		gr_get_string_size(&w, NULL, XSTR( "More", 459));
		gr_printf(Debrief_text_wnd_coords[gr_screen.res][0] + Debrief_text_wnd_coords[gr_screen.res][2] / 2 - w / 2, Debrief_text_wnd_coords[gr_screen.res][1] + Debrief_text_wnd_coords[gr_screen.res][3], XSTR( "More", 459));
	}

	debrief_render_stagenum();
	debrief_multi_list_draw();

	// render some extra stuff in multiplayer
	if (Game_mode & GM_MULTIPLAYER) {
		// render the chatbox last
		chatbox_render();

		// draw tooltips
		Debrief_ui_window.draw_tooltip();

		// render the status indicator for the voice system
		multi_common_voice_display_status();
	}

	// AL 3-6-98: Needed to move key reading here, since popups are launched from this code, and we don't
	//				  want to include the mouse pointer which is drawn in the flip

	if ( !help_overlay_active(DEBRIEFING_OVERLAY) ) {
		debrief_check_buttons();
		debrief_do_keys(new_k);	
	}

	// blit help overlay if active
	help_overlay_maybe_blit(DEBRIEFING_OVERLAY);

	gr_flip();

	// maybe show skip mission popup
	if ( Must_replay_mission && (!Debrief_skip_popup_already_shown) && (Player->show_skip_popup) && (Game_mode & GM_NORMAL) && (Game_mode & GM_CAMPAIGN_MODE) && (Player->failures_this_session >= PLAYER_MISSION_FAILURE_LIMIT) && !(Game_mode & GM_MULTIPLAYER)) {
		int popup_choice = popup(0, 3, XSTR("Do Not Skip This Mission", 1473),
												 XSTR("Advance To The Next Mission", 1474),
												 XSTR("Don't Show Me This Again", 1475),
												 XSTR("You have failed this mission five times.  If you like, you may advance to the next mission.", 1472) );
		switch (popup_choice) {
		case 0:
			// stay on this mission, so proceed to normal debrief
			// in other words, do nothing.
			break;
		case 1:
			// skip this mission
			mission_campaign_skip_to_next();
			gameseq_post_event(GS_EVENT_START_GAME);
			break;
		case 2:
			// don't show this again
			Player->show_skip_popup = 0;
			break;
		}

		Debrief_skip_popup_already_shown = 1;
	}

	// check to see if we should be showing a pilot info popup in multiplayer (if a guy was double clicked)
	if ((Game_mode & GM_MULTIPLAYER) && Debrief_should_show_popup) {
		Assert((Multi_list_select >= 0) && (Multi_list_select < Multi_list_size));
		multi_pinfo_popup(&Net_players[Multi_list[Multi_list_select].net_player_index]);

		Debrief_should_show_popup = 0;
	}
}

void debrief_rebuild_player_list()
{
	int i;
	net_player *np;
	debrief_multi_list_info *list;

	Multi_list_size = 0;  // number of net players to choose from

	for ( i=0; i<MAX_PLAYERS; i++ ) {
		np = &Net_players[i];
		// remember not to include the standalone.
		if ( MULTI_CONNECTED((*np)) && !MULTI_STANDALONE((*np))){
			list = &Multi_list[Multi_list_size++];
			list->net_player_index = i;
			strcpy_s(list->callsign, np->m_player->callsign);
			
			// make sure to leave some room to blit the team indicator
			gr_force_fit_string(list->callsign, CALLSIGN_LEN - 1, Debrief_list_coords[gr_screen.res][2] - MULTI_LIST_TEAM_OFFSET);
		}
	} // end for
}

void debrief_handle_player_drop()
{
	debrief_rebuild_player_list();
}

void debrief_disable_accept()
{
}

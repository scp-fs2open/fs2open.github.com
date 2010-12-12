/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hudconfig.h"
#include "gamesequence/gamesequence.h"
#include "playerman/player.h"
#include "io/key.h"
#include "ui/ui.h"
#include "hud/hudobserver.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/pstypes.h"
#include "popup/popup.h"
#include "ship/ship.h"
#include "parse/parselo.h"
#include "cfile/cfile.h"
#include "iff_defs/iff_defs.h"


//////////////////////////////////////////////////////////////////////////////
// Game-wide Globals
//////////////////////////////////////////////////////////////////////////////

#define MAX_HCF_FILES			30
int HC_num_files = -1;						// num known hcf files
int HC_current_file = -1;					// current hcf file
char *HC_filenames[MAX_HCF_FILES];

char HC_fname[MAX_FILENAME_LEN+1] = "";
UI_INPUTBOX HC_fname_input;
int HC_fname_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		44,	449,	218,	17
	},
	{ // GR_1024
		101,	730,	319,	27
	}
};

HUD_CONFIG_TYPE HUD_config;	// Player HUD configuration
 
char Hud_Gauge_Names[NUM_HUD_GAUGES][NAME_LENGTH] = {
	"lead indicator",
	"target orientation",
	"closest attacking hostile",
	"current target direction",
	"mission time",
	"reticle",
	"throttle",
	"radar",
	"target monitor",
	"center of reticle",
	"extra target info",
	"target shield",
	"player shield",
	"power management",
	"auto-target icon",
	"auto-speed-match icon",
	"weapons display",
	"monitoring view",
	"directives view",
	"threat gauge",
	"afterburner energy",
	"weapons energy",
	"weapon linking",
	"target hull/shield icon",
	"offscreen indicator",
	"comm video",
	"damage display",
	"message output",
	"locked missile direction",
	"countermeasures",
	"objective notify",
	"wingmen status",
	"offscreen range",
	"kills gauge",
	"attacking target count",
	"warning flash",
	"comm menu",
	"support gauge",
	"lag gauge"
};

// specify the max distance that the radar should detect objects
// See RR_ #defines in HUDconfig.h.

float Radar_ranges[RR_MAX_RANGES] = {
	2000.0f,		// short
	10000.0f,		// med
	10000000.0f,	// infinity
};

char *Radar_range_text(int n)
{
	#if RR_MAX_RANGES != 3
	#error Number of ranges is wrong!
	#endif

	switch(n)	{
	case 0:
		return XSTR( "2000 M", 246);
	case 1:
		return XSTR( "10,000 M", 247);
	case 2:
		return XSTR( "infinity", 248);
	}
	return NULL;
}

// default flags for observer HUD
int HUD_observer_default_flags = 
{
	(1<<HUD_CENTER_RETICLE)			|	
	(1<<HUD_OFFSCREEN_INDICATOR)	|
	(1<<HUD_MESSAGE_LINES)			|
	(1<<HUD_HOSTILE_TRIANGLE)		|
	(1<<HUD_TARGET_TRIANGLE)		|
	(1<<HUD_TARGET_MINI_ICON)		|
	(1<<HUD_TARGET_MONITOR)			 	
};

int HUD_observer_default_flags2 = {
	(1<<(HUD_OFFSCREEN_RANGE - 32))
};

// default flags for regular HUD
int HUD_config_default_flags = 
{
	(1<<HUD_LEAD_INDICATOR) |
	(1<<HUD_ORIENTATION_TEE) |
	(1<<HUD_HOSTILE_TRIANGLE) |
	(1<<HUD_TARGET_TRIANGLE) |
	(1<<HUD_MISSION_TIME) |
	(1<<HUD_RETICLE_CIRCLE) |
	(1<<HUD_THROTTLE_GAUGE) |
	(1<<HUD_RADAR) |
	(1<<HUD_TARGET_MONITOR) |
	(1<<HUD_CENTER_RETICLE) |
	(1<<HUD_TARGET_MONITOR_EXTRA_DATA) |
	(1<<HUD_TARGET_SHIELD_ICON) |
	(1<<HUD_PLAYER_SHIELD_ICON) |
	(1<<HUD_ETS_GAUGE) |
	(1<<HUD_AUTO_TARGET) |
	(1<<HUD_AUTO_SPEED) |
	(1<<HUD_WEAPONS_GAUGE) |
	(1<<HUD_ESCORT_VIEW) |
	(1<<HUD_DIRECTIVES_VIEW) |
	(1<<HUD_THREAT_GAUGE) |
	(1<<HUD_AFTERBURNER_ENERGY) |
	(1<<HUD_WEAPONS_ENERGY) |
	(1<<HUD_WEAPON_LINKING_GAUGE) |
	(1<<HUD_TARGET_MINI_ICON) |
	(1<<HUD_OFFSCREEN_INDICATOR) |
	(1<<HUD_TALKING_HEAD) |
	(1<<HUD_DAMAGE_GAUGE) |
	(1<<HUD_MESSAGE_LINES) |
	(1<<HUD_MISSILE_WARNING_ARROW) |
	(1<<HUD_CMEASURE_GAUGE) |
	(1<<HUD_OBJECTIVES_NOTIFY_GAUGE) |
	(1<<HUD_WINGMEN_STATUS)
};

int HUD_config_default_flags2 = 
{
	(1<<(HUD_OFFSCREEN_RANGE-32)) |
	(1<<(HUD_KILLS_GAUGE-32)) |
	(1<<(HUD_ATTACKING_TARGET_COUNT-32)) | 
	(1<<(HUD_SUPPORT_GAUGE-32)) | 
	(1<<(HUD_LAG_GAUGE-32))
};

// bits to tell whether a given gauge should be treated as pop-up or not
int HUD_default_popup_mask =
{
	0	|											// (1<<HUD_LEAD_INDICATOR) |
	0	|											// (1<<HUD_ORIENTATION_TEE) |
	0	|											//	(1<<HUD_HOSTILE_TRIANGLE) |
	0	|											// (1<<HUD_TARGET_TRIANGLE) |
	0	|											//	(1<<HUD_MISSION_TIME) |
	0	|											//	(1<<HUD_RETICLE_CIRCLE) |
	0	|											//	(1<<HUD_THROTTLE_GAUGE) |
	0	|											//	(1<<HUD_RADAR) |
	0	|											//	(1<<HUD_TARGET_MONITOR) |
	0	|											//	(1<<HUD_CENTER_RETICLE) |
	0	|											//	(1<<HUD_TARGET_MONITOR_EXTRA_DATA) |
	0	|											//	(1<<HUD_TARGET_SHIELD_ICON) |
	0	|											//	(1<<HUD_PLAYER_SHIELD_ICON) |
	0	|											//	(1<<HUD_ETS_GAUGE) |
	0	|											//	(1<<HUD_AUTO_TARGET) |
	0	|											//	(1<<HUD_AUTO_SPEED) |
	0	|											//	(1<<HUD_WEAPONS_GAUGE) |
	0	|											//	(1<<HUD_ESCORT_VIEW) |
	0	|											//	(1<<HUD_DIRECTIVES_VIEW) |
	0	|											//	(1<<HUD_THREAT_GAUGE) |
	0	|											//	(1<<HUD_AFTERBURNER_ENERGY) |
	0	|											//	(1<<HUD_WEAPONS_ENERGY) |
	0	|											//	(1<<HUD_WEAPON_LINKING_GAUGE) |
	0	|											//	(1<<HUD_TARGET_MINI_ICON) |
	0	|											//(1<<HUD_OFFSCREEN_INDICATOR)
	0  |											// talking head
	0	|											// damage gauge
	0	|											// message lines				
	0	|											// missile warning arrow
	0	|											// countermeasures
	0												// wingman status
};

int HUD_default_popup_mask2 =
{
	0  |											// offscreen indicator range
	0	|
	0											// kills gauge
};

void hud_config_color_save(char *name);
void hud_config_select_all_toggle(int toggle);

int HC_select_all = 0;

//////////////////////////////////////////////////////////////////////////////
// Module Globals
//////////////////////////////////////////////////////////////////////////////


char *Hud_config_fname[GR_NUM_RESOLUTIONS] = {
	"HUDConfig",
	"2_HUDConfig"
};

char *Hud_config_mask_fname[GR_NUM_RESOLUTIONS] = {
	"HUDConfig-m",
	"2_HUDConfig-m"
};

struct HC_gauge_region
{
	char			*filename;
	int			x,y;
	int			hotspot;
	int			use_iff;
	int			can_popup;
	int			bitmap;
	int			nframes;
	int			color;
	UI_BUTTON	button;

	HC_gauge_region(char *name, int x1, int y1, int h, int iff, int cp, int b, int nf, int cl) : filename(name), x(x1), y(y1), hotspot(h), use_iff(iff), can_popup(cp), bitmap(b), nframes(nf), color(cl){}
};

// hud config gauges
struct HC_gauge_region	HC_gauge_regions[GR_NUM_RESOLUTIONS][NUM_HUD_GAUGES] =
{
	{ // GR_640
	//XSTR:OFF
		HC_gauge_region("HCB_35",	407,	69,	35,	1,	0,	-1, 0,	2),			// lead indicator
		HC_gauge_region("HCB_36",	305,	119,	36,	1,	0,	-1, 0,	2),			// orientation tee
		HC_gauge_region("none",		1,		1,		-1,	1,	0,	-1, 0,	0),			// hostile triangle
		HC_gauge_region("HCB_37",	391,	107,	37,	1,	0,	-1, 0,	2),			// target triangle
		HC_gauge_region("HCB_63",	575,	352,	63,	0,	0,	-1, 0,	0),			// mission time
		HC_gauge_region("none",		1,		1,		1,		0,	0,	-1, 0,	0),			// reticle circle?
		HC_gauge_region("HCB_40",	285,	146,	40,	0,	0,	-1, 0,	0),			// throttle gauge
		HC_gauge_region("HCB_50",	317,	291,	50,	0,	0,	-1, 0,	0),			// radar
		HC_gauge_region("HCB_31",	123,	249,	31,	0,	0,	-1, 0,	0),			// target monitor
		HC_gauge_region("HCB_41",	361,	188,	41,	0,	0,	-1, 0,	0),			// center of reticle
		HC_gauge_region("HCB_30",	123,	221,	30,	0,	0,	-1, 0,	0),			// extra target data
		HC_gauge_region("HCB_49",	237,	303,	49,	0,	0,	-1, 0,	0),			// target shield icon
		HC_gauge_region("HCB_51",	435,	304,	51,	0,	0,	-1, 0,	0),			// player shield icon
		HC_gauge_region("HCB_58",	524,	299,	58,	0,	1, -1, 0,	0),			// ets gauge
		HC_gauge_region("HCB_61",	566,	299,	61,	0,	1, -1, 0,	0),			// auto target
		HC_gauge_region("HCB_62",	566,	317,	62,	0,	1, -1, 0,	0),			// auto speed
		HC_gauge_region("HCB_55",	504,	216,	55,	0,	1, -1, 0,	0),			// weapons gauge
		HC_gauge_region("HCB_54",	496,	166,	54,	0,	1, -1, 0,	0),			// escort view
		HC_gauge_region("HCB_29",	123,	142,	29,	0,	0, -1, 0,	0),			// directives view
		HC_gauge_region("HCB_43",	398,	147,	43,	0,	0, -1, 0,	0),			// threat gauge
		HC_gauge_region("HCB_39",	250,	212,	39,	0,	0, -1, 0,	0),			// afterburner energy
		HC_gauge_region("HCB_44",	449,	212,	44,	0,	0, -1, 0,	0),			// weapons energy
		HC_gauge_region("none",		1,		1,		-1,	0,	0, -1, 0,	0),			// weapon linking
		HC_gauge_region("HCB_42",	356,	232,	42,	0,	1, -1, 0,	0),			// target mini icon (shield)
		HC_gauge_region("HCB_34",	438,	5,		34,	1,	0, -1, 0,	2),			// offscreen indicator
		HC_gauge_region("HCB_28",	123,	31,	28,	0,	0, -1, 0,	0),			// talking head
		HC_gauge_region("HCB_32",	309,	33,	32,	0,	1, -1, 0,	0),			// damage gauge
		HC_gauge_region("HCB_27",	124,	19,	27,	0,	0, -1, 0,	0),			// message lines
		HC_gauge_region("HCB_45",	307,	249,	45,	1,	0, -1, 0,	1),			// missile warnings
		HC_gauge_region("HCB_56",	505,	271,	56,	0,	1,	-1, 0,	0),			// cmeasure gauge
		HC_gauge_region("HCB_33",	309,	87,	33,	0,	0,	-1, 0,	0),			// objectives notify gauge
		HC_gauge_region("HCB_53",	546,	117,	53,	0,	0,	-1, 0,	0),			// wingman status gauge
		HC_gauge_region("none",		1,		1,		-1,	0,	0,	-1, 0,	0),			// offscreen indicator range
		HC_gauge_region("HCB_57",	505,	285,	57,	0,	1,	-1, 0,	0),			// kills gauge
		HC_gauge_region("none",		1,		1,		-1,	0,	0,	-1, 0,	0),			// attacking target count
		HC_gauge_region("HCB_38",	342,	138,	38,	0,	0,	-1, 0,	0),			// text flash gauge
		HC_gauge_region("HCB_52",	465,	8,		52,	0,	0,	-1, 0,	0),			// comm menu
		HC_gauge_region("HCB_46",	324,	264,	46,	0,	0,	-1, 0,	0),			// support view gauge
		HC_gauge_region("HCB_47",	418,	262,	47,	0,	0,	-1, 0,	0),			// netlag icon gauge
	//XSTR:ON
	},
	{ // GR_1024
	//XSTR:OFF
		HC_gauge_region("2_HCB_35",	652,	112,	35,	1,	0,	-1, 0,	2),			// lead indicator
		HC_gauge_region("2_HCB_36",	489,	191,	36,	1,	0,	-1, 0,	2),			// orientation tee
		HC_gauge_region("none",			1,		1,		-1,	1,	0,	-1, 0,	0),			// hostile triangle
		HC_gauge_region("2_HCB_37",	626,	173,	37,	1,	0,	-1, 0,	2),			// target triangle
		HC_gauge_region("2_HCB_63",	920,	564,	63,	0,	0,	-1, 0,	0),			// mission time
		HC_gauge_region("none",			1,		1,		1,		0,	0,	-1, 0,	0),			// reticle circle?
		HC_gauge_region("2_HCB_40",	456,	235,	40,	0,	0,	-1, 0,	0),			// throttle gauge
		HC_gauge_region("2_HCB_50",	508,	466,	50,	0,	0,	-1, 0,	0),			// radar
		HC_gauge_region("2_HCB_31",	198,	399,	31,	0,	0,	-1, 0,	0),			// target monitor
		HC_gauge_region("2_HCB_41",	578,	302,	41,	0,	0,	-1, 0,	0),			// center of reticle
		HC_gauge_region("2_HCB_30",	198,	354,	30,	0,	0,	-1, 0,	0),			// extra target data
		HC_gauge_region("2_HCB_49",	380,	485,	49,	0,	0,	-1, 0,	0),			// target shield icon
		HC_gauge_region("2_HCB_51",	696,	486,	51,	0,	0,	-1, 0,	0),			// player shield icon
		HC_gauge_region("2_HCB_58",	839,	479,	58,	0,	1, -1, 0,	0),			// ets gauge
		HC_gauge_region("2_HCB_61",	906,	479,	61,	0,	1, -1, 0,	0),			// auto target
		HC_gauge_region("2_HCB_62",	906,	508,	62,	0,	1, -1, 0,	0),			// auto speed
		HC_gauge_region("2_HCB_55",	807,	346,	55,	0,	1, -1, 0,	0),			// weapons gauge
		HC_gauge_region("2_HCB_54",	794,	265,	54,	0,	1, -1, 0,	0),			// escort view
		HC_gauge_region("2_HCB_29",	198,	228,	29,	0,	0, -1, 0,	0),			// directives view
		HC_gauge_region("2_HCB_43",	637,	237,	43,	0,	0, -1, 0,	0),			// threat gauge
		HC_gauge_region("2_HCB_39",	403,	339,	39,	0,	0, -1, 0,	0),			// afterburner energy
		HC_gauge_region("2_HCB_44",	719,	339,	44,	0,	0, -1, 0,	0),			// weapons energy
		HC_gauge_region("none",			1,		1,		-1,	0,	0, -1, 0,	0),			// weapon linking
		HC_gauge_region("2_HCB_42",	569,	371,	42,	0,	1, -1, 0,	0),			// target mini icon (shield)
		HC_gauge_region("2_HCB_34",	701,	9,		34,	1,	0, -1, 0,	2),			// offscreen indicator
		HC_gauge_region("2_HCB_28",	198,	50,	28,	0,	0, -1, 0,	0),			// talking head
		HC_gauge_region("2_HCB_32",	495,	55,	32,	0,	1, -1, 0,	0),			// damage gauge
		HC_gauge_region("2_HCB_27",	199,	30,	27,	0,	0, -1, 0,	0),			// message lines
		HC_gauge_region("2_HCB_45",	491,	399,	45,	1,	0, -1, 0,	1),			// missile warnings
		HC_gauge_region("2_HCB_56",	808,	433,	56,	0,	1,	-1, 0,	0),			// cmeasure gauge
		HC_gauge_region("2_HCB_33",	495,	141,	33,	0,	0,	-1, 0,	0),			// objectives notify gauge
		HC_gauge_region("2_HCB_53",	873,	188,	53,	0,	0,	-1, 0,	0),			// wingman status gauge
		HC_gauge_region("none",			1,		1,		-1,	0,	0,	-1, 0,	0),			// offscreen indicator range
		HC_gauge_region("2_HCB_57",	808,	456,	57,	0,	1,	-1, 0,	0),			// kills gauge
		HC_gauge_region("none",			1,		1,		-1,	0,	0,	-1, 0,	0),			// attacking target count
		HC_gauge_region("2_HCB_38",	548,	222,	38,	0,	0,	-1, 0,	0),			// text flash gauge
		HC_gauge_region("2_HCB_52",	744,	14,	52,	0,	0,	-1, 0,	0),			// comm menu
		HC_gauge_region("2_HCB_46",	520,	422,	46,	0,	0,	-1, 0,	0),			// support view gauge
		HC_gauge_region("2_HCB_47",	670,	419,	47,	0,	0,	-1, 0,	0),			// netlag icon gauge
	//XSTR:ON
	}
};


int HC_gauge_description_coords[GR_NUM_RESOLUTIONS][3] = {
	{	// GR_640
		35, 397, 197
	},
	{	// GR_1024
		56, 632, 307
	}
};

char *HC_gauge_descriptions(int n)
{
	switch(n)	{
	case 0:
		return XSTR( "lead indicator", 249);
	case 1:
		return XSTR( "target orientation", 250);
	case 2:
		return XSTR( "closest attacking hostile", 251);
	case 3:
		return XSTR( "current target direction", 252);
	case 4:
		return XSTR( "mission time", 253);
	case 5:
		return XSTR( "reticle", 254);
	case 6:
		return XSTR( "throttle", 255);
	case 7:
		return XSTR( "radar", 256);
	case 8:
		return XSTR( "target monitor", 257);
	case 9:
		return XSTR( "center of reticle", 258);
	case 10:
		return XSTR( "extra target info", 259);
	case 11:
		return XSTR( "target shield", 260);
	case 12:
		return XSTR( "player shield", 261);
	case 13:
		return XSTR( "power management", 262);
	case 14:
		return XSTR( "auto-target icon", 263);
	case 15:
		return XSTR( "auto-speed-match icon", 264);
	case 16:
		return XSTR( "weapons display", 265);
	case 17:
		return XSTR( "monitoring view", 266);
	case 18:
		return XSTR( "directives view", 267);
	case 19:
		return XSTR( "threat gauge", 268);
	case 20:
		return XSTR( "afterburner energy", 269);
	case 21:
		return XSTR( "weapons energy", 270);
	case 22:
		return XSTR( "weapon linking", 271);
	case 23:
		return XSTR( "target hull/shield icon", 272);
	case 24:
		return XSTR( "offscreen indicator", 273);
	case 25:
		return XSTR( "comm video", 274);
	case 26:
		return XSTR( "damage display", 275);
	case 27:
		return XSTR( "message output", 276);
	case 28:
		return XSTR( "locked missile direction", 277);
	case 29:
		return XSTR( "countermeasures", 278);
	case 30:
		return XSTR( "objective notify", 279);
	case 31:
		return XSTR( "wingmen status", 280);
	case 32:
		return XSTR( "offscreen range", 281);
	case 33:
		return XSTR( "kills gauge", 282);
	case 34:
		return XSTR( "attacking target count", 283);
	case 35: 
		return XSTR("warning flash", 1459);
	case 36:
		return XSTR("comm menu", 1460);
	case 37: 
		return XSTR("support gauge", 1461);
	case 38:
		return XSTR("lag gauge", 1462);
	}
	return NULL;
}

#define NUM_HUD_BUTTONS			20

#define HCB_RED_UP				0
#define HCB_GREEN_UP				1
#define HCB_BLUE_UP				2
#define HCB_I_UP					3
#define HCB_RED_DOWN				4
#define HCB_GREEN_DOWN			5
#define HCB_BLUE_DOWN			6
#define HCB_I_DOWN				7
#define HCB_ON						8
#define HCB_OFF					9
#define HCB_POPUP					10
#define HCB_SAVE_HCF				11
#define HCB_PREV_HCF				12
#define HCB_NEXT_HCF				13
#define HCB_AMBER					14
#define HCB_BLUE					15
#define HCB_GREEN					16
#define HCB_SELECT_ALL			17
#define HCB_RESET					18
#define HCB_ACCEPT				19


ui_button_info HC_buttons[GR_NUM_RESOLUTIONS][NUM_HUD_BUTTONS] = {
	{ // GR_640
		ui_button_info("HCB_00",		6,		27,	-1,	-1,	0),
		ui_button_info("HCB_01",		30,	27,	-1,	-1,	1),
		ui_button_info("HCB_02",		55,	27,	-1,	-1,	2),
		ui_button_info("HCB_03",		80,	27,	-1,	-1,	3),
		ui_button_info("HCB_08",		6,		291,	-1,	-1,	8),
		ui_button_info("HCB_09",		30,	291,	-1,	-1,	9),
		ui_button_info("HCB_10",		55,	291,	-1,	-1,	10),
		ui_button_info("HCB_11",		80,	291,	-1,	-1,	11),
		ui_button_info("HCB_12",		4,		329,	-1,	-1,	12),
		ui_button_info("HCB_13",		4,		348,	-1,	-1,	13),
		ui_button_info("HCB_14",		4,		367,	-1,	-1,	14),
		ui_button_info("HCB_15",		2,		439,	-1,	-1,	15),
		ui_button_info("HCB_16",		266,	456,	-1,	-1,	16),
		ui_button_info("HCB_17",		292,	456,	-1,	-1,	17),
		ui_button_info("HCB_18",		327,	421,	-1,	-1,	18),
		ui_button_info("HCB_19",		327,	440,	-1,	-1,	19),
		ui_button_info("HCB_20",		327,	459,	-1,	-1,	20),
		ui_button_info("HCB_24",		472,	436,	-1,	-1,	24),
		ui_button_info("HCB_25",		523,	433,	-1,	-1,	25),
		ui_button_info("HCB_26",		576,	434,	-1,	-1,	26),
	},
	{ // GR_1024
		ui_button_info("2_HCB_00",		9,		44,	-1,	-1,	0),
		ui_button_info("2_HCB_01",		48,	44,	-1,	-1,	1),
		ui_button_info("2_HCB_02",		88,	44,	-1,	-1,	2),
		ui_button_info("2_HCB_03",		127,	44,	-1,	-1,	3),
		ui_button_info("2_HCB_08",		9,		466,	-1,	-1,	8),
		ui_button_info("2_HCB_09",		48,	466,	-1,	-1,	9),
		ui_button_info("2_HCB_10",		88,	466,	-1,	-1,	10),
		ui_button_info("2_HCB_11",		127,	466,	-1,	-1,	11),
		ui_button_info("2_HCB_12",		6,		526,	-1,	-1,	12),
		ui_button_info("2_HCB_13",		6,		556,	-1,	-1,	13),
		ui_button_info("2_HCB_14",		6,		586,	-1,	-1,	14),
		ui_button_info("2_HCB_15",		3,		703,	-1,	-1,	15),
		ui_button_info("2_HCB_16",		426,	730,	-1,	-1,	16),
		ui_button_info("2_HCB_17",		467,	730,	-1,	-1,	17),
		ui_button_info("2_HCB_18",		524,	674,	-1,	-1,	18),
		ui_button_info("2_HCB_19",		524,	704,	-1,	-1,	19),
		ui_button_info("2_HCB_20",		524,	734,	-1,	-1,	20),
		ui_button_info("2_HCB_24",		755,	698,	-1,	-1,	24),
		ui_button_info("2_HCB_25",		837,	693,	-1,	-1,	25),
		ui_button_info("2_HCB_26",		922,	695,	-1,	-1,	26),
	},
};

// text
#define NUM_HUD_TEXT					15
UI_XSTR HC_text[GR_NUM_RESOLUTIONS][NUM_HUD_TEXT] = {
	{ // GR_640
		{ "R",				1512,	14,	8,		UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "G",				1513,	37,	8,		UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "B",				1514,	62,	8,		UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "I",				1515,	90,	8,		UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "On",				1285,	36,	334,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_ON].button },
		{ "Off",				1286,	36,	353,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_OFF].button },
		{ "Popup",			1453,	36,	372,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_POPUP].button },
		{ "Save",			1454,	51,	428,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_SAVE_HCF].button },
		{ "Amber",			1455,	364,	426,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_AMBER].button },
		{ "Blue",			1456,	364,	445,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_BLUE].button },
		{ "Green",			1457,	364,	464,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_GREEN].button },		
		{ "Select",			1550,	442,	413,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_SELECT_ALL].button },
		{ "All",				1551,	442,	424,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_SELECT_ALL].button },
		{ "Reset",			1337,	515,	413,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[0][HCB_RESET].button },
		{ "Accept",			1035,	573,	413,	UI_XSTR_COLOR_PINK,	-1, &HC_buttons[0][HCB_ACCEPT].button },
	},
	{ // GR_1024
		{ "R",				1512,	23,	14,	UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "G",				1513,	60,	14,	UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "B",				1514,	100,	14,	UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "I",				1515,	144,	14,	UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "On",				1285,	58,	536,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_ON].button },
		{ "Off",				1286,	58,	566,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_OFF].button },
		{ "Popup",			1453,	58,	596,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_POPUP].button },
		{ "Save",			1454,	82,	688,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_SAVE_HCF].button },
		{ "Amber",			1455,	582,	685,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_AMBER].button },
		{ "Blue",			1456,	582,	715,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_BLUE].button },
		{ "Green",			1457,	582,	745,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_GREEN].button },		
		{ "Select",			1550,	760,	671,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_SELECT_ALL].button },
		{ "All",				1551,	760,	682,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_SELECT_ALL].button },
		{ "Reset",			1337,	850,	669,	UI_XSTR_COLOR_GREEN,	-1, &HC_buttons[1][HCB_RESET].button },
		{ "Accept",			1035,	930,	670,	UI_XSTR_COLOR_PINK,	-1, &HC_buttons[1][HCB_ACCEPT].button },
	}
};

void hud_config_set_rgb(int gauge, int r, int g, int b);
void hud_config_set_alpha(int gauge, int a);

/*
#define NUM_HC_SPECIAL_BITMAPS	2
#define HC_SPECIAL_RETICLE			0
#define HC_SPECIAL_TEXT				1
typedef struct HC_special_bitmap
{
	char *filename;
	int x,y;
	int bitmap;
} HC_special_bitmap;

HC_special_bitmap HC_special_bitmaps[NUM_HC_SPECIAL_BITMAPS] =
{
//XSTR:OFF
	{"HG_00a", 272, 146, -1},
	{"HG_04a", 132, 271, -1},
//XSTR:ON
};
*/

static int							HC_background_bitmap;
static UI_WINDOW					HC_ui_window;

static int							HC_gauge_hot;			// mouse is over this gauge
static int							HC_gauge_selected;	// gauge is selected

// slider controls
/*
#define HC_NUM_SLIDERS							1
#define HC_BRIGHTNESS_SLIDER					0
struct hc_sliders {
	// base slider
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	int dot_w;
	int dots;

	// left and right buttons
	char *left_filename;
	int left_mask, left_x, left_y;
	char *right_filename;
	int right_mask, right_x, right_y;

	// slider control
	UI_DOT_SLIDER_NEW slider;  // because we have a class inside this struct, we need the constructor below..

	hc_sliders(char *name, int x1, int y1, int xt1, int yt1, int h, int _dot_w, int _dots, char *_left_filename, int _left_mask, int _left_x, int _left_y, char *_right_filename, int _right_mask, int _right_x, int _right_y) : 
				 filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), dot_w(_dot_w), dots(_dots), left_filename(_left_filename), left_mask(_left_mask), left_x(_left_x), left_y(_left_y), right_filename(_right_filename), right_mask(_right_mask), right_x(_right_x), right_y(_right_y) {}
};

hc_sliders HC_sliders[GR_NUM_RESOLUTIONS][HC_NUM_SLIDERS] = {
	{ // GR_640
		hc_sliders(	"HCB_22",	419,	379,	-1,	-1,	22,	20,	10,							
						"HCB_23",	23,	611,	379,
						"HCB_21",	21,	391,	379)
	},
	{ // GR_1024
		hc_sliders(	"2_HCB_22",	670,	606,	-1,	-1,	22,	32,	10,							
						"2_HCB_23",	23,	698,	606,
						"2_HCB_21",	21,	625,	606),
	}	
};
*/


// HUD colors
typedef struct hc_col {
	ubyte	r, g, b;
} hc_col;

hc_col HC_colors[3] =
{
	{0, 255, 0},		// Green - get RGB from Adam so it matches palette?-??.pcx
	{67, 123, 203},	// Blue - get RGB from Adam so it matches palette?-??.pcx
	{255, 197, 0},		// Amber - get RGB from Adam so it matches palette?-??.pcx
};

static HUD_CONFIG_TYPE	HUD_config_backup;		// backup HUD config, used to restore old config if changes not applied
static int					HUD_config_inited = 0;

// rgba slider stuff
void hud_config_red_slider();
void hud_config_green_slider();
void hud_config_blue_slider();
void hud_config_alpha_slider_up();
void hud_config_alpha_slider_down();
void hud_config_recalc_alpha_slider();
void hud_config_process_colors();
void hud_config_color_init();
void hud_config_color_close();
#define NUM_HC_SLIDERS			4
#define HCS_RED					0
#define HCS_GREEN					1
#define HCS_BLUE					2
#define HCS_ALPHA					3
UI_SLIDER2 HC_color_sliders[NUM_HC_SLIDERS];
int HC_slider_coords[GR_NUM_RESOLUTIONS][NUM_HC_SLIDERS][4] = {
	{ // GR_640
		{ 8,	53,	15, 225 },
		{ 33, 53,	15, 225 },
		{ 58, 53,	15, 225 },
		{ 83, 53,	15, 225 },
	},
	{ // GR_1024
		{ 13,	85, 32, 350 },
		{ 53, 85, 32, 350 },
		{ 93, 85, 32, 350 },
		{ 133, 85, 32, 350 },
	},	
};
#define HCS_CONV(__v)			( 255 - (__v) )

char *HC_slider_fname[GR_NUM_RESOLUTIONS] = {
	"slider",
	"2_slider"
};

// sync sliders
void hud_config_synch_sliders(int i)
{
	if(i >= 0){
		HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV(HUD_config.clr[i].red) );
		HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV(HUD_config.clr[i].green) );
		HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV(HUD_config.clr[i].blue) );
		HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV(HUD_config.clr[i].alpha) );
	}
}

// reset some ui components based on HUD config data
void hud_config_synch_ui()
{
	HUD_init_hud_color_array();
	// HC_sliders[gr_screen.res][HC_BRIGHTNESS_SLIDER].slider.pos = HUD_color_alpha-3;		// convert to value from 0-10	

	// sync sliders to currently selected gauge
	hud_config_synch_sliders(HC_gauge_selected);
}

// Init the UI components
void hud_config_init_ui()
{
	int i;
	struct HC_gauge_region	*hg;
	struct ui_button_info			*hb;

//	common_set_interface_palette("HUDConfigPalette");  // set the interface palette
	hud_config_synch_ui();
	HC_background_bitmap = bm_load(Hud_config_fname[gr_screen.res]);

	HC_ui_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );
	HC_ui_window.set_mask_bmap(Hud_config_mask_fname[gr_screen.res]);

	for (i=0; i<NUM_HUD_GAUGES; i++) {
		hg = &HC_gauge_regions[gr_screen.res][i];
		if ( !stricmp(hg->filename, NOX("none")) ) {
			continue;
		}
		hg->button.create(&HC_ui_window, "", hg->x, hg->y, 60, 30, 0, 1);
		// set up callback for when a mouse first goes over a button
//		hg->button.set_highlight_action(common_play_highlight_sound);
		hg->button.hide();
		hg->button.link_hotspot(hg->hotspot);

		// if ( hg->use_iff ) {
// 			hg->bitmap = bm_load_animation(hg->filename, &hg->nframes);
	// 	} else {
			hg->bitmap = bm_load(hg->filename);
			hg->nframes = 1;
		// }
	}

	// add text
	for(i=0; i<NUM_HUD_TEXT; i++){
		HC_ui_window.add_XSTR(&HC_text[gr_screen.res][i]);
	}

	// initialize sliders
	HC_color_sliders[HCS_RED].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_RED][0], HC_slider_coords[gr_screen.res][HCS_RED][1], HC_slider_coords[gr_screen.res][HCS_RED][2], HC_slider_coords[gr_screen.res][HCS_RED][3],
										255, HC_slider_fname[gr_screen.res], hud_config_red_slider, hud_config_red_slider, hud_config_red_slider);

	HC_color_sliders[HCS_GREEN].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_GREEN][0], HC_slider_coords[gr_screen.res][HCS_GREEN][1], HC_slider_coords[gr_screen.res][HCS_GREEN][2], HC_slider_coords[gr_screen.res][HCS_GREEN][3],
										255, HC_slider_fname[gr_screen.res], hud_config_green_slider, hud_config_green_slider, hud_config_green_slider);

	HC_color_sliders[HCS_BLUE].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_BLUE][0], HC_slider_coords[gr_screen.res][HCS_BLUE][1], HC_slider_coords[gr_screen.res][HCS_BLUE][2], HC_slider_coords[gr_screen.res][HCS_BLUE][3],
										255, HC_slider_fname[gr_screen.res], hud_config_blue_slider, hud_config_blue_slider, hud_config_blue_slider);

	HC_color_sliders[HCS_ALPHA].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_ALPHA][0], HC_slider_coords[gr_screen.res][HCS_ALPHA][1], HC_slider_coords[gr_screen.res][HCS_ALPHA][2], HC_slider_coords[gr_screen.res][HCS_ALPHA][3],
										255, HC_slider_fname[gr_screen.res], hud_config_alpha_slider_up, hud_config_alpha_slider_down, NULL);
	
	hud_config_color_init();

	for (i=0; i<NUM_HUD_BUTTONS; i++) {
		hb = &HC_buttons[gr_screen.res][i];
		hb->button.create(&HC_ui_window, "", hb->x, hb->y, 60, 30, 0, 1);
		// set up callback for when a mouse first goes over a button
		hb->button.set_bmaps(hb->filename);
		hb->button.set_highlight_action(common_play_highlight_sound);
		hb->button.link_hotspot(hb->hotspot);
	}

	// config file input name	
	HC_fname_input.create(&HC_ui_window, HC_fname_coords[gr_screen.res][0], HC_fname_coords[gr_screen.res][1], HC_fname_coords[gr_screen.res][2], MAX_FILENAME_LEN, "", UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_ESC_FOC);	
	HC_fname_input.set_text("");

	/*
	for (i=0; i<NUM_HC_SPECIAL_BITMAPS; i++) {
		HC_special_bitmaps[i].bitmap = bm_load(HC_special_bitmaps[i].filename);
	}
	*/

	// create sliders
	/*
	for(i=0; i<HC_NUM_SLIDERS; i++){
		HC_sliders[gr_screen.res][i].slider.create(&HC_ui_window, HC_sliders[gr_screen.res][i].x, HC_sliders[gr_screen.res][i].y,
																		HC_sliders[gr_screen.res][i].dots, HC_sliders[gr_screen.res][i].filename,
																		HC_sliders[gr_screen.res][i].hotspot, HC_sliders[gr_screen.res][i].right_filename, HC_sliders[gr_screen.res][i].right_mask, HC_sliders[gr_screen.res][i].right_x, HC_sliders[gr_screen.res][i].right_y,
																		HC_sliders[gr_screen.res][i].left_filename, HC_sliders[gr_screen.res][i].left_mask, HC_sliders[gr_screen.res][i].left_x, HC_sliders[gr_screen.res][i].left_y,
																		HC_sliders[gr_screen.res][i].dot_w);
	}
	HC_sliders[gr_screen.res][HC_BRIGHTNESS_SLIDER].slider.pos = HUD_color_alpha - 3;
	*/
	
	
	HC_gauge_hot = -1;
	HC_gauge_selected = -1;

	HC_select_all = 0;

	strcpy_s(HC_fname, "");
}

int hud_config_show_flag_is_set(int i)
{
	int show_flag_set;

	if ( i < 32 ) {
		show_flag_set = HUD_config.show_flags & (1<<i);
	} else {
		show_flag_set = HUD_config.show_flags2 & (1<<(i-32));
	}

	return show_flag_set;
}

void hud_config_show_flag_set(int i)
{
	if ( i < 32 ) {
		HUD_config.show_flags |= (1<<i);
	} else {
		HUD_config.show_flags2 |= (1<<(i-32));
	}
}

void hud_config_show_flag_clear(int i)
{
	if ( i < 32 ) {
		HUD_config.show_flags &= ~(1<<i);
	} else {
		HUD_config.show_flags2 &= ~(1<<(i-32));
	}
}

int hud_config_popup_flag_is_set(int i)
{
	int popup_flag_set;

	if ( i < 32 ) {
		popup_flag_set = HUD_config.popup_flags & (1<<i);
	} else {
		popup_flag_set = HUD_config.popup_flags2 & (1<<(i-32));
	}

	return popup_flag_set;
}

void hud_config_popup_flag_set(int i)
{
	if ( i < 32 ) {
		HUD_config.popup_flags |= (1<<i);
	} else {
		HUD_config.popup_flags2 |= (1<<(i-32));
	}
}

void hud_config_popup_flag_clear(int i)
{
	if ( i < 32 ) {
		HUD_config.popup_flags &= ~(1<<i);
	} else {
		HUD_config.popup_flags2 &= ~(1<<(i-32));
	}
}

// render all the hud config gauges
void hud_config_render_gauges()
{
	int i;

	for ( i=0; i<NUM_HUD_GAUGES; i++ ) {
		color *use_color;
		int alpha;
		if ( (hud_config_show_flag_is_set(i)) ) {
			// set the correct color
			if(!HC_gauge_regions[gr_screen.res][i].use_iff){
				use_color = &HUD_config.clr[i];			
			} else {
				if(HC_gauge_regions[gr_screen.res][i].color == 1){
					use_color = iff_get_color(IFF_COLOR_TAGGED, 0);
				} else {
					use_color = &Color_bright_red;
				}
			}

			if ( (HC_gauge_selected == i) || HC_select_all ) {
				alpha = 255;				
			} else if ( HC_gauge_hot == i ) {
				alpha = 200;				
			} else {			
				alpha = 150;				
			}
			gr_init_alphacolor(use_color, use_color->red, use_color->green, use_color->blue, alpha);
			gr_set_color_fast(use_color);			
		} else {
			// if its off, make it dark gray
			use_color = &HUD_config.clr[i];
			gr_init_alphacolor(use_color, 127, 127, 127, 64);
			gr_set_color_fast(use_color);			
		}

		// draw
		if ( HC_gauge_regions[gr_screen.res][i].bitmap >= 0 ) {
			gr_set_bitmap(HC_gauge_regions[gr_screen.res][i].bitmap);
			gr_aabitmap(HC_gauge_regions[gr_screen.res][i].x, HC_gauge_regions[gr_screen.res][i].y);
		}
		
		/*
		else {

			int offset=0;
				// set correct frame if using iff
			if ( HC_gauge_regions[i].use_iff ) {
				if ( HC_gauge_selected == i ) {
					offset=2;
				} else if ( HC_gauge_hot == i ) {
					offset=1;
				}

				// If gauge is disabled, then draw disabled frame
				if ( !(hud_config_show_flag_is_set(i)) ) {
					offset=3;
				}
			}

			if ( HC_gauge_regions[i].bitmap >= 0 ) {
				Assert(offset < HC_gauge_regions[i].nframes);
				gr_set_bitmap(HC_gauge_regions[i].bitmap+offset);
				gr_bitmap(HC_gauge_regions[i].x, HC_gauge_regions[i].y);
			}
		}
		*/
	}
}

// hud_config_init() is called when the game enters the state GS_STATE_HUD_CONFIG
void hud_config_init()
{
	hud_config_init_ui();
	hud_config_backup(); // save the HUD configuration in case the player decides to cancel changes
	HUD_config_inited = 1;
}

// check for the mouse over gauge regions
void hud_config_check_regions()
{
	int			i;
	UI_BUTTON	*b;

	for ( i=0; i<NUM_HUD_GAUGES; i++ ) {
		b = &HC_gauge_regions[gr_screen.res][i].button;

		// check for mouse over gauges
		if ( b->button_hilighted() ) {
			HC_gauge_hot = i;
		}

		if ( b->pressed() ) {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_gauge_selected = i;

			// turn off select all
			hud_config_select_all_toggle(0);			
			
			// maybe setup rgb sliders
			if(HC_gauge_regions[gr_screen.res][i].use_iff){
				HC_color_sliders[HCS_RED].hide();
				HC_color_sliders[HCS_GREEN].hide();
				HC_color_sliders[HCS_BLUE].hide();
				HC_color_sliders[HCS_ALPHA].hide();

				HC_color_sliders[HCS_RED].disable();
				HC_color_sliders[HCS_GREEN].disable();
				HC_color_sliders[HCS_BLUE].disable();
				HC_color_sliders[HCS_ALPHA].disable();
			} else {
				HC_color_sliders[HCS_RED].enable();
				HC_color_sliders[HCS_GREEN].enable();
				HC_color_sliders[HCS_BLUE].enable();
				HC_color_sliders[HCS_ALPHA].enable();			

				HC_color_sliders[HCS_RED].unhide();
				HC_color_sliders[HCS_GREEN].unhide();
				HC_color_sliders[HCS_BLUE].unhide();
				HC_color_sliders[HCS_ALPHA].unhide();				

				HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV(HUD_config.clr[i].red) );
				HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV(HUD_config.clr[i].green) );
				HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV(HUD_config.clr[i].blue) );
				HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV(HUD_config.clr[i].alpha) );
			}

			// recalc alpha slider
			hud_config_recalc_alpha_slider();
		}
	}
}

// set the display flags for a HUD gauge
void hud_config_set_gauge_flags(int gauge_index, int on_flag, int popup_flag)
{
	if ( on_flag ) {
		hud_config_show_flag_set(gauge_index);
	} else {
		hud_config_show_flag_clear(gauge_index);
	}

	if ( popup_flag ) {
		hud_config_popup_flag_set(gauge_index);
	} else {
		hud_config_popup_flag_clear(gauge_index);
	}
}

void hud_config_record_color(int color)
{
	HUD_config.main_color = color;
	HUD_color_red = HC_colors[color].r;
	HUD_color_green = HC_colors[color].g;
	HUD_color_blue = HC_colors[color].b;
}

// Set the HUD color
void hud_config_set_color(int color)
{
	int idx;	

	hud_config_record_color(color);

	HUD_init_hud_color_array();

	// apply the color to all gauges
	for(idx=0; idx<NUM_HUD_GAUGES; idx++){
		gr_init_alphacolor(&HUD_config.clr[idx], HC_colors[color].r, HC_colors[color].g, HC_colors[color].b, (HUD_color_alpha+1)*16);
	}
}

void hud_config_stuff_colors(int r, int g, int b)
{
	int idx;

	// apply the color to all gauges
	for(idx=0; idx<NUM_HUD_GAUGES; idx++){
		gr_init_alphacolor(&HUD_config.clr[idx], r, g, b, 255);
	}
}

// leave hud config without accepting changes
void hud_config_cancel()
{
	hud_config_restore();
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

// leave hud config with accepting changes
void hud_config_commit()
{
	gamesnd_play_iface(SND_COMMIT_PRESSED);
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

// move gauge state from on->off->popup
void hud_cycle_gauge_status()
{
	if ( HC_gauge_selected < 0 ) {
		return;
	}

	// gauge is off, move to popup
	if ( !(hud_config_show_flag_is_set(HC_gauge_selected)) ) {
		if ( HC_gauge_regions[gr_screen.res][HC_gauge_selected].can_popup ) {
			hud_config_set_gauge_flags(HC_gauge_selected, 1, 1);	
		} else {
			hud_config_set_gauge_flags(HC_gauge_selected, 1, 0);	
		}
		return;
	}

	// if gauge is popup, move to on
	if ( hud_config_popup_flag_is_set(HC_gauge_selected) ) {
		hud_config_set_gauge_flags(HC_gauge_selected, 1, 0);
		return;
	}
	
	// gauge must be on, move to off
	hud_config_set_gauge_flags(HC_gauge_selected, 0, 0);
}

// handle keyboard input while in hud config
void hud_config_handle_keypresses(int k)
{
	switch(k) {
	case KEY_ESC:
		hud_config_cancel();
		break;
	case KEY_CTRLED+KEY_ENTER:
		hud_config_commit();
		break;
	case KEY_TAB:
		gamesnd_play_iface(SND_USER_SELECT);
		hud_cycle_gauge_status();
		break;
	}
}

// Handlers for when buttons get pressed
void hud_config_button_do(int n)
{
	int idx;
	char name[256] = "";

	switch (n) {
	case HCB_AMBER:
		hud_config_set_color(HUD_COLOR_AMBER);
		gamesnd_play_iface(SND_USER_SELECT);
		break;
	case HCB_BLUE:
		hud_config_set_color(HUD_COLOR_BLUE);
		gamesnd_play_iface(SND_USER_SELECT);
		break;
	case HCB_GREEN:
		hud_config_set_color(HUD_COLOR_GREEN);
		gamesnd_play_iface(SND_USER_SELECT);
		break;
	case HCB_ON:
		if ( HC_gauge_selected < 0 ) {
			break;
		}
		gamesnd_play_iface(SND_USER_SELECT);
		hud_config_set_gauge_flags(HC_gauge_selected,1,0);
		break;
	case HCB_OFF:
		if ( HC_gauge_selected < 0 ) {
			break;
		}
		gamesnd_play_iface(SND_USER_SELECT);
		hud_config_set_gauge_flags(HC_gauge_selected,0,0);
		break;
	case HCB_POPUP:
		if ( HC_gauge_selected < 0 ) {
			break;
		}
		gamesnd_play_iface(SND_USER_SELECT);
		hud_config_set_gauge_flags(HC_gauge_selected,1,1);
		break;
	case HCB_RESET:
		gamesnd_play_iface(SND_RESET_PRESSED);
		hud_config_select_all_toggle(0);
		hud_set_default_hud_config(Player);
		hud_config_synch_ui();
		break;
	case HCB_ACCEPT:
		hud_config_commit();
		break;

	// new stuff
	case HCB_RED_UP:
		if( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) >= 255){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) + 1)  );
			hud_config_red_slider();			
		}		
		break;

	case HCB_GREEN_UP:
		if( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) >= 255){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) + 1) );
			hud_config_green_slider();
		}		
		break;

	case HCB_BLUE_UP:
		if( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) >= 255){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) + 1) );
			hud_config_blue_slider();
		}		
		break;

	case HCB_I_UP:
		if( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) >= 255){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) + 1) );
			hud_config_alpha_slider_up();
		}		
		break;

	case HCB_RED_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) <= 0){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) - 1) );
			hud_config_red_slider();
		}		
		break;

	case HCB_GREEN_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) <= 0){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) - 1) );
			hud_config_green_slider();
		}		
		break;

	case HCB_BLUE_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) <= 0){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) - 1) );
			hud_config_blue_slider();
		}		
		break;

	case HCB_I_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) <= 0){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) - 1) );
			hud_config_alpha_slider_down();
		}		
		break;

	case HCB_SAVE_HCF:		
		int exists;
		char *out;

		// get the text in the input control
		exists = 0;
		HC_fname_input.get_text(name);
		if(strlen(name) > 0){
			// if the filename in there already exists
			for(idx=0; idx<HC_num_files; idx++){
				if(!stricmp(HC_filenames[idx], name)){
					exists = 1;
				}
			}
		}

		// already exists?
		if(exists){
			// save the file			
			out = cf_add_ext(name, ".hcf");
			hud_config_color_save(out);
			break;
		}

		// otherwise we have to create a new file
		if(HC_num_files >= MAX_HCF_FILES){
			popup(PF_USE_AFFIRMATIVE_ICON, 1, "OK", "Max number of hud config files reached!");
			break;
		}

		// save the file, maybe generating a new filename
		if(strlen(name) <= 0){
			sprintf(name, "hud_%d.hcf", HC_num_files + 1);
			out = name;
		} else {
			out = cf_add_ext(name, ".hcf");
		}
		HC_filenames[HC_num_files++] = vm_strdup(out);
		hud_config_color_save(out);		

		HC_fname_input.set_text(out);
		break;

	case HCB_PREV_HCF:
		if(HC_num_files <= 0){
			break;
		}

		if(HC_current_file <= 0){
			HC_current_file = HC_num_files - 1;
		} else {
			HC_current_file--;
		}
		// load em up
		hud_config_color_load(HC_filenames[HC_current_file]);
		hud_config_synch_ui();

		HC_fname_input.set_text(HC_filenames[HC_current_file]);
		break;

	case HCB_NEXT_HCF:
		if(HC_num_files <= 0){
			break;
		}

		if(HC_current_file >= HC_num_files - 1){
			HC_current_file = 0;
		} else {
			HC_current_file++;
		}

		// load em up		
		hud_config_color_load(HC_filenames[HC_current_file]);
		hud_config_synch_ui();

		HC_fname_input.set_text(HC_filenames[HC_current_file]);
		break;

	case HCB_SELECT_ALL:				
		hud_config_select_all_toggle(!HC_select_all);
		break;	

	default:
		Int3();
		break;
	}
}

// Check if any buttons have been pressed
void hud_config_check_buttons()
{
	int			i;
	UI_BUTTON	*b;

	for ( i=0; i<NUM_HUD_BUTTONS; i++ ) {
		b = &HC_buttons[gr_screen.res][i].button;
		if ( b->pressed() ) {
			hud_config_button_do(i);
		}
	}
}

// set the hud color button
void hud_config_draw_color_status()
{
	if ( HC_buttons[gr_screen.res][HCB_AMBER].button.button_down() || HC_buttons[gr_screen.res][HCB_GREEN].button.button_down() || HC_buttons[gr_screen.res][HCB_BLUE].button.button_down() ) {
		return;
	}

	switch(HUD_config.main_color) {
	case HUD_COLOR_AMBER:
		HC_buttons[gr_screen.res][HCB_AMBER].button.draw_forced(2);
		break;
	case HUD_COLOR_GREEN:
		HC_buttons[gr_screen.res][HCB_GREEN].button.draw_forced(2);
		break;
	case HUD_COLOR_BLUE:
		HC_buttons[gr_screen.res][HCB_BLUE].button.draw_forced(2);
		break;
	}
}

// set the status (on/off/popup) for the selected gauge
void hud_config_draw_gauge_status()
{
	if ( HC_gauge_selected < 0 ) {
		return;
	}

	if ( HC_buttons[gr_screen.res][HCB_OFF].button.button_down() || HC_buttons[gr_screen.res][HCB_POPUP].button.button_down() || HC_buttons[gr_screen.res][HCB_ON].button.button_down() ) {
		return;
	}

	// check if off
	if ( !(hud_config_show_flag_is_set(HC_gauge_selected)) ) {
		HC_buttons[gr_screen.res][HCB_OFF].button.draw_forced(2);
		return;
	}

	// check if popup
	if ( hud_config_popup_flag_is_set(HC_gauge_selected) ) {
		HC_buttons[gr_screen.res][HCB_POPUP].button.draw_forced(2);
		return;
	}

	// check if on
	if ( hud_config_show_flag_is_set(HC_gauge_selected) ) {
		HC_buttons[gr_screen.res][HCB_ON].button.draw_forced(2);
		return;
	}

	Int3();	// should never get here
}

// disable a HUD config button
void hud_config_button_disable(int index)
{
	// HC_buttons[gr_screen.res][index].button.hide();
	HC_buttons[gr_screen.res][index].button.disable();
}

// enable a HUD config button
void hud_config_button_enable(int index)
{
	// HC_buttons[gr_screen.res][index].button.unhide();
	HC_buttons[gr_screen.res][index].button.enable();
}

// determine if on/off/popup buttons should be shown
void hud_config_set_button_state()
{
	if ( HC_gauge_selected < 0 ) {
		hud_config_button_disable(HCB_ON);
		hud_config_button_disable(HCB_OFF);
		hud_config_button_disable(HCB_POPUP);
		return;
	}

	// on/off are always on
	hud_config_button_enable(HCB_ON);
	hud_config_button_enable(HCB_OFF);

	// popup is maybe available
	if ( HC_gauge_regions[gr_screen.res][HC_gauge_selected].can_popup ) {
		hud_config_button_enable(HCB_POPUP);
	} else {
		hud_config_button_disable(HCB_POPUP);
	}
}

void hud_config_render_description()
{
	int w,h,sx,sy;

	if ( HC_gauge_selected >= 0 ) {
		gr_set_color_fast(&Color_normal);

		gr_get_string_size(&w, &h, HC_gauge_descriptions(HC_gauge_selected));
		sx = fl2i(HC_gauge_description_coords[gr_screen.res][0] + (HC_gauge_description_coords[gr_screen.res][2] - w)/2.0f);
		sy = HC_gauge_description_coords[gr_screen.res][1];
		gr_string(sx, sy, HC_gauge_descriptions(HC_gauge_selected));
	}
}

void hud_config_render_special_bitmaps()
{
	/*
	int i;
	for (i=1; i<NUM_HC_SPECIAL_BITMAPS; i++) {
		if (HC_special_bitmaps[i].bitmap >= 0) {
			gr_set_bitmap(HC_special_bitmaps[i].bitmap);
			gr_bitmap(HC_special_bitmaps[i].x, HC_special_bitmaps[i].y);
		}
	}
	*/
}

// update HUD_color_alpha based on brightness slider
void hud_config_update_brightness()
{
	// HUD_color_alpha = HC_sliders[gr_screen.res][HC_BRIGHTNESS_SLIDER].slider.pos+3;
	// Assert(HUD_color_alpha >= HUD_COLOR_ALPHA_USER_MIN);
	// Assert(HUD_color_alpha <= HUD_COLOR_ALPHA_USER_MAX);
}

// redraw any pressed buttons, needed since the glow on pressed buttons might get clipped off by
// adjacent buttons otherwise
void hud_config_redraw_pressed_buttons()
{
	int			i;
	UI_BUTTON	*b;

	for ( i = 0; i < NUM_HUD_BUTTONS; i++ ) {
		b = &HC_buttons[gr_screen.res][i].button;
		if ( b->button_down() ) {
			b->draw_forced(2);
		}
	}
}

// hud_config_do_frame() is called from the main freespace loop while the game is in the state
// GS_STATE_HUD_CONFIG.
//
void hud_config_do_frame(float frametime) 
{
	int k;

	if (!HUD_config_inited) {
		hud_config_init();
	}

	HC_gauge_hot = -1;

	hud_config_set_button_state();

	k = HC_ui_window.process();

	hud_config_handle_keypresses(k);
	hud_config_check_regions();
	hud_config_check_buttons();
	hud_config_update_brightness();

	// set the background
	GR_MAYBE_CLEAR_RES(HC_background_bitmap);
	if ( HC_background_bitmap > 0 ) {
		gr_set_bitmap(HC_background_bitmap);
		gr_bitmap(0,0);
	}

	// rgb slider/button stuff
	hud_config_process_colors();

	HC_ui_window.draw();
	hud_config_redraw_pressed_buttons();

	hud_config_draw_gauge_status();
	hud_config_draw_color_status();

	/*
	if (HC_special_bitmaps[HC_SPECIAL_RETICLE].bitmap >= 0) {
		hud_set_default_color();
		gr_set_bitmap(HC_special_bitmaps[HC_SPECIAL_RETICLE].bitmap);
		gr_aabitmap(HC_special_bitmaps[HC_SPECIAL_RETICLE].x, HC_special_bitmaps[HC_SPECIAL_RETICLE].y);
	}
	*/

	// maybe force draw the select all button
	if(HC_select_all){
		HC_buttons[gr_screen.res][HCB_SELECT_ALL].button.draw_forced(2);
	}

	hud_config_render_gauges();
	hud_config_render_special_bitmaps();
	hud_config_render_description();	

	gr_flip();
}

void hud_config_unload_gauges()
{
	int					i;
	HC_gauge_region	*hg;

	for (i=0; i<NUM_HUD_GAUGES; i++) {
		hg = &HC_gauge_regions[gr_screen.res][i];

		if ( hg->bitmap >= 0 ) {
			bm_release(hg->bitmap);
		}

		hg->bitmap=-1;
		hg->nframes=0;
	}
}

// hud_config_close() is called when the player leaves the hud configuration screen
//
void hud_config_close()
{
//	common_free_interface_palette();		// restore game palette
	hud_config_unload_gauges();

	hud_config_color_close();

	HC_ui_window.destroy();
}

// hud_set_default_hud_config() will set the hud configuration to default values
void hud_set_default_hud_config(player *p)
{
	int idx;

	HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;
	HUD_config.main_color = HUD_COLOR_GREEN;
	HUD_color_red = HC_colors[HUD_config.main_color].r;
	HUD_color_green = HC_colors[HUD_config.main_color].g;
	HUD_color_blue = HC_colors[HUD_config.main_color].b;

	for(idx=0; idx<NUM_HUD_GAUGES; idx++){
		gr_init_alphacolor(&HUD_config.clr[idx], HUD_color_red, HUD_color_green, HUD_color_blue, (HUD_color_alpha+1)*16);
	}

	HUD_config.show_flags = HUD_config_default_flags;
	HUD_config.show_flags2 = HUD_config_default_flags2;
	HUD_config.popup_flags = HUD_default_popup_mask;
	HUD_config.popup_flags2 = HUD_default_popup_mask2;
	HUD_config.num_msg_window_lines			= 4;	// one more than is actually visible
	HUD_config.rp_flags = RP_DEFAULT;
	HUD_config.rp_dist = RR_INFINITY;
	HUD_config.is_observer = 0;
}

// hud_config_restore() will restore the hud configuration the player started with when the 
// hud configuration screen was started
//
void hud_config_restore()
{
	HUD_config = HUD_config_backup;
}

// hud_config_backup() will save the players hud configuration when they enter the hud configuration
// screen.  This is done in case the player decides to cancel the changes that were made.
//
void hud_config_backup()
{
	HUD_config_backup = HUD_config;
}

void hud_config_as_observer(ship *shipp,ai_info *aif)
{
	// store the current hus
	hud_config_backup();

	// bash these values so the HUD is not offset incorrectly
	HUD_offset_x = 0.0f;
	HUD_offset_y = 0.0f;

	// initialize the observer HUD
	hud_observer_init(shipp,aif);	
}

void hud_config_as_player()
{
} 

// ---------------------------------------------------------------------------------------------------------------
// RGB color stuff
//

void hud_config_color_save(char *name)
{
	int idx;
	CFILE *out = cfopen(name, "wt", CFILE_NORMAL, CF_TYPE_PLAYERS);
	char vals[255] = "";

	// bad
	if(out == NULL){
		Int3();
		return;
	}	

	// write out all gauges
	for(idx=0; idx<NUM_HUD_GAUGES; idx++){
		cfputs("+Gauge: ", out);
		cfputs(HC_gauge_descriptions(idx), out);		
		cfputs("\n", out);
		cfputs("+RGBA: ", out);
		sprintf(vals, "%d %d %d %d\n\n", HUD_config.clr[idx].red, HUD_config.clr[idx].green, HUD_config.clr[idx].blue, HUD_config.clr[idx].alpha);
		cfputs(vals, out);
	}
	
	// close the file
	cfclose(out);	
}

void hud_config_color_load(char *name)
{
	int idx, rval;
	char str[1024];
	char *fname;

	fname = cf_add_ext(name, ".hcf");

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("HUDCONFIG: Unable to parse '%s'!  Error code = %i.\n", fname, rval));
		return;
	}

	read_file_text(fname);
	reset_parse();

	// write out all gauges
	for(idx=0; idx<NUM_HUD_GAUGES; idx++){		
		required_string("+Gauge:");
		stuff_string(str, F_NAME, sizeof(str));

		required_string("+RGBA:");
		stuff_ubyte(&HUD_config.clr[idx].red);
		stuff_ubyte(&HUD_config.clr[idx].green);
		stuff_ubyte(&HUD_config.clr[idx].blue);
		stuff_ubyte(&HUD_config.clr[idx].alpha);
	}	
}

void hud_config_alpha_slider_up()
{	
	int pos = HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem());
	int max = MAX(MAX( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) ), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) );

	// if this would put the brightest element past its limit, skip
	if(max >= 255){
		HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV(pos - 1) );
		return;
	}
	
	// otherwise bump everybody up by one
	HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) + 1) );	
	HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) + 1) );
	HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) + 1) );
	
	// apply
	if(HC_select_all){
		hud_config_stuff_colors( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) );
	} else {
		gr_init_alphacolor(&HUD_config.clr[HC_gauge_selected], HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()), 255);
	}
}

void hud_config_alpha_slider_down()
{	
	int pos = HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem());
	int min = MIN(MIN( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) ), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) );

	// if this would put the brightest element past its limit, skip
	if(min <= 0){
		HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV(pos + 1) );
		return;
	}
	
	// otherwise bump everybody up by one
	HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) - 1) );	
	HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) - 1) );
	HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) - 1) );	

	// apply
	if(HC_select_all){
		hud_config_stuff_colors( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) );
	} else {
		gr_init_alphacolor(&HUD_config.clr[HC_gauge_selected], HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()), 255);
	}
}

void hud_config_recalc_alpha_slider()
{
	int avg =HC_color_sliders[HCS_RED].get_currentItem() + HC_color_sliders[HCS_GREEN].get_currentItem() + HC_color_sliders[HCS_BLUE].get_currentItem();
	avg /= 3;
	HC_color_sliders[HCS_ALPHA].force_currentItem( avg );
}

void hud_config_red_slider()
{
	int idx;
	int pos = HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) ;

	// select all ?
	if(HC_select_all){
		for(idx=0; idx<NUM_HUD_GAUGES; idx++){
			gr_init_alphacolor(&HUD_config.clr[idx], pos, HUD_config.clr[idx].green, HUD_config.clr[idx].blue, HUD_config.clr[idx].alpha);
		}
	}
	// individual gauge
	else {
		if(HC_gauge_selected < 0){
			return;
		}

		// get slider position	
		gr_init_alphacolor(&HUD_config.clr[HC_gauge_selected], pos, HUD_config.clr[HC_gauge_selected].green, HUD_config.clr[HC_gauge_selected].blue, HUD_config.clr[HC_gauge_selected].alpha);
	}	

	hud_config_recalc_alpha_slider();
}

void hud_config_green_slider()
{
	int idx;
	int pos = HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) ;

	// select all ?
	if(HC_select_all){
		for(idx=0; idx<NUM_HUD_GAUGES; idx++){
			gr_init_alphacolor(&HUD_config.clr[idx], HUD_config.clr[idx].red, pos, HUD_config.clr[idx].blue, HUD_config.clr[idx].alpha);
		}
	}
	// individual gauge
	else {
		if(HC_gauge_selected < 0){
			return;
		}

		// get slider position	
		gr_init_alphacolor(&HUD_config.clr[HC_gauge_selected], HUD_config.clr[HC_gauge_selected].red, pos, HUD_config.clr[HC_gauge_selected].blue, HUD_config.clr[HC_gauge_selected].alpha);
	}	

	hud_config_recalc_alpha_slider();
}

void hud_config_blue_slider()
{
	int idx;
	int pos = HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem());

	// select all ?
	if(HC_select_all){
		for(idx=0; idx<NUM_HUD_GAUGES; idx++){
			gr_init_alphacolor(&HUD_config.clr[idx], HUD_config.clr[idx].red, HUD_config.clr[idx].green, pos, HUD_config.clr[idx].alpha);
		}
	}
	// individual gauge
	else {
		if(HC_gauge_selected < 0){
			return;
		}

		// get slider position	
		gr_init_alphacolor(&HUD_config.clr[HC_gauge_selected], HUD_config.clr[HC_gauge_selected].red, HUD_config.clr[HC_gauge_selected].green, pos, HUD_config.clr[HC_gauge_selected].alpha);
	}	

	hud_config_recalc_alpha_slider();
}

void hud_config_process_colors()
{	
}

void hud_config_color_init()
{
	HC_current_file = -1;

	// get a list of all hcf files
	memset(HC_filenames, 0, sizeof(char*) * MAX_HCF_FILES);
	HC_num_files = cf_get_file_list(MAX_HCF_FILES, HC_filenames, CF_TYPE_PLAYERS, "*.hcf", CF_SORT_NAME);
}

void hud_config_color_close()
{
	int idx;

	// free all 
	for(idx=0; idx<HC_num_files; idx++){
		if(HC_filenames[idx] != NULL){
			vm_free(HC_filenames[idx]);
			HC_filenames[idx] = NULL;
		}
	}
}

void hud_config_select_all_toggle(int toggle)
{	
	int r, g, b, a;

	// if we're turning off
	if(!toggle){				
		// determine if on/off/popup buttons should be shown
		hud_config_set_button_state();

		HC_select_all = 0;
	} else {
		// synch stuff up
		hud_config_synch_ui();
		
		// if we had a gauge previously selected, use its color everywhere
		if(HC_gauge_selected < 0){
			r = HUD_config.clr[HUD_RADAR].red;
			g = HUD_config.clr[HUD_RADAR].green;
			b = HUD_config.clr[HUD_RADAR].blue;			
			a = HUD_config.clr[HUD_RADAR].alpha;
		} else {
			r = HUD_config.clr[HC_gauge_selected].red;
			g = HUD_config.clr[HC_gauge_selected].green;
			b = HUD_config.clr[HC_gauge_selected].blue;			
			a = HUD_config.clr[HC_gauge_selected].alpha;
		}
		hud_config_stuff_colors(r, g, b);

		// no gauge selected
		HC_gauge_selected = -1;		

		// enable all sliders
		HC_color_sliders[HCS_RED].enable();
		HC_color_sliders[HCS_GREEN].enable();
		HC_color_sliders[HCS_BLUE].enable();
		HC_color_sliders[HCS_ALPHA].enable();
		HC_color_sliders[HCS_RED].unhide();
		HC_color_sliders[HCS_GREEN].unhide();
		HC_color_sliders[HCS_BLUE].unhide();
		HC_color_sliders[HCS_ALPHA].unhide();
		HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV(r) );
		HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV(g) );
		HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV(b) );
		HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV(a) );

		// recalc alpha
		hud_config_recalc_alpha_slider();

		// disable all three buttons
		hud_config_button_disable(HCB_ON);
		hud_config_button_disable(HCB_OFF);
		hud_config_button_disable(HCB_POPUP);

		HC_select_all = 1;
	}
}

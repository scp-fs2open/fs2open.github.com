/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "cfile/cfile.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/pstypes.h"
#include "hud/hudconfig.h"
#include "hud/hudobserver.h"
#include "hud/hudshield.h"
#include "iff_defs/iff_defs.h"
#include "io/key.h"
#include "io/mouse.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "scripting/scripting.h"
#include "scripting/global_hooks.h"
#include "ship/ship.h"
#include "ui/ui.h"


//////////////////////////////////////////////////////////////////////////////
// Game-wide Globals
//////////////////////////////////////////////////////////////////////////////

int HC_current_file = -1;					// current hcf file
SCP_vector<SCP_string> HC_preset_filenames;

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


// specify the max distance that the radar should detect objects
// See RR_ #defines in HUDconfig.h.

float Radar_ranges[RR_MAX_RANGES] = {
	2000.0f,		// short
	10000.0f,		// med
	10000000.0f,	// infinity
};

const char *Radar_range_text(int n)
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

// Default enabled HUD gauges (Observer Mode)
SCP_vector<SCP_string> observer_visible_gauges = {
	"Builtin::CenterOfReticle",
	"Builtin::OffscreenIndicator",
	"Builtin::MessageOutput",
	"Builtin::ClosestAttackingHostile",
	"Builtin::CurrentTargetDirection",
	"Builtin::TargetHullShieldIcon",
	"Builtin::TargetMonitor",
	"Builtin::OffscreenRange"
};

SCP_vector<SCP_string> default_visible_gauges = {
    "Builtin::LeadIndicator",
    "Builtin::TargetOrientation",
    "Builtin::ClosestAttackingHostile",
    "Builtin::CurrentTargetDirection",
    "Builtin::MissionTime",
    "Builtin::Reticle",
    "Builtin::Throttle",
    "Builtin::Radar",
    "Builtin::TargetMonitor",
    "Builtin::CenterOfReticle",
    "Builtin::ExtraTargetInfo",
    "Builtin::TargetShield",
    "Builtin::PlayerShield",
    "Builtin::PowerManagement",
    "Builtin::AutoTargetIcon",
    "Builtin::AutoSpeedMatchIcon",
    "Builtin::WeaponsDisplay",
    "Builtin::MonitoringView",
    "Builtin::DirectivesView",
    "Builtin::ThreatGauge",
    "Builtin::AfterburnerEnergy",
    "Builtin::WeaponsEnergy",
    "Builtin::WeaponLinking",
    "Builtin::TargetHullShieldIcon",
    "Builtin::OffscreenIndicator",
    "Builtin::CommVideo",
    "Builtin::DamageDisplay",
    "Builtin::MessageOutput",
    "Builtin::LockedMissileDirection",
    "Builtin::Countermeasures",
    "Builtin::ObjectiveNotify",
    "Builtin::WingmenStatus",
    "Builtin::OffscreenRange",
    "Builtin::KillsGauge",
    "Builtin::AttackingTargetCount",
    "Builtin::SupportGauge",
    "Builtin::LagGauge",
    "Builtin::WarningFlash",
    "Builtin::CommMenu"
};

// Can be customized in hud_gauges.tbl
char HC_wingam_gauge_status_names[MAX_SQUADRON_WINGS][32] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};

bool HC_select_all = false;

//////////////////////////////////////////////////////////////////////////////
// Module Globals
//////////////////////////////////////////////////////////////////////////////

// Coordinates for the new HUD configuration menu
const int HC_gauge_config_coords[GR_NUM_RESOLUTIONS][4] = {
	{121, 615, 6, 371}, // Coordinates for 640x480
	{195, 985, 10, 595} // Coordinates for 1024x768
};

const char *Hud_config_fname[GR_NUM_RESOLUTIONS] = {
	"HUDConfig",
	"2_HUDConfig"
};

const char *Hud_config_mask_fname[GR_NUM_RESOLUTIONS] = {
	"HUDConfig-m",
	"2_HUDConfig-m"
};

// keep a list of gauge pointers so we can easily get information from them
SCP_vector<std::pair<SCP_string, HudGauge*>> HC_gauge_map;
bool HC_gauge_list_clear = true;

int HC_gauge_description_coords[GR_NUM_RESOLUTIONS][3] = {
	{	// GR_640
		35, 397, 197
	},
	{	// GR_1024
		56, 632, 307
	}
};

int HC_talking_head_frame = -1;
SCP_string HC_head_anim_filename;
bool HC_show_default_hud = true;
std::unordered_set<SCP_string> HC_ignored_huds;
SCP_map<SCP_string, std::array<SCP_string, num_shield_gauge_types>> HC_hud_shield_ships;
SCP_map<SCP_string, SCP_vector<SCP_string>> HC_hud_primary_weapons;
SCP_map<SCP_string, SCP_vector<SCP_string>> HC_hud_secondary_weapons;

int HC_resize_mode = GR_RESIZE_MENU;

SCP_vector<std::pair<size_t, SCP_string>> HC_available_huds;
int HC_chosen_hud = -1;

SCP_string HC_arrow_bitmaps[2][2][2] = {
	{{"BAB_030001", "BAB_030002"}, {"BAB_040001", "BAB_040002"}},
	{{"2_BAB_030001", "2_BAB_030002"}, {"2_BAB_040001", "2_BAB_040002"}}
};
int HC_arrow_bm_handles[2][2] = {{-1, -1}, {-1, -1}};
std::pair<int, int> HC_arrow_coords[2][2] = {
	{
		{566, 368},
		{603, 368},
	},
	{
		{911, 581},
		{967, 581},
	}
};
int HC_arrow_hot = -1;

#define NUM_HUD_BUTTONS   20

#define HCB_RED_UP        0
#define HCB_GREEN_UP      1
#define HCB_BLUE_UP       2
#define HCB_I_UP          3
#define HCB_RED_DOWN      4
#define HCB_GREEN_DOWN    5
#define HCB_BLUE_DOWN     6
#define HCB_I_DOWN        7
#define HCB_ON            8
#define HCB_OFF           9
#define HCB_POPUP         10
#define HCB_SAVE_HCF      11
#define HCB_PREV_HCF      12
#define HCB_NEXT_HCF      13
#define HCB_AMBER         14
#define HCB_BLUE          15
#define HCB_GREEN         16
#define HCB_SELECT_ALL    17
#define HCB_RESET         18
#define HCB_ACCEPT        19


ui_button_info HC_buttons[GR_NUM_RESOLUTIONS][NUM_HUD_BUTTONS] = {
    { // GR_640
        ui_button_info("HCB_00",    6,    27,   -1, -1, 0),
        ui_button_info("HCB_01",    30,   27,   -1, -1, 1),
        ui_button_info("HCB_02",    55,   27,   -1, -1, 2),
        ui_button_info("HCB_03",    80,   27,   -1, -1, 3),
        ui_button_info("HCB_08",    6,    291,  -1, -1, 8),
        ui_button_info("HCB_09",    30,   291,  -1, -1, 9),
        ui_button_info("HCB_10",    55,   291,  -1, -1, 10),
        ui_button_info("HCB_11",    80,   291,  -1, -1, 11),
        ui_button_info("HCB_12",    4,    329,  -1, -1, 12),
        ui_button_info("HCB_13",    4,    348,  -1, -1, 13),
        ui_button_info("HCB_14",    4,    367,  -1, -1, 14),
        ui_button_info("HCB_15",    2,    439,  -1, -1, 15),
        ui_button_info("HCB_16",    266,  456,  -1, -1, 16),
        ui_button_info("HCB_17",    292,  456,  -1, -1, 17),
        ui_button_info("HCB_18",    327,  421,  -1, -1, 18),
        ui_button_info("HCB_19",    327,  440,  -1, -1, 19),
        ui_button_info("HCB_20",    327,  459,  -1, -1, 20),
        ui_button_info("HCB_24",    472,  436,  -1, -1, 24),
        ui_button_info("HCB_25",    523,  433,  -1, -1, 25),
        ui_button_info("HCB_26",    576,  434,  -1, -1, 26),
    },
    { // GR_1024
        ui_button_info("2_HCB_00",  9,    44,   -1, -1, 0),
        ui_button_info("2_HCB_01",  48,   44,   -1, -1, 1),
        ui_button_info("2_HCB_02",  88,   44,   -1, -1, 2),
        ui_button_info("2_HCB_03",  127,  44,   -1, -1, 3),
        ui_button_info("2_HCB_08",  9,    466,  -1, -1, 8),
        ui_button_info("2_HCB_09",  48,   466,  -1, -1, 9),
        ui_button_info("2_HCB_10",  88,   466,  -1, -1, 10),
        ui_button_info("2_HCB_11",  127,  466,  -1, -1, 11),
        ui_button_info("2_HCB_12",  6,    526,  -1, -1, 12),
        ui_button_info("2_HCB_13",  6,    556,  -1, -1, 13),
        ui_button_info("2_HCB_14",  6,    586,  -1, -1, 14),
        ui_button_info("2_HCB_15",  3,    703,  -1, -1, 15),
        ui_button_info("2_HCB_16",  426,  730,  -1, -1, 16),
        ui_button_info("2_HCB_17",  467,  730,  -1, -1, 17),
        ui_button_info("2_HCB_18",  524,  674,  -1, -1, 18),
        ui_button_info("2_HCB_19",  524,  704,  -1, -1, 19),
        ui_button_info("2_HCB_20",  524,  734,  -1, -1, 20),
        ui_button_info("2_HCB_24",  755,  698,  -1, -1, 24),
        ui_button_info("2_HCB_25",  837,  693,  -1, -1, 25),
        ui_button_info("2_HCB_26",  922,  695,  -1, -1, 26),
    },
};

// text
#define NUM_HUD_TEXT					15
UI_XSTR HC_text[GR_NUM_RESOLUTIONS][NUM_HUD_TEXT] = {
    { // GR_640
        { "R",              1512,   14,     8,      UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "G",              1513,   37,     8,      UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "B",              1514,   62,     8,      UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "I",              1515,   90,     8,      UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "On",             1285,   36,     334,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_ON].button },
        { "Off",            1286,   36,     353,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_OFF].button },
        { "Popup",          1453,   36,     372,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_POPUP].button },
        { "Save",           1454,   51,     428,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_SAVE_HCF].button },
        { "Amber",          1455,   364,    426,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_AMBER].button },
        { "Blue",           1456,   364,    445,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_BLUE].button },
        { "Green",          1457,   364,    464,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_GREEN].button },     
        { "Select",         1550,   442,    413,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_SELECT_ALL].button },
        { "All",            1551,   442,    424,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_SELECT_ALL].button },
        { "Reset",          1337,   515,    413,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[0][HCB_RESET].button },
        { "Accept",         1035,   573,    413,    UI_XSTR_COLOR_PINK,     -1, &HC_buttons[0][HCB_ACCEPT].button },
    },
    { // GR_1024
        { "R",              1512,   23,     14,     UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "G",              1513,   60,     14,     UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "B",              1514,   100,    14,     UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "I",              1515,   144,    14,     UI_XSTR_COLOR_GREEN,    -1, nullptr },
        { "On",             1285,   58,     536,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_ON].button },
        { "Off",            1286,   58,     566,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_OFF].button },
        { "Popup",          1453,   58,     596,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_POPUP].button },
        { "Save",           1454,   82,     688,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_SAVE_HCF].button },
        { "Amber",          1455,   582,    685,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_AMBER].button },
        { "Blue",           1456,   582,    715,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_BLUE].button },
        { "Green",          1457,   582,    745,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_GREEN].button },     
        { "Select",         1550,   760,    671,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_SELECT_ALL].button },
        { "All",            1551,   760,    682,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_SELECT_ALL].button },
        { "Reset",          1337,   850,    669,    UI_XSTR_COLOR_GREEN,    -1, &HC_buttons[1][HCB_RESET].button },
        { "Accept",         1035,   930,    670,    UI_XSTR_COLOR_PINK,     -1, &HC_buttons[1][HCB_ACCEPT].button },
    }
};

static int							HC_background_bitmap;
static int							HC_background_bitmap_mask;
static UI_WINDOW					HC_ui_window;

SCP_string							HC_gauge_hot;			// mouse is over this gauge
SCP_string							HC_gauge_selected;	// gauge is selected
int HC_gauge_coordinates[6]; // x1, x2, y1, y1, w, h of the example HUD render area. Used for calculating new gauge coordinates
SCP_vector<std::pair<SCP_string, BoundingBox>> HC_gauge_mouse_coords;

// Names and XSTR IDs for these come from HC_text above
hc_col HC_colors[NUM_HUD_COLOR_PRESETS] =
{
	{0, 255, 0, "", -1},    // Green
	{67, 123, 203, "", -1}, // Blue
	{255, 197, 0, "", -1},  // Amber
};

int HC_default_color = HUD_COLOR_PRESET_1;
SCP_string HC_default_preset_file = "hud_3.hcf";

static HUD_CONFIG_TYPE	HUD_config_backup;		// backup HUD config, used to restore old config if changes not applied
static int				HUD_config_inited = 0;

// rgba slider stuff
void hud_config_red_slider();
void hud_config_green_slider();
void hud_config_blue_slider();
void hud_config_alpha_slider_up();
void hud_config_alpha_slider_down();
void hud_config_recalc_alpha_slider();
void hud_config_process_colors();
#define NUM_HC_SLIDERS			4
#define HCS_RED					0
#define HCS_GREEN					1
#define HCS_BLUE					2
#define HCS_ALPHA					3
UI_SLIDER2 HC_color_sliders[NUM_HC_SLIDERS];
int HC_slider_coords[GR_NUM_RESOLUTIONS][NUM_HC_SLIDERS][4] = {
	{ // GR_640
		{ 8,    53,	15, 225 },
		{ 33,   53,	15, 225 },
		{ 58,   53,	15, 225 },
		{ 83,   53,	15, 225 },
	},
	{ // GR_1024
		{ 13,	85, 32, 350 },
		{ 53, 85, 32, 350 },
		{ 93, 85, 32, 350 },
		{ 133, 85, 32, 350 },
	},	
};
#define HCS_CONV(__v)			( 255 - (__v) )

const char *HC_slider_fname[GR_NUM_RESOLUTIONS] = {
	"slider",
	"2_slider"
};

// Used throughout this file to map from retail gauge ids (numeric or HCF) to the newer internal string format
const auto& gauge_map = HC_gauge_mappings::get_instance();

HudGauge* hud_config_get_gauge_pointer(const SCP_string& gauge_id)
{
	for (auto& pair : HC_gauge_map) {
		if (pair.first == gauge_id) {
			return pair.second;
		}
	}
	return nullptr;
}

// sync sliders
void hud_config_synch_sliders(const SCP_string& gauge)
{
	if(!gauge.empty()){
		color clr = HUD_config.get_gauge_color(gauge);
		HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV(clr.red) );
		HC_color_sliders[HCS_GREEN].force_currentItem(HCS_CONV(clr.green));
		HC_color_sliders[HCS_BLUE].force_currentItem(HCS_CONV(clr.blue));
		HC_color_sliders[HCS_ALPHA].force_currentItem(HCS_CONV(clr.alpha));
	}
}

/*!
 * @brief reset some ui components based on HUD config data
 *
 * param[in] API_Access		whether or not this method has been called from the lua api
 */
void hud_config_synch_ui(bool API_Access)
{
	HUD_init_hud_color_array();
	// HC_sliders[gr_screen.res][HC_BRIGHTNESS_SLIDER].slider.pos = HUD_color_alpha-3;		// convert to value from 0-10	

	// sync sliders to currently selected gauge
	if (!API_Access) {
		hud_config_synch_sliders(HC_gauge_selected);
	}
}

void hud_config_init_dimensions(int x1, int x2, int y1, int y2)
{
	// Calculate the menu width and height
	int menuWidth = x2 - x1;
	int menuHeight = y2 - y1;

	HC_gauge_coordinates[0] = x1;
	HC_gauge_coordinates[1] = x2;
	HC_gauge_coordinates[2] = y1;
	HC_gauge_coordinates[3] = y2;
	HC_gauge_coordinates[4] = menuWidth;
	HC_gauge_coordinates[5] = menuHeight;
}

void hud_config_get_unique_huds()
{
	std::unordered_set<SCP_string> seenHuds; // Tracks HUDs we've already encountered

	for (const auto& pair : Hud_parsed_ships) {
		const auto& hudName = pair.first; // Extract the HUD name
		if (seenHuds.find(hudName) == seenHuds.end()) {
			// If this HUD hasn't been encountered, maybe add it to the result
			if (HC_ignored_huds.find(hudName) != HC_ignored_huds.end()) {
				// Skip ignored HUDs
				continue;
			}

			std::pair<size_t, SCP_string> newPair;
			newPair.second = hudName;

			// Get the ship index associated
			for (size_t i = 0; i < Ship_info.size(); i++) {
				if (!stricmp(Ship_info[i].name, pair.second.c_str())) {
					newPair.first = i;
					break;
				}
			}

			HC_available_huds.push_back(newPair);
			seenHuds.insert(hudName);
		}
	}
}

/*!
 * @brief init the UI components
 *
 * param[in] API_Access		whether or not this method has been called from the lua api
 * param[in] x				the x coord to render the preview display
 * param[in] y				the y coord to render the preview display
 * param[in] w				the width of the preview display
 */
void hud_config_init_ui(bool API_Access, int x, int y, int w, int h)
{
	struct ui_button_info			*hb;

	HC_gauge_mouse_coords.clear();

	hud_config_get_unique_huds();

	if (!HC_show_default_hud) {
		if (HC_available_huds.empty()) {
			HC_show_default_hud = true;
			HC_chosen_hud = -1;
		} else {
			HC_chosen_hud = 0;
		}
	}

	if (!HC_show_default_hud && HC_available_huds.empty()) {
		HC_show_default_hud = true;
	}

	if (w < 0 || h < 0) {
		hud_config_init_dimensions(HC_gauge_config_coords[gr_screen.res][0],
			HC_gauge_config_coords[gr_screen.res][1],
			HC_gauge_config_coords[gr_screen.res][2],
			HC_gauge_config_coords[gr_screen.res][3]);
		HC_resize_mode = GR_RESIZE_MENU;
	} else {

		hud_config_init_dimensions(x, x + w, y, y + h);
		if (API_Access) {
			HC_resize_mode = GR_RESIZE_NONE;
		}
	}

	hud_config_synch_ui(API_Access);

	if (!API_Access) {
		HC_background_bitmap = bm_load(Hud_config_fname[gr_screen.res]);
		if (HC_background_bitmap < 0) {
			Warning(LOCATION, "Error loading HUD config menu background %s", Hud_config_fname[gr_screen.res]);
		}

		HC_ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);

		HC_background_bitmap_mask = bm_load(Hud_config_mask_fname[gr_screen.res]);
		if (HC_background_bitmap_mask < 0) {
			Warning(LOCATION, "Error loading HUD config menu mask %s", Hud_config_mask_fname[gr_screen.res]);
			return;
		} else {
			HC_ui_window.set_mask_bmap(Hud_config_mask_fname[gr_screen.res]);
		}

		HC_arrow_bm_handles[0][0] = bm_load(HC_arrow_bitmaps[gr_screen.res][0][0]);
		HC_arrow_bm_handles[0][1] = bm_load(HC_arrow_bitmaps[gr_screen.res][0][1]);
		HC_arrow_bm_handles[1][0] = bm_load(HC_arrow_bitmaps[gr_screen.res][1][0]);
		HC_arrow_bm_handles[1][1] = bm_load(HC_arrow_bitmaps[gr_screen.res][1][1]);
	}

	if (!API_Access){
		// Get our custom color names ready and add text
		for (int i = 0; i < NUM_HUD_TEXT; i++) {
			if (i >= 8 && i <= 10) {    // Check if it's a color index which are hardcoded to 8, 9, 10.
				int colorIndex = 10 - i; // Calculate the index for HC_colors
				if (!HC_colors[colorIndex].name.empty()) {

					// UI_XSTR holds strings as const char so without making huge changes to the codebase
					// we instead create a temp UI_XSTR with the new name
					UI_XSTR temp = {
						HC_colors[colorIndex].name.c_str(),
						HC_colors[colorIndex].xstr,
						HC_text[gr_screen.res][i].x,
						HC_text[gr_screen.res][i].y,
						HC_text[gr_screen.res][i].clr,
						HC_text[gr_screen.res][i].font_id,
						HC_text[gr_screen.res][i].assoc,
					};
					HC_ui_window.add_XSTR(&temp);
				} else {
					// Add the original string
					HC_ui_window.add_XSTR(&HC_text[gr_screen.res][i]);
				}
			} else {
				// Add the original string
				HC_ui_window.add_XSTR(&HC_text[gr_screen.res][i]);
			}
		}

		// initialize sliders
		HC_color_sliders[HCS_RED].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_RED][0], HC_slider_coords[gr_screen.res][HCS_RED][1], HC_slider_coords[gr_screen.res][HCS_RED][2], HC_slider_coords[gr_screen.res][HCS_RED][3],
											255, HC_slider_fname[gr_screen.res], hud_config_red_slider, hud_config_red_slider, hud_config_red_slider);

		HC_color_sliders[HCS_GREEN].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_GREEN][0], HC_slider_coords[gr_screen.res][HCS_GREEN][1], HC_slider_coords[gr_screen.res][HCS_GREEN][2], HC_slider_coords[gr_screen.res][HCS_GREEN][3],
											255, HC_slider_fname[gr_screen.res], hud_config_green_slider, hud_config_green_slider, hud_config_green_slider);

		HC_color_sliders[HCS_BLUE].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_BLUE][0], HC_slider_coords[gr_screen.res][HCS_BLUE][1], HC_slider_coords[gr_screen.res][HCS_BLUE][2], HC_slider_coords[gr_screen.res][HCS_BLUE][3],
											255, HC_slider_fname[gr_screen.res], hud_config_blue_slider, hud_config_blue_slider, hud_config_blue_slider);

		HC_color_sliders[HCS_ALPHA].create(&HC_ui_window, HC_slider_coords[gr_screen.res][HCS_ALPHA][0], HC_slider_coords[gr_screen.res][HCS_ALPHA][1], HC_slider_coords[gr_screen.res][HCS_ALPHA][2], HC_slider_coords[gr_screen.res][HCS_ALPHA][3],
											255, HC_slider_fname[gr_screen.res], hud_config_alpha_slider_up, hud_config_alpha_slider_down, nullptr);

		// now disable them until the player clicks on something
		HC_color_sliders[HCS_RED].hide();
		HC_color_sliders[HCS_GREEN].hide();
		HC_color_sliders[HCS_BLUE].hide();
		HC_color_sliders[HCS_ALPHA].hide();

		HC_color_sliders[HCS_RED].disable();
		HC_color_sliders[HCS_GREEN].disable();
		HC_color_sliders[HCS_BLUE].disable();
		HC_color_sliders[HCS_ALPHA].disable();
	}
	
	hud_config_preset_init();

	if (!API_Access) {
		for (int i = 0; i < NUM_HUD_BUTTONS; i++) {
			hb = &HC_buttons[gr_screen.res][i];
			hb->button.create(&HC_ui_window, "", hb->x, hb->y, 60, 30, 0, 1);
			// set up callback for when a mouse first goes over a button
			hb->button.set_bmaps(hb->filename);
			hb->button.set_highlight_action(common_play_highlight_sound);
			hb->button.link_hotspot(hb->hotspot);
		}

		// config file input name
		HC_fname_input.create(&HC_ui_window,
			HC_fname_coords[gr_screen.res][0],
			HC_fname_coords[gr_screen.res][1],
			HC_fname_coords[gr_screen.res][2],
			MAX_FILENAME_LEN,
			"",
			UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_ESC_FOC);
		HC_fname_input.set_text("");

		HC_gauge_hot.clear();
		HC_gauge_selected.clear();

		HC_select_all = false;
	}
}

// Used for debugging only
void hud_config_draw_box(int x1, int x2, int y1, int y2)
{
	gr_line(x1, y1, x1, y2, HC_resize_mode); // Left vertical line
	gr_line(x1, y1, x2, y1, HC_resize_mode); // Top horizontal line
	gr_line(x2, y1, x2, y2, HC_resize_mode); // Right vertical line
	gr_line(x1, y2, x2, y2, HC_resize_mode); // Bottom horizontal line
}

void hud_config_set_mouse_coords(const SCP_string& gauge_id, int x1, int x2, int y1, int y2)
{
	BoundingBox newBox(x1, x2, y1, y2);

	// There is a complicated issue here that I have not yet solved.
	// Gauges like the missile warning will search each frame to find a position in HUD Config
	// that does not overlap with other gauges so they can be clicked on. This may take a few frames
	// for each auto-positioned gauge to get sorted out and they need to be able to update their mouse coords
	// for each of those frames.
	// 
	// However, the newer ETS gauge can have up to 3 distinct sets of mouse coordinates. It's an unfortunate
	// side-effect of splitting each system of the ETS into a distinct gauge that shares the same gauge ID.
	// There is the possibility of a newer gauge type added in the future at has multiple gauges like ETS and
	// also needs to auto-position. If that's the case then this code will cause bugs. As it is now there are
	// no gauges that fit that description.
	// 
	// I think the long term solution can happen when HUD Config support is added to custom gauges. The 3 distinct 
	// ETS gauges at that point can be given unique IDs matching however custom gauges are handled with care taken
	// in regards to backwards compatibility for that special case. When that happens, HC_gauge_mouse_coords can be
	// changed to enforce only 1 set of mouse coords per gauge. - Mjn
	auto it = std::find_if(HC_gauge_mouse_coords.begin(),
		HC_gauge_mouse_coords.end(),
		[gauge_id](const std::pair<SCP_string, BoundingBox>& item) { return item.first == gauge_id; });

	if (it != HC_gauge_mouse_coords.end()) {
		if (it->second == newBox) {
			return; // No change, early exit
		}
		it->second = newBox; // Replace existing
		return;
	}

	HC_gauge_mouse_coords.emplace_back(std::make_pair(gauge_id, newBox));
}

std::pair<int, int> hud_config_convert_coords(int x, int y, float scale)
{
	int outX = HC_gauge_coordinates[0] + static_cast<int>(x * scale);
	int outY = HC_gauge_coordinates[2] + static_cast<int>(y * scale);

	return std::make_pair(outX, outY);
}

std::pair<float, float> hud_config_convert_coords(float x, float y, float scale)
{
	float outX = HC_gauge_coordinates[0] + x * scale;
	float outY = HC_gauge_coordinates[2] + y * scale;

	return std::make_pair(outX, outY);
}

float hud_config_get_scale(int baseW, int baseH)
{
	// Determine the scaling factor
	float scaleX = static_cast<float>(HC_gauge_coordinates[4]) / baseW;
	float scaleY = static_cast<float>(HC_gauge_coordinates[5]) / baseH;

	// Use the smallest scale factor
	return std::min(scaleX, scaleY);
}

std::tuple<int, int, float> hud_config_convert_coord_sys(int x, int y, int baseW, int baseH)
{
	float scale = hud_config_get_scale(baseW, baseH);
	auto coords = hud_config_convert_coords(x, y, scale);

	return std::make_tuple(coords.first, coords.second, scale);
}

std::tuple<float, float, float> hud_config_convert_coord_sys(float x, float y, int baseW, int baseH)
{
	float scale = hud_config_get_scale(baseW, baseH);
	auto coords = hud_config_convert_coords(x, y, scale);

	return std::make_tuple(coords.first, coords.second, scale);
}

std::pair<float, float> hud_config_calc_coords_from_angle(float angle_degrees, int centerX, int centerY, float radius)
{
	// Convert angle to radians, adjust so 0 degrees is at the top (12 o'clock)
	float angle_radians = (angle_degrees + 90.0f) * static_cast<float>(M_PI) / 180.0f;

	// Offset to ensure the arrow points outward
	float adjusted_radius = radius + 4.0f;

	// Calculate offsets based on the adjusted radius and angle
	float xOffset = -cos(angle_radians) * adjusted_radius; // Negate to mirror direction
	float yOffset = sin(angle_radians) * adjusted_radius;

	// Map to screen coordinates (centerX and centerY represent the center of the circle)
	float screenX = centerX + xOffset;
	float screenY = centerY - yOffset;

	return {screenX, screenY};
}

float hud_config_find_valid_angle(const SCP_string& gauge, float initial_angle, int centerX, int centerY, float radius)
{
	const int max_iterations = 360; // Prevent infinite loops
	float angle = initial_angle;
	int x1, x2, y1, y2;

	for (int i = 0; i < max_iterations; ++i) {
		// Calculate coordinates for the current angle
		auto [screenX, screenY] = hud_config_calc_coords_from_angle(angle, centerX, centerY, radius);
		int boundingBoxSize = 10; // Guestimate
		x1 = fl2i(screenX - boundingBoxSize);
		x2 = fl2i(screenX + boundingBoxSize);
		y1 = fl2i(screenY - boundingBoxSize);
		y2 = fl2i(screenY + boundingBoxSize);

		BoundingBox newBox = {x1, x2, y1, y2};

		// Check for overlap
		if (!BoundingBox::isOverlappingAny(HC_gauge_mouse_coords, newBox, gauge)) {
			return angle;
		}

		// Increment angle and try again
		angle += 10.0f; // Increment by 10 degrees
		if (angle >= 360.0f) {
			angle -= 360.0f;
		}
	}

	return initial_angle; // No valid angle found
}

/*!
 * @brief render all the hud config gauges
 *
 * param[in] API_Access		whether or not this method has been called from the lua api
 */
void hud_config_render_gauges(bool API_Access)
{
	// Check if this ship has its own HUD gauges.
	SCP_string hud_name;
	if (SCP_vector_inbounds(HC_available_huds, HC_chosen_hud)) {
		ship_info* sip = &Ship_info[HC_available_huds[HC_chosen_hud].first];
		hud_name = HC_available_huds[HC_chosen_hud].second;

		for (const std::unique_ptr<HudGauge>& gauge : sip->hud_gauges) {
			GR_DEBUG_SCOPE("Render HUD gauge");
			if (gauge->getVisibleInConfig()) {
				gauge->setFont();
				gauge->render(0, true);
			}
			if (HC_gauge_list_clear) {
				HC_gauge_map.emplace_back(std::make_pair(gauge->getConfigId(), gauge.get()));
			}
		}
	} else {
		hud_name = XSTR("Default HUD", 1876);

		for (const std::unique_ptr<HudGauge>& gauge : default_hud_gauges) {
			GR_DEBUG_SCOPE("Render HUD gauge");
			if (gauge->getVisibleInConfig()) {
				gauge->setFont();
				gauge->render(0, true);
			}
			if (HC_gauge_list_clear) {
				HC_gauge_map.emplace_back(std::make_pair(gauge->getConfigId(), gauge.get()));
			}
		}
	}

	// DEBUGGING ONLY
	/*gr_set_color_fast(&Color_normal);
	for (auto& box : HC_gauge_mouse_coords) {
		hud_config_draw_box(box.second.x1, box.second.x2, box.second.y1, box.second.y2);
	}*/

	HC_gauge_list_clear = false;

	// Render the name of the HUD
	if (!API_Access) {
		gr_set_color_fast(&Color_normal);
		int w;
		gr_get_string_size(&w, nullptr, hud_name.c_str());
		int x = HC_gauge_coordinates[0] + ((HC_gauge_coordinates[4] / 2) - (w / 2));
		gr_string(x, HC_gauge_coordinates[3] + 10, hud_name.c_str(), GR_RESIZE_MENU);
	}

	hud_name.clear();
}

void hud_config_init(bool API_Access, int x, int y, int w, int h)
{
	hud_config_init_ui(API_Access, x, y, w, h);
	hud_config_backup(); // save the HUD configuration in case the player decides to cancel changes
	HUD_config_inited = 1;
}

bool hud_config_check_mouse_in_hud_area(int mx, int my)
{
	if (mx < HC_gauge_config_coords[gr_screen.res][0]) {
		return false;
	}
	if (mx > HC_gauge_config_coords[gr_screen.res][1]) {
		return false;
	}
	if (my < HC_gauge_config_coords[gr_screen.res][2]) {
		return false;
	}
	if (my > HC_gauge_config_coords[gr_screen.res][3]) {
		return false;
	}

	return true;
}

/*!
 * @brief check mouse position against all hud gauge preview display using mouse coordinates instead of a mask
 *
 */
void hud_config_check_regions_by_mouse(int mx, int my)
{
	for (const auto& coords : HC_gauge_mouse_coords) {
		if (coords.second.x1 < 0)
			continue;

		if (mx < coords.second.x1 || mx > coords.second.x2 || my < coords.second.y1 || my > coords.second.y2) {
			continue;
		}

		// If we've got here, it's a hit
		HC_gauge_hot = coords.first;
		return; // Stop checking once we find the first match
	}
}

/*!
 * @brief check mouse position against all ui buttons using the ui mask
 *
 */
void hud_config_check_regions(int mx, int my)
{
	// If we click on one of the new arrows then try to select a new HUD
	if (HC_arrow_hot >= 0 && mouse_down(MOUSE_LEFT_BUTTON)) {
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		if (HC_arrow_hot == 0) {
			hud_config_select_hud(false);
		} else {
			hud_config_select_hud(true);
		}
		mouse_flush();
		return;
	}

	// If we're not in the HUD area then nothing else applies here
	if (!hud_config_check_mouse_in_hud_area(mx, my)) {
		return;
	}

	// Sets HC_gauge_hot to a gauge name if we're hovering over one of them
	hud_config_check_regions_by_mouse(mx, my);

	// If we click inside the HUD area then reset the UI
	if (mouse_down(MOUSE_LEFT_BUTTON)) {
		HC_gauge_selected.clear();
		HC_color_sliders[HCS_RED].hide();
		HC_color_sliders[HCS_GREEN].hide();
		HC_color_sliders[HCS_BLUE].hide();
		HC_color_sliders[HCS_ALPHA].hide();

		HC_color_sliders[HCS_RED].disable();
		HC_color_sliders[HCS_GREEN].disable();
		HC_color_sliders[HCS_BLUE].disable();
		HC_color_sliders[HCS_ALPHA].disable();

		// Turn off select all
		hud_config_select_all_toggle(false);

		// See if we have a gauge that we were hovering over when we clicked
		const auto gauge = hud_config_get_gauge_pointer(HC_gauge_hot);

		// If we clicked on a gauge then set the UI up for managing that gauge
		if (gauge) {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_gauge_selected = HC_gauge_hot;

			// Setup rgb sliders if the gauge doesn't use IFF colors
			if (!gauge->getConfigUseIffColor()) {
				HC_color_sliders[HCS_RED].enable();
				HC_color_sliders[HCS_GREEN].enable();
				HC_color_sliders[HCS_BLUE].enable();
				HC_color_sliders[HCS_ALPHA].enable();

				HC_color_sliders[HCS_RED].unhide();
				HC_color_sliders[HCS_GREEN].unhide();
				HC_color_sliders[HCS_BLUE].unhide();
				HC_color_sliders[HCS_ALPHA].unhide();

				color clr = HUD_config.get_gauge_color(HC_gauge_selected);

				HC_color_sliders[HCS_RED].force_currentItem(HCS_CONV(clr.red));
				HC_color_sliders[HCS_GREEN].force_currentItem(HCS_CONV(clr.green));
				HC_color_sliders[HCS_BLUE].force_currentItem(HCS_CONV(clr.blue));
				HC_color_sliders[HCS_ALPHA].force_currentItem(HCS_CONV(clr.alpha));
			}

			// recalc alpha slider
			hud_config_recalc_alpha_slider();
		}
	}

	mouse_flush();
}

// set the display flags for a HUD gauge
void hud_config_set_gauge_flags(const SCP_string& gauge, bool on_flag, bool popup_flag)
{
	HUD_config.set_gauge_visibility(gauge, on_flag);
	HUD_config.set_gauge_popup(gauge, popup_flag);
}

void hud_config_record_color(int in_color)
{
	HUD_config.main_color = in_color;
	HUD_color_red = HC_colors[in_color].r;
	HUD_color_green = HC_colors[in_color].g;
	HUD_color_blue = HC_colors[in_color].b;
}

// Set the HUD color to one of the color presets
void hud_config_set_color(int in_color)
{
	hud_config_record_color(in_color);

	HUD_init_hud_color_array();

	color clr;
	gr_init_alphacolor(&clr, HC_colors[in_color].r, HC_colors[in_color].g, HC_colors[in_color].b, (HUD_color_alpha+1)*16);

	// apply the color to all built-in gauges gauges
	for(const auto& gauge_pair : HC_gauge_map){
        const SCP_string& gauge_id = gauge_pair.first;
        if (!gauge_id.empty()) {
             HUD_config.set_gauge_color(gauge_id, clr);
        }
    }
}

// Set the HUD color when ALL GAUGES is selected in the HUD Config UI
void hud_config_stuff_colors(int r, int g, int b)
{
	color clr;
	gr_init_alphacolor(&clr, r, g, b, 255);

	// apply the color to all gauges
	for (const auto& gauge_pair : HC_gauge_map) {
		const SCP_string& gauge_id = gauge_pair.first;
		if (!gauge_id.empty()) {
			HUD_config.set_gauge_color(gauge_id, clr);
		}
	}
}

void hud_config_cancel(bool change_state)
{
	hud_config_restore();

	// adds scripting hook for 'On HUD Config Menu Closed' --wookieejedi
	if (scripting::hooks::OnHUDConfigMenuClosed->isActive()) {
		scripting::hooks::OnHUDConfigMenuClosed->run(scripting::hook_param_list(scripting::hook_param("OptionsAccepted", 'b', false)));
	}

	if (change_state) {
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}
}

void hud_config_commit()
{
	// adds scripting hook for 'On HUD Config Menu Closed' --wookieejedi
	if (scripting::hooks::OnHUDConfigMenuClosed->isActive()) {
		scripting::hooks::OnHUDConfigMenuClosed->run(scripting::hook_param_list(scripting::hook_param("OptionsAccepted", 'b', true)));
	}

	gamesnd_play_iface(InterfaceSounds::COMMIT_PRESSED);
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

// move gauge state from on->off->popup
void hud_cycle_gauge_status()
{
	if ( !HC_gauge_selected.empty() ) {
		return;
	}

	// gauge is off, move to popup
	if ( !(HUD_config.is_gauge_visible(HC_gauge_selected)) ) {
		const auto gauge = hud_config_get_gauge_pointer(HC_gauge_selected);
		if (gauge != nullptr && gauge->getConfigCanPopup()) {
			hud_config_set_gauge_flags(HC_gauge_selected, true, true);	
		} else {
			hud_config_set_gauge_flags(HC_gauge_selected, true, false);	
		}
		return;
	}

	// if gauge is popup, move to on
	if ( HUD_config.is_gauge_popup(HC_gauge_selected) ) {
		hud_config_set_gauge_flags(HC_gauge_selected, true, false);
		return;
	}
	
	// gauge must be on, move to off
	hud_config_set_gauge_flags(HC_gauge_selected, false, false);
}

// handle keyboard input while in hud config
void hud_config_handle_keypresses(int k)
{
	switch(k) {
	case KEY_ESC:
		if (escape_key_behavior_in_options == EscapeKeyBehaviorInOptions::SAVE) {
			hud_config_commit();
		} else {
			hud_config_cancel();
		}
		break;
	case KEY_CTRLED+KEY_ENTER:
		hud_config_commit();
		break;
	case KEY_TAB:
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		hud_cycle_gauge_status();
		break;
	case KEY_RIGHT:
		hud_config_select_hud(true);
		break;
	case KEY_LEFT:
		hud_config_select_hud(false);
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
		hud_config_set_color(HUD_COLOR_PRESET_3);
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		break;
	case HCB_BLUE:
		hud_config_set_color(HUD_COLOR_PRESET_2);
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		break;
	case HCB_GREEN:
		hud_config_set_color(HUD_COLOR_PRESET_1);
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		break;
	case HCB_ON:
		if (HC_gauge_selected.empty()) {
			break;
		}
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		hud_config_set_gauge_flags(HC_gauge_selected,true,false);
		break;
	case HCB_OFF:
		if (HC_gauge_selected.empty()) {
			break;
		}
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		hud_config_set_gauge_flags(HC_gauge_selected,false,false);
		break;
	case HCB_POPUP:
		if (HC_gauge_selected.empty()) {
			break;
		}
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		hud_config_set_gauge_flags(HC_gauge_selected,true,true);
		break;
	case HCB_RESET:
		gamesnd_play_iface(InterfaceSounds::RESET_PRESSED);
		hud_config_select_all_toggle(0);
		hud_set_default_hud_config(Player, HC_default_preset_file);
		hud_config_synch_ui(false);
		break;
	case HCB_ACCEPT:
		hud_config_commit();
		break;

	// new stuff
	case HCB_RED_UP:
		if( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) >= 255){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) + 1)  );
			hud_config_red_slider();			
		}		
		break;

	case HCB_GREEN_UP:
		if( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) >= 255){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) + 1) );
			hud_config_green_slider();
		}		
		break;

	case HCB_BLUE_UP:
		if( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) >= 255){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) + 1) );
			hud_config_blue_slider();
		}		
		break;

	case HCB_I_UP:
		if( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) >= 255){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) + 1) );
			hud_config_alpha_slider_up();
		}		
		break;

	case HCB_RED_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) <= 0){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) - 1) );
			hud_config_red_slider();
		}		
		break;

	case HCB_GREEN_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) <= 0){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) - 1) );
			hud_config_green_slider();
		}		
		break;

	case HCB_BLUE_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) <= 0){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) - 1) );
			hud_config_blue_slider();
		}		
		break;

	case HCB_I_DOWN:
		if( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) <= 0){
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		} else {
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV( HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem()) - 1) );
			hud_config_alpha_slider_down();
		}		
		break;

	case HCB_SAVE_HCF:		
		int exists;
		const char *out;

		// get the text in the input control
		exists = 0;
		HC_fname_input.get_text(name);
		if(name[0] != '\0'){
			// if the filename in there already exists
			for (idx = 0; idx < (int)HC_preset_filenames.size(); idx++) {
				if (!stricmp(HC_preset_filenames[idx].c_str(), name)) {
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

		// save the file, maybe generating a new filename
		if(strlen(name) <= 0){
			sprintf(name, "hud_%d.hcf", (int)HC_preset_filenames.size());
			out = name;
		} else {
			out = cf_add_ext(name, ".hcf");
		}
		HC_preset_filenames.push_back(out);
		hud_config_color_save(out);		

		HC_fname_input.set_text(out);
		break;

	case HCB_PREV_HCF:
		if (HC_preset_filenames.size() <= 0) {
			break;
		}

		if(HC_current_file <= 0){
			HC_current_file = (int)(HC_preset_filenames.size() - 1);
		} else {
			HC_current_file--;
		}
		// load em up
		hud_config_color_load(HC_preset_filenames[HC_current_file].c_str());
		hud_config_synch_ui(false);

		HC_fname_input.set_text(HC_preset_filenames[HC_current_file].c_str());
		break;

	case HCB_NEXT_HCF:
		if (HC_preset_filenames.size() <= 0) {
			break;
		}

		if ((++HC_current_file) >= static_cast<int> (HC_preset_filenames.size())) {
			HC_current_file = 0;
		}

		// load em up		
		hud_config_color_load(HC_preset_filenames[HC_current_file].c_str());
		hud_config_synch_ui(false);

		HC_fname_input.set_text(HC_preset_filenames[HC_current_file].c_str());
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
	UI_BUTTON	*b;

	for (int i=0; i<NUM_HUD_BUTTONS; i++ ) {
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
	case HUD_COLOR_PRESET_3:
		HC_buttons[gr_screen.res][HCB_AMBER].button.draw_forced(2);
		break;
	case HUD_COLOR_PRESET_1:
		HC_buttons[gr_screen.res][HCB_GREEN].button.draw_forced(2);
		break;
	case HUD_COLOR_PRESET_2:
		HC_buttons[gr_screen.res][HCB_BLUE].button.draw_forced(2);
		break;
	}
}

// set the status (on/off/popup) for the selected gauge
void hud_config_draw_gauge_status()
{
	if (HC_gauge_selected.empty()) {
		return;
	}

	if ( HC_buttons[gr_screen.res][HCB_OFF].button.button_down() || HC_buttons[gr_screen.res][HCB_POPUP].button.button_down() || HC_buttons[gr_screen.res][HCB_ON].button.button_down() ) {
		return;
	}

	// check if off
	if (!(HUD_config.is_gauge_visible(HC_gauge_selected))) {
		HC_buttons[gr_screen.res][HCB_OFF].button.draw_forced(2);
		return;
	}

	// check if popup
	if (HUD_config.is_gauge_popup(HC_gauge_selected)) {
		HC_buttons[gr_screen.res][HCB_POPUP].button.draw_forced(2);
		return;
	}

	// check if on
	if (HUD_config.is_gauge_visible(HC_gauge_selected)) {
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
	const auto gauge = hud_config_get_gauge_pointer(HC_gauge_selected);

	// Custom gauges cannot currently be set to popup or toggle visibility
	if (HC_gauge_selected.empty() || gauge->isCustom()) {
		hud_config_button_disable(HCB_ON);
		hud_config_button_disable(HCB_OFF);
		hud_config_button_disable(HCB_POPUP);
		return;
	}

	// on/off are always on
	hud_config_button_enable(HCB_ON);
	hud_config_button_enable(HCB_OFF);

	// popup is maybe available
	if (gauge != nullptr && gauge->getConfigCanPopup()) {
		hud_config_button_enable(HCB_POPUP);
	} else {
		hud_config_button_disable(HCB_POPUP);
	}
}

void hud_config_render_description()
{
	int w,h,sx,sy;

	if (!HC_gauge_selected.empty()) {
		const auto gauge = hud_config_get_gauge_pointer(HC_gauge_selected);

		if (gauge != nullptr) {
			gr_set_color_fast(&Color_normal);

			gr_get_string_size(&w, &h, gauge->getConfigName().c_str());
			sx = fl2i(HC_gauge_description_coords[gr_screen.res][0] + (HC_gauge_description_coords[gr_screen.res][2] - w) / 2.0f);
			sy = HC_gauge_description_coords[gr_screen.res][1];
			gr_string(sx, sy, gauge->getConfigName().c_str(), GR_RESIZE_MENU);
		}
	}
}

void hud_config_render_special_bitmaps()
{
	/*
	int i;
	for (i=1; i<NUM_HC_SPECIAL_BITMAPS; i++) {
		if (HC_special_bitmaps[i].bitmap >= 0) {
			gr_set_bitmap(HC_special_bitmaps[i].bitmap);
			gr_bitmap(HC_special_bitmaps[i].x, HC_special_bitmaps[i].y, GR_RESIZE_MENU);
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

void hud_config_draw_hud_select_arrow_buttons(int mx, int my)
{
	int w1, h1;
	bm_get_info(HC_arrow_bm_handles[gr_screen.res][0], &w1, &h1);
	int w2, h2;
	bm_get_info(HC_arrow_bm_handles[gr_screen.res][0], &w2, &h2);


	BoundingBox l_box(HC_arrow_coords[gr_screen.res][0].first,
		HC_arrow_coords[gr_screen.res][0].first + w1,
		HC_arrow_coords[gr_screen.res][0].second,
		HC_arrow_coords[gr_screen.res][0].second + h1);
	BoundingBox r_box(HC_arrow_coords[gr_screen.res][1].first,
		HC_arrow_coords[gr_screen.res][1].first + w1,
		HC_arrow_coords[gr_screen.res][1].second,
		HC_arrow_coords[gr_screen.res][1].second + h1);
	
	int hot = -1;
	if (mx >= l_box.x1 && mx <= l_box.x2 && my >= l_box.y1 && my <= l_box.y2) {
		hot = 0;
	}

	if (mx >= r_box.x1 && mx <= r_box.x2 && my >= r_box.y1 && my <= r_box.y2) {
		hot = 1;

	}

	if (hot != HC_arrow_hot) {
		HC_arrow_hot = hot;
		if (HC_arrow_hot >= 0) {
			gamesnd_play_iface(InterfaceSounds::USER_OVER);
		}
	}

	for (int i = 0; i < 2; ++i) {
		int on = (i == HC_arrow_hot) ? 1 : 0;
		gr_set_bitmap(HC_arrow_bm_handles[i][on]);
		gr_bitmap(HC_arrow_coords[gr_screen.res][i].first, HC_arrow_coords[gr_screen.res][i].second, GR_RESIZE_MENU);
	}
}

void hud_config_do_frame(float /*frametime*/, bool API_Access, int mx, int my)
{
	int k;

	if (!HUD_config_inited) {
		hud_config_init();
	}

	// If we don't have a mask, we don't have enough data to do anything with this screen.
	if (!API_Access && (HC_background_bitmap_mask == -1)) {
		popup_game_feature_not_in_demo();
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
		return;
	}

	HC_gauge_hot.clear();

	if (!API_Access) {
		hud_config_set_button_state();

		k = HC_ui_window.process();

		mouse_get_pos_unscaled(&mx, &my);

		hud_config_handle_keypresses(k);
		hud_config_check_regions(mx, my);
		hud_config_check_buttons();
		hud_config_update_brightness();

		// set the background
		GR_MAYBE_CLEAR_RES(HC_background_bitmap);
		if (HC_background_bitmap > 0) {
			gr_set_bitmap(HC_background_bitmap);
			gr_bitmap(0, 0, GR_RESIZE_MENU);
		}

		// rgb slider/button stuff
		hud_config_process_colors();

		HC_ui_window.draw();
		hud_config_redraw_pressed_buttons();

		hud_config_draw_gauge_status();
		hud_config_draw_color_status();

		// maybe force draw the select all button
		if (HC_select_all) {
			HC_buttons[gr_screen.res][HCB_SELECT_ALL].button.draw_forced(2);
		}
	} else {
		hud_config_check_regions_by_mouse(mx, my);
	}

	hud_config_render_gauges(API_Access);

	if (!API_Access) {
		hud_config_render_special_bitmaps();
		hud_config_render_description();

		if (HC_available_huds.size() + (HC_show_default_hud ? 1 : 0) > 1) {
			hud_config_draw_hud_select_arrow_buttons(mx, my);
		}

		gr_flip();
	}
}

// hud_config_close() is called when the player leaves the hud configuration screen
//
void hud_config_close(bool API_Access)
{
	HC_preset_filenames.clear();

	if (!API_Access) {
		if (HC_background_bitmap != -1) {
			bm_release(HC_background_bitmap);
		}

		HC_ui_window.destroy();

		if (HC_background_bitmap_mask != -1) {
			bm_release(HC_background_bitmap_mask);
		}
	}

	for (const auto& handle : HC_arrow_bm_handles) {
		bm_unload(handle[0]);
		bm_unload(handle[1]);
	}

	bm_unload(HC_talking_head_frame);
	HC_talking_head_frame = -1;

	HUD_config_inited = 0;
}

// hud_set_default_hud_config() will set the hud configuration to default values
void hud_set_default_hud_config(player * /*p*/, const SCP_string& filename)
{
	HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;
	HUD_config.main_color = HC_default_color;
	HUD_color_red = HC_colors[HUD_config.main_color].r;
	HUD_color_green = HC_colors[HUD_config.main_color].g;
	HUD_color_blue = HC_colors[HUD_config.main_color].b;

	color clr;
	gr_init_alphacolor(&clr, HUD_color_red, HUD_color_green, HUD_color_blue, (HUD_color_alpha + 1) * 16);

	for (const auto& gauge_pair : HC_gauge_map) {
		const SCP_string& gauge_id = gauge_pair.first;
		if (!gauge_id.empty()) {
			HUD_config.set_gauge_color(gauge_id, clr);
		}
	}

	HUD_config.show_flags_map.clear();
	HUD_config.popup_flags_map.clear();

	// Built-in have specific settings
	for (const auto& gauge_id : default_visible_gauges) {
		HUD_config.show_flags_map[gauge_id] = true;
	}

	// Custom gauges are always visible by default
	for (const auto& gauge_pair : HC_gauge_map) {
		if (gauge_pair.second->isCustom()) {
			const SCP_string& gauge_id = gauge_pair.first;
			HUD_config.show_flags_map[gauge_id] = true;
		}
	}

	HUD_config.rp_dist = RR_INFINITY;
	HUD_config.is_observer = false;

	// load up the default colors
	hud_config_color_load(filename.c_str());
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
	// store the current hud
	hud_config_backup();

	// initialize the observer HUD
	hud_observer_init(shipp,aif);	
}

void hud_config_as_player()
{
	if (HUD_config.is_observer) {		// If he was observer before
		hud_config_restore();
	}
} 

// ---------------------------------------------------------------------------------------------------------------
// RGB color stuff
//

void hud_config_color_save(const char *name, int version)
{
	CFILE* out     = cfopen(name, "wt", CF_TYPE_PLAYERS, false,
                        CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	try {

		if (out == nullptr) {
			char message[256];
			sprintf(message, "Unable to open file: %s", name);
			throw parse::ParseException(message);
		}

		switch (version) {
			case 1: {
				for (int idx = 0; idx < NUM_HUD_GAUGES; idx++) {
					// Get the gauge string ID from numeric ID
					SCP_string gauge_id = gauge_map.get_string_id_from_numeric_id(idx);
					// If the gauge isn't found, default to white
					color clr = HUD_config.get_gauge_color(gauge_id);

					cfputs("+Gauge: ", out);
					cfputs(gauge_map.get_hcf_name_from_string_id(gauge_id).c_str(), out);
					cfputs("\n", out);
					cfputs("+RGBA: ", out);
					char vals[255] = "";
					sprintf(vals, "%d %d %d %d\n\n", clr.red, clr.green, clr.blue, clr.alpha);
					cfputs(vals, out);
				}
				break;
			}
			case 2: {
				cfputs("+VERSION 2\n\n", out);
				for (const auto& gauge : HUD_config.gauge_colors) {
					if (gauge.first.empty()) {
						continue;
					}
					cfputs("+Gauge: ", out);
					cfputs(gauge.first.c_str(), out);
					cfputs("\n", out);
					cfputs("+RGBA: ", out);
					char vals[255] = "";
					sprintf(vals, "%d %d %d %d\n\n", gauge.second.red, gauge.second.green, gauge.second.blue, gauge.second.alpha);
					cfputs(vals, out);
				}
				break;
			}
			default: {
				throw parse::ParseException("Unknown HUD config version: " + std::to_string(version));
			}
		}
	
	} catch (const parse::ParseException& e) {
		mprintf(("HUDCONFIG: Unable to save '%s'!  Error message = %s.\n", name, e.what()));
		return;
	}
	
	// close the file
	cfclose(out);	
}

void hud_config_color_load(const char *name)
{
	const char *fname = cf_add_ext(name, ".hcf");

	try
	{
		read_file_text(fname);
		reset_parse();

		HUD_config.gauge_colors.clear();
		for (const auto& gauge : HC_gauge_map) {
			color clr;
			gr_init_alphacolor(&clr, HUD_color_red, HUD_color_green, HUD_color_blue, (HUD_color_alpha + 1) * 16);
			HUD_config.set_gauge_color(gauge.first, clr);
		}

		// Now read in the color values for the gauges
		int version = 1;
		if (optional_string("+VERSION 2")) {
			version = 2;
		}
		while (optional_string("+Gauge:")) {
			SCP_string str;
			stuff_string(str, F_NAME);

			required_string("+RGBA:");
			ubyte r, g, b, a;
			stuff_ubyte(&r);
			stuff_ubyte(&g);
			stuff_ubyte(&b);
			stuff_ubyte(&a);

			color clr;
			gr_init_alphacolor(&clr, r, g, b, a);

			switch (version) {
				case 1: {
					SCP_string gauge = gauge_map.get_string_id_from_hcf_id(str);
					if (!gauge.empty()) {
						HUD_config.set_gauge_color(gauge, clr);
					}
					break;
				}
				case 2: {
					HUD_config.set_gauge_color(str, clr);
					break;
				}
				default: {
					throw parse::ParseException("Unknown HUD config version: " + std::to_string(version));
				}
			}
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("HUDCONFIG: Unable to parse '%s'!  Error message = %s.\n", fname, e.what()));
		return;
	}
}

void hud_config_alpha_slider_up()
{	
	int pos = HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem());
	int max = std::max({ HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) });

	// if this would put the brightest element past its limit, skip
	if(max >= 255){
		HC_color_sliders[HCS_ALPHA].force_currentItem( HCS_CONV(pos - 1) );
		return;
	}
	
	// otherwise bump everybody up by one
	HC_color_sliders[HCS_RED].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) + 1) );	
	HC_color_sliders[HCS_GREEN].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) + 1) );
	HC_color_sliders[HCS_BLUE].force_currentItem( HCS_CONV(HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) + 1) );
	
	// apply -- Cyborg17 -- Unless you have nothing to apply it to!
	if(HC_select_all){
		hud_config_stuff_colors( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) );
	} else if (!HC_gauge_selected.empty()) {
		color clr;
		gr_init_alphacolor(&clr, HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()), 255);
		HUD_config.set_gauge_color(HC_gauge_selected, clr);
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

	// apply -- Cyborg17 -- Unless you have nothing to apply it to!
	if(HC_select_all){
		hud_config_stuff_colors( HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()) );
	} else if (!HC_gauge_selected.empty()) {
		color clr;
		gr_init_alphacolor(&clr, HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()), HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem()), 255);
		HUD_config.set_gauge_color(HC_gauge_selected, clr);
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
	int pos = HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem()) ;

	if(HC_select_all){
		for(const auto& gauge_pair : HC_gauge_map){
            const SCP_string& gauge_id = gauge_pair.first;
            if (!gauge_id.empty()) {                
                gr_init_alphacolor(&HUD_config.gauge_colors[gauge_id], pos, HUD_config.gauge_colors[gauge_id].green, HUD_config.gauge_colors[gauge_id].blue, HUD_config.gauge_colors[gauge_id].alpha);
            }
        }
	}
	// individual gauge
	else {
		if(HC_gauge_selected.empty()){
			return;
		}

		gr_init_alphacolor(&HUD_config.gauge_colors[HC_gauge_selected], pos, HUD_config.gauge_colors[HC_gauge_selected].green, HUD_config.gauge_colors[HC_gauge_selected].blue, HUD_config.gauge_colors[HC_gauge_selected].alpha);
	}	

	hud_config_recalc_alpha_slider();
}

void hud_config_green_slider()
{
	int pos = HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem()) ;

	if(HC_select_all){
		for(const auto& gauge_pair : HC_gauge_map){
            const SCP_string& gauge_id = gauge_pair.first;
            if (!gauge_id.empty()) {                
                gr_init_alphacolor(&HUD_config.gauge_colors[gauge_id], pos, HUD_config.gauge_colors[gauge_id].green, HUD_config.gauge_colors[gauge_id].blue, HUD_config.gauge_colors[gauge_id].alpha);
            }
        }
	}
	// individual gauge
	else {
		if(HC_gauge_selected.empty()){
			return;
		}

		gr_init_alphacolor(&HUD_config.gauge_colors[HC_gauge_selected], HUD_config.gauge_colors[HC_gauge_selected].red, pos, HUD_config.gauge_colors[HC_gauge_selected].blue, HUD_config.gauge_colors[HC_gauge_selected].alpha);
	}	

	hud_config_recalc_alpha_slider();
}

void hud_config_blue_slider()
{
	int pos = HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem());

	if(HC_select_all){
		for(const auto& gauge_pair : HC_gauge_map){
            const SCP_string& gauge_id = gauge_pair.first;
            if (!gauge_id.empty()) {                
                gr_init_alphacolor(&HUD_config.gauge_colors[gauge_id], pos, HUD_config.gauge_colors[gauge_id].green, HUD_config.gauge_colors[gauge_id].blue, HUD_config.gauge_colors[gauge_id].alpha);
            }
        }
	}
	// individual gauge
	else {
		if(HC_gauge_selected.empty()){
			return;
		}

		gr_init_alphacolor(&HUD_config.gauge_colors[HC_gauge_selected], HUD_config.gauge_colors[HC_gauge_selected].red, HUD_config.gauge_colors[HC_gauge_selected].green, pos, HUD_config.gauge_colors[HC_gauge_selected].alpha);
	}	

	hud_config_recalc_alpha_slider();
}

void hud_config_get_sliders_color(color & clr)
{
	int r = HCS_CONV(HC_color_sliders[HCS_RED].get_currentItem());
	int g = HCS_CONV(HC_color_sliders[HCS_GREEN].get_currentItem());
	int b = HCS_CONV(HC_color_sliders[HCS_BLUE].get_currentItem());
	int a = HCS_CONV(HC_color_sliders[HCS_ALPHA].get_currentItem());

	gr_init_alphacolor(&clr, r, g, b, a);
}

void hud_config_process_colors()
{	
}

void hud_config_preset_init()
{
	HC_current_file = -1;
	HC_preset_filenames.clear();
	cf_get_file_list(HC_preset_filenames, CF_TYPE_PLAYERS, "*.hcf", CF_SORT_NAME, nullptr,
						CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
}

void hud_config_delete_preset(SCP_string filename)
{
	filename += ".hcf"; //cfile strips the extension when adding to the list, so add it back here
	cf_delete(filename.c_str(), CF_TYPE_PLAYERS,
		CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	// Reload the presets from file.
	hud_config_preset_init();
}

void hud_config_select_none()
{
	HC_select_all = false;
	HC_gauge_selected.clear();
}

void hud_config_select_hud(bool next)
{
	if (next) {
		HC_chosen_hud++;
		if (HC_chosen_hud >= static_cast<int>(HC_available_huds.size())) {
			HC_chosen_hud = HC_show_default_hud ? -1 : 0;
		}
	} else {
		HC_chosen_hud--;
		if (HC_chosen_hud < (HC_show_default_hud ? -1 : 0)) {
			HC_chosen_hud = static_cast<int>(HC_available_huds.size()) - 1;
		}
	}
	HC_gauge_map.clear();
	HC_gauge_mouse_coords.clear();
	HC_gauge_list_clear = true;
}

void hud_config_select_all_toggle(bool toggle, bool API_Access)
{	
	int r, g, b, a;

	// if we're turning off
	if(!toggle){				
		// determine if on/off/popup buttons should be shown
		if (!API_Access) {
			hud_config_set_button_state();
		}

		HC_select_all = false;
	} else {
		// synch stuff up
		hud_config_synch_ui(API_Access);
		
		// if we had a gauge previously selected, use its color everywhere
		if(HC_gauge_selected.empty()){
			SCP_string gauge = gauge_map.get_string_id_from_numeric_id(HUD_RADAR);
			r = HUD_config.gauge_colors[gauge].red;
			g = HUD_config.gauge_colors[gauge].green;
			b = HUD_config.gauge_colors[gauge].blue;			
			a = HUD_config.gauge_colors[gauge].alpha;
		} else {
			r = HUD_config.gauge_colors[HC_gauge_selected].red;
			g = HUD_config.gauge_colors[HC_gauge_selected].green;
			b = HUD_config.gauge_colors[HC_gauge_selected].blue;			
			a = HUD_config.gauge_colors[HC_gauge_selected].alpha;
		}
		hud_config_stuff_colors(r, g, b);

		// no gauge selected
		HC_gauge_selected.clear();		

		// enable all sliders
		if (!API_Access) {
			HC_color_sliders[HCS_RED].enable();
			HC_color_sliders[HCS_GREEN].enable();
			HC_color_sliders[HCS_BLUE].enable();
			HC_color_sliders[HCS_ALPHA].enable();
			HC_color_sliders[HCS_RED].unhide();
			HC_color_sliders[HCS_GREEN].unhide();
			HC_color_sliders[HCS_BLUE].unhide();
			HC_color_sliders[HCS_ALPHA].unhide();
			HC_color_sliders[HCS_RED].force_currentItem(HCS_CONV(r));
			HC_color_sliders[HCS_GREEN].force_currentItem(HCS_CONV(g));
			HC_color_sliders[HCS_BLUE].force_currentItem(HCS_CONV(b));
			HC_color_sliders[HCS_ALPHA].force_currentItem(HCS_CONV(a));

			// recalc alpha
			hud_config_recalc_alpha_slider();

			// disable all three buttons
			hud_config_button_disable(HCB_ON);
			hud_config_button_disable(HCB_OFF);
			hud_config_button_disable(HCB_POPUP);
		}

		HC_select_all = true;
	}
}

SCP_string create_custom_gauge_id(const SCP_string& gauge_name) {
	Assertion(!gauge_name.empty(), "Custom gauge has no name!");

	SCP_string id;
	if (Mod_title.empty()) {
		id = Cmdline_mod;

		// Basic cleanup attempt
		id = id.substr(0, id.find_first_of(DIR_SEPARATOR_CHAR));
	} else {
		id = Mod_title;
	}

	if (!id.empty()) {
		std::replace(id.begin(), id.end(), ' ', '_');
	} else {
		id = "Custom";
	}

	id += "::" + gauge_name;

	return id;
}

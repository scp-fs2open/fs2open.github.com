/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "controlconfig/controlsconfig.h"
#include "debugconsole/console.h"
#include "freespace.h"
#include "gamehelp/contexthelp.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/undosys.h"
#include "graphics/font.h"
#include "hud/hudsquadmsg.h"
#include "io/joy.h"
#include "io/key.h"
#include "io/timer.h"
#include "missionui/missionscreencommon.h"
#include "network/multi_pmsg.h"
#include "network/multiutil.h"
#include "scripting/scripting.h"
#include "pilotfile/pilotfile.h"
#include "popup/popup.h"
#include "ui/ui.h"
#include "ui/uidefs.h"


#ifndef NDEBUG
#include "hud/hud.h"
#endif



#define NUM_BUTTONS				19
#define NUM_TABS				4

// coordinate indicies
#define CONTROL_X_COORD 0
#define CONTROL_Y_COORD 1
#define CONTROL_W_COORD 2
#define CONTROL_H_COORD 3

const char* Conflict_background_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"ControlConfig",		// GR_640
	"2_ControlConfig"		// GR_1024
};

const char* Conflict_background_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"ControlConfig-m",		// GR_640
	"2_ControlConfig-m"		// GR_1024
};

// control list area
int Control_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		20, 58, 596, 259	// GR_640
	},
	{
		32, 94, 956, 424	// GR_1024
	}
};

// width of the control name section of the list
int Control_list_ctrl_w[GR_NUM_RESOLUTIONS] = {
	350,	// GR_640
	600		// GR_1024
};

// x start position of the primary binding area column
int Control_list_first_x[GR_NUM_RESOLUTIONS] = {
	350,	// GR_640
	640		// GR_1024 (712)
};

// width of the both binding area columns
int Control_list_first_w[GR_NUM_RESOLUTIONS] = {
	116,	// GR_640 (198)
	150		// GR_1024 (230)
};

// x start position of the secondary binding area column
int Control_list_second_x[GR_NUM_RESOLUTIONS] = {
	480,	// GR_640
	806,	// GR_1024
};

// display the "more..." text under the control list
int Control_more_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		320, 326			// GR_640
	},
	{
		500, 542			// GR_1024
	}
};

// area to display "conflicts with..." text
int Conflict_wnd_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		32, 313, 250, 32	// GR_640
	},
	{
		48, 508, 354, 46	// GR_1024
	}
};

// conflict warning anim coords
int Conflict_warning_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		320, 420			// GR_640
	},
	{
		512, 669			// GR_1024
	}
};

// for flashing the conflict text
#define CONFLICT_FLASH_TIME	250
int Conflict_stamp = -1;
int Conflict_bright = 0;

#define LIST_BUTTONS_MAX	42
#define JOY_AXIS			0x80000

static int Num_cc_lines;	// Number of Cc_lines to display on the current page. Is, at worse, CCFG_MAX + NUM_JOY_AXIS_ACTIONS

/**
 * @struct cc_line
 * @brief Defines an interactable line to display control names and their bindings
 */
struct cc_line {
	const char *label;
	int cc_index;  // index into Control_config of item
	int y;  // Y coordinate of line
	int kx, kw, jx, jw;  // x start and width of keyboard and joystick bound text
};

enum class selItem : int {
	selItem_REND,	// Must be first to allow cycling

	None,
	Primary,
	Secondary,

	selItem_END	// Must be last to allow cycling
};

SCP_vector<cc_line> Cc_lines;

// Backups for use when user closes the config menu without saving
SCP_vector<CCI> Control_config_backup;
int Axis_map_to_backup[NUM_JOY_AXIS_ACTIONS];
int Invert_axis_backup[JOY_NUM_AXES];

// Undo system
Undo_system Undo_controls;

// all this stuff is localized/externalized
#define NUM_AXIS_TEXT			6
#define NUM_MOUSE_TEXT			5
#define NUM_MOUSE_AXIS_TEXT		2
#define NUM_INVERT_TEXT			2	
char *Joy_axis_action_text[NUM_JOY_AXIS_ACTIONS];
char *Joy_axis_text[NUM_AXIS_TEXT];
char *Mouse_button_text[NUM_MOUSE_TEXT];
char *Mouse_axis_text[NUM_MOUSE_AXIS_TEXT];
char *Invert_text[NUM_INVERT_TEXT];

int Control_check_count = 0;

static int Tab;  // which tab we are currently in
static int Binding_mode = 0;  // are we waiting for a key to bind it?
static int Bind_time = 0;
static int Search_mode = 0;  // are we waiting for a key to bind it?
static int Last_key = -1;
static int Selected_line = 0;  // line that is currently selected for binding
static selItem Selected_item = selItem::None;
static int Scroll_offset;
static int Axis_override = -1;
static int Background_bitmap;
static int Conflicts_tabs[NUM_TABS];
static UI_BUTTON List_buttons[LIST_BUTTONS_MAX];  // buttons for each line of text in list
static UI_WINDOW Ui_window;
static unsigned int Defaults_cycle_pos = 0; // the controls preset that was last selected

int Control_config_overlay_id;

struct conflict {
	int first = -1;     // index of other control in conflict with this one
	int second = -1;    // index of other control in conflict with this one
};

SCP_vector<conflict> Conflicts;

int Conflicts_axes[NUM_JOY_AXIS_ACTIONS];

#define TARGET_TAB				0
#define SHIP_TAB				1
#define WEAPON_TAB				2
#define COMPUTER_TAB			3
#define SCROLL_UP_BUTTON		4
#define SCROLL_DOWN_BUTTON		5
#define ALT_TOGGLE				6
#define SHIFT_TOGGLE			7
#define INVERT_AXIS				8
#define CANCEL_BUTTON			9
#define UNDO_BUTTON				10
#define RESET_BUTTON			11
#define SEARCH_MODE				12
#define BIND_BUTTON				13
#define HELP_BUTTON				14
#define ACCEPT_BUTTON			15
#define CLEAR_OTHER_BUTTON		16
#define CLEAR_ALL_BUTTON		17
#define CLEAR_BUTTON			18

ui_button_info CC_Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("CCB_00",	32,	348,	17,	384,	0),	// target tab
		ui_button_info("CCB_01",	101,	348,	103,	384,	1),	// ship tab
		ui_button_info("CCB_02",	173,	352,	154,	384,	2),	// weapon tab
		ui_button_info("CCB_03",	242,	347,	244,	384,	3),	// computer/misc tab
		ui_button_info("CCB_04",	614,	73,	-1,	-1,	4),	// scroll up
		ui_button_info("CCB_05",	614,	296,	-1,	-1,	5),	// scroll down
		ui_button_info("CCB_06",	17,	452,	12,	440,	6),	// alt toggle
		ui_button_info("CCB_07",	56,	452,	50,	440,	7),	// shift toggle
		ui_button_info("CCB_09",	162,	452,	155,	440,	9),	// invert
		ui_button_info("CCB_10",	404,	1,		397,	45,	10),	// cancel
		ui_button_info("CCB_11",	582,	347,	586,	386,	11),	// undo
		ui_button_info("CCB_12",	576,	1,		578,	45,	12),	// default
		ui_button_info("CCB_13",	457,	4,		453,	45,	13),	// search
		ui_button_info("CCB_14",	516,	4,		519,	45,	14),	// bind
		ui_button_info("CCB_15",	540,	428,	500,	440,	15),	// help
		ui_button_info("CCB_16",	574,	432,	571,	412,	16),	// accept
		ui_button_info("CCB_18",	420,	346,	417,	386,	18),	// clear other 
		ui_button_info("CCB_19",	476,	346,	474,	386,	19),	// clear all
		ui_button_info("CCB_20",	524,	346,	529,	386,	20),	// clear button
	},
	{ // GR_1024
		ui_button_info("2_CCB_00",	51,	557,	27,	615,	0),	// target tab
		ui_button_info("2_CCB_01",	162,	557,	166,	615,	1),	// ship tab
		ui_button_info("2_CCB_02",	277,	563,	246,	615,	2),	// weapon tab
		ui_button_info("2_CCB_03",	388,	555,	391,	615,	3),	// computer/misc tab
		ui_button_info("2_CCB_04",	982,	117,	-1,	-1,	4),	// scroll up
		ui_button_info("2_CCB_05",	982,	474,	-1,	-1,	5),	// scroll down
		ui_button_info("2_CCB_06",	28,	723,	24,	704,	6),	// alt toggle
		ui_button_info("2_CCB_07",	89,	723,	80,	704,	7),	// shift toggle
		ui_button_info("2_CCB_09",	260,	723,	249,	704,	9),	// invert
		ui_button_info("2_CCB_10",	646,	2,		635,	71,	10),	// cancel
		ui_button_info("2_CCB_11",	932,	555,	938,	619,	11),	// undo
		ui_button_info("2_CCB_12",	921,	1,		923,	71,	12),	// default
		ui_button_info("2_CCB_13",	732,	6,		726,	71,	13),	// search
		ui_button_info("2_CCB_14",	825,	6,		831,	71,	14),	// bind
		ui_button_info("2_CCB_15",	864,	685,	800,	704,	15),	// help
		ui_button_info("2_CCB_16",	919,	692,	914,	660,	16),	// accept
		ui_button_info("2_CCB_18",	672,	553,	668,	619,	18),	// clear other 
		ui_button_info("2_CCB_19",	761,	553,	749,	619,	19),	// clear all
		ui_button_info("2_CCB_20",	838,	553,	846,	619,	20),	// clear button
	}
};

// strings
#define CC_NUM_TEXT		20
UI_XSTR CC_text[GR_NUM_RESOLUTIONS][CC_NUM_TEXT] = {
	{ // GR_640
		{ "Targeting",		1340,		17,	384,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][TARGET_TAB].button },
		{ "Ship",			1341,		103,	384,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][SHIP_TAB].button },
		{ "Weapons",		1065,		154,	384,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][WEAPON_TAB].button },
		{ "Misc",			1411,		244,	384,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][COMPUTER_TAB].button },		
		{ "Alt",				1510,		12,	440,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][ALT_TOGGLE].button },
		{ "Shift",			1511,		50,	440,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][SHIFT_TOGGLE].button },
		{ "Invert",			1342,		155,	440,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][INVERT_AXIS].button },
		{ "Cancel",			641,		397,	45,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[0][CANCEL_BUTTON].button },
		{ "Undo",			1343,		586,	386,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][UNDO_BUTTON].button },
		{ "Defaults",		1344,		568,	45,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][RESET_BUTTON].button },
		{ "Search",			1345,		453,	45,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][SEARCH_MODE].button },
		{ "Bind",			1346,		519,	45,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[0][BIND_BUTTON].button },
		{ "Help",			928,		500,	440,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][HELP_BUTTON].button },
		{ "Accept",			1035,		571,	412,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[0][ACCEPT_BUTTON].button },
		{ "Clear",			1347,		417,	386,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][CLEAR_OTHER_BUTTON].button },
		{ "Conflict",		1348,		406,	396,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][CLEAR_OTHER_BUTTON].button },
		{ "Clear",			1413,		474,	386,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][CLEAR_ALL_BUTTON].button },
		{ "All",				1349,		483,	396,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[0][CLEAR_ALL_BUTTON].button },
		{ "Clear",			1414,		529,	388,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[0][CLEAR_BUTTON].button },
		{ "Selected",		1350,		517,	396,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[0][CLEAR_BUTTON].button },
	},
	{ // GR_1024
		{ "Targeting",		1340,		47,	615,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][TARGET_TAB].button },
		{ "Ship",			1341,		176,	615,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][SHIP_TAB].button },
		{ "Weapons",		1065,		266,	615,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][WEAPON_TAB].button },
		{ "Misc",			1411,		401,	615,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][COMPUTER_TAB].button },		
		{ "Alt",				1510,		29,	704,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][ALT_TOGGLE].button },
		{ "Shift",			1511,		85,	704,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][SHIFT_TOGGLE].button },
		{ "Invert",			1342,		254,	704,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][INVERT_AXIS].button },
		{ "Cancel",			641,		655,	71,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[1][CANCEL_BUTTON].button },
		{ "Undo",			1343,		938,	619,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][UNDO_BUTTON].button },
		{ "Defaults",		1344,		923,	71,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][RESET_BUTTON].button },
		{ "Search",			1345,		746,	71,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][SEARCH_MODE].button },
		{ "Bind",			1346,		846,	71,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[1][BIND_BUTTON].button },
		{ "Help",			928,		800,	704,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][HELP_BUTTON].button },
		{ "Accept",			1035,		914,	660,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[1][ACCEPT_BUTTON].button },
		{ "Clear",			1347,		683,	619,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][CLEAR_OTHER_BUTTON].button },
		{ "Conflict",		1348,		666,	634,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][CLEAR_OTHER_BUTTON].button },
		{ "Clear",			1413,		759,	619,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][CLEAR_ALL_BUTTON].button },
		{ "All",				1349,		772,	634,	UI_XSTR_COLOR_GREEN, -1, &CC_Buttons[1][CLEAR_ALL_BUTTON].button },
		{ "Clear",			1414,		871,	619,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[1][CLEAR_BUTTON].button },
		{ "Selected",		1350,		852,	634,	UI_XSTR_COLOR_PINK, -1, &CC_Buttons[1][CLEAR_BUTTON].button },
	}
};


// same indices as Scan_code_text[].  Indicates if a scancode is allowed to be bound.
int Config_allowed[] = {
	0, 0, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 0, 1, 0, 1, 0, 1,
	1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};

#ifndef NDEBUG
int Show_controls_info = 0;

DCF_BOOL(show_controls_info, Show_controls_info);

DCF(cc_adjust, "UI tool Used to adjust positioning and size of the controls config widgets") {
	bool done = false;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Available sub commands: \n");
		dc_printf("  list - configures control_list_coords\n");
		dc_printf("  col - configures the horizontal position and widths of control_list columns\n");
		dc_printf("\n Each sub command has their own arguments, pass 'help' to them for further details.");
		return;
	}

	if (dc_optional_string("list")) {
		// listbox
		if (dc_optional_string_either("help", "--help")) {
			dc_printf("Configures control_list_coords\n");
			dc_printf(" Available arguments: -x -y -w -h");
			return;
		}

		if (dc_optional_string("-x")) {
			dc_stuff_int(&Control_list_coords[gr_screen.res][CONTROL_X_COORD]);
			done = true;
		}
		if (dc_optional_string("-y")) {
			dc_stuff_int(&Control_list_coords[gr_screen.res][CONTROL_Y_COORD]);
			done = true;
		}
		if (dc_optional_string("-w")) {
			dc_stuff_int(&Control_list_coords[gr_screen.res][CONTROL_W_COORD]);
			done = true;
		}
		if (dc_optional_string("-h")) {
			dc_stuff_int(&Control_list_coords[gr_screen.res][CONTROL_H_COORD]);
			done = true;
		}

	} else if (dc_optional_string("col")) {
		if (dc_optional_string_either("help", "--help")) {
			dc_printf("Configures the column positioning and sizes for Control_list\n");
			dc_printf("  Avilable arguments: -x1 -w1 -x2");
			return;
		}

		if (dc_optional_string("-x1")) {
			dc_stuff_int(&Control_list_first_x[gr_screen.res]);
			done = true;
		}
		if (dc_optional_string("-w1")) {
			dc_stuff_int(&Control_list_first_w[gr_screen.res]);
			done = true;
		}
		if (dc_optional_string("-x2")) {
			dc_stuff_int(&Control_list_second_x[gr_screen.res]);
			done = true;
		}
	}

	if (!done) {
		dc_printf("Nothing done. Enter 'help' or '--help' for available arguments.");
	}
}
#endif

static int Axes_origin[JOY_NUM_AXES];


void control_config_do_undo();

// Rotate selItem forwards
selItem operator++(selItem& item, int) {
	Assert(item != selItem::selItem_END);
	using type = typename std::underlying_type<selItem>::type;

	item = static_cast<selItem>(static_cast<type>(item) + 1);

	if (item == selItem::selItem_END) {
		// Set to first item
		item = static_cast<selItem>(static_cast<type>(selItem::selItem_REND) + 1);;
	}

	return item;
}

// Rotate selItem backwards
selItem operator--(selItem& item, int) {
	Assert(item != selItem::selItem_REND);
	using type = typename std::underlying_type<selItem>::type;

	item = static_cast<selItem>( static_cast<type>(item) - 1 );
	
	if (item == selItem::selItem_REND) {
		// Set to last item
		item = static_cast<selItem>(static_cast<type>(selItem::selItem_END) - 1);;
	}

	return item;
}

static int joy_get_unscaled_reading(int raw)
{
	int rng;

	rng = JOY_AXIS_MAX - JOY_AXIS_MIN;
	raw -= JOY_AXIS_MIN;  // adjust for linear range starting at 0

	// cap at limits
	if (raw < 0)
		raw = 0;
	if (raw > rng)
		raw = rng;

	return (int) ((std::uint64_t) raw * (std::uint64_t) F1_0 / (std::uint64_t) rng);  // convert to 0 - F1_0 range, 64bit ints used to avoid uint overflow
}

int joy_get_scaled_reading(int raw)
{
	int x, d, dead_zone, rng;
	float percent, sensitivity_percent, non_sensitivity_percent;

	raw -= JOY_AXIS_CENTER;

	dead_zone = (JOY_AXIS_MAX - JOY_AXIS_MIN) * Joy_dead_zone_size / 100;

	if (raw < -dead_zone) {
		rng = JOY_AXIS_CENTER - JOY_AXIS_MIN - dead_zone;
		d = -raw - dead_zone;
	} else if (raw > dead_zone) {
		rng = JOY_AXIS_MAX - JOY_AXIS_CENTER - dead_zone;
		d = raw - dead_zone;
	} else
		return 0;

	if (d > rng)
		d = rng;

	Assert(Joy_sensitivity >= 0 && Joy_sensitivity <= 9);

	// compute percentages as a range between 0 and 1
	sensitivity_percent = (float) Joy_sensitivity / 9.0f;
	non_sensitivity_percent = (float) (9 - Joy_sensitivity) / 9.0f;

	// find percent of max axis is at
	percent = (float) d / (float) rng;

	// work sensitivity on axis value
	percent = (percent * sensitivity_percent + percent * percent * percent * percent * percent * non_sensitivity_percent);

	x = (int) ((float) F1_0 * percent);

	//nprintf(("AI", "d=%6i, sens=%3i, percent=%6.3f, val=%6i, ratio=%6.3f\n", d, Joy_sensitivity, percent, (raw<0) ? -x : x, (float) d/x));

	if (raw < 0)
		return -x;

	return x;
}

void control_config_detect_axis_reset()
{
	joystick_read_raw_axis(JOY_NUM_AXES, Axes_origin);
}

int control_config_detect_axis()
{
	int i, d, axis = -1, delta = 16384;
	int axes_values[JOY_NUM_AXES];
	int dx, dy, dz, fudge = 7;

	joystick_read_raw_axis(JOY_NUM_AXES, axes_values);
	for (i=0; i<JOY_NUM_AXES; i++) {
		d = abs(axes_values[i] - Axes_origin[i]);
		if (d > delta) {
			axis = i;
			delta = d;
		}
	}

	if ( (axis == -1) && Use_mouse_to_fly ) {
		mouse_get_delta( &dx, &dy, &dz );

		if ( (dx > fudge) || (dx < -fudge) ) {
			axis = 0;
		} else if ( (dy > fudge) || (dy < -fudge) ) {
			axis = 1;
		} else if ( (dz > fudge) || (dz < -fudge) ) {
			axis = 2;
		}
	}
		
	return axis;
}

/**
 * @brief Checks all controls for conflicts.  This should be called after any change to any bindings.
 */
void control_config_conflict_check()
{
	int i, j;
	const int CCFG_SIZE = static_cast<int>(Control_config.size());
	
	for (i=0; i < CCFG_SIZE; i++) {
		Conflicts[i].first = Conflicts[i].second = -1;
	}

	for (i=0; i<NUM_TABS; i++) {
		Conflicts_tabs[i] = 0;
	}

	for (i = 0; i < (CCFG_SIZE - 1); i++) {
		auto &item_i = Control_config[i];
		if (item_i.empty()) {
			// skip
			continue;
		}

		for (j = i + 1; j < CCFG_SIZE; j++) {
			auto &item_j = Control_config[j];
			if (item_j.empty()) {
				// skip
				continue;
			}

			if (item_i.disabled && (item_i.has_first(item_j) || item_i.has_second(item_j))) {
				// item_i conflicts with item_j and is disabled.  Silently clear item_i
				item_i.clear();
			}

			if (item_j.disabled && (item_j.has_first(item_i) || item_j.has_second(item_i))) {
				// item_j conflicts with item_i and is disabled.  Silently clear item_j
				item_j.clear();
			}

			// Clearly a headache
			// This mess is needed so that only the conflicting bind is highlighted instead of both of them
			if (item_i.has_first(item_j)) {
				// item_i's first binding has conflict
				Conflicts[i].first = j;
				Conflicts_tabs[ item_i.tab ] = 1;
			}

			if (item_i.has_second(item_j)) {
				// item_i's second bining has conflict
				Conflicts[i].second = j;
				Conflicts_tabs[ item_i.tab ] = 1;
			}

			if (item_j.has_first(item_i)) {
				// item_j's first binding has conflict
				Conflicts[j].first = i;
				Conflicts_tabs[item_j.tab] = 1;
			}

			if (item_j.has_second(item_i)) {
				// item_j's second binding has conflict
				Conflicts[j].second = i;
				Conflicts_tabs[ item_j.tab ] = 1;
			}
		}
	}

	for (i=0; i<NUM_JOY_AXIS_ACTIONS; i++) {
		Conflicts_axes[i] = -1;
	}

	for (i=0; i<NUM_JOY_AXIS_ACTIONS-1; i++) {
		for (j=i+1; j<NUM_JOY_AXIS_ACTIONS; j++) {
			if ((Axis_map_to[i] >= 0) && (Axis_map_to[i] == Axis_map_to[j])) {
				Conflicts_axes[i] = j;
				Conflicts_axes[j] = i;
				Conflicts_tabs[SHIP_TAB] = 1;
 			}
		}
	}
}

// do list setup required prior to rendering and checking for the controls listing.  Called when list changes
void control_config_list_prepare()
{
	int y;	// Offset, in pixels, the Cc_line has from the top
	int z;	// index into Control_config[]
	int font_height = gr_get_font_height();

	Num_cc_lines = y = z = 0;

	// Populate the digital controls
	for (const auto &item : Control_config) {
		if (item.tab == Tab && !item.disabled) {
			if (item.indexXSTR > 1) {
				Cc_lines[Num_cc_lines].label = XSTR(item.text.c_str(), item.indexXSTR, true);
			} else if (item.indexXSTR == 1) {
				Cc_lines[Num_cc_lines].label = XSTR(item.text.c_str(), CONTROL_CONFIG_XSTR + z, true);
			} else {
				Cc_lines[Num_cc_lines].label = item.text.c_str();
			}

			Cc_lines[Num_cc_lines].cc_index = z;
			Cc_lines[Num_cc_lines++].y = y;
			y += font_height + 2;
		} // Else, Ignore and hide items

		z++;	// z is the index position in Control_config[]
	}

	// Populate the analog controls.
	if (Tab == SHIP_TAB) {
		for (int j = 0; j < NUM_JOY_AXIS_ACTIONS; j++) {
			Cc_lines[Num_cc_lines].label = Joy_axis_action_text[j];
			Cc_lines[Num_cc_lines].cc_index = j | JOY_AXIS;
			Cc_lines[Num_cc_lines++].y = y;
			y += font_height + 2;
		}
	}
}

int cc_line_query_visible(int n)
{
	int y;

	if ((n < 0) || (n >= Num_cc_lines)) {
		return 0;
	}
	
	y = Cc_lines[n].y - Cc_lines[Scroll_offset].y;
	if ((y < 0) || (y + gr_get_font_height() > Control_list_coords[gr_screen.res][CONTROL_H_COORD])){
		return 0;
	}

	return 1;
}

/**
 * @brief Wrapper for CC_bind::take(), binds a key combo or button to the given control
 */
void control_config_bind_btn(int i, const CC_bind &new_bind, int order)
{
	switch (order) {
	// Bind states. Saving covered by below
	case 0:
	case 1:
		break;
		
	// Ignore states
	case -1:
		// Ignore silently
		return;

	// Error
	default:
		UNREACHABLE("Unknown order (%i) passed to control_config_bind_btn.", order);
	}

	// Save both bindings, because ::take() can clear the other binding if it is equal.
	Undo_stack stack;
	stack.save(Control_config[i].first);
	stack.save(Control_config[i].second);
	Undo_controls.save_stack(stack);

	Control_config[i].take(new_bind, order);
}


/**
 * @brief binds a joystick axis to the given control
 */
void control_config_bind_axis(int i, int axis)
{
	Undo_controls.save(Axis_map_to[i]);
	Axis_map_to[i] = axis;
}

/**
 * @brief Unbinds the selected control
 */
int control_config_remove_binding()
{
	int z;

	if (Selected_line < 0) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return -1;
	}

	z = Cc_lines[Selected_line].cc_index;
	if (z & JOY_AXIS) {
		z &= ~JOY_AXIS;
		if (Axis_map_to[z] < 0) {
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
			return -1;
		}

		Undo_controls.save(Axis_map_to[z]);
		Axis_map_to[z] = -1;
		control_config_conflict_check();
		control_config_list_prepare();
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		Selected_item = selItem::None;
		return 0;
	}

	switch (Selected_item) {
	case selItem::None:
		// Clear both
		if (!(Control_config[z].empty())) {
			Undo_stack stack;

			stack.save(Control_config[z].first);
			stack.save(Control_config[z].second);
			Undo_controls.save_stack(stack);

			Control_config[z].first.clear();
			Control_config[z].second.clear();

		} else {
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
			return -1;
		}
		break;

	case selItem::Primary:
		// Clear only primary
		if (!Control_config[z].first.empty()) {
			Undo_controls.save(Control_config[z].first);
			Control_config[z].first.clear();

		} else {
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
			return -1;
		}
		break;

	case selItem::Secondary:
		// Clear only Secondary
		if (!Control_config[z].second.empty()) {
			Undo_controls.save(Control_config[z].second);
			Control_config[z].second.clear();

		} else {
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
			return -1;
		}
		break;

	default:
		// Coder forgot to add a case!
		UNREACHABLE("Unhandled selItem case.");
	}

	control_config_conflict_check();
	control_config_list_prepare();
	gamesnd_play_iface(InterfaceSounds::USER_SELECT);
	Selected_item = selItem::None;
	return 0;
}

/**
 * @brief Clears all conflicting control bindings, except the selected control
 */
int control_config_clear_other()
{
	int z, i, total = 0;
	const int CCFG_SIZE = static_cast<int>(Control_config.size());

	if (Selected_line < 0) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return -1;
	}

	z = Cc_lines[Selected_line].cc_index;
	if (z & JOY_AXIS) {
		// is in Axis_map_to[]
		z &= ~JOY_AXIS;

		// Fail if axis is unbound
		if (Axis_map_to[z] < 0) {
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
			return -1;
		}

		Undo_stack stack;
		for (i = total = 0; i < NUM_JOY_AXIS_ACTIONS; i++) {
			if (i == z) {
				// skip
				continue;
			}

			if (Axis_map_to[i] == Axis_map_to[z]) {
				stack.save(Axis_map_to[i]);
				Axis_map_to[i] = -1;
				total++;
			}
		}

		// Fail if no conflicts
		if (total == 0) {
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
			return -1;
		}

		Undo_controls.save_stack(stack);

		control_config_conflict_check();
		control_config_list_prepare();
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		return 0;
	} // Else, is in Control_config

	const auto &selected = Control_config[z];

	// Fail if selected item is empty
	if (selected.empty()) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return -1;
	}

	// Back up the old bindings
	Undo_stack stack;

	for (i = total = 0; i < CCFG_SIZE; ++i) {
		if (i == z) {
			// skip
			continue;
		}

		auto &other = Control_config[i];

		if (other.has_first(selected)) {
			stack.save(other.first);
			other.first.clear();
			total++;
		}

		if (other.has_second(selected)) {
			stack.save(other.second);
			other.second.clear();
			total++;
		}
	}

	// Fail if no conflicts
	if (total == 0) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return -1;
	}

	Undo_controls.save_stack(stack);

	control_config_conflict_check();
	control_config_list_prepare();
	gamesnd_play_iface(InterfaceSounds::USER_SELECT);
	return 0;
}

/**
 * @brief Unbinds ALL controls
 * TODO: unbind axes and reset inversion
 */
int control_config_clear_all()
{
	int total = 0;

	// Back up items for undo and then clear the item
	Undo_stack stack;
	for (auto &item : Control_config) {
		if (!item.empty()) {
			stack.save(item.first);
			stack.save(item.second);

			item.clear();
			total++;
		}
	}

	// Fail if nothing was cleared
	if (total == 0) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return -1;
	}

	Undo_controls.save_stack(stack);

	control_config_conflict_check();
	control_config_list_prepare();
	gamesnd_play_iface(InterfaceSounds::RESET_PRESSED);
	return 0;
}

/**
 * @brief Gets the default axis binding
 */
int control_config_axis_default(int axis)
{
	Assert(axis >= 0);

	if ( axis > 1 ) {
		if (Axis_map_to_defaults[axis] < 0) {
			return -1;
		}

		auto joystick = io::joystick::getCurrentJoystick();
		if (joystick == nullptr || Axis_map_to_defaults[axis] >= joystick->numAxes()) {
			return -1;
		}
	}

	return Axis_map_to_defaults[axis];
}

/**
 * @brief Reverts all bindings to their preset. If already default, cycle to the next presets.
 */
int control_config_do_reset()
{
	int i, total = 0;
	Undo_stack stack;
	auto &default_bindings = Control_config_presets[0].bindings;

	// first, determine how many bindings need to be changed
	for (size_t e = 0; e < Control_config.size(); ++e) {
		auto item = Control_config[e];
		auto default_item = default_bindings[e];

		if (item.disabled) {
			// skip
			continue;
		}

		if ((item.first != default_item.first) ||
		    (item.second != default_item.second)) {
			total++;
		}
	}

	for (i=0; i<NUM_JOY_AXIS_ACTIONS; i++) {
		if ((Axis_map_to[i] != control_config_axis_default(i)) || (Invert_axis[i] != Invert_axis_defaults[i])) {
			total++;
		}
	}

	if ((total == 0) && (Control_config_presets.size() <= 1)) {
		// Nothing to reset, no other presets besides default
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return -1;
	}

	if (total == 0) {
		// Select next preset
		stack.save(Defaults_cycle_pos);

		Defaults_cycle_pos++;
		if (Defaults_cycle_pos >= Control_config_presets.size()) {
			Defaults_cycle_pos = 0;
		}
	}

	// Save all item bindings for undo
	for (auto &item : Control_config) {
		stack.save(item.first);
		stack.save(item.second);
	}
	Undo_controls.save_stack(stack);

	control_config_reset_defaults();

	control_config_conflict_check();
	control_config_list_prepare();
	gamesnd_play_iface(InterfaceSounds::RESET_PRESSED);
	return 0;
}

void control_config_reset_defaults()
{
	// Reset keyboard defaults
	const auto &preset = Control_config_presets[Defaults_cycle_pos].bindings;
	for (size_t i = 0; i < Control_config.size(); ++i) {
		Control_config[i].first = preset[i].first;
		Control_config[i].second = preset[i].second;
	}

	// Reset joy defaults.  No presets for joysticks currently
	for (int i = 0; i < NUM_JOY_AXIS_ACTIONS; i++) {
		Axis_map_to[i] = control_config_axis_default(i);
		Invert_axis[i] = Invert_axis_defaults[i];
	}
}

void control_config_scroll_screen_up()
{
	if (Scroll_offset) {
		Scroll_offset--;
		Assert(Selected_line > Scroll_offset);
		while (!cc_line_query_visible(Selected_line)) {
			Selected_line--;
		}

		Selected_item = selItem::None;
		gamesnd_play_iface(InterfaceSounds::SCROLL);

	} else {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void control_config_scroll_line_up()
{
	if (Selected_line) {
		Selected_line--;
		if (Selected_line < Scroll_offset) {
			Scroll_offset = Selected_line;
		}

		Selected_item = selItem::None;
		gamesnd_play_iface(InterfaceSounds::SCROLL);

	} else {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void control_config_scroll_screen_down()
{
	if (Cc_lines[Num_cc_lines - 1].y + gr_get_font_height() > Cc_lines[Scroll_offset].y + Control_list_coords[gr_screen.res][CONTROL_H_COORD]) {
		Scroll_offset++;
		while (!cc_line_query_visible(Selected_line)) {
			Selected_line++;
			Assert(Selected_line < Num_cc_lines);
		}

		Selected_item = selItem::None;
		gamesnd_play_iface(InterfaceSounds::SCROLL);

	} else {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void control_config_scroll_line_down()
{
	if (Selected_line < Num_cc_lines - 1) {
		Selected_line++;
		Assert(Selected_line > Scroll_offset);
		while (!cc_line_query_visible(Selected_line)) {
			Scroll_offset++;
		}

		Selected_item = selItem::None;
		gamesnd_play_iface(InterfaceSounds::SCROLL);

	} else {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void control_config_toggle_modifier(int bit)
{
	int k, z;

	z = Cc_lines[Selected_line].cc_index;
	Assert(!(z & JOY_AXIS));
	k = Control_config[z].get_btn(CID_KEYBOARD);
	if (k < 0) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return;
	}

	Undo_stack stack;
	stack.save(Control_config[z].first);
	stack.save(Control_config[z].second);
	Undo_controls.save_stack(stack);

	Control_config[z].take(CC_bind(CID_KEYBOARD, static_cast<short>(k ^ bit)), -1);
	control_config_conflict_check();
	gamesnd_play_iface(InterfaceSounds::USER_SELECT);
}

/**
 * @brief Toggles inversion for the selected axis control
 */
void control_config_toggle_invert()
{
	int z;

	z = Cc_lines[Selected_line].cc_index;
	Assert(z & JOY_AXIS);
	z &= ~JOY_AXIS;
	
	Undo_controls.save(Invert_axis[z]);
	Invert_axis[z] = !Invert_axis[z];
}

/*!
 * Sets menu in bind mode.  Menu will watch controller input and bind to the currently selected item, if any.
 */
void control_config_do_bind()
{
	int i;

	game_flush();
//	if ((Selected_line < 0) || (Cc_lines[Selected_line].cc_index & JOY_AXIS)) {
	if (Selected_line < 0) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return;
	}

	for (i=0; i<NUM_BUTTONS; i++) {
		if (i != CANCEL_BUTTON) {
			CC_Buttons[gr_screen.res][i].button.reset_status();
			CC_Buttons[gr_screen.res][i].button.disable();
		}
	}
	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.enable();
	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.set_hotkey(KEY_ESC);

	for (i=0; i<JOY_TOTAL_BUTTONS; i++){
		joy_down_count(i, 1);  // clear checking status of all joystick buttons
	}

	control_config_detect_axis_reset();

	Binding_mode = 1;
	Bind_time = timer_get_milliseconds();
	Search_mode = 0;
	Last_key = -1;
	Axis_override = -1;
	gamesnd_play_iface(InterfaceSounds::USER_SELECT);
}

/*!
 * @brief Sets menu in search mode.  Menu will watch for control input, search for the item that is bound to it, and then focus/highlight the item if found.
 */
void control_config_do_search()
{
	int i;

	for (i=0; i<NUM_BUTTONS; i++){
		if (i != CANCEL_BUTTON) {
			CC_Buttons[gr_screen.res][i].button.reset_status();
			CC_Buttons[gr_screen.res][i].button.disable();
		}
	}

	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.enable();
	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.set_hotkey(KEY_ESC);

	for (i=0; i<JOY_TOTAL_BUTTONS; i++){
		joy_down_count(i, 1);  // clear checking status of all joystick buttons
	}

	Binding_mode = 0;
	Search_mode = 1;
	Last_key = -1;
	gamesnd_play_iface(InterfaceSounds::USER_SELECT);
}

/*!
 * (Re)sets the menu mode to Browse mode.  This mode lets users browse through the bindings using controller input to navigate the item lists
 */
void control_config_do_cancel(int fail = 0)
{
	int i;

	game_flush();

	for (i=0; i<NUM_BUTTONS; i++){
		if ( (i != CANCEL_BUTTON) && (i != INVERT_AXIS) ){
			CC_Buttons[gr_screen.res][i].button.enable();
		}
	}

	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.reset_status();
	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.disable();
	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.set_hotkey(-1);
	CC_Buttons[gr_screen.res][BIND_BUTTON].button.reset_status();
	CC_Buttons[gr_screen.res][SEARCH_MODE].button.reset_status();

	Binding_mode = Search_mode = 0;
	if (fail){
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	} else {
		gamesnd_play_iface(InterfaceSounds::USER_SELECT);
	}
}

/*!
 * @brief Performs a single undo opration, reverting the most recent change to bindings, if any
 */
void control_config_do_undo() {
	Undo_controls.undo();
	control_config_conflict_check();

	gamesnd_play_iface(InterfaceSounds::USER_SELECT);
}

/*!
 * Does a cursory conflict check, then accepts changes to the bindings, if any, and request the menu to close.
 */
int control_config_accept()
{
	int i;

	for (i=0; i<NUM_TABS; i++) {
		if (Conflicts_tabs[i]) {
			break;
		}
	}

	if (i < NUM_TABS) {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
		return -1;
	}

	hud_squadmsg_save_keys();  // rebuild map for saving/restoring keys in squadmsg mode
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	gamesnd_play_iface(InterfaceSounds::COMMIT_PRESSED);
	return 0;
}

/*!
 * Reverts all changes, if any, and requests the menu to close.
 */
void control_config_cancel_exit()
{
	// Restore all bindings with the backup
	std::move(Control_config_backup.begin(), Control_config_backup.end(), Control_config.begin());
	std::move(Axis_map_to_backup, Axis_map_to_backup + NUM_JOY_AXIS_ACTIONS, Axis_map_to);
	std::move(Invert_axis_backup, Invert_axis_backup + JOY_NUM_AXES, Invert_axis);

	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

/**
 * Button widget handler
 */
void control_config_button_pressed(int n)
{
	switch (n) {
		case TARGET_TAB:
		case SHIP_TAB:
		case WEAPON_TAB:
		case COMPUTER_TAB:
			Tab = n;
			Scroll_offset = Selected_line = 0;
			control_config_list_prepare();
			gamesnd_play_iface(InterfaceSounds::SCREEN_MODE_PRESSED);
			break;

		case BIND_BUTTON:
			control_config_do_bind();
			break;

		case SEARCH_MODE:
			control_config_do_search();
			break;

		case SHIFT_TOGGLE:
			control_config_toggle_modifier(KEY_SHIFTED);
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			break;

		case ALT_TOGGLE:
			control_config_toggle_modifier(KEY_ALTED);
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			break;

		case INVERT_AXIS:
			control_config_toggle_invert();
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			break;

		case SCROLL_UP_BUTTON:
			control_config_scroll_screen_up();
			break;

		case SCROLL_DOWN_BUTTON:
			control_config_scroll_screen_down();
			break;

		case ACCEPT_BUTTON:
			control_config_accept();
			break;

		case CLEAR_BUTTON:
			control_config_remove_binding();
			break;

		case HELP_BUTTON:
			launch_context_help();
			gamesnd_play_iface(InterfaceSounds::HELP_PRESSED);
			break;

		case RESET_BUTTON:
			control_config_do_reset();
			break;

		case UNDO_BUTTON:
			control_config_do_undo();
			break;

		case CANCEL_BUTTON:
			control_config_do_cancel();
			break;

		case CLEAR_OTHER_BUTTON:
			control_config_clear_other();
			break;

		case CLEAR_ALL_BUTTON:
			control_config_clear_all();
			break;		
	}
}

/*!
 * Localization handler for "@Conflict", should str equal it (case insensitive)
 */
const char *control_config_tooltip_handler(const char *str)
{
	int i;

	if (!stricmp(str, NOX("@conflict"))) {
		for (i=0; i<NUM_TABS; i++) {
			if (Conflicts_tabs[i]) {
				return XSTR( "Conflict!", 205);
			}
		}
	}

	return NULL;
}

void control_config_init()
{
	int i;
	ui_button_info *b;

	// Init the backup vectors
	Control_config_backup.clear();
	Control_config_backup.reserve(Control_config.size());
	std::copy(Control_config.begin(), Control_config.end(), std::back_inserter(Control_config_backup));

	std::copy(Axis_map_to, Axis_map_to + NUM_JOY_AXIS_ACTIONS, Axis_map_to_backup);
	std::copy(Invert_axis, Invert_axis + JOY_NUM_AXES, Invert_axis_backup);

	// Init conflict vector
	Conflicts.clear();
	Conflicts.resize(Control_config.size());

	// Init Cc_lines
	Cc_lines.clear();
	Cc_lines.resize(Control_config.size() + NUM_JOY_AXIS_ACTIONS);	// Can't use CCFG_MAX here, since scripts or might add controls

	common_set_interface_palette(NOX("ControlConfigPalette"));  // set the interface palette
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Conflict_background_bitmap_mask_fname[gr_screen.res]);
	Ui_window.tooltip_handler = control_config_tooltip_handler;

	// load in help overlay bitmap	
	Control_config_overlay_id = help_overlay_get_index(CONTROL_CONFIG_OVERLAY);
	help_overlay_set_state(Control_config_overlay_id,gr_screen.res,0);

	// reset conflict flashing
	Conflict_stamp = -1;

	for (i=0; i<NUM_BUTTONS; i++) {
		b = &CC_Buttons[gr_screen.res][i];

		if (b->hotspot < 0) {  // temporary
			b->button.create(&Ui_window, NOX("Clear other"), b->x, b->y, 150, 30, 0, 1);  // temporary
			b->button.set_highlight_action(common_play_highlight_sound);
			continue;
		}

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, ((i == SCROLL_UP_BUTTON) || (i == SCROLL_DOWN_BUTTON)), 1);

		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);		
		if (i<4) {
			b->button.set_bmaps(b->filename, 5, 1);		// a bit of a hack here, but buttons 0-3 need 4 frames loaded
		} else {
			b->button.set_bmaps(b->filename);
		}
		b->button.link_hotspot(b->hotspot);
	}	

	// create all text
	for(i=0; i<CC_NUM_TEXT; i++){
		Ui_window.add_XSTR(&CC_text[gr_screen.res][i]);
	}

	for (i=0; i<LIST_BUTTONS_MAX; i++) {
		List_buttons[i].create(&Ui_window, "", 0, 0, 60, 30, 0, 1);
		List_buttons[i].hide();
		List_buttons[i].disable();
	}

	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	CC_Buttons[gr_screen.res][SCROLL_UP_BUTTON].button.set_hotkey(KEY_PAGEUP);
	CC_Buttons[gr_screen.res][SCROLL_DOWN_BUTTON].button.set_hotkey(KEY_PAGEDOWN);
	CC_Buttons[gr_screen.res][BIND_BUTTON].button.set_hotkey(KEY_ENTER);
	CC_Buttons[gr_screen.res][CLEAR_OTHER_BUTTON].button.set_hotkey(KEY_CTRLED | KEY_DELETE);
	CC_Buttons[gr_screen.res][UNDO_BUTTON].button.set_hotkey(KEY_CTRLED | KEY_Z);
	CC_Buttons[gr_screen.res][CLEAR_BUTTON].button.set_hotkey(KEY_DELETE);
	CC_Buttons[gr_screen.res][ACCEPT_BUTTON].button.set_hotkey(KEY_CTRLED | KEY_ENTER);
	CC_Buttons[gr_screen.res][HELP_BUTTON].button.set_hotkey(KEY_F1);
	CC_Buttons[gr_screen.res][RESET_BUTTON].button.set_hotkey(KEY_CTRLED | KEY_R);
	CC_Buttons[gr_screen.res][INVERT_AXIS].button.set_hotkey(KEY_I);

	CC_Buttons[gr_screen.res][CANCEL_BUTTON].button.disable();
	CC_Buttons[gr_screen.res][CLEAR_OTHER_BUTTON].button.disable();

	Background_bitmap = bm_load(Conflict_background_bitmap_fname[gr_screen.res]);	

	Scroll_offset = Selected_line = 0;
	control_config_conflict_check();

	// setup strings					
	Joy_axis_action_text[0] = vm_strdup(XSTR("Turn (Yaw) Axis", 1016));
	Joy_axis_action_text[1] = vm_strdup(XSTR("Pitch Axis", 1017));
	Joy_axis_action_text[2] = vm_strdup(XSTR("Bank Axis", 1018));
	Joy_axis_action_text[3] = vm_strdup(XSTR("Absolute Throttle Axis", 1019));
	Joy_axis_action_text[4] = vm_strdup(XSTR("Relative Throttle Axis", 1020));
	Joy_axis_text[0] = vm_strdup(XSTR("Joystick/Mouse X Axis", 1021));
	Joy_axis_text[1] = vm_strdup(XSTR("Joystick/Mouse Y Axis", 1022));
	Joy_axis_text[2] = vm_strdup(XSTR("Joystick Z Axis", 1023));
	Joy_axis_text[3] = vm_strdup(XSTR("Joystick rX Axis", 1024));
	Joy_axis_text[4] = vm_strdup(XSTR("Joystick rY Axis", 1025));
	Joy_axis_text[5] = vm_strdup(XSTR("Joystick rZ Axis", 1026));
	Mouse_button_text[0] = vm_strdup("");
	Mouse_button_text[1] = vm_strdup(XSTR("Left Button", 1027));
	Mouse_button_text[2] = vm_strdup(XSTR("Right Button", 1028));
	Mouse_button_text[3] = vm_strdup(XSTR("Mid Button", 1029));
	Mouse_button_text[4] = vm_strdup("");
	Mouse_axis_text[0] = vm_strdup(XSTR("L/R", 1030));
	Mouse_axis_text[1] = vm_strdup(XSTR("U/B", 1031));
	Invert_text[0] = vm_strdup(XSTR("N", 1032));
	Invert_text[1] = vm_strdup(XSTR("Y", 1033));

	control_config_list_prepare();
}

void control_config_close()
{
	int idx;
	
	if (Background_bitmap){
		bm_release(Background_bitmap);
	}

	Ui_window.destroy();
	common_free_interface_palette();		// restore game palette
	hud_squadmsg_save_keys();				// rebuild map for saving/restoring keys in squadmsg mode
	game_flush();

	if (Game_mode & GM_MULTIPLAYER) {
		Pilot.save_player();
	} else {
		Pilot.save_savefile();
	}

	// free strings	
	for(idx=0; idx<NUM_JOY_AXIS_ACTIONS; idx++){
		if(Joy_axis_action_text[idx] != NULL){
			vm_free(Joy_axis_action_text[idx]);
			Joy_axis_action_text[idx] = NULL;
		}
	}
	for(idx=0; idx<NUM_AXIS_TEXT; idx++){
		if(Joy_axis_text[idx] != NULL){
			vm_free(Joy_axis_text[idx]);
			Joy_axis_text[idx] = NULL;
		}
	}
	for(idx=0; idx<NUM_MOUSE_TEXT; idx++){
		if(Mouse_button_text[idx] != NULL){
			vm_free(Mouse_button_text[idx]);
			Mouse_button_text[idx] = NULL;
		}
	}
	for(idx=0; idx<NUM_MOUSE_AXIS_TEXT; idx++){
		if(Mouse_axis_text[idx] != NULL){
			vm_free(Mouse_axis_text[idx]);
			Mouse_axis_text[idx] = NULL;
		}
	}
	for(idx=0; idx<NUM_INVERT_TEXT; idx++){
		if(Invert_text[idx] != NULL){
			vm_free(Invert_text[idx]);
			Invert_text[idx] = NULL;
		}
	}

	// Free up memory from dynamic containers
	Control_config_backup.clear();
	Cc_lines.clear();
	Conflicts.clear();
	Undo_controls.clear();
}

/**
 * @brief Display the currently selected preset
 */
void control_config_draw_selected_preset() {
	SCP_string preset_str;
	auto preset_it = Control_config_presets.begin();

	// Find the matching preset.
	// We do this instead of relying on Defaults_cycle_pos because the player may end up duplicating a preset
	for (; preset_it != Control_config_presets.end(); ++preset_it) {
		bool found_match = true;

		// Check digital controls
		for (size_t i = 0; i < Control_config.size(); ++i) {
			if (Control_config[i].disabled) {
				// Skip
				continue;
			}

			// Check key
			if (Control_config[i].first != preset_it->bindings[i].first) {
				found_match = false;
				break;
			}

			// Check Joy
			if (Control_config[i].second != preset_it->bindings[i].second) {
				found_match = false;
				break;
			}
		}


		if (found_match) {
			break;
		}
	}

	if (preset_it != Control_config_presets.end()) {
		sprintf(preset_str, "Controls: %s", preset_it->name.c_str());
	} else {
		sprintf(preset_str, "Controls: custom");
	}

	// Draw the string
	int font_height = gr_get_font_height();
	int w;
	gr_get_string_size(&w, nullptr, preset_str.c_str());
	gr_set_color_fast(&Color_text_normal);

	if (gr_screen.res == GR_640) {
		gr_string(16, (24 - font_height) / 2, preset_str.c_str(), GR_RESIZE_MENU);
	} else {
		gr_string(24, (40 - font_height) / 2, preset_str.c_str(), GR_RESIZE_MENU);
	}
}

/**
 * Sets the color for binding text according to various states
 *
 * @param[in] line                  Current line to render
 * @param[in] select_tease_line     Line that the mouse is hovering over
 * @param[in] item                  Which selItem to test against
 * @param[in] empty                 True if the item is empty or not
 *
 * @returns 0 if no conflicts found, or
 * @returns 1 if a conflict was found
 */
int set_item_color(int line, int select_tease_line, selItem item, bool empty) {
	int z = Cc_lines[line].cc_index;
	int conflict_id = -1;
	int found_conflict = 0;	// Is this even needed?

	if (z & JOY_AXIS) {
		// analogue
		z &= ~JOY_AXIS;

		conflict_id = Conflicts_axes[z];

	} else {
		// digital
		switch (item) {
		case selItem::Primary:
			conflict_id = Conflicts[z].first;
			break;
		case selItem::Secondary:
			conflict_id = Conflicts[z].second;
			break;
		default:
			UNREACHABLE("Invalid selItem passed to set_item_color: %i", static_cast<int>(item));
		}
	}

	if (conflict_id >= 0) {
		// In conflict
		if ((line == Selected_line) && (Selected_item == item)) {
			// Highlight selected item 
			gr_set_color_fast(&Color_text_error_hi);
		} else {
			gr_set_color_fast(&Color_text_error);
		}
		found_conflict++;
	} else if (line == Selected_line) {
		// Highlight selected item
		if (Selected_item == item) {
			gr_set_color_fast(&Color_text_selected);
		} else {
			gr_set_color_fast(&Color_text_subselected);
		}
	} else if (line == select_tease_line) {
		gr_set_color_fast(&Color_text_subselected);
	} else if (empty) {
		gr_set_color_fast(&Color_grey);
	} else {
		gr_set_color_fast(&Color_text_normal);
	}

	return found_conflict;
}

/*!
 * Draws the list box of controls and their bindings
 */
int control_config_draw_list(int select_tease_line) {
	color* c;        // Color to draw the text
	int conflict = 0;   // 0 if no conflict found
	int line;       // Current line to render
	int i;  // Generic index
	//int w;  // width of the string to draw (pixels)
	//int h;  // height of the string to draw (pixels)
	int x;  // x coordinate of text anchor
	int y;  // y coordinate of text anchor
	int z;  // cc_index of the line being drawn
	char buf[256];  // c_str buffer
	int font_height = gr_get_font_height();

	for (line = Scroll_offset; cc_line_query_visible(line); ++line) {
		z = Cc_lines[line].cc_index;

		// screen coordinate y = list box origin y + (this item's relative y - topmost item's relative y)
		y = Control_list_coords[gr_screen.res][CONTROL_Y_COORD] + Cc_lines[line].y - Cc_lines[Scroll_offset].y;

		// Update the y position for this button
		List_buttons[line - Scroll_offset].update_dimensions(Control_list_coords[gr_screen.res][CONTROL_X_COORD], y, Control_list_coords[gr_screen.res][CONTROL_W_COORD], font_height);
		
		// Enable selection/hover of items when not in binding mode
		List_buttons[line - Scroll_offset].enable(!Binding_mode);

		Cc_lines[line].kw = Cc_lines[line].jw = 0;

		if (line == Selected_line) {
			c = &Color_text_selected;
		} else if (line == select_tease_line) {
			c = &Color_text_subselected;
		} else {
			c = &Color_text_normal;
		}

		gr_set_color_fast(c);
		if (Cc_lines[line].label) {
			strcpy_s(buf, Cc_lines[line].label);
			font::force_fit_string(buf, 255, Control_list_ctrl_w[gr_screen.res]);
			gr_printf_menu(Control_list_coords[gr_screen.res][CONTROL_X_COORD], y, "%s", buf);
		}

		if (!(z & JOY_AXIS)) {
			// is a digital control
			// Textify and print the primary and secondary bindings
			SCP_string first = Control_config[z].first.textify();
			SCP_string second = Control_config[z].second.textify();

			// Set color for primary according to state
			conflict += set_item_color(line, select_tease_line, selItem::Primary, Control_config[z].first.empty());

			// Print primary
			x = Control_list_first_x[gr_screen.res];
			*buf = 0;
			strcpy_s(buf, first.c_str());
			font::force_fit_string(buf, 255, Control_list_first_w[gr_screen.res]);
			gr_printf_menu(x, y, "%s", buf);

			Cc_lines[line].kx = x - Control_list_coords[gr_screen.res][CONTROL_X_COORD];
			Cc_lines[line].kw = Control_list_first_w[gr_screen.res];

			// Set color for secondary according to state
			conflict += set_item_color(line, select_tease_line, selItem::Secondary, Control_config[z].second.empty());

			// Print secondary
			x = Control_list_second_x[gr_screen.res];
			*buf = 0;
			strcpy_s(buf, second.c_str());
			font::force_fit_string(buf, 255, Control_list_first_w[gr_screen.res]);
			gr_printf_menu(x, y, "%s", buf);

			Cc_lines[line].jx = x - Control_list_coords[gr_screen.res][CONTROL_X_COORD];
			Cc_lines[line].jw = Control_list_first_w[gr_screen.res];

		} else {
			// Is an analogue control
			x = Control_list_first_x[gr_screen.res];
			int j = Axis_map_to[z & ~JOY_AXIS];
			if (Binding_mode && (line == Selected_line)) {
				j = Axis_override;
			}

			conflict += set_item_color(line, select_tease_line, selItem::Primary, (j < 0));
			
			if (j < 0) {
				gr_printf_menu(x, y, "%s", XSTR("None", 211));
			} else {
				gr_string(x, y, Joy_axis_text[j], GR_RESIZE_MENU);
			}
		}
	}

	// Disable remaining empty lines
	i = line - Scroll_offset;
	while (i < LIST_BUTTONS_MAX) {
		List_buttons[i++].disable();
	}

	return conflict;
}

void control_config_do_frame(float frametime)
{
	const char *str;
	char buf[256];
	int i; // generic index
	const int CCFG_SIZE = static_cast<int>(Control_config.size());	// hack to get around signed/unsigned mismatch errors
	int w, x, y, conflict;
	int k; // polled key.  Can be masked with SHIFT and/or ALT
	short j; // polled joy button
	int a; // polled joy axis
	int z = Cc_lines[Selected_line].cc_index; // Selected line's cc_index; value: (z &= ~JOY_AXIS); Is an axis index if (z & JOY_AXIS) == true;
	int font_height = gr_get_font_height();
	int select_tease_line = -1;  // line mouse is down on, but won't be selected until button released
	static float timer = 0.0f;
	static int bound_timestamp = 0;
	static char bound_string[40];
	
	timer += frametime;

	if (Binding_mode) {
		bool bind = false;	// is true if binding should happen.  Actually is an "Input detected" flag.
		bool done = false;	// is true if we're done binding and ready for exiting this mode

		// Poll for keypress
		k = game_poll();
		Ui_window.use_hack_to_get_around_stupid_problem_flag = 1;
		Ui_window.process(0);

		if (k == KEY_ENTER) {
			// Cancel axis bind if Enter is pressed
			bind = true;
		}

		// Poll for joy btn presses
		for (j = 0; j < JOY_TOTAL_BUTTONS; j++) {
			if (joy_down_count(j, 1)) {
				// btn is down, save it in j
				// Cancel axis bind if any button is pressed
				bind = true;
				break;
			}
		}

		if (help_overlay_active(Control_config_overlay_id)) {
			// Help overlay is active.  Reset the Help button state and ignore gadgets
			CC_Buttons[gr_screen.res][HELP_BUTTON].button.reset_status();
			Ui_window.set_ignore_gadgets(1);

			if ((k > 0) || (j < JOY_TOTAL_BUTTONS) || B1_JUST_RELEASED) {
				// If a key, joy, or mouse button was pressed, dismiss the overlay, watch gadgets, and consume them
				help_overlay_set_state(Control_config_overlay_id, gr_screen.res, 0);
				Ui_window.set_ignore_gadgets(0);
				k = 0;
				j = JOY_TOTAL_BUTTONS;
			}
		} else {
			// Help overlay is not active, watch gadgets
			Ui_window.set_ignore_gadgets(0);
		}
		
		if (k == KEY_ESC) {
			// Cancel bind if ESC is pressed
			strcpy_s(bound_string, XSTR("Canceled", 206));
			bound_timestamp = timestamp(2500);
			control_config_do_cancel();

		} else if (z & JOY_AXIS) {
			// Is an analogue control
			// Poll for joy axis
			z &= ~JOY_AXIS;
			a = control_config_detect_axis();
			if (a >= 0) {
				Axis_override = a;
				bind = true;
			}

			if (!done && bind) {
				if (Axis_override >= 0) {
					control_config_bind_axis(z, Axis_override);
					done = true;
					strcpy_s(bound_string, Joy_axis_text[Axis_override]);

				} else {
					// Canceled
					control_config_do_cancel(1);
				}
			}

		} else {
			// Is a digital control
			switch (k & KEY_MASK) {
				case KEY_LSHIFT:
				case KEY_RSHIFT:
				case KEY_LALT:
				case KEY_RALT:
					// k is a modifier.  Store the mask in Last_key and consume k
					Last_key = k & KEY_MASK;
					k = 0;
					break;
			}

			if ((z == BANK_WHEN_PRESSED || z == GLIDE_WHEN_PRESSED) &&
			    (Last_key >= 0) &&
			    (k <= 0) &&
			    !keyd_pressed[Last_key]) {
				// If the selected cc_item is BANK_WHEN_PRESSED or GLIDE_WHEN_PRESSED, and
				// If the polled key is a modifier, and
				// k was consumed, and
				// the key was just released, then
					// allow binding the modifier key by itself
				k = Last_key;
			}

			if ((k > 0) && !Config_allowed[k & KEY_MASK]) {
				// This key isn't allowed to be bound.  Consume k and inform the player
				popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR( "That is a non-bindable key.  Please try again.", 207));
				k = 0;
			}

			k &= (KEY_MASK | KEY_SHIFTED | KEY_ALTED);	// This shouldn't be needed, but just in case...
			if (!done && (k > 0)) {
				Assert(!(z & JOY_AXIS));
				control_config_bind_btn(z, CC_bind(CID_KEYBOARD, static_cast<short>(k)), 0);

				strcpy_s(bound_string, textify_scancode(k));
				done = true;
			}

			if (!done && (j < JOY_TOTAL_BUTTONS)) {
				Assert(!(z & JOY_AXIS));
				control_config_bind_btn(z, CC_bind(CID_JOY0, j), 1);

				strcpy_s(bound_string, Joy_button_text[j]);
				done = true;
			}

			// Debounce timer to allow mouse double-click (maybe?)
			if (!done && (Bind_time + 375 < timer_get_milliseconds())) {
				for (i=0; i<NUM_BUTTONS; i++){
					if ( (CC_Buttons[gr_screen.res][i].button.is_mouse_on()) && (CC_Buttons[gr_screen.res][i].button.enabled()) ){
						break;
					}
				}

				if (i == NUM_BUTTONS) {  // no buttons pressed, go ahead with polling the mouse
					for (i=0; i<MOUSE_NUM_BUTTONS; i++) {
						if (mouse_down(1 << i)) {
							Assert(!(z & JOY_AXIS));
							control_config_bind_btn(z, CC_bind(CID_JOY0, static_cast<short>(i)), 1);

							strcpy_s(bound_string, Joy_button_text[i]);
							done = true;

							break;
						}
					}
				}
			}
		}

		if (done) {
			// done with binding mode, clean up and prepare for display
			font::force_fit_string(bound_string, 39, Conflict_wnd_coords[gr_screen.res][CONTROL_W_COORD]);
			bound_timestamp = timestamp(2500);
			control_config_conflict_check();
			control_config_list_prepare();
			control_config_do_cancel();
			for (j = 0; j < NUM_BUTTONS; j++) {
				CC_Buttons[gr_screen.res][j].button.reset();
			}
		}

	} else if (Search_mode) {
		// Poll for keys
		k = game_poll();
		Ui_window.use_hack_to_get_around_stupid_problem_flag = 1;
		Ui_window.process(0);

		// Poll for joy buttons
		for (j = 0; j < JOY_TOTAL_BUTTONS; j++) {
			if (joy_down_count(j, 1)) {
				// btn is down, save it in j
				break;
			}
		}

		if (help_overlay_active(Control_config_overlay_id)) {
			// Help overlay is active.  Reset the Help button state and ignore gadgets
			CC_Buttons[gr_screen.res][HELP_BUTTON].button.reset_status();
			Ui_window.set_ignore_gadgets(1);

			if ((k > 0) || (j < JOY_TOTAL_BUTTONS) || B1_JUST_RELEASED) {
				// If a key, joy, or mouse button was pressed, dismiss the overlay, watch  gadgets, and consume them
				help_overlay_set_state(Control_config_overlay_id, gr_screen.res, 0);
				Ui_window.set_ignore_gadgets(0);
				k = 0;
			}
		} else {
			// Overlay not active, watch gadgets
			Ui_window.set_ignore_gadgets(0);
		}

		if (k == KEY_ESC) {
			// Cancel search if ESC is pressed
			control_config_do_cancel();

		} else {
			if ((k > 0) && !Config_allowed[k & KEY_MASK]) {
				// Ignore disallowed keys
				k = 0;
			}

			k &= (KEY_MASK | KEY_SHIFTED | KEY_ALTED);
			z = -1;
			// If not done, Find the control bound to the given key
			if ((z < 0) && (k > 0)) {
				for (i=0; i < CCFG_SIZE; ++i) {
					if (Control_config[i].first == CC_bind(CID_KEYBOARD, static_cast<short>(k))) {
						Selected_item = selItem::Primary;
						z = i;
						break;
					} else if (Control_config[i].second == CC_bind(CID_KEYBOARD, static_cast<short>(k))) {
						Selected_item = selItem::Secondary;
						z = i;
						break;
					}
				}
			}

			// If not done, Find the control bound to the given joy
			if ((z < 0) && (j < JOY_TOTAL_BUTTONS)) {
				for (i = 0; i < CCFG_SIZE; ++i) {
					if (Control_config[i].first == CC_bind(CID_JOY0, j)) {
						Selected_item = selItem::Primary;
						z = i;
						break;
					} else if (Control_config[i].second == CC_bind(CID_JOY0, j)) {
						Selected_item = selItem::Secondary;
						z = i;
						break;
					}
				}
			}

			// check if not on enabled button
			for (i = 0; i < NUM_BUTTONS; ++i){
				if ( (CC_Buttons[gr_screen.res][i].button.is_mouse_on()) && (CC_Buttons[gr_screen.res][i].button.enabled()) ){
					break;
				}
			}

			// If not done, and no buttons pressed, poll the mouse and find controls bound to buttons
			if ((z < 0) && (i == NUM_BUTTONS)) {
				for (j = 0; j < MOUSE_NUM_BUTTONS; ++j) {
					if (mouse_down(1 << j)) {
						// Find the control bound to the given mouse button (as joy button)
						for (i = 0; i < CCFG_SIZE; ++i) {
							if (Control_config[i].first == CC_bind(CID_JOY0, j)) {
								Selected_item = selItem::Primary;
								z = i;
								break;
							} else if (Control_config[i].second == CC_bind(CID_JOY0, j)) {
								Selected_item = selItem::Secondary;
								z = i;
								break;
							}
						}
						break;
					}
				}
			}

			// If done, Focus on the found control
			if (z >= 0) {
				Tab = Control_config[z].tab;
				control_config_list_prepare();
				Selected_line = Scroll_offset = 0;
				
				// Reverse Lookup cc_index to find the line its on
				for (i=0; i<Num_cc_lines; i++) {
					if (Cc_lines[i].cc_index == z) {
						Selected_line = i;
						break;
					}
				}
				Assert(i != Num_cc_lines);

				// Scroll to line if it is not visible
				while (!cc_line_query_visible(Selected_line)) {
					Scroll_offset++;
					Assert(Scroll_offset < Num_cc_lines);
				}

				// Reset all nav buttons
				for (size_t buttonid = 0; buttonid < NUM_BUTTONS; buttonid++) {
					CC_Buttons[gr_screen.res][buttonid].button.reset();
				}
			}
		}

	} else {
		// Browse/default mode

		//Enable modifier buttons according to selected item type
		z = Cc_lines[Selected_line].cc_index & JOY_AXIS;
		CC_Buttons[gr_screen.res][ALT_TOGGLE].button.enable(!z);    // Enabled for keys/buttons
		CC_Buttons[gr_screen.res][SHIFT_TOGGLE].button.enable(!z);  // Enabled for keys/buttons
		CC_Buttons[gr_screen.res][INVERT_AXIS].button.enable(z);    // Enabled for axes

		// If selected item is not an axis, and
		// If the bound key is a modifier, disable the modifier UI buttons
		if (!z) {
			z = Cc_lines[Selected_line].cc_index;
			k = Control_config[z].get_btn(CID_KEYBOARD);

			if ((k == KEY_LALT) || (k == KEY_RALT) || (k == KEY_LSHIFT) || (k == KEY_RSHIFT) ) {
				CC_Buttons[gr_screen.res][ALT_TOGGLE].button.enable(0);
				CC_Buttons[gr_screen.res][SHIFT_TOGGLE].button.enable(0);
			}
		}

		// Enable the undo button if our undo stack has something in it
		CC_Buttons[gr_screen.res][UNDO_BUTTON].button.enable(!Undo_controls.empty());

		// Poll for keypress (navigational only)
		k = Ui_window.process();

		// Poll for joy buttons
		for (j = 0; j < JOY_TOTAL_BUTTONS; j++) {
			if (joy_down_count(j, 1)) {
				// btn is down, save it in j
				break;
			}
		}

		if ( help_overlay_active(Control_config_overlay_id) ) {
			// If the help overlay is active, reset the help button state and ignore gadgets.
			CC_Buttons[gr_screen.res][HELP_BUTTON].button.reset_status();
			Ui_window.set_ignore_gadgets(1);

			if ((k > 0) || (j < JOY_TOTAL_BUTTONS) || B1_JUST_RELEASED) {
				// If a key, joy, or mouse button was pressed, dismiss the overlay, watch  gadgets, and consume them
				help_overlay_set_state(Control_config_overlay_id, gr_screen.res, 0);
				Ui_window.set_ignore_gadgets(0);
				k = 0;
			}
		} else {
			// Overlay not active, watch gadgets
			Ui_window.set_ignore_gadgets(0);
		}

		// Navigate according to keypress
		switch (k) {
			case KEY_DOWN:  // select next line
				control_config_scroll_line_down();
				break;

			case KEY_UP:  // select previous line
				control_config_scroll_line_up();
				break;

			case KEY_SHIFTED | KEY_TAB:  // activate previous tab
				Tab--;
				if (Tab < 0) {
					Tab = NUM_TABS - 1;
				}

				Scroll_offset = Selected_line = 0;
				control_config_list_prepare();
				gamesnd_play_iface(InterfaceSounds::SCREEN_MODE_PRESSED);
				break;

			case KEY_TAB:  // activate next tab
				Tab++;
				if (Tab >= NUM_TABS) {
					Tab = 0;
				}

				Scroll_offset = Selected_line = 0;
				control_config_list_prepare();
				gamesnd_play_iface(InterfaceSounds::SCREEN_MODE_PRESSED);
				break;

			case KEY_LEFT:
				// Select Previous item
				Selected_item--;

				if (z & JOY_AXIS) {
					// is analogue
					if (Selected_item == selItem::Secondary) {
						// Currently only one axis to bind to an axis item
						Selected_item = selItem::Primary;
					}

					if ((Axis_map_to[z & ~JOY_AXIS] < 0) && (Selected_item == selItem::Primary)) {
						// Nothing bound, set to None
						Selected_item = selItem::None;
					}
				}
				gamesnd_play_iface(InterfaceSounds::SCROLL);
				break;

			case KEY_RIGHT:
				// Next item
				Selected_item++;

				if (z & JOY_AXIS) {
					// is analogue
					if ((Axis_map_to[z & ~JOY_AXIS] < 0) && (Selected_item == selItem::Primary)) {
						// Nothing bound, set to None
						Selected_item = selItem::None;
					}

					if (Selected_item == selItem::Secondary) {
						// Currently only one axis to bind to an axis item
						Selected_item = selItem::None;
					}
				}

				gamesnd_play_iface(InterfaceSounds::SCROLL);
				break;

			case KEY_BACKSP:
				// undo last action
				control_config_do_undo();
				break;

			case KEY_ESC:
				// Escape from menu
				control_config_cancel_exit();
				break;
		}	// end switch
	}	// End mode specific logic

	// Process UI Button presses
	for (i=0; i<NUM_BUTTONS; i++){
		if (CC_Buttons[gr_screen.res][i].button.pressed()){
			control_config_button_pressed(i);
		}
	}

	// Process list box
	for (i=0; i<LIST_BUTTONS_MAX; i++) {
		if (List_buttons[i].is_mouse_on()) {
			// Set this line's state as mouse-over
			select_tease_line = i + Scroll_offset;
		}
	
		if (List_buttons[i].pressed()) {
			// Select the pressed line
			Selected_line = i + Scroll_offset;
			Selected_item = selItem::None;
			List_buttons[i].get_mouse_pos(&x, &y);

			// If the mouse is over the Primary binding, select it
			if ((x >= Cc_lines[Selected_line].kx) && (x < Cc_lines[Selected_line].kx + Cc_lines[Selected_line].kw)) {
				Selected_item = selItem::Primary;
			}

			// If the mouse is over the Secondary binding, select it
			if ((x >= Cc_lines[Selected_line].jx) && (x < Cc_lines[Selected_line].jx + Cc_lines[Selected_line].jw)) {
				Selected_item = selItem::Secondary;
			}

			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
		}

		if (List_buttons[i].double_clicked()) {
			control_config_do_bind();
		}
	}

	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0, GR_RESIZE_MENU);
	} 

	// highlight tab with conflict
	Ui_window.draw();
	for (i=z=0; i<NUM_TABS; i++) {
		if (Conflicts_tabs[i]) {
			CC_Buttons[gr_screen.res][i].button.draw_forced(4);
			z++;
		}
	}

	if (z) {
		// maybe switch from bright to normal
		if((Conflict_stamp == -1) || timestamp_elapsed(Conflict_stamp)){
			Conflict_bright = !Conflict_bright;

			Conflict_stamp = timestamp(CONFLICT_FLASH_TIME);
		}

		// set color and font
		font::set_font(font::FONT2);
		if(Conflict_bright){
			gr_set_color_fast(&Color_bright_red);
		} else {
			gr_set_color_fast(&Color_red);
		}

		// setup the conflict string
		char conflict_str[512] = "";
		strncpy(conflict_str, XSTR("Conflict!", 205), 511);
		int sw, sh;
		gr_get_string_size(&sw, &sh, conflict_str);

		x = Conflict_warning_coords[gr_screen.res][CONTROL_X_COORD] - (sw / 2);
		y = Conflict_warning_coords[gr_screen.res][CONTROL_Y_COORD];
		gr_printf_menu(x, y, "%s", conflict_str);

		font::set_font(font::FONT1);
	} else {
		// might as well always reset the conflict stamp
		Conflict_stamp = -1;
	}

	// Find if a tab button was pressed
	for (i=0; i<NUM_TABS; i++) {
		if (CC_Buttons[gr_screen.res][i].button.button_down()) {
			break;
		}
	}

	// No tab buttons down, show selected tab button as down
	if (i == NUM_TABS) {
		CC_Buttons[gr_screen.res][Tab].button.draw_forced(2);
	}

	if (Search_mode) {
		CC_Buttons[gr_screen.res][SEARCH_MODE].button.draw_forced(2);
	}

	if (Selected_line >= 0) {
		z = Cc_lines[Selected_line].cc_index;
		if (z & JOY_AXIS) {
			// Show inversion button as down, if the selected axis is inverted
			if (Invert_axis[z & ~JOY_AXIS]) {
				CC_Buttons[gr_screen.res][INVERT_AXIS].button.draw_forced(2);
			}

		} else {
			// Show modifier buttons as down, if the selected item uses them
			k = Control_config[z].get_btn(CID_KEYBOARD);
			if (k >= 0) {
				if (k & KEY_SHIFTED) {
					CC_Buttons[gr_screen.res][SHIFT_TOGGLE].button.draw_forced(2);
				}
				if (k & KEY_ALTED) {
					CC_Buttons[gr_screen.res][ALT_TOGGLE].button.draw_forced(2);
				}
			}
		}
	}

	if (Binding_mode) {
		CC_Buttons[gr_screen.res][BIND_BUTTON].button.draw_forced(2);
	}

	z = Cc_lines[Selected_line].cc_index;
	x = Conflict_wnd_coords[gr_screen.res][CONTROL_X_COORD] + Conflict_wnd_coords[gr_screen.res][CONTROL_W_COORD] / 2;
	y = Conflict_wnd_coords[gr_screen.res][CONTROL_Y_COORD] + Conflict_wnd_coords[gr_screen.res][CONTROL_H_COORD] / 2;
	if (Binding_mode) {
		int t;

		t = (int) (timer * 3);
		if (t % 2) {
			gr_set_color_fast(&Color_text_normal);
			gr_get_string_size(&w, NULL, XSTR( "?", 208));
			gr_printf_menu(x - w / 2, y - font_height / 2, "%s", XSTR( "?", 208));
		}

	} else if (!(z & JOY_AXIS) && ((Conflicts[z].first >= 0) || (Conflicts[z].second >= 0))) {
		i = Conflicts[z].first;
		if (i < 0) {
			i = Conflicts[z].second;
		}

		gr_set_color_fast(&Color_text_normal);
		str = XSTR( "Control conflicts with:", 209);
		gr_get_string_size(&w, NULL, str);
		gr_printf_menu(x - w / 2, y - font_height, "%s", str);

		if (Control_config[i].indexXSTR > 1) {
			strcpy_s(buf, XSTR(Control_config[i].text.c_str(), Control_config[i].indexXSTR, true));
		} else if (Control_config[i].indexXSTR == 1) {
			strcpy_s(buf, XSTR(Control_config[i].text.c_str(), CONTROL_CONFIG_XSTR + i, true));
		} else {
			strcpy_s(buf, Control_config[i].text.c_str());
		}

		font::force_fit_string(buf, 255, Conflict_wnd_coords[gr_screen.res][CONTROL_W_COORD]);
		gr_get_string_size(&w, NULL, buf);
		gr_printf_menu(x - w / 2, y, "%s", buf);

	} else if (*bound_string) {
		gr_set_color_fast(&Color_text_normal);
		gr_get_string_size(&w, NULL, bound_string);
		gr_printf_menu(x - w / 2, y - font_height / 2, "%s", bound_string);
		if (timestamp_elapsed(bound_timestamp)) {
			*bound_string = 0;
		}
	}

	if (Cc_lines[Num_cc_lines - 1].y + font_height > Cc_lines[Scroll_offset].y + Control_list_coords[gr_screen.res][CONTROL_H_COORD]) {
		gr_set_color_fast(&Color_white);
		gr_printf_menu(Control_more_coords[gr_screen.res][CONTROL_X_COORD], Control_more_coords[gr_screen.res][CONTROL_Y_COORD], "%s", XSTR( "More...", 210));
	}

	conflict = control_config_draw_list(select_tease_line);

	CC_Buttons[gr_screen.res][CLEAR_OTHER_BUTTON].button.enable(conflict);

	// Display preset in use
	control_config_draw_selected_preset();

	// blit help overlay if active
	help_overlay_maybe_blit(Control_config_overlay_id, gr_screen.res);

	gr_flip();
}

float check_control_timef(int id)
{
	float t1, t2;

	// if type isn't continuous, we shouldn't be using this function, cause it won't work.
	Assert(Control_config[id].type == CC_TYPE_CONTINUOUS);

	// first, see if control actually used (makes sure modifiers match as well)
	if (!check_control(id)) {
		Control_config[id].continuous_ongoing = false;

		return 0.0f;
	}

	t1 = key_down_timef(Control_config[id].get_btn(CID_KEYBOARD));
	if (t1) {
		control_used(id);
	}

	t2 = joy_down_time(Control_config[id].get_btn(CID_JOY0));
	if (t2) {
		control_used(id);
	}

	if (t1 + t2) {
		// We want to set this to true only after visiting control_used() (above)
		// to allow it to tell the difference between an ongoing continuous action
		// started before and a continuous action being started right now.
		Control_config[id].continuous_ongoing = true;

		return t1 + t2;
	}

	return 1.0f;
}

void control_check_indicate()
{
#ifndef NDEBUG
	if (Show_controls_info) {
		gr_set_color_fast(&HUD_color_debug);
		gr_printf_no_resize(gr_screen.center_offset_x + gr_screen.center_w - 154, gr_screen.center_offset_y + 5, NOX("Ctrls checked: %d"), Control_check_count);
	}
#endif

	Control_check_count = 0;
}

int check_control_used(int id, int key)
{
	int mask;
	static int last_key = 0;
	auto & item = Control_config[id];

	Control_check_count++;
	if (key < 0) {
		key = last_key;
	}

	last_key = key;

	// if we're in multiplayer text enter (for chat) mode, check to see if we should ignore controls
	if ((Game_mode & GM_MULTIPLAYER) && multi_ignore_controls()){
		return 0;
	}

	if (item.disabled)
		return 0;

	short btn = item.get_btn(CID_JOY0);
	short z = item.get_btn(CID_KEYBOARD);

	if (item.type == CC_TYPE_CONTINUOUS) {
		
		if (joy_down(btn) || joy_down_count(btn, 1)) {
			control_used(id);
			return 1;
		}

		if ((btn >= 0) && (btn < MOUSE_NUM_BUTTONS)) {
			if (mouse_down(1 << btn) || mouse_down_count(1 << btn)) {
				control_used(id);
				return 1;
			}
		}

		// check what current modifiers are pressed
		mask = 0;
		if (keyd_pressed[KEY_LSHIFT] || key_down_count(KEY_LSHIFT) || keyd_pressed[KEY_RSHIFT] || key_down_count(KEY_RSHIFT)) {
			mask |= KEY_SHIFTED;
		}

		if (keyd_pressed[KEY_LALT] || key_down_count(KEY_LALT) || keyd_pressed[KEY_RALT] || key_down_count(KEY_RALT)) {
			mask |= KEY_ALTED;
		}

		
		if (z >= 0) {
			if ( (z != KEY_LALT) && (z != KEY_RALT) && (z != KEY_LSHIFT) && (z != KEY_RSHIFT) ) {
				// if current modifiers don't match action's modifiers, don't register control active.
				if ((z & (KEY_SHIFTED | KEY_ALTED)) != mask) {
					return 0;
				}
			}

			z &= KEY_MASK;

			if (keyd_pressed[z] || key_down_count(z)) {
				control_used(id);
				return 1;
			}
		}

		return 0;
	}

	if ((z == key) || joy_down_count(btn, 1) ||
			((btn >= 0) && (btn < MOUSE_NUM_BUTTONS) && mouse_down_count(1 << btn))) {
		//mprintf(("Key used %d\n", key));
		control_used(id);
		return 1;
	}

	return 0;
}

int check_control(int id, int key) 
{
	if (check_control_used(id, key)) {
		if (Ignored_keys[id]) {
			if (Ignored_keys[id] > 0) {
				Ignored_keys[id]--;
			}
			return 0;
		}
		return 1;
	}

	if (Control_config[id].continuous_ongoing) {
		// If we reach this point, then it means this is a continuous control
		// which has just been released

		if (Script_system.IsActiveAction(CHA_ONACTIONSTOPPED)) {
			Script_system.SetHookVar("Action", 's', Control_config[id].text);
			Script_system.RunCondition(CHA_ONACTIONSTOPPED, nullptr, id);
			Script_system.RemHookVar("Action");
		}

		Control_config[id].continuous_ongoing = false;
	}

	return 0;
}

void control_get_axes_readings(int *h, int *p, int *b, int *ta, int *tr)
{
	int axes_values[JOY_NUM_AXES];

	joystick_read_raw_axis(JOY_NUM_AXES, axes_values);

	//	joy_get_scaled_reading will return a value represents the joystick pos from -1 to +1 (fixed point)
	*h = 0;
	if (Axis_map_to[0] >= 0) {
		*h = joy_get_scaled_reading(axes_values[Axis_map_to[0]]);
	}

	*p = 0;
	if (Axis_map_to[1] >= 0) {
		*p = joy_get_scaled_reading(axes_values[Axis_map_to[1]]);
	}

	*b = 0;
	if (Axis_map_to[2] >= 0) {
		*b = joy_get_scaled_reading(axes_values[Axis_map_to[2]]);
	}

	*ta = 0;
	if (Axis_map_to[3] >= 0) {
		*ta = joy_get_unscaled_reading(axes_values[Axis_map_to[3]]);
	}

	*tr = 0;
	if (Axis_map_to[4] >= 0) {
		*tr = joy_get_scaled_reading(axes_values[Axis_map_to[4]]);
	}

	if (Invert_axis[0]) {
		*h = -(*h);
	}
	if (Invert_axis[1]) {
		*p = -(*p);
	}
	if (Invert_axis[2]) {
		*b = -(*b);
	}
	if (Invert_axis[3]) {
		*ta = F1_0 - *ta;
	}
	if (Invert_axis[4]) {
		*tr = -(*tr);
	}

	return;
}

int Last_frame_timestamp;
void control_used(int id)
{
	// if we have set this key to be ignored, ignore it
	if (Ignored_keys[id]) {
		return;
	}

	// This check needs to be done because the control code might call this function more than once per frame,
	// and we don't want to run the hooks more than once per frame
	if (Control_config[id].used < Last_frame_timestamp) {
		if (!Control_config[id].continuous_ongoing) {
			Script_system.SetHookVar("Action", 's', Control_config[id].text);
			Script_system.RunCondition(CHA_ONACTION, nullptr, id);
			Script_system.RemHookVar("Action");

			if (Control_config[id].type == CC_TYPE_CONTINUOUS)
				Control_config[id].continuous_ongoing = true;
		}

		Control_config[id].used = timestamp();
	}
}

void control_config_clear_used_status()
{
	for (auto &item : Control_config) {
		item.used = 0;
	}
}

void control_config_clear()
{
	for (auto &item : Control_config) {
		item.clear();
	}
}

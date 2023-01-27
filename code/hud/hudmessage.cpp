/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "hud/hudmessage.h"

#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"

#include "freespace.h"

#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudconfig.h"
#include "iff_defs/iff_defs.h"
#include "io/key.h"
#include "io/timer.h"
#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h" // for MAX_MISSION_MESSAGES
#include "missionui/missionscreencommon.h"
#include "network/multi.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "scripting/hook_api.h"
#include "scripting/scripting.h"
#include "ship/ship.h"
#include "sound/audiostr.h"
#include "ui/ui.h"
#include "weapon/weapon.h"

#include <cstdarg>
#include <cstdlib>

/* replaced with those static ints that follow
#define LIST_X		46
#define LIST_X2	108  // second column x start position
#define LIST_Y		60
#define LIST_W		558  // total width including both columns
#define LIST_W2	(LIST_W + LIST_X - LIST_X2)  // width of second column
#define LIST_H		297
#define LIST_H_O	275  // applies only to objectives mode
*/

// 1st column, width includes both columns
static int Hud_mission_log_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		46,60,558,269		// GR_640
	},
	{
		74,96,558,297		// GR_1024
	}
};

// 2nd column, width is just of second column
static int Hud_mission_log_list2_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		108, 60, 496, 297		// GR_640
	},
	{
		136, 96, 496, 436		// GR_1024
	}
};

static int Hud_mission_log_list_objective_x_coord[GR_NUM_RESOLUTIONS] = {
	275,	// GR_640
	440	// GR_1024
};

static int Hud_mission_log_time_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		41, 372	// GR_640
	},
	{
		66, 595	// GR_1024
	}
};

static int Hud_mission_log_time2_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		103, 372	// GR_640
	},
	{
		128, 595	// GR_1024
	}
};


#define SCROLLBACK_MODE_MSGS_LOG		0
#define SCROLLBACK_MODE_EVENT_LOG	1
#define SCROLLBACK_MODE_OBJECTIVES	2

#define NUM_BUTTONS			6

#define SCROLL_UP_BUTTON	0
#define SCROLL_DOWN_BUTTON	1
#define SHOW_MSGS_BUTTON	2
#define SHOW_EVENTS_BUTTON	3
#define SHOW_OBJS_BUTTON	4
#define ACCEPT_BUTTON		5

#define HUD_MSG_LENGTH_MAX		2048
//#define HUD_MSG_MAX_PIXEL_W	439	// maximum number of pixels wide message display area is
//#define HUD_MSG_MAX_PIXEL_W	619	// maximum number of pixels wide message display area is

/* not used anymore
static int Hud_mission_log_status_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		170, 339		// GR_640
	},
	{
		361, 542		// GR_1024
	}
};
*/

struct scrollback_buttons {
	const char *filename;
	int x, y;
	int xt, yt;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	scrollback_buttons(const char *name, int x1, int y1, int x2, int y2, int h) : filename(name), x(x1), y(y1), xt(x2), yt(y2), hotspot(h) {}
};

SCP_vector<HUD_message_data> HUD_msg_buffer;

int HUD_msg_inited = FALSE;

SCP_vector<line_node> Msg_scrollback_vec;
static SCP_vector<line_node> Msg_scrollback_lines;

#define	MAX_HUD_FT	1

typedef struct HUD_ft {
	int	end_time;						//	Timestamp at which this message will go away.
	char	text[MAX_HUD_LINE_LEN];		//	Text to display.
	int	color;							//	0rgb color, 8 bit fields.
} HUD_ft;

HUD_ft HUD_fixed_text[MAX_HUD_FT];

static int Num_obj_lines;
static int Scroll_offset;
static int Scroll_max;
static int Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
// static int Status_bitmap;
static int Background_bitmap;
static UI_WINDOW Ui_window;

static const char* Hud_mission_log_fname[GR_NUM_RESOLUTIONS] = {
	"MissionLog",		// GR_640
	"2_MissionLog"		// GR_1024
};

/* not used anymore
static char* Hud_mission_log_status_fname[GR_NUM_RESOLUTIONS] = {
	"MLStatus",		// GR_640
	"MLStatus"		// GR_1024
};
*/

static const char* Hud_mission_log_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MissionLog-m",		// GR_640
	"2_MissionLog-m"		// GR_1024
};

static scrollback_buttons Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
	{	// GR_640
	//XSTR:OFF
		scrollback_buttons("LB_00",	1,		67,	-1,	-1,	0),
		scrollback_buttons("LB_01",	1,		307,	-1,	-1,	1),
		scrollback_buttons("LB_02",	111,	376,	108,	413,	2),
		scrollback_buttons("LB_03",	209,	376,	205,	413,	3),
		scrollback_buttons("LB_04",	12,	376,	7,		413,	4),
		scrollback_buttons("CB_05a",	571,	425,	564,	413,	5)
	//XSTR:ON
	},
	{	// GR_1024
	//XSTR:OFF
		scrollback_buttons("2_LB_00",	1,		108,	-1,	-1,	0),
		scrollback_buttons("2_LB_01",	1,		492,	-1,	-1,	1),
		scrollback_buttons("2_LB_02",	177,	602,	173,	661,	2),
		scrollback_buttons("2_LB_03",	335,	602,	335,	661,	3),
		scrollback_buttons("2_LB_04",	20,	602,	11,	661,	4),
		scrollback_buttons("2_CB_05a",914,	681,	946,	661,	5)
	//XSTR:ON
	}
};

const auto OnHudMessageReceivedHook = scripting::Hook<>::Factory(
	"On HUD Message Received",
	"Called when a HUD message is received. For normal messages this will be called with the final text that appears "
	"on the HUD (e.g. [ship]: [message]). Will also be called for engine generated messages.",
	{
		{"Text", "string", "The text of the message."},
		{"SourceType", "number", "The type of message sent by the engine."},
	});

// ----------------------------------------------------------------------
// HUD_init_fixed_text()
//
void HUD_init_fixed_text()
{
	int	i;

	for (i=0; i<MAX_HUD_FT; i++)
		HUD_fixed_text[i].end_time = timestamp(0);
}

// ----------------------------------------------------------------------
// hud_init_msg_window()
//
// Called from HUD_init(), which is called from game_level_init()
//
void hud_init_msg_window()
{
	Msg_scrollback_vec.clear();

	HUD_init_fixed_text();
	HUD_msg_inited = TRUE;
}

void hud_clear_msg_buffer()
{
	HUD_msg_buffer.clear();
}

HudGaugeMessages::HudGaugeMessages():
HudGauge(HUD_OBJECT_MESSAGES, HUD_MESSAGE_LINES, false, true, (VM_WARP_CHASE), 255, 255, 255)
{
}

void HudGaugeMessages::initMaxLines(int lines)
{
	Max_lines = lines + 1; // One additional line that's not displayed to scroll offscreen.
}

void HudGaugeMessages::initMaxWidth(int width)
{
	Window_width = Max_width = width;
}

void HudGaugeMessages::initScrollTime(int ms)
{
	Scroll_time = ms;
}

void HudGaugeMessages::initStepSize(int h)
{
	Step_size = h;
}

void HudGaugeMessages::initTotalLife(int ms)
{
	Total_life = ms;
}

void HudGaugeMessages::initLineHeight(int h)
{
	Line_h = h;
}

void HudGaugeMessages::initHiddenByCommsMenu(bool hide)
{
	Hidden_by_comms_menu = hide;
}

void HudGaugeMessages::initialize()
{
	// calculate the window height based on the number of lines, and line height
	Window_height = Max_lines * Line_h;

	active_messages.clear();
	pending_messages = SCP_queue<HUD_message_data>(); // there's no clear() method for queues :/

	Scroll_needed = false;
	Scroll_in_progress = false;
	Scroll_time_id = 1;

	HudGauge::initialize();
}

void HudGaugeMessages::pageIn()
{
}

void HudGaugeMessages::processMessageBuffer()
{
	int x, offset = 0;
	size_t i;
	char *msg;
	char *split_str, *ptr;

	for ( i = 0; i < HUD_msg_buffer.size(); i++ ) {
		msg = new char [HUD_msg_buffer[i].text.size()+1];
		strcpy(msg, HUD_msg_buffer[i].text.c_str());

		ptr = strstr(msg, NOX(": "));
		if ( ptr ) {
			int sw;
			gr_get_string_size(&sw, nullptr, msg, (int)(ptr + 2 - msg));
			offset = sw;
		}

		x = 0;
		split_str = msg;

		while ((ptr = split_str_once(split_str, Max_width - x - 7)) != nullptr) {		// the 7 is a fudge hack
			// make sure that split went ok, if not then bail
			if (ptr == split_str) {
				break;
			}

			addPending(split_str, HUD_msg_buffer[i].source, x);
			split_str = ptr;
			x = offset;
		}

		addPending(split_str, HUD_msg_buffer[i].source, x);

		delete[] msg;
	}
}

void HudGaugeMessages::addPending(const char *text, int source, int x)
{
	Assert(text != NULL);

	HUD_message_data new_message;

	new_message.text = text;
	new_message.source = source;
	new_message.x = x;

	pending_messages.push(new_message);
}

void HudGaugeMessages::scrollMessages()
{
	// check if there is a message to display on the HUD, and if there is room to display it
	if ( !pending_messages.empty() && !Scroll_needed ) {
		Hud_display_info new_active_msg;

		new_active_msg.msg = pending_messages.front();
		new_active_msg.total_life = timestamp(Total_life);

		pending_messages.pop();

		for ( SCP_vector<Hud_display_info>::iterator m = active_messages.begin(); m != active_messages.end(); ++m ) {
			// determine if there are any existing messages, if so need to scroll them up
			if ( !timestamp_elapsed(m->total_life) ) {
				m->target_y -= Line_h;
				Scroll_needed = true;
			}
		}

		if (Scroll_needed) {
			new_active_msg.y = (Max_lines-1)*Line_h;
			new_active_msg.target_y = new_active_msg.y - Line_h;
		} else {
			new_active_msg.y = (Max_lines-2)*Line_h;
			new_active_msg.target_y = new_active_msg.y;
		}

		active_messages.push_back(new_active_msg);
	}

	Scroll_in_progress = false;
	Scroll_needed = false;

	for ( SCP_vector<Hud_display_info>::iterator m = active_messages.begin(); m != active_messages.end(); ) {
		if ( !timestamp_elapsed(m->total_life) ) {
			if ( m->y > m->target_y ) {
				Scroll_needed = true;

				if ( timestamp_elapsed(Scroll_time_id) ) {
					m->y -= Step_size;

					if ( m->y < m->target_y ) {
						m->y = m->target_y;
					}

					Scroll_in_progress = true;
				}
			}
		} else {
			bool at_end = m == (active_messages.end() - 1);

			if (at_end) {
				// Iterator will be invalid
				active_messages.pop_back();
				break;
			}

			*m = active_messages.back();
			active_messages.pop_back();

			if (active_messages.empty())
			{
				// We may not use the iterator any longer
				break;
			}
			else
			{
				continue;
			}
		}

		++m;
	}

	if (Scroll_in_progress) {
		Scroll_time_id = timestamp(Scroll_time);
	}
}

void HudGaugeMessages::clearMessages()
{
	active_messages.clear();
}

void HudGaugeMessages::preprocess()
{
	setFont();
	processMessageBuffer();
	scrollMessages();
}

/**
 * HudGaugeMessages::render() will display the active HUD messages on the HUD.  It will scroll
 * the messages up when a new message arrives.
 */
void HudGaugeMessages::render(float  /*frametime*/)
{
	hud_set_default_color();

	// dependant on max_width, max_lines, and line_height
	setClip(position[0], position[1], Window_width, Window_height+2);

	for ( SCP_vector<Hud_display_info>::iterator m = active_messages.begin(); m != active_messages.end(); ++m) {
		if ( !timestamp_elapsed(m->total_life) ) {
			if ( !(Player->flags & PLAYER_FLAGS_MSG_MODE) || !Hidden_by_comms_menu) {
				// set the appropriate color					
				if ( m->msg.source ) {
					setGaugeColor(HUD_C_BRIGHT);
				} else {
					setGaugeColor();
				}

				// print the message out
				renderPrintf(m->msg.x, m->y, "%s", m->msg.text.c_str());
			}
		}
	}
}

//	Similar to HUD printf, but shows only one message at a time, at a fixed location.
void HUD_fixed_printf(float duration, color col, const char *format, ...)
{
	va_list	args;
	char		tmp[HUD_MSG_LENGTH_MAX];
	size_t		msg_length;

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state != NETGAME_STATE_IN_MISSION)){
		nprintf(("Network","HUD_fixed_printf bailing because not in multiplayer game play state\n"));
		return;
	}

	va_start(args, format);
	vsnprintf(tmp, sizeof(tmp)-1, format, args);
	va_end(args);
	tmp[sizeof(tmp)-1] = '\0';

	msg_length = strlen(tmp);

	if ( !msg_length ) {
		nprintf(("Warning", "HUD_fixed_printf ==> attempt to print a 0 length string in msg window\n"));
		return;

	} else if (msg_length > MAX_HUD_LINE_LEN - 1){
		nprintf(("Warning", "HUD_fixed_printf ==> Following string truncated to %d chars: %s\n", MAX_HUD_LINE_LEN - 1, tmp));
		tmp[MAX_HUD_LINE_LEN-1] = '\0';
	}

	strcpy_s(HUD_fixed_text[0].text, tmp);

	if (duration == 0.0f){
		HUD_fixed_text[0].end_time = timestamp(-1);
	} else {
		HUD_fixed_text[0].end_time = timestamp((int) (1000.0f * duration));
	}
	HUD_fixed_text[0].color = col.red << 16 | col.green << 8 | col.blue; 
}

//	Clear all pending text.
void HUD_fixed_printf_reset()
{
	HUD_init_fixed_text();
}

// converts a TEAM_* define to a HUD_SOURCE_* define
int HUD_team_get_source(int team)
{
	return team + HUD_SOURCE_TEAM_OFFSET;
}

// converts a HUD_SOURCE_* define to a TEAM_* define
int HUD_source_get_team(int source)
{
	return source - HUD_SOURCE_TEAM_OFFSET;
}


void HUD_printf(const char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->state != NETPLAYER_STATE_IN_MISSION)){
		nprintf(("Network","HUD_printf bailing because not in multiplayer game play state\n"));
		return;
	}

	va_start(args, format);
	vsnprintf(tmp, sizeof(tmp)-1, format, args);
	va_end(args);
	tmp[sizeof(tmp)-1] = '\0';

	hud_sourced_print(HUD_SOURCE_COMPUTER, tmp);
}

// --------------------------------------------------------------------------------------
// HUD_sourced_printf() 
//
// HUD_sourced_printf() has the same parameters as printf(), but displays the text as a scrolling
// message on the HUD.  Text is split into multiple lines if width exceeds msg display area
// width.  'source' is used to indicate who send the message, and is used to color code text.
//
void HUD_sourced_printf(int source, const char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->state != NETPLAYER_STATE_IN_MISSION)){
		nprintf(("Network","HUD_sourced_printf bailing because not in multiplayer game play state\n"));
		return;
	}
	
	va_start(args, format);
	vsnprintf(tmp, sizeof(tmp)-1, format, args);
	va_end(args);
	tmp[sizeof(tmp)-1] = '\0';

	hud_sourced_print(source, tmp);
}

void hud_sourced_print(int source, const char *msg)
{
	if ( !strlen(msg) ) {
		nprintf(("Warning", "HUD ==> attempt to print a 0 length string in msg window\n"));
		return;
	}

	// add message to the scrollback log first
	hud_add_msg_to_scrollback(msg, source, Missiontime);

	HUD_message_data new_msg;

	new_msg.text = SCP_string(msg);
	new_msg.source = source;
	new_msg.x = 0;

	HUD_msg_buffer.push_back(new_msg);

	// Invoke the scripting hook
	if (OnHudMessageReceivedHook->isActive()) {
		OnHudMessageReceivedHook->run(scripting::hook_param_list(scripting::hook_param("Text", 's', msg),
			scripting::hook_param("SourceType", 'i', source)));
	}
}

int hud_query_scrollback_size()
{
	int count = 0, y_add = 0;
	int font_height = gr_get_font_height();

	if (Msg_scrollback_lines.size() <= 1 || !HUD_msg_inited)
		return 0;

	for (int i = 0; i < (int)Msg_scrollback_lines.size(); i++) {
		if (Msg_scrollback_lines[i].source != HUD_SOURCE_HIDDEN) {
			y_add = Msg_scrollback_lines[i].y;
			count += font_height + Msg_scrollback_lines[i].y;
		}
	}

	count -= y_add;
	return count;
}

// add text directly to the hud scrollback log, without displaying on the hud
void HUD_add_to_scrollback(const char *text, int source)
{
	if (!strlen(text)) {
		nprintf(("Warning", "HUD ==> attempt to print a 0 length string in msg window\n"));
		return;
	}

	hud_add_msg_to_scrollback(text, source, Missiontime);
}

void hud_add_msg_to_scrollback(const char *text, int source, int t)
{
	size_t msg_len = strlen(text);
	if (msg_len == 0)
		return;
	
	Assert(msg_len < HUD_MSG_LENGTH_MAX);

	char buf[HUD_MSG_LENGTH_MAX], *ptr;
	strcpy_s(buf, text);
	ptr = strstr(buf, NOX(": "));

	int w = 0;

	// determine the length of the sender's name for underlining
	if (ptr) {
		gr_get_string_size(&w, nullptr, buf, (int)(ptr - buf));
	}

	// create the new node for the vector
	line_node newLine = {t, source, 0, 1, w, ""};
	newLine.text = text;

	Msg_scrollback_vec.push_back(newLine);
}

// how many lines to skip
int hud_get_scroll_max_pos()
{
	int max = 0, font_height = gr_get_font_height();

	if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
		int count = 0;
		// number of pixels in excess of what can be displayed
		int excess = Scroll_max - Hud_mission_log_list_coords[gr_screen.res][3];

		if (Msg_scrollback_lines.size() <= 1 || !HUD_msg_inited) {
			max = 0;
		} else {

			for (int i = 0; i < (int)Msg_scrollback_lines.size(); i++) {
				if (Msg_scrollback_lines[i].source != HUD_SOURCE_HIDDEN) {

					if (excess > 0) {
						excess -= font_height;
						count++;
					}
					if (excess <= 0) {
						max = count;
						break;
					}

					//spacing between lines 
					excess -= Msg_scrollback_lines[i].y;
				}
			}
		}

	} else {
		max = (Scroll_max - Hud_mission_log_list_coords[gr_screen.res][3]) / font_height;
	}

	if (max < 0)
		max = 0;

	return max;
}

void hud_scroll_reset()
{
	if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
		Scroll_offset = 0;

	} else {
		Scroll_offset = hud_get_scroll_max_pos();
	}
}

void hud_scroll_list(int dir)
{
	if (dir) {
		if (Scroll_offset) {
			Scroll_offset--;
			gamesnd_play_iface(InterfaceSounds::SCROLL);

		} else
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);

	} else {
		if (Scroll_offset < hud_get_scroll_max_pos()) {
			Scroll_offset++;
			gamesnd_play_iface(InterfaceSounds::SCROLL);

		} else
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void hud_goto_pos(int delta)
{
	int pos=0, font_height = gr_get_font_height();

	if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
		int count = 0, y = 0;

		if (Msg_scrollback_lines.size() <= 1 || !HUD_msg_inited)
			return;

		for (int i = 0; i < (int)Msg_scrollback_lines.size(); i++) {
			if (Msg_scrollback_lines[i].source != HUD_SOURCE_HIDDEN) {
				if (count == Scroll_offset) {
					pos = y;
					break;
				}

				y += font_height + Msg_scrollback_lines[i].y;
				count++;
			}
		}

		Scroll_offset = count = y = 0;

		for (int i = 0; i < (int)Msg_scrollback_lines.size(); i++) {
			if (Msg_scrollback_lines[i].source != HUD_SOURCE_HIDDEN) {

				if (y <= pos + delta)
					Scroll_offset = count;

				y += font_height + Msg_scrollback_lines[i].y;
				count++;
			}
		}

	} else {
		pos = Scroll_offset * font_height;
		pos += delta;
		Scroll_offset = pos / font_height;
	}
}

void hud_page_scroll_list(int dir)
{
	int max = hud_get_scroll_max_pos();

	if (dir) {
		if (Scroll_offset) {
			hud_goto_pos(-Hud_mission_log_list_coords[gr_screen.res][3]);
			if (Scroll_offset < 0)
				Scroll_offset = 0;

			gamesnd_play_iface(InterfaceSounds::SCROLL);

		} else
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);

	} else {
		if (Scroll_offset < max) {
			hud_goto_pos(Hud_mission_log_list_coords[gr_screen.res][3]);
			if (Scroll_offset > max)
				Scroll_offset = max;

			gamesnd_play_iface(InterfaceSounds::SCROLL);

		} else
			gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void hud_scrollback_button_pressed(int n)
{
	switch (n) {
		case SCROLL_UP_BUTTON:
			hud_scroll_list(1);
			break;

		case SCROLL_DOWN_BUTTON:
			hud_scroll_list(0);
			break;

		case SHOW_MSGS_BUTTON:
			Scrollback_mode = SCROLLBACK_MODE_MSGS_LOG;
			Scroll_max = hud_query_scrollback_size();
			hud_scroll_reset();
			break;

		case SHOW_EVENTS_BUTTON:
			Scrollback_mode = SCROLLBACK_MODE_EVENT_LOG;
			Scroll_max = Num_log_lines * gr_get_font_height();
			hud_scroll_reset();
			break;

		case SHOW_OBJS_BUTTON:
			Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
			Scroll_max = Num_obj_lines * gr_get_font_height();
			Scroll_offset = 0;
			break;

		case ACCEPT_BUTTON:
			hud_scrollback_exit();			
			break;
	}
}

// breaks up Msg_scrollback_vec into individual lines that can be rendered
// to the screen one at a time. Also helps with figuring out the actual
// scroll height of the scrollback UI.
void hud_initialize_scrollback_lines()
{

	Msg_scrollback_lines.clear();

	if ((Msg_scrollback_vec.size() > 0) && HUD_msg_inited) {

		for (int j = 0; j < (int)Msg_scrollback_vec.size(); j++) {
			line_node node_msg = Msg_scrollback_vec[j];

			int width = 0;
			int height = 0;
			gr_get_string_size(&width, &height, node_msg.text.c_str(), (int)node_msg.text.length());

			int max_width = Hud_mission_log_list2_coords[gr_screen.res][2];
			if (width > max_width) {
				char c_text[HUD_MSG_LENGTH_MAX];
				strcpy_s(c_text, node_msg.text.c_str());

				char* text = c_text;
				char* split = nullptr;

				split = split_str_once(text, max_width);
				Msg_scrollback_lines.push_back({node_msg.time, node_msg.source, node_msg.x, 1, node_msg.underline_width, text});

				while (split != nullptr) {
					text = split;
					split = nullptr;
					split = split_str_once(text, max_width);

					int offset = 1;
					if (split == nullptr)
						offset = height / 3;

					Msg_scrollback_lines.push_back({0, node_msg.source, node_msg.x, offset, 0, text});
				}
			} else {
				node_msg.y = height / 3;
				Msg_scrollback_lines.push_back(node_msg);
			}
		}
	}
}

void hud_scrollback_init()
{

	// pause all game sounds
	weapon_pause_sounds();
	audiostream_pause_all();

	hud_initialize_scrollback_lines();

	common_set_interface_palette("BriefingPalette");  // set the interface palette
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Hud_mission_log_mask_fname[gr_screen.res]);

	for (int i=0; i<NUM_BUTTONS; i++) {
		scrollback_buttons* b;

		b = &Buttons[gr_screen.res][i];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, (i < 2), 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add all strings	
	Ui_window.add_XSTR("Continue", 1069, Buttons[gr_screen.res][ACCEPT_BUTTON].xt,  Buttons[gr_screen.res][ACCEPT_BUTTON].yt, &Buttons[gr_screen.res][ACCEPT_BUTTON].button, UI_XSTR_COLOR_PINK);
	Ui_window.add_XSTR("Events", 1070, Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].xt,  Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].yt, &Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Objectives", 1071, Buttons[gr_screen.res][SHOW_OBJS_BUTTON].xt,  Buttons[gr_screen.res][SHOW_OBJS_BUTTON].yt, &Buttons[gr_screen.res][SHOW_OBJS_BUTTON].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Messages", 1072, Buttons[gr_screen.res][SHOW_MSGS_BUTTON].xt,  Buttons[gr_screen.res][SHOW_MSGS_BUTTON].yt, &Buttons[gr_screen.res][SHOW_MSGS_BUTTON].button, UI_XSTR_COLOR_GREEN);

	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	Buttons[gr_screen.res][SCROLL_UP_BUTTON].button.set_hotkey(KEY_UP);
	Buttons[gr_screen.res][SCROLL_DOWN_BUTTON].button.set_hotkey(KEY_DOWN);

	Background_bitmap = bm_load(Hud_mission_log_fname[gr_screen.res]);
	// Status_bitmap = bm_load(Hud_mission_log_status_fname[gr_screen.res]);

	message_log_init_scrollback(Hud_mission_log_list_coords[gr_screen.res][2]);
	if (Scrollback_mode == SCROLLBACK_MODE_EVENT_LOG)
		Scroll_max = Num_log_lines * gr_get_font_height();
	else if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES)
		Scroll_max = Num_obj_lines * gr_get_font_height();
	else
		Scroll_max = hud_query_scrollback_size();

	Num_obj_lines = ML_objectives_init(Hud_mission_log_list_coords[gr_screen.res][0], Hud_mission_log_list_coords[gr_screen.res][1], Hud_mission_log_list_coords[gr_screen.res][2], Hud_mission_log_list_objective_x_coord[gr_screen.res]);
	hud_scroll_reset();
}

void hud_scrollback_close()
{
	ML_objectives_close();
	message_log_shutdown_scrollback();
	if (Background_bitmap >= 0)
		bm_release(Background_bitmap);
	//if (Status_bitmap >= 0)
	//	bm_unload(Status_bitmap);

	Ui_window.destroy();
	common_free_interface_palette();		// restore game palette
	game_flush();

	// unpause all game sounds
	weapon_unpause_sounds();
	audiostream_unpause_all();

}

void hud_scrollback_do_frame(float  /*frametime*/)
{
	int font_height = gr_get_font_height();

	int k = Ui_window.process();
	switch (k) {
		case KEY_RIGHT:
		case KEY_TAB:
			if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
				Scrollback_mode = SCROLLBACK_MODE_MSGS_LOG;
				Scroll_max = hud_query_scrollback_size();
				hud_scroll_reset();

			} else if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
				Scrollback_mode = SCROLLBACK_MODE_EVENT_LOG;
				Scroll_max = Num_log_lines * gr_get_font_height();
				hud_scroll_reset();

			} else {
				Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
				Scroll_max = Num_obj_lines * gr_get_font_height();
				Scroll_offset = 0;
			}

			break;

		case KEY_LEFT:
		case KEY_SHIFTED | KEY_TAB:
			if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
				Scrollback_mode = SCROLLBACK_MODE_EVENT_LOG;
				Scroll_max = Num_log_lines * gr_get_font_height();
				hud_scroll_reset();

			} else if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
				Scrollback_mode = SCROLLBACK_MODE_OBJECTIVES;
				Scroll_max = Num_obj_lines * gr_get_font_height();
				Scroll_offset = 0;

			} else {
				Scrollback_mode = SCROLLBACK_MODE_MSGS_LOG;
				Scroll_max = hud_query_scrollback_size();
				hud_scroll_reset();
			}

			break;

		case KEY_PAGEUP:
			hud_page_scroll_list(1);
			break;

		case KEY_PAGEDOWN:
			hud_page_scroll_list(0);
			break;

		case KEY_ENTER:
		case KEY_CTRLED | KEY_ENTER:
		case KEY_ESC:			
			hud_scrollback_exit();
			break;

		case KEY_F1:  // show help overlay
			break;

		case KEY_F2:  // goto options screen
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);
			break;
	}	// end switch

	for (int i=0; i<NUM_BUTTONS; i++){
		if (Buttons[gr_screen.res][i].button.pressed()){
			hud_scrollback_button_pressed(i);		
		}
	}

	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0, GR_RESIZE_MENU);
	}

	/*
	if ((Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) && (Status_bitmap >= 0)) {
		gr_set_bitmap(Status_bitmap);
		gr_bitmap(Hud_mission_log_status_coords[gr_screen.res][0], Hud_mission_log_status_coords[gr_screen.res][1], GR_RESIZE_MENU);
	}
	*/

	// draw the objectives key at the bottom of the ingame objectives screen
	if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
		ML_render_objectives_key();
	}

	Ui_window.draw();

	if (Scrollback_mode == SCROLLBACK_MODE_EVENT_LOG) {
		Buttons[gr_screen.res][SHOW_EVENTS_BUTTON].button.draw_forced(2);
		mission_log_scrollback(Scroll_offset, Hud_mission_log_list_coords[gr_screen.res][0], Hud_mission_log_list_coords[gr_screen.res][1], Hud_mission_log_list_coords[gr_screen.res][2], Hud_mission_log_list_coords[gr_screen.res][3]);

	} else if (Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) {
		Buttons[gr_screen.res][SHOW_OBJS_BUTTON].button.draw_forced(2);
		ML_objectives_do_frame(Scroll_offset);

	} else {

		Buttons[gr_screen.res][SHOW_MSGS_BUTTON].button.draw_forced(2);

		int y = 0;
		if (!(Msg_scrollback_lines.size() <= 1) && HUD_msg_inited) {
			int i = 0;
			for (int j = 0; j < (int)Msg_scrollback_lines.size(); j++) {
				line_node node_msg = Msg_scrollback_lines[j];
				if ((node_msg.source == HUD_SOURCE_HIDDEN) || (i++ < Scroll_offset)) {
					continue;

				} else {
					int team = HUD_source_get_team(node_msg.source);

					if (team >= 0) {
						gr_set_color_fast(iff_get_color_by_team(team, Player_ship->team, 0));
					} else {
						switch (node_msg.source) {
						case HUD_SOURCE_TRAINING:
							gr_set_color_fast(&Color_bright_blue);
							break;

						case HUD_SOURCE_TERRAN_CMD:
							gr_set_color_fast(&Color_bright_white);
							break;

						case HUD_SOURCE_IMPORTANT:
						case HUD_SOURCE_FAILED:
						case HUD_SOURCE_SATISFIED:
							gr_set_color_fast(&Color_bright_white);
							break;

						default:
							gr_set_color_fast(&Color_text_normal);
							break;
						}
					}

					if (node_msg.time)
						gr_print_timestamp(Hud_mission_log_list_coords[gr_screen.res][0], Hud_mission_log_list_coords[gr_screen.res][1] + y, node_msg.time, GR_RESIZE_MENU);

					int x = Hud_mission_log_list2_coords[gr_screen.res][0] + node_msg.x;
					
					gr_printf_menu(x, Hud_mission_log_list_coords[gr_screen.res][1] + y, "%s", node_msg.text.c_str());
					if (node_msg.underline_width)
						gr_line(x, Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height - 1, x + node_msg.underline_width, Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height - 1, GR_RESIZE_MENU);

					if ((node_msg.source == HUD_SOURCE_FAILED) || (node_msg.source == HUD_SOURCE_SATISFIED)) {
						// draw goal icon
						if (node_msg.source == HUD_SOURCE_FAILED)
							gr_set_color_fast(&Color_bright_red);
						else
							gr_set_color_fast(&Color_bright_green);

						i = Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height / 2 - 1;
						gr_circle(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i, 5, GR_RESIZE_MENU);

						gr_set_color_fast(&Color_bright);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 10, i, Hud_mission_log_list2_coords[gr_screen.res][0] - 8, i, GR_RESIZE_MENU);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i - 4, Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i - 2, GR_RESIZE_MENU);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 4, i, Hud_mission_log_list2_coords[gr_screen.res][0] - 2, i, GR_RESIZE_MENU);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i + 2, Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i + 4, GR_RESIZE_MENU);
					}

					y += font_height + node_msg.y;

					if (y + font_height > Hud_mission_log_list_coords[gr_screen.res][3])
						break;

				}
			}
		}
	}

	gr_set_color_fast(&Color_text_heading);
	gr_print_timestamp(Hud_mission_log_time_coords[gr_screen.res][0], Hud_mission_log_time_coords[gr_screen.res][1] - font_height, Missiontime, GR_RESIZE_MENU);
	gr_string(Hud_mission_log_time2_coords[gr_screen.res][0], Hud_mission_log_time_coords[gr_screen.res][1] - font_height, XSTR( "Current time", 289), GR_RESIZE_MENU);
	gr_flip();
}

void hud_scrollback_exit()
{
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

HudGaugeTalkingHead::HudGaugeTalkingHead():
HudGauge(HUD_OBJECT_TALKING_HEAD, HUD_TALKING_HEAD, false, true, (VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY), 255, 255, 255)
{
}

void HudGaugeTalkingHead::initialize()
{
	head_anim = NULL;
	msg_id = -1;

	HudGauge::initialize();
}

void HudGaugeTalkingHead::initHeaderOffsets(int x, int y)
{
	Header_offsets[0] = x;
	Header_offsets[1] = y;
}

void HudGaugeTalkingHead::initAnimOffsets(int x, int y)
{
	Anim_offsets[0] = x;
	Anim_offsets[1] = y;
}

void HudGaugeTalkingHead::initAnimSizes(int w, int h)
{
	Anim_size[0] = w;
	Anim_size[1] = h;
}

void HudGaugeTalkingHead::initBitmaps(const char *fname)
{
	Head_frame.first_frame = bm_load_animation(fname, &Head_frame.num_frames);
	if ( Head_frame.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in ani: %s\n", fname);
	}
}

/**
 * Renders everything for a head animation
 * Also checks for when new head ani's need to start playing
 */
void HudGaugeTalkingHead::render(float frametime)
{
	if ( Head_frame.first_frame == -1 ){
		return;
	}

	if(msg_id != -1 && head_anim != NULL) {
		if(!head_anim->done_playing) {
			// draw frame
			// hud_set_default_color();
			setGaugeColor();

			// clear
			setClip(position[0] + Anim_offsets[0], position[1] + Anim_offsets[1], Anim_size[0], Anim_size[1]);
			gr_clear();
			resetClip();

			renderBitmap(Head_frame.first_frame, position[0], position[1]);		// head ani border
			float scale_x = i2fl(Anim_size[0]) / i2fl(head_anim->width);
			float scale_y = i2fl(Anim_size[1]) / i2fl(head_anim->height);
			gr_set_screen_scale(fl2ir(base_w / scale_x), fl2ir(base_h / scale_y));
			setGaugeColor();
			generic_anim_render(head_anim,frametime, fl2ir((position[0] + Anim_offsets[0] + HUD_offset_x) / scale_x), fl2ir((position[1] + Anim_offsets[1] + HUD_offset_y) / scale_y));
			// draw title
			gr_set_screen_scale(base_w, base_h);
			renderString(position[0] + Header_offsets[0], position[1] + Header_offsets[1], XSTR("message", 217));
		} else {
			for (int j = 0; j < Num_messages_playing; ++j) {
				if (Playing_messages[j].id == msg_id) {
					Playing_messages[j].play_anim = false;
					break;  // only one head ani plays at a time
				}
			}
			msg_id = -1;    // allow repeated messages to display a new head ani
			head_anim = NULL; // Nothing to see here anymore, move along
		}
	}
	// check playing messages to see if we have any messages with talking animations that need to be created.
	for (int i = 0; i < Num_messages_playing; i++ ) {
		if(Playing_messages[i].play_anim && Playing_messages[i].id != msg_id ) {
			msg_id = Playing_messages[i].id;
			if (Playing_messages[i].anim_data)
				head_anim = Playing_messages[i].anim_data;	
			else
				head_anim = NULL;

			return;
		}
	}
}

void HudGaugeTalkingHead::pageIn()
{
	bm_page_in_aabitmap( Head_frame.first_frame, Head_frame.num_frames );
}

bool HudGaugeTalkingHead::canRender()
{
	if (sexp_override) {
		return false;
	}

	if (hud_disabled() && !hud_disabled_except_messages()) {
		return false;
	}

	if(!active)
		return false;
	
	if ( !(Game_detail_flags & DETAIL_FLAG_HUD) ) {
		return false;
	}

	if ((Viewer_mode & disabled_views)) {
		return false;
	}

	if(pop_up) {
		if(!popUpActive()) {
			return false;
		}
	}

	return true;
}

HudGaugeFixedMessages::HudGaugeFixedMessages()
	: HudGauge(HUD_OBJECT_FIXED_MESSAGES, HUD_MESSAGE_LINES, false, true, (VM_WARP_CHASE), 255, 255, 255)
	, center_text(true)
{
}

void HudGaugeFixedMessages::initCenterText(bool center) {
	center_text = center;
}

void HudGaugeFixedMessages::render(float  /*frametime*/) {
	HUD_ft	*hp;

	hp = &HUD_fixed_text[0];

	if (!timestamp_elapsed(hp->end_time)) {
		gr_set_color((hp->color >> 16) & 0xff, (hp->color >> 8) & 0xff, hp->color & 0xff);
		
		if (center_text) {
			int w = 0;
			gr_get_string_size(&w, nullptr, hp->text);
			renderString(position[0] - (w / 2), position[1], hp->text);
		} else {
			renderString(position[0], position[1], hp->text);
		}
		//renderString(0x8000, MSG_WINDOW_Y_START + MSG_WINDOW_HEIGHT + 8, hp->text);
	}
}

void HudGaugeFixedMessages::pageIn()
{
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <stdlib.h>
#include <stdarg.h>


#include "hud/hud.h"
#include "hud/hudmessage.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "io/timer.h"
#include "playerman/player.h"
#include "globalincs/linklist.h"
#include "mission/missionlog.h"
#include "ui/ui.h"
#include "missionui/missionscreencommon.h"
#include "graphics/font.h"
#include "gamesnd/gamesnd.h"
#include "mission/missiongoals.h"
#include "globalincs/alphacolors.h"
#include "weapon/beam.h"
#include "sound/audiostr.h"
#include "ship/ship.h"
#include "parse/parselo.h"
#include "mission/missionmessage.h"		// for MAX_MISSION_MESSAGES
#include "iff_defs/iff_defs.h"
#include "network/multi.h"




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

#define HUD_MESSAGE_TOTAL_LIFE	14000	// total time a HUD message is alive (in milliseconds)

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
	char *filename;
	int x, y;
	int xt, yt;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	scrollback_buttons(char *name, int x1, int y1, int x2, int y2, int h) : filename(name), x(x1), y(y1), xt(x2), yt(y2), hotspot(h) {}
};

int Scroll_time_id;

int MSG_WINDOW_X_START = 5;
int MSG_WINDOW_Y_START = 5;
int MSG_WINDOW_WIDTH;		// initialed in hud_init_msg_window()
int MSG_WINDOW_HEIGHT;
int MSG_WINDOW_FONT_HEIGHT;
int ACTIVE_BUFFER_LINES	= 4;				// number of HUD messages that can be shown at one time + 1
int OLD_ACTIVE_BUFFER_LINES;

int Hud_list_start;			// points to the next msg to be printed in the queue
int Hud_list_end;				// points to the last msg in the queue

int Active_index=0;
int Scroll_needed=0;
int Scroll_in_progress=0;

HUD_message_data HUD_pending[SCROLL_BUFFER_LINES];
Hud_display_info HUD_active_msgs_list[MAX_ACTIVE_BUFFER_LINES];

int HUD_msg_inited = FALSE;

// There is a maximum number of lines that will be stored in the message scrollback.  Oldest
// messages are deleted to make way for newest messages.
#define MAX_MSG_SCROLLBACK_LINES	1000
line_node Msg_scrollback_lines[MAX_MSG_SCROLLBACK_LINES];

line_node Msg_scrollback_free_list;
line_node Msg_scrollback_used_list;

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

static char* Hud_mission_log_fname[GR_NUM_RESOLUTIONS] = {
	"MissionLog",		// GR_640
	"2_MissionLog"		// GR_1024
};

/* not used anymore
static char* Hud_mission_log_status_fname[GR_NUM_RESOLUTIONS] = {
	"MLStatus",		// GR_640
	"MLStatus"		// GR_1024
};
*/

static char* Hud_mission_log_mask_fname[GR_NUM_RESOLUTIONS] = {
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
}
;

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
	int i, h;

	MSG_WINDOW_WIDTH = gr_screen.clip_width_unscaled - 20;

	Hud_list_start = 0;
	Hud_list_end = 0;

	for (i=0; i<SCROLL_BUFFER_LINES; i++)
		HUD_pending[i].text[0] = HUD_pending[i].text[MAX_HUD_LINE_LEN - 1] = 0;
	
	for ( i=0; i < MAX_ACTIVE_BUFFER_LINES; i++ ) {
		HUD_active_msgs_list[i].total_life = 1;
	}

	Scroll_time_id = 1;

	// determine the height of the msg window, which depends on the font height	
	gr_set_font(FONT1);
	h = gr_get_font_height();

	//ACTIVE_BUFFER_LINES = Players[Player_num].HUD_config.num_msg_window_lines;
//	ACTIVE_BUFFER_LINES = HUD_config.num_msg_window_lines;
	ACTIVE_BUFFER_LINES = 4;

	MSG_WINDOW_FONT_HEIGHT = h;
	MSG_WINDOW_HEIGHT = MSG_WINDOW_FONT_HEIGHT * (ACTIVE_BUFFER_LINES-1);

	// starting a mission, free the scroll-back buffers, but only if we've been
	// through this function once already
	if ( HUD_msg_inited == TRUE ) {
		hud_free_scrollback_list();
	}

	list_init( &Msg_scrollback_free_list );
	list_init( &Msg_scrollback_used_list );

	// first slot is reserved for dummy node
	for (i=1; i < MAX_MSG_SCROLLBACK_LINES; i++)	{
		Msg_scrollback_lines[i].text = NULL;
		list_append(&Msg_scrollback_free_list, &Msg_scrollback_lines[i]);
	}

	Active_index=0;
	Scroll_needed=0;
	Scroll_in_progress=0;

	OLD_ACTIVE_BUFFER_LINES = ACTIVE_BUFFER_LINES;

	HUD_init_fixed_text();
	HUD_msg_inited = TRUE;
}

// ---------------------------------------------------------------------------------------
// hud_show_msg_window() will display the active HUD messages on the HUD.  It will scroll
// the messages up when a new message arrives.  
//
void hud_show_msg_window()
{
	int i, index;

	hud_set_default_color();
	gr_set_font(FONT1);

	HUD_set_clip(MSG_WINDOW_X_START,MSG_WINDOW_Y_START, MSG_WINDOW_WIDTH, MSG_WINDOW_HEIGHT+2);

	if ( OLD_ACTIVE_BUFFER_LINES != ACTIVE_BUFFER_LINES ) {
		// the size of the message window has changed, the best thing to do is to put all
		// the blank out the current hud messages.  There is no need to add them to the 
		// scrollback buffer, since they are already there!
	
		for ( i=0; i < ACTIVE_BUFFER_LINES; i++ ) {
			if ( !timestamp_elapsed(HUD_active_msgs_list[i].total_life) ) {
				HUD_active_msgs_list[i].total_life = 1;
				Active_index=0;
			}
		}
	}
	
	OLD_ACTIVE_BUFFER_LINES = ACTIVE_BUFFER_LINES;

	// check if there is a message to display on the HUD, and if there is room to display it
	if ( Hud_list_start != Hud_list_end && !Scroll_needed) {

		Hud_list_start++;

		// if the pointer exceeds the array size, wrap around to element 1.  element 0 is not used.		
		if (Hud_list_start >= SCROLL_BUFFER_LINES)
			Hud_list_start = 1;

		HUD_active_msgs_list[Active_index].msg = HUD_pending[Hud_list_start];
		HUD_active_msgs_list[Active_index].total_life = timestamp(HUD_MESSAGE_TOTAL_LIFE);

		for (i=Active_index+1; i < Active_index+ACTIVE_BUFFER_LINES; i++) {
			index = i % ACTIVE_BUFFER_LINES;

			// determine if there are any existing messages, if so need to scroll them up

			if ( !timestamp_elapsed(HUD_active_msgs_list[index].total_life) ) {
				HUD_active_msgs_list[index].target_y -=  MSG_WINDOW_FONT_HEIGHT;
				Scroll_needed=1;
			}

		}

		if (Scroll_needed) {
			HUD_active_msgs_list[Active_index].y = (ACTIVE_BUFFER_LINES-1)*MSG_WINDOW_FONT_HEIGHT;
			HUD_active_msgs_list[Active_index].target_y = HUD_active_msgs_list[Active_index].y - MSG_WINDOW_FONT_HEIGHT;
		}
		else {
			HUD_active_msgs_list[Active_index].y = (ACTIVE_BUFFER_LINES-2)*MSG_WINDOW_FONT_HEIGHT;
			HUD_active_msgs_list[Active_index].target_y = HUD_active_msgs_list[Active_index].y;
		}

		Active_index++;
		if (Active_index >= ACTIVE_BUFFER_LINES) Active_index = 0;

		if (Hud_list_end == Hud_list_start) {	// just printed the last msg
			Hud_list_start = Hud_list_end = 0;
		}
	}

	Scroll_in_progress=0;
	Scroll_needed = 0;

	for ( i=0; i < ACTIVE_BUFFER_LINES; i++ ) {

		if ( !timestamp_elapsed(HUD_active_msgs_list[i].total_life) ) {

			if (HUD_active_msgs_list[i].y > HUD_active_msgs_list[i].target_y) {
				Scroll_needed=1;
				if (timestamp_elapsed(Scroll_time_id) ){
					HUD_active_msgs_list[i].y -= SCROLL_STEP_SIZE;
					if (HUD_active_msgs_list[i].y < HUD_active_msgs_list[i].target_y)
						HUD_active_msgs_list[i].y = HUD_active_msgs_list[i].target_y;

					Scroll_in_progress=1;
				}

			}

			if ( hud_gauge_active(HUD_MESSAGE_LINES) ) {
				if ( !(Player->flags & PLAYER_FLAGS_MSG_MODE) ) {
					// set the appropriate color					
					if(HUD_active_msgs_list[i].msg.source){
						hud_set_gauge_color(HUD_MESSAGE_LINES, HUD_C_BRIGHT);
					} else {
						hud_set_gauge_color(HUD_MESSAGE_LINES);
					}

					// print the message out
					gr_printf(MSG_WINDOW_X_START + HUD_active_msgs_list[i].msg.x - 2, HUD_active_msgs_list[i].y, "%s", HUD_active_msgs_list[i].msg.text);
				}
			}
		}

	} // end for

	if (Scroll_in_progress)
		Scroll_time_id = timestamp(SCROLL_TIME);

	HUD_reset_clip();
}

void hud_show_fixed_text()
{
	HUD_ft	*hp;

	hp = &HUD_fixed_text[0];

	if (!timestamp_elapsed(hp->end_time)) {
		//gr_set_color((hp->color >> 16) & 0xff, (hp->color >> 8) & 0xff, hp->color & 0xff);
		gr_printf(0x8000, MSG_WINDOW_Y_START + MSG_WINDOW_HEIGHT + 8, hp->text);
	}
}

//	Similar to HUD printf, but shows only one message at a time, at a fixed location.
void HUD_fixed_printf(float duration, char * format, ...)
{
	va_list	args;
	char		tmp[HUD_MSG_LENGTH_MAX];
	int		msg_length;

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state != NETGAME_STATE_IN_MISSION)){
		nprintf(("Network","HUD_fixed_printf bailing because not in multiplayer game play state\n"));
		return;
	}

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	msg_length = strlen(tmp);
	Assert(msg_length < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.

	if ( !msg_length ) {
		nprintf(("Warning", "HUD_fixed_printf ==> attempt to print a 0 length string in msg window\n"));
		return;

	} else if (msg_length > MAX_HUD_LINE_LEN - 1){
		nprintf(("Warning", "HUD_fixed_printf ==> Following string truncated to %d chars: %s\n",MAX_HUD_LINE_LEN,tmp));
	}

	if (duration == 0.0f){
		HUD_fixed_text[0].end_time = timestamp(-1);
	} else {
		HUD_fixed_text[0].end_time = timestamp((int) (1000.0f * duration));
	}

	strncpy(HUD_fixed_text[0].text, tmp, MAX_HUD_LINE_LEN - 1);
	HUD_fixed_text[0].color = 0xff0000;
}

//	Clear all pending text.
void HUD_fixed_printf_reset()
{
	HUD_init_fixed_text();
}


// --------------------------------------------------------------------------------------
// HUD_printf_line() 
//
//	Print a single line of text to the HUD.  We know that the text will fit on the screen,
// since that was taken care of in HUD_printf();
//
void HUD_printf_line(char *text, int source, int time = 0, int x = 0)
{
	Assert(text != NULL);

	// if the pointer exceeds the array size, wrap around to element 1.  element 0 is not used.		
	Hud_list_end++;
	if (Hud_list_end >= SCROLL_BUFFER_LINES)
		Hud_list_end = 1;

	if (Hud_list_end == Hud_list_start) {
		nprintf(("Warning", "HUD ==> Exceeded msg scroll buffer, discarding message %s\n", text));
		Hud_list_end--;
		if (Hud_list_end == 0)
			Hud_list_end = SCROLL_BUFFER_LINES - 1;
		return;
	}

	if ( strlen(text) > MAX_HUD_LINE_LEN - 1 ){
		nprintf(("Warning", "HUD_printf_line() ==> Following string truncated to %d chars: %s\n", MAX_HUD_LINE_LEN, text));
	}

	strncpy(HUD_pending[Hud_list_end].text, text, MAX_HUD_LINE_LEN - 1);
	HUD_pending[Hud_list_end].text[MAX_HUD_LINE_LEN - 1] = 0;
	HUD_pending[Hud_list_end].source = source;
	HUD_pending[Hud_list_end].time = time;
	HUD_pending[Hud_list_end].x = x;
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


void HUD_printf(char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];
	int len;

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->state != NETPLAYER_STATE_IN_MISSION)){
		nprintf(("Network","HUD_printf bailing because not in multiplayer game play state\n"));
		return;
	}

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	len = strlen(tmp);
	Assert(len < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.
	hud_sourced_print(HUD_SOURCE_COMPUTER, tmp);
}

void HUD_ship_sent_printf(int sh, char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];
	int len;

	sprintf(tmp, NOX("%s: "), Ships[sh].ship_name);
	len = strlen(tmp);
	Assert(len < HUD_MSG_LENGTH_MAX);

	va_start(args, format);
	vsprintf(tmp + len, format, args);
	va_end(args);

	len = strlen(tmp);
	Assert(len < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.
	hud_sourced_print(HUD_team_get_source(Ships[sh].team), tmp);
}

// --------------------------------------------------------------------------------------
// HUD_sourced_printf() 
//
// HUD_sourced_printf() has the same parameters as printf(), but displays the text as a scrolling
// message on the HUD.  Text is split into multiple lines if width exceeds msg display area
// width.  'source' is used to indicate who send the message, and is used to color code text.
//
void HUD_sourced_printf(int source, char *format, ...)
{
	va_list args;
	char tmp[HUD_MSG_LENGTH_MAX];

	// make sure we only print these messages if we're in the correct state
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->state != NETPLAYER_STATE_IN_MISSION)){
		nprintf(("Network","HUD_sourced_printf bailing because not in multiplayer game play state\n"));
		return;
	}
	
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);
	Assert(strlen(tmp) < HUD_MSG_LENGTH_MAX);	//	If greater than this, probably crashed anyway.
	hud_sourced_print(source, tmp);
}

void hud_sourced_print(int source, char *msg)
{
	char *ptr, *str;
	//char *src_str, *msg_str;
	int sw, t, x, offset = 0;
	//int fudge = (gr_screen.res == GR_640) ? 15 : 50;		// prevents string from running off screen

	if ( !strlen(msg) ) {
		nprintf(("Warning", "HUD ==> attempt to print a 0 length string in msg window\n"));
		return;
	}

	// add message to the scrollback log first
	hud_add_msg_to_scrollback(msg, source, timestamp());

	ptr = strstr(msg, NOX(": ")) + 2;
	if (ptr) {
		gr_get_string_size(&sw, NULL, msg, ptr - msg);			// get width of the speaker field
		//if (sw < MSG_WINDOW_WIDTH - 20)
		offset = sw;
	}

	x = 0;
	t = timestamp();
	str = msg;

	//Because functions to get font size don't compensate for *actual* screen size
	int pretend_width = (gr_screen.res == GR_640) ? 620 : 1004;

	while ((ptr = split_str_once(str, pretend_width - x - 7)) != NULL) {		// the 7 is a fudge hack
		// make sure that split went ok, if not then bail
		if (ptr == str) {
			Int3();
			break;
		}

		HUD_printf_line(str, source, t, x);
		str = ptr;
		x = offset;
		t = 0;
	}

	HUD_printf_line(str, source, t, x);
}

int hud_query_scrollback_size()
{
	int count = 0, y_add = 0;
	int font_height = gr_get_font_height();
	line_node *ptr;

	if (EMPTY(&Msg_scrollback_used_list) || !HUD_msg_inited)
		return 0;

	ptr = GET_FIRST(&Msg_scrollback_used_list);
	while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
		if (ptr->source != HUD_SOURCE_HIDDEN) {
			y_add = ptr->y;
			count += font_height + ptr->y;
		}

		ptr = GET_NEXT(ptr);
	}

	count -= y_add;
	return count;
}

// add text directly to the hud scrollback log, without displaying on the hud
void HUD_add_to_scrollback(char *text, int source)
{
	if (!strlen(text)) {
		nprintf(("Warning", "HUD ==> attempt to print a 0 length string in msg window\n"));
		return;
	}

	hud_add_msg_to_scrollback(text, source, timestamp());
}

// hud_add_msg_to_scrollback() adds the new_msg to the scroll-back message list.  If there
// are no more free slots, the first slot is released to make room for the new message.
//
void hud_add_line_to_scrollback(char *text, int source, int t, int x, int y, int underline_width)
{
	line_node *new_line;

	Assert(HUD_msg_inited);
	if (!text || !strlen(text))
		return;

	if ( EMPTY(&Msg_scrollback_free_list) ) {
		new_line = GET_FIRST(&Msg_scrollback_used_list);
		list_remove(&Msg_scrollback_used_list, new_line);
		vm_free(new_line->text);

	} else {
		new_line = GET_FIRST(&Msg_scrollback_free_list);
		list_remove(&Msg_scrollback_free_list, new_line);
	}

	new_line->x = x;
	new_line->y = y;
	new_line->underline_width = underline_width;
	new_line->time = t;
	new_line->source = source;
	new_line->text = (char *) vm_malloc( strlen(text) + 1 );
	strcpy(new_line->text, text);
	list_append(&Msg_scrollback_used_list, new_line);
}

void hud_add_msg_to_scrollback(char *text, int source, int t)
{
	char buf[HUD_MSG_LENGTH_MAX], *ptr, *str;
	int msg_len, w, max_width, x, offset = 0;

	max_width = Hud_mission_log_list2_coords[gr_screen.res][2];
	msg_len = strlen(text);
	if (msg_len == 0)
		return;

	w = 0;
	Assert(msg_len < HUD_MSG_LENGTH_MAX);
	strcpy_s(buf, text);
	ptr = strstr(buf, NOX(": "));
	if (ptr) {
		gr_get_string_size(&w, NULL, buf, ptr - buf);
	}

//	if (ptr) {
//		gr_get_string_size(&w, NULL, buf, ptr - buf + 2);
//		if (w < max_width - 20)
//			offset = w;
//	}

	x = 0;
	str = buf;
	while ((ptr = split_str_once(str, max_width - x)) != NULL) {
		hud_add_line_to_scrollback(str, source, t, x, 1, w);
		if ( str == ptr )
			break;
		str = ptr;
		x = offset;
		t = w = 0;
	}

	hud_add_line_to_scrollback(str, source, t, x, 3, w);
}

// hud_free_scrollback_list() will free the memory that was allocated to store the messages
// for the scroll-back list
//
void hud_free_scrollback_list()
{
	line_node *A;

	// check if the list has been inited yet.  If not, return with doing nothing.
	if ( Msg_scrollback_used_list.next == NULL || Msg_scrollback_used_list.prev == NULL )
		return;

	A = GET_FIRST(&Msg_scrollback_used_list);
	while( A !=END_OF_LIST(&Msg_scrollback_used_list) )	{
		if ( A->text != NULL ) {
			vm_free(A->text);
			A->text = NULL;
		}

		A = GET_NEXT(A);
	}
}

// how many lines to skip
int hud_get_scroll_max_pos()
{
	int max = 0, font_height = gr_get_font_height();

	if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
		int count = 0;
		line_node *ptr;
		// number of pixels in excess of what can be displayed
		int excess = Scroll_max - Hud_mission_log_list_coords[gr_screen.res][3];

		if (EMPTY(&Msg_scrollback_used_list) || !HUD_msg_inited) {
			max = 0;

		} else {
			ptr = GET_FIRST(&Msg_scrollback_used_list);
			while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
				if (ptr->source != HUD_SOURCE_HIDDEN) {

					if (excess > 0) {
						excess -= font_height;
						count++;
					}

					if (excess <= 0) {
						max = count;
						break;
					}

					// spacing between lines
					excess -= ptr->y;

				}

				ptr = GET_NEXT(ptr);
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
			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);

	} else {
		if (Scroll_offset < hud_get_scroll_max_pos()) {
			Scroll_offset++;
			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void hud_goto_pos(int delta)
{
	int pos=0, font_height = gr_get_font_height();

	if (Scrollback_mode == SCROLLBACK_MODE_MSGS_LOG) {
		int count = 0, y = 0;
		line_node *ptr;

		if (EMPTY(&Msg_scrollback_used_list) || !HUD_msg_inited)
			return;

		ptr = GET_FIRST(&Msg_scrollback_used_list);
		while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
			if (ptr->source != HUD_SOURCE_HIDDEN) {
				if (count == Scroll_offset) {
					pos = y;
					break;
				}

				y += font_height + ptr->y;
				count++;
			}

			ptr = GET_NEXT(ptr);
		}

		Scroll_offset = count = y = 0;
		ptr = GET_FIRST(&Msg_scrollback_used_list);
		while (ptr != END_OF_LIST(&Msg_scrollback_used_list)) {
			if (ptr->source != HUD_SOURCE_HIDDEN) {
				if (y <= pos + delta)
					Scroll_offset = count;

				y += font_height + ptr->y;
				count++;
			}

			ptr = GET_NEXT(ptr);
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

			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);

	} else {
		if (Scroll_offset < max) {
			hud_goto_pos(Hud_mission_log_list_coords[gr_screen.res][3]);
			if (Scroll_offset > max)
				Scroll_offset = max;

			gamesnd_play_iface(SND_SCROLL);

		} else
			gamesnd_play_iface(SND_GENERAL_FAIL);
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

void hud_scrollback_init()
{
	int i;
	scrollback_buttons *b;

	// pause all game sounds
	beam_pause_sounds();
	audiostream_pause_all();

	common_set_interface_palette("BriefingPalette");  // set the interface palette
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Hud_mission_log_mask_fname[gr_screen.res]);

	for (i=0; i<NUM_BUTTONS; i++) {
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
	beam_unpause_sounds();
	audiostream_unpause_all();

}

void hud_scrollback_do_frame(float frametime)
{
	int i, k, x, y;
	int font_height = gr_get_font_height();

	k = Ui_window.process();
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

	for (i=0; i<NUM_BUTTONS; i++){
		if (Buttons[gr_screen.res][i].button.pressed()){
			hud_scrollback_button_pressed(i);		
		}
	}

	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0);
	}

	/*
	if ((Scrollback_mode == SCROLLBACK_MODE_OBJECTIVES) && (Status_bitmap >= 0)) {
		gr_set_bitmap(Status_bitmap);
		gr_bitmap(Hud_mission_log_status_coords[gr_screen.res][0], Hud_mission_log_status_coords[gr_screen.res][1]);
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
		line_node *node_ptr;

		Buttons[gr_screen.res][SHOW_MSGS_BUTTON].button.draw_forced(2);
//		y = ((LIST_H / font_height) - 1) * font_height;
		y = 0;
		if ( !EMPTY(&Msg_scrollback_used_list) && HUD_msg_inited ) {
			node_ptr = GET_FIRST(&Msg_scrollback_used_list);
			i = 0;
			while ( node_ptr != END_OF_LIST(&Msg_scrollback_used_list) ) {
				if ((node_ptr->source == HUD_SOURCE_HIDDEN) || (i++ < Scroll_offset)) {
					node_ptr = GET_NEXT(node_ptr);

				} else {
					int team = HUD_source_get_team(node_ptr->source);

					if (team >= 0)
					{
						gr_set_color_fast(iff_get_color_by_team(team, Player_ship->team, 0));
					}
					else
					{
						switch (node_ptr->source)
						{
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

					if (node_ptr->time)
						gr_print_timestamp(Hud_mission_log_list_coords[gr_screen.res][0], Hud_mission_log_list_coords[gr_screen.res][1] + y, node_ptr->time);

					x = Hud_mission_log_list2_coords[gr_screen.res][0] + node_ptr->x;
					gr_printf(x, Hud_mission_log_list_coords[gr_screen.res][1] + y, "%s", node_ptr->text);
					if (node_ptr->underline_width)
						gr_line(x, Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height - 1, x + node_ptr->underline_width, Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height - 1);

					if ((node_ptr->source == HUD_SOURCE_FAILED) || (node_ptr->source == HUD_SOURCE_SATISFIED)) {
						// draw goal icon
						if (node_ptr->source == HUD_SOURCE_FAILED)
							gr_set_color_fast(&Color_bright_red);
						else
							gr_set_color_fast(&Color_bright_green);

						i = Hud_mission_log_list_coords[gr_screen.res][1] + y + font_height / 2 - 1;
						gr_circle(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i, 5);

						gr_set_color_fast(&Color_bright);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 10, i, Hud_mission_log_list2_coords[gr_screen.res][0] - 8, i);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i - 4, Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i - 2);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 4, i, Hud_mission_log_list2_coords[gr_screen.res][0] - 2, i);
						gr_line(Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i + 2, Hud_mission_log_list2_coords[gr_screen.res][0] - 6, i + 4);
					}

					y += font_height + node_ptr->y;
					node_ptr = GET_NEXT(node_ptr);
					if (y + font_height > Hud_mission_log_list_coords[gr_screen.res][3])
						break;
				}
			}
		}
	}

	gr_set_color_fast(&Color_text_heading);
	gr_print_timestamp(Hud_mission_log_time_coords[gr_screen.res][0], Hud_mission_log_time_coords[gr_screen.res][1] - font_height, (int) (f2fl(Missiontime) * 1000));
	gr_string(Hud_mission_log_time2_coords[gr_screen.res][0], Hud_mission_log_time_coords[gr_screen.res][1] - font_height, XSTR( "Current time", 289));
	gr_flip();
}

void hud_scrollback_exit()
{
	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
}

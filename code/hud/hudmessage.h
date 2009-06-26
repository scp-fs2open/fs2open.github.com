/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUDMESSAGE_H
#define _HUDMESSAGE_H

#define SCROLL_BUFFER_LINES		128			// maximum number of HUD messages that can be stored
#define SCROLL_TIME					30				// time in milliseconds between scrolling a message
#define SCROLL_STEP_SIZE			3
#define MAX_HUD_LINE_LEN			256			// maximum number of characters for a HUD message
#define MAX_ACTIVE_BUFFER_LINES	10

#define HUD_SOURCE_COMPUTER		0
#define HUD_SOURCE_TRAINING		1
#define HUD_SOURCE_HIDDEN		2
#define HUD_SOURCE_IMPORTANT	3
#define HUD_SOURCE_FAILED		4
#define HUD_SOURCE_SATISFIED	5
#define HUD_SOURCE_TERRAN_CMD	6
#define HUD_SOURCE_NETPLAYER	7

#define HUD_SOURCE_TEAM_OFFSET	8	// must be higher than any previous hud source


extern int ACTIVE_BUFFER_LINES;					// user-preferred number of message buffer lines

typedef struct {
	char text[MAX_HUD_LINE_LEN];
	int source;  // where this message came from so we can color code it
	int time;  // timestamp message was originally sent
	int x;
} HUD_message_data;

typedef struct line_node {
	line_node* next;
	line_node* prev;
	int time;  // timestamp when message was added
	int source;  // who/what the source of the message was (for color coding)
	int x;
	int y;
	int underline_width;
	char *text;
} line_node;

extern line_node Msg_scrollback_used_list;

typedef struct Hud_display_info {
	HUD_message_data msg;
	int y;						// y Coordinate to draw message at
	int target_y;
	int total_life;			// timestamp id to control how long a HUD message stays alive	
} Hud_display_info;

extern HUD_message_data HUD_pending[SCROLL_BUFFER_LINES];
extern Hud_display_info HUD_active_msgs_list[MAX_ACTIVE_BUFFER_LINES];
extern int Hud_list_start;				// points to the next msg to be printed in the queue
extern int Hud_list_end;				// points to the last msg in the queue
extern int Scroll_time_id;
extern int Active_index;
extern int Scroll_needed;
extern int Scroll_in_progress;

extern int MSG_WINDOW_HEIGHT;			// extern'ed since needed in save/restore code
extern int MSG_WINDOW_FONT_HEIGHT;	// extern'ed since needed in save/restore code

void hud_scrollback_init();
void hud_scrollback_close();
void hud_scrollback_do_frame(float frametime);
void hud_scrollback_exit();

void hud_init_msg_window();
void hud_show_msg_window();
void hud_show_fixed_text();
int HUD_team_get_source(int team);
int HUD_source_get_team(int team);
void HUD_printf(char *format, ...);
void hud_sourced_print(int source, char *msg);
void HUD_sourced_printf(int source, char *format, ...);  // send hud message from specified source
void HUD_ship_sent_printf(int sh, char *format, ...);  // send hud message from a specific ship
void HUD_fixed_printf(float duration, char *format, ...);		//	Display a single message for duration seconds.
void HUD_init_fixed_text();			//	Clear all pending fixed text.

void HUD_add_to_scrollback(char *text, int source);
void hud_add_line_to_scrollback(char *text, int source, int t, int x, int y, int w);
void hud_add_msg_to_scrollback(char *text, int source, int t);
void hud_free_scrollback_list();

#endif

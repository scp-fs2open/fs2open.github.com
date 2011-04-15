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

#include "hud/hud.h"
#include "anim/packunpack.h"

#define MAX_HUD_LINE_LEN			256			// maximum number of characters for a HUD message

#define HUD_SOURCE_COMPUTER		0
#define HUD_SOURCE_TRAINING		1
#define HUD_SOURCE_HIDDEN		2
#define HUD_SOURCE_IMPORTANT	3
#define HUD_SOURCE_FAILED		4
#define HUD_SOURCE_SATISFIED	5
#define HUD_SOURCE_TERRAN_CMD	6
#define HUD_SOURCE_NETPLAYER	7

#define HUD_SOURCE_TEAM_OFFSET	8	// must be higher than any previous hud source

typedef struct HUD_message_data {
	SCP_string text;
	int source;  // where this message came from so we can color code it
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


void hud_scrollback_init();
void hud_scrollback_close();
void hud_scrollback_do_frame(float frametime);
void hud_scrollback_exit();

void hud_init_msg_window();
void hud_clear_msg_buffer();
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

class HudGaugeMessages: public HudGauge // HUD_MESSAGE_LINES
{
protected:
	// User-defined properties
	int Max_lines;
	int Max_width; // 620 for GR_640 and 1004 for GR_1024
	int Scroll_time;
	int Step_size;
	int Total_life;
	int Line_h;

	int Window_width;
	int Window_height;

	SCP_vector<Hud_display_info> active_messages;
	SCP_queue<HUD_message_data> pending_messages;

	bool Scroll_needed;
	bool Scroll_in_progress;
	int Scroll_time_id;
public:
	HudGaugeMessages();

	void initLineHeight(int h);
	void initMaxLines(int lines);
	void initMaxWidth(int width);
	void initScrollTime(int ms);
	void initStepSize(int h);
	void initTotalLife(int ms);

	void processMessageBuffer();
	void addPending(char *text, int source, int x = 0);
	void scrollMessages();
	void render(float frametime);
	void initialize();
	void pageIn();
};

class HudGaugeTalkingHead: public HudGauge // HUD_TALKING_HEAD
{
	hud_frames Head_frame;

	int Header_offsets[2];
	int Anim_offsets[2];
	int Anim_size[2];

	anim_instance *head_anim;
	int msg_id;
public:
	HudGaugeTalkingHead();
	void initBitmaps(char *fname);
	void initHeaderOffsets(int x, int y);
	void initAnimOffsets(int x, int y);
	void initAnimSizes(int w, int h);
	void pageIn();
	void render(float frametime);
	void initialize();
	bool canRender();
	anim_instance* createAnim(int anim_start_frame, anim* anim_data);
};

class HudGaugeFixedMessages: public HudGauge
{
public:
	HudGaugeFixedMessages();
	void render(float frametime);
	void pageIn();
};

#endif

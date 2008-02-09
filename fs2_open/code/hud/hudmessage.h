/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDmessage.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Header file for functions that control and manage the message window on the HUD
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     8/23/99 11:11a Jefff
 * increased  MAX_HUD_LINE_LENGTH
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 25    4/14/98 5:06p Dave
 * Don't load or send invalid pilot pics. Fixed chatbox graphic errors.
 * Made chatbox display team icons in a team vs. team game. Fixed up pause
 * and endgame sequencing issues.
 * 
 * 24    4/05/98 3:30p Dave
 * Print netplayer messages in brighter green on the hud, with
 * accompanying sound. Echo netplayer messages on sending machine. Fixed
 * standalone sequencing bug where host never get the "enter mission"
 * button.
 * 
 * 23    3/17/98 4:01p Hoffoss
 * Added HUD_SOURCE_TERRAN_CMD and changed code to utilize it when a
 * message is being sent from Terran Command.
 * 
 * 22    3/12/98 4:03p Hoffoss
 * Changed formatting used in hug scrollbacl log.
 * 
 * 21    1/18/98 5:09p Lawrance
 * Added support for TEAM_TRAITOR
 * 
 * 20    12/03/97 11:35a Hoffoss
 * Made changes to HUD messages send throughout the game.
 * 
 * 19    12/02/97 5:57p Hoffoss
 * Changed Hud messaging code to align text to right after sending ship's
 * name.
 * 
 * 18    11/13/97 10:16p Hoffoss
 * Added icons to mission log scrollback.
 * 
 * 17    11/13/97 4:05p Hoffoss
 * Added hiding code for mission log entries.
 * 
 * 16    11/13/97 1:13p Hoffoss
 * Added HUD_SOURCE_HIDDEN to be used to hide items from the message
 * scrollback log.
 * 
 * 15    11/12/97 6:00p Hoffoss
 * Added training messages to hud scrollback log.
 * 
 * 14    11/05/97 7:11p Hoffoss
 * Made changed to the hud message system.  Hud messages can now have
 * sources so they can be color coded.
 * 
 * 13    11/03/97 10:12p Hoffoss
 * Finished up work on the hud message/mission log scrollback screen.
 * 
 * 12    10/25/97 4:02p Lawrance
 * took out unused hud_message struct members
 * 
 * 11    9/05/97 4:59p Lawrance
 * changed MAX_HUD_LINE_LEN
 * 
 * 10    8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 9     6/23/97 12:03p Lawrance
 * move split_str() to Parselo
 * 
 * 8     6/17/97 12:25p Lawrance
 * HUD message lines are split into multiple lines when they exceed
 * display width
 * 
 * 7     4/15/97 1:26p Lawrance
 * using a static array of nodes to store hud scrollback messages, storage
 * for text is dynamic
 * 
 * 6     4/14/97 9:55a Mike
 * Fixed HUD message system.
 * Better game sequencing.
 * 
 * 5     1/24/97 9:47a Lawrance
 * made number of message lines in HUD message area confiurable
 * 
 * 4     1/07/97 5:36p Lawrance
 * Enabled save/restore for  old/present/pending hud messages
 * 
 * 3     11/27/96 3:20p Lawrance
 * added scroll-back message code
 * 
 * 2     11/15/96 12:11a Lawrance
 * HUD message bar working
 *
 * $NoKeywords: $
 *
*/

#ifndef _HUDMESSAGE_H
#define _HUDMESSAGE_H

#define SCROLL_BUFFER_LINES		128			// maximum number of HUD messages that can be stored
#define SCROLL_TIME					30				// time in milliseconds between scrolling a message
#define SCROLL_STEP_SIZE			3
#define MAX_HUD_LINE_LEN			256			// maximum number of characters for a HUD message
#define MAX_ACTIVE_BUFFER_LINES	10

#define HUD_SOURCE_COMPUTER	0
#define HUD_SOURCE_FRIENDLY	1
#define HUD_SOURCE_HOSTILE		2
#define HUD_SOURCE_NEUTRAL		3
#define HUD_SOURCE_UNKNOWN		4
#define HUD_SOURCE_TRAITOR		5
#define HUD_SOURCE_TRAINING	6
#define HUD_SOURCE_HIDDEN		7
#define HUD_SOURCE_IMPORTANT	8
#define HUD_SOURCE_FAILED		9
#define HUD_SOURCE_SATISFIED	10
#define HUD_SOURCE_TERRAN_CMD	11
#define HUD_SOURCE_NETPLAYER	12

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
int HUD_get_team_source(int team);
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

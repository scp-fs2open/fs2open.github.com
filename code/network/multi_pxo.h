/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _PARALLAX_ONLINE_HEADER_FILE
#define _PARALLAX_ONLINE_HEADER_FILE

#include "playerman/player.h"

// ----------------------------------------------------------------------------------------------------
// PXO DEFINES/VARS
//

#define MAX_PXO_TEXT_LEN 255

// default url for PXO rankings
//#define MULTI_PXO_RANKINGS_URL				"http://www.volition-inc.com"
#define MULTI_PXO_RANKINGS_URL				"http://www.pxo.net/rankings/fs2full.cfm"


// default url for PXO account creation
//#define MULTI_PXO_CREATE_URL				"http://www.parallaxonline.com/register.html"
#define MULTI_PXO_CREATE_URL					"http://www.pxo.net/newaccount.cfm"

// default url for PXO account verification
//#define MULTI_PXO_VERIFY_URL				"http://www.parallaxonline.com/verify.html"
#define MULTI_PXO_VERIFY_URL					"http://www.pxo.net/verify.cfm"

// default url for PXO banners
#define MULTI_PXO_BANNER_URL					"http://www.pxo.net/files/banners"

// tracker and PXO addresses
#define MULTI_PXO_USER_TRACKER_IP		"ut.pxo.net"
#define MULTI_PXO_GAME_TRACKER_IP		"gt.pxo.com"
#define MULTI_PXO_CHAT_IP					"chat.pxo.net"

// channel related stuff -------------------------------------------
#define MAX_CHANNEL_NAME_LEN 32
#define MAX_CHANNEL_DESCRIPT_LEN 120

typedef struct pxo_channel {
	char name[MAX_CHANNEL_NAME_LEN + 1];     // name
	char desc[MAX_CHANNEL_DESCRIPT_LEN + 1]; // description
	short num_users;                         // # users, or -1 if not in use
	short num_servers;                       // the # of servers registered on this channel
} pxo_channel;

extern SCP_vector<pxo_channel> Multi_pxo_channels;

// channel we're currently connected to, num_users == -1, if we're not connected
extern pxo_channel Multi_pxo_channel_current;

// player related stuff -------------------------------------------
#define MAX_CHAT_LINES 500 //Abritrary size limit. After this number, old messages are removed from the start of the chat vector
#define MAX_PLAYER_NAME_LEN 32

extern SCP_vector<SCP_string> Multi_pxo_players;

// chat related stuff ----------------------------------------------
#define MAX_CHAT_LINE_LEN 256

// chat flags
#define CHAT_MODE_NORMAL 0         // normal chat from someone
#define CHAT_MODE_SERVER 1         // is from the server, display appropriately
#define CHAT_MODE_CARRY 2          // is a carryover from a previous line
#define CHAT_MODE_PRIVATE 3        // is a private message
#define CHAT_MODE_CHANNEL_SWITCH 4 // "switching channels" message - draw in red
#define CHAT_MODE_MOTD 5           // message of the day from the chat server

typedef struct chat_line {
	char text[MAX_CHAT_LINE_LEN + 1];
	int mode;
} chat_line;

extern SCP_list<chat_line> Multi_pxo_chat;

// banner related stuff --------------------------------------------
// banners
typedef struct pxo_banner {
	SCP_string ban_file;     // base filename of the banner
	SCP_string ban_file_url; // full url of the file to get (convenient)
	SCP_string ban_url;      // url to go to when clicked
	int ban_bitmap;          // banner bitmap
} pxo_banner;

// active pxo banner
extern pxo_banner Multi_pxo_banner;

// help page related stuff -----------------------------------------
#define MULTI_PXO_MAX_LINES_PP 57
#define MULTI_PXO_MAX_PAGES 5

// help text pages
typedef struct help_page {
	char* text[MULTI_PXO_MAX_LINES_PP];
	int num_lines;
} help_page;

extern help_page Multi_pxo_help_pages[MULTI_PXO_MAX_PAGES];

// Globals for the UI API

extern player Multi_pxo_pinfo_player;

// if we're connected
extern int Multi_pxo_connected;

// the status text itself
extern char Multi_pxo_status_text[MAX_PXO_TEXT_LEN];

// the motd text itself
extern char Pxo_motd[1024];

// ----------------------------------------------------------------------------------------------------
// PXO FUNCTIONS
//

// initialize the PXO screen
void multi_pxo_init(int use_last_channel, bool api_access = false);

// do frame for the PXO screen
void multi_pxo_do();

// close the PXO screen
void multi_pxo_close(bool api_access = false);

// initialize the PXO help screen
void multi_pxo_help_init();

// load the help file up
void multi_pxo_help_load();

// free the help text from memory
void multi_pxo_help_free();

// do frame for PXO help
void multi_pxo_help_do();

// close the pxo screen
void multi_pxo_help_close();

// open up a URL
void multi_pxo_url(const char *url);

// called from the game tracker API - server count update for a channel
void multi_pxo_channel_count_update(char *name,int count);

// run the networking api
void multi_pxo_api_process();

// run normally (no popups)
void multi_pxo_do_normal(bool api_access = false);

// process the various network list items
void multi_pxo_process_common(bool api_access = false);

// send a chat string to the current pxo channel
void multi_pxo_chat_send(const char* msg);

// try to join a channel
void multi_pxo_maybe_join_channel(pxo_channel* chan);

// public method to get a player's info. Returns true if successful, false otherwise
bool multi_pxo_maybe_get_player(const char* name);

#endif

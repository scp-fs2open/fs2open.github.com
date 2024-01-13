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

// ----------------------------------------------------------------------------------------------------
// PXO DEFINES/VARS
//

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

// player related stuff -------------------------------------------
#define MAX_CHAT_LINES 500 //Abritrary size limit. After this number, old messages are removed from the start of the chat vector
#define MAX_PLAYER_NAME_LEN 32

extern SCP_vector<SCP_string> Multi_pxo_players;

// chat related stuff ----------------------------------------------
#define MAX_CHAT_LINE_LEN 256

typedef struct chat_line {
	char text[MAX_CHAT_LINE_LEN + 1];
	int mode;
} chat_line;

extern SCP_list<chat_line> Multi_pxo_chat;

// ----------------------------------------------------------------------------------------------------
// PXO FUNCTIONS
//

// initialize the PXO screen
void multi_pxo_init(int use_last_channel);

// do frame for the PXO screen
void multi_pxo_do();

// close the PXO screen
void multi_pxo_close();


// initialize the PXO help screen
void multi_pxo_help_init();

// do frame for PXO help
void multi_pxo_help_do();

// close the pxo screen
void multi_pxo_help_close();

// open up a URL
void multi_pxo_url(const char *url);

// called from the game tracker API - server count update for a channel
void multi_pxo_channel_count_update(char *name,int count);

#endif

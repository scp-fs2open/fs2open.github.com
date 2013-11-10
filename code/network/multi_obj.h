/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _MULTI_NEW_OBJECT_UPDATE_HEADER_FILE
#define _MULTI_NEW_OBJECT_UPDATE_HEADER_FILE

#include "math/vecmat.h"

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE DEFINES/VARS
//
#include "globalincs/pstypes.h"

struct interp_info;
class object;
struct header;
struct net_player;


// client button info flags
#define OOC_FIRE_SECONDARY			(1<<0)
#define OOC_TARGET_LOCKED			(1<<1)
#define OOC_TARGET_SEEK_LOCK		(1<<2)
#define OOC_LOCKING_ON_CENTER		(1<<3)
#define OOC_TRIGGER_DOWN			(1<<4)
#define OOC_PRIMARY_BANK			(1<<5)
#define OOC_PRIMARY_LINKED			(1<<6)
#define OOC_AFTERBURNER_ON			(1<<7)
// NOTE: no additional flags here unless it's sent in an extra data byte

// update info
typedef struct np_update {	
	ubyte		seq;							// sequence #
	int		update_stamp;				// global update stamp
	int		status_update_stamp;
	int		subsys_update_stamp;
	ushort	pos_chksum;					// positional checksum
	ushort	orient_chksum;				// orient checksum
} np_update;

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE FUNCTIONS
//

// process all object update details for this frame
void multi_oo_process();

// process incoming object update data
void multi_oo_process_update(ubyte *data, header *hinfo);

// initialize all object update timestamps (call whenever entering gameplay state)
void multi_oo_gameplay_init();

// send control info for a client (which is basically a "reverse" object update)
void multi_oo_send_control_info();
void multi_oo_send_changed_object(object *changedobj);

// reset all sequencing info
void multi_oo_reset_sequencing();

// is this object one which needs to go through the interpolation
int multi_oo_is_interp_object(object *objp);

// interp
void multi_oo_interp(object *objp);


// ---------------------------------------------------------------------------------------------------
// DATARATE DEFINES/VARS
//

#define OO_HIGH_RATE_DEFAULT				11000


// ---------------------------------------------------------------------------------------------------
// DATARATE FUNCTIONS
//

// process all object update datarate details
void multi_oo_rate_process();

// initialize all datarate checking stuff
void multi_oo_rate_init_all();

// initialize the rate limiting for the passed in player
void multi_oo_rate_init(net_player *pl);

// if the given net-player has exceeded his datarate limit, or if the overall datarate limit has been reached
int multi_oo_rate_exceeded(net_player *pl);

// if it is ok for me to send a control info (will be ~N times a second)
int multi_oo_cirate_can_send();

// display any oo info on the hud
void multi_oo_display();

// notify of a player join
void multi_oo_player_reset_all(net_player *pl = NULL);

#endif

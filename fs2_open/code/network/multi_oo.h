/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _MULTIPLAYER_OBJECT_UPDATE_HEADER_FILE
#define _MULTIPLAYER_OBJECT_UPDATE_HEADER_FILE

#include "math/vecmat.h"

#define OO_NEW

#ifdef OO_NEW
	#include "network/multi_obj.h"
#else 

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE DEFINES/VARS
//
struct interp_info;
struct object;
struct header;
struct net_player;

// client button info flags
#define OOC_FIRE_PRIMARY			(1<<0)
#define OOC_FIRE_SECONDARY			(1<<1)
#define OOC_FIRE_COUNTERMEASURE	(1<<2)
#define OOC_TARGET_LOCKED			(1<<3)
#define OOC_TARGET_SEEK_LOCK		(1<<4)
#define OOC_LOCKING_ON_CENTER		(1<<5)

// interpolation info struct 
typedef struct interp_info {
	// position and timestamp
	vec3d pos;
	int pos_time;

	// velocity and timestamp
	vec3d vel;
	int vel_time;

	// desired velocity and timestamp
	vec3d desired_vel;
	int desired_vel_time;

	// orientation and timestamp
	matrix orient;
	int orient_time;

	// rotvel and timestamp
	vec3d rotvel;
	int rotvel_time;

	// desired rotvel and timestamp
	vec3d desired_rotvel;
	int desired_rotvel_time;

	// ping info (in ms)
	int lowest_ping;				// lowest ping (or -1, if not known)
	int lowest_ping_avg;			// (lowest ping + average ping)/2   or -1 if not known
} interp_info;


// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE FUNCTIONS
//

// process all object update details for this frame
void multi_oo_process();

// process incoming object update data
void multi_oo_process_update(ubyte *data, header *hinfo);

// initialize all object update timestamps (call whenever entering gameplay state)
void multi_oo_gameplay_init();

// process an object update sync packet
void multi_oo_process_update_sync(ubyte *data, header *hinfo);

// send an update sync packet
void multi_oo_send_update_sync(net_player *pl = NULL);

// initialize the server's time sync stuff
void multi_oo_sync_init();

// send control info for a client (which is basically a "reverse" object update)
void multi_oo_send_control_info();

// reset all sequencing info
void multi_oo_reset_sequencing();

// interpolate for this object
void multi_oo_interpolate(object *objp, interp_info *current, interp_info *last);

// do all interpolation for this frame - client side and server side
void multi_oo_interpolate_all();

// set global object update timestamp for this frame
void multi_oo_set_global_timestamp();


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

#endif // #ifdef OO_NEW

#endif

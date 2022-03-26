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
class ship;
struct physics_info;
struct weapon;


// client button info flags
#define OOC_FIRE_CONTROL_PRESSED	(1<<0)
#define OOC_TARGET_SEEK_LOCK		(1<<1)
#define OOC_TRIGGER_DOWN			(1<<2)
#define OOC_PRIMARY_BANK			(1<<3)
#define OOC_PRIMARY_LINKED			(1<<4)
#define OOC_AFTERBURNER_ON			(1<<5)
// two spots now left for more OOC flags

// Cyborg17, Server will be tracking only the last 0.5-1.0 second of frames
#define MAX_FRAMES_RECORDED		30
#define PRIMARY_PACKET_CUTOFF			2000

// one special value to help with multilock
#define OOC_INDEX_NULLPTR_SUBSYSEM USHRT_MAX

// some values for subsystem packing
#define OO_SUBSYS_HEALTH			(1<<0)		// Did this subsystem's health change
#define OO_SUBSYS_ROTATION_1b		(1<<1)		// Did this subsystem's base rotation angles change
#define OO_SUBSYS_ROTATION_1h		(1<<2)		// Did this subsystem's base rotation angles change
#define OO_SUBSYS_ROTATION_1p		(1<<3)		// Did this subsystem's base rotation angles change
#define OO_SUBSYS_ROTATION_2b		(1<<4)		// Did this subsystem's barrel rotation angles change
#define OO_SUBSYS_ROTATION_2h		(1<<5)		// Did this subsystem's barrel rotation angles change
#define OO_SUBSYS_ROTATION_2p		(1<<6)		// Did this subsystem's barrel rotation angles change
#define OO_SUBSYS_TRANSLATION		(1<<7)		// Only for backwards compatibility of future builds.

// combo values
#define OO_SUBSYS_ROTATION_1	(OO_SUBSYS_ROTATION_1b | OO_SUBSYS_ROTATION_1h | OO_SUBSYS_ROTATION_1p)
#define OO_SUBSYS_ROTATION_2	(OO_SUBSYS_ROTATION_2b | OO_SUBSYS_ROTATION_2h | OO_SUBSYS_ROTATION_2p)


// ---------------------------------------------------------------------------------------------------
// POSITION AND ORIENTATION RECORDING
// if it breaks, find Cyborg17 so you can yell at him
// This section is almost all server side

// Add a new ship *ON IN-GAME SHIP CREATION* to the tracking struct
void multi_rollback_ship_record_add_ship(int obj_num);

// Update the tracking struct whenver the object is updated in-game
void multi_ship_record_update_all();

// increment the tracker per frame, before incoming object packets are processed
void multi_ship_record_increment_frame();

// find the right frame to start our weapon simulation
int multi_ship_record_find_frame(int client_frame, int time_elapsed);

// a quick lookups for position and orientation
vec3d multi_ship_record_lookup_position(object* objp, int frame);

// a quick lookups for orientation
matrix multi_ship_record_lookup_orientation(object* objp, int frame);

// figures out how much time has passed bwetween the two frames.
int multi_ship_record_find_time_after_frame(int client_frame, int frame, int time_elapsed);

// This stores the information we got from the client to create later.
void multi_ship_record_add_rollback_shot(object* pobjp, vec3d* pos, matrix* orient, int frame, bool secondary);

// Lookup whether rollback mode is on
bool multi_ship_record_get_rollback_wep_mode();

// Adds a weapon to the rollback tracker.
void multi_ship_record_add_rollback_wep(int wep_objnum);

// Manage rollback for a frame
void multi_ship_record_do_rollback();

// ---------------------------------------------------------------------------------------------------
// Client side frame tracking, for now used only to help lookup info from packets to improve client accuracy.
// 

// Quick lookup for the most recently received frame
ushort multi_client_lookup_ref_obj_net_sig();

// Quick lookup for the most recently received frame
int multi_client_lookup_frame_idx();

// Quick lookup for the most recently received timestamp.
int multi_client_lookup_frame_timestamp();

// reset all the necessary info for respawning player.
void multi_oo_respawn_reset_info(object* objp);

// ---------------------------------------------------------------------------------------------------
// OBJECT UPDATE FUNCTIONS
//

// process all object update details for this frame
void multi_oo_process();

// process incoming object update data
void multi_oo_process_update(ubyte *data, header *hinfo);

// initialize all object update timestamps (call whenever entering gameplay state)
void multi_init_oo_and_ship_tracker();
// release memory allocated for object update
void multi_close_oo_and_ship_tracker();

// send control info for a client (which is basically a "reverse" object update)
void multi_oo_send_control_info();
void multi_oo_send_changed_object(object *changedobj);


// reset all sequencing info
void multi_oo_reset_sequencing();


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

// notify of a player join
void multi_oo_player_reset_all(net_player *pl = NULL);

// is this object one which needs to go through the interpolation
int multi_oo_is_interp_object(object *objp);

#endif

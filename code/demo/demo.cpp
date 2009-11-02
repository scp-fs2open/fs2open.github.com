/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "demo/demo.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "globalincs/linklist.h"
#include "freespace2/freespace.h"
#include "object/object.h"
#include "io/timer.h"
#include "gamesequence/gamesequence.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "cfile/cfile.h"



// -----------------------------------------------------------------------------------------------------------------------------
// DEMO DEFINES/VARS
//

CFILE *Demo_file = NULL;

// how often we dump
#define DEMO_DEFAULT_FPS				15
int Demo_fps = 1000 / DEMO_DEFAULT_FPS;

// timestamp for frame dumping
int Demo_stamp = -1;

// missiontime
float Demo_missiontime = 0.0f;

// buffer for reading and writing demo stuff
#define DEMO_BUF_SIZE					32768
char *Demo_buf = NULL;
int Demo_buf_pos = 0;

// # of events posted for this frame
int Demo_frame_events = 0;

// current offset into the demo file - only used for playback
int Demo_cur_offset = -1;

// demo version #
#define DEMO_VERSION						2

// an error reading or writing the demo file
int Demo_error = DEMO_ERROR_NONE;

// all strings read out of the demo file must be no longer than this
#define DEMO_STRING_LEN					255

// macros
#define DEMO_DATA_FRAME()				do { \
	if(Demo_file == NULL){\
		Int3();\
		DEMO_ERROR(DEMO_ERROR_GENERAL);\
		break;\
	}\
	if(Game_mode & GM_DEMO_RECORD){\
		if(Demo_buf_pos == 0) {\
			Int3();\
			break;\
		}\
		if(Demo_frame_events <= 0){\
			break;\
		}\
		if(!cfwrite_ushort((ushort)Demo_buf_pos, Demo_file)){\
			DEMO_ERROR(DEMO_ERROR_DISK_SPACE);\
			break;\
		}\
		if(!cfwrite(Demo_buf, Demo_buf_pos, 1, Demo_file)){\
			DEMO_ERROR(DEMO_ERROR_DISK_SPACE);\
			break;\
		}\
	} else if(Game_mode & GM_DEMO_PLAYBACK){\
		Demo_buf_pos = (int)cfread_ushort(Demo_file);\
		if(!cfread(Demo_buf, Demo_buf_pos, 1, Demo_file)){\
			DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);\
			break;\
		}\
	}\
 } while(0)
#define DEMO_DATA(vl, vl_size)		do { if(Demo_buf == NULL){ DEMO_ERROR(DEMO_ERROR_GENERAL); break; } if((Demo_buf_pos + vl_size) >= DEMO_BUF_SIZE){	Int3(); DEMO_ERROR(DEMO_ERROR_FRAMESIZE); break; } if(Game_mode & GM_DEMO_RECORD){ memcpy(Demo_buf + Demo_buf_pos, &vl, vl_size); } else if(Game_mode & GM_DEMO_PLAYBACK){ memcpy(&vl, Demo_buf + Demo_buf_pos, vl_size); } Demo_buf_pos += vl_size; } while(0)
#define DEMO_INT(vl)						do { DEMO_DATA(vl, sizeof(int)); } while(0)
#define DEMO_UINT(vl)					do { DEMO_DATA(vl, sizeof(uint)); } while(0)
#define DEMO_SHORT(vl)					do { DEMO_DATA(vl, sizeof(short)); } while(0)
#define DEMO_USHORT(vl)					do { DEMO_DATA(vl, sizeof(ushort)); } while(0)
#define DEMO_BYTE(vl)					do { DEMO_DATA(vl, sizeof(char)); } while(0)
#define DEMO_UBYTE(vl)					do { DEMO_DATA(vl, sizeof(ubyte)); } while(0)
#define DEMO_FLOAT(vl)					do { DEMO_DATA(vl, sizeof(float)); } while(0)
#define DEMO_VECTOR(vl)					do { DEMO_DATA(vl, sizeof(vec3d)); } while(0)
#define DEMO_MATRIX(vl)					do { DEMO_DATA(vl, sizeof(matrix)); } while(0)
#define DEMO_STRING(vl)					do { int stlen; if(Game_mode & GM_DEMO_RECORD){ stlen = strlen(vl); if(stlen <= 0){ break; }	DEMO_DATA(stlen, sizeof(ushort)); DEMO_DATA(*vl, strlen(vl)); } else { ushort len = 0; DEMO_USHORT(len); DEMO_DATA(*vl, len); vl[len] = '\0'; } } while(0)
		
// demo events types
#define DE_DUMP							1			// standard object dump
#define DE_TRAILER						2			// end of demo trailer
#define DE_PRIMARY						3			// primary weapon fired
#define DE_UNIQUE_MESSAGE				4			// unique hud message
#define DE_BUILTIN_MESSAGE				5			// builtin hud message
#define DE_OBJ_CREATE					6			// object create message
#define DE_OBJ_WARPIN					7			// ship warpin
#define DE_OBJ_WARPOUT					8			// ship warpout
#define DE_OBJ_DEPARTED					9			// ship departed
#define DE_SHIP_KILL						10			// ship kill

// call this when posting an error
#define DEMO_ERROR(er)					do { Demo_error = er; Int3(); } while(0)

int Demo_make = 0;
DCF(demo, "")
{
	Demo_make = !Demo_make;
	if(Demo_make){
		dc_printf("Demo will be recorded\n");
	} else {
		dc_printf("Demo will NOT be recorded\n");
	}
}


// -----------------------------------------------------------------------------------------------------------------------------
// DEMO FORWARD DECLARATIONS
//

// write demo header
int demo_write_header();

// read the demo header
int demo_read_header();

// write the demo trailer
void demo_write_trailer();

// do a recording frame
void demo_do_recording_frame_start();

// do a recording frame
void demo_do_recording_frame_end();

// do a playback frame
void demo_do_playback_frame();

// seek through the demo file to the proper location
// return 0 on error, 1 on success/continue, 2 if the demo is done
int demo_playback_seek();

// scan through a read in frame of data and apply everything necessary. returns true if the trailer (end of demo) was found
int demo_playback_seek_sub(int frame_size);


// -----------------------------------------------------------------------------------------------------------------------------
// DEMO FUNCTIONS
//

// do frame for the demo - playback and recording, returns 0 if errors were encountered during frame processing
int demo_do_frame_start()
{
#ifndef DEMO_SYSTEM
	return 1;
#else
	// if we're not doing any demo stuff
	if(!(Game_mode & GM_DEMO)){
		return 1;
	}

	// bad
	if(Demo_file == NULL){		
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// make sure we're not trying to record and playback at the same time
	Assert( ((Game_mode & GM_DEMO_RECORD) && !(Game_mode & GM_DEMO_PLAYBACK)) || (!(Game_mode & GM_DEMO_RECORD) && (Game_mode & GM_DEMO_PLAYBACK)) );

	// recording
	if(Game_mode & GM_DEMO_RECORD){		
		demo_do_recording_frame_start();
	} else {
		demo_do_playback_frame();
	}	

	// bad bad bad, get mwa
	if(Demo_error){
		return 0;
	}	

	// continue
	return 1;
#endif
}

// do frame for the demo - playback and recording, returns 0 if errors were encountered during frame processing
int demo_do_frame_end()
{
#ifndef DEMO_SYSTEM
	return 1;
#else
	// if we're not doing any demo stuff
	if(!(Game_mode & GM_DEMO)){
		return 1;
	}

	// bad
	if(Demo_file == NULL){		
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// make sure we're not trying to record and playback at the same time
	Assert( ((Game_mode & GM_DEMO_RECORD) && !(Game_mode & GM_DEMO_PLAYBACK)) || (!(Game_mode & GM_DEMO_RECORD) && (Game_mode & GM_DEMO_PLAYBACK)) );

	// recording. there's nothing to do here for playback
	if(Game_mode & GM_DEMO_RECORD){		
		demo_do_recording_frame_end();
	}

	// bad bad bad, get mwa
	if(Demo_error){
		return 0;
	}	

	// continue
	return 1;
#endif
}

// initialize a demo for recording
// NOTE : call this after loading the mission and going through the briefing, but _before_ physically moving into the mission
int demo_start_record(char *file)
{
#ifndef DEMO_SYSTEM
	return 0;
#else
	char full_name[MAX_FILENAME_LEN] = "";	

	// try and allocate the buffer
	Demo_buf = (char*)vm_malloc(DEMO_BUF_SIZE);
	if(Demo_buf == NULL){
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// open the outfile
	strcpy_s(full_name, file);
	cf_add_ext(full_name, ".fsd");
	Demo_file = cfopen(full_name, "wb", CFILE_NORMAL, CF_TYPE_DEMOS);
	if(Demo_file == NULL){
		Int3();
		Demo_error = DEMO_ERROR_DISK_ACCESS;
		return 0;
	}

	// no errors
	Demo_error = DEMO_ERROR_NONE;

	// write the header
	if(!demo_write_header()){
		return 0;
	}

	// flag demo mode
	Game_mode |= GM_DEMO_RECORD;

	// no events yet
	Demo_frame_events = 0;

	// success
	return 1;
#endif
}

// initialize a demo for playback - calling this will load up the demo file and move the player into the playback state
int demo_start_playback(char *file)
{
#ifndef DEMO_SYSTEM
	return 0;
#else
	char full_name[MAX_FILENAME_LEN] = "";

	// try and allocate the buffer
	Demo_buf = (char*)vm_malloc(DEMO_BUF_SIZE);
	if(Demo_buf == NULL){		
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// open the outfile
	strcpy_s(full_name, file);
	cf_add_ext(full_name, ".fsd");
	Demo_file = cfopen(full_name, "rb", CFILE_NORMAL, CF_TYPE_DEMOS);
	if(Demo_file == NULL){
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// no errors
	Demo_error = DEMO_ERROR_NONE;

	// read the header
	Demo_cur_offset = -1;
	if(!demo_read_header()){
		return 0;
	}

	// flag demo mode
	Game_mode |= GM_DEMO_PLAYBACK;	

	// everything is cool, so jump into the mission
	gameseq_post_event(GS_EVENT_ENTER_GAME);
	return 1;
#endif
}

// finish the demo
void demo_close()
{
#ifdef DEMO_SYSTEM
	// if we're recording, write the trailer
	if(Game_mode & GM_DEMO_RECORD){
		demo_write_trailer();
	}

	// close the demo file
	if(Demo_file != NULL){
		cfclose(Demo_file);
		Demo_file = NULL;
	}

	// free the buffer
	if(Demo_buf != NULL){
		vm_free(Demo_buf);
		Demo_buf = NULL;
	}

	// if we're playing back, go back to the main hall
	if(Game_mode & GM_DEMO_PLAYBACK){
		gameseq_post_event(GS_EVENT_MAIN_MENU);
	}

	// unflag demo mode
	Game_mode &= ~(GM_DEMO_RECORD | GM_DEMO_PLAYBACK);
#endif
}

// if we should run the simulation for this object, or let the demo system handle it
int demo_should_sim(object *objp)
{
#ifndef DEMO_SYSTEM
	return 1;
#else
	// always sim stuff in non-demo mode
	if(!(Game_mode & GM_DEMO)){
		return 1;
	}

	// don't sim ships or missiles
	if((objp->type == OBJ_SHIP) || ((objp->type == OBJ_WEAPON) && (objp->instance >= 0) && (Weapon_info[Weapons[objp->instance].weapon_info_index].subtype == WP_MISSILE))){
		return 0;
	}	

	// sim everything else
	return 1;
#endif
}


// -----------------------------------------------------------------------------------------------------------------------------
// DEMO RECORDING FUNCTIONS
//

// write demo header
int demo_write_header()
{
	uint chksum;
	char *full_name;

	// write demo version #
	if(!cfwrite_int(DEMO_VERSION, Demo_file)){
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// write mission filename
	if(!cfwrite_string_len(Game_current_mission_filename, Demo_file)){
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// write mission checksum
	full_name = cf_add_ext(Game_current_mission_filename, FS_MISSION_FILE_EXT);
	cf_chksum_long(full_name, &chksum);
	if(!cfwrite_int(chksum, Demo_file)){
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}

	// success
	return 1;
}

// write the demo trailer
void demo_write_trailer()
{
	// start a new chunk
	demo_do_recording_frame_start();

	// trailer event
	ubyte frame_type = DE_TRAILER;
	DEMO_UBYTE(frame_type);

	// 1 event
	Demo_frame_events++;

	// write frame data to disk
	DEMO_DATA_FRAME();
}

// start recording frame
void demo_do_recording_frame_start()
{
	// if we're not physically doing the mission
	if(gameseq_get_state() != GS_STATE_GAME_PLAY){
		return;
	}

	// add in frametime
	Demo_missiontime += flFrametime;

	// clear the buffer, set no events, and write the header
	Demo_buf_pos = 0;
	Demo_frame_events = 0;

	// missiontime
	float fl_time = f2fl(Missiontime);
	DEMO_FLOAT(fl_time);	
}

// end recording frame
void demo_do_recording_frame_end()
{
	// if we're not physically doing the mission
	if(gameseq_get_state() != GS_STATE_GAME_PLAY){
		return;
	}

	// if its time to dump objects (the last thing we might dump per frame)
	if((Demo_stamp == -1) || timestamp_elapsed(Demo_stamp)){
		// post an object dump event
		demo_POST_object_dump();

		// reset the stamp
		Demo_stamp = timestamp(Demo_fps);
	}	

	// write all accumulated frame data to disk if necessary
	DEMO_DATA_FRAME();	
}

// post an object dump event
void demo_POST_object_dump()
{
	ship_obj *sobjp;
	object *objp;
	ushort obj_count;	
	ubyte team;

	// object dump event
	ubyte event_type = DE_DUMP;
	DEMO_UBYTE(event_type);
	
	// go through the ship list and count
	obj_count = 0;	
	for ( sobjp = GET_FIRST(&Ship_obj_list); sobjp !=END_OF_LIST(&Ship_obj_list); sobjp = GET_NEXT(sobjp) ){
		// object pointer
		if(sobjp->objnum < 0){
			continue;
		}

		obj_count++;
	}

	// write out the object count
	DEMO_USHORT(obj_count);

	// go through the ship list and dump necessary stuff
	for ( sobjp = GET_FIRST(&Ship_obj_list); sobjp !=END_OF_LIST(&Ship_obj_list); sobjp = GET_NEXT(sobjp) ){
		// object pointer
		if(sobjp->objnum < 0){
			continue;
		}
		objp = &Objects[sobjp->objnum];

		// just ships for now		
		DEMO_INT(objp->signature);
		DEMO_VECTOR(objp->pos);
		DEMO_MATRIX(objp->orient);
		DEMO_FLOAT(objp->phys_info.forward_thrust);	
		team = (ubyte)Ships[objp->instance].team;
		DEMO_UBYTE(team);
		DEMO_INT(Ships[objp->instance].flags);
	}	

	// up the event count
	Demo_frame_events++;
}

// post a primary fired event
void demo_POST_primary_fired(object *objp, int banks, int linked)
{
	ubyte fire_info = 0;

	// object dump event
	ubyte event_type = DE_PRIMARY;
	DEMO_UBYTE(event_type);

	// object signature
	DEMO_INT(objp->signature);

	// get fire info
	fire_info = (ubyte)banks;
	fire_info &= ~(1<<7);
	if(linked){
		fire_info |= (1<<7);
	} 
	DEMO_UBYTE(fire_info);

	// up the event count
	Demo_frame_events++;
}

// post a unique message
void demo_POST_unique_message(char *id, char *who_from, int m_source, int priority)
{
	// sanity
	if((id == NULL) || (who_from == NULL) || (strlen(id) <= 0) || (strlen(who_from) <= 0)){
		return;
	}

	// write it
	ubyte event = DE_UNIQUE_MESSAGE;
	DEMO_UBYTE(event);
	DEMO_STRING(id);
	DEMO_STRING(who_from);
	DEMO_INT(m_source);
	DEMO_INT(priority);	

	// up the event count
	Demo_frame_events++;
}

// post a builtin message
void demo_POST_builtin_message(int type, ship *shipp, int priority, int timing)
{	
	int sig = 0;

	// write it
	ubyte event = DE_BUILTIN_MESSAGE;
	DEMO_UBYTE(event);
	DEMO_INT(type);
	if(shipp == NULL){
		sig = -1;
	} else if(shipp->objnum >= 0){
		sig = Objects[shipp->objnum].signature;
	}
	DEMO_INT(sig);
	DEMO_INT(priority);
	DEMO_INT(timing);

	// up the event count
	Demo_frame_events++;
}

// post an object create message
void demo_POST_obj_create(char *pobj_name, int signature)
{
	// write it
	ubyte event = DE_OBJ_CREATE;
	DEMO_UBYTE(event);
	DEMO_STRING(pobj_name);
	DEMO_INT(signature);
	
	// up the event count
	Demo_frame_events++;
}

// post a warpin event
void demo_POST_warpin(int signature, int ship_flags)
{
	// write it
	ubyte event = DE_OBJ_WARPIN;	
	DEMO_UBYTE(event);
	DEMO_INT(signature);
	DEMO_INT(ship_flags);
	
	// up the event count
	Demo_frame_events++;
}

// post a warpout event
void demo_POST_warpout(int signature, int ship_flags)
{
	// write it
	ubyte event = DE_OBJ_WARPOUT;
	DEMO_UBYTE(event);
	DEMO_INT(signature);
	DEMO_INT(ship_flags);
	
	// up the event count
	Demo_frame_events++;
}

// post a departed event
void demo_POST_departed(int signature, int ship_flags)
{
	// write it
	ubyte event = DE_OBJ_DEPARTED;	
	DEMO_UBYTE(event);
	DEMO_INT(signature);
	DEMO_INT(ship_flags);
	
	// up the event count
	Demo_frame_events++;
}

// post a ship kill event
void demo_POST_ship_kill(object *objp)
{
	// write it
	ubyte event = DE_SHIP_KILL;	
	DEMO_UBYTE(event);
	DEMO_INT(objp->signature);	
	
	// up the event count
	Demo_frame_events++;
}


// -----------------------------------------------------------------------------------------------------------------------------
// DEMO PLAYBACK FUNCTIONS
//

// read the demo header
int demo_read_header()
{
	int version;
	uint file_checksum, my_checksum;
	char *full_name;

	// read the version #
	version = cfread_int(Demo_file);
	if(version != DEMO_VERSION){
		DEMO_ERROR(DEMO_ERROR_VERSION);
		return 0;
	}

	// read mission filename
	cfread_string_len(Game_current_mission_filename, MAX_FILENAME_LEN, Demo_file);

	// get mission checksum
	file_checksum = cfread_int(Demo_file);	
	full_name = cf_add_ext(Game_current_mission_filename, FS_MISSION_FILE_EXT);
	if(!cf_chksum_long(full_name, &my_checksum)){
		DEMO_ERROR(DEMO_ERROR_DISK_ACCESS);
		return 0;
	}
	if(file_checksum != my_checksum){
		DEMO_ERROR(DEMO_ERROR_MISSION);
		return 0;
	}

	// get the file offset of this first frame
	Demo_cur_offset = cftell(Demo_file);

	// success
	return 1;
}

// do a playback frame
void demo_do_playback_frame()
{
	// seek to the best location in the demo file
	switch(demo_playback_seek()){
	// error
	case 0:
		return;

	// continue
	case 1:		
		break;

	// found the trailer - this demo is done
	case 2:
		demo_close();
		break;	
	}
}

// seek through the demo file to the proper location
// return 0 on error, 1 on success/continue, 2 if the demo is done
int demo_playback_seek()
{
	float this_time = 0.0f;
	int frame_size;
	int this_offset;
	int ret = 0;	

	// scan until we find something useful
	while(1){
		// record the beginning of this chunk
		this_offset = cftell(Demo_file);

		// read in the data for the next frame		
		DEMO_DATA_FRAME();
		frame_size = Demo_buf_pos;
		Demo_buf_pos = 0;

		// check the time		
		DEMO_FLOAT(this_time);

		// ahead of us
		if(this_time > f2fl(Missiontime)){
			// success
			ret = 1;

			// seek back to the beginning of this chunk
			Demo_cur_offset = this_offset;

			// bust out
			break;
		}
		// we should scan through this chunk
		else {
			// returns true if it finds the demo trailer
			if(demo_playback_seek_sub(frame_size)){
				return 2;
			}
		}
	}

	// seek back to the new frame
	cfseek(Demo_file, Demo_cur_offset, CF_SEEK_SET);

	// return
	return ret;
}

// scan through a read in frame of data and apply everything necessary. returns true if the trailer (end of demo) was found
int demo_playback_seek_sub(int frame_size)
{
	ubyte event = 0;
	int idx;	

	// apply everything
	while(Demo_buf_pos < frame_size){
		// next event
		DEMO_UBYTE(event);

		// process it
		switch(event){
		// oops, trailer. we're done
		case DE_TRAILER:
			return 1;

		// object dump
		case DE_DUMP: {
			ushort obj_count = 0;
			int obj_sig = 0;
			ubyte team = 0;
			vec3d obj_pos = vmd_zero_vector;
			matrix obj_orient = vmd_identity_matrix;
			float obj_fthrust = 0;
			int ship_index = 0;
			int ship_flags = 0;

			// get the object count
			DEMO_USHORT(obj_count);

			// read in all the objects
			for(idx=0; idx<obj_count; idx++){
				DEMO_INT(obj_sig);
				DEMO_VECTOR(obj_pos);
				DEMO_MATRIX(obj_orient);
				DEMO_FLOAT(obj_fthrust);
				DEMO_UBYTE(team);
				DEMO_INT(ship_flags);

				// find our ship object
				ship_index = ship_get_by_signature(obj_sig);
				// Assert(ship_index >= 0);
				if(ship_index >= 0){
					Objects[Ships[ship_index].objnum].pos = obj_pos;
					Objects[Ships[ship_index].objnum].orient = obj_orient;
					Objects[Ships[ship_index].objnum].phys_info.forward_thrust = obj_fthrust;
					Ships[ship_index].team = team;
					Ships[ship_index].flags = ship_flags;
				}
			}		
			}
			break;						  

		// primary fired
		case DE_PRIMARY: {
			int obj_sig = 0 ;
			ubyte fire_info = 0;
			int ship_index = -1;

			// get the data and ship
			DEMO_INT(obj_sig);
			DEMO_UBYTE(fire_info);
			ship_index = ship_get_by_signature(obj_sig);
			if(ship_index >= 0){
				Ships[ship_index].weapons.current_primary_bank = (int)(fire_info & ~(1<<7));
				if(fire_info & (1<<7)){
					Ships[ship_index].flags |= SF_PRIMARY_LINKED;
				} else {
					Ships[ship_index].flags &= ~(SF_PRIMARY_LINKED);
				}

				// fire
				ship_fire_primary(&Objects[Ships[ship_index].objnum], 0, 1);
			}
			}
			break;

		// unique message
		case DE_UNIQUE_MESSAGE: {
			char id[255] = "";
			char who_from[255] = "";
			int m_source = 0;
			int priority = 0;

			// read the stuff in
			DEMO_STRING(id);
			DEMO_STRING(who_from);
			DEMO_INT(m_source);
			DEMO_INT(priority);
			
			// send the message
			message_send_unique_to_player(id, who_from, m_source, priority, 0, 0);
			}
			break;

		// builtin message
		case DE_BUILTIN_MESSAGE:{
			int type = 0;
			int sig = 0;
			int ship_index = 0;
			int priority = 0;
			int timing = 0;

			// get data and ship
			DEMO_INT(type);
			DEMO_INT(sig);
			DEMO_INT(priority);
			DEMO_INT(timing);
			if(sig == -1){
				// message_send_builtin_to_player(type, NULL, priority, timing, 0, 0);
			} else {
				ship_index = ship_get_by_signature(sig);
				if(ship_index >= 0){
					// message_send_builtin_to_player(type, &Ships[ship_index], priority, timing, 0, 0);
				}
			}
			}
			break;

		// obj create
		case DE_OBJ_CREATE:{
			char pobj_name[255] = "";
			int obj_sig = 0;
			int objnum = 0;
			p_object *objp = NULL;

			// try and create the ship
			DEMO_STRING(pobj_name);
			DEMO_INT(obj_sig);
			
			objp = mission_parse_get_arrival_ship( pobj_name );
			if(objp != NULL){
				objnum = parse_create_object(objp);
				if(objnum >= 0){
					Objects[objnum].signature = obj_sig;
				}
			}
			}
			break;

		// warpin effect
		case DE_OBJ_WARPIN:{
			int obj_sig = 0;
			int ship_flags = 0;
			int ship_index = 0;

			// get the data and ship
			DEMO_INT(obj_sig);
			DEMO_INT(ship_flags);
			ship_index = ship_get_by_signature(obj_sig);
			if(ship_index >= 0){
				Ships[ship_index].flags = ship_flags;
				shipfx_warpin_start(&Objects[Ships[ship_index].objnum]);
			}
			}
			break;

		// warpout effect
		case DE_OBJ_WARPOUT:{
			int obj_sig = 0;
			int ship_flags = 0;
			int ship_index = 0;

			// get the data and ship
			DEMO_INT(obj_sig);
			DEMO_INT(ship_flags);
			ship_index = ship_get_by_signature(obj_sig);
			if(ship_index >= 0){
				Ships[ship_index].flags = ship_flags;
				shipfx_warpout_start(&Objects[Ships[ship_index].objnum]);
			}
			}
			break;

		// departed
		case DE_OBJ_DEPARTED:{
			int obj_sig = 0;
			int ship_flags = 0;
			int ship_index = 0;

			// get the data and ship
			DEMO_INT(obj_sig);
			DEMO_INT(ship_flags);
			ship_index = ship_get_by_signature(obj_sig);
			if(ship_index >= 0){
				Ships[ship_index].flags = ship_flags;
				ship_actually_depart(ship_index);
			}
			}
			break;

		// ship kill
		case DE_SHIP_KILL:{
			int obj_sig = 0;			
			int ship_index = 0;

			// get the data and ship
			DEMO_INT(obj_sig);			
			ship_index = ship_get_by_signature(obj_sig);
			if(ship_index >= 0){				
				Objects[Ships[ship_index].objnum].hull_strength = 0.0f;
				ship_generic_kill_stuff(&Objects[Ships[ship_index].objnum], 1.0f);
			}
			}
			break;

		default:
			Int3();
			return 0;
		}
	}

	return 0;
}

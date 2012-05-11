/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FS2_SUPERNOVA_FUN_HEADER_FILE
#define __FS2_SUPERNOVA_FUN_HEADER_FILE

// --------------------------------------------------------------------------------------------------------------------------
// SUPERNOVA DEFINES/VARS
//

struct vec3d;
struct matrix;

// supernova timing stuff
#define SUPERNOVA_MIN_TIME							15.0f				// must be at least 15 seconds out
#define SUPERNOVA_CUT_TIME							5.0f				// note this is also the minimum time for the supernova sexpression
#define SUPERNOVA_CAMERA_MOVE_TIME				2.0f				// this is the amount of time the camera will cut from the sun to the player
#define SUPERNOVA_FADE_TO_WHITE_TIME			1.0f				// fade to white over this amount of time

// how much bigger the sun will be when the effect hits
#define SUPERNOVA_SUN_SCALE						3.0f

// status for the supernova this mission
#define SUPERNOVA_NONE								0					// nothing happened in this mission
#define SUPERNOVA_STARTED							1					// started, but the player never got hit by it
#define SUPERNOVA_HIT								2					// started and killed the player
extern int Supernova_status;

// --------------------------------------------------------------------------------------------------------------------------
// SUPERNOVA FUNCTIONS
//

// level init
void supernova_level_init();

// start a supernova
void supernova_start(int seconds);

// stop a supernova
void supernova_stop();

// call once per frame
void supernova_process();

// is there a supernova active
// 0 : not active.
// 1 : player still in control. shockwave approaching.
// 2 : camera cut. player controls locked. letterbox
// 3 : tooltime. lots of particles
// 4 : player is effectively dead. fade to white. stop simulation
// 5 : give dead popup
int supernova_active();

// time left before the supernova hits
float supernova_time_left();

// pct complete the supernova (0.0 to 1.0)
float supernova_pct_complete();

// if the camera should cut to the "you-are-toast" cam
int supernova_camera_cut();

// get view params from supernova
void supernova_get_eye(vec3d *eye_pos, matrix *eye_orient);

#endif

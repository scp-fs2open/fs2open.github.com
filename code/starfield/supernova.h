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
constexpr float SUPERNOVA_CLOSE_TIME = 15.0f;							// must be at least 15 seconds out
constexpr float SUPERNOVA_HIT_TIME = 5.0f;								// note this is also the minimum time for the supernova sexpression
constexpr float SUPERNOVA_CAMERA_MOVE_DURATION = 2.0f;					// this is the amount of time the camera will cut from the sun to the player
constexpr float SUPERNOVA_FADE_TO_WHITE_DURATION = 1.0f;				// fade to white over this amount of time

// how much bigger the sun will be when the effect hits
constexpr float SUPERNOVA_SUN_SCALE = 3.0f;

// stages for the supernova this mission
enum class SUPERNOVA_STAGE
{
	NONE,																// not active.
	STARTED,															// player still in control. shockwave approaching.
	CLOSE,																// shockwave still approaching, but very close. sound1 has started
	HIT,																// camera cut. player controls locked. letterbox. sound2 has started. particles start
	TOOLTIME,															// tooltime. lots of particles. camera stops moving
	DEAD1,																// player is effectively dead. fade to white. stop simulation
	DEAD2,																// give dead popup
};

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
SUPERNOVA_STAGE supernova_stage();
bool supernova_active();

// time left before the supernova hits - for displaying on HUD
float supernova_hud_time_left();

// percent to complete the supernova (0.0 to 1.0)
// note: this covers total time, not time until the camera cuts
float supernova_pct_complete();

// special sunspot percent calculation (0.0 to 1.0)
float supernova_sunspot_pct();

// if the camera should cut to the "you-are-toast" cam
bool supernova_camera_cut();

// get view params from supernova
void supernova_get_eye(vec3d *eye_pos, matrix *eye_orient);

#endif

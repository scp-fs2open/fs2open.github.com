/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Starfield/Supernova.h $
 * $Revision: 2.2 $
 * $Date: 2005-04-05 05:53:25 $
 * $Author: taylor $
 *
 * Include file for nebula stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:35  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 3     9/03/99 1:32a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 2     7/21/99 8:10p Dave
 * First run of supernova effect.
 *  
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
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
void supernova_set_view(vec3d *eye_pos, matrix *eye_orient);

#endif

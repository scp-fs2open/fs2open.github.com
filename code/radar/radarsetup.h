/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Radar/radarsetup.h $
 * $Revision: 2.2 $
 * $Date: 2004-07-03 06:08:54 $
 * $Author: wmcoolmon $
 *
 * C module containg functions switch between radar modes
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/07/01 01:51:54  phreak
 * function pointer radar update.
 * will enable us to make different radar styles that we can switch between
 *
 *
 * $NoKeywords: $
 *
 */

#ifndef _FS2OPEN_RADARSETUP_H
#define _FS2OPEN_RADARSETUP_H

//which radar type are we using
//to add another radar type, begin by adding a RADAR_MODE_* define and increment MAX_RADAR_MODES
#define RADAR_MODE_STANDARD 1
#define RADAR_MODE_ORB 2
#define MAX_RADAR_MODES 3

//structures
#define BLIP_CURRENT_TARGET	(1<<0)
#define BLIP_DRAW_DIM			(1<<1)	// object is farther than Radar_dim_range units away
#define BLIP_DRAW_DISTORTED	(1<<2)	// object is resistant to sensors, so draw distorted

typedef struct blip	{
	blip	*prev, *next;
	int	x, y, rad;
	int	flags;	// BLIP_ flags defined above
} blip;

struct object;

//function pointers to radar functions.
//i'll explain everything that needs to be done if you want to add another radar type
extern int  (*radar_blip_color)(object *objp);
extern void (*radar_blip_draw_distorted)(blip *b);
extern void (*radar_blip_draw_flicker)(blip *b);
extern void (*radar_blit_gauge)();
extern void (*radar_draw_blips_sorted)(int distort);
extern void (*radar_draw_circle)( int x, int y, int rad );
extern void (*radar_draw_range)();
extern void (*radar_frame_init)();
extern void (*radar_frame_render)(float frametime);
extern void (*radar_init)();
extern void (*radar_mission_init)();
extern void (*radar_null_nblips)();
extern void (*radar_page_in)();
extern void (*radar_plot_object)( object *objp );


//selects the radar mode so you can switch between radar modes depending on command lines/mission requirements
void select_radar_mode(int radar_mode);

#endif //_FS2OPEN_RADARSETUP_H

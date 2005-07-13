/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionGrid.h $
 * $Revision: 2.4 $
 * $Date: 2005-07-13 03:25:59 $
 * $Author: Goober5000 $
 *
 * Type and defines for grids
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2005/04/05 05:53:19  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.2  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/05 09:02:06  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 6     10/03/97 8:55a John
 * moved Fred's grid_render code out of MissionGrid and into Fred.   Added
 * code to turn background under overlays grey.
 * 
 * 5     7/14/97 12:04a Lawrance
 * added function that navmap calls to draw elevation lines
 * 
 * 4     6/18/97 11:36p Lawrance
 * move grid rendering code to MissionGrid.cpp
 * 
 * 3     6/12/97 11:25a Lawrance
 * added grid_read_camera_controls()
 * 
 * 2     6/12/97 9:58a Lawrance
 * holds grid specific types and #defines
 * 
 * 1     6/12/97 9:51a Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __MISSIONGRID_H__
#define __MISSIONGRID_H__

#include "globalincs/pstypes.h"
#include "physics/physics.h"

#define	MAX_GRIDLINE_POINTS	201
#define	L_MAX_LINES				128

typedef struct grid {
	int		nrows, ncols;
	vec3d	center;
	matrix	gmatrix;
	physics_info	physics;
	float	square_size;
	float	planeD;		//	D component of plane equation (A, B, C are uvec in gmatrix)
	vec3d	gpoints1[MAX_GRIDLINE_POINTS];  // 1 -4 are edge gridpoints for small grid.
	vec3d	gpoints2[MAX_GRIDLINE_POINTS];
	vec3d	gpoints3[MAX_GRIDLINE_POINTS];
	vec3d	gpoints4[MAX_GRIDLINE_POINTS];
	vec3d	gpoints5[MAX_GRIDLINE_POINTS];  // 5-8 are edge gridpoints for large grid.
	vec3d	gpoints6[MAX_GRIDLINE_POINTS];
	vec3d	gpoints7[MAX_GRIDLINE_POINTS];
	vec3d	gpoints8[MAX_GRIDLINE_POINTS];
} grid;

typedef struct tline {
	int	istart, iend, color;
} tline;

extern grid Global_grid;
extern grid	*The_grid;
extern int	double_fine_gridlines;

void grid_read_camera_controls( control_info * ci, float frametime );
void maybe_create_new_grid(grid *gridp, vec3d *pos, matrix *orient, int force = 0);
grid *create_grid(grid *gridp, vec3d *forward, vec3d *right, vec3d *center, int nrows, int ncols, float square_size);
grid *create_default_grid(void);
void render_grid(grid *gridp);
void modify_grid(grid *gridp);
void rpd_line(vec3d *v0, vec3d *v1);
void grid_render_elevation_line(vec3d *pos, grid* gridp);

#endif

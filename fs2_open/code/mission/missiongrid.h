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
 * $Revision: 2.1 $
 * $Date: 2004-03-05 09:02:06 $
 * $Author: Goober5000 $
 *
 * Type and defines for grids
 *
 * $Log: not supported by cvs2svn $
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
	vector	center;
	matrix	gmatrix;
	physics_info	physics;
	float	square_size;
	float	planeD;		//	D component of plane equation (A, B, C are uvec in gmatrix)
	vector	gpoints1[MAX_GRIDLINE_POINTS];  // 1 -4 are edge gridpoints for small grid.
	vector	gpoints2[MAX_GRIDLINE_POINTS];
	vector	gpoints3[MAX_GRIDLINE_POINTS];
	vector	gpoints4[MAX_GRIDLINE_POINTS];
	vector	gpoints5[MAX_GRIDLINE_POINTS];  // 5-8 are edge gridpoints for large grid.
	vector	gpoints6[MAX_GRIDLINE_POINTS];
	vector	gpoints7[MAX_GRIDLINE_POINTS];
	vector	gpoints8[MAX_GRIDLINE_POINTS];
} grid;

typedef struct tline {
	int	istart, iend, color;
} tline;

extern grid Global_grid;
extern grid	*The_grid;
extern int	double_fine_gridlines;

void grid_read_camera_controls( control_info * ci, float frametime );
void maybe_create_new_grid(grid *gridp, vector *pos, matrix *orient, int force = 0);
grid *create_grid(grid *gridp, vector *forward, vector *right, vector *center, int nrows, int ncols, float square_size);
grid *create_default_grid(void);
void render_grid(grid *gridp);
void modify_grid(grid *gridp);
void rpd_line(vector *v0, vector *v1);
void grid_render_elevation_line(vector *pos, grid* gridp);

#endif

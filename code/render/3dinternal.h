/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Render/3dInternal.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:27 $
 * $Author: penguin $
 *
 * Used internally by the 3d renderer lib
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 5     3/16/98 4:51p John
 * Added low-level code to clip all polygons against an arbritary plane.
 * Took out all old model_interp_zclip and used this new method instead.  
 * 
 * 4     4/29/97 9:55a John
 * 
 * 3     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#ifndef _3DINTERNAL_H
#define _3DINTERNAL_H

#include "3d.h"

extern int Canvas_width,Canvas_height;	//the actual width & height
extern float Canv_w2,Canv_h2;			//fixed-point width,height/2

extern vector Window_scale;
extern int free_point_num;

extern float View_zoom;
extern vector View_position,Matrix_scale;
extern matrix View_matrix,Unscaled_matrix;


//vertex buffers for polygon drawing and clipping
extern vertex *Vbuf0[];
extern vertex *Vbuf1[];

extern void free_temp_point(vertex *p);
extern vertex **clip_polygon(vertex **src,vertex **dest,int *nv,ccodes *cc,uint flags);
extern void init_free_points(void);
extern void clip_line(vertex **p0,vertex **p1,ubyte codes_or, uint flags);

extern int G3_count;

extern int G3_user_clip;
extern vector G3_user_clip_normal;
extern vector G3_user_clip_point;

// Returns TRUE if point is behind user plane
extern int g3_point_behind_user_plane( vector *pnt );

#endif

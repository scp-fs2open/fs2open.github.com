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
 * $Revision: 2.5 $
 * $Date: 2006-03-18 10:23:46 $
 * $Author: taylor $
 *
 * Used internally by the 3d renderer lib
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2005/07/13 03:35:29  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.3  2005/04/05 05:53:24  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.2  2004/08/11 05:06:33  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
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

#include "render/3d.h"

extern int Canvas_width,Canvas_height;	//the actual width & height
extern float Canv_w2,Canv_h2;			//fixed-point width,height/2

extern vec3d Window_scale;
extern int free_point_num;

extern float View_zoom;
extern vec3d View_position,Matrix_scale;
extern matrix View_matrix,Unscaled_matrix;

extern void free_temp_point(vertex *p);
extern vertex **clip_polygon(vertex **src,vertex **dest,int *nv,ccodes *cc,uint flags);
extern void init_free_points(void);
extern void clip_line(vertex **p0,vertex **p1,ubyte codes_or, uint flags);

extern int G3_count;

extern int G3_user_clip;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;

// Returns TRUE if point is behind user plane
extern int g3_point_behind_user_plane( vec3d *pnt );

#endif

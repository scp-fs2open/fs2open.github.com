/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGL.h $
 * $Revision: 2.2 $
 * $Date: 2004-04-03 20:27:57 $
 * $Author: phreak $
 *
 * Include file for OpenGL renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2003/03/07 00:15:45  phreak
 * added some hacks that shutdown and restore opengl because of cutscenes
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 1     5/12/97 12:14p John
 *
 * $NoKeywords: $
 */

#ifndef _GROPENGL_H
#define _GROPENGL_H

#include <windows.h>
#include "globalincs/pstypes.h"

typedef enum gr_texture_source {
	TEXTURE_SOURCE_NONE=0,
	TEXTURE_SOURCE_DECAL,
	TEXTURE_SOURCE_NO_FILTERING,
	TEXTURE_SOURCE_MODULATE4X,
} gr_texture_source;

typedef enum gr_alpha_blend {
        ALPHA_BLEND_NONE=0,			// 1*SrcPixel + 0*DestPixel
        ALPHA_BLEND_ALPHA_ADDITIVE,             // Alpha*SrcPixel + 1*DestPixel
        ALPHA_BLEND_ALPHA_BLEND_ALPHA,          // Alpha*SrcPixel + (1-Alpha)*DestPixel
        ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR,      // Alpha*SrcPixel + (1-SrcPixel)*DestPixel
} gr_alpha_blend;

typedef enum gr_zbuffer_type {
        ZBUFFER_TYPE_NONE=0,
        ZBUFFER_TYPE_READ,
        ZBUFFER_TYPE_WRITE,
        ZBUFFER_TYPE_FULL,
} gr_zbuffer_type;

void gr_opengl_init(int reinit=0);
void gr_opengl_cleanup(int minimize=1);
void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler);
void gr_opengl_set_state(gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt);
void gr_opengl_bitmap(int x, int y);
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy);

extern int VBO_ENABLED;

#endif

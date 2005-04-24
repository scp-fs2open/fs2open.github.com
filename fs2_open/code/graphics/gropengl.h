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
 * $Revision: 2.9 $
 * $Date: 2005-04-24 12:56:42 $
 * $Author: taylor $
 *
 * Include file for OpenGL renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2005/03/24 23:42:21  taylor
 * s/gr_ogl_/gr_opengl_/g
 * add empty gr_opengl_draw_line_list() so that it's not a NULL pointer
 * make gr_opengl_draw_htl_sphere() just use GLU so we don't need yet another friggin API
 *
 * Revision 2.7  2005/03/20 00:09:07  phreak
 * Added gr_draw_htl_line and gr_draw_htl sphere
 * There still needs to be D3D versions implemented, but OGL is done.
 * Follow that or ask phreak about how its implemented/
 *
 * Revision 2.6  2005/02/23 05:11:13  taylor
 * more consolidation of various graphics variables
 * some header cleaning
 * only one tmapper_internal for OGL, don't use more than two tex/pass now
 * seperate out 2d matrix mode to allow -2d_poof in OGL and maybe fix missing
 *    interface when set 3d matrix stuff doesn't have corresponding end
 * add dump_frame stuff for OGL, mostly useless but allows trailer recording
 *
 * Revision 2.5  2005/01/01 11:24:23  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 2.4  2004/10/31 21:42:31  taylor
 * Linux tree merge, use linear mag filter, small FRED fix, AA lines (disabled), use rgba colors for 3dunlit, proper gamma adjustment, bmpman merge
 *
 * Revision 2.3  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/04/03 20:27:57  phreak
 * OpenGL files spilt up to make developing and finding bugs much easier
 *
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

#include "PreProcDefines.h"


#ifdef _WIN32
	#include <windows.h>

	#include "graphics/gl/gl.h"
	#include "graphics/gl/glu.h"
	#include "graphics/gl/glext.h"

	#define STUB_FUNCTION 0
#elif defined(SCP_UNIX)
	#define GL_GLEXT_PROTOTYPES

	#include "SDL.h"

#if ( SDL_VERSION_ATLEAST(1, 2, 7) )
	// this will include all needed GL headers for Win32, Unix & OSX
	#include "SDL_opengl.h"
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glext.h>
#endif // SDL_VERSION check
#endif

#include "globalincs/pstypes.h"
#include "graphics/grinternal.h"

const ubyte GL_zero_3ub[3] = { 0, 0, 0 };

#define CLAMP(x, min, max) do { if ( (x) < (min) ) (x) = (min); else if ((x) > (max)) (x) = (max); } while(0)


void gr_opengl_init(int reinit=0);
void gr_opengl_cleanup(int minimize=1);
void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler);
void gr_opengl_set_state(gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt);
void gr_opengl_bitmap(int x, int y);
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy);
void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage = 0 );
void opengl_reset_spec_mapping();

extern int VBO_ENABLED;

#endif

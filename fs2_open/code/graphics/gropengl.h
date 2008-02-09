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
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Include file for OpenGL renderer
 *
 * $Log: not supported by cvs2svn $
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

void gr_opengl_init();
void gr_opengl_cleanup();

void gr_opengl_bitmap(int x, int y);
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy);

#endif

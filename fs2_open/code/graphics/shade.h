/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Shade.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Header file for Shade.cpp
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 7     3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 6     3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 5     11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 4     11/18/96 1:48p Allender
 * added 16 bit version of (very slow) shader
 * 
 * 3     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#ifndef _SHADE_H
#define _SHADE_H

extern void grx_create_shader(shader * shade, float r, float g, float b, float c );
extern void grx_set_shader( shader * shade );
extern void gr8_shade(int x,int y,int w,int h);

#endif

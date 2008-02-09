/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrGlideInternal.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:22 $
 * $Author: penguin $
 *
 * Common include file for Glide modules
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 3     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 4     5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 3     4/09/98 4:38p John
 * Made non-darkening and transparent textures work under Glide.  Fixed
 * bug with Jim's computer not drawing any bitmaps.
 * 
 * 2     4/08/98 8:47a John
 * Moved all texture caching into a new module
 * 
 * 1     3/03/98 4:42p John
 * Added in Leighton's code to do texture caching on Glide.
 *
 * $NoKeywords: $
 */

#ifndef _GRGLIDEINTERNAL_H
#define _GRGLIDEINTERNAL_H

#include "grinternal.h"

void glide_tcache_init();
void glide_tcache_cleanup();
void glide_tcache_flush();
void glide_tcache_frame();

// Bitmap_type see TCACHE_ defines in GrInternal.h
int glide_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0);

extern int Glide_textures_in;

#endif //_GRGLIDEINTERNAL_H

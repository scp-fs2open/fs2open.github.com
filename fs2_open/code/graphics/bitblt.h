/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Bitblt.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:57 $
 * $Author: penguin $
 *
 * Include file for software bitblt type stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 2     4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 1     3/25/98 8:07p John
 * Split software renderer into Win32 and DirectX
 *
 * $NoKeywords: $
 */

#ifndef _BITBLT_H
#define _BITBLT_H

void grx_bitmap(int x,int y);
void grx_bitmap_ex(int x,int y,int w,int h,int sx,int sy);
void grx_aabitmap(int x,int y);
void grx_aabitmap_ex(int x,int y,int w,int h,int sx,int sy);

#endif //_BITBLT_H

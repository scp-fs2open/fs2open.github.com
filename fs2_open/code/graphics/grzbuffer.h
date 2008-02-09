/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrZbuffer.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Include for software render zbuffering
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
 * 1     3/25/98 8:07p John
 * Split software renderer into Win32 and DirectX
 *
 * $NoKeywords: $
 */

#ifndef _GRZBUFFER_H
#define _GRZBUFFER_H

// Z-buffer stuff
extern uint *gr_zbuffer;
extern uint gr_zbuffer_offset;	// Add this to pixel location to get zbuffer location
extern int gr_zoffset;				// add this to w before interpolation

extern int gr_zbuffering, gr_zbuffering_mode;
extern int gr_global_zbuffering;				

#define GR_Z_RANGE 0x400000		//(2^31)/GR_Z_COUNT
#define GR_Z_COUNT 500				// How many frames between zbuffer clear.
											// The bigger, the less precise.

// If mode is FALSE, turn zbuffer off the entire frame,
// no matter what people pass to gr_zbuffer_set.
void gr8_zbuffer_clear(int mode);
int gr8_zbuffer_get();
int gr8_zbuffer_set(int mode);


#endif //_GRZBUFFER_H

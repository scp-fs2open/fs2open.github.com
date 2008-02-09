/*
 * Copyright (C) Mike Harris, 2002.  Released under the LGPL.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Io/Mouse.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:06 $
 * $Author: penguin $
 *
 * Low-level X mouse routines.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:10:40  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/21 15:43:23  mharris
 * Added SDL mouse support
 *
 * Revision 1.1  2002/05/17 03:04:09  mharris
 * Make mouse routines more portable
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "io/mouse.h"
#include "SDL.h"


void getWindowMousePos(POINT * pt)
{
	Assert(pt != NULL);
	SDL_GetMouseState(&pt->x, &pt->y);
//  	mprintf(("mouse at (%3d, %3d)\n", pt->x, pt->y));
}


void setWindowMousePos(POINT * pt)
{
	Assert(pt != NULL);
	// TODO
}

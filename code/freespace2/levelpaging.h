/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FREESPACE2/LevelPaging.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 02:50:52 $
 * $Author: Goober5000 $
 *
 * Code to page in all the bitmaps at the beginning of a level.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:22  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 1     3/26/98 5:14p John
 *
 * $NoKeywords: $
 */

#ifndef _LEVELPAGING_H
#define _LEVELPAGING_H

// Call this and it calls the page in code for all the subsystems
void level_page_in();

#endif	//_LEVELPAGING_H


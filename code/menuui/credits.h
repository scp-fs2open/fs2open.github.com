/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/Credits.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * C header file for displaying game credits
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 3     8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 2     4/22/97 11:06a Lawrance
 * credits music playing, credits screen is a separate state
 *
 * $NoKeywords: $
 */

#ifndef __CREDITS_H__
#define __CREDITS_H__

void credits_init();
void credits_do_frame(float frametime);
void credits_close();

#endif /* __CREDITS_H__ */

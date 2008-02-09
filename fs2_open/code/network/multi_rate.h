/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_rate.h $
 * $Revision: 2.2 $
 * $Date: 2004-08-11 05:06:29 $
 * $Author: Kazan $
 * 
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/04/03 06:22:32  Goober5000
 * fixed some stub functions and a bunch of compile warnings
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     3/09/99 6:24p Dave
 * More work on object update revamping. Identified several sources of
 * unnecessary bandwidth.
 *  
 *   
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _FS2_MULTI_DATA_RATE_HEADER_FILE
#define _FS2_MULTI_DATA_RATE_HEADER_FILE

// keep this defined to compile in rate checking
#if !defined(NDEBUG) || defined(MULTIPLAYER_BETA_BUILD) || defined(FS2_DEMO)
	#define MULTI_RATE
#endif

// -----------------------------------------------------------------------------------------------------------------------
// MULTI RATE DEFINES/VARS
//

#define MAX_RATE_TYPE_LEN			50				// max length of a type string
#define MAX_RATE_PLAYERS			12				// how many player we'll keep track of
#define MAX_RATE_TYPES				32				// how many types we'll keep track of per player

// -----------------------------------------------------------------------------------------------------------------------
// MULTI RATE FUNCTIONS
//
#ifdef MULTI_RATE

// notify of a player join
void multi_rate_reset(int np_index);

// add data of the specified type to datarate processing, returns 0 on fail (if we ran out of types, etc, etc)
int multi_rate_add(int np_index, char *type, int size);

// process. call _before_ doing network operations each frame
void multi_rate_process();

// display
void multi_rate_display(int np_index, int x, int y);

#else

// stubs using #defines (c.f. NO_SOUND)
#define multi_rate_reset(np_index) ((void) (np_index))
#define multi_rate_add(np_index, type, size) ((np_index), (type), (size), 0)
#define multi_rate_process()
#define multi_rate_display(np_index, x, y) ((void) ((np_index), (x), (y)))

#endif



#endif	// header define

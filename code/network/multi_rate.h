/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FS2_MULTI_DATA_RATE_HEADER_FILE
#define _FS2_MULTI_DATA_RATE_HEADER_FILE

// keep this defined to compile in rate checking
#if !defined(NDEBUG)
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
#define multi_rate_reset(np_index)
#define multi_rate_add(np_index, type, size) 	do { } while (0)
#define multi_rate_process()
#define multi_rate_display(np_index, x, y)

#endif



#endif	// header define

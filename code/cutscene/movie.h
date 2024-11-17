/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MOVIE_HEADER_FILE
#define _MOVIE_HEADER_FILE

namespace movie {

bool play(const char* filename, bool via_tech_room = false);

void play_two(const char* filename1, const char* filename2);

}

#endif

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

extern const char *movie_ext_list[];
extern const int NUM_MOVIE_EXT;

bool movie_play(char *name);
void movie_play_two(char *name1, char *name2);

#endif

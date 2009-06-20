/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MAIN_HALL_MENU_HEADER_FILE
#define _MAIN_HALL_MENU_HEADER_FILE

// the # of main halls we're supporting
#define MAIN_HALLS_MAX			10		// Goober5000 - bumped down to 10; don't go above 256 (size of ubyte)

// initialize the main hall proper 
void main_hall_init(int main_hall_num);

// do a frame for the main hall
void main_hall_do(float frametime);

// close the main hall proper
void main_hall_close();

// start the main hall music playing
void main_hall_start_music();

// stop the main hall music
void main_hall_stop_music();

// get the music index
int main_hall_get_music_index(int main_hall_num);

// what main hall we're on
int main_hall_id();

// Vasudan?
int main_hall_is_vasudan();

// start the ambient sounds playing in the main hall
void main_hall_start_ambient();
void main_hall_stop_ambient();
void main_hall_reset_ambient_vol();

void main_hall_do_multi_ready();

// make the vasudan main hall funny
void main_hall_vasudan_funny();

void main_hall_pause();
void main_hall_unpause();

#endif

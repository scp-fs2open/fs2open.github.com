/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _OPTIONS_MENU_MULTI_HEADER_FILE
#define _OPTIONS_MENU_MULTI_HEADER_FILE

// This file is basically just a sister module to the OptionsMenu file

class UI_WINDOW;

// called when the options screen is initialized, pass in the UI window
void options_multi_init(UI_WINDOW *options_window);

// do frame for the multi options screen
void options_multi_do(int key);

// called when the entire options screen is closed (note - does not do any settings updates. this is purely for ui shutdown)
void options_multi_close();

// called if the accept button on the main options screen was hit
bool options_multi_accept();

// called when the multiplayer tab is hit - initializes/switches all necessary data.
// NOTE : this is different from the initialization function, which is called only when the options menu is started
void options_multi_select();

// called when the multiplayer tab has been switched from
void options_multi_unselect();

// return the bitmap handle of the current background bitmap, or -1 if the multiplayer tab is not active
int options_multi_background_bitmap();

// set voice sound buffer for display 
void options_multi_set_voice_data(unsigned char *sound_buf, int buf_size, double gain);

// process and blit any voice waveform if necessary
void options_multi_vox_process_waveform();

// return whether we want to eat a tabbed keypress
int options_multi_eat_tab();

#endif

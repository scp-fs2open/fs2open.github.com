/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/OptionsMenuMulti.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:59 $
 * $Author: penguin $
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
 * 3     5/06/98 8:06p Dave
 * Made standalone reset properly under weird conditions. Tweak
 * optionsmulti screen. Upped MAX_WEAPONS to 350. Put in new launch
 * countdown anim. Minro ui fixes/tweaks.
 * 
 * 2     4/17/98 5:27p Dave
 * More work on the multi options screen. Fixed many minor ui todo bugs.
 * 
 * 1     4/16/98 11:39p Dave
 *  
 * 
 * $NoKeywords: $
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
void options_multi_accept();

// called when the multiplayer tab is hit - initializes/switches all necessary data.
// NOTE : this is different from the initialization function, which is called only when the options menu is started
void options_multi_select();

// called when the multiplayer tab has been switched from
void options_multi_unselect();

// return the bitmap handle of the current background bitmap, or -1 if the multiplayer tab is not active
int options_multi_background_bitmap();

// set voice sound buffer for display 
void options_multi_set_voice_data(unsigned char *sound_buf,int buf_size,unsigned char *comp_buf, int comp_size, int uncomp_size, double gain);

// process and blit any voice waveform if necessary
void options_multi_vox_process_waveform();

// return whether we want to eat a tabbed keypress
int options_multi_eat_tab();

#endif

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/OptionsMenu.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:24 $
 * $Author: penguin $
 *
 * Header file for code that controls the Options menu
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     6/25/99 11:59a Dave
 * Multi options screen.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 10    1/28/98 6:21p Dave
 * Made the standalone use ~8 megs less memory. Fixed multiplayer submenu
 * endgame problem.
 * 
 * 9     12/27/97 8:07p Lawrance
 * remove old code
 * 
 * 8     8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 7     1/09/97 12:57p Lawrance
 * supporting a new state where the player picks to either save or restore
 * 
 * 6     11/21/96 7:14p Lawrance
 * converted menu code to use a file (menu.tbl) to get the data for the
 * menu
 * 
 * 5     11/13/96 4:02p Lawrance
 * complete over-haul of the menu system and the states associated with
 * them
 * 
 * 4     11/13/96 8:32a Lawrance
 * streamlined menu code
 * 
 * 3     11/06/96 8:54a Lawrance
 * added revision templates, made more efficient
 *
 * $NoKeywords: $
 *
*/

#ifndef _OPTIONSMENU_H
#define _OPTIONSMENU_H

#include "ui.h"

struct op_sliders {
	// base slider
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	int dot_w;
	int dots;

	// left and right buttons
	char *left_filename;
	int left_mask, left_x, left_y;
	char *right_filename;
	int right_mask, right_x, right_y;

	// slider control
	UI_DOT_SLIDER_NEW slider;  // because we have a class inside this struct, we need the constructor below..

	op_sliders(char *name, int x1, int y1, int xt1, int yt1, int h, int _dot_w, int _dots, char *_left_filename, int _left_mask, int _left_x, int _left_y, char *_right_filename, int _right_mask, int _right_x, int _right_y) : 
				 filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), dot_w(_dot_w), dots(_dots), left_filename(_left_filename), left_mask(_left_mask), left_x(_left_x), left_y(_left_y), right_filename(_right_filename), right_mask(_right_mask), right_x(_right_x), right_y(_right_y) {}
};

void options_menu_init();
void options_menu_close();
void options_menu_do_frame(float frametime);

// kill the options menu
void options_cancel_exit();

#endif

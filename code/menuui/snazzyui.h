/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/SnazzyUI.h $
 * $Revision: 2.2 $
 * $Date: 2004-08-11 05:06:27 $
 * $Author: Kazan $
 *
 *  Header file for the Snazzy User Interface routines.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 17    4/09/98 5:51p Lawrance
 * Be able to disable sounds for certain menus
 * 
 * 16    2/03/98 11:52p Lawrance
 * call snazzy_flush() from game_flush()
 * 
 * 15    12/23/97 5:28p Hoffoss
 * Made enter key act the same as clicking the mouse button in main hall
 * screen.
 * 
 * 14    9/07/97 10:06p Lawrance
 * let snazzy code keep track of mouse status
 * 
 * 13    8/11/97 9:48p Lawrance
 * don't poll keyboard if not requested 
 * 
 * 12    2/25/97 11:11a Lawrance
 * adding more functionality needed for ship selection screen
 * 
 * 11    12/10/96 4:18p Lawrance
 * added snazzy_menu_close() call and integrated with existing menus
 * 
 * 10    12/09/96 2:53p Lawrance
 * fixed bug where both the snazzy code and ui code were reading the
 * keyboard, and the keypress from the snazzy code was being lost
 * 
 * 9     11/21/96 7:14p Lawrance
 * converted menu code to use a file (menu.tbl) to get the data for the
 * menu
 * 
 * 8     11/15/96 12:09p John
 * Added new UI code.  Made mouse not return Enter when you click it.
 * Changed the doSnazzyUI function and names to be snazzy_menu_xxx.   
 * 
 * 7     11/13/96 4:02p Lawrance
 * complete over-haul of the menu system and the states associated with
 * them
 * 
 * 6     11/13/96 8:32a Lawrance
 * streamlined menu code
 * 
 * 5     11/06/96 8:54a Lawrance
 * added revision templates, made more efficient
 *
 * $NoKeywords: $
 *
*/

#include "PreProcDefines.h"
#ifndef _SNAZZYUI_H
#define _SNAZZYUI_H

#define MAX_CHAR		150
#define ESC_PRESSED	-2

#include "globalincs/pstypes.h"

typedef struct menu_region {
	int 	mask;					// mask color for the region
	int	key;					// shortcut key for the region
	char	text[MAX_CHAR];	// The text associated with this item.
	int	click_sound;		// Id of sound to play when mask area clicked on
} MENU_REGION;

// These are the actions thare are returned in the action parameter.  
#define SNAZZY_OVER			1	// mouse is over a region
#define SNAZZY_CLICKED		2	// mouse button has gone from down to up over a region

int snazzy_menu_do(ubyte *data, int mask_w, int mask_h, int num_regions, MENU_REGION *regions, int *action, int poll_key = 1, int *key = NULL);
void read_menu_tbl(char *menu_name, char *bkg_filename, char *mask_filename, MENU_REGION *regions, int* num_regions, int play_sound=1);
void snazzy_menu_add_region(MENU_REGION *region, char* text, int mask, int key, int click_sound = -1);

void snazzy_menu_init();		// Call the first time a snazzy menu is inited
void snazzy_menu_close();
void snazzy_flush();

#endif

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



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

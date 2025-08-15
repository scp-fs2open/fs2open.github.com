/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _BARRACKS_H
#define _BARRACKS_H

#include "playerman/player.h"
#include "ui/ui.h"

extern int Barracks_overlay_id;

// initialize the barracks 
void barracks_init();

// do a frame for the barrracks
void barracks_do_frame(float frametime);

// close the barracks
void barracks_close();

void barracks_accept_pilot(player* plr, bool changeState);

// convenient struct for handling all button controls
struct barracks_buttons {
	const char *filename;
	int x, y;
	int text_x, text_y;	// this is where the text label is
	int hotspot;
	int repeat;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	barracks_buttons(const char *name, int x1, int y1, int x2, int y2, int h, int r = 0) : filename(name), x(x1), y(y1), text_x(x2), text_y(y2), hotspot(h), repeat(r) {}
};

#endif // _BARRACKS_H

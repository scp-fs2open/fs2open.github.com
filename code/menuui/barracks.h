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

extern int Barracks_overlay_id;

// initialize the barracks 
void barracks_init();

// do a frame for the barrracks
void barracks_do_frame(float frametime);

// close the barracks
void barracks_close();

#endif // _BARRACKS_H

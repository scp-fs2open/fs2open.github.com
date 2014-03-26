/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FREESPACE2_SHIP_CONTRAIL_HEADER_FILE
#define _FREESPACE2_SHIP_CONTRAIL_HEADER_FILE

// ----------------------------------------------------------------------------------------------
// CONTRAIL DEFINES/VARS
//

// prototypes
class ship;

// ----------------------------------------------------------------------------------------------
// CONTRAIL FUNCTIONS
//

// call during level initialization
void ct_level_init();

// call during level shutdown
void ct_level_close();

// call when a ship is created to initialize its contrail stuff
void ct_ship_create(ship *shipp);

// call when a ship is deleted to free up its contrail stuff
void ct_ship_delete(ship *shipp);

// call each frame for processing a ship's contrails
void ct_ship_process(ship *shipp);

// determine if the ship has AB trails
int ct_has_ABtrails(ship *shipp);

// update active ABtrails - moving existing ones, adding new ones where necessary
void ct_update_ABtrails(ship *shipp);

// create new ABtrails
void ct_create_ABtrails(ship *shipp);

#endif

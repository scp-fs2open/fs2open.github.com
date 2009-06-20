/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "freespace2/freespace.h"
#include "freespace2/levelpaging.h"


// All the page in functions
extern void ship_page_in();
extern void debris_page_in();
extern void particle_page_in();
extern void stars_page_in();
extern void hud_page_in();
extern void (*radar_page_in)();
extern void weapons_page_in();
extern void fireballs_page_in();
extern void shockwave_page_in();
extern void shield_hit_page_in();
extern void asteroid_page_in();
extern void training_mission_page_in();
extern void neb2_page_in();
extern void message_pagein_mission_messages();
extern void model_page_in_stop();
extern void mflash_page_in(bool);

// Pages in all the texutures for the currently
// loaded mission.  Call game_busy() occasionally...
void level_page_in()
{

	// moved to freespace.cpp on 2005/04/18 - taylor
//	mprintf(( "Beginning level bitmap paging...\n" ));
//
//	if(!(Game_mode & GM_STANDALONE_SERVER)){		
//		bm_page_in_start();
//	}

	// Most important ones first
	game_busy( NOX("*** paging in ships ***") );
	ship_page_in();
	//Must be called after paging in ships
	game_busy( NOX("*** paging in weapons ***") );
	weapons_page_in();
	game_busy( NOX("*** paging in various effects ***") );
	fireballs_page_in();
	particle_page_in();
	debris_page_in();
	hud_page_in();
	radar_page_in();
	training_mission_page_in();
	stars_page_in();
	shockwave_page_in();
	shield_hit_page_in();
	asteroid_page_in();
	neb2_page_in();
	mflash_page_in(false);  // just so long as it happens after weapons_page_in()

	// preload mission messages if NOT running low-memory (greater than 48MB)
	if (game_using_low_mem() == false) {
		message_pagein_mission_messages();
	}

	if(!(Game_mode & GM_STANDALONE_SERVER)){
		model_page_in_stop();		// free any loaded models that aren't used
		bm_page_in_stop();
	}

	mprintf(( "Ending level bitmap paging...\n" ));

}

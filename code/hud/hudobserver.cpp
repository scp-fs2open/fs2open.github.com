/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "hud/hudconfig.h"
#include "hud/hudobserver.h"
#include "hud/hudtargetbox.h"
#include "network/multi.h"
#include "ship/ship.h"


// use these to redirect Player_ship and Player_ai when switching into ai mode
ship Hud_obs_ship;
ai_info Hud_obs_ai;

/**
 * Initialize observer hud stuff
 */
void hud_observer_init(ship *shipp, ai_info *aip)
{
	// setup the pseduo ship and ai
	Hud_obs_ai = *aip;
	// (we used to do a memcpy here, but that doesn't work any longer, so let's just assign the values we need)
	Hud_obs_ship.clear();
	strcpy_s(Hud_obs_ship.ship_name, shipp->ship_name);
	Hud_obs_ship.display_name = shipp->display_name;
	Hud_obs_ship.team = shipp->team;
	Hud_obs_ship.ai_index = shipp->ai_index;
	Hud_obs_ship.flags = shipp->flags;
	Hud_obs_ship.ship_info_index = shipp->ship_info_index;
	Hud_obs_ship.objnum = shipp->objnum;
	Hud_obs_ship.wingnum = shipp->wingnum;
	Hud_obs_ship.alt_type_index = shipp->alt_type_index;
	Hud_obs_ship.callsign_index = shipp->callsign_index;
	Hud_obs_ship.ship_max_hull_strength = shipp->ship_max_hull_strength;
	Hud_obs_ship.ship_max_shield_strength = shipp->ship_max_shield_strength;
	Hud_obs_ship.weapons = shipp->weapons;

	HUD_config.is_observer = 1;
	HUD_config.show_flags = HUD_observer_default_flags;
	HUD_config.show_flags2 = HUD_observer_default_flags2;

	HUD_config.popup_flags = 0x0;
	HUD_config.popup_flags2 = 0x0;

	// shutdown any playing static animations
	hud_init_target_static();
}

void hud_obs_render_player(int  /*loc*/,net_player * /*pl*/)
{
}

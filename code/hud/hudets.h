/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUD_ETS_H
#define _HUD_ETS_H

struct object;

#define	ETS_RECHARGE_RATE	4.0f			//	Recharge this percent of total shields/second

extern float Energy_levels[];
extern int Weapon_energy_cheat;


enum SYSTEM_TYPE {WEAPONS, SHIELDS, ENGINES};

void update_ets(object* obj, float fl_frametime);
void ets_init_ship(object* obj);
void ai_manage_ets(object* obj);

void hud_init_ets();
void hud_show_ets();

void increase_recharge_rate(object* obj, SYSTEM_TYPE enum_value);
void decrease_recharge_rate(object* obj, SYSTEM_TYPE enum_value);
void set_default_recharge_rates(object* obj);

void transfer_energy_to_shields(object* obj);
void transfer_energy_to_weapons(object* obj);


#endif

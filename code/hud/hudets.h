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

#include "hud/hud.h"

class object;

#define ENERGY_DIVERT_DELTA				0.2f	// percentage of energy transferred in a shield->weapon or weapon->shield energy transfer
#define INTIAL_SHIELD_RECHARGE_INDEX	4		// default shield charge rate (index in Energy_levels[])
#define INTIAL_WEAPON_RECHARGE_INDEX	4		// default weapon charge rate (index in Energy_levels[])
#define INTIAL_ENGINE_RECHARGE_INDEX	4		// default engine charge rate (index in Energy_levels[])

#define NUM_ENERGY_LEVELS	13
#define MAX_ENERGY_INDEX	(NUM_ENERGY_LEVELS - 1)

#define AI_MODIFY_ETS_INTERVAL 500	// time between ets modifications for ai's (in milliseconds)

#define ZERO_INDEX			0
#define ONE_THIRD_INDEX		4
#define ONE_HALF_INDEX		6
#define ALL_INDEX			12

#define HAS_ENGINES			(1<<0)
#define HAS_SHIELDS			(1<<1)
#define HAS_WEAPONS			(1<<2)

#define	ETS_RECHARGE_RATE	4.0f			//	Recharge this percent of total shields/second

const int num_retail_ets_gauges = 3;

extern float Energy_levels[];
extern int Weapon_energy_cheat;

enum SYSTEM_TYPE {WEAPONS, SHIELDS, ENGINES};

void update_ets(object* obj, float fl_frametime);
void ets_init_ship(object* obj);
void ai_manage_ets(object* obj);

void increase_recharge_rate(object* obj, SYSTEM_TYPE enum_value);
void decrease_recharge_rate(object* obj, SYSTEM_TYPE enum_value);
void set_default_recharge_rates(object* obj);

void transfer_energy_to_shields(object* obj);
void transfer_energy_to_weapons(object* obj);

float ets_get_max_speed(object* objp, float engine_energy);
void sanity_check_ets_inputs(int (&ets_indexes)[num_retail_ets_gauges]);
bool validate_ship_ets_indxes(const int &ship_idx, int (&ets_indexes)[num_retail_ets_gauges]);
void zero_one_ets (int *reduce, int *add1, int *add2);

class HudGaugeEts: public HudGauge // HUD_ETS_GAUGE
{
protected:
	hud_frames Ets_bar;
	int System_type;

	char Letter;
	int	Letter_offsets[2];
	int	Top_offsets[2];
	int	Bottom_offsets[2];

	int ETS_bar_h;
public:
	HudGaugeEts();
	HudGaugeEts(int _gauge_object, int _system_type);
	void initLetterOffsets(int _x, int _y);
	void initTopOffsets(int _x, int _y);
	void initBottomOffsets(int _x, int _y);
	void initLetter(char _letter);	// obligatory PC Load Letter joke. (Swifty)
	void initBarHeight(int _ets_bar_h);
	void initBitmaps(char *fname);
	virtual void blitGauge(int index);
	virtual void render(float frametime);
	void pageIn();
};

class HudGaugeEtsWeapons: public HudGaugeEts
{
public:
	HudGaugeEtsWeapons();
	void render(float frametime);
};

class HudGaugeEtsShields: public HudGaugeEts
{
public:
	HudGaugeEtsShields();
	void render(float frametime);
};

class HudGaugeEtsEngines: public HudGaugeEts
{
public:
	HudGaugeEtsEngines();
	void render(float frametime);
};

class HudGaugeEtsRetail: public HudGaugeEts
{
protected:
	char Letters[num_retail_ets_gauges];
	int Gauge_positions[num_retail_ets_gauges];
public:
	HudGaugeEtsRetail();
	void render(float frametime);
	void initLetters(char *_letters);
	void initGaugePositions(int *_gauge_positions);
};

#endif

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

struct object;

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

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FREESPACE_HUDSHIELD_H__
#define __FREESPACE_HUDSHIELD_H__

#include "globalincs/pstypes.h"
#include "hud/hud.h"

#define SHIELD_HIT_DURATION	1400	// time a shield quadrant flashes after being hit
#define SHIELD_FLASH_INTERVAL	200	// time between shield quadrant flashes

#define NUM_SHIELD_HIT_MEMBERS	5
#define HULL_HIT_OFFSET				4		// used to access the members in shield_hit_info that pertain to the hull
typedef struct shield_hit_info
{
	int shield_hit_status;		// bitfield, if offset for shield quadrant is set, that means shield is being hit
	int shield_show_bright;		// bitfield, if offset for shield quadrant is set, that means play bright frame
	int shield_hit_timers[NUM_SHIELD_HIT_MEMBERS];	// timestamps that get set for SHIELD_FLASH_TIME when a quadrant is hit
	int shield_hit_next_flash[NUM_SHIELD_HIT_MEMBERS];
} shield_hit_info;

extern ubyte Quadrant_xlate[4];

struct player;
struct object;
struct ship_info;

//Not needed, info is taken from ships.tbl. Dunno why Volition did it this way in the first place.
//void hud_shield_game_init();
void hud_shield_level_init();
void hud_shield_equalize(object *objp, player *pl);
void hud_augment_shield_quadrant(object *objp, int direction);
void hud_shield_assign_info(ship_info *sip, char *filename);
void hud_show_mini_ship_integrity(object *objp, int force_x = -1, int force_y = -1);
void hud_shield_show_mini(object *objp, int x_force = -1, int y_force = -1, int x_hull_offset = 0, int y_hull_offset = 0);
void hud_shield_hit_update();
void hud_shield_quadrant_hit(object *objp, int quadrant);
void hud_shield_hit_reset(int player=0);

void shield_info_reset(shield_hit_info *shi);

// random page in stuff - moved here by Goober5000
extern void hud_ship_icon_page_in(ship_info *sip);

class HudGaugeShield: public HudGauge
{
protected:
public:
	HudGaugeShield();
	HudGaugeShield(int _gauge_object, int _gauge_config);
	virtual void render(float frametime);
	void showShields(object *objp);
	int maybeFlashShield(int target_index, int shield_offset);
};

class HudGaugeShieldPlayer: public HudGaugeShield
{
protected:
public:
	HudGaugeShieldPlayer();
	void render(float frametime);
};

class HudGaugeShieldTarget: public HudGaugeShield
{
protected:
public:
	HudGaugeShieldTarget();
	void render(float frametime);
};

class HudGaugeShieldMini: public HudGauge
{
protected:
	hud_frames Shield_mini_gauge;

	int Mini_3digit_offsets[2];
	int Mini_1digit_offsets[2];
	int Mini_2digit_offsets[2];
public:
	HudGaugeShieldMini();
	void initBitmaps(char *fname);
	void init3DigitOffsets(int x, int y);
	void init1DigitOffsets(int x, int y);
	void init2DigitOffsets(int x, int y);
	int maybeFlashShield(int target_index, int shield_offset);
	void showMiniShields(object *objp);
	void showIntegrity(float p_target_integrity);
	void render(float frametime);
	void pageIn();
};
#endif /* __FREESPACE_HUDSHIELDBOX_H__ */

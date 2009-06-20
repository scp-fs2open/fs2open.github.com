/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __HUD_COMMON_H__
#define __HUD_COMMON_H__

// HUD gauge types
#define NUM_HUD_GAUGES							39

#define HUD_LEAD_INDICATOR						0
#define HUD_ORIENTATION_TEE					1
#define HUD_HOSTILE_TRIANGLE					2
#define HUD_TARGET_TRIANGLE					3
#define HUD_MISSION_TIME						4
#define HUD_RETICLE_CIRCLE						5
#define HUD_THROTTLE_GAUGE						6
#define HUD_RADAR									7
#define HUD_TARGET_MONITOR						8
#define HUD_CENTER_RETICLE						9
#define HUD_TARGET_MONITOR_EXTRA_DATA		10
#define HUD_TARGET_SHIELD_ICON				11
#define HUD_PLAYER_SHIELD_ICON				12
#define HUD_ETS_GAUGE							13
#define HUD_AUTO_TARGET							14
#define HUD_AUTO_SPEED							15
#define HUD_WEAPONS_GAUGE						16
#define HUD_ESCORT_VIEW							17
#define HUD_DIRECTIVES_VIEW					18
#define HUD_THREAT_GAUGE						19
#define HUD_AFTERBURNER_ENERGY				20
#define HUD_WEAPONS_ENERGY						21
#define HUD_WEAPON_LINKING_GAUGE				22
#define HUD_TARGET_MINI_ICON					23
#define HUD_OFFSCREEN_INDICATOR				24
#define HUD_TALKING_HEAD						25
#define HUD_DAMAGE_GAUGE						26
#define HUD_MESSAGE_LINES						27
#define HUD_MISSILE_WARNING_ARROW			28
#define HUD_CMEASURE_GAUGE						29
#define HUD_OBJECTIVES_NOTIFY_GAUGE			30
#define HUD_WINGMEN_STATUS						31
#define HUD_OFFSCREEN_RANGE					32
#define HUD_KILLS_GAUGE							33
#define HUD_ATTACKING_TARGET_COUNT			34
#define HUD_TEXT_FLASH							35					// (formerly split up among emp, collision, etc)
#define HUD_MESSAGE_BOX							36
#define HUD_SUPPORT_GAUGE						37
#define HUD_LAG_GAUGE							38

extern char *HUD_gauge_text[NUM_HUD_GAUGES];					// defined in sexp.cpp!!!!


#endif	/* __HUD_COMMON_H__ */

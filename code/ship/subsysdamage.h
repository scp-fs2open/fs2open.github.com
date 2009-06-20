/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __SUBSYS_DAMAGE_H__
#define __SUBSYS_DAMAGE_H__

/////////////////////////////////////////
// engines
/////////////////////////////////////////
#define SHIP_MIN_ENGINES_FOR_FULL_SPEED	0.5f	// % engine strength below which ships start slowing down
#define SHIP_MIN_ENGINES_TO_WARP		0.3f	// % engine strength required to engage warp
#define ENGINE_MIN_STR					0.15f	// if engines are below this level, still contribute this percent to total
												// (unless destroyed, then contribute none).

/////////////////////////////////////////
// weapons
/////////////////////////////////////////
#define SUBSYS_WEAPONS_STR_FIRE_OK		0.7f	// 70% strength or better, weapons always fire
#define SUBSYS_WEAPONS_STR_FIRE_FAIL	0.2f	// below 20%, weapons will not fire


/////////////////////////////////////////
// sensors - targeting
/////////////////////////////////////////
#define SENSOR_STR_TARGET_NO_EFFECTS	0.3f	// % strength of sensors at which no negative effects on targeting
#define MIN_SENSOR_STR_TO_TARGET		0.2f	// % strength of sensors at which targeting ceases
												// to function

/////////////////////////////////////////
// sensors - radar
/////////////////////////////////////////
#define SENSOR_STR_RADAR_NO_EFFECTS		0.4f	// % strength of sensors at which no negative effects on radar
#define MIN_SENSOR_STR_TO_RADAR			0.1f	// % strength of sensors at which radar ceases to function


/////////////////////////////////////////
// communications
/////////////////////////////////////////
#define MIN_COMM_STR_TO_MESSAGE			0.3f	// % strength of communications at which player
												// is unable to use squadmate messaging
#define COMM_DESTROYED	0
#define COMM_DAMAGED		1
#define COMM_OK			2


/////////////////////////////////////////
// navigation
/////////////////////////////////////////
#define SHIP_MIN_NAV_TO_WARP			0.3f	// % navigation strength required to engage warp

/////////////////////////////////////////
// shields
/////////////////////////////////////////
#define MIN_SHIELDS_FOR_FULL_STRENGTH	0.5f	// % shield subsystem strength below which shield becomes less effective
#define MIN_SHIELDS_FOR_FULL_COVERAGE	0.3f	// % shield subsystem strength below which shield starts flickering

#endif

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __FREESPACE_EMP_MISSILE_HEADER_FILE_
#define __FREESPACE_EMP_MISSILE_HEADER_FILE_

// ----------------------------------------------------------------------------------------------------
// EMP EFFECT DEFINES/VARS
//
struct vec3d;

// default EMP effect values for weapons which do not specify them
// NOTE : anything aboce intensity max or time max will be capped
#define EMP_INTENSITY_MAX						(500.0f)
#define EMP_TIME_MAX								(30.0f)
#define EMP_DEFAULT_INTENSITY					(300.0f)
#define EMP_DEFAULT_TIME						(10.0f)

// for identifying specific text gauges when messing text up
#define NUM_TEXT_STAMPS			36
#define EG_NULL					-1				// meaning, always make the string empty
#define EG_WEAPON_TITLE			0				// title bar of the weapon gauge
#define EG_WEAPON_P1				1				// first primary weapon slot
#define EG_WEAPON_P2				2				// second primary weapon slot
#define EG_WEAPON_P3				3				// third primary weapon slot
#define EG_WEAPON_S1				4				// first secondary weapon slot
#define EG_WEAPON_S2				5				// second secondary weapon slot
#define EG_ESCORT1				6				// first item on escort list
#define EG_ESCORT2				7				// second item on escort list
#define EG_ESCORT3				8				// third item on escort list
#define EG_OBJ_TITLE				9				// titlebar of the directives gauge
#define EG_OBJ1					10				// line 1 of the directives display
#define EG_OBJ2					11				// line 2
#define EG_OBJ3					12				// line 3
#define EG_OBJ4					13				// line 4
#define EG_OBJ5					14				// line 5
#define EG_TBOX_EXTRA1			15				// extra target info line 1
#define EG_TBOX_EXTRA2			16				// extra target info line 2
#define EG_TBOX_EXTRA3			17				// extra target info line 3
#define EG_TBOX_CLASS			18				// target class
#define EG_TBOX_DIST				20				// target dist
#define EG_TBOX_SPEED			21				// target speed
#define EG_TBOX_CARGO			22				// target cargo
#define EG_TBOX_HULL				23				// target hull
#define EG_TBOX_NAME				24				// target name
#define EG_TBOX_INTEG			25				// target integrity
#define EG_SQ1						26				// squadmsg 1
#define EG_SQ2						27				// squadmsg 2
#define EG_SQ3						28				// squadmsg 3
#define EG_SQ4						29				// squadmsg 4
#define EG_SQ5						30				// squadmsg 5
#define EG_SQ6						31				// squadmsg 6
#define EG_SQ7						32				// squadmsg 7
#define EG_SQ8						33				// squadmsg 8
#define EG_SQ9						34				// squadmsg 9
#define EG_SQ10					35				// squadmsg 10

struct object;
struct ship;
struct weapon_info;


// ----------------------------------------------------------------------------------------------------
// EMP EFFECT FUNCTIONS
//

// initialize the EMP effect for the mission
void emp_level_init();

// apply the EMP effect to all relevant ships
void emp_apply(vec3d *pos, float inner_radius, float outer_radius, float emp_intensity, float emp_time, bool use_emp_time_for_capship_turrets = false);

// start the emp effect for the passed ship (setup lightning arcs, timestamp, etc)
// NOTE : if this ship is also me, I should call emp_start_local() as well
void emp_start_ship(object *ship_obj, float intensity, float time);

// process a ship for this frame
void emp_process_ship(ship *shipp);

// start the emp effect for MYSELF (intensity == arbitrary intensity variable, time == time the effect will last)
// NOTE : time should be in seconds
void emp_start_local(float intensity, float time);

// stop the emp effect cold
void emp_stop_local();

// if the EMP effect is active
int emp_active_local();

// process some stuff every frame (before frame is rendered)
void emp_process_local();

// randomly say yes or no to a gauge, if emp is not active, always say yes
int emp_should_blit_gauge();

// emp hud string
void emp_hud_string(int x, int y, int gauge_id, char *str, bool resize);

// emp hud printf
void emp_hud_printf(int x, int y, int gauge_id, char *format, ...);

// throw some jitter into HUD x and y coords
void emp_hud_jitter(int *x, int *y);

// current intensity of the EMP effect (0.0 - 1.0)
float emp_current_intensity();

#endif

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Emp.h $
 * $Revision: 2.2 $
 * $Date: 2004-08-11 05:06:36 $
 * $Author: Kazan $
 *
 * Header file for managing corkscrew missiles
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     7/02/99 4:31p Dave
 * Much more sophisticated lightning support.
 * 
 * 3     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 3     8/28/98 3:29p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 2     8/25/98 1:49p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 1     8/24/98 9:29a Dave
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __FREESPACE_EMP_MISSILE_HEADER_FILE_
#define __FREESPACE_EMP_MISSILE_HEADER_FILE_

// ----------------------------------------------------------------------------------------------------
// EMP EFFECT DEFINES/VARS
//
struct vector;

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
void emp_apply(vector *pos, float inner_radius, float outer_radius, float emp_intensity, float emp_time);

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
void emp_hud_string(int x, int y, int gauge_id, char *str);

// emp hud printf
void emp_hud_printf(int x, int y, int gauge_id, char *format, ...);

// throw some jitter into HUD x and y coords
void emp_hud_jitter(int *x, int *y);

// current intensity of the EMP effect (0.0 - 1.0)
float emp_current_intensity();

#endif

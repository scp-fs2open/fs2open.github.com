/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDgauges.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * HUD data common to FRED and FreeSpace
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     8/19/99 9:20a Andsager
 * Enable flashing for all guages
 * 
 * 5     8/16/99 4:04p Dave
 * Big honking checkin.
 * 
 * 4     7/30/99 10:31p Dave
 * Added comm menu to the configurable hud files.
 * 
 * 3     7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 2     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 1     10/12/98 1:53p Dave
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 10    5/08/98 10:16a Lawrance
 * Add new "ship attacking count" gauge
 * 
 * 9     4/05/98 7:42p Lawrance
 * Add kills gauge
 * 
 * 8     3/26/98 5:45p Lawrance
 * Added new gauges to HUD config
 * 
 * 7     1/23/98 6:26p Lawrance
 * Added wingman status gauge
 * 
 * 6     1/17/98 1:30a Lawrance
 * Add countermeasure gauge
 * 
 * 5     1/12/98 11:16p Lawrance
 * Wonderful HUD config.
 * 
 * 4     1/05/98 9:38p Lawrance
 * Implement flashing HUD gauges.
 * 
 * 3     1/05/98 4:24p Allender
 * added sexpression to flash a hud gauge -- a training only operator
 * 
 * 2     1/05/98 3:18p Lawrance
 * Common HUD data for FRED and FreeSpace
 * 
 * 1     1/05/98 3:07p Lawrance
 *
 * $NoKeywords: $
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

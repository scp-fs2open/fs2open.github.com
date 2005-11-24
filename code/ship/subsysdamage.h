/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/SubsysDamage.h $
 * $Revision: 2.3 $
 * $Date: 2005-11-24 08:46:10 $
 * $Author: Goober5000 $
 *
 * Header file for various subystem damage defines
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.1  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 4     4/07/98 5:30p Lawrance
 * Player can't send/receive messages when comm is destroyed.  Garble
 * messages when comm is damaged.
 * 
 * 3     10/13/97 7:42p Lawrance
 * added MIN_COMM_STR_RECEIVE_OK
 * 
 * 2     10/11/97 6:38p Lawrance
 * move subsys damage #defines to SubsysDamage.h
 * 
 * 1     10/10/97 7:57p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __SUBSYS_DAMAGE_H__
#define __SUBSYS_DAMAGE_H__

/////////////////////////////////////////
// engines
/////////////////////////////////////////
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


#endif

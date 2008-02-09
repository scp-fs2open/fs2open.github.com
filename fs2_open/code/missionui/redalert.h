/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/RedAlert.h $
 * $Revision: 2.2 $
 * $Date: 2004-03-05 09:01:55 $
 * $Author: Goober5000 $
 *
 * Header file for Red Alert mission interface and code
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     9/06/99 6:38p Dave
 * Improved CD detection code.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 7     5/05/98 6:19p Lawrance
 * Fix problems with "retry mission" for red alerts
 * 
 * 6     5/04/98 6:06p Lawrance
 * Make red alert mode work!
 * 
 * 5     3/28/98 2:53p Allender
 * added hud gauge when entering a red alert mission
 * 
 * 4     3/09/98 4:30p Allender
 * multiplayer secondary weapon changes.  red-alert and cargo-known-delay
 * sexpressions.  Add time cargo revealed to ship structure
 * 
 * 3     3/09/98 4:23p Lawrance
 * Replay mission, full save/restore of wingman status
 * 
 * 2     3/09/98 12:13a Lawrance
 * Add support for Red Alert missions
 * 
 * 1     3/08/98 4:54p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __REDALERT_H__
#define __REDALERT_H__

struct CFILE;

void	red_alert_start_mission();

void	red_alert_init();
void	red_alert_close();
void	red_alert_do_frame(float frametime);
int	red_alert_mission();
void	red_alert_set_status(int status);
int	red_alert_check_status();

void red_alert_store_wingman_status();
void red_alert_bash_wingman_status();
void red_alert_write_wingman_status(CFILE *fp);
void red_alert_read_wingman_status(CFILE *fp, int version);

#endif

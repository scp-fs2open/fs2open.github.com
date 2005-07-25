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
 * $Revision: 2.7 $
 * $Date: 2005-07-25 05:24:16 $
 * $Author: Goober5000 $
 *
 * Header file for Red Alert mission interface and code
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2005/07/13 03:25:58  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.5  2005/06/03 06:39:26  taylor
 * better audio pause/unpause support when game window loses focus or is minimized
 *
 * Revision 2.4  2004/10/31 21:53:24  taylor
 * new pilot code support, no-multiplayer and compiler warning fixes, center mouse cursor for redalert missions
 *
 * Revision 2.3  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:01:55  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
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

#include "globalincs/globals.h"

struct CFILE;

void	red_alert_start_mission();

void	red_alert_init();
void	red_alert_close();
void	red_alert_do_frame(float frametime);
int	red_alert_mission();
void	red_alert_invalidate_timestamp();
int	red_alert_check_status();

void red_alert_store_wingman_status();
void red_alert_bash_wingman_status();
void red_alert_write_wingman_status(CFILE *fp);
void red_alert_read_wingman_status(CFILE *fp, int version);

// campaign savefile versions
void red_alert_write_wingman_status_campaign(CFILE *fp);
void red_alert_read_wingman_status_campaign(CFILE *fp, char ships[][NAME_LENGTH], char weapons[][NAME_LENGTH]);

void red_alert_voice_pause();
void red_alert_voice_unpause();

#endif

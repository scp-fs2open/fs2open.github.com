/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDWingmanStatus.h $
 * $Revision: 2.1 $
 * $Date: 2004-08-11 05:06:25 $
 * $Author: Kazan $
 *
 * Header for the wingman status gauge
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 4     3/31/98 11:46p Lawrance
 * Fix several bugs related to departing ships.
 * 
 * 3     3/22/98 11:13p Allender
 * work with respawning -- save parse object so ship can be correctly
 * restored.  Restore wingman status information so gauge updates
 * correctly
 * 
 * 2     3/14/98 4:59p Lawrance
 * Totally rework HUD wingman status gauge to work with 5 arbitrary wings
 * 
 * 1     3/14/98 8:23a Lawrance
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef __HUDWINGMAN_STATUS_H__
#define __HUDWINGMAN_STATUS_H__

void	hud_init_wingman_status_gauge();
void	hud_wingman_status_update();
void	hud_wingman_status_render();
void	hud_wingman_status_init_flash();
int	hud_wingman_status_maybe_flash(int wing_index, int wing_pos);
void	hud_set_wingman_status_dead(int wing_index, int wing_pos);
void	hud_set_wingman_status_departed(int wing_index, int wing_pos);
void	hud_set_wingman_status_alive( int wing_index, int wing_pos);
void	hud_set_wingman_status_none( int wing_index, int wing_pos);
void	hud_wingman_status_start_flash(int wing_index, int wing_pos);
void	hud_wingman_status_set_index(int shipnum);

#endif

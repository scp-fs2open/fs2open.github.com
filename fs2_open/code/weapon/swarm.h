/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Swarm.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:29 $
 * $Author: penguin $
 *
 * Header file for managing swarm missiles
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 4     1/29/99 2:25p Andsager
 * Added turret_swarm_missiles
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 3     2/26/98 10:08p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 2     8/10/97 6:16p Lawrance
 * split off swarm missile code into a separate file
 *
 * $NoKeywords: $
 */


#ifndef __FREESPACE_SWARM_H__
#define __FREESPACE_SWARM_H__

#include "object.h"
#include "cfile.h"
#include "ship.h"

#define SWARM_DEFAULT_NUM_MISSILES_FIRED					4		// number of swarm missiles that launch when fired

void	swarm_level_init();
void	swarm_delete(int index);
int	swarm_create();
void	swarm_update_direction(object *objp, float frametime);
void	swarm_maybe_fire_missile(int shipnum);

int	turret_swarm_create();
void	turret_swarm_delete(int i);
void	turret_swarm_set_up_info(int parent_objnum, ship_subsys *turret, int turret_weapon_class);
void	turret_swarm_check_validity();

#endif /* __FREESPACE_SWARM_H__ */

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
 * $Revision: 2.5 $
 * $Date: 2005-07-13 03:35:30 $
 * $Author: Goober5000 $
 *
 * Header file for managing swarm missiles
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2005/03/25 06:57:38  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.3  2004/08/11 05:06:36  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
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

struct object;
struct ship_subsys;

typedef struct turret_swarm_info {
	int				flags;
	int				weapon_class;
	int				num_to_launch;
	int				parent_objnum;
	int				parent_sig;
	int				target_objnum;
	int				target_sig;
	ship_subsys*	turret;
	ship_subsys*	target_subsys;
	int				time_to_fire;
	int				weapon_num;
} turret_swarm_info;

#define SWARM_DEFAULT_NUM_MISSILES_FIRED					4		// number of swarm missiles that launch when fired

void	swarm_level_init();
void	swarm_delete(int index);
int	swarm_create();
void	swarm_update_direction(object *objp, float frametime);
void	swarm_maybe_fire_missile(int shipnum);

int	turret_swarm_create();
void	turret_swarm_delete(int i);
void	turret_swarm_set_up_info(int parent_objnum, ship_subsys *turret, struct weapon_info *wip, int weapon_num);
void	turret_swarm_check_validity();

#endif /* __FREESPACE_SWARM_H__ */

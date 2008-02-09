/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDescort.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * Header file for managing and displaying ships that are in an escort
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     7/30/99 7:01p Dave
 * Dogfight escort gauge. Fixed up laser rendering in Glide.
 * 
 * 5     5/24/99 11:28a Dave
 * Sexpression for adding/removing ships from the hud escort list.
 * 
 * 4     3/04/99 9:22a Andsager
 * Make escort list work with ship-is-visible.  When not visible, dump,
 * when becoming visible, maybe add.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 9     4/30/98 3:32p Lawrance
 * Cull dead/departed ships from escort ship in hud_update_frame()
 * 
 * 8     3/02/98 11:31p Lawrance
 * create functions to access ships on escort list
 * 
 * 7     2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 6     1/20/98 4:45p Allender
 * made HUD escorts which arrive late show up on list
 * 
 * 5     11/24/97 10:20p Lawrance
 * Add key 'KEY_N' to target next ship on monitoring view
 * 
 * 4     11/18/97 5:58p Lawrance
 * flash escort view info when that ship is taking hits
 * 
 * 3     11/13/97 10:46p Lawrance
 * implemented new escort view, damage view and weapons
 * 
 * 2     8/19/97 11:46p Lawrance
 * adding new hud gauges for shileds, escort view, and weapons
 * 
 * 1     8/19/97 3:13p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __FREESPACE_HUDESCORT_VIEW_H__
#define __FREESPACE_HUDESCORT_VIEW_H__

void	hud_escort_init();
void	hud_setup_escort_list(int level = 1);
void	hud_display_escort();
void	hud_escort_view_toggle();
void	hud_add_remove_ship_escort(int objnum, int supress_feedback = 0);
void	hud_escort_clear_all();
void	hud_escort_ship_hit(object *objp, int quadrant);
void	hud_escort_target_next();
void	hud_escort_cull_list();
void	hud_add_ship_to_escort(int objnum, int supress_feedback);
void  hud_remove_ship_from_escort(int objnum);
int	hud_escort_num_ships_on_list();
int	hud_escort_return_objnum(int index);
void	hud_escort_add_player(short id);
void	hud_escort_remove_player(short id);

#endif /* __FREESPACE_HUDESCORT_VIEW_H__ */

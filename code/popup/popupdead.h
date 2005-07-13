/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Popup/PopupDead.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:35:35 $
 * $Author: Goober5000 $
 *
 * Header for the death popup
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:33  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 3     4/22/98 4:59p Allender
 * new multiplayer dead popup.  big changes to the comm menu system for
 * team vs. team.  Start of debriefing stuff for team vs. team  Make form
 * on my wing work with individual ships who have high priority orders
 * 
 * 2     2/10/98 11:20p Lawrance
 * Implement separate dead popup system
 * 
 * 1     2/10/98 6:02p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __POPUPDEAD_H__
#define __POPUPDEAD_H__

// return values for popup_do_frame for multiplayer
#define POPUPDEAD_DO_RESPAWN		0
#define POPUPDEAD_DO_OBSERVER		1
#define POPUPDEAD_DO_MAIN_HALL	2

void	popupdead_start();
void	popupdead_close();
int	popupdead_do_frame(float frametime);
int	popupdead_is_active();

#endif

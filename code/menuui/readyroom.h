/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/ReadyRoom.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:26:00 $
 * $Author: Goober5000 $
 *
 * Ready Room code, which is the UI screen for selecting Campaign/mission to play next mainly.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:27  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 5     4/02/98 5:40p Hoffoss
 * Added the Load Mission screen to FreeSpace.
 * 
 * 4     3/02/98 5:22p Hoffoss
 * Removed ready room and added campaign room.
 * 
 * 3     3/02/98 3:39p Hoffoss
 * Added new Campaign Room screen.
 * 
 * 2     11/16/97 1:11p Hoffoss
 * Coded up readyroom screen, first pass.
 * 
 * 1     11/15/97 7:30p Hoffoss
 *
 * $NoKeywords: $
 */

void sim_room_init();
void sim_room_close();
void sim_room_do_frame(float frametime);

// called by main menu to continue on with current campaign (if there is one).
int readyroom_continue_campaign();

void campaign_room_init();
void campaign_room_close();
void campaign_room_do_frame(float frametime);

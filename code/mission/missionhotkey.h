/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionHotKey.h $
 * $Revision: 2.2 $
 * $Date: 2005-07-13 03:25:59 $
 * $Author: Goober5000 $
 *
 * Header file for the Hotkey selection screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 9     1/28/98 6:23p Dave
 * Made standalone use ~8 megs less memory. Fixed multiplayer submenu
 * sequencing problem.
 * 
 * 8     1/26/98 4:42p Allender
 * fixed restoration of hotkeys when replaying mission.  Change the
 * meaning of "departed wing" to mean anytime a wing "departs" (with any
 * number of remaining wingmen).
 * 
 * 7     1/14/98 5:22p Allender
 * save/restore hotkey selections when replaying the same mission
 * 
 * 6     8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 5     5/30/97 1:00p Allender
 * aded F11 and F12 to hotkey selection -- F12 requies that
 * UserDebuggerKey in registry be changed!!!
 * 
 * 4     4/30/97 11:34a Lawrance
 * making ship selection and hotkey assignment work in all cases
 * 
 * 3     4/28/97 5:43p Lawrance
 * allow hotkey assignment screen to work from ship selection
 * 
 * 2     4/25/97 3:41p Lawrance
 * added support for hotkey assignment screen
 *
 * $NoKeywords: $
 */

#ifndef __MISSIONHOTKEY_H__
#define __MISSIONHOTKEY_H__

void mission_hotkey_init();
void mission_hotkey_close();
void mission_hotkey_do_frame(float frametime);
void mission_hotkey_set_defaults();
void mission_hotkey_validate();
void mission_hotkey_maybe_save_sets();
void mission_hotkey_reset_saved();
void mission_hotkey_mf_add( int set, int objnum, int how_to_add );

void mission_hotkey_exit();

// function to return the hotkey set number of the given key
extern int mission_hotkey_get_set_num( int k );

#endif

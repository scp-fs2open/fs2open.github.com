/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Io/KeyControl.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * Header file for managing keyboard/joystick/mouse button presses
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 6     4/23/98 1:28a Dave
 * Seemingly nailed the current_primary_bank and current_secondary_bank -1
 * problem. Made sure non-critical button presses are _never_ sent to the
 * server.
 * 
 * 5     3/12/98 4:02p Lawrance
 * Cleanup how pause works, fix couple pause-related bugs.
 * 
 * 4     1/27/98 5:54p Lawrance
 * extern Dead key set, so popups can use it.
 * 
 * 3     10/21/97 7:06p Hoffoss
 * Overhauled the key/joystick control structure and usage throughout the
 * entire FreeSpace code.  The whole system is very different now.
 * 
 * 2     9/15/97 11:38p Lawrance
 * redo how game actions are recorded from player input and executed
 * 
 * 1     9/15/97 1:23p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __FREESPACE_KEYCONTROL_H__
#define __FREESPACE_KEYCONTROL_H__

#include "controlsconfig.h"

// Holds the bit arrays that indicate which action is to be executed.
#define NUM_BUTTON_FIELDS	((CCFG_MAX + 31) / 32)

extern int Dead_key_set[];
extern int Dead_key_set_size;

typedef struct button_info
{
	int status[NUM_BUTTON_FIELDS];
} button_info;

void button_info_set(button_info *bi, int n);
void button_info_unset(button_info *bi, int n);
int button_info_query(button_info *bi, int n);
void button_info_do(button_info *bi);
void button_info_clear(button_info *bi);
void process_set_of_keys(int key, int count, int *list);
void game_process_pause_key();
void button_strip_noncritical_keys(button_info *bi);


#endif

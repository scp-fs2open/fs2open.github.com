/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/MissionBrief.h $
 * $Revision: 2.6 $
 * $Date: 2005-07-13 03:25:58 $
 * $Author: Goober5000 $
 *
 * Header file for code to display the mission briefing to the player
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2005/06/03 06:39:26  taylor
 * better audio pause/unpause support when game window loses focus or is minimized
 *
 * Revision 2.4  2005/01/31 23:27:54  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.3  2004/08/11 05:06:28  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2003/03/30 21:16:21  Goober5000
 * fixed stupid spelling mistake
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
 * 3     9/07/99 6:53p Jefff
 * functionality to break out of a loop
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 7     5/19/98 8:35p Dave
 * Revamp PXO channel listing system. Send campaign goals/events to
 * clients for evaluation. Made lock button pressable on all screens. 
 * 
 * 6     4/20/98 3:53p Lawrance
 * Fix various bugs with auto-advancing through briefings.
 * 
 * 5     3/30/98 5:16p Lawrance
 * centralize a check for disabled loadout screens
 * 
 * 4     3/11/98 10:28a Lawrance
 * Add 'skip training' button
 * 
 * 3     1/13/98 5:35p Lawrance
 * Added brief_turn_off_closeup_icon()
 * 
 * 2     12/05/97 2:39p Lawrance
 * added some different sounds to main hall, add support for looping
 * ambient sounds
 * 
 * 1     9/30/97 10:16a Lawrance
 * move files from Mission lib to MissionUI lib
 * 
 * 39    9/24/97 5:29p Lawrance
 * add voice playback of briefing text
 * 
 * 38    9/24/97 5:03p Dave
 * Spliced a bunch of stuff into MissionScreenCommon.[h,cpp]
 * 
 * 37    9/22/97 5:12p Dave
 * Added stats transfer game _mode_. Started work on multiplayer chat
 * screens for weapon and ship select
 * 
 * 36    9/18/97 11:11p Lawrance
 * extern Brief_background_bitmap
 * 
 * 35    8/19/97 1:33p Dave
 * Enhancements to multi brief chat screen (sounds, scrolling, etc.)
 * 
 * 34    8/17/97 2:41p Lawrance
 * improving interface
 * 
 * 33    8/15/97 8:25p Lawrance
 * fix bug with freeing input box on briefing screens
 * 
 * 32    8/15/97 7:58p Lawrance
 * integrate new art for the briefing screens
 * 
 * 31    8/14/97 5:21p Dave
 * Added multiplayer briefing chat system.
 * 
 * 30    7/31/97 1:38p Lawrance
 * show multiplayer chat window in all screens (blited from common_render)
 * 
 * 29    7/20/97 6:59p Lawrance
 * changed name of some anim functions to be more consistent
 * 
 * 28    7/14/97 3:58p Lawrance
 * limit frametime to 33 ms for animation timing
 * 
 * 27    6/26/97 12:12a Lawrance
 * supporting anti-aliased bitmap animations
 * 
 * 26    6/24/97 11:46p Lawrance
 * supporting icon text and rotating models
 * 
 * 25    6/18/97 11:00a Lawrance
 * add in ship icons, move briefing render code into MissionBriefCommon
 * 
 * 24    6/12/97 11:09p Lawrance
 * getting map and text integrated into briefing
 * 
 * 23    6/12/97 5:15p Lawrance
 * added hook for ambient sound in briefing/ship select
 * 
 * 22    6/12/97 11:27a Lawrance
 * separating FRED dependant briefing code
 * 
 * 21    6/12/97 9:58a Lawrance
 * Move grid header stuff to separate file
 * 
 * 20    6/12/97 2:48a Lawrance
 * integrating briefing into ship select / weapon loadout screen
 * 
 * 19    6/11/97 11:55a Lawrance
 * added new data structures to hold briefing/debriefing info
 * 
 * 18    4/02/97 11:57a Lawrance
 * pre-load buffer for briefing music so no delay when briefing starts
 * 
 * 17    3/31/97 5:45p Lawrance
 * supporting changes to allow multiple streamed audio files
 * 
 * 16    2/05/97 10:35a Lawrance
 * supporting spooled music at menus, briefings, credits etc.
 * 
 *
 * $NoKeywords: $
 *
 */

#ifndef _MISSIONBRIEF_H
#define _MISSIONBRIEF_H

#include "ui/ui.h"

// #defines to identify which screen we are on
#define ON_BRIEFING_SELECT			1
#define ON_SHIP_SELECT				2
#define ON_WEAPON_SELECT			3

// briefing buttons
#define BRIEF_BUTTON_LAST_STAGE		0
#define BRIEF_BUTTON_NEXT_STAGE		1
#define BRIEF_BUTTON_PREV_STAGE		2
#define BRIEF_BUTTON_FIRST_STAGE		3
#define BRIEF_BUTTON_SCROLL_UP		4
#define BRIEF_BUTTON_SCROLL_DOWN		5
#define BRIEF_BUTTON_SKIP_TRAINING	6
#define BRIEF_BUTTON_PAUSE				7
#define BRIEF_BUTTON_MULTI_LOCK		8
#define BRIEF_BUTTON_EXIT_LOOP		9


#define NUM_BRIEFING_REGIONS	(NUM_COMMON_REGIONS + 8)

extern int	Brief_multitext_bitmap;	// bitmap for multiplayer chat window
extern int	Brief_background_bitmap;
extern UI_INPUTBOX	Common_multi_text_inputbox[3];

// Sounds
#define		BRIEFING_MUSIC_DELAY	2500		// 650 ms delay before breifing music starts
extern int	Briefing_music_handle;
extern int	Briefing_music_begin_timestamp;

extern int Briefing_paused;	// for stopping audio and stage progression

struct brief_icon;

void brief_init();
void brief_close();
void brief_do_frame(float frametime);
void brief_unhide_buttons();
brief_icon *brief_get_closeup_icon();
void brief_turn_off_closeup_icon();

void briefing_stop_music();
void briefing_start_music();
void briefing_load_music(char* fname);
void brief_stop_voices();

void brief_pause();
void brief_unpause();

int brief_only_allow_briefing();

#endif // don't add anything past this line

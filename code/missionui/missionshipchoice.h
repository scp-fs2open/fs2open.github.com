/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/MissionShipChoice.h $
 * $Revision: 2.2 $
 * $Date: 2003-09-16 11:56:46 $
 * $Author: unknownplayer $
 *
 * Header file to support functions that allow player ship selection for the mission
 *
 * $Log: not supported by cvs2svn $
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
 * 3     7/15/99 6:36p Jamesa
 * Moved default ship name into the ships.tbl
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 21    3/05/98 5:03p Dave
 * More work on team vs. team support for multiplayer. Need to fix bugs in
 * weapon select.
 * 
 * 20    2/24/98 6:21p Lawrance
 * Integrate new reset button into loadout screens
 * 
 * 19    2/18/98 3:56p Dave
 * Several bugs fixed for mp team select screen. Put in standalone packet
 * routing for team select.
 * 
 * 18    2/17/98 6:07p Dave
 * Tore out old multiplayer team select screen, installed new one.
 * 
 * 17    1/15/98 4:11p Lawrance
 * Add call to check if slot is player occupied.
 * 
 * 16    1/09/98 6:06p Dave
 * Put in network sound support for multiplayer ship/weapon select
 * screens. Made clients exit game correctly through warp effect. Fixed
 * main hall menu help overlay bug.
 * 
 * 15    12/29/97 4:21p Lawrance
 * Flash buttons on briefing/ship select/weapons loadout when enough time
 * has elapsed without activity.
 * 
 * 14    12/24/97 1:19p Lawrance
 * fix some bugs with the multiplayer ship/weapons loadout
 * 
 * 13    12/23/97 5:25p Allender
 * more fixes to multiplayer ship selection.  Fixed strange reentrant
 * problem with cf_callback when loading freespace data
 * 
 * 12    12/22/97 6:18p Lawrance
 * Get save/restore of player loadout working with new code
 * 
 * 11    12/22/97 1:40a Lawrance
 * Re-write ship select/weapons loadout to be multiplayer friendly
 * 
 * 10    12/19/97 1:23p Dave
 * Put in multiplayer groundwork for new weapon/ship select screens.
 * 
 * 9     12/18/97 8:59p Dave
 * Finished putting in basic support for weapon select and ship select in
 * multiplayer.
 * 
 * 8     12/18/97 10:24a Lawrance
 * Fix problem with ships having no weapons due to bogus weapons pools.
 * 
 * 7     12/08/97 9:37p Lawrance
 * fix up auto-restore of player ship selection and weapons
 * 
 * 6     12/03/97 1:22p Lawrance
 * implement saving/restoring of ship selection and weapons loadout
 * 
 * 5     12/02/97 10:51p Lawrance
 * implement save/restore of ship selection and weapon loadouts
 * 
 * 4     11/27/97 10:03a Lawrance
 * supporting SF_LOCKED flag
 * 
 * 3     10/04/97 5:56p Lawrance
 * keep track of original ship class in ship selection
 * 
 * 2     10/04/97 12:00a Lawrance
 * grey out background when help overlay is activated
 * 
 * 1     9/30/97 10:16a Lawrance
 * move files from Mission lib to MissionUI lib
 * 
 * 34    9/25/97 3:49p Lawrance
 * add a disabled icon to ship icon frame list
 * 
 * 33    9/24/97 11:43p Hoffoss
 * Changed Parse_player to Team_data, removed the fields we don't want in
 * it anymore, and fixed some code that had problems with that.
 * 
 * 32    9/24/97 5:03p Dave
 * Spliced a bunch of stuff into MissionScreenCommon.[h,cpp]
 * 
 * 31    9/23/97 11:55p Lawrance
 * add ss_return_name() function
 * 
 * 30    9/19/97 4:22p Allender
 * externed some functions that multi team selection screen will use
 * 
 * 29    9/18/97 7:58a Lawrance
 * fix some bugs associated with the player ship being created early on
 * 
 * 28    9/16/97 5:15p Allender
 * fixed link problem with Fred
 * 
 * 27    9/07/97 10:04p Lawrance
 * make drag/drop and selection work as Windows does with mouse-button up
 * for selection
 * 
 * 26    8/30/97 12:24p Lawrance
 * supporting animations in the weapons loadout screen, fixed some bugs
 * 
 * 25    8/29/97 7:33p Lawrance
 * further work on weapons loadout
 * 
 * 24    8/24/97 5:24p Lawrance
 * improve drawing of buttons 
 * 
 * 23    8/18/97 5:28p Lawrance
 * integrating sounds for when mouse goes over an active control
 * 
 * 22    8/17/97 2:41p Lawrance
 * improving interface
 * 
 * 21    8/15/97 8:00p Lawrance
 * integrating new art for the briefing screens
 * 
 * 20    8/11/97 9:47p Lawrance
 * get multiplayer chat window hooks working
 * 
 * 19    7/23/97 11:36a Lawrance
 * support common buttons through the briefing/ship select/weapons
 * loadout, be able to hide buttons when necessary
 * 
 * 18    7/21/97 11:41a Lawrance
 * make playback time of .ani files keyed of frametime
 * 
 * 17    7/20/97 6:59p Lawrance
 * changed name of some anim functions to be more consistent
 * 
 * 16    7/14/97 3:58p Lawrance
 * limit frametime to 33 ms for animation timing
 * 
 * 15    6/26/97 12:12a Lawrance
 * supporting anti-aliased bitmap animations
 * 
 * 14    6/24/97 11:46p Lawrance
 * supporting icon text and rotating models
 * 
 * 13    6/12/97 11:09p Lawrance
 * getting map and text integrated into briefing
 * 
 * 12    6/12/97 11:28a Lawrance
 * separating FRED dependant code
 * 
 * 11    6/12/97 2:48a Lawrance
 * integrating briefing into ship select / weapon loadout screen
 * 
 * 10    5/23/97 2:01p Lawrance
 * added update_player_weapons()
 * 
 * 9     5/20/97 2:44p Allender
 * move player ship creation into player_level_init which is done before
 * most other things when a level loads
 * 
 * 8     3/28/97 12:09p Lawrance
 * change create_default_player_ship() to allow the default to be the last
 * ship the player has flown
 * 
 * 7     3/20/97 3:01p Lawrance
 * made scroll buttons repeat action when held down, don't do button
 * actions until down and release
 * 
 * 6     3/07/97 8:14p Lawrance
 * added code to drag and drop to wing formations
 * 
 * 5     2/26/97 10:05a Lawrance
 * move ship_create() to end of loop when commit pressed, so don't skip on
 * animation
 * 
 * 4     2/25/97 11:11a Lawrance
 * ship selection and weapon loadout interfaces working at basic level
 * 
 * 3     12/30/96 1:58a Lawrance
 * supporting ship choice in multiplayer
 * 
 * 2     11/19/96 1:21p Lawrance
 * got ship selection working inside out
 *
 * $NoKeywords: $
 *
*/

#ifndef __MISSIONSHIPCHOICE_H__
#define __MISSIONSHIPCHOICE_H__

#include "missionui/missionscreencommon.h"
#include "mission/missionparse.h"

///////////////////////////////////////////////////////
// Ships selection hot spots
///////////////////////////////////////////////////////
#define SHIP_SELECT_SHIP_SCROLL_UP				8
#define SHIP_SELECT_SHIP_SCROLL_DOWN			9

#define SHIP_SELECT_ICON_0							10
#define SHIP_SELECT_ICON_1							11
#define SHIP_SELECT_ICON_2							12
#define SHIP_SELECT_ICON_3							13

#define WING_0_SHIP_0								14
#define WING_0_SHIP_1								15
#define WING_0_SHIP_2								16
#define WING_0_SHIP_3								17
#define WING_1_SHIP_0								18
#define WING_1_SHIP_1								19
#define WING_1_SHIP_2								20
#define WING_1_SHIP_3								21
#define WING_2_SHIP_0								22
#define WING_2_SHIP_1								23
#define WING_2_SHIP_2								24
#define WING_2_SHIP_3								25

#define SHIP_SELECT_RESET							39

#define NUM_SHIP_SELECT_REGIONS					(NUM_COMMON_REGIONS + 19)

extern int Ship_select_open;	// This game-wide global flag is set to 1 to indicate that the ship
										// select screen has been opened and memory allocated.  This flag
										// is needed so we can know if ship_select_close() needs to called if
										// restoring a game from the Options screen invoked from ship select

extern int Commit_pressed;	// flag to indicate that the commit button was pressed
									// use a flag, so the ship_create() can be done at the end of the loop

extern char default_player_ship[255];
extern int Select_default_ship;

extern float ShipSelectScreenShipRot;

void draw_wing_block(int wb_num, int hot_slot, int selected_slot, int class_select);
void ship_select_init();
void ship_select_do(float frametime);
void ship_select_close();
void ship_select_common_init();
void ship_stop_animation();
int ss_get_ship_class(int ship_entry_index);
int ss_get_selected_ship();

void ss_blit_ship_icon(int x,int y,int ship_class,int bmap_num);

// called from weapon select
int	ss_return_ship(int wing_block, int wing_slot, int *ship_index, p_object **ppobjp);
void	ss_return_name(int wing_block, int wing_slot, char *name);
int	ss_return_original_ship_class(int slot_num);
int	ss_return_saindex(int slot_num);
int	ss_disabled_slot(int slot_num);
int	ss_wing_slot_is_console_player(int index);

// lock/unlock any necessary slots for multiplayer
void ss_recalc_multiplayer_slots();

int	create_default_player_ship( int use_last_flown = 1 );
void	update_player_ship(int si_index);

void ss_synch_interface();

// set the necessary pointers
void ss_set_team_pointers(int team);

// called by multiplayer team select to set the slot based flags
void ss_make_slot_empty(int slot_index);
void ss_make_slot_full(int slot_index);

int ss_dump_to_list(int from_slot, int to_list, int *sound);
int ss_swap_slot_slot(int from_slot, int to_slot, int *sound);
int ss_grab_from_list(int from_list, int to_slot, int *sound);
int ss_swap_list_slot(int from_list, int to_slot, int *sound);

void ss_apply(int mode, int from_slot,int from_index,int to_slot,int to_index,int player_index = -1);
void ss_drop(int from_slot,int from_index,int to_slot,int to_index,int player_index = -1);

#endif  /* __MISSIONSHIPCHOICE_H__ */

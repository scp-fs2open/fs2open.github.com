/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/MissionWeaponChoice.h $
 * $Revision: 2.4 $
 * $Date: 2004-03-05 09:01:55 $
 * $Author: Goober5000 $
 *
 * Header file for the weapon loadout screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2003/03/05 09:17:14  Goober5000
 * cleaned out Bobboau's buggy code - about to rewrite with new, bug-free code :)
 * --Goober5000
 *
 * Revision 2.2  2003/02/25 06:22:49  bobboau
 * fixed a bunch of fighter beam bugs,
 * most notabley the sound now works corectly,
 * and they have limeted range with atenuated damage (table option)
 * added bank specific compatabilities
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
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 21    4/17/98 5:27p Dave
 * More work on the multi options screen. Fixed many minor ui todo bugs.
 * 
 * 20    3/31/98 1:50p Duncan
 * ALAN: fix bugs with selecting alternate weapons 
 * 
 * 19    2/28/98 7:04p Lawrance
 * Don't show reset button in multiplayer
 * 
 * 18    2/24/98 6:21p Lawrance
 * Integrate new reset button into loadout screens
 * 
 * 17    1/17/98 4:14p Lawrance
 * fix mask problem with primary weapon scrolling
 * 
 * 16    1/17/98 1:32a Lawrance
 * fix mask problem that was mixing up scroll up and scroll down on
 * weapons loadout
 * 
 * 15    1/09/98 6:06p Dave
 * Put in network sound support for multiplayer ship/weapon select
 * screens. Made clients exit game correctly through warp effect. Fixed
 * main hall menu help overlay bug.
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
 * 8     12/17/97 7:42p Lawrance
 * re-work how weapons are re-set when ships change in ship select
 * 
 * 7     12/17/97 4:53p Lawrance
 * changes to support multiplayer
 * 
 * 6     12/17/97 2:33p Dave
 * Finished up basic weapon select support for multiplayer.
 * 
 * 5     12/16/97 6:17p Dave
 * Put in primary weapon support for multiplayer weapon select screen.
 * 
 * 4     12/03/97 1:22p Lawrance
 * implement saving/restoring of ship selection and weapons loadout
 * 
 * 3     12/02/97 10:51p Lawrance
 * implement save/restore of ship selection and weapon loadouts
 * 
 * 2     11/15/97 6:12p Lawrance
 * don't allow Player ship to have all weapons removed
 * 
 * 1     9/30/97 10:16a Lawrance
 * move files from Mission lib to MissionUI lib
 * 
 * 8     9/18/97 7:58a Lawrance
 * fix some bugs associated with the player ship being created early on
 * 
 * 7     8/30/97 12:24p Lawrance
 * supporting animations in the weapons loadout screen, fixed some bugs
 * 
 * 6     8/29/97 7:33p Lawrance
 * further work on weapons loadout
 * 
 * 5     8/15/97 8:00p Lawrance
 * integrating new art for the briefing screens
 * 
 * 4     7/23/97 11:36a Lawrance
 * support common buttons through the briefing/ship select/weapons
 * loadout, be able to hide buttons when necessary
 * 
 * 3     7/14/97 3:58p Lawrance
 * limit frametime to 33 ms for animation timing
 * 
 * 2     2/25/97 11:11a Lawrance
 * ship selection and weapon loadout interfaces working at basic level
 *
 * $NoKeywords: $
 */


#ifndef __MISSION_WEAPON_CHOICE_H__
#define __MISSION_WEAPON_CHOICE_H__

struct p_object;
struct wss_unit;
struct ship_weapon;

// mask regions for icons in the scrollable lists
#define ICON_PRIMARY_0				28
#define ICON_PRIMARY_1				29
#define ICON_PRIMARY_2				30
#define ICON_PRIMARY_3				31
#define ICON_SECONDARY_0			10
#define ICON_SECONDARY_1			11
#define ICON_SECONDARY_2			12
#define ICON_SECONDARY_3			13

// mask regions for icons that sit above the ship
#define ICON_SHIP_PRIMARY_0		32
#define ICON_SHIP_PRIMARY_1		33
#define ICON_SHIP_PRIMARY_2		34
#define ICON_SHIP_SECONDARY_0		35
#define ICON_SHIP_SECONDARY_1		36
#define ICON_SHIP_SECONDARY_2		37
#define ICON_SHIP_SECONDARY_3		38

// mask region for weapon loadout specific buttons
#define PRIMARY_SCROLL_UP					27	
#define PRIMARY_SCROLL_DOWN				26
#define SECONDARY_SCROLL_UP				9
#define SECONDARY_SCROLL_DOWN				8
#define WL_RESET_BUTTON_MASK				39

#define NUM_WEAPON_REGIONS		(NUM_COMMON_REGIONS + 32)

void weapon_select_init();
void weapon_select_common_init();
void weapon_select_do(float frametime);
void weapon_select_close();

void	wl_update_parse_object_weapons(p_object *pobjp, wss_unit *slot);
int	wl_update_ship_weapons(int objnum, wss_unit *slot);
void	wl_bash_ship_weapons(ship_weapon *swp, wss_unit *slot);

void wl_set_default_weapons(int index, int ship_class);
void wl_reset_to_defaults();

// Set selected slot to first placed ship
void wl_reset_selected_slot();

void wl_remove_weps_from_pool(int *wep, int *wep_count, int ship_class);
void wl_get_ship_class_weapons(int ship_class, int *wep, int *wep_count);
void wl_get_default_weapons(int ship_class, int slot_num, int *wep, int *wep_count);

void wl_synch_interface();
void wl_apply(int mode,int from_bank,int from_list,int to_bank,int to_list,int ship_slot,int player_index = -1);
void wl_drop(int from_bank,int from_list,int to_bank,int to_list, int ship_slot,int player_index = -1);

#endif /* __MISSION_WEAPON_CHOICE_H__ */

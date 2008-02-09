/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDshield.h $
 * $Revision: 2.1 $
 * $Date: 2003-01-06 17:14:52 $
 * $Author: Goober5000 $
 *
 * Header file for the display and management of the HUD shield
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     8/27/99 10:36a Dave
 * Impose a 2% penalty for hitting the shield balance key.
 * 
 * 3     7/22/99 4:00p Dave
 * Fixed beam weapon muzzle glow rendering. Externalized hud shield info.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 7     4/25/98 2:00p Dave
 * Installed a bunch of multiplayer context help screens. Reworked ingame
 * join ship select screen. Fix places where network timestamps get hosed.
 * 
 * 6     2/12/98 4:58p Lawrance
 * Change to new flashing method.
 * 
 * 5     11/18/97 5:58p Lawrance
 * flash escort view info when that ship is taking hits
 * 
 * 4     11/08/97 11:08p Lawrance
 * implement new "mini-shield" view that sits near bottom of reticle
 * 
 * 3     11/04/97 7:49p Lawrance
 * integrating new HUD reticle and shield icons
 * 
 * 2     8/25/97 12:24a Lawrance
 * implemented HUD shield management
 * 
 * 1     8/24/97 10:31p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __FREESPACE_HUDSHIELD_H__
#define __FREESPACE_HUDSHIELD_H__

#define SHIELD_HIT_DURATION	1400	// time a shield quadrant flashes after being hit
#define SHIELD_FLASH_INTERVAL	200	// time between shield quadrant flashes

#define NUM_SHIELD_HIT_MEMBERS	5
#define HULL_HIT_OFFSET				4		// used to access the members in shield_hit_info that pertain to the hull
typedef struct shield_hit_info
{
	int shield_hit_status;		// bitfield, if offset for shield quadrant is set, that means shield is being hit
	int shield_show_bright;		// bitfield, if offset for shield quadrant is set, that means play bright frame
	int shield_hit_timers[NUM_SHIELD_HIT_MEMBERS];	// timestamps that get set for SHIELD_FLASH_TIME when a quadrant is hit
	int shield_hit_next_flash[NUM_SHIELD_HIT_MEMBERS];
} shield_hit_info;

extern ubyte Quadrant_xlate[4];

struct player;

void hud_shield_game_init();
void hud_shield_level_init();
void hud_shield_show(object *objp);
void hud_shield_equalize(object *objp, player *pl);
void hud_augment_shield_quadrant(object *objp, int direction);
void hud_shield_assign_info(ship_info *sip, char *filename);
void hud_show_mini_ship_integrity(object *objp, int force_x = -1, int force_y = -1);
void hud_shield_show_mini(object *objp, int x_force = -1, int y_force = -1, int x_hull_offset = 0, int y_hull_offset = 0);
void hud_shield_hit_update();
void hud_shield_quadrant_hit(object *objp, int quadrant);
void hud_shield_hit_reset(int player=0);

void shield_info_reset(shield_hit_info *shi);

// random page in stuff - moved here by Goober5000
extern void hud_ship_icon_page_in(ship_info *sip);


#endif /* __FREESPACE_HUDSHIELDBOX_H__ */

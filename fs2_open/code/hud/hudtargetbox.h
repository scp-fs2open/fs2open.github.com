/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDtargetbox.h $
 * $Revision: 2.5 $
 * $Date: 2005-07-13 03:15:52 $
 * $Author: Goober5000 $
 *
 * Header file for drawing the target monitor box on the HUD
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/08/11 05:06:25  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.3  2004/04/06 03:09:53  phreak
 * added a control config option for the wireframe hud targetbox i enabled ages ago
 *
 * Revision 2.2  2004/03/05 09:02:04  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/06 16:50:13  phreak
 * added Targetbox_wire variable to check what mode the
 * hud targetbox uses
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2 2002/08/04  22:46:55  PhReAk
 * Added "Targetbox_wire" variable to toggle if wireframes are on=1 or off=0
 *
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     5/21/99 1:44p Andsager
 * Add engine wash gauge
 * 
 * 3     12/21/98 5:03p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 26    8/28/98 3:28p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 25    5/08/98 5:32p Lawrance
 * Allow cargo scanning even if target gauge is disabled
 * 
 * 24    3/30/98 1:08a Lawrance
 * Implement "blast" icon.  Blink HUD icon when player ship is hit by a
 * blast.
 * 
 * 23    2/16/98 8:47p Lawrance
 * Allow flashing to occur at twice normal rate
 * 
 * 22    2/12/98 11:54p Lawrance
 * abbreviate communication to comm
 * 
 * 21    2/12/98 4:58p Lawrance
 * Add support for 'All Clear' radio message
 * 
 * 20    2/09/98 8:05p Lawrance
 * Add new gauges: cmeasure success, warp-out, and missiontime
 * 
 * 19    1/15/98 5:23p Lawrance
 * Add HUD gauge to indicate completed objectives.
 * 
 * 18    1/12/98 10:57p Allender
 * minor changes to the message box.  made some indicators flash to bring
 * attention to them.
 * 
 * 17    1/12/98 9:44p Lawrance
 * ug, fix return type error
 * 
 * 16    1/12/98 9:40p Lawrance
 * extern hud_targetbox_maybe_flash()
 * 
 * 15    1/12/98 9:39p Lawrance
 * flash DOCKED_WITH text
 * 
 * 14    1/10/98 12:42a Lawrance
 * make cargo inspection more realistic
 * 
 * 13    12/16/97 9:13p Lawrance
 * Integrate new gauges into HUD config.
 * 
 * 12    12/09/97 6:15p Lawrance
 * add flashing of destroyed subsystems on target box
 * 
 * 11    12/01/97 12:27a Lawrance
 * redo default alpha color for HUD, make it easy to modify in the future
 * 
 * 10    11/19/97 10:19p Lawrance
 * add target status to targetbox
 * 
 * 9     11/17/97 6:37p Lawrance
 * new gauges: extended target view, new lock triangles, support ship view
 * 
 * 8     11/11/97 10:27p Lawrance
 * show docking information on the target monitor
 * 
 * 7     11/11/97 5:06p Lawrance
 * flash different areas of the target box
 * 
 * 6     11/11/97 12:58a Lawrance
 * implement new target monitor view
 * 
 * 5     11/06/97 5:01p Dave
 * Finished reworking standalone multiplayer sequencing. Put in
 * configurable observer-mode HUD.
 * 
 * 4     10/11/97 6:38p Lawrance
 * having damage affect targeting
 * 
 * 3     8/19/97 11:46p Lawrance
 * adding new hud gauges for shileds, escort view, and weapons
 * 
 * 2     8/15/97 9:26a Lawrance
 * split off target box code into HUDtargetbox.cpp
 * 
 * 1     8/15/97 8:54a Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __FREESPACE_HUDTARGETBOX_H__
#define __FREESPACE_HUDTARGETBOX_H__

#include "graphics/2d.h"

struct object;

#define TBOX_FLASH_DURATION	1400
#define TBOX_FLASH_INTERVAL	200

#define NUM_TBOX_FLASH_TIMERS		14
#define TBOX_FLASH_NAME				0
#define TBOX_FLASH_CARGO			1
#define TBOX_FLASH_HULL				2
#define TBOX_FLASH_STATUS			3
#define TBOX_FLASH_SUBSYS			4
#define TBOX_FLASH_DOCKED			5
#define TBOX_FLASH_SQUADMSG		6
#define TBOX_FLASH_OBJECTIVE		7
#define TBOX_FLASH_COLLISION		8
#define TBOX_FLASH_CMEASURE		9
#define TBOX_FLASH_NETLAG			10
#define TBOX_FLASH_BLAST			11
#define TBOX_FLASH_EMP				12
#define TBOX_FLASH_ENGINE_WASH	13

extern int Target_static_looping;

extern int Target_window_coords[GR_NUM_RESOLUTIONS][4];

// flag to indicate whether to show the extra information about a target 
// The HUD_config controls whether this can be shown... but the player can still toggle it on/off
// during the game.
extern int Targetbox_show_extra_info;

//used to track if the player has wireframe hud target box turned on
extern int Targetbox_wire;
extern bool Lock_targetbox_mode;

void	hud_targetbox_init();
void	hud_targetbox_init_flash();
void	hud_render_target_model();
void	hud_show_target_data(float frametime);
void	hud_get_target_strength(object *objp, float *shields, float *integrity);

// used to flash text, uses the TBOX_FLASH_ #defines above
void	hud_targetbox_start_flash(int index, int duration=TBOX_FLASH_DURATION);
int	hud_targetbox_maybe_flash(int index, int flash_fast=0);
void	hud_targetbox_end_flash(int index);
int	hud_targetbox_is_bright(int index);
int	hud_targetbox_flash_expired(int index);

// functions to manage the targetbox static that appears when sensors are severely damaged
void	hud_targetbox_static_init();
int	hud_targetbox_static_maybe_blit(float frametime);

void hud_render_target_ship(object *target_objp);
void hud_render_target_debris(object *target_objp);
void hud_render_target_weapon(object *target_objp);

void hud_update_cargo_scan_sound();
void hud_cargo_scan_update(object *targetp, float frametime);

char *hud_targetbox_truncate_subsys_name(char *outstr);

//swich through the valid targetbox modes
void hud_targetbox_switch_wireframe_mode();

#endif /* __FREESPACE_HUDTARGETBOX_H__ */

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDconfig.h $
 * $Revision: 2.5.2.2 $
 * $Date: 2008-01-19 00:27:08 $
 * $Author: Goober5000 $
 *
 * Header file for HUD configuration
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5.2.1  2007/08/17 03:29:48  Goober5000
 * generalize the way radar ranges are handled (inspired by Shade's fix)
 *
 * Revision 2.5  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.4  2004/08/11 05:06:25  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.3  2004/03/05 09:02:03  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2004/02/04 09:02:45  Goober5000
 * got rid of unnecessary double semicolons
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     8/02/99 9:55p Dave
 * Hardcode a nice hud color config for the demo.
 * 
 * 3     6/08/99 1:14a Dave
 * Multi colored hud test.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 30    4/29/98 6:00p Dave
 * Fixed chatbox font colors. Made observer offscreen indicators work.
 * Numerous small UI fixes. Fix rank limitations for mp games. 
 * 
 * 29    4/16/98 4:07p Hoffoss
 * Fixed bug with palette reseting when loading a new pilot.  Also made
 * barracks default to palette merging mode.
 * 
 * 28    4/14/98 6:16p Lawrance
 * Fix bug that was preventing non-green HUD color from getting set.
 * 
 * 27    3/26/98 5:45p Lawrance
 * Added new gauges to HUD config
 * 
 * 26    3/12/98 5:45p Dave
 * Put in new observer HUD. Made it possible for observers to join at the
 * beginning of a game and follow it around as an observer full-time.
 * 
 * 25    2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 24    1/28/98 6:19p Dave
 * Reduced standalone memory usage ~8 megs. Put in support for handling
 * multiplayer submenu handling for endgame, etc.
 * 
 * 23    1/18/98 5:09p Lawrance
 * Added support for TEAM_TRAITOR
 * 
 * 22    1/17/98 10:02p Lawrance
 * Fix bug with damage popup that would sometimes display with <none>.
 * 
 * 21    1/14/98 11:07p Lawrance
 * Hook in brightness slider to HUD config.
 * 
 * 20    1/13/98 5:33p Lawrance
 * Tweaking HUD config.
 * 
 * 19    1/12/98 11:16p Lawrance
 * Wonderful HUD config.
 * 
 * 18    1/10/98 12:41a Lawrance
 * start work on new HUD config
 * 
 * 17    1/05/98 9:38p Lawrance
 * Implement flashing HUD gauges.
 * 
 * 16    12/16/97 9:13p Lawrance
 * Integrate new gauges into HUD config.
 * 
 * 15    11/06/97 5:01p Dave
 * Finished reworking standalone multiplayer sequencing. Put in
 * configurable observer-mode HUD.
 * 
 * 14    11/06/97 9:53a Dave
 * 
 * 13    11/03/97 5:38p Dave
 * Cleaned up more multiplayer sequencing. Added OBJ_OBSERVER module/type.
 * Restructured HUD_config structs/flags.
 * 
 * 12    10/28/97 12:43a Lawrance
 * fix bug that was not setting HUD config correctly when a new pilot was
 * made
 * 
 * 11    9/22/97 11:46p Lawrance
 * make default radar range infinity
 * 
 * 10    9/06/97 2:13p Mike
 * Replace support for TEAM_NEUTRAL
 * 
 * 9     8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 8     5/26/97 5:49p Lawrance
 * supporting max range on radar
 * 
 * 7     3/26/97 8:56a Lawrance
 * removing target and game debug flags from HUD config
 * 
 * 6     3/19/97 5:53p Lawrance
 * integrating new Misc_sounds[] array (replaces old Game_sounds
 * structure)
 * 
 * 5     1/28/97 5:33p Lawrance
 * saving number of msg window lines in save game and player file
 * 
 * 4     12/10/96 4:18p Lawrance
 * took out In_hud_config and use game state instead
 * 
 * 3     12/08/96 1:54a Lawrance
 * integrating hud configuration
 * 
 * 2     11/29/96 6:10p Lawrance
 * HUD configuration working
 *
 * $NoKeywords: $
 *
*/

#ifndef _HUDCONFIG_H
#define _HUDCONFIG_H

#include "hud/hud.h"

struct player;
struct ship;
struct ai_info;

#define HUD_COLOR_GREEN		0
#define HUD_COLOR_BLUE		1
#define HUD_COLOR_AMBER		2

// specify the max distance that the radar should detect objects
// Index in Radar_ranges[] array to get values

#define RR_MAX_RANGES		3				// keep up to date
#define RR_SHORT			0
#define RR_LONG				1	
#define RR_INFINITY			2
extern float Radar_ranges[RR_MAX_RANGES];
extern char *Radar_range_text(int range_num);

#define RP_SHOW_DEBRIS						(1<<0)
#define RP_SHOW_FRIENDLY_MISSILES		(1<<1)
#define RP_SHOW_HOSTILE_MISSILES			(1<<2)

#define RP_DEFAULT ( RP_SHOW_DEBRIS | RP_SHOW_FRIENDLY_MISSILES | RP_SHOW_HOSTILE_MISSILES )

extern int HUD_observer_default_flags;
extern int HUD_observer_default_flags2;
extern int HUD_default_popup_mask;
extern int HUD_default_popup_mask2;
extern int HUD_config_default_flags;
extern int HUD_config_default_flags2;

typedef struct HUD_CONFIG_TYPE {		
	int show_flags;				// whether to show gauge
	int show_flags2;				// whether to show gauge
	int popup_flags;				// whether gauge is popup 	
	int popup_flags2;				// whether gauge is popup 		
	int num_msg_window_lines;	
	int rp_flags;					// see RP_ flags above
	int rp_dist;					// one of RR_ #defines above
	int is_observer;				// 1 or 0, observer mode or not, respectively
	int main_color;				// the main color

	// colors for all the gauges
	color clr[NUM_HUD_GAUGES];
} HUD_CONFIG_TYPE;

extern HUD_CONFIG_TYPE HUD_config;

void hud_config_init();
void hud_config_do_frame(float frametime);
void hud_config_close();

void hud_set_default_hud_config(player *p);
void hud_config_set_gauge_flags(int gauge_index, int on_flag, int popup_flag);

void hud_config_restore();
void hud_config_backup();
void hud_config_as_observer(ship *shipp,ai_info *aif);


void hud_config_as_observer();
void hud_config_as_player();
void hud_config_display_text(char* gauge_text, int x, int y);
void hud_set_display_gauge_cbox();

// leave hud config without accepting changes
void hud_config_cancel();

// leave hud config with accepting changes
void hud_config_commit();

// flag access/manipulation routines
int	hud_config_show_flag_is_set(int i);
void	hud_config_show_flag_set(int i);
void	hud_config_show_flag_clear(int i);
int	hud_config_popup_flag_is_set(int i);
void	hud_config_popup_flag_set(int i);
void	hud_config_popup_flag_clear(int i);

void hud_config_record_color(int color);

// load up the given hcf file
void hud_config_color_load(char *name);

#endif


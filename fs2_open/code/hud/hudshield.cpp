/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDshield.cpp $
 * $Revision: 2.11 $
 * $Date: 2004-03-05 09:02:03 $
 * $Author: Goober5000 $
 *
 * C file for the display and management of the HUD shield
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.10  2004/02/04 08:41:04  Goober5000
 * made code more uniform and simplified some things,
 * specifically shield percentage and quadrant stuff
 * --Goober5000
 *
 * Revision 2.9  2003/09/13 08:27:29  Goober5000
 * added some minor things, such as code cleanup and the following:
 * --turrets will not fire at cargo
 * --MAX_SHIELD_SECTIONS substituted for the number 4 in many places
 * --supercaps have their own default message bitfields (distinguished from capships)
 * --turrets are allowed on fighters
 * --jump speed capped at 65m/s, to avoid ship travelling too far
 * --non-huge weapons now scale their damage, instead of arbitrarily cutting off
 * ----Goober5000
 *
 * Revision 2.8  2003/09/13 06:02:05  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.5  2003/09/09 05:51:14  Goober5000
 * if player has primitive sensors, hud will not display shield icons or message sender brackets
 * --Goober5000
 *
 * Revision 2.4  2003/04/29 01:03:23  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.3  2003/03/17 10:37:32  Goober5000
 * pressing Q no longer makes a sound if the player's ship doesn't have shields
 * --Goober5000
 *
 * Revision 2.2  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/20 23:51:59  DTP
 * Bumped Max Shield Icons to 80
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/13 15:11:03  mharris
 * More NO_NETWORK ifndefs added
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    8/27/99 10:36a Dave
 * Impose a 2% penalty for hitting the shield balance key.
 * 
 * 11    8/23/99 11:34a Dave
 * Fixed shield intensity rendering problems.
 * 
 * 10    8/01/99 12:39p Dave
 * Added HUD contrast control key (for nebula).
 * 
 * 9     7/22/99 4:00p Dave
 * Fixed beam weapon muzzle glow rendering. Externalized hud shield info.
 * 
 * 8     6/10/99 3:43p Dave
 * Do a better job of syncing text colors to HUD gauges.
 * 
 * 7     1/07/99 9:06a Jasen
 * coords...
 * 
 * 6     12/30/98 8:57a Jasen
 * updated coords for hi res
 * 
 * 5     12/28/98 3:17p Dave
 * Support for multiple hud bitmap filenames for hi-res mode.
 * 
 * 4     12/21/98 5:02p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 3     12/14/98 1:15p Jasen
 * added new HUD shield gauges
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 35    9/19/98 3:11p Adam
 * Added new hardcoded values for Hud_shield_filenames
 * 
 * 34    8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 33    5/17/98 3:32p Lawrance
 * Make shield gauge more readable when flashing
 * 
 * 32    4/25/98 5:39p Dave
 * Removed an unneeded assert.
 * 
 * 31    4/25/98 3:56p Mike
 * Make player's shield icon flash when Fred tells it to.
 * 
 * 30    4/25/98 2:00p Dave
 * Installed a bunch of multiplayer context help screens. Reworked ingame
 * join ship select screen. Fix places where network timestamps get hosed.
 * 
 * 29    4/21/98 12:19a Allender
 * only play equalize shield sound if player equalizes shields
 * 
 * 28    3/26/98 5:26p John
 * added new paging code. nonfunctional.
 * 
 * 27    3/21/98 3:35p Lawrance
 * Tweak position of numeric integrity for target
 * 
 * 26    3/14/98 4:59p Lawrance
 * Flash shield/ship icons when ships are hit
 * 
 * 25    3/02/98 5:42p John
 * Removed WinAVI stuff from Freespace.  Made all HUD gauges wriggle from
 * afterburner.  Made gr_set_clip work good with negative x &y.  Made
 * model_caching be on by default.  Made each cached model have it's own
 * bitmap id.  Made asteroids not rotate when model_caching is on.  
 * 
 * 24    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 23    2/12/98 4:58p Lawrance
 * Change to new flashing method.
 * 
 * 22    1/12/98 11:16p Lawrance
 * Wonderful HUD config.
 * 
 * 21    1/08/98 4:36p Lawrance
 * Fix bug in shield drawing code.
 * 
 * 20    1/05/98 9:38p Lawrance
 * Implement flashing HUD gauges.
 * 
 * 19    1/02/98 9:10p Lawrance
 * Big changes to how colors get set on the HUD.
 * 
 * 18    12/29/97 9:48p Mike
 * Prevent indexing before array start when quadrant_num = -1.
 * 
 * 17    12/01/97 12:27a Lawrance
 * redo default alpha color for HUD, make it easy to modify in the future
 * 
 * 16    11/18/97 5:58p Lawrance
 * flash escort view info when that ship is taking hits
 * 
 * 15    11/18/97 1:21p Mitri
 * ALAN: be sure to only draw shield icons for targets that are ships
 * 
 * 14    11/17/97 6:37p Lawrance
 * new gauges: extended target view, new lock triangles, support ship view
 * 
 * 13    11/13/97 10:46p Lawrance
 * implemented new escort view, damage view and weapons
 * 
 * 12    11/12/97 9:42a Lawrance
 * show player ship integrity above shield icon
 * 
 * 11    11/11/97 5:06p Lawrance
 * fix bug with flashing frequency of hull
 * 
 * 10    11/09/97 11:27p Lawrance
 * move target shield icon closer to center
 * 
 * 9     11/09/97 4:39p Lawrance
 * don't draw mini ship icon anymore
 * 
 * 8     11/08/97 11:08p Lawrance
 * implement new "mini-shield" view that sits near bottom of reticle
 * 
 * 7     11/05/97 11:21p Lawrance
 * implement dynamic alpha on the shields
 * 
 * 6     11/04/97 8:34p Lawrance
 * fix warning: remove unused variable
 * 
 * 5     11/04/97 7:49p Lawrance
 * integrating new HUD reticle and shield icons
 * 
 * 4     10/24/97 5:51p Lawrance
 * don't show shield % if ship has no shields
 * 
 * 3     9/03/97 4:32p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 2     8/25/97 12:24a Lawrance
 * implemented HUD shield management
 * 
 * 1     8/24/97 10:31p Lawrance
 *
 * $NoKeywords: $
 */

#include "hud/hudshield.h"
#include "graphics/2d.h"
#include "object/object.h"
#include "hud/hud.h"
#include "hud/hudtargetbox.h"
#include "playerman/player.h"
#include "gamesnd/gamesnd.h"
#include "io/timer.h"
#include "hud/hudescort.h"
#include "weapon/emp.h"
#include "parse/parselo.h"
#include "ship/ship.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#endif

#define NUM_SHIELD_LEVELS		8

#define SHIELD_TRANSFER_PERCENT	0.083f		// 1/12 total shield strength

#define SHIELD_HIT_DURATION_SHORT	300	// time a shield quadrant flashes after being hit
#define SHIELD_FLASH_INTERVAL_FAST	200	// time between shield quadrant flashes

// now read in from hud.tbl
#define MAX_SHIELD_ICONS		80	//DTP bumped from 40 to 80
int Hud_shield_filename_count = 0;
char Hud_shield_filenames[MAX_SHIELD_ICONS][MAX_FILENAME_LEN];

char Shield_mini_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"targhit1",
	"targhit1"
};

hud_frames Shield_gauges[MAX_SHIELD_ICONS];

static int Player_shield_coords[GR_NUM_RESOLUTIONS][2] = 
{
	{ // GR_640
		396, 379
	},
	{ // GR_1024
		634, 670
	}
};

static int Target_shield_coords[GR_NUM_RESOLUTIONS][2] = 
{
	{ // GR_640
		142, 379
	},
	{ // GR_1024
		292, 670
	}
};

static int Hud_shield_inited = 0;

int Shield_mini_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		305, 291
	},
	{ // GR_1024
		497, 470
	}
};

// draw on the mini shield icon what the ship integrity is
int Hud_mini_3digit[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		310, 298, 0
	},
	{ // GR_1024
		502, 477, 0
	}
};
int Hud_mini_2digit[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		213, 298, 2
	},
	{ // GR_1024
		346, 477, 2
	}
};
int Hud_mini_1digit[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		316, 298, 6
	},
	{ // GR_1024
		511, 477, 6
	}
};
int Hud_mini_base[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		310, 298
	},
	{ // GR_1024
		502, 477
	}
};
	
int Shield_mini_loaded = 0;
hud_frames Shield_mini_gauge;

#define	SHIELD_HIT_PLAYER		0
#define	SHIELD_HIT_TARGET		1
static shield_hit_info	Shield_hit_data[2];

// translate between clockwise-from-top shield quadrant ordering to way quadrants are numbered in the game
ubyte Quadrant_xlate[MAX_SHIELD_SECTIONS] = {1,0,2,3};

void hud_shield_game_init()
{
	char name[MAX_FILENAME_LEN+1] = "";

	// read in hud.tbl
	read_file_text("hud.tbl");
	reset_parse();

	Hud_shield_filename_count = 0;
	required_string("#Shield Icons Begin");
	while(!optional_string("#End")){
		required_string("$Shield:");

		stuff_string(name, F_NAME, NULL);

		// maybe store
		Assert(Hud_shield_filename_count < MAX_SHIELD_ICONS);
		if(Hud_shield_filename_count < MAX_SHIELD_ICONS){
			strcpy(Hud_shield_filenames[Hud_shield_filename_count++], name);
		}
	}
}

// called at the start of each level from HUD_init.  Use Hud_shield_init so we only init Shield_gauges[] once.
void hud_shield_level_init()
{
	int i;	

	hud_shield_hit_reset(1);	// reset for the player

	if ( Hud_shield_inited ) {
		return;
	}	

	for ( i = 0; i < MAX_SHIELD_ICONS; i++ ) {
		Shield_gauges[i].first_frame = -1;
		Shield_gauges[i].num_frames  = 0;
	}

	Hud_shield_inited = 1;

	if ( !Shield_mini_loaded ) {
		Shield_mini_gauge.first_frame = bm_load_animation(Shield_mini_fname[gr_screen.res], &Shield_mini_gauge.num_frames);
		if ( Shield_mini_gauge.first_frame == -1 ) {
			Warning(LOCATION, "Could not load in the HUD shield ani: Shield_mini_fname[gr_screen.res]\n");
			return;
		}
		Shield_mini_loaded = 1;
	}
}

int hud_shield_maybe_flash(int gauge, int target_index, int shield_offset)
{
	int					flashed = 0;
	shield_hit_info	*shi;

	shi = &Shield_hit_data[target_index];

	if ( !timestamp_elapsed(shi->shield_hit_timers[shield_offset]) ) {
		if ( timestamp_elapsed(shi->shield_hit_next_flash[shield_offset]) ) {
			shi->shield_hit_next_flash[shield_offset] = timestamp(SHIELD_FLASH_INTERVAL_FAST);
			shi->shield_show_bright ^= (1<<shield_offset);	// toggle between default and bright frames
		}

		if ( shi->shield_show_bright & (1<<shield_offset) ) {
			// hud_set_bright_color();
			hud_set_gauge_color(gauge, HUD_C_BRIGHT);
			flashed = 1;
		} else {
			hud_set_gauge_color(gauge, HUD_C_NORMAL);
			// hud_set_default_color();
		}
	}

	return flashed;
}

// ------------------------------------------------------------------
// hud_shield_show()
//
// Show the players shield strength and integrity
//
void hud_shield_show(object *objp)
{
	float			max_shield;
	int			hud_color_index, range;
	int			sx, sy, i;
	ship			*sp;
	ship_info	*sip;
	hud_frames	*sgp;

	if ( objp->type != OBJ_SHIP )
		return;

	// Goober5000 - don't show if primitive sensors
	if ( Ships[Player_obj->instance].flags2 & SF2_PRIMITIVE_SENSORS )
		return;

	sp = &Ships[objp->instance];
	sip = &Ship_info[sp->ship_info_index];

	if ( sip->shield_icon_index == 255 ) {
		return;
	}

	if (objp == Player_obj) {
		hud_set_gauge_color(HUD_PLAYER_SHIELD_ICON);
	} else {
		hud_set_gauge_color(HUD_TARGET_SHIELD_ICON);
	}

	// load in shield frames if not already loaded
	Assert(sip->shield_icon_index >= 0 && sip->shield_icon_index < Hud_shield_filename_count);
	sgp = &Shield_gauges[sip->shield_icon_index];

	if ( sgp->first_frame == -1 ) {
		sgp->first_frame = bm_load_animation(Hud_shield_filenames[sip->shield_icon_index], &sgp->num_frames);
		if ( sgp->first_frame == -1 ) {
			Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", Hud_shield_filenames[sip->shield_icon_index]);
			return;
		}
	}

	if ( objp == Player_obj ) {
		sx = Player_shield_coords[gr_screen.res][0];
		sy = Player_shield_coords[gr_screen.res][1];
	} else {
		sx = Target_shield_coords[gr_screen.res][0];
		sy = Target_shield_coords[gr_screen.res][1];
	}

	sx += fl2i(HUD_offset_x);
	sy += fl2i(HUD_offset_y);

	// draw the ship first
	if ( objp == Player_obj ) {
		hud_shield_maybe_flash(HUD_PLAYER_SHIELD_ICON, SHIELD_HIT_PLAYER, HULL_HIT_OFFSET);
	} else {
		hud_shield_maybe_flash(HUD_TARGET_SHIELD_ICON, SHIELD_HIT_TARGET, HULL_HIT_OFFSET);
	}

	GR_AABITMAP(sgp->first_frame, sx, sy);	

	// draw the four quadrants
	//
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	max_shield = get_max_shield_quad(objp);

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {

		if ( objp->flags & OF_NO_SHIELDS ) {
			break;
		}

		if ( objp->shield_quadrant[Quadrant_xlate[i]] < 0.1f ) {
			continue;
		}

		range = max(HUD_COLOR_ALPHA_MAX, HUD_color_alpha + 4);
		hud_color_index = fl2i( (objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range + 0.5);
		Assert(hud_color_index >= 0 && hud_color_index <= range);

		if ( hud_color_index < 0 ) {
			hud_color_index = 0;
		}
		if ( hud_color_index >= HUD_NUM_COLOR_LEVELS ) {
			hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
		}

		int flash=0;
		if ( objp == Player_obj ) {
			flash = hud_shield_maybe_flash(HUD_PLAYER_SHIELD_ICON, SHIELD_HIT_PLAYER, i);
		} else {
			flash = hud_shield_maybe_flash(HUD_TARGET_SHIELD_ICON, SHIELD_HIT_TARGET, i);
		}
				
		if ( !flash ) {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			if ( objp == Player_obj ) {
				hud_set_gauge_color(HUD_PLAYER_SHIELD_ICON, hud_color_index);
			} else {
				hud_set_gauge_color(HUD_TARGET_SHIELD_ICON, hud_color_index);
			}

			GR_AABITMAP(sgp->first_frame+i+1, sx, sy);			
		}
	}

	// hud_set_default_color();
}

// called at beginning of level to page in all ship icons
// used in this level
void hud_ship_icon_page_in(ship_info *sip)
{
	hud_frames	*sgp;

	if ( sip->shield_icon_index == 255 ) {
		return;
	}

	// load in shield frames if not already loaded
	Assert(sip->shield_icon_index >= 0 && sip->shield_icon_index < Hud_shield_filename_count);
	sgp = &Shield_gauges[sip->shield_icon_index];

	if ( sgp->first_frame == -1 ) {
		sgp->first_frame = bm_load_animation(Hud_shield_filenames[sip->shield_icon_index], &sgp->num_frames);
		if ( sgp->first_frame == -1 ) {
			Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", Hud_shield_filenames[sip->shield_icon_index]);
			return;
		}
	}

	int i;
	for (i=0; i<sgp->num_frames; i++ )	{
		bm_page_in_aabitmap(sgp->first_frame+i);
	}

}

// ------------------------------------------------------------------
// hud_shield_equalize()
//
// Equalize the four shield quadrants for an object
//
void hud_shield_equalize(object *objp, player *pl)
{
	float	strength;
	int idx;
	int all_equal = 1;

	Assert(objp != NULL);
	if(objp == NULL){
		return;
	}
	Assert(pl != NULL);
	if(pl == NULL){
		return;
	}
	Assert(objp->type == OBJ_SHIP);
	if(objp->type != OBJ_SHIP){
		return;
	}

	// Goober5000 - quick out if we have no shields
	// (mainly to prevent the sound if player presses Q)
	if (objp->flags & OF_NO_SHIELDS)
	{
		return;
	}

	// are all quadrants equal?
	for(idx=0; idx<MAX_SHIELD_SECTIONS-1; idx++){
		if(objp->shield_quadrant[idx] != objp->shield_quadrant[idx+1]){
			all_equal = 0;
			break;
		}
	}

	// not all equal
	if(!all_equal){
		strength = get_shield_strength(objp);
		if ( strength != 0 ) {
			// maybe impose a 2% penalty - server side and single player only
#ifndef NO_NETWORK
			if(!MULTIPLAYER_CLIENT &&  (pl->shield_penalty_stamp < 0) || timestamp_elapsed_safe(pl->shield_penalty_stamp, 1000) ){
#else
			if((pl->shield_penalty_stamp < 0) || timestamp_elapsed_safe(pl->shield_penalty_stamp, 1000) ){
#endif
				strength *= 0.98f;

				// reset the penalty timestamp
				pl->shield_penalty_stamp = timestamp(1000);
			}
			
			set_shield_strength(objp, strength);					
		}
	}

	// beep
	if ( objp == Player_obj ){
		snd_play( &Snds[SND_SHIELD_XFER_OK] );
	}
}

// ------------------------------------------------------------------
// hud_augment_shield_quadrant()
//
// Transfer shield energy to a shield quadrant from the three other
//	quadrants.  Works by trying to transfer a fixed amount of shield
//	energy from the other three quadrants, taking the same percentage
// from each quadrant.
//
//	input:	objp			=>		object to perform shield transfer on
//				direction	=>		which quadrant to augment:
//										0 - right
//										1 - top
//										2 - bottom
//										3 - left
//
void hud_augment_shield_quadrant(object *objp, int direction)
{
	float	xfer_amount, energy_avail, percent_to_take, delta;
	float	max_quadrant_val;
	int	i;

	Assert(direction >= 0 && direction < MAX_SHIELD_SECTIONS);
	Assert(objp->type == OBJ_SHIP);
	
	xfer_amount = Ships[objp->instance].ship_initial_shield_strength * SHIELD_TRANSFER_PERCENT;
	max_quadrant_val = get_max_shield_quad(objp);

	if ( (objp->shield_quadrant[direction] + xfer_amount) > max_quadrant_val )
		xfer_amount = max_quadrant_val - objp->shield_quadrant[direction];

	Assert(xfer_amount >= 0);
	if ( xfer_amount == 0 ) {
		// TODO: provide a feedback sound
		return;
	}
	else {
		snd_play( &Snds[SND_SHIELD_XFER_OK] );
	}

	energy_avail = 0.0f;
	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {
		if ( i == direction )
			continue;
		energy_avail += objp->shield_quadrant[i];
	}

	percent_to_take = xfer_amount/energy_avail;
	if ( percent_to_take > 1.0f )
		percent_to_take = 1.0f;

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {
		if ( i == direction )
			continue;
		delta = percent_to_take * objp->shield_quadrant[i];
		objp->shield_quadrant[i] -= delta;
		Assert(objp->shield_quadrant[i] >= 0 );
		objp->shield_quadrant[direction] += delta;
		if ( objp->shield_quadrant[direction] > max_quadrant_val )
			objp->shield_quadrant[direction] = max_quadrant_val;
	}
}

// Try to find a match between filename and the names inside
// of Hud_shield_filenames.  This will provide us with an 
// association of ship class to shield icon information.
void hud_shield_assign_info(ship_info *sip, char *filename)
{
	ubyte i;

	for ( i = 0; i < Hud_shield_filename_count; i++ ) {
		if ( !stricmp(filename, Hud_shield_filenames[i]) ) {
			sip->shield_icon_index = i;
		}
	}		
}

void hud_show_mini_ship_integrity(object *objp, int x_force, int y_force)
{
	char	text_integrity[64];
	int	numeric_integrity;
	float p_target_integrity,initial_hull;
	int	nx, ny;

	initial_hull = Ships[objp->instance].ship_initial_hull_strength;
	if (  initial_hull <= 0 ) {
		Int3(); // illegal initial hull strength
		p_target_integrity = 0.0f;
	} else {
		p_target_integrity = objp->hull_strength / initial_hull;
		if (p_target_integrity < 0){
			p_target_integrity = 0.0f;
		}
	}

	numeric_integrity = fl2i(p_target_integrity*100 + 0.5f);
	if(numeric_integrity > 100){
		numeric_integrity = 100;
	}
	// Assert(numeric_integrity <= 100);

	// base coords
	nx = (x_force == -1) ? Hud_mini_base[gr_screen.res][0] : x_force;
	ny = (y_force == -1) ? Hud_mini_base[gr_screen.res][1] : y_force;

	// 3 digit hull strength
	if ( numeric_integrity == 100 ) {
		nx += Hud_mini_3digit[gr_screen.res][2];
	} 
	// 2 digit hull strength
	else if ( numeric_integrity < 10 ) {
		nx += Hud_mini_1digit[gr_screen.res][2];		
	}
	// 1 digit hull strength
	else {
		nx += Hud_mini_2digit[gr_screen.res][2];		
	}	

	if ( numeric_integrity == 0 ) {
		if ( p_target_integrity > 0 ) {
			numeric_integrity = 1;
		}
	}

	nx += fl2i( HUD_offset_x );
	ny += fl2i( HUD_offset_y );

	sprintf(text_integrity, "%d", numeric_integrity);
	if ( numeric_integrity < 100 ) {
		hud_num_make_mono(text_integrity);
	}	

	gr_string(nx, ny, text_integrity);
}

// Draw the miniature shield icon that is drawn near the reticle
void hud_shield_show_mini(object *objp, int x_force, int y_force, int x_hull_offset, int y_hull_offset)
{
	float			max_shield;
	int			hud_color_index, range, frame_offset;
	int			sx, sy, i;
	shield_hit_info	*shi;

	shi = &Shield_hit_data[SHIELD_HIT_TARGET];

	if ( objp->type != OBJ_SHIP ) {
		return;
	}

	hud_set_gauge_color(HUD_TARGET_MINI_ICON);

	if (!Shield_mini_loaded)
		return;

	sx = (x_force == -1) ? Shield_mini_coords[gr_screen.res][0]+fl2i(HUD_offset_x) : x_force;
	sy = (y_force == -1) ? Shield_mini_coords[gr_screen.res][1]+fl2i(HUD_offset_y) : y_force;

	// draw the ship first
	hud_shield_maybe_flash(HUD_TARGET_MINI_ICON, SHIELD_HIT_TARGET, HULL_HIT_OFFSET);
	hud_show_mini_ship_integrity(objp,x_force + x_hull_offset,y_force + y_hull_offset);

	// draw the four quadrants
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	max_shield = get_max_shield_quad(objp);

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {

		if ( objp->flags & OF_NO_SHIELDS ) {
			break;
		}

		if ( objp->shield_quadrant[Quadrant_xlate[i]] < 0.1f ) {
			continue;
		}

		if ( hud_shield_maybe_flash(HUD_TARGET_MINI_ICON, SHIELD_HIT_TARGET, i) ) {
			frame_offset = i+MAX_SHIELD_SECTIONS;
		} else {
			frame_offset = i;
		}
				
		range = HUD_color_alpha;
		hud_color_index = fl2i( (objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range + 0.5);
		Assert(hud_color_index >= 0 && hud_color_index <= range);
	
		if ( hud_color_index < 0 ) {
			hud_color_index = 0;
		}
		if ( hud_color_index >= HUD_NUM_COLOR_LEVELS ) {
			hud_color_index = HUD_NUM_COLOR_LEVELS - 1;
		}

		if ( hud_gauge_maybe_flash(HUD_TARGET_MINI_ICON) == 1) {
			// hud_set_bright_color();
			hud_set_gauge_color(HUD_TARGET_MINI_ICON, HUD_C_BRIGHT);
		} else {
			// gr_set_color_fast(&HUD_color_defaults[hud_color_index]);
			hud_set_gauge_color(HUD_TARGET_MINI_ICON, hud_color_index);
		}					 

		GR_AABITMAP(Shield_mini_gauge.first_frame + frame_offset, sx, sy);		
	}
	
	// hud_set_default_color();
}

// reset the shield_hit_info data structure
void shield_info_reset(shield_hit_info *shi)
{
	int i;

	shi->shield_hit_status = 0;
	shi->shield_show_bright = 0;
	for ( i = 0; i < NUM_SHIELD_HIT_MEMBERS; i++ ) {
		shi->shield_hit_timers[i] = 1;
		shi->shield_hit_next_flash[i] = 1;
	}
}

// reset the timers and hit flags for the shield gauges
//
// This needs to be called whenever the player selects a new target
//
// input:	player	=>	optional parameter (default value 0).  This is to indicate that player shield hit
//								info should be reset.  This is normally not the case.
//								is for the player's current target
void hud_shield_hit_reset(int player)
{
	shield_hit_info	*shi;

	if (player) {
		shi = &Shield_hit_data[SHIELD_HIT_PLAYER];
	} else {
		shi = &Shield_hit_data[SHIELD_HIT_TARGET];
	}

	shield_info_reset(shi);
}

// called once per frame to update the state of Shield_hit_status based on the Shield_hit_timers[]
void hud_shield_hit_update()
{
	int i, j, limit;		

	limit = 1;
	if ( Player_ai->target_objnum >= 0 ) {
		limit = 2;
	}

	for ( i = 0; i < limit; i++ ) {
		for ( j = 0; j < NUM_SHIELD_HIT_MEMBERS; j++ ) {
			if ( timestamp_elapsed(Shield_hit_data[i].shield_hit_timers[j]) ) {
				Shield_hit_data[i].shield_hit_status &= ~(1<<j);
				Shield_hit_data[i].shield_show_bright &= ~(1<<j);
			}
		}
	}
}

// called when a shield quadrant is struct, so we can update the timer that will draw the quadrant
// as flashing
//
// input:
//				objp		=>	object pointer for ship that has been hit
//				quadrant	=> quadrant of shield getting hit (-1 if no shield is present)
void hud_shield_quadrant_hit(object *objp, int quadrant)
{
	shield_hit_info	*shi;
	int					num;

	if ( objp->type != OBJ_SHIP )
		return;

	hud_escort_ship_hit(objp, quadrant);
	hud_gauge_popup_start(HUD_TARGET_MINI_ICON);

	if ( OBJ_INDEX(objp) == Player_ai->target_objnum ) {
		shi = &Shield_hit_data[SHIELD_HIT_TARGET];
	} else if ( objp == Player_obj ) {
		shi = &Shield_hit_data[SHIELD_HIT_PLAYER];
	} else {
		return;
	}

	if ( quadrant >= 0 ) {
		num = Quadrant_xlate[quadrant];
		shi->shield_hit_timers[num] = timestamp(300);
	} else {
		shi->shield_hit_timers[HULL_HIT_OFFSET] = timestamp(SHIELD_HIT_DURATION_SHORT);
		hud_targetbox_start_flash(TBOX_FLASH_HULL);
	}
}


void hudshield_page_in()
{
	bm_page_in_aabitmap( Shield_mini_gauge.first_frame, Shield_mini_gauge.num_frames );
}

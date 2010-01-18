/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "hud/hudshield.h"
#include "graphics/2d.h"
#include "object/object.h"
#include "object/objectshield.h"
#include "hud/hud.h"
#include "hud/hudparse.h"
#include "hud/hudtargetbox.h"
#include "playerman/player.h"
#include "gamesnd/gamesnd.h"
#include "io/timer.h"
#include "hud/hudescort.h"
#include "weapon/emp.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "render/3d.h"	//For g3_start_frame
#include "network/multi.h"




#define NUM_SHIELD_LEVELS		8

#define SHIELD_TRANSFER_PERCENT	0.083f		// 1/12 total shield strength

#define SHIELD_HIT_DURATION_SHORT	300	// time a shield quadrant flashes after being hit
#define SHIELD_FLASH_INTERVAL_FAST	200	// time between shield quadrant flashes

// now read in from hud.tbl
#define MAX_SHIELD_ICONS		80	//DTP bumped from 40 to 80
int Hud_shield_filename_count = 0;
char Hud_shield_filenames[MAX_SHIELD_ICONS][MAX_FILENAME_LEN];

hud_frames Shield_gauges[MAX_SHIELD_ICONS];

static int Hud_shield_inited = 0;
/*static int Player_shield_coords[GR_NUM_RESOLUTIONS][2] = 
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
};*/
	
int Shield_mini_loaded = 0;
hud_frames Shield_mini_gauge;

#define	SHIELD_HIT_PLAYER		0
#define	SHIELD_HIT_TARGET		1
static shield_hit_info	Shield_hit_data[2];

// translate between clockwise-from-top shield quadrant ordering to way quadrants are numbered in the game
ubyte Quadrant_xlate[MAX_SHIELD_SECTIONS] = {1,0,2,3};

/*void hud_shield_game_init()
{
	int rval;
	char name[MAX_FILENAME_LEN+1] = "";

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "hud.tbl", rval));
		return;
	}

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
			strcpy_s(Hud_shield_filenames[Hud_shield_filename_count++], name);
		}
	}
}*/

// called at the start of each level from HUD_init.  Use Hud_shield_init so we only init Shield_gauges[] once.
void hud_shield_level_init()
{
	int i;	

	hud_shield_hit_reset(1);	// reset for the player

	if ( !Hud_shield_inited ) {
		for ( i = 0; i < MAX_SHIELD_ICONS; i++ ) {
			Shield_gauges[i].first_frame = -1;
			Shield_gauges[i].num_frames  = 0;
		}
		
		Hud_shield_inited = 1;
	}
	
	Shield_mini_gauge.first_frame = bm_load_animation(current_hud->Shield_mini_fname, &Shield_mini_gauge.num_frames);
	if ( Shield_mini_gauge.first_frame == -1 ) {
		Warning(LOCATION, "Could not load in the HUD shield ani: Shield_mini_fname\n");
		return;
	}
	Shield_mini_loaded = 1;
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
extern int Cmdline_nohtl;
bool shield_ani_warning_displayed_already = false;
void hud_shield_show(object *objp)
{
//	static int fod_model = -1;
	float			max_shield;
	int			hud_color_index, range;
	int			sx, sy, i;
	ship			*sp;
	ship_info	*sip;
	hud_frames	*sgp=NULL;

	if ( objp->type != OBJ_SHIP )
		return;

	// Goober5000 - don't show if primitive sensors
	if ( Ships[Player_obj->instance].flags2 & SF2_PRIMITIVE_SENSORS )
		return;

	sp = &Ships[objp->instance];
	sip = &Ship_info[sp->ship_info_index];

//	bool digitus_improbus = (fod_model != -2 && strstr(sp->ship_name, "Sathanas") != NULL);
	if ( sip->shield_icon_index == 255 && !(sip->flags2 & SIF2_GENERATE_HUD_ICON) /*&& !digitus_improbus*/) {
		return;
	}

	if (objp == Player_obj) {
		hud_set_gauge_color(HUD_PLAYER_SHIELD_ICON);
	} else {
		hud_set_gauge_color(HUD_TARGET_SHIELD_ICON);
	}

	// load in shield frames if not already loaded
	if(sip->shield_icon_index != 255)
	{
		sgp = &Shield_gauges[sip->shield_icon_index];

		if ( sgp->first_frame == -1 && sip->shield_icon_index < Hud_shield_filename_count) {
			sgp->first_frame = bm_load_animation(Hud_shield_filenames[sip->shield_icon_index], &sgp->num_frames);
			if ( sgp->first_frame == -1 ) {
				if(!shield_ani_warning_displayed_already){
					shield_ani_warning_displayed_already = true;
					Warning(LOCATION, "Could not load in the HUD shield ani: %s\n", Hud_shield_filenames[sip->shield_icon_index]);
				}
				return;
			}
		}
	}

	if ( objp == Player_obj ) {
		sx = current_hud->Player_shield_coords[0];
		sy = current_hud->Player_shield_coords[1];
	} else {
		sx = current_hud->Target_shield_coords[0];
		sy = current_hud->Target_shield_coords[1];
	}

	sx += fl2i(HUD_offset_x);
	sy += fl2i(HUD_offset_y);

	// draw the ship first
	if ( objp == Player_obj ) {
		hud_shield_maybe_flash(HUD_PLAYER_SHIELD_ICON, SHIELD_HIT_PLAYER, HULL_HIT_OFFSET);
	} else {
		hud_shield_maybe_flash(HUD_TARGET_SHIELD_ICON, SHIELD_HIT_TARGET, HULL_HIT_OFFSET);
	}

	if(sip->shield_icon_index != 255)
	{
		GR_AABITMAP(sgp->first_frame, sx, sy);
	}
	else
	{
		bool g3_yourself = !g3_in_frame();
		angles rot_angles = {-1.570796327f,0.0f,0.0f};
		matrix	object_orient;

		vm_angles_2_matrix(&object_orient, &rot_angles);

		gr_screen.clip_width = 112;
		gr_screen.clip_height = 93;

		//Fire it up
		if(g3_yourself)
			g3_start_frame(1);
		hud_save_restore_camera_data(1);
		HUD_set_clip(sx, sy, 112, 93);
		model_set_detail_level(1);

		//if(!digitus_improbus)
			g3_set_view_matrix( &sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * 2.5f);
		/*else
		{
			vec3d finger_vec = {0.0f, 0.0f, 176.0f};
			g3_set_view_matrix( &finger_vec, &vmd_identity_matrix, 1.0f);
		}*/

		if (!Cmdline_nohtl) {
			gr_set_proj_matrix(0.5f*Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
			gr_set_view_matrix(&Eye_position, &Eye_matrix);
		}

		//We're ready to show stuff
		ship_model_start(objp);
		//if(!digitus_improbus)
		{
			model_render( sip->model_num, &object_orient, &vmd_zero_vector, MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING, -1, -1, sp->ship_replacement_textures);
		}
		/*else
		{
			if(fod_model == -1)
			{
				fod_model = model_load(NOX("FoD.pof"), 0, NULL);
				if(fod_model == -1)
				{
					fod_model = -2;
					return;
				}
			}
			model_render(fod_model, &object_orient, &vmd_zero_vector, MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING, -1, -1);
		}*/
		ship_model_stop( objp );

		//We're done
		if(!Cmdline_nohtl)
		{
			gr_end_view_matrix();
			gr_end_proj_matrix();
		}
		if(g3_yourself)
			g3_end_frame();
		hud_save_restore_camera_data(0);

		HUD_reset_clip();
	}

	if(!sip->max_shield_strength)
		return;

	// draw the four quadrants
	//
	// Draw shield quadrants at one of NUM_SHIELD_LEVELS
	max_shield = get_max_shield_quad(objp);
	
	int j, x_val, y_val, mid_val;

	for ( i = 0; i < MAX_SHIELD_SECTIONS; i++ ) {

		if ( objp->flags & OF_NO_SHIELDS ) {
			break;
		}

		if ( objp->shield_quadrant[Quadrant_xlate[i]] < 0.1f ) {
			continue;
		}

		range = MAX(HUD_COLOR_ALPHA_MAX, HUD_color_alpha + 4);
		hud_color_index = fl2i( (objp->shield_quadrant[Quadrant_xlate[i]] / max_shield) * range);
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

			if(sip->shield_icon_index != 255)
			{
				GR_AABITMAP(sgp->first_frame+i+1, sx, sy);
			}
			else
			{
				//Ugh, draw four shield quadrants
				switch(i)
				{
					//Top
					case 0:
						sy += 3;
						for(j = 0; j < 6; j++)
						{
							y_val = sy + 10;
							gr_gradient(sx + j,
										sy,
										sx + j,
										y_val - j);
						}
						mid_val = sy + 5;
						for(; j < 106; j++)
						{
							gr_gradient(sx + j,
										sy,
										sx + j,
										mid_val);
						}
						for(; j < 112; j++)
						{
							gr_gradient(sx + j,
										sy,
										sx + j,
										sy + (j - 101));
						}
						y_val = sy - 1;
						sy -= 3;
						for(j = 0; j < 112; j++)
							gr_gradient(sx + j, y_val, sx + j, sy);
						break;
					//Left
					case 1:
						sx += 1;
						x_val = sx + 10;
						y_val = sy + 15;
						for(j = 0; j < 6; j++)
						{
							gr_gradient(sx,
										y_val + j,
										x_val - j,
										y_val + j);
						}
						mid_val = sx + 5;
						for(; j < 48; j++)
						{
							gr_gradient(sx,
										y_val + j,
										mid_val,
										y_val + j);
						}
						for(; j < 54; j++)
						{
							gr_gradient(sx,
										y_val + j,
										sx + (j - 43),
										y_val + j);
						}
						x_val = sx;
						sx -= 3;
						for(j = 0; j < 54; j++)
							gr_gradient(x_val, y_val + j, sx, y_val + j);
						sx += 2;
						break;
					//Right
					case 2:
						x_val = sx + 109;	//-3 for border
						y_val = sy + 15;
						for(j = 0; j < 6; j++)
						{
							gr_gradient(x_val,
										y_val + j,
										x_val - (10 - j),
										y_val + j);
						}
						mid_val = x_val - 5;
						for(; j < 48; j++)
						{
							gr_gradient(x_val,
										y_val + j,
										mid_val,
										y_val + j);
						}
						for(; j < 54; j++)
						{
							gr_gradient(x_val,
										y_val + j,
										x_val - (j - 43),
										y_val + j);
						}
						mid_val = x_val;
						x_val += 3;
						for(j = 0; j < 54; j++)
							gr_gradient(mid_val, y_val + j, x_val, y_val + j);
						break;
					//Bottom
					case 3:
						y_val = sy + 80; //-3 for border
						for(j = 0; j < 6; j++)
							gr_gradient(sx + j,
										y_val,
										sx + j,
										y_val - (10 - j));
						mid_val = y_val - 5;
						for(; j < 106; j++)
							gr_gradient(sx + j,
										y_val,
										sx + j,
										mid_val);
						for(; j < 112; j++)
							gr_gradient(sx + j,
										y_val,
										sx + j,
										y_val - (j - 101));
						mid_val = y_val + 1;
						y_val += 3;
						for(j = 0; j < 112; j++)
							gr_gradient(sx + j, mid_val, sx + j, y_val);
				}
			}
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
	Assert(sip->shield_icon_index < Hud_shield_filename_count);
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
		bm_page_in_aabitmap(sgp->first_frame+i, 1);
	}

}

// ------------------------------------------------------------------
// hud_shield_equalize()
//
// Equalize the four shield quadrants for an object
//
void hud_shield_equalize(object *objp, player *pl)
{
	float strength;
	int idx;
	int all_equal;

	Assert(objp != NULL);
	if (objp == NULL)
		return;

	Assert(pl != NULL);
	if (pl == NULL)
		return;

	Assert(objp->type == OBJ_SHIP);
	if (objp->type != OBJ_SHIP)
		return;

	// Goober5000 - quick out if we have no shields
	if (objp->flags & OF_NO_SHIELDS)
		return;

	// are all quadrants equal?
	all_equal = 1;
	for (idx = 0; idx < MAX_SHIELD_SECTIONS - 1; idx++) {
		if (objp->shield_quadrant[idx] != objp->shield_quadrant[idx + 1]) {
			all_equal = 0;
			break;
		}
	}

	if (all_equal)
		return;

	strength = shield_get_strength(objp);
	if (strength == 0.0f)
		return;

	// maybe impose a 2% penalty - server side and single player only
	if (!MULTIPLAYER_CLIENT && (pl->shield_penalty_stamp < 0) || timestamp_elapsed_safe(pl->shield_penalty_stamp, 1000)) {
		strength *= 0.98f;

		// reset the penalty timestamp
		pl->shield_penalty_stamp = timestamp(1000);
	}
			
	shield_set_strength(objp, strength);					

	// beep
	if (objp == Player_obj) {
		snd_play(&Snds[SND_SHIELD_XFER_OK]);
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
	
	xfer_amount = Ships[objp->instance].ship_max_shield_strength * SHIELD_TRANSFER_PERCENT;
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
			return;
		}
	}

	//No HUD icon found. Add one!
	Assert(Hud_shield_filename_count < MAX_SHIELD_ICONS);
	if(Hud_shield_filename_count < MAX_SHIELD_ICONS){
		sip->shield_icon_index = (unsigned char) Hud_shield_filename_count;
		strcpy_s(Hud_shield_filenames[Hud_shield_filename_count++], filename);
	}
}

void hud_show_mini_ship_integrity(object *objp, int x_force, int y_force)
{
	char	text_integrity[64];
	int	numeric_integrity;
	float p_target_integrity;
	int	final_pos[2];

	p_target_integrity = get_hull_pct(objp);

	numeric_integrity = fl2i(p_target_integrity*100 + 0.5f);
	if(numeric_integrity > 100){
		numeric_integrity = 100;
	}
	// Assert(numeric_integrity <= 100);

	// 3 digit hull strength
	if ( numeric_integrity == 100 ) {
		memcpy(final_pos, current_hud->Hud_mini_3digit, sizeof(final_pos));
	} 
	// 1 digit hull strength
	else if ( numeric_integrity < 10 ) {
		memcpy(final_pos, current_hud->Hud_mini_1digit, sizeof(final_pos));		
	}
	// 2 digit hull strength
	else {
		memcpy(final_pos, current_hud->Hud_mini_2digit, sizeof(final_pos));
	}	

	if ( numeric_integrity == 0 ) {
		if ( p_target_integrity > 0 ) {
			numeric_integrity = 1;
		}
	}

	final_pos[0] += fl2i( HUD_offset_x );
	final_pos[1] += fl2i( HUD_offset_y );

	sprintf(text_integrity, "%d", numeric_integrity);
	if ( numeric_integrity < 100 ) {
		hud_num_make_mono(text_integrity);
	}	

	gr_string(final_pos[0] + HUD_nose_x, final_pos[1] + HUD_nose_y, text_integrity);
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

	sx = (x_force == -1) ? current_hud->Shield_mini_coords[0]+fl2i(HUD_offset_x) : x_force;
	sy = (y_force == -1) ? current_hud->Shield_mini_coords[1]+fl2i(HUD_offset_y) : y_force;

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

		GR_AABITMAP(Shield_mini_gauge.first_frame + frame_offset, sx + HUD_nose_x, sy + HUD_nose_y);		
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

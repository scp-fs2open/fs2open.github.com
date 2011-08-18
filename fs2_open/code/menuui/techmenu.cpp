/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "gamesequence/gamesequence.h"
#include "ui/ui.h"
#include "menuui/techmenu.h"
#include "render/3d.h"
#include "io/key.h"
#include "missionui/missionscreencommon.h"
#include "gamesnd/gamesnd.h"
#include "graphics/font.h"
#include "io/mouse.h"
#include "ui/uidefs.h"
#include "gamehelp/contexthelp.h"
#include "globalincs/alphacolors.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "localization/localize.h"
#include "lighting/lighting.h"
#include "sound/fsspeech.h"
#include "playerman/player.h"
#include "parse/parselo.h"
#include "ship/ship.h"
#include "weapon/weapon.h"



#define REVOLUTION_RATE	5.2f

#define NUM_BUTTONS	16
#define NUM_TABS		3
#define LIST_BUTTONS_MAX	42

#define SHIPS_DATA_MODE		(1<<0)
#define WEAPONS_DATA_MODE	(1<<1)
#define SPECIES_DATA_MODE	(1<<2)
#define WEAPONS_SPECIES_DATA_MODE	(WEAPONS_DATA_MODE | SPECIES_DATA_MODE)

#define SHIPS_DATA_TAB					0
#define WEAPONS_DATA_TAB				1
#define INTEL_DATA_TAB					2
#define TECH_DATABASE_TAB				3
#define SIMULATOR_TAB					4
#define CUTSCENES_TAB					5
#define CREDITS_TAB						6

#define SCROLL_LIST_UP					7
#define SCROLL_LIST_DOWN				8
#define SCROLL_INFO_UP					9
#define SCROLL_INFO_DOWN				10

#define PREV_ENTRY_BUTTON				11
#define NEXT_ENTRY_BUTTON				12

#define HELP_BUTTON						13
#define OPTIONS_BUTTON					14
#define EXIT_BUTTON						15


#define REPEAT						(1<<0)
#define NO_MOUSE_OVER_SOUND	(1<<1)

// indicies for coords
#define SHIP_X_COORD 0 
#define SHIP_Y_COORD 1
#define SHIP_W_COORD 2
#define SHIP_H_COORD 3

// background filename for species
// note weapon filename is now same as ship filename
char *Tech_background_filename[GR_NUM_RESOLUTIONS] = {
	"TechShipData",
	"2_TechShipData"
};
char *Tech_mask_filename[GR_NUM_RESOLUTIONS] = {
	"TechShipData-M",
	"2_TechShipData-M"
};
char *Tech_slider_filename[GR_NUM_RESOLUTIONS] = {
	"slider",
	"2_slider"
};

int Tech_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		27, 98, 161, 234
	},
	{ // GR_1024
		43, 157, 253, 374
	}
};

int Tech_ship_display_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		239, 98, 393, 222
	},
	{ // GR_1024
		382, 158, 629, 355
	}
};

int Tech_desc_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		29, 347, 365, 125
	},
	{ // GR_1024
		47, 555, 584, 200
	}
};

int Tech_ani_centre_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		416, 215
	},
	{ // GR_1024
		669, 345
	}
};

int Tech_slider_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		2, 118, 20, 194
	},
	{ // GR_1024
		3, 190, 32, 310
	}
};

#define MAX_TEXT_LINES		100
#define MAX_TEXT_LINE_LEN	256

struct techroom_buttons {
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	int tab;
	int flags;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	techroom_buttons(char *name, int x1, int y1, int xt1, int yt1, int h, int t, int f = 0) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), tab(t), flags(f) {}
};

static techroom_buttons Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
	{	// GR_640
		techroom_buttons("TDB_04",	406,	384,	447,	393,	4,	-1),											// ship data tab
		techroom_buttons("TDB_05",	404,	418,	447,	429,	5,	-1),											// weapons data tab
		techroom_buttons("TDB_06",	404,	447,	447,	461,	6,	-1),											// species data tab
		techroom_buttons("TDB_00",	7,		3,		37,	7,		0,	-1),											// technical database tab
		techroom_buttons("TDB_01",	7,		18,	37,	23,	1,	-1),											// mission simulator tab
		techroom_buttons("TDB_02",	7,		34,	37,	38,	2,	-1),											// cutscenes tab
		techroom_buttons("TDB_03",	7,		49,	37,	54,	3,	-1),											// credits tab
		techroom_buttons("TDB_07",	1,		86,	-1,	-1,	7,	SHIPS_DATA_MODE, REPEAT),				// prev data entry
		techroom_buttons("TDB_08",	1,		317,	-1,	-1,	8,	SHIPS_DATA_MODE, REPEAT),				// next data entry
		techroom_buttons("TDB_09",	1,		406,	-1,	-1,	9,	SHIPS_DATA_MODE, REPEAT),				// prev data entry
		techroom_buttons("TDB_10",	1,		447,	-1,	-1,	10,	SHIPS_DATA_MODE, REPEAT),			// next data entry
		techroom_buttons("TDB_11a",559,	323,	-1,	-1,	11,	SHIPS_DATA_MODE, REPEAT),			// prev data entry
		techroom_buttons("TDB_12a",609,	323,	-1,	-1,	12,	SHIPS_DATA_MODE, REPEAT),			// next data entry
		techroom_buttons("TDB_13",	533,	425,	500,	440,	13,	-1),										// help
		techroom_buttons("TDB_14",	533,	455,	479,	464,	14,	-1),										// options
		techroom_buttons("TDB_15a",571,	425,	588,	413,	15,	-1),										// exit		
	}, 
	{	// GR_1024
		techroom_buttons("2_TDB_04",	649,	614,	717,	630,	4,	-1),										// ship data tab
		techroom_buttons("2_TDB_05",	646,	669,	717,	687,	5,	-1),										// weapons data tab
		techroom_buttons("2_TDB_06",	646,	716,	717,	739,	6,	-1),										// species data tab
		techroom_buttons("2_TDB_00",	12,	5,		59,	12,	0,	-1),										// technical database tab
		techroom_buttons("2_TDB_01",	12,	31,	59,	37,	1,	-1),										// mission simulator tab
		techroom_buttons("2_TDB_02",	12,	56,	59,	62,	2,	-1),										// cutscenes tab
		techroom_buttons("2_TDB_03",	12,	81,	59,	88,	3,	-1),										// credits tab
		techroom_buttons("2_TDB_07",	1,		138,	-1,	-1,	7,	SHIPS_DATA_MODE, REPEAT),			// prev data entry
		techroom_buttons("2_TDB_08",	1,		507,	-1,	-1,	8,	SHIPS_DATA_MODE, REPEAT),			// next data entry
		techroom_buttons("2_TDB_09",	1,		649,	-1,	-1,	9,	SHIPS_DATA_MODE, REPEAT),			// prev data entry
		techroom_buttons("2_TDB_10",	1,		716,	-1,	-1,	10,	SHIPS_DATA_MODE, REPEAT),		// next data entry
		techroom_buttons("2_TDB_11a",	895,	518,	-1,	-1,	11,	SHIPS_DATA_MODE, REPEAT),		// prev data entry
		techroom_buttons("2_TDB_12a",	974,	518,	-1,	-1,	12,	SHIPS_DATA_MODE, REPEAT),		// next data entry
		techroom_buttons("2_TDB_13",	854,	681,	800,	704,	13,	-1),									// help
		techroom_buttons("2_TDB_14",	854,	728,	780,	743,	14,	-1),									// options
		techroom_buttons("2_TDB_15a",	914,	681,	930,	660,	15,	-1),									// exit		
	}, 
};

static UI_WINDOW Ui_window;
static UI_BUTTON View_window;
static int Tech_background_bitmap;
static int Tab = SHIPS_DATA_TAB;
static int List_offset;
static int Select_tease_line;
static int Trackball_mode = 1;
static int Trackball_active = 0;
static matrix Techroom_ship_orient = IDENTITY_MATRIX;
static int Techroom_show_all = 0;

static int Text_size;
static int Text_offset;
static int Text_line_size[MAX_TEXT_LINES];
static char *Text_lines[MAX_TEXT_LINES];

static int Cur_entry = -1;				// this is the current entry selected, using entry indexing
static int Cur_entry_index = -1;		// this is the current entry selected, using master list indexing
static int Techroom_ship_modelnum;
static float Techroom_ship_rot;
static UI_BUTTON List_buttons[LIST_BUTTONS_MAX];  // buttons for each line of text in list
static int Palette_bmp;

static int Ships_loaded = 0;
static int Weapons_loaded = 0;
static int Intel_loaded = 0;

// out entry data struct & vars
typedef struct {
	int	index;		// index into the master table that its in (ie Ship_info[])
	char* name;			// ptr to name string
	char* desc;			// ptr to description string
	char tech_anim_filename[MAX_FILENAME_LEN];	//duh
	generic_anim animation;	// animation info
	int	bitmap;		// bitmap handle
	int	has_anim;	// flag to indicate the presence of an animation for this item
	int model_num;	// model reference handle
	int textures_loaded;	// if the model has textures loaded for it or not (hacky mem management)
} tech_list_entry;

static tech_list_entry *Ship_list = NULL;
static int Ship_list_size = 0;
static tech_list_entry *Weapon_list = NULL;
static int Weapon_list_size = 0;
static tech_list_entry Intel_list[MAX_INTEL_ENTRIES];
static int Intel_list_size = 0;
static tech_list_entry *Current_list;								// points to currently valid display list
static int Current_list_size = 0;

// slider stuff
static UI_SLIDER2 Tech_slider;

// Intelligence master data structs (these get inited @ game startup from species.tbl)
intel_data Intel_info[MAX_INTEL_ENTRIES];
int Intel_info_size = 0;

// some prototypes to make you happy
int techroom_load_ani(anim **animpp, char *name);
void tech_common_render();
void tech_scroll_list_up();
void tech_scroll_list_down();


//stuff for ht&l, vars and such
extern int Cmdline_nohtl;

////////////////////////////////////////////////////
// like, functions and stuff

void techroom_init_desc(char *src, int w)
{
	Text_size = Text_offset = 0;
	if (!src) {
		return;
	}

	Text_size = split_str(src, w, Text_line_size, Text_lines, MAX_TEXT_LINES);
	Assert(Text_size >= 0 && Text_size < MAX_TEXT_LINES);
}

void techroom_unload_animation()
{
	int i;

	//clear everything, just in case, it will get loaded when needed later
	if (Weapon_list != NULL) {
		for (i = 0; i < Weapon_list_size; i++) {
			if (Weapon_list[i].animation.num_frames != 0) {
				generic_anim_unload(&Weapon_list[i].animation);
			}

			if (Weapon_list[i].bitmap >= 0) {
				bm_release(Weapon_list[i].bitmap);
				Weapon_list[i].bitmap = -1;
			}
		}
	}

	for (i = 0; i < Intel_list_size; i++) {
		if (Intel_list[i].animation.num_frames != 0) {
			generic_anim_unload(&Intel_list[i].animation);
		}

		if (Intel_list[i].bitmap >= 0) {
			bm_release(Intel_list[i].bitmap);
			Intel_list[i].bitmap = -1;
		}
	}
}

void techroom_select_new_entry()
{
	Assert(Current_list != NULL);
	if (Current_list == NULL || Current_list_size <= 0) {
		Cur_entry_index = Cur_entry = -1;
		techroom_init_desc(NULL,0);
		return;
	}

	Cur_entry_index = Current_list[Cur_entry].index;
	Assert( Cur_entry_index >= 0 );

	// if we are in the ships tab, load the ship model
	if (Tab == SHIPS_DATA_TAB) {
		ship_info *sip = &Ship_info[Cur_entry_index];

		// little memory management, kinda hacky but it should keep the techroom at around
		// 100meg rather than the 700+ it can get to with all ships loaded - taylor
		for (int i=0; i<Current_list_size; i++) {
			if ((Current_list[i].model_num > -1) && (Current_list[i].textures_loaded)) {
				// don't unload any spot within 5 of current
				if ( (i < Cur_entry + 5) && (i > Cur_entry - 5) )
					continue;

				mprintf(("TECH ROOM: Dumping excess ship textures...\n"));

				model_page_out_textures(Current_list[i].model_num);

				Current_list[i].textures_loaded = 0;
			}
		}

		Techroom_ship_modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);

		Current_list[Cur_entry].model_num = Techroom_ship_modelnum;

		// page in ship textures properly (takes care of nondimming pixels)
		model_page_in_textures(Techroom_ship_modelnum, Cur_entry_index);

		Current_list[Cur_entry].textures_loaded = 1;
	} else {
		Techroom_ship_modelnum = -1;
		Trackball_mode = 0;

		// load animation here, we now only have one loaded
		int stream_result = generic_anim_init_and_stream(&Current_list[Cur_entry].animation, Current_list[Cur_entry].tech_anim_filename, bm_get_type(Tech_background_bitmap), true);

		if (stream_result >= 0) {
			Current_list[Cur_entry].has_anim = 1;
		} else {
			// we've failed to load any animation
			// load an image and treat it like a 1 frame animation
			Current_list[Cur_entry].bitmap = bm_load(Current_list[Cur_entry].tech_anim_filename);
		}
	}

	techroom_init_desc(Current_list[Cur_entry].desc, Tech_desc_coords[gr_screen.res][SHIP_W_COORD]);
	fsspeech_play(FSSPEECH_FROM_TECHROOM, Current_list[Cur_entry].desc);
}

// write out the current description in the bottom window
void techroom_render_desc(int xo, int yo, int ho)
{
	int y, z, len, font_height;
	char line[MAX_TEXT_LINE_LEN + 1];

	font_height = gr_get_font_height();

	y = 0;
	z = Text_offset;
	while (y + font_height <= ho) {
		if (z >= Text_size){
			break;
		}

		len = Text_line_size[z];
		if (len > MAX_TEXT_LINE_LEN){
			len = MAX_TEXT_LINE_LEN;
		}

		strncpy(line, Text_lines[z], len);
		line[len] = 0;
		gr_string(xo, yo + y, line);

		y += font_height;
		z++;
	}

	// maybe output 'more' indicator
	if ( z < Text_size ) {
		// can be scrolled down
		int more_txt_x = Tech_desc_coords[gr_screen.res][0] + (Tech_desc_coords[gr_screen.res][2]/2) - 10;	// FIXME should move these to constants since they don't move
		int more_txt_y = Tech_desc_coords[gr_screen.res][1] + Tech_desc_coords[gr_screen.res][3];				// located below brief text, centered
		int w, h;
		gr_get_string_size(&w, &h, XSTR("more", 1469), strlen(XSTR("more", 1469)));
		gr_set_color_fast(&Color_black);
		gr_rect(more_txt_x-2, more_txt_y, w+3, h);
		gr_set_color_fast(&Color_red);
		gr_string(more_txt_x, more_txt_y, XSTR("more", 1469));  // base location on the input x and y?
	}

}

// renders the stuff common to all 3 tech room tabs
void tech_common_render()
{
	char buf[256];
	int y, z, font_height;

	// render description in its box
	gr_set_color_fast(&Color_text_normal);
	techroom_render_desc(Tech_desc_coords[gr_screen.res][SHIP_X_COORD], Tech_desc_coords[gr_screen.res][SHIP_Y_COORD], Tech_desc_coords[gr_screen.res][SHIP_H_COORD]);

	font_height = gr_get_font_height();

	// draw the list of entries
	y = 0;
	z = List_offset;
	while (y + font_height <= Tech_list_coords[gr_screen.res][SHIP_H_COORD]) {
		if (z >= Current_list_size) {
			break;
		}

		if (z == Cur_entry) {
			gr_set_color_fast(&Color_text_selected);
		} else if (z == Select_tease_line) {
			gr_set_color_fast(&Color_text_subselected);
		} else {
			gr_set_color_fast(&Color_text_normal);
		}

		memset( buf, 0, sizeof(buf) );
		strncpy(buf, Current_list[z].name, sizeof(buf) - 1);

		if (Lcl_gr)
			lcl_translate_ship_name(buf);

		gr_force_fit_string(buf, 255, Tech_list_coords[gr_screen.res][SHIP_W_COORD]);
		gr_string(Tech_list_coords[gr_screen.res][SHIP_X_COORD], Tech_list_coords[gr_screen.res][SHIP_Y_COORD] + y, buf);

		List_buttons[z - List_offset].update_dimensions(Tech_list_coords[gr_screen.res][SHIP_X_COORD], Tech_list_coords[gr_screen.res][SHIP_Y_COORD] + y, Tech_list_coords[gr_screen.res][SHIP_W_COORD], font_height);
		List_buttons[z - List_offset].enable(1);

		y += font_height;
		z++;
	}

	// disable the rest of the list buttons
	z -= List_offset;
	while (z < LIST_BUTTONS_MAX) {
		List_buttons[z++].disable();
	}
}

void techroom_ships_render(float frametime)
{
	// render all the common stuff
	tech_common_render();
	
	if(Cur_entry_index == -1)
		return;

	// now render the trackball ship, which is unique to the ships tab
	float rev_rate = REVOLUTION_RATE;
	angles rot_angles, view_angles;
	int z;
	ship_info *sip = &Ship_info[Cur_entry_index];

	// get correct revolution rate
	z = sip->flags;
	if (z & SIF_BIG_SHIP) {
		rev_rate *= 1.7f;
	}
	if (z & SIF_HUGE_SHIP) {
		rev_rate *= 3.0f;
	}

	// rotate the ship as much as required for this frame
	Techroom_ship_rot += PI2 * frametime / rev_rate;
	while (Techroom_ship_rot > PI2){
		Techroom_ship_rot -= PI2;	
	}

	//	reorient ship
	if (Trackball_active) {
		int dx, dy;
		matrix mat1, mat2;

		if (Trackball_active) {
			mouse_get_delta(&dx, &dy);
			if (dx || dy) {
				vm_trackball(-dx, -dy, &mat1);
				vm_matrix_x_matrix(&mat2, &mat1, &Techroom_ship_orient);
				Techroom_ship_orient = mat2;
			}
		}

	} else {
		// setup stuff needed to render the ship
		view_angles.p = -0.6f;
		view_angles.b = 0.0f;
		view_angles.h = 0.0f;
		vm_angles_2_matrix(&Techroom_ship_orient, &view_angles);

		rot_angles.p = 0.0f;
		rot_angles.b = 0.0f;
		rot_angles.h = Techroom_ship_rot;
		vm_rotate_matrix_by_angles(&Techroom_ship_orient, &rot_angles);
	}

	gr_set_clip(Tech_ship_display_coords[gr_screen.res][SHIP_X_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_Y_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_W_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_H_COORD]);	

	// render the ship
	g3_start_frame(1);
	g3_set_view_matrix(&sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * 1.3f);

	if (!Cmdline_nohtl) {
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	// lighting for techroom
	light_reset();
	vec3d light_dir = vmd_zero_vector;
	light_dir.xyz.y = 1.0f;	
	light_add_directional(&light_dir, 0.85f, 1.0f, 1.0f, 1.0f);
	light_rotate_all();
	// lighting for techroom

	model_clear_instance(Techroom_ship_modelnum);
	model_set_detail_level(0);
	model_render(Techroom_ship_modelnum, &Techroom_ship_orient, &vmd_zero_vector, MR_LOCK_DETAIL | MR_AUTOCENTER);

	if (!Cmdline_nohtl)
	{
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	g3_end_frame();

	gr_reset_clip();
}

// select previous entry in current list
void tech_prev_entry()
{
	//unload the current animation, we load another one for the new current entry
	techroom_unload_animation();

	Cur_entry--;
	if (Cur_entry < 0) {
		Cur_entry = Current_list_size - 1;

		// scroll to end of list
		List_offset = Cur_entry - Tech_list_coords[gr_screen.res][SHIP_H_COORD] / gr_get_font_height() + 1;
		if (List_offset < 0) {
			// this happens when there are not enough items to scroll
			List_offset = 0;
		}
		Tech_slider.force_currentItem(Tech_slider.get_numberItems());
	} else {
		// maybe adjust list position by 1
		if (List_offset > Cur_entry) {
			tech_scroll_list_up();
			Tech_slider.forceUp();
		}
	}

	techroom_select_new_entry();
	gamesnd_play_iface(SND_SCROLL);
}

// select next entry in current list
void tech_next_entry()
{
	//unload the current animation, we load another one for the new current entry
	techroom_unload_animation();

	Cur_entry++;
	if (Cur_entry >= Current_list_size) {
		Cur_entry = 0;

		// scroll to beginning of list
		List_offset = 0;
		Tech_slider.force_currentItem(Cur_entry);
	} else {
		// maybe adjust list position by 1
		if (List_offset + Tech_list_coords[gr_screen.res][SHIP_H_COORD] / gr_get_font_height() <= Cur_entry) {
			tech_scroll_list_down();
			Tech_slider.forceDown();
		}
	}

	techroom_select_new_entry();
	gamesnd_play_iface(SND_SCROLL);
}

void tech_scroll_info_up()
{
	if (Text_offset) {
		Text_offset--;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void tech_scroll_info_down()
{
	int h;

	if (Tab == SHIPS_DATA_TAB){
		h = Tech_desc_coords[gr_screen.res][SHIP_H_COORD];
	} else {
		h = Tech_desc_coords[gr_screen.res][3];
	}

	if (Text_offset + h / gr_get_font_height() < Text_size) {
		Text_offset++;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void tech_scroll_list_up()
{
	//int last;

	if (List_offset > 0) {
		List_offset--;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void tech_scroll_list_down()
{
	if (List_offset + Tech_list_coords[gr_screen.res][SHIP_H_COORD] / gr_get_font_height() < Current_list_size) {
		List_offset++;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// this doesn't do a thing...
void tech_ship_scroll_capture()
{
	//unload the current animation, we load another one for the new current entry
	techroom_unload_animation();

	techroom_select_new_entry();
}

void techroom_anim_render(float frametime)
{
	int x, y;

	// render common stuff
	tech_common_render();

	// render the animation
	if(Current_list[Cur_entry].animation.num_frames > 0)
	{
		//grab dimensions
		bm_get_info((Current_list[Cur_entry].animation.streaming) ? Current_list[Cur_entry].animation.bitmap_id : Current_list[Cur_entry].animation.first_frame, &x, &y, NULL, NULL, NULL);
		//get the centre point - adjust
		x = Tech_ani_centre_coords[gr_screen.res][0] - x / 2;
		y = Tech_ani_centre_coords[gr_screen.res][1] - y / 2;
		generic_anim_render(&Current_list[Cur_entry].animation, frametime, x, y);
	}
	// if our active item has a bitmap instead of an animation, draw it
	else if((Cur_entry >= 0) && (Current_list[Cur_entry].bitmap >= 0)){
		//grab dimensions
		bm_get_info(Current_list[Cur_entry].bitmap, &x, &y, NULL, NULL, NULL);
		//get the centre point - adjust
		x = Tech_ani_centre_coords[gr_screen.res][0] - x / 2;
		y = Tech_ani_centre_coords[gr_screen.res][1] - y / 2;
		gr_set_bitmap(Current_list[Cur_entry].bitmap);
		gr_bitmap(x, y);
	}
}

void techroom_change_tab(int num)
{
	int i, multi = 0, mask, mask2, font_height, max_num_entries_viewable;	

	//unload the current animation, we load another one for the new current entry
	if(Tab != SHIPS_DATA_TAB)
		techroom_unload_animation();

	Tab = num;
	List_offset = 0;
	Cur_entry = 0;
	multi = Player->flags & PLAYER_FLAGS_IS_MULTI;

	for (i=0; i<LIST_BUTTONS_MAX; i++){
		List_buttons[i].disable();
	}

	// disable some stuff in multiplayer
	if (Player->flags & PLAYER_FLAGS_IS_MULTI) {
		Buttons[gr_screen.res][SIMULATOR_TAB].button.disable();
		Buttons[gr_screen.res][CUTSCENES_TAB].button.disable();
	}

	switch (Tab) {
		case SHIPS_DATA_TAB:
			mask = multi ? SIF_IN_TECH_DATABASE_M : SIF_IN_TECH_DATABASE;
			mask2 = multi ? SIF2_DEFAULT_IN_TECH_DATABASE_M : SIF2_DEFAULT_IN_TECH_DATABASE;
			
			// load ship info if necessary
			if ( Ships_loaded == 0 ) {
				if (Ship_list == NULL) {
					Ship_list = new tech_list_entry[Num_ship_classes];

					if (Ship_list == NULL)
						Error(LOCATION, "Couldn't init ships list!");
				}

				Ship_list_size = 0;

				for (i=0; i<Num_ship_classes; i++)
				{
					if (Techroom_show_all || (Ship_info[i].flags & mask) || (Ship_info[i].flags2 & mask2))
					{
						// this ship should be displayed, fill out the entry struct
						Ship_list[Ship_list_size].bitmap = -1;
						Ship_list[Ship_list_size].index = i;
						Ship_list[Ship_list_size].animation.num_frames = 0;			// no anim for ships
						Ship_list[Ship_list_size].has_anim = 0;				// no anim for ships
						Ship_list[Ship_list_size].name = *Ship_info[i].tech_title ? Ship_info[i].tech_title : (*Ship_info[i].alt_name ? Ship_info[i].alt_name : Ship_info[i].name);
						Ship_list[Ship_list_size].desc = Ship_info[i].tech_desc;
						Ship_list[Ship_list_size].model_num = -1;
						Ship_list[Ship_list_size].textures_loaded = 0;

						Ship_list_size++;
					}				
				}

				// make sure that at least the default entry is cleared out if we didn't grab anything
				if (Num_ship_classes && !Ship_list_size) {
					Ship_list[0].index = -1;
					Ship_list[0].desc = NULL;
					Ship_list[0].name = NULL;
					Ship_list[0].bitmap = -1;
					Ship_list[0].has_anim = 0;
					Ship_list[0].animation.num_frames = 0;
					Ship_list[0].model_num = -1;
					Ship_list[0].textures_loaded = 0;
				}

				Ships_loaded = 1;
			}

			Current_list = Ship_list;
			Current_list_size = Ship_list_size;

			font_height = gr_get_font_height();
			max_num_entries_viewable = Tech_list_coords[gr_screen.res][SHIP_H_COORD] / font_height;
			Tech_slider.set_numberItems(Current_list_size > max_num_entries_viewable ? Current_list_size-max_num_entries_viewable : 0);

			// no anim to start here
			break;

		case WEAPONS_DATA_TAB:
				
			// load weapon info & anims if necessary
			if ( Weapons_loaded == 0 ) {
				if (Weapon_list == NULL) {
					Weapon_list = new tech_list_entry[Num_weapon_types];

					if (Weapon_list == NULL)
						Error(LOCATION, "Couldn't init ships list!");
				}

				Weapon_list_size = 0;
				mask = multi ? WIF_PLAYER_ALLOWED : WIF_IN_TECH_DATABASE;
				mask2 = WIF2_DEFAULT_IN_TECH_DATABASE;

				for (i=0; i<Num_weapon_types; i++)
				{
					if (Techroom_show_all || (Weapon_info[i].wi_flags & mask) || (Weapon_info[i].wi_flags2 & mask2))
					{ 
						// we have a weapon that should be in the tech db, so fill out the entry struct
						Weapon_list[Weapon_list_size].index = i;
						Weapon_list[Weapon_list_size].desc = Weapon_info[i].tech_desc;
						Weapon_list[Weapon_list_size].has_anim = 1;
						Weapon_list[Weapon_list_size].name = *Weapon_info[i].tech_title ? Weapon_info[i].tech_title : Weapon_info[i].name;
						Weapon_list[Weapon_list_size].bitmap = -1;
						Weapon_list[Weapon_list_size].animation.num_frames = 0;
						Weapon_list[Weapon_list_size].model_num = -1;
						Weapon_list[Weapon_list_size].textures_loaded = 0;
						// copy the weapon animation filename
						strncpy(Weapon_list[Weapon_list_size].tech_anim_filename, Weapon_info[i].tech_anim_filename, MAX_FILENAME_LEN - 1);

						Weapon_list_size++;
					}				
				}

				// make sure that at least the default entry is cleared out if we didn't grab anything
				if (Num_weapon_types && !Weapon_list_size) {
					Weapon_list[0].index = -1;
					Weapon_list[0].desc = NULL;
					Weapon_list[0].name = NULL;
					Weapon_list[0].bitmap = -1;
					Weapon_list[0].has_anim = 0;
					Weapon_list[0].animation.num_frames = 0;
					Weapon_list[0].model_num = -1;
					Weapon_list[0].textures_loaded = 0;
				}

				Weapons_loaded = 1;
			}

			Current_list = Weapon_list;
			Current_list_size = Weapon_list_size;

			font_height = gr_get_font_height();
			max_num_entries_viewable = Tech_list_coords[gr_screen.res][SHIP_H_COORD] / font_height;
			Tech_slider.set_numberItems(Current_list_size > max_num_entries_viewable ? Current_list_size-max_num_entries_viewable : 0);

			break;

		case INTEL_DATA_TAB:

			// load intel if necessary
			if ( Intel_loaded == 0 ) {
				// now populate the entry structs
				Intel_list_size = 0;

				for (i=0; i<Intel_info_size; i++) {
					if (Techroom_show_all || (Intel_info[i].flags & IIF_IN_TECH_DATABASE) || (Intel_info[i].flags & IIF_DEFAULT_IN_TECH_DATABASE)) {
						// leave option for no animation if string == "none"
						if (!strcmp(Intel_info[i].anim_filename, "none")) {
							Intel_list[Intel_list_size].has_anim = 0;
							Intel_list[Intel_list_size].animation.num_frames = 0;
						} else {
							// try and load as an animation
							Intel_list[Intel_list_size].has_anim = 0;
							Intel_list[Intel_list_size].bitmap = -1;
							strncpy(Intel_list[Intel_list_size].tech_anim_filename, Intel_info[i].anim_filename, NAME_LENGTH - 1);
						}

						Intel_list[Intel_list_size].desc = Intel_info[i].desc;
						Intel_list[Intel_list_size].index = i;
						Intel_list[Intel_list_size].name = Intel_info[i].name;
						Intel_list[Intel_list_size].model_num = -1;
						Intel_list[Intel_list_size].textures_loaded = 0;

						Intel_list_size++;
					}
				}

				// make sure that at least the default entry is cleared out if we didn't grab anything
				if (Intel_info_size && !Intel_list_size) {
					Intel_list[0].index = -1;
					Intel_list[0].desc = NULL;
					Intel_list[0].name = NULL;
					Intel_list[0].bitmap = -1;
					Intel_list[0].has_anim = 0;
					Intel_list[0].animation.num_frames = 0;
					Intel_list[0].model_num = -1;
					Intel_list[0].textures_loaded = 0;
				}

				Intel_loaded = 1;
			}

			// index lookup on intel is a pretty pointless, but it keeps everything 
			// consistent and doesn't really hurt anything
			Current_list = Intel_list;
			Current_list_size = Intel_list_size;

			font_height = gr_get_font_height();
			max_num_entries_viewable = Tech_list_coords[gr_screen.res][SHIP_H_COORD] / font_height;
			Tech_slider.set_numberItems(Current_list_size > max_num_entries_viewable ? Current_list_size-max_num_entries_viewable : 0);

			break;
	}

	// reset the entry
	Cur_entry = 0;
	techroom_select_new_entry();

}

int techroom_button_pressed(int num)
{
	switch (num) {
		case SHIPS_DATA_TAB:
		case WEAPONS_DATA_TAB:
		case INTEL_DATA_TAB:
			fsspeech_stop();
			techroom_change_tab(num);
			break;

		case SIMULATOR_TAB:
			fsspeech_stop();
#if !defined(E3_BUILD) && !defined(PD_BUILD)
			gamesnd_play_iface(SND_SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_SIMULATOR_ROOM);
			return 1;
#else
			return 0;
#endif

		case CUTSCENES_TAB:
			fsspeech_stop();
#if !defined(E3_BUILD) && !defined(PD_BUILD)
			gamesnd_play_iface(SND_SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN);
			return 1;
#else
			return 0;
#endif

		case CREDITS_TAB:
			fsspeech_stop();
#if !defined(E3_BUILD) && !defined(PD_BUILD)
			gamesnd_play_iface(SND_SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_CREDITS);
			return 1;
#else 
			return 0;
#endif

		case PREV_ENTRY_BUTTON:
			tech_prev_entry();
			break;

		case NEXT_ENTRY_BUTTON:
			tech_next_entry();
			break;

		case SCROLL_LIST_UP:
			tech_scroll_list_up();
			Tech_slider.forceUp();
			break;

		case SCROLL_LIST_DOWN:
			tech_scroll_list_down();
			Tech_slider.forceDown();
			break;

		case SCROLL_INFO_UP:
			tech_scroll_info_up();
			break;

		case SCROLL_INFO_DOWN:
			tech_scroll_info_down();
			break;

		case HELP_BUTTON:
			launch_context_help();
			gamesnd_play_iface(SND_HELP_PRESSED);
			break;

		case OPTIONS_BUTTON:
			gamesnd_play_iface(SND_SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);
			break;

		case EXIT_BUTTON:
			fsspeech_stop();
			gamesnd_play_iface(SND_COMMIT_PRESSED);
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			break;
	}

	return 0;
}

int techroom_load_ani(anim **animpp, char *name)
{
	int load_attempts = 0;
	char anim_filename[64] = "2_";

	// hi-res support
	// (i don't think there are any hi-res anims for these tho)
	if (gr_screen.res == GR_1024) {
		strcat_s(anim_filename, name);
	} else {
		strcpy_s(anim_filename, name);
	}

	while(1) {
		if ( load_attempts++ > 5 ) {
			return 0;
		}

		return 1;
	}

	// bogus
	return 0;
}


void techroom_intel_init()
{
	int rval, temp;
	static int inited = 0;

	if (inited)
		return;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "species.tbl", rval));
		lcl_ext_close();
		return;
	}
	
	read_file_text("species.tbl", CF_TYPE_TABLES);
	reset_parse();

	Intel_info_size = 0;
	while (optional_string("$Entry:")) {
		Assert(Intel_info_size < MAX_INTEL_ENTRIES);
		if (Intel_info_size >= MAX_INTEL_ENTRIES) {
			mprintf(("TECHMENU: Too many intel entries!\n"));
			break;
		}

		Intel_info[Intel_info_size].flags = IIF_DEFAULT_VALUE;

		required_string("$Name:");
		stuff_string(Intel_info[Intel_info_size].name, F_NAME, NAME_LENGTH);

		required_string("$Anim:");
		stuff_string(Intel_info[Intel_info_size].anim_filename, F_NAME, NAME_LENGTH);

		required_string("$AlwaysInTechRoom:");
		stuff_int(&temp);
		if (temp) {
			// set default to align with what we read - Goober5000
			Intel_info[Intel_info_size].flags |= IIF_IN_TECH_DATABASE;
			Intel_info[Intel_info_size].flags |= IIF_DEFAULT_IN_TECH_DATABASE;
		}

		required_string("$Description:");
		stuff_string(Intel_info[Intel_info_size].desc, F_MULTITEXT, TECH_INTEL_DESC_LEN);

		Intel_info_size++;
	}

	inited = 1;

	// close localization
	lcl_ext_close();
}

void techroom_init()
{
	int i, idx;
	techroom_buttons *b;

	Ships_loaded = 0;
	Weapons_loaded = 0;
	Intel_loaded = 0;

	Techroom_show_all = 0;

	// set up UI stuff
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Tech_mask_filename[gr_screen.res]);

	Tech_background_bitmap = bm_load(Tech_background_filename[gr_screen.res]);
	if (Tech_background_bitmap < 0) {
		// failed to load bitmap, not a good thing
		Error(LOCATION,"Couldn't load techroom background bitmap");
	}

	for (i=0; i<NUM_BUTTONS; i++) {
		b = &Buttons[gr_screen.res][i];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, b->flags & REPEAT, 1);
		// set up callback for when a mouse first goes over a button
		if (b->filename) {
			b->button.set_bmaps(b->filename);
			b->button.set_highlight_action(common_play_highlight_sound);
		} else {
			b->button.hide();
		}

		b->button.link_hotspot(b->hotspot);
	}

	// common tab button text
	Ui_window.add_XSTR("Technical Database", 1055, Buttons[gr_screen.res][TECH_DATABASE_TAB].xt,  Buttons[gr_screen.res][TECH_DATABASE_TAB].yt, &Buttons[gr_screen.res][TECH_DATABASE_TAB].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Mission Simulator", 1056, Buttons[gr_screen.res][SIMULATOR_TAB].xt,  Buttons[gr_screen.res][SIMULATOR_TAB].yt, &Buttons[gr_screen.res][SIMULATOR_TAB].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Cutscenes", 1057, Buttons[gr_screen.res][CUTSCENES_TAB].xt,  Buttons[gr_screen.res][CUTSCENES_TAB].yt, &Buttons[gr_screen.res][CUTSCENES_TAB].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Credits", 1058, Buttons[gr_screen.res][CREDITS_TAB].xt,  Buttons[gr_screen.res][CREDITS_TAB].yt, &Buttons[gr_screen.res][CREDITS_TAB].button, UI_XSTR_COLOR_GREEN);

	// common ship/weapon/intel text
	Ui_window.add_XSTR("Ships", 293, Buttons[gr_screen.res][SHIPS_DATA_TAB].xt,  Buttons[gr_screen.res][SHIPS_DATA_TAB].yt, &Buttons[gr_screen.res][SHIPS_DATA_TAB].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Weapons", 1553, Buttons[gr_screen.res][WEAPONS_DATA_TAB].xt,  Buttons[gr_screen.res][WEAPONS_DATA_TAB].yt, &Buttons[gr_screen.res][WEAPONS_DATA_TAB].button, UI_XSTR_COLOR_GREEN);
	Ui_window.add_XSTR("Intelligence", 1066, Buttons[gr_screen.res][INTEL_DATA_TAB].xt,  Buttons[gr_screen.res][INTEL_DATA_TAB].yt, &Buttons[gr_screen.res][INTEL_DATA_TAB].button, UI_XSTR_COLOR_GREEN);

	// common help/options/commit text
	Ui_window.add_XSTR("Exit", 1418, Buttons[gr_screen.res][EXIT_BUTTON].xt,  Buttons[gr_screen.res][EXIT_BUTTON].yt, &Buttons[gr_screen.res][EXIT_BUTTON].button, UI_XSTR_COLOR_PINK);		

	if (Player->flags & PLAYER_FLAGS_IS_MULTI) {
		Buttons[gr_screen.res][SIMULATOR_TAB].button.disable();
		Buttons[gr_screen.res][CUTSCENES_TAB].button.disable();
	}

	// set some hotkeys
	Buttons[gr_screen.res][PREV_ENTRY_BUTTON].button.set_hotkey(KEY_LEFT);
	Buttons[gr_screen.res][NEXT_ENTRY_BUTTON].button.set_hotkey(KEY_RIGHT);
	Buttons[gr_screen.res][SCROLL_INFO_UP].button.set_hotkey(KEY_UP);
	Buttons[gr_screen.res][SCROLL_INFO_DOWN].button.set_hotkey(KEY_DOWN);


	for (i=0; i<LIST_BUTTONS_MAX; i++) {
		List_buttons[i].create(&Ui_window, "", 0, 0, 60, 30, 0, 1);
		List_buttons[i].hide();
		List_buttons[i].disable();
	}

	View_window.create(&Ui_window, "", Tech_ship_display_coords[gr_screen.res][SHIP_X_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_Y_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_W_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_H_COORD], 1, 1);
	View_window.hide();

	Buttons[gr_screen.res][HELP_BUTTON].button.set_hotkey(KEY_F1);
	Buttons[gr_screen.res][EXIT_BUTTON].button.set_hotkey(KEY_CTRLED | KEY_ENTER);
	Buttons[gr_screen.res][SCROLL_LIST_UP].button.set_hotkey(KEY_PAGEUP);
	Buttons[gr_screen.res][SCROLL_LIST_DOWN].button.set_hotkey(KEY_PAGEDOWN);

	// init help overlay states
	help_overlay_set_state(TECH_ROOM_OVERLAY, 0);

	// setup slider
	Tech_slider.create(&Ui_window, Tech_slider_coords[gr_screen.res][SHIP_X_COORD], Tech_slider_coords[gr_screen.res][SHIP_Y_COORD], Tech_slider_coords[gr_screen.res][SHIP_W_COORD], Tech_slider_coords[gr_screen.res][SHIP_H_COORD], Num_ship_classes, Tech_slider_filename[gr_screen.res], &tech_scroll_list_up, &tech_scroll_list_down, &tech_ship_scroll_capture);

	// zero intel anim/bitmap stuff
	for(idx=0; idx<MAX_INTEL_ENTRIES; idx++){
		Intel_list[idx].animation.num_frames = 0;
		Intel_list[idx].bitmap = -1;
	}

	mprintf(("Techroom successfully initialized, now changing tab...\n"));
	techroom_change_tab(Tab);
}

void techroom_lists_reset()
{
	int i;

	//unload the current animation, we load another one for the new current entry
	if(Tab != SHIPS_DATA_TAB)
		techroom_unload_animation();

	Current_list = NULL;
	Current_list_size = 0;

	model_free_all();

	if (Ship_list != NULL) {
		delete[] Ship_list;
		Ship_list = NULL;
	}

	Ship_list_size = 0;
	Ships_loaded = 0;

	if (Weapon_list != NULL) {
		for (i = 0; i < Weapon_list_size; i++) {
			if (Weapon_list[i].animation.num_frames != 0) {
				generic_anim_unload(&Weapon_list[i].animation);
			}

			if (Weapon_list[i].bitmap >= 0) {
				bm_release(Weapon_list[i].bitmap);
				Weapon_list[i].bitmap = -1;
			}
		}

		delete[] Weapon_list;
		Weapon_list = NULL;
	}

	Weapon_list_size = 0;
	Weapons_loaded = 0;

	for (i = 0; i < Intel_list_size; i++) {
		if (Intel_list[i].animation.num_frames != 0) {
			generic_anim_unload(&Intel_list[i].animation);
		}

		if (Intel_list[i].bitmap >= 0) {
			bm_release(Intel_list[i].bitmap);
			Intel_list[i].bitmap = -1;
		}
	}

	Intel_list_size = 0;
	Intel_loaded = 0;
}

void techroom_close()
{
	fsspeech_stop();

	techroom_lists_reset();

	Techroom_show_all = 0;

	if (Tech_background_bitmap) {
		bm_release(Tech_background_bitmap);
	}

	Ui_window.destroy();
	common_free_interface_palette();		// restore game palette
	if (Palette_bmp){
		bm_release(Palette_bmp);
	}
}

void techroom_do_frame(float frametime)
{
	
	int i, k;	

	// turn off controls when overlay is on
	if ( help_overlay_active(TECH_ROOM_OVERLAY) ) {
		Buttons[gr_screen.res][HELP_BUTTON].button.reset_status();
		Ui_window.set_ignore_gadgets(1);
	}

	// turn off controls in trackball mode
	if (Trackball_active) {
		Ui_window.set_ignore_gadgets(1);
	} else {
		Ui_window.set_ignore_gadgets(0);
	}

	k = Ui_window.process() & ~KEY_DEBUGGED;

	if ( (k > 0) || B1_JUST_RELEASED ) {
		if ( help_overlay_active(TECH_ROOM_OVERLAY) ) {
			help_overlay_set_state(TECH_ROOM_OVERLAY, 0);
			Ui_window.set_ignore_gadgets(0);
			k = 0;
		}
	}

	if ( !help_overlay_active(TECH_ROOM_OVERLAY) ) {
		Ui_window.set_ignore_gadgets(0);
	}

	switch (k) {
		case KEY_SHIFTED | KEY_TAB:  // activate previous tab
			i = Tab - 1;
			if (i < 0) {
				i = NUM_TABS - 1;
			}

			techroom_change_tab(i);
			break;

		case KEY_TAB:  // activate next tab
			i = Tab + 1;
			if (i >= NUM_TABS) {
				i = 0;
			}

			techroom_change_tab(i);
			break;

		case KEY_CTRLED | KEY_DOWN:
			if ( !(Player->flags & PLAYER_FLAGS_IS_MULTI) ) {
				techroom_button_pressed(SIMULATOR_TAB);
				break;
			}
			// fall through

		case KEY_CTRLED | KEY_UP:
			techroom_button_pressed(CREDITS_TAB);
			break;

		case KEY_CTRLED | KEY_ENTER:
		case KEY_ESC:
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			break;

		case KEY_CTRLED | KEY_SHIFTED | KEY_S:
			Techroom_show_all = 1;
			techroom_lists_reset();
			techroom_change_tab(Tab);
			break;

	}	

	// check ship model window for activity
	if (View_window.pressed()) {
		Trackball_active = 1;
		Trackball_mode = 1;
	}
	if (B1_RELEASED) {
		Trackball_active = 0;
	}

	// check all da buttons
	for (i=0; i<NUM_BUTTONS; i++) {
		if (Buttons[gr_screen.res][i].button.pressed()) {
			if (techroom_button_pressed(i)) {
				return;
			}
		}
	}

	// check for mouseovers/clicks on the selection list
	Select_tease_line = -1;
	for (i=0; i<LIST_BUTTONS_MAX; i++) {
		if (List_buttons[i].is_mouse_on()) {
			Select_tease_line = i + List_offset;
		}
	
		if (List_buttons[i].pressed()) {
			Cur_entry = i + List_offset;
			gamesnd_play_iface(SND_USER_SELECT);
			techroom_select_new_entry();
		}
	}

	// clear & draw bg bitmap
	GR_MAYBE_CLEAR_RES(Tech_background_bitmap);
	if (Tech_background_bitmap >= 0) {
		gr_set_bitmap(Tech_background_bitmap);
		gr_bitmap(0, 0);
	}

	// render
	switch (Tab) {
		case SHIPS_DATA_TAB:
			techroom_ships_render(frametime);
			break;

		case WEAPONS_DATA_TAB:
		case INTEL_DATA_TAB:
			techroom_anim_render(frametime);
			break;
	}

	Ui_window.draw();

	for (i=TECH_DATABASE_TAB; i<=CREDITS_TAB; i++) {
		if (Buttons[gr_screen.res][i].button.button_down()) {
			break;
		}
	}
	if (i > CREDITS_TAB) {
		Buttons[gr_screen.res][TECH_DATABASE_TAB].button.draw_forced(2);
	}

	for (i=0; i<NUM_TABS; i++){
		if (Buttons[gr_screen.res][i].button.button_down()){
			break;
		}
	}
	if (i == NUM_TABS){
		Buttons[gr_screen.res][Tab].button.draw_forced(2);
	}

	// blit help overlay if active
	help_overlay_maybe_blit(TECH_ROOM_OVERLAY);

	gr_flip();
}

int intel_info_lookup(char *name)
{
	int	i;

	// bogus
	if (!name)
		return -1;

	for (i=0; i<Intel_info_size; i++)
		if (!stricmp(name, Intel_info[i].name))
			return i;

	return -1;
}

// Goober5000
void tech_reset_to_default()
{
	int i;

	// ships
	for (i=0; i<Num_ship_classes; i++)
	{
		if (Ship_info[i].flags2 & SIF2_DEFAULT_IN_TECH_DATABASE)
			Ship_info[i].flags |= SIF_IN_TECH_DATABASE;
		else
			Ship_info[i].flags &= ~SIF_IN_TECH_DATABASE;

		if (Ship_info[i].flags2 & SIF2_DEFAULT_IN_TECH_DATABASE_M)
			Ship_info[i].flags |= SIF_IN_TECH_DATABASE_M;
		else
			Ship_info[i].flags &= ~SIF_IN_TECH_DATABASE_M;
	}

	// weapons
	for (i=0; i<Num_weapon_types; i++)
	{
		if (Weapon_info[i].wi_flags2 & WIF2_DEFAULT_IN_TECH_DATABASE)
			Weapon_info[i].wi_flags |= WIF_IN_TECH_DATABASE;
		else
			Weapon_info[i].wi_flags &= ~WIF_IN_TECH_DATABASE;
	}

	// intelligence
	for (i=0; i<Intel_info_size; i++)
	{
		if (Intel_info[i].flags & IIF_DEFAULT_IN_TECH_DATABASE)
			Intel_info[i].flags |= IIF_IN_TECH_DATABASE;
		else
			Intel_info[i].flags &= ~IIF_IN_TECH_DATABASE;
	}
}
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "cmdline/cmdline.h"
#include "gamehelp/contexthelp.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "graphics/font.h"
#include "graphics/shadows.h"
#include "graphics/matrix.h"
#include "io/key.h"
#include "io/mouse.h"
#include "lighting/lighting.h"
#include "localization/localize.h"
#include "menuui/techmenu.h"
#include "missionui/missionscreencommon.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "sound/fsspeech.h"
#include "ui/ui.h"
#include "ui/uidefs.h"
#include "weapon/weapon.h"



#define REVOLUTION_RATE	5.2f

#define NUM_BUTTONS	16
#define NUM_TABS		3
#define LIST_BUTTONS_MAX	50

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
const char *Tech_background_filename[GR_NUM_RESOLUTIONS] = {
	"TechShipData",
	"2_TechShipData"
};
const char *Tech_mask_filename[GR_NUM_RESOLUTIONS] = {
	"TechShipData-M",
	"2_TechShipData-M"
};
const char *Tech_slider_filename[GR_NUM_RESOLUTIONS] = {
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

#define MAX_TEXT_LINES		150
#define MAX_TEXT_LINE_LEN	256

struct techroom_buttons {
	const char *filename;
	int x, y, xt, yt;
	int hotspot;
	int tab;
	int flags;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	techroom_buttons(const char *name, int x1, int y1, int xt1, int yt1, int h, int t, int f = 0) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), tab(t), flags(f) {}
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
static int Tech_background_bitmap_mask;
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
static const char *Text_lines[MAX_TEXT_LINES];

static int Cur_entry = -1;				// this is the current entry selected, using entry indexing
static int Cur_entry_index = -1;		// this is the current entry selected, using master list indexing
static int Techroom_ship_modelnum = -1;
static int Techroom_ship_model_instance = -1;
static float Techroom_ship_rot;
static UI_BUTTON List_buttons[LIST_BUTTONS_MAX];  // buttons for each line of text in list

static bool Ships_loaded = false;
static bool Weapons_loaded = false;
static bool Intel_loaded = false;

int Techroom_overlay_id;

// out entry data struct & vars
typedef struct {
	int	index;		// index into the master table that its in (ie Ship_info[])
	const char* name;			// ptr to name string
	const char* desc;			// ptr to description string
	char tech_anim_filename[MAX_FILENAME_LEN];	//duh
	generic_anim animation;	// animation info
	int	bitmap;		// bitmap handle
	int	has_anim;	// flag to indicate the presence of an animation for this item
	int model_num;	// model reference handle
	int textures_loaded;	// if the model has textures loaded for it or not (hacky mem management)
} tech_list_entry;

static SCP_vector<tech_list_entry> Ship_list;
static SCP_vector<tech_list_entry> Weapon_list;
static SCP_vector<tech_list_entry> Intel_list;
static SCP_vector<tech_list_entry>* Current_list = &Ship_list;	// A pointer to the current display list

// slider stuff
static UI_SLIDER2 Tech_slider;

// Intelligence master data structs (these get inited @ game startup from species.tbl)
SCP_vector<intel_data> Intel_info;
bool Intel_inited = false;

// some prototypes to make you happy
int techroom_load_ani(anim **animpp, char *name);
void tech_common_render();
void tech_scroll_list_up();
void tech_scroll_list_down();


////////////////////////////////////////////////////
// like, functions and stuff

void techroom_init_desc(const char *src, int w)
{
	Text_size = Text_offset = 0;
	if (!src) {
		return;
	}

	Text_size = split_str(src, w, Text_line_size, Text_lines, MAX_TEXT_LINES, MAX_TEXT_LINE_LEN);
	Assert(Text_size >= 0 && Text_size < MAX_TEXT_LINES);
}

void techroom_unload_animation()
{
	//clear everything, just in case, it will get loaded when needed later
	for (auto& list_entry : Weapon_list) {
		if (list_entry.animation.type != BM_TYPE_NONE && list_entry.has_anim != 0) {
			generic_anim_unload(&list_entry.animation);
		}

		if (list_entry.bitmap >= 0) {
			bm_release(list_entry.bitmap);
			list_entry.bitmap = -1;
		}
	}

	for (auto & intel_entry : Intel_list) {
		if (intel_entry.animation.type != BM_TYPE_NONE && intel_entry.has_anim != 0) {
			generic_anim_unload(&intel_entry.animation);
		}

		if (intel_entry.bitmap >= 0) {
			bm_release(intel_entry.bitmap);
			intel_entry.bitmap = -1;
		}
	}
}

void techroom_select_new_entry()
{
	if (Current_list->empty()) {
		Cur_entry_index = Cur_entry = -1;
		techroom_init_desc(nullptr,0);
		return;
	}

	Assert(Cur_entry < static_cast<int>(Current_list->size()));

	Cur_entry_index = Current_list->at(Cur_entry).index;
	Assert( Cur_entry_index >= 0 );

	// if we are in the ships tab, load the ship model
	if (Tab == SHIPS_DATA_TAB) {
		ship_info *sip = &Ship_info[Cur_entry_index];

		int i = 0;
		// little memory management, kinda hacky but it should keep the techroom at around
		// 100meg rather than the 700+ it can get to with all ships loaded - taylor
		for (auto & list_entry : *Current_list) {
			if ((list_entry.model_num > -1) && (list_entry.textures_loaded)) {
				// don't unload any spot within 5 of current
				if ((i < Cur_entry + 5) && (i > Cur_entry - 5) )
					continue;

				mprintf(("TECH ROOM: Dumping excess ship textures...\n"));

				model_page_out_textures(list_entry.model_num);

				list_entry.textures_loaded = 0;
			}
			i++;
		}

		Techroom_ship_modelnum = model_load(sip, true);

		if (Techroom_ship_model_instance >= 0) {
			model_delete_instance(Techroom_ship_model_instance);
		}
		Techroom_ship_model_instance = model_create_instance(-1, Techroom_ship_modelnum);

		model_set_up_techroom_instance(sip, Techroom_ship_model_instance);

		Current_list->at(Cur_entry).model_num = Techroom_ship_modelnum;

		// page in ship textures properly (takes care of nondimming pixels)
		model_page_in_textures(Techroom_ship_modelnum, Cur_entry_index);

		Current_list->at(Cur_entry).textures_loaded = 1;
	} else {
		Techroom_ship_modelnum = -1;

		if (Techroom_ship_model_instance >= 0) {
			model_delete_instance(Techroom_ship_model_instance);
			Techroom_ship_model_instance = -1;
		}

		Trackball_mode = 0;

		// load animation here, we now only have one loaded
		int stream_result = generic_anim_init_and_stream(&Current_list->at(Cur_entry).animation, Current_list->at(Cur_entry).tech_anim_filename, bm_get_type(Tech_background_bitmap), true);

		if (stream_result >= 0) {
			Current_list->at(Cur_entry).has_anim = 1;
		} else {
			// we've failed to load any animation
			// load an image and treat it like a 1 frame animation
			Current_list->at(Cur_entry).bitmap = bm_load(Current_list->at(Cur_entry).tech_anim_filename);
		}
	}

	techroom_init_desc(Current_list->at(Cur_entry).desc, Tech_desc_coords[gr_screen.res][SHIP_W_COORD]);
	fsspeech_play(FSSPEECH_FROM_TECHROOM, Current_list->at(Cur_entry).desc);
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
		gr_string(xo, yo + y, line, GR_RESIZE_MENU);

		y += font_height;
		z++;
	}

	// maybe output 'more' indicator
	if ( z < Text_size ) {
		// can be scrolled down
		int more_txt_x = Tech_desc_coords[gr_screen.res][0] + (Tech_desc_coords[gr_screen.res][2]/2) - 10;	// FIXME should move these to constants since they don't move
		int more_txt_y = Tech_desc_coords[gr_screen.res][1] + Tech_desc_coords[gr_screen.res][3];				// located below brief text, centered
		int w, h;
		gr_get_string_size(&w, &h, XSTR("more", 1469), static_cast<int>(strlen(XSTR("more", 1469))));
		gr_set_color_fast(&Color_black);
		gr_rect(more_txt_x-2, more_txt_y, w+3, h, GR_RESIZE_MENU);
		gr_set_color_fast(&Color_more_indicator);
		gr_string(more_txt_x, more_txt_y, XSTR("more", 1469), GR_RESIZE_MENU);  // base location on the input x and y?
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
		if ((z - List_offset) >= LIST_BUTTONS_MAX || z >= static_cast<int>(Current_list->size())) {
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
		strncpy(buf, Current_list->at(z).name, sizeof(buf) - 1);

		if (Lcl_gr && !Disable_built_in_translations)
			lcl_translate_ship_name_gr(buf);

		font::force_fit_string(buf, 255, Tech_list_coords[gr_screen.res][SHIP_W_COORD]);
		gr_string(Tech_list_coords[gr_screen.res][SHIP_X_COORD], Tech_list_coords[gr_screen.res][SHIP_Y_COORD] + y, buf, GR_RESIZE_MENU);

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
	ship_info *sip = &Ship_info[Cur_entry_index];
	model_render_params render_info;

	if (sip->uses_team_colors) {
		render_info.set_team_color(sip->default_team_name, "none", 0, 0);
	}

	// get correct revolution rate
	if (sip->is_big_ship()) {
		rev_rate *= 1.7f;
	}
	if (sip->is_huge_ship()) {
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

	gr_set_clip(Tech_ship_display_coords[gr_screen.res][SHIP_X_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_Y_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_W_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_H_COORD], GR_RESIZE_MENU);	

	// render the ship
	g3_start_frame(1);
	g3_set_view_matrix(&sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * 1.3f);

	//setup lights
	common_setup_room_lights();

	Glowpoint_use_depth_buffer = false;

	model_clear_instance(Techroom_ship_modelnum);
	render_info.set_detail_level_lock(0);

	if (sip->replacement_textures.size() > 0)
	{
		render_info.set_replacement_textures(Techroom_ship_modelnum, sip->replacement_textures);
	}

    if(shadow_maybe_start_frame(Shadow_disable_overrides.disable_techroom))
    {
        gr_reset_clip();

		auto pm = model_get(Techroom_ship_modelnum);

		shadows_start_render(&Eye_matrix, &Eye_position, Proj_fov, gr_screen.clip_aspect, -sip->closeup_pos.xyz.z + pm->rad, -sip->closeup_pos.xyz.z + pm->rad + 200.0f, -sip->closeup_pos.xyz.z + pm->rad + 2000.0f, -sip->closeup_pos.xyz.z + pm->rad + 10000.0f);
        render_info.set_flags(MR_NO_TEXTURING | MR_NO_LIGHTING | MR_AUTOCENTER);
		
		model_render_immediate(&render_info, Techroom_ship_modelnum, Techroom_ship_model_instance, &Techroom_ship_orient, &vmd_zero_vector);
        shadows_end_render();

		gr_set_clip(Tech_ship_display_coords[gr_screen.res][SHIP_X_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_Y_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_W_COORD], Tech_ship_display_coords[gr_screen.res][SHIP_H_COORD], GR_RESIZE_MENU);
    }
	
	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	uint render_flags = MR_AUTOCENTER;

	if(sip->flags[Ship::Info_Flags::No_lighting])
		render_flags |= MR_NO_LIGHTING;

	render_info.set_flags(render_flags);

	model_render_immediate(&render_info, Techroom_ship_modelnum, Techroom_ship_model_instance, &Techroom_ship_orient, &vmd_zero_vector);

	Glowpoint_use_depth_buffer = true;

	batching_render_all();

	shadow_end_frame();

	gr_end_view_matrix();
	gr_end_proj_matrix();

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
		Cur_entry = static_cast<int>(Current_list->size() - 1);

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
	gamesnd_play_iface(InterfaceSounds::SCROLL);
}

// select next entry in current list
void tech_next_entry()
{
	//unload the current animation, we load another one for the new current entry
	techroom_unload_animation();

	Cur_entry++;
	if (Cur_entry >= static_cast<int>(Current_list->size())) {
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
	gamesnd_play_iface(InterfaceSounds::SCROLL);
}

void tech_scroll_info_up()
{
	if (Text_offset) {
		Text_offset--;
		gamesnd_play_iface(InterfaceSounds::SCROLL);
	} else {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void tech_scroll_info_down()
{
	int h;

	h = Tech_desc_coords[gr_screen.res][SHIP_H_COORD];

	if (Text_offset + h / gr_get_font_height() < Text_size) {
		Text_offset++;
		gamesnd_play_iface(InterfaceSounds::SCROLL);
	} else { //-V523
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void tech_scroll_list_up()
{
	//int last;

	if (List_offset > 0) {
		List_offset--;
		gamesnd_play_iface(InterfaceSounds::SCROLL);
	} else {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
	}
}

void tech_scroll_list_down()
{
	if (List_offset + Tech_list_coords[gr_screen.res][SHIP_H_COORD] / gr_get_font_height() < static_cast<int>(Current_list->size())) {
		List_offset++;
		gamesnd_play_iface(InterfaceSounds::SCROLL);
	} else {
		gamesnd_play_iface(InterfaceSounds::GENERAL_FAIL);
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

	// exit now if there are no entries to show
	if (Current_list->empty() || Cur_entry < 0 || Cur_entry >= static_cast<int>(Current_list->size()))
		return;

	// render the animation
	if(Current_list->at(Cur_entry).animation.num_frames > 0)
	{
		//grab dimensions
		bm_get_info((Current_list->at(Cur_entry).animation.streaming) ? Current_list->at(Cur_entry).animation.bitmap_id : Current_list->at(Cur_entry).animation.first_frame, &x, &y, nullptr, nullptr, nullptr);
		//get the centre point - adjust
		x = Tech_ani_centre_coords[gr_screen.res][0] - x / 2;
		y = Tech_ani_centre_coords[gr_screen.res][1] - y / 2;
		generic_anim_render(&Current_list->at(Cur_entry).animation, frametime, x, y, true);
	}
	// if our active item has a bitmap instead of an animation, draw it
	else if((Cur_entry >= 0) && (Current_list->at(Cur_entry).bitmap >= 0)){
		//grab dimensions
		bm_get_info(Current_list->at(Cur_entry).bitmap, &x, &y, nullptr, nullptr, nullptr);
		//get the centre point - adjust
		x = Tech_ani_centre_coords[gr_screen.res][0] - x / 2;
		y = Tech_ani_centre_coords[gr_screen.res][1] - y / 2;
		gr_set_bitmap(Current_list->at(Cur_entry).bitmap);
		gr_bitmap(x, y, GR_RESIZE_MENU);
	}
}

void techroom_change_tab(int num)
{
	int multi = 0, font_height, max_num_entries_viewable;
    flagset<Weapon::Info_Flags> wi_mask;
    flagset<Ship::Info_Flags> si_mask;

	//unload the current animation, we load another one for the new current entry
	if(Tab != SHIPS_DATA_TAB)
		techroom_unload_animation();

	Tab = num;
	List_offset = 0;
	Cur_entry = 0;
	multi = Player->flags & PLAYER_FLAGS_IS_MULTI;

	for (int i=0; i<LIST_BUTTONS_MAX; i++){
		List_buttons[i].disable();
	}

	// disable some stuff in multiplayer
	if (Player->flags & PLAYER_FLAGS_IS_MULTI) {
		Buttons[gr_screen.res][SIMULATOR_TAB].button.disable();
		Buttons[gr_screen.res][CUTSCENES_TAB].button.disable();
	}

	switch (Tab) {
		case SHIPS_DATA_TAB:
            si_mask.set(multi ? Ship::Info_Flags::In_tech_database_m : Ship::Info_Flags::In_tech_database);
			
			// load ship info if necessary
			if ( !Ships_loaded ) {
				if (Ship_list.empty()) {
					Ship_list.reserve(Ship_info.size());
				}

				tech_list_entry temp_entry;

				// we always initially set these values, so keep them outside the loop.
				temp_entry.bitmap = -1;
				temp_entry.animation.num_frames = 0;			// no anim for ships
				temp_entry.has_anim = 0;				// no anim for ships
				temp_entry.model_num = -1;
				temp_entry.textures_loaded = 0;

				for (auto it = Ship_info.begin(); it != Ship_info.end(); ++it)
				{
                    if (Techroom_show_all || (it->flags & si_mask).any_set())
					{
						// this ship should be displayed, fill out the entry struct
						temp_entry.index = (int)std::distance(Ship_info.begin(), it);
						temp_entry.name = *it->tech_title ? it->tech_title : it->get_display_name();
						temp_entry.desc = it->tech_desc;

                        Ship_list.push_back(temp_entry);
                    }
                }

				Ships_loaded = true;
			}

			Current_list = &Ship_list;

			font_height = gr_get_font_height();
			max_num_entries_viewable = Tech_list_coords[gr_screen.res][SHIP_H_COORD] / font_height;
			Tech_slider.set_numberItems((int)Current_list->size() > max_num_entries_viewable ? (int)Current_list->size()-max_num_entries_viewable : 0);

			// no anim to start here
			break;

		case WEAPONS_DATA_TAB:
				
			// load weapon info & anims if necessary
			if ( !Weapons_loaded ) {
				Weapon_list.reserve(Weapon_info.size());

				wi_mask.set(multi ? Weapon::Info_Flags::Player_allowed : Weapon::Info_Flags::In_tech_database);

				int i = 0;
				tech_list_entry temp_entry;

				// we always initially set these values, so keep them outside the loop.
				temp_entry.has_anim = 1;
				temp_entry.bitmap = -1;
				temp_entry.animation.num_frames = 0;
				temp_entry.model_num = -1;
				temp_entry.textures_loaded = 0;

				for (auto &wi : Weapon_info)
				{
					if (Techroom_show_all || (wi.wi_flags & wi_mask).any_set())
					{ 
						// we have a weapon that should be in the tech db, so fill out specific info
						temp_entry.index = i;
						temp_entry.desc = wi.tech_desc;
						temp_entry.name = wi.tech_title[0] ? wi.tech_title : wi.get_display_name();
						// copy the weapon animation filename
						strncpy(temp_entry.tech_anim_filename, wi.tech_anim_filename, MAX_FILENAME_LEN - 1);
						
						Weapon_list.push_back(temp_entry);
					}
					++i;
				}

				Weapons_loaded = true;
			}

			Current_list = &Weapon_list;

			font_height = gr_get_font_height();
			max_num_entries_viewable = Tech_list_coords[gr_screen.res][SHIP_H_COORD] / font_height;
			Tech_slider.set_numberItems(static_cast<int>(Current_list->size()) > max_num_entries_viewable ? static_cast<int>(Current_list->size())-max_num_entries_viewable : 0);

			break;

		case INTEL_DATA_TAB:

			// load intel if necessary
			if ( !Intel_loaded ) {
				if (Intel_list.empty()) {
					Intel_list.reserve(Intel_info.size());
				}

				int i = 0;
				tech_list_entry temp_entry;

				// we always initially set these values, so keep them outside the loop.
				temp_entry.has_anim = 0;
				temp_entry.model_num = -1;
				temp_entry.textures_loaded = 0;

				for (auto &ii : Intel_info) {
					
					if (Techroom_show_all || (ii.flags & IIF_IN_TECH_DATABASE)) {
						// leave option for no animation if string == "none"
						if (!strcmp(ii.anim_filename, "none")) {
							temp_entry.animation.num_frames = 0;
						} else {
							// try and load as an animation
							temp_entry.bitmap = -1;
							strncpy(temp_entry.tech_anim_filename, ii.anim_filename, NAME_LENGTH - 1);
						}

						temp_entry.desc = ii.desc.c_str();
						temp_entry.index = i;
						temp_entry.name = ii.name;

						Intel_list.push_back(temp_entry);
					}
					++i;
				}
				Intel_loaded = true;
			}

			Current_list = &Intel_list;

			font_height = gr_get_font_height();
			max_num_entries_viewable = Tech_list_coords[gr_screen.res][SHIP_H_COORD] / font_height;
			Tech_slider.set_numberItems(static_cast<int>(Current_list->size()) > max_num_entries_viewable ? static_cast<int>(Current_list->size())-max_num_entries_viewable : 0);

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
			gamesnd_play_iface(InterfaceSounds::SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_SIMULATOR_ROOM);
			return 1;

		case CUTSCENES_TAB:
			fsspeech_stop();
			gamesnd_play_iface(InterfaceSounds::SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN);
			return 1;

		case CREDITS_TAB:
			fsspeech_stop();
			gamesnd_play_iface(InterfaceSounds::SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_CREDITS);
			return 1;

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
			gamesnd_play_iface(InterfaceSounds::HELP_PRESSED);
			break;

		case OPTIONS_BUTTON:
			gamesnd_play_iface(InterfaceSounds::SWITCH_SCREENS);
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);
			break;

		case EXIT_BUTTON:
			fsspeech_stop();
			gamesnd_play_iface(InterfaceSounds::COMMIT_PRESSED);
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			break;
	}

	return 0;
}

int techroom_load_ani(anim ** /*animpp*/, char *name)
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

static intel_data* get_intel_pointer(const char* intel_name)
{
	for (int i = 0; i < (int)Intel_info.size(); i++) {
		if (!stricmp(intel_name, Intel_info[i].name)) {
			return &Intel_info[i];
		}
	}

	// Didn't find anything.
	return nullptr;
}

static void intel_info_init(intel_data* inteli)
{
	inteli->name[0] = '\0';
	inteli->desc = "";
	inteli->anim_filename[0] = '\0';
	inteli->flags = IIF_DEFAULT_VALUE;
}

void parse_intel_table(const char* filename)
{

	try {
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		//retail doesn't have this so it can't be required, but it's here for absent minded modders -Mjn
		optional_string("#Intel");

		while (optional_string("$Entry:")) {
			
			bool create_new_entry = true;
			intel_data intel_t;
			intel_info_init(&intel_t);

			intel_data* intel_p;

			required_string("$Name:");
			stuff_string(intel_t.name, F_NAME, NAME_LENGTH);

			if (optional_string("+nocreate")) {
				if (!Parsing_modular_table) {
					Warning(LOCATION, "+nocreate flag used for intel entry in non-modular table\n");
				} else {
					create_new_entry = false;
				}
			}

			//Check if we're creating a new entry.
			intel_p = get_intel_pointer(intel_t.name);
			if (create_new_entry) {

				// Current behavior is to warn about a duplicate entry, but append it to the list anyway
				// So do that here - Mjn
				if (intel_p != nullptr) {
					error_display(0, "Duplicate entry %s in %s!", intel_t.name, filename);
				}
				Intel_info.push_back(intel_t);
				intel_p = &Intel_info[Intel_info.size() - 1];
			} else {
				if (intel_p == nullptr) {
					mprintf(("Partial entry for [%s] found, but it does not already exist. Skipping!\n", intel_t.name));
					if (!skip_to_start_of_string("$Entry:")) {
						return;
					}
				}
			}

			if (optional_string("$Anim:")) {
				stuff_string(intel_p->anim_filename, F_NAME, NAME_LENGTH);
			}

			if (optional_string("$AlwaysInTechRoom:")) {
				//Change this from stuff_int to stuff_boolean because it can only ever be 1 or 0 here - Mjn
				int temp;
				stuff_boolean(&temp);
				//If we are modifying an existing entry, then reset the flags first
				if (!create_new_entry) {
					intel_p->flags = IIF_DEFAULT_VALUE;
				}
				if (temp) {
					// set default to align with what we read - Goober5000
					intel_p->flags |= IIF_IN_TECH_DATABASE;
					intel_p->flags |= IIF_DEFAULT_IN_TECH_DATABASE;
				}
			}

			if (optional_string("$Description:")) {
				stuff_string(intel_p->desc, F_MULTITEXT);
			}

			//retail table doesn't have #end so we have to check for the start of the next entry
			//or for the end of the file instead. I have also Added #end compatibility here to 
			//bring the table in line with other tables for absent minded modders - Mjn
			if (check_for_string("$Entry:") || check_for_string("#end") || check_for_eof()) {
				continue;
			} else {
				error_display(0, "Missing required token: [$Entry]. Found [%.32s] instead.\n", next_tokens());
				return;
			}

		}

	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void techroom_intel_init()
{
	if (Intel_inited)
		return;

	Intel_info.clear();

	//Allow intel.tbl to be a alias of species.tbl, but only load one or the other.
	//Intel.tbl would be newer so assume intended, but print to the log to be sure - Mjn
	char filename[MAX_FILENAME_LEN] = "species.tbl";
	if (cf_exists_full("intel.tbl", CF_TYPE_TABLES)){
		mprintf(("Intel.tbl was found! Using that instead of Species.tbl...\n"));
		strcpy_s(filename, "intel.tbl");
	}

	// first parse the default table
	parse_intel_table(filename);

	// parse any modular tables
	parse_modular_table("*-intl.tbm", parse_intel_table);

	Intel_inited = true;
}

void techroom_intel_reset()
{
	Intel_info.clear();
	Intel_inited = false;
}

void techroom_init()
{
	int i;
	techroom_buttons *b;

	Ships_loaded = false;
	Weapons_loaded = false;
	Intel_loaded = false;

	Techroom_show_all = 0;

	// set up UI stuff
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);

	Tech_background_bitmap = bm_load(Tech_background_filename[gr_screen.res]);
	if (Tech_background_bitmap < 0) {
		// failed to load bitmap, not a good thing
		Warning(LOCATION,"Error loading techroom background bitmap %s", Tech_background_filename[gr_screen.res]);
	}

	Tech_background_bitmap_mask = bm_load(Tech_mask_filename[gr_screen.res]);
	if (Tech_background_bitmap_mask < 0) {
		Warning(LOCATION, "Error loading techroom background mask %s", Tech_mask_filename[gr_screen.res]);
		return;
	} else {
		Ui_window.set_mask_bmap(Tech_mask_filename[gr_screen.res]);
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
	Techroom_overlay_id = help_overlay_get_index(TECH_ROOM_OVERLAY);
	help_overlay_set_state(Techroom_overlay_id, gr_screen.res, 0);

	// setup slider
	Tech_slider.create(&Ui_window, Tech_slider_coords[gr_screen.res][SHIP_X_COORD], Tech_slider_coords[gr_screen.res][SHIP_Y_COORD], Tech_slider_coords[gr_screen.res][SHIP_W_COORD], Tech_slider_coords[gr_screen.res][SHIP_H_COORD], (int)Ship_info.size(), Tech_slider_filename[gr_screen.res], &tech_scroll_list_up, &tech_scroll_list_down, &tech_ship_scroll_capture);

	// zero intel anim/bitmap stuff
	for(auto & intel_item : Intel_list){
		intel_item.animation.num_frames = 0;
		intel_item.bitmap = -1;
	}

	mprintf(("Techroom successfully initialized, now changing tab...\n"));
	techroom_change_tab(Tab);
}

void techroom_lists_reset()
{
	//unload the current animation, we load another one for the new current entry
	if (Tab != SHIPS_DATA_TAB)
		techroom_unload_animation();

	model_free_all();
	Techroom_ship_modelnum = -1;
	Techroom_ship_model_instance = -1;

	// This can be cleared immediately because there are no anims or bitmaps associated.
	Ship_list.clear();
	Ships_loaded = false;

	// now that we're sure all the bitmaps are released, clear the vectors.
	Weapon_list.clear();
	Weapons_loaded = false;

	Intel_list.clear();
	Intel_loaded = false;
}

void techroom_close()
{
	fsspeech_stop();

	techroom_lists_reset();

	Techroom_show_all = 0;

	if (Tech_background_bitmap != -1) {
		bm_release(Tech_background_bitmap);
	}

	Ui_window.destroy();

	if (Tech_background_bitmap_mask != -1) {
		bm_release(Tech_background_bitmap_mask);
	}

	common_free_interface_palette();		// restore game palette
}

void techroom_do_frame(float frametime)
{
	
	int i, k;	

	// If we don't have a mask, we don't have enough data to do anything with this screen.
	if (Tech_background_bitmap_mask == -1) {
		popup_game_feature_not_in_demo();
		gameseq_post_event(GS_EVENT_MAIN_MENU);
		return;
	}

	// turn off controls when overlay is on
	if ( help_overlay_active(Techroom_overlay_id) ) {
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
		if ( help_overlay_active(Techroom_overlay_id) ) {
			help_overlay_set_state(Techroom_overlay_id, gr_screen.res, 0);
			Ui_window.set_ignore_gadgets(0);
			k = 0;
		}
	}

	if ( !help_overlay_active(Techroom_overlay_id) ) {
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
			gamesnd_play_iface(InterfaceSounds::USER_SELECT);
			techroom_select_new_entry();
		}
	}

	// clear & draw bg bitmap
	GR_MAYBE_CLEAR_RES(Tech_background_bitmap);
	if (Tech_background_bitmap >= 0) {
		gr_set_bitmap(Tech_background_bitmap);
		gr_bitmap(0, 0, GR_RESIZE_MENU);
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
	help_overlay_maybe_blit(Techroom_overlay_id, gr_screen.res);

	gr_flip();
}

// note: the name has to be pre-translated before being passed into this function
int intel_info_lookup(const char *name)
{
	// bogus
	if (!name)
		return -1;

	for (int i = 0; i < intel_info_size(); i++)
		if (!stricmp(name, Intel_info[i].name))
			return i;

	return -1;
}

// Goober5000
void tech_reset_to_default()
{
	// ships
	for (auto& si : Ship_info)
	{
		si.flags.set(Ship::Info_Flags::In_tech_database, si.flags[Ship::Info_Flags::Default_in_tech_database]);
		si.flags.set(Ship::Info_Flags::In_tech_database_m, si.flags[Ship::Info_Flags::Default_in_tech_database_m]);
	}

	// weapons
	for (auto& wi : Weapon_info)
	{
		wi.wi_flags.set(Weapon::Info_Flags::In_tech_database, wi.wi_flags[Weapon::Info_Flags::Default_in_tech_database]);
	}

	// intelligence
	for (int i = 0; i < intel_info_size(); ++i)
	{
		if (Intel_info[i].flags & IIF_DEFAULT_IN_TECH_DATABASE)
			Intel_info[i].flags |= IIF_IN_TECH_DATABASE;
		else
			Intel_info[i].flags &= ~IIF_IN_TECH_DATABASE;
	}
}

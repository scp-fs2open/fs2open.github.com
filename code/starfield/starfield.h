/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _STARFIELD_H
#define _STARFIELD_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "model/model.h"

#define DEFAULT_NMODEL_FLAGS  (MR_NO_ZBUFFER | MR_NO_CULL | MR_ALL_XPARENT | MR_NO_LIGHTING)

#define MAX_STARFIELD_BITMAP_LISTS	1
#define MAX_ASTEROID_FIELDS			4

// nice low polygon background
#define BACKGROUND_MODEL_FILENAME			"spherec.pof"


// starfield list
typedef struct starfield_list_entry {
	char filename[MAX_FILENAME_LEN];		// bitmap filename
	float scale_x, scale_y;					// x and y scale
	int div_x, div_y;						// # of x and y divisions
	angles ang;								// angles from FRED
} starfield_list_entry;

// backgrounds
typedef struct background_t {
	SCP_vector<starfield_list_entry> bitmaps;
	SCP_vector<starfield_list_entry> suns;
} background_t;

#define MAX_BACKGROUNDS	2

extern int Num_backgrounds;
extern int Cur_background;
extern background_t Backgrounds[MAX_BACKGROUNDS];
extern int Nmodel_flags;

extern bool Dynamic_environment;
extern bool Motion_debris_override;

void stars_swap_backgrounds(int idx1, int idx2);
void stars_pack_backgrounds();
bool stars_background_empty();


// add a new sun or bitmap instance
int stars_add_sun_entry(starfield_list_entry *sun_ptr);
int stars_add_bitmap_entry(starfield_list_entry *bitmap);

// get the number of entries that each vector contains
// "is_a_sun" will get sun instance counts, otherwise it gets normal starfield bitmap instance counts
// "bitmap_count" will get number of starfield_bitmap entries rather than starfield_bitmap_instance entries
int stars_get_num_entries(bool is_a_sun, bool bitmap_count);

// macros to get the number of sun or starfield bitmap *instances* available
#define stars_get_num_bitmaps()	stars_get_num_entries(false, false)
#define stars_get_num_suns()	stars_get_num_entries(true, false)

// make a bitmap or sun instance as unusable (doesn't free anything but does prevent rendering)
void stars_mark_instance_unused(int index, bool is_a_sun);
// macros to easily mark a sun or bitmap as unused
#define stars_mark_sun_unused(x)	stars_mark_instance_unused((x),true)
#define stars_mark_bitmap_unused(x)	stars_mark_instance_unused((x),false)

// get a name from an instance index
const char *stars_get_name_from_instance(int index, bool is_a_sun);
// macros to easily get a sun or a bitmap name
#define stars_get_sun_name(x)		stars_get_name_from_instance((x),true)
#define stars_get_bitmap_name(x)	stars_get_name_from_instance((x),false)

extern const int MAX_STARS;
extern int Num_stars;

// call on game startup
void stars_init();
// call on game shutdown
void stars_close();

// call this before mission parse to reset all data to a sane state
void stars_pre_level_init(bool clear_backgrounds = true);

// call this in game_post_level_init() so we know whether we're running in full nebula mode or not
void stars_post_level_init();

// draw background bitmaps
void stars_draw_background();

// This *must* be called to initialize the lighting.
// You can turn off all the stars and suns and nebulas, though.
void stars_draw(int show_stars, int show_suns, int show_nebulas, int show_subspace, int env);
// void calculate_bitmap_matrix(starfield_bitmaps *bm, vec3d *v);
// void calculate_bitmap_points(starfield_bitmaps *bm, float bank = 0.0f);

// draw the corresponding glow for sun_n
void stars_draw_sun_glow(int sun_n);

// Call when the viewer camera "cuts" so stars and debris
// don't draw incorrect blurs between last frame and this frame.
void stars_camera_cut();

// call this to set a specific model as the background model
void stars_set_background_model(char *model_name, char *texture_name, int flags = DEFAULT_NMODEL_FLAGS);
void stars_set_background_orientation(matrix *orient = NULL);

// lookup a starfield bitmap, return index or -1 on fail
int stars_find_bitmap(char *name);

// lookup a sun by bitmap filename, return index or -1 on fail
int stars_find_sun(char *name);

// get the world coords of the sun pos on the unit sphere.
void stars_get_sun_pos(int sun_n, vec3d *pos);

// for SEXP stuff so that we can mark a bitmap as being used regardless of whether 
// or not there is an instance for it yet
void stars_preload_sun_bitmap(char *fname);
void stars_preload_background_bitmap(char *fname);

void stars_set_nebula(bool activate);

// Starfield functions that should be used only by FRED ...

// get a name based on the index into starfield_bitmap, only FRED should ever need this
const char *stars_get_name_FRED(int index, bool is_a_sun);
// erase an instance, note that this is very slow so it should only be done in FRED
void stars_delete_entry_FRED(int index, bool is_a_sun);
// modify an existing starfield bitmap instance, or add a new one if needed
void stars_modify_entry_FRED(int index, const char *name, starfield_list_entry *sbi_new, bool is_a_sun);


// Goober5000
void stars_load_first_valid_background();
int stars_get_first_valid_background();
void stars_load_background(int background_idx);

#endif

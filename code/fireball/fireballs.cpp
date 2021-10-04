/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "fireball/fireballs.h"
#include "asteroid/asteroid.h"
#include "cmdline/cmdline.h"
#include "gamesnd/gamesnd.h"
#include "graphics/tmapper.h"
#include "localization/localize.h"
#include "model/model.h"
#include "nebula/neb.h"
#include "object/object.h"
#include "options/Option.h"
#include "parse/parselo.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "tracing/Monitor.h"

#include <cstdlib>


int Knossos_warp_ani_used;

#define WARPHOLE_GROW_TIME		(2.35f)	// time for warphole to reach max size (also time to shrink to nothing once it begins to shrink)

#define MAX_WARP_LOD	0

constexpr int INTITIAL_FIREBALL_CONTAINTER_SIZE = 256;

SCP_vector<fireball> Fireballs;
SCP_vector<int> Unused_fireball_indices;

fireball_info Fireball_info[MAX_FIREBALL_TYPES];

int fireball_used[MAX_FIREBALL_TYPES];

int Num_fireball_types = 0;

bool fireballs_inited = false;
bool fireballs_parsed = false;

bool Fireball_use_3d_warp = false;

static auto WarpOption = options::OptionBuilder<bool>("Graphics.3dWarp", "3D Warp", "Use a 3D model for warp effects")
                             .category("Graphics")
                             .default_val(true)
                             .level(options::ExpertLevel::Advanced)
                             .bind_to(&Fireball_use_3d_warp)
                             .importance(65)
                             .finish();

/**
 * Play warp in sound for warp effect
 */
void fireball_play_warphole_open_sound(int ship_class, fireball *fb)
{
	gamesnd_id sound_index;
	float		range_multiplier = 1.0f;
	object	*fireball_objp;	
		
	Assert((fb != NULL) && (fb->objnum >= 0));
	if((fb == NULL) || (fb->objnum < 0)){
		return;
	}
	fireball_objp = &Objects[fb->objnum];

	sound_index = gamesnd_id(GameSounds::WARP_IN);

	if(fb->warp_open_sound_index.isValid()) {
		sound_index = fb->warp_open_sound_index;
	} else if ((ship_class >= 0) && (ship_class < ship_info_size())) {
		if ( Ship_info[ship_class].is_huge_ship() ) {
			sound_index = gamesnd_id(GameSounds::CAPITAL_WARP_IN);
			fb->flags |= FBF_WARP_CAPITAL_SIZE;
		} else if ( Ship_info[ship_class].is_big_ship() ) {
			range_multiplier = 6.0f;
			fb->flags |= FBF_WARP_CRUISER_SIZE;
		}
	}

	snd_play_3d(gamesnd_get_game_sound(sound_index), &fireball_objp->pos, &Eye_position, fireball_objp->radius, NULL, 0, 1.0f, SND_PRIORITY_DOUBLE_INSTANCE, NULL, range_multiplier); // play warp sound effect
}

/**
 * Play warp out sound for warp effect
 */
void fireball_play_warphole_close_sound(fireball *fb)
{
	gamesnd_id sound_index;

	object *fireball_objp;

	fireball_objp = &Objects[fb->objnum];

	sound_index = gamesnd_id(GameSounds::WARP_OUT);

	if ( fb->warp_close_sound_index.isValid() ) {
		sound_index = fb->warp_close_sound_index;
	} else if ( fb->flags & FBF_WARP_CAPITAL_SIZE ) {
		sound_index = gamesnd_id(GameSounds::CAPITAL_WARP_OUT);
	} else {
		return;
	}

	snd_play_3d(gamesnd_get_game_sound(sound_index), &fireball_objp->pos, &Eye_position, fireball_objp->radius); // play warp sound effect
}

static void fireball_generate_unique_id(char *unique_id, int buffer_len, int fireball_index)
{
	Assert((fireball_index >= 0) && (fireball_index < MAX_FIREBALL_TYPES));

	switch (fireball_index)
	{
		// use sensible names for the fireball.tbl default entries
		case FIREBALL_EXPLOSION_MEDIUM:
			strncpy(unique_id, "Medium Explosion", buffer_len);
			break;

		case FIREBALL_WARP:
			strncpy(unique_id, "Warp Effect", buffer_len);
			break;

		case FIREBALL_KNOSSOS:
			strncpy(unique_id, "Knossos Effect", buffer_len);
			break;

		case FIREBALL_ASTEROID:
			strncpy(unique_id, "Asteroid Explosion", buffer_len);
			break;

		case FIREBALL_EXPLOSION_LARGE1:
			strncpy(unique_id, "Large Explosion 1", buffer_len);
			break;

		case FIREBALL_EXPLOSION_LARGE2:
			strncpy(unique_id, "Large Explosion 2", buffer_len);
			break;

		// base the id on the index
		default:
			snprintf(unique_id, buffer_len, "Custom Fireball %d", fireball_index - NUM_DEFAULT_FIREBALLS + 1);
			break;
	}

	// null-terminate
	unique_id[buffer_len - 1] = '\0';
}

/**
 * Set default colors for each explosion type (original values from object.cpp)
 */
static void fireball_set_default_color(int idx)
{
	Assert((idx >= 0) && (idx < MAX_FIREBALL_TYPES));

	switch (idx)
	{
		case FIREBALL_EXPLOSION_LARGE1:
		case FIREBALL_EXPLOSION_LARGE2:
		case FIREBALL_EXPLOSION_MEDIUM:
		case FIREBALL_ASTEROID:
			Fireball_info[idx].exp_color[0] = 1.0f;
			Fireball_info[idx].exp_color[1] = 0.5f;
			Fireball_info[idx].exp_color[2] = 0.125f;
			break;

		case FIREBALL_WARP:
			Fireball_info[idx].exp_color[0] = 0.75f;
			Fireball_info[idx].exp_color[1] = 0.75f;
			Fireball_info[idx].exp_color[2] = 1.0f;
			break;


		case FIREBALL_KNOSSOS:
			Fireball_info[idx].exp_color[0] = 0.75f;
			Fireball_info[idx].exp_color[1] = 1.0f;
			Fireball_info[idx].exp_color[2] = 0.75f;
			break;

		default:
			Fireball_info[idx].exp_color[0] = 1.0f;
			Fireball_info[idx].exp_color[1] = 1.0f;
			Fireball_info[idx].exp_color[2] = 1.0f;
			break;
	}
}

static void fireball_set_default_warp_attributes(int idx)
{
	Assert((idx >= 0) && (idx < MAX_FIREBALL_TYPES));

	switch (idx)
	{
		case FIREBALL_WARP:
		case FIREBALL_KNOSSOS:
			strcpy_s(Fireball_info[idx].warp_glow, "warpglow01");
			strcpy_s(Fireball_info[idx].warp_ball, "warpball01");
			strcpy_s(Fireball_info[idx].warp_model, "warp.pof");
			break;
	}
}

void fireball_info_clear(fireball_info *fb)
{
	Assert(fb != nullptr);
	memset(fb, 0, sizeof(fireball_info));

	for (int i = 0; i < MAX_FIREBALL_LOD; ++i)
		fb->lod[i].bitmap_id = -1;

	fb->warp_glow_bitmap = -1;
	fb->warp_ball_bitmap = -1;
	fb->warp_model_id = -1;
}

int fireball_info_lookup(const char *unique_id)
{
	for (int i = 0; i < Num_fireball_types; ++i)
		if (!stricmp(Fireball_info[i].unique_id, unique_id))
			return i;

	return -1;
}

/**
 * Parse fireball tbl
 */
static void parse_fireball_tbl(const char *table_filename)
{
	try
	{
		read_file_text(table_filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Start");

		while (required_string_one_of(3, "#End", "$Name:", "$Unique ID:"))
		{
			fireball_info *fi;
			int existing_idx = -1;
			char unique_id[NAME_LENGTH];
			char fireball_filename[MAX_FILENAME_LEN];

			// unique ID, because indexes are unpredictable
			memset(unique_id, 0, NAME_LENGTH);
			if (optional_string("$Unique ID:"))
				stuff_string(unique_id, F_NAME, NAME_LENGTH);

			// base filename
			required_string("$Name:");
			stuff_string(fireball_filename, F_NAME, MAX_FILENAME_LEN);

			// find out if we are overriding a previous entry;
			// per precedent, these strings should only be in TBMs
			// UNLIKE precedent, we can now add fireballs in modular tables, not just replace them
			if (Parsing_modular_table)
			{
				if (optional_string("+Explosion_Medium"))
					existing_idx = FIREBALL_EXPLOSION_MEDIUM;
				else if (optional_string("+Warp_Effect"))
					existing_idx = FIREBALL_WARP;
				else if (optional_string("+Knossos_Effect"))
					existing_idx = FIREBALL_KNOSSOS;
				else if (optional_string("+Asteroid"))
					existing_idx = FIREBALL_ASTEROID;
				else if (optional_string("+Explosion_Large1"))
					existing_idx = FIREBALL_EXPLOSION_LARGE1;
				else if (optional_string("+Explosion_Large2"))
					existing_idx = FIREBALL_EXPLOSION_LARGE2;
				else if (optional_string("+Custom_Fireball"))
					stuff_int(&existing_idx);

				// we can ALSO override a previous entry by specifying a previously used unique ID
				// either way will work, but if both are specified, unique ID takes precedence
				if (strlen(unique_id) > 0)
				{
					int temp_idx = fireball_info_lookup(unique_id);
					if (temp_idx >= 0)
						existing_idx = temp_idx;
				}
			}

			bool first_time;
			// now select our entry accordingly...
			// are we using a previous entry?
			if (existing_idx >= 0)
			{
				fi = &Fireball_info[existing_idx];
				first_time = false;
			}
			// we are creating a new entry, so set some defaults
			else
			{
				// make sure we don't exceed the max
				if (Num_fireball_types >= MAX_FIREBALL_TYPES)
				{
					error_display(0, "Too many fireball entries!  Max is %d", MAX_FIREBALL_TYPES);
					return;
				}

				fi = &Fireball_info[Num_fireball_types];
				fireball_info_clear(fi);

				// If the table didn't specify a unique ID, generate one.  This will be assigned a few lines later.
				if (strlen(unique_id) == 0)
					fireball_generate_unique_id(unique_id, NAME_LENGTH, Num_fireball_types);

				// Set remaining fireball defaults
				fireball_set_default_color(Num_fireball_types);
				fireball_set_default_warp_attributes(Num_fireball_types);

				Num_fireball_types++;
				first_time = true;
			}

			// copy over what we already parsed
			if (strlen(unique_id) > 0)
				strcpy_s(fi->unique_id, unique_id);
			strcpy_s(fi->lod[0].filename, fireball_filename);

			
			// Do we have a LOD num?
			if (optional_string("$LOD:"))
			{
				stuff_int(&fi->lod_count);

				if (fi->lod_count > MAX_FIREBALL_LOD)
					fi->lod_count = MAX_FIREBALL_LOD;
			} else if (first_time) {
				//assume a LOD of at least 1
				fi->lod_count = 1;
			}

			// check for particular lighting color
			if (optional_string("$Light color:"))
			{
				int r, g, b;

				stuff_int(&r);
				stuff_int(&g);
				stuff_int(&b);

				CLAMP(r, 0, 255);
				CLAMP(g, 0, 255);
				CLAMP(b, 0, 255);

				fi->exp_color[0] = (r / 255.0f);
				fi->exp_color[1] = (g / 255.0f);
				fi->exp_color[2] = (b / 255.0f);
			}

			// check for custom warp glow
			if (optional_string("$Warp glow:"))
				stuff_string(fi->warp_glow, F_NAME, NAME_LENGTH);

			// check for custom warp ball
			if (optional_string("$Warp ball:"))
				stuff_string(fi->warp_ball, F_NAME, NAME_LENGTH);

			// check for custom warp model
			if (optional_string("$Warp model:"))
				stuff_string(fi->warp_model, F_NAME, NAME_LENGTH);
		}

		required_string("#End");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", table_filename, e.what()));
		return;
	}
}

void fireball_parse_tbl()
{
	if (fireballs_parsed)
		return;

	// every newly parsed fireball_info will get cleared before being added
	// must do this outside of parse_fireball_tbl because it's called twice
	Num_fireball_types = 0;

	parse_fireball_tbl("fireball.tbl");

	// look for any modular tables
	parse_modular_table(NOX("*-fbl.tbm"), parse_fireball_tbl);

	// fill in extra LOD filenames
	for (auto &fi: Fireball_info)
	{
		if (fi.lod_count > 1)
		{
			auto lod0 = fi.lod[0].filename;

			for (int j = 1; j < fi.lod_count; ++j)
				sprintf(fi.lod[j].filename, "%s_%d", lod0, j);
		}
	}

	fireballs_parsed = true;
}

void fireball_load_data()
{
	int				i, idx;
	fireball_info	*fd;

	for ( i = 0; i < Num_fireball_types; i++ ) {
		fd = &Fireball_info[i];

		for(idx=0; idx<fd->lod_count; idx++){
			// we won't use a warp effect lod greater than MAX_WARP_LOD so don't load it either
			if ( (i == FIREBALL_WARP) && (idx > MAX_WARP_LOD) )
				continue;

			fd->lod[idx].bitmap_id	= bm_load_animation( fd->lod[idx].filename, &fd->lod[idx].num_frames, &fd->lod[idx].fps, nullptr, nullptr, true );
			if ( fd->lod[idx].bitmap_id < 0 ) {
				Error(LOCATION, "Could not load %s anim file\n", fd->lod[idx].filename);
			}
		}

		if (strlen(fd->warp_glow) > 0) {
			mprintf(("Loading warp glow '%s'\n", fd->warp_glow));
			fd->warp_glow_bitmap = bm_load(fd->warp_glow);
		} else {
			fd->warp_glow_bitmap = -1;
		}

		if (strlen(fd->warp_ball) > 0) {
			mprintf(("Loading warp ball '%s'\n", fd->warp_ball));
			fd->warp_ball_bitmap = bm_load(fd->warp_ball);
		} else {
			fd->warp_ball_bitmap = -1;
		}
	}
}

// This will get called at the start of each level.
void fireball_init()
{
	if ( !fireballs_inited ) {
		// Do all the processing that happens only once
		fireball_parse_tbl();
		fireball_load_data();

		fireballs_inited = true;
	}
	
	// Reset everything between levels
	Fireballs.clear();
	Fireballs.reserve(INTITIAL_FIREBALL_CONTAINTER_SIZE);
	Unused_fireball_indices.clear();
	Unused_fireball_indices.reserve(INTITIAL_FIREBALL_CONTAINTER_SIZE);

	// Goober5000 - reset Knossos warp flag
	Knossos_warp_ani_used = 0;
}

MONITOR( NumFireballsRend )

/**
 * Delete a fireball.  
 * Called by object_delete() code... do not call directly.
 */
void fireball_delete( object * obj )
{
	int	num;
	fireball	*fb;

	num = obj->instance;
	// Make sure the new system works fine.
	Assert(obj->instance > -1);
	Assert(static_cast<int>(Fireballs.size()) > obj->instance);
	fb = &Fireballs[num];

	Assert( fb->objnum == OBJ_INDEX(obj));

	Fireballs[num].objnum = -1;
	Unused_fireball_indices.push_back(num);
}

/**
 * Delete all active fireballs, by calling obj_delete directly.
 */
void fireball_delete_all()
{
	for (auto& current_fireball : Fireballs) {
		if ( current_fireball.objnum != -1 ) {
			obj_delete(current_fireball.objnum);
		}
	}
}

void fireball_set_framenum(int num)
{
	int				framenum;
	fireball			*fb;
	fireball_info	*fd;
	fireball_lod	*fl;

	Assert(static_cast<int>(Fireballs.size()) > num);

	fb = &Fireballs[num];
	fd = &Fireball_info[Fireballs[num].fireball_info_index];

	// valid lod?
	fl = NULL;
	if((fb->lod >= 0) && (fb->lod < fd->lod_count)){
		fl = &Fireball_info[Fireballs[num].fireball_info_index].lod[fb->lod];
	}
	if(fl == NULL){
		// argh
		return;
	}

	if ( fb->fireball_render_type == FIREBALL_WARP_EFFECT )	{
		framenum = bm_get_anim_frame(fl->bitmap_id, fb->time_elapsed, 0.0f, true);

		if ( fb->orient )	{
			// warp out effect plays backwards
			framenum = fl->num_frames-framenum-1;
			fb->current_bitmap = fl->bitmap_id + framenum;
		} else {
			fb->current_bitmap = fl->bitmap_id + framenum;
		}
	} else {
		// ignore setting of OF_SHOULD_BE_DEAD, see fireball_process_post
		framenum = bm_get_anim_frame(fl->bitmap_id, fb->time_elapsed, fb->total_time);
		fb->current_bitmap = fl->bitmap_id + framenum;
	}
}

int fireball_is_perishable(object * obj)
{
	//	return 1;
	int			num, objnum;
	fireball		*fb;

	num = obj->instance;
	objnum = OBJ_INDEX(obj);
	// Make sure the new system works fine.
	Assert(obj->instance > -1);
	Assert((int)Fireballs.size() > obj->instance);
	Assert( Fireballs[num].objnum == objnum );

	fb = &Fireballs[num];

	if ( fb->fireball_render_type == FIREBALL_MEDIUM_EXPLOSION )	
		return 1;

	if ( !(fb->fireball_render_type == FIREBALL_WARP_EFFECT) )	{
		if ( !(obj->flags[Object::Object_Flags::Was_rendered]))	{
			return 1;
		}
	}

	return 0;
}

int fireball_is_warp(object * obj)
{
	int			num, objnum;
	fireball		*fb;

	num = obj->instance;
	objnum = OBJ_INDEX(obj);
	// Make sure the new system works fine.
	Assert(obj->instance > -1);
	Assert(static_cast<int>(Fireballs.size()) > obj->instance);
	Assert( Fireballs[num].objnum == objnum );

	fb = &Fireballs[num];

	if ( fb->fireball_render_type == FIREBALL_WARP_EFFECT)	
		return 1;

	return 0;
}

// maybe play sound effect for warp hole closing
void fireball_maybe_play_warp_close_sound(fireball *fb)
{
	float life_left;

	// If not a warphole fireball, do a quick out
	if ( !(fb->fireball_render_type == FIREBALL_WARP_EFFECT)) {
		return;
	}

	// If the warhole close sound has been played, don't play it again!
	if ( fb->flags & FBF_WARP_CLOSE_SOUND_PLAYED ) {
		return;
	}

	life_left = fb->total_time - fb->time_elapsed;

	if ( life_left < fb->warp_close_duration ) {
		fireball_play_warphole_close_sound(fb);
		fb->flags |= FBF_WARP_CLOSE_SOUND_PLAYED;
	}
}

MONITOR( NumFireballs )

void fireball_process_post(object * obj, float frame_time)
{
	int			num, objnum;
	fireball		*fb;

	MONITOR_INC( NumFireballs, 1 );	

	num = obj->instance;
	objnum = OBJ_INDEX(obj);
	// Make sure the new system works fine.
	Assert(obj->instance > -1);
	Assert(static_cast<int>(Fireballs.size()) > obj->instance);
	Assert( Fireballs[num].objnum == objnum );

	fb = &Fireballs[num];

	fb->time_elapsed += frame_time;
	if ( fb->time_elapsed > fb->total_time ) {
        obj->flags.set(Object::Object_Flags::Should_be_dead);
	}

	fireball_maybe_play_warp_close_sound(fb);

	fireball_set_framenum(num);
}

/**
 * Returns life left of a fireball in seconds
 */
float fireball_lifeleft( object *obj )
{
	int			num, objnum;
	fireball		*fb;

	num = obj->instance;
	objnum = OBJ_INDEX(obj);
	// Make sure the new system works fine.
	Assert(obj->instance > -1);
	Assert(static_cast<int>(Fireballs.size()) > obj->instance);

	Assert( Fireballs[num].objnum == objnum );

	fb = &Fireballs[num];

	return fb->total_time - fb->time_elapsed;
}

/**
 * Returns life left of a fireball in percent
 */
float fireball_lifeleft_percent( object *obj )
{
	int			num, objnum;
	fireball		*fb;

	// Make sure the new system works fine.
	Assert(obj->instance > -1);
	Assert(static_cast<int>(Fireballs.size()) > obj->instance);

	num = obj->instance;
	objnum = OBJ_INDEX(obj);
	Assert( Fireballs[num].objnum == objnum );

	fb = &Fireballs[num];

	float p = (fb->total_time - fb->time_elapsed) / fb->total_time;
	if (p < 0)p=0.0f;
	return p;
}

/**
 * Determine LOD to use
 */
int fireball_get_lod(vec3d *pos, fireball_info *fd, float size)
{
	vertex v;
	int x, y, w, h, bm_size;
	int must_stop = 0;
	int ret_lod = 1;
	int behind = 0;

	// bogus
	if(fd == NULL){
		return 1;
	}

	// start the frame
	extern int G3_count;

	if(!G3_count){
		g3_start_frame(1);
		must_stop = 1;
	}
	g3_set_view_matrix(&Eye_position, &Eye_matrix, Eye_fov);

	// get extents of the rotated bitmap
	g3_rotate_vertex(&v, pos);

	// if vertex is behind, find size if in front, then drop down 1 LOD
	if (v.codes & CC_BEHIND) {
		float dist = vm_vec_dist_quick(&Eye_position, pos);
		vec3d temp;

		behind = 1;
		vm_vec_scale_add(&temp, &Eye_position, &Eye_matrix.vec.fvec, dist);
		g3_rotate_vertex(&v, &temp);

		// if still behind, bail and go with default
		if (v.codes & CC_BEHIND) {
			behind = 0;
		}
	}

	if(!g3_get_bitmap_dims(fd->lod[0].bitmap_id, &v, size, &x, &y, &w, &h, &bm_size)) {
		if (Detail.hardware_textures == 4) {
			// straight LOD
			if(w <= bm_size/8){
				ret_lod = 3;
			} else if(w <= bm_size/2){
				ret_lod = 2;
			} else if(w <= (1.56*bm_size)){
				ret_lod = 1;
			} else {
				ret_lod = 0;
			}		
		} else {
			// less aggressive LOD for lower detail settings
			if(w <= bm_size/8){
				ret_lod = 3;
			} else if(w <= bm_size/3){
				ret_lod = 2;
			} else if(w <= (1.2*bm_size)){
				ret_lod = 1;
			} else {
				ret_lod = 0;
			}		
		}
	}

	// if it's behind, bump up LOD by 1
	if (behind) {
		ret_lod++;
	}

	// end the frame
	if(must_stop){
		g3_end_frame();
	}

	// return the best lod
	return MIN(ret_lod, fd->lod_count - 1);
}

/**
 * Create a fireball, return object index.
 */
int fireball_create(vec3d *pos, int fireball_type, int render_type, int parent_obj, float size, bool reverse, vec3d *velocity, float warp_lifetime, int ship_class, matrix *orient_override, int low_res, int extra_flags, gamesnd_id warp_open_sound, gamesnd_id warp_close_sound, float warp_open_duration, float warp_close_duration)
{
	int				n, objnum, fb_lod;
	object			*obj;
	fireball_info	*fd;
	fireball_lod	*fl;
	Assert( fireball_type > -1 );
	Assert( fireball_type < Num_fireball_types );

	fd = &Fireball_info[fireball_type];

	// check to make sure this fireball type exists
	if (!fd->lod_count)
		return -1;

	if ( !(Game_detail_flags & DETAIL_FLAG_FIREBALLS) )	{
		if ( !((fireball_type == FIREBALL_WARP) || (fireball_type == FIREBALL_KNOSSOS)) )	{
			return -1;
		}
	}

	if (Num_objects >= MAX_OBJECTS) {
		return -1;
	}


	if (!Unused_fireball_indices.empty()) {
		n = Unused_fireball_indices.back();
		Unused_fireball_indices.pop_back();
	}
	else {
		n = static_cast<int>(Fireballs.size());
		Fireballs.emplace_back();
	}

	fireball* new_fireball = &Fireballs[n];

	// get an lod to use	
	fb_lod = fireball_get_lod(pos, fd, size);

	// change lod if low res is desired
	if (low_res) {
		fb_lod++;
		fb_lod = MIN(fb_lod, fd->lod_count - 1);
	}

	// if this is a warpout fireball, never go higher than LOD 1
	if(fireball_type == FIREBALL_WARP){
		fb_lod = MAX_WARP_LOD;
	}
	fl = &fd->lod[fb_lod];

	new_fireball->lod = (char)fb_lod;

	new_fireball->flags = extra_flags;
	new_fireball->warp_open_sound_index = warp_open_sound;
	new_fireball->warp_close_sound_index = warp_close_sound;
	new_fireball->warp_open_duration = (warp_open_duration < 0.0f) ? WARPHOLE_GROW_TIME : warp_open_duration;
	new_fireball->warp_close_duration = (warp_close_duration < 0.0f) ? WARPHOLE_GROW_TIME : warp_close_duration;

	matrix orient;
	if(orient_override != NULL){
		orient = *orient_override;
	} else {
		if ( parent_obj < 0 )	{
			orient = vmd_identity_matrix;
		} else {
			orient = Objects[parent_obj].orient;
		}
	}
	
    flagset<Object::Object_Flags> default_flags;
    default_flags.set(Object::Object_Flags::Renders);
	objnum = obj_create(OBJ_FIREBALL, parent_obj, n, &orient, pos, size, default_flags, false);

	obj = &Objects[objnum];

	new_fireball->fireball_info_index = fireball_type;
	new_fireball->fireball_render_type = render_type;
	new_fireball->time_elapsed = 0.0f;
	new_fireball->objnum = objnum;
	new_fireball->current_bitmap = -1;
	
	switch( new_fireball->fireball_render_type )	{

		case FIREBALL_MEDIUM_EXPLOSION:	
			new_fireball->orient = Random::next() & 7;							// 0 - 7
			break;

		case FIREBALL_LARGE_EXPLOSION:
			new_fireball->orient = Random::next(360);						// 0 - 359
			break;

		case FIREBALL_WARP_EFFECT:
			// Play sound effect for warp hole opening up
			fireball_play_warphole_open_sound(ship_class, new_fireball);

			// warp in type
			if (reverse)	{
				new_fireball->orient = 1;
				// if warp out, then reverse the orientation
				vm_vec_scale( &obj->orient.vec.fvec, -1.0f );	// Reverse the forward vector
				vm_vec_scale( &obj->orient.vec.rvec, -1.0f );	// Reverse the right vector
			} else {
				new_fireball->orient = 0;
			}
			break;

		default:
			UNREACHABLE("Bad type set in fireball_create");
			break;
	}

	if ( new_fireball->fireball_render_type == FIREBALL_WARP_EFFECT )	{
		Assert( warp_lifetime >= 4.0f );		// Warp lifetime must be at least 4 seconds!
		if ( warp_lifetime < 4.0f )
			warp_lifetime = 4.0f;
		new_fireball->total_time = warp_lifetime;	// in seconds
	} else {
		new_fireball->total_time = i2fl(fl->num_frames) / fl->fps;	// in seconds
	}

	fireball_set_framenum(n);

	if ( velocity )	{
		// Make the explosion move at a constant velocity.
        obj->flags.set(Object::Object_Flags::Physics);
		obj->phys_info.mass = 1.0f;
		obj->phys_info.side_slip_time_const = 0.0f;
		obj->phys_info.rotdamp = 0.0f;
		obj->phys_info.vel = *velocity;
		obj->phys_info.max_vel = *velocity;
		obj->phys_info.desired_vel = *velocity;
		obj->phys_info.speed = vm_vec_mag(velocity);
		vm_vec_zero(&obj->phys_info.max_rotvel);
	}
	
	return objnum;
}

/**
 * Called at game shutdown to clean up the fireball system
 */
void fireball_close()
{
	if ( !fireballs_inited )
		return;

	fireball_delete_all();
}

void fireballs_page_in()
{
	int				i, idx;
	fireball_info	*fd;

	for ( i = 0; i < Num_fireball_types; i++ ) {
		fd = &Fireball_info[i];

		if((i < NUM_DEFAULT_FIREBALLS) || fireball_used[i]) {
			// if this is a Knossos ani, only load if Knossos_warp_ani_used is true
			if ( (i == FIREBALL_KNOSSOS) && !Knossos_warp_ani_used)
				continue;

			for(idx=0; idx<fd->lod_count; idx++) {
				// we won't use a warp effect lod greater than MAX_WARP_LOD so don't load it either
				if ( (i == FIREBALL_WARP) && (idx > MAX_WARP_LOD) )
					continue;

				bm_page_in_texture( fd->lod[idx].bitmap_id, fd->lod[idx].num_frames );
			}
		}

		// page in glow and ball bitmaps, if we have any
		bm_page_in_texture(fd->warp_glow_bitmap);
		bm_page_in_texture(fd->warp_ball_bitmap);

		// load the warp model, if we have one
		if (strlen(fd->warp_model) > 0 && cf_exists_full(fd->warp_model, CF_TYPE_MODELS)) {
			mprintf(("Loading warp model '%s'\n", fd->warp_model));
			fd->warp_model_id = model_load(fd->warp_model, 0, nullptr, 0);
		} else {
			fd->warp_model_id = -1;
		}
	}
}

void fireball_get_color(int idx, float *red, float *green, float *blue)
{
	Assert( red && blue && green );

	if ( (idx < 0) || (idx >= Num_fireball_types) ) {
		Int3();
		
		*red = 1.0f;
		*green = 1.0f;
		*blue = 1.0f;

		return;
	}

	fireball_info *fbi = &Fireball_info[idx];

	*red = fbi->exp_color[0];
	*green = fbi->exp_color[1];
	*blue = fbi->exp_color[2];
}

int fireball_ship_explosion_type(ship_info *sip)
{
	Assert( sip != NULL );

	int index = -1;
	int ship_fireballs = (int)sip->explosion_bitmap_anims.size();
	int objecttype_fireballs = -1;

	if (sip->class_type >= 0) {
		objecttype_fireballs = (int)Ship_types[sip->class_type].explosion_bitmap_anims.size();
	}

	if(ship_fireballs > 0){
		index = sip->explosion_bitmap_anims[Random::next(ship_fireballs)];
	} else if(objecttype_fireballs > 0){
		index = Ship_types[sip->class_type].explosion_bitmap_anims[Random::next(objecttype_fireballs)];
	}

	return index;
}

int fireball_asteroid_explosion_type(asteroid_info *aip)
{
	Assert( aip != NULL );

	if (aip->explosion_bitmap_anims.empty())
		return -1;

	int index = -1;
	int roid_fireballs = (int)aip->explosion_bitmap_anims.size();

	if (roid_fireballs > 0) {
		index = aip->explosion_bitmap_anims[Random::next(roid_fireballs)];
	}

	return index;
}

float fireball_wormhole_intensity(fireball *fb)
{
	float t = fb->time_elapsed;
	float rad;

	if ( t < fb->warp_open_duration )	{
		rad = (float)pow(t / fb->warp_open_duration, 0.4f);
	} else if ( t < fb->total_time - fb->warp_close_duration )	{
		rad = 1.0f;
	} else {
		rad = (float)pow((fb->total_time - t) / fb->warp_close_duration, 0.4f);
	}
	return rad;
} 

extern void warpin_queue_render(model_draw_list *scene, object *obj, matrix *orient, vec3d *pos, int texture_bitmap_num, float radius, float life_percent, float max_radius, bool warp_3d, int warp_glow_bitmap, int warp_ball_bitmap, int warp_model_id);

void fireball_render(object* obj, model_draw_list *scene)
{
	int		num;
	vertex	p;
	fireball	*fb;

	MONITOR_INC( NumFireballsRend, 1 );	

	// Make sure the new system works fine.
	Assert(obj->instance > -1);
	Assert(static_cast<int>(Fireballs.size()) > obj->instance);

	num = obj->instance;
	fb = &Fireballs[num];

	if ( fb->current_bitmap < 0 )
		return;

	float alpha = 1.0f;

	if (The_mission.flags[Mission::Mission_Flags::Fullneb] && Neb_affects_fireballs)
		alpha *= neb2_get_fog_visibility(&obj->pos, 
			Neb2_fog_visibility_fireball_const + (obj->radius * Neb2_fog_visibility_fireball_scaled_factor));
	
	g3_transfer_vertex(&p, &obj->pos);

	switch ( fb->fireball_render_type )	{

		case FIREBALL_MEDIUM_EXPLOSION: {
			batching_add_volume_bitmap(fb->current_bitmap, &p, fb->orient, obj->radius, alpha);
		}
		break;

		case FIREBALL_LARGE_EXPLOSION: {
			// Make the big explosions rotate with the viewer.
			batching_add_volume_bitmap_rotated(fb->current_bitmap, &p, (i2fl(fb->orient)*PI) / 180.0f, obj->radius, alpha);
		}
		break;

		case FIREBALL_WARP_EFFECT: {
			float percent_life = fb->time_elapsed / fb->total_time;
			float rad = obj->radius * fireball_wormhole_intensity(fb);

			fireball_info *fi = &Fireball_info[fb->fireball_info_index];
			warpin_queue_render(scene, obj, &obj->orient, &obj->pos, fb->current_bitmap, rad, percent_life, obj->radius, (fb->flags & FBF_WARP_3D) != 0, fi->warp_glow_bitmap, fi->warp_ball_bitmap, fi->warp_model_id);
		}
		break;


		default:
			Int3();
	}
}

// Because fireballs are only added and removed in two places, and Unused_fireball_indices is always updated in those places to contain unused indices,
// this very simple code will give you the correct count of currently existing fireballs in use. 
int fireball_get_count()
{
	int count = static_cast<int>(Fireballs.size()) - static_cast<int>(Unused_fireball_indices.size());

	Assert (count >= 0);
	
	return count;
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "asteroid/asteroid.h"
#include "cmdline/cmdline.h"
#include "fireball/fireballs.h"
#include "gamesnd/gamesnd.h"
#include "graphics/tmapper.h"
#include "localization/localize.h"
#include "model/model.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "render/3d.h"
#include "ship/ship.h"

#include <stdlib.h>


// make use of the LOD checker for tbl/tbm parsing (from weapons.cpp)
extern SCP_vector<lod_checker> LOD_checker;

static SCP_vector<color> LOD_color;

int Warp_model;
int Knossos_warp_ani_used;

#define WARPHOLE_GROW_TIME		(2.35f)	// time for warphole to reach max size (also time to shrink to nothing once it begins to shrink)

#define MAX_FIREBALL_LOD						4

#define MAX_FIREBALLS	200

#define MAX_WARP_LOD	0

fireball Fireballs[MAX_FIREBALLS];

fireball_info Fireball_info[MAX_FIREBALL_TYPES];

int fireball_used[MAX_FIREBALL_TYPES];

int Num_fireballs = 0;
int Num_fireball_types = 0;

int fireballs_inited = 0;

int Warp_glow_bitmap = -1;
int Warp_ball_bitmap = -1;

#define FB_INDEX(fb)	(fb-Fireballs)

/**
 * Play warp in sound for warp effect
 */
void fireball_play_warphole_open_sound(int ship_class, fireball *fb)
{
	int		sound_index;
	float		range_multiplier = 1.0f;
	object	*fireball_objp;	
		
	Assert((fb != NULL) && (fb->objnum >= 0));
	if((fb == NULL) || (fb->objnum < 0)){
		return;
	}
	fireball_objp = &Objects[fb->objnum];

	sound_index = SND_WARP_IN;

	if(fb->warp_open_sound_index > -1) {
		sound_index = fb->warp_open_sound_index;
	} else if ((ship_class >= 0) && (ship_class < static_cast<int>(Ship_info.size()))) {
		if ( Ship_info[ship_class].flags & SIF_HUGE_SHIP ) {
			sound_index = SND_CAPITAL_WARP_IN;
			fb->flags |= FBF_WARP_CAPITAL_SIZE;
		} else if ( Ship_info[ship_class].flags & SIF_BIG_SHIP ) {
			range_multiplier = 6.0f;
			fb->flags |= FBF_WARP_CRUISER_SIZE;
		}
	}

	snd_play_3d(&Snds[sound_index], &fireball_objp->pos, &Eye_position, fireball_objp->radius, NULL, 0, 1.0f, SND_PRIORITY_DOUBLE_INSTANCE, NULL, range_multiplier); // play warp sound effect
}

/**
 * Play warp out sound for warp effect
 */
void fireball_play_warphole_close_sound(fireball *fb)
{
	int	sound_index;	

	object *fireball_objp;

	fireball_objp = &Objects[fb->objnum];

	sound_index = SND_WARP_OUT;

	if ( fb->warp_close_sound_index > -1 ) {
		sound_index = fb->warp_close_sound_index;
	} else if ( fb->flags & FBF_WARP_CAPITAL_SIZE ) {
		sound_index = SND_CAPITAL_WARP_OUT;
	} else {
		return;
	}

	snd_play_3d(&Snds[sound_index], &fireball_objp->pos, &Eye_position, fireball_objp->radius); // play warp sound effect
}

/**
 * Set default colors for each explosion type (original values from object.cpp)
 */
static void fireball_set_default_color(int idx)
{
	Assert( (idx >= 0) && (idx < MAX_FIREBALL_TYPES) );

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

/**
 * Parse fireball tbl
 *
 * NOTE: we can't be too trusting here so a tbm will only modify the LOD count, not add an entry
 */
void parse_fireball_tbl(const char *filename)
{
	lod_checker lod_check;
	color fb_color;

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Start");

		while (required_string_either("#End", "$Name:")) {
			memset(&lod_check, 0, sizeof(lod_checker));

			// base filename
			required_string("$Name:");
			stuff_string(lod_check.filename, F_NAME, MAX_FILENAME_LEN);

			lod_check.override = -1;

			// these entries should only be in TBMs, and it has to include at least one
			if (Parsing_modular_table) {
				if (optional_string("+Explosion_Medium")) {
					lod_check.override = FIREBALL_EXPLOSION_MEDIUM;
				}
				else if (optional_string("+Warp_Effect")) {
					lod_check.override = FIREBALL_WARP;
				}
				else if (optional_string("+Knossos_Effect")) {
					lod_check.override = FIREBALL_KNOSSOS;
				}
				else if (optional_string("+Asteroid")) {
					lod_check.override = FIREBALL_ASTEROID;
				}
				else if (optional_string("+Explosion_Large1")) {
					lod_check.override = FIREBALL_EXPLOSION_LARGE1;
				}
				else if (optional_string("+Explosion_Large2")){
					lod_check.override = FIREBALL_EXPLOSION_LARGE2;
				}
				else {
					required_string("+Custom_Fireball");
					stuff_int(&lod_check.override);
				}
			}

			lod_check.num_lods = 1;

			// Do we have an LOD num
			if (optional_string("$LOD:")) {
				stuff_int(&lod_check.num_lods);
			}

			if (lod_check.num_lods > MAX_FIREBALL_LOD) {
				lod_check.num_lods = MAX_FIREBALL_LOD;
			}

			// check for particular lighting color
			if (optional_string("$Light color:")) {
				int r, g, b;

				stuff_int(&r);
				stuff_int(&g);
				stuff_int(&b);

				CLAMP(r, 0, 255);
				CLAMP(g, 0, 255);
				CLAMP(b, 0, 255);

				gr_init_color(&fb_color, r, g, b);
			}
			else {
				// to keep things simple, we just use 0 alpha to indicate that a default value should be used
				memset(&fb_color, 0, sizeof(color));
			}

			// we may use one filename for multiple entries so we'll have to handle dupes post parse
			LOD_checker.push_back(lod_check);
			LOD_color.push_back(fb_color);
		}

		required_string("#End");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void fireball_parse_tbl()
{
	int i = 0, j;
	SCP_vector<lod_checker>::iterator lod;

	memset( &Fireball_info, 0, sizeof(fireball_info) * MAX_FIREBALL_TYPES );


	parse_fireball_tbl("fireball.tbl");

	// look for any modular tables
	parse_modular_table(NOX("*-fbl.tbm"), parse_fireball_tbl);

	// we've got our list so pass it off for final checking and loading.
	// we assume that entries in fireball.tbl are in the correct order
	for (lod = LOD_checker.begin(); lod != LOD_checker.end(); ++lod) {
		if ( (i < MAX_FIREBALL_TYPES) && (lod->override < 0) ) {
			strcpy_s( Fireball_info[i].lod[0].filename, lod->filename );
			Fireball_info[i].lod_count = lod->num_lods;
			Num_fireball_types++;

			if (LOD_color[i].alpha == 255) {
				Fireball_info[i].exp_color[0] = (LOD_color[i].red / 255.0f);
				Fireball_info[i].exp_color[1] = (LOD_color[i].green / 255.0f);
				Fireball_info[i].exp_color[2] = (LOD_color[i].blue / 255.0f);
			} else {
				fireball_set_default_color(i);
			}
		}
		i++;
	}

	// having to do this twice is less than optimal, but less error prone too.
	// this handles (and should only have to handle) TBM related entries
	i = 0;
	for (lod = LOD_checker.begin(); lod != LOD_checker.end(); ++lod) {
		// try entry replacement
		if ( (lod->override >= 0) && (lod->override < Num_fireball_types) ) {
			strcpy_s( Fireball_info[lod->override].lod[0].filename, lod->filename );
			Fireball_info[lod->override].lod_count = lod->num_lods;

			if (LOD_color[i].alpha == 255) {
				Fireball_info[lod->override].exp_color[0] = (LOD_color[i].red / 255.0f);
				Fireball_info[lod->override].exp_color[1] = (LOD_color[i].green / 255.0f);
				Fireball_info[lod->override].exp_color[2] = (LOD_color[i].blue / 255.0f);
			} else {
				fireball_set_default_color(lod->override);
			}
		}
	}

	// fill in extra LOD filenames
	for (i = 0; i < Num_fireball_types; i++) {
		for (j = 1; j < Fireball_info[i].lod_count; j++) {
			sprintf( Fireball_info[i].lod[j].filename, "%s_%d", Fireball_info[i].lod[0].filename, j);
		}
	}

	// done
	LOD_checker.clear();
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

			fd->lod[idx].bitmap_id	= bm_load_animation( fd->lod[idx].filename, &fd->lod[idx].num_frames, &fd->lod[idx].fps, NULL, 1 );
			if ( fd->lod[idx].bitmap_id < 0 ) {
				Error(LOCATION, "Could not load %s anim file\n", fd->lod[idx].filename);
			}
		}
	} 

	if ( Warp_glow_bitmap == -1 )	{
		Warp_glow_bitmap = bm_load( NOX("warpglow01") );
	}
	if ( Warp_ball_bitmap == -1 )	{
		Warp_ball_bitmap = bm_load( NOX("warpball01") );
	}
}

// This will get called at the start of each level.
void fireball_init()
{
	int i;

	if ( !fireballs_inited ) {
		fireballs_inited = 1;

		// Do all the processing that happens only once
		fireball_parse_tbl();
		fireball_load_data();
	}
	
	// Reset everything between levels
	Num_fireballs = 0;
	for (i=0; i<MAX_FIREBALLS; i++ )	{
		Fireballs[i].objnum	= -1;
	}

	// Goober5000 - reset Knossos warp flag
	Knossos_warp_ani_used = 0;

	mprintf(("Loading warp model\n"));
	Warp_model = -1;

	// Goober5000 - check for existence of file before trying to load it
	if (cf_exists_full("warp.pof", CF_TYPE_MODELS))
	{
		Warp_model = model_load("warp.pof", 0, NULL, 0);
	}

	mprintf((" %d\n", Warp_model));
}

MONITOR( NumFireballsRend )

void fireball_render_DEPRECATED(object * obj)
{
	int		num;
	vertex	p;
	fireball	*fb;
    
	MONITOR_INC( NumFireballsRend, 1 );	
	
	memset(&p, 0, sizeof(p));
    
    num = obj->instance;
	fb = &Fireballs[num];

	if ( Fireballs[num].current_bitmap < 0 )
		return;

	// turn off fogging
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	if(Cmdline_nohtl) {
		g3_rotate_vertex(&p, &obj->pos );
	}else{
		g3_transfer_vertex(&p, &obj->pos);
	}

	switch( fb->fireball_render_type )	{

		case FIREBALL_MEDIUM_EXPLOSION:
			batch_add_bitmap (
				Fireballs[num].current_bitmap, 
				TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD, 
				&p, 
				fb->orient, 
				obj->radius
			);
			break;

		case FIREBALL_LARGE_EXPLOSION:
			// Make the big explosions rotate with the viewer.
			batch_add_bitmap_rotated ( 
				Fireballs[num].current_bitmap, 
				TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD, 
				&p, 
				(i2fl(fb->orient)*PI)/180.0f,
				obj->radius
			);
			break;

		case FIREBALL_WARP_EFFECT: {
			
				float percent_life = fb->time_elapsed / fb->total_time;

				float rad;

				// Code to make effect grow/shrink. 
				float t = fb->time_elapsed;
			
				if ( t < WARPHOLE_GROW_TIME )	{
					rad = (float)pow(t/WARPHOLE_GROW_TIME,0.4f)*obj->radius;
				} else if ( t < fb->total_time - WARPHOLE_GROW_TIME )	{
					rad = obj->radius;
				} else {
					rad = (float)pow((fb->total_time - t)/WARPHOLE_GROW_TIME,0.4f)*obj->radius;
				}

				warpin_render(obj, &obj->orient, &obj->pos, Fireballs[num].current_bitmap, rad, percent_life, obj->radius, (Fireballs[num].flags & FBF_WARP_3D) );
			}
			break;

			
		default:
			Int3();
	}
}

/**
 * Delete a fireball.  
 * Called by object_delete() code... do not call directly.
 */
void fireball_delete( object * obj )
{
	int	num;
	fireball	*fb;

	num = obj->instance;
	fb = &Fireballs[num];

	Assert( fb->objnum == OBJ_INDEX(obj));

	Fireballs[num].objnum = -1;
	Num_fireballs--;
	Assert( Num_fireballs >= 0 );
}

/**
 * Delete all active fireballs, by calling obj_delete directly.
 */
void fireball_delete_all()
{
	fireball	*fb;
	int		i;

	for ( i = 0; i < MAX_FIREBALLS; i++ ) {
		fb = &Fireballs[i];
		if ( fb->objnum != -1 ) {
			obj_delete(fb->objnum);
		}
	}
}

void fireball_set_framenum(int num)
{
	int				framenum;
	fireball			*fb;
	fireball_info	*fd;
	fireball_lod	*fl;

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
		float total_time = i2fl(fl->num_frames) / fl->fps;	// in seconds

		framenum = fl2i(fb->time_elapsed * fl->num_frames / total_time + 0.5);

		if ( framenum < 0 ) framenum = 0;

		framenum = framenum % fl->num_frames;

		if ( fb->orient )	{
			// warp out effect plays backwards
			framenum = fl->num_frames-framenum-1;
			fb->current_bitmap = fl->bitmap_id + framenum;
		} else {
			fb->current_bitmap = fl->bitmap_id + framenum;
		}
	} else {

		framenum = fl2i(fb->time_elapsed / fb->total_time * fl->num_frames + 0.5);

		// ensure we don't go past the number of frames of animation
		if ( framenum > (fl->num_frames-1) ) {
			framenum = (fl->num_frames-1);
			Objects[fb->objnum].flags |= OF_SHOULD_BE_DEAD;
		}

		if ( framenum < 0 ) framenum = 0;
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
	Assert( Fireballs[num].objnum == objnum );

	fb = &Fireballs[num];

	if ( fb->fireball_render_type == FIREBALL_MEDIUM_EXPLOSION )	
		return 1;

	if ( !(fb->fireball_render_type == FIREBALL_WARP_EFFECT) )	{
		if ( !(obj->flags & OF_WAS_RENDERED))	{
			return 1;
		}
	}

	return 0;
}


/**
 * There are too many fireballs, so delete the oldest small one
 * to free up a slot.  
 *
 * @return The fireball slot freed.
 */
int fireball_free_one()
{
	fireball	*fb;
	int		i;

	int		oldest_objnum = -1, oldest_slotnum = -1;
	float		lifeleft, oldest_lifeleft = 0.0f;

	for ( i = 0; i < MAX_FIREBALLS; i++ ) {
		fb = &Fireballs[i];

		// only remove the ones that aren't warp effects
		if ( (fb->objnum >= 0) && fireball_is_perishable(&Objects[fb->objnum]) )	{

			lifeleft = fb->total_time - fb->time_elapsed;
			if ( (oldest_objnum < 0) || (lifeleft < oldest_lifeleft) )	{
				oldest_slotnum = i;
				oldest_lifeleft = lifeleft;
				oldest_objnum = fb->objnum;
			}
			break;
		}
	}

	if ( oldest_objnum > -1 )	{
		obj_delete(oldest_objnum);
	}
	return oldest_slotnum;
}

int fireball_is_warp(object * obj)
{
	int			num, objnum;
	fireball		*fb;

	num = obj->instance;
	objnum = OBJ_INDEX(obj);
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

	if ( life_left < WARPHOLE_GROW_TIME ) {
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
	Assert( Fireballs[num].objnum == objnum );

	fb = &Fireballs[num];

	fb->time_elapsed += frame_time;
	if ( fb->time_elapsed > fb->total_time ) {
		obj->flags |= OF_SHOULD_BE_DEAD;
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
int fireball_create( vec3d * pos, int fireball_type, int render_type, int parent_obj, float size, int reverse, vec3d *velocity, float warp_lifetime, int ship_class, matrix *orient_override, int low_res, int extra_flags, int warp_open_sound, int warp_close_sound)
{
	int				n, objnum, fb_lod;
	object			*obj;
	fireball			*fb;
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

	if ( (Num_fireballs >= MAX_FIREBALLS) || (Num_objects >= MAX_OBJECTS) )	{

		// out of slots, so free one up.
		n = fireball_free_one();
		if ( n < 0 ) {
			return -1;
		}
	} else {
		for ( n = 0; n < MAX_FIREBALLS; n++ )	{
			if ( Fireballs[n].objnum < 0  )	{
				break;
			}
		}
		Assert( n != MAX_FIREBALLS );
	}

	fb = &Fireballs[n];

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

	fb->lod = (char)fb_lod;

	fb->flags = extra_flags;
	fb->warp_open_sound_index = warp_open_sound;
	fb->warp_close_sound_index = warp_close_sound;

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
	
	objnum = obj_create(OBJ_FIREBALL, parent_obj, n, &orient, pos, size, OF_RENDERS);

	if (objnum < 0) {
		Int3();				// Get John, we ran out of objects for fireballs
		return objnum;
	}

	obj = &Objects[objnum];

	fb->fireball_info_index = fireball_type;
	fb->fireball_render_type = render_type;
	fb->time_elapsed = 0.0f;
	fb->objnum = objnum;
	fb->current_bitmap = -1;
	
	switch( fb->fireball_render_type )	{

		case FIREBALL_MEDIUM_EXPLOSION:	
			fb->orient = (myrand()>>8) & 7;							// 0 - 7
			break;

		case FIREBALL_LARGE_EXPLOSION:
			fb->orient = (myrand()>>8) % 360;						// 0 - 359
			break;

		case FIREBALL_WARP_EFFECT:
			// Play sound effect for warp hole opening up
			fireball_play_warphole_open_sound(ship_class, fb);

			// warp in type
			if (reverse)	{
				fb->orient = 1;
				// if warp out, then reverse the orientation
				vm_vec_scale( &obj->orient.vec.fvec, -1.0f );	// Reverse the forward vector
				vm_vec_scale( &obj->orient.vec.rvec, -1.0f );	// Reverse the right vector
			} else {
				fb->orient = 0;
			}
			break;

		default:
			Int3();
			break;
	}

	if ( fb->fireball_render_type == FIREBALL_WARP_EFFECT )	{
		Assert( warp_lifetime >= 4.0f );		// Warp lifetime must be at least 4 seconds!
		if ( warp_lifetime < 4.0f )
			warp_lifetime = 4.0f;
		fb->total_time = warp_lifetime;	// in seconds
	} else {
		fb->total_time = i2fl(fl->num_frames) / fl->fps;	// in seconds
	}
	
	fireball_set_framenum(n);

	if ( velocity )	{
		// Make the explosion move at a constant velocity.
		obj->flags |= OF_PHYSICS;
		obj->phys_info.mass = 1.0f;
		obj->phys_info.side_slip_time_const = 0.0f;
		obj->phys_info.rotdamp = 0.0f;
		obj->phys_info.vel = *velocity;
		obj->phys_info.max_vel = *velocity;
		obj->phys_info.desired_vel = *velocity;
		obj->phys_info.speed = vm_vec_mag(velocity);
		vm_vec_zero(&obj->phys_info.max_rotvel);
	}
	
	Num_fireballs++;
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
		if((i < NUM_DEFAULT_FIREBALLS) || fireball_used[i]){
			fd = &Fireball_info[i];

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
	}

	bm_page_in_texture( Warp_glow_bitmap );
	bm_page_in_texture( Warp_ball_bitmap );
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
		index = sip->explosion_bitmap_anims[rand()%ship_fireballs];
	} else if(objecttype_fireballs > 0){
		index = Ship_types[sip->class_type].explosion_bitmap_anims[rand()%objecttype_fireballs];
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
		index = aip->explosion_bitmap_anims[rand()%roid_fireballs];
	}

	return index;
}

float fireball_wormhole_intensity( object *obj )
{
	int			num, objnum;
	fireball		*fb;

	num = obj->instance;
	objnum = OBJ_INDEX(obj);
	Assertion( Fireballs[num].objnum == objnum, "Basic sanity check. Fireballs[num].objnum (%d) should == objnum (%d)", Fireballs[num].objnum, objnum );

	fb = &Fireballs[num];

	float t = fb->time_elapsed;
	float rad;

	if ( t < WARPHOLE_GROW_TIME )	{
		rad = (float)pow(t/WARPHOLE_GROW_TIME,0.4f);
	} else if ( t < fb->total_time - WARPHOLE_GROW_TIME )	{
		rad = 1;
	} else {
		rad = (float)pow((fb->total_time - t)/WARPHOLE_GROW_TIME,0.4f);
	}
	return rad;
} 

void fireball_render(object* obj, draw_list *scene)
{
	int		num;
	vertex	p;
	fireball	*fb;

	MONITOR_INC( NumFireballsRend, 1 );	

	num = obj->instance;
	fb = &Fireballs[num];

	if ( Fireballs[num].current_bitmap < 0 )
		return;
	
	if ( Cmdline_nohtl ) {
		g3_rotate_vertex(&p, &obj->pos );
	} else {
		g3_transfer_vertex(&p, &obj->pos);
	}

	switch ( fb->fireball_render_type )	{

		case FIREBALL_MEDIUM_EXPLOSION: {
			batch_add_bitmap (
				Fireballs[num].current_bitmap, 
				TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD, 
				&p, 
				fb->orient, 
				obj->radius
				);
		}
		break;

		case FIREBALL_LARGE_EXPLOSION: {
			// Make the big explosions rotate with the viewer.
			batch_add_bitmap_rotated ( 
				Fireballs[num].current_bitmap, 
				TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT | TMAP_FLAG_SOFT_QUAD, 
				&p, 
				(i2fl(fb->orient)*PI)/180.0f,
				obj->radius
				);
		}
		break;

		case FIREBALL_WARP_EFFECT: {
			float percent_life = fb->time_elapsed / fb->total_time;

			float rad;

			// Code to make effect grow/shrink. 
			float t = fb->time_elapsed;

			if ( t < WARPHOLE_GROW_TIME )	{
				rad = (float)pow(t/WARPHOLE_GROW_TIME,0.4f)*obj->radius;
			} else if ( t < fb->total_time - WARPHOLE_GROW_TIME )	{
				rad = obj->radius;
			} else {
				rad = (float)pow((fb->total_time - t)/WARPHOLE_GROW_TIME,0.4f)*obj->radius;
			}

			warpin_queue_render(scene, obj, &obj->orient, &obj->pos, Fireballs[num].current_bitmap, rad, percent_life, obj->radius, (Fireballs[num].flags & FBF_WARP_3D) );
		}
		break;


		default:
			Int3();
	}
}

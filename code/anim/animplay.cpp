/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "anim/animplay.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "bmpman/bmpman.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "pcxutils/pcxutils.h"
#include "anim/packunpack.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"



static color Color_xparent;

anim *first_anim = NULL;
anim_instance anim_free_list;
anim_instance anim_render_list;

#define MAX_ANIM_INSTANCES 25
anim_instance anim_render_instance[MAX_ANIM_INSTANCES];

int Anim_paused;	/// Global variable to pause the playing back of anims
int Anim_inited = FALSE;

fix t1,t2;

int Anim_ignore_frametime=0;	// flag used to ignore frametime... useful when need to avoid saturated frametimes

/**
 * @brief Initialise animation
 * @details Queue all the ::anim_render_instance[] elements onto the ::anim_free_list
 */
void anim_init()
{
	int i;

	if ( Anim_inited == TRUE )
		return;

	list_init( &anim_free_list );
	list_init( &anim_render_list );

	// Link all anim render slots into the free list
	for (i=1; i < MAX_ANIM_INSTANCES; i++)	{
		list_append(&anim_free_list, &anim_render_instance[i]);
	}
	
	Anim_paused = 0;
	Anim_inited = TRUE;
}

/**
 * @brief Display the frames for the currently playing anims
 */
void anim_render_all(int screen_id, float frametime)
{
	anim_instance* A;
	anim_instance* temp;

	A = GET_FIRST(&anim_render_list);
	while( A !=END_OF_LIST(&anim_render_list) )	{
		temp = GET_NEXT(A);
		if ( A->screen_id == screen_id ) {
			if ( Anim_ignore_frametime ) {
				frametime = 0.0f;
				Anim_ignore_frametime=0;
			}
			if ( anim_show_next_frame(A, frametime) == -1 ) {
				A->data = NULL;
				anim_release_render_instance(A);	
			}
		}
		A = temp;
	}
}

/**
 * @brief Display the frames for the passed animation
 * @details It will ignore animations which do not have the same id as the passed screen_id
 */
void anim_render_one(int screen_id, anim_instance *ani, float frametime)
{
	// make sure this guy's screen id matches the passed one
	if(screen_id != ani->screen_id){
		return;
	}

	// otherwise render it	
	if ( Anim_ignore_frametime ) {
		frametime = 0.0f;
		Anim_ignore_frametime=0;
	}
	if ( anim_show_next_frame(ani, frametime) == -1 ) {
		ani->data = NULL;
		anim_release_render_instance(ani);	
	}	
}

MONITOR(NumANIPlayed)

/**
 * @brief Setup an anim_play_struct for passing into ::anim_play().  
 * @details Will fill in default values, which you can then change before calling ::anim_play().
 */
void anim_play_init(anim_play_struct *aps, anim *a_info, int x, int y, int base_w, int base_h)
{
	aps->anim_info = a_info;
	aps->x = x;
	aps->y = y;
	aps->base_w = base_w;
	aps->base_h = base_h;
	aps->start_at = 0;
	aps->stop_at = a_info->total_frames - 1;
	aps->screen_id = 0;
	aps->world_pos = NULL;
	aps->radius = 0.0f;
	aps->framerate_independent = 0;
	aps->color = NULL;
	aps->skip_frames = 1;
	aps->looped = 0;
	aps->ping_pong = 0;
}

/**
 * @brief Will add an anim instance to the anim_render_list.  
 * This will cause the anim to be played at the x,y position specified in the parameter list.
 *
 * @param aps Compressed animation that we should make an instance from
 * @return If success pointer to instance, NULL if anim anim could not be played
 */
anim_instance *anim_play(anim_play_struct *aps)
{
	Assert( aps->anim_info != NULL );
	Assert( aps->start_at >= 0 );
	Assert( aps->stop_at < aps->anim_info->total_frames );
	Assert( !(aps->looped && aps->ping_pong) );  // shouldn't have these both set at once

	MONITOR_INC(NumANIPlayed, 1);

	anim_instance *instance;

	// Find next free anim instance slot on queue
	instance = GET_FIRST(&anim_free_list);
	Assert( instance != &anim_free_list );  // shouldn't have the dummy element

	// remove instance from the free list
	list_remove( &anim_free_list, instance );

	// insert instance onto the end of anim_render_list
	list_append( &anim_render_list, instance );

	aps->anim_info->instance_count++;
	instance->frame_num = -1;
	instance->last_frame_num = -99;
	instance->parent = aps->anim_info;
	instance->data = aps->anim_info->data;
	if ( anim_instance_is_streamed(instance) ) {
		instance->file_offset = instance->parent->file_offset;
	}
	instance->frame = (ubyte *) vm_malloc(instance->parent->width * instance->parent->height * 2);
	Assert( instance->frame != NULL );
	memset( instance->frame, 0, instance->parent->width * instance->parent->height * 2 );
	instance->time_elapsed = 0.0f;
	instance->stop_at = aps->stop_at;
	instance->x = aps->x;
	instance->y = aps->y;
	instance->world_pos = aps->world_pos;
	instance->radius = aps->radius;
	instance->framerate_independent = aps->framerate_independent;
	instance->last_bitmap = -1;
	instance->stop_now = FALSE;
	instance->screen_id = aps->screen_id;
	instance->aa_color = aps->color;
	instance->skip_frames = aps->skip_frames;
	instance->looped = aps->looped;
	instance->ping_pong = aps->ping_pong;
	instance->direction = ANIM_DIRECT_FORWARD;
	instance->paused = 0;
	instance->loop_count = 0;
	if ( aps->color == NULL ){
		instance->xlate_pal = 1;
	} else {
		instance->xlate_pal = 0;
	}

	if(aps->base_w < 0 || aps->base_h < 0) {
		instance->base_w = gr_screen.max_w_unscaled;
		instance->base_h = gr_screen.max_h_unscaled;
	} else {
		instance->base_w = aps->base_w;
		instance->base_h = aps->base_h;
	}

	// determining the start_at frame is more complicated, since it must be a key-frame.
	// Futhermore, need to subtract 1 from key-frame number, since frame number is always
	// incremented the first time anim_show_next_frame() is called

	instance->start_at = aps->start_at;

	if ( aps->start_at > 0 ) {
		key_frame *keyp;
		int idx;
		int key = 0;
		int offset = 0;
		int frame_num = aps->start_at;

		keyp = instance->parent->keys;
		idx = 0;
		while (idx < instance->parent->num_keys) {
			if (key == frame_num)
				break;

			key = keyp[idx].frame_num - 1;
			offset = keyp[idx].offset;

			idx++;
		}

		if (key > instance->frame_num) {  // best key is closer than current position
			instance->frame_num = key;
			if ( anim_instance_is_streamed(instance) ) {
				instance->file_offset = instance->parent->file_offset + offset;
			} else {
				instance->data = instance->parent->data + offset;
			}

		}

		instance->frame_num--;	// required
	}

	return instance;
}

/**
 * @brief This function is called to blit the next frame of an anim instance to the screen.
 * This is normally called by the anim_render_all() function.
 *
 * @param instance Pointer to animation instance
 * @param frametime	Time elapsed since last call, in seconds
 */
int anim_show_next_frame(anim_instance *instance, float frametime)
{
	int	bitmap_id, bitmap_flags=0, new_frame_num, frame_diff=0, i, n_frames=0,frame_save;
	float percent_through, decompress_time, render_time, time;
	vertex	image_vertex;
	int aabitmap = 0;
	int bpp = 16;

	Assert( instance != NULL );

	instance->time_elapsed += frametime;

	// Advance to the next frame, if we determine enough time has elapsed.
	if(instance->direction == ANIM_DIRECT_FORWARD)
		n_frames = instance->stop_at - instance->start_at + 1;
	else if(instance->direction == ANIM_DIRECT_REVERSE)
		n_frames = instance->start_at - instance->stop_at + 1;

	time = n_frames / i2fl(instance->parent->fps);

	percent_through = instance->time_elapsed / time;

	if(instance->direction == ANIM_DIRECT_FORWARD)
		new_frame_num = instance->start_at - 1 + fl2i(percent_through * n_frames + 0.5f);
	else
		new_frame_num = instance->start_at - 1 - fl2i(percent_through * n_frames + 0.5f);

	frame_save = instance->frame_num;

	// If framerate independent, use the new_frame_num... unless instance->skip_frames is
	// FALSE, then only advance a maximum of one frame (this is needed since some big animations
	// should just play slower rather than taking the hit of decompressing multiple frames and
	// creating an even greater slowdown	
	if (instance->framerate_independent) {
		if(instance->direction == ANIM_DIRECT_FORWARD){
			if ( new_frame_num > instance->last_frame_num) {
				if ( instance->skip_frames )
					instance->frame_num = new_frame_num;
				else
					instance->frame_num++;
			}
		} else if(instance->direction == ANIM_DIRECT_REVERSE){
			if( new_frame_num < instance->last_frame_num) {
				if ( instance->skip_frames )
					instance->frame_num = new_frame_num;
				else
					instance->frame_num--;
			}
		}
	}
	else {			
		if(instance->direction == ANIM_DIRECT_FORWARD){
			if ( new_frame_num > instance->last_frame_num) {
				instance->frame_num++;
			}
		} else if(instance->direction == ANIM_DIRECT_REVERSE){
			if ( new_frame_num < instance->last_frame_num) {
				instance->frame_num--;
			}
		}			
	}

	if(instance->direction == ANIM_DIRECT_FORWARD){
		if ( instance->frame_num < instance->start_at ) {
			instance->frame_num = instance->start_at;
		}	
	} else if(instance->direction == ANIM_DIRECT_REVERSE){
		if ( instance->frame_num > instance->start_at ) {
			instance->frame_num = instance->start_at;
		}	
	}

	if ( instance->stop_now == TRUE ) {
		return -1;
	}

	// If past the last frame, clamp to the last frame and then set the stop_now flag in the
	// anim instance.  The next iteration, the animation will stop.
	if(instance->direction == ANIM_DIRECT_FORWARD){
		if (instance->frame_num >= instance->stop_at ) {
			if (instance->looped) {										// looped animations
				instance->frame_num = instance->stop_at;
				instance->time_elapsed = 0.0f;
			} else if(instance->ping_pong) {							// pingponged animations
				instance->frame_num = instance->stop_at;
				anim_reverse_direction(instance);
			} else {															// one-shot animations
				instance->frame_num = instance->stop_at;
				instance->last_frame_num = instance->frame_num;
				instance->stop_now = TRUE;
			}
		}
	} else if(instance->direction == ANIM_DIRECT_REVERSE){
		if (instance->frame_num <= instance->stop_at ) {
			if (instance->looped) {										// looped animations
				instance->frame_num = instance->stop_at;
				instance->time_elapsed = 0.0f;
			} else if(instance->ping_pong) {							// pingponged animations
				instance->frame_num = instance->stop_at;
				anim_reverse_direction(instance);
			} else {															// one-shot animations
				instance->frame_num = instance->stop_at+1;
				instance->last_frame_num = instance->frame_num;
				instance->stop_now = TRUE;
			}
		}
	}

	if(instance->direction == ANIM_DIRECT_FORWARD){
		if( instance->last_frame_num >= instance->start_at ) {
			frame_diff = instance->frame_num - instance->last_frame_num;		
		} else {
			frame_diff = 1;
		}
	} else if(instance->direction == ANIM_DIRECT_REVERSE){
		if( instance->last_frame_num <= instance->start_at ) {
			frame_diff = instance->last_frame_num - instance->frame_num;
		} else {
			frame_diff = 1;
		}
	}		
	Assert(frame_diff >= 0);
	Assert( instance->frame_num >= 0 && instance->frame_num < instance->parent->total_frames );

	// if the anim is paused, ignore all the above changes and still display this frame
	if(instance->paused || Anim_paused){
		instance->frame_num = frame_save;
		instance->time_elapsed -= frametime;
		frame_diff = 0;
	}

	if (instance->parent->flags & ANF_XPARENT){
		bitmap_flags = 0;
	} 
	bpp = 16;
	if(instance->aa_color != NULL){
		bitmap_flags |= BMP_AABITMAP;
		aabitmap = 1;
		bpp = 8;
	}	

	if ( frame_diff > 0 ) {
		instance->last_frame_num = instance->frame_num;		

		t1 = timer_get_fixed_seconds();
		for ( i = 0; i < frame_diff; i++ ) {
			anim_check_for_palette_change(instance);			

			// if we're playing backwards, every frame must be a keyframe and we set the data ptr here
			if(instance->direction == ANIM_DIRECT_REVERSE){
				if ( anim_instance_is_streamed(instance) ) {
					instance->file_offset = instance->parent->file_offset + instance->parent->keys[instance->frame_num-1].offset;
				} else {
					instance->data = instance->parent->data + instance->parent->keys[instance->frame_num-1].offset;
				}
			}

			ubyte *temp = NULL;
			int temp_file_offset = -1;			

			// if we're using bitmap polys
			BM_SELECT_TEX_FORMAT();

			if ( anim_instance_is_streamed(instance) ) {
				if ( instance->xlate_pal ){
					temp_file_offset = unpack_frame_from_file(instance, instance->frame, instance->parent->width*instance->parent->height, instance->parent->palette_translation, aabitmap, bpp);
				} else {
					temp_file_offset = unpack_frame_from_file(instance, instance->frame, instance->parent->width*instance->parent->height, NULL, aabitmap, bpp);
				}
			} else {
				if ( instance->xlate_pal ){
					temp = unpack_frame(instance, instance->data, instance->frame, instance->parent->width*instance->parent->height, instance->parent->palette_translation, aabitmap, bpp);
				} else {
					temp = unpack_frame(instance, instance->data, instance->frame, instance->parent->width*instance->parent->height, NULL, aabitmap, bpp);
				}
			}

			// always go back to screen format
			BM_SELECT_SCREEN_FORMAT();

			// see if we had an error during decode (corrupted anim stream)
			if ( (temp == NULL) && (temp_file_offset < 0) ) {
				mprintf(("ANI: Fatal ERROR at frame %i!!  Aborting playback of \"%s\"...\n", instance->frame_num, instance->parent->name));

				// return -1 to end all playing of this anim instanc
				return -1;
			}

			if(instance->direction == ANIM_DIRECT_FORWARD){
				if ( anim_instance_is_streamed(instance) ) {
					instance->file_offset = temp_file_offset;
				} else {
					instance->data = temp;
				}
			}			
		}
		t2 = timer_get_fixed_seconds();
	}
	else {
		t2=t1=0;
	}

	// this only happens when the anim is being looped, we need to reset the last_frame_num
	if ( (instance->time_elapsed == 0) && (instance->looped) ) {
		instance->last_frame_num = -1;
		instance->frame_num = -1;
		instance->data = instance->parent->data;
		instance->file_offset = instance->parent->file_offset;
		instance->loop_count++;
	}
		
	decompress_time = f2fl(t2-t1);

	t1 = timer_get_fixed_seconds();
	if ( frame_diff == 0 && instance->last_bitmap != -1 ) {
		bitmap_id = instance->last_bitmap;
	}
	else {
		if ( instance->last_bitmap != -1 ){
			bm_release(instance->last_bitmap);
		}
		bitmap_id = bm_create(bpp, instance->parent->width, instance->parent->height, instance->frame, bitmap_flags);
	}
	
	if ( bitmap_id == -1 ) {
		// anim has finsished playing, free the instance frame data
		anim_release_render_instance(instance);	
		return -1;

		// NOTE: there is no need to free the instance, since it was pre-allocated as 
		//       part of the anim_free_list
	}
	else {
		gr_set_bitmap(bitmap_id);
		
		// determine x,y to display the bitmap at
		if ( instance->world_pos == NULL ) {
			gr_set_screen_scale(instance->base_w, instance->base_h);
			gr_set_clip(0, 0, instance->base_w, instance->base_h, false);
			if ( instance->aa_color == NULL ) {
				gr_bitmap(instance->x, instance->y);
			}
			else {
				gr_set_color_fast( (color*)instance->aa_color );
				gr_aabitmap(instance->x, instance->y);
			}
			gr_reset_screen_scale();
			gr_reset_clip();
		}
		else {
			g3_rotate_vertex(&image_vertex,instance->world_pos);
			Assert(instance->radius != 0.0f);
			g3_draw_bitmap(&image_vertex, 0, instance->radius*1.5f, TMAP_FLAG_TEXTURED | TMAP_HTL_2D);
		}
									  
		instance->last_bitmap = bitmap_id;
	}

	t2 = timer_get_fixed_seconds();
	render_time = f2fl(t2-t1);

	return 0;
}

/**
 * @brief Stop an anim instance that is on the anim_render_list from playing
 */
int anim_stop_playing(anim_instance* instance)
{
	Assert(instance != NULL);

	if ( anim_playing(instance) ) {
		anim_release_render_instance(instance);
	}
	return 0;
}

/**
 * @brief Free a particular animation instance that is on the anim_render_list.  
 * Do not call this function to free an animation instance in general (use 
 * free_anim_instance() for that), only when you want to free an instance
 * that is on the anim_render_list
 */
void anim_release_render_instance(anim_instance* instance)
{
	Assert( instance != NULL );

	if (instance->frame != NULL)
		vm_free(instance->frame);

	instance->frame = NULL;
	instance->parent->instance_count--;

	if ( instance->last_bitmap != -1 ) {
		bm_release(instance->last_bitmap); 
		instance->last_bitmap = -1;
	}

	// remove instance from anim_render_list
	list_remove( &anim_render_list, instance );

	// insert instance into the anim_free_list
	list_append( &anim_free_list, instance );
}

/**
 * @brief Free all anim instances that are on the anim_render_list.
 *
 * @param screen_id	Optional parameter that lets you only free a subset of the anim instances.	
 * A screen_id of 0 is the default value, and this is used for animations that always play when 
 * they are placed on the aim_render_list.
 */
void anim_release_all_instances(int screen_id)
{
	anim_instance* A;
	anim_instance* temp;

	if ( Anim_inited == FALSE )
		return;

	A = GET_FIRST(&anim_render_list);
	while( A !=END_OF_LIST(&anim_render_list) )	{
		temp = GET_NEXT(A);
		if ( A->screen_id == screen_id || screen_id == 0 ) {
			anim_release_render_instance(A);
		}
		A = temp;
	}
}

// -----------------------------------------------------------------------------
//	anim_read_header()
//
// Read the header of a .ani file.  Below is the format of a .ani header
//
//	#bytes		|	description
//	2			|	obsolete, kept for compatibility with old versions
//	2			|	version number
//	2			|	fps
//	1			|	transparent red value
//  1			|	transparent green value
//	1			|	transparent blue value
//	2			|	width
//	2			|	height
//	2			|	number of frames
//	1			|	packer code
//	763			|	palette
//	2			|	number of key frames
//	2			|	key frame number	}		repeats
//	4			|	key frame offset	}		repeats
//	4			|	compressed data length
//
void anim_read_header(anim *ptr, CFILE *fp)
{
	ptr->width = cfread_short(fp);
	// If first 2 bytes are zero, this means we are using a new format, which includes
	// a version, and fps values. This is only done since a version number was not included
	// in the original header.

	// default
	Color_xparent.red = 0;
	Color_xparent.green = 255;
	Color_xparent.blue = 0;

	if ( ptr->width == 0 ) {
		ptr->version = cfread_short(fp);
		ptr->fps = cfread_short(fp);

		// version 2 added a custom transparency color
		if ( ptr->version >= 2 ) {
			cfread(&Color_xparent.red, 1, 1, fp);
			cfread(&Color_xparent.green, 1, 1, fp);
			cfread(&Color_xparent.blue, 1, 1, fp);
		}

		ptr->width = cfread_short(fp);
	}
	else {
		ptr->version = 0;
		ptr->fps = 30;
	}
	
	ptr->height = cfread_short(fp);

#ifndef NDEBUG
	// get size of ani compared to power of 2
	int r, floor_pow;
	r = ptr->height;
	floor_pow = 0;
	
	while(r >= 2) {
		r /= 2;
		floor_pow++;
	}

	int floor_size = (int) pow(2.0, floor_pow);
	int diff = ptr->height - floor_size;
	float waste = 100.0f * float((floor_size - diff))/(2.0f *(float)floor_size);

	if (diff != 0) {
		if (ptr->height > 16) {
			mprintf(("ANI %s with size %dx%d (%.1f%% wasted)\n", ptr->name, ptr->width, ptr->height, waste));
		}
	}
#endif

	ptr->total_frames = cfread_short(fp);
	ptr->packer_code = cfread_ubyte(fp);
	cfread(&ptr->palette, 256, 3, fp);
	ptr->num_keys = cfread_short(fp);

	// store xparent colors
	ptr->xparent_r = Color_xparent.red;
	ptr->xparent_g = Color_xparent.green;
	ptr->xparent_b = Color_xparent.blue;

	if(ptr->total_frames == ptr->num_keys){
		ptr->flags |= ANF_ALL_KEYFRAMES;
	}
}

/**
 * @brief Load an animation.  This stores the compressed data, which instances of the animation can reference.  
 * Must be free'ed later with anim_free().
 * 
 * @param real_filename Filename of animation
 * @param cf_dir_type 
 * @param file_mapped Whether to use memory-mapped file or not.
 * 
 * @details Memory-mapped files will page in the animation from disk as it is needed, but performance is not as good.
 * @return Pointer to anim that is loaded if sucess, NULL if failure.
 */
anim *anim_load(char *real_filename, int cf_dir_type, int file_mapped)
{
	anim			*ptr;
	CFILE			*fp;
	int			count,idx;
	char name[_MAX_PATH];

	Assert( real_filename != NULL );

	strcpy_s( name, real_filename );
	char *p = strchr( name, '.' );
	if ( p ) {
		*p = 0;
	}
	strcat_s( name, ".ani" );

	ptr = first_anim;
	while (ptr) {
		if (!stricmp(name, ptr->name))
			break;

		ptr = ptr->next;
	}

	if (!ptr) {
		fp = cfopen(name, "rb", CFILE_NORMAL, cf_dir_type);
		if ( !fp )
			return NULL;

		ptr = (anim *) vm_malloc(sizeof(anim));
		Assert(ptr);

		ptr->flags = 0;
		ptr->next = first_anim;
		first_anim = ptr;
		Assert(strlen(name) < _MAX_PATH - 1);
		strcpy_s(ptr->name, name);
		ptr->instance_count = 0;
		ptr->width = 0;
		ptr->height = 0;
		ptr->total_frames = 0;
		ptr->keys = NULL;
		ptr->ref_count=0;

		anim_read_header(ptr, fp);

		if(ptr->num_keys > 0){
			ptr->keys = (key_frame*)vm_malloc(sizeof(key_frame) * ptr->num_keys);
			Assert(ptr->keys != NULL);
		} 			

		// store how long the anim should take on playback (in seconds)
		ptr->time = i2fl(ptr->total_frames)/ptr->fps;

		for(idx=0;idx<ptr->num_keys;idx++){
			ptr->keys[idx].frame_num = 0;
			cfread(&ptr->keys[idx].frame_num, 2, 1, fp);
			cfread(&ptr->keys[idx].offset, 4, 1, fp);
			ptr->keys[idx].frame_num = INTEL_INT( ptr->keys[idx].frame_num ); //-V570
			ptr->keys[idx].offset = INTEL_INT( ptr->keys[idx].offset ); //-V570
		}

		cfread(&count, 4, 1, fp);	// size of compressed data
		count = INTEL_INT( count );

		ptr->cfile_ptr = NULL;

		if ( file_mapped == PAGE_FROM_MEM) {
			// Try mapping the file to memory 
			ptr->flags |= ANF_MEM_MAPPED;
			ptr->cfile_ptr = cfopen(name, "rb", CFILE_MEMORY_MAPPED, cf_dir_type);
		}

		// couldn't memory-map file... must be in a packfile, so stream manually
		if ( file_mapped && !ptr->cfile_ptr ) {
			ptr->flags &= ~ANF_MEM_MAPPED;
			ptr->flags |= ANF_STREAMED;
			ptr->cfile_ptr = cfopen(name, "rb", CFILE_NORMAL, cf_dir_type);
		}

		ptr->cache = NULL;

		// If it opened properly as mem-mapped (or streamed)
		if (ptr->cfile_ptr != NULL)	{
			// VERY IMPORTANT STEP
			// Set the data pointer to the compressed data (which is not at the start of the
			// file).  Use ftell() to find out how far we've already parsed into the file
			//
			int offset;
			offset = cftell(fp);
			ptr->file_offset = offset;
			if ( ptr->flags & ANF_STREAMED ) {
				ptr->data = NULL;
				ptr->cache_file_offset = ptr->file_offset;
				ptr->cache = (ubyte*)vm_malloc(ANI_STREAM_CACHE_SIZE+2);
				Assert(ptr->cache);
				cfseek(ptr->cfile_ptr, offset, CF_SEEK_SET);
				cfread(ptr->cache, ANI_STREAM_CACHE_SIZE, 1, ptr->cfile_ptr);
			} else {
				ptr->data = (ubyte*)cf_returndata(ptr->cfile_ptr) + offset;
			}
		} else {
			// Not a memory mapped file (or streamed)
			ptr->flags &= ~ANF_MEM_MAPPED;
			ptr->flags &= ~ANF_STREAMED;
			ptr->data = (ubyte *) vm_malloc(count);
			ptr->file_offset = -1;
			cfread(ptr->data, count, 1, fp);
		}

		cfclose(fp);

		// store screen signature, so we can tell if palette changes
		ptr->screen_sig = gr_screen.signature;

		anim_set_palette(ptr);
	}

	ptr->ref_count++;
	return ptr;
}

/**
 * @brief Free an animation that was loaded with anim_load().  
 * @details All instances referencing this animation must be free'ed or get an assert.
 */
int anim_free(anim *ptr)
{
	Assert ( ptr != NULL );
	anim *list, **prev_anim;

	list = first_anim;
	prev_anim = &first_anim;
	while (list && (list != ptr)) {
		prev_anim = &list->next;
		list = list->next;
	}

	if ( !list )
		return -2;

	// only free when ref_count is 0
	ptr->ref_count--;
	if ( ptr->ref_count > 0 ) 
		return -1;

	// only free if there are no playing instances
	if ( ptr->instance_count > 0 )
		return -1;

	if(ptr->keys != NULL){
		vm_free(ptr->keys);
		ptr->keys = NULL;
	}

	if ( ptr->flags & (ANF_MEM_MAPPED|ANF_STREAMED) ) {
		cfclose(ptr->cfile_ptr);
		if (ptr->cache != NULL) {
			vm_free(ptr->cache);
			ptr->cache = NULL;
		}
	}
	else {
		Assert(ptr->data);
		if (ptr->data != NULL) {
			vm_free(ptr->data);
			ptr->data = NULL;
		}
	}

	*prev_anim = ptr->next;
	vm_free(ptr);
	return 0;
}


/**
 * @brief Return if an anim is playing or not.  
 */
int anim_playing(anim_instance *ai)
{
	Assert(ai != NULL);
	if ( ai->frame == NULL )
		return 0;
	else 
		return 1;
}


/**
 * @brief Called at the beginning of a mission to initialize any mission dependent anim data.
 * @todo Redundant?
 */
void anim_level_init()
{
}

/**
 * @brief Called after the end of a mission to clean up any mission dependent anim data. 
 */
void anim_level_close()
{
	anim_release_all_instances();
}

/**
 * @brief Write the frames of a .ani file out to disk as .pcx files.
 * @details Use naming convention: filename0000.pcx, filename0001.pcx etc.
 *
 * @return If success 0, or if failed -1
 */
int anim_write_frames_out(char *filename)
{
	anim				*source_anim;
	anim_instance	*ai;
	char				root_name[256], pcxname[256];
	char				buf[64];
	int				i,j;
	ubyte				**row_data;

	strcpy_s(root_name, filename);
	root_name[strlen(filename)-4] = 0;

	source_anim = anim_load(filename);
	if ( source_anim == NULL ) 
		return -1;

	ai = init_anim_instance(source_anim, 16);

	row_data = (ubyte**)vm_malloc((source_anim->height+1) * 4);

	for ( i = 0; i < source_anim->total_frames; i++ ) {
		anim_get_next_raw_buffer(ai, 0, 0, 16);
		strcpy_s(pcxname, root_name);
		sprintf(buf,"%04d",i);
		strcat_s(pcxname, buf);

		for ( j = 0; j < source_anim->height; j++ ) {
			row_data[j] = &ai->frame[j*source_anim->width];
		}


		pcx_write_bitmap( pcxname,
								source_anim->width,
								source_anim->height,
								row_data,
								source_anim->palette);

		printf(".");

	}
	printf("\n");
	vm_free(row_data);
	return 0;
}

/**
 * @brief Display information and statistics about a .ani file.
 * @details This is called when -i switch is on when running ac.exe
 */
void anim_display_info(char *real_filename)
{
	CFILE				*fp;
	anim				A;
	float				percent;
	int				i, uncompressed, compressed, *key_frame_nums=NULL, tmp;
	char filename[MAX_FILENAME_LEN];

	strcpy_s( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) {
		*p = 0;
	}
	strcat_s( filename, ".ani" );

	fp = cfopen(filename, "rb");
	if ( !fp ) {
		printf("Fatal error opening %s", filename);
		return;
	}

	anim_read_header(&A, fp);
	// read the keyframe frame nums and offsets
	key_frame_nums = (int*)vm_malloc(sizeof(int)*A.num_keys);
	Assert(key_frame_nums != NULL);
	if (key_frame_nums == NULL)
		return;

	for ( i = 0; i < A.num_keys; i++ ) {
		key_frame_nums[i] = 0;
		cfread(&key_frame_nums[i], 2, 1, fp);
		cfread(&tmp, 4, 1, fp);
	}

	cfread(&compressed, 4, 1, fp);

	uncompressed = A.width * A.height * A.total_frames;	// 8 bits per pixel
	percent = i2fl(compressed) / uncompressed * 100.0f;

	printf("%% of uncompressed size:    %.0f%%\n", percent);
	printf("Width:                     %d\n", A.width);
	printf("Height:                    %d\n", A.height);
	printf("Total Frames:              %d\n", A.total_frames);

#ifndef NDEBUG
	printf("Key Frames:                %d\n", A.num_keys);
	if ( A.num_keys > 1 && (A.total_frames != A.num_keys) ) {
		printf("key list: (");
		for ( i = 0; i < A.num_keys; i++ ) {
			if ( i < A.num_keys-1 ) 
				printf("%d, ", key_frame_nums[i]);
			else
				printf("%d)\n", key_frame_nums[i]);
		}
	}
#endif

	printf("FPS:                       %d\n", A.fps);

#ifndef NDEBUG
	printf("Transparent RGB:           (%u,%u,%u)\n", A.xparent_r, A.xparent_g, A.xparent_b); 
#endif

	printf("ac version:                %d\n", A.version);

	if ( key_frame_nums != NULL ) {
		vm_free(key_frame_nums);
	}

	cfclose(fp);
}

void anim_reverse_direction(anim_instance *ai)
{
	int temp;

	// you're not allowed to call anim_reverse_direction(...) unless every frame is a keyframe!!!!
	// The God of Delta-RLE demands it be thus.
	Assertion( ai->parent->flags & ANF_ALL_KEYFRAMES, "Ani was set to play backwards. In order to enable this, all frames of the animation MUST be keyframes.");
		
	// flip the animation direction
	if(ai->direction == ANIM_DIRECT_FORWARD){
		ai->direction = ANIM_DIRECT_REVERSE;				
	} else if(ai->direction == ANIM_DIRECT_REVERSE){
		ai->direction = ANIM_DIRECT_FORWARD;
	}

	// flip frame_num and last_frame_num
	temp = ai->frame_num;
	ai->frame_num = ai->last_frame_num;
	ai->last_frame_num = temp;

	// flip the start and stop at frames
	temp = ai->stop_at;
	ai->stop_at = ai->start_at;
	ai->start_at = temp;

	// make sure to sync up the time correctly 
	if(ai->direction == ANIM_DIRECT_FORWARD){
		ai->time_elapsed = ((float)ai->frame_num - (float)ai->start_at - 1.0f) / (float)ai->parent->fps;	
	} else if(ai->direction == ANIM_DIRECT_REVERSE) {
		ai->time_elapsed = ((float)ai->start_at - (float)ai->frame_num - 1.0f) / (float)ai->parent->fps;	
	}	
}

void anim_pause(anim_instance *ai)
{
	ai->paused = 1;
}

void anim_unpause(anim_instance *ai)
{
	ai->paused = 0;
}

void anim_ignore_next_frametime()
{
	Anim_ignore_frametime=1;
}

int anim_instance_is_streamed(anim_instance *ai)
{
	Assert(ai);
	return ( ai->parent->flags & ANF_STREAMED );
}

unsigned char anim_instance_get_byte(anim_instance *ai, int offset)
{
	int absolute_offset;
	anim *parent;
	
	Assert(ai);
	Assert(ai->parent->cfile_ptr);
	Assert(ai->parent->flags & ANF_STREAMED);

	parent = ai->parent;
	absolute_offset = ai->file_offset + offset;

	// maybe in cache?
	int cache_offset;
	cache_offset = absolute_offset - parent->cache_file_offset;
	if ( (cache_offset >= 0) && (cache_offset < ANI_STREAM_CACHE_SIZE) ) {
		return parent->cache[cache_offset];
	} else {
		// fill cache
		cfseek(parent->cfile_ptr, absolute_offset, CF_SEEK_SET);
		cfread(parent->cache, ANI_STREAM_CACHE_SIZE, 1, parent->cfile_ptr);
		parent->cache_file_offset = absolute_offset;
		return parent->cache[0];
	}
}

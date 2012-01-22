#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "bmpman/bmpman.h"
#include "graphics/generic.h"
#include "graphics/2d.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"
#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"
#ifdef _WIN32
#include <windows.h>	// for MAX_PATH
#else
#define MAX_PATH	255
#endif
//#define TIMER
#ifdef TIMER
#include "io/timer.h"
#endif
 
//we check background type to avoid messed up colours for ANI
#define ANI_BPP_CHECK		(ga->ani.bg_type == BM_TYPE_PCX) ? 16 : 32

// Goober5000
int generic_anim_init_and_stream(generic_anim *anim, char *anim_filename, ubyte bg_type, bool attempt_hi_res)
{
	int stream_result = -1;
	char filename[NAME_LENGTH];
	char *p;

	Assert(anim != NULL);
	Assert(anim_filename != NULL);

	// hi-res support
	if (attempt_hi_res && (gr_screen.res == GR_1024)) {
		// attempt to load a hi-res animation
		memset(filename, 0, NAME_LENGTH);
		strcpy_s(filename, "2_");
		strncat(filename, anim_filename, NAME_LENGTH - 3);

		// remove extension
		p = strchr(filename, '.');
		if(p) {
			*p = '\0';
		}

		// attempt to stream the hi-res ani
		generic_anim_init(anim, filename);
		anim->ani.bg_type = bg_type;
		stream_result = generic_anim_stream(anim);
	}

	// we failed to stream hi-res, or we aren't running in hi-res, so try low-res
	if (stream_result < 0) {
		strcpy_s(filename, anim_filename);

		// remove extension
		p = strchr(filename, '.');
		if(p) {
			*p = '\0';
		}

		// attempt to stream the low-res ani
		generic_anim_init(anim, filename);
		anim->ani.bg_type = bg_type;
		stream_result = generic_anim_stream(anim);
	}

	return stream_result;
}

// Goober5000
void generic_anim_init(generic_anim *ga, char *filename)
{
	//memset(ga, 0, sizeof(ga));	//this makes the mission load screen crash :(
	if (filename == NULL) {
		ga->filename[0] = '\0';
	} else {
		strncpy(ga->filename, filename, MAX_FILENAME_LEN - 1);
	}

	ga->first_frame = -1;
	ga->num_frames = 0;
	ga->keyframe = 0;
	ga->keyoffset = 0;
	ga->current_frame = 0;
	ga->previous_frame = -1;
	ga->direction = GENERIC_ANIM_DIRECTION_FORWARDS;
	ga->done_playing = 0;
	ga->total_time = 0.0f;
	ga->anim_time = 0.0f;

	//we only care about the stuff below if we're streaming
	ga->ani.animation = NULL;
	ga->ani.instance = NULL;
	ga->ani.bg_type = 0;
	ga->type = BM_TYPE_NONE;
	ga->streaming = 0;
	ga->buffer = NULL;
	ga->height = 0;
	ga->width = 0;
	ga->bitmap_id = -1;
}
/**
 * CommanderDJ - same as generic_anim_init, just with an SCP_string 
 */
void generic_anim_init(generic_anim *ga, const SCP_string& filename)
{
	generic_anim_init(ga, const_cast<char*> (filename.c_str()));
}

// Goober5000
void generic_bitmap_init(generic_bitmap *gb, char *filename)
{
	if (filename == NULL) {
		gb->filename[0] = '\0';
	} else {
		strncpy(gb->filename, filename, MAX_FILENAME_LEN - 1);
	}

	gb->bitmap_id = -1;
}

// Goober5000
// load a generic_anim
// return 0 is successful, otherwise return -1
int generic_anim_load(generic_anim *ga)
{
	int fps;

	if ( !VALID_FNAME(ga->filename) )
		return -1;

	ga->first_frame = bm_load_animation(ga->filename, &ga->num_frames, &fps, &ga->keyframe);
	//mprintf(("generic_anim_load: %s - keyframe = %d\n", ga->filename, ga->keyframe));

	if (ga->first_frame < 0)
		return -1;

	Assert(fps != 0);
	ga->total_time = ga->num_frames / (float)fps;
	ga->done_playing = 0;
	ga->anim_time = 0.0f;

	return 0;
}

int generic_anim_stream(generic_anim *ga)
{
	CFILE *img_cfp = NULL;
	int anim_fps = 0;
	char full_path[MAX_PATH];
	int size = 0, offset = 0;
	const int NUM_TYPES = 2;
	const ubyte type_list[NUM_TYPES] = {BM_TYPE_EFF, BM_TYPE_ANI};
	const char *ext_list[NUM_TYPES] = {".eff", ".ani"};
	int rval = -1;
	int bpp;

	ga->type = BM_TYPE_NONE;

	rval = cf_find_file_location_ext(ga->filename, NUM_TYPES, ext_list, CF_TYPE_ANY, sizeof(full_path) - 1, full_path, &size, &offset, 0);

	// could not be found, or is invalid for some reason
	if ( (rval < 0) || (rval >= NUM_TYPES) )
		return -1;

	//make sure we can open it
	img_cfp = cfopen_special(full_path, "rb", size, offset, CF_TYPE_ANY);

	if (img_cfp == NULL) {
		return -1;
	}

	strcat_s(ga->filename, ext_list[rval]);
	ga->type = type_list[rval];
	//seek to the end
	cfseek(img_cfp, 0, CF_SEEK_END);

	cfclose(img_cfp);

	//TODO: add streaming EFF
	if(ga->type == BM_TYPE_ANI) {
		bpp = ANI_BPP_CHECK;
		ga->ani.animation = anim_load(ga->filename, CF_TYPE_ANY, 0);
		ga->ani.instance = init_anim_instance(ga->ani.animation, bpp);

	#ifndef NDEBUG
		// for debug of ANI sizes
		strcpy_s(ga->ani.animation->name, ga->filename);
	#endif

		ga->num_frames = ga->ani.animation->total_frames;
		anim_fps = ga->ani.animation->fps;
		ga->height = ga->ani.animation->height;
		ga->width = ga->ani.animation->width;
		ga->buffer = ga->ani.instance->frame;
		ga->bitmap_id = bm_create(bpp, ga->width, ga->height, ga->buffer, 0);
		ga->ani.instance->last_bitmap = -1;

		ga->ani.instance->file_offset = ga->ani.animation->file_offset;
		ga->ani.instance->data = ga->ani.animation->data;

		ga->previous_frame = -1;
	}
	else {
		//placeholder until we get eff streaming working
		return generic_anim_load(ga);
	}

	// keyframe info
	if (ga->type == BM_TYPE_ANI) {
		//we only care if there are 2 keyframes - first frame, other frame to jump to for ship/weapons
		//mainhall door anis hav every frame as keyframe, so we don't care
		//other anis only have the first frame
		if(ga->ani.animation->num_keys == 2) {
			int key1 = ga->ani.animation->keys[0].frame_num;
			int key2 = ga->ani.animation->keys[1].frame_num;

			if (key1 < 0 || key1 >= ga->num_frames) key1 = -1;
			if (key2 < 0 || key2 >= ga->num_frames) key2 = -1;

			// some retail anis have their keyframes reversed
			// and some have their keyframes out of bounds
			if (key1 >= 0 && key1 >= key2) {
				ga->keyframe = ga->ani.animation->keys[0].frame_num;
				ga->keyoffset = ga->ani.animation->keys[0].offset;
			}
			else if (key2 >= 0 && key2 >= key1) {
				ga->keyframe = ga->ani.animation->keys[1].frame_num;
				ga->keyoffset = ga->ani.animation->keys[1].offset;
			}
		}
	}
	else {
		// EFF not handled yet; code shouldn't get here anyway per above block
	}

	ga->streaming = 1;

	Assert(anim_fps != 0);
	ga->total_time = ga->num_frames / (float) anim_fps;
	ga->done_playing = 0;
	ga->anim_time = 0.0f;

	return 0;
}

int generic_bitmap_load(generic_bitmap *gb)
{
	if ( !VALID_FNAME(gb->filename) )
		return -1;

	gb->bitmap_id = bm_load(gb->filename);

	if (gb->bitmap_id < 0)
		return -1;

	return 0;
}

void generic_anim_unload(generic_anim *ga)
{
	if(ga->num_frames > 0) {
		if(ga->streaming) {
			if(ga->type == BM_TYPE_ANI) {
				anim_free(ga->ani.animation);
				free_anim_instance(ga->ani.instance);
			}
		}
		else {
			//trying to release the first frame will release ALL frames
			bm_release(ga->first_frame);
		}
		if(ga->buffer) {
			bm_release(ga->bitmap_id);
		}
	}
	generic_anim_init(ga, NULL);
}

//for timer debug, #define TIMER
void generic_render_ani_stream(generic_anim *ga)
{
	int i;
	int bpp = ANI_BPP_CHECK;

	#ifdef TIMER
		int start_time = timer_get_fixed_seconds();
	#endif

	if(ga->current_frame == ga->previous_frame)
		return;

	#ifdef TIMER
		mprintf(("=========================\n"));
		mprintf(("frame: %d\n", ga->current_frame));
	#endif

	anim_check_for_palette_change(ga->ani.instance);
	// if we're using bitmap polys
	BM_SELECT_TEX_FORMAT();
	if(ga->direction & GENERIC_ANIM_DIRECTION_BACKWARDS) {
		//grab the keyframe - every frame is a keyframe for ANI
		if(ga->ani.animation->flags & ANF_STREAMED) {
			ga->ani.instance->file_offset = ga->ani.animation->file_offset + ga->ani.animation->keys[ga->current_frame].offset;
		} else {
			ga->ani.instance->data = ga->ani.animation->data + ga->ani.animation->keys[ga->current_frame].offset;
		}
		if(ga->ani.animation->flags & ANF_STREAMED) {
			ga->ani.instance->file_offset = unpack_frame_from_file(ga->ani.instance, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, 0, bpp);
		}
		else {
			ga->ani.instance->data = unpack_frame(ga->ani.instance, ga->ani.instance->data, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, 0, bpp);
		}
	}
	else {
		//looping back
		if((ga->current_frame == 0) || (ga->current_frame < ga->previous_frame)) {
			//go back to keyframe if there is one
			if(ga->keyframe && (ga->current_frame > 0)) {
				if(ga->ani.animation->flags & ANF_STREAMED) {
					ga->ani.instance->file_offset = ga->ani.animation->file_offset + ga->keyoffset;
				} else {
					ga->ani.instance->data = ga->ani.animation->data + ga->keyoffset;
				}
				ga->previous_frame = ga->keyframe - 1;
			}
			//go back to the start
			else {
				ga->ani.instance->file_offset = ga->ani.animation->file_offset;
				ga->ani.instance->data = ga->ani.animation->data;
				ga->previous_frame = -1;
			}
		}
		#ifdef TIMER
				mprintf(("proc: %d\n", timer_get_fixed_seconds() - start_time));
				mprintf(("previous frame: %d\n", ga->previous_frame));
		#endif
		for(i = ga->previous_frame + 1; i <= ga->current_frame; i++) {
			if(ga->ani.animation->flags & ANF_STREAMED) {
				ga->ani.instance->file_offset = unpack_frame_from_file(ga->ani.instance, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, 0, bpp);
			}
			else {
				ga->ani.instance->data = unpack_frame(ga->ani.instance, ga->ani.instance->data, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, 0, bpp);
			}
		}
	}
	// always go back to screen format
	BM_SELECT_SCREEN_FORMAT();
	//we need to use this because performance is worse if we flush the gfx card buffer
	if ( ga->ani.instance->last_bitmap != -1 ){
		bm_release(ga->ani.instance->last_bitmap);
	}
	ga->bitmap_id = bm_create(bpp, ga->width, ga->height, ga->buffer, 0);

	//in case we want to check that the frame is actually changing
	//mprintf(("frame crc = %08X\n", cf_add_chksum_long(0, ga->buffer, ga->width * ga->height * (bpp >> 3))));
	ga->ani.instance->last_bitmap = ga->bitmap_id;

	#ifdef TIMER
		mprintf(("end: %d\n", timer_get_fixed_seconds() - start_time));
		mprintf(("=========================\n"));
	#endif
}

void generic_anim_render(generic_anim *ga, float frametime, int x, int y)
{
	float keytime = 0.0;

	if(ga->keyframe)
		keytime = (ga->total_time * ((float)ga->keyframe / (float)ga->num_frames));
	//don't mess with the frame time if we're paused
	if((ga->direction & GENERIC_ANIM_DIRECTION_PAUSED) == 0) {
		if(ga->direction & GENERIC_ANIM_DIRECTION_BACKWARDS) {
			//keep going forwards if we're in a keyframe loop
			if(ga->keyframe && (ga->anim_time >= keytime)) {
				ga->anim_time += frametime;
				if(ga->anim_time >= ga->total_time) {
					ga->anim_time = keytime - 0.001f;
					ga->done_playing = 0;
				}
			}
			else {
				//playing backwards
				ga->anim_time -= frametime;
				if((ga->direction & GENERIC_ANIM_DIRECTION_NOLOOP) && ga->anim_time <= 0.0) {
					ga->anim_time = 0;		//stop on first frame when playing in reverse
				}
				else {
					while(ga->anim_time <= 0.0)
						ga->anim_time += ga->total_time;	//make sure we're always positive, so we can go back to the end
				}
			}
		}
		else {
			ga->anim_time += frametime;
			if(ga->anim_time >= ga->total_time) {
				if(ga->direction & GENERIC_ANIM_DIRECTION_NOLOOP) {
					ga->anim_time = ga->total_time - 0.001f;		//stop on last frame when playing - if it's equal we jump to the first frame
				}
				else if(!ga->done_playing){
					//we've played this at least once
					ga->done_playing = 1;
				}
			}
		}
	}
	if(ga->num_frames > 0)
	{
		ga->current_frame = 0;
		if(ga->done_playing && ga->keyframe) {
			ga->anim_time = fmod(ga->anim_time - keytime, ga->total_time - keytime) + keytime;
		}
		else {
			ga->anim_time = fmod(ga->anim_time, ga->total_time);
		}
		ga->current_frame += fl2i(ga->anim_time * ga->num_frames / ga->total_time);
		//sanity check
		CLAMP(ga->current_frame, 0, ga->num_frames - 1);
		if(ga->streaming) {
			//handle streaming - render one frame
			//TODO: add EFF streaming
			if(ga->type == BM_TYPE_ANI) {
				generic_render_ani_stream(ga);
			}
			gr_set_bitmap(ga->bitmap_id);
		}
		else {
			gr_set_bitmap(ga->first_frame + ga->current_frame);
		}
		ga->previous_frame = ga->current_frame;
		gr_bitmap(x, y);
	}
}

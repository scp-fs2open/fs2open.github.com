#include "anim/packunpack.h"
#include "globalincs/globals.h"
#include "graphics/2d.h"
#include "graphics/generic.h"
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

// These two functions find if a bitmap or animation exists by filename, no extension needed.
bool generic_bitmap_exists(const char *filename)
{
	return cf_exists_full_ext(filename, CF_TYPE_ANY, BM_NUM_TYPES, bm_ext_list) != 0;
}

bool generic_anim_exists(const char *filename)
{
	return cf_exists_full_ext(filename, CF_TYPE_ANY, BM_ANI_NUM_TYPES, bm_ani_ext_list) != 0;
}

// Goober5000
int generic_anim_init_and_stream(generic_anim *ga, const char *anim_filename, BM_TYPE bg_type, bool attempt_hi_res)
{
	int stream_result = -1;
	char filename[NAME_LENGTH];
	char *p;

	Assert(ga != NULL);
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
		generic_anim_init(ga, filename);
		ga->ani.bg_type = bg_type;
		stream_result = generic_anim_stream(ga);
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
		generic_anim_init(ga, filename);
		ga->ani.bg_type = bg_type;
		stream_result = generic_anim_stream(ga);
	}

	return stream_result;
}

// Goober5000
void generic_anim_init(generic_anim *ga)
{
	generic_anim_init(ga, NULL);
}

// Goober5000
void generic_anim_init(generic_anim *ga, const char *filename)
{
	if (filename != NULL)
		strcpy_s(ga->filename, filename);
	else
		memset(ga->filename, 0, MAX_FILENAME_LEN);
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
	ga->ani.bg_type = BM_TYPE_NONE;
	ga->type = BM_TYPE_NONE;
	ga->streaming = 0;
	ga->buffer = NULL;
	ga->height = 0;
	ga->width = 0;
	ga->bitmap_id = -1;
	ga->use_hud_color = false;
}

// CommanderDJ - same as generic_anim_init, just with an SCP_string 
void generic_anim_init(generic_anim *ga, const SCP_string& filename)
{
	generic_anim_init(ga);
	filename.copy(ga->filename, MAX_FILENAME_LEN - 1);
}

// Goober5000
void generic_bitmap_init(generic_bitmap *gb, const char *filename)
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

	ga->first_frame = bm_load_animation(ga->filename, &ga->num_frames, &fps, &ga->keyframe, &ga->total_time);
	//mprintf(("generic_anim_load: %s - keyframe = %d\n", ga->filename, ga->keyframe));

	if (ga->first_frame < 0)
		return -1;

	ga->done_playing = 0;
	ga->anim_time = 0.0f;

	return 0;
}

int generic_anim_stream(generic_anim *ga, const bool cache)
{
	CFILE *img_cfp = NULL;
	int anim_fps = 0;
	int bpp;

	ga->type = BM_TYPE_NONE;

	auto res = cf_find_file_location_ext(ga->filename, BM_ANI_NUM_TYPES, bm_ani_ext_list, CF_TYPE_ANY);

	// could not be found, or is invalid for some reason
	if ( !res.found )
		return -1;

	//make sure we can open it
	img_cfp = cfopen_special(res, "rb", CF_TYPE_ANY);

	if (img_cfp == NULL) {
		return -1;
	}

	strcat_s(ga->filename, bm_ani_ext_list[res.extension_index]);
	ga->type = bm_ani_type_list[res.extension_index];
	//seek to the end
	cfseek(img_cfp, 0, CF_SEEK_END);

	cfclose(img_cfp);

	if(ga->type == BM_TYPE_ANI) {
		bpp = ANI_BPP_CHECK;
		if(ga->use_hud_color)
			bpp = 8;
		if (ga->ani.animation == nullptr) {
			ga->ani.animation = anim_load(ga->filename, CF_TYPE_ANY, 0);
		}
		if (ga->ani.instance == nullptr) {
			ga->ani.instance = init_anim_instance(ga->ani.animation, bpp);
		}

	#ifndef NDEBUG
		// for debug of ANI sizes
		strcpy_s(ga->ani.animation->name, ga->filename);
	#endif

		ga->num_frames = ga->ani.animation->total_frames;
		anim_fps = ga->ani.animation->fps;
		ga->height = ga->ani.animation->height;
		ga->width = ga->ani.animation->width;
		ga->buffer = ga->ani.instance->frame;
		ga->bitmap_id = bm_create(bpp, ga->width, ga->height, ga->buffer, (bpp==8)?BMP_AABITMAP:0);
		ga->ani.instance->last_bitmap = -1;

		ga->ani.instance->file_offset = ga->ani.animation->file_offset;
		ga->ani.instance->data = ga->ani.animation->data;

		ga->previous_frame = -1;
	}
	else if (ga->type == BM_TYPE_PNG) {
		if (ga->png.anim == nullptr) {
			try {
				ga->png.anim = new apng::apng_ani(ga->filename, cache);
			}
			catch (const apng::ApngException& e) {
				nprintf(("apng","Failed to load apng: %s\n", e.what() ));
				delete ga->png.anim;
				ga->png.anim = nullptr;
				ga->type = BM_TYPE_NONE;
				return -1;
			}
			nprintf(("apng", "apng read OK (%ix%i@%i) duration (%f)\n", ga->png.anim->w, ga->png.anim->h,
					ga->png.anim->bpp, ga->png.anim->anim_time));
		}
		ga->png.anim->goto_start();
		ga->current_frame = 0;
		ga->png.previous_frame_time = 0.0f;
		ga->num_frames = ga->png.anim->nframes;
		ga->height = ga->png.anim->h;
		ga->width = ga->png.anim->w;
		ga->previous_frame = -1;
		ga->buffer = ga->png.anim->frame.data.data();
		ga->bitmap_id = bm_create(ga->png.anim->bpp, ga->width, ga->height, ga->buffer, 0);
	}
	else {
		bpp = 32;
		if(ga->use_hud_color)
			bpp = 8;
		bm_load_and_parse_eff(ga->filename, CF_TYPE_ANY, &ga->num_frames, &anim_fps, &ga->keyframe, 0);
		char *p = strrchr( ga->filename, '.' );
		if ( p )
			*p = 0;
		char frame_name[MAX_FILENAME_LEN];
		if (snprintf(frame_name, MAX_FILENAME_LEN, "%s_0000", ga->filename) >= MAX_FILENAME_LEN) {
			// Make sure the string is null terminated
			frame_name[MAX_FILENAME_LEN - 1] = '\0';
		}
		ga->bitmap_id = bm_load(frame_name);
		if(ga->bitmap_id < 0) {
			mprintf(("Cannot find first frame for eff streaming. eff Filename: %s\n", ga->filename));
			return -1;
		}
		if (snprintf(frame_name, MAX_FILENAME_LEN, "%s_0001", ga->filename) >= MAX_FILENAME_LEN) {
			// Make sure the string is null terminated
			frame_name[MAX_FILENAME_LEN - 1] = '\0';
		}
		ga->eff.next_frame = bm_load(frame_name);
		bm_get_info(ga->bitmap_id, &ga->width, &ga->height);
		ga->previous_frame = 0;
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

	ga->streaming = 1;

	if (ga->type == BM_TYPE_PNG) {
		ga->total_time = ga->png.anim->anim_time;
	}
	else {
		if (anim_fps == 0) {
			Error(LOCATION, "animation (%s) has invalid fps of zero, fix this!", ga->filename);
		}
		ga->total_time = ga->num_frames / (float) anim_fps;
	}
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
				free_anim_instance(ga->ani.instance);
				anim_free(ga->ani.animation);
			}
			if(ga->type == BM_TYPE_EFF) {
				if(ga->eff.next_frame >= 0) 
					bm_release(ga->eff.next_frame);
				if(ga->bitmap_id >= 0)
					bm_release(ga->bitmap_id);
			}
			if(ga->type == BM_TYPE_PNG) {
				if(ga->bitmap_id >= 0)
					bm_release(ga->bitmap_id);
				if (ga->png.anim != nullptr) {
					delete ga->png.anim;
					ga->png.anim = nullptr;
				}
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
void generic_render_eff_stream(generic_anim *ga)
{
	if(ga->current_frame == ga->previous_frame)
		return;
	ubyte bpp = 32;
	if(ga->use_hud_color)
		bpp = 8;
	#ifdef TIMER
		int start_time = timer_get_fixed_seconds();
	#endif

	#ifdef TIMER
		mprintf(("=========================\n"));
		mprintf(("frame: %d\n", ga->current_frame));
	#endif
		char frame_name[MAX_FILENAME_LEN];
		if (snprintf(frame_name, MAX_FILENAME_LEN, "%s_%.4d", ga->filename, ga->current_frame) >= MAX_FILENAME_LEN) {
			// Make sure the string is null terminated
			frame_name[MAX_FILENAME_LEN - 1] = '\0';
		}
		if(bm_reload(ga->eff.next_frame, frame_name) == ga->eff.next_frame)
		{
			bitmap* next_frame_bmp = bm_lock(ga->eff.next_frame, bpp, (bpp==8)?BMP_AABITMAP:BMP_TEX_NONCOMP, true);
			if(next_frame_bmp->data)
				gr_update_texture(ga->bitmap_id, bpp, (ubyte*)next_frame_bmp->data, ga->width, ga->height);
			bm_unlock(ga->eff.next_frame);
			bm_unload(ga->eff.next_frame, 0, true);
			if (ga->current_frame == ga->num_frames-1)
			{
				if (snprintf(frame_name, MAX_FILENAME_LEN, "%s_0001", ga->filename) >= MAX_FILENAME_LEN) {
					// Make sure the string is null terminated
					frame_name[MAX_FILENAME_LEN - 1] = '\0';
				}
				bm_reload(ga->eff.next_frame, frame_name);
			}
		}
	#ifdef TIMER
		mprintf(("end: %d\n", timer_get_fixed_seconds() - start_time));
		mprintf(("=========================\n"));
	#endif
}

void generic_render_ani_stream(generic_anim *ga)
{
	int i;
	int bpp = ANI_BPP_CHECK;
	if(ga->use_hud_color)
		bpp = 8;
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
			ga->ani.instance->file_offset = unpack_frame_from_file(ga->ani.instance, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, (bpp==8)?1:0, bpp);
		}
		else {
			ga->ani.instance->data = unpack_frame(ga->ani.instance, ga->ani.instance->data, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, (bpp==8)?1:0, bpp);
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
				ga->ani.instance->file_offset = unpack_frame_from_file(ga->ani.instance, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, (bpp==8)?1:0, bpp);
			}
			else {
				ga->ani.instance->data = unpack_frame(ga->ani.instance, ga->ani.instance->data, ga->buffer, ga->width * ga->height, (ga->ani.instance->xlate_pal) ? ga->ani.animation->palette_translation : NULL, (bpp==8)?1:0, bpp);
			}
		}
	}
	// always go back to screen format
	BM_SELECT_SCREEN_FORMAT();
	//we need to use this because performance is worse if we flush the gfx card buffer
	
	gr_update_texture(ga->bitmap_id, bpp, ga->buffer, ga->width, ga->height);

	//in case we want to check that the frame is actually changing
	//mprintf(("frame crc = %08X\n", cf_add_chksum_long(0, ga->buffer, ga->width * ga->height * (bpp >> 3))));
	ga->ani.instance->last_bitmap = ga->bitmap_id;

	#ifdef TIMER
		mprintf(("end: %d\n", timer_get_fixed_seconds() - start_time));
		mprintf(("=========================\n"));
	#endif
}

/*
 * @brief apng specific animation rendering
 *
 * @param [in] ga  pointer to generic_anim struct
 */
void generic_render_png_stream(generic_anim* ga)
{
	if(ga->current_frame == ga->previous_frame) {
		return;
	}

	try {
		if ((ga->direction & GENERIC_ANIM_DIRECTION_BACKWARDS) && (ga->previous_frame != -1)) {
			// mainhall door anims start backwards to ensure they stay shut
			// in that case (i.e. previous_frame is -1) we actually want to call
			// next_frame, in order to retrieve the 1st frame of the animation
			ga->png.anim->prev_frame();
		}
		else {
			ga->png.anim->next_frame();
		}
	}
	catch (const apng::ApngException& e) {
		nprintf(("apng", "Unable to get next/prev apng frame: %s\n", e.what()));
		return;
	}

	bm_lock(ga->bitmap_id, ga->png.anim->bpp, BMP_TEX_NONCOMP, true);  // lock in 32 bpp for png
	int bpp = ga->png.anim->bpp;
	if (ga->use_hud_color) {
		bpp = 8;
	}
	gr_update_texture(ga->bitmap_id, bpp, ga->buffer, ga->width, ga->height);  // this will convert to 8 bpp if required
	bm_unlock(ga->bitmap_id);
}

/*
 * @brief calculate current frame for fixed frame delay animation formats (ani & eff)
 *
 * @param [in] *ga  animation data
 * @param [in] frametime  how long this frame took
 */
void generic_anim_render_fixed_frame_delay(generic_anim* ga, float frametime)
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
				if(!ga->done_playing){
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
			if(ga->type == BM_TYPE_ANI) {
				generic_render_ani_stream(ga);
			} else {
				generic_render_eff_stream(ga);
			}
			gr_set_bitmap(ga->bitmap_id);
		}
		else {
			gr_set_bitmap(ga->first_frame + ga->current_frame);
		}
	}
}

/*
 * @brief calculate current frame for variable frame delay animation formats (e.g. apng)
 *
 * @param [in] *ga  animation data
 * @param [in] frametime  how long this frame took
 * @param [in] alpha  transparency to draw frame with (0.0 - 1.0)
 *
 * @note
 * uses both time & frame counts to determine end state; so that if the anims playing
 * can't be processed fast enough, all frames will still play, rather than the end
 * frames being skipped
 */
void generic_anim_render_variable_frame_delay(generic_anim* ga, float frametime, float alpha)
{
	Assertion(ga->type == BM_TYPE_PNG, "only valid for apngs (currently); get a coder!");
	if (ga->keyframe != 0) {
		Warning(LOCATION, "apngs don't support keyframes");
		return;
	}

	// don't change the frame time if we're paused
	if((ga->direction & GENERIC_ANIM_DIRECTION_PAUSED) == 0) {
		if(ga->direction & GENERIC_ANIM_DIRECTION_BACKWARDS) {
			// playing backwards
			ga->anim_time -= frametime;
			if (ga->anim_time <= 0.0 && ga->png.anim->current_frame <= 0) {
				if(ga->direction & GENERIC_ANIM_DIRECTION_NOLOOP) {
					ga->anim_time = 0;  //stop on first frame when playing in reverse
				}
				else {
					// loop back to end
					ga->anim_time = ga->total_time;
					ga->png.previous_frame_time = ga->total_time;
					ga->png.anim->current_frame = ga->num_frames-1;
					ga->current_frame = ga->num_frames-1;
				}
			}
		}
		else {
			// playing forwards
			ga->anim_time += frametime;
			if(ga->anim_time >= ga->total_time && ga->png.anim->current_frame >= ga->png.anim->nframes) {
				if(ga->direction & GENERIC_ANIM_DIRECTION_NOLOOP) {
					ga->anim_time = ga->total_time;  // stop on last frame when playing
				}
				else {
					// loop back to start
					ga->anim_time = 0.0f;
					ga->png.previous_frame_time = 0.0f;
					ga->current_frame = 0;
					ga->png.anim->goto_start();
				}
				ga->done_playing = 1;
			}
		}
	}

	if (ga->num_frames > 0) {

		// just increment or decrement the frame by one
		// jumping forwards multiple frames will just exacerbate slowdowns as multiple frames
		// would need to be composed
		if (ga->direction & GENERIC_ANIM_DIRECTION_BACKWARDS) {
			if (ga->anim_time <= ga->png.previous_frame_time - ga->png.anim->frame.delay &&
					ga->png.anim->current_frame > 0) {
				ga->png.previous_frame_time -= ga->png.anim->frame.delay;
				ga->current_frame--;
			}
		}
		else {
			if (ga->anim_time >= ga->png.previous_frame_time + ga->png.anim->frame.delay &&
					ga->png.anim->current_frame < ga->png.anim->nframes) {
				ga->png.previous_frame_time += ga->png.anim->frame.delay;
				ga->current_frame++;
			}
		}

		// verbose debug; but quite useful
		nprintf(("apng", "apng generic render timings/frames: %04f %04f %04f %04f | %03i %03i %03i\n",
				frametime, ga->anim_time, ga->png.anim->frame.delay, ga->png.previous_frame_time,
				ga->previous_frame, ga->current_frame, ga->png.anim->current_frame));

		Assertion(ga->streaming != 0, "non-streaming apngs not implemented yet");
		// note: generic anims are not currently ever non-streaming in FSO
		// I'm not even sure that the existing ani/eff code would allow non-streaming generic anims
		generic_render_png_stream(ga);
		gr_set_bitmap(ga->bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha);
	}
}


/*
 * @brief render animations
 *
 * @param [in] *ga  animation data
 * @param [in] frametime  how long this frame took
 * @param [in] x  2D screen x co-ordinate to render at
 * @param [in] y  2D screen y co-ordinate to render at
 * @param [in] menu select if this is rendered in menu screen, or fullscreen
 */
void generic_anim_render(generic_anim *ga, float frametime, int x, int y, bool menu, const generic_extras *ge)
{
	if ((ge != nullptr) && (ga->use_hud_color == true)) {
		Warning(LOCATION, "Monochrome generic anims can't use extra info (yet)");
		return;
	}

	float a = 1.0f;
	if (ge != nullptr) {
		a = ge->alpha;
	}
	if (ga->type == BM_TYPE_PNG) {
		generic_anim_render_variable_frame_delay(ga, frametime, a);
	}
	else {
		generic_anim_render_fixed_frame_delay(ga, frametime);
	}

	if(ga->num_frames > 0) {
		ga->previous_frame = ga->current_frame;

		if(ga->use_hud_color) {
			gr_aabitmap(x, y, (menu ? GR_RESIZE_MENU : GR_RESIZE_FULL));
		}
		else {
			if (ge == nullptr) {
				gr_bitmap(x, y, (menu ? GR_RESIZE_MENU : GR_RESIZE_FULL));
			}
			else if (ge->draw == true) {
				// currently only for lua streaminganim objects
				// and don't draw them unless requested...
				gr_bitmap_uv(x, y, ge->width, ge->height, ge->u0, ge->v0, ge->u1, ge->v1, GR_RESIZE_NONE);
			}
		}
	}
}

/*
 * @brief reset an animation back to the start
 *
 * @param [in] *ga  animation data
 */
void generic_anim_reset(generic_anim *ga) {
	ga->anim_time = 0.0f;
	ga->current_frame = 0;
	if (ga->type == BM_TYPE_PNG) {
		ga->png.previous_frame_time = 0.0f;
		ga->png.anim->goto_start();
	}
}

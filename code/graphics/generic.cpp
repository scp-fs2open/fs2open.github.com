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

// Ends a failed EFF stream: releases the pair and next-frame load the stream owns and leaves the struct as an unloaded
// one (num_frames 0, type NONE, as the APNG branch does), since generic_anim_unload would otherwise release handle -1.
static void eff_stream_drop(generic_anim* ga, bool owned)
{
	if (owned) {
		if (bm_is_valid(ga->eff.next_frame)) {
			bm_release(ga->eff.next_frame);
		}
		if (bm_is_valid(ga->bitmap_id)) {
			bm_release(ga->bitmap_id);
		}
		vm_free(ga->buffer);
	}
	ga->buffer = nullptr;
	ga->bitmap_id = -1;
	ga->eff.next_frame = -1;
	ga->num_frames = 0;
	ga->type = BM_TYPE_NONE;
}

// Loaders keep a frame at its file depth whatever was asked for, so a 24 bpp frame gains alpha for a 32 bpp stream and
// color folds to luminance for an 8 bpp HUD-color stream (the OpenGL update path's formula). False means nothing was
// copied: no lock, a compressed payload (a DDS decoded on read is recorded as uncompressed), another size or depth.
static bool eff_copy_frame(const generic_anim* ga, int handle, const bitmap* bmp, int bpp)
{
	if (bmp == nullptr || bmp->data == 0 || bm_is_compressed(handle) || bmp->w != ga->width || bmp->h != ga->height) {
		return false;
	}
	auto src = reinterpret_cast<const ubyte*>(bmp->data);
	size_t pixels = static_cast<size_t>(ga->width) * ga->height;
	int src_bytes = bmp->bpp / 8;
	if (bmp->bpp == bpp) {
		memcpy(ga->buffer, src, pixels * src_bytes);
		return true;
	}
	if (bpp == 32 && bmp->bpp == 24) {
		for (size_t px = 0; px < pixels; px++) {
			ga->buffer[px * 4] = src[px * 3];
			ga->buffer[px * 4 + 1] = src[px * 3 + 1];
			ga->buffer[px * 4 + 2] = src[px * 3 + 2];
			ga->buffer[px * 4 + 3] = 255;
		}
		return true;
	}
	if (bpp == 8 && (bmp->bpp == 24 || bmp->bpp == 32)) {
		for (size_t px = 0; px < pixels; px++) {
			int luminance = src[px * src_bytes] + src[px * src_bytes + 1] + src[px * src_bytes + 2];
			auto value = static_cast<ubyte>(luminance / 3);
			if (src_bytes == 4) {
				value = static_cast<ubyte>((luminance / 3) * (src[px * 4 + 3] / 255.0f));
			}
			ga->buffer[px] = value;
		}
		return true;
	}
	return false;
}

int generic_anim_stream(generic_anim *ga, const bool cache)
{
	CFILE *img_cfp = NULL;
	int anim_fps = 0;
	int bpp;
	// what this struct streamed last: only a buffer the EFF path allocated is the EFF path's to reuse or free
	BM_TYPE previous_type = ga->type;

	ga->type = BM_TYPE_NONE;

	auto res = cf_find_file_location_ext(ga->filename, BM_ANI_NUM_TYPES, bm_ani_ext_list, CF_TYPE_ANY);

	// could not be found, or is invalid for some reason; a failed attempt must not change what the struct still owns
	if ( !res.found ) {
		ga->type = previous_type;
		return -1;
	}

	//make sure we can open it
	img_cfp = cfopen_special(res, "rb", CF_TYPE_ANY);

	if (img_cfp == NULL) {
		ga->type = previous_type;
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
			ga->ani.animation = anim_load(ga->filename, CF_TYPE_ANY);
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
		// the talking-head code streams the same generic_anim again without unloading it, so what the previous EFF run
		// owns is reused when it still fits and released when it does not
		bool owned = (previous_type == BM_TYPE_EFF) && (ga->buffer != nullptr);
		if (!owned) {
			// the union field still holds another member's bytes on a fresh struct; nothing is loaded yet
			ga->eff.next_frame = -1;
		}
		// a private slot: bm_load would hand back another stream's next-frame slot when that one sits on frame 0
		// (bm_reload names the slot after the frame it holds), and releasing it below would freeze that stream
		int first_frame = bm_load_duplicate(frame_name);
		if(first_frame < 0) {
			mprintf(("Cannot find first frame for eff streaming. eff Filename: %s\n", ga->filename));
			eff_stream_drop(ga, owned);
			return -1;
		}
		// the stream draws from a user bitmap over its own buffer, like the ANI and APNG streams, so the current frame
		// survives any renderer-side texture flush
		int frame_w = 0, frame_h = 0;
		bm_get_info(first_frame, &frame_w, &frame_h);
		size_t frame_bytes = static_cast<size_t>(frame_w) * frame_h * (bpp / 8);
		bool reuse = owned && bm_is_valid(ga->bitmap_id) && (ga->width == frame_w) && (ga->height == frame_h)
			&& (ga->eff.bpp == bpp);
		if (!reuse) {
			if (owned) {
				if (bm_is_valid(ga->bitmap_id)) {
					bm_release(ga->bitmap_id);
				}
				vm_free(ga->buffer);
			}
			ga->width = frame_w;
			ga->height = frame_h;
			ga->eff.bpp = bpp;
			ga->buffer = static_cast<ubyte*>(vm_malloc(frame_bytes));
			ga->bitmap_id = bm_create(bpp, frame_w, frame_h, ga->buffer, (bpp==8)?BMP_AABITMAP:0);
			if (ga->bitmap_id < 0) {
				mprintf(("Cannot create the stream bitmap for eff streaming. eff Filename: %s\n", ga->filename));
				bm_release(first_frame);
				eff_stream_drop(ga, true);
				return -1;
			}
		}
		bitmap* first_bmp = bm_lock(first_frame, bpp, (bpp==8)?BMP_AABITMAP:BMP_TEX_NONCOMP, true);
		if (first_bmp == nullptr) {
			// the failed lock has unloaded the private slot already; the release frees it
			mprintf(("Cannot read first frame for eff streaming. eff Filename: %s\n", ga->filename));
			bm_release(first_frame);
			eff_stream_drop(ga, true);
			return -1;
		}
		if (!eff_copy_frame(ga, first_frame, first_bmp, bpp)) {
			mprintf(("eff stream %s: first frame could not be copied, the stream starts blank\n", ga->filename));
			memset(ga->buffer, 0, frame_bytes);
		}
		bm_unlock(first_frame);
		bm_release(first_frame);
		if (reuse) {
			// the kept bitmap still holds the previous run's last frame on the GPU
			gr_update_texture(ga->bitmap_id, bpp, ga->buffer, ga->width, ga->height);
		}
		if (snprintf(frame_name, MAX_FILENAME_LEN, "%s_0001", ga->filename) >= MAX_FILENAME_LEN) {
			// Make sure the string is null terminated
			frame_name[MAX_FILENAME_LEN - 1] = '\0';
		}
		if (owned && bm_is_valid(ga->eff.next_frame)) {
			// the restart loads the next-frame file again below; drop the previous run's load first
			bm_release(ga->eff.next_frame);
		}
		// private for the same reason as frame 0: bm_reload renames this slot every advance, and a second stream of the
		// file would otherwise share it and lose it when the first stream unloads
		ga->eff.next_frame = bm_load_duplicate(frame_name);
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
				if (ga->buffer != nullptr) {
					vm_free(ga->buffer);
					ga->buffer = nullptr;
				}
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
			if (eff_copy_frame(ga, ga->eff.next_frame, next_frame_bmp, bpp)) {
				gr_update_texture(ga->bitmap_id, bpp, ga->buffer, ga->width, ga->height);
			} else {
				bool unread = (next_frame_bmp == nullptr);
				nprintf(("Warning",
					"eff stream %s: frame %d %s (%dx%d at %d bpp%s, stream is %dx%d at %d bpp); frame kept\n",
					ga->filename, ga->current_frame, unread ? "could not be read" : "could not be copied",
					unread ? 0 : next_frame_bmp->w, unread ? 0 : next_frame_bmp->h, unread ? 0 : next_frame_bmp->bpp,
					bm_is_compressed(ga->eff.next_frame) ? " compressed" : "", ga->width, ga->height, bpp));
			}
			if (next_frame_bmp != nullptr) {
				bm_unlock(ga->eff.next_frame);
			}
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
void generic_anim_render_fixed_frame_delay(generic_anim* ga, float frametime, float alpha = 1.0f)
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

			if (alpha == 1.0f) {
				gr_set_bitmap(ga->bitmap_id);
			} else {
				gr_set_bitmap(ga->bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha);
			}
		}
		else {
			if (alpha == 1.0f) {
				gr_set_bitmap(ga->first_frame + ga->current_frame);
			} else {
				gr_set_bitmap(ga->first_frame + ga->current_frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, alpha);
			}
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
void generic_anim_render(generic_anim *ga, float frametime, int x, int y, bool menu, const generic_extras *ge, float scale_factor)
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
		generic_anim_render_fixed_frame_delay(ga, frametime, a);
	}

	if(ga->num_frames > 0) {
		ga->previous_frame = ga->current_frame;

		if(ga->use_hud_color) {
			gr_aabitmap(x, y, (menu ? GR_RESIZE_MENU : GR_RESIZE_FULL), false, scale_factor);
		}
		else {
			if (ge == nullptr) {
				gr_bitmap(x, y, (menu ? GR_RESIZE_MENU : GR_RESIZE_FULL), false, scale_factor);
			}
			else if (ge->draw == true) {
				// currently only for lua streaminganim objects
				// and don't draw them unless requested...
				gr_bitmap_uv(x, y, ge->width, ge->height, ge->u0, ge->v0, ge->u1, ge->v1, ge->resize_mode);
			}
		}
	}
}

void generic_anim_bitmap_set(generic_anim* ga, float frametime, const generic_extras* ge)
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
		generic_anim_render_fixed_frame_delay(ga, frametime, a);
	}

	if (ga->num_frames > 0)
		ga->previous_frame = ga->current_frame;
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

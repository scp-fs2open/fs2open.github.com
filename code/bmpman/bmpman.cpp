/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/
#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif

#define WIN32_LEAN_AND_MEAN
#define BMPMAN_INTERNAL

#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "bmpman/bm_internal.h"
#include "ddsutils/ddsutils.h"
#include "debugconsole/console.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "io/key.h"
#include "io/timer.h"
#include "jpgutils/jpgutils.h"
#include "network/multiutil.h"
#include "parse/parselo.h"
#include "pcxutils/pcxutils.h"
#include "pngutils/pngutils.h"
#include "ship/ship.h"
#include "tgautils/tgautils.h"
#include "tracing/Monitor.h"
#include "tracing/tracing.h"

#include <cctype>
#include <climits>
#include <iomanip>
#include <memory>

// --------------------------------------------------------------------------------------------------------------------
// Private macros.

/**
 * @todo upgrade this to an inline funciton, taking bitmap_entry and const char* as arguments
 */
#define EFF_FILENAME_CHECK { if ( be->type == BM_TYPE_EFF ) strcpy_s( filename, be->info.ani.eff.filename ); else strcpy_s( filename, be->filename ); }
// --------------------------------------------------------------------------------------------------------------------
// Monitor variables
MONITOR(NumBitmapPage)
MONITOR(SizeBitmapPage)

// --------------------------------------------------------------------------------------------------------------------
// Definition of public variables (declared as extern in bmpman.h).
int ENVMAP = -1;

size_t bm_texture_ram = 0;
int Bm_paging = 0;

// Extension type lists
const BM_TYPE bm_type_list[] = { BM_TYPE_DDS, BM_TYPE_TGA, BM_TYPE_PNG, BM_TYPE_JPG, BM_TYPE_PCX };
const char *bm_ext_list[] = { ".dds", ".tga", ".png", ".jpg", ".pcx" };
const int BM_NUM_TYPES = sizeof(bm_type_list) / sizeof(bm_type_list[0]);

const BM_TYPE bm_ani_type_list[] = { BM_TYPE_EFF, BM_TYPE_ANI, BM_TYPE_PNG };
// NOTE: it would be better to have apng files use the .apng extension
// However there's lots of assumptions throughout the codebase that file extensions are only
// three chars long. If this is ever changed the the .apng extension could be used
const char *bm_ani_ext_list[] = { ".eff", ".ani", ".png" };
const int BM_ANI_NUM_TYPES = sizeof(bm_ani_type_list) / sizeof(bm_ani_type_list[0]);

void(*bm_set_components)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a) = NULL;

// --------------------------------------------------------------------------------------------------------------------
// Definition of public variables (declared as extern in bm_internal.h).
SCP_vector<std::array<bitmap_slot, BM_BLOCK_SIZE>> bm_blocks;

// --------------------------------------------------------------------------------------------------------------------
// Definition of private variables at file scope (static).
static bool bm_inited = false;
static uint Bm_next_signature = 0x1234;
static int Bm_low_mem = 0;

SCP_map<int,ubyte*> bm_lookup_cache;

/**
 * How much RAM bmpman can use for textures.
 *
 * @details Set to <1 to make it use all it wants.
 *
 * @note was initialized to 16*1024*1024 at some point to "use only 16MB for textures"
 */
static int Bm_max_ram = 0;

static int Bm_ignore_duplicates = 0;
static int Bm_ignore_load_count = 0;

// This needs to be declared somewhere and bm_internal.h has no own source file
gr_bitmap_info::~gr_bitmap_info() = default;

/**
 * Finds if a bitmap entry contains an animation
 */
static bool bm_is_anim(bitmap_entry* entry)
{
	return ((entry->type == BM_TYPE_ANI) ||
		(entry->type == BM_TYPE_EFF) ||
		(entry->type == BM_TYPE_PNG && entry->info.ani.apng.is_apng));
}

bitmap_slot* bm_get_slot(int handle, bool separate_ani_frames) {
	Assertion(handle >= 0, "Invalid handle %d passed to bm_get_slot!", handle);

	// The lower 16-bit contain the index in the bitmap block
	auto index = handle & 0xFFFF;
	// The upper 16-bit contain the number of the used block
	auto block_index = handle >> 16;

	Assertion(block_index >= 0 && block_index < (int) bm_blocks.size(),
			  "Bitmap handle %d has an invalid block number %d!",
			  handle,
			  block_index);
	Assertion(index >= 0 && index < (int) bm_blocks[block_index].size(),
			  "Bitmap handle %d has an invalid block index %d!",
			  handle,
			  index);

	auto slot = &bm_blocks[block_index][index];
	auto entry = &slot->entry;

	if (separate_ani_frames || !bm_is_anim(entry)) {
		// Not an animation or we don't want the first frame so we are done now
		return slot;
	}

	// Get the entry of the first frame by calling ourself again but this time we enforce separate ani frames (since we
	// already know that this handle is the correct one)
	return bm_get_slot(entry->info.ani.first_frame, true);
}

// --------------------------------------------------------------------------------------------------------------------
// Declaration of private functions and templates(declared as static type func(type param);)

bitmap_lookup::bitmap_lookup(int bitmap_num):
	Bitmap_data(NULL)
{
	if ( !bm_is_valid(bitmap_num) ) return;

	Num_channels = 3;

	if ( bm_has_alpha_channel(bitmap_num) ) {
		Num_channels = 4;
	}

	bitmap_entry *be = bm_get_entry(bitmap_num);
	
	Width = be->bm.w;
	Height = be->bm.h;


	auto cache_search = bm_lookup_cache.find(bitmap_num);
	if (cache_search == bm_lookup_cache.end()) {
		Bitmap_data = (ubyte*)vm_malloc(Width * Height * Num_channels * sizeof(ubyte));

		gr_get_bitmap_from_texture((void*)Bitmap_data, bitmap_num);
		bm_lookup_cache.insert({bitmap_num, Bitmap_data});
	} else {
		Bitmap_data = cache_search->second;
	}
}

bitmap_lookup::~bitmap_lookup()
{
}

bool bitmap_lookup::valid()
{
	return Bitmap_data != NULL;
}

float bitmap_lookup::map_texture_address(float address)
{
	// assume we're just wrapping
	return address - floorf(address);
}

float bitmap_lookup::get_channel_alpha(float u, float v)
{
	Assert( Bitmap_data != NULL );

	int x = fl2i(map_texture_address(u) * (Width-1));
	int y = fl2i(map_texture_address(v) * (Height-1));

	return i2fl(Bitmap_data[(y*Width + x)*Num_channels + 3]) / 255.0f;
}

void clear_bm_lookup_cache() {
	for(auto &iter: bm_lookup_cache) {
		free(iter.second);
	}
	bm_lookup_cache.clear();
}

/**
 * Converts the bitmap referenced by bmp to the type specified by flags
 */
static void bm_convert_format(bitmap *bmp, ushort flags);

/**
 * Frees a bitmap's data if it can
 */
static void bm_free_data(bitmap_slot* n, bool release = false);

/**
 * A special version of bm_free_data() that can be safely used in gr_*_texture
 * to save system memory once textures have been transfered to API memory
 * it doesn't restore the slot to a pristine state, it only releases the data
 *
 * @attention: THIS SHOULD ONLY BE USED FROM bm_unload_fast()!!!
 */
static void bm_free_data_fast(int handle);

/**
 * Given a raw filename and an extension set, try and find the bitmap
 * that isn't already loaded and may exist somewhere on the disk
 *
 * @returns -1 if it could not be found,
 * @returns index into ext_list[] if it was found as a file, fills img_cfg if available
 */
static int bm_load_sub_slow(const char *real_filename, const int num_ext, const char **ext_list, CFILE **img_cfp = NULL, int dir_type = CF_TYPE_ANY);

/**
 * Given a raw filename, try and find a bitmap that's already loaded
 *
 * @returns 0 if it could not be found,
 * @returns 1 if it already exists, fills in handle
 */
static int bm_load_sub_fast(const char *real_filename, int *handle, int dir_type = CF_TYPE_ANY, bool animated_type = false);

/**
 * @brief Finds a start handle to a block of contiguous bitmap slots
 *
 * This will return a bitmap handle which has space for n slots.
 *
 * @param n How many slots the bitmap needs
 * @param start_block At which block the search should start. Do not use. This is only for internal purposes.
 *
 * @returns -1 if the block could not be found
 * @returns the handle of the block ?
 */
static int find_block_of(int n, int start_block = 0);


static int get_handle(int block, int index) {
	Assertion(block >= 0, "Negative block values are not allowed!");
	Assertion(index >= 0, "Negative index values are not allowed!");

	return (uint32_t) block << 16 | (uint16_t) index;
}

static void allocate_new_block() {
	bm_blocks.emplace_back();
	auto& new_block = bm_blocks.back();

	for (auto& slot : new_block) {
		auto& entry = slot.entry;

		entry.filename[0] = '\0';
		entry.type = BM_TYPE_NONE;
		entry.comp_type = BM_TYPE_NONE;
		entry.dir_type = CF_TYPE_ANY;
		entry.info.user.data = nullptr;
		entry.mem_taken = 0;
		entry.bm.data = 0;
		entry.bm.palette = nullptr;
		entry.info.ani.eff.type = BM_TYPE_NONE;
		entry.info.ani.eff.filename[0] = '\0';
#ifdef BMPMAN_NDEBUG
		entry.data_size = 0;
		entry.used_count = 0;
		entry.used_last_frame = 0;
		entry.used_this_frame = 0;
#endif
		entry.load_count = 0;

		gr_bm_init(&slot);

		// clears flags, bbp, data, etc
		bm_free_data(&slot);
	}
}

// --------------------------------------------------------------------------------------------------------------------
// Macro-defined functions

DCF(bm_frag, "Shows BmpMan fragmentation") {
	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Displays a graphic showing the BmpMan fragmentation. Color key:\n");
		dc_printf("\tGray  : NONE\n");
		dc_printf("\tRed   : PCXn");
		dc_printf("\tGreen : USER, TGA, PNG, DDS, other\n");
		dc_printf("\tBlue  : ANI, EFF\n\n");

		dc_printf("Once done reviewing the graphic, press any key to return to the console\n");
		return;
	}

	gr_clear();

	int x = 0, y = 0;
	int xs = 2, ys = 2;
	int w = 4, h = 4;

	for (auto& block : bm_blocks) {
		for (size_t i = 0; i < BM_BLOCK_SIZE; ++i) {
			switch (block[i].entry.type) {
			case BM_TYPE_NONE:
				gr_set_color(128, 128, 128);
				break;
			case BM_TYPE_PCX:
				gr_set_color(255, 0, 0);
				break;
			case BM_TYPE_USER:
			case BM_TYPE_TGA:
			case BM_TYPE_PNG:
			case BM_TYPE_DDS:
				gr_set_color(0, 255, 0);
				break;
			case BM_TYPE_ANI:
			case BM_TYPE_EFF:
				gr_set_color(0, 0, 255);
				break;
			default:
				gr_set_color(0, 255, 0);
				break;
			}

			gr_rect(x + xs, y + ys, w, h);
			x += w + xs + xs;
			if (x > 639) {
				x = 0;
				y += h + ys + ys;
			}
		}
	}

	gr_flip();
	key_getch();
}

DCF(bm_used, "Shows BmpMan Slot Usage") {
	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Displays used bmpman slots usage with a breakdown per filetype\n\n");
		return;
	}

	int none = 0, pcx = 0, user = 0, tga = 0, png = 0; int jpg = 0, dds = 0, ani = 0;
	int eff = 0, eff_dds = 0, eff_tga = 0, eff_png = 0, eff_jpg = 0, eff_pcx = 0;
	int render_target_dynamic = 0, render_target_static = 0;

	for (auto& block : bm_blocks) {
		for (size_t i = 0; i < BM_BLOCK_SIZE; ++i) {
			switch (block[i].entry.type) {
			case BM_TYPE_NONE:
				none++;
				break;
			case BM_TYPE_PCX:
				pcx++;
				break;
			case BM_TYPE_USER:
				user++;
				break;
			case BM_TYPE_TGA:
				tga++;
				break;
			case BM_TYPE_PNG:
				// TODO distinguish png(static) from apng
				png++;
				break;
			case BM_TYPE_JPG:
				jpg++;
				break;
			case BM_TYPE_DDS:
				dds++;
				break;
			case BM_TYPE_ANI:
				ani++;
				break;
			case BM_TYPE_EFF:
				eff++;
				switch (block[i].entry.info.ani.eff.type) {
				case BM_TYPE_DDS:
					eff_dds++;
					break;
				case BM_TYPE_TGA:
					eff_tga++;
					break;
				case BM_TYPE_PNG:
					eff_png++;
					break;
				case BM_TYPE_JPG:
					eff_jpg++;
					break;
				case BM_TYPE_PCX:
					eff_pcx++;
					break;
				default:
					Warning(LOCATION, "Unhandled EFF image type (%i), get a coder!", block[i].entry.info.ani.eff.type);
					break;
				}
				break;
			case BM_TYPE_RENDER_TARGET_STATIC:
				render_target_static++;
				break;
			case BM_TYPE_RENDER_TARGET_DYNAMIC:
				render_target_dynamic++;
				break;
			default:
				Warning(LOCATION, "Unhandled image type (%i), get a coder!", block[i].entry.type);
				break;
			}
		}
	}

	SCP_stringstream text;
	text << "BmpMan Used Slots\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << pcx  << ", PCX\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << user << ", User\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << tga  << ", TGA\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << png  << ", PNG\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << jpg  << ", JPG\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << dds  << ", DDS\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << ani  << ", ANI\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << eff  << ", EFF\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << eff_dds  << ", EFF/DDS\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << eff_tga  << ", EFF/TGA\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << eff_png  << ", EFF/PNG\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << eff_jpg  << ", EFF/JPG\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << eff_pcx  << ", EFF/PCX\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << render_target_static  << ", Render/Static\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << render_target_dynamic  << ", Render/Dynamic\n";
	text << "  " << std::dec << std::setw(4) << std::setfill('0') << bmpman_count_bitmaps() << "/" << bmpman_count_available_slots()  << ", Total\n";
	text << "\n";

	// TODO consider converting 1's to monospace to make debug console output prettier
	mprintf(("%s", text.str().c_str())); // log for ease for copying data
	dc_printf("%s", text.str().c_str()); // instant gratification
}


DCF(bmpman, "Shows/changes bitmap caching parameters and usage") {
	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Usage: BmpMan [arg]\nWhere arg can be any of the following:\n");
		dc_printf("\tflush    Unloads all bitmaps.\n");
		dc_printf("\tram [x]  Sets max mem usage to x MB. (Set to 0 to have no limit.)\n");
		dc_printf("\t?        Displays status of Bitmap manager.\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Total RAM usage: " SIZE_T_ARG " bytes\n", bm_texture_ram);

		if (Bm_max_ram > 1024 * 1024) {
			dc_printf("\tMax RAM allowed: %.1f MB\n", i2fl(Bm_max_ram) / (1024.0f*1024.0f));
		} else if (Bm_max_ram > 1024) {
			dc_printf("\tMax RAM allowed: %.1f KB\n", i2fl(Bm_max_ram) / (1024.0f));
		} else if (Bm_max_ram > 0) {
			dc_printf("\tMax RAM allowed: %d bytes\n", Bm_max_ram);
		} else {
			dc_printf("\tNo RAM limit\n");
		}
		return;
	}


	if (dc_optional_string("flush")) {
		dc_printf("Total RAM usage before flush: " SIZE_T_ARG " bytes\n", bm_texture_ram);
		for (auto& block : bm_blocks) {
			for (size_t i = 0; i < BM_BLOCK_SIZE; ++i) {
				if (block[i].entry.type != BM_TYPE_NONE) {
					bm_free_data(&block[i]);
				}
			}
		}
		dc_printf("Total RAM after flush: " SIZE_T_ARG " bytes\n", bm_texture_ram);
	} else if (dc_optional_string("ram")) {
		dc_stuff_int(&Bm_max_ram);

		if (Bm_max_ram > 0) {
			dc_printf("BmpMan limited to %i, MB's\n", Bm_max_ram);
			Bm_max_ram *= 1024 * 1024;
		} else if (Bm_max_ram == 0) {
			dc_printf("!!BmpMan memory is unlimited!!\n");
		} else {
			dc_printf("Illegal value. Must be non-negative.");
		}
	} else {
		dc_printf("<BmpMan> No argument given\n");
	}
}

DCF(bmpslots, "Writes bitmap slot info to fs2_open.log") {
	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Usage: bmpslots\n");
		dc_printf("\tWrites bitmap slot info to fs2_open.log\n");
		return;
	}
	bm_print_bitmaps();
}

// --------------------------------------------------------------------------------------------------------------------
// Definition of all functions, in alphabetical order
void bm_close() {
	if (bm_inited) {
		for (auto& block : bm_blocks) {
			for (auto& slot : block) {
				bm_free_data(&slot);            // clears flags, bbp, data, etc

				if (slot.gr_info != nullptr) {
					// free graphics data
					delete slot.gr_info;
					slot.gr_info = nullptr;
				}
			}
		}
		bm_blocks.clear();
		bm_inited = false;
	}
}

int bm_create(int bpp, int w, int h, void *data, int flags) {
	if (bpp == 8) {
		Assert(flags & BMP_AABITMAP);
	} else {
		Assert((bpp == 16) || (bpp == 24) || (bpp == 32));
	}

	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	int n = find_block_of(1);

	// Out of bitmap slots
	if (n == -1)
		return -1;

	// make sure that we have valid data
	if (data == NULL) {
		Int3();
		return -1;
	}

	auto entry = bm_get_entry(n);

	memset(entry, 0, sizeof(bitmap_entry));

	sprintf_safe(entry->filename, "TMP%dx%d+%d", w, h, bpp);
	entry->type = BM_TYPE_USER;
	entry->comp_type = BM_TYPE_NONE;

	entry->bm.w = (short)w;
	entry->bm.h = (short)h;
	entry->bm.rowsize = (short)w;
	entry->bm.bpp = (ubyte)bpp;
	entry->bm.true_bpp = (ubyte)bpp;
	entry->bm.flags = (ubyte)flags;
	entry->bm.data = 0;
	entry->bm.palette = nullptr;

	entry->info.user.bpp = (ubyte)bpp;
	entry->info.user.data = data;
	entry->info.user.flags = (ubyte)flags;

	entry->signature = Bm_next_signature++;

	entry->handle = n;
	entry->mem_taken = (w * h * (bpp >> 3));

	entry->load_count++;

	bm_update_memory_used(n, (int)entry->mem_taken);

	gr_bm_create(bm_get_slot(n));

	return n;
}

void bm_convert_format(bitmap *bmp, ushort flags) {
	int idx;

	// no transparency for 24 bpp images
	if (!(flags & BMP_AABITMAP) && (bmp->bpp == 24))
		return;

	if (Is_standalone) {
		Assert(bmp->bpp == 8);
		return;
	} else {
		if (flags & BMP_AABITMAP)
			Assert(bmp->bpp == 8);
		else
			Assert((bmp->bpp == 16) || (bmp->bpp == 32));
	}

	// maybe swizzle to be an xparent texture
	if (!(bmp->flags & BMP_TEX_XPARENT) && (flags & BMP_TEX_XPARENT)) {
		for (idx = 0; idx<bmp->w*bmp->h; idx++) {
			// if the pixel is transparent
			if (((ushort*)bmp->data)[idx] == Gr_t_green.mask) {
				((ushort*)bmp->data)[idx] = 0;
			}
		}

		bmp->flags |= BMP_TEX_XPARENT;
	}
}

void bm_free_data(bitmap_slot* bs, bool release)
{
	bitmap *bmp;

	auto be = &bs->entry;
	bmp = &be->bm;

	gr_bm_free_data(bs, release);

	// If there isn't a bitmap in this structure, don't
	// do anything but clear out the bitmap info
	if (be->type==BM_TYPE_NONE)
		goto SkipFree;

	// Don't free up memory for user defined bitmaps, since
	// BmpMan isn't the one in charge of allocating/deallocing them.
	if (be->type==BM_TYPE_USER) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
#endif
		goto SkipFree;
	}

	// If this bitmap doesn't have any data to free, skip
	// the freeing it part of this.
	if (bmp->data == 0) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
#endif
		goto SkipFree;
	}

	// Free up the data now!
#ifdef BMPMAN_NDEBUG
	bm_texture_ram -= be->data_size;
#endif
	vm_free((void *)bmp->data);

	// reset the load_count to at least 1, don't do this in SkipFree though
	// since the real count ends up wrong
	be->load_count = 1;

SkipFree:

	// Clear out & reset the bitmap data structure
	bmp->flags = 0;
	bmp->bpp = 0;
	bmp->data = 0;
	bmp->palette = NULL;
#ifdef BMPMAN_NDEBUG
	be->data_size = 0;
#endif
	be->signature = Bm_next_signature++;
}

void bm_free_data_fast(int handle)
{
	bitmap_entry *be;
	bitmap *bmp;

	be = bm_get_entry(handle);
	bmp = &be->bm;

	// If there isn't a bitmap in this structure, don't
	// do anything but clear out the bitmap info
	if (be->type == BM_TYPE_NONE)
		return;

	// Don't free up memory for user defined bitmaps, since
	// BmpMan isn't the one in charge of allocating/deallocing them.
	if (be->type == BM_TYPE_USER) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
#endif
		return;
	}

	// If this bitmap doesn't have any data to free, skip
	// the freeing it part of this.
	if (bmp->data == 0) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 ) {
			bm_texture_ram -= be->data_size;
			be->data_size = 0;
		}
#endif
		return;
	}

	// Free up the data now!
#ifdef BMPMAN_NDEBUG
	bm_texture_ram -= be->data_size;
	be->data_size = 0;
#endif
	vm_free((void *)bmp->data);
	bmp->data = 0;
}

int bm_get_anim_frame(const int frame1_handle, float elapsed_time, const float divisor, const bool loop)
{
	bitmap_entry *be = bm_get_entry(frame1_handle);

	if (be->info.ani.num_frames <= 1) {
		// this is a still image
		return 0;
	}
	int last_frame = be->info.ani.num_frames - 1;
	bitmap_entry *last_framep = bm_get_entry(frame1_handle + last_frame);

	if (elapsed_time < 0.0f) {
		elapsed_time = 0.0f;
	}

	int frame = 0;
	// variable frame delay animations
	if (be->info.ani.apng.is_apng == true) {
		if (divisor > 0.0f) {
			// scale to get the real elapsed time
			elapsed_time = elapsed_time / divisor * last_framep->info.ani.apng.frame_delay;
		}

		if (loop == true) {
			elapsed_time = fmod(elapsed_time, last_framep->info.ani.apng.frame_delay);
		}

		int i = frame1_handle;
		for ( ; i < (frame1_handle + be->info.ani.num_frames); ++i) {
			// see bm_lock_apng for precalculated incremental delay for each frame
			if (elapsed_time <= bm_get_entry(i)->info.ani.apng.frame_delay) {
				break;
			}
		}
		frame = i - frame1_handle;
	}
	// fixed frame delay animations; simpler
	else {
		if (divisor > 0.0f) {
			// scale to get the real elapsed time
			frame = fl2i(elapsed_time / divisor * be->info.ani.num_frames);
		}
		else {
			frame = fl2i(elapsed_time * i2fl(be->info.ani.fps));
		}

		if (loop == true) {
			frame %= be->info.ani.num_frames;
		}
	}
	// note; this also makes non-looping anims hold on their last frame
	CLAMP(frame, 0, last_frame);

	return frame;
}

void bm_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a) {
	int bit_32 = 0;

	if ((gr_screen.bits_per_pixel == 32) && (Gr_current_red == &Gr_red)) {
		bit_32 = 1;
	}

	if (r != NULL) {
		if (bit_32) {
			*r = ubyte(((*((unsigned int*)pixel) & Gr_current_red->mask) >> Gr_current_red->shift)*Gr_current_red->scale);
		} else {
			*r = ubyte(((((unsigned short*)pixel)[0] & Gr_current_red->mask) >> Gr_current_red->shift)*Gr_current_red->scale);
		}
	}

	if (g != NULL) {
		if (bit_32) {
			*g = ubyte(((*((unsigned int*)pixel) & Gr_current_green->mask) >> Gr_current_green->shift)*Gr_current_green->scale);
		} else {
			*g = ubyte(((((unsigned short*)pixel)[0] & Gr_current_green->mask) >> Gr_current_green->shift)*Gr_current_green->scale);
		}
	}

	if (b != NULL) {
		if (bit_32) {
			*b = ubyte(((*((unsigned int*)pixel) & Gr_current_blue->mask) >> Gr_current_blue->shift)*Gr_current_blue->scale);
		} else {
			*b = ubyte(((((unsigned short*)pixel)[0] & Gr_current_blue->mask) >> Gr_current_blue->shift)*Gr_current_blue->scale);
		}
	}

	// get the alpha value
	if (a != NULL) {
		*a = 1;

		Assert(!bit_32);
		if (!(((unsigned short*)pixel)[0] & 0x8000)) {
			*a = 0;
		}
	}
}

const char *bm_get_filename(int handle) {
	auto entry = bm_get_entry(handle);
	return entry->filename;
}

void bm_get_filename(int bitmapnum, char *filename) {
	if (!bm_is_valid(bitmapnum)) {
		strcpy(filename, "");
		return;
	}

	// return filename
	strcpy(filename, bm_get_entry(bitmapnum)->filename);
}

void bm_get_frame_usage(int *ntotal, int *nnew) {
#ifdef BMPMAN_NDEBUG
	*ntotal = 0;
	*nnew = 0;

	for (auto& block : bm_blocks) {
		for (auto& slot : block) {
			auto& entry = slot.entry;
			if ((entry.type != BM_TYPE_NONE) && (entry.used_this_frame)) {
				if (!entry.used_last_frame) {
					*nnew += (int)entry.mem_taken;
				}
				*ntotal += (int)entry.mem_taken;
			}
			entry.used_last_frame = entry.used_this_frame;
			entry.used_this_frame = 0;
		}
	}
#endif
}

int bm_get_info(int handle, int *w, int * h, ushort* flags, int *nframes, int *fps) {
	bitmap * bmp;

	if (!bm_inited) return -1;

	auto entry = bm_get_entry(handle);

	Assertion(entry->handle == handle, "Invalid bitmap handle %d passed to bm_get_info().\nThis might be due to an invalid animation somewhere else.\n", handle);		// INVALID BITMAP HANDLE!

	if ((entry->type == BM_TYPE_NONE) || (entry->handle != handle)) {
		if (w) *w = 0;
		if (h) *h = 0;
		if (flags) *flags = 0;
		if (nframes) *nframes = 0;
		if (fps) *fps = 0;
		return -1;
	}

	bmp = &(entry->bm);

	if (w) *w = bmp->w;
	if (h) *h = bmp->h;
	if (flags) *flags = bmp->flags;

	if (bm_is_anim(entry)) {
		if (nframes) {
			*nframes = entry->info.ani.num_frames;
		}
		if (fps) {
			*fps = entry->info.ani.fps;
		}

		return bm_get_entry(entry->info.ani.first_frame)->handle;
	} else {
		if (nframes) {
			*nframes = 1;
		}
		if (fps) {
			*fps = 0;
		}

		return handle;
	}
}

int bm_get_num_mipmaps(int num) {
	auto entry = bm_get_entry(num);

	if (entry->num_mipmaps == 0)
		return 1;

	return entry->num_mipmaps;
}

void bm_get_palette(int handle, ubyte *pal, char *name) {
	int w, h;

	const char* filename = bm_get_entry(handle)->filename;

	if (name) {
		strcpy(name, filename);
	}

	int pcx_error = pcx_read_header(filename, NULL, &w, &h, NULL, pal);
	if (pcx_error != PCX_ERROR_NONE) {
		// Error(LOCATION, "Couldn't open '%s'\n", filename );
	}
}

int bm_get_tcache_type(int num) {
	if (bm_is_compressed(num))
		return TCACHE_TYPE_COMPRESSED;

	return TCACHE_TYPE_NORMAL;
}

BM_TYPE bm_get_type(int handle) {
	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	return bm_get_entry(handle)->type;
}

bool bm_has_alpha_channel(int handle) {
	auto entry = bm_get_entry(handle);

	// assume that PCX never has a real alpha channel (it may be 32-bit, but without any alpha)
	if (entry->type == BM_TYPE_PCX)
		return 0;

	return (entry->bm.true_bpp == 32);
}

void bm_init() {
	Assertion(!bm_inited, "bmpman cannot be initialized more than once!");

	// Allocate one block by default
	allocate_new_block();

	bm_inited = true;
}

int bm_is_compressed(int num) {
	auto entry = bm_get_entry(num);
	BM_TYPE type = BM_TYPE_NONE;

	//duh
	if (!Use_compressed_textures)
		return 0;

	type = entry->comp_type;

	switch (type) {
	case BM_TYPE_NONE:
	case BM_TYPE_DDS:
		return 0;

	case BM_TYPE_DXT1:
		return DDS_DXT1;

	case BM_TYPE_DXT3:
		return DDS_DXT3;

	case BM_TYPE_DXT5:
		return DDS_DXT5;

	case BM_TYPE_BC7:
		return DDS_BC7;

	case BM_TYPE_CUBEMAP_DXT1:
		return DDS_CUBEMAP_DXT1;

	case BM_TYPE_CUBEMAP_DXT3:
		return DDS_CUBEMAP_DXT3;

	case BM_TYPE_CUBEMAP_DXT5:
		return DDS_CUBEMAP_DXT5;

	default:
		return 0;
	}
}

int bm_is_render_target(int bitmap_id) {
	auto entry = bm_get_entry(bitmap_id);

	if (!((entry->type == BM_TYPE_RENDER_TARGET_STATIC) || (entry->type == BM_TYPE_RENDER_TARGET_DYNAMIC))) {
		return 0;
	}

	return entry->type;
}

int bm_is_valid(int handle) {
	// Ensure that certain known false or out of range handles are quickly returned as invalid,
	// prior to utilising the handle in a way which leads to memory access outside bm_bitmaps[]
	if (!bm_inited) return 0;
	if (handle < 0) return 0;

	auto entry = bm_get_entry(handle);

	return entry->handle == handle;
}


// Load an image and validate it while retrieving information for later use
// Input:	type		= current BM_TYPE_*
//			n			= location in bm_bitmaps[]
//			filename	= name of the current file
//			img_cfp		= already open CFILE handle, if available
//
// Output:	w			= bmp width
//			h			= bmp height
//			bpp			= bmp bits per pixel
//			c_type		= output for an updated BM_TYPE_*
//			mm_lvl		= number of mipmap levels for the image
//			size		= size of the data contained in the image
static int bm_load_info(BM_TYPE type, const char *filename, CFILE *img_cfp, int *w, int *h, int *bpp, BM_TYPE *c_type, int *mm_lvl, size_t *size)
{
	int dds_ct;

	if (type == BM_TYPE_DDS) {
		int dds_error = dds_read_header(filename, img_cfp, w, h, bpp, &dds_ct, mm_lvl, size);

		if (dds_error != DDS_ERROR_NONE) {
			mprintf(("DDS ERROR: Couldn't open '%s' -- %s\n", filename, dds_error_string(dds_error)));
			return -1;
		}

		switch (dds_ct) {
		case DDS_DXT1:
			*c_type = BM_TYPE_DXT1;
			break;

		case DDS_DXT3:
			*c_type = BM_TYPE_DXT3;
			break;

		case DDS_DXT5:
			*c_type = BM_TYPE_DXT5;
			break;

		case DDS_BC7:
			*c_type = BM_TYPE_BC7;
			break;

		case DDS_UNCOMPRESSED:
			*c_type = BM_TYPE_DDS;
			break;

		case DDS_CUBEMAP_DXT1:
			*c_type = BM_TYPE_CUBEMAP_DXT1;
			break;

		case DDS_CUBEMAP_DXT3:
			*c_type = BM_TYPE_CUBEMAP_DXT3;
			break;

		case DDS_CUBEMAP_DXT5:
			*c_type = BM_TYPE_CUBEMAP_DXT5;
			break;

		case DDS_CUBEMAP_UNCOMPRESSED:
			*c_type = BM_TYPE_CUBEMAP_DDS;
			break;

		default:
			Error(LOCATION, "Bad DDS file compression! Not using DXT1,3,5: %s", filename);
			return -1;
		}
	}
	// if its a tga file
	else if (type == BM_TYPE_TGA) {
		int tga_error = targa_read_header(filename, img_cfp, w, h, bpp, NULL);
		if (tga_error != TARGA_ERROR_NONE) {
			mprintf(("tga: Couldn't open '%s'\n", filename));
			return -1;
		}
	}
	// if its a png file
	else if (type == BM_TYPE_PNG) {
		int png_error = png_read_header(filename, img_cfp, w, h, bpp, NULL);
		if (png_error != PNG_ERROR_NONE) {
			mprintf(("png: Couldn't open '%s'\n", filename));
			return -1;
		}
	}
	// if its a jpg file
	else if (type == BM_TYPE_JPG) {
		int jpg_error = jpeg_read_header(filename, img_cfp, w, h, bpp, NULL);
		if (jpg_error != JPEG_ERROR_NONE) {
			mprintf(("jpg: Couldn't open '%s'\n", filename));
			return -1;
		}
	}
	// if its a pcx file
	else if (type == BM_TYPE_PCX) {
		int pcx_error = pcx_read_header(filename, img_cfp, w, h, bpp, NULL);
		if (pcx_error != PCX_ERROR_NONE) {
			mprintf(("pcx: Couldn't open '%s'\n", filename));
			return -1;
		}
	}
	else {
		UNREACHABLE("Unknown file type specified! This is probably a coding error.");

		return -1;
	}

	return 0;
}

int bm_load(const char *real_filename) {
	int free_slot = -1;
	int w, h, bpp = 8;
	int rc = 0;
	size_t bm_size = 0;
	int mm_lvl = 0;
	char filename[MAX_FILENAME_LEN];
	BM_TYPE type = BM_TYPE_NONE;
	BM_TYPE c_type = BM_TYPE_NONE;
	CFILE *img_cfp = NULL;
	int handle = -1;

	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	// if no file was passed then get out now
	if ((real_filename == NULL) || (strlen(real_filename) <= 0))
		return -1;

	// make sure no one passed an extension
	memset(filename, 0, MAX_FILENAME_LEN);
	strncpy(filename, real_filename, MAX_FILENAME_LEN - 1);
	char *p = strrchr(filename, '.');
	if (p) {
		mprintf(("Someone passed an extension to bm_load for file '%s'\n", real_filename));
		*p = 0;
	}

	// If we are standalone server keep replacing the 'right_bracket' (right side help bracket) as the filename
	// should keep the game happy while loading only single pcx file which the needs to be present in any case
	if (Is_standalone) {
		char standalone_filename[MAX_FILENAME_LEN] = "right_bracket";
		strcpy_s(filename, standalone_filename);
	}

	// safety catch for strcat...
	// MAX_FILENAME_LEN-5 == '.' plus 3 letter ext plus NULL terminator
	if (strlen(filename) > MAX_FILENAME_LEN - 5) {
		Warning(LOCATION, "Passed filename, '%s', is too long to support an extension!!\n\nMaximum length, minus the extension, is %i characters.\n", filename, MAX_FILENAME_LEN - 5);
		return -1;
	}

	// Lets find out what type it is
	{
		// see if it's already loaded (checks for any type with filename)
		if (bm_load_sub_fast(filename, &handle))
			return handle;

		// if we are still here then we need to fall back to a file-based search
		int rval = bm_load_sub_slow(filename, BM_NUM_TYPES, bm_ext_list, &img_cfp);

		if (rval < 0)
			return -1;

		strcat_s(filename, bm_ext_list[rval]);
		type = bm_type_list[rval];
	}

	Assert(type != BM_TYPE_NONE);

	// Find an open slot
	free_slot = find_block_of(1);

	if (free_slot < 0) {
		Assertion(free_slot < 0, "Could not find free BMPMAN slot for bitmap: %s", real_filename);
		if (img_cfp != nullptr)
			cfclose(img_cfp);
		return -1;
	}

	rc = bm_load_info(type, filename, img_cfp, &w, &h, &bpp, &c_type, &mm_lvl, &bm_size);

	if (rc != 0) {
		if (img_cfp != nullptr)
			cfclose(img_cfp);
		return -1;
	}

	if ((bm_size <= 0) && (w) && (h) && (bpp))
		bm_size = (w * h * (bpp >> 3));


	handle = free_slot;

	auto entry = bm_get_entry(handle);

	// ensure fields are cleared out from previous bitmap
	memset(entry, 0, sizeof(bitmap_entry));

	// Mark the slot as filled, because cf_read might load a new bitmap
	// into this slot.
	strcpy_s(entry->filename, filename);
	entry->type = type;
	entry->comp_type = c_type;
	entry->signature = Bm_next_signature++;
	entry->bm.w = (short)w;
	entry->bm.rowsize = (short)w;
	entry->bm.h = (short)h;
	entry->bm.bpp = 0;
	entry->bm.true_bpp = (ubyte)bpp;
	entry->bm.flags = 0;
	entry->bm.data = 0;
	entry->bm.palette = nullptr;
	entry->num_mipmaps = mm_lvl;
	entry->mem_taken = (size_t)bm_size;
	entry->dir_type = CF_TYPE_ANY;
	entry->handle = handle;

	entry->load_count++;

	if (img_cfp != nullptr)
		cfclose(img_cfp);

	return handle;
}

int bm_load(const SCP_string& filename) {
	return bm_load(filename.c_str());
}

bool bm_load_and_parse_eff(const char *filename, int dir_type, int *nframes, int *nfps, int *key, BM_TYPE *type) {
	int frames = 0, fps = 30, keyframe = 0;
	char ext[8];
	BM_TYPE c_type = BM_TYPE_NONE;
	char file_text[1024];
	char file_text_raw[1024];

	memset(ext, 0, sizeof(ext));
	memset(file_text, 0, sizeof(file_text));
	memset(file_text_raw, 0, sizeof(file_text_raw));

	// pause anything that may happen to be parsing right now
	pause_parse();

	try
	{
		// now start parsing the EFF
		read_file_text(filename, dir_type, file_text, file_text_raw);
		reset_parse(file_text);

		required_string("$Type:");
		stuff_string(ext, F_NAME, sizeof(ext));

		required_string("$Frames:");
		stuff_int(&frames);

		if (optional_string("$FPS:"))
			stuff_int(&fps);

		if (optional_string("$Keyframe:"))
			stuff_int(&keyframe);
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("BMPMAN: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		unpause_parse();
		return false;
	}

	// done with EFF so unpause parsing so whatever can continue
	unpause_parse();

	if (!stricmp(NOX("dds"), ext)) {
		c_type = BM_TYPE_DDS;
	} else if (!stricmp(NOX("tga"), ext)) {
		c_type = BM_TYPE_TGA;
	} else if (!stricmp(NOX("png"), ext)) {
		c_type = BM_TYPE_PNG;
	} else if (!stricmp(NOX("jpg"), ext)) {
		c_type = BM_TYPE_JPG;
	} else if (!stricmp(NOX("pcx"), ext)) {
		c_type = BM_TYPE_PCX;
	} else {
		mprintf(("BMPMAN: Unknown file type in EFF parse!\n"));
		return false;
	}

	// did we do anything?
	if (c_type == BM_TYPE_NONE || frames == 0) {
		mprintf(("BMPMAN: EFF parse ERROR!\n"));
		return false;
	}

	if (type)
		*type = c_type;

	if (nframes)
		*nframes = frames;

	if (nfps)
		*nfps = fps;

	if (key)
		*key = keyframe;

	return true;
}

/**
* Lock an image files data into memory
*/
static int bm_load_image_data(int handle, int bpp, ushort flags, bool nodebug)
{
	BM_TYPE c_type = BM_TYPE_NONE;
	int true_bpp;

	auto bs = bm_get_slot(handle);
	auto be = &bs->entry;
	bitmap *bmp = &be->bm;

	if (bmp->true_bpp > bpp)
		true_bpp = bmp->true_bpp;
	else
		true_bpp = bpp;

	// don't do a bpp check here since it could be different in OGL - taylor
	if (bmp->data == 0) {
		Assert(be->ref_count == 1);

		if (be->type != BM_TYPE_USER && !nodebug) {
			if (bmp->data == 0)
				nprintf(("BmpMan", "Loading %s for the first time.\n", be->filename));
		}

		if (!Bm_paging) {
			if (be->type != BM_TYPE_USER && !nodebug)
				nprintf(("Paging", "Loading %s (%dx%dx%d)\n", be->filename, bmp->w, bmp->h, true_bpp));
		}

		// select proper format
		if (flags & BMP_AABITMAP)
			BM_SELECT_ALPHA_TEX_FORMAT();
		else if (flags & BMP_TEX_ANY)
			BM_SELECT_TEX_FORMAT();
		else
			BM_SELECT_SCREEN_FORMAT();

		// make sure we use the real graphic type for EFFs
		if (be->type == BM_TYPE_EFF) {
			c_type = be->info.ani.eff.type;
		}
		else {
			c_type = be->type;
		}

		switch (c_type)
		{
		case BM_TYPE_PCX:
			bm_lock_pcx(handle, bs, bmp, true_bpp, flags);
			break;

		case BM_TYPE_ANI:
			bm_lock_ani(handle, bs, bmp, true_bpp, flags);
			break;

		case BM_TYPE_TGA:
			bm_lock_tga(handle, bs, bmp, true_bpp, flags);
			break;

		case BM_TYPE_PNG:
			//libpng handles compression with zlib
			if (be->info.ani.apng.is_apng == true) {
				bm_lock_apng( handle, bs, bmp, true_bpp, flags );
			}
			else {
				bm_lock_png( handle, bs, bmp, true_bpp, flags );
			}
			break;

		case BM_TYPE_JPG:
			bm_lock_jpg(handle, bs, bmp, true_bpp, flags);
			break;

		case BM_TYPE_DDS:
		case BM_TYPE_DXT1:
		case BM_TYPE_DXT3:
		case BM_TYPE_DXT5:
		case BM_TYPE_BC7:
		case BM_TYPE_CUBEMAP_DDS:
		case BM_TYPE_CUBEMAP_DXT1:
		case BM_TYPE_CUBEMAP_DXT3:
		case BM_TYPE_CUBEMAP_DXT5:
			bm_lock_dds(handle, bs, bmp, true_bpp, flags);
			break;

		case BM_TYPE_USER:
			bm_lock_user(handle, bs, bmp, true_bpp, flags);
			break;

		default:
			Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", c_type);
			return -1;
		}

		// always go back to screen format
		BM_SELECT_SCREEN_FORMAT();

		// make sure we actually did something
		if (!(bmp->data))
			return -1;
	}

	return 0;
}

int bm_load_animation(const char *real_filename, int *nframes, int *fps, int *keyframe, float *total_time, bool can_drop_frames, int dir_type, bool rethrow_exceptions) {
	int	i, n;
	anim	the_anim;
	CFILE	*img_cfp = nullptr;
	char filename[MAX_FILENAME_LEN];
	int reduced = 0;
	int anim_fps = 0, anim_frames = 0, key = 0;
	float anim_total_time = 0.0f;
	int anim_width = 0, anim_height = 0;
	BM_TYPE type = BM_TYPE_NONE, eff_type = BM_TYPE_NONE, c_type = BM_TYPE_NONE;
	int bpp = 0, mm_lvl = 0;
	size_t img_size = 0;
	char clean_name[MAX_FILENAME_LEN];

	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	// set output param defaults before going any further
	if (nframes != nullptr)
		*nframes = 0;

	if (fps != nullptr)
		*fps = 0;

	if (keyframe != nullptr)
		*keyframe = 0;

	if (total_time != nullptr)
		*total_time = 0.0f;

	memset(filename, 0, MAX_FILENAME_LEN);
	strncpy(filename, real_filename, MAX_FILENAME_LEN - 1);
	char *p = strchr(filename, '.');
	if (p) {
		mprintf(("Someone passed an extension to bm_load_animation for file '%s'\n", real_filename));
		*p = 0;
	}

	// If we are standalone server keep replacing the 'cursorweb' (mouse cursor) as the filename
	// should keep the game happy while loading only single ani file which the needs to be present in any case
	if (Is_standalone) {
		char standalone_filename[MAX_FILENAME_LEN] = "cursorweb";
		strcpy_s(filename, standalone_filename);
	}

	// safety catch for strcat...
	// MAX_FILENAME_LEN-5 == '.' plus 3 letter ext plus NULL terminator
	if (strlen(filename) > MAX_FILENAME_LEN - 5) {
		Warning(LOCATION, "Passed filename, '%s', is too long to support an extension!!\n\nMaximum length, minus the extension, is %i characters.\n", filename, MAX_FILENAME_LEN - 5);
		return -1;
	}

	// used later if EFF type
	strcpy_s(clean_name, filename);

	// Lets find out what type it is
	{
		int handle = -1;

		// do a search for any previously loaded files (looks at filename only)
		if (bm_load_sub_fast(filename, &handle, dir_type, true)) {
			auto entry = bm_get_entry(handle);

			if (nframes)
				*nframes = entry->info.ani.num_frames;

			if (fps)
				*fps = entry->info.ani.fps;

			if (keyframe)
				*keyframe = entry->info.ani.keyframe;

			if (total_time != nullptr)
				*total_time = entry->info.ani.total_time;

			return handle;
		}

		// if we are still here then we need to fall back to a file-based search
		int rval = bm_load_sub_slow(filename, BM_ANI_NUM_TYPES, bm_ani_ext_list, &img_cfp, dir_type);

		if (rval < 0)
			return -1;

		strcat_s(filename, bm_ani_ext_list[rval]);
		type = bm_ani_type_list[rval];
	}

	// If we found an animation then there is an extra 5 char size limit to adhere to. We don't do this check earlier since it's only needed if we found an anim
	// an ANI needs about 5 extra characters to have the "[###]" frame designator
	// EFF/APNG need 5 extra characters for each frame filename too, which just happens to be the same length as the frame designator needed otherwise
	// MAX_FILENAME_LEN-10 == 5 character frame designator plus '.' plus 3 letter ext plus NULL terminator
	// we only check for -5 here since the filename should already have the extension on it, and it must have passed the previous check
	if (strlen(filename) > MAX_FILENAME_LEN - 5) {
		Warning(LOCATION, "Passed filename, '%s', is too long to support an extension and frames!!\n\nMaximum length for an ANI/EFF/APNG, minus the extension, is %i characters.\n", filename, MAX_FILENAME_LEN - 10);
		if (img_cfp != nullptr)
			cfclose(img_cfp);
		return -1;
	}

	// it's an effect file, any readable image type with eff being txt
	if (type == BM_TYPE_EFF) {
		if (!bm_load_and_parse_eff(filename, dir_type, &anim_frames, &anim_fps, &key, &eff_type)) {
			mprintf(("BMPMAN: Error reading EFF\n"));
			if (img_cfp != nullptr)
				cfclose(img_cfp);
			return -1;
		} else {
			mprintf(("BMPMAN: Found EFF (%s) with %d frames at %d fps.\n", filename, anim_frames, anim_fps));
		}
		if (anim_fps == 0) {
			Error(LOCATION, "animation (%s) has invalid fps of 0, fix this!", filename);
		}
		anim_total_time = anim_frames / i2fl(anim_fps);
	}
	// regular ani file
	else if (type == BM_TYPE_ANI) {
#ifndef NDEBUG
		// for debug of ANI sizes
		strcpy_s(the_anim.name, real_filename);
#endif
		anim_read_header(&the_anim, img_cfp);

		if (the_anim.width < 0 || the_anim.height < 0) {
			Error(LOCATION, "Ani file %s has a faulty header and cannot be loaded.", real_filename);
		}

		anim_frames = the_anim.total_frames;
		anim_fps = the_anim.fps;
		if (anim_fps == 0) {
			Error(LOCATION, "animation (%s) has invalid fps of 0, fix this!", filename);
		}
		anim_total_time = anim_frames / i2fl(anim_fps);
		anim_width = the_anim.width;
		anim_height = the_anim.height;
		bpp = 8;
		img_size = (anim_width * anim_height * bpp);
		//we only care if there are 2 keyframes - first frame, other frame to jump to for ship/weapons
		//mainhall door anis hav every frame as keyframe, so we don't care
		//other anis only have the first frame
		if (the_anim.num_keys == 2) {
			the_anim.keys = (key_frame*)vm_malloc(sizeof(key_frame) * the_anim.num_keys);
			Assert(the_anim.keys != NULL);

			for (i = 0; i<the_anim.num_keys; i++) {
				the_anim.keys[i].frame_num = 0;
				cfread(&the_anim.keys[i].frame_num, 2, 1, img_cfp);
				cfread(&the_anim.keys[i].offset, 4, 1, img_cfp);
				the_anim.keys[i].frame_num = INTEL_INT(the_anim.keys[i].frame_num); //-V570
				the_anim.keys[i].offset = INTEL_INT(the_anim.keys[i].offset); //-V570
			}
			//some retail anis have their keyframes reversed
			key = MAX(the_anim.keys[0].frame_num, the_anim.keys[1].frame_num);
			
			vm_free(the_anim.keys);
			the_anim.keys = nullptr;
		}
	}
	else if (type == BM_TYPE_PNG) {
		nprintf(("apng", "Loading apng: %s\n", filename));
		try {
			apng::apng_ani the_apng = apng::apng_ani(filename);
			anim_frames = the_apng.nframes;
			anim_total_time = the_apng.anim_time;
			anim_fps = fl2i(i2fl(anim_frames) / the_apng.anim_time); // note; apng bails on loading if anim_time is <= 0.0f
			anim_width = the_apng.w;
			anim_height = the_apng.h;
			bpp = the_apng.bpp;
			img_size = the_apng.imgsize();
		}
		catch (const apng::ApngException& e) {
			if (img_cfp != nullptr)
				cfclose(img_cfp);
			if (rethrow_exceptions) {
				throw e;
			}
			else {
				mprintf(("Failed to load apng: %s\n", e.what()));
				return -1;
			}
		}
	}
	else {
		Warning(LOCATION, "Unsupported image type: %i", type);
		if (img_cfp != nullptr)
			cfclose(img_cfp);
		return -1;
	}

	if ((can_drop_frames == true) && (type == BM_TYPE_ANI)) {
		if (Bm_low_mem == 1) {
			reduced = 1;
			anim_frames = (anim_frames + 1) / 2;
			anim_fps = (anim_fps / 2);
		} else if (Bm_low_mem == 2) {
			anim_frames = 1;
		}
	}


	n = find_block_of(anim_frames);

	if (n < 0) {
		if (img_cfp != nullptr)
			cfclose(img_cfp);

		return -1;
	}

	auto first_entry = bm_get_entry(n);
	// if all images of the animation have the same size then we can use a texture array
	bool is_array = true;

	for (i = 0; i < anim_frames; i++) {
		auto entry = bm_get_entry(n + i);
		memset(entry, 0, sizeof(bitmap_entry));

		if (type == BM_TYPE_EFF) {
			entry->info.ani.eff.type = eff_type;
			sprintf_safe(entry->info.ani.eff.filename, "%s_%.4d", clean_name, i);

			// bm_load_info() returns non-0 on failure
			if (bm_load_info(eff_type, entry->info.ani.eff.filename,
							 nullptr, &anim_width, &anim_height, &bpp, &c_type, &mm_lvl, &img_size)) {
				// if we didn't get anything then bail out now
				if (i == 0) {
					Warning(LOCATION, "EFF: No frame images were found.  EFF, %s, is invalid.\n", filename);

					if (img_cfp != nullptr)
						cfclose(img_cfp);

					return -1;
				}

				Warning(LOCATION, "EFF: Unable to load all frames for '%s', stopping at #%d\n", filename, i);

				// reset total frames to current
				anim_frames = i;

				// update all previous frames with the new count
				for (int j = 0; j<anim_frames; j++)
					bm_get_entry(n + j)->info.ani.num_frames = anim_frames;

				break;
			}

			if ((img_size <= 0) && (anim_width) && (anim_height) && (bpp)) {
				img_size = (anim_width * anim_height * (bpp >> 3));
			}
		}

		entry->info.ani.first_frame = n;
		entry->info.ani.num_frames = anim_frames;
		entry->info.ani.fps = (ubyte)anim_fps;
		entry->info.ani.keyframe = key;
		entry->info.ani.total_time = anim_total_time;
		entry->bm.w = (short)anim_width;
		entry->bm.rowsize = (short)anim_width;
		entry->bm.h = (short)anim_height;
		if (reduced) {
			entry->bm.w /= 2;
			entry->bm.rowsize /= 2;
			entry->bm.h /= 2;
		}
		entry->bm.flags = 0;
		entry->bm.bpp = 0;
		entry->bm.true_bpp = (ubyte)bpp;
		entry->bm.data = 0;
		entry->bm.palette = nullptr;
		entry->type = type;
		entry->comp_type = c_type;
		entry->signature = Bm_next_signature++;
		entry->handle = n + i;
		entry->num_mipmaps = mm_lvl;
		entry->mem_taken = (size_t)img_size;
		entry->dir_type = dir_type;

		entry->load_count++;

		if (i == 0) {
			sprintf_safe(entry->filename, "%s", filename);
		} else {
			if (type == BM_TYPE_PNG) {
				sprintf_safe(entry->filename, "%s_%04d", filename, i);
			}
			else {
				sprintf_safe(entry->filename, "%s[%d]", filename, i);
			}
		}

		entry->info.ani.apng.frame_delay = 0.0f;
		if (type == BM_TYPE_PNG) {
			entry->info.ani.apng.is_apng = true;
		}
		else {
			entry->info.ani.apng.is_apng = false;
		}

		if (first_entry->bm.w != entry->bm.w || first_entry->bm.h != entry->bm.h) {
			// We found a frame with a different size than the first frame -> this can't be used as a texture array
			is_array = false;

			Warning(LOCATION, "Animation '%s' has images that are of different sizes (currently at frame %d)."
				"Performance could be improved by making all images the same size.", filename, i + 1);
		}
		if (first_entry->comp_type != entry->comp_type) {
			// Different compression type
			is_array = false;

			Warning(LOCATION, "Animation '%s' has images that are of different compression formats (currently at frame %d)."
				"Performance could be improved by making all images the same compression format.", filename, i + 1);
		}
		if (first_entry->bm.true_bpp != entry->bm.true_bpp) {
			// We found a frame with an incompatible pixel format
			is_array = false;

			Warning(LOCATION, "Animation '%s' has images that are of different pixel formats (currently at frame %d)."
				"Performance could be improved by making all images the same pixel format.", filename, i + 1);
		}
		if (first_entry->num_mipmaps != entry->num_mipmaps) {
			// We found a frame with a different number of mipmaps
			is_array = false;

			Warning(LOCATION, "Animation '%s' has images that have a different number of mipmaps (currently at frame %d)."
				"Performance could be improved by giving all frames the same number of mipmaps.", filename, i + 1);
		}
	}

	// Set array flag of first frame
	first_entry->info.ani.is_array = is_array;

	if (nframes != nullptr)
		*nframes = anim_frames;

	if (fps != nullptr)
		*fps = anim_fps;

	if (img_cfp != nullptr)
		cfclose(img_cfp);

	if (keyframe != nullptr)
		*keyframe = key;

	if (total_time != nullptr)
		*total_time = anim_total_time;

	return first_entry->handle;
}

int bm_load_duplicate(const char *filename) {
	int ret;

	// ignore duplicates
	Bm_ignore_duplicates = 1;

	// load
	ret = bm_load(filename);

	// back to normal
	Bm_ignore_duplicates = 0;

	return ret;
}

int bm_load_either(const char *filename, int *nframes, int *fps, int *keyframe, bool can_drop_frames, int dir_type) {
	if (nframes != NULL)
		*nframes = 0;
	if (fps != NULL)
		*fps = 0;
	int tidx = bm_load_animation(filename, nframes, fps, keyframe, nullptr, can_drop_frames, dir_type);
	if (tidx == -1) {
		tidx = bm_load(filename);
		if (tidx != -1 && nframes != NULL)
			*nframes = 1;
	}

	return tidx;
}

int bm_load_sub_fast(const char *real_filename, int *handle, int dir_type, bool animated_type) {
	if (Bm_ignore_duplicates)
		return 0;

	for (auto& block : bm_blocks) {
		for (auto& slot : block) {
			auto& entry = slot.entry;
			if (entry.type == BM_TYPE_NONE)
				continue;

			if (entry.dir_type != dir_type)
				continue;

			bool animated = bm_is_anim(&entry);

			if (animated_type && !animated)
				continue;
			else if (!animated_type && animated)
				continue;

			if (!strextcmp(real_filename, entry.filename)) {
				entry.load_count++;
				*handle = entry.handle;
				return 1;
			}
		}
	}

	// not found to be loaded already
	return 0;
}

int bm_load_sub_slow(const char *real_filename, const int num_ext, const char **ext_list, CFILE **img_cfp, int dir_type) {
	auto res = cf_find_file_location_ext(real_filename, num_ext, ext_list, dir_type, false);

	// could not be found, or is invalid for some reason
	if (!res.found)
		return -1;

	CFILE *test = cfopen_special(res, "rb", dir_type);

	if (test != NULL) {
		if (img_cfp != NULL)
			*img_cfp = test;

		return res.extension_index;
	}

	// umm, that's not good...
	Warning(LOCATION, "Could not open file %s!", real_filename);
	return -1;
}

bitmap * bm_lock(int handle, int bpp, ushort flags, bool nodebug) {
	bitmap			*bmp;

	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	auto be = bm_get_entry(handle);

	// to fix a couple of OGL bpp passes, force 8bit on AABITMAP - taylor
	if (flags & BMP_AABITMAP)
		bpp = 8;

	// if we're on a standalone server, aways for it to lock to 8 bits
	if (Is_standalone) {
		bpp = 8;
		flags = 0;
	}
	// otherwise do it as normal
	else {
		if (flags & BMP_AABITMAP) {
			Assert(bpp == 8);
		} else if ((flags & BMP_TEX_NONCOMP) && (!(flags & BMP_TEX_COMP))) {
			Assert(bpp >= 16);  // cheating but bpp passed isn't what we normally end up with
		} else if ((flags & BMP_TEX_DXT1) || (flags & BMP_TEX_DXT3) || (flags & BMP_TEX_DXT5) || (flags & BMP_TEX_BC7)) {
			Assert(bpp >= 16); // cheating but bpp passed isn't what we normally end up with
		} else if (flags & BMP_TEX_CUBEMAP) {
			Assert((be->type == BM_TYPE_CUBEMAP_DDS) ||
				(be->type == BM_TYPE_CUBEMAP_DXT1) ||
				(be->type == BM_TYPE_CUBEMAP_DXT3) ||
				(be->type == BM_TYPE_CUBEMAP_DXT5));
			Assert(bpp >= 16);
		} else {
			Assert(0);		//?
		}
	}

	bmp = &be->bm;

	// If you hit this assert, chances are that someone freed the
	// wrong bitmap and now someone is trying to use that bitmap.
	// See John.
	Assert(be->type != BM_TYPE_NONE);

	// Increment ref count for bitmap since lock was made on it.
	Assert(be->ref_count >= 0);
	be->ref_count++;					// Lock it before we page in data; this prevents a callback from freeing this
	// as it gets read in

	// Mark this bitmap as used this frame
#ifdef BMPMAN_NDEBUG
	if (be->used_this_frame < 255) {
		be->used_this_frame++;
	}
#endif

	// read the file data
	if (bm_load_image_data(handle, bpp, flags, nodebug) == -1) {
		// oops, this isn't good - reset and return NULL
		bm_unlock( handle );
		bm_unload( handle );

		return NULL;
	}
	
	if (!gr_bm_data(handle, bmp)) {
		// graphics subsystem failed, reset and return NULL
		bm_unlock( handle );
		bm_unload( handle );

		return NULL;
	}

	MONITOR_INC(NumBitmapPage, 1);
	MONITOR_INC(SizeBitmapPage, bmp->w*bmp->h);

	if (bm_is_anim(be) == true) {
		int i;
		auto first = be->info.ani.first_frame;

		auto frames = bm_get_entry(first)->info.ani.num_frames;
		for (i = 0; i< frames; i++) {
			// Mark all the bitmaps in this bitmap or animation as recently used
			auto frame_entry = bm_get_entry(first + i);

#ifdef BMPMAN_NDEBUG
			// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
			frame_entry->used_count++;
#endif

			frame_entry->used_flags = flags;
		}
	} else {
#ifdef BMPMAN_NDEBUG
		// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
		be->used_count++;
#endif
		be->used_flags = flags;
	}

	return bmp;
}

void bm_lock_ani(int /*handle*/, bitmap_slot *bs, bitmap* /*bmp*/, int bpp, ushort flags) {
	anim				*the_anim;
	anim_instance	*the_anim_instance;
	bitmap			*bm;
	ubyte				*frame_data;
	int				size, i;
	int				first_frame, nframes;

	auto be = &bs->entry;
	first_frame = be->info.ani.first_frame;

	auto first_entry = bm_get_entry(first_frame);

	nframes = first_entry->info.ani.num_frames;

	if ((the_anim = anim_load(first_entry->filename, first_entry->dir_type)) == nullptr) {
		nprintf(("BMPMAN", "Error opening %s in bm_lock\n", be->filename));
		return;
	}

	if ((the_anim_instance = init_anim_instance(the_anim, bpp)) == nullptr) {
		nprintf(("BMPMAN", "Error opening %s in bm_lock\n", be->filename));
		anim_free(the_anim);
		return;
	}

	int can_drop_frames = 0;

	if (the_anim->total_frames != first_entry->info.ani.num_frames) {
		can_drop_frames = 1;
	}

	bm = &first_entry->bm;
	size = bm->w * bm->h * (bpp >> 3);
	be->mem_taken = (size_t)size;

	Assert(size > 0);

	for (i = 0; i<nframes; i++) {
		auto slot = bm_get_slot(first_frame + i);
		be = &slot->entry;
		bm = &be->bm;

		// Unload any existing data
		bm_free_data(slot);

		bm->flags = 0;

		// briefing editor in Fred2 uses aabitmaps (ani's) - force to 8 bit
		bm->bpp = Is_standalone ? (ubyte)8 : bpp;

		bm->data = (ptr_u)bm_malloc(first_frame + i, size);

		frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, flags & BMP_AABITMAP ? 1 : 0, bm->bpp);

		ubyte *dptr, *sptr;

		sptr = frame_data;
		dptr = (ubyte *)bm->data;

		if ((bm->w != the_anim->width) || (bm->h != the_anim->height)) {
			// Scale it down
			// 8 bit
			if (bpp == 8) {
				int w, h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = (the_anim->width*F1_0) / bm->w;
				dv = (the_anim->height*F1_0) / bm->h;

				for (h = 0; h < bm->h; h++) {
					ubyte *drow = &dptr[bm->w * h];
					ubyte *srow = &sptr[f2i(v)*the_anim->width];

					utmp = u;

					for (w = 0; w < bm->w; w++) {
						*drow++ = srow[f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}
			}
			// 16 bpp
			else {
				int w, h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = (the_anim->width*F1_0) / bm->w;
				dv = (the_anim->height*F1_0) / bm->h;

				for (h = 0; h < bm->h; h++) {
					unsigned short *drow = &((unsigned short*)dptr)[bm->w * h];
					unsigned short *srow = &((unsigned short*)sptr)[f2i(v)*the_anim->width];

					utmp = u;

					for (w = 0; w < bm->w; w++) {
						*drow++ = srow[f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}
			}
		} else {
			// 1-to-1 mapping
			memcpy(dptr, sptr, size);
		}

		bm_convert_format(bm, flags);

		// Skip a frame
		if ((i < nframes - 1) && can_drop_frames) {
			frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, flags & BMP_AABITMAP ? 1 : 0, bm->bpp);
		}
	}

	free_anim_instance(the_anim_instance);
	anim_free(the_anim);
}


void bm_lock_apng(int /*handle*/, bitmap_slot *bs, bitmap *bmp, int bpp, ushort /*flags*/) {
	auto be = &bs->entry;
	int first_frame = be->info.ani.first_frame;
	auto first_entry = bm_get_entry(first_frame);
	int nframes = first_entry->info.ani.num_frames;

	std::unique_ptr<apng::apng_ani> the_apng;
	try {
		the_apng.reset(new apng::apng_ani(first_entry->filename));
	}
	catch (const apng::ApngException& e) {
		Warning(LOCATION, "Failed to load apng: %s", e.what());
		return;
	}

	float cumulative_frame_delay = 0.0f;
	for (int i = 0; i<nframes; i++) {
		auto slot = bm_get_slot(first_frame + i);
		be = &slot->entry;
		bitmap* bm = &be->bm;

		// Unload any existing data
		bm_free_data(slot);

		ubyte* data = static_cast<ubyte*>(bm_malloc(first_frame + i, be->mem_taken));
		try {
			// this is a reasonably expensive operation
			// especially when run on all frames at once
			// i.e. try to preload the apng before using it
			the_apng->next_frame();
		}
		catch (const apng::ApngException& e) {
			Warning(LOCATION, "Failed to get next apng frame: %s", e.what());
			bm_release(first_frame);
			return;
		}
		memcpy(data, the_apng->frame.data.data(), be->mem_taken);

		bm->data = reinterpret_cast<ptr_u>(data);
		bm->palette = nullptr;
		bm->bpp = bpp;
		bm->flags = 0;
		cumulative_frame_delay += the_apng->frame.delay;
		be->info.ani.apng.frame_delay = cumulative_frame_delay;

		nprintf(("apng", "locking apng frame: %s (%i|%i|%i) (%f) " SIZE_T_ARG "\n", be->filename, bpp, bmp->bpp, bm->true_bpp, be->info.ani.apng.frame_delay, be->mem_taken));
	}
}


void bm_lock_dds(int handle, bitmap_slot *bs, bitmap *bmp, int /*bpp*/, ushort /*flags*/) {
	ubyte *data = NULL;
	int error;
	ubyte dds_bpp = 0;
	char filename[MAX_FILENAME_LEN];

	auto be = &bs->entry;

	// free any existing data
	bm_free_data(bs);

	Assert(be->mem_taken > 0);
	Assert(&be->bm == bmp);

	data = (ubyte*)bm_malloc(handle, be->mem_taken);

	if (data == NULL)
		return;

	memset(data, 0, be->mem_taken);

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	error = dds_read_bitmap(filename, data, &dds_bpp, be->dir_type);

#if BYTE_ORDER == BIG_ENDIAN
	// same as with TGA, we need to byte swap 16 & 32-bit, uncompressed, DDS images
	if ((be->comp_type == BM_TYPE_DDS) || (be->comp_type == BM_TYPE_CUBEMAP_DDS)) {
		size_t i = 0;

		if (dds_bpp == 32) {
			unsigned int *swap_tmp;

			for (i = 0; i < be->mem_taken; i += 4) {
				swap_tmp = (unsigned int *)(data + i);
				*swap_tmp = INTEL_INT(*swap_tmp);
			}
		} else if (dds_bpp == 16) {
			unsigned short *swap_tmp;

			for (i = 0; i < be->mem_taken; i += 2) {
				swap_tmp = (unsigned short *)(data + i);
				*swap_tmp = INTEL_SHORT(*swap_tmp);
			}
		}
	}
#endif

	bmp->bpp = dds_bpp;
	bmp->data = (ptr_u)data;
	bmp->flags = 0;

	if (error != DDS_ERROR_NONE) {
		bm_free_data(bs);
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert(be->data_size > 0);
#endif
}

void bm_lock_jpg(int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort /*flags*/) {
	ubyte *data = NULL;
	int d_size = 0;
	int jpg_error = JPEG_ERROR_INVALID;
	char filename[MAX_FILENAME_LEN];

	auto be = &bs->entry;

	// Unload any existing data
	bm_free_data(bs);

	// JPEG actually only support 24 bits per pixel so we enforce that here
	bpp = 24;

	d_size = (bpp >> 3);

	// allocate bitmap data
	Assert(be->mem_taken > 0);
	data = (ubyte*)bm_malloc(handle, be->mem_taken);

	if (data == NULL)
		return;

	memset(data, 0, be->mem_taken);

	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert(&be->bm == bmp);

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	jpg_error = jpeg_read_bitmap(filename, data, NULL, d_size, be->dir_type);

	if (jpg_error != JPEG_ERROR_NONE) {
		bm_free_data(bs);
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert(be->data_size > 0);
#endif
}

void bm_lock_pcx(int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags) {
	ubyte *data;
	int pcx_error;
	char filename[MAX_FILENAME_LEN];

	auto be = &bs->entry;

	// Unload any existing data
	bm_free_data(bs);

	be->mem_taken = (bmp->w * bmp->h * (bpp >> 3));
	data = (ubyte *)bm_malloc(handle, be->mem_taken);
	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;
	memset(data, 0, be->mem_taken);

	Assert(&be->bm == bmp);
#ifdef BMPMAN_NDEBUG
	Assert(be->data_size > 0);
#endif

	// some sanity checks on flags
	Assert(!((flags & BMP_AABITMAP) && (flags & BMP_TEX_ANY)));						// no aabitmap textures

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	pcx_error = pcx_read_bitmap(filename, data, NULL, (bpp >> 3), (flags & BMP_AABITMAP), (flags & BMP_MASK_BITMAP) != 0, be->dir_type);

	if (pcx_error != PCX_ERROR_NONE) {
		mprintf(("Couldn't load PCX!!! (%s)\n", filename));
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert(be->data_size > 0);
#endif

	bmp->flags = 0;

	bm_convert_format(bmp, flags);
}

void bm_lock_png(int handle, bitmap_slot *bs, bitmap *bmp, int /*bpp*/, ushort /*flags*/) {
	ubyte *data = NULL;
	//assume 32 bit - libpng should expand everything
	int d_size;
	int png_error = PNG_ERROR_INVALID;
	char filename[MAX_FILENAME_LEN];

	auto be = &bs->entry;

	// Unload any existing data
	bm_free_data(bs);

	// allocate bitmap data
	Assert(bmp->w * bmp->h > 0);

	//if it's not 32-bit, we expand when we read it
	bmp->bpp = 32;
	d_size = bmp->bpp >> 3;
	//we waste memory if it turns out to be 24-bit, but the way this whole thing works is dodgy anyway
	data = (ubyte*)bm_malloc(handle, bmp->w * bmp->h * d_size);
	if (data == NULL)
		return;
	memset(data, 0, bmp->w * bmp->h * d_size);
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert(&be->bm == bmp);

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	//bmp->bpp gets set correctly in here after reading into memory
	png_error = png_read_bitmap(filename, data, &bmp->bpp, d_size, be->dir_type);

	if (png_error != PNG_ERROR_NONE) {
		bm_free_data(bs);
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert(be->data_size > 0);
#endif
}

void bm_lock_tga(int handle, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags) {
	ubyte *data = NULL;
	int byte_size;
	char filename[MAX_FILENAME_LEN];

	auto be = &bs->entry;

	// Unload any existing data
	bm_free_data(bs);

	bpp = be->bm.true_bpp;

	if (Is_standalone) {
		Assert(bpp == 8);
	} else {
		Assert((bpp == 16) || (bpp == 24) || (bpp == 32));
	}

	// allocate bitmap data
	byte_size = (bpp >> 3);

	Assert(byte_size);
	Assert(be->mem_taken > 0);

	data = (ubyte*)bm_malloc(handle, static_cast<size_t>(bmp->w * bmp->h * byte_size));

	if (data) {
		memset(data, 0, be->mem_taken);
	} else {
		return;
	}

	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert(&be->bm == bmp);
#ifdef BMPMAN_NDEBUG
	Assert(be->data_size > 0);
#endif

	int tga_error;

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	tga_error = targa_read_bitmap(filename, data, nullptr, byte_size, be->dir_type);

	if (tga_error != TARGA_ERROR_NONE) {
		bm_free_data(bs);
		return;
	}

	bmp->flags = 0;

	bm_convert_format(bmp, flags);
}

void bm_lock_user(int /*handle*/, bitmap_slot *bs, bitmap *bmp, int bpp, ushort flags) {
	auto be = &bs->entry;

	// Unload any existing data
	bm_free_data(bs);

	if ((bpp != be->info.user.bpp) && !(flags & BMP_AABITMAP))
		bpp = be->info.user.bpp;

	switch (bpp) {
	case 32:	// user 32-bit bitmap
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;
		bmp->data = (ptr_u)be->info.user.data;
		break;

	case 24:	// user 24-bit bitmap
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;
		bmp->data = (ptr_u)be->info.user.data;
		break;

	case 16:			// user 16 bit bitmap
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;
		bmp->data = (ptr_u)be->info.user.data;
		break;

	case 8:			// Going from 8 bpp to something (probably only for aabitmaps)
		Assert(flags & BMP_AABITMAP);
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;
		bmp->data = (ptr_u)be->info.user.data;
		break;

	default:
		Error(LOCATION, "Unhandled user bitmap conversion from %d to %d bpp", be->info.user.bpp, bmp->bpp);
		break;
	}

	bm_convert_format(bmp, flags);
}

int bm_make_render_target(int width, int height, int flags) {
	int mm_lvl = 0;
	// final w and h may be different from passed width and height
	int w = width, h = height;
	int bpp = 32;
	int size = 0;

	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	// Find an open slot (starting from the end)
	int n = find_block_of(1);

	// Out of bitmap slots
	if (n == -1)
		return -1;

	if (!gr_bm_make_render_target(n, &w, &h, &bpp, &mm_lvl, flags))
		return -1;

	Assert(mm_lvl > 0);

	if (flags & BMP_FLAG_RENDER_TARGET_STATIC) {
		// data size
		size = (w * h * (bpp >> 3));

		if (mm_lvl > 1) {
			// include size of all mipmap levels (there should be a full chain)
			size += (size / 3) - 1;
		}

		// make sure to count all faces if a cubemap
		if (flags & BMP_FLAG_CUBEMAP) {
			size *= 6;
		}
	}

	auto entry = bm_get_entry(n);
	// ensure fields are cleared out from previous bitmap
	memset(entry, 0, sizeof(bitmap_entry));

	entry->type = (flags & BMP_FLAG_RENDER_TARGET_STATIC) ? BM_TYPE_RENDER_TARGET_STATIC : BM_TYPE_RENDER_TARGET_DYNAMIC;
	entry->signature = Bm_next_signature++;
	sprintf_safe(entry->filename, "RT_%dx%d+%d", w, h, bpp);
	entry->bm.w = (short)w;
	entry->bm.h = (short)h;
	entry->bm.rowsize = (short)w;
	entry->bm.bpp = bpp;
	entry->bm.true_bpp = bpp;
	entry->bm.flags = (ubyte)flags;
	entry->bm.data = 0;
	entry->bm.palette = nullptr;
	entry->num_mipmaps = mm_lvl;
	entry->mem_taken = (size_t)size;
	entry->dir_type = CF_TYPE_ANY;

	entry->handle = n;

	if (entry->mem_taken) {
		entry->bm.data = (ptr_u)bm_malloc(n, entry->mem_taken);
	}

	return entry->handle;
}

void *bm_malloc(int n, size_t size) {
	if (size == 0)
		return nullptr;

#ifdef BMPMAN_NDEBUG
	auto entry = bm_get_entry(n);
	Assert(entry->data_size == 0);
	entry->data_size += size;
	bm_texture_ram += size;
#endif

	return vm_malloc(size);
}

void bm_page_in_aabitmap(int handle, int nframes) {
	int i;

	if (handle == -1)
		return;

	Assert(bm_get_entry(handle)->handle == handle);

	for (i = 0; i<nframes; i++) {
		auto frame_entry = bm_get_entry(handle + i);

		frame_entry->preloaded = 2;
		frame_entry->preload_count++;
		frame_entry->used_flags = BMP_AABITMAP;
	}
}

void bm_page_in_start() {
	Bm_paging = 1;

	// Mark all as inited
	for (auto& block : bm_blocks) {
		for (auto& slot : block) {
			auto& entry = slot.entry;

			entry.preloaded = 0;
			entry.preload_count = 0;
#ifdef BMPMAN_NDEBUG
			entry.used_count = 0;
#endif
			entry.used_flags = 0;
		}
	}

	gr_bm_page_in_start();
}

void bm_page_in_stop() {
	TRACE_SCOPE(tracing::PageInStop);

#ifndef NDEBUG
	char busy_text[60];
#endif

	nprintf(("BmpInfo", "BMPMAN: Loading all used bitmaps.\n"));

	// Load all the ones that are supposed to be loaded for this level.
	int n = 0;

	int bm_preloading = 1;

	for (auto& block : bm_blocks) {
		for (auto& slot : block) {
			auto& entry = slot.entry;

			if ((entry.type != BM_TYPE_NONE) && (entry.type != BM_TYPE_RENDER_TARGET_DYNAMIC)
				&& (entry.type != BM_TYPE_RENDER_TARGET_STATIC)) {
				if (entry.preloaded) {
					TRACE_SCOPE(tracing::PageInSingleBitmap);
					if (bm_preloading) {
						if (!gr_preload(entry.handle, (entry.preloaded == 2))) {
							mprintf(("Out of VRAM.  Done preloading.\n"));
							bm_preloading = 0;
						}
					} else {
						bm_lock(entry.handle, (entry.used_flags == BMP_AABITMAP) ? 8 : 16, entry.used_flags);
						if (entry.ref_count >= 1) {
							bm_unlock(entry.handle);
						}
					}

					n++;

					multi_send_anti_timeout_ping();

					if ((entry.info.ani.first_frame == 0) || (entry.info.ani.first_frame == entry.handle)) {
#ifndef NDEBUG
						memset(busy_text, 0, sizeof(busy_text));

						strcat_s(busy_text, "** BmpMan: ");
						strcat_s(busy_text, entry.filename);
						strcat_s(busy_text, " **");

						game_busy(busy_text);
#else
						game_busy();
#endif
					}
				} else {
					bm_unload_fast(entry.handle);
				}
			}
		}
	}

	nprintf(("BmpInfo", "BMPMAN: Loaded %d bitmaps that are marked as used for this level.\n", n));

#ifndef NDEBUG
	int total_bitmaps = 0;
	int total_slots = 0;
	for (auto& block : bm_blocks) {
		total_slots += BM_BLOCK_SIZE;
		for (auto& slot : block) {
			auto& entry = slot.entry;

			if (entry.type != BM_TYPE_NONE) {
				total_bitmaps++;
			}
			if (entry.type == BM_TYPE_USER) {
				mprintf(("User bitmap '%s'\n", entry.filename));
			}
		}
	}

	mprintf(("Bmpman: %d/%d bitmap slots in use.\n", total_bitmaps, total_slots));
#endif

	Bm_paging = 0;
}

void bm_page_in_texture(int bitmapnum, int nframes) {
	int i;

	if (bitmapnum < 0)
		return;

	auto entry = bm_get_entry(bitmapnum);

	if (nframes <= 0) {
		if (bm_is_anim(entry) == true)
			nframes = entry->info.ani.num_frames;
		else
			nframes = 1;
	}

	for (i = 0; i < nframes; i++) {
		auto frame_entry = bm_get_entry(bitmapnum + i);

		frame_entry->preloaded = 1;

		frame_entry->preload_count++;

		frame_entry->used_flags = BMP_TEX_OTHER;

		//check if its compressed
		switch (frame_entry->comp_type) {
		case BM_TYPE_NONE:
			continue;

		case BM_TYPE_DXT1:
			frame_entry->used_flags = BMP_TEX_DXT1;
			continue;

		case BM_TYPE_DXT3:
			frame_entry->used_flags = BMP_TEX_DXT3;
			continue;

		case BM_TYPE_DXT5:
			frame_entry->used_flags = BMP_TEX_DXT5;
			continue;

		case BM_TYPE_BC7:
			frame_entry->used_flags = BMP_TEX_BC7;
			continue;

		case BM_TYPE_CUBEMAP_DXT1:
		case BM_TYPE_CUBEMAP_DXT3:
		case BM_TYPE_CUBEMAP_DXT5:
			frame_entry->used_flags = BMP_TEX_CUBEMAP;
			continue;

		default:
			continue;
		}
	}
}

void bm_page_in_xparent_texture(int bitmapnum, int nframes) {
	int i;

	for (i = 0; i < nframes; i++) {
		auto entry = bm_get_entry(bitmapnum + i);

		entry->preloaded = 3;

		entry->preload_count++;

		entry->used_flags = BMP_TEX_XPARENT;

		//check if its compressed
		switch (entry->comp_type) {
		case BM_TYPE_NONE:
			continue;

		case BM_TYPE_DXT1:
			entry->used_flags = BMP_TEX_DXT1;
			continue;

		case BM_TYPE_DXT3:
			entry->used_flags = BMP_TEX_DXT3;
			continue;

		case BM_TYPE_DXT5:
			entry->used_flags = BMP_TEX_DXT5;
			continue;

		case BM_TYPE_BC7:
			entry->used_flags = BMP_TEX_BC7;
			continue;

		case BM_TYPE_CUBEMAP_DXT1:
		case BM_TYPE_CUBEMAP_DXT3:
		case BM_TYPE_CUBEMAP_DXT5:
			entry->used_flags = BMP_TEX_CUBEMAP;
			continue;

		default:
			continue;
		}
	}
}

void bm_print_bitmaps() {
#ifdef BMPMAN_NDEBUG
	for (auto& block : bm_blocks) {
		for (auto& slot : block) {
			auto& entry = slot.entry;

			if (entry.type != BM_TYPE_NONE) {
				if (entry.data_size) {
					nprintf(("BMP DEBUG", "BMPMAN = num: %d, name: %s, handle: %d - (%s) size: %.3fM\n", entry.handle, entry.filename, entry.handle, entry.data_size
																																					 ? NOX(
							"*LOCKED*") : NOX(""), ((float) entry.data_size / 1024.0f) / 1024.0f));
				} else {
					nprintf(("BMP DEBUG", "BMPMAN = num: %d, name: %s, handle: %d\n", entry.handle, entry.filename, entry.handle));
				}
			}
		}
	}

	nprintf(("BMP DEBUG", "BMPMAN = LOCKED memory usage: %.3fM\n", ((float) bm_texture_ram / 1024.0f) / 1024.0f));
#endif
}

int bm_release(int handle, int clear_render_targets) {
	Assert(handle >= 0);

	bitmap_entry *be;

	be = bm_get_entry(handle);

	if (be->type == BM_TYPE_NONE) {
		return 0;	// Already been released?
	}

	Assertion(be->handle == handle, "Invalid bitmap handle number %d (expected %d) for %s passed to bm_release()\n", be->handle, handle, be->filename);

	if (!clear_render_targets && ((be->type == BM_TYPE_RENDER_TARGET_STATIC) || (be->type == BM_TYPE_RENDER_TARGET_DYNAMIC))) {
		nprintf(("BmpMan", "Tried to release a render target!\n"));
		return 0;
	}

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "Tried to release %s that has a lock count of %d.. not releasing\n", be->filename, be->ref_count));
		return 0;
	}

	// kind of like ref_count except it gets around the lock/unlock usage problem
	// this gets set for each bm_load() call so we can make sure and not unload it
	// from memory, even if we *can*, until it's really not needed anymore
	if (be->load_count > 0)
		be->load_count--;

	if (be->load_count != 0) {
		nprintf(("BmpMan", "Tried to release %s that has a load count of %d.. not releasing\n", be->filename, be->load_count + 1));
		return 0;
	}

	if (be->type != BM_TYPE_USER) {
		nprintf(("BmpMan", "Releasing bitmap %s with handle %i\n", be->filename, handle));
	}

	// be sure that all frames of an ani are unloaded - taylor
	if (bm_is_anim(be)) {
		int i, first = be->info.ani.first_frame;
		int total = bm_get_entry(first)->info.ani.num_frames;

		// The graphics system requires that the bitmap information for the whole animation is still valid when the data
		// is freed so we first free all the data and then clear the bitmaps data
		for (i = 0; i < total; i++) {
			bm_free_data(bm_get_slot(first + i), true);
		}

		for (i = 0; i < total; i++) {
			auto entry = bm_get_entry(first + i);

			memset(entry, 0, sizeof(bitmap_entry));

			entry->type = BM_TYPE_NONE;
			entry->comp_type = BM_TYPE_NONE;
			entry->dir_type = CF_TYPE_ANY;
			// Fill in bogus structures!

			// For debugging:
			strcpy_s(entry->filename, "IVE_BEEN_RELEASED!");
			entry->signature = 0xDEADBEEF;									// a unique signature identifying the data

			// bookeeping
			entry->ref_count = -1;									// Number of locks on bitmap.  Can't unload unless ref_count is 0.

			// Stuff needed for animations
			entry->info.ani.first_frame = -1;

			entry->handle = -1;
		}
	} else {
		auto slot = bm_get_slot(handle);
		auto entry = &slot->entry;

		bm_free_data(slot, true);		// clears flags, bbp, data, etc


		memset(entry, 0, sizeof(bitmap_entry));

		entry->type = BM_TYPE_NONE;
		entry->comp_type = BM_TYPE_NONE;
		entry->dir_type = CF_TYPE_ANY;
		// Fill in bogus structures!

		// For debugging:
		strcpy_s(entry->filename, "IVE_BEEN_RELEASED!");
		entry->signature = 0xDEADBEEF;									// a unique signature identifying the data

		// bookeeping
		entry->ref_count = -1;									// Number of locks on bitmap.  Can't unload unless ref_count is 0.

		// Stuff needed for animations
		entry->info.ani.first_frame = -1;

		entry->handle = -1;
	}

	return 1;
}

bool bm_release_rendertarget(int handle) {
	Assert(handle >= 0);

	bitmap_entry* be = bm_get_entry(handle);

	if (be->type == BM_TYPE_NONE) {
		return false;	// Already been released?
	}

	Assertion(be->handle == handle, "Invalid bitmap handle number %d (expected %d) for %s passed to bm_release_rendertarget()\n", be->handle, handle, be->filename);
	Assertion(!bm_is_anim(be), "Cannot release a render target of an animation (bitmap handle number %d for %s)!\n", be->handle, be->filename);

	if (!((be->type == BM_TYPE_RENDER_TARGET_STATIC) || (be->type == BM_TYPE_RENDER_TARGET_DYNAMIC))) {
		nprintf(("BmpMan", "Tried to release a render target of a non-rendered bitmap!\n"));
		return false;
	}

	gr_bm_free_data(bm_get_slot(handle), false);

	return true;
}

int bm_reload(int bitmap_handle, const char* filename) {
	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	// if no file was passed then get out now
	if ((filename == NULL) || (strlen(filename) <= 0))
		return -1;

	auto entry = bm_get_entry(bitmap_handle);

	if (entry->type == BM_TYPE_NONE)
		return -1;

	if (entry->ref_count) {
		nprintf(("BmpMan", "Trying to reload a bitmap that is still locked. Filename: %s, ref_count: %d", entry->filename, entry->ref_count));
		return -1;
	}

	strcpy_s(entry->filename, filename);
	return bitmap_handle;
}

void BM_SELECT_ALPHA_TEX_FORMAT() {
	Gr_current_red = &Gr_ta_red;
	Gr_current_green = &Gr_ta_green;
	Gr_current_blue = &Gr_ta_blue;
	Gr_current_alpha = &Gr_ta_alpha;

	// setup pointers
	// should always assume that 16-bit is the default request
	bm_set_components = bm_set_components_argb_16_tex;
}

void BM_SELECT_SCREEN_FORMAT() {
	Gr_current_red = &Gr_red;
	Gr_current_green = &Gr_green;
	Gr_current_blue = &Gr_blue;
	Gr_current_alpha = &Gr_alpha;

	// setup pointers
	// should always assume that 16-bit is the default request
	bm_set_components = bm_set_components_argb_16_screen;
}

void BM_SELECT_TEX_FORMAT() {
	Gr_current_red = &Gr_t_red;
	Gr_current_green = &Gr_t_green;
	Gr_current_blue = &Gr_t_blue;
	Gr_current_alpha = &Gr_t_alpha;

	// setup pointers
	// should always assume that 16-bit is the default request
	bm_set_components = bm_set_components_argb_16_tex;
}

void bm_set_components_argb_16_screen(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av) {
	if (*av == 0) {
		*((unsigned short*)pixel) = (unsigned short)Gr_current_green->mask;
		return;
	}

	*((unsigned short*)pixel) = (unsigned short)(((int)*rv / Gr_current_red->scale) << Gr_current_red->shift);
	*((unsigned short*)pixel) |= (unsigned short)(((int)*gv / Gr_current_green->scale) << Gr_current_green->shift);
	*((unsigned short*)pixel) |= (unsigned short)(((int)*bv / Gr_current_blue->scale) << Gr_current_blue->shift);
}

void bm_set_components_argb_16_tex(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av) {
	if (*av == 0) {
		*((unsigned short*)pixel) = 0;
		return;
	}

	*((unsigned short*)pixel) = (unsigned short)(((int)*rv / Gr_current_red->scale) << Gr_current_red->shift);
	*((unsigned short*)pixel) |= (unsigned short)(((int)*gv / Gr_current_green->scale) << Gr_current_green->shift);
	*((unsigned short*)pixel) |= (unsigned short)(((int)*bv / Gr_current_blue->scale) << Gr_current_blue->shift);
	*((unsigned short*)pixel) |= (unsigned short)(Gr_current_alpha->mask);
}

void bm_set_low_mem(int mode) {
	Assert((mode >= 0) && (mode <= 2));

	CLAMP(mode, 0, 2);
	Bm_low_mem = mode;
}

bool bm_set_render_target(int handle, int face) {
	GR_DEBUG_SCOPE("Set render target");

	auto entry = handle >= 0 ? bm_get_entry(handle) : nullptr;

	if (handle >= 0) {
		if ((entry->type != BM_TYPE_RENDER_TARGET_STATIC) && (entry->type != BM_TYPE_RENDER_TARGET_DYNAMIC)) {
			// odds are that someone passed a normal texture created with bm_load()
			mprintf(("Trying to set invalid bitmap (handle: %i) as render target!\n", handle));
			return false;
		}
	}

	if (gr_bm_set_render_target(handle, face)) {
		if (gr_screen.rendering_to_texture == -1) {
			//if we are moving from the back buffer to a texture save whatever the current settings are
			gr_screen.save_max_w = gr_screen.max_w;
			gr_screen.save_max_h = gr_screen.max_h;

			gr_screen.save_max_w_unscaled = gr_screen.max_w_unscaled;
			gr_screen.save_max_h_unscaled = gr_screen.max_h_unscaled;

			gr_screen.save_max_w_unscaled_zoomed = gr_screen.max_w_unscaled_zoomed;
			gr_screen.save_max_h_unscaled_zoomed = gr_screen.max_h_unscaled_zoomed;

			gr_screen.save_center_w = gr_screen.center_w;
			gr_screen.save_center_h = gr_screen.center_h;

			gr_screen.save_center_offset_x = gr_screen.center_offset_x;
			gr_screen.save_center_offset_y = gr_screen.center_offset_y;
		}

		if (handle < 0) {
			gr_screen.max_w = gr_screen.save_max_w;
			gr_screen.max_h = gr_screen.save_max_h;

			gr_screen.max_w_unscaled = gr_screen.save_max_w_unscaled;
			gr_screen.max_h_unscaled = gr_screen.save_max_h_unscaled;

			gr_screen.max_w_unscaled_zoomed = gr_screen.save_max_w_unscaled_zoomed;
			gr_screen.max_h_unscaled_zoomed = gr_screen.save_max_h_unscaled_zoomed;

			gr_screen.center_w = gr_screen.save_center_w;
			gr_screen.center_h = gr_screen.save_center_h;

			gr_screen.center_offset_x = gr_screen.save_center_offset_x;
			gr_screen.center_offset_y = gr_screen.save_center_offset_y;
		} else {
			gr_screen.max_w = entry->bm.w;
			gr_screen.max_h = entry->bm.h;

			gr_screen.max_w_unscaled = entry->bm.w;
			gr_screen.max_h_unscaled = entry->bm.h;

			gr_screen.max_w_unscaled_zoomed = entry->bm.w;
			gr_screen.max_h_unscaled_zoomed = entry->bm.h;

			gr_screen.center_w = entry->bm.w;
			gr_screen.center_h = entry->bm.h;

			gr_screen.center_offset_x = 0;
			gr_screen.center_offset_y = 0;
		}

		gr_screen.rendering_to_face = face;
		gr_screen.rendering_to_texture = handle;

		gr_reset_clip();

		gr_setup_viewport();

		return true;
	}

	return false;
}

int bm_unload(int handle, int clear_render_targets, bool nodebug) {
	bitmap_entry *be;
	bitmap *bmp;

	if (handle == -1) {
		return -1;
	}

	be = bm_get_entry(handle);
	bmp = &be->bm;

	if (!clear_render_targets && ((be->type == BM_TYPE_RENDER_TARGET_STATIC) || (be->type == BM_TYPE_RENDER_TARGET_DYNAMIC))) {
		return -1;
	}

	if (be->type == BM_TYPE_NONE) {
		return -1;		// Already been released
	}

	Assert(be->handle == handle);		// INVALID BITMAP HANDLE!

	// If it is locked, cannot free it.
	if (be->ref_count != 0 && !nodebug) {
		nprintf(("BmpMan", "Tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return 0;
	}

	// kind of like ref_count except it gets around the lock/unlock usage problem
	// this gets set for each bm_load() call so we can make sure and not unload it
	// from memory, even if we *can*, until it's really not needed anymore
	if (!Bm_ignore_load_count) {
		if (be->load_count > 0)
			be->load_count--;

		if (be->load_count != 0 && !nodebug) {
			nprintf(("BmpMan", "Tried to unload %s that has a load count of %d.. not unloading\n", be->filename, be->load_count + 1));
			return 0;
		}
	}

	// be sure that all frames of an ani are unloaded - taylor
	if (bm_is_anim(be) == true) {
		int i, first = be->info.ani.first_frame;

		auto first_entry = bm_get_entry(first);

		// for the unload all case, don't try to unload every frame of every frame
		// all additional frames automatically get unloaded with the first one
		if ((handle != be->info.ani.first_frame) && (first_entry->bm.data == 0))
			return 1;

		for (i = 0; i < first_entry->info.ani.num_frames; i++) {
			if (!nodebug)
				nprintf(("BmpMan", "Unloading %s frame %d.  %dx%dx%d\n", be->filename, i, bmp->w, bmp->h, bmp->bpp));
			bm_free_data(bm_get_slot(first + i));		// clears flags, bbp, data, etc
		}
	} else {
		if (!nodebug)
			nprintf(("BmpMan", "Unloading %s.  %dx%dx%d\n", be->filename, bmp->w, bmp->h, bmp->bpp));
		bm_free_data(bm_get_slot(handle));		// clears flags, bbp, data, etc
	}

	return 1;
}

void bm_unload_all() {
	// since bm_unload_all() should only be called from game_shutdown() it should be
	// safe to ignore load_count's and unload anyway
	Bm_ignore_load_count = 1;
	for(auto& block : bm_blocks) {
		for (auto& slot : block) {
			auto& entry = slot.entry;

			if (entry.type != BM_TYPE_NONE) {
				bm_unload(entry.handle, 1);
			}
		}
	}

	Bm_ignore_load_count = 0;
}

int bm_unload_fast(int handle, int clear_render_targets) {
	bitmap_entry *be;
	bitmap *bmp;

	be = bm_get_entry(handle);
	bmp = &be->bm;

	if (be->type == BM_TYPE_NONE) {
		return -1;		// Already been released
	}

	if (be->type == BM_TYPE_USER) {
		return -1;
	}

	if (!clear_render_targets && ((be->type == BM_TYPE_RENDER_TARGET_STATIC) || (be->type == BM_TYPE_RENDER_TARGET_DYNAMIC))) {
		return -1;
	}

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "Tried to unload_fast %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return 0;
	}

	Assert(be->handle == handle);		// INVALID BITMAP HANDLE!

	// unlike bm_unload(), we handle each frame of an animation separately, for safer use in the graphics API
	nprintf(("BmpMan", "Fast-unloading %s.  %dx%dx%d\n", be->filename, bmp->w, bmp->h, bmp->bpp));
	bm_free_data_fast(handle);		// clears flags, bbp, data, etc

	return 1;
}

void bm_unlock(int handle) {
	bitmap_entry	*be;

	Assertion(bm_inited, "bmpman must be initialized before this function can be called!");

	be = bm_get_entry(handle);

	be->ref_count--;
	Assert(be->ref_count >= 0);		// Trying to unlock data more times than lock was called!!!
}

void bm_update_memory_used(int n, size_t size)
{
#ifdef BMPMAN_NDEBUG
	auto entry = bm_get_entry(n);
	Assert( entry->data_size == 0 );
	entry->data_size += size;
	bm_texture_ram += size;
#endif
}

static int find_block_of(int n, int start_block)
{
	Assertion(n < (int) BM_BLOCK_SIZE, "Can not allocate bitmap block with %d slots! Block size is only "
		SIZE_T_ARG
		"!", n, BM_BLOCK_SIZE);

	int nstart = 0;

	if (n < 1) {
		Int3();
		return -1;
	}

	int block_idx;
	for (block_idx = start_block; block_idx < (int)bm_blocks.size(); ++block_idx) {
		auto& block = bm_blocks[block_idx];

		auto entry_idx = 0;
		auto cnt = 0;
		for (auto& slot : block) {
			auto& entry = slot.entry;

			if (entry.type == BM_TYPE_NONE) {
				if (cnt == 0) {
					nstart = get_handle(block_idx, entry_idx);
				}

				cnt++;
			} else {
				cnt = 0;
			}

			if (cnt == n) {
				return nstart;
			}

			++entry_idx;
		}
	}

	// If we are here it means that we could not find a block to store the bitmap blocks in.
	// In that case we just allocate a new block and try to allocate the block in that
	allocate_new_block();

	// This call is sure to succeed since we now have a contiguous block of size BM_BLOCK_SIZE and n must be less than that
	return find_block_of(n, block_idx);
}

bool bm_is_texture_array(const int handle) {
	auto entry = bm_get_entry(handle);

	if (!bm_is_anim(entry)) {
		return true;
	}

	// This will return the index of the first animation frame
	return entry->info.ani.is_array;
}

int bm_get_base_frame(const int handle, int* num_frames) {
	if (handle == -1) {
		if (num_frames != nullptr) {
			*num_frames = 0;
		}
		return -1;
	}

	// If the bitmap is no animation then num_frames will still be at least 1
	auto animation_begin = bm_get_info(handle, nullptr, nullptr, nullptr, num_frames);

	if (animation_begin < 0) {
		// Some kind of error
		return -1;
	}

	if (!bm_is_texture_array(animation_begin)) {
		// If this is not a texture array then we just treat it like a single frame animation
		animation_begin = handle;
		if (num_frames != nullptr) {
			*num_frames = 1;
		}
	}
	return animation_begin;
}
int bm_get_array_index(const int handle) {
	return handle - bm_get_base_frame(handle, nullptr);
}

int bmpman_count_bitmaps() {
	if (!bm_inited)
		return -1;

	int total_bitmaps = 0;
	for(auto& block : bm_blocks) {
		for (auto& slot : block) {
			auto& entry = slot.entry;

			if (entry.type != BM_TYPE_NONE) {
				total_bitmaps++;
			}
		}
	}

	return total_bitmaps;
}

int bmpman_count_available_slots() {
	if (!bm_inited)
		return -1;

	return (int) (bm_blocks.size() * BM_BLOCK_SIZE);
}

bool bm_validate_filename(const SCP_string& file, bool single_frame, bool animation) {
	Assertion(single_frame || animation, "At least one of single_frame or animation must be true!");

	if (!VALID_FNAME(file.c_str())) {
		return false;
	}

	int handle = -1;
	if (single_frame && animation) {
		// Caller has indicated that single frames and animations are valid
		handle = bm_load_either(file.c_str());
	} else if (single_frame) {
		handle = bm_load(file);
	} else {
		handle = bm_load_animation(file.c_str());
	}
	if (handle != -1) {
		bm_release(handle);
		return true;
	}
	return false;
}
SDL_Surface* bm_to_sdl_surface(int handle) {
	Assertion(bm_is_valid(handle), "%d is no valid bitmap handle!", handle);

	int w;
	int h;

	bm_get_info(handle, &w, &h, nullptr, nullptr);
	Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x0000ff00;
		gmask = 0x00ff0000;
		bmask = 0xff000000;
		amask = 0x000000ff;
#else
	rmask = 0x00ff0000;
	gmask = 0x0000ff00;
	bmask = 0x000000ff;
	amask = 0xff000000;
#endif

	SDL_Surface* bitmapSurface = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
	if (SDL_LockSurface(bitmapSurface) < 0) {
		return nullptr;
	}
	bitmap* bmp = bm_lock(handle, 32, BMP_TEX_XPARENT);

	memcpy(bitmapSurface->pixels, reinterpret_cast<void*>(bmp->data), static_cast<size_t>(w * h * 4));

	bm_unlock(handle);
	SDL_UnlockSurface(bitmapSurface);

	return bitmapSurface;

}

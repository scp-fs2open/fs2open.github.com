/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <ctype.h>

#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <windows.h>	// for MAX_PATH
#endif

#include "bmpman/bmpman.h"
#include "pcxutils/pcxutils.h"
#include "palman/palman.h"
#include "graphics/2d.h"
#include "anim/animplay.h"
#include "io/timer.h"
#include "globalincs/systemvars.h"
#include "io/key.h"
#include "anim/packunpack.h"
#include "graphics/grinternal.h"
#include "tgautils/tgautils.h"
#include "ship/ship.h"
#include "ddsutils/ddsutils.h"
#include "cfile/cfile.h"
#include "pngutils/pngutils.h"
#include "jpgutils/jpgutils.h"
#include "parse/parselo.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

extern int Cmdline_cache_bitmaps;

#ifndef NDEBUG
#define BMPMAN_NDEBUG
#endif


// globals
int GLOWMAP = -1;
int SPECMAP = -1;
int ENVMAP = -1;
int NORMMAP = -1;
int HEIGHTMAP = -1;

bitmap_entry bm_bitmaps[MAX_BITMAPS];

int bm_texture_ram = 0;
int bm_inited = 0;
int Bm_paging = 0;

// locals
static unsigned int Bm_next_signature = 0x1234;
static int bm_next_handle = 0;
int Bm_low_mem = 0;
// Bm_max_ram - How much RAM bmpman can use for textures.
// Set to <1 to make it use all it wants.
int Bm_max_ram = 0;		//16*1024*1024;			// Only use 16 MB for textures
static int Bm_ignore_duplicates = 0;
static int Bm_ignore_load_count = 0;

#define EFF_FILENAME_CHECK { if ( be->type == BM_TYPE_EFF ) strncpy( filename, be->info.ani.eff.filename, MAX_FILENAME_LEN ); else strncpy( filename, be->filename, MAX_FILENAME_LEN ); }



// ===========================================
// Mode: 0 = High memory
//       1 = Low memory ( every other frame of ani's)
//       2 = Debug low memory ( only use first frame of each ani )
void bm_set_low_mem( int mode )
{
	Assert( (mode >= 0)  && (mode<=2 ));
	Bm_low_mem = mode;
}


int bm_get_next_handle()
{
	int n = bm_next_handle;
	bm_next_handle++;
	if ( bm_next_handle > 30000 )	{
		bm_next_handle = 1;
	}
	return n;
}

// Frees a bitmaps data if it should, and
// Returns true if bitmap n can free it's data.
static void bm_free_data(int n, bool release = false)
{
	bitmap_entry *be;
	bitmap *bmp;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	be = &bm_bitmaps[n];
	bmp = &be->bm;

	gr_bm_free_data(n, release);

	// If there isn't a bitmap in this structure, don't
	// do anything but clear out the bitmap info
	if ( be->type==BM_TYPE_NONE) 
		goto SkipFree;

	// Don't free up memory for user defined bitmaps, since
	// BmpMan isn't the one in charge of allocating/deallocing them.
	if ( ( be->type==BM_TYPE_USER ) ) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
#endif
		goto SkipFree;
	}

	// If this bitmap doesn't have any data to free, skip
	// the freeing it part of this.
	if ( (bmp->data == 0) ) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
#endif
		goto SkipFree;
	}

	// Free up the data now!

	//	mprintf(( "Bitmap %d freed %d bytes\n", n, bm_bitmaps[n].data_size ));
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

// a special version of bm_free_data() that can be safely used in gr_*_texture
// to save system memory once textures have been transfered to API memory
// it doesn't restore the slot to a pristine state, it only releases the data
// NOTE: THIS SHOULD ONLY BE USED FROM bm_unload_fast()!!!
static void bm_free_data_fast(int n)
{
	bitmap_entry *be;
	bitmap *bmp;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	be = &bm_bitmaps[n];
	bmp = &be->bm;

	// If there isn't a bitmap in this structure, don't
	// do anything but clear out the bitmap info
	if ( be->type == BM_TYPE_NONE) 
		return;

	// Don't free up memory for user defined bitmaps, since
	// BmpMan isn't the one in charge of allocating/deallocing them.
	if ( ( be->type == BM_TYPE_USER ) ) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 )
			bm_texture_ram -= be->data_size;
#endif
		return;
	}

	// If this bitmap doesn't have any data to free, skip
	// the freeing it part of this.
	if ( (bmp->data == 0) ) {
#ifdef BMPMAN_NDEBUG
		if ( be->data_size != 0 ) {
			bm_texture_ram -= be->data_size;
			be->data_size = 0;
		}
#endif
		return;
	}

	// Free up the data now!

	//	mprintf(( "Bitmap %d freed %d bytes\n", n, bm_bitmaps[n].data_size ));
#ifdef BMPMAN_NDEBUG
	bm_texture_ram -= be->data_size;
	be->data_size = 0;
#endif
	vm_free((void *)bmp->data);
	bmp->data = 0;
}


void bm_clean_slot(int n)
{
	bm_free_data(n);
}


void *bm_malloc( int n, int size )
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	if (size <= 0)
		return NULL;

#ifdef BMPMAN_NDEBUG
	Assert( bm_bitmaps[n].data_size == 0 );
	bm_bitmaps[n].data_size += size;
	bm_texture_ram += size;
#endif

//	mprintf(( "Bitmap %d allocated %d bytes\n", n, size ));
	return vm_malloc(size);
}

// kinda like bm_malloc but only keeps track of how much memory is getting used
void bm_update_memory_used( int n, int size )
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	Assert( size >= 0 );

#ifdef BMPMAN_NDEBUG
	Assert( bm_bitmaps[n].data_size == 0 );
	bm_bitmaps[n].data_size += size;
	bm_texture_ram += size;
#endif
}

void bm_close()
{
	int i;
	if ( bm_inited )	{
		for (i=0; i<MAX_BITMAPS; i++ )	{
			bm_free_data(i);			// clears flags, bbp, data, etc
		}
		bm_inited = 0;
	}
}

void bm_init()
{
	int i;

	mprintf(( "Size of bitmap info = %d KB\n", sizeof( bm_bitmaps )/1024 ));
	mprintf(( "Size of bitmap extra info = %d bytes\n", sizeof( bm_extra_info ) ));
	
	if (!bm_inited)	{
		bm_inited = 1;
		atexit(bm_close);
	}
	
	for (i=0; i<MAX_BITMAPS; i++ ) {
		bm_bitmaps[i].filename[0] = '\0';
		bm_bitmaps[i].type = BM_TYPE_NONE;
		bm_bitmaps[i].comp_type = BM_TYPE_NONE;
		bm_bitmaps[i].dir_type = CF_TYPE_ANY;
		bm_bitmaps[i].info.user.data = NULL;
		bm_bitmaps[i].mem_taken = 0;
		bm_bitmaps[i].bm.data = 0;
		bm_bitmaps[i].bm.palette = NULL;
		bm_bitmaps[i].info.ani.eff.type = BM_TYPE_NONE;
		bm_bitmaps[i].info.ani.eff.filename[0] = '\0';
		#ifdef BMPMAN_NDEBUG
			bm_bitmaps[i].data_size = 0;
			bm_bitmaps[i].used_count = 0;
			bm_bitmaps[i].used_last_frame = 0;
			bm_bitmaps[i].used_this_frame = 0;
		#endif
		bm_bitmaps[i].load_count = 0;

		gr_bm_init(i);

		bm_free_data(i);  	// clears flags, bbp, data, etc
	}
}

// Returns number of bytes of bitmaps locked this frame
// ntotal = number of bytes of bitmaps locked this frame
// nnew = number of bytes of bitmaps locked this frame that weren't locked last frame
void bm_get_frame_usage(int *ntotal, int *nnew)
{
#ifdef BMPMAN_NDEBUG
	int i;
	
	*ntotal = 0;
	*nnew = 0;

	for (i=0; i<MAX_BITMAPS; i++ ) {
		if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && (bm_bitmaps[i].used_this_frame))	{
			if ( !bm_bitmaps[i].used_last_frame )	{
				*nnew += bm_bitmaps[i].mem_taken; 
			}
			*ntotal += bm_bitmaps[i].mem_taken;
		}
		bm_bitmaps[i].used_last_frame = bm_bitmaps[i].used_this_frame;
		bm_bitmaps[i].used_this_frame = 0;
	}
#endif
}

// Creates a bitmap that exists in RAM somewhere, instead
// of coming from a disk file.  You pass in a pointer to a
// block of 32 (or 8)-bit-per-pixel data.  Right now, the only
// bpp you can pass in is 32 or 8.  On success, it returns the
// bitmap number.  You cannot free that RAM until bm_release
// is called on that bitmap.
int bm_create( int bpp, int w, int h, void *data, int flags )
{
	if (bpp == 8) {
		Assert(flags & BMP_AABITMAP);
	} else {
		Assert( (bpp == 16) || (bpp == 24) || (bpp == 32) );
	}

	if ( !bm_inited ) bm_init();

	int n = -1;

	for (int i = MAX_BITMAPS-1; i >= 0; i-- ) {
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			n = i;
			break;
		}
	}

	Assert( n > -1 );

	// Out of bitmap slots
	if ( n == -1 )
		return -1;

	// make sure that we have valid data
	if (data == NULL) {
		Int3();
		return -1;
	}

	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );

	sprintf( bm_bitmaps[n].filename, "TMP%dx%d+%d", w, h, bpp );
	bm_bitmaps[n].type = BM_TYPE_USER;
	bm_bitmaps[n].comp_type = BM_TYPE_NONE;
	bm_bitmaps[n].palette_checksum = 0;

	bm_bitmaps[n].bm.w = (short) w;
	bm_bitmaps[n].bm.h = (short) h;
	bm_bitmaps[n].bm.rowsize = (short) w;
	bm_bitmaps[n].bm.bpp = (ubyte) bpp;
	bm_bitmaps[n].bm.flags = (ubyte) flags;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;

	bm_bitmaps[n].info.user.bpp = (ubyte)bpp;
	bm_bitmaps[n].info.user.data = data;
	bm_bitmaps[n].info.user.flags = (ubyte)flags;

	bm_bitmaps[n].signature = Bm_next_signature++;

	bm_bitmaps[n].handle = bm_get_next_handle() * MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;
	bm_bitmaps[n].mem_taken = (w * h * (bpp >> 3));

	bm_bitmaps[n].load_count++;

	bm_update_memory_used( n, bm_bitmaps[n].mem_taken );

	gr_bm_create(n);

	return bm_bitmaps[n].handle;
}

// slow sub helper function. Given a raw filename and an extension set, try and find the bitmap
// that isn't already loaded and may exist somewhere on the disk
// returns  -1 if it could not be found
//          index into ext_list[] if it was found as a file, fills img_cfg if available
int bm_load_sub_slow(char *real_filename, const int num_ext, const char **ext_list, CFILE **img_cfp = NULL, int dir_type = CF_TYPE_ANY)
{	
	char full_path[MAX_PATH];
	int size = 0, offset = 0;
	int rval = -1;

	rval = cf_find_file_location_ext(real_filename, num_ext, ext_list, dir_type, sizeof(full_path) - 1, full_path, &size, &offset, 0);

	// could not be found, or is invalid for some reason
	if ( (rval < 0) || (rval >= num_ext) )
		return -1;

	CFILE *test = cfopen_special(full_path, "rb", size, offset, dir_type);

	if (test != NULL) {
		if (img_cfp != NULL)
			*img_cfp = test;

		return rval;
	}

	// umm, that's not good...
	return -1;
}

// fast sub helper function. Given a raw filename, try and find a bitmap
// that's already loaded
// returns  0 if it could not be found
//          1 if it already exists, fills in handle
int bm_load_sub_fast(char *real_filename, int *handle, int dir_type = CF_TYPE_ANY, bool animated_type = false)
{
	if (Bm_ignore_duplicates)
		return 0;

	int i;

	for (i = 0; i < MAX_BITMAPS; i++) {
		if (bm_bitmaps[i].type == BM_TYPE_NONE)
			continue;

		if (bm_bitmaps[i].dir_type != dir_type)
			continue;

		bool animated = ((bm_bitmaps[i].type == BM_TYPE_EFF) || (bm_bitmaps[i].type == BM_TYPE_ANI));

		if ( animated_type && !animated )
			continue;
		else if ( !animated_type && animated )
			continue;

		if ( !strextcmp(real_filename, bm_bitmaps[i].filename) ) {
			nprintf(("BmpFastLoad", "Found bitmap %s -- number %d\n", bm_bitmaps[i].filename, i));
			bm_bitmaps[i].load_count++;
			*handle = bm_bitmaps[i].handle;
			return 1;
		}
	}

	// not found to be loaded already
	return 0;
}

// This loads a bitmap so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number.  Function doesn't actually load the data, only
// width, height, and possibly flags.
int bm_load( char *real_filename )
{
	int i, free_slot = -1;
	int w, h, bpp = 8;
	int rc = 0;
	int bm_size = 0, mm_lvl = 0;
	char filename[MAX_FILENAME_LEN];
	ubyte type = BM_TYPE_NONE;
	ubyte c_type = BM_TYPE_NONE;
	CFILE *img_cfp = NULL;
	int handle = -1;

	if ( !bm_inited )
		bm_init();

	// if no file was passed then get out now
	if ( (real_filename == NULL) || (strlen(real_filename) <= 0) )
		return -1;

	// make sure no one passed an extension
	memset( filename, 0, MAX_FILENAME_LEN );
	strncpy( filename, real_filename, MAX_FILENAME_LEN-1 );
	char *p = strrchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}

	// If we are standalone server keep replacing the 'right_bracket' (right side help bracket) as the filename
	// should keep the game happy while loading only single pcx file which the needs to be present in any case
	if (Is_standalone) {
		char standalone_filename[MAX_FILENAME_LEN] = "right_bracket";
		strcpy_s(filename,standalone_filename);
	}

	// safety catch for strcat...
	// MAX_FILENAME_LEN-5 == '.' plus 3 letter ext plus NULL terminator
	if (strlen(filename) > MAX_FILENAME_LEN-5) {
		Warning( LOCATION, "Passed filename, '%s', is too long to support an extension!!\n\nMaximum length, minus the extension, is %i characters.\n", filename, MAX_FILENAME_LEN-5 );
		return -1;
	}

	// Lets find out what type it is
	{
		const int NUM_TYPES	= 5;
		const ubyte type_list[NUM_TYPES] = { BM_TYPE_DDS, BM_TYPE_TGA, BM_TYPE_PNG, BM_TYPE_JPG, BM_TYPE_PCX };
		const char *ext_list[NUM_TYPES] = { ".dds", ".tga", ".png", ".jpg", ".pcx" };

		// see if it's already loaded (checks for any type with filename)
		if ( bm_load_sub_fast(filename, &handle) )
			return handle;

		// if we are still here then we need to fall back to a file-based search
		int rval = bm_load_sub_slow(filename, NUM_TYPES, ext_list, &img_cfp);

		if (rval < 0)
			return -1;

		strcat_s(filename, ext_list[rval]);
		type = type_list[rval];
	}

	Assert(type != BM_TYPE_NONE);

	// Find an open slot
	for (i = 0; i < MAX_BITMAPS; i++) {
		if (bm_bitmaps[i].type == BM_TYPE_NONE) {
			free_slot = i;
			break;
		}
	}

	if (free_slot < 0) {
		Assertion(free_slot < 0, "Could not find free BMPMAN slot for bitmap: %s", real_filename);
		goto Done;
	}

	rc = gr_bm_load( type, free_slot, filename, img_cfp, &w, &h, &bpp, &c_type, &mm_lvl, &bm_size );

	if (rc != 0)
		goto Done;

	if ( (bm_size <= 0) && (w) && (h) && (bpp) )
		bm_size = (w * h * (bpp >> 3));


	handle = bm_get_next_handle() * MAX_BITMAPS + free_slot;

	// ensure fields are cleared out from previous bitmap
	memset( &bm_bitmaps[free_slot], 0, sizeof(bitmap_entry) );

	// Mark the slot as filled, because cf_read might load a new bitmap
	// into this slot.
	strncpy(bm_bitmaps[free_slot].filename, filename, MAX_FILENAME_LEN-1);
	bm_bitmaps[free_slot].type = type;
	bm_bitmaps[free_slot].comp_type = c_type;
	bm_bitmaps[free_slot].signature = Bm_next_signature++;
	bm_bitmaps[free_slot].bm.w = (short) w;
	bm_bitmaps[free_slot].bm.rowsize = (short) w;
	bm_bitmaps[free_slot].bm.h = (short) h;
	bm_bitmaps[free_slot].bm.bpp = 0;
	bm_bitmaps[free_slot].bm.true_bpp = (ubyte) bpp;
	bm_bitmaps[free_slot].bm.flags = 0;
	bm_bitmaps[free_slot].bm.data = 0;
	bm_bitmaps[free_slot].bm.palette = NULL;
	bm_bitmaps[free_slot].num_mipmaps = mm_lvl;
	bm_bitmaps[free_slot].mem_taken = bm_size;
	bm_bitmaps[free_slot].dir_type = CF_TYPE_ANY;
	bm_bitmaps[free_slot].palette_checksum = 0;
	bm_bitmaps[free_slot].handle = handle;
	bm_bitmaps[free_slot].last_used = -1;

	bm_bitmaps[free_slot].load_count++;

Done:
	if (img_cfp != NULL)
		cfclose(img_cfp);

	return handle;
}

// special load function. basically allows you to load a bitmap which already exists (by filename). 
// this is useful because in some cases we need to have a bitmap which is locked in screen format
// _and_ texture format, such as pilot pics and squad logos
int bm_load_duplicate(char *filename)
{
	int ret;

	// ignore duplicates
	Bm_ignore_duplicates = 1;
	
	// load
	ret = bm_load(filename);

	// back to normal
	Bm_ignore_duplicates = 0;

	return ret;
}

DCF(bm_frag,"Shows BmpMan fragmentation")
{
	if ( Dc_command )	{

		gr_clear();

		int x=0, y=0;
		int xs=2, ys=2;
		int w=4, h=4;

		for (int i=0; i<MAX_BITMAPS; i++ )	{
			switch( bm_bitmaps[i].type )	{
			case BM_TYPE_NONE:
				gr_set_color(128,128,128);
				break;
			case BM_TYPE_PCX:
				gr_set_color(255,0,0);
				break;
			case BM_TYPE_USER:
			case BM_TYPE_TGA:
			case BM_TYPE_PNG:
			case BM_TYPE_DDS:
				gr_set_color(0,255,0);
				break;
			case BM_TYPE_ANI:
			case BM_TYPE_EFF:
				gr_set_color(0,0,255);
				break;
			}

			gr_rect( x+xs, y+ys, w, h );
			x += w+xs+xs;
			if ( x > 639 )	{
				x = 0;
				y += h + ys + ys;
			}

		}

		gr_flip();
		key_getch();
	}
}

static int find_block_of(int n)
{
	int i, cnt = 0, nstart = 0;

	if (n < 1) {
		Int3();
		return -1;
	}

	for (i = 0; i < MAX_BITMAPS; i++) {
		if (bm_bitmaps[i].type == BM_TYPE_NONE) {
			if (cnt == 0)
				nstart = i;

			cnt++;
		} else {
			cnt = 0;
		}

		if (cnt == n)
			return nstart;
	}

	// Error( LOCATION, "Couldn't find block of %d frames\n", n );
	return -1;
}

int bm_load_and_parse_eff(char *filename, int dir_type, int *nframes, int *nfps, int *key, ubyte *type)
{
	int frames = 0, fps = 30, keyframe = 0, rval;
	char ext[8];
	ubyte c_type = BM_TYPE_NONE;
	char file_text[1024];
	char file_text_raw[1024];

	memset( ext, 0, sizeof(ext) );
	memset( file_text, 0, sizeof(file_text) );
	memset( file_text_raw, 0, sizeof(file_text_raw) );

	// pause anything that may happen to be parsing right now
	pause_parse();

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("BMPMAN: Unable to parse '%s'!  Error code = %i.\n", filename, rval));
		unpause_parse();
		return -1;
	}

	// now start parsing the EFF
	read_file_text(filename, dir_type, file_text, file_text_raw);
	reset_parse(file_text);

	required_string("$Type:");
	stuff_string(ext, F_NAME, sizeof(ext));

	required_string( "$Frames:" );
	stuff_int(&frames);

	if (optional_string( "$FPS:" ))
		stuff_int(&fps);

	if (optional_string( "$Keyframe:" ))
		stuff_int(&keyframe);

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
		return -1;
	}

	// did we do anything?
	if (c_type == BM_TYPE_NONE || frames == 0) {
		mprintf(("BMPMAN: EFF parse ERROR!\n"));
		return -1;
	}

	if (type)
		*type = c_type;

	if (nframes)
		*nframes = frames;

	if (nfps)
		*nfps = fps;

	if (key)
		*key = keyframe;

	return 0;
}

// ------------------------------------------------------------------
// bm_load_animation()
//
//	input:		filename		=>		filename of animation
//					nframes		=>		OUTPUT parameter:	number of frames in the animation
//					fps			=>		OUTPUT/OPTIONAL parameter: intended fps for the animation
//
// returns:		bitmap number of first frame in the animation
//
int bm_load_animation( char *real_filename, int *nframes, int *fps, int *keyframe, int can_drop_frames, int dir_type)
{
	int	i, n;
	anim	the_anim;
	CFILE	*img_cfp = NULL;
	char filename[MAX_FILENAME_LEN];
	int reduced = 0;
	int anim_fps = 0, anim_frames = 0, key = 0;
	int anim_width = 0, anim_height = 0;
	ubyte type = BM_TYPE_NONE, eff_type = BM_TYPE_NONE, c_type = BM_TYPE_NONE;
	int bpp = 0, mm_lvl = 0, img_size = 0;
	char clean_name[MAX_FILENAME_LEN];
	const int NUM_TYPES	= 2;
	const ubyte type_list[NUM_TYPES] = {BM_TYPE_EFF, BM_TYPE_ANI};
	const char *ext_list[NUM_TYPES] = {".eff", ".ani"};

	if ( !bm_inited )
		bm_init();

	// set defaults for frame count and fps before going any further
	if (nframes)
		*nframes = 0;

	if (fps)
		*fps = 0;

	if (keyframe)
		*keyframe = 0;

	memset( filename, 0, MAX_FILENAME_LEN );
	strncpy( filename, real_filename, MAX_FILENAME_LEN-1 );
	char *p = strchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load_animation for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}

	// If we are standalone server keep replacing the 'cursorweb' (mouse cursor) as the filename
	// should keep the game happy while loading only single ani file which the needs to be present in any case
	if (Is_standalone) {
		char standalone_filename[MAX_FILENAME_LEN] = "cursorweb";
		strcpy_s(filename,standalone_filename);
	}

	// safety catch for strcat...
	// MAX_FILENAME_LEN-5 == '.' plus 3 letter ext plus NULL terminator
	if (strlen(filename) > MAX_FILENAME_LEN-5) {
		Warning( LOCATION, "Passed filename, '%s', is too long to support an extension!!\n\nMaximum length, minus the extension, is %i characters.\n", filename, MAX_FILENAME_LEN-5 );
		return -1;
	}

	// used later if EFF type
	strcpy_s( clean_name, filename );

	// Lets find out what type it is
	{
		int handle = -1;

		// do a search for any previously loaded files (looks at filename only)
		if ( bm_load_sub_fast(filename, &handle, dir_type, true) ) {
			n = handle % MAX_BITMAPS;
			Assert( bm_bitmaps[n].handle == handle );

			if (nframes)
				*nframes = bm_bitmaps[n].info.ani.num_frames;

			if (fps)
				*fps = bm_bitmaps[n].info.ani.fps;
	
			if (keyframe)
				*keyframe = bm_bitmaps[n].info.ani.keyframe;

			return handle;
		}

		// if we are still here then we need to fall back to a file-based search
		int rval = bm_load_sub_slow(filename, NUM_TYPES, ext_list, &img_cfp, dir_type);

		if (rval < 0)
			return -1;

		strcat_s(filename, ext_list[rval]);
		type = type_list[rval];
	}

	// If we found an animation then there is an extra 5 char size limit to adhere to. We don't do this check earlier since it's only needed if we found an anim
	// an ANI needs about 5 extra characters to have the "[###]" frame designator
	// an EFF needs 5 extra characters for each frame filename too, which just happens to be the same length as the frame designator needed otherwise
	// MAX_FILENAME_LEN-10 == 5 character frame designator plus '.' plus 3 letter ext plus NULL terminator
	// we only check for -5 here since the filename should already have the extension on it, and it must have passed the previous check
	if (strlen(filename) > MAX_FILENAME_LEN-5) {
		Warning( LOCATION, "Passed filename, '%s', is too long to support an extension and frames!!\n\nMaximum length for an ANI/EFF, minus the extension, is %i characters.\n", filename, MAX_FILENAME_LEN-10 );
		return -1;
	}

	// it's an effect file, any readable image type with eff being txt
	if (type == BM_TYPE_EFF) {
		if ( bm_load_and_parse_eff(filename, dir_type, &anim_frames, &anim_fps, &key, &eff_type) != 0 ) {
			mprintf(("BMPMAN: Error reading EFF\n"));
			return -1;
		} else {
			mprintf(("BMPMAN: Found EFF (%s) with %d frames at %d fps.\n", filename, anim_frames, anim_fps));
		}
	}
	// regular ani file
	else if (type == BM_TYPE_ANI) {
#ifndef NDEBUG
		// for debug of ANI sizes
		strcpy_s(the_anim.name, real_filename);
#endif
		anim_read_header(&the_anim, img_cfp);

		anim_frames = the_anim.total_frames;
		anim_fps = the_anim.fps;
		anim_width = the_anim.width;
		anim_height = the_anim.height;
		bpp = 8;
		img_size = (anim_width * anim_height * bpp);
		//we only care if there are 2 keyframes - first frame, other frame to jump to for ship/weapons
		//mainhall door anis hav every frame as keyframe, so we don't care
		//other anis only have the first frame
		if(the_anim.num_keys == 2){
			the_anim.keys = (key_frame*)vm_malloc(sizeof(key_frame) * the_anim.num_keys);
			Assert(the_anim.keys != NULL);

			for(i=0;i<the_anim.num_keys;i++){
				the_anim.keys[i].frame_num = 0;
				cfread(&the_anim.keys[i].frame_num, 2, 1, img_cfp);
				cfread(&the_anim.keys[i].offset, 4, 1, img_cfp);
				the_anim.keys[i].frame_num = INTEL_INT( the_anim.keys[i].frame_num );
				the_anim.keys[i].offset = INTEL_INT( the_anim.keys[i].offset );
			}
			//some retail anis have their keyframes reversed
			key = MAX(the_anim.keys[0].frame_num, the_anim.keys[1].frame_num);
		}
	} else {
		return -1;
	}

	if ( (can_drop_frames) && (type != BM_TYPE_EFF) ) {
		if ( Bm_low_mem == 1 ) {
			reduced = 1;
			anim_frames = (anim_frames+1)/2;
			anim_fps = (anim_fps/2);
		} else if ( Bm_low_mem == 2 ) {
			anim_frames = 1;
		}
	}


	n = find_block_of(anim_frames);

	if (n < 0) {
		if (img_cfp != NULL)
			cfclose(img_cfp);

		return -1;
	}

	int first_handle = bm_get_next_handle();

	Assert ( strlen(filename) < MAX_FILENAME_LEN );
	for ( i = 0; i < anim_frames; i++ ) {
		memset( &bm_bitmaps[n+i], 0, sizeof(bitmap_entry) );
	
		if (type == BM_TYPE_EFF) {
			bm_bitmaps[n+i].info.ani.eff.type = eff_type;
			sprintf(bm_bitmaps[n+i].info.ani.eff.filename, "%s_%.4d", clean_name, i);

			// gr_bm_load() returns non-0 on failure
			if ( gr_bm_load(eff_type, n+i, bm_bitmaps[n+i].info.ani.eff.filename, NULL, &anim_width, &anim_height, &bpp, &c_type, &mm_lvl, &img_size) ) {
				// if we didn't get anything then bail out now
				if ( i == 0 ) {
					Warning(LOCATION, "EFF: No frame images were found.  EFF, %s, is invalid.\n", filename);

					if (img_cfp != NULL)
						cfclose(img_cfp);

					return -1;
				}

				Warning(LOCATION, "EFF: Unable to load all frames for '%s', stopping at #%d\n", filename, i);

				// reset total frames to current
				anim_frames = i;

				// update all previous frames with the new count
				for (int j=0; j<anim_frames; j++)
					bm_bitmaps[n+j].info.ani.num_frames = anim_frames;

				break;
			}

			if ( (img_size <= 0) && (anim_width) && (anim_height) && (bpp) ) {
				img_size = (anim_width * anim_height * (bpp >> 3));
			}
		}

		bm_bitmaps[n+i].info.ani.first_frame = n;
		bm_bitmaps[n+i].info.ani.num_frames = anim_frames;
		bm_bitmaps[n+i].info.ani.fps = (ubyte)anim_fps;
		bm_bitmaps[n+i].info.ani.keyframe = key;
		bm_bitmaps[n+i].bm.w = (short) anim_width ;
		bm_bitmaps[n+i].bm.rowsize = (short) anim_width;
		bm_bitmaps[n+i].bm.h = (short) anim_height;
		if ( reduced ) {
			bm_bitmaps[n+i].bm.w /= 2;
			bm_bitmaps[n+i].bm.rowsize /= 2;
			bm_bitmaps[n+i].bm.h /= 2;
		}
		bm_bitmaps[n+i].bm.flags = 0;
		bm_bitmaps[n+i].bm.bpp = 0;
		bm_bitmaps[n+i].bm.true_bpp = (ubyte)bpp;
		bm_bitmaps[n+i].bm.data = 0;
		bm_bitmaps[n+i].bm.palette = NULL;
		bm_bitmaps[n+i].type = type;
		bm_bitmaps[n+i].comp_type = c_type;
		bm_bitmaps[n+i].palette_checksum = 0;
		bm_bitmaps[n+i].signature = Bm_next_signature++;
		bm_bitmaps[n+i].handle = first_handle*MAX_BITMAPS + n+i;
		bm_bitmaps[n+i].last_used = -1;
		bm_bitmaps[n+i].num_mipmaps = mm_lvl;
		bm_bitmaps[n+i].mem_taken = img_size;
		bm_bitmaps[n+i].dir_type = dir_type;

		bm_bitmaps[n+i].load_count++;

		if ( i == 0 )	{
			sprintf( bm_bitmaps[n+i].filename, "%s", filename );
		} else {
			sprintf( bm_bitmaps[n+i].filename, "%s[%d]", filename, i );
		}

	}

	if (nframes)
		*nframes = anim_frames;

	if (fps)
		*fps = anim_fps;

	if (img_cfp != NULL)
		cfclose(img_cfp);

	if (keyframe)
		*keyframe = key;

	return bm_bitmaps[n].handle;
}

int bm_load_either(char *filename, int *nframes, int *fps, int *keyframe, int can_drop_frames, int dir_type)
{
	if(nframes != NULL)
		*nframes = 0;
	if(fps != NULL)
		*fps = 0;
	int tidx = bm_load_animation(filename, nframes, fps, keyframe, can_drop_frames, dir_type);
	if(tidx == -1)
	{
		tidx = bm_load(filename);
		if(tidx != -1 && nframes != NULL)
			*nframes = 1;
	}

	return tidx;
}

int bm_is_valid(int handle)
{
	if(!bm_inited) return 0;
	return (bm_bitmaps[handle % MAX_BITMAPS].handle == handle);
}

// Gets info.   w,h,or flags,nframes or fps can be NULL if you don't care.
int bm_get_info( int handle, int *w, int * h, ubyte * flags, int *nframes, int *fps)
{
	bitmap * bmp;

	if ( !bm_inited ) return -1;

	int bitmapnum = handle % MAX_BITMAPS;

	#ifndef NDEBUG
	if(bm_bitmaps[bitmapnum].handle != handle) {
		mprintf(("bm_get_info - %s: bm_bitmaps[%d].handle = %d, handle = %d\n", bm_bitmaps[bitmapnum].filename, bitmapnum, bm_bitmaps[bitmapnum].handle, handle));
	}
	#endif

	Assertion( bm_bitmaps[bitmapnum].handle == handle, "Invalid bitmap handle %d passed to bm_get_info().\nThis might be due to an invalid animation somewhere else.\n", handle );		// INVALID BITMAP HANDLE!

	if ( (bm_bitmaps[bitmapnum].type == BM_TYPE_NONE) || (bm_bitmaps[bitmapnum].handle != handle) ) {
		if (w) *w = 0;
		if (h) *h = 0;
		if (flags) *flags = 0;
		if (nframes) *nframes=0;
		if (fps) *fps=0;
		return -1;
	}

	bmp = &(bm_bitmaps[bitmapnum].bm);

	if (w) *w = bmp->w;
	if (h) *h = bmp->h;
	if (flags) *flags = bmp->flags;

	if ( (bm_bitmaps[bitmapnum].type == BM_TYPE_ANI) || (bm_bitmaps[bitmapnum].type == BM_TYPE_EFF) )	{
		if (nframes) {
			*nframes = bm_bitmaps[bitmapnum].info.ani.num_frames;
		} 
		if (fps) {
			*fps= bm_bitmaps[bitmapnum].info.ani.fps;
		}

		return bm_bitmaps[bm_bitmaps[bitmapnum].info.ani.first_frame].handle;
	} else {
		if (nframes) {
			*nframes = 1;
		} 
		if (fps) {
			*fps= 0;
		}

		return handle;
	}
}

unsigned int bm_get_signature(int handle)
{
	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;
	Assertion( bm_bitmaps[bitmapnum].handle == handle, "Invalid bitmap handle %d passed to bm_get_signature().\nThis might be due to an invalid animation somewhere else.\n", handle );		// INVALID BITMAP HANDLE!

	return bm_bitmaps[bitmapnum].signature;
}

//gets the image type
ubyte bm_get_type(int handle)
{
	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;
	Assertion( bm_bitmaps[bitmapnum].handle == handle, "Invalid bitmap handle %d passed to bm_get_type().\nThis might be due to an invalid animation somewhere else.\n", handle );		// INVALID BITMAP HANDLE!

	return bm_bitmaps[bitmapnum].type;
}

extern int palman_is_nondarkening(int r,int g, int b);
static void bm_convert_format( int bitmapnum, bitmap *bmp, ubyte bpp, ubyte flags )
{
	int idx;

	// no transparency for 24 bpp images
	if ( !(flags & BMP_AABITMAP) && (bmp->bpp == 24) )
		return;

	if (Is_standalone) {
		Assert(bmp->bpp == 8);
		return;
	} else {
		if (flags & BMP_AABITMAP)
			Assert(bmp->bpp == 8);
		else
			Assert( (bmp->bpp == 16) || (bmp->bpp == 32) );
	}

	// maybe swizzle to be an xparent texture
	if(!(bmp->flags & BMP_TEX_XPARENT) && (flags & BMP_TEX_XPARENT)){
		for(idx=0; idx<bmp->w*bmp->h; idx++){			
			// if the pixel is transparent
			if ( ((ushort*)bmp->data)[idx] == Gr_t_green.mask)	{
				((ushort*)bmp->data)[idx] = 0;
			}
		}

		bmp->flags |= BMP_TEX_XPARENT;
	}	
}

// basically, map the bitmap into the current palette. used to be done for all pcx's, now just for
// Fred, since its the only thing that uses the software tmapper
void bm_swizzle_8bit_for_fred(bitmap_entry *be, bitmap *bmp, ubyte *data, ubyte *palette)
{
/* 2004/10/17 - taylor - no longer needed since FRED is OGL now*/
}

void bm_lock_pcx( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data;
	int pcx_error;
	char filename[MAX_FILENAME_LEN];

	// Unload any existing data
	bm_free_data( bitmapnum );	

	be->mem_taken = (bmp->w * bmp->h * (bpp >> 3));
	data = (ubyte *)bm_malloc(bitmapnum, be->mem_taken);
	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = (bpp == 8) ? gr_palette : NULL;
	memset( data, 0, be->mem_taken );

	Assert( &be->bm == bmp );
#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
#endif

	// some sanity checks on flags
	Assert(!((flags & BMP_AABITMAP) && (flags & BMP_TEX_ANY)));						// no aabitmap textures

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	pcx_error = pcx_read_bitmap( filename, data, NULL, (bpp >> 3), (flags & BMP_AABITMAP), 0, be->dir_type );

	if ( pcx_error != PCX_ERROR_NONE ) {
		mprintf(("Couldn't load PCX!!! (%s)\n", filename));
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
#endif		
	
	bmp->flags = 0;	

	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

void bm_lock_ani( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	anim				*the_anim;
	anim_instance	*the_anim_instance;
	bitmap			*bm;
	ubyte				*frame_data;
	int				size, i;
	int				first_frame, nframes;	

	first_frame = be->info.ani.first_frame;
	nframes = bm_bitmaps[first_frame].info.ani.num_frames;

	if ( (the_anim = anim_load(bm_bitmaps[first_frame].filename, bm_bitmaps[first_frame].dir_type)) == NULL ) {
		nprintf(("BMPMAN", "Error opening %s in bm_lock\n", be->filename));
		return;
	}

	if ( (the_anim_instance = init_anim_instance(the_anim, bpp)) == NULL ) {
		nprintf(("BMPMAN", "Error opening %s in bm_lock\n", be->filename));
		anim_free(the_anim);
		return;
	}

	int can_drop_frames = 0;

	if ( the_anim->total_frames != bm_bitmaps[first_frame].info.ani.num_frames )	{
		can_drop_frames = 1;
	}

	bm = &bm_bitmaps[first_frame].bm;
	size = bm->w * bm->h * (bpp >> 3);
	be->mem_taken = size;

	Assert( size > 0 );

	for ( i=0; i<nframes; i++ )	{
		be = &bm_bitmaps[first_frame+i];
		bm = &bm_bitmaps[first_frame+i].bm;

		// Unload any existing data
		bm_free_data( first_frame+i );

		bm->flags = 0;

		// briefing editor in Fred2 uses aabitmaps (ani's) - force to 8 bit
		bm->bpp = Is_standalone ? (ubyte)8 : bpp;

		bm->data = (ptr_u)bm_malloc(first_frame + i, size);

		frame_data = anim_get_next_raw_buffer(the_anim_instance, 0 ,flags & BMP_AABITMAP ? 1 : 0, bm->bpp);

		if ( frame_data == NULL ) {
			// Error(LOCATION,"Fatal error locking .ani file: %s\n", be->filename);
		}		
		
		ubyte *dptr, *sptr;

		sptr = frame_data;
		dptr = (ubyte *)bm->data;

		if ( (bm->w!=the_anim->width) || (bm->h!=the_anim->height) )	{
			// Scale it down
			// Int3();			// not ready yet - should only be ingame
	
			// 8 bit
			if(bpp == 8){
				int w,h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( the_anim->width*F1_0 ) / bm->w;
				dv = ( the_anim->height*F1_0 ) / bm->h;
												
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
				int w,h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( the_anim->width*F1_0 ) / bm->w;
				dv = ( the_anim->height*F1_0 ) / bm->h;
												
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

		bm_convert_format( first_frame+i, bm, bpp, flags );

		// Skip a frame
		if ( (i < nframes-1)  && can_drop_frames )	{
			frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, flags & BMP_AABITMAP ? 1 : 0, bm->bpp);
		}

		//mprintf(( "Checksum = %d\n", be->palette_checksum ));
	}

	free_anim_instance(the_anim_instance);
	anim_free(the_anim);
}


void bm_lock_user( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	// Unload any existing data
	bm_free_data( bitmapnum );	

	if ((bpp != be->info.user.bpp) && !(flags & BMP_AABITMAP))
		bpp = be->info.user.bpp;

	switch ( bpp ) {
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
			Int3();
			break;
			// Error( LOCATION, "Unhandled user bitmap conversion from %d to %d bpp", be->info.user.bpp, bmp->bpp );
	}

	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

void bm_lock_tga( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data = NULL;
	int d_size, byte_size;
	char filename[MAX_FILENAME_LEN];

	// Unload any existing data
	bm_free_data( bitmapnum );	

	if(Is_standalone){
		Assert(bpp == 8);
	} 
	else 
	{
		Assert( (bpp == 16) || (bpp == 24 ) || (bpp == 32) );
	}

	// allocate bitmap data
	byte_size = (bpp >> 3);

	Assert( byte_size );
	Assert( be->mem_taken > 0 );

	data = (ubyte*)bm_malloc(bitmapnum, be->mem_taken);

	if (data) {
		memset( data, 0, be->mem_taken);
		d_size = byte_size;
 	} else {
		return;
 	}

	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert( &be->bm == bmp );
#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
#endif
	
	int tga_error;

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	tga_error = targa_read_bitmap( filename, data, NULL, d_size, be->dir_type);

	if ( tga_error != TARGA_ERROR_NONE )	{
		bm_free_data( bitmapnum );
		return;
	}

	bmp->flags = 0;	
	
	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

//lock a dds file
void bm_lock_dds( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data = NULL;
	int error;
	ubyte dds_bpp = 0;
	char filename[MAX_FILENAME_LEN];

	// free any existing data
	bm_free_data( bitmapnum );

	Assert( be->mem_taken > 0 );
	Assert( &be->bm == bmp );

	data = (ubyte*)bm_malloc(bitmapnum, be->mem_taken);

	if ( data == NULL )
		return;

	memset( data, 0, be->mem_taken );

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	error = dds_read_bitmap( filename, data, &dds_bpp, be->dir_type );

#if BYTE_ORDER == BIG_ENDIAN
	// same as with TGA, we need to byte swap 16 & 32-bit, uncompressed, DDS images
	if ( (be->comp_type == BM_TYPE_DDS) || (be->comp_type == BM_TYPE_CUBEMAP_DDS) ) {
		unsigned int i = 0;

		if (dds_bpp == 32) {
			unsigned int *swap_tmp;

			for (i = 0; i < (unsigned int)be->mem_taken; i += 4) {
				swap_tmp = (unsigned int *)(data + i);
				*swap_tmp = INTEL_INT(*swap_tmp);
			}
		} else if (dds_bpp == 16) {
			unsigned short *swap_tmp;

			for (i = 0; i < (unsigned int)be->mem_taken; i += 2) {
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
		bm_free_data( bitmapnum );
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
#endif
}

// lock a PNG file
void bm_lock_png( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data = NULL;
	//assume 32 bit - libpng should expand everything
	int d_size;
	int png_error = PNG_ERROR_INVALID;
	char filename[MAX_FILENAME_LEN];
	//int size = bmp->w * bmp->h * d_size;

	// Unload any existing data
	bm_free_data( bitmapnum );

	// allocate bitmap data
	Assert( bmp->w * bmp->h > 0 );

	//if it's not 32-bit, we expand when we read it
	bmp->bpp = 32;
	d_size = bmp->bpp >> 3;
	//we waste memory if it turns out to be 24-bit, but the way this whole thing works is dodgy anyway
	data = (ubyte*)bm_malloc(bitmapnum, bmp->w * bmp->h * d_size);
	if (data == NULL)
		return;
	memset( data, 0, bmp->w * bmp->h * d_size);
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert( &be->bm == bmp );

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	//bmp->bpp gets set correctly in here after reading into memory
	png_error = png_read_bitmap( filename, data, &bmp->bpp, d_size, be->dir_type );

	if ( png_error != PNG_ERROR_NONE )	{
		bm_free_data( bitmapnum );
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
#endif
}

// lock a JPEG file
void bm_lock_jpg( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data = NULL;
	int d_size = 0;
	int jpg_error = JPEG_ERROR_INVALID;
	char filename[MAX_FILENAME_LEN];

	// Unload any existing data
	bm_free_data( bitmapnum );	

	d_size = (bpp >> 3);

	// allocate bitmap data
	Assert( be->mem_taken > 0 );
	data = (ubyte*)bm_malloc(bitmapnum, be->mem_taken);

	if (data == NULL)
		return;

	memset( data, 0, be->mem_taken);
 
	bmp->bpp = bpp;
	bmp->data = (ptr_u)data;
	bmp->palette = NULL;

	Assert( &be->bm == bmp );

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	jpg_error = jpeg_read_bitmap( filename, data, NULL, d_size, be->dir_type );

	if ( jpg_error != JPEG_ERROR_NONE )	{
		bm_free_data( bitmapnum );
		return;
	}

#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
#endif
}

MONITOR( NumBitmapPage )
MONITOR( SizeBitmapPage )

// This locks down a bitmap and returns a pointer to a bitmap
// that can be accessed until you call bm_unlock.   Only lock
// a bitmap when you need it!  This will convert it into the 
// appropriate format also.
bitmap * bm_lock( int handle, ubyte bpp, ubyte flags )
{
	bitmap			*bmp;
	bitmap_entry	*be;

	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;

	Assertion( bm_bitmaps[bitmapnum].handle == handle, "Invalid handle %d passed to bm_lock. This might be due to an animation elsewhere in the code being too short.\n", handle );		// INVALID BITMAP HANDLE

//	flags &= (~BMP_RLE);

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
			Assert( bpp == 8 );
		} else if ((flags & BMP_TEX_NONCOMP) && (!(flags & BMP_TEX_COMP))) {
			Assert( bpp >= 16 );  // cheating but bpp passed isn't what we normally end up with
		} else if ((flags & BMP_TEX_DXT1) || (flags & BMP_TEX_DXT3) || (flags & BMP_TEX_DXT5)){
			Assert( bpp >= 16 ); // cheating but bpp passed isn't what we normally end up with
		} else if (flags & BMP_TEX_CUBEMAP) {
			Assert( (bm_bitmaps[bitmapnum].type == BM_TYPE_CUBEMAP_DDS) ||
					(bm_bitmaps[bitmapnum].type == BM_TYPE_CUBEMAP_DXT1) ||
					(bm_bitmaps[bitmapnum].type == BM_TYPE_CUBEMAP_DXT3) ||
					(bm_bitmaps[bitmapnum].type == BM_TYPE_CUBEMAP_DXT5) );
			Assert( bpp >= 16 );
		} else {
			Assert(0);		//?
		}
	}

	be = &bm_bitmaps[bitmapnum];
	bmp = &be->bm;

	// If you hit this assert, chances are that someone freed the
	// wrong bitmap and now someone is trying to use that bitmap.
	// See John.
	Assert( be->type != BM_TYPE_NONE );		

	// Increment ref count for bitmap since lock was made on it.
	Assert(be->ref_count >= 0);
	be->ref_count++;					// Lock it before we page in data; this prevents a callback from freeing this
											// as it gets read in

	// Mark this bitmap as used this frame
	#ifdef BMPMAN_NDEBUG
	if ( be->used_this_frame < 255 )	{
		be->used_this_frame++;
	}
	#endif

	// read the file data
	if ( gr_bm_lock( be->filename, handle, bitmapnum, bpp, flags ) == -1 ) {
		// oops, this isn't good - reset and return NULL
		bm_unlock( bitmapnum );
		bm_unload( bitmapnum );

		return NULL;
	}

	MONITOR_INC( NumBitmapPage, 1 );
	MONITOR_INC( SizeBitmapPage, bmp->w*bmp->h );

	if ( (be->type == BM_TYPE_ANI) || (be->type == BM_TYPE_EFF) ) {
		int i,first = bm_bitmaps[bitmapnum].info.ani.first_frame;

		for ( i=0; i< bm_bitmaps[first].info.ani.num_frames; i++ )	{
			// Mark all the bitmaps in this bitmap or animation as recently used
			bm_bitmaps[first+i].last_used = timer_get_milliseconds();

#ifdef BMPMAN_NDEBUG
			// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
			bm_bitmaps[first+i].used_count++;
#endif

			bm_bitmaps[first+i].used_flags = flags;
		}
	} else {
		// Mark all the bitmaps in this bitmap or animation as recently used
		be->last_used = timer_get_milliseconds();

#ifdef BMPMAN_NDEBUG
		// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
		be->used_count++;
#endif
		be->used_flags = flags;
	}

	return bmp;
}

// Unlocks a bitmap
//
// Decrements the ref_count member of the bitmap_entry struct.  A bitmap can only be unloaded
// when the ref_count is 0.
//
void bm_unlock( int handle )
{
	bitmap_entry	*be;
	bitmap			*bmp;

	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;

	#ifndef NDEBUG
	if(bm_bitmaps[bitmapnum].handle != handle) {
		mprintf(("bm_unlock - %s: bm_bitmaps[%d].handle = %d, handle = %d\n", bm_bitmaps[bitmapnum].filename, bitmapnum, bm_bitmaps[bitmapnum].handle, handle));
	}
	#endif

	Assert( bm_bitmaps[bitmapnum].handle == handle );	// INVALID BITMAP HANDLE

	Assert( (bitmapnum >= 0) && (bitmapnum < MAX_BITMAPS) );

	be = &bm_bitmaps[bitmapnum];
	bmp = &be->bm;

	be->ref_count--;
	Assert(be->ref_count >= 0);		// Trying to unlock data more times than lock was called!!!
}


void bm_update()
{
}

char *bm_get_filename(int handle)
{
	int n;

	n = handle % MAX_BITMAPS;
	Assert(bm_bitmaps[n].handle == handle);		// INVALID BITMAP HANDLE
	return bm_bitmaps[n].filename;
}

void bm_get_palette(int handle, ubyte *pal, char *name)
{
	char *filename;
	int w,h;

	int n= handle % MAX_BITMAPS;
	Assert( bm_bitmaps[n].handle == handle );		// INVALID BITMAP HANDLE

	filename = bm_bitmaps[n].filename;

	if (name)	{
		strcpy( name, filename );
	}

	int pcx_error=pcx_read_header( filename, NULL, &w, &h, NULL, pal );
	if ( pcx_error != PCX_ERROR_NONE ){
		// Error(LOCATION, "Couldn't open '%s'\n", filename );
	}
}

// --------------------------------------------------------------------------------------
// bm_release()  - unloads the bitmap's data and entire slot, so bitmap 'n' won't be valid anymore
//                 NOTE: this releases the slot of EVERY frame in an ANI so don't pass any frame but the first
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			1 on successful release, 0 otherwise

int bm_release(int handle, int clear_render_targets)
{
	bitmap_entry *be;

	int n = handle % MAX_BITMAPS;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	be = &bm_bitmaps[n];

	if ( be->type == BM_TYPE_NONE ) {
		return 0;	// Already been released?
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE

	if ( !clear_render_targets && ((be->type == BM_TYPE_RENDER_TARGET_STATIC) || (be->type == BM_TYPE_RENDER_TARGET_DYNAMIC)) ) {
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
	if ( be->load_count > 0 )
		be->load_count--;

	if ( be->load_count != 0 ) {
		nprintf(("BmpMan", "Tried to release %s that has a load count of %d.. not releasing\n", be->filename, be->load_count + 1));
		return 0;
	}

	if ( be->type != BM_TYPE_USER )	{
		nprintf(("BmpMan", "Releasing bitmap %s in slot %i with handle %i\n", be->filename, n, handle));
	}

	// be sure that all frames of an ani are unloaded - taylor
	if ( (be->type == BM_TYPE_ANI) || (be->type == BM_TYPE_EFF) ) {
		int i, first = be->info.ani.first_frame, total = bm_bitmaps[first].info.ani.num_frames;

		for (i = 0; i < total; i++)	{
			bm_free_data(first+i, true);		// clears flags, bbp, data, etc

			memset( &bm_bitmaps[first+i], 0, sizeof(bitmap_entry) );

			bm_bitmaps[first+i].type = BM_TYPE_NONE;
			bm_bitmaps[first+i].comp_type = BM_TYPE_NONE;
			bm_bitmaps[first+i].dir_type = CF_TYPE_ANY;
			// Fill in bogus structures!

			// For debugging:
			strcpy_s( bm_bitmaps[first+i].filename, "IVE_BEEN_RELEASED!" );
			bm_bitmaps[first+i].signature = 0xDEADBEEF;									// a unique signature identifying the data
			bm_bitmaps[first+i].palette_checksum = 0xDEADBEEF;							// checksum used to be sure bitmap is in current palette

			// bookeeping
			bm_bitmaps[first+i].ref_count = -1;									// Number of locks on bitmap.  Can't unload unless ref_count is 0.

			// Stuff needed for animations
			bm_bitmaps[first+i].info.ani.first_frame = -1;

			bm_bitmaps[first+i].handle = -1;
		}
	} else {
		bm_free_data(n, true);		// clears flags, bbp, data, etc

		memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );

		bm_bitmaps[n].type = BM_TYPE_NONE;
		bm_bitmaps[n].comp_type = BM_TYPE_NONE;
		bm_bitmaps[n].dir_type = CF_TYPE_ANY;
		// Fill in bogus structures!

		// For debugging:
		strcpy_s( bm_bitmaps[n].filename, "IVE_BEEN_RELEASED!" );
		bm_bitmaps[n].signature = 0xDEADBEEF;									// a unique signature identifying the data
		bm_bitmaps[n].palette_checksum = 0xDEADBEEF;							// checksum used to be sure bitmap is in current palette

		// bookeeping
		bm_bitmaps[n].ref_count = -1;									// Number of locks on bitmap.  Can't unload unless ref_count is 0.

		// Stuff needed for animations
		bm_bitmaps[n].info.ani.first_frame = -1;

		bm_bitmaps[n].handle = -1;
	}

	return 1;
}

// --------------------------------------------------------------------------------------
// bm_unload()  - unloads the data, but not the bitmap info.
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			0		=>		unload failed
//						1		=>		unload successful
//
int bm_unload( int handle, int clear_render_targets )
{
	bitmap_entry *be;
	bitmap *bmp;

	int n = handle % MAX_BITMAPS;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	be = &bm_bitmaps[n];
	bmp = &be->bm;

	if ( !clear_render_targets && ((be->type == BM_TYPE_RENDER_TARGET_STATIC) || (be->type == BM_TYPE_RENDER_TARGET_DYNAMIC)) ) {
		return -1;
	}

	if ( be->type == BM_TYPE_NONE ) {
		return -1;		// Already been released
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE!

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "Tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return 0;
	}

	// kind of like ref_count except it gets around the lock/unlock usage problem
	// this gets set for each bm_load() call so we can make sure and not unload it
	// from memory, even if we *can*, until it's really not needed anymore
	if (!Bm_ignore_load_count) {
		if ( be->load_count > 0 )
			be->load_count--;

		if ( be->load_count != 0 ) {
			nprintf(("BmpMan", "Tried to unload %s that has a load count of %d.. not unloading\n", be->filename, be->load_count + 1));
			return 0;
		}
	}

	// be sure that all frames of an ani are unloaded - taylor
	if ( (be->type == BM_TYPE_ANI) || (be->type == BM_TYPE_EFF) ) {
		int i, first = be->info.ani.first_frame;

		// for the unload all case, don't try to unload every frame of every frame
		// all additional frames automatically get unloaded with the first one
		if ( (n > be->info.ani.first_frame) && (bm_bitmaps[first].bm.data == 0) )
			return 1;

		for (i = 0; i < bm_bitmaps[first].info.ani.num_frames; i++) {
			nprintf(("BmpMan", "Unloading %s frame %d.  %dx%dx%d\n", be->filename, i, bmp->w, bmp->h, bmp->bpp));
			bm_free_data(first+i);		// clears flags, bbp, data, etc
		}
	} else {
		nprintf(("BmpMan", "Unloading %s.  %dx%dx%d\n", be->filename, bmp->w, bmp->h, bmp->bpp));
		bm_free_data(n);		// clears flags, bbp, data, etc
	}

	return 1;
}

// just like bm_unload() except that it doesn't care about what load_count is
// and will just plow through and release the data anyway
// (NOTE that bm_free_data_fast() is used here and NOT bm_free_data()!)
int bm_unload_fast( int handle, int clear_render_targets )
{
	bitmap_entry *be;
	bitmap *bmp;

	int n = handle % MAX_BITMAPS;


	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	be = &bm_bitmaps[n];
	bmp = &be->bm;

	if ( !clear_render_targets && ((be->type == BM_TYPE_RENDER_TARGET_STATIC) || (be->type == BM_TYPE_RENDER_TARGET_DYNAMIC)) ) {
		return -1;
	}

	if ( be->type == BM_TYPE_NONE ) {
		return -1;		// Already been released
	}

	if ( be->type == BM_TYPE_USER ) {
		return -1;
	}

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "Tried to unload_fast %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return 0;
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE!

	// unlike bm_unload(), we handle each frame of an animation separately, for safer use in the graphics API
	nprintf(("BmpMan", "Fast-unloading %s.  %dx%dx%d\n", be->filename, bmp->w, bmp->h, bmp->bpp));
	bm_free_data_fast(n);		// clears flags, bbp, data, etc

	return 1;
}

// unload all used bitmaps
void bm_unload_all()
{
	int i;

	// since bm_unload_all() should only be called from game_shutdown() it should be
	// safe to ignore load_count's and unload anyway
	Bm_ignore_load_count = 1;

	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			bm_unload(bm_bitmaps[i].handle, 1);
		}
	}

	Bm_ignore_load_count = 0;
}


DCF(bmpman,"Shows/changes bitmap caching parameters and usage")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "flush" ))	{
			dc_printf( "Total RAM usage before flush: %d bytes\n", bm_texture_ram );
			int i;
			for (i = 0; i < MAX_BITMAPS; i++)	{
				if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
					bm_free_data(i);
				}
			}
			dc_printf( "Total RAM after flush: %d bytes\n", bm_texture_ram );
		} else if ( !strcmp( Dc_arg, "ram" ))	{
			dc_get_arg(ARG_INT);
			Bm_max_ram = Dc_arg_int*1024*1024;
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: BmpMan keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "BmpMan flush    Unloads all bitmaps.\n" );
		dc_printf( "BmpMan ram x    Sets max mem usage to x MB. (Set to 0 to have no limit.)\n" );
		dc_printf( "\nUse '? BmpMan' to see status of Bitmap manager.\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Total RAM usage: %d bytes\n", bm_texture_ram );


		if ( Bm_max_ram > 1024*1024 )
			dc_printf( "Max RAM allowed: %.1f MB\n", i2fl(Bm_max_ram)/(1024.0f*1024.0f) );
		else if ( Bm_max_ram > 1024 )
			dc_printf( "Max RAM allowed: %.1f KB\n", i2fl(Bm_max_ram)/(1024.0f) );
		else if ( Bm_max_ram > 0 )
			dc_printf( "Max RAM allowed: %d bytes\n", Bm_max_ram );
		else
			dc_printf( "No RAM limit\n" );


	}
}

// Marks a texture as being used for this level
void bm_page_in_texture( int bitmapnum, int nframes )
{
	int i;
	int n = bitmapnum % MAX_BITMAPS;

	if (bitmapnum < 0)
		return;

	Assertion( (bm_bitmaps[n].handle == bitmapnum), "bitmapnum = %i, n = %i, bm_bitmaps[n].handle = %i", bitmapnum, n, bm_bitmaps[n].handle );

	if (nframes <= 0) {
		if ( (bm_bitmaps[n].type == BM_TYPE_ANI) || (bm_bitmaps[n].type == BM_TYPE_EFF) )
			nframes = bm_bitmaps[n].info.ani.num_frames;
		else
			nframes = 1;
	}

	for (i = 0; i < nframes;i++) {
		bm_bitmaps[n+i].preloaded = 1;

		bm_bitmaps[n+i].preload_count++;

		bm_bitmaps[n+i].used_flags = BMP_TEX_OTHER;

		//check if its compressed
		switch (bm_bitmaps[n+i].comp_type)
		{
			case BM_TYPE_NONE:
				continue;

			case BM_TYPE_DXT1:
				bm_bitmaps[n+i].used_flags = BMP_TEX_DXT1;
				continue;

			case BM_TYPE_DXT3:
				bm_bitmaps[n+i].used_flags = BMP_TEX_DXT3;
				continue;

			case BM_TYPE_DXT5:
				bm_bitmaps[n+i].used_flags = BMP_TEX_DXT5;
				continue;

			case BM_TYPE_CUBEMAP_DXT1:
			case BM_TYPE_CUBEMAP_DXT3:
			case BM_TYPE_CUBEMAP_DXT5:
				bm_bitmaps[n+i].used_flags = BMP_TEX_CUBEMAP;
				continue;
		}
	}
}

// marks a texture as being a transparent textyre used for this level
// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_page_in_xparent_texture( int bitmapnum, int nframes)
{
	int i;
	int n = bitmapnum % MAX_BITMAPS;

	if (n == -1)
		return;

	Assert( bm_bitmaps[n].handle == bitmapnum );

	for (i = 0; i < nframes; i++) {
		bm_bitmaps[n+i].preloaded = 3;

		bm_bitmaps[n+i].preload_count++;

		bm_bitmaps[n+i].used_flags = BMP_TEX_XPARENT;

		//check if its compressed
		switch (bm_bitmaps[n+i].comp_type)
		{
			case BM_TYPE_NONE:
				continue;

			case BM_TYPE_DXT1:
				bm_bitmaps[n+i].used_flags = BMP_TEX_DXT1;
				continue;

			case BM_TYPE_DXT3:
				bm_bitmaps[n+i].used_flags = BMP_TEX_DXT3;
				continue;

			case BM_TYPE_DXT5:
				bm_bitmaps[n+i].used_flags = BMP_TEX_DXT5;
				continue;

			case BM_TYPE_CUBEMAP_DXT1:
			case BM_TYPE_CUBEMAP_DXT3:
			case BM_TYPE_CUBEMAP_DXT5:
				bm_bitmaps[n+i].used_flags = BMP_TEX_CUBEMAP;
				continue;
		}
	}
}

// Marks an aabitmap as being used for this level
void bm_page_in_aabitmap( int bitmapnum, int nframes )
{
	int i;
	int n = bitmapnum % MAX_BITMAPS;

	if (n == -1)
		return;

	Assert( bm_bitmaps[n].handle == bitmapnum );

	for (i=0; i<nframes;i++ )	{
		bm_bitmaps[n+i].preloaded = 2;

		bm_bitmaps[n+i].preload_count++;
	
		bm_bitmaps[n+i].used_flags = BMP_AABITMAP;
	}
}



// Tell the bitmap manager to start keeping track of what bitmaps are used where.
void bm_page_in_start()
{
	int i;

	Bm_paging = 1;

	// Mark all as inited
	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( !Cmdline_cache_bitmaps && (bm_bitmaps[i].type != BM_TYPE_NONE) )	{
			bm_unload_fast(bm_bitmaps[i].handle);
		}
		bm_bitmaps[i].preloaded = 0;
		bm_bitmaps[i].preload_count = 0;
#ifdef BMPMAN_NDEBUG
		bm_bitmaps[i].used_count = 0;
#endif
		bm_bitmaps[i].used_flags = 0;
	}

	gr_bm_page_in_start();
}

extern int Multi_ping_timestamp;
extern void multi_ping_send_all();

void bm_page_in_stop()
{	
	int i;	

#ifndef NDEBUG
	char busy_text[60];
#endif

	nprintf(( "BmpInfo","BMPMAN: Loading all used bitmaps.\n" ));

	// Load all the ones that are supposed to be loaded for this level.
	int n = 0;

	int bm_preloading = 1;

	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && (bm_bitmaps[i].type != BM_TYPE_RENDER_TARGET_DYNAMIC) && (bm_bitmaps[i].type != BM_TYPE_RENDER_TARGET_STATIC) ) {
			if ( bm_bitmaps[i].preloaded )	{
				if ( bm_preloading ) {
					if ( !gr_preload(bm_bitmaps[i].handle, (bm_bitmaps[i].preloaded==2)) )	{
						mprintf(( "Out of VRAM.  Done preloading.\n" ));
						bm_preloading = 0;
					}
				} else {
					bm_lock( bm_bitmaps[i].handle, (bm_bitmaps[i].used_flags == BMP_AABITMAP) ? 8 : 16, bm_bitmaps[i].used_flags );
					bm_unlock( bm_bitmaps[i].handle );
				}

				n++;

				// send out a ping if we are multi so that psnet2 doesn't kill us off for a long load
				// NOTE that we can't use the timestamp*() functions here since they won't increment
				//      during this loading process
				if (Game_mode & GM_MULTIPLAYER) {
					if ( (Multi_ping_timestamp == -1) || (Multi_ping_timestamp <= timer_get_milliseconds()) ) {
						multi_ping_send_all();
						Multi_ping_timestamp = timer_get_milliseconds() + 10000; // timeout is 10 seconds between pings
					}
				}

				if ( (bm_bitmaps[i].info.ani.first_frame == 0) || (bm_bitmaps[i].info.ani.first_frame == i) ) {
#ifndef NDEBUG
					memset(busy_text, 0, sizeof(busy_text));

					strcat_s( busy_text, "** BmpMan: " );
					strcat_s( busy_text, bm_bitmaps[i].filename );
					strcat_s( busy_text, " **" );

					game_busy(busy_text);
#else
					game_busy();
#endif
				}
			} else {
				bm_unload_fast(bm_bitmaps[i].handle);
			}
		}
	}

	nprintf(( "BmpInfo","BMPMAN: Loaded %d bitmaps that are marked as used for this level.\n", n ));

	int total_bitmaps = 0;
	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			total_bitmaps++;
		}
		if ( bm_bitmaps[i].type == BM_TYPE_USER )	{
			mprintf(( "User bitmap '%s'\n", bm_bitmaps[i].filename ));
		}
	}	

	mprintf(( "Bmpman: %d/%d bitmap slots in use.\n", total_bitmaps, MAX_BITMAPS ));
	//mprintf(( "Bmpman: Usage went from %d KB to %d KB.\n", usage_before/1024, usage_after/1024 ));

	Bm_paging = 0;
}

// for unloading bitmaps while a mission is going
int bm_page_out( int bitmap_id )
{
	int n = bitmap_id % MAX_BITMAPS;

	Assert( n >= 0 && n < MAX_BITMAPS );

	// in case it's already been released
	if ( bm_bitmaps[n].type == BM_TYPE_NONE )
		return 0;


	Assert( bm_bitmaps[n].handle == bitmap_id );	// INVALID BITMAP HANDLE

	// it's possible to hit < 0 here when model_page_out_textures() is called from
	// anywhere other than in a mission
	if ( bm_bitmaps[n].preload_count > 0 ) {
		nprintf(("BmpMan", "PAGE-OUT: %s - preload_count remaining: %d\n", bm_bitmaps[n].filename, bm_bitmaps[n].preload_count));

		// lets decrease it for next time around
		bm_bitmaps[n].preload_count--;

		return 0;
	}

	return ( bm_unload(bitmap_id) == 1 );
}

int bm_get_cache_slot( int bitmap_id, int separate_ani_frames )
{
	int n = bitmap_id % MAX_BITMAPS;

	Assert( bm_bitmaps[n].handle == bitmap_id );		// INVALID BITMAP HANDLE

	bitmap_entry	*be = &bm_bitmaps[n];

	if ( (!separate_ani_frames) && ((be->type == BM_TYPE_ANI) || (be->type == BM_TYPE_EFF)) )	{
		return be->info.ani.first_frame;
	} 

	return n;

}

void (*bm_set_components)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a) = NULL;
void (*bm_set_components_32)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a) = NULL;

void bm_set_components_argb_16_screen(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	if ( *av == 0 ) {
		*((unsigned short*)pixel) = (unsigned short)Gr_current_green->mask;
		return;
	}

	*((unsigned short*)pixel) = (unsigned short)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((unsigned short*)pixel) |= (unsigned short)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((unsigned short*)pixel) |= (unsigned short)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);	
}

void bm_set_components_argb_32_screen(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	if ( *av == 0 ) {
		*((unsigned int*)pixel) = (unsigned int)Gr_current_green->mask;
		return;
	}

	*((unsigned int*)pixel) = (unsigned int)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((unsigned int*)pixel) |= (unsigned int)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((unsigned int*)pixel) |= (unsigned int)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
}

void bm_set_components_argb_16_tex(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	if ( *av == 0 ) {
		*((unsigned short*)pixel) = 0;
		return;
	}

	*((unsigned short*)pixel) = (unsigned short)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((unsigned short*)pixel) |= (unsigned short)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((unsigned short*)pixel) |= (unsigned short)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	*((unsigned short*)pixel) |= (unsigned short)(Gr_current_alpha->mask);
}

void bm_set_components_argb_32_tex(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	if ( *av == 0 ) {
		*((unsigned int*)pixel) = 0;
		return;
	}

	*((unsigned int*)pixel) = (unsigned int)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((unsigned int*)pixel) |= (unsigned int)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((unsigned int*)pixel) |= (unsigned int)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	*((unsigned int*)pixel) |= (unsigned int)(Gr_current_alpha->mask);
}

// for selecting pixel formats
void BM_SELECT_SCREEN_FORMAT()
{
	Gr_current_red = &Gr_red;
	Gr_current_green = &Gr_green;
	Gr_current_blue = &Gr_blue;
	Gr_current_alpha = &Gr_alpha;

	// setup pointers
	bm_set_components_32 = bm_set_components_argb_32_screen;
	// should always assume that 16-bit is the default request
	bm_set_components = bm_set_components_argb_16_screen;
}

void BM_SELECT_TEX_FORMAT()
{
	Gr_current_red = &Gr_t_red; 
	Gr_current_green = &Gr_t_green; 
	Gr_current_blue = &Gr_t_blue; 
	Gr_current_alpha = &Gr_t_alpha;

	// setup pointers
	bm_set_components_32 = bm_set_components_argb_32_tex;
	// should always assume that 16-bit is the default request
	bm_set_components = bm_set_components_argb_16_tex;
}

void BM_SELECT_ALPHA_TEX_FORMAT()
{
	Gr_current_red = &Gr_ta_red; 
	Gr_current_green = &Gr_ta_green; 
	Gr_current_blue = &Gr_ta_blue; 
	Gr_current_alpha = &Gr_ta_alpha;

	// setup pointers
	bm_set_components_32 = bm_set_components_argb_32_tex;
	// should always assume that 16-bit is the default request
	bm_set_components = bm_set_components_argb_16_tex;
}

// get the rgba components of a pixel, any of the parameters can be NULL
void bm_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a)
{
	int bit_32 = 0;

	if((gr_screen.bits_per_pixel==32) && (Gr_current_red == &Gr_red)){
		bit_32 = 1;
	}


	if(r != NULL){
		if(bit_32){
			*r = ubyte(( (*((unsigned int*)pixel) & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		} else {
			*r = ubyte(( ( ((unsigned short*)pixel)[0] & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		}
	}
	if(g != NULL){
		if(bit_32){
			*g = ubyte(( (*((unsigned int*)pixel) & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		} else {
			*g = ubyte(( ( ((unsigned short*)pixel)[0] & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		}
	}
	if(b != NULL){
		if(bit_32){
			*b = ubyte(( (*((unsigned int*)pixel) & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		} else {
			*b = ubyte(( ( ((unsigned short*)pixel)[0] & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		}
	}

	// get the alpha value
	if(a != NULL){		
		*a = 1;
		
		Assert(!bit_32);
		if(!( ((unsigned short*)pixel)[0] & 0x8000)){
			*a = 0;
		} 
	}
}

// get filename
void bm_get_filename(int bitmapnum, char *filename)
{
	if(!bm_is_valid(bitmapnum))
	{
		strcpy(filename, "");
		return;
	}

	int n = bitmapnum % MAX_BITMAPS;

	// return filename
	strcpy(filename, bm_bitmaps[n].filename);
}

// needed only for compressed bitmaps
int bm_is_compressed(int num)
{
	int n = num % MAX_BITMAPS;
	ubyte type = BM_TYPE_NONE;

	//duh
	if (!Use_compressed_textures)
		return 0;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	Assert(num == bm_bitmaps[n].handle);

	type = bm_bitmaps[n].comp_type;

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

		case BM_TYPE_CUBEMAP_DXT1:
			return DDS_CUBEMAP_DXT1;

		case BM_TYPE_CUBEMAP_DXT3:
			return DDS_CUBEMAP_DXT3;

		case BM_TYPE_CUBEMAP_DXT5:
			return DDS_CUBEMAP_DXT5;
	}

	return 0;
}

int bm_has_alpha_channel(int handle)
{
	int n = handle % MAX_BITMAPS;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	Assert( handle == bm_bitmaps[n].handle );

	// assume that PCX never has a real alpha channel (it may be 32-bit, but without any alpha)
	if (bm_bitmaps[n].type == BM_TYPE_PCX)
		return 0;

	return (bm_bitmaps[n].bm.true_bpp == 32);
}

// the only real purpose for this is to return the correct TCACHE_TYPE for compressed graphics,
// uncompressed graphics are assumed to be of type NORMAL.  The only other real format to check
// for is TCACHE_TYPE_SECTIONED - taylor
int bm_get_tcache_type(int num)
{
	if ( bm_is_compressed(num) )
		return TCACHE_TYPE_COMPRESSED;

//	if ( bm_is_render_target(num) )
//		return TCACHE_TYPE_RENDER_TARGET;

	return TCACHE_TYPE_NORMAL;
}

int bm_get_size(int num)
{
	int n = num % MAX_BITMAPS;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	Assert(num == bm_bitmaps[n].handle);

	return bm_bitmaps[n].mem_taken;
}

int bm_get_num_mipmaps(int num)
{
	int n = num % MAX_BITMAPS;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );
	Assert( num == bm_bitmaps[n].handle );

	if (bm_bitmaps[n].num_mipmaps == 0)
		return 1;

	return bm_bitmaps[n].num_mipmaps;
}

// convert an 8-bit (256 color) image to a 24-bit BGR image sending new bitmap data to "out_data"
int bm_convert_color_index_to_BGR(int num, ubyte **out_data)
{
	int n = num % MAX_BITMAPS;
	bitmap_entry *be;
	bitmap *bmp;
	ubyte *datap, *bgr_data = NULL, *palette = NULL;
	char filename[MAX_FILENAME_LEN];
	int i, j, bpp = 0, size = 0;
	int index = 0, mult = 3;

	Assert( out_data != NULL );
	Assert( num == bm_bitmaps[n].handle );

	if ( num != bm_bitmaps[n].handle )
		return 1;

	be = &bm_bitmaps[n];
	bmp = &be->bm;
 
	if ( (bmp->bpp != 8) || !(bmp->data) || ((be->type != BM_TYPE_DDS) && (be->type != BM_TYPE_PCX)) )
	{
		return 1;
	}

	// it's up to the calling function to free() this but not to malloc() it!!
	bgr_data = (ubyte*)vm_malloc_q( bmp->w * bmp->h * 3 );

	ubyte *in_data = (ubyte*)bmp->data;

	if ( bgr_data == NULL )
		return 1;

	memset( bgr_data, 0, bmp->w * bmp->h * 3 );

	palette = new ubyte[1024]; // 256*4, largest size we should have to process
	Assert( palette != NULL );

	// make sure we are using the correct filename in the case of an EFF.
	// this will populate filename[] whether it's EFF or not
	EFF_FILENAME_CHECK;

	if ( be->type == BM_TYPE_PCX ) {
		pcx_read_header( filename, NULL, NULL, NULL, &bpp, palette );
		mult = 3; // PCX has RGB for 256 entries
	} else if ( be->type == BM_TYPE_DDS ) {
		dds_read_header( filename, NULL, NULL, NULL, &bpp, NULL, NULL, &size, palette );
		mult = 4; // DDS has RGBX for 256 entries, 'X' being an alpha setting that we don't need
	} else {
		// we really shouldn't be here at this point but give it another check anyway
		delete[] palette;
		vm_free(bgr_data);
		return 1;
	}

	Assert( bpp == 8 );

	// we can only accept 8bits obviously, but this is actually a read error check
	if ( bpp != 8 ) {
		delete[] palette;
		vm_free(bgr_data);
		return 1;
	}

	datap = bgr_data;

	for (i = 0; i < bmp->h; i++) {
		for (j = 0; j < bmp->w; j++) {
			index = *in_data++;
			*datap++ = palette[index * mult + 2];
			*datap++ = palette[index * mult + 1];
			*datap++ = palette[index * mult];
		}
	}

	*out_data = bgr_data;

	delete[] palette;

	// no errors
	return 0;
}

// list of all bitmaps loaded, but not necessarily in memory
// used to debug bmpman after a mission load
void bm_print_bitmaps()
{
#ifdef BMPMAN_NDEBUG
	int i;

	for (i=0; i<MAX_BITMAPS; i++) {
		if (bm_bitmaps[i].type != BM_TYPE_NONE) {
			if (bm_bitmaps[i].data_size) {
				nprintf(("BMP DEBUG", "BMPMAN = num: %d, name: %s, handle: %d - (%s) size: %.3fM\n", i, bm_bitmaps[i].filename, bm_bitmaps[i].handle, bm_bitmaps[i].data_size ? NOX("*LOCKED*") : NOX(""), ((float)bm_bitmaps[i].data_size/1024.0f)/1024.0f));
			} else {
				nprintf(("BMP DEBUG", "BMPMAN = num: %d, name: %s, handle: %d\n", i, bm_bitmaps[i].filename, bm_bitmaps[i].handle));
			}
		}
	}
	nprintf(("BMP DEBUG", "BMPMAN = LOCKED memory usage: %.3fM\n", ((float)bm_texture_ram/1024.0f)/1024.0f));
#endif
}

// this will create a render target as close to the desiered resolution as posable of the following base types:
//  - BMP_FLAG_RENDER_TARGET_STATIC
//      static render targets are ones that you intend to draw to once or not very often in game
//  - BMP_FLAG_RENDER_TARGET_DYNAMIC
//     dynamic render targets are ones that you will be drawing to all the time (like once per frame)

int bm_make_render_target( int width, int height, int flags )
{
	int i, n;
	int mm_lvl = 0;
	// final w and h may be different from passed width and height
	int w = width, h = height;
	ubyte bpp = 32;
	int size = 0;

	if ( !bm_inited )
		bm_init();

	// Find an open slot (starting from the end)
	for (n = -1, i = MAX_BITMAPS-1; i >= 0; i-- ) {
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			n = i;
			break;
		}
	}

	// Out of bitmap slots
	if ( n == -1 )
		return -1;


	if ( !gr_bm_make_render_target(n, &w, &h, &bpp, &mm_lvl, flags) )
		return -1;


	Assert( mm_lvl > 0 );

	if (flags & BMP_FLAG_RENDER_TARGET_STATIC) {
		// data size
		size = (w * h * (bpp >> 3));

		if (mm_lvl > 1) {
			// include size of all mipmap levels (there should be a full chain)
			size += (size/3) - 1;
		}

		// make sure to count all faces if a cubemap
		if (flags & BMP_FLAG_CUBEMAP) {
			size *= 6;
		}
	}

	// ensure fields are cleared out from previous bitmap
	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );

	bm_bitmaps[n].type = (flags & BMP_FLAG_RENDER_TARGET_STATIC) ? BM_TYPE_RENDER_TARGET_STATIC : BM_TYPE_RENDER_TARGET_DYNAMIC;
	bm_bitmaps[n].signature = Bm_next_signature++;
	sprintf( bm_bitmaps[n].filename, "RT_%dx%d+%d", w, h, bpp );
	bm_bitmaps[n].bm.w = (short) w;
	bm_bitmaps[n].bm.h = (short) h;
	bm_bitmaps[n].bm.rowsize = (short) w;
	bm_bitmaps[n].bm.bpp = (ubyte) bpp;
	bm_bitmaps[n].bm.true_bpp = (ubyte) bpp;
	bm_bitmaps[n].bm.flags = (ubyte) flags;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;
	bm_bitmaps[n].num_mipmaps = mm_lvl;
	bm_bitmaps[n].mem_taken = size;
	bm_bitmaps[n].dir_type = CF_TYPE_ANY;

	bm_bitmaps[n].palette_checksum = 0;
	bm_bitmaps[n].handle = bm_get_next_handle() * MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;

	if (bm_bitmaps[n].mem_taken) {
		bm_bitmaps[n].bm.data = (ptr_u) bm_malloc(n, bm_bitmaps[n].mem_taken);
	}
	//	bm_update_memory_used( n, bm_bitmaps[n].mem_taken );

	return bm_bitmaps[n].handle;
}

int bm_is_render_target(int bitmap_id)
{
	int n = bitmap_id % MAX_BITMAPS;

	Assert(bitmap_id == bm_bitmaps[n].handle);

	if ( !((bm_bitmaps[n].type == BM_TYPE_RENDER_TARGET_STATIC) || (bm_bitmaps[n].type == BM_TYPE_RENDER_TARGET_DYNAMIC)) ) {
		return 0;
	}

	return bm_bitmaps[n].type;
}

int bm_set_render_target(int handle, int face)
{
	int n = handle % MAX_BITMAPS;

	if ( n >= 0 ) {
		Assert( handle == bm_bitmaps[n].handle );

		if ( (bm_bitmaps[n].type != BM_TYPE_RENDER_TARGET_STATIC) && (bm_bitmaps[n].type != BM_TYPE_RENDER_TARGET_DYNAMIC) ) {
			// odds are that someone passed a normal texture created with bm_load()
			mprintf(("Trying to set invalid bitmap (slot: %i, handle: %i) as render target!\n", n, handle));
			return 0;
		}
	}

	if ( gr_bm_set_render_target(n, face) ) {
		if (gr_screen.rendering_to_texture == -1) {
			//if we are moveing from the back buffer to a texture save whatever the current settings are
			gr_screen.save_max_w = gr_screen.max_w;
			gr_screen.save_max_h = gr_screen.max_h;
		}

		if (n < 0) {
			gr_screen.max_w = gr_screen.save_max_w;
			gr_screen.max_h = gr_screen.save_max_h;
		} else {
			gr_screen.max_w = bm_bitmaps[n].bm.w;
			gr_screen.max_h = bm_bitmaps[n].bm.h;
		}

		gr_screen.rendering_to_face = face;
		gr_screen.rendering_to_texture = n;

		gr_reset_clip();

		if (gr_screen.mode == GR_OPENGL) {
			extern void opengl_setup_viewport();
			opengl_setup_viewport();
		}

		return 1;
	}

	return 0;
}


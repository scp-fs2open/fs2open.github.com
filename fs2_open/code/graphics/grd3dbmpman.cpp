/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

// D3D8 includes
#include <d3d8.h>
#include <d3dx8.h>

#include <ctype.h>
#include "globalincs/pstypes.h"
#include "pcxutils/pcxutils.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/2d.h"
#include "anim/animplay.h"
#include "io/timer.h"
#include "globalincs/systemvars.h"
#include "io/key.h"
#include "anim/packunpack.h"
#include "cfile/cfile.h"
#include "graphics/grinternal.h"
#include "graphics/grd3dinternal.h"
#include "ship/ship.h"
#include "debugconsole/dbugfile.h"
#include "bmpman/bmpman.h"
#include "Cmdline/cmdline.h"

#include "graphics/grd3dbmpman.h"

int max_bitmap_size = 256;

void bm_d3d_set_max_bitmap_size(int size)
{
	DBUGFILE_OUTPUT_1("Setting max bitmap size %d", size);
	max_bitmap_size = size;
}

extern bitmap_entry bm_bitmaps[];

typedef struct {

	IDirect3DBaseTexture8 *tinterface;
	float uscale, vscale;

} D3DBitmapData;

D3DBitmapData d3d_bitmap_entry[MAX_BITMAPS];

// keep this defined to use per-ship nondarkening pixels
#define BMPMAN_SPECIAL_NONDARK

int bm_d3d_inited = 0;
int bm_d3d_next_handle = 1;

// 16 bit pixel formats
//extern int Bm_pixel_format;

int bm_d3d_get_next_handle()
{
	int n = bm_d3d_next_handle;
	bm_d3d_next_handle++;
	if ( bm_d3d_next_handle > 30000 )	{
		bm_d3d_next_handle = 1;
	}
	return n;
}

// Frees a bitmaps data if it should, and
// Returns true if bitmap n can free it's data.
static void bm_d3d_free_data(int n)
{
	bitmap_entry	*be;
	bitmap			*bmp;

	Assert( n >= 0 && n < MAX_BITMAPS );

	be = &bm_bitmaps[n];
	bmp = &be->bm;

	if(d3d_bitmap_entry[n].tinterface != NULL) {
	   	d3d_bitmap_entry[n].tinterface->Release();
		d3d_bitmap_entry[n].tinterface = NULL;
	}

	// If there isn't a bitmap in this structure, don't
	// do anything but clear out the bitmap info
	if ( be->type==BM_TYPE_NONE) 
		goto SkipFree;

	// If this bitmap doesn't have any data to free, skip
	// the freeing it part of this.
	if ( bmp->data == 0 ) 
		goto SkipFree;

	// Don't free up memory for user defined bitmaps, since
	// BmpMan isn't the one in charge of allocating/deallocing them.
	if ( ( be->type==BM_TYPE_USER ) )	
		goto SkipFree;

	// Free up the data now!

	//	mprintf(( "Bitmap %d freed %d bytes\n", n, bm_bitmaps[n].data_size ));
	if(bmp->data != NULL) {
		free((void *)bmp->data);
	}

SkipFree:

	// Clear out & reset the bitmap data structure
	bmp->flags = 0;
	bmp->bpp = 0;
	bmp->data = 0;
	bmp->palette = NULL;
}

static void *bm_d3d_malloc( int n, int size )
{
	Assert( n >= 0 && n < MAX_BITMAPS );
//	mprintf(( "Bitmap %d allocated %d bytes\n", n, size ));
	return malloc(size);
}

void bm_d3d_close()
{
	if ( bm_d3d_inited )	{
		for (int i=0; i<MAX_BITMAPS; i++ )	{
			bm_d3d_free_data(i);			// clears flags, bbp, data, etc
		}
		bm_d3d_inited = 0;
	}
}

void bm_d3d_init()
{
	if (!bm_d3d_inited)	{
		bm_d3d_inited = 1;
		atexit(bm_d3d_close);
	}
	
	for (int i = 0; i < MAX_BITMAPS; i++ ) {
		bm_bitmaps[i].filename[0] = '\0';
		bm_bitmaps[i].type = BM_TYPE_NONE;
		bm_bitmaps[i].info.user.data = NULL;
		bm_bitmaps[i].bm.data = 0;
		bm_bitmaps[i].bm.palette = NULL;

		d3d_bitmap_entry[i].tinterface = NULL;
		   
		bm_d3d_free_data(i);  	// clears flags, bbp, data, etc
	}
}

void bm_d3d_get_frame_usage(int *ntotal, int *nnew)
{
}

// given a loaded bitmap with valid info, calculate sections
void bm_d3d_calc_sections(bitmap *be)
{
	int idx;

	// number of x and y sections
	be->sections.num_x = (ubyte)(be->w / max_bitmap_size);
	if((be->sections.num_x * max_bitmap_size) < be->w){
		be->sections.num_x++;
	}
	be->sections.num_y = (ubyte)(be->h / max_bitmap_size);
	if((be->sections.num_y * max_bitmap_size) < be->h){
		be->sections.num_y++;
	}

	// calculate the offsets for each section
	for(idx=0; idx<be->sections.num_x; idx++){
		be->sections.sx[idx] = (ushort)(max_bitmap_size * idx);
	}
	for(idx=0; idx<be->sections.num_y; idx++){
		be->sections.sy[idx] = (ushort)(max_bitmap_size * idx);
	}
}

// Creates a bitmap that exists in RAM somewhere, instead
// of coming from a disk file.  You pass in a pointer to a
// block of 32 (or 8)-bit-per-pixel data.  Right now, the only
// bpp you can pass in is 32 or 8.  On success, it returns the
// bitmap number.  You cannot free that RAM until bm_release
// is called on that bitmap.
int bm_d3d_create( int bpp, int w, int h, void * data, int flags )
{

	Assert(bm_d3d_inited);

	int i, n, first_slot = MAX_BITMAPS;

	if(bpp == 8){
		Assert(flags & BMP_AABITMAP);
	} else {
		Assert(bpp == 16);
	}

	for (i = MAX_BITMAPS-1; i >= 0; i-- ) {
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			first_slot = i;
			break;
		}
	}

	Assert(d3d_bitmap_entry[i].tinterface == NULL);


	n = first_slot;
	Assert( n > -1 );

	// Out of bitmap slots
	if ( n == -1 ) return -1;

	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );

	sprintf( bm_bitmaps[n].filename, "TMP%dx%d", w, h );
	bm_bitmaps[n].type = BM_TYPE_USER;
	bm_bitmaps[n].palette_checksum = 0;

	bm_bitmaps[n].bm.w = (short) w;
	bm_bitmaps[n].bm.h = (short) h;
	bm_bitmaps[n].bm.rowsize = (short) w;
	bm_bitmaps[n].bm.bpp = (unsigned char) bpp;
	bm_bitmaps[n].bm.flags = 0;
	bm_bitmaps[n].bm.flags |= flags;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;

	bm_bitmaps[n].info.user.bpp = ubyte(bpp);
	bm_bitmaps[n].info.user.data = data;
	bm_bitmaps[n].info.user.flags = ubyte(flags);

	bm_bitmaps[n].handle = bm_get_next_handle()*MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;

	d3d_bitmap_entry[n].tinterface = NULL;

	// fill in section info
	bm_d3d_calc_sections(&bm_bitmaps[n].bm);
	
	return bm_bitmaps[n].handle;
}

// sub helper function. Given a raw filename and an extension, try and find the bitmap
// returns -1 if it could not be found
//          0 if it was found as a file
//          1 if it already exists, fills in handle
int Bm_d3d_ignore_duplicates = 0;
int bm_d3d_load_sub(char *real_filename, const char *ext, int *handle)
{	
	int i;
	char filename[MAX_FILENAME_LEN] = "";
	
	strcpy( filename, real_filename );
	strcat( filename, ext );	
	for (i=0; i<(int)strlen(filename); i++ ){
		filename[i] = char(tolower(filename[i]));
	}		

	// try to find given filename to see if it has been loaded before
	if(!Bm_d3d_ignore_duplicates){
		for (i = 0; i < MAX_BITMAPS; i++) {
			if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && !stricmp(filename, bm_bitmaps[i].filename) ) {
				nprintf (("BmpMan", "Found bitmap %s -- number %d\n", filename, i));
				*handle = bm_bitmaps[i].handle;

				return 1;
			}
		}	
	}

	// try and find the file
	CFILE *test = cfopen(filename, "rb");
	if(test != NULL){
		cfclose(test);
		return 0;
	}

	// could not be found
	return -1;
}

// This loads a bitmap so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number.  Function doesn't acutally load the data, only
// width, height, and possibly flags.
int bm_d3d_load( char * real_filename )
{
	Assert(bm_d3d_inited);

	int i, n;
	int w, h;
	char filename[MAX_FILENAME_LEN];

	// nice little trick for keeping standalone memory usage way low - always return a bogus bitmap 
	if(Game_mode & GM_STANDALONE_SERVER){
		strcpy(filename,"test128");
	}

	// make sure no one passed an extension
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}

	// Lets find out what type it is
	int type = BM_TYPE_NONE;
	{
		bool found = false;

		//int handle = -1;
		const int NUM_TYPES	= 4;
		const int type_list[NUM_TYPES] = {BM_TYPE_TGA, BM_TYPE_JPG, BM_TYPE_DDS, BM_TYPE_PCX};
		const char *ext_list[NUM_TYPES] = {".tga", ".jpg", ".dds", ".pcx"};
		
		// Only load TGA and JPG if given flag, support DDS and PCX by default
		int i = Cmdline_jpgtga ? 0 : 2; // 2 Means start with DDS and fall back to PCX

		for(; i < NUM_TYPES; i++) {
		
			int handle = 0;
			int result = bm_d3d_load_sub(filename, ext_list[i], &handle);
			
			// Image is already loaded
			if(result == 1) {
				return handle;
			}

			// File was found
			if(result == 0)	{
				strcat(filename, ext_list[i]);
				type = type_list[i];
				found = true;
				break;
			}
		}
		
		// No match was found
		if(found == false) {
			return -1;
		}
	}

	Assert(type != BM_TYPE_NONE);

	// Find an open slot
	int first_slot = MAX_BITMAPS;
	for (i = 0; i < MAX_BITMAPS; i++) {
		if ( (bm_bitmaps[i].type == BM_TYPE_NONE) ){
			first_slot = i;
			break;
		}
	}

	Assert(d3d_bitmap_entry[i].tinterface == NULL);

	n = first_slot;
	Assert( n < MAX_BITMAPS );	

	if ( n == MAX_BITMAPS ) return -1;	


	if(type == BM_TYPE_PCX) {
		int pcx_error=pcx_read_header( filename, &w, &h, NULL );		
		if ( pcx_error != PCX_ERROR_NONE )	{
			DBUGFILE_OUTPUT_1("bm_pcx: Cant load %s",filename);
			mprintf(( "bm_pcx: Couldn't open '%s'\n", filename ));
			return -1;
		}
	} else {
		// Let D3DX handle this
		if(d3d_read_header_d3dx( filename, type, &w, &h) == false) {
			DBUGFILE_OUTPUT_1("not bm_pcx: Cant load %s",filename);
			mprintf(( "not bm_pcx: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}

	// ensure fields are cleared out from previous bitmap
	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );
	
	// Mark the slot as filled, because cf_read might load a new bitmap
	// into this slot.
	bm_bitmaps[n].type = (ubyte) type;
	Assert ( strlen(filename) < MAX_FILENAME_LEN );
	strncpy(bm_bitmaps[n].filename, filename, MAX_FILENAME_LEN-1 );
	bm_bitmaps[n].bm.w = short(w);
	bm_bitmaps[n].bm.rowsize = short(w);
	bm_bitmaps[n].bm.h = short(h);
	bm_bitmaps[n].bm.bpp = 0;
	bm_bitmaps[n].bm.flags = 0;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;

	bm_bitmaps[n].palette_checksum = 0;
	bm_bitmaps[n].handle = bm_get_next_handle()*MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;

	d3d_bitmap_entry[n].tinterface = NULL;

	bm_d3d_calc_sections(&bm_bitmaps[n].bm);
	return bm_bitmaps[n].handle;
}

// special load function. basically allows you to load a bitmap which already exists (by filename). 
// this is useful because in some cases we need to have a bitmap which is locked in screen format
// _and_ texture format, such as pilot pics and squad logos
int bm_d3d_load_duplicate(char *filename)
{
	int ret;

	// ignore duplicates
	Bm_d3d_ignore_duplicates = 1;
	
	// load
	ret = bm_load(filename);

	// back to normal
	Bm_d3d_ignore_duplicates = 0;

	return ret;
}

static int find_block_of(int n)
{
	int i, cnt, nstart;

	cnt=0;
	nstart = 0;
	for (i=0; i<MAX_BITMAPS; i++ )	{
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			if (cnt==0) nstart = i;
			cnt++;
		} else
			cnt=0;
		if ( cnt == n ) return nstart;
	}

	// Error( LOCATION, "Couldn't find block of %d frames\n", n );
	return -1;
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
int bm_d3d_load_animation( char *real_filename, int *nframes, int *fps, int can_drop_frames)
{
	Assert(bm_d3d_inited);

	int	i, n;
	anim	the_anim;
	CFILE	*fp;
	char filename[MAX_FILENAME_LEN];

	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load_animation for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}
	strcat( filename, ".ani" );

	if ( (fp = cfopen(filename, "rb")) == NULL ) {
		return -1;
	}

	int reduced = 0;
#ifndef NDEBUG
	// for debug of ANI sizes
	strcpy(the_anim.name, real_filename);
#endif
	anim_read_header(&the_anim, fp);
	cfclose(fp);

	*nframes = the_anim.total_frames;
	if ( fps != NULL )	{
		if ( reduced )	{
			*fps = the_anim.fps / 2;
		} else {
			*fps = the_anim.fps;
		}
	}

	// first check to see if this ani already has it's frames loaded
	for (i = 0; i < MAX_BITMAPS; i++) {
		if ( (bm_bitmaps[i].type == BM_TYPE_ANI) && !stricmp(filename, bm_bitmaps[i].filename) ) {
			break;
		}
	}
	
	if ( i < MAX_BITMAPS ) {
		Assert(bm_bitmaps[i].info.ani.num_frames == *nframes);
		return bm_bitmaps[i].handle;
	}

	n = find_block_of(*nframes);
	if(n < 0){
		return -1;
	}

	int first_handle = bm_get_next_handle();

	Assert ( strlen(filename) < MAX_FILENAME_LEN );
	for ( i = 0; i < *nframes; i++ ) {
		memset( &bm_bitmaps[n+i], 0, sizeof(bitmap_entry) );
		bm_bitmaps[n+i].info.ani.first_frame = n;
		bm_bitmaps[n+i].info.ani.num_frames = ubyte(the_anim.total_frames);
		bm_bitmaps[n+i].info.ani.fps = ubyte(the_anim.fps);
		bm_bitmaps[n+i].bm.w = short(the_anim.width);
		bm_bitmaps[n+i].bm.rowsize = short(the_anim.width);
		bm_bitmaps[n+i].bm.h = short(the_anim.height);
		if ( reduced )	{
			bm_bitmaps[n+i].bm.w /= 2;
			bm_bitmaps[n+i].bm.rowsize /= 2;
			bm_bitmaps[n+i].bm.h /= 2;
		}
		bm_bitmaps[n+i].bm.flags = 0;
		bm_bitmaps[n+i].bm.bpp = 0;
		bm_bitmaps[n+i].bm.data = 0;
		bm_bitmaps[n+i].bm.palette = NULL;
		bm_bitmaps[n+i].type = BM_TYPE_ANI;
		bm_bitmaps[n+i].palette_checksum = 0;
		bm_bitmaps[n+i].handle = first_handle*MAX_BITMAPS + n+i;
		bm_bitmaps[n+i].last_used = -1;

		// fill in section info
		bm_d3d_calc_sections(&bm_bitmaps[n+i].bm);

		if ( i == 0 )	{
			sprintf( bm_bitmaps[n+i].filename, "%s", filename );
		} else {
			sprintf( bm_bitmaps[n+i].filename, "%s[%d]", filename, i );
		}
	}

	return bm_bitmaps[n].handle;
}

// Gets info.   w,h,or flags,nframes or fps can be NULL if you don't care.
void bm_d3d_get_info( int handle, int *w, int * h, ubyte * flags, int *nframes, int *fps, bitmap_section_info **sections )
{
	Assert(bm_d3d_inited);

	bitmap * bmp;

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE!	
	
	if ( (bm_bitmaps[bitmapnum].type == BM_TYPE_NONE) || (bm_bitmaps[bitmapnum].handle != handle) ) {
		if (w) *w = 0;
		if (h) *h = 0;
		if (flags) *flags = 0;
		if (nframes) *nframes=0;
		if (fps) *fps=0;
		if (sections != NULL) *sections = NULL;
		return;
	}

	bmp = &(bm_bitmaps[bitmapnum].bm);

	if (w) *w = bmp->w;
	if (h) *h = bmp->h;
	if (flags) *flags = bmp->flags;
	if ( bm_bitmaps[bitmapnum].type == BM_TYPE_ANI )	{
		if (nframes) {
			*nframes = bm_bitmaps[bitmapnum].info.ani.num_frames;
		} 
		if (fps) {
			*fps= bm_bitmaps[bitmapnum].info.ani.fps;
		}
	} else {
		if (nframes) {
			*nframes = 1;
		} 
		if (fps) {
			*fps= 0;
		}
	}
	if(sections != NULL){
		*sections = &bm_bitmaps[bitmapnum].bm.sections;
	}
}

uint bm_d3d_get_signature( int handle )
{
	Assert(bm_d3d_inited);

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

	return bm_bitmaps[bitmapnum].signature;
}

static void bm_d3d_convert_format( int bitmapnum, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	int idx;	

	if(flags & BMP_AABITMAP){
   //		Assert(bmp->bpp == 8);
	} else {
		Assert(bmp->bpp == 16);
	}

	// maybe swizzle to be an xparent texture
	if(!(bmp->flags & BMP_TEX_XPARENT) && (flags & BMP_TEX_XPARENT)) {
		for(idx=0; idx<bmp->w*bmp->h; idx++){			
			
			// if the pixel is transparent
			if ( ((ushort*)bmp->data)[idx] == Gr_t_green.mask)	{
					((ushort*)bmp->data)[idx] = 0;
			}
		}

		bmp->flags |= BMP_TEX_XPARENT;
	} 
}

void bm_d3d_lock_pcx( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	ubyte *data, *palette;
	ubyte pal[768];
	palette = NULL;

	// Unload any existing data
	bm_d3d_free_data( bitmapnum );	

	// allocate bitmap data
	if(bpp == 16 && Cmdline_pcx32) {
		bpp = 32;
	}

	data = (ubyte *)bm_d3d_malloc(bitmapnum, bmp->w * bmp->h * (bpp / 8));
	bmp->data = (uint)data;
	memset( data, 0, bmp->w * bmp->h * (bpp / 8));

	bmp->bpp = bpp;

	if(bpp == 8){
		palette = pal;
		bmp->palette = gr_palette;
	} else {
		bmp->palette = NULL;
	}	

	Assert( &be->bm == bmp );

	// some sanity checks on flags
	Assert(!((flags & BMP_AABITMAP) && (flags & BMP_TEX_ANY)));						// no aabitmap textures
	Assert(!((flags & BMP_TEX_XPARENT) && (flags & BMP_TEX_NONDARK)));			// can't be a transparent texture and a nondarkening texture 

	if(bpp == 8){
		int pcx_error=pcx_read_bitmap_8bpp( be->filename, data, palette );
		if ( pcx_error != PCX_ERROR_NONE )	{
			//return -1;
		}
	} else {	
		int pcx_error;

		if(bpp == 16){
		
			// load types
			if(flags & BMP_AABITMAP){
				pcx_error = pcx_read_bitmap_16bpp_aabitmap( be->filename, data );
			} else if(flags & BMP_TEX_NONDARK){
				pcx_error = pcx_read_bitmap_16bpp_nondark( be->filename, data );
			} else {
				pcx_error = pcx_read_bitmap_16bpp( be->filename, data );
			}
		} else {
			pcx_error = pcx_read_bitmap_32( be->filename, data );
		}

		Assert(pcx_error == PCX_ERROR_NONE );
	}

	bmp->flags = 0;	
	if(bpp != 32){
		bm_d3d_convert_format( bitmapnum, bmp, bpp, flags );
	}
}

void bm_d3d_lock_ani( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	anim				*the_anim;
	anim_instance	*the_anim_instance;
	bitmap			*bm;
	ubyte				*frame_data;
	int				size, i;
	int				first_frame, nframes;	

	first_frame = be->info.ani.first_frame;
	nframes = bm_bitmaps[first_frame].info.ani.num_frames;

	if ( (the_anim = anim_load(bm_bitmaps[first_frame].filename)) == NULL ) {
		// Error(LOCATION, "Error opening %s in bm_lock\n", be->filename);
	}

	if ( (the_anim_instance = init_anim_instance(the_anim, bpp)) == NULL ) {
		// Error(LOCATION, "Error opening %s in bm_lock\n", be->filename);
		anim_free(the_anim);
	}

	int can_drop_frames = 0;

	if ( the_anim->total_frames != bm_bitmaps[first_frame].info.ani.num_frames )	{
		can_drop_frames = 1;
	}

	bm = &bm_bitmaps[first_frame].bm;
	if(bpp == 16){
		size = bm->w * bm->h * 2;
	} else {
		size = bm->w * bm->h;
	}
		
	for ( i=0; i<nframes; i++ )	{
		be = &bm_bitmaps[first_frame+i];
		bm = &bm_bitmaps[first_frame+i].bm;

		// Unload any existing data
		bm_d3d_free_data( first_frame+i );

		bm->flags = 0;
		bm->bpp  = bpp;
		bm->data = (uint)bm_d3d_malloc(first_frame + i, size);

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
					ushort *drow = &((ushort*)dptr)[bm->w * h];
					ushort *srow = &((ushort*)sptr)[f2i(v)*the_anim->width];

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

		bm_d3d_convert_format( first_frame+i, bm, bpp, flags );

		// Skip a frame
		if ( (i < nframes-1)  && can_drop_frames )	{
			frame_data = anim_get_next_raw_buffer(the_anim_instance, 0, flags & BMP_AABITMAP ? 1 : 0, bm->bpp);
		}

		//mprintf(( "Checksum = %d\n", be->palette_checksum ));
	}

	free_anim_instance(the_anim_instance);
	anim_free(the_anim);
}


void bm_d3d_lock_user( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	// Unload any existing data
	bm_d3d_free_data( bitmapnum );	

	switch( be->info.user.bpp )	{
	case 16:			// user 16 bit bitmap
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;		
		bmp->data = (uint)be->info.user.data;								
		break;	
												 
	case 8:			// Going from 8 bpp to something (probably only for aabitmaps)
	 //	Assert(flags & BMP_AABITMAP);
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;		
		bmp->data = (uint)be->info.user.data;								
		break;
	}

	bm_d3d_convert_format( bitmapnum, bmp, bpp, flags );
}

bool d3d_lock_and_set_internal_texture(int stage, int handle, ubyte bpp, ubyte flags, float *u_scale, float *v_scale )
{
	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

	int valid_type = 
		bm_bitmaps[bitmapnum].type == BM_TYPE_TGA || 
		bm_bitmaps[bitmapnum].type == BM_TYPE_DDS || 
		bm_bitmaps[bitmapnum].type == BM_TYPE_JPG;

	if(valid_type)	
	{
		// There is no internal texture
		bm_d3d_lock(handle, bpp, flags );
		bm_unlock(handle); 
		
	 	if(u_scale) *u_scale = 1.0;
	 	if(v_scale) *v_scale = 1.0;

		d3d_SetTexture(stage, d3d_bitmap_entry[bitmapnum].tinterface);
		return true;
	}

	return false;
}

// This locks down a bitmap and returns a pointer to a bitmap
// that can be accessed until you call bm_unlock.   Only lock
// a bitmap when you need it!  This will convert it into the 
// appropriate format also.
bitmap * bm_d3d_lock( int handle, ubyte bpp, ubyte flags )
{
	Assert(bm_d3d_inited);

	bitmap			*bmp;
	bitmap_entry	*be;

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

	// if we're on a standalone server, aways for it to lock to 8 bits
	if(Is_standalone){
		bpp = 8;
		flags = 0;
	} 
	// otherwise do it as normal
	else if(flags & BMP_AABITMAP){
 //		Assert( bpp == 8 );
	} 
	else {
		Assert( bpp == 16 );
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

	// if bitmap hasn't been loaded yet, then load it from disk
	// reread the bitmap from disk under certain conditions
	if(be->type > BM_TYPE_32_BIT_FORMATS)
	{
		if(d3d_bitmap_entry[bitmapnum].tinterface == NULL)
		{
			switch ( be->type ) {
		
			// We'll let D3DX handle this
			case BM_TYPE_JPG:
			case BM_TYPE_DDS:
			case BM_TYPE_TGA:
			{
				d3d_bitmap_entry[bitmapnum].tinterface = 
					(IDirect3DBaseTexture8 *) d3d_lock_d3dx_types(be->filename, be->type, flags );			
				break;
			}
			default:
				Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", be->type );
				return NULL;
			} 
		}
	}
	else if ( (bmp->data == 0) || (bpp != bmp->bpp)) {
		Assert(be->ref_count == 1);

		if ( be->type != BM_TYPE_USER ) {
			if ( bmp->data == 0 ) {
				nprintf (("BmpMan","Loading %s for the first time.\n", be->filename));
			} else if ( bpp != bmp->bpp ) {
				nprintf (("BmpMan","Reloading %s from bitdepth %d to bitdepth %d\n", be->filename, bmp->bpp, bpp));
			} 
		}

		// select proper format
		if(flags & BMP_AABITMAP){
			BM_SELECT_ALPHA_TEX_FORMAT();
		} else if(flags & BMP_TEX_ANY){
			BM_SELECT_TEX_FORMAT();					
		} else {
		   	BM_SELECT_SCREEN_FORMAT();
		}

		switch ( be->type ) {
		case BM_TYPE_PCX:
			bm_d3d_lock_pcx( handle, bitmapnum, be, bmp, bpp, flags );
			break;

		case BM_TYPE_ANI: 
			bm_d3d_lock_ani( handle, bitmapnum, be, bmp, bpp, flags );
			break;

		case BM_TYPE_USER:	
			bm_d3d_lock_user( handle, bitmapnum, be, bmp, bpp, flags );
			break;
		default:
			Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", be->type );
			return NULL;
		}		

		// always go back to screen format
		BM_SELECT_SCREEN_FORMAT();
	}

	if ( be->type == BM_TYPE_ANI ) {
		int i,first = bm_bitmaps[bitmapnum].info.ani.first_frame;

		for ( i=0; i< bm_bitmaps[first].info.ani.num_frames; i++ )	{
			// Mark all the bitmaps in this bitmap or animation as recently used
			bm_bitmaps[first+i].last_used = timer_get_milliseconds();

			// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
			bm_bitmaps[first+i].used_flags = flags;
		}
	} else {
		// Mark all the bitmaps in this bitmap or animation as recently used
		be->last_used = timer_get_milliseconds();

		// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
		be->used_flags = flags;
	}

	return bmp;
}

// Unlocks a bitmap
//
// Decrements the ref_count member of the bitmap_entry struct.  A bitmap can only be unloaded
// when the ref_count is 0.
//
void bm_d3d_unlock( int handle )
{
	Assert(bm_d3d_inited);

	bitmap_entry	*be;
	bitmap			*bmp;

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );	// INVALID BITMAP HANDLE

	Assert(bitmapnum >= 0 && bitmapnum < MAX_BITMAPS);

	be = &bm_bitmaps[bitmapnum];
	bmp = &be->bm;

	be->ref_count--;
	Assert(be->ref_count >= 0);		// Trying to unlock data more times than lock was called!!!

}

void bm_d3d_get_palette(int handle, ubyte *pal, char *name)
{
	char *filename;
	int w,h;

	int n= handle % MAX_BITMAPS;
	Assert( bm_bitmaps[n].handle == handle );		// INVALID BITMAP HANDLE

	filename = bm_bitmaps[n].filename;

	if (name)	{
		strcpy( name, filename );
	}

	int pcx_error=pcx_read_header( filename, &w, &h, pal );
	if ( pcx_error != PCX_ERROR_NONE ){
		// Error(LOCATION, "Couldn't open '%s'\n", filename );
	}
}

// --------------------------------------------------------------------------------------
// bm_release()  - unloads the bitmap's data and entire slot, so bitmap 'n' won't be valid anymore
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			nothing

void bm_d3d_release(int handle)
{
	bitmap_entry	*be;

	int n = handle % MAX_BITMAPS;

	Assert(n >= 0 && n < MAX_BITMAPS);
	be = &bm_bitmaps[n];

	if ( bm_bitmaps[n].type == BM_TYPE_NONE ) {
		return;	// Already been released?
	}

	if ( bm_bitmaps[n].type != BM_TYPE_USER )	{
		return;
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return;
	}

	bm_d3d_free_data(n);

	if ( bm_bitmaps[n].type == BM_TYPE_USER )	{
		bm_bitmaps[n].info.user.data = NULL;
		bm_bitmaps[n].info.user.bpp = 0;
	}

	bm_bitmaps[n].type = BM_TYPE_NONE;

	// Fill in bogus structures!

	// For debugging:
	strcpy( bm_bitmaps[n].filename, "IVE_BEEN_RELEASED!" );
	bm_bitmaps[n].signature = 0xDEADBEEF;									// a unique signature identifying the data
	bm_bitmaps[n].palette_checksum = 0xDEADBEEF;							// checksum used to be sure bitmap is in current palette

	// bookeeping
	bm_bitmaps[n].ref_count = -1;									// Number of locks on bitmap.  Can't unload unless ref_count is 0.

	// Bitmap info
	bm_bitmaps[n].bm.w = bm_bitmaps[n].bm.h = -1;
	
	// Stuff needed for animations
	// Stuff needed for user bitmaps
	memset( &bm_bitmaps[n].info, 0, sizeof(bm_extra_info) );

	bm_bitmaps[n].handle = -1;
}

// --------------------------------------------------------------------------------------
// bm_unload()  - unloads the data, but not the bitmap info.
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			0		=>		unload failed
//						1		=>		unload successful
//
int bm_d3d_unload( int handle )
{
	bitmap_entry	*be;
	bitmap			*bmp;

	int n = handle % MAX_BITMAPS;

	Assert(n >= 0 && n < MAX_BITMAPS);
	be = &bm_bitmaps[n];
	bmp = &be->bm;

	if ( be->type == BM_TYPE_NONE ) {
		return 0;		// Already been released
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE!

	if(stricmp("2_Credits.jpg", be->filename) == 0)
	{
		mprintf(("unloading 2_Credits.jpg\n"));
	}

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return 0;
	}

	nprintf(("BmpMan", "unloading %s.  %dx%dx%d\n", be->filename, bmp->w, bmp->h, bmp->bpp));
	bm_d3d_free_data(n);		// clears flags, bbp, data, etc

	return 1;
}


// unload all used bitmaps
void bm_d3d_unload_all()
{
	for (int i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			bm_unload(bm_bitmaps[i].handle);
		}
	}
}

// Marks a texture as being used for this level
void bm_d3d_page_in_texture( int bitmapnum, int nframes )
{
	int n = bitmapnum % MAX_BITMAPS;
	for (int i=0; i<nframes;i++ )	{

		bm_bitmaps[n+i].preloaded = 1;

		if ( D3D_enabled )	{
			bm_bitmaps[n+i].used_flags = BMP_TEX_OTHER;
		} else {			
			bm_bitmaps[n+i].used_flags = 0;
		}
	}
}

// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_d3d_page_in_nondarkening_texture( int bitmapnum, int nframes )
{
	int n = bitmapnum % MAX_BITMAPS;
	for (int i=0; i<nframes;i++ )	{

		bm_bitmaps[n+i].preloaded = 4;

		if ( D3D_enabled )	{			
			bm_bitmaps[n+i].used_flags = BMP_TEX_NONDARK;
		} else {
			bm_bitmaps[n+i].used_flags = 0;
		}
	}
}

// marks a texture as being a transparent textyre used for this level
// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_d3d_page_in_xparent_texture( int bitmapnum, int nframes)
{
	int n = bitmapnum % MAX_BITMAPS;
	for (int i=0; i<nframes;i++ )	{

		bm_bitmaps[n+i].preloaded = 3;

		if ( D3D_enabled )	{
			bm_bitmaps[n+i].used_flags = BMP_TEX_XPARENT;
		} else {
			bm_bitmaps[n+i].used_flags = 0;
		}
	}
}

// Marks an aabitmap as being used for this level
void bm_d3d_page_in_aabitmap( int bitmapnum, int nframes )
{
	int n = bitmapnum % MAX_BITMAPS;
	for (int i=0; i<nframes;i++ )	{

		bm_bitmaps[n+i].preloaded = 2;
	
		if ( D3D_enabled )	{
			bm_bitmaps[n+i].used_flags = BMP_AABITMAP;
		} else {
			bm_bitmaps[n+i].used_flags = 0;
		}
	}
}

// Tell the bitmap manager to start keeping track of what bitmaps are used where.
void bm_d3d_page_in_start()
{
	// Mark all as inited
	for (int i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			bm_unload(bm_bitmaps[i].handle);
		}
		bm_bitmaps[i].preloaded = 0;
		bm_bitmaps[i].used_flags = 0;
	}
}

void bm_d3d_page_in_stop()
{	
	int ship_info_index;
	extern void gr_d3d_preload_init();
	extern int gr_d3d_preload(int bitmap_num, int is_aabitmap );
	int i;	

	nprintf(( "BmpInfo","BMPMAN: Loading all used bitmaps.\n" ));

	// Load all the ones that are supposed to be loaded for this level.
	int n = 0;

	int d3d_preloading = 1;
	gr_d3d_preload_init();

	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			if ( bm_bitmaps[i].preloaded )	{

#ifdef BMPMAN_SPECIAL_NONDARK
				// if this is a texture, check to see if a ship uses it
				ship_info_index = ship_get_texture(bm_bitmaps[i].handle);
				// use the colors from this ship
				if((ship_info_index >= 0) && (Ship_info[ship_info_index].num_nondark_colors > 0)){
					// mprintf(("Using custom pixels for %s\n", Ship_info[ship_info_index].name));
					palman_set_nondarkening(Ship_info[ship_info_index].nondark_colors, Ship_info[ship_info_index].num_nondark_colors);
				}
				// use the colors from the default table
				else {
					// mprintf(("Using default pixels\n"));
					palman_set_nondarkening(Palman_non_darkening_default, Palman_num_nondarkening_default);
				}
#endif

				// if preloaded == 3, load it as an xparent texture				
				if(bm_bitmaps[i].used_flags == BMP_AABITMAP){
					bm_lock( bm_bitmaps[i].handle, 8, bm_bitmaps[i].used_flags );
				} else {
					bm_lock( bm_bitmaps[i].handle, 16, bm_bitmaps[i].used_flags );
				}
				bm_unlock( bm_bitmaps[i].handle );

				if ( d3d_preloading )	{
					if ( !gr_d3d_preload(bm_bitmaps[i].handle, (bm_bitmaps[i].preloaded==2) ) )	{
						mprintf(( "Out of VRAM.  Done preloading.\n" ));
						d3d_preloading = 0;
					}
				}

				n++;
			} 
		}
		game_busy();
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
}

int bm_d3d_get_cache_slot( int bitmap_id, int separate_ani_frames )
{
	int n = bitmap_id % MAX_BITMAPS;

	Assert( bm_bitmaps[n].handle == bitmap_id );		// INVALID BITMAP HANDLE

	bitmap_entry	*be = &bm_bitmaps[n];

	if ( (!separate_ani_frames) && (be->type == BM_TYPE_ANI) )	{
		return be->info.ani.first_frame;
	} 

	return n;

}

// get the rgba components of a pixel, any of the parameters can be NULL
void bm_d3d_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a)
{
	extern int D3D_32bit;
	int bit_32 = 0;

	// pick a byte size - 32 bits only if 32 bit mode d3d and screen format
	if(D3D_32bit && (Gr_current_red == &Gr_red)){
		bit_32 = 1;
	}

	if(r != NULL){
		if(bit_32){
			*r = ubyte(( (*((uint*)pixel) & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		} else {
			*r = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		}
	}
	if(g != NULL){
		if(bit_32){
			*g = ubyte(( (*((uint*)pixel) & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		} else {
			*g = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		}
	}
	if(b != NULL){
		if(bit_32){
			*b = ubyte(( (*((uint*)pixel) & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		} else {
			*b = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		}
	}

	// get the alpha value - this is never asked for
	if(a != NULL){		
		*a = 1;

			Assert(!bit_32);
			if(!( ((ushort*)pixel)[0] & 0x8000)){
				*a = 0;
			} 
	}
}

// given a bitmap and a section, return the size (w, h)
void bm_d3d_get_section_size(int bitmapnum, int sx, int sy, int *w, int *h)
{
	int bw, bh;
	bitmap_section_info *sections;

	// bogus input?
	Assert((w != NULL) && (h != NULL));
	if((w == NULL) || (h == NULL)){
		return;
	}

	// get bitmap info
	bm_get_info(bitmapnum, &bw, &bh, NULL, NULL, NULL, &sections);

	// determine the width and height of this section
	*w = sx < (sections->num_x - 1) ? max_bitmap_size : bw - sections->sx[sx];
	*h = sy < (sections->num_y - 1) ? max_bitmap_size : bh - sections->sy[sy];										
}

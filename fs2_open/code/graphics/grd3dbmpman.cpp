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
#include <D3dx8tex.h>

#include <ctype.h>
#include "globalincs/pstypes.h"
#include "pcxutils/pcxutils.h"
#include "bmpman/bmpman.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/grd3dinternal.h"
#include "debugconsole/dbugfile.h"
#include "cmdline/cmdline.h"
#include "cfile/cfile.h"
#include "ddsutils/ddsutils.h"
#include "tgautils/tgautils.h"
#include "graphics/grd3dbmpman.h"
#include "globalincs/systemvars.h"
#include "jpgutils/jpgutils.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

extern int Cmdline_jpgtga;

D3DBitmapData d3d_bitmap_entry[MAX_BITMAPS];
bool Supports_compression[NUM_COMPRESSION_TYPES];

int d3d_get_valid_texture_size(int value, bool width);


// anything API specific to freeing bm data
void gr_d3d_bm_free_data(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	int rc = 0;

	if (d3d_bitmap_entry[n].tinterface != NULL) {
		do { 
			rc = d3d_bitmap_entry[n].tinterface->Release(); 
			if(rc > 1)Int3();
		} while ( rc > 0 );
	//	} while ( rc > 1 );
	//	d3d_bitmap_entry[n].tinterface->Release();

		d3d_bitmap_entry[n].tinterface = NULL;
	}
}

// API specifics for creating a user bitmap
void gr_d3d_bm_create(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	Assert(d3d_bitmap_entry[n].tinterface == NULL);

	d3d_bitmap_entry[n].tinterface = NULL;
}

// create a texture from data stored in memory
IDirect3DTexture8 *d3d_make_texture(void *data, int bitmapnum, int size, int type, int flags) 
{
	D3DXIMAGE_INFO source_desc;
	if(FAILED(D3DXGetImageInfoFromFileInMemory(data, size,	&source_desc))) {
		return NULL;
	} 

	D3DFORMAT use_format  = D3DFMT_UNKNOWN;

	// User is requesting we use compressed textures then pretend that the source is compressed
	// DX will workout that its not but it means it can use the same code as before
	if(Cmdline_dxt)
	{
		source_desc.Format = default_compressed_format;
	}	

	// Determine the destination (texture) format to hold the image
	switch(source_desc.Format)
	{
		case D3DFMT_DXT1: if(Supports_compression[0]) {use_format = D3DFMT_DXT1; break;}
		case D3DFMT_DXT2: if(Supports_compression[1]) {use_format = D3DFMT_DXT2; break;}
		case D3DFMT_DXT3: if(Supports_compression[2]) {use_format = D3DFMT_DXT3; break;}
		case D3DFMT_DXT4: if(Supports_compression[3]) {use_format = D3DFMT_DXT4; break;}
		case D3DFMT_DXT5: if(Supports_compression[4]) {use_format = D3DFMT_DXT5; break;}
		default:
		{
			bool use_alpha_format = false;	// initialization added by Goober5000

			// Determine if the destination format needs to store alpha details
			switch(type)
			{
			case BM_TYPE_TGA: use_alpha_format = (source_desc.Format == D3DFMT_A8R8G8B8); break; // 32 Bit TGAs only
			case BM_TYPE_JPG: use_alpha_format = false; break; // JPG: Never 
			case BM_TYPE_DDS: use_alpha_format = true;  break; // DDS: Always
			default: Assert(0);	// just in case -- Goober5000
			}

			if(gr_screen.bits_per_pixel == 32) {
				use_format = use_alpha_format ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
			} else {
				use_format = use_alpha_format ? default_alpha_tformat : default_non_alpha_tformat;
			}
		}
	}

	extern D3DBitmapData d3d_bitmap_entry[MAX_BITMAPS];

	float *uscale = &(d3d_bitmap_entry[bitmapnum].uscale);
	float *vscale = &(d3d_bitmap_entry[bitmapnum].vscale);
	  
	bool use_mipmapping = (Cmdline_d3dmipmap > 0);

	DWORD filter = D3DX_FILTER_LINEAR; // Linear, enough to smooth rescales but not too much blur

	*uscale = *vscale = 1.0;

	if(flags == TCACHE_TYPE_BITMAP_SECTION) {
		use_mipmapping = 0;
	  	filter = D3DX_FILTER_NONE; 
		*uscale = ((float) source_desc.Width)  / ((float) d3d_get_valid_texture_size(source_desc.Width, true));
		*vscale = ((float) source_desc.Height) / ((float) d3d_get_valid_texture_size(source_desc.Height, false));
	} else if(gr_screen.bits_per_pixel == 16 && ((type == BM_TYPE_TGA) || (type == BM_TYPE_JPG))) {
	  	filter |=D3DX_FILTER_DITHER;
	}

	IDirect3DTexture8 *ptexture = NULL;
	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
		GlobalD3DVars::lpD3DDevice,
		data, size,
	  	// Opertunity to control sizes here.
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		use_mipmapping ? 0 : 1, 
		0, 
		use_format,
		D3DPOOL_MANAGED, 
		filter,
	 	D3DX_DEFAULT,
		0, 
		&source_desc, 
		NULL, &ptexture);

	return SUCCEEDED(hr) ? ptexture : NULL;
}

// read an image file header using D3DX
bool d3d_read_header_d3dx(char *file, CFILE *img_cfp, int type, int *w, int *h, int *bpp, int *bm_size)
{
	char filename[MAX_FILENAME_LEN];
	CFILE *d3dx_file;
	int size = 0;
	void *img_data = NULL;

	if (img_cfp == NULL) {
		strcpy( filename, file);
		char *p = strchr( filename, '.' );
		if ( p ) *p = 0;

		// only use formats that aren't otherwise supported since that's faster than
		// reading the entire file just to get the header info.
		switch (type) {
			case BM_TYPE_JPG:
				strcat( filename, ".jpg" );
				break;

			default:
				return false;
		}

		d3dx_file = cfopen( filename , "rb" );

		if (d3dx_file == NULL){
			return false;
		}
	} else {
		d3dx_file = img_cfp;
	}

	size = cfilelength(d3dx_file);

	img_data = malloc(size); //I freed this - Bobboau

	if (img_data == NULL)
		return false;

	cfread(img_data, size, 1, d3dx_file);	

	if (img_cfp == NULL) {
		cfclose(d3dx_file);
		d3dx_file = NULL;
	}

	D3DXIMAGE_INFO source_desc;
	if(FAILED(D3DXGetImageInfoFromFileInMemory(img_data, size,	&source_desc))) {
		free(img_data);//right here -Bobboau
		return false;
	} 

	if(w) *w = source_desc.Width;
	if(h) *h = source_desc.Height;
	if(bpp) *bpp = (source_desc.Depth / 8);
	if(bm_size) *bm_size = (source_desc.Width * source_desc.Height * (source_desc.Depth / 8));
	
	free(img_data);//and right here -Bobboau
	return true;
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
int gr_d3d_bm_load(ubyte type, int n, char *filename, CFILE *img_cfp, int *w, int *h, int *bpp, ubyte *c_type, int *mm_lvl, int *bm_size)
{
	int dds_ct;

	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	Assert(d3d_bitmap_entry[n].tinterface == NULL);

	if ( d3d_bitmap_entry[n].tinterface != NULL )
		return -1;

	// DDS file
	if (type == BM_TYPE_DDS) {
		int dds_error = dds_read_header( filename, img_cfp, w, h, bpp, &dds_ct, mm_lvl, bm_size );
		if (dds_error != DDS_ERROR_NONE) {
			mprintf(("dds: Couldn't open '%s' -- error description %s\n", filename, dds_error_string(dds_error)));
			return -1;
		}
	}
	// if its a tga file
	else if (type == BM_TYPE_TGA) {
		int tga_error = targa_read_header( filename, img_cfp, w, h, bpp, NULL );
		if ( tga_error != TARGA_ERROR_NONE )	{
			mprintf(( "tga: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}
	// maybe it's a PCX
	else if (type == BM_TYPE_PCX) {
		int pcx_error = pcx_read_header( filename, img_cfp, w, h, bpp, NULL );		
		if ( pcx_error != PCX_ERROR_NONE )	{
			DBUGFILE_OUTPUT_1("bm_pcx: Cant load %s",filename);
			mprintf(( "bm_pcx: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}
	// maybe it's a JPG
	else if (type == BM_TYPE_JPG) {
		int jpg_error = jpeg_read_header( filename, img_cfp, w, h, bpp, NULL );
		if ( jpg_error != JPEG_ERROR_NONE ) {
			mprintf(( "jpg: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}
	// send anything else through D3DX
	else {
		if(d3d_read_header_d3dx( filename, img_cfp, type, w, h, bpp, bm_size) == false) {
			DBUGFILE_OUTPUT_1("not bm_pcx: Cant load %s",filename);
			mprintf(( "not bm_pcx: Couldn't open '%s'\n", filename ));
			return -1;
		}
	}

	return 0;
}

// API specific init instructions
void gr_d3d_bm_init(int n)
{
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	d3d_bitmap_entry[n].flags = 0;
	d3d_bitmap_entry[n].tinterface = NULL;
}

extern void gr_d3d_preload_init();
// specifics for setting up the start of a page-in session
void gr_d3d_bm_page_in_start()
{
	gr_d3d_preload_init();

	if (GlobalD3DVars::lpD3DDevice != NULL)
		GlobalD3DVars::lpD3DDevice->ResourceManagerDiscardBytes(0);
}

// create D3D texture from an image file
void *d3d_lock_d3dx_types(char *file, int type, ubyte flags, int bitmapnum)
{
	char filename[MAX_FILENAME_LEN];

	strcpy( filename, file);
	char *p = strchr( filename, '.' );
	if ( p ) *p = 0;

	switch (type) {
		case BM_TYPE_DDS:
			strcat( filename, ".dds" );
			break;
		
		case BM_TYPE_TGA:
			strcat( filename, ".tga" );
			break;
		
		case BM_TYPE_JPG:
			strcat( filename, ".jpg" );
			break;

		default:
			return NULL;
	}

	CFILE *targa_file = cfopen( filename , "rb" );

	if (targa_file == NULL) {
		DBUGFILE_OUTPUT_1("Failed to open through cfopen '%s'", filename);
		return NULL;
	}

	int size = cfilelength(targa_file);
	void *tga_data = malloc(size);

	if  (tga_data == NULL)
		return NULL;

	cfread(tga_data, size, 1, targa_file);	

	cfclose(targa_file);
	targa_file = NULL;

	IDirect3DTexture8 *ptexture = d3d_make_texture(tga_data, bitmapnum, size, type, flags);

	free(tga_data);

	return ptexture;
}

// Lock an image files data into memory
int gr_d3d_bm_lock(char *filename, int handle, int bitmapnum, ubyte bpp, ubyte flags)
{
	ubyte c_type = BM_TYPE_NONE;
	char realname[MAX_FILENAME_LEN];
	ubyte true_bpp;

	bitmap_entry *be = &bm_bitmaps[bitmapnum];
	bitmap *bmp = &be->bm;

	if (Is_standalone) {
		true_bpp = 8;
	}
	// not really sure how well this is going to work out in every case but...
	else if ( Cmdline_jpgtga && (bmp->true_bpp > bpp) ) {
		true_bpp = bmp->true_bpp;
	} else {
		true_bpp = bpp;
	}

	// make sure we use the real graphic type for EFFs
	if ( be->type == BM_TYPE_EFF ) {
		c_type = be->info.eff.type;
		strcpy(realname, be->info.eff.filename);
	} else {
		c_type = be->type;
		strcpy(realname, be->filename);
	}

	if (c_type > BM_TYPE_32_BIT_FORMATS) {
		if(d3d_bitmap_entry[bitmapnum].tinterface == NULL) {
			Assert(be->ref_count == 1);
	
			switch ( c_type ) {
				// We'll let D3DX handle this
				case BM_TYPE_JPG:
				case BM_TYPE_DDS:
				case BM_TYPE_TGA:
					d3d_bitmap_entry[bitmapnum].tinterface = 
						(IDirect3DBaseTexture8 *) d3d_lock_d3dx_types(realname, c_type, flags, bitmapnum);			
					break;

				default:
					Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", c_type );
					return -1;
			}

			bm_update_memory_used( bitmapnum, be->mem_taken );
		}
	} else if ( (bmp->data == 0) && (d3d_bitmap_entry[bitmapnum].tinterface == NULL) || (true_bpp != bmp->bpp && bmp->bpp != 32)) {
		Assert(be->ref_count == 1);

		if ( c_type != BM_TYPE_USER ) {
			if ( bmp->data == 0 ) {
				mprintf (("Loading %s for the first time.\n", filename));
			} else if ( bpp != bmp->bpp ) {
				mprintf (("Reloading %s from bitdepth %d to bitdepth %d\n", filename, bmp->bpp, true_bpp));
			} 
		}

		if ( !Bm_paging )	{
			if ( c_type != BM_TYPE_USER ) {
				char flag_text[64];
				strcpy( flag_text, "--" );							
				nprintf(( "Paging", "Loading %s (%dx%dx%dx%s)\n", filename, bmp->w, bmp->h, true_bpp, flag_text ));
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

		switch ( c_type ) {
			case BM_TYPE_PCX:
				mprintf(("MEMLEAK DEBUG: lock pcx\n"));
		  		bm_lock_pcx( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			case BM_TYPE_ANI: 
				bm_lock_ani( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;

			case BM_TYPE_USER:	
				bm_lock_user( handle, bitmapnum, be, bmp, true_bpp, flags );
				break;
			default:
				Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", c_type );
				return -1;
		}		

		// always go back to screen format
		BM_SELECT_SCREEN_FORMAT();
	}

	// make sure we actually did something
	if ( !(bmp->data) && (d3d_bitmap_entry[bitmapnum].tinterface == NULL) ) {
		// oops, this isn't good - nothing ever got loaded
		return -1;
	}

	return 0;
}

// Lock a image file's data into memory and set it as a texture
bool d3d_lock_and_set_internal_texture(int stage, int handle, ubyte bpp, ubyte flags, float *u_scale, float *v_scale )
{
	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

	ubyte c_type = BM_TYPE_NONE;

	if(bm_bitmaps[bitmapnum].type == BMP_TEX_DYNAMIC_RENDER_TARGET || bm_bitmaps[bitmapnum].type == BMP_TEX_STATIC_RENDER_TARGET)return true;

	if (bm_bitmaps[bitmapnum].type == BM_TYPE_EFF) {
		c_type = bm_bitmaps[bitmapnum].info.eff.type;
	} else {
		c_type = bm_bitmaps[bitmapnum].type;
	}

	int valid_type = 
		c_type == BM_TYPE_TGA || 
		c_type == BM_TYPE_DDS || 
		c_type == BM_TYPE_JPG;

	if(valid_type)	
	{
		// There is no internal texture
		bm_lock(handle, bpp, flags );
		bm_unlock(handle); 
		
	 	if(u_scale) *u_scale = d3d_bitmap_entry[bitmapnum].uscale;
	 	if(v_scale) *v_scale = d3d_bitmap_entry[bitmapnum].vscale;

		d3d_SetTexture(stage, d3d_bitmap_entry[bitmapnum].tinterface);
		return true;
	}

	if(Cmdline_d3d_lesstmem && c_type == BM_TYPE_PCX)
	{
		if(d3d_bitmap_entry[bitmapnum].tinterface == NULL)
		{
			// Trick the normal functions into getting the info we need
			tcache_slot_d3d t;
			t.bitmap_id = -1;

			d3d_create_texture(handle, flags, &t, 0); 
			d3d_bitmap_entry[bitmapnum].uscale = t.u_scale;
	 		d3d_bitmap_entry[bitmapnum].vscale = t.v_scale;
		  	d3d_bitmap_entry[bitmapnum].tinterface = t.d3d8_thandle;

			free((void *) bm_bitmaps[bitmapnum].bm.data);
			bm_bitmaps[bitmapnum].bm.data = NULL;
		}

		if(u_scale) *u_scale = d3d_bitmap_entry[bitmapnum].uscale;
	 	if(v_scale) *v_scale = d3d_bitmap_entry[bitmapnum].vscale;

		d3d_SetTexture(stage, d3d_bitmap_entry[bitmapnum].tinterface);
		return true;
	}

	return false;
}

	static IDirect3DSurface8 *surface = NULL;
	IDirect3DSurface8 *depth = NULL;
	static IDirect3DSurface8 *back_depth = NULL;	//the back_buffer's zbuffer
	static IDirect3DSurface8 *texture_depth = NULL;	//a zbuffer assosiated with rendeing to a texture

void bm_pre_lost(){
	int I;
	if(back_depth)I = back_depth->Release();
	back_depth = NULL;
	if(surface)I = surface->Release();
	surface = NULL;
	if(texture_depth)texture_depth->Release();
		texture_depth=NULL;
	for( int i = 0; i<MAX_BITMAPS; i++){
		if(d3d_bitmap_entry[i].flags & DXT_DEFAULT_MEM_POOL && d3d_bitmap_entry[i].tinterface){
			I = d3d_bitmap_entry[i].tinterface->Release();
			d3d_bitmap_entry[i].tinterface = NULL;
		}
	}
}

void bm_post_lost(){
	int I = 0;
	for(int i = 0; i<MAX_BITMAPS; i++){
		if(d3d_bitmap_entry[i].flags&DXT_DEFAULT_MEM_POOL){
			Assert(!d3d_bitmap_entry[i].tinterface);

			bool cube = (d3d_bitmap_entry[i].flags & DXT_CUBEMAP);

			if(cube)GlobalD3DVars::lpD3DDevice->CreateCubeTexture(max(d3d_bitmap_entry[i].x,d3d_bitmap_entry[i].y),1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DCubeTexture8**)&d3d_bitmap_entry[i].tinterface);
			else GlobalD3DVars::lpD3DDevice->CreateTexture(d3d_bitmap_entry[i].x,d3d_bitmap_entry[i].y,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DTexture8**)&d3d_bitmap_entry[i].tinterface);

			if(d3d_bitmap_entry[i].backup_tinterface){
				//if it has a backup copy that backup back into it
				//if it doesn't have a backup then it is a dynamic render target and doesn't need it (and can't get it)
				IDirect3DSurface8* texture;
				IDirect3DSurface8* backup;
				int nf = (cube)?6:1;
				for(int f = 0; f<nf; f++){
					if(cube){
						(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[i].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(f),0,&texture);
						(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[i].backup_tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(f),0,&backup);
					}else{
						(*((IDirect3DTexture8**)(&d3d_bitmap_entry[i].tinterface)))->GetSurfaceLevel(0,&texture);
						(*((IDirect3DTexture8**)(&d3d_bitmap_entry[i].backup_tinterface)))->GetSurfaceLevel(0,&backup);
					}
	
					D3DXLoadSurfaceFromSurface(texture,NULL,NULL,backup,NULL,NULL, D3DX_FILTER_POINT,0);
					I= backup->Release();
					I= texture->Release();
				}
			}
		}
	}
}


bool gr_d3d_make_render_target(int n, int &x, int &y, int flags){
	Assert( (n >= 0) && (n < MAX_BITMAPS) );

	Assert(d3d_bitmap_entry[n].tinterface == NULL);

	//mark this surface as haveing a resource we are going to have to clean up dureing a lost device
		d3d_bitmap_entry[n].flags |= DXT_DEFAULT_MEM_POOL;

		//make the drimary drawing surface
		if(flags & BMP_TEX_CUBEMAP){
			d3d_bitmap_entry[n].flags |= DXT_CUBEMAP;
			GlobalD3DVars::lpD3DDevice->CreateCubeTexture(max(x,y),1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DCubeTexture8**)&d3d_bitmap_entry[n].tinterface);
		}else{
			GlobalD3DVars::lpD3DDevice->CreateTexture(x,y,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, (IDirect3DTexture8**)&d3d_bitmap_entry[n].tinterface);
		}

		if(flags & BMP_TEX_STATIC_RENDER_TARGET){
			//if we are going to want to keep this
			//if this is a static render target 
			//then we are going to want to keep a copy of it in system memory
			d3d_bitmap_entry[n].flags	|= DXT_STATIC;
			if(flags & BMP_TEX_CUBEMAP){
				GlobalD3DVars::lpD3DDevice->CreateCubeTexture	(max(x,y),	1,D3DUSAGE_DYNAMIC,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM, (IDirect3DCubeTexture8**)	&d3d_bitmap_entry[n].backup_tinterface);
			}else{
				GlobalD3DVars::lpD3DDevice->CreateTexture		(x,y,		1,D3DUSAGE_DYNAMIC,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM, (IDirect3DTexture8**)		&d3d_bitmap_entry[n].backup_tinterface);
			}
		}else{
			//if this is a dynamic render target then it will not live long enough for us to care
			d3d_bitmap_entry[n].flags	|= DXT_DYNAMIC;
			d3d_bitmap_entry[n].backup_tinterface = NULL;
		}

		//now see what we actualy got
	D3DSURFACE_DESC desc;
	if(flags & BMP_TEX_CUBEMAP){
		((IDirect3DCubeTexture8*)(d3d_bitmap_entry[n].tinterface))->GetLevelDesc(0, &desc);
	}else{
		((IDirect3DTexture8*)(d3d_bitmap_entry[n].tinterface))->GetLevelDesc(0, &desc);
	}
	x = desc.Width;
	y = desc.Height;

		//we need to know how big it was so we can rebuild it if the deviece gets lost
	d3d_bitmap_entry[n].x = x;
	d3d_bitmap_entry[n].y = y;
	return true;
}


bool gr_d3d_set_render_target(int handle, int face){
	int i = 0;
	static bool once = false;
	if(!once){atexit(bm_pre_lost);once = true;}
	//some cleanup code, these have to be frees before the program terminates

	int n = handle % MAX_BITMAPS;
	if(handle != -1){
		//data validation
		if(d3d_bitmap_entry[n].flags & DXT_CUBEMAP)Assert(face!=-1);
		//if this is a cube map then the face parameter gets used and is important
	
		Assert( bm_bitmaps[n].handle == handle );		// INVALID BITMAP HANDLE

		Assert( (n > -1) && (n < MAX_BITMAPS) );

		Assert(d3d_bitmap_entry[n].tinterface != NULL);	//make sure this texture has a surface

		if(bm_bitmaps[n].type != BM_TYPE_RENDER_TARGET){
			//odds are somone passed a normal texture created with bm_load
			Error( LOCATION, "trying to set invalid bitmap as render target" );
			return false;
		}

	}else n = -1;

	if(!(back_depth)){
		//get the backbuffer's surface so we can return it later
		GlobalD3DVars::lpD3DDevice->GetDepthStencilSurface(&back_depth);
	}

	if(!texture_depth && n!= -1){
		//make the texture zbuffer if it is missing for some reason
		GlobalD3DVars::lpD3DDevice->CreateDepthStencilSurface(d3d_bitmap_entry[n].x,d3d_bitmap_entry[n].y, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, &texture_depth);
	}

	if(gr_screen.rendering_to_texture != -1 && surface && d3d_bitmap_entry[gr_screen.rendering_to_texture].backup_tinterface){
		//if the current surface is alright and if this is a static texture 
		//(dynamic render targets' backup texture is always NULL) 
		//always copy the current surface to it's backup before we change it
		IDirect3DSurface8* backup;
		if(d3d_bitmap_entry[gr_screen.rendering_to_texture].flags&DXT_CUBEMAP){
			if(gr_screen.rendering_to_face != -1){
				(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[gr_screen.rendering_to_texture].backup_tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(gr_screen.rendering_to_face),0,&backup);
				D3DXLoadSurfaceFromSurface(backup,NULL,NULL,surface,NULL,NULL, D3DX_FILTER_POINT,0);
				i = backup->Release();
			}
		}else{
			(*((IDirect3DTexture8**)(&d3d_bitmap_entry[gr_screen.rendering_to_texture].backup_tinterface)))->GetSurfaceLevel(0,&backup);
			D3DXLoadSurfaceFromSurface(backup,NULL,NULL,surface,NULL,NULL, D3DX_FILTER_POINT,0);
			i = backup->Release();
		}
	}

	//ok get rid of the last rendering surface we were useing, it isn't needed anymore
	if(surface)i = surface->Release();

	if(n == -1){
		//we are not rendering to a texture
		GlobalD3DVars::lpD3DDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO, &surface);
		depth = back_depth;
//		gr_screen.max_w = gr_screen.save_max_w;
//		gr_screen.max_h = gr_screen.save_max_h;
	}else{
		i = 0;
		//make sure we clear out what ever was in it before we get a new one

		if(d3d_bitmap_entry[n].flags&DXT_CUBEMAP){
			Assert(face > -1 &&face <6);
			(*((IDirect3DCubeTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetCubeMapSurface(_D3DCUBEMAP_FACES(face),0,&surface);
		}else{
			(*((IDirect3DTexture8**)(&d3d_bitmap_entry[n].tinterface)))->GetSurfaceLevel(0,&surface);
		}
		depth = texture_depth;

//		gr_screen.max_w = d3d_bitmap_entry[n].x;
//		gr_screen.max_h = d3d_bitmap_entry[n].y;
	}

	if(D3D_OK != GlobalD3DVars::lpD3DDevice->SetRenderTarget(surface, depth))return false;

	return true;

}

IDirect3DBaseTexture8* get_render_target_texture(int handle){
	int n = handle % MAX_BITMAPS;
	return d3d_bitmap_entry[n].tinterface;
}

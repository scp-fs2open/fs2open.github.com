/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "grd3dsetup.h"
#include <d3d8.h>

#include "osapi/osapi.h"
#include "io/mouse.h"
#include "globalincs/systemvars.h"

#include "graphics/2d.h"
#include "graphics/grinternal.h"

#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"
#include "graphics/grd3dbmpman.h"
#include "graphics/grd3dlight.h"

#include "debugconsole/timerbar.h"
#include "cmdline/cmdline.h"   

// Lets define our global D3D variables
IDirect3D8           *GlobalD3DVars::lpD3D = NULL;
IDirect3DDevice8     *GlobalD3DVars::lpD3DDevice = NULL;

D3DCAPS8			  GlobalD3DVars::d3d_caps;
D3DPRESENT_PARAMETERS GlobalD3DVars::d3dpp; 

bool GlobalD3DVars::D3D_inited		 = 0;
bool GlobalD3DVars::D3D_activate	 = 0;
bool GlobalD3DVars::D3D_Antialiasing = 0;
bool GlobalD3DVars::D3D_window		 = 0;

int GlobalD3DVars::D3D_rendition_uvs = 0;
int GlobalD3DVars::D3D_custom_size   = -1;
int GlobalD3DVars::D3D_zbias         = 1;

ID3DXMatrixStack *world_matrix_stack, *view_matrix_stack, *proj_matrix_stack;

const int multisample_max = 16;
const D3DMULTISAMPLE_TYPE multisample_types[multisample_max] =
{
	D3DMULTISAMPLE_NONE,
	D3DMULTISAMPLE_2_SAMPLES,
	D3DMULTISAMPLE_3_SAMPLES,
	D3DMULTISAMPLE_4_SAMPLES,
	D3DMULTISAMPLE_5_SAMPLES,
	D3DMULTISAMPLE_6_SAMPLES,
	D3DMULTISAMPLE_7_SAMPLES,
	D3DMULTISAMPLE_8_SAMPLES,
	D3DMULTISAMPLE_9_SAMPLES,
	D3DMULTISAMPLE_10_SAMPLES,
	D3DMULTISAMPLE_11_SAMPLES,
	D3DMULTISAMPLE_12_SAMPLES,
	D3DMULTISAMPLE_13_SAMPLES,
	D3DMULTISAMPLE_14_SAMPLES,
	D3DMULTISAMPLE_15_SAMPLES,
	D3DMULTISAMPLE_16_SAMPLES
};

char Device_init_error[512] = "No errror set";
RECT D3D_cursor_clip_rect;

D3DMATERIAL8 material;

// DirectDraw structures used without DD
// To contain shift and scale variables to convert graphics files to textures 
PIXELFORMAT			AlphaTextureFormat;
PIXELFORMAT			NonAlphaTextureFormat;

D3DFORMAT default_non_alpha_tformat = D3DFMT_UNKNOWN;
D3DFORMAT default_alpha_tformat		= D3DFMT_UNKNOWN;

void d3d_clip_cursor()
{
 	ClipCursor(&D3D_cursor_clip_rect);
}

// Outputs a format to debug console
void d3d_dump_format(PIXELFORMAT *pf)
{
	unsigned long m;
	int r, g, b, a;
	for (r = 0, m = pf->dwRBitMask; !(m & 1); r++, m >>= 1);
	for (r = 0; m & 1; r++, m >>= 1);
	for (g = 0, m = pf->dwGBitMask; !(m & 1); g++, m >>= 1);
	for (g = 0; m & 1; g++, m >>= 1); 
	for (b = 0, m = pf->dwBBitMask; !(m & 1); b++, m >>= 1);
	for (b = 0; m & 1; b++, m >>= 1);
	if ( pf->dw_is_alpha_mode == true ) {
		for (a = 0, m = pf->dwRGBAlphaBitMask; !(m & 1); a++, m >>= 1);
		for (a = 0; m & 1; a++, m >>= 1);
		mprintf(( "ARGB, %d:%d:%d:%d\n", a, r, g, b ));
	} else {
		a = 0;
		mprintf(( "RGB, %d:%d:%d\n", r, g, b ));
	}
}

void gr_d3d_activate(int active)
{
	mprintf(( "Direct3D activate: %d\n", active ));

	HWND hwnd = (HWND)os_get_window();
	
	if ( active  )	{
		GlobalD3DVars::D3D_activate = 1;

		if ( hwnd )	{
			d3d_clip_cursor();
			ShowWindow(hwnd,SW_RESTORE);
		}

	} else {

		GlobalD3DVars::D3D_activate = 0;

		if ( hwnd )	{
			ClipCursor(NULL);
			ShowWindow(hwnd,SW_MINIMIZE);
		}
	}
//	ID3DXMatrixStack *world_matrix_stack, *view_matrix_stack, *proj_matrix_stack;
	D3DXCreateMatrixStack(0, &world_matrix_stack);
	D3DXCreateMatrixStack(0, &view_matrix_stack);
	D3DXCreateMatrixStack(0, &proj_matrix_stack);

}

void d3d_setup_format_components(
	PIXELFORMAT *surface, color_gun *r_gun, color_gun *g_gun, color_gun *b_gun, color_gun *a_gun)
{
	int s;
	unsigned long m;		

	for (s = 0, m = surface->dwRBitMask; !(m & 1); s++, m >>= 1);
	r_gun->mask = surface->dwRBitMask;
	r_gun->shift = s;
	r_gun->scale = 255 / (surface->dwRBitMask >> s);

	for (s = 0, m = surface->dwGBitMask; !(m & 1); s++, m >>= 1);
	g_gun->mask = surface->dwGBitMask;
	g_gun->shift = s;
	g_gun->scale = 255 / (surface->dwGBitMask >> s);

	for (s = 0, m = surface->dwBBitMask; !(m & 1); s++, m >>= 1);
	b_gun->mask = surface->dwBBitMask;
	b_gun->shift = s;
	b_gun->scale = 255 / (surface->dwBBitMask >> s);

	a_gun->mask = surface->dwRGBAlphaBitMask;

	// UP: Filter out cases which cause infinite loops
	if ((surface->dw_is_alpha_mode == true) && (surface->dwRGBAlphaBitMask != 0) ) 
	{	
		// UP: This is the exact line causing problems - it formed an infinite loop
		for (s = 0, m = surface->dwRGBAlphaBitMask; !(m & 1); s++, m >>= 1); 

		// Friendly debugging loop
//		for (s = 0, m = surface->dwRGBAlphaBitMask; !(m & 1); s++)
//			m >>= 1;

		a_gun->shift = s;
		a_gun->scale = 255 / (surface->dwRGBAlphaBitMask >> s);			
	} 
	else 
	{
		a_gun->shift = 0;
		a_gun->scale = 256;
	}
}

void d3d_release_rendering_objects()
{
	if ( GlobalD3DVars::lpD3DDevice )	{
		GlobalD3DVars::lpD3DDevice->Release();
		GlobalD3DVars::lpD3DDevice = NULL;
	}

//	DBUGFILE_OUTPUT_COUNTER(0, "Number of unfreed textures");
}

void gr_d3d_cleanup()
{
	if (!GlobalD3DVars::D3D_inited) return;

	d3d_tcache_cleanup();  

	// Ensures gamma options dont return to the desktop
	gr_d3d_set_gamma(1.0);

	// release surfaces
	d3d_release_rendering_objects();

	if ( GlobalD3DVars::lpD3D ) {
		GlobalD3DVars::lpD3D->Release();
		GlobalD3DVars::lpD3D = NULL; 
	}

	// restore windows clipping rectangle
 	ClipCursor(NULL);
	GlobalD3DVars::D3D_inited = 0;
}

bool d3d_init_win32(int screen_width, int screen_height)
{
	HWND hwnd = (HWND)os_get_window();

	if ( !hwnd )	{
		strcpy(Device_init_error, "Could not get application window handle");
		return false;
	}

	// windowed
	if(GlobalD3DVars::D3D_window)
	{
		SetWindowPos(hwnd, HWND_TOP, 0, 0, gr_screen.max_w, gr_screen.max_h, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME); 
		
		D3D_cursor_clip_rect.left = 0;
		D3D_cursor_clip_rect.top = 0;
		D3D_cursor_clip_rect.right = gr_screen.max_w-1;
		D3D_cursor_clip_rect.bottom = gr_screen.max_h-1;
	} else {
		// Prepare the window to go full screen
	#ifndef NDEBUG
		mprintf(( "Window in debugging mode... mouse clicking may cause problems!\n" ));
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		RECT work_rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
		SetWindowPos( hwnd, HWND_TOPMOST, work_rect.left, work_rect.top, gr_screen.max_w, gr_screen.max_h, 0 );	
		D3D_cursor_clip_rect.left = work_rect.left;
		D3D_cursor_clip_rect.top = work_rect.top;
		D3D_cursor_clip_rect.right = work_rect.left + gr_screen.max_w - 1;
		D3D_cursor_clip_rect.bottom = work_rect.top + gr_screen.max_h - 1;
	#else
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	
		D3D_cursor_clip_rect.left = 0;
		D3D_cursor_clip_rect.top = 0;
		D3D_cursor_clip_rect.right = gr_screen.max_w - 1;
		D3D_cursor_clip_rect.bottom = gr_screen.max_h - 1;
	#endif
	}

	SetActiveWindow(hwnd);
	SetForegroundWindow(hwnd);

	d3d_clip_cursor();
	return true;
}

/**
 * Determines value of D3d_rendition_uvs
 *
 * @return void
 */
void d3d_detect_texture_origin_32()
{
	int test_bmp = -1;
	ubyte data[32*32];
	color ac;
	uint pix1b(0), pix2b(0);

	mprintf(( "Detecting uv type...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac,255,255,255,255);
		
	memset( data, 0, 32*32 );
	data[15*32+15] = 14;
	
	test_bmp = bm_create( 8, 32, 32, data, BMP_AABITMAP );
	
	mprintf(( "Trial #1\n" ));
	GlobalD3DVars::D3D_rendition_uvs = 0;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	GlobalD3DVars::D3D_rendition_uvs = 1;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	bm_release(test_bmp);

	if ( (pix1b!=0) || (pix2b!=0)  )	{
		GlobalD3DVars::D3D_rendition_uvs = 1;
	} else {
		GlobalD3DVars::D3D_rendition_uvs = 0;
	}

	mprintf(( "Rendition uvs: %d\n", GlobalD3DVars::D3D_rendition_uvs ));
}
	
/**
 * Determines value of D3d_rendition_uvs
 *
 * @return void
 */
void d3d_detect_texture_origin_16()
{
	int test_bmp = -1;
	ubyte data[32*32];
	color ac;
	ushort pix1b(0), pix2b(0);

	mprintf(( "Detecting uv type...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac,255,255,255,255);
		
	memset( data, 0, 32*32 );
	data[15*32+15] = 14;
	
	test_bmp = bm_create( 8, 32, 32, data, BMP_AABITMAP );
	
	mprintf(( "Trial #1\n" ));
	GlobalD3DVars::D3D_rendition_uvs = 0;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	GlobalD3DVars::D3D_rendition_uvs = 1;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	bm_release(test_bmp);

	if ( (pix1b!=0) || (pix2b!=0)  )	{
		GlobalD3DVars::D3D_rendition_uvs = 1;
	} else {
		GlobalD3DVars::D3D_rendition_uvs = 0;
	}

	mprintf(( "Rendition uvs: %d\n", GlobalD3DVars::D3D_rendition_uvs ));
}

/**
 * Determines value of D3D_line_offset
 *
 * @return void
 */
void d3d_detect_line_offset_32()
{
	extern float D3D_line_offset;

	color ac;
	uint pix1a(0), pix2a(0);

	mprintf(( "Detecting line offset...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac, 255,255, 255, 255);
	
	mprintf(( "Trial #1\n" ));
	D3D_line_offset = 0.0f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3D_line_offset = 0.5f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	if ( (pix1a!=0) && (pix2a==0)  )	{
		D3D_line_offset = 0.0f;
	} else if ( (pix1a==0) && (pix2a!=0)  )	{
		D3D_line_offset = 0.5f;
	} else {
		D3D_line_offset = 0.0f;
	}

	mprintf(( "Line offset: %.1f\n", D3D_line_offset ));
}

/**
 * Determines value of D3D_line_offset
 *
 * @return void
 */
void d3d_detect_line_offset_16()
{
	extern float D3D_line_offset;

	color ac;
	ushort pix1a(0), pix2a(0);

	mprintf(( "Detecting line offset...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac, 255,255, 255, 255);
	
	mprintf(( "Trial #1\n" ));
	D3D_line_offset = 0.0f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3D_line_offset = 0.5f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	if ( (pix1a!=0) && (pix2a==0)  )	{
		D3D_line_offset = 0.0f;
	} else if ( (pix1a==0) && (pix2a!=0)  )	{
		D3D_line_offset = 0.5f;
	} else {
		D3D_line_offset = 0.0f;
	}

	mprintf(( "Line offset: %.1f\n", D3D_line_offset ));
}

/**
 * D3D8 Launcher func: Return bit type for modes, those not listed or simply not valid for FS2
 *
 * @return int, 32, 16 or 0 if not valid 
 * @param D3DFORMAT type 
 */
int d3d_get_mode_bit(D3DFORMAT type)
{
	switch(type)
	{
		case D3DFMT_X8R8G8B8: 
		case D3DFMT_A8R8G8B8:		
		//case D3DFMT_A2B10G10R10:	
			return 32;
			
		case D3DFMT_R8G8B8:
		case D3DFMT_R5G6B5:   
		case D3DFMT_X1R5G5B5: 
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A1R5G5B5:		
		case D3DFMT_A4R4G4B4:		
		case D3DFMT_A8R3G3B2:		
			return 16;
	}

	return 0;
}

/**
 * This checks that the given texture format is supported in the chosen adapter and mode
 *
 * @return bool
 * @param D3DFORMAT tformat
 */
bool d3d_texture_format_is_supported(D3DFORMAT tformat, int adapter, D3DDISPLAYMODE *mode)
{
	HRESULT hr;

	hr = GlobalD3DVars::lpD3D->CheckDeviceFormat(
			adapter,
			D3DDEVTYPE_HAL,
			mode->Format,
			0,
			D3DRTYPE_TEXTURE,
			tformat);

	return SUCCEEDED(hr); 
}

/**
 * Fills the old style direct draw DDPIXELFORMAT with details needed for later
 * We are not using direct draw, just making use of one of its structures 
 * The shift values are used to convert textures from load set to correct texture format
 *
 * @return void
 * @param DDPIXELFORMAT *pixelf
 * @param D3DFORMAT tformat
 */
void d3d_fill_pixel_format(PIXELFORMAT *pixelf, D3DFORMAT tformat)
{
	switch(tformat)
	{
		case D3DFMT_X8R8G8B8:
			pixelf->dwRGBBitCount      = 32;
			pixelf->dwRBitMask         = 0xff0000;      
			pixelf->dwGBitMask         = 0xff00;      
			pixelf->dwBBitMask         = 0xff;       
			pixelf->dw_is_alpha_mode   = false;
			pixelf->dwRGBAlphaBitMask  = 0;
			printf(("Using: D3DFMT_X8R8G8B8"));
			break;
		case D3DFMT_R8G8B8:
			pixelf->dwRGBBitCount      = 24;   
			pixelf->dwRBitMask         = 0xff0000;      
			pixelf->dwGBitMask         = 0xff00;      
			pixelf->dwBBitMask         = 0xff;           
			pixelf->dw_is_alpha_mode   = false;
			pixelf->dwRGBAlphaBitMask  = 0;
			printf(("Using: D3DFMT_R8G8B8"));
			break;
		case D3DFMT_X1R5G5B5:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0x7c00;      
			pixelf->dwGBitMask         = 0x3e0;      
			pixelf->dwBBitMask         = 0x1f;      
			pixelf->dw_is_alpha_mode   = false;
			pixelf->dwRGBAlphaBitMask  = 0;
			printf(("Using: D3DFMT_X1R5G5B5"));
			break;
		case D3DFMT_R5G6B5:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xf800;      
			pixelf->dwGBitMask         = 0x7e0;      
			pixelf->dwBBitMask         = 0x1f;      
			pixelf->dw_is_alpha_mode   = false;
			pixelf->dwRGBAlphaBitMask  = 0;
			printf(("Using: D3DFMT_R5G6B5"));
			break;
		case D3DFMT_X4R4G4B4:	 
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xf00;      
			pixelf->dwGBitMask         = 0xf0;      
			pixelf->dwBBitMask         = 0xf;
			pixelf->dw_is_alpha_mode   = false;
			pixelf->dwRGBAlphaBitMask  = 0;
			printf(("Using: D3DFMT_X4R4G4B4"));
			break;
		case D3DFMT_A8R8G8B8:		
			pixelf->dwRGBBitCount      = 32;   
			pixelf->dwRBitMask         = 0xff0000;      
			pixelf->dwGBitMask         = 0xff00;      
			pixelf->dwBBitMask         = 0xff;           
			pixelf->dwRGBAlphaBitMask  = 0xff000000;  
			pixelf->dw_is_alpha_mode   = true;
			printf(("Using: D3DFMT_A8R8G8B8"));
			break;
		case D3DFMT_A1R5G5B5:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0x7c00;      
			pixelf->dwGBitMask         = 0x3e0;       
			pixelf->dwBBitMask         = 0x1f;        
			pixelf->dwRGBAlphaBitMask  = 0x8000;
			pixelf->dw_is_alpha_mode   = true;
			printf(("Using: D3DFMT_A1R5G5B5"));
			break;
		case D3DFMT_A4R4G4B4:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xf00;      
			pixelf->dwGBitMask         = 0xf0;       
			pixelf->dwBBitMask         = 0xf;        
			pixelf->dwRGBAlphaBitMask  = 0xf000;;    
			pixelf->dw_is_alpha_mode   = true;
			printf(("Using: D3DFMT_A4R4G4B4"));
			break;
		case D3DFMT_A8R3G3B2:		
			pixelf->dwRGBBitCount      = 16;   
			pixelf->dwRBitMask         = 0xe0;       
			pixelf->dwGBitMask         = 0x1c;      
			pixelf->dwBBitMask         = 0x3;      
			pixelf->dwRGBAlphaBitMask  = 0xff00;
			pixelf->dw_is_alpha_mode   = true;
			printf(("Using: D3DFMT_A8R3G3B2"));
			break;
		/*case D3DFMT_A2B10G10R10:
			pixelf->dwRGBBitCount      = 32;   
			pixelf->dwRBitMask         = 0x3ff00000;      
			pixelf->dwGBitMask         = 0xffc00;      
			pixelf->dwBBitMask         = 0x3ff;      
			pixelf->dwRGBAlphaBitMask  = 0xc0000000;
			pixelf->dwFlags			   = DDPF_ALPHAPIXELS;
			mprintf(("Using: D3DFMT_A2B10G10R10"));
			break;*/
	}
}

/**
 * Works out the best texture formats to use in the chosen mode
 *
 * @return void
 */
void d3d_determine_texture_formats(int adapter, D3DDISPLAYMODE *mode)
{
	const int num_non_alpha = 3;
	const int num_alpha     = 4;

	default_non_alpha_tformat	 = D3DFMT_UNKNOWN;
	default_alpha_tformat		 = D3DFMT_UNKNOWN;

	// Non alpha (listed from best to worst)
	D3DFORMAT non_alpha_list[num_non_alpha] =
	{
		D3DFMT_A1R5G5B5,		
		D3DFMT_A4R4G4B4,		
		D3DFMT_A8R3G3B2,
	};

	// Alpha (listed from best to worst)
	D3DFORMAT alpha_list[num_alpha] =
	{
		D3DFMT_A4R4G4B4,		
		D3DFMT_A1R5G5B5,		
		D3DFMT_A8R3G3B2,		
	};

	// Go through the alpha list and find a texture format of a valid depth
	// and is supported in this adapter mode
	for(int i = 0; i < num_alpha; i++) {
		if(d3d_get_mode_bit(alpha_list[i]) != 16) {
			continue;
		}

		if(d3d_texture_format_is_supported(alpha_list[i], adapter, mode) == true) {
			default_alpha_tformat = alpha_list[i];
			break;
		}
	}

	// If this is unknown this has failed
	if(default_alpha_tformat == D3DFMT_UNKNOWN) {
		mprintf(("alpha texture format not selected"));
	} else {
		d3d_fill_pixel_format(&AlphaTextureFormat, default_alpha_tformat);
	}

		// Go through the non alpha list and find a texture format of a valid depth
	// and is supported in this adapter mode
	for(i = 0; i < num_non_alpha; i++)
	{
		if(d3d_get_mode_bit(non_alpha_list[i]) != 16)
		{
			continue;
		}

		if(d3d_texture_format_is_supported(non_alpha_list[i], adapter, mode) == true)
		{
			default_non_alpha_tformat = non_alpha_list[i];
			break;
		}
	}

	// If this is unknown this has failed
	if(default_non_alpha_tformat == D3DFMT_UNKNOWN)
	{
		mprintf(("non alpha texture format not selected"));
	}
	else
	{
		// Um hack here, forget about non alpha formats!
		d3d_fill_pixel_format(&NonAlphaTextureFormat, default_non_alpha_tformat);
	}

	if(	d3d_get_mode_bit(mode->Format) < 32)
		Cmdline_32bit_textures = 0;
}

/**
 * Sets up all the graphics function pointers to the relevent d3d functions
 *
 * @return void
 */
void d3d_setup_function_pointers()
{
		// Set all the pointer to functions to the correct D3D functions
	gr_screen.gf_flip = gr_d3d_flip;
	gr_screen.gf_set_clip = gr_d3d_set_clip;
	gr_screen.gf_reset_clip = gr_d3d_reset_clip;
	gr_screen.gf_set_font = grx_set_font;

	gr_screen.gf_get_color = gr_d3d_get_color;
	gr_screen.gf_init_color = gr_d3d_init_color;
	gr_screen.gf_set_color_fast = gr_d3d_set_color_fast;
	gr_screen.gf_set_color = gr_d3d_set_color;
	gr_screen.gf_init_color = gr_d3d_init_color;
	gr_screen.gf_init_alphacolor = gr_d3d_init_alphacolor;

	gr_screen.gf_set_bitmap = gr_d3d_set_bitmap;
	gr_screen.gf_create_shader = gr_d3d_create_shader;
	gr_screen.gf_set_shader = gr_d3d_set_shader;
	gr_screen.gf_clear = gr_d3d_clear;
	gr_screen.gf_aabitmap = gr_d3d_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_d3d_aabitmap_ex;

	gr_screen.gf_rect = gr_d3d_rect;
	gr_screen.gf_shade = gr_d3d_shade;
	gr_screen.gf_string = gr_d3d_string;
	gr_screen.gf_circle = gr_d3d_circle;

	gr_screen.gf_line = gr_d3d_line;
	gr_screen.gf_aaline = gr_d3d_aaline;
	gr_screen.gf_pixel = gr_d3d_pixel;
	gr_screen.gf_scaler = gr_d3d_scaler;
	gr_screen.gf_aascaler = gr_d3d_aascaler;
	gr_screen.gf_tmapper = gr_d3d_tmapper;

	gr_screen.gf_gradient = gr_d3d_gradient;

	gr_screen.gf_set_palette = gr_d3d_set_palette;
	gr_screen.gf_print_screen = gr_d3d_print_screen;

	gr_screen.gf_fade_in = gr_d3d_fade_in;
	gr_screen.gf_fade_out = gr_d3d_fade_out;
	gr_screen.gf_flash = gr_d3d_flash;

	gr_screen.gf_zbuffer_get = gr_d3d_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_d3d_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_d3d_zbuffer_clear;

	gr_screen.gf_save_screen = gr_d3d_save_screen;
	gr_screen.gf_restore_screen = gr_d3d_restore_screen;
	gr_screen.gf_free_screen = gr_d3d_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr_d3d_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_d3d_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_d3d_dump_frame;

	gr_screen.gf_set_gamma = gr_d3d_set_gamma;

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_d3d_lock;
	gr_screen.gf_unlock = gr_d3d_unlock;

	// screen region
	gr_screen.gf_get_region = gr_d3d_get_region;

	// fog stuff
	gr_screen.gf_fog_set = gr_d3d_fog_set;

	// poly culling
	gr_screen.gf_set_cull = gr_d3d_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_d3d_cross_fade;

	// filtering
	gr_screen.gf_filter_set = gr_d3d_filter_set;

	// texture cache
	gr_screen.gf_tcache_set = d3d_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_d3d_set_clear_color;

	// now for the bitmap functions
	gr_screen.gf_bm_set_max_bitmap_size     = bm_d3d_set_max_bitmap_size;     
	gr_screen.gf_bm_get_next_handle         = bm_d3d_get_next_handle;         
	gr_screen.gf_bm_close                   = bm_d3d_close;                   
	gr_screen.gf_bm_init                    = bm_d3d_init;                    
	gr_screen.gf_bm_get_frame_usage         = bm_d3d_get_frame_usage;         
	gr_screen.gf_bm_create                  = bm_d3d_create;                  
	gr_screen.gf_bm_load                    = bm_d3d_load;                   
	gr_screen.gf_bm_load_duplicate          = bm_d3d_load_duplicate;          
	gr_screen.gf_bm_load_animation          = bm_d3d_load_animation;          
	gr_screen.gf_bm_get_info                = bm_d3d_get_info;                
	gr_screen.gf_bm_lock                    = bm_d3d_lock;                    
	gr_screen.gf_bm_unlock                  = bm_d3d_unlock;                  
	gr_screen.gf_bm_get_palette             = bm_d3d_get_palette;             
	gr_screen.gf_bm_release                 = bm_d3d_release;                 
	gr_screen.gf_bm_unload                  = bm_d3d_unload;                  
	gr_screen.gf_bm_unload_all              = bm_d3d_unload_all;              
	gr_screen.gf_bm_page_in_texture         = bm_d3d_page_in_texture;         
	gr_screen.gf_bm_page_in_start           = bm_d3d_page_in_start;           
	gr_screen.gf_bm_page_in_stop            = bm_d3d_page_in_stop;            
	gr_screen.gf_bm_get_cache_slot          = bm_d3d_get_cache_slot;          
	gr_screen.gf_bm_get_components          = bm_d3d_get_components;          
	gr_screen.gf_bm_get_section_size        = bm_d3d_get_section_size;      
	
	gr_screen.gf_bm_page_in_nondarkening_texture = bm_d3d_page_in_nondarkening_texture; 
	gr_screen.gf_bm_page_in_xparent_texture		 = bm_d3d_page_in_xparent_texture;		 
	gr_screen.gf_bm_page_in_aabitmap			 = bm_d3d_page_in_aabitmap;	 
	
	gr_screen.gf_push_texture_matrix = gr_d3d_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix = gr_d3d_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix = gr_d3d_translate_texture_matrix;

	if(!Cmdline_nohtl) {
		gr_screen.gf_make_buffer = gr_d3d_make_buffer;
		gr_screen.gf_destroy_buffer = gr_d3d_destroy_buffer;
		gr_screen.gf_render_buffer = gr_d3d_render_buffer;

		gr_screen.gf_set_proj_matrix				= gr_d3d_set_proj_matrix;
		gr_screen.gf_end_proj_matrix				= gr_d3d_end_proj_matrix;
		gr_screen.gf_set_view_matrix				= gr_d3d_set_view_matrix;
		gr_screen.gf_end_view_matrix				= gr_d3d_end_view_matrix;
		gr_screen.gf_push_scale_matrix				= gr_d3d_set_scale_matrix;
		gr_screen.gf_pop_scale_matrix				= gr_d3d_end_scale_matrix;
		gr_screen.gf_start_instance_matrix			= gr_d3d_start_instance_matrix;
		gr_screen.gf_start_angles_instance_matrix	= gr_d3d_start_angles_instance_matrix;
		gr_screen.gf_end_instance_matrix			= gr_d3d_end_instance_matrix;

		gr_screen.gf_make_light = gr_d3d_make_light;
		gr_screen.gf_modify_light = gr_d3d_modify_light;
		gr_screen.gf_destroy_light = gr_d3d_destroy_light;
		gr_screen.gf_set_light = gr_d3d_set_light;
		gr_screen.gf_reset_lighting = gr_d3d_reset_lighting;

		gr_screen.gf_lighting = gr_d3d_lighting;

		gr_screen.start_clip_plane = d3d_start_clip;
		gr_screen.end_clip_plane = d3d_end_clip;
	}

}

int d3d_match_mode(int adapter)
{
	char *ptr = os_config_read_string(NULL, NOX("videocardFs2open"), NULL);	
	uint width, height;
	int cdepth;

	if(ptr == NULL)
	{
		strcpy(Device_init_error, "Cant get 'videocardFs2open' reg entry");
		return -1;
	}

	if(sscanf(ptr, "D3D8-(%dx%d)x%d bit", &width, &height, &cdepth)  != 3) {
		strcpy(Device_init_error, "Cant understand 'videocardFs2open' reg entry");
		return -1;
	}

	int num_modes = GlobalD3DVars::lpD3D->GetAdapterModeCount(adapter);

	if(num_modes == 0) {
		strcpy(Device_init_error, "No modes for this adapter");
		return -1;
	}

	for(int i = 0; i < num_modes; i++)
	{
		D3DDISPLAYMODE mode;
		GlobalD3DVars::lpD3D->EnumAdapterModes(adapter, i, &mode); 

		// ignore invalid modes
		if(cdepth != d3d_get_mode_bit(mode.Format)) continue; 
		if(width  != mode.Width)  continue; 
		if(height != mode.Height) continue; 

		// This is the mode we want
		return i;
	}

	strcpy(Device_init_error, "No suitable mode found");
	return -1;
}

/**
 * This deals totally with mode stuff
 *
 * @return bool
 */
bool d3d_init_device()
{
	D3DDISPLAYMODE mode;

	int adapter_choice = D3DADAPTER_DEFAULT;

	// Set up the common device	parameters
	ZeroMemory( &GlobalD3DVars::d3dpp, sizeof(GlobalD3DVars::d3dpp) );

	GlobalD3DVars::d3dpp.BackBufferCount		 = 1;
	GlobalD3DVars::d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    GlobalD3DVars::d3dpp.EnableAutoDepthStencil = TRUE;
	// This gets changed later
    GlobalD3DVars::d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if(Cmdline_nohtl) {
		// Only need this for software fog
		GlobalD3DVars::d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	}
    
	extern int Cmdline_window;
	extern int D3D_window;
	GlobalD3DVars::D3D_window = Cmdline_window ? true : false;

	// Get caps
	bool got_caps = 
		SUCCEEDED(
			GlobalD3DVars::lpD3D->GetDeviceCaps(
				adapter_choice, D3DDEVTYPE_HAL, &GlobalD3DVars::d3d_caps));

	if (GlobalD3DVars::D3D_window) {	
		// If we go windowed, then we need to adjust some other present parameters		
		if (FAILED(GlobalD3DVars::lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode )))
		{
			strcpy(Device_init_error, "Could not get adapter display mode");
			return false;
		}

		GlobalD3DVars::d3dpp.MultiSampleType  = D3DMULTISAMPLE_NONE;
		GlobalD3DVars::d3dpp.BackBufferWidth  = 1024;
		GlobalD3DVars::d3dpp.BackBufferHeight = 768;

		GlobalD3DVars::d3dpp.FullScreen_RefreshRateInHz      = 0;
		GlobalD3DVars::d3dpp.FullScreen_PresentationInterval = 0;

		GlobalD3DVars::d3dpp.Windowed		   = TRUE;
	} else {

		// Attempt to get options from the registry
		adapter_choice     = os_config_read_uint( NULL, "D3D8_Adapter", 0xffff);
		int aatype_choice  = os_config_read_uint( NULL, "D3D8_AAType",  0x0);
		int mode_choice	   = d3d_match_mode(adapter_choice);
		
		if(mode_choice == -1)
		{
			strcpy(Device_init_error, "Couldnt match mode");
			return false;
		}

		// Should only activate if a value in the registry is not set or its going to run in a window or
		// a optional parameter forces it to run. Otherwise the mode values are taken from reg value
		if( adapter_choice == 0xffff)
		{
			strcpy(Device_init_error, "DX8 options not set, please run launcher");
			return false;
		}

		if(FAILED(GlobalD3DVars::lpD3D->EnumAdapterModes(adapter_choice, mode_choice, &mode)))
		{
			sprintf(Device_init_error, "Could not use selected mode: %d", mode_choice);
			return false;
		}

		GlobalD3DVars::D3D_Antialiasing = (aatype_choice != 0);

		GlobalD3DVars::d3dpp.MultiSampleType  = multisample_types[aatype_choice];
		GlobalD3DVars::d3dpp.BackBufferWidth  = mode.Width;
		GlobalD3DVars::d3dpp.BackBufferHeight = mode.Height;

		// Determine if we are using a custom size
		if((mode.Width != 1024 && mode.Height != 768) &&
		   (mode.Width !=  640 && mode.Height != 480) ) {
			GlobalD3DVars::D3D_custom_size = GR_1024;

			// Override these values
			gr_screen.max_w = mode.Width;
			gr_screen.max_h = mode.Height;
			gr_screen.clip_right  = gr_screen.max_w - 1;
			gr_screen.clip_bottom = gr_screen.max_h - 1;
			gr_screen.clip_width  = gr_screen.max_w;
			gr_screen.clip_height = gr_screen.max_h;

		}

		GlobalD3DVars::d3dpp.FullScreen_RefreshRateInHz      = D3DPRESENT_RATE_DEFAULT;
		GlobalD3DVars::d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

		if(got_caps && GlobalD3DVars::d3d_caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) {
			GlobalD3DVars::d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		GlobalD3DVars::d3dpp.Windowed		  = FALSE;
	}

	GlobalD3DVars::d3dpp.BackBufferFormat = mode.Format;

	//trying to use a higher bit depth in the back buffer, the deepest one posale -Bobboau
	const int NUM_FORMATS = 3;
	enum _D3DFORMAT format_type[NUM_FORMATS] = {D3DFMT_D24X8, D3DFMT_D32, D3DFMT_D16};

	const int NUM_VPTYPES = 3;
	DWORD vptypes[NUM_VPTYPES] = {
		D3DCREATE_HARDWARE_VERTEXPROCESSING, 
			D3DCREATE_MIXED_VERTEXPROCESSING, 
			D3DCREATE_SOFTWARE_VERTEXPROCESSING};

	// Use a pure device if we can, it really speeds things up
	// However it means we cant use Get* functions (aside from GetFrontBuffer, GetCaps, etc)
	if(got_caps && GlobalD3DVars::d3d_caps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
	 	vptypes[0] |= D3DCREATE_PUREDEVICE;
	}

	bool have_device = false;
	for(int vp = 0;	vp < NUM_VPTYPES; vp++)					 
	{
		// It trys to use the highest suported back buffer through trial and error, 
		// I wraped the existing code in a for loop to cycle through the diferent formats -Bobboau
		for(int t = 0; t < NUM_FORMATS; t++){
		
			GlobalD3DVars::d3dpp.AutoDepthStencilFormat = format_type[t];
		
			if( SUCCEEDED( GlobalD3DVars::lpD3D->CreateDevice(
									adapter_choice, 
									D3DDEVTYPE_HAL, 
									(HWND) os_get_window(),
									vptypes[vp],
		                            &GlobalD3DVars::d3dpp, 
									&GlobalD3DVars::lpD3DDevice) ) ) {
				have_device = true;
				break;
			}
		}

		if(have_device == true) {
			break;
		}
	}

	if (have_device == false) {
		sprintf(Device_init_error, "Failed to get init mode");
		return false;
	} 

	GlobalD3DVars::d3dpp.BackBufferWidth = mode.Width;

	d3d_determine_texture_formats(adapter_choice, &mode);


	return true;
}

void d3d_init_vars_from_reg()
{
	// Not sure now relevent this is now
	uint tmp = os_config_read_uint( NULL, "D3DTextureOrigin", 0xFFFF );

	if ( tmp != 0xFFFF )	{
		GlobalD3DVars::D3D_rendition_uvs = ( tmp ) ? 1 : 0;
	} 
	
#if 0
	else {
		if(D3D_32bit){
			d3d_detect_texture_origin_32();
		} else {
			d3d_detect_texture_origin_16();
		}
	}

	mprintf(("D3d_rendition_uvs: %d",GlobalD3DVars::D3D_rendition_uvs));

	// Not sure now relevent this is now
	tmp = os_config_read_uint( NULL, "D3DLineOffset", 0xFFFF );

	extern float D3D_line_offset;
	if ( tmp != 0xFFFF )	{
		D3D_line_offset = ( tmp ) ? 0.5f : 0.0f;
	} else {
		if(D3D_32bit){
			d3d_detect_line_offset_32();
		} else {
			d3d_detect_line_offset_16();
		}
	}
#endif

	// zbiasing?
	if(os_config_read_uint(NULL, "DisableZbias", 0)){
		GlobalD3DVars::D3D_zbias = 0;
	}  
}

void d3d_setup_color()
{
	// RT This stuff is needed for software fog
	Gr_current_red =   &Gr_red;
	Gr_current_blue =  &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

	PIXELFORMAT temp_format;
	d3d_fill_pixel_format(&temp_format, GlobalD3DVars::d3dpp.BackBufferFormat);

	d3d_setup_format_components(&temp_format, Gr_current_red, Gr_current_green, Gr_current_blue, Gr_current_alpha);

	d3d_setup_format_components(&NonAlphaTextureFormat, &Gr_t_red, &Gr_t_green, &Gr_t_blue, &Gr_t_alpha);
	d3d_setup_format_components(&AlphaTextureFormat, &Gr_ta_red, &Gr_ta_green, &Gr_ta_blue, &Gr_ta_alpha);

	mprintf(( "Alpha texture format = " ));
	d3d_dump_format(&AlphaTextureFormat);

	mprintf(( "Non-alpha texture format = " ));
	d3d_dump_format(&NonAlphaTextureFormat);

	if(!Cmdline_nohtl) {

		extern D3DMATERIAL8 material;
		ZeroMemory(&material,sizeof(D3DMATERIAL8));
		material.Diffuse.r = 1.0f;
		material.Diffuse.g = 1.0f;
		material.Diffuse.b = 1.0f;
		material.Diffuse.a = 1.0f;

		material.Ambient.r = 1.0f;
		material.Ambient.g = 1.0f;
		material.Ambient.b = 1.0f;
		material.Ambient.a = 1.0f;

		material.Emissive.r = 0.0f;
		material.Emissive.g = 0.0f;
		material.Emissive.b = 0.0f;
		material.Emissive.a = 0.0f;

 		material.Specular.r = 1.0f;
		material.Specular.g = 1.0f;
		material.Specular.b = 1.0f;
		material.Specular.a = 1.0f;

		material.Power = 16.0f;

		GlobalD3DVars::lpD3DDevice->SetMaterial(&material);
	}

	// determine 32 bit status
	gr_screen.bits_per_pixel  = d3d_get_mode_bit(GlobalD3DVars::d3dpp.BackBufferFormat);
	gr_screen.bytes_per_pixel = gr_screen.bits_per_pixel / 8;
	D3D_32bit                 = gr_screen.bits_per_pixel == 32 ? 1 : 0;
	mprintf(("D3D_32bit %d, bits_per_pixel %d",D3D_32bit, gr_screen.bits_per_pixel));
}

bool d3d_inspect_caps()
{
	GlobalD3DVars::lpD3DDevice->GetDeviceCaps(&GlobalD3DVars::d3d_caps);
	
	int not_good = 0;

	char missing_features[128*1024];

	strcpy( missing_features, XSTR("Your video card is missing the following features required by FreeSpace:\r\n\r\n",623) );

	// fog
	if ( !(GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX) && 
		!(GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGTABLE)){

		strcat( missing_features, XSTR("Vertex fog or Table fog\r\n", 1499) );
		not_good++;			
	}		
			
	// Texture blending values
	if ( !(GlobalD3DVars::d3d_caps.TextureOpCaps & D3DTEXOPCAPS_MODULATE  ))	{
		strcat( missing_features, XSTR("Texture blending mode = Modulate\r\n", 624) );
		not_good++;
	}

	if ( !(GlobalD3DVars::d3d_caps.SrcBlendCaps & (D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_BOTHSRCALPHA)) )	{
		strcat( missing_features, XSTR("Source blending mode = SRCALPHA or BOTHSRCALPHA\r\n", 625) );
		not_good++;
	}

	// Dest blending values
	if ( !(GlobalD3DVars::d3d_caps.DestBlendCaps & (D3DPBLENDCAPS_INVSRCALPHA|D3DPBLENDCAPS_BOTHINVSRCALPHA)) )	{
		strcat( missing_features, XSTR("Destination blending mode = INVSRCALPHA or BOTHINVSRCALPHA\r\n",626) );
		not_good++;
	}
	
	if ( not_good )	{
		strcpy(Device_init_error, missing_features);
	//	return false; I dont think it should fail on this - RT
	}	

	return true;
}


bool gr_d3d_init()
{
	GlobalD3DVars::lpD3D = Direct3DCreate8( D3D_SDK_VERSION );

	if( GlobalD3DVars::lpD3D == NULL ) {
		sprintf(Device_init_error, "Please make sure you have DX8.1b installed");
		return false;
	}

	ShowCursor(false);

	if(d3d_init_device() == false) {
		// Func will give its own errors
		return false;
	}
											   
	if(d3d_inspect_caps() == false) {
		// Func will give its own errors
		return false;
	}

	if(d3d_init_win32(GlobalD3DVars::d3dpp.BackBufferWidth, GlobalD3DVars::d3dpp.BackBufferHeight) == false) {
		// Func will give its own errors
		return false;
	}	

	d3d_setup_color();

	// Tell Freespace code that we're using Direct3D.
	D3D_enabled    = 1;
  	Gr_bitmap_poly = 1;
	GlobalD3DVars::D3D_inited = true;	

  	d3d_tcache_init();

	d3d_setup_function_pointers();
	d3d_init_vars_from_reg();

	d3d_reset_render_states();
	d3d_reset_texture_stage_states();

	d3d_set_initial_render_state();

	gr_d3d_activate(1);
	d3d_start_frame();

	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	TIMERBAR_SET_DRAW_FUNC(d3d_render_timer_bar);
	mprintf(( "Direct3D Initialized OK!\n" ));

	return true;

}

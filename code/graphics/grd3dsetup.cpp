/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Log: not supported by cvs2svn $
 * Revision 2.59.2.2  2006/11/15 00:47:57  taylor
 * properly support the updated window create code (all told: should take of of Mantis bugs #542, #624, #1140, and possibly #962 and #1124)
 *
 * Revision 2.59.2.1  2006/08/22 05:41:35  taylor
 * clean up the grstub mess (for work on standalone server, and just for sanity sake)
 * move color and shader functions to 2d.cpp since they are exactly the same everywhere
 * don't bother with the function pointer for gr_set_font(), it's the same everywhere anyway
 *
 * Revision 2.59  2006/06/01 04:44:19  taylor
 * fix the scope issues that someone complained about in the forums
 *
 * Revision 2.58  2006/05/27 17:07:48  taylor
 * remove grd3dparticle.* and grd3dbatch.*, they are obsolete
 * allow us to build without D3D support under Windows (just define NO_DIRECT3D)
 * clean up TMAP flags
 * fix a couple of minor OpenGL state change issues with spec and env map rendering
 * make sure we build again for OS X (OGL extension functions work a little different there)
 * render targets always need to be power-of-2 to avoid incomplete buffer issues in the code
 * when we disable culling in opengl_3dunlit be sure that we re-enable it on exit of function
 * re-fix screenshots
 * add true alpha blending support (with cmdline for now since the artwork has the catch up)
 * draw lines with float positioning, to be more accurate with resizing on non-standard resolutions
 * don't load cubemaps from file for D3D, not sure how to do it anyway
 * update geometry batcher code, memory fixes, dynamic stuff, basic fixage, etc.
 *
 * Revision 2.57  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.56  2006/04/15 00:13:22  phreak
 * gr_flash_alpha(), much like gr_flash(), but allows an alpha value to be passed
 *
 * Revision 2.55  2006/02/25 21:47:00  Goober5000
 * spelling
 *
 * Revision 2.54  2006/01/20 17:15:16  taylor
 * gr_*_bitmap_ex() stuff, D3D side is 100% untested to even compile
 * several other very minor changes as well
 *
 * Revision 2.53  2005/12/06 02:53:02  taylor
 * clean up some D3D debug messages to better match new OGL messages (for easier debugging)
 * remove D3D_32bit variable since it's basically useless and the same thing can be done another way
 *
 * Revision 2.52  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 2.51  2005/08/20 20:34:51  taylor
 * some bmpman and render_target function name changes so that they make sense
 * always use bm_set_render_target() rather than the gr_ version so that the graphics state is set properly
 * save the original gamma ramp on OGL init so that it can be restored on exit
 *
 * Revision 2.50  2005/05/01 23:23:18  phreak
 * added CVS info to top of file.
 *
 *
*/

#ifndef NO_DIRECT3D

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
#include "graphics/grd3dstateblock.h"

#include "debugconsole/timerbar.h"
#include "debugconsole/dbugfile.h"
#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"



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
int GlobalD3DVars::D3D_zbias         = 1;

float GlobalD3DVars::texture_adjust_u = 0;
float GlobalD3DVars::texture_adjust_v = 0;

ID3DXMatrixStack *world_matrix_stack, *view_matrix_stack, *proj_matrix_stack;

const int MULTISAMPLE_MAX = 16;
const D3DMULTISAMPLE_TYPE multisample_types[MULTISAMPLE_MAX] =
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
D3DFORMAT default_compressed_format = D3DFMT_UNKNOWN;

void d3d_clip_cursor()
{
	if (Cmdline_window) return;

	GetWindowRect((HWND)os_get_window(), &D3D_cursor_clip_rect);
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

extern int Gr_inited;

void gr_d3d_activate(int active)
{
	//mprintf(( "Direct3D activate: %d\n", active ));

	HWND hwnd = (HWND)os_get_window();
	
	if ( active  )	{
		GlobalD3DVars::D3D_activate = 1;

		if ( hwnd )	{
			ShowWindow(hwnd,SW_RESTORE);
			UpdateWindow( hwnd );
			if(!Cmdline_window){
				d3d_lost_device(true);
			}
			GetWindowRect(hwnd, &D3D_cursor_clip_rect);
			d3d_clip_cursor();
			ShowCursor(FALSE);
		}

	} else {

		GlobalD3DVars::D3D_activate = 0;

		if ( hwnd )	{
//			d3d_lost_device();
			if(!Cmdline_window){
				GlobalD3DVars::d3dpp.Windowed = true;
				d3d_lost_device(true);
				//you will minimize! /*twitch*/
				GlobalD3DVars::d3dpp.Windowed = false;
			}
			ClipCursor(NULL);
			ShowCursor(TRUE);
			UpdateWindow( hwnd );
			ShowWindow(hwnd,SW_MINIMIZE);
		}
	}
//	ID3DXMatrixStack *world_matrix_stack, *view_matrix_stack, *proj_matrix_stack;

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

bool d3d_init_win32(int screen_width, int screen_height)
{
	HWND hwnd = (HWND)os_get_window();
//	RECT rect;

	if ( !hwnd )	{
		strcpy(Device_init_error, "Could not get application window handle");
		return false;
	}
/*
	// windowed
	if(GlobalD3DVars::D3D_window)
	{
		GetClientRect(hwnd, &rect);
		SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.bottom, rect.right, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME); 
	} else {
		// Prepare the window to go full screen
	#ifndef NDEBUG
		mprintf(( "  Window in debugging mode... mouse clicking may cause problems!\n" ));
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		RECT work_rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
		SetWindowPos( hwnd, HWND_TOPMOST, work_rect.left, work_rect.top, gr_screen.max_w, gr_screen.max_h, 0 );	
	#else
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	
	#endif
	}
*/

	SetForegroundWindow(hwnd);
	SetActiveWindow(hwnd);
	SetFocus(hwnd);

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
			pixelf->dwRGBAlphaBitMask  = 0xf000;
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
	int i;

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
	for(i = 0; i < num_alpha; i++) {
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

	// Check compressed textures here
	Use_compressed_textures = ( d3d_texture_format_is_supported(D3DFMT_DXT1, adapter, mode) &&
								d3d_texture_format_is_supported(D3DFMT_DXT3, adapter, mode) &&
								d3d_texture_format_is_supported(D3DFMT_DXT5, adapter, mode) );

	Texture_compression_available = ( d3d_texture_format_is_supported(D3DFMT_A8R8G8B8, adapter, mode) &&
									  d3d_texture_format_is_supported(D3DFMT_X8R8G8B8, adapter, mode) &&
									  Use_compressed_textures );
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

	gr_screen.gf_set_bitmap = gr_d3d_set_bitmap;
	gr_screen.gf_clear = gr_d3d_clear;
	gr_screen.gf_bitmap_ex = gr_d3d_bitmap_ex;
	gr_screen.gf_aabitmap = gr_d3d_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_d3d_aabitmap_ex;

//	gr_screen.gf_rect = gr_d3d_rect;
//	gr_screen.gf_shade = gr_d3d_shade;
	gr_screen.gf_string = gr_d3d_string;
	gr_screen.gf_circle = gr_d3d_circle;
	gr_screen.gf_curve = gr_d3d_curve;

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
	gr_screen.gf_flash_alpha = gr_d3d_flash_alpha;

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

	// preload a bitmap into texture memory
	gr_screen.gf_preload = gr_d3d_preload;

	// now for the bitmap functions
	gr_screen.gf_bm_free_data				= gr_d3d_bm_free_data;
	gr_screen.gf_bm_create					= gr_d3d_bm_create;
	gr_screen.gf_bm_init					= gr_d3d_bm_init;
	gr_screen.gf_bm_load					= gr_d3d_bm_load;
	gr_screen.gf_bm_page_in_start			= gr_d3d_bm_page_in_start;
	gr_screen.gf_bm_lock					= gr_d3d_bm_lock;
	gr_screen.gf_bm_make_render_target		= gr_d3d_bm_make_render_target;
	gr_screen.gf_bm_set_render_target		= gr_d3d_bm_set_render_target;
	
	
	
	gr_screen.gf_push_texture_matrix = gr_d3d_push_texture_matrix;
	gr_screen.gf_pop_texture_matrix = gr_d3d_pop_texture_matrix;
	gr_screen.gf_translate_texture_matrix = gr_d3d_translate_texture_matrix;

	gr_screen.gf_set_texture_addressing = gr_d3d_set_texture_addressing;
	gr_screen.gf_zbias = gr_d3d_zbias;
	gr_screen.gf_set_fill_mode = gr_d3d_set_fill_mode;
	gr_screen.gf_set_texture_panning = d3d_set_texture_panning;

	gr_screen.gf_start_state_block = gr_d3d_start_state_block;
	gr_screen.gf_end_state_block = gr_d3d_end_state_block;
	gr_screen.gf_set_state_block = gr_d3d_set_state_block;

	if(!Cmdline_nohtl) {
		gr_screen.gf_make_buffer = gr_d3d_make_buffer;
		gr_screen.gf_destroy_buffer = gr_d3d_destroy_buffer;
		gr_screen.gf_render_buffer = gr_d3d_render_buffer;
		gr_screen.gf_set_buffer = gr_d3d_set_buffer;
		gr_screen.gf_make_flat_buffer = gr_d3d_make_flat_buffer;
		gr_screen.gf_make_line_buffer = gr_d3d_make_line_buffer;

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
		gr_screen.gf_set_ambient_light = gr_d3d_set_ambient_light;

		gr_screen.gf_lighting = gr_d3d_lighting;
		gr_screen.gf_center_alpha = gr_d3d_center_alpha;

		gr_screen.gf_start_clip_plane = gr_d3d_start_clip;
		gr_screen.gf_end_clip_plane   = gr_d3d_end_clip;
		gr_screen.gf_setup_background_fog	= gr_d3d_setup_background_fog;
		gr_screen.gf_draw_line_list			= gr_d3d_draw_line_list;
        gr_screen.gf_set_line_width = gr_d3d_set_line_width;
		gr_screen.gf_draw_htl_line = gr_d3d_draw_htl_line;
		gr_screen.gf_draw_htl_sphere = gr_d3d_draw_htl_sphere;
//		gr_screen.gf_set_environment_mapping = d3d_render_to_env;
	}

}

int d3d_match_mode(int adapter)
{
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
		if(gr_screen.bits_per_pixel != d3d_get_mode_bit(mode.Format)) continue; 
		if(gr_screen.max_w          != (int)mode.Width)  continue; 
		if(gr_screen.max_h          != (int)mode.Height) continue; 

		// This is the mode we want
		return i;
	}

	strcpy(Device_init_error, "No suitable mode found");
	return -1;
}

int d3d_check_multisample_types(int adapter, int chosen_ms, D3DFORMAT back_buffer_format, D3DFORMAT depth_buffer_format)
{
	HRESULT hr;
	int i;

	// If fails try a lesser mode
	for(i = chosen_ms; i >= 0; i--)
	{
		hr = GlobalD3DVars::lpD3D->CheckDeviceMultiSampleType( 
			adapter, D3DDEVTYPE_HAL, back_buffer_format, GlobalD3DVars::D3D_window, multisample_types[i]);

		if(FAILED(hr)) 
		{
			DBUGFILE_OUTPUT_1("Failed aa %d",i);
			continue;
		}

		hr = GlobalD3DVars::lpD3D->CheckDeviceMultiSampleType( 
			adapter, D3DDEVTYPE_HAL, depth_buffer_format, GlobalD3DVars::D3D_window, multisample_types[i]);

		if(FAILED(hr)) 
		{
			DBUGFILE_OUTPUT_1("Failed aa %d",i);
			continue;
		}

		// Success!
		DBUGFILE_OUTPUT_1("Success aa %d",i);
		return i;
	}

	// Now try higher modes
	for(i = (chosen_ms + 1); i < MULTISAMPLE_MAX; i++)
	{
		hr = GlobalD3DVars::lpD3D->CheckDeviceMultiSampleType( 
			adapter, D3DDEVTYPE_HAL, back_buffer_format, GlobalD3DVars::D3D_window, multisample_types[i]);

		if(FAILED(hr)) 
		{
			DBUGFILE_OUTPUT_1("Failed aa %d",i);
			continue;
		}

		hr = GlobalD3DVars::lpD3D->CheckDeviceMultiSampleType( 
			adapter, D3DDEVTYPE_HAL, depth_buffer_format, GlobalD3DVars::D3D_window, multisample_types[i]);

		if(FAILED(hr)) 
		{
			DBUGFILE_OUTPUT_1("Failed aa %d",i);
			continue;
		}

		// Success!
		DBUGFILE_OUTPUT_1("Success aa %d",i);
		return i;
	}
	
	// Nothing found
	DBUGFILE_OUTPUT_0("Terrible news, we didnt find a match");
    return -1;
}

void d3d_compare_adapter(int adapter_choice)
{
	HRESULT hrd, hrc;
	D3DADAPTER_IDENTIFIER8 info_default, info_chosen;
	hrd = GlobalD3DVars::lpD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_NO_WHQL_LEVEL, &info_default);
	hrc = GlobalD3DVars::lpD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_NO_WHQL_LEVEL, &info_chosen);

	if(FAILED(hrd) || FAILED(hrc))
	{
		DBUGFILE_OUTPUT_2("Failed to identify default %d, chosen %d",FAILED(hrd), FAILED(hrc));
		return;
	}

	D3DADAPTER_IDENTIFIER8 *info = &info_default;
	DBUGFILE_OUTPUT_0("Default:");
	DBUGFILE_OUTPUT_2("%s %s",info->Driver, info->Description);

	info = &info_chosen;
	DBUGFILE_OUTPUT_0("Chosen:");
	DBUGFILE_OUTPUT_2("%s %s",info->Driver, info->Description);
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
	int aatype_choice = 0;
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
		GlobalD3DVars::d3dpp.BackBufferWidth  = (UINT)gr_screen.max_w;
		GlobalD3DVars::d3dpp.BackBufferHeight = (UINT)gr_screen.max_h;

		GlobalD3DVars::d3dpp.FullScreen_RefreshRateInHz      = 0;
		GlobalD3DVars::d3dpp.FullScreen_PresentationInterval = 0;

		GlobalD3DVars::d3dpp.Windowed		   = TRUE;
	} else {

		// Attempt to get options from the registry
		adapter_choice     = os_config_read_uint( NULL, "D3D8_Adapter", 0xffff);
		aatype_choice	   = os_config_read_uint( NULL, "D3D8_AAType",  0x0);
		int mode_choice	   = d3d_match_mode(adapter_choice);
		
		if(mode_choice == -1)
		{
			strcpy(Device_init_error, "Couldnt match mode");
			return false;
		}

		// Sanity cap
		if(aatype_choice < 0 || aatype_choice >= MULTISAMPLE_MAX)
		{
			aatype_choice = 0;
		}

		// Should only activate if a value in the registry is not set or its going to run in a window or
		// a optional parameter forces it to run. Otherwise the mode values are taken from reg value
		if( adapter_choice == 0xffff)
		{
			strcpy(Device_init_error, "DX8 options not set, please run launcher");
			return false;
		}

		d3d_compare_adapter(adapter_choice);

		if(FAILED(GlobalD3DVars::lpD3D->EnumAdapterModes(adapter_choice, mode_choice, &mode)))
		{
			sprintf(Device_init_error, "Could not use selected mode: %d", mode_choice);
			return false;
		}

		GlobalD3DVars::D3D_Antialiasing = (aatype_choice != 0);

		GlobalD3DVars::d3dpp.MultiSampleType  = multisample_types[aatype_choice];
		GlobalD3DVars::d3dpp.BackBufferWidth  = mode.Width;
		GlobalD3DVars::d3dpp.BackBufferHeight = mode.Height;

		// Calculate texture pullback values
		if(gr_screen.custom_size != -1)
		{
			GlobalD3DVars::texture_adjust_u = 0.0044f;
			GlobalD3DVars::texture_adjust_v = 0.0044f;
			gr_unsize_screen_posf(&GlobalD3DVars::texture_adjust_u, &GlobalD3DVars::texture_adjust_v);
		}

		GlobalD3DVars::d3dpp.FullScreen_RefreshRateInHz      = D3DPRESENT_RATE_DEFAULT;
		GlobalD3DVars::d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

		if(Cmdline_no_vsync && got_caps && GlobalD3DVars::d3d_caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) {
			GlobalD3DVars::d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		GlobalD3DVars::d3dpp.Windowed		  = FALSE;
	}

	D3DADAPTER_IDENTIFIER8 adapter_info;
	if(SUCCEEDED(GlobalD3DVars::lpD3D->GetAdapterIdentifier(adapter_choice, D3DENUM_NO_WHQL_LEVEL, &adapter_info)))
	{
		mprintf(("  Direct3D Adapter        : %s\n", adapter_info.Description));
		mprintf(("  Direct3D Driver Version : %i.%i.%i.%i\n", HIWORD(adapter_info.DriverVersion.HighPart), LOWORD(adapter_info.DriverVersion.HighPart),
				 HIWORD(adapter_info.DriverVersion.LowPart), LOWORD(adapter_info.DriverVersion.LowPart)));
		mprintf(("\n"));
	}

	GlobalD3DVars::d3dpp.BackBufferFormat = mode.Format;

	//trying to use a higher bit depth in the back buffer, the deepest one posale -Bobboau
  	const int NUM_FORMATS = 3;
  //	const int NUM_FORMATS = 6;

	enum _D3DFORMAT format_type[NUM_FORMATS] = 
		{D3DFMT_D24X8, D3DFMT_D32, D3DFMT_D16};
	 //	{D3DFMT_D32, D3DFMT_D24X4S4, D3DFMT_D24S8, D3DFMT_D24X8, D3DFMT_D16, D3DFMT_D15S1};

	const int NUM_VPTYPES = 3;
	DWORD vptypes[NUM_VPTYPES] = {
		D3DCREATE_HARDWARE_VERTEXPROCESSING, 
			D3DCREATE_MIXED_VERTEXPROCESSING, 
			D3DCREATE_SOFTWARE_VERTEXPROCESSING};

	// Use a pure device if we can, it really speeds things up
	// However it means we cant use Get* functions (aside from GetFrontBuffer, GetCaps, etc)
	if(got_caps && GlobalD3DVars::d3d_caps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
	  	vptypes[0] |= D3DCREATE_PUREDEVICE;
		mprintf(("  Using PURE D3D Device\n"));
	}

	bool have_device = false;
	for(int vp = 0;	vp < NUM_VPTYPES; vp++)					 
	{
		// It trys to use the highest suported back buffer through trial and error, 
		// I wraped the existing code in a for loop to cycle through the diferent formats -Bobboau
		for(int t = 0; t < NUM_FORMATS; t++){
		
			GlobalD3DVars::d3dpp.AutoDepthStencilFormat = format_type[t];

			int aaresult = d3d_check_multisample_types(
				adapter_choice, aatype_choice, 
				GlobalD3DVars::d3dpp.BackBufferFormat, 
				GlobalD3DVars::d3dpp.AutoDepthStencilFormat);

			if(aaresult == -1)
				continue;

			GlobalD3DVars::d3dpp.MultiSampleType = multisample_types[aaresult];

		
			if( SUCCEEDED( GlobalD3DVars::lpD3D->CreateDevice(
									adapter_choice, 
									D3DDEVTYPE_HAL, 
									(HWND) os_get_window(),
									vptypes[vp],
		                            &GlobalD3DVars::d3dpp, 
									&GlobalD3DVars::lpD3DDevice) ) ) {
				have_device = true;
				DBUGFILE_OUTPUT_2("Depth buffer format %d, aaresult %d",t,aaresult);
				break;
			}
		}

		if(have_device == true) {
			break;
		}
	}

	// This is bad, the mode hasnt initialised
  	if (have_device == false) 
	{

		sprintf(Device_init_error, "Failed to get init mode");
	
		// Lets draw attension to this bug which we dont yet understand
		if(got_caps == false) 
		{
			sprintf(Device_init_error, 
				"Failed to get init mode AND GETCAPS FAILED\n"
				"Please contact Random Tiger about this problem directly\n"
				"tlwhittaker2000@hotmail.com");
		}

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
		if(gr_screen.bits_per_pixel == 32){
			d3d_detect_texture_origin_32();
		} else {
			d3d_detect_texture_origin_16();
		}
	}

	mprintf(("  D3d_rendition_uvs: %d",GlobalD3DVars::D3D_rendition_uvs));

	// Not sure now relevent this is now
	tmp = os_config_read_uint( NULL, "D3DLineOffset", 0xFFFF );

	extern float D3D_line_offset;
	if ( tmp != 0xFFFF )	{
		D3D_line_offset = ( tmp ) ? 0.5f : 0.0f;
	} else {
		if(gr_screen.bits_per_pixel == 32){
			d3d_detect_line_offset_32();
		} else {
			d3d_detect_line_offset_16();
		}
	}
#endif
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

	mprintf(( "  Alpha texture format = " ));
	d3d_dump_format(&AlphaTextureFormat);

	mprintf(( "  Non-alpha texture format = " ));
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


/*
enum stage_state{
	NONE = -1, 
	INITIAL = 0, 
	DEFUSE = 1, 
	GLOW_MAPPED_DEFUSE = 2, 
	NONMAPPED_SPECULAR = 3, 
	GLOWMAPPED_NONMAPPED_SPECULAR = 4, 
	MAPPED_SPECULAR = 5, 
	CELL = 6, 
	GLOWMAPPED_CELL = 7, 
	ADDITIVE_GLOWMAPPING = 8, 
	SINGLE_PASS_SPECMAPPING = 9, 
	SINGLE_PASS_GLOW_SPEC_MAPPING = 10,
	BACKGROUND_FOG = 11,
	ENV = 12
	};
*/
/*
void d3d_set_initial_render_state(bool set = true)	;
void set_stage_for_defuse(bool set = true);
void set_stage_for_glow_mapped_defuse(bool set = true);
void set_stage_for_defuse_and_non_mapped_spec(bool set = true);
void set_stage_for_glow_mapped_defuse_and_non_mapped_spec(bool set = true);
bool set_stage_for_spec_mapped(bool set = true);
void set_stage_for_cell_shaded(bool set = true);
void set_stage_for_cell_glowmapped_shaded(bool set = true);
void set_stage_for_additive_glowmapped(bool set = true);
void set_stage_for_background_fog(bool set = true);
bool set_stage_for_env_mapped(bool set = true);
void set_stage_for_single_pass_specmapping(int SAME, bool set = true);
void set_stage_for_single_pass_glow_specmapping(int SAME, bool set = true);
*/

DWORD 
initial_state_block, 
defuse_state_block, 
glow_mapped_defuse_state_block, 
nonmapped_specular_state_block, 
glow_mapped_nonmapped_specular_state_block, 
mapped_specular_state_block,
cell_state_block, 
glow_mapped_cell_state_block, 
additive_glow_mapping_state_block, 
//single_pass_specmapping_state_block, 
//single_pass_glow_spec_mapping_state_block, 
background_fog_state_block, 
env_state_block, 
cloak_state_block;

void d3d_generate_state_blocks(){
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &initial_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &defuse_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &glow_mapped_defuse_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &nonmapped_specular_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &glow_mapped_nonmapped_specular_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &mapped_specular_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &cell_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &glow_mapped_cell_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &additive_glow_mapping_state_block);
//	CreateStateBlock(D3DSBT_PIXELSTATE, &single_pass_specmapping_state_block);
//	CreateStateBlock(D3DSBT_PIXELSTATE, &single_pass_glow_spec_mapping_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &background_fog_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &env_state_block);
	GlobalD3DVars::lpD3DDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &cloak_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	d3d_set_initial_render_state(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&initial_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_defuse(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&defuse_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_glow_mapped_defuse(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&glow_mapped_defuse_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_defuse_and_non_mapped_spec(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&nonmapped_specular_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_glow_mapped_defuse_and_non_mapped_spec(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&glow_mapped_nonmapped_specular_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_spec_mapped(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&mapped_specular_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_cell_shaded(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&glow_mapped_cell_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_cell_glowmapped_shaded(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&glow_mapped_cell_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_additive_glowmapped(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&additive_glow_mapping_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_background_fog(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&background_fog_state_block);

	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_env_mapped(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&env_state_block);

/*	GlobalD3DVars::lpD3DDevice->BeginStateBlock();
	set_stage_for_cloaked(true);
	GlobalD3DVars::lpD3DDevice->EndStateBlock(&cloak_state_block);
*/
}

void d3d_kill_state_blocks(){
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( initial_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( defuse_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( glow_mapped_defuse_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( nonmapped_specular_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( glow_mapped_nonmapped_specular_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( mapped_specular_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( cell_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( glow_mapped_cell_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( additive_glow_mapping_state_block);
//	DeleteStateBlock( &single_pass_specmapping_state_block);
//	DeleteStateBlock( &single_pass_glow_spec_mapping_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( background_fog_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( env_state_block);
	GlobalD3DVars::lpD3DDevice->DeleteStateBlock( cloak_state_block);
}

//void d3d_init_environment();

bool gr_d3d_init()
{
	mprintf(( "Initializing Direct3D graphics device at %ix%i with %i-bit color...\n", gr_screen.max_w, gr_screen.max_h, gr_screen.bits_per_pixel ));

	GlobalD3DVars::lpD3D = Direct3DCreate8( D3D_SDK_VERSION );

	if( GlobalD3DVars::lpD3D == NULL ) {
		sprintf(Device_init_error, "Please make sure you have DX8.1b installed");
		return false;
	}

//	ShowCursor(false);

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

	if(d3d_init_light() == false) {
		// Func will give its own errors
		sprintf(Device_init_error, "Failed to setup lighting");
		return false;
	}

	d3d_setup_color();

	// Tell FreeSpace code that we're using Direct3D.
	GlobalD3DVars::D3D_inited = true;
	
	d3d_generate_state_blocks();

  	d3d_tcache_init();

	d3d_setup_function_pointers();
	d3d_init_vars_from_reg();

	d3d_reset_render_states();
	d3d_reset_texture_stage_states();

	d3d_set_initial_render_state();

	gr_d3d_activate(1);

	D3DXCreateMatrixStack(0, &world_matrix_stack);
	D3DXCreateMatrixStack(0, &view_matrix_stack);
	D3DXCreateMatrixStack(0, &proj_matrix_stack);

	d3d_start_frame();

	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "  Max texture units: %i\n", GlobalD3DVars::d3d_caps.MaxSimultaneousTextures ));
	mprintf(( "  Max texture size: %ix%i\n", GlobalD3DVars::d3d_caps.MaxTextureWidth, GlobalD3DVars::d3d_caps.MaxTextureHeight ));
	mprintf(( "  Can use compressed textures: %s\n", Use_compressed_textures ? NOX("YES") : NOX("NO") ));
	mprintf(( "  Texture compression available: %s\n", Texture_compression_available ? NOX("YES") : NOX("NO") ));

	TIMERBAR_SET_DRAW_FUNC(d3d_render_timer_bar);

//	d3d_init_environment();

	mprintf(("... Direct3D init is complete!\n"));

	return true;

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

#endif // !NO_DIRECT3D

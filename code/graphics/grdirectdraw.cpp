/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrDirectDraw.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:22 $
 * $Author: penguin $
 *
 * Code for software 8-bpp rendering using DirectDraw
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 9     7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 8     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 7     2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 6     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 5     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 4     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 17    5/22/98 10:28p John
 * Made software movies not click your monitor when switching to 16-bpp.
 * Total hack around my restrictive code, but at this point...
 * 
 * 16    5/20/98 9:45p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 15    5/17/98 5:03p John
 * Fixed some bugs that make the taskbar interfere with the DEBUG-only
 * mouse cursor.
 * 
 * 14    5/16/98 1:18p John
 * Made softtware DirectDraw reset palette after Alt+TAB.
 * 
 * 13    5/15/98 9:34p John
 * Removed the initial ugly little cursor part that drew right at program
 * start.
 * 
 * 12    5/14/98 5:42p John
 * Revamped the whole window position/mouse code for the graphics windows.
 * 
 * 11    5/12/98 8:15a John
 * Made dd code not print out rgb surface info.
 * 
 * 10    5/07/98 6:58p Hoffoss
 * Made changes to mouse code to fix a number of problems.
 * 
 * 9     5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 8     4/23/98 8:24a John
 * Changed the way palette effect works so that:
 * 1. If gr_flash isn't called this frame, screen shows no flash.
 * 2. With hardware, only 3d portion of screen gets flashed.
 * 
 * 7     4/21/98 5:22p John
 * Fixed all the &^#@$ cursor bugs with popups.   For Glide, had problem
 * with mouse restoring assuming back buffer was same buffer last frame,
 * for software, problems with half drawn new frames, then mouse got
 * restored on top of that with old data.
 * 
 * 6     4/17/98 3:56p Mike
 * Fix palette trashing on MK's computer when he Alt-Tabs out of FreeSpace
 * in software.  At John's suggestion.
 * 
 * 5     4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 4     4/13/98 8:08p John
 * Made surface restoration also restore the palette.
 * 
 * 3     4/09/98 6:56p John
 * Put in better code to restore surfaces when restoring them.  Worth a
 * try for an Interplay QA bug I can't find.
 * 
 * 2     4/09/98 11:05a John
 * Removed all traces of Direct3D out of the demo version of Freespace and
 * the launcher.
 * 
 * 1     3/25/98 8:07p John
 * Split software renderer into Win32 and DirectX
 *
 * $NoKeywords: $
 */

#include <math.h>
#include <windows.h>
#include <windowsx.h>

#include "vddraw.h"

#include "osapi.h"
#include "2d.h"
#include "bmpman.h"
#include "key.h"
#include "floating.h"
#include "palman.h"
#include "grsoft.h"
#include "grinternal.h"

// Headers for 2d functions
#include "pixel.h"
#include "line.h"
#include "scaler.h"
#include "tmapper.h"
#include "circle.h"
#include "shade.h"
#include "rect.h"
#include "gradient.h"
#include "pcxutils.h"
#include "osapi.h"
#include "mouse.h"
#include "font.h"
#include "timer.h"
#include "colors.h"
#include "bitblt.h"
#include "grzbuffer.h"

static LPDIRECTDRAW			lpDD = NULL;
static LPDIRECTDRAWSURFACE	lpBackBuffer = NULL;
static LPDIRECTDRAWSURFACE	lpFrontBuffer = NULL;
static LPDIRECTDRAWCLIPPER	lpClipper = NULL;
static LPDIRECTDRAWPALETTE	lpPalette = NULL;

void gr_directdraw_unlock();
void gr_directdraw_set_palette_internal( ubyte * new_pal );

void directdraw_clear_all_vram()
{
#ifndef HARDWARE_ONLY
	DDSURFACEDESC ddsd;
	HRESULT ddrval;
	LPDIRECTDRAWSURFACE buffer[16];
	
	int i;
	int num_buffers = 0;
	for (i=0; i<16; i++ )	{
		memset( &ddsd, 0, sizeof( ddsd ) );

		ddsd.dwSize  = sizeof( ddsd );
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = gr_screen.max_w;
		ddsd.dwHeight = gr_screen.max_h;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

		// create the back buffer
		ddrval = lpDD->CreateSurface( &ddsd, &buffer[i], NULL );
		if ( ddrval == DD_OK )	{
			// Clear the surface
			DDBLTFX ddBltFx;

			memset(&ddBltFx, 0, sizeof(DDBLTFX));
			ddBltFx.dwSize = sizeof(DDBLTFX);
			ddBltFx.dwFillColor = 0;

			buffer[i]->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddBltFx);

		} else {
			num_buffers = i;
			buffer[i] = NULL;
			break;
		}
	}

	for (i=0; i<num_buffers; i++ )	{
		buffer[i]->Release();
	}
#else
	Int3();
#endif
}

LPDIRECTDRAW gr_directdraw_start_mve()
{
#ifndef HARDWARE_ONLY
	ubyte tmp_pal[768];
	memset( tmp_pal, 0, 768 );

	gr_directdraw_set_palette_internal(tmp_pal);

	// clear both buffers
	gr_reset_clip();
	gr_clear();
	gr_flip();

	gr_reset_clip();
	gr_clear();
	gr_flip();

	gr_directdraw_unlock();

	// Clear all of VRAM...
	directdraw_clear_all_vram();

	if ( lpPalette )	{
		lpPalette->Release();
		lpPalette = NULL;
	}

	if (lpFrontBuffer)	{
		lpFrontBuffer->Release();
		lpFrontBuffer = NULL;
	}

	
	return lpDD;
#else
	Int3();
	return 0;
#endif
}


void gr_directdraw_stop_mve()
{
#ifndef HARDWARE_ONLY
	DDSURFACEDESC ddsd;
	HRESULT ddrval;

	memset( &ddsd, 0, sizeof( ddsd ) );

	ddsd.dwSize  = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	ddrval = lpDD->CreateSurface( &ddsd, &lpFrontBuffer, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: CreateSurface (Front) failed.\n" ));
		lpDD->Release();
		exit(1);
	}

	int i;
	PALETTEENTRY	pe[256];
	for (i=0; i<256; i++ )	{
		pe[i].peFlags = PC_NOCOLLAPSE|PC_RESERVED;
		pe[i].peRed = gr_palette[i*3+0];
		pe[i].peGreen = gr_palette[i*3+1];
		pe[i].peBlue = gr_palette[i*3+2];
	}	

	ddrval = lpDD->CreatePalette( DDPCAPS_8BIT | DDPCAPS_ALLOW256, pe, &lpPalette, NULL);
	if (ddrval != DD_OK) {
		mprintf(( "Error creating palette\n" ));
	}

	ddrval = lpFrontBuffer->SetPalette( lpPalette );
	if (ddrval != DD_OK) {
		mprintf(( "Error setting palette in gr_directdraw_set_palette\n" ));
	}

	gr_reset_clip();
	gr_clear();
	gr_flip();
#else 
	Int3();
#endif
}

//#define DD_FLIP 1

static int Locked = 0;

uint gr_directdraw_lock()
{
#ifndef HARDWARE_ONLY
	if ( !Locked )	{

		HRESULT ddrval;
		DDSURFACEDESC ddsd;

		memset( &ddsd, 0, sizeof( ddsd ) );
		ddsd.dwSize = sizeof( ddsd );

		ddrval = lpBackBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
		if ( ddrval != DD_OK )	{
			mprintf(( "Error locking surface in gr_d3d_lock\n" ));
			gr_screen.offscreen_buffer_base = NULL;	//(void *)fake_vram;
			gr_screen.rowsize = 0;
			gr_screen.offscreen_buffer = gr_screen.offscreen_buffer_base;
		} else {
			gr_screen.offscreen_buffer_base = (void *)ddsd.lpSurface;
			gr_screen.rowsize = ddsd.lPitch;
			gr_screen.offscreen_buffer = gr_screen.offscreen_buffer_base;
			Locked = 1;
		}

	}
	return 1;
#else 
	Int3();
	return 0;
#endif
}

void gr_directdraw_unlock()
{
#ifndef HARDWARE_ONLY
	if ( Locked )	{
		// Unlock the back buffer
		lpBackBuffer->Unlock( NULL );
		Locked = 0;
	}
#else
	Int3();
#endif
}

void gr_directdraw_unlock_fake()
{
}


static int Palette_flashed = 0;
static int Palette_flashed_last_frame = 0;


static volatile int Gr_dd_activated = 0;

void gr_dd_activate(int active)
{
#ifndef HARDWARE_ONLY
	if ( active  )	{
		Gr_dd_activated++;
	}
#else
	Int3();
#endif
}


void gr_directdraw_flip()
{
#ifndef HARDWARE_ONLY
	if ( Gr_dd_activated || ((!Palette_flashed) && (Palette_flashed_last_frame)) )	{
		// Reset flash
		gr_directdraw_set_palette_internal( gr_palette );
		Gr_dd_activated = 0;
	}

	Palette_flashed_last_frame = Palette_flashed;
	Palette_flashed = 0;

	gr_reset_clip();

	int mx, my;

	Grx_mouse_saved = 0;		// assume not saved

	mouse_eval_deltas();
	if ( mouse_is_visible() )	{
		gr_reset_clip();
		mouse_get_pos( &mx, &my );
		grx_save_mouse_area(mx,my,32,32);
		if ( Gr_cursor == -1 )	{
			gr_set_color(255,255,255);
			gr_line( mx, my, mx+7, my + 7 );
			gr_line( mx, my, mx+5, my );
			gr_line( mx, my, mx, my+5 );
		} else {
			gr_set_bitmap(Gr_cursor);
			gr_bitmap( mx, my );
		}
	} 

	gr_directdraw_unlock();

#ifdef DD_FLIP
TryFlipAgain:

	if ( lpFrontBuffer->IsLost() == DDERR_SURFACELOST )	{
		lpFrontBuffer->Restore();
		ddrval = lpFrontBuffer->SetPalette( lpPalette );
		if (ddrval != DD_OK) {
			mprintf(( "Error setting palette after restore\n" ));
		}
	}
	if ( lpBackBuffer->IsLost() == DDERR_SURFACELOST )	{
		lpBackBuffer->Restore();
	}

	HRESULT ddrval = lpFrontBuffer->Flip( NULL, DDFLIP_WAIT  );
	if ( ddrval == DDERR_SURFACELOST )	{
		mprintf(( "Front surface lost... attempting to restore...\n" ));
		os_sleep(1000);	// Wait a second
		goto TryFlipAgain;
	} else if (ddrval != DD_OK )	{
		mprintf(( "Fullscreen flip failed!\n" ));
	}

#else
	RECT src_rect, dst_rect;
	POINT pt;
	HRESULT ddrval;

	HWND hWnd = (HWND)os_get_window();

	if ( hWnd )	{
		// src_rect is relative to offscreen buffer
		GetClientRect( hWnd, &src_rect );

		// dst_rect is relative to screen space so needs translation
		pt.x = pt.y = 0;
		ClientToScreen( hWnd, &pt );
		dst_rect = src_rect;
		dst_rect.left += pt.x;
		dst_rect.right += pt.x;
		dst_rect.top += pt.y;
		dst_rect.bottom += pt.y;

		// perform the blit from backbuffer to primary, using
		// src_rect and dst_rect

TryFlipAgain:
		if ( lpFrontBuffer->IsLost() == DDERR_SURFACELOST )	{
			lpFrontBuffer->Restore();

/*			ddrval = lpFrontBuffer->SetPalette( lpPalette );
			if (ddrval != DD_OK) {
				mprintf(( "Error setting palette after restore\n" ));
			}
*/		}
		if ( lpBackBuffer->IsLost() == DDERR_SURFACELOST )	{
			lpBackBuffer->Restore();
		}
		ddrval = lpFrontBuffer->Blt( &dst_rect, lpBackBuffer, &src_rect, DDBLT_WAIT, 0 );
		if ( ddrval == DDERR_SURFACELOST )	{
			mprintf(( "Front surface lost... attempting to restore...\n" ));
			os_sleep(1000);	// Wait a second
			goto TryFlipAgain;
		} else if (ddrval != DD_OK )	{
			mprintf(( "Flip failed! $%x\n", ddrval ));
		}
	}
#endif

	if ( Grx_mouse_saved )	{
		grx_restore_mouse_area();
	}
#else
	Int3();
#endif
}

void gr_directdraw_flip_window(uint _hdc, int x, int y, int w, int h )
{
}

void gr_directdraw_start_frame()
{

}

void gr_directdraw_stop_frame()
{

}

void gr_directdraw_set_palette_internal( ubyte * new_pal )
{
#ifndef HARDWARE_ONLY
	PALETTEENTRY	pe[256];

	gr_directdraw_unlock();

	int i;
	for (i=0; i<256; i++ )	{
		pe[i].peFlags = PC_NOCOLLAPSE|PC_RESERVED;
		pe[i].peRed = new_pal[i*3+0];
		pe[i].peGreen = new_pal[i*3+1];
		pe[i].peBlue = new_pal[i*3+2];
	}	

	HRESULT ddrval;
	ddrval = lpPalette->SetEntries( 0, 0, 256, pe );
	if ( ddrval != DD_OK )	{
		mprintf(( "Error setting palette\n" ));
	}

	gr_lock();
#else
	Int3();
#endif
}

void gr_directdraw_set_palette( ubyte * new_pal, int restrict_alphacolor )
{
#ifndef HARDWARE_ONLY
	if ( !restrict_alphacolor )	{
		Mouse_hidden++;
		gr_reset_clip();
		gr_clear();
		gr_flip();
		Mouse_hidden--;
	}

	// Make sure color 0 is black
	if ( (new_pal[0]!=0) || (new_pal[1]!=0) || (new_pal[2]!=0) )	{
		// color 0 isn't black!! switch it!
		int i;
		int black_index = -1;

		for (i=1; i<256; i++ )	{
			if ( (new_pal[i*3+0]==0) && (new_pal[i*3+1]==0) && (new_pal[i*3+2]==0) )	{	
				black_index = i;
				break;
			}
		}
		if ( black_index > -1 )	{
			// swap black and color 0, so color 0 is black
			ubyte tmp[3];
			tmp[0] = new_pal[black_index*3+0];
			tmp[1] = new_pal[black_index*3+1];
			tmp[2] = new_pal[black_index*3+2];

			new_pal[black_index*3+0] = new_pal[0];
			new_pal[black_index*3+1] = new_pal[1];
			new_pal[black_index*3+2] = new_pal[2];

			new_pal[0] = tmp[0];
			new_pal[1] = tmp[1];
			new_pal[2] = tmp[2];
		} else {
			// no black in palette, force color 0 to be black.
			new_pal[0] = 0;
			new_pal[1] = 0;
			new_pal[2] = 0;
		}
	}

	gr_directdraw_set_palette_internal( new_pal );
#else
	Int3();
#endif
}

void gr_directdraw_flash( int r, int g, int b )
{
#ifndef HARDWARE_ONLY
	int t,i;
	ubyte new_pal[768];

	if ( (r==0) && (b==0) && (g==0) )	{
		return;
	}

	Palette_flashed++;

	for (i=0; i<256; i++ )	{
		t = gr_palette[i*3+0] + r;
		if ( t < 0 ) t = 0; else if (t>255) t = 255;
		new_pal[i*3+0] = (ubyte)t;

		t = gr_palette[i*3+1] + g;
		if ( t < 0 ) t = 0; else if (t>255) t = 255;
		new_pal[i*3+1] = (ubyte)t;

		t = gr_palette[i*3+2] + b;
		if ( t < 0 ) t = 0; else if (t>255) t = 255;
		new_pal[i*3+2] = (ubyte)t;
	}

	gr_directdraw_set_palette_internal( new_pal );
#else
	Int3();
#endif
}


static int gr_palette_faded_out = 0;

#define FADE_TIME (F1_0/4)		// How long to fade out

void gr_directdraw_fade_out(int instantaneous)	
{
#ifndef HARDWARE_ONLY
	int i;
	ubyte new_pal[768];

	if (!gr_palette_faded_out) {

		if ( !instantaneous )	{
	
			int count = 0;
			fix start_time, stop_time, t1;

			start_time = timer_get_fixed_seconds();
			t1 = 0;

			do {
				for (i=0; i<768; i++ )	{		
					int c = (gr_palette[i]*(FADE_TIME-t1))/FADE_TIME;
					if (c < 0 )
						c = 0;
					else if ( c > 255 )
						c = 255;
			
					new_pal[i] = (ubyte)c;
				}
				gr_directdraw_set_palette_internal( new_pal );
				count++;

				t1 = timer_get_fixed_seconds() - start_time;

			} while ( (t1 < FADE_TIME) && (t1>=0) );		// Loop as long as time not up and timer hasn't rolled

			stop_time = timer_get_fixed_seconds();

			mprintf(( "Took %d frames (and %.1f secs) to fade out\n", count, f2fl(stop_time-start_time) ));
		
		}
		gr_palette_faded_out = 1;
	}

	memset( new_pal, 0, 768 );
	gr_directdraw_set_palette_internal( new_pal );
#else
	Int3();
#endif
}


void gr_directdraw_fade_in(int instantaneous)	
{
#ifndef HARDWARE_ONLY
	int i;
	ubyte new_pal[768];

	if (gr_palette_faded_out)	{

		if ( !instantaneous )	{
			int count = 0;
			fix start_time, stop_time, t1;

			start_time = timer_get_fixed_seconds();
			t1 = 0;

			do {
				for (i=0; i<768; i++ )	{		
					int c = (gr_palette[i]*t1)/FADE_TIME;
					if (c < 0 )
						c = 0;
					else if ( c > 255 )
						c = 255;
			
					new_pal[i] = (ubyte)c;
				}
				gr_directdraw_set_palette_internal( new_pal );
				count++;

				t1 = timer_get_fixed_seconds() - start_time;

			} while ( (t1 < FADE_TIME) && (t1>=0) );		// Loop as long as time not up and timer hasn't rolled

			stop_time = timer_get_fixed_seconds();

			mprintf(( "Took %d frames (and %.1f secs) to fade in\n", count, f2fl(stop_time-start_time) ));
		}
		gr_palette_faded_out = 0;
	}

	memcpy( new_pal, gr_palette, 768 );
	gr_directdraw_set_palette_internal( new_pal );
#else
	Int3();
#endif
}

void gr_directdraw_get_region(int front, int w, int h, ubyte *data)
{
}

// sets the clipping region & offset
void gr_directdraw_set_clip(int x,int y,int w,int h)
{
#ifndef HARDWARE_ONLY
	gr_screen.offset_x = x;
	gr_screen.offset_y = y;

	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;

	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;

	// check for sanity of parameters
	if ( gr_screen.clip_left+x < 0 ) {
		gr_screen.clip_left = -x;
	} else if ( gr_screen.clip_left+x > gr_screen.max_w-1 )	{
		gr_screen.clip_left = gr_screen.max_w-1-x;
	}
	if ( gr_screen.clip_right+x < 0 ) {
		gr_screen.clip_right = -x;
	} else if ( gr_screen.clip_right+x >= gr_screen.max_w-1 )	{
		gr_screen.clip_right = gr_screen.max_w-1-x;
	}

	if ( gr_screen.clip_top+y < 0 ) {
		gr_screen.clip_top = -y;
	} else if ( gr_screen.clip_top+y > gr_screen.max_h-1 )	{
		gr_screen.clip_top = gr_screen.max_h-1-y;
	}

	if ( gr_screen.clip_bottom+y < 0 ) {
		gr_screen.clip_bottom = -y;
	} else if ( gr_screen.clip_bottom+y > gr_screen.max_h-1 )	{
		gr_screen.clip_bottom = gr_screen.max_h-1-y;
	}

	gr_screen.clip_width = gr_screen.clip_right - gr_screen.clip_left + 1;
	gr_screen.clip_height = gr_screen.clip_bottom - gr_screen.clip_top + 1;
#else
	Int3();
#endif
}

// resolution checking
int gr_directdraw_supports_res_ingame(int res)
{
	return 1;
}

int gr_directdraw_supports_res_interface(int res)
{
	return 1;
}

void gr_directdraw_set_cull(int cull)
{
}

// resets the clipping region to entire screen
//
// should call this before gr_surface_flip() if you clipped some portion of the screen but still 
// want a full-screen display
void gr_directdraw_reset_clip()
{
#ifndef HARDWARE_ONLY
	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;
#else
	Int3();
#endif
}


// Sets the current bitmap
void gr_directdraw_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
#ifndef HARDWARE_ONLY
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
#else
	Int3();
#endif
}

// clears entire clipping region to black.
void gr_directdraw_clear()
{
#ifndef HARDWARE_ONLY

#ifdef DD_FLIP
	gr_directdraw_unlock();

	DDBLTFX ddBltFx;

	ddBltFx.dwSize = sizeof(DDBLTFX);
	ddBltFx.dwFillColor = 0;

	//DDBLT_WAIT | 
	lpBackBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddBltFx);
//		ddBltFx.dwROP = BLACKNESS;
//		lpBackBuffer->Blt(NULL, NULL, NULL, DDBLT_ROP, &ddBltFx);

	gr_lock();

#else
	gr_lock();

	int i,w;
	ubyte *pDestBits;

	w = gr_screen.clip_right-gr_screen.clip_left+1;
	for (i=gr_screen.clip_top; i<=gr_screen.clip_bottom; i++)	{
		pDestBits = GR_SCREEN_PTR(ubyte,gr_screen.clip_left,i);
		memset( pDestBits, 0, w );
	}	

	gr_directdraw_unlock();
#endif
#else
	Int3();
#endif
}

void gr_directdraw_force_windowed()
{
#ifndef HARDWARE_ONLY
	gr_directdraw_unlock();

	HWND hwnd = (HWND)os_get_window();

	// Simulate Alt+Tab
	PostMessage(hwnd,WM_SYSKEYUP, 0x9, 0xa00f0001 );
	PostMessage(hwnd,WM_SYSKEYUP, 0x12, 0xc0380001 );

	// Wait a second to give things a change to settle down.				
	Sleep(1000);
#else
	Int3();
#endif
}

void gr_directdraw_cleanup()
{
#ifndef HARDWARE_ONLY
	HWND hwnd = (HWND)os_get_window();
	
	gr_directdraw_unlock();

	if ( lpPalette )	{
		lpPalette->Release();
		lpPalette = NULL;
	}

	if (lpClipper)	{
		lpClipper->Release();
		lpClipper = NULL;
	}

	if (lpBackBuffer)	{
		lpBackBuffer->Release();
		lpBackBuffer = NULL;
	}

	if (lpFrontBuffer)	{
		lpFrontBuffer->Release();
		lpFrontBuffer = NULL;
	}


// JAS:  I took this out because my taskbar seems to get screwed up
// if I leave this in.   Doesn't make sense, hence this comment.
	HRESULT ddrval = lpDD->RestoreDisplayMode();
	if( ddrval != DD_OK )	{
		mprintf(( "WIN_DD32: RestoreDisplayMode failed (0x%x)\n", ddrval ));
	}
	ddrval = lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
	if( ddrval != DD_OK )	{
		mprintf(( "WIN_DD32: SetCooperativeLevel W Failed (0x%x)\n", ddrval ));
	}

	os_suspend();
	if ( lpDD ) {
		lpDD->Release();
		lpDD = NULL; 
	}
	os_resume();
#else
	Int3();
#endif
}

void dd_get_shift_masks( DDSURFACEDESC *ddsd )
{
	unsigned long m;
	int s;
	int r, red_shift, red_scale, red_mask;
	int g, green_shift, green_scale, green_mask;
	int b, blue_shift, blue_scale, blue_mask;
	int a, alpha_shift, alpha_scale, alpha_mask;
	
	if (gr_screen.bits_per_pixel == 8 ) {
		Gr_red.bits = 8;
		Gr_red.shift = 16;
		Gr_red.scale = 1;
		Gr_red.mask = 0xff0000;

		Gr_green.bits = 8;
		Gr_green.shift = 8;
		Gr_green.scale = 1;
		Gr_green.mask = 0xff00;

		Gr_blue.bits = 8;
		Gr_blue.shift = 0;
		Gr_blue.scale = 1;
		Gr_blue.mask = 0xff;
		return;
	}

	// Determine the red, green and blue masks' shift and scale.
	for (s = 0, m = ddsd->ddpfPixelFormat.dwRBitMask; !(m & 1); s++, m >>= 1);
	for (r = 0; m & 1; r++, m >>= 1);
	red_shift = s;
	red_scale = 255 / (ddsd->ddpfPixelFormat.dwRBitMask >> s);
	red_mask = ddsd->ddpfPixelFormat.dwRBitMask;

	for (s = 0, m = ddsd->ddpfPixelFormat.dwGBitMask; !(m & 1); s++, m >>= 1);
	for (g = 0; m & 1; g++, m >>= 1);
	green_shift = s;
	green_scale = 255 / (ddsd->ddpfPixelFormat.dwGBitMask >> s);
	green_mask = ddsd->ddpfPixelFormat.dwGBitMask;
	
	for (s = 0, m = ddsd->ddpfPixelFormat.dwBBitMask; !(m & 1); s++, m >>= 1);
	for (b = 0; m & 1; b++, m >>= 1);
	blue_shift = s;
	blue_scale = 255 / (ddsd->ddpfPixelFormat.dwBBitMask >> s);
	blue_mask = ddsd->ddpfPixelFormat.dwBBitMask;

	if ( ddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS ) {
		for (s = 0, m = ddsd->ddpfPixelFormat.dwRGBAlphaBitMask; !(m & 1); s++, m >>= 1);
		for (a = 0; m & 1; a++, m >>= 1);
		alpha_shift = s;
		alpha_scale = 255 / (ddsd->ddpfPixelFormat.dwRGBAlphaBitMask >> s);
		alpha_mask = ddsd->ddpfPixelFormat.dwRGBAlphaBitMask;
	} else {
		alpha_shift = a = alpha_scale = alpha_mask = 0;
	}

//	mprintf(( "Red    Bits:%2d, Shift:%2d, Scale:%3d, Mask:$%08x\n", r, red_shift, red_scale, red_mask ));
//	mprintf(( "Green  Bits:%2d, Shift:%2d, Scale:%3d, Mask:$%08x\n", g, green_shift, green_scale, green_mask ));
//	mprintf(( "Blue   Bits:%2d, Shift:%2d, Scale:%3d, Mask:$%08x\n", b, blue_shift, blue_scale, blue_mask ));
//	mprintf(( "Alpha  Bits:%2d, Shift:%2d, Scale:%3d, Mask:$%08x\n", a, alpha_shift, alpha_scale, alpha_mask ));

	// we need to set stuff up so that the high bit is always an alpha bit. 

	Gr_red.bits = r;
	Gr_red.shift = red_shift;
	Gr_red.scale = red_scale;
	Gr_red.mask = red_mask;

	Gr_green.bits = g;
	Gr_green.shift = green_shift;
	Gr_green.scale = green_scale;
	Gr_green.mask = green_mask;

	Gr_blue.bits = b;
	Gr_blue.shift = blue_shift;
	Gr_blue.scale = blue_scale;
	Gr_blue.mask = blue_mask;
}

// cross fade
void gr_directdraw_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	if ( pct <= 50 )	{
		gr_set_bitmap(bmap1);
		gr_bitmap(x1, y1);
	} else {
		gr_set_bitmap(bmap2);
		gr_bitmap(x2, y2);
	}	
}

// filter
void gr_directdraw_filter_set(int filter)
{
}

// set clear color
void gr_directdraw_set_clear_color(int r, int g, int b)
{
}

void gr_directdraw_init()
{
#ifndef HARDWARE_ONLY
//	int i;
	HRESULT ddrval;

	Palette_flashed = 0;
	Palette_flashed_last_frame = 0;

	gr_screen.bits_per_pixel = 8;
	gr_screen.bytes_per_pixel = 1;

	Gr_dd_activated = 0;

	HWND hwnd = (HWND)os_get_window();

	if ( !hwnd )	{
		mprintf(( "GR_SOFT_INIT: No window handle.\n" ));
		exit(1);
	}

	// Prepare the window to go full screen
#ifndef NDEBUG
	mprintf(( "Window in debugging mode... mouse clicking may cause problems!\n" ));
	SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
	SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
	ShowWindow(hwnd, SW_SHOWNORMAL );
	RECT work_rect;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
	SetWindowPos( hwnd, HWND_NOTOPMOST, work_rect.left, work_rect.top, gr_screen.max_w, gr_screen.max_h, 0 );	
	SetActiveWindow(hwnd);
	SetForegroundWindow(hwnd);
#else
	SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
	SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
	ShowWindow(hwnd, SW_SHOWNORMAL );
	SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ), 0 );	
	SetActiveWindow(hwnd);
	SetForegroundWindow(hwnd);
#endif

	os_suspend();
	ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
	os_resume();
	if ( ddrval != DD_OK ) {
		mprintf(( "GR_SOFT_INIT: DirectDrawCreate failed.\n" ));
		exit(1);
	}

	Assert( lpDD );

	ddrval = lpDD->SetCooperativeLevel( hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: SetCooperativeLevel EXCLUSIVE failed.\n" ));
		lpDD->Release();
		exit(1);
	}

	// Go to full screen!
	os_suspend();
	ddrval = lpDD->SetDisplayMode( gr_screen.max_w, gr_screen.max_h, gr_screen.bits_per_pixel );
	os_resume();
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: SetDisplayMode failed.\n" ));
		lpDD->Release();
		exit(1);
	}


	DDSURFACEDESC ddsd;

#ifdef DD_FLIP
	DDSCAPS ddscaps;

	memset( &ddsd, 0, sizeof( ddsd ));

	ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 2;

	ddrval = lpDD->CreateSurface( &ddsd, &lpFrontBuffer, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: CreateSurface (Front) failed.\n" ));
		lpDD->Release();
		exit(1);
	}

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

	ddrval = lpFrontBuffer->GetAttachedSurface( &ddscaps, &lpBackBuffer );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: GetAttachedSurface (Back) failed.\n" ));
		lpDD->Release();
		exit(1);
	}

	lpClipper = NULL;
#else

	memset( &ddsd, 0, sizeof( ddsd ) );

	ddsd.dwSize  = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	ddrval = lpDD->CreateSurface( &ddsd, &lpFrontBuffer, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: CreateSurface (Front) failed.\n" ));
		lpDD->Release();
		exit(1);
	}

	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.dwWidth = gr_screen.max_w;
	ddsd.dwHeight = gr_screen.max_h;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	// If debugging we have to create our surfaces in system memory
	// so that our debugger isn't hosed when locking surfaces.

	//ddsd.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
	ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	// create the back buffer
	ddrval = lpDD->CreateSurface( &ddsd, &lpBackBuffer , NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: CreateSurface (Back) failed.\n" ));
		goto DDError;
	}

	// create a clipper and attach it to our window
	ddrval = lpDD->CreateClipper( 0, &lpClipper, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: CreateClipper failed.\n" ));
		goto DDError;
	}


	ddrval = lpClipper->SetHWnd( 0, hwnd );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: SetHWnd failed.\n" ));
		goto DDError;
	}


	ddrval = lpFrontBuffer->SetClipper( lpClipper );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_SOFT_INIT: SetClipper failed.\n" ));
		goto DDError;
	}
#endif

	int i;
	PALETTEENTRY	pe[256];
	for (i=0; i<256; i++ )	{
		pe[i].peFlags = PC_NOCOLLAPSE|PC_RESERVED;
		pe[i].peRed = 0;
		pe[i].peGreen = 0;
		pe[i].peBlue = 0;
	}	
	ddrval = lpDD->CreatePalette( DDPCAPS_8BIT | DDPCAPS_ALLOW256, pe, &lpPalette, NULL);
	if (ddrval != DD_OK) {
		mprintf(( "Error creating palette\n" ));
		goto DDError;
	}

	ddrval = lpFrontBuffer->SetPalette( lpPalette );
	if (ddrval != DD_OK) {
		mprintf(( "Error setting palette in gr_directdraw_set_palette\n" ));
	}


	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof(ddsd);
	lpFrontBuffer->GetSurfaceDesc(&ddsd);   
	dd_get_shift_masks( &ddsd );

	mprintf(( "Surfaces created...\n" ));

	goto NoDDError;

DDError:
	lpDD->Release();
	exit(1);
		

NoDDError:
		;

	grx_init_alphacolors();

	gr_screen.gf_flip = gr_directdraw_flip;
	gr_screen.gf_flip_window = gr_directdraw_flip_window;
	gr_screen.gf_set_clip = gr_directdraw_set_clip;
	gr_screen.gf_reset_clip = gr_directdraw_reset_clip;
	gr_screen.gf_set_font = grx_set_font;
	gr_screen.gf_set_color = grx_set_color;
	gr_screen.gf_set_bitmap = gr_directdraw_set_bitmap;
	gr_screen.gf_create_shader = grx_create_shader;
	gr_screen.gf_set_shader = grx_set_shader;
	gr_screen.gf_clear = gr_directdraw_clear;
	// gr_screen.gf_bitmap = grx_bitmap;
	// gr_screen.gf_bitmap_ex = grx_bitmap_ex;

	gr_screen.gf_aabitmap = grx_aabitmap;
	gr_screen.gf_aabitmap_ex = grx_aabitmap_ex;

	gr_screen.gf_rect = grx_rect;
	gr_screen.gf_shade = gr8_shade;
	gr_screen.gf_string = gr8_string;
	gr_screen.gf_circle = gr8_circle;

	gr_screen.gf_line = gr8_line;
	gr_screen.gf_aaline = gr8_aaline;
	gr_screen.gf_pixel = gr8_pixel;
	gr_screen.gf_scaler = gr8_scaler;
	gr_screen.gf_aascaler = gr8_aascaler;
	gr_screen.gf_tmapper = grx_tmapper;

	gr_screen.gf_gradient = gr8_gradient;

	gr_screen.gf_set_palette = gr_directdraw_set_palette;
	gr_screen.gf_get_color = grx_get_color;
	gr_screen.gf_init_color = grx_init_color;
	gr_screen.gf_init_alphacolor = grx_init_alphacolor;
	gr_screen.gf_set_color_fast = grx_set_color_fast;
	gr_screen.gf_print_screen = grx_print_screen;

	gr_screen.gf_start_frame = gr_directdraw_start_frame;
	gr_screen.gf_stop_frame = gr_directdraw_stop_frame;

	gr_screen.gf_fade_in = gr_directdraw_fade_in;
	gr_screen.gf_fade_out = gr_directdraw_fade_out;
	gr_screen.gf_flash = gr_directdraw_flash;

	// Retrieves the zbuffer mode.
	gr_screen.gf_zbuffer_get = gr8_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr8_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr8_zbuffer_clear;

	gr_screen.gf_save_screen = gr8_save_screen;
	gr_screen.gf_restore_screen = gr8_restore_screen;
	gr_screen.gf_free_screen = gr8_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr8_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr8_dump_frame_stop;
	gr_screen.gf_dump_frame = gr8_dump_frame;

	// Gamma stuff
	gr_screen.gf_set_gamma = gr8_set_gamma;

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_directdraw_lock;
	gr_screen.gf_unlock = gr_directdraw_unlock_fake;

	// region
	gr_screen.gf_get_region = gr_directdraw_get_region;

	// poly culling
	gr_screen.gf_set_cull = gr_directdraw_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_directdraw_cross_fade;

	// filter
	gr_screen.gf_filter_set = gr_directdraw_filter_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_directdraw_set_clear_color;

	gr_lock();

	mprintf(( "Surfaces locked...\n" ));

	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;
#else
	Int3();
#endif
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrSoft.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-07-07 19:55:59 $
 * $Author: penguin $
 *
 * Code for our software renderer using standard Win32 functions.  (Dibsections, etc)
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/17 02:52:38  mharris
 * More porting hacks
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 11    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 10    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 9     2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 8     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 7     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 6     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 5     11/30/98 5:31p Dave
 * Fixed up Fred support for software mode.
 * 
 * 4     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 3     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 84    5/20/98 9:46p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 83    5/18/98 11:17a John
 * Fixed some bugs with software window and output window.
 * 
 * 82    5/17/98 5:03p John
 * Fixed some bugs that make the taskbar interfere with the DEBUG-only
 * mouse cursor.
 * 
 * 81    5/15/98 2:44p John
 * Made windowed mode not touch the window if no handle to window found.
 * 
 * 80    5/14/98 5:42p John
 * Revamped the whole window position/mouse code for the graphics windows.
 * 
 * 79    5/07/98 6:58p Hoffoss
 * Made changes to mouse code to fix a number of problems.
 * 
 * 78    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 77    4/23/98 8:24a John
 * Changed the way palette effect works so that:
 * 1. If gr_flash isn't called this frame, screen shows no flash.
 * 2. With hardware, only 3d portion of screen gets flashed.
 * 
 * 76    4/21/98 5:22p John
 * Fixed all the &^#@$ cursor bugs with popups.   For Glide, had problem
 * with mouse restoring assuming back buffer was same buffer last frame,
 * for software, problems with half drawn new frames, then mouse got
 * restored on top of that with old data.
 * 
 * 75    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 74    4/11/98 12:48p John
 * Made software fade out actually clear screen to black.
 * 
 * 73    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 72    3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 71    3/24/98 3:58p John
 * Put in (hopefully) final gamma setting code.
 * 
 * 70    3/17/98 5:55p John
 * Added code to dump Glide frames.   Moved Allender's  "hack" code out of
 * Freespace.cpp into the proper place, graphics lib.
 * 
 * 69    3/14/98 5:46p John
 * 
 * 68    3/14/98 5:43p John
 * Saved area under mouse cursor.  Made save_Screen restore it so no mouse
 * is on those screens.
 * 
 * 67    3/12/98 8:50a John
 * took out "allocating zbuffer" mprintf
 * 
 * 66    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 65    3/06/98 4:09p John
 * Changed the way we do bitmap RLE'ing... this speds up HUD bitmaps by
 * about 2x
 * 
 * 64    3/05/98 4:28p John
 * Made mouse cursor hide while setting palette.
 * 
 * 63    3/02/98 5:42p John
 * Removed WinAVI stuff from Freespace.  Made all HUD gauges wriggle from
 * afterburner.  Made gr_set_clip work good with negative x &y.  Made
 * model_caching be on by default.  Made each cached model have it's own
 * bitmap id.  Made asteroids not rotate when model_caching is on.  
 * 
 * 62    2/24/98 3:50p John
 * Made Alt+Tabbing work with Glide.  Made Int3's restore Glide screen on
 * next flip.
 * 
 * 61    2/24/98 1:59p John
 * Made gr_aabitmap_ex clip properly
 * 
 * 60    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 59    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 58    1/15/98 4:07p John
 * added grx_set_palette_internal which doesn't clear the screen and flip
 * before setting the palette.
 * 
 * 57    1/15/98 4:02p John
 * Fixed stupid bug on my part where I called graphics rendering functions
 * from another thread!!! Duh!  Fixed it by setting a variable and then
 * resetting the palette at the next flip.
 * 
 * 56    1/08/98 5:06p John
 * Replaced Int3 with a mprintf
 * 
 * 55    1/08/98 3:10p John
 * Undid my palette changes for interplay rev
 * 
 * 54    1/08/98 3:03p John
 * 
 * 53    1/05/98 11:52a Jasen
 * JAS: Made graphics code work if os_get_window returns NULL when trying
 * to go fullscreen.
 * 
 * 52    12/30/97 6:47p John
 * Made fade time correct
 * 
 * 51    12/30/97 6:46p John
 * Added first rev of palette fade in out functions
 * 
 * 50    12/21/97 4:33p John
 * Made debug console functions a class that registers itself
 * automatically, so you don't need to add the function to
 * debugfunctions.cpp.  
 * 
 * 49    12/05/97 4:26p John
 * made palette set code be like it was before.
 * 
 * 48    12/04/97 8:05p John
 * added test code to brighten palette
 * 
 * 47    12/03/97 2:16p John
 * Took out blur code and fake_vram buffer.
 * 
 * 46    12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 45    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 44    11/29/97 2:06p John
 * added mode 16-bpp support
 * 
 * 43    11/21/97 11:32a John
 * Added nebulas.   Fixed some warpout bugs.
 * 
 * 42    11/20/97 9:51a John
 * added code to force screen to 16-bit even if rendering 8.
 * 
 * 41    11/17/97 10:33a John
 * updated force_windowed code.
 * 
 * 40    11/14/97 3:54p John
 * Added triple buffering.
 * 
 * 39    11/14/97 3:08p Johnson
 * fixed bug with fred.
 * 
 * 38    11/14/97 12:30p John
 * Fixed some DirectX bugs.  Moved the 8-16 xlat tables into Graphics
 * libs.  Made 16-bpp DirectX modes know what bitmap format they're in.
 * 
 * 37    11/06/97 4:12p John
 * Took out debug code that returned NULL for DD.
 * 
 * 36    11/05/97 12:35p John
 * added correct gr_bitmap_ex that does clipping
 * 
 * 
 * 35    10/31/97 9:43a John
 * fixed red background color bug.
 * 
 * 34    10/19/97 1:53p John
 * hacked in a fix to a zbuffer error caused by inaccuracies.
 * 
 * 33    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 32    10/15/97 4:48p John
 * added 16-bpp aascaler
 * 
 * 31    10/14/97 4:50p John
 * more 16 bpp stuff.
 * 
 * 30    10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 29    10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 28    10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 27    9/23/97 11:50a Lawrance
 * jas: added xparency to rle'd bitblts
 * 
 * 26    9/23/97 10:45a John
 * made so you can tell bitblt code to rle a bitmap by passing flag to
 * gr_set_bitmap
 * 
 * 25    9/20/97 10:48a John
 * added motion blur code.
 * 
 * 24    9/09/97 10:39a Sandeep
 * fixed warning level 4
 * 
 * 23    9/07/97 2:34p John
 * fixed bug with fullscreen toggling not working when in > 8bpp mode.
 * 
 * 22    9/03/97 4:32p John
 * changed bmpman to only accept ani and pcx's.  made passing .pcx or .ani
 * to bm_load functions not needed.   Made bmpman keep track of palettes
 * for bitmaps not mapped into game palettes.
 * 
 * 21    8/26/97 11:30a John
 * removed call to restoredisplymode when going from fullscreen to
 * windowed, which hopefully fixes the screwey taskbar problem.
 * 
 * 20    8/04/97 4:47p John
 * added gr_aascaler.
 * 
 * 19    7/17/97 9:31a John
 * made all directX header files name start with a v
 * 
 * 18    7/10/97 2:06p John
 * added code to specify alphablending type for bitmaps.
 * 
 * 17    6/20/97 1:50p John
 * added rle code to bmpman.  made gr8_aabitmap use it.
 * 
 * 16    6/17/97 12:03p John
 * Moved color/alphacolor functions into their own module.  Made all color
 * functions be part of the low-level graphics drivers, not just the
 * grsoft.
 * 
 * 15    6/13/97 5:35p John
 * added some antialiased bitmaps and lines
 * 
 * 14    6/12/97 2:50a Lawrance
 * bm_unlock() now passed bitmap number, not pointer
 * 
 * 13    6/11/97 5:49p John
 * Changed palette code to only recalculate alphacolors when needed, not
 * when palette changes.
 * 
 * 12    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 11    6/06/97 5:03p John
 * fixed bug withalpha colors failing after gr_init
 * 
 * 10    6/06/97 4:41p John
 * Fixed alpha colors to be smoothly integrated into gr_set_color_fast
 * code.
 * 
 * 9     5/29/97 3:09p John
 * Took out debug menu.  
 * Made software scaler draw larger bitmaps.
 * Optimized Direct3D some.
 * 
 * 8     5/16/97 9:11a John
 * fixed bug that made Ctrl+Break in fullscreen hang
 * 
 * 7     5/14/97 4:40p John
 * took out mprintf
 * 
 * 6     5/14/97 4:38p John
 * Fixed print_screen bug.
 * 
 * 5     5/14/97 2:09p John
 * made fullscreen use 254 palette entries.    Used
 * CreateCompatibleDC(NULL) Instead of GetDC(HWND_DESKTOP) because the
 * GetDC method fails when you change screen depth dynamically under NT.
 * 
 * 4     5/14/97 10:53a John
 * fixed some discrepencies between d3d and software palette setting.
 * 
 * 3     5/14/97 8:53a John
 * Fixed a palette bug when switching into/outof d3d mode.
 * 
 * 2     5/13/97 12:39p John
 * Got fullscreen mode working.
 * 
 * 1     5/12/97 12:14p John
 *
 * $NoKeywords: $
 */

#include <math.h>
#include <windows.h>
#include <windowsx.h>

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

#ifdef _WIN32
 // Windows specific

// This structure is the same as LOGPALETTE except that LOGPALETTE
// requires you to malloc out space for the palette, which just isn't
// worth the trouble.

typedef struct {
    WORD         palVersion; 
    WORD         palNumEntries; 
    PALETTEENTRY palPalEntry[256]; 
} EZ_LOGPALETTE; 

// This structure is the same as BITMAPINFO except that BITMAPINFO
// requires you to malloc out space for the palette, which just isn't
// worth the trouble.   I also went ahead and threw a handy union to
// easily reference the hicolor masks in 15/16 bpp modes.
typedef struct	{
	BITMAPINFOHEADER Header;
	union {
		RGBQUAD aColors[256];
		ushort PalIndex[256];
		uint hicolor_masks[3];
	} Colors;
} EZ_BITMAPINFO;

EZ_BITMAPINFO DibInfo;
HBITMAP hDibSection = NULL;
HBITMAP hOldBitmap = NULL;
HDC hDibDC = NULL;
void *lpDibBits=NULL;

HPALETTE hOldPalette=NULL, hPalette = NULL;	
#endif  // ifdef WIN32

int Gr_soft_inited = 0;

static volatile int Grsoft_activated = 0;				// If set, that means application got focus, so reset palette

void gr_buffer_release()
{
	if ( hPalette )	{
		if (hDibDC)
			SelectPalette( hDibDC, hOldPalette, FALSE );
		if (!DeleteObject(hPalette))	{
			mprintf(( "JOHN: Couldn't delete palette object\n" ));
		}
		hPalette = NULL;
	}

	if ( hDibDC )	{
		SelectObject(hDibDC, hOldBitmap );
		DeleteDC(hDibDC);
		hDibDC = NULL;
	}

	if ( hDibSection )	{
		DeleteObject(hDibSection);
		hDibSection = NULL;
	}
}


void gr_buffer_create( int w, int h, int bpp )
{
	int i;

	if (w & 3) {
		Int3();	// w must be multiple 4
		return;
	}

	gr_buffer_release();

	memset( &DibInfo, 0, sizeof(EZ_BITMAPINFO));
   DibInfo.Header.biSize = sizeof(BITMAPINFOHEADER);
	DibInfo.Header.biWidth = w;
	DibInfo.Header.biHeight = h;
	DibInfo.Header.biPlanes = 1; 
	DibInfo.Header.biClrUsed = 0;

	switch( bpp )	{
	case 8:
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

		DibInfo.Header.biCompression = BI_RGB; 
		DibInfo.Header.biBitCount = 8; 
		for (i=0; i<256; i++ )	{
			DibInfo.Colors.aColors[i].rgbRed = 0;
			DibInfo.Colors.aColors[i].rgbGreen = 0;
			DibInfo.Colors.aColors[i].rgbBlue = 0;
			DibInfo.Colors.aColors[i].rgbReserved = 0;
		}
		break;

	case 15:
		Gr_red.bits = 5;
		Gr_red.shift = 10;
		Gr_red.scale = 8;
		Gr_red.mask = 0x7C00;

		Gr_green.bits = 5;
		Gr_green.shift = 5;
		Gr_green.scale = 8;
		Gr_green.mask = 0x3E0;

		Gr_blue.bits = 5;
		Gr_blue.shift = 0;
		Gr_blue.scale = 8;
		Gr_blue.mask = 0x1F;

		DibInfo.Header.biCompression = BI_BITFIELDS;
		DibInfo.Header.biBitCount = 16; 
		DibInfo.Colors.hicolor_masks[0] = Gr_red.mask;
		DibInfo.Colors.hicolor_masks[1] = Gr_green.mask;
		DibInfo.Colors.hicolor_masks[2] = Gr_blue.mask;


		break;

	case 16:
		Gr_red.bits = 5;
		Gr_red.shift = 11;
		Gr_red.scale = 8;
		Gr_red.mask = 0xF800;

		Gr_green.bits = 6;
		Gr_green.shift = 5;
		Gr_green.scale = 4;
		Gr_green.mask = 0x7E0;

		Gr_blue.bits = 5;
		Gr_blue.shift = 0;
		Gr_blue.scale = 8;
		Gr_blue.mask = 0x1F;

		DibInfo.Header.biCompression = BI_BITFIELDS;
		DibInfo.Header.biBitCount = 16; 
		DibInfo.Colors.hicolor_masks[0] = Gr_red.mask;
		DibInfo.Colors.hicolor_masks[1] = Gr_green.mask;
		DibInfo.Colors.hicolor_masks[2] = Gr_blue.mask;
		break;

	case 24:
	case 32:
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

		DibInfo.Header.biCompression = BI_RGB; 
		DibInfo.Header.biBitCount = unsigned short(bpp); 
		break;

	default:
		Int3();	// Illegal bpp
	}

	lpDibBits = NULL;

	hDibDC = CreateCompatibleDC(NULL);
	hDibSection = CreateDIBSection(hDibDC,(BITMAPINFO *)&DibInfo,DIB_RGB_COLORS,&lpDibBits,NULL,NULL);
	hOldBitmap = (HBITMAP)SelectObject(hDibDC, hDibSection );

	if ( hDibSection == NULL )	{
		Int3();	// couldn't allocate dib section
	}
}


// This makes a best-fit palette from the 256 target colors and
// the system colors which should look better than only using the
// colors in the range 10-246, but it will totally reorder the palette.
// All colors get changed in target_palette.


HPALETTE gr_create_palette_0(ubyte * target_palette)
{
	EZ_LOGPALETTE LogicalPalette;
	HDC ScreenDC;
	HPALETTE hpal,hpalOld;
	PALETTEENTRY pe[256];
	int NumSysColors, NumColors;
	int i;

	// Create a 1-1 mapping of the system palette
	LogicalPalette.palVersion = 0x300;
	LogicalPalette.palNumEntries = 256;

	// Pack in all the colors
	for (i=0;i<256; i++)	{
		LogicalPalette.palPalEntry[i].peRed = target_palette[i*3+0];
		LogicalPalette.palPalEntry[i].peGreen = target_palette[i*3+1];
		LogicalPalette.palPalEntry[i].peBlue = target_palette[i*3+2];
		LogicalPalette.palPalEntry[i].peFlags = 0;	//PC_EXPLICIT;
	} 

	hpal = CreatePalette( (LOGPALETTE *)&LogicalPalette );

	ScreenDC = CreateCompatibleDC(NULL);

	if ( !(GetDeviceCaps(ScreenDC,RASTERCAPS) & RC_PALETTE) ) {
		DeleteDC(ScreenDC);
		return hpal;
	}
	 
	NumSysColors = GetDeviceCaps( ScreenDC, NUMCOLORS );
	NumColors = GetDeviceCaps( ScreenDC, SIZEPALETTE );

	// Reset all the Palette Manager tables
	SetSystemPaletteUse( ScreenDC, SYSPAL_NOSTATIC );
	SetSystemPaletteUse( ScreenDC, SYSPAL_STATIC );

	// Enter our palette's values into the free slots
	hpalOld=SelectPalette( ScreenDC, hpal, FALSE );
	RealizePalette( ScreenDC );
	SelectPalette( ScreenDC, hpalOld, FALSE );

	GetSystemPaletteEntries(ScreenDC,0,NumColors,pe);

	for (i=0; i<NumSysColors/2; i++ )	{
		pe[i].peFlags = 0;
	}
	for (; i<NumColors - NumSysColors/2; i++ )	{
		pe[i].peFlags = PC_NOCOLLAPSE;
	}
	for (; i<NumColors; i++ )	{
		pe[i].peFlags = 0;
	}
	ResizePalette( hpal, NumColors);
	SetPaletteEntries( hpal, 0, NumColors, pe );
		
	for (i=0; i<256; i++ )	{
		target_palette[i*3+0] = pe[i].peRed;
		target_palette[i*3+1] = pe[i].peGreen;
		target_palette[i*3+2] = pe[i].peBlue;
	}

	DeleteDC(ScreenDC);

	return hpal;
}

HPALETTE gr_create_palette_256( ubyte * target_palette )
{
	EZ_LOGPALETTE LogicalPalette;
	int i;

	// Pack in all the colors
	for (i=0;i<256; i++)	{
		LogicalPalette.palPalEntry[i].peRed = target_palette[i*3+0];
		LogicalPalette.palPalEntry[i].peGreen = target_palette[i*3+1];
		LogicalPalette.palPalEntry[i].peBlue = target_palette[i*3+2];
		LogicalPalette.palPalEntry[i].peFlags = 0;	//PC_RESERVED;	//PC_EXPLICIT;
	} 

	// Create a 1-1 mapping of the system palette
	LogicalPalette.palVersion = 0x300;
	LogicalPalette.palNumEntries = 256;

	return CreatePalette( (LOGPALETTE *)&LogicalPalette );
}

// This makes an indentity logical palette that saves entries 10-246
// and leaves them in place.  Colors 0-9 and 246-255 get changed in 
// target_palette.  trash_flag tells us whether to trash the system palette
// or not
HPALETTE gr_create_palette_236( ubyte * target_palette )
{
	EZ_LOGPALETTE LogicalPalette;
	HDC ScreenDC;
	int NumSysColors, NumColors, UserLowest, UserHighest;
	int i;

	// Pack in all the colors
	for (i=0;i<256; i++)	{
		LogicalPalette.palPalEntry[i].peRed = target_palette[i*3+0];
		LogicalPalette.palPalEntry[i].peGreen = target_palette[i*3+1];
		LogicalPalette.palPalEntry[i].peBlue = target_palette[i*3+2];
		LogicalPalette.palPalEntry[i].peFlags = 0;	//PC_EXPLICIT;
	} 

	// Create a 1-1 mapping of the system palette
	LogicalPalette.palVersion = 0x300;
	LogicalPalette.palNumEntries = 256;

	ScreenDC = CreateCompatibleDC(NULL);

	// Reset all the Palette Manager tables
	SetSystemPaletteUse( ScreenDC, SYSPAL_NOSTATIC );
	SetSystemPaletteUse( ScreenDC, SYSPAL_STATIC );
		
	if ( !(GetDeviceCaps(ScreenDC,RASTERCAPS) & RC_PALETTE) ) {
		DeleteDC(ScreenDC);
		return CreatePalette( (LOGPALETTE *)&LogicalPalette );
	}

	NumSysColors = GetDeviceCaps( ScreenDC, NUMCOLORS );
	NumColors = GetDeviceCaps( ScreenDC, SIZEPALETTE );

	Assert( NumColors <= 256 );

	UserLowest = NumSysColors/2;								// 10 normally
	UserHighest = NumColors - NumSysColors/2 - 1;		// 245 normally

	Assert( (UserHighest - UserLowest + 1) >= 236 );
			
	GetSystemPaletteEntries(ScreenDC,0,NumSysColors/2,LogicalPalette.palPalEntry);
	GetSystemPaletteEntries(ScreenDC,UserHighest+1,NumSysColors/2,LogicalPalette.palPalEntry+1+UserHighest);

	DeleteDC(ScreenDC);
		
	for (i=0; i<256; i++ )	{

		if ( (i >= UserLowest) && (i<=UserHighest) )	{
			LogicalPalette.palPalEntry[i].peFlags = PC_NOCOLLAPSE;
		} else
			LogicalPalette.palPalEntry[i].peFlags = 0;

		target_palette[i*3+0] = LogicalPalette.palPalEntry[i].peRed;
		target_palette[i*3+1] = LogicalPalette.palPalEntry[i].peGreen;
		target_palette[i*3+2] = LogicalPalette.palPalEntry[i].peBlue;
	}

	return CreatePalette( (LOGPALETTE *)&LogicalPalette );
}

HPALETTE gr_create_palette_254( ubyte * target_palette )
{
	EZ_LOGPALETTE LogicalPalette;
	HDC ScreenDC;
	int NumSysColors, NumColors, UserLowest, UserHighest;
	int i;

	// Pack in all the colors
	for (i=0;i<256; i++)	{
		LogicalPalette.palPalEntry[i].peRed = target_palette[i*3+0];
		LogicalPalette.palPalEntry[i].peGreen = target_palette[i*3+1];
		LogicalPalette.palPalEntry[i].peBlue = target_palette[i*3+2];
		LogicalPalette.palPalEntry[i].peFlags = 0;	//PC_EXPLICIT;
	} 

	// Create a 1-1 mapping of the system palette
	LogicalPalette.palVersion = 0x300;
	LogicalPalette.palNumEntries = 256;

	ScreenDC = CreateCompatibleDC(NULL);

	// Reset all the Palette Manager tables
	SetSystemPaletteUse( ScreenDC, SYSPAL_NOSTATIC );
	SetSystemPaletteUse( ScreenDC, SYSPAL_STATIC );
		
	if ( !(GetDeviceCaps(ScreenDC,RASTERCAPS) & RC_PALETTE) ) {
		DeleteDC(ScreenDC);
		return CreatePalette( (LOGPALETTE *)&LogicalPalette );
	}

	SetSystemPaletteUse( ScreenDC, SYSPAL_NOSTATIC );
	NumSysColors = 2;
	NumColors = GetDeviceCaps( ScreenDC, SIZEPALETTE );

	Assert( NumColors <= 256 );

	UserLowest = NumSysColors/2;								// 10 normally
	UserHighest = NumColors - NumSysColors/2 - 1;		// 245 normally

	Assert( (UserHighest - UserLowest + 1) >= 236 );
			
	GetSystemPaletteEntries(ScreenDC,0,NumSysColors/2,LogicalPalette.palPalEntry);
	GetSystemPaletteEntries(ScreenDC,UserHighest+1,NumSysColors/2,LogicalPalette.palPalEntry+1+UserHighest);

	DeleteDC(ScreenDC);
	
	for (i=0; i<256; i++ )	{

		if ( (i >= UserLowest) && (i<=UserHighest) )	{
			LogicalPalette.palPalEntry[i].peFlags = PC_NOCOLLAPSE;
		} else
			LogicalPalette.palPalEntry[i].peFlags = 0;

		target_palette[i*3+0] = LogicalPalette.palPalEntry[i].peRed;
		target_palette[i*3+1] = LogicalPalette.palPalEntry[i].peGreen;
		target_palette[i*3+2] = LogicalPalette.palPalEntry[i].peBlue;
	}

	return CreatePalette( (LOGPALETTE *)&LogicalPalette );
}

void grx_set_palette_internal( ubyte * new_pal )
{
	if ( hPalette )	{
		if (hDibDC)
			SelectPalette( hDibDC, hOldPalette, FALSE );
		if (!DeleteObject(hPalette))	{
			mprintf(( "JOHN: Couldn't delete palette object\n" ));
		}
		hPalette = NULL;
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



	if ( gr_screen.bits_per_pixel==8 )	{

		// Name                    n_preserved  One-one     Speed      Windowed? 
		// -------------------------------------------------------------------
		// gr_create_palette_256   256          0-255       Slow       Yes
		// gr_create_palette_254   254          1-254       Fast       No
		// gr_create_palette_236   236          10-245      Fast       Yes
		// gr_create_palette_0     0            none        Fast       Yes		

/*
		n_preserved = 256;

		if ( n_preserved <= 0 )	{
			hPalette = gr_create_palette_0(new_pal);	// No colors mapped one-to-one, but probably has close to all 256 colors in it somewhere.
		} else if ( n_preserved <= 236 )	{
			hPalette = gr_create_palette_236(new_pal);	// All colors except low 10 and high 10 mapped one-to-one
		} else if ( n_preserved <= 254 )	{
			hPalette = gr_create_palette_254(new_pal);	// All colors except 0 and 255 mapped one-to-one, but changes system colors.  Not pretty in a window.
		} else {
*/
		hPalette = gr_create_palette_256(new_pal);	// All 256 mapped one-to-one, but BLT's are slow.

		if ( hDibDC )	{
			int i; 
			for (i=0; i<256; i++ )	{
				DibInfo.Colors.aColors[i].rgbRed = new_pal[i*3+0];
				DibInfo.Colors.aColors[i].rgbGreen = new_pal[i*3+1];
				DibInfo.Colors.aColors[i].rgbBlue = new_pal[i*3+2];
				DibInfo.Colors.aColors[i].rgbReserved = 0;
			}

			hOldPalette = SelectPalette( hDibDC, hPalette, FALSE );
			SetDIBColorTable( hDibDC, 0, 256, DibInfo.Colors.aColors );
		}
	} else {
		hPalette = NULL;
	}
}



void grx_set_palette( ubyte * new_pal, int is_alphacolor )
{
	if ( hPalette )	{
		Mouse_hidden++;
		gr_reset_clip();
		gr_clear();
		gr_flip();
		Mouse_hidden--;
	}

	grx_set_palette_internal(new_pal);
}


void grx_print_screen(char * filename)
{
	int i;
	ubyte **row_data = (ubyte **)malloc( gr_screen.max_h * sizeof(ubyte *) );
	if ( !row_data )	{
		mprintf(( "couldn't allocate enough memory to dump screen\n" ));
		return;
	}

	gr_lock();

	for (i=0; i<gr_screen.max_h; i++ )	{
		row_data[i] = GR_SCREEN_PTR(ubyte,0,i);
	}

	pcx_write_bitmap( filename, gr_screen.max_w, gr_screen.max_h, row_data, Gr_current_palette );

	gr_unlock();

	free(row_data);
}


uint gr_soft_lock()
{
	return 1;
}

void gr_soft_unlock()
{
}


void grx_set_palette_internal( ubyte * new_pal );

int Grx_mouse_saved = 0;
int Grx_mouse_saved_x1 = 0;
int Grx_mouse_saved_y1 = 0;
int Grx_mouse_saved_x2 = 0;
int Grx_mouse_saved_y2 = 0;
int Grx_mouse_saved_w = 0;
int Grx_mouse_saved_h = 0;
#define MAX_SAVE_SIZE (32*32)
ubyte Grx_mouse_saved_data[MAX_SAVE_SIZE];

// Clamps X between R1 and R2
#define CLAMP(x,r1,r2) do { if ( (x) < (r1) ) (x) = (r1); else if ((x) > (r2)) (x) = (r2); } while(0)

void grx_save_mouse_area(int x, int y, int w, int h )
{
	Grx_mouse_saved_x1 = x; 
	Grx_mouse_saved_y1 = y;
	Grx_mouse_saved_x2 = x+w-1;
	Grx_mouse_saved_y2 = y+h-1;
	 
	CLAMP(Grx_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Grx_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Grx_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(Grx_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );

	Grx_mouse_saved_w = Grx_mouse_saved_x2 - Grx_mouse_saved_x1 + 1;
	Grx_mouse_saved_h = Grx_mouse_saved_y2 - Grx_mouse_saved_y1 + 1;

	if ( Grx_mouse_saved_w < 1 ) return;
	if ( Grx_mouse_saved_h < 1 ) return;

	// Make sure we're not saving too much!
	Assert( (Grx_mouse_saved_w*Grx_mouse_saved_h) <= MAX_SAVE_SIZE );

	Grx_mouse_saved = 1;

	gr_lock();

	ubyte *sptr, *dptr;

	dptr = Grx_mouse_saved_data;

	for (int i=0; i<Grx_mouse_saved_h; i++ )	{
		sptr = GR_SCREEN_PTR(ubyte,Grx_mouse_saved_x1,Grx_mouse_saved_y1+i);

		for(int j=0; j<Grx_mouse_saved_w; j++ )	{
			*dptr++ = *sptr++;
		}
	}

	gr_unlock();
}


void grx_restore_mouse_area()
{
	if ( !Grx_mouse_saved )	{
		return;
	}

	gr_lock();

	ubyte *sptr, *dptr;

	sptr = Grx_mouse_saved_data;

	for (int i=0; i<Grx_mouse_saved_h; i++ )	{
		dptr = GR_SCREEN_PTR(ubyte,Grx_mouse_saved_x1,Grx_mouse_saved_y1+i);

		for(int j=0; j<Grx_mouse_saved_w; j++ )	{
			*dptr++ = *sptr++;
		}
	}

	gr_unlock();
}


void gr_soft_activate(int active)
{
	if ( active  )	{
		Grsoft_activated++;
	}
}



static int Palette_flashed = 0;
static int Palette_flashed_last_frame = 0;

void grx_change_palette( ubyte *pal );

void grx_flip()
{
	if ( (!Palette_flashed) && (Palette_flashed_last_frame) )	{
		// Reset flash
		grx_change_palette( gr_palette );
	}

	Palette_flashed_last_frame = Palette_flashed;
	Palette_flashed = 0;

	// If program reactivated, flip set new palette.
	// We do the cnt temporary variable because Grsoft_activated
	// can be set during interrupts.

	int cnt = Grsoft_activated;
	if ( cnt )	{
		Grsoft_activated -= cnt;

		ubyte new_pal[768];
		memcpy( new_pal, gr_palette, 768 );
		grx_set_palette_internal( new_pal );		// Call internal one so it doesn't clear screen and call flip
	}

	gr_reset_clip();

//	if (0) {
//		int i;
//		for (i=0; i<gr_screen.max_h; i++ )	{
//			memset( gr_screen.row_data[i], i & 255, abs(gr_screen.rowsize) );
//		}
//	}

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

	fix t1, t2, d, t;

	HWND hwnd = (HWND)os_get_window();

	if ( hwnd )	{
		int x = gr_screen.offset_x;
		int y = gr_screen.offset_y;
		int w = gr_screen.clip_width;
		int h = gr_screen.clip_height;

		HPALETTE hOldPalette = NULL;
		HDC hdc = GetDC(hwnd);

		if ( hdc )	{
			t1 = timer_get_fixed_seconds();

			if (hPalette)	{
				hOldPalette=SelectPalette(hdc,hPalette, FALSE );
				uint nColors = RealizePalette( hdc );
				nColors;
				//if (nColors)	mprintf(( "Actually set %d palette colors.\n", nColors ));
			}

#if 0 
			BitBlt(hdc,0,h/2,w,h/2,hDibDC,x,y+h/2,SRCCOPY);
#else
			BitBlt(hdc,0,0,w,h,hDibDC,x,y,SRCCOPY);
#endif

			if ( hOldPalette )	
				SelectPalette(hdc,hOldPalette, FALSE );

			ReleaseDC( hwnd, hdc );

			t2 = timer_get_fixed_seconds();
			d = t2 - t1;
			t = (w*h*gr_screen.bytes_per_pixel)/1024;
			//mprintf(( "%d MB/s\n", fixmuldiv(t,65,d) ));

		}
	}

	if ( Grx_mouse_saved )	{
		grx_restore_mouse_area();
	}
}


// switch onscreen, offscreen
// Set msg to 0 if calling outside of the window handler.
void grx_flip_window(uint _hdc, int x, int y, int w, int h )
{
	HDC hdc = (HDC)_hdc;
	HPALETTE hOldPalette = NULL;
	int min_w, min_h;

	if (hPalette)	{
		hOldPalette=SelectPalette(hdc,hPalette, FALSE );
		RealizePalette( hdc );
	}

	min_w = gr_screen.clip_width;
	if ( w < min_w ) min_w = w;

	min_h = gr_screen.clip_height;
	if ( h < min_h ) min_h = h;

	BitBlt(hdc,x,y,min_w,min_h,hDibDC,gr_screen.offset_x,gr_screen.offset_y,SRCCOPY);

	//StretchBlt( hdc, 0, 0, w, h, hDibDC, 0, 0, 640, 480, SRCCOPY );
	
	if ( hOldPalette ){	
		SelectPalette(hdc,hOldPalette, FALSE );
	}
}


// sets the clipping region & offset
void grx_set_clip(int x,int y,int w,int h)
{
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
}

// resets the clipping region to entire screen
//
// should call this before gr_surface_flip() if you clipped some portion of the screen but still 
// want a full-screen display
void grx_reset_clip()
{
	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;
}


// Sets the current bitmap
void grx_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
}


// clears entire clipping region to black.
void grx_clear()
{
	gr_lock();

	int i,w;
	ubyte *pDestBits;

	w = gr_screen.clip_right-gr_screen.clip_left+1;
	for (i=gr_screen.clip_top; i<=gr_screen.clip_bottom; i++)	{
		pDestBits = GR_SCREEN_PTR(ubyte,gr_screen.clip_left,i);
		memset( pDestBits, 0, w );
	}	

	gr_unlock();
}



void grx_start_frame()
{
}

void grx_stop_frame()
{
}

void gr_soft_fog_set(int fog_mode, int r, int g, int b, float near, float far)
{
}

void gr_soft_get_pixel(int x, int y, int *r, int *g, int *b)
{
}

void grx_fade_in(int instantaneous);
void grx_fade_out(int instantaneous);
void grx_flash(int r, int g, int b);

static ubyte *Gr_saved_screen = NULL;
static uint Gr_saved_screen_palette_checksum = 0;
static ubyte Gr_saved_screen_palette[768];

int gr8_save_screen()
{
	int i;
	gr_reset_clip();

	if (gr_screen.bits_per_pixel != 8) {
		mprintf(( "Save Screen only works in 8 bpp!\n" ));
		return -1;
	}

	if ( Gr_saved_screen )	{
		mprintf(( "Screen alread saved!\n" ));
		return -1;
	}

	Gr_saved_screen = (ubyte *)malloc( gr_screen.max_w*gr_screen.max_h );
	if (!Gr_saved_screen) {
		mprintf(( "Couldn't get memory for saved screen!\n" ));
		return -1;
	}

	Gr_saved_screen_palette_checksum = gr_palette_checksum;
	memcpy( Gr_saved_screen_palette, gr_palette, 768 );

	gr_lock();

	for (i=0; i<gr_screen.max_h; i++ )	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,0,i);
		memcpy( &Gr_saved_screen[gr_screen.max_w*i], dptr, gr_screen.max_w );
	}

	gr_unlock();

	return 0;
}


void gr8_restore_screen(int id)
{
	int i;
	gr_reset_clip();

	if ( !Gr_saved_screen )	{
		gr_clear();
		return;
	}

	if ( Gr_saved_screen_palette_checksum != gr_palette_checksum )	{
		// Palette changed! Remap the bitmap!
		ubyte xlat[256];
		for (i=0; i<256; i++ )	{
			xlat[i] = (ubyte)palette_find( Gr_saved_screen_palette[i*3+0], Gr_saved_screen_palette[i*3+1], Gr_saved_screen_palette[i*3+2] );
		}	

		for (i=0; i<gr_screen.max_h*gr_screen.max_w; i++ )	{
			Gr_saved_screen[i] = xlat[Gr_saved_screen[i]];
		}

		memcpy( Gr_saved_screen_palette, gr_palette, 768 );
		Gr_saved_screen_palette_checksum = gr_palette_checksum;
	}

	gr_lock();

	for (i=0; i<gr_screen.max_h; i++ )	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,0,i);
		memcpy( dptr, &Gr_saved_screen[gr_screen.max_w*i], gr_screen.max_w );
	}

	gr_unlock();
}


void gr8_free_screen(int id)
{
	if ( Gr_saved_screen )	{
		free( Gr_saved_screen );
		Gr_saved_screen = NULL;
	}
}

static int Gr8_dump_frames = 0;
static ubyte *Gr8_dump_buffer = NULL;
static int Gr8_dump_frame_number = 0;
static int Gr8_dump_frame_count = 0;
static int Gr8_dump_frame_count_max = 0;
static int Gr8_dump_frame_size = 0;


void gr8_dump_frame_start(int first_frame, int frames_between_dumps)
{
	if ( Gr8_dump_frames )	{
		Int3();		//  We're already dumping frames.  See John.
		return;
	}	
	Gr8_dump_frames = 1;
	Gr8_dump_frame_number = first_frame;
	Gr8_dump_frame_count = 0;
	Gr8_dump_frame_count_max = frames_between_dumps;
	Gr8_dump_frame_size = 640 * 480;
	
	if ( !Gr8_dump_buffer ) {
		int size = Gr8_dump_frame_count_max * Gr8_dump_frame_size;
		Gr8_dump_buffer = (ubyte *)malloc(size);
		if ( !Gr8_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}
}

// A hacked function to dump the frame buffer contents
void gr8_dump_screen_hack( void * dst )
{
	int i;

	gr_lock();
	for (i = 0; i < 480; i++)	{
		memcpy( (ubyte *)dst+(i*640), GR_SCREEN_PTR(ubyte,0,i), 640 );
	}
	gr_unlock();
}

void gr8_flush_frame_dump()
{
	ubyte *buffer[480];
	char filename[MAX_PATH_LEN], *movie_path = "";

	int i;
	for (i = 0; i < Gr8_dump_frame_count; i++) {
		int j;

		for ( j = 0; j < 480; j++ )
			buffer[j] = Gr8_dump_buffer+(i*Gr8_dump_frame_size)+(j*640);

		sprintf(filename, NOX("%sfrm%04d"), movie_path, Gr8_dump_frame_number );
		pcx_write_bitmap(filename, 640, 480, buffer, gr_palette);
		Gr8_dump_frame_number++;
	}
}

void gr8_dump_frame()
{
	// A hacked function to dump the frame buffer contents
	gr8_dump_screen_hack( Gr8_dump_buffer+(Gr8_dump_frame_count*Gr8_dump_frame_size) );

	Gr8_dump_frame_count++;

	if ( Gr8_dump_frame_count == Gr8_dump_frame_count_max ) {
		gr8_flush_frame_dump();
		Gr8_dump_frame_count = 0;
	}
}

void grx_get_region(int front, int w, int h, ubyte *data)
{
}

// resolution checking
int gr_soft_supports_res_ingame(int res)
{
	return 1;
}

int gr_soft_supports_res_interface(int res)
{
	return 1;
}

void gr8_dump_frame_stop()
{
	if ( !Gr8_dump_frames )	{
		Int3();		//  We're not dumping frames.  See John.
		return;
	}	

	// dump any remaining frames
	gr8_flush_frame_dump();
	
	Gr8_dump_frames = 0;
	if ( Gr8_dump_buffer )	{
		free(Gr8_dump_buffer);
		Gr8_dump_buffer = NULL;
	}
}

void gr_soft_set_cull(int cull)
{
}

// cross fade
void gr_soft_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
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
void gr_soft_filter_set(int filter)
{
}

// tcache
int gr_soft_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0 )
{
	return 1;
}

// clear color
void gr_soft_set_clear_color(int r, int g, int b)
{
}

extern uint Gr_signature;

//extern void gr_set_palette_internal(char *name, ubyte *pal);	

void gr8_set_gamma(float gamma)
{
	Gr_gamma = gamma;
	Gr_gamma_int = int(Gr_gamma*100);

	// Create the Gamma lookup table
	int i;
	for (i=0; i<256; i++ )	{
		int v = fl2i(pow(i2fl(i)/255.0f, 1.0f/Gr_gamma)*255.0f);
		if ( v > 255 ) {
			v = 255;
		} else if ( v < 0 )	{
			v = 0;
		}
		Gr_gamma_lookup[i] = v;
	}

//	ubyte new_pal[768];
//	if ( gr_screen.bits_per_pixel!=8 )	return;
//
//	for (i=0; i<768; i++ )	{
//		new_pal[i] = ubyte(Gr_gamma_lookup[gr_palette[i]]);
//	}
//	grx_change_palette( new_pal );

	gr_screen.signature = Gr_signature++;
}

void gr_soft_init()
{
//	int i;
	HWND hwnd = (HWND)os_get_window();
	
	// software mode only supports 640x480
	Assert(gr_screen.res == GR_640);
	if(gr_screen.res != GR_640){
		gr_screen.res = GR_640;
		gr_screen.max_w = 640;
		gr_screen.max_h = 480;
	}

	// Prepare the window to go full screen
	if ( hwnd )	{
		DWORD style, exstyle;
		RECT		client_rect;

		exstyle = 0;
		style = WS_CAPTION | WS_SYSMENU;
		
		//	Create Game Window
		client_rect.left = client_rect.top = 0;
		client_rect.right = gr_screen.max_w;
		client_rect.bottom = gr_screen.max_h;
		AdjustWindowRect(&client_rect,style,FALSE);

		RECT work_rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
		int x = work_rect.left + (( work_rect.right - work_rect.left )-(client_rect.right - client_rect.left))/2;
		int y = work_rect.top;
		if ( x < work_rect.left ) {
			x = work_rect.left;
		}
		int WinX = x;
		int WinY = y;
		int WinW = client_rect.right - client_rect.left;
		int WinH = client_rect.bottom - client_rect.top;

		ShowWindow(hwnd, SW_SHOWNORMAL );
		SetWindowLong( hwnd, GWL_STYLE, style );
		SetWindowLong( hwnd, GWL_EXSTYLE, exstyle );
		SetWindowPos( hwnd, HWND_NOTOPMOST, WinX, WinY, WinW, WinH, SWP_SHOWWINDOW );
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
	}

	Palette_flashed = 0;
	Palette_flashed_last_frame = 0;

	gr_screen.bits_per_pixel = 8;
	gr_screen.bytes_per_pixel = 1;

	gr_buffer_create( gr_screen.max_w, gr_screen.max_h, gr_screen.bits_per_pixel );

	gr_screen.offscreen_buffer_base = lpDibBits;

	gr_screen.rowsize = DibInfo.Header.biWidth*((gr_screen.bits_per_pixel+7)/8);
	Assert( DibInfo.Header.biWidth == gr_screen.max_w );

	if (DibInfo.Header.biHeight > 0)	{
		// top down 
		gr_screen.offscreen_buffer = (void *)((uint)gr_screen.offscreen_buffer_base + (gr_screen.max_h - 1) * gr_screen.rowsize);
		gr_screen.rowsize *= -1;
	} else {
		// top up
		gr_screen.offscreen_buffer = gr_screen.offscreen_buffer_base;
	}

	grx_init_alphacolors();

	gr_screen.gf_flip = grx_flip;
	gr_screen.gf_flip_window = grx_flip_window;
	gr_screen.gf_set_clip = grx_set_clip;
	gr_screen.gf_reset_clip = grx_reset_clip;
	gr_screen.gf_set_font = grx_set_font;
	gr_screen.gf_set_color = grx_set_color;
	gr_screen.gf_set_bitmap = grx_set_bitmap;
	gr_screen.gf_create_shader = grx_create_shader;
	gr_screen.gf_set_shader = grx_set_shader;
	gr_screen.gf_clear = grx_clear;
	// gr_screen.gf_bitmap = grx_bitmap;
	// ]gr_screen.gf_bitmap_ex = grx_bitmap_ex;

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

	gr_screen.gf_set_palette = grx_set_palette;
	gr_screen.gf_get_color = grx_get_color;
	gr_screen.gf_init_color = grx_init_color;
	gr_screen.gf_init_alphacolor = grx_init_alphacolor;
	gr_screen.gf_set_color_fast = grx_set_color_fast;
	gr_screen.gf_print_screen = grx_print_screen;
	gr_screen.gf_start_frame = grx_start_frame;
	gr_screen.gf_stop_frame = grx_stop_frame;

	gr_screen.gf_fade_in = grx_fade_in;
	gr_screen.gf_fade_out = grx_fade_out;
	gr_screen.gf_flash = grx_flash;


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
	gr_screen.gf_lock = gr_soft_lock;
	gr_screen.gf_unlock = gr_soft_unlock;

	// region
	gr_screen.gf_get_region = grx_get_region;

	// fog stuff
	gr_screen.gf_fog_set = gr_soft_fog_set;	

	// pixel get
	gr_screen.gf_get_pixel = gr_soft_get_pixel;

	// poly culling
	gr_screen.gf_set_cull = gr_soft_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_soft_cross_fade;

	// filter
	gr_screen.gf_filter_set = gr_soft_filter_set;

	// tcache set
	gr_screen.gf_tcache_set = gr_soft_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_soft_set_clear_color;

	gr_reset_clip();
	gr_clear();
	gr_flip();
}

void gr_soft_force_windowed()
{
}

void gr_soft_cleanup()
{
	if (Gr_soft_inited) {
		gr_buffer_release();
		Gr_soft_inited = 0;
	}
}

void grx_change_palette( ubyte * new_pal )
{
	if ( hPalette )	{
		if (hDibDC)
			SelectPalette( hDibDC, hOldPalette, FALSE );
		if (!DeleteObject(hPalette))
			Int3();
		hPalette = NULL;
	}

	hPalette = gr_create_palette_256(new_pal);	// All 256 mapped one-to-one, but BLT's are slow.

	if ( hDibDC )	{
		int i; 
		for (i=0; i<256; i++ )	{
			DibInfo.Colors.aColors[i].rgbRed = new_pal[i*3+0];
			DibInfo.Colors.aColors[i].rgbGreen = new_pal[i*3+1];
			DibInfo.Colors.aColors[i].rgbBlue = new_pal[i*3+2];
			DibInfo.Colors.aColors[i].rgbReserved = 0;
		}

		hOldPalette = SelectPalette( hDibDC, hPalette, FALSE );
		SetDIBColorTable( hDibDC, 0, 256, DibInfo.Colors.aColors );
	}
}

void grx_flash( int r, int g, int b )
{
	int t,i;
	ubyte new_pal[768];

	if ( (r==0) && (g==0) && (b==0) )	{
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

	grx_change_palette( new_pal );
}


static int gr_palette_faded_out = 0;

#define FADE_TIME (F1_0/4)		// How long to fade out

void grx_fade_out(int instantaneous)	
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
				grx_change_palette( new_pal );
				gr_flip();
				count++;

				t1 = timer_get_fixed_seconds() - start_time;

			} while ( (t1 < FADE_TIME) && (t1>=0) );		// Loop as long as time not up and timer hasn't rolled

			stop_time = timer_get_fixed_seconds();

			mprintf(( "Took %d frames (and %.1f secs) to fade out\n", count, f2fl(stop_time-start_time) ));
		
		}
		gr_palette_faded_out = 1;
	}

	gr_reset_clip();
	gr_clear();
	gr_flip();
	memset( new_pal, 0, 768 );
	grx_change_palette( new_pal );
#else
	Int3();
#endif
}


void grx_fade_in(int instantaneous)	
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
				grx_change_palette( new_pal );
				gr_flip();
				count++;

				t1 = timer_get_fixed_seconds() - start_time;

			} while ( (t1 < FADE_TIME) && (t1>=0) );		// Loop as long as time not up and timer hasn't rolled

			stop_time = timer_get_fixed_seconds();

			mprintf(( "Took %d frames (and %.1f secs) to fade in\n", count, f2fl(stop_time-start_time) ));
		}
		gr_palette_faded_out = 0;
	}

	memcpy( new_pal, gr_palette, 768 );
	grx_change_palette( new_pal );
#else 
	Int3();
#endif
}



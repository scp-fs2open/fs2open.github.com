/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrInternal.h $
 * $Revision: 2.5 $
 * $Date: 2004-11-21 11:27:31 $
 * $Author: taylor $
 *
 * Include file for our Graphics directory
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.3  2004/02/16 11:47:33  randomtiger
 * Removed a lot of files that we dont need anymore.
 * Changed htl to be on by default, command now -nohtl
 * Changed D3D to use a 2D vertex for 2D operations which should cut down on redundant data having to go though the system.
 * Added small change to all -start_mission flag to take you to any mission by filename, very useful for testing.
 * Removed old dshow code and took away timerbar compile flag condition since it uses a runtime flag now.
 *
 * Revision 2.2  2003/01/18 19:49:02  phreak
 * added tcache flag TCACHE_FLAG_COMPRESSED
 *
 * Revision 2.1  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 6     1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 5     1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 4     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 3     12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 23    5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 22    4/08/98 4:09p John
 * Fixed some potential bugs with high bit set in screen piointer.
 * 
 * 21    3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 20    3/24/98 3:58p John
 * Put in (hopefully) final gamma setting code.
 * 
 * 19    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 18    2/07/98 7:50p John
 * Added code so that we can use the old blending type of alphacolors if
 * we want to.  Made the stars use them.
 * 
 * 17    1/26/98 5:12p John
 * Added in code for Pentium Pro specific optimizations. Speed up
 * zbuffered correct tmapper about 35%.   Speed up non-zbuffered scalers
 * by about 25%.
 * 
 * 16    11/30/97 3:57p John
 * Made fixed 32-bpp translucency.  Made BmpMan always map translucent
 * color into 255 even if you aren't supposed to remap and make it's
 * palette black.
 * 
 * 15    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 14    11/29/97 2:06p John
 * added mode 16-bpp support
 * 
 * 13    11/14/97 12:30p John
 * Fixed some DirectX bugs.  Moved the 8-16 xlat tables into Graphics
 * libs.  Made 16-bpp DirectX modes know what bitmap format they're in.
 * 
 * 12    11/03/97 10:08p Hoffoss
 * Changed gr_get_string_size to utilize an optional length specifier, if
 * you want to use non-null terminated strings.
 * 
 * 11    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 10    10/14/97 4:50p John
 * more 16 bpp stuff.
 * 
 * 9     10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 8     7/18/97 12:40p John
 * cached alphacolors to disk.  Also made cfopen be able to delete a file
 * by passing NULL for mode.
 * 
 * 7     6/17/97 12:03p John
 * Moved color/alphacolor functions into their own module.  Made all color
 * functions be part of the low-level graphics drivers, not just the
 * grsoft.
 * 
 * 6     6/12/97 5:04p John
 * Initial rev of Glide support
 * 
 * 5     6/11/97 5:49p John
 * Changed palette code to only recalculate alphacolors when needed, not
 * when palette changes.
 * 
 * 4     6/06/97 4:41p John
 * Fixed alpha colors to be smoothly integrated into gr_set_color_fast
 * code.
 * 
 * 3     6/06/97 2:40p John
 * Made all the radar dim in/out
 * 
 * 2     5/14/97 4:38p John
 * Fixed print_screen bug.
 * 
 * 1     5/12/97 12:13p John
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _GRINTERNAL_H
#define _GRINTERNAL_H

#include "graphics/font.h"
#include "graphics/2d.h"
#include "graphics/grzbuffer.h"

extern int Gr_cursor;

#define GR_SCREEN_PTR(type,x,y) ((type *)(ptr_u(gr_screen.offscreen_buffer) + ptr_u(((x)+gr_screen.offset_x)*sizeof(type)) + ptr_u(((y)+gr_screen.offset_y)*gr_screen.rowsize)))
#define GR_SCREEN_PTR_SIZE(bpp,x,y) ((ptr_u)(ptr_u(gr_screen.offscreen_buffer) + ptr_u(((x)+gr_screen.offset_x)*(bpp)) + ptr_u(((y)+gr_screen.offset_y)*gr_screen.rowsize)))

extern ubyte Gr_original_palette[768];		// The palette 
extern ubyte Gr_current_palette[768];

typedef struct alphacolor {
	int	used;
	int	r,g,b,alpha;
	int	type;						// See AC_TYPE_??? define
	color	*clr;
	/*
	union {
		ubyte		lookup[16][256];		// For 8-bpp rendering modes
	} table;
	*/
} alphacolor;

// for backwards fred aabitmap compatibility
typedef struct alphacolor_old {
	int	used;
	int	r,g,b,alpha;
	int	type;						// See AC_TYPE_??? define
	color	*clr;	
	union {
		ubyte		lookup[16][256];		// For 8-bpp rendering modes
	} table;	
} alphacolor_old;

extern alphacolor * Current_alphacolor;
void gr_init_alphacolors();

extern char Gr_current_palette_name[128];

typedef struct color_gun {
	int	bits;
	int	shift;
	int	scale;
	int	mask;
} color_gun;

// screen format
extern color_gun Gr_red, Gr_green, Gr_blue, Gr_alpha;

// texture format
extern color_gun Gr_t_red, Gr_t_green, Gr_t_blue, Gr_t_alpha;

// alpha texture format
extern color_gun Gr_ta_red, Gr_ta_green, Gr_ta_blue, Gr_ta_alpha;

// CURRENT FORMAT - note - this is what bmpman uses when fiddling with pixels/colors. so be sure its properly set to one
// of the above values
extern color_gun *Gr_current_red, *Gr_current_green, *Gr_current_blue, *Gr_current_alpha;


// Translate the 768 byte 'src' palette into 
// the current screen format's palette.
// The size of the dst array is assumed to be gr_screen.bpp
// bytes per element.
void gr_xlat_palette( void *dst, bitmap *bmp );

// CPU identification variables
extern int Gr_cpu;			// What type of CPU.  5=Pentium, 6=Ppro/PII
extern int Gr_mmx;			// MMX capabilities?  0=No, 1=Yes

extern float Gr_gamma;
extern int Gr_gamma_int;				// int(Gr_gamma*100)
extern int Gr_gamma_lookup[256];

#define TCACHE_TYPE_AABITMAP				0		// HUD bitmap.  All Alpha.
#define TCACHE_TYPE_NORMAL					1		// Normal bitmap. Alpha = 0.
#define TCACHE_TYPE_XPARENT				2		// Bitmap with 0,255,0 = transparent.  Alpha=0 if transparent, 1 if not.
#define TCACHE_TYPE_NONDARKENING			3		// Bitmap with 255,255,255 = non-darkening.  Alpha=1 if non-darkening, 0 if not.
#define TCACHE_TYPE_BITMAP_SECTION		4		// section of a bitmap
#define TCACHE_TYPE_COMPRESSED			(1<<31)

extern int Ambient_r_default;
extern int Ambient_g_default;
extern int Ambient_b_default;

#endif


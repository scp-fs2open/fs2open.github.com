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
 * $Revision: 2.18 $
 * $Date: 2007-01-10 01:49:16 $
 * $Author: taylor $
 *
 * Include file for our Graphics directory
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.17  2007/01/07 13:13:39  taylor
 * cleanup various bits of obsolete or unused code
 *
 * Revision 2.16  2006/11/06 05:42:44  taylor
 * various bits of cleanup (slight reformatting to help readability, remove old/dead code bits, etc.)
 * deal with a index_buffer memory leak that Valgrind has always complained about
 * make HTL model buffers dynamic (get rid of MAX_BUFFERS_PER_SUBMODEL)
 * get rid of MAX_BUFFERS
 * make D3D vertex buffers dynamic, like OGL has already done
 *
 * Revision 2.15  2006/10/06 09:56:42  taylor
 * clean up some old software rendering stuff that we don't use any longer
 * remove grzbuffer.*, since all it did was give us 3 variables, which were moved to 2d.*
 *
 * Revision 2.14  2006/09/08 06:20:14  taylor
 * fix things that strict compiling balked at (from compiling with -ansi and -pedantic)
 *
 * Revision 2.13  2006/05/27 17:07:48  taylor
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
 * Revision 2.12  2006/05/13 07:29:52  taylor
 * OpenGL envmap support
 * newer OpenGL extension support
 * add GL_ARB_texture_rectangle support for non-power-of-2 textures as interface graphics
 * add cubemap reading and writing support to DDS loader
 * fix bug in DDS loader that made compressed images with mipmaps use more memory than they really required
 * add support for a default envmap named "cubemap.dds"
 * new mission flag "$Environment Map:" to use a pre-existing envmap
 * minor cleanup of compiler warning messages
 * get rid of wasteful math from gr_set_proj_matrix()
 * remove extra gr_set_*_matrix() calls from starfield.cpp as there was no longer a reason for them to be there
 * clean up bmpman flags in reguards to cubemaps and render targets
 * disable D3D envmap code until it can be upgraded to current level of code
 * remove bumpmap code from OpenGL stuff (sorry but it was getting in the way, if it was more than copy-paste it would be worth keeping)
 * replace gluPerspective() call with glFrustum() call, it's a lot less math this way and saves the extra function call
 *
 * Revision 2.11  2005/12/16 06:48:28  taylor
 * "House Keeping!!"
 *   - minor cleanup of things that have bothered me at one time or another
 *   - slight speedup from state switching
 *   - slightly better specmap handling, fixes a couple of (not frequent) strange and sorta random issues
 *   - make sure to only disable HTL arb stuff when in HTL mode
 *   - handle any extra lighting pass before spec pass so the light can be applied properly
 *
 * Revision 2.10  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.9  2005/04/24 12:56:42  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
 * Revision 2.8  2005/02/23 05:11:13  taylor
 * more consolidation of various graphics variables
 * some header cleaning
 * only one tmapper_internal for OGL, don't use more than two tex/pass now
 * seperate out 2d matrix mode to allow -2d_poof in OGL and maybe fix missing
 *    interface when set 3d matrix stuff doesn't have corresponding end
 * add dump_frame stuff for OGL, mostly useless but allows trailer recording
 *
 * Revision 2.7  2005/02/15 00:06:27  taylor
 * clean up some model related globals
 * code to disable individual thruster glows
 * fix issue where 1 extra OGL light pass didn't render
 *
 * Revision 2.6  2005/01/31 10:34:38  taylor
 * merge with Linux/OSX tree - p0131
 *
 * Revision 2.5  2004/11/21 11:27:31  taylor
 * some 64-bit OS comaptibility fixes
 *
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

#ifndef _GRINTERNAL_H
#define _GRINTERNAL_H


#include "graphics/font.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h" // IAM_64BIT
#include "globalincs/globals.h" // just in case pstypes.h messed up

extern int Gr_cursor;

//#define GR_SCREEN_PTR(type,x,y) ((type *)(ptr_u(gr_screen.offscreen_buffer) + ptr_u(((x)+gr_screen.offset_x)*sizeof(type)) + ptr_u(((y)+gr_screen.offset_y)*gr_screen.rowsize)))
//#define GR_SCREEN_PTR_SIZE(bpp,x,y) ((ptr_u)(ptr_u(gr_screen.offscreen_buffer) + ptr_u(((x)+gr_screen.offset_x)*(bpp)) + ptr_u(((y)+gr_screen.offset_y)*gr_screen.rowsize)))

extern ubyte Gr_original_palette[768];		// The palette 
extern ubyte Gr_current_palette[768];

/*typedef struct alphacolor {
	int	used;
	int	r,g,b,alpha;
	int	type;						// See AC_TYPE_??? define
	color	*clr;

//	union {
//		ubyte		lookup[16][256];		// For 8-bpp rendering modes
//	} table;
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
void gr_init_alphacolors();*/

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
//void gr_xlat_palette( void *dst, bitmap *bmp );

// CPU identification variables
//extern int Gr_cpu;			// What type of CPU.  5=Pentium, 6=Ppro/PII
//extern int Gr_mmx;			// MMX capabilities?  0=No, 1=Yes

extern float Gr_gamma;
extern int Gr_gamma_int;				// int(Gr_gamma*100)

#define TCACHE_TYPE_AABITMAP				0		// HUD bitmap.  All Alpha.
#define TCACHE_TYPE_NORMAL					1		// Normal bitmap. Alpha = 0.
#define TCACHE_TYPE_XPARENT					2		// Bitmap with 0,255,0 = transparent.  Alpha=0 if transparent, 1 if not.
#define TCACHE_TYPE_INTERFACE				3		// for graphics that are using in the interface (for special filtering or sizing)
#define TCACHE_TYPE_COMPRESSED				4		// Compressed bitmap type (DXT1, DXT3, DXT5)
#define TCACHE_TYPE_CUBEMAP					5

//extern int Ambient_r_default;
//extern int Ambient_g_default;
//extern int Ambient_b_default;

#define NEBULA_COLORS 20

typedef enum gr_texture_source {
	TEXTURE_SOURCE_NONE,
	TEXTURE_SOURCE_DECAL,
	TEXTURE_SOURCE_NO_FILTERING,
	TEXTURE_SOURCE_MODULATE4X
} gr_texture_source;

typedef enum gr_alpha_blend {
	ALPHA_BLEND_NONE,					// 1*SrcPixel + 0*DestPixel
	ALPHA_BLEND_ADDITIVE,				// 1*SrcPixel + 1*DestPixel
	ALPHA_BLEND_ALPHA_ADDITIVE,			// Alpha*SrcPixel + 1*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_ALPHA,		// Alpha*SrcPixel + (1-Alpha)*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
} gr_alpha_blend;

typedef enum gr_zbuffer_type {
	ZBUFFER_TYPE_NONE,
	ZBUFFER_TYPE_READ,
	ZBUFFER_TYPE_WRITE,
	ZBUFFER_TYPE_FULL,
	ZBUFFER_TYPE_DEFAULT
} gr_zbuffer_type;


#endif

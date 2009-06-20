/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _GRINTERNAL_H
#define _GRINTERNAL_H


#include "graphics/font.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h" // IAM_64BIT
#include "globalincs/globals.h" // just in case pstypes.h messed up

extern int Gr_cursor;
extern int Gr_cursor_size;

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

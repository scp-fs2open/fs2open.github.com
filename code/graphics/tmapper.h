/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _TMAPPER_H
#define _TMAPPER_H

#include "globalincs/pstypes.h"

/*
struct vertex;

// call this to reinit the scanline function pointers.
extern void tmapper_setup();

// Used to tell the tmapper what the current lighting values are
// if the TMAP_FLAG_RAMP or TMAP_FLAG_RGB are set and the TMAP_FLAG_GOURAUD 
// isn't set.   
void tmapper_set_light(vertex *v, uint flags);

// DO NOT CALL grx_tmapper DIRECTLY!!!! Only use the 
// gr_tmapper equivalent!!!!
extern void grx_tmapper( int nv, vertex * verts[], uint flags );
*/

#define TMAP_MAX_VERTS	25		// Max number of vertices per polygon

// Flags to pass to g3_draw_??? routines
#define TMAP_FLAG_TEXTURED			(1<<0)	// Uses texturing (Interpolate uv's)
#define TMAP_FLAG_CORRECT			(1<<1)	// Perspective correct (Interpolate sw)
#define TMAP_FLAG_RAMP				(1<<2)	// Use RAMP lighting (interpolate L)
#define TMAP_FLAG_RGB				(1<<3)	// Use RGB lighting (interpolate RGB)
#define TMAP_FLAG_GOURAUD			(1<<4)	// Lighting values differ on each vertex. 
											//   If this is not set, then the texture mapper will use
											//   the lighting parameters in each vertex, otherwise it
											//   will use the ones specified in tmapper_set_??
#define TMAP_FLAG_XPARENT			(1<<5)	// texture could have transparency
#define TMAP_FLAG_TILED				(1<<6)	// This means uv's can be > 1.0
#define TMAP_FLAG_NEBULA			(1<<7)	// Must be used with RAMP and GOURAUD.  Means l 0-1 is 0-31 palette entries

//#define TMAP_HIGHEST_FLAG_BIT		7		// The highest bit used in the TMAP_FLAGS
//#define TMAP_MAX_SCANLINES		(1<<(TMAP_HIGHEST_FLAG_BIT+1))

// Add any entries that don't work for software under here:
// Make sure to disable them at top of grx_tmapper
#define TMAP_FLAG_ALPHA				(1<<8)	// Has an alpha component
#define TMAP_FLAG_BATCH_TRANSFORMS	(1<<9)	// Use batched transform data transmitted via texture/uniform buffer

// Interface specific stuff (for separate filtering, sizing, etc.), replaces old TMAP_FLAG_BITMAP_SECTION 
#define TMAP_FLAG_INTERFACE			(1<<10)

// flags for full nebula effect
#define TMAP_FLAG_PIXEL_FOG			(1<<11)	// fog the polygon based upon the average pixel colors of the backbuffer behind it

// RT Flags added to determine whats being drawn for HT&L
#define TMAP_HTL_3D_UNLIT			(1<<12)
#define TMAP_HTL_2D					(1<<13)		// I don't think this flag is being used (Swifty)

//tristrips, for trails mostly, might find other uses eventualy
#define TMAP_FLAG_TRISTRIP			(1<<14)
#define TMAP_FLAG_TRILIST			(1<<15)
#define TMAP_FLAG_QUADLIST			(1<<16)
#define TMAP_FLAG_QUADSTRIP			(1<<17)

// use greyscale texture
#define TMAP_FLAG_BW_TEXTURE		(1<<18)

// use animated Shader - Valathil
#define TMAP_ANIMATED_SHADER		(1<<19)

// use soft particle shader - Swifty
#define TMAP_FLAG_SOFT_QUAD			(1<<20)

// use framebuffer distortion mapping with generated distortion map - Valathil
#define TMAP_FLAG_DISTORTION_THRUSTER	(1<<21)

// use framebuffer distortion mapping  - Valathil
#define TMAP_FLAG_DISTORTION		(1<<22)

#define TMAP_FLAG_DESATURATE		(1<<23)

#define TMAP_FLAG_POINTLIST			(1<<24)
#define TMAP_FLAG_LINESTRIP			(1<<25)
#define TMAP_FLAG_LINES				(1<<26)
#define TMAP_FLAG_VERTEX_GEN		(1<<27)
#define TMAP_FLAG_EMISSIVE			(1<<28)

#define TMAP_ADDRESS_WRAP			1
#define TMAP_ADDRESS_MIRROR			2
#define TMAP_ADDRESS_CLAMP			3

//WMC - moved this here so it'd be in 2d.h and 3d.h
//bitmap_2d_list, 
//x and y: the 2d position of the upper left hand corner
//w and h: the hidth and hight of the bitmap (some functions 
//will overide these, others will only overide if givein an invalid size like 0 or -1)
struct bitmap_2d_list{
	bitmap_2d_list(int X=0, int Y=0, int W=-1, int H=-1):x(X),y(Y),w(W),h(H){}
	int x;
	int y;
	int w;
	int h;
};

//texture_rect
//defines a rectangular reagon within a texture
//similar to the above structure only all values are relitive 
//from 0,0 in the upper left to 1,1 in the lowwer right
//out of range values are valid
struct texture_rect_list{
	texture_rect_list(float u0In=0.0f, float v0In=0.0f, float u1In=1.0f, float v1In=1.0f):u0(u0In),v0(v0In),u1(u1In),v1(v1In){}
	float u0;
	float v0;
	float u1;
	float v1;
};

struct bitmap_rect_list{
	bitmap_rect_list(float X, float Y, float W, float H):texture_rect(X,Y,W,H){}
	bitmap_rect_list(int X=0, int Y=0, int W=-1, int H=-1, float TX=0.0f, float TY=0.0f, float TW=1.0f, float TH=1.0f):screen_rect(X,Y,W,H),texture_rect(TX,TY,TW,TH){}
	bitmap_2d_list screen_rect;
	texture_rect_list texture_rect;
};

#endif

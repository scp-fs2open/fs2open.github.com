/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3DInternal.h $
 * $Revision: 2.2 $
 * $Date: 2002-10-05 16:46:09 $
 * $Author: randomtiger $
 *
 * Prototypes for the variables used internally by the Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
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
 * 5     7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 4     7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 3     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 21    5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 20    5/20/98 9:45p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 19    5/12/98 10:34a John
 * Added d3d_shade functionality.  Added d3d_flush function, since the
 * shader seems to get reorganzed behind the overlay text stuff!
 * 
 * 18    5/12/98 8:18a John
 * Put in code to use a different texture format for alpha textures and
 * normal textures.   Turned off filtering for aabitmaps.  Took out
 * destblend=invsrccolor alpha mode that doesn't work on riva128. 
 * 
 * 17    5/11/98 10:19a John
 * Added caps checking
 * 
 * 16    5/07/98 3:02p John
 * Mpre texture cleanup.   You can now reinit d3d without a crash.
 * 
 * 15    5/07/98 9:54a John
 * Added in palette flash functionallity.
 * 
 * 14    5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 13    5/06/98 8:41p John
 * Fixed some font clipping bugs.   Moved texture handle set code for d3d
 * into the texture module.
 * 
 * 12    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 11    5/05/98 10:37p John
 * Added code to optionally use execute buffers.
 * 
 * 10    5/04/98 3:36p John
 * Got zbuffering working with Direct3D.
 * 
 * 9     4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 8     3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 7     3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 6     3/09/98 6:06p John
 * Restructured font stuff to avoid duplicate code in Direct3D and Glide.
 * Restructured Glide to avoid redundent state setting.
 * 
 * 5     3/08/98 12:33p John
 * Made d3d cleanup free textures.  Made d3d always divide texture size by
 * 2 for now.
 * 
 * 4     3/07/98 8:29p John
 * Put in some Direct3D features.  Transparency on bitmaps.  Made fonts &
 * aabitmaps render nice.
 * 
 * 3     2/17/98 7:28p John
 * Got fonts and texturing working in Direct3D
 * 
 * 2     2/07/98 7:50p John
 * Added code so that we can use the old blending type of alphacolors if
 * we want to.  Made the stars use them.
 * 
 * 1     2/03/98 9:24p John
 *
 * $NoKeywords: $
 */


#ifndef _GRD3DINTERNAL_H
#define _GRD3DINTERNAL_H

#include <windows.h>
#include <windowsx.h>

#define D3D_OVERLOADS
#include "directx/vddraw.h"

// To remove an otherwise well-lodged compiler error
// 4201 nonstandard extension used : nameless struct/union (happens a lot in Windows include headers)
#pragma warning(disable: 4201)

#include "directx/vd3d.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"


extern LPDIRECTDRAW			lpDD1;
extern LPDIRECTDRAW2			lpDD;
extern LPDIRECT3D2			lpD3D;
extern LPDIRECT3DDEVICE		lpD3DDeviceEB; 
extern LPDIRECT3DDEVICE2	lpD3DDevice; 
extern LPDIRECTDRAWSURFACE	lpBackBuffer;
extern LPDIRECTDRAWSURFACE	lpFrontBuffer;
extern LPDIRECTDRAWSURFACE	lpZBuffer;

extern LPDIRECT3DVIEWPORT2	lpViewport;
extern LPDIRECTDRAWPALETTE	lpPalette;

extern DDPIXELFORMAT			AlphaTextureFormat;
extern DDPIXELFORMAT			NonAlphaTextureFormat;
extern DDPIXELFORMAT			ScreenFormat;

extern D3DDEVICEDESC D3DHWDevDesc, D3DHELDevDesc;
extern LPD3DDEVICEDESC lpDevDesc;
extern DDCAPS DD_driver_caps;
extern DDCAPS DD_hel_caps;

extern int D3D_texture_divider;

extern int D3D_32bit;

extern char* d3d_error_string(HRESULT error);

void d3d_tcache_init(int use_sections);
void d3d_tcache_cleanup();
void d3d_tcache_flush();
void d3d_tcache_frame();

// Flushes any pending operations
void d3d_flush();

int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int sx = -1, int sy = -1, int force = 0);

// Functions in GrD3DRender.cpp stuffed into gr_screen structure

void gr_d3d_flash(int r, int g, int b);
void gr_d3d_zbuffer_clear(int mode);
int gr_d3d_zbuffer_get();
int gr_d3d_zbuffer_set(int mode);
void gr_d3d_tmapper( int nverts, vertex **verts, uint flags );
void gr_d3d_scaler(vertex *va, vertex *vb );
void gr_d3d_aascaler(vertex *va, vertex *vb );
void gr_d3d_pixel(int x, int y);
void gr_d3d_clear();
void gr_d3d_set_clip(int x,int y,int w,int h);
void gr_d3d_reset_clip();
void gr_d3d_init_color(color *c, int r, int g, int b);
void gr_d3d_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type );
void gr_d3d_set_color( int r, int g, int b );
void gr_d3d_get_color( int * r, int * g, int * b );
void gr_d3d_set_color_fast(color *dst);
void gr_d3d_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx=-1, int sy=-1 );
void gr_d3d_bitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d_bitmap(int x, int y);
void gr_d3d_aabitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d_aabitmap(int x, int y);
void gr_d3d_rect(int x,int y,int w,int h);
void gr_d3d_create_shader(shader * shade, float r, float g, float b, float c );
void gr_d3d_set_shader( shader * shade );
void gr_d3d_shade(int x,int y,int w,int h);
void gr_d3d_create_font_bitmap();
void gr_d3d_char(int x,int y,int letter);
void gr_d3d_string( int sx, int sy, char *s );
void gr_d3d_circle( int xc, int yc, int d );
void gr_d3d_line(int x1,int y1,int x2,int y2);
void gr_d3d_aaline(vertex *v1, vertex *v2);
void gr_d3d_gradient(int x1,int y1,int x2,int y2);
void gr_d3d_set_palette(ubyte *new_palette, int restrict_alphacolor);
void gr_d3d_diamond(int x, int y, int width, int height);
void gr_d3d_print_screen(char *filename);

void d3d_render_timer_bar(int colour, float x, float y, float w, float h);

// Functions used to render.  Calls either DrawPrim or Execute buffer code
HRESULT d3d_SetRenderState( D3DRENDERSTATETYPE dwRenderStateType,  DWORD dwRenderState );
HRESULT d3d_DrawPrimitive( D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags );

#endif //_GRD3DINTERNAL_H

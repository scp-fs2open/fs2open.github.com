/*
	Prototypes for variables used by the D3D 8.1 renderer.
	Note, because there are gonna be some changes to the way things are done, in this header
	particularly we will not be mirroring the [V] headers.
*/

#ifndef _GRD3D81INTERNAL_H
#define _GRD3D81INTERNAL_H

#include <windows.h>
#include <windowsx.h>

#include <d3d8.h>
#include <d3dx8.h>			// This is the Dx8.1 utility library. Automates some functions and
							// provides the D3D_OVERLOADS functionality
#include "graphics/2d.h"
#include "graphics/grinternal.h"

/*				!!! All of these need to be replaced with up to date versions
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
*/
extern int D3D8_texture_divider;

extern int D3D8_32bit;

extern char* d3d8_error_string(HRESULT error);

void d3d8_tcache_init(int use_sections);
void d3d8_tcache_cleanup();
void d3d8_tcache_flush();
void d3d8_tcache_frame();

// Flushes any pending operations
void d3d8_flush();

int d3d8_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int sx = -1, int sy = -1, int force = 0);

// Functions in GrD3DRender.cpp stuffed into gr_screen structure
void gr_d3d8_flash(int r, int g, int b);
void gr_d3d8_zbuffer_clear(int mode);
int gr_d3d8_zbuffer_get();
int gr_d3d8_zbuffer_set(int mode);
void gr_d3d8_tmapper( int nverts, vertex **verts, uint flags );
void gr_d3d8_scaler(vertex *va, vertex *vb );
void gr_d3d8_aascaler(vertex *va, vertex *vb );
void gr_d3d8_pixel(int x, int y);
void gr_d3d8_clear();
void gr_d3d8_set_clip(int x,int y,int w,int h);
void gr_d3d8_reset_clip();
void gr_d3d8_init_color(color *c, int r, int g, int b);
void gr_d3d8_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type );
void gr_d3d8_set_color( int r, int g, int b );
void gr_d3d8_get_color( int * r, int * g, int * b );
void gr_d3d8_set_color_fast(color *dst);
void gr_d3d8_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx=-1, int sy=-1 );
void gr_d3d8_bitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d8_bitmap(int x, int y);
void gr_d3d8_aabitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d8_aabitmap(int x, int y);
void gr_d3d8_rect(int x,int y,int w,int h);
void gr_d3d8_create_shader(shader * shade, float r, float g, float b, float c );
void gr_d3d8_set_shader( shader * shade );
void gr_d3d8_shade(int x,int y,int w,int h);
void gr_d3d8_create_font_bitmap();
void gr_d3d8_char(int x,int y,int letter);
void gr_d3d8_string( int sx, int sy, char *s );
void gr_d3d8_circle( int xc, int yc, int d );
void gr_d3d8_line(int x1,int y1,int x2,int y2);
void gr_d3d8_aaline(vertex *v1, vertex *v2);
void gr_d3d8_gradient(int x1,int y1,int x2,int y2);
void gr_d3d8_set_palette(ubyte *new_palette, int restrict_alphacolor);
void gr_d3d8_diamond(int x, int y, int width, int height);
void gr_d3d8_print_screen(char *filename);


// Functions used to render.  Calls either DrawPrim or Execute buffer code
HRESULT d3d8_SetRenderState( D3DRENDERSTATETYPE dwRenderStateType,  DWORD dwRenderState );
HRESULT d3d8_DrawPrimitive( D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags );

#endif // _GRD3D81INTERNAL_H

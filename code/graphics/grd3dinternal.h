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
 * $Revision: 2.6 $
 * $Date: 2003-08-22 07:35:08 $
 * $Author: bobboau $
 *
 * Prototypes for the variables used internally by the Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2003/08/16 03:52:23  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.4  2003/07/04 02:27:48  phreak
 * added support for cloaking.
 * i will need to contact someone who knows d3d to get this to work
 *
 * Revision 2.3  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.2  2002/10/05 16:46:09  randomtiger
 * Added us fs2_open people to the credits. Worth looking at just for that.
 * Added timer bar code, by default its not compiled in.
 * Use TIMEBAR_ACTIVE in project and dependancy code settings to activate.
 * Added the new timebar files with the new code.
 *
 * Revision 2.1.2.9  2002/11/11 21:26:04  randomtiger
 *
 * Tided up D3DX8 calls, did some documentation and add new file: grd3dcalls.cpp. - RT
 *
 * Revision 2.1.2.8  2002/11/04 21:24:59  randomtiger
 *
 * When running in D3D all ani's are memory mapped for speed, this takes up more memory but stops gametime locking of textures which D3D8 hates.
 * Added new command line tag Cmdline_d3dlowmem for people who dont want to make use of this because they have no memory.
 * Cleaned up some more texture stuff enabled console debug for D3D.
 *
 * Revision 2.1.2.7  2002/10/30 22:57:21  randomtiger
 *
 * Changed DX8 code to not use set render and texture states if they are already set to that value.
 * Disabled buffer saving and restoring code when windowed to stop DX8 debug runs from crashing. - RT
 *
 * Revision 2.1.2.6  2002/10/21 16:33:41  randomtiger
 * Added D3D only 32 bit TGA functionality. Will load a texture as big as your graphics card allows. Code not finished yet and will forge the beginnings of the new texture system. - RT
 *
 * Revision 2.1.2.5  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
 *
 * Revision 2.1.2.4  2002/10/04 00:48:42  randomtiger
 * Fixed video memory leaks
 * Added code to cope with lost device, not tested
 * Got rid of some DX5 stuff we definately dont need
 * Moved some enum's into internal,h because gr_d3d_set_state should be able to be called from any dx file
 * Cleaned up some stuff - RT
 *
 * Revision 2.1.2.3  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.1.2.2  2002/09/28 22:13:43  randomtiger
 * Sorted out some bits and pieces. The background nebula blends now which is nice. – RT
 *
 * Revision 2.1.2.1  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
>>>>>>> 2.1.2.9
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

#include "directx/vddraw.h"

// To remove an otherwise well-lodged compiler error
// 4201 nonstandard extension used : nameless struct/union (happens a lot in Windows include headers)
#pragma warning(disable: 4201)

#include <d3d8.h>
#include "graphics/2d.h"
#include "graphics/grinternal.h"

extern IDirect3D8 *lpD3D;
extern IDirect3DDevice8 *lpD3DDevice;	 
extern D3DVIEWPORT8 viewport;
extern D3DCAPS8 d3d_caps;
extern D3DPRESENT_PARAMETERS d3dpp; 

extern int D3D_texture_divider;
extern int D3D_32bit;

extern const char* d3d_error_string(HRESULT error);

void set_stage_for_defuse();
void set_stage_for_glow_mapped_defuse();
void set_stage_for_defuse_and_non_mapped_spec();
void set_stage_for_glow_mapped_defuse_and_non_mapped_spec();
bool set_stage_for_spec_mapped();
void set_stage_for_cell_shaded();
void set_stage_for_cell_glowmapped_shaded();
void set_stage_for_additive_glowmapped();

void d3d_tcache_init();
void d3d_tcache_cleanup();
void d3d_tcache_flush();
void d3d_tcache_frame();

// Flushes any pending operations
void d3d_flush();

int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int sx = -1, int sy = -1, int force = 0);
int d3d_tcache_set_internal(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int sx = -1, int sy = -1, int force = 0, int stage = 0);

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
void gr_d3d_push_texture_matrix(int unit);
void gr_d3d_pop_texture_matrix(int unit);
void gr_d3d_translate_texture_matrix(int unit, vector *shift);

void d3d_render_timer_bar(int colour, float x, float y, float w, float h);

enum
{
	D3DVT_TLVERTEX,
	D3DVT_LVERTEX,
	D3DVT_VERTEX,
	D3DVT_MAX
};

// This vertex type tells D3D that it has already been transformed an lit
// D3D will simply display the polygon with no extra processing
typedef struct { 
    float sx, sy, sz; 
	float rhw; 

    DWORD color; 
    DWORD specular; 
    float tu, tv; 
    float env_u, env_v; 

} D3DTLVERTEX;

// This vertex type should be used for vertices that have already been lit
// make sure lighting is set to off while these polygons are rendered 
typedef struct { 
    float sx, sy, sz;
  
    DWORD color; 
    DWORD specular; 
    float tu, tv; 

} D3DLVERTEX;

// Renders a normal polygon that is to be transformed and lit by D3D
typedef struct { 
    float sx, sy, sz;
  	float nx, ny, nz;

    float tu, tv; 

} D3DVERTEX;

typedef struct {
	int fvf;
	int size;

} VertexTypeInfo;

typedef float D3DVALUE;

// 32 bit formats, only for use in a 32 bit mode.
extern D3DFORMAT default_32_non_alpha_tformat;
extern D3DFORMAT default_32_alpha_tformat;

// 16 bit formats for pcx media
extern D3DFORMAT default_non_alpha_tformat;
extern D3DFORMAT default_alpha_tformat;

extern DDPIXELFORMAT AlphaTextureFormat;
extern DDPIXELFORMAT NonAlphaTextureFormat;

typedef enum gr_texture_source {
	TEXTURE_SOURCE_NONE,
	TEXTURE_SOURCE_DECAL,
	TEXTURE_SOURCE_NO_FILTERING,
} gr_texture_source;

typedef enum gr_alpha_blend {
	ALPHA_BLEND_NONE,							// 1*SrcPixel + 0*DestPixel
	ALPHA_BLEND_ALPHA_ADDITIVE,			// Alpha*SrcPixel + 1*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_ALPHA,		// Alpha*SrcPixel + (1-Alpha)*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR,	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
} gr_alpha_blend;

typedef enum gr_zbuffer_type {
	ZBUFFER_TYPE_NONE,
	ZBUFFER_TYPE_READ,
	ZBUFFER_TYPE_WRITE,
	ZBUFFER_TYPE_FULL,
	ZBUFFER_TYPE_DEFAULT,
} gr_zbuffer_type;

// INTERNAL D3D Functions

// GrD3D functions
void d3d_set_initial_render_state();
int d3d_get_mode_bit(D3DFORMAT type);

// GrD3DRender functions
void gr_d3d_set_state( gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt );

// GrD3DTexture functions

// GrD3DCall functions
void d3d_reset_render_states();
HRESULT d3d_SetRenderState( D3DRENDERSTATETYPE render_state_type,  DWORD render_state );
HRESULT d3d_DrawPrimitive(int vertex_type, D3DPRIMITIVETYPE prim_type, LPVOID pvertices, DWORD vertex_count);
void d3d_reset_texture_stage_states();
HRESULT d3d_SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
void d3d_lost_device();
HRESULT d3d_SetTexture(int stage, IDirect3DBaseTexture8* texture_ptr);

#endif //_GRD3DINTERNAL_H

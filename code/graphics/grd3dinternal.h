/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _GRD3DINTERNAL_H
#define _GRD3DINTERNAL_H

#ifndef NO_DIRECT3D

#include <windows.h>

#include <d3d8.h>
#include "graphics/2d.h"
#include "graphics/grinternal.h"


// easy macro for doing a Release() on d3d stuff
// we give release three chances and Assert() if there is still a reference
//   x	==  the texture/surface/etc. that you want to release
//   i  ==  the integer that will get the ref count (must be declared in calling function)
#define D3D_RELEASE(x, i)	{		\
	(i) = 0;						\
	if ( (x) != NULL ) {			\
		(i) = (x)->Release();		\
		if ( (i) > 0 ) {			\
			(i) = (x)->Release();	\
		}							\
		if ( (i) > 0 ) {			\
			(i) = (x)->Release();	\
		}							\
		Assert( (i) < 1 );			\
		(x) = NULL;					\
	}								\
}

/* Structures */

// Keeping the globals in here now to try and keep some order
typedef struct {

	static IDirect3D8 *lpD3D;
	static IDirect3DDevice8 *lpD3DDevice;
	static D3DCAPS8 d3d_caps;
	static D3DPRESENT_PARAMETERS d3dpp; 

	static bool D3D_inited;
	static bool D3D_activate;
	static bool D3D_Antialiasing;
	static bool D3D_window;

	static int D3D_rendition_uvs;
	static int D3D_zbias;

	static int unlit_3D_batch;

	static float texture_adjust_u;
	static float texture_adjust_v;

} GlobalD3DVars;

/*
 * Stolen definion of DDPIXELFORMAT
 */
typedef struct
{
    bool	dw_is_alpha_mode;	// pixel format flags
	DWORD	dwRGBBitCount;		// how many bits per pixel
	DWORD	dwRBitMask;			// mask for red bit
	DWORD	dwGBitMask;			// mask for green bits
	DWORD	dwBBitMask;			// mask for blue bits
	DWORD	dwRGBAlphaBitMask;	// mask for alpha channel

} PIXELFORMAT;

typedef struct tcache_slot_d3d {

	IDirect3DTexture8 *d3d8_thandle;

	float						u_scale, v_scale;
	int							bitmap_id;
	int							size;
	char						used_this_frame;
	int							time_created;
	ushort						w, h;
} tcache_slot_d3d;

// This vertex type tells D3D that it has already been transformed an lit
// D3D will simply display the polygon with no extra processing
typedef struct { 
    float sx, sy, sz; 
	float rhw; 
    DWORD color; 
    float tu, tv; 

} D3DVERTEX2D;

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
//(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)
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

    float tu, tv, tu2, tv2; 

} D3DVERTEX;

typedef struct {
	float x, y, z;
	float size;
	// Warning this custom value is not in use by D3D
	DWORD custom;
} D3DPOINTVERTEX;

typedef struct {
	int fvf;
	int size;

} VertexTypeInfo;

/* Enums and typedefs */

enum
{
	D3DVT_VERTEX2D,
	D3DVT_TLVERTEX,
	D3DVT_LVERTEX,
	D3DVT_VERTEX,
	D3DVT_PVERTEX,
	D3DVT_MAX
};

typedef float D3DVALUE;

/* External vars - booo! */

// 16 bit formats for pcx media
extern D3DFORMAT default_non_alpha_tformat;
extern D3DFORMAT default_alpha_tformat;
extern D3DFORMAT default_compressed_format;

extern PIXELFORMAT AlphaTextureFormat;
extern PIXELFORMAT NonAlphaTextureFormat;

/* Functions */
/*
void set_stage_for_defuse();
void set_stage_for_glow_mapped_defuse();
void set_stage_for_defuse_and_non_mapped_spec();
void set_stage_for_glow_mapped_defuse_and_non_mapped_spec();
bool set_stage_for_spec_mapped();
void set_stage_for_cell_shaded();
void set_stage_for_cell_glowmapped_shaded();
void set_stage_for_additive_glowmapped();
*/
void d3d_tcache_init();
void d3d_tcache_cleanup();
void d3d_tcache_flush();
void d3d_tcache_frame();

void d3d_flush();

int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int force = 0, int stage = 0);
int d3d_tcache_set_internal(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int force = 0, int stage = 0);

// Functions in GrD3DRender.cpp stuffed into gr_screen structure
void gr_d3d_flash(int r, int g, int b);
void gr_d3d_flash_alpha(int r, int g, int b, int a);
void gr_d3d_zbuffer_clear(int mode);
int gr_d3d_zbuffer_get();
int gr_d3d_zbuffer_set(int mode);
void gr_d3d_tmapper( int nverts, vertex **verts, uint flags );
void gr_d3d_scaler(vertex *va, vertex *vb );
void gr_d3d_aascaler(vertex *va, vertex *vb );
void gr_d3d_pixel(int x, int y, bool resize);
void gr_d3d_clear();
void gr_d3d_set_clip(int x,int y,int w,int h,bool resize);
void gr_d3d_reset_clip();
void gr_d3d_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha );
void gr_d3d_bitmap(int x, int y);
void gr_d3d_bitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize);
void gr_d3d_aabitmap_ex(int x,int y,int w,int h,int sx,int sy,bool resize, bool mirror);
void gr_d3d_aabitmap(int x, int y, bool resize, bool mirror);
//void gr_d3d_rect(int x,int y,int w,int h,bool resize);
//void gr_d3d_shade(int x,int y,int w,int h);
void gr_d3d_create_font_bitmap();
void gr_d3d_char(int x,int y,int letter);
void gr_d3d_string( int sx, int sy, char *s, bool resize = true );
void gr_d3d_circle( int xc, int yc, int d, bool resize = true );
void gr_d3d_curve( int xc, int yc, int r, int direction );
void gr_d3d_line(int x1,int y1,int x2,int y2, bool resize = true);
void gr_d3d_aaline(vertex *v1, vertex *v2);
void gr_d3d_gradient(int x1,int y1,int x2,int y2, bool resize = true);
void gr_d3d_set_palette(ubyte *new_palette, int restrict_alphacolor);
void gr_d3d_diamond(int x, int y, int width, int height);
void gr_d3d_print_screen(char *filename);
void gr_d3d_push_texture_matrix(int unit);
void gr_d3d_pop_texture_matrix(int unit);
void gr_d3d_translate_texture_matrix(int unit, vec3d *shift);
void gr_d3d_zbias(int zbias);
void gr_d3d_set_fill_mode(int mode);

void gr_d3d_draw_htl_line(vec3d *start, vec3d* end);
void gr_d3d_draw_htl_sphere(float rad);

void d3d_render_timer_bar(int colour, float x, float y, float w, float h);

// GrD3DRender functions
void gr_d3d_set_state( gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt );
void gr_d3d_tmapper_internal_batch_3d_unlit( int nverts, vertex *verts, uint flags);	

// GrD3DCall functions
void d3d_reset_render_states();
HRESULT d3d_SetRenderState( D3DRENDERSTATETYPE render_state_type,  DWORD render_state, bool set = true, bool init = false );
HRESULT d3d_DrawPrimitive(int vertex_type, D3DPRIMITIVETYPE prim_type, LPVOID pvertices, DWORD vertex_count);
void d3d_reset_texture_stage_states();
HRESULT d3d_SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value, bool set = true, bool init = false);
BOOL d3d_lost_device(bool force = false);
HRESULT d3d_SetTexture(int stage, IDirect3DBaseTexture8* texture_ptr);
HRESULT d3d_SetVertexShader(uint vertex_type);
HRESULT d3d_CreateVertexBuffer(int vertex_type, int size, DWORD usage, void **buffer);
int d3d_get_num_prims(int vertex_count, D3DPRIMITIVETYPE prim_type);

// GrD3Dtexture
int gr_d3d_preload(int bitmap_num, int is_aabitmap );

bool d3d_init_light();
int d3d_create_texture(int bitmap_handle, int bitmap_type, tcache_slot_d3d *tslot, int fail_on_full);

extern D3DCOLOR ambient_light;
extern VertexTypeInfo vertex_types[];

struct dynamic_buffer{
	dynamic_buffer():size(0),buffer(NULL){}
	~dynamic_buffer(){if(buffer)buffer->Release();}

	void allocate(int n_verts, int type);
	void allocate(int n_verts, uint _fvf, int _size);

	void lock(ubyte**v, int v_type){
		fvf = vertex_types[v_type].fvf;
		vsize = vertex_types[v_type].size;
		buffer->Lock(0, 0, v, D3DLOCK_DISCARD);
	}
	void lock(ubyte**v, uint FVF, int SIZE){
		fvf = FVF;
		vsize = SIZE;
		buffer->Lock(0, 0, v, D3DLOCK_DISCARD);
	}
	void unlock();
	void lost(){
		if(buffer)buffer->Release();
		size = 0;
		buffer = NULL;
	}
	void draw(_D3DPRIMITIVETYPE TYPE, int num);
	uint fvf;
	int vsize;
	int size;
	IDirect3DVertexBuffer8 *buffer;
};

extern dynamic_buffer render_buffer;

#endif // !NO_DIRECT3D

#endif //_GRD3DINTERNAL_H

/*
	Main header file for the DX8.1 graphics routines.
	
	This class is in fact a singleton pattern, but it depends upon being manually destroyed
	through the destroyer god function. For safety's sake, we need to ensure this isn't invoked
	before it should be, and is owned before we try to create some other graphics interface.

	As such used gr_d3d8_init() as the first time its called, and gr_d3d8_cleanup to destroy it.

	Access to the class is through Instance() - i.e. (D3D8RENDER::Instance)-><whatever you want>
	is acceptable to get to it.
*/

#ifndef _GRD3D81_H
#define _GRD3D81_H

// DX8.1 header files
#include <d3d8.h>
#include <d3dx8.h>
#include "graphics/2d.h"
#include "graphics/grinternal.h"

void gr_d3d8_init();		// Creation god function
void gr_d3d8_cleanup();		// Destruction god function

extern int D3D_window;
extern int Cmdline_force_32bit;

extern color_gun *Gr_current_red, *Gr_current_green, *Gr_current_blue, *Gr_current_alpha;
extern color_gun Gr_red, Gr_green, Gr_blue, Gr_alpha;

extern bool D3D8_init;

// Officially C++'d!
class D3D8RENDER
{
public:
	static D3D8RENDER* Instance();		// Accessor Function
	
	~D3D8RENDER();
	
	HRESULT InitD3D(int screenx, int screeny, int depth);		// Self-explanatory
	void	AssignFunctions(void);								// Fill out the gr_screen structure with function pointers
	void	StartD3D8Frame();									// Starts a frame
	void	EndD3D8Frame();

	// Public D3D8 Structures
	LPDIRECT3D8			lpD3D;
	LPDIRECT3DDEVICE8	lpD3DDevice;
	D3DCAPS8			D3DCaps;		// Holds info about the D3D device
	D3DVIEWPORT8		viewdata;		// Holds info about the current device viewport

	// gr_screen functions - the way I do this is probably inefficient, but it is consistent
	// we'll fix it up later
	void gr_d3d8_flash(int r, int g, int b);
	void gr_d3d8_zbuffer_clear(int mode);
	int	gr_d3d8_zbuffer_get();
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

	void d3d8_tcache_init(int use_sections);
	void d3d8_tcache_cleanup();
	void d3d8_tcache_flush();
	void d3d8_tcache_frame();
	void d3d8_flush();
	int d3d8_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int sx = -1, int sy = -1, int force = 0);

	void d3d8_dump_frame();
	void gr_d3d8_flip();
	void gr_d3d8_flip_cleanup();
	void gr_d3d8_flip_window(uint _hdc, int x, int y, int w, int h);
	void gr_d3d8_fade_out(int instantaneous);
	void gr_d3d8_fade_in(int instantaneous);
	int gr_d3d8_save_screen();
	void gr_d3d8_restore_screen(int id);
	void gr_d3d8_free_screen(int id);
	void gr_d3d8_dump_frame_start(int first_frame, int frames_between_dumps);
	void gr_d3d8_dump_frame_stop();
	void gr_d3d8_set_gamma(float gamma);
	uint gr_d3d8_lock();
	void gr_d3d8_unlock();
	void gr_d3d8_get_region(int front, int w, int h, ubyte *data);
	void gr_d3d8_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far);
	void gr_d3d8_get_pixel(int x, int y, int *r, int *g, int *b);
	void gr_d3d8_set_cull(int cull);
	void gr_d3d8_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);
	void gr_d3d8_filter_set(int filter);
	void gr_d3d8_set_clear_color(int r, int g, int b);

protected:
	D3D8RENDER();			// Constructor function - called by the Instance function

private:
	static D3D8RENDER* pInstance;		// Singleton pointer
	RECT D3D_cursor_clip_rect;			// Area where the cursor is clipped - possibly quite pointless right now
	bool in_frame;						// Are we rendering a frame currently?

};

// Stub Functions which call the in class functions
// This is because I don't want to edit gr_screen, but if we do more replacementr of the functions
// then these will probably become obsolete
void gr_d3d8e_flash(int r, int g, int b);
void gr_d3d8e_zbuffer_clear(int mode);
int gr_d3d8e_zbuffer_get();
int gr_d3d8e_zbuffer_set(int mode);
void gr_d3d8e_tmapper( int nverts, vertex **verts, uint flags );
void gr_d3d8e_scaler(vertex *va, vertex *vb );
void gr_d3d8e_aascaler(vertex *va, vertex *vb );
void gr_d3d8e_pixel(int x, int y);
void gr_d3d8e_clear();
void gr_d3d8e_set_clip(int x,int y,int w,int h);
void gr_d3d8e_reset_clip();
void gr_d3d8e_init_color(color *c, int r, int g, int b);
void gr_d3d8e_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type );
void gr_d3d8e_set_color( int r, int g, int b );
void gr_d3d8e_get_color( int * r, int * g, int * b );
void gr_d3d8e_set_color_fast(color *dst);
void gr_d3d8e_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx=-1, int sy=-1 );
void gr_d3d8e_bitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d8e_bitmap(int x, int y);
void gr_d3d8e_aabitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d8e_aabitmap(int x, int y);
void gr_d3d8e_rect(int x,int y,int w,int h);
void gr_d3d8e_create_shader(shader * shade, float r, float g, float b, float c );
void gr_d3d8e_set_shader( shader * shade );
void gr_d3d8e_shade(int x,int y,int w,int h);
void gr_d3d8e_create_font_bitmap();
void gr_d3d8e_char(int x,int y,int letter);
void gr_d3d8e_string( int sx, int sy, char *s );
void gr_d3d8e_circle( int xc, int yc, int d );
void gr_d3d8e_line(int x1,int y1,int x2,int y2);
void gr_d3d8e_aaline(vertex *v1, vertex *v2);
void gr_d3d8e_gradient(int x1,int y1,int x2,int y2);
void gr_d3d8e_set_palette(ubyte *new_palette, int restrict_alphacolor);
void gr_d3d8e_diamond(int x, int y, int width, int height);
void gr_d3d8e_print_screen(char *filename);

void d3d8e_tcache_init(int use_sections);
void d3d8e_tcache_cleanup();
void d3d8e_tcache_flush();
void d3d8e_tcache_frame();
void d3d8e_flush();
int d3d8e_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, int sx = -1, int sy = -1, int force = 0);

void gr_d3d8e_dump_frame();
void gr_d3d8e_flip();
void gr_d3d8e_flip_cleanup();
void gr_d3d8e_flip_window(uint _hdc, int x, int y, int w, int h);
void gr_d3d8e_fade_out(int instantaneous);
void gr_d3d8e_fade_in(int instantaneous);
int gr_d3d8e_save_screen();
void gr_d3d8e_restore_screen(int id);
void gr_d3d8e_free_screen(int id);
void gr_d3d8e_dump_frame_start(int first_frame, int frames_between_dumps);
void gr_d3d8e_dump_frame_stop();
void gr_d3d8e_set_gamma(float gamma);
uint gr_d3d8e_lock();
void gr_d3d8e_unlock();
void gr_d3d8e_get_region(int front, int w, int h, ubyte *data);
void gr_d3d8e_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far);
void gr_d3d8e_get_pixel(int x, int y, int *r, int *g, int *b);
void gr_d3d8e_set_cull(int cull);
void gr_d3d8e_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);
void gr_d3d8e_filter_set(int filter);
void gr_d3d8e_set_clear_color(int r, int g, int b);


#endif // _GRD3D81_H	
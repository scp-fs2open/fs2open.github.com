/*
	Stores stub functions for the D3D8RENDER class so we can access it through gr_screen	
 */

#include "graphics/directx8/grd3d81.h"

#define d3d8 (D3D8RENDER::Instance())

void gr_d3d8e_flash(int r, int g, int b)
{
	d3d8->gr_d3d8_flash(r, g, b);
}

void gr_d3d8e_zbuffer_clear(int mode)
{
	d3d8->gr_d3d8_zbuffer_clear(mode);
}

int gr_d3d8e_zbuffer_get()
{
	return d3d8->gr_d3d8_zbuffer_get();
}

int gr_d3d8e_zbuffer_set(int mode)
{
	return d3d8->gr_d3d8_zbuffer_set(mode);
}

void gr_d3d8e_tmapper( int nverts, vertex **verts, uint flags )
{
	d3d8->gr_d3d8_tmapper( nverts, verts, flags );
}

void gr_d3d8e_scaler(vertex *va, vertex *vb )
{
	d3d8->gr_d3d8_scaler(va, vb );
}

void gr_d3d8e_aascaler(vertex *va, vertex *vb )
{
	d3d8->gr_d3d8_aascaler(va, vb );
}

void gr_d3d8e_pixel(int x, int y)
{
	d3d8->gr_d3d8_pixel(x, y);
}

void gr_d3d8e_clear()
{
	d3d8->gr_d3d8_clear();
}

void gr_d3d8e_set_clip(int x,int y,int w,int h)
{
	d3d8->gr_d3d8_set_clip(x, y, w, h);
}
void gr_d3d8e_reset_clip()
{
	d3d8->gr_d3d8_reset_clip();
}

void gr_d3d8e_init_color(color *c, int r, int g, int b)
{
	d3d8->gr_d3d8_init_color(c, r, g, b);
}

void gr_d3d8e_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	d3d8->gr_d3d8_init_alphacolor( clr, r, g, b, alpha, type );
}

void gr_d3d8e_set_color( int r, int g, int b )
{
	d3d8->gr_d3d8_set_color( r, g, b );
}

void gr_d3d8e_get_color( int * r, int * g, int * b )
{
	d3d8->gr_d3d8_get_color(r, g, b );
}

void gr_d3d8e_set_color_fast(color *dst)
{
	d3d8->gr_d3d8_set_color_fast(dst);
}

void gr_d3d8e_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	d3d8->gr_d3d8_set_bitmap( bitmap_num, alphablend_mode, bitblt_mode, alpha, sx, sy );
}

void gr_d3d8e_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	d3d8->gr_d3d8_bitmap_ex( x, y, w, h, sx,sy);
}

void gr_d3d8e_bitmap( int x, int y)
{
	d3d8->gr_d3d8_bitmap( x, y);
}

void gr_d3d8e_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	d3d8->gr_d3d8_aabitmap_ex(x, y, w, h, sx, sy);
}

void gr_d3d8e_aabitmap(int x, int y)
{
	d3d8->gr_d3d8_aabitmap( x, y);
}

void gr_d3d8e_rect(int x,int y,int w,int h)
{
	d3d8->gr_d3d8_rect( x, y, w, h);
}

void gr_d3d8e_create_shader(shader * shade, float r, float g, float b, float c )
{
	d3d8->gr_d3d8_create_shader(shade, r, g, b, c );
}

void gr_d3d8e_set_shader( shader * shade )
{
	d3d8->gr_d3d8_set_shader( shade );
}

void gr_d3d8e_shade(int x,int y,int w,int h)
{
	d3d8->gr_d3d8_shade(x, y, w, h);
}

void gr_d3d8e_create_font_bitmap()
{
	d3d8->gr_d3d8_create_font_bitmap();
}

void gr_d3d8e_char(int x,int y,int letter)
{
	d3d8->gr_d3d8_char( x, y, letter);
}

void gr_d3d8e_string( int sx, int sy, char *s )
{
	d3d8->gr_d3d8_string( sx, sy, s );
}

void gr_d3d8e_circle( int xc, int yc, int d )
{
	d3d8->gr_d3d8_circle( xc, yc, d );
}

void gr_d3d8e_line(int x1,int y1,int x2,int y2)
{
	d3d8->gr_d3d8_line( x1, y1, x2, y2);
}

void gr_d3d8e_aaline(vertex *v1, vertex *v2)
{
	d3d8->gr_d3d8_aaline(v1,v2);
}

void gr_d3d8e_gradient(int x1,int y1,int x2,int y2)
{
	d3d8->gr_d3d8_gradient(x1,y1,x2,y2);
}

void gr_d3d8e_set_palette(ubyte *new_palette, int restrict_alphacolor)
{
	d3d8->gr_d3d8_set_palette(new_palette, restrict_alphacolor);
}

void gr_d3d8e_diamond(int x, int y, int width, int height)
{
	d3d8->gr_d3d8_diamond(x, y, width, height);
}

void gr_d3d8e_print_screen(char *filename)
{
	d3d8->gr_d3d8_print_screen(filename);
}

void d3d8e_tcache_init(int use_sections)
{
	d3d8->d3d8_tcache_init(use_sections);
}

void d3d8e_tcache_cleanup()
{
	d3d8->d3d8_tcache_cleanup();
}

void d3d8e_tcache_flush()
{
	d3d8->d3d8_tcache_flush();
}

void d3d8e_tcache_frame()
{
	d3d8->d3d8_tcache_frame();
}

void d3d8e_flush()
{
	d3d8->d3d8_flush();
}

int d3d8e_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full, int sx, int sy, int force)
{
	return d3d8->d3d8_tcache_set(bitmap_id, bitmap_type, u_ratio, v_ratio, fail_on_full, sx, sy, force);
}

void gr_d3d8e_dump_frame()
{
	d3d8->d3d8_dump_frame();
}

void gr_d3d8e_flip()
{
	d3d8->gr_d3d8_flip();
}

void gr_d3d8e_flip_cleanup()
{
	d3d8->gr_d3d8_flip_cleanup();
}

void gr_d3d8e_flip_window(uint _hdc, int x, int y, int w, int h)
{
	d3d8->gr_d3d8_flip_window(_hdc, x, y, w, h);
}

void gr_d3d8e_fade_out(int instantaneous)
{
	d3d8->gr_d3d8_fade_out(instantaneous);
}

void gr_d3d8e_fade_in(int instantaneous)
{
	d3d8->gr_d3d8_fade_in(instantaneous);
}

int gr_d3d8e_save_screen()
{
	return d3d8->gr_d3d8_save_screen();
}

void gr_d3d8e_restore_screen(int id)
{
	d3d8->gr_d3d8_restore_screen(id);
}

void gr_d3d8e_free_screen(int id)
{
	d3d8->gr_d3d8_free_screen(id);
}

void gr_d3d8e_dump_frame_start(int first_frame, int frames_between_dumps)
{
	d3d8->gr_d3d8_dump_frame_start(first_frame, frames_between_dumps);
}

void gr_d3d8e_dump_frame_stop()
{
	d3d8->gr_d3d8_dump_frame_stop();
}

void gr_d3d8e_set_gamma(float gamma)
{
	d3d8->gr_d3d8_set_gamma(gamma);
}

uint gr_d3d8e_lock()
{
	return d3d8->gr_d3d8_lock();
}

void gr_d3d8e_unlock()
{
	d3d8->gr_d3d8_unlock();
}

void gr_d3d8e_get_region(int front, int w, int h, ubyte *data)
{
	d3d8->gr_d3d8_get_region(front, w, h, data);
}

void gr_d3d8e_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
	d3d8->gr_d3d8_fog_set(fog_mode, r, g, b, fog_near, fog_far);
}

void gr_d3d8e_get_pixel(int x, int y, int *r, int *g, int *b)
{
	d3d8->gr_d3d8_get_pixel(x, y, r, g, b);
}

void gr_d3d8e_set_cull(int cull)
{
	d3d8->gr_d3d8_set_cull(cull);
}

void gr_d3d8e_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	d3d8->gr_d3d8_cross_fade(bmap1, bmap2, x1, y1, x2, y2, pct);
}

void gr_d3d8e_filter_set(int filter)
{
	d3d8->gr_d3d8_filter_set(filter);
}

void gr_d3d8e_set_clear_color(int r, int g, int b)
{
	d3d8->gr_d3d8_set_clear_color(r, g, b);
}

#undef d3d
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGL.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:57 $
 * $Author: penguin $
 *
 * Code that uses the OpenGL graphics library
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 10    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 9     7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 8     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 7     2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 6     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 5     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 4     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 3     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 14    5/20/98 9:46p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 13    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 12    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 11    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 10    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 9     12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 8     10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 7     9/23/97 10:45a John
 * made so you can tell bitblt code to rle a bitmap by passing flag to
 * gr_set_bitmap
 * 
 * 6     9/09/97 11:01a Sandeep
 * fixed warning level 4 bugs
 * 
 * 5     7/10/97 2:06p John
 * added code to specify alphablending type for bitmaps.
 * 
 * 4     6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 3     6/12/97 2:50a Lawrance
 * bm_unlock() now passed bitmap number, not pointer
 * 
 * 2     6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 1     5/12/97 12:14p John
 *
 * $NoKeywords: $
 */

#include <windows.h>
#include <windowsx.h>

#include "osapi.h"
#include "2d.h"
#include "bmpman.h"
#include "floating.h"
#include "palman.h"
#include "grinternal.h"
#include "gropengl.h"
#include "line.h"

static int Inited = 0;

void gr_opengl_pixel(int x, int y)
{
	if ( x < gr_screen.clip_left ) return;
	if ( x > gr_screen.clip_right ) return;
	if ( y < gr_screen.clip_top ) return;
	if ( y > gr_screen.clip_bottom ) return;
}

void gr_opengl_clear()
{
}


void gr_opengl_flip()
{
}

void gr_opengl_flip_window(uint _hdc, int x, int y, int w, int h )
{
}

void gr_opengl_set_clip(int x,int y,int w,int h)
{
	// check for sanity of parameters
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	if (x >= gr_screen.max_w)
		x = gr_screen.max_w - 1;
	if (y >= gr_screen.max_h)
		y = gr_screen.max_h - 1;

	if (x + w > gr_screen.max_w)
		w = gr_screen.max_w - x;
	if (y + h > gr_screen.max_h)
		h = gr_screen.max_h - y;
	
	if (w > gr_screen.max_w)
		w = gr_screen.max_w;
	if (h > gr_screen.max_h)
		h = gr_screen.max_h;
	
	gr_screen.offset_x = x;
	gr_screen.offset_y = y;
	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;
	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;
	gr_screen.clip_width = w;
	gr_screen.clip_height = h;
}

void gr_opengl_reset_clip()
{
	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;
}

void gr_opengl_set_font(int fontnum)
{
}

void gr_opengl_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_screen.current_color.red = (unsigned char)r;
	gr_screen.current_color.green = (unsigned char)g;
	gr_screen.current_color.blue = (unsigned char)b;
}

void gr_opengl_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;

	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
}

void gr_opengl_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;
}

void gr_opengl_set_shader( shader * shade )
{	
	if ( shade )	{
		if (shade->screen_sig != gr_screen.signature)	{
			gr_create_shader( shade, shade->r, shade->g, shade->b, shade->c );
		}
		gr_screen.current_shader = *shade;
	} else {
		gr_create_shader( &gr_screen.current_shader, 0.0f, 0.0f, 0.0f, 0.0f );
	}
}


void gr_opengl_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int i,j;
	bitmap * bmp;
	ubyte * sptr;

	bmp = bm_lock( gr_screen.current_bitmap, 8, 0 );
	sptr = (ubyte *)( bmp->data + (sy*bmp->w+sx) );

//	mprintf(( "x=%d, y=%d, w=%d, h=%d\n", x, y, w, h ));
//	mprintf(( "sx=%d, sy=%d, bw=%d, bh=%d\n", sx, sy, bmp->w, bmp->h ));

	for (i=0; i<h; i++ )	{
		for ( j=0; j<w; j++ )	{
			gr_set_color( gr_palette[sptr[j]*3+0], gr_palette[sptr[j]*3+1], gr_palette[sptr[j]*3+2] );
			gr_pixel( x+j, i+y );
		}
		sptr += bmp->w;
	}
	bm_unlock(gr_screen.current_bitmap);
}

void gr_opengl_bitmap(int x, int y)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );
	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
	if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
	if ( dx1 < gr_screen.clip_left ) { sx = gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
	if ( dy1 < gr_screen.clip_top ) { sy = gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
	if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
	if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	gr_bitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

static void opengl_scanline(int x1,int x2,int y)
{
}

void gr_opengl_rect(int x,int y,int w,int h)
{
	int i, swapped=0;
	int x1 = x, x2;
	int y1 = y, y2;

	if ( w > 0 )
		 x2 = x + w - 1;
	else
		 x2 = x + w + 1;

	if ( h > 0 )
		y2 = y + h - 1;
	else
		y2 = y + h + 1;
		
	if ( x2 < x1 )	{
		int tmp;	
		tmp = x1;
		x1 = x2;
		x2 = tmp;
		w = -w;
		swapped = 1;
	}

	if ( y2 < y1 )	{
		int tmp;	
		tmp = y1;
		y1 = y2;
		y2 = tmp;
		h = -h;
		swapped = 1;
	}

	for (i=0; i<h; i++ )
		opengl_scanline( x1, x2, y1+i );
}


void gr_opengl_shade(int x,int y,int w,int h)
{
}

void opengl_mtext(int x, int y, char *s, int len )
{
}

void gr_opengl_string(int x,int y,char * text)
{
	char *p, *p1;
	int w, h;

	p1 = text;
	do {
		p = strchr( p1, '\n' );
		if ( p ) { 
			*p = 0;
			p++;
		}
		gr_get_string_size( &w, &h, p1 );

		if ( x == 0x8000 )
			opengl_mtext(gr_screen.offset_x+(gr_screen.clip_width-w)/2,y+gr_screen.offset_y,p1,strlen(p1));
		else
			opengl_mtext(gr_screen.offset_x+x,y+gr_screen.offset_y,p1,strlen(p1));

		p1 = p;
		if ( p1 && (strlen(p1) < 1) ) p1 = NULL;
		y += h;
	} while(p1!=NULL);
}




void gr_opengl_circle( int xc, int yc, int d )
{
	int p,x, y, r;

	r = d/2;
	p=3-d;
	x=0;
	y=r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) return;
	if ( (xc-r) > gr_screen.clip_right ) return;
	if ( (yc+r) < gr_screen.clip_top ) return;
	if ( (yc-r) > gr_screen.clip_bottom ) return;

	while(x<y)	{
		// Draw the first octant
		opengl_scanline( xc-y, xc+y, yc-x );
		opengl_scanline( xc-y, xc+y, yc+x );

		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			opengl_scanline( xc-x, xc+x, yc-y );
			opengl_scanline( xc-x, xc+x, yc+y );
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y)	{
		opengl_scanline( xc-x, xc+x, yc-y );
		opengl_scanline( xc-x, xc+x, yc+y );
	}
	return;
}


void gr_opengl_line(int x1,int y1,int x2,int y2)
{
	int i;
   int xstep,ystep;
   int dy=y2-y1;
   int dx=x2-x1;
   int error_term=0;
	int clipped = 0, swapped=0;

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);
		
	if(dy<0)	{
		dy=-dy;
      ystep=-1;
	}	else	{
      ystep=1;
	}

   if(dx<0)	{
      dx=-dx;
      xstep=-1;
   } else {
      xstep=1;
	}

	if(dx>dy)	{

		for(i=dx+1;i>0;i--) {
			gr_pixel( x1, y1 ); 
			x1 += xstep;
         error_term+=dy;

         if(error_term>dx)	{
				error_term-=dx;
            y1+=ystep;
         }
      }
   } else {

      for(i=dy+1;i>0;i--)	{
			gr_pixel( x1, y1 ); 
			y1 += ystep;
         error_term+=dx;
         if(error_term>0)	{
            error_term-=dy;
            x1+=xstep;
         }

      }

   }
}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

void gr_opengl_scaler(vertex *va, vertex *vb )
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	//============= CLIP IT =====================

	x0 = va->sx; y0 = va->sy;
	x1 = vb->sx; y1 = vb->sy;

	xmin = i2fl(gr_screen.clip_left); ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right); ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->u; v0 = va->v;
	u1 = vb->u; v1 = vb->v;

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<xmin ) || (x0>xmax) ) return;
	if ( (y1<ymin ) || (y0>ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = fl2i(clipped_x0); dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0); dy1 = fl2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	//============= DRAW IT =====================
	int u, v, du, dv;
	int y, w;
	ubyte * sbits;
	bitmap * bp;
	ubyte * spixels;
	float tmpu, tmpv;

	tmpu = (clipped_u1-clipped_u0) / (dx1-dx0);
	if ( fl_abs(tmpu) < 0.001f ) {
		return;		// scaled up way too far!
	}
	tmpv = (clipped_v1-clipped_v0) / (dy1-dy0);
	if ( fl_abs(tmpv) < 0.001f ) {
		return;		// scaled up way too far!
	}

	bp = bm_lock( gr_screen.current_bitmap, 8, 0 );

	du = fl2f(tmpu*(bp->w-1));
	dv = fl2f(tmpv*(bp->h-1));

	v = fl2f(clipped_v0*(bp->h-1));
	u = fl2f(clipped_u0*(bp->w-1)); 
	w = dx1 - dx0 + 1;

	spixels = (ubyte *)bp->data;

	for (y=dy0; y<=dy1; y++ )			{
		sbits = &spixels[bp->rowsize*(v>>16)];

		int x, tmp_u;
		tmp_u = u;
		for (x=0; x<w; x++ )			{
			ubyte c = sbits[ tmp_u >> 16 ];
			if ( c != 255 ) {
				gr_set_color( gr_palette[c*3+0], gr_palette[c*3+1], gr_palette[c*3+2] );
				gr_pixel( x+dx0, y );
			}
			tmp_u += du;
		}
		v += dv;
	}

	bm_unlock(gr_screen.current_bitmap);

}



void gr_opengl_tmapper( int nv, vertex * verts[], uint flags )
{
}


void gr_opengl_gradient(int x1,int y1,int x2,int y2)
{
}

void gr_opengl_set_palette(ubyte *new_palette, int is_alphacolor)
{
}

void gr_opengl_get_color( int * r, int * g, int * b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

void gr_opengl_init_color(color *c, int r, int g, int b)
{
	gr_screen.current_color.screen_sig = gr_screen.signature;
	gr_screen.current_color.red = (unsigned char)r;
	gr_screen.current_color.green = (unsigned char)g;
	gr_screen.current_color.blue = (unsigned char)b;
}

void gr_opengl_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		gr_init_color( dst, dst->red, dst->green, dst->blue );
		return;
	}
	gr_screen.current_color = *dst;
}



void gr_opengl_print_screen(char *filename)
{

}

int gr_opengl_supports_res_ingame(int res)
{
	return 1;
}

int gr_opengl_supports_res_interface(int res)
{
	return 1;
}

void gr_opengl_cleanup()
{
	if ( !Inited )	return;

	gr_reset_clip();
	gr_clear();
	gr_flip();

	Inited = 0;
}

void gr_opengl_fog_set(int fog_mode, int r, int g, int b, float near, float far)
{
}

void gr_opengl_get_pixel(int x, int y, int *r, int *g, int *b)
{
}

void gr_opengl_get_region(int front, int w, int g, ubyte *data)
{
}

void gr_opengl_set_cull(int cull)
{
}

void gr_opengl_filter_set(int filter)
{
}

// cross fade
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
}

int gr_opengl_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0)
{
	return 1;
}

void gr_opengl_set_clear_color(int r, int g, int b)
{
}

void gr_opengl_init()
{
	if ( Inited )	{
		gr_opengl_cleanup();
		Inited = 0;
	}

	mprintf(( "Initializing opengl graphics device...\n" ));
	Inited = 1;

	gr_opengl_clear();

	gr_screen.gf_flip = gr_opengl_flip;
	gr_screen.gf_flip_window = gr_opengl_flip_window;
	gr_screen.gf_set_clip = gr_opengl_set_clip;
	gr_screen.gf_reset_clip = gr_opengl_reset_clip;
	gr_screen.gf_set_font = gr_opengl_set_font;
	gr_screen.gf_set_color = gr_opengl_set_color;
	gr_screen.gf_set_bitmap = gr_opengl_set_bitmap;
	gr_screen.gf_create_shader = gr_opengl_create_shader;
	gr_screen.gf_set_shader = gr_opengl_set_shader;
	gr_screen.gf_clear = gr_opengl_clear;
	// gr_screen.gf_bitmap = gr_opengl_bitmap;
	// gr_screen.gf_bitmap_ex = gr_opengl_bitmap_ex;
	gr_screen.gf_rect = gr_opengl_rect;
	gr_screen.gf_shade = gr_opengl_shade;
	gr_screen.gf_string = gr_opengl_string;
	gr_screen.gf_circle = gr_opengl_circle;

	gr_screen.gf_line = gr_opengl_line;
	gr_screen.gf_pixel = gr_opengl_pixel;
	gr_screen.gf_scaler = gr_opengl_scaler;
	gr_screen.gf_tmapper = gr_opengl_tmapper;

	gr_screen.gf_gradient = gr_opengl_gradient;

	gr_screen.gf_set_palette = gr_opengl_set_palette;
	gr_screen.gf_get_color = gr_opengl_get_color;
	gr_screen.gf_init_color = gr_opengl_init_color;
	gr_screen.gf_set_color_fast = gr_opengl_set_color_fast;
	gr_screen.gf_print_screen = gr_opengl_print_screen;

	gr_screen.gf_fog_set = gr_opengl_fog_set;	

	gr_screen.gf_get_region = gr_opengl_get_region;

	gr_screen.gf_get_pixel = gr_opengl_get_pixel;

	gr_screen.gf_set_cull = gr_opengl_set_cull;

	gr_screen.gf_cross_fade = gr_opengl_cross_fade;

	gr_screen.gf_filter_set = gr_opengl_filter_set;

	gr_screen.gf_tcache_set = gr_opengl_tcache_set;

	gr_screen.gf_set_clear_color = gr_opengl_set_clear_color;
}




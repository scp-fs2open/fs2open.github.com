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
 * $Revision: 2.0 $
 * $Date: 2002-06-03 03:55:40 $
 * $Author: penguin $
 *
 * Code that uses the OpenGL graphics library
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.12  2002/06/03 03:40:34  mharris
 * Only set projection matrix once (at init);  aabitmap now uses
 * the current_alpha setting
 *
 * Revision 1.11  2002/06/01 02:35:08  mharris
 * tmapper does (simple) rendering now!  Added gr_opengl_line_internal(),
 * removed some annoying printfs
 *
 * Revision 1.10  2002/05/30 13:17:57  mharris
 * aaline is working... first sorta 3D thing!
 * Added scissor calls
 *
 * Revision 1.9  2002/05/30 10:57:46  mharris
 * bitmap_ex and aabitmap_ex use x & y offset correctly.
 * First attempt at aaline...
 *
 * Revision 1.8  2002/05/29 16:24:03  mharris
 * Trying to debug aa palette stuff...
 *
 * Revision 1.7  2002/05/29 14:43:04  mharris
 * Added mprintfs to unimplemented callbacks.
 * Added set_gamma code
 * Removed (unused) gr_opengl_init_alphacolor
 *
 * Revision 1.6  2002/05/26 14:10:02  mharris
 * More testing
 *
 * Revision 1.5  2002/05/24 16:47:22  mharris
 * Still work-in-progress...
 *
 * Revision 1.4  2002/05/21 21:54:59  mharris
 * Added call to grx_init_alphacolors()
 *
 * Revision 1.3  2002/05/21 15:40:35  mharris
 * Added SDL/OpenGL support!  Still work-in-progress
 *
 * Revision 1.2  2002/05/17 23:43:03  mharris
 * Rearrange function defs in init() so we can see what's undefined.
 * Added (empty) gr_opengl_set_gamma()
 *
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

#if defined WIN32
#include <windows.h>
#include <windowsx.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include "pstypes.h"
#include "osapi.h"
#include "2d.h"
#include "bmpman.h"
#include "floating.h"
#include "palman.h"
#include "grinternal.h"
#include "gropengl.h"
#include "line.h"
#include "colors.h"

SDL_Surface *opengl_screen = NULL;
ubyte *opengl_bmp_buffer = NULL;

static int Inited = 0;

int RunGLTest( int argc, char* argv[],
               int logo, int slowly, int bpp, float gamma, int noframe );

void gr_opengl_pixel(int x, int y)
{
	if ( x < gr_screen.clip_left ) return;
	if ( x > gr_screen.clip_right ) return;
	if ( y < gr_screen.clip_top ) return;
	if ( y > gr_screen.clip_bottom ) return;
	
	// set the color
	glColor4ub(gr_screen.current_color.red,
				  gr_screen.current_color.green,
				  gr_screen.current_color.blue,
				  gr_screen.current_color.alpha);

	// 0,0 is lower-left in GL...
	int clip_top = gr_screen.max_h - gr_screen.offset_y - 1;
	int clip_bottom = gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_bottom - 1;
	GLfloat x1 =                   gr_screen.offset_x + x;
	GLfloat y1 = gr_screen.max_h - gr_screen.offset_y - y - 1;

	// draw the line
	glBegin(GL_POINTS);
	glVertex2f(x1, y1);
	glEnd();
}

void gr_opengl_clear()
{
//TEMP
//  	mprintf(("gr_opengl_clear()\n"));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void gr_opengl_flip()
{
	int status;

	SDL_GL_SwapBuffers();
//	gr_opengl_clear();
}

void gr_opengl_flip_window(uint _hdc, int x, int y, int w, int h )
{
//TEMP
  	mprintf(("gr_opengl_flip_window()\n"));
	debug_int3();
}

void gr_opengl_set_scissor()
{
	int bottom = gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_bottom - 1;
//  	mprintf(("glScissor(%4d, %4d, %4d, %4d)\n",
//  				gr_screen.offset_x, bottom, 				
//  				gr_screen.clip_width, gr_screen.clip_height));
	glScissor(gr_screen.offset_x, bottom, 				
				 gr_screen.clip_width, gr_screen.clip_height);
}


void gr_opengl_set_clip(int x,int y,int w,int h)
{
//  	mprintf(("gr_opengl_set_clip: at (%3d,%3d), size (%3d, %3d)\n",
//  				x, y, w, h));
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

	gr_opengl_set_scissor();
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

	gr_opengl_set_scissor();
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
//TEMP
//  	mprintf(("gr_opengl_set_bitmap(%d, %s)\n", bitmap_num, bm_get_filename(bitmap_num)));

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
//TEMP
//  	mprintf(("gr_opengl_bitmap_ex: at (%3d,%3d) size (%3d,%3d) name %s\n", 
//  				x, y, w, h, 
//  				bm_get_filename(gr_screen.current_bitmap)));

	int i,j;
	bitmap * bmp;

  	Assert(opengl_screen != NULL);
	Assert(opengl_bmp_buffer != NULL);

	// mharris mod - not sure if this is right...
	bmp = bm_lock( gr_screen.current_bitmap, 16, 0 );
	ushort * sptr = (ushort *)( bmp->data + (sy*bmp->w+sx) );

	// slow blit...
	for (i=0; i<h; i++ )	{
		for ( j=0; j<w; j++ )	{
			ubyte * buffer = &opengl_bmp_buffer[(((h-i-1)*w) + j) * 4]; // 4bytes per pixel (RGBA)
			// hack for now... hard-coded 16-bit values
			if (sptr[j] == 0x07e0) {
				// transparent color
				buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0x00;
			} else {
				buffer[0] = ((sptr[j] & 0xf800) >> 11) << 3;
				buffer[1] = ((sptr[j] & 0x07e0) >>  5) << 2;
				buffer[2] = ((sptr[j] & 0x001f)      ) << 3;
				buffer[3] = 0xff;
			}
//  			gr_set_color( gr_palette[sptr[j]*3+0], gr_palette[sptr[j]*3+1], gr_palette[sptr[j]*3+2] );
//  			gr_pixel( x+j, i+y );
		}
		sptr += bmp->w;
	}
	bm_unlock(gr_screen.current_bitmap);

	// set the raster pos
	int gl_y_origin = gr_screen.max_h - gr_screen.offset_y - y - h + 1;
	glRasterPos2i(gr_screen.offset_x + x, gl_y_origin);

	// put the bitmap into the GL framebuffer
	glDrawPixels(w, h,					// width, height
					 GL_RGBA,				// format
					 GL_UNSIGNED_BYTE,	// type
					 opengl_bmp_buffer);
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

void gr_opengl_aabitmap_ex(int x, int y, int w, int h, int sx, int sy)
{
//TEMP
//  	if (sx || sy)
//  		mprintf(("gr_opengl_aabitmap_ex(%3d,%3d; %3d,%3d; %3d,%3d) name %s\n", 
//  					x, y, w, h, sx, sy,
//  					bm_get_filename(gr_screen.current_bitmap)));

	int i,j;
	bitmap * bmp;

  	Assert(opengl_screen != NULL);
	Assert(opengl_bmp_buffer != NULL);

	// mharris mod - not sure if this is right...
	bmp = bm_lock( gr_screen.current_bitmap, 8, BMP_AABITMAP );
	ubyte * sptr = (ubyte *)( bmp->data + (sy*bmp->w+sx) );

	// slow blit...
//	ubyte * buffer = &opengl_bmp_buffer[(h-1) * w * 4];
	for (i=0; i<h; i++ )	{
		for ( j=0; j<w; j++ )	{

			ubyte * buffer = &opengl_bmp_buffer[(((h-i-1)*w) + j) * 4]; // 4bytes per pixel (RGBA)

			// what a hideous hack... why isn't the palette working???
			uint pal = ((unsigned int)sptr[j]) * 18;
			if (pal == 0x10e) {
				pal = 0;
			} else if (pal > 255) {
				mprintf(("pal %02x at (%3d, %3d) \"%s\"\n",
							pal, j, i, 
							bm_get_filename(gr_screen.current_bitmap)));
				pal = 255;
			}

			buffer[0] = gr_screen.current_color.red;
			buffer[1] = gr_screen.current_color.green;
			buffer[2] = gr_screen.current_color.blue;
			buffer[3] = (gr_screen.current_color.alpha * pal) / 255;

//			buffer += 4;  // 4bytes per pixel (RGBA)



//			if ( gr_screen.current_color.is_alphacolor )	{
//  				color = RGBA_MAKE(r,g,b, 255 );
//  			} else {
//  				color = RGB_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
//  			}
		}
//		buffer -= (w * 4);
		sptr += bmp->w;
	}
	bm_unlock(gr_screen.current_bitmap);
	
	// set the raster pos
	int gl_y_origin = gr_screen.max_h - gr_screen.offset_y - y - h + 1;
	glRasterPos2i(gr_screen.offset_x + x, gl_y_origin);

	// put the bitmap into the GL framebuffer
	glDrawPixels(w, h,					// width, height
					 GL_RGBA,				// format
					 GL_UNSIGNED_BYTE,	// type
					 opengl_bmp_buffer);
}

void gr_opengl_aabitmap(int x, int y)
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

	gr_aabitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}


void gr_opengl_line_internal(float x1, float y1, float x2, float y2)
{
	// set the color
	glColor4ub(gr_screen.current_color.red,
				  gr_screen.current_color.green,
				  gr_screen.current_color.blue,
				  gr_screen.current_color.alpha);

	// 0,0 is lower-left in GL...
	int clip_top = gr_screen.max_h - gr_screen.offset_y - 1;
	int clip_bottom = gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_bottom - 1;
	GLfloat gl_x1 =                   gr_screen.offset_x + x1;
	GLfloat gl_y1 = gr_screen.max_h - gr_screen.offset_y - y1 - 1;
	GLfloat gl_x2 =                   gr_screen.offset_x + x2;
	GLfloat gl_y2 = gr_screen.max_h - gr_screen.offset_y - y2 - 1;

	// draw the line
	glBegin(GL_LINES);
	glVertex2f(gl_x1, gl_y1);
	glVertex2f(gl_x2, gl_y2);
	glEnd();
}


static void opengl_scanline(int x1,int x2,int y)
{
//  	mprintf(("opengl_scanline(x1=%d, x2=%d, y=%d)\n",
//  				x1, x2, y));
	// FIXME
	gr_opengl_line_internal(x1, y, x2, y);
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

void old_gr_opengl_string(int x,int y,char * text)
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



void gr_opengl_string( int sx, int sy, char *s )
{
	int width, spacing, letter;
	int x, y;

	if ( !Current_font )	{
		return;
	}

	gr_set_bitmap(Current_font->bitmap_id);
	bm_get_palette(Current_font->bitmap_id, gr_palette, NULL);

	x = sx;
	y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}
	
	spacing = 0;

	while (*s)	{
		x += spacing;

		while (*s== '\n' )	{
			s++;
			y += Current_font->h;
			if (sx==0x8000) {			//centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}
		if (*s == 0 ) break;

		letter = get_char_width(s[0],s[1],&width,&spacing);
		s++;

		//not in font, draw as space
		if (letter<0)	{
			continue;
		}

//  		mprintf(("print char '%c' color %02x/%02x/%02x/%02x\n",
//  					*(s-1), 
//  					gr_screen.current_color.red,
//  					gr_screen.current_color.green,
//  					gr_screen.current_color.blue,
//  					gr_screen.current_color.alpha));


		int xd, yd, xc, yc;
		int wc, hc;

		// Check if this character is totally clipped
		if ( x + width < gr_screen.clip_left ) continue;
		if ( y + Current_font->h < gr_screen.clip_top ) continue;
		if ( x > gr_screen.clip_right ) continue;
		if ( y > gr_screen.clip_bottom ) continue;

		xd = yd = 0;
		if ( x < gr_screen.clip_left ) xd = gr_screen.clip_left - x;
		if ( y < gr_screen.clip_top ) yd = gr_screen.clip_top - y;
		xc = x+xd;
		yc = y+yd;

		wc = width - xd; hc = Current_font->h - yd;
		if ( xc + wc > gr_screen.clip_right ) wc = gr_screen.clip_right - xc;
		if ( yc + hc > gr_screen.clip_bottom ) hc = gr_screen.clip_bottom - yc;

		if ( wc < 1 ) continue;
		if ( hc < 1 ) continue;

		font_char *ch;
	
		ch = &Current_font->char_data[letter];

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		gr_opengl_aabitmap_ex( xc, yc, wc, hc, u+xd, v+yd );
	}
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


void old_gr_opengl_line(int x1,int y1,int x2,int y2)
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
//  	mprintf(("TODO: gr_opengl_scaler:\n"
//  				"  va: w = (%7.1f,%7.1f,%7.1f) s = (%7.1f,%7.1f,%7.1f)\n"
//  				"  vb: w = (%7.1f,%7.1f,%7.1f) s = (%7.1f,%7.1f,%7.1f)\n",
//  				va->x, va->y, va->z, va->sx, va->sy, va->sw, 
//  				vb->x, vb->y, vb->z, vb->sx, vb->sy, vb->sw));
	return;

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

	bp = bm_lock( gr_screen.current_bitmap, 16, 0 );

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
	// TODO!!!!
//  	if (flags)
//  		return;
//  	mprintf(("gr_opengl_tmapper: (%d verts, flags=%04x)\n", nv, flags));
//  	for (int i = 0;  i < nv;  i++) {
//  		mprintf(("  v[%2d]:  (%7.1f, %7.1f)\n", i, verts[i]->sx, verts[i]->sy));
//  	}

//  	if (flags) {
//  		mprintf(("gr_opengl_tmapper: (%d verts, flags=%04x)\n", nv, flags));
//  //  		return;
//  	}


	if (flags & TMAP_FLAG_TEXTURED) {
		bitmap * bmp = bm_lock( gr_screen.current_bitmap, 16, 0 );
		bm_unlock(gr_screen.current_bitmap);
		// just for yux
		glColor4ub(0x80, 0x80, 0x80, 0x80);
	} else {
		// set the color
		glColor4ub(gr_screen.current_color.red,
					  gr_screen.current_color.green,
					  gr_screen.current_color.blue,
					  gr_screen.current_color.alpha);
	}


	// draw the poly
	glBegin(GL_POLYGON);
	for (int i = 0;  i < nv;  i++) {
		  GLfloat x =                   gr_screen.offset_x + verts[i]->sx;
		  GLfloat y = gr_screen.max_h - gr_screen.offset_y - verts[i]->sy - 1;
		glVertex2f(x, y);
	}
	glEnd();
}


void gr_opengl_gradient(int x1,int y1,int x2,int y2)
{
	// FIXME
	gr_opengl_line_internal(x1, y1, x2, y2);
}

void gr_opengl_set_palette(ubyte *new_palette, int is_alphacolor)
{
	mprintf(("gr_opengl_set_palette(%p, %d)\n", new_palette, is_alphacolor));
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
	mprintf(("TODO: gr_opengl_print_screen()\n"));
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
//  	gr_opengl_screen = NULL;
  	opengl_screen = NULL;
	if (opengl_bmp_buffer)
		free(opengl_bmp_buffer);
	opengl_bmp_buffer = NULL;

	SDL_Quit( );
}

void gr_opengl_fog_set(int fog_mode, int r, int g, int b, float near, float far)
{
	if (fog_mode == GR_FOGMODE_NONE){
		// only change state if we need to
		if(gr_screen.current_fog_mode != fog_mode){
			glDisable(GL_FOG);
		}
		gr_screen.current_fog_mode = fog_mode;

		// to prevent further state changes
		return;
	}

	mprintf(("TODO: gr_opengl_fog_set(fog_mode=%d, r=%02x, g=%02x, b=%02x, near=%8.2f, far=%8.2f)\n",
				fog_mode, r, g, b, near, far));

//  	// switch fogging on
//  	if(gr_screen.current_fog_mode != fog_mode){		
//  		glEnable(GL_FOG);
//  		gr_screen.current_fog_mode = fog_mode;	
//  	}	

}

void gr_opengl_get_pixel(int x, int y, int *r, int *g, int *b)
{
	mprintf(("TODO: gr_opengl_get_pixel()\n"));
}

void gr_opengl_get_region(int front, int w, int g, ubyte *data)
{
	mprintf(("TODO: gr_opengl_get_region()\n"));
}

void gr_opengl_set_cull(int cull)
{
//  	mprintf(("TODO: gr_opengl_set_cull(%02x)\n", cull));
}

void gr_opengl_filter_set(int filter)
{
	mprintf(("TODO: gr_opengl_filter_set()\n"));
}

// cross fade
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	mprintf(("TODO: gr_opengl_cross_fade()\n"));
}

int gr_opengl_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0)
{
	mprintf(("TODO: gr_opengl_tcache_set()\n"));
	return 1;
}

void gr_opengl_set_clear_color(int r, int g, int b)
{
	mprintf(("TODO: gr_opengl_set_clear_color()\n"));
}

void gr_opengl_set_gamma(float gamma)
{
//	mprintf(("gr_opengl_set_gamma(%4.2f)\n", gamma));
	SDL_SetGamma(gamma, gamma, gamma);
}

/********************/
void gr_opengl_fade_in(int instantaneous)
{
	mprintf(("TODO: gr_opengl_fade_in()\n"));
}

void gr_opengl_fade_out(int instantaneous)
{
	mprintf(("TODO: gr_opengl_fade_out()\n"));
}

void gr_opengl_flash( int r, int g, int b )
{
	mprintf(("TODO: gr_opengl_flash()\n"));
}

void gr_opengl_aaline(vertex *v1, vertex *v2)
{
	  gr_opengl_line_internal(fl2i(v1->sx), fl2i(v1->sy), 
									  fl2i(v2->sx), fl2i(v2->sy));
}


void gr_opengl_line(int x1,int y1,int x2,int y2)
{
	  gr_opengl_line_internal(x1, y1, x2, y2);
}


void gr_opengl_aascaler(vertex *va, vertex *vb )
{
	mprintf(("TODO: gr_opengl_aascaler:\n"
				"  va: w = (%7.1f,%7.1f,%7.1f) s = (%7.1f,%7.1f,%7.1f)\n"
				"  vb: w = (%7.1f,%7.1f,%7.1f) s = (%7.1f,%7.1f,%7.1f)\n",
				va->x, va->y, va->z, va->sx, va->sy, va->sw, 
				vb->x, vb->y, vb->z, vb->sx, vb->sy, vb->sw));
}

void gr_opengl_start_frame()
{
	mprintf(("TODO: gr_opengl_start_frame()\n"));
}

void gr_opengl_stop_frame()
{
	mprintf(("TODO: gr_opengl_stop_frame()\n"));
}


static int opengl_zbuffer_mode = 0;

int gr_opengl_zbuffer_get()
{
//	mprintf(("TODO: gr_opengl_zbuffer_get()\n"));
	return opengl_zbuffer_mode;
}

int gr_opengl_zbuffer_set(int mode)
{
//	mprintf(("TODO: gr_opengl_zbuffer_set(%02x)\n", mode));
	int tmp = opengl_zbuffer_mode;
	opengl_zbuffer_mode = mode;
	return tmp;
}

void gr_opengl_zbuffer_clear(int use_zbuffer)
{
//  	mprintf(("TODO: gr_opengl_zbuffer_clear(%d)\n", use_zbuffer));
}

int gr_opengl_save_screen()
{
	mprintf(("TODO: gr_opengl_save_screen()\n"));
	return 0;
}

void gr_opengl_restore_screen(int id)
{
	mprintf(("TODO: gr_opengl_restore_screen()\n"));
}

void gr_opengl_free_screen(int id)
{
	mprintf(("TODO: gr_opengl_free_screen()\n"));
}

void gr_opengl_dump_frame_start( int first_frame_number, int nframes_between_dumps )
{
	mprintf(("TODO: gr_opengl_dump_frame_start()\n"));
}

void gr_opengl_dump_frame()
{
	mprintf(("TODO: gr_opengl_dump_frame()\n"));
}

void gr_opengl_dump_frame_stop()
{
	mprintf(("TODO: gr_opengl_dump_frame_stop()\n"));
}

uint gr_opengl_lock()
{
	mprintf(("TODO: gr_opengl_lock()\n"));
	return 0;
}

void gr_opengl_unlock()
{
	mprintf(("TODO: gr_opengl_unlock()\n"));
}


void gr_opengl_init_colorbits(int bpp)
{
	switch( bpp )	{
	case 8:
		Gr_red.bits = 8;
		Gr_red.shift = 16;
		Gr_red.scale = 1;
		Gr_red.mask = 0xff0000;

		Gr_green.bits = 8;
		Gr_green.shift = 8;
		Gr_green.scale = 1;
		Gr_green.mask = 0xff00;

		Gr_blue.bits = 8;
		Gr_blue.shift = 0;
		Gr_blue.scale = 1;
		Gr_blue.mask = 0xff;
		break;

	case 15:
		Gr_red.bits = 5;
		Gr_red.shift = 10;
		Gr_red.scale = 8;
		Gr_red.mask = 0x7C00;

		Gr_green.bits = 5;
		Gr_green.shift = 5;
		Gr_green.scale = 8;
		Gr_green.mask = 0x3E0;

		Gr_blue.bits = 5;
		Gr_blue.shift = 0;
		Gr_blue.scale = 8;
		Gr_blue.mask = 0x1F;
		break;

	case 16:
		Gr_red.bits = 5;
		Gr_red.shift = 11;
		Gr_red.scale = 8;
		Gr_red.mask = 0xF800;

		Gr_green.bits = 6;
		Gr_green.shift = 5;
		Gr_green.scale = 4;
		Gr_green.mask = 0x7E0;

		Gr_blue.bits = 5;
		Gr_blue.shift = 0;
		Gr_blue.scale = 8;
		Gr_blue.mask = 0x1F;
		break;

	case 24:
	case 32:
		Gr_red.bits = 8;
		Gr_red.shift = 16;
		Gr_red.scale = 1;
		Gr_red.mask = 0xff0000;

		Gr_green.bits = 8;
		Gr_green.shift = 8;
		Gr_green.scale = 1;
		Gr_green.mask = 0xff00;

		Gr_blue.bits = 8;
		Gr_blue.shift = 0;
		Gr_blue.scale = 1;
		Gr_blue.mask = 0xff;
		break;

	default:
		Int3();	// Illegal bpp
	}

	// mharris FIXME: should we do this here?
	Gr_t_red   = Gr_red;
	Gr_t_green = Gr_green;
	Gr_t_blue  = Gr_blue;

	Gr_ta_red   = Gr_red;
	Gr_ta_green = Gr_green;
	Gr_ta_blue  = Gr_blue;
}


void gr_opengl_init()
{
	if ( Inited )	{
		gr_opengl_cleanup();
		Inited = 0;
	}

	grx_init_alphacolors();

	// mharris FIXME: this is probably wrong
	gr_opengl_init_colorbits(16);

	mprintf(( "Initializing opengl graphics device...\n" ));
	Inited = 1;

	// set up functions
	gr_screen.gf_flip = gr_opengl_flip;
	gr_screen.gf_flip_window = gr_opengl_flip_window;
	gr_screen.gf_set_palette = gr_opengl_set_palette;
	gr_screen.gf_fade_in = gr_opengl_fade_in;
	gr_screen.gf_fade_out = gr_opengl_fade_out;
	gr_screen.gf_flash = gr_opengl_flash;
	gr_screen.gf_set_clip = gr_opengl_set_clip;
	gr_screen.gf_reset_clip = gr_opengl_reset_clip;
	gr_screen.gf_set_color = grx_set_color;
	gr_screen.gf_get_color = grx_get_color;
	gr_screen.gf_init_color = grx_init_color;
	gr_screen.gf_init_alphacolor = grx_init_alphacolor;
	gr_screen.gf_set_color_fast = grx_set_color_fast;
//  	gr_screen.gf_set_color = gr_opengl_set_color;
//  	gr_screen.gf_get_color = gr_opengl_get_color;
//  	gr_screen.gf_init_color = gr_opengl_init_color;
//  	gr_screen.gf_init_alphacolor = gr_opengl_init_alphacolor;
//  	gr_screen.gf_set_color_fast = gr_opengl_set_color_fast;
	gr_screen.gf_set_font = grx_set_font;
	gr_screen.gf_set_bitmap = gr_opengl_set_bitmap;
	gr_screen.gf_create_shader = gr_opengl_create_shader;
	gr_screen.gf_set_shader = gr_opengl_set_shader;
	gr_screen.gf_clear = gr_opengl_clear;
	gr_screen.gf_aabitmap = gr_opengl_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_opengl_aabitmap_ex;
	gr_screen.gf_rect = gr_opengl_rect;
	gr_screen.gf_shade = gr_opengl_shade;
	gr_screen.gf_string = gr_opengl_string;
	gr_screen.gf_gradient = gr_opengl_gradient;
	gr_screen.gf_circle = gr_opengl_circle;
	gr_screen.gf_line = gr_opengl_line;
//	gr_screen.gf_line = gr_opengl_aaline;
	gr_screen.gf_aaline = gr_opengl_aaline;
	gr_screen.gf_pixel = gr_opengl_pixel;
	gr_screen.gf_scaler = gr_opengl_scaler;
	gr_screen.gf_aascaler = gr_opengl_aascaler;
	gr_screen.gf_tmapper = gr_opengl_tmapper;
	gr_screen.gf_print_screen = gr_opengl_print_screen;
	gr_screen.gf_start_frame = gr_opengl_start_frame;
	gr_screen.gf_stop_frame = gr_opengl_stop_frame;
	gr_screen.gf_zbuffer_get = gr_opengl_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_opengl_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_opengl_zbuffer_clear;
	gr_screen.gf_save_screen = gr_opengl_save_screen;
	gr_screen.gf_restore_screen = gr_opengl_restore_screen;
	gr_screen.gf_free_screen = gr_opengl_free_screen;
	gr_screen.gf_dump_frame_start = gr_opengl_dump_frame_start;
	gr_screen.gf_dump_frame = gr_opengl_dump_frame;
	gr_screen.gf_dump_frame_stop = gr_opengl_dump_frame_stop;
	gr_screen.gf_set_gamma = gr_opengl_set_gamma;
	gr_screen.gf_lock = gr_opengl_lock;
	gr_screen.gf_unlock = gr_opengl_unlock;
	gr_screen.gf_get_region = gr_opengl_get_region;
	gr_screen.gf_fog_set = gr_opengl_fog_set;	
	gr_screen.gf_get_pixel = gr_opengl_get_pixel;
	gr_screen.gf_set_cull = gr_opengl_set_cull;
	gr_screen.gf_cross_fade = gr_opengl_cross_fade;
	gr_screen.gf_filter_set = gr_opengl_filter_set;
	gr_screen.gf_tcache_set = gr_opengl_tcache_set;
	gr_screen.gf_set_clear_color = gr_opengl_set_clear_color;

	RunGLTest( 0, NULL, 0, 0, 0, 0.0, 0 );

	gr_opengl_clear();
}


int RunGLTest( int argc, char* argv[],
               int logo, int slowly, int bpp, float gamma, int noframe )
{
	int i;
	int rgb_size[3];
	int w = 640;
	int h = 480;
	Uint32 video_flags;
	int value;

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		fprintf(stderr,"Couldn't initialize SDL: %s\n",SDL_GetError());
		exit( 1 );
	}

	/* See if we should detect the display depth */
	if ( bpp == 0 ) {
		if ( SDL_GetVideoInfo()->vfmt->BitsPerPixel <= 8 ) {
			bpp = 8;
		} else {
			bpp = 16;  /* More doesn't seem to work */
		}
	}

	/* Set the flags we want to use for setting the video mode */
  	video_flags = SDL_OPENGL | SDL_ANYFORMAT;
//	video_flags = SDL_SWSURFACE | SDL_ANYFORMAT;

//  			video_flags |= SDL_FULLSCREEN;
//             video_flags |= SDL_NOFRAME;

	/* Initialize the display */
	switch (bpp) {
	    case 8:
		rgb_size[0] = 2;
		rgb_size[1] = 3;
		rgb_size[2] = 3;
		break;
	    case 15:
	    case 16:
		rgb_size[0] = 5;
		rgb_size[1] = 5;
		rgb_size[2] = 5;
		break;
            default:
		rgb_size[0] = 8;
		rgb_size[1] = 8;
		rgb_size[2] = 8;
		break;
	}
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	opengl_screen = SDL_SetVideoMode( w, h, bpp, video_flags );
	if (opengl_screen  == NULL ) {
		fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	mprintf(("Screen BPP: %d\n", SDL_GetVideoSurface()->format->BitsPerPixel));
	mprintf(("\n"));
	mprintf(( "Vendor     : %s\n", glGetString( GL_VENDOR ) ));
	mprintf(( "Renderer   : %s\n", glGetString( GL_RENDERER ) ));
	mprintf(( "Version    : %s\n", glGetString( GL_VERSION ) ));
	mprintf(( "Extensions : %s\n", glGetString( GL_EXTENSIONS ) ));
	mprintf(("\n"));

	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &value );
	mprintf(( "SDL_GL_RED_SIZE: requested %d, got %d\n", rgb_size[0],value));
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &value );
	mprintf(( "SDL_GL_GREEN_SIZE: requested %d, got %d\n", rgb_size[1],value));
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &value );
	mprintf(( "SDL_GL_BLUE_SIZE: requested %d, got %d\n", rgb_size[2],value));
	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &value );
	mprintf(( "SDL_GL_DEPTH_SIZE: requested %d, got %d\n", bpp, value ));
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &value );
	mprintf(( "SDL_GL_DOUBLEBUFFER: requested 1, got %d\n", value ));

	/* Set the window manager title bar */
	SDL_WM_SetCaption( "FS2 Open", "fs2_open" );

	/* Set the gamma for the window */
	if ( gamma != 0.0 ) {
		SDL_SetGamma(gamma, gamma, gamma);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glEnable(GL_SCISSOR_TEST);

	/* This allows alpha blending of 2D textures with the scene */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, w, h);


	// set up ortho projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D((GLfloat) 0,
				  (GLfloat) w,
				  (GLfloat) 0,
				  (GLfloat) h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	opengl_bmp_buffer = (ubyte *)malloc(w * h * 4);  // 4 bytes for RGBA 
	return 0;
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Shade.cpp $
 * $Revision: 2.2 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Routines to shade an area.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/08 02:36:01  mharris
 * porting
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 19    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 18    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 17    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 16    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 15    10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 14    10/03/97 12:16p John
 * optimized the shader.  About 50% faster.
 * 
 * 13    10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 12    9/09/97 10:41a Sandeep
 * fixed warning level 4
 * 
 * 11    6/18/97 12:07p John
 * fixed some color bugs
 * 
 * 10    6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 9     5/28/97 8:59a John
 * Fixed bug with shader not working when switching to fullscreen.
 * 
 * 8     5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 7     11/19/96 2:44p Allender
 * fix up shader for 15 bpp
 * 
 * 6     11/18/96 2:27p Allender
 * made faster hacked version of shader for 16 bits
 * 
 * 5     11/18/96 1:48p Allender
 * added 16 bit version of (very slow) shader
 * 
 * 4     11/15/96 3:34p Allender
 * started on 16 bit support for the shader
 * 
 * 3     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "math/floating.h"
#include "graphics/line.h"
#include "palman/palman.h"

void grx_create_shader(shader * shade, float r, float g, float b, float c )
{
	int i;
	float Sr, Sg, Sb;
	float Dr, Dg, Db;
	int ri, gi, bi;

	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;

	for (i=0; i<256; i++ )	{
		Sr = i2fl( gr_palette[i*3+0] );
		Sg = i2fl( gr_palette[i*3+1] );
		Sb = i2fl( gr_palette[i*3+2] );
		Dr = Sr*r + Sg*r + Sb*r + c*256.0f;
		Dg = Sr*g + Sg*g + Sb*g + c*256.0f;
		Db = Sr*b + Sg*b + Sb*b + c*256.0f;
		ri = fl2i(Dr); if ( ri < 0 ) ri = 0; else if (ri>255) ri = 255;
		gi = fl2i(Dg); if ( gi < 0 ) gi = 0; else if (gi>255) gi = 255;
		bi = fl2i(Db); if ( bi < 0 ) bi = 0; else if (bi>255) bi = 255;
		shade->lookup[i] = (unsigned char)(palette_find(ri,gi,bi));
	}

}

void grx_set_shader( shader * shade )
{
	if ( shade ) {
		if (shade->screen_sig != gr_screen.signature)	{
			gr_create_shader( shade, shade->r, shade->g, shade->b, shade->c );
		}
		gr_screen.current_shader = *shade;
	} else {
		gr_create_shader( &gr_screen.current_shader, 0.0f, 0.0f, 0.0f, 0.0f );
	}
}


void gr8_shade(int x,int y,int w,int h)
{
	int x1, y1, x2, y2;
	ubyte *xlat_table;

	x1 = x; 
	if (x1 < gr_screen.clip_left) x1 = gr_screen.clip_left;
	if (x1 > gr_screen.clip_right) x1 = gr_screen.clip_right;

	x2 = x+w-1; 
	if (x2 < gr_screen.clip_left) x2 = gr_screen.clip_left;
	if (x2 > gr_screen.clip_right) x2 = gr_screen.clip_right;

	y1 = y; 
	if (y1 < gr_screen.clip_top) y1 = gr_screen.clip_top;
	if (y1 > gr_screen.clip_bottom) y1 = gr_screen.clip_bottom;

	y2 = y+h-1; 
	if (y2 < gr_screen.clip_top) y2 = gr_screen.clip_top;
	if (y2 > gr_screen.clip_bottom) y2 = gr_screen.clip_bottom;

	w = x2 - x1 + 1;
	if ( w < 1 ) return;

	h = y2 - y1 + 1;
	if ( h < 1 ) return;

	int i;
	xlat_table = gr_screen.current_shader.lookup;

	gr_lock();

	for (i=0; i<h; i++ )	{
		ubyte * dp = GR_SCREEN_PTR(ubyte,x1,y1+i);
				#ifdef USE_INLINE_ASM

					int w1=w;

					// 4 byte align
					while ( (uint)dp & 3 )	{
						*dp = xlat_table[*dp];
						dp++;
						w1--;
						if ( w1 < 1 ) break;
					}

					if ( w1 < 1 ) continue;
				
					int wd4 = w1 / 4;
					int left_over = w1 % 4;
			
					if ( wd4 > 0 )	{
#ifdef _WIN32
						_asm push eax
						_asm push ebx
						_asm push ecx
						_asm push edx
						_asm push edi		
						_asm push esi
						_asm mov esi, xlat_table
						_asm mov edi, dp
						_asm mov edi, dp
						_asm mov ecx, wd4
						_asm mov eax, 0
						_asm mov ebx, 0
						_asm mov edx, 0

	NextPixel:
						_asm mov eax, [edi]

						_asm mov dl, al
						_asm mov bl, ah

						_asm add edi, 4

						_asm mov al, [edx+esi]
						_asm mov ah, [ebx+esi]

						_asm ror eax, 16
						
						_asm mov dl, al
						_asm mov bl, ah

						_asm mov al, [edx+esi]
						_asm mov ah, [ebx+esi]

						_asm ror eax, 16

						_asm mov [edi-4], eax

						_asm dec ecx
						_asm jnz NextPixel


						_asm mov dp, edi

						_asm pop esi
						_asm pop edi
						_asm pop edx
						_asm pop ecx
						_asm pop ebx
						_asm pop eax
#else
#warning not implemented
#endif
					}

					for (int j=0; j<left_over; j++ )	{
						*dp = xlat_table[*dp];
						dp++;
					}

			
				#else
					for (int j=0; j<w; j++ )	{
						*dp = xlat_table[*dp];
						dp++;
					}
				#endif
	}

	gr_unlock();
	
}


/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrGlide.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Code that uses 3DFX's Glide graphics library
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 39    9/14/99 5:14a Dave
 * Fixed credits drawing in Glide.
 * 
 * 38    9/08/99 11:38p Dave
 * Make V2 minimize/maximize the old way.
 * 
 * 37    9/04/99 8:00p Dave
 * Fixed up 1024 and 32 bit movie support.
 * 
 * 36    9/01/99 10:49p Dave
 * Added nice SquadWar checkbox to the client join wait screen.
 * 
 * 35    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 34    8/06/99 12:29a Dave
 * Multiple bug fixes.
 * 
 * 33    8/04/99 5:36p Dave
 * Make glide and D3D switch out properly.
 * 
 * 32    7/19/99 3:29p Dave
 * Fixed gamma bitmap in the options screen.
 * 
 * 31    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 30    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 29    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 28    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 27    2/21/99 1:48p Dave
 * Some code for monitoring datarate for multiplayer in detail.
 * 
 * 26    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 25    2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 24    2/04/99 6:29p Dave
 * First full working rev of FS2 PXO support.  Fixed Glide lighting
 * problems.
 * 
 * 23    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 22    1/30/99 1:29a Dave
 * Fixed nebula thumbnail problem. Full support for 1024x768 choose pilot
 * screen.  Fixed beam weapon death messages.
 * 
 * 21    1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 20    1/23/99 5:33p Dave
 * Put in support for 1024x768 Glide.
 * 
 * 19    1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 18    12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 17    12/14/98 12:13p Dave
 * Spiffed up xfer system a bit. Put in support for squad logo file xfer.
 * Need to test now.
 * 
 * 16    12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 15    12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 14    12/04/98 10:20a Dave
 * Fixed up gr_glide_get_pixel(...)
 * 
 * 13    12/02/98 9:58a Dave
 * Got fonttool working under Glide/Direct3d.
 * 
 * 12    12/01/98 5:54p Dave
 * Simplified the way pixel data is swizzled. Fixed tga bitmaps to work
 * properly in D3D and Glide.
 * 
 * 11    12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 10    12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 9     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 8     11/24/98 4:43p Dave
 * Make sure glide starts up in 640x480
 * 
 * 7     11/20/98 11:16a Dave
 * Fixed up IPX support a bit. Making sure that switching modes and
 * loading/saving pilot files maintains proper state.
 * 
 * 6     11/14/98 5:32p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 5     11/12/98 12:19a Dave
 * Removed compiler warning.
 * 
 * 4     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 3     11/09/98 2:11p Dave
 * Nebula optimizations.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 116   9/14/98 1:59p Allender
 * allow for interlaced movies
 * 
 * 115   5/25/98 10:32a John
 * Took out redundant code for font bitmap offsets that converted to a
 * float, then later on converted back to an integer.  Quite unnecessary.
 * 
 * 114   5/22/98 9:09a John
 * fixed type in previous
 * 
 * 113   5/22/98 9:07a John
 * added in code that fixed Rush Glide startup problemmo.
 * 
 * 112   5/20/98 9:46p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 111   5/17/98 5:03p John
 * Fixed some bugs that make the taskbar interfere with the DEBUG-only
 * mouse cursor.
 * 
 * 110   5/17/98 3:23p John
 * Took out capibility check for additive blending.  Made gr_bitmap_ex
 * clip properly in glide and direct3d.
 * 
 * 109   5/15/98 9:34p John
 * Removed the initial ugly little cursor part that drew right at program
 * start.
 * 
 * 108   5/15/98 8:48a John
 * Fixed bug where one-pixel line was getting left on right and bottom.
 * 
 * 107   5/14/98 5:42p John
 * Revamped the whole window position/mouse code for the graphics windows.
 * 
 * 106   5/08/98 5:38p John
 * Made Glide wait for retrace.
 * 
 * 105   5/08/98 5:37p John
 * 
 * 104   5/07/98 6:58p Hoffoss
 * Made changes to mouse code to fix a number of problems.
 * 
 * 103   5/07/98 8:43a John
 * Turned off 3dfx gamma correction.
 * 
 * 102   5/07/98 8:36a John
 * Fixed Glide gradients
 * 
 * 101   5/06/98 8:41p John
 * Fixed some font clipping bugs.   Moved texture handle set code for d3d
 * into the texture module.
 * 
 * 100   5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 *
 */

//#define USE_8BPP_TEXTURES 

#include <windows.h>
#include <windowsx.h>
#include "glide/glide.h"
#include "glide/glideutl.h"

#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "bmpman/bmpman.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "graphics/grinternal.h"
#include "graphics/grglide.h"
#include "graphics/line.h"
#include "graphics/font.h"
#include "io/mouse.h"
#include "io/key.h"
#include "globalincs/systemvars.h"
#include "graphics/grglideinternal.h"
#include "cfile/cfile.h"
#include "io/timer.h"
// #include "movie.h"
#include "nebula/neb.h"

#define NEBULA_COLORS 20

GrFog_t Glide_linear_fogtable[GR_FOG_TABLE_SIZE];	

int Glide_textures_in = 0;

int Glide_voodoo3 = 0;

static int Inited = 0;

// voodoo3 is a little sensitive to getting deactivated
#define VOODOO3_DEACTIVATED			-1
#define VOODOO3_INACTIVE()				( (Glide_voodoo3 == 1) && (Glide_deactivate == VOODOO3_DEACTIVATED) )

float Gr_gamma_lookup_float[256];

volatile int Glide_running = 0;
volatile int Glide_activate = 0;
volatile int Glide_deactivate = 0;

typedef enum gr_texture_source {
	TEXTURE_SOURCE_NONE,
	TEXTURE_SOURCE_DECAL,
} gr_texture_source;

typedef enum gr_color_source {
	COLOR_SOURCE_VERTEX,
	COLOR_SOURCE_TEXTURE,
	COLOR_SOURCE_VERTEX_TIMES_TEXTURE,
} gr_color_source;

typedef enum gr_alpha_source {
	ALPHA_SOURCE_VERTEX,
	ALPHA_SOURCE_VERTEX_NONDARKENING,
	ALPHA_SOURCE_TEXTURE,
	ALPHA_SOURCE_VERTEX_TIMES_TEXTURE,
} gr_alpha_source;

typedef enum gr_alpha_blend {
	ALPHA_BLEND_NONE,							// 1*SrcPixel + 0*DestPixel
	ALPHA_BLEND_ADDITIVE,					// 1*SrcPixel + 1*DestPixel
	ALPHA_BLEND_ALPHA_ADDITIVE,			// Alpha*SrcPixel + 1*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_ALPHA,		// Alpha*SrcPixel + (1-Alpha)*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR,	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
} gr_alpha_blend;

typedef enum gr_zbuffer_type {
	ZBUFFER_TYPE_NONE,
	ZBUFFER_TYPE_READ,
	ZBUFFER_TYPE_WRITE,
	ZBUFFER_TYPE_FULL,
} gr_zbuffer_type;

int Glide_last_state = -1;

void gr_glide_set_state( gr_texture_source ts, gr_color_source cs, gr_alpha_source as, gr_alpha_blend ab, gr_zbuffer_type zt )
{
	int current_state = 0;

	if(VOODOO3_INACTIVE()){
		return;
	}

	current_state = current_state | (ts<<0);
	current_state = current_state | (cs<<5);
	current_state = current_state | (as<<10);
	current_state = current_state | (ab<<15);
	current_state = current_state | (zt<<20);

	if ( current_state == Glide_last_state ) {
		return;
	}
	Glide_last_state = current_state;

	switch( ts )	{
	case TEXTURE_SOURCE_NONE:
		grTexCombineFunction( GR_TMU0, GR_TEXTURECOMBINE_ONE );
		break;
	case TEXTURE_SOURCE_DECAL:
		grTexCombineFunction( GR_TMU0, GR_TEXTURECOMBINE_DECAL );
		break;
	default:
		Int3();
	}

	switch( cs )	{
	case COLOR_SOURCE_VERTEX:
		grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE );
		break;
	
	case COLOR_SOURCE_TEXTURE:
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
		break;

	case COLOR_SOURCE_VERTEX_TIMES_TEXTURE:
		grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
		break;

	default:
		Int3();
	}

	switch( as )	{
	case ALPHA_SOURCE_VERTEX:
		grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE );
		grAlphaControlsITRGBLighting(FXFALSE);
		break;

	case ALPHA_SOURCE_VERTEX_NONDARKENING:
		grAlphaCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE );
		grAlphaControlsITRGBLighting(FXTRUE);
		grConstantColorValue(0xFFFFFFFF);			// Non-darkening colors will use this
		break;

	case ALPHA_SOURCE_TEXTURE:
		grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
		grAlphaControlsITRGBLighting(FXFALSE);
		break;

	case ALPHA_SOURCE_VERTEX_TIMES_TEXTURE:
		grAlphaCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL, GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
		grAlphaControlsITRGBLighting(FXFALSE);
		break;

	default:
		Int3();
	}


	switch( ab )	{
	case ALPHA_BLEND_NONE:							// 1*SrcPixel + 0*DestPixel
		grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO );
		break;

	case ALPHA_BLEND_ADDITIVE:						// 1*SrcPixel + 1*DestPixel
		grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO );
		break;

	case ALPHA_BLEND_ALPHA_ADDITIVE:				// Alpha*SrcPixel + 1*DestPixel
		grAlphaBlendFunction( GR_BLEND_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO);
		break;

	case ALPHA_BLEND_ALPHA_BLEND_ALPHA:			// Alpha*SrcPixel + (1-Alpha)*DestPixel
		grAlphaBlendFunction( GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ZERO, GR_BLEND_ZERO );
		break;

	case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
		grAlphaBlendFunction( GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO );
		break;

	default:
		Int3();
	}
	
	switch( zt )	{

	case ZBUFFER_TYPE_NONE:
		grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
		grDepthMask( FXFALSE );
		break;

	case ZBUFFER_TYPE_READ:
		grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
		grDepthBufferFunction(GR_CMP_LEQUAL);
		grDepthMask( FXFALSE );
		break;

	case ZBUFFER_TYPE_WRITE:
		grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
		grDepthBufferFunction(GR_CMP_ALWAYS);
		grDepthMask( FXTRUE );
		break;

	case ZBUFFER_TYPE_FULL:
		grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
		grDepthBufferFunction(GR_CMP_LEQUAL);
		grDepthMask( FXTRUE );
		break;

	default:
		Int3();
	}

}

/*
	gr_glide_set_state( 
	TEXTURE_SOURCE_NONE, TEXTURE_SOURCE_DECAL,

	COLOR_SOURCE_VERTEX, COLOR_SOURCE_TEXTURE, COLOR_SOURCE_VERTEX_TIMES_TEXTURE,

	ALPHA_SOURCE_VERTEX, ALPHA_SOURCE_TEXTURE, ALPHA_SOURCE_VERTEX_TIMES_TEXTURE,

	ALPHA_BLEND_NONE, ALPHA_BLEND_ADDITIVE, ALPHA_BLEND_ALPHA_ADDITIVE, ALPHA_BLEND_ALPHA_BLEND_ALPHA,	ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR,
	
	ZBUFFER_TYPE_NONE, ZBUFFER_TYPE_READ, ZBUFFER_TYPE_WRITE, ZBUFFER_TYPE_FULL,

	);

*/


// If mode is FALSE, turn zbuffer off the entire frame,
// no matter what people pass to gr_zbuffer_set.
void gr_glide_zbuffer_clear(int mode)
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( mode )	{
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		// Make sure zbuffering is on
		gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );

		// Disable writes to color and alpha buffers
		grColorMask( FXFALSE, FXFALSE );	
		// Clear the zbuffer
		grBufferClear( 0x0, 0, GR_WDEPTHVALUE_FARTHEST );
		// Re-enable writes to the color buffer
		grColorMask( FXTRUE, FXFALSE );	
	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
	}
}

int gr_glide_zbuffer_get()
{
	if ( !gr_global_zbuffering )	{
		return GR_ZBUFF_NONE;
	}
	return gr_zbuffering_mode;
}

int gr_glide_zbuffer_set(int mode)
{
	if ( !gr_global_zbuffering )	{
		gr_zbuffering = 0;
		return GR_ZBUFF_NONE;
	}

	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if ( gr_zbuffering_mode == GR_ZBUFF_NONE )	{
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	return tmp;
}


void gr_glide_pixel(int x, int y)
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	gr_line(x,y,x,y);
/*
	if ( x < gr_screen.clip_left ) return;
	if ( x > gr_screen.clip_right ) return;
	if ( y < gr_screen.clip_top ) return;
	if ( y > gr_screen.clip_bottom ) return;

	// Set up Render State - flat shading - alpha blending
	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	GrVertex p;
	p.x = 0.5f + i2fl(x + gr_screen.offset_x);
	p.y = 0.5f + i2fl(y + gr_screen.offset_y);
	p.a = 255.0f;
	p.r = i2fl(gr_screen.current_color.red);
	p.g = i2fl(gr_screen.current_color.green);
	p.b = i2fl(gr_screen.current_color.blue);

	grDrawPoint(&p);
*/
}

float snap(float x)
{
	int xi = fl2i(x*16.0f);
	return i2fl(xi)/16.0f;
}

void gr_glide_clear()
{
	GrColor_t clear_color;

	if(VOODOO3_INACTIVE()){
		return;
	}

	// Turn off zbuffering so this doesn't clear the zbuffer
	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

	// Clear the screen	
	clear_color = 0;
	clear_color |= ((ubyte)gr_screen.current_clear_color.red);
	clear_color |= ((ubyte)gr_screen.current_clear_color.green << 8);
	clear_color |= ((ubyte)gr_screen.current_clear_color.blue << 16);
	grBufferClear( clear_color, 0, GR_WDEPTHVALUE_FARTHEST );
}

static RECT Glide_cursor_clip_rect;

void gr_glide_clip_cursor(int active)
{
	if ( active  )	{
		ClipCursor(&Glide_cursor_clip_rect);
	} else {
		ClipCursor(NULL);
	}
}


int Gr_glide_mouse_saved = 0;
int Gr_glide_mouse_saved_x1 = 0;
int Gr_glide_mouse_saved_y1 = 0;
int Gr_glide_mouse_saved_x2 = 0;
int Gr_glide_mouse_saved_y2 = 0;
int Gr_glide_mouse_saved_w = 0;
int Gr_glide_mouse_saved_h = 0;
#define MAX_SAVE_SIZE (32*32)
ushort Gr_glide_mouse_saved_data[MAX_SAVE_SIZE];

// Clamps X between R1 and R2
#define CLAMP(x,r1,r2) do { if ( (x) < (r1) ) (x) = (r1); else if ((x) > (r2)) (x) = (r2); } while(0)

void gr_glide_save_mouse_area(int x, int y, int w, int h )
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	Gr_glide_mouse_saved_x1 = x; 
	Gr_glide_mouse_saved_y1 = y;
	Gr_glide_mouse_saved_x2 = x+w-1;
	Gr_glide_mouse_saved_y2 = y+h-1;
	 
	CLAMP(Gr_glide_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_glide_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_glide_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(Gr_glide_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );

	Gr_glide_mouse_saved_w = Gr_glide_mouse_saved_x2 - Gr_glide_mouse_saved_x1 + 1;
	Gr_glide_mouse_saved_h = Gr_glide_mouse_saved_y2 - Gr_glide_mouse_saved_y1 + 1;

	if ( Gr_glide_mouse_saved_w < 1 ) return;
	if ( Gr_glide_mouse_saved_h < 1 ) return;

	// Make sure we're not saving too much!
	Assert( (Gr_glide_mouse_saved_w*Gr_glide_mouse_saved_h) <= MAX_SAVE_SIZE );

	GrLfbInfo_t info;

	info.size=sizeof(GrLfbInfo_t);

	// get a read pointer 
	if ( grLfbLock(	GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565,
							GR_ORIGIN_UPPER_LEFT, FXFALSE, &info))		{

		ushort *rptr;
		int short_per_row=info.strideInBytes/2;

		rptr = (ushort *)info.lfbPtr;

		ushort *sptr, *dptr;

		dptr = Gr_glide_mouse_saved_data;

		for (int i=0; i<Gr_glide_mouse_saved_h; i++ )	{
			sptr = &rptr[(Gr_glide_mouse_saved_y1+i)*short_per_row+Gr_glide_mouse_saved_x1];

			for(int j=0; j<Gr_glide_mouse_saved_w; j++ )	{
				*dptr++ = *sptr++;
			}
		}


		// Release the lock
		grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER );

		Gr_glide_mouse_saved = 1;

	}  else {
		mprintf(( "Couldn't get read-only lock to backbuffer for glide mouse save\n" ));
	}

}

// lock the backbuffer and return a pointer
uint gr_glide_lock()
{
	GrLfbInfo_t info;

	if(VOODOO3_INACTIVE()){
		return NULL;
	}

	info.size=sizeof(GrLfbInfo_t);
	if(grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_1555, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)){		
		return (uint)info.lfbPtr;
	} 

	// fail
	return NULL;
}

// unlock the backbuffer
void gr_glide_unlock()
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	// Release the lock
	grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER );
}

static int gr_palette_faded_out = 0;

void gr_glide_flip()
{
	int cnt;	

	if(VOODOO3_INACTIVE()){
		return;
	}
	
	// voodoo3 ?	
	if(!Glide_voodoo3){
		cnt = Glide_activate;
		if ( cnt )	{
			Glide_activate -= cnt;

			gr_glide_clip_cursor(1);
			grSstControl(GR_CONTROL_ACTIVATE);
		}

		cnt = Glide_deactivate;
		if ( cnt )	{
			Glide_deactivate -= cnt;

			gr_glide_clip_cursor(0);
			grSstControl(GR_CONTROL_DEACTIVATE);
		}
	}	

	gr_reset_clip();

	if ( gr_palette_faded_out )	{
		return;
	}

	int mx, my;

	Gr_glide_mouse_saved = 0;		// assume not saved

	mouse_eval_deltas();
	if ( mouse_is_visible() )	{
		gr_reset_clip();
		mouse_get_pos( &mx, &my );
		gr_glide_save_mouse_area(mx,my,32,32);
		if ( Gr_cursor == -1 )	{
			gr_set_color(255,255,255);
			gr_line( mx, my, mx+7, my + 7 );
			gr_line( mx, my, mx+5, my );
			gr_line( mx, my, mx, my+5 );
		} else {
			gr_set_bitmap(Gr_cursor);
			gr_bitmap( mx, my );
		}
	} 

#ifndef NDEBUG
	if(Interface_framerate){
		if(Interface_last_tick > 0){		
			int diff = timer_get_milliseconds() - Interface_last_tick;
			gr_printf(10, 10, "%f", 1000.0f / (float)diff);
		}
		Interface_last_tick = timer_get_milliseconds();
	}
#endif

#ifndef NDEBUG
	grBufferSwap( 0 );
#else
	grBufferSwap( 1 );
#endif
   grSstIdle();

	glide_tcache_frame();
}

void gr_glide_flip_window(uint _hdc, int x, int y, int w, int h )
{
}

void gr_glide_set_clip(int x,int y,int w,int h)
{
	gr_screen.offset_x = x;
	gr_screen.offset_y = y;

	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;

	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;

	if(VOODOO3_INACTIVE()){
		return;
	}

	// check for sanity of parameters
	if ( gr_screen.clip_left+x < 0 ) {
		gr_screen.clip_left = -x;
	} else if ( gr_screen.clip_left+x > gr_screen.max_w-1 )	{
		gr_screen.clip_left = gr_screen.max_w-1-x;
	}
	if ( gr_screen.clip_right+x < 0 ) {
		gr_screen.clip_right = -x;
	} else if ( gr_screen.clip_right+x >= gr_screen.max_w-1 )	{
		gr_screen.clip_right = gr_screen.max_w-1-x;
	}

	if ( gr_screen.clip_top+y < 0 ) {
		gr_screen.clip_top = -y;
	} else if ( gr_screen.clip_top+y > gr_screen.max_h-1 )	{
		gr_screen.clip_top = gr_screen.max_h-1-y;
	}

	if ( gr_screen.clip_bottom+y < 0 ) {
		gr_screen.clip_bottom = -y;
	} else if ( gr_screen.clip_bottom+y > gr_screen.max_h-1 )	{
		gr_screen.clip_bottom = gr_screen.max_h-1-y;
	}

	gr_screen.clip_width = gr_screen.clip_right - gr_screen.clip_left + 1;
	gr_screen.clip_height = gr_screen.clip_bottom - gr_screen.clip_top + 1;

	grClipWindow( gr_screen.clip_left+x, gr_screen.clip_top+y, gr_screen.clip_right+1+x, gr_screen.clip_bottom+1+y );
}

void gr_glide_reset_clip()
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;

	grClipWindow( gr_screen.clip_left, gr_screen.clip_top, gr_screen.clip_right+1, gr_screen.clip_bottom+1 );
}


void gr_glide_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;

	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
}

void gr_glide_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
	
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;
}

void gr_glide_set_shader( shader * shade )
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

void gr_glide_bitmap_ex_internal(int x,int y,int w,int h,int sx,int sy)
{
	int i,j;
	bitmap * bmp;
	ushort * sptr;

	if(VOODOO3_INACTIVE()){
		return;
	}

	bmp = bm_lock( gr_screen.current_bitmap, 16, 0 );
	sptr = (ushort *)( bmp->data + (sy*bmp->w*2) + (sx*2) );

	if ( x < gr_screen.clip_left ) return;
	if ( x > gr_screen.clip_right ) return;
	if ( y < gr_screen.clip_top ) return;
	if ( y > gr_screen.clip_bottom ) return;

	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );	
	
	GrLfbInfo_t info;

	if ( grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_1555, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info) ) {

		ushort *vram = (ushort *)info.lfbPtr;
		int stride = info.strideInBytes / sizeof(ushort);
		
		for (i=0; i<h; i++ )	{
				
			ushort *dptr = &vram[stride*(gr_screen.offset_y+i+y)+(gr_screen.offset_x+x)];			
			
			for ( j=0; j<w; j++ )	{
				if(sptr[j] & 0x8000){
					*dptr = sptr[j];
				}

				dptr++;
			}			
			sptr += bmp->w;
		}

		grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
	}	

	bm_unlock(gr_screen.current_bitmap);
}


void gr_glide_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	gr_glide_bitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}


void gr_glide_bitmap(int x, int y)
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

	gr_glide_bitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}


static void glide_scanline( int x1, int x2, int y )
{
	int w;

	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( x1 > x2 )	{
		int tmp = x1;
		x1 = x2;
		x2 = tmp;
	}

	if ( y < gr_screen.clip_top ) return;
	if ( y > gr_screen.clip_bottom ) return;
	
	if ( x1 < gr_screen.clip_left ) x1 = gr_screen.clip_left;
	if ( x2 > gr_screen.clip_right ) x2 = gr_screen.clip_right;

	w = x2 - x1 + 1;

	if ( w < 1 ) return;

//	for (i=0; i<w; i++)
//		gr_pixel(x1+i,y);


	// Set up Render State - flat shading - alpha blending
	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
	
	GrVertex a,b;

	a.x = i2fl(x1 + gr_screen.offset_x);
	a.y = i2fl(y + gr_screen.offset_y);
 	a.r = i2fl(gr_screen.current_color.red);
 	a.g = i2fl(gr_screen.current_color.green);
 	a.b = i2fl(gr_screen.current_color.blue);
 	a.a = i2fl(gr_screen.current_color.alpha);

	b.x = i2fl(x2 + gr_screen.offset_x);
	b.y = i2fl(y + gr_screen.offset_y);
 	b.r = i2fl(gr_screen.current_color.red);
 	b.g = i2fl(gr_screen.current_color.green);
 	b.b = i2fl(gr_screen.current_color.blue);
 	b.a = i2fl(gr_screen.current_color.alpha);

	if (a.x<b.x) {
		a.x+=0.5f;
		b.x+=1.0f;
	} else {
		b.x+=0.5f;
		a.x+=1.0f;
	}

	if (a.y<b.y)	{
		a.y+=0.5f;
		b.y+=1.0f;
	} else {
		b.y+=0.5f;
		a.y+=1.0f;
	}

	grDrawLine(&a, &b);
}


void gr_glide_rect(int x,int y,int w,int h)
{
	int swapped=0;
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

	for (int i=0; i<h; i++ )
		glide_scanline( x1, x2, y1+i );
}


static void glide_shade_scanline( int x1, int x2, int y, int r, int g, int b, int a )
{
	int w;

	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( x1 > x2 )	{
		int tmp = x1;
		x1 = x2;
		x2 = tmp;
	}

	if ( y < gr_screen.clip_top ) return;
	if ( y > gr_screen.clip_bottom ) return;
	
	if ( x1 < gr_screen.clip_left ) x1 = gr_screen.clip_left;
	if ( x2 > gr_screen.clip_right ) x2 = gr_screen.clip_right;

	w = x2 - x1 + 1;

	if ( w < 1 ) return;


	// Set up Render State - flat shading - alpha blending
	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	GrVertex v1,v2;

	v1.x = i2fl(x1 + gr_screen.offset_x);
	v1.y = i2fl(y + gr_screen.offset_y);
 	v1.r = i2fl(r);
 	v1.g = i2fl(g);
 	v1.b = i2fl(b);
 	v1.a = i2fl(a);

	v2.x = i2fl(x2 + gr_screen.offset_x);
	v2.y = i2fl(y + gr_screen.offset_y);
 	v2.r = i2fl(r);
 	v2.g = i2fl(g);
 	v2.b = i2fl(b);
 	v2.a = i2fl(a);

	if (v1.x<v2.x) {
		v1.x+=0.5f;
		v2.x+=1.0f;
	} else {
		v2.x+=0.5f;
		v1.x+=1.0f;
	}

	if (v1.y<v2.y)	{
		v1.y+=0.5f;
		v2.y+=1.0f;
	} else {
		v2.y+=0.5f;
		v1.y+=1.0f;
	}

	grDrawLine(&v1, &v2);

}

float shade1 = 1.0f;
float shade2 = 6.0f;

DCF(shade1,"Set shade1")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_FLOAT )	{
			shade1 = Dc_arg_float;
		}
	}
}

DCF(shade2,"Set shade2")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_FLOAT )	{
			shade2 = Dc_arg_float;
		}
	}
}


void gr_glide_shade(int x,int y,int w,int h)
{
	int swapped=0;
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

	int r,g,b,a;
	r = fl2i(gr_screen.current_shader.r*255.0f*shade1);
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	g = fl2i(gr_screen.current_shader.g*255.0f*shade1);
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	b = fl2i(gr_screen.current_shader.b*255.0f*shade1);
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	a = fl2i(gr_screen.current_shader.c*255.0f*shade2);
	if ( a < 0 ) a = 0; else if ( a > 255 ) a = 255;

	for (int i=0; i<h; i++ )	{
		glide_shade_scanline( x1, x2, y1+i, r, g, b, a );
	}
}


void gr_glide_aabitmap_ex_new(int x,int y,int w,int h,int sx,int sy);

void gr_glide_char(int x,int y,int letter)
{
	font_char *ch;
	
	ch = &Current_font->char_data[letter];
	
	int sx = Current_font->bm_u[letter];
	int sy = Current_font->bm_v[letter];

	gr_glide_aabitmap_ex_new( x, y, ch->byte_width, Current_font->h, sx, sy );

//	mprintf(( "String = %s\n", text ));
}


void gr_glide_string( int sx, int sy, char *s )
{
	int width, spacing, letter;
	int x, y;

	if ( !Current_font )	{
		return;
	}

	gr_set_bitmap(Current_font->bitmap_id);

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

		if (letter<0) {	//not in font, draw as space
			continue;
		}

		gr_glide_char( x, y, letter );
	}
}

void gr_glide_circle( int xc, int yc, int d )
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
		glide_scanline( xc-y, xc+y, yc-x );
		glide_scanline( xc-y, xc+y, yc+x );

		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			glide_scanline( xc-x, xc+x, yc-y );
			glide_scanline( xc-x, xc+x, yc+y );
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y)	{
		glide_scanline( xc-x, xc+x, yc-y );
		glide_scanline( xc-x, xc+x, yc+y );
	}
	return;
}

void gr_glide_line(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	if(VOODOO3_INACTIVE()){
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	// Set up Render State - flat shading - alpha blending
	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	GrVertex a,b;

	a.x = i2fl(x1 + gr_screen.offset_x);
	a.y = i2fl(y1 + gr_screen.offset_y);
 	a.r = i2fl(gr_screen.current_color.red);
 	a.g = i2fl(gr_screen.current_color.green);
 	a.b = i2fl(gr_screen.current_color.blue);
 	a.a = i2fl(gr_screen.current_color.alpha);

	b.x = i2fl(x2 + gr_screen.offset_x);
	b.y = i2fl(y2 + gr_screen.offset_y);
 	b.r = i2fl(gr_screen.current_color.red);
 	b.g = i2fl(gr_screen.current_color.green);
 	b.b = i2fl(gr_screen.current_color.blue);
 	b.a = i2fl(gr_screen.current_color.alpha);

	if (a.x<b.x) {
		a.x+=0.5f;
		b.x+=1.0f;
	} else {
		b.x+=0.5f;
		a.x+=1.0f;
	}

	if (a.y<b.y)	{
		a.y+=0.5f;
		b.y+=1.0f;
	} else {
		b.y+=0.5f;
		a.y+=1.0f;
	}

	grDrawLine(&a, &b);
}


void gr_glide_aaline( vertex *v1, vertex *v2 )
{
	float x1, y1, x2, y2;
	int clipped = 0, swapped = 0;
	float a1, b1, a2, b2;

	if(VOODOO3_INACTIVE()){
		return;
	}

	x1 = v1->sx;
	y1 = v1->sy;

	x2 = v2->sx;
	y2 = v2->sy;

	a1 = (float)gr_screen.clip_left;
	b1 = (float)gr_screen.clip_top;
	a2 = (float)gr_screen.clip_right;
	b2 = (float)gr_screen.clip_bottom;

	FL_CLIPLINE(x1,y1,x2,y2,a1,b1,a2,b2,return,clipped=1,swapped=1);

	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	GrVertex a,b;

	a.x = i2fl(fl2i(x1)) + gr_screen.offset_x;
	a.y = i2fl(fl2i(y1)) + gr_screen.offset_y;
 	a.r = i2fl(gr_screen.current_color.red);
 	a.g = i2fl(gr_screen.current_color.green);
 	a.b = i2fl(gr_screen.current_color.blue);
 	a.a = i2fl(gr_screen.current_color.alpha);

	b.x = i2fl(fl2i(x2)) + gr_screen.offset_x;
	b.y = i2fl(fl2i(y2)) + gr_screen.offset_y;
 	b.r = i2fl(gr_screen.current_color.red);
 	b.g = i2fl(gr_screen.current_color.green);
 	b.b = i2fl(gr_screen.current_color.blue);
 	b.a = i2fl(gr_screen.current_color.alpha);

	if (a.x<b.x) {
		a.x+=0.5f;
		b.x+=1.0f;
	} else {
		b.x+=0.5f;
		a.x+=1.0f;
	}

	if (a.y<b.y)	{
		a.y+=0.5f;
		b.y+=1.0f;
	} else {
		b.y+=0.5f;
		a.y+=1.0f;
	}

	grAADrawLine(&a, &b);
}

void gr_glide_tmapper_internal( int nv, vertex * verts[], uint flags, int is_scaler )
{
	GrVertex GrVerts[25];
	int i;
	float Glide_u_ratio = 0.0f;
	float Glide_v_ratio = 0.0f;

	if(VOODOO3_INACTIVE()){
		return;
	}

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA )	{
		Int3();
		/*
		flags |= TMAP_FLAG_TEXTURED;

		static int test_bmp = -1;
		static ushort data[16];		
		if ( test_bmp == -1 ){
			ushort pix;
			ubyte a, r, g, b;
			int idx;
			
			// stuff the fake bitmap
			a = 1; r = 255; g = 255; b = 255;
			pix = 0;
			bm_set_components((ubyte*)&pix, &r, &g, &b, &a);			
			for(idx=0; idx<16; idx++){
				data[idx] = pix;
			}			
			test_bmp = bm_create( 16, 4, 4, data );
		}
		gr_set_bitmap( test_bmp );

		for (i=0; i<nv; i++ )	{
			verts[i]->u = verts[i]->v = 0.5f;
		}
		*/
	}

	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_color_source color_source = (gr_color_source)-1;
	gr_alpha_source alpha_source = (gr_alpha_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;

	if ( gr_zbuffering )	{
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	)	{
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	float alpha;

	alpha_source = ALPHA_SOURCE_VERTEX;

	switch(gr_screen.current_alphablend_mode){
	case GR_ALPHABLEND_FILTER:
		// Blend with screen pixel using src*alpha+dst				
		if ( gr_screen.current_alpha > 1.0f )	{
			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR;
			alpha = 255.0f;
		} else {
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;
			alpha = gr_screen.current_alpha*255.0f;
		}
		break;	

	default :	
		alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		alpha = 255.0f;
		break;
	}

	if ( flags & TMAP_FLAG_TEXTURED )	{
		int texture_type = TCACHE_TYPE_NORMAL;

		if ( flags & TMAP_FLAG_NONDARKENING )	{
			alpha_source = ALPHA_SOURCE_VERTEX_NONDARKENING;			
			//alpha_source = ALPHA_SOURCE_VERTEX_TIMES_TEXTURE;
			texture_type = TCACHE_TYPE_NONDARKENING;
		} else {			
			alpha_source = ALPHA_SOURCE_VERTEX_TIMES_TEXTURE;
			texture_type = TCACHE_TYPE_XPARENT;
		}

		// common settings
		color_source = COLOR_SOURCE_VERTEX_TIMES_TEXTURE;
		texture_source = TEXTURE_SOURCE_DECAL;

		// force texture type
		if(flags & TMAP_FLAG_BITMAP_SECTION){
			texture_type = TCACHE_TYPE_BITMAP_SECTION;
		}
		
		if ( !gr_tcache_set( gr_screen.current_bitmap, texture_type, &Glide_u_ratio, &Glide_v_ratio, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))	{
			// Error setting texture!
			mprintf(( "GLIDE: Error setting texture!\n" ));
			// Mark as no texturing
			color_source = COLOR_SOURCE_VERTEX;
			texture_source = TEXTURE_SOURCE_NONE;
		}		
	} else {
		color_source = COLOR_SOURCE_VERTEX;		
		texture_source = TEXTURE_SOURCE_NONE;
		if(flags & TMAP_FLAG_ALPHA){
			alpha_source = ALPHA_SOURCE_VERTEX;
		}
	}

//	zbuffer_type = ZBUFFER_TYPE_NONE;
//	alpha_source = ALPHA_SOURCE_VERTEX;
//	alpha_blend = ALPHA_BLEND_NONE;
//	alpha = 255.0f;
//	color_source = COLOR_SOURCE_VERTEX;
//	texture_source = TEXTURE_SOURCE_NONE;	
	gr_glide_set_state( texture_source, color_source, alpha_source, alpha_blend, zbuffer_type );

	int x1, y1, x2, y2;
	x1 = gr_screen.clip_left*16;
	x2 = gr_screen.clip_right*16+15;
	y1 = gr_screen.clip_top*16;
	y2 = gr_screen.clip_bottom*16+15;	
	
//	memset( GrVerts, 0, sizeof(GrVertex) * 25 );

	for (i=0; i<nv; i++ )	{
		if ( flags & TMAP_FLAG_ALPHA )	{			
			GrVerts[i].a = i2fl(verts[i]->a);							
		} else {
			GrVerts[i].a = alpha;
		}

		if ( flags & TMAP_FLAG_NEBULA )	{
			int pal = (verts[i]->b*(NEBULA_COLORS-1))/255;
			GrVerts[i].r = i2fl(gr_palette[pal*3+0]);
			GrVerts[i].g = i2fl(gr_palette[pal*3+1]);
			GrVerts[i].b = i2fl(gr_palette[pal*3+2]);
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) )	{
			GrVerts[i].r = Gr_gamma_lookup_float[verts[i]->b];
			GrVerts[i].g = Gr_gamma_lookup_float[verts[i]->b];
			GrVerts[i].b = Gr_gamma_lookup_float[verts[i]->b];
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{			
			// Make 0.75 be 256.0f
			GrVerts[i].r = Gr_gamma_lookup_float[verts[i]->r];
			GrVerts[i].g = Gr_gamma_lookup_float[verts[i]->g];
			GrVerts[i].b = Gr_gamma_lookup_float[verts[i]->b];
		} else	{
			if ( flags & TMAP_FLAG_TEXTURED )	{
				GrVerts[i].r = 255.0f;
				GrVerts[i].g = 255.0f;
				GrVerts[i].b = 255.0f;
			} else {
				GrVerts[i].r = i2fl(gr_screen.current_color.red);
				GrVerts[i].g = i2fl(gr_screen.current_color.green);
				GrVerts[i].b = i2fl(gr_screen.current_color.blue);
			}
		}

		int x, y;
		x = fl2i(verts[i]->sx*16.0f);
		y = fl2i(verts[i]->sy*16.0f);

		if ( flags & TMAP_FLAG_CORRECT )	{
			// "clip" it 
			if ( x < x1 ) {
				x = x1;
			} else if ( x > x2 )	{
				x = x2;
			}
			if ( y < y1 )	{
				y = y1;
			} else if ( y > y2 )	{
				y = y2;
			}
		}

		x += gr_screen.offset_x*16;
		y += gr_screen.offset_y*16;
		
		GrVerts[i].x = i2fl(x) / 16.0f;
		GrVerts[i].y = i2fl(y) / 16.0f;

		//verts[i]->sw = 1.0f;

		GrVerts[i].oow=verts[i]->sw;
	
		if ( flags & TMAP_FLAG_TEXTURED )	{
			GrVerts[i].tmuvtx[GR_TMU0].oow=verts[i]->sw;
			GrVerts[i].tmuvtx[GR_TMU0].sow=verts[i]->u * verts[i]->sw * Glide_u_ratio;
			GrVerts[i].tmuvtx[GR_TMU0].tow=verts[i]->v * verts[i]->sw * Glide_v_ratio;
		}
	}

	// if we're rendering against a fullneb background
	if(flags & TMAP_FLAG_PIXEL_FOG){	
		int r, g, b;
		int ra, ga, ba;		
		ra = ga = ba = 0;		

		// get the average pixel color behind the vertices
		for(i=0; i<nv; i++){			
			neb2_get_pixel((int)GrVerts[i].x, (int)GrVerts[i].y, &r, &g, &b);
			ra += r;
			ga += g;
			ba += b;
		}		
		
		ra /= nv;
		ga /= nv;
		ba /= nv;		

		// set fog
		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}
					
	if ( flags & TMAP_FLAG_CORRECT )	{
		grDrawPolygonVertexList( nv, GrVerts );
	} else {
		for (i=1; i<nv-1; i++ )	{
			guDrawTriangleWithClip(&GrVerts[0],&GrVerts[i],&GrVerts[i+1]);
		}
	} 

	// turn off fog
	// if(flags & TMAP_FLAG_PIXEL_FOG){
		// gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	// }
}

void gr_glide_tmapper( int nv, vertex * verts[], uint flags )
{
	gr_glide_tmapper_internal( nv, verts, flags, 0 );
}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

void gr_glide_aascaler(vertex *va, vertex *vb )
{
					
}

void gr_glide_scaler(vertex *va, vertex *vb )
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	if(VOODOO3_INACTIVE()){
		return;
	}

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

	vertex v[4];
	vertex *vl[4];

	vl[0] = &v[0];	
	v->sx = clipped_x0;
	v->sy = clipped_y0;
	v->sw = va->sw;
	v->z = va->z;
	v->u = clipped_u0;
	v->v = clipped_v0;

	vl[1] = &v[1];	
	v[1].sx = clipped_x1;
	v[1].sy = clipped_y0;
	v[1].sw = va->sw;
	v[1].z = va->z;
	v[1].u = clipped_u1;
	v[1].v = clipped_v0;

	vl[2] = &v[2];	
	v[2].sx = clipped_x1;
	v[2].sy = clipped_y1;
	v[2].sw = va->sw;
	v[2].z = va->z;
	v[2].u = clipped_u1;
	v[2].v = clipped_v1;

	vl[3] = &v[3];	
	v[3].sx = clipped_x0;
	v[3].sy = clipped_y1;
	v[3].sw = va->sw;
	v[3].z = va->z;
	v[3].u = clipped_u0;
	v[3].v = clipped_v1;

	//glide_zbuffering(0);
	gr_glide_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
}


void gr_glide_aabitmap_ex_new(int x,int y,int w,int h,int sx,int sy)
{
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	if ( !gr_screen.current_color.is_alphacolor )	return;

	if(VOODOO3_INACTIVE()){
		return;
	}

//	mprintf(( "x=%d, y=%d, w=%d, h=%d\n", x, y, w, h ));
//	mprintf(( "sx=%d, sy=%d, bw=%d, bh=%d\n", sx, sy, bmp->w, bmp->h ));

	float Glide_u_ratio;
	float Glide_v_ratio;

	// Set up Render State - flat shading - alpha blending
	gr_glide_set_state( TEXTURE_SOURCE_DECAL, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX_TIMES_TEXTURE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &Glide_u_ratio, &Glide_v_ratio ) )	{
		// Couldn't set texture
		mprintf(( "GLIDE: Error setting aabitmap texture!\n" ));
		return;
	}

	GrVertex GrVerts[4];

	float u0, u1, v0, v1;
	float r,g,b,a;

	r = i2fl(gr_screen.current_color.red);
	g = i2fl(gr_screen.current_color.green);
	b = i2fl(gr_screen.current_color.blue);
	a = i2fl(gr_screen.current_color.alpha);

	int bw, bh;

	bm_get_info( gr_screen.current_bitmap, &bw, &bh );
	
	u0 = Glide_u_ratio*i2fl(sx)/i2fl(bw);
	v0 = Glide_v_ratio*i2fl(sy)/i2fl(bh);

	u1 = Glide_u_ratio*i2fl(sx+w)/i2fl(bw);
	v1 = Glide_v_ratio*i2fl(sy+h)/i2fl(bh);

	float x1, x2, y1, y2;
	x1 = i2fl(x+gr_screen.offset_x);
	y1 = i2fl(y+gr_screen.offset_y);
	x2 = i2fl(x+w+gr_screen.offset_x);
	y2 = i2fl(y+h+gr_screen.offset_y);

	int i;

	i = 0;
	GrVerts[i].x = x1;
	GrVerts[i].y = y1;
	GrVerts[i].oow = 1.0f;
	GrVerts[i].r = r;
	GrVerts[i].g = g;
	GrVerts[i].b = b;
	GrVerts[i].a = a;
	GrVerts[i].tmuvtx[GR_TMU0].sow=u0;
	GrVerts[i].tmuvtx[GR_TMU0].tow=v0;

	i = 1;
	GrVerts[i].x = x2;
	GrVerts[i].y = y1;
	GrVerts[i].oow = 1.0f;
	GrVerts[i].r = r;
	GrVerts[i].g = g;
	GrVerts[i].b = b;
	GrVerts[i].a = a;
	GrVerts[i].tmuvtx[GR_TMU0].sow=u1;
	GrVerts[i].tmuvtx[GR_TMU0].tow=v0;

	i = 2;
	GrVerts[i].x = x2;
	GrVerts[i].y = y2;
	GrVerts[i].oow = 1.0f;
	GrVerts[i].r = r;
	GrVerts[i].g = g;
	GrVerts[i].b = b;
	GrVerts[i].a = a;
	GrVerts[i].tmuvtx[GR_TMU0].sow=u1;
	GrVerts[i].tmuvtx[GR_TMU0].tow=v1;

	i = 3;
	GrVerts[i].x = x1;
	GrVerts[i].y = y2;
	GrVerts[i].oow = 1.0f;
	GrVerts[i].r = r;
	GrVerts[i].g = g;
	GrVerts[i].b = b;
	GrVerts[i].a = a;
	GrVerts[i].tmuvtx[GR_TMU0].sow=u0;
	GrVerts[i].tmuvtx[GR_TMU0].tow=v1;

	grDrawPolygonVertexList( 4, GrVerts );
}



void gr_glide_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	gr_glide_aabitmap_ex_new(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_glide_string_hack( int sx, int sy, char *s )
{
	int width, spacing, letter;
	int x, y;

	if ( !Current_font )	{
		return;
	}

	gr_set_bitmap(Current_font->bitmap_id);

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

		if (letter<0) {	//not in font, draw as space
			continue;
		}

		// formerly a call to gr_glide_char(...)
		{
			font_char *ch;
	
			ch = &Current_font->char_data[letter];
			
			int _sx = Current_font->bm_u[letter];
			int _sy = Current_font->bm_v[letter];

			gr_glide_aabitmap_ex( x, y, ch->byte_width, Current_font->h, _sx, _sy );
		}		
	}
}

void gr_glide_aabitmap(int x, int y)
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


void gr_glide_gradient(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( !gr_screen.current_color.is_alphacolor )	{
		gr_line( x1, y1, x2, y2 );
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);   

	// Set up Render State - flat shading - alpha blending
	gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	GrVertex a,b;

	a.x = i2fl(x1 + gr_screen.offset_x);
	a.y = i2fl(y1 + gr_screen.offset_y);
 	a.r = i2fl(gr_screen.current_color.red);
 	a.g = i2fl(gr_screen.current_color.green);
 	a.b = i2fl(gr_screen.current_color.blue);

	b.x = i2fl(x2 + gr_screen.offset_x);
	b.y = i2fl(y2 + gr_screen.offset_y);
 	b.r = i2fl(gr_screen.current_color.red);
 	b.g = i2fl(gr_screen.current_color.green);
 	b.b = i2fl(gr_screen.current_color.blue);

	if ( swapped )	{
		a.a = 0.0f;
		b.a = i2fl(gr_screen.current_color.alpha);
	} else {
		b.a = 0.0f;
		a.a = i2fl(gr_screen.current_color.alpha);
	}

	if (a.x<b.x) {
		a.x+=0.5f;
		b.x+=1.0f;
	} else {
		b.x+=0.5f;
		a.x+=1.0f;
	}

	if (a.y<b.y)	{
		a.y+=0.5f;
		b.y+=1.0f;
	} else {
		b.y+=0.5f;
		a.y+=1.0f;
	}

	grDrawLine(&a, &b);
}

void gr_glide_set_palette(ubyte *new_palette, int force_flag )
{
	gr_palette_faded_out = 0;

	if(VOODOO3_INACTIVE()){
		return;
	}

#ifdef USE_8BPP_TEXTURES
	GuTexPalette palette;

	glide_free_textures();

	int i;

	for (i=0;i<256; i++ )	{
		palette.data[i] = 0xFF000000;
		palette.data[i] |= new_palette[i*3+2];
		palette.data[i] |= new_palette[i*3+1]<<8;
		palette.data[i] |= new_palette[i*3+0]<<16;
	}

	grTexDownloadTable( GR_TMU0, GR_TEXTABLE_PALETTE, &palette );		
#endif
	
}

void gr_glide_init_color(color *c, int r, int g, int b)
{
	c->screen_sig = gr_screen.signature;
	c->red = unsigned char(r);
	c->green = unsigned char(g);
	c->blue = unsigned char(b);
	c->alpha = 255;
	c->ac_type = AC_TYPE_NONE;
	c->alphacolor = -1;
	c->is_alphacolor = 0;
	c->magic = 0xAC01;
}

void gr_glide_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( alpha < 0 ) alpha = 0; else if ( alpha > 255 ) alpha = 255;

	gr_glide_init_color( clr, r, g, b );

	clr->alpha = unsigned char(alpha);
	clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

void gr_glide_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_glide_init_color( &gr_screen.current_color, r, g, b );
}

void gr_glide_get_color( int * r, int * g, int * b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

void gr_glide_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )	{
			gr_glide_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_glide_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}
	gr_screen.current_color = *dst;
}



void gr_glide_flash(int r, int g, int b)
{
	int Flash_r = r;  
	int Flash_g = g;
	int Flash_b = b;

	if(VOODOO3_INACTIVE()){
		return;
	}

	CAP(Flash_r,0,255);
	CAP(Flash_g,0,255);
	CAP(Flash_b,0,255);

	if ( Flash_r || Flash_g || Flash_b )	{
		gr_glide_set_state( TEXTURE_SOURCE_NONE, COLOR_SOURCE_VERTEX, ALPHA_SOURCE_VERTEX, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
	
		GrVertex GrVerts[4];

		float r,g,b,a;

		r = i2fl(Flash_r);
		g = i2fl(Flash_g);
		b = i2fl(Flash_b);
		a = i2fl(255);

		float x1, x2, y1, y2;
		x1 = i2fl(gr_screen.clip_left+gr_screen.offset_x);
		y1 = i2fl(gr_screen.clip_top+gr_screen.offset_y);
		x2 = i2fl(gr_screen.clip_right+gr_screen.offset_x);
		y2 = i2fl(gr_screen.clip_bottom+gr_screen.offset_y);
	
		int i;

		i = 0;
		GrVerts[i].x = x1;
		GrVerts[i].y = y1;
		GrVerts[i].r = r;
		GrVerts[i].g = g;
		GrVerts[i].b = b;
		GrVerts[i].a = a;

		i = 1;
		GrVerts[i].x = x2;
		GrVerts[i].y = y1;
		GrVerts[i].r = r;
		GrVerts[i].g = g;
		GrVerts[i].b = b;
		GrVerts[i].a = a;

		i = 2;
		GrVerts[i].x = x2;
		GrVerts[i].y = y2;
		GrVerts[i].r = r;
		GrVerts[i].g = g;
		GrVerts[i].b = b;
		GrVerts[i].a = a;

		i = 3;
		GrVerts[i].x = x1;
		GrVerts[i].y = y2;
		GrVerts[i].r = r;
		GrVerts[i].g = g;
		GrVerts[i].b = b;
		GrVerts[i].a = a;

		grDrawPolygonVertexList( 4, GrVerts );
	}

}



void gr_glide_activate(int active)
{
	if (!Glide_running)	{
		return;
	}	

	mprintf(( "Glide activate: %d\n", active ));

	// voodoo3
	if(Glide_voodoo3){		
		// choose resolution
		GrScreenResolution_t res_mode;
		if((gr_screen.max_w == 1024) && (gr_screen.max_h == 768)){
			res_mode = GR_RESOLUTION_1024x768;
		} else {
			res_mode = GR_RESOLUTION_640x480;
		}	

		HWND hwnd = (HWND)os_get_window();	
		
		if ( active  )	{			
			// already active
			if(Glide_deactivate == 0){
				return;
			}

			Glide_deactivate = 0;

			if ( hwnd )	{						
				SetActiveWindow(hwnd);
				SetForegroundWindow(hwnd);
				grSstWinOpen( (DWORD)hwnd, res_mode, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR, GR_ORIGIN_UPPER_LEFT, 2, 1 );
				ShowWindow(hwnd,SW_MAXIMIZE);
				gr_glide_clip_cursor(1);
				glide_tcache_init();
				grGammaCorrectionValue(1.0f);								
			}
		} else {			
			// already deactivated
			if(Glide_deactivate == VOODOO3_DEACTIVATED){
				return;
			}

			Glide_deactivate = VOODOO3_DEACTIVATED;

			if ( hwnd )	{
				gr_glide_clip_cursor(0);
				ShowWindow(hwnd,SW_MINIMIZE);			
				grSstWinClose();
			}
		}
	} else {
		HWND hwnd = (HWND)os_get_window();
	
		if ( active  )	{
			Glide_activate++;			

			if ( hwnd )	{
				// SetActiveWindow(hwnd);
				// SetForegroundWindow(hwnd);				
				ShowWindow(hwnd,SW_RESTORE);
				// gr_glide_clip_cursor(1);
				// grSstControl(GR_CONTROL_ACTIVATE);				
			}
		} else {
			Glide_deactivate++;
	
			if ( hwnd )	{
				// grSstControl(GR_CONTROL_DEACTIVATE);
				ClipCursor(NULL);
				ShowWindow(hwnd,SW_MINIMIZE);
			}
		}
	}	
}


// copy from one pixel buffer to another
//
//    from			pointer to source buffer
//    to				pointer to dest. buffet
//    pixels		number of pixels to copy
//    fromsize		source pixel size
//    tosize		dest. pixel size

static int tga_copy_data(char *to, char *from, int pixels, int fromsize, int tosize)
{
	int rmask = 0xf800;
	int rshift = 11;
	int rscale = 8;;
	int gmask = 0x7e0;
	int gshift = 5;
	int gscale = 4;
	int bmask = 0x1f;
	int bshift = 0;
	int bscale = 8;;

	if ( (fromsize == 2) && (tosize == 3) )	{
		ushort *src = (ushort *)from;
		ubyte *dst  = (ubyte *)to;

		int i;
		for (i=0; i<pixels; i++ )	{
			ushort pixel = *src++;

			int r,g,b;		
			b = ((pixel & bmask)>>bshift)*bscale;
			g = ((pixel & gmask)>>gshift)*gscale;
			r = ((pixel & rmask)>>rshift)*rscale;

			// Adjust for gamma and output it
			*dst++ = ubyte(b);
			*dst++ = ubyte(g);
			*dst++ = ubyte(r);
		}
		return tosize*pixels;
	} else {
		Int3();
		return tosize*pixels;
	}
}



//
//	tga_pixels_equal -- Test if two pixels are identical
//
//		Returns:
//			0 if No Match
//			1 if Match

static int tga_pixels_equal(char *pix1, char *pix2, int pixbytes)
{
	do	{
		if ( *pix1++ != *pix2++ ) {
			return 0;
		}
	} while ( --pixbytes > 0 );

	return 1;
}


//	tga_compress - Do the Run Length Compression
//
//	Usage:
//				out			Buffer to write it out to
//				in				Buffer to compress
//				bytecount	Number of bytes input
//				pixsize		Number of bytes in input pixel
//				outsize		Number of bytes in output buffer

int tga_compress(char *out, char *in, int bytecount )
{
#define pixsize 2
#define outsize 3
	int pixcount;		// number of pixels in the current packet
	char *inputpixel=NULL;	// current input pixel position
	char *matchpixel=NULL;	// pixel value to match for a run
	char *flagbyte=NULL;		// location of last flag byte to set
	int rlcount;		// current count in r.l. string 
	int rlthresh;		// minimum valid run length
	char *copyloc;		// location to begin copying at

	// set the threshold -- the minimum valid run length

	#if outsize == 1
		rlthresh = 2;					// for 8bpp, require a 2 pixel span before rle'ing
	#else
		rlthresh = 1;			
	#endif
	
	// set the first pixel up

	flagbyte = out;	// place to put next flag if run
	inputpixel = in;
	pixcount = 1;
	rlcount = 0;
	copyloc = (char *)0;

	// loop till data processing complete
	do	{

		// if we have accumulated a 128-byte packet, process it
		if ( pixcount == 129 )	{
			*flagbyte = 127;

			// set the run flag if this is a run

			if ( rlcount >= rlthresh )	{
					*flagbyte |= 0x80;
					pixcount = 2;
			}

			// copy the data into place
			++flagbyte;
			flagbyte += tga_copy_data(flagbyte, copyloc, pixcount-1, pixsize, outsize);
			pixcount = 1;

			// set up for next packet
			continue;
		}

		// if zeroth byte, handle as special case
		if ( pixcount == 1 )	{
			rlcount = 0;
			copyloc = inputpixel;		/* point to 1st guy in packet */
			matchpixel = inputpixel;	/* set pointer to pix to match */
			pixcount = 2;
			inputpixel += pixsize;
			continue;
		}

		// assembling a packet -- look at next pixel

		// current pixel == match pixel?
		if ( tga_pixels_equal(inputpixel, matchpixel, outsize) )	{

			//	establishing a run of enough length to
			//	save space by doing it
			//		-- write the non-run length packet
			//		-- start run-length packet

			if ( ++rlcount == rlthresh )	{
				
				//	close a non-run packet
				
				if ( pixcount > (rlcount+1) )	{
					// write out length and do not set run flag

					*flagbyte++ = (char)(pixcount - 2 - rlthresh);

					flagbyte += tga_copy_data(flagbyte, copyloc, (pixcount-1-rlcount), pixsize, outsize);

					copyloc = inputpixel;
					pixcount = rlcount + 1;
				}
			}
		} else {

			// no match -- either break a run or continue without one
			//	if a run exists break it:
			//		write the bytes in the string (outsize+1)
			//		start the next string

			if ( rlcount >= rlthresh )	{

				*flagbyte++ = (char)(0x80 | rlcount);
				flagbyte += tga_copy_data(flagbyte, copyloc, 1, pixsize, outsize);
				pixcount = 1;
				continue;
			} else {

				//	not a match and currently not a run
				//		- save the current pixel
				//		- reset the run-length flag
				rlcount = 0;
				matchpixel = inputpixel;
			}
		}
		pixcount++;
		inputpixel += pixsize;
	} while ( inputpixel < (in + bytecount));

	// quit this buffer without loosing any data

	if ( --pixcount >= 1 )	{
		*flagbyte = (char)(pixcount - 1);
		if ( rlcount >= rlthresh )	{
			*flagbyte |= 0x80;
			pixcount = 1;
		}

		// copy the data into place
		++flagbyte;
		flagbyte += tga_copy_data(flagbyte, copyloc, pixcount, pixsize, outsize);
	}
	return(flagbyte-out);
}



void gr_glide_print_screen(char *filename)
{
	GrLfbInfo_t info;
	int i;
	ubyte outrow[1024*3*4];

	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( gr_screen.max_w > 1024 )	{
		mprintf(( "Screen too wide for print_screen\n" ));
		return;
	}

	info.size=sizeof(GrLfbInfo_t);

	// get a read pointer 
	if ( grLfbLock(	GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565,
							GR_ORIGIN_UPPER_LEFT, FXFALSE, &info))		{
		int w=gr_screen.max_w,h=gr_screen.max_h;
		ushort *rptr;
		int short_per_row=info.strideInBytes/2;

		rptr = (ushort *)info.lfbPtr;


		char tmp[1024];

		strcpy( tmp, NOX(".\\"));	// specify a path mean files goes in root
		strcat( tmp, filename );
		strcat( tmp, NOX(".tga"));

		CFILE *f = cfopen(tmp, "wb");

		// Write the TGA header
		cfwrite_ubyte( 0, f );	//	IDLength;
		cfwrite_ubyte( 0, f );	//	ColorMapType;
		cfwrite_ubyte( 10, f );	//	ImageType;		// 2 = 24bpp, uncompressed, 10=24bpp rle compressed
		cfwrite_ushort( 0, f );	// CMapStart;
		cfwrite_ushort( 0, f );	//	CMapLength;
		cfwrite_ubyte( 0, f );	// CMapDepth;
		cfwrite_ushort( 0, f );	//	XOffset;
		cfwrite_ushort( 0, f );	//	YOffset;
		cfwrite_ushort( (ushort)w, f );	//	Width;
		cfwrite_ushort( (ushort)h, f );	//	Height;
		cfwrite_ubyte( 24, f );	//PixelDepth;
		cfwrite_ubyte( 0, f );	//ImageDesc;

		// Go through and read our pixels
		for (i=0;i<h;i++)	{
			int len = tga_compress( (char *)outrow, (char *)&rptr[(h-i-1)*short_per_row], w*sizeof(short) );

			cfwrite(outrow,len,1,f);
		}

		cfclose(f);

		// Release the lock
		grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER );
	} else {
		mprintf(( "Couldn't get a lock to glide's back buffer!\n" ));
		Int3();
	}

}

int gr_glide_save_screen_internal(ushort *src_data)
{
	GrLfbInfo_t info;
	int i;

	if(VOODOO3_INACTIVE()){
		return 0;
	}

	info.size=sizeof(GrLfbInfo_t);

	// get a read pointer 
	if ( grLfbLock(	GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER, GR_LFBWRITEMODE_565,
							GR_ORIGIN_UPPER_LEFT, FXFALSE, &info))		{
		int w=gr_screen.max_w,h=gr_screen.max_h;
		ushort *rptr;
		int short_per_row=info.strideInBytes/2;

		rptr = (ushort *)info.lfbPtr;

		// Go through and read our pixels
		for (i=0;i<h;i++)	{
			memcpy( &src_data[gr_screen.max_w*i], &rptr[i*short_per_row], w*sizeof(short) );
		}

		// Release the lock
		grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER );
	} else {
		mprintf(( "Couldn't get a read lock to glide's back buffer!\n" ));
		return 0;
	}
	return 1;
}


static ushort *Gr_saved_screen = NULL;

int gr_glide_save_screen()
{
	if(VOODOO3_INACTIVE()){
		return -1;
	}	

	if ( Gr_saved_screen )	{
		mprintf(( "Screen alread saved!\n" ));
		return -1;
	}

	gr_reset_clip();

	Gr_saved_screen = (ushort *)malloc( gr_screen.max_w*gr_screen.max_h*sizeof(ushort) );
	if (!Gr_saved_screen) {
		mprintf(( "Couldn't get memory for saved screen!\n" ));
		return -1;
	}

	if ( !gr_glide_save_screen_internal(Gr_saved_screen) )	{
		free(Gr_saved_screen);
		Gr_saved_screen = NULL;
		return -1;
	}

	if ( Gr_glide_mouse_saved )	{
		ushort *sptr, *dptr;

		sptr = Gr_glide_mouse_saved_data;

		for (int i=0; i<Gr_glide_mouse_saved_h; i++ )	{
			dptr = &Gr_saved_screen[(Gr_glide_mouse_saved_y1+i)*gr_screen.max_w+Gr_glide_mouse_saved_x1];

			for(int j=0; j<Gr_glide_mouse_saved_w; j++ )	{
				*dptr++ = *sptr++;
			}
		}
	}

	return 0;
}

void gr_glide_restore_screen_internal(ushort *src_data)
{
	GrLfbInfo_t info;
	int i;

	if(VOODOO3_INACTIVE()){
		return;
	}

	info.size=sizeof(GrLfbInfo_t);

	// get a read pointer 
	if ( grLfbLock(	GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565,
							GR_ORIGIN_UPPER_LEFT, FXFALSE, &info))		{
		int w=gr_screen.max_w,h=gr_screen.max_h;
		ushort *rptr;
		int short_per_row=info.strideInBytes/2;

		rptr = (ushort *)info.lfbPtr;

		// Go through and read our pixels
		for (i=0;i<h;i++)	{
			memcpy( &rptr[i*short_per_row], &src_data[gr_screen.max_w*i], w*sizeof(short) );
		}

		// Release the lock
		grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER );
	} else {
		mprintf(( "Couldn't get a write lock to glide's back buffer!\n" ));
	}
}


void gr_glide_restore_screen(int id)
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	gr_reset_clip();

	if ( !Gr_saved_screen )	{
		gr_clear();
		return;
	}

	gr_glide_restore_screen_internal(Gr_saved_screen);
}

void gr_glide_free_screen(int id)
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( Gr_saved_screen )	{
		free( Gr_saved_screen );
		Gr_saved_screen = NULL;
	}
}


void gr_glide_force_windowed()
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	gr_glide_clip_cursor(0);
	grSstControl(GR_CONTROL_DEACTIVATE);
}



static int Glide_dump_frames = 0;
static ubyte *Glide_dump_buffer = NULL;
static int Glide_dump_frame_number = 0;
static int Glide_dump_frame_count = 0;
static int Glide_dump_frame_count_max = 0;
static int Glide_dump_frame_size = 0;

void gr_glide_dump_frame_start(int first_frame, int frames_between_dumps)
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( Glide_dump_frames )	{
		Int3();		//  We're already dumping frames.  See John.
		return;
	}	
	Glide_dump_frames = 1;
	Glide_dump_frame_number = first_frame;
	Glide_dump_frame_count = 0;
	Glide_dump_frame_count_max = frames_between_dumps;
	Glide_dump_frame_size = gr_screen.max_w * gr_screen.max_h * 2;
	
	if ( !Glide_dump_buffer ) {
		int size = Glide_dump_frame_count_max * Glide_dump_frame_size;
		Glide_dump_buffer = (ubyte *)malloc(size);
		if ( !Glide_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}

}

// A hacked function to dump the frame buffer contents
void gr_glide_dump_screen_hack( ushort * dst )
{
	GrLfbInfo_t info;
	int i;

	if(VOODOO3_INACTIVE()){
		return;
	}

	info.size=sizeof(GrLfbInfo_t);	

	// get a read pointer 
	if ( grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_565, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info))		{
		int w=gr_screen.max_w,h=gr_screen.max_h;
		ushort *rptr;
		int short_per_row=info.strideInBytes/2;

		rptr = (ushort *)info.lfbPtr;

		// Go through and read our pixels
		for (i=0;i<h;i++)	{
			memcpy( &dst[gr_screen.max_w*i], &rptr[(h-i-1)*short_per_row], w*sizeof(short) );
		}

		// Release the lock
		grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_BACKBUFFER );
	} else {
		mprintf(( "Couldn't get a read lock to glide's back buffer for frame dump!\n" ));
	}
}

void gr_glide_get_region(int front, int w, int h, ubyte *data)
{
	GrLfbInfo_t info;
	ushort bit_16;
	ubyte r, g, b, a;
	a = 1;

	if(VOODOO3_INACTIVE()){
		return;
	}

	info.size=sizeof(GrLfbInfo_t);
	// get a read pointer 
	if ( grLfbLock(GR_LFB_READ_ONLY, front ? GR_BUFFER_FRONTBUFFER : GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_1555, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)){
		ushort *rptr;
		int short_per_row=info.strideInBytes/2;

		rptr = (ushort *)info.lfbPtr;

		ushort *sptr, *dptr;

		dptr = (ushort*)data;

		for (int i=0; i<h; i++ )	{
			sptr = &rptr[i*short_per_row];

			// 565 data is what we always get, so we need to swizzle
			for(int j=0; j<w; j++ )	{
				bit_16 = *sptr++;			
				
				r = (ubyte)((bit_16 & 0xf800) >> 8);
				g = (ubyte)((bit_16 & 0x07e0) >> 3);
				b = (ubyte)((bit_16 & 0x001f) << 3);
				
				// swizzle the data to 1555 (BM_PIXEL_FORMAT_ARGB)
				*dptr = 0;
				bm_set_components((ubyte*)dptr++, &r, &g, &b, &a);				
			}
		}

		// Release the lock
		grLfbUnlock(GR_LFB_READ_ONLY, front ? GR_BUFFER_FRONTBUFFER : GR_BUFFER_BACKBUFFER);
	} 
}

void gr_glide_flush_frame_dump()
{
	int i,j;
	char filename[MAX_PATH_LEN], *movie_path = ".\\";
	ubyte outrow[1024*3*4];

	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( gr_screen.max_w > 1024)	{
		mprintf(( "Screen too wide for frame_dump\n" ));
		return;
	}

	for (i = 0; i < Glide_dump_frame_count; i++) {

		int w = gr_screen.max_w;
		int h = gr_screen.max_h;

		sprintf(filename, NOX("%sfrm%04d.tga"), movie_path, Glide_dump_frame_number );
		Glide_dump_frame_number++;

		CFILE *f = cfopen(filename, "wb");

		// Write the TGA header
		cfwrite_ubyte( 0, f );	//	IDLength;
		cfwrite_ubyte( 0, f );	//	ColorMapType;
		cfwrite_ubyte( 10, f );	//	ImageType;		// 2 = 24bpp, uncompressed, 10=24bpp rle compressed
		cfwrite_ushort( 0, f );	// CMapStart;
		cfwrite_ushort( 0, f );	//	CMapLength;
		cfwrite_ubyte( 0, f );	// CMapDepth;
		cfwrite_ushort( 0, f );	//	XOffset;
		cfwrite_ushort( 0, f );	//	YOffset;
		cfwrite_ushort( (ushort)w, f );	//	Width;
		cfwrite_ushort( (ushort)h, f );	//	Height;
		cfwrite_ubyte( 24, f );	//PixelDepth;
		cfwrite_ubyte( 0, f );	//ImageDesc;

		// Go through and write our pixels
		for (j=0;j<h;j++)	{
			ubyte *src_ptr = Glide_dump_buffer+(i*Glide_dump_frame_size)+(j*w*2);

			int len = tga_compress( (char *)outrow, (char *)src_ptr, w*sizeof(short) );

			cfwrite(outrow,len,1,f);
		}

		cfclose(f);

	}

	Glide_dump_frame_count = 0;
}

void gr_glide_dump_frame()
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	// A hacked function to dump the frame buffer contents
	gr_glide_dump_screen_hack( (ushort *)(Glide_dump_buffer+(Glide_dump_frame_count*Glide_dump_frame_size)) );

	Glide_dump_frame_count++;

	if ( Glide_dump_frame_count == Glide_dump_frame_count_max ) {
		gr_glide_flush_frame_dump();
	}
}

void gr_glide_dump_frame_stop()
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	if ( !Glide_dump_frames )	{
		Int3();		//  We're not dumping frames.  See John.
		return;
	}	

	// dump any remaining frames
	gr_glide_flush_frame_dump();
	
	Glide_dump_frames = 0;
	if ( Glide_dump_buffer )	{
		free(Glide_dump_buffer);
		Glide_dump_buffer = NULL;
	}
}

#define FADE_TIME (F1_0/4)		// How long to fade out

void gr_glide_fade_out(int instantaneous)	
{
	ushort *tmp_data;

	if(VOODOO3_INACTIVE()){
		return;
	}

	gr_reset_clip();
	Mouse_hidden++;

	tmp_data = (ushort *)malloc( gr_screen.max_w*gr_screen.max_h*sizeof(ushort) );

	if ( tmp_data )	{
		gr_glide_save_screen_internal(tmp_data);
	}

	if (!gr_palette_faded_out) {

		if ( !instantaneous )	{

			if ( tmp_data )	{
				int count = 0;
				fix start_time, stop_time, t1;


				start_time = timer_get_fixed_seconds();
				t1 = 0;

				do {
					//int c = (255*(FADE_TIME-t1))/FADE_TIME;
					int c = (255*t1)/FADE_TIME;
					if (c < 0 )	{
						c = 0;
					} else if ( c > 255 )	{
						c = 255;
					}

					gr_glide_restore_screen_internal(tmp_data);
				
					for (int i=0; i<gr_screen.max_h; i++ )	{
						glide_shade_scanline( 0, gr_screen.max_w-1, i, 0, 0, 0, c );
					}

					gr_flip();
					count++;

					t1 = timer_get_fixed_seconds() - start_time;

				} while ( (t1 < FADE_TIME) && (t1>=0) );		// Loop as long as time not up and timer hasn't rolled

				stop_time = timer_get_fixed_seconds();

				mprintf(( "Took %d frames (and %.1f secs) to fade out\n", count, f2fl(stop_time-start_time) ));
		
			}
		}
	}

	gr_clear();
	gr_flip();
	gr_palette_faded_out = 1;
	Mouse_hidden--;

	if ( tmp_data )	{
		free(tmp_data);
	}
}

void gr_glide_fade_in(int instantaneous)	
{
	ushort *tmp_data;

	if(VOODOO3_INACTIVE()){
		return;
	}

	Mouse_hidden++;
	gr_reset_clip();

	tmp_data = (ushort *)malloc( gr_screen.max_w*gr_screen.max_h*sizeof(ushort) );

	if ( tmp_data )	{
		gr_glide_save_screen_internal(tmp_data);
	}

	if (gr_palette_faded_out) {

		gr_palette_faded_out = 0;

		if ( !instantaneous )	{

			if ( tmp_data )	{
				int count = 0;
				fix start_time, stop_time, t1;


				start_time = timer_get_fixed_seconds();
				t1 = 0;

				do {
					int c = (255*(FADE_TIME-t1))/FADE_TIME;
					if (c < 0 )	{
						c = 0;
					} else if ( c > 255 )	{
						c = 255;
					}

					gr_glide_restore_screen_internal(tmp_data);
					
					for (int i=0; i<gr_screen.max_h; i++ )	{
						glide_shade_scanline( 0, gr_screen.max_w-1, i, 0, 0, 0, c );
					}

					gr_flip();
					count++;

					t1 = timer_get_fixed_seconds() - start_time;

				} while ( (t1 < FADE_TIME) && (t1>=0) );		// Loop as long as time not up and timer hasn't rolled

				stop_time = timer_get_fixed_seconds();

				mprintf(( "Took %d frames (and %.1f secs) to fade out\n", count, f2fl(stop_time-start_time) ));
		
			}
		}
	}


	if ( tmp_data )	{
		gr_glide_restore_screen_internal(tmp_data);
	}
	gr_flip();
	Mouse_hidden--;

	if ( tmp_data )	{
		free(tmp_data);
	}
}

void gr_glide_cleanup()
{
	if ( !Inited )	return;


	grGlideShutdown();

	vglide_close();

	gr_glide_clip_cursor(0);

	glide_tcache_cleanup();

	Inited = 0;
}

void gr_glide_set_gamma(float gamma)
{
	Gr_gamma = gamma;
	Gr_gamma_int = int(Gr_gamma*100);

	// Create the Gamma lookup table
	int i;
	for (i=0; i<256; i++ )	{
		int v = fl2i(pow(i2fl(i)/255.0f, 1.0f/Gr_gamma)*255.0f);
		if ( v > 255 ) {
			v = 255;
		} else if ( v < 0 )	{
			v = 0;
		}
		Gr_gamma_lookup[i] = v;
	}

	for (i=0; i<256; i++ )	{
		float v = (float)pow(i2fl(i)/255.0f, 1.0f/Gr_gamma)*255.0f;
		if ( v > 255.0f ) {
			v = 255.0f;
		} else if ( v < 0.0f )	{
			v = 0.0f;
		}
		Gr_gamma_lookup_float[i] = v;
	}

	// Flush any existing textures
	glide_tcache_flush();

}

void gr_glide_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{		
	GrColor_t color = 0;

	if(VOODOO3_INACTIVE()){
		return;
	}
	
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	// store the values
	gr_glide_init_color( &gr_screen.current_fog_color, r, g, b );
	if(fog_near >= 0.0f){
		gr_screen.fog_near = fog_near;
	}
	if(fog_far >= 0.0f){
		gr_screen.fog_far = fog_far;
	}	
	gr_screen.current_fog_mode = fog_mode;

	// enable/disable fog
	if(fog_mode == GR_FOGMODE_NONE){
		grFogMode(GR_FOG_DISABLE);

		// always unset the global for value if we're disabling fog
		gr_screen.fog_near = -1.0f;
		gr_screen.fog_far = -1.0f;

		return;
	} 
	grFogMode(GR_FOG_WITH_TABLE);
	
	// set the fog color
	color |= ((ubyte)r);
	color |= ((ubyte)g << 8);
	color |= ((ubyte)b << 16);
	grFogColorValue(color);

	// only generate a new fog table if we have to
	if((fog_near >= 0.0f) && (fog_far > fog_near)){
		guFogGenerateLinear(Glide_linear_fogtable, fog_near, fog_far);
	}

	// set the fog table		
	grFogTable(Glide_linear_fogtable);		
}

void gr_glide_get_pixel(int x, int y, int *r, int *g, int *b)
{
	ushort pixel;
	*r = 0;
	*g = 0;
	*b = 0;		

	if(VOODOO3_INACTIVE()){
		return;
	}

	// returns data in 565 format
	grLfbReadRegion(GR_BUFFER_BACKBUFFER, (FxU32)x, (FxU32)y, 1, 1, 2, &pixel);

	// unpack pixel color
	*r = (0xf800 & pixel) >> 8;
	*g = (0x07e0 & pixel) >> 3;
	*b = (0x001f & pixel) << 3;		
}

// resolution checking
int gr_glide_supports_res_ingame(int res)
{
	return 1;
}

int gr_glide_supports_res_interface(int res)
{
	return 1;
}

void gr_glide_set_cull(int cull)
{
}

void gr_glide_filter_set(int filter)
{
	if(VOODOO3_INACTIVE()){
		return;
	}

	if(filter){
		grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	} else {
		grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_POINT_SAMPLED, GR_TEXTUREFILTER_POINT_SAMPLED);
	}
}

// set clear color
void gr_glide_set_clear_color(int r, int g, int b)
{
	gr_init_color(&gr_screen.current_clear_color, r, g, b);
}

extern int movie_rendered_flag;
extern int movie_frame_count;

void __cdecl grglide_MovieShowFrame(void *buf,uint bufw,uint bufh, uint sx,uint sy,uint w,uint h,uint dstx,uint dsty, uint hi_color)
{
	// no soup for you!

	/*
	RECT srect, drect;

	if(VOODOO3_INACTIVE()){
		return;
	}	

	gr_reset_clip();
	gr_clear();

	ushort *src_data = (ushort *)buf;

	movie_rendered_flag = 1;
	
	if ( hi_color ) {
		bufw >>= 1;
		sx >>= 1;
		w >>= 1;
		dstx >>= 1;
	}	

	SetRect(&srect, sx, sy, sx+w-1, sy+h-1);
	//SetRect(&drect, dstx, dsty, dstx+w-1, dsty+h-1);
	dstx = (gr_screen.max_w - w)/2;
	dsty = (gr_screen.max_h - h)/2;
	SetRect(&drect, dstx, dsty, dstx+w-1, dsty+h-1);

	GrLfbInfo_t info;
	int i;

	info.size=sizeof(GrLfbInfo_t);

	// get a read pointer 
	if ( grLfbLock(	GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_1555,
							GR_ORIGIN_UPPER_LEFT, FXFALSE, &info))		{
		ushort *rptr;
		int short_per_row=info.strideInBytes/2;

		rptr = (ushort *)info.lfbPtr;

		// if doing interlaced mode, then go through every other scanline
		if ( Interlace_movies ) {
			static int start_at = 0;

			for ( i = start_at; i < (int)h; i += 2 ) {
				memcpy( &rptr[(dsty+i)*short_per_row+dstx], &src_data[bufw*i], w*sizeof(short) );
			}
			//start_at = (start_at + 1) % 2;
		} else { 
			// Go through and read our pixels
			for (i=0;i<(int)h;i++)	{
				memcpy( &rptr[(dsty+i)*short_per_row+dstx], &src_data[bufw*i], w*sizeof(short) );
			}
		}

		// Release the lock
		grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER );
	} else {
		mprintf(( "Couldn't get a write lock to glide's back buffer!\n" ));
	}

	Mouse_hidden++;
	gr_flip();
	Mouse_hidden--;
	*/
}

// cross fade
#define FETCH_A(i, j)	{								\
	ubyte code = 0;										\
	code |= ((i+min_x<x1) << 0);						\
	code |= ((j+min_y<y1) << 1);						\
	code |= ((i+min_x<x1+bmp1->w) << 2);			\
	code |= ((j+min_y<y1+bmp1->h) << 3);			\
	if(code && (code < 4)){								\
		pixel_a = sptr1[i - (x1 - min_x)];			\
	} else {													\
		pixel_a = pixel_black;							\
	}															\
}
#define FETCH_B(i, j)	{								\
	ubyte code = 0;										\
	code |= ((i+min_x<x2) << 0);						\
	code |= ((j+min_y<y2) << 1);						\
	code |= ((i+min_x<x2+bmp2->w) << 2);			\
	code |= ((j+min_y<y2+bmp2->h) << 3);			\
	if(code && (code < 4)){								\
		pixel_b = sptr2[i - (x2 - min_x)];			\
	} else {													\
		pixel_b = pixel_black;							\
	}															\
}
#define MIX(pout, p1, p2)				{ pout = p1; }
void gr_glide_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	if ( pct <= 50 )	{
		gr_set_bitmap(bmap1);
		gr_bitmap(x1, y1);
	} else {
		gr_set_bitmap(bmap2);
		gr_bitmap(x2, y2);
	}	
	/*
	int min_x = x1 < x2 ? x1 : x2;
	int min_y = y1 < y2 ? y1 : y2;
	int max_x = x2 > x1 ? x2 : x1;
	int max_y = y2 > y1 ? y2 : y1;
	int max_w, max_h;
	int i, j;
	ushort *sptr1;
	ushort *sptr2;
	bitmap *bmp1, *bmp2;			
	ushort pixel_a, pixel_b;
	ushort pixel_black;
	ushort pixel_out;
	ubyte r, g, b, a;

	// stuff the black pixel
	r = 0; g = 0; b = 0; a = 255;
	pixel_black = 0;
	bm_set_components(&pixel_black, &r, &g, &b, &a);
	
	// lock the first bitmap
	bmp1 = bm_lock( bmap1, 16, 0 );
	sptr1 = (ushort *)( bmp1->data );

	// lock the second bitmap
	bmp2 = bm_lock( bmap2, 16, 0 );
	sptr2 = (ushort *)( bmp2->data );	

	if(x1 > x2){
		max_x = x1;
		min_x = x2;
	} else {
		min_x = x1;
		max_x = x2;
	}
	if(y1 > y2){
		max_y = y1;
		min_y = y2;
	} else {
		min_y = y1;
		max_y = y2;
	}
	if(bmp1->w > bmp2->w){
		max_w = bmp1->w;
	} else {
		max_w = bmp2->w;
	}
	if(bmp1->h > bmp2->h){
		max_h = bmp1->h;
	} else {
		max_h = bmp2->h;
	}
	
	GrLfbInfo_t info;

	// lock the framebuffer
	if ( grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER, GR_LFBWRITEMODE_1555, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info) ) {

		// pointer into vram
		ushort *vram = (ushort *)info.lfbPtr;
		int stride = info.strideInBytes / sizeof(ushort);
		
		// for all scanlines
		for (i=0; i<max_h; i++ ){
			// this scanline in vram
			ushort *dptr = &vram[stride*(gr_screen.offset_y+i+min_y)+(gr_screen.offset_x+min_x)];
			
			// for this scanline
			for ( j=0; j<max_w; j++ ){				
				// fetch the A and B pixels
				// FETCH_A(i, j);
				// FETCH_B(i, j);
				{								
					ubyte code = 0;										
					code |= ((i+min_x>=x1) << 0);						
					code |= ((j+min_y>=y1) << 1);						
					code |= ((i+min_x>x1+bmp1->w) << 2);			
					code |= ((j+min_y>y1+bmp1->h) << 3);			
					if(code && (code < 4)){								
						pixel_a = sptr1[i - (x1 - min_x)];			
					} else {													
						pixel_a = pixel_black;							
					}															
				}
				{								
					ubyte code = 0;										
					code |= ((i+min_x>=x2) << 0);						
					code |= ((j+min_y>=y2) << 1);						
					code |= ((i+min_x>x2+bmp2->w) << 2);			
					code |= ((j+min_y>y2+bmp2->h) << 3);			
					if(code && (code < 4)){								
						pixel_b = sptr2[i - (x2 - min_x)];			
					} else {													
						pixel_b = pixel_black;							
					}															
				}


				// mix them - for now just always pick pixel A
				MIX(pixel_out, pixel_a, pixel_b);

				// write to vram
				*dptr = pixel_out;

				// next pixel in vram
				dptr++;
			}			
			
			// increment if we need to
			if((j+min_y >= y1) && (j+min_y < y1+bmp1->h)){
				sptr1 += bmp1->w;
			}
			// increment if we need to
			if((j+min_y >= y2) && (j+min_y < y2+bmp2->h)){
				sptr2 += bmp2->w;			
			}
		}

		grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_BACKBUFFER);
	}	

	bm_unlock(bmap1);
	bm_unlock(bmap2);
	*/
}

GrHwConfiguration hwconfig;

void gr_glide_init()
{
	D3D_enabled = 1;
	Glide_running = 0;	
	int res_mode;

	if ( Inited )	{
		gr_glide_cleanup();
		Inited = 0;
	}

	// Turn off the 3Dfx splash screen
	SetEnvironmentVariable("FX_GLIDE_NO_SPLASH","1");

	// Turn off the 3Dfx gamma correction
	SetEnvironmentVariable("SST_RGAMMA","1.0");
	SetEnvironmentVariable("SST_GGAMMA","1.0");
	SetEnvironmentVariable("SST_BGAMMA","1.0");

	mprintf(( "Initializing glide graphics device...\n" ));
	Inited = 1;

	if ( !vglide_init() )	{
		mprintf(( "Glide DLL not found!\n" ));
		exit(1);
	}

//	os_suspend();

	// Find the extents of the window
	HWND hwnd = (HWND)os_get_window();

	// Prepare the window to go full screen
#ifndef NDEBUG
	mprintf(( "Window in debugging mode... mouse clicking may cause problems!\n" ));
	SetWindowLong( hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT );
	SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
	ShowWindow(hwnd, SW_SHOWNORMAL );
	RECT work_rect;
	SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
	SetWindowPos( hwnd, HWND_NOTOPMOST, work_rect.left, work_rect.top, gr_screen.max_w, gr_screen.max_h, 0 );	
	SetActiveWindow(hwnd);
	SetForegroundWindow(hwnd);
	Glide_cursor_clip_rect.left = work_rect.left;
	Glide_cursor_clip_rect.top = work_rect.top;
	Glide_cursor_clip_rect.right = work_rect.left + gr_screen.max_w - 1;
	Glide_cursor_clip_rect.bottom = work_rect.top + gr_screen.max_h - 1;
#else
	SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
	SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
	ShowWindow(hwnd, SW_SHOWNORMAL );
	SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ), 0 );	
	SetActiveWindow(hwnd);
	SetForegroundWindow(hwnd);
	Glide_cursor_clip_rect.left = 0;
	Glide_cursor_clip_rect.top = 0;
	Glide_cursor_clip_rect.right = gr_screen.max_w;
	Glide_cursor_clip_rect.bottom = gr_screen.max_h;
#endif

	// Let things catch up....
	Sleep(2000);
	
	grGlideInit();

	gr_screen.bytes_per_pixel = 2;
	gr_screen.bits_per_pixel = 16;

	char version[80];
	grGlideGetVersion(version);
	mprintf(( "Glide version: %s\n", version ));

	if ( !grSstQueryHardware( &hwconfig ))	{
		mprintf(( "Glide: Query hardaware failed!\n" ));
		os_resume();
		exit(1);
	}	

	grSstSelect(0);

	// voodoo 3
	Glide_voodoo3 = 0;
	if(hwconfig.SSTs[0].sstBoard.Voodoo2Config.fbRam >= 12){
		Glide_voodoo3 = 1;
	}

	// choose resolution
	if((gr_screen.max_w == 1024) && (gr_screen.max_h == 768)){
		res_mode = GR_RESOLUTION_1024x768;
	} else {
		res_mode = GR_RESOLUTION_640x480;
	}

	int retval = grSstWinOpen( (DWORD)hwnd, res_mode, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR, GR_ORIGIN_UPPER_LEFT, 2, 1 );
	if ( !retval )	{
		mprintf(( "Glide: grSstOpen failed!\n" ));
		os_resume();
		exit(1);
	}

	// pixel format
	Bm_pixel_format = BM_PIXEL_FORMAT_ARGB;

//	grChromakeyMode( GR_CHROMAKEY_ENABLE );
//	grChromakeyValue( 0x00FF00 );

//	os_resume();

	// Setup the surface format
	Gr_red.bits = 5;
	Gr_red.shift = 10;
	Gr_red.scale = 256/32;
	Gr_red.mask = 0x7C00;
	Gr_t_red = Gr_red;
	Gr_current_red = &Gr_red;	

	Gr_green.bits = 5;
	Gr_green.shift = 5;
	Gr_green.scale = 256/32;
	Gr_green.mask = 0x03e0;
	Gr_t_green = Gr_green;
	Gr_current_green = &Gr_green;

	Gr_blue.bits = 5;
	Gr_blue.shift = 0;
	Gr_blue.scale = 256/32;
	Gr_blue.mask = 0x1F;
	Gr_t_blue = Gr_blue;
	Gr_current_blue = &Gr_blue;

	Gr_current_alpha = &Gr_alpha;

	glide_tcache_init();

	gr_glide_clip_cursor(1);

	grGammaCorrectionValue(1.0f);

	gr_screen.gf_flip = gr_glide_flip;
	gr_screen.gf_flip_window = gr_glide_flip_window;
	gr_screen.gf_set_clip = gr_glide_set_clip;
	gr_screen.gf_reset_clip = gr_glide_reset_clip;
	gr_screen.gf_set_font = grx_set_font;
	gr_screen.gf_set_color = gr_glide_set_color;
	gr_screen.gf_set_bitmap = gr_glide_set_bitmap;
	gr_screen.gf_create_shader = gr_glide_create_shader;
	gr_screen.gf_set_shader = gr_glide_set_shader;
	gr_screen.gf_clear = gr_glide_clear;

	// gr_screen.gf_bitmap = gr_glide_bitmap;	
	// gr_screen.gf_bitmap_ex = gr_glide_bitmap_ex;

	gr_screen.gf_rect = gr_glide_rect;
	gr_screen.gf_shade = gr_glide_shade;
	gr_screen.gf_string = gr_glide_string;
	gr_screen.gf_circle = gr_glide_circle;

	gr_screen.gf_line = gr_glide_line;
	gr_screen.gf_aaline = gr_glide_aaline;
	gr_screen.gf_pixel = gr_glide_pixel;
	gr_screen.gf_scaler = gr_glide_scaler;
	gr_screen.gf_aascaler = gr_glide_aascaler;
	gr_screen.gf_tmapper = gr_glide_tmapper;

	gr_screen.gf_gradient = gr_glide_gradient;

	gr_screen.gf_set_palette = gr_glide_set_palette;
	gr_screen.gf_get_color = gr_glide_get_color;
	gr_screen.gf_init_color = gr_glide_init_color;
	gr_screen.gf_init_alphacolor = gr_glide_init_alphacolor;
	gr_screen.gf_set_color_fast = gr_glide_set_color_fast;
	gr_screen.gf_print_screen = gr_glide_print_screen;

	gr_screen.gf_aabitmap = gr_glide_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_glide_aabitmap_ex;

	gr_screen.gf_fade_in = gr_glide_fade_in;
	gr_screen.gf_fade_out = gr_glide_fade_out;
	gr_screen.gf_flash = gr_glide_flash;

	gr_screen.gf_zbuffer_get = gr_glide_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_glide_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_glide_zbuffer_clear;

	gr_screen.gf_save_screen = gr_glide_save_screen;
	gr_screen.gf_restore_screen = gr_glide_restore_screen;
	gr_screen.gf_free_screen = gr_glide_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr_glide_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_glide_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_glide_dump_frame;

	gr_screen.gf_set_gamma = gr_glide_set_gamma;

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_glide_lock;
	gr_screen.gf_unlock = gr_glide_unlock;

	// region
	gr_screen.gf_get_region = gr_glide_get_region;

	// fog stuff
	gr_screen.gf_fog_set = gr_glide_fog_set;	

	// pixel get
	gr_screen.gf_get_pixel = gr_glide_get_pixel;

	// poly culling
	gr_screen.gf_set_cull = gr_glide_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_glide_cross_fade;

	// filter
	gr_screen.gf_filter_set = gr_glide_filter_set;

	// texture cache set
	gr_screen.gf_tcache_set = glide_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_glide_set_clear_color;

	Glide_running=1;

	// default linear fog table
	guFogGenerateLinear(Glide_linear_fogtable, 1.0f, 1000.0f);
	
	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;
}


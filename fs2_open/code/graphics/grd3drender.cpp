/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3DRender.cpp $
 * $Revision: 2.5 $
 * $Date: 2003-01-05 23:41:50 $
 * $Author: bobboau $
 *
 * Code to actually render stuff using Direct3D
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2002/10/05 16:46:09  randomtiger
 * Added us fs2_open people to the credits. Worth looking at just for that.
 * Added timer bar code, by default its not compiled in.
 * Use TIMEBAR_ACTIVE in project and dependancy code settings to activate.
 * Added the new timebar files with the new code.
 *
 * Revision 2.3  2002/08/07 00:45:25  DTP
 * Implented -GF4FIX commandline switch & #include "cmdline/cmdline.h"
 *
 * Revision 2.2  2002/08/03 19:42:17  randomtiger
 * Fixed Geforce 4 bug that caused font and hall video distortion.
 * Very small change in 'gr_d3d_aabitmap_ex_internal'
 *
 * Tested and works on the following systems
 *
 * OUTSIDER Voodoo 3 win98
 * OUTSIDER Geforce 2 win2000
 * Me Geforce 4 PNY 4600 XP
 * JBX-Phoenix Geforce 4 PNY XP
 * Mehrunes GeForce 3 XP
 * WMCoolmon nVidia TNT2 M64 win2000
 * Orange GeForce 4 4200 XP
 * ShadowWolf_IH Monster2 win98
 * ShadowWolf_IH voodoo 2 win98
 *
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
 * 27    9/13/99 11:30a Dave
 * Added checkboxes and functionality for disabling PXO banners as well as
 * disabling d3d zbuffer biasing.
 * 
 * 26    9/08/99 12:03a Dave
 * Make squad logos render properly in D3D all the time. Added intel anim
 * directory.
 * 
 * 25    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 24    7/30/99 4:04p Anoop
 * Fixed D3D shader.
 * 
 * 23    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 22    7/27/99 3:09p Dave
 * Made g400 work. Whee.
 * 
 * 21    7/24/99 4:19p Dave
 * Fixed dumb code with briefing bitmaps. Made d3d zbuffer work much
 * better. Made model code use zbuffer more intelligently.
 * 
 * 20    7/24/99 1:54p Dave
 * Hud text flash gauge. Reworked dead popup to use 4 buttons in red-alert
 * missions.
 * 
 * 19    7/19/99 3:29p Dave
 * Fixed gamma bitmap in the options screen.
 * 
 * 18    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 17    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 16    7/12/99 11:42a Jefff
 * Made rectangle drawing smarter in D3D. Made plines draw properly on Ati
 * Rage Pro.
 * 
 * 15    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 14    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 13    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 12    1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 11    12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 10    12/08/98 7:03p Dave
 * Much improved D3D fogging. Also put in vertex fogging for the cheesiest
 * of 3d cards.
 * 
 * 9     12/08/98 2:47p Johnson
 * Made D3D fog use eye-relative fog instead of z depth fog.
 * 
 * 8     12/08/98 9:36a Dave
 * Almost done nebula effect for D3D. Looks 85% as good as Glide.
 * 
 * 7     12/07/98 5:51p Dave
 * Finally got d3d fog working! Now we just need to tweak values.
 * 
 * 6     12/07/98 9:00a Dave
 * Fixed d3d rendered. Still don't have fog working.
 * 
 * 5     12/06/98 6:53p Dave
 * 
 * 4     12/01/98 5:54p Dave
 * Simplified the way pixel data is swizzled. Fixed tga bitmaps to work
 * properly in D3D and Glide.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 54    5/25/98 10:32a John
 * Took out redundant code for font bitmap offsets that converted to a
 * float, then later on converted back to an integer.  Quite unnecessary.
 * 
 * 53    5/24/98 6:45p John
 * let direct3d do all clipping.
 * 
 * 52    5/24/98 3:42p John
 * Let Direct3D do clipping on any linear textures, like lasers.
 * 
 * 51    5/23/98 7:18p John
 * optimized the uv bashing a bit.
 * 
 * 50    5/22/98 1:11p John
 * Added code to actually detect which offset a line needs
 * 
 * 49    5/22/98 12:54p John
 * added .5 to each pixel of a line.  This seemed to make single pixel
 * lines draw on all cards.
 * 
 * 48    5/22/98 9:00a John
 * Fixed problem of no fading out of additive textures due to Permedia2
 * fix.  Did this by dimming out the vertex RGB values.
 * 
 * 47    5/21/98 9:56p John
 * Made Direct3D work with classic alpha-blending only devices, like the
 * Virge.  Added a texture type XPARENT that fills the alpha in in the
 * bitmap for Virge.   Added support for Permedia by making making
 * additive alphablending be one/one instead of alpha/one, which didn't
 * work, and there is no way to tell this from caps.
 * 
 * 46    5/20/98 9:45p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 45    5/20/98 3:10p John
 * Made lines work even if no alphagouraud capabilities on the card.
 * 
 * 44    5/19/98 4:50p Lawrance
 * JAS: Fixed some bugs on Alan's nVidia Riva128 PCI where some
 * unitiallized fields, namely vertex->shw were causing glitches.
 * 
 * 43    5/19/98 1:46p John
 * Fixed Rendition/Riva128 uv problems.
 * 
 * 42    5/19/98 12:34p John
 * added code to fix uv's on rendition.  added code to fix zbuffering
 * problem on rendition.
 * 
 * 41    5/18/98 8:26p John
 * Made scanline be line.   Made lines work if no line alpha blending
 * supported.   Made no alpha mode use alpha off.  
 * 
 * 40    5/17/98 4:13p John
 * Made zbuffer clear only clear current clip region
 * 
 * 39    5/17/98 3:23p John
 * Took out capibility check for additive blending.  Made gr_bitmap_ex
 * clip properly in glide and direct3d.
 * 
 * 38    5/15/98 8:48a John
 * Fixed bug where one-pixel line was getting left on right and bottom.
 * 
 * 37    5/12/98 8:43p John
 * fixed particle zbuffering.
 * 
 * 36    5/12/98 10:34a John
 * Added d3d_shade functionality.  Added d3d_flush function, since the
 * shader seems to get reorganzed behind the overlay text stuff!
 * 
 * 35    5/12/98 10:06a John
 * Made all tmaps "clamp-clip".  This fixed bug with offscreen hud
 * indicators not rendering.
 * 
 * 34    5/12/98 8:18a John
 * Put in code to use a different texture format for alpha textures and
 * normal textures.   Turned off filtering for aabitmaps.  Took out
 * destblend=invsrccolor alpha mode that doesn't work on riva128. 
 * 
 * 33    5/11/98 10:58a John
 * Fixed pilot name cursor bug.  Started adding in code for alphachannel
 * textures.
 * 
 * 32    5/09/98 12:37p John
 * More texture caching
 * 
 * 31    5/09/98 12:16p John
 * Even better texture caching.
 * 
 * 30    5/08/98 10:12a John
 * took out an mprintf
 * 
 * 29    5/07/98 11:31a John
 * Removed DEMO defines
 * 
 * 28    5/07/98 10:28a John
 * Made texture format use 4444.   Made fonts use alpha to render.
 * 
 * 27    5/07/98 10:09a John
 * Fixed some bugs with short lines in D3D.
 * 
 * 26    5/07/98 9:54a John
 * Added in palette flash functionallity.
 * 
 * 25    5/07/98 9:40a John
 * Fixed some bitmap transparency issues with Direct3D.
 * 
 * 24    5/06/98 11:21p John
 * Fixed a bitmap bug with Direct3D.  Started adding new caching code into
 * D3D.
 * 
 * 23    5/06/98 8:41p John
 * Fixed some font clipping bugs.   Moved texture handle set code for d3d
 * into the texture module.
 * 
 * 22    5/06/98 8:07p John
 * made d3d clear work correctly.
 * 
 * 21    5/06/98 8:00p John
 * Got stars working under D3D.
 * 
 * 20    5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 19    5/05/98 10:37p John
 * Added code to optionally use execute buffers.
 * 
 * 18    5/04/98 3:36p John
 * Got zbuffering working with Direct3D.
 * 
 * 17    5/03/98 10:52a John
 * Made D3D sort of work on 3dfx.
 * 
 * 16    5/03/98 10:43a John
 * Working on Direct3D.
 * 
 * 15    4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 14    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 13    4/09/98 11:05a John
 * Removed all traces of Direct3D out of the demo version of Freespace and
 * the launcher.
 * 
 * 12    3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 11    3/11/98 1:55p John
 * Fixed warnings
 * 
 * 10    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 9     3/08/98 12:33p John
 * Added more lines, tris, and colored flat polys (lasers!) correctly.
 * 
 * 8     3/08/98 10:25a John
 * Added in lines
 * 
 * 7     3/07/98 8:29p John
 * Put in some Direct3D features.  Transparency on bitmaps.  Made fonts &
 * aabitmaps render nice.
 * 
 * 6     3/06/98 5:39p John
 * Started adding in aabitmaps
 * 
 * 5     3/02/98 5:42p John
 * Removed WinAVI stuff from Freespace.  Made all HUD gauges wriggle from
 * afterburner.  Made gr_set_clip work good with negative x &y.  Made
 * model_caching be on by default.  Made each cached model have it's own
 * bitmap id.  Made asteroids not rotate when model_caching is on.  
 * 
 * 4     2/26/98 3:24p John
 * fixed optimized warning
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

#include "graphics/grd3dinternal.h"
#include "graphics/2d.h"
#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/line.h"
#include "cfile/cfile.h"
#include "nebula/neb.h"
#include "render/3d.h"
#include "cmdline/cmdline.h"	//DTP for random tigers GF4fix, and the commandline switch.
#include "debugconsole/timerbar.h"

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
} gr_zbuffer_type;

int D3d_last_state = -1;

// Hack! move to another file!
extern int D3d_rendition_uvs;	

// Hack! move to another file!
extern int D3D_fog_mode;

void gr_d3d_set_state( gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt )
{
	int current_state = 0;

	current_state = current_state | (ts<<0);
	current_state = current_state | (ab<<5);
	current_state = current_state | (zt<<10);

	if ( current_state == D3d_last_state ) {
		return;
	}
	D3d_last_state = current_state;

	switch( ts )	{
	case TEXTURE_SOURCE_NONE:
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL );
		// Let the texture cache system know whe set the handle to NULL
		gr_tcache_set(-1, -1, NULL, NULL );

		break;
	case TEXTURE_SOURCE_DECAL:
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );

		if ( lpDevDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA )	{
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
		} else {
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );
		}
		break;

	case TEXTURE_SOURCE_NO_FILTERING:
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST );
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST );
		if ( lpDevDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA )	{
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
		} else {
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );
		}
		break;

	default:
		Int3();
	}

	switch( ab )	{
	case ALPHA_BLEND_NONE:							// 1*SrcPixel + 0*DestPixel
		d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		break;

	case ALPHA_BLEND_ALPHA_ADDITIVE:				// Alpha*SrcPixel + 1*DestPixel
	case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
		if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			// Must use ONE:ONE as the Permedia2 can't do SRCALPHA:ONE.
			// But I lower RGB values so we don't loose anything.
			d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE );
			//d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
			d3d_SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
			break;
		}
		// Fall through to normal alpha blending mode...

	case ALPHA_BLEND_ALPHA_BLEND_ALPHA:			// Alpha*SrcPixel + (1-Alpha)*DestPixel
		if ( lpDevDesc->dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA  )	{
			d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
			d3d_SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		} else {
			d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_BOTHSRCALPHA );
		}
		break;


	default:
		Int3();
	}

	switch( zt )	{

	case ZBUFFER_TYPE_NONE:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
		break;

	case ZBUFFER_TYPE_READ:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
		break;

	case ZBUFFER_TYPE_WRITE:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
		break;

	case ZBUFFER_TYPE_FULL:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
		break;

	default:
		Int3();
	}

}

extern int D3D_zbias;
void d3d_zbias(int bias)
{
	if(D3D_zbias){
		d3d_SetRenderState(D3DRENDERSTATE_ZBIAS, bias);
	}
}

// If mode is FALSE, turn zbuffer off the entire frame,
// no matter what people pass to gr_zbuffer_set.
void gr_d3d_zbuffer_clear(int mode)
{
	if ( mode )	{
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		// Make sure zbuffering is on
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );


		// An application can clear z-buffers by using the IDirectDrawSurface2::Blt method. 
		// The DDBLT_DEPTHFILL flag indicates that the blit clears z-buffers. If this flag 
		// is specified, the DDBLTFX structure passed to the IDirectDrawSurface2::Blt method 
		// should have its dwFillDepth member set to the required z-depth. If the DirectDraw device 
		// driver for a 3D-accelerated display card is designed to provide support for z-buffer 
		// clearing in hardware, it should export the DDCAPS_BLTDEPTHFILL flag and should 
		// handle DDBLT_DEPTHFILL blits. The destination surface of a depth-fill blit must 
		// be a z-buffer.
		// Note The actual interpretation of a depth value is specific to the 3D renderer.

		D3DRECT rect;

		rect.x1 = gr_screen.clip_left + gr_screen.offset_x;
		rect.y1 = gr_screen.clip_top + gr_screen.offset_y;
		rect.x2 = gr_screen.clip_right + gr_screen.offset_x;
		rect.y2 = gr_screen.clip_bottom + gr_screen.offset_y;

		if (lpViewport->Clear( 1, &rect, D3DCLEAR_ZBUFFER ) != D3D_OK )	{
			mprintf(( "Failed to clear zbuffer!\n" ));
			return;
		}


	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
	}
}

// internal d3d rect function
void gr_d3d_rect_internal(int x, int y, int w, int h, int r, int g, int b, int a)
{
	int saved_zbuf;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};

	saved_zbuf = gr_zbuffer_get();
	
	// start the frame, no zbuffering, no culling
	g3_start_frame(1);	
	gr_zbuffer_set(GR_ZBUFF_NONE);		
	gr_set_cull(0);		

	// stuff coords		
	v[0].sx = i2fl(x);
	v[0].sy = i2fl(y);
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;
	v[0].r = (ubyte)r;
	v[0].g = (ubyte)g;
	v[0].b = (ubyte)b;
	v[0].a = (ubyte)a;

	v[1].sx = i2fl(x + w);
	v[1].sy = i2fl(y);	
	v[1].sw = 0.0f;
	v[1].u = 0.0f;
	v[1].v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;
	v[1].r = (ubyte)r;
	v[1].g = (ubyte)g;
	v[1].b = (ubyte)b;
	v[1].a = (ubyte)a;

	v[2].sx = i2fl(x + w);
	v[2].sy = i2fl(y + h);
	v[2].sw = 0.0f;
	v[2].u = 0.0f;
	v[2].v = 0.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;
	v[2].r = (ubyte)r;
	v[2].g = (ubyte)g;
	v[2].b = (ubyte)b;
	v[2].a = (ubyte)a;

	v[3].sx = i2fl(x);
	v[3].sy = i2fl(y + h);
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 0.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;				
	v[3].r = (ubyte)r;
	v[3].g = (ubyte)g;
	v[3].b = (ubyte)b;
	v[3].a = (ubyte)a;

	// draw the polys
	g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB | TMAP_FLAG_ALPHA, 0.1f);		

	g3_end_frame();

	// restore zbuffer and culling
	gr_zbuffer_set(saved_zbuf);
	gr_set_cull(1);	
}

int gr_d3d_zbuffer_get()
{
	if ( !gr_global_zbuffering )	{
		return GR_ZBUFF_NONE;
	}
	return gr_zbuffering_mode;
}

int gr_d3d_zbuffer_set(int mode)
{
	/*
	if ( !gr_global_zbuffering )	{
		gr_zbuffering = 0;
		return GR_ZBUFF_NONE;
	}
	*/

	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if ( gr_zbuffering_mode == GR_ZBUFF_NONE )	{
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	return tmp;
}

float D3D_line_offset = 0.0f;

void d3d_make_rect( D3DTLVERTEX *a, D3DTLVERTEX *b, int x1, int y1, int x2, int y2 )
{
	// Alan's nvidia riva128 PCI screws up targetting brackets if 
	// rhw are uninitialized.
	a->rhw = 1.0f;
	b->rhw = 1.0f;

	// just for completeness, initialize specular and sz.
	a->specular = 0;
	b->specular = 0;

	a->sz = 0.99f;
	b->sz = 0.99f;

	a->sx = i2fl(x1 + gr_screen.offset_x)+D3D_line_offset;
	a->sy = i2fl(y1 + gr_screen.offset_y)+D3D_line_offset;

	b->sx = i2fl(x2 + gr_screen.offset_x)+D3D_line_offset;
	b->sy = i2fl(y2 + gr_screen.offset_y)+D3D_line_offset;

	if ( x1 == x2 )	{
		// Verticle line
		if ( a->sy < b->sy )	{
			b->sy += 0.5f;
		} else {
			a->sy += 0.5f;
		}
	} else if ( y1 == y2 )	{
		// Horizontal line
		if ( a->sx < b->sx )	{
			b->sx += 0.5f;
		} else {
			a->sx += 0.5f;
		}
	}

}

// basically just fills in the alpha component of the specular color. Hardware does the rest
// when rendering the poly
void gr_d3d_stuff_fog_value(float z, D3DCOLOR *spec)
{
	float f_float;	
	*spec = 0;

	// linear fog formula
	f_float = (gr_screen.fog_far - z) / (gr_screen.fog_far - gr_screen.fog_near);
	if(f_float < 0.0f){
		f_float = 0.0f;
	} else if(f_float > 1.0f){
		f_float = 1.0f;
	}
	*spec = D3DRGBA(0.0f, 0.0f, 0.0f, f_float);
}

float z_mult = 30000.0f;
DCF(zmult, "")
{
	dc_get_arg(ARG_FLOAT);
	z_mult = Dc_arg_float;
}

float flCAP( float x, float minx, float maxx)
{
	if ( x < minx )	{
		return minx;
	} else if ( x > maxx )	{
		return maxx;
	}
	return x;
}

#define NEBULA_COLORS 20

void gr_d3d_tmapper_internal( int nverts, vertex **verts, uint flags, int is_scaler )	
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;
	int bw = 1, bh = 1;		

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3();
		/*
		flags |= TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT;

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

		for (i=0; i<nverts; i++ )	{
			verts[i]->u = verts[i]->v = 0.5f;
		}
		*/
	}

	gr_texture_source texture_source = (gr_texture_source)-1;
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

	int alpha;

	int tmap_type = TCACHE_TYPE_NORMAL;

	int r, g, b;

	if ( flags & TMAP_FLAG_TEXTURED )	{
		r = 255;
		g = 255;
		b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{

		if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			tmap_type = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if ( factor <= 1.0f )	{
				int tmp_alpha = fl2i(gr_screen.current_alpha*255.0f);
				r = (r*tmp_alpha)/255;
				g = (g*tmp_alpha)/255;
				b = (b*tmp_alpha)/255;
			}
		} else {

			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if ( factor > 1.0f )	{
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
	} else {
		if(Bm_pixel_format == BM_PIXEL_FORMAT_ARGB_D3D){
			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		} else {
			alpha_blend = ALPHA_BLEND_NONE;
		}
		alpha = 255;
	}

	if(flags & TMAP_FLAG_BITMAP_SECTION){
		tmap_type = TCACHE_TYPE_BITMAP_SECTION;
	}

	texture_source = TEXTURE_SOURCE_NONE;
 
	if ( flags & TMAP_FLAG_TEXTURED )	{
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))	{
			mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		// use nonfiltered textures for bitmap sections
		if(flags & TMAP_FLAG_BITMAP_SECTION){
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	}
	
	gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
	
	D3DTLVERTEX d3d_verts[32];
	D3DTLVERTEX *src_v = d3d_verts;

	int x1, y1, x2, y2;
	x1 = gr_screen.clip_left*16;
	x2 = gr_screen.clip_right*16+15;
	y1 = gr_screen.clip_top*16;
	y2 = gr_screen.clip_bottom*16+15;

	float uoffset = 0.0f;
	float voffset = 0.0f;

	float minu=0.0f, minv=0.0f, maxu=1.0f, maxv=1.0f;

	if ( flags & TMAP_FLAG_TEXTURED )	{								
		if ( D3d_rendition_uvs )	{				
			bm_get_info(gr_screen.current_bitmap, &bw, &bh);			
				
			uoffset = 2.0f/i2fl(bw);
			voffset = 2.0f/i2fl(bh);

			minu = uoffset;
			minv = voffset;

			maxu = 1.0f - uoffset;
			maxv = 1.0f - voffset;
		}				
	}	

	// turn on pixel fog if we're rendering against a fullneb background
	// if(flags & TMAP_FLAG_PIXEL_FOG){					
		// set fog
 	//	gr_fog_set(GR_FOGMODE_FOG, gr_screen.current_fog_color.red, gr_screen.current_fog_color.green, gr_screen.current_fog_color.blue);
	// }					
		
	for (i=0; i<nverts; i++ )	{
		vertex * va = verts[i];		
				
		// store in case we're doing vertex fog.		
		if ( gr_zbuffering || (flags & TMAP_FLAG_NEBULA) )	{
			src_v->sz = va->z / z_mult;	// For zbuffering and fogging
			if ( src_v->sz > 0.98f )	{
				src_v->sz = 0.98f;
			}		
		} else {
			src_v->sz = 0.99f;
		}			

		if ( flags & TMAP_FLAG_CORRECT )	{
			src_v->rhw = va->sw;				// For texture correction 						
		} else {
			src_v->rhw = 1.0f;				// For texture correction 
		}

		int a;

		if ( flags & TMAP_FLAG_ALPHA )	{
			a = verts[i]->a;
		} else {
			a = alpha;
		}

		if ( flags & TMAP_FLAG_NEBULA )	{
			int pal = (verts[i]->b*(NEBULA_COLORS-1))/255;
			r = gr_palette[pal*3+0];
			g = gr_palette[pal*3+1];
			b = gr_palette[pal*3+2];
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) )	{
			r = Gr_gamma_lookup[verts[i]->b];
			g = Gr_gamma_lookup[verts[i]->b];
			b = Gr_gamma_lookup[verts[i]->b];
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{
			// Make 0.75 be 256.0f
			r = Gr_gamma_lookup[verts[i]->r];
			g = Gr_gamma_lookup[verts[i]->g];
			b = Gr_gamma_lookup[verts[i]->b];
		} else {
			// use constant RGB values...
		}

		src_v->color = RGBA_MAKE(r, g, b, a);

		// if we're fogging and we're doing vertex fog
		if((gr_screen.current_fog_mode != GR_FOGMODE_NONE) && (D3D_fog_mode == 1)){
			gr_d3d_stuff_fog_value(va->z, &src_v->specular);
		} else {
			src_v->specular = 0;
		}

		int x, y;
		x = fl2i(va->sx*16.0f);
		y = fl2i(va->sy*16.0f);

		x += gr_screen.offset_x*16;
		y += gr_screen.offset_y*16;
		
		src_v->sx = i2fl(x) / 16.0f;
		src_v->sy = i2fl(y) / 16.0f;

		if ( flags & TMAP_FLAG_TEXTURED )	{
			// argh. rendition
			if ( D3d_rendition_uvs ){				
				// tiled texture (ships, etc), bitmap sections
				if(flags & TMAP_FLAG_TILED){					
					src_v->tu = va->u*u_scale;
					src_v->tv = va->v*v_scale;
				}
				// sectioned
				else if(flags & TMAP_FLAG_BITMAP_SECTION){
					int sw, sh;
					bm_get_section_size(gr_screen.current_bitmap, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, &sw, &sh);

					src_v->tu = (va->u + (0.5f / i2fl(sw))) * u_scale;
					src_v->tv = (va->v + (0.5f / i2fl(sh))) * v_scale;
				}
				// all else.
				else {				
					src_v->tu = flCAP(va->u, minu, maxu);
					src_v->tv = flCAP(va->v, minv, maxv);
				}				
			}
			// yay. non-rendition
			else {
				src_v->tu = va->u*u_scale;
				src_v->tv = va->v*v_scale;
			}							
		} else {
			src_v->tu = 0.0f;
			src_v->tv = 0.0f;
		}
		src_v++;
	}

	// if we're rendering against a fullneb background
	if(flags & TMAP_FLAG_PIXEL_FOG){	
		int r, g, b;
		int ra, ga, ba;		
		ra = ga = ba = 0;		

		// get the average pixel color behind the vertices
		for(i=0; i<nverts; i++){			
			neb2_get_pixel((int)d3d_verts[i].sx, (int)d3d_verts[i].sy, &r, &g, &b);
			ra += r;
			ga += g;
			ba += b;
		}				
		ra /= nverts;
		ga /= nverts;
		ba /= nverts;		

		// set fog
		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}					

	d3d_DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, (LPVOID)d3d_verts, nverts, NULL);


	//this is my hack to get some sort of luminence mapping-Bobboau
	if(GLOWMAP[gr_screen.current_bitmap] > 0){

		gr_screen.gf_set_bitmap(GLOWMAP[gr_screen.current_bitmap], GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.0f);
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy ))	{
			mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, zbuffer_type );

		for (i=0; i<nverts; i++ )	{
			d3d_verts[i].color = RGBA_MAKE(255, 255, 255, 0);
		}
		d3d_DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, (LPVOID)d3d_verts, nverts, NULL);
	}
	// turn off fog
	// if(flags & TMAP_FLAG_PIXEL_FOG){
		// gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	// }
}

void gr_d3d_tmapper( int nverts, vertex **verts, uint flags )	
{
	gr_d3d_tmapper_internal( nverts, verts, flags, 0 );
}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

void gr_d3d_scaler(vertex *va, vertex *vb )
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

	gr_d3d_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
}

void gr_d3d_aascaler(vertex *va, vertex *vb )
{
}


void gr_d3d_pixel(int x, int y)
{
	gr_line(x,y,x,y);
}


void gr_d3d_clear()
{
	// Turn off zbuffering so this doesn't clear the zbuffer
	gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

	RECT dst;
	DDBLTFX ddbltfx;
	DDSURFACEDESC ddsd;	

	// Get the surface desc
	ddsd.dwSize = sizeof(ddsd);
	lpBackBuffer->GetSurfaceDesc(&ddsd);   

	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(DDBLTFX);

	ddbltfx.dwFillColor = RGB_MAKE(gr_screen.current_clear_color.red, gr_screen.current_clear_color.green, gr_screen.current_clear_color.blue);

	dst.left = gr_screen.clip_left+gr_screen.offset_x;
	dst.top = gr_screen.clip_top+gr_screen.offset_y;
	dst.right = gr_screen.clip_right+1+gr_screen.offset_x;
	dst.bottom = gr_screen.clip_bottom+1+gr_screen.offset_y;	

	if ( lpBackBuffer->Blt( &dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx ) != DD_OK )	{
		return;
	}
}


// sets the clipping region & offset
void gr_d3d_set_clip(int x,int y,int w,int h)
{
	gr_screen.offset_x = x;
	gr_screen.offset_y = y;

	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;

	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;

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

	// Setup the viewport for a reasonable viewing area
	D3DVIEWPORT viewdata;
	DWORD       largest_side;
	HRESULT		ddrval;

	// Compensate for aspect ratio
	if ( gr_screen.clip_width > gr_screen.clip_height )
		largest_side = gr_screen.clip_width;
	else
		largest_side = gr_screen.clip_height;

	viewdata.dwSize = sizeof( viewdata );
	viewdata.dwX = gr_screen.clip_left+x;
	viewdata.dwY = gr_screen.clip_top+y;
	viewdata.dwWidth = gr_screen.clip_width;
	viewdata.dwHeight = gr_screen.clip_height;
	viewdata.dvScaleX = largest_side / 2.0F;
	viewdata.dvScaleY = largest_side / 2.0F;
	viewdata.dvMaxX = ( float ) ( viewdata.dwWidth / ( 2.0F * viewdata.dvScaleX ) );
	viewdata.dvMaxY = ( float ) ( viewdata.dwHeight / ( 2.0F * viewdata.dvScaleY ) );
	viewdata.dvMinZ = 0.0F;
	viewdata.dvMaxZ = 0.0F; // choose something appropriate here!

	ddrval = lpViewport->SetViewport( &viewdata );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_D3D_SET_CLIP: SetViewport failed.\n" ));
	}

}


void gr_d3d_reset_clip()
{
	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;

	// Setup the viewport for a reasonable viewing area
	D3DVIEWPORT viewdata;
	DWORD       largest_side;
	HRESULT		ddrval;

	// Compensate for aspect ratio
	if ( gr_screen.clip_width > gr_screen.clip_height )
		largest_side = gr_screen.clip_width;
	else
		largest_side = gr_screen.clip_height;

	viewdata.dwSize = sizeof( viewdata );
	viewdata.dwX = gr_screen.clip_left;
	viewdata.dwY = gr_screen.clip_top;
	viewdata.dwWidth = gr_screen.clip_width;
	viewdata.dwHeight = gr_screen.clip_height;
	viewdata.dvScaleX = largest_side / 2.0F;
	viewdata.dvScaleY = largest_side / 2.0F;
	viewdata.dvMaxX = ( float ) ( viewdata.dwWidth / ( 2.0F * viewdata.dvScaleX ) );
	viewdata.dvMaxY = ( float ) ( viewdata.dwHeight / ( 2.0F * viewdata.dvScaleY ) );
	viewdata.dvMinZ = 0.0F;
	viewdata.dvMaxZ = 0.0F; // choose something appropriate here!

	ddrval = lpViewport->SetViewport( &viewdata );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_D3D_SET_CLIP: SetViewport failed.\n" ));
	}

}

void gr_d3d_init_color(color *c, int r, int g, int b)
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

void gr_d3d_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( alpha < 0 ) alpha = 0; else if ( alpha > 255 ) alpha = 255;

	gr_d3d_init_color( clr, r, g, b );

	clr->alpha = unsigned char(alpha);
	clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

void gr_d3d_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_d3d_init_color( &gr_screen.current_color, r, g, b );
}

void gr_d3d_get_color( int * r, int * g, int * b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

void gr_d3d_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )	{
			gr_d3d_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_d3d_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}
	gr_screen.current_color = *dst;
}

void gr_d3d_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
	gr_screen.current_bitmap_sx = sx;
	gr_screen.current_bitmap_sy = sy;
}

void gr_d3d_bitmap_ex_internal(int x,int y,int w,int h,int sx,int sy)
{
	int i,j;
	bitmap * bmp;
	ushort * sptr;
	ushort * dptr;
	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	ddrval = lpBackBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "Error locking surface for bitmap_ex, %s\n", d3d_error_string(ddrval) ));
		return;
	}

	dptr = (ushort *)((int)ddsd.lpSurface+ddsd.lPitch*(y+gr_screen.offset_y)+(x+gr_screen.offset_x)*2);

	bmp = bm_lock( gr_screen.current_bitmap, 16, 0 );
	sptr = (ushort *)( bmp->data + (sy*bmp->w + sx) );

	// nice and speedy compared to the old way
	for (i=0; i<h; i++ )	{
		for ( j=0; j<w; j++ )	{				
			// if its not transparent, blit it			
			if(sptr[j] != Gr_green.mask){
				dptr[j] = sptr[j];
			}
		}
		
		// next row on the screen
		dptr = (ushort *)((uint)dptr + ddsd.lPitch);

		// next row in the bitmap
		sptr += bmp->w;
	}	

	bm_unlock(gr_screen.current_bitmap);

	// Unlock the back buffer
	lpBackBuffer->Unlock( NULL );
}

void gr_d3d_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
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

	gr_d3d_bitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_d3d_bitmap(int x, int y)
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

	gr_d3d_bitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}



void gr_d3d_aabitmap_ex_internal(int x,int y,int w,int h,int sx,int sy)
{
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	if ( !gr_screen.current_color.is_alphacolor )	return;

	float u_scale, v_scale;

	gr_d3d_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		//mprintf(( "GLIDE: Error setting aabitmap texture!\n" ));
		return;
	}

	LPD3DTLVERTEX src_v;
	D3DTLVERTEX d3d_verts[4];

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh;

	bm_get_info( gr_screen.current_bitmap, &bw, &bh );

	// Rendition 
	if ( D3d_rendition_uvs )	{
		u0 = u_scale*(i2fl(sx)+0.5f)/i2fl(bw);
		v0 = v_scale*(i2fl(sy)+0.5f)/i2fl(bh);

		u1 = u_scale*(i2fl(sx+w)+0.5f)/i2fl(bw);
		v1 = v_scale*(i2fl(sy+h)+0.5f)/i2fl(bh);
	//} else { //DTP; commented out, for easy re-editing
	//	u0 = u_scale*i2fl((sx)-0.5f)/i2fl(bw);
	//	v0 = v_scale*i2fl((sy)-0.5f)/i2fl(bh);

	//	u1 = u_scale*i2fl((sx+w)-0.5f)/i2fl(bw);
	//	v1 = v_scale*i2fl((sy+h)-0.5f)/i2fl(bh);
	//}
	} else if (!Cmdline_gf4fix) { //DTP if not set at commandline do the original code
		u0 = u_scale*i2fl(sx)/i2fl(bw);
		v0 = v_scale*i2fl(sy)/i2fl(bh);
		u1 = u_scale*i2fl(sx+w)/i2fl(bw);
		v1 = v_scale*i2fl(sy+h)/i2fl(bh);
	} else {	//DTP; these next 4 lines is random tigers Fix. activated by setting -GF4FIX in the commandline.
		u0 = u_scale*(i2fl(sx)-0.5f)/i2fl(bw);
		v0 = v_scale*(i2fl(sy)-0.5f)/i2fl(bh);
		u1 = u_scale*(i2fl(sx+w)-0.5f)/i2fl(bw);
		v1 = v_scale*(i2fl(sy+h)-0.5f)/i2fl(bh);
	}

	x1 = i2fl(x+gr_screen.offset_x);
	y1 = i2fl(y+gr_screen.offset_y);
	x2 = i2fl(x+w+gr_screen.offset_x);
	y2 = i2fl(y+h+gr_screen.offset_y);

	src_v = d3d_verts;

	uint color;

	if ( gr_screen.current_color.is_alphacolor )	{
		if ( lpDevDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA )	{
			color = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue,gr_screen.current_color.alpha);
		} else {
			int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
			int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
			int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;
		
			color = RGBA_MAKE(r,g,b, 255 );
		}
	} else {
		color = RGB_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x1;
	src_v->sy = y1;
	src_v->tu = u0;
	src_v->tv = v0;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x2;
	src_v->sy = y1;
	src_v->tu = u1;
	src_v->tv = v0;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x2;
	src_v->sy = y2;
	src_v->tu = u1;
	src_v->tv = v1;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x1;
	src_v->sy = y2;
	src_v->tu = u0;
	src_v->tv = v1;

	d3d_DrawPrimitive(D3DPT_TRIANGLEFAN,D3DVT_TLVERTEX,(LPVOID)d3d_verts,4,NULL);
}

void gr_d3d_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
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
	gr_d3d_aabitmap_ex_internal(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_d3d_aabitmap(int x, int y)
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


void gr_d3d_string( int sx, int sy, char *s )
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

		//not in font, draw as space
		if (letter<0)	{
			continue;
		}

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

		gr_d3d_aabitmap_ex_internal( xc, yc, wc, hc, u+xd, v+yd );
	}
}

void gr_d3d_rect(int x,int y,int w,int h)
{
	gr_d3d_rect_internal(x, y, w, h, gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);	
}

void gr_d3d_flash(int r, int g, int b)
{
	CAP(r,0,255);
	CAP(g,0,255);
	CAP(b,0,255);

	if ( r || g || b )	{
		uint color;
		if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
			color = RGBA_MAKE(r, g, b, 255);
		} else {
			gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
	
			int a = (r+g+b)/3;
			color = RGBA_MAKE(r,g,b,a);
		}
	
		float x1, x2, y1, y2;
		x1 = i2fl(gr_screen.clip_left+gr_screen.offset_x);
		y1 = i2fl(gr_screen.clip_top+gr_screen.offset_y);
		x2 = i2fl(gr_screen.clip_right+gr_screen.offset_x);
		y2 = i2fl(gr_screen.clip_bottom+gr_screen.offset_y);
	
		LPD3DTLVERTEX src_v;
		D3DTLVERTEX d3d_verts[4];

		src_v = d3d_verts;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x1;
		src_v->sy = y1;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x2;
		src_v->sy = y1;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x2;
		src_v->sy = y2;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x1;
		src_v->sy = y2;

		d3d_DrawPrimitive(D3DPT_TRIANGLEFAN,D3DVT_TLVERTEX,(LPVOID)d3d_verts,4,NULL);
	}
}



void gr_d3d_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;
}

void gr_d3d_set_shader( shader * shade )
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

void gr_d3d_shade(int x,int y,int w,int h)
{	
	int r,g,b,a;

	float shade1 = 1.0f;
	float shade2 = 6.0f;

	r = fl2i(gr_screen.current_shader.r*255.0f*shade1);
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	g = fl2i(gr_screen.current_shader.g*255.0f*shade1);
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	b = fl2i(gr_screen.current_shader.b*255.0f*shade1);
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	a = fl2i(gr_screen.current_shader.c*255.0f*shade2);
	if ( a < 0 ) a = 0; else if ( a > 255 ) a = 255;

	gr_d3d_rect_internal(x, y, w, h, r, g, b, a);	
}

void gr_d3d_circle( int xc, int yc, int d )
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
		gr_d3d_line( xc-y, yc-x, xc+y, yc-x );
		gr_d3d_line( xc-y, yc+x, xc+y, yc+x );

		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			gr_d3d_line( xc-x, yc-y, xc+x, yc-y );
			gr_d3d_line( xc-x, yc+y, xc+x, yc+y );
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y)	{
		gr_d3d_line( xc-x, yc-y, xc+x, yc-y );
		gr_d3d_line( xc-x, yc+y, xc+x, yc+y );
	}
	return;

}


void gr_d3d_line(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;
	DWORD color;

	// Set up Render State - flat shading - alpha blending
	if ( (lpDevDesc->dpcLineCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && (lpDevDesc->dpcLineCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)  )	{
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
		color = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );
	} else {
		// Matrox MGA-G200 doesn't support alpha-blended lines.
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

		int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
		int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
		int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;
		
		color = RGBA_MAKE(r,g,b, 255 );
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	D3DTLVERTEX d3d_verts[2];
	D3DTLVERTEX *a = d3d_verts;
	D3DTLVERTEX *b = d3d_verts+1;

	d3d_make_rect(a,b,x1,y1,x2,y2);

	a->color = color;
	b->color = color;

	d3d_DrawPrimitive(D3DPT_LINELIST,D3DVT_TLVERTEX,(LPVOID)d3d_verts,2,NULL);
}

void gr_d3d_aaline(vertex *v1, vertex *v2)
{
	gr_d3d_line( fl2i(v1->sx), fl2i(v1->sy), fl2i(v2->sx), fl2i(v2->sy) );
}


void gr_d3d_gradient(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	if ( !gr_screen.current_color.is_alphacolor )	{
		gr_line( x1, y1, x2, y2 );
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	uint color1, color2;

	// Set up Render State - flat shading - alpha blending
	if ( (lpDevDesc->dpcLineCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && (lpDevDesc->dpcLineCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)  )	{
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

		if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND )	{
			color1 = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );
			color2 = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, 0 );
		} else if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB )	{
			color1 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
			color2 = RGBA_MAKE(0,0,0,gr_screen.current_color.alpha);
		} else {
			color1 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
			color2 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
		}
	} else {
		// Matrox MGA-G200 doesn't support alpha-blended lines.
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

		int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
		int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
		int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;

		if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB )	{
			color1 = RGBA_MAKE(r,g,b, 255 );
			color2 = RGBA_MAKE(0,0,0, 255 );
		} else {
			color1 = RGBA_MAKE(r,g,b, 255 );
			color2 = RGBA_MAKE(r,g,b, 255 );
		}
	}

//	gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );
//	color1 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,255);
//	color2 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,255);

	D3DTLVERTEX d3d_verts[2];
	D3DTLVERTEX *a = d3d_verts;
	D3DTLVERTEX *b = d3d_verts+1;

	d3d_make_rect( a, b, x1, y1, x2, y2 );

	if ( swapped )	{
		b->color = color1;
		a->color = color2;
	} else {
		a->color = color1;
		b->color = color2;
	}
	d3d_DrawPrimitive(D3DPT_LINELIST,D3DVT_TLVERTEX,(LPVOID)d3d_verts,2,NULL);
}


void gr_d3d_set_palette(ubyte *new_palette, int restrict_alphacolor)
{
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
	if ( (fromsize == 2) && (tosize == 3) )	{
		ushort *src = (ushort *)from;
		ubyte *dst  = (ubyte *)to;

		int i;
		for (i=0; i<pixels; i++ )	{
			ushort pixel = *src++;

			*dst++ = ubyte(((pixel & Gr_blue.mask)>>Gr_blue.shift)*Gr_blue.scale);
			*dst++ = ubyte(((pixel & Gr_green.mask)>>Gr_green.shift)*Gr_green.scale);
			*dst++ = ubyte(((pixel & Gr_red.mask)>>Gr_red.shift)*Gr_red.scale);
		}
		return tosize*pixels;
	} else if( (fromsize == 4) && (tosize == 3) ){
		uint *src = (uint *)from;
		ubyte *dst  = (ubyte *)to;

		int i;
		for (i=0; i<pixels; i++ )	{
			uint pixel = *src++;

			*dst++ = ubyte(((pixel & Gr_blue.mask)>>Gr_blue.shift)*Gr_blue.scale);
			*dst++ = ubyte(((pixel & Gr_green.mask)>>Gr_green.shift)*Gr_green.scale);
			*dst++ = ubyte(((pixel & Gr_red.mask)>>Gr_red.shift)*Gr_red.scale);
		}
		return tosize*pixels;
	}	else {
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

static int tga_compress(char *out, char *in, int bytecount, int pixsize )
{	
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

void gr_d3d_print_screen(char *filename)
{
	HRESULT ddrval;
	DDSURFACEDESC ddsd;
	ubyte outrow[1024*3*4];

	if ( gr_screen.max_w > 1024 )	{
		mprintf(( "Screen too wide for print_screen\n" ));
		return;
	}

	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	ddrval = lpBackBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "Error locking surface for print_screen, %s\n", d3d_error_string(ddrval) ));
	} 

	ubyte *dptr = (ubyte *)ddsd.lpSurface;

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
	cfwrite_ushort( (ushort)gr_screen.max_w, f );	//	Width;
	cfwrite_ushort( (ushort)gr_screen.max_h, f );	//	Height;
	cfwrite_ubyte( 24, f );	//PixelDepth;
	cfwrite_ubyte( 0, f );	//ImageDesc;	

	// Go through and read our pixels
	int i;
	for (i=0;i<gr_screen.max_h;i++)	{
		int len = tga_compress( (char *)outrow, (char *)&dptr[(gr_screen.max_h-i-1)*ddsd.lPitch], gr_screen.max_w * gr_screen.bytes_per_pixel, gr_screen.bytes_per_pixel );

		cfwrite(outrow, len, 1, f);
	}

	cfclose(f);

	// Unlock the back buffer
	lpBackBuffer->Unlock( NULL );

}

void d3d_render_timer_bar(int colour, float x, float y, float w, float h)
{
	static D3DCOLOR pre_set_colours[TIMERBAR_COLOUR_MAX] = 
	{
		0xffff0000, // red
		0xff00ff00, // green
		0xff0000ff, // blue
	};

	D3DTLVERTEX d3d_verts[4];

	static float max_fw = (float) gr_screen.max_w; 
	static float max_fh = (float) gr_screen.max_h; 

	d3d_verts[0].rhw   = 1;
	d3d_verts[0].color = pre_set_colours[colour];
	d3d_verts[0].sx = max_fw * x;
	d3d_verts[0].sy = max_fh * y;

	d3d_verts[1].rhw   = 1;
	d3d_verts[1].color = pre_set_colours[colour];
	d3d_verts[1].sx = max_fw * (x + w);
	d3d_verts[1].sy = max_fh * y;

	d3d_verts[2].rhw   = 1;
	d3d_verts[2].color = pre_set_colours[colour];
	d3d_verts[2].sx = max_fw * (x + w);
	d3d_verts[2].sy = max_fh * (y + h);

	d3d_verts[3].rhw   = 1;
	d3d_verts[3].color = pre_set_colours[colour];
	d3d_verts[3].sx = max_fw * x;
	d3d_verts[3].sy = max_fh * (y + h);

	gr_d3d_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE);
	d3d_DrawPrimitive(D3DPT_TRIANGLEFAN,D3DVT_TLVERTEX,(LPVOID)d3d_verts,4,NULL);
}





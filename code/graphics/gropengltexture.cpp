/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLTexture.cpp $
 * $Revision: 1.48.2.16 $
 * $Date: 2007-10-04 16:18:19 $
 * $Author: taylor $
 *
 * source for texturing in OpenGL
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.48.2.15  2007/03/22 20:50:27  taylor
 * some generic code cleanup
 *
 * Revision 1.48.2.14  2007/02/11 10:03:29  taylor
 * remove some dead code
 * general cleanup
 * fix issues with texture state changes not affecting things properly
 * fix stupid texture addressing thing that I had done a long time ago
 * deal with -img2dds error issue
 *
 * Revision 1.48.2.13  2006/12/27 09:24:31  taylor
 * wurrps, someone besides me might actually want to use that ;)
 *
 * Revision 1.48.2.12  2006/12/26 05:25:18  taylor
 * lots of little cleanup, stale code removal, and small performance adjustments
 * get rid of the default combine texture state, we don't need it in general, and it can screw up fonts
 * get rid of the secondary color support, it doesn't do much in non-HTL mode, screws up various things, and has long since been obsolete but material setup
 * get rid of the old gamma setup, it actually conflicts with newer gamma support
 * default texture wrapping to edge clamp
 * do second gr_clear() on init to be sure and catch double-buffer
 * make sure that our active texture will always get reset to 0, rather than leaving it at whatever was used last
 * fixed that damn FBO bug from it hanging on textures and causing some rendering errors for various people
 * only lock verts once in HTL model rendering
 *
 * Revision 1.48.2.11  2006/12/07 18:14:49  taylor
 * get rid of "vram_full", it can never actually be full the way the code works, so having this is redundant
 * comment out APPLE_client_storage extension usage, I think this was messing some stuff up on OS X and causing memory errors
 * fix handling of cubemap data so that they can be read properly (can't believe it took so long to notice to an obvious and stupid bug)
 * be sure to un-bind textures during preload to avoid freaky little errors later
 * ifdef an error message from the framebuffer error code, Apple doesn't have this one define in their headers for some reason
 *
 * Revision 1.48.2.10  2006/08/12 13:14:45  taylor
 * some minor cleanup to anisotropy setting
 * only use negative lod bias when not using anisotropy to help avoid shimmering
 * stick lod bias setting in with anisotropy function
 *
 * Revision 1.48.2.9  2006/08/09 14:40:43  taylor
 * fix for setting of texture lod bias
 *
 * Revision 1.48.2.8  2006/07/17 01:06:41  taylor
 * maybe this will finally shut-up some of the people who have complained about certain mipmap issues
 *
 * Revision 1.48.2.7  2006/07/15 06:10:51  taylor
 * change clamp mode for font/interface graphics, don't want the border factored in for scaling/filtering
 *
 * Revision 1.48.2.6  2006/06/22 14:59:44  taylor
 * fix various things that Valgrind has been complaining about
 *
 * Revision 1.48.2.5  2006/06/19 22:50:58  taylor
 * as a temporary measure, until I can figure out what is going on here:
 *   - disable depth buffer for RTT, may be causing the ATI issues, should work ok without it for most things
 *   - disable mipmap generation for RTT, something strange is going on there, it's sometimes making black mipmap levels (even for the same texture)
 *
 * Revision 1.48.2.4  2006/06/18 16:49:40  taylor
 * fix so that multiple FBOs can be used with different sizes (plus a few other minor adjustments)
 *
 * Revision 1.48.2.3  2006/06/15 00:15:17  taylor
 * fix Assert() on value of face variable, it should be able to be -1 for non-cubemap images
 *
 * Revision 1.48.2.2  2006/06/12 03:37:24  taylor
 * sync current OGL changes:
 *  - go back to using minimize mode which non-active, but doin't minimize when Fred_running
 *  - remove temporary cmdline options (-spec_scale, -env_scale, -alpha_alpha_blend)
 *  - change FBO renderbuffer link around a little to maybe avoid freaky drivers (or freaky code)
 *
 * Revision 1.48.2.1  2006/06/05 23:59:33  taylor
 * don't Int3() here, it's actually going to happen in some cases when using -img2dds
 *
 * Revision 1.48  2006/05/13 07:29:52  taylor
 * OpenGL envmap support
 * newer OpenGL extension support
 * add GL_ARB_texture_rectangle support for non-power-of-2 textures as interface graphics
 * add cubemap reading and writing support to DDS loader
 * fix bug in DDS loader that made compressed images with mipmaps use more memory than they really required
 * add support for a default envmap named "cubemap.dds"
 * new mission flag "$Environment Map:" to use a pre-existing envmap
 * minor cleanup of compiler warning messages
 * get rid of wasteful math from gr_set_proj_matrix()
 * remove extra gr_set_*_matrix() calls from starfield.cpp as there was no longer a reason for them to be there
 * clean up bmpman flags in reguards to cubemaps and render targets
 * disable D3D envmap code until it can be upgraded to current level of code
 * remove bumpmap code from OpenGL stuff (sorry but it was getting in the way, if it was more than copy-paste it would be worth keeping)
 * replace gluPerspective() call with glFrustum() call, it's a lot less math this way and saves the extra function call
 *
 * Revision 1.47  2006/04/12 01:10:35  taylor
 * some cleanup and slight reorg
 *  - remove special uv offsets for non-standard res, they were stupid anyway and don't actually fix the problem (which should actually be fixed now)
 *  - avoid some costly math where possible in the drawing functions
 *  - add opengl_error_string(), this is part of a later update but there wasn't a reason to not go ahead and commit this peice now
 *  - minor cleanup to Win32 extension defines
 *  - make opengl_lights[] allocate only when using OGL
 *  - cleanup some costly per-frame lighting stuff
 *  - clamp textures for interface and aabitmap (font) graphics since they shouldn't normally repeat anyway (the default)
 *    (doing this for D3D, if it doesn't already, may fix the blue-lines problem since a similar issue was seen with OGL)
 *
 * Revision 1.46  2006/04/06 23:23:56  taylor
 * bits of cleanup
 * minor fixes for -img2dds
 *
 * Revision 1.45  2006/03/22 18:14:52  taylor
 * if -mipmap is used with -img2dds to then have compressed image also contain mipmaps
 * use nicest hints for texture compression, should improve quality a little
 * when reporting compressed sizes to debug log make ani size be total, not per frame
 *
 * Revision 1.44  2006/03/15 17:33:05  taylor
 * couple of nitpicks
 *
 * Revision 1.43  2006/03/12 07:34:39  taylor
 * when I forget that I'm an idiot little things like this slip through (me == moron)
 *
 * Revision 1.42  2006/01/30 06:52:15  taylor
 * better lighting for OpenGL
 * remove some extra stuff that was from sectional bitmaps since we don't need it anymore
 *
 * Revision 1.41  2006/01/14 06:23:39  taylor
 * be sure to account for skipped mipmap levels in calculation of textures in VRAM
 *
 * Revision 1.40  2006/01/12 17:42:56  wmcoolmon
 * Even more scripting stuff.
 *
 * Revision 1.39  2006/01/03 22:46:52  taylor
 * oops, should have been checking my logs on that one, fixes minor GL error
 *
 * Revision 1.38  2006/01/03 02:59:14  taylor
 * fix a couple of minor mipmap problems
 * add resizing with mipmaps rather than a physical change in memory when detail levels dictate scaling (maybe this won't blow up)
 *
 * Revision 1.37  2005/12/29 00:00:18  taylor
 * handle case where we make an atexit (on failure) flush call before the texture system is initted
 *
 * Revision 1.36  2005/12/28 22:28:44  taylor
 * add support for glCompressedTexSubImage2D(), we don't use it yet but there is nothing wrong with adding it already
 * better support for mipmaps and mipmap filtering
 * add reg option "TextureFilter" to set bilinear or trilinear filter
 * clean up bitmap_id/bitmap_handle/texture_handle madness that made things difficult to understand
 * small fix for using 24-bit images on 16-bit bpp visual (untested)
 *
 * Revision 1.35  2005/12/16 06:48:28  taylor
 * "House Keeping!!"
 *   - minor cleanup of things that have bothered me at one time or another
 *   - slight speedup from state switching
 *   - slightly better specmap handling, fixes a couple of (not frequent) strange and sorta random issues
 *   - make sure to only disable HTL arb stuff when in HTL mode
 *   - handle any extra lighting pass before spec pass so the light can be applied properly
 *
 * Revision 1.34  2005/12/15 16:27:45  taylor
 * make sure we always disable all VBO related elements to avoid crashing (new that was a problem, did it anyway, me == stupid)
 *
 * Revision 1.33  2005/12/08 15:10:07  taylor
 * add APPLE_client_storage support to improve texture performance and reduce memory usage a tiny bit on OS X
 *
 * Revision 1.32  2005/12/07 12:40:51  taylor
 * fix last bit of recent spec issue
 *
 * Revision 1.31  2005/12/06 02:50:41  taylor
 * clean up some init stuff and fix a minor SDL annoyance
 * make debug messages a bit more readable
 * clean up the debug console commands for minimize and anisotropic filter setting
 * make anisotropic filter actually work correctly and have it settable with a reg option
 * give opengl_set_arb() the ability to disable all features on all arbs at once so I don't have to everywhere
 *
 * Revision 1.30  2005/11/16 07:45:35  taylor
 * umm, no one saw that, right? :)
 *
 * Revision 1.29  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 1.28  2005/10/23 20:34:30  taylor
 * some cleanup, fix some general memory leaks, safety stuff and whatever else Valgrind complained about
 *
 * Revision 1.27  2005/10/23 19:07:18  taylor
 * make AABITMAP use GL_ALPHA rather than GL_LUMINANCE_ALPHA (now 8-bit instead of 16-bit, fixes several minor rendering issues)
 *
 * Revision 1.26  2005/09/20 02:46:52  taylor
 * slight speedup for font rendering
 * fix a couple of things that Valgrind complained about
 *
 * Revision 1.25  2005/09/05 09:36:41  taylor
 * merge of OSX tree
 * fix OGL fullscreen switch for SDL since the old way only worked under Linux and not OSX or Windows
 * fix OGL version check, it would allow a required major version to be higher if the required minor version was lower than current
 *
 * Revision 1.24  2005/06/19 02:37:02  taylor
 * general cleanup, remove some old code
 * speed up gr_opengl_flip() just a tad
 * inverted gamma slider fix that Sticks made to D3D
 * possible fix for ATI green screens
 * move opengl_check_for_errors() out of gropentnl so we can use it everywhere
 * fix logged OGL info from debug builds to be a little more readable
 * if an extension is found but required function is not then fail
 * try to optimize glDrawRangeElements so we are not rendering more than the card is optimized for
 * some 2d matrix usage checks
 *
 * Revision 1.23  2005/06/03 06:44:17  taylor
 * cleanup of gr_bitmap since it's the same for D3D and OGL, skip if GR_STUB though
 * fix resize/rendering issue when detail culling is used with optimized opengl_create_texture_sub()
 *
 * Revision 1.22  2005/06/01 09:37:44  taylor
 * little cleanup
 * optimize opengl_create_texture_sub() by not doing any extra image processing if we don't have to
 *   if an image is already power-of-2 or we don't need power-of-2 then avoid the extra time and memory usage
 *
 * Revision 1.21  2005/05/12 17:42:13  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 * set byte_mult to 2 for TCACHE_TYPE_AABITMAP to get the memory size right for Debug builds
 *
 * Revision 1.20  2005/04/24 12:56:42  taylor
 * really are too many changes here:
 *  - remove all bitmap section support and fix problems with previous attempt
 *  ( code/bmpman/bmpman.cpp, code/bmpman/bmpman.h, code/globalincs/pstypes.h,
 *    code/graphics/2d.cpp, code/graphics/2d.h code/graphics/grd3dbmpman.cpp,
 *    code/graphics/grd3dinternal.h, code/graphics/grd3drender.cpp, code/graphics/grd3dtexture.cpp,
 *    code/graphics/grinternal.h, code/graphics/gropengl.cpp, code/graphics/gropengl.h,
 *    code/graphics/gropengllight.cpp, code/graphics/gropengltexture.cpp, code/graphics/gropengltexture.h,
 *    code/graphics/tmapper.h, code/network/multi_pinfo.cpp, code/radar/radarorb.cpp
 *    code/render/3ddraw.cpp )
 *  - use CLAMP() define in gropengl.h for gropengllight instead of single clamp() function
 *  - remove some old/outdated code from gropengl.cpp and gropengltexture.cpp
 *
 * Revision 1.19  2005/04/23 01:17:09  wmcoolmon
 * Removed GL_SECTIONS
 *
 * Revision 1.18  2005/04/21 15:50:47  taylor
 * we can't resize compressed textures so... don't, fixes rendering problems when not at max texture detail
 *
 * Revision 1.17  2005/03/10 08:00:05  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 1.16  2005/01/30 09:27:40  Goober5000
 * nitpicked some boolean tests, and fixed two small bugs
 * --Goober5000
 *
 * Revision 1.15  2005/01/21 08:54:53  taylor
 * slightly better memory management
 *
 * Revision 1.14  2005/01/21 08:25:14  taylor
 * fill in gr_opengl_set_texture_addressing()
 * add support for non-power-of-two textures for cards that have it
 * temporary crash fix from multiple mipmap levels in uncompressed formats
 *
 * Revision 1.13  2005/01/01 11:24:23  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 1.12  2004/12/05 01:28:39  taylor
 * support uncompressed DDS images
 * use TexSubImage2D for video anis
 *
 * Revision 1.11  2004/12/02 11:14:29  taylor
 * not used at the moment but that code was stupid
 *
 * Revision 1.10  2004/11/29 18:02:01  taylor
 * make sure we are checking the right bitmap_id
 *
 * Revision 1.9  2004/10/31 21:46:10  taylor
 * Linux tree merge, better DDS support, texture panning
 *
 * Revision 1.8  2004/09/05 19:23:24  Goober5000
 * fixed a few warnings
 * --Goober5000
 *
 * Revision 1.7  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 1.6  2004/07/21 00:03:46  taylor
 * fix problem with sectioned menu bitmaps in OGL
 *
 * Revision 1.5  2004/07/17 18:49:57  taylor
 * oops, I can't spell
 *
 * Revision 1.4  2004/07/17 18:43:46  taylor
 * don't use bitmap sections by default, openil_close()
 *
 * Revision 1.3  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.2  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
 *
 * Revision 2.5  2004/05/06 22:26:52  taylor
 * fix DDS textures
 *
 * Revision 2.4  2004/04/26 12:43:58  taylor
 * minor fixes, HTL lines, 32-bit texture support
 *
 * Revision 2.3  2004/04/13 01:55:41  phreak
 * put in the correct fields for the CVS comments to register
 * fixed a glowmap problem that occured when rendering glowmapped and non-glowmapped ships
 *
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <vector>

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "osapi/osregistry.h"
#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "graphics/gropengl.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropenglstate.h"
#include "graphics/grinternal.h"
#include "ddsutils/ddsutils.h"
#include "math/vecmat.h"


static tcache_slot_opengl *Textures = NULL;
static int *Tex_used_this_frame = NULL;

int GL_texture_ram = 0;
int GL_min_texture_width = 0;
GLint GL_max_texture_width = 0;
int GL_min_texture_height = 0;
GLint GL_max_texture_height = 0;
int GL_textures_in = 0;
int GL_textures_in_frame = 0;
int GL_last_detail = -1;
GLint GL_supported_texture_units = 2;
int GL_should_preload = 0;
ubyte GL_xlat[256];
GLfloat GL_anisotropy = 1.0f;
GLfloat GL_max_anisotropy = 2.0f;
int GL_mipmap_filter = 0;
GLenum GL_texture_target = GL_TEXTURE_2D;
GLenum GL_texture_face = GL_TEXTURE_2D;
GLenum GL_texture_addressing = GL_REPEAT;
bool GL_rendering_to_framebuffer = false;
GLint GL_max_renderbuffer_size = 0;

extern int GLOWMAP;
extern int SPECMAP;
extern int CLOAKMAP;
extern int ENVMAP;
extern int Interp_multitex_cloakmap;


// forward declarations
int opengl_free_texture(tcache_slot_opengl *t);
void opengl_free_texture_with_handle(int handle);
void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out);
int opengl_create_texture_sub(int bitmap_handle, int bitmap_type, int bmap_w, int bmap_h, int tex_w, int tex_h, ubyte *data = NULL, tcache_slot_opengl *t = NULL, int base_level = 0, int resize = 0, int reload = 0);
int opengl_create_texture (int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot = NULL);

void opengl_set_additive_tex_env()
{
	GL_CHECK_FOR_ERRORS("start of set_additive_tex_env()");

	if ( Is_Extension_Enabled(OGL_ARB_TEXTURE_ENV_COMBINE) ) {
		GL_state.Texture.SetEnvCombineMode(GL_COMBINE_RGB, GL_ADD);
		GL_state.Texture.SetRGBScale(1.0f);
	} else {
		GL_state.Texture.SetEnvMode(GL_ADD);
	}

	GL_CHECK_FOR_ERRORS("end of set_additive_tex_env()");
}

void opengl_set_modulate_tex_env()
{
	GL_CHECK_FOR_ERRORS("start of set_modulate_tex_env()");

	if ( Is_Extension_Enabled(OGL_ARB_TEXTURE_ENV_COMBINE) ) {
		GL_state.Texture.SetEnvCombineMode(GL_COMBINE_RGB, GL_MODULATE);
		GL_state.Texture.SetRGBScale(4.0f);
	} else {
		GL_state.Texture.SetEnvMode(GL_MODULATE);
	}

	GL_CHECK_FOR_ERRORS("end of set_modulate_tex_env()");
}

GLfloat opengl_get_max_anisotropy()
{
	if ( !Is_Extension_Enabled(OGL_EXT_TEXTURE_FILTER_ANISOTROPIC) ) {
		return 0.0f;
	}

	if ( !GL_max_anisotropy ) {
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &GL_max_anisotropy);
	}

	// the spec says that it should be a minimum of 2.0
	Assert( GL_max_anisotropy >= 2.0f );

	return GL_max_anisotropy;
}

void opengl_set_texture_target( GLenum target )
{
    GL_texture_target = target;
}

void opengl_tcache_init()
{
	int i;

	GL_should_preload = 1;

	GL_min_texture_width = 16;
	GL_min_texture_height = 16;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GL_max_texture_width);

	GL_max_texture_height = GL_max_texture_width;

	// if we are not using sections then make sure we have the min texture size available to us
	// 1024 is what we need with the standard resolutions - taylor
	if (GL_max_texture_width < 1024) {
		Error(LOCATION, "A minimum texture size of \"1024x1024\" is required for FS2_Open but only \"%ix%i\" was found.  Can not continue.", GL_max_texture_width, GL_max_texture_height);
	}

	if (Textures == NULL) {
		Textures = (tcache_slot_opengl *) vm_malloc_q(MAX_BITMAPS * sizeof(tcache_slot_opengl));
	}

	if (Tex_used_this_frame == NULL) {
		Tex_used_this_frame = (int *) vm_malloc_q(MAX_BITMAPS * sizeof(int));
	}

	if ( !Textures || !Tex_used_this_frame )
		Error(LOCATION, "Unable to allocate memory for OpenGL texture slots!");


	memset( Textures, 0, MAX_BITMAPS * sizeof(tcache_slot_opengl) );
	memset( Tex_used_this_frame, 0, MAX_BITMAPS * sizeof(int) );

	// Init the texture structures
	for (i = 0; i < MAX_BITMAPS; i++) {
		Textures[i].bitmap_handle = -1;
	}

	// check what mipmap filter we should be using
	//   0  ==  Bilinear
	//   1  ==  Trilinear
	GL_mipmap_filter = os_config_read_uint(NULL, "TextureFilter", 1);

	if (GL_mipmap_filter > 1) {
		GL_mipmap_filter = 1;
	}

	// max size (width and/or height) that we can use for framebuffer/renderbuffer
	if ( Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) ) {
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &GL_max_renderbuffer_size);

		// if we can't do at least 128x128 then just disable FBOs
		if (GL_max_renderbuffer_size < 128) {
			mprintf(("WARNING: Max dimensions of FBO, %ix%i, is less the required minimum!!  Extension will be disabled!\n", GL_max_renderbuffer_size, GL_max_renderbuffer_size));
			GL_Extensions[OGL_EXT_FRAMEBUFFER_OBJECT].enabled = 0;
		}
	}

	// anisotropy
	if ( Is_Extension_Enabled(OGL_EXT_TEXTURE_FILTER_ANISOTROPIC) ) {
		// set max value first thing
		opengl_get_max_anisotropy();

		// now for the user setting
		char *plevel = os_config_read_string( NULL, NOX("OGL_AnisotropicFilter"), NOX("0.0") );
		GL_anisotropy = (GLfloat) strtod(plevel, (char**)NULL);

		CLAMP(GL_anisotropy, 0.0f, GL_max_anisotropy);
	}

	// set the alpha gamma settings (for fonts)
	memset( GL_xlat, 0, sizeof(GL_xlat) );

	for (i = 1; i < 15; i++) {
		GL_xlat[i] = (ubyte)(GL_xlat[i-1] + 17);
	}

	GL_xlat[15] = GL_xlat[1];

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &GL_supported_texture_units);
	Assert( GL_supported_texture_units >= 2 );

	GL_last_detail = Detail.hardware_textures;

	GL_textures_in = 0;
	GL_textures_in_frame = 0;
}

void opengl_free_texture_with_handle(int handle)
{
	int n = bm_get_cache_slot(handle, 1);

	Tex_used_this_frame[n] = 0;
	opengl_free_texture( &Textures[n] );
}

void opengl_tcache_flush()
{
	int i;

	if ( Textures == NULL )
		return;

	for (i = 0; i < MAX_BITMAPS; i++)
		opengl_free_texture( &Textures[i] );

	if (GL_textures_in != 0) {
		mprintf(( "WARNING: VRAM is at %d instead of zero after flushing!\n", GL_textures_in ));
		GL_textures_in = 0;
	}
}

extern void opengl_kill_all_render_targets();

void opengl_tcache_shutdown()
{
	opengl_kill_all_render_targets();

	opengl_tcache_flush();

	GL_textures_in = 0;
	GL_textures_in_frame = 0;

	if (Textures != NULL) {
		vm_free(Textures);
		Textures = NULL;
	}

	if (Tex_used_this_frame != NULL) {
		vm_free(Tex_used_this_frame);
		Tex_used_this_frame = NULL;
	}
}

void opengl_tcache_frame()
{
	GL_textures_in_frame = 0;

	// make all textures as not used
	memset( Tex_used_this_frame, 0, MAX_BITMAPS * sizeof(int) );
}

extern bool GL_initted;

void opengl_free_texture_slot( int n )
{
	if ( !GL_initted ) {
		return;
	}

	Tex_used_this_frame[n] = 0;
	opengl_free_texture( &Textures[n] );
}

// determine if a bitmap is in API memory, so that we can just reuse it rather
// that having to load it from disk again
bool opengl_texture_slot_valid(int n, int handle)
{
	tcache_slot_opengl *t = &Textures[n];

	if (t->bitmap_handle < 0) {
		return false;
	}

	if (t->bitmap_handle != handle) {
		return false;
	}

	if ( !glIsTexture(t->texture_id) ) {
		return false;
	}

	return true;
}

int opengl_free_texture(tcache_slot_opengl *t)
{
	// Bitmap changed!!     
	if (/*t->bitmap_handle > -1*/t->texture_id) {
		// if I, or any of my children have been used this frame, bail  
		// can't use bm_get_cache_slot() here since bitmap_id probably isn't valid
		if ( (t->bitmap_handle >= 0) && Tex_used_this_frame[t->bitmap_handle % MAX_BITMAPS] /*&& (t->bpp == 8)*/ ) {
			return 0;
		}

		// ok, now we know its legal to free everything safely
		GL_state.Texture.Delete(t->texture_id);
		glDeleteTextures (1, &t->texture_id);

		GL_textures_in -= t->size;
		memset( t, 0, sizeof(tcache_slot_opengl) );
		t->bitmap_handle = -1;
	}

	return 1;
}

void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	int i, tex_w, tex_h;

	// bogus
	if ( (w_out == NULL) ||  (h_out == NULL) ) {
		return;
	}

	// if we can support non-power-of-2 textures then just return current sizes - taylor
	if ( Is_Extension_Enabled(OGL_ARB_TEXTURE_NON_POWER_OF_TWO) || (GL_texture_target == GL_TEXTURE_RECTANGLE_ARB) ) {
		*w_out = w_in;
		*h_out = h_in;

		return;
	}

	// starting size
	tex_w = w_in;
	tex_h = h_in;

	for (i = 0; i < 16; i++) {
		if ( (tex_w > (1<<i)) && (tex_w <= (1<<(i+1))) ) {
			tex_w = 1 << (i+1);
			break;
		}
	}

	for (i = 0; i < 16; i++) {
		if ( (tex_h > (1<<i)) && (tex_h <= (1<<(i+1))) ) {
			tex_h = 1 << (i+1);
			break;
		}
	}

	CLAMP(tex_w, GL_min_texture_width, GL_max_texture_width);
	CLAMP(tex_h, GL_min_texture_height, GL_max_texture_height);

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

// data == start of bitmap data
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int opengl_create_texture_sub(int bitmap_handle, int bitmap_type, int bmap_w, int bmap_h, int tex_w, int tex_h, ubyte *data, tcache_slot_opengl *t, int base_level, int resize, int reload)
{
	int ret_val = 1;
	int byte_mult = 0;
	GLenum texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
	GLenum glFormat = GL_BGRA;
	GLint intFormat = GL_RGBA;
	ubyte saved_bpp = 0;
	int mipmap_w = 0, mipmap_h = 0;
	int dsize = 0, doffset = 0, block_size = 0;
	int i,j,k;
	ubyte *bmp_data = data;
	ubyte *texmem = NULL, *texmemp = NULL;
	int skip_size = 0, mipmap_levels = 0;

	GL_CHECK_FOR_ERRORS("start of create_texture_sub()");

	// bogus
	if ( (t == NULL) || (bmp_data == NULL) ) {
		return 0;
	}

	int idx = bm_get_cache_slot( bitmap_handle, 1 );

	if ( Tex_used_this_frame[idx] ) {
		mprintf(( "ARGHH!!! Texture already used this frame!  Cannot free it!\n" ));
		return 0;
	}

	if ( !reload )  {
		// save the bpp since it will get reset - fixes anis being 0 bpp
		saved_bpp = t->bpp;

		// gah
		if ( !opengl_free_texture(t) ) {
			return 0;
		}
	}

	// for everything that might use mipmaps
	mipmap_w = tex_w;
	mipmap_h = tex_h;

	if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_INTERFACE) ) {
		// when using GL_TEXTURE_RECTANGLE_ARB the uv coords are in the [0..w]/[0..h] range instead
		// of the normal [0..1]/[0..1] range.  to make it simple we just use the tex width and height
		// as the scale factor since this should only be used on things where the UV coords are
		// already either 0 or 1.
		if (GL_state.Texture.GetTarget() == GL_TEXTURE_RECTANGLE_ARB) {
			t->u_scale = (float)tex_w;
			t->v_scale = (float)tex_h;
		} else {
			t->u_scale = (float)bmap_w / (float)tex_w;
			t->v_scale = (float)bmap_h / (float)tex_h;
		}
	} else {
		t->u_scale = 1.0f;
		t->v_scale = 1.0f;
	}

	if ( !reload ) {
		glGenTextures (1, &t->texture_id);
	}
	
	if (t->texture_id == 0) {
		mprintf(("!!OpenGL DEBUG!! t->texture_id == 0\n"));
		return 0;
	}
	
	if (t->bpp == 0) {
		// it got reset, revert to saved setting
		t->bpp = saved_bpp;
	}

	// set the byte per pixel multiplier
	byte_mult = (t->bpp >> 3);

	// GL_BGRA_EXT is *much* faster with some hardware/drivers
	if (byte_mult == 4) {
		texFormat = GL_UNSIGNED_INT_8_8_8_8_REV;
		intFormat = (gr_screen.bits_per_pixel == 32) ? GL_RGBA8 : GL_RGB5_A1;
		glFormat = GL_BGRA;
	} else if (byte_mult == 3) {
		texFormat = GL_UNSIGNED_BYTE;
		intFormat = (gr_screen.bits_per_pixel == 32) ? GL_RGB8 : GL_RGB5;
		glFormat = GL_BGR;
	} else if (byte_mult == 2) {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGB5_A1;
		glFormat = GL_BGRA;
	} else if (byte_mult == 1) {
		Assert( bitmap_type == TCACHE_TYPE_AABITMAP );
		texFormat = GL_UNSIGNED_BYTE;
		intFormat = GL_ALPHA;
		glFormat = GL_ALPHA;
	} else {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGBA;
		glFormat = GL_BGRA;
	}

	// check for compressed image types
	switch ( bm_is_compressed(bitmap_handle) ) {
		case DDS_DXT1:
		case DDS_CUBEMAP_DXT1:
			intFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			block_size = 8;
			break;

		case DDS_DXT3:
		case DDS_CUBEMAP_DXT3:
			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			block_size = 16;
			break;

		case DDS_DXT5:
		case DDS_CUBEMAP_DXT5:
			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			block_size = 16;
			break;
	}


	GL_state.Texture.Enable(t->texture_id);

	mipmap_levels = bm_get_num_mipmaps(bitmap_handle);

	switch (bitmap_type) {
		case TCACHE_TYPE_COMPRESSED: {
			if (block_size > 0) {
				// size of data block (4x4)
				dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

				// if we are skipping mipmap levels in order to resize then we have to calc the new offset
				for (i = 0; i < base_level; i++) {
					doffset += dsize;

					mipmap_w >>= 1;
					mipmap_h >>= 1;

					if (mipmap_w <= 0) {
						mipmap_w = 1;
					}

					if (mipmap_h <= 0) {
						mipmap_h = 1;
					}

					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;
				}

				skip_size = doffset;

				if ( !reload ) {
					vglCompressedTexImage2D(GL_TEXTURE_2D, 0, intFormat, mipmap_w, mipmap_h, 0, dsize, bmp_data + doffset);
				} else {
					vglCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);
				}

				// now that the base image is done handle any mipmap levels
				for (i = 1; i < (mipmap_levels - base_level); i++) {
					// adjust the data offset for the next block
					doffset += dsize;

					// reduce size by half for the next pass
					mipmap_w >>= 1;
					mipmap_h >>= 1;

					if (mipmap_w <= 0) {
						mipmap_w = 1;
					}

					if (mipmap_h <= 0) {
						mipmap_h = 1;
					}

					// size of data block (4x4)
					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

					if ( !reload ) {
						vglCompressedTexImage2D(GL_TEXTURE_2D, i, intFormat, mipmap_w, mipmap_h, 0, dsize, bmp_data + doffset);
					} else {
						vglCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);
					}
				}
			} else {
				Int3();
				return 0;
			}

			break;
		}

		case TCACHE_TYPE_AABITMAP: {
			texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
			texmemp = texmem;

			Assert( texmem != NULL );

			for (i = 0; i < tex_h; i++) {
				for (j = 0; j < tex_w; j++) {
					if ( (i < bmap_h) && (j < bmap_w) ) {
						*texmemp++ = GL_xlat[bmp_data[i*bmap_w+j]];
					} else {
						*texmemp++ = 0;
					}
				}
			}

			if ( !reload || (GL_state.Texture.GetTarget() == GL_TEXTURE_RECTANGLE_ARB) ) {
				glTexImage2D (GL_state.Texture.GetTarget(), 0, GL_ALPHA, tex_w, tex_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texmem);
			} else { // faster anis
				glTexSubImage2D (GL_state.Texture.GetTarget(), 0, 0, 0, tex_w, tex_h, GL_ALPHA, GL_UNSIGNED_BYTE, texmem);
			}

			if (texmem != NULL) {
				vm_free(texmem);
			}

			break;
		}

		case TCACHE_TYPE_INTERFACE: {
			// if we aren't resizing then we can just use bmp_data directly
			if (resize) {
				texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
				texmemp = texmem;

				Assert( texmem != NULL );

				for (i = 0; i < tex_h; i++) {
					for (j = 0;j < tex_w; j++) {
						if ( (i < bmap_h) && (j < bmap_w) ) {
							for (k = 0; k < byte_mult; k++) {
								*texmemp++ = bmp_data[(i*bmap_w+j)*byte_mult+k];
							}
						} else {
							for (k = 0; k < byte_mult; k++) {
								*texmemp++ = 0;
							}
						}
					}
				}
			}

			if ( !reload || (GL_state.Texture.GetTarget() == GL_TEXTURE_RECTANGLE_ARB) ) {
				glTexImage2D (GL_state.Texture.GetTarget(), 0, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, (resize) ? texmem : bmp_data);
			} else { // faster anis
				glTexSubImage2D (GL_state.Texture.GetTarget(), 0, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, (resize) ? texmem : bmp_data);
			}

			if (texmem != NULL) {
				vm_free(texmem);
			}

			break;
		}

		case TCACHE_TYPE_CUBEMAP: {
			Assert( !resize );
			Assert( texmem == NULL );
			Assert( Is_Extension_Enabled(OGL_ARB_TEXTURE_CUBE_MAP) );

			// we have to load in all 6 faces...
			for (i = 0; i < 6; i++) {
				doffset += dsize;

				// check if it's a compressed cubemap first
				if (block_size > 0) {
					// size of data block (4x4)
					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

					if ( !reload ) {
						vglCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, intFormat, mipmap_w, mipmap_h, 0, dsize, bmp_data + doffset);
					} else {
						vglCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);
					}

					// now that the base image is done handle any mipmap levels
					for (j = 1; j < mipmap_levels; j++) {
						// adjust the data offset for the next block
						doffset += dsize;

						// reduce size by half for the next pass
						mipmap_w >>= 1;
						mipmap_h >>= 1;

						if (mipmap_w <= 0) {
							mipmap_w = 1;
						}

						if (mipmap_h <= 0) {
							mipmap_h = 1;
						}

						// size of data block (4x4)
						dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

						if ( !reload ) {
							vglCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, j, intFormat, mipmap_w, mipmap_h, 0, dsize, bmp_data + doffset);
						} else {
							vglCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, j, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);
						}
					}
				}
				// nope, it's uncompressed...
				else {
					dsize = mipmap_h * mipmap_w * byte_mult;

					if ( !reload ) {
						glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, bmp_data + doffset);
					} else { // faster anis
						glTexSubImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, bmp_data + doffset);
					}

					// base image is done so now take care of any mipmap levels
					for (j = 1; j < mipmap_levels; j++) {
						doffset += dsize;
						mipmap_w >>= 1;
						mipmap_h >>= 1;

						if (mipmap_w <= 0) {
							mipmap_w = 1;
						}

						if (mipmap_h <= 0) {
							mipmap_h = 1;
						}

						dsize = mipmap_h * mipmap_w * byte_mult;

						if ( !reload ) {
							glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, j, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, bmp_data + doffset);
						} else {
							glTexSubImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, j, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, bmp_data + doffset);
						}
					}
				}

				// reset width and height for next face
				mipmap_w = tex_w;
				mipmap_h = tex_h;
			}

			break;
		}

		default: {
			// if we aren't resizing then we can just use bmp_data directly
			if (resize) {
				texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
				texmemp = texmem;

				Assert( texmem != NULL );
	
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( (bmap_w-1)*F1_0 ) / tex_w;
				dv = ( (bmap_h-1)*F1_0 ) / tex_h;

				for (j=0;j<tex_h;j++)
				{
					utmp = u;
					for (i=0;i<tex_w;i++)
					{
						for (k = 0; k < byte_mult; k++) {
							*texmemp++ = bmp_data[(f2i(v)*bmap_w+f2i(utmp))*byte_mult+k];
						}
						utmp += du;
					}
					v += dv;
				}
			}

			// should never have mipmap levels if we also have to manually resize
			if ( (mipmap_levels > 1) && resize ) {
				Assert( texmem == NULL );
			}

			dsize = mipmap_h * mipmap_w * byte_mult;

			// if we are skipping mipmap levels in order to resize then we have to calc the new offset
			for (i = 0; i < base_level; i++) {
				doffset += dsize;

				mipmap_w >>= 1;
				mipmap_h >>= 1;

				if (mipmap_w <= 0) {
					mipmap_w = 1;
				}

				if (mipmap_h <= 0) {
					mipmap_h = 1;
				}

				dsize = mipmap_h * mipmap_w * byte_mult;
			}

			skip_size = doffset;

			if ( !reload ) {
				glTexImage2D (GL_TEXTURE_2D, 0, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, (texmem != NULL) ? texmem : bmp_data + doffset);
			} else { // faster anis
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, (texmem != NULL) ? texmem : bmp_data + doffset);
			}


			// base image is done so now take care of any mipmap levels
			for (i = 1; i < (mipmap_levels - base_level); i++) {
				doffset += dsize;
				mipmap_w >>= 1;
				mipmap_h >>= 1;

				if (mipmap_w <= 0) {
					mipmap_w = 1;
				}

				if (mipmap_h <= 0) {
					mipmap_h = 1;
				}

				dsize = mipmap_h * mipmap_w * byte_mult;

				if ( !reload ) {
					glTexImage2D (GL_TEXTURE_2D, i, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, bmp_data + doffset);
				} else {
					glTexSubImage2D (GL_TEXTURE_2D, i, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, bmp_data + doffset);
				}
			}

			if (texmem != NULL) {
				vm_free(texmem);
			}

			break;
		}
	}//end switch


	t->bitmap_handle = bitmap_handle;
	t->size = (dsize) ? ((doffset + dsize) - skip_size) : (tex_w * tex_h * byte_mult);
	t->w = (ushort)tex_w;
	t->h = (ushort)tex_h;

	GL_textures_in_frame += t->size;

	if ( !reload ) {
		GL_textures_in += t->size;
	}

	Tex_used_this_frame[idx] = 0;

	GL_CHECK_FOR_ERRORS("end of create_texture_sub()");

	return ret_val;
}

int opengl_create_texture(int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	ubyte bpp = 16;
	int reload = 0, resize = 0, base_level = 0;
	int max_levels = 0;

	GL_CHECK_FOR_ERRORS("start of create_texture()");

	if (tslot == NULL) {
		return 0;
	}

	// setup texture/bitmap flags
	flags = 0;
	switch (bitmap_type) {
		case TCACHE_TYPE_AABITMAP:
			flags |= BMP_AABITMAP;
			bpp = 8;
			break;

		case TCACHE_TYPE_CUBEMAP:
		case TCACHE_TYPE_NORMAL:
			flags |= BMP_TEX_OTHER;
			break;

		case TCACHE_TYPE_INTERFACE:
		case TCACHE_TYPE_XPARENT:
			flags |= BMP_TEX_XPARENT;
			break;

		case TCACHE_TYPE_COMPRESSED:
			switch ( bm_is_compressed(bitmap_handle) ) {
				case DDS_DXT1:				//dxt1
					bpp = 24;
					flags |= BMP_TEX_DXT1;
					break;

				case DDS_DXT3:				//dxt3
					bpp = 32;
					flags |= BMP_TEX_DXT3;
					break;

				case DDS_DXT5:				//dxt5
					bpp = 32;
					flags |= BMP_TEX_DXT5;
					break;

				case DDS_CUBEMAP_DXT1:
					bpp = 24;
					flags |= BMP_TEX_CUBEMAP;
					break;

				case DDS_CUBEMAP_DXT3:
				case DDS_CUBEMAP_DXT5:
					bpp = 32;
					flags |= BMP_TEX_CUBEMAP;
					break;

				default:
					Assert( 0 );
					break;
			}

			break;
	}

	// lock the bitmap into the proper format
	bmp = bm_lock(bitmap_handle, bpp, flags);

	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}

	int max_w = bmp->w;
	int max_h = bmp->h;

	max_levels = bm_get_num_mipmaps(bitmap_handle);

	// there should only ever be one mipmap level for interface graphics!!!
	if ( (bitmap_type == TCACHE_TYPE_INTERFACE) && (max_levels > 1) ) {
		max_levels = 1;
	}

	// if we ended up locking a texture that wasn't originally compressed then this should catch it
	if ( !(bitmap_type == TCACHE_TYPE_CUBEMAP) && bm_is_compressed(bitmap_handle) ) {
		bitmap_type = TCACHE_TYPE_COMPRESSED;
	}

	if ( (Detail.hardware_textures < 4) && (bitmap_type != TCACHE_TYPE_AABITMAP) && (bitmap_type != TCACHE_TYPE_INTERFACE)
			&& ((bitmap_type != TCACHE_TYPE_COMPRESSED) || (bitmap_type == TCACHE_TYPE_COMPRESSED) && (max_levels > 1)) )
	{
		if (max_levels == 1) {
			// if we are going to cull the size then we need to force a resize
			// Detail.debris_culling goes from 0 to 4.
			max_w /= (16 >> Detail.hardware_textures);
			max_h /= (16 >> Detail.hardware_textures);

			resize = 1;
		} else {
			// we have mipmap levels so use those as a resize point (image should already be power-of-2)
			base_level = -(Detail.hardware_textures - 4);
			Assert(base_level >= 0);

			if (base_level >= max_levels) {
				base_level = max_levels - 1;
			}

			Assert( (max_levels - base_level) >= 1 );
		}
	}

	// get final texture size
	opengl_tcache_get_adjusted_texture_size(max_w, max_h, &final_w, &final_h);

	// only resize if we actually need a new size, better data use and speed out of opengl_create_texture_sub()
	if ( (max_w != final_w) || (max_h != final_h) ) {
		resize = 1;
		// little safety check for later, we can't manually resize AND use mipmap resizing
		Assert( !base_level );
	}

	if ( (final_h < 1) || (final_w < 1) )       {
		mprintf(("Bitmap is to small at %dx%d.\n", final_w, final_h ));
		return 0;
	}

	// if this tcache slot has no bitmap
	if (tslot->bitmap_handle < 0) {
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_handle != bitmap_handle) {
		if ( (final_w == tslot->w) && (final_h == tslot->h) ) {
			reload = 1;
		} else {
			reload = 0;
		}
	}

	// set the bits per pixel
	tslot->bpp = bmp->bpp;

	// max number of mipmap levels (NOTE: this is the max number used by the API, not how many true mipmap levels there are in the image!!)
	tslot->mipmap_levels = (ubyte)(max_levels - base_level);

	// call the helper
	int ret_val = opengl_create_texture_sub(bitmap_handle, bitmap_type, bmp->w, bmp->h, final_w, final_h, (ubyte*)bmp->data, tslot, base_level, resize, reload);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	// if we successfully sent the texture to API memory then go ahead and dump
	// the rest of the bitmap data in system memory.
	// NOTE: this doesn't do anything for user bitmaps (like screen grabs or streamed animations)
	if (ret_val) {
		bm_unload_fast(bitmap_handle);
	}

	GL_CHECK_FOR_ERRORS("end of create_texture()");

	return ret_val;
}

// WARNING:  Needs to match what is in bm_internal.h!!!!!
#define RENDER_TARGET_DYNAMIC	17

int gr_opengl_tcache_set_internal(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, int tex_unit = 0)
{
	int ret_val = 1;

	GL_CHECK_FOR_ERRORS("start of tcache_set_internal()");

	if (GL_last_detail != Detail.hardware_textures) {
		GL_last_detail = Detail.hardware_textures;
		opengl_tcache_flush();
	}

	int n = bm_get_cache_slot (bitmap_handle, 1);
	tcache_slot_opengl *t = &Textures[n];

	// this is a little sloppy (should have been done before now), but it is actually easier and faster here
	if (t->texture_target > 0) {
		opengl_set_texture_target(t->texture_target);
	}

	GL_state.Texture.SetActiveUnit(tex_unit);
	GL_state.Texture.SetTarget(GL_texture_target);

	if ( /*(bm_is_render_target(bitmap_handle) != RENDER_TARGET_DYNAMIC) &&*/
		!bm_is_render_target(bitmap_handle) &&
		((t->bitmap_handle < 0) || (bitmap_handle != t->bitmap_handle)) )
	{
		ret_val = opengl_create_texture( bitmap_handle, bitmap_type, t );
	}

	// everything went ok
	if (ret_val && t->texture_id) {
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;

		GL_state.Texture.Enable(t->texture_id);

		GL_state.Texture.SetMagFilter(GL_LINEAR);

		// set the filter type again, just to make sure it's correct
		if (t->mipmap_levels > 1) {
			Assert( GL_state.Texture.GetTarget() != GL_TEXTURE_RECTANGLE_ARB );

			// OGL expects mipmap levels all the way down to 1x1 but I think this will avoid
			// white texture issues when we have fewer levels than that, it caps the total
			// number of levels available, with 0 as min value
			GL_state.Texture.SetMaxLevel(t->mipmap_levels-1);

			GL_state.Texture.SetMinFilter( (GL_mipmap_filter) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST );

			GL_state.Texture.AnisoFilter(GL_anisotropy);
		} else {
			GL_state.Texture.SetMaxLevel(0);
			GL_state.Texture.SetMinFilter(GL_LINEAR);

			if (GL_state.Texture.GetTarget() != GL_TEXTURE_RECTANGLE_ARB) {
				GL_state.Texture.AnisoFilter(1.0f);
			}
		}

		if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_INTERFACE) || (bitmap_type == TCACHE_TYPE_CUBEMAP) ) {
			GL_state.Texture.SetWrapS(GL_CLAMP_TO_EDGE);
			GL_state.Texture.SetWrapT(GL_CLAMP_TO_EDGE);

			if (bitmap_type == TCACHE_TYPE_CUBEMAP) {
				GL_state.Texture.SetWrapR(GL_CLAMP_TO_EDGE);
			}
		} else {
			GL_state.Texture.SetWrapS(GL_texture_addressing);
			GL_state.Texture.SetWrapT(GL_texture_addressing);
		}

		Tex_used_this_frame[n]++;
	}
	// gah
	else {
		Int3();
		GL_state.Texture.Disable();

		return 0;
	}

	GL_CHECK_FOR_ERRORS("end of tcache_set_internal()");

	return 1;
}

int gr_opengl_tcache_set(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, int stage)
{
	int rc = 0;

	if (bitmap_handle < 0) {
		return 0;
	}

	GL_CHECK_FOR_ERRORS("start of tcache_set()");

	// set special type if it's so, needed to be right later, but cubemaps are special
	if ( !(bitmap_type == TCACHE_TYPE_CUBEMAP) ) {
		int type = bm_get_tcache_type(bitmap_handle);

		if (type != TCACHE_TYPE_NORMAL) {
			bitmap_type = type;
		}
	}

	// if we are interface, we have ARB_texture_rectangle, and don't have ARB_non_power_of_two, then use texture_rectangle target
	if ( (bitmap_type == TCACHE_TYPE_INTERFACE) && Is_Extension_Enabled(OGL_ARB_TEXTURE_RECTANGLE)
			&& !Is_Extension_Enabled(OGL_ARB_TEXTURE_NON_POWER_OF_TWO) )
	{
		//opengl_set_texture_target( GL_TEXTURE_RECTANGLE_ARB );
	}

	if ( (bitmap_type == TCACHE_TYPE_CUBEMAP) ) {
		opengl_set_texture_target(GL_TEXTURE_CUBE_MAP);
	}

	rc = gr_opengl_tcache_set_internal(bitmap_handle, bitmap_type, u_scale, v_scale, stage);

	// reset texture target to default
	opengl_set_texture_target();

	GL_CHECK_FOR_ERRORS("end of tcache_set()");

	return rc;
}

void opengl_preload_init()
{
	if (gr_screen.mode != GR_OPENGL)
		return;

//	opengl_tcache_flush ();
}

int gr_opengl_preload(int bitmap_num, int is_aabitmap)
{
	float u_scale, v_scale;
	int retval;

	Assert( gr_screen.mode == GR_OPENGL );

	if ( !GL_should_preload ) {
		return 0;
	}

	retval = gr_opengl_tcache_set(bitmap_num, (is_aabitmap) ? TCACHE_TYPE_AABITMAP : TCACHE_TYPE_NORMAL, &u_scale, &v_scale);

	GL_state.Texture.Disable();

	if ( !retval ) {
		mprintf(("Texture upload failed!\n"));
	}

	return retval;
}

static int GL_texture_panning_enabled = 0;
void gr_opengl_set_texture_panning(float u, float v, bool enable)
{
	if (Cmdline_nohtl)
		return;

	GLint current_matrix;

	if (enable) {
		glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
		glMatrixMode( GL_TEXTURE );
		glPushMatrix();
		glTranslatef( u, v, 0.0f );
		glMatrixMode(current_matrix);

		GL_texture_panning_enabled = 1;
	} else if (GL_texture_panning_enabled) {
		glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
		glMatrixMode( GL_TEXTURE );
		glPopMatrix();
		glMatrixMode(current_matrix);
	
		GL_texture_panning_enabled = 0;
	}
}

void gr_opengl_set_texture_addressing(int mode)
{
	GL_CHECK_FOR_ERRORS("start of set_texture_addressing()");

	switch (mode) {
		case TMAP_ADDRESS_CLAMP:
			GL_texture_addressing = GL_CLAMP_TO_EDGE;
			break;

		case TMAP_ADDRESS_WRAP:
			GL_texture_addressing = GL_REPEAT;
			break;

		case TMAP_ADDRESS_MIRROR: {
			if ( Is_Extension_Enabled(OGL_ARB_TEXTURE_MIRRORED_REPEAT) ) {
				GL_texture_addressing = GL_MIRRORED_REPEAT_ARB;
			} else {
				GL_texture_addressing = GL_REPEAT;
			}

			break;
		}

		default:
			Int3();
			break;
	}

	GL_CHECK_FOR_ERRORS("end of set_texture_addressing()");
}

int opengl_compress_image( ubyte **compressed_data, ubyte *in_data, int width, int height, int alpha, int num_mipmaps )
{
	Assert( in_data != NULL );

	if ( !Texture_compression_available ) {
		return 0;
	}

	GL_CHECK_FOR_ERRORS("start of compress_image()");

	GLuint tex;
	GLint compressed = GL_FALSE;
	GLint compressed_size = 0;
	ubyte *out_data = NULL;
	GLint testing = 0;
	GLint intFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	GLenum texFormat = GL_UNSIGNED_BYTE;
	GLenum glFormat = GL_BGR;
	int i;

	if (alpha) {
		intFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		texFormat = GL_UNSIGNED_INT_8_8_8_8_REV;
		glFormat = GL_BGRA;
	}

	glGenTextures(1, &tex);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(tex);

	// a quick proxy test.  this will tell us if it's possible without wasting a lot of time and resources in the attempt
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, intFormat, width, height, 0, glFormat, texFormat, in_data);

	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &testing);

	if (compressed == GL_TRUE) {
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &compressed);
	}

	if ( (compressed == GL_FALSE) || (compressed != intFormat) || (testing == 0) ) {
		GL_state.Texture.Disable();
		GL_state.Texture.Delete(tex);
		glDeleteTextures(1, &tex);
		return 0;
	}

	// if mipmaps are requested then make them with the nicest possible settings
	if (num_mipmaps > 1) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
		glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
	}

	// use best compression quality
	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	// alright, it should work if we are still here, now do it for real
	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0, glFormat, texFormat, in_data);

	// if we got this far then it should have worked, but check anyway
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
	Assert( compressed != GL_FALSE );

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &compressed);
	Assert( compressed == intFormat );

	// for each mipmap level we generate go ahead and figure up the total memory required
	for (i = 0; i < num_mipmaps; i++) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &testing);
		compressed_size += testing;
	}

	out_data = (ubyte*)vm_malloc(compressed_size * sizeof(ubyte));

	Assert( out_data != NULL );

	memset(out_data, 0, compressed_size * sizeof(ubyte));

	// reset compressed_size and go back through each mipmap level to get both size and the image data itself
	compressed_size = 0;

	for (i = 0; i < num_mipmaps; i++) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &testing);
		vglGetCompressedTexImageARB(GL_TEXTURE_2D, i, out_data + compressed_size);
		compressed_size += testing;
	}

	// done with everything so reset hints to default values
	if (num_mipmaps > 1) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);
	}

	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_DONT_CARE);


	GL_state.Texture.Disable();
	GL_state.Texture.Delete(tex);
	glDeleteTextures(1, &tex);

	// send the data back out
	*compressed_data = out_data;

	GL_CHECK_FOR_ERRORS("end of compress_image()");

	return compressed_size;
}

// sends a texture object out to "image_data", which should be memory which is already allocated
// this should only be used for uncompressed 24-bit or 32-bit (distiguished by "alpha" var) images
// returns 0 on failure, size of data on success
int opengl_export_image( int slot, int width, int height, int alpha, int num_mipmaps, ubyte *image_data )
{
	tcache_slot_opengl *ts = &Textures[slot];

	GL_CHECK_FOR_ERRORS("start of export_image()");

	if (!image_data) {
		mprintf(("OpenGL ERROR: Tried to export a texture without a valid export location!\n"));
		return 0;
	}

	if (!ts->texture_target) {
		mprintf(("OpenGL ERROR: Tried to export a texture for which I don't know the texture target!\n"));
		return 0;
	}

	if (ts->mipmap_levels != num_mipmaps) {
		mprintf(("OpenGL ERROR: Number of mipmap levels requested is different from number available!\n"));
		return 0;
	}

	if ( (ts->texture_target != GL_TEXTURE_2D) && (ts->texture_target != GL_TEXTURE_CUBE_MAP) ) {
		mprintf(("OpenGL ERROR: Only 2D textures and cube maps can be exported!\n"));
		return 0;
	}

	if ( (ts->w != width) && (ts->h != height) ) {
		mprintf(("OpenGL ERROR: Passed width and height do not match values for texture!\n"));
		return 0;
	}

	int faces = 1;
	int m_width = width;
	int m_height = height;
	int m_bpp = (alpha) ? 4 : 3;
	int m_offset = 0;
	GLenum target = (ts->texture_target == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;

	if (ts->texture_target == GL_TEXTURE_CUBE_MAP)
		faces = 6;

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(ts->texture_target);
	GL_state.Texture.Enable(ts->texture_id);

	for (int i = 0; i < faces; i++) {
		for (int j = 0; j < ts->mipmap_levels; j++) {
			glGetTexImage(target + i, j, (alpha) ? GL_BGRA : GL_BGR, (alpha) ? GL_UNSIGNED_INT_8_8_8_8_REV : GL_UNSIGNED_BYTE, image_data + m_offset);

			m_offset += (m_width * m_height * m_bpp);

			// reduce by half for next mipmap level
			m_width >>= 1;
			m_height >>= 1;

			if (m_width < 1)
				m_width = 1;

			if (m_height < 1)
				m_height = 1;
		}

		// restore original width and height for next face
		m_width = width;
		m_height = height;
	}

	GL_state.Texture.Disable();

	GL_CHECK_FOR_ERRORS("end of export_image()");

	return m_offset;
}

// -----------------------------------------------------------------------------
// GL_EXT_framebuffer_object stuff (ie, render-to-texture)
//

struct fbo_t {
	// these first vars should only be modified in opengl_make_render_target()
	GLuint renderbuffer_id;
	GLuint framebuffer_id;
	int width;
	int height;
	int ref_count;
	// these next 2 should only be modifed in opengl_set_render_target()
	int working_slot;
	int is_static;

	fbo_t(): renderbuffer_id(0), framebuffer_id(0), width(0), height(0), ref_count(0), working_slot(-1), is_static(0) { }
};

static std::vector<fbo_t> RenderTarget;
static fbo_t *render_target = NULL;


int opengl_check_framebuffer()
{
	GLenum status;

	status = vglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		char err_txt[100] = { 0 };

		switch (status) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
				strcpy(err_txt, "Incomplete attachments!");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				strcpy(err_txt, "Missing one or more image attachments!");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				strcpy(err_txt, "Attached images do not have the same width and height!");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				strcpy(err_txt, "Attached images do not have the same internal format!");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				strcpy(err_txt, "Draw buffer attachment point is NONE!");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
				strcpy(err_txt, "Read buffer attachment point is NONE!");
				break;

			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
				strcpy(err_txt, "Attached images violate current FBO restrictions!");
				break;

			default:
				strcpy(err_txt, "Unknown error!\n");
				break;
		}

		mprintf(("Framebuffer ERROR: %s\n", err_txt));

		return (int)status;
	}

	return 0;
}

static fbo_t *opengl_get_fbo(int width, int height)
{
	uint rt_size = RenderTarget.size();

	for (uint i = 0; i < rt_size; i++) {
		if ( (RenderTarget[i].width == width) && (RenderTarget[i].height == height) ) {
			return &RenderTarget[i];
		}
	}

	return NULL;
}

void opengl_kill_render_target(int slot)
{
	if ( (slot < 0) || (slot >= MAX_BITMAPS) ) {
		Int3();
		return;
	}

	// this will happen when opengl_kill_all_render_targets() gets called first on exit
	if ( RenderTarget.empty() ) {
		return;
	}

	tcache_slot_opengl *ts = &Textures[slot];
	uint idx = 0;

	for (idx = 0; idx < RenderTarget.size(); idx++) {
		if ( (RenderTarget[idx].width == ts->w) && (RenderTarget[idx].height == ts->h) ) {
			break;
		}
	}

	// this may happen when textures are flushed, the w and h will get reset to 0
	if ( idx >= RenderTarget.size() ) {
		return;
	}

	RenderTarget[idx].ref_count--;

	if (RenderTarget[idx].ref_count <= 0) {
		mprintf(("OpenGL: Killing off unused %ix%i render target...\n", ts->w, ts->h));

		if (RenderTarget[idx].framebuffer_id) {
			vglDeleteFramebuffersEXT(1, &RenderTarget[idx].framebuffer_id);
			RenderTarget[idx].framebuffer_id = 0;
		}

		if (RenderTarget[idx].renderbuffer_id) {
			vglDeleteRenderbuffersEXT(1, &RenderTarget[idx].renderbuffer_id);
			RenderTarget[idx].renderbuffer_id = 0;
		}

		RenderTarget.erase( RenderTarget.begin() + idx );
	} else {
		mprintf(("OpenGL: Keeping %ix%i render target with ref_count of %i.\n", ts->w, ts->h, RenderTarget[idx].ref_count));
	}
}

void opengl_kill_all_render_targets()
{
	for (uint i = 0; i < RenderTarget.size(); i++) {
		fbo_t *fbo = &RenderTarget[i];

		if (fbo->framebuffer_id) {
			vglDeleteFramebuffersEXT(1, &fbo->framebuffer_id);
			fbo->framebuffer_id = 0;
		}

		if (fbo->renderbuffer_id) {
			vglDeleteRenderbuffersEXT(1, &fbo->renderbuffer_id);
			fbo->renderbuffer_id = 0;
		}
	}

	RenderTarget.clear();
}

int opengl_set_render_target( int slot, int face, int is_static )
{
	tcache_slot_opengl *ts = NULL;
	fbo_t *fbo = NULL;

	GL_CHECK_FOR_ERRORS("start of set_render_target()");

	if (slot < 0) {
		if ( (render_target != NULL) && (render_target->working_slot >= 0) ) {
		//	if (Cmdline_mipmap) {
		//		ts = &Textures[render_target->working_slot];

		//		glBindTexture(ts->texture_target, ts->texture_id);
		//		vglGenerateMipmapEXT(ts->texture_target);
		//		glBindTexture(ts->texture_target, 0);
		//	}

			if (render_target->is_static) {
				extern void gr_opengl_bm_save_render_target(int slot);
				gr_opengl_bm_save_render_target(render_target->working_slot);
			}
		}

		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);

		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		vglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		if (render_target != NULL) {
			render_target->working_slot = -1;
			render_target->is_static = 0;
		}

		// done with this render target so lets move on
		render_target = NULL;

		GL_rendering_to_framebuffer = false;

		GL_CHECK_FOR_ERRORS("end of set_render_target(0)");

		return 1;
	}

	ts = &Textures[slot];
	Assert( ts != NULL );

	if (!ts->texture_id) {
		Int3();
		return 0;
	}

	fbo = opengl_get_fbo(ts->w, ts->h);

	if (fbo == NULL) {
		mprintf(("Tried to get an OpenGL FBO that didn't exist!\n"));
		return 0;
	}

	if ( !vglIsFramebufferEXT(fbo->framebuffer_id) /*|| !vglIsRenderbufferEXT(fbo->renderbuffer_id)*/ ) {
		Int3();
		return 0;
	}

//	vglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo->renderbuffer_id);
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo->framebuffer_id);

	if (ts->texture_target == GL_TEXTURE_CUBE_MAP) {
		Assert( (face >= 0) && (face < 6) );
		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, ts->texture_id, 0);
	} else {
		Assert( face <= 0 );
		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, ts->texture_target, ts->texture_id, 0);
	}

//	vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo->renderbuffer_id);

	if ( opengl_check_framebuffer() ) {
		Int3();
		return 0;
	}

	fbo->working_slot = slot;
	fbo->is_static = is_static;

	// save current fbo for later use
	render_target = fbo;

	GL_rendering_to_framebuffer = true;

	GL_CHECK_FOR_ERRORS("end of set_render_target()");

	return 1;
}

int opengl_make_render_target( int handle, int slot, int *w, int *h, ubyte *bpp, int *mm_lvl, int flags )
{
	Assert( !GL_rendering_to_framebuffer );

	if (slot < 0) {
		Int3();
		return 0;
	}

	// got to have at least width and height!
	if (!w || !h) {
		Int3();
		return 0;
	}

	// size check
	if (*w > GL_max_renderbuffer_size) {
		*w = GL_max_renderbuffer_size;
	}

	if (*h > GL_max_renderbuffer_size) {
		*h = GL_max_renderbuffer_size;
	}

	tcache_slot_opengl *ts = &Textures[slot];
	fbo_t *fbo = opengl_get_fbo(*w, *h);

	GL_CHECK_FOR_ERRORS("start of make_render_target()");

	// since we only deal with one frame/render buffer, see if we need to modify what we have or use it as is
	if ( (fbo != NULL) && (fbo->width == *w) && (fbo->height == *h) ) {
		// both should be valid, but we check to catch the off-chance that something is fubar
		Assert( vglIsFramebufferEXT(fbo->framebuffer_id) /*&& vglIsRenderbufferEXT(fbo->renderbuffer_id)*/ );

		// we can use the existing FBO without modification so just setup the texture slot and move on
		if (flags & BMP_FLAG_CUBEMAP) {
			opengl_set_texture_target( GL_TEXTURE_CUBE_MAP );
		} else {
			opengl_set_texture_target();
		}

		glGenTextures(1, &ts->texture_id);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_texture_target);
		GL_state.Texture.Enable(ts->texture_id);

		GL_state.Texture.SetMinFilter(GL_LINEAR);
		GL_state.Texture.SetMagFilter(GL_LINEAR);
		GL_state.Texture.SetWrapS(GL_CLAMP_TO_EDGE);
		GL_state.Texture.SetWrapT(GL_CLAMP_TO_EDGE);

		if (flags & BMP_FLAG_CUBEMAP) {
			// if a cubemap then we have to initalize each face
			for (int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, *w, *h, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
			}

			ts->texture_target = GL_TEXTURE_CUBE_MAP;
		} else {
			// non-cubemap so just do the normal thing
			glTexImage2D(GL_state.Texture.GetTarget(), 0, GL_RGB8, *w, *h, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
			ts->texture_target = GL_state.Texture.GetTarget();
		}

	/*	if (Cmdline_mipmap) {
			vglGenerateMipmapEXT(GL_state.Texture.GetTarget());

			extern int get_num_mipmap_levels(int w, int h);
			ts->mipmap_levels = get_num_mipmap_levels(*w, *h);
		} else */
		{
			ts->mipmap_levels = 1;
		}

		GL_state.Texture.Disable();

		ts->w = (ushort)*w;
		ts->h = (ushort)*h;
		ts->bpp = 24;
		ts->bitmap_handle = -1;//handle;
		ts->u_scale = 1.0f;
		ts->v_scale = 1.0f;

		if (bpp) {
			*bpp = ts->bpp;
		}

		if (mm_lvl) {
			*mm_lvl = ts->mipmap_levels;
		}

		opengl_set_texture_target();

		fbo->ref_count++;

		mprintf(("OpenGL: Reusing %ix%i FBO!\n", *w, *h));

		GL_CHECK_FOR_ERRORS("end of make_render_target(0)");

		return 1;
	}

	// now on to the good parts...

	fbo_t new_fbo;

	if (flags & BMP_FLAG_CUBEMAP) {
		opengl_set_texture_target( GL_TEXTURE_CUBE_MAP );
	} else {
		opengl_set_texture_target();
	}

	// initialize color texture
	glGenTextures(1, &ts->texture_id);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_texture_target);
	GL_state.Texture.Enable(ts->texture_id);

	GL_state.Texture.SetMinFilter(GL_LINEAR);
	GL_state.Texture.SetMagFilter(GL_LINEAR);
	GL_state.Texture.SetWrapS(GL_CLAMP_TO_EDGE);
	GL_state.Texture.SetWrapT(GL_CLAMP_TO_EDGE);

	if (flags & BMP_FLAG_CUBEMAP) {
		// if a cubemap then we have to initalize each face
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, *w, *h, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
		}
	} else {
		// non-cubemap so just do the normal thing
		glTexImage2D(GL_state.Texture.GetTarget(), 0, GL_RGB8, *w, *h, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
	}

/*	if (Cmdline_mipmap) {
		vglGenerateMipmapEXT(GL_state.Texture.GetTarget());

		extern int get_num_mipmap_levels(int w, int h);
		ts->mipmap_levels = get_num_mipmap_levels(*w, *h);
	} else */
	{
		ts->mipmap_levels = 1;
	}

	GL_state.Texture.Disable();

	// render buffer
//	vglGenRenderbuffersEXT(1, &new_fbo.renderbuffer_id);
//	vglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, new_fbo.renderbuffer_id);
//	vglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, *w, *h);
	vglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	// frame buffer
	vglGenFramebuffersEXT(1, &new_fbo.framebuffer_id);
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, new_fbo.framebuffer_id);

	if (flags & BMP_FLAG_CUBEMAP) {
		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ts->texture_id, 0);
	} else {
		vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_state.Texture.GetTarget(), ts->texture_id, 0);
	}

//	vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, new_fbo.renderbuffer_id);

	if ( opengl_check_framebuffer() ) {
		// Oops!!  reset everything and then bail
		mprintf(("OpenGL: Unable to create FBO!\n"));

		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		GL_state.Texture.Disable();
		GL_state.Texture.Delete(ts->texture_id);
		glDeleteTextures(1, &ts->texture_id);
		ts->texture_id = 0;

		vglDeleteFramebuffersEXT(1, &new_fbo.framebuffer_id);

	//	vglDeleteRenderbuffersEXT(1, &new_fbo.renderbuffer_id);

		opengl_set_texture_target();

		return 0;
	}

	// save anything that needs saving, cleanup, and then exit
	ts->w = (ushort)*w;
	ts->h = (ushort)*h;
	ts->bpp = 24;
	ts->texture_target = GL_state.Texture.GetTarget();
	ts->bitmap_handle = -1;//handle;
	ts->u_scale = 1.0f;
	ts->v_scale = 1.0f;

	new_fbo.width = ts->w;
	new_fbo.height = ts->h;

	if (bpp) {
		*bpp = ts->bpp;
	}

	if (mm_lvl) {
		*mm_lvl = ts->mipmap_levels;
	}

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	new_fbo.ref_count++;

	RenderTarget.push_back(new_fbo);

	opengl_set_texture_target();


	mprintf(("OpenGL: Created %ix%i FBO!\n", ts->w, ts->h));

	GL_CHECK_FOR_ERRORS("end of make_render_target()");

	return 1;
}

//
// End of GL_EXT_framebuffer_object stuff
// -----------------------------------------------------------------------------

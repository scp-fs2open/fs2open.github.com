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
 * $Revision: 1.40 $
 * $Date: 2006-01-12 17:42:56 $
 * $Author: wmcoolmon $
 *
 * source for texturing in OpenGL
 *
 * $Log: not supported by cvs2svn $
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

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "osapi/osregistry.h"
#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "graphics/gropengl.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "graphics/grinternal.h"
#include "ddsutils/ddsutils.h"


static tcache_slot_opengl *Textures = NULL;

ubyte Tex_used_this_frame[MAX_BITMAPS];

int GL_texture_ram = 0;
int GL_frame_count = 0;
int GL_min_texture_width = 0;
GLint GL_max_texture_width = 0;
int GL_min_texture_height = 0;
GLint GL_max_texture_height = 0;
int GL_square_textures = 0;
int GL_textures_in = 0;
int GL_textures_in_frame = 0;
int GL_last_bitmap_id = -1;
int GL_last_detail = -1;
int GL_last_bitmap_type = -1;
GLint GL_supported_texture_units = 2;
int GL_should_preload = 0;
ubyte GL_xlat[256] = { 0 };
GLfloat GL_anisotropy = 0.0f;
GLfloat GL_max_anisotropy = 0.0f;
static int vram_full = 0;			// UnknownPlayer
int GL_mipmap_filter = 0;

extern int GLOWMAP;
extern int SPECMAP;
extern int CLOAKMAP;
extern int Interp_multitex_cloakmap;

//opengl supports 32 multitexture units
//we will too incase people are playing fs2_open in 2020
static int GL_texture_units_enabled[32]={0};

// forward declarations
int opengl_free_texture(tcache_slot_opengl *t);
void opengl_free_texture_with_handle(int handle);
void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out);
int opengl_create_texture_sub(int bitmap_handle, int bitmap_type, int bmap_w, int bmap_h, int tex_w, int tex_h, ushort *data = NULL, tcache_slot_opengl *t = NULL, int base_level = 0, int resize = 0, int reload = 0, int fail_on_full = 0);
int opengl_create_texture (int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot = NULL, int fail_on_full = 0);

void opengl_set_additive_tex_env()
{
	if (opengl_extension_is_enabled(GL_ARB_ENV_COMBINE))
	{
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
	}
	else if (opengl_extension_is_enabled(GL_EXT_ENV_COMBINE))
	{
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
	}
	else {
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	}
}

void opengl_set_modulate_tex_env()
{
	if (opengl_extension_is_enabled(GL_ARB_ENV_COMBINE))
	{
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE); // make sure
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 4.0f);
	}
	else if (opengl_extension_is_enabled(GL_EXT_ENV_COMBINE))
	{
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE); // make sure
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 4.0f);
	}
	else {
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
}

void gr_opengl_set_tex_env_scale(float scale)
{
	if (GL_Extensions[GL_ARB_ENV_COMBINE].enabled)
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, scale);
	else if (GL_Extensions[GL_EXT_ENV_COMBINE].enabled)
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, scale);
	else
	{}
}

GLfloat opengl_get_max_anisotropy()
{
	if ( !opengl_extension_is_enabled(GL_TEX_FILTER_ANISO) )
		return 0.0f;

	if ( !GL_max_anisotropy ) {
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &GL_max_anisotropy);
	}

	return GL_max_anisotropy;
}

// setup anisotropic filtering if we can
void opengl_set_anisotropy(GLfloat aniso_value)
{
	if ( !opengl_extension_is_enabled(GL_TEX_FILTER_ANISO) )
		return;

	if ( !GL_max_anisotropy ) {
		opengl_get_max_anisotropy();
	}

	if ( !GL_anisotropy ) {
		char *plevel;

		plevel = os_config_read_string( NULL, NOX("OGL_AnisotropicFilter"), NOX("1.0") );
		GL_anisotropy = (GLfloat)atof(plevel);

		if ( GL_anisotropy < 1.0f ) {
			GL_anisotropy = 1.0f;
		} else if ( GL_anisotropy > GL_max_anisotropy ) {
			GL_anisotropy = GL_max_anisotropy;
		}
	}

	if ( (aniso_value >= 1.0f) && (aniso_value <= GL_max_anisotropy) ) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso_value);
		GL_anisotropy = aniso_value;
	} else {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, GL_anisotropy);
	}
}

void opengl_switch_arb(int unit, int state)
{
	if (unit >= GL_supported_texture_units)
		return;

	if ( unit < 0 ) {
		// support a mass disable on all arbs so we don't have to make each call ourselves
		if ( !state ) {
			for ( int i = 0; i < GL_supported_texture_units; i++ ) {
				// only turn off 2D if the arb was previously enabled (fixes a specmap problem)
				if (GL_texture_units_enabled[i]) {
					glActiveTextureARB(GL_TEXTURE0_ARB + i);
					glDisable(GL_TEXTURE_2D);
				}

				if ( !Cmdline_nohtl ) {
					// the rest of this is always turned off regardless of whether or not the arb
					// was previously enabled or not
					glClientActiveTextureARB(GL_TEXTURE0_ARB + i);

					glDisableClientState( GL_VERTEX_ARRAY );
					glDisableClientState( GL_NORMAL_ARRAY );
					glDisableClientState( GL_TEXTURE_COORD_ARRAY );

					if (VBO_ENABLED) {
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
					}
				}

				GL_texture_units_enabled[i] = 0;
			}
		}

		return;
	}
			
	if (state) {
		if (GL_texture_units_enabled[unit])
			return;

		glActiveTextureARB(GL_TEXTURE0_ARB + unit);
		glEnable(GL_TEXTURE_2D);
		GL_texture_units_enabled[unit] = 1;
	} else {
		if (!GL_texture_units_enabled[unit])
			return;

		glActiveTextureARB(GL_TEXTURE0_ARB + unit);
		glDisable(GL_TEXTURE_2D);
	
		GL_texture_units_enabled[unit] = 0;
	}
}

void opengl_tcache_init (int use_sections)
{
	int i;

	// DDOI - FIXME skipped a lot of stuff here
	GL_should_preload = 0;

	//uint tmp_pl = os_config_read_uint( NULL, NOX("D3DPreloadTextures"), 255 );
	uint tmp_pl = 1;

	if ( tmp_pl == 0 )      {
		GL_should_preload = 0;
	} else if ( tmp_pl == 1 )       {
		GL_should_preload = 1;
	} else {
		GL_should_preload = 1;
	}

	GL_min_texture_width = 16;
	GL_min_texture_height = 16;
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GL_max_texture_width);

	GL_max_texture_height = GL_max_texture_width;

	// if we are not using sections then make sure we have the min texture size available to us
	// 1024 is what we need with the standard resolutions - taylor
	if (GL_max_texture_width < 1024)
		Error(LOCATION, "A minimum texture size of \"1024x1024\" is required for FS2_Open but only \"%ix%i\" was found.  Can not continue.", GL_max_texture_width, GL_max_texture_height);

	GL_square_textures = 0;

	if ( Textures == NULL ) {
		Textures = (tcache_slot_opengl *)vm_malloc(MAX_BITMAPS*sizeof(tcache_slot_opengl));
	}

	if ( !Textures ) {
		Error(LOCATION, "Unable to allocate memory for OpenGL texture slots!");
	}

	memset( Textures, 0, MAX_BITMAPS * sizeof(tcache_slot_opengl) );

	memset( Tex_used_this_frame, 0, MAX_BITMAPS * sizeof(ubyte) );

	// Init the texture structures
	for( i=0; i<MAX_BITMAPS; i++ )  {
		Textures[i].bitmap_handle = -1;
	}

	// check what mipmap filter we should be using
	//   0  ==  Bilinear
	//   1  ==  Trilinear
	GL_mipmap_filter = os_config_read_uint(NULL, "TextureFilter", 0);

	if (GL_mipmap_filter > 1)
		GL_mipmap_filter = 1;

	//GL_last_detail = Detail.hardware_textures;
	GL_last_bitmap_id = -1;
	GL_last_bitmap_type = -1;

	GL_textures_in = 0;
	GL_textures_in_frame = 0;
}

int opengl_free_texture (tcache_slot_opengl *t);

void opengl_free_texture_with_handle(int handle)
{
	int n = bm_get_cache_slot(handle, 1);

	Tex_used_this_frame[n] = 0;
	opengl_free_texture( &Textures[n] );
}

void opengl_tcache_flush ()
{
	int i;

	if ( Textures == NULL )
		return;

	for( i=0; i<MAX_BITMAPS; i++ )  {
		opengl_free_texture( &Textures[i] );
	}

	if (GL_textures_in != 0) {
		mprintf(( "WARNING: VRAM is at %d instead of zero after flushing!\n", GL_textures_in ));
		GL_textures_in = 0;
	}

	GL_last_bitmap_id = -1;
}

void opengl_tcache_cleanup ()
{
	opengl_tcache_flush ();

	GL_textures_in = 0;
	GL_textures_in_frame = 0;

	if ( Textures ) {
		vm_free(Textures);
		Textures = NULL;
	}
}

void opengl_tcache_frame ()
{

	GL_last_bitmap_id = -1;
	GL_textures_in_frame = 0;

	GL_frame_count++;

	// make all textures as not used
	memset( Tex_used_this_frame, 0, MAX_BITMAPS * sizeof(ubyte) );

	if ( vram_full )        {
		opengl_tcache_flush();
		vram_full = 0;
	}
}

void opengl_free_texture_slot( int n )
{
	Tex_used_this_frame[n] = 0;
	opengl_free_texture( &Textures[n] );
}

int opengl_free_texture ( tcache_slot_opengl *t )
{
	// Bitmap changed!!     
	if ( t->bitmap_handle > -1 )        {
		// if I, or any of my children have been used this frame, bail  
		// can't use bm_get_cache_slot() here since bitmap_id probably isn't valid
		if ( Tex_used_this_frame[t->bitmap_handle % MAX_BITMAPS] ) {
			return 0;
		}

		// ok, now we know its legal to free everything safely
		glDeleteTextures (1, &t->texture_id);
	//	t->texture_id = 0;

		if ( GL_last_bitmap_id == t->bitmap_handle )       {
			GL_last_bitmap_id = -1;
		}

		GL_textures_in -= t->size;
		memset( t, 0, sizeof(tcache_slot_opengl) );
		t->bitmap_handle = -1;
	//	t->bpp = 0;
	}

	return 1;
}

void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	int i, tex_w, tex_h;

	// bogus
	if((w_out == NULL) ||  (h_out == NULL)){
		return;
	}

	// if we can support non-power-of-2 textures then just return current sizes - taylor
	if ( opengl_extension_is_enabled(GL_ARB_TEXTURE_NON_POWER_OF_TWO) ) {
		*w_out = w_in;
		*h_out = h_in;

		return;
	}

	// starting size
	tex_w = w_in;
	tex_h = h_in;

	for (i=0; i<16; i++ )   {
		if ( (tex_w > (1<<i)) && (tex_w <= (1<<(i+1))) )        {
			tex_w = 1 << (i+1);
			break;
		}
	}

	for (i=0; i<16; i++ )   {
		if ( (tex_h > (1<<i)) && (tex_h <= (1<<(i+1))) )        {
			tex_h = 1 << (i+1);
			break;
		}
	}

	if ( tex_w < GL_min_texture_width ) {
		tex_w = GL_min_texture_width;
	} else if ( tex_w > GL_max_texture_width )     {
		tex_w = GL_max_texture_width;
	}

	if ( tex_h < GL_min_texture_height ) {
		tex_h = GL_min_texture_height;
	} else if ( tex_h > GL_max_texture_height )    {
		tex_h = GL_max_texture_height;
	}

	if ( GL_square_textures )      {
		int new_size;
		// Make the both be equal to larger of the two
		new_size = MAX(tex_w, tex_h);
		tex_w = new_size;
		tex_h = new_size;
	}

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

// data == start of bitmap data
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int opengl_create_texture_sub(int bitmap_handle, int bitmap_type, int bmap_w, int bmap_h, int tex_w, int tex_h, ushort *data, tcache_slot_opengl *t, int base_level, int resize, int reload, int fail_on_full)
{
	int ret_val = 1;
	int byte_mult = 0;
	GLenum texFormat = GL_UNSIGNED_BYTE;
	GLenum glFormat = GL_BGRA_EXT;
	GLuint intFormat = GL_RGBA;
	ubyte saved_bpp = 0;
	int mipmap_w = 0, mipmap_h = 0;
	int dsize = 0, doffset = 0, block_size = 0;
	int i,j,k;
	ubyte *bmp_data = ((ubyte*)data);
	ubyte *texmem = NULL, *texmemp;
	int mipmap_levels = 0;


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
		if(!opengl_free_texture(t)){
			return 0;
		}
	}

	// for everything that might use mipmaps
	mipmap_w = tex_w;
	mipmap_h = tex_h;

	if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_INTERFACE) ) {
		t->u_scale = (float)bmap_w / (float)tex_w;
		t->v_scale = (float)bmap_h / (float)tex_h;
	} else {
		t->u_scale = 1.0f;
		t->v_scale = 1.0f;
	}

	if (!reload) {
		glGenTextures (1, &t->texture_id);
	}
	
	if (t->texture_id == 0) {
		mprintf(("!!OpenGL DEBUG!! t->texture_id == 0"));
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
		glFormat = GL_BGRA_EXT;
	} else if (byte_mult == 3) {
		texFormat = GL_UNSIGNED_BYTE;
		intFormat = (gr_screen.bits_per_pixel == 32) ? GL_RGB8 : GL_RGB5;
		glFormat = GL_BGR_EXT;
	} else if (byte_mult == 2) {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGB5_A1;
		glFormat = GL_BGRA_EXT;
	} else if (byte_mult == 1) {
		Assert( bitmap_type == TCACHE_TYPE_AABITMAP );
		texFormat = GL_UNSIGNED_BYTE;
		intFormat = GL_ALPHA;
		glFormat = GL_ALPHA;
	} else {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGBA;
		glFormat = GL_BGRA_EXT;
	}

	// check for compressed image types
	switch (bm_is_compressed(bitmap_handle)) {
		case DDS_DXT1:
			intFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			block_size = 8;
			break;

		case DDS_DXT3:
			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			block_size = 16;
			break;

		case DDS_DXT5:
			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			block_size = 16;
			break;
	}


	glBindTexture (GL_TEXTURE_2D, t->texture_id);

#ifdef __APPLE__
	if ( opengl_extension_is_enabled(GL_APPLE_CLIENT_STORAGE) && !resize && (byte_mult != 1)) {
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
	}
#endif

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	mipmap_levels = bm_get_num_mipmaps(bitmap_handle);

	if ( (mipmap_levels > 1) && (base_level+1 < mipmap_levels) ) {
		// bifilter  = GL_LINEAR_MIPMAP_NEAREST
		// trifilter = GL_LINEAR_MIPMAP_LINEAR
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GL_mipmap_filter) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
	}


	switch (bitmap_type)
	{
		case TCACHE_TYPE_COMPRESSED:
		{
			if (block_size > 0) {
				// size of data block (4x4)
				dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

				// if we are skipping mipmap levels in order to resize then we have to calc the new offset
				for (i = 0; i < base_level; i++) {
					doffset += dsize;

					mipmap_w /= 2;
					mipmap_h /= 2;

					if (mipmap_w <= 0)
						mipmap_w = 1;

					if (mipmap_h <= 0)
						mipmap_h = 1;

					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;
				}

				if (!reload)
					glCompressedTexImage2D(GL_TEXTURE_2D, 0, intFormat, mipmap_w, mipmap_h, 0, dsize, bmp_data + doffset);
				else
					glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);

				// now that the base image is done handle any mipmap levels
				for (i = 1; i < (mipmap_levels - base_level); i++) {
					// adjust the data offset for the next block
					doffset += dsize;

					// reduce size by half for the next pass
					mipmap_w /= 2;
					mipmap_h /= 2;

					if (mipmap_w <= 0)
						mipmap_w = 1;

					if (mipmap_h <= 0)
						mipmap_h = 1;

					// size of data block (4x4)
					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

					if (!reload)
						glCompressedTexImage2D(GL_TEXTURE_2D, i, intFormat, mipmap_w, mipmap_h, 0, dsize, bmp_data + doffset);
					else
						glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);
				}
			} else {
				Int3();
				return 0;
			}

			break;
		}

		case TCACHE_TYPE_AABITMAP:
		{
			texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
			texmemp = texmem;

			Assert( texmem != NULL );

			for (i=0;i<tex_h;i++)
			{
				for (j=0;j<tex_w;j++)
				{
					if (i < bmap_h && j < bmap_w) {
						*texmemp++ = GL_xlat[bmp_data[i*bmap_w+j]];
					} else {
						*texmemp++ = 0;
					}
				}
			}

			if (!reload)
				glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, tex_w, tex_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texmem);
			else // faster anis
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_ALPHA, GL_UNSIGNED_BYTE, texmem);

			if (texmem != NULL)
				vm_free (texmem);

			break;
		}

		case TCACHE_TYPE_INTERFACE:
		{
			// if we aren't resizing then we can just use bmp_data directly
			if ( resize ) {
				texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
				texmemp = texmem;

				Assert( texmem != NULL );

				for (i=0;i<tex_h;i++)
				{
					for (j=0;j<tex_w;j++)
					{
						if (i < bmap_h && j < bmap_w) {
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

			if (!reload)
				glTexImage2D (GL_TEXTURE_2D, 0, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, (resize) ? texmem : bmp_data);
			else // faster anis
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, (resize) ? texmem : bmp_data);

			if (texmem != NULL)
				vm_free(texmem);

			break;
		}

		default:
		{
			// if we aren't resizing then we can just use bmp_data directly
			if ( resize ) {
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
			if ( (mipmap_levels > 1) && resize )
				Assert( texmem == NULL );

			dsize = mipmap_h * mipmap_w * byte_mult;

			// if we are skipping mipmap levels in order to resize then we have to calc the new offset
			for (i = 0; i < base_level; i++) {
				doffset += dsize;

				mipmap_w /= 2;
				mipmap_h /= 2;

				if (mipmap_w <= 0)
					mipmap_w = 1;

				if (mipmap_h <= 0)
					mipmap_h = 1;

				dsize = mipmap_h * mipmap_w * byte_mult;
			}

			if (!reload)
				glTexImage2D (GL_TEXTURE_2D, 0, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, (texmem != NULL) ? texmem : bmp_data + doffset);
			else // faster anis
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, (texmem != NULL) ? texmem : bmp_data + doffset);


			// base image is done so now take care of any mipmap levels
			for (i = 1; i < (mipmap_levels - base_level); i++) {
				doffset += dsize;
				mipmap_w /= 2;
				mipmap_h /= 2;

				if (mipmap_w <= 0)
					mipmap_w = 1;

				if (mipmap_h <= 0)
					mipmap_h = 1;

				dsize = mipmap_h * mipmap_w * byte_mult;

				if (!reload)
					glTexImage2D (GL_TEXTURE_2D, i, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, bmp_data + doffset);
				else
					glTexSubImage2D (GL_TEXTURE_2D, i, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, bmp_data + doffset);
			}

			if (texmem != NULL)
				vm_free(texmem);

			break;
		}
	}//end switch

	t->bitmap_handle = bitmap_handle;
	t->time_created = GL_frame_count;
	Tex_used_this_frame[idx] = 0;
	if (bitmap_type == TCACHE_TYPE_COMPRESSED) {
		t->size = bm_get_size(bitmap_handle);
	} else {
		t->size = tex_w * tex_h * byte_mult;
	}
	t->w = (ushort)tex_w;
	t->h = (ushort)tex_h;
	GL_textures_in_frame += t->size;
	if (!reload) {
		GL_textures_in += t->size;
	}

#ifdef __APPLE__
	if ( opengl_extension_is_enabled(GL_APPLE_CLIENT_STORAGE) && !resize && (byte_mult != 1)) {
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 0);
	}
#endif

	return ret_val;
}

int opengl_create_texture (int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	ubyte bpp = 16;
	int reload = 0, resize = 0, base_level = 0;
	int max_levels = 0;

	if (tslot == NULL) {
		return 0;
	}

	// setup texture/bitmap flags
	flags = 0;
	switch(bitmap_type){
		case TCACHE_TYPE_AABITMAP:
			flags |= BMP_AABITMAP;
			bpp = 8;
			break;
		case TCACHE_TYPE_NORMAL:
			flags |= BMP_TEX_OTHER;
			break;
		case TCACHE_TYPE_INTERFACE:
		case TCACHE_TYPE_XPARENT:
			flags |= BMP_TEX_XPARENT;
			break;
		case TCACHE_TYPE_NONDARKENING:
			Int3();
			flags |= BMP_TEX_NONDARK;
			break;
		case TCACHE_TYPE_COMPRESSED:
			switch (bm_is_compressed(bitmap_handle)) {
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

	// if we ended up locking a texture that wasn't originally compressed then this should catch it
	if ( bm_is_compressed(bitmap_handle) ) {
		bitmap_type = TCACHE_TYPE_COMPRESSED;
	}

	// DDOI - TODO
	if ( (Detail.hardware_textures < 4) && (bitmap_type != TCACHE_TYPE_AABITMAP) && (bitmap_type != TCACHE_TYPE_INTERFACE)
			&& ((bitmap_type == TCACHE_TYPE_COMPRESSED) && (max_levels > 1)) )
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

			if ( base_level >= max_levels ) {
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
	if ( tslot->bitmap_handle < 0) {
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_handle != bitmap_handle)     {
		if((final_w == tslot->w) && (final_h == tslot->h)){
			reload = 1;
			//ml_printf("Reloading texture %d\n", bitmap_handle);
		} else {
			reload = 0;
		}
	}

	// set the bits per pixel
	tslot->bpp = bmp->bpp;

	// max number of mipmap levels (NOTE: this is the max number used by the API, not how many true mipmap levels there are in the image!!)
	tslot->mipmap_levels = (ubyte)(max_levels - base_level);

	// call the helper
	int ret_val = opengl_create_texture_sub(bitmap_handle, bitmap_type, bmp->w, bmp->h, final_w, final_h, (ushort*)bmp->data, tslot, base_level, resize, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

int gr_opengl_tcache_set_internal(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0, int tex_unit = 0)
{
	int ret_val = 1;

	if ( GL_last_detail != Detail.hardware_textures )      {
		GL_last_detail = Detail.hardware_textures;
		opengl_tcache_flush();
	}

	if (vram_full) {
		return 0;
	}

	int n = bm_get_cache_slot (bitmap_handle, 1);
	tcache_slot_opengl *t = &Textures[n];

	if ( (GL_last_bitmap_id == bitmap_handle) && (GL_last_bitmap_type == bitmap_type) && (t->bitmap_handle == bitmap_handle) )       {
		Tex_used_this_frame[n]++;

		*u_scale = t->u_scale;
		*v_scale = t->v_scale;
		return 1;
	}


	glActiveTextureARB(GL_TEXTURE0_ARB+tex_unit);

	opengl_set_anisotropy();

	if ((t->bitmap_handle < 0) || (bitmap_handle != t->bitmap_handle)) {
		ret_val = opengl_create_texture( bitmap_handle, bitmap_type, t, fail_on_full );
	}

	// everything went ok
	if(ret_val && (t->texture_id) && !vram_full){
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;

		
		glBindTexture (GL_TEXTURE_2D, t->texture_id );

		// OGL expects mipmap levels all the way down to 1x1 but I think this will avoid white texture
		// issues when we have fewer levels than that, it caps the total number of levels available with 0 as min value
		if ( t->mipmap_levels > 1 )
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, t->mipmap_levels - 1);

		GL_last_bitmap_id = t->bitmap_handle;
		GL_last_bitmap_type = bitmap_type;
		Tex_used_this_frame[n]++;
	}
	// gah
	else {
		glBindTexture (GL_TEXTURE_2D, 0);	// test - DDOI

		return 0;
	}

	return 1;
}

int gr_opengl_tcache_set(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int sx, int sy, int force, int stage)
{
	if (bitmap_handle < 0)
	{
		GL_last_bitmap_id = -1;
		return 0;
	}

	// set compressed type if it's so, needed to be right later
	if (bm_is_compressed(bitmap_handle) > 0) {
		bitmap_type = TCACHE_TYPE_COMPRESSED;
	}

	//make sure textuing is on
	opengl_switch_arb(stage, 1);

	return gr_opengl_tcache_set_internal(bitmap_handle, bitmap_type, u_scale, v_scale, fail_on_full, sx, sy, force, stage);
}

void gr_opengl_preload_init()
{
	if (gr_screen.mode != GR_OPENGL) {
		return;
	}

	opengl_tcache_flush ();
}

int gr_opengl_preload(int bitmap_num, int is_aabitmap)
{
	if ( gr_screen.mode != GR_OPENGL) {
		return 0;
	}

	if ( !GL_should_preload )      {
		return 0;
	}

	float u_scale, v_scale;

	int retval;
	if ( is_aabitmap )      {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale, 1 );
	} else {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 1 );
	}

	if ( !retval )  {
		mprintf(("Texture upload failed!\n" ));
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
	if (mode == TMAP_ADDRESS_WRAP) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	} else if (mode == TMAP_ADDRESS_MIRROR) {
		if (opengl_extension_is_enabled(GL_ARB_TEXTURE_MIRRORED_REPEAT)) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_ARB);
		} else {
			// just use a standard repeat if the mirror extension isn't supported
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
	} else if (mode == TMAP_ADDRESS_CLAMP) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
}

int opengl_compress_image( ubyte **compressed_data, ubyte *in_data, int width, int height, int alpha )
{
	Assert( in_data != NULL );

	GLuint tex;
	GLint compressed = GL_FALSE;
	GLint compressed_size = 0;
	ubyte *out_data = NULL;
	GLint testing = 0;

	if ( !Texture_compression_available )
		return 0;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// a quick proxy test.  this will tell us if it's possible without wasting a lot of time and resources in the attempt
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, (alpha) ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT, width, height, 0, (alpha) ? GL_BGRA_EXT : GL_BGR_EXT, GL_UNSIGNED_BYTE, in_data);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &testing);

	if ( (compressed == GL_FALSE) || (testing == 0) ) {
		glDeleteTextures(1, &tex);
		return 0;
	}

	// alright, it should work if we are still here, now do it for real
	glTexImage2D(GL_TEXTURE_2D, 0, (alpha) ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT, width, height, 0, (alpha) ? GL_BGRA_EXT : GL_BGR_EXT, GL_UNSIGNED_BYTE, in_data);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);

	// if we got this far then it should have worked, but check anyway
	Assert( compressed != GL_FALSE );

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &compressed_size);

	out_data = (ubyte*)vm_malloc(compressed_size * sizeof(ubyte));

	Assert( out_data != NULL );

	memset(out_data, 0, compressed_size * sizeof(ubyte));

	glGetCompressedTexImageARB(GL_TEXTURE_2D, 0, out_data);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	// send the data back out
	*compressed_data = out_data;

	return compressed_size;
}

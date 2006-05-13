/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLExtension.cpp $
 * $Revision: 1.15 $
 * $Date: 2006-05-13 07:29:52 $
 * $Author: taylor $
 *
 * source for extension implementation in OpenGL
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.14  2006/03/22 18:14:52  taylor
 * if -mipmap is used with -img2dds to then have compressed image also contain mipmaps
 * use nicest hints for texture compression, should improve quality a little
 * when reporting compressed sizes to debug log make ani size be total, not per frame
 *
 * Revision 1.13  2006/02/24 07:35:48  taylor
 * add v-sync support for OGL (I skimmped on this a bit but will go back to do something better, "special" extension wise, at a later date)
 *
 * Revision 1.12  2005/12/28 22:28:44  taylor
 * add support for glCompressedTexSubImage2D(), we don't use it yet but there is nothing wrong with adding it already
 * better support for mipmaps and mipmap filtering
 * add reg option "TextureFilter" to set bilinear or trilinear filter
 * clean up bitmap_id/bitmap_handle/texture_handle madness that made things difficult to understand
 * small fix for using 24-bit images on 16-bit bpp visual (untested)
 *
 * Revision 1.11  2005/12/08 15:10:07  taylor
 * add APPLE_client_storage support to improve texture performance and reduce memory usage a tiny bit on OS X
 *
 * Revision 1.10  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 1.9  2005/06/19 02:37:02  taylor
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
 * Revision 1.8  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 1.7  2005/04/13 23:24:21  phreak
 * Updated the message of a required extension that hasn't been found.
 * It was referencing the old Glide engine.
 *
 * Revision 1.6  2005/02/04 23:29:31  taylor
 * merge with Linux/OSX tree - p0204-3
 *
 * Revision 1.5  2005/01/21 08:25:14  taylor
 * fill in gr_opengl_set_texture_addressing()
 * add support for non-power-of-two textures for cards that have it
 * temporary crash fix from multiple mipmap levels in uncompressed formats
 *
 * Revision 1.4  2004/10/31 21:45:13  taylor
 * Linux tree merge, single array for VBOs/HTL
 *
 * Revision 1.3  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 1.2  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
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
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"

#include "osapi/outwnd.h"



char *OGL_extension_string;

// ogl_extension is:
//   - required for game flag
//   - enabled flag
//   - number of extensions to check for 
//   - extension name(s)  (max is 3 variants)
//   - number of functions that extension needs/wants
//   - function names that extension needs (max is 15)

// OpenGL extensions that we will want to use
ogl_extension GL_Extensions[NUM_OGL_EXTENSIONS] =
{
	// allows for per vertex fog coordinate
	{ 0, 0, 1, { "GL_EXT_fog_coord" }, 2, {
		"glFogCoordfEXT", "glFogCoordPointerEXT" } },

	// provides multiple texture units for rendering more than one texture in a single pass
	// (NOTE: this was included in OpenGL 1.2.1 standard, but we still need to check for it or require > 1.2 OGL)
	{ 1, 0, 1, { "GL_ARB_multitexture" } , 3, {
		"glMultiTexCoord2fARB", "glActiveTextureARB", "glClientActiveTextureARB" } },

	// "ADD" function for texture environment
	{ 1, 0, 2, { "GL_ARB_texture_env_add", "GL_EXT_texture_env_add" }, 0, { NULL } },

	// framework for using compressed textures (don't provide actual compression formats we use)
	{ 0, 0, 1, { "GL_ARB_texture_compression" }, 3, {
		"glCompressedTexImage2D", "glCompressedTexSubImage2D", "glGetCompressedTexImageARB" } },

	// S3TC texture compression/decompression support (DXT? encoded DDS images)
	{ 0, 0, 1, { "GL_EXT_texture_compression_s3tc" }, 0, { NULL } },

	// allows for setting of anisotropic filter (for mipmap texture filtering)
	{ 0, 0, 1, { "GL_EXT_texture_filter_anisotropic" }, 0, { NULL } },

	// provides app support to control how distance is calculated in fog computations
	{ 0, 0, 1, { "GL_NV_fog_distance" }, 0, { NULL } },

	// specify RGB values for secondary color
	{ 0, 0, 1, { "GL_EXT_secondary_color" }, 2, {
		"glSecondaryColor3fvEXT", "glSecondaryColor3ubvEXT" } },

	// "COMBINE" function for texture environment, these two are basically the same
	{ 0, 0, 2, { "GL_ARB_texture_env_combine", "GL_EXT_texture_env_combine" }, 0, { NULL } },

	// lock a vertex array buffer so that OGL can transform it just once with multiple draws
	{ 0, 0, 1, { "GL_EXT_compiled_vertex_array" }, 2, {
		"glLockArraysEXT", "glUnlockArraysEXT" } },

	// allows for row major order matrices rather than the standard column major order
	{ 1, 0, 1, { "GL_ARB_transpose_matrix" }, 2, {
		"glLoadTransposeMatrixfARB", "glMultTransposeMatrixfARB" } },

	// this is obsolete with OGL 1.2, which is why the function name is the standard, but we check for the orginal extension name
	{ 1, 0, 1, { "GL_EXT_draw_range_elements" }, 1, {
		"glDrawRangeElements" } },

	// extra texture wrap repeat mode
	{ 0, 0, 1, { "GL_ARB_texture_mirrored_repeat" }, 0, { NULL } },

	// allows for non-power-of-two textures, but without any cost to normal functionality or usage
	{ 0, 0, 1, { "GL_ARB_texture_non_power_of_two" }, 0, { NULL } },

	// creates buffer objects (cached in hi-speed video card memory) for vertex data
	{ 0, 0, 1, { "GL_ARB_vertex_buffer_object" }, 6, {
		"glBindBufferARB", "glDeleteBuffersARB", "glGenBuffersARB", "glBufferDataARB",
		"glMapBufferARB", "glUnmapBufferARB" } },

	// allows pixel data to use buffer objects
	{ 0, 0, 1, { "GL_ARB_pixel_buffer_object" }, 6, {
		"glBindBufferARB", "glDeleteBuffersARB", "glGenBuffersARB", "glBufferDataARB",
		"glMapBufferARB", "glUnmapBufferARB" } },

	// Mac-only extension that allows use of system copy of texture to avoid an additional API copy
	{ 0, 0, 1, { "GL_APPLE_client_storage" }, 0, { NULL } },

	// make me some mipmaps!
	{ 0, 0, 1, { "GL_SGIS_generate_mipmap" }, 0, { NULL } },

	// framebuffer object gives us render-to-texture support, among other things
	{ 0, 0, 1, { "GL_EXT_framebuffer_object" }, 15, { 
		"glIsRenderbufferEXT", "glBindRenderbufferEXT", "glDeleteRenderbuffersEXT", "glGenRenderbuffersEXT",
		"glRenderbufferStorageEXT", "glGetRenderbufferParameterivEXT", "glIsFramebufferEXT", "glBindFramebufferEXT",
		"glDeleteFramebuffersEXT", "glGenFramebuffersEXT", "glCheckFramebufferStatusEXT", "glFramebufferTexture2DEXT",
		"glFramebufferRenderbufferEXT", "glGetFramebufferAttachmentParameterivEXT", "glGenerateMipmapEXT" } },

	// these next three are almost exactly the same, just different stages of naming.
	// allows for non-power-of-textures, but at a cost of functionality
	// (NOTE: the EXT version is usually found only on the Mac)
	{ 0, 0, 3, { "GL_ARB_texture_rectangle", "GL_EXT_texture_rectangle", "GL_NV_texture_rectangle" }, 0, { NULL } },

	// for BGRA rather than RGBA support (it's faster in most cases)
	{ 1, 0, 1, { "GL_EXT_bgra" }, 0, { NULL } },

	// cube map support (for environment maps, normal maps, bump maps, etc.)
	{ 0, 0, 2, { "GL_ARB_texture_cube_map", "GL_EXT_texture_cube_map" }, 0, { NULL } }
};

// ogl_funcion is:
//   - function name
//   - pointer for extension

// All of the OpenGL functions that the extensions need
ogl_function GL_Functions[NUM_OGL_FUNCTIONS] =
{
	{ "glFogCoordfEXT", 0 },
	{ "glFogCoordPointerEXT", 0 },
	{ "glMultiTexCoord2fARB", 0 },
	{ "glActiveTextureARB", 0 },
	{ "glClientActiveTextureARB", 0 },
	{ "glCompressedTexImage2D", 0 },
	{ "glCompressedTexSubImage2D", 0 },
	{ "glGetCompressedTexImageARB", 0 },
	{ "glSecondaryColor3fvEXT", 0 },
	{ "glSecondaryColor3ubvEXT", 0 },
	{ "glLockArraysEXT", 0 },
	{ "glUnlockArraysEXT", 0 },
	{ "glLoadTransposeMatrixfARB", 0 },
	{ "glMultTransposeMatrixfARB", 0 },
	{ "glDrawRangeElements", 0 },
	{ "glBindBufferARB", 0 },
	{ "glDeleteBuffersARB", 0 },
	{ "glGenBuffersARB", 0 },
	{ "glBufferDataARB", 0 },
	{ "glMapBufferARB", 0 },
	{ "glUnmapBufferARB", 0 },
	{ "glIsRenderbufferEXT", 0 },
	{ "glBindRenderbufferEXT", 0 },
	{ "glDeleteRenderbuffersEXT", 0 },
	{ "glGenRenderbuffersEXT", 0 },
	{ "glRenderbufferStorageEXT", 0 },
	{ "glGetRenderbufferParameterivEXT", 0 },
	{ "glIsFramebufferEXT", 0 },
	{ "glBindFramebufferEXT", 0 },
	{ "glDeleteFramebuffersEXT", 0 },
	{ "glGenFramebuffersEXT", 0 },
	{ "glCheckFramebufferStatusEXT", 0 },
	{ "glFramebufferTexture2DEXT", 0 },
	{ "glFramebufferRenderbufferEXT", 0 },
	{ "glGetFramebufferAttachmentParameterivEXT", 0 },
	{ "glGenerateMipmapEXT", 0 }
};

// special extensions (only special functions are supported at the moment)
ogl_function GL_EXT_Special[NUM_OGL_EXT_SPECIAL] = {
	{ "wglSwapIntervalEXT", 0 },
	{ "glXSwapIntervalSGI", 0 }
};



//tries to find a certain extension
static inline int opengl_find_extension(const char* ext_to_find)
{
	return ( strstr(OGL_extension_string, ext_to_find) != NULL );
}

// these extensions may not be listed the normal way so we don't check the extension string for them
static int opengl_get_extensions_special()
{
	int i, num_found = 0;
	ogl_function *func = NULL;

	for (i = 0; i < NUM_OGL_EXT_SPECIAL; i++) {
		func = &GL_EXT_Special[i];

		Assert( func->function_name != NULL );

#ifdef _WIN32
		func->function_ptr = (ptr_u)wglGetProcAddress(func->function_name);
#else
		func->function_ptr = (ptr_u)SDL_GL_GetProcAddress(func->function_name);
#endif

		if (func->function_ptr) {
			mprintf(("  Found special extension function \"%s\".\n", func->function_name));
			num_found++;
		}
	}

	return num_found;
}

ogl_function *get_ogl_function( const char *name )
{
	if (name == NULL) {
		return NULL;
	}

	for (int i = 0; i < NUM_OGL_FUNCTIONS; i++) {
		if ( !strcmp(GL_Functions[i].function_name, name) ) {
			return &GL_Functions[i];
		}
	}

	return NULL;
}
	
//finds OGL extension functions
//returns number found
int opengl_get_extensions()
{
	OGL_extension_string = (char*)glGetString(GL_EXTENSIONS);
	int i, j, k, num_found = 0;
	ogl_extension *ext = NULL;
	ogl_function *func = NULL;

	for (i = 0; i < NUM_OGL_EXTENSIONS; i++) {
		ext = &GL_Extensions[i];
		k = 0;

		while ( !ext->enabled && (k < ext->num_extensions) ) {
			if ( opengl_find_extension(ext->extension_name[k]) ) {
				// some extensions do not have functions
				if (!ext->num_functions) {
					mprintf(("  Using extension \"%s\".\n", ext->extension_name[k]));
					ext->enabled = 1;
					num_found++;
					goto Next;
				}

				// we do have functions so check any/all of them
				for (j = 0; j < ext->num_functions; j++) {
					func = get_ogl_function( ext->function_names[j] );

					if (func == NULL)
						break;

					if (!func->function_ptr) {
#ifdef _WIN32
						func->function_ptr = (ptr_u)wglGetProcAddress(func->function_name);
#else
						func->function_ptr = (ptr_u)SDL_GL_GetProcAddress(func->function_name);
#endif
					}

					if (!func->function_ptr)
						break;
				}

				if ( j != ext->num_functions ) {
					mprintf(("  Found extension \"%s\", but can't find the required function \"%s()\".  Extension will be disabled!\n", ext->extension_name[k], ext->function_names[j]));

					if (ext->required_to_run) {
#ifdef _WIN32
						Error( LOCATION, "The required OpenGL extension '%s' is not fully supported by your current driver version or graphics card.  You can either use the Direct3D rendering engine (non-FRED builds only) or update your video card drivers.\n\n", ext->extension_name[k] );
#else
						Error( LOCATION, "The required OpenGL extension '%s' is not fully supported by your current driver version or graphics card.\n", ext->extension_name[k] );
#endif
					}
				} else {
					mprintf(("  Using extension \"%s\".\n", ext->extension_name[k]));
					ext->enabled = 1;
					num_found++;
				}
			} else {
				// only report if unable to find when we have checked all available extension name variants
				if ( k+1 >= ext->num_extensions ) {
					mprintf(("  Unable to find extension \"%s\".\n", ext->extension_name[k]));

					if (ext->required_to_run) {
#ifdef _WIN32
						Error( LOCATION, "The required OpenGL extension '%s' is not supported by your current driver version or graphics card.  You can either use the Direct3D rendering engine (non-FRED builds only) or update your video card drivers.\n\n", ext->extension_name );
#else
						Error( LOCATION, "The required OpenGL extension '%s' is not supported by your current driver version or graphics card.\n", ext->extension_name );
#endif
					}
				}
			}
			
			// now move the the next extension name
Next:
			k++;
		}
	}

	num_found += opengl_get_extensions_special();

	mprintf(( "\n" ));

	return num_found;
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifdef _WIN32
#include <windows.h>
#endif

#include "globalincs/pstypes.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"
#include "popup/popup.h"

#include "osapi/outwnd.h"



char *OGL_extension_string = NULL;

// ogl_extension is:
//   - required for game flag
//   - enabled flag
//   - number of extensions to check for 
//   - extension name(s)  (max is 3 variants)
//   - number of functions that extension needs/wants
//   - function names that extension needs (max is 20)

// OpenGL extensions that we will want to use
ogl_extension GL_Extensions[NUM_OGL_EXTENSIONS] =
{
	// allows for per vertex fog coordinate
	{ false, false, 1, { "GL_EXT_fog_coord" }, 2, {
		"glFogCoordfEXT", "glFogCoordPointerEXT" } },

	// provides multiple texture units for rendering more than one texture in a single pass
	// (NOTE: this was included in OpenGL 1.2.1 standard, but we still need to check for it or require > 1.2 OGL)
	{ true,  false, 1, { "GL_ARB_multitexture" } , 3, {
		"glMultiTexCoord2fARB", "glActiveTextureARB", "glClientActiveTextureARB" } },

	// "ADD" function for texture environment
	{ true,  false, 2, { "GL_ARB_texture_env_add", "GL_EXT_texture_env_add" }, 0, { NULL } },

	// framework for using compressed textures (doesn't provide actual compression formats we use)
	{ false, false, 1, { "GL_ARB_texture_compression" }, 3, {
		"glCompressedTexImage2D", "glCompressedTexSubImage2D", "glGetCompressedTexImageARB" } },

	// S3TC texture compression/decompression support (DXT? encoded DDS images)
	{ false, false, 1, { "GL_EXT_texture_compression_s3tc" }, 0, { NULL } },

	// allows for setting of anisotropic filter (for mipmap texture filtering)
	{ false, false, 1, { "GL_EXT_texture_filter_anisotropic" }, 0, { NULL } },

	// provides app support to control how distance is calculated in fog computations
//	{ false, false, 1, { "GL_NV_fog_distance" }, 0, { NULL } },

	// specify RGB values for secondary color
//	{ false, false, 1, { "GL_EXT_secondary_color" }, 2, {
//		"glSecondaryColor3fvEXT", "glSecondaryColor3ubvEXT" } },

	// "COMBINE" function for texture environment, these two are basically the same
	{ false, false, 2, { "GL_ARB_texture_env_combine", "GL_EXT_texture_env_combine" }, 0, { NULL } },

	// lock a vertex array buffer so that OGL can transform it just once with multiple draws
	{ false, false, 1, { "GL_EXT_compiled_vertex_array" }, 2, {
		"glLockArraysEXT", "glUnlockArraysEXT" } },

	// allows for row major order matrices rather than the standard column major order
//	{ true,  false, 1, { "GL_ARB_transpose_matrix" }, 2, {
//		"glLoadTransposeMatrixfARB", "glMultTransposeMatrixfARB" } },

	// this is obsolete with OGL 1.2, which is why the function name is the standard, but we check for the orginal extension name
	{ true,  false, 1, { "GL_EXT_draw_range_elements" }, 1, {
		"glDrawRangeElements" } },

	// extra texture wrap repeat mode
	{ false, false, 1, { "GL_ARB_texture_mirrored_repeat" }, 0, { NULL } },

	// allows for non-power-of-two textures, but without any cost to normal functionality or usage
	{ false, false, 1, { "GL_ARB_texture_non_power_of_two" }, 0, { NULL } },

	// creates buffer objects (cached in hi-speed video card memory) for vertex data
	{ false, false, 1, { "GL_ARB_vertex_buffer_object" }, 6, {
		"glBindBufferARB", "glDeleteBuffersARB", "glGenBuffersARB", "glBufferDataARB",
		"glMapBufferARB", "glUnmapBufferARB" } },

	// allows pixel data to use buffer objects
	{ false, false, 2, { "GL_ARB_pixel_buffer_object", "GL_EXT_pixel_buffer_object" }, 7, {
		"glBindBufferARB", "glDeleteBuffersARB", "glGenBuffersARB", "glBufferDataARB",
		"glBufferSubDataARB", "glMapBufferARB", "glUnmapBufferARB" } },

	// make me some mipmaps!
	{ false, false, 1, { "GL_SGIS_generate_mipmap" }, 0, { NULL } },

	// framebuffer object gives us render-to-texture support, among other things
	{ false, false, 1, { "GL_EXT_framebuffer_object" }, 16, { 
		"glIsRenderbufferEXT", "glBindRenderbufferEXT", "glDeleteRenderbuffersEXT", "glGenRenderbuffersEXT",
		"glRenderbufferStorageEXT", "glGetRenderbufferParameterivEXT", "glIsFramebufferEXT", "glBindFramebufferEXT",
		"glDeleteFramebuffersEXT", "glGenFramebuffersEXT", "glCheckFramebufferStatusEXT", "glFramebufferTexture2DEXT",
		"glFramebufferRenderbufferEXT", "glGetFramebufferAttachmentParameterivEXT", "glGenerateMipmapEXT", "glDrawBuffers" } },

	// these next three are almost exactly the same, just different stages of naming.
	// allows for non-power-of-textures, but at a cost of functionality
	// (NOTE: the EXT version is usually found only on the Mac)
	{ false, false, 3, { "GL_ARB_texture_rectangle", "GL_EXT_texture_rectangle", "GL_NV_texture_rectangle" }, 0, { NULL } },

	// for BGRA rather than RGBA support (it's faster in most cases)
	{ true,  false, 1, { "GL_EXT_bgra" }, 0, { NULL } },

	// cube map support (for environment maps, normal maps, bump maps, etc.)
	{ false, false, 2, { "GL_ARB_texture_cube_map", "GL_EXT_texture_cube_map" }, 0, { NULL } },

	// apply bias to level-of-detail lamda
	{ false, false, 1, { "GL_EXT_texture_lod_bias" }, 0, { NULL } },

	// point sprites (for particles)
	{ false, false, 2, { "GL_ARB_point_sprite", "NV_point_sprite" }, 0, { NULL } },

	// OpenGL Shading Language support. If this is present, then GL_ARB_shader_objects, GL_ARB_fragment_shader
	// and GL_ARB_vertex_shader should also be available.
	{ false, false, 1, { "GL_ARB_shading_language_100" }, 0, { NULL } },

	// shader objects and program object management
	{ false, false, 1, { "GL_ARB_shader_objects" }, 18, { "glDeleteObjectARB", "glCreateShaderObjectARB", "glShaderSourceARB",
		"glCompileShaderARB", "glGetObjectParameterivARB", "glGetInfoLogARB", "glCreateProgramObjectARB",
		"glAttachObjectARB", "glLinkProgramARB", "glUseProgramObjectARB", "glValidateProgramARB", "glGetUniformLocationARB",
		"glGetUniformivARB", "glUniform1fARB", "glUniform2fARB", "glUniform3fARB", "glUniform1iARB", "glUniformMatrix4fvARB" } },

	// programmable vertex level processing
	// some of functions are provided by GL_ARB_vertex_program
	{ false, false, 1, { "GL_ARB_vertex_shader" }, 4, { "glEnableVertexAttribArrayARB", "glDisableVertexAttribArrayARB",
		"glGetAttribLocationARB", "glVertexAttribPointerARB" } },

	// programmable fragment level processing
	{ false, false, 1, { "GL_ARB_fragment_shader" }, 0, { NULL } },

	// shader version 3.0 detection extensions (if any of these extensions exist then we should have a SM3.0 compatible card, hopefully)
	{ false, false, 3, { "GL_ARB_shader_texture_lod", "GL_NV_vertex_program3", "GL_ATI_shader_texture_lod" }, 0, { NULL } },

	{ false, false, 1, { "GL_ARB_texture_float" }, 0, { NULL } },

	{ false, false, 1, { "GL_ARB_draw_elements_base_vertex" }, 4, { "glDrawElementsBaseVertex", "glDrawRangeElementsBaseVertex", 
		"glDrawElementsInstancedBaseVertex", "glMultiDrawElementsBaseVertex" } }
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
//	{ "glSecondaryColor3fvEXT", 0 },
//	{ "glSecondaryColor3ubvEXT", 0 },
	{ "glLockArraysEXT", 0 },
	{ "glUnlockArraysEXT", 0 },
//	{ "glLoadTransposeMatrixfARB", 0 },
//	{ "glMultTransposeMatrixfARB", 0 },
	{ "glDrawRangeElements", 0 },
	{ "glBindBufferARB", 0 },
	{ "glDeleteBuffersARB", 0 },
	{ "glGenBuffersARB", 0 },
	{ "glBufferDataARB", 0 },
	{ "glBufferSubDataARB", 0 },
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
	{ "glGenerateMipmapEXT", 0 },
	{ "glDeleteObjectARB", 0 },
	{ "glCreateShaderObjectARB", 0 },
	{ "glShaderSourceARB", 0 },
	{ "glCompileShaderARB", 0 },
	{ "glGetObjectParameterivARB", 0 },
	{ "glGetInfoLogARB", 0 },
	{ "glCreateProgramObjectARB", 0 },
	{ "glAttachObjectARB", 0 },
	{ "glLinkProgramARB", 0 },
	{ "glUseProgramObjectARB", 0 },
	{ "glValidateProgramARB", 0 },
	{ "glEnableVertexAttribArrayARB", 0 },
	{ "glDisableVertexAttribArrayARB", 0 },
	{ "glGetAttribLocationARB", 0 },
	{ "glVertexAttribPointerARB", 0 },
	{ "glGetUniformLocationARB", 0 },
	{ "glGetUniformivARB", 0 },
	{ "glUniform1fARB", 0 },
	{ "glUniform2fARB", 0 },
	{ "glUniform3fARB", 0 },
	{ "glUniform4fARB", 0 },
	{ "glUniform3fvARB", 0 },
	{ "glUniform4fvARB", 0 },
	{ "glUniform1iARB", 0 },
	{ "glUniformMatrix4fvARB", 0 },
	{ "glDrawBuffers", 0 },
	{ "glDrawElementsBaseVertex", 0	},
	{ "glDrawRangeElementsBaseVertex", 0 }, 
	{ "glDrawElementsInstancedBaseVertex", 0 },
	{ "glMultiDrawElementsBaseVertex", 0 }
};

// special extensions (only special functions are supported at the moment)
ogl_function GL_EXT_Special[NUM_OGL_EXT_SPECIAL] = {
	{ "wglSwapIntervalEXT", 0 },
	{ "glXSwapIntervalSGI", 0 }
};


#ifdef _WIN32
#define GET_PROC_ADDRESS(x)		wglGetProcAddress((x))
#else
#define GET_PROC_ADDRESS(x)		SDL_GL_GetProcAddress((x))
#endif

//tries to find a certain extension
static inline int opengl_find_extension(const char *ext_to_find)
{
	if (OGL_extension_string == NULL)
		return 0;

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

		func->function_ptr = (ptr_u)GET_PROC_ADDRESS(func->function_name);

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
	int i, j, k, num_found = 0;
	ogl_extension *ext = NULL;
	ogl_function *func = NULL;

	OGL_extension_string = (char*)glGetString(GL_EXTENSIONS);

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

					if ( !func->function_ptr )
						func->function_ptr = (ptr_u)GET_PROC_ADDRESS(func->function_name);

					if ( !func->function_ptr )
						break;
				}

				if ( j != ext->num_functions ) {
					mprintf(("  Found extension \"%s\", but can't find the required function \"%s()\".  Extension will be disabled!\n", ext->extension_name[k], ext->function_names[j]));

					if (ext->required_to_run)
						Error( LOCATION, "The required OpenGL extension '%s' is not fully supported by your current driver version or graphics card.\n", ext->extension_name[k] );
				} else {
					mprintf(("  Using extension \"%s\".\n", ext->extension_name[k]));
					ext->enabled = 1;
					num_found++;
				}
			} else {
				// only report if unable to find when we have checked all available extension name variants
				if ( k+1 >= ext->num_extensions ) {
					mprintf(("  Unable to find extension \"%s\".\n", ext->extension_name[k]));

					if (ext->required_to_run)
						Error( LOCATION, "The required OpenGL extension '%s' is not supported by your current driver version or graphics card.\n", ext->extension_name[k] );
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


extern int OGL_fogmode;

void opengl_extensions_init()
{
	opengl_get_extensions();

	// if S3TC compression is found, then "GL_ARB_texture_compression" must be an extension
	Use_compressed_textures = Is_Extension_Enabled(OGL_EXT_TEXTURE_COMPRESSION_S3TC);
	Texture_compression_available = Is_Extension_Enabled(OGL_ARB_TEXTURE_COMPRESSION);
	int use_base_vertex = Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX);

	//allow VBOs to be used
	if ( !Cmdline_nohtl && !Cmdline_novbo && Is_Extension_Enabled(OGL_ARB_VERTEX_BUFFER_OBJECT) ) {
		Use_VBOs = 1;
	}

	if ( Is_Extension_Enabled(OGL_ARB_PIXEL_BUFFER_OBJECT) ) {
		Use_PBOs = 1;
	}

	// setup the best fog function found
	if ( !Fred_running ) {
		if ( Is_Extension_Enabled(OGL_EXT_FOG_COORD) ) {
			OGL_fogmode = 2;
		} else {
			OGL_fogmode = 1;
		}
	}

	// if we can't do cubemaps then turn off Cmdline_env
	if ( !(Is_Extension_Enabled(OGL_ARB_TEXTURE_CUBE_MAP) && Is_Extension_Enabled(OGL_ARB_TEXTURE_ENV_COMBINE)) ) {
		Cmdline_env = 0;
	}

	if ( !Cmdline_noglsl && Is_Extension_Enabled(OGL_ARB_SHADER_OBJECTS) && Is_Extension_Enabled(OGL_ARB_FRAGMENT_SHADER)
			&& Is_Extension_Enabled(OGL_ARB_VERTEX_SHADER) ) {
		int ver = 0, major = 0, minor = 0;
		const char *glsl_ver = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION_ARB);

		sscanf(glsl_ver, "%d.%d", &major, &minor);
		ver = (major * 100) + minor;

		// SM 4.0 compatible or better
		if (ver >= 400) {
			Use_GLSL = 4;
		}
		// SM 3.0 compatible
		else if ( ver >= 130 ) {
			Use_GLSL = 3;
		}
		// SM 2.0 compatible
		else if (ver >= 120) {
			Use_GLSL = 2;
		}
		// we require GLSL 1.20 or higher
		else if (ver < 110) {
			Use_GLSL = 0;
			mprintf(("  OpenGL Shading Language version %s is not sufficient to use GLSL mode in FSO. Defaulting to fixed-function renderer.\n", glGetString(GL_SHADING_LANGUAGE_VERSION_ARB) ));
#ifdef NDEBUG
			popup(0, 1, POPUP_OK, "GLSL support not available on this GPU. Disabling shader support and defaulting to fixed-function rendering.\n");
#endif
		}
	}

	// can't have this stuff without GLSL support
	if ( !Use_GLSL ) {
		Cmdline_normal = 0;
		Cmdline_height = 0;
		Cmdline_postprocess = 0;
	}

	if (Use_GLSL) {
		GLint max_texture_units;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &max_texture_units);

		// we need enough texture slots for this stuff to work
		
		if (max_texture_units < 6) {
			mprintf(( "Not enough texture units for height map support. We need at least 6, we found %d.\n", max_texture_units ));
			Cmdline_height = 0;
		} else if (max_texture_units < 5) {
			mprintf(( "Not enough texture units for height and normal map support. We need at least 5, we found %d.\n", max_texture_units ));
			Cmdline_normal = 0;
			Cmdline_height = 0;
		} else if (max_texture_units < 4) {
			mprintf(( "Not enough texture units found for GLSL support. We need at least 4, we found %d.\n", max_texture_units ));
			Use_GLSL = 0;
		}
	}
}

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLExtension.h $
 * $Revision: 1.15 $
 * $Date: 2006-08-09 14:42:24 $
 * $Author: taylor $
 *
 * header file to contain the defenitions for the OpenGL exetension
 * functions used in fs2_open
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.14  2006/05/27 17:07:48  taylor
 * remove grd3dparticle.* and grd3dbatch.*, they are obsolete
 * allow us to build without D3D support under Windows (just define NO_DIRECT3D)
 * clean up TMAP flags
 * fix a couple of minor OpenGL state change issues with spec and env map rendering
 * make sure we build again for OS X (OGL extension functions work a little different there)
 * render targets always need to be power-of-2 to avoid incomplete buffer issues in the code
 * when we disable culling in opengl_3dunlit be sure that we re-enable it on exit of function
 * re-fix screenshots
 * add true alpha blending support (with cmdline for now since the artwork has the catch up)
 * draw lines with float positioning, to be more accurate with resizing on non-standard resolutions
 * don't load cubemaps from file for D3D, not sure how to do it anyway
 * update geometry batcher code, memory fixes, dynamic stuff, basic fixage, etc.
 *
 * Revision 1.13  2006/05/13 07:29:52  taylor
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
 * Revision 1.12  2006/04/12 01:10:35  taylor
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
 * Revision 1.11  2006/03/22 18:14:52  taylor
 * if -mipmap is used with -img2dds to then have compressed image also contain mipmaps
 * use nicest hints for texture compression, should improve quality a little
 * when reporting compressed sizes to debug log make ani size be total, not per frame
 *
 * Revision 1.10  2006/02/24 07:35:48  taylor
 * add v-sync support for OGL (I skimmped on this a bit but will go back to do something better, "special" extension wise, at a later date)
 *
 * Revision 1.9  2005/12/28 22:28:44  taylor
 * add support for glCompressedTexSubImage2D(), we don't use it yet but there is nothing wrong with adding it already
 * better support for mipmaps and mipmap filtering
 * add reg option "TextureFilter" to set bilinear or trilinear filter
 * clean up bitmap_id/bitmap_handle/texture_handle madness that made things difficult to understand
 * small fix for using 24-bit images on 16-bit bpp visual (untested)
 *
 * Revision 1.8  2005/12/08 15:10:07  taylor
 * add APPLE_client_storage support to improve texture performance and reduce memory usage a tiny bit on OS X
 *
 * Revision 1.7  2005/12/06 02:50:41  taylor
 * clean up some init stuff and fix a minor SDL annoyance
 * make debug messages a bit more readable
 * clean up the debug console commands for minimize and anisotropic filter setting
 * make anisotropic filter actually work correctly and have it settable with a reg option
 * give opengl_set_arb() the ability to disable all features on all arbs at once so I don't have to everywhere
 *
 * Revision 1.6  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 1.5  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.4  2005/01/21 08:25:14  taylor
 * fill in gr_opengl_set_texture_addressing()
 * add support for non-power-of-two textures for cards that have it
 * temporary crash fix from multiple mipmap levels in uncompressed formats
 *
 * Revision 1.3  2004/10/31 21:45:13  taylor
 * Linux tree merge, single array for VBOs/HTL
 *
 * Revision 1.2  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
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

#ifndef _GROPENGLEXT_H
#define _GROPENGLEXT_H

#include "globalincs/pstypes.h"

//EXTENSIONS!!!!
//be sure to check for this at startup and handle not finding it gracefully

//to add extensions/functions:
//define an index after the last one, either an extension or a function
//increment NUM_OGL_EXTENSIONS if an extension or NUM_OGL_FUNCTIONS if function
//add function macro for Win32
//add function info to GL_Functions struct
//the structure of extensions/functions are located in gropenglextension.cpp

typedef struct ogl_extension {
	/*const*/ int required_to_run;
	int enabled;
	/*const*/ int num_extensions;
	const char *extension_name[3];
	/*const*/ int num_functions;
	const char *function_names[15];
} ogl_extension;

typedef struct ogl_function {
	const char *function_name;
	ptr_u function_ptr;
} ogl_function;

extern ogl_function GL_Functions[];
extern ogl_extension GL_Extensions[];
extern ogl_function GL_EXT_Special[];


// Extensions
#define OGL_EXT_FOG_COORD					0
#define OGL_ARB_MULTITEXTURE				1
#define OGL_ARB_TEXTURE_ENV_ADD				2
#define OGL_ARB_TEXTURE_COMPRESSION			3
#define OGL_EXT_TEXTURE_COMPRESSION_S3TC	4
#define OGL_EXT_TEXTURE_FILTER_ANISOTROPIC	5
#define OGL_NV_FOG_DISTANCE					6
#define OGL_EXT_SECONDARY_COLOR				7
#define OGL_ARB_TEXTURE_ENV_COMBINE			8
#define OGL_EXT_COMPILED_VERTEX_ARRAY		9
#define OGL_ARB_TRANSPOSE_MATRIX			10
#define OGL_EXT_DRAW_RANGE_ELEMENTS			11
#define OGL_ARB_TEXTURE_MIRRORED_REPEAT		12
#define OGL_ARB_TEXTURE_NON_POWER_OF_TWO	13
#define OGL_ARB_VERTEX_BUFFER_OBJECT		14
#define OGL_ARB_PIXEL_BUFFER_OBJECT			15
#define OGL_APPLE_CLIENT_STORAGE			16
#define OGL_SGIS_GENERATE_MIPMAP			17
#define OGL_EXT_FRAMEBUFFER_OBJECT			18
#define OGL_ARB_TEXTURE_RECTANGLE			19
#define OGL_EXT_BGRA						20
#define OGL_ARB_TEXTURE_CUBE_MAP			21
#define OGL_EXT_TEXTURE_LOD_BIAS			22

#define NUM_OGL_EXTENSIONS					23


// Functions
#define OGL_FOG_COORDF						0			// for better looking fog
#define OGL_FOG_COORD_POINTER				1			// used with vertex arrays
#define OGL_MULTI_TEX_COORD_2F				2			// multitex coordinates
#define OGL_ACTIVE_TEXTURE					3			// currenly active multitexture
#define OGL_CLIENT_ACTIVE_TEXTURE			4
#define OGL_COMPRESSED_TEX_IMAGE_2D			5			// 2d compressed texture
#define OGL_COMPRESSED_TEX_SUB_IMAGE_2D		6			// 2d compressed sub texture
#define OGL_GET_COMPRESSED_TEX_IMAGE		7
#define OGL_SECONDARY_COLOR_3FV				8			// for better looking fog
#define OGL_SECONDARY_COLOR_3UBV			9			// specular
#define OGL_LOCK_ARRAYS						10			// HTL
#define OGL_UNLOCK_ARRAYS					11			// HTL
#define OGL_LOAD_TRANSPOSE_MATRIX_F			12			
#define OGL_MULT_TRANSPOSE_MATRIX_F			13
#define OGL_DRAW_RANGE_ELEMENTS				14
#define OGL_BIND_BUFFER						15
#define OGL_DELETE_BUFFERS					16
#define OGL_GEN_BUFFERS						17
#define OGL_BUFFER_DATA						18
#define OGL_MAP_BUFFER						19
#define OGL_UNMAP_BUFFER					20
#define OGL_IS_RENDERBUFFER					21
#define OGL_BIND_RENDERBUFFER				22
#define OGL_DELETE_RENDERBUFFERS			23
#define OGL_GEN_RENDERBUFFERS				24
#define OGL_RENDERBUFFER_STORAGE			25
#define OGL_GET_RENDERBUFFER_PARAMETER_IV	26
#define OGL_IS_FRAMEBUFFER					27
#define OGL_BIND_FRAMEBUFFER				28
#define OGL_DELETE_FRAMEBUFFERS				29
#define OGL_GEN_FRAMEBUFFERS				30
#define OGL_CHECK_FRAMEBUFFER_STATUS		31
#define OGL_FRAMEBUFFER_TEXTURE_2D			32
#define OGL_FRAMEBUFFER_RENDERBUFFER		33
#define OGL_GET_FRAMEBUFFER_ATTACHMENT_PARAMETER_IV		34
#define OGL_GENERATE_MIPMAP					35

#define NUM_OGL_FUNCTIONS					36


// special extensions/functions (OS specific, non-GL stuff)
#define OGL_SPC_WGL_SWAP_INTERVAL		0
#define OGL_SPC_GLX_SWAP_INTERVAL		1

#define NUM_OGL_EXT_SPECIAL				2


#define Is_Extension_Enabled(x)		GL_Extensions[x].enabled

int opengl_get_extensions();
void opengl_print_extensions();


#define GLEXT_CALL(i,x) if (GL_Functions[i].function_ptr)\
							((x)GL_Functions[i].function_ptr)

// the same as GLEXT_CALL() except that it can be used with a cast or in an if statement
// this doesn't do NULL ptr checking so you have to be careful with it!
#define GLEXT_CALL2(i,x) ((x)GL_Functions[i].function_ptr)

#define GLEXT_SPC_CALL(i,x) if (GL_EXT_Special[i].function_ptr)	\
							((x)GL_EXT_Special[i].function_ptr)


#ifdef __APPLE__
// OS X doesn't have the PFN* names so we have to use the real OSX functions, so until
// we move to true runtime loading this will have to suffice...
#define vglFogCoordfEXT					glFogCoordfEXT
#define vglFogCoordPointerEXT			glFogCoordPointerEXT
#define vglMultiTexCoord2fARB			glMultiTexCoord2fARB
#define vglActiveTextureARB				glActiveTextureARB	
#define vglClientActiveTextureARB		glClientActiveTextureARB
#define vglCompressedTexImage2D			glCompressedTexImage2D
#define vglCompressedTexSubImage2D		glCompressedTexSubImage2D
#define vglGetCompressedTexImageARB		glGetCompressedTexImageARB
#define vglSecondaryColor3fvEXT			glSecondaryColor3fvEXT
#define vglSecondaryColor3ubvEXT		glSecondaryColor3ubvEXT
#define vglLockArraysEXT				glLockArraysEXT
#define vglUnlockArraysEXT				glUnlockArraysEXT
#define vglLoadTransposeMatrixfARB		glLoadTransposeMatrixfARB
#define vglMultTransposeMatrixfARB		glMultTransposeMatrixfARB
#define vglDrawRangeElements			glDrawRangeElements
#define vglBindBufferARB				glBindBufferARB
#define vglDeleteBuffersARB				glDeleteBuffersARB
#define vglGenBuffersARB				glGenBuffersARB
#define vglBufferDataARB				glBufferDataARB
#define vglMapBufferARB					glMapBufferARB
#define vglUnmapBufferARB				glUnmapBufferARB
#define vglIsRenderbufferEXT			glIsRenderbufferEXT
#define vglBindRenderbufferEXT			glBindRenderbufferEXT
#define vglDeleteRenderbuffersEXT		glDeleteRenderbuffersEXT
#define vglGenRenderbuffersEXT			glGenRenderbuffersEXT
#define vglRenderbufferStorageEXT		glRenderbufferStorageEXT
#define vglGetRenderbufferParameterivEXT	glGetRenderbufferParameterivEXT
#define vglIsFramebufferEXT				glIsFramebufferEXT
#define vglBindFramebufferEXT			glBindFramebufferEXT
#define vglDeleteFramebuffersEXT		glDeleteFramebuffersEXT
#define vglGenFramebuffersEXT			glGenFramebuffersEXT
#define vglCheckFramebufferStatusEXT	glCheckFramebufferStatusEXT
#define vglFramebufferTexture2DEXT		glFramebufferTexture2DEXT
#define vglFramebufferRenderbufferEXT	glFramebufferRenderbufferEXT
#define vglGetFramebufferAttachmentParameterivEXT	glGetFramebufferAttachmentParameterivEXT
#define vglGenerateMipmapEXT			glGenerateMipmapEXT

#else

#define vglFogCoordfEXT					GLEXT_CALL( OGL_FOG_COORDF, PFNGLFOGCOORDFEXTPROC )
#define vglFogCoordPointerEXT			GLEXT_CALL( OGL_FOG_COORD_POINTER, PFNGLFOGCOORDPOINTEREXTPROC )
#define vglMultiTexCoord2fARB			GLEXT_CALL( OGL_MULTI_TEX_COORD_2F, PFNGLMULTITEXCOORD2FARBPROC )
#define vglActiveTextureARB				GLEXT_CALL( OGL_ACTIVE_TEXTURE, PFNGLACTIVETEXTUREARBPROC )
#define vglClientActiveTextureARB		GLEXT_CALL( OGL_CLIENT_ACTIVE_TEXTURE, PFNGLCLIENTACTIVETEXTUREARBPROC )
#define vglCompressedTexImage2D			GLEXT_CALL( OGL_COMPRESSED_TEX_IMAGE_2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC )
#define vglCompressedTexSubImage2D		GLEXT_CALL( OGL_COMPRESSED_TEX_SUB_IMAGE_2D, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC )
#define vglGetCompressedTexImageARB		GLEXT_CALL( OGL_GET_COMPRESSED_TEX_IMAGE, PFNGLGETCOMPRESSEDTEXIMAGEARBPROC )
#define vglSecondaryColor3fvEXT			GLEXT_CALL( OGL_SECONDARY_COLOR_3FV, PFNGLSECONDARYCOLOR3FVEXTPROC )
#define vglSecondaryColor3ubvEXT		GLEXT_CALL( OGL_SECONDARY_COLOR_3UBV, PFNGLSECONDARYCOLOR3UBVEXTPROC )
#define vglLockArraysEXT				GLEXT_CALL( OGL_LOCK_ARRAYS, PFNGLLOCKARRAYSEXTPROC )
#define vglUnlockArraysEXT				GLEXT_CALL( OGL_UNLOCK_ARRAYS, PFNGLUNLOCKARRAYSEXTPROC )
#define vglLoadTransposeMatrixfARB		GLEXT_CALL( OGL_LOAD_TRANSPOSE_MATRIX_F, PFNGLLOADTRANSPOSEMATRIXFARBPROC )
#define vglMultTransposeMatrixfARB		GLEXT_CALL( OGL_MULT_TRANSPOSE_MATRIX_F, PFNGLMULTTRANSPOSEMATRIXFARBPROC )
#define vglDrawRangeElements			GLEXT_CALL( OGL_DRAW_RANGE_ELEMENTS, PFNGLDRAWRANGEELEMENTSPROC )
#define vglBindBufferARB				GLEXT_CALL( OGL_BIND_BUFFER, PFNGLBINDBUFFERARBPROC )
#define vglDeleteBuffersARB				GLEXT_CALL( OGL_DELETE_BUFFERS, PFNGLDELETEBUFFERSARBPROC )
#define vglGenBuffersARB				GLEXT_CALL( OGL_GEN_BUFFERS, PFNGLGENBUFFERSARBPROC )
#define vglBufferDataARB				GLEXT_CALL( OGL_BUFFER_DATA, PFNGLBUFFERDATAARBPROC )
#define vglMapBufferARB					GLEXT_CALL2( OGL_MAP_BUFFER, PFNGLMAPBUFFERARBPROC )
#define vglUnmapBufferARB				GLEXT_CALL( OGL_UNMAP_BUFFER, PFNGLUNMAPBUFFERARBPROC )
#define vglIsRenderbufferEXT			GLEXT_CALL2( OGL_IS_RENDERBUFFER, PFNGLISRENDERBUFFEREXTPROC )
#define vglBindRenderbufferEXT			GLEXT_CALL( OGL_BIND_RENDERBUFFER, PFNGLBINDRENDERBUFFEREXTPROC )
#define vglDeleteRenderbuffersEXT		GLEXT_CALL( OGL_DELETE_RENDERBUFFERS, PFNGLDELETERENDERBUFFERSEXTPROC )
#define vglGenRenderbuffersEXT			GLEXT_CALL( OGL_GEN_RENDERBUFFERS, PFNGLGENRENDERBUFFERSEXTPROC )
#define vglRenderbufferStorageEXT		GLEXT_CALL( OGL_RENDERBUFFER_STORAGE, PFNGLRENDERBUFFERSTORAGEEXTPROC )
#define vglGetRenderbufferParameterivEXT	GLEXT_CALL( OGL_GET_RENDERBUFFER_PARAMETER_IV, PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC )
#define vglIsFramebufferEXT				GLEXT_CALL2( OGL_IS_FRAMEBUFFER, PFNGLISFRAMEBUFFEREXTPROC )
#define vglBindFramebufferEXT			GLEXT_CALL( OGL_BIND_FRAMEBUFFER, PFNGLBINDFRAMEBUFFEREXTPROC )
#define vglDeleteFramebuffersEXT		GLEXT_CALL( OGL_DELETE_FRAMEBUFFERS, PFNGLDELETEFRAMEBUFFERSEXTPROC )
#define vglGenFramebuffersEXT			GLEXT_CALL( OGL_GEN_FRAMEBUFFERS, PFNGLGENFRAMEBUFFERSEXTPROC )
#define vglCheckFramebufferStatusEXT	GLEXT_CALL2( OGL_CHECK_FRAMEBUFFER_STATUS, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC )
#define vglFramebufferTexture2DEXT		GLEXT_CALL( OGL_FRAMEBUFFER_TEXTURE_2D, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC )
#define vglFramebufferRenderbufferEXT	GLEXT_CALL( OGL_FRAMEBUFFER_RENDERBUFFER, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC )
#define vglGetFramebufferAttachmentParameterivEXT	GLEXT_CALL( OGL_GET_FRAMEBUFFER_ATTACHMENT_PARAMETER_IV, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC )
#define vglGenerateMipmapEXT			GLEXT_CALL( OGL_GENERATE_MIPMAP, PFNGLGENERATEMIPMAPEXTPROC )

#endif	// __APPLE__

// special extensions
#define vwglSwapIntervalEXT			GLEXT_SPC_CALL( OGL_SPC_WGL_SWAP_INTERVAL, PFNWGLSWAPINTERVALEXTPROC )
#define vglXSwapIntervalSGI			GLEXT_SPC_CALL( OGL_SPC_GLX_SWAP_INTERVAL, PFNGLXSWAPINTERVALSGIPROC )

#endif // _GROPENGLEXT_H

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGL.h $
 * $Revision: 2.18.2.5 $
 * $Date: 2007-10-04 16:18:19 $
 * $Author: taylor $
 *
 * Include file for OpenGL renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.18.2.4  2007/04/13 00:31:23  taylor
 * some basic cleanup for the extension code
 * fix the old bug that didn't report a missing extension name properly
 * get rid of the message that says to use D3D, that's not normally going to be an option anyway
 * add ARB_point_sprite support, mostly for fun since no code will actually use it until after 3.6.10
 *
 * Revision 2.18.2.3  2007/02/11 09:35:11  taylor
 * add VALID_FNAME() macro and put it around a few places (more to come)
 * clean out some old variables
 * move CLAMP() macro from opengl header to global header
 * update COUNT_ESTIMATE to match new bmpman changes
 *
 * Revision 2.18.2.2  2006/08/09 14:40:43  taylor
 * fix for setting of texture lod bias
 *
 * Revision 2.18.2.1  2006/06/15 23:33:53  taylor
 * apparently missed this commit a month or so ago with the other OS X fixes
 *
 * Revision 2.18  2006/05/13 07:29:52  taylor
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
 * Revision 2.17  2006/02/24 07:35:48  taylor
 * add v-sync support for OGL (I skimmped on this a bit but will go back to do something better, "special" extension wise, at a later date)
 *
 * Revision 2.16  2006/01/30 06:40:49  taylor
 * better lighting for OpenGL
 * remove some extra stuff that was from sectional bitmaps since we don't need it anymore
 * some basic lighting code cleanup
 *
 * Revision 2.15  2006/01/20 17:15:16  taylor
 * gr_*_bitmap_ex() stuff, D3D side is 100% untested to even compile
 * several other very minor changes as well
 *
 * Revision 2.14  2005/12/16 06:48:28  taylor
 * "House Keeping!!"
 *   - minor cleanup of things that have bothered me at one time or another
 *   - slight speedup from state switching
 *   - slightly better specmap handling, fixes a couple of (not frequent) strange and sorta random issues
 *   - make sure to only disable HTL arb stuff when in HTL mode
 *   - handle any extra lighting pass before spec pass so the light can be applied properly
 *
 * Revision 2.13  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 2.12  2005/08/29 02:20:56  phreak
 * Record state changes in gr_opengl_set_state()
 *
 * Revision 2.11  2005/07/13 03:15:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.10  2005/06/19 02:37:02  taylor
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
 * Revision 2.9  2005/04/24 12:56:42  taylor
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
 * Revision 2.8  2005/03/24 23:42:21  taylor
 * s/gr_ogl_/gr_opengl_/g
 * add empty gr_opengl_draw_line_list() so that it's not a NULL pointer
 * make gr_opengl_draw_htl_sphere() just use GLU so we don't need yet another friggin API
 *
 * Revision 2.7  2005/03/20 00:09:07  phreak
 * Added gr_draw_htl_line and gr_draw_htl sphere
 * There still needs to be D3D versions implemented, but OGL is done.
 * Follow that or ask phreak about how its implemented/
 *
 * Revision 2.6  2005/02/23 05:11:13  taylor
 * more consolidation of various graphics variables
 * some header cleaning
 * only one tmapper_internal for OGL, don't use more than two tex/pass now
 * seperate out 2d matrix mode to allow -2d_poof in OGL and maybe fix missing
 *    interface when set 3d matrix stuff doesn't have corresponding end
 * add dump_frame stuff for OGL, mostly useless but allows trailer recording
 *
 * Revision 2.5  2005/01/01 11:24:23  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 2.4  2004/10/31 21:42:31  taylor
 * Linux tree merge, use linear mag filter, small FRED fix, AA lines (disabled), use rgba colors for 3dunlit, proper gamma adjustment, bmpman merge
 *
 * Revision 2.3  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/04/03 20:27:57  phreak
 * OpenGL files spilt up to make developing and finding bugs much easier
 *
 * Revision 2.1  2003/03/07 00:15:45  phreak
 * added some hacks that shutdown and restore opengl because of cutscenes
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 1     5/12/97 12:14p John
 *
 * $NoKeywords: $
 */

#ifndef _GROPENGL_H
#define _GROPENGL_H



#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#include "graphics/gl/gl.h"
	#include "graphics/gl/glu.h"
	#include "graphics/gl/glext.h"
 	#include "graphics/gl/wglext.h"

	#define STUB_FUNCTION 0
#elif defined(SCP_UNIX)
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/glext.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glext.h>
#endif // __APPLE__
#endif

#include "globalincs/pstypes.h"
#include "graphics/grinternal.h"


// in case it's missing from the GL headers
#ifndef GL_ARB_pixel_buffer_object
#define GL_ARB_pixel_buffer_object	1
#define GL_PIXEL_PACK_BUFFER_ARB			0x88EB
#define GL_PIXEL_UNPACK_BUFFER_ARB			0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING_ARB	0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING_ARB	0x88EF
#endif

#ifndef GL_ARB_texture_rectangle
#define GL_ARB_texture_rectangle	1
#define GL_TEXTURE_RECTANGLE_ARB			0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ARB	0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_ARB		0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB	0x84F8
#endif

#ifndef GL_VERSION_1_4
#define GL_MAX_TEXTURE_LOD_BIAS				0x84FD
#define GL_TEXTURE_FILTER_CONTROL			0x8500
#define GL_TEXTURE_LOD_BIAS					0x8501
#endif

#ifndef GL_VERSION_1_5
#define GL_READ_ONLY						0x88B8
#define GL_WRITE_ONLY						0x88B9
#define GL_READ_WRITE						0x88BA
#define GL_BUFFER_ACCESS					0x88BB
#define GL_BUFFER_MAPPED					0x88BC
#define GL_BUFFER_MAP_POINTER				0x88BD
#define GL_STREAM_DRAW						0x88E0
#define GL_STREAM_READ						0x88E1
#define GL_STREAM_COPY						0x88E2
#define GL_STATIC_DRAW						0x88E4
#define GL_STATIC_READ						0x88E5
#define GL_STATIC_COPY						0x88E6
#define GL_DYNAMIC_DRAW						0x88E8
#define GL_DYNAMIC_READ						0x88E9
#define GL_DYNAMIC_COPY						0x88EA
#define GL_SAMPLES_PASSED					0x8914
#endif

#if !defined(GL_ARB_texture_env_combine) || !defined(GL_ARB_texture_env_add)
#define GL_ARB_texture_env_combine	1
#define GL_ARB_texture_env_add	1
#define GL_COMBINE_ARB						0x8570
#define GL_COMBINE_RGB_ARB					0x8571
#define GL_COMBINE_ALPHA_ARB				0x8572
#define GL_SOURCE0_RGB_ARB					0x8580
#define GL_SOURCE1_RGB_ARB					0x8581
#define GL_SOURCE2_RGB_ARB					0x8582
#define GL_SOURCE0_ALPHA_ARB				0x8588
#define GL_SOURCE1_ALPHA_ARB				0x8589
#define GL_SOURCE2_ALPHA_ARB				0x858A
#define GL_OPERAND0_RGB_ARB					0x8590
#define GL_OPERAND1_RGB_ARB					0x8591
#define GL_OPERAND2_RGB_ARB					0x8592
#define GL_OPERAND0_ALPHA_ARB				0x8598
#define GL_OPERAND1_ALPHA_ARB				0x8599
#define GL_OPERAND2_ALPHA_ARB				0x859A
#define GL_RGB_SCALE_ARB					0x8573
#define GL_ADD_SIGNED_ARB					0x8574
#define GL_INTERPOLATE_ARB					0x8575
#define GL_SUBTRACT_ARB						0x84E7
#define GL_CONSTANT_ARB						0x8576
#define GL_PRIMARY_COLOR_ARB				0x8577
#define GL_PREVIOUS_ARB						0x8578
#endif

#ifndef GL_ARB_point_sprite
#define GL_ARB_point_sprite	1
#define GL_POINT_SPRITE_ARB					0x8861
#define GL_COORD_REPLACE_ARB				0x8862
#endif

#ifndef GL_EXT_framebuffer_object
#define GL_EXT_framebuffer_object	1
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT				0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT						0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT							0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT							0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT			0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT			0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT			0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT	0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT	0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT							0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT			0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT	0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT			0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT				0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT			0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT			0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT						0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS_EXT						0x8CDF
#define GL_COLOR_ATTACHMENT0_EXT							0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT							0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT							0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT							0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT							0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT							0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT							0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT							0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT							0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT							0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT							0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT							0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT							0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT							0x8CED
#define GL_COLOR_ATTACHMENT14_EXT							0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT							0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT								0x8D00
#define GL_STENCIL_ATTACHMENT_EXT							0x8D20
#define GL_FRAMEBUFFER_EXT									0x8D40
#define GL_RENDERBUFFER_EXT									0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT							0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT							0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT					0x8D44
#define GL_STENCIL_INDEX_EXT								0x8D45
#define GL_STENCIL_INDEX1_EXT								0x8D46
#define GL_STENCIL_INDEX4_EXT								0x8D47
#define GL_STENCIL_INDEX8_EXT								0x8D48
#define GL_STENCIL_INDEX16_EXT								0x8D49
#define GL_RENDERBUFFER_RED_SIZE_EXT						0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT						0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT						0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT						0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT						0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT					0x8D55

#ifndef APIENTRYP
#define APIENTRYP	APIENTRY *
#endif

typedef GLboolean (APIENTRYP PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);
#endif	// GL_EXT_framebuffer_object

const ubyte GL_zero_3ub[3] = { 0, 0, 0 };

void gr_opengl_init(int reinit=0);
void gr_opengl_cleanup(int minimize=1);
void opengl_setup_render_states(int &r,int &g,int &b,int &alpha, int &tmap_type, int flags, int is_scaler = 0);
void opengl_set_state(gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt);
//void gr_opengl_bitmap(int x, int y);
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize);
void opengl_set_spec_mapping(int tmap_type, float *u_scale, float *v_scale, int stage = 0 );
void opengl_reset_spec_mapping();
int opengl_check_for_errors();

extern int VBO_ENABLED;

extern gr_texture_source	GL_current_tex_src;
extern gr_alpha_blend		GL_current_alpha_blend;
extern gr_zbuffer_type		GL_current_ztype;

#endif

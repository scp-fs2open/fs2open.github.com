/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
	#define GL_GLEXT_LEGACY // I'd like to punch the idiot that made this needed
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#define GL_GLEXT_FUNCTION_POINTERS	// we need the ptr versions of the functions
	#include <OpenGL/glext.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glext.h>
#endif // __APPLE__
#endif

#include "globalincs/pstypes.h"
#include "graphics/grinternal.h"


#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef APIENTRYP
#define APIENTRYP	APIENTRY *
#endif

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

#ifndef GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_buffer_object	1
#ifdef __APPLE__
typedef long GLintptrARB;
typedef long GLsizeiptrARB;

typedef void (* glBindBufferARBProcPtr) (GLenum target, GLuint buffer);
typedef void (* glDeleteBuffersARBProcPtr) (GLsizei n, const GLuint *buffers);
typedef void (* glGenBuffersARBProcPtr) (GLsizei n, GLuint *buffers);
typedef GLboolean (* glIsBufferARBProcPtr) (GLuint buffer);
typedef void (* glBufferDataARBProcPtr) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void (* glBufferSubDataARBProcPtr) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
typedef void (* glGetBufferSubDataARBProcPtr) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
typedef GLvoid *(* glMapBufferARBProcPtr) (GLenum target, GLenum access);
typedef GLboolean (* glUnmapBufferARBProcPtr) (GLenum target);
typedef void (* glGetBufferParameterivARBProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetBufferPointervARBProcPtr) (GLenum target, GLenum pname, GLvoid **params);
#else
typedef void (APIENTRYP PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef GLboolean (APIENTRYP PFNGLISBUFFERARBPROC) (GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
typedef void (APIENTRYP PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERARBPROC) (GLenum target);
typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
#endif	// __APPLE__
#endif

#ifndef GL_EXT_compiled_vertex_array
#define GL_EXT_compiled_vertex_array	1
#ifdef __APPLE__
typedef void (* glLockArraysEXTProcPtr) (GLint, GLsizei);
typedef void (* glUnlockArraysEXTProcPtr) (void);
#else
typedef void (APIENTRYP PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLUNLOCKARRAYSEXTPROC) (void);
#endif	// __APPLE__
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

#ifdef __APPLE__
typedef GLboolean (* glIsRenderbufferEXTProcPtr) (GLuint renderbuffer);
typedef void (* glBindRenderbufferEXTProcPtr) (GLenum target, GLuint renderbuffer);
typedef void (* glDeleteRenderbuffersEXTProcPtr) (GLsizei n, const GLuint *renderbuffers);
typedef void (* glGenRenderbuffersEXTProcPtr) (GLsizei n, GLuint *renderbuffers);
typedef void (* glRenderbufferStorageEXTProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (* glGetRenderbufferParameterivEXTProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (* glIsFramebufferEXTProcPtr) (GLuint framebuffer);
typedef void (* glBindFramebufferEXTProcPtr) (GLenum target, GLuint framebuffer);
typedef void (* glDeleteFramebuffersEXTProcPtr) (GLsizei n, const GLuint *framebuffers);
typedef void (* glGenFramebuffersEXTProcPtr) (GLsizei n, GLuint *framebuffers);
typedef GLenum (* glCheckFramebufferStatusEXTProcPtr) (GLenum target);
typedef void (* glFramebufferTexture1DEXTProcPtr) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (* glFramebufferTexture2DEXTProcPtr) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (* glFramebufferTexture3DEXTProcPtr) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (* glFramebufferRenderbufferEXTProcPtr) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (* glGetFramebufferAttachmentParameterivEXTProcPtr) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (* glGenerateMipmapEXTProcPtr) (GLenum target);
#else
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
#endif	// __APPLE__
#endif	// GL_EXT_framebuffer_object

#ifndef GL_ARB_shader_objects
#define GL_ARB_shader_objects	1
#define GL_PROGRAM_OBJECT_ARB             0x8B40
#define GL_SHADER_OBJECT_ARB              0x8B48
#define GL_OBJECT_TYPE_ARB                0x8B4E
#define GL_OBJECT_SUBTYPE_ARB             0x8B4F
#define GL_FLOAT_VEC2_ARB                 0x8B50
#define GL_FLOAT_VEC3_ARB                 0x8B51
#define GL_FLOAT_VEC4_ARB                 0x8B52
#define GL_INT_VEC2_ARB                   0x8B53
#define GL_INT_VEC3_ARB                   0x8B54
#define GL_INT_VEC4_ARB                   0x8B55
#define GL_BOOL_ARB                       0x8B56
#define GL_BOOL_VEC2_ARB                  0x8B57
#define GL_BOOL_VEC3_ARB                  0x8B58
#define GL_BOOL_VEC4_ARB                  0x8B59
#define GL_FLOAT_MAT2_ARB                 0x8B5A
#define GL_FLOAT_MAT3_ARB                 0x8B5B
#define GL_FLOAT_MAT4_ARB                 0x8B5C
#define GL_SAMPLER_1D_ARB                 0x8B5D
#define GL_SAMPLER_2D_ARB                 0x8B5E
#define GL_SAMPLER_3D_ARB                 0x8B5F
#define GL_SAMPLER_CUBE_ARB               0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB          0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB          0x8B62
#define GL_SAMPLER_2D_RECT_ARB            0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB     0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB       0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB     0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB     0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB    0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB     0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88

typedef char GLcharARB;		/* native character */
typedef unsigned int GLhandleARB;	/* shader object handle */

#ifdef __APPLE__
typedef void (* glDeleteObjectARBProcPtr) (GLhandleARB obj);
typedef GLhandleARB (* glGetHandleARBProcPtr) (GLenum pname);
typedef void (* glDetachObjectARBProcPtr) (GLhandleARB containerObj, GLhandleARB attachedObj);
typedef GLhandleARB (* glCreateShaderObjectARBProcPtr) (GLenum shaderType);
typedef void (* glShaderSourceARBProcPtr) (GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
typedef void (* glCompileShaderARBProcPtr) (GLhandleARB shaderObj);
typedef GLhandleARB (* glCreateProgramObjectARBProcPtr) (void);
typedef void (* glAttachObjectARBProcPtr) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (* glLinkProgramARBProcPtr) (GLhandleARB programObj);
typedef void (* glUseProgramObjectARBProcPtr) (GLhandleARB programObj);
typedef void (* glValidateProgramARBProcPtr) (GLhandleARB programObj);
typedef void (* glUniform1fARBProcPtr) (GLint location, GLfloat v0);
typedef void (* glUniform2fARBProcPtr) (GLint location, GLfloat v0, GLfloat v1);
typedef void (* glUniform3fARBProcPtr) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (* glUniform4fARBProcPtr) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (* glUniform1iARBProcPtr) (GLint location, GLint v0);
typedef void (* glUniform2iARBProcPtr) (GLint location, GLint v0, GLint v1);
typedef void (* glUniform3iARBProcPtr) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (* glUniform4iARBProcPtr) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (* glUniform1fvARBProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform2fvARBProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform3fvARBProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform4fvARBProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform1ivARBProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform2ivARBProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform3ivARBProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform4ivARBProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniformMatrix2fvARBProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix3fvARBProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix4fvARBProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glGetObjectParameterfvARBProcPtr) (GLhandleARB obj, GLenum pname, GLfloat *params);
typedef void (* glGetObjectParameterivARBProcPtr) (GLhandleARB obj, GLenum pname, GLint *params);
typedef void (* glGetInfoLogARBProcPtr) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (* glGetAttachedObjectsARBProcPtr) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
typedef GLint (* glGetUniformLocationARBProcPtr) (GLhandleARB programObj, const GLcharARB *name);
typedef void (* glGetActiveUniformARBProcPtr) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef void (* glGetUniformfvARBProcPtr) (GLhandleARB programObj, GLint location, GLfloat *params);
typedef void (* glGetUniformivARBProcPtr) (GLhandleARB programObj, GLint location, GLint *params);
typedef void (* glGetShaderSourceARBProcPtr) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
#else
typedef void (APIENTRYP PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
typedef GLhandleARB (APIENTRYP PFNGLGETHANDLEARBPROC) (GLenum pname);
typedef void (APIENTRYP PFNGLDETACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB attachedObj);
typedef GLhandleARB (APIENTRYP PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (APIENTRYP PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void (APIENTRYP PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRYP PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef void (APIENTRYP PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRYP PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (APIENTRYP PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef void (APIENTRYP PFNGLVALIDATEPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (APIENTRYP PFNGLUNIFORM1FARBPROC) (GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORM2FARBPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRYP PFNGLUNIFORM3FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRYP PFNGLUNIFORM4FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRYP PFNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORM2IARBPROC) (GLint location, GLint v0, GLint v1);
typedef void (APIENTRYP PFNGLUNIFORM3IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRYP PFNGLUNIFORM4IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRYP PFNGLUNIFORM1FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM2FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM3FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM4FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM1IVARBPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM2IVARBPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM3IVARBPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM4IVARBPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLGETOBJECTPARAMETERFVARBPROC) (GLhandleARB obj, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (APIENTRYP PFNGLGETATTACHEDOBJECTSARBPROC) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef void (APIENTRYP PFNGLGETUNIFORMFVARBPROC) (GLhandleARB programObj, GLint location, GLfloat *params);
typedef void (APIENTRYP PFNGLGETUNIFORMIVARBPROC) (GLhandleARB programObj, GLint location, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERSOURCEARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
#endif	// __APPLE__
#endif	// GL_ARB_shader_objects

#ifndef GL_ARB_vertex_shader
#define GL_ARB_vertex_shader	1
#define GL_VERTEX_SHADER_ARB              0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB 0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB         0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB 0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB   0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB 0x8B8A

#ifdef __APPLE__
typedef void (* glBindAttribLocationARBProcPtr) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
typedef void (* glGetActiveAttribARBProcPtr) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLint (* glGetAttribLocationARBProcPtr) (GLhandleARB programObj, const GLcharARB *name);
#else
typedef void (APIENTRYP PFNGLBINDATTRIBLOCATIONARBPROC) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
typedef void (APIENTRYP PFNGLGETACTIVEATTRIBARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLint (APIENTRYP PFNGLGETATTRIBLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
#endif	// __APPLE__
#endif	// GL_ARB_vertex_shader

#ifndef GL_ARB_fragment_shader
#define GL_ARB_fragment_shader	1
#define GL_FRAGMENT_SHADER_ARB            0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB 0x8B8B
#endif	// GL_ARB_fragment_shader

#ifndef GL_ARB_shading_language_100
#define GL_ARB_shading_language_100	1
#define GL_SHADING_LANGUAGE_VERSION_ARB   0x8B8C
#endif	// GL_ARB_shading_language_100

// as usual, OS X headers completely suck
#ifndef GL_SHADING_LANGUAGE_VERSION_ARB
#define GL_SHADING_LANGUAGE_VERSION_ARB   0x8B8C
#endif

// most of following is probably useless, but some of it is required by GL_ARB_*_shader
#ifndef GL_ARB_vertex_program
#define GL_ARB_vertex_program 1
#define GL_COLOR_SUM_ARB                  0x8458
#define GL_VERTEX_PROGRAM_ARB             0x8620
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB   0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB   0x8625
#define GL_CURRENT_VERTEX_ATTRIB_ARB      0x8626
#define GL_PROGRAM_LENGTH_ARB             0x8627
#define GL_PROGRAM_STRING_ARB             0x8628
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB 0x862E
#define GL_MAX_PROGRAM_MATRICES_ARB       0x862F
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB 0x8640
#define GL_CURRENT_MATRIX_ARB             0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB  0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB    0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB 0x8645
#define GL_PROGRAM_ERROR_POSITION_ARB     0x864B
#define GL_PROGRAM_BINDING_ARB            0x8677
#define GL_MAX_VERTEX_ATTRIBS_ARB         0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB 0x886A
#define GL_PROGRAM_ERROR_STRING_ARB       0x8874
#define GL_PROGRAM_FORMAT_ASCII_ARB       0x8875
#define GL_PROGRAM_FORMAT_ARB             0x8876
#define GL_PROGRAM_INSTRUCTIONS_ARB       0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB   0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB        0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB    0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A7
#define GL_PROGRAM_PARAMETERS_ARB         0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB     0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB  0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AB
#define GL_PROGRAM_ATTRIBS_ARB            0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB        0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB     0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB  0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB 0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB 0x88B6
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB   0x88B7
#define GL_MATRIX0_ARB                    0x88C0
#define GL_MATRIX1_ARB                    0x88C1
#define GL_MATRIX2_ARB                    0x88C2
#define GL_MATRIX3_ARB                    0x88C3
#define GL_MATRIX4_ARB                    0x88C4
#define GL_MATRIX5_ARB                    0x88C5
#define GL_MATRIX6_ARB                    0x88C6
#define GL_MATRIX7_ARB                    0x88C7
#define GL_MATRIX8_ARB                    0x88C8
#define GL_MATRIX9_ARB                    0x88C9
#define GL_MATRIX10_ARB                   0x88CA
#define GL_MATRIX11_ARB                   0x88CB
#define GL_MATRIX12_ARB                   0x88CC
#define GL_MATRIX13_ARB                   0x88CD
#define GL_MATRIX14_ARB                   0x88CE
#define GL_MATRIX15_ARB                   0x88CF
#define GL_MATRIX16_ARB                   0x88D0
#define GL_MATRIX17_ARB                   0x88D1
#define GL_MATRIX18_ARB                   0x88D2
#define GL_MATRIX19_ARB                   0x88D3
#define GL_MATRIX20_ARB                   0x88D4
#define GL_MATRIX21_ARB                   0x88D5
#define GL_MATRIX22_ARB                   0x88D6
#define GL_MATRIX23_ARB                   0x88D7
#define GL_MATRIX24_ARB                   0x88D8
#define GL_MATRIX25_ARB                   0x88D9
#define GL_MATRIX26_ARB                   0x88DA
#define GL_MATRIX27_ARB                   0x88DB
#define GL_MATRIX28_ARB                   0x88DC
#define GL_MATRIX29_ARB                   0x88DD
#define GL_MATRIX30_ARB                   0x88DE
#define GL_MATRIX31_ARB                   0x88DF

#ifdef __APPLE__
typedef void (* glVertexAttrib1dARBProcPtr) (GLuint index, GLdouble x);
typedef void (* glVertexAttrib1dvARBProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib1fARBProcPtr) (GLuint index, GLfloat x);
typedef void (* glVertexAttrib1fvARBProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib1sARBProcPtr) (GLuint index, GLshort x);
typedef void (* glVertexAttrib1svARBProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib2dARBProcPtr) (GLuint index, GLdouble x, GLdouble y);
typedef void (* glVertexAttrib2dvARBProcPtr) (GLuint index, const GLdouble *v); 
typedef void (* glVertexAttrib2fARBProcPtr) (GLuint index, GLfloat x, GLfloat y);
typedef void (* glVertexAttrib2fvARBProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib2sARBProcPtr) (GLuint index, GLshort x, GLshort y);
typedef void (* glVertexAttrib2svARBProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib3dARBProcPtr) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (* glVertexAttrib3dvARBProcPtr) (GLuint index, const GLdouble *v); 
typedef void (* glVertexAttrib3fARBProcPtr) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (* glVertexAttrib3fvARBProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib3sARBProcPtr) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (* glVertexAttrib3svARBProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4NbvARBProcPtr) (GLuint index, const GLbyte *v);
typedef void (* glVertexAttrib4NivARBProcPtr) (GLuint index, const GLint *v);
typedef void (* glVertexAttrib4NsvARBProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4NubARBProcPtr) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (* glVertexAttrib4NubvARBProcPtr) (GLuint index, const GLubyte *v);
typedef void (* glVertexAttrib4NuivARBProcPtr) (GLuint index, const GLuint *v);
typedef void (* glVertexAttrib4NusvARBProcPtr) (GLuint index, const GLushort *v);
typedef void (* glVertexAttrib4bvARBProcPtr) (GLuint index, const GLbyte *v);
typedef void (* glVertexAttrib4dARBProcPtr) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glVertexAttrib4dvARBProcPtr) (GLuint index, const GLdouble *v); 
typedef void (* glVertexAttrib4fARBProcPtr) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glVertexAttrib4fvARBProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib4ivARBProcPtr) (GLuint index, const GLint *v);
typedef void (* glVertexAttrib4sARBProcPtr) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (* glVertexAttrib4svARBProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4ubvARBProcPtr) (GLuint index, const GLubyte *v);
typedef void (* glVertexAttrib4uivARBProcPtr) (GLuint index, const GLuint *v);
typedef void (* glVertexAttrib4usvARBProcPtr) (GLuint index, const GLushort *v);
typedef void (* glVertexAttribPointerARBProcPtr) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (* glDisableVertexAttribArrayARBProcPtr) (GLuint index);
typedef void (* glEnableVertexAttribArrayARBProcPtr) (GLuint index);
typedef void (* glGetVertexAttribPointervARBProcPtr) (GLuint index, GLenum pname, GLvoid **pointer);
typedef void (* glGetVertexAttribdvARBProcPtr) (GLuint index, GLenum pname, GLdouble *params);
typedef void (* glGetVertexAttribfvARBProcPtr) (GLuint index, GLenum pname, GLfloat *params);
typedef void (* glGetVertexAttribivARBProcPtr) (GLuint index, GLenum pname, GLint *params);
typedef void (* glBindProgramARBProcPtr) (GLenum target, GLuint program);
typedef void (* glDeleteProgramsARBProcPtr) (GLsizei n, const GLuint *programs);
typedef void (* glGenProgramsARBProcPtr) (GLsizei n, GLuint *programs);
typedef GLboolean (* glIsProgramARBProcPtr) (GLuint program);
typedef void (* glProgramEnvParameter4dARBProcPtr) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glProgramEnvParameter4dvARBProcPtr) (GLenum target, GLuint index, const GLdouble *params);
typedef void (* glProgramEnvParameter4fARBProcPtr) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glProgramEnvParameter4fvARBProcPtr) (GLenum target, GLuint index, const GLfloat *params);
typedef void (* glProgramLocalParameter4dARBProcPtr) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glProgramLocalParameter4dvARBProcPtr) (GLenum target, GLuint index, const GLdouble *params);
typedef void (* glProgramLocalParameter4fARBProcPtr) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glProgramLocalParameter4fvARBProcPtr) (GLenum target, GLuint index, const GLfloat *params);
typedef void (* glGetProgramEnvParameterdvARBProcPtr) (GLenum target, GLuint index, GLdouble *params);
typedef void (* glGetProgramEnvParameterfvARBProcPtr) (GLenum target, GLuint index, GLfloat *params);
typedef void (* glGetProgramLocalParameterdvARBProcPtr) (GLenum target, GLuint index, GLdouble *params);
typedef void (* glGetProgramLocalParameterfvARBProcPtr) (GLenum target, GLuint index, GLfloat *params);
typedef void (* glProgramStringARBProcPtr) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (* glGetProgramStringARBProcPtr) (GLenum target, GLenum pname, GLvoid *string);
typedef void (* glGetProgramivARBProcPtr) (GLenum target, GLenum pname, GLint *params);
#else
typedef void (APIENTRYP PFNGLVERTEXATTRIB1DARBPROC) (GLuint index, GLdouble x);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1FARBPROC) (GLuint index, GLfloat x);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1SARBPROC) (GLuint index, GLshort x);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2DARBPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2SARBPROC) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NBVARBPROC) (GLuint index, const GLbyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NIVARBPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NSVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUBARBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUBVARBPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUIVARBPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUSVARBPROC) (GLuint index, const GLushort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4BVARBPROC) (GLuint index, const GLbyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4IVARBPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4SVARBPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4UBVARBPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4UIVARBPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4USVARBPROC) (GLuint index, const GLushort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
typedef void (APIENTRYP PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (APIENTRYP PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (APIENTRYP PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRYP PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRYP PFNGLPROGRAMENVPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLPROGRAMENVPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRYP PFNGLPROGRAMENVPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRYP PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRYP PFNGLPROGRAMLOCALPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLPROGRAMLOCALPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRYP PFNGLPROGRAMLOCALPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRYP PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRYP PFNGLGETPROGRAMENVPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRYP PFNGLGETPROGRAMENVPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
typedef void (APIENTRYP PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRYP PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
typedef void (APIENTRYP PFNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMSTRINGARBPROC) (GLenum target, GLenum pname, GLvoid *string);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBDVARBPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBFVARBPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBIVARBPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBPOINTERVARBPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
typedef GLboolean (APIENTRYP PFNGLISPROGRAMARBPROC) (GLuint program);
#endif	// __APPLE__
#endif	// GL_ARB_vertex_program


const ubyte GL_zero_3ub[3] = { 0, 0, 0 };

bool gr_opengl_init();
void gr_opengl_cleanup(int minimize=1);
int opengl_check_for_errors(char *err_at = NULL);

#ifndef NDEBUG
#define GL_CHECK_FOR_ERRORS(s)	opengl_check_for_errors((s))
#else
#define GL_CHECK_FOR_ERRORS(s)
#endif

extern int GL_version;

extern int Use_VBOs;
extern int Use_PBOs;
extern int Use_GLSL;

#endif

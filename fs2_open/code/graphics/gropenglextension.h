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
 * $Revision: 1.4 $
 * $Date: 2005-01-21 08:25:14 $
 * $Author: taylor $
 *
 * header file to contain the defenitions for the OpenGL exetension
 * functions used in fs2_open
 *
 * $Log: not supported by cvs2svn $
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

#include "PreProcDefines.h"
#ifndef _GROPENGLEXT_H
#define _GROPENGLEXT_H

#include "globalincs/pstypes.h"

//EXTENSIONS!!!!
//be sure to check for this at startup and handle not finding it gracefully

//to add extensions:
//define an index after the last one
//increment GL_NUM_EXTENSIONS
//add function macro
//add function info to GL_Extensions struct
//the structure of extensions are located in gropenglextension.cpp
typedef struct ogl_extension
{
	int enabled;					//is this extension enabled
	ptr_u func_pointer;				//address of function
	const char* function_name;		//name passed to wglGetProcAddress()
	const char* extension_name;		//name found in extension string
	int required_to_run;			//is this extension required for use	
} ogl_extension;

extern ogl_extension GL_Extensions[];

/*#define GL_FOG_COORD_EXT				0
#define GL_MULTITEXTURE_ARB				1
#define GL_TEXTURE_ENV_ADD_ARB			2		// additive texture environment
#define GL_COMP_TEX_ARB					3		// texture compression
#define GL_TEX_COMP_S3TC_EXT			4		// S3TC/DXTC compression format
#define GL_TEX_FILTER_ANSIO				5		// anisotrophic filtering
#define GL_NV_RADIAL_FOG				6		// for better looking fog
#define GL_SECONDARY_COLOR				7
#define GL_ARB_ENV_COMBINE				8			// spec mapping
#define GL_EXT_ENV_COMBINE				9			// spec mapping
#define GL_COMPILED_VERTEX_ARR			10
#define GL_TRANSPOSE_MATRIX				11
#define GL_VERTEX_BUFFER_OBJECT			12
#define GL_NUM_EXTENSIONS				13*/

#define GL_FOG_COORDF					0			// for better looking fog
#define GL_FOG_COORD_POINTER			1			// used with vertex arrays
#define GL_MULTITEXTURE_COORD2F			2			// multitex coordinates
#define GL_ACTIVE_TEX					3			// currenly active multitexture
#define GL_TEXTURE_ENV_ADD				4			// additive texture environment
#define GL_COMP_TEX						5			// texture compression
#define GL_TEX_COMP_S3TC				6			// S3TC/DXTC compression format
#define GL_TEX_FILTER_ANSIO				7			// anisotrophic filtering
#define GL_NV_RADIAL_FOG				8			// for better looking fog
#define GL_SECONDARY_COLOR_3FV			9			// for better looking fog
#define GL_SECONDARY_COLOR_3UBV			10			// specular
#define GL_ARB_ENV_COMBINE				11			// spec mapping
#define GL_EXT_ENV_COMBINE				12			// spec mapping
#define GL_LOCK_ARRAYS					13			// HTL
#define GL_UNLOCK_ARRAYS				14			// HTL
#define GL_LOAD_TRANSPOSE				15			
#define GL_MULT_TRANSPOSE				16
#define GL_CLIENT_ACTIVE_TEX			17
#define GL_DRAW_RANGE_ELEMENTS			18
#define GL_ARB_TEXTURE_MIRRORED_REPEAT	19
#define GL_ARB_TEXTURE_NON_POWER_OF_TWO	20

//GL_ARB_vertex_buffer_object FUNCTIONS
#define GL_ARB_VBO_BIND_BUFFER			21
#define GL_ARB_VBO_DEL_BUFFER			22
#define GL_ARB_VBO_GEN_BUFFER			23
#define GL_ARB_VBO_BUFFER_DATA			24
//#define GL_ARB_VBO_MAP_BUFFER			23
//#define GL_ARB_VBO_UNMAP_BUFFER			24


#define GL_NUM_EXTENSIONS				25


int opengl_get_extensions();
void opengl_print_extensions();
int opengl_extension_is_enabled(int idx);

#ifdef _WIN32
#define GLEXT_CALL(x,i) if (GL_Extensions[i].enabled)\
							((x)GL_Extensions[i].func_pointer)

//void glFogCoordfEXT(float value);
//void glFogCoordPointerEXT(unsigned int, unsigned int, void*);
//void glMultiTexCoord2fARB(unsigned int tex_unit, float u_coord, float v_coord);
//void glActiveTextureARB(unsigned int tex_unit);
//void CompressedTexImage2DARB(unsigned int target, int level, unsigned int internalformat, unsigned int width, unsigned int height, int border, unsigned int sizeinbytes, const void *data);
//void glSecondaryColor3fvEXT(float* v);
//void glSecondaryColor3ubEXT(unsigned char* v);
//void glLockArraysEXT(int first, int count);
//void glUnlockArraysEXT();
//void glLoadTransposeMatrixfARB(unsigned int type, float *m);			 
//void glBindBufferARB(unsigned int count, int* buffers);

#define glFogCoordfEXT GLEXT_CALL(PFNGLFOGCOORDFEXTPROC, GL_FOG_COORDF)
#define glFogCoordPointerEXT GLEXT_CALL(PFNGLFOGCOORDPOINTEREXTPROC, GL_FOG_COORD_POINTER);
#define glMultiTexCoord2fARB GLEXT_CALL(PFNGLMULTITEXCOORD2FARBPROC, GL_MULTITEXTURE_COORD2F)
#define glActiveTextureARB GLEXT_CALL(PFNGLACTIVETEXTUREARBPROC, GL_ACTIVE_TEX)
#define glCompressedTexImage2D GLEXT_CALL(PFNGLCOMPRESSEDTEXIMAGE2DPROC, GL_COMP_TEX)
#define glSecondaryColor3fvEXT GLEXT_CALL(PFNGLSECONDARYCOLOR3FVEXTPROC, GL_SECONDARY_COLOR_3FV)
#define glSecondaryColor3ubvEXT GLEXT_CALL(PFNGLSECONDARYCOLOR3UBVEXTPROC, GL_SECONDARY_COLOR_3UBV)
#define glLockArraysEXT GLEXT_CALL(PFNGLLOCKARRAYSEXTPROC, GL_LOCK_ARRAYS)
#define glUnlockArraysEXT GLEXT_CALL(PFNGLUNLOCKARRAYSEXTPROC, GL_UNLOCK_ARRAYS)
#define glLoadTransposeMatrixfARB GLEXT_CALL(PFNGLLOADTRANSPOSEMATRIXFARBPROC, GL_LOAD_TRANSPOSE)
#define glMultTransposeMatrixfARB GLEXT_CALL(PFNGLMULTTRANSPOSEMATRIXFARBPROC, GL_MULT_TRANSPOSE)
#define glClientActiveTextureARB GLEXT_CALL(PFNGLCLIENTACTIVETEXTUREARBPROC, GL_CLIENT_ACTIVE_TEX)
#define glBindBufferARB GLEXT_CALL(PFNGLBINDBUFFERARBPROC, GL_ARB_VBO_BIND_BUFFER)
#define glDeleteBuffersARB GLEXT_CALL(PFNGLDELETEBUFFERSARBPROC, GL_ARB_VBO_DEL_BUFFER)
#define glGenBuffersARB GLEXT_CALL(PFNGLGENBUFFERSARBPROC, GL_ARB_VBO_GEN_BUFFER)
#define glBufferDataARB GLEXT_CALL(PFNGLBUFFERDATAARBPROC, GL_ARB_VBO_BUFFER_DATA)
#define glDrawRangeElements GLEXT_CALL(PFNGLDRAWRANGEELEMENTSPROC, GL_DRAW_RANGE_ELEMENTS)
#endif // _WIN32

#endif // _GROPENGLEXT_H

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
 * $Revision: 2.2 $
 * $Date: 2004-04-06 01:37:21 $
 * $Author: phreak $
 *
 * source for extension implementation in OpenGL
 *
 *
 * $NoKeywords: $
 */

#include <windows.h>

#include "globalincs/pstypes.h"
#include "graphics/gl/gl.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"

#include "osapi/outwnd.h"


char *OGL_extension_string;
/*
typedef struct ogl_extension
{
	int enabled;					//is this extension enabled
	const char* extension_name;		//name found in extension string
	int required_to_run;			//is this extension required for use	
} ogl_extension;

ogl_extension GL_Extensions[GL_NUM_EXTENSIONS]=
{
	{0, "GL_EXT_fog_coord",0},
	{0, "GL_ARB_multitexture",1},		//required for glow maps
	{0, "GL_ARB_texture_env_add", 1},					//required for glow maps
	{0, "GL_ARB_texture_compression",0},
	{0, "GL_EXT_texture_compression_s3tc",0},
	{0, "GL_EXT_texture_filter_anisotropic", 0},
	{0, "GL_NV_fog_distance", 0},
	{0, "GL_EXT_secondary_color", 0},
	{0, "GL_ARB_texture_env_combine",0},
	{0, "GL_EXT_texture_env_combine",0},
	{0, "GL_EXT_compiled_vertex_array",0},
	{0, "GL_ARB_transpose_matrix",1},
	{0, "GL_ARB_vertex_buffer_object",0}
};*/

ogl_extension GL_Extensions[GL_NUM_EXTENSIONS]=
{
	{0, NULL, "glFogCoordfEXT", "GL_EXT_fog_coord",0},
	{0, NULL, "glFogCoordPointerEXT", "GL_EXT_fog_coord",0},
	{0, NULL, "glMultiTexCoord2fARB", "GL_ARB_multitexture",1},		//required for glow maps
	{0, NULL, "glActiveTextureARB", "GL_ARB_multitexture",1},		//required for glow maps
	{0, NULL, NULL, "GL_ARB_texture_env_add", 1},					//required for glow maps
	{0, NULL, "glCompressedTexImage2D", "GL_ARB_texture_compression",0},
	{0, NULL, NULL, "GL_EXT_texture_compression_s3tc",0},
	{0, NULL, NULL, "GL_EXT_texture_filter_anisotropic", 0},
	{0, NULL, NULL, "GL_NV_fog_distance", 0},
	{0, NULL, "glSecondaryColor3fvEXT", "GL_EXT_secondary_color", 0},
	{0, NULL, "glSecondaryColor3ubvEXT", "GL_EXT_secondary_color", 0},
	{0, NULL, NULL, "GL_ARB_texture_env_combine",0},
	{0, NULL, NULL, "GL_EXT_texture_env_combine",0},
	{0, NULL, "glLockArraysEXT", "GL_EXT_compiled_vertex_array",0},
	{0, NULL, "glUnlockArraysEXT", "GL_EXT_compiled_vertex_array",0},
	{0, NULL,"glLoadTransposeMatrixfARB","GL_ARB_transpose_matrix",	1},
	{0, NULL, "glMultTransposeMatrixfARB", "GL_ARB_transpose_matrix",1},
	{0, NULL, "glClientActiveTextureARB", "GL_ARB_multitexture",1},
	{0, NULL, "glBindBufferARB", "GL_ARB_vertex_buffer_object",0},
	{0, NULL, "glDeleteBuffersARB", "GL_ARB_vertex_buffer_object",0},
	{0, NULL, "glGenBuffersARB", "GL_ARB_vertex_buffer_object",0},
	{0, NULL, "glBufferDataARB", "GL_ARB_vertex_buffer_object",0}

};

//tries to find a certain extension
static inline int opengl_find_extension(const char* ext_to_find)
{
	return (strstr(OGL_extension_string,ext_to_find)!=NULL);
}

void opengl_print_extensions()
{
	//print out extensions
	const char*OGL_extensions=(const char*)glGetString(GL_EXTENSIONS);
	char *extlist;
	char *curext;

	extlist=(char*)malloc(strlen(OGL_extensions));
	memcpy(extlist, OGL_extensions, strlen(OGL_extensions));
	
	curext=strtok(extlist, " ");
	while (curext)
	{
		mprintf(( "%s\n", curext ));
		curext=strtok(NULL, " ");
	}
	free(extlist);
}

int opengl_extension_is_enabled(int idx)
{
	if ((idx < 0) || (idx >= GL_NUM_EXTENSIONS)) return 0;
	return GL_Extensions[idx].enabled;
}

//finds OGL extension functions
//returns number found
int opengl_get_extensions()
{
	OGL_extension_string = (char*)glGetString(GL_EXTENSIONS);
	int num_found=0;
	ogl_extension *cur=NULL;

	for (int i=0; i < GL_NUM_EXTENSIONS; i++)
	{
		cur=&GL_Extensions[i];
		if (opengl_find_extension(cur->extension_name))
		{
			//some extensions do not have functions
			if (cur->function_name==NULL)
			{
				mprintf(("found extension %s\n", cur->extension_name));
				cur->enabled=1;
				num_found++;
				continue;
			}
			
			cur->func_pointer=(uint)wglGetProcAddress(cur->function_name);
			if (cur->func_pointer)
			{
				cur->enabled=1;
				mprintf(("found extension function: %s -- extension: %s\n", cur->function_name, cur->extension_name));
				num_found++;
			}
			else
			{
				mprintf(("found extension, but not function: %s -- extension:%s\n", cur->function_name, cur->extension_name));
			}
		}
		else
		{
			mprintf(("did not find extension: %s\n", cur->extension_name));
			if (cur->required_to_run)
			{
				Error(__FILE__,__LINE__,"The required OpenGL extension %s is not supported by your graphics card, please use the Glide or Direct3D rendering engines.\n\n",cur->extension_name);
			}
		}
	}
	return num_found;
}

/*
//finds OGL extension functions
//returns number found
int opengl_get_extensions()
{
	int num_found=0;
	ogl_extension *cur=NULL;
	OGL_extension_string=(char*)glGetString(GL_EXTENSIONS);

	for (int i=0; i < GL_NUM_EXTENSIONS; i++)
	{
		cur=&GL_Extensions[i];
		if (opengl_find_extension(cur->extension_name))
		{
			num_found++;
			cur->enabled = 1;
		}
		else
		{
			mprintf(("did not find extension: %s\n", cur->extension_name));
			if (cur->required_to_run)
			{
				Error(__FILE__,__LINE__,"The required OpenGL extension %s is not supported by your graphics card, please use the Glide or Direct3D rendering engines.\n\n",cur->extension_name);
			}
		}
	}
	return num_found;
}

*/
//Extension Implementation Functions
//ifdef this out if compiling for linux
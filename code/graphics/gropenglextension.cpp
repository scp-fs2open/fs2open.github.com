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
 * $Revision: 1.4 $
 * $Date: 2004-10-31 21:45:13 $
 * $Author: taylor $
 *
 * source for extension implementation in OpenGL
 *
 * $Log: not supported by cvs2svn $
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
	{0, 0, "glFogCoordfEXT", "GL_EXT_fog_coord",0},
	{0, 0, "glFogCoordPointerEXT", "GL_EXT_fog_coord",0},
	{0, 0, "glMultiTexCoord2fARB", "GL_ARB_multitexture",1},		//required for glow maps
	{0, 0, "glActiveTextureARB", "GL_ARB_multitexture",1},		//required for glow maps
	{0, 0, NULL, "GL_ARB_texture_env_add", 1},					//required for glow maps
	{0, 0, "glCompressedTexImage2D", "GL_ARB_texture_compression",0},
	{0, 0, NULL, "GL_EXT_texture_compression_s3tc",0},
	{0, 0, NULL, "GL_EXT_texture_filter_anisotropic", 0},
	{0, 0, NULL, "GL_NV_fog_distance", 0},
	{0, 0, "glSecondaryColor3fvEXT", "GL_EXT_secondary_color", 0},
	{0, 0, "glSecondaryColor3ubvEXT", "GL_EXT_secondary_color", 0},
	{0, 0, NULL, "GL_ARB_texture_env_combine",0},
	{0, 0, NULL, "GL_EXT_texture_env_combine",0},
	{0, 0, "glLockArraysEXT", "GL_EXT_compiled_vertex_array",0},
	{0, 0, "glUnlockArraysEXT", "GL_EXT_compiled_vertex_array",0},
	{0, 0,"glLoadTransposeMatrixfARB","GL_ARB_transpose_matrix",	1},
	{0, 0, "glMultTransposeMatrixfARB", "GL_ARB_transpose_matrix",1},
	{0, 0, "glClientActiveTextureARB", "GL_ARB_multitexture",1},
	{0, 0, "glDrawRangeElements", "GL_EXT_draw_range_elements", 1},
	{0, 0, "glBindBufferARB", "GL_ARB_vertex_buffer_object",0},
	{0, 0, "glDeleteBuffersARB", "GL_ARB_vertex_buffer_object",0},
	{0, 0, "glGenBuffersARB", "GL_ARB_vertex_buffer_object",0},
	{0, 0, "glBufferDataARB", "GL_ARB_vertex_buffer_object",0},

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
			
#ifdef _WIN32
			cur->func_pointer=(ptr_u)wglGetProcAddress(cur->function_name);
#else
			cur->func_pointer=(ptr_u)SDL_GL_GetProcAddress(cur->function_name);
#endif
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

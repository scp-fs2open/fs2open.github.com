/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLTNL.cpp $
 * $Revision: 1.3 $
 * $Date: 2004-07-05 05:09:19 $
 * $Author: bobboau $
 *
 * source for doing the fun TNL stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/06/28 02:13:07  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
 *
 * Revision 2.4  2004/04/26 13:05:19  taylor
 * respect -glow and -spec
 *
 * Revision 2.3  2004/04/13 01:55:41  phreak
 * put in the correct fields for the CVS comments to register
 * fixed a glowmap problem that occured when rendering glowmapped and non-glowmapped ships
 *
 *
 * $NoKeywords: $
 */

#include <windows.h>

#include "globalincs/pstypes.h"

#include "model/model.h"


#include "graphics/gl/gl.h"
#include "graphics/gl/glu.h"
#include "graphics/gl/glext.h"

#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltnl.h"

#include "math/vecmat.h"
#include "render/3d.h"

#include "debugconsole/timerbar.h"

extern int VBO_ENABLED;
extern int GLOWMAP;
extern int CLOAKMAP;
extern int SPECMAP;
extern vector G3_user_clip_normal;
extern vector G3_user_clip_point;
extern int Interp_multitex_cloakmap;
extern int Interp_cloakmap_alpha;

int GL_modelview_matrix_depth = 1;
int GL_htl_projection_matrix_set = 0;
int GL_htl_view_matrix_set = 0;

struct opengl_vertex_buffer
{
	int n_poly;
	vector *vertex_array;
	uv_pair *texcoord_array;
	vector *normal_array;

	int used;

	uint vbo_vert;
	uint vbo_norm;
	uint vbo_tex;

};

#define MAX_SUBOBJECTS 64

#ifdef INF_BUILD
#define MAX_BUFFERS_PER_SUBMODEL 24
#else
#define MAX_BUFFERS_PER_SUBMODEL 16
#endif

#define MAX_BUFFERS MAX_POLYGON_MODELS*MAX_SUBOBJECTS*MAX_BUFFERS_PER_SUBMODEL

static opengl_vertex_buffer vertex_buffers[MAX_BUFFERS];

//zeros everything out
void opengl_init_vertex_buffers()
{
	memset(vertex_buffers,0,sizeof(opengl_vertex_buffer)*MAX_BUFFERS);
}

int opengl_find_first_free_buffer()
{
	for (int i=0; i < MAX_BUFFERS; i++)
	{
		if (!vertex_buffers[i].used)
			return i;
	}
	
	return -1;
}

int opengl_check_for_errors()
{
#ifdef _DEBUG
	int error=0;
	if ((error=glGetError()) != GL_NO_ERROR)
	{
		mprintf(("!!ERROR!!: %s\n", gluErrorString(error)));
		return 1;
	}
#endif
	return 0;
}

int opengl_mod_depth()
{
	int mv;
	glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &mv);
	return mv;
}

uint opengl_create_vbo(uint size, void** data)
{
	if (!data)
		return 0;

	if (!*data)
		return 0;

	if (size == 0)
		return 0;


	// Kazan: A) This makes that if (buffer_name) work correctly (false = 0, true = anything not 0)
	//				if glGenBuffersARB() doesn't initialized it for some reason
	//        B) It shuts up MSVC about may be used without been initalized
	uint buffer_name=0;

	glGenBuffersARB(1, &buffer_name);
	
	//make sure we have one
	if (buffer_name)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer_name);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, *data, GL_STATIC_DRAW_ARB );
				
		//just in case
		if (opengl_check_for_errors())
		{
			return 0;
		}
		else
		{
			free(*data);
			*data = NULL;
		//	mprintf(("VBO Created: %d\n", buffer_name));
		}
	}

	return buffer_name;
}

int gr_opengl_make_buffer(poly_list *list, uint flags)
{
	int buffer_num=opengl_find_first_free_buffer();

	//we have a valid buffer
	if (buffer_num > -1)
	{
		opengl_vertex_buffer *vbp=&vertex_buffers[buffer_num];

		vbp->used=1;

		vbp->n_poly=list->n_verts / 3;

		vbp->texcoord_array=(uv_pair*)malloc(list->n_verts * sizeof(uv_pair));	
		memset(vbp->texcoord_array,0,list->n_verts*sizeof(uv_pair));

		vbp->normal_array=(vector*)malloc(list->n_verts * sizeof(vector));
		memset(vbp->normal_array,0,list->n_verts*sizeof(vector));

		vbp->vertex_array=(vector*)malloc(list->n_verts * sizeof(vector));
		memset(vbp->vertex_array,0,list->n_verts*sizeof(vector));

		vector *n=vbp->normal_array;
		vector *v=vbp->vertex_array;
		uv_pair *t=vbp->texcoord_array;
	
		vertex *vl;

		memcpy(n,list->norm,list->n_verts*sizeof(vector));
				

		for (int i=0; i < list->n_verts; i++)
		{
				vl=&list->vert[i];
				v->xyz.x=vl->x; 
				v->xyz.y=vl->y;
				v->xyz.z=vl->z;
				v++;
				
				t->u=vl->u;
				t->v=vl->v;
				t++;

		}

		//maybe load it into a vertex buffer object
		if (VBO_ENABLED)
		{
			vbp->vbo_vert=opengl_create_vbo(vbp->n_poly*9*sizeof(float),(void**)&vbp->vertex_array);
			vbp->vbo_norm=opengl_create_vbo(vbp->n_poly*9*sizeof(float),(void**)&vbp->normal_array);
			vbp->vbo_tex=opengl_create_vbo(vbp->n_poly*6*sizeof(float),(void**)&vbp->texcoord_array);
		}

	}
	

	return buffer_num;
}
	
void gr_opengl_destroy_buffer(int idx)
{
	opengl_vertex_buffer *vbp=&vertex_buffers[idx];
	if (vbp->normal_array)		free(vbp->normal_array);
	if (vbp->texcoord_array)	free(vbp->texcoord_array);
	if (vbp->vertex_array)		free(vbp->vertex_array);

	if (vbp->vbo_norm)			glDeleteBuffersARB(1,&vbp->vbo_norm);
	if (vbp->vbo_vert)			glDeleteBuffersARB(1,&vbp->vbo_vert);
	if (vbp->vbo_tex)			glDeleteBuffersARB(1,&vbp->vbo_tex);

	memset(vbp,0,sizeof(opengl_vertex_buffer));
}

opengl_vertex_buffer *g_vbp;
void gr_opengl_set_buffer(int idx){
	g_vbp=&vertex_buffers[idx];
}


//#define DRAW_DEBUG_LINES
extern float Model_Interp_scale_x,Model_Interp_scale_y,Model_Interp_scale_z;

//start is the first part of the buffer to render, n_prim is the number of primitives, index_list is an index buffer, if index_list == NULL render non-indexed
void gr_opengl_render_buffer(int start, int n_prim, short* index_list)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	TIMERBAR_PUSH(2);
	float u_scale,v_scale;

	if (glIsEnabled(GL_CULL_FACE))	glFrontFace(GL_CW);
	
	glColor3ub(255,255,255);
	
	opengl_vertex_buffer *vbp=g_vbp;

	glEnableClientState(GL_VERTEX_ARRAY);
	if (vbp->vbo_vert)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_vert);
		glVertexPointer(3,GL_FLOAT,0, (void*)NULL);
	}
	else
	{
		glVertexPointer(3,GL_FLOAT,0,vbp->vertex_array);
	}

	glEnableClientState(GL_NORMAL_ARRAY);
	if (vbp->vbo_norm)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_norm);
		glNormalPointer(GL_FLOAT,0, (void*)NULL);
	}
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glNormalPointer(GL_FLOAT,0,vbp->normal_array);
	}

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (vbp->vbo_tex)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_tex);
		glTexCoordPointer(2,GL_FLOAT,0,(void*)NULL);
	}
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glTexCoordPointer(2,GL_FLOAT,0,vbp->texcoord_array);
	}

	if (Interp_multitex_cloakmap > 0)
	{
		SPECMAP = -1;	// don't add a spec map if we are cloaked
		GLOWMAP = -1;	// don't use a glowmap either, shouldn't see them

		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo_tex)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_tex);
			glTexCoordPointer(2,GL_FLOAT,0,(void*)NULL);
		}
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glTexCoordPointer(2,GL_FLOAT,0,vbp->texcoord_array);
		}
	}

	if ( (GLOWMAP > -1) && !Cmdline_noglow )
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo_tex)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_tex);
			glTexCoordPointer(2,GL_FLOAT,0,(void*)NULL);
		}
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glTexCoordPointer(2,GL_FLOAT,0,vbp->texcoord_array);
		}
	}

	if ((SPECMAP > -1) && !Cmdline_nospec && (GL_supported_texture_units > 2))
	{
		glClientActiveTextureARB(GL_TEXTURE2_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo_tex)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_tex);
			glTexCoordPointer(2,GL_FLOAT,0,(void*)NULL);
		}
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glTexCoordPointer(2,GL_FLOAT,0,vbp->texcoord_array);
		}
	}

	int r,g,b,a,tmap_type;

	opengl_setup_render_states(r,g,b,a,tmap_type,TMAP_FLAG_TEXTURED,0);

	if (gr_screen.current_bitmap==CLOAKMAP)
	{
		glBlendFunc(GL_ONE,GL_ONE);
		r=g=b=Interp_cloakmap_alpha;
		a=255;
	}

	gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0);
	
	glLockArraysEXT(0,vbp->n_poly*3);
	
	opengl_pre_render_init_lights();
	opengl_change_active_lights(0);

   	glDrawArrays(GL_TRIANGLES,0,vbp->n_poly*3);

	if((lighting_is_enabled)&&((n_active_gl_lights-1)/max_gl_lights > 0)) {
		gr_opengl_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
		opengl_switch_arb(1,0);
		opengl_switch_arb(2,0);
		for(int i=1; i< (n_active_gl_lights-1)/max_gl_lights; i++)
		{
			opengl_change_active_lights(i);
			glDrawArrays(GL_TRIANGLES,0,vbp->n_poly*3); 
		}
	}

	glUnlockArraysEXT();


	TIMERBAR_POP();

	if (VBO_ENABLED)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}


#if defined(DRAW_DEBUG_LINES) && defined(_DEBUG)
	glBegin(GL_LINES);
		glColor3ub(255,0,0);
		glVertex3d(0,0,0);
		glVertex3d(20,0,0);

		glColor3ub(0,255,0);
		glVertex3d(0,0,0);
		glVertex3d(0,20,0);

		glColor3ub(0,0,255);
		glVertex3d(0,0,0);
		glVertex3d(0,0,20);
	glEnd();
#endif

	
}

void gr_opengl_start_instance_matrix(vector *offset, matrix* rotation)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	if (!offset)
		offset = &vmd_zero_vector;
	if (!rotation)
		rotation = &vmd_identity_matrix;	

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	vector axis;
	float ang;
	vm_matrix_to_rot_axis_and_angle(rotation,&ang,&axis);
	glTranslatef(offset->xyz.x,offset->xyz.y,offset->xyz.z);
	glRotatef(fl_degrees(ang),axis.xyz.x,axis.xyz.y,axis.xyz.z);
	GL_modelview_matrix_depth++;
}

void gr_opengl_start_instance_angles(vector *pos, angles* rotation)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	matrix m;
	vm_angles_2_matrix(&m,rotation);
	gr_opengl_start_instance_matrix(pos,&m);
}

void gr_opengl_end_instance_matrix()
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	GL_modelview_matrix_depth--;
}

//the projection matrix; fov, aspect ratio, near, far
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(fl_degrees(fov),aspect,z_near,z_far);
	glMatrixMode(GL_MODELVIEW);
	GL_htl_projection_matrix_set = 1;
}

void gr_opengl_end_projection_matrix()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	GL_htl_projection_matrix_set = 0;
}

void gr_opengl_set_view_matrix(vector *pos, matrix* orient)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_modelview_matrix_depth == 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
	
	vector fwd;
	vector *uvec=&orient->vec.uvec;

	vm_vec_add(&fwd, pos, &orient->vec.fvec);

	gluLookAt(pos->xyz.x,pos->xyz.y,-pos->xyz.z,
	fwd.xyz.x,fwd.xyz.y,-fwd.xyz.z,
	uvec->xyz.x, uvec->xyz.y,-uvec->xyz.z);

	glScalef(1,1,-1);
	glViewport(gr_screen.offset_x,gr_screen.max_h-gr_screen.offset_y-gr_screen.clip_height,gr_screen.clip_width,gr_screen.clip_height);

	GL_modelview_matrix_depth = 2;
	GL_htl_view_matrix_set = 1;	
}

void gr_opengl_end_view_matrix()
{
	Assert(GL_modelview_matrix_depth == 2);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glLoadIdentity();
	glViewport(0,0,gr_screen.max_w, gr_screen.max_h);

	GL_modelview_matrix_depth = 1;
	GL_htl_view_matrix_set = 0;
}

void gr_opengl_push_scale_matrix(vector *scale_factor)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	GL_modelview_matrix_depth++;
	glScalef(scale_factor->xyz.x,scale_factor->xyz.y,scale_factor->xyz.z);
}

void gr_opengl_pop_scale_matrix()
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	GL_modelview_matrix_depth--;
}

void gr_opengl_end_clip_plane()
{
	glDisable(GL_CLIP_PLANE0);
}

void gr_opengl_start_clip_plane()
{
	double clip_equation[4];
	vector n;
	vector p;

	n=G3_user_clip_normal;	
	p=G3_user_clip_point;

	clip_equation[0]=n.xyz.x;
	clip_equation[1]=n.xyz.y;
	clip_equation[2]=n.xyz.z;
	clip_equation[3]=-vm_vec_dot(&p, &n);
	glClipPlane(GL_CLIP_PLANE0, clip_equation);
	glEnable(GL_CLIP_PLANE0);
}

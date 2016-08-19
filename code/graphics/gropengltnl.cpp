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

#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "globalincs/alphacolors.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengltnl.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "weapon/trails.h"
#include "particle/particle.h"
#include "graphics/shadows.h"
#include "graphics/material.h"

extern int GLOWMAP;
extern int CLOAKMAP;
extern int SPECMAP;
extern int SPECGLOSSMAP;
extern int NORMMAP;
extern int MISCMAP;
extern int HEIGHTMAP;
extern int G3_user_clip;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;
extern int Interp_multitex_cloakmap;
extern int Interp_cloakmap_alpha;

extern bool Basemap_override;
extern bool Envmap_override;
extern bool Specmap_override;
extern bool Normalmap_override;
extern bool Heightmap_override;
extern bool Shadow_override;

static int GL_modelview_matrix_depth = 1;
static int GL_htl_projection_matrix_set = 0;
static int GL_htl_view_matrix_set = 0;
static int GL_htl_2d_matrix_depth = 0;
static int GL_htl_2d_matrix_set = 0;

static GLfloat GL_env_texture_matrix[16] = { 0.0f };
static bool GL_env_texture_matrix_set = false;

static GLdouble eyex, eyey, eyez;
static GLdouble vmatrix[16];

static vec3d last_view_pos;
static matrix last_view_orient;

vec3d shadow_ref_point;

static bool use_last_view = false;

size_t GL_vertex_data_in = 0;

GLint GL_max_elements_vertices = 4096;
GLint GL_max_elements_indices = 4096;

float GL_thrust_scale = -1.0f;
team_color* Current_team_color;
team_color Current_temp_color;
bool Using_Team_Color = false;

size_t GL_transform_buffer_offset = INVALID_SIZE;

GLuint Shadow_map_texture = 0;
GLuint Shadow_map_depth_texture = 0;
GLuint shadow_fbo = 0;
GLint saved_fb = 0;
bool Rendering_to_shadow_map = false;

int Transform_buffer_handle = -1;

transform_stack GL_model_matrix_stack;
matrix4 GL_view_matrix;
matrix4 GL_model_view_matrix;
matrix4 GL_projection_matrix;
matrix4 GL_last_projection_matrix;
matrix4 GL_last_view_matrix;

struct opengl_buffer_object {
	GLuint buffer_id;
	GLenum type;
	GLenum usage;
	size_t size;

	GLuint texture;	// for texture buffer objects
};

struct opengl_vertex_buffer {
	GLfloat *array_list;	// interleaved array
	GLubyte *index_list;

	int vb_handle;
	int ib_handle;

	size_t vbo_size;
	size_t ibo_size;

	opengl_vertex_buffer() :
		array_list(NULL), index_list(NULL), 
		vb_handle(-1), ib_handle(-1), vbo_size(0), ibo_size(0)
	{
	}

	void clear();
};

void opengl_vertex_buffer::clear()
{
	if (array_list) {
		vm_free(array_list);
	}
	
	if (index_list) {
		vm_free(index_list);
	}
	
	if ( vb_handle >= 0 ) {
		gr_opengl_delete_buffer(vb_handle);
	}

	if ( ib_handle >= 0 ) {
		gr_opengl_delete_buffer(ib_handle);
	}
}

static SCP_vector<opengl_buffer_object> GL_buffer_objects;
static SCP_vector<opengl_vertex_buffer> GL_vertex_buffers;
static opengl_vertex_buffer *g_vbp = NULL;
static int GL_vertex_buffers_in_use = 0;

int GL_immediate_buffer_handle = -1;
static uint GL_immediate_buffer_offset = 0;
static uint GL_immediate_buffer_size = 0;
static const int IMMEDIATE_BUFFER_RESIZE_BLOCK_SIZE = 2048;

int opengl_create_buffer_object(GLenum type, GLenum usage)
{
	opengl_buffer_object buffer_obj;

	buffer_obj.usage = usage;
	buffer_obj.type = type;
	buffer_obj.size = 0;

	glGenBuffers(1, &buffer_obj.buffer_id);

	GL_buffer_objects.push_back(buffer_obj);

	return (int)(GL_buffer_objects.size() - 1);
}

void opengl_bind_buffer_object(int handle)
{
	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	switch ( buffer_obj.type ) {
	case GL_ARRAY_BUFFER:
		GL_state.Array.BindArrayBuffer(buffer_obj.buffer_id);
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		GL_state.Array.BindElementBuffer(buffer_obj.buffer_id);
		break;
	case GL_TEXTURE_BUFFER:
		GL_state.Array.BindTextureBuffer(buffer_obj.buffer_id);
		break;
	case GL_UNIFORM_BUFFER:
		GL_state.Array.BindUniformBuffer(buffer_obj.buffer_id);
		break;
	default:
		Int3();
		return;
		break;
	}
}

void gr_opengl_update_buffer_data(int handle, size_t size, void* data)
{
	if ( !Use_VBOs ) {
		return;
	}

	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	opengl_bind_buffer_object(handle);

	GL_vertex_data_in -= buffer_obj.size;
	buffer_obj.size = size;
	GL_vertex_data_in += buffer_obj.size;

	glBufferData(buffer_obj.type, size, data, buffer_obj.usage);
}

void opengl_update_buffer_data_offset(int handle, uint offset, uint size, void* data)
{
	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	opengl_bind_buffer_object(handle);
	
	glBufferSubData(buffer_obj.type, offset, size, data);
}

void gr_opengl_delete_buffer(int handle)
{
	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	// de-bind the buffer point so we can clear the recorded state.
	switch ( buffer_obj.type ) {
	case GL_ARRAY_BUFFER:
		GL_state.Array.BindArrayBuffer(0);
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		GL_state.Array.BindElementBuffer(0);
		break;
	case GL_TEXTURE_BUFFER:
		GL_state.Array.BindTextureBuffer(0);
		break;
	case GL_UNIFORM_BUFFER:
		GL_state.Array.BindUniformBuffer(0);
		break;
	default:
		Int3();
		return;
		break;
	}

	if ( buffer_obj.type == GL_TEXTURE_BUFFER ) {
		glDeleteTextures(1, &buffer_obj.texture);
	}

	GL_vertex_data_in -= buffer_obj.size;

	glDeleteBuffers(1, &buffer_obj.buffer_id);
}

int gr_opengl_create_vertex_buffer(bool static_buffer)
{
	if ( !Use_VBOs ) {
		return -1;
	}

	return opengl_create_buffer_object(GL_ARRAY_BUFFER, static_buffer ? GL_STATIC_DRAW : GL_STREAM_DRAW);
}

int gr_opengl_create_index_buffer(bool static_buffer)
{
	if ( !Use_VBOs ) {
		return -1;
	}

	return opengl_create_buffer_object(GL_ELEMENT_ARRAY_BUFFER, static_buffer ? GL_STATIC_DRAW : GL_STREAM_DRAW);
}

int gr_opengl_create_stream_buffer_object()
{
	if (!Use_VBOs) {
		return -1;
	}

	return opengl_create_buffer_object(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
}

uint opengl_add_to_immediate_buffer(uint size, void *data)
{
	if ( GL_immediate_buffer_handle < 0 ) {
		GL_immediate_buffer_handle = opengl_create_buffer_object(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	}

	Assert(size > 0 && data != NULL);

	if ( GL_immediate_buffer_offset + size > GL_immediate_buffer_size ) {
		// incoming data won't fit the immediate buffer. time to reallocate.
		GL_immediate_buffer_offset = 0;
		GL_immediate_buffer_size += MAX(IMMEDIATE_BUFFER_RESIZE_BLOCK_SIZE, size);
		
		gr_opengl_update_buffer_data(GL_immediate_buffer_handle, GL_immediate_buffer_size, NULL);
	}

	// only update a section of the immediate vertex buffer
	opengl_update_buffer_data_offset(GL_immediate_buffer_handle, GL_immediate_buffer_offset, size, data);

	uint old_offset = GL_immediate_buffer_offset;

	GL_immediate_buffer_offset += size;

	return old_offset;
}

void opengl_reset_immediate_buffer()
{
	if ( GL_immediate_buffer_handle < 0 ) {
		// we haven't used the immediate buffer yet
		return;
	}

	// orphan the immediate buffer so we can start fresh in a new frame
	gr_opengl_update_buffer_data(GL_immediate_buffer_handle, GL_immediate_buffer_size, NULL);

	// bring our offset to the beginning of the immediate buffer
	GL_immediate_buffer_offset = 0;
}

int opengl_create_texture_buffer_object()
{
	if ( GLSL_version < 130 || !GLAD_GL_ARB_texture_buffer_object || !GLAD_GL_ARB_texture_float ) {
		return -1;
	}

	// create the buffer
	int buffer_object_handle = opengl_create_buffer_object(GL_TEXTURE_BUFFER_ARB, GL_DYNAMIC_DRAW);

	opengl_check_for_errors();

	opengl_buffer_object &buffer_obj = GL_buffer_objects[buffer_object_handle];

	// create the texture
	glGenTextures(1, &buffer_obj.texture);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, buffer_obj.texture);

	gr_opengl_update_buffer_data(buffer_object_handle, 100, NULL);

	glTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_RGBA32F_ARB, buffer_obj.buffer_id);

	opengl_check_for_errors();

	return buffer_object_handle;
}

void gr_opengl_update_transform_buffer(void* data, size_t size)
{
	if ( Transform_buffer_handle < 0 || size <= 0 ) {
		return;
	}

	gr_opengl_update_buffer_data(Transform_buffer_handle, size, data);

	opengl_buffer_object &buffer_obj = GL_buffer_objects[Transform_buffer_handle];

	// need to rebind the buffer object to the texture buffer after it's been updated.
	// didn't have to do this on AMD and Nvidia drivers but Intel drivers seem to want it.
	glBindTexture(GL_TEXTURE_BUFFER, buffer_obj.texture);
	glTexBufferARB(GL_TEXTURE_BUFFER, GL_RGBA32F_ARB, buffer_obj.buffer_id);
}

GLuint opengl_get_transform_buffer_texture()
{
	if ( Transform_buffer_handle < 0 ) {
		return 0;
	}

	return GL_buffer_objects[Transform_buffer_handle].texture;
}

void gr_opengl_set_transform_buffer_offset(size_t offset)
{
	GL_transform_buffer_offset = offset;
}

static void opengl_gen_buffer(opengl_vertex_buffer *vbp)
{
	if ( !Use_VBOs ) {
		return;
	}

	if ( !vbp ) {
		return;
	}

	if ( !(vbp->vbo_size && vbp->ibo_size) ) {
		return;
	}

	// create vertex buffer
	{
		// clear any existing errors
		glGetError();

		vbp->vb_handle = opengl_create_buffer_object(GL_ARRAY_BUFFER, GL_STATIC_DRAW);

		// make sure we have one
		if ( vbp->vb_handle >= 0 ) {
			gr_opengl_update_buffer_data(vbp->vb_handle, vbp->vbo_size, vbp->array_list);

			// just in case
			if ( opengl_check_for_errors() ) {
				gr_opengl_delete_buffer(vbp->vb_handle);
				vbp->vb_handle = -1;
				return;
			}

			GL_state.Array.BindArrayBuffer(0);

			vm_free(vbp->array_list);
			vbp->array_list = NULL;
		}
	}

	// create index buffer
	{
		// clear any existing errors
		glGetError();

		vbp->ib_handle = opengl_create_buffer_object(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

		// make sure we have one
		if ( vbp->ib_handle >= 0 ) {
			gr_opengl_update_buffer_data(vbp->ib_handle, vbp->ibo_size, vbp->index_list);

			// just in case
			if ( opengl_check_for_errors() ) {
				gr_opengl_delete_buffer(vbp->ib_handle);
				vbp->ib_handle = -1;
				return;
			}

			GL_state.Array.BindElementBuffer(0);

			vm_free(vbp->index_list);
			vbp->index_list = NULL;
		}
	}
}

int gr_opengl_create_buffer()
{
	opengl_vertex_buffer vbuffer;

	GL_vertex_buffers.push_back( vbuffer );
	GL_vertex_buffers_in_use++;

	return (int)(GL_vertex_buffers.size() - 1);
}

bool gr_opengl_config_buffer(const int buffer_id, vertex_buffer *vb, bool update_ibuffer_only)
{
	if (buffer_id < 0) {
		return false;
	}

	Assert( buffer_id < (int)GL_vertex_buffers.size() );

	if (vb == NULL) {
		return false;
	}

	if ( !(vb->flags & VB_FLAG_POSITION) ) {
		Int3();
		return false;
	}

	opengl_vertex_buffer *m_vbp = &GL_vertex_buffers[buffer_id];


	vb->stride = 0;

	// position
	Verify( update_ibuffer_only || vb->model_list->vert != NULL );

	vb->stride += (3 * sizeof(GLfloat));

	// normals
	if (vb->flags & VB_FLAG_NORMAL) {
		Verify( update_ibuffer_only || vb->model_list->norm != NULL );

		vb->stride += (3 * sizeof(GLfloat));
	}

	// uv coords
	if (vb->flags & VB_FLAG_UV1) {
		vb->stride += (2 * sizeof(GLfloat));
	}

	// tangent space data for normal maps (shaders only)
	if (vb->flags & VB_FLAG_TANGENT) {
		Assert( Cmdline_normal );

		Verify( update_ibuffer_only || vb->model_list->tsb != NULL );
		vb->stride += (4 * sizeof(GLfloat));
	}

	if (vb->flags & VB_FLAG_MODEL_ID) {
		Assert( GLSL_version >= 150 );

		Verify( update_ibuffer_only || vb->model_list->submodels != NULL );
		vb->stride += (1 * sizeof(GLfloat));
	}

	// offsets for this chunk
	if ( !update_ibuffer_only ) {
		vb->vertex_offset = m_vbp->vbo_size;
		m_vbp->vbo_size += vb->stride * vb->model_list->n_verts;
	}

	for (size_t idx = 0; idx < vb->tex_buf.size(); idx++) {
		buffer_data *bd = &vb->tex_buf[idx];

		bd->index_offset = m_vbp->ibo_size;
		m_vbp->ibo_size += bd->n_verts * ((bd->flags & VB_FLAG_LARGE_INDEX) ? sizeof(uint) : sizeof(ushort));

		// even out index buffer so we are always word aligned
		m_vbp->ibo_size += m_vbp->ibo_size % sizeof(uint); 
	}

	return true;
}

bool gr_opengl_pack_buffer(const int buffer_id, vertex_buffer *vb)
{
	if (buffer_id < 0) {
		return false;
	}

	Assert( buffer_id < (int)GL_vertex_buffers.size() );

	opengl_vertex_buffer *m_vbp = &GL_vertex_buffers[buffer_id];

	// NULL means that we are done with the buffer and can create the IBO/VBO
	// returns false here only for some minor error prevention
	if (vb == NULL) {
		opengl_gen_buffer(m_vbp);
		return false;
	}

	size_t n_verts = 0;
	size_t j;
	uint arsize = 0;


	if (m_vbp->array_list == NULL) {
		m_vbp->array_list = (GLfloat*)vm_malloc(m_vbp->vbo_size, memory::quiet_alloc);

		// return invalid if we don't have the memory
		if (m_vbp->array_list == NULL) {
			return false;
		}

		memset(m_vbp->array_list, 0, m_vbp->vbo_size);
	}

	if (m_vbp->index_list == NULL) {
		m_vbp->index_list = (GLubyte*)vm_malloc(m_vbp->ibo_size, memory::quiet_alloc);

		// return invalid if we don't have the memory
		if (m_vbp->index_list == NULL) {
			return false;
		}

		memset(m_vbp->index_list, 0, m_vbp->ibo_size);
	}

	// bump to our index in the array
	GLfloat *array = m_vbp->array_list + (vb->vertex_offset / sizeof(GLfloat));

	// generate the vertex array
	n_verts = vb->model_list->n_verts;
	for (size_t i = 0; i < n_verts; i++) {
		vertex *vl = &vb->model_list->vert[i];

		// don't try to generate more data than what's available
		Assert( ((arsize * sizeof(GLfloat)) + vb->stride) <= (m_vbp->vbo_size - vb->vertex_offset) );

		// NOTE: UV->NORM->TSB->VERT, This array order *must* be preserved!!

		// tex coords
		if (vb->flags & VB_FLAG_UV1) {
			array[arsize++] = vl->texture_position.u;
			array[arsize++] = vl->texture_position.v;
		}

		// normals
		if (vb->flags & VB_FLAG_NORMAL) {
			vec3d *nl = &vb->model_list->norm[i];
			array[arsize++] = nl->xyz.x;
			array[arsize++] = nl->xyz.y;
			array[arsize++] = nl->xyz.z;
		}

		// tangent space data
		if (vb->flags & VB_FLAG_TANGENT) {
			tsb_t *tsb = &vb->model_list->tsb[i];
			array[arsize++] = tsb->tangent.xyz.x;
			array[arsize++] = tsb->tangent.xyz.y;
			array[arsize++] = tsb->tangent.xyz.z;
			array[arsize++] = tsb->scaler;
		}

		if (vb->flags & VB_FLAG_MODEL_ID) {
			array[arsize++] = (float)vb->model_list->submodels[i];
		}

		// verts
		array[arsize++] = vl->world.xyz.x;
		array[arsize++] = vl->world.xyz.y;
		array[arsize++] = vl->world.xyz.z;
	}

	// generate the index array
	for (j = 0; j < vb->tex_buf.size(); j++) {
		buffer_data* tex_buf = &vb->tex_buf[j];
		n_verts = tex_buf->n_verts;
		size_t offset = tex_buf->index_offset;
		const uint *index = tex_buf->get_index();

		// bump to our spot in the buffer
		GLubyte *ibuf = m_vbp->index_list + offset;

		if (vb->tex_buf[j].flags & VB_FLAG_LARGE_INDEX) {
			memcpy(ibuf, index, n_verts * sizeof(uint));
		} else {
			ushort *mybuf = (ushort*)ibuf;

			for (size_t i = 0; i < n_verts; i++) {
				mybuf[i] = (ushort)index[i];
			}
		}
	}
	
	return true;
}

void gr_opengl_set_buffer(int idx)
{
	g_vbp = NULL;

	if (idx < 0) {
		if (Use_VBOs) {
			GL_state.Array.BindArrayBuffer(0);
			GL_state.Array.BindElementBuffer(0);
		}

		if ( is_minimum_GLSL_version() ) {
			opengl_shader_set_current();
		}

		return;
	}

	Assert( idx < (int)GL_vertex_buffers.size() );

	g_vbp = &GL_vertex_buffers[idx];
}

void gr_opengl_destroy_buffer(int idx)
{
	if (idx < 0) {
		return;
	}

	Assert( idx < (int)GL_vertex_buffers.size() );

	opengl_vertex_buffer *vbp = &GL_vertex_buffers[idx];

	vbp->clear();

	// we try to take advantage of the fact that there shouldn't be a lot of buffer
	// deletions/additions going on all of the time, so a model_unload_all() and/or
	// game_level_close() should pretty much keep everything cleared out on a
	// regular basis
	if (--GL_vertex_buffers_in_use <= 0) {
		GL_vertex_buffers.clear();
	}

	g_vbp = NULL;
}

void opengl_destroy_all_buffers()
{
	for (uint i = 0; i < GL_vertex_buffers.size(); i++) {
		gr_opengl_destroy_buffer(i);
	}

	for ( uint i = 0; i < GL_buffer_objects.size(); i++ ) {
		gr_opengl_delete_buffer(i);
	}

	GL_vertex_buffers.clear();
	GL_vertex_buffers_in_use = 0;
}

void opengl_tnl_init()
{
	GL_vertex_buffers.reserve(MAX_POLYGON_MODELS);
	gr_opengl_deferred_light_cylinder_init(16);
	gr_opengl_deferred_light_sphere_init(16, 16);

	Transform_buffer_handle = opengl_create_texture_buffer_object();

	if ( Transform_buffer_handle < 0 ) {
		Cmdline_no_batching = true;
	}

	if(Cmdline_shadow_quality)
	{
		//Setup shadow map framebuffer
		glGenFramebuffers(1, &shadow_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);

		glGenTextures(1, &Shadow_map_depth_texture);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY_EXT);
//		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Shadow_map_depth_texture);
		
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);
		int size = (Cmdline_shadow_quality == 2 ? 1024 : 512);
		glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_DEPTH_COMPONENT32, size, size, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glFramebufferTextureARB(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Shadow_map_depth_texture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Shadow_map_depth_texture, 0);

		glGenTextures(1, &Shadow_map_texture);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY_EXT);
		//GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Shadow_map_texture);

		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGB32F_ARB, size, size, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size, size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		glFramebufferTextureARB(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Shadow_map_texture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Shadow_map_texture, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		opengl_check_for_errors("post_init_framebuffer()");
	}
}

void opengl_tnl_shutdown()
{
	if ( Shadow_map_depth_texture ) {
		glDeleteTextures(1, &Shadow_map_depth_texture);
		Shadow_map_depth_texture = 0;
	}

	if ( Shadow_map_texture ) {
		glDeleteTextures(1, &Shadow_map_texture);
		Shadow_map_texture = 0;
	}

	opengl_destroy_all_buffers();
}

void mix_two_team_colors(team_color* dest, team_color* a, team_color* b, float mix_factor) {
	dest->base.r = a->base.r * (1.0f - mix_factor) + b->base.r * mix_factor;
	dest->base.g = a->base.g * (1.0f - mix_factor) + b->base.g * mix_factor;
	dest->base.b = a->base.b * (1.0f - mix_factor) + b->base.b * mix_factor;

	dest->stripe.r = a->stripe.r * (1.0f - mix_factor) + b->stripe.r * mix_factor;
	dest->stripe.g = a->stripe.g * (1.0f - mix_factor) + b->stripe.g * mix_factor;
	dest->stripe.b = a->stripe.b * (1.0f - mix_factor) + b->stripe.b * mix_factor;
}

void gr_opengl_set_team_color(const team_color *colors)
{
	if ( colors == NULL ) {
		Using_Team_Color = false;
	} else {
		Using_Team_Color = true;
		Current_temp_color = *colors;
		Current_team_color = &Current_temp_color;
	}
}

void gr_opengl_set_thrust_scale(float scale)
{
	GL_thrust_scale = scale;
}

static void opengl_init_arrays(opengl_vertex_buffer *vert_src, vertex_buffer *bufferp)
{
	GLint vert_offset = (GLint)bufferp->vertex_num_offset;
	GLubyte *ptr = NULL;

	if ( is_minimum_GLSL_version() && GLAD_GL_ARB_draw_elements_base_vertex ) {
		vert_offset = 0;
	}

	if ( vert_src->vb_handle >= 0 ) {
		opengl_bind_buffer_object(vert_src->vb_handle);
	} else {
		ptr = (GLubyte*)vert_src->array_list;
	}

	opengl_bind_vertex_layout(bufferp->layout, vert_offset, ptr);
}

static void opengl_init_arrays(indexed_vertex_source *vert_src, vertex_buffer *bufferp)
{
	GLint vert_offset = (GLint)bufferp->vertex_num_offset;
	GLubyte *ptr = NULL;

	if ( is_minimum_GLSL_version() && GLAD_GL_ARB_draw_elements_base_vertex ) {
		vert_offset = 0;
	}

	if ( vert_src->Vbuffer_handle >= 0 ) {
		opengl_bind_buffer_object(vert_src->Vbuffer_handle);
	} else {
		ptr = (GLubyte*)vert_src->Vertex_list;
	}
	
	opengl_bind_vertex_layout(bufferp->layout, vert_offset, ptr);
}

void opengl_render_model_program(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, buffer_data *datap)
{
	GL_state.Texture.SetShaderMode(GL_TRUE);

	opengl_tnl_set_model_material(material_info);

	GLubyte *ibuffer = NULL;

	int start = 0;
	int end = (datap->n_verts - 1);
	int count = (end - (start * 3) + 1);

	GLenum element_type = (datap->flags & VB_FLAG_LARGE_INDEX) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

	Assert(vert_source);

	// basic setup of all data
	opengl_init_arrays(vert_source, bufferp);

	if ( vert_source->Ibuffer_handle >= 0 ) {
		opengl_bind_buffer_object(vert_source->Ibuffer_handle);
	} else {
		ibuffer = (GLubyte*)vert_source->Index_list;
	}

	if ( Rendering_to_shadow_map ) {
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start), 4, (GLint)bufferp->vertex_num_offset);
	} else {
		if ( GLAD_GL_ARB_draw_elements_base_vertex ) {
			if ( Cmdline_drawelements ) {
				glDrawElementsBaseVertex(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_num_offset);
			} else {
				glDrawRangeElementsBaseVertex(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_num_offset);
			}
		} else {
			if ( Cmdline_drawelements ) {
				glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start));
			} else {
				glDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));
			}
		}
	}

	GL_state.Texture.SetShaderMode(GL_FALSE);
}

void opengl_render_model_fixed(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer *bufferp, buffer_data *datap)
{

}

void gr_opengl_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, int texi)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	Verify(bufferp != NULL);

	GL_CHECK_FOR_ERRORS("start of render_buffer()");

	Assert(texi >= 0);

	buffer_data *datap = &bufferp->tex_buf[texi];

	if ( is_minimum_GLSL_version() ) {
		opengl_render_model_program(material_info, vert_source, bufferp, datap);
	} else {
		opengl_render_model_fixed(material_info, vert_source, bufferp, datap);
	}

	GL_CHECK_FOR_ERRORS("end of render_buffer()");
}

#define DO_RENDER()	\
	if (Cmdline_drawelements) \
		glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start)); \
	else \
		glDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));

unsigned int GL_last_shader_flags = 0;
int GL_last_shader_index = -1;

static void opengl_render_pipeline_fixed(int start, vertex_buffer *bufferp, buffer_data *datap, int flags);

extern bool Scene_framebuffer_in_frame;
extern GLuint Framebuffer_fallback_texture_id;
extern matrix Object_matrix;
extern vec3d Object_position;
extern int Interp_thrust_scale_subobj;
extern float Interp_thrust_scale;
static void opengl_render_pipeline_program(int start, vertex_buffer *bufferp, buffer_data *datap, int flags)
{
	unsigned int shader_flags = 0;
	int sdr_index = -1;
	int r, g, b, a, tmap_type;
	GLubyte *ibuffer = NULL;

	auto end = (datap->n_verts - 1);
	auto count = (end - (start*3) + 1);

	GLenum element_type = (datap->flags & VB_FLAG_LARGE_INDEX) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

	opengl_vertex_buffer *vbp = g_vbp;
	Assert( vbp );

	int textured = ((flags & TMAP_FLAG_TEXTURED) && (bufferp->flags & VB_FLAG_UV1));

	// setup shader flags for the things that we want/need
	shader_flags = gr_determine_model_shader_flags(
		lighting_is_enabled, 
		GL_state.Fog() ? true : false, 
		textured ? true : false, 
		Rendering_to_shadow_map,
		GL_thrust_scale > 0.0f,
		(flags & TMAP_FLAG_BATCH_TRANSFORMS) && (GL_transform_buffer_offset != INVALID_SIZE) && (bufferp->flags & VB_FLAG_MODEL_ID),
		Using_Team_Color, 
		flags, 
		(SPECGLOSSMAP > 0) ? SPECGLOSSMAP : SPECMAP, 
		GLOWMAP, 
		NORMMAP, 
		HEIGHTMAP, 
		AMBIENTMAP,
		ENVMAP, 
		MISCMAP
	);

	// find proper shader
	if (shader_flags == GL_last_shader_flags) {
		sdr_index = GL_last_shader_index;
	} else {
		sdr_index = gr_opengl_maybe_create_shader(SDR_TYPE_MODEL, shader_flags);

		if (sdr_index < 0) {
			opengl_render_pipeline_fixed(start, bufferp, datap, flags);
			return;
		}

		GL_last_shader_index = sdr_index;
		GL_last_shader_flags = shader_flags;
	}

	Assert( sdr_index >= 0 );

	opengl_shader_set_current( sdr_index );

	opengl_default_light_settings( !GL_center_alpha, (GL_light_factor > 0.25f) );
	gr_opengl_set_center_alpha(GL_center_alpha);

	opengl_setup_render_states(r, g, b, a, tmap_type, flags);
	GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );

	GL_state.Texture.SetShaderMode(GL_TRUE);

	// basic setup of all data
	opengl_init_arrays(vbp, bufferp);

	if ( vbp->ib_handle >= 0 ) {
		opengl_bind_buffer_object(vbp->ib_handle);
	} else {
		ibuffer = (GLubyte*)vbp->index_list;
	}
	
	if(Rendering_to_shadow_map) {
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, (GLsizei)count, element_type, ibuffer + (datap->index_offset + start), 4, (GLint)bufferp->vertex_num_offset);
	} else {
		if ( GLAD_GL_ARB_draw_elements_base_vertex ) {
			if (Cmdline_drawelements) {
				glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_num_offset);
			} else {
				glDrawRangeElementsBaseVertex(GL_TRIANGLES, datap->i_first, datap->i_last, (GLsizei)count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_num_offset);
			}
		} else {
			if (Cmdline_drawelements) {
				glDrawElements(GL_TRIANGLES, (GLsizei)count, element_type, ibuffer + (datap->index_offset + start));
			} else {
				glDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, (GLsizei)count, element_type, ibuffer + (datap->index_offset + start));
			}
		}
	}
/*
	int n_light_passes = (MIN(Num_active_gl_lights, GL_max_lights) - 1) / 3;

	if (lighting_is_enabled && n_light_passes > 0) {
		shader_flags = SDR_FLAG_LIGHT;

		if (textured) {
			if ( !Basemap_override ) {
				shader_flags |= SDR_FLAG_DIFFUSE_MAP;
			}

			if ( (SPECMAP > 0) && !Specmap_override ) {
				shader_flags |= SDR_FLAG_SPEC_MAP;
			}
		}

		opengl::shader_manager::get()->apply_main_shader(shader_flags);
		sdr = opengl::shader_manager::get()->get_main_shader();

		if (sdr) {
			GL_state.BlendFunc(GL_ONE, GL_ONE);

			int zbuf = gr_zbuffer_set(GR_ZBUFF_READ);

			static const float GL_light_zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			glMaterialfv( GL_FRONT, GL_AMBIENT, GL_light_zero );
			glMaterialfv( GL_FRONT, GL_EMISSION, GL_light_zero );

			GL_state.DepthMask(GL_FALSE);
			GL_state.DepthFunc(GL_LEQUAL);

			for (int i = 0; i < n_light_passes; i++) {
				int offset = 3 * (i+1) - 1;
				opengl_change_active_lights(0, offset);

				n_lights = MIN(Num_active_gl_lights, GL_max_lights) - offset;
				sdr->set_uniform(opengl::main_shader::n_lights, n_lights);

				DO_RENDER();
			}

			gr_zbuffer_set(zbuf);
		}
	}
*/
/*
	if (Num_active_gl_lights > 4) {
		opengl_change_active_lights(0, 4);

		int n_lights = MIN(Num_active_gl_lights, GL_max_lights) - 5;
		glUniform1iARB( opengl_shader_get_uniform("n_lights"), n_lights );

		opengl_default_light_settings(0, 0, 0);

		opengl_shader_set_current( &GL_shader[2] );

		GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);

		GL_state.DepthMask(GL_FALSE);
		GL_state.DepthFunc(GL_LEQUAL);

		DO_RENDER();
	}
*/
	GL_state.Texture.SetShaderMode(GL_FALSE);
}

static void opengl_render_pipeline_fixed(int start, vertex_buffer *bufferp, buffer_data *datap, int flags)
{
	
}

// start is the first part of the buffer to render, n_prim is the number of primitives, index_list is an index buffer, if index_list == NULL render non-indexed
void gr_opengl_render_buffer(int start, vertex_buffer *bufferp, size_t texi, int flags)
{
	Assert( GL_htl_projection_matrix_set );
	Assert( GL_htl_view_matrix_set );

	Verify( bufferp != NULL );

	GL_CHECK_FOR_ERRORS("start of render_buffer()");

	if ( GL_state.CullFace() ) {
		GL_state.FrontFaceValue(GL_CW);
	}

	buffer_data *datap = &bufferp->tex_buf[texi];

	if ( is_minimum_GLSL_version() ) {
		opengl_render_pipeline_program(start, bufferp, datap, flags);
	} else {
		opengl_render_pipeline_fixed(start, bufferp, datap, flags);
	}

	GL_CHECK_FOR_ERRORS("end of render_buffer()");
}

extern GLuint Scene_depth_texture;
extern GLuint Scene_position_texture;
extern GLuint Distortion_texture[2];
extern int Distortion_switch;
void gr_opengl_render_stream_buffer(int buffer_handle, size_t offset, size_t n_verts, int flags)
{
	int alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;
	GL_CHECK_FOR_ERRORS("start of render3d()");

	int stride = 0;

	GLubyte *ptr = NULL;
	int vert_offset = 0;

	int pos_offset = -1;
	int tex_offset = -1;
	int radius_offset = -1;
	int color_offset = -1;
	int up_offset = -1;

	if ( flags & TMAP_FLAG_VERTEX_GEN ) {	
		stride = sizeof(particle_pnt);

		pos_offset = vert_offset;
		vert_offset += sizeof(vec3d);

		radius_offset = vert_offset;
		vert_offset += sizeof(float);

		up_offset = vert_offset;
		//tex_offset = vert_offset;
		vert_offset += sizeof(vec3d);
	} else {
		stride = sizeof(effect_vertex);

		pos_offset = vert_offset;
		vert_offset += sizeof(vec3d);

		tex_offset = vert_offset;
		vert_offset += sizeof(uv_pair);

		radius_offset = vert_offset;
		vert_offset += sizeof(float);

		color_offset = vert_offset;
		vert_offset += sizeof(ubyte)*4;
	}

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	opengl_bind_buffer_object(buffer_handle);

	vertex_layout vert_def;

	if ( flags & TMAP_FLAG_TEXTURED ) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}

		if ( tex_offset >= 0 ) {
			vert_def.add_vertex_component(vertex_format_data::TEX_COORD, stride, ptr + tex_offset);
		}
	}

	if (flags & TMAP_FLAG_TRILIST) {
		gl_mode = GL_TRIANGLES;
	} else if (flags & TMAP_FLAG_TRISTRIP) {
		gl_mode = GL_TRIANGLE_STRIP;
	} else if (flags & TMAP_FLAG_POINTLIST) {
		gl_mode = GL_POINTS;
	} else if (flags & TMAP_FLAG_LINESTRIP) {
		gl_mode = GL_LINE_STRIP;
	}

	if ( (flags & TMAP_FLAG_RGB) && (flags & TMAP_FLAG_GOURAUD) && color_offset >= 0 ) {
		vert_def.add_vertex_component(vertex_format_data::COLOR4, stride, ptr + color_offset);
	} else {
		// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
		GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	if ( pos_offset >= 0 ) {
		vert_def.add_vertex_component(vertex_format_data::POSITION3, stride, ptr + pos_offset);
	}

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(gl_mode, (GLint)offset, (GLsizei)n_verts);

	if( (flags & TMAP_FLAG_DISTORTION) || (flags & TMAP_FLAG_DISTORTION_THRUSTER) ) {
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, buffers);
	}
	
	GL_CHECK_FOR_ERRORS("end of render3d()");
}

void opengl_create_perspective_projection_matrix(matrix4 *out, float left, float right, float bottom, float top, float near_dist, float far_dist)
{
	memset(out, 0, sizeof(matrix4));

	out->a1d[0] = 2.0 * near_dist / (right - left);
	out->a1d[5] = 2.0 * near_dist / (top - bottom);
	out->a1d[8] = (right + left) / (right - left);
	out->a1d[9] = (top + bottom) / (top - bottom);
	out->a1d[10] = -(far_dist + near_dist) / (far_dist - near_dist);
	out->a1d[11] = -1.0f;
	out->a1d[14] = -2.0f * far_dist * near_dist / (far_dist - near_dist);
}

void opengl_create_orthographic_projection_matrix(matrix4* out, float left, float right, float bottom, float top, float near_dist, float far_dist)
{
	memset(out, 0, sizeof(matrix4));

	out->a1d[0] = 2.0f / (right - left);
	out->a1d[5] = 2.0f / (top - bottom);
	out->a1d[10] = -2.0f / (far_dist - near_dist);
	out->a1d[12] = -(right + left) / (right - left);
	out->a1d[13] = -(top + bottom) / (top - bottom);
	out->a1d[14] = -(far_dist + near_dist) / (far_dist - near_dist);
	out->a1d[15] = 1.0f;
}

void opengl_create_view_matrix(matrix4 *out, const vec3d *pos, const matrix *orient)
{
	vec3d scaled_pos;
	vec3d inv_pos;
	matrix scaled_orient = *orient;
	matrix inv_orient;

	vm_vec_copy_scale(&scaled_pos, pos, -1.0f);
	vm_vec_scale(&scaled_orient.vec.fvec, -1.0f);

	vm_copy_transpose(&inv_orient, &scaled_orient);
	vm_vec_rotate(&inv_pos, &scaled_pos, &scaled_orient);

	vm_matrix4_set_transform(out, &inv_orient, &inv_pos);
}

void opengl_start_instance_matrix_fixed_pipeline(const vec3d *offset, const matrix *rotation)
{

}

void gr_opengl_start_instance_matrix(const vec3d *offset, const matrix *rotation)
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_start_instance_matrix_fixed_pipeline(offset, rotation);
		return;
	}

	Assert( GL_htl_projection_matrix_set );
	Assert( GL_htl_view_matrix_set );

	if (offset == NULL) {
		offset = &vmd_zero_vector;
	}

	if (rotation == NULL) {
		rotation = &vmd_identity_matrix;	
	}

	GL_CHECK_FOR_ERRORS("start of start_instance_matrix()");

	vec3d axis;
	float ang;
	vm_matrix_to_rot_axis_and_angle(rotation, &ang, &axis);

	GL_model_matrix_stack.push(offset, rotation);

	matrix4 model_matrix = GL_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&GL_model_view_matrix, &GL_view_matrix, &model_matrix);

	GL_CHECK_FOR_ERRORS("end of start_instance_matrix()");

	GL_modelview_matrix_depth++;
}

void gr_opengl_start_instance_angles(const vec3d *pos, const angles *rotation)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	matrix m;
	vm_angles_2_matrix(&m, rotation);

	gr_opengl_start_instance_matrix(pos, &m);
}

void opengl_end_instance_matrix_fixed_pipeline()
{

}

void gr_opengl_end_instance_matrix()
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_end_instance_matrix_fixed_pipeline();
		return;
	}

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	GL_model_matrix_stack.pop();

	matrix4 model_matrix = GL_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&GL_model_view_matrix, &GL_view_matrix, &model_matrix);

	GL_modelview_matrix_depth--;
}


void opengl_set_projection_matrix_fixed_pipeline(float fov, float aspect, float z_near, float z_far)
{

}

// the projection matrix; fov, aspect ratio, near, far
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_set_projection_matrix_fixed_pipeline(fov, aspect, z_near, z_far);
	}
	
	GL_CHECK_FOR_ERRORS("start of set_projection_matrix()()");
	
	if (GL_rendering_to_texture) {
		glViewport(gr_screen.offset_x, gr_screen.offset_y, gr_screen.clip_width, gr_screen.clip_height);
	} else {
		glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
	}
	
	float clip_width, clip_height;

	clip_height = tan( fov * 0.5 ) * z_near;
	clip_width = clip_height * aspect;

	GL_last_projection_matrix = GL_projection_matrix;

	if (GL_rendering_to_texture) {
		opengl_create_perspective_projection_matrix(&GL_projection_matrix, -clip_width, clip_width, clip_height, -clip_height, z_near, z_far);
	} else {
		opengl_create_perspective_projection_matrix(&GL_projection_matrix, -clip_width, clip_width, -clip_height, clip_height, z_near, z_far);
	}

	GL_CHECK_FOR_ERRORS("end of set_projection_matrix()()");

	GL_htl_projection_matrix_set = 1;
}

void opengl_end_projection_matrix_fixed_pipeline()
{

}

void gr_opengl_end_projection_matrix()
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_end_projection_matrix_fixed_pipeline();
		return;
	}

	GL_CHECK_FOR_ERRORS("start of end_projection_matrix()");

	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	GL_last_projection_matrix = GL_projection_matrix;

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_texture) {
		opengl_create_orthographic_projection_matrix(&GL_projection_matrix, 0, gr_screen.max_w, 0, gr_screen.max_h, -1.0, 1.0);
	} else {
		opengl_create_orthographic_projection_matrix(&GL_projection_matrix, 0, gr_screen.max_w, gr_screen.max_h, 0, -1.0, 1.0);
	}

	GL_CHECK_FOR_ERRORS("end of end_projection_matrix()");

	GL_htl_projection_matrix_set = 0;
}

void opengl_set_view_matrix_fixed_pipeline(const vec3d *pos, const matrix *orient)
{

}

void gr_opengl_set_view_matrix(const vec3d *pos, const matrix *orient)
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_set_view_matrix_fixed_pipeline(pos, orient);
		return;
	}

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_modelview_matrix_depth == 1);

	GL_CHECK_FOR_ERRORS("start of set_view_matrix()");

	opengl_create_view_matrix(&GL_view_matrix, pos, orient);
	
	GL_model_matrix_stack.clear();
	GL_model_view_matrix = GL_view_matrix;

	if (Cmdline_env) {
		GL_env_texture_matrix_set = true;

		// setup the texture matrix which will make the the envmap keep lined
		// up properly with the environment

		// r.xyz  <--  r.x, u.x, f.x
		GL_env_texture_matrix[0] = GL_model_view_matrix.a1d[0];
		GL_env_texture_matrix[1] = GL_model_view_matrix.a1d[4];
		GL_env_texture_matrix[2] = GL_model_view_matrix.a1d[8];
		// u.xyz  <--  r.y, u.y, f.y
		GL_env_texture_matrix[4] = GL_model_view_matrix.a1d[1];
		GL_env_texture_matrix[5] = GL_model_view_matrix.a1d[5];
		GL_env_texture_matrix[6] = GL_model_view_matrix.a1d[9];
		// f.xyz  <--  r.z, u.z, f.z
		GL_env_texture_matrix[8] = GL_model_view_matrix.a1d[2];
		GL_env_texture_matrix[9] = GL_model_view_matrix.a1d[6];
		GL_env_texture_matrix[10] = GL_model_view_matrix.a1d[10];

		GL_env_texture_matrix[15] = 1.0f;
	}

	GL_CHECK_FOR_ERRORS("end of set_view_matrix()");

	GL_modelview_matrix_depth = 2;
	GL_htl_view_matrix_set = 1;
}

void opengl_end_view_matrix_fixed_pipeline()
{

}

void gr_opengl_end_view_matrix()
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_end_view_matrix_fixed_pipeline();
		return;
	}

	Assert(GL_modelview_matrix_depth == 2);

	GL_model_matrix_stack.clear();
	vm_matrix4_set_identity(&GL_view_matrix);
	vm_matrix4_set_identity(&GL_model_view_matrix);

	GL_modelview_matrix_depth = 1;
	GL_htl_view_matrix_set = 0;
	GL_env_texture_matrix_set = false;
}

void opengl_set_2d_matrix_fixed_pipeline()
{

}

// set a view and projection matrix for a 2D element
// TODO: this probably needs to accept values
void gr_opengl_set_2d_matrix(/*int x, int y, int w, int h*/)
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_set_2d_matrix_fixed_pipeline();
		return;
	}

	// don't bother with this if we aren't even going to need it
	if ( !GL_htl_projection_matrix_set ) {
		return;
	}

	Assert( GL_htl_2d_matrix_set == 0 );
	Assert( GL_htl_2d_matrix_depth == 0 );

	// the viewport needs to be the full screen size since glOrtho() is relative to it
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	GL_last_projection_matrix = GL_projection_matrix;

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_texture) {
		opengl_create_orthographic_projection_matrix(&GL_projection_matrix, 0, gr_screen.max_w, 0, gr_screen.max_h, -1, 1);
	} else {
		opengl_create_orthographic_projection_matrix(&GL_projection_matrix, 0, gr_screen.max_w, gr_screen.max_h, 0, -1, 1);
	}

	matrix4 identity_mat;
	vm_matrix4_set_identity(&identity_mat);

	GL_model_matrix_stack.push_and_replace(identity_mat);

	GL_last_view_matrix = GL_view_matrix;
	GL_view_matrix = identity_mat;

	vm_matrix4_x_matrix4(&GL_model_view_matrix, &GL_view_matrix, &identity_mat);

	GL_htl_2d_matrix_set++;
	GL_htl_2d_matrix_depth++;
}

void opengl_end_2d_matrix_fixed_pipeline()
{

}

// ends a previously set 2d view and projection matrix
void gr_opengl_end_2d_matrix()
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_end_2d_matrix_fixed_pipeline();
		return;
	}

	if (!GL_htl_2d_matrix_set)
		return;

	Assert( GL_htl_2d_matrix_depth == 1 );

	// reset viewport to what it was originally set to by the proj matrix
	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);

	GL_projection_matrix = GL_last_projection_matrix;
		
	GL_model_matrix_stack.pop();

	GL_view_matrix = GL_last_view_matrix;

	matrix4 model_matrix = GL_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&GL_model_view_matrix, &GL_view_matrix, &model_matrix);

	GL_htl_2d_matrix_set = 0;
	GL_htl_2d_matrix_depth = 0;
}

static bool GL_scale_matrix_set = false;

void opengl_push_scale_matrix_fixed_pipeline(const vec3d *scale_factor)
{

}

void gr_opengl_push_scale_matrix(const vec3d *scale_factor)
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_push_scale_matrix_fixed_pipeline(scale_factor);
		return;
	}

	if ( (scale_factor->xyz.x == 1) && (scale_factor->xyz.y == 1) && (scale_factor->xyz.z == 1) )
		return;

	GL_scale_matrix_set = true;

	GL_modelview_matrix_depth++;

	GL_model_matrix_stack.push(NULL, NULL, scale_factor);

	matrix4 model_matrix = GL_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&GL_model_view_matrix, &GL_view_matrix, &model_matrix);
}

void opengl_pop_scale_matrix_fixed_pipeline()
{

}

void gr_opengl_pop_scale_matrix()
{
	if ( !is_minimum_GLSL_version() ) {
		opengl_pop_scale_matrix_fixed_pipeline();
		return;
	}

	if (!GL_scale_matrix_set) 
		return;

	GL_model_matrix_stack.pop();

	matrix4 model_matrix = GL_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&GL_model_view_matrix, &GL_view_matrix, &model_matrix);

	GL_modelview_matrix_depth--;
	GL_scale_matrix_set = false;
}

void gr_opengl_end_clip_plane()
{
	if ( is_minimum_GLSL_version() ) {
		return;
	}

	GL_state.ClipPlane(0, GL_FALSE);
}

void gr_opengl_start_clip_plane()
{
	if ( is_minimum_GLSL_version() ) {
		// bail since we're gonna clip in the shader
		return;
	}

	GLdouble clip_equation[4];

	clip_equation[0] = (GLdouble)G3_user_clip_normal.xyz.x;
	clip_equation[1] = (GLdouble)G3_user_clip_normal.xyz.y;
	clip_equation[2] = (GLdouble)G3_user_clip_normal.xyz.z;

	clip_equation[3] = (GLdouble)(G3_user_clip_normal.xyz.x * G3_user_clip_point.xyz.x)
						+ (GLdouble)(G3_user_clip_normal.xyz.y * G3_user_clip_point.xyz.y)
						+ (GLdouble)(G3_user_clip_normal.xyz.z * G3_user_clip_point.xyz.z);
	clip_equation[3] *= -1.0;


	GL_state.ClipPlane(0, GL_TRUE);
}

void gr_opengl_set_clip_plane(vec3d *clip_normal, vec3d *clip_point)
{
	if ( is_minimum_GLSL_version() && Current_shader != NULL && Current_shader->shader == SDR_TYPE_MODEL) {
		return;
	}

	if ( clip_normal == NULL || clip_point == NULL ) {
		GL_state.ClipPlane(0, GL_FALSE);
	} else {
		GLdouble clip_equation[4];

		clip_equation[0] = (GLdouble)clip_normal->xyz.x;
		clip_equation[1] = (GLdouble)clip_normal->xyz.y;
		clip_equation[2] = (GLdouble)clip_normal->xyz.z;

		clip_equation[3] = (GLdouble)(clip_normal->xyz.x * clip_point->xyz.x)
			+ (GLdouble)(clip_normal->xyz.y * clip_point->xyz.y)
			+ (GLdouble)(clip_normal->xyz.z * clip_point->xyz.z);
		clip_equation[3] *= -1.0;


		GL_state.ClipPlane(0, GL_TRUE);
	}
}

//************************************State blocks************************************

//this is an array of reference counts for state block IDs
//static GLuint *state_blocks = NULL;
//static uint n_state_blocks = 0;
//static GLuint current_state_block;

//this is used for places in the array that a state block ID no longer exists
//#define EMPTY_STATE_BOX_REF_COUNT	0xffffffff

int opengl_get_new_state_block_internal()
{
/*	uint i;

	if (state_blocks == NULL) {
		state_blocks = (GLuint*)vm_malloc(sizeof(GLuint));
		memset(&state_blocks[n_state_blocks], 'f', sizeof(GLuint));
		n_state_blocks++;
	}

	for (i = 0; i < n_state_blocks; i++) {
		if (state_blocks[i] == EMPTY_STATE_BOX_REF_COUNT) {
			return i;
		}
	}

	// "i" should be n_state_blocks since we got here.
	state_blocks = (GLuint*)vm_realloc(state_blocks, sizeof(GLuint) * i);
	memset(&state_blocks[i], 'f', sizeof(GLuint));

	n_state_blocks++;

	return n_state_blocks-1;*/
	return -1;
}

void gr_opengl_start_state_block()
{
/*	gr_screen.recording_state_block = true;
	current_state_block = opengl_get_new_state_block_internal();
	glNewList(current_state_block, GL_COMPILE);*/
}

int gr_opengl_end_state_block()
{
/*	//sanity check
	if(!gr_screen.recording_state_block)
		return -1;

	//End the display list
	gr_screen.recording_state_block = false;
	glEndList();

	//now return
	return current_state_block;*/
	return -1;
}

void gr_opengl_set_state_block(int handle)
{
/*	if(handle < 0) return;
	glCallList(handle);*/
}

extern bool Glowpoint_override;
bool Glowpoint_override_save;

void gr_opengl_shadow_map_start(matrix4 *shadow_view_matrix, const matrix *light_orient)
{
	if ( !Cmdline_shadow_quality )
		return;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &saved_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);

	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gr_opengl_set_lighting(false,false);
	
	Rendering_to_shadow_map = true;
	Glowpoint_override_save = Glowpoint_override;
	Glowpoint_override = true;

	GL_htl_projection_matrix_set = 1;
	gr_set_view_matrix(&Eye_position, light_orient);

	*shadow_view_matrix = GL_view_matrix;

	int size = (Cmdline_shadow_quality == 2 ? 1024 : 512);
	glViewport(0, 0, size, size);
}

void gr_opengl_shadow_map_end()
{
		if(!Rendering_to_shadow_map)
			return;

		gr_end_view_matrix();
		Rendering_to_shadow_map = false;

		gr_zbuffer_set(ZBUFFER_TYPE_FULL);
		glBindFramebuffer(GL_FRAMEBUFFER, saved_fb);
		if(saved_fb)
		{
// 			GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
// 			glDrawBuffers(2, buffers);
		}

		Glowpoint_override = Glowpoint_override_save;
		GL_htl_projection_matrix_set = 0;
		
		glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
		glScissor(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
}

void opengl_tnl_set_material(material* material_info, bool set_base_map)
{
	int shader_handle = material_info->get_shader_handle();
	int base_map = material_info->get_texture_map(TM_BASE_TYPE);
	color clr = material_info->get_color();

	if ( shader_handle >= 0 ) {
		opengl_shader_set_current(shader_handle);
	} else {
		opengl_shader_set_passthrough(base_map >= 0, material_info->get_texture_type() == TCACHE_TYPE_AABITMAP, &clr, material_info->get_color_scale());
	}

	GL_state.SetTextureSource(material_info->get_texture_source());
	GL_state.SetAlphaBlendMode(material_info->get_blend_mode());
	GL_state.SetZbufferType(material_info->get_depth_mode());

	if ( !is_minimum_GLSL_version() ) {
		GL_state.Color(clr.red, clr.green, clr.blue, clr.alpha);
	}

	gr_set_cull(material_info->get_cull_mode() ? 1 : 0);

	gr_zbias(material_info->get_depth_bias());

	gr_set_fill_mode(material_info->get_fill_mode());

	material::fog &fog_params = material_info->get_fog();

	if ( fog_params.enabled ) {
		gr_fog_set(GR_FOGMODE_FOG, fog_params.r, fog_params.g, fog_params.b, fog_params.dist_near, fog_params.dist_far);
	} else {
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	gr_set_texture_addressing(material_info->get_texture_addressing());

	material::clip_plane &clip_params = material_info->get_clip_plane();

	if ( clip_params.enabled ) {
		gr_opengl_set_clip_plane(&clip_params.normal, &clip_params.position);
	} else {
		gr_opengl_set_clip_plane(NULL, NULL);
	}

	if ( set_base_map && base_map >= 0 ) {
		float u_scale, v_scale;

		if ( !gr_opengl_tcache_set(base_map, material_info->get_texture_type(), &u_scale, &v_scale) ) {
			mprintf(("WARNING: Error setting bitmap texture (%i)!\n", base_map));
		}
	}
}

void opengl_tnl_set_model_material(model_material *material_info)
{
	float u_scale, v_scale;
	int render_pass = 0;

	opengl_tnl_set_material(material_info, false);

	if ( GL_state.CullFace() ) {
		GL_state.FrontFaceValue(GL_CW);
	}
	
	gr_opengl_set_center_alpha(material_info->get_center_alpha());

	Assert( Current_shader->shader == SDR_TYPE_MODEL );

	GL_state.Texture.SetShaderMode(GL_TRUE);
	
	GL_state.Uniform.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	GL_state.Uniform.setUniformMatrix4f("modelMatrix", GL_model_matrix_stack.get_transform());
	GL_state.Uniform.setUniformMatrix4f("viewMatrix", GL_view_matrix);
	GL_state.Uniform.setUniformMatrix4f("projMatrix", GL_projection_matrix);
	GL_state.Uniform.setUniformMatrix4f("textureMatrix", GL_texture_matrix);

	color &clr = material_info->get_color();
	GL_state.Uniform.setUniform4f("color", clr.red / 255.0f, clr.green / 255.0f, clr.blue / 255.0f, clr.alpha / 255.0f);

	if ( Current_shader->flags & SDR_FLAG_MODEL_ANIMATED ) {
		GL_state.Uniform.setUniformf("anim_timer", material_info->get_animated_effect_time());
		GL_state.Uniform.setUniformi("effect_num", material_info->get_animated_effect());
		GL_state.Uniform.setUniformf("vpwidth", 1.0f / gr_screen.max_w);
		GL_state.Uniform.setUniformf("vpheight", 1.0f / gr_screen.max_h);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_CLIP ) {
		bool clip = material_info->is_clipped();

		if ( clip ) {
			material::clip_plane &clip_info = material_info->get_clip_plane();
			
			GL_state.Uniform.setUniformi("use_clip_plane", 1);
			GL_state.Uniform.setUniform3f("clip_normal", clip_info.normal);
			GL_state.Uniform.setUniform3f("clip_position", clip_info.position);
		} else {
			GL_state.Uniform.setUniformi("use_clip_plane", 0);
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_LIGHT ) {
		int num_lights = MIN(Num_active_gl_lights, GL_max_lights) - 1;
		float light_factor = material_info->get_light_factor();
		GL_state.Uniform.setUniformi("n_lights", num_lights);
		GL_state.Uniform.setUniform4fv("lightPosition", GL_max_lights, opengl_light_uniforms.Position);
		GL_state.Uniform.setUniform3fv("lightDirection", GL_max_lights, opengl_light_uniforms.Direction);
		GL_state.Uniform.setUniform3fv("lightDiffuseColor", GL_max_lights, opengl_light_uniforms.Diffuse_color);
		GL_state.Uniform.setUniform3fv("lightSpecColor", GL_max_lights, opengl_light_uniforms.Spec_color);
		GL_state.Uniform.setUniform1iv("lightType", GL_max_lights, opengl_light_uniforms.Light_type);
		GL_state.Uniform.setUniform1fv("lightAttenuation", GL_max_lights, opengl_light_uniforms.Attenuation);

		if ( !material_info->get_center_alpha() ) {
			GL_state.Uniform.setUniform3f("diffuseFactor", GL_light_color[0] * light_factor, GL_light_color[1] * light_factor, GL_light_color[2] * light_factor);
			GL_state.Uniform.setUniform3f("ambientFactor", GL_light_ambient[0], GL_light_ambient[1], GL_light_ambient[2]);
		} else {
			//GL_state.Uniform.setUniform3f("diffuseFactor", GL_light_true_zero[0], GL_light_true_zero[1], GL_light_true_zero[2]);
			//GL_state.Uniform.setUniform3f("ambientFactor", GL_light_true_zero[0], GL_light_true_zero[1], GL_light_true_zero[2]);
			GL_state.Uniform.setUniform3f("diffuseFactor", GL_light_color[0] * light_factor, GL_light_color[1] * light_factor, GL_light_color[2] * light_factor);
			GL_state.Uniform.setUniform3f("ambientFactor", GL_light_ambient[0], GL_light_ambient[1], GL_light_ambient[2]);
		}

		if ( material_info->get_light_factor() > 0.25f && !Cmdline_no_emissive ) {
			GL_state.Uniform.setUniform3f("emissionFactor", GL_light_emission[0], GL_light_emission[1], GL_light_emission[2]);
		} else {
			GL_state.Uniform.setUniform3f("emissionFactor", GL_light_zero[0], GL_light_zero[1], GL_light_zero[2]);
		}

		GL_state.Uniform.setUniformf("specPower", Cmdline_ogl_spec);

		if ( Gloss_override_set ) {
			GL_state.Uniform.setUniformf("defaultGloss", Gloss_override);
		} else {
			GL_state.Uniform.setUniformf("defaultGloss", 0.6f); // add user configurable default gloss in the command line later
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_DIFFUSE_MAP ) {
		GL_state.Uniform.setUniformi("sBasemap", render_pass);

		if ( material_info->is_desaturated() ) {
			color &clr = material_info->get_color();
			GL_state.Uniform.setUniform3f("desaturate_clr", clr.red / 255.0f, clr.green / 255.0f, clr.blue / 255.0f);

			GL_state.Uniform.setUniformi("desaturate", 1);
		} else {
			GL_state.Uniform.setUniformi("desaturate", 0);
		}

		if ( Basemap_color_override_set ) {
			GL_state.Uniform.setUniformi("overrideDiffuse", 1);
			GL_state.Uniform.setUniform3f("diffuseClr", Basemap_color_override[0], Basemap_color_override[1], Basemap_color_override[2]);
		} else {
			GL_state.Uniform.setUniformi("overrideDiffuse", 0);
		}

		switch ( material_info->get_blend_mode() ) {
		case ALPHA_BLEND_PREMULTIPLIED:
			GL_state.Uniform.setUniformi("blend_alpha", 1);
			break;
		case ALPHA_BLEND_ADDITIVE:
			GL_state.Uniform.setUniformi("blend_alpha", 2);
			break;
		default:
			GL_state.Uniform.setUniformi("blend_alpha", 0);
			break;
		}

		if ( material_info->get_texture_map(TM_UNLIT_TYPE) >= 0 ) {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_UNLIT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);
		} else {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_BASE_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);
		}
		
		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_GLOW_MAP ) {
		GL_state.Uniform.setUniformi("sGlowmap", render_pass);

		if ( Glowmap_color_override_set ) {
			GL_state.Uniform.setUniformi("overrideGlow", 1);
			GL_state.Uniform.setUniform3f("glowClr", Glowmap_color_override[0], Glowmap_color_override[1], Glowmap_color_override[2]);
		} else {
			GL_state.Uniform.setUniformi("overrideGlow", 0);
		}

		gr_opengl_tcache_set(material_info->get_texture_map(TM_GLOW_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SPEC_MAP ) {
		GL_state.Uniform.setUniformi("sSpecmap", render_pass);

		if ( Specmap_color_override_set ) {
			GL_state.Uniform.setUniformi("overrideSpec", 1);
			GL_state.Uniform.setUniform3f("specClr", Specmap_color_override[0], Specmap_color_override[1], Specmap_color_override[2]);
		} else {
			GL_state.Uniform.setUniformi("overrideSpec", 0);
		}

		if ( material_info->get_texture_map(TM_SPEC_GLOSS_TYPE) > 0 ) {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPEC_GLOSS_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);

			GL_state.Uniform.setUniformi("gammaSpec", 1);

			if ( Gloss_override_set ) {
				GL_state.Uniform.setUniformi("alphaGloss", 0);
			} else {
				GL_state.Uniform.setUniformi("alphaGloss", 1);
			}
		} else {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPECULAR_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);

			GL_state.Uniform.setUniformi("gammaSpec", 0);
			GL_state.Uniform.setUniformi("alphaGloss", 0);
		}
		
		++render_pass;

		if ( Current_shader->flags & SDR_FLAG_MODEL_ENV_MAP ) {
			matrix4 texture_mat;

			for ( int i = 0; i < 16; ++i ) {
				texture_mat.a1d[i] = GL_env_texture_matrix[i];
			}

			if ( material_info->get_texture_map(TM_SPEC_GLOSS_TYPE) > 0 || Gloss_override_set ) {
				GL_state.Uniform.setUniformi("envGloss", 1);
			} else {
				GL_state.Uniform.setUniformi("envGloss", 0);
			}

			GL_state.Uniform.setUniformMatrix4f("envMatrix", texture_mat);
			GL_state.Uniform.setUniformi("sEnvmap", render_pass);

			gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, render_pass);

			++render_pass;
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_MAP ) {
		GL_state.Uniform.setUniformi("sNormalmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_NORMAL_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_HEIGHT_MAP ) {
		GL_state.Uniform.setUniformi("sHeightmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_HEIGHT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_MISC_MAP ) {
		GL_state.Uniform.setUniformi("sMiscmap", render_pass);

		gr_opengl_tcache_set(material_info->get_texture_map(TM_MISC_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SHADOWS ) {
		GL_state.Uniform.setUniformMatrix4f("shadow_mv_matrix", Shadow_view_matrix);
		GL_state.Uniform.setUniformMatrix4fv("shadow_proj_matrix", MAX_SHADOW_CASCADES, Shadow_proj_matrix);
		GL_state.Uniform.setUniformf("veryneardist", Shadow_cascade_distances[0]);
		GL_state.Uniform.setUniformf("neardist", Shadow_cascade_distances[1]);
		GL_state.Uniform.setUniformf("middist", Shadow_cascade_distances[2]);
		GL_state.Uniform.setUniformf("fardist", Shadow_cascade_distances[3]);
		GL_state.Uniform.setUniformi("shadow_map", render_pass);

		GL_state.Texture.SetActiveUnit(render_pass);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY_EXT);
		GL_state.Texture.Enable(Shadow_map_texture);

		++render_pass; // bump!
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SHADOW_MAP ) {
		GL_state.Uniform.setUniformMatrix4fv("shadow_proj_matrix", MAX_SHADOW_CASCADES, Shadow_proj_matrix);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_ANIMATED ) {
		GL_state.Uniform.setUniformi("sFramebuffer", render_pass);

		GL_state.Texture.SetActiveUnit(render_pass);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);

		if ( Scene_framebuffer_in_frame ) {
			GL_state.Texture.Enable(Scene_effect_texture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		} else {
			GL_state.Texture.Enable(Framebuffer_fallback_texture_id);
		}

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_TRANSFORM ) {
		GL_state.Uniform.setUniformi("transform_tex", render_pass);
		GL_state.Uniform.setUniformi("buffer_matrix_offset", (int)GL_transform_buffer_offset);
		
		GL_state.Texture.SetActiveUnit(render_pass);
		GL_state.Texture.SetTarget(GL_TEXTURE_BUFFER);
		GL_state.Texture.Enable(opengl_get_transform_buffer_texture());

		++render_pass;
	}

	// Team colors are passed to the shader here, but the shader needs to handle their application.
	// By default, this is handled through the r and g channels of the misc map, but this can be changed
	// in the shader; test versions of this used the normal map r and b channels
	if ( Current_shader->flags & SDR_FLAG_MODEL_TEAMCOLOR ) {
		team_color &tm_clr = material_info->get_team_color();
		vec3d stripe_color;
		vec3d base_color;

		stripe_color.xyz.x = tm_clr.stripe.r;
		stripe_color.xyz.y = tm_clr.stripe.g;
		stripe_color.xyz.z = tm_clr.stripe.b;

		base_color.xyz.x = tm_clr.base.r;
		base_color.xyz.y = tm_clr.base.g;
		base_color.xyz.z = tm_clr.base.b;

		GL_state.Uniform.setUniform3f("stripe_color", stripe_color);
		GL_state.Uniform.setUniform3f("base_color", base_color);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_THRUSTER ) {
		GL_state.Uniform.setUniformf("thruster_scale", material_info->get_thrust_scale());
	}

	
	if ( Current_shader->flags & SDR_FLAG_MODEL_FOG ) {
		material::fog fog_params = material_info->get_fog();

		if ( fog_params.enabled ) {
			GL_state.Uniform.setUniformf("fogStart", fog_params.dist_near);
			GL_state.Uniform.setUniformf("fogScale", 1.0f / (fog_params.dist_far - fog_params.dist_near));
			GL_state.Uniform.setUniform4f("fogColor", i2fl(fog_params.r) / 255.0f, i2fl(fog_params.g) / 255.0f, i2fl(fog_params.b) / 255.0f, 1.0f);
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_ALPHA ) {
		GL_state.Uniform.setUniform2f("normalAlphaMinMax", material_info->get_normal_alpha_min(), material_info->get_normal_alpha_max());
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_EXTRUDE ) {
		GL_state.Uniform.setUniformf("extrudeWidth", material_info->get_normal_extrude_width());
	}

	if ( Deferred_lighting ) {
		// don't blend if we're drawing to the g-buffers
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	}
}

void opengl_tnl_set_material_particle(particle_material * material_info)
{
	opengl_tnl_set_material(material_info, true);

	color &clr = material_info->get_color();

	GL_state.Uniform.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	GL_state.Uniform.setUniformMatrix4f("projMatrix", GL_projection_matrix);

	GL_state.Uniform.setUniformi("baseMap", 0);
	GL_state.Uniform.setUniformi("depthMap", 1);
	GL_state.Uniform.setUniformf("window_width", (float)gr_screen.max_w);
	GL_state.Uniform.setUniformf("window_height", (float)gr_screen.max_h);
	GL_state.Uniform.setUniformf("nearZ", Min_draw_distance);
	GL_state.Uniform.setUniformf("farZ", Max_draw_distance);
	GL_state.Uniform.setUniformi("srgb", High_dynamic_range ? 1 : 0);
	GL_state.Uniform.setUniformi("blend_alpha", material_info->get_blend_mode() != ALPHA_BLEND_ADDITIVE);

	if ( Cmdline_no_deferred_lighting ) {
		GL_state.Uniform.setUniformi("linear_depth", 0);
	} else {
		GL_state.Uniform.setUniformi("linear_depth", 1);
	}

	if ( !Cmdline_no_deferred_lighting ) {
		Assert(Scene_position_texture != 0);

		GL_state.Texture.SetActiveUnit(1);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Scene_position_texture);
	} else {
		Assert(Scene_depth_texture != 0);

		GL_state.Texture.SetActiveUnit(1);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Scene_depth_texture);
	}
}

void opengl_tnl_set_material_distortion(distortion_material* material_info)
{
	opengl_tnl_set_material(material_info, true);

	GL_state.Uniform.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	GL_state.Uniform.setUniformMatrix4f("projMatrix", GL_projection_matrix);

	GL_state.Uniform.setUniformi("baseMap", 0);
	GL_state.Uniform.setUniformi("depthMap", 1);
	GL_state.Uniform.setUniformf("window_width", (float)gr_screen.max_w);
	GL_state.Uniform.setUniformf("window_height", (float)gr_screen.max_h);
	GL_state.Uniform.setUniformf("nearZ", Min_draw_distance);
	GL_state.Uniform.setUniformf("farZ", Max_draw_distance);
	GL_state.Uniform.setUniformi("frameBuffer", 2);

	GL_state.Texture.SetActiveUnit(2);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_effect_texture);

	if(material_info->get_thruster_rendering()) {
		GL_state.Uniform.setUniformi("distMap", 3);

		GL_state.Texture.SetActiveUnit(3);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Distortion_texture[!Distortion_switch]);
		GL_state.Uniform.setUniformf("use_offset", 1.0f);
	} else {
		GL_state.Uniform.setUniformi("distMap", 0);
		GL_state.Uniform.setUniformf("use_offset", 0.0f);
	}

	Assert(Scene_depth_texture != 0);

	GL_state.Texture.SetActiveUnit(1);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_depth_texture);
}

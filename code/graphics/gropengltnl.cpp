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
#include "globalincs/alphacolors.h"
#include "globalincs/def_files.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengltnl.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "weapon/trails.h"

extern int GLOWMAP;
extern int CLOAKMAP;
extern int SPECMAP;
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
extern bool GLSL_override;
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

int GL_vertex_data_in = 0;

GLint GL_max_elements_vertices = 4096;
GLint GL_max_elements_indices = 4096;

float GL_thrust_scale = -1.0f;
team_color* Current_team_color;
team_color Current_temp_color;
bool Using_Team_Color = false;

int GL_transform_buffer_offset = -1;

GLuint Shadow_map_texture = 0;
GLuint Shadow_map_depth_texture = 0;
GLuint shadow_fbo = 0;
GLint saved_fb = 0;
bool Rendering_to_shadow_map = false;

int Transform_buffer_handle = -1;

struct opengl_buffer_object {
	GLuint buffer_id;
	GLenum type;
	GLenum usage;
	uint size;

	GLuint texture;	// for texture buffer objects
};

struct opengl_vertex_buffer {
	GLfloat *array_list;	// interleaved array
	GLubyte *index_list;

	int vb_handle;
	int ib_handle;

	uint vbo_size;
	uint ibo_size;

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
		opengl_delete_buffer_object(vb_handle);
	}

	if ( ib_handle >= 0 ) {
		opengl_delete_buffer_object(ib_handle);
	}
}

static SCP_vector<opengl_buffer_object> GL_buffer_objects;
static SCP_vector<opengl_vertex_buffer> GL_vertex_buffers;
static opengl_vertex_buffer *g_vbp = NULL;
static int GL_vertex_buffers_in_use = 0;

int opengl_create_buffer_object(GLenum type, GLenum usage)
{
	opengl_buffer_object buffer_obj;

	buffer_obj.usage = usage;
	buffer_obj.type = type;
	buffer_obj.size = 0;

	vglGenBuffersARB(1, &buffer_obj.buffer_id);

	GL_buffer_objects.push_back(buffer_obj);

	return GL_buffer_objects.size() - 1;
}

void opengl_bind_buffer_object(int handle)
{
	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	switch ( buffer_obj.type ) {
	case GL_ARRAY_BUFFER_ARB:
		GL_state.Array.BindArrayBuffer(buffer_obj.buffer_id);
		break;
	case GL_ELEMENT_ARRAY_BUFFER_ARB:
		GL_state.Array.BindElementBuffer(buffer_obj.buffer_id);
		break;
	case GL_TEXTURE_BUFFER_ARB:
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

void gr_opengl_update_buffer_object(int handle, uint size, void* data)
{
	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	opengl_bind_buffer_object(handle);

	GL_vertex_data_in -= buffer_obj.size;
	buffer_obj.size = size;
	GL_vertex_data_in += buffer_obj.size;

	vglBufferDataARB(buffer_obj.type, size, data, buffer_obj.usage);
}

void opengl_delete_buffer_object(int handle)
{
	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	if ( buffer_obj.type == GL_TEXTURE_BUFFER_ARB ) {
		glDeleteTextures(1, &buffer_obj.texture);
	}

	GL_vertex_data_in -= buffer_obj.size;

	vglDeleteBuffersARB(1, &buffer_obj.buffer_id);
}

int gr_opengl_create_stream_buffer_object()
{
	if (!Use_VBOs) {
		return -1;
	}

	return opengl_create_buffer_object(GL_ARRAY_BUFFER_ARB, GL_STREAM_DRAW_ARB);
}

int opengl_create_texture_buffer_object()
{
	if ( Use_GLSL < 3 || !Is_Extension_Enabled(OGL_ARB_TEXTURE_BUFFER) || !Is_Extension_Enabled(OGL_ARB_FLOATING_POINT_TEXTURES) ) {
		return -1;
	}

	// create the buffer
	int buffer_object_handle = opengl_create_buffer_object(GL_TEXTURE_BUFFER_ARB, GL_DYNAMIC_DRAW_ARB);

	opengl_check_for_errors();

	opengl_buffer_object &buffer_obj = GL_buffer_objects[buffer_object_handle];

	// create the texture
	glGenTextures(1, &buffer_obj.texture);
	glBindTexture(GL_TEXTURE_BUFFER_ARB, buffer_obj.texture);

	opengl_check_for_errors();

	gr_opengl_update_buffer_object(buffer_object_handle, 100, NULL);

	vglTexBufferARB(GL_TEXTURE_BUFFER_ARB, GL_RGBA32F_ARB, buffer_obj.buffer_id);

	opengl_check_for_errors();

	return buffer_object_handle;
}

void gr_opengl_update_transform_buffer(void* data, uint size)
{
	if ( Transform_buffer_handle < 0 || size <= 0 ) {
		return;
	}

	gr_opengl_update_buffer_object(Transform_buffer_handle, size, data);
}

GLuint opengl_get_transform_buffer_texture()
{
	if ( Transform_buffer_handle < 0 ) {
		return 0;
	}

	return GL_buffer_objects[Transform_buffer_handle].texture;
}

void gr_opengl_set_transform_buffer_offset(int offset)
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

		vbp->vb_handle = opengl_create_buffer_object(GL_ARRAY_BUFFER_ARB, GL_STATIC_DRAW_ARB);

		// make sure we have one
		if ( vbp->vb_handle >= 0 ) {
			gr_opengl_update_buffer_object(vbp->vb_handle, vbp->vbo_size, vbp->array_list);

			// just in case
			if ( opengl_check_for_errors() ) {
				opengl_delete_buffer_object(vbp->vb_handle);
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

		vbp->ib_handle = opengl_create_buffer_object(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_STATIC_DRAW_ARB);

		// make sure we have one
		if ( vbp->ib_handle >= 0 ) {
			gr_opengl_update_buffer_object(vbp->ib_handle, vbp->ibo_size, vbp->index_list);

			// just in case
			if ( opengl_check_for_errors() ) {
				opengl_delete_buffer_object(vbp->ib_handle);
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
	if (Cmdline_nohtl) {
		return -1;
	}

	opengl_vertex_buffer vbuffer;

	GL_vertex_buffers.push_back( vbuffer );
	GL_vertex_buffers_in_use++;

	return (int)(GL_vertex_buffers.size() - 1);
}

bool gr_opengl_config_buffer(const int buffer_id, vertex_buffer *vb, bool update_ibuffer_only)
{
	if (Cmdline_nohtl) {
		return false;
	}

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
		Assert( Use_GLSL >= 3 );

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
	if (Cmdline_nohtl) {
		return false;
	}

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

	int i, n_verts = 0;
	size_t j;
	uint arsize = 0;


	if (m_vbp->array_list == NULL) {
		m_vbp->array_list = (GLfloat*)vm_malloc_q(m_vbp->vbo_size);

		// return invalid if we don't have the memory
		if (m_vbp->array_list == NULL) {
			return false;
		}

		memset(m_vbp->array_list, 0, m_vbp->vbo_size);
	}

	if (m_vbp->index_list == NULL) {
		m_vbp->index_list = (GLubyte*)vm_malloc_q(m_vbp->ibo_size);

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
	for (i = 0; i < n_verts; i++) {
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
		uint offset = tex_buf->index_offset;
		const uint *index = tex_buf->get_index();

		// bump to our spot in the buffer
		GLubyte *ibuf = m_vbp->index_list + offset;

		if (vb->tex_buf[j].flags & VB_FLAG_LARGE_INDEX) {
			memcpy(ibuf, index, n_verts * sizeof(uint));
		} else {
			ushort *mybuf = (ushort*)ibuf;

			for (i = 0; i < n_verts; i++) {
				mybuf[i] = (ushort)index[i];
			}
		}
	}
	
	return true;
}

void gr_opengl_set_buffer(int idx)
{
	if (Cmdline_nohtl) {
		return;
	}

	g_vbp = NULL;

	if (idx < 0) {
		if (Use_VBOs) {
			GL_state.Array.BindArrayBuffer(0);
			GL_state.Array.BindElementBuffer(0);
		}

		if ( (Use_GLSL > 1) && !GLSL_override ) {
			opengl_shader_set_current();
		}

		return;
	}

	Assert( idx < (int)GL_vertex_buffers.size() );

	g_vbp = &GL_vertex_buffers[idx];
}

void gr_opengl_destroy_buffer(int idx)
{
	if (Cmdline_nohtl) {
		return;
	}

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
		opengl_delete_buffer_object(i);
	}

	GL_vertex_buffers.clear();
	GL_vertex_buffers_in_use = 0;
}

void opengl_tnl_init()
{
	GL_vertex_buffers.reserve(MAX_POLYGON_MODELS);
	gr_opengl_deferred_light_sphere_init(16, 16);
	gr_opengl_deferred_light_cylinder_init(16);

	Transform_buffer_handle = opengl_create_texture_buffer_object();

	if ( Transform_buffer_handle < 0 ) {
		Cmdline_no_batching = true;
	}

	if(Cmdline_shadow_quality)
	{
		//Setup shadow map framebuffer
		vglGenFramebuffersEXT(1, &shadow_fbo);
		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadow_fbo);

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
		vglTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_DEPTH_COMPONENT32, size, size, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		vglFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, Shadow_map_depth_texture, 0);
		//vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, Shadow_map_depth_texture, 0);

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
		vglTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGB32F_ARB, size, size, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size, size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		vglFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, Shadow_map_texture, 0);
		//vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Shadow_map_texture, 0);

		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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

static void opengl_init_arrays(opengl_vertex_buffer *vbp, const vertex_buffer *bufferp)
{
	GLint offset = (GLint)bufferp->vertex_offset;
	GLubyte *ptr = NULL;

	if ( Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
		offset = 0;
	}

	// vertex buffer

	if ( vbp->vb_handle >= 0 ) {
		opengl_bind_buffer_object(vbp->vb_handle);
	} else {
		ptr = (GLubyte*)vbp->array_list;
	}

	vertex_layout vert_def;

	if (bufferp->flags & VB_FLAG_UV1) {
		vert_def.add_vertex_component(vertex_format_data::TEX_COORD, bufferp->stride, ptr + offset);
		offset += (2 * sizeof(GLfloat));
	}

	if (bufferp->flags & VB_FLAG_NORMAL) {
		vert_def.add_vertex_component(vertex_format_data::NORMAL, bufferp->stride, ptr + offset);
		offset += (3 * sizeof(GLfloat));
	}

	if (bufferp->flags & VB_FLAG_TANGENT) {
		// we treat this as texture coords for ease of use
		// NOTE: this is forced on tex unit 1!!!
		vert_def.add_vertex_component(vertex_format_data::TANGENT, bufferp->stride, ptr + offset);
		offset += (4 * sizeof(GLfloat));
	}

	if (bufferp->flags & VB_FLAG_MODEL_ID) {
		vert_def.add_vertex_component(vertex_format_data::MODEL_ID, bufferp->stride, ptr + offset);
		offset += (1 * sizeof(GLfloat));
	}

	Assert( bufferp->flags & VB_FLAG_POSITION );
	vert_def.add_vertex_component(vertex_format_data::POSITION3, bufferp->stride, ptr + offset);
	offset += (3 * sizeof(GLfloat));

	opengl_bind_vertex_layout(vert_def);
}

#define DO_RENDER()	\
	if (Cmdline_drawelements) \
		glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start)); \
	else \
		vglDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));

unsigned int GL_last_shader_flags = 0;
int GL_last_shader_index = -1;

static void opengl_render_pipeline_fixed(int start, const vertex_buffer *bufferp, const buffer_data *datap, int flags);

extern bool Scene_framebuffer_in_frame;
extern GLuint Framebuffer_fallback_texture_id;
extern matrix Object_matrix;
extern vec3d Object_position;
extern int Interp_thrust_scale_subobj;
extern float Interp_thrust_scale;
static void opengl_render_pipeline_program(int start, const vertex_buffer *bufferp, const buffer_data *datap, int flags)
{
	unsigned int shader_flags = 0;
	int sdr_index = -1;
	int r, g, b, a, tmap_type;
	GLubyte *ibuffer = NULL;

	int end = (datap->n_verts - 1);
	int count = (end - (start*3) + 1);

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
		(flags & TMAP_FLAG_BATCH_TRANSFORMS) && (GL_transform_buffer_offset >= 0) && (bufferp->flags & VB_FLAG_MODEL_ID),
		Using_Team_Color, 
		flags, 
		SPECMAP, 
		GLOWMAP, 
		NORMMAP, 
		HEIGHTMAP, 
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

	opengl_tnl_set_material(flags, shader_flags, tmap_type);
	
	if(Rendering_to_shadow_map) {
		vglDrawElementsInstancedBaseVertex(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start), 4, (GLint)bufferp->vertex_offset/bufferp->stride);
	} else {
		if ( Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
			if (Cmdline_drawelements) {
				vglDrawElementsBaseVertex(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
			} else {
				vglDrawRangeElementsBaseVertex(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
			}
		} else {
			if (Cmdline_drawelements) {
				glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start)); 
			} else {
				vglDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));
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
		vglUniform1iARB( opengl_shader_get_uniform("n_lights"), n_lights );

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

static void opengl_render_pipeline_fixed(int start, const vertex_buffer *bufferp, const buffer_data *datap, int flags)
{
	float u_scale, v_scale;
	int render_pass = 0;
	int r, g, b, a, tmap_type;
	GLubyte *ibuffer = NULL;
	GLubyte *vbuffer = NULL;

	bool rendered_env = false;
	bool using_glow = false;
	bool using_spec = false;
	bool using_env = false;

	int end = (datap->n_verts - 1);
	int count = (end - start + 1);

	GLenum element_type = (datap->flags & VB_FLAG_LARGE_INDEX) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

	opengl_vertex_buffer *vbp = g_vbp;
	Assert( vbp );

	int textured = ((flags & TMAP_FLAG_TEXTURED) && (bufferp->flags & VB_FLAG_UV1));

	if (textured ) {
		if ( Cmdline_glow && (GLOWMAP > 0) ) {
			using_glow = true;
		}

		if (lighting_is_enabled) {
			GL_state.Normalize(GL_TRUE);

			if ( !GL_state.Fog() && (SPECMAP > 0) && !Specmap_override ) {
				using_spec = true;

				if ( (ENVMAP > 0) && !Envmap_override ) {
					using_env = true;
				}
			}
		}
	}

	render_pass = 0;

	opengl_default_light_settings( !GL_center_alpha, (GL_light_factor > 0.25f), (using_spec) ? 0 : 1 );
	gr_opengl_set_center_alpha(GL_center_alpha);

	opengl_setup_render_states(r, g, b, a, tmap_type, flags);
	GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );

	// basic setup of all data
	opengl_init_arrays(vbp, bufferp);

	if ( vbp->ib_handle >= 0 ) {
		opengl_bind_buffer_object(vbp->ib_handle);
	} else {
		ibuffer = (GLubyte*)vbp->index_list;
	}

	if ( vbp->vb_handle < 0 ) {
		vbuffer = (GLubyte*)vbp->array_list;
	}

	// if we're not doing an alpha pass, turn on the alpha mask
	if ( !(flags & TMAP_FLAG_ALPHA) ) {
		gr_alpha_mask_set(1, 0.95f);
	}

	#define BUFFER_OFFSET(off) (vbuffer+bufferp->vertex_offset+(off))

// -------- Begin 1st PASS (base texture, glow) ---------------------------------- //
	if (textured) {
		render_pass = 0;

		// base texture
		if ( !Basemap_override ) {
			if ( !Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
				GL_state.Array.SetActiveClientUnit(render_pass);
				GL_state.Array.EnableClientTexture();
				GL_state.Array.TexPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );
			}

			gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, render_pass);

			// increment texture count for this pass
			render_pass++; // bump!
		}

		// glowmaps!
		if (using_glow) {
			GL_state.Array.SetActiveClientUnit(render_pass);
			GL_state.Array.EnableClientTexture();
			if ( Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
				GL_state.Array.TexPointer( 2, GL_FLOAT, bufferp->stride, 0 );
			} else {
				GL_state.Array.TexPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );
			}

			// set glowmap on relevant ARB
			gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, render_pass);

			opengl_set_additive_tex_env();

			render_pass++; // bump!
		}
	}

	// DRAW IT!!
	if ( Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
		if (Cmdline_drawelements) {
			vglDrawElementsBaseVertex(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
		} else {
			vglDrawRangeElementsBaseVertex(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
		}
	} else {
		if (Cmdline_drawelements) {
			glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start)); 
		} else {
			vglDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));
		}
	}

// -------- End 2nd PASS --------------------------------------------------------- //


// -------- Begin 2nd pass (additional lighting) --------------------------------- //
/*	if ( (textured) && (lighting_is_enabled) && !(GL_state.Fog()) && (Num_active_gl_lights > GL_max_lights) ) {
		// the lighting code needs to do this better, may need some adjustment later since I'm only trying
		// to avoid rendering 7+ extra passes for lights which probably won't affect current object, but as
		// a performance hack I guess this will have to do for now...
		// restrict the number of extra lighting passes based on LOD:
		//  - LOD0:  only 2 extra passes (3 main passes total, rendering 24 light sources)
		//  - LOD1:  only 1 extra pass   (2 main passes total, rendering 16 light sources)
		//  - LOD2+: no extra passes     (1 main pass   total, rendering  8 light sources)
		extern int Interp_detail_level;
		int max_passes = (2 - Interp_detail_level);

		if (max_passes > 0) {
			int max_lights = (Num_active_gl_lights - 1) / GL_max_lights;

			if (max_lights > 0) {
				int i;

				opengl_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );

				for (i = 1; i < render_pass; i++) {
					opengl_switch_arb(i, 0);
				}

				for (i = 1; (i < max_lights) && (i < max_passes); i++) {
					opengl_change_active_lights(i);

					// DRAW IT!!
					DO_RENDER();
				}

				// reset the active lights to the first set to render the spec related passes with
				// for performance and quality reasons they don't get special lighting passes
				opengl_change_active_lights(0);
			}
		}
	}*/
// -------- End 2nd PASS --------------------------------------------------------- //


// -------- Begin 3rd PASS (environment map) ------------------------------------- //
	if (using_env) {
		// turn all previously used arbs off before the specular pass
		// this fixes the glowmap multitexture rendering problem - taylor
		GL_state.Texture.DisableAll();

		render_pass = 0;

		// set specmap, for us to modulate against
		if ( !Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
			GL_state.Array.SetActiveClientUnit(render_pass);
			GL_state.Array.EnableClientTexture();
			GL_state.Array.TexPointer(2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );
		}

		// set specmap on relevant ARB
		gr_opengl_tcache_set(SPECMAP, tmap_type, &u_scale, &v_scale, render_pass);

		GL_state.DepthMask(GL_TRUE);
		GL_state.DepthFunc(GL_LEQUAL);

		// as a crazy and sometimes useless hack, avoid using alpha when specmap has none
		if ( bm_has_alpha_channel(SPECMAP) ) {
			GL_state.Texture.SetEnvCombineMode(GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_ALPHA );
			glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB );
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR );
			GL_state.Texture.SetRGBScale(1.0f);
			GL_state.Texture.SetAlphaScale(1.0f);
		} else {
			GL_state.Texture.SetEnvCombineMode(GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
			glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB );
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR );
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );
			GL_state.Texture.SetRGBScale(1.0f);
		}

		render_pass++; // bump!

		// now move the to the envmap
		if ( !Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
			GL_state.Array.SetActiveClientUnit(render_pass);
			GL_state.Array.EnableClientTexture();
			GL_state.Array.TexPointer(2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );
		}

		gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, render_pass);

		GL_state.Texture.SetEnvCombineMode(GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

		GL_state.Texture.SetRGBScale(2.0f);

		GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);

		GL_state.Texture.SetWrapS(GL_CLAMP_TO_EDGE);
		GL_state.Texture.SetWrapT(GL_CLAMP_TO_EDGE);
		GL_state.Texture.SetWrapR(GL_CLAMP_TO_EDGE);

		GL_state.Texture.SetTexgenModeS(GL_REFLECTION_MAP);
		GL_state.Texture.SetTexgenModeT(GL_REFLECTION_MAP);
		GL_state.Texture.SetTexgenModeR(GL_REFLECTION_MAP);

		GL_state.Texture.TexgenS(GL_TRUE);
		GL_state.Texture.TexgenT(GL_TRUE);
		GL_state.Texture.TexgenR(GL_TRUE);

		// set the matrix for the texture mode
		if (GL_env_texture_matrix_set) {
			glMatrixMode(GL_TEXTURE);
			glPushMatrix();
			glLoadMatrixf(GL_env_texture_matrix);
			// switch back to the default modelview mode
			glMatrixMode(GL_MODELVIEW);
		}

		render_pass++; // bump!

		GLfloat ambient_save[4];
		glGetMaterialfv( GL_FRONT, GL_AMBIENT, ambient_save );

		GLfloat ambient[4] = { 0.47f, 0.47f, 0.47f, 1.0f };
		glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );

		// DRAW IT!!
		if ( Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
			if (Cmdline_drawelements) {
				vglDrawElementsBaseVertex(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
			} else {
				vglDrawRangeElementsBaseVertex(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
			}
		} else {
			if (Cmdline_drawelements) {
				glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start)); 
			} else {
				vglDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));
			}
		}

		// disable and reset everything we changed
		GL_state.Texture.SetRGBScale(1.0f);

		// reset original ambient light value
		glMaterialfv( GL_FRONT, GL_AMBIENT, ambient_save );

		// pop off the texture matrix we used for the envmap
		if (GL_env_texture_matrix_set) {
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		GL_state.Texture.TexgenS(GL_FALSE);
		GL_state.Texture.TexgenT(GL_FALSE);
		GL_state.Texture.TexgenR(GL_FALSE);

		opengl_set_texture_target();

		GL_state.Texture.SetActiveUnit(0);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);

		rendered_env = true;
	}
// -------- End 3rd PASS --------------------------------------------------------- //


// -------- Begin 4th PASS (specular/shine map) ---------------------------------- //
	if (using_spec) {
		// turn all previously used arbs off before the specular pass
		// this fixes the glowmap multitexture rendering problem - taylor
		GL_state.Texture.DisableAll();
		GL_state.Array.SetActiveClientUnit(1);
		GL_state.Array.DisableClientTexture();

		render_pass = 0;

		if ( !Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
			GL_state.Array.SetActiveClientUnit(0);
			GL_state.Array.EnableClientTexture();
			GL_state.Array.TexPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );
		}

		gr_opengl_tcache_set(SPECMAP, tmap_type, &u_scale, &v_scale, render_pass);

		// render with spec lighting only
		opengl_default_light_settings(0, 0, 1);

		GL_state.Texture.SetEnvCombineMode(GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

		GL_state.Texture.SetRGBScale( (rendered_env) ? 2.0f : 4.0f );

		GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);

		GL_state.DepthMask(GL_TRUE);
		GL_state.DepthFunc(GL_LEQUAL);

		// DRAW IT!!
		if ( Is_Extension_Enabled(OGL_ARB_DRAW_ELEMENTS_BASE_VERTEX) ) {
			if (Cmdline_drawelements) {
				vglDrawElementsBaseVertex(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
			} else {
				vglDrawRangeElementsBaseVertex(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_offset/bufferp->stride);
			}
		} else {
			if (Cmdline_drawelements) {
				glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start)); 
			} else {
				vglDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));
			}
		}

		opengl_default_light_settings();

		GL_state.Texture.SetRGBScale(1.0f);
	}
// -------- End 4th PASS --------------------------------------------------------- //

	// make sure everthing gets turned back off
	gr_alpha_mask_set(0, 1.0f);
	GL_state.Texture.DisableAll();
	GL_state.Normalize(GL_FALSE);
	GL_state.Array.SetActiveClientUnit(1);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
	GL_state.Array.DisableClientTexture();
	GL_state.Array.SetActiveClientUnit(0);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
	GL_state.Array.DisableClientTexture();
	GL_state.Array.DisableClientVertex();
	GL_state.Array.DisableClientNormal();
}

// start is the first part of the buffer to render, n_prim is the number of primitives, index_list is an index buffer, if index_list == NULL render non-indexed
void gr_opengl_render_buffer(int start, const vertex_buffer *bufferp, int texi, int flags)
{
	Assert( GL_htl_projection_matrix_set );
	Assert( GL_htl_view_matrix_set );

	Verify( bufferp != NULL );

	GL_CHECK_FOR_ERRORS("start of render_buffer()");

	if ( GL_state.CullFace() ) {
		GL_state.FrontFaceValue(GL_CW);
	}

	Assert( texi >= 0 );

	const buffer_data *datap = &bufferp->tex_buf[texi];

	if ( (Use_GLSL > 1) && !GLSL_override ) {
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
void gr_opengl_render_stream_buffer(int buffer_handle, int offset, int n_verts, int flags)
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
		if ( flags & TMAP_FLAG_SOFT_QUAD ) {
			if( (flags & TMAP_FLAG_DISTORTION) || (flags & TMAP_FLAG_DISTORTION_THRUSTER) ) {
				opengl_tnl_set_material_distortion(flags);

				glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

				if ( radius_offset >= 0 ) {
					vert_def.add_vertex_component(vertex_format_data::RADIUS, stride, ptr + radius_offset);
				}
				gr_zbuffer_set(GR_ZBUFF_READ);
			} else if ( Cmdline_softparticles ) {
				opengl_tnl_set_material_soft_particle(flags);

				gr_zbuffer_set(GR_ZBUFF_NONE);

				if ( radius_offset >= 0 ) {
					vert_def.add_vertex_component(vertex_format_data::RADIUS, stride, ptr + radius_offset);
				}

				if ( up_offset >= 0 ) {
					vert_def.add_vertex_component(vertex_format_data::UVEC, stride, ptr + up_offset);
				}
			}
		}

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
	} else if (flags & TMAP_FLAG_QUADLIST) {
		gl_mode = GL_QUADS;
	} else if (flags & TMAP_FLAG_QUADSTRIP) {
		gl_mode = GL_QUAD_STRIP;
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

	glDrawArrays(gl_mode, offset, n_verts);

	if( (flags & TMAP_FLAG_DISTORTION) || (flags & TMAP_FLAG_DISTORTION_THRUSTER) ) {
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
		vglDrawBuffers(2, buffers);
	}
	
	GL_CHECK_FOR_ERRORS("end of render3d()");
}

void gr_opengl_start_instance_matrix(const vec3d *offset, const matrix *rotation)
{
	if (Cmdline_nohtl) {
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

	glPushMatrix();

	vec3d axis;
	float ang;
	vm_matrix_to_rot_axis_and_angle(rotation, &ang, &axis);

	glTranslatef( offset->xyz.x, offset->xyz.y, offset->xyz.z );
	if (fl_abs(ang) > 0.0f) {
		glRotatef( fl_degrees(ang), axis.xyz.x, axis.xyz.y, axis.xyz.z );
	}

	GL_CHECK_FOR_ERRORS("end of start_instance_matrix()");

	GL_modelview_matrix_depth++;
}

void gr_opengl_start_instance_angles(const vec3d *pos, const angles *rotation)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	matrix m;
	vm_angles_2_matrix(&m, rotation);

	gr_opengl_start_instance_matrix(pos, &m);
}

void gr_opengl_end_instance_matrix()
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	glPopMatrix();

	GL_modelview_matrix_depth--;
}

// the projection matrix; fov, aspect ratio, near, far
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
	if (Cmdline_nohtl) {
		return;
	}

	GL_CHECK_FOR_ERRORS("start of set_projection_matrix()()");
	
	if (GL_rendering_to_texture) {
		glViewport(gr_screen.offset_x, gr_screen.offset_y, gr_screen.clip_width, gr_screen.clip_height);
	} else {
		glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
	}
	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble clip_width, clip_height;

	clip_height = tan( (double)fov * 0.5 ) * z_near;
	clip_width = clip_height * (GLdouble)aspect;

	if (GL_rendering_to_texture) {
		glFrustum( -clip_width, clip_width, clip_height, -clip_height, z_near, z_far );
	} else {
		glFrustum( -clip_width, clip_width, -clip_height, clip_height, z_near, z_far );
	}

	glMatrixMode(GL_MODELVIEW);

	GL_CHECK_FOR_ERRORS("end of set_projection_matrix()()");

	GL_htl_projection_matrix_set = 1;
}

void gr_opengl_end_projection_matrix()
{
	if (Cmdline_nohtl) {
		return;
	}

	GL_CHECK_FOR_ERRORS("start of end_projection_matrix()");

	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_texture) {
		glOrtho(0, gr_screen.max_w, 0, gr_screen.max_h, -1.0, 1.0);
	} else {
		glOrtho(0, gr_screen.max_w, gr_screen.max_h, 0, -1.0, 1.0);
	}

	glMatrixMode(GL_MODELVIEW);

	GL_CHECK_FOR_ERRORS("end of end_projection_matrix()");

	GL_htl_projection_matrix_set = 0;
}

void gr_opengl_set_view_matrix(const vec3d *pos, const matrix *orient)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_modelview_matrix_depth == 1);

	GL_CHECK_FOR_ERRORS("start of set_view_matrix()");

	glPushMatrix();

	// right now it depends on your settings as to whether this has any effect in-mission
	// not much good now, but should be a bit more useful later on
	if ( !memcmp(pos, &last_view_pos, sizeof(vec3d)) && !memcmp(orient, &last_view_orient, sizeof(matrix)) ) {
		use_last_view = true;
	} else {
		memcpy(&last_view_pos, pos, sizeof(vec3d));
		memcpy(&last_view_orient, orient, sizeof(matrix));

		use_last_view = false;
	}

	if ( !use_last_view ) {
		// should already be normalized
		eyex =  (GLdouble)pos->xyz.x;
		eyey =  (GLdouble)pos->xyz.y;
		eyez = -(GLdouble)pos->xyz.z;

		// should already be normalized
		GLdouble fwdx =  (GLdouble)orient->vec.fvec.xyz.x;
		GLdouble fwdy =  (GLdouble)orient->vec.fvec.xyz.y;
		GLdouble fwdz = -(GLdouble)orient->vec.fvec.xyz.z;

		// should already be normalized
		GLdouble upx =  (GLdouble)orient->vec.uvec.xyz.x;
		GLdouble upy =  (GLdouble)orient->vec.uvec.xyz.y;
		GLdouble upz = -(GLdouble)orient->vec.uvec.xyz.z;

		GLdouble mag;

		// setup Side vector (crossprod of forward and up vectors)
		GLdouble Sx = (fwdy * upz) - (fwdz * upy);
		GLdouble Sy = (fwdz * upx) - (fwdx * upz);
		GLdouble Sz = (fwdx * upy) - (fwdy * upx);

		// normalize Side
		mag = 1.0 / sqrt( (Sx*Sx) + (Sy*Sy) + (Sz*Sz) );

		Sx *= mag;
		Sy *= mag;
		Sz *= mag;

		// setup Up vector (crossprod of Side and forward vectors)
		GLdouble Ux = (Sy * fwdz) - (Sz * fwdy);
		GLdouble Uy = (Sz * fwdx) - (Sx * fwdz);
		GLdouble Uz = (Sx * fwdy) - (Sy * fwdx);

		// normalize Up
		mag = 1.0 / sqrt( (Ux*Ux) + (Uy*Uy) + (Uz*Uz) );

		Ux *= mag;
		Uy *= mag;
		Uz *= mag;

		// store the result in our matrix
		memset( vmatrix, 0, sizeof(vmatrix) );
		vmatrix[0]  = Sx;   vmatrix[1]  = Ux;   vmatrix[2]  = -fwdx;
		vmatrix[4]  = Sy;   vmatrix[5]  = Uy;   vmatrix[6]  = -fwdy;
		vmatrix[8]  = Sz;   vmatrix[9]  = Uz;   vmatrix[10] = -fwdz;
		vmatrix[15] = 1.0;
	}

	glLoadMatrixd(vmatrix);
	
	glTranslated(-eyex, -eyey, -eyez);
	glScalef(1.0f, 1.0f, -1.0f);


	if (Cmdline_env) {
		GL_env_texture_matrix_set = true;

		// if our view setup is the same as previous call then we can skip this
		if ( !use_last_view ) {
			// setup the texture matrix which will make the the envmap keep lined
			// up properly with the environment
			GLfloat mview[16];

			glGetFloatv(GL_MODELVIEW_MATRIX, mview);

			// r.xyz  <--  r.x, u.x, f.x
			GL_env_texture_matrix[0]  =  mview[0];
			GL_env_texture_matrix[1]  =  mview[4];
			GL_env_texture_matrix[2]  =  mview[8];
			// u.xyz  <--  r.y, u.y, f.y
			GL_env_texture_matrix[4]  =  mview[1];
			GL_env_texture_matrix[5]  =  mview[5];
			GL_env_texture_matrix[6]  =  mview[9];
			// f.xyz  <--  r.z, u.z, f.z
			GL_env_texture_matrix[8]  =  mview[2];
			GL_env_texture_matrix[9]  =  mview[6];
			GL_env_texture_matrix[10] =  mview[10];

			GL_env_texture_matrix[15] = 1.0f;
		}
	}

	GL_CHECK_FOR_ERRORS("end of set_view_matrix()");

	GL_modelview_matrix_depth = 2;
	GL_htl_view_matrix_set = 1;
}

void gr_opengl_end_view_matrix()
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_modelview_matrix_depth == 2);

	glPopMatrix();
	glLoadIdentity();

	GL_modelview_matrix_depth = 1;
	GL_htl_view_matrix_set = 0;
	GL_env_texture_matrix_set = false;
}

// set a view and projection matrix for a 2D element
// TODO: this probably needs to accept values
void gr_opengl_set_2d_matrix(/*int x, int y, int w, int h*/)
{
	if (Cmdline_nohtl) {
		return;
	}

	// don't bother with this if we aren't even going to need it
	if ( !GL_htl_projection_matrix_set ) {
		return;
	}

	Assert( GL_htl_2d_matrix_set == 0 );
	Assert( GL_htl_2d_matrix_depth == 0 );

	glPushAttrib(GL_TRANSFORM_BIT);

	// the viewport needs to be the full screen size since glOrtho() is relative to it
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_texture) {
		glOrtho( 0, gr_screen.max_w, 0, gr_screen.max_h, -1, 1 );
	} else {
		glOrtho( 0, gr_screen.max_w, gr_screen.max_h, 0, -1, 1 );
	}

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

#ifndef NDEBUG
	// safety check to make sure we don't use more than 2 projection matrices
	GLint num_proj_stacks = 0;
	glGetIntegerv( GL_PROJECTION_STACK_DEPTH, &num_proj_stacks );
	Assert( num_proj_stacks <= 2 );
#endif

	GL_htl_2d_matrix_set++;
	GL_htl_2d_matrix_depth++;
}

// ends a previously set 2d view and projection matrix
void gr_opengl_end_2d_matrix()
{
	if (Cmdline_nohtl)
		return;

	if (!GL_htl_2d_matrix_set)
		return;

	Assert( GL_htl_2d_matrix_depth == 1 );

	// reset viewport to what it was originally set to by the proj matrix
	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glPopAttrib();

	GL_htl_2d_matrix_set = 0;
	GL_htl_2d_matrix_depth = 0;
}

static bool GL_scale_matrix_set = false;

void gr_opengl_push_scale_matrix(const vec3d *scale_factor)
{
	if ( (scale_factor->xyz.x == 1) && (scale_factor->xyz.y == 1) && (scale_factor->xyz.z == 1) )
		return;

	GL_scale_matrix_set = true;
	glPushMatrix();

	GL_modelview_matrix_depth++;

	glScalef(scale_factor->xyz.x, scale_factor->xyz.y, scale_factor->xyz.z);
}

void gr_opengl_pop_scale_matrix()
{
	if (!GL_scale_matrix_set) 
		return;

	glPopMatrix();

	GL_modelview_matrix_depth--;
	GL_scale_matrix_set = false;
}

void gr_opengl_end_clip_plane()
{
	if (Cmdline_nohtl) {
		return;
	}

	if ( Use_GLSL > 1 ) {
		return;
	}

	GL_state.ClipPlane(0, GL_FALSE);
}

void gr_opengl_start_clip_plane()
{
	if (Cmdline_nohtl) {
		return;
	}

	if ( Use_GLSL > 1 ) {
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


	glClipPlane(GL_CLIP_PLANE0, clip_equation);
	GL_state.ClipPlane(0, GL_TRUE);
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

void gr_opengl_shadow_map_start(const matrix4 *shadow_view_matrix, const matrix *light_orient)
{
	if(!Cmdline_shadow_quality)
		return;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &saved_fb);
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadow_fbo);

	//glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT};
	vglDrawBuffers(1, buffers);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gr_opengl_set_lighting(false,false);
	
	Rendering_to_shadow_map = true;
	Glowpoint_override_save = Glowpoint_override;
	Glowpoint_override = true;

	GL_htl_projection_matrix_set = 1;
	gr_set_view_matrix(&Eye_position, light_orient);

	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)shadow_view_matrix);

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
		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, saved_fb);
		if(saved_fb)
		{
// 			GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
// 			vglDrawBuffers(2, buffers);
		}

		Glowpoint_override = Glowpoint_override_save;
		GL_htl_projection_matrix_set = 0;
		
		glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
		glScissor(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
}

void opengl_tnl_set_material(int flags, uint shader_flags, int tmap_type)
{
	float u_scale, v_scale;
	int render_pass = 0;

	if ( flags & TMAP_ANIMATED_SHADER ) {
		GL_state.Uniform.setUniformf("anim_timer", opengl_shader_get_animated_timer());
		GL_state.Uniform.setUniformi("effect_num", opengl_shader_get_animated_effect());
		GL_state.Uniform.setUniformf("vpwidth", 1.0f/gr_screen.max_w);
		GL_state.Uniform.setUniformf("vpheight", 1.0f/gr_screen.max_h);
	}

	int num_lights = MIN(Num_active_gl_lights, GL_max_lights) - 1;
	GL_state.Uniform.setUniformi("n_lights", num_lights);
	GL_state.Uniform.setUniformf( "light_factor", GL_light_factor );
	
	if ( shader_flags & SDR_FLAG_MODEL_CLIP ) {
		GL_state.Uniform.setUniformi("use_clip_plane", G3_user_clip);

		if ( G3_user_clip ) {
			vec3d normal, pos;

			vm_vec_unrotate(&normal, &G3_user_clip_normal, &Eye_matrix);
			vm_vec_normalize(&normal);

			vm_vec_unrotate(&pos, &G3_user_clip_point, &Eye_matrix);
			vm_vec_add2(&pos, &Eye_position);

			matrix4 model_matrix;
			memset( &model_matrix, 0, sizeof(model_matrix) );

			model_matrix.a1d[0]  = Object_matrix.vec.rvec.xyz.x;   model_matrix.a1d[4]  = Object_matrix.vec.uvec.xyz.x;   model_matrix.a1d[8]  = Object_matrix.vec.fvec.xyz.x;
			model_matrix.a1d[1]  = Object_matrix.vec.rvec.xyz.y;   model_matrix.a1d[5]  = Object_matrix.vec.uvec.xyz.y;   model_matrix.a1d[9]  = Object_matrix.vec.fvec.xyz.y;
			model_matrix.a1d[2]  = Object_matrix.vec.rvec.xyz.z;   model_matrix.a1d[6]  = Object_matrix.vec.uvec.xyz.z;   model_matrix.a1d[10] = Object_matrix.vec.fvec.xyz.z;
			model_matrix.a1d[12] = Object_position.xyz.x;
			model_matrix.a1d[13] = Object_position.xyz.y;
			model_matrix.a1d[14] = Object_position.xyz.z;
			model_matrix.a1d[15] = 1.0f;

			GL_state.Uniform.setUniform3f("clip_normal", normal);
			GL_state.Uniform.setUniform3f("clip_position", pos);
			GL_state.Uniform.setUniformMatrix4f("world_matrix", model_matrix);
		}
	}

	if ( shader_flags & SDR_FLAG_MODEL_DIFFUSE_MAP ) {
		GL_state.Uniform.setUniformi("sBasemap", render_pass);
		
		if ( flags & TMAP_FLAG_DESATURATE ) {
			GL_state.Uniform.setUniformi("desaturate", 1);
			GL_state.Uniform.setUniform3f("desaturate_clr", gr_screen.current_color.red/255.0f, gr_screen.current_color.green/255.0f, gr_screen.current_color.blue/255.0f);
		} else {
			GL_state.Uniform.setUniformi("desaturate", 0);
		}

		if ( flags & TMAP_FLAG_ALPHA ) {
			if ( bm_has_alpha_channel(gr_screen.current_bitmap) ) {
				GL_state.SetAlphaBlendMode(ALPHA_BLEND_PREMULTIPLIED);
				GL_state.Uniform.setUniformi("blend_alpha", 1);
			} else {
				GL_state.Uniform.setUniformi("blend_alpha", 2);
			}
		} else {
			GL_state.Uniform.setUniformi("blend_alpha", 0);
		}

		gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( shader_flags & SDR_FLAG_MODEL_GLOW_MAP ) {
		GL_state.Uniform.setUniformi("sGlowmap", render_pass);

		gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( shader_flags & SDR_FLAG_MODEL_SPEC_MAP ) {
		GL_state.Uniform.setUniformi("sSpecmap", render_pass);

		gr_opengl_tcache_set(SPECMAP, tmap_type, &u_scale, &v_scale, render_pass);

		++render_pass;

		if ( shader_flags & SDR_FLAG_MODEL_ENV_MAP) {
			// 0 == env with non-alpha specmap, 1 == env with alpha specmap
			int alpha_spec = bm_has_alpha_channel(SPECMAP) ? 1 : 0;

			matrix4 texture_mat, envMatrix;

			for ( int i = 0; i < 16; ++i ) {
				texture_mat.a1d[i] = GL_env_texture_matrix[i];
			}

			if (!vm_inverse_matrix4(&texture_mat, &envMatrix)) {
				Error(LOCATION, "Unable to invert environment mapping matrix.\n");
			}

			GL_state.Uniform.setUniformi("alpha_spec", alpha_spec);
			GL_state.Uniform.setUniformMatrix4fv("envMatrix", 1, &envMatrix);
			GL_state.Uniform.setUniformi("sEnvmap", render_pass);

			gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, render_pass);

			++render_pass;
		}
	}

	if ( shader_flags & SDR_FLAG_MODEL_NORMAL_MAP ) {
		GL_state.Uniform.setUniformi("sNormalmap", render_pass);

		gr_opengl_tcache_set(NORMMAP, tmap_type, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( shader_flags & SDR_FLAG_MODEL_HEIGHT_MAP ) {
		GL_state.Uniform.setUniformi("sHeightmap", render_pass);
		
		gr_opengl_tcache_set(HEIGHTMAP, tmap_type, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( shader_flags & SDR_FLAG_MODEL_MISC_MAP ) {
		GL_state.Uniform.setUniformi("sMiscmap", render_pass);

		gr_opengl_tcache_set(MISCMAP, tmap_type, &u_scale, &v_scale, render_pass);

		++render_pass;
	}

	if ( shader_flags & SDR_FLAG_MODEL_SHADOWS ) {
		matrix4 model_matrix;
		memset( &model_matrix, 0, sizeof(model_matrix) );

		model_matrix.a1d[0]  = Object_matrix.vec.rvec.xyz.x;   model_matrix.a1d[4]  = Object_matrix.vec.uvec.xyz.x;   model_matrix.a1d[8]  = Object_matrix.vec.fvec.xyz.x;
		model_matrix.a1d[1]  = Object_matrix.vec.rvec.xyz.y;   model_matrix.a1d[5]  = Object_matrix.vec.uvec.xyz.y;   model_matrix.a1d[9]  = Object_matrix.vec.fvec.xyz.y;
		model_matrix.a1d[2]  = Object_matrix.vec.rvec.xyz.z;   model_matrix.a1d[6]  = Object_matrix.vec.uvec.xyz.z;   model_matrix.a1d[10] = Object_matrix.vec.fvec.xyz.z;
		model_matrix.a1d[12] = Object_position.xyz.x;
		model_matrix.a1d[13] = Object_position.xyz.y;
		model_matrix.a1d[14] = Object_position.xyz.z;
		model_matrix.a1d[15] = 1.0f;

		GL_state.Uniform.setUniformMatrix4f("shadow_mv_matrix", Shadow_view_matrix);
		GL_state.Uniform.setUniformMatrix4fv("shadow_proj_matrix", MAX_SHADOW_CASCADES, Shadow_proj_matrix);
		GL_state.Uniform.setUniformMatrix4f("model_matrix", model_matrix);
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

	if ( shader_flags & SDR_FLAG_MODEL_SHADOW_MAP ) {
		GL_state.Uniform.setUniformMatrix4fv("shadow_proj_matrix", MAX_SHADOW_CASCADES, Shadow_proj_matrix);
	}

	if ( shader_flags & SDR_FLAG_MODEL_ANIMATED ) {
		GL_state.Uniform.setUniformi("sFramebuffer", render_pass);
		
		GL_state.Texture.SetActiveUnit(render_pass);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);

		if ( Scene_framebuffer_in_frame ) {
			GL_state.Texture.Enable(Scene_effect_texture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		} else {
			GL_state.Texture.Enable(Framebuffer_fallback_texture_id);
		}

		++render_pass;
	}

	if ( shader_flags & SDR_FLAG_MODEL_TRANSFORM ) {
		GL_state.Uniform.setUniformi("transform_tex", render_pass);
		GL_state.Uniform.setUniformi("buffer_matrix_offset", GL_transform_buffer_offset);
		
		GL_state.Texture.SetActiveUnit(render_pass);
		GL_state.Texture.SetTarget(GL_TEXTURE_BUFFER_ARB);
		GL_state.Texture.Enable(opengl_get_transform_buffer_texture());

		++render_pass;
	}

	// Team colors are passed to the shader here, but the shader needs to handle their application.
	// By default, this is handled through the r and g channels of the misc map, but this can be changed
	// in the shader; test versions of this used the normal map r and b channels
	if ( shader_flags & SDR_FLAG_MODEL_TEAMCOLOR ) {
		vec3d stripe_color;
		vec3d base_color;

		stripe_color.xyz.x = Current_team_color->stripe.r;
		stripe_color.xyz.y = Current_team_color->stripe.g;
		stripe_color.xyz.z = Current_team_color->stripe.b;

		base_color.xyz.x = Current_team_color->base.r;
		base_color.xyz.y = Current_team_color->base.g;
		base_color.xyz.z = Current_team_color->base.b;

		GL_state.Uniform.setUniform3f("stripe_color", stripe_color);
		GL_state.Uniform.setUniform3f("base_color", base_color);
	}

	if ( shader_flags & SDR_FLAG_MODEL_THRUSTER ) {
		GL_state.Uniform.setUniformf("thruster_scale", GL_thrust_scale);
	}
}

void opengl_tnl_set_material_soft_particle(uint flags)
{
	uint sdr_effect_flags = 0;

	if ( flags & TMAP_FLAG_VERTEX_GEN ) {
		sdr_effect_flags |= SDR_FLAG_PARTICLE_POINT_GEN;
	}

	int sdr_index = gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, sdr_effect_flags);

	opengl_shader_set_current(sdr_index);

	GL_state.Uniform.setUniformi("baseMap", 0);
	GL_state.Uniform.setUniformi("depthMap", 1);
	GL_state.Uniform.setUniformf("window_width", (float)gr_screen.max_w);
	GL_state.Uniform.setUniformf("window_height", (float)gr_screen.max_h);
	GL_state.Uniform.setUniformf("nearZ", Min_draw_distance);
	GL_state.Uniform.setUniformf("farZ", Max_draw_distance);

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

void opengl_tnl_set_material_distortion(uint flags)
{
	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_EFFECT_DISTORTION, 0) );

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

	if(flags & TMAP_FLAG_DISTORTION_THRUSTER) {
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

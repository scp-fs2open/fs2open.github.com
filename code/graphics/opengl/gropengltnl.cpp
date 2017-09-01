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
#include "gropengldraw.h"
#include "gropengllight.h"
#include "gropenglshader.h"
#include "gropenglstate.h"
#include "gropengltexture.h"
#include "gropengltnl.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "weapon/trails.h"
#include "particle/particle.h"
#include "graphics/shadows.h"
#include "graphics/material.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

size_t GL_vertex_data_in = 0;

GLint GL_max_elements_vertices = 4096;
GLint GL_max_elements_indices = 4096;

size_t GL_transform_buffer_offset = INVALID_SIZE;

GLuint Shadow_map_texture = 0;
GLuint Shadow_map_depth_texture = 0;
GLuint shadow_fbo = 0;
bool Rendering_to_shadow_map = false;

int Transform_buffer_handle = -1;

transform_stack GL_model_matrix_stack;
glm::mat4 GL_view_matrix;
glm::mat4 GL_model_view_matrix;
glm::mat4 GL_projection_matrix;
glm::mat4 GL_last_projection_matrix;
glm::mat4 GL_last_view_matrix;

struct opengl_buffer_object {
	GLuint buffer_id;
	GLenum type;
	GLenum usage;
	size_t size;

	GLuint texture;	// for texture buffer objects
};

static SCP_vector<opengl_buffer_object> GL_buffer_objects;
static int GL_vertex_buffers_in_use = 0;

int opengl_create_buffer_object(GLenum type, GLenum usage)
{
	GR_DEBUG_SCOPE("Create buffer object");

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
	GR_DEBUG_SCOPE("Bind buffer handle");

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
	GR_DEBUG_SCOPE("Update buffer data");

	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	opengl_bind_buffer_object(handle);

	GL_vertex_data_in -= buffer_obj.size;
	buffer_obj.size = size;
	GL_vertex_data_in += buffer_obj.size;

	glBufferData(buffer_obj.type, size, data, buffer_obj.usage);
}

void gr_opengl_update_buffer_data_offset(int handle, size_t offset, size_t size, void* data)
{
	GR_DEBUG_SCOPE("Update buffer data with offset");

	Assert(handle >= 0);
	Assert((size_t)handle < GL_buffer_objects.size());

	opengl_buffer_object &buffer_obj = GL_buffer_objects[handle];

	opengl_bind_buffer_object(handle);
	
	glBufferSubData(buffer_obj.type, offset, size, data);
}

void gr_opengl_delete_buffer(int handle)
{
	GR_DEBUG_SCOPE("Deleting buffer");

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
	return opengl_create_buffer_object(GL_ARRAY_BUFFER, static_buffer ? GL_STATIC_DRAW : GL_STREAM_DRAW);
}

int gr_opengl_create_index_buffer(bool static_buffer)
{
	return opengl_create_buffer_object(GL_ELEMENT_ARRAY_BUFFER, static_buffer ? GL_STATIC_DRAW : GL_STREAM_DRAW);
}

int opengl_create_texture_buffer_object()
{
	// create the buffer
	int buffer_object_handle = opengl_create_buffer_object(GL_TEXTURE_BUFFER, GL_DYNAMIC_DRAW);

	opengl_check_for_errors();

	opengl_buffer_object &buffer_obj = GL_buffer_objects[buffer_object_handle];

	// create the texture
	glGenTextures(1, &buffer_obj.texture);
	glBindTexture(GL_TEXTURE_BUFFER, buffer_obj.texture);

	gr_opengl_update_buffer_data(buffer_object_handle, 100, NULL);

	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_obj.buffer_id);

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
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer_obj.buffer_id);
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

void opengl_destroy_all_buffers()
{
	for ( uint i = 0; i < GL_buffer_objects.size(); i++ ) {
		gr_opengl_delete_buffer(i);
	}

	GL_vertex_buffers_in_use = 0;
}

void opengl_tnl_init()
{
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
		GL_state.BindFrameBuffer(shadow_fbo);

		glGenTextures(1, &Shadow_map_depth_texture);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
		GL_state.Texture.Enable(Shadow_map_depth_texture);
		
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
		//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);
		int size = (Cmdline_shadow_quality == 2 ? 1024 : 512);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, size, size, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Shadow_map_depth_texture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Shadow_map_depth_texture, 0);

		glGenTextures(1, &Shadow_map_texture);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D_ARRAY);
		GL_state.Texture.Enable(Shadow_map_texture);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, size, size, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size, size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Shadow_map_texture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Shadow_map_texture, 0);

		GL_state.BindFrameBuffer(0);

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

static void opengl_init_arrays(indexed_vertex_source *vert_src, vertex_buffer *bufferp)
{
	Assertion(vert_src->Vbuffer_handle >= 0, "Vertex buffers require a valid buffer handle!");

	opengl_bind_buffer_object(vert_src->Vbuffer_handle);
	
	opengl_bind_vertex_layout(bufferp->layout);
}

void opengl_render_model_program(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, buffer_data *datap)
{
	GL_state.Texture.SetShaderMode(GL_TRUE);

	opengl_tnl_set_model_material(material_info);

	GLubyte *ibuffer = NULL;

	size_t start = 0;
	size_t end = (datap->n_verts - 1);
	size_t count = (end - (start * 3) + 1);

	GLenum element_type = (datap->flags & VB_FLAG_LARGE_INDEX) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

	Assert(vert_source);

	// basic setup of all data
	opengl_init_arrays(vert_source, bufferp);

	Assertion(vert_source->Ibuffer_handle >= 0, "The index values must be located in a GPU buffer!");

	opengl_bind_buffer_object(vert_source->Ibuffer_handle);

	if ( Rendering_to_shadow_map ) {
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, (GLsizei) count, element_type,
										  ibuffer + (datap->index_offset + start), 4, (GLint)bufferp->vertex_num_offset);
	} else {
		if ( Cmdline_drawelements ) {
			glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei) count,
									 element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_num_offset);
		} else {
			glDrawRangeElementsBaseVertex(GL_TRIANGLES, datap->i_first, datap->i_last, (GLsizei) count,
										  element_type, ibuffer + (datap->index_offset + start), (GLint)bufferp->vertex_num_offset);
		}
	}

	GL_state.Texture.SetShaderMode(GL_FALSE);
}

void gr_opengl_render_model(model_material* material_info, indexed_vertex_source *vert_source, vertex_buffer* bufferp, size_t texi)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	Verify(bufferp != NULL);

	GL_CHECK_FOR_ERRORS("start of render_buffer()");

	buffer_data *datap = &bufferp->tex_buf[texi];

	opengl_render_model_program(material_info, vert_source, bufferp, datap);

	GL_CHECK_FOR_ERRORS("end of render_buffer()");
}

extern bool Scene_framebuffer_in_frame;
extern GLuint Framebuffer_fallback_texture_id;
extern GLuint Scene_depth_texture;
extern GLuint Scene_position_texture;
extern GLuint Distortion_texture[2];
extern int Distortion_switch;

glm::mat4 opengl_create_view_matrix(const vec3d *pos, const matrix *orient)
{
	vec3d scaled_pos;
	vec3d inv_pos;
	matrix scaled_orient = *orient;
	matrix inv_orient;

	vm_vec_copy_scale(&scaled_pos, pos, -1.0f);
	vm_vec_scale(&scaled_orient.vec.fvec, -1.0f);

	vm_copy_transpose(&inv_orient, &scaled_orient);
	vm_vec_rotate(&inv_pos, &scaled_pos, &scaled_orient);

	glm::mat4 out = vm_mat_to_glm(inv_orient);
	out[3] = glm::vec4(vm_vec_to_glm(inv_pos), 1.0f);

	return out;
}

void gr_opengl_start_instance_matrix(const vec3d *offset, const matrix *rotation)
{
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

	auto model_matrix = GL_model_matrix_stack.get_transform();
	GL_model_view_matrix = GL_view_matrix * model_matrix;

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

void gr_opengl_end_instance_matrix()
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	GL_model_matrix_stack.pop();

	auto model_matrix = GL_model_matrix_stack.get_transform();
	GL_model_view_matrix = GL_view_matrix * model_matrix;

	GL_modelview_matrix_depth--;
}

// the projection matrix; fov, aspect ratio, near, far
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
	GL_CHECK_FOR_ERRORS("start of set_projection_matrix()()");
	
	if (GL_rendering_to_texture) {
		glViewport(gr_screen.offset_x, gr_screen.offset_y, gr_screen.clip_width, gr_screen.clip_height);
	} else {
		glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
	}
	
	GL_last_projection_matrix = GL_projection_matrix;
	GL_projection_matrix = glm::perspective(fov, aspect, z_near, z_far);
	
	GL_CHECK_FOR_ERRORS("end of set_projection_matrix()()");

	GL_htl_projection_matrix_set = 1;
}

void gr_opengl_end_projection_matrix()
{
	GL_CHECK_FOR_ERRORS("start of end_projection_matrix()");

	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	GL_last_projection_matrix = GL_projection_matrix;

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (GL_rendering_to_texture) {
		GL_projection_matrix = glm::ortho(0.f, i2fl(gr_screen.max_w), 0.0f, i2fl(gr_screen.max_h), -1.0f, 1.0f);
	} else {
		GL_projection_matrix = glm::ortho(0.f, i2fl(gr_screen.max_w), i2fl(gr_screen.max_h), 0.f, -1.0f, 1.0f);
	}

	GL_CHECK_FOR_ERRORS("end of end_projection_matrix()");

	GL_htl_projection_matrix_set = 0;
}

void gr_opengl_set_view_matrix(const vec3d *pos, const matrix *orient)
{
	Assert(GL_htl_projection_matrix_set);
	Assert(GL_modelview_matrix_depth == 1);

	GL_CHECK_FOR_ERRORS("start of set_view_matrix()");

	GL_view_matrix = opengl_create_view_matrix(pos, orient);
	
	GL_model_matrix_stack.clear();
	GL_model_view_matrix = GL_view_matrix;

	if (Cmdline_env) {
		GL_env_texture_matrix_set = true;

		// setup the texture matrix which will make the the envmap keep lined
		// up properly with the environment

		// r.xyz  <--  r.x, u.x, f.x
		GL_env_texture_matrix[0] = GL_model_view_matrix[0][0];
		GL_env_texture_matrix[1] = GL_model_view_matrix[1][0];
		GL_env_texture_matrix[2] = GL_model_view_matrix[2][0];
		// u.xyz  <--  r.y, u.y, f.y
		GL_env_texture_matrix[4] = GL_model_view_matrix[0][1];
		GL_env_texture_matrix[5] = GL_model_view_matrix[1][2];
		GL_env_texture_matrix[6] = GL_model_view_matrix[2][3];
		// f.xyz  <--  r.z, u.z, f.z
		GL_env_texture_matrix[8] = GL_model_view_matrix[0][2];
		GL_env_texture_matrix[9] = GL_model_view_matrix[1][2];
		GL_env_texture_matrix[10] = GL_model_view_matrix[2][2];

		GL_env_texture_matrix[15] = 1.0f;
	}

	GL_CHECK_FOR_ERRORS("end of set_view_matrix()");

	GL_modelview_matrix_depth = 2;
	GL_htl_view_matrix_set = 1;
}

void gr_opengl_end_view_matrix()
{
	Assert(GL_modelview_matrix_depth == 2);

	GL_model_matrix_stack.clear();
	GL_view_matrix = glm::mat4();
	GL_model_view_matrix = glm::mat4();

	GL_modelview_matrix_depth = 1;
	GL_htl_view_matrix_set = 0;
	GL_env_texture_matrix_set = false;
}

// set a view and projection matrix for a 2D element
// TODO: this probably needs to accept values
void gr_opengl_set_2d_matrix(/*int x, int y, int w, int h*/)
{
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
		GL_projection_matrix = glm::ortho(0.f, i2fl(gr_screen.max_w), 0.f, i2fl(gr_screen.max_h), -1.f, 1.f);
	} else {
		GL_projection_matrix = glm::ortho(0.f, i2fl(gr_screen.max_w), i2fl(gr_screen.max_h), 0.f, -1.f, 1.f);
	}

	GL_model_matrix_stack.push_and_replace(glm::mat4());

	GL_last_view_matrix = GL_view_matrix;
	GL_view_matrix = glm::mat4();
	GL_model_view_matrix = glm::mat4();

	GL_htl_2d_matrix_set++;
	GL_htl_2d_matrix_depth++;
}

// ends a previously set 2d view and projection matrix
void gr_opengl_end_2d_matrix()
{
	if (!GL_htl_2d_matrix_set)
		return;

	Assert( GL_htl_2d_matrix_depth == 1 );

	// reset viewport to what it was originally set to by the proj matrix
	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);

	GL_projection_matrix = GL_last_projection_matrix;
		
	GL_model_matrix_stack.pop();

	GL_view_matrix = GL_last_view_matrix;

	auto model_matrix = GL_model_matrix_stack.get_transform();
	GL_model_view_matrix = GL_view_matrix * model_matrix;

	GL_htl_2d_matrix_set = 0;
	GL_htl_2d_matrix_depth = 0;
}

static bool GL_scale_matrix_set = false;

void gr_opengl_push_scale_matrix(const vec3d *scale_factor)
{
	if ( (scale_factor->xyz.x == 1) && (scale_factor->xyz.y == 1) && (scale_factor->xyz.z == 1) )
		return;

	GL_scale_matrix_set = true;

	GL_modelview_matrix_depth++;

	GL_model_matrix_stack.push(NULL, NULL, scale_factor);

	auto model_matrix = GL_model_matrix_stack.get_transform();
	GL_model_view_matrix = GL_view_matrix * model_matrix;
}

void gr_opengl_pop_scale_matrix()
{
	if (!GL_scale_matrix_set) 
		return;

	GL_model_matrix_stack.pop();

	auto model_matrix = GL_model_matrix_stack.get_transform();
	GL_model_view_matrix = GL_view_matrix * model_matrix;

	GL_modelview_matrix_depth--;
	GL_scale_matrix_set = false;
}

void gr_opengl_end_clip_plane()
{
	// The shaders handle this now
}

void gr_opengl_start_clip_plane()
{
	// The shaders handle this now
}

void gr_opengl_set_clip_plane(vec3d *clip_normal, vec3d *clip_point)
{
	if ( clip_normal == NULL || clip_point == NULL ) {
		GL_state.ClipDistance(0, false);
	} else {
		Assertion(Current_shader != NULL &&
				  (Current_shader->shader == SDR_TYPE_MODEL || Current_shader->shader == SDR_TYPE_PASSTHROUGH_RENDER),
				  "Clip planes are not supported by this shader!");

		GL_state.ClipDistance(0, true);
	}
}

extern bool Glowpoint_override;
bool Glowpoint_override_save;

void gr_opengl_shadow_map_start(glm::mat4& shadow_view_matrix, const matrix *light_orient)
{
	if ( !Cmdline_shadow_quality )
		return;

	GL_state.PushFramebufferState();
	GL_state.BindFrameBuffer(shadow_fbo);

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

	shadow_view_matrix = GL_view_matrix;

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
	GL_state.PopFramebufferState();

	Glowpoint_override = Glowpoint_override_save;
	GL_htl_projection_matrix_set = 0;

	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
	glScissor(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
}

void opengl_tnl_set_material(material* material_info, bool set_base_map)
{
	int shader_handle = material_info->get_shader_handle();
	int base_map = material_info->get_texture_map(TM_BASE_TYPE);
	auto clr = material_info->get_color();

	Assert(shader_handle >= 0);

	opengl_shader_set_current(shader_handle);

	GL_state.SetAlphaBlendMode(material_info->get_blend_mode());
	GL_state.SetZbufferType(material_info->get_depth_mode());

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

	if ( material_info->is_clipped() ) {
		gr_opengl_set_clip_plane(&clip_params.normal, &clip_params.position);
	} else {
		gr_opengl_set_clip_plane(NULL, NULL);
	}

	// This is only needed for the passthrough shader
	uint32_t array_index = 0;
	if ( set_base_map && base_map >= 0 ) {
		float u_scale, v_scale;

		if ( !gr_opengl_tcache_set(base_map, material_info->get_texture_type(), &u_scale, &v_scale, &array_index) ) {
			mprintf(("WARNING: Error setting bitmap texture (%i)!\n", base_map));
		}
	}

	if ( Current_shader->shader == SDR_TYPE_DEFAULT_MATERIAL ) {
		opengl_shader_set_default_material(base_map >= 0,
										   material_info->get_texture_type() == TCACHE_TYPE_AABITMAP,
										   &clr,
										   material_info->get_color_scale(),
										   array_index,
										   clip_params);
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
	
	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("modelMatrix", GL_model_matrix_stack.get_transform());
	Current_shader->program->Uniforms.setUniformMatrix4f("viewMatrix", GL_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", GL_projection_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("textureMatrix", GL_texture_matrix);

	auto clr = material_info->get_color();
	Current_shader->program->Uniforms.setUniform4f("color", clr);

	if ( Current_shader->flags & SDR_FLAG_MODEL_ANIMATED ) {
		Current_shader->program->Uniforms.setUniformf("anim_timer", material_info->get_animated_effect_time());
		Current_shader->program->Uniforms.setUniformi("effect_num", material_info->get_animated_effect());
		Current_shader->program->Uniforms.setUniformf("vpwidth", 1.0f / gr_screen.max_w);
		Current_shader->program->Uniforms.setUniformf("vpheight", 1.0f / gr_screen.max_h);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_CLIP ) {
		if (material_info->is_clipped()) {
			material::clip_plane &clip_info = material_info->get_clip_plane();
			
			Current_shader->program->Uniforms.setUniformi("use_clip_plane", 1);

			glm::vec4 clip_equation;
			clip_equation.x = clip_info.normal.xyz.x;
			clip_equation.y = clip_info.normal.xyz.y;
			clip_equation.z = clip_info.normal.xyz.z;
			clip_equation.w = -vm_vec_dot(&clip_info.normal, &clip_info.position);

			Current_shader->program->Uniforms.setUniform4f("clip_equation", clip_equation);
		} else {
			Current_shader->program->Uniforms.setUniformi("use_clip_plane", 0);
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_LIGHT ) {
		int num_lights = MIN(Num_active_gl_lights, GL_max_lights) - 1;
		float light_factor = material_info->get_light_factor();
		Current_shader->program->Uniforms.setUniformi("n_lights", num_lights);
		Current_shader->program->Uniforms.setUniform4fv("lightPosition", GL_max_lights, opengl_light_uniforms.Position);
		Current_shader->program->Uniforms.setUniform3fv("lightDirection", GL_max_lights, opengl_light_uniforms.Direction);
		Current_shader->program->Uniforms.setUniform3fv("lightDiffuseColor", GL_max_lights, opengl_light_uniforms.Diffuse_color);
		Current_shader->program->Uniforms.setUniform3fv("lightSpecColor", GL_max_lights, opengl_light_uniforms.Spec_color);
		Current_shader->program->Uniforms.setUniform1iv("lightType", GL_max_lights, opengl_light_uniforms.Light_type);
		Current_shader->program->Uniforms.setUniform1fv("lightAttenuation", GL_max_lights, opengl_light_uniforms.Attenuation);

		if ( !material_info->get_center_alpha() ) {
			Current_shader->program->Uniforms.setUniform3f("diffuseFactor", GL_light_color[0] * light_factor, GL_light_color[1] * light_factor, GL_light_color[2] * light_factor);
			Current_shader->program->Uniforms.setUniform3f("ambientFactor", GL_light_ambient[0], GL_light_ambient[1], GL_light_ambient[2]);
		} else {
			//Current_shader->program->Uniforms.setUniform3f("diffuseFactor", GL_light_true_zero[0], GL_light_true_zero[1], GL_light_true_zero[2]);
			//Current_shader->program->Uniforms.setUniform3f("ambientFactor", GL_light_true_zero[0], GL_light_true_zero[1], GL_light_true_zero[2]);
			Current_shader->program->Uniforms.setUniform3f("diffuseFactor", GL_light_color[0] * light_factor, GL_light_color[1] * light_factor, GL_light_color[2] * light_factor);
			Current_shader->program->Uniforms.setUniform3f("ambientFactor", GL_light_ambient[0], GL_light_ambient[1], GL_light_ambient[2]);
		}

		if ( material_info->get_light_factor() > 0.25f && !Cmdline_no_emissive ) {
			Current_shader->program->Uniforms.setUniform3f("emissionFactor", GL_light_emission[0], GL_light_emission[1], GL_light_emission[2]);
		} else {
			Current_shader->program->Uniforms.setUniform3f("emissionFactor", GL_light_zero[0], GL_light_zero[1], GL_light_zero[2]);
		}

		Current_shader->program->Uniforms.setUniformf("specPower", Cmdline_ogl_spec);

		if ( Gloss_override_set ) {
			Current_shader->program->Uniforms.setUniformf("defaultGloss", Gloss_override);
		} else {
			Current_shader->program->Uniforms.setUniformf("defaultGloss", 0.6f); // add user configurable default gloss in the command line later
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_DIFFUSE_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sBasemap", render_pass);

		if ( material_info->is_desaturated() ) {
			Current_shader->program->Uniforms.setUniformi("desaturate", 1);
		} else {
			Current_shader->program->Uniforms.setUniformi("desaturate", 0);
		}

		if ( Basemap_color_override_set ) {
			Current_shader->program->Uniforms.setUniformi("overrideDiffuse", 1);
			Current_shader->program->Uniforms.setUniform3f("diffuseClr", Basemap_color_override[0], Basemap_color_override[1], Basemap_color_override[2]);
		} else {
			Current_shader->program->Uniforms.setUniformi("overrideDiffuse", 0);
		}

		switch ( material_info->get_blend_mode() ) {
		case ALPHA_BLEND_PREMULTIPLIED:
			Current_shader->program->Uniforms.setUniformi("blend_alpha", 1);
			break;
		case ALPHA_BLEND_ADDITIVE:
			Current_shader->program->Uniforms.setUniformi("blend_alpha", 2);
			break;
		default:
			Current_shader->program->Uniforms.setUniformi("blend_alpha", 0);
			break;
		}

		uint32_t array_index = 0;
		gr_opengl_tcache_set(material_info->get_texture_map(TM_BASE_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		Current_shader->program->Uniforms.setUniformi("sBasemapIndex", array_index);
		
		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_GLOW_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sGlowmap", render_pass);

		if ( Glowmap_color_override_set ) {
			Current_shader->program->Uniforms.setUniformi("overrideGlow", 1);
			Current_shader->program->Uniforms.setUniform3f("glowClr", Glowmap_color_override[0], Glowmap_color_override[1], Glowmap_color_override[2]);
		} else {
			Current_shader->program->Uniforms.setUniformi("overrideGlow", 0);
		}

		uint32_t array_index = 0;
		gr_opengl_tcache_set(material_info->get_texture_map(TM_GLOW_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		Current_shader->program->Uniforms.setUniformi("sGlowmapIndex", array_index);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SPEC_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sSpecmap", render_pass);

		if ( Specmap_color_override_set ) {
			Current_shader->program->Uniforms.setUniformi("overrideSpec", 1);
			Current_shader->program->Uniforms.setUniform3f("specClr", Specmap_color_override[0], Specmap_color_override[1], Specmap_color_override[2]);
		} else {
			Current_shader->program->Uniforms.setUniformi("overrideSpec", 0);
		}

		uint32_t array_index = 0;
		if ( material_info->get_texture_map(TM_SPEC_GLOSS_TYPE) > 0 ) {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPEC_GLOSS_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);

			Current_shader->program->Uniforms.setUniformi("gammaSpec", 1);

			if ( Gloss_override_set ) {
				Current_shader->program->Uniforms.setUniformi("alphaGloss", 0);
			} else {
				Current_shader->program->Uniforms.setUniformi("alphaGloss", 1);
			}
		} else {
			gr_opengl_tcache_set(material_info->get_texture_map(TM_SPECULAR_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);

			Current_shader->program->Uniforms.setUniformi("gammaSpec", 0);
			Current_shader->program->Uniforms.setUniformi("alphaGloss", 0);
		}
		Current_shader->program->Uniforms.setUniformi("sSpecmapIndex", array_index);
		
		++render_pass;

		if ( Current_shader->flags & SDR_FLAG_MODEL_ENV_MAP ) {
			if ( material_info->get_texture_map(TM_SPEC_GLOSS_TYPE) > 0 || Gloss_override_set ) {
				Current_shader->program->Uniforms.setUniformi("envGloss", 1);
			} else {
				Current_shader->program->Uniforms.setUniformi("envGloss", 0);
			}

			Current_shader->program->Uniforms.setUniformMatrix4f("envMatrix", glm::make_mat4(GL_env_texture_matrix));
			Current_shader->program->Uniforms.setUniformi("sEnvmap", render_pass);

			gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, &array_index, render_pass);
			Assertion(array_index == 0, "Cube map arrays are not supported yet!");

			++render_pass;
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sNormalmap", render_pass);

		uint32_t array_index = 0;
		gr_opengl_tcache_set(material_info->get_texture_map(TM_NORMAL_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		Current_shader->program->Uniforms.setUniformi("sNormalmapIndex", array_index);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_HEIGHT_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sHeightmap", render_pass);

		uint32_t array_index = 0;
		gr_opengl_tcache_set(material_info->get_texture_map(TM_HEIGHT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		Current_shader->program->Uniforms.setUniformi("sHeightmapIndex", array_index);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_AMBIENT_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sAmbientmap", render_pass);

		uint32_t array_index = 0;
		gr_opengl_tcache_set(material_info->get_texture_map(TM_AMBIENT_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		Current_shader->program->Uniforms.setUniformi("sAmbientmapIndex", array_index);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_MISC_MAP ) {
		Current_shader->program->Uniforms.setUniformi("sMiscmap", render_pass);

		uint32_t array_index = 0;
		gr_opengl_tcache_set(material_info->get_texture_map(TM_MISC_TYPE), TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index, render_pass);
		Current_shader->program->Uniforms.setUniformi("sMiscmapIndex", array_index);

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SHADOWS ) {
		Current_shader->program->Uniforms.setUniformMatrix4f("shadow_mv_matrix", Shadow_view_matrix);
		Current_shader->program->Uniforms.setUniformMatrix4fv("shadow_proj_matrix", MAX_SHADOW_CASCADES, Shadow_proj_matrix);
		Current_shader->program->Uniforms.setUniformf("veryneardist", Shadow_cascade_distances[0]);
		Current_shader->program->Uniforms.setUniformf("neardist", Shadow_cascade_distances[1]);
		Current_shader->program->Uniforms.setUniformf("middist", Shadow_cascade_distances[2]);
		Current_shader->program->Uniforms.setUniformf("fardist", Shadow_cascade_distances[3]);
		Current_shader->program->Uniforms.setUniformi("shadow_map", render_pass);

		GL_state.Texture.Enable(render_pass, GL_TEXTURE_2D_ARRAY, Shadow_map_texture);

		++render_pass; // bump!
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_SHADOW_MAP ) {
		Current_shader->program->Uniforms.setUniformMatrix4fv("shadow_proj_matrix", MAX_SHADOW_CASCADES, Shadow_proj_matrix);
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_ANIMATED ) {
		Current_shader->program->Uniforms.setUniformi("sFramebuffer", render_pass);

		if ( Scene_framebuffer_in_frame ) {
			GL_state.Texture.Enable(render_pass, GL_TEXTURE_2D, Scene_effect_texture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		} else {
			GL_state.Texture.Enable(render_pass, GL_TEXTURE_2D, Framebuffer_fallback_texture_id);
		}

		++render_pass;
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_TRANSFORM ) {
		Current_shader->program->Uniforms.setUniformi("transform_tex", render_pass);
		Current_shader->program->Uniforms.setUniformi("buffer_matrix_offset", (int)GL_transform_buffer_offset);

		GL_state.Texture.Enable(render_pass, GL_TEXTURE_BUFFER, opengl_get_transform_buffer_texture());

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

		Current_shader->program->Uniforms.setUniform3f("stripe_color", stripe_color);
		Current_shader->program->Uniforms.setUniform3f("base_color", base_color);

		if ( bm_has_alpha_channel(material_info->get_texture_map(TM_MISC_TYPE)) ) {
			Current_shader->program->Uniforms.setUniformi("team_glow_enabled", 1);
		} else {
			Current_shader->program->Uniforms.setUniformi("team_glow_enabled", 0);
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_THRUSTER ) {
		Current_shader->program->Uniforms.setUniformf("thruster_scale", material_info->get_thrust_scale());
	}

	
	if ( Current_shader->flags & SDR_FLAG_MODEL_FOG ) {
		material::fog fog_params = material_info->get_fog();

		if ( fog_params.enabled ) {
			Current_shader->program->Uniforms.setUniformf("fogStart", fog_params.dist_near);
			Current_shader->program->Uniforms.setUniformf("fogScale", 1.0f / (fog_params.dist_far - fog_params.dist_near));
			Current_shader->program->Uniforms.setUniform4f("fogColor", i2fl(fog_params.r) / 255.0f, i2fl(fog_params.g) / 255.0f, i2fl(fog_params.b) / 255.0f, 1.0f);
		}
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_ALPHA ) {
		Current_shader->program->Uniforms.setUniform2f("normalAlphaMinMax", material_info->get_normal_alpha_min(), material_info->get_normal_alpha_max());
	}

	if ( Current_shader->flags & SDR_FLAG_MODEL_NORMAL_EXTRUDE ) {
		Current_shader->program->Uniforms.setUniformf("extrudeWidth", material_info->get_normal_extrude_width());
	}

	if ( Deferred_lighting ) {
		// don't blend if we're drawing to the g-buffers
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	}
}

void opengl_tnl_set_material_particle(particle_material * material_info)
{
	opengl_tnl_set_material(material_info, true);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", GL_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("baseMap", 0);
	Current_shader->program->Uniforms.setUniformi("depthMap", 1);
	Current_shader->program->Uniforms.setUniformf("window_width", (float)gr_screen.max_w);
	Current_shader->program->Uniforms.setUniformf("window_height", (float)gr_screen.max_h);
	Current_shader->program->Uniforms.setUniformf("nearZ", Min_draw_distance);
	Current_shader->program->Uniforms.setUniformf("farZ", Max_draw_distance);
	Current_shader->program->Uniforms.setUniformi("srgb", High_dynamic_range ? 1 : 0);
	Current_shader->program->Uniforms.setUniformi("blend_alpha", material_info->get_blend_mode() != ALPHA_BLEND_ADDITIVE);

	if ( Cmdline_no_deferred_lighting ) {
		Current_shader->program->Uniforms.setUniformi("linear_depth", 0);
	} else {
		Current_shader->program->Uniforms.setUniformi("linear_depth", 1);
	}

	if ( !Cmdline_no_deferred_lighting ) {
		Assert(Scene_position_texture != 0);

		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_position_texture);
	} else {
		Assert(Scene_depth_texture != 0);

		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_depth_texture);
	}
}
void opengl_tnl_set_material_batched(batched_bitmap_material* material_info) {
	// This material assumes that the array index is supplied via the vertex attributes
	opengl_tnl_set_material(material_info, true);

	Current_shader->program->Uniforms.setUniformf("intensity", material_info->get_color_scale());

	Current_shader->program->Uniforms.setUniform4f("color", material_info->get_color());

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", GL_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("baseMap", 0);
}

void opengl_tnl_set_material_distortion(distortion_material* material_info)
{
	opengl_tnl_set_material(material_info, true);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", GL_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("baseMap", 0);
	Current_shader->program->Uniforms.setUniformi("depthMap", 1);
	Current_shader->program->Uniforms.setUniformf("window_width", (float)gr_screen.max_w);
	Current_shader->program->Uniforms.setUniformf("window_height", (float)gr_screen.max_h);
	Current_shader->program->Uniforms.setUniformf("nearZ", Min_draw_distance);
	Current_shader->program->Uniforms.setUniformf("farZ", Max_draw_distance);
	Current_shader->program->Uniforms.setUniformi("frameBuffer", 2);

	GL_state.Texture.Enable(2, GL_TEXTURE_2D, Scene_effect_texture);

	Current_shader->program->Uniforms.setUniformi("distMap", 3);

	if(material_info->get_thruster_rendering()) {
		GL_state.Texture.Enable(3, GL_TEXTURE_2D, Distortion_texture[!Distortion_switch]);

		Current_shader->program->Uniforms.setUniformf("use_offset", 1.0f);
	} else {
		// Disable this texture unit
		GL_state.Texture.Enable(3, GL_TEXTURE_2D, 0);

		Current_shader->program->Uniforms.setUniformf("use_offset", 0.0f);
	}

	Assert(Scene_depth_texture != 0);

	GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_depth_texture);
}

void opengl_tnl_set_material_movie(movie_material* material_info) {
	opengl_tnl_set_material(material_info, false);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", GL_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", GL_projection_matrix);

	Current_shader->program->Uniforms.setUniformi("ytex", 0);
	Current_shader->program->Uniforms.setUniformi("utex", 1);
	Current_shader->program->Uniforms.setUniformi("vtex", 2);

	float u_scale, v_scale;
	uint32_t index;
	if ( !gr_opengl_tcache_set(material_info->getYtex(), material_info->get_texture_type(), &u_scale, &v_scale, &index, 0) ) {
		mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->getYtex()));
	}
	if ( !gr_opengl_tcache_set(material_info->getUtex(), material_info->get_texture_type(), &u_scale, &v_scale, &index, 1) ) {
		mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->getUtex()));
	}
	if ( !gr_opengl_tcache_set(material_info->getVtex(), material_info->get_texture_type(), &u_scale, &v_scale, &index, 2) ) {
		mprintf(("WARNING: Error setting bitmap texture (%i)!\n", material_info->getVtex()));
	}
}

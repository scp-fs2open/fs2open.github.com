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

#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"

#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltnl.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglstate.h"

#include "math/vecmat.h"
#include "render/3d.h"
#include "cmdline/cmdline.h"
#include "model/model.h"


extern int GLOWMAP;
extern int CLOAKMAP;
extern int SPECMAP;
extern int NORMMAP;
extern int HEIGHTMAP;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;
extern int Interp_multitex_cloakmap;
extern int Interp_cloakmap_alpha;
extern float Interp_light;

extern bool Basemap_override;
extern bool Envmap_override;
extern bool Specmap_override;
extern bool Normalmap_override;
extern bool Heightmap_override;
extern bool GLSL_override;

static int GL_modelview_matrix_depth = 1;
static int GL_htl_projection_matrix_set = 0;
static int GL_htl_view_matrix_set = 0;
static int GL_htl_2d_matrix_depth = 0;
static int GL_htl_2d_matrix_set = 0;

static GLfloat GL_env_texture_matrix[16] = { 0.0f };
static bool GL_env_texture_matrix_set = false;

int GL_vertex_data_in = 0;

GLint GL_max_elements_vertices = 4096;
GLint GL_max_elements_indices = 4096;


struct opengl_vertex_buffer {
	GLfloat *array_list;	// interleaved array
	GLubyte *index_list;

	GLuint vbo;			// buffer for VBO
	GLuint ibo;

	uint vbo_size;
	uint ibo_size;

	opengl_vertex_buffer() :
		array_list(NULL), index_list(NULL), vbo(0), ibo(0),
		vbo_size(0), ibo_size(0)
	{
	}

	void clear();
};

void opengl_vertex_buffer::clear()
{
	if (array_list) {
		vm_free(array_list);
	}

	if (vbo) {
		vglDeleteBuffersARB(1, &vbo);
		GL_vertex_data_in -= vbo_size;
	}

	if (index_list) {
		vm_free(index_list);
	}

	if (ibo) {
		vglDeleteBuffersARB(1, &ibo);
		GL_vertex_data_in -= ibo_size;
	}
}

static SCP_vector<opengl_vertex_buffer> GL_vertex_buffers;
static opengl_vertex_buffer *g_vbp = NULL;
static int GL_vertex_buffers_in_use = 0;


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

		vglGenBuffersARB(1, &vbp->vbo);

		// make sure we have one
		if (vbp->vbo) {
			vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
			vglBufferDataARB(GL_ARRAY_BUFFER_ARB, vbp->vbo_size, vbp->array_list, GL_STATIC_DRAW_ARB);

			// just in case
			if ( opengl_check_for_errors() ) {
				vglDeleteBuffersARB(1, &vbp->vbo);
				vbp->vbo = 0;
				return;
			}

			vglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

			vm_free(vbp->array_list);
			vbp->array_list = NULL;	

			GL_vertex_data_in += vbp->vbo_size;
		}
	}

	// create index buffer
	{
		// clear any existing errors
		glGetError();

		vglGenBuffersARB(1, &vbp->ibo);

		// make sure we have one
		if (vbp->ibo) {
			vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbp->ibo);
			vglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbp->ibo_size, vbp->index_list, GL_STATIC_DRAW_ARB);

			// just in case
			if ( opengl_check_for_errors() ) {
				vglDeleteBuffersARB(1, &vbp->ibo);
				vbp->ibo = 0;
				return;
			}

			vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

			vm_free(vbp->index_list);
			vbp->index_list = NULL;

			GL_vertex_data_in += vbp->ibo_size;
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

bool gr_opengl_config_buffer(const int buffer_id, vertex_buffer *vb)
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
	Verify( vb->model_list->vert != NULL );
	vb->stride += (3 * sizeof(GLfloat));

	// normals
	if (vb->flags & VB_FLAG_NORMAL) {
		Verify( vb->model_list->norm != NULL );
		vb->stride += (3 * sizeof(GLfloat));
	}

	// uv coords
	if (vb->flags & VB_FLAG_UV1) {
		vb->stride += (2 * sizeof(GLfloat));
	}

	// tangent space data for normal maps (shaders only)
	if (vb->flags & VB_FLAG_TANGENT) {
		Assert( Cmdline_normal );

		Verify( vb->model_list->tsb != NULL );
		vb->stride += (4 * sizeof(GLfloat));
	}

	// offsets for this chunk
	vb->vertex_offset = m_vbp->vbo_size;
	m_vbp->vbo_size += vb->stride * vb->model_list->n_verts;

	for (size_t idx = 0; idx < vb->tex_buf.size(); idx++) {
		buffer_data *bd = &vb->tex_buf[idx];

		bd->index_offset = m_vbp->ibo_size;
		m_vbp->ibo_size += bd->n_verts * ((bd->flags & VB_FLAG_LARGE_INDEX) ? sizeof(uint) : sizeof(ushort));
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

		// verts
		array[arsize++] = vl->world.xyz.x;
		array[arsize++] = vl->world.xyz.y;
		array[arsize++] = vl->world.xyz.z;
	}

	// generate the index array
	for (j = 0; j < vb->tex_buf.size(); j++) {
		n_verts = vb->tex_buf[j].n_verts;
		uint offset = vb->tex_buf[j].index_offset;
		const uint *index = vb->tex_buf[j].get_index();

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
			vglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
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

	GL_vertex_buffers.clear();
	GL_vertex_buffers_in_use = 0;
}

void opengl_tnl_init()
{
	GL_vertex_buffers.reserve(MAX_POLYGON_MODELS);
}

void opengl_tnl_shutdown()
{
	opengl_destroy_all_buffers();
}

static void opengl_init_arrays(opengl_vertex_buffer *vbp, const vertex_buffer *bufferp)
{
	GLint offset = (GLint)bufferp->vertex_offset;
	GLubyte *ptr = NULL;

	// vertex buffer

	if (vbp->vbo) {
		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
	} else {
		ptr = (GLubyte*)vbp->array_list;
	}

	if (bufferp->flags & VB_FLAG_UV1) {
		vglClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, bufferp->stride, ptr + offset);
		offset += (2 * sizeof(GLfloat));
	}

	if (bufferp->flags & VB_FLAG_NORMAL) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, bufferp->stride, ptr + offset);
		offset += (3 * sizeof(GLfloat));
	}

	if (bufferp->flags & VB_FLAG_TANGENT) {
		// we treat this as texture coords for ease of use
		// NOTE: this is forced on tex unit 1!!!
		vglClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(4, GL_FLOAT, bufferp->stride, ptr + offset);
		offset += (4 * sizeof(GLfloat));
	}

	Assert( bufferp->flags & VB_FLAG_POSITION );
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, bufferp->stride, ptr + offset);
	offset += (3 * sizeof(GLfloat));
}

#define DO_RENDER()	\
	if (Cmdline_drawelements) \
		glDrawElements(GL_TRIANGLES, count, element_type, ibuffer + (datap->index_offset + start)); \
	else \
		vglDrawRangeElements(GL_TRIANGLES, datap->i_first, datap->i_last, count, element_type, ibuffer + (datap->index_offset + start));

int GL_last_shader_flags = -1;
int GL_last_shader_index = -1;

static void opengl_render_pipeline_fixed(int start, const vertex_buffer *bufferp, const buffer_data *datap, int flags);

extern bool Scene_framebuffer_in_frame;
extern GLuint Framebuffer_fallback_texture_id;
static void opengl_render_pipeline_program(int start, const vertex_buffer *bufferp, const buffer_data *datap, int flags)
{
	float u_scale, v_scale;
	int render_pass = 0;
	int shader_flags = 0;
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
	if (lighting_is_enabled) {
		shader_flags |= SDR_FLAG_LIGHT;
	}

	if ( GL_state.Fog() ) {
		shader_flags |= SDR_FLAG_FOG;
	}

	if (flags & TMAP_ANIMATED_SHADER)
		shader_flags |= SDR_FLAG_ANIMATED;

	if (textured) {
		if ( !Basemap_override ) {
			shader_flags |= SDR_FLAG_DIFFUSE_MAP;
		}

		if (GLOWMAP > 0) {
			shader_flags |= SDR_FLAG_GLOW_MAP;
		}

		if (lighting_is_enabled) {
			if ( (SPECMAP > 0) && !Specmap_override ) {
				shader_flags |= SDR_FLAG_SPEC_MAP;
		
				if ( (ENVMAP > 0) && !Envmap_override ) {
					shader_flags |= SDR_FLAG_ENV_MAP;
				}
			}

			if ( (NORMMAP > 0) && GL_state.Light(0) && !Normalmap_override ) {
				shader_flags |= SDR_FLAG_NORMAL_MAP;

				if ( (HEIGHTMAP > 0) && !Heightmap_override ) {
					shader_flags |= SDR_FLAG_HEIGHT_MAP;
				}
			}
		}
	}

	// find proper shader
	if (shader_flags == GL_last_shader_flags) {
		sdr_index = GL_last_shader_index;
	} else {
		sdr_index = opengl_shader_get_index(shader_flags);

		if (sdr_index < 0) {
			opengl_render_pipeline_fixed(start, bufferp, datap, flags);
			return;
		}

		GL_last_shader_index = sdr_index;
		GL_last_shader_flags = shader_flags;
	}

	Assert( sdr_index >= 0 );

	opengl_shader_set_current( &GL_shader[sdr_index] );


	opengl_default_light_settings( !GL_center_alpha, (Interp_light > 0.25f) );
	gr_opengl_set_center_alpha(GL_center_alpha);

	opengl_setup_render_states(r, g, b, a, tmap_type, flags);
	glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );


	render_pass = 0;

	GL_state.Texture.SetShaderMode(GL_TRUE);


	// basic setup of all data
	opengl_init_arrays(vbp, bufferp);

	if (vbp->ibo) {
		vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbp->ibo);
	} else {
		ibuffer = (GLubyte*)vbp->index_list;
	}

	if(flags & TMAP_ANIMATED_SHADER)
	{
		vglUniform1fARB( opengl_shader_get_uniform("anim_timer"), opengl_shader_get_animated_timer() );
		vglUniform1iARB( opengl_shader_get_uniform("effect_num"), opengl_shader_get_animated_effect() );
		vglUniform1fARB( opengl_shader_get_uniform("vpwidth"), 1.0f/gr_screen.max_w );
		vglUniform1fARB( opengl_shader_get_uniform("vpheight"), 1.0f/gr_screen.max_h );
	}
	int n_lights = MIN(Num_active_gl_lights, GL_max_lights) - 1;
	vglUniform1iARB( opengl_shader_get_uniform("n_lights"), n_lights );

	// base texture
	if (shader_flags & SDR_FLAG_DIFFUSE_MAP) {
		vglUniform1iARB( opengl_shader_get_uniform("sBasemap"), render_pass );

		gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, render_pass);
	
		render_pass++; // bump!
	}

	if (shader_flags & SDR_FLAG_GLOW_MAP) {
		vglUniform1iARB( opengl_shader_get_uniform("sGlowmap"), render_pass );

		gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, render_pass);

		render_pass++; // bump!
	}

	if (shader_flags & SDR_FLAG_SPEC_MAP) {
		vglUniform1iARB( opengl_shader_get_uniform("sSpecmap"), render_pass );

		gr_opengl_tcache_set(SPECMAP, tmap_type, &u_scale, &v_scale, render_pass);

		render_pass++;

		if (shader_flags & SDR_FLAG_ENV_MAP) {
			// 0 == env with non-alpha specmap, 1 == env with alpha specmap
			int alpha_spec = bm_has_alpha_channel(SPECMAP);

			vglUniform1iARB( opengl_shader_get_uniform("alpha_spec"), alpha_spec );
			vglUniformMatrix4fvARB( opengl_shader_get_uniform("envMatrix"), 1, GL_FALSE, &GL_env_texture_matrix[0] );
			vglUniform1iARB( opengl_shader_get_uniform("sEnvmap"), render_pass );
	
			gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, render_pass);
	
			render_pass++;
		}
	}

	if (shader_flags & SDR_FLAG_NORMAL_MAP) {
		vglUniform1iARB( opengl_shader_get_uniform("sNormalmap"), render_pass );

		gr_opengl_tcache_set(NORMMAP, tmap_type, &u_scale, &v_scale, render_pass);

		render_pass++; // bump!

		if (shader_flags & SDR_FLAG_HEIGHT_MAP) {
			vglUniform1iARB( opengl_shader_get_uniform("sHeightmap"), render_pass );

			gr_opengl_tcache_set(HEIGHTMAP, tmap_type, &u_scale, &v_scale, render_pass);

			render_pass++;
		}
	}

	if ((shader_flags & SDR_FLAG_ANIMATED))
	{
		GL_state.Texture.SetActiveUnit(render_pass);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		if( Scene_framebuffer_in_frame )
		{
			GL_state.Texture.Enable(Scene_effect_texture);
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		}
		else
			GL_state.Texture.Enable(Framebuffer_fallback_texture_id);
		vglUniform1iARB( opengl_shader_get_uniform("sFramebuffer"), render_pass );
		render_pass++;
	}
	// DRAW IT!!
	DO_RENDER();
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

	// make sure everthing gets turned back off
	GL_state.Texture.SetShaderMode(GL_FALSE);
	GL_state.Texture.DisableAll();
	vglClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	vglClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
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

	opengl_default_light_settings( !GL_center_alpha, (Interp_light > 0.25f), (using_spec) ? 0 : 1 );
	gr_opengl_set_center_alpha(GL_center_alpha);

	opengl_setup_render_states(r, g, b, a, tmap_type, flags);
	glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );

	// basic setup of all data
	opengl_init_arrays(vbp, bufferp);

	if (vbp->ibo) {
		vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbp->ibo);
	} else {
		ibuffer = (GLubyte*)vbp->index_list;
	}

	if ( !vbp->vbo ) {
		vbuffer = (GLubyte*)vbp->array_list;
	}

	#define BUFFER_OFFSET(off) (vbuffer+bufferp->vertex_offset+(off))

// -------- Begin 1st PASS (base texture, glow) ---------------------------------- //
	if (textured) {
		render_pass = 0;

		// base texture
		if ( !Basemap_override ) {
			vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );

			gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, render_pass);

			// increment texture count for this pass
			render_pass++; // bump!
		}

		// glowmaps!
		if (using_glow) {
			vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );

			// set glowmap on relevant ARB
			gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, render_pass);

			opengl_set_additive_tex_env();

			render_pass++; // bump!
		}
	}

	// DRAW IT!!
	DO_RENDER();
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
		vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );

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
		vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );

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
		DO_RENDER();

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
		vglClientActiveTextureARB(GL_TEXTURE1_ARB);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		render_pass = 0;

		vglClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer( 2, GL_FLOAT, bufferp->stride, BUFFER_OFFSET(0) );

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
		DO_RENDER();

		opengl_default_light_settings();

		GL_state.Texture.SetRGBScale(1.0f);
	}
// -------- End 4th PASS --------------------------------------------------------- //

	// make sure everthing gets turned back off
	GL_state.Texture.DisableAll();
	GL_state.Normalize(GL_FALSE);
	vglClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	vglClientActiveTextureARB(GL_TEXTURE0_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
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

void gr_opengl_start_instance_matrix(vec3d *offset, matrix *rotation)
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

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	vec3d axis;
	float ang;
	vm_matrix_to_rot_axis_and_angle(rotation, &ang, &axis);

	glTranslatef( offset->xyz.x, offset->xyz.y, offset->xyz.z );
	glRotatef( fl_degrees(ang), axis.xyz.x, axis.xyz.y, axis.xyz.z );

	GL_CHECK_FOR_ERRORS("end of start_instance_matrix()");

	GL_modelview_matrix_depth++;
}

void gr_opengl_start_instance_angles(vec3d *pos, angles *rotation)
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

	glMatrixMode(GL_MODELVIEW);
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


static GLdouble eyex, eyey, eyez;
static GLdouble vmatrix[16];

static vec3d last_view_pos;
static matrix last_view_orient;

static bool use_last_view = false;

void gr_opengl_set_view_matrix(vec3d *pos, matrix *orient)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_modelview_matrix_depth == 1);

	GL_CHECK_FOR_ERRORS("start of set_view_matrix()");

	glMatrixMode(GL_MODELVIEW);
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
			GL_env_texture_matrix[1]  = -mview[4];
			GL_env_texture_matrix[2]  =  mview[8];
			// u.xyz  <--  r.y, u.y, f.y
			GL_env_texture_matrix[4]  =  mview[1];
			GL_env_texture_matrix[5]  = -mview[5];
			GL_env_texture_matrix[6]  =  mview[9];
			// f.xyz  <--  r.z, u.z, f.z
			GL_env_texture_matrix[8]  =  mview[2];
			GL_env_texture_matrix[9]  = -mview[6];
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

	glMatrixMode(GL_MODELVIEW);
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

void gr_opengl_push_scale_matrix(vec3d *scale_factor)
{
	if (GL_scale_matrix_set) 
	{
		Int3();
		return;
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	GL_modelview_matrix_depth++;

	glScalef(scale_factor->xyz.x, scale_factor->xyz.y, scale_factor->xyz.z);
}

void gr_opengl_pop_scale_matrix()
{
	if (GL_scale_matrix_set) 
	{
		Int3();
		return;
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	GL_modelview_matrix_depth--;
}

void gr_opengl_end_clip_plane()
{
	if (Cmdline_nohtl) {
		return;
	}

	GL_state.ClipPlane(0, GL_FALSE);
}

void gr_opengl_start_clip_plane()
{
	if (Cmdline_nohtl) {
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

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


#include <vector>


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
	uint stride;		// the current stride
	uint n_prim;
	uint n_verts;
	GLfloat *array_list;	// interleaved array
	GLuint vbo;			// buffer for VBO
	uint flags;			// FVF
	uint vbo_size;

	opengl_vertex_buffer() { memset( this, 0, sizeof(opengl_vertex_buffer) ); }
};


static std::vector<opengl_vertex_buffer> GL_vertex_buffers;
static opengl_vertex_buffer *g_vbp = NULL;
static int GL_vertex_buffers_in_use = 0;


GLuint opengl_create_vbo(uint size, GLfloat *data)
{
	if (data == NULL) {
		return 0;
	}

	if (size == 0) {
		return 0;
	}

	GLuint buffer_name = 0;

	// clear any existing errors
	glGetError();

	vglGenBuffersARB(1, &buffer_name);

	// make sure we have one
	if (buffer_name) {
		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer_name);
		vglBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data, GL_STATIC_DRAW_ARB);

		// just in case
		if ( opengl_check_for_errors() ) {
			vglDeleteBuffersARB(1, &buffer_name);
			return 0;
		}

		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}

	return buffer_name;
}

int gr_opengl_make_buffer(poly_list *list, uint flags)
{
	if (Cmdline_nohtl) {
		return -1;
	}

	int i;
	uint arsize = 0, list_size = 0;
	opengl_vertex_buffer vbuffer;

	// clear out any old errors before we continue with this
	opengl_check_for_errors();

	if ( !(flags & VERTEX_FLAG_POSITION) ) {
		Int3();
		return -1;
	}

	vbuffer.stride = 0;

	// setup using flags...

	// verts
	Verify( list->vert != NULL );
	vbuffer.stride += (3 * sizeof(GLfloat));

	// normals
	if (flags & VERTEX_FLAG_NORMAL) {
		Verify( list->norm != NULL );
		vbuffer.stride += (3 * sizeof(GLfloat));
	}

	// uv coords
	if (flags & VERTEX_FLAG_UV1) {
		vbuffer.stride += (2 * sizeof(GLfloat));
	}

	// tangent space data for normal maps (shaders only)
	if (flags & VERTEX_FLAG_TANGENT) {
		Verify( list->tsb != NULL );
		Assert( Cmdline_normal );

		vbuffer.stride += (4 * sizeof(GLfloat));
	}

	// total size of data
	list_size = vbuffer.stride * list->n_verts;

	// allocate the storage list
	vbuffer.array_list = (GLfloat*)vm_malloc_q(list_size);

	// return invalid if we don't have the memory
	if (vbuffer.array_list == NULL) {
		return -1;
	}

	memset( vbuffer.array_list, 0, list_size );

	// generate the array
	for (i = 0; i < list->n_verts; i++) {
		vertex *vl = &list->vert[i];

		// don't try to generate more data than what's available
		Assert( ((arsize * sizeof(GLfloat)) + vbuffer.stride) <= list_size );

		// NOTE: UV->NORM->TSB->VERT, This array order *must* be preserved!!

		// tex coords
		if (flags & VERTEX_FLAG_UV1) {
			vbuffer.array_list[arsize++] = vl->u;
			vbuffer.array_list[arsize++] = vl->v;
		}

		// normals
		if (flags & VERTEX_FLAG_NORMAL) {
			vec3d *nl = &list->norm[i];
			vbuffer.array_list[arsize++] = nl->xyz.x;
			vbuffer.array_list[arsize++] = nl->xyz.y;
			vbuffer.array_list[arsize++] = nl->xyz.z;
		}

		// tangent space data
		if (flags & VERTEX_FLAG_TANGENT) {
			tsb_t *tsb = &list->tsb[i];
			vbuffer.array_list[arsize++] = tsb->tangent.xyz.x;
			vbuffer.array_list[arsize++] = tsb->tangent.xyz.y;
			vbuffer.array_list[arsize++] = tsb->tangent.xyz.z;
			vbuffer.array_list[arsize++] = tsb->scaler;
		}

		// verts
		vbuffer.array_list[arsize++] = vl->x;
		vbuffer.array_list[arsize++] = vl->y;
		vbuffer.array_list[arsize++] = vl->z;
	}

	vbuffer.flags = flags;

	vbuffer.n_prim = (list->n_verts / 3);
	vbuffer.n_verts = list->n_verts;

	// maybe load it into a vertex buffer object
	if (Use_VBOs) {
		vbuffer.vbo = opengl_create_vbo(list_size, vbuffer.array_list);

		if (vbuffer.vbo) {
			// figure up the size so we can know how much VBO data is in card memory
			vbuffer.vbo_size = list_size;
			GL_vertex_data_in += list_size;

			vm_free(vbuffer.array_list);
			vbuffer.array_list = NULL;
		}
	}

	// allocate additional blocks if we need them to prevent memory fragmentation, 500 at a time
	if ( GL_vertex_buffers.size() >= GL_vertex_buffers.capacity() ) {
		GL_vertex_buffers.reserve( GL_vertex_buffers.size() + 500 );
	}

	GL_vertex_buffers.push_back( vbuffer );
	GL_vertex_buffers_in_use++;

	return (int)(GL_vertex_buffers.size() - 1);
}

void gr_opengl_set_buffer(int idx)
{
	if (Cmdline_nohtl) {
		return;
	}

	g_vbp = NULL;

	if (idx < 0) {
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

	if (vbp->array_list) {
		vm_free(vbp->array_list);
	}

	if (vbp->vbo) {
		vglDeleteBuffersARB(1, &vbp->vbo);
		GL_vertex_data_in -= vbp->vbo_size;
	}

	memset( vbp, 0, sizeof(opengl_vertex_buffer) );

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
	GL_vertex_buffers.reserve(1000);
}

void opengl_tnl_shutdown()
{
	opengl_destroy_all_buffers();
}

static void opengl_init_arrays(opengl_vertex_buffer *vbp)
{
	GLint offset = 0;
	GLubyte *ptr = NULL;

	if (vbp->vbo) {
		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
	} else {
		ptr = (GLubyte*)vbp->array_list;
	}

	if (vbp->flags & VERTEX_FLAG_UV1) {
		vglClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, vbp->stride, ptr + offset);
		offset += (2 * sizeof(GLfloat));
	}

	if (vbp->flags & VERTEX_FLAG_NORMAL) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, vbp->stride, ptr + offset);
		offset += (3 * sizeof(GLfloat));
	}

	if (vbp->flags & VERTEX_FLAG_TANGENT) {
		// we treat this as texture coords for ease of use
		// NOTE: this is forced on tex unit 1!!!
		vglClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(4, GL_FLOAT, vbp->stride, ptr + offset);
		offset += (4 * sizeof(GLfloat));
	}

	Assert( vbp->flags & VERTEX_FLAG_POSITION );
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, vbp->stride, ptr + offset);
	offset += (3 * sizeof(GLfloat));
}

#define DO_RENDER() {	\
	if (sbuffer) {	\
		vglDrawRangeElements(GL_TRIANGLES, start, end, count, GL_UNSIGNED_SHORT, sbuffer + start);	\
	} else if (ibuffer) {	\
		vglDrawRangeElements(GL_TRIANGLES, start, end, count, GL_UNSIGNED_INT, ibuffer + start);	\
	} else {	\
		glDrawArrays(GL_TRIANGLES, 0, vbp->n_verts);	\
	}	\
}

int GL_last_shader_flags = -1;
int GL_last_shader_index = -1;

static void opengl_render_pipeline_fixed(int start, int n_prim, ushort *sbuffer, uint *ibuffer, int flags);

static void opengl_render_pipeline_program(int start, int n_prim, ushort *sbuffer, uint *ibuffer, int flags)
{
	float u_scale, v_scale;
	int render_pass = 0;
	int shader_flags = 0;
	int sdr_index = -1;
	int r, g, b, a, tmap_type;

	int end = ((n_prim * 3) - 1);
	int count = (end - start + 1);

	opengl_vertex_buffer *vbp = g_vbp;
	Assert( vbp );

	int textured = ((flags & TMAP_FLAG_TEXTURED) && (vbp->flags & VERTEX_FLAG_UV1));

	// init lights
	opengl_change_active_lights(0);
	opengl_default_light_settings( !GL_center_alpha, (Interp_light > 0.25f) );
	gr_opengl_set_center_alpha(GL_center_alpha);

	opengl_setup_render_states(r, g, b, a, tmap_type, flags);
	glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );

	// setup shader flags for the things that we want/need
	if (lighting_is_enabled) {
		shader_flags |= SDR_FLAG_LIGHT;
	}

	if ( GL_state.Fog() ) {
		shader_flags |= SDR_FLAG_FOG;
	}

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

	if (shader_flags == GL_last_shader_flags) {
		sdr_index = GL_last_shader_index;
	} else {
		sdr_index = opengl_shader_get_index(shader_flags);

		if (sdr_index < 0) {
			opengl_render_pipeline_fixed(start, n_prim, sbuffer, ibuffer, flags);
			return;
		}

		GL_last_shader_flags = shader_flags;
		GL_last_shader_index = sdr_index;
	}

	Assert( sdr_index >= 0 );

	opengl_shader_set_current( &GL_shader[sdr_index] );

	render_pass = 0;

	GL_state.Texture.SetShaderMode(GL_TRUE);

	// basic setup of all data
	opengl_init_arrays(vbp);

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
	
			vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
	
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

	// DRAW IT!!
	DO_RENDER();

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
	opengl_shader_set_current();
	GL_state.Texture.SetShaderMode(GL_FALSE);
	GL_state.Texture.DisableAll();
	vglClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	vglClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

static void opengl_render_pipeline_fixed(int start, int n_prim, ushort *sbuffer, uint *ibuffer, int flags)
{
	float u_scale, v_scale;
	int render_pass = 0;
	int r, g, b, a, tmap_type;

	bool rendered_env = false;
	bool using_glow = false;
	bool using_spec = false;
	bool using_env = false;

	int end = ((n_prim * 3) - 1);
	int count = (end - start + 1);

	opengl_vertex_buffer *vbp = g_vbp;
	Assert( vbp );

	int textured = ((flags & TMAP_FLAG_TEXTURED) && (vbp->flags & VERTEX_FLAG_UV1));

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

	// init lights
	opengl_change_active_lights(0);
	opengl_default_light_settings( !GL_center_alpha, (Interp_light > 0.25f), (using_spec) ? 0 : 1 );
	gr_opengl_set_center_alpha(GL_center_alpha);

	opengl_setup_render_states(r, g, b, a, tmap_type, flags);
	glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );

	// basic setup of all data
	opengl_init_arrays(vbp);

// -------- Begin 1st PASS (base texture, glow) ---------------------------------- //
	if (textured) {
		render_pass = 0;

		// base texture
		if ( !Basemap_override ) {
			vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			if (vbp->vbo) {
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
			} else {
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
			}

			gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, render_pass);

			// increment texture count for this pass
			render_pass++; // bump!
		}

		// glowmaps!
		if (using_glow) {
			vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			if (vbp->vbo) {
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
			} else {
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
			}

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
		if (vbp->vbo) {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
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
		vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo) {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
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
		if (vbp->vbo) {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
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
void gr_opengl_render_buffer(int start, int n_prim, ushort *sbuffer, uint *ibuffer, int flags)
{
	Assert( GL_htl_projection_matrix_set );
	Assert( GL_htl_view_matrix_set );

	GL_CHECK_FOR_ERRORS("start of render_buffer()");

	if ( GL_state.CullFace() ) {
		GL_state.FrontFaceValue(GL_CW);
	}

	if ( Use_GLSL && !GLSL_override ) {
		opengl_render_pipeline_program(start, n_prim, sbuffer, ibuffer, flags);
	} else {
		opengl_render_pipeline_fixed(start, n_prim, sbuffer, ibuffer, flags);
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

	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble clip_width, clip_height;

	clip_height = tan( (double)fov * 0.5 ) * z_near;
	clip_width = clip_height * (GLdouble)aspect;

	glFrustum( -clip_width, clip_width, -clip_height, clip_height, z_near, z_far );

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
	if (GL_rendering_to_framebuffer) {
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
	if (GL_rendering_to_framebuffer) {
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

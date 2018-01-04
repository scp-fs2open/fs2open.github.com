

#include "graphics/2d.h"
#include "gropenglstate.h"
#include "gropengldraw.h"
#include "gropengldeferred.h"
#include "gropengltnl.h"
#include "graphics/matrix.h"
#include "graphics/util/UniformAligner.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/util/UniformBuffer.h"
#include "tracing/tracing.h"
#include "lighting/lighting.h"
#include "render/3d.h"
#include "ShaderProgram.h"

void gr_opengl_deferred_init() {
	gr_opengl_deferred_light_cylinder_init(16);
	gr_opengl_deferred_light_sphere_init(16, 16);
}
void gr_opengl_deferred_shutdown() {
}

void opengl_clear_deferred_buffers()
{
	GR_DEBUG_SCOPE("Clear deferred buffers");

	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_CLEAR, 0) );

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	opengl_shader_set_current();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);
}

void gr_opengl_deferred_lighting_begin()
{
	if ( Cmdline_no_deferred_lighting)
		return;

	GR_DEBUG_SCOPE("Deferred lighting begin");

	Deferred_lighting = true;
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, buffers);
}

void gr_opengl_deferred_lighting_end()
{
	if(!Deferred_lighting)
		return;

	GR_DEBUG_SCOPE("Deferred lighting end");

	Deferred_lighting = false;
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
}

extern light Lights[MAX_LIGHTS];
extern int Num_lights;
extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;

void gr_opengl_deferred_lighting_finish()
{
	GR_DEBUG_SCOPE("Deferred lighting finish");
	TRACE_SCOPE(tracing::ApplyLights);

	if ( Cmdline_no_deferred_lighting ) {
		return;
	}

	GL_state.SetAlphaBlendMode( ALPHA_BLEND_ADDITIVE);
	gr_zbuffer_set(GR_ZBUFF_NONE);

	//GL_state.DepthFunc(GL_GREATER);
	//GL_state.DepthMask(GL_FALSE);

	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_LIGHTING, 0) );

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_luminance_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Scene_stencil_buffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Scene_stencil_buffer);

	GL_state.Texture.SetShaderMode(GL_TRUE);

	GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_color_texture);

	GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_normal_texture);

	GL_state.Texture.Enable(2, GL_TEXTURE_2D, Scene_position_texture);

	GL_state.Texture.Enable(3, GL_TEXTURE_2D, Scene_specular_texture);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	light lights_copy[MAX_LIGHTS];
	memcpy(lights_copy, Lights, MAX_LIGHTS * sizeof(light));

	std::sort(lights_copy, lights_copy+Num_lights, light_compare_by_type);

	using namespace graphics;

	// Get a uniform buffer for out data
	auto buffer = gr_get_uniform_buffer(uniform_block_type::Lights);
	auto& uniformAligner = buffer->aligner();

	{
		GR_DEBUG_SCOPE("Build buffer data");

		for (int i = 0; i < Num_lights; ++i) {
			light* l = &lights_copy[i];
			if (l->type != LT_CONE && l->type != LT_POINT && l->type != LT_TUBE) {
				continue;
			}

			auto light_data = uniformAligner.addTypedElement<deferred_light_data>();

			light_data->lightType = 0;

			vec3d diffuse;
			diffuse.xyz.x = l->r * l->intensity;
			diffuse.xyz.y = l->g * l->intensity;
			diffuse.xyz.z = l->b * l->intensity;

			vec3d spec;
			spec.xyz.x = l->spec_r * l->intensity;
			spec.xyz.y = l->spec_g * l->intensity;
			spec.xyz.z = l->spec_b * l->intensity;

			switch (l->type) {
			case LT_CONE:
				light_data->lightType = 2;
				light_data->dualCone = l->dual_cone ? 1.0f : 0.0f;
				light_data->coneAngle = l->cone_angle;
				light_data->coneInnerAngle = l->cone_inner_angle;
				light_data->coneDir = l->vec2;
				FALLTHROUGH;
			case LT_POINT:
				light_data->diffuseLightColor = diffuse;
				light_data->specLightColor = spec;
				light_data->lightRadius = MAX(l->rada, l->radb) * 1.25f;

				light_data->scale.xyz.x = MAX(l->rada, l->radb) * 1.28f;
				light_data->scale.xyz.y = MAX(l->rada, l->radb) * 1.28f;
				light_data->scale.xyz.z = MAX(l->rada, l->radb) * 1.28f;
				break;
			case LT_TUBE:
				light_data->diffuseLightColor = diffuse;
				light_data->specLightColor = spec;
				light_data->lightRadius = l->radb * 1.5f;
				light_data->lightType = 1;

				vec3d a;
				vm_vec_sub(&a, &l->vec, &l->vec2);
				auto length = vm_vec_mag(&a);

				light_data->scale.xyz.x = l->radb * 1.53f;
				light_data->scale.xyz.y = l->radb * 1.53f;
				light_data->scale.xyz.z = length;

				// Tube lights consist of two different types of lights with almost the same properties
				light_data = uniformAligner.addTypedElement<deferred_light_data>();
				light_data->diffuseLightColor = diffuse;
				light_data->specLightColor = spec;
				light_data->lightRadius = l->radb * 1.5f;
				light_data->lightType = 0;

				light_data->scale.xyz.x = l->radb * 1.53f;
				light_data->scale.xyz.y = l->radb * 1.53f;
				light_data->scale.xyz.z = l->radb * 1.53f;
				break;
			}
		}

		// Uniform data has been assembled, upload it to the GPU and issue the draw calls
		buffer->submitData();
	}

	{
		GR_DEBUG_SCOPE("Render light geometry");

		size_t element_index = 0;
		for (int i = 0; i < Num_lights; ++i) {
			GR_DEBUG_SCOPE("Deferred apply single light");

			light* l = &lights_copy[i];
			switch (l->type) {
			case LT_CONE:
			case LT_POINT:
				gr_bind_uniform_buffer(uniform_block_type::Lights,
									   uniformAligner.getOffset(element_index),
									   sizeof(graphics::deferred_light_data),
									   buffer->bufferHandle());
				gr_opengl_draw_deferred_light_sphere(&l->vec);
				++element_index;
				break;
			case LT_TUBE:
				gr_bind_uniform_buffer(uniform_block_type::Lights,
									   uniformAligner.getOffset(element_index),
									   sizeof(graphics::deferred_light_data),
									   buffer->bufferHandle());

				vec3d a;
				matrix orient;
				vm_vec_sub(&a, &l->vec, &l->vec2);
				vm_vector_2_matrix(&orient, &a, NULL, NULL);

				gr_opengl_draw_deferred_light_cylinder(&l->vec2, &orient);
				++element_index;

				// The next two draws use the same uniform block element
				gr_bind_uniform_buffer(uniform_block_type::Lights,
									   uniformAligner.getOffset(element_index),
									   sizeof(graphics::deferred_light_data),
									   buffer->bufferHandle());

				gr_opengl_draw_deferred_light_sphere(&l->vec);
				gr_opengl_draw_deferred_light_sphere(&l->vec2);
				++element_index;
				break;
			}
		}

		// We don't need the buffer anymore
		buffer->finished();
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_color_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Scene_stencil_buffer);

	gr_end_view_matrix();
	gr_end_proj_matrix();

	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	if ( High_dynamic_range ) {
		High_dynamic_range = false;
		opengl_shader_set_passthrough(true);
		High_dynamic_range = true;
	} else {
		opengl_shader_set_passthrough(true);
	}

	GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_luminance_texture);

	GL_state.SetAlphaBlendMode( ALPHA_BLEND_ADDITIVE );
	GL_state.DepthMask(GL_FALSE);

	opengl_draw_textured_quad(0.0f, 0.0f, 0.0f, Scene_texture_v_scale, (float)gr_screen.max_w, (float)gr_screen.max_h, Scene_texture_u_scale, 0.0f);

	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	// reset state
	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);

	GL_state.SetAlphaBlendMode( ALPHA_BLEND_NONE );
	GL_state.Texture.SetShaderMode(GL_FALSE);

	gr_clear_states();
}


void gr_opengl_draw_deferred_light_sphere(const vec3d *position)
{
	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", gr_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", gr_projection_matrix);

	opengl_draw_sphere();

	g3_done_instance(true);
}


static GLuint deferred_light_cylinder_vbo = 0;
static GLuint deferred_light_cylinder_ibo = 0;
static GLushort deferred_light_cylinder_vcount = 0;
static GLuint deferred_light_cylinder_icount = 0;

void gr_opengl_deferred_light_cylinder_init(int segments) // Generate a VBO of a cylinder of radius and height 1.0f, based on code at http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
{
	unsigned int nVertex = (segments + 1) * 2 * 3 + 6; // Can someone verify this?
	unsigned int nIndex = deferred_light_cylinder_icount = 12 * (segments + 1) - 6; //This too
	float *Vertices = (float*)vm_malloc(sizeof(float) * nVertex);
	float *pVertex = Vertices;
	ushort *Indices = (ushort*)vm_malloc(sizeof(ushort) * nIndex);
	ushort *pIndex = Indices;

	float fDeltaSegAngle = (2.0f * PI / segments);
	unsigned short wVerticeIndex = 0 ;

	*pVertex++ = 0.0f;
	*pVertex++ = 0.0f;
	*pVertex++ = 0.0f;
	wVerticeIndex ++;
	*pVertex++ = 0.0f;
	*pVertex++ = 0.0f;
	*pVertex++ = 1.0f;
	wVerticeIndex ++;

	for( int ring = 0; ring <= 1; ring++ ) {
		float z0 = (float)ring;

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= segments; seg++) {
			float x0 = sinf(seg * fDeltaSegAngle);
			float y0 = cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the cylinder
			*pVertex++ = x0;
			*pVertex++ = y0;
			*pVertex++ = z0;

			if (!ring) {
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex;
				*pIndex++ = wVerticeIndex + (ushort)segments;
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex + 1;
				*pIndex++ = wVerticeIndex;
				if(seg != segments)
				{
					*pIndex++ = wVerticeIndex + 1;
					*pIndex++ = wVerticeIndex;
					*pIndex++ = 0;
				}
				wVerticeIndex ++;
			}
			else
			{
				if(seg != segments)
				{
					*pIndex++ = wVerticeIndex + 1;
					*pIndex++ = wVerticeIndex;
					*pIndex++ = 1;
					wVerticeIndex ++;
				}
			}
		}; // end for seg
	} // end for ring

	deferred_light_cylinder_vcount = wVerticeIndex;

	glGetError();

	glGenBuffers(1, &deferred_light_cylinder_vbo);

	// make sure we have one
	if (deferred_light_cylinder_vbo) {
		glBindBuffer(GL_ARRAY_BUFFER, deferred_light_cylinder_vbo);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float), Vertices, GL_STATIC_DRAW);

		// just in case
		if ( opengl_check_for_errors() ) {
			glDeleteBuffers(1, &deferred_light_cylinder_vbo);
			deferred_light_cylinder_vbo = 0;
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vm_free(Vertices);
		Vertices = NULL;
	}

	glGenBuffers(1, &deferred_light_cylinder_ibo);

	// make sure we have one
	if (deferred_light_cylinder_ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, deferred_light_cylinder_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndex * sizeof(ushort), Indices, GL_STATIC_DRAW);

		// just in case
		if ( opengl_check_for_errors() ) {
			glDeleteBuffers(1, &deferred_light_cylinder_ibo);
			deferred_light_cylinder_ibo = 0;
			return;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		vm_free(Indices);
		Indices = NULL;
	}

}

static GLuint deferred_light_sphere_vbo = 0;
static GLuint deferred_light_sphere_ibo = 0;
static GLushort deferred_light_sphere_vcount = 0;
static GLuint deferred_light_sphere_icount = 0;

void gr_opengl_deferred_light_sphere_init(int rings, int segments) // Generate a VBO of a sphere of radius 1.0f, based on code at http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
{
	unsigned int nVertex = (rings + 1) * (segments+1) * 3;
	unsigned int nIndex = deferred_light_sphere_icount = 6 * rings * (segments + 1);
	float *Vertices = (float*)vm_malloc(sizeof(float) * nVertex);
	float *pVertex = Vertices;
	ushort *Indices = (ushort*)vm_malloc(sizeof(ushort) * nIndex);
	ushort *pIndex = Indices;

	float fDeltaRingAngle = (PI / rings);
	float fDeltaSegAngle = (2.0f * PI / segments);
	unsigned short wVerticeIndex = 0 ;

	// Generate the group of rings for the sphere
	for( int ring = 0; ring <= rings; ring++ ) {
		float r0 = sinf (ring * fDeltaRingAngle);
		float y0 = cosf (ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= segments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			*pVertex++ = x0;
			*pVertex++ = y0;
			*pVertex++ = z0;

			if (ring != rings) {
				// each vertex (except the last) has six indices pointing to it
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex;
				*pIndex++ = wVerticeIndex + (ushort)segments;
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex + 1;
				*pIndex++ = wVerticeIndex;
				wVerticeIndex ++;
			}
		}; // end for seg
	} // end for ring

	deferred_light_sphere_vcount = wVerticeIndex;

	glGetError();

	glGenBuffers(1, &deferred_light_sphere_vbo);

	// make sure we have one
	if (deferred_light_sphere_vbo) {
		glBindBuffer(GL_ARRAY_BUFFER, deferred_light_sphere_vbo);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float), Vertices, GL_STATIC_DRAW);

		// just in case
		if ( opengl_check_for_errors() ) {
			glDeleteBuffers(1, &deferred_light_sphere_vbo);
			deferred_light_sphere_vbo = 0;
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vm_free(Vertices);
		Vertices = NULL;
	}

	glGenBuffers(1, &deferred_light_sphere_ibo);

	// make sure we have one
	if (deferred_light_sphere_ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, deferred_light_sphere_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndex * sizeof(ushort), Indices, GL_STATIC_DRAW);

		// just in case
		if ( opengl_check_for_errors() ) {
			glDeleteBuffers(1, &deferred_light_sphere_ibo);
			deferred_light_sphere_ibo = 0;
			return;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		vm_free(Indices);
		Indices = NULL;
	}

}

void opengl_draw_sphere()
{
	vertex_layout vertex_declare;

	vertex_declare.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

	opengl_bind_vertex_layout(vertex_declare, deferred_light_sphere_vbo, deferred_light_sphere_ibo);

	glDrawRangeElements(GL_TRIANGLES, 0, deferred_light_sphere_vcount, deferred_light_sphere_icount, GL_UNSIGNED_SHORT, 0);
}

void gr_opengl_draw_deferred_light_cylinder(const vec3d *position, const matrix *orient)
{
	g3_start_instance_matrix(position, orient, true);

	Current_shader->program->Uniforms.setUniformMatrix4f("modelViewMatrix", gr_model_view_matrix);
	Current_shader->program->Uniforms.setUniformMatrix4f("projMatrix", gr_projection_matrix);

	vertex_layout vertex_declare;

	vertex_declare.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

	opengl_bind_vertex_layout(vertex_declare, deferred_light_cylinder_vbo, deferred_light_cylinder_ibo);

	glDrawRangeElements(GL_TRIANGLES, 0, deferred_light_cylinder_vcount, deferred_light_cylinder_icount, GL_UNSIGNED_SHORT, 0);

	g3_done_instance(true);
}

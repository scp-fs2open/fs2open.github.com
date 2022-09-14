

#include "gropengldeferred.h"

#include "ShaderProgram.h"
#include "gropengldraw.h"
#include "gropenglstate.h"
#include "gropengltnl.h"

#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/util/UniformAligner.h"
#include "graphics/util/UniformBuffer.h"
#include "graphics/util/uniform_structs.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "mission/mission_flags.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "render/3d.h"
#include "tracing/tracing.h"

#include <math/bitarray.h>

void gr_opengl_deferred_init()
{
	gr_opengl_deferred_light_cylinder_init(16);
	gr_opengl_deferred_light_sphere_init(16, 16);
}
void gr_opengl_deferred_shutdown() {}

void opengl_clear_deferred_buffers()
{
	GR_DEBUG_SCOPE("Clear deferred buffers");

	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	GL_state.ColorMask(true, true, true, true);

	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_CLEAR, 0) );

	opengl_draw_full_screen_textured(0.0f, 0.0f, 1.0f, 1.0f);

	opengl_shader_set_current();

	GL_state.ColorMask(true, true, true, false);

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
	GL_state.ColorMask(true, true, true, true);

	// Copy the existing color data into the emissive part of the G-buffer since everything that already existed is
	// treated as emissive
	glDrawBuffer(GL_COLOR_ATTACHMENT4);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, gr_screen.max_w, gr_screen.max_h, 0, 0, gr_screen.max_w, gr_screen.max_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, buffers);

	static const float black[] = { 0, 0, 0, 1.0f };
	glClearBufferfv(GL_COLOR, 0, black);
}

void gr_opengl_deferred_lighting_end()
{
	if(!Deferred_lighting)
		return;

	GR_DEBUG_SCOPE("Deferred lighting end");

	Deferred_lighting = false;
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	GL_state.ColorMask(true, true, true, false);
}

extern SCP_vector<light> Lights;
extern int Num_lights;

void gr_opengl_deferred_lighting_finish()
{
	GR_DEBUG_SCOPE("Deferred lighting finish");
	TRACE_SCOPE(tracing::ApplyLights);

	if (Cmdline_no_deferred_lighting) {
		return;
	}

	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);
	gr_zbuffer_set(GR_ZBUFF_NONE);

	//GL_state.DepthFunc(GL_GREATER);
	//GL_state.DepthMask(GL_FALSE);

	opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_LIGHTING, 0));

	// Render on top of the emissive buffer texture
	glDrawBuffer(GL_COLOR_ATTACHMENT4);

	GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_color_texture);
	GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_normal_texture);
	GL_state.Texture.Enable(2, GL_TEXTURE_2D, Scene_position_texture);
	GL_state.Texture.Enable(3, GL_TEXTURE_2D, Scene_specular_texture);
	if (Shadow_quality != ShadowQuality::Disabled) {
		GL_state.Texture.Enable(4, GL_TEXTURE_2D_ARRAY, Shadow_map_texture);
	}
	
	// We need to use stable sorting here to make sure that the relative ordering of the same light types is the same as
	// the rest of the code. Otherwise the shadow mapping would be applied while rendering the wrong light which would
	// lead to flickering lights in some circumstances
	std::stable_sort(Lights.begin(), Lights.end(), light_compare_by_type);
	using namespace graphics;

	// We need to precompute how many elements we are going to need
	size_t num_data_elements = Lights.size();

	// Get a uniform buffer for our data
	auto buffer          = gr_get_uniform_buffer(uniform_block_type::Lights, num_data_elements);
	auto& uniformAligner = buffer.aligner();

	{
		GR_DEBUG_SCOPE("Build buffer data");

		auto header = uniformAligner.getHeader<deferred_global_data>();
		if (Shadow_quality != ShadowQuality::Disabled) {
			// Avoid this overhead when we are not going to use these values
			header->shadow_mv_matrix = Shadow_view_matrix;
			for (size_t i = 0; i < MAX_SHADOW_CASCADES; ++i) {
				header->shadow_proj_matrix[i] = Shadow_proj_matrix[i];
			}
			header->veryneardist = Shadow_cascade_distances[0];
			header->neardist = Shadow_cascade_distances[1];
			header->middist = Shadow_cascade_distances[2];
			header->fardist = Shadow_cascade_distances[3];

			vm_inverse_matrix4(&header->inv_view_matrix, &gr_view_matrix);
		}

		header->invScreenWidth = 1.0f / gr_screen.max_w;
		header->invScreenHeight = 1.0f / gr_screen.max_h;

		// Only the first directional light uses shaders so we need to know when we already saw that light
		bool first_directional = true;
		for (auto& l : Lights) {
			auto light_data = uniformAligner.addTypedElement<deferred_light_data>();

			light_data->lightType = static_cast<int>(l.type);

			vec3d diffuse;
			diffuse.xyz.x = l.r * l.intensity;
			diffuse.xyz.y = l.g * l.intensity;
			diffuse.xyz.z = l.b * l.intensity;

			light_data->diffuseLightColor = diffuse;

			// Set a default value for all lights. Only the first directional light will change this.
			light_data->enable_shadows = false;

			switch (l.type) {
			case Light_Type::Directional:
				if (Shadow_quality != ShadowQuality::Disabled) {
					light_data->enable_shadows = first_directional ? 1 : 0;
				}
				vec4 light_dir;
				light_dir.xyzw.x = -l.vec.xyz.x;
				light_dir.xyzw.y = -l.vec.xyz.y;
				light_dir.xyzw.z = -l.vec.xyz.z;
				light_dir.xyzw.w = 0.0f;
				vec4 view_dir;

				vm_vec_transform(&view_dir, &light_dir, &gr_view_matrix);

				light_data->lightDir.xyz.x = view_dir.xyzw.x;
				light_data->lightDir.xyz.y = view_dir.xyzw.y;
				light_data->lightDir.xyz.z = view_dir.xyzw.z;

				first_directional = false;
				break;
			case Light_Type::Cone:
				light_data->dualCone = l.dual_cone ? 1.0f : 0.0f;
				light_data->coneAngle = l.cone_angle;
				light_data->coneInnerAngle = l.cone_inner_angle;
				light_data->coneDir = l.vec2;
				FALLTHROUGH;
			case Light_Type::Point:
				light_data->lightRadius = MAX(l.rada, l.radb);
				//A small padding factor is added to guard against potentially clipping the edges of the light with facets of the volume mesh.
				light_data->scale.xyz.x = MAX(l.rada, l.radb) * 1.05f;
				light_data->scale.xyz.y = MAX(l.rada, l.radb) * 1.05f;
				light_data->scale.xyz.z = MAX(l.rada, l.radb) * 1.05f;
				break;
			case Light_Type::Tube: {
				light_data->lightRadius = l.radb;
				light_data->lightType = LT_TUBE;

				vec3d a;
				vm_vec_sub(&a, &l.vec, &l.vec2);
				auto length = vm_vec_mag(&a);
				//Tube light volumes must be extended past the length of their requested light vector
				//to allow smooth fall-off from all angles. Since the light volume starts at the mesh
				//origin we must extend it here. Later the position will be adjusted as well.
				length += light_data->lightRadius * 2.0f;

				//A small padding factor is added to guard against potentially clipping the edges of the light with facets of the volume mesh.
				light_data->scale.xyz.x = l.radb * 1.05f;
				light_data->scale.xyz.y = l.radb * 1.05f;
				light_data->scale.xyz.z = length;

				break;
			}
			}
		}

		// Uniform data has been assembled, upload it to the GPU and issue the draw calls
		buffer.submitData();
	}
	{
		GR_DEBUG_SCOPE("Render light geometry");
		gr_bind_uniform_buffer(uniform_block_type::DeferredGlobals, buffer.getBufferOffset(0),
		                       sizeof(graphics::deferred_global_data), buffer.bufferHandle());

		size_t element_index = 0;
		for (auto& l : Lights) {
			GR_DEBUG_SCOPE("Deferred apply single light");

			switch (l.type) {
			case Light_Type::Directional:
				gr_bind_uniform_buffer(uniform_block_type::Lights, buffer.getAlignerElementOffset(element_index),
				                       sizeof(graphics::deferred_light_data), buffer.bufferHandle());
				opengl_draw_full_screen_textured(0.0f, 0.0f, 1.0f, 1.0f);
				++element_index;
				break;
			case Light_Type::Cone:
			case Light_Type::Point:
				gr_bind_uniform_buffer(uniform_block_type::Lights, buffer.getAlignerElementOffset(element_index),
				                       sizeof(graphics::deferred_light_data), buffer.bufferHandle());
				gr_opengl_draw_deferred_light_sphere(&l.vec);
				++element_index;
				break;
			case Light_Type::Tube:
				gr_bind_uniform_buffer(uniform_block_type::Lights, buffer.getAlignerElementOffset(element_index),
				                       sizeof(graphics::deferred_light_data), buffer.bufferHandle());

				vec3d dir, newPos;
				matrix orient;
				vm_vec_sub(&dir, &l.vec, &l.vec2);
				vm_vector_2_matrix(&orient, &dir, nullptr, nullptr);
				//Tube light volumes must be extended past the length of their requested light vector
				//to allow smooth fall-off from all angles. Since the light volume starts at the mesh
				//origin we must extend it, which has been done above, and then move it backwards one radius.
				vm_vec_normalize(&dir);
				vm_vec_scale_sub(&newPos, &l.vec2, &dir, l.radb);
				gr_opengl_draw_deferred_light_cylinder(&newPos, &orient);
				++element_index;
				break;
			default:
				continue;
			}
		}
	}

	gr_end_view_matrix();
	gr_end_proj_matrix();

	// Now reset back to drawing into the color buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (The_mission.flags[Mission::Mission_Flags::Fullneb] && Neb2_render_mode != NEB2_RENDER_NONE) {
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		gr_zbuffer_set(GR_ZBUFF_NONE);
		opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_SCENE_FOG, 0));

		GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_emissive_texture);
		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_depth_texture);

		float fog_near, fog_far, fog_density;
		neb2_get_adjusted_fog_values(&fog_near, &fog_far, &fog_density);
		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);

		Current_shader->program->Uniforms.setTextureUniform("tex", 0);
		Current_shader->program->Uniforms.setTextureUniform("depth_tex", 1);

		opengl_set_generic_uniform_data<graphics::generic_data::fog_data>([&](graphics::generic_data::fog_data* data) {
			data->fog_start       = fog_near;
			data->fog_density     = fog_density;
			data->fog_color.xyz.x = r / 255.f;
			data->fog_color.xyz.y = g / 255.f;
			data->fog_color.xyz.z = b / 255.f;
			data->zNear           = Min_draw_distance;
			data->zFar            = Max_draw_distance;
		});

		opengl_draw_full_screen_textured(0.0f, 0.0f, 1.0f, 1.0f);
	} else {
		// Transfer the resolved lighting back to the color texture
		// TODO: Maybe this could be improved so that it doesn't require the copy back operation?
		glReadBuffer(GL_COLOR_ATTACHMENT4);
		glBlitFramebuffer(0,
						  0,
						  gr_screen.max_w,
						  gr_screen.max_h,
						  0,
						  0,
						  gr_screen.max_w,
						  gr_screen.max_h,
						  GL_COLOR_BUFFER_BIT,
						  GL_NEAREST);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}

	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	// reset state
	gr_clear_states();
}


void gr_opengl_draw_deferred_light_sphere(const vec3d *position)
{
	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	gr_matrix_set_uniforms();

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

			vm_free(Indices);
			Indices = nullptr;
			vm_free(Vertices);
			Vertices = nullptr;
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
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

			vm_free(Indices);
			Indices = nullptr;
			vm_free(Vertices);
			Vertices = nullptr;
			return;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	vm_free(Indices);
	Indices = nullptr;
	vm_free(Vertices);
	Vertices = nullptr;
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
			
			vm_free(Vertices);
			Vertices = nullptr;
			vm_free(Indices);
			Indices = nullptr;
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
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
			
			vm_free(Vertices);
			Vertices = nullptr;
			vm_free(Indices);
			Indices = nullptr;
			return;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	vm_free(Vertices);
	Vertices = nullptr;
	vm_free(Indices);
	Indices = nullptr;
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

	gr_matrix_set_uniforms();

	vertex_layout vertex_declare;

	vertex_declare.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

	opengl_bind_vertex_layout(vertex_declare, deferred_light_cylinder_vbo, deferred_light_cylinder_ibo);

	glDrawRangeElements(GL_TRIANGLES, 0, deferred_light_cylinder_vcount, deferred_light_cylinder_icount, GL_UNSIGNED_SHORT, 0);

	g3_done_instance(true);
}

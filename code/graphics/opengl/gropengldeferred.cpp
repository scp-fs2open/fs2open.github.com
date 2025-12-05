

#include "gropengldeferred.h"
#include "globalincs/vmallocator.h"

#include "ShaderProgram.h"
#include "gropengldraw.h"
#include "gropenglstate.h"
#include "gropengltnl.h"

#include "graphics/2d.h"
#include "graphics/light.h"
#include "graphics/matrix.h"
#include "graphics/util/UniformAligner.h"
#include "graphics/util/UniformBuffer.h"
#include "graphics/util/uniform_structs.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "mission/mission_flags.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "render/3d.h"
#include "tracing/tracing.h"
#ifdef USE_OPENGL_ES
#include "es_compatibility.h"
#endif

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

void gr_opengl_deferred_lighting_begin(bool clearNonColorBufs)
{
	if (!light_deferred_enabled())
		return;

	static const float black[] = {0, 0, 0, 1.0f};

	GR_DEBUG_SCOPE("Deferred lighting begin");

	Deferred_lighting = true;
	GL_state.ColorMask(true, true, true, true);
	
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };

	if (Cmdline_msaa_enabled > 0) {
		//Ensure MSAA Mode if necessary
		GL_state.BindFrameBuffer(Scene_framebuffer_ms);
		glDrawBuffer(GL_COLOR_ATTACHMENT4);

		opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_COPY, 0));
		GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_color_texture);
		Current_shader->program->Uniforms.setTextureUniform("tex", 0);
		GL_state.SetAlphaBlendMode(gr_alpha_blend::ALPHA_BLEND_NONE);
		GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);
		opengl_draw_full_screen_textured(0, 0, 1, 1);
	} else {
		// Copy the existing color data into the emissive part of the G-buffer since everything that already existed is
		// treated as emissive
		#ifndef USE_OPENGL_ES
		glDrawBuffer(GL_COLOR_ATTACHMENT4);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, gr_screen.max_w, gr_screen.max_h, 0, 0, gr_screen.max_w, gr_screen.max_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		#else
		// one again ES does not consider GL_COLOR_ATTACHMENT4 as valid draw buffer
		GLint prev_read_fbo = 0, prev_tex2d = 0;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_read_fbo);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex2d);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, Scene_framebuffer);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindTexture(GL_TEXTURE_2D, Scene_emissive_texture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, gr_screen.max_w, gr_screen.max_h);
		glBindTexture(GL_TEXTURE_2D, prev_tex2d);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, prev_read_fbo);
		#endif
	}
	
	glDrawBuffers(6, buffers);
	glClearBufferfv(GL_COLOR, 0, black);
	if (clearNonColorBufs) {
		glClearBufferfv(GL_COLOR, 1, black);
		glClearBufferfv(GL_COLOR, 2, black);
		glClearBufferfv(GL_COLOR, 3, black);
		glClearBufferfv(GL_COLOR, 5, black);
	}
}

void gr_opengl_deferred_lighting_msaa()
{
	if (!Deferred_lighting)
		return;

	if (Cmdline_msaa_enabled <= 0)
		return;
	
	GR_DEBUG_SCOPE("MSAA Pass");
	GL_state.BindFrameBuffer(Scene_framebuffer);

	GLenum buffers[] = {GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4};
	glDrawBuffers(5, buffers);

	int msaa_resolve_flags = 0;
	switch (Cmdline_msaa_enabled) {
	case 4:
		msaa_resolve_flags = SDR_FLAG_MSAA_SAMPLES_4;
		break;
	case 8:
		msaa_resolve_flags = SDR_FLAG_MSAA_SAMPLES_8;
		break;
	case 16:
		msaa_resolve_flags = SDR_FLAG_MSAA_SAMPLES_16;
		break;
	default:
		UNREACHABLE("Disallowed MSAA shader sample count!");
		break;
	}

	opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_MSAA_RESOLVE, msaa_resolve_flags));
	GL_state.Texture.Enable(0, GL_TEXTURE_2D_MULTISAMPLE, Scene_color_texture_ms);
	GL_state.Texture.Enable(1, GL_TEXTURE_2D_MULTISAMPLE, Scene_position_texture_ms);
	GL_state.Texture.Enable(2, GL_TEXTURE_2D_MULTISAMPLE, Scene_normal_texture_ms);
	GL_state.Texture.Enable(3, GL_TEXTURE_2D_MULTISAMPLE, Scene_specular_texture_ms);
	GL_state.Texture.Enable(4, GL_TEXTURE_2D_MULTISAMPLE, Scene_emissive_texture_ms);
	GL_state.Texture.Enable(5, GL_TEXTURE_2D_MULTISAMPLE, Scene_depth_texture_ms);
	Current_shader->program->Uniforms.setTextureUniform("texColor", 0);
	Current_shader->program->Uniforms.setTextureUniform("texPos", 1);
	Current_shader->program->Uniforms.setTextureUniform("texNormal", 2);
	Current_shader->program->Uniforms.setTextureUniform("texSpecular", 3);
	Current_shader->program->Uniforms.setTextureUniform("texEmissive", 4);
	Current_shader->program->Uniforms.setTextureUniform("texDepth", 5);
	opengl_set_generic_uniform_data<graphics::generic_data::msaa_data>(
		[&](graphics::generic_data::msaa_data* data) {
			data->samples = Cmdline_msaa_enabled;
			data->fov = g3_get_hfov(Proj_fov);
		});
	GL_state.SetAlphaBlendMode(gr_alpha_blend::ALPHA_BLEND_NONE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_WRITE);
	opengl_draw_full_screen_textured(0, 0, 1, 1);
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

static GLuint deferred_light_cylinder_vbo = 0;
static GLuint deferred_light_cylinder_ibo = 0;
static GLushort deferred_light_cylinder_vcount = 0;
static GLuint deferred_light_cylinder_icount = 0;

static GLuint deferred_light_sphere_vbo = 0;
static GLuint deferred_light_sphere_ibo = 0;
static GLushort deferred_light_sphere_vcount = 0;
static GLuint deferred_light_sphere_icount = 0;

extern SCP_vector<light> Lights;
extern int Num_lights;
namespace ltp = lighting_profiles;
using namespace ltp; 
static bool override_fog = false;
graphics::deferred_light_data*

// common conversion operations to translate a game light data structure into a render-ready light uniform.
prepare_light_uniforms(light& l, graphics::util::UniformAligner& uniformAligner, const ltp::profile* lp)
{
	graphics::deferred_light_data* light_data = uniformAligner.addTypedElement<graphics::deferred_light_data>();

	light_data->lightType = static_cast<int>(l.type);

	float intensity =
		(Lighting_mode == lighting_mode::COCKPIT) ? lp->cockpit_light_intensity_modifier.handle(l.intensity) : l.intensity;

	vec3d diffuse;
	diffuse.xyz.x = l.r * intensity;
	diffuse.xyz.y = l.g * intensity;
	diffuse.xyz.z = l.b * intensity;

	light_data->diffuseLightColor = diffuse;

	// Set a default value for all lights. Only the first directional light will change this.
	light_data->enable_shadows = false;
	light_data->sourceRadius = l.source_radius;
	return light_data;
}

void gr_opengl_deferred_lighting_finish()
{
	GR_DEBUG_SCOPE("Deferred lighting finish");
	TRACE_SCOPE(tracing::ApplyLights);

	if (!light_deferred_enabled()) {
		return;
	}

	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ADDITIVE);
	gr_zbuffer_set(GR_ZBUFF_NONE);

	// GL_state.DepthFunc(GL_GREATER);
	// GL_state.DepthMask(GL_FALSE);

	opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_LIGHTING, ENVMAP > 0 ? SDR_FLAG_ENV_MAP : 0));
	
	#ifndef USE_OPENGL_ES
	glDrawBuffer(GL_COLOR_ATTACHMENT5);
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
	#else
	//Another case of ES not considering GL_COLOR_ATTACHMENT5 as valid draw buffer
	GLint prev_read_fbo = 0, prev_tex2d = 0;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_read_fbo);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex2d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, Scene_framebuffer);
	glReadBuffer(GL_COLOR_ATTACHMENT4);
	glBindTexture(GL_TEXTURE_2D, Scene_composite_texture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, gr_screen.max_w, gr_screen.max_h);
	glBindTexture(GL_TEXTURE_2D, prev_tex2d);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, prev_read_fbo);
	#endif

	GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_color_texture);
	GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_normal_texture);
	GL_state.Texture.Enable(2, GL_TEXTURE_2D, Scene_position_texture);
	GL_state.Texture.Enable(3, GL_TEXTURE_2D, Scene_specular_texture);
	if (Shadow_quality != ShadowQuality::Disabled) {
		GL_state.Texture.Enable(4, GL_TEXTURE_2D_ARRAY, Shadow_map_texture);
	}

	if (ENVMAP > 0) {
		Current_shader->program->Uniforms.setTextureUniform("sEnvmap", 5);
		Current_shader->program->Uniforms.setTextureUniform("sIrrmap", 6);
		float u_scale, v_scale;
		uint32_t array_index;
		gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, &array_index, 5);
		gr_opengl_tcache_set(IRRMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, &array_index, 6);
		Assertion(array_index == 0, "Cube map arrays are not supported yet!");
	}

	// We need to use stable sorting here to make sure that the relative ordering of the same light types is the same as
	// the rest of the code. Otherwise the shadow mapping would be applied while rendering the wrong light which would
	// lead to flickering lights in some circumstances
	std::stable_sort(Lights.begin(), Lights.end(), light_compare_by_type);
	using namespace graphics;

	// We need to precompute how many elements we are going to need
	size_t num_data_elements = Lights.size() + 1;

	// Get a uniform buffer for our data
	auto light_buffer = gr_get_uniform_buffer(uniform_block_type::Lights, num_data_elements);
	auto& light_uniform_aligner = light_buffer.aligner();
	auto matrix_buffer = gr_get_uniform_buffer(uniform_block_type::Matrices, num_data_elements);
	auto& matrix_uniform_aligner = matrix_buffer.aligner();

	// This is the light which is responsible for shadows and volumetric nebula lighting
	const light* global_light = nullptr;
	vec3d global_light_diffuse;

	// To allow reduced bind calls, we sort lights into subsets based on rendering methods.
	// It might seem optimal to create these subsets as lights are added, or some other method,
	// but this keeps the graphics implementation methods better contained and profiling currently
	// (dec 2023) shows negligable cost of doing it this way.
	SCP_vector<light> full_frame_lights = SCP_vector<light>();
	SCP_vector<light> sphere_lights = SCP_vector<light>();
	SCP_vector<light> cylinder_lights = SCP_vector<light>();
	for (auto& l : Lights) {
		switch (l.type) {
		case Light_Type::Directional:
			full_frame_lights.push_back(l);
			break;
		case Light_Type::Cone:
			FALLTHROUGH;
		case Light_Type::Point:
			sphere_lights.push_back(l);
			break;
		case Light_Type::Tube:
			cylinder_lights.push_back(l);
			break;
		case Light_Type::Ambient:
			UNREACHABLE("Multiple ambient lights are not supported!");
		}
	}
	{
		GR_DEBUG_SCOPE("Build buffer data");

		auto lp = ltp::current();

		auto header = light_uniform_aligner.getHeader<deferred_global_data>();
		if (Shadow_quality != ShadowQuality::Disabled) {
			// Avoid this overhead when we are not going to use these values
			header->shadow_mv_matrix = Shadow_view_matrix_light;
			for (size_t i = 0; i < MAX_SHADOW_CASCADES; ++i) {
				header->shadow_proj_matrix[i] = Shadow_proj_matrix[i];
			}
			header->veryneardist = Shadow_cascade_distances[0];
			header->neardist = Shadow_cascade_distances[1];
			header->middist = Shadow_cascade_distances[2];
			header->fardist = Shadow_cascade_distances[3];

			vm_inverse_matrix4(&header->inv_view_matrix, &Shadow_view_matrix_render);
		}

		header->invScreenWidth = 1.0f / gr_screen.max_w;
		header->invScreenHeight = 1.0f / gr_screen.max_h;
		header->nearPlane = gr_near_plane;

		{
			//Prepare ambient light
			light& l = full_frame_lights.emplace_back();
			vec3d ambient;
			gr_get_ambient_light(&ambient);
			l.r = ambient.xyz.x;
			l.g = ambient.xyz.y;
			l.b = ambient.xyz.z;
			l.type = Light_Type::Ambient;
			l.intensity = 1.f;
			l.source_radius = 0.f;
		}

		// Only the first directional light uses shaders so we need to know when we already saw that light
		bool first_directional = true;

		for (auto& l : full_frame_lights) {
			auto light_data = prepare_light_uniforms(l, light_uniform_aligner, lp);

			if (l.type == Light_Type::Directional ) {
				if (Shadow_quality != ShadowQuality::Disabled) {
					light_data->enable_shadows = first_directional ? 1 : 0;
				}

				// Global light direction should match shadow light direction
				if (first_directional) {
					global_light = &l;
					global_light_diffuse = light_data->diffuseLightColor;

					first_directional = false;
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
			}
		}
		for (auto& l : sphere_lights) {
			auto light_data = prepare_light_uniforms(l, light_uniform_aligner, lp);

			if (l.type == Light_Type::Cone) {
				light_data->dualCone = (l.flags & LF_DUAL_CONE) ? 1.0f : 0.0f;
				light_data->coneAngle = l.cone_angle;
				light_data->coneInnerAngle = l.cone_inner_angle;
				light_data->coneDir = l.vec2;
			}
			float rad = (Lighting_mode == lighting_mode::COCKPIT)
							? lp->cockpit_light_radius_modifier.handle(MAX(l.rada, l.radb))
							: MAX(l.rada, l.radb);
			light_data->lightRadius = rad;

			// A small padding factor is added to guard against potentially clipping the edges of the light with facets
			// of the volume mesh.
			light_data->scale.xyz.x = rad * 1.05f;
			light_data->scale.xyz.y = rad * 1.05f;
			light_data->scale.xyz.z = rad * 1.05f;
		}
		for (auto& l : cylinder_lights) {
			auto light_data = prepare_light_uniforms(l, light_uniform_aligner, lp);
			float rad =
				(Lighting_mode == lighting_mode::COCKPIT) ? lp->cockpit_light_radius_modifier.handle(l.radb) : l.radb;

			light_data->lightRadius = rad;

			light_data->lightType = LT_TUBE;

			vec3d a;
			vm_vec_sub(&a, &l.vec, &l.vec2);
			auto length = vm_vec_mag(&a);
			// Tube light volumes must be extended past the length of their requested light vector
			// to allow smooth fall-off from all angles. Since the light volume starts at the mesh
			// origin we must extend it here. Later the position will be adjusted as well.
			length += light_data->lightRadius * 2.0f;

			// A small padding factor is added to guard against potentially clipping the edges of the light with facets
			// of the volume mesh.
			light_data->scale.xyz.x = rad * 1.05f;
			light_data->scale.xyz.y = rad * 1.05f;
			light_data->scale.xyz.z = length;
		}
		// Uniform data has been assembled, upload it to the GPU and issue the draw calls
		light_buffer.submitData();
	}
	{
		for (size_t i = 0; i<full_frame_lights.size(); i++) {
			// just keeping things aligned really.
			auto matrix_data = matrix_uniform_aligner.addTypedElement<graphics::matrix_uniforms>();
			matrix_data->modelViewMatrix = gr_env_texture_matrix;
		}
		for (auto& l : sphere_lights) {
			auto matrix_data = matrix_uniform_aligner.addTypedElement<graphics::matrix_uniforms>();
			g3_start_instance_matrix(&l.vec, &vmd_identity_matrix, true);
			matrix_data->modelViewMatrix = gr_model_view_matrix;
			matrix_data->projMatrix = gr_projection_matrix;
			g3_done_instance(true);
		}
		for (auto& l : cylinder_lights ) {
			auto matrix_data = matrix_uniform_aligner.addTypedElement<graphics::matrix_uniforms>();
			vec3d dir, newPos;
			matrix orient;
			vm_vec_normalized_dir(&dir, &l.vec, &l.vec2);
			vm_vector_2_matrix_norm(&orient, &dir, nullptr, nullptr);
			// Tube light volumes must be extended past the length of their requested light vector
			// to allow smooth fall-off from all angles. Since the light volume starts at the mesh
			// origin we must extend it, which has been done above, and then move it backwards one radius.
			vm_vec_scale_sub(&newPos, &l.vec2, &dir, l.radb);

			g3_start_instance_matrix(&newPos, &orient, true);
			matrix_data->modelViewMatrix = gr_model_view_matrix;
			matrix_data->projMatrix = gr_projection_matrix;
			g3_done_instance(true);
		}
		matrix_buffer.submitData();
	}
	{
		GR_DEBUG_SCOPE("Render light geometry");
		gr_bind_uniform_buffer(uniform_block_type::DeferredGlobals,
			light_buffer.getBufferOffset(0),
			sizeof(graphics::deferred_global_data),
			light_buffer.bufferHandle());
		gr_bind_uniform_buffer(uniform_block_type::Matrices,
			matrix_buffer.getBufferOffset(0),
			sizeof(graphics::matrix_uniforms),
			matrix_buffer.bufferHandle());

		size_t element_index = 0;
		vertex_layout vertex_declare;
		vertex_declare.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

		for (size_t i = 0; i<full_frame_lights.size(); i++) {
			GR_DEBUG_SCOPE("Deferred apply single dir light");

			gr_bind_uniform_buffer(uniform_block_type::Lights,
				light_buffer.getAlignerElementOffset(element_index),
				sizeof(graphics::deferred_light_data),
				light_buffer.bufferHandle());
			opengl_draw_full_screen_textured(0.0f, 0.0f, 1.0f, 1.0f);
			++element_index;
		}
		if (!sphere_lights.empty()) {
			opengl_bind_vertex_layout(vertex_declare, deferred_light_sphere_vbo, deferred_light_sphere_ibo);
		}
		for (size_t i = 0; i<sphere_lights.size(); i++) {

			gr_bind_uniform_buffer(uniform_block_type::Lights,
				light_buffer.getAlignerElementOffset(element_index),
				sizeof(graphics::deferred_light_data),
				light_buffer.bufferHandle());
			gr_bind_uniform_buffer(uniform_block_type::Matrices,
				matrix_buffer.getAlignerElementOffset(element_index),
				sizeof(graphics::matrix_uniforms),
				matrix_buffer.bufferHandle());

			glDrawRangeElements(GL_TRIANGLES,
				0,
				deferred_light_sphere_vcount,
				deferred_light_sphere_icount,
				GL_UNSIGNED_SHORT,
				0);
			opengl_draw_sphere();
			++element_index;
		}
		if (!cylinder_lights.empty()) {
			opengl_bind_vertex_layout(vertex_declare, deferred_light_cylinder_vbo, deferred_light_cylinder_ibo);
		}
		for (size_t i = 0; i<cylinder_lights.size(); i++) {
			gr_bind_uniform_buffer(uniform_block_type::Lights,
				light_buffer.getAlignerElementOffset(element_index),
				sizeof(graphics::deferred_light_data),
				light_buffer.bufferHandle());
			gr_bind_uniform_buffer(uniform_block_type::Matrices,
				matrix_buffer.getAlignerElementOffset(element_index),
				sizeof(graphics::matrix_uniforms),
				matrix_buffer.bufferHandle());

			glDrawRangeElements(GL_TRIANGLES,
				0,
				deferred_light_cylinder_vcount,
				deferred_light_cylinder_icount,
				GL_UNSIGNED_SHORT,
				0);

			++element_index;
		}
	}

	gr_end_view_matrix();
	gr_end_proj_matrix();

	// Now reset back to drawing into the color buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	bool bDrawFullNeb = The_mission.flags[Mission::Mission_Flags::Fullneb] && Neb2_render_mode != NEB2_RENDER_NONE && !override_fog;
	bool bDrawNebVolumetrics = The_mission.volumetrics && The_mission.volumetrics->get_enabled() && !override_fog;

	if (bDrawFullNeb) {
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		gr_zbuffer_set(GR_ZBUFF_NONE);
		opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_SCENE_FOG, 0));

		GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_composite_texture);
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

		if (bDrawNebVolumetrics) {
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT5);
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
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
		}
		
	} 
	if (bDrawNebVolumetrics) {
		GR_DEBUG_SCOPE("Volumetric Nebulae");
		TRACE_SCOPE(tracing::Volumetrics);

		const volumetric_nebula& neb = *The_mission.volumetrics;

		Assertion(neb.isVolumeBitmapValid(), "The volumetric nebula was not properly initialized!");

		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		gr_zbuffer_set(GR_ZBUFF_NONE);
		opengl_shader_set_current(gr_opengl_maybe_create_shader(SDR_TYPE_VOLUMETRIC_FOG,
			(neb.getEdgeSmoothing() ? SDR_FLAG_VOLUMETRICS_DO_EDGE_SMOOTHING : 0) |
			(neb.getNoiseActive() ? SDR_FLAG_VOLUMETRICS_NOISE : 0)
		));

		GL_state.Texture.Enable(0, GL_TEXTURE_2D, Scene_composite_texture);
		GL_state.Texture.Enable(1, GL_TEXTURE_2D, Scene_emissive_texture);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		GL_state.Texture.Enable(2, GL_TEXTURE_2D, Scene_depth_texture);
		
		{
			//The following are not required, but the graphics API still returns them
			float u_scale, v_scale;
			uint32_t array_index;
			gr_set_texture_addressing(TMAP_ADDRESS_CLAMP);
			gr_opengl_tcache_set(neb.getVolumeBitmapHandle(), TCACHE_TYPE_3DTEX, &u_scale, &v_scale, &array_index, 3);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if (neb.getNoiseActive()) {
				gr_set_texture_addressing(TMAP_ADDRESS_WRAP);
				gr_opengl_tcache_set(neb.getNoiseVolumeBitmapHandle(), TCACHE_TYPE_3DTEX, &u_scale, &v_scale, &array_index, 4);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		}

		opengl_set_generic_uniform_data<graphics::generic_data::volumetric_fog_data>([&](graphics::generic_data::volumetric_fog_data* data) {
			vm_inverse_matrix4(&data->p_inv, &gr_projection_matrix);
			vm_inverse_matrix4(&data->v_inv, &gr_view_matrix);
			data->zNear = Min_draw_distance;
			data->zFar = Max_draw_distance;
			data->cameraPos = Eye_position;
			data->globalLightDirection = global_light ? global_light->vec : vec3d(ZERO_VECTOR);
			data->globalLightDiffuse = global_light_diffuse;
			data->nebPos = neb.getPos();
			data->nebSize = neb.getSize();
			data->stepsize = neb.getStepsize();
			data->opacitydistance = neb.getOpacityDistance();
			data->alphalimit = neb.getAlphaLim();
			data->nebColor[0] = std::get<0>(neb.getNebulaColor());
			data->nebColor[1] = std::get<1>(neb.getNebulaColor());
			data->nebColor[2] = std::get<2>(neb.getNebulaColor());
			data->udfScale = neb.getUDFScale();
			data->emissiveSpreadFactor = neb.getEmissiveSpread();
			data->emissiveIntensity = neb.getEmissiveIntensity();
			data->emissiveFalloff = neb.getEmissiveFalloff();
			data->henyeyGreensteinCoeff = neb.getHenyeyGreensteinCoeff();
			data->directionalLightSampleSteps = neb.getGlobalLightSteps();
			data->directionalLightStepSize = neb.getGlobalLightStepsize();
			data->noiseColor[0] = std::get<0>(neb.getNoiseColor());
			data->noiseColor[1] = std::get<1>(neb.getNoiseColor());
			data->noiseColor[2] = std::get<2>(neb.getNoiseColor());
			data->noiseColorScale1 = std::get<0>(neb.getNoiseColorScale());
			data->noiseColorScale2 = std::get<1>(neb.getNoiseColorScale());
			data->noiseColorIntensity = neb.getNoiseColorIntensity();
			data->aspect = gr_screen.clip_aspect;
			data->fov = g3_get_hfov(Proj_fov);
			});

		{
			GR_DEBUG_SCOPE("Volumetric Nebulae Draw");
			opengl_draw_full_screen_textured(0.0f, 0.0f, 1.0f, 1.0f);
		}
		GL_state.Texture.Enable(Scene_emissive_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	if(!bDrawFullNeb && !bDrawNebVolumetrics) {
		// Transfer the resolved lighting back to the color texture
		// TODO: Maybe this could be improved so that it doesn't require the copy back operation?
		glReadBuffer(GL_COLOR_ATTACHMENT5);
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

void gr_opengl_override_fog(bool set_override)
{
	override_fog = set_override;
}

void gr_opengl_draw_deferred_light_sphere(const vec3d *position)
{
	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	gr_matrix_set_uniforms();

	opengl_draw_sphere();

	g3_done_instance(true);
}


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

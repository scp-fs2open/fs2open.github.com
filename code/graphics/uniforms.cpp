//
//

#include "uniforms.h"
#include "matrix.h"
#include "light.h"
#include "globalincs/systemvars.h"
#include "shadows.h"
#include "def_files/data/effects/model_shader_flags.h"

namespace {
void scale_matrix(matrix4& mat, const vec3d& scale) {
	mat.a2d[0][0] *= scale.xyz.x;
	mat.a2d[0][1] *= scale.xyz.x;
	mat.a2d[0][2] *= scale.xyz.x;

	mat.a2d[1][0] *= scale.xyz.y;
	mat.a2d[1][1] *= scale.xyz.y;
	mat.a2d[1][2] *= scale.xyz.y;

	mat.a2d[2][0] *= scale.xyz.z;
	mat.a2d[2][1] *= scale.xyz.z;
	mat.a2d[2][2] *= scale.xyz.z;
}
}

namespace graphics {
namespace uniforms {

void convert_model_material(model_uniform_data* data_out,
							const model_material& material,
							const matrix4& model_transform,
							const vec3d& scale,
							size_t transform_buffer_offset) {
	auto shader_flags = material.get_shader_flags();

	Assertion(gr_model_matrix_stack.depth() == 1, "Uniform conversion does not respect previous transforms! "
		"Model matrix stack must be empty!");

	matrix4 scaled_matrix = model_transform;
	scale_matrix(scaled_matrix, scale);

	data_out->modelMatrix = scaled_matrix;
	data_out->viewMatrix = gr_view_matrix;
	vm_matrix4_x_matrix4(&data_out->modelViewMatrix, &gr_view_matrix, &scaled_matrix);
	data_out->projMatrix = gr_projection_matrix;
	data_out->textureMatrix = gr_texture_matrix;

	data_out->color = material.get_color();

	data_out->vpwidth = 1.0f / i2fl(gr_screen.max_w);
	data_out->vpheight = 1.0f / i2fl(gr_screen.max_h);

	data_out->effect_num = -1;
	data_out->anim_timer = 0;
	data_out->use_clip_plane = 0;

	data_out->flags = 0;
	if (material.is_lit())
		data_out->flags |= MODEL_SDR_FLAG_LIGHT;
	if (material.is_deferred())
		data_out->flags |= MODEL_SDR_FLAG_DEFERRED;
	if (material.is_hdr())
		data_out->flags |= MODEL_SDR_FLAG_HDR;
	if (material.get_texture_map(TM_BASE_TYPE) > 0)
		data_out->flags |= MODEL_SDR_FLAG_DIFFUSE;
	if (material.get_texture_map(TM_GLOW_TYPE) > 0)
		data_out->flags |= MODEL_SDR_FLAG_GLOW;
	if (material.get_texture_map(TM_SPECULAR_TYPE) > 0 || material.get_texture_map(TM_SPEC_GLOSS_TYPE) > 0)
		data_out->flags |= MODEL_SDR_FLAG_SPEC;
	if (ENVMAP > 0)
		data_out->flags |= MODEL_SDR_FLAG_ENV;
	if (material.get_texture_map(TM_NORMAL_TYPE) > 0)
		data_out->flags |= MODEL_SDR_FLAG_NORMAL;
	if (material.get_texture_map(TM_AMBIENT_TYPE) > 0)
		data_out->flags |= MODEL_SDR_FLAG_AMBIENT;
	if (material.get_texture_map(TM_MISC_TYPE) > 0)
		data_out->flags |= MODEL_SDR_FLAG_MISC;
	if (material.get_texture_map(TM_MISC_TYPE) > 0 && material.is_team_color_set())
		data_out->flags |= MODEL_SDR_FLAG_TEAMCOLOR;
	if (material.is_fogged())
		data_out->flags |= MODEL_SDR_FLAG_FOG;
	if (material.is_batched())
		data_out->flags |= MODEL_SDR_FLAG_TRANSFORM;
	if (material.is_shadow_receiving())
		data_out->flags |= MODEL_SDR_FLAG_SHADOWS;
	if (material.get_thrust_scale() > 0.0f)
		data_out->flags |= MODEL_SDR_FLAG_THRUSTER;
	if (material.is_alpha_mult_active())
		data_out->flags |= MODEL_SDR_FLAG_ALPHA_MULT; 

	if (material.get_animated_effect() > 0) {
		data_out->anim_timer = material.get_animated_effect_time();
		data_out->effect_num = material.get_animated_effect();
	}

	data_out->sBasemapIndex = 0;
	data_out->sAmbientmapIndex = 0;
	data_out->sGlowmapIndex = 0;
	data_out->sMiscmapIndex = 0;
	data_out->sNormalmapIndex = 0;
	data_out->sSpecmapIndex = 0;

	if (material.is_clipped()) {
		auto& clip_info = material.get_clip_plane();

		data_out->use_clip_plane = true;

		vec4 clip_equation;
		clip_equation.xyzw.x = clip_info.normal.xyz.x;
		clip_equation.xyzw.y = clip_info.normal.xyz.y;
		clip_equation.xyzw.z = clip_info.normal.xyz.z;
		clip_equation.xyzw.w = -vm_vec_dot(&clip_info.normal, &clip_info.position);
		data_out->clip_equation = clip_equation;
	}

	if (material.is_lit()) {
		int num_lights = MIN(Num_active_gr_lights, (int)graphics::MAX_UNIFORM_LIGHTS);
		data_out->n_lights = num_lights;

		gr_lighting_fill_uniforms(data_out->lights, sizeof(data_out->lights));

		float light_factor = material.get_light_factor();
		data_out->diffuseFactor.xyz.x = gr_light_color[0] * light_factor;
		data_out->diffuseFactor.xyz.y = gr_light_color[1] * light_factor;
		data_out->diffuseFactor.xyz.z = gr_light_color[2] * light_factor;
		gr_get_ambient_light(&data_out->ambientFactor);

		if (material.get_light_factor() > 0.25f && Cmdline_emissive) {
			data_out->emissionFactor.xyz.x = gr_light_emission[0];
			data_out->emissionFactor.xyz.y = gr_light_emission[1];
			data_out->emissionFactor.xyz.z = gr_light_emission[2];
		} else {
			data_out->emissionFactor.xyz.x = gr_light_zero[0];
			data_out->emissionFactor.xyz.y = gr_light_zero[1];
			data_out->emissionFactor.xyz.z = gr_light_zero[2];
		}

	}
	data_out->defaultGloss = 0.6f;

	if (material.get_texture_map(TM_BASE_TYPE) > 0) {
		if (material.is_desaturated()) {
			data_out->desaturate = 1;
		} else {
			data_out->desaturate = 0;
		}


		switch (material.get_blend_mode()) {
		case ALPHA_BLEND_PREMULTIPLIED:
			data_out->blend_alpha = 1;
			break;
		case ALPHA_BLEND_ADDITIVE:
			data_out->blend_alpha = 2;
			break;
		default:
			data_out->blend_alpha = 0;
			break;
		}

		data_out->sBasemapIndex = bm_get_array_index(material.get_texture_map(TM_BASE_TYPE));
	}

	if (material.get_texture_map(TM_GLOW_TYPE) > 0) {
		data_out->sGlowmapIndex = bm_get_array_index(material.get_texture_map(TM_GLOW_TYPE));
	}


	if (material.get_texture_map(TM_SPEC_GLOSS_TYPE) > 0) {
		data_out->sSpecmapIndex = bm_get_array_index(material.get_texture_map(TM_SPEC_GLOSS_TYPE));

		data_out->gammaSpec = 1;
		data_out->alphaGloss = 1;

	} 

	if (material.get_texture_map(TM_SPECULAR_TYPE) > 0) {
		data_out->sSpecmapIndex = bm_get_array_index(material.get_texture_map(TM_SPECULAR_TYPE));

		data_out->gammaSpec = 0;
		data_out->alphaGloss = 0;
	}

	if (ENVMAP > 0) {
		if (material.get_texture_map(TM_SPEC_GLOSS_TYPE) > 0) {
			data_out->envGloss = 1;
		} else {
			data_out->envGloss = 0;
		}

		data_out->envMatrix = gr_env_texture_matrix;
	}
	
	if (material.get_texture_map(TM_NORMAL_TYPE) > 0) {
		data_out->sNormalmapIndex = bm_get_array_index(material.get_texture_map(TM_NORMAL_TYPE));
	}

	if (material.get_texture_map(TM_AMBIENT_TYPE) > 0) {
		data_out->sAmbientmapIndex = bm_get_array_index(material.get_texture_map(TM_AMBIENT_TYPE));
	}

	if (material.get_texture_map(TM_MISC_TYPE) > 0) {
		data_out->sMiscmapIndex = bm_get_array_index(material.get_texture_map(TM_MISC_TYPE));
	}

	if (material.is_shadow_receiving()) {
		data_out->shadow_mv_matrix = Shadow_view_matrix_light;

		for (size_t i = 0; i < MAX_SHADOW_CASCADES; ++i) {
			data_out->shadow_proj_matrix[i] = Shadow_proj_matrix[i];
		}

		data_out->veryneardist = Shadow_cascade_distances[0];
		data_out->neardist = Shadow_cascade_distances[1];
		data_out->middist = Shadow_cascade_distances[2];
		data_out->fardist = Shadow_cascade_distances[3];
	}

	if (shader_flags & SDR_FLAG_MODEL_SHADOW_MAP) {
		for (size_t i = 0; i < MAX_SHADOW_CASCADES; ++i) {
			data_out->shadow_proj_matrix[i] = Shadow_proj_matrix[i];
		}
	}

	if (material.is_batched()) {
		data_out->buffer_matrix_offset = (int) transform_buffer_offset;
	}

	// Team colors are passed to the shader here, but the shader needs to handle their application.
	// By default, this is handled through the r and g channels of the misc map, but this can be changed
	// in the shader; test versions of this used the normal map r and b channels
	if (material.get_texture_map(TM_MISC_TYPE) > 0 && material.is_team_color_set()) {
		auto& tm_clr = material.get_team_color();
		vec3d stripe_color;
		vec3d base_color;

		stripe_color.xyz.x = tm_clr.stripe.r;
		stripe_color.xyz.y = tm_clr.stripe.g;
		stripe_color.xyz.z = tm_clr.stripe.b;

		base_color.xyz.x = tm_clr.base.r;
		base_color.xyz.y = tm_clr.base.g;
		base_color.xyz.z = tm_clr.base.b;

		data_out->stripe_color = stripe_color;
		data_out->base_color = base_color;

		data_out->team_glow_enabled = bm_has_alpha_channel(material.get_texture_map(TM_MISC_TYPE)) ? 1 : 0;
	}

	if (material.get_thrust_scale() > 0.0f) {
		data_out->thruster_scale = material.get_thrust_scale();
	}


	if (material.is_fogged()) {
		auto& fog_params = material.get_fog();

		if (fog_params.enabled) {
			data_out->fogStart = fog_params.dist_near;
			data_out->fogScale = 1.0f / (fog_params.dist_far - fog_params.dist_near);
			data_out->fogColor.xyzw.x = i2fl(fog_params.r) / 255.0f;
			data_out->fogColor.xyzw.y = i2fl(fog_params.g) / 255.0f;
			data_out->fogColor.xyzw.z = i2fl(fog_params.b) / 255.0f;
			data_out->fogColor.xyzw.w = 1.0f;
		}
	}

	if ( shader_flags & SDR_FLAG_MODEL_THICK_OUTLINES ) {
		data_out->outlineWidth = material.get_outline_thickness();
	}

	if (material.is_alpha_mult_active()) {
		data_out->alphaMult = material.get_alpha_mult();
	}
}

}
}

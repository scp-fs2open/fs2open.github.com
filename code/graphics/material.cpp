#include "globalincs/pstypes.h"
#include "graphics/grinternal.h"
#include "graphics/2d.h"
#include "graphics/material.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"

gr_alpha_blend material_determine_blend_mode(int base_bitmap, bool blending)
{
	if ( blending ) {
		if ( base_bitmap >= 0 && bm_has_alpha_channel(base_bitmap) ) {
			return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		}

		return ALPHA_BLEND_ADDITIVE;
	}

	return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
}

gr_zbuffer_type material_determine_depth_mode(bool depth_testing, bool blending)
{
	if ( depth_testing ) {
		if ( blending ) {
			return ZBUFFER_TYPE_READ;
		}

		return ZBUFFER_TYPE_FULL;
	}

	return ZBUFFER_TYPE_NONE;
}

void material_set_unlit(material* mat_info, int texture, float alpha, bool blending, bool depth_testing)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);

	gr_alpha_blend blend_mode = material_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = material_determine_depth_mode(depth_testing, blending);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(depth_mode);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);

	if ( blend_mode == ALPHA_BLEND_ADDITIVE ) {
		mat_info->set_color(alpha, alpha, alpha, 1.0f);
	} else {
		mat_info->set_color(1.0f, 1.0f, 1.0f, alpha);
	}

	if ( texture >= 0 && bm_has_alpha_channel(texture) ) {
		mat_info->set_texture_type(material::TEX_TYPE_XPARENT);
	}
}

void material_set_unlit_emissive(material* mat_info, int texture, float alpha, float color_scale)
{
	material_set_unlit(mat_info, texture, alpha, true, true);

	mat_info->set_color_scale(color_scale);
}

void material_set_unlit_color(material* mat_info, int texture, color *clr, bool blending, bool depth_testing)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);

	gr_alpha_blend blend_mode = material_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = material_determine_depth_mode(depth_testing, blending);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(depth_mode);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	mat_info->set_color(*clr);
}

void material_set_unlit_color(material* mat_info, int texture, color *clr, float alpha, bool blending, bool depth_testing)
{
	color clr_with_alpha;
	gr_init_alphacolor(&clr_with_alpha, clr->red, clr->green, clr->blue, fl2i(alpha * 255.0f));

	material_set_unlit_color(mat_info, texture, &clr_with_alpha, blending, depth_testing);
}

void material_set_interface(material* mat_info, int texture, bool blended, float alpha)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_texture_type(material::TEX_TYPE_INTERFACE);

	mat_info->set_blend_mode(material_determine_blend_mode(texture, blended));
	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);

	mat_info->set_color(1.0f, 1.0f, 1.0f, blended ? alpha : 1.0f);
}

void material_set_unlit_volume(particle_material* mat_info, int texture, bool point_sprites)
{
	mat_info->set_point_sprite_mode(point_sprites);
	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);

	mat_info->set_blend_mode(material_determine_blend_mode(texture, true));

	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_cull_mode(false);
	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);
}

void material_set_distortion(distortion_material *mat_info, int texture, bool thruster)
{
	mat_info->set_thruster_rendering(thruster);

	mat_info->set_depth_mode(ZBUFFER_TYPE_READ);

	mat_info->set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_cull_mode(false);
	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);
}

template <> SCP_vector<int>& uniform_data::get_array<int>() { return int_data; }
template <> SCP_vector<float>& uniform_data::get_array<float>() { return float_data; }
template <> SCP_vector<vec2d>& uniform_data::get_array<vec2d>() { return vec2_data; }
template <> SCP_vector<vec3d>& uniform_data::get_array<vec3d>() { return vec3_data; }
template <> SCP_vector<vec4>& uniform_data::get_array<vec4>() { return vec4_data; }
template <> SCP_vector<matrix4>& uniform_data::get_array<matrix4>() { return matrix4_data; }

template <> bool uniform_data::compare<int>(int index, const int& val)
{
    return int_data[index] == val;
}

template <> bool uniform_data::compare<float>(int index, const float& val)
{
    return fl_equal(float_data[index], val);
}

template <> bool uniform_data::compare<vec2d>(int index, const vec2d& val)
{
    return vm_vec_equal(vec2_data[index], val);
}

template <> bool uniform_data::compare<vec3d>(int index, const vec3d& val)
{
    return vm_vec_equal(vec3_data[index], val);
}

template <> bool uniform_data::compare<vec4>(int index, const vec4& val)
{
    return vm_vec_equal(vec4_data[index], val);
}

template <> bool uniform_data::compare<matrix4>(int index, const matrix4& val)
{
    return vm_matrix_equal(matrix4_data[index], val);
}

template <> bool uniform_data::compare<matrix4>(int index, matrix4* val, int count)
{
    for (int i = 0; i < count; ++i) {
        if (!vm_matrix_equal(matrix4_data[index + i], val[i])) {
            return false;
        }
    }
    
    return true;
}

template <> uniform::data_type uniform::determine_type<int>() { return INT; }
template <> uniform::data_type uniform::determine_type<float>() { return FLOAT; }
template <> uniform::data_type uniform::determine_type<vec2d>() { return VEC2; }
template <> uniform::data_type uniform::determine_type<vec3d>() { return VEC3; }
template <> uniform::data_type uniform::determine_type<vec4>() { return VEC4; }
template <> uniform::data_type uniform::determine_type<matrix4>() { return MATRIX4; }

material::material():
Sdr_handle(-1), 
Sdr_type(SDR_TYPE_NONE),
Tex_source(TEXTURE_SOURCE_NO_FILTERING), 
Tex_type(TEX_TYPE_NORMAL),
Texture_addressing(TMAP_ADDRESS_WRAP),
Depth_bias(0),
Depth_mode(ZBUFFER_TYPE_NONE),
Blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA),
Cull_mode(true),
Fill_mode(GR_FILL_MODE_SOLID),
Clr_scale(1.0f)
{
	gr_init_alphacolor(&Clr, 255, 255, 255, 255);
	
	Texture_maps[TM_BASE_TYPE]		= -1;
	Texture_maps[TM_GLOW_TYPE]		= -1;
	Texture_maps[TM_SPECULAR_TYPE]	= -1;
	Texture_maps[TM_NORMAL_TYPE]	= -1;
	Texture_maps[TM_HEIGHT_TYPE]	= -1;
	Texture_maps[TM_MISC_TYPE]		= -1;
	Texture_maps[TM_SPEC_GLOSS_TYPE] = -1;
	Texture_maps[TM_AMBIENT_TYPE] = -1;

	Fog_params.dist_near = -1.0f;
	Fog_params.dist_far = -1.0f;
	Fog_params.r = 0;
	Fog_params.g = 0;
	Fog_params.b = 0;
	Fog_params.enabled = false;

	Clip_params.enabled = false;
};

void material::set_shader_type(shader_type init_sdr_type)
{
	Sdr_type = init_sdr_type;
}

void material::set_shader_handle(int handle)
{
	Sdr_handle = handle;
}

int material::get_shader_handle()
{
	return Sdr_handle;
}

void material::set_texture_map(int texture_type, int texture_num)
{
	Assert(texture_type > -1 && texture_type < TM_NUM_TYPES);

	Texture_maps[texture_type] = texture_num;
}

int material::get_texture_map(int texture_type)
{
	Assert(texture_type > -1 && texture_type < TM_NUM_TYPES);

	return Texture_maps[texture_type];
}

bool material::is_textured()
{
	return 
	Texture_maps[TM_BASE_TYPE]		> -1 ||
	Texture_maps[TM_GLOW_TYPE]		> -1 ||
	Texture_maps[TM_SPECULAR_TYPE]	> -1 ||
	Texture_maps[TM_NORMAL_TYPE]	> -1 ||
	Texture_maps[TM_HEIGHT_TYPE]	> -1 ||
	Texture_maps[TM_MISC_TYPE]		> -1;
}

void material::set_texture_type(texture_type t_type)
{
	Tex_type = t_type;
}

int material::get_texture_type()
{
	switch ( Tex_type ) {
	default:
	case TEX_TYPE_NORMAL:
		return TCACHE_TYPE_NORMAL;
	case TEX_TYPE_XPARENT:
		return TCACHE_TYPE_XPARENT;
	case TEX_TYPE_INTERFACE:
		return TCACHE_TYPE_INTERFACE;
	case TEX_TYPE_AABITMAP:
		return TCACHE_TYPE_AABITMAP;
	}
}

void material::set_texture_source(gr_texture_source source)
{
	Tex_source = source;
}

gr_texture_source material::get_texture_source()
{
	return Tex_source;
}

bool material::is_clipped()
{
	return Clip_params.enabled;
}

void material::set_clip_plane(const vec3d &normal, const vec3d &position)
{
	Clip_params.enabled = true;
	Clip_params.normal = normal;
	Clip_params.position = position;
}

void material::set_clip_plane()
{
	Clip_params.enabled = false;
}

material::clip_plane& material::get_clip_plane()
{
	return Clip_params;
}

void material::set_texture_addressing(int addressing)
{
	Texture_addressing = addressing;
}

int material::get_texture_addressing()
{
	return Texture_addressing;
}

void material::set_fog(int r, int g, int b, float near, float far)
{
	Fog_params.enabled = true;
	Fog_params.r = r;
	Fog_params.g = g;
	Fog_params.b = b;
	Fog_params.dist_near = near;
	Fog_params.dist_far = far;
}

void material::set_fog()
{
	Fog_params.enabled = false;
}

bool material::is_fogged()
{
	return Fog_params.enabled;
}

material::fog& material::get_fog()
{
	return Fog_params; 
}

void material::set_depth_mode(gr_zbuffer_type mode)
{
	Depth_mode = mode;
}

gr_zbuffer_type material::get_depth_mode()
{
	return Depth_mode;
}

void material::set_cull_mode(bool mode)
{
	Cull_mode = mode;
}

bool material::get_cull_mode()
{
	return Cull_mode;
}

void material::set_fill_mode(int mode)
{
	Fill_mode = mode;
}

int material::get_fill_mode()
{
	return Fill_mode;
}

void material::set_blend_mode(gr_alpha_blend mode)
{
	Blend_mode = mode;
}

gr_alpha_blend material::get_blend_mode()
{
	return Blend_mode;
}

void material::set_depth_bias(int bias)
{
	Depth_bias = bias;
}

int material::get_depth_bias()
{
	return Depth_bias;
}

void material::set_color(float red, float green, float blue, float alpha)
{
	set_color(fl2i(red * 255.0f), fl2i(green * 255.0f), fl2i(blue * 255.0f), fl2i(alpha * 255.0f));
}

void material::set_color(int r, int g, int b, int a)
{
	gr_init_alphacolor(&Clr, r, g, b, a);
}

void material::set_color(color &clr_in)
{
	if ( clr_in.is_alphacolor ) {
		Clr = clr_in;
	} else {
		gr_init_alphacolor(&Clr, clr_in.red, clr_in.green, clr_in.blue, 255);
	}
}

color& material::get_color()
{
	return Clr;
}

void material::set_color_scale(float scale)
{
	Clr_scale = scale;
}

float material::get_color_scale()
{
	return Clr_scale;
}

model_material::model_material(): 
material(), 
Animated_effect(-1), 
Animated_timer(0.0f), 
Thrust_scale(-1.0f), 
lighting(false), 
Light_factor(1.0f), 
Batched(false), 
Team_color_set(false), 
Center_alpha(0),
Desaturate(false),
Normal_alpha(false),
Normal_alpha_min(0.0f),
Normal_alpha_max(1.0f),
Normal_extrude(false),
Deferred(false),
HDR(false)
{ 
	set_shader_type(SDR_TYPE_MODEL); 
}

void model_material::set_desaturation(bool enabled)
{
	Desaturate = enabled;
}

bool model_material::is_desaturated()
{
	return Desaturate;
}

void model_material::set_shadow_casting(bool enabled)
{
	Shadow_casting = enabled;
}

void model_material::set_light_factor(float factor)
{
	Light_factor = factor;
}

float model_material::get_light_factor()
{
	return Light_factor;
}

void model_material::set_lighting(bool mode)
{
	lighting = mode;
} 

bool model_material::is_lit()
{
	return lighting;
}

void model_material::set_deferred_lighting(bool enabled)
{
	Deferred = enabled;
}

void model_material::set_high_dynamic_range(bool enabled)
{
	HDR = enabled;
}

void model_material::set_center_alpha(int c_alpha)
{
	Center_alpha = c_alpha;
}

int model_material::get_center_alpha()
{
	return Center_alpha;
}

void model_material::set_thrust_scale(float scale)
{
	Thrust_scale = scale;
}

float model_material::get_thrust_scale()
{
	return Thrust_scale;
}

void model_material::set_team_color(const team_color &color)
{
	Team_color_set = true;
	Tm_color = color;
}

void model_material::set_team_color()
{
	Team_color_set = false;
}

team_color& model_material::get_team_color()
{
	return Tm_color;
}

void model_material::set_animated_effect(int effect, float time)
{
	Animated_effect = effect;
	Animated_timer = time;
}

void model_material::set_animated_effect()
{
	Animated_effect = 0;
	Animated_timer = 0.0f;
}

int model_material::get_animated_effect()
{
	return Animated_effect;
}

float model_material::get_animated_effect_time()
{
	return Animated_timer;
}

void model_material::set_batching(bool enabled)
{
	Batched = enabled;
}

bool model_material::is_batched()
{
	return Batched;
}

void model_material::set_normal_alpha(float min, float max)
{
	Normal_alpha = true;
	Normal_alpha_min = min;
	Normal_alpha_max = max;
}

void model_material::set_normal_alpha()
{
	Normal_alpha = false;
}

bool model_material::is_normal_alpha_active()
{
	return Normal_alpha;
}

float model_material::get_normal_alpha_min()
{
	return Normal_alpha_min;
}

float model_material::get_normal_alpha_max()
{
	return Normal_alpha_max;
}

void model_material::set_normal_extrude(float width)
{
	Normal_extrude = true;
	Normal_extrude_width = width;
}

void model_material::set_normal_extrude()
{
	Normal_extrude = false;
}

bool model_material::is_normal_extrude_active()
{
	return Normal_extrude;
}

float model_material::get_normal_extrude_width()
{
	return Normal_extrude_width;
}

uint model_material::get_shader_flags()
{
	uint Shader_flags = 0;

	if ( is_clipped() ) {
		Shader_flags |= SDR_FLAG_MODEL_CLIP;
	}

	if ( is_batched() ) {
		Shader_flags |= SDR_FLAG_MODEL_TRANSFORM;
	}

	if ( Shadow_casting ) {
		// if we're building the shadow map, we likely only need the flags here and above so bail
		Shader_flags |= SDR_FLAG_MODEL_SHADOW_MAP;

		return Shader_flags;
	}
	
	if ( is_fogged() ) {
		Shader_flags |= SDR_FLAG_MODEL_FOG;
	}

	if ( Animated_effect >= 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_ANIMATED;
	}

	if ( get_texture_map(TM_BASE_TYPE) > 0 && !Basemap_override ) {
		Shader_flags |= SDR_FLAG_MODEL_DIFFUSE_MAP;
	}

	if ( get_texture_map(TM_GLOW_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_GLOW_MAP;
	}

	if ( lighting ) {
		Shader_flags |= SDR_FLAG_MODEL_LIGHT;

		if ( ( get_texture_map(TM_SPECULAR_TYPE) > 0) && !Specmap_override ) {
			Shader_flags |= SDR_FLAG_MODEL_SPEC_MAP;

			if ( (ENVMAP > 0) && !Envmap_override ) {
				Shader_flags |= SDR_FLAG_MODEL_ENV_MAP;
			}
		}

		if ( (get_texture_map(TM_NORMAL_TYPE) > 0) && !Normalmap_override ) {
			Shader_flags |= SDR_FLAG_MODEL_NORMAL_MAP;
		}

		if ( (get_texture_map(TM_HEIGHT_TYPE) > 0) && !Heightmap_override ) {
			Shader_flags |= SDR_FLAG_MODEL_HEIGHT_MAP;
		}

		if ( Cmdline_shadow_quality && !Shadow_casting && !Shadow_override ) {
			Shader_flags |= SDR_FLAG_MODEL_SHADOWS;
		}
	}

	if ( get_texture_map(TM_MISC_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_MISC_MAP;

		if ( Team_color_set ) {
			Shader_flags |= SDR_FLAG_MODEL_TEAMCOLOR;
		}
	}

	if ( Deferred ) {
		Shader_flags |= SDR_FLAG_MODEL_DEFERRED;
	}

	if ( HDR ) {
		Shader_flags |= SDR_FLAG_MODEL_HDR;
	}

	if ( Thrust_scale > 0.0f ) {
		Shader_flags |= SDR_FLAG_MODEL_THRUSTER;
	}

	if ( Normal_alpha ) {
		Shader_flags |= SDR_FLAG_MODEL_NORMAL_ALPHA;
	}

	if ( Normal_extrude ) {
		Shader_flags |= SDR_FLAG_MODEL_NORMAL_EXTRUDE;
	}

	return Shader_flags;
}

int model_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_MODEL, get_shader_flags()));

	return material::get_shader_handle();
}

particle_material::particle_material(): 
material()  
{
	set_shader_type(SDR_TYPE_EFFECT_PARTICLE);
}

void particle_material::set_point_sprite_mode(bool enabled)
{
	Point_sprite = enabled;
}

bool particle_material::get_point_sprite_mode()
{
	return Point_sprite;
}

int particle_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	uint flags = 0;

	if ( Point_sprite ) {
		flags |= SDR_FLAG_PARTICLE_POINT_GEN;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, flags));

	return material::get_shader_handle();
}

distortion_material::distortion_material(): 
material()
{
	set_shader_type(SDR_TYPE_EFFECT_DISTORTION);
}

void distortion_material::set_thruster_rendering(bool enabled)
{
	Thruster_render = enabled;
}

bool distortion_material::get_thruster_rendering()
{
	return Thruster_render;
}

int distortion_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_EFFECT_DISTORTION, 0));

	return material::get_shader_handle();
}
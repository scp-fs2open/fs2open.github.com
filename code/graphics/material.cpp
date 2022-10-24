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

	if ( blend_mode == ALPHA_BLEND_ADDITIVE ) {
		mat_info->set_color(alpha, alpha, alpha, 1.0f);
	} else {
		mat_info->set_color(1.0f, 1.0f, 1.0f, alpha);
	}

	if ( texture >= 0 && bm_has_alpha_channel(texture) ) {
		mat_info->set_texture_type(material::TEX_TYPE_XPARENT);
	}
}

void material_set_unlit_opaque(material* mat_info, int texture, bool depth_testing)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);

	gr_alpha_blend blend_mode = ALPHA_BLEND_NONE;
	gr_zbuffer_type depth_mode = material_determine_depth_mode(depth_testing, false);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(depth_mode);
	mat_info->set_cull_mode(false);

	
	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);
	

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

	gr_alpha_blend blend_mode = material_determine_blend_mode(texture, blended);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);
	mat_info->set_cull_mode(false);

	if ( blend_mode == ALPHA_BLEND_ADDITIVE ) {
		mat_info->set_color(alpha, alpha, alpha, 1.0f);
	} else {
		mat_info->set_color(1.0f, 1.0f, 1.0f, alpha);
	}
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
void material_set_rocket_interface(interface_material* mat_info,
	int texture,
	const vec2d& offset,
	float horizontal_swipe)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_offset(offset);
	mat_info->set_horizontal_swipe(horizontal_swipe);

	mat_info->set_cull_mode(false);
	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);

	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);

	mat_info->set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	mat_info->set_texture_type(material::TEX_TYPE_INTERFACE);
}

void material_set_movie(movie_material* mat_info, int y_bm, int u_bm, int v_bm, float alpha) {
	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);
	mat_info->set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	mat_info->set_cull_mode(false);
	mat_info->set_color(1.0f, 1.0f, 1.0f, alpha);

	mat_info->setYtex(y_bm);
	mat_info->setUtex(u_bm);
	mat_info->setVtex(v_bm);

	mat_info->set_texture_type(material::TEX_TYPE_AABITMAP);
}

void material_set_batched_bitmap(batched_bitmap_material* mat_info, int base_tex, float alpha, float color_scale) {
	material_set_unlit(mat_info, base_tex, alpha, true, true);

	mat_info->set_color_scale(color_scale);
}

void material_set_batched_opaque_bitmap(batched_bitmap_material* mat_info, int base_tex, float color_scale) {
	material_set_unlit_opaque(mat_info, base_tex, true);

	mat_info->set_color_scale(color_scale);
}

void material_set_nanovg(nanovg_material* mat_info, int base_tex) {
	material_set_unlit(mat_info, base_tex, 1.0f, true, false);

	mat_info->set_cull_mode(false);

	mat_info->set_color_mask(true, true, true, true);

	mat_info->set_blend_mode(ALPHA_BLEND_PREMULTIPLIED);

	mat_info->set_stencil_mask(0xFFFFFFFF);
	mat_info->set_stencil_func(ComparisionFunction::Always, 0, 0xFFFFFFFF);
	mat_info->set_front_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);
	mat_info->set_back_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);
}

void material_set_decal(material* mat_info, int diffuse_tex, int glow_tex, int normal_tex) {
	mat_info->set_depth_mode(ZBUFFER_TYPE_READ);

	mat_info->set_blend_mode(0, material_determine_blend_mode(diffuse_tex, true));
	mat_info->set_blend_mode(1, ALPHA_BLEND_ADDITIVE); // Normal blending must always be additive
	mat_info->set_blend_mode(2, material_determine_blend_mode(glow_tex, true));

	mat_info->set_texture_map(TM_BASE_TYPE, diffuse_tex);
	mat_info->set_texture_map(TM_GLOW_TYPE, glow_tex);
	mat_info->set_texture_map(TM_NORMAL_TYPE, normal_tex);
	mat_info->set_cull_mode(false);

	// Done't write the alpha channel in any case since that will cause changes to other material properties
	// TODO: If decals should be able to change more than just the material diffuse color then a solution with separate
	// blending equations must be used
	mat_info->set_color_mask(true, true, true, false);
}

material::material():
Sdr_type(SDR_TYPE_DEFAULT_MATERIAL),
Tex_type(TEX_TYPE_NORMAL),
Texture_addressing(TMAP_ADDRESS_WRAP),
Depth_mode(ZBUFFER_TYPE_NONE),
Blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA),
Cull_mode(true),
Fill_mode(GR_FILL_MODE_SOLID),
Clr_scale(1.0f),
Depth_bias(0)
{
	Clr.xyzw.x = 1.0f;
	Clr.xyzw.y = 1.0f; 
	Clr.xyzw.z = 1.0f; 
	Clr.xyzw.w = 1.0f;
	
	Texture_maps[TM_BASE_TYPE]		= -1;
	Texture_maps[TM_GLOW_TYPE]		= -1;
	Texture_maps[TM_SPECULAR_TYPE]	= -1;
	Texture_maps[TM_NORMAL_TYPE]	= -1;
	Texture_maps[TM_HEIGHT_TYPE]	= -1;
	Texture_maps[TM_MISC_TYPE]		= -1;
	Texture_maps[TM_SPEC_GLOSS_TYPE] = -1;
	Texture_maps[TM_AMBIENT_TYPE] = -1;

	Clip_params.enabled = false;

	Color_mask.x = true;
	Color_mask.y = true;
	Color_mask.z = true;
	Color_mask.w = true;

	Buffer_blend_mode.fill(Blend_mode);
};

void material::set_shader_type(shader_type init_sdr_type)
{
	Sdr_type = init_sdr_type;
}

uint material::get_shader_flags() const
{
	return 0;
}

int material::get_shader_handle() const
{
	return gr_maybe_create_shader(Sdr_type, get_shader_flags());
}

void material::set_texture_map(int tex_type, int texture_num)
{
	Assert(tex_type > -1 && tex_type < TM_NUM_TYPES);

	Texture_maps[tex_type] = texture_num;
}

int material::get_texture_map(int tex_type) const
{
	Assert(tex_type > -1 && tex_type < TM_NUM_TYPES);

	return Texture_maps[tex_type];
}

bool material::is_textured() const
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

int material::get_texture_type() const
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

bool material::is_clipped() const
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

const material::clip_plane& material::get_clip_plane() const
{
	return Clip_params;
}

void material::set_texture_addressing(int addressing)
{
	Texture_addressing = addressing;
}

int material::get_texture_addressing() const
{
	return Texture_addressing;
}

void material::set_depth_mode(gr_zbuffer_type mode)
{
	Depth_mode = mode;
}

gr_zbuffer_type material::get_depth_mode() const
{
	return Depth_mode;
}

void material::set_cull_mode(bool mode)
{
	Cull_mode = mode;
}

bool material::get_cull_mode() const
{
	return Cull_mode;
}

void material::set_fill_mode(int mode)
{
	Fill_mode = mode;
}

int material::get_fill_mode() const
{
	return Fill_mode;
}

void material::set_blend_mode(gr_alpha_blend mode)
{
	Blend_mode = mode;
	Has_buffer_blends = false;
	Buffer_blend_mode.fill(mode);
}

void material::set_blend_mode(int buffer, gr_alpha_blend mode)
{
	Assertion(buffer >= 0 && buffer < (int) Buffer_blend_mode.size(), "Invalid buffer index %d found!", buffer);

	Has_buffer_blends = true;
	Buffer_blend_mode[buffer] = mode;
}

bool material::has_buffer_blend_modes() const {
	return Has_buffer_blends;
}

gr_alpha_blend material::get_blend_mode(int buffer) const
{
	if (!Has_buffer_blends) {
		return Blend_mode;
	}

	Assertion(buffer >= 0 && buffer < (int) Buffer_blend_mode.size(), "Invalid buffer index %d found!", buffer);

	return Buffer_blend_mode[buffer];
}

void material::set_depth_bias(int bias)
{
	Depth_bias = bias;
}

int material::get_depth_bias() const
{
	return Depth_bias;
}

void material::set_color(float red, float green, float blue, float alpha)
{
	Clr.xyzw.x = red;
	Clr.xyzw.y = green;
	Clr.xyzw.z = blue;
	Clr.xyzw.w = alpha;
}

void material::set_color(int r, int g, int b, int a)
{
	CLAMP(r, 0, 255);
	CLAMP(b, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(a, 0, 255);

	Clr.xyzw.x = i2fl(r) / 255.0f;
	Clr.xyzw.y = i2fl(g) / 255.0f;
	Clr.xyzw.z = i2fl(b) / 255.0f;
	Clr.xyzw.w = i2fl(a) / 255.0f;
}

void material::set_color(color &clr_in)
{
	if ( clr_in.is_alphacolor ) {
		Clr.xyzw.x = i2fl(clr_in.red) / 255.0f;
		Clr.xyzw.y = i2fl(clr_in.green) / 255.0f;
		Clr.xyzw.z = i2fl(clr_in.blue) / 255.0f;
		Clr.xyzw.w = i2fl(clr_in.alpha) / 255.0f;
	} else {
		Clr.xyzw.x = i2fl(clr_in.red) / 255.0f;
		Clr.xyzw.y = i2fl(clr_in.green) / 255.0f;
		Clr.xyzw.z = i2fl(clr_in.blue) / 255.0f;
		Clr.xyzw.w = 1.0f;
	}
}

const vec4& material::get_color() const
{
	return Clr;
}

void material::set_color_scale(float scale)
{
	Clr_scale = scale;
}

float material::get_color_scale() const
{
	return Clr_scale;
}
void material::set_stencil_test(bool stencil) {
	Stencil_test = stencil;
}
bool material::is_stencil_enabled() const {
	return Stencil_test;
}
void material::set_stencil_mask(uint32_t mask) {
	Stencil_mask = mask;
}
uint32_t material::get_stencil_mask() const {
	return Stencil_mask;
}
void material::set_stencil_func(ComparisionFunction compare, int ref, uint32_t mask) {
	Stencil_func.compare = compare;
	Stencil_func.ref = ref;
	Stencil_func.mask = mask;
}
const material::StencilFunc& material::get_stencil_func() const {
	return Stencil_func;
}
void material::set_stencil_op(StencilOperation stencilFailOperation,
							  StencilOperation depthFailOperation,
							  StencilOperation successOperation) {
	set_front_stencil_op(stencilFailOperation, depthFailOperation, successOperation);
	set_back_stencil_op(stencilFailOperation, depthFailOperation, successOperation);
}
void material::set_front_stencil_op(StencilOperation stencilFailOperation,
									StencilOperation depthFailOperation,
									StencilOperation successOperation) {
	Front_stencil_op.stencilFailOperation = stencilFailOperation;
	Front_stencil_op.depthFailOperation = depthFailOperation;
	Front_stencil_op.successOperation = successOperation;
}
const material::StencilOp& material::get_front_stencil_op() const {
	return Front_stencil_op;
}
void material::set_back_stencil_op(StencilOperation stencilFailOperation,
								   StencilOperation depthFailOperation,
								   StencilOperation successOperation) {
	Back_stencil_op.stencilFailOperation = stencilFailOperation;
	Back_stencil_op.depthFailOperation = depthFailOperation;
	Back_stencil_op.successOperation = successOperation;
}
const material::StencilOp& material::get_back_stencil_op() const {
	return Back_stencil_op;
}
void material::set_color_mask(bool red, bool green, bool blue, bool alpha) {
	Color_mask.x = red;
	Color_mask.y = green;
	Color_mask.z = blue;
	Color_mask.w = alpha;
}
const bvec4& material::get_color_mask() const {
	return Color_mask;
}

model_material::model_material() : material() {
	set_shader_type(SDR_TYPE_MODEL);
}

void model_material::set_desaturation(bool enabled)
{
	Desaturate = enabled;
}

bool model_material::is_desaturated() const
{
	return Desaturate;
}

void model_material::set_shadow_casting(bool enabled)
{
	Shadow_casting = enabled;
}

bool model_material::is_shadow_casting() const {
	return Shadow_casting;
}

void model_material::set_shadow_receiving(bool enabled) {
	Shadow_receiving = enabled;
}

bool model_material::is_shadow_receiving() const {
	return Shadow_receiving;
}

void model_material::set_light_factor(float factor)
{
	Light_factor = factor;
}

float model_material::get_light_factor() const
{
	return Light_factor;
}

void model_material::set_lighting(bool mode)
{
	lighting = mode;
} 

bool model_material::is_lit() const
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

int model_material::get_center_alpha() const
{
	return Center_alpha;
}

void model_material::set_thrust_scale(float scale)
{
	Thrust_scale = scale;
}

float model_material::get_thrust_scale() const
{
	return Thrust_scale;
}

void model_material::set_team_color(const team_color &team_clr)
{
	Team_color_set = true;
	Tm_color = team_clr;
}

void model_material::set_team_color()
{
	Team_color_set = false;
}

const team_color& model_material::get_team_color() const
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

int model_material::get_animated_effect() const
{
	return Animated_effect;
}

float model_material::get_animated_effect_time() const
{
	return Animated_timer;
}

void model_material::set_batching(bool enabled)
{
	Batched = enabled;
}

bool model_material::is_batched() const
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

bool model_material::is_normal_alpha_active() const
{
	return Normal_alpha;
}

float model_material::get_normal_alpha_min() const
{
	return Normal_alpha_min;
}

float model_material::get_normal_alpha_max() const
{
	return Normal_alpha_max;
}

void model_material::set_fog(int r, int g, int b, float _near, float _far)
{
	Fog_params.enabled = true;
	Fog_params.r = r;
	Fog_params.g = g;
	Fog_params.b = b;
	Fog_params.dist_near = _near;
	Fog_params.dist_far = _far;
}

void model_material::set_fog()
{
	Fog_params.enabled = false;
}

bool model_material::is_fogged() const
{
	return Fog_params.enabled;
}

const model_material::fog& model_material::get_fog() const
{
	return Fog_params;
}

void model_material::set_outline_thickness(float thickness) {
	Outline_thickness = thickness;
}
float model_material::get_outline_thickness() const {
	return Outline_thickness;
}
bool model_material::uses_thick_outlines() const {
	return Outline_thickness > 0.0f;
}

float model_material::get_alpha_mult() const {
	return Alpha_mult;
}

bool model_material::is_alpha_mult_active() const {
	return Use_alpha_mult;
}

void model_material::set_alpha_mult(float alpha) {
	Use_alpha_mult = true;
	Alpha_mult = alpha;
}

void model_material::reset_alpha_mult() {
	Use_alpha_mult = false;
	Alpha_mult = 1.0f;
}

uint model_material::get_shader_flags() const
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

	if ( get_texture_map(TM_BASE_TYPE) > 0) {
		Shader_flags |= SDR_FLAG_MODEL_DIFFUSE_MAP;
	}

	if ( get_texture_map(TM_GLOW_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_GLOW_MAP;
	}

	if ( get_texture_map(TM_SPECULAR_TYPE) > 0 || get_texture_map(TM_SPEC_GLOSS_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_SPEC_MAP;
	}
	if ( (ENVMAP > 0) && !Envmap_override ) {
		Shader_flags |= SDR_FLAG_MODEL_ENV_MAP;
	}

	if ( get_texture_map(TM_NORMAL_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_NORMAL_MAP;
	}

	if ( get_texture_map(TM_HEIGHT_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_HEIGHT_MAP;
	}

	if ( get_texture_map(TM_AMBIENT_TYPE) > 0) {
		Shader_flags |= SDR_FLAG_MODEL_AMBIENT_MAP;
	}

	if ( lighting ) {
		Shader_flags |= SDR_FLAG_MODEL_LIGHT;
		
		if ( Shadow_receiving && !Shadow_override ) {
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

	if ( uses_thick_outlines() ) {
		Shader_flags |= SDR_FLAG_MODEL_THICK_OUTLINES;
	}

	if (is_alpha_mult_active()) {
		Shader_flags |= SDR_FLAG_MODEL_ALPHA_MULT;
	}

	return Shader_flags;
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

uint particle_material::get_shader_flags() const
{
	uint flags = 0;

	if ( Point_sprite ) {
		flags |= SDR_FLAG_PARTICLE_POINT_GEN;
	}

	return flags;
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

shield_material::shield_material() :
	material()
{
	set_shader_type(SDR_TYPE_SHIELD_DECAL);

	vm_set_identity(&Impact_orient);

	Impact_pos.xyz.x = 0.0f;
	Impact_pos.xyz.y = 0.0f;
	Impact_pos.xyz.z = 0.0f;

	Impact_radius = 1.0f;
}

void shield_material::set_impact_transform(matrix &orient, vec3d &pos)
{
	Impact_orient = orient;
	Impact_pos = pos;
}

void shield_material::set_impact_radius(float radius)
{
	Impact_radius = radius;
}

const matrix& shield_material::get_impact_orient()
{
	return Impact_orient;
}

const vec3d& shield_material::get_impact_pos()
{
	return Impact_pos;
}

float shield_material::get_impact_radius()
{
	return Impact_radius;
}

movie_material::movie_material() : material() {
	set_shader_type(SDR_TYPE_VIDEO_PROCESS);
}
int movie_material::getYtex() const {
	return Ytex;
}
int movie_material::getUtex() const {
	return Utex;
}
int movie_material::getVtex() const {
	return Vtex;
}
void movie_material::setYtex(int _Ytex) {
	this->Ytex = _Ytex;
}
void movie_material::setUtex(int _Utex) {
	this->Utex = _Utex;
}
void movie_material::setVtex(int _Vtex) {
	movie_material::Vtex = _Vtex;
}

batched_bitmap_material::batched_bitmap_material() {
	set_shader_type(SDR_TYPE_BATCHED_BITMAP);
}

nanovg_material::nanovg_material() {
	set_shader_type(SDR_TYPE_NANOVG);
}

decal_material::decal_material() {
	set_shader_type(SDR_TYPE_DECAL);
}
uint decal_material::get_shader_flags() const {
	uint flags = 0;

	if (get_texture_map(TM_NORMAL_TYPE) == -1) {
		// If we don't write to the normal map then we can use the existing normal map for better normal accuracy
		flags |= SDR_FLAG_DECAL_USE_NORMAL_MAP;
	}

	return flags;
}

interface_material::interface_material()
{
	set_shader_type(SDR_TYPE_ROCKET_UI);

	offset.x = 0;
	offset.y = 0;
}
void interface_material::set_offset(const vec2d& new_offset) { this->offset = new_offset; }
vec2d interface_material::get_offset() const { return offset; }
void interface_material::set_horizontal_swipe(float hor_offset) { this->horizontalSwipeOff = hor_offset; }
float interface_material::get_horizontal_swipe() const { return this->horizontalSwipeOff; }

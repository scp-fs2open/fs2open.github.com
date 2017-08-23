#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "graphics/grinternal.h"
#include "model/model.h"

class material
{
public:
	struct fog 
	{
		bool enabled;
		int r;
		int g;
		int b;
		float dist_near;
		float dist_far;
	};

	struct clip_plane
	{
		bool enabled;
		vec3d normal;
		vec3d position;
	};

	enum texture_type {
		TEX_TYPE_NORMAL,
		TEX_TYPE_XPARENT,
		TEX_TYPE_AABITMAP,
		TEX_TYPE_INTERFACE
	};

private:
	shader_type Sdr_type;

	int Texture_maps[TM_NUM_TYPES];
	texture_type Tex_type;

	clip_plane Clip_params;
	int Texture_addressing;
	fog Fog_params;
	gr_zbuffer_type Depth_mode;
	gr_alpha_blend Blend_mode;
	bool Cull_mode;
	int Fill_mode;
	vec4 Clr;
	float Clr_scale;
	int Depth_bias;

protected:
	void set_shader_type(shader_type init_sdr_type = SDR_TYPE_NONE);
public:
	material();

	int get_shader_handle();
	virtual uint get_shader_flags();

	void set_texture_map(int tex_type, int texture_num);
	int get_texture_map(int tex_type);
	bool is_textured();

	void set_texture_type(texture_type t_type);
	int get_texture_type();
	
	bool is_clipped();
	void set_clip_plane(const vec3d &normal, const vec3d &position);
	void set_clip_plane();
	clip_plane& get_clip_plane();

	void set_texture_addressing(int addressing);
	int get_texture_addressing();

	void set_fog(int r, int g, int b, float near, float far);
	void set_fog();
	bool is_fogged();
	fog& get_fog();

	void set_depth_mode(gr_zbuffer_type mode);
	gr_zbuffer_type get_depth_mode();

	void set_cull_mode(bool mode);
	bool get_cull_mode();

	void set_fill_mode(int mode);
	int get_fill_mode();

	void set_blend_mode(gr_alpha_blend mode);
	gr_alpha_blend get_blend_mode();

	void set_depth_bias(int bias);
	int get_depth_bias();

	void set_color(float red, float green, float blue, float alpha);
	void set_color(int r, int g, int b, int a);
	void set_color(color &clr_in);
	const vec4& get_color();

	void set_color_scale(float scale);
	float get_color_scale();
};

class model_material : public material
{
	bool Desaturate = false;

	bool Shadow_casting = false;
	bool Batched = false;

	bool Deferred = false;
	bool HDR = false;
	bool lighting = false;
	float Light_factor = 1.0f;

	int Center_alpha = 0;

	int Animated_effect = -1;
	float Animated_timer = 0.0f;

	float Thrust_scale = -1.0f;

	bool Team_color_set = false;
	team_color Tm_color;

	bool Normal_alpha = false;
	float Normal_alpha_min = 0.0f;
	float Normal_alpha_max = 1.0f;

	bool Normal_extrude = false;
	float Normal_extrude_width = -1.0f;

public:
	model_material();

	void set_desaturation(bool enabled);
	bool is_desaturated();

	void set_shadow_casting(bool enabled);
	bool is_shadow_casting();

	void set_light_factor(float factor);
	float get_light_factor();

	void set_lighting(bool mode);
	bool is_lit();

	void set_deferred_lighting(bool enabled);
	void set_high_dynamic_range(bool enabled);
	
	void set_center_alpha(int center_alpha);
	int get_center_alpha();

	void set_thrust_scale(float scale = -1.0f);
	float get_thrust_scale();

	void set_team_color(const team_color &Team_clr);
	void set_team_color();
	team_color& get_team_color();

	void set_animated_effect(int effect, float time);
	void set_animated_effect();
	int get_animated_effect();
	float get_animated_effect_time();

	void set_normal_alpha(float min, float max);
	void set_normal_alpha();
	bool is_normal_alpha_active();
	float get_normal_alpha_min();
	float get_normal_alpha_max();

	void set_normal_extrude(float width);
	void set_normal_extrude();
	bool is_normal_extrude_active();
	float get_normal_extrude_width();

	void set_batching(bool enabled);
	bool is_batched();

	virtual uint get_shader_flags();
};

class particle_material : public material
{
	bool Point_sprite;
public:
	particle_material();

	void set_point_sprite_mode(bool enabled);
	bool get_point_sprite_mode();

	virtual uint get_shader_flags();
};

class distortion_material: public material
{
	bool Thruster_render;
public:
	distortion_material();

	void set_thruster_rendering(bool enabled);
	bool get_thruster_rendering();
};

class shield_material : public material
{
	matrix Impact_orient;
	vec3d Impact_pos;
	float Impact_radius;
public:
	shield_material();

	void set_impact_transform(matrix &orient, vec3d &pos);
	void set_impact_radius(float radius);

	const matrix& get_impact_orient();
	const vec3d& get_impact_pos();
	float get_impact_radius();
};

class movie_material : public material
{
	int Ytex = -1;
	int Utex = -1;
	int Vtex = -1;
 public:
	movie_material();

	int getYtex() const;
	int getUtex() const;
	int getVtex() const;

	void setYtex(int _Ytex);
	void setUtex(int _Utex);
	void setVtex(int _Vtex);
};

gr_alpha_blend material_determine_blend_mode(int base_bitmap, bool is_transparent);
gr_zbuffer_type material_determine_depth_mode(bool depth_testing, bool is_transparent);

void material_set_interface(material* mat_info, int texture, bool blended, float alpha);
void material_set_unlit(material* mat_info, int texture, float alpha, bool blending, bool depth_testing);
void material_set_unlit_emissive(material* mat_info, int texture, float alpha, float color_scale);
void material_set_unlit_color(material* mat_info, int texture, color *clr, bool blending, bool depth_testing);
void material_set_unlit_color(material* mat_info, int texture, color *clr, float alpha, bool blending, bool depth_testing);
void material_set_unlit_volume(particle_material* mat_info, int texture, bool point_sprites);
void material_set_distortion(distortion_material *mat_info, int texture, bool thruster);
void material_set_movie(movie_material *mat_info, int y_bm, int u_bm, int v_bm);

#endif

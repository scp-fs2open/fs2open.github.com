#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "graphics/grinternal.h"
#include "model/model.h"

struct uniform_data
{
	// when adding a new data type to support, create a vector for it here
	SCP_vector<int> int_data;
	SCP_vector<float> float_data;
	SCP_vector<vec2d> vec2_data;
	SCP_vector<vec3d> vec3_data;
	SCP_vector<vec4> vec4_data;
	SCP_vector<matrix4> matrix4_data;

	void clear()
	{
		int_data.clear();
		float_data.clear();
		vec2_data.clear();
		vec3_data.clear();
		vec4_data.clear();
		matrix4_data.clear();
	}

	template <class T> SCP_vector<T>& get_array();

	// add a single value to the data pool
	template <class T> int set_value(const T& val)
	{
		SCP_vector<T>& data = get_array<T>();

		data.push_back(val);

		return (int)data.size() - 1;
	}

	// overwrite an existing single value to the data pool
	template <class T> void set_value(int location, const T& val)
	{
		SCP_vector<T>& data = get_array<T>();

		Assert(location < (int)data.size());

		data[location] = val;
	}

	// add multiple values to the data pool
	template <class T> int set_values(T* val, int size)
	{
		SCP_vector<T>& data = get_array<T>();

		int start_index = data.size();

		for ( int i = 0; i < size; i++ ) {
			data.push_back(val[i]);
		}

		return start_index;
	}

	// overwrite multiple existing values to the data pool
	template <class T> void set_values(int location, T* val, int size)
	{
		SCP_vector<T>& data = get_array<T>();

		Assert(location < (int)data.size());

		for (int i = 0; i < size; i++) {
			// check to make sure we don't go out of bounds
			if ( location + i < (int)data.size() ) {
				data[location + i] = val[i];
			} else {
				data.push_back(val[i]);
			}
			
		}
	}

	template <class T> bool compare(int index, const T& val);
	template <class T> bool compare(int index, T* val, int count);
};

struct uniform
{
	SCP_string name;

	enum data_type {
		INT,
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		MATRIX4
	};

	data_type type;
	int index;

	int count;

	uniform_data *data_src;

	uniform(): data_src(NULL)
	{

	}

	uniform(uniform_data* _data_src):
		data_src(_data_src)
	{

	}

	void operator=(uniform& u)
	{
		name = u.name;
		type = u.type;
		index = u.index;
		count = u.count;
		data_src = u.data_src;
	}

	template <class T> static data_type determine_type();

	template <class T>
	void init(const SCP_string& init_name, const T& val)
	{
		name = init_name;
		type = determine_type<T>();
		count = 1;
		index = data_src->set_value(val);
	}

	template <class T>
	void init(const SCP_string &init_name, T* val, int size)
	{
		name = init_name;
		type = determine_type<T>();
		count = size;
		index = data_src->set_values(val, size);
	}

	template <class T>
	bool update(const T& val)
	{
		data_type uniform_type = determine_type<T>();

		if ( uniform_type == type ) {
			if ( count == 1 && data_src->compare(index, val) ) {
				return false;
			}

			count = 1;

			data_src->set_value(index, val);

			return true;
		}

		count = 1;
		uniform_type = type;
		index = data_src->set_value(val);

		return true;
	}

	template <class T>
	bool update(T* val, int size)
	{
        data_type uniform_type = uniform::determine_type<T>();

		if (uniform_type == type) {
			if (count == size && data_src->compare(index, val, size)) {
				return false;
			}

			if (count < size) {
				// we're going to overflow so append instead of replace
				count = size;
				index = data_src->set_values(val, size);
			} else {
				count = size;
				data_src->set_values(index, val, size);
			}

			return true;
		}

		count = size;
		uniform_type = type;
		index = data_src->set_values(val, size);

		return true;
	}
};

class uniform_block
{
	SCP_vector<uniform> Uniforms;

	uniform_data* Data_store;
	bool Local_data_store;

	int find_uniform(const SCP_string& name)
	{
		size_t count = Uniforms.size();

		for (size_t i = 0; i < count; ++i) {
			uniform& u = Uniforms[i];

			if (u.name == name) {
				return i;
			}
		}

		return -1;
	}
public:
	uniform_block(uniform_data* data_store = NULL)
	{
		if (Data_store != NULL) {
			Local_data_store = false;
		} else {
			// allocate our own data storage
			Data_store = new uniform_data;
			Local_data_store = true;
		}
	}

	~uniform_block()
	{
		if (Local_data_store && Data_store != NULL) {
			delete Data_store;
			Data_store = NULL;
		}
	}

	template <class T> bool set_value(const SCP_string& name, const T& val)
	{
		Assert(Data_store != NULL);

		int index = find_uniform(name);

		if (index >= 0) {
			return Uniforms[index].update(val);
		}

		uniform u(Data_store);

		u.init(name, val);

		Uniforms.push_back(u);

		return true;
	}

	template <class T> bool set_values(const SCP_string& name, T* val, int size)
	{
		Assert(Data_store != NULL);

		int index = find_uniform(name);

		if (index >= 0) {
			return Uniforms[index].update(val, size);
		}

		uniform u(Data_store);

		u.init(name, val, size);

		Uniforms.push_back(u);

		return true;
	}

	void invalidate_value(const SCP_string& name)
	{
		int index = find_uniform(name);

		if ( index < 0 ) {
			return;
		}

		Uniforms[index] = Uniforms.back();
		Uniforms.pop_back();
	}

	int num_uniforms() 
	{ 
		return (int)Uniforms.size(); 
	}

	uniform::data_type get_type(int i) 
	{
		Assert(i < (int)Uniforms.size());

		return Uniforms[i].type;
	}

	const SCP_string& get_name(int i)
	{
		Assert(i < (int)Uniforms.size());

		return Uniforms[i].name;
	}

	template <class T> T& get_value(int i)
	{
		int index = Uniforms[i].index;

		Assert(Data_store != NULL);
		Assert(Uniforms[i].type == uniform::determine_type<T>());

		SCP_vector<T>& data = Data_store->get_array<T>();

		return data[index];
	}

	template <class T> T& get_value(int i, int offset)
	{
		int index = Uniforms[i].index;

		Assert(Data_store != NULL);
		Assert(Uniforms[i].type == uniform::determine_type<T>());
		Assert(offset < Uniforms[i].count);

		SCP_vector<T>& data = Data_store->get_array<T>();

		Assert(index + offset < (int)data.size());

		return data[index + offset];
	}
};

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
	int Sdr_handle;

	int Texture_maps[TM_NUM_TYPES];
	texture_type Tex_type;
	gr_texture_source Tex_source;

	clip_plane Clip_params;
	int Texture_addressing;
	fog Fog_params;
	gr_zbuffer_type Depth_mode;
	gr_alpha_blend Blend_mode;
	bool Cull_mode;
	int Fill_mode;
	color Clr;
	float Clr_scale;
	int Depth_bias;

protected:
	void set_shader_type(shader_type init_sdr_type = SDR_TYPE_NONE);

public:
	material();

	void set_shader_handle(int handle);
	virtual int get_shader_handle();

	void set_texture_map(int texture_type, int texture_num);
	int get_texture_map(int texture_type);
	bool is_textured();

	void set_texture_type(texture_type t_type);
	int get_texture_type();

	void set_texture_source(gr_texture_source source);
	gr_texture_source get_texture_source();

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
	color& get_color();

	void set_color_scale(float scale);
	float get_color_scale();
};

class model_material : public material
{
	bool Desaturate;

	bool Shadow_casting;
	bool Batched;

	bool Deferred;
	bool HDR;
	bool lighting;
	float Light_factor;

	int Center_alpha;

	int Animated_effect;
	float Animated_timer;

	float Thrust_scale;

	bool Team_color_set;
	team_color Tm_color;

	bool Normal_alpha;
	float Normal_alpha_min;
	float Normal_alpha_max;

	bool Normal_extrude;
	float Normal_extrude_width;

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

	void set_team_color(const team_color &color);
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

	uint get_shader_flags();
	virtual int get_shader_handle();
};

class particle_material : public material
{
	bool Point_sprite;
public:
	particle_material();

	void set_point_sprite_mode(bool enabled);
	bool get_point_sprite_mode();

	virtual int get_shader_handle();
};

class distortion_material: public material
{
	bool Thruster_render;
public:
	distortion_material();

	void set_thruster_rendering(bool enabled);
	bool get_thruster_rendering();

	virtual int get_shader_handle();
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

#endif
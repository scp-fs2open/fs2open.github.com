#pragma once

#include "globalincs/vmallocator.h"

//This file handles the lighting_profiles.tbl and -ltp.tbm files. The purpose of these files is to provide control over the HDR lighting pipeline and enviroment, and related matters.

//Tonemapping options, aside from the previous standard UC2, pulled from wookiejedi and qazwsxal's
//work on testing them for FSO, which itself was based on these references:
//https://64.github.io/tonemapping/ Delta - Blog by 64: Tone Mapping
//http://filmicworlds.com/blog/filmic-tonemapping-operators/ Filmic Worlds: Filmic Tonemapping Operators by John Hable

enum TonemapperAlgorithm :int {
	tnm_Invalid = -1,
	tnm_Linear = 0,
	tnm_Uncharted = 1,
	tnm_Aces = 2,
	tnm_Aces_Approx = 3,
	tnm_Cineon = 4,
	tnm_Reinhard_Jodie = 5,
	tnm_Reinhard_Extended = 6,
	tnm_PPC = 7,
	tnm_PPC_RGB= 8
};

struct piecewise_power_curve_values{
	float toe_strength;
	float toe_length;
	float shoulder_strength;
	float shoulder_length;
	float shoulder_angle;
};

struct piecewise_power_curve_intermediates{
	float x0;
	float y0;
	float x1;
	float toe_B;
	float toe_lnA;
	float sh_B;
	float sh_lnA;
	float sh_offsetX;
	float sh_offsetY;
};

/**
 * @brief To hold configuration of, and handle application of, modifiers to lighting values.
 */
struct lighting_profile_value{
	float base;
	bool only_positive = true;

	float handle(float input);
	void reset();
	void set_adjust(const float in);
	void set_multiplier(const float in);
	void stack_multiplier(const float in);
	void set_maximum(const float in);
	void set_minimum(const float in);
	void stack_minimum(const float in);

	static bool parse(const char *filename, const char * valuename, const SCP_string &profile_name, lighting_profile_value* value_target, bool required=false);
	bool read_multiplier(float *out) const;
	bool read_adjust(float *out) const;

private:
	bool has_adjust = false;
	float adjust;
	bool has_multiplier=false;
	float multipier;
	bool has_minimum = false;
	float minimum;
	bool has_maximum = false;
	float maximum;
};

class lighting_profile{
public:
	static lighting_profile * current();
	static enum TonemapperAlgorithm name_to_tonemapper(SCP_string &name);
	static void load_profiles();
	static TonemapperAlgorithm current_tonemapper();
	static const piecewise_power_curve_values & current_piecewise_values();
	static piecewise_power_curve_intermediates current_piecewise_intermediates();
	static piecewise_power_curve_intermediates calc_intermediates(piecewise_power_curve_values input);
	static float current_exposure();
	static void lab_set_exposure(float exIn);
	static void lab_set_tonemapper(TonemapperAlgorithm tnin);
	static void lab_set_ppc(piecewise_power_curve_values ppcin );
	static piecewise_power_curve_values lab_get_ppc();
	static float lab_get_light();
	static void lab_set_light(float in);
	static float lab_get_ambient();
	static void lab_set_ambient(float in);
	static float lab_get_emissive();
	static void lab_set_emissive(float in);

	SCP_string name;
    TonemapperAlgorithm tonemapper;
	piecewise_power_curve_values ppc_values;
	float exposure;
	lighting_profile_value missile_light_brightness;
	lighting_profile_value missile_light_radius;
	lighting_profile_value laser_light_brightness;
	lighting_profile_value laser_light_radius;
	lighting_profile_value beam_light_brightness;
	lighting_profile_value beam_light_radius;

	lighting_profile_value tube_light_brightness;
	lighting_profile_value tube_light_radius;
	lighting_profile_value point_light_brightness;
	lighting_profile_value point_light_radius;
	lighting_profile_value cone_light_brightness;
	lighting_profile_value cone_light_radius;
	lighting_profile_value directional_light_brightness;
	lighting_profile_value ambient_light_brightness;
	//Strictly speaking this should be handled by postproc but we need something for the non-postproc people.
	lighting_profile_value overall_brightness;
	lighting_profile_value cockpit_light_radius_modifier;

    void reset();

private:

	static lighting_profile default_profile;
	static void parse_all();
	static void parse_file(const char *filename);
	static void parse_default_section(const char *filename);
};

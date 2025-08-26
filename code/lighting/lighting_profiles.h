// clang-format on
#pragma once

#include "globalincs/adjustment.h"
#include "globalincs/vmallocator.h"


namespace lighting_profiles {
// This file handles the profiles.tbl and -ltp.tbm files. The purpose of these files is to provide control over the HDR
// lighting pipeline and enviroment, and related matters.

// Tonemapping options, aside from the previous standard UC2, pulled from wookiejedi and qazwsxal's
// work on testing them for FSO, which itself was based on these references:
// https://64.github.io/tonemapping/ Delta - Blog by 64: Tone Mapping
// http://filmicworlds.com/blog/filmic-tonemapping-operators/ Filmic Worlds: Filmic Tonemapping Operators by John Hable

enum class TonemapperAlgorithm : int
{
	Invalid = -1,
	Linear = 0,
	Uncharted = 1,
	Aces = 2,
	Aces_Approx = 3,
	Cineon = 4,
	Reinhard_Jodie = 5,
	Reinhard_Extended = 6,
	PPC = 7,
	PPC_RGB = 8
};

struct piecewise_power_curve_values {
	float toe_strength;
	float toe_length;
	float shoulder_strength;
	float shoulder_length;
	float shoulder_angle;
};

struct piecewise_power_curve_intermediates {
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

class profile {
  public:
	SCP_string name;
	TonemapperAlgorithm tonemapper;
	piecewise_power_curve_values ppc_values;
	float exposure;
	adjustment missile_light_brightness;
	adjustment missile_light_radius;
	adjustment laser_light_brightness;
	adjustment laser_light_radius;
	adjustment beam_light_brightness;
	adjustment beam_light_radius;

	adjustment tube_light_brightness;
	adjustment tube_light_radius;
	adjustment point_light_brightness;
	adjustment point_light_radius;
	adjustment cone_light_brightness;
	adjustment cone_light_radius;
	adjustment directional_light_brightness;
	adjustment ambient_light_brightness;
	// Strictly speaking this should be handled by postproc but we need something for the non-postproc people.
	adjustment overall_brightness;
	adjustment cockpit_light_radius_modifier;
	adjustment cockpit_light_intensity_modifier;

	void reset();
	profile& operator=(const profile& rhs);
	void parse(const char* filename, const SCP_string& profile_name, const SCP_string& end_tag);

  private:
};

const SCP_string &default_name();
const profile* current();
enum TonemapperAlgorithm name_to_tonemapper(SCP_string name);
SCP_string tonemapper_to_name(TonemapperAlgorithm tnm);
void load_profiles();
TonemapperAlgorithm current_tonemapper();
const piecewise_power_curve_values& current_piecewise_values();
piecewise_power_curve_intermediates current_piecewise_intermediates();
piecewise_power_curve_intermediates calc_intermediates(piecewise_power_curve_values input);
float current_exposure();
void lab_set_exposure(float exIn);
void lab_set_tonemapper(TonemapperAlgorithm tnin);
void lab_set_ppc(const piecewise_power_curve_values &ppcin);
const piecewise_power_curve_values &lab_get_ppc();
float lab_get_light();
void lab_set_light(float in);
float lab_get_ambient();
void lab_set_ambient(float in);
float lab_get_emissive();
void lab_set_emissive(float in);
SCP_vector<SCP_string> list_profiles();
void switch_to(const SCP_string& name);
} // namespace lighting_profiles
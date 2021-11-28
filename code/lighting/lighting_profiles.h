#pragma once

#include "globalincs/vmallocator.h"

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

class lighting_profile{
public:
	static enum TonemapperAlgorithm name_to_tonemapper(SCP_string &name);
	static void load_profiles();
	static TonemapperAlgorithm current_tonemapper();
	static piecewise_power_curve_values current_piecewise_values();
	static float current_exposure();

	SCP_string name;
    TonemapperAlgorithm tonemapper;
	piecewise_power_curve_values ppc_values;
	float exposure;

    void reset();

private:
	static lighting_profile default_profile;
	static void parse_all();
	static void parse_file(const char *filename);
	static void parse_default_section();
	static void add_default_default();
};

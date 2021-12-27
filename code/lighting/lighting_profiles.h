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

class lighting_profile{
public:
	static enum TonemapperAlgorithm name_to_tonemapper(SCP_string &name);
	static void load_profiles();
	static TonemapperAlgorithm current_tonemapper();
	static const piecewise_power_curve_values & current_piecewise_values();
	static piecewise_power_curve_intermediates current_piecewise_intermediates();
	static piecewise_power_curve_intermediates calc_intermediates(piecewise_power_curve_values input);
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
};

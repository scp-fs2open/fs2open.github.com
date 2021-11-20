

#pragma once


#include "globalincs/vmallocator.h"
#include <vcruntime.h>
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
struct table_line{
	SCP_string entity_name;
	SCP_string value_name;
	SCP_string line_data;
	SCP_string blame;
	bool set = false;
};

struct om_table_line{
	SCP_string line_data;
	SCP_string blame;
	bool set = false;
};

struct piecewise_power_curve_values{
	float toe_strength;
	float toe_length;
	float shoulder_strength; 
	float shoulder_length;
	float shoulder_angle;
};
class light_profile{
public:
	static void parse_all();
	static void parse_file(const char *filename);
	static void parse_profile(const char *blame);
	static bool parse_nextline(const char *blame, SCP_string *const profile_name);
	
	SCP_string name;

    TonemapperAlgorithm tonemapper;
	piecewise_power_curve_values ppc_values;
	float exposure;

    void reset();
	static void add_default_default();
	static void load_profiles();
};

class light_profile_manager{
	public:
	static TonemapperAlgorithm tonemapper();
	static piecewise_power_curve_values piecewise_values();
	static float exposure();
	static enum TonemapperAlgorithm name_to_tonemapper(SCP_string &name);
};
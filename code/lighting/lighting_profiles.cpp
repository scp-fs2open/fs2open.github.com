#include "globalincs/pstypes.h"
#include "globalincs/safe_strings.h"
#include "globalincs/vmallocator.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "parse/parselo.h"

//TODO: maybe a parsehi.cpp would be in order here?
bool optional_parse_into_float(const SCP_string &fieldname, float* valuetarget)
{
	if(optional_string(fieldname.c_str())){
		stuff_float(valuetarget);
		return true;
	}
	return false;
}

void lighting_profile::reset()
{
	name = "";

    tonemapper = tnm_Uncharted;

	ppc_values.toe_strength = 0.5f;
    ppc_values.toe_length = 0.5f;
    ppc_values.shoulder_strength = 0.0f;
    ppc_values.shoulder_length = 0.5f;
    ppc_values.shoulder_angle = 0.1f;

	exposure = 4.0f;
}

TonemapperAlgorithm lighting_profile::name_to_tonemapper(SCP_string &name)
{
	SCP_tolower(name);
	TonemapperAlgorithm r = tnm_Invalid;
	if(name == "uncharted" || name == "uncharted 2" ){
		r = tnm_Uncharted;
	}
	if(name == "linear"){
		r = tnm_Linear;
	}
	if(name == "aces"){
		r = tnm_Aces;
	}
	if(name == "aces approximate"){
		r = tnm_Aces_Approx;
	}
	if(name == "cineon"){
		r = tnm_Cineon;
	}
	if(name == "reinhard jodie"){
		r = tnm_Reinhard_Jodie;
	}
	if(name == "reinhard extended"){
		r = tnm_Reinhard_Extended;
	}
	if(name == "PPC"){
		r = tnm_PPC;
	}
	if(name == "PPC RGB"){
		r = tnm_PPC_RGB;
	}
	return r;
}

lighting_profile lighting_profile::default_profile;

void lighting_profile::load_profiles()
{
	parse_all();
}

//The logic for grabbing all the parseable files

void lighting_profile::parse_all()
{
	default_profile.reset();
	if (cf_exists_full("lighting_profiles.tbl", CF_TYPE_TABLES)){
		mprintf(("TABLES:Starting parse of lighting profiles.tbl"));
		lighting_profile::parse_file("lighting_profiles.tbl");
	}

	mprintf(("TBM  =>  Starting parse of lighting profiles ...\n"));
	parse_modular_table("*-ltp.tbm", lighting_profile::parse_file);
}

void lighting_profile::parse_default_section()
{
	bool keep_going = true;
	SCP_string buffer;
	TonemapperAlgorithm tn;
	while(!optional_string("#END DEFAULT PROFILE") && keep_going){
		keep_going = false;
		if(optional_string("$Tonemapper:")){
			stuff_string(buffer,F_NAME);
			tn = lighting_profile::name_to_tonemapper(buffer);
			default_profile.tonemapper = tn;
			keep_going = true;
		}
		keep_going |= optional_parse_into_float("$PPC Toe Strength:",&default_profile.ppc_values.toe_strength);
		keep_going |= optional_parse_into_float("$PPC Toe Length:",&default_profile.ppc_values.toe_length);
		keep_going |= optional_parse_into_float("$PPC Shoulder Length:",&default_profile.ppc_values.shoulder_length);
		keep_going |= optional_parse_into_float("$PPC Shoulder Strength:",&default_profile.ppc_values.shoulder_strength);
		keep_going |= optional_parse_into_float("$PPC Toe Strength:",&default_profile.ppc_values.shoulder_angle);
		keep_going |= optional_parse_into_float("$Exposure:",&default_profile.exposure);
		//TODO: Handle case when there's no line matched but we haven't hit an #end
		Assert(keep_going);
	}
}

//Handle an individual file.
void lighting_profile::parse_file(const char *filename)
{
	try
	{
		if (filename == nullptr){
			//All defaults currently handled by lighting_profile.reset()
			return;
		}
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();
		if(optional_string("#DEFAULT PROFILE")){
			parse_default_section();
		}
    }	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", (filename) ? filename : "<default ai_profiles.tbl>", e.what()));
		return;
	}
}


//these accessor stubs are futureproofing abstraction


TonemapperAlgorithm lighting_profile::current_tonemapper()
{
	return default_profile.tonemapper;
}

piecewise_power_curve_values lighting_profile::current_piecewise_values()
{
	return default_profile.ppc_values;
}

float lighting_profile::current_exposure()
{
	return default_profile.exposure;
}
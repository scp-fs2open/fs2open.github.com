#include "globalincs/pstypes.h"
#include "globalincs/safe_strings.h"
#include "globalincs/vmallocator.h"
#include "def_files/def_files.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "osapi/dialogs.h"
#include "parse/parsehi.h"
#include "parse/parselo.h"

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
	if(name == "ppc"){
		r = tnm_PPC;
	}
	if(name == "ppc rgb"){
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
	bool parsed;
	SCP_string buffer;
	TonemapperAlgorithm tn;
	while(!optional_string("#END DEFAULT PROFILE")){
		parsed = false;
		if(optional_string("$Tonemapper:")){
			stuff_string(buffer,F_NAME);
			tn = lighting_profile::name_to_tonemapper(buffer);
			default_profile.tonemapper = tn;
			parsed = true;
		}
		parsed |= parse_optional_float_into("$PPC Toe Strength:",&default_profile.ppc_values.toe_strength);
		parsed |= parse_optional_float_into("$PPC Toe Length:",&default_profile.ppc_values.toe_length);
		parsed |= parse_optional_float_into("$PPC Shoulder Length:",&default_profile.ppc_values.shoulder_length);
		parsed |= parse_optional_float_into("$PPC Shoulder Strength:",&default_profile.ppc_values.shoulder_strength);
		parsed |= parse_optional_float_into("$PPC Shoulder Angle:",&default_profile.ppc_values.shoulder_angle);
		parsed |= parse_optional_float_into("$Exposure:",&default_profile.exposure);
		if(!parsed){
			stuff_string(buffer,F_RAW);
			Warning(LOCATION,"Unhandled line in lighting profile\n\t%s",buffer.c_str());
		}
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

const piecewise_power_curve_values & lighting_profile::current_piecewise_values()
{
	return default_profile.ppc_values;
}

float lighting_profile::current_exposure()
{
	return default_profile.exposure;
}
piecewise_power_curve_intermediates lighting_profile::current_piecewise_intermediates(){
	return calc_intermediates(default_profile.ppc_values);
}
piecewise_power_curve_intermediates lighting_profile::calc_intermediates(piecewise_power_curve_values input){

	piecewise_power_curve_intermediates ppci;

	//Some safety clamping based on John Hable's implementation: https://github.com/johnhable/fw-public
	CLAMP(input.toe_length, 0.0f, 1.0f);
	CLAMP(input.toe_strength, 0.0f, 1.0f);
	CLAMP(input.shoulder_angle, 0.0f, 1.0f);
	CLAMP(input.shoulder_length, 0.0f, 1.0f);
	input.shoulder_strength = fmax(0.0f, input.shoulder_strength);

	ppci.x0 = input.toe_length * 0.5f; //L,F,P
	ppci.y0 = (1.0f - input.toe_strength) * ppci.x0; //L
	float remainingY = 1.0f - ppci.y0; 
	float initialW = ppci.x0 + remainingY;
	float y1_offset = (1.0f - input.shoulder_length) * remainingY;
	ppci.x1 = ppci.x0 + y1_offset; //F,P
	float y1 = ppci.y0 + y1_offset;
	float extraW = exp2f(input.shoulder_length) - 1.0f;
	float W = initialW + extraW;
	float overshootX = (W * 2.0f) * input.shoulder_strength + (ppci.x0-ppci.y0);
	float overshootY = 0.5f * input.shoulder_angle;

	ppci.toe_B = ppci.x0/ppci.y0; //T
	ppci.toe_lnA = log(ppci.y0) - ppci.toe_B*log(ppci.x0);//T

	float sh_x0 = (1.0f + overshootX) - ppci.x1;
	float sh_y0 = (1.0f + overshootY) - y1;

	ppci.sh_B = sh_x0/sh_y0;//S
	ppci.sh_lnA = log(sh_y0) - ppci.sh_B*log(sh_x0);//S
	ppci.sh_offsetX = 1.0f + overshootX; //F,P,S
	ppci.sh_offsetY = 1.0f + overshootY; //F,P,S
	return ppci;

}
void lighting_profile::lab_set_exposure(float exIn){
	default_profile.exposure = exIn;
}


void lighting_profile::lab_set_tonemapper(TonemapperAlgorithm tnin){
	default_profile.tonemapper = tnin;
}

void lighting_profile::lab_set_ppc(piecewise_power_curve_values ppcin ){
	default_profile.ppc_values = ppcin;

}

piecewise_power_curve_values lighting_profile::lab_get_ppc(){
	return default_profile.ppc_values;
}
#include "globalincs/pstypes.h"
#include "globalincs/safe_strings.h"
#include "globalincs/vmallocator.h"
#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "osapi/dialogs.h"
#include "parse/parsehi.h"
#include "parse/parselo.h"

//*************************************************
//			lighting_profile_value funcs
//*************************************************

/**
 * @brief Processes an input according to the user's configuration and returns the result.
 */
float lighting_profile_value::handle(float input)
{
	if (only_positive && input < 0.0f)
		input = base;
	if (has_multiplier)
		input *= multipier;
	//handling adjust after multiplier makes it possible to multiply by 0 and still adjust.
	if (has_adjust)
		input += adjust;
	if (has_minimum)
		input = MAX(input, minimum);
	if (has_maximum)
		input = MIN(input, maximum);
	return input;
}

void lighting_profile_value::reset()
{
	base = 1.0f;
	has_adjust = false;
	has_minimum = false;
	has_maximum = false;
	has_multiplier = false;
	only_positive = true;
}

void lighting_profile_value::set_adjust(float in)
{
	has_adjust = true;
	adjust = in;
}

void lighting_profile_value::set_multiplier(float in)
{
	has_multiplier = true;
	multipier = in;
}

void lighting_profile_value::stack_multiplier(float in)
{
	if(has_multiplier)
	{
		multipier*= in;
	}
	else 
	{
		multipier = in;
		has_multiplier = true;
	}
}

void lighting_profile_value::set_maximum(float in)
{
	has_maximum = true;
	maximum = in;
}

void lighting_profile_value::set_minimum(float in)
{
	has_minimum = true;
	minimum = in;
}

void lighting_profile_value::stack_minimum(float in)
{
	if(has_minimum)
	{
		minimum = MAX(minimum,in);
	}
	else
	{
		minimum = in;
		has_minimum = true;
	}
}
bool lighting_profile_value::read_adjust(float *out) const
{
	*out = adjust;
	return has_adjust;
}
/**
 * @brief for use during the parsing of a light profile to attempt to read in an LPV
 *
 * @param filename for error reporting primairly
 * @param valuename Text of the field 
 * @param profile_name The name of the profile being parsed, for error reporting
 * @param value_target The profile object parsing into
 * @param required If false it is an error for this to not find a value.
 * @return true if a succesful parse, false otherwise
 */
bool lighting_profile_value::parse(const char* filename,
	const char* valuename,
	const SCP_string& profile_name,
	lighting_profile_value* value_target,
	bool required)
{
	if (optional_string(valuename)) {
		bool parsed = true;
		int parses = 0;
		while (parsed) {
			parsed = false;
			if (parse_optional_float_into("+default:", &value_target->base)) {
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+maximum:", &value_target->maximum)) {
				value_target->has_maximum = true;
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+minimum:", &value_target->minimum)) {
				value_target->has_minimum = true;
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+multiplier:", &value_target->multipier)) {
				value_target->has_multiplier = true;
				parsed = true;
				parses++;
			}
			if (parse_optional_float_into("+adjust:", &value_target->adjust)) {
				value_target->has_adjust = true;
				parsed = true;
				parses++;
			}
		}
		if (parses == 0) {
			Warning(LOCATION,
				"Lighting profile value '%s' in file '%s' profile '%s' parsed but set no properties, possible "
				"malformed table.",
				valuename,
				filename,
				profile_name.c_str());
		} else if (parses > 5) {
			Warning(LOCATION,
				"Lighting profile value '%s' in file '%s' profile '%s' parsed too many properties, possible "
				"malformed table.",
				valuename,
				filename,
				profile_name.c_str());
		}
		return true;

	} else if (required) {
		Error(LOCATION,
			"Expected lightingprofile value '%s' in file '%s' profile '%s' not found",
			valuename,
			filename,
			profile_name.c_str());
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

	missile_light_brightness.reset();
	missile_light_radius.reset();
	missile_light_radius.base = 20.0f;

	laser_light_brightness.reset();
	laser_light_radius.reset();
	laser_light_radius.base = 100.0f;

	beam_light_brightness.reset();
	beam_light_radius.reset();
	beam_light_radius.base = 37.5f;


	tube_light_brightness.reset();
	tube_light_radius.reset();

	point_light_brightness.reset();
	point_light_radius.reset();
	point_light_radius.set_multiplier(1.25f);

	cone_light_brightness.reset();
	cone_light_radius.reset();
	cone_light_radius.set_multiplier(1.25f);

	directional_light_brightness.reset();


	overall_brightness.reset();
	//Default Multiplier to maintain legacy visual compatibility
	overall_brightness.set_multiplier(3.5f); 
	overall_brightness.set_minimum(0.0f);
	overall_brightness.stack_multiplier(Cmdline_light_power);

	ambient_light_brightness.reset();
	//Inverse of the overall brightness boost
	ambient_light_brightness.set_multiplier(0.286f); 
	ambient_light_brightness.stack_multiplier(Cmdline_ambient_power);
	ambient_light_brightness.set_adjust(MAX(0.0f,Cmdline_emissive_power));

	cockpit_light_radius_modifier.reset();
	cockpit_light_radius_modifier.set_multiplier(1.0f);
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

lighting_profile * lighting_profile::current(){
	//in time this may host more complex logic to allow such things as blending between two different profiles and managing a temporary profile.
	//put in place now so that access can be generally future-proofed from here out.
	return &lighting_profile::default_profile;
}

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

void lighting_profile::parse_default_section(const char *filename)
{
	bool parsed;
	SCP_string buffer;
	TonemapperAlgorithm tn;
	const char* profile_name = "Default Profile";
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

		parsed |= lighting_profile_value::parse(filename,"$Missile light brightness:",profile_name,
										&default_profile.missile_light_brightness);
		parsed |= lighting_profile_value::parse(filename,"$Missile light radius:",profile_name,
										&default_profile.missile_light_radius);

		parsed |= lighting_profile_value::parse(filename,"$Laser light brightness:",profile_name,
										&default_profile.laser_light_brightness);
		parsed |= lighting_profile_value::parse(filename,"$Laser light radius:",profile_name,
										&default_profile.laser_light_radius);

		parsed |= lighting_profile_value::parse(filename,"$Beam light brightness:",profile_name,
										&default_profile.beam_light_brightness);
		parsed |= lighting_profile_value::parse(filename,"$Beam light radius:",profile_name,
										&default_profile.beam_light_radius);

		parsed |= lighting_profile_value::parse(filename,"$Tube light brightness:",profile_name,
										&default_profile.tube_light_brightness);
		parsed |= lighting_profile_value::parse(filename,"$Tube light radius:",profile_name,
										&default_profile.tube_light_radius);

		parsed |= lighting_profile_value::parse(filename,"$Point light brightness:",profile_name,
										&default_profile.point_light_brightness);
		parsed |= lighting_profile_value::parse(filename,"$Point light radius:",profile_name,
										&default_profile.point_light_radius);
		parsed |= lighting_profile_value::parse(filename,"$Directional light brightness:",profile_name,
										&default_profile.directional_light_brightness);
		if(lighting_profile_value::parse(filename,"$Ambient light brightness:",profile_name, &default_profile.ambient_light_brightness))
		{
			parsed = true;
			default_profile.ambient_light_brightness.stack_multiplier(Cmdline_ambient_power);
			default_profile.ambient_light_brightness.stack_multiplier(Cmdline_light_power);
			default_profile.ambient_light_brightness.stack_minimum(Cmdline_emissive_power);
		}

		if(lighting_profile_value::parse(filename,"$Overall brightness:",profile_name, &default_profile.overall_brightness))
		{
			parsed = true;
			default_profile.overall_brightness.stack_multiplier(Cmdline_light_power);
		}

		parsed |= parse_optional_float_into("$Exposure:",&default_profile.exposure);

		parsed |= lighting_profile_value::parse(filename, "$Cockpit light radius modifier:", profile_name,
			&default_profile.cockpit_light_radius_modifier);

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
			parse_default_section(filename);
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

float lighting_profile::lab_get_light(){
	return default_profile.overall_brightness.handle(1.0f);
}
float lighting_profile::lab_get_ambient(){
	return default_profile.ambient_light_brightness.handle(1.0f);
}
float lighting_profile::lab_get_emissive(){
	float r;
	default_profile.ambient_light_brightness.read_adjust(&r);

	return r;
}
void lighting_profile::lab_set_light(float in){
	default_profile.overall_brightness.set_multiplier(in);
}
void lighting_profile::lab_set_ambient(float in){
	default_profile.ambient_light_brightness.set_multiplier(in);
}
void lighting_profile::lab_set_emissive(float in){
	default_profile.ambient_light_brightness.set_adjust(MAX(0.0f,in));
}
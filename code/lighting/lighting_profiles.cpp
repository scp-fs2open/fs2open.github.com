#include "def_files/def_files.h"
#include "globalincs/pstypes.h"
#include "globalincs/safe_strings.h"
#include "globalincs/vmallocator.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "osapi/dialogs.h"
#include "parse/parselo.h"


bool optional_parse_into_float(const SCP_string &fieldname, float* valuetarget){
	if(optional_string(fieldname.c_str())){
		stuff_float(valuetarget);
		return true;
	}
	return false;
}

void light_profile::reset(){

    static_light_factor = 1.0f;
    static_tube_factor = 1.0f;
    static_point_factor = 1.0f;
    tonemapper = tnm_Uncharted;

	ppc_values.toe_strength = 0.5f;
    ppc_values.toe_length = 0.5f;
    ppc_values.shoulder_strength = 0.0f;
    ppc_values.shoulder_length = 0.5f;
    ppc_values.shoulder_angle = 0.1f;
	exposure = 4.0f;
}

TonemapperAlgorithm light_profile_manager::name_to_tonemapper(SCP_string &name){
	SCP_tolower(name);
	TonemapperAlgorithm r = tnm_Invalid;
	if(name == "uncharted" || name == "uncharted 2" ){
		r = tnm_Uncharted;
	}
	if(name == "linear"){
		r = tnm_Invalid;
	}
	if(name == "aces"){
		r = tnm_Invalid;
	}
	if(name == "aces approximate"){
		r = tnm_Invalid;
	}
	if(name == "cineon"){
		r = tnm_Invalid;
	}
	if(name == "cineon"){
		r = tnm_Invalid;
	}
	if(name == "reinhard jodie"){
		r = tnm_Invalid;
	}
	if(name == "reinhard extended"){
		r = tnm_Invalid;
	}
	if(name == "PPC"){
		r = tnm_Invalid;
	}
	if(name == "PPC RGB"){
		r = tnm_Invalid;
	}
	return r;
}

light_profile default_profile;
void light_profile::load_profiles(){
	parse_all();
}

//The logic for grabbing all the parseable files

void light_profile::parse_all(){
	default_profile.reset();
	if (cf_exists_full("lighting_profiles.tbl", CF_TYPE_TABLES)){
		mprintf(("TABLES:Starting parse of lighting profiles.tbl"));
		light_profile::parse_file("lighting_profiles.tbl");
	}
	
	mprintf(("TBM  =>  Starting parse of lighting profiles ...\n"));
	parse_modular_table("*-ltp.tbm", light_profile::parse_file);

}

void parse_default_section(const char *filename){
	bool keep_going = true;
	SCP_string buffer;
	TonemapperAlgorithm tn;
	while(!optional_string("#END DEFAULT PROFILE") && keep_going){
		keep_going = false;
		if(optional_string("$Tonemapper:")){
			stuff_string(buffer,F_NAME);
			tn = light_profile_manager::name_to_tonemapper(buffer);
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
		Assert(keep_going == true);
	}
}
//Handle an individual file.
void light_profile::parse_file(const char *filename)
{
	try
	{
		if (filename == nullptr){
			//Normal parsing uses this:
			//read_file_text_from_default(defaults_get_file("lighting_profiles.tbl"));
			//To set the defaults, I intend to set them as virtual table lines before here
			//so, if no file, gtfo.
			return;
		}
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();
		bool keep_going = true;
		while(check_for_eof()==0 && keep_going){
			if(optional_string("#DEFAULT PROFILE")){
				keep_going = true;
				parse_default_section(filename);
			}
			//TODO: Handle case when there's no line matched but we haven't hit an #end
			if(keep_going == false)
			{
				advance_to_eoln(nullptr);
				keep_going = true;
			}
		}
    }	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", (filename) ? filename : "<default ai_profiles.tbl>", e.what()));
		return;
	}
}


//these accessor stubs are futureproofing abstraction


TonemapperAlgorithm light_profile_manager::tonemapper(){
	return default_profile.tonemapper;
}

piecewise_power_curve_values light_profile_manager::piecewise_values(){
	return default_profile.ppc_values;
}
float light_profile_manager::exposure(){
	return default_profile.exposure;
}
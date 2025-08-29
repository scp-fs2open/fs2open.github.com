// clang-format on
#include "lighting/lighting_profiles.h"

#include "globalincs/adjustment.h"
#include "globalincs/pstypes.h"
#include "globalincs/safe_strings.h"
#include "globalincs/vmallocator.h"

#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "io/timer.h"
#include "lighting/lighting.h"
#include "osapi/dialogs.h"
#include "parse/parsehi.h"
#include "parse/parselo.h"

namespace lighting_profiles {
typedef int profile_index;
//*************************************************
//			profile management
//*************************************************
//TIMESTAMP switch_start;
//TIMESTAMP switch_end;

profile _current;
SCP_unordered_map<SCP_string, profile> Profiles;
SCP_string default_profile_name;

const SCP_string &default_name()
{
	return default_profile_name;
}

const profile* current()
{
	return &_current;
}

SCP_vector<SCP_string> list_profiles()
{
	SCP_vector<SCP_string> r;
	for (auto& p : Profiles) {
		r.push_back(p.second.name);
	}
	return r;
}

void switch_to(const SCP_string& name)
{
	if (Profiles.find(name) == Profiles.end()) {
		Warning(LOCATION, "Attempted to switch to unknown lighting profile '%s';", name.c_str());
		return;
	}
	_current = Profiles[name];
}

void update_current_profile()
{
	// processes any change-overs and transitions
}

//*************************************************
//			General interface
//*************************************************
TonemapperAlgorithm current_tonemapper()
{
	return _current.tonemapper;
}

const piecewise_power_curve_values& current_piecewise_values()
{
	return _current.ppc_values;
}

float current_exposure()
{
	return _current.exposure;
}

piecewise_power_curve_intermediates current_piecewise_intermediates()
{
	return calc_intermediates(_current.ppc_values);
}

piecewise_power_curve_intermediates calc_intermediates(piecewise_power_curve_values input)
{
	piecewise_power_curve_intermediates ppci;

	// Some safety clamping based on John Hable's implementation: https://github.com/johnhable/fw-public
	CLAMP(input.toe_length, 0.0f, 1.0f);
	CLAMP(input.toe_strength, 0.0f, 1.0f);
	CLAMP(input.shoulder_angle, 0.0f, 1.0f);
	CLAMP(input.shoulder_length, 0.0f, 1.0f);
	input.shoulder_strength = fmax(0.0f, input.shoulder_strength);

	ppci.x0 = input.toe_length * 0.5f;               // L,F,P
	ppci.y0 = (1.0f - input.toe_strength) * ppci.x0; // L
	float remainingY = 1.0f - ppci.y0;
	float initialW = ppci.x0 + remainingY;
	float y1_offset = (1.0f - input.shoulder_length) * remainingY;
	ppci.x1 = ppci.x0 + y1_offset; // F,P
	float y1 = ppci.y0 + y1_offset;
	float extraW = exp2f(input.shoulder_length) - 1.0f;
	float W = initialW + extraW;
	float overshootX = (W * 2.0f) * input.shoulder_strength + (ppci.x0 - ppci.y0);
	float overshootY = 0.5f * input.shoulder_angle;

	ppci.toe_B = ppci.x0 / ppci.y0;                          // T
	ppci.toe_lnA = log(ppci.y0) - ppci.toe_B * log(ppci.x0); // T

	float sh_x0 = (1.0f + overshootX) - ppci.x1;
	float sh_y0 = (1.0f + overshootY) - y1;

	ppci.sh_B = sh_x0 / sh_y0;                         // S
	ppci.sh_lnA = log(sh_y0) - ppci.sh_B * log(sh_x0); // S
	ppci.sh_offsetX = 1.0f + overshootX;               // F,P,S
	ppci.sh_offsetY = 1.0f + overshootY;               // F,P,S
	return ppci;
}

TonemapperAlgorithm name_to_tonemapper(SCP_string name)
{
	SCP_tolower(name);
	TonemapperAlgorithm r = TonemapperAlgorithm::Invalid;
	if (name == "uncharted" || name == "uncharted 2") {
		r = TonemapperAlgorithm::Uncharted;
	}
	if (name == "linear") {
		r = TonemapperAlgorithm::Linear;
	}
	if (name == "aces") {
		r = TonemapperAlgorithm::Aces;
	}
	if (name == "aces approximate") {
		r = TonemapperAlgorithm::Aces_Approx;
	}
	if (name == "cineon") {
		r = TonemapperAlgorithm::Cineon;
	}
	if (name == "reinhard jodie") {
		r = TonemapperAlgorithm::Reinhard_Jodie;
	}
	if (name == "reinhard extended") {
		r = TonemapperAlgorithm::Reinhard_Extended;
	}
	if (name == "ppc" || name == "piecewise power curve") {
		r = TonemapperAlgorithm::PPC;
	}
	if (name == "ppc rgb" || name == "piecewise power curve (rgb)") {
		r = TonemapperAlgorithm::PPC_RGB;
	}
	return r;
}

SCP_string tonemapper_to_name(TonemapperAlgorithm tnm)
{
	switch (tnm) {
	case TonemapperAlgorithm::Aces:
		return "ACES";
	case TonemapperAlgorithm::Aces_Approx:
		return "ACES Approximate";
	case TonemapperAlgorithm::Cineon:
		return "Cineon";
	case TonemapperAlgorithm::Invalid:
		return "invalid";
	case TonemapperAlgorithm::Linear:
		return "Linear";
	case TonemapperAlgorithm::PPC:
		return "Piecewise Power Curve";
	case TonemapperAlgorithm::PPC_RGB:
		return "Piecewise Power Curve (RGB)";
	case TonemapperAlgorithm::Reinhard_Extended:
		return "Reinhard Extended";
	case TonemapperAlgorithm::Reinhard_Jodie:
		return "Reinhard Jodie";
	case TonemapperAlgorithm::Uncharted:
		return "Uncharted 2";
	default:
		return "<unknown algorithm>";
	}
}

//*************************************************
//			Lab access
//*************************************************

void lab_set_exposure(float exIn)
{
	_current.exposure = exIn;
}

void lab_set_tonemapper(TonemapperAlgorithm tnin)
{
	_current.tonemapper = tnin;
}

void lab_set_ppc(const piecewise_power_curve_values &ppcin)
{
	_current.ppc_values = ppcin;
}

const piecewise_power_curve_values &lab_get_ppc()
{
	return _current.ppc_values;
}

float lab_get_light()
{
	float r;
	_current.overall_brightness.read_multiplier(&r);

	return r;
}
float lab_get_ambient()
{
	float r;
	_current.ambient_light_brightness.read_multiplier(&r);

	return r;
}
float lab_get_emissive()
{
	float r;
	_current.ambient_light_brightness.read_adjust(&r);

	return r;
}
void lab_set_light(float in)
{
	_current.overall_brightness.set_multiplier(in);
}
void lab_set_ambient(float in)
{
	_current.ambient_light_brightness.set_multiplier(in);
}
void lab_set_emissive(float in)
{
	_current.ambient_light_brightness.set_adjust(MAX(0.0f, in));
}

//*************************************************
//			Parsing
//*************************************************

void parse_profile_section(const char* filename);
void parse_file(const char* filename);
void parse_default_section(const char* filename);
void parse_all();

void load_profiles()
{
	default_profile_name = "Default Profile";
	parse_all();
	switch_to(default_profile_name);
}
// The logic for grabbing all the parseable files

void parse_all()
{
	Profiles[default_profile_name] = profile();
	Profiles[default_profile_name].reset();
	//.reset() resets name to "" which makes sense in all cases but this one
	Profiles[default_profile_name].name = default_profile_name;
	_current.reset();
	if (cf_exists_full("lighting_profiles.tbl", CF_TYPE_TABLES)) {
		mprintf(("TABLES: Starting parse of lighting profiles.tbl\n"));
		parse_file("lighting_profiles.tbl");
	}

	mprintf(("TBM  =>  Starting parse of lighting profiles ...\n"));
	parse_modular_table("*-ltp.tbm", parse_file);
}

// Handle an individual file.
void parse_file(const char* filename)
{
	try {
		if (filename == nullptr) {
			// All defaults currently handled by profile.reset()
			return;
		}
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();
		int index = optional_string_one_of(4, "#DEFAULT PROFILE", "#PROFILES", "#END DEFAULT PROFILE", "#END PROFILES");
		while (index >= 0) {
			switch (index) {
			case 0: //#DEFAULT PROFILE
				parse_default_section(filename);
				break;
			case 1: //#PROFILES
				parse_profile_section(filename);
				break;
			}
			index = optional_string_one_of(4, "#DEFAULT PROFILE", "#PROFILES", "#END DEFAULT PROFILE", "#END PROFILES");
		}
	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n",
			(filename) ? filename : "<default lighting_profiles.tbl>",
			e.what()));
		return;
	}
}

void parse_default_section(const char* filename)
{
	SCP_string profile_name = default_profile_name;
	SCP_string endtag = "#END DEFAULT PROFILE";
	if (Profiles.find(profile_name) == Profiles.end()) {
		Profiles[profile_name] = profile();
	}
	profile* p = &Profiles.at(profile_name);
	p->parse(filename, profile_name, endtag);
}

void parse_profile_section(const char* filename)
{
	SCP_string buffer;
	while (optional_string("$Profile:")) {
		stuff_string(buffer, F_NAME);
		if (Profiles.find(buffer) == Profiles.end()) {
			Profiles[buffer] = profile();
			Profiles[buffer].reset();
		}
		profile* p = &Profiles.at(buffer);
		SCP_string endtag = "#END PROFILES";
		p->parse(filename, buffer, endtag);
	}
}

void profile::parse(const char* filename, const SCP_string& profile_name, const SCP_string& end_tag)
{
	name = profile_name;
	bool parsed;
	SCP_string buffer;

	while (0 > optional_string_either(end_tag.c_str(), "$Profile:", false)) {
		parsed = false;
		if (optional_string("$Tonemapper:")) {
			stuff_string(buffer, F_NAME);
			TonemapperAlgorithm tn = name_to_tonemapper(buffer);
			tonemapper = tn;
			parsed = true;
		}
		parsed |= parse_optional_float_into("$PPC Toe Strength:", &ppc_values.toe_strength);
		parsed |= parse_optional_float_into("$PPC Toe Length:", &ppc_values.toe_length);
		parsed |= parse_optional_float_into("$PPC Shoulder Length:", &ppc_values.shoulder_length);
		parsed |= parse_optional_float_into("$PPC Shoulder Strength:", &ppc_values.shoulder_strength);
		parsed |= parse_optional_float_into("$PPC Shoulder Angle:", &ppc_values.shoulder_angle);
		parsed |= parse_optional_float_into("$Exposure:", &exposure);

		parsed |= adjustment::parse(filename, "$Missile light brightness:", profile_name, &missile_light_brightness);
		parsed |= adjustment::parse(filename, "$Missile light radius:", profile_name, &missile_light_radius);

		parsed |= adjustment::parse(filename, "$Laser light brightness:", profile_name, &laser_light_brightness);
		parsed |= adjustment::parse(filename, "$Laser light radius:", profile_name, &laser_light_radius);

		parsed |= adjustment::parse(filename, "$Beam light brightness:", profile_name, &beam_light_brightness);
		parsed |= adjustment::parse(filename, "$Beam light radius:", profile_name, &beam_light_radius);

		parsed |= adjustment::parse(filename, "$Tube light brightness:", profile_name, &tube_light_brightness);
		parsed |= adjustment::parse(filename, "$Tube light radius:", profile_name, &tube_light_radius);

		parsed |= adjustment::parse(filename, "$Point light brightness:", profile_name, &point_light_brightness);
		parsed |= adjustment::parse(filename, "$Point light radius:", profile_name, &point_light_radius);
		parsed |=
			adjustment::parse(filename, "$Directional light brightness:", profile_name, &directional_light_brightness);
		parsed |= adjustment::parse(filename, "$Cone light radius:", profile_name, &cone_light_radius);
		parsed |= adjustment::parse(filename, "$Cone light brightness:", profile_name, &cone_light_brightness);
		if (adjustment::parse(filename, "$Ambient light brightness:", profile_name, &ambient_light_brightness)) {
			parsed = true;
			ambient_light_brightness.stack_multiplier(Cmdline_ambient_power);
			ambient_light_brightness.stack_multiplier(Cmdline_light_power);
			ambient_light_brightness.stack_minimum(Cmdline_emissive_power);
		}

		if (adjustment::parse(filename, "$Overall brightness:", profile_name, &overall_brightness)) {
			parsed = true;
			overall_brightness.stack_multiplier(Cmdline_light_power);
		}

		parsed |= parse_optional_float_into("$Exposure:", &exposure);

		parsed |= adjustment::parse(filename,
			"$Cockpit light radius modifier:",
			profile_name,
			&cockpit_light_radius_modifier);

		parsed |= adjustment::parse(filename,
			"$Cockpit light intensity modifier:",
			profile_name,
			&cockpit_light_intensity_modifier);

		if (!parsed) {
			stuff_string(buffer, F_RAW);
			Warning(LOCATION, "Unhandled line in lighting profile\n\t%s", buffer.c_str());
		}
	}
}

void profile::reset()
{
	name = "";

	tonemapper = TonemapperAlgorithm::Uncharted;

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
	// Default Multiplier to maintain legacy visual compatibility
	overall_brightness.set_multiplier(3.5f);
	overall_brightness.set_minimum(0.0f);
	overall_brightness.stack_multiplier(Cmdline_light_power);

	ambient_light_brightness.reset();
	// Inverse of the overall brightness boost
	ambient_light_brightness.set_multiplier(0.286f);
	ambient_light_brightness.stack_multiplier(Cmdline_ambient_power);
	ambient_light_brightness.set_adjust(MAX(0.0f, Cmdline_emissive_power));

	cockpit_light_radius_modifier.reset();
	cockpit_light_radius_modifier.set_multiplier(1.0f);
	cockpit_light_intensity_modifier.reset();
	cockpit_light_intensity_modifier.set_multiplier(1.0f);
}

profile& profile::operator=(const profile& rhs) = default;

} // namespace lighting_profiles
#include "lab_ui_helpers.h"

#include "lab/labv2_internal.h"
#include "utils/table_viewer.h"

SCP_map<int, SCP_string> get_docking_point_map(int model_index)
{
	SCP_map<int, SCP_string> result;

	polymodel* pm = model_get(model_index);
	if (pm == nullptr || pm->n_docks <= 0)
		return result;

	for (int i = 0; i < pm->n_docks; ++i) {
		const char* name = pm->docking_bays[i].name;
		result[i] = (name && *name) ? SCP_string(name) : SCP_string("<unnamed " + std::to_string(i) + ">");
	}

	return result;
}

SCP_map<int, SCP_string> get_bay_paths_map(int model_index)
{
	SCP_map<int, SCP_string> result;

	polymodel* pm = model_get(model_index);
	if (pm == nullptr || pm->ship_bay->num_paths <= 0)
		return result;

	for (int i = 0; i < pm->ship_bay->num_paths; ++i) {
		const char* name = pm->paths[pm->ship_bay->path_indexes[i]].name;
		result[i] = (name && *name) ? SCP_string(name) : SCP_string("<unnamed " + std::to_string(i) + ">");
	}

	return result;
}


SCP_string get_ship_table_text(ship_info* sip)
{
	return table_viewer::get_table_entry_text("ships.tbl", "*-shp.tbm", sip->name);
}

SCP_string get_weapon_table_text(weapon_info* wip)
{
	return table_viewer::get_table_entry_text("weapons.tbl", "*-wep.tbm", wip->name);
}

SCP_string get_asteroid_table_text(const asteroid_info* aip)
{
	return table_viewer::get_table_entry_text("asteroid.tbl", "*-ast.tbm", aip->name, "No asteroids.tbl found.\r\n");
}

SCP_string get_prop_table_text(const prop_info* pip)
{
	return table_viewer::get_table_entry_text("props.tbl", "*-prp.tbm", pip->name.c_str(), "No props.tbl found.\r\n");
}

SCP_string get_directory_or_vp(const char* path)
{
	SCP_string result(path);

	// Is this a mission in a directory?
	size_t found = result.find("data" DIR_SEPARATOR_STR "missions");

	if (found == std::string::npos) // Guess not
	{
		found = result.find(".vp");
	}

	const auto directory_name_pos = result.rfind(DIR_SEPARATOR_CHAR, found - strlen(DIR_SEPARATOR_STR) - 1);

	result = result.substr(directory_name_pos, found - directory_name_pos);

	found = result.find(DIR_SEPARATOR_CHAR);
	// Strip directory separators
	while (found != std::string::npos) {
		result.erase(found, strlen(DIR_SEPARATOR_STR));
		found = result.find(DIR_SEPARATOR_CHAR);
	}

	return result;
}
namespace ltp = lighting_profiles;
using namespace ltp;
bool graphics_options_changed()
{
	const auto& stored_settings = getLabManager()->graphicsSettings;

	if (stored_settings.ambient_factor != ltp::lab_get_ambient())
		return true;

	if (stored_settings.light_factor != ltp::lab_get_light())
		return true;

	if (stored_settings.emissive_factor != ltp::lab_get_emissive())
		return true;

	if (stored_settings.exposure_level != ltp::current_exposure())
		return true;

	if (stored_settings.tonemapper != ltp::current_tonemapper())
		return true;

	if (stored_settings.bloom_level != gr_bloom_intensity())
		return true;

	if (stored_settings.aa_mode != Gr_aa_mode)
		return true;

	if (stored_settings.ppcv.shoulder_angle != ltp::lab_get_ppc().shoulder_angle)
		return true;

	if (stored_settings.ppcv.shoulder_length != ltp::lab_get_ppc().shoulder_length)
		return true;

	if (stored_settings.ppcv.shoulder_strength != ltp::lab_get_ppc().shoulder_strength)
		return true;

	if (stored_settings.ppcv.toe_length != ltp::lab_get_ppc().toe_length)
		return true;

	if (stored_settings.ppcv.toe_strength != ltp::lab_get_ppc().toe_strength)
		return true;

	return false;
}
#include "lab_ui_helpers.h"

#include "cfile/cfile.h"
#include "lab/labv2_internal.h"


SCP_string get_ship_table_text(ship_info* sip)
{
	char line[256], line2[256], file_text[82];
	int i, j, n, found = 0, comment = 0, num_files = 0;
	SCP_vector<SCP_string> tbl_file_names;
	SCP_string result;

	auto fp = cfopen("ships.tbl", "r");
	Assert(fp);

	while (cfgets(line, 255, fp)) {
		while (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

		for (i = j = 0; line[i]; i++) {
			if (line[i] == '/' && line[i + 1] == '/')
				break;

			if (line[i] == '/' && line[i + 1] == '*') {
				comment = 1;
				i++;
				continue;
			}

			if (line[i] == '*' && line[i + 1] == '/') {
				comment = 0;
				i++;
				continue;
			}

			if (!comment)
				line2[j++] = line[i];
		}

		line2[j] = 0;
		if (!strnicmp(line2, "$Name:", 6)) {
			drop_trailing_white_space(line2);
			found = 0;
			i = 6;

			while (line2[i] == ' ' || line2[i] == '\t' || line2[i] == '@')
				i++;

			if (!stricmp(line2 + i, sip->name)) {
				result += "-- ships.tbl  -------------------------------\r\n";
				found = 1;
			}
		}

		if (found) {
			result += line;
			result += "\r\n";
		}
	}

	cfclose(fp);

	// done with ships.tbl, so now check all modular ship tables...
	num_files = cf_get_file_list(tbl_file_names, CF_TYPE_TABLES, NOX("*-shp.tbm"), CF_SORT_REVERSE);

	for (n = 0; n < num_files; n++) {
		tbl_file_names[n] += ".tbm";

		fp = cfopen(tbl_file_names[n].c_str(), "r");
		Assert(fp);

		memset(line, 0, sizeof(line));
		memset(line2, 0, sizeof(line2));
		found = 0;
		comment = 0;

		while (cfgets(line, 255, fp)) {
			while (line[strlen(line) - 1] == '\n')
				line[strlen(line) - 1] = 0;

			for (i = j = 0; line[i]; i++) {
				if (line[i] == '/' && line[i + 1] == '/')
					break;

				if (line[i] == '/' && line[i + 1] == '*') {
					comment = 1;
					i++;
					continue;
				}

				if (line[i] == '*' && line[i + 1] == '/') {
					comment = 0;
					i++;
					continue;
				}

				if (!comment)
					line2[j++] = line[i];
			}

			line2[j] = 0;
			if (!strnicmp(line2, "$Name:", 6)) {
				drop_trailing_white_space(line2);
				found = 0;
				i = 6;

				while (line2[i] == ' ' || line2[i] == '\t' || line2[i] == '@')
					i++;

				if (!stricmp(line2 + i, sip->name)) {
					memset(file_text, 0, sizeof(file_text));
					snprintf(file_text,
						sizeof(file_text) - 1,
						"--  %s  -------------------------------\r\n",
						tbl_file_names[n].c_str());
					result += file_text;
					found = 1;
				}
			}

			if (found) {
				result += line;
				result += "\r\n";
			}
		}

		cfclose(fp);
	}

	return result;
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

bool graphics_options_changed()
{
	auto stored_settings = getLabManager()->graphicsSettings;

	if (stored_settings.ambient_factor != lighting_profile::lab_get_ambient())
		return true;

	if (stored_settings.light_factor != lighting_profile::lab_get_light())
		return true;

	if (stored_settings.emissive_factor != lighting_profile::lab_get_emissive())
		return true;

	if (stored_settings.exposure_level != lighting_profile::current_exposure())
		return true;

	if (stored_settings.tonemapper != lighting_profile::current_tonemapper())
		return true;

	if (stored_settings.bloom_level != gr_bloom_intensity())
		return true;

	if (stored_settings.aa_mode != Gr_aa_mode)
		return true;

	if (stored_settings.ppcv.shoulder_angle != lighting_profile::lab_get_ppc().shoulder_angle)
		return true;

	if (stored_settings.ppcv.shoulder_length != lighting_profile::lab_get_ppc().shoulder_length)
		return true;

	if (stored_settings.ppcv.shoulder_strength != lighting_profile::lab_get_ppc().shoulder_strength)
		return true;

	if (stored_settings.ppcv.toe_length != lighting_profile::lab_get_ppc().toe_length)
		return true;

	if (stored_settings.ppcv.toe_strength != lighting_profile::lab_get_ppc().toe_strength)
		return true;

	return false;
}
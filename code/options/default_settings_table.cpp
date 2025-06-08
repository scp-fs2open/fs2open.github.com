/*
 * Created by Mike "MjnMixael" Nelson for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 * This file is in charge of the "default_settings.tbl", and allows games and mods
 * to define default options settings that will be used on first launch or if a player
 * has not chosen and saved a specific option value.
 */

#include "options/default_settings_table.h"
#include "options/Option.h"
#include "options/OptionsManager.h"
#include "parse/parselo.h"

void parse_default_settings_table(const char* filename)
{
	try {
		read_file_text(filename, CF_TYPE_TABLES);

		reset_parse();

		// start parsing
		optional_string("#DEFAULT SETTINGS");

		// allow settings to be in any order
		while (!check_for_string("#END")) {

			if (optional_string("$Option Key:")) {
				SCP_string name;
				stuff_string(name, F_NAME);

				const options::OptionBase* thisOpt = options::OptionsManager::instance()->getOptionByKey(name);

				if (thisOpt == nullptr) {
					// keep this just a debug print, as options may be removed but still valid for the table
					// such as VR mode removing the Window option --Mjn and wookieejedi 
					mprintf(("Warning: %s is not a valid option for the default_settings table!\n", name.c_str()));
					skip_to_start_of_string_either("$Option Key:", "#END");
					continue;
				}
				required_string("+Value:");
				thisOpt->parseDefaultSetting();

				// If an option is enforced and not retail we set the flag so that the default value is set
				// later during initialization, the option will be hidden from the options menu
				// Retail options cannot be hidden because we can't really hide them from the menu
				if ((optional_string_one_of(2, "+Enforce", "+Enforced")) != -1) {
					if (!(thisOpt->getFlags()[options::OptionFlags::RetailBuiltinOption])) {
						options::OptionsManager::instance()->enforceOption(name);
					} else {
						error_display(0, "%s is a retail builtin option and cannot be enforced!", name.c_str());
					}
				}
			} else {
				break;
			}
		}

		required_string("#END");
	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n",
			(filename) ? filename : "<default_settings.tbl>",
			e.what()));
		return;
	}
}

void default_settings_init()
{
	// if a default_settings.tbl exists read it
	if (cf_exists_full("default_settings.tbl", CF_TYPE_TABLES)) {
		parse_default_settings_table("default_settings.tbl");
	}

	// parse any modular tables
	parse_modular_table("*-defs.tbm", parse_default_settings_table);
}
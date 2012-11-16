/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"
#include "mission/missioncampaign.h"
#include "mission/missionmessage.h"
#include "mod_table/mod_table.h"
#include "localization/localize.h"
#include "parse/parselo.h"

int Directive_wait_time = 3000;
bool True_loop_argument_sexps = false;
bool Fixed_turret_collisions = false;
bool Damage_impacted_subsystem_first = false;
bool Cutscene_camera_displays_hud = false;
bool Alternate_chaining_behavior = false;
int Default_ship_select_effect = 2;
int Default_weapon_select_effect = 2;
bool Enable_external_shaders = false;
int Default_detail_level = 3; // "very high" seems a reasonable default in 2012 -zookeeper
bool Full_color_head_anis = false;
bool Weapons_inherit_parent_collision_group = false;


void parse_mod_table(char *filename)
{
	int rval;
	// SCP_vector<SCP_string> lines;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", (filename) ? filename : "<default game_settings.tbl>", rval));
		lcl_ext_close();
		return;
	}

	if (filename == NULL)
		read_file_text_from_array(defaults_get_file("game_settings.tbl"));
	else
		read_file_text(filename, CF_TYPE_TABLES);

	reset_parse();	

	// start parsing
	optional_string("#CAMPAIGN SETTINGS");

	if (optional_string("$Default Campaign File Name:")) {
		char temp[MAX_FILENAME_LEN];
		stuff_string(temp, F_NAME, MAX_FILENAME_LEN);

		// remove extension?
		char *p = strrchr(temp, '.');
		if (p != NULL) {
			mprintf(("Game Settings Table: Removing extension on default campaign file name %s\n", temp));
			*p = 0;
		}

		// check length
		int maxlen = (MAX_FILENAME_LEN - 4);
		int len = strlen(temp);
		if (len > maxlen) {
			Warning(LOCATION, "Token too long: [%s].  Length = %i.  Max is %i.\n", temp, len, maxlen);
			temp[maxlen] = 0;
		}

		strcpy_s(Default_campaign_file_name, temp);
	}

	if (optional_string("#Ignored Campaign File Names")) {
		SCP_string campaign_name; 
		while (optional_string("$Campaign File Name:")) {
			stuff_string(campaign_name, F_NAME); 
			Ignored_campaigns.push_back(campaign_name); 
		}
	}

	optional_string("#HUD SETTINGS");

	// how long should the game wait before displaying a directive?
	if (optional_string("$Directive Wait Time:")) {
		stuff_int(&Directive_wait_time);
	}

	if (optional_string("$Cutscene camera displays HUD:")) {
		stuff_boolean(&Cutscene_camera_displays_hud);
	}
	// compatibility
	if (optional_string("$Cutscene camera disables HUD:")) {
		mprintf(("Game Settings Table: \"$$Cutscene camera disables HUD\" is deprecated in favor of \"$Cutscene camera displays HUD\"\n"));
		bool temp;
		stuff_boolean(&temp);
		Cutscene_camera_displays_hud = !temp;
	}
	
	if (optional_string("$Full color head animations:")) {
		stuff_boolean(&Full_color_head_anis);
	}
	// compatibility
	if (optional_string("$Color head animations with hud colors:")) {
		mprintf(("Game Settings Table: \"$Color head animations with hud colors\" is deprecated in favor of \"$Full color head animations\"\n"));
		bool temp;
		stuff_boolean(&temp);
		Full_color_head_anis = !temp;
	}

	optional_string("#SEXP SETTINGS"); 

	if (optional_string("$Loop SEXPs Then Arguments:")) { 
		stuff_boolean(&True_loop_argument_sexps);
		if (True_loop_argument_sexps) {
			mprintf(("Game Settings Table: Using Reversed Loops For SEXP Arguments\n"));
		} else {
			mprintf(("Game Settings Table: Using Standard Loops For SEXP Arguments\n"));
		}
	}

	if (optional_string("$Use Alternate Chaining Behavior:")) {
		stuff_boolean(&Alternate_chaining_behavior);
		if (Alternate_chaining_behavior) {
			mprintf(("Game Settings Table: Using alternate event chaining behavior\n"));
		} else {
			mprintf(("Game Settings Table: Using standard event chaining behavior\n"));
		}
	}

	optional_string("#GRAPHICS SETTINGS");

	if (optional_string("$Enable External Shaders:")) {
		stuff_boolean(&Enable_external_shaders);
		if (Enable_external_shaders)
			mprintf(("Game Settings Table: External shaders are enabled\n"));
		else
			mprintf(("Game Settings Table: External shaders are DISABLED\n"));
	}

	if (optional_string("$Default Detail Level:")) {
		int detail_level;

		stuff_int(&detail_level);

		mprintf(("Game Settings Table: Setting default detail level to %i of %i-%i\n", detail_level, 0, NUM_DEFAULT_DETAIL_LEVELS-1));

		if (detail_level < 0 || detail_level > NUM_DEFAULT_DETAIL_LEVELS-1) {
			Warning(LOCATION, "Invalid detail level: %i, setting to %i\n", detail_level, Default_detail_level);
		} else {
			Default_detail_level = detail_level;
		}
	}
	
	optional_string("#OTHER SETTINGS"); 

	if (optional_string("$Fixed Turret Collisions:")) { 
		stuff_boolean(&Fixed_turret_collisions);
	}

	if (optional_string("$Damage Impacted Subsystem First:")) { 
		stuff_boolean(&Damage_impacted_subsystem_first);
	}

	if (optional_string("$Default ship select effect:")) {
		char effect[NAME_LENGTH];
		stuff_string(effect, F_NAME, NAME_LENGTH);
		if (!stricmp(effect, "FS2"))
			Default_ship_select_effect = 2;
		else if (!stricmp(effect, "FS1"))
			Default_ship_select_effect = 1;
		else if (!stricmp(effect, "off"))
			Default_ship_select_effect = 0;
	}

	if (optional_string("$Default weapon select effect:")) {
		char effect[NAME_LENGTH];
		stuff_string(effect, F_NAME, NAME_LENGTH);
		if (!stricmp(effect, "FS2"))
			Default_weapon_select_effect = 2;
		else if (!stricmp(effect, "FS1"))
			Default_weapon_select_effect = 1;
		else if (!stricmp(effect, "off"))
			Default_weapon_select_effect = 0;
	}

	if (optional_string("$Weapons inherit parent collision group:")) {
		stuff_boolean(&Weapons_inherit_parent_collision_group);
		if (Weapons_inherit_parent_collision_group)
			mprintf(("Game Settings Table: Weapons inherit parent collision group\n"));
	}

	required_string("#END");

	// close localization
	lcl_ext_close();
}

void mod_table_init()
{	
	// first parse the default table
	parse_mod_table(NULL);

	// if a mod.tbl exists read it
	if (cf_exists_full("game_settings.tbl", CF_TYPE_TABLES)) {
		parse_mod_table("game_settings.tbl");
	}

	// parse any modular tables
	parse_modular_table("*-mod.tbm", parse_mod_table);
}

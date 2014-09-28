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
#include "sound/sound.h"
#include "gamesnd/eventmusic.h"

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
bool Flight_controls_follow_eyepoint_orientation = false;
int FS2NetD_port = 0;
float Briefing_window_FOV = 0.29375f;
bool Disable_hc_message_ani = false;
bool Red_alert_applies_to_delayed_ships = false;
bool Beams_use_damage_factors = false;


void parse_mod_table(const char *filename)
{
	int rval;
	// SCP_vector<SCP_string> lines;

	if ((rval = setjmp(parse_abort)) != 0)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", (filename) ? filename : "<default game_settings.tbl>", rval));
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
		if (drop_extension(temp)) {
			mprintf(("Game Settings Table: Removed extension on default campaign file name %s\n", temp));
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

			// remove extension?
			if (drop_extension(campaign_name)) {
				mprintf(("Game Settings Table: Removed extension on ignored campaign file name %s\n", campaign_name.c_str()));
			}

			Ignored_campaigns.push_back(campaign_name);
		}
	}

	if (optional_string("$Red-alert applies to delayed ships:")) {
		stuff_boolean(&Red_alert_applies_to_delayed_ships);
		if (Red_alert_applies_to_delayed_ships) {
			mprintf(("Game Settings Table: Red-alert stats will be loaded for ships that arrive later in missions\n"));
		} else {
			mprintf(("Game Settings Table: Red-alert stats will NOT be loaded for ships that arrive later in missions (this is retail behavior)\n"));
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

	if (optional_string("$Briefing Window FOV:")) {
		float fov;

		stuff_float(&fov);

		mprintf(("Game Settings Table: Setting briefing window FOV from %f to %f\n", Briefing_window_FOV, fov));

		Briefing_window_FOV = fov;
	}
	
	optional_string("#NETWORK SETTINGS"); 

	if (optional_string("$FS2NetD port:")) {
		stuff_int(&FS2NetD_port);
		if (FS2NetD_port)
			mprintf(("Game Settings Table: FS2NetD connecting to port %i\n", FS2NetD_port));
	}

	optional_string("#SOUND SETTINGS"); 

	if (optional_string("$Default Sound Volume:")) {
		stuff_float(&Master_sound_volume);
	}

	if (optional_string("$Default Music Volume:")) {
		stuff_float(&Master_event_music_volume);
	}

	if (optional_string("$Default Voice Volume:")) {
		stuff_float(&Master_voice_volume);
	}

	optional_string("#FRED SETTINGS");
	
	if (optional_string("$Disable Hard Coded Message Head Ani Files:")) {
		stuff_boolean(&Disable_hc_message_ani);
		if (Disable_hc_message_ani) {
			mprintf(("Game Settings Table: FRED - Disabling Hard Coded Message Ani Files\n"));
		} else {
			mprintf(("Game Settings Table: FRED - Using Hard Coded Message Ani Files\n"));
			
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

	if (optional_string("$Flight controls follow eyepoint orientation:")) {
		stuff_boolean(&Flight_controls_follow_eyepoint_orientation);
		if (Flight_controls_follow_eyepoint_orientation)
			mprintf(("Game Settings Table: Flight controls follow eyepoint orientation\n"));
	}

	if (optional_string("$Beams Use Damage Factors:")) {
		stuff_boolean(&Beams_use_damage_factors);
		if (Beams_use_damage_factors) {
			mprintf(("Game Settings Table: Beams will use Damage Factors\n"));
		} else {
			mprintf(("Game Settings Table: Beams will ignore Damage Factors (retail behavior)\n"));
		}
	}

	required_string("#END");
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

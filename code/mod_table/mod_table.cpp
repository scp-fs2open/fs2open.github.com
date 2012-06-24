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

int Directive_wait_time;
bool True_loop_argument_sexps;
bool Fixed_turret_collisions;
bool Damage_impacted_subsystem_first;
bool Cutscene_camera_disables_hud;

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
		stuff_string(Default_campaign_file_name, F_NAME, (MAX_FILENAME_LEN - 4) );
	}

	optional_string("#HUD SETTINGS"); 
	// how long should the game wait before displaying a directive?
	if (optional_string("$Directive Wait Time:")) {
		stuff_int(&Directive_wait_time);
	}

	if (optional_string("$Cutscene camera disables HUD:")) {
		stuff_boolean(&Cutscene_camera_disables_hud);
		if (!Cutscene_camera_disables_hud)
			mprintf(("Game Settings Table : HUD will not be disabled by default in in-game cutscenes.\n"));
	} else {
		Cutscene_camera_disables_hud = true;
	}

	optional_string("#SEXP SETTINGS"); 
	if (optional_string("$Loop SEXPs Then Arguments:")) { 
		stuff_boolean(&True_loop_argument_sexps);
		if (True_loop_argument_sexps){
			mprintf(("Game Settings Table : Using Reversed Loops For SEXP Arguments\n"));
		}
		else {
			mprintf(("Game Settings Table : Using Standard Loops For SEXP Arguments\n"));
		}
	}

	optional_string("#OTHER SETTINGS"); 

	if (optional_string("$Fixed Turret Collisions:")) { 
		stuff_boolean(&Fixed_turret_collisions);
	}

	if (optional_string("$Damage Impacted Subsystem First:")) { 
		stuff_boolean(&Damage_impacted_subsystem_first);
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

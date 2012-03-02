/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "globalincs/def_files.h"
#include "mod_table/mod_table.h"
#include "localization/localize.h"
#include "parse/parselo.h"

int Directive_wait_time;
bool True_loop_argument_sexps;

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
	required_string("#HUD SETTINGS");
	
	// how long should the game wait before displaying a directive?
	if (optional_string("$Directive Wait Time:")) {
		stuff_int(&Directive_wait_time);
	}

	required_string("#SEXP SETTINGS");

	if (optional_string("$Loop SEXPs Then Arguments:")) {
		stuff_boolean(&True_loop_argument_sexps);
		if (True_loop_argument_sexps){
			mprintf(("Game Settings Table : Using Reversed Loops For SEXP Arguments"));
		}
		else {
			mprintf(("Game Settings Table : Using Standard Loops For SEXP Arguments"));
		}
	}
	else 



	required_string("#END");

	// close localization
	lcl_ext_close();
}

void mod_table_init()
{	
	// if a mod.tbl exists read it, otherwise fallback to the default
	if (cf_exists_full("game_settings.tbl", CF_TYPE_TABLES))
		parse_mod_table("game_settings.tbl");
	else
		parse_mod_table(NULL);

	// parse any modular tables
	parse_modular_table("*-mod.tbm", parse_mod_table);
}

/*
 * z64555's debug console
 * Created for the FreeSpace Source Code project
 *
 * Portions of this source code are based on works by Volition, Inc. circa
 * 1999. You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source
 */

/**
 *  @file consolecmds.cpp
 *  
 *  @brief This file contains the "built-in" commands for the debug console, and is listed by the 'help' and '?' commands
 *  
 *  @details
 *  All other debug commands should be in their respective files and added to the console with the DCF macro. For
 *  further documentation, please see console.h
 */

#include "debugconsole/console.h"
#include "debugconsole/consoleparse.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "io/key.h"

#include <algorithm>

// ========================= GLOBALS =========================
SCP_vector<debug_command*> dc_commands;
typedef SCP_vector<debug_command*>::iterator dc_commands_it;

// ========================= LOCALS ==========================
// dcf_shell commands
void dc_shell_font( void );
void dc_shell_resize( void );
void dc_shell_resize_buf( void );

// =================== class debug_command ===================
debug_command::debug_command(const char *_name, const char *_help, void (*_func)())
	: name(_name), help(_help), func(_func)
{
	dc_commands_it it = std::find_if(dc_commands.begin(), dc_commands.end(), is_dcmd(_name));
	
	if (it != dc_commands.end()) {
		Int3();		// Command already exists! Somebody didn't use the DCF macro as they should've...
	}

	dc_commands.push_back(this);
}

// ============================== IMPLEMENTATIONS =============================

DCF(debug, "Runs a command in debug mode.")
{
	SCP_string command = "";
	Dc_debug_on = true;

	dc_stuff_string_white(command);

	if (command == "") {
		dc_printf("<debug> No command given\n");
		return;
	} // Else, command is present.

	dc_commands_it it = std::find_if(dc_commands.begin(), dc_commands.end(), is_dcmd(command.c_str()));

	if (it == dc_commands.end()) {
		dc_printf("<debug> Command not found: '%s'\n", command.c_str());
		return;
	} // Else, command exists. Run it.

	dc_printf("<debug> Executing command: '%s'\n", command.c_str());
	// try {
	(*it)->func();
	// } catch {
	// }

	Dc_debug_on = false;
}

DCF(help, "Displays the help list." )
{
	extern uint DROWS;

	SCP_string command = "";

	dc_maybe_stuff_string_white(command);
	if ((command == "help") || (command == "man"))
	{
		// Moron filter :D
		dc_printf("GTVA Command: Sorry pilot. You're on your own.\n");
		return;

	} else if (command != "") {
		dc_commands_it it = std::find_if(dc_commands.begin(), dc_commands.end(), is_dcmd(command.c_str()));

		if (it == dc_commands.end()) {
			dc_printf("Command not found: '%s'\n", command.c_str());
			return;
		}

		dc_printf("%s\n", (*it)->help);
		return;
	} // Else, command line is empty, print out the help list

	dc_printf("FreeSpace Open Debug Console\n");
	dc_printf(" These commands are defined internally.\n");
	dc_printf(" Typing 'help function_name' will give the short help on the function.\n");
	dc_printf(" Some functions may have detailed help, try passing \"help\" or \"--help\" to them.");
	dc_printf(" F3 selects last command line. Up and Down arrow keys scroll through the command history\n");
	dc_printf("\n");

	dc_printf(" Available commands:\n");
	for (dc_commands_it it = dc_commands.begin(); it != dc_commands.end(); ++it) {
		if (((lastline % DROWS) == 0) && (lastline != 0)) {
			dc_pause_output();
		}

		dc_printf(" %s - %s\n", (*it)->name, (*it)->help);
	}
}

debug_command dc_man("man", "Also displays the help list", dcf_help);

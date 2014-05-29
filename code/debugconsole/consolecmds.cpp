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
debug_command *dc_commands[DC_MAX_COMMANDS];
int dc_commands_size = 0;

// ========================= LOCALS ==========================
// dcf_shell commands
void dc_shell_font( void );
void dc_shell_resize( void );
void dc_shell_resize_buf( void );

// =================== class debug_command ===================
debug_command::debug_command()
: name(""), help(""), func(NULL) {
};

debug_command::debug_command(const char *_name, const char *_help, void(*_func)())
	: name(_name), help(_help), func(_func) {
	int i = 0;
	int ret = 0;

	if (dc_commands_size >= DC_MAX_COMMANDS) {
		Error(LOCATION, "Too many debug console commands! Please inform a coder to increase DC_MAX_COMMANDS.");
		return;
	}

	// Start the insertion sort by finding where to stick the debug command
	for (; i < dc_commands_size; ++i) {
		ret = stricmp(dc_commands[i]->name, _name);

		if (ret == 0) {
			Error(LOCATION, "Debug Command %s already exists! Please inform a coder immediately.", _name);
			return;
		} else if (ret > 0) {
			// Insert the command here
			break;
		} // Else, do nothing
	}

	// Then, do the insertion
	if (i < dc_commands_size) {
		for (int j = dc_commands_size; j > i; --j) {
			dc_commands[j] = dc_commands[j - 1];
		}
		dc_commands[i] = this;
		dc_commands_size++;
	} else {
		dc_commands[dc_commands_size] = this;
		dc_commands_size++;
	}
}

// ============================== IMPLEMENTATIONS =============================

DCF(debug, "Runs a command in debug mode.")
{
	int i;
	SCP_string command = "";
	
	Dc_debug_on = true;

	dc_stuff_string_white(command);

	if (command == "") {
		dc_printf("<debug> No command given\n");
		return;
	} // Else, command is present.

	for (i = 0; i < dc_commands_size; ++i) {
		if (stricmp(dc_commands[i]->name, command.c_str()) == 0) {
			break;
		} // Else, continue
	}

	if (i == dc_commands_size) {
		dc_printf("<debug> Command not found: '%s'\n", command.c_str());
		return;
	} // Else, we found our command

	dc_printf("<debug> Executing command: '%s'\n", command.c_str());
	// try {
	dc_commands[i]->func();
	// } catch {
	// }

	Dc_debug_on = false;
}

DCF(help, "Displays the help list." )
{
	extern uint DROWS;

	int i;
	SCP_string command = "";

	dc_maybe_stuff_string_white(command);
	if ((command == "help") || (command == "man"))
	{
		// Moron filter :D
		dc_printf("GTVA Command: Sorry pilot. You're on your own.\n");
		return;

	} else if (command != "") {
		for (i = 0; i < dc_commands_size; ++i) {
			if (stricmp(dc_commands[i]->name, command.c_str()) == 0) {
				break;
			} // Else, continue
		}

		if (i == dc_commands_size) {
			dc_printf("Command not found: '%s'\n", command.c_str());
			return;
		} // Else, we found our command

		dc_printf("%s\n", dc_commands[i]->help);
		return;
	} // Else, command line is empty, print out the help list

	dc_printf("FreeSpace Open Debug Console\n");
	dc_printf(" These commands are defined internally.\n");
	dc_printf(" Typing 'help function_name' will give the short help on the function.\n");
	dc_printf(" Some functions may have detailed help, try passing \"help\" or \"--help\" to them.");
	dc_printf(" F3 selects last command line. Up and Down arrow keys scroll through the command history\n");
	dc_printf("\n");

	dc_printf(" Available commands:\n");
	for (i = 0; i < dc_commands_size; ++i) {
		if (((lastline % DROWS) == 0) && (lastline != 0)) {
			dc_pause_output();
		}

		dc_printf(" %s - %s\n", dc_commands[i]->name, dc_commands[i]->help);
	}
}

debug_command dc_man("man", "Also displays the help list", dcf_help);

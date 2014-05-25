#ifndef _CONSOLE_H
#define _CONSOLE_H
/*
 * z64555's debug console, created for the FreeSpace Source Code project
 *
 * Portions of this source code are based on works by Volition, Inc. circa 1999. You may not sell or otherwise
 * commercially exploit the source or things you created based on the source.
 */

/**
 * @file console.h
 * @brief An overhauled/updated debug console to allow monitoring, testing, and general debugging of new features.
 *
 * @details
 * Of key interest is Volition's DCF macro, which adds the function argument to the available command list in the
 * debug console. These functions may be defined in the .cpp file that they are related to, but it is recommended 
 * that they be in their own .cpp if they have multiple sub-arguments (ex: Git has its sub-arguments delimited by
 * a pair of -'s, or --)
 */

#include "debugconsole/consoleparse.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"

#define DC_MAX_COMMANDS 300

class debug_command;

/**
 * @def DCF
 * 
 * @brief The potent DCF macro, used to define new debug commands for the console.
 * 
 * @param function_name[in] The name of the function, as shown in the debug console
 * @param help_txt[in] The short-help text, as shown as listed from the 'help' command
 *
 * @details 
 * Each DCF is responsible for getting data from the debug console's command line. The parsing functions for the
 * command line have been set up to have similar syntax and usage as the parselo commands. (see consoleparse.h)
 * Most, if not all, argument and subcommand strings should be parsed by the dc_optional_string functions.
 * Usage example:
 * DCF(toggle_it, "description")
 * {
 *     if (dc_optional_string_either("help", "--help")) {
 *         dc_printf("Usage: sample. Toggles This_var on/off\n");
 *
 *     } else if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
 *         dc_printf("This_var is %s\n", (This_var ? "ON" : "OFF"));
 *
 *     } else {
 *         This_var = !This_var;
 *
 *     }
 * }
 *
 *  In the console, the command would be listed as 'toggle_it', and dcf_help would display it as:
 *      toggle_it  - Usage: sample. Toggles This_var on/off.
 *  Note: The only allowed function type is a void fn( void )
 */
#define DCF(function_name, help_text)	\
		void dcf_##function_name();	\
		debug_command dcmd_##function_name(#function_name, help_text, dcf_##function_name);	\
		void dcf_##function_name()

/**
 *  @def Shortcut for debug commands that toggle a bool, such as Show_lightning
 *  
 *  @param [in] function_name Name of the function, as shown in the debug console
 *  @param [in] bool_variable Name of the variable to allow toggling.
 */
#define DCF_BOOL(function_name, bool_variable)	\
	void dcf_##function_name();		\
	debug_command dcmd_##function_name(#function_name, "Sets or toggles the boolean: "#bool_variable, dcf_##function_name );	\
	void dcf_##function_name() {	\
		bool bool_tmp = bool_variable != 0;	\
		if (dc_optional_string_either("help", "--help")) {	\
				dc_printf( "Usage: %s [bool]\nSets %s to true or false.  If nothing passed, then toggles it.\n", #function_name, #bool_variable );	\
				return;		\
		}	\
		if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {	\
			dc_printf("%s = %s\n", #bool_variable, (bool_variable ? "TRUE" : "FALSE"));		\
			return;		\
		}	\
		if (!dc_maybe_stuff_boolean(&bool_tmp)) {	\
			if (bool_variable != 0) \
				bool_variable = 0; \
			else \
				bool_variable = 1;	\
		} else { \
			if (bool_tmp) \
				bool_variable = 1; \
			else \
				bool_variable = 0;	\
		}	\
		dc_printf("%s set to %s\n", #bool_variable, (bool_variable != 0 ? "TRUE" : "FALSE"));	\
	}


/**
 *  @def Same as DCF_BOOL, but with custom help strings
 *  
 *  @param [in] function_name Name of the function, as shown in the debug console
 *  @param [in] bool_variable Name of the variable to allow toggling.
 */
#define DCF_BOOL2(function_name, bool_variable, short_help, long_help)	\
	void dcf_##function_name();		\
	debug_command dcmd_##function_name(#function_name, short_help, dcf_##function_name );	\
	void dcf_##function_name() {	\
		if (dc_optional_string_either("help", "--help")) {	\
			dc_printf( #long_help );	\
				return;		\
		}	\
		if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {	\
			dc_printf("%s = %s\n", #function_name, (bool_variable ? "TRUE" : "FALSE"));		\
			return;		\
		}	\
		if (!dc_maybe_stuff_boolean(&bool_variable)) {	\
			bool_variable = !bool_variable;	\
		}	\
	}

 /**
  * @def Shortcut for single-variable setters/monitors
  *
  * @param [in] function_name
  * @param [in] float_variable
  * @param [in] short_help
  */
 #define DCF_FLOAT(function_name, float_variable, short_help)	\
	void dcf_##function_name();		\
	debug_command dcmd_##function_name(#function_name, short_help, dcf_##function_name );	\
	void dcf_##function_name() {	\
		float value;	\
		if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {	\
			dc_printf("%s = %f\n", #float_variable, float_variable);		\
			return;		\
		}	\
		dc_stuff_float(&value);	\
		float_variable = value;	\
		dc_printf("%s set to %f\n", #float_variable, float_variable);	\
	}

 /**
  * @def Shortcut for single-variable setters/monitors with lower/upper bounds clamping
  *
  * @param [in] function_name
  * @param [in] float_variable
  * @param [in] lower_bounds
  * @param [in] upper_bounds
  * @param [in] short_help
  */
 #define DCF_FLOAT2(function_name, float_variable, lower_bounds, upper_bounds, short_help)	\
	void dcf_##function_name();		\
	debug_command dcmd_##function_name(#function_name, short_help, dcf_##function_name );	\
	void dcf_##function_name() {	\
		float value;	\
		if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {	\
			dc_printf("%s = %f\n", #float_variable, float_variable);		\
			return;		\
		}	\
		dc_stuff_float(&value);	\
		CLAMP(float_variable, lower_bounds, upper_bounds);	\
		float_variable = value;	\
		dc_printf("%s set to %f\n", #float_variable, float_variable);	\
	}

 /**
  * @def Shortcut for single-variable setters/monitors
  *
  * @param [in] function_name
  * @param [in] int_variable
  * @param [in] short_help
  */
 #define DCF_INT(function_name, int_variable, short_help)	\
	void dcf_##function_name();		\
	debug_command dcmd_##function_name(#function_name, short_help, dcf_##function_name );	\
	void dcf_##function_name() {	\
		int value;	\
		if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {	\
			dc_printf("%s = %i\n", #int_variable, int_variable);		\
			return;		\
		}	\
		dc_stuff_int(&value);	\
		int_variable = value;	\
		dc_printf("%s set to %i\n", #int_variable, int_variable);	\
	}

 /**
  * @def Shortcut for single-variable setters/monitors with lower/upper bounds clamping
  *
  * @param [in] function_name
  * @param [in] int_variable
  * @param [in] lower_bounds
  * @param [in] upper_bounds
  * @param [in] short_help
  */
 #define DCF_INT2(function_name, int_variable, lower_bounds, upper_bounds, short_help)	\
	void dcf_##function_name();		\
	debug_command dcmd_##function_name(#function_name, short_help, dcf_##function_name );	\
	void dcf_##function_name() {	\
		int value;	\
		if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {	\
			dc_printf("%s = %i\n", #int_variable, int_variable);		\
			return;		\
		}	\
		dc_stuff_int(&value);	\
		CLAMP(int_variable, lower_bounds, upper_bounds);	\
		int_variable = value;	\
		dc_printf("%s set to %i\n", #int_variable, int_variable);	\
	}

/**
 *  @class debug_command
 *  @brief Class to aggregate a debug command with its name (as shown in the console) and short help.
 *  
 *  @details
 *  Note: Long help, as evoked by '<command> help', should be handled by the function itself. It is recommended
 *  that arguments that have sub-arguments be in their own function, so as to aide in organization and to keep the
 *  size of the command function down.
 */
class debug_command {
public:
	const char *name;		//!< The name of the command, as shown in the debug console
	const char *help;		//!< The short help string, as shown by 'help <command>'
	void (*func)();	//!< Pointer to the function that to run when this command is evoked

	debug_command();

	/**
	* @brief Adds a debug command to the debug_commands map, if it isn't in there already.
	*
	* @details The DCF macro more or less guarantees that a command won't be duplicated on compile time. But no harm in
	*   some extra caution.
	*/
	debug_command(const char *name, const char *help, void (*func)());
};

/**
 * @class is_dcmd
 * @brief Predicate class used to search for a dcmd by name
 */
class is_dcmd {
public:
	const char *name;

	is_dcmd(const char *_name) : name(_name) {};

	bool operator() (debug_command* dcmd)
	{
		return (strcmp(name, dcmd->name) == 0);
	}
};

extern bool Dc_debug_on;
extern int dc_commands_size;
extern uint lastline;
extern SCP_string dc_command_str;	// The rest of the command line, from the end of the last processed arg on.

/**
 * @brief   Pauses the output of a command and allows user to scroll through the output history.
 *
 * @details Returns true if user has pressed Esc, returns false otherwise. Use this in your function to (safely?) break
 *     out of the loop it's presumably in.
 */
bool dc_pause_output(void);

/**
 * @brieft Prints the given char string to the debug console
 * @details See the doc for std::printf() for formating and more details
 */
void dc_printf(const char *format, ...);

/**
 * @brief Opens and processes the debug console. (Blocking call)
 * @details TODO: Make this a non-blocking call so that the game can still run while the debug console is open.
 */
void debug_console(void (*func)(void) = NULL);

#endif // _CONSOLE_H

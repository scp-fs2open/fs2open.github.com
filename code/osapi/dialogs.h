
#ifndef _OSAPI_DIALOGS_H
#define _OSAPI_DIALOGS_H
#pragma once

#include "globalincs/pstypes.h"

struct lua_State;

// Stupid windows workaround...
#ifdef MessageBox
#undef MessageBox
#endif

namespace os
{
	namespace dialogs
	{
		// These map onto the SDL ShowSimpleMessageBox flags
		enum MessageBoxType
		{
			MESSAGEBOX_ERROR,
			MESSAGEBOX_WARNING,
			MESSAGEBOX_INFORMATION,
		};
		
		/**
		 * @brief Displays an assert message.
		 * @note Used by Assert() and Assertion() to display an error message, should not be used directly
		 *
		 * @param text The text to display
		 * @param filename The source code filename where this function was called
		 * @param linenum The source code line number where this function was called
		 * @param format An optional message to display in addition to the specified text
		 */
		void AssertMessage(const char* text, const char* filename, int linenum,
				SCP_FORMAT_STRING const char* format = nullptr, ...) SCP_FORMAT_STRING_ARGS(4, 5);

		/**
		 * @brief Shows a lua error.
		 * This captures the current state of the given lua_State and displays a dialog describing the error.
		 * If @c format is @c nullptr this function pops a string from the top of the lua stack and uses that as the error message.
		 * @param L The lua_State to capture the state of
		 * @param format The error message to display, may be @c nullptr
		 */
		void LuaError(lua_State *L, SCP_FORMAT_STRING const char *format = nullptr, ...) SCP_FORMAT_STRING_ARGS(2, 3);

		/**
		 * @brief Shows an error dialog.
		 * Only use this function if the program is in an unrecoverable state because of invalid user data, programming errors should
		 * be handled with Assert() and Assertion(). This function usually doesn't return as the generated error is considered fatal.
		 *
		 * @param filename The source code filename where this function was called
		 * @param line The source code line number where this function was called
		 * @param format The error message to display (a format string)
		 */
		void Error(const char* filename, int line, SCP_FORMAT_STRING const char* format, ...) SCP_FORMAT_STRING_ARGS(3, 4);

		/**
		 * @brief Shows an error dialog.
		 * This is a more general version of Error(const char*,int,const char*,...) that only displays the dialog.
		 *
		 * @param text The text to display
		 */
		void Error(const char* text);

		/**
		 * @brief Shows a warning dialog.
		 * A warning should be generated if a recoverable user data error was detected. This function is only enabled in debug builds.
		 *
		 * @param filename The source code filename where this function was called
		 * @param line The source code line number where this function was called
		 * @param format The message to display
		 */
		void Warning(const char* filename, int line, SCP_FORMAT_STRING const char* format, ...) SCP_FORMAT_STRING_ARGS(3, 4);

		/**
		 * @brief Shows an extra warning.
		 * Same as Warning(const char*,int,const char*) but only Cmdline_extra_warn is set to @c 1.
		 *
		 * @param filename The source code filename where this function was called
		 * @param line The source code line number where this function was called
		 * @param format The message to display
		 */
		void WarningEx(const char* filename, int line, SCP_FORMAT_STRING const char* format, ...) SCP_FORMAT_STRING_ARGS(3, 4);

		/**
		 * @brief Shows a warning dialog.
		 * Same as #Warning but also appears in release builds.
		 *
		 * @param filename The source code filename where this function was called
		 * @param line The source code line number where this function was called
		 * @param format The message to display
		 */
		void ReleaseWarning(const char* filename, int line, SCP_FORMAT_STRING const char* format, ...) SCP_FORMAT_STRING_ARGS(3, 4);
		
		void MessageBox(MessageBoxType type, const char* message, const char* title = NULL);
	}
}

// Make these available in the global namespace for compatibility
using os::dialogs::LuaError;
using os::dialogs::Error;
using os::dialogs::Warning;
using os::dialogs::ReleaseWarning;
using os::dialogs::WarningEx;

#endif // _OSAPI_DIALOGS_H

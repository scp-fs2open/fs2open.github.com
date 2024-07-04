
#ifndef _OSAPI_DIALOGS_H
#define _OSAPI_DIALOGS_H
#pragma once

#include "globalincs/pstypes.h"

#include <stdexcept>

struct lua_State;

namespace os
{
	namespace dialogs
	{
		class DialogException : public std::runtime_error {
		 public:
			explicit DialogException(const std::string& msg) : std::runtime_error(msg) {}
		};
		class AssertException : public DialogException {
		 public:
			explicit AssertException(const std::string& msg) : DialogException(msg) {}
		};
		class LuaErrorException : public DialogException {
		 public:
			explicit LuaErrorException(const std::string& msg) : DialogException(msg) {}
		};
		class ErrorException : public DialogException {
		 public:
			explicit ErrorException(const std::string& msg) : DialogException(msg) {}
		};
		class WarningException : public DialogException {
		 public:
			explicit WarningException(const std::string& msg) : DialogException(msg) {}
		};

		// These map onto the SDL ShowSimpleMessageBox flags
		enum MessageType
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
		void AssertMessage(const char* text, const char* filename, int linenum, SCP_FORMAT_STRING
						   const char* format = nullptr, ...)
		SCP_FORMAT_STRING_ARGS(4, 5) CLANG_ANALYZER_NORETURN;

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
		void Error(const char* filename, int line, SCP_FORMAT_STRING const char* format, ...)
		SCP_FORMAT_STRING_ARGS(3, 4) CLANG_ANALYZER_NORETURN;

		/**
		 * @brief Shows an error dialog.
		 * This is a more general version of Error(const char*,int,const char*,...) that only displays the dialog.
		 *
		 * @param text The text to display
		 */
		void Error(const char* text) CLANG_ANALYZER_NORETURN;;

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
		
		/**
		 * @brief Shows an information dialog
		 *  Displays a modal dialog with at most an OK button and some text.  Use instead of ReleaseWarning if
		 * debugging is not necassary and you just want to inform the player of something important and can't use
		 * other UI.  Use sparingly.
		 *
		 * @param filename The source code filename where this function was called
		 * @param line The source code line number where this function was called
		 * @param format The message to display
		 */
		void Information(const char* filename, int line, SCP_FORMAT_STRING const char* format, ...) SCP_FORMAT_STRING_ARGS(3, 4);


		void Message(MessageType type, const char* message, const char* title = NULL);
	}
}

// Make these available in the global namespace for compatibility
using os::dialogs::LuaError; //NOLINT
using os::dialogs::Error; //NOLINT
using os::dialogs::Warning; //NOLINT
using os::dialogs::ReleaseWarning; //NOLINT
using os::dialogs::WarningEx; //NOLINT
using os::dialogs::Information; //NOLINT

#endif // _OSAPI_DIALOGS_H

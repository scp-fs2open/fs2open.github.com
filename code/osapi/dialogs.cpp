
#include "osapi/dialogs.h"
#include "osapi/osapi.h"
#include "parse/parselo.h"
#include "cmdline/cmdline.h"
#include "graphics/2d.h"
#include "scripting/ade.h"

#include <SDL_messagebox.h>
#include <SDL_clipboard.h>

#include <string>
#include <algorithm>

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

namespace
{
	const char* Separator = "------------------------------------------------------------------\n";

	const int Messagebox_lines = 30;
	
	template<typename Stream>
	void LuaDebugPrint(Stream& stream, lua_Debug &ar)
	{
		if (ar.name == nullptr)
		{
			// Invalid lua_Debug struct
			return;
		}

		stream << "Name:\t\t" << ar.name << "\n";
		stream << "Name of:\t" << ar.namewhat << "\n";
		stream << "Function type:\t" << ar.what << "\n";
		stream << "Defined on:\t" << ar.linedefined << "\n";
		stream << "Upvalues:\t" << ar.nups << "\n";
		stream << "\n";
		stream << "Source:\t\t" << ar.source << "\n";
		stream << "Short source:\t" << ar.short_src << "\n";
		stream << "Current line:\t" << ar.currentline << "\n";
		stream << "- Function line:\t" << (ar.linedefined ? (1 + ar.currentline - ar.linedefined) : 0) << "\n";
	}

	SCP_string truncateLines(SCP_stringstream& s, int maxLines)
	{
		SCP_stringstream outStream;
		s.seekp(0, std::ios::beg);

		for (SCP_string line; std::getline(s, line);)
		{
			outStream << line << "\n";

			--maxLines;

			if (maxLines <= 0)
			{
				outStream << "[...]";
				break;
			}
		}

		return outStream.str();
	}

	const char* clean_filename(const char* filename)
	{
		auto separator = strrchr(filename, DIR_SEPARATOR_CHAR);
		if (separator != nullptr)
		{
			filename = separator + 1;
		}

		return filename;
	}

	void set_clipboard_text(const char* text)
	{
		// Make sure video is enabled
		if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
		{
			SDL_SetClipboardText(text);
		}
	}
}

int Global_warning_count = 0;
int Global_error_count = 0;

namespace os
{
	namespace dialogs
	{
		//This function is a helper function to determine if the dialogs should have a parent window
		//Normally, this should always be FSO's window, but there is an issue with this in fullscreen
		//where occasionally (when knossos is on the same screen as FSO in a multi-monitor setup on
		//windows) this results in the dialog losing focus, unable to regain it, thus rendering the 
		//buttons not clickable. So as a hotfix, the dialog has no blocking parent in that case and
		//is instead rendered as it's own window
		SDL_Window* getDialogParent() {
			return !(Cmdline_fullscreen_window || Cmdline_window) ? NULL : os::getSDLMainWindow();
		}

		void AssertMessage(const char * text, const char * filename, int linenum, const char * format, ...)
		{
			// We only want to display the file name
			filename = clean_filename(filename);

			SCP_stringstream msgStream;
			msgStream << "Assert: \"" << text << "\"\n";
			msgStream << "File: " << filename << "\n";
			msgStream << "Line: " << linenum << "\n";
			
			if (format != nullptr)
			{
				SCP_string buffer;
				va_list args;

				va_start(args, format);
				vsprintf(buffer, format, args);
				va_end(args);

				msgStream << buffer << "\n";
				mprintf(("ASSERTION: \"%s\" at %s:%d\n %s\n", text, filename, linenum, buffer.c_str()));
			}
			else
			{
				// No additional message
				mprintf(("ASSERTION: \"%s\" at %s:%d\n", text, filename, linenum));
			}

			if (running_unittests) {
				throw AssertException(msgStream.str());
			}

			msgStream << "\n";
			msgStream << dump_stacktrace();

			SCP_string messageText = msgStream.str();
			set_clipboard_text(messageText.c_str());

			messageText = truncateLines(msgStream, Messagebox_lines);
			messageText += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";
			messageText += "\n\nUse Debug to break into Debugger, Exit will close the application.\n";

			Error(messageText.c_str());
		}

		void LuaError(lua_State * L, const char * format, ...)
		{
			SCP_stringstream msgStream;
			
			//WMC - if format is set to NULL, assume this is acting as an
			//error handler for Lua.
			if (format == NULL)
			{
				msgStream << "LUA ERROR: " << lua_tostring(L, -1);
				lua_pop(L, -1);
			}
			else
			{
				SCP_string formatText;

				va_list args;
				va_start(args, format);
				vsprintf(formatText, format, args);
				va_end(args);

				msgStream << formatText;
			}

			msgStream << "\n";
			msgStream << "\n";

			msgStream << Separator;

			// Get the stack via the debug.traceback() function
			lua_getglobal(L, LUA_DBLIBNAME);

			if (!lua_isnil(L, -1))
			{
				msgStream << "\n";
				lua_getfield(L, -1, "traceback");
				lua_remove(L, -2);

				if (lua_pcall(L, 0, 1, 0) != 0)
					msgStream << "Error while retrieving stack: " << lua_tostring(L, -1);
				else
					msgStream << lua_tostring(L, -1);

				lua_pop(L, 1);
			}
			msgStream << "\n";

			msgStream << Separator;

			char stackText[1024];
			stackText[0] = '\0';
			scripting::ade_stackdump(L, stackText);
			msgStream << stackText;
			msgStream << "\n";
			msgStream << Separator;

			nprintf(("scripting","Lua Error: %s\n", msgStream.str().c_str()));

			if (running_unittests) {
				throw LuaErrorException(msgStream.str());
			}

			if (Cmdline_lua_devmode) {
				return;
			}

			if (Cmdline_noninteractive) {
				throw LuaErrorException(msgStream.str());
				return;
			}

			set_clipboard_text(msgStream.str().c_str());

			// truncate text
			auto truncatedText = truncateLines(msgStream, Messagebox_lines);

			SCP_stringstream boxTextStream;
			boxTextStream << truncatedText << "\n";

			boxTextStream << "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";

			auto boxText = boxTextStream.str();
			const SDL_MessageBoxButtonData buttons[] = {
				{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 2, "Exit" },
				{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Continue" },
				{ /* .flags, .buttonid, .text */        0, 0, "Debug" },
			};

			SDL_MessageBoxData boxData;
			memset(&boxData, 0, sizeof(boxData));

			boxData.buttons = buttons;
			boxData.numbuttons = 3;
			boxData.colorScheme = nullptr;
			boxData.flags = SDL_MESSAGEBOX_ERROR;
			boxData.message = boxText.c_str();
			boxData.title = "Error!";
			boxData.window = getDialogParent();

			gr_activate(0);

			int buttonId;
			if (SDL_ShowMessageBox(&boxData, &buttonId) < 0)
			{
				// Call failed
				buttonId = 1; // No action
			}

			switch (buttonId)
			{
			case 2:
				abort();

			case 0:
				Int3();
				break;

			default:
				break;
			}

			gr_activate(1);
		}

		void Error(const char * filename, int line, const char * format, ...)
		{
			SCP_string formatText;
			filename = clean_filename(filename);

			va_list args;
			va_start(args, format);
			vsprintf(formatText, format, args);
			va_end(args);

			SCP_stringstream messageStream;
			messageStream << "Error: " << formatText << "\n";
			messageStream << "File: " << filename << "\n";
			messageStream << "Line: " << line << "\n";

			Error(messageStream.str().c_str());
		}

		void Error(const char* text)
		{
			mprintf(("\n%s\n", text));

			if (running_unittests) {
				throw ErrorException(text);
			}

			if (Cmdline_noninteractive) {
				abort();
				return;
			}

			SCP_stringstream messageStream;
			messageStream << text << "\n";
			messageStream << dump_stacktrace();

			SCP_string fullText = messageStream.str();
			set_clipboard_text(fullText.c_str());

			fullText = truncateLines(messageStream, Messagebox_lines);

			fullText += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";
			fullText += "\n\nUse Debug to break into Debugger, Exit will close the application.\n";

			const SDL_MessageBoxButtonData buttons[] = {
				{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Exit" },
				{ /* .flags, .buttonid, .text */        0, 0, "Debug" },
			};

			SDL_MessageBoxData boxData;
			memset(&boxData, 0, sizeof(boxData));

			boxData.buttons = buttons;
			boxData.numbuttons = 2;
			boxData.colorScheme = nullptr;
			boxData.flags = SDL_MESSAGEBOX_ERROR;
			boxData.message = text;
			boxData.title = "Error!";
			boxData.window = getDialogParent();

			gr_activate(0);

			int buttonId;
			if (SDL_ShowMessageBox(&boxData, &buttonId) < 0)
			{
				// Call failed
				abort();
			}

			switch (buttonId)
			{
			case 1:
				abort();

			default:
				Int3();
				break;
			}
			gr_activate(1);
		}

		// Actual implementation of the warning function. Used by the various warning functions
		void WarningImpl(const char* filename, int line, const SCP_string& text)
		{
			filename = clean_filename(filename);

			// output to the debug log before anything else (so that we have a complete record)
			mprintf(("WARNING: \"%s\" at %s:%d\n", text.c_str(), filename, line));

			if (running_unittests) {
				throw WarningException(text);
			}

			// now go for the additional popup window, if we want it ...
			if (Cmdline_noninteractive) {
				return;
			}

			SCP_stringstream boxMsgStream;
			boxMsgStream << "Warning: " << text << "\n";
			boxMsgStream << "File: " << filename << "\n";
			boxMsgStream << "Line: " << line << "\n";

			set_clipboard_text(boxMsgStream.str().c_str());

			boxMsgStream << "\n";

			SCP_string boxMessage = truncateLines(boxMsgStream, Messagebox_lines);
			boxMessage += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";
			boxMessage += "\n\nUse Debug to break into Debugger\n";

			const SDL_MessageBoxButtonData buttons[] = {
				{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 2, "Exit" },
				{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Continue" },
				{ /* .flags, .buttonid, .text */        0, 0, "Debug" },
			};

			SDL_MessageBoxData boxData;
			memset(&boxData, 0, sizeof(boxData));

			boxData.buttons = buttons;
			boxData.numbuttons = 3;
			boxData.colorScheme = nullptr;
			boxData.flags = SDL_MESSAGEBOX_WARNING;
			boxData.message = boxMessage.c_str();
			boxData.title = "Warning!";
			boxData.window = getDialogParent();

			gr_activate(0);

			int buttonId;
			if (SDL_ShowMessageBox(&boxData, &buttonId) < 0)
			{
				// Call failed
				buttonId = 1; // No action
			}

			switch (buttonId)
			{
			case 2:
				abort();

			case 0:
				Int3();
				break;

			default:
				break;
			}

			gr_activate(1);
		}


		void ReleaseWarning(const char* filename, int line, const char* format, ...) {
			Global_warning_count++;

			SCP_string msg;
			va_list args;

			va_start(args, format);
			vsprintf(msg, format, args);
			va_end(args);

			WarningImpl(filename, line, msg);
		}
		
		void Warning(const char* filename, int line, const char* format, ...)
		{
			Global_warning_count++;

#ifndef NDEBUG
			SCP_string msg;
			va_list args;

			va_start(args, format);
			vsprintf(msg, format, args);
			va_end(args);

			WarningImpl(filename, line, msg);
#endif
		}

		void WarningEx(const char* filename, int line, const char* format, ...)
		{
#ifndef NDEBUG
			if (Cmdline_extra_warn) {
				SCP_string msg;
				va_list args;

				va_start(args, format);
				vsprintf(msg, format, args);
				va_end(args);

				Warning(filename, line, "%s", msg.c_str());
			}
#endif
		}

		void Information(const char* filename, int line, const char* format, ...) {
			SCP_string msg;
			va_list args;

			va_start(args, format);
			vsprintf(msg, format, args);
			va_end(args);

			// Below is essentially a stripped down copy pasta of WarningImpl
			filename = clean_filename(filename);

			// output to the debug log before anything else (so that we have a complete record)
			mprintf(("INFO: \"%s\" at %s:%d\n", msg.c_str(), filename, line));

			// now go for the additional popup window, if we want it ...
			if (Cmdline_noninteractive || running_unittests) {
				return;
			}

			SCP_stringstream boxMsgStream;
			boxMsgStream << "Information: " << msg << "\n";
			boxMsgStream << "File: " << filename << "\n";
			boxMsgStream << "Line: " << line << "\n";

			set_clipboard_text(boxMsgStream.str().c_str());

			boxMsgStream << "\n";

			SCP_string boxMessage = truncateLines(boxMsgStream, Messagebox_lines);
			boxMessage += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";

			gr_activate(0);

			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Information", boxMessage.c_str(), getDialogParent());

			gr_activate(1);
		}

		void Message(MessageType type, const char* message, const char* title)
		{
			if (running_unittests) {
				throw WarningException(message);
			}

			int flags = 1;

			switch (type) 
			{
				case MESSAGEBOX_ERROR:
					flags = SDL_MESSAGEBOX_ERROR;
					if (title == NULL)
						title = "Error";
					break;
				case MESSAGEBOX_INFORMATION:
					flags = SDL_MESSAGEBOX_INFORMATION;
					if (title == NULL)
						title = "Information";
					break;
				case MESSAGEBOX_WARNING:
					flags = SDL_MESSAGEBOX_WARNING;
					if (title == NULL)
						title = "Warning";
					break;
				default:
					Int3();
					title = ""; // Remove warning about unitialized variable
					break;
			}

			SDL_ShowSimpleMessageBox(flags, title, message, getDialogParent());
		}
	}
}

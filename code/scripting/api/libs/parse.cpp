//
//

#include "parse.h"
#include "cfile.h"
#include "cfile/cfile.h"
#include "parse/parselo.h"

namespace scripting {
namespace api {

ADE_LIB(l_Parsing, "Parsing", "parse", "Engine parsing library");

ADE_FUNC(readFileText, l_Parsing, "string file, string directory = <any>",
         "Reads the text of the given file into the parsing system. If a directory is given then the file is read from "
         "that location.",
         "boolean", "true if the operation was successful, false otherwise")
{
	const char* file;
	const char* dir = nullptr;
	if (!ade_get_args(L, "s|s", &file, &dir)) {
		return ADE_RETURN_FALSE;
	}

	int type = CF_TYPE_ANY;
	if (dir != nullptr) {
		type = l_cf_get_path_id(dir);
	}

	if (type == CF_TYPE_INVALID) {
		return ADE_RETURN_FALSE;
	}

	try {
		read_file_text(file, type);
		reset_parse();
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(stop, l_Parsing, nullptr, "Stops parsing and frees any allocated resources.", "boolean",
         "true if the operation was successful, false otherwise")
{
	try {
		stop_parse();
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(displayMessage, l_Parsing, "string message, boolean error = false",
         "Displays a message dialog which includes the current file name and line number. If <i>error</i> is set the "
         "message will be displayed as an error.",
         "boolean", "true if the operation was successful, false otherwise")
{
	if (Parse_text == nullptr) {
		LuaError(L, "Parsing system is currently not active!");
		return ADE_RETURN_FALSE;
	}

	const char* message;
	bool error = false;
	if (!ade_get_args(L, "s|b", &message, &error)) {
		return ADE_RETURN_FALSE;
	}

	try {
		error_display(error ? 1 : 0, "%s", message);
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(requiredString, l_Parsing, "string token", "Require that a string appears at the current position.", "boolean",
         "true if the operation was successful, false otherwise")
{
	if (Parse_text == nullptr) {
		LuaError(L, "Parsing system is currently not active!");
		return ADE_RETURN_FALSE;
	}

	const char* str;
	if (!ade_get_args(L, "s", &str)) {
		return ADE_RETURN_FALSE;
	}

	try {
		required_string(str);
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(optionalString, l_Parsing, "string token", "Check if the string appears at the current position in the file.",
         "boolean", "true if the token is present, false otherwise")
{
	if (Parse_text == nullptr) {
		LuaError(L, "Parsing system is currently not active!");
		return ADE_RETURN_FALSE;
	}

	const char* str;
	if (!ade_get_args(L, "s", &str)) {
		return ADE_RETURN_FALSE;
	}

	try {
		if (optional_string(str)) {
			return ADE_RETURN_TRUE;
		} else {
			return ADE_RETURN_FALSE;
		}
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(getString, l_Parsing, nullptr, "Gets a single line of text from the file", "string", "Text or nil on error")
{
	if (Parse_text == nullptr) {
		LuaError(L, "Parsing system is currently not active!");
		return ADE_RETURN_NIL;
	}

	try {
		SCP_string str;
		stuff_string(str, F_NAME);

		return ade_set_args(L, "s", str.c_str());
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(getFloat, l_Parsing, nullptr, "Gets a floating point number from the file", "string", "number or nil on error")
{
	if (Parse_text == nullptr) {
		LuaError(L, "Parsing system is currently not active!");
		return ADE_RETURN_NIL;
	}

	try {
		float f;
		stuff_float(&f);

		return ade_set_args(L, "f", f);
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(getInt, l_Parsing, nullptr, "Gets an integer number from the file", "string", "number or nil on error")
{
	if (Parse_text == nullptr) {
		LuaError(L, "Parsing system is currently not active!");
		return ADE_RETURN_NIL;
	}

	try {
		int i;
		stuff_int(&i);

		return ade_set_args(L, "i", i);
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(getBoolean, l_Parsing, nullptr, "Gets a boolean value from the file", "boolean", "boolean value or nil on error")
{
	if (Parse_text == nullptr) {
		LuaError(L, "Parsing system is currently not active!");
		return ADE_RETURN_NIL;
	}

	try {
		bool b;
		stuff_boolean(&b);

		return ade_set_args(L, "b", b);
	} catch (const parse::ParseException& e) {
		mprintf(("PARSE: Error while parsing: %s\n", e.what()));
		return ADE_RETURN_NIL;
	}
}

} // namespace api
} // namespace scripting

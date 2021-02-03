
#include "scripting/ade_args.h"

#include "scripting.h"

#include "mod_table/mod_table.h"
#include "scripting/ade.h"
#include "scripting/lua/LuaConvert.h"
#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaTable.h"

#include <utf8.h>

namespace scripting {

namespace internal {

// WMC - hack to skip X number of arguments on the stack
// Lets me use ade_get_args for global hook return values
int Ade_get_args_skip       = 0;
bool Ade_get_args_lfunction = false;

bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, bool* b)
{
	Assertion(fmt == 'b', "Invalid character '%c' for boolean type!", fmt);

	if (lua_isboolean(L, state.nargs)) {
		*b = lua_toboolean(L, state.nargs) > 0;
	} else {
		LuaError(L, "%s: Argument %d is an invalid type '%s'; boolean expected", state.funcname, state.nargs,
		         ade_get_type_string(L, state.nargs));
		return false;
	}
	return true;
}
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, const char** s)
{
	Assertion(fmt == 's', "Invalid character '%c' for string type!", fmt);

	if (lua_isstring(L, state.nargs)) {
		auto value = lua_tostring(L, state.nargs);

		if (Unicode_text_mode) {
			// Validate the string when we are in unicode mode to ensure that FSO doesn't just crash when a
			// script passes an invalid UTF-8 sequence to the API
			auto end     = value + strlen(value);
			auto invalid = utf8::find_invalid(value, end);

			if (invalid != end) {
				if (invalid == value) {
					// The first character is the invalid sequence
					LuaError(L, "An invalid UTF-8 encoding sequence was detected! The first invalid character "
					            "was as the start of the string.");
				} else {
					// If the invalid sequence is inside the string then we try to give some context to make
					// finding the bug easier
					auto display_text_start = std::max(value, invalid - 32);
					SCP_string context_text(display_text_start, invalid);

					LuaError(L,
					         "An invalid UTF-8 encoding sequence was detected! The error was detected directly "
					         "after this string \"%s\".",
					         context_text.c_str());
				}

				// Finally, assign a default value so that we can continue
				value = "Invalid UTF-8 sequence detected!";
			}
		}

		*s = value;
	} else {
		LuaError(L, "%s: Argument %d is an invalid type '%s'; string expected", state.funcname, state.nargs,
		         ade_get_type_string(L, state.nargs));
		return false;
	}
	return true;
}
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, luacpp::LuaTable* t)
{
	Assertion(fmt == 't', "Invalid character '%c' for table type!", fmt);

	// Get a table
	if (!luacpp::convert::popValue(L, *t, state.nargs, false)) {
		LuaError(L, "%s: Argument %d is an invalid type '%s'; table expected.", state.funcname, state.nargs,
		         ade_get_type_string(L, state.nargs));
		return false;
	}
	return true;
}
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, luacpp::LuaFunction* f)
{
	Assertion(fmt == 'u', "Invalid character '%c' for function type!", fmt);

	// Get a function
	if (!luacpp::convert::popValue(L, *f, state.nargs, false)) {
		LuaError(L, "%s: Argument %d is an invalid type '%s'; function expected.", state.funcname, state.nargs,
		         ade_get_type_string(L, state.nargs));
		return false;
	}
	// Automatically assign our error function to function values retrieved from the API
	f->setErrorFunction(luacpp::LuaFunction::createFromCFunction(L, ade_friendly_error));
	return true;
}
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, luacpp::LuaValue* f)
{
	Assertion(fmt == 'a', "Invalid character '%c' for any type!", fmt);

	// Get a function
	if (!luacpp::convert::popValue(L, *f, state.nargs, false)) {
		LuaError(L, "%s: Failed to get argument %d. Internal error.", state.funcname, state.nargs);
		return false;
	}
	return true;
}

void set_single_arg(lua_State* L, char fmt, const char* s)
{
	Assertion(fmt == 's', "Invalid format character '%c' for string type!", fmt);
	// WMC - Isn't working with HookVar for some strange reason
	lua_pushstring(L, s);
}

void set_single_arg(lua_State* L, char fmt, const SCP_string& s)
{
	Assertion(fmt == 's', "Invalid format character '%c' for string type!", fmt);
	// WMC - Isn't working with HookVar for some strange reason
	lua_pushlstring(L, s.c_str(), s.size());
}

void set_single_arg(lua_State* L, char fmt, luacpp::LuaTable* table) { set_single_arg(L, fmt, *table); }
void set_single_arg(lua_State* L, char fmt, const luacpp::LuaTable& table)
{
	Assertion(fmt == 't', "Invalid format character '%c' for table type!", fmt);
	table.pushValue(L);
}

void set_single_arg(lua_State* L, char fmt, luacpp::LuaFunction* func)
{
	set_single_arg(L, fmt, *func);
}
void set_single_arg(lua_State* L, char fmt, const luacpp::LuaFunction& func)
{
	Assertion(fmt == 'u', "Invalid format character '%c' for function type!", fmt);
	func.pushValue(L);
}
void set_single_arg(lua_State* L, char fmt, const luacpp::LuaValue& func)
{
	Assertion(fmt == 'a', "Invalid format character '%c' for any type!", fmt);
	func.pushValue(L);
}

} // namespace internal
}

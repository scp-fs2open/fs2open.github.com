
#include "scripting/ade_args.h"
#include "scripting/ade.h"

#include "scripting/lua/LuaTable.h"
#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaConvert.h"

#include "mod_table/mod_table.h"

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
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, ade_odata od)
{
	Assertion(fmt == 'o', "Invalid character '%c' for object type!", fmt);

	if (lua_isuserdata(L, state.nargs)) {
		// Use the helper function
		if (!luacpp::convert::popValue(L, od, state.nargs, false)) {
			return false;
		}
	} else if (lua_isnil(L, state.nargs) && state.optional_args) {
		// WMC - Modder has chosen to ignore this argument
	} else {
		LuaError(L, "%s: Argument %d is an invalid type '%s'; type '%s' expected", state.funcname, state.nargs,
		         ade_get_type_string(L, state.nargs), internal::getTableEntry(od.idx).GetName());
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

bool get_args_actual(lua_State*, get_args_state&, const char* fmt)
{
	Assertion(strlen(fmt) == 0, "No class parameters left but format is not empty!");
	return true;
}
}

//ade_set_args(state, arguments, variables)
//----------------------------------------------
//based on "Programming in Lua"
//
//Takes variables given and pushes them onto the
//Lua stack. Use it to return variables from a
//Lua scripting function.
//
//NOTE: You can also use this to push arguments
//on to the stack in series. See script_state::SetHookVar
int ade_set_args(lua_State *L, const char *fmt, ...)
{
	//Start throught
	va_list vl;
	int nargs;
	int setargs;	//args actually set

	va_start(vl, fmt);
	nargs = 0;
	setargs = 0;
	while(*fmt != '\0')
	{
		switch(*fmt++)
		{
			case '*':
				lua_pushnil(L);
				break;
			case 'b':	//WMC - Bool is actually int for GCC (Why...?)
				lua_pushboolean(L, va_arg(vl, int) ? 1 : 0);
				break;
			case 'd':
				lua_pushnumber(L, va_arg(vl, double));
				break;
			case 'f':
				lua_pushnumber(L, va_arg(vl, double));
				break;
			case 'i':
				lua_pushnumber(L, va_arg(vl, int));
				break;
			case 's':
			{
				//WMC - Isn't working with HookVar for some strange reason
				char *s = va_arg(vl, char*);
				lua_pushstring(L, s);
				break;
			}
			case 'x':
				lua_pushnumber(L, f2fl(va_arg(vl, fix)));
				break;
			case 'o':
			{
				//Copy over objectdata
				ade_odata od = (ade_odata) va_arg(vl, ade_odata);

				// Use the common helper method
				luacpp::convert::pushValue(L, od);
				break;
			}
			case 't':
			{
				// Set a table
				luacpp::LuaTable* table = va_arg(vl, luacpp::LuaTable*); // This must be a pointer since C++ classes can't be passed by vararg
				table->pushValue();
				break;
			}
			case 'u':
			{
				// Set a function
				luacpp::LuaFunction* func = va_arg(vl, luacpp::LuaFunction*); // This must be a pointer since C++ classes can't be passed by vararg
				func->pushValue();
				break;
			}
				//WMC -  Don't forget to update lua_set_arg
			default:
				Error(LOCATION, "Bad character passed to ade_set_args; (%c)", *(fmt-1));
				setargs--;
		}
		nargs++;
		setargs++;
	}
	va_end(vl);
	return setargs;
}

}

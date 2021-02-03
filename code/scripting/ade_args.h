//
//

#ifndef FS2_OPEN_ADE_ARGS_H
#define FS2_OPEN_ADE_ARGS_H

#include "globalincs/pstypes.h"
#include "mod_table/mod_table.h"
#include "scripting/ade.h"
#include "scripting/lua/LuaConvert.h"
#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaTable.h"
#include <utf8/checked.h>

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

namespace scripting {

namespace internal {

//*************************Lua hacks*************************
// WMC - Hack to allow for quick&easy return value parsing
extern int Ade_get_args_skip;
// WMC - Tell ade_get_args it is parsing scripting functions,
// which have no upvalues
extern bool Ade_get_args_lfunction;

struct get_args_state {
	int nargs        = -1;
	int counted_args = -1;

	int needed_args = -1;
	int total_args  = -1;

	bool optional_args = false;

	char funcname[128] = "\0";
};

bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, bool* b);
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
get_single_arg(lua_State* L, const get_args_state& state, char fmt, T* f)
{
	Assertion(fmt == 'f' || fmt == 'd', "Invalid character '%c' for number type!", fmt);

	if (lua_isnumber(L, state.nargs)) {
		*f = (T)lua_tonumber(L, state.nargs);
	} else {
		LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", state.funcname, state.nargs,
		         ade_get_type_string(L, state.nargs));
		return false;
	}
	return true;
}
template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
get_single_arg(lua_State* L, const get_args_state& state, char fmt, T* i)
{
	// fix is also an int for C++ so we need to check the format character to determine what should be done
	Assertion(fmt == 'i' || fmt == 'x', "Invalid character '%c' for number type!", fmt);

	if (lua_isnumber(L, state.nargs)) {
		if (fmt == 'x') {
			*i = (T)fl2f((float)lua_tonumber(L, state.nargs));
		} else {
			*i = (T)lua_tonumber(L, state.nargs);
		}
	} else {
		LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", state.funcname, state.nargs,
		         ade_get_type_string(L, state.nargs));
		return false;
	}
	return true;
}
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, const char** s);

template <typename T>
bool ade_odata_getter_helper(lua_State* L, const get_args_state& state, char fmt, T&& od) {
	Assertion(fmt == 'o', "Invalid character '%c' for object type!", fmt);

	if (lua_isuserdata(L, state.nargs)) {
		// Use the helper function
		if (!luacpp::convert::popValue(L, std::forward<T>(od), state.nargs, false)) {
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
template <typename T>
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, ade_odata_getter<T>&& od)
{
	return ade_odata_getter_helper(L, state, fmt, std::forward<ade_odata_getter<T>>(od));
}
template <typename T>
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, ade_odata_ptr_getter<T>&& od)
{
	return ade_odata_getter_helper(L, state, fmt, std::forward<ade_odata_ptr_getter<T>>(od));
}

bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, luacpp::LuaTable* t);
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, luacpp::LuaFunction* f);
bool get_single_arg(lua_State* L, const get_args_state& state, char fmt, luacpp::LuaValue* f);

// This is not a template function so we can put the implementation in a source file
inline bool get_args_actual(lua_State* /*L*/, get_args_state& state, const char* fmt)
{
	while (*fmt != '\0') {
		switch(*fmt) {
		case '|':
			state.optional_args = true;
			break;
		case '*':
			state.nargs++;
			state.counted_args++;
			break;
		default:
			UNREACHABLE("Invalid format string '%s'!", fmt);
			return false;
		}

		// Ignored parameters are still valid here so we skip them until we reach the end
		++fmt;
	}
	// Now there should be nothing left in the parameter string
	Assertion(strlen(fmt) == 0, "No class parameters left but format is not empty!");
	return true;
}

template <typename T, typename... Args>
bool get_args_actual(lua_State* L, get_args_state& state, const char* fmt, T&& current, Args&&... args)
{
	Assertion(strlen(fmt) > 0, "Format was empty but there were still parameters in the argument list!");

	if (state.nargs > state.total_args) {
		return true;
	}

	if (*fmt == '*') {
		state.nargs++;
		state.counted_args++;
		return get_args_actual(L, state, fmt + 1, std::forward<T>(current), std::forward<Args>(args)...);
	}

	if (*fmt == '|') {
		state.optional_args = true;
		return get_args_actual(L, state, fmt + 1, std::forward<T>(current), std::forward<Args>(args)...);
	}

	if (!get_single_arg(L, state, *fmt, std::forward<T>(current))) {
		if (!state.optional_args) {
			return false;
		}
	}

	state.nargs++;
	state.counted_args++;
	return get_args_actual(L, state, fmt + 1, std::forward<Args>(args)...);
}

} // namespace internal

/**
 * @brief Parses arguments from the Lua stack
 *
 * based on "Programming in Lua"
 *
 * Parses arguments from string to variables given
 * a '|' divides required and optional arguments.
 * Returns 0 if a required argument is invalid,
 * or there are too few arguments actually passed
 *
 * @note This function essentially takes objects
 * from the stack in series, so it can easily be used
 * to get the return values from a chunk of Lua code
 * after it has been executed. See RunByteCode()
 *
 * @param L The lua state
 * @param fmt The argument format string
 * @param ... The parameters
 * @return The number of arguments taken from the stack or 0 on error
 */
template <typename... Args>
inline int ade_get_args(lua_State* L, const char* fmt, Args&&... args)
{
	// Capture these variables locally and reset them. This is needed in case we indirectly call this function again
	// from below which would cause some weird issues. This can happen if we call a Lua API function which then causes a
	// GC run which then calls one of our destructors.
	const auto get_args_skip = internal::Ade_get_args_skip;
	const auto get_args_lfunction = internal::Ade_get_args_lfunction;
	internal::Ade_get_args_skip = 0;
	internal::Ade_get_args_lfunction = false;

	// Check that we have all the arguments that we need
	// If we don't, return 0
	internal::get_args_state state;
	state.needed_args = (int)strlen(fmt);
	state.total_args  = lua_gettop(L) - get_args_skip;

	if (strchr(fmt, '|') != nullptr) {
		state.needed_args = (int)(strchr(fmt, '|') - fmt);
	}

	memset(state.funcname, 0, sizeof(state.funcname));
#ifndef NDEBUG
	lua_Debug ar;
	memset(&ar, 0, sizeof(ar));
	if (lua_getstack(L, 0, &ar)) {
		lua_getinfo(L, "nl", &ar);
		strcpy_s(state.funcname, "");
		if (ar.name != nullptr) {
			strcat_s(state.funcname, ar.name);
		}
		if (ar.currentline > -1) {
			char buf[33];
			sprintf(buf, "%d", ar.currentline);
			strcat_s(state.funcname, " (Line ");
			strcat_s(state.funcname, buf);
			strcat_s(state.funcname, ")");
		}
	}
#endif
	if (!strlen(state.funcname)) {
		// WMC - This was causing crashes with user-defined functions.
		// WMC - Try and get at function name from upvalue
		if (!get_args_lfunction && !lua_isnone(L, lua_upvalueindex(ADE_FUNCNAME_UPVALUE_INDEX))) {
			if (lua_type(L, lua_upvalueindex(ADE_FUNCNAME_UPVALUE_INDEX)) == LUA_TSTRING)
				strcpy_s(state.funcname, lua_tostring(L, lua_upvalueindex(ADE_FUNCNAME_UPVALUE_INDEX)));
		}

		// WMC - Totally unknown function
		if (!strlen(state.funcname)) {
			strcpy_s(state.funcname, "<UNKNOWN>");
		}
	}
	if (state.total_args < state.needed_args) {
		LuaError(L,
		         "Not enough arguments for '%s' - need %d, had %d. If you are using objects or handles, make sure that "
		         "you are using \":\" to access member functions, rather than \".\"",
		         state.funcname, state.needed_args, state.total_args);
		return 0;
	}

	// Start throught
	state.counted_args = 0;

	// Are we parsing optional args yet?
	state.optional_args = false;

	state.nargs = 1 + get_args_skip;
	state.total_args += get_args_skip;
	if (!internal::get_args_actual(L, state, fmt, std::forward<Args>(args)...)) {
		return 0;
	}

	return state.counted_args;
}

namespace internal {
struct set_args_state {
	int nargs;
	int setargs; // args actually set
};

// This is an extremely complicated way of telling the compiler that this function should not be called with pointers
template <typename T>
typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, bool>::value, void>::type
set_single_arg(lua_State* L, char fmt, T b)
{
	Assertion(fmt == 'b', "Invalid format character '%c' for boolean type!", fmt);
	lua_pushboolean(L, b ? 1 : 0);
}
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type set_single_arg(lua_State* L, char fmt, T f)
{
	Assertion(fmt == 'f' || fmt == 'd', "Invalid character '%c' for number type!", fmt);
	lua_pushnumber(L, static_cast<lua_Number>(f));
}
template <typename T>
typename std::enable_if<!std::is_same<T, bool>::value && std::is_integral<T>::value, void>::type
set_single_arg(lua_State* L, char fmt, T i)
{
	// fix is also an int for C++ so we need to check the format character to determine what should be done
	Assertion(fmt == 'i' || fmt == 'x', "Invalid character '%c' for number type!", fmt);

	if (fmt == 'x') {
		lua_pushnumber(L, static_cast<lua_Number>(f2fl((fix)i)));
	} else {
		lua_pushinteger(L, static_cast<lua_Integer>(i));
	}
}
void set_single_arg(lua_State* L, char fmt, const char* s);
void set_single_arg(lua_State* L, char fmt, const SCP_string& s);
template<typename T>
void set_single_arg(lua_State* L, char fmt, ade_odata_setter<T>&& od)
{
	Assertion(fmt == 'o', "Invalid format character '%c' for object type!", fmt);
	// Use the common helper method
	luacpp::convert::pushValue(L, std::forward<ade_odata_setter<T>>(od));
}
void set_single_arg(lua_State* /*L*/, char fmt, luacpp::LuaTable* table);
void set_single_arg(lua_State* /*L*/, char fmt, const luacpp::LuaTable& table);

void set_single_arg(lua_State* /*L*/, char fmt, luacpp::LuaFunction* func);
void set_single_arg(lua_State* /*L*/, char fmt, const luacpp::LuaFunction& func);

void set_single_arg(lua_State* /*L*/, char fmt, const luacpp::LuaValue& func);

// This is not a template function so we can put the implementation in a source file
inline void set_args_actual(lua_State* /*L*/, set_args_state& /*state*/, const char* fmt)
{
	Assertion(strlen(fmt) == 0, "No class parameters left but format is not empty!");
}

template <typename T, typename... Args>
void set_args_actual(lua_State* L, set_args_state& state, const char* fmt, T&& current, Args&&... args)
{
	Assertion(strlen(fmt) > 0, "Format was empty but there were still parameters in the argument list!");

	if (*fmt == '*') {
		lua_pushnil(L);
		state.nargs++;
		state.setargs++;
		return set_args_actual(L, state, fmt + 1, std::forward<T>(current), std::forward<Args>(args)...);
	}

	set_single_arg(L, *fmt, std::forward<T>(current));

	state.nargs++;
	state.setargs++;
	return set_args_actual(L, state, fmt + 1, std::forward<Args>(args)...);
}
} // namespace internal

// ade_set_args(state, arguments, variables)
//----------------------------------------------
// based on "Programming in Lua"
//
// Takes variables given and pushes them onto the
// Lua stack. Use it to return variables from a
// Lua scripting function.
//
// NOTE: You can also use this to push arguments
// on to the stack in series. See script_state::SetHookVar
template <typename... Args>
int ade_set_args(lua_State* L, const char* fmt, Args&&... args)
{
	// Start throught
	internal::set_args_state state{};
	state.nargs   = 0;
	state.setargs = 0;

	internal::set_args_actual(L, state, fmt, std::forward<Args>(args)...);

	return state.setargs;
}

/**
 * @brief Return a value in error
 *
 * Should be used if the value is not valid
 *
 * @ingroup ade_api
 */
#define ade_set_error ade_set_args
}

#endif //FS2_OPEN_ADE_ARGS_H


#ifndef LUA_ARGS_H
#define LUA_ARGS_H
#pragma once

#include "LuaHeaders.h"
#include "LuaException.h"
#include "LuaConvert.h"

#include <type_traits>

namespace luacpp {
namespace args {
class ArgumentException: public LuaException {
 public:

	ArgumentException(const std::string& message = "Argument Error!") throw() : LuaException(message) {
	}

	virtual ~ArgumentException() throw() {
	}
};

class opt {};

extern opt optional;

inline int getArgsInternal(lua_State* L, bool, int, int) {
	return 0;
}

template<typename T>
inline int popArgumentValue(lua_State* L, T& target, bool& optionalOut, int& stackIndexOut) {
	convert::popValue(L, target, stackIndexOut, false);
	stackIndexOut = stackIndexOut + 1;

	return 1;
}

template<>
inline int popArgumentValue<opt>(lua_State* L, opt& target, bool& optionalOut, int& stackIndexOut) {
	optionalOut = true;

	return 0;
}

template<typename T, typename ...Args>
inline int getArgsInternal(lua_State* L, bool optional_arg, int stackIndex, int numArgs, T& target, Args& ... args) {
	if (!std::is_same<T, opt>::value) {
		if (stackIndex > numArgs) {
			// No more arguments there
			if (optional_arg) {
				// Everything is fine, this argument is optional
				return 0;
			} else {
				// No good, no more arguments but no optional
				throw ArgumentException("Not enough arguments!");
			}
		}
	}

	return popArgumentValue(L, target, optional_arg, stackIndex) +
		getArgsInternal(L, optional_arg, stackIndex, numArgs, args...);
}

template<typename T, typename ...Args>
int getArgs(lua_State* L, T& target, Args& ... args) {
	return getArgsInternal(L, false, 1, lua_gettop(L), target, args...);
}
}
}

#endif

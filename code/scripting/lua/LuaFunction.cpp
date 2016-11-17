#include "LuaFunction.h"
#include "LuaException.h"

#include "LuaHeaders.h"

namespace luacpp {
LuaFunction LuaFunction::createFromCFunction(lua_State* L, lua_CFunction function, const LuaValueList& upvalues) {
	LuaFunction func;

	for (auto& val : upvalues) {
		val.pushValue();
	}

	lua_pushcclosure(L, function, (int) upvalues.size());

	func.setReference(UniqueLuaReference::create(L));

	lua_pop(L, 1);

	return func;
}

LuaFunction LuaFunction::createFromCode(lua_State* L, std::string const& code, std::string const& name) {
	int load_err = luaL_loadbuffer(L, code.c_str(), code.length(), name.c_str());

	if (!load_err) {
		LuaFunction func;

		func.setReference(UniqueLuaReference::create(L));

		lua_pop(L, 1);

		return func;
	} else {
		// Get the error message
		size_t len;
		const char* err = lua_tolstring(L, -1, &len);

		lua_pop(L, 1);

		throw LuaException(std::string(err, len));
	}
}

LuaFunction::LuaFunction() : LuaValue(), _isCFunction(false), _errorFunction(nullptr) {
}

LuaFunction::LuaFunction(const LuaFunction& other) : LuaValue(other), _errorFunction(nullptr) {
}

LuaFunction::~LuaFunction() {
}

bool LuaFunction::setEnvironment(const LuaTable& table) {
	if (!table.getReference()->isValid()) {
		throw LuaException("Table reference is not valid!");
	}

	this->pushValue();
	table.pushValue();

	bool ret = lua_setfenv(_luaState, -2) != 0;

	// Pop the function again
	lua_pop(_luaState, 1);

	return ret;
}

LuaValueList LuaFunction::operator()(const LuaValueList& args) {
	return this->call(args);
}

void LuaFunction::setReference(LuaReference ref) {
	ref->pushValue();

	lua_State* L = ref->getState();

	if (lua_type(L, -1) != LUA_TFUNCTION) {
		lua_pop(L, 1);
		throw LuaException("Reference does not refere to a function!");
	} else {
		lua_pop(L, 1);
		LuaValue::setReference(ref);
	}
}

LuaValueList LuaFunction::call(const LuaValueList& args) {
	int err_idx = 0;
	int stackTop;

	if (_errorFunction) {
		// push the error function
		_errorFunction->pushValue();
		err_idx = lua_gettop(_luaState);
		stackTop = err_idx;
	} else {
		stackTop = lua_gettop(_luaState);
	}

	// Push the function onto the stack
	this->pushValue();

	// Push the arguments onto the stack
	for (LuaValueList::const_iterator iter = args.begin(); iter != args.end(); ++iter) {
		iter->pushValue();
	}

	// actually call the function now!
	int err = lua_pcall(_luaState, (int) args.size(), LUA_MULTRET, err_idx);

	if (!err) {
		int numReturn = lua_gettop(_luaState) - stackTop;
		LuaValueList values;
		values.reserve(numReturn);

		LuaValue val;
		for (int i = 0; i < numReturn; ++i) {
			if (convert::popValue(_luaState, val)) {
				// Add values at the begin as the last return value is on top
				// of the stack.
				values.insert(values.begin(), val);
			}
		}

		if (err_idx != 0) {
			// Remove the error function
			lua_pop(_luaState, 1);
		}

		return values;
	} else {
		// Make sure that there is exactly one parameter left on the stack
		// If the error function didn't return anything then this will push nil
		// If it pushed more than one value then this will discard all of them except the last one
		lua_settop(_luaState, stackTop + 1);

		std::string err_msg;
		if (!lua_isstring(_luaState, -1)) {
			err_msg = "Invalid lua value on stack!";
			lua_pop(_luaState, 1); // Remove the value on the stack
		} else {
			err_msg = convert::popValue<std::string>(_luaState);
		}

		// Throw exception with generated message
		LuaException exception(err_msg);

		if (err_idx != 0) {
			// Pop the error function
			lua_pop(_luaState, 1);
		}

		throw exception;
	}
}
}
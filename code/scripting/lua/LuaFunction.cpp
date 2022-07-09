#include "LuaFunction.h"
#include "LuaException.h"

#include "LuaHeaders.h"

namespace luacpp {
namespace {

int stdFunctionDestructor(lua_State* L)
{
	// The userdata is the function object so we call the destructor here to free up the internal resources
	const auto luaPtr = lua_touserdata(L, 1);
	const auto funcPtr = static_cast<LuaFunctionObject*>(luaPtr);

	funcPtr->~LuaFunctionObject();

	return 0;
}

int stdFunctionCaller(lua_State* L)
{
	const auto numArgs = lua_gettop(L);

	// Push the actual function object on the stack
	lua_pushvalue(L, lua_upvalueindex(1));

	const auto funcObj = static_cast<LuaFunctionObject*>(lua_touserdata(L, -1));
	lua_pop(L, 1); // We have the pointer so we can remove this from the stack again

	LuaValueList params;
	params.reserve(numArgs);

	for (int i = 1; i <= numArgs; ++i) {
		LuaValue val;
		val.setReference(UniqueLuaReference::create(L, i));
		params.push_back(val);
	}

	const auto ret = (*funcObj)(L, params);

	// Prepare to return values. Clear the stack
	lua_settop(L, 0);
	for (const auto& retVal : ret)
	{
		retVal.pushValue(L);
	}

	// Done!
	return static_cast<int>(ret.size());
}

}

LuaFunction LuaFunction::createFromCFunction(lua_State* L, lua_CFunction function, const LuaValueList& upvalues)
{
	LuaFunction func;

	for (auto& val : upvalues) {
		val.pushValue(L);
	}

	lua_pushcclosure(L, function, (int)upvalues.size());

	func.setReference(UniqueLuaReference::create(L));

	lua_pop(L, 1);

	return func;
}
LuaFunction LuaFunction::createFromStdFunction(lua_State* L, LuaFunctionObject function) {
	auto userdataMetatable = LuaTable::create(L);
	userdataMetatable.addValue("__gc", stdFunctionDestructor);

	// Create storage for the function object in Lua and initialize our data there via placement new
	auto functionStorage = lua_newuserdata(L, sizeof(function));
	new (functionStorage) LuaFunctionObject(std::move(function));

	userdataMetatable.pushValue(L);
	lua_setmetatable(L, -2);

	LuaValue funcValue;
	funcValue.setReference(UniqueLuaReference::create(L));
	lua_pop(L, 1); // Value is referenced. We can remove it from the stack now

	return LuaFunction::createFromCFunction(L, stdFunctionCaller, { funcValue });
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

LuaFunction::LuaFunction() : LuaValue(), _errorFunction(nullptr) {
}

LuaFunction::LuaFunction(const LuaFunction&) = default;
LuaFunction& LuaFunction::operator=(const LuaFunction&) = default;

LuaFunction::LuaFunction(LuaFunction&&) noexcept = default;
LuaFunction& LuaFunction::operator=(LuaFunction&&) noexcept = default;

LuaFunction::~LuaFunction() = default;

bool LuaFunction::setEnvironment(const LuaTable& table) {
	if (!table.getReference()->isValid()) {
		throw LuaException("Table reference is not valid!");
	}

	this->pushValue(_luaState);
	table.pushValue(_luaState);

	bool ret = lua_setfenv(_luaState, -2) != 0;

	// Pop the function again
	lua_pop(_luaState, 1);

	return ret;
}

LuaValueList LuaFunction::operator()(lua_State* L, const LuaValueList& args) const {
	return this->call(L, args);
}

void LuaFunction::setReference(const LuaReference& ref) {
	if (ref == nullptr)
	{
		// If not a valid reference then let the base class handle everything
		LuaValue::setReference(ref);
		return;
	}

	lua_State* L = ref->getState();

	ref->pushValue(L);

	if (lua_type(L, -1) != LUA_TFUNCTION) {
		lua_pop(L, 1);
		throw LuaException("Reference does not refere to a function!");
	} else {
		lua_pop(L, 1);
		LuaValue::setReference(ref);
	}
}

LuaValueList LuaFunction::call(lua_State* L, const LuaValueList& args) const {
	int err_idx = 0;
	int stackTop;

	if (_errorFunction) {
		// push the error function
		_errorFunction->pushValue(L);
		err_idx = lua_gettop(L);
		stackTop = err_idx;
	} else {
		stackTop = lua_gettop(L);
	}

	if(!lua_checkstack(L, args.size() + 1))
		throw LuaException("Lua Stack Overflow!");

	// Push the function onto the stack
	this->pushValue(L, true);

	// Push the arguments onto the stack
	for (const auto& arg : args) {
		arg.pushValue(L, true);
	}

	// actually call the function now!
	int err = lua_pcall(L, (int) args.size(), LUA_MULTRET, err_idx);

	if (!err) {
		int numReturn = lua_gettop(L) - stackTop;
		LuaValueList values;
		values.reserve(numReturn);

		LuaValue val;
		for (int i = 0; i < numReturn; ++i) {
			if (convert::popValue(L, val)) {
				// Add values at the begin as the last return value is on top
				// of the stack.
				values.insert(values.begin(), val);
			}
		}

		if (err_idx != 0) {
			// Remove the error function
			lua_pop(L, 1);
		}

		return values;
	} else {
		// Make sure that there is exactly one parameter left on the stack
		// If the error function didn't return anything then this will push nil
		// If it pushed more than one value then this will discard all of them except the last one
		lua_settop(L, stackTop + 1);

		std::string err_msg;
		if (!lua_isstring(L, -1)) {
			err_msg = "Invalid lua value on stack!";
			lua_pop(L, 1); // Remove the value on the stack
		} else {
			if (!convert::popValue(L, err_msg)) {
				err_msg = "Failed to get error message from Lua stack!";
			}
		}

		if (err_idx != 0) {
			// Pop the error function
			lua_pop(L, 1);
		}

		// Throw exception with generated message
		throw LuaException(err_msg);
	}
}

bool ::luacpp::convert::popValue(lua_State* luaState, LuaFunction& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isfunction(luaState, stackposition)) {
		return false;
	} else {
		target.setReference(UniqueLuaReference::create(luaState, stackposition));

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}

}

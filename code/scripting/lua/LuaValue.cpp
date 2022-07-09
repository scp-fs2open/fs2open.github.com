#include "LuaValue.h"
#include "LuaException.h"

namespace {
using namespace luacpp;

ValueType luaToEnumType(int luaType) {
	switch (luaType) {
		case LUA_TNONE:
			return ValueType::NONE;
		case LUA_TNIL:
			return ValueType::NIL;
		case LUA_TBOOLEAN:
			return ValueType::BOOLEAN;
		case LUA_TLIGHTUSERDATA:
			return ValueType::LIGHTUSERDATA;
		case LUA_TNUMBER:
			return ValueType::NUMBER;
		case LUA_TSTRING:
			return ValueType::STRING;
		case LUA_TTABLE:
			return ValueType::TABLE;
		case LUA_TFUNCTION:
			return ValueType::FUNCTION;
		case LUA_TUSERDATA:
			return ValueType::USERDATA;
		case LUA_TTHREAD:
			return ValueType::THREAD;

		default:
			return ValueType::NONE;
	}
}
}

namespace luacpp {
LuaValue LuaValue::createNil(lua_State* L)
{
	lua_pushnil(L);

	LuaValue val(L);

	val.setReference(UniqueLuaReference::create(L));

	return val;
}

LuaValue::LuaValue() = default;

LuaValue::LuaValue(lua_State* state) : _luaState(state)
{
	Assertion(state != nullptr, "Lua state pointer is not valid!");
}

LuaValue::LuaValue(const LuaValue&) = default;
LuaValue& LuaValue::operator=(const LuaValue&) = default;

LuaValue::LuaValue(LuaValue&&) noexcept = default;
LuaValue& LuaValue::operator=(LuaValue&&) noexcept = default;

LuaValue::~LuaValue() = default;

void LuaValue::setReference(const LuaReference& ref)
{
	this->_reference = ref;
	this->_luaState = ref->getState();

	if (isValid()) {
		this->_reference->pushValue(_luaState);

		this->_luaType = luaToEnumType(lua_type(_luaState, -1));

		lua_pop(_luaState, 1);
	} else {
		_luaType = ValueType::NONE;
	}
}

const LuaReference LuaValue::getReference() const {
	return _reference;
}

ValueType LuaValue::getValueType() const {
	return _luaType;
}

bool LuaValue::isValid() const {
	return _reference && _reference->isValid();
}

bool LuaValue::pushValue(lua_State* thread, bool guaranteeStack) const
{
	if (this->_reference->isValid()) {
		this->_reference->pushValue(thread, guaranteeStack);
		return true;
	} else {
		return false;
	}
}

lua_State* LuaValue::getLuaState() const { return _luaState; }

namespace convert {

void pushValue(lua_State* luaState, const LuaValue& value) { value.pushValue(luaState); }

bool popValue(lua_State* luaState, LuaValue& target, int stackposition, bool remove)
{
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	target.setReference(UniqueLuaReference::create(luaState, stackposition));

	if (remove) {
		lua_remove(luaState, stackposition);
	}

	return true;
}

}

}

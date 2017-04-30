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
LuaValue LuaValue::createNil(lua_State* L) {
	lua_pushnil(L);

	LuaValue val(L);

	val.setReference(UniqueLuaReference::create(L));

	return val;
}

LuaValue::LuaValue(lua_State* state) : _luaState(state), _luaType(ValueType::NONE) {
	if (state == nullptr) {
		throw LuaException("Lua state pointer is not valid!");
	}
}

LuaValue::LuaValue(const LuaValue& other) : _luaState(other._luaState) {
	// Just copy the reference object from the other
	this->setReference(other._reference);
}

LuaValue::~LuaValue() {
}

void LuaValue::setReference(LuaReference ref) {
	this->_reference = ref;

	if (isValid()) {
		this->_reference->pushValue();

		_luaState = ref->getState();
		this->_luaType = luaToEnumType(lua_type(_luaState, -1));

		lua_pop(_luaState, 1);
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

bool LuaValue::pushValue() const {
	if (this->_reference->isValid()) {
		this->_reference->pushValue();
		return true;
	} else {
		return false;
	}
}

lua_State* LuaValue::getLuaState() const {
	return _luaState;
}
}
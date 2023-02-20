
#include "LuaReference.h"

#include "globalincs/pstypes.h"

#include "LuaException.h"
#include "LuaHeaders.h"
#include "LuaUtil.h"

namespace luacpp {
LuaReference UniqueLuaReference::create(lua_State* state, int position)
{
	if (state == nullptr) {
		throw LuaException("Need a valid lua state!");
	}

	lua_pushvalue(state, position);

	// Always store the main thread here to ensure that we do not store a possible invalid thread reference
	LuaReference ref =
		std::make_shared<UniqueLuaReference>(util::getMainThread(state), luaL_ref(state, LUA_REGISTRYINDEX));

	return ref;
}

LuaReference UniqueLuaReference::copy(const LuaReference& other) {
	other->pushValue(other->_luaState);

	return create(other->_luaState);
}

UniqueLuaReference::UniqueLuaReference(lua_State* state, int reference) : _luaState(state), _reference(reference)
{
	if (state == nullptr) {
		throw LuaException("Need a valid lua state!");
	}

	if (reference < 0 && reference != LUA_REFNIL) {
		throw LuaException("Reference must be greater than or equal to zero!");
	}
}
UniqueLuaReference::UniqueLuaReference() : _luaState(nullptr), _reference(LUA_NOREF) {}

UniqueLuaReference::UniqueLuaReference(UniqueLuaReference&& other) noexcept : _luaState(nullptr), _reference(LUA_NOREF) {
	*this = std::move(other);
}
UniqueLuaReference& UniqueLuaReference::operator=(UniqueLuaReference&& other) noexcept {
	std::swap(_luaState, other._luaState);
	std::swap(_reference, other._reference);
	return *this;
}

UniqueLuaReference::~UniqueLuaReference() {
	this->removeReference();
}

bool UniqueLuaReference::removeReference() {
	if (this->isValid()) {
		luaL_unref(_luaState, LUA_REGISTRYINDEX, _reference);
		_reference = LUA_NOREF;
		return true;
	} else {
		return false;
	}
}

int UniqueLuaReference::getReference() const {
	return _reference;
}

bool UniqueLuaReference::isValid() const {
	if (_luaState == nullptr) {
		return false;
	}

	if (_reference == LUA_NOREF) {
		return false;
	}

	return true;
}

void UniqueLuaReference::pushValue(lua_State* thread, bool manualStackAllocation) const
{
	Assertion(thread != nullptr, "Valid thread state must be specified!");

	if (!manualStackAllocation) {
		lua_checkstack(thread, 1);
	}

	if (this->isValid()) {
		lua_rawgeti(thread, LUA_REGISTRYINDEX, this->getReference());
	}
}
}

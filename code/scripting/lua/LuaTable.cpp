#include "LuaException.h"
#include "LuaTable.h"

namespace luacpp {
LuaTable LuaTable::create(lua_State* state) {
	LuaTable table;

	lua_newtable(state);

	table.setReference(UniqueLuaReference::create(state));

	lua_pop(state, 1);

	return table;
}

LuaTable::LuaTable() : LuaValue() {
}

LuaTable::LuaTable(const LuaTable& other) : LuaValue(other) {
}

LuaTable::~LuaTable() {
}

bool LuaTable::setMetatable(const LuaTable& table) {
	if (!table.getReference()->isValid()) {
		throw LuaException("Meta table reference is not valid!");
	}

	this->pushValue();
	table.pushValue();

	lua_setmetatable(_luaState, -2);

	lua_pop(_luaState, 1);

	return true;
}

void LuaTable::setReference(luacpp::LuaReference ref) {
	ref->pushValue();

	lua_State* L = ref->getState();

	if (lua_type(L, -1) != LUA_TTABLE) {
		lua_pop(L, 1);
		throw LuaException("Reference does not refere to a table!");
	} else {
		lua_pop(L, 1);
		LuaValue::setReference(ref);
	}
}

size_t LuaTable::getLength() {
	this->pushValue();

	size_t length = lua_objlen(_luaState, -1);

	lua_pop(_luaState, 1);

	return length;
}

LuaTable::iterator::iterator(const LuaTable& parent) {
	_iter.reset(new LuaTableIterator(parent));
}
LuaTable::iterator::iterator() : _iter(nullptr), _atEnd(true) {
}
bool LuaTable::iterator::operator==(const iterator& other) {
	return _atEnd == other._atEnd;
}
bool LuaTable::iterator::operator!=(const LuaTable::iterator& other) {
	return !(*this == other);
}
LuaTable::iterator& LuaTable::iterator::operator++() {
	_iter->toNextElement();

	_atEnd = !_iter->hasElement();

	return *this;
}
LuaTable::iterator& LuaTable::iterator::operator++(int) {
	return ++(*this); // Post-fix operator doesn't make sense for us.
}
std::pair<LuaValue, LuaValue> LuaTable::iterator::operator*() {
	return _iter->getElement();
}

LuaTable::iterator LuaTable::begin() {
	iterator iter(*this);

	// This will call lua_next and automatically handle the end of the table
	return iter;
}
LuaTable::iterator LuaTable::end() {
	return iterator(); // Empty iterator
}

LuaTableIterator::LuaTableIterator(const LuaTable& t) : _luaState(t.getLuaState()) {
	_stackTop = lua_gettop(_luaState);

	t.pushValue();
	lua_pushnil(_luaState);

	toNextElement();
}
LuaTableIterator::~LuaTableIterator() {
	lua_settop(_luaState, _stackTop);
}
bool LuaTableIterator::hasElement() {
	return _hasElement;
}
void LuaTableIterator::toNextElement() {
	auto ret = lua_next(_luaState, -2);

	_hasElement = ret != 0;

	if (_hasElement) {
		LuaValue key;
		key.setReference(UniqueLuaReference::create(_luaState, -2));

		LuaValue value;
		value.setReference(UniqueLuaReference::create(_luaState, -1));

		_currentVal = std::make_pair(key, value);

		// Remove value from stack
		lua_pop(_luaState, 1);
	}
}
std::pair<LuaValue, LuaValue> LuaTableIterator::getElement() {
	return _currentVal;
}

bool convert::popValue(lua_State* luaState, LuaTable& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_istable(luaState, stackposition)) {
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

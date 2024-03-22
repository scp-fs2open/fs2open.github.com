
#include "LuaUtil.h"

namespace {
const char* mainStateRefName = "_lua_mainthread";
}

namespace luacpp {
namespace util {
const char* getValueName(ValueType type)
{
	switch (type) {
	case ValueType::NONE:
		return "none";
	case ValueType::NIL:
		return "nil";
	case ValueType::BOOLEAN:
		return "boolean";
		case ValueType::LIGHTUSERDATA:
			return "light userdata";
		case ValueType::STRING:
			return "string";
		case ValueType::NUMBER:
			return "number";
		case ValueType::TABLE:
			return "table";
		case ValueType::FUNCTION:
			return "function";
		case ValueType::USERDATA:
			return "userdata";
		case ValueType::THREAD:
			return "thread";
		default:
			return "unknown";
		}
}

void initializeLuaSupportLib(lua_State* L)
{
	auto mainThread = lua_pushthread(L);
	Assertion(mainThread, "Must be called with the main thread as the parameter!");

	lua_setfield(L, LUA_REGISTRYINDEX, mainStateRefName);
}

lua_State* getMainThread(lua_State* L)
{
	if (lua_pushthread(L)) {
		// State is the main thread, just return that
		lua_pop(L, 1);
		return L;
	}

	lua_pushstring(L, mainStateRefName);
	lua_rawget(L, LUA_REGISTRYINDEX);

	auto state = lua_tothread(L, -1);

	// Need to pop both the thread from L and the main thread to return the stack back to neutral
	lua_pop(L, 2);

	return state;
}

} // namespace util
} // namespace luacpp

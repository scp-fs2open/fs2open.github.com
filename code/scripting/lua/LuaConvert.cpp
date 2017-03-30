
#include "LuaConvert.h"

namespace
{
using namespace scripting;

ade_table_entry& getTableEntry(size_t idx) {
	return ade_manager::getInstance()->getEntry(idx);
}
}

namespace luacpp {
namespace convert {
namespace internal {

bool isValidIndex(lua_State* state, int index) {
	if (1 <= std::abs(index) && std::abs(index) <= lua_gettop(state)) {
		return true;
	} else {
		return false;
	}
}

}

void pushValue(lua_State* luaState, const double& value) {
	lua_pushnumber(luaState, value);
}
void pushValue(lua_State* luaState, const float& value) {
	lua_pushnumber(luaState, value);
}
void pushValue(lua_State* luaState, const int& value) {
	lua_pushnumber(luaState, value);
}
void pushValue(lua_State* luaState, const size_t& value) {
	lua_pushnumber(luaState, (lua_Number)value);
}
void pushValue(lua_State* luaState, const std::string& value) {
	lua_pushlstring(luaState, value.c_str(), value.size());
}
void pushValue(lua_State* luaState, const char* value) {
	lua_pushstring(luaState, value);
}
void pushValue(lua_State* luaState, const bool& value) {
	lua_pushboolean(luaState, value);
}
void pushValue(lua_State* luaState, const lua_CFunction& value) {
	lua_pushcfunction(luaState, value);
}

void pushValue(lua_State* L, const ade_odata& od) {
	using namespace scripting;

	//WMC - char must be 1 byte, foo.
	static_assert(sizeof(char) == 1, "char must be 1 byte!");
	//WMC - step by step

	//Create new LUA object and get handle
	char *newod = (char*)lua_newuserdata(L, od.size + sizeof(ODATA_SIG_TYPE));
	//Create or get object metatable
	luaL_getmetatable(L, getTableEntry(od.idx).Name);
	//Set the metatable for the object
	lua_setmetatable(L, -2);

	//Copy the actual object data to the Lua object
	memcpy(newod, od.buf, od.size);

	//Also copy in the unique sig
	if (od.sig != NULL)
		memcpy(newod + od.size, od.sig, sizeof(ODATA_SIG_TYPE));
	else
	{
		ODATA_SIG_TYPE tempsig = ODATA_SIG_DEFAULT;
		memcpy(newod + od.size, &tempsig, sizeof(ODATA_SIG_TYPE));
	}
}

bool popValue(lua_State* luaState, int& target, int stackposition, bool remove) {
	double temp;

	auto ret = popValue(luaState, temp, stackposition, remove);
	if (ret) {
		target = (int)temp;
	}
	return ret;
}
bool popValue(lua_State* luaState, size_t& target, int stackposition, bool remove) {
	double temp;

	auto ret = popValue(luaState, temp, stackposition, remove);
	if (ret) {
		target = (size_t)temp;
	}
	return ret;
}
bool popValue(lua_State* luaState, float& target, int stackposition, bool remove) {
	double temp;

	auto ret = popValue(luaState, temp, stackposition, remove);
	if (ret) {
		target = (float)temp;
	}
	return ret;
}
bool popValue(lua_State* luaState, double& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isnumber(luaState, stackposition)) {
		return false;
	} else {
		target = lua_tonumber(luaState, stackposition);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}
bool popValue(lua_State* luaState, std::string& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isstring(luaState, stackposition)) {
		return false;
	} else {
		size_t size;
		const char* string = lua_tolstring(luaState, stackposition, &size);
		target.assign(string, size);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}

bool popValue(lua_State* luaState, bool& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isboolean(luaState, stackposition)) {
		return false;
	} else {
		target = lua_toboolean(luaState, stackposition) != 0;

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return target;
	}
}

bool popValue(lua_State* luaState, lua_CFunction& target, int stackposition, bool remove) {
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_iscfunction(luaState, stackposition)) {
		return false;
	} else {
		target = lua_tocfunction(luaState, stackposition);

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}

bool popValue(lua_State* L, scripting::ade_odata& od, int stackposition, bool remove) {
	//WMC - Get metatable
	lua_getmetatable(L, stackposition);
	int mtb_ldx = lua_gettop(L);
	Assert(!lua_isnil(L, -1));

	//Get ID
	lua_pushstring(L, "__adeid");
	lua_rawget(L, mtb_ldx);

	if (lua_tonumber(L, -1) != od.idx)
	{
		lua_pushstring(L, "__adederivid");
		lua_rawget(L, mtb_ldx);
		if ((uint)lua_tonumber(L, -1) != od.idx)
		{
			// Issue the LuaError here since this is the only place where we have all relevant information
			LuaError(L, "Argument %d is the wrong type of userdata; '%s' given, but '%s' expected", stackposition, getTableEntry((uint)lua_tonumber(L, -2)).Name, getTableEntry(od.idx).GetName());
			return false;
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
	if (od.size != ODATA_PTR_SIZE)
	{
		memcpy(od.buf, lua_touserdata(L, stackposition), od.size);
		if (od.sig != NULL) {
			//WMC - char must be 1
			Assert(sizeof(char) == 1);
			//WMC - Yuck. Copy sig data.
			//Maybe in the future I'll do a packet userdata thing.
			(*od.sig) = *(ODATA_SIG_TYPE*)(*(char **)od.buf + od.size);
		}
	}
	else {
		(*(void**)od.buf) = lua_touserdata(L, stackposition);
	}

	if (remove) {
		lua_remove(L, stackposition);
	}

	return true;
}

}
}

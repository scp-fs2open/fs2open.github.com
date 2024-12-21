
#include "LuaUtil.h"

#include "scripting/api/objs/vecmath.h"

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

vec3d valueToVec3d(const LuaValue &luaValue)
{
	if (luaValue.is(luacpp::ValueType::TABLE))
	{
		vec3d tablePoint = vmd_zero_vector;
		int index = 0;
		bool is_array = false;
		bool is_table = false;

		// we have to go deeper
		luacpp::LuaTable childTable;
		childTable.setReference(luaValue.getReference());
		for (const auto &childEntry : childTable)
		{
			// note: this is pre-increment
			if (index >= 3)
				throw LuaException("Vec3d table has more than three entries!");

			if (!childEntry.second.is(luacpp::ValueType::NUMBER))
				throw LuaException("Value in vec3d table is not a number!");

			auto number = static_cast<float>(childEntry.second.getValue<double>());

			if (childEntry.first.is(luacpp::ValueType::STRING))
			{
				is_table = true;
				if (is_array)
					throw LuaException("Vec3d table has an unexpected format!");

				const auto &str = childEntry.first.getValue<SCP_string>();
				if (lcase_equal(str, "x"))
					tablePoint.xyz.x = number;
				else if (lcase_equal(str, "y"))
					tablePoint.xyz.y = number;
				else if (lcase_equal(str, "z"))
					tablePoint.xyz.z = number;
				else
					throw LuaException("Vec3d table has an entry other than x/X, y/Y, or z/Z!");
			}
			else
			{
				is_array = true;
				if (is_table)
					throw LuaException("Vec3d table has an unexpected format!");

				tablePoint.a1d[index] = number;
			}

			index++;
		}

		// note: this is post-increment
		if (index < 3)
			throw LuaException("Vec3d table has fewer than three entries!");

		return tablePoint;
	}
	else if (luaValue.is(luacpp::ValueType::USERDATA))
	{
		try
		{
			vec3d v;
			luaValue.getValue(scripting::api::l_Vector.Get(&v));
			return v;
		}
		catch (const luacpp::LuaException& /*e*/)
		{
			throw LuaException("Userdata in vec3d table is not a vector!");
		}
	}
	else
	{
		throw LuaException("Vec3d value is of an unhandled type!");
	}
}

} // namespace util
} // namespace luacpp

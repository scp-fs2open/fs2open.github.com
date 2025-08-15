#pragma once

#include <type_traits>
#include "globalincs/pstypes.h"

struct object_ship_wing_point_team;
struct object_h;

namespace scripting {
	namespace internal {
		template <typename T>
		struct ade_serializable_external : std::false_type { };

		/*
		 * If you need a multi serializer that shouldn't be part of the struct (i.e. because the struct isn't just a lua handler, but also used elsewhere),
		 * then you can declare and external serializer here and implement it in the cpp of the respective scripting object
		 * */

		template<>
		struct ade_serializable_external<vec3d> : std::true_type {
			static void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& luaValue, ubyte* data, int& packet_size);
			static void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
		};

		template<>
		struct ade_serializable_external<object_ship_wing_point_team> : std::true_type {
			static void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& luaValue, ubyte* data, int& packet_size);
			static void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
		};

		template<>
		struct ade_serializable_external<object_h> : std::true_type {
			static void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& luaValue, ubyte* data, int& packet_size);
			static void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
		};
	}
}
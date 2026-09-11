#pragma once

#include "nebula/volumetrics.h"
#include "scripting/ade_api.h"

namespace scripting::api {

struct volumetric_h {
	volumetric_h() = default;
	explicit volumetric_h(int idx);

	int index = -1;

	bool isValid() const;
	volumetric_nebula* get() const;

	static void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size);
	static void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset); // NOLINT(readability-non-const-parameter)
};

DECLARE_ADE_OBJ(l_Volumetric, volumetric_h);

} // namespace scripting::api
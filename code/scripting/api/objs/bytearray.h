#pragma once

#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct bytearray_h {
	SCP_vector<uint8_t> m_data;

  public:
	bytearray_h();
	explicit bytearray_h(SCP_vector<uint8_t> data);

	const SCP_vector<uint8_t>& data() const;

	void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size);
	void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
};

DECLARE_ADE_OBJ(l_Bytearray, bytearray_h);

} // namespace api
} // namespace scripting

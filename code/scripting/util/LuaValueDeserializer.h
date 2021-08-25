#pragma once

#include "libs/jansson.h"
#include "scripting/lua/LuaTable.h"
#include "scripting/lua/LuaValue.h"

namespace scripting {
namespace util {

class LuaValueDeserializer {
  public:
	LuaValueDeserializer(lua_State* L);

	luacpp::LuaValue deserialize(const SCP_vector<uint8_t>& serialized) const;
  private:
	luacpp::LuaValue deserializePlainJson(const uint8_t* data, size_t size) const;
	luacpp::LuaValue jsonToValue(json_t* json) const;

	lua_State* m_L = nullptr;
};

} // namespace util
} // namespace scripting

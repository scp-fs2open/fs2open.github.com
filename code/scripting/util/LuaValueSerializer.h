#pragma once

#include "scripting/lua/LuaValue.h"
#include "scripting/lua/LuaTable.h"

#include "libs/jansson.h"

namespace scripting {
namespace util {

enum class SerializationType : uint8_t {
	PlainJson = 0,
};

class LuaValueSerializer {
  public:
	LuaValueSerializer(luacpp::LuaValue value);

	SCP_vector<uint8_t> serialize() const;
  private:
	json_t* toJson() const;

	json_t* tableToJson() const;
	static json_t* tableToJsonArray(const luacpp::LuaTable& table);
	static json_t* tableToJsonObject(const luacpp::LuaTable& table);

	luacpp::LuaValue m_value;
};

} // namespace util
} // namespace scripting

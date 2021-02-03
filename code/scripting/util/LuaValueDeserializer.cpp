
#include "LuaValueDeserializer.h"

#include "LuaValueSerializer.h"

namespace scripting {
namespace util {

LuaValueDeserializer::LuaValueDeserializer(lua_State* L) : m_L(L) {}

luacpp::LuaValue LuaValueDeserializer::deserialize(const SCP_vector<uint8_t>& serialized) const
{
	if (serialized.empty()) {
		throw std::runtime_error("Not enough data");
	}

	const auto type = static_cast<SerializationType>(serialized.front());

	switch (type) {
	case SerializationType::PlainJson:
		return deserializePlainJson(serialized.data() + 1, serialized.size() - 1);
	default:
		throw std::runtime_error("Unknown serialization type.");
	}
}
luacpp::LuaValue LuaValueDeserializer::deserializePlainJson(const uint8_t* data, size_t size) const
{
	json_error_t error;
	std::unique_ptr<json_t> json(json_loadb(reinterpret_cast<const char*>(data),
		size,
		JSON_DECODE_ANY | JSON_DECODE_INT_AS_REAL | JSON_ALLOW_NUL,
		&error));

	if (!json) {
		throw json_exception(error);
	}

	return jsonToValue(json.get());
}
luacpp::LuaValue LuaValueDeserializer::jsonToValue(json_t* json) const
{
	switch (json_typeof(json)) {
	case JSON_OBJECT: {
		auto objectTbl = luacpp::LuaTable::create(m_L);
		for(const auto& pair : json::object_range(json))
		{
			objectTbl.addValue(std::get<0>(pair), jsonToValue(std::get<1>(pair)));
		}
		return std::move(objectTbl);
	}
	case JSON_ARRAY: {
		auto arrayTbl = luacpp::LuaTable::create(m_L);
		size_t i = 1;
		for (const auto& value : json::array_range(json))
		{
			arrayTbl.addValue(i, jsonToValue(value));
			++i;
		}
		return std::move(arrayTbl);
	}
	case JSON_STRING: {
		const auto val = json_string_value(json);
		const auto len = json_string_length(json);
		return luacpp::LuaValue::createValue(m_L, SCP_string(val, val + len));
	}
	case JSON_REAL:
		return luacpp::LuaValue::createValue(m_L, json_real_value(json));
	case JSON_TRUE:
		return luacpp::LuaValue::createValue(m_L, true);
	case JSON_FALSE:
		return luacpp::LuaValue::createValue(m_L, false);
	case JSON_NULL:
		return luacpp::LuaValue::createNil(m_L);
	default:
		return luacpp::LuaValue();
	}
}

} // namespace util
} // namespace scripting

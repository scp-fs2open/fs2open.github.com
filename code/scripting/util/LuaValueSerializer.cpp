
#include "LuaValueSerializer.h"

#include "scripting/lua/LuaTable.h"

namespace scripting {
namespace util {

LuaValueSerializer::LuaValueSerializer(luacpp::LuaValue value) : m_value(std::move(value)) {}

SCP_vector<uint8_t> LuaValueSerializer::serialize() const
{
	std::unique_ptr<json_t> serializedJson(toJson());

	const auto json = json_dump_string(serializedJson.get(), JSON_ENCODE_ANY | JSON_COMPACT);

	SCP_vector<uint8_t> data;
	data.reserve(json.size() + 1);

	// Add a tiny header describing the following data for future proofing.
	data.push_back(static_cast<uint8_t>(SerializationType::PlainJson));

	std::transform(json.cbegin(), json.cend(), std::back_inserter(data), [](char c) {
		return static_cast<uint8_t>(c);
	});

	return data;
}

json_t* LuaValueSerializer::toJson() const
{
	switch (m_value.getValueType()) {
	case luacpp::ValueType::NIL:
		return json_null();
	case luacpp::ValueType::BOOLEAN:
		return json_boolean(m_value.getValue<bool>());
	case luacpp::ValueType::STRING: {
		const auto string = m_value.getValue<SCP_string>();
		return json_stringn(string.c_str(), string.size());
	}
	case luacpp::ValueType::NUMBER:
		return json_real(m_value.getValue<double>());
	case luacpp::ValueType::TABLE:
		return tableToJson();
	case luacpp::ValueType::NONE:
	case luacpp::ValueType::USERDATA:
	case luacpp::ValueType::FUNCTION:
	case luacpp::ValueType::LIGHTUSERDATA:
	case luacpp::ValueType::THREAD:
	default:
		throw std::runtime_error("Unsupported value type!");
	}
}
json_t* LuaValueSerializer::tableToJson() const
{
	luacpp::LuaTable tableVal;
	tableVal.setReference(m_value.getReference());

	auto testVal = tableVal.getValue<luacpp::LuaValue>(1);
	if (testVal.getValueType() == luacpp::ValueType::NIL) {
		// If there is no first array element then we assume it is an object
		return tableToJsonObject(tableVal);
	} else {
		// Assume this is an object
		return tableToJsonArray(tableVal);
	}
}
json_t* LuaValueSerializer::tableToJsonArray(const luacpp::LuaTable& table)
{
	auto size = table.getLength();
	auto jsonArray = json_array();
	for (size_t i = 1; i <= size; ++i) {
		luacpp::LuaValue arrayVal;
		if (!table.getValue(i, arrayVal)) {
			json_array_append_new(jsonArray, json_null());
		} else {
			LuaValueSerializer valueSerializer(arrayVal);
			json_array_append_new(jsonArray, valueSerializer.toJson());
		}
	}
	return jsonArray;
}
json_t* LuaValueSerializer::tableToJsonObject(const luacpp::LuaTable& table)
{
	auto jsonObj = json_object();
	for (const auto& entry : table) {
		if (entry.first.getValueType() != luacpp::ValueType::STRING) {
			// Ignore non-string indices
			continue;
		}

		const auto index = entry.first.getValue<SCP_string>();

		LuaValueSerializer valueSerializer(entry.second);
		json_object_set_new(jsonObj, index.c_str(), valueSerializer.toJson());
	}
	return jsonObj;
}

} // namespace util
} // namespace scripting

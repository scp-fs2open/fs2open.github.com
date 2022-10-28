#include "multi_lua.h"

#include "multimsgs.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"

//This will send a byte more per userdata, but catch more faults due to players having different APIs
//Also, it makes the remaining faults safer, since this way we'll only get invalid data, not accessing potentially invalid memory
#define SAFE_MULTI_LUA_USERDATA_SIZES true


enum class lua_net_data_type : uint8_t { NIL, BOOL, NUMBER, STRING8, STRING16, USERDATA, TABLE };

union lua_packet_header {
	struct {
		unsigned short target : 15;
		unsigned short isOrdered : 1;
	} data;
	unsigned short packed;
};
static_assert(sizeof(lua_packet_header) == 2, "Lua Packet Header is not properly packed!");

static luacpp::LuaValue process_lua_userdata(ubyte* data, int& offset, lua_State* L) {
	luacpp::LuaValue retVal(L);
	uint16_t adeIdx;
	GET_USHORT(adeIdx);

#if SAFE_MULTI_LUA_USERDATA_SIZES
	uint8_t size;
	GET_DATA(size);
#endif

	const auto& objType = scripting::ade_manager::getInstance()->getEntry(adeIdx);
	if (objType.Type != 'o' || objType.Instanced || objType.Deserializer == nullptr
#if SAFE_MULTI_LUA_USERDATA_SIZES
		|| size != objType.Size
#endif
		) {
		//There is a case to be made for this to be an assert.
		//This happens when the scripting API changes but no multi bump occurs.
		LuaError(L, "Lua Network packet with Userdata has bad adeidx! Make sure every placer is using the same game version as the host.");
		return luacpp::LuaValue::createNil(L); //TODO replace with throw. Due to (potentially) bad size, we're now unable to decode the rest of the packet as well
	}
	
	//Create new LUA object and get handle
	auto newod = (char*)lua_newuserdata(L, objType.Size);
	//Create or get object metatable
	luaL_getmetatable(L, objType.Name);
	//Set the metatable for the object
	lua_setmetatable(L, -2);

	//Deserialize and fill newod space
	objType.Deserializer(L, objType, newod, data, offset);

	retVal.setReference(luacpp::UniqueLuaReference::create(L));

	// Remove the value again
	lua_pop(L, 1);

	return retVal;
}

static luacpp::LuaValue process_lua_data(ubyte* data, int& offset, lua_State* L) {
	uint8_t dataType = static_cast<uint8_t>(lua_net_data_type::NIL);
	GET_DATA(dataType);

	switch (static_cast<lua_net_data_type>(dataType)) {
	case lua_net_data_type::NIL:
		return luacpp::LuaValue::createNil(L);
	case lua_net_data_type::BOOL: {
		uint8_t value;
		GET_DATA(value);
		return luacpp::LuaValue::createValue(L, value != 0);
	}
	case lua_net_data_type::NUMBER: {
		float value;
		GET_FLOAT(value);
		return luacpp::LuaValue::createValue(L, value);
	}
	case lua_net_data_type::STRING8: {
		char text[0xff];
		GET_STRING(text);
		return luacpp::LuaValue::createValue(L, text);
	}
	case lua_net_data_type::STRING16: {
		char text[MAX_PACKET_SIZE - sizeof(lua_packet_header)];
		GET_STRING_16(text);
		return luacpp::LuaValue::createValue(L, text);
	}
	case lua_net_data_type::USERDATA:
		return process_lua_userdata(data, offset, L);
	case lua_net_data_type::TABLE: {
		luacpp::LuaTable table = luacpp::LuaTable::create(L);
		uint8_t entries = 0;
		GET_DATA(entries);
		for(uint8_t i = 0; i < entries; i++)
			table.addValue(process_lua_data(data, offset, L), process_lua_data(data, offset, L));
		return table;
	}
	default:
		UNREACHABLE("Got invalid lua multi packet data type %d!", dataType);
		return luacpp::LuaValue::createNil(L);
	}
}

static void send_lua_userdata(ubyte* data, int& packet_size, const luacpp::LuaValue& value) {
	lua_State* L = value.getLuaState();

	value.pushValue(L);
	lua_getmetatable(L, -1);

	int mtb_ldx = lua_gettop(L);
	lua_pushstring(L, "__adeid");
	lua_rawget(L, mtb_ldx);
	ushort adeIdx = static_cast<ushort>(lua_tonumber(L, -1));
	const auto& objType = scripting::ade_manager::getInstance()->getEntry(adeIdx);

	lua_pop(L, 3);

	ADD_USHORT(adeIdx);

#if SAFE_MULTI_LUA_USERDATA_SIZES
	ADD_DATA(objType.Size);
#endif

	//Serialize
	objType.Serializer(L, objType, value, data, packet_size);
}

#define SEND_LUA_DATA_CHECK_SPACE(requiredSpace) \
if(MAX_PACKET_SIZE - packet_size < (requiredSpace)) { \
	LuaError(value.getLuaState(), "Tried to add too much data to a lua packet. Please reduce the amount of data to send. Maximum %d bytes supported!", MAX_PACKET_SIZE); \
	/*TODO throw here*/ \
}
#define USERDATA_REQUIRED_ESTIMATE 17 /*I suspect all userdata objects will fit into 17 bytes. The largest is likely an orientation matrix if not sent as angles. */

static void send_lua_data(ubyte* data, int& packet_size, const luacpp::LuaValue& value) {
	switch (value.getValueType()) {
	case luacpp::ValueType::NIL:
	case luacpp::ValueType::NONE: {
		SEND_LUA_DATA_CHECK_SPACE(1);
		uint8_t type = static_cast<uint8_t>(lua_net_data_type::NIL);
		ADD_DATA(type);
		break;
	}
	case luacpp::ValueType::BOOLEAN: {
		SEND_LUA_DATA_CHECK_SPACE(2);
		uint8_t valuedata = value.getValue<bool>() ? 1 : 0;
		uint8_t type = static_cast<uint8_t>(lua_net_data_type::BOOL);
		ADD_DATA(type);
		ADD_DATA(valuedata);
		break;
	}
	case luacpp::ValueType::NUMBER: {
		SEND_LUA_DATA_CHECK_SPACE(1 + static_cast<int>(sizeof(float)));
		float valuedata = value.getValue<float>();
		uint8_t type = static_cast<uint8_t>(lua_net_data_type::NUMBER);
		ADD_DATA(type);
		ADD_FLOAT(valuedata);
		break;
	}
	case luacpp::ValueType::STRING: {
		SCP_string valuedata = value.getValue<SCP_string>();
		SEND_LUA_DATA_CHECK_SPACE(1 + static_cast<int>(valuedata.size()) + 2);
		bool isLongString = valuedata.size() > 0xff;
		uint8_t type = static_cast<uint8_t>(isLongString ? lua_net_data_type::STRING16 : lua_net_data_type::STRING8);
		ADD_DATA(type);
		if (isLongString)
			ADD_STRING_16(valuedata.c_str());
		else
			ADD_STRING(valuedata.c_str());
		break;
	}
	case luacpp::ValueType::USERDATA: {
		SEND_LUA_DATA_CHECK_SPACE(1 + USERDATA_REQUIRED_ESTIMATE);
		uint8_t type = static_cast<uint8_t>(lua_net_data_type::USERDATA);
		ADD_DATA(type);
		send_lua_userdata(data, packet_size, value);
		break;
	}
	case luacpp::ValueType::TABLE: {
		SEND_LUA_DATA_CHECK_SPACE(2);
		uint8_t type = static_cast<uint8_t>(lua_net_data_type::TABLE);
		luacpp::LuaTable table;
		table.setReference(value.getReference());
		uint8_t size = 0;
		SCP_vector<std::pair<luacpp::LuaValue, luacpp::LuaValue>> dataPairs;
		for (const auto& value_pair : table) {
			//Since we can't rely on getLength / # to get a non-numeric-key length of a table, we need to count what we can actually emplace
			if (++size == 0xff) {
				LuaError(value.getLuaState(), "Tried to send a table with too many keys over the network. Maximum %d supported!", 0xff);
				//TODO throw here
			}
			SCP_UNUSED(value_pair);
		}
		ADD_DATA(type);
		ADD_DATA(size);
		for (const auto& value_pair : table) {
			send_lua_data(data, packet_size, value_pair.first);
			send_lua_data(data, packet_size, value_pair.second);
		}
		break;
	}
	default:
		LuaError(value.getLuaState(), "Tried to send an invalid type of lua data over the network. Support are only nil, boolean, number, string, tables and certain FSO userdata!");
		//TODO throw here
		break;
	}
}

#undef SEND_LUA_DATA_CHECK_SPACE
#undef USERDATA_REQUIRED_ESTIMATE

void process_lua_packet(ubyte* data, header* hinfo) {
	int offset; 
	lua_State* L = Script_system.GetLuaSession();

	offset = HEADER_LENGTH;

	lua_packet_header header;
	
	//The idea is, that we reject packets which are in the past, unless the last packet we actually got is so far in the past, we're not sure if it might have overflowed
	//Basically, assume that if packetTime - lastPacketTime < 1000, then it's either delayed over 60 seconds, or not a past packet. If it's larger than that, compare it to
	//the difference of local timestamps. If the local timestamp is over 2^16, it's also safe, otherwise, allow a 10% delay margin.
	ushort packetTime = 0U;
	UI_TIMESTAMP packetLocalTime = ui_timestamp();

	GET_USHORT(header.packed);
	if (header.data.isOrdered != 0) {
		GET_USHORT(packetTime);
	}
	
	luacpp::LuaValue value = process_lua_data(data, offset, L);

	PACKET_SET_SIZE();
	
}

void send_lua_packet(const luacpp::LuaValue& value, ushort target, lua_net_mode mode, net_player* reciever) {
	int packet_size;
	ubyte data[MAX_PACKET_SIZE];

	BUILD_HEADER(LUA_DATA_PACKET);

	bool isOrdered = mode == lua_net_mode::ORDERED;

	lua_packet_header header;
	header.data.target = target & 0b0111111111111111U;
	header.data.isOrdered = isOrdered ? 1 : 0;
	ADD_USHORT(header.packed);

	if (isOrdered) {
		ushort time = static_cast<ushort>(ui_timestamp().value());
		ADD_USHORT(time);
	}

	send_lua_data(data, packet_size, value);

	if (mode == lua_net_mode::RELIABLE) {
		if (reciever == nullptr)
			multi_io_send_to_all_reliable(data, packet_size);
		else
			multi_io_send_reliable(reciever, data, packet_size);
	}
	else {
		if (reciever == nullptr)
			multi_io_send_to_all(data, packet_size);
		else
			multi_io_send(reciever, data, packet_size);
	}
}
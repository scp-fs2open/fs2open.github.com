#include "multi_lua.h"

#include "multimsgs.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"

enum class lua_net_data_type : uint8_t { NIL, BOOL, NUMBER, STRING8, STRING16, USERDATA, TABLE };

union lua_packet_header {
	struct {
		unsigned short target : 10;
		unsigned short order : 6;
	} data;
	unsigned short packed;
};
static_assert(sizeof(lua_packet_header) == 2, "Lua Packet Header is not properly packed!");

static luacpp::LuaValue process_lua_userdata(ubyte* data, header* hinfo, int& offset, lua_State* L) {
	luacpp::LuaValue retVal(L);
	uint16_t adeIdx;
	GET_USHORT(adeIdx);

	const auto& objType = scripting::ade_manager::getInstance()->getEntry(adeIdx);
	if (objType.Type != 'o' || objType.Instanced) {
		//There is a case to be made for this to be an assert.
		//This happens when the scripting API changes but no multi bump occurs.
		Error(LOCATION, "Lua Network packet with Userdata has bad adeidx! Make sure every placer is using the same game version as the host.");
		return luacpp::LuaValue::createNil(L);
	}
	
	//Create new LUA object and get handle
	auto newod = (char*)lua_newuserdata(L, objType.Size);
	//Create or get object metatable
	luaL_getmetatable(L, objType.Name);
	//Set the metatable for the object
	lua_setmetatable(L, -2);

	//Copy the actual object data to the Lua object
	//new(newod) T(std::move(value.value));

	retVal.setReference(luacpp::UniqueLuaReference::create(L));

	// Remove the value again
	lua_pop(L, 1);

	return retVal;
}

static luacpp::LuaValue process_lua_data(ubyte* data, header* hinfo, int& offset, lua_State* L) {
	uint8_t dataType = static_cast<uint8_t>(lua_net_data_type::NIL);
	GET_DATA(dataType);

	switch (static_cast<lua_net_data_type>(dataType)) {
	case lua_net_data_type::NIL:
		return luacpp::LuaValue::createNil(L);
	case lua_net_data_type::BOOL: {
		bool value;
		GET_DATA(value);
		return luacpp::LuaValue::createValue(L, value);
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
		return process_lua_userdata(data, hinfo, offset, L);
	case lua_net_data_type::TABLE: {
		luacpp::LuaTable table = luacpp::LuaTable::create(L);
		uint8_t entries = 0;
		GET_DATA(entries);
		for(uint8_t i = 0; i < entries; i++)
			table.addValue(process_lua_data(data, hinfo, offset, L), process_lua_data(data, hinfo, offset, L));
		return table;
	}
	}
}

void process_lua_packet(ubyte* data, header* hinfo) {
	int offset; 
	lua_State* L = Script_system.GetLuaSession();

	offset = HEADER_LENGTH;

	lua_packet_header header;
	GET_USHORT(header.packed);
	


	PACKET_SET_SIZE();

}
#include "bytearray.h"

#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"

namespace scripting {
namespace api {

bytearray_h::bytearray_h() = default;
bytearray_h::bytearray_h(SCP_vector<uint8_t> data) : m_data(std::move(data)) {}
const SCP_vector<uint8_t>& bytearray_h::data() const
{
	return m_data;
}

void bytearray_h::serialize(lua_State* L, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	bytearray_h arr;
	value.getValue(l_Bytearray.Get(&arr));
	if (arr.m_data.size() > 0xffU) {
		LuaError(L, "bytearray too large to be network serialized. Maxium size is %d bytes!", 0xff);
		throw lua_net_exception("Bytearray too large to be network serialized.");
	}
	else if (MAX_PACKET_SIZE - packet_size < static_cast<int>(arr.m_data.size())) {
		LuaError(L, "Packet is too full to accomodate bytearray. Put less into one RPC call!");
		throw lua_net_exception("Packet is too full to accomodate bytearray.");
	}
	uint8_t size = static_cast<uint8_t>(arr.m_data.size());
	ADD_DATA(size);
	ADD_DATA_BLOCK(arr.m_data.data(), size);
}

void bytearray_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	uint8_t size;
	uint8_t read[0xff];
	SCP_vector<uint8_t> assign;
	GET_DATA(size);
	GET_DATA_BLOCK(read, size);
	assign.assign(read, read + size);
	new(data_ptr) bytearray_h(std::move(assign));
}

//**********HANDLE: bytearray
ADE_OBJ(l_Bytearray, bytearray_h, "bytearray", "An array of binary data");

ADE_FUNC(__len, l_Bytearray, nullptr, "The number of bytes in this array", "number", "The length in bytes")
{
	bytearray_h* array = nullptr;
	if (!ade_get_args(L, "o", l_Bytearray.GetPtr(&array))) {
		return ade_set_args(L, "i", 0);
	}

	return ade_set_args(L, "i", array->data().size());
}

} // namespace api
} // namespace scripting

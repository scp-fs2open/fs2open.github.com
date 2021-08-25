#include "bytearray.h"

namespace scripting {
namespace api {

bytearray_h::bytearray_h() = default;
bytearray_h::bytearray_h(SCP_vector<uint8_t> data) : m_data(std::move(data)) {}
const SCP_vector<uint8_t>& bytearray_h::data() const
{
	return m_data;
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

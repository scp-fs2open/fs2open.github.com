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
};

DECLARE_ADE_OBJ(l_Bytearray, bytearray_h);

} // namespace api
} // namespace scripting

#pragma once

#include "globalincs/pstypes.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

struct texture_h {
	int handle = -1;

	texture_h();
	explicit texture_h(int bm, bool refcount = true);

	~texture_h();

	texture_h(const texture_h&) = delete;
	texture_h& operator=(const texture_h&) = delete;

	texture_h(texture_h&&) noexcept;
	texture_h& operator=(texture_h&&) noexcept;

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Texture, texture_h);

}
}


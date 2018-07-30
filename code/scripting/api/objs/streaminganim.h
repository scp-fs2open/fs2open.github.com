#pragma once

#include "globalincs/pstypes.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"
#include "graphics/generic.h"

namespace scripting {
namespace api {

class streaminganim_h {
 public:
	generic_anim ga;

	bool IsValid();
	explicit streaminganim_h (const char* filename);
	~streaminganim_h();

	streaminganim_h(const streaminganim_h&) = delete;
	streaminganim_h& operator=(const streaminganim_h&) = delete;

	streaminganim_h(streaminganim_h&&);
	streaminganim_h& operator=(streaminganim_h&&);
};

DECLARE_ADE_OBJ(l_streaminganim, streaminganim_h);

}
}

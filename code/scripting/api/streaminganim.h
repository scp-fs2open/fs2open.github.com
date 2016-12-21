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
};

DECLARE_ADE_OBJ(l_streaminganim, streaminganim_h);

}
}

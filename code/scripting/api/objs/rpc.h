#pragma once

#include "scripting/ade_api.h"
#include "object/object.h"

namespace scripting {
	namespace api {
		struct rpc_h {
			luacpp::LuaFunction func;
		};

		DECLARE_ADE_OBJ(l_RPC, rpc_h);
	}
}

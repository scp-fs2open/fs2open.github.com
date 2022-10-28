#pragma once

#include "scripting/ade_api.h"
#include "object/object.h"

#include <memory>

namespace scripting {
	namespace api {
		class rpc_h_impl;
		using rpc_h = std::shared_ptr<rpc_h_impl>;
		using rpc_h_ref = std::weak_ptr<rpc_h_impl>;

		class rpc_h_impl {

			explicit rpc_h_impl(SCP_string _name);
		public:
			static rpc_h create(lua_State* L, SCP_string name);
			~rpc_h_impl();

			const ushort hash : 13;
			const SCP_string name;
			luacpp::LuaFunction func;
		};

		DECLARE_ADE_OBJ(l_RPC, rpc_h);
	}
}

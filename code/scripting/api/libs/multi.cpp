#include "multi.h"

#include "scripting/api/objs/rpc.h"
#include "network/multi.h"

namespace scripting {
	namespace api {
		//**********LIBRARY: Multi
		ADE_LIB(l_Multi, "Multi", "multi", "Functions for scripting for and in multiplayer environments.");

		ADE_FUNC(isServer, l_Multi, nullptr, "Prints a string", "boolean", "true if the script is running on the server, false if it is running on a client or in singleplayer.")
		{
			return ade_set_args(L, "b", MULTIPLAYER_MASTER);
		}

		ADE_FUNC(addRPC, l_Multi, "string name, function(any arg) => void rpc_body, [enumeration mode = RPC_RELIABLE, enumeration recipient = RPC_BOTH]", "Adds a remote procedure call. This call must run on all clients / the server where the RPC is expected to be able to execute. "
			"The given RPC name must be unique.\nFor advanced users: "
			"It is possible have different clients / servers add different RPC methods with the same name. "
			"In this case, each client will run their registered method when a different client calls the RPC with the corresponding name. "
			"Passing nil as the execution function means that the RPC can be called from this client, but not on this client.\n"
			"The mode is used to determine how the data is transmitted. RPC_RELIABLE means that data is guaranteed to arrive, and to be in order. "
			"RPC_ORDERED is a faster variant that guarantees that calls from the same caller to the same functions will always be in order, but can drop. "
			"Calls to clients are expected to drop slightly more often than calls to servers in this mode. RPC_UNRELIABLE is the fastest mode, but has no "
			"guarantees about ordering of calls, and does not guarantee arrival (but will be slightly better than RPC_ORDERED).\n"
			"The recipient will be used as the default recipient for this RPC, but can be overridden on a per-call basis. "
			"Valid are: RPC_SERVER, RPC_CLIENTS, RPC_BOTH", "rpc", "An RPC object, or an invalid RPC object on failure.")
		{
			const char* name;
			luacpp::LuaFunction func;
			enum_h recipient{LE_RPC_BOTH}, mode{LE_RPC_RELIABLE};
			if (!ade_get_args(L, "su|oo", &name, &func, l_Enum.Get(&mode), l_Enum.Get(&recipient))) {
				return ade_set_error(L, "o", l_RPC.Set(nullptr));
			}

			if (!recipient.IsValid() || (recipient.index != LE_RPC_SERVER && recipient.index != LE_RPC_CLIENTS && recipient.index != LE_RPC_BOTH)) {
				LuaError(L, "Tried to create an RPC with an invalid recipient enumeration!");
				return ade_set_error(L, "o", l_RPC.Set(nullptr));
			}

			if (!mode.IsValid() || (mode.index != LE_RPC_RELIABLE && mode.index != LE_RPC_ORDERED && mode.index != LE_RPC_UNRELIABLE)) {
				LuaError(L, "Tried to create an RPC with an invalid mode enumeration!");
				return ade_set_error(L, "o", l_RPC.Set(nullptr));
			}

			auto rpc = rpc_h_impl::create(L, name);
			rpc->func = func;
			rpc->recipient = recipient;
			rpc->mode = mode;
			return ade_set_args(L, "o", l_RPC.Set(std::move(rpc)));
		}
	}
}
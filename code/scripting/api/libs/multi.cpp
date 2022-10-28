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

		ADE_FUNC(addRPC, l_Multi, "string name, function(any arg) => void rpc_body", "Adds a remote procedure call. This call must run on all clients / the server where the RPC is expected to be able to execute. "
			"The given RPC name must be unique.\nFor advanced users: "
			"It is possible have different clients / servers add different RPC methods with the same name. "
			"In this case, each client will run their registered method when a different client calls the RPC with the corresponding name. "
			"Passing nil as the execution function means that the RPC can be called from this client, but not on this client.", "rpc", "An RPC object.")
		{
			return ade_set_args(L, "o", l_RPC.Set(rpc_h_impl::create(L, "")));
		}
	}
}
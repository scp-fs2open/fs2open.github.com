#include "multi.h"

#include "network/multi.h"

namespace scripting {
	namespace api {
		//**********LIBRARY: Multi
		ADE_LIB(l_Multi, "Multi", "multi", "Functions for scripting for and in multiplayer environments.");

		ADE_FUNC(isServer, l_Multi, nullptr, "Prints a string", "boolean", "true if the script is running on the server, false if it is running on a client or in singleplayer.")
		{
			return ade_set_args(L, "b", MULTIPLAYER_MASTER);
		}
	}
}
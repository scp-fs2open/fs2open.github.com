#include "rpc.h"

#include "network/multi_lua.h"
#include "scripting/ade_api.h"
#include "scripting/api/objs/enums.h"

namespace scripting {
namespace api {

rpc_h rpc_h_impl::create(lua_State* L, SCP_string name) {
	rpc_h newRPC{ new rpc_h_impl(std::move(name)) };
	if (!add_rpc(newRPC)) {
		LuaError(L, "This name is already in use for an RPC on this machine.\nIf you are using different RPC's for different "
			"machines with the same name, make sure that each machine actually only intializes one RPC object.");
		return nullptr;
	}
	return newRPC;
}

rpc_h_impl::rpc_h_impl(SCP_string _name) : hash(0), name(std::move(_name)) {

}

rpc_h_impl::~rpc_h_impl() {
	//This destructor needs to unregister the function. Once this gets called, we have no further access to the RPC method. It can't be called anymore, and shouldn't recieve packets

	//cleaing the RPC-Repositories weak_ptr list here causes UB per LWG issue 2751. However, since this is just cleanup,
	//it doesn't matter to us whether the weak_ptr pointing to this very object is cleared as well.
	//Worst case, and it'll be cleared when the next RPC object gets destroyed.
	clean_rpc_refs();
}

//**********HANDLE: Remote Procedure Call
ADE_OBJ(l_RPC, rpc_h, "rpc", "A function object for remote procedure calls");

ADE_FUNC(__newindex, l_RPC, "function(any arg) => void rpc_body", "Sets the function to be called when the RPC is invoked on this client", "function(any arg) => void", "The function the RPC is set to")
{
	rpc_h rpc;
	luacpp::LuaFunction func;
	if (!ade_get_args(L, "ou", l_RPC.Get(&rpc), &func)) {
		return ade_set_error(L, "u", rpc->func);
	}

	rpc->func = func;

	return ade_set_args(L, "u", rpc->func);
}

ADE_FUNC(__call, l_RPC, "[any = nil, enumeration recipient = default /* as set on RPC creation */]", "Calls the RPC on the specified recipients with the given argument.", nullptr, nullptr)
{
	rpc_h rpc;
	luacpp::LuaValue argument;
	enum_h* recipient;
	if (!ade_get_args(L, "o|ao", l_RPC.Get(&rpc), &argument, l_Enum.GetPtr(&recipient))) {
		return ADE_RETURN_NIL;
	}

	if(!argument.isValid())
		argument = luacpp::LuaValue::createNil(L);

	if (!recipient->IsValid()) {

	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(waitAsync,
	l_RPC,
	nullptr,
	"Performs an asynchronous wait until this RPC has been evoked on this client and the RPC function has finished running. Does NOT trigger when the RPC is called from this client.",
	"promise",
	"A promise with no return value that resolves when this RPC has been called the next time.")
{
	return 0;
}

}
}
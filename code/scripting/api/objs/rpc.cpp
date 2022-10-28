#include "rpc.h"

#include "network/multi_lua.h"
#include "scripting/ade_api.h"
#include "scripting/api/objs/enums.h"

namespace scripting {
namespace api {

//**********HANDLE: Remote Procedure Call
ADE_OBJ(l_RPC, rpc_h, "rpc", "A function object for remote procedure calls");

ADE_FUNC(__newindex, l_RPC, "function(any arg) => void rpc_body", "Sets the function to be called when the RPC is invoked on this client", "function(any arg) => void", "The function the RPC is set to")
{
	rpc_h* rpc;
	luacpp::LuaFunction func;
	if (!ade_get_args(L, "ou", l_RPC.GetPtr(&rpc), &func)) {
		return ade_set_error(L, "u", rpc->func);
	}

	rpc->func = func;

	return ade_set_args(L, "u", rpc->func);
}

ADE_FUNC(__call, l_RPC, "[any = nil, enumeration recipient = default /* as set on RPC creation */]", "Calls the RPC on the specified recipients with the given argument.", nullptr, nullptr)
{
	rpc_h* rpc;
	luacpp::LuaValue argument;
	enum_h* recipient;
	if (!ade_get_args(L, "o|ao", l_RPC.GetPtr(&rpc), &argument, l_Enum.GetPtr(&recipient))) {
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

}
}
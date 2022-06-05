//
//

#include "control_binding.h"

namespace scripting {
namespace api {

cci_h::cci_h() { idx = CCFG_MAX; }

cci_h::cci_h(int n_id) { idx = static_cast<IoActionId>(n_id); }

bool cci_h::IsValid() { return (idx > -1 && idx < IoActionId::CCFG_MAX); }

IoActionId cci_h::Get() { return idx; }

ADE_OBJ(l_ControlBinding, cci_h, "keybinding", "Key Binding");

ADE_FUNC(__tostring, l_ControlBinding, nullptr, "Key binding name", "string", "Key binding name, or empty string if handle is invalid")
{
	cci_h* cci = nullptr;
	if(!ade_get_args(L, "o", l_ControlBinding.GetPtr(&cci)))
		return ade_set_error(L, "s", "");

	if(!cci->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", ValToAction(cci->Get()));
}

ADE_VIRTVAR(Name, l_ControlBinding, nullptr, "Key binding name", "string", "Key binding name, or empty string if handle is invalid")
{
	cci_h* cci = nullptr;
	if (!ade_get_args(L, "o", l_ControlBinding.GetPtr(&cci)))
		return ade_set_error(L, "s", "");

	if (!cci->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", ValToAction(cci->Get()));
}

ADE_FUNC(getInputName, l_ControlBinding, "[boolean primaryBinding = true]", "The name of the bound input", "string", "The name of the bound input, or empty string if nothing is bound or handle is invalid")
{
	cci_h* cci = nullptr;
	bool primary = true;
	if (!ade_get_args(L, "o|b", l_ControlBinding.GetPtr(&cci), &primary))
		return ADE_RETURN_NIL;

	if (!cci->IsValid()) 
		return ade_set_error(L, "s", "");

	CCI &cci_ref = Control_config[cci->Get()];

	return ade_set_args(L, "s", (primary ? cci_ref.first : cci_ref.second).textify());
}

ADE_FUNC(lock, l_ControlBinding, "boolean lock", "Locks this control binding when true, disables if false. Persistent between missions.", nullptr, nullptr)
{
	cci_h* cci = nullptr;
	bool lock = false;
	if (!ade_get_args(L, "ob", l_ControlBinding.GetPtr(&cci), &lock))
		return ADE_RETURN_NIL;

	if (!cci->IsValid()) {
		LuaError(L, "Cannot lock hooks for an invalid binding.\n");

		return ADE_RETURN_NIL;
	}

	Control_config[cci->Get()].locked = lock;

	return ADE_RETURN_NIL;
}

ADE_FUNC(isLocked, l_ControlBinding, nullptr, "If this control is locked", "boolean lock", "If this control is locked, nil if the handle is invalid")
{
	cci_h* cci = nullptr;
	if (!ade_get_args(L, "o", l_ControlBinding.GetPtr(&cci)))
		return ADE_RETURN_NIL;

	if (!cci->IsValid()) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", Control_config[cci->Get()].locked);
}

ADE_FUNC(registerHook, l_ControlBinding, "function() => void | boolean hook, [boolean enabledByDefault = false, boolean isOverride = false]", "Registers a hook for this keybinding, either as a normal hook, or as an override", nullptr, nullptr) {

	cci_h* cci = nullptr;
	luacpp::LuaFunction hook;
	bool enabled = false;
	bool isOverride = false;

	if (!ade_get_args(L, "ou|bb", l_ControlBinding.GetPtr(&cci), &hook, &enabled, &isOverride)) {
		return ADE_RETURN_NIL;
	}

	if (!cci->IsValid()) {
		return ADE_RETURN_NIL;
	}

	if(!hook.isValid()){
		//Nil removes the hook
		control_register_hook(cci->Get(), luacpp::LuaFunction(), isOverride, false);

		return ADE_RETURN_NIL;
	}

	control_register_hook(cci->Get(), hook, isOverride, enabled);

	return ADE_RETURN_NIL;
}

ADE_FUNC(enableScripting, l_ControlBinding, "boolean enable", "Enables scripted control hooks for this keybinding when true, disables if false. Not persistent between missions.", nullptr, nullptr)
{
	cci_h* cci = nullptr;
	bool enable = false;
	if (!ade_get_args(L, "ob", l_ControlBinding.GetPtr(&cci), &enable))
		return ADE_RETURN_NIL;

	if (!cci->IsValid()) {
		LuaError(L, "Cannot enable or disable scripting hooks for an invalid binding.\n");
		
		return ADE_RETURN_NIL;
	}

	control_enable_hook(cci->Get(), enable);

	return ADE_RETURN_NIL;
}


}
}

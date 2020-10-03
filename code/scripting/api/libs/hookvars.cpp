//
//

#include "hookvars.h"

#include "scripting/scripting.h"

namespace scripting {
namespace api {

//**********LIBRARY: Scripting Variables
ADE_LIB(l_HookVar, "HookVariables", "hv", "Hook variables repository");

ADE_INDEXER(l_HookVar,
	"string variableName",
	"Retrieves a hook variable value",
	"any",
	"The hook variable value or nil if hook variable is not defined")
{
	const char* name;
	if (!ade_get_args(L, "*s", &name)) {
		return ADE_RETURN_NIL;
	}

	const auto scriptSystem = script_state::GetScriptState(L);

	const auto& hookVars = scriptSystem->GetHookVariableReferences();
	const auto iter = hookVars.find(name);
	if (iter == hookVars.end()) {
		return ADE_RETURN_NIL;
	}
	if (iter->second.empty()) {
		// Hook variable existed at some point but was removed again
		return ADE_RETURN_NIL;
	}

	// Use the value on top of the stack
	iter->second.back()->pushValue(L);
	return 1;
}

// WMC: IMPORTANT
// Be very careful when modifying this library, as the Globals[] library does depend
// on the current number of items in the library. If you add _anything_, modify __len.
// Or run changes by me.

//*****LIBRARY: Scripting Variables
ADE_LIB_DERIV(l_HookVar_Globals, "Globals", nullptr, nullptr, l_HookVar);

ADE_INDEXER(l_HookVar_Globals,
	"number Index",
	"Array of current HookVariable names",
	"string",
	"Hookvariable name, or empty string if invalid index specified")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "s", "");

	const auto scriptSystem = script_state::GetScriptState(L);

	const auto& hookVars = scriptSystem->GetHookVariableReferences();

	// List 'em
	int count = 1;
	for (const auto& pair : hookVars) {
		if (pair.second.empty()) {
			// Skip empty value stacks
			continue;
		}

		if (count == idx) {
			return ade_set_args(L, "s", pair.first);
		}
		count++;
	}

	return ade_set_error(L, "s", "");
}

ADE_FUNC(__len, l_HookVar_Globals, NULL, "Number of HookVariables", "number", "Number of HookVariables")
{
	const auto scriptSystem = script_state::GetScriptState(L);

	const auto& hookVars = scriptSystem->GetHookVariableReferences();

	// Since the values are on a stack, it is possible to have entries in the map that have no values at the moment
	auto validHookVars = std::count_if(hookVars.cbegin(),
		hookVars.cend(),
		[](const std::pair<SCP_string, SCP_vector<luacpp::LuaReference>>& values) { return !values.second.empty(); });

	return ade_set_args(L, "i", validHookVars);
}

} // namespace api
} // namespace scripting

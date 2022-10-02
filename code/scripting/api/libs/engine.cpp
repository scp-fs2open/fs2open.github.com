//
//

#include "engine.h"

#include "osapi/outwnd.h"
#include "scripting/ade_api.h"
#include "scripting/api/objs/tracing_category.h"
#include "scripting/scripting.h"

namespace scripting {
namespace api {

//**********LIBRARY: Engine
ADE_LIB(l_Engine, "Engine", "engine", "Basic engine access functions");

ADE_FUNC(addHook,
	l_Engine,
	"string name, function() => void hookFunction, [table conditionals /* Empty table by default */, function() => "
	"boolean override_func "
	"/* Function returning false by default */]",
	"Adds a function to be called from the specified game hook",
	"boolean",
	"true if hook was installed properly, false otherwise")
{
	using namespace luacpp;

	const char* hook_name;
	LuaFunction hook;
	LuaTable conditionals;
	LuaFunction override_func;
	if (!ade_get_args(L, "su|tu", &hook_name, &hook, &conditionals, &override_func)) {
		return ADE_RETURN_FALSE;
	}

	if (!hook.isValid()) {
		LuaError(L, "Hook function is invalid!");
		return ADE_RETURN_FALSE;
	}

	ConditionedHook cond_hook;

	script_action action;
	action.action_type = scripting_string_to_action(hook_name);

	if (action.action_type == CHA_NONE) {
		LuaError(L, "Invalid hook name '%s'!", hook_name);
		return ADE_RETURN_FALSE;
	}

	action.hook.hook_function.language = SC_LUA;
	action.hook.hook_function.function = hook;

	if (override_func.isValid()) {
		action.hook.override_function.language = SC_LUA;
		action.hook.override_function.function = override_func;
	}

	cond_hook.AddAction(&action);

	if (conditionals.isValid()) {
		for (const auto& tableEntry : conditionals) {
			const auto& key   = tableEntry.first;
			const auto& value = tableEntry.second;

			if (!key.is(ValueType::STRING)) {
				LuaError(L, "Conditional key '%s' is not a string", key.getValue<SCP_string>().c_str());
				return ADE_RETURN_FALSE;
			}

			if (!value.is(ValueType::STRING)) {
				LuaError(L, "Conditional value '%s' for key '%s' is not a string", value.getValue<SCP_string>().c_str(),
						 key.getValue<SCP_string>().c_str());
				return ADE_RETURN_FALSE;
			}

			script_condition cond;
			cond.condition_type   = scripting_string_to_condition(key.getValue<SCP_string>().c_str());
			cond.condition_string = value.getValue<SCP_string>();
			cond.condition_cached_value = -1;

			cond_hook.AddCondition(&cond);
		}
	}

	Script_system.AddConditionedHook(std::move(cond_hook));
	return ADE_RETURN_TRUE;
}

ADE_FUNC(sleep,
	l_Engine,
	"number seconds",
	"Executes a <b>blocking</b> sleep. Usually only necessary for development or testing purposes. Use with care!",
	nullptr,
	nullptr)
{
	float seconds = 0.0f;
	if (!ade_get_args(L, "f", &seconds)) {
		return ADE_RETURN_NIL;
	}

	os_sleep(fl2i(seconds * 1000.0f));
	return ADE_RETURN_NIL;
}

ADE_FUNC(createTracingCategory,
	l_Engine,
	"string name, [boolean gpu_category = false]",
	"Creates a new category for tracing the runtime of a code segment. Also allows to trace how long the corresponding "
	"code took on the GPU.",
	"tracing_category",
	"The allocated category.")
{
	const char* name  = nullptr;
	bool gpu_category = false;
	if (!ade_get_args(L, "s|b", &name, &gpu_category)) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_TracingCategory.Set(tracing::Category(name, gpu_category)));
}

ADE_FUNC(restartLog,
	l_Engine,
	"","Closes and reopens the fs2_open.log",nullptr,nullptr)
{
	//These returns are very crude, but if we return nil then l_Engine isn't use and tidy gets cranky.
	if (Log_debug_output_to_file) {
		outwnd_close();
		outwnd_init();
		return ADE_RETURN_TRUE;
	}
	return ADE_RETURN_FALSE;
}

} // namespace api
} // namespace scripting

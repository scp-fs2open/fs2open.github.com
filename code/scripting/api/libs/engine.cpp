//
//

#include "engine.h"

#include "parse/parselo.h"
#include "osapi/outwnd.h"
#include "scripting/ade_api.h"
#include "scripting/api/objs/tracing_category.h"
#include "scripting/scripting.h"
#include "scripting/hook_api.h"

extern int cache_condition(ConditionalType type, const SCP_string& value);

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

	script_action action;
	auto action_hook = scripting_string_to_action(hook_name);

	if (action_hook == nullptr) {
		LuaError(L, "Invalid hook name '%s'!", hook_name);
		return ADE_RETURN_FALSE;
	}

	action.hook.hook_function.language = SC_LUA;
	action.hook.hook_function.function = hook;

	if (override_func.isValid()) {
		action.hook.override_function.language = SC_LUA;
		action.hook.override_function.function = override_func;
	}

	if (action_hook->getDeprecation()) {
		const auto& deprecation = *action_hook->getDeprecation();
		bool shownWarn = false;
		if (deprecation.level_hook == HookDeprecationOptions::DeprecationLevel::LEVEL_ERROR) {
			LuaError(L, "Hook '%s' is removed since version %s and cannot be used!", action_hook->getHookName().c_str(), gameversion::format_version(deprecation.deprecatedSince).c_str());
		}
		else if (mod_supports_version(deprecation.deprecatedSince)) {
			LuaError(L, "Hook '%s' is deprecated since version %s and cannot be used if the mod targets that version or higher!", action_hook->getHookName().c_str(), gameversion::format_version(deprecation.deprecatedSince).c_str());
		}
		else {
			Warning(LOCATION, "Hook '%s' is deprecated from version %s and should be replaced!", action_hook->getHookName().c_str(), gameversion::format_version(deprecation.deprecatedSince).c_str());
			shownWarn = true;
		}
		if (override_func.isValid()) {
			if (deprecation.level_override == HookDeprecationOptions::DeprecationLevel::LEVEL_ERROR) {
				LuaError(L, "Overriding Hook '%s' is removed since version %s and cannot be used!", action_hook->getHookName().c_str(), gameversion::format_version(deprecation.deprecatedSince).c_str());
			}
			else if (mod_supports_version(deprecation.deprecatedSince)) {
				LuaError(L, "Overriding Hook '%s' is deprecated since version %s and cannot be used if the mod targets that version or higher!", action_hook->getHookName().c_str(), gameversion::format_version(deprecation.deprecatedSince).c_str());
			}
			else if (!shownWarn) {
				Warning(LOCATION, "Overriding Hook '%s' is deprecated from version %s and should be replaced!", action_hook->getHookName().c_str(), gameversion::format_version(deprecation.deprecatedSince).c_str());
			}
		}
	}

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

			const SCP_string& condition_key = key.getValue<SCP_string>();
			auto condition = scripting_string_to_condition(condition_key.c_str());
			SCP_string condition_value = value.getValue<SCP_string>();

			if (condition != CHC_NONE) {
				//It's a global condition
				int cache = cache_condition(condition, condition_value);
				action.global_conditions.emplace_back(script_condition{ condition, std::move(condition_value), cache });
			}
			else {
				auto condition_it = action_hook->_conditions.find(condition_key);

				if (condition_it == action_hook->_conditions.end()) {
					LuaError(L, "Condition '%s' is not valid for hook '%s'. The hook will not evaluate!", condition_value.c_str(), action_hook->getHookName().c_str());
					return ADE_RETURN_FALSE;
				}

				action.local_conditions.emplace_back(condition_it->second->parse(condition_value));
			}
		}
	}

	Script_system.AddConditionedHook(action_hook->getHookId(), std::move(action));
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

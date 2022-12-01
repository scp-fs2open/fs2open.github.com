#pragma once

#include "globalincs/pstypes.h"

#include "ade_api.h"
#include "scripting.h"

#include "utils/tuples.h"

#include <utility>

namespace scripting {

namespace detail {
ade_odata_setter<object_h> convert_arg_type(object* objp);
ade_odata_setter<vec3d> convert_arg_type(vec3d vec);

template <typename T>
T&& convert_arg_type(T&& arg)
{
	return std::forward<T>(arg);
}

template <typename T>
struct HookParameterInstance {
	SCP_string name;
	char type = '\0';
	T value;
	bool enabled = true;

	HookParameterInstance(SCP_string name_, char type_, T&& value_, bool enabled_)
		: name(std::move(name_)), type(type_), value(std::forward<T>(value_)), enabled(enabled_)
	{
	}
};

struct SetSingleHookVarHelper {
	SCP_vector<SCP_string>& paramNames;

	SetSingleHookVarHelper(SCP_vector<SCP_string>& paramNames_) : paramNames(paramNames_) {}

	template <typename T>
	void operator()(HookParameterInstance<T>&& instance)
	{
		// If a parameter is not enabled, skip it
		if (!instance.enabled) {
			return;
		}

		paramNames.push_back(instance.name);

		Script_system.SetHookVar(instance.name.c_str(),
								 instance.type,
								 detail::convert_arg_type(std::move(instance.value)));
	}
};

template <typename... Args>
struct HookParameterInstanceList {
	std::tuple<HookParameterInstance<Args>...> params;

	HookParameterInstanceList(HookParameterInstance<Args>&&... params_)
		: params(std::forward<HookParameterInstance<Args>>(params_)...)
	{
	}

	void setHookVars(SCP_vector<SCP_string>& paramNames)
	{
		util::tuples::for_each<0, SetSingleHookVarHelper, HookParameterInstance<Args>...>(
			std::move(params),
			SetSingleHookVarHelper(paramNames));
	}
};

} // namespace detail

template <typename T>
detail::HookParameterInstance<T> hook_param(SCP_string name_, char type_, T&& value_, bool enabled = true)
{
	return detail::HookParameterInstance<T>(std::move(name_), type_, std::forward<T>(value_), enabled);
}

template <typename... Args>
detail::HookParameterInstanceList<Args...> hook_param_list(detail::HookParameterInstance<Args>&&... params)
{
	return detail::HookParameterInstanceList<Args...>(std::forward<detail::HookParameterInstance<Args>>(params)...);
}

struct HookVariableDocumentation {
	const char* name = nullptr;
	scripting::ade_type_info type;
	const char* description = nullptr;

	HookVariableDocumentation(const char* name_, scripting::ade_type_info type_, const char* description_);
};


class HookBase {
  public:
	HookBase(SCP_string hookName,
			 SCP_string description,
			 SCP_vector<HookVariableDocumentation> parameters,
			 const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>& conditions,
			 int32_t hookId = -1);
	virtual ~HookBase();

	const SCP_string& getHookName() const;
	const SCP_string& getDescription() const;
	const SCP_vector<HookVariableDocumentation>& getParameters() const;
	int32_t getHookId() const;

	virtual bool isActive() const = 0;
	virtual bool isOverridable() const = 0;

	const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>& _conditions;

  protected:
	SCP_string _hookName;
	SCP_string _description;
	SCP_vector<HookVariableDocumentation> _parameters;
	int32_t _hookId = 0;
	
	bool hasParameter(const SCP_string& param) const
	{
		return std::find_if(_parameters.begin(), _parameters.end(), [&param](const HookVariableDocumentation& test) {
				   return test.name == param;
			   }) != _parameters.end();
	}
};

template<typename condition_t = void>
class Hook : public HookBase {
  protected:
	template<typename C = condition_t>
	inline static typename std::enable_if<std::is_void<C>::value, const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>>::type getConditions() {
		return SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>{};
	}
	template<typename C = condition_t>
	inline static typename std::enable_if<!std::is_void<C>::value, const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>>::type getConditions() {
		return condition_t::conditions;
	}

  public:
	Hook(SCP_string hookName, SCP_string description, SCP_vector<HookVariableDocumentation> parameters, int32_t hookId)
		  : HookBase(std::move(hookName), std::move(description), std::move(parameters), getConditions(), hookId) { };
	~Hook() override = default;

	static std::shared_ptr<Hook<condition_t>> Factory(SCP_string hookName,
		SCP_string description,
		SCP_vector<HookVariableDocumentation> parameters,
		int32_t hookId = -1) {
		return std::make_shared<Hook<condition_t>>(std::move(hookName), std::move(description), std::move(parameters), hookId);
	}
	static std::shared_ptr<Hook<condition_t>> Factory(SCP_string hookName, int32_t hookId) {
		return std::make_shared<Hook<condition_t>>(std::move(hookName), SCP_string(), SCP_vector<HookVariableDocumentation>(), hookId);
	}

	bool isActive() const override {
		return Script_system.IsActiveAction(_hookId);
	}

	bool isOverridable() const override {
		return false;
	};

	template <typename C = condition_t, typename condition_pass_t = typename std::enable_if<!std::is_void<C>::value, C>::type, typename... Args>
	typename int run(condition_pass_t condition, detail::HookParameterInstanceList<Args...> argsList = hook_param_list<Args...>()) const
	{
		SCP_vector<SCP_string> paramNames;
		argsList.setHookVars(paramNames);

#ifndef NDEBUG
		std::for_each(paramNames.begin(), paramNames.end(), [this](const SCP_string& param) {
			Assertion(hasParameter(param),
					  "Hook '%s' does not accept parameter '%s'.",
					  _hookName.c_str(),
					  param.c_str());
		});
#endif

		const auto num_run = Script_system.RunCondition(_hookId, linb::any(std::move(condition)));

		for (const auto& param : paramNames) {
			Script_system.RemHookVar(param.c_str());
		}

		return num_run;
	}

	template <typename C = condition_t, typename... Args>
	typename std::enable_if<std::is_void<C>::value, int>::type run(detail::HookParameterInstanceList<Args...> argsList = hook_param_list<Args...>()) const
	{
		SCP_vector<SCP_string> paramNames;
		argsList.setHookVars(paramNames);

#ifndef NDEBUG
		std::for_each(paramNames.begin(), paramNames.end(), [this](const SCP_string& param) {
			Assertion(hasParameter(param),
				"Hook '%s' does not accept parameter '%s'.",
				_hookName.c_str(),
				param.c_str());
			});
#endif

		const auto num_run = Script_system.RunCondition(_hookId, linb::any{});

		for (const auto& param : paramNames) {
			Script_system.RemHookVar(param.c_str());
		}

		return num_run;
	}
};

template<typename condition_t = void>
class OverridableHook : public Hook<condition_t> {
  public:
	OverridableHook(SCP_string hookName,
		SCP_string description,
		SCP_vector<HookVariableDocumentation> parameters,
		int32_t hookId) : Hook<condition_t>(std::move(hookName), std::move(description), std::move(parameters), hookId) { }
	~OverridableHook() override = default;

	static std::shared_ptr<OverridableHook<condition_t>> Factory(SCP_string hookName,
		SCP_string description,
		SCP_vector<HookVariableDocumentation> parameters,
		int32_t hookId = -1) {
		return std::make_shared<OverridableHook<condition_t>>(std::move(hookName),
			std::move(description),
			std::move(parameters),
			hookId);
	}

	bool isOverridable() const override {
		return true;
	}

	template <typename C = condition_t, typename condition_pass_t = typename std::enable_if<!std::is_void<C>::value, C>::type, typename... Args>
	typename bool isOverride(condition_pass_t condition, detail::HookParameterInstanceList<Args...> argsList = hook_param_list<Args...>()) const
	{
		SCP_vector<SCP_string> paramNames;
		argsList.setHookVars(paramNames);

#ifndef NDEBUG
		std::for_each(paramNames.begin(), paramNames.end(), [this](const SCP_string& param) {
			Assertion(hasParameter(param),
					  "Hook '%s' does not accept parameter '%s'.",
					  _hookName.c_str(),
					  param.c_str());
		});
#endif

		const auto ret_val = Script_system.IsConditionOverride(_hookId, linb::any(std::move(condition)));

		for (const auto& param : paramNames) {
			Script_system.RemHookVar(param.c_str());
		}

		return ret_val;
	}

	template <typename C = condition_t, typename... Args>
	typename std::enable_if<std::is_void<C>::value, bool>::type isOverride(detail::HookParameterInstanceList<Args...> argsList = hook_param_list<Args...>()) const
	{
		SCP_vector<SCP_string> paramNames;
		argsList.setHookVars(paramNames);

#ifndef NDEBUG
		std::for_each(paramNames.begin(), paramNames.end(), [this](const SCP_string& param) {
			Assertion(hasParameter(param),
				"Hook '%s' does not accept parameter '%s'.",
				_hookName.c_str(),
				param.c_str());
			});
#endif

		const auto ret_val = Script_system.IsConditionOverride(_hookId, linb::any{});

		for (const auto& param : paramNames) {
			Script_system.RemHookVar(param.c_str());
		}

		return ret_val;
	}
};

const SCP_vector<HookBase*>& getHooks();

} // namespace scripting

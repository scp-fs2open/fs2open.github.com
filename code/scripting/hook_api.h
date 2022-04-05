#pragma once

#include "globalincs/pstypes.h"

#include "ade_api.h"
#include "scripting.h"

#include "utils/tuples.h"

#include <utility>

namespace scripting {

namespace detail {
ade_odata_setter<object_h> convert_arg_type(object* objp);

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

	HookParameterInstance(SCP_string name_, char type_, T&& value_)
		: name(std::move(name_)), type(type_), value(std::forward<T>(value_))
	{
	}
};

struct SetSingleHookVarHelper {
	SCP_vector<SCP_string>& paramNames;

	SetSingleHookVarHelper(SCP_vector<SCP_string>& paramNames_) : paramNames(paramNames_) {}

	template <typename T>
	void operator()(HookParameterInstance<T>&& instance)
	{
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
detail::HookParameterInstance<T> hook_param(SCP_string name_, char type_, T&& value_)
{
	return detail::HookParameterInstance<T>(std::move(name_), type_, std::forward<T>(value_));
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
			 int32_t hookId = -1);
	virtual ~HookBase();

	const SCP_string& getHookName() const;
	const SCP_string& getDescription() const;
	const SCP_vector<HookVariableDocumentation>& getParameters() const;
	int32_t getHookId() const;

	virtual bool isOverridable() const = 0;

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

class Hook : public HookBase {
  public:
	Hook(SCP_string hookName, SCP_string description, SCP_vector<HookVariableDocumentation> parameters, int32_t hookId);
	~Hook() override;

	static std::shared_ptr<Hook> Factory(SCP_string hookName,
										 SCP_string description,
										 SCP_vector<HookVariableDocumentation> parameters,
										 int32_t hookId = -1);
	static std::shared_ptr<Hook> Factory(SCP_string hookName, int32_t hookId);

	bool isOverridable() const override;

	template <typename... Args>
	int run(detail::HookParameterInstanceList<Args...> argsList = hook_param_list<Args...>(),
			object* objp                                        = nullptr,
			int more_data                                       = 0) const
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

		const auto num_run = Script_system.RunCondition(_hookId, objp, more_data);

		for (const auto& param : paramNames) {
			Script_system.RemHookVar(param.c_str());
		}

		return num_run;
	}
};

class OverridableHook : public Hook {
  public:
	OverridableHook(SCP_string hookName,
					SCP_string description,
					SCP_vector<HookVariableDocumentation> parameters,
					int32_t hookId);
	~OverridableHook() override;

	static std::shared_ptr<OverridableHook> Factory(SCP_string hookName,
													SCP_string description,
													SCP_vector<HookVariableDocumentation> parameters,
													int32_t hookId = -1);

	bool isOverridable() const override;

	template <typename... Args>
	bool isOverride(detail::HookParameterInstanceList<Args...> argsList = hook_param_list<Args...>(),
					object* objp                                        = nullptr) const
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

		const auto ret_val = Script_system.IsConditionOverride(_hookId, objp);

		for (const auto& param : paramNames) {
			Script_system.RemHookVar(param.c_str());
		}

		return ret_val;
	}
};

const SCP_vector<HookBase*>& getHooks();

} // namespace scripting

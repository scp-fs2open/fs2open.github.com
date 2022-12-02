#include "hook_api.h"

#include "globalincs/pstypes.h"

#include "scripting/api/objs/vecmath.h"

namespace scripting {

namespace {

class HookManager {
  public:
	int32_t addHook(HookBase* hook)
	{
		Assertion(std::find_if(_hooks.cbegin(),
							   _hooks.cend(),
							   [hook](const HookBase* test) { return test->getHookName() == hook->getHookName(); }) ==
					  _hooks.cend(),
				  "Hook '%s' already exists!",
				  hook->getHookName().c_str());

		const auto id = ++_lastId;
		_hooks.push_back(hook);
		return id;
	}

	void addHookWithId(HookBase* hook)
	{
		Assertion(std::find_if(_hooks.cbegin(),
							   _hooks.cend(),
							   [hook](const HookBase* test) { return test->getHookName() == hook->getHookName(); }) ==
					  _hooks.cend(),
				  "Hook '%s' already exists!",
				  hook->getHookName().c_str());

		_hooks.push_back(hook);
	}

	const SCP_vector<HookBase*>& getHooks() const { return _hooks; }

  private:
	int32_t _lastId = CHA_LAST;
	SCP_vector<HookBase*> _hooks;
};

HookManager& getHookManager()
{
	static HookManager mgr;
	return mgr;
}
} // namespace

namespace detail {
ade_odata_setter<object_h> convert_arg_type(object* objp)
{
	return ade_object_to_odata(objp != nullptr ? OBJ_INDEX(objp) : -1);
}
ade_odata_setter<vec3d> convert_arg_type(vec3d vec)
{
	return scripting::api::l_Vector.Set(vec);
}
} // namespace detail

HookVariableDocumentation::HookVariableDocumentation(const char* name_, ade_type_info type_, const char* description_)
	: name(name_), type(std::move(type_)), description(description_)
{
}

HookBase::HookBase(SCP_string hookName,
				   SCP_string description,
				   SCP_vector<HookVariableDocumentation> parameters,
				   const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>& conditions,
				   int32_t hookId)
	: _conditions(conditions), _hookName(std::move(hookName)), _description(std::move(description)), _parameters(std::move(parameters))
{
	// If we specify a forced id then use that. This is for special hooks that need a guaranteed id
	if (hookId >= 0) {
		_hookId = hookId;
		getHookManager().addHookWithId(this);
	} else {
		_hookId = getHookManager().addHook(this);
	}
}

const SCP_string& HookBase::getHookName() const { return _hookName; }
const SCP_string& HookBase::getDescription() const { return _description; }
const SCP_vector<HookVariableDocumentation>& HookBase::getParameters() const { return _parameters; }
int32_t HookBase::getHookId() const { return _hookId; }
HookBase::~HookBase() = default;

const SCP_vector<HookBase*>& getHooks() { return getHookManager().getHooks(); }

const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>> HookImpl<void>::emptyConditions{};

} // namespace scripting

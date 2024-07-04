#pragma once

#include "ActionDefinition.h"

#include "expression/Value.h"

namespace actions {

template <typename TAction>
class BuiltinActionDefinition : public ActionDefinition {
	using ActionValueType = typename TAction::ValueType;

  public:
	BuiltinActionDefinition(SCP_string name)
		: ActionDefinition(std::move(name), {{"parameter", expression::ValueTypeTraits<ActionValueType>::type}})
	{
	}
	~BuiltinActionDefinition() override = default;

	flagset<ProgramContextFlags> getRequiredContext() const override
	{
		return flagset<ProgramContextFlags>(TAction::getRequiredExecutionContextFlags());
	}

	std::unique_ptr<Action> createAction(
		const SCP_unordered_map<SCP_string, expression::ActionExpression>& parameterExpressions) const override
	{
		const auto paramIter = parameterExpressions.find("parameter");

		// This should have been caught earlier
		Assertion(paramIter != parameterExpressions.cend(), "Could not find built-in parameter!");

		return std::unique_ptr<Action>(new TAction(paramIter->second.template asTyped<ActionValueType>()));
	}
};

} // namespace actions

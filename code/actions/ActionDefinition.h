#pragma once

#include "Action.h"

#include "actions/expression/ActionExpression.h"
#include "actions/expression/Value.h"

namespace actions {

struct ActionParameter {
	SCP_string name;
	expression::ValueType type;
};

class ActionDefinition {
  public:
	ActionDefinition(SCP_string name, SCP_vector<ActionParameter> parameters);
	virtual ~ActionDefinition();

	const SCP_string& getName() const;
	const SCP_vector<actions::ActionParameter>& getParameters() const;

	virtual flagset<ProgramContextFlags> getRequiredContext() const = 0;

	virtual std::unique_ptr<Action> createAction(
		const SCP_unordered_map<SCP_string, expression::ActionExpression>& parameterExpressions) const = 0;

  private:
	SCP_string m_name;
	SCP_vector<ActionParameter> m_parameters;
};

} // namespace actions

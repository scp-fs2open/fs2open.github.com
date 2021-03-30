
#include "ActionDefinition.h"

namespace actions {

ActionDefinition::ActionDefinition(SCP_string name, SCP_vector<ActionParameter> parameters)
	: m_name(std::move(name)), m_parameters(std::move(parameters))
{
}
ActionDefinition::~ActionDefinition() = default;

const SCP_string& ActionDefinition::getName() const
{
	return m_name;
}
const SCP_vector<ActionParameter>& ActionDefinition::getParameters() const
{
	return m_parameters;
}

} // namespace actions

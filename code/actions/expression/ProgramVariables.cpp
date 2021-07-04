
#include "ProgramVariables.h"

namespace actions {
namespace expression {

ProgramVariablesDefinition& ProgramVariablesDefinition::addScope(const SCP_string& name)
{
	auto scope = std::unique_ptr<ProgramVariablesDefinition>(new ProgramVariablesDefinition());

	auto scopePtr = scope.get();

	m_scopes.insert(std::make_pair(name, std::move(scope)));

	return *scopePtr;
}
void ProgramVariablesDefinition::addMember(const SCP_string& name, ValueType type)
{
	m_memberTypes[name] = type;
}
ProgramVariablesDefinition* ProgramVariablesDefinition::getScope(const SCP_string& name) const
{
	const auto iter = m_scopes.find(name);

	if (iter == m_scopes.cend()) {
		return nullptr;
	}

	return iter->second.get();
}
ValueType ProgramVariablesDefinition::getMemberType(const SCP_string& name) const
{
	const auto iter = m_memberTypes.find(name);

	if (iter == m_memberTypes.cend()) {
		return ValueType::Invalid;
	}

	return iter->second;
}

Value ProgramVariables::getValue(std::initializer_list<const char*> memberPathParts) const
{
	return getValue<decltype(memberPathParts)>(memberPathParts);
}
void ProgramVariables::setValue(std::initializer_list<const char*> memberPathParts, Value value)
{
	setValue<decltype(memberPathParts)>(memberPathParts, std::move(value));
}
Value ProgramVariables::getValue(const SCP_string& fullMemberName) const
{
	const auto iter = m_values.find(fullMemberName);

	// This should have been validated earlier
	Assertion(iter != m_values.end(), "Could not find member %s in variables!", fullMemberName.c_str());

	return iter->second;
}

} // namespace expression
} // namespace actions

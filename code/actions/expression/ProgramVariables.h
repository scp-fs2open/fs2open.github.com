#pragma once

#include "Value.h"

#include "utils/join_string.h"

namespace actions {
namespace expression {

struct ProgramVariables {
  public:
	template <typename TContainer>
	static SCP_string getMemberName(const TContainer& memberPathParts)
	{
		return util::join_container(memberPathParts, ".");
	}

	template <typename TContainer>
	Value getValue(const TContainer& memberPathParts) const
	{
		const auto name = getMemberName(memberPathParts);

		return getValue(name);
	}

	Value getValue(std::initializer_list<const char*> memberPathParts) const;

	Value getValue(const SCP_string& fullMemberName) const;

	template <typename TContainer>
	void setValue(const TContainer& memberPathParts, Value value)
	{
		const auto name = getMemberName(memberPathParts);

		m_values[name] = std::move(value);
	}

	void setValue(std::initializer_list<const char*> memberPathParts, Value value);

  private:
	SCP_unordered_map<SCP_string, Value> m_values;
};

class ProgramVariablesDefinition {
  public:
	ProgramVariablesDefinition& addScope(const SCP_string& name);

	void addMember(const SCP_string& name, ValueType type);

	ProgramVariablesDefinition* getScope(const SCP_string& name) const;

	ValueType getMemberType(const SCP_string& name) const;

  private:
	SCP_unordered_map<SCP_string, std::unique_ptr<ProgramVariablesDefinition>> m_scopes;

	SCP_unordered_map<SCP_string, ValueType> m_memberTypes;
};

} // namespace expression
} // namespace actions

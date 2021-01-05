#pragma once

#include "Value.h"

namespace actions {
namespace expression {

struct FunctionDefinition {
	ValueType returnType;

	SCP_vector<ValueType> parameterTypes;
};

class TypeDefinition {
  public:
	TypeDefinition(SCP_string name, SCP_unordered_map<SCP_string, SCP_vector<FunctionDefinition>> operators);

	const SCP_string& getName() const;
	const SCP_unordered_map<SCP_string, SCP_vector<FunctionDefinition>>& operators() const;

	static const TypeDefinition& forValueType(ValueType type);

	// This type is supposed to be global so it may not be copied
	TypeDefinition(const TypeDefinition&) = delete;
	TypeDefinition& operator=(const TypeDefinition&) = delete;

	TypeDefinition(TypeDefinition&&) = delete;
	TypeDefinition& operator=(TypeDefinition&&) = delete;

  private:
	SCP_string m_name;

	SCP_unordered_map<SCP_string, SCP_vector<FunctionDefinition>> m_operators;

	static TypeDefinition s_integer;
	static TypeDefinition s_float;
	static TypeDefinition s_vector;
	static TypeDefinition s_identifier;
};

} // namespace expression
} // namespace actions

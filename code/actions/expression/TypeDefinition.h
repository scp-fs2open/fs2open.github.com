#pragma once

#include "Value.h"

namespace actions {
namespace expression {

class TypeDefinition {
  public:
	TypeDefinition(SCP_string name, SCP_vector<actions::expression::ValueType> allowedImplicitConversion);

	const SCP_string& getName() const;

	const SCP_vector<actions::expression::ValueType>& getAllowedImplicitConversions() const;

	static const TypeDefinition& forValueType(ValueType type);

	// This type is supposed to be global so it may not be copied
	TypeDefinition(const TypeDefinition&) = delete;
	TypeDefinition& operator=(const TypeDefinition&) = delete;

	TypeDefinition(TypeDefinition&&) = delete;
	TypeDefinition& operator=(TypeDefinition&&) = delete;

  private:
	SCP_string m_name;

	// The types this type is allowed to implicitly convert to
	SCP_vector<ValueType> m_allowedImplicitConversions;

	static TypeDefinition s_integer;
	static TypeDefinition s_float;
	static TypeDefinition s_vector;
	static TypeDefinition s_identifier;
};

bool checkTypeWithImplicitConversion(ValueType currentType, ValueType expectedType);

} // namespace expression
} // namespace actions

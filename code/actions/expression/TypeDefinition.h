#pragma once

#include "Value.h"

namespace actions {
namespace expression {

class TypeDefinition {
  public:
	TypeDefinition(SCP_string name);

	const SCP_string& getName() const;

	static const TypeDefinition& forValueType(ValueType type);

	// This type is supposed to be global so it may not be copied
	TypeDefinition(const TypeDefinition&) = delete;
	TypeDefinition& operator=(const TypeDefinition&) = delete;

	TypeDefinition(TypeDefinition&&) = delete;
	TypeDefinition& operator=(TypeDefinition&&) = delete;

  private:
	SCP_string m_name;

	static TypeDefinition s_integer;
	static TypeDefinition s_float;
	static TypeDefinition s_vector;
	static TypeDefinition s_identifier;
};

} // namespace expression
} // namespace actions


#include "TypeDefinition.h"

namespace actions {
namespace expression {

TypeDefinition TypeDefinition::s_integer("Integer");
// No implicit conversions here
TypeDefinition TypeDefinition::s_float("Float");
TypeDefinition TypeDefinition::s_vector("Vector");
// Identifiers are a technicality needed for making non-math values work but they have no further use other than their
// string content
TypeDefinition TypeDefinition::s_identifier("Identifier");

TypeDefinition::TypeDefinition(SCP_string name) : m_name(std::move(name)) {}

const SCP_string& TypeDefinition::getName() const
{
	return m_name;
}
const TypeDefinition& TypeDefinition::forValueType(ValueType type)
{
	switch (type) {
	case ValueType::Integer:
		return s_integer;
	case ValueType::Float:
		return s_float;
	case ValueType::Vector:
		return s_vector;
	case ValueType::Identifier:
		return s_identifier;
	default:
		UNREACHABLE("Invalid value type!");
		return s_integer; // Make compiler happy
	}
}

} // namespace expression
} // namespace actions

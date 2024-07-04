
#include "TypeDefinition.h"

namespace actions {
namespace expression {

// Integers can be safely converted to floats (for the most part)
TypeDefinition TypeDefinition::s_integer("Integer", {ValueType::Float});
// No implicit conversions here
TypeDefinition TypeDefinition::s_float("Float", {});
TypeDefinition TypeDefinition::s_vector("Vector", {});
// Identifiers are a technicality needed for making non-math values work but they have no further use other than their
// string content
TypeDefinition TypeDefinition::s_identifier("Identifier", {});

TypeDefinition::TypeDefinition(SCP_string name, SCP_vector<actions::expression::ValueType> allowedImplicitConversion)
	: m_name(std::move(name)), m_allowedImplicitConversions(std::move(allowedImplicitConversion))
{
}

const SCP_string& TypeDefinition::getName() const
{
	return m_name;
}
const SCP_vector<actions::expression::ValueType>& TypeDefinition::getAllowedImplicitConversions() const
{
	return m_allowedImplicitConversions;
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
	case ValueType::String:
		return s_identifier;
	default:
		UNREACHABLE("Invalid value type!");
		return s_integer; // Make compiler happy
	}
}

bool checkTypeWithImplicitConversion(ValueType currentType, ValueType expectedType)
{
	if (currentType == expectedType) {
		// No need to consider implicit conversions;
		return true;
	}

	const auto& currentTypeDef = TypeDefinition::forValueType(currentType);
	const auto& allowedConversions = currentTypeDef.getAllowedImplicitConversions();

	// For now, no transitiv conversions are allowed
	return std::find(allowedConversions.cbegin(), allowedConversions.cend(), expectedType) != allowedConversions.cend();
}
} // namespace expression
} // namespace actions

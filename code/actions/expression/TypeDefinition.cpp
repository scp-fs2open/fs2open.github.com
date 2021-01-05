
#include "TypeDefinition.h"

namespace actions {
namespace expression {

TypeDefinition TypeDefinition::s_integer("Integer",
	{
		{"+",
			{
				{ValueType::Integer, {ValueType::Integer}},
			}},
		{"-",
			{
				{ValueType::Integer, {ValueType::Integer}},
			}},
	});
// No implicit conversions here
TypeDefinition TypeDefinition::s_float("Float",
	{
		{"+",
			{
				{ValueType::Float, {ValueType::Float}},
			}},
		{"-",
			{
				{ValueType::Float, {ValueType::Float}},
			}},
	});
TypeDefinition TypeDefinition::s_vector("Vector",
	{
		{"+",
			{
				{ValueType::Vector, {ValueType::Vector}},
			}},
		{"-",
			{{
				ValueType::Vector,
				{ValueType::Vector},
			}}},
	});
// Identifiers are a technicality needed for making non-math values work but they have no further use other than their
// string content
TypeDefinition TypeDefinition::s_identifier("Identifier", {});

TypeDefinition::TypeDefinition(SCP_string name, SCP_unordered_map<SCP_string, SCP_vector<FunctionDefinition>> operators)
	: m_name(std::move(name)), m_operators(std::move(operators))
{
}

const SCP_string& TypeDefinition::getName() const
{
	return m_name;
}
const SCP_unordered_map<SCP_string, SCP_vector<FunctionDefinition>>& TypeDefinition::operators() const
{
	return m_operators;
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

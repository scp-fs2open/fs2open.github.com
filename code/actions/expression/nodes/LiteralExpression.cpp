
#include "LiteralExpression.h"

namespace actions {
namespace expression {
namespace nodes {

LiteralExpression::LiteralExpression(antlr4::Token* token, Value literalValue)
	: AbstractExpression(token), m_literalValue(std::move(literalValue))
{
}
LiteralExpression::~LiteralExpression() = default;

Value LiteralExpression::execute(const ProgramVariables& /*variables*/) const
{
	return m_literalValue;
}
bool LiteralExpression::validate(antlr4::Parser* /*parser*/, const ParseContext& /*context*/)
{
	// Literals are always valid
	return true;
}
ValueType LiteralExpression::determineReturnType() const
{
	return m_literalValue.getType();
}

} // namespace nodes
} // namespace expression
} // namespace actions

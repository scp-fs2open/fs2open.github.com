
#include "AbstractExpression.h"

namespace actions {
namespace expression {
namespace nodes {

AbstractExpression::AbstractExpression(antlr4::Token* token) : m_token(token) {}
AbstractExpression::~AbstractExpression() = default;

antlr4::Token* AbstractExpression::getToken() const
{
	return m_token;
}
ValueType AbstractExpression::getExpressionType() const
{
	// Since expressions are immutable, caching this will be more efficient
	if (m_cachedType == ValueType::Invalid) {
		m_cachedType = determineReturnType();
	}
	return m_cachedType;
}
void AbstractExpression::validationDone()
{
	// This pointer will become invalid after this so reset it now to avoid issues
	m_token = nullptr;
}

} // namespace nodes
} // namespace expression
} // namespace actions

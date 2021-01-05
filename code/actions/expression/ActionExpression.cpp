
#include "ActionExpression.h"

#include "AbstractExpression.h"
#include "ExpressionParser.h"
#include "TypeDefinition.h"

namespace actions {
namespace expression {

std::shared_ptr<AbstractExpression> BaseActionExpression::parseExpression(const SCP_string& expressionText)
{
	ExpressionParser parser(expressionText);

	auto expression = parser.parse();

	if (!expression) {
		error_display(0, "Failed to parse action expression:\n%s", parser.getErrorText().c_str());
		return nullptr;
	}

	return expression;
}
SCP_string BaseActionExpression::getTypeName(ValueType type)
{
	const auto& typeDef = TypeDefinition::forValueType(type);
	return typeDef.getName();
}

} // namespace expression
} // namespace actions

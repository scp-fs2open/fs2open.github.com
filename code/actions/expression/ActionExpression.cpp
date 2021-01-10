
#include "ActionExpression.h"

#include "ExpressionParser.h"
#include "TypeDefinition.h"

#include "actions/expression/nodes/AbstractExpression.h"

namespace actions {
namespace expression {
namespace {

SCP_string getTypeName(ValueType type)
{
	const auto& typeDef = TypeDefinition::forValueType(type);
	return typeDef.getName();
}

} // namespace

ActionExpression ActionExpression::parseFromTable(ValueType expectedReturnType, const ParseContext& context)
{
	SCP_string expressionText;

	stuff_string(expressionText, F_NAME);

	ExpressionParser parser(expressionText);

	auto expression = parser.parse(context);

	if (!expression) {
		error_display(0, "Failed to parse action expression:\n%s", parser.getErrorText().c_str());
		return ActionExpression();
	}

	auto type = expression->getExpressionType();

	// Check if the type this evaluates to matches what we expect
	if (!checkTypeWithImplicitConversion(type, expectedReturnType)) {
		error_display(0,
			"Expression evaluates to type <%s> but <%s> was expected!",
			getTypeName(type).c_str(),
			getTypeName(expectedReturnType).c_str());
		return ActionExpression();
	}

	// Everything is valid
	return ActionExpression(expression);
}

Value ActionExpression::execute(const ProgramVariables& variables) const
{
	// Do not crash on invalid expressions
	if (!isValid()) {
		return Value();
	}

	return m_expression->execute(variables);
}
bool ActionExpression::isValid() const
{
	return m_expression != nullptr;
}

} // namespace expression
} // namespace actions

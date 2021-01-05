
#include "OperatorCallExpression.h"

#include "TypeDefinition.h"

namespace actions {
namespace expression {

OperatorCallExpression::OperatorCallExpression(antlr4::Token* token,
	SCP_string op,
	std::shared_ptr<AbstractExpression> leftExpression,
	std::shared_ptr<AbstractExpression> rightExpression)
	: AbstractExpression(token), m_operator(std::move(op)), m_leftExpression(std::move(leftExpression)),
	  m_rightExpression(std::move(rightExpression))
{
}
OperatorCallExpression::~OperatorCallExpression() = default;

Value OperatorCallExpression::execute()
{
	Assertion(m_operator.size() == 1, "Invalid operator '%s' encountered", m_operator.c_str());

	const auto leftType = m_leftExpression->getExpressionType();
	const auto rightType = m_rightExpression->getExpressionType();

	Assertion(leftType == rightType, "Currently, only same type expressions are supported.");

	const auto leftValue = m_leftExpression->execute();
	const auto rightValue = m_rightExpression->execute();

	// For actual execution we can be a bit quicker and just cover the cases we know
	switch (m_operator[0]) {
	case '+':
		switch (leftType) {
		case ValueType::Integer:
			return Value(leftValue.getInteger() + rightValue.getInteger());
		case ValueType::Float:
			return Value(leftValue.getFloat() + rightValue.getFloat());
		default:
			UNREACHABLE("Unsupported types.");
			return Value();
		}
	case '-':
		switch (leftType) {
		case ValueType::Integer:
			return Value(leftValue.getInteger() - rightValue.getInteger());
		case ValueType::Float:
			return Value(leftValue.getFloat() - rightValue.getFloat());
		default:
			UNREACHABLE("Unsupported types.");
			return Value();
		}
	default:
		UNREACHABLE("Unhandled operator '%s' encountered.", m_operator.c_str());
		return Value();
	}
}
bool OperatorCallExpression::validate(antlr4::Parser* parser)
{
	bool valid = true;

	valid &= m_leftExpression->validate(parser);
	valid &= m_rightExpression->validate(parser);

	if (!valid) {
		// Exit early if there were errors since the values below are meaningless otherwise
		return false;
	}

	const auto leftType = m_leftExpression->getExpressionType();
	const auto rightType = m_rightExpression->getExpressionType();

	const auto& leftTypedef = TypeDefinition::forValueType(leftType);
	const auto& rightTypedef = TypeDefinition::forValueType(rightType);

	const auto& operators = leftTypedef.operators();

	const auto opDefs = operators.find(m_operator);

	if (opDefs == operators.end()) {
		parser->notifyErrorListeners(m_token,
			"Could not find a definition for the operator '" + m_operator + "' in type <" + leftTypedef.getName() + ">",
			nullptr);
		return false;
	}

	// Now check if there is an overload which matches
	for (const auto& overload : opDefs->second) {
		Assertion(overload.parameterTypes.size() == 1, "Invalid size for operator function definition.");

		if (overload.parameterTypes.front() == rightType) {
			// found our overload! We are good to go.
			return true;
		}
	}

	parser->notifyErrorListeners(m_token,
		"Could not find a definition for the operator <" + leftTypedef.getName() + "> '" + m_operator + "' <" +
			rightTypedef.getName() + ">",
		nullptr);
	return false;
}
ValueType OperatorCallExpression::determineReturnType() const
{
	const auto leftType = m_leftExpression->getExpressionType();
	const auto rightType = m_rightExpression->getExpressionType();

	const auto& leftTypedef = TypeDefinition::forValueType(leftType);

	const auto& operators = leftTypedef.operators();

	const auto opDefs = operators.find(m_operator);

	Assertion(opDefs != operators.end(),
		"Could not find a definition for operator '%s'. This should have been caught earlier.",
		m_operator.c_str());

	// Now check if there is an overload which matches
	for (const auto& overload : opDefs->second) {
		Assertion(overload.parameterTypes.size() == 1, "Invalid size for operator function definition.");

		if (overload.parameterTypes.front() == rightType) {
			// found our overload!
			return overload.returnType;
		}
	}

	// No valid overload found. Shouldn't happen...
	return ValueType::Invalid;
}
void OperatorCallExpression::validationDone()
{
	AbstractExpression::validationDone();

	m_leftExpression->validationDone();
	m_rightExpression->validationDone();
}

} // namespace expression
} // namespace actions


#include "RandomRangeExpression.h"

#include "actions/expression/TypeDefinition.h"
#include "utils/RandomRange.h"

namespace actions {
namespace expression {
namespace nodes {

namespace {

template <typename T>
Value executeHelper(const Value& leftVal, const Value& rightVal)
{
	auto leftContentVal = leftVal.get<T>();
	auto rightContentVal = rightVal.get<T>();

	if (leftContentVal == rightContentVal) {
		// no need to do random stuff here
		return Value(rightContentVal);
	}

	if (rightContentVal < leftContentVal) {
		// We can fix this issue
		std::swap(leftContentVal, rightContentVal);
	}

	// Might be a bit inefficient not to cache this somehow but the bounds can be dynamic
	return Value(util::UniformRange<T>(leftContentVal, rightContentVal).next());
}
} // namespace

RandomRangeExpression::RandomRangeExpression(antlr4::Token* token,
	std::shared_ptr<AbstractExpression> leftExpression,
	std::shared_ptr<AbstractExpression> rightExpression)
	: AbstractExpression(token), m_leftExpression(std::move(leftExpression)),
	  m_rightExpression(std::move(rightExpression))
{
}
RandomRangeExpression::~RandomRangeExpression() = default;

Value RandomRangeExpression::execute(const ProgramVariables& variables) const
{
	const auto leftVal = m_leftExpression->execute(variables);
	const auto rightVal = m_rightExpression->execute(variables);

	if (leftVal.getType() == ValueType::Integer) {
		return executeHelper<int>(leftVal, rightVal);
	} else {
		return executeHelper<float>(leftVal, rightVal);
	}
}
bool RandomRangeExpression::validate(antlr4::Parser* parser, const ParseContext& context)
{
	bool valid = true;

	valid &= m_leftExpression->validate(parser, context);
	valid &= m_rightExpression->validate(parser, context);

	if (!valid) {
		// Exit early if there were errors since the values below are meaningless otherwise
		return false;
	}

	const auto leftType = m_leftExpression->getExpressionType();
	const auto rightType = m_rightExpression->getExpressionType();

	const auto& leftTypedef = TypeDefinition::forValueType(leftType);
	const auto& rightTypedef = TypeDefinition::forValueType(rightType);

	// Both values must be the same
	if (leftType != rightType) {
		parser->notifyErrorListeners(m_rightExpression->getToken(),
			"Inconsistent types for random range. Must be the same but got <" + leftTypedef.getName() + "> and <" +
				rightTypedef.getName() + ">.",
			nullptr);
		return false;
	}

	// And we only support floats or integers
	if (leftType != ValueType::Float && leftType != ValueType::Integer) {
		parser->notifyErrorListeners(m_leftExpression->getToken(),
			"Invalid parameter type for random range. Must be <Float> or <Integer> but got <" + leftTypedef.getName() +
				">.",
			nullptr);
		return false;
	}

	// Everything is fine.
	return true;
}
ValueType RandomRangeExpression::determineReturnType() const
{
	// Return type is the same as the contained return types
	return m_leftExpression->getExpressionType();
}
void RandomRangeExpression::validationDone()
{
	AbstractExpression::validationDone();

	m_leftExpression->validationDone();
	m_rightExpression->validationDone();
}

} // namespace nodes
} // namespace expression
} // namespace actions

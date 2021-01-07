
#include "VectorConstructorExpression.h"

#include "TypeDefinition.h"

namespace actions {
namespace expression {

VectorConstructorExpression::VectorConstructorExpression(antlr4::Token* token,
	std::shared_ptr<AbstractExpression> xExpression,
	std::shared_ptr<AbstractExpression> yExpression,
	std::shared_ptr<AbstractExpression> zExpression)
	: AbstractExpression(token), m_xExpression(std::move(xExpression)), m_yExpression(std::move(yExpression)),
	  m_zExpression(std::move(zExpression))
{
}
VectorConstructorExpression::~VectorConstructorExpression() = default;

Value VectorConstructorExpression::execute() const
{
	auto xValue = m_xExpression->execute();
	auto yValue = m_yExpression->execute();
	auto zValue = m_zExpression->execute();

	vec3d vecVal;

	vecVal.xyz.x = xValue.getFloat();
	vecVal.xyz.y = yValue.getFloat();
	vecVal.xyz.z = zValue.getFloat();

	return Value(vecVal);
}
bool VectorConstructorExpression::validate(antlr4::Parser* parser)
{
	bool valid = true;

	valid &= m_xExpression->validate(parser);
	valid &= m_yExpression->validate(parser);
	valid &= m_zExpression->validate(parser);

	if (!valid) {
		// Exit early if there were errors since the values below are meaningless otherwise
		return false;
	}

	if (!checkTypeWithImplicitConversion(m_xExpression->getExpressionType(), ValueType::Float)) {
		const auto& typeDef = TypeDefinition::forValueType(m_xExpression->getExpressionType());
		parser->notifyErrorListeners(m_xExpression->getToken(),
			"Invalid value type for vector constructor. Expected <Float>, got <" + typeDef.getName() + ">",
			nullptr);
		valid = false;
	}
	if (!checkTypeWithImplicitConversion(m_yExpression->getExpressionType(), ValueType::Float)) {
		const auto& typeDef = TypeDefinition::forValueType(m_yExpression->getExpressionType());
		parser->notifyErrorListeners(m_yExpression->getToken(),
			"Invalid value type for vector constructor. Expected <Float>, got <" + typeDef.getName() + ">",
			nullptr);
		valid = false;
	}
	if (!checkTypeWithImplicitConversion(m_zExpression->getExpressionType(), ValueType::Float)) {
		const auto& typeDef = TypeDefinition::forValueType(m_zExpression->getExpressionType());
		parser->notifyErrorListeners(m_zExpression->getToken(),
			"Invalid value type for vector constructor. Expected <Float>, got <" + typeDef.getName() + ">",
			nullptr);
		valid = false;
	}

	return valid;
}
ValueType VectorConstructorExpression::determineReturnType() const
{
	// No need to look at the expressions here since we will always be a vector
	return ValueType::Vector;
}
void VectorConstructorExpression::validationDone()
{
	AbstractExpression::validationDone();

	m_xExpression->validationDone();
	m_yExpression->validationDone();
	m_zExpression->validationDone();
}

} // namespace expression
} // namespace actions


#include "FunctionCallExpression.h"

#include "actions/expression/TypeDefinition.h"

namespace actions {
namespace expression {
namespace nodes {

FunctionCallExpression::FunctionCallExpression(antlr4::Token* token,
	bool isOperator,
	SCP_string functionName,
	SCP_vector<std::shared_ptr<AbstractExpression>> parameterExpressions)
	: AbstractExpression(token), m_isOperator(isOperator), m_functionName(std::move(functionName)),
	  m_parameterExpressions(std::move(parameterExpressions))
{
}
FunctionCallExpression::~FunctionCallExpression() = default;

Value FunctionCallExpression::execute(const ProgramVariables& variables) const
{
	SCP_vector<Value> parameterValues;
	parameterValues.reserve(m_parameterExpressions.size());

	std::transform(m_parameterExpressions.cbegin(),
		m_parameterExpressions.cend(),
		std::back_inserter(parameterValues),
		[&variables](const std::shared_ptr<AbstractExpression>& expression) { return expression->execute(variables); });

	return m_functionDef->implementation(parameterValues);
}
bool FunctionCallExpression::validate(antlr4::Parser* parser, const ParseContext& context)
{
	bool valid = true;

	for (const auto& paramExpression : m_parameterExpressions) {
		valid &= paramExpression->validate(parser, context);
	}

	if (!valid) {
		// Exit early if there were errors since the values below are meaningless otherwise
		return false;
	}

	Assertion(m_isOperator, "Non-operator functions are not supported yet!");

	SCP_vector<ValueType> parameterTypes;

	std::transform(m_parameterExpressions.cbegin(),
		m_parameterExpressions.cend(),
		std::back_inserter(parameterTypes),
		[](const std::shared_ptr<AbstractExpression>& expression) { return expression->getExpressionType(); });

	const auto functionDef = FunctionManager::instance().findOperator(m_functionName, parameterTypes);

	if (functionDef == nullptr) {
		SCP_stringstream paramListStream;
		if (!parameterTypes.empty()) {
			paramListStream << "<" << TypeDefinition::forValueType(parameterTypes.front()).getName() << ">";

			for (auto iter = parameterTypes.cbegin() + 1; iter != parameterTypes.cend(); ++iter) {
				paramListStream << ", <" << TypeDefinition::forValueType(*iter).getName() << ">";
			}
		}

		parser->notifyErrorListeners(m_token,
			SCP_string("Could not find a definition for the ") + (m_isOperator ? "operator" : "function") + " '" +
				m_functionName + "' with parameters (" + paramListStream.str() + ")",
			nullptr);
		return false;
	}

	m_functionDef = functionDef;
	return true;
}
ValueType FunctionCallExpression::determineReturnType() const
{
	return m_functionDef->returnType;
}
void FunctionCallExpression::validationDone()
{
	AbstractExpression::validationDone();

	// We no longer need this so we can save some memory
	m_functionName.clear();
	m_functionName.shrink_to_fit();

	for (const auto& paramExpression : m_parameterExpressions) {
		paramExpression->validationDone();
	}
}

} // namespace nodes
} // namespace expression
} // namespace actions


#include "VariableReferenceExpression.h"

namespace actions {
namespace expression {
namespace nodes {

VariableReferenceExpression::VariableReferenceExpression(SCP_vector<SCP_string> references,
	SCP_vector<antlr4::Token*> referenceTokens)
	: AbstractExpression(referenceTokens.back()), m_references(std::move(references)),
	  m_referenceTokens(std::move(referenceTokens))
{
}
VariableReferenceExpression::~VariableReferenceExpression() = default;

Value VariableReferenceExpression::execute(const ProgramVariables& variables) const
{
	return variables.getValue(m_fullVariablePath);
}
bool VariableReferenceExpression::validate(antlr4::Parser* parser, const ParseContext& context)
{
	// Start out in the global scope
	auto currentScope = &context.variables;
	SCP_string currentScopeName("<global>");

	for (size_t i = 0; i < m_references.size() - 1; ++i) {
		const auto nextScope = currentScope->getScope(m_references[i]);

		if (nextScope == nullptr) {
			parser->notifyErrorListeners(m_referenceTokens[i],
				"Could not find scope " + m_references[i] + " in scope " + currentScopeName + ".",
				nullptr);
			return false;
		}

		currentScope = nextScope;
		currentScopeName = m_references[i];
	}

	auto variableType = currentScope->getMemberType(m_references.back());

	if (variableType == ValueType::Invalid) {
		parser->notifyErrorListeners(m_referenceTokens.back(),
			"Could not find variable " + m_references.back() + " in scope " + currentScopeName + ".",
			nullptr);
		return false;
	}

	// Cache this so we don't duplicate all this code in determineReturnType()
	m_variableType = variableType;
	m_fullVariablePath = ProgramVariables::getMemberName(m_references);
	return true;
}
ValueType VariableReferenceExpression::determineReturnType() const
{
	return m_variableType;
}
void VariableReferenceExpression::validationDone()
{
	AbstractExpression::validationDone();

	// Clear these since they are not necessary anymore
	m_references.clear();
	m_referenceTokens.clear();
}

} // namespace nodes
} // namespace expression
} // namespace actions

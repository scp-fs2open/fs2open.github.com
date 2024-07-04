#pragma once

#include "AbstractExpression.h"

namespace actions {
namespace expression {
namespace nodes {

class VariableReferenceExpression : public AbstractExpression {
  public:
	VariableReferenceExpression(const SCP_vector<SCP_string> references,
		SCP_vector<antlr4::Token*> referenceTokens);
	~VariableReferenceExpression() override;

	Value execute(const ProgramVariables& variables) const override;
	bool validate(antlr4::Parser* parser, const ParseContext& context) override;
	void validationDone() override;

  protected:
	ValueType determineReturnType() const override;

  private:
	ValueType m_variableType = ValueType::Invalid;
	SCP_string m_fullVariablePath;

	SCP_vector<SCP_string> m_references;
	SCP_vector<antlr4::Token*> m_referenceTokens;
};

} // namespace nodes
} // namespace expression
} // namespace actions

#pragma once

#include "AbstractExpression.h"

#include "actions/expression/FunctionManager.h"
#include "actions/expression/Value.h"

namespace actions {
namespace expression {
namespace nodes {

class FunctionCallExpression : public AbstractExpression {
  public:
	FunctionCallExpression(antlr4::Token* token,
		bool isOperator,
		SCP_string functionName,
		SCP_vector<std::shared_ptr<AbstractExpression>> parameterExpressions);
	~FunctionCallExpression() override;

	Value execute(const ProgramVariables& variables) const override;
	bool validate(antlr4::Parser* parser, const ParseContext& context) override;
	void validationDone() override;

  protected:
	ValueType determineReturnType() const override;

  private:
	bool m_isOperator;
	SCP_string m_functionName;

	const FunctionDefinition* m_functionDef = nullptr;

	SCP_vector<std::shared_ptr<AbstractExpression>> m_parameterExpressions;
};

} // namespace nodes
} // namespace expression
} // namespace actions

#pragma once

#include "AbstractExpression.h"
#include "Value.h"

namespace actions {
namespace expression {

class OperatorCallExpression : public AbstractExpression {
  public:
	OperatorCallExpression(antlr4::Token* token,
		SCP_string op,
		std::shared_ptr<AbstractExpression> leftExpression,
		std::shared_ptr<AbstractExpression> rightExpression);
	~OperatorCallExpression() override;

	Value execute() override;
	bool validate(antlr4::Parser* parser) override;
	void validationDone() override;

  protected:
	ValueType determineReturnType() const override;

  private:
	SCP_string m_operator;

	std::shared_ptr<AbstractExpression> m_leftExpression;
	std::shared_ptr<AbstractExpression> m_rightExpression;
};

} // namespace expression
} // namespace actions

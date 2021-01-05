#pragma once

#include "AbstractExpression.h"

namespace actions {
namespace expression {

class RandomRangeExpression : public AbstractExpression {
  public:
	RandomRangeExpression(antlr4::Token* token,
		std::shared_ptr<AbstractExpression> leftExpression,
		std::shared_ptr<AbstractExpression> rightExpression);
	~RandomRangeExpression() override;

	Value execute() override;
	bool validate(antlr4::Parser* parser) override;
	void validationDone() override;

  protected:
	ValueType determineReturnType() const override;

  private:
	std::shared_ptr<AbstractExpression> m_leftExpression;
	std::shared_ptr<AbstractExpression> m_rightExpression;
};

} // namespace expression
} // namespace actions

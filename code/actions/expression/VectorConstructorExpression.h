#pragma once

#include "AbstractExpression.h"
#include "Value.h"

namespace actions {
namespace expression {

class VectorConstructorExpression : public AbstractExpression {
  public:
	VectorConstructorExpression(antlr4::Token* token,
		std::shared_ptr<AbstractExpression> xExpression,
		std::shared_ptr<AbstractExpression> yExpression,
		std::shared_ptr<AbstractExpression> zExpression);
	~VectorConstructorExpression() override;

	Value execute() override;
	bool validate(antlr4::Parser* parser) override;
	void validationDone() override;

  protected:
	ValueType determineReturnType() const override;

  private:
	std::shared_ptr<AbstractExpression> m_xExpression;
	std::shared_ptr<AbstractExpression> m_yExpression;
	std::shared_ptr<AbstractExpression> m_zExpression;
};

} // namespace expression
} // namespace actions

#pragma once

#include "AbstractExpression.h"
#include "Value.h"

namespace actions {
namespace expression {

class LiteralExpression : public AbstractExpression {
  public:
	LiteralExpression(antlr4::Token* token, Value literalValue);
	~LiteralExpression() override;

	Value execute() override;
	bool validate(antlr4::Parser* parser) override;

  protected:
	ValueType determineReturnType() const override;

  private:
	Value m_literalValue;
};

} // namespace expression
} // namespace actions

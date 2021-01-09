#pragma once

#include "AbstractExpression.h"

#include "actions/expression/Value.h"

namespace actions {
namespace expression {
namespace nodes {

class LiteralExpression : public AbstractExpression {
  public:
	LiteralExpression(antlr4::Token* token, Value literalValue);
	~LiteralExpression() override;

	Value execute() const override;
	bool validate(antlr4::Parser* parser) override;

  protected:
	ValueType determineReturnType() const override;

  private:
	Value m_literalValue;
};

} // namespace nodes
} // namespace expression
} // namespace actions

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

	Value execute(const ProgramVariables& variables) const override;
	bool validate(antlr4::Parser* parser, const ParseContext& context) override;

  protected:
	ValueType determineReturnType() const override;

  private:
	Value m_literalValue;
};

} // namespace nodes
} // namespace expression
} // namespace actions

#pragma once

#include "AbstractExpression.h"

#include "actions/expression/Value.h"

namespace actions {
namespace expression {
namespace nodes {

class VectorConstructorExpression : public AbstractExpression {
  public:
	VectorConstructorExpression(antlr4::Token* token,
		std::shared_ptr<AbstractExpression> xExpression,
		std::shared_ptr<AbstractExpression> yExpression,
		std::shared_ptr<AbstractExpression> zExpression);
	~VectorConstructorExpression() override;

	Value execute(const ProgramVariables& variables) const override;
	bool validate(antlr4::Parser* parser, const ParseContext& context) override;
	void validationDone() override;

  protected:
	ValueType determineReturnType() const override;

  private:
	std::shared_ptr<AbstractExpression> m_xExpression;
	std::shared_ptr<AbstractExpression> m_yExpression;
	std::shared_ptr<AbstractExpression> m_zExpression;
};

} // namespace nodes
} // namespace expression
} // namespace actions

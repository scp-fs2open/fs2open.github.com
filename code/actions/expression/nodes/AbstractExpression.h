#pragma once

#include <Parser.h>
#include <Token.h>

#include "actions/expression/ParseContext.h"
#include "actions/expression/Value.h"

namespace actions {
namespace expression {
namespace nodes {

class AbstractExpression {
  public:
	AbstractExpression(antlr4::Token* token);
	virtual ~AbstractExpression();

	virtual Value execute(const ProgramVariables& variables) const = 0;

	virtual bool validate(antlr4::Parser* parser, const ParseContext& context) = 0;

	virtual void validationDone();

	ValueType getExpressionType() const;

	antlr4::Token* getToken() const;

  protected:
	virtual ValueType determineReturnType() const = 0;

	antlr4::Token* m_token = nullptr;

	mutable ValueType m_cachedType = ValueType::Invalid;
};

} // namespace nodes
} // namespace expression
} // namespace actions

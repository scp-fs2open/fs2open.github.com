#pragma once

#include "Value.h"

#include <Token.h>
#include <Parser.h>

namespace actions {
namespace expression {

class AbstractExpression {
  public:
	AbstractExpression(antlr4::Token* token);
	virtual ~AbstractExpression();

	virtual Value execute() = 0;

	virtual bool validate(antlr4::Parser* parser) = 0;

	virtual void validationDone();

	ValueType getExpressionType() const;

	antlr4::Token* getToken() const;

  protected:
	virtual ValueType determineReturnType() const = 0;

	antlr4::Token* m_token = nullptr;

	mutable ValueType m_cachedType = ValueType::Invalid;
};

} // namespace expression
} // namespace actions

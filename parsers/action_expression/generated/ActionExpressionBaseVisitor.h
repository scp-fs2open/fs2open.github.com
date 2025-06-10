
// Generated from ActionExpression.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ActionExpressionVisitor.h"


/**
 * This class provides an empty implementation of ActionExpressionVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ActionExpressionBaseVisitor : public ActionExpressionVisitor {
public:

  virtual std::any visitExpression_main(ActionExpressionParser::Expression_mainContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(ActionExpressionParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParenthesis_expression(ActionExpressionParser::Parenthesis_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_expression(ActionExpressionParser::Value_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral_expression(ActionExpressionParser::Literal_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariable_reference_expression(ActionExpressionParser::Variable_reference_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRandom_range_expression(ActionExpressionParser::Random_range_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVec3d_constructor(ActionExpressionParser::Vec3d_constructorContext *ctx) override {
    return visitChildren(ctx);
  }


};


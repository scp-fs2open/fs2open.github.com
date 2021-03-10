
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/action_expression/ActionExpression.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "ActionExpressionVisitor.h"


/**
 * This class provides an empty implementation of ActionExpressionVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ActionExpressionBaseVisitor : public ActionExpressionVisitor {
public:

  virtual antlrcpp::Any visitExpression_main(ActionExpressionParser::Expression_mainContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpression(ActionExpressionParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParenthesis_expression(ActionExpressionParser::Parenthesis_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue_expression(ActionExpressionParser::Value_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLiteral_expression(ActionExpressionParser::Literal_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariable_reference_expression(ActionExpressionParser::Variable_reference_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRandom_range_expression(ActionExpressionParser::Random_range_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVec3d_constructor(ActionExpressionParser::Vec3d_constructorContext *ctx) override {
    return visitChildren(ctx);
  }


};


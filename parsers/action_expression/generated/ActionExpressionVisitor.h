
// Generated from ActionExpression.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ActionExpressionParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by ActionExpressionParser.
 */
class  ActionExpressionVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by ActionExpressionParser.
   */
    virtual std::any visitExpression_main(ActionExpressionParser::Expression_mainContext *context) = 0;

    virtual std::any visitExpression(ActionExpressionParser::ExpressionContext *context) = 0;

    virtual std::any visitParenthesis_expression(ActionExpressionParser::Parenthesis_expressionContext *context) = 0;

    virtual std::any visitValue_expression(ActionExpressionParser::Value_expressionContext *context) = 0;

    virtual std::any visitLiteral_expression(ActionExpressionParser::Literal_expressionContext *context) = 0;

    virtual std::any visitVariable_reference_expression(ActionExpressionParser::Variable_reference_expressionContext *context) = 0;

    virtual std::any visitRandom_range_expression(ActionExpressionParser::Random_range_expressionContext *context) = 0;

    virtual std::any visitVec3d_constructor(ActionExpressionParser::Vec3d_constructorContext *context) = 0;


};


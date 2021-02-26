
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/action_expression/ActionExpression.g4 by ANTLR 4.8

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
    virtual antlrcpp::Any visitExpression_main(ActionExpressionParser::Expression_mainContext *context) = 0;

    virtual antlrcpp::Any visitExpression(ActionExpressionParser::ExpressionContext *context) = 0;

    virtual antlrcpp::Any visitParenthesis_expression(ActionExpressionParser::Parenthesis_expressionContext *context) = 0;

    virtual antlrcpp::Any visitValue_expression(ActionExpressionParser::Value_expressionContext *context) = 0;

    virtual antlrcpp::Any visitLiteral_expression(ActionExpressionParser::Literal_expressionContext *context) = 0;

    virtual antlrcpp::Any visitVariable_reference_expression(ActionExpressionParser::Variable_reference_expressionContext *context) = 0;

    virtual antlrcpp::Any visitRandom_range_expression(ActionExpressionParser::Random_range_expressionContext *context) = 0;

    virtual antlrcpp::Any visitVec3d_constructor(ActionExpressionParser::Vec3d_constructorContext *context) = 0;


};



// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/arg_parser/ArgumentList.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "ArgumentListParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by ArgumentListParser.
 */
class  ArgumentListVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by ArgumentListParser.
   */
    virtual antlrcpp::Any visitArg_list(ArgumentListParser::Arg_listContext *context) = 0;

    virtual antlrcpp::Any visitStandalone_type(ArgumentListParser::Standalone_typeContext *context) = 0;

    virtual antlrcpp::Any visitSimple_type(ArgumentListParser::Simple_typeContext *context) = 0;

    virtual antlrcpp::Any visitVarargs_or_simple_type(ArgumentListParser::Varargs_or_simple_typeContext *context) = 0;

    virtual antlrcpp::Any visitFunc_arg(ArgumentListParser::Func_argContext *context) = 0;

    virtual antlrcpp::Any visitFunc_arglist(ArgumentListParser::Func_arglistContext *context) = 0;

    virtual antlrcpp::Any visitFunction_type(ArgumentListParser::Function_typeContext *context) = 0;

    virtual antlrcpp::Any visitMap_type(ArgumentListParser::Map_typeContext *context) = 0;

    virtual antlrcpp::Any visitIterator_type(ArgumentListParser::Iterator_typeContext *context) = 0;

    virtual antlrcpp::Any visitType(ArgumentListParser::TypeContext *context) = 0;

    virtual antlrcpp::Any visitBoolean(ArgumentListParser::BooleanContext *context) = 0;

    virtual antlrcpp::Any visitValue(ArgumentListParser::ValueContext *context) = 0;

    virtual antlrcpp::Any visitActual_argument(ArgumentListParser::Actual_argumentContext *context) = 0;

    virtual antlrcpp::Any visitOptional_argument(ArgumentListParser::Optional_argumentContext *context) = 0;

    virtual antlrcpp::Any visitArgument(ArgumentListParser::ArgumentContext *context) = 0;


};


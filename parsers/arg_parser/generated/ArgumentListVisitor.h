
// Generated from ArgumentList.g4 by ANTLR 4.13.2

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
    virtual std::any visitArg_list(ArgumentListParser::Arg_listContext *context) = 0;

    virtual std::any visitStandalone_type(ArgumentListParser::Standalone_typeContext *context) = 0;

    virtual std::any visitSimple_type(ArgumentListParser::Simple_typeContext *context) = 0;

    virtual std::any visitVarargs_or_simple_type(ArgumentListParser::Varargs_or_simple_typeContext *context) = 0;

    virtual std::any visitFunc_arg(ArgumentListParser::Func_argContext *context) = 0;

    virtual std::any visitFunc_arglist(ArgumentListParser::Func_arglistContext *context) = 0;

    virtual std::any visitFunction_type(ArgumentListParser::Function_typeContext *context) = 0;

    virtual std::any visitMap_type(ArgumentListParser::Map_typeContext *context) = 0;

    virtual std::any visitIterator_type(ArgumentListParser::Iterator_typeContext *context) = 0;

    virtual std::any visitType(ArgumentListParser::TypeContext *context) = 0;

    virtual std::any visitBoolean(ArgumentListParser::BooleanContext *context) = 0;

    virtual std::any visitValue(ArgumentListParser::ValueContext *context) = 0;

    virtual std::any visitActual_argument(ArgumentListParser::Actual_argumentContext *context) = 0;

    virtual std::any visitOptional_argument(ArgumentListParser::Optional_argumentContext *context) = 0;

    virtual std::any visitArgument(ArgumentListParser::ArgumentContext *context) = 0;


};


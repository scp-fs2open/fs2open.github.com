
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/arg_parser/ArgumentList.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "ArgumentListVisitor.h"


/**
 * This class provides an empty implementation of ArgumentListVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ArgumentListBaseVisitor : public ArgumentListVisitor {
public:

  virtual antlrcpp::Any visitArg_list(ArgumentListParser::Arg_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStandalone_type(ArgumentListParser::Standalone_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSimple_type(ArgumentListParser::Simple_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarargs_or_simple_type(ArgumentListParser::Varargs_or_simple_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunc_arg(ArgumentListParser::Func_argContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunc_arglist(ArgumentListParser::Func_arglistContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunction_type(ArgumentListParser::Function_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMap_type(ArgumentListParser::Map_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIterator_type(ArgumentListParser::Iterator_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType(ArgumentListParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBoolean(ArgumentListParser::BooleanContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue(ArgumentListParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitActual_argument(ArgumentListParser::Actual_argumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOptional_argument(ArgumentListParser::Optional_argumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArgument(ArgumentListParser::ArgumentContext *ctx) override {
    return visitChildren(ctx);
  }


};



// Generated from ArgumentList.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ArgumentListVisitor.h"


/**
 * This class provides an empty implementation of ArgumentListVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ArgumentListBaseVisitor : public ArgumentListVisitor {
public:

  virtual std::any visitArg_list(ArgumentListParser::Arg_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStandalone_type(ArgumentListParser::Standalone_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimple_type(ArgumentListParser::Simple_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarargs_or_simple_type(ArgumentListParser::Varargs_or_simple_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunc_arg(ArgumentListParser::Func_argContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunc_arglist(ArgumentListParser::Func_arglistContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_type(ArgumentListParser::Function_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMap_type(ArgumentListParser::Map_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIterator_type(ArgumentListParser::Iterator_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType(ArgumentListParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBoolean(ArgumentListParser::BooleanContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(ArgumentListParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitActual_argument(ArgumentListParser::Actual_argumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOptional_argument(ArgumentListParser::Optional_argumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArgument(ArgumentListParser::ArgumentContext *ctx) override {
    return visitChildren(ctx);
  }


};


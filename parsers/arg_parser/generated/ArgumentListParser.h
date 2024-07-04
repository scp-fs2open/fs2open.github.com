
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/arg_parser/ArgumentList.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"




class  ArgumentListParser : public antlr4::Parser {
public:
  enum {
    COMMA = 1, EQUALS = 2, STRING = 3, NIL = 4, TRUE = 5, FALSE = 6, FUNCTION = 7, 
    VARARGS_SPECIFIER = 8, NUMBER = 9, TYPE_ALT = 10, L_BRACKET = 11, R_BRACKET = 12, 
    L_PAREN = 13, R_PAREN = 14, L_CURLY = 15, R_CURLY = 16, ARROW = 17, 
    ITERATOR = 18, L_ANGLE_BRACKET = 19, R_ANGLE_BRACKET = 20, ARG_COMMENT = 21, 
    ID = 22, SPACE = 23, OTHER = 24
  };

  enum {
    RuleArg_list = 0, RuleStandalone_type = 1, RuleSimple_type = 2, RuleVarargs_or_simple_type = 3, 
    RuleFunc_arg = 4, RuleFunc_arglist = 5, RuleFunction_type = 6, RuleMap_type = 7, 
    RuleIterator_type = 8, RuleType = 9, RuleBoolean = 10, RuleValue = 11, 
    RuleActual_argument = 12, RuleOptional_argument = 13, RuleArgument = 14
  };

  ArgumentListParser(antlr4::TokenStream *input);
  ~ArgumentListParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class Arg_listContext;
  class Standalone_typeContext;
  class Simple_typeContext;
  class Varargs_or_simple_typeContext;
  class Func_argContext;
  class Func_arglistContext;
  class Function_typeContext;
  class Map_typeContext;
  class Iterator_typeContext;
  class TypeContext;
  class BooleanContext;
  class ValueContext;
  class Actual_argumentContext;
  class Optional_argumentContext;
  class ArgumentContext; 

  class  Arg_listContext : public antlr4::ParserRuleContext {
  public:
    Arg_listContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    ArgumentContext *argument();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Arg_listContext* arg_list();

  class  Standalone_typeContext : public antlr4::ParserRuleContext {
  public:
    Standalone_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TypeContext *> type();
    TypeContext* type(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Standalone_typeContext* standalone_type();

  class  Simple_typeContext : public antlr4::ParserRuleContext {
  public:
    Simple_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *NIL();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Simple_typeContext* simple_type();

  class  Varargs_or_simple_typeContext : public antlr4::ParserRuleContext {
  public:
    Varargs_or_simple_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Simple_typeContext *simple_type();
    antlr4::tree::TerminalNode *VARARGS_SPECIFIER();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Varargs_or_simple_typeContext* varargs_or_simple_type();

  class  Func_argContext : public antlr4::ParserRuleContext {
  public:
    Func_argContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *ID();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Func_argContext* func_arg();

  class  Func_arglistContext : public antlr4::ParserRuleContext {
  public:
    Func_arglistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Func_argContext *> func_arg();
    Func_argContext* func_arg(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Func_arglistContext* func_arglist();

  class  Function_typeContext : public antlr4::ParserRuleContext {
  public:
    Function_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FUNCTION();
    antlr4::tree::TerminalNode *L_PAREN();
    Func_arglistContext *func_arglist();
    antlr4::tree::TerminalNode *R_PAREN();
    antlr4::tree::TerminalNode *ARROW();
    TypeContext *type();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Function_typeContext* function_type();

  class  Map_typeContext : public antlr4::ParserRuleContext {
  public:
    Map_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *L_CURLY();
    std::vector<TypeContext *> type();
    TypeContext* type(size_t i);
    antlr4::tree::TerminalNode *ARROW();
    antlr4::tree::TerminalNode *VARARGS_SPECIFIER();
    antlr4::tree::TerminalNode *R_CURLY();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Map_typeContext* map_type();

  class  Iterator_typeContext : public antlr4::ParserRuleContext {
  public:
    Iterator_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ITERATOR();
    antlr4::tree::TerminalNode *L_ANGLE_BRACKET();
    TypeContext *type();
    antlr4::tree::TerminalNode *R_ANGLE_BRACKET();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Iterator_typeContext* iterator_type();

  class  TypeContext : public antlr4::ParserRuleContext {
  public:
    TypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Varargs_or_simple_typeContext *varargs_or_simple_type();
    Function_typeContext *function_type();
    Map_typeContext *map_type();
    Iterator_typeContext *iterator_type();
    std::vector<TypeContext *> type();
    TypeContext* type(size_t i);
    antlr4::tree::TerminalNode *L_BRACKET();
    antlr4::tree::TerminalNode *R_BRACKET();
    std::vector<antlr4::tree::TerminalNode *> TYPE_ALT();
    antlr4::tree::TerminalNode* TYPE_ALT(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeContext* type();
  TypeContext* type(int precedence);
  class  BooleanContext : public antlr4::ParserRuleContext {
  public:
    BooleanContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BooleanContext* boolean();

  class  ValueContext : public antlr4::ParserRuleContext {
  public:
    ValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRING();
    antlr4::tree::TerminalNode *NIL();
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *ID();
    BooleanContext *boolean();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueContext* value();

  class  Actual_argumentContext : public antlr4::ParserRuleContext {
  public:
    Actual_argumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *EQUALS();
    ValueContext *value();
    antlr4::tree::TerminalNode *ARG_COMMENT();
    antlr4::tree::TerminalNode *COMMA();
    ArgumentContext *argument();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Actual_argumentContext* actual_argument();

  class  Optional_argumentContext : public antlr4::ParserRuleContext {
  public:
    Optional_argumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *L_BRACKET();
    Actual_argumentContext *actual_argument();
    antlr4::tree::TerminalNode *R_BRACKET();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Optional_argumentContext* optional_argument();

  class  ArgumentContext : public antlr4::ParserRuleContext {
  public:
    ArgumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Actual_argumentContext *actual_argument();
    Optional_argumentContext *optional_argument();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArgumentContext* argument();


  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool typeSempred(TypeContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};


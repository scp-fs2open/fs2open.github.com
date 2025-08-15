
// Generated from ActionExpression.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ActionExpressionParser : public antlr4::Parser {
public:
  enum {
    PLUS = 1, MINUS = 2, FLOAT = 3, INT = 4, RAND_L_PAREN = 5, L_PAREN = 6, 
    R_PAREN = 7, IDENTIFIER = 8, DOT = 9, STRING = 10, SPACE = 11, OTHER = 12
  };

  enum {
    RuleExpression_main = 0, RuleExpression = 1, RuleParenthesis_expression = 2, 
    RuleValue_expression = 3, RuleLiteral_expression = 4, RuleVariable_reference_expression = 5, 
    RuleRandom_range_expression = 6, RuleVec3d_constructor = 7
  };

  explicit ActionExpressionParser(antlr4::TokenStream *input);

  ActionExpressionParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~ActionExpressionParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class Expression_mainContext;
  class ExpressionContext;
  class Parenthesis_expressionContext;
  class Value_expressionContext;
  class Literal_expressionContext;
  class Variable_reference_expressionContext;
  class Random_range_expressionContext;
  class Vec3d_constructorContext; 

  class  Expression_mainContext : public antlr4::ParserRuleContext {
  public:
    Expression_mainContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *EOF();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Expression_mainContext* expression_main();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Value_expressionContext *value_expression();
    Random_range_expressionContext *random_range_expression();
    Parenthesis_expressionContext *parenthesis_expression();
    Variable_reference_expressionContext *variable_reference_expression();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *PLUS();
    antlr4::tree::TerminalNode *MINUS();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  Parenthesis_expressionContext : public antlr4::ParserRuleContext {
  public:
    Parenthesis_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *L_PAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *R_PAREN();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Parenthesis_expressionContext* parenthesis_expression();

  class  Value_expressionContext : public antlr4::ParserRuleContext {
  public:
    Value_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Literal_expressionContext *literal_expression();
    Vec3d_constructorContext *vec3d_constructor();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Value_expressionContext* value_expression();

  class  Literal_expressionContext : public antlr4::ParserRuleContext {
  public:
    Literal_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FLOAT();
    antlr4::tree::TerminalNode *INT();
    antlr4::tree::TerminalNode *STRING();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Literal_expressionContext* literal_expression();

  class  Variable_reference_expressionContext : public antlr4::ParserRuleContext {
  public:
    Variable_reference_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> IDENTIFIER();
    antlr4::tree::TerminalNode* IDENTIFIER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> DOT();
    antlr4::tree::TerminalNode* DOT(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Variable_reference_expressionContext* variable_reference_expression();

  class  Random_range_expressionContext : public antlr4::ParserRuleContext {
  public:
    Random_range_expressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RAND_L_PAREN();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *R_PAREN();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Random_range_expressionContext* random_range_expression();

  class  Vec3d_constructorContext : public antlr4::ParserRuleContext {
  public:
    Vec3d_constructorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *L_PAREN();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *R_PAREN();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Vec3d_constructorContext* vec3d_constructor();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};


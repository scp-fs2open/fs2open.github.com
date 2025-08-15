
// Generated from ActionExpression.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ActionExpressionLexer : public antlr4::Lexer {
public:
  enum {
    PLUS = 1, MINUS = 2, FLOAT = 3, INT = 4, RAND_L_PAREN = 5, L_PAREN = 6, 
    R_PAREN = 7, IDENTIFIER = 8, DOT = 9, STRING = 10, SPACE = 11, OTHER = 12
  };

  explicit ActionExpressionLexer(antlr4::CharStream *input);

  ~ActionExpressionLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};


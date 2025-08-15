
// Generated from ArgumentList.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ArgumentListLexer : public antlr4::Lexer {
public:
  enum {
    COMMA = 1, EQUALS = 2, STRING = 3, NIL = 4, TRUE = 5, FALSE = 6, FUNCTION = 7, 
    VARARGS_SPECIFIER = 8, NUMBER = 9, TYPE_ALT = 10, L_BRACKET = 11, R_BRACKET = 12, 
    L_PAREN = 13, R_PAREN = 14, L_CURLY = 15, R_CURLY = 16, ARROW = 17, 
    ITERATOR = 18, L_ANGLE_BRACKET = 19, R_ANGLE_BRACKET = 20, ARG_COMMENT = 21, 
    ID = 22, SPACE = 23, OTHER = 24
  };

  explicit ArgumentListLexer(antlr4::CharStream *input);

  ~ArgumentListLexer() override;


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


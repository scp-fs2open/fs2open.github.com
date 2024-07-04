
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/arg_parser/ArgumentList.g4 by ANTLR 4.8

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

  ArgumentListLexer(antlr4::CharStream *input);
  ~ArgumentListLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};


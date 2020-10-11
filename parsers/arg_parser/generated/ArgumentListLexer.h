
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/arg_parser/ArgumentList.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"




class  ArgumentListLexer : public antlr4::Lexer {
public:
  enum {
    COMMA = 1, EQUALS = 2, STRING = 3, PLACEHOLDER = 4, NIL = 5, TRUE = 6, 
    FALSE = 7, FUNCTION = 8, VARARGS_SPECIFIER = 9, NUMBER = 10, TYPE_ALT = 11, 
    L_BRACKET = 12, R_BRACKET = 13, L_PAREN = 14, R_PAREN = 15, ARROW = 16, 
    L_ANGLE_BRACKET = 17, R_ANGLE_BRACKET = 18, ARG_COMMENT = 19, ID = 20, 
    SPACE = 21, OTHER = 22
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


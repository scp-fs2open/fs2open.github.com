
// Generated from ArgumentList.g4 by ANTLR 4.13.2


#include "ArgumentListVisitor.h"

#include "ArgumentListParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct ArgumentListParserStaticData final {
  ArgumentListParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  ArgumentListParserStaticData(const ArgumentListParserStaticData&) = delete;
  ArgumentListParserStaticData(ArgumentListParserStaticData&&) = delete;
  ArgumentListParserStaticData& operator=(const ArgumentListParserStaticData&) = delete;
  ArgumentListParserStaticData& operator=(ArgumentListParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag argumentlistParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<ArgumentListParserStaticData> argumentlistParserStaticData = nullptr;

void argumentlistParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (argumentlistParserStaticData != nullptr) {
    return;
  }
#else
  assert(argumentlistParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<ArgumentListParserStaticData>(
    std::vector<std::string>{
      "arg_list", "standalone_type", "simple_type", "varargs_or_simple_type", 
      "func_arg", "func_arglist", "function_type", "map_type", "iterator_type", 
      "type", "boolean", "value", "actual_argument", "optional_argument", 
      "argument"
    },
    std::vector<std::string>{
      "", "','", "'='", "", "'nil'", "'true'", "'false'", "'function'", 
      "'...'", "", "", "'['", "']'", "'('", "')'", "'{'", "'}'", "'=>'", 
      "'iterator'", "'<'", "'>'"
    },
    std::vector<std::string>{
      "", "COMMA", "EQUALS", "STRING", "NIL", "TRUE", "FALSE", "FUNCTION", 
      "VARARGS_SPECIFIER", "NUMBER", "TYPE_ALT", "L_BRACKET", "R_BRACKET", 
      "L_PAREN", "R_PAREN", "L_CURLY", "R_CURLY", "ARROW", "ITERATOR", "L_ANGLE_BRACKET", 
      "R_ANGLE_BRACKET", "ARG_COMMENT", "ID", "SPACE", "OTHER"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,24,137,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,1,0,3,0,32,8,0,1,0,1,0,1,1,1,1,1,1,5,1,39,8,1,10,1,12,1,42,9,1,1,2,
  	1,2,1,3,1,3,3,3,48,8,3,1,4,1,4,1,4,1,5,1,5,1,5,1,5,5,5,57,8,5,10,5,12,
  	5,60,9,5,3,5,62,8,5,1,6,1,6,1,6,1,6,1,6,1,6,1,6,1,7,1,7,1,7,1,7,1,7,1,
  	7,1,7,1,8,1,8,1,8,1,8,1,8,1,9,1,9,1,9,1,9,1,9,3,9,88,8,9,1,9,1,9,1,9,
  	1,9,1,9,1,9,4,9,96,8,9,11,9,12,9,97,5,9,100,8,9,10,9,12,9,103,9,9,1,10,
  	1,10,1,11,1,11,1,11,1,11,1,11,3,11,112,8,11,1,12,1,12,3,12,116,8,12,1,
  	12,1,12,3,12,120,8,12,1,12,3,12,123,8,12,1,12,1,12,3,12,127,8,12,1,13,
  	1,13,1,13,1,13,1,14,1,14,3,14,135,8,14,1,14,0,1,18,15,0,2,4,6,8,10,12,
  	14,16,18,20,22,24,26,28,0,2,2,0,4,4,22,22,1,0,5,6,141,0,31,1,0,0,0,2,
  	35,1,0,0,0,4,43,1,0,0,0,6,45,1,0,0,0,8,49,1,0,0,0,10,61,1,0,0,0,12,63,
  	1,0,0,0,14,70,1,0,0,0,16,77,1,0,0,0,18,87,1,0,0,0,20,104,1,0,0,0,22,111,
  	1,0,0,0,24,113,1,0,0,0,26,128,1,0,0,0,28,134,1,0,0,0,30,32,3,28,14,0,
  	31,30,1,0,0,0,31,32,1,0,0,0,32,33,1,0,0,0,33,34,5,0,0,1,34,1,1,0,0,0,
  	35,40,3,18,9,0,36,37,5,1,0,0,37,39,3,18,9,0,38,36,1,0,0,0,39,42,1,0,0,
  	0,40,38,1,0,0,0,40,41,1,0,0,0,41,3,1,0,0,0,42,40,1,0,0,0,43,44,7,0,0,
  	0,44,5,1,0,0,0,45,47,3,4,2,0,46,48,5,8,0,0,47,46,1,0,0,0,47,48,1,0,0,
  	0,48,7,1,0,0,0,49,50,3,18,9,0,50,51,5,22,0,0,51,9,1,0,0,0,52,62,1,0,0,
  	0,53,58,3,8,4,0,54,55,5,1,0,0,55,57,3,8,4,0,56,54,1,0,0,0,57,60,1,0,0,
  	0,58,56,1,0,0,0,58,59,1,0,0,0,59,62,1,0,0,0,60,58,1,0,0,0,61,52,1,0,0,
  	0,61,53,1,0,0,0,62,11,1,0,0,0,63,64,5,7,0,0,64,65,5,13,0,0,65,66,3,10,
  	5,0,66,67,5,14,0,0,67,68,5,17,0,0,68,69,3,18,9,0,69,13,1,0,0,0,70,71,
  	5,15,0,0,71,72,3,18,9,0,72,73,5,17,0,0,73,74,3,18,9,0,74,75,5,8,0,0,75,
  	76,5,16,0,0,76,15,1,0,0,0,77,78,5,18,0,0,78,79,5,19,0,0,79,80,3,18,9,
  	0,80,81,5,20,0,0,81,17,1,0,0,0,82,83,6,9,-1,0,83,88,3,6,3,0,84,88,3,12,
  	6,0,85,88,3,14,7,0,86,88,3,16,8,0,87,82,1,0,0,0,87,84,1,0,0,0,87,85,1,
  	0,0,0,87,86,1,0,0,0,88,101,1,0,0,0,89,90,10,2,0,0,90,91,5,11,0,0,91,100,
  	5,12,0,0,92,95,10,1,0,0,93,94,5,10,0,0,94,96,3,18,9,0,95,93,1,0,0,0,96,
  	97,1,0,0,0,97,95,1,0,0,0,97,98,1,0,0,0,98,100,1,0,0,0,99,89,1,0,0,0,99,
  	92,1,0,0,0,100,103,1,0,0,0,101,99,1,0,0,0,101,102,1,0,0,0,102,19,1,0,
  	0,0,103,101,1,0,0,0,104,105,7,1,0,0,105,21,1,0,0,0,106,112,5,3,0,0,107,
  	112,5,4,0,0,108,112,5,9,0,0,109,112,5,22,0,0,110,112,3,20,10,0,111,106,
  	1,0,0,0,111,107,1,0,0,0,111,108,1,0,0,0,111,109,1,0,0,0,111,110,1,0,0,
  	0,112,23,1,0,0,0,113,115,3,18,9,0,114,116,5,22,0,0,115,114,1,0,0,0,115,
  	116,1,0,0,0,116,119,1,0,0,0,117,118,5,2,0,0,118,120,3,22,11,0,119,117,
  	1,0,0,0,119,120,1,0,0,0,120,122,1,0,0,0,121,123,5,21,0,0,122,121,1,0,
  	0,0,122,123,1,0,0,0,123,126,1,0,0,0,124,125,5,1,0,0,125,127,3,28,14,0,
  	126,124,1,0,0,0,126,127,1,0,0,0,127,25,1,0,0,0,128,129,5,11,0,0,129,130,
  	3,24,12,0,130,131,5,12,0,0,131,27,1,0,0,0,132,135,3,24,12,0,133,135,3,
  	26,13,0,134,132,1,0,0,0,134,133,1,0,0,0,135,29,1,0,0,0,15,31,40,47,58,
  	61,87,97,99,101,111,115,119,122,126,134
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  argumentlistParserStaticData = std::move(staticData);
}

}

ArgumentListParser::ArgumentListParser(TokenStream *input) : ArgumentListParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

ArgumentListParser::ArgumentListParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  ArgumentListParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *argumentlistParserStaticData->atn, argumentlistParserStaticData->decisionToDFA, argumentlistParserStaticData->sharedContextCache, options);
}

ArgumentListParser::~ArgumentListParser() {
  delete _interpreter;
}

const atn::ATN& ArgumentListParser::getATN() const {
  return *argumentlistParserStaticData->atn;
}

std::string ArgumentListParser::getGrammarFileName() const {
  return "ArgumentList.g4";
}

const std::vector<std::string>& ArgumentListParser::getRuleNames() const {
  return argumentlistParserStaticData->ruleNames;
}

const dfa::Vocabulary& ArgumentListParser::getVocabulary() const {
  return argumentlistParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ArgumentListParser::getSerializedATN() const {
  return argumentlistParserStaticData->serializedATN;
}


//----------------- Arg_listContext ------------------------------------------------------------------

ArgumentListParser::Arg_listContext::Arg_listContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::Arg_listContext::EOF() {
  return getToken(ArgumentListParser::EOF, 0);
}

ArgumentListParser::ArgumentContext* ArgumentListParser::Arg_listContext::argument() {
  return getRuleContext<ArgumentListParser::ArgumentContext>(0);
}


size_t ArgumentListParser::Arg_listContext::getRuleIndex() const {
  return ArgumentListParser::RuleArg_list;
}


std::any ArgumentListParser::Arg_listContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitArg_list(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Arg_listContext* ArgumentListParser::arg_list() {
  Arg_listContext *_localctx = _tracker.createInstance<Arg_listContext>(_ctx, getState());
  enterRule(_localctx, 0, ArgumentListParser::RuleArg_list);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(31);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4491408) != 0)) {
      setState(30);
      argument();
    }
    setState(33);
    match(ArgumentListParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Standalone_typeContext ------------------------------------------------------------------

ArgumentListParser::Standalone_typeContext::Standalone_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArgumentListParser::TypeContext *> ArgumentListParser::Standalone_typeContext::type() {
  return getRuleContexts<ArgumentListParser::TypeContext>();
}

ArgumentListParser::TypeContext* ArgumentListParser::Standalone_typeContext::type(size_t i) {
  return getRuleContext<ArgumentListParser::TypeContext>(i);
}

std::vector<tree::TerminalNode *> ArgumentListParser::Standalone_typeContext::COMMA() {
  return getTokens(ArgumentListParser::COMMA);
}

tree::TerminalNode* ArgumentListParser::Standalone_typeContext::COMMA(size_t i) {
  return getToken(ArgumentListParser::COMMA, i);
}


size_t ArgumentListParser::Standalone_typeContext::getRuleIndex() const {
  return ArgumentListParser::RuleStandalone_type;
}


std::any ArgumentListParser::Standalone_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitStandalone_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Standalone_typeContext* ArgumentListParser::standalone_type() {
  Standalone_typeContext *_localctx = _tracker.createInstance<Standalone_typeContext>(_ctx, getState());
  enterRule(_localctx, 2, ArgumentListParser::RuleStandalone_type);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(35);
    type(0);
    setState(40);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ArgumentListParser::COMMA) {
      setState(36);
      match(ArgumentListParser::COMMA);
      setState(37);
      type(0);
      setState(42);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Simple_typeContext ------------------------------------------------------------------

ArgumentListParser::Simple_typeContext::Simple_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::Simple_typeContext::ID() {
  return getToken(ArgumentListParser::ID, 0);
}

tree::TerminalNode* ArgumentListParser::Simple_typeContext::NIL() {
  return getToken(ArgumentListParser::NIL, 0);
}


size_t ArgumentListParser::Simple_typeContext::getRuleIndex() const {
  return ArgumentListParser::RuleSimple_type;
}


std::any ArgumentListParser::Simple_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitSimple_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Simple_typeContext* ArgumentListParser::simple_type() {
  Simple_typeContext *_localctx = _tracker.createInstance<Simple_typeContext>(_ctx, getState());
  enterRule(_localctx, 4, ArgumentListParser::RuleSimple_type);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(43);
    _la = _input->LA(1);
    if (!(_la == ArgumentListParser::NIL

    || _la == ArgumentListParser::ID)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Varargs_or_simple_typeContext ------------------------------------------------------------------

ArgumentListParser::Varargs_or_simple_typeContext::Varargs_or_simple_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArgumentListParser::Simple_typeContext* ArgumentListParser::Varargs_or_simple_typeContext::simple_type() {
  return getRuleContext<ArgumentListParser::Simple_typeContext>(0);
}

tree::TerminalNode* ArgumentListParser::Varargs_or_simple_typeContext::VARARGS_SPECIFIER() {
  return getToken(ArgumentListParser::VARARGS_SPECIFIER, 0);
}


size_t ArgumentListParser::Varargs_or_simple_typeContext::getRuleIndex() const {
  return ArgumentListParser::RuleVarargs_or_simple_type;
}


std::any ArgumentListParser::Varargs_or_simple_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitVarargs_or_simple_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Varargs_or_simple_typeContext* ArgumentListParser::varargs_or_simple_type() {
  Varargs_or_simple_typeContext *_localctx = _tracker.createInstance<Varargs_or_simple_typeContext>(_ctx, getState());
  enterRule(_localctx, 6, ArgumentListParser::RuleVarargs_or_simple_type);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(45);
    simple_type();
    setState(47);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      setState(46);
      match(ArgumentListParser::VARARGS_SPECIFIER);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Func_argContext ------------------------------------------------------------------

ArgumentListParser::Func_argContext::Func_argContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArgumentListParser::TypeContext* ArgumentListParser::Func_argContext::type() {
  return getRuleContext<ArgumentListParser::TypeContext>(0);
}

tree::TerminalNode* ArgumentListParser::Func_argContext::ID() {
  return getToken(ArgumentListParser::ID, 0);
}


size_t ArgumentListParser::Func_argContext::getRuleIndex() const {
  return ArgumentListParser::RuleFunc_arg;
}


std::any ArgumentListParser::Func_argContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunc_arg(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Func_argContext* ArgumentListParser::func_arg() {
  Func_argContext *_localctx = _tracker.createInstance<Func_argContext>(_ctx, getState());
  enterRule(_localctx, 8, ArgumentListParser::RuleFunc_arg);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(49);
    type(0);
    setState(50);
    match(ArgumentListParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Func_arglistContext ------------------------------------------------------------------

ArgumentListParser::Func_arglistContext::Func_arglistContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<ArgumentListParser::Func_argContext *> ArgumentListParser::Func_arglistContext::func_arg() {
  return getRuleContexts<ArgumentListParser::Func_argContext>();
}

ArgumentListParser::Func_argContext* ArgumentListParser::Func_arglistContext::func_arg(size_t i) {
  return getRuleContext<ArgumentListParser::Func_argContext>(i);
}

std::vector<tree::TerminalNode *> ArgumentListParser::Func_arglistContext::COMMA() {
  return getTokens(ArgumentListParser::COMMA);
}

tree::TerminalNode* ArgumentListParser::Func_arglistContext::COMMA(size_t i) {
  return getToken(ArgumentListParser::COMMA, i);
}


size_t ArgumentListParser::Func_arglistContext::getRuleIndex() const {
  return ArgumentListParser::RuleFunc_arglist;
}


std::any ArgumentListParser::Func_arglistContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunc_arglist(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Func_arglistContext* ArgumentListParser::func_arglist() {
  Func_arglistContext *_localctx = _tracker.createInstance<Func_arglistContext>(_ctx, getState());
  enterRule(_localctx, 10, ArgumentListParser::RuleFunc_arglist);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(61);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::R_PAREN: {
        enterOuterAlt(_localctx, 1);

        break;
      }

      case ArgumentListParser::NIL:
      case ArgumentListParser::FUNCTION:
      case ArgumentListParser::L_CURLY:
      case ArgumentListParser::ITERATOR:
      case ArgumentListParser::ID: {
        enterOuterAlt(_localctx, 2);
        setState(53);
        func_arg();
        setState(58);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == ArgumentListParser::COMMA) {
          setState(54);
          match(ArgumentListParser::COMMA);
          setState(55);
          func_arg();
          setState(60);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Function_typeContext ------------------------------------------------------------------

ArgumentListParser::Function_typeContext::Function_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::Function_typeContext::FUNCTION() {
  return getToken(ArgumentListParser::FUNCTION, 0);
}

tree::TerminalNode* ArgumentListParser::Function_typeContext::L_PAREN() {
  return getToken(ArgumentListParser::L_PAREN, 0);
}

ArgumentListParser::Func_arglistContext* ArgumentListParser::Function_typeContext::func_arglist() {
  return getRuleContext<ArgumentListParser::Func_arglistContext>(0);
}

tree::TerminalNode* ArgumentListParser::Function_typeContext::R_PAREN() {
  return getToken(ArgumentListParser::R_PAREN, 0);
}

tree::TerminalNode* ArgumentListParser::Function_typeContext::ARROW() {
  return getToken(ArgumentListParser::ARROW, 0);
}

ArgumentListParser::TypeContext* ArgumentListParser::Function_typeContext::type() {
  return getRuleContext<ArgumentListParser::TypeContext>(0);
}


size_t ArgumentListParser::Function_typeContext::getRuleIndex() const {
  return ArgumentListParser::RuleFunction_type;
}


std::any ArgumentListParser::Function_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunction_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Function_typeContext* ArgumentListParser::function_type() {
  Function_typeContext *_localctx = _tracker.createInstance<Function_typeContext>(_ctx, getState());
  enterRule(_localctx, 12, ArgumentListParser::RuleFunction_type);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(63);
    match(ArgumentListParser::FUNCTION);
    setState(64);
    match(ArgumentListParser::L_PAREN);
    setState(65);
    func_arglist();
    setState(66);
    match(ArgumentListParser::R_PAREN);
    setState(67);
    match(ArgumentListParser::ARROW);
    setState(68);
    type(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Map_typeContext ------------------------------------------------------------------

ArgumentListParser::Map_typeContext::Map_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::Map_typeContext::L_CURLY() {
  return getToken(ArgumentListParser::L_CURLY, 0);
}

std::vector<ArgumentListParser::TypeContext *> ArgumentListParser::Map_typeContext::type() {
  return getRuleContexts<ArgumentListParser::TypeContext>();
}

ArgumentListParser::TypeContext* ArgumentListParser::Map_typeContext::type(size_t i) {
  return getRuleContext<ArgumentListParser::TypeContext>(i);
}

tree::TerminalNode* ArgumentListParser::Map_typeContext::ARROW() {
  return getToken(ArgumentListParser::ARROW, 0);
}

tree::TerminalNode* ArgumentListParser::Map_typeContext::VARARGS_SPECIFIER() {
  return getToken(ArgumentListParser::VARARGS_SPECIFIER, 0);
}

tree::TerminalNode* ArgumentListParser::Map_typeContext::R_CURLY() {
  return getToken(ArgumentListParser::R_CURLY, 0);
}


size_t ArgumentListParser::Map_typeContext::getRuleIndex() const {
  return ArgumentListParser::RuleMap_type;
}


std::any ArgumentListParser::Map_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitMap_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Map_typeContext* ArgumentListParser::map_type() {
  Map_typeContext *_localctx = _tracker.createInstance<Map_typeContext>(_ctx, getState());
  enterRule(_localctx, 14, ArgumentListParser::RuleMap_type);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(70);
    match(ArgumentListParser::L_CURLY);
    setState(71);
    type(0);
    setState(72);
    match(ArgumentListParser::ARROW);
    setState(73);
    type(0);
    setState(74);
    match(ArgumentListParser::VARARGS_SPECIFIER);
    setState(75);
    match(ArgumentListParser::R_CURLY);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Iterator_typeContext ------------------------------------------------------------------

ArgumentListParser::Iterator_typeContext::Iterator_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::Iterator_typeContext::ITERATOR() {
  return getToken(ArgumentListParser::ITERATOR, 0);
}

tree::TerminalNode* ArgumentListParser::Iterator_typeContext::L_ANGLE_BRACKET() {
  return getToken(ArgumentListParser::L_ANGLE_BRACKET, 0);
}

ArgumentListParser::TypeContext* ArgumentListParser::Iterator_typeContext::type() {
  return getRuleContext<ArgumentListParser::TypeContext>(0);
}

tree::TerminalNode* ArgumentListParser::Iterator_typeContext::R_ANGLE_BRACKET() {
  return getToken(ArgumentListParser::R_ANGLE_BRACKET, 0);
}


size_t ArgumentListParser::Iterator_typeContext::getRuleIndex() const {
  return ArgumentListParser::RuleIterator_type;
}


std::any ArgumentListParser::Iterator_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitIterator_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Iterator_typeContext* ArgumentListParser::iterator_type() {
  Iterator_typeContext *_localctx = _tracker.createInstance<Iterator_typeContext>(_ctx, getState());
  enterRule(_localctx, 16, ArgumentListParser::RuleIterator_type);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(77);
    match(ArgumentListParser::ITERATOR);
    setState(78);
    match(ArgumentListParser::L_ANGLE_BRACKET);
    setState(79);
    type(0);
    setState(80);
    match(ArgumentListParser::R_ANGLE_BRACKET);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeContext ------------------------------------------------------------------

ArgumentListParser::TypeContext::TypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArgumentListParser::Varargs_or_simple_typeContext* ArgumentListParser::TypeContext::varargs_or_simple_type() {
  return getRuleContext<ArgumentListParser::Varargs_or_simple_typeContext>(0);
}

ArgumentListParser::Function_typeContext* ArgumentListParser::TypeContext::function_type() {
  return getRuleContext<ArgumentListParser::Function_typeContext>(0);
}

ArgumentListParser::Map_typeContext* ArgumentListParser::TypeContext::map_type() {
  return getRuleContext<ArgumentListParser::Map_typeContext>(0);
}

ArgumentListParser::Iterator_typeContext* ArgumentListParser::TypeContext::iterator_type() {
  return getRuleContext<ArgumentListParser::Iterator_typeContext>(0);
}

std::vector<ArgumentListParser::TypeContext *> ArgumentListParser::TypeContext::type() {
  return getRuleContexts<ArgumentListParser::TypeContext>();
}

ArgumentListParser::TypeContext* ArgumentListParser::TypeContext::type(size_t i) {
  return getRuleContext<ArgumentListParser::TypeContext>(i);
}

tree::TerminalNode* ArgumentListParser::TypeContext::L_BRACKET() {
  return getToken(ArgumentListParser::L_BRACKET, 0);
}

tree::TerminalNode* ArgumentListParser::TypeContext::R_BRACKET() {
  return getToken(ArgumentListParser::R_BRACKET, 0);
}

std::vector<tree::TerminalNode *> ArgumentListParser::TypeContext::TYPE_ALT() {
  return getTokens(ArgumentListParser::TYPE_ALT);
}

tree::TerminalNode* ArgumentListParser::TypeContext::TYPE_ALT(size_t i) {
  return getToken(ArgumentListParser::TYPE_ALT, i);
}


size_t ArgumentListParser::TypeContext::getRuleIndex() const {
  return ArgumentListParser::RuleType;
}


std::any ArgumentListParser::TypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitType(this);
  else
    return visitor->visitChildren(this);
}


ArgumentListParser::TypeContext* ArgumentListParser::type() {
   return type(0);
}

ArgumentListParser::TypeContext* ArgumentListParser::type(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  ArgumentListParser::TypeContext *_localctx = _tracker.createInstance<TypeContext>(_ctx, parentState);
  ArgumentListParser::TypeContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 18;
  enterRecursionRule(_localctx, 18, ArgumentListParser::RuleType, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(87);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::NIL:
      case ArgumentListParser::ID: {
        setState(83);
        varargs_or_simple_type();
        break;
      }

      case ArgumentListParser::FUNCTION: {
        setState(84);
        function_type();
        break;
      }

      case ArgumentListParser::L_CURLY: {
        setState(85);
        map_type();
        break;
      }

      case ArgumentListParser::ITERATOR: {
        setState(86);
        iterator_type();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(101);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(99);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<TypeContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleType);
          setState(89);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(90);
          match(ArgumentListParser::L_BRACKET);
          setState(91);
          match(ArgumentListParser::R_BRACKET);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<TypeContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleType);
          setState(92);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(95); 
          _errHandler->sync(this);
          alt = 1;
          do {
            switch (alt) {
              case 1: {
                    setState(93);
                    match(ArgumentListParser::TYPE_ALT);
                    setState(94);
                    type(0);
                    break;
                  }

            default:
              throw NoViableAltException(this);
            }
            setState(97); 
            _errHandler->sync(this);
            alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
          } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
          break;
        }

        default:
          break;
        } 
      }
      setState(103);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- BooleanContext ------------------------------------------------------------------

ArgumentListParser::BooleanContext::BooleanContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::BooleanContext::TRUE() {
  return getToken(ArgumentListParser::TRUE, 0);
}

tree::TerminalNode* ArgumentListParser::BooleanContext::FALSE() {
  return getToken(ArgumentListParser::FALSE, 0);
}


size_t ArgumentListParser::BooleanContext::getRuleIndex() const {
  return ArgumentListParser::RuleBoolean;
}


std::any ArgumentListParser::BooleanContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitBoolean(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::BooleanContext* ArgumentListParser::boolean() {
  BooleanContext *_localctx = _tracker.createInstance<BooleanContext>(_ctx, getState());
  enterRule(_localctx, 20, ArgumentListParser::RuleBoolean);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(104);
    _la = _input->LA(1);
    if (!(_la == ArgumentListParser::TRUE

    || _la == ArgumentListParser::FALSE)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValueContext ------------------------------------------------------------------

ArgumentListParser::ValueContext::ValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::ValueContext::STRING() {
  return getToken(ArgumentListParser::STRING, 0);
}

tree::TerminalNode* ArgumentListParser::ValueContext::NIL() {
  return getToken(ArgumentListParser::NIL, 0);
}

tree::TerminalNode* ArgumentListParser::ValueContext::NUMBER() {
  return getToken(ArgumentListParser::NUMBER, 0);
}

tree::TerminalNode* ArgumentListParser::ValueContext::ID() {
  return getToken(ArgumentListParser::ID, 0);
}

ArgumentListParser::BooleanContext* ArgumentListParser::ValueContext::boolean() {
  return getRuleContext<ArgumentListParser::BooleanContext>(0);
}


size_t ArgumentListParser::ValueContext::getRuleIndex() const {
  return ArgumentListParser::RuleValue;
}


std::any ArgumentListParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::ValueContext* ArgumentListParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 22, ArgumentListParser::RuleValue);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(111);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::STRING: {
        enterOuterAlt(_localctx, 1);
        setState(106);
        match(ArgumentListParser::STRING);
        break;
      }

      case ArgumentListParser::NIL: {
        enterOuterAlt(_localctx, 2);
        setState(107);
        match(ArgumentListParser::NIL);
        break;
      }

      case ArgumentListParser::NUMBER: {
        enterOuterAlt(_localctx, 3);
        setState(108);
        match(ArgumentListParser::NUMBER);
        break;
      }

      case ArgumentListParser::ID: {
        enterOuterAlt(_localctx, 4);
        setState(109);
        match(ArgumentListParser::ID);
        break;
      }

      case ArgumentListParser::TRUE:
      case ArgumentListParser::FALSE: {
        enterOuterAlt(_localctx, 5);
        setState(110);
        boolean();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Actual_argumentContext ------------------------------------------------------------------

ArgumentListParser::Actual_argumentContext::Actual_argumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArgumentListParser::TypeContext* ArgumentListParser::Actual_argumentContext::type() {
  return getRuleContext<ArgumentListParser::TypeContext>(0);
}

tree::TerminalNode* ArgumentListParser::Actual_argumentContext::ID() {
  return getToken(ArgumentListParser::ID, 0);
}

tree::TerminalNode* ArgumentListParser::Actual_argumentContext::EQUALS() {
  return getToken(ArgumentListParser::EQUALS, 0);
}

ArgumentListParser::ValueContext* ArgumentListParser::Actual_argumentContext::value() {
  return getRuleContext<ArgumentListParser::ValueContext>(0);
}

tree::TerminalNode* ArgumentListParser::Actual_argumentContext::ARG_COMMENT() {
  return getToken(ArgumentListParser::ARG_COMMENT, 0);
}

tree::TerminalNode* ArgumentListParser::Actual_argumentContext::COMMA() {
  return getToken(ArgumentListParser::COMMA, 0);
}

ArgumentListParser::ArgumentContext* ArgumentListParser::Actual_argumentContext::argument() {
  return getRuleContext<ArgumentListParser::ArgumentContext>(0);
}


size_t ArgumentListParser::Actual_argumentContext::getRuleIndex() const {
  return ArgumentListParser::RuleActual_argument;
}


std::any ArgumentListParser::Actual_argumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitActual_argument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Actual_argumentContext* ArgumentListParser::actual_argument() {
  Actual_argumentContext *_localctx = _tracker.createInstance<Actual_argumentContext>(_ctx, getState());
  enterRule(_localctx, 24, ArgumentListParser::RuleActual_argument);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(113);
    type(0);
    setState(115);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArgumentListParser::ID) {
      setState(114);
      match(ArgumentListParser::ID);
    }
    setState(119);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArgumentListParser::EQUALS) {
      setState(117);
      match(ArgumentListParser::EQUALS);
      setState(118);
      value();
    }
    setState(122);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArgumentListParser::ARG_COMMENT) {
      setState(121);
      match(ArgumentListParser::ARG_COMMENT);
    }
    setState(126);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArgumentListParser::COMMA) {
      setState(124);
      match(ArgumentListParser::COMMA);
      setState(125);
      argument();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Optional_argumentContext ------------------------------------------------------------------

ArgumentListParser::Optional_argumentContext::Optional_argumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ArgumentListParser::Optional_argumentContext::L_BRACKET() {
  return getToken(ArgumentListParser::L_BRACKET, 0);
}

ArgumentListParser::Actual_argumentContext* ArgumentListParser::Optional_argumentContext::actual_argument() {
  return getRuleContext<ArgumentListParser::Actual_argumentContext>(0);
}

tree::TerminalNode* ArgumentListParser::Optional_argumentContext::R_BRACKET() {
  return getToken(ArgumentListParser::R_BRACKET, 0);
}


size_t ArgumentListParser::Optional_argumentContext::getRuleIndex() const {
  return ArgumentListParser::RuleOptional_argument;
}


std::any ArgumentListParser::Optional_argumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitOptional_argument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Optional_argumentContext* ArgumentListParser::optional_argument() {
  Optional_argumentContext *_localctx = _tracker.createInstance<Optional_argumentContext>(_ctx, getState());
  enterRule(_localctx, 26, ArgumentListParser::RuleOptional_argument);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(128);
    match(ArgumentListParser::L_BRACKET);
    setState(129);
    actual_argument();
    setState(130);
    match(ArgumentListParser::R_BRACKET);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArgumentContext ------------------------------------------------------------------

ArgumentListParser::ArgumentContext::ArgumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ArgumentListParser::Actual_argumentContext* ArgumentListParser::ArgumentContext::actual_argument() {
  return getRuleContext<ArgumentListParser::Actual_argumentContext>(0);
}

ArgumentListParser::Optional_argumentContext* ArgumentListParser::ArgumentContext::optional_argument() {
  return getRuleContext<ArgumentListParser::Optional_argumentContext>(0);
}


size_t ArgumentListParser::ArgumentContext::getRuleIndex() const {
  return ArgumentListParser::RuleArgument;
}


std::any ArgumentListParser::ArgumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitArgument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::ArgumentContext* ArgumentListParser::argument() {
  ArgumentContext *_localctx = _tracker.createInstance<ArgumentContext>(_ctx, getState());
  enterRule(_localctx, 28, ArgumentListParser::RuleArgument);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(134);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::NIL:
      case ArgumentListParser::FUNCTION:
      case ArgumentListParser::L_CURLY:
      case ArgumentListParser::ITERATOR:
      case ArgumentListParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(132);
        actual_argument();
        break;
      }

      case ArgumentListParser::L_BRACKET: {
        enterOuterAlt(_localctx, 2);
        setState(133);
        optional_argument();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool ArgumentListParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 9: return typeSempred(antlrcpp::downCast<TypeContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool ArgumentListParser::typeSempred(TypeContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 2);
    case 1: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void ArgumentListParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  argumentlistParserInitialize();
#else
  ::antlr4::internal::call_once(argumentlistParserOnceFlag, argumentlistParserInitialize);
#endif
}

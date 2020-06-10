
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/arg_parser/ArgumentList.g4 by ANTLR 4.8


#include "ArgumentListVisitor.h"

#include "ArgumentListParser.h"


using namespace antlrcpp;
using namespace antlr4;

ArgumentListParser::ArgumentListParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

ArgumentListParser::~ArgumentListParser() {
  delete _interpreter;
}

std::string ArgumentListParser::getGrammarFileName() const {
  return "ArgumentList.g4";
}

const std::vector<std::string>& ArgumentListParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& ArgumentListParser::getVocabulary() const {
  return _vocabulary;
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


antlrcpp::Any ArgumentListParser::Arg_listContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitArg_list(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Arg_listContext* ArgumentListParser::arg_list() {
  Arg_listContext *_localctx = _tracker.createInstance<Arg_listContext>(_ctx, getState());
  enterRule(_localctx, 0, ArgumentListParser::RuleArg_list);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(23);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << ArgumentListParser::FUNCTION)
      | (1ULL << ArgumentListParser::L_BRACKET)
      | (1ULL << ArgumentListParser::ID))) != 0)) {
      setState(22);
      argument();
    }
    setState(25);
    match(ArgumentListParser::EOF);
   
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


size_t ArgumentListParser::Simple_typeContext::getRuleIndex() const {
  return ArgumentListParser::RuleSimple_type;
}


antlrcpp::Any ArgumentListParser::Simple_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitSimple_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Simple_typeContext* ArgumentListParser::simple_type() {
  Simple_typeContext *_localctx = _tracker.createInstance<Simple_typeContext>(_ctx, getState());
  enterRule(_localctx, 2, ArgumentListParser::RuleSimple_type);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(27);
    match(ArgumentListParser::ID);
   
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


antlrcpp::Any ArgumentListParser::Func_argContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunc_arg(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Func_argContext* ArgumentListParser::func_arg() {
  Func_argContext *_localctx = _tracker.createInstance<Func_argContext>(_ctx, getState());
  enterRule(_localctx, 4, ArgumentListParser::RuleFunc_arg);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(29);
    type(0);
    setState(30);
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


antlrcpp::Any ArgumentListParser::Func_arglistContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunc_arglist(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Func_arglistContext* ArgumentListParser::func_arglist() {
  Func_arglistContext *_localctx = _tracker.createInstance<Func_arglistContext>(_ctx, getState());
  enterRule(_localctx, 6, ArgumentListParser::RuleFunc_arglist);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(41);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::R_PAREN: {
        enterOuterAlt(_localctx, 1);

        break;
      }

      case ArgumentListParser::FUNCTION:
      case ArgumentListParser::ID: {
        enterOuterAlt(_localctx, 2);
        setState(33);
        func_arg();
        setState(38);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == ArgumentListParser::COMMA) {
          setState(34);
          match(ArgumentListParser::COMMA);
          setState(35);
          func_arg();
          setState(40);
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


antlrcpp::Any ArgumentListParser::Function_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunction_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Function_typeContext* ArgumentListParser::function_type() {
  Function_typeContext *_localctx = _tracker.createInstance<Function_typeContext>(_ctx, getState());
  enterRule(_localctx, 8, ArgumentListParser::RuleFunction_type);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(43);
    match(ArgumentListParser::FUNCTION);
    setState(44);
    match(ArgumentListParser::L_PAREN);
    setState(45);
    func_arglist();
    setState(46);
    match(ArgumentListParser::R_PAREN);
    setState(47);
    match(ArgumentListParser::ARROW);
    setState(48);
    type(0);
   
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

ArgumentListParser::Simple_typeContext* ArgumentListParser::TypeContext::simple_type() {
  return getRuleContext<ArgumentListParser::Simple_typeContext>(0);
}

ArgumentListParser::Function_typeContext* ArgumentListParser::TypeContext::function_type() {
  return getRuleContext<ArgumentListParser::Function_typeContext>(0);
}

std::vector<ArgumentListParser::TypeContext *> ArgumentListParser::TypeContext::type() {
  return getRuleContexts<ArgumentListParser::TypeContext>();
}

ArgumentListParser::TypeContext* ArgumentListParser::TypeContext::type(size_t i) {
  return getRuleContext<ArgumentListParser::TypeContext>(i);
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


antlrcpp::Any ArgumentListParser::TypeContext::accept(tree::ParseTreeVisitor *visitor) {
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
  size_t startState = 10;
  enterRecursionRule(_localctx, 10, ArgumentListParser::RuleType, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(53);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::ID: {
        setState(51);
        simple_type();
        break;
      }

      case ArgumentListParser::FUNCTION: {
        setState(52);
        function_type();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(64);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<TypeContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleType);
        setState(55);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(58); 
        _errHandler->sync(this);
        alt = 1;
        do {
          switch (alt) {
            case 1: {
                  setState(56);
                  match(ArgumentListParser::TYPE_ALT);
                  setState(57);
                  type(0);
                  break;
                }

          default:
            throw NoViableAltException(this);
          }
          setState(60); 
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx);
        } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER); 
      }
      setState(66);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
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


antlrcpp::Any ArgumentListParser::BooleanContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitBoolean(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::BooleanContext* ArgumentListParser::boolean() {
  BooleanContext *_localctx = _tracker.createInstance<BooleanContext>(_ctx, getState());
  enterRule(_localctx, 12, ArgumentListParser::RuleBoolean);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(67);
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

tree::TerminalNode* ArgumentListParser::ValueContext::PLACEHOLDER() {
  return getToken(ArgumentListParser::PLACEHOLDER, 0);
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


antlrcpp::Any ArgumentListParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::ValueContext* ArgumentListParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 14, ArgumentListParser::RuleValue);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(75);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::STRING: {
        enterOuterAlt(_localctx, 1);
        setState(69);
        match(ArgumentListParser::STRING);
        break;
      }

      case ArgumentListParser::PLACEHOLDER: {
        enterOuterAlt(_localctx, 2);
        setState(70);
        match(ArgumentListParser::PLACEHOLDER);
        break;
      }

      case ArgumentListParser::NIL: {
        enterOuterAlt(_localctx, 3);
        setState(71);
        match(ArgumentListParser::NIL);
        break;
      }

      case ArgumentListParser::NUMBER: {
        enterOuterAlt(_localctx, 4);
        setState(72);
        match(ArgumentListParser::NUMBER);
        break;
      }

      case ArgumentListParser::ID: {
        enterOuterAlt(_localctx, 5);
        setState(73);
        match(ArgumentListParser::ID);
        break;
      }

      case ArgumentListParser::TRUE:
      case ArgumentListParser::FALSE: {
        enterOuterAlt(_localctx, 6);
        setState(74);
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

tree::TerminalNode* ArgumentListParser::Actual_argumentContext::COMMA() {
  return getToken(ArgumentListParser::COMMA, 0);
}

ArgumentListParser::ArgumentContext* ArgumentListParser::Actual_argumentContext::argument() {
  return getRuleContext<ArgumentListParser::ArgumentContext>(0);
}


size_t ArgumentListParser::Actual_argumentContext::getRuleIndex() const {
  return ArgumentListParser::RuleActual_argument;
}


antlrcpp::Any ArgumentListParser::Actual_argumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitActual_argument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Actual_argumentContext* ArgumentListParser::actual_argument() {
  Actual_argumentContext *_localctx = _tracker.createInstance<Actual_argumentContext>(_ctx, getState());
  enterRule(_localctx, 16, ArgumentListParser::RuleActual_argument);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(77);
    type(0);
    setState(79);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArgumentListParser::ID) {
      setState(78);
      match(ArgumentListParser::ID);
    }
    setState(83);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArgumentListParser::EQUALS) {
      setState(81);
      match(ArgumentListParser::EQUALS);
      setState(82);
      value();
    }
    setState(87);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == ArgumentListParser::COMMA) {
      setState(85);
      match(ArgumentListParser::COMMA);
      setState(86);
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


antlrcpp::Any ArgumentListParser::Optional_argumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitOptional_argument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Optional_argumentContext* ArgumentListParser::optional_argument() {
  Optional_argumentContext *_localctx = _tracker.createInstance<Optional_argumentContext>(_ctx, getState());
  enterRule(_localctx, 18, ArgumentListParser::RuleOptional_argument);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(89);
    match(ArgumentListParser::L_BRACKET);
    setState(90);
    actual_argument();
    setState(91);
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


antlrcpp::Any ArgumentListParser::ArgumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitArgument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::ArgumentContext* ArgumentListParser::argument() {
  ArgumentContext *_localctx = _tracker.createInstance<ArgumentContext>(_ctx, getState());
  enterRule(_localctx, 20, ArgumentListParser::RuleArgument);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(95);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ArgumentListParser::FUNCTION:
      case ArgumentListParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(93);
        actual_argument();
        break;
      }

      case ArgumentListParser::L_BRACKET: {
        enterOuterAlt(_localctx, 2);
        setState(94);
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
    case 5: return typeSempred(dynamic_cast<TypeContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool ArgumentListParser::typeSempred(TypeContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> ArgumentListParser::_decisionToDFA;
atn::PredictionContextCache ArgumentListParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN ArgumentListParser::_atn;
std::vector<uint16_t> ArgumentListParser::_serializedATN;

std::vector<std::string> ArgumentListParser::_ruleNames = {
  "arg_list", "simple_type", "func_arg", "func_arglist", "function_type", 
  "type", "boolean", "value", "actual_argument", "optional_argument", "argument"
};

std::vector<std::string> ArgumentListParser::_literalNames = {
  "", "','", "'='", "", "", "'nil'", "'true'", "'false'", "'function'", 
  "", "", "'['", "']'", "'('", "')'", "'=>'"
};

std::vector<std::string> ArgumentListParser::_symbolicNames = {
  "", "COMMA", "EQUALS", "STRING", "PLACEHOLDER", "NIL", "TRUE", "FALSE", 
  "FUNCTION", "NUMBER", "TYPE_ALT", "L_BRACKET", "R_BRACKET", "L_PAREN", 
  "R_PAREN", "ARROW", "ID", "SPACE", "OTHER"
};

dfa::Vocabulary ArgumentListParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> ArgumentListParser::_tokenNames;

ArgumentListParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0x14, 0x64, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 0x4, 
    0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 0xb, 0x9, 
    0xb, 0x4, 0xc, 0x9, 0xc, 0x3, 0x2, 0x5, 0x2, 0x1a, 0xa, 0x2, 0x3, 0x2, 
    0x3, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x5, 
    0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x7, 0x5, 0x27, 0xa, 0x5, 0xc, 0x5, 0xe, 
    0x5, 0x2a, 0xb, 0x5, 0x5, 0x5, 0x2c, 0xa, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 
    0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x7, 0x3, 0x7, 0x3, 
    0x7, 0x5, 0x7, 0x38, 0xa, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x6, 0x7, 
    0x3d, 0xa, 0x7, 0xd, 0x7, 0xe, 0x7, 0x3e, 0x7, 0x7, 0x41, 0xa, 0x7, 
    0xc, 0x7, 0xe, 0x7, 0x44, 0xb, 0x7, 0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 
    0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x5, 0x9, 0x4e, 0xa, 0x9, 
    0x3, 0xa, 0x3, 0xa, 0x5, 0xa, 0x52, 0xa, 0xa, 0x3, 0xa, 0x3, 0xa, 0x5, 
    0xa, 0x56, 0xa, 0xa, 0x3, 0xa, 0x3, 0xa, 0x5, 0xa, 0x5a, 0xa, 0xa, 0x3, 
    0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xc, 0x3, 0xc, 0x5, 0xc, 0x62, 
    0xa, 0xc, 0x3, 0xc, 0x2, 0x3, 0xc, 0xd, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 
    0xe, 0x10, 0x12, 0x14, 0x16, 0x2, 0x3, 0x3, 0x2, 0x8, 0x9, 0x2, 0x67, 
    0x2, 0x19, 0x3, 0x2, 0x2, 0x2, 0x4, 0x1d, 0x3, 0x2, 0x2, 0x2, 0x6, 0x1f, 
    0x3, 0x2, 0x2, 0x2, 0x8, 0x2b, 0x3, 0x2, 0x2, 0x2, 0xa, 0x2d, 0x3, 0x2, 
    0x2, 0x2, 0xc, 0x37, 0x3, 0x2, 0x2, 0x2, 0xe, 0x45, 0x3, 0x2, 0x2, 0x2, 
    0x10, 0x4d, 0x3, 0x2, 0x2, 0x2, 0x12, 0x4f, 0x3, 0x2, 0x2, 0x2, 0x14, 
    0x5b, 0x3, 0x2, 0x2, 0x2, 0x16, 0x61, 0x3, 0x2, 0x2, 0x2, 0x18, 0x1a, 
    0x5, 0x16, 0xc, 0x2, 0x19, 0x18, 0x3, 0x2, 0x2, 0x2, 0x19, 0x1a, 0x3, 
    0x2, 0x2, 0x2, 0x1a, 0x1b, 0x3, 0x2, 0x2, 0x2, 0x1b, 0x1c, 0x7, 0x2, 
    0x2, 0x3, 0x1c, 0x3, 0x3, 0x2, 0x2, 0x2, 0x1d, 0x1e, 0x7, 0x12, 0x2, 
    0x2, 0x1e, 0x5, 0x3, 0x2, 0x2, 0x2, 0x1f, 0x20, 0x5, 0xc, 0x7, 0x2, 
    0x20, 0x21, 0x7, 0x12, 0x2, 0x2, 0x21, 0x7, 0x3, 0x2, 0x2, 0x2, 0x22, 
    0x2c, 0x3, 0x2, 0x2, 0x2, 0x23, 0x28, 0x5, 0x6, 0x4, 0x2, 0x24, 0x25, 
    0x7, 0x3, 0x2, 0x2, 0x25, 0x27, 0x5, 0x6, 0x4, 0x2, 0x26, 0x24, 0x3, 
    0x2, 0x2, 0x2, 0x27, 0x2a, 0x3, 0x2, 0x2, 0x2, 0x28, 0x26, 0x3, 0x2, 
    0x2, 0x2, 0x28, 0x29, 0x3, 0x2, 0x2, 0x2, 0x29, 0x2c, 0x3, 0x2, 0x2, 
    0x2, 0x2a, 0x28, 0x3, 0x2, 0x2, 0x2, 0x2b, 0x22, 0x3, 0x2, 0x2, 0x2, 
    0x2b, 0x23, 0x3, 0x2, 0x2, 0x2, 0x2c, 0x9, 0x3, 0x2, 0x2, 0x2, 0x2d, 
    0x2e, 0x7, 0xa, 0x2, 0x2, 0x2e, 0x2f, 0x7, 0xf, 0x2, 0x2, 0x2f, 0x30, 
    0x5, 0x8, 0x5, 0x2, 0x30, 0x31, 0x7, 0x10, 0x2, 0x2, 0x31, 0x32, 0x7, 
    0x11, 0x2, 0x2, 0x32, 0x33, 0x5, 0xc, 0x7, 0x2, 0x33, 0xb, 0x3, 0x2, 
    0x2, 0x2, 0x34, 0x35, 0x8, 0x7, 0x1, 0x2, 0x35, 0x38, 0x5, 0x4, 0x3, 
    0x2, 0x36, 0x38, 0x5, 0xa, 0x6, 0x2, 0x37, 0x34, 0x3, 0x2, 0x2, 0x2, 
    0x37, 0x36, 0x3, 0x2, 0x2, 0x2, 0x38, 0x42, 0x3, 0x2, 0x2, 0x2, 0x39, 
    0x3c, 0xc, 0x3, 0x2, 0x2, 0x3a, 0x3b, 0x7, 0xc, 0x2, 0x2, 0x3b, 0x3d, 
    0x5, 0xc, 0x7, 0x2, 0x3c, 0x3a, 0x3, 0x2, 0x2, 0x2, 0x3d, 0x3e, 0x3, 
    0x2, 0x2, 0x2, 0x3e, 0x3c, 0x3, 0x2, 0x2, 0x2, 0x3e, 0x3f, 0x3, 0x2, 
    0x2, 0x2, 0x3f, 0x41, 0x3, 0x2, 0x2, 0x2, 0x40, 0x39, 0x3, 0x2, 0x2, 
    0x2, 0x41, 0x44, 0x3, 0x2, 0x2, 0x2, 0x42, 0x40, 0x3, 0x2, 0x2, 0x2, 
    0x42, 0x43, 0x3, 0x2, 0x2, 0x2, 0x43, 0xd, 0x3, 0x2, 0x2, 0x2, 0x44, 
    0x42, 0x3, 0x2, 0x2, 0x2, 0x45, 0x46, 0x9, 0x2, 0x2, 0x2, 0x46, 0xf, 
    0x3, 0x2, 0x2, 0x2, 0x47, 0x4e, 0x7, 0x5, 0x2, 0x2, 0x48, 0x4e, 0x7, 
    0x6, 0x2, 0x2, 0x49, 0x4e, 0x7, 0x7, 0x2, 0x2, 0x4a, 0x4e, 0x7, 0xb, 
    0x2, 0x2, 0x4b, 0x4e, 0x7, 0x12, 0x2, 0x2, 0x4c, 0x4e, 0x5, 0xe, 0x8, 
    0x2, 0x4d, 0x47, 0x3, 0x2, 0x2, 0x2, 0x4d, 0x48, 0x3, 0x2, 0x2, 0x2, 
    0x4d, 0x49, 0x3, 0x2, 0x2, 0x2, 0x4d, 0x4a, 0x3, 0x2, 0x2, 0x2, 0x4d, 
    0x4b, 0x3, 0x2, 0x2, 0x2, 0x4d, 0x4c, 0x3, 0x2, 0x2, 0x2, 0x4e, 0x11, 
    0x3, 0x2, 0x2, 0x2, 0x4f, 0x51, 0x5, 0xc, 0x7, 0x2, 0x50, 0x52, 0x7, 
    0x12, 0x2, 0x2, 0x51, 0x50, 0x3, 0x2, 0x2, 0x2, 0x51, 0x52, 0x3, 0x2, 
    0x2, 0x2, 0x52, 0x55, 0x3, 0x2, 0x2, 0x2, 0x53, 0x54, 0x7, 0x4, 0x2, 
    0x2, 0x54, 0x56, 0x5, 0x10, 0x9, 0x2, 0x55, 0x53, 0x3, 0x2, 0x2, 0x2, 
    0x55, 0x56, 0x3, 0x2, 0x2, 0x2, 0x56, 0x59, 0x3, 0x2, 0x2, 0x2, 0x57, 
    0x58, 0x7, 0x3, 0x2, 0x2, 0x58, 0x5a, 0x5, 0x16, 0xc, 0x2, 0x59, 0x57, 
    0x3, 0x2, 0x2, 0x2, 0x59, 0x5a, 0x3, 0x2, 0x2, 0x2, 0x5a, 0x13, 0x3, 
    0x2, 0x2, 0x2, 0x5b, 0x5c, 0x7, 0xd, 0x2, 0x2, 0x5c, 0x5d, 0x5, 0x12, 
    0xa, 0x2, 0x5d, 0x5e, 0x7, 0xe, 0x2, 0x2, 0x5e, 0x15, 0x3, 0x2, 0x2, 
    0x2, 0x5f, 0x62, 0x5, 0x12, 0xa, 0x2, 0x60, 0x62, 0x5, 0x14, 0xb, 0x2, 
    0x61, 0x5f, 0x3, 0x2, 0x2, 0x2, 0x61, 0x60, 0x3, 0x2, 0x2, 0x2, 0x62, 
    0x17, 0x3, 0x2, 0x2, 0x2, 0xd, 0x19, 0x28, 0x2b, 0x37, 0x3e, 0x42, 0x4d, 
    0x51, 0x55, 0x59, 0x61, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

ArgumentListParser::Initializer ArgumentListParser::_init;

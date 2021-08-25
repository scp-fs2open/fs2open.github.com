
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
    setState(31);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << ArgumentListParser::NIL)
      | (1ULL << ArgumentListParser::FUNCTION)
      | (1ULL << ArgumentListParser::L_BRACKET)
      | (1ULL << ArgumentListParser::L_CURLY)
      | (1ULL << ArgumentListParser::ITERATOR)
      | (1ULL << ArgumentListParser::ID))) != 0)) {
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


antlrcpp::Any ArgumentListParser::Standalone_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitStandalone_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Standalone_typeContext* ArgumentListParser::standalone_type() {
  Standalone_typeContext *_localctx = _tracker.createInstance<Standalone_typeContext>(_ctx, getState());
  enterRule(_localctx, 2, ArgumentListParser::RuleStandalone_type);
  size_t _la = 0;

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Simple_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitSimple_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Simple_typeContext* ArgumentListParser::simple_type() {
  Simple_typeContext *_localctx = _tracker.createInstance<Simple_typeContext>(_ctx, getState());
  enterRule(_localctx, 4, ArgumentListParser::RuleSimple_type);
  size_t _la = 0;

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Varargs_or_simple_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitVarargs_or_simple_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Varargs_or_simple_typeContext* ArgumentListParser::varargs_or_simple_type() {
  Varargs_or_simple_typeContext *_localctx = _tracker.createInstance<Varargs_or_simple_typeContext>(_ctx, getState());
  enterRule(_localctx, 6, ArgumentListParser::RuleVarargs_or_simple_type);

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Func_argContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunc_arg(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Func_argContext* ArgumentListParser::func_arg() {
  Func_argContext *_localctx = _tracker.createInstance<Func_argContext>(_ctx, getState());
  enterRule(_localctx, 8, ArgumentListParser::RuleFunc_arg);

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Func_arglistContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunc_arglist(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Func_arglistContext* ArgumentListParser::func_arglist() {
  Func_arglistContext *_localctx = _tracker.createInstance<Func_arglistContext>(_ctx, getState());
  enterRule(_localctx, 10, ArgumentListParser::RuleFunc_arglist);
  size_t _la = 0;

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Function_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitFunction_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Function_typeContext* ArgumentListParser::function_type() {
  Function_typeContext *_localctx = _tracker.createInstance<Function_typeContext>(_ctx, getState());
  enterRule(_localctx, 12, ArgumentListParser::RuleFunction_type);

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Map_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitMap_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Map_typeContext* ArgumentListParser::map_type() {
  Map_typeContext *_localctx = _tracker.createInstance<Map_typeContext>(_ctx, getState());
  enterRule(_localctx, 14, ArgumentListParser::RuleMap_type);

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Iterator_typeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitIterator_type(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Iterator_typeContext* ArgumentListParser::iterator_type() {
  Iterator_typeContext *_localctx = _tracker.createInstance<Iterator_typeContext>(_ctx, getState());
  enterRule(_localctx, 16, ArgumentListParser::RuleIterator_type);

  auto onExit = finally([=] {
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
  size_t startState = 18;
  enterRecursionRule(_localctx, 18, ArgumentListParser::RuleType, precedence);

    

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::BooleanContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitBoolean(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::BooleanContext* ArgumentListParser::boolean() {
  BooleanContext *_localctx = _tracker.createInstance<BooleanContext>(_ctx, getState());
  enterRule(_localctx, 20, ArgumentListParser::RuleBoolean);
  size_t _la = 0;

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::ValueContext* ArgumentListParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 22, ArgumentListParser::RuleValue);

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Actual_argumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitActual_argument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Actual_argumentContext* ArgumentListParser::actual_argument() {
  Actual_argumentContext *_localctx = _tracker.createInstance<Actual_argumentContext>(_ctx, getState());
  enterRule(_localctx, 24, ArgumentListParser::RuleActual_argument);
  size_t _la = 0;

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::Optional_argumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitOptional_argument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::Optional_argumentContext* ArgumentListParser::optional_argument() {
  Optional_argumentContext *_localctx = _tracker.createInstance<Optional_argumentContext>(_ctx, getState());
  enterRule(_localctx, 26, ArgumentListParser::RuleOptional_argument);

  auto onExit = finally([=] {
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


antlrcpp::Any ArgumentListParser::ArgumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ArgumentListVisitor*>(visitor))
    return parserVisitor->visitArgument(this);
  else
    return visitor->visitChildren(this);
}

ArgumentListParser::ArgumentContext* ArgumentListParser::argument() {
  ArgumentContext *_localctx = _tracker.createInstance<ArgumentContext>(_ctx, getState());
  enterRule(_localctx, 28, ArgumentListParser::RuleArgument);

  auto onExit = finally([=] {
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
    case 9: return typeSempred(dynamic_cast<TypeContext *>(context), predicateIndex);

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

// Static vars and initialization.
std::vector<dfa::DFA> ArgumentListParser::_decisionToDFA;
atn::PredictionContextCache ArgumentListParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN ArgumentListParser::_atn;
std::vector<uint16_t> ArgumentListParser::_serializedATN;

std::vector<std::string> ArgumentListParser::_ruleNames = {
  "arg_list", "standalone_type", "simple_type", "varargs_or_simple_type", 
  "func_arg", "func_arglist", "function_type", "map_type", "iterator_type", 
  "type", "boolean", "value", "actual_argument", "optional_argument", "argument"
};

std::vector<std::string> ArgumentListParser::_literalNames = {
  "", "','", "'='", "", "'nil'", "'true'", "'false'", "'function'", "'...'", 
  "", "", "'['", "']'", "'('", "')'", "'{'", "'}'", "'=>'", "'iterator'", 
  "'<'", "'>'"
};

std::vector<std::string> ArgumentListParser::_symbolicNames = {
  "", "COMMA", "EQUALS", "STRING", "NIL", "TRUE", "FALSE", "FUNCTION", "VARARGS_SPECIFIER", 
  "NUMBER", "TYPE_ALT", "L_BRACKET", "R_BRACKET", "L_PAREN", "R_PAREN", 
  "L_CURLY", "R_CURLY", "ARROW", "ITERATOR", "L_ANGLE_BRACKET", "R_ANGLE_BRACKET", 
  "ARG_COMMENT", "ID", "SPACE", "OTHER"
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
    0x3, 0x1a, 0x8b, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 0x4, 
    0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 0xb, 0x9, 
    0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 0xe, 0x9, 0xe, 0x4, 
    0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x3, 0x2, 0x5, 0x2, 0x22, 0xa, 
    0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x7, 0x3, 0x29, 
    0xa, 0x3, 0xc, 0x3, 0xe, 0x3, 0x2c, 0xb, 0x3, 0x3, 0x4, 0x3, 0x4, 0x3, 
    0x5, 0x3, 0x5, 0x5, 0x5, 0x32, 0xa, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 
    0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x7, 0x7, 0x3b, 0xa, 0x7, 0xc, 
    0x7, 0xe, 0x7, 0x3e, 0xb, 0x7, 0x5, 0x7, 0x40, 0xa, 0x7, 0x3, 0x8, 0x3, 
    0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 
    0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0xa, 0x3, 
    0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 
    0xb, 0x3, 0xb, 0x5, 0xb, 0x5a, 0xa, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 
    0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x6, 0xb, 0x62, 0xa, 0xb, 0xd, 0xb, 0xe, 
    0xb, 0x63, 0x7, 0xb, 0x66, 0xa, 0xb, 0xc, 0xb, 0xe, 0xb, 0x69, 0xb, 
    0xb, 0x3, 0xc, 0x3, 0xc, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 
    0xd, 0x5, 0xd, 0x72, 0xa, 0xd, 0x3, 0xe, 0x3, 0xe, 0x5, 0xe, 0x76, 0xa, 
    0xe, 0x3, 0xe, 0x3, 0xe, 0x5, 0xe, 0x7a, 0xa, 0xe, 0x3, 0xe, 0x5, 0xe, 
    0x7d, 0xa, 0xe, 0x3, 0xe, 0x3, 0xe, 0x5, 0xe, 0x81, 0xa, 0xe, 0x3, 0xf, 
    0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0x10, 0x3, 0x10, 0x5, 0x10, 0x89, 
    0xa, 0x10, 0x3, 0x10, 0x2, 0x3, 0x14, 0x11, 0x2, 0x4, 0x6, 0x8, 0xa, 
    0xc, 0xe, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x2, 0x4, 
    0x4, 0x2, 0x6, 0x6, 0x18, 0x18, 0x3, 0x2, 0x7, 0x8, 0x2, 0x8f, 0x2, 
    0x21, 0x3, 0x2, 0x2, 0x2, 0x4, 0x25, 0x3, 0x2, 0x2, 0x2, 0x6, 0x2d, 
    0x3, 0x2, 0x2, 0x2, 0x8, 0x2f, 0x3, 0x2, 0x2, 0x2, 0xa, 0x33, 0x3, 0x2, 
    0x2, 0x2, 0xc, 0x3f, 0x3, 0x2, 0x2, 0x2, 0xe, 0x41, 0x3, 0x2, 0x2, 0x2, 
    0x10, 0x48, 0x3, 0x2, 0x2, 0x2, 0x12, 0x4f, 0x3, 0x2, 0x2, 0x2, 0x14, 
    0x59, 0x3, 0x2, 0x2, 0x2, 0x16, 0x6a, 0x3, 0x2, 0x2, 0x2, 0x18, 0x71, 
    0x3, 0x2, 0x2, 0x2, 0x1a, 0x73, 0x3, 0x2, 0x2, 0x2, 0x1c, 0x82, 0x3, 
    0x2, 0x2, 0x2, 0x1e, 0x88, 0x3, 0x2, 0x2, 0x2, 0x20, 0x22, 0x5, 0x1e, 
    0x10, 0x2, 0x21, 0x20, 0x3, 0x2, 0x2, 0x2, 0x21, 0x22, 0x3, 0x2, 0x2, 
    0x2, 0x22, 0x23, 0x3, 0x2, 0x2, 0x2, 0x23, 0x24, 0x7, 0x2, 0x2, 0x3, 
    0x24, 0x3, 0x3, 0x2, 0x2, 0x2, 0x25, 0x2a, 0x5, 0x14, 0xb, 0x2, 0x26, 
    0x27, 0x7, 0x3, 0x2, 0x2, 0x27, 0x29, 0x5, 0x14, 0xb, 0x2, 0x28, 0x26, 
    0x3, 0x2, 0x2, 0x2, 0x29, 0x2c, 0x3, 0x2, 0x2, 0x2, 0x2a, 0x28, 0x3, 
    0x2, 0x2, 0x2, 0x2a, 0x2b, 0x3, 0x2, 0x2, 0x2, 0x2b, 0x5, 0x3, 0x2, 
    0x2, 0x2, 0x2c, 0x2a, 0x3, 0x2, 0x2, 0x2, 0x2d, 0x2e, 0x9, 0x2, 0x2, 
    0x2, 0x2e, 0x7, 0x3, 0x2, 0x2, 0x2, 0x2f, 0x31, 0x5, 0x6, 0x4, 0x2, 
    0x30, 0x32, 0x7, 0xa, 0x2, 0x2, 0x31, 0x30, 0x3, 0x2, 0x2, 0x2, 0x31, 
    0x32, 0x3, 0x2, 0x2, 0x2, 0x32, 0x9, 0x3, 0x2, 0x2, 0x2, 0x33, 0x34, 
    0x5, 0x14, 0xb, 0x2, 0x34, 0x35, 0x7, 0x18, 0x2, 0x2, 0x35, 0xb, 0x3, 
    0x2, 0x2, 0x2, 0x36, 0x40, 0x3, 0x2, 0x2, 0x2, 0x37, 0x3c, 0x5, 0xa, 
    0x6, 0x2, 0x38, 0x39, 0x7, 0x3, 0x2, 0x2, 0x39, 0x3b, 0x5, 0xa, 0x6, 
    0x2, 0x3a, 0x38, 0x3, 0x2, 0x2, 0x2, 0x3b, 0x3e, 0x3, 0x2, 0x2, 0x2, 
    0x3c, 0x3a, 0x3, 0x2, 0x2, 0x2, 0x3c, 0x3d, 0x3, 0x2, 0x2, 0x2, 0x3d, 
    0x40, 0x3, 0x2, 0x2, 0x2, 0x3e, 0x3c, 0x3, 0x2, 0x2, 0x2, 0x3f, 0x36, 
    0x3, 0x2, 0x2, 0x2, 0x3f, 0x37, 0x3, 0x2, 0x2, 0x2, 0x40, 0xd, 0x3, 
    0x2, 0x2, 0x2, 0x41, 0x42, 0x7, 0x9, 0x2, 0x2, 0x42, 0x43, 0x7, 0xf, 
    0x2, 0x2, 0x43, 0x44, 0x5, 0xc, 0x7, 0x2, 0x44, 0x45, 0x7, 0x10, 0x2, 
    0x2, 0x45, 0x46, 0x7, 0x13, 0x2, 0x2, 0x46, 0x47, 0x5, 0x14, 0xb, 0x2, 
    0x47, 0xf, 0x3, 0x2, 0x2, 0x2, 0x48, 0x49, 0x7, 0x11, 0x2, 0x2, 0x49, 
    0x4a, 0x5, 0x14, 0xb, 0x2, 0x4a, 0x4b, 0x7, 0x13, 0x2, 0x2, 0x4b, 0x4c, 
    0x5, 0x14, 0xb, 0x2, 0x4c, 0x4d, 0x7, 0xa, 0x2, 0x2, 0x4d, 0x4e, 0x7, 
    0x12, 0x2, 0x2, 0x4e, 0x11, 0x3, 0x2, 0x2, 0x2, 0x4f, 0x50, 0x7, 0x14, 
    0x2, 0x2, 0x50, 0x51, 0x7, 0x15, 0x2, 0x2, 0x51, 0x52, 0x5, 0x14, 0xb, 
    0x2, 0x52, 0x53, 0x7, 0x16, 0x2, 0x2, 0x53, 0x13, 0x3, 0x2, 0x2, 0x2, 
    0x54, 0x55, 0x8, 0xb, 0x1, 0x2, 0x55, 0x5a, 0x5, 0x8, 0x5, 0x2, 0x56, 
    0x5a, 0x5, 0xe, 0x8, 0x2, 0x57, 0x5a, 0x5, 0x10, 0x9, 0x2, 0x58, 0x5a, 
    0x5, 0x12, 0xa, 0x2, 0x59, 0x54, 0x3, 0x2, 0x2, 0x2, 0x59, 0x56, 0x3, 
    0x2, 0x2, 0x2, 0x59, 0x57, 0x3, 0x2, 0x2, 0x2, 0x59, 0x58, 0x3, 0x2, 
    0x2, 0x2, 0x5a, 0x67, 0x3, 0x2, 0x2, 0x2, 0x5b, 0x5c, 0xc, 0x4, 0x2, 
    0x2, 0x5c, 0x5d, 0x7, 0xd, 0x2, 0x2, 0x5d, 0x66, 0x7, 0xe, 0x2, 0x2, 
    0x5e, 0x61, 0xc, 0x3, 0x2, 0x2, 0x5f, 0x60, 0x7, 0xc, 0x2, 0x2, 0x60, 
    0x62, 0x5, 0x14, 0xb, 0x2, 0x61, 0x5f, 0x3, 0x2, 0x2, 0x2, 0x62, 0x63, 
    0x3, 0x2, 0x2, 0x2, 0x63, 0x61, 0x3, 0x2, 0x2, 0x2, 0x63, 0x64, 0x3, 
    0x2, 0x2, 0x2, 0x64, 0x66, 0x3, 0x2, 0x2, 0x2, 0x65, 0x5b, 0x3, 0x2, 
    0x2, 0x2, 0x65, 0x5e, 0x3, 0x2, 0x2, 0x2, 0x66, 0x69, 0x3, 0x2, 0x2, 
    0x2, 0x67, 0x65, 0x3, 0x2, 0x2, 0x2, 0x67, 0x68, 0x3, 0x2, 0x2, 0x2, 
    0x68, 0x15, 0x3, 0x2, 0x2, 0x2, 0x69, 0x67, 0x3, 0x2, 0x2, 0x2, 0x6a, 
    0x6b, 0x9, 0x3, 0x2, 0x2, 0x6b, 0x17, 0x3, 0x2, 0x2, 0x2, 0x6c, 0x72, 
    0x7, 0x5, 0x2, 0x2, 0x6d, 0x72, 0x7, 0x6, 0x2, 0x2, 0x6e, 0x72, 0x7, 
    0xb, 0x2, 0x2, 0x6f, 0x72, 0x7, 0x18, 0x2, 0x2, 0x70, 0x72, 0x5, 0x16, 
    0xc, 0x2, 0x71, 0x6c, 0x3, 0x2, 0x2, 0x2, 0x71, 0x6d, 0x3, 0x2, 0x2, 
    0x2, 0x71, 0x6e, 0x3, 0x2, 0x2, 0x2, 0x71, 0x6f, 0x3, 0x2, 0x2, 0x2, 
    0x71, 0x70, 0x3, 0x2, 0x2, 0x2, 0x72, 0x19, 0x3, 0x2, 0x2, 0x2, 0x73, 
    0x75, 0x5, 0x14, 0xb, 0x2, 0x74, 0x76, 0x7, 0x18, 0x2, 0x2, 0x75, 0x74, 
    0x3, 0x2, 0x2, 0x2, 0x75, 0x76, 0x3, 0x2, 0x2, 0x2, 0x76, 0x79, 0x3, 
    0x2, 0x2, 0x2, 0x77, 0x78, 0x7, 0x4, 0x2, 0x2, 0x78, 0x7a, 0x5, 0x18, 
    0xd, 0x2, 0x79, 0x77, 0x3, 0x2, 0x2, 0x2, 0x79, 0x7a, 0x3, 0x2, 0x2, 
    0x2, 0x7a, 0x7c, 0x3, 0x2, 0x2, 0x2, 0x7b, 0x7d, 0x7, 0x17, 0x2, 0x2, 
    0x7c, 0x7b, 0x3, 0x2, 0x2, 0x2, 0x7c, 0x7d, 0x3, 0x2, 0x2, 0x2, 0x7d, 
    0x80, 0x3, 0x2, 0x2, 0x2, 0x7e, 0x7f, 0x7, 0x3, 0x2, 0x2, 0x7f, 0x81, 
    0x5, 0x1e, 0x10, 0x2, 0x80, 0x7e, 0x3, 0x2, 0x2, 0x2, 0x80, 0x81, 0x3, 
    0x2, 0x2, 0x2, 0x81, 0x1b, 0x3, 0x2, 0x2, 0x2, 0x82, 0x83, 0x7, 0xd, 
    0x2, 0x2, 0x83, 0x84, 0x5, 0x1a, 0xe, 0x2, 0x84, 0x85, 0x7, 0xe, 0x2, 
    0x2, 0x85, 0x1d, 0x3, 0x2, 0x2, 0x2, 0x86, 0x89, 0x5, 0x1a, 0xe, 0x2, 
    0x87, 0x89, 0x5, 0x1c, 0xf, 0x2, 0x88, 0x86, 0x3, 0x2, 0x2, 0x2, 0x88, 
    0x87, 0x3, 0x2, 0x2, 0x2, 0x89, 0x1f, 0x3, 0x2, 0x2, 0x2, 0x11, 0x21, 
    0x2a, 0x31, 0x3c, 0x3f, 0x59, 0x63, 0x65, 0x67, 0x71, 0x75, 0x79, 0x7c, 
    0x80, 0x88, 
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

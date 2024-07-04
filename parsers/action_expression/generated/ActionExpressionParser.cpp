
// Generated from /media/cache/code/asarium/fs2open.github.com/parsers/action_expression/ActionExpression.g4 by ANTLR 4.8


#include "ActionExpressionVisitor.h"

#include "ActionExpressionParser.h"


using namespace antlrcpp;
using namespace antlr4;

ActionExpressionParser::ActionExpressionParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

ActionExpressionParser::~ActionExpressionParser() {
  delete _interpreter;
}

std::string ActionExpressionParser::getGrammarFileName() const {
  return "ActionExpression.g4";
}

const std::vector<std::string>& ActionExpressionParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& ActionExpressionParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- Expression_mainContext ------------------------------------------------------------------

ActionExpressionParser::Expression_mainContext::Expression_mainContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ActionExpressionParser::ExpressionContext* ActionExpressionParser::Expression_mainContext::expression() {
  return getRuleContext<ActionExpressionParser::ExpressionContext>(0);
}

tree::TerminalNode* ActionExpressionParser::Expression_mainContext::EOF() {
  return getToken(ActionExpressionParser::EOF, 0);
}


size_t ActionExpressionParser::Expression_mainContext::getRuleIndex() const {
  return ActionExpressionParser::RuleExpression_main;
}


antlrcpp::Any ActionExpressionParser::Expression_mainContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitExpression_main(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Expression_mainContext* ActionExpressionParser::expression_main() {
  Expression_mainContext *_localctx = _tracker.createInstance<Expression_mainContext>(_ctx, getState());
  enterRule(_localctx, 0, ActionExpressionParser::RuleExpression_main);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(16);
    expression(0);
    setState(17);
    match(ActionExpressionParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

ActionExpressionParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ActionExpressionParser::Value_expressionContext* ActionExpressionParser::ExpressionContext::value_expression() {
  return getRuleContext<ActionExpressionParser::Value_expressionContext>(0);
}

ActionExpressionParser::Random_range_expressionContext* ActionExpressionParser::ExpressionContext::random_range_expression() {
  return getRuleContext<ActionExpressionParser::Random_range_expressionContext>(0);
}

ActionExpressionParser::Parenthesis_expressionContext* ActionExpressionParser::ExpressionContext::parenthesis_expression() {
  return getRuleContext<ActionExpressionParser::Parenthesis_expressionContext>(0);
}

ActionExpressionParser::Variable_reference_expressionContext* ActionExpressionParser::ExpressionContext::variable_reference_expression() {
  return getRuleContext<ActionExpressionParser::Variable_reference_expressionContext>(0);
}

std::vector<ActionExpressionParser::ExpressionContext *> ActionExpressionParser::ExpressionContext::expression() {
  return getRuleContexts<ActionExpressionParser::ExpressionContext>();
}

ActionExpressionParser::ExpressionContext* ActionExpressionParser::ExpressionContext::expression(size_t i) {
  return getRuleContext<ActionExpressionParser::ExpressionContext>(i);
}

tree::TerminalNode* ActionExpressionParser::ExpressionContext::PLUS() {
  return getToken(ActionExpressionParser::PLUS, 0);
}

tree::TerminalNode* ActionExpressionParser::ExpressionContext::MINUS() {
  return getToken(ActionExpressionParser::MINUS, 0);
}


size_t ActionExpressionParser::ExpressionContext::getRuleIndex() const {
  return ActionExpressionParser::RuleExpression;
}


antlrcpp::Any ActionExpressionParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitExpression(this);
  else
    return visitor->visitChildren(this);
}


ActionExpressionParser::ExpressionContext* ActionExpressionParser::expression() {
   return expression(0);
}

ActionExpressionParser::ExpressionContext* ActionExpressionParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  ActionExpressionParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  ActionExpressionParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 2;
  enterRecursionRule(_localctx, 2, ActionExpressionParser::RuleExpression, precedence);

    size_t _la = 0;

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(24);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
    case 1: {
      setState(20);
      value_expression();
      break;
    }

    case 2: {
      setState(21);
      random_range_expression();
      break;
    }

    case 3: {
      setState(22);
      parenthesis_expression();
      break;
    }

    case 4: {
      setState(23);
      variable_reference_expression();
      break;
    }

    }
    _ctx->stop = _input->LT(-1);
    setState(31);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleExpression);
        setState(26);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(27);
        _la = _input->LA(1);
        if (!(_la == ActionExpressionParser::PLUS

        || _la == ActionExpressionParser::MINUS)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(28);
        expression(2); 
      }
      setState(33);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- Parenthesis_expressionContext ------------------------------------------------------------------

ActionExpressionParser::Parenthesis_expressionContext::Parenthesis_expressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ActionExpressionParser::Parenthesis_expressionContext::L_PAREN() {
  return getToken(ActionExpressionParser::L_PAREN, 0);
}

ActionExpressionParser::ExpressionContext* ActionExpressionParser::Parenthesis_expressionContext::expression() {
  return getRuleContext<ActionExpressionParser::ExpressionContext>(0);
}

tree::TerminalNode* ActionExpressionParser::Parenthesis_expressionContext::R_PAREN() {
  return getToken(ActionExpressionParser::R_PAREN, 0);
}


size_t ActionExpressionParser::Parenthesis_expressionContext::getRuleIndex() const {
  return ActionExpressionParser::RuleParenthesis_expression;
}


antlrcpp::Any ActionExpressionParser::Parenthesis_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitParenthesis_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Parenthesis_expressionContext* ActionExpressionParser::parenthesis_expression() {
  Parenthesis_expressionContext *_localctx = _tracker.createInstance<Parenthesis_expressionContext>(_ctx, getState());
  enterRule(_localctx, 4, ActionExpressionParser::RuleParenthesis_expression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(34);
    match(ActionExpressionParser::L_PAREN);
    setState(35);
    expression(0);
    setState(36);
    match(ActionExpressionParser::R_PAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_expressionContext ------------------------------------------------------------------

ActionExpressionParser::Value_expressionContext::Value_expressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ActionExpressionParser::Literal_expressionContext* ActionExpressionParser::Value_expressionContext::literal_expression() {
  return getRuleContext<ActionExpressionParser::Literal_expressionContext>(0);
}

ActionExpressionParser::Vec3d_constructorContext* ActionExpressionParser::Value_expressionContext::vec3d_constructor() {
  return getRuleContext<ActionExpressionParser::Vec3d_constructorContext>(0);
}


size_t ActionExpressionParser::Value_expressionContext::getRuleIndex() const {
  return ActionExpressionParser::RuleValue_expression;
}


antlrcpp::Any ActionExpressionParser::Value_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitValue_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Value_expressionContext* ActionExpressionParser::value_expression() {
  Value_expressionContext *_localctx = _tracker.createInstance<Value_expressionContext>(_ctx, getState());
  enterRule(_localctx, 6, ActionExpressionParser::RuleValue_expression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(40);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ActionExpressionParser::FLOAT:
      case ActionExpressionParser::INT:
      case ActionExpressionParser::STRING: {
        enterOuterAlt(_localctx, 1);
        setState(38);
        literal_expression();
        break;
      }

      case ActionExpressionParser::L_PAREN: {
        enterOuterAlt(_localctx, 2);
        setState(39);
        vec3d_constructor();
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

//----------------- Literal_expressionContext ------------------------------------------------------------------

ActionExpressionParser::Literal_expressionContext::Literal_expressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ActionExpressionParser::Literal_expressionContext::FLOAT() {
  return getToken(ActionExpressionParser::FLOAT, 0);
}

tree::TerminalNode* ActionExpressionParser::Literal_expressionContext::INT() {
  return getToken(ActionExpressionParser::INT, 0);
}

tree::TerminalNode* ActionExpressionParser::Literal_expressionContext::STRING() {
  return getToken(ActionExpressionParser::STRING, 0);
}


size_t ActionExpressionParser::Literal_expressionContext::getRuleIndex() const {
  return ActionExpressionParser::RuleLiteral_expression;
}


antlrcpp::Any ActionExpressionParser::Literal_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitLiteral_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Literal_expressionContext* ActionExpressionParser::literal_expression() {
  Literal_expressionContext *_localctx = _tracker.createInstance<Literal_expressionContext>(_ctx, getState());
  enterRule(_localctx, 8, ActionExpressionParser::RuleLiteral_expression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(42);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << ActionExpressionParser::FLOAT)
      | (1ULL << ActionExpressionParser::INT)
      | (1ULL << ActionExpressionParser::STRING))) != 0))) {
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

//----------------- Variable_reference_expressionContext ------------------------------------------------------------------

ActionExpressionParser::Variable_reference_expressionContext::Variable_reference_expressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> ActionExpressionParser::Variable_reference_expressionContext::IDENTIFIER() {
  return getTokens(ActionExpressionParser::IDENTIFIER);
}

tree::TerminalNode* ActionExpressionParser::Variable_reference_expressionContext::IDENTIFIER(size_t i) {
  return getToken(ActionExpressionParser::IDENTIFIER, i);
}

std::vector<tree::TerminalNode *> ActionExpressionParser::Variable_reference_expressionContext::DOT() {
  return getTokens(ActionExpressionParser::DOT);
}

tree::TerminalNode* ActionExpressionParser::Variable_reference_expressionContext::DOT(size_t i) {
  return getToken(ActionExpressionParser::DOT, i);
}


size_t ActionExpressionParser::Variable_reference_expressionContext::getRuleIndex() const {
  return ActionExpressionParser::RuleVariable_reference_expression;
}


antlrcpp::Any ActionExpressionParser::Variable_reference_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitVariable_reference_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Variable_reference_expressionContext* ActionExpressionParser::variable_reference_expression() {
  Variable_reference_expressionContext *_localctx = _tracker.createInstance<Variable_reference_expressionContext>(_ctx, getState());
  enterRule(_localctx, 10, ActionExpressionParser::RuleVariable_reference_expression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(44);
    match(ActionExpressionParser::IDENTIFIER);
    setState(49);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(45);
        match(ActionExpressionParser::DOT);
        setState(46);
        match(ActionExpressionParser::IDENTIFIER); 
      }
      setState(51);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Random_range_expressionContext ------------------------------------------------------------------

ActionExpressionParser::Random_range_expressionContext::Random_range_expressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ActionExpressionParser::Random_range_expressionContext::RAND_L_PAREN() {
  return getToken(ActionExpressionParser::RAND_L_PAREN, 0);
}

std::vector<ActionExpressionParser::ExpressionContext *> ActionExpressionParser::Random_range_expressionContext::expression() {
  return getRuleContexts<ActionExpressionParser::ExpressionContext>();
}

ActionExpressionParser::ExpressionContext* ActionExpressionParser::Random_range_expressionContext::expression(size_t i) {
  return getRuleContext<ActionExpressionParser::ExpressionContext>(i);
}

tree::TerminalNode* ActionExpressionParser::Random_range_expressionContext::R_PAREN() {
  return getToken(ActionExpressionParser::R_PAREN, 0);
}


size_t ActionExpressionParser::Random_range_expressionContext::getRuleIndex() const {
  return ActionExpressionParser::RuleRandom_range_expression;
}


antlrcpp::Any ActionExpressionParser::Random_range_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitRandom_range_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Random_range_expressionContext* ActionExpressionParser::random_range_expression() {
  Random_range_expressionContext *_localctx = _tracker.createInstance<Random_range_expressionContext>(_ctx, getState());
  enterRule(_localctx, 12, ActionExpressionParser::RuleRandom_range_expression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(52);
    match(ActionExpressionParser::RAND_L_PAREN);
    setState(53);
    expression(0);
    setState(54);
    expression(0);
    setState(55);
    match(ActionExpressionParser::R_PAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Vec3d_constructorContext ------------------------------------------------------------------

ActionExpressionParser::Vec3d_constructorContext::Vec3d_constructorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* ActionExpressionParser::Vec3d_constructorContext::L_PAREN() {
  return getToken(ActionExpressionParser::L_PAREN, 0);
}

std::vector<ActionExpressionParser::ExpressionContext *> ActionExpressionParser::Vec3d_constructorContext::expression() {
  return getRuleContexts<ActionExpressionParser::ExpressionContext>();
}

ActionExpressionParser::ExpressionContext* ActionExpressionParser::Vec3d_constructorContext::expression(size_t i) {
  return getRuleContext<ActionExpressionParser::ExpressionContext>(i);
}

tree::TerminalNode* ActionExpressionParser::Vec3d_constructorContext::R_PAREN() {
  return getToken(ActionExpressionParser::R_PAREN, 0);
}


size_t ActionExpressionParser::Vec3d_constructorContext::getRuleIndex() const {
  return ActionExpressionParser::RuleVec3d_constructor;
}


antlrcpp::Any ActionExpressionParser::Vec3d_constructorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitVec3d_constructor(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Vec3d_constructorContext* ActionExpressionParser::vec3d_constructor() {
  Vec3d_constructorContext *_localctx = _tracker.createInstance<Vec3d_constructorContext>(_ctx, getState());
  enterRule(_localctx, 14, ActionExpressionParser::RuleVec3d_constructor);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(57);
    match(ActionExpressionParser::L_PAREN);
    setState(58);
    expression(0);
    setState(59);
    expression(0);
    setState(60);
    expression(0);
    setState(61);
    match(ActionExpressionParser::R_PAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool ActionExpressionParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 1: return expressionSempred(dynamic_cast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool ActionExpressionParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> ActionExpressionParser::_decisionToDFA;
atn::PredictionContextCache ActionExpressionParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN ActionExpressionParser::_atn;
std::vector<uint16_t> ActionExpressionParser::_serializedATN;

std::vector<std::string> ActionExpressionParser::_ruleNames = {
  "expression_main", "expression", "parenthesis_expression", "value_expression", 
  "literal_expression", "variable_reference_expression", "random_range_expression", 
  "vec3d_constructor"
};

std::vector<std::string> ActionExpressionParser::_literalNames = {
  "", "'+'", "'-'", "", "", "'~('", "'('", "')'", "", "'.'"
};

std::vector<std::string> ActionExpressionParser::_symbolicNames = {
  "", "PLUS", "MINUS", "FLOAT", "INT", "RAND_L_PAREN", "L_PAREN", "R_PAREN", 
  "IDENTIFIER", "DOT", "STRING", "SPACE", "OTHER"
};

dfa::Vocabulary ActionExpressionParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> ActionExpressionParser::_tokenNames;

ActionExpressionParser::Initializer::Initializer() {
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
    0x3, 0xe, 0x42, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 0x4, 
    0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 
    0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x5, 0x3, 0x1b, 0xa, 0x3, 
    0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x7, 0x3, 0x20, 0xa, 0x3, 0xc, 0x3, 0xe, 
    0x3, 0x23, 0xb, 0x3, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x5, 
    0x3, 0x5, 0x5, 0x5, 0x2b, 0xa, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x7, 0x3, 
    0x7, 0x3, 0x7, 0x7, 0x7, 0x32, 0xa, 0x7, 0xc, 0x7, 0xe, 0x7, 0x35, 0xb, 
    0x7, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 
    0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x2, 0x3, 0x4, 
    0xa, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 0x2, 0x4, 0x3, 0x2, 0x3, 
    0x4, 0x4, 0x2, 0x5, 0x6, 0xc, 0xc, 0x2, 0x3f, 0x2, 0x12, 0x3, 0x2, 0x2, 
    0x2, 0x4, 0x1a, 0x3, 0x2, 0x2, 0x2, 0x6, 0x24, 0x3, 0x2, 0x2, 0x2, 0x8, 
    0x2a, 0x3, 0x2, 0x2, 0x2, 0xa, 0x2c, 0x3, 0x2, 0x2, 0x2, 0xc, 0x2e, 
    0x3, 0x2, 0x2, 0x2, 0xe, 0x36, 0x3, 0x2, 0x2, 0x2, 0x10, 0x3b, 0x3, 
    0x2, 0x2, 0x2, 0x12, 0x13, 0x5, 0x4, 0x3, 0x2, 0x13, 0x14, 0x7, 0x2, 
    0x2, 0x3, 0x14, 0x3, 0x3, 0x2, 0x2, 0x2, 0x15, 0x16, 0x8, 0x3, 0x1, 
    0x2, 0x16, 0x1b, 0x5, 0x8, 0x5, 0x2, 0x17, 0x1b, 0x5, 0xe, 0x8, 0x2, 
    0x18, 0x1b, 0x5, 0x6, 0x4, 0x2, 0x19, 0x1b, 0x5, 0xc, 0x7, 0x2, 0x1a, 
    0x15, 0x3, 0x2, 0x2, 0x2, 0x1a, 0x17, 0x3, 0x2, 0x2, 0x2, 0x1a, 0x18, 
    0x3, 0x2, 0x2, 0x2, 0x1a, 0x19, 0x3, 0x2, 0x2, 0x2, 0x1b, 0x21, 0x3, 
    0x2, 0x2, 0x2, 0x1c, 0x1d, 0xc, 0x3, 0x2, 0x2, 0x1d, 0x1e, 0x9, 0x2, 
    0x2, 0x2, 0x1e, 0x20, 0x5, 0x4, 0x3, 0x4, 0x1f, 0x1c, 0x3, 0x2, 0x2, 
    0x2, 0x20, 0x23, 0x3, 0x2, 0x2, 0x2, 0x21, 0x1f, 0x3, 0x2, 0x2, 0x2, 
    0x21, 0x22, 0x3, 0x2, 0x2, 0x2, 0x22, 0x5, 0x3, 0x2, 0x2, 0x2, 0x23, 
    0x21, 0x3, 0x2, 0x2, 0x2, 0x24, 0x25, 0x7, 0x8, 0x2, 0x2, 0x25, 0x26, 
    0x5, 0x4, 0x3, 0x2, 0x26, 0x27, 0x7, 0x9, 0x2, 0x2, 0x27, 0x7, 0x3, 
    0x2, 0x2, 0x2, 0x28, 0x2b, 0x5, 0xa, 0x6, 0x2, 0x29, 0x2b, 0x5, 0x10, 
    0x9, 0x2, 0x2a, 0x28, 0x3, 0x2, 0x2, 0x2, 0x2a, 0x29, 0x3, 0x2, 0x2, 
    0x2, 0x2b, 0x9, 0x3, 0x2, 0x2, 0x2, 0x2c, 0x2d, 0x9, 0x3, 0x2, 0x2, 
    0x2d, 0xb, 0x3, 0x2, 0x2, 0x2, 0x2e, 0x33, 0x7, 0xa, 0x2, 0x2, 0x2f, 
    0x30, 0x7, 0xb, 0x2, 0x2, 0x30, 0x32, 0x7, 0xa, 0x2, 0x2, 0x31, 0x2f, 
    0x3, 0x2, 0x2, 0x2, 0x32, 0x35, 0x3, 0x2, 0x2, 0x2, 0x33, 0x31, 0x3, 
    0x2, 0x2, 0x2, 0x33, 0x34, 0x3, 0x2, 0x2, 0x2, 0x34, 0xd, 0x3, 0x2, 
    0x2, 0x2, 0x35, 0x33, 0x3, 0x2, 0x2, 0x2, 0x36, 0x37, 0x7, 0x7, 0x2, 
    0x2, 0x37, 0x38, 0x5, 0x4, 0x3, 0x2, 0x38, 0x39, 0x5, 0x4, 0x3, 0x2, 
    0x39, 0x3a, 0x7, 0x9, 0x2, 0x2, 0x3a, 0xf, 0x3, 0x2, 0x2, 0x2, 0x3b, 
    0x3c, 0x7, 0x8, 0x2, 0x2, 0x3c, 0x3d, 0x5, 0x4, 0x3, 0x2, 0x3d, 0x3e, 
    0x5, 0x4, 0x3, 0x2, 0x3e, 0x3f, 0x5, 0x4, 0x3, 0x2, 0x3f, 0x40, 0x7, 
    0x9, 0x2, 0x2, 0x40, 0x11, 0x3, 0x2, 0x2, 0x2, 0x6, 0x1a, 0x21, 0x2a, 
    0x33, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

ActionExpressionParser::Initializer ActionExpressionParser::_init;

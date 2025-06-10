
// Generated from ActionExpression.g4 by ANTLR 4.13.2


#include "ActionExpressionVisitor.h"

#include "ActionExpressionParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct ActionExpressionParserStaticData final {
  ActionExpressionParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  ActionExpressionParserStaticData(const ActionExpressionParserStaticData&) = delete;
  ActionExpressionParserStaticData(ActionExpressionParserStaticData&&) = delete;
  ActionExpressionParserStaticData& operator=(const ActionExpressionParserStaticData&) = delete;
  ActionExpressionParserStaticData& operator=(ActionExpressionParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag actionexpressionParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<ActionExpressionParserStaticData> actionexpressionParserStaticData = nullptr;

void actionexpressionParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (actionexpressionParserStaticData != nullptr) {
    return;
  }
#else
  assert(actionexpressionParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<ActionExpressionParserStaticData>(
    std::vector<std::string>{
      "expression_main", "expression", "parenthesis_expression", "value_expression", 
      "literal_expression", "variable_reference_expression", "random_range_expression", 
      "vec3d_constructor"
    },
    std::vector<std::string>{
      "", "'+'", "'-'", "", "", "'~('", "'('", "')'", "", "'.'"
    },
    std::vector<std::string>{
      "", "PLUS", "MINUS", "FLOAT", "INT", "RAND_L_PAREN", "L_PAREN", "R_PAREN", 
      "IDENTIFIER", "DOT", "STRING", "SPACE", "OTHER"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,12,64,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,7,
  	7,7,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,3,1,25,8,1,1,1,1,1,1,1,5,1,30,8,1,
  	10,1,12,1,33,9,1,1,2,1,2,1,2,1,2,1,3,1,3,3,3,41,8,3,1,4,1,4,1,5,1,5,1,
  	5,5,5,48,8,5,10,5,12,5,51,9,5,1,6,1,6,1,6,1,6,1,6,1,7,1,7,1,7,1,7,1,7,
  	1,7,1,7,0,1,2,8,0,2,4,6,8,10,12,14,0,2,1,0,1,2,2,0,3,4,10,10,61,0,16,
  	1,0,0,0,2,24,1,0,0,0,4,34,1,0,0,0,6,40,1,0,0,0,8,42,1,0,0,0,10,44,1,0,
  	0,0,12,52,1,0,0,0,14,57,1,0,0,0,16,17,3,2,1,0,17,18,5,0,0,1,18,1,1,0,
  	0,0,19,20,6,1,-1,0,20,25,3,6,3,0,21,25,3,12,6,0,22,25,3,4,2,0,23,25,3,
  	10,5,0,24,19,1,0,0,0,24,21,1,0,0,0,24,22,1,0,0,0,24,23,1,0,0,0,25,31,
  	1,0,0,0,26,27,10,1,0,0,27,28,7,0,0,0,28,30,3,2,1,2,29,26,1,0,0,0,30,33,
  	1,0,0,0,31,29,1,0,0,0,31,32,1,0,0,0,32,3,1,0,0,0,33,31,1,0,0,0,34,35,
  	5,6,0,0,35,36,3,2,1,0,36,37,5,7,0,0,37,5,1,0,0,0,38,41,3,8,4,0,39,41,
  	3,14,7,0,40,38,1,0,0,0,40,39,1,0,0,0,41,7,1,0,0,0,42,43,7,1,0,0,43,9,
  	1,0,0,0,44,49,5,8,0,0,45,46,5,9,0,0,46,48,5,8,0,0,47,45,1,0,0,0,48,51,
  	1,0,0,0,49,47,1,0,0,0,49,50,1,0,0,0,50,11,1,0,0,0,51,49,1,0,0,0,52,53,
  	5,5,0,0,53,54,3,2,1,0,54,55,3,2,1,0,55,56,5,7,0,0,56,13,1,0,0,0,57,58,
  	5,6,0,0,58,59,3,2,1,0,59,60,3,2,1,0,60,61,3,2,1,0,61,62,5,7,0,0,62,15,
  	1,0,0,0,4,24,31,40,49
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  actionexpressionParserStaticData = std::move(staticData);
}

}

ActionExpressionParser::ActionExpressionParser(TokenStream *input) : ActionExpressionParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

ActionExpressionParser::ActionExpressionParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  ActionExpressionParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *actionexpressionParserStaticData->atn, actionexpressionParserStaticData->decisionToDFA, actionexpressionParserStaticData->sharedContextCache, options);
}

ActionExpressionParser::~ActionExpressionParser() {
  delete _interpreter;
}

const atn::ATN& ActionExpressionParser::getATN() const {
  return *actionexpressionParserStaticData->atn;
}

std::string ActionExpressionParser::getGrammarFileName() const {
  return "ActionExpression.g4";
}

const std::vector<std::string>& ActionExpressionParser::getRuleNames() const {
  return actionexpressionParserStaticData->ruleNames;
}

const dfa::Vocabulary& ActionExpressionParser::getVocabulary() const {
  return actionexpressionParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ActionExpressionParser::getSerializedATN() const {
  return actionexpressionParserStaticData->serializedATN;
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


std::any ActionExpressionParser::Expression_mainContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitExpression_main(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Expression_mainContext* ActionExpressionParser::expression_main() {
  Expression_mainContext *_localctx = _tracker.createInstance<Expression_mainContext>(_ctx, getState());
  enterRule(_localctx, 0, ActionExpressionParser::RuleExpression_main);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
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


std::any ActionExpressionParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
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

    default:
      break;
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


std::any ActionExpressionParser::Parenthesis_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitParenthesis_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Parenthesis_expressionContext* ActionExpressionParser::parenthesis_expression() {
  Parenthesis_expressionContext *_localctx = _tracker.createInstance<Parenthesis_expressionContext>(_ctx, getState());
  enterRule(_localctx, 4, ActionExpressionParser::RuleParenthesis_expression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
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


std::any ActionExpressionParser::Value_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitValue_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Value_expressionContext* ActionExpressionParser::value_expression() {
  Value_expressionContext *_localctx = _tracker.createInstance<Value_expressionContext>(_ctx, getState());
  enterRule(_localctx, 6, ActionExpressionParser::RuleValue_expression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
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


std::any ActionExpressionParser::Literal_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitLiteral_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Literal_expressionContext* ActionExpressionParser::literal_expression() {
  Literal_expressionContext *_localctx = _tracker.createInstance<Literal_expressionContext>(_ctx, getState());
  enterRule(_localctx, 8, ActionExpressionParser::RuleLiteral_expression);
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
    setState(42);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1048) != 0))) {
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


std::any ActionExpressionParser::Variable_reference_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitVariable_reference_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Variable_reference_expressionContext* ActionExpressionParser::variable_reference_expression() {
  Variable_reference_expressionContext *_localctx = _tracker.createInstance<Variable_reference_expressionContext>(_ctx, getState());
  enterRule(_localctx, 10, ActionExpressionParser::RuleVariable_reference_expression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
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


std::any ActionExpressionParser::Random_range_expressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitRandom_range_expression(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Random_range_expressionContext* ActionExpressionParser::random_range_expression() {
  Random_range_expressionContext *_localctx = _tracker.createInstance<Random_range_expressionContext>(_ctx, getState());
  enterRule(_localctx, 12, ActionExpressionParser::RuleRandom_range_expression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
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


std::any ActionExpressionParser::Vec3d_constructorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<ActionExpressionVisitor*>(visitor))
    return parserVisitor->visitVec3d_constructor(this);
  else
    return visitor->visitChildren(this);
}

ActionExpressionParser::Vec3d_constructorContext* ActionExpressionParser::vec3d_constructor() {
  Vec3d_constructorContext *_localctx = _tracker.createInstance<Vec3d_constructorContext>(_ctx, getState());
  enterRule(_localctx, 14, ActionExpressionParser::RuleVec3d_constructor);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
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
    case 1: return expressionSempred(antlrcpp::downCast<ExpressionContext *>(context), predicateIndex);

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

void ActionExpressionParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  actionexpressionParserInitialize();
#else
  ::antlr4::internal::call_once(actionexpressionParserOnceFlag, actionexpressionParserInitialize);
#endif
}

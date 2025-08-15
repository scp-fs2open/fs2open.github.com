
// Generated from ActionExpression.g4 by ANTLR 4.13.2


#include "ActionExpressionLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct ActionExpressionLexerStaticData final {
  ActionExpressionLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  ActionExpressionLexerStaticData(const ActionExpressionLexerStaticData&) = delete;
  ActionExpressionLexerStaticData(ActionExpressionLexerStaticData&&) = delete;
  ActionExpressionLexerStaticData& operator=(const ActionExpressionLexerStaticData&) = delete;
  ActionExpressionLexerStaticData& operator=(ActionExpressionLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag actionexpressionlexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<ActionExpressionLexerStaticData> actionexpressionlexerLexerStaticData = nullptr;

void actionexpressionlexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (actionexpressionlexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(actionexpressionlexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<ActionExpressionLexerStaticData>(
    std::vector<std::string>{
      "PLUS", "MINUS", "FLOAT", "INT", "RAND_L_PAREN", "L_PAREN", "R_PAREN", 
      "IDENTIFIER", "DOT", "STRING", "SPACE", "OTHER"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
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
  	4,0,12,82,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,1,0,1,0,1,1,1,1,1,2,3,2,
  	31,8,2,1,2,4,2,34,8,2,11,2,12,2,35,1,2,1,2,4,2,40,8,2,11,2,12,2,41,1,
  	3,3,3,45,8,3,1,3,4,3,48,8,3,11,3,12,3,49,1,4,1,4,1,4,1,5,1,5,1,6,1,6,
  	1,7,1,7,5,7,61,8,7,10,7,12,7,64,9,7,1,8,1,8,1,9,1,9,5,9,70,8,9,10,9,12,
  	9,73,9,9,1,9,1,9,1,10,1,10,1,10,1,10,1,11,1,11,1,71,0,12,1,1,3,2,5,3,
  	7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,1,0,4,1,0,48,57,2,0,65,
  	90,97,122,3,0,48,57,65,90,97,122,3,0,9,10,13,13,32,32,88,0,1,1,0,0,0,
  	0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,
  	0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,
  	1,25,1,0,0,0,3,27,1,0,0,0,5,30,1,0,0,0,7,44,1,0,0,0,9,51,1,0,0,0,11,54,
  	1,0,0,0,13,56,1,0,0,0,15,58,1,0,0,0,17,65,1,0,0,0,19,67,1,0,0,0,21,76,
  	1,0,0,0,23,80,1,0,0,0,25,26,5,43,0,0,26,2,1,0,0,0,27,28,5,45,0,0,28,4,
  	1,0,0,0,29,31,5,45,0,0,30,29,1,0,0,0,30,31,1,0,0,0,31,33,1,0,0,0,32,34,
  	7,0,0,0,33,32,1,0,0,0,34,35,1,0,0,0,35,33,1,0,0,0,35,36,1,0,0,0,36,37,
  	1,0,0,0,37,39,5,46,0,0,38,40,7,0,0,0,39,38,1,0,0,0,40,41,1,0,0,0,41,39,
  	1,0,0,0,41,42,1,0,0,0,42,6,1,0,0,0,43,45,5,45,0,0,44,43,1,0,0,0,44,45,
  	1,0,0,0,45,47,1,0,0,0,46,48,7,0,0,0,47,46,1,0,0,0,48,49,1,0,0,0,49,47,
  	1,0,0,0,49,50,1,0,0,0,50,8,1,0,0,0,51,52,5,126,0,0,52,53,5,40,0,0,53,
  	10,1,0,0,0,54,55,5,40,0,0,55,12,1,0,0,0,56,57,5,41,0,0,57,14,1,0,0,0,
  	58,62,7,1,0,0,59,61,7,2,0,0,60,59,1,0,0,0,61,64,1,0,0,0,62,60,1,0,0,0,
  	62,63,1,0,0,0,63,16,1,0,0,0,64,62,1,0,0,0,65,66,5,46,0,0,66,18,1,0,0,
  	0,67,71,5,34,0,0,68,70,9,0,0,0,69,68,1,0,0,0,70,73,1,0,0,0,71,72,1,0,
  	0,0,71,69,1,0,0,0,72,74,1,0,0,0,73,71,1,0,0,0,74,75,5,34,0,0,75,20,1,
  	0,0,0,76,77,7,3,0,0,77,78,1,0,0,0,78,79,6,10,0,0,79,22,1,0,0,0,80,81,
  	9,0,0,0,81,24,1,0,0,0,8,0,30,35,41,44,49,62,71,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  actionexpressionlexerLexerStaticData = std::move(staticData);
}

}

ActionExpressionLexer::ActionExpressionLexer(CharStream *input) : Lexer(input) {
  ActionExpressionLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *actionexpressionlexerLexerStaticData->atn, actionexpressionlexerLexerStaticData->decisionToDFA, actionexpressionlexerLexerStaticData->sharedContextCache);
}

ActionExpressionLexer::~ActionExpressionLexer() {
  delete _interpreter;
}

std::string ActionExpressionLexer::getGrammarFileName() const {
  return "ActionExpression.g4";
}

const std::vector<std::string>& ActionExpressionLexer::getRuleNames() const {
  return actionexpressionlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& ActionExpressionLexer::getChannelNames() const {
  return actionexpressionlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& ActionExpressionLexer::getModeNames() const {
  return actionexpressionlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& ActionExpressionLexer::getVocabulary() const {
  return actionexpressionlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ActionExpressionLexer::getSerializedATN() const {
  return actionexpressionlexerLexerStaticData->serializedATN;
}

const atn::ATN& ActionExpressionLexer::getATN() const {
  return *actionexpressionlexerLexerStaticData->atn;
}




void ActionExpressionLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  actionexpressionlexerLexerInitialize();
#else
  ::antlr4::internal::call_once(actionexpressionlexerLexerOnceFlag, actionexpressionlexerLexerInitialize);
#endif
}


// Generated from src/query/PqlLexer.g4 by ANTLR 4.13.2


#include "PqlLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct PqlLexerStaticData final {
  PqlLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PqlLexerStaticData(const PqlLexerStaticData&) = delete;
  PqlLexerStaticData(PqlLexerStaticData&&) = delete;
  PqlLexerStaticData& operator=(const PqlLexerStaticData&) = delete;
  PqlLexerStaticData& operator=(PqlLexerStaticData&&) = delete;

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

::antlr4::internal::OnceFlag pqllexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<PqlLexerStaticData> pqllexerLexerStaticData = nullptr;

void pqllexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (pqllexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(pqllexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<PqlLexerStaticData>(
    std::vector<std::string>{
      "OR", "AND", "NOT", "LPAREN", "RPAREN", "FIELD", "STRING", "WORD", 
      "WS", "V_GTE", "V_LTE", "V_GT", "V_LT", "V_EQ", "V_STRING", "V_WORD", 
      "V_WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE", "VALUE"
    },
    std::vector<std::string>{
      "", "", "", "", "'('", "')'", "", "", "", "", "'>='", "'<='", "'>'", 
      "'<'", "'='"
    },
    std::vector<std::string>{
      "", "OR", "AND", "NOT", "LPAREN", "RPAREN", "FIELD", "STRING", "WORD", 
      "WS", "V_GTE", "V_LTE", "V_GT", "V_LT", "V_EQ", "V_STRING", "V_WORD", 
      "V_WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,17,135,6,-1,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,
  	2,6,7,6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,
  	13,2,14,7,14,2,15,7,15,2,16,7,16,1,0,1,0,1,0,1,0,3,0,41,8,0,1,1,1,1,1,
  	1,1,1,1,1,3,1,48,8,1,1,2,1,2,1,2,1,2,3,2,54,8,2,1,3,1,3,1,4,1,4,1,5,4,
  	5,61,8,5,11,5,12,5,62,1,5,1,5,1,5,1,5,1,6,1,6,1,6,1,6,5,6,73,8,6,10,6,
  	12,6,76,9,6,1,6,1,6,1,7,1,7,5,7,82,8,7,10,7,12,7,85,9,7,1,8,4,8,88,8,
  	8,11,8,12,8,89,1,8,1,8,1,9,1,9,1,9,1,10,1,10,1,10,1,11,1,11,1,12,1,12,
  	1,13,1,13,1,14,1,14,1,14,1,14,5,14,110,8,14,10,14,12,14,113,9,14,1,14,
  	1,14,1,14,1,14,1,15,1,15,5,15,121,8,15,10,15,12,15,124,9,15,1,15,1,15,
  	1,16,4,16,129,8,16,11,16,12,16,130,1,16,1,16,1,16,0,0,17,2,1,4,2,6,3,
  	8,4,10,5,12,6,14,7,16,8,18,9,20,10,22,11,24,12,26,13,28,14,30,15,32,16,
  	34,17,2,0,1,8,2,0,33,33,45,45,3,0,65,90,95,95,97,122,2,0,34,34,92,92,
  	9,0,9,10,13,13,32,34,38,38,40,41,45,45,58,58,60,62,124,124,6,0,9,10,13,
  	13,32,34,40,41,58,58,60,62,3,0,9,10,13,13,32,32,6,0,9,10,13,13,32,32,
  	34,34,40,41,60,62,5,0,9,10,13,13,32,32,34,34,40,41,145,0,2,1,0,0,0,0,
  	4,1,0,0,0,0,6,1,0,0,0,0,8,1,0,0,0,0,10,1,0,0,0,0,12,1,0,0,0,0,14,1,0,
  	0,0,0,16,1,0,0,0,0,18,1,0,0,0,1,20,1,0,0,0,1,22,1,0,0,0,1,24,1,0,0,0,
  	1,26,1,0,0,0,1,28,1,0,0,0,1,30,1,0,0,0,1,32,1,0,0,0,1,34,1,0,0,0,2,40,
  	1,0,0,0,4,47,1,0,0,0,6,53,1,0,0,0,8,55,1,0,0,0,10,57,1,0,0,0,12,60,1,
  	0,0,0,14,68,1,0,0,0,16,79,1,0,0,0,18,87,1,0,0,0,20,93,1,0,0,0,22,96,1,
  	0,0,0,24,99,1,0,0,0,26,101,1,0,0,0,28,103,1,0,0,0,30,105,1,0,0,0,32,118,
  	1,0,0,0,34,128,1,0,0,0,36,37,5,79,0,0,37,41,5,82,0,0,38,39,5,124,0,0,
  	39,41,5,124,0,0,40,36,1,0,0,0,40,38,1,0,0,0,41,3,1,0,0,0,42,43,5,65,0,
  	0,43,44,5,78,0,0,44,48,5,68,0,0,45,46,5,38,0,0,46,48,5,38,0,0,47,42,1,
  	0,0,0,47,45,1,0,0,0,48,5,1,0,0,0,49,50,5,78,0,0,50,51,5,79,0,0,51,54,
  	5,84,0,0,52,54,7,0,0,0,53,49,1,0,0,0,53,52,1,0,0,0,54,7,1,0,0,0,55,56,
  	5,40,0,0,56,9,1,0,0,0,57,58,5,41,0,0,58,11,1,0,0,0,59,61,7,1,0,0,60,59,
  	1,0,0,0,61,62,1,0,0,0,62,60,1,0,0,0,62,63,1,0,0,0,63,64,1,0,0,0,64,65,
  	5,58,0,0,65,66,1,0,0,0,66,67,6,5,0,0,67,13,1,0,0,0,68,74,5,34,0,0,69,
  	70,5,92,0,0,70,73,9,0,0,0,71,73,8,2,0,0,72,69,1,0,0,0,72,71,1,0,0,0,73,
  	76,1,0,0,0,74,72,1,0,0,0,74,75,1,0,0,0,75,77,1,0,0,0,76,74,1,0,0,0,77,
  	78,5,34,0,0,78,15,1,0,0,0,79,83,8,3,0,0,80,82,8,4,0,0,81,80,1,0,0,0,82,
  	85,1,0,0,0,83,81,1,0,0,0,83,84,1,0,0,0,84,17,1,0,0,0,85,83,1,0,0,0,86,
  	88,7,5,0,0,87,86,1,0,0,0,88,89,1,0,0,0,89,87,1,0,0,0,89,90,1,0,0,0,90,
  	91,1,0,0,0,91,92,6,8,1,0,92,19,1,0,0,0,93,94,5,62,0,0,94,95,5,61,0,0,
  	95,21,1,0,0,0,96,97,5,60,0,0,97,98,5,61,0,0,98,23,1,0,0,0,99,100,5,62,
  	0,0,100,25,1,0,0,0,101,102,5,60,0,0,102,27,1,0,0,0,103,104,5,61,0,0,104,
  	29,1,0,0,0,105,111,5,34,0,0,106,107,5,92,0,0,107,110,9,0,0,0,108,110,
  	8,2,0,0,109,106,1,0,0,0,109,108,1,0,0,0,110,113,1,0,0,0,111,109,1,0,0,
  	0,111,112,1,0,0,0,112,114,1,0,0,0,113,111,1,0,0,0,114,115,5,34,0,0,115,
  	116,1,0,0,0,116,117,6,14,2,0,117,31,1,0,0,0,118,122,8,6,0,0,119,121,8,
  	7,0,0,120,119,1,0,0,0,121,124,1,0,0,0,122,120,1,0,0,0,122,123,1,0,0,0,
  	123,125,1,0,0,0,124,122,1,0,0,0,125,126,6,15,2,0,126,33,1,0,0,0,127,129,
  	7,5,0,0,128,127,1,0,0,0,129,130,1,0,0,0,130,128,1,0,0,0,130,131,1,0,0,
  	0,131,132,1,0,0,0,132,133,6,16,1,0,133,134,6,16,2,0,134,35,1,0,0,0,14,
  	0,1,40,47,53,62,72,74,83,89,109,111,122,130,3,5,1,0,6,0,0,4,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  pqllexerLexerStaticData = std::move(staticData);
}

}

PqlLexer::PqlLexer(CharStream *input) : Lexer(input) {
  PqlLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *pqllexerLexerStaticData->atn, pqllexerLexerStaticData->decisionToDFA, pqllexerLexerStaticData->sharedContextCache);
}

PqlLexer::~PqlLexer() {
  delete _interpreter;
}

std::string PqlLexer::getGrammarFileName() const {
  return "PqlLexer.g4";
}

const std::vector<std::string>& PqlLexer::getRuleNames() const {
  return pqllexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& PqlLexer::getChannelNames() const {
  return pqllexerLexerStaticData->channelNames;
}

const std::vector<std::string>& PqlLexer::getModeNames() const {
  return pqllexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& PqlLexer::getVocabulary() const {
  return pqllexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView PqlLexer::getSerializedATN() const {
  return pqllexerLexerStaticData->serializedATN;
}

const atn::ATN& PqlLexer::getATN() const {
  return *pqllexerLexerStaticData->atn;
}




void PqlLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  pqllexerLexerInitialize();
#else
  ::antlr4::internal::call_once(pqllexerLexerOnceFlag, pqllexerLexerInitialize);
#endif
}

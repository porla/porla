
// Generated from PorlaQueryLang.g4 by ANTLR 4.13.2


#include "PorlaQueryLangLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct PorlaQueryLangLexerStaticData final {
  PorlaQueryLangLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PorlaQueryLangLexerStaticData(const PorlaQueryLangLexerStaticData&) = delete;
  PorlaQueryLangLexerStaticData(PorlaQueryLangLexerStaticData&&) = delete;
  PorlaQueryLangLexerStaticData& operator=(const PorlaQueryLangLexerStaticData&) = delete;
  PorlaQueryLangLexerStaticData& operator=(PorlaQueryLangLexerStaticData&&) = delete;

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

::antlr4::internal::OnceFlag porlaquerylanglexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<PorlaQueryLangLexerStaticData> porlaquerylanglexerLexerStaticData = nullptr;

void porlaquerylanglexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (porlaquerylanglexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(porlaquerylanglexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<PorlaQueryLangLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "OR", "AND", "NOT", "OPER_EQ", "OPER_GTE", "OPER_GT", 
      "OPER_LTE", "OPER_LT", "WHITESPACE", "FLOAT", "INT", "STRING", "QUALIFIER", 
      "ID"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'('", "')'", "", "", "", "'='", "'>='", "'>'", "'<='", "'<'"
    },
    std::vector<std::string>{
      "", "", "", "OR", "AND", "NOT", "OPER_EQ", "OPER_GTE", "OPER_GT", 
      "OPER_LTE", "OPER_LT", "WHITESPACE", "FLOAT", "INT", "STRING", "QUALIFIER", 
      "ID"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,16,125,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,1,0,1,0,1,1,1,1,1,2,1,2,1,2,1,2,3,2,42,8,2,1,3,1,3,1,3,
  	1,3,1,3,3,3,49,8,3,1,4,1,4,1,5,1,5,1,6,1,6,1,6,1,7,1,7,1,8,1,8,1,8,1,
  	9,1,9,1,10,4,10,66,8,10,11,10,12,10,67,1,10,1,10,1,11,3,11,73,8,11,1,
  	11,4,11,76,8,11,11,11,12,11,77,1,11,1,11,4,11,82,8,11,11,11,12,11,83,
  	1,12,3,12,87,8,12,1,12,4,12,90,8,12,11,12,12,12,91,1,13,1,13,5,13,96,
  	8,13,10,13,12,13,99,9,13,1,13,1,13,1,14,4,14,104,8,14,11,14,12,14,105,
  	1,14,1,14,1,15,4,15,111,8,15,11,15,12,15,112,1,15,1,15,4,15,117,8,15,
  	11,15,12,15,118,5,15,121,8,15,10,15,12,15,124,9,15,1,97,0,16,1,1,3,2,
  	5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,27,14,29,15,31,
  	16,1,0,4,2,0,33,33,45,45,3,0,9,10,13,13,32,32,1,0,48,57,3,0,65,90,95,
  	95,97,122,137,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,
  	0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,
  	0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,
  	1,0,0,0,1,33,1,0,0,0,3,35,1,0,0,0,5,41,1,0,0,0,7,48,1,0,0,0,9,50,1,0,
  	0,0,11,52,1,0,0,0,13,54,1,0,0,0,15,57,1,0,0,0,17,59,1,0,0,0,19,62,1,0,
  	0,0,21,65,1,0,0,0,23,72,1,0,0,0,25,86,1,0,0,0,27,93,1,0,0,0,29,103,1,
  	0,0,0,31,110,1,0,0,0,33,34,5,40,0,0,34,2,1,0,0,0,35,36,5,41,0,0,36,4,
  	1,0,0,0,37,38,5,79,0,0,38,42,5,82,0,0,39,40,5,124,0,0,40,42,5,124,0,0,
  	41,37,1,0,0,0,41,39,1,0,0,0,42,6,1,0,0,0,43,44,5,65,0,0,44,45,5,78,0,
  	0,45,49,5,68,0,0,46,47,5,38,0,0,47,49,5,38,0,0,48,43,1,0,0,0,48,46,1,
  	0,0,0,49,8,1,0,0,0,50,51,7,0,0,0,51,10,1,0,0,0,52,53,5,61,0,0,53,12,1,
  	0,0,0,54,55,5,62,0,0,55,56,5,61,0,0,56,14,1,0,0,0,57,58,5,62,0,0,58,16,
  	1,0,0,0,59,60,5,60,0,0,60,61,5,61,0,0,61,18,1,0,0,0,62,63,5,60,0,0,63,
  	20,1,0,0,0,64,66,7,1,0,0,65,64,1,0,0,0,66,67,1,0,0,0,67,65,1,0,0,0,67,
  	68,1,0,0,0,68,69,1,0,0,0,69,70,6,10,0,0,70,22,1,0,0,0,71,73,5,45,0,0,
  	72,71,1,0,0,0,72,73,1,0,0,0,73,75,1,0,0,0,74,76,7,2,0,0,75,74,1,0,0,0,
  	76,77,1,0,0,0,77,75,1,0,0,0,77,78,1,0,0,0,78,79,1,0,0,0,79,81,5,46,0,
  	0,80,82,7,2,0,0,81,80,1,0,0,0,82,83,1,0,0,0,83,81,1,0,0,0,83,84,1,0,0,
  	0,84,24,1,0,0,0,85,87,5,45,0,0,86,85,1,0,0,0,86,87,1,0,0,0,87,89,1,0,
  	0,0,88,90,7,2,0,0,89,88,1,0,0,0,90,91,1,0,0,0,91,89,1,0,0,0,91,92,1,0,
  	0,0,92,26,1,0,0,0,93,97,5,34,0,0,94,96,9,0,0,0,95,94,1,0,0,0,96,99,1,
  	0,0,0,97,98,1,0,0,0,97,95,1,0,0,0,98,100,1,0,0,0,99,97,1,0,0,0,100,101,
  	5,34,0,0,101,28,1,0,0,0,102,104,7,3,0,0,103,102,1,0,0,0,104,105,1,0,0,
  	0,105,103,1,0,0,0,105,106,1,0,0,0,106,107,1,0,0,0,107,108,5,58,0,0,108,
  	30,1,0,0,0,109,111,7,3,0,0,110,109,1,0,0,0,111,112,1,0,0,0,112,110,1,
  	0,0,0,112,113,1,0,0,0,113,122,1,0,0,0,114,116,5,45,0,0,115,117,7,3,0,
  	0,116,115,1,0,0,0,117,118,1,0,0,0,118,116,1,0,0,0,118,119,1,0,0,0,119,
  	121,1,0,0,0,120,114,1,0,0,0,121,124,1,0,0,0,122,120,1,0,0,0,122,123,1,
  	0,0,0,123,32,1,0,0,0,124,122,1,0,0,0,14,0,41,48,67,72,77,83,86,91,97,
  	105,112,118,122,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  porlaquerylanglexerLexerStaticData = std::move(staticData);
}

}

PorlaQueryLangLexer::PorlaQueryLangLexer(CharStream *input) : Lexer(input) {
  PorlaQueryLangLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *porlaquerylanglexerLexerStaticData->atn, porlaquerylanglexerLexerStaticData->decisionToDFA, porlaquerylanglexerLexerStaticData->sharedContextCache);
}

PorlaQueryLangLexer::~PorlaQueryLangLexer() {
  delete _interpreter;
}

std::string PorlaQueryLangLexer::getGrammarFileName() const {
  return "PorlaQueryLang.g4";
}

const std::vector<std::string>& PorlaQueryLangLexer::getRuleNames() const {
  return porlaquerylanglexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& PorlaQueryLangLexer::getChannelNames() const {
  return porlaquerylanglexerLexerStaticData->channelNames;
}

const std::vector<std::string>& PorlaQueryLangLexer::getModeNames() const {
  return porlaquerylanglexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& PorlaQueryLangLexer::getVocabulary() const {
  return porlaquerylanglexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView PorlaQueryLangLexer::getSerializedATN() const {
  return porlaquerylanglexerLexerStaticData->serializedATN;
}

const atn::ATN& PorlaQueryLangLexer::getATN() const {
  return *porlaquerylanglexerLexerStaticData->atn;
}




void PorlaQueryLangLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  porlaquerylanglexerLexerInitialize();
#else
  ::antlr4::internal::call_once(porlaquerylanglexerLexerOnceFlag, porlaquerylanglexerLexerInitialize);
#endif
}


// Generated from PorlaQueryLang.g4 by ANTLR 4.11.1


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
PorlaQueryLangLexerStaticData *porlaquerylanglexerLexerStaticData = nullptr;

void porlaquerylanglexerLexerInitialize() {
  assert(porlaquerylanglexerLexerStaticData == nullptr);
  auto staticData = std::make_unique<PorlaQueryLangLexerStaticData>(
    std::vector<std::string>{
      "T__0", "AND", "OR", "OPER_EQ", "OPER_CONTAINS", "OPER_GT", "OPER_GTE", 
      "OPER_LT", "OPER_LTE", "WHITESPACE", "INT", "STRING", "UNIT_DURATION", 
      "UNIT_SIZE", "UNIT_SPEED", "ID"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'is:'", "'and'", "'or'", "'='", "'contains'", "'>'", "'>='", 
      "'<'", "'<='"
    },
    std::vector<std::string>{
      "", "", "AND", "OR", "OPER_EQ", "OPER_CONTAINS", "OPER_GT", "OPER_GTE", 
      "OPER_LT", "OPER_LTE", "WHITESPACE", "INT", "STRING", "UNIT_DURATION", 
      "UNIT_SIZE", "UNIT_SPEED", "ID"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,16,134,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,2,1,3,1,3,1,
  	4,1,4,1,4,1,4,1,4,1,4,1,4,1,4,1,4,1,5,1,5,1,6,1,6,1,6,1,7,1,7,1,8,1,8,
  	1,8,1,9,4,9,67,8,9,11,9,12,9,68,1,9,1,9,1,10,3,10,74,8,10,1,10,4,10,77,
  	8,10,11,10,12,10,78,1,11,1,11,5,11,83,8,11,10,11,12,11,86,9,11,1,11,1,
  	11,1,12,1,12,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,3,
  	13,103,8,13,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,
  	14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,3,14,128,8,
  	14,1,15,4,15,131,8,15,11,15,12,15,132,1,84,0,16,1,1,3,2,5,3,7,4,9,5,11,
  	6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,27,14,29,15,31,16,1,0,4,3,0,
  	9,10,13,13,32,32,1,0,48,57,5,0,100,100,104,104,109,109,115,115,119,119,
  	2,0,65,90,97,122,148,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,
  	0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,
  	1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,
  	0,0,0,31,1,0,0,0,1,33,1,0,0,0,3,37,1,0,0,0,5,41,1,0,0,0,7,44,1,0,0,0,
  	9,46,1,0,0,0,11,55,1,0,0,0,13,57,1,0,0,0,15,60,1,0,0,0,17,62,1,0,0,0,
  	19,66,1,0,0,0,21,73,1,0,0,0,23,80,1,0,0,0,25,89,1,0,0,0,27,102,1,0,0,
  	0,29,127,1,0,0,0,31,130,1,0,0,0,33,34,5,105,0,0,34,35,5,115,0,0,35,36,
  	5,58,0,0,36,2,1,0,0,0,37,38,5,97,0,0,38,39,5,110,0,0,39,40,5,100,0,0,
  	40,4,1,0,0,0,41,42,5,111,0,0,42,43,5,114,0,0,43,6,1,0,0,0,44,45,5,61,
  	0,0,45,8,1,0,0,0,46,47,5,99,0,0,47,48,5,111,0,0,48,49,5,110,0,0,49,50,
  	5,116,0,0,50,51,5,97,0,0,51,52,5,105,0,0,52,53,5,110,0,0,53,54,5,115,
  	0,0,54,10,1,0,0,0,55,56,5,62,0,0,56,12,1,0,0,0,57,58,5,62,0,0,58,59,5,
  	61,0,0,59,14,1,0,0,0,60,61,5,60,0,0,61,16,1,0,0,0,62,63,5,60,0,0,63,64,
  	5,61,0,0,64,18,1,0,0,0,65,67,7,0,0,0,66,65,1,0,0,0,67,68,1,0,0,0,68,66,
  	1,0,0,0,68,69,1,0,0,0,69,70,1,0,0,0,70,71,6,9,0,0,71,20,1,0,0,0,72,74,
  	5,45,0,0,73,72,1,0,0,0,73,74,1,0,0,0,74,76,1,0,0,0,75,77,7,1,0,0,76,75,
  	1,0,0,0,77,78,1,0,0,0,78,76,1,0,0,0,78,79,1,0,0,0,79,22,1,0,0,0,80,84,
  	5,34,0,0,81,83,9,0,0,0,82,81,1,0,0,0,83,86,1,0,0,0,84,85,1,0,0,0,84,82,
  	1,0,0,0,85,87,1,0,0,0,86,84,1,0,0,0,87,88,5,34,0,0,88,24,1,0,0,0,89,90,
  	7,2,0,0,90,26,1,0,0,0,91,103,5,98,0,0,92,93,5,107,0,0,93,103,5,98,0,0,
  	94,95,5,109,0,0,95,103,5,98,0,0,96,97,5,103,0,0,97,103,5,98,0,0,98,99,
  	5,116,0,0,99,103,5,98,0,0,100,101,5,112,0,0,101,103,5,98,0,0,102,91,1,
  	0,0,0,102,92,1,0,0,0,102,94,1,0,0,0,102,96,1,0,0,0,102,98,1,0,0,0,102,
  	100,1,0,0,0,103,28,1,0,0,0,104,105,5,98,0,0,105,106,5,112,0,0,106,128,
  	5,115,0,0,107,108,5,107,0,0,108,109,5,98,0,0,109,110,5,112,0,0,110,128,
  	5,115,0,0,111,112,5,109,0,0,112,113,5,98,0,0,113,114,5,112,0,0,114,128,
  	5,115,0,0,115,116,5,103,0,0,116,117,5,98,0,0,117,118,5,112,0,0,118,128,
  	5,115,0,0,119,120,5,116,0,0,120,121,5,98,0,0,121,122,5,112,0,0,122,128,
  	5,115,0,0,123,124,5,112,0,0,124,125,5,98,0,0,125,126,5,112,0,0,126,128,
  	5,115,0,0,127,104,1,0,0,0,127,107,1,0,0,0,127,111,1,0,0,0,127,115,1,0,
  	0,0,127,119,1,0,0,0,127,123,1,0,0,0,128,30,1,0,0,0,129,131,7,3,0,0,130,
  	129,1,0,0,0,131,132,1,0,0,0,132,130,1,0,0,0,132,133,1,0,0,0,133,32,1,
  	0,0,0,8,0,68,73,78,84,102,127,132,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  porlaquerylanglexerLexerStaticData = staticData.release();
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
  ::antlr4::internal::call_once(porlaquerylanglexerLexerOnceFlag, porlaquerylanglexerLexerInitialize);
}

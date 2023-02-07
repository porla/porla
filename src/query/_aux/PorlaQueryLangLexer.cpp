
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
      "OPER_LT", "OPER_LTE", "WHITESPACE", "INT", "FLOAT", "STRING", "UNIT_DURATION", 
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
      "OPER_LT", "OPER_LTE", "WHITESPACE", "INT", "FLOAT", "STRING", "UNIT_DURATION", 
      "UNIT_SIZE", "UNIT_SPEED", "ID"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,17,142,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,2,
  	1,3,1,3,1,4,1,4,1,4,1,4,1,4,1,4,1,4,1,4,1,4,1,5,1,5,1,6,1,6,1,6,1,7,1,
  	7,1,8,1,8,1,8,1,9,4,9,69,8,9,11,9,12,9,70,1,9,1,9,1,10,3,10,76,8,10,1,
  	10,4,10,79,8,10,11,10,12,10,80,1,11,3,11,84,8,11,1,11,4,11,87,8,11,11,
  	11,12,11,88,1,11,1,11,4,11,93,8,11,11,11,12,11,94,1,12,1,12,5,12,99,8,
  	12,10,12,12,12,102,9,12,1,12,1,12,1,13,1,13,1,14,1,14,1,14,1,14,1,14,
  	1,14,1,14,1,14,1,14,1,14,1,14,3,14,119,8,14,1,15,1,15,1,15,1,15,1,15,
  	1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,3,15,136,8,15,1,16,
  	4,16,139,8,16,11,16,12,16,140,1,100,0,17,1,1,3,2,5,3,7,4,9,5,11,6,13,
  	7,15,8,17,9,19,10,21,11,23,12,25,13,27,14,29,15,31,16,33,17,1,0,4,3,0,
  	9,10,13,13,32,32,1,0,48,57,5,0,100,100,104,104,109,109,115,115,119,119,
  	3,0,65,90,95,95,97,122,157,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,
  	0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,
  	0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,
  	1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,1,35,1,0,0,0,3,39,1,0,0,0,5,43,1,0,
  	0,0,7,46,1,0,0,0,9,48,1,0,0,0,11,57,1,0,0,0,13,59,1,0,0,0,15,62,1,0,0,
  	0,17,64,1,0,0,0,19,68,1,0,0,0,21,75,1,0,0,0,23,83,1,0,0,0,25,96,1,0,0,
  	0,27,105,1,0,0,0,29,118,1,0,0,0,31,135,1,0,0,0,33,138,1,0,0,0,35,36,5,
  	105,0,0,36,37,5,115,0,0,37,38,5,58,0,0,38,2,1,0,0,0,39,40,5,97,0,0,40,
  	41,5,110,0,0,41,42,5,100,0,0,42,4,1,0,0,0,43,44,5,111,0,0,44,45,5,114,
  	0,0,45,6,1,0,0,0,46,47,5,61,0,0,47,8,1,0,0,0,48,49,5,99,0,0,49,50,5,111,
  	0,0,50,51,5,110,0,0,51,52,5,116,0,0,52,53,5,97,0,0,53,54,5,105,0,0,54,
  	55,5,110,0,0,55,56,5,115,0,0,56,10,1,0,0,0,57,58,5,62,0,0,58,12,1,0,0,
  	0,59,60,5,62,0,0,60,61,5,61,0,0,61,14,1,0,0,0,62,63,5,60,0,0,63,16,1,
  	0,0,0,64,65,5,60,0,0,65,66,5,61,0,0,66,18,1,0,0,0,67,69,7,0,0,0,68,67,
  	1,0,0,0,69,70,1,0,0,0,70,68,1,0,0,0,70,71,1,0,0,0,71,72,1,0,0,0,72,73,
  	6,9,0,0,73,20,1,0,0,0,74,76,5,45,0,0,75,74,1,0,0,0,75,76,1,0,0,0,76,78,
  	1,0,0,0,77,79,7,1,0,0,78,77,1,0,0,0,79,80,1,0,0,0,80,78,1,0,0,0,80,81,
  	1,0,0,0,81,22,1,0,0,0,82,84,5,45,0,0,83,82,1,0,0,0,83,84,1,0,0,0,84,86,
  	1,0,0,0,85,87,7,1,0,0,86,85,1,0,0,0,87,88,1,0,0,0,88,86,1,0,0,0,88,89,
  	1,0,0,0,89,90,1,0,0,0,90,92,5,46,0,0,91,93,7,1,0,0,92,91,1,0,0,0,93,94,
  	1,0,0,0,94,92,1,0,0,0,94,95,1,0,0,0,95,24,1,0,0,0,96,100,5,34,0,0,97,
  	99,9,0,0,0,98,97,1,0,0,0,99,102,1,0,0,0,100,101,1,0,0,0,100,98,1,0,0,
  	0,101,103,1,0,0,0,102,100,1,0,0,0,103,104,5,34,0,0,104,26,1,0,0,0,105,
  	106,7,2,0,0,106,28,1,0,0,0,107,119,5,98,0,0,108,109,5,107,0,0,109,119,
  	5,98,0,0,110,111,5,109,0,0,111,119,5,98,0,0,112,113,5,103,0,0,113,119,
  	5,98,0,0,114,115,5,116,0,0,115,119,5,98,0,0,116,117,5,112,0,0,117,119,
  	5,98,0,0,118,107,1,0,0,0,118,108,1,0,0,0,118,110,1,0,0,0,118,112,1,0,
  	0,0,118,114,1,0,0,0,118,116,1,0,0,0,119,30,1,0,0,0,120,121,5,98,0,0,121,
  	122,5,112,0,0,122,136,5,115,0,0,123,124,5,107,0,0,124,125,5,98,0,0,125,
  	126,5,112,0,0,126,136,5,115,0,0,127,128,5,109,0,0,128,129,5,98,0,0,129,
  	130,5,112,0,0,130,136,5,115,0,0,131,132,5,103,0,0,132,133,5,98,0,0,133,
  	134,5,112,0,0,134,136,5,115,0,0,135,120,1,0,0,0,135,123,1,0,0,0,135,127,
  	1,0,0,0,135,131,1,0,0,0,136,32,1,0,0,0,137,139,7,3,0,0,138,137,1,0,0,
  	0,139,140,1,0,0,0,140,138,1,0,0,0,140,141,1,0,0,0,141,34,1,0,0,0,11,0,
  	70,75,80,83,88,94,100,118,135,140,1,6,0,0
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


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
      "T__0", "AND", "OR", "NOT", "OPER_EQ", "OPER_CONTAINS", "OPER_GT", 
      "OPER_GTE", "OPER_LT", "OPER_LTE", "WHITESPACE", "INT", "FLOAT", "STRING", 
      "UNIT_DURATION", "UNIT_SIZE", "UNIT_SPEED", "ID"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'is:'", "'and'", "'or'", "'not'", "'='", "'contains'", "'>'", 
      "'>='", "'<'", "'<='"
    },
    std::vector<std::string>{
      "", "", "AND", "OR", "NOT", "OPER_EQ", "OPER_CONTAINS", "OPER_GT", 
      "OPER_GTE", "OPER_LT", "OPER_LTE", "WHITESPACE", "INT", "FLOAT", "STRING", 
      "UNIT_DURATION", "UNIT_SIZE", "UNIT_SPEED", "ID"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,18,148,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,
  	2,1,2,1,2,1,3,1,3,1,3,1,3,1,4,1,4,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,
  	1,6,1,6,1,7,1,7,1,7,1,8,1,8,1,9,1,9,1,9,1,10,4,10,75,8,10,11,10,12,10,
  	76,1,10,1,10,1,11,3,11,82,8,11,1,11,4,11,85,8,11,11,11,12,11,86,1,12,
  	3,12,90,8,12,1,12,4,12,93,8,12,11,12,12,12,94,1,12,1,12,4,12,99,8,12,
  	11,12,12,12,100,1,13,1,13,5,13,105,8,13,10,13,12,13,108,9,13,1,13,1,13,
  	1,14,1,14,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,3,15,
  	125,8,15,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,
  	1,16,1,16,1,16,3,16,142,8,16,1,17,4,17,145,8,17,11,17,12,17,146,1,106,
  	0,18,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,
  	27,14,29,15,31,16,33,17,35,18,1,0,4,3,0,9,10,13,13,32,32,1,0,48,57,5,
  	0,100,100,104,104,109,109,115,115,119,119,3,0,65,90,95,95,97,122,163,
  	0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,
  	0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,
  	0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,
  	1,0,0,0,0,35,1,0,0,0,1,37,1,0,0,0,3,41,1,0,0,0,5,45,1,0,0,0,7,48,1,0,
  	0,0,9,52,1,0,0,0,11,54,1,0,0,0,13,63,1,0,0,0,15,65,1,0,0,0,17,68,1,0,
  	0,0,19,70,1,0,0,0,21,74,1,0,0,0,23,81,1,0,0,0,25,89,1,0,0,0,27,102,1,
  	0,0,0,29,111,1,0,0,0,31,124,1,0,0,0,33,141,1,0,0,0,35,144,1,0,0,0,37,
  	38,5,105,0,0,38,39,5,115,0,0,39,40,5,58,0,0,40,2,1,0,0,0,41,42,5,97,0,
  	0,42,43,5,110,0,0,43,44,5,100,0,0,44,4,1,0,0,0,45,46,5,111,0,0,46,47,
  	5,114,0,0,47,6,1,0,0,0,48,49,5,110,0,0,49,50,5,111,0,0,50,51,5,116,0,
  	0,51,8,1,0,0,0,52,53,5,61,0,0,53,10,1,0,0,0,54,55,5,99,0,0,55,56,5,111,
  	0,0,56,57,5,110,0,0,57,58,5,116,0,0,58,59,5,97,0,0,59,60,5,105,0,0,60,
  	61,5,110,0,0,61,62,5,115,0,0,62,12,1,0,0,0,63,64,5,62,0,0,64,14,1,0,0,
  	0,65,66,5,62,0,0,66,67,5,61,0,0,67,16,1,0,0,0,68,69,5,60,0,0,69,18,1,
  	0,0,0,70,71,5,60,0,0,71,72,5,61,0,0,72,20,1,0,0,0,73,75,7,0,0,0,74,73,
  	1,0,0,0,75,76,1,0,0,0,76,74,1,0,0,0,76,77,1,0,0,0,77,78,1,0,0,0,78,79,
  	6,10,0,0,79,22,1,0,0,0,80,82,5,45,0,0,81,80,1,0,0,0,81,82,1,0,0,0,82,
  	84,1,0,0,0,83,85,7,1,0,0,84,83,1,0,0,0,85,86,1,0,0,0,86,84,1,0,0,0,86,
  	87,1,0,0,0,87,24,1,0,0,0,88,90,5,45,0,0,89,88,1,0,0,0,89,90,1,0,0,0,90,
  	92,1,0,0,0,91,93,7,1,0,0,92,91,1,0,0,0,93,94,1,0,0,0,94,92,1,0,0,0,94,
  	95,1,0,0,0,95,96,1,0,0,0,96,98,5,46,0,0,97,99,7,1,0,0,98,97,1,0,0,0,99,
  	100,1,0,0,0,100,98,1,0,0,0,100,101,1,0,0,0,101,26,1,0,0,0,102,106,5,34,
  	0,0,103,105,9,0,0,0,104,103,1,0,0,0,105,108,1,0,0,0,106,107,1,0,0,0,106,
  	104,1,0,0,0,107,109,1,0,0,0,108,106,1,0,0,0,109,110,5,34,0,0,110,28,1,
  	0,0,0,111,112,7,2,0,0,112,30,1,0,0,0,113,125,5,98,0,0,114,115,5,107,0,
  	0,115,125,5,98,0,0,116,117,5,109,0,0,117,125,5,98,0,0,118,119,5,103,0,
  	0,119,125,5,98,0,0,120,121,5,116,0,0,121,125,5,98,0,0,122,123,5,112,0,
  	0,123,125,5,98,0,0,124,113,1,0,0,0,124,114,1,0,0,0,124,116,1,0,0,0,124,
  	118,1,0,0,0,124,120,1,0,0,0,124,122,1,0,0,0,125,32,1,0,0,0,126,127,5,
  	98,0,0,127,128,5,112,0,0,128,142,5,115,0,0,129,130,5,107,0,0,130,131,
  	5,98,0,0,131,132,5,112,0,0,132,142,5,115,0,0,133,134,5,109,0,0,134,135,
  	5,98,0,0,135,136,5,112,0,0,136,142,5,115,0,0,137,138,5,103,0,0,138,139,
  	5,98,0,0,139,140,5,112,0,0,140,142,5,115,0,0,141,126,1,0,0,0,141,129,
  	1,0,0,0,141,133,1,0,0,0,141,137,1,0,0,0,142,34,1,0,0,0,143,145,7,3,0,
  	0,144,143,1,0,0,0,145,146,1,0,0,0,146,144,1,0,0,0,146,147,1,0,0,0,147,
  	36,1,0,0,0,11,0,76,81,86,89,94,100,106,124,141,146,1,6,0,0
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


// Generated from src/query/PqlParser.g4 by ANTLR 4.13.2



#include "PqlParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct PqlParserStaticData final {
  PqlParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PqlParserStaticData(const PqlParserStaticData&) = delete;
  PqlParserStaticData(PqlParserStaticData&&) = delete;
  PqlParserStaticData& operator=(const PqlParserStaticData&) = delete;
  PqlParserStaticData& operator=(PqlParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag pqlparserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<PqlParserStaticData> pqlparserParserStaticData = nullptr;

void pqlparserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (pqlparserParserStaticData != nullptr) {
    return;
  }
#else
  assert(pqlparserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<PqlParserStaticData>(
    std::vector<std::string>{
      "query", "orExpr", "andExpr", "unary", "primary", "op", "fieldValue"
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
  	4,1,17,59,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,1,0,
  	3,0,16,8,0,1,0,1,0,1,1,1,1,1,1,5,1,23,8,1,10,1,12,1,26,9,1,1,2,1,2,3,
  	2,30,8,2,1,2,5,2,33,8,2,10,2,12,2,36,9,2,1,3,1,3,1,3,3,3,41,8,3,1,4,1,
  	4,1,4,1,4,1,4,1,4,3,4,49,8,4,1,4,1,4,3,4,53,8,4,1,5,1,5,1,6,1,6,1,6,0,
  	0,7,0,2,4,6,8,10,12,0,3,1,0,7,8,1,0,10,14,1,0,15,16,59,0,15,1,0,0,0,2,
  	19,1,0,0,0,4,27,1,0,0,0,6,40,1,0,0,0,8,52,1,0,0,0,10,54,1,0,0,0,12,56,
  	1,0,0,0,14,16,3,2,1,0,15,14,1,0,0,0,15,16,1,0,0,0,16,17,1,0,0,0,17,18,
  	5,0,0,1,18,1,1,0,0,0,19,24,3,4,2,0,20,21,5,1,0,0,21,23,3,4,2,0,22,20,
  	1,0,0,0,23,26,1,0,0,0,24,22,1,0,0,0,24,25,1,0,0,0,25,3,1,0,0,0,26,24,
  	1,0,0,0,27,34,3,6,3,0,28,30,5,2,0,0,29,28,1,0,0,0,29,30,1,0,0,0,30,31,
  	1,0,0,0,31,33,3,6,3,0,32,29,1,0,0,0,33,36,1,0,0,0,34,32,1,0,0,0,34,35,
  	1,0,0,0,35,5,1,0,0,0,36,34,1,0,0,0,37,38,5,3,0,0,38,41,3,6,3,0,39,41,
  	3,8,4,0,40,37,1,0,0,0,40,39,1,0,0,0,41,7,1,0,0,0,42,43,5,4,0,0,43,44,
  	3,2,1,0,44,45,5,5,0,0,45,53,1,0,0,0,46,48,5,6,0,0,47,49,3,10,5,0,48,47,
  	1,0,0,0,48,49,1,0,0,0,49,50,1,0,0,0,50,53,3,12,6,0,51,53,7,0,0,0,52,42,
  	1,0,0,0,52,46,1,0,0,0,52,51,1,0,0,0,53,9,1,0,0,0,54,55,7,1,0,0,55,11,
  	1,0,0,0,56,57,7,2,0,0,57,13,1,0,0,0,7,15,24,29,34,40,48,52
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  pqlparserParserStaticData = std::move(staticData);
}

}

PqlParser::PqlParser(TokenStream *input) : PqlParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

PqlParser::PqlParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  PqlParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *pqlparserParserStaticData->atn, pqlparserParserStaticData->decisionToDFA, pqlparserParserStaticData->sharedContextCache, options);
}

PqlParser::~PqlParser() {
  delete _interpreter;
}

const atn::ATN& PqlParser::getATN() const {
  return *pqlparserParserStaticData->atn;
}

std::string PqlParser::getGrammarFileName() const {
  return "PqlParser.g4";
}

const std::vector<std::string>& PqlParser::getRuleNames() const {
  return pqlparserParserStaticData->ruleNames;
}

const dfa::Vocabulary& PqlParser::getVocabulary() const {
  return pqlparserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView PqlParser::getSerializedATN() const {
  return pqlparserParserStaticData->serializedATN;
}


//----------------- QueryContext ------------------------------------------------------------------

PqlParser::QueryContext::QueryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PqlParser::QueryContext::EOF() {
  return getToken(PqlParser::EOF, 0);
}

PqlParser::OrExprContext* PqlParser::QueryContext::orExpr() {
  return getRuleContext<PqlParser::OrExprContext>(0);
}


size_t PqlParser::QueryContext::getRuleIndex() const {
  return PqlParser::RuleQuery;
}


PqlParser::QueryContext* PqlParser::query() {
  QueryContext *_localctx = _tracker.createInstance<QueryContext>(_ctx, getState());
  enterRule(_localctx, 0, PqlParser::RuleQuery);
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
    setState(15);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 472) != 0)) {
      setState(14);
      orExpr();
    }
    setState(17);
    match(PqlParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OrExprContext ------------------------------------------------------------------

PqlParser::OrExprContext::OrExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PqlParser::AndExprContext *> PqlParser::OrExprContext::andExpr() {
  return getRuleContexts<PqlParser::AndExprContext>();
}

PqlParser::AndExprContext* PqlParser::OrExprContext::andExpr(size_t i) {
  return getRuleContext<PqlParser::AndExprContext>(i);
}

std::vector<tree::TerminalNode *> PqlParser::OrExprContext::OR() {
  return getTokens(PqlParser::OR);
}

tree::TerminalNode* PqlParser::OrExprContext::OR(size_t i) {
  return getToken(PqlParser::OR, i);
}


size_t PqlParser::OrExprContext::getRuleIndex() const {
  return PqlParser::RuleOrExpr;
}


PqlParser::OrExprContext* PqlParser::orExpr() {
  OrExprContext *_localctx = _tracker.createInstance<OrExprContext>(_ctx, getState());
  enterRule(_localctx, 2, PqlParser::RuleOrExpr);
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
    setState(19);
    andExpr();
    setState(24);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == PqlParser::OR) {
      setState(20);
      match(PqlParser::OR);
      setState(21);
      andExpr();
      setState(26);
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

//----------------- AndExprContext ------------------------------------------------------------------

PqlParser::AndExprContext::AndExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PqlParser::UnaryContext *> PqlParser::AndExprContext::unary() {
  return getRuleContexts<PqlParser::UnaryContext>();
}

PqlParser::UnaryContext* PqlParser::AndExprContext::unary(size_t i) {
  return getRuleContext<PqlParser::UnaryContext>(i);
}

std::vector<tree::TerminalNode *> PqlParser::AndExprContext::AND() {
  return getTokens(PqlParser::AND);
}

tree::TerminalNode* PqlParser::AndExprContext::AND(size_t i) {
  return getToken(PqlParser::AND, i);
}


size_t PqlParser::AndExprContext::getRuleIndex() const {
  return PqlParser::RuleAndExpr;
}


PqlParser::AndExprContext* PqlParser::andExpr() {
  AndExprContext *_localctx = _tracker.createInstance<AndExprContext>(_ctx, getState());
  enterRule(_localctx, 4, PqlParser::RuleAndExpr);
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
    setState(27);
    unary();
    setState(34);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 476) != 0)) {
      setState(29);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == PqlParser::AND) {
        setState(28);
        match(PqlParser::AND);
      }
      setState(31);
      unary();
      setState(36);
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

//----------------- UnaryContext ------------------------------------------------------------------

PqlParser::UnaryContext::UnaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PqlParser::UnaryContext::getRuleIndex() const {
  return PqlParser::RuleUnary;
}

void PqlParser::UnaryContext::copyFrom(UnaryContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- PrimaryExprContext ------------------------------------------------------------------

PqlParser::PrimaryContext* PqlParser::PrimaryExprContext::primary() {
  return getRuleContext<PqlParser::PrimaryContext>(0);
}

PqlParser::PrimaryExprContext::PrimaryExprContext(UnaryContext *ctx) { copyFrom(ctx); }


//----------------- NotExprContext ------------------------------------------------------------------

tree::TerminalNode* PqlParser::NotExprContext::NOT() {
  return getToken(PqlParser::NOT, 0);
}

PqlParser::UnaryContext* PqlParser::NotExprContext::unary() {
  return getRuleContext<PqlParser::UnaryContext>(0);
}

PqlParser::NotExprContext::NotExprContext(UnaryContext *ctx) { copyFrom(ctx); }


PqlParser::UnaryContext* PqlParser::unary() {
  UnaryContext *_localctx = _tracker.createInstance<UnaryContext>(_ctx, getState());
  enterRule(_localctx, 6, PqlParser::RuleUnary);

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
      case PqlParser::NOT: {
        _localctx = _tracker.createInstance<PqlParser::NotExprContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(37);
        match(PqlParser::NOT);
        setState(38);
        unary();
        break;
      }

      case PqlParser::LPAREN:
      case PqlParser::FIELD:
      case PqlParser::STRING:
      case PqlParser::WORD: {
        _localctx = _tracker.createInstance<PqlParser::PrimaryExprContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(39);
        primary();
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

//----------------- PrimaryContext ------------------------------------------------------------------

PqlParser::PrimaryContext::PrimaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PqlParser::PrimaryContext::getRuleIndex() const {
  return PqlParser::RulePrimary;
}

void PqlParser::PrimaryContext::copyFrom(PrimaryContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- GroupExprContext ------------------------------------------------------------------

tree::TerminalNode* PqlParser::GroupExprContext::LPAREN() {
  return getToken(PqlParser::LPAREN, 0);
}

PqlParser::OrExprContext* PqlParser::GroupExprContext::orExpr() {
  return getRuleContext<PqlParser::OrExprContext>(0);
}

tree::TerminalNode* PqlParser::GroupExprContext::RPAREN() {
  return getToken(PqlParser::RPAREN, 0);
}

PqlParser::GroupExprContext::GroupExprContext(PrimaryContext *ctx) { copyFrom(ctx); }


//----------------- TextExprContext ------------------------------------------------------------------

tree::TerminalNode* PqlParser::TextExprContext::WORD() {
  return getToken(PqlParser::WORD, 0);
}

tree::TerminalNode* PqlParser::TextExprContext::STRING() {
  return getToken(PqlParser::STRING, 0);
}

PqlParser::TextExprContext::TextExprContext(PrimaryContext *ctx) { copyFrom(ctx); }


//----------------- QualifierExprContext ------------------------------------------------------------------

tree::TerminalNode* PqlParser::QualifierExprContext::FIELD() {
  return getToken(PqlParser::FIELD, 0);
}

PqlParser::FieldValueContext* PqlParser::QualifierExprContext::fieldValue() {
  return getRuleContext<PqlParser::FieldValueContext>(0);
}

PqlParser::OpContext* PqlParser::QualifierExprContext::op() {
  return getRuleContext<PqlParser::OpContext>(0);
}

PqlParser::QualifierExprContext::QualifierExprContext(PrimaryContext *ctx) { copyFrom(ctx); }


PqlParser::PrimaryContext* PqlParser::primary() {
  PrimaryContext *_localctx = _tracker.createInstance<PrimaryContext>(_ctx, getState());
  enterRule(_localctx, 8, PqlParser::RulePrimary);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(52);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PqlParser::LPAREN: {
        _localctx = _tracker.createInstance<PqlParser::GroupExprContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(42);
        match(PqlParser::LPAREN);
        setState(43);
        orExpr();
        setState(44);
        match(PqlParser::RPAREN);
        break;
      }

      case PqlParser::FIELD: {
        _localctx = _tracker.createInstance<PqlParser::QualifierExprContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(46);
        match(PqlParser::FIELD);
        setState(48);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 31744) != 0)) {
          setState(47);
          op();
        }
        setState(50);
        fieldValue();
        break;
      }

      case PqlParser::STRING:
      case PqlParser::WORD: {
        _localctx = _tracker.createInstance<PqlParser::TextExprContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(51);
        _la = _input->LA(1);
        if (!(_la == PqlParser::STRING

        || _la == PqlParser::WORD)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
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

//----------------- OpContext ------------------------------------------------------------------

PqlParser::OpContext::OpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PqlParser::OpContext::V_GTE() {
  return getToken(PqlParser::V_GTE, 0);
}

tree::TerminalNode* PqlParser::OpContext::V_LTE() {
  return getToken(PqlParser::V_LTE, 0);
}

tree::TerminalNode* PqlParser::OpContext::V_GT() {
  return getToken(PqlParser::V_GT, 0);
}

tree::TerminalNode* PqlParser::OpContext::V_LT() {
  return getToken(PqlParser::V_LT, 0);
}

tree::TerminalNode* PqlParser::OpContext::V_EQ() {
  return getToken(PqlParser::V_EQ, 0);
}


size_t PqlParser::OpContext::getRuleIndex() const {
  return PqlParser::RuleOp;
}


PqlParser::OpContext* PqlParser::op() {
  OpContext *_localctx = _tracker.createInstance<OpContext>(_ctx, getState());
  enterRule(_localctx, 10, PqlParser::RuleOp);
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
    setState(54);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 31744) != 0))) {
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

//----------------- FieldValueContext ------------------------------------------------------------------

PqlParser::FieldValueContext::FieldValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PqlParser::FieldValueContext::V_WORD() {
  return getToken(PqlParser::V_WORD, 0);
}

tree::TerminalNode* PqlParser::FieldValueContext::V_STRING() {
  return getToken(PqlParser::V_STRING, 0);
}


size_t PqlParser::FieldValueContext::getRuleIndex() const {
  return PqlParser::RuleFieldValue;
}


PqlParser::FieldValueContext* PqlParser::fieldValue() {
  FieldValueContext *_localctx = _tracker.createInstance<FieldValueContext>(_ctx, getState());
  enterRule(_localctx, 12, PqlParser::RuleFieldValue);
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
    setState(56);
    _la = _input->LA(1);
    if (!(_la == PqlParser::V_STRING

    || _la == PqlParser::V_WORD)) {
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

void PqlParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  pqlparserParserInitialize();
#else
  ::antlr4::internal::call_once(pqlparserParserOnceFlag, pqlparserParserInitialize);
#endif
}

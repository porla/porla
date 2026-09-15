
// Generated from PorlaQueryLang.g4 by ANTLR 4.13.2


#include "PorlaQueryLangVisitor.h"

#include "PorlaQueryLangParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct PorlaQueryLangParserStaticData final {
  PorlaQueryLangParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  PorlaQueryLangParserStaticData(const PorlaQueryLangParserStaticData&) = delete;
  PorlaQueryLangParserStaticData(PorlaQueryLangParserStaticData&&) = delete;
  PorlaQueryLangParserStaticData& operator=(const PorlaQueryLangParserStaticData&) = delete;
  PorlaQueryLangParserStaticData& operator=(PorlaQueryLangParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag porlaquerylangParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<PorlaQueryLangParserStaticData> porlaquerylangParserStaticData = nullptr;

void porlaquerylangParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (porlaquerylangParserStaticData != nullptr) {
    return;
  }
#else
  assert(porlaquerylangParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<PorlaQueryLangParserStaticData>(
    std::vector<std::string>{
      "filter", "orExpr", "andExpr", "term", "qualifier", "operator", "value", 
      "text"
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
  	4,1,16,66,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,7,
  	7,7,1,0,1,0,1,0,1,1,1,1,1,1,5,1,23,8,1,10,1,12,1,26,9,1,1,2,1,2,3,2,30,
  	8,2,1,2,5,2,33,8,2,10,2,12,2,36,9,2,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,3,
  	3,46,8,3,1,4,1,4,3,4,50,8,4,1,4,1,4,1,5,1,5,1,6,1,6,3,6,58,8,6,1,6,1,
  	6,3,6,62,8,6,1,7,1,7,1,7,0,0,8,0,2,4,6,8,10,12,14,0,3,1,0,6,10,1,0,12,
  	13,2,0,12,14,16,16,67,0,16,1,0,0,0,2,19,1,0,0,0,4,27,1,0,0,0,6,45,1,0,
  	0,0,8,47,1,0,0,0,10,53,1,0,0,0,12,61,1,0,0,0,14,63,1,0,0,0,16,17,3,2,
  	1,0,17,18,5,0,0,1,18,1,1,0,0,0,19,24,3,4,2,0,20,21,5,3,0,0,21,23,3,4,
  	2,0,22,20,1,0,0,0,23,26,1,0,0,0,24,22,1,0,0,0,24,25,1,0,0,0,25,3,1,0,
  	0,0,26,24,1,0,0,0,27,34,3,6,3,0,28,30,5,4,0,0,29,28,1,0,0,0,29,30,1,0,
  	0,0,30,31,1,0,0,0,31,33,3,6,3,0,32,29,1,0,0,0,33,36,1,0,0,0,34,32,1,0,
  	0,0,34,35,1,0,0,0,35,5,1,0,0,0,36,34,1,0,0,0,37,38,5,5,0,0,38,46,3,6,
  	3,0,39,40,5,1,0,0,40,41,3,2,1,0,41,42,5,2,0,0,42,46,1,0,0,0,43,46,3,8,
  	4,0,44,46,3,14,7,0,45,37,1,0,0,0,45,39,1,0,0,0,45,43,1,0,0,0,45,44,1,
  	0,0,0,46,7,1,0,0,0,47,49,5,15,0,0,48,50,3,10,5,0,49,48,1,0,0,0,49,50,
  	1,0,0,0,50,51,1,0,0,0,51,52,3,12,6,0,52,9,1,0,0,0,53,54,7,0,0,0,54,11,
  	1,0,0,0,55,57,7,1,0,0,56,58,5,16,0,0,57,56,1,0,0,0,57,58,1,0,0,0,58,62,
  	1,0,0,0,59,62,5,14,0,0,60,62,5,16,0,0,61,55,1,0,0,0,61,59,1,0,0,0,61,
  	60,1,0,0,0,62,13,1,0,0,0,63,64,7,2,0,0,64,15,1,0,0,0,7,24,29,34,45,49,
  	57,61
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  porlaquerylangParserStaticData = std::move(staticData);
}

}

PorlaQueryLangParser::PorlaQueryLangParser(TokenStream *input) : PorlaQueryLangParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

PorlaQueryLangParser::PorlaQueryLangParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  PorlaQueryLangParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *porlaquerylangParserStaticData->atn, porlaquerylangParserStaticData->decisionToDFA, porlaquerylangParserStaticData->sharedContextCache, options);
}

PorlaQueryLangParser::~PorlaQueryLangParser() {
  delete _interpreter;
}

const atn::ATN& PorlaQueryLangParser::getATN() const {
  return *porlaquerylangParserStaticData->atn;
}

std::string PorlaQueryLangParser::getGrammarFileName() const {
  return "PorlaQueryLang.g4";
}

const std::vector<std::string>& PorlaQueryLangParser::getRuleNames() const {
  return porlaquerylangParserStaticData->ruleNames;
}

const dfa::Vocabulary& PorlaQueryLangParser::getVocabulary() const {
  return porlaquerylangParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView PorlaQueryLangParser::getSerializedATN() const {
  return porlaquerylangParserStaticData->serializedATN;
}


//----------------- FilterContext ------------------------------------------------------------------

PorlaQueryLangParser::FilterContext::FilterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PorlaQueryLangParser::OrExprContext* PorlaQueryLangParser::FilterContext::orExpr() {
  return getRuleContext<PorlaQueryLangParser::OrExprContext>(0);
}

tree::TerminalNode* PorlaQueryLangParser::FilterContext::EOF() {
  return getToken(PorlaQueryLangParser::EOF, 0);
}


size_t PorlaQueryLangParser::FilterContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleFilter;
}


std::any PorlaQueryLangParser::FilterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitFilter(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::FilterContext* PorlaQueryLangParser::filter() {
  FilterContext *_localctx = _tracker.createInstance<FilterContext>(_ctx, getState());
  enterRule(_localctx, 0, PorlaQueryLangParser::RuleFilter);

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
    orExpr();
    setState(17);
    match(PorlaQueryLangParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OrExprContext ------------------------------------------------------------------

PorlaQueryLangParser::OrExprContext::OrExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PorlaQueryLangParser::AndExprContext *> PorlaQueryLangParser::OrExprContext::andExpr() {
  return getRuleContexts<PorlaQueryLangParser::AndExprContext>();
}

PorlaQueryLangParser::AndExprContext* PorlaQueryLangParser::OrExprContext::andExpr(size_t i) {
  return getRuleContext<PorlaQueryLangParser::AndExprContext>(i);
}

std::vector<tree::TerminalNode *> PorlaQueryLangParser::OrExprContext::OR() {
  return getTokens(PorlaQueryLangParser::OR);
}

tree::TerminalNode* PorlaQueryLangParser::OrExprContext::OR(size_t i) {
  return getToken(PorlaQueryLangParser::OR, i);
}


size_t PorlaQueryLangParser::OrExprContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleOrExpr;
}


std::any PorlaQueryLangParser::OrExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitOrExpr(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::OrExprContext* PorlaQueryLangParser::orExpr() {
  OrExprContext *_localctx = _tracker.createInstance<OrExprContext>(_ctx, getState());
  enterRule(_localctx, 2, PorlaQueryLangParser::RuleOrExpr);
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
    while (_la == PorlaQueryLangParser::OR) {
      setState(20);
      match(PorlaQueryLangParser::OR);
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

PorlaQueryLangParser::AndExprContext::AndExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<PorlaQueryLangParser::TermContext *> PorlaQueryLangParser::AndExprContext::term() {
  return getRuleContexts<PorlaQueryLangParser::TermContext>();
}

PorlaQueryLangParser::TermContext* PorlaQueryLangParser::AndExprContext::term(size_t i) {
  return getRuleContext<PorlaQueryLangParser::TermContext>(i);
}

std::vector<tree::TerminalNode *> PorlaQueryLangParser::AndExprContext::AND() {
  return getTokens(PorlaQueryLangParser::AND);
}

tree::TerminalNode* PorlaQueryLangParser::AndExprContext::AND(size_t i) {
  return getToken(PorlaQueryLangParser::AND, i);
}


size_t PorlaQueryLangParser::AndExprContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleAndExpr;
}


std::any PorlaQueryLangParser::AndExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitAndExpr(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::AndExprContext* PorlaQueryLangParser::andExpr() {
  AndExprContext *_localctx = _tracker.createInstance<AndExprContext>(_ctx, getState());
  enterRule(_localctx, 4, PorlaQueryLangParser::RuleAndExpr);
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
    term();
    setState(34);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 127026) != 0)) {
      setState(29);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == PorlaQueryLangParser::AND) {
        setState(28);
        match(PorlaQueryLangParser::AND);
      }
      setState(31);
      term();
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

//----------------- TermContext ------------------------------------------------------------------

PorlaQueryLangParser::TermContext::TermContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PorlaQueryLangParser::TermContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleTerm;
}

void PorlaQueryLangParser::TermContext::copyFrom(TermContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- TextTermContext ------------------------------------------------------------------

PorlaQueryLangParser::TextContext* PorlaQueryLangParser::TextTermContext::text() {
  return getRuleContext<PorlaQueryLangParser::TextContext>(0);
}

PorlaQueryLangParser::TextTermContext::TextTermContext(TermContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::TextTermContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitTextTerm(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotTermContext ------------------------------------------------------------------

tree::TerminalNode* PorlaQueryLangParser::NotTermContext::NOT() {
  return getToken(PorlaQueryLangParser::NOT, 0);
}

PorlaQueryLangParser::TermContext* PorlaQueryLangParser::NotTermContext::term() {
  return getRuleContext<PorlaQueryLangParser::TermContext>(0);
}

PorlaQueryLangParser::NotTermContext::NotTermContext(TermContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::NotTermContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitNotTerm(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GroupTermContext ------------------------------------------------------------------

PorlaQueryLangParser::OrExprContext* PorlaQueryLangParser::GroupTermContext::orExpr() {
  return getRuleContext<PorlaQueryLangParser::OrExprContext>(0);
}

PorlaQueryLangParser::GroupTermContext::GroupTermContext(TermContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::GroupTermContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitGroupTerm(this);
  else
    return visitor->visitChildren(this);
}
//----------------- QualifierTermContext ------------------------------------------------------------------

PorlaQueryLangParser::QualifierContext* PorlaQueryLangParser::QualifierTermContext::qualifier() {
  return getRuleContext<PorlaQueryLangParser::QualifierContext>(0);
}

PorlaQueryLangParser::QualifierTermContext::QualifierTermContext(TermContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::QualifierTermContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitQualifierTerm(this);
  else
    return visitor->visitChildren(this);
}
PorlaQueryLangParser::TermContext* PorlaQueryLangParser::term() {
  TermContext *_localctx = _tracker.createInstance<TermContext>(_ctx, getState());
  enterRule(_localctx, 6, PorlaQueryLangParser::RuleTerm);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(45);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PorlaQueryLangParser::NOT: {
        _localctx = _tracker.createInstance<PorlaQueryLangParser::NotTermContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(37);
        match(PorlaQueryLangParser::NOT);
        setState(38);
        term();
        break;
      }

      case PorlaQueryLangParser::T__0: {
        _localctx = _tracker.createInstance<PorlaQueryLangParser::GroupTermContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(39);
        match(PorlaQueryLangParser::T__0);
        setState(40);
        orExpr();
        setState(41);
        match(PorlaQueryLangParser::T__1);
        break;
      }

      case PorlaQueryLangParser::QUALIFIER: {
        _localctx = _tracker.createInstance<PorlaQueryLangParser::QualifierTermContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(43);
        qualifier();
        break;
      }

      case PorlaQueryLangParser::FLOAT:
      case PorlaQueryLangParser::INT:
      case PorlaQueryLangParser::STRING:
      case PorlaQueryLangParser::ID: {
        _localctx = _tracker.createInstance<PorlaQueryLangParser::TextTermContext>(_localctx);
        enterOuterAlt(_localctx, 4);
        setState(44);
        text();
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

//----------------- QualifierContext ------------------------------------------------------------------

PorlaQueryLangParser::QualifierContext::QualifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PorlaQueryLangParser::QualifierContext::QUALIFIER() {
  return getToken(PorlaQueryLangParser::QUALIFIER, 0);
}

PorlaQueryLangParser::ValueContext* PorlaQueryLangParser::QualifierContext::value() {
  return getRuleContext<PorlaQueryLangParser::ValueContext>(0);
}

PorlaQueryLangParser::OperatorContext* PorlaQueryLangParser::QualifierContext::operator_() {
  return getRuleContext<PorlaQueryLangParser::OperatorContext>(0);
}


size_t PorlaQueryLangParser::QualifierContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleQualifier;
}


std::any PorlaQueryLangParser::QualifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitQualifier(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::QualifierContext* PorlaQueryLangParser::qualifier() {
  QualifierContext *_localctx = _tracker.createInstance<QualifierContext>(_ctx, getState());
  enterRule(_localctx, 8, PorlaQueryLangParser::RuleQualifier);
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
    setState(47);
    match(PorlaQueryLangParser::QUALIFIER);
    setState(49);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1984) != 0)) {
      setState(48);
      operator_();
    }
    setState(51);
    value();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OperatorContext ------------------------------------------------------------------

PorlaQueryLangParser::OperatorContext::OperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PorlaQueryLangParser::OperatorContext::OPER_EQ() {
  return getToken(PorlaQueryLangParser::OPER_EQ, 0);
}

tree::TerminalNode* PorlaQueryLangParser::OperatorContext::OPER_GT() {
  return getToken(PorlaQueryLangParser::OPER_GT, 0);
}

tree::TerminalNode* PorlaQueryLangParser::OperatorContext::OPER_GTE() {
  return getToken(PorlaQueryLangParser::OPER_GTE, 0);
}

tree::TerminalNode* PorlaQueryLangParser::OperatorContext::OPER_LT() {
  return getToken(PorlaQueryLangParser::OPER_LT, 0);
}

tree::TerminalNode* PorlaQueryLangParser::OperatorContext::OPER_LTE() {
  return getToken(PorlaQueryLangParser::OPER_LTE, 0);
}


size_t PorlaQueryLangParser::OperatorContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleOperator;
}


std::any PorlaQueryLangParser::OperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitOperator(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::OperatorContext* PorlaQueryLangParser::operator_() {
  OperatorContext *_localctx = _tracker.createInstance<OperatorContext>(_ctx, getState());
  enterRule(_localctx, 10, PorlaQueryLangParser::RuleOperator);
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
    setState(53);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1984) != 0))) {
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

PorlaQueryLangParser::ValueContext::ValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::INT() {
  return getToken(PorlaQueryLangParser::INT, 0);
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::FLOAT() {
  return getToken(PorlaQueryLangParser::FLOAT, 0);
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::ID() {
  return getToken(PorlaQueryLangParser::ID, 0);
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::STRING() {
  return getToken(PorlaQueryLangParser::STRING, 0);
}


size_t PorlaQueryLangParser::ValueContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleValue;
}


std::any PorlaQueryLangParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::ValueContext* PorlaQueryLangParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 12, PorlaQueryLangParser::RuleValue);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(61);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PorlaQueryLangParser::FLOAT:
      case PorlaQueryLangParser::INT: {
        enterOuterAlt(_localctx, 1);
        setState(55);
        _la = _input->LA(1);
        if (!(_la == PorlaQueryLangParser::FLOAT

        || _la == PorlaQueryLangParser::INT)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(57);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
        case 1: {
          setState(56);
          match(PorlaQueryLangParser::ID);
          break;
        }

        default:
          break;
        }
        break;
      }

      case PorlaQueryLangParser::STRING: {
        enterOuterAlt(_localctx, 2);
        setState(59);
        match(PorlaQueryLangParser::STRING);
        break;
      }

      case PorlaQueryLangParser::ID: {
        enterOuterAlt(_localctx, 3);
        setState(60);
        match(PorlaQueryLangParser::ID);
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

//----------------- TextContext ------------------------------------------------------------------

PorlaQueryLangParser::TextContext::TextContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PorlaQueryLangParser::TextContext::STRING() {
  return getToken(PorlaQueryLangParser::STRING, 0);
}

tree::TerminalNode* PorlaQueryLangParser::TextContext::ID() {
  return getToken(PorlaQueryLangParser::ID, 0);
}

tree::TerminalNode* PorlaQueryLangParser::TextContext::INT() {
  return getToken(PorlaQueryLangParser::INT, 0);
}

tree::TerminalNode* PorlaQueryLangParser::TextContext::FLOAT() {
  return getToken(PorlaQueryLangParser::FLOAT, 0);
}


size_t PorlaQueryLangParser::TextContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleText;
}


std::any PorlaQueryLangParser::TextContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitText(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::TextContext* PorlaQueryLangParser::text() {
  TextContext *_localctx = _tracker.createInstance<TextContext>(_ctx, getState());
  enterRule(_localctx, 14, PorlaQueryLangParser::RuleText);
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
    setState(63);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 94208) != 0))) {
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

void PorlaQueryLangParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  porlaquerylangParserInitialize();
#else
  ::antlr4::internal::call_once(porlaquerylangParserOnceFlag, porlaquerylangParserInitialize);
#endif
}

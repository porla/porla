
// Generated from PorlaQueryLang.g4 by ANTLR 4.11.1


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
PorlaQueryLangParserStaticData *porlaquerylangParserStaticData = nullptr;

void porlaquerylangParserInitialize() {
  assert(porlaquerylangParserStaticData == nullptr);
  auto staticData = std::make_unique<PorlaQueryLangParserStaticData>(
    std::vector<std::string>{
      "filter", "expression", "predicate", "flag", "value", "operator", 
      "reference"
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
  	4,1,18,71,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,1,0,
  	1,0,1,1,1,1,1,1,1,1,1,1,3,1,22,8,1,1,1,1,1,1,1,1,1,1,1,1,1,5,1,30,8,1,
  	10,1,12,1,33,9,1,1,2,1,2,1,2,1,2,1,3,1,3,1,3,1,4,1,4,3,4,44,8,4,1,4,3,
  	4,47,8,4,1,4,1,4,3,4,51,8,4,1,4,3,4,54,8,4,1,4,1,4,3,4,58,8,4,1,4,3,4,
  	61,8,4,1,4,1,4,3,4,65,8,4,1,5,1,5,1,6,1,6,1,6,0,1,2,7,0,2,4,6,8,10,12,
  	0,1,1,0,5,10,77,0,14,1,0,0,0,2,21,1,0,0,0,4,34,1,0,0,0,6,38,1,0,0,0,8,
  	64,1,0,0,0,10,66,1,0,0,0,12,68,1,0,0,0,14,15,3,2,1,0,15,1,1,0,0,0,16,
  	17,6,1,-1,0,17,22,3,4,2,0,18,22,3,6,3,0,19,20,5,4,0,0,20,22,3,6,3,0,21,
  	16,1,0,0,0,21,18,1,0,0,0,21,19,1,0,0,0,22,31,1,0,0,0,23,24,10,5,0,0,24,
  	25,5,2,0,0,25,30,3,2,1,6,26,27,10,4,0,0,27,28,5,3,0,0,28,30,3,2,1,5,29,
  	23,1,0,0,0,29,26,1,0,0,0,30,33,1,0,0,0,31,29,1,0,0,0,31,32,1,0,0,0,32,
  	3,1,0,0,0,33,31,1,0,0,0,34,35,3,12,6,0,35,36,3,10,5,0,36,37,3,8,4,0,37,
  	5,1,0,0,0,38,39,5,1,0,0,39,40,3,12,6,0,40,7,1,0,0,0,41,43,5,12,0,0,42,
  	44,5,11,0,0,43,42,1,0,0,0,43,44,1,0,0,0,44,46,1,0,0,0,45,47,5,15,0,0,
  	46,45,1,0,0,0,46,47,1,0,0,0,47,65,1,0,0,0,48,50,5,12,0,0,49,51,5,11,0,
  	0,50,49,1,0,0,0,50,51,1,0,0,0,51,53,1,0,0,0,52,54,5,16,0,0,53,52,1,0,
  	0,0,53,54,1,0,0,0,54,65,1,0,0,0,55,57,5,12,0,0,56,58,5,11,0,0,57,56,1,
  	0,0,0,57,58,1,0,0,0,58,60,1,0,0,0,59,61,5,17,0,0,60,59,1,0,0,0,60,61,
  	1,0,0,0,61,65,1,0,0,0,62,65,5,13,0,0,63,65,5,14,0,0,64,41,1,0,0,0,64,
  	48,1,0,0,0,64,55,1,0,0,0,64,62,1,0,0,0,64,63,1,0,0,0,65,9,1,0,0,0,66,
  	67,7,0,0,0,67,11,1,0,0,0,68,69,5,18,0,0,69,13,1,0,0,0,10,21,29,31,43,
  	46,50,53,57,60,64
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  porlaquerylangParserStaticData = staticData.release();
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

PorlaQueryLangParser::ExpressionContext* PorlaQueryLangParser::FilterContext::expression() {
  return getRuleContext<PorlaQueryLangParser::ExpressionContext>(0);
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
    setState(14);
    expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

PorlaQueryLangParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PorlaQueryLangParser::ExpressionContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleExpression;
}

void PorlaQueryLangParser::ExpressionContext::copyFrom(ExpressionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- AndExpressionContext ------------------------------------------------------------------

std::vector<PorlaQueryLangParser::ExpressionContext *> PorlaQueryLangParser::AndExpressionContext::expression() {
  return getRuleContexts<PorlaQueryLangParser::ExpressionContext>();
}

PorlaQueryLangParser::ExpressionContext* PorlaQueryLangParser::AndExpressionContext::expression(size_t i) {
  return getRuleContext<PorlaQueryLangParser::ExpressionContext>(i);
}

tree::TerminalNode* PorlaQueryLangParser::AndExpressionContext::AND() {
  return getToken(PorlaQueryLangParser::AND, 0);
}

PorlaQueryLangParser::AndExpressionContext::AndExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::AndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitAndExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FlagExpressionContext ------------------------------------------------------------------

PorlaQueryLangParser::FlagContext* PorlaQueryLangParser::FlagExpressionContext::flag() {
  return getRuleContext<PorlaQueryLangParser::FlagContext>(0);
}

PorlaQueryLangParser::FlagExpressionContext::FlagExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::FlagExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitFlagExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- PredicateExpressionContext ------------------------------------------------------------------

PorlaQueryLangParser::PredicateContext* PorlaQueryLangParser::PredicateExpressionContext::predicate() {
  return getRuleContext<PorlaQueryLangParser::PredicateContext>(0);
}

PorlaQueryLangParser::PredicateExpressionContext::PredicateExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::PredicateExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitPredicateExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OrExpressionContext ------------------------------------------------------------------

std::vector<PorlaQueryLangParser::ExpressionContext *> PorlaQueryLangParser::OrExpressionContext::expression() {
  return getRuleContexts<PorlaQueryLangParser::ExpressionContext>();
}

PorlaQueryLangParser::ExpressionContext* PorlaQueryLangParser::OrExpressionContext::expression(size_t i) {
  return getRuleContext<PorlaQueryLangParser::ExpressionContext>(i);
}

tree::TerminalNode* PorlaQueryLangParser::OrExpressionContext::OR() {
  return getToken(PorlaQueryLangParser::OR, 0);
}

PorlaQueryLangParser::OrExpressionContext::OrExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::OrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitOrExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotFlagExpressionContext ------------------------------------------------------------------

tree::TerminalNode* PorlaQueryLangParser::NotFlagExpressionContext::NOT() {
  return getToken(PorlaQueryLangParser::NOT, 0);
}

PorlaQueryLangParser::FlagContext* PorlaQueryLangParser::NotFlagExpressionContext::flag() {
  return getRuleContext<PorlaQueryLangParser::FlagContext>(0);
}

PorlaQueryLangParser::NotFlagExpressionContext::NotFlagExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::NotFlagExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitNotFlagExpression(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::ExpressionContext* PorlaQueryLangParser::expression() {
   return expression(0);
}

PorlaQueryLangParser::ExpressionContext* PorlaQueryLangParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  PorlaQueryLangParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  PorlaQueryLangParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 2;
  enterRecursionRule(_localctx, 2, PorlaQueryLangParser::RuleExpression, precedence);

    

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
    setState(21);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case PorlaQueryLangParser::ID: {
        _localctx = _tracker.createInstance<PredicateExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(17);
        predicate();
        break;
      }

      case PorlaQueryLangParser::T__0: {
        _localctx = _tracker.createInstance<FlagExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(18);
        flag();
        break;
      }

      case PorlaQueryLangParser::NOT: {
        _localctx = _tracker.createInstance<NotFlagExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(19);
        match(PorlaQueryLangParser::NOT);
        setState(20);
        flag();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(31);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(29);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<AndExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(23);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(24);
          match(PorlaQueryLangParser::AND);
          setState(25);
          expression(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<OrExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(26);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(27);
          match(PorlaQueryLangParser::OR);
          setState(28);
          expression(5);
          break;
        }

        default:
          break;
        } 
      }
      setState(33);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- PredicateContext ------------------------------------------------------------------

PorlaQueryLangParser::PredicateContext::PredicateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t PorlaQueryLangParser::PredicateContext::getRuleIndex() const {
  return PorlaQueryLangParser::RulePredicate;
}

void PorlaQueryLangParser::PredicateContext::copyFrom(PredicateContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- OperatorPredicateContext ------------------------------------------------------------------

PorlaQueryLangParser::ReferenceContext* PorlaQueryLangParser::OperatorPredicateContext::reference() {
  return getRuleContext<PorlaQueryLangParser::ReferenceContext>(0);
}

PorlaQueryLangParser::OperatorContext* PorlaQueryLangParser::OperatorPredicateContext::operator_() {
  return getRuleContext<PorlaQueryLangParser::OperatorContext>(0);
}

PorlaQueryLangParser::ValueContext* PorlaQueryLangParser::OperatorPredicateContext::value() {
  return getRuleContext<PorlaQueryLangParser::ValueContext>(0);
}

PorlaQueryLangParser::OperatorPredicateContext::OperatorPredicateContext(PredicateContext *ctx) { copyFrom(ctx); }


std::any PorlaQueryLangParser::OperatorPredicateContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitOperatorPredicate(this);
  else
    return visitor->visitChildren(this);
}
PorlaQueryLangParser::PredicateContext* PorlaQueryLangParser::predicate() {
  PredicateContext *_localctx = _tracker.createInstance<PredicateContext>(_ctx, getState());
  enterRule(_localctx, 4, PorlaQueryLangParser::RulePredicate);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    _localctx = _tracker.createInstance<PorlaQueryLangParser::OperatorPredicateContext>(_localctx);
    enterOuterAlt(_localctx, 1);
    setState(34);
    reference();
    setState(35);
    operator_();
    setState(36);
    value();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FlagContext ------------------------------------------------------------------

PorlaQueryLangParser::FlagContext::FlagContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

PorlaQueryLangParser::ReferenceContext* PorlaQueryLangParser::FlagContext::reference() {
  return getRuleContext<PorlaQueryLangParser::ReferenceContext>(0);
}


size_t PorlaQueryLangParser::FlagContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleFlag;
}


std::any PorlaQueryLangParser::FlagContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitFlag(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::FlagContext* PorlaQueryLangParser::flag() {
  FlagContext *_localctx = _tracker.createInstance<FlagContext>(_ctx, getState());
  enterRule(_localctx, 6, PorlaQueryLangParser::RuleFlag);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(38);
    match(PorlaQueryLangParser::T__0);
    setState(39);
    reference();
   
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

tree::TerminalNode* PorlaQueryLangParser::ValueContext::WHITESPACE() {
  return getToken(PorlaQueryLangParser::WHITESPACE, 0);
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::UNIT_DURATION() {
  return getToken(PorlaQueryLangParser::UNIT_DURATION, 0);
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::UNIT_SIZE() {
  return getToken(PorlaQueryLangParser::UNIT_SIZE, 0);
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::UNIT_SPEED() {
  return getToken(PorlaQueryLangParser::UNIT_SPEED, 0);
}

tree::TerminalNode* PorlaQueryLangParser::ValueContext::FLOAT() {
  return getToken(PorlaQueryLangParser::FLOAT, 0);
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
  enterRule(_localctx, 8, PorlaQueryLangParser::RuleValue);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(64);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(41);
      match(PorlaQueryLangParser::INT);
      setState(43);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx)) {
      case 1: {
        setState(42);
        match(PorlaQueryLangParser::WHITESPACE);
        break;
      }

      default:
        break;
      }
      setState(46);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
      case 1: {
        setState(45);
        match(PorlaQueryLangParser::UNIT_DURATION);
        break;
      }

      default:
        break;
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(48);
      match(PorlaQueryLangParser::INT);
      setState(50);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
      case 1: {
        setState(49);
        match(PorlaQueryLangParser::WHITESPACE);
        break;
      }

      default:
        break;
      }
      setState(53);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
      case 1: {
        setState(52);
        match(PorlaQueryLangParser::UNIT_SIZE);
        break;
      }

      default:
        break;
      }
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(55);
      match(PorlaQueryLangParser::INT);
      setState(57);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
      case 1: {
        setState(56);
        match(PorlaQueryLangParser::WHITESPACE);
        break;
      }

      default:
        break;
      }
      setState(60);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx)) {
      case 1: {
        setState(59);
        match(PorlaQueryLangParser::UNIT_SPEED);
        break;
      }

      default:
        break;
      }
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(62);
      match(PorlaQueryLangParser::FLOAT);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(63);
      match(PorlaQueryLangParser::STRING);
      break;
    }

    default:
      break;
    }
   
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

tree::TerminalNode* PorlaQueryLangParser::OperatorContext::OPER_CONTAINS() {
  return getToken(PorlaQueryLangParser::OPER_CONTAINS, 0);
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
    setState(66);
    _la = _input->LA(1);
    if (!(((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2016) != 0)) {
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

//----------------- ReferenceContext ------------------------------------------------------------------

PorlaQueryLangParser::ReferenceContext::ReferenceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* PorlaQueryLangParser::ReferenceContext::ID() {
  return getToken(PorlaQueryLangParser::ID, 0);
}


size_t PorlaQueryLangParser::ReferenceContext::getRuleIndex() const {
  return PorlaQueryLangParser::RuleReference;
}


std::any PorlaQueryLangParser::ReferenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<PorlaQueryLangVisitor*>(visitor))
    return parserVisitor->visitReference(this);
  else
    return visitor->visitChildren(this);
}

PorlaQueryLangParser::ReferenceContext* PorlaQueryLangParser::reference() {
  ReferenceContext *_localctx = _tracker.createInstance<ReferenceContext>(_ctx, getState());
  enterRule(_localctx, 12, PorlaQueryLangParser::RuleReference);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(68);
    match(PorlaQueryLangParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool PorlaQueryLangParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 1: return expressionSempred(antlrcpp::downCast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool PorlaQueryLangParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 5);
    case 1: return precpred(_ctx, 4);

  default:
    break;
  }
  return true;
}

void PorlaQueryLangParser::initialize() {
  ::antlr4::internal::call_once(porlaquerylangParserOnceFlag, porlaquerylangParserInitialize);
}

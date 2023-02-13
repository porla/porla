
// Generated from PorlaQueryLang.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  PorlaQueryLangParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, AND = 2, OR = 3, NOT = 4, OPER_EQ = 5, OPER_CONTAINS = 6, 
    OPER_GT = 7, OPER_GTE = 8, OPER_LT = 9, OPER_LTE = 10, WHITESPACE = 11, 
    INT = 12, FLOAT = 13, STRING = 14, UNIT_DURATION = 15, UNIT_SIZE = 16, 
    UNIT_SPEED = 17, ID = 18
  };

  enum {
    RuleFilter = 0, RuleExpression = 1, RulePredicate = 2, RuleFlag = 3, 
    RuleValue = 4, RuleOperator = 5, RuleReference = 6
  };

  explicit PorlaQueryLangParser(antlr4::TokenStream *input);

  PorlaQueryLangParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~PorlaQueryLangParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class FilterContext;
  class ExpressionContext;
  class PredicateContext;
  class FlagContext;
  class ValueContext;
  class OperatorContext;
  class ReferenceContext; 

  class  FilterContext : public antlr4::ParserRuleContext {
  public:
    FilterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FilterContext* filter();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExpressionContext() = default;
    void copyFrom(ExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  AndExpressionContext : public ExpressionContext {
  public:
    AndExpressionContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *AND();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FlagExpressionContext : public ExpressionContext {
  public:
    FlagExpressionContext(ExpressionContext *ctx);

    FlagContext *flag();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  PredicateExpressionContext : public ExpressionContext {
  public:
    PredicateExpressionContext(ExpressionContext *ctx);

    PredicateContext *predicate();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  OrExpressionContext : public ExpressionContext {
  public:
    OrExpressionContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *OR();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NotFlagExpressionContext : public ExpressionContext {
  public:
    NotFlagExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *NOT();
    FlagContext *flag();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  PredicateContext : public antlr4::ParserRuleContext {
  public:
    PredicateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    PredicateContext() = default;
    void copyFrom(PredicateContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  OperatorPredicateContext : public PredicateContext {
  public:
    OperatorPredicateContext(PredicateContext *ctx);

    ReferenceContext *reference();
    OperatorContext *operator_();
    ValueContext *value();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  PredicateContext* predicate();

  class  FlagContext : public antlr4::ParserRuleContext {
  public:
    FlagContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ReferenceContext *reference();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FlagContext* flag();

  class  ValueContext : public antlr4::ParserRuleContext {
  public:
    ValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INT();
    antlr4::tree::TerminalNode *WHITESPACE();
    antlr4::tree::TerminalNode *UNIT_DURATION();
    antlr4::tree::TerminalNode *UNIT_SIZE();
    antlr4::tree::TerminalNode *UNIT_SPEED();
    antlr4::tree::TerminalNode *FLOAT();
    antlr4::tree::TerminalNode *STRING();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueContext* value();

  class  OperatorContext : public antlr4::ParserRuleContext {
  public:
    OperatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPER_EQ();
    antlr4::tree::TerminalNode *OPER_CONTAINS();
    antlr4::tree::TerminalNode *OPER_GT();
    antlr4::tree::TerminalNode *OPER_GTE();
    antlr4::tree::TerminalNode *OPER_LT();
    antlr4::tree::TerminalNode *OPER_LTE();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OperatorContext* operator_();

  class  ReferenceContext : public antlr4::ParserRuleContext {
  public:
    ReferenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReferenceContext* reference();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};


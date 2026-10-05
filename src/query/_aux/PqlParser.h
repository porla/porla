
// Generated from PqlParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  PqlParser : public antlr4::Parser {
public:
  enum {
    OR = 1, AND = 2, NOT = 3, LPAREN = 4, RPAREN = 5, FIELD = 6, STRING = 7, 
    WORD = 8, WS = 9, V_GTE = 10, V_LTE = 11, V_GT = 12, V_LT = 13, V_EQ = 14, 
    V_STRING = 15, V_WORD = 16, V_WS = 17
  };

  enum {
    RuleQuery = 0, RuleOrExpr = 1, RuleAndExpr = 2, RuleUnary = 3, RulePrimary = 4, 
    RuleOp = 5, RuleFieldValue = 6
  };

  explicit PqlParser(antlr4::TokenStream *input);

  PqlParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~PqlParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class QueryContext;
  class OrExprContext;
  class AndExprContext;
  class UnaryContext;
  class PrimaryContext;
  class OpContext;
  class FieldValueContext; 

  class  QueryContext : public antlr4::ParserRuleContext {
  public:
    QueryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    OrExprContext *orExpr();

   
  };

  QueryContext* query();

  class  OrExprContext : public antlr4::ParserRuleContext {
  public:
    OrExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<AndExprContext *> andExpr();
    AndExprContext* andExpr(size_t i);
    std::vector<antlr4::tree::TerminalNode *> OR();
    antlr4::tree::TerminalNode* OR(size_t i);

   
  };

  OrExprContext* orExpr();

  class  AndExprContext : public antlr4::ParserRuleContext {
  public:
    AndExprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<UnaryContext *> unary();
    UnaryContext* unary(size_t i);
    std::vector<antlr4::tree::TerminalNode *> AND();
    antlr4::tree::TerminalNode* AND(size_t i);

   
  };

  AndExprContext* andExpr();

  class  UnaryContext : public antlr4::ParserRuleContext {
  public:
    UnaryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    UnaryContext() = default;
    void copyFrom(UnaryContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  PrimaryExprContext : public UnaryContext {
  public:
    PrimaryExprContext(UnaryContext *ctx);

    PrimaryContext *primary();
  };

  class  NotExprContext : public UnaryContext {
  public:
    NotExprContext(UnaryContext *ctx);

    antlr4::tree::TerminalNode *NOT();
    UnaryContext *unary();
  };

  UnaryContext* unary();

  class  PrimaryContext : public antlr4::ParserRuleContext {
  public:
    PrimaryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    PrimaryContext() = default;
    void copyFrom(PrimaryContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  GroupExprContext : public PrimaryContext {
  public:
    GroupExprContext(PrimaryContext *ctx);

    antlr4::tree::TerminalNode *LPAREN();
    OrExprContext *orExpr();
    antlr4::tree::TerminalNode *RPAREN();
  };

  class  TextExprContext : public PrimaryContext {
  public:
    TextExprContext(PrimaryContext *ctx);

    antlr4::tree::TerminalNode *WORD();
    antlr4::tree::TerminalNode *STRING();
  };

  class  QualifierExprContext : public PrimaryContext {
  public:
    QualifierExprContext(PrimaryContext *ctx);

    antlr4::tree::TerminalNode *FIELD();
    FieldValueContext *fieldValue();
    OpContext *op();
  };

  PrimaryContext* primary();

  class  OpContext : public antlr4::ParserRuleContext {
  public:
    OpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *V_GTE();
    antlr4::tree::TerminalNode *V_LTE();
    antlr4::tree::TerminalNode *V_GT();
    antlr4::tree::TerminalNode *V_LT();
    antlr4::tree::TerminalNode *V_EQ();

   
  };

  OpContext* op();

  class  FieldValueContext : public antlr4::ParserRuleContext {
  public:
    FieldValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *V_WORD();
    antlr4::tree::TerminalNode *V_STRING();

   
  };

  FieldValueContext* fieldValue();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};


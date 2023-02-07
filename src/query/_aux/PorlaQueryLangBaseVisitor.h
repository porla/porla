
// Generated from PorlaQueryLang.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "PorlaQueryLangVisitor.h"


/**
 * This class provides an empty implementation of PorlaQueryLangVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  PorlaQueryLangBaseVisitor : public PorlaQueryLangVisitor {
public:

  virtual std::any visitFilter(PorlaQueryLangParser::FilterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpression(PorlaQueryLangParser::AndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFlagExpression(PorlaQueryLangParser::FlagExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPredicateExpression(PorlaQueryLangParser::PredicateExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrExpression(PorlaQueryLangParser::OrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperatorPredicate(PorlaQueryLangParser::OperatorPredicateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFlag(PorlaQueryLangParser::FlagContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(PorlaQueryLangParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperator(PorlaQueryLangParser::OperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReference(PorlaQueryLangParser::ReferenceContext *ctx) override {
    return visitChildren(ctx);
  }


};


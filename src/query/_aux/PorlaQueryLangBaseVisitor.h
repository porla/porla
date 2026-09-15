
// Generated from PorlaQueryLang.g4 by ANTLR 4.13.2

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

  virtual std::any visitOrExpr(PorlaQueryLangParser::OrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpr(PorlaQueryLangParser::AndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNotTerm(PorlaQueryLangParser::NotTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGroupTerm(PorlaQueryLangParser::GroupTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualifierTerm(PorlaQueryLangParser::QualifierTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTextTerm(PorlaQueryLangParser::TextTermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualifier(PorlaQueryLangParser::QualifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperator(PorlaQueryLangParser::OperatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(PorlaQueryLangParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitText(PorlaQueryLangParser::TextContext *ctx) override {
    return visitChildren(ctx);
  }


};


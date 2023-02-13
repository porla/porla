
// Generated from PorlaQueryLang.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "PorlaQueryLangParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by PorlaQueryLangParser.
 */
class  PorlaQueryLangVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by PorlaQueryLangParser.
   */
    virtual std::any visitFilter(PorlaQueryLangParser::FilterContext *context) = 0;

    virtual std::any visitAndExpression(PorlaQueryLangParser::AndExpressionContext *context) = 0;

    virtual std::any visitFlagExpression(PorlaQueryLangParser::FlagExpressionContext *context) = 0;

    virtual std::any visitPredicateExpression(PorlaQueryLangParser::PredicateExpressionContext *context) = 0;

    virtual std::any visitOrExpression(PorlaQueryLangParser::OrExpressionContext *context) = 0;

    virtual std::any visitNotFlagExpression(PorlaQueryLangParser::NotFlagExpressionContext *context) = 0;

    virtual std::any visitOperatorPredicate(PorlaQueryLangParser::OperatorPredicateContext *context) = 0;

    virtual std::any visitFlag(PorlaQueryLangParser::FlagContext *context) = 0;

    virtual std::any visitValue(PorlaQueryLangParser::ValueContext *context) = 0;

    virtual std::any visitOperator(PorlaQueryLangParser::OperatorContext *context) = 0;

    virtual std::any visitReference(PorlaQueryLangParser::ReferenceContext *context) = 0;


};


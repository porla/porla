
// Generated from PorlaQueryLang.g4 by ANTLR 4.13.2

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

    virtual std::any visitOrExpr(PorlaQueryLangParser::OrExprContext *context) = 0;

    virtual std::any visitAndExpr(PorlaQueryLangParser::AndExprContext *context) = 0;

    virtual std::any visitNotTerm(PorlaQueryLangParser::NotTermContext *context) = 0;

    virtual std::any visitGroupTerm(PorlaQueryLangParser::GroupTermContext *context) = 0;

    virtual std::any visitQualifierTerm(PorlaQueryLangParser::QualifierTermContext *context) = 0;

    virtual std::any visitTextTerm(PorlaQueryLangParser::TextTermContext *context) = 0;

    virtual std::any visitQualifier(PorlaQueryLangParser::QualifierContext *context) = 0;

    virtual std::any visitOperator(PorlaQueryLangParser::OperatorContext *context) = 0;

    virtual std::any visitValue(PorlaQueryLangParser::ValueContext *context) = 0;

    virtual std::any visitText(PorlaQueryLangParser::TextContext *context) = 0;


};


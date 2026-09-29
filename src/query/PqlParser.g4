parser grammar PqlParser;

options { tokenVocab = PqlLexer; }

query      : orExpr? EOF ;
orExpr     : andExpr (OR andExpr)* ;
andExpr    : unary (AND? unary)* ;
unary      : NOT unary                  #NotExpr
           | primary                    #PrimaryExpr
           ;
primary    : LPAREN orExpr RPAREN       #GroupExpr
           | FIELD op? fieldValue       #QualifierExpr
           | (WORD | STRING)            #TextExpr
           ;
op         : V_GTE | V_LTE | V_GT | V_LT | V_EQ ;
fieldValue : V_WORD | V_STRING ;

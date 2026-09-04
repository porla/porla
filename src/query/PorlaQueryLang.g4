grammar PorlaQueryLang;

// ---------- Lexer ----------
OR            : 'OR' | '||' ;
AND           : 'AND' | '&&' ;
NOT           : '-' | '!' ;

OPER_EQ       : '=';
OPER_GTE      : '>=';
OPER_GT       : '>';
OPER_LTE      : '<=';
OPER_LT       : '<';

WHITESPACE    : [ \t\r\n]+ -> skip;
FLOAT         : '-'? [0-9]+ '.' [0-9]+ ;
INT           : '-'? [0-9]+ ;
STRING        : '"' .*? '"';
QUALIFIER     : [a-zA-Z_]+ ':' ;
ID            : [a-zA-Z_]+ ('-' [a-zA-Z_]+)* ;

// ---------- Parser ----------
filter   : orExpr EOF ;

orExpr   : andExpr (OR andExpr)* ;

andExpr  : term (AND? term)* ;

term
    : NOT term        #NotTerm
    | '(' orExpr ')'  #GroupTerm
    | qualifier       #QualifierTerm
    | text            #TextTerm
    ;

qualifier : QUALIFIER operator? value ;

operator  : OPER_EQ | OPER_GT | OPER_GTE | OPER_LT | OPER_LTE ;

value     : (INT | FLOAT) ID? | STRING | ID ;

text      : STRING | ID | INT | FLOAT ;
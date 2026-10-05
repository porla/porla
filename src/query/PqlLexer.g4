lexer grammar PqlLexer;

OR      : 'OR' | '||' ;
AND     : 'AND' | '&&' ;
NOT     : 'NOT' | '!' | '-' ;
LPAREN  : '(' ;
RPAREN  : ')' ;

FIELD   : '$'? [a-zA-Z_] [a-zA-Z0-9_.\-]* ':' -> pushMode(VALUE) ;
STRING  : '"' ( '\\' . | ~["\\] )* '"' ;
WORD    : ~[ \t\r\n()"<>=!:&|\-] ~[ \t\r\n()"<>=!:]* ;
WS      : [ \t\r\n]+ -> skip ;

mode VALUE;

V_GTE    : '>=' ;
V_LTE    : '<=' ;
V_GT     : '>' ;
V_LT     : '<' ;
V_EQ     : '=' ;
V_STRING : '"' ( '\\' . | ~["\\] )* '"'          -> popMode ;
V_WORD   : ~[ \t\r\n()"<>=] ~[ \t\r\n()"]*       -> popMode ;
V_WS     : [ \t\r\n]+                            -> skip, popMode ;   // "name: foo" → empty value → syntax error

grammar PorlaQueryLang;


AND           : 'and';
OR            : 'or';
NOT           : 'not';

OPER_EQ       : '=';
OPER_CONTAINS : 'contains';
OPER_GT       : '>';
OPER_GTE      : '>=';
OPER_LT       : '<';
OPER_LTE      : '<=';

WHITESPACE    : [ \t\r\n]+ -> skip;
INT           : '-'? [0-9]+ ;
FLOAT         : '-'? [0-9]+ '.' [0-9]+ ;
STRING        : '"' .*? '"';

UNIT_DURATION : 's' | 'm' | 'h' | 'd' | 'w';
UNIT_SIZE     : 'b' | 'kb' | 'mb' | 'gb' | 'tb' | 'pb';
UNIT_SPEED    : 'bps' | 'kbps' | 'mbps' | 'gbps';

ID            : [a-zA-Z_]+;


filter
    : expression
    ;

expression
    : expression AND expression #AndExpression
    | expression OR expression  #OrExpression
    | predicate                 #PredicateExpression
    | flag                      #FlagExpression
    | NOT flag                  #NotFlagExpression
    ;

predicate
    : reference operator value  #OperatorPredicate
    ;

flag
    : 'is:' reference
    ;

value
    : INT WHITESPACE? UNIT_DURATION?
    | INT WHITESPACE? UNIT_SIZE?
    | INT WHITESPACE? UNIT_SPEED?
    | FLOAT
    | STRING
    ;

operator
    : OPER_EQ
    | OPER_CONTAINS
    | OPER_GT
    | OPER_GTE
    | OPER_LT
    | OPER_LTE
    ;

reference: ID;

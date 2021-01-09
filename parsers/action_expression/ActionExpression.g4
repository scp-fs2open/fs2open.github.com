grammar ActionExpression;

expression_main: expression EOF;

expression
 : value_expression
 | random_range_expression
 | parenthesis_expression
 | expression ( PLUS | MINUS ) expression
 ;

parenthesis_expression: L_PAREN expression R_PAREN;

value_expression
 : literal_expression
 | vec3d_constructor
 ;

literal_expression
 : FLOAT
 | INT
 | STRING
 ;

random_range_expression: RAND_L_PAREN expression expression R_PAREN;

vec3d_constructor: L_PAREN expression expression expression R_PAREN;

PLUS: '+';
MINUS: '-';

FLOAT
 : '-'? ([0-9]+ '.' [0-9]+)
 ;

INT: '-'? [0-9]+;

// The left parenthesis for the start of a random range
RAND_L_PAREN: '~(';

L_PAREN: '(';
R_PAREN: ')';

STRING: '"' .*? '"';

SPACE
 : [ \t\r\n] -> skip
 ;

OTHER
 : . 
 ;

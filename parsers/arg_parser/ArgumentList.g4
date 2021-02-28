grammar ArgumentList;

arg_list: argument? EOF;

// This is used for parsing standalone types. Tuples are not valid in argument lists
standalone_type: type (COMMA type)*;

simple_type
 : ID
 | NIL
 ;

varargs_or_simple_type: simple_type VARARGS_SPECIFIER?;

func_arg: type ID;

func_arglist
 :
 | func_arg (COMMA func_arg)*
 ;

function_type: FUNCTION L_PAREN func_arglist R_PAREN ARROW type;

map_type: L_CURLY type ARROW type VARARGS_SPECIFIER R_CURLY;

iterator_type: ITERATOR L_ANGLE_BRACKET type R_ANGLE_BRACKET;

type
 : varargs_or_simple_type
 | function_type
 | map_type
 | iterator_type
 | type L_BRACKET R_BRACKET // array types
 | type (TYPE_ALT type)+ // Alternative types
 ;

boolean: TRUE | FALSE;

value
 : STRING
 | NIL
 | NUMBER
 | ID
 | boolean
 ;

actual_argument: type ID? (EQUALS value)? ARG_COMMENT? (COMMA argument)?;

optional_argument: L_BRACKET actual_argument R_BRACKET;

argument
 : actual_argument
 | optional_argument;

COMMA: ',';

EQUALS: '=';

STRING
 : '"' (~["\r\n])* '"'
 | '\'' (~['\r\n])* '\''
 ;

NIL: 'nil';

TRUE: 'true';
FALSE: 'false';

FUNCTION: 'function';

VARARGS_SPECIFIER: '...';

NUMBER
 : '-'? (
  [0-9]+
  | [0-9]+ '.' [0-9]*
  | '.' [0-9]+
 )
 ;

TYPE_ALT
 : '|'
 | '/'
 ;

L_BRACKET: '[';
R_BRACKET: ']';

L_PAREN: '(';
R_PAREN: ')';

L_CURLY: '{';
R_CURLY: '}';

ARROW: '=>';

ITERATOR: 'iterator';

L_ANGLE_BRACKET: '<';
R_ANGLE_BRACKET: '>';

ARG_COMMENT: '/*' .*? '*/';

ID
 : [a-zA-Z_] [a-zA-Z_0-9:]*
 ;

SPACE
 : [ \t\r\n] -> skip
 ;

OTHER
 : . 
 ;

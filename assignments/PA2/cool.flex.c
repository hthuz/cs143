
/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

%}


/*
 * Define names for regular expressions here.
 */

CLASS   "class"
ELSE    "else"
FI      "fi"
IF      "if"
IN      "in"
INHERITS "inherits"
LET     "let"
LOOP    "loop"
POOL    "pool"
THEN    "then"
WHILE   "while"
CASE    "case"
ESAC    "esac"
OF      "of"
DARROW          =>
NEW     "new"
ISVOID  "isvoid"
TRUE    "true"
FALSE   "false"

DIGIT [0-9]
DIGITS DIGIT+
LETTER [a-zA-Z]
ID LETTER(LETTER|DIGIT)*
WHITESPACE (" "|"\n"|"\t")+
// STRING \"[^\"]\"
expr TRUE|FALSE|STRING|ID|DIGITS

%%

 /*
  *  Nested comments
  */


 /*
  *  The multiple-character operators.
  */
{CLASS} {return {CLASS};}
{ELSE} {return {ELSE};}
{FI} {return {FI};}
{IF} {return {IF};}
{IN} {return {IN};}
{INHERITS} {return {INHERITS};}
{LET} {return {LET};}
{LOOP} {return {LOOP};}
{POOL} {return {POOL};}
{THEN} {return {THEN};}
{WHILE} {return {WHILE};}
{CASE} {return {CASE};}
{ESAC} {return {ESAC};}
{OF} {return {OF};}
{DARROW}		{ return (DARROW); }
{NEW} {return {NEW};}
{ISVOID} {return {ISVOID};}
{BOOL_CONST} {}
{TRUE} {
    curr_lineno = 
    yylval.boolean = true;
    return BOOL_CONST;
}
{FALSE} {
    yylval.boolean = false;
    return BOOL_CONST;
}

{DIGITS} {yylval.}

"\n" {
    curr_lineno += 1;
}

expr {
    yylval.expression = strdup(yytext);
}

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */


%%

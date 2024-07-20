
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
char *string_buf_ptr_end;
bool eof_err_reported = false;
bool null_err_reported = false;
bool escaped_null_err_reported = false;
bool long_str_err_reported = false;

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
DARROW  =>
NEW     "new"
ISVOID  "isvoid"
INT_CONST [0-9]*
TRUE    "true"
FALSE   "false"
TYPEID [A-Z]([a-zA-Z]|[0-9]|_)*
OBJECTID [a-z]([a-zA-Z]|[0-9]|_)*
ASSIGN "<-"
NOT "not"
LE "<="

WHITESPACE (" "|"\n"|"\t")+
EXPR TRUE|FALSE|STRING|ID|DIGITS

%x COMMENT
%x STR
%%

 /*
  *  Nested comments
  */

"(*" BEGIN(COMMENT);
<COMMENT>[^*\n]* {}
<COMMENT>"*"+[^*)\n]* {}
<COMMENT>"\n" {curr_lineno++;}
<COMMENT>"*)" BEGIN(INITIAL);
<COMMENT><<EOF>> {
  if (eof_err_reported) {
    yyterminate();
  }
  yylval.error_msg = "EOF in comment"; 
  eof_err_reported = true;
  return (ERROR);
  }

"--"+.*+"\n" {curr_lineno++;}
"--"+.* {}
"*)" {yylval.error_msg = "Unmatched *)"; return (ERROR);}

"\"" {
  null_err_reported = false;
  escaped_null_err_reported = false;
  long_str_err_reported = false;
  string_buf_ptr = string_buf;
  /* The actual max len of string is 1024 */
  string_buf_ptr_end = string_buf_ptr + MAX_STR_CONST - 1;
  BEGIN(STR);
} 
<STR>"\"" {
    BEGIN(INITIAL);
    *string_buf_ptr = '\0';
    /* cout << (int) string_buf_ptr << " " << (int) string_buf_ptr_end << endl; */
    if (long_str_err_reported) {
      yylval.error_msg = "String constant too long";
      return (ERROR);
    }
    if (null_err_reported) {
      yylval.error_msg = "String contains null character";
      return (ERROR);
    }
    if (escaped_null_err_reported) {
      yylval.error_msg = "String contains escaped null character";
      return (ERROR);
    }
    yylval.symbol = stringtable.add_string(string_buf);
    return STR_CONST;
}

<STR>\n {
  BEGIN(INITIAL);
  curr_lineno++;
  yylval.error_msg = "Unterminated string constant";
  return ERROR;
}
<STR>\0 {
  null_err_reported = true;
}
<STR>"\\0" {
  if (string_buf_ptr == string_buf_ptr_end) 
    long_str_err_reported = true;
  *string_buf_ptr++ = '0';
}
<STR>"\\\0" {
  escaped_null_err_reported = true;
}
<STR>"\\b" {
  if (string_buf_ptr == string_buf_ptr_end) 
    long_str_err_reported = true;
  *string_buf_ptr++ = '\b';
}
<STR>"\\t" {
  if (string_buf_ptr == string_buf_ptr_end)
    long_str_err_reported = true;
  *string_buf_ptr++ = '\t';
}
<STR>"\\n" {
  if (string_buf_ptr == string_buf_ptr_end)  {
    long_str_err_reported = true;
  }
  *string_buf_ptr++ = '\n';
}
<STR>"\\f" {
  if (string_buf_ptr == string_buf_ptr_end) 
    long_str_err_reported = true;
  *string_buf_ptr++ = '\f';
}

<STR>\\(.|\n) {
  if (string_buf_ptr == string_buf_ptr_end) {
    long_str_err_reported = true;
  }
  if (yytext[1] == '\n') 
    curr_lineno++;
  *string_buf_ptr++ = yytext[1];
}

<STR>[^\0\\\n\"]+ {
  int i = 0;
  while (yytext[i]) {
    if (string_buf_ptr == string_buf_ptr_end)
      long_str_err_reported = true;
    *string_buf_ptr++ = yytext[i++];
  }

}
<STR><<EOF>> {
  if (eof_err_reported)
    yyterminate();
  yylval.error_msg = "EOF in string constant"; 
  eof_err_reported = true;
  return (ERROR);
} 

 /*
  *  The multiple-character operators.
  */
{INT_CONST} { yylval.symbol = inttable.add_string(yytext); return INT_CONST; }
t(?i:rue) { yylval.boolean = true; return BOOL_CONST;}
f(?i:alse) { yylval.boolean = false; return BOOL_CONST; }

"\n" {curr_lineno += 1;}
" "|"\f"|"\r"|"\t"|"\v" {}


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
"self" { yylval.symbol = idtable.add_string(yytext); return OBJECTID; }
"SELF_TYPE" { yylval.symbol = idtable.add_string(yytext); return TYPEID; }
(?i:{CLASS}) {return (CLASS);}
(?i:{ELSE}) {return (ELSE);}
(?i:{FI}) {return (FI);}
(?i:{IF}) {return (IF);}
(?i:{IN}) {return (IN);}
(?i:{INHERITS}) {return (INHERITS);}
(?i:{LET}) {return (LET);}
(?i:{LOOP}) {return (LOOP);}
(?i:{POOL}) {return (POOL);}
(?i:{THEN}) {return (THEN);}
(?i:{WHILE}) {return (WHILE);}
(?i:{CASE}) {return (CASE);}
(?i:{ESAC}) {return (ESAC);}
(?i:{OF}) {return (OF);}
(?i:{DARROW}) { return (DARROW); }
(?i:{NEW}) {return (NEW);}
(?i:{ISVOID}) {return (ISVOID);}
(?i:{ASSIGN}) {return (ASSIGN);}
(?i:{NOT}) {return (NOT);}
(?i:{LE}) {return (LE);}
"(" {return 40;}
")" {return 41;}
"*" {return 42;}
"+" {return 43;}
"," {return 44;}
"-" {return 45;}
"." {return 46;}
"/" {return 47;}
":" {return 58;}
";" {return 59;}
"<" {return 60;}
"=" {return 61;}
"@" {return 64;}
"{" {return 123;}
"}" {return 125;}
"~" {return 126;}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

{TYPEID} { yylval.symbol = idtable.add_string(yytext); return TYPEID; }
{OBJECTID} { yylval.symbol = idtable.add_string(yytext); return OBJECTID; }
. {yylval.error_msg = yytext; return ERROR;}

%%

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef LEXONLY
#include "y.tab.h"

extern int debug; // declared and set in parser.y
#else

typedef union { int ival; char* str; } yystype;
#define YYSTYPE yystype
yystype yylval;
#define ID  1
#define STRING  2
#define LPAREN  3
#define RPAREN  4
#define LBRACE  5
#define RBRACE  6
#define SEMICOLON  7
#define COMMA 8
#define ADDOP 9
#define NUMBER 10
#define KWINT 11
#define KWSTRING 12
#define EQUALS 13
#define KWWHILE 14
#define KWIF 15
#define KWELSE 16 
#define RELOP 17
#define RBRACKET 18
#define LBRACKET 19
int debug = 1;
#endif
%}

%option yylineno

%%
[ \t\n\r]+ { /* skipping white space */ }

\[  {
      if (debug) printf("lex left bracket: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return LBRACKET;
    }

\]  {
    if (debug) printf("lex right bracket: (%s)\n", yytext);
    yylval.ival = yytext[0];
    return RBRACKET;
    }

"=" {
      if (debug) printf("lex equal: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return EQUALS;
    }

"<" {
      if (debug) printf("lex less than: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return RELOP;
    }

">" {
      if (debug) printf("lex equal: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return RELOP;
    }

"==" {
      if (debug) printf("lex equal: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return RELOP;
    }

"!=" {
      if (debug) printf("lex equal: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return RELOP;
    }

\{ {
      if (debug) printf("lex left brack: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return LBRACE;
    }
\} {
      if (debug) printf("lex right brack: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return RBRACE;
    }

\( {
      if (debug) printf("lex left paran: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return LPAREN;
    }

\) {
      if (debug) printf("lex right paran: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return RPAREN;
    }

\; {
      if (debug) printf("lex SEMICOLON: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return SEMICOLON;
    }

\,  {
      if (debug) printf("lex COMMA: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return COMMA;
    }

\+ {
      if (debug) printf("lex ADDOP: (%s)\n", yytext);
      yylval.ival = yytext[0];
      return ADDOP;
    }

\- {
  if (debug) printf("lex ADDOP: (%s)\n", yytext);
  yylval.ival = yytext[0];
  return ADDOP;
}

"int" {
if (debug) printf("lex kwint: (%s)\n", yytext);
    yylval.str = strdup(yytext);
    return(KWINT);
}// for the keyword int

"char\*" {
    if (debug) printf("lex kwchars: (%s)\n", yytext);
      yylval.str = strdup(yytext);
      return(KWSTRING);
    }// for the keyword cahr

"while" {
    if (debug) printf("lex kwwhile: (%s)\n", yytext);
      yylval.str = strdup(yytext);
      return(KWWHILE);
    }// for the keyword while

"if" {
    if (debug) printf("lex kwif: (%s)\n", yytext);
      yylval.str = strdup(yytext);
      return(KWIF);
    }// for the keyword if

"else" {
    if (debug) printf("lex kwelse: (%s)\n", yytext);
      yylval.str = strdup(yytext);
      return(KWELSE);
    }// for the keyword else

[a-zA-Z][a-zA-Z0-9_]* {
    if (debug) printf("lex id: (%s)\n", yytext);
    yylval.str = strdup(yytext);
    return(ID);
}// for the ID

\"[^\"]*\"  {
      if (debug) printf("lex string const: (%s)\n", yytext);
      yylval.str = strdup(yytext);
      return (STRING);
    }//for const strings

[0-9]+ {
      if (debug) printf("lex NUMBER: (%s)\n", yytext);
      yylval.ival = strtol(yytext,0,10);
      return(NUMBER);
    }// for the NUMBER


%%

#ifdef LEXONLY

// A main for standalone testing (uses just stdin as input)
int main(int argc, char **argv)
{
   do {
      yylex();
   } while (1);
   // while (!feof(stdin)); doesn't work if redirecting a file
   // in because the file is read in and buffered, and eof is
   // true too early, causing lex to stop; rather, use while(1)
   // and then do a hard exit in yywrap(); ugly, but works!
   return 0;
}

int yywrap()
{
   exit(0); // hard exit!
   //return(1);
}

#endif // LEXONLY

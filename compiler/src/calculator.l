%{
// #define YYSTYPE double 
#include "y.tab.h"
#include "../include/express.h"
extern int lineno;
%} 
  
/* Rules Section*/
%%
[ \t]* ;
([0-9])*[.]?[0-9]+              {yylval.value = strtof(yytext,NULL);return NUMBER;}  
"begindecl"                     {return BEGINDECL;}
"enddecl"                       {return ENDDECL;}
"integer"                       {return INTEGER;}
"print"                         {yylval.fName = PRINT;return FNAME;}
[a-zA-Z_][a-zA-Z0-9_]*          {yylval.IDname = strndup(yytext,yyleng); return ID;}
[\n] 				            {lineno++;}
. {return (int)*yytext;};
%%
  
int yywrap(void){return 1;}

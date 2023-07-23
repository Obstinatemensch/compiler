%{
// #define YYSTYPE NODPTR
#include "../bin/y.tab.h"
extern int lineno;
%} 
  
/* Rules Section*/
%%
[ \t]* ;
"decl"                          {return DECL;}
"enddecl"                       {return ENDDECL;}
"begin"				            {return BEG;}	
"end"       					{return END;}	

"main"	                 	    {return MAIN;}
return 				            {return RETURN;}

"integer"                       {return T_INT;}
"boolean"                       {return T_BOOL;}

"if"				            {return IF;}
"then"			    	        {return THEN;}	
"else"			    	        {return ELSE;}	
"endif"			    	        {return ENDIF;}

"do"			    	        {return DO;}
"while"			    	        {return WHILE;}
"endwhile"		    	        {return ENDWHILE;}

"=="				            {return EQUALEQUAL;}
">="                            {return GREATERTHANOREQUAL; }
"<="                            {return LESSTHANOREQUAL; }
"!="                            {return NOTEQUAL; }
AND					            {return LOGICAL_AND;}	
OR					            {return LOGICAL_OR;}
NOT					            {return LOGICAL_NOT;}
true				            {yylval.boolVal = true; return T;}	
false			                {yylval.boolVal = false; return F;}

"read"				            {return READ;}
"write"			                {return WRITE;}


[0-9]*  			            {yylval.intVal = atoi(yytext);  return NUM;}
[a-zA-Z_][a-zA-Z0-9_]*          {yylval.varName = strndup(yytext,yyleng); return VAR;}
[-+*/%=()><;,\{\}\[\]\%]        {return (int)*yytext;}

["]                             {return (int)*yytext;}
"\\n"                           {}
[\n] 				            {lineno++;}	

.								{return (int)*yytext;}	
%%
  
int yywrap(void){return 1;}
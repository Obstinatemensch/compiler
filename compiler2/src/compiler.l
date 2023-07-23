/*
 *   This file is part of SIL Compiler.
 *
 *  SIL Compiler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SIL Compiler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SIL Compiler.  If not, see <http://www.gnu.org/licenses/>.
 */

%{
	
	#define YYSTYPE NODPTR
#include "y.tab.h"
#include "../include/express.h"
extern int lineno;
int     wflag;

%}

%option noyywrap
digit	[0-9]
letter	[a-zA-Z]

%%
main	                 	{ return MAIN;					}	
					
return 			{	return RETURN;
				}
begin				{ return BEG;
					}	
end			{	return END:
				}	
read				{ return READ;
				}
write			{	return WRITE;
					}	
integer			{	yylval.type=INT; return INT;
					}	
boolean				{ yylval.type=BOOL; return BOOL;
					}	
decl				{	return DECL;
				}
enddecl				{	return ENDDECL;
				}
if				{
				}
then				{
					}	
else				{
					}	
endif				{
				}
do				{
				}
while				{
			}
endwhile			{
				}
for 				{
				}		
"=="				{
					}	
"<="				{
				}
">="				{
				}
"!="				{
					}	
AND				{
					}	
OR				{
				}
NOT			{
				}
true			{
					}	
false			      {
				}

([0-9])*[.]?[0-9]+              {yylval.value = strtof(yytext,NULL);return NUMBER;}  

[a-zA-Z_][a-zA-Z0-9_]*          {yylval.IDname = strndup(yytext,yyleng); return ID;}
				
[-+*/=()><;,\{\}\[\]\%]         {
				}
			
["]                             { }
[\n] 				            {lineno++;}
. {return (int)*yytext;};
[ \t]				{}

%%


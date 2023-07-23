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
	#include <stdio.h>
	#include "../include/express.hpp"
// #define YYSTYPE double
	int yylex();
	void yyerror( char* );
        int i;	
%}

%union {
	struct expr * tree;
	char * varName;
	struct varExpr * varexpr;
	int intVal;
	bool boolVal;
	struct declarationNode * declNode;
	struct declaration * decl;
	int type;
	struct statement * stmt;
	struct str_expr * strExp;
	struct main_block * mainBlock;
}

%token BEG END
%token T_INT T_BOOL
%token READ WRITE
%token DECL ENDDECL
%token <varName> VAR 
%token <intVal> NUM
%token IF THEN ELSE ENDIF
%token LOGICAL_AND LOGICAL_NOT LOGICAL_OR
%token EQUALEQUAL LESSTHANOREQUAL GREATERTHANOREQUAL NOTEQUAL
%token WHILE DO ENDWHILE FOR 
%token T F 
%token MAIN RETURN

%type <tree> expr ret_stmt
%type <varexpr> var_expr 
%type <declNode> Gid Glist
%type <type> ret_type func_ret_type
%type <decl> Gdecl_list Gdecl 
%type <stmt> assign_stmt cond_stmt write_stmt 
%type <stmt> statement stmt_list
%type <strExp> str_expr 
%type <mainBlock> MainBlock


%left '<' '>'
%left EQUALEQUAL LESSTHANOREQUAL GREATERTHANOREQUAL NOTEQUAL
%left '+' '-'
%left '*' '/'
%left '%'
%left LOGICAL_AND LOGICAL_OR
%left LOGICAL_NOT
%%

	Prog	:	Gdecl_sec MainBlock 			{ addMainBlock ($2); PrintProgram(); execProg(); printGlobalSymbolTable();return 0; }
			;
		
	Gdecl_sec	:	DECL Gdecl_list ENDDECL		{ addDeclarationBlock ($2);  }
				;
		
	Gdecl_list	:	{$$ = NULL; } 	 					
				| 	Gdecl Gdecl_list  	{$$ = createDeclarationList ($1,$2); }
				;

	Gdecl 	:	ret_type Glist ';' {$$ = createDeclaration ((types)$1,$2); }
			;
		
	ret_type	:	T_INT		{ $$ = Int; }
				|	T_BOOL 		{ $$ = Bool; }
				;
		
	Glist 	:	Gid 			{ $$ = $1; }
			|	Gid ',' Glist 	{ $$ = createDeclList ($1,$3); }
			;
	
	Gid	:	VAR					{ $$ = createDeclNode ($1); }
		|	VAR '[' NUM ']'		{ $$ = createDeclNode ($1,$3); }
		;
				
	func_ret_type:	T_INT		{ $$ = Int; }
		|	T_BOOL				{ $$ = Bool; }
		;
			
	ret_stmt:	RETURN expr ';'	{ $$ = $2; }
		;
			
	MainBlock: 	func_ret_type main '('')''{' BEG stmt_list ret_stmt END  '}'		{ $$ = createMainBlock ((types)$1,NULL,$7,$8); }
					  
		;
		
	main	:	MAIN					{ }
		;

	stmt_list:	/* NULL */			{ $$ = NULL; }
		|	statement stmt_list		{ $$ = createStmtList($1,$2); }
		|	error ';' 				{ $$= NULL; printf ("Error in statement.\n"); exit (1);}
		;

	statement:	assign_stmt  ';'	{ $$ = $1; }   
		|	write_stmt ';'			{ $$ = $1; }
		|	cond_stmt 				{ $$ = $1; }  
		;

	write_stmt:	WRITE '(' expr ')' 			{ $$ = createWriteStmt ($3); }
		 | WRITE '(''"' str_expr '"'')'     { $$ = createWriteStmt ($4); }

		;
	
	assign_stmt:	var_expr '=' expr 					{ $$ = createAssignStmt($1,$3); }
		;

	cond_stmt:	IF expr THEN stmt_list ENDIF 											{ $$ = createIfStmt ($2,$4); }
		|	IF expr THEN stmt_list ELSE stmt_list ENDIF 								{ $$ = createIfElseStmt ($2,$4,$6); }
		|	WHILE expr DO stmt_list ENDWHILE ';'										{ $$ = createWhileStmt ($2,$4); }
		;
	


	expr	:	NUM 					{ $$ = addLeafExpr($1);	}
		|	'-' NUM						{ $$ = addLeafExpr(-$2); }
		|	var_expr					{ $$ = addLeafExpr($1); }
		|	T							{ $$ = addLeafExpr(true); }
		|	F							{ $$ = addLeafExpr(false); }
		|	'(' expr ')'				{ $$ = $2; }
		|	expr '+' expr 				{ $$ = addNonLeafExpr($1 ,'+', $3); }
		|	expr '-' expr	 			{ $$ = addNonLeafExpr($1 ,'-', $3); }
		|	expr '*' expr				{ $$ = addNonLeafExpr($1 ,'*', $3); }
		|	expr '/' expr 				{ $$ = addNonLeafExpr($1 ,'/', $3); }
		|	expr '%' expr 				{ $$ = addNonLeafExpr($1 ,'%', $3); }
		|	expr '<' expr				{ $$ = addNonLeafExpr($1 ,'<', $3); }
		|	expr '>' expr				{ $$ = addNonLeafExpr($1 ,'>', $3); }
		|	expr GREATERTHANOREQUAL expr{ $$ = addNonLeafExpr($1 ,'g', $3); }
		|	expr LESSTHANOREQUAL expr	{ $$ = addNonLeafExpr($1 ,'l', $3); }
		|	expr NOTEQUAL expr			{ $$ = addNonLeafExpr($1 ,'n', $3);	}
		|	expr EQUALEQUAL expr		{ $$ = addNonLeafExpr($1 ,'e', $3); }
		|	LOGICAL_NOT expr			{ $$ = addNonLeafExpr($2 ,'!', $2); } // added both the params as $2 to avoid making a new function.
		|	expr LOGICAL_AND expr		{ $$ = addNonLeafExpr($1 ,'&', $3); }
		|	expr LOGICAL_OR expr		{ $$ = addNonLeafExpr($1 ,'|', $3); }
		;

	str_expr 	:  	VAR                 { $$ = createStrExpr ($1); }
                | 	VAR str_expr   		{ $$ = createStrExpr ($1,$2);}
                ;
	
	var_expr:	VAR					{ $$ = createVarExpr($1); }
		|	VAR '[' expr ']'		{ $$ = createVarExpr($1,$3); }
		;
%%

#include <stdio.h>
#include <ctype.h>
char	*progname;	/* for error messages */
int	lineno = 1;
void warning(char *s, char *t);

int main(int argc,char *argv[])	
{
	progname = argv[0];
	yyparse();
	return 0;
}

void yyerror(char *s)	/* called for yacc syntax error */
{
	warning(s, (char *) 0);
}

void warning(char *s, char *t)	/* print warning message */
{
	fprintf(stderr, "%s: %s", progname, s);
	if (t)
		fprintf(stderr, " %s", t);
	fprintf(stderr, " near line %d\n", lineno);
	return ;
}
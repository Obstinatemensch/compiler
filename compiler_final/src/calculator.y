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
	struct declVarNode * declNode;
	struct declaration * decl;
	int type;
	struct statement * stmt;
	struct str_expr * strExp;
	struct main_block * mainBlock;
	struct Glist * glist;
	struct var_list * varList;
	struct arg_list * argList;
	struct arg * Arg;
	struct func * Func;
	struct Lid_list * lidList;
	struct Fdef * fdef;
	struct exprList * exprlist;
	struct func_call * funCall;
}

%token BEG END
%token T_INT T_BOOL
%token READ WRITE
%token DECL ENDDECL
%token <varName> VAR STRING
%token <intVal> NUM
%token IF THEN ELSE ENDIF
%token LOGICAL_AND LOGICAL_NOT LOGICAL_OR
%token EQUALEQUAL LESSTHANOREQUAL GREATERTHANOREQUAL NOTEQUAL
%token WHILE DO ENDWHILE FOR 
%token T F 
%token MAIN RETURN

%type <tree> expr ret_stmt para
%type <varexpr> var_expr 
%type <declNode> Gid Lid
%type <glist> Glist Lid_list
%type <type> ret_type func_ret_type arg_type type
%type <decl> Gdecl_list Gdecl Ldecl_sec Ldecl Ldecl_list
%type <stmt> assign_stmt cond_stmt write_stmt read_stmt func_stmt
%type <stmt> statement stmt_list
%type <strExp> str_expr 
%type <mainBlock> MainBlock 
%type <varList> var_list
%type <argList> arg_list arg_list1 FargList
%type <Arg> arg
%type <Func> func
%type <varName> func_name
%type <fdef> Fdef Fdef_sec
%type <exprlist> param_list1 param_list
%type <funCall> func_call

%left '<' '>'
%left EQUALEQUAL LESSTHANOREQUAL GREATERTHANOREQUAL NOTEQUAL
%left '+' '-'
%left '*' '/'
%left '%'
%left LOGICAL_AND LOGICAL_OR
%left LOGICAL_NOT
%%

	Prog	:	Gdecl_sec Fdef_sec MainBlock   		{ addFdef_sec($2); addMainBlock ($3); PrintProgram() ;  generate_c_code(); return 0; }
			;
		
	Gdecl_sec	:	DECL Gdecl_list ENDDECL			{ addGlobalDeclarationBlock ($2); }
				;
		
	Gdecl_list	: 						{$$ = NULL; }
				| 	Gdecl Gdecl_list	{$$ = createDeclarationList ($1,$2);}
				;
		
	Gdecl 	:	ret_type Glist ';'		{$$ = createDeclaration ((types)$1,$2);}
			;
		
	ret_type	:	T_INT				{$$ = Int; }
				|	T_BOOL 				{$$ = Bool; }
				;
		
	Glist 	:	Gid						{ $$ = createGlist($1); }
			| 	func 					{ $$ = createGlist($1); }
			|	Gid ',' Glist 			{ $$ = createGlist ($1,$3); }
			|	func ',' Glist			{ $$ = createGlist ($1,$3); }
			;
	
	Gid	:	VAR							{ $$ = createDeclNode ($1); }
		|	Gid '[' NUM ']'				{ $$ = createDeclNode ($1,$3); }

		;
		
	func:	VAR '(' arg_list ')'		{ $$ = createFunc($1,$3); }
		;
			
	arg_list:							{$$ = NULL;}
			|	arg_list1				{$$ = $1;}
			;
		
	arg_list1	:	arg ';' arg_list1 	{$$ = createArgList($1,$3);}
				|	arg					{$$ = createArgList($1);}
				;
		
	arg 	:	arg_type var_list		{ $$ = createArg((types)$1,$2); }
		;
		
	arg_type:	T_INT					{ $$ = Int; }
		|		T_BOOL 		 			{ $$ = Bool; }
		;

	var_list:	VAR 		 			{ $$ = createVarList($1); }
		|	VAR ',' var_list 			{ $$ = createVarList($1,$3); }
		;
		
	Fdef_sec:							{ $$ = NULL; }
		|	 Fdef_sec Fdef 				{ $$ = createFdefSec($2,$1); }
		;
		
	Fdef	:	func_ret_type func_name '(' FargList ')' '{' Ldecl_sec BEG stmt_list ret_stmt END '}'	{ $$ = createFdef((types)$1,$2,$4,$7,$9,$10); }
		;
		
	func_ret_type:	T_INT		{ $$ = Int; }
		|	T_BOOL				{ $$ = Bool; }
		;
			
	func_name:	VAR				{ $$ = $1; }
		;
		
	FargList:	arg_list		{ $$ = $1; }
		;

	ret_stmt:	RETURN expr ';'	{ $$ = $2; }
		;
			
	MainBlock: 	func_ret_type main '('')''{' Ldecl_sec BEG stmt_list ret_stmt END  '}'		{ $$ = createMainBlock ((types)$1,$6,$8,$9); }
					  
		;
		
	main	:	MAIN						{  }
		;
		
	Ldecl_sec	:							{$$ = NULL; }
				|DECL Ldecl_list ENDDECL	{$$ = $2; }
				;

	Ldecl_list:						{$$ = NULL;}
		|	Ldecl Ldecl_list		{$$ = createDeclarationList ($1,$2);}
		;

	Ldecl	:	type Lid_list ';'	{$$ = createDeclaration ((types)$1,$2);}
		;

	type	:	T_INT				{ $$ = Int; }
		|	T_BOOL					{ $$ = Bool; }
		;

	Lid_list:	Lid					{$$ = createGlist($1);}
		|	Lid ',' Lid_list		{$$ = createGlist($1,$3);}
		;
		
	Lid	:	VAR						{ $$ = createDeclNode ($1); }
		;

	stmt_list:	/* NULL */			{ $$ = NULL; }
		|	statement stmt_list		{ $$ = createStmtList($1,$2); }
		|	error ';' 				{ $$= NULL; printf ("Error in statement.\n"); exit (1); }
		;

	statement:	assign_stmt  ';'	{ $$ = $1; }
		|	read_stmt ';'			{ $$ = $1; }
		|	write_stmt ';'			{ $$ = $1; }
		|	cond_stmt 				{ $$ = $1; }
		|	func_stmt ';'			{ $$ = $1; }
		;

	read_stmt:	READ '(' var_expr ')' {	$$ = createReadStmt($3); }
		;

	write_stmt:	WRITE '(' expr ')' 			{ $$ = createWriteStmt ($3); }
		//  | WRITE '(''"' str_expr '"'')'     { $$ = createWriteStmt ($4); }
		| WRITE '('str_expr')'				{ $$ = createWriteStmt ($3); }
		;
	
	assign_stmt:	var_expr '=' expr 					{ $$ = createAssignStmt($1,$3); }
		;

	cond_stmt:	IF expr THEN stmt_list ENDIF 											{ $$ = createIfStmt ($2,$4); }
		|	IF expr THEN stmt_list ELSE stmt_list ENDIF 								{ $$ = createIfElseStmt ($2,$4,$6); }
		|	WHILE expr DO stmt_list ENDWHILE ';'										{ $$ = createWhileStmt ($2,$4); }
		|   FOR '(' assign_stmt  ';'  expr ';'  assign_stmt ')' '{' stmt_list '}'		{ $$ = createForStmt($3,$5,$7,$10); }
		;
	
	func_stmt:	func_call 																{ $$ = createFuncStmt($1); }
		;
		
	func_call:	VAR '(' param_list ')'													{ $$ = createfunc_call($1,$3);}
		;
		
	param_list:																			{ $$ = NULL; }
		|	param_list1																	{ $$ = $1; }
		;
		
	param_list1:	para				{ $$ = createExprList($1);}
		|	para ',' param_list1		{ $$ = createExprList($1,$3);}
		;

	para	:	expr					{ 	$$ = $1;	}
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
		|	func_call					{ $$ = addLeafExpr($1); }
		;

	str_expr :  STRING                 { $$ = createStrExpr ($1); }
                // | VAR str_expr    	{ $$ = createStrExpr ($1,$2); }
                ;
	
	var_expr:	VAR					{ $$ = createVarExpr($1); }
		|	var_expr '[' expr ']'	{ $$ = createVarExpr($1,$3); }
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

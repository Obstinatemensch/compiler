%{
// #define	YYSTYPE	double 	/* data type of yacc stack */
#include <stdio.h>
#include "../include/express.h"

SymbolTable symTable = {0,NULL,NULL};
instruct *codeStart = NULL;
instruct *codeEnd = NULL;
%}


%left	'+' '-'	  /* left associaexive, same precedence */
%left	'*' '/'	  /* left assoc., higher precedence */
%union {
    struct expr * tree;
    double value;
	char* IDname;
	struct symbol* syms;
	struct instruct * ins;
	struct exprList * expList;
	int fName;
}

%start code
%token INTEGER
%token <fName> FNAME
%token <IDname> ID
%token <value> NUMBER
%token ENDDECL BEGINDECL
%type <tree> expr
%type <syms> IDs
%type <expList> exprs
%type <ins> declaration function assignment
/* %type <value>  */

%%
code 	: statements	{ exec_code(codeStart);printCode();return 0;}
		;
statements 	: statement statements
			| statement 
			;

statement	: BEGINDECL declarationBlock ENDDECL    
			| assignment ';'    					{addinstruct($1);/*printCode();executeinstruct($2);*/}
			| function ';'   						{addinstruct($1);/*printCode();executeinstruct($2);*/}
			;

expr:	  NUMBER	{ $$ = addLeafExpr($1); }
	| 		ID		/*{ $$ = getVal($1)} */ {$$ = addLeafVariable($1);}
	| expr '+' expr { $$ = addNonLeafExpr($1 ,'+', $3); }
	| expr '-' expr	{ $$ = addNonLeafExpr($1 ,'-', $3); }
	| expr '*' expr	{ $$ = addNonLeafExpr($1 ,'*', $3); }
	| expr '/' expr	{ $$ = addNonLeafExpr($1 ,'/', $3); }
	| '(' expr ')'	{ $$ = $2; }
	;

declarationBlock: declarationBlock declaration ';' 	{addinstruct($2);}
			 	| declaration ';' 					{addinstruct($1);}
				;

declaration:	DATATYPE IDs {$$ = returnDeclaration($2,INT);}
		   ;

DATATYPE:		INTEGER
		;

exprs:			expr ',' exprs 	{$$ = addExpression ($1,$3);  }
	 |			expr 				{$$ = addExpression ($1,NULL);}
	 ;

IDs:			ID ',' IDs 	{$$ = addSymbol($1,$3);} 
   |			ID 			{$$ = addSymbol($1,NULL);}
   ;

assignment:		ID '=' expr {$$ = returnAssignment($1,$3);}
		  ;

function:		FNAME '('exprs')' {$$ = returnFunction($1,$3);}
		;

%%
	/* end of grammar */

#include <stdio.h>
#include <ctype.h>
char	*progname;	/* for error messages */
int	lineno = 1;

main(argc, argv)	
	char *argv[];
{
	progname = argv[0];
	yyparse();
}

yyerror(s)	/* called for yacc syntax error */
	char *s;
{
	warning(s, (char *) 0);
}

warning(s, t)	/* print warning message */
	char *s, *t;
{
	fprintf(stderr, "%s: %s", progname, s);
	if (t)
		fprintf(stderr, " %s", t);
	fprintf(stderr, " near line %d\n", lineno);
}

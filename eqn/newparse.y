%{
#include <stdio.h>
#include <string.h>
#include "eqn.h"

#define grballoc(node) (node *) malloc(sizeof(node))
#define yylex() eelex()      /* redefine the name of lexical analyiser */
#define yyparse() eeparse()  /* and the parser  */

extern eqnode *equation_base;
extern char errorstring[];
extern int  ptrerrstring;

%}

%union{
	eqnode *y_equ;
	double y_dbl;
	char   *y_str;
      };

%token YYLBRACE YYRBRACE
%token <y_dbl> YYNUMBER
%token <y_str> YYNAME
%type <y_equ> expr

%right YYEQUAL
%left YYPLUS YYMINUS
%nonassoc UMINUS
%left YYTIMES YYDEVIDE  
%left ASSOCIATE  
%left YYPOWER

%%

equation : expr YYEQUAL expr
	{
	  equation_base = grballoc( eqnode );
	  equation_base->op = '=';
	  equation_base->u.n.l = $1;
	  equation_base->u.n.r = $3;
	}
	;

expr:  expr YYPLUS expr
	{
	  $$ = grballoc( eqnode );
	  $$->op = '+';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| expr YYMINUS expr
	{
	  $$ = grballoc( eqnode );
	  $$->op = '-';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| expr YYTIMES expr
	{
	  $$ = grballoc( eqnode );
	  $$->op = '*';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| expr YYDEVIDE expr 
	{
	  $$ = grballoc( eqnode );
	  $$->op = '/';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| expr expr %prec ASSOCIATE
	{
	  $$ = grballoc( eqnode );
	  $$->op = '*';
	  $$->u.n.l = $1;
	  $$->u.n.r = $2;
	}
	| expr YYPOWER expr
	{
	  $$ = grballoc( eqnode );
	  $$->op = '^';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| YYMINUS expr %prec UMINUS
	{
	  $$ = grballoc( eqnode );
	  $$->op = '-';
	  $$->u.n.l = grballoc( eqnode );
	  $$->u.n.l-> op = NUMBER;
	  $$->u.n.l->u.num = (double) 0;
	  $$->u.n.r = $2;
	}
	| YYPLUS expr %prec UMINUS
	{
	  $$ = $2;
	}
	| YYNAME
	{
	  $$ = grballoc( eqnode );
	  $$->op = NAME;
	  $$->u.str = (char *) calloc( strlen($1) , sizeof( char ) );
	  strcpy($$->u.str,$1);
	}
	| YYNUMBER
	{
	  $$ = grballoc( eqnode );
	  $$->op = NUMBER;
	  $$->u.num = $1;
	}
	| YYLBRACE expr YYRBRACE
	{
	  $$ = $2;
	}
%%
yyerror(string)
char *string;
{
  int i;

  fprintf(stderr,"yyerror: %s\n",string);
  fprintf(stderr,"equation to date is :-\n");

  for( i = 0; i< ptrerrstring; ++i ) putc( errorstring[i], stderr );

  fprintf(stderr,"\n");
}

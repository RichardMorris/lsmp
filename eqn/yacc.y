%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eqn.h"

#define grballoc(node) (node *) malloc(sizeof(node))
/* seem to need yyparse(z) on cygwin but not on scs */
#ifdef YYPARSE_ARGS
#define yyparse(z) eeparse(z)  
#else
#define yyparse() eeparse()  
#endif

/*  #define gettxt(a,b) a  */

extern eqnode *equation_base;
extern char errorstring[];
extern int  ptrerrstring;
eqnode *zap;

%}

%union{
	eqnode *y_equ;
	double y_dbl;
	char   *y_str;
	eqn_funs *y_fun;
      };

%token YYLBRACE YYRBRACE YYLSQUARE YYRSQUARE YYCOLON 
%token <y_dbl> YYNUMBER
%token <y_str> YYNAME
%token <y_fun> YYFUNNAME YYCONSTANT
%type <y_equ> expr summand prod simppow simp

%right YYEQUAL
%nonassoc YYDOTDOT
%right YYCOMMA
%left YYPLUS YYMINUS
%nonassoc UMINUS
%left YYTIMES YYDEVIDE YYDOT
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
	| expr
	{
	  equation_base = $1;
	}
	|
	{
	  equation_base = NULL;
	}
	;

expr:	summand YYDOTDOT summand
	{
	  $$ = grballoc( eqnode );
	  $$->op = INTERVAL;
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	|  expr YYCOMMA expr
	{
	  $$ = grballoc( eqnode );
	  $$->op = ',';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| summand;

/* a sum of products */

summand: summand YYPLUS prod
	{
	  $$ = grballoc( eqnode );
	  $$->op = '+';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| summand YYMINUS prod
	{
	  $$ = grballoc( eqnode );
	  $$->op = '-';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| summand YYPLUS YYMINUS prod
	{
	  $$ = grballoc( eqnode );
	  $$->op = '-';
	  $$->u.n.l = $1;
	  $$->u.n.r = $4;
	}
	| summand YYMINUS YYPLUS prod
	{
	  $$ = grballoc( eqnode );
	  $$->op = '-';
	  $$->u.n.l = $1;
	  $$->u.n.r = $4;
	}
	| YYMINUS prod %prec UMINUS
        {
          $$ = grballoc( eqnode );
	  $$->op = '*';
	  $$->u.n.l = grballoc(eqnode);
	  $$->u.n.l->op = NUMBER;
	  $$->u.n.l->u.num = (double) -1.0;
  	  $$->u.n.r = $2;
        }
        | YYPLUS prod %prec UMINUS
        {
          $$ = $2;
        }
	| prod;

/* a product of terms */

prod:	simppow
	| prod YYTIMES simppow 
	{
	  $$ = grballoc( eqnode );
	  $$->op = '*';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| prod YYDOT simppow
	{
	  $$ = grballoc( eqnode );
	  $$->op = '.';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| prod YYDEVIDE simppow
	{
	  $$ = grballoc( eqnode );
	  $$->op = '/';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| prod YYTIMES YYMINUS simppow 
	{
	  $$ = grballoc( eqnode );
	  $$->op = '*';
	  $$->u.n.l = $1;
	  $$->u.n.r = grballoc( eqnode );
	  $$->u.n.r->op = '-';
	  $$->u.n.r->u.n.l = grballoc( eqnode );
	  $$->u.n.r->u.n.l->op = NUMBER;
	  $$->u.n.r->u.n.l->u.num = 0.0;
	  $$->u.n.r->u.n.r = $4;
	}
	| prod YYDEVIDE YYMINUS simppow 
	{
	  $$ = grballoc( eqnode );
	  $$->op = '/';
	  $$->u.n.l = $1;
	  $$->u.n.r = grballoc( eqnode );
	  $$->u.n.r->op = '-';
	  $$->u.n.r->u.n.l = grballoc( eqnode );
	  $$->u.n.r->u.n.l->op = NUMBER;
	  $$->u.n.r->u.n.l->u.num = 0.0;
	  $$->u.n.r->u.n.r = $4;
	}
	| prod simppow %prec ASSOCIATE
	{
	  $$ = grballoc( eqnode );
	  $$->op = '*';
	  $$->u.n.l = $1;
	  $$->u.n.r = $2;
	};

/* x or x^y or x^-y */
simppow:  simp
	| simp YYPOWER simp
	{
	  $$ = grballoc( eqnode );
	  $$->op = '^';
	  $$->u.n.l = $1;
	  $$->u.n.r = $3;
	}
	| simp YYPOWER YYMINUS simp
	{
	  $$ = grballoc( eqnode );
	  $$->op = '^';
	  $$->u.n.l = $1;
	  $$->u.n.r = grballoc( eqnode );
	  $$->u.n.r->op = '-';
	  $$->u.n.r->u.n.l = grballoc( eqnode );
	  $$->u.n.r->u.n.l->op = NUMBER;
	  $$->u.n.r->u.n.l->u.num = 0.0;
	  $$->u.n.r->u.n.r = $4;
	}
	;

/* just something simple x, 5, ( ), pi, sin x [,] */
simp:   YYNAME
	{
	  zap = grballoc( eqnode );
	  $$ = zap;
	  $$->op = NAME;
	  $$->u.str = (char *) calloc( strlen($1)+1 , sizeof( char ) );
	  strcpy($$->u.str,$1);
	}
	| YYNUMBER
	{
	  $$ = grballoc( eqnode );
	  $$->op = NUMBER;
	  $$->u.num = $1;
	}
	| YYFUNNAME simp
	{
	  $$ = grballoc( eqnode );
	  $$->op = FUNCTION;
	  $$->u.f.f = $1;
	  $$->u.f.a = $2;
	}
	| YYCONSTANT
	{
	  $$ = grballoc( eqnode );
	  $$->op = FUNCTION;
	  $$->u.f.f = $1;
	  $$->u.f.a = NULL;
	}
	| YYLBRACE expr YYRBRACE
	{
	  $$ = $2;
	}
	| YYLSQUARE expr YYCOMMA expr YYRSQUARE
	{
	  $$ = grballoc( eqnode );
	  $$->op = INTERVAL;
	  $$->u.n.l = $2;
	  $$->u.n.r = $4;
	};
%%
yyerror(string)
char *string;
{

  eprintf("%s: ",string);
  /*
  int i;
  eprintf("equation to date is :-\n");
  
  for( i = 0; i< ptrerrstring; ++i ) putc( errorstring[i], stderr );
  */
  errorstring[ptrerrstring] = '\0';

  eprintf("%s\n",errorstring);
}

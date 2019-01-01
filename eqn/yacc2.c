#include <stdio.h>
#include <string.h>
#include "eqn.h"

#define grballoc(node) (node *) malloc(sizeof(node))
#define yyparse() eeparse()  /* and the parser  */

extern eqnode *equation_base;
extern char errorstring[];
extern int  ptrerrstring;
eqnode *zap;

typedef union {
	eqnode *y_equ;
	double y_dbl;
	char   *y_str;
      } YYSTYPE;
# define YYLBRACE 257
# define YYRBRACE 258
# define YYLSQUARE 259
# define YYRSQUARE 260
# define YYCOMMA 261
# define YYCOLON 262
# define YYDOTDOT 263
# define YYNUMBER 264
# define YYNAME 265
# define YYEQUAL 266
# define YYPLUS 267
# define YYMINUS 268
# define UMINUS 269
# define YYTIMES 270
# define YYDEVIDE 271
# define ASSOCIATE 272
# define YYPOWER 273
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

yyerror(string)
char *string;
{
  int i;

  fprintf(stderr,"yyerror: %s\n",string);
  fprintf(stderr,"equation to date is :-\n");

  for( i = 0; i< ptrerrstring; ++i ) putc( errorstring[i], stderr );

  fprintf(stderr,"\n");
}
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 19
# define YYLAST 142
yytabelem yyact[]={

     8,    20,     9,    34,    12,     1,    11,     7,     6,     0,
    13,    14,     5,    15,    16,     8,    31,     9,     0,    12,
     0,    11,     7,     6,     0,    13,    14,     0,    15,    16,
     8,     0,     9,    30,    12,     0,    11,     7,     6,    10,
    13,    14,     0,    15,    16,     8,     0,     9,     0,    12,
     0,    11,     7,     6,     0,    13,    14,     0,    15,    16,
     8,     0,     9,     0,    32,     0,    11,     7,     6,     0,
    13,    14,     0,    15,    16,     8,     0,     9,     0,     0,
     0,    11,     7,     6,     8,     0,     9,     0,    15,    16,
     0,     7,     6,     0,     4,     3,     8,     0,     9,     8,
     0,     9,    11,     7,     6,     0,     7,     6,    17,     2,
     0,     0,    18,    19,     0,     0,     0,    21,    22,    23,
    24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    33 };
yytabelem yypact[]={

  -173, -1000,  -227,  -173,  -173,  -272, -1000, -1000,  -173,  -173,
  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -161,  -182,  -182,
  -158,  -242,  -197,  -212,  -212,  -212,  -182,  -182,  -161,  -161,
 -1000, -1000,  -173,  -257, -1000 };
yytabelem yypgo[]={

     0,   108,    12,     5 };
yytabelem yyr1[]={

     0,     3,     3,     3,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     2,     2,     2,     2 };
yytabelem yyr2[]={

     0,     7,     3,     1,     7,     7,     7,     7,     7,     7,
     5,     5,     5,     2,     7,     3,     3,     7,    11 };
yytabelem yychk[]={

 -1000,    -3,    -1,   268,   267,    -2,   265,   264,   257,   259,
   266,   263,   261,   267,   268,   270,   271,    -1,    -1,    -1,
   273,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -2,   258,   261,    -1,   260 };
yytabelem yydef[]={

     3,    -2,     2,     0,     0,    13,    15,    16,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    10,    11,    12,
     0,     0,     0,     1,     4,     5,     6,     7,     8,     9,
    14,    17,     0,     0,    18 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"YYLBRACE",	257,
	"YYRBRACE",	258,
	"YYLSQUARE",	259,
	"YYRSQUARE",	260,
	"YYCOMMA",	261,
	"YYCOLON",	262,
	"YYDOTDOT",	263,
	"YYNUMBER",	264,
	"YYNAME",	265,
	"YYEQUAL",	266,
	"YYPLUS",	267,
	"YYMINUS",	268,
	"UMINUS",	269,
	"YYTIMES",	270,
	"YYDEVIDE",	271,
	"ASSOCIATE",	272,
	"YYPOWER",	273,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"equation : expr YYEQUAL expr",
	"equation : expr",
	"equation : /* empty */",
	"expr : expr YYDOTDOT expr",
	"expr : expr YYCOMMA expr",
	"expr : expr YYPLUS expr",
	"expr : expr YYMINUS expr",
	"expr : expr YYTIMES expr",
	"expr : expr YYDEVIDE expr",
	"expr : expr expr",
	"expr : YYMINUS expr",
	"expr : YYPLUS expr",
	"expr : simp",
	"expr : simp YYPOWER simp",
	"simp : YYNAME",
	"simp : YYNUMBER",
	"simp : YYLBRACE expr YYRBRACE",
	"simp : YYLSQUARE expr YYCOMMA expr YYRSQUARE",
};
#endif /* YYDEBUG */
/* 
 *	Copyright 1987 Silicon Graphics, Inc. - All Rights Reserved
 */

/* #ident	"@(#)yacc:yaccpar	1.10" */
#ident	"$Revision: 1.5 $"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax errorB" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:{
	  equation_base = grballoc( eqnode );
	  equation_base->op = '=';
	  equation_base->u.n.l = yypvt[-2].y_equ;
	  equation_base->u.n.r = yypvt[-0].y_equ;
	} break;
case 2:{
	  equation_base = yypvt[-0].y_equ;
	} break;
case 3:{
	  equation_base = NULL;
	} break;
case 4:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = INTERVAL;
	  yyval.y_equ->u.n.l = yypvt[-2].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 5:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = ',';
	  yyval.y_equ->u.n.l = yypvt[-2].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 6:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = '+';
	  yyval.y_equ->u.n.l = yypvt[-2].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 7:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = '-';
	  yyval.y_equ->u.n.l = yypvt[-2].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 8:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = '*';
	  yyval.y_equ->u.n.l = yypvt[-2].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 9:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = '/';
	  yyval.y_equ->u.n.l = yypvt[-2].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 10:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = '*';
	  yyval.y_equ->u.n.l = yypvt[-1].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 11:{
          yyval.y_equ = grballoc( eqnode );
          yyval.y_equ->op = '-';
          yyval.y_equ->u.n.l = grballoc( eqnode );
          yyval.y_equ->u.n.l-> op = NUMBER;
          yyval.y_equ->u.n.l->u.num = (double) 0;
          yyval.y_equ->u.n.r = yypvt[-0].y_equ;
        } break;
case 12:{
          yyval.y_equ = yypvt[-0].y_equ;
        } break;
case 14:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = '^';
	  yyval.y_equ->u.n.l = yypvt[-2].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-0].y_equ;
	} break;
case 15:{
	  zap = grballoc( eqnode );
	  yyval.y_equ = zap;
	  yyval.y_equ->op = NAME;
	  yyval.y_equ->u.str = (char *) calloc( strlen(yypvt[-0].y_str)+1 , sizeof( char ) );
	  strcpy(yyval.y_equ->u.str,yypvt[-0].y_str);
	} break;
case 16:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = NUMBER;
	  yyval.y_equ->u.num = yypvt[-0].y_dbl;
	} break;
case 17:{
	  yyval.y_equ = yypvt[-1].y_equ;
	} break;
case 18:{
	  yyval.y_equ = grballoc( eqnode );
	  yyval.y_equ->op = INTERVAL;
	  yyval.y_equ->u.n.l = yypvt[-3].y_equ;
	  yyval.y_equ->u.n.r = yypvt[-1].y_equ;
	} break;
	}
	goto yystack;		/* reset registers in driver code */
}

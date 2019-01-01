%{
/*
 * Copyright I guess there should be some copywrite for this package,
 * 
 * 			Copyright (c) 1992
 * 
 * 	Liverpool University Department of Pure Mathematics,
 * 	Liverpool, L69 3BX, England.
 * 
 * 	Author Dr R. J. Morris.
 * 
 * 	e-mail rmorris@uk.ac.liv.uxb
 *
 * This software is copyrighted as noted above.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is
 * preserved on all copies.
 *
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the authors, who may or may not act on them as they desire.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 *
 * All this software is public domain, as long as it is not used by any military
 * establishment. Please note if you are a military establishment then a mutating
 * virus has now escaped into you computer and is presently turning all your
 * programs into socially useful, peaceful ones.
 * 
 */
/*****************************************************************************/
/*                                                                           */
/*  multiplecharacter.c  a lexical analysier for equations                   */
/*    this version treats varibles as being a multiple character             */
/*    alpha-numeric strings which must be separated by a non alpha-numeric   */
/*    character.                                                             */
/*    for portability this has been written in 'C' so it can run on the IBM  */ 
/*                                                                           */
/*****************************************************************************/
#define  I_AM_LEX
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "eqn.h"
#ifdef __BORLANDC__
#include "ytab.h"
#else
#include "y.tab.h"
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !(FALSE)
#endif
#define grballoc(node) (node *)malloc(sizeof(node))
#define yyparse() eeparse()
#ifdef NOT_DEF
#define YY_INPUT(buf,result,max_size) \
	if ( yy_current_buffer->yy_is_interactive ) \
		{ \
		int c = '*', n; \
		for ( n = 0; n < max_size && \
			     (c = getc( yyin )) != EOF && c != '\n' \
				&& c != ';' && ; ++n ) {\
		    if(c == '#') { \
			while( (c = getc( yyin )) != EOF && c != '\n' );\
			if ( c == '\n' ) \
				{ buf[n++] = (char) c; break; } \
			if ( c == EOF && ferror( yyin ) ) \
				YY_FATAL_ERROR( "input in flex scanner failed" ); \
			--n;
			}
			else
				buf[n] = (char) c; \
		if ( c == '\n' ) \
			buf[n++] = (char) c; \
		if ( c == EOF && ferror( yyin ) ) \
			YY_FATAL_ERROR( "input in flex scanner failed" ); \
		result = n; \
		} \
	else  \
		{ \
		int c = '*', n; \
		for ( n = 0; n < max_size && \
			     (c = getc( yyin )) != EOF && c != ';' && ; ++n ) {\
			if(c == '#') { \
			{
			while( (c = getc( yyin )) != EOF && c != '\n' );\
		if ( c == EOF && ferror( yyin ) ) \
			YY_FATAL_ERROR( "input in flex scanner failed" ); \
				--n;
			}
			else
				buf[n] = (char) c; \
		if ( c == '\n' ) \
			buf[n++] = (char) c; \
		if ( c == EOF && ferror( yyin ) ) \
			YY_FATAL_ERROR( "input in flex scanner failed" ); \
		result = n; \
		} \
		  && ferror( yyin ) ) \
		YY_FATAL_ERROR( "input in flex scanner failed" );
#endif

extern eqn_funs *base_funs;
%}

%%
"("	return( YYLBRACE );
")"	return( YYRBRACE );
"["	return( YYLSQUARE );
"]"	return( YYRSQUARE );
".."	return( YYDOTDOT );
"+"	return( YYPLUS   );
"*"	return( YYTIMES  );
"-"	return( YYMINUS  );
"/"	return( YYDEVIDE );
"^"	return( YYPOWER );
"**"	return( YYPOWER );
"="	return( YYEQUAL );
":="	return( YYEQUAL );
","	return( YYCOMMA );
":"	return( YYCOLON );
[0-9]*\.?[0-9]* {
		char c1;

		if(yyleng == 1 && yytext[0] == '.' ) return(YYDOT);
		if(yytext[yyleng-1] == '.')
		{
			c1 = input();	/* test for 1..1 */
			if( c1 == '.' )
			{
				unput(c1);
				yytext[yyleng-1] = '\0';
				--yyleng;
#ifdef NOT_DEF
				yyless(yyleng-1);
#endif
				unput(c1); /* Puts back two .'s */
			}
			else unput(c1);
		}
	
		/* now test for exponants */
#ifdef NOT_DEF
		c1 = input();
        	if( c1 == 'e' || c1 == 'E' )
		{
			c1 = input();
			if( c1 == '-' ) 
			{
				c1 = input();
				if( isdigit(c1) )
				{
					yytext[yyleng++] = 'e';
					yytext[yyleng++] = '-';
					while( isdigit(c1) )
					{
						yytext[yyleng++] = c1;
						c1 = input();
					}
					unput(c1);
					yytext[yyleng] = '\0';
				}
				else
				{
					unput(c1);
					unput('-');
					unput('e');
				}
			}
			else if( isdigit(c1) )
			{
				yytext[yyleng++] = 'e';
				while( isdigit(c1) )
				{
					yytext[yyleng++] = c1;
					c1 = input();
				}
				yytext[yyleng] = '\0';
				unput(c1);
			}
			else
			{
				unput(c1);
				unput('e');
			}
		}
		else unput(c1);
#endif
	        yylval.y_dbl = atof(yytext); 
      		return(YYNUMBER);
        }
[?A-Za-z_][A-Za-z0-9_]*[']*	{
		eqn_funs *fun;

		/* First test the name for a function */

		fun = base_funs;
		while( fun != NULL )
		{
			if(!strcmp(fun->name,yytext))
			{
			    if(fun->type == EXTERN_FUN 
			     || fun->type == INTERN_FUN 
			     || fun->type == EXTERN_MAP 
			     || fun->type == INTERN_MAP 
			     || fun->type == SUM_FUN
			     || fun->type == OPERATOR )
			    {
				yylval.y_fun = fun;
				return(YYFUNNAME);
			    }
			    else if(fun->type == CONSTANT_FUN )
			    {
				yylval.y_fun = fun;
				return(YYCONSTANT);
			    }
			    else
			    {
				eprintf("Bad function type %d for %s\n",
					fun->type,fun->name);
			    }
			}
			fun = fun->next;
		}
		/* Now its just a name */

                yylval.y_str = (char *) calloc(yyleng + 1,sizeof(char));
                strcpy(yylval.y_str,yytext);
#ifdef NOT_DEF
		c1 = input();
		if( c1 == '(' ) return(YYFUNNAME);
		unput(c1);
#endif
                return(YYNAME);
        }
"/*"	{
		int  c1, c2;

		do 
		{
			while ((c1 = input()) != '*' && c1 != EOF) ;
			c2 = input();
		
		} while (c2 != '/' && c2 != EOF);
		
		if (c1 == EOF || c2 == EOF)
		{
      			eprintf("unterminated comment \n");
      			exit(-3);
		}
	}
[ \n\t\r]	;
[;]	return(0);
#.*$	;	/* remove coments */
\0	return(0);
\d	return(0);
">"	return(YYMINUS);
"<"	return(YYMINUS);
"!"	;
"{"	;
"}"	;

%%

char errorstring[1024];
int  ptrerrstring = 0;
eqnode *equation_base;
int yywrap()
{
return 1;
}


/*** input from stdin ****/

eqnode *scan_eqn()
{
  ptrerrstring = 0;
  yyin = stdin;
  yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));
  if(yyparse()) equation_base = NULL;
  yy_delete_buffer(YY_CURRENT_BUFFER);
  return(equation_base);
}

/**** input from a file ****/

eqnode *fscan_eqn(fp)
FILE *fp;
{
  ptrerrstring = 0;
 	if( yyin == fp )
	{
		if( fileno(yyin) != fileno(fp) )
		{
			fprintf(stderr,
		"error in lexical analysis, file numbers different %d %d\n",
			fileno(yyin),fileno(fp));
			exit(-1);
		}
		if(yyparse()) return(NULL);
		return(equation_base);
	}
  yyin = fp;
  yy_delete_buffer(YY_CURRENT_BUFFER);
  yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));
  if(yyparse()) return(NULL);
  return(equation_base);
}

/**** input from a string ****/

eqnode *sscan_eqn(string)
char *string;
{
  YY_BUFFER_STATE buf;

  buf = yy_scan_string(string);
  if(yyparse()) equation_base = NULL;
  yy_delete_buffer(buf);
  if(equation_base == NULL) fprintf(stderr,"Null equ_base\n");
  return(equation_base);
}


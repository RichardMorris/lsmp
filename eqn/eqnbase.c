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
/*
#define SUN_SYS
*/
#define VARARGS
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#ifdef VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif
#define I_AM_EQNBASE
#include "eqn.h"
/*
#include <values.h>
#define SORT_ADD
#define PRINT_EXPANSION
#define PRINT_DIFF_FUN
*/
#define SILLYFUNS

/* this is now defined from the makefile
#define USING_FLEX
*/
#ifdef USING_FLEX
extern FILE *yyin;
#endif
#define grballoc(node) (node *) malloc(sizeof(node))
#define MAX(a,b)       a > b ? a : b ;
#define TRUE 1
#define FALSE 0

/*************************************************************************/
/*                                                                       */
/*  Global Varibles                                                      */
/*                                                                       */
/*************************************************************************/

int eqnin_mode;     /* mode of input file/stdin/string */
FILE *eqnin_file;   /* input file for equation         */
char *eqnin_str;    /* input string for equation       */
eqnode *equation_base; /* a pointer the base of the equation tree being read */
                       /* this is assigned in eeparse                        */

extern int ptrerrstring;
char      eqn_err_string[256];
int    eqn_err_flag = FALSE;
int    eqn_errs_to_stderr = TRUE;
/*
 * Generic print statement for error output
 */

#ifdef VARARGS
int eprintf(va_alist ) va_dcl
#else
int eprintf(char * format, ... ) 
#endif
{
    int result;
    va_list ap;
    char tmp[256];

#ifdef VARARGS
    char *format;
    va_start(ap);
    format = va_arg(ap, char *);
#else
    va_start(ap,format);
#endif
    if( eqn_errs_to_stderr )
    {
	result = vfprintf(stderr,format,ap);
    }
    else
    {
    	eqn_err_flag = TRUE;
	result = vsprintf(tmp,format,ap);
	strncat(eqn_err_string,tmp,255-strlen(eqn_err_string));
    }
    va_end(ap);
    return result;
}

void reset_eqn_errs()
{
   eqn_err_flag = FALSE;
   eqn_err_string[0] = '\0';
}

#ifdef __BORLANDC__
int matherr(struct exception *a)
{
  switch(a->type)
  {
  case  DOMAIN:
	eprintf("Out of range, function %s value %f",a->name,a->arg1);
        a->retval = MAXDOUBLE;
	break;
  case SING:
  	eprintf("Bad argument, function %s value %f",a->name,a->arg1);
	a->retval = MAXDOUBLE;
	break;
  case OVERFLOW:
	eprintf("Overflow, function %s value %f",a->name,a->arg1);
	a->retval = MAXDOUBLE;
	break;
  case UNDERFLOW:
	eprintf("Underdlow, function %s value %f",a->name,a->arg1);
	a->retval = 0.0;
	break;
  case TLOSS:
	eprintf("Can't calculate, function %s value %f",a->name,a->arg1);
	a->retval = MAXDOUBLE;
	break;
  }
  return 1;
}
#endif
/*************************************************************************/
/*                                                                       */
/* first some subroutines to call the parser                             */
/*                                                                       */
/*************************************************************************/

#ifndef USING_FLEX
/**** input from stdin ****/

eqnode *scan_eqn()
{
  eqnin_mode = EQNFROM_STDIN;
  ptrerrstring = 0;
  eeparse();
  return(equation_base);
}

/**** input from a file ****/

eqnode *fscan_eqn(fp)
FILE *fp;
{
  eqnin_mode = EQNFROM_FILE;
  eqnin_file = fp;
  ptrerrstring = 0;
  eeparse();
  return(equation_base);
}

/**** input from a string ****/

eqnode *sscan_eqn(string)
char *string;
{
  char stringblock[256];

  eqnin_mode = EQNFROM_STRING;
  if( strlen(string) >= 255 ) 
  {
    eprintf("sscan_eqn: string to long <%s>\n",string);
    exit(-1);
  }
  eqnin_str = stringblock;
  strcpy( eqnin_str, string);
  ptrerrstring = 0;
  eeparse();
  return(equation_base);
}
#endif

/*************************************************************************/
/*                                                                       */
/* Now some routines for basic tree handeling                            */
/*                                                                       */
/*************************************************************************/


#ifdef __BORLANDC__
#endif
#define rint(a) floor(0.5+a)

/* prints the equation to a string
   needs the following function which puts the output at the end of the string */

#ifdef VARARGS
int csprintf(va_alist ) va_dcl
#else
int csprintf(char *string, char *format, ...)
#endif
{
    int result;
    va_list ap;
    char tmp[256];

#ifdef VARARGS
    char *string;
    char *format;
    va_start(ap);
    string = va_arg(ap, char *);
    format = va_arg(ap, char *);
#else
    va_start(ap,format);
#endif
    result = vsprintf(tmp,format,ap);
    strncat(string,tmp,255-strlen(string));
    va_end(ap);
    return result;
}

void fprint_num(FILE *file,double num)
{
	short j;
    if( num == rint(num) ) 
	fprintf(file,"%.0lf",num);
    else if( num*10.0 == rint(num*10.0) ) 
	fprintf(file,"%.1lf",num);
    else if( num*100.0 == rint(num*100.0) ) 
	fprintf(file,"%.2lf",num);
    else if( num*1000.0 == rint(num*1000.0) ) 
	fprintf(file,"%.3lf",num);
    else if( num*10000.0 == rint(num*10000.0) ) 
	fprintf(file,"%.4lf",num);
    else
    {
	j = (int) rint(-log10(num));
	if(j>6)
		fprintf(file,"%.*lf",j+2,num);
	else
		fprintf(file,"%.8lf",num);
    }
}

void csprint_num(char *string,double num)
{
	short j;
    if( num == rint(num) ) 
	csprintf(string,"%.0lf",num);
    else if( num*10.0 == rint(num*10.0) ) 
	csprintf(string,"%.1lf",num);
    else if( num*100.0 == rint(num*100.0) ) 
	csprintf(string,"%.2lf",num);
    else if( num*1000.0 == rint(num*1000.0) ) 
	csprintf(string,"%.3lf",num);
    else if( num*10000.0 == rint(num*10000.0) ) 
	csprintf(string,"%.4lf",num);
    else
    {
	j = (int) rint(-log10(num));
	if(j>6)
		csprintf(string,"%.*lf",j+2,num);
	else
		csprintf(string,"%.8lf",num);
    }
}

void eprint_num(double num)
{
	short j;
    if( num == rint(num) ) 
	eprintf("%.0lf",num);
    else if( num*10.0 == rint(num*10.0) ) 
	eprintf("%.1lf",num);
    else if( num*100.0 == rint(num*100.0) ) 
	eprintf("%.2lf",num);
    else if( num*1000.0 == rint(num*1000.0) ) 
	eprintf("%.3lf",num);
    else if( num*10000.0 == rint(num*10000.0) ) 
	eprintf("%.4lf",num);
    else
    {
	j = (int) rint(-log10(num));
	if(j>6)
		eprintf("%.*lf",j+2,num);
	else
		eprintf("%.8lf",num);
    }
}
	

void fprint_op(fp,op)
FILE *fp;
int op;
{
	switch(op)
	{
		case '+': case '-': case '*': case '/': case '=': 
		case ',': case '^': case '.':
		fprintf(fp,"'%c'",op);
		case NUMBER:
			fprintf(fp,"NUMBER"); break;
		case INTERVAL:
			fprintf(fp,"INTERVAL"); break;
		case FUNCTION:
			fprintf(fp,"FUNCTION"); break;
		case NAME:
			fprintf(fp,"NAME"); break;
		case SUM1:
			fprintf(fp,"+"); break;
		case SUM2:
			fprintf(fp,"SUM2"); break;
		case SUM3:
			fprintf(fp,"SUM3"); break;
		case SUB1:
			fprintf(fp,"-"); break;
		case SUB2:
			fprintf(fp,"SUB2"); break;
		case SUB3:
			fprintf(fp,"SUB3"); break;
		case DOT2:
			fprintf(fp,"DOT2"); break;
		case DOT3:
			fprintf(fp,"DOT3"); break;
		case CROSS3:
			fprintf(fp,"CROSS3"); break;
		case SCALE2:
			fprintf(fp,"SCALE2"); break;
		case SCALE3:
			fprintf(fp,"SCALE3"); break;
		case INT_POW:
			fprintf(fp,"INT_POW"); break;
		case MULT1:
			fprintf(fp,"*"); break;
		case DIV1:
			fprintf(fp,"/"); break;
		case DIV2:
			fprintf(fp,"DIV2"); break;
		case DIV3:
			fprintf(fp,"DIV3"); break;
		case EQUALS1:
			fprintf(fp,"="); break;
		case POW1:
			fprintf(fp,"^"); break;
		default:
			fprintf(fp,"unknown %d",op); break;
	}
}

void sprint_op(string,op)
char *string;
int op;
{
	switch(op)
	{
		case '+': case '-': case '*': case '/': case '=': 
		case ',': case '^': case '.':
		csprintf(string,"'%c'",op);
		case NUMBER:
			csprintf(string,"NUMBER"); break;
		case INTERVAL:
			csprintf(string,"INTERVAL"); break;
		case FUNCTION:
			csprintf(string,"FUNCTION"); break;
		case NAME:
			csprintf(string,"NAME"); break;
		case SUM1:
			csprintf(string,"+"); break;
		case SUM2:
			csprintf(string,"SUM2"); break;
		case SUM3:
			csprintf(string,"SUM3"); break;
		case SUB1:
			csprintf(string,"-"); break;
		case SUB2:
			csprintf(string,"SUB2"); break;
		case SUB3:
			csprintf(string,"SUB3"); break;
		case DOT2:
			csprintf(string,"DOT2"); break;
		case DOT3:
			csprintf(string,"DOT3"); break;
		case CROSS3:
			csprintf(string,"CROSS3"); break;
		case SCALE2:
			csprintf(string,"SCALE2"); break;
		case SCALE3:
			csprintf(string,"SCALE3"); break;
		case INT_POW:
			csprintf(string,"INT_POW"); break;
		case MULT1:
			csprintf(string,"*"); break;
		case DIV1:
			csprintf(string,"/"); break;
		case DIV2:
			csprintf(string,"DIV2"); break;
		case DIV3:
			csprintf(string,"DIV3"); break;
		case EQUALS1:
			csprintf(string,"="); break;
		case POW1:
			csprintf(string,"^"); break;
		default:
			csprintf(string,"unknown %d",op); break;
	}
}

void eprint_op(op)
int op;
{
	if( eqn_errs_to_stderr )
	{
		fprint_op(stderr,op);
		return;
        }
	switch(op)
	{
		case '+': case '-': case '*': case '/': case '=': 
		case ',': case '^': case '.':
		eprintf("'%c'",op);
		case NUMBER:
			eprintf("NUMBER"); break;
		case INTERVAL:
			eprintf("INTERVAL"); break;
		case FUNCTION:
			eprintf("FUNCTION"); break;
		case NAME:
			eprintf("NAME"); break;
		case SUM1:
			eprintf("+"); break;
		case SUM2:
			eprintf("SUM2"); break;
		case SUM3:
			eprintf("SUM3"); break;
		case SUB1:
			eprintf("-"); break;
		case SUB2:
			eprintf("SUB2"); break;
		case SUB3:
			eprintf("SUB3"); break;
		case DOT2:
			eprintf("DOT2"); break;
		case DOT3:
			eprintf("DOT3"); break;
		case CROSS3:
			eprintf("CROSS3"); break;
		case SCALE2:
			eprintf("SCALE2"); break;
		case SCALE3:
			eprintf("SCALE3"); break;
		case INT_POW:
			eprintf("INT_POW"); break;
		case MULT1:
			eprintf("*"); break;
		case DIV1:
			eprintf("/"); break;
		case DIV2:
			eprintf("DIV2"); break;
		case DIV3:
			eprintf("DIV3"); break;
		case EQUALS1:
			eprintf("="); break;
		case POW1:
			eprintf("^"); break;
		default:
			eprintf("unknown %d",op); break;
	}
}

void print_op(op)
int op;
{
	fprint_op(stdout,op);
}

/**** prints the equation with lots of brackets! ****/

void fprint_eqn(file, base )
FILE *file;
eqnode *base;
{
  if( base == NULL )
  {
	fprintf(file,"NULL eqn");
	return;
  }

  switch( base->op )
  {
  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN)
    {
	fprintf(file,"%s",base->u.f.f->name);
	break;
    }
    fprintf(file,"%s(",base->u.f.f->name);
    fprint_eqn(file,base->u.f.a);
    fprintf(file,")");
    break;
  case NAME:
    fprintf(file,"%s",base->u.str);
    break;
  case NUMBER:
    fprint_num(file,eqn_val(base));
    break;
  case BRACKET:
    fprintf(file,"(");
    fprint_eqn(file,base->u.n.r);
    fprintf(file,")");
    break;

  case INTERVAL:
    fprintf(file,"[");
    fprint_eqn(file,base->u.n.l);
    fprintf(file,",");
    fprint_eqn(file,base->u.n.r);
    fprintf(file,"]");
    break;
  case '+':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      fprint_eqn(file,base->u.n.l); break;
    default:	/* ',' and '-' and '=' */
      fprintf(file,"("); fprint_eqn(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"+");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      fprint_eqn(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '-':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      fprint_eqn(file,base->u.n.l); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"-");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case NAME: case NUMBER:
    case FUNCTION:
      fprint_eqn(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '*': case '.':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      fprint_eqn(file,base->u.n.l); break;
    default: /* '+' '-' '/' ',' '=' */
      fprintf(file,"("); fprint_eqn(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"%c",base->op);

    switch( base->u.n.r->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      fprint_eqn(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '/':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      fprint_eqn(file,base->u.n.l); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"/");

    switch( base->u.n.r->op )
    {
    case '^': case NAME: case NUMBER: case FUNCTION:
      fprint_eqn(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '^':
    switch( base->u.n.l->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      fprint_eqn(file,base->u.n.l); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"^");

    switch( base->u.n.r->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      fprint_eqn(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fprint_eqn(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '=': case ',':
    fprint_eqn(file,base->u.n.l);
    fprintf(file,"%c",base->op);
    fprint_eqn(file,base->u.n.r);
    break;
  default:
    fprintf(file,"bad op: ");
    fprint_op(file,base->op);
    fprintf(file," ");
  }
}


void sprint_eqn(string, base )
char *string;
eqnode *base;
{
  if( base == NULL )
  {
	sprintf(string,"NULL equation");
	return;
  }

  switch( base->op )
  {
  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN)
    {
	csprintf(string,"%s",base->u.f.f->name);
	break;
    }
    csprintf(string,"%s(",base->u.f.f->name);
    sprint_eqn(string,base->u.f.a);
    csprintf(string,")");
    break;
  case NAME:
    csprintf(string,"%s",base->u.str);
    break;
  case NUMBER:
    csprint_num(string,eqn_val(base));
/*
#ifdef __BORLANDC__
#define rint(a) floor(a)
#endif
    if( eqnval(base) == rint(eqn_val(base)) ) 
	csprintf(string,"%.0lf",eqn_val(base));
    else if( eqnval(base)*10.0 == rint(eqn_val(base)*10.0) ) 
	csprintf(string,"%.1lf",eqn_val(base));
    else if( eqnval(base)*100.0 == rint(eqn_val(base)*100.0) ) 
	csprintf(string,"%.2lf",eqn_val(base));
    else if( eqnval(base)*1000.0 == rint(eqn_val(base)*1000.0) ) 
	csprintf(string,"%.3lf",eqn_val(base));
    else if( eqnval(base)*10000.0 == rint(eqn_val(base)*10000.0) ) 
	csprintf(string,"%.4lf",eqn_val(base));
    else
	csprintf(string,"%lf",eqn_val(base));
*/
    break;
  case BRACKET:
    csprintf(string,"(");
    sprint_eqn(string,base->u.n.r);
    csprintf(string,")");
    break;

  case INTERVAL:
    csprintf(string,"[");
    sprint_eqn(string,base->u.n.l);
    csprintf(string,",");
    sprint_eqn(string,base->u.n.r);
    csprintf(string,"]");
    break;
  case '+':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      sprint_eqn(string,base->u.n.l); break;
    default:	/* ',' and '-' and '=' */
      csprintf(string,"("); sprint_eqn(string,base->u.n.l); csprintf(string,")"); break;
    }

    csprintf(string,"+");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      sprint_eqn(string,base->u.n.r); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.r); csprintf(string,")"); break;
    }
    break;
  case '-':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      sprint_eqn(string,base->u.n.l); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.l); csprintf(string,")"); break;
    }

    csprintf(string,"-");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case NAME: case NUMBER:
    case FUNCTION:
      sprint_eqn(string,base->u.n.r); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.r); csprintf(string,")"); break;
    }
    break;
  case '*': case '.':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      sprint_eqn(string,base->u.n.l); break;
    default: /* '+' '-' '/' ',' '=' */
      csprintf(string,"("); sprint_eqn(string,base->u.n.l); csprintf(string,")"); break;
    }

    csprintf(string,"%c",base->op);

    switch( base->u.n.r->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      sprint_eqn(string,base->u.n.r); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.r); csprintf(string,")"); break;
    }
    break;
  case '/':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      sprint_eqn(string,base->u.n.l); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.l); csprintf(string,")"); break;
    }

    csprintf(string,"/");

    switch( base->u.n.r->op )
    {
    case '^': case NAME: case NUMBER: case FUNCTION:
      sprint_eqn(string,base->u.n.r); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.r); csprintf(string,")"); break;
    }
    break;
  case '^':
    switch( base->u.n.l->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      sprint_eqn(string,base->u.n.l); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.l); csprintf(string,")"); break;
    }

    csprintf(string,"^");

    switch( base->u.n.r->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      sprint_eqn(string,base->u.n.r); break;
    default:
      csprintf(string,"("); sprint_eqn(string,base->u.n.r); csprintf(string,")"); break;
    }
    break;
  case '=': case ',':
    sprint_eqn(string,base->u.n.l);
    csprintf(string,"%c",base->op);
    sprint_eqn(string,base->u.n.r);
    break;
  default:
    csprintf(string,"bad op: ");
    sprint_op(string,base->op);
    csprintf(string," ");
  }
}

void eprint_eqn( base )
eqnode *base;
{
  if( eqn_errs_to_stderr )
  {
	fprint_eqn(stderr,base);
	return;
  }

  if( base == NULL )
  {
	eprintf("NULL\007");
	return;
  }

  switch( base->op )
  {
  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN)
    {
	eprintf("%s",base->u.f.f->name);
	break;
    }
    eprintf("%s(",base->u.f.f->name);
    eprint_eqn(base->u.f.a);
    eprintf(")");
    break;
  case NAME:
    eprintf("%s",base->u.str);
    break;
  case NUMBER:
    eprint_num(eqn_val(base));
/*
#ifdef __BORLANDC__
#define rint(a) floor(a)
#endif
    if( eqnval(base) == rint(eqn_val(base)) ) 
	eprintf("%.0lf",eqn_val(base));
    else if( eqnval(base)*10.0 == rint(eqn_val(base)*10.0) ) 
	eprintf("%.1lf",eqn_val(base));
    else if( eqnval(base)*100.0 == rint(eqn_val(base)*100.0) ) 
	eprintf("%.2lf",eqn_val(base));
    else if( eqnval(base)*1000.0 == rint(eqn_val(base)*1000.0) ) 
	eprintf("%.3lf",eqn_val(base));
    else if( eqnval(base)*10000.0 == rint(eqn_val(base)*10000.0) ) 
	eprintf("%.4lf",eqn_val(base));
    else
	eprintf("%lf",eqn_val(base));
*/
    break;
  case BRACKET:
    eprintf("(");
    eprint_eqn(base->u.n.r);
    eprintf(")");
    break;

  case INTERVAL:
    eprintf("[");
    eprint_eqn(base->u.n.l);
    eprintf(",");
    eprint_eqn(base->u.n.r);
    eprintf("]");
    break;
  case '+':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      eprint_eqn(base->u.n.l); break;
    default:	/* ',' and '-' and '=' */
      eprintf("("); eprint_eqn(base->u.n.l); eprintf(")"); break;
    }

    eprintf("+");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      eprint_eqn(base->u.n.r); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.r); eprintf(")"); break;
    }
    break;
  case '-':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      eprint_eqn(base->u.n.l); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.l); eprintf(")"); break;
    }

    eprintf("-");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case NAME: case NUMBER:
    case FUNCTION:
      eprint_eqn(base->u.n.r); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.r); eprintf(")"); break;
    }
    break;
  case '*': case '.':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      eprint_eqn(base->u.n.l); break;
    default: /* '+' '-' '/' ',' '=' */
      eprintf("("); eprint_eqn(base->u.n.l); eprintf(")"); break;
    }

    eprintf("%c",base->op);

    switch( base->u.n.r->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      eprint_eqn(base->u.n.r); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.r); eprintf(")"); break;
    }
    break;
  case '/':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      eprint_eqn(base->u.n.l); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.l); eprintf(")"); break;
    }

    eprintf("/");

    switch( base->u.n.r->op )
    {
    case '^': case NAME: case NUMBER: case FUNCTION:
      eprint_eqn(base->u.n.r); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.r); eprintf(")"); break;
    }
    break;
  case '^':
    switch( base->u.n.l->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      eprint_eqn(base->u.n.l); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.l); eprintf(")"); break;
    }

    eprintf("^");

    switch( base->u.n.r->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      eprint_eqn(base->u.n.r); break;
    default:
      eprintf("("); eprint_eqn(base->u.n.r); eprintf(")"); break;
    }
    break;
  case '=': case ',':
    eprint_eqn(base->u.n.l);
    eprintf("%c",base->op);
    eprint_eqn(base->u.n.r);
    break;
  default:
    eprintf("bad op: ");
    eprint_op(base->op);
    eprintf(" ");
  }
}

void print_eqn(eqn)
eqn_node *eqn;
{
	fprint_eqn(stdout,eqn);
}


/**** displays the equation in a tree fashion ****/

void display_eqn( base, depth )
eqnode *base; int depth;
{
  int i;
 
  if( base == NULL )
  {
	eprintf("display_eqn: base == NULL\n");
	return;
  }

  switch( base->op )
  {
  case NAME:
    for(i=1;i<=depth;++i) printf("    ");
    printf("%s\n",base->u.str);
    break;

  case NUMBER:
    for(i=1;i<=depth;++i) printf("    ");
    printf("%lf\n",base->u.num);
    break;

  case FUNCTION:
    for(i=1;i<=depth;++i) printf("    ");
    printf("%s\n",base->u.f.f->name);
    if(base->u.f.f->type != CONSTANT_FUN)
    	display_eqn(base->u.f.a,depth+1);
    break;
 
  case '+': case '-': case '*': case '/': case '^': case '=': case ',':
  case '.':
    display_eqn(base->u.n.l,depth+1);
    for(i=1;i<=depth;++i) printf("    ");
    printf("%c\n",base->op);
    display_eqn(base->u.n.r,depth+1);
    break;

  default:
    eprintf("display_eqn: bad op ");
        eprint_op(base->op);
        eprintf("\n");
  }
}
/**** returns a pointer to a copy of the equation given by base ****/

eqnode *duplicate( base )
eqnode *base;
{
  eqnode *temp;

  if( base == NULL )
  {
	eprintf("Tried to duplicate a NULL equation\n");
	return(NULL);
  }
  temp = grballoc( eqnode );
  temp->op = base->op;
  switch( base->op )
  {
  case BRACKET:
    temp->u.n.r=duplicate(base->u.n.r);
    break;

  case FUNCTION:
    temp->u.f.f = base->u.f.f;
    if(base->u.f.f->type == CONSTANT_FUN)
	temp->u.f.a = NULL;
    else
    	temp->u.f.a = duplicate(base->u.f.a);
    break;
  case NAME:
    temp->u.str = (char *) calloc( strlen(base->u.str)+1 , sizeof( char ) );
    strcpy(temp->u.str,base->u.str);
    break;
  case NUMBER:
    temp->u.num = base->u.num;
    break;
  case '+': case '-': case '*': case '/': case '^': case '=': case ',': case INTERVAL: case '.':
    temp->u.n.l=duplicate(base->u.n.l);
    temp->u.n.r=duplicate(base->u.n.r);
    break;
  default:
	eprintf("duplicate: bad op ");
	eprint_op(base->op);
	eprintf("\n");
	break;
  }
  return( temp );
}

/*
 * Function:	copy node
 * Action:	copies the information from right to base
 */

void copy_node(base,right)
eqnode *base,*right;
{
	if(right == NULL)
	{
		eprintf("Tried to copy a Null equation\n");
		return;
	}
	if(base == NULL)
	{
		eprintf("Tried to copy onto a Null node\n");
		return;
	}

	base->op = right->op;
	switch( right->op )
	{
	case NUMBER:
		base->u.num = right->u.num;
		break;
	case NAME:
		base->u.str = right->u.str;
		break;
	case '+': case '-': case '*': case '/': case '^': case '=': case ',':
	case '.':
		base->u.n.l = right->u.n.l;
		base->u.n.r = right->u.n.r;
		break;
	case FUNCTION:
		base->u.f.f = right->u.f.f;
		base->u.f.a = right->u.f.a;
		break;
	case BRACKET:
		base->u.n.r = right->u.n.r;
		break;

	default:
		eprintf("copy_node: bad op ");
        	eprint_op(base->op);
	        eprintf("\n");
		break;
	}
}

/*
* Function:	count_eqn_tree
* Action:	counts the number of nodes in a tree 
*		note a,b only counts as two nodes
*/

int count_eqn_tree( base )
eqnode *base;
{
  if( base == NULL )
  {
	eprintf("tried to count the nodes in a null equation\n");
	return(0);
  }
  switch( base->op )
  {
  case NAME:
  case NUMBER:
    return( 1 );
  case '+': case '-': case '*': case '/': case '^': case '=': case '.':
    return( 1 + count_eqn_tree(base->u.n.l) + count_eqn_tree(base->u.n.r) );
  case ',':
    return( count_eqn_tree(base->u.n.l) + count_eqn_tree(base->u.n.r) );
  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN ) return( 1 );
    if(base->u.f.f->type == SUM_FUN )
   	return(1+count_eqn_tree(base->u.f.a->u.n.r->u.n.r));
    return( 1 + count_eqn_tree(base->u.f.a) );
  case BRACKET:
    return(count_eqn_tree(base->u.n.r));
  default:
	eprintf("count_eqn_tree: bad op ");
        eprint_op(base->op);
        eprintf("\n");
	break;
  }
  return( 0 );
}

int count_eqn_args(base)
eqnode *base;
{
  int leftcount,rightcount;

  if(base == NULL)
  {
	eprintf("Null equation while trying to count number of arguments\n");
	return(0);
  }

  switch( base->op )
  {
  case NAME:
  case NUMBER:
    return( 1 );

  case '+': case '-': case '=':
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != rightcount )
    {
	eprintf("Different counts for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(leftcount);

  case '*':
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount == 2 && rightcount == 2 ) return(2);
    if(leftcount != 1  && rightcount != 1)
    {
	eprintf("Bad counts for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    if(leftcount == 1) return(rightcount);
    return(leftcount);

  case '.':
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != rightcount )
    {
	eprintf("Bad counts for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(1);

  case '/':
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount == 2 && rightcount == 2 ) return(2);
    if( rightcount != 1)
    {
	eprintf("Bad counts for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(leftcount);

  case '^':
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount == 2 && rightcount == 2 ) return(1);
    if(leftcount != rightcount || ( leftcount != 1 && leftcount != 3) )
    {
	eprintf("Bad counts for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(rightcount);

  case FUNCTION:
    if( base->u.f.f->type == EXTERN_MAP || base->u.f.f->type == INTERN_MAP )
	return(base->u.f.f->dim);
    else
	return(1);

  case ',':
    return( count_eqn_args(base->u.n.l) + count_eqn_args(base->u.n.r) );

  default:
	eprintf("count_eqn_args: bad op ");
        eprint_op(base->op);
        eprintf("\n");
	break;
  }
  return( 0 );
}

/*
 * Function:	get_eqn_arg
 * Action:	gets the i th argument from a comma seperated list
 *		can cope with (a,b,c),(d,e,f)
 */

eqnode *get_eqn_arg(base,i)
eqnode *base;
int i;
{
	int leftcount;

	if(base == NULL)
	{
		eprintf("get_eqn_arg: base = NULL\n");
		return(NULL);
	}

	if( base->op != ',' )
	{
		if(i == 1) return(base);
		else	   return(NULL);
	}

	leftcount = count_eqn_args(base->u.n.l);
	if(i <= leftcount ) return(get_eqn_arg(base->u.n.l,i));
	else return(get_eqn_arg(base->u.n.r,i-leftcount));
}

/**** frees the space used by the equation base ****/

void free_eqn_tree( base )
eqnode *base;
{
  if( base == NULL )
  {
	eprintf("Tried to free a null equation\n");
	return;
  }
  switch( base->op )
  {
  case NAME:
	if( base->u.str == NULL )
	{
		fprintf(stderr,"NULL string in free_eqn_tree base %p\n",base);
	}
	else
    free( base->u.str );
    break;
  case NUMBER:
    break;
  case '+': case '-': case '*': case '/': case '^': case '=': case ',': case INTERVAL: case '.':
    free_eqn_tree(base->u.n.l);
    free_eqn_tree(base->u.n.r);
    break;

  case FUNCTION:
    if(base->u.f.f->type != CONSTANT_FUN)
    	free_eqn_tree(base->u.f.a);
    break;

  case BRACKET:
    free_eqn_tree(base->u.n.r);
    break;

  }
  free( base );
}

void free_eqn_node( node )
eqnode *node;
{
	if( node->op == NAME) free( node->u.str );
	free( node );
}

/*
 * Function:	eqn_list_product
 * Action:	multiply together two lists of same structure
 * Returns:	pointer to product, NULL on error
 */

eqnode *eqn_list_product(left,right)
eqnode *left,*right;
{
	eqnode *node;

	if(left == NULL )
	{
		eprintf("eqn_list_product: NULL left\n");
		return(NULL);
	}
	if(right == NULL )
	{
		eprintf("eqn_list_product: NULL right\n");
		return(NULL);
	}

	if( left->op == ',' && right->op == ',' )
	{
		node = grballoc(eqnode);
		node->op = '+';
		node->u.n.l = eqn_list_product(left->u.n.l,right->u.n.l);
		node->u.n.r = eqn_list_product(left->u.n.r,right->u.n.r);
		if(node->u.n.l == NULL || node->u.n.r == NULL)
		{
			free(node);
			return(NULL);
		}
	}
	else if( left->op != ',' && right->op != ',' )
	{
		node = grballoc(eqnode);
		node->op = '*';
		node->u.n.l = left;
		node->u.n.r = right;
	}
	else
	{
		eprintf("eqn_list_product: left and right trees didn't match\n");
		eprint_eqn(left); eprintf("\n");
		eprint_eqn(right); eprintf("\n");
		return(NULL);
	}
	return(node);
}

/*
 * Functions:	join_eqn,join_dup_eqn
 * Action:	Joins two equations together linked by op
 *		for join_dup_eqns copys of the equations are used.
 */

eqnode *join_eqns(op,left,right)
int op;
eqnode *left, *right;
{
	eqnode *temp;

	if(left == NULL )
	{
		eprintf("Null left equation while joining equations\n");
		return(NULL);
	}
	if(right == NULL )
	{
		eprintf("Null right equation while joining equations\n");
		return(NULL);
	}

	switch(op)
	{
  		case '+': case '-': case '*': case '/': case '^': case '=':
		case ',': case '.':
			break;
		default:
			eprintf("Bad op ");
			eprint_op(op);
			eprintf(" while joining equations\n");
			return(NULL);
	}
	temp = grballoc(eqnode);
	temp->op = op;
	temp->u.n.l = left;
	temp->u.n.r = right;
	return(temp);
}

eqnode *join_dup_eqns(op,left,right)
int op;
eqnode *left, *right;
{
	eqnode *temp;

	if(left == NULL )
	{
		eprintf("Null left equation while joining equations\n");
		return(NULL);
	}
	if(right == NULL )
	{
		eprintf("Null right equation while joining equations\n");
		return(NULL);
	}

	switch(op)
	{
  		case '+': case '-': case '*': case '/': case '^': case '=':
		case ',': case '.':
			break;
		default:
			eprintf("Bad op ");
			eprint_op(op);
			eprintf(" while joining equations\n");
			return(NULL);
	}
	temp = grballoc(eqnode);
	temp->op = op;
	temp->u.n.l = duplicate(left);
	temp->u.n.r = duplicate(right);
	return(temp);
}

int name_in_eqn(eqnode *eqn,char *name)
{
	switch(eqn->op)
	{
	case NAME:
		return( !strcmp(eqn->u.str,name));
	case NUMBER:
		return(FALSE);
        case '+': case '-': case '*': case '/': case '^': case '=':
        case ',': case '.': case INTERVAL:
		return( name_in_eqn(eqn->u.n.l,name)
		     || name_in_eqn(eqn->u.n.r,name) );
	case FUNCTION:
		return( name_in_eqn(eqn->u.f.a,name) );
	default:
		eprintf("name_in_eqn: Bad Op ");
		eprint_op(eqn->op);
		eprintf("\n");
		return(FALSE);
	}
}

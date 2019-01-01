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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#define I_AM_EQNFUNCTIONS
#include "eqn.h"
#include <varargs.h>
/*
#define SORT_ADD
#define PRINT_EXPANSION
#define PRINT_DIFF_FUN
*/
#define SILLYFUNS

#define grballoc(node) (node *) malloc(sizeof(node))
#define MAX(a,b)       a > b ? a : b ;
#define TRUE 1
#define FALSE 0

#define MAX_FUN_ARGS 10

typedef rpeint vrpeint;

/**************************************************************************/
/*                                                                        */
/* Now we hav a reverse polish calculator                                 */
/*   make_vrpe( eqn , n , names[n] ) creates a reverse polish string       */
/*   eval_vrpe( vrpe, vars[] )        evaluates it using a stack            */
/*   clear_vrpe_const()              clears the constant array             */
/*   check_vrpe( vrpe )               checks that vrpe will be sucessfully   */
/*                                    evaluated.                          */
/*   print_vrpe( vrpe, n ,names[n] )  prints out the rp-string              */
/*                                                                        */
/*                                                                        */
/*   A stack is used for the evaluation of an rp-string and a global      */
/*   array of constants is used.                                          */
/*                                                                        */
/*   For example if eqn is                                                */
/*   (x+y)/(w-z)=0.7, n = 4 names[] = "x","y","z","w" the string produced */
/*   is " '`1 `2 + `4 `3 - / ``0 = "   where `1, `2, `3, `4 refere to     */
/*   the 1st, 2nd, 3rd and 4th names ``0 referes to constant[0]           */
/*   which is in this case 0.7.                                           */ 
/*   When evaluating we have the following sequence of ops                */
/*                                                                        */
/*   token    action              state of stack                          */
/*                                                                        */
/*     `1     push(vars[0])         x                                     */
/*     `2     push(vars[1])         y , x                                 */
/*      +     r = pop()             x                                     */
/*            l = pop()                                                   */
/*            push(l+r)             x+y                                   */
/*     `4     push(vars[3])         w, x+y                                */
/*     `3     push(vars[2])         z, w, x+y                             */
/*      -     r = pop()             w, x+y                                */
/*            l = pop()             x+y                                   */
/*            push(l-r)             w-z, x+y                              */
/*      /     r = pop()             x+y                                   */
/*            l = pop()                                                   */
/*            push(l/r)             (x+y)/(w-z)                           */
/*      ``0   push(const[0])        0.7, (x+y)/(w-z)                      */
/*      =     r = pop()             (x+y)/(w-z)                           */
/*            l = pop()                                                   */
/*            return(l-r)                                                 */
/*                                                                        */
/*   so we get a return value of (x+y)/(w-z) - 0.7                        */
/*                                                                        */
/*   the vrpe string consists of integers coded as follows                 */
/*                                                                        */
/*  0 = vrpe_name_base..vrpe_op_base-1     variable( i )                     */
/*      vrpe_op_base..vrpe_const_base-1    op( i - vrpe_op_base )            */
/*      vrpe_const_base..                 constant( i - vrpe_const_base )   */
/*                                                                        */
/*   the constants are stored in a global array 'vrpe_const' and the       */
/*   global pointer 'vrpe_const_ptr' contains the current top of list      */
/*                                                                        */
/*   DIAGNOSTIC                                                           */
/*      make_vrpe() returns NULL if a valid rp-string can not be found     */
/*   in particular this will happen if eqn contains an unknown name.      */
/*                                                                        */
/**************************************************************************/

/**** Definitions used for the reverse polish calculator ****/

#define vrpe_name_base 0
#define vrpe_op_base 256
#define END_RPE vrpe_op_base 
#define vrpe_fun_base 512
#define vrpe_fun_max  256
#define vrpe_sum_max 32
#define vrpe_const_base (vrpe_fun_base + vrpe_fun_max + vrpe_sum_max)
#ifdef BOR_THINK
#define vrpe_const_max  1000
#else
#define vrpe_const_max  10000
#endif
eqn_funs *vrpe_funs[vrpe_fun_max];
int vrpe_fun_ptr = 0;
double vrpe_const[vrpe_const_max];
int vrpe_const_ptr = 0 ;		/* the current position in array */
int vrpe_ptr;

typedef struct {
	rpeint *vrpe;
	eqn_funs *fun;
	int	var_ref;
} vrpe_sum_type;
vrpe_sum_type vrpe_sums[vrpe_sum_max];

#define vrpe_sum_base (vrpe_fun_base+vrpe_fun_max)
int vrpe_sum_ptr = 0;



/**** A stack 'vdbl_stack' is used for evaluating an rp-string *****/


#ifndef stack_size 
#  define stack_size 500
#endif
double vdbl_stack[stack_size];

/*
 * Function:	make_vrpe
 * Action:	constructs an vrpe string,
 *		returns NULL if failed
*/

vrpeint *make_vrpe( eqn , n , names )
eqnode *eqn; int n; char **names;
{
  vrpeint *temp;
  int vrpe_size,i;

  if(eqn == NULL )
  {
	eprintf("make_vrpe: NULL eqnation\n");
	return(NULL);
  }

  if(names == NULL)
  {
	eprintf("make_vrpe: NULL names list\n");
	return(NULL);
  }
  for(i=0;i<n;++i)
  {
	if(names[i] == NULL)
	{
		eprintf("make_vrpe: NULL name number %d\n",i);
		return(NULL);
	}
  }

  vrpe_size = count_eqn_tree( eqn ); /* the length of string is number of
                                         nodes in the equation */
  vrpe_ptr = 0;
  temp = (vrpeint *) calloc( vrpe_size + 1, sizeof(vrpeint) );
  temp[vrpe_size] = END_RPE; /* a flag to signal end of string */
  if( !make_vrpe2( temp, eqn, n, names )
   || !check_vrpe( temp, n, names ,count_eqn_args(eqn)) )
  {
    free(temp);
    return(NULL);
  }
/*
printf("vrpe for ");
print_eqn(eqn);
printf(" is\n");
print_vrpe(temp,names);
*/
  return( temp );
}

/**** a recursive sub-routine called by make_vrpe  ****/

int make_vrpe2( vrpe, eqn, n, names )
vrpeint vrpe[]; eqnode *eqn; int n; char **names;
{
  int leftres,rightres,leftcount,rightcount,i;
  double num;
  int old_vrpe_ptr;

  switch( eqn->op )
  {
  case NAME:
    for(i=0;i<n;++i)
      if( !strcmp( eqn->u.str, names[i] ) )
      {
        vrpe[vrpe_ptr++] = (vrpeint) i + vrpe_name_base;
        return( TRUE );
      }
    eprintf("make_vrpe2: name not found %s\n",eqn->u.str);
    return( FALSE );

  case NUMBER:
    for(i=0;i<vrpe_const_ptr;++i)
    {
	if( vrpe_const[i] == eqn->u.num )
	{
		vrpe[vrpe_ptr++] = (vrpeint) vrpe_const_base + i;
		return(TRUE);
	}
    }
    vrpe[vrpe_ptr++] = (vrpeint) vrpe_const_ptr + vrpe_const_base ;
    vrpe_const[vrpe_const_ptr] = eqn->u.num;
    if ( ++vrpe_const_ptr >= vrpe_const_max )
    {
      eprintf("make_vrpe2: too many constants\n");
      return(FALSE);
    }
    return( TRUE );

  case '+':
    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
    leftcount = count_eqn_args(eqn->u.n.l);
    rightcount = count_eqn_args(eqn->u.n.r);
    if( leftcount != rightcount )
    {
	eprintf("make_vrpe: different counts for + %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    switch(leftcount)
    {
	case 1: vrpe[vrpe_ptr++] = SUM1 + vrpe_op_base; break;
	case 2: vrpe[vrpe_ptr++] = SUM2 + vrpe_op_base; break;
	case 3: vrpe[vrpe_ptr++] = SUM3 + vrpe_op_base; break;
	case 4: vrpe[vrpe_ptr++] = SUM4 + vrpe_op_base; break;
	default:
		eprintf("make_vrpe: bad count %d\n",leftcount);
		return(FALSE);
    }
    return( leftres && rightres );

  case '-': case '=':
    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
    leftcount = count_eqn_args(eqn->u.n.l);
    rightcount = count_eqn_args(eqn->u.n.r);
    if( leftcount != rightcount )
    {
	eprintf("make_vrpe: different counts for + %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    switch(leftcount)
    {
	case 1: vrpe[vrpe_ptr++] = SUB1 + vrpe_op_base; break;
	case 2: vrpe[vrpe_ptr++] = SUB2 + vrpe_op_base; break;
	case 3: vrpe[vrpe_ptr++] = SUB3 + vrpe_op_base; break;
	case 4: vrpe[vrpe_ptr++] = SUB4 + vrpe_op_base; break;
	default:
		eprintf("make_vrpe: bad count %d\n",leftcount);
		return(FALSE);
    }
    return( leftres && rightres );

  case '*':
    leftcount = count_eqn_args(eqn->u.n.l);
    rightcount = count_eqn_args(eqn->u.n.r);
    if( leftcount == 2 && rightcount == 2 )
    {
	    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
	    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
	    vrpe[vrpe_ptr++] = CPLX_MUL + vrpe_op_base; 
	    return( leftres && rightres );
    }
		
    if( leftcount != 1 && rightcount != 1)
    {
	eprintf("make_vrpe: bad counts for '*' %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    if(leftcount == 1)
    {
	    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
	    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
	    switch(rightcount)
	    {
		case 1: vrpe[vrpe_ptr++] = MULT1 + vrpe_op_base; break;
		case 2: vrpe[vrpe_ptr++] = SCALE2 + vrpe_op_base; break;
		case 3: vrpe[vrpe_ptr++] = SCALE3 + vrpe_op_base; break;
		case 4: vrpe[vrpe_ptr++] = SCALE4 + vrpe_op_base; break;
		default:
			eprintf("make_vrpe: bad count %d\n",rightcount);
			return(FALSE);
	    }
	    return( leftres && rightres );
    }
    else
    {
		/* reverse order */

	    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
	    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
	    switch(leftcount)
	    {
		case 1: vrpe[vrpe_ptr++] = MULT1 + vrpe_op_base; break;
		case 2: vrpe[vrpe_ptr++] = SCALE2 + vrpe_op_base; break;
		case 3: vrpe[vrpe_ptr++] = SCALE3 + vrpe_op_base; break;
		case 4: vrpe[vrpe_ptr++] = SCALE4 + vrpe_op_base; break;
		default:
			eprintf("make_vrpe: bad count %d\n",leftcount);
			return(FALSE);
	    }
	    return( leftres && rightres );
    }

  case '/':
#ifdef NOT_DEF
	/* right added before the left */
    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
    leftcount = count_eqn_args(eqn->u.n.l);
    rightcount = count_eqn_args(eqn->u.n.r);A
    if( leftcount != rightcount || leftcount != 1 ) 
    {
	eprintf("make_vrpe: bad counts for / %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    vrpe[vrpe_ptr++] = DIV1 + vrpe_op_base;
    return( leftres && rightres );
#endif
    leftcount = count_eqn_args(eqn->u.n.l);
    rightcount = count_eqn_args(eqn->u.n.r);
    if( leftcount == 2 && rightcount == 2 )
    {
	    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
	    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
	    vrpe[vrpe_ptr++] = CPLX_DIV + vrpe_op_base; 
	    return( leftres && rightres );
    }
		
    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
 
    if( rightcount != 1 )
    {
        eprintf("make_vrpe: bad counts for / %d %d\n",
                leftcount,rightcount);
        return(FALSE);
    }
    switch(leftcount)
    {
        case 1: vrpe[vrpe_ptr++] = DIV1 + vrpe_op_base; break;
        case 2: vrpe[vrpe_ptr++] = DIV2 + vrpe_op_base; break;
        case 3: vrpe[vrpe_ptr++] = DIV3 + vrpe_op_base; break;
        default:
                eprintf("make_vrpe: bad counts for / %d 1\n",leftcount);
                        return(FALSE);
    }
    return( leftres && rightres );


  case '^':
    rightcount = count_eqn_args(eqn->u.n.r);
    leftcount = count_eqn_args(eqn->u.n.l);
    if( leftcount == 3 && rightcount == 3)	/* Cross product */
    {
	rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
	leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
	vrpe[vrpe_ptr++] = CROSS3 + vrpe_op_base;
	return(leftres && rightres);
    }
    if( leftcount == 2 && rightcount == 2 )	/* 2D vec product returns scaler */
    {
	    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
	    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
	    vrpe[vrpe_ptr++] = CROSS2 + vrpe_op_base; 
	    return( leftres && rightres );
    }
    else if( leftcount == 1 && rightcount == 1 )/* Power */
    {
	if(eqn->u.n.r->op == NUMBER )
	{
		num = eqn->u.n.r->u.num;
		if( num - floor(num) < 1.0e-9 && num > 0.0 && num < 32768.0 )
		{
			leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
			vrpe[vrpe_ptr++] = (vrpeint) INT_POW + vrpe_op_base;
			vrpe[vrpe_ptr++] = (vrpeint) num;
			return(leftres);
		}
	}
	rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
	leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
	vrpe[vrpe_ptr++] = POW1 + vrpe_op_base;
	return( leftres && rightres );
    }
    else
    {
	eprintf("make_vrpe: bad counts for ^ %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }

  case '.':
    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
    leftcount = count_eqn_args(eqn->u.n.l);
    rightcount = count_eqn_args(eqn->u.n.r);
    if( leftcount != rightcount )
    {
	eprintf("make_vrpe: different counts for '.' %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    switch(leftcount)
    {
	case 1: vrpe[vrpe_ptr++] = MULT1 + vrpe_op_base; break;
	case 2: vrpe[vrpe_ptr++] = DOT2 + vrpe_op_base; break;
	case 3: vrpe[vrpe_ptr++] = DOT3 + vrpe_op_base; break;
	case 4: vrpe[vrpe_ptr++] = DOT4 + vrpe_op_base; break;
	default:
		eprintf("make_vrpe: bad count %d\n",leftcount);
		return(FALSE);
    }
    return( leftres && rightres );

  case ',':
    rightres = make_vrpe2( vrpe, eqn->u.n.r, n, names );
    leftres = make_vrpe2( vrpe, eqn->u.n.l, n, names );
    return( leftres && rightres );

  case FUNCTION:
	switch(eqn->u.f.f->type)
	{
	case CONSTANT_FUN:
	    for(i=0;i<vrpe_const_ptr;++i)
	    {
		if( vrpe_const[i] == eqn->u.f.f->val )
		{
			vrpe[vrpe_ptr++] = (vrpeint) vrpe_const_base + i;
			return(TRUE);
		}
	    }
	    vrpe[vrpe_ptr++] = (vrpeint) vrpe_const_ptr + vrpe_const_base ;
	    vrpe_const[vrpe_const_ptr] = eqn->u.f.f->val;
	    if ( ++vrpe_const_ptr >= vrpe_const_max )
	    {
	      eprintf("make_vrpe2: too many constants\n");
	      return(FALSE);
	    }
	    return( TRUE );

	case OPERATOR:
		eprintf("Can't convert equation involving operator %s into a reverse polish string\n",eqn->u.f.f->name);
		return(FALSE);

	case SUM_FUN:
		if( count_eqn_args(eqn->u.f.a) != eqn->u.f.f->nvars )
		{
			eprintf("Actual argument count different to profile for function %s\n",eqn->u.f.f->name);
			return(FALSE);
		}
		if( eqn->u.f.f->nvars > MAX_FUN_ARGS )
		{
			eprintf("Sorry function %s has too many arguments to convert to reverse polish string\n",eqn->u.f.f->name);
			return(FALSE);
		}
		if( eqn->u.f.a->u.n.r->u.n.l->op != NAME)
		{
			eprintf("second opperand of %s must be a name\n",
				eqn->u.f.f->name);
			return(FALSE);
		} 
		for(i=0;i<n;++i)
		{
			if(!strcmp( eqn->u.f.a->u.n.r->u.n.l->u.str,names[i]) )
			{
				vrpe_sums[vrpe_sum_ptr].var_ref = i;
				break;
			}
		}
		if(i==n)
		{
			eprintf("Second operator (%s) of %s not found in list of names\n", eqn->u.f.a->u.n.r->u.n.l->u.str,eqn->u.f.f->name);
			return(FALSE);
		}
/*
printf("Before vrpe is\n");
print_vrpe(vrpe,names);
*/
		vrpe_sums[vrpe_sum_ptr].fun = eqn->u.f.f;
		leftres = make_vrpe2(vrpe,eqn->u.f.a->u.n.r->u.n.r, n, names );
		vrpe[vrpe_ptr++] = (rpeint)
			 vrpe_sum_ptr + vrpe_sum_base;
/*
printf("make_vrpe2 vrpe is\n");
print_vrpe(vrpe,names);
*/
		old_vrpe_ptr = vrpe_ptr;
		vrpe_sums[vrpe_sum_ptr].vrpe =
				make_vrpe(eqn->u.f.a->u.n.l,n,names);
		vrpe_ptr = old_vrpe_ptr;
		if(  vrpe_sums[vrpe_sum_ptr].vrpe == NULL ) 
		{
			eprintf("Error making sum equation\n");
			eprint_eqn(eqn->u.f.a->u.n.l);
			eprintf("\n");
			return(FALSE);
		}
/*
printf("After\n");
print_vrpe(vrpe,names);
*/

		++vrpe_sum_ptr;
		if( vrpe_sum_ptr >= vrpe_sum_max )
		{
			eprintf("Too many sum-type expressions max %d\n",
				vrpe_sum_max-1);
			return(FALSE);
		}
		return(leftres);
		
	case INTERN_FUN: case EXTERN_FUN:
	case INTERN_MAP: case EXTERN_MAP:
		if( count_eqn_args(eqn->u.f.a) != eqn->u.f.f->nvars )
		{
			eprintf("Actual argument count different to profile for function %s\n",eqn->u.f.f->name);
			return(FALSE);
		}
	
		if( eqn->u.f.f->nvars > MAX_FUN_ARGS )
		{
			eprintf("Sorry function %s has too many arguments to convert to reverse polish string\n",eqn->u.f.f->name);
			return(FALSE);
		}
	
		leftres = make_vrpe2(vrpe,eqn->u.f.a, n, names );

		/* Check the list of vrpe_funs */

		for(i=0;i<vrpe_fun_ptr;++i)
		{
			if(vrpe_funs[i] == eqn->u.f.f)
			{
				vrpe[vrpe_ptr++] = (vrpeint) vrpe_fun_base + i;
				return(TRUE);
			}
		}
		vrpe[vrpe_ptr++] = (vrpeint) vrpe_fun_ptr + vrpe_fun_base;
		vrpe_funs[vrpe_fun_ptr] = eqn->u.f.f;
		if( ++vrpe_fun_ptr >= vrpe_fun_max )
		{
			eprintf("make_vrpe2: too many functions\n");
			return(FALSE);
		}
		return(leftres);
	default:
		eprintf("Bad function type %d in make_vrpe\n",eqn->u.f.f->type);
		return(FALSE);
	}

  default:
	eprintf("make_vrpe2: bad op ");
        eprint_op(eqn->op);
        eprintf("\n");
	break;
  }
  return(FALSE);
}

/*****
*     prints out the vrpe string 'vrpe'
*     'names' is an array of 'n' names.
*****/

void print_vrpe( vrpe, names )
vrpeint vrpe[]; char **names;
{
  int ptr = 0;
  vrpeint c;

  if( vrpe == NULL )
  {
    printf("NULL reverse polish string\n");
    return;
  }

  do
  {
    c = vrpe[ptr++];
    if( c < vrpe_op_base )          printf("\tvar\t%s\n",names[c]); 
    else if( c < vrpe_fun_base ) {  printf("\top\t");
				    print_op(c-vrpe_op_base); 
				    printf("\n"); }
    else if ( c < vrpe_sum_base ) printf("\tfun\t%s\n",
					vrpe_funs[c-vrpe_fun_base]->name);
    else if ( c < vrpe_const_base ) printf("\tsum\t%s\n",
					vrpe_sums[c-vrpe_sum_base].fun->name);
    else			   printf("\tconst\t%lf\n",
					vrpe_const[c-vrpe_const_base]);
  }
  while( c != END_RPE && c != vrpe_op_base + '=' );
}

void fprint_vrpe(fp, vrpe, names )
FILE *fp;
vrpeint vrpe[]; char **names;
{
  int ptr = 0;
  vrpeint c;

  if( vrpe == NULL )
  {
    fprintf(fp,"NULL reverse polish string\n");
    return;
  }

  do
  {
    c = vrpe[ptr++];
    if( c < vrpe_op_base )          fprintf(fp,"\tvar\t%s\n",names[c]);
    else if ( c < vrpe_fun_base ) { fprintf(fp,"\top\t");
				    fprint_op(fp,c-vrpe_op_base); 
				    fprintf(fp,"\n"); }
    else if ( c < vrpe_sum_base )   fprintf(fp,"\tfun\t%s\n",
					vrpe_funs[c-vrpe_fun_base]->name);
    else if ( c < vrpe_const_base ) fprintf(fp,"\tsum\t%s\n",
					vrpe_sums[c-vrpe_sum_base].fun->name);
    else			   fprintf(fp,"\tconst\t%lf\n",
					vrpe_const[c-vrpe_const_base]);
  }
  while( c != END_RPE && c != vrpe_op_base + '=' );
}

void eprint_vrpe(vrpe, names )
vrpeint vrpe[]; char **names;
{
  int ptr = 0;
  vrpeint c;

  if( vrpe == NULL )
  {
    eprintf("NULL reverse polish string\n");
    return;
  }

  do
  {
    c = vrpe[ptr++];
    if( c < vrpe_op_base )          eprintf("\tvar\t%s\n",names[c]);
    else if ( c < vrpe_fun_base ) { eprintf("\top\t");
				    eprint_op(c-vrpe_op_base);
				    eprintf("\n"); }
    else if ( c < vrpe_sum_base ) eprintf("\tfun\t%s\n",
					vrpe_funs[c-vrpe_fun_base]->name);
    else if ( c < vrpe_const_base ) eprintf("\tsum\t%s\n",
					vrpe_sums[c-vrpe_sum_base].fun->name);
    else			   eprintf("\tconst\t%lf\n",
					vrpe_const[c-vrpe_const_base]);
  }
  while( c != END_RPE && c != vrpe_op_base + '=' );
}

/*****
*     The no-holds bared vrpe-calculator.
*     This has been tweeked for super fast operation.
*     Note 
*       all variables are registers.
*       the state of the stack is one behind what it should be
*         the top of stack is contained in 'r' 
*         this prevents wastful operations such as push(r); .. ; pop(r); 
*       the stack is an array but we use pointers for speed.
*       push and pop are defined by macros.
*****/

#define push_dbl(x) *(--sp) = x 
#define pop_dbl()   *(sp++) 

double *dbl_vstack_base = vdbl_stack + stack_size; /* pointers quicker than arrays */
 
double *eval_vrpe( vrpe, vars )
register vrpeint *vrpe; 		/*  All variables are registers  */
register double *vars;
{
  register vrpeint c;
  register double l,r,t;
  register double *sp;
  register double low,high,inc;
  double   *old_stack_base;
  eqn_funs *fun;
  double   itt_vars[4];
  vrpe_sum_type sum;

  if( vrpe == NULL )
  {
    eprintf("eval_vrpe: NULL reverse polish string\n");
    *dbl_vstack_base = sqrt(-1.0);
    return(dbl_vstack_base);
  }
  sp = dbl_vstack_base;

  while( TRUE )
  {
    c = *(vrpe++);

    if( c < vrpe_op_base ) 
    {
        push_dbl(r);			/* one extra push at begining */
        r = *(vars+c); 			/* r holds the top of stack */
    }

    else if( c >= vrpe_const_base ) 
    {
        push_dbl(r);
        r = vrpe_const[ c - vrpe_const_base ];
    }

    else if( c >= vrpe_sum_base )
    {
	/* Stack holds low, high, inc */

	low = r; high = *sp; inc = *(sp+1);  
  	    old_stack_base = dbl_vstack_base;
  	    dbl_vstack_base = sp+1; /* set global stack ptr */

	sum= vrpe_sums[c-vrpe_sum_base];
	vars[sum.var_ref] = r; 
	itt_vars[1] = *(eval_vrpe(sum.vrpe,vars));
	itt_vars[2] = 1.0;
	itt_vars[3] = r;
	itt_vars[0] = eval_rpe(sum.fun->rpe2,itt_vars);
	itt_vars[2] = 2.0;
/*
printf("res %f\n",itt_vars[0]);
printf("Stack %f %f %f %f\n",*sp,*(sp+1),*(sp+2),*(sp+3));
*/
	for(r=low+inc;r<high+TAD;r=r+inc)
	{
		/* now evaluate the summand */

		vars[sum.var_ref] = r; 
		itt_vars[3] = r;
		itt_vars[1] = *(eval_vrpe(sum.vrpe,vars));
/*
printf("var %f res %f num %f\n",r,itt_vars[1],itt_vars[2]);
*/
		/* and eval itterative function */

		itt_vars[0] = eval_rpe(sum.fun->rpe,itt_vars);
		itt_vars[2] += 1.0;
/*
printf("res %f\n",itt_vars[0]);
printf("Stack %f %f %f %f\n",*sp,*(sp+1),*(sp+2),*(sp+3));
*/
	}
	r = itt_vars[0];
        dbl_vstack_base = old_stack_base;
	sp += 2;
    }	 
    else if( c >= vrpe_fun_base )
    {
	fun =  vrpe_funs[c - vrpe_fun_base];
	switch(fun->type)
	{
	case EXTERN_FUN:
	    switch( fun->nvars )
	    {
	    case 1: r = (*fun->fun)(r); break;
	    case 2: r = (*fun->fun)(r,*sp); sp += 1; break;
	    case 3: r = (*fun->fun)(r, *sp, *(sp+1)); sp += 2; break;

	    case 4: 
		r = (*fun->fun)(r, *(sp),
			 *(sp+1), *(sp+2));
		sp += 3;
		break;

	    case 5: 
		r = (*fun->fun)(r, *(sp),
			 *(sp+1), *(sp+2),
			 *(sp+3));
		sp += 4;
		break;

	    case 6: 
		r = (*fun->fun)(r, *(sp),
			 *(sp+1), *(sp+2),
			 *(sp+3), *(sp+4));
		sp += 5;
		break;

	    case 7: 
		r = (*fun->fun)(r, *(sp),
			 *(sp+1), *(sp+2),
			 *(sp+3), *(sp+4),
			 *(sp+5));
		sp += 6;
		break;

	    case 8: 
		r = (*fun->fun)(r, *(sp),
			 *(sp+1), *(sp+2),
			 *(sp+3), *(sp+4),
			 *(sp+5), *(sp+6));
		sp += 7;
		break;

	    case 9: 
		r = (*fun->fun)(r, *(sp),
			 *(sp+1), *(sp+2),
			 *(sp+3), *(sp+4),
			 *(sp+5), *(sp+6),
			 *(sp+7));
		sp += 8;
		break;

	    case 10: 
		r = (*fun->fun)(r, *(sp),
			 *(sp+1), *(sp+2),
			 *(sp+3), *(sp+4),
			 *(sp+5), *(sp+6),
			 *(sp+7), *(sp+8));
		sp += 9;
		break;

	    default:
		break;
	    }
	    break;

	case INTERN_FUN:
	    push_dbl(r);
  	    old_stack_base = dbl_vstack_base;
  	    dbl_vstack_base = sp;
	    r = eval_rpe(fun->rpe,sp);
  	    dbl_vstack_base = old_stack_base;
	    sp += fun->nvars;
	    break;

#ifdef NOT_DEF
	case EXTERN_MAP:
	    switch( fun->nvars )
	    {
	    case 1: res = (*fun->vfun)(r); break;
	    case 2: res = (*fun->vfun)(r,pop_dbl()); break;
	    case 3: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2));
		sp -= 2;
		break;

	    case 4: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2),
			 *(sp-3));
		sp -= 3;
		break;

	    case 5: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2),
			 *(sp-3), *(sp-4));
		sp -= 4;
		break;

	    case 6: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2),
			 *(sp-3), *(sp-4),
			 *(sp-5));
		sp -= 5;
		break;

	    case 7: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2),
			 *(sp-3), *(sp-4),
			 *(sp-5), *(sp-6));
		sp -= 6;
		break;

	    case 8: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2),
			 *(sp-3), *(sp-4),
			 *(sp-5), *(sp-6),
			 *(sp-7));
		sp -= 7;
		break;

	    case 9: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2),
			 *(sp-3), *(sp-4),
			 *(sp-5), *(sp-6),
			 *(sp-7), *(sp-8));
		sp -= 8;
		break;

	    case 10: 
		res = (*fun->vfun)(r, *(sp-1), *(sp-2),
			 *(sp-3), *(sp-4),
			 *(sp-5), *(sp-6),
			 *(sp-7), *(sp-8),
			 *(sp-9));
		sp -= 9;
		break;

	    default:
		break;
	    }
	    switch(fun->dim)
	    {
		case 1: r = res[0]; break;
		case 2: r = res[0]; push_dbl(res[1]); break;
		case 3: r = res[0]; push_dbl(res[2]); push_dbl(res[1]); break;
	    }
	    break;

	case INTERN_MAP:
	    switch(fun->nvars)
	    {
	    case 1:
		fun_vars[0] = r;
		break;
	    case 2:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		break;
	    case 3:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		break;
	    case 4:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		fun_vars[3] = pop_dbl();
		break;
	    case 5:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		fun_vars[3] = pop_dbl();
		fun_vars[4] = pop_dbl();
		break;

	    case 6:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		fun_vars[3] = pop_dbl();
		fun_vars[4] = pop_dbl();
		fun_vars[5] = pop_dbl();
		break;

	    case 7:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		fun_vars[3] = pop_dbl();
		fun_vars[4] = pop_dbl();
		fun_vars[5] = pop_dbl();
		fun_vars[6] = pop_dbl();
		break;

	    case 8:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		fun_vars[3] = pop_dbl();
		fun_vars[4] = pop_dbl();
		fun_vars[5] = pop_dbl();
		fun_vars[6] = pop_dbl();
		fun_vars[7] = pop_dbl();
		break;

	    case 9:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		fun_vars[3] = pop_dbl();
		fun_vars[4] = pop_dbl();
		fun_vars[5] = pop_dbl();
		fun_vars[6] = pop_dbl();
		fun_vars[7] = pop_dbl();
		fun_vars[8] = pop_dbl();
		break;

	    case 10:
		fun_vars[0] = r;
		fun_vars[1] = pop_dbl();
		fun_vars[2] = pop_dbl();
		fun_vars[3] = pop_dbl();
		fun_vars[4] = pop_dbl();
		fun_vars[5] = pop_dbl();
		fun_vars[6] = pop_dbl();
		fun_vars[7] = pop_dbl();
		fun_vars[8] = pop_dbl();
		fun_vars[9] = pop_dbl();
		break;

	    default:
		break;
	    }
  	    old_stack_base = dbl_vstack_base;
  	    dbl_vstack_base = sp; /* set global stack ptr */
	    switch(fun->dim)
	    {
		case 1:
	    		r = eval_vrpe(fun->vrpe[0],fun_vars);
			break;
		case 2:
	    		r = eval_vrpe(fun->vrpe[1],fun_vars); push_dbl(r);
  	    		dbl_vstack_base = sp;
	    		r = eval_vrpe(fun->vrpe[0],fun_vars);
			break;
		case 3:
	    		r = eval_vrpe(fun->vrpe[2],fun_vars); push_dbl(r);
  	    		dbl_vstack_base = sp;
	    		r = eval_vrpe(fun->vrpe[1],fun_vars); push_dbl(r);
  	    		dbl_vstack_base = sp;
	    		r = eval_vrpe(fun->vrpe[0],fun_vars);
			break;
	    }
  	    dbl_vstack_base = old_stack_base;
	    break;

#endif
	} /* End switch fun->type */
    }   

    else		/* Its got to be an op */

	/* Note for speed all the cases are close in value
		this results in faster switch statement on iris */
      switch( c )
      {
      case SUM1 + vrpe_op_base:
        r += pop_dbl();
        break;

      case SUB1 + vrpe_op_base:
        l = pop_dbl();
	r = l - r;
        break;

      case MULT1 + vrpe_op_base:
        r *= pop_dbl();
        break;

      case DIV1 + vrpe_op_base:
        l = pop_dbl();
	r = l / r;
        break;

      case POW1 + vrpe_op_base:
        l = pop_dbl();
        r = pow(r,l);
        break;

      case INT_POW + vrpe_op_base:
    	c = *(vrpe++);
	l = 1.0;
	for(;c>0;--c) l = l*r;
	r = l;
	break;

      case SUM2 + vrpe_op_base:
	r += *(sp+1);
	*(sp+2) += *(sp);
	sp += 2;
	break;

      case SUM3 + vrpe_op_base:
	r += *(sp+2);
	*(sp+3) += *sp;
	*(sp+4) += *(sp+1);
	sp += 3;
	break;

      case SUB2 + vrpe_op_base:
	r = *(sp+1) - r;
	*(sp+2) -= *sp;
	sp += 2;
	break;

      case SUB3 + vrpe_op_base:
	r = *(sp+2) - r;
	*(sp+3) -= *sp;
	*(sp+4) -= *(sp+1);
	sp += 3;
	break;

      case DOT2 + vrpe_op_base:
	r = *(sp+1) * r + *sp * *(sp+2);
	sp += 3;
	break;

      case DOT3 + vrpe_op_base:
	r = *(sp+2)  *r
	  + *sp * *(sp+3)
	  + *(sp+1) * *(sp+4);
	sp += 5;
	break;

      case SCALE2 + vrpe_op_base:
	*(sp+1) *= r;
	r *= *(sp);
	sp++;
	break;

      case SCALE3 + vrpe_op_base:
	*(sp+1) *= r;
	*(sp+2) *= r;
	r *= *(sp);
	sp++;
	break;

      case CPLX_MUL + vrpe_op_base:
	l = r * *(sp+2) + *sp * *(sp+1);
	r = r * *(sp+1) - *sp * *(sp+2);
	sp += 2;
	*(sp) = l;
	break;

      case CROSS2 + vrpe_op_base:
	r = r * *(sp+2) - *sp * *(sp+1);
	sp += 3;
	break;

      case CROSS3 + vrpe_op_base:
	l = r * *(sp+3) - *(sp) * *(sp+2);
	t = *(sp+1) * *(sp+2) - r * *(sp+4);
	r = *sp * *(sp+4) - *(sp+1) * *(sp+3);
	sp += 3;
	*sp = t;
	*(sp+1) = l;
	break;

      case EQUALS1 + vrpe_op_base:
        l = pop_dbl();
	r = l-r;
	push_dbl(r);
        return( sp );

      case SUM4 + vrpe_op_base:
	r += *(sp+3);
	*(sp+4) += *sp;
	*(sp+5) += *(sp+1);
	*(sp+6) += *(sp+2);
	sp += 4;
	break;

      case SUB4 + vrpe_op_base:
	r = *(sp+3) - r;
	*(sp+4) -= *sp;
	*(sp+5) -= *(sp+1);
	*(sp+6) -= *(sp+2);
	sp += 4;
	break;

      case DOT4 + vrpe_op_base:
	r = *(sp+3)  *r
	  + *sp * *(sp+4)
	  + *(sp+1) * *(sp+5)
	  + *(sp+2) * *(sp+6);
	sp += 7;
	break;

      case SCALE4 + vrpe_op_base:
	*(sp+1) *= r;
	*(sp+2) *= r;
	*(sp+3) *= r;
	r *= *(sp);
	sp++;
	break;

      case DIV2 + vrpe_op_base:
        *(sp+1) /= r;
        r = *(sp) / r;
        sp++;
        break;

      case DIV3 + vrpe_op_base:
        *(sp+1) /= r;
        *(sp+2) /= r;
        r = *(sp) / r;
        sp++;
        break;

      case CPLX_DIV + vrpe_op_base:
	t = *(sp+1) * *(sp+1) + *(sp+2) * *(sp+2);
	l =   (*sp * *(sp+1)  - r * *(sp+2) ) / t;
	r = ( r * *(sp+1) + *sp * *(sp+2) ) / t;
	sp += 2;
	*(sp) = l;
	break;

      case END_RPE:
	push_dbl(r);
        return( sp );
      }
   }
} 

#undef push_dbl
#undef pop_dbl

/*****
*     a routine to check that the rp-string will evaluate ok
*     'n' is the number of variables which are expected.
*****/

#define StackERR {\
      if( stackptr >= stack_size ) { \
        eprintf("check_vrpe: stack full, require %d\n",stackptr); \
        eprintf("           please recompile with 'cc -Dstack_size=999 ...\n"); \
        flag =  FALSE; } }

int check_vrpe( vrpe, n, names, nargs)
vrpeint vrpe[]; int n,nargs;
char **names;
{
  int ptr = 0;
  vrpeint c;
  eqn_funs *fun;
  static int stackptr=0;
  auto   int initStackPtr = stackptr;
  int flag = TRUE;
  char *itt_names[3];

  itt_names[0] = "res";
  itt_names[1] = "val";
  itt_names[2] = "num";

  if( vrpe == NULL )
  {
    eprintf("check_vrpe: NULL reverse polish string\n");
    return(FALSE);
  }

  do
  {
    c = vrpe[ptr++];

    if( c < vrpe_op_base )          /* c a variable */
    {
      StackERR;
      ++stackptr;
      if( c >= n )
      {
        eprintf("check_vrpe: bad variable referance var[%d]\n",c);
        flag =  FALSE;
      }
    }

    else if( c < vrpe_fun_base )  /* c an operator */
    {
      switch( c )
      {
      case SUM1 + vrpe_op_base:
      case SUB1 + vrpe_op_base:
      case MULT1 + vrpe_op_base:
      case DIV1 + vrpe_op_base:
      case POW1 + vrpe_op_base:
      case EQUALS1 + vrpe_op_base:
	--stackptr;
        break;
      case SUM2 + vrpe_op_base:
      case SUB2 + vrpe_op_base:
	stackptr -= 2;
	break;
      case SUM3 + vrpe_op_base:
      case SUB3 + vrpe_op_base:
	stackptr -= 3;
	break;
      case SUM4 + vrpe_op_base:
      case SUB4 + vrpe_op_base:
	stackptr -= 4;
	break;
      case DOT2 + vrpe_op_base:
	stackptr -= 3;
	break;
      case DOT3 + vrpe_op_base:
	stackptr -= 5;
	break;
      case DOT4 + vrpe_op_base:
	stackptr -= 7;
	break;
      case SCALE2 + vrpe_op_base:
      case SCALE3 + vrpe_op_base:
      case SCALE4 + vrpe_op_base:
      case DIV2 + vrpe_op_base:
      case DIV3 + vrpe_op_base:
	--stackptr;
	break;
      case CPLX_MUL + vrpe_op_base:
      case CPLX_DIV + vrpe_op_base:
	stackptr -= 2;
	break;
      case CROSS2 + vrpe_op_base:
      case CROSS3 + vrpe_op_base:
	stackptr -= 3;
	break;
      case INT_POW + vrpe_op_base:
        c = vrpe[ptr++];
        break;
      case END_RPE:
	break;

      default:
	eprintf("Bad op "); eprint_op(c-vrpe_op_base); eprintf(" in check_vrpe\n");
	break;
      }
    }

    else if( c < vrpe_sum_base ) /* a function */
    {
      fun = vrpe_funs[c-vrpe_fun_base];
      switch( fun->type )
      {
      case EXTERN_FUN:
      	stackptr -= fun->nvars - 1;
      	StackERR;
	break;
      
      case INTERN_FUN:
      	stackptr -= fun->nvars - 1;
      	StackERR;
	flag = flag && check_rpe(fun->rpe,fun->nvars,fun->vars);
	break;

      case EXTERN_MAP:
      	stackptr -= fun->nvars - fun->dim;
      	StackERR;
	break;

      case INTERN_MAP:
      	stackptr -= fun->nvars - fun->dim;
      	StackERR;
	    flag = flag && check_vrpe(fun->vrpe,fun->nvars,fun->vars);
	break;
      
      default:
	eprintf("Bad type for function when checking vrpe %d\n",
		fun->type);
	break;
      }
    }
    else if( c < vrpe_const_base ) /* a sum type function */
    {
      fun = vrpe_sums[c-vrpe_sum_base].fun;
      if ( fun->type != SUM_FUN )
      {
		eprintf("Function should be a sum function but its a %d\n",
			fun->type);
		return(FALSE);
      }
      	stackptr -= (fun->nvars-2);
	flag = flag && check_vrpe(vrpe_sums[c-vrpe_sum_base].vrpe,n,names,1);
/* one result left on stack by the check_vrpe fun 
*/
      	StackERR;
    }
    else			/*  c a constant */
    {
      StackERR;
      ++stackptr;
      if( c < vrpe_const_base || c >= vrpe_const_base + vrpe_const_max )
      {
        eprintf("check_vrpe: bad constant ref, const[%d]\n",c-vrpe_const_base);
        flag =  FALSE;
      }
    }    /* end if */

  } while( c != END_RPE );

  if( stackptr != initStackPtr + nargs )
  {
	eprintf("check_vrpe: stack corupted on exit: initial %d final %d args %d\n",
		initStackPtr,stackptr,nargs);
	eprint_vrpe(vrpe,names);
/*
  	flag = FALSE;
*/
	stackptr = initStackPtr;
  }
  if( !flag ) stackptr = initStackPtr;
  return( flag );
}

/****  resets constant stack to zero ****/

void clear_vrpe_const() {vrpe_const_ptr = 0;}

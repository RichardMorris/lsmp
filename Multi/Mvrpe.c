/*
 * Copyright I guess there should be some copywrite for this package,
 * 
 * 			Copyright (c) 1996
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
#include "Multi.h"
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


/**************************************************************************/
/*                                                                        */
/* Now we hav a reverse polish calculator                                 */
/*  This is the versions which works with Multi : multi equation blocks   */
/*  It can handle variables of diferent dimensions                        */
/*                                                                        */
/*   make_Mvrpe( eqn , mul ) creates a reverse polish string  		  */
/*   eval_Mvrpe( vrpe, vars[] )        evaluates it using a stack         */
/*   clear_Mvrpe_const()              clears the constant array           */
/*   check_Mvrpe( vrpe, .. )       checks that vrpe will be sucessfully   */
/*                                    evaluated.                          */
/*   print_Mvrpe( vrpe, n ,names[n] )  prints out the rp-string           */
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
/*      make_Mvrpe() returns NULL if a valid rp-string can not be found     */
/*   in particular this will happen if eqn contains an unknown name.      */
/*                                                                        */
/**************************************************************************/

/**** Definitions used for the reverse polish calculator ****/

#define Mvrpe_name_base 0
#define Mvrpe_op_base 256
#define END_RPE Mvrpe_op_base 
#define Mvrpe_fun_base 512
#define Mvrpe_fun_max  256
#define Mvrpe_const_base (Mvrpe_fun_base + Mvrpe_fun_max)
#ifdef BOR_THINK
#define Mvrpe_const_max  1000
#else
#define Mvrpe_const_max  10000
#endif

eqn_funs *Mvrpe_funs[Mvrpe_fun_max];
int Mvrpe_fun_ptr = 0;
double Mvrpe_const[Mvrpe_const_max];
int Mvrpe_const_ptr = 0 ;		/* the current position in array */
int Mvrpe_ptr;

/**** A stack 'Mvdbl_stack' is used for evaluating an rp-string *****/


#ifndef stack_size 
#  define stack_size 500
#endif
double Mvdbl_stack[stack_size];

int Mglobal_check_stackptr = 0;

int make_Mvrpe2();
int check_Mvrpe();

/*
 * Function:	make_Mvrpe
 * Action:	constructs an Mvrpe string,
 *		returns NULL if failed
*/

Mvrpeint *Mmake_vrpe(mul, eqn )
eqnode *eqn; Multi *mul;
{
  Mvrpeint *temp;
  int Mvrpe_size;

  if(eqn == NULL )
  {
	eprintf("make_Mvrpe: NULL eqnation\n");
	return(NULL);
  }
  if(mul == NULL)
  {
	eprintf("makeMvrpe: NULL multiple equation block\n");
	return(NULL);
  }
  Mglobal_check_stackptr = 0;
  Mvrpe_size = Mcount_eqn( eqn, mul); /* the length of string is number of
                                         nodes in the equation */
  Mvrpe_ptr = 0;
  temp = (Mvrpeint *) calloc( Mvrpe_size + 1, sizeof(Mvrpeint) );
  temp[Mvrpe_size] = END_RPE; /* a flag to signal end of string */
  if( !make_Mvrpe2( temp, eqn, mul )
   || !check_Mvrpe( temp, Mget_dim(mul,eqn), mul ))
  {
    free(temp);
    return(NULL);
  }
  return( temp );
}

/**** a recursive sub-routine called by make_Mvrpe  ****/

int make_Mvrpe2( Mvrpe, eqn, mul )
Mvrpeint Mvrpe[]; eqnode *eqn; Multi *mul;
{
  int leftres,rightres,leftcount,rightcount,i,j;
  double num;
/*
fprintf(stderr,"Make Mvrpe2 ");
fprint_eqn(stderr,eqn);
fprintf(stderr,"\n");
*/
  switch( eqn->op )
  {
  case NAME:
    for(i=0;i<mul->n_big;++i)
    {
      if( !strcmp( eqn->u.str, mul->bignames[i] ) )
      {
	for(j=mul->bigdim[i];j>0;--j)
        	Mvrpe[Mvrpe_ptr++] = (Mvrpeint) (mul->dimsum[i] - j)
			 + Mvrpe_name_base;
        return( TRUE );
      }
    }
    eprintf("make_Mvrpe2: name not found %s\n",eqn->u.str);
    return( FALSE );

  case NUMBER:
    for(i=0;i<Mvrpe_const_ptr;++i)
    {
	if( Mvrpe_const[i] == eqn->u.num )
	{
		Mvrpe[Mvrpe_ptr++] = (Mvrpeint) Mvrpe_const_base + i;
		return(TRUE);
	}
    }
    Mvrpe[Mvrpe_ptr++] = (Mvrpeint) Mvrpe_const_ptr + Mvrpe_const_base ;
    Mvrpe_const[Mvrpe_const_ptr] = eqn->u.num;
    if ( ++Mvrpe_const_ptr >= Mvrpe_const_max )
    {
      eprintf("make_Mvrpe2: too many constants\n");
      return(FALSE);
    }
    return( TRUE );

  case '+':
    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
    leftcount = Mget_dim(mul,eqn->u.n.l);
    rightcount = Mget_dim(mul,eqn->u.n.r);
    if( leftcount != rightcount )
    {
	eprintf("make_Mvrpe: different counts for + %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    switch(leftcount)
    {
	case 1: Mvrpe[Mvrpe_ptr++] = SUM1 + Mvrpe_op_base; break;
	case 2: Mvrpe[Mvrpe_ptr++] = SUM2 + Mvrpe_op_base; break;
	case 3: Mvrpe[Mvrpe_ptr++] = SUM3 + Mvrpe_op_base; break;
	case 4: Mvrpe[Mvrpe_ptr++] = SUM4 + Mvrpe_op_base; break;
	default:
		eprintf("make_Mvrpe: bad count %d\n",leftcount);
		return(FALSE);
    }
    return( leftres && rightres );

  case '-': case '=':
    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
    leftcount = Mget_dim(mul,eqn->u.n.l);
    rightcount = Mget_dim(mul,eqn->u.n.r);
    if( leftcount != rightcount )
    {
	eprintf("make_Mvrpe: different counts for + %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    switch(leftcount)
    {
	case 1: Mvrpe[Mvrpe_ptr++] = SUB1 + Mvrpe_op_base; break;
	case 2: Mvrpe[Mvrpe_ptr++] = SUB2 + Mvrpe_op_base; break;
	case 3: Mvrpe[Mvrpe_ptr++] = SUB3 + Mvrpe_op_base; break;
	case 4: Mvrpe[Mvrpe_ptr++] = SUB4 + Mvrpe_op_base; break;
	default:
		eprintf("make_Mvrpe: bad count %d\n",leftcount);
		return(FALSE);
    }
    return( leftres && rightres );

  case '*':
    leftcount = Mget_dim(mul,eqn->u.n.l);
    rightcount = Mget_dim(mul,eqn->u.n.r);
    if( leftcount == 2 && rightcount == 2 )
    {
            rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul );
            leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul );
            Mvrpe[Mvrpe_ptr++] = CPLX_MUL + Mvrpe_op_base;
            return( leftres && rightres );
    }


    if( leftcount != 1 && rightcount != 1)
    {
	eprintf("make_Mvrpe: bad counts for '*' %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    if(leftcount == 1)
    {
	    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
	    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
	    switch(rightcount)
	    {
		case 1: Mvrpe[Mvrpe_ptr++] = MULT1 + Mvrpe_op_base; break;
		case 2: Mvrpe[Mvrpe_ptr++] = SCALE2 + Mvrpe_op_base; break;
		case 3: Mvrpe[Mvrpe_ptr++] = SCALE3 + Mvrpe_op_base; break;
		case 4: Mvrpe[Mvrpe_ptr++] = SCALE4 + Mvrpe_op_base; break;
		default:
			eprintf("make_Mvrpe: bad count %d\n",rightcount);
			return(FALSE);
	    }
	    return( leftres && rightres );
    }
    else
    {
		/* reverse order */

	    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
	    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
	    switch(leftcount)
	    {
		case 1: Mvrpe[Mvrpe_ptr++] = MULT1 + Mvrpe_op_base; break;
		case 2: Mvrpe[Mvrpe_ptr++] = SCALE2 + Mvrpe_op_base; break;
		case 3: Mvrpe[Mvrpe_ptr++] = SCALE3 + Mvrpe_op_base; break;
		case 4: Mvrpe[Mvrpe_ptr++] = SCALE4 + Mvrpe_op_base; break;
		default:
			eprintf("make_Mvrpe: bad count %d\n",leftcount);
			return(FALSE);
	    }
	    return( leftres && rightres );
    }

  case '/':
#ifdef NOT_DEF
	/* right added before the left */
    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
    leftcount = Mget_dim(mul,eqn->u.n.l);
    rightcount = Mget_dim(mul,eqn->u.n.r);
    if( leftcount != rightcount || leftcount != 1 ) 
    {
	eprintf("make_Mvrpe: bad counts for / %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    Mvrpe[Mvrpe_ptr++] = DIV1 + Mvrpe_op_base;
    return( leftres && rightres );
#endif
    leftcount = Mget_dim(mul,eqn->u.n.l);
    rightcount = Mget_dim(mul,eqn->u.n.r);
    if( leftcount == 2 && rightcount == 2 )
    {
            rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul );
            leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul );
            Mvrpe[Mvrpe_ptr++] = CPLX_DIV + Mvrpe_op_base;
            return( leftres && rightres );
    }

    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
 
    if( rightcount != 1 )
    {
        eprintf("make_Mvrpe: bad counts for / %d %d\n",
                leftcount,rightcount);
        return(FALSE);
    }
    switch(leftcount)
    {
        case 1: Mvrpe[Mvrpe_ptr++] = DIV1 + Mvrpe_op_base; break;
        case 2: Mvrpe[Mvrpe_ptr++] = DIV2 + Mvrpe_op_base; break;
        case 3: Mvrpe[Mvrpe_ptr++] = DIV3 + Mvrpe_op_base; break;
        default:
                eprintf("make_Mvrpe: bad counts for / %d 1\n",leftcount);
                        return(FALSE);
    }
    return( leftres && rightres );


  case '^':
    rightcount = Mget_dim(mul,eqn->u.n.r);
    leftcount = Mget_dim(mul,eqn->u.n.l);
    if( leftcount == 3 && rightcount == 3)	/* Cross product */
    {
	rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
	leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
	Mvrpe[Mvrpe_ptr++] = CROSS3 + Mvrpe_op_base;
	return(leftres && rightres);
    }
    if( leftcount == 2 && rightcount == 2)	/* Cross product 2D */
    {
	rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
	leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
	Mvrpe[Mvrpe_ptr++] = CROSS2 + Mvrpe_op_base;
	return(leftres && rightres);
    }

    else if( leftcount == 1 && rightcount == 1 )/* Power */
    {
	if(eqn->u.n.r->op == NUMBER )
	{
		num = eqn->u.n.r->u.num;
		if( num - floor(num) < 1.0e-9 && num > 0.0 && num < 32768.0 )
		{
			leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
			Mvrpe[Mvrpe_ptr++] = (Mvrpeint) INT_POW + Mvrpe_op_base;
			Mvrpe[Mvrpe_ptr++] = (Mvrpeint) num;
			return(leftres);
		}
	}
	rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
	leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
	Mvrpe[Mvrpe_ptr++] = POW1 + Mvrpe_op_base;
	return( leftres && rightres );
    }
    else
    {
	eprintf("make_Mvrpe: bad counts for ^ %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }

  case '.':
    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
    leftcount = Mget_dim(mul,eqn->u.n.l);
    rightcount = Mget_dim(mul,eqn->u.n.r);
    if( leftcount != rightcount )
    {
	eprintf("make_Mvrpe: different counts for '.' %d %d\n",
		leftcount,rightcount);
	return(FALSE);
    }
    switch(leftcount)
    {
	case 1: Mvrpe[Mvrpe_ptr++] = MULT1 + Mvrpe_op_base; break;
	case 2: Mvrpe[Mvrpe_ptr++] = DOT2 + Mvrpe_op_base; break;
	case 3: Mvrpe[Mvrpe_ptr++] = DOT3 + Mvrpe_op_base; break;
	case 4: Mvrpe[Mvrpe_ptr++] = DOT4 + Mvrpe_op_base; break;
	default:
		eprintf("make_Mvrpe: bad count %d\n",leftcount);
		return(FALSE);
    }
    return( leftres && rightres );

  case ',':
    rightres = make_Mvrpe2( Mvrpe, eqn->u.n.r, mul);
    leftres = make_Mvrpe2( Mvrpe, eqn->u.n.l, mul);
    return( leftres && rightres );

  case FUNCTION:
	switch(eqn->u.f.f->type)
	{
	case CONSTANT_FUN:
	    for(i=0;i<Mvrpe_const_ptr;++i)
	    {
		if( Mvrpe_const[i] == eqn->u.f.f->val )
		{
			Mvrpe[Mvrpe_ptr++] = (Mvrpeint) Mvrpe_const_base + i;
			return(TRUE);
		}
	    }
	    Mvrpe[Mvrpe_ptr++] = (Mvrpeint) Mvrpe_const_ptr + Mvrpe_const_base ;
	    Mvrpe_const[Mvrpe_const_ptr] = eqn->u.f.f->val;
	    if ( ++Mvrpe_const_ptr >= Mvrpe_const_max )
	    {
	      eprintf("make_Mvrpe2: too many constants\n");
	      return(FALSE);
	    }
	    return( TRUE );

	case OPERATOR:
		eprintf("Can't convert equation involving operator %s into a reverse polish string\n",eqn->u.f.f->name);
		return(FALSE);

	case INTERN_FUN: case EXTERN_FUN:
	case INTERN_MAP: case EXTERN_MAP:
		if( Mget_dim(mul,eqn->u.f.a) != eqn->u.f.f->nvars )
		{
			eprintf("Actual argument count different to profile for function %s\n",eqn->u.f.f->name);
			return(FALSE);
		}
	
		if( eqn->u.f.f->nvars > MAX_FUN_ARGS )
		{
			eprintf("Sorry function %s has too many arguments to convert to reverse polish string\n",eqn->u.f.f->name);
			return(FALSE);
		}
	
		leftres = make_Mvrpe2(Mvrpe,eqn->u.f.a, mul);

		/* Check the list of Mvrpe_funs */

		for(i=0;i<Mvrpe_fun_ptr;++i)
		{
			if(Mvrpe_funs[i] == eqn->u.f.f)
			{
				Mvrpe[Mvrpe_ptr++] = (Mvrpeint) Mvrpe_fun_base + i;
				return(TRUE);
			}
		}
		Mvrpe[Mvrpe_ptr++] = (Mvrpeint) Mvrpe_fun_ptr + Mvrpe_fun_base;
		Mvrpe_funs[Mvrpe_fun_ptr] = eqn->u.f.f;
		if( ++Mvrpe_fun_ptr >= Mvrpe_fun_max )
		{
			eprintf("make_Mvrpe2: too many functions\n");
			return(FALSE);
		}
		return(leftres);

	case SUM_FUN:
#ifdef NOT_DEF
		if( Mget_dim(mul,eqn->u.f.a) != eqn->u.f.f->nvars )
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
    		for(i=0;i<mul->n_big;++i)
                {
                        if(!strcmp( eqn->u.f.a->u.n.r->u.n.l->u.str,
				mul->bignames[i]) )
                        {
                                Mvrpe_sums[Mvrpe_sum_ptr].var_ref = i;
                                break;
                        }
                }
                if(i==mul->n_big)
                {
                        eprintf("Second operator (%s) of %s not found in list of names\n", eqn->u.f.a->u.n.r->u.n.l->u.str,eqn->u.f.f->name);
                        return(FALSE);
                }
/*
printf("Before vrpe is\n");
print_vrpe(Mvrpe,mul->bignames);
*/
                Mvrpe_sums[Mvrpe_sum_ptr].fun = eqn->u.f.f;
                leftres = Mmake_vrpe2(mul,vrpe,eqn->u.f.a->u.n.r->u.n.r);
                Mvrpe[Mvrpe_ptr++] = (rpeint)
                         Mvrpe_sum_ptr + Mvrpe_sum_base;
/*
printf("Mmake_vrpe2 vrpe is\n");
*/
                old_Mvrpe_ptr = Mvrpe_ptr;
                Mvrpe_sums[Mvrpe_sum_ptr].Mvrpe =
                                make_Mvrpe(eqn->u.f.a->u.n.l,n,names);
                Mvrpe_ptr = old_Mvrpe_ptr;
                if(  Mvrpe_sums[Mvrpe_sum_ptr].Mvrpe == NULL ) 
                {
                        eprintf("Error making sum equation\n");
                        eprint_eqn(eqn->u.f.a->u.n.l);
                        eprintf("\n");
                        return(FALSE);
                }
/*
printf("After\n");
print_vrpe(Mvrpe,mul->bignames);
*/

                ++Mvrpe_sum_ptr;
                if( Mvrpe_sum_ptr >= Mvrpe_sum_max )
                {
                        eprintf("Too many sum-type expressions max %d\n",
                                Mvrpe_sum_max-1);
                        return(FALSE);
                }
                return(leftres);
#endif
		

		
	default:
		eprintf("Bad function type %d in make_Mvrpe\n",eqn->u.f.f->type);
		return(FALSE);
	}

  default:
	eprintf("make_Mvrpe2: bad op ");
        eprint_op(eqn->op);
        eprintf("\n");
	break;
  }
  return(FALSE);
}

/*****
*     prints out the Mvrpe string 'Mvrpe'
*     'names' is an array of 'n' names.
*****/

void print_Mvrpe( Mvrpe, mul )
Mvrpeint Mvrpe[]; Multi *mul;
{
  int ptr = 0,i,j;
  Mvrpeint c;

  if( Mvrpe == NULL )
  {
    printf("NULL reverse polish string\n");
    return;
  }

  do
  {
    c = Mvrpe[ptr++];
    if( c < Mvrpe_op_base )          
    {
	for(i=0;i<mul->n_big;++i)
	{
		j = mul->dimsum[i];
		if( c < j )
		{
			 printf("\tvar\t%s\tcomp\t%d\n",
				mul->bignames[i], 
				j-c);
			break;
		}
	}
    }
    else if( c < Mvrpe_fun_base ) {  printf("\top\t");
				    print_op(c-Mvrpe_op_base); 
				    printf("\n"); }
    else if( c < Mvrpe_const_base )  printf("\tfun\t%s\n",
					Mvrpe_funs[c-Mvrpe_fun_base]->name);
    else			   printf("\tconst\t%lf\n",
					Mvrpe_const[c-Mvrpe_const_base]);
  }
  while( c != END_RPE && c != Mvrpe_op_base + '=' );
}

void fprint_Mvrpe(fp, Mvrpe, mul )
FILE *fp;
Mvrpeint Mvrpe[]; Multi *mul;
{
  int ptr = 0,i,j;
  Mvrpeint c;

  if( Mvrpe == NULL )
  {
    fprintf(fp,"NULL reverse polish string\n");
    return;
  }

  do
  {
    c = Mvrpe[ptr++];
    if( c < Mvrpe_op_base )
    {
	for(i=0;i<mul->n_big;++i)
	{
		j = mul->dimsum[i];
		if( c < j )
		{
			 fprintf(fp,"\tvar\t%s\tcomp\t%d\n",
				mul->bignames[i], 
				j-c);
			break;
		}
	}
    }
    else if ( c < Mvrpe_fun_base ) { fprintf(fp,"\top\t");
				    fprint_op(fp,c-Mvrpe_op_base); 
				    fprintf(fp,"\n"); }
    else if ( c < Mvrpe_const_base ) fprintf(fp,"\tfun\t%s\n",
					Mvrpe_funs[c-Mvrpe_fun_base]->name);
    else			   fprintf(fp,"\tconst\t%lf\n",
					Mvrpe_const[c-Mvrpe_const_base]);
  }
  while( c != END_RPE && c != Mvrpe_op_base + '=' );
}

void eprint_Mvrpe(Mvrpe, mul )
Mvrpeint Mvrpe[]; Multi *mul;
{
  int ptr = 0,i,j;
  Mvrpeint c;

  if( Mvrpe == NULL )
  {
    eprintf("NULL reverse polish string\n");
    return;
  }

  do
  {
    c = Mvrpe[ptr++];
    if( c < Mvrpe_op_base )
    {
	for(i=0;i<mul->n_big;++i)
	{
		j = mul->dimsum[i];
		if( c < j )
		{
			 eprintf("\tvar\t%s\tcomp\t%d\n",
				mul->bignames[i], 
				j-c);
			break;
		}
	}
    }
    else if ( c < Mvrpe_fun_base ) { eprintf("\top\t");
				    eprint_op(c-Mvrpe_op_base);
				    eprintf("\n"); }
    else if ( c < Mvrpe_const_base ) eprintf("\tfun\t%s\n",
					Mvrpe_funs[c-Mvrpe_fun_base]->name);
    else			   eprintf("\tconst\t%lf\n",
					Mvrpe_const[c-Mvrpe_const_base]);
  }
  while( c != END_RPE && c != Mvrpe_op_base + '=' );
}

/*****
*     The no-holds bared Mvrpe-calculator.
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

double *Mdbl_vstack_base = Mvdbl_stack + stack_size; 
					/* pointers quicker than arrays */
 
double *Meval_vrpe( Mvrpe, vars )
register Mvrpeint *Mvrpe; 		/*  All variables are registers  */
register double *vars;
{
  register Mvrpeint c;
  register double l,r,t;
  register double *sp = Mdbl_vstack_base;
  double   *old_stack_base;
  eqn_funs *fun;

  if( Mvrpe == NULL )
  {
    eprintf("eval_Mvrpe: NULL reverse polish string\n");
    *Mdbl_vstack_base = sqrt(-1.0);
    return(Mdbl_vstack_base);
  }

	/*CONSTCOND*/
  while( TRUE )
  {
    c = *(Mvrpe++);

    if( c < Mvrpe_op_base ) 
    {
        push_dbl(r);			/* one extra push at begining */
        r = *(vars+c); 			/* r holds the top of stack */
    }

    else if( c >= Mvrpe_const_base ) 
    {
        push_dbl(r);
        r = Mvrpe_const[ c - Mvrpe_const_base ];
    }

    else if( c >= Mvrpe_fun_base )
    {
	fun =  Mvrpe_funs[c - Mvrpe_fun_base];
	switch(fun->type)
	{
	case EXTERN_FUN:
	    switch( fun->nvars )
	    {
	    case 1: r = (*fun->fun)(r); break;
	    case 2: r = (*fun->fun)(r,*sp); sp += 1; break;
	    case 3: r = (*fun->fun)(r, *sp, *(sp+1)); sp += 2; break;

	    case 4: 
		r = (*fun->fun)(r, *(sp), *(sp+1), *(sp+2));
		sp += 3;
		break;

	    case 5: 
		r = (*fun->fun)(r, *(sp), *(sp+1), *(sp+2), *(sp+3));
		sp += 4;
		break;

	    case 6: 
		r = (*fun->fun)(r, *(sp), *(sp+1), *(sp+2), *(sp+3), *(sp+4));
		sp += 5;
		break;

	    case 7: 
		r = (*fun->fun)(r, *(sp), *(sp+1), *(sp+2), *(sp+3), *(sp+4),
			 *(sp+5));
		sp += 6;
		break;

	    case 8: 
		r = (*fun->fun)(r, *(sp), *(sp+1), *(sp+2), *(sp+3), *(sp+4),
			 *(sp+5), *(sp+6));
		sp += 7;
		break;

	    case 9: 
		r = (*fun->fun)(r, *(sp), *(sp+1), *(sp+2), *(sp+3), *(sp+4),
			 *(sp+5), *(sp+6), *(sp+7));
		sp += 8;
		break;

	    case 10: 
		r = (*fun->fun)(r, *(sp), *(sp+1), *(sp+2), *(sp+3), *(sp+4),
			 *(sp+5), *(sp+6), *(sp+7), *(sp+8));
		sp += 9;
		break;

	    default:
		break;
	    }
	    break;

	case INTERN_FUN:
	    push_dbl(r);
  	    old_stack_base = Mdbl_vstack_base;
  	    Mdbl_vstack_base = sp;
	    r = eval_rpe(fun->rpe,sp);
  	    Mdbl_vstack_base = old_stack_base;
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
  	    old_stack_base = Mdbl_vstack_base;
  	    Mdbl_vstack_base = sp; /* set global stack ptr */
	    switch(fun->dim)
	    {
		case 1:
	    		r = eval_vrpe(fun->Mvrpe[0],fun_vars);
			break;
		case 2:
	    		r = eval_vrpe(fun->Mvrpe[1],fun_vars); push_dbl(r);
  	    		Mdbl_vstack_base = sp;
	    		r = eval_vrpe(fun->Mvrpe[0],fun_vars);
			break;
		case 3:
	    		r = eval_vrpe(fun->Mvrpe[2],fun_vars); push_dbl(r);
  	    		Mdbl_vstack_base = sp;
	    		r = eval_vrpe(fun->Mvrpe[1],fun_vars); push_dbl(r);
  	    		Mdbl_vstack_base = sp;
	    		r = eval_vrpe(fun->Mvrpe[0],fun_vars);
			break;
	    }
  	    Mdbl_vstack_base = old_stack_base;
	    break;

#endif
	} /* End switch fun->type */
    }   

    else		/* Its got to be an op */

	/* Note for speed all the cases are close in value
		this results in faster switch statement on iris */
      switch( c )
      {
      case SUM1 + Mvrpe_op_base:
        r += pop_dbl();
        break;

      case SUB1 + Mvrpe_op_base:
        l = pop_dbl();
	r = l - r;
        break;

      case MULT1 + Mvrpe_op_base:
        r *= pop_dbl();
        break;

      case DIV1 + Mvrpe_op_base:
        l = pop_dbl();
	r = l / r;
        break;

      case POW1 + Mvrpe_op_base:
        l = pop_dbl();
        r = pow(r,l);
        break;

      case INT_POW + Mvrpe_op_base:
    	c = *(Mvrpe++);
	l = 1.0;
	for(;c>0;--c) l = l*r;
	r = l;
	break;

      case SUM2 + Mvrpe_op_base:
	r += *(sp+1);
	*(sp+2) += *(sp);
	sp += 2;
	break;

      case SUM3 + Mvrpe_op_base:
	r += *(sp+2);
	*(sp+3) += *sp;
	*(sp+4) += *(sp+1);
	sp += 3;
	break;

      case SUB2 + Mvrpe_op_base:
	r = *(sp+1) - r;
	*(sp+2) -= *sp;
	sp += 2;
	break;

      case SUB3 + Mvrpe_op_base:
	r = *(sp+2) - r;
	*(sp+3) -= *sp;
	*(sp+4) -= *(sp+1);
	sp += 3;
	break;

      case DOT2 + Mvrpe_op_base:
	r = *(sp+1) * r + *sp * *(sp+2);
	sp += 3;
	break;

      case DOT3 + Mvrpe_op_base:
	r = *(sp+2)  *r
	  + *sp * *(sp+3)
	  + *(sp+1) * *(sp+4);
	sp += 5;
	break;

      case SCALE2 + Mvrpe_op_base:
	*(sp+1) *= r;
	r *= *(sp);
	sp++;
	break;

      case SCALE3 + Mvrpe_op_base:
	*(sp+1) *= r;
	*(sp+2) *= r;
	r *= *(sp);
	sp++;
	break;

      case CPLX_MUL + Mvrpe_op_base:
        l = r * *(sp+2) + *sp * *(sp+1);
        r = r * *(sp+1) - *sp * *(sp+2);
        sp += 2;
        *(sp) = l;
        break;

      case CROSS2 + Mvrpe_op_base:
        r = r * *(sp+2) - *sp * *(sp+1);
        sp += 3;
        break;

      case CROSS3 + Mvrpe_op_base:
	l = r * *(sp+3) - *(sp) * *(sp+2);
	t = *(sp+1) * *(sp+2) - r * *(sp+4);
	r = *sp * *(sp+4) - *(sp+1) * *(sp+3);
	sp += 3;
	*sp = t;
	*(sp+1) = l;
	break;

      case EQUALS1 + Mvrpe_op_base:
        l = pop_dbl();
	r = l-r;
	push_dbl(r);
        return( sp );

      case SUM4 + Mvrpe_op_base:
	r += *(sp+3);
	*(sp+4) += *sp;
	*(sp+5) += *(sp+1);
	*(sp+6) += *(sp+2);
	sp += 4;
	break;

      case SUB4 + Mvrpe_op_base:
	r = *(sp+3) - r;
	*(sp+4) -= *sp;
	*(sp+5) -= *(sp+1);
	*(sp+6) -= *(sp+2);
	sp += 4;
	break;

      case DOT4 + Mvrpe_op_base:
	r = *(sp+3)  *r
	  + *sp * *(sp+4)
	  + *(sp+1) * *(sp+5)
	  + *(sp+2) * *(sp+6);
	sp += 7;
	break;

      case SCALE4 + Mvrpe_op_base:
	*(sp+1) *= r;
	*(sp+2) *= r;
	*(sp+3) *= r;
	r *= *(sp);
	sp++;
	break;

      case DIV2 + Mvrpe_op_base:
        *(sp+1) /= r;
        r = *(sp) / r;
        sp++;
        break;

      case DIV3 + Mvrpe_op_base:
        *(sp+1) /= r;
        *(sp+2) /= r;
        r = *(sp) / r;
        sp++;
        break;

      case CPLX_DIV + Mvrpe_op_base:
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
/*NOTREACHED*/
} 

#undef push_dbl
#undef pop_dbl

/*****
*     a routine to check that the rp-string will evaluate ok
*     'n' is the number of variables which are expected.
*****/


#define StackERR {\
      if( Mglobal_check_stackptr >= stack_size ) { \
        eprintf("check_Mvrpe: stack full, require %d\n",Mglobal_check_stackptr); \
        eprintf("           please recompile with 'cc -Dstack_size=999 ...\n"); \
        flag =  FALSE; } }

int check_Mvrpe( Mvrpe, nargs, mul)
Mvrpeint Mvrpe[]; int nargs; Multi *mul;
{
  int ptr = 0,i;
  Mvrpeint c;
  eqn_funs *fun;
  int initStackPtr = Mglobal_check_stackptr;
  int flag = TRUE;

  if( Mvrpe == NULL )
  {
    eprintf("check_Mvrpe: NULL reverse polish string\n");
    return(FALSE);
  }

  do
  {
    c = Mvrpe[ptr++];

    if( c < Mvrpe_op_base )          /* c a variable */
    {
      StackERR;
      ++Mglobal_check_stackptr;
      if( c >= mul->dimsum[mul->n_big-1] )
      {
        eprintf("check_Mvrpe: bad variable referance var[%d]\n",c);
        flag =  FALSE;
      }
    }

    else if( c < Mvrpe_fun_base )  /* c an operator */
    {
      switch( c )
      {
      case SUM1 + Mvrpe_op_base:
      case SUB1 + Mvrpe_op_base:
      case MULT1 + Mvrpe_op_base:
      case DIV1 + Mvrpe_op_base:
      case POW1 + Mvrpe_op_base:
      case EQUALS1 + Mvrpe_op_base:
	--Mglobal_check_stackptr;
        break;
      case SUM2 + Mvrpe_op_base:
      case SUB2 + Mvrpe_op_base:
	Mglobal_check_stackptr -= 2;
	break;
      case SUM3 + Mvrpe_op_base:
      case SUB3 + Mvrpe_op_base:
	Mglobal_check_stackptr -= 3;
	break;
      case SUM4 + Mvrpe_op_base:
      case SUB4 + Mvrpe_op_base:
	Mglobal_check_stackptr -= 4;
	break;
      case DOT2 + Mvrpe_op_base:
	Mglobal_check_stackptr -= 3;
	break;
      case DOT3 + Mvrpe_op_base:
	Mglobal_check_stackptr -= 5;
	break;
      case DOT4 + Mvrpe_op_base:
	Mglobal_check_stackptr -= 7;
	break;
      case SCALE2 + Mvrpe_op_base:
      case SCALE3 + Mvrpe_op_base:
      case SCALE4 + Mvrpe_op_base:
      case DIV2 + Mvrpe_op_base:
      case DIV3 + Mvrpe_op_base:
	--Mglobal_check_stackptr;
	break;
      case CPLX_MUL + Mvrpe_op_base:
      case CPLX_DIV + Mvrpe_op_base:
	Mglobal_check_stackptr -= 2;
	break;
      case CROSS3 + Mvrpe_op_base:
      case CROSS2 + Mvrpe_op_base:
	Mglobal_check_stackptr -= 3;
	break;
      case INT_POW + Mvrpe_op_base:
        c = Mvrpe[ptr++];
        break;
      case END_RPE:
	break;

      default:
	eprintf("Bad op "); eprint_op(c-Mvrpe_op_base); eprintf(" in check_Mvrpe\n");
	break;
      }
    }

    else if( c < Mvrpe_const_base ) /* a function */
    {
      fun = Mvrpe_funs[c-Mvrpe_fun_base];
      switch( fun->type )
      {
      case EXTERN_FUN:
      	Mglobal_check_stackptr -= fun->nvars - 1;
      	StackERR;
	break;
      
      case INTERN_FUN:
      	Mglobal_check_stackptr -= fun->nvars - 1;
      	StackERR;
	flag = flag && check_rpe(fun->rpe,fun->nvars,fun->vars);
	break;

      case EXTERN_MAP:
      	Mglobal_check_stackptr -= fun->nvars - fun->dim;
      	StackERR;
	break;

      case INTERN_MAP:
      	Mglobal_check_stackptr -= fun->nvars - fun->dim;
      	StackERR;
	for(i=0;i<fun->dim;++i)
	    flag = flag && check_vrpe(fun->vrpe[i],fun->nvars,fun->vars);
	break;
      
      default:
	eprintf("Bad type for function when checking Mvrpe %d\n",
		fun->type);
	break;
      }
    }

    else			/*  c a constant */
    {
      StackERR;
      ++Mglobal_check_stackptr;
      if( c < Mvrpe_const_base || c >= Mvrpe_const_base + Mvrpe_const_max )
      {
        eprintf("check_Mvrpe: bad constant ref, const[%d]\n",c-Mvrpe_const_base);
        flag =  FALSE;
      }
    }    /* end if */

  } while( c != END_RPE );

  if( Mglobal_check_stackptr != initStackPtr + nargs )
  {
	eprintf("check_Mvrpe: stack corupted on exit: initial %d final %d\n",
		initStackPtr,Mglobal_check_stackptr);
	eprint_Mvrpe(Mvrpe,mul);
/*
  	flag = FALSE;
*/
	Mglobal_check_stackptr = initStackPtr;
  }
  if( !flag ) Mglobal_check_stackptr = initStackPtr;
  return( flag );
}

/****  resets constant stack to zero ****/

void clear_Mvrpe_const() {Mvrpe_const_ptr = 0;}

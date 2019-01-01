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
#define PRINT_DIFF_FUN
#define PRINT_EXPANSION
*/
#define SILLYFUNS

#define grballoc(node) (node *) malloc(sizeof(node))
#define MAX(a,b)       a > b ? a : b ;
#define TRUE 1
#define FALSE 0

/********** Subroutines to clean-up and expand equations ******************/

/*
 * Function:	clean_eqn
 * Action:	clean up the equation, remove all instances like
 *		0+eqn, or 1*eqn
 */

int clean_eqn(base)
eqnode *base;
{
  int leftres, rightres,leftcount,rightcount;
  double num;
  eqnode *left,*right,*node;

  if(base == NULL )
  {
	eprintf("Tried to clean a null equation\n");
	return(FALSE);
  }
#ifdef PRINT_EXPANSION
  printf("clean_eqn(");
  print_eqn(base);
  printf(")\n");
#endif

  switch( base->op )
  {
  case NAME:
  case NUMBER:
    return( TRUE );

  case INTERVAL: case ',':
    leftres = clean_eqn( base->u.n.l );
    rightres = clean_eqn( base->u.n.r );
    return( leftres && rightres );

  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN ) return(TRUE);
    return(clean_eqn(base->u.f.a));

  case '+': case '-':
    leftres = clean_eqn( base->u.n.l );
    rightres = clean_eqn( base->u.n.r );

    /* Add numbers together */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = ( base->op == '+' ) ?
            base->u.n.l->u.num + base->u.n.r->u.num :
            base->u.n.l->u.num - base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

    /* Remove zero on left */

    else if( eqnop(base) == '+' && base->u.n.l->op == NUMBER &&
		 base->u.n.l->u.num == 0.0 )
    {
	/* 0.0 + a  -> a */

	free_eqn_node( base->u.n.l );
	right = base->u.n.r;
	copy_node(base,right);
	free(right);
    }

   /* Remove zero on right */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	/* a +/- 0.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* x - (-2)  --> x + 2 ,  x + (-2) --> x - 2 */

    else if( eqnop(eqnr(base)) == NUMBER && eqnval(eqnr(base)) < 0.0 )
    {
	if( eqnop(base) == '+' ) eqnop(base) = '-';
	else			 eqnop(base) = '+';
	eqnval(eqnr(base)) = - eqnval(eqnr(base));
    }

    /***  2 +/- ( 3 +/- x )  -->  (2 +/- 3 ) +/- x ***/

    else if( base->u.n.l->op == NUMBER && 
	    (base->u.n.r->op == '+' || base->u.n.r->op == '-' )
	    && base->u.n.r->u.n.l->op == NUMBER )
    {
	node = base->u.n.r;
	if(eqnop(base) == '+')
		num = base->u.n.l->u.num + node->u.n.l->u.num ;
	else
		num = base->u.n.l->u.num - node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.l->u.num = num;
	if(eqnop(base) == eqnop(eqnr(base)) )
		eqnop(base) = '+';
	else
		eqnop(base) = '-';
	base->u.n.r = node->u.n.r;
	free(node);
	clean_eqn(base);
    }

    /***  2 +/- ( x +/- 3 )  -->  (2+/-3) +/- x ***/

    else if( base->u.n.l->op == NUMBER && 
	    (base->u.n.r->op == '+' || base->u.n.r->op == '-' )
	  && base->u.n.r->u.n.r->op == NUMBER )
    {
	node = base->u.n.r;
	if(eqnop(base) == eqnop(eqnr(base)) )
		num = base->u.n.l->u.num + node->u.n.r->u.num ;
	else
		num = base->u.n.l->u.num - node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.l;
	free(node);
	clean_eqn(base);
    }

    /***  ( 2 +/- x ) +/- 3  -->  (2+/-3) +/- x  ***/

    else if( base->u.n.r->op == NUMBER &&
	    (base->u.n.l->op == '+' || base->u.n.l->op == '-' )
	  && base->u.n.l->u.n.l->op == NUMBER )
    {
	node = base->u.n.l;
	if( eqnop(base) == '+' )
		num = node->u.n.l->u.num + base->u.n.r->u.num ;
	else
		num = node->u.n.l->u.num - base->u.n.r->u.num ;
      	free_eqn_tree( base->u.n.r );
	base->u.n.r = node->u.n.r;
	eqnop(base) = eqnop(eqnl(base));
      	free_eqn_tree( node->u.n.l );
	eqnop(eqnl(base)) = NUMBER;
	eqnval(eqnl(base)) = num;
	clean_eqn(base);
    }

    /***  ( x +/- 2 ) +/- 3 --> x +/- ( 2 +/- 3)  ***/

    else if( base->u.n.r->op == NUMBER &&
	    (base->u.n.l->op == '+' || base->u.n.l->op == '-' )
	  && base->u.n.l->u.n.r->op == NUMBER )
    {
	node = base->u.n.l;
	if( eqnop(eqnl(base)) == eqnop(base) )
		num = node->u.n.r->u.num + base->u.n.r->u.num ;
	else
		num = node->u.n.r->u.num - base->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
	eqnop(base) = eqnop(eqnl(base));
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.l;
	free(node);
	clean_eqn(base);
    }
    return( leftres && rightres );

  case '.':
    leftres = clean_eqn( base->u.n.l );
    rightres = clean_eqn( base->u.n.r );
    return( leftres && rightres );

  case '*':
    leftres = clean_eqn( base->u.n.l );
    rightres = clean_eqn( base->u.n.r );

    /* multiply numbers together */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = base->u.n.l->u.num * base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

    /* Remove identity on left */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 1.0 )
    {
	/* 1.0 * a  ->  a */

	free_eqn_node( base->u.n.l );
	right = base->u.n.r;
	copy_node(base,right);
	free(right);
    }

   /* Remove identity on right */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a * 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* Multiply by zero on left and right */

    else if( ( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    	  || ( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 ) )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

    /***  2 * ( 3 * x )  -->  6 * x ***/

    else if( base->u.n.l->op == NUMBER && base->u.n.r->op == '*'
	  && base->u.n.r->u.n.l->op == NUMBER )
    {
	node = base->u.n.r;
	num = base->u.n.l->u.num * node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.r;
	free(node);
    }

    /***  2 * ( x * 3 )  -->  6 * x ***/

    else if( base->u.n.l->op == NUMBER && base->u.n.r->op == '*'
	  && base->u.n.r->u.n.r->op == NUMBER )
    {
	node = base->u.n.r;
	num = base->u.n.l->u.num * node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.l;
	free(node);
    }

    /***  ( 2 * x ) * 3  -->  x * 6  ***/

    else if( base->u.n.r->op == NUMBER && base->u.n.l->op == '*'
	  && base->u.n.l->u.n.l->op == NUMBER )
    {
	node = base->u.n.l;
	num = base->u.n.r->u.num * node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.r;
	free(node);
    }

    /***  ( x * 2 ) * 3 --> x * 6 ***/

    else if( base->u.n.r->op == NUMBER && base->u.n.l->op == '*'
	  && base->u.n.l->u.n.r->op == NUMBER )
    {
	node = base->u.n.l;
	num = base->u.n.r->u.num * node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.l;
	free(node);
    }

    return( leftres && rightres );

  case '/':
    leftres = clean_eqn( base->u.n.l );
    rightres = clean_eqn( base->u.n.r );

    /* devide numbers */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = base->u.n.l->u.num / base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

   /* Remove division by one */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a / 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* Multiply by zero */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

#ifdef DEVIDE_ZERO
    /* division by zero  */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 1.0 / 0.0;
    }
#endif

    return( leftres && rightres );

  case '^':
    leftres = clean_eqn( base->u.n.l );
    rightres = clean_eqn( base->u.n.r );
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount != rightcount)
    {
	eprintf("Different counts for '^' while cleaning up equation %d %d\n",leftcount,rightcount);
	return(FALSE);
    }
    if( leftcount == 3 ) return(leftres && rightres );
    

    /* evaluate numbers */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = pow(base->u.n.l->u.num, base->u.n.r->u.num);
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

   /* Remove raising to one */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a ^ 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* raising zero */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

    /* raise to power zero  */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 1.0;
    }
    return( leftres && rightres );

  case '=':
    leftres = clean_eqn( base->u.n.l );
    rightres = clean_eqn( base->u.n.r );
    return( leftres && rightres );

  default:
	eprintf("clean_eqn: bad op ");
        eprint_op(base->op);
        eprintf("\n");
	break;
  }
  return( FALSE );
}

/*
 * Function:	eval_funs
 * Action:	clean up the equation, remove all instances like
 *		0+eqn, or 1*eqn
 *		also evaluate cos(0) etc.
 */

int eval_funs(base)
eqnode *base;
{
  int leftres, rightres;
  double num;
  eqnode *left,*right,*node;
  eqn_funs *fun;
  double fun_vars[5], fun_val;
  int i;

  if(base == NULL )
  {
	eprintf("Tried to evaluate a null equation\n");
	return(FALSE);
  }
#ifdef PRINT_EXPANSION
  printf("eval_funs(");
  print_eqn(base);
  printf(")\n");
#endif
  switch( base->op )
  {
  case NAME:
  case NUMBER:
    return( TRUE );

  case INTERVAL: case ',':
    leftres = eval_funs( base->u.n.l );
    rightres = eval_funs( base->u.n.r );
    return( leftres && rightres );

  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN)
    {
	num = base->u.f.f->val;
	base->op = NUMBER;
	base->u.num = num;
	return(TRUE);
    }
    else if(base->u.f.f->type == OPERATOR || base->u.f.f->type == SUM_FUN )
    {
    	rightres = eval_funs( base->u.f.a );
	return(rightres);
    }
    rightres = eval_funs( base->u.f.a );
    fun = base->u.f.f;
    node = base->u.f.a;
    for(i=0;i<fun->nvars;++i)
    {
	if(node->op == ',')
	{
		if(node->u.n.l->op == NUMBER )
		{
			fun_vars[i] = node->u.n.l->u.num;
			node = node->u.n.r;
		}
		else return(FALSE);
	}
	else if( node->op == NUMBER )
	{
		fun_vars[i] = node->u.num;
	}
	else return(FALSE);
    }
    if( fun->type == EXTERN_FUN )
    {
	switch(fun->nvars)
	{
		case 1: fun_val = (*fun->fun)(fun_vars[0]); break;
		case 2: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1]);break;
		case 3: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1],
			fun_vars[2]); break;
		case 4: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1],
			fun_vars[2],fun_vars[3]); break;
		case 5: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1],
			fun_vars[2],fun_vars[3],fun_vars[4]); break;
	}
    }
    else if( fun->type == INTERN_FUN)
	fun_val = eval_rpe(fun->rpe,fun_vars);
    else
	eprintf("Bad type of function %d\n",fun->type);

    free_eqn_tree(base->u.f.a);
    base->op = NUMBER;
    base->u.num = fun_val;
    return(rightres);

  case BRACKET:
    return(eval_funs(base->u.n.r));

  case '+': case '-':
    leftres = eval_funs( base->u.n.l );
    rightres = eval_funs( base->u.n.r );

    /* Add numbers together */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = ( base->op == '+' ) ?
            base->u.n.l->u.num + base->u.n.r->u.num :
            base->u.n.l->u.num - base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

    /* Remove zero on left */

    else if( eqnop(base) == '+' &&
	 base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	/* 0.0 + a  -> a */

	free_eqn_node( base->u.n.l );
	right = base->u.n.r;
	copy_node(base,right);
	free(right);
    }

   /* Remove zero on right */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	/* a +/- 0.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* x - (-2)  --> x + 2 ,  x + (-2) --> x - 2 */

    else if( eqnop(eqnr(base)) == NUMBER && eqnval(eqnr(base)) < 0.0 )
    {
	if( eqnop(base) == '+' ) eqnop(base) = '-';
	else			 eqnop(base) = '+';
	eqnval(eqnr(base)) = - eqnval(eqnr(base));
    }

    /***  2 +/- ( 3 +/- x )  -->  (2 +/- 3 ) +/- x ***/

    else if( base->u.n.l->op == NUMBER && 
	    (base->u.n.r->op == '+' || base->u.n.r->op == '-' )
	    && base->u.n.r->u.n.l->op == NUMBER )
    {
	node = base->u.n.r;
	if(eqnop(base) == '+')
		num = base->u.n.l->u.num + node->u.n.l->u.num ;
	else
		num = base->u.n.l->u.num - node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.l->u.num = num;
	if(eqnop(base) == eqnop(eqnr(base)) )
		eqnop(base) = '+';
	else
		eqnop(base) = '-';
	base->u.n.r = node->u.n.r;
	free(node);
	eval_funs(base);
    }

    /***  2 +/- ( x +/- 3 )  -->  (2+/-3) +/- x ***/

    else if( base->u.n.l->op == NUMBER && 
	    (base->u.n.r->op == '+' || base->u.n.r->op == '-' )
	  && base->u.n.r->u.n.r->op == NUMBER )
    {
	node = base->u.n.r;
	if(eqnop(base) == eqnop(eqnr(base)) )
		num = base->u.n.l->u.num + node->u.n.r->u.num ;
	else
		num = base->u.n.l->u.num - node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.l;
	free(node);
	eval_funs(base);
    }

    /***  ( 2 +/- x ) +/- 3  -->  (2+/-3) +/- x  ***/

    else if( base->u.n.r->op == NUMBER &&
	    (base->u.n.l->op == '+' || base->u.n.l->op == '-' )
	  && base->u.n.l->u.n.l->op == NUMBER )
    {
	node = base->u.n.l;
	if( eqnop(base) == '+' )
		num = node->u.n.l->u.num + base->u.n.r->u.num ;
	else
		num = node->u.n.l->u.num - base->u.n.r->u.num ;
      	free_eqn_tree( base->u.n.r );
	base->u.n.r = node->u.n.r;
	eqnop(base) = eqnop(eqnl(base));
      	free_eqn_tree( node->u.n.l );
	eqnop(eqnl(base)) = NUMBER;
	eqnval(eqnl(base)) = num;
	eval_funs(base);
    }

    /***  ( x +/- 2 ) +/- 3 --> x +/- ( 2 +/- 3)  ***/

    else if( base->u.n.r->op == NUMBER &&
	    (base->u.n.l->op == '+' || base->u.n.l->op == '-' )
	  && base->u.n.l->u.n.r->op == NUMBER )
    {
	node = base->u.n.l;
	if( eqnop(eqnl(base)) == eqnop(base) )
		num = node->u.n.r->u.num + base->u.n.r->u.num ;
	else
		num = node->u.n.r->u.num - base->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
	eqnop(base) = eqnop(eqnl(base));
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.l;
	free(node);
	eval_funs(base);
    }

    return( leftres && rightres );

  case '.':
    leftres = eval_funs( base->u.n.l );
    rightres = eval_funs( base->u.n.r );
    return( leftres && rightres );

  case '*':
    leftres = eval_funs( base->u.n.l );
    rightres = eval_funs( base->u.n.r );

    /* multiply numbers together */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = base->u.n.l->u.num * base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

    /* Remove identity on left */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 1.0 )
    {
	/* 1.0 * a  ->  a */

	free_eqn_node( base->u.n.l );
	right = base->u.n.r;
	copy_node(base,right);
	free(right);
    }

   /* Remove identity on right */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a * 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* Multiply by zero on left and right */

    else if( ( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    	  || ( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 ) )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

    /***  2 * ( 3 * x )  -->  6 * x ***/

    else if( base->u.n.l->op == NUMBER && base->u.n.r->op == '*'
	  && base->u.n.r->u.n.l->op == NUMBER )
    {
	node = base->u.n.r;
	num = base->u.n.l->u.num * node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.r;
	free(node);
    }

    /***  2 * ( x * 3 )  -->  6 * x ***/

    else if( base->u.n.l->op == NUMBER && base->u.n.r->op == '*'
	  && base->u.n.r->u.n.r->op == NUMBER )
    {
	node = base->u.n.r;
	num = base->u.n.l->u.num * node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.l;
	free(node);
    }

    /***  ( 2 * x ) * 3  -->  x * 6  ***/

    else if( base->u.n.r->op == NUMBER && base->u.n.l->op == '*'
	  && base->u.n.l->u.n.l->op == NUMBER )
    {
	node = base->u.n.l;
	num = base->u.n.r->u.num * node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.r;
	free(node);
    }

    /***  ( x * 2 ) * 3 --> x * 6 ***/

    else if( base->u.n.r->op == NUMBER && base->u.n.l->op == '*'
	  && base->u.n.l->u.n.r->op == NUMBER )
    {
	node = base->u.n.l;
	num = base->u.n.r->u.num * node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.l;
	free(node);
    }

    /*** if(x,1,0) * y --> if(x,y,0) 
         if(x,0,1) * y --> if(x,0,y)
	needed to cope with the diff(if(x,sqrt(x),sqrt(-x))) bug ***/

    else if( base->u.n.l->op == FUNCTION && 
	     !strcmp(base->u.n.l->u.f.f->name,"if") )
    {
	if(
	   base->u.n.l->u.f.a->op == ',' && 
	   base->u.n.l->u.f.a->u.n.r->op == ',' &&

	   base->u.n.l->u.f.a->u.n.r->op == ',' && 
	   base->u.n.l->u.f.a->u.n.r->u.n.l->op == NUMBER &&
	   base->u.n.l->u.f.a->u.n.r->u.n.l->u.num == 1.0 &&
	   base->u.n.l->u.f.a->u.n.r->u.n.r->op == NUMBER &&
	   base->u.n.l->u.f.a->u.n.r->u.n.r->u.num == 0.0 )
	{
		base->u.n.l->u.f.a->u.n.r->u.n.l = base->u.n.r;
		copy_node(base,base->u.n.l);
	}
	else if(
	   base->u.n.l->u.f.a->op == ',' && 
	   base->u.n.l->u.f.a->u.n.r->op == ',' &&

	   base->u.n.l->u.f.a->u.n.r->op == ',' && 
	   base->u.n.l->u.f.a->u.n.r->u.n.l->op == NUMBER &&
	   base->u.n.l->u.f.a->u.n.r->u.n.l->u.num == 0.0 &&
	   base->u.n.l->u.f.a->u.n.r->u.n.r->op == NUMBER &&
	   base->u.n.l->u.f.a->u.n.r->u.n.r->u.num == 1.0 )
	{
		base->u.n.l->u.f.a->u.n.r->u.n.r = base->u.n.r;
		copy_node(base,base->u.n.l);
	}
    }
    

    return( leftres && rightres );

  case '/':
    leftres = eval_funs( base->u.n.l );
    rightres = eval_funs( base->u.n.r );

    /* devide numbers */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = base->u.n.l->u.num / base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

   /* Remove division by one */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a / 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* Multiply by zero */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

#ifdef DEVIDE_ZERO
    /* division by zero  */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 1.0 / 0.0;
    }
#endif

    return( leftres && rightres );

  case '^':
    leftres = eval_funs( base->u.n.l );
    rightres = eval_funs( base->u.n.r );
#ifdef NOT_DEF
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount != rightcount)
    {
	eprintf("Different counts for '^' while evaluating functions %d %d\n",leftcount,rightcount);
	eprint_eqn(base);
	eprintf("\n");
	return(FALSE);
    }
    if( leftcount == 3 ) return(leftres && rightres );
#endif
    
    /* evaluate numbers */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = pow(base->u.n.l->u.num, base->u.n.r->u.num);
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

   /* Remove raising to one */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a ^ 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* raising zero */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

    /* raise to power zero  */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 1.0;
    }
    return( leftres && rightres );
#ifdef NOT_DEF
#endif

  case '=':
    leftres = eval_funs( base->u.n.l );
    rightres = eval_funs( base->u.n.r );
    return( leftres && rightres );

  default:
	eprintf("eval_funs: bad op ");
        eprint_op(base->op);
        eprintf("\n");
	break;
  }
  return( FALSE );
}

/*
 * Function:	eval_ops
 * Action:	clean up the equation, remove all instances like
 *		0+eqn, or 1*eqn
 *		also evaluate cos(0) etc.
 *		and evaluate operators diff(x^2,x)
 */

int eval_ops(base)
eqnode *base;
{
  int leftres, rightres;
  double num;
  eqnode *left,*right,*node;
  eqn_funs *fun;
  double fun_vars[5], fun_val;
  int i;

  if(base == NULL )
  {
	eprintf("Tried to evaluate a null equation\n");
	return(FALSE);
  }
#ifdef PRINT_EXPANSION
  printf("eval_ops(");
  print_eqn(base);
  printf(")\n");
#endif
  switch( base->op )
  {
  case NAME:
  case NUMBER:
    return( TRUE );

  case INTERVAL: case ',':
    leftres = eval_ops( base->u.n.l );
    rightres = eval_ops( base->u.n.r );
    return( leftres && rightres );

  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN)
    {
	num = base->u.f.f->val;
	base->op = NUMBER;
	base->u.num = num;
	return(TRUE);
    }
    else if(base->u.f.f->type == OPERATOR )
    {
    	rightres = eval_ops( base->u.f.a );
	node = base->u.f.f->op(base->u.f.a);
	copy_node(base,node);
	free_eqn_node(node);
	rightres = eval_ops( base );
	return(rightres);
    }
    else if(base->u.f.f->type == SUM_FUN )
    {
    	rightres = eval_ops( base->u.f.a );
	return(rightres);
    }

    rightres = eval_ops( base->u.f.a );
    fun = base->u.f.f;
    node = base->u.f.a;
    for(i=0;i<fun->nvars;++i)
    {
	if(node->op == ',')
	{
		if(node->u.n.l->op == NUMBER )
		{
			fun_vars[i] = node->u.n.l->u.num;
			node = node->u.n.r;
		}
		else return(FALSE);
	}
	else if( node->op == NUMBER )
	{
		fun_vars[i] = node->u.num;
	}
	else return(FALSE);
    }
    if( fun->type == EXTERN_FUN )
    {
	switch(fun->nvars)
	{
		case 1: fun_val = (*fun->fun)(fun_vars[0]); break;
		case 2: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1]);break;
		case 3: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1],
			fun_vars[2]); break;
		case 4: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1],
			fun_vars[2],fun_vars[3]); break;
		case 5: fun_val = (*fun->fun)(fun_vars[0],fun_vars[1],
			fun_vars[2],fun_vars[3],fun_vars[4]); break;
	}
    }
    else if( fun->type == INTERN_FUN)
	fun_val = eval_rpe(fun->rpe,fun_vars);
    else
	eprintf("Bad type of function %d\n",fun->type);

    free_eqn_tree(base->u.f.a);
    base->op = NUMBER;
    base->u.num = fun_val;
    return(rightres);

  case BRACKET:
    return(eval_ops(base->u.n.r));

  case '+': case '-':
    leftres = eval_ops( base->u.n.l );
    rightres = eval_ops( base->u.n.r );

    /* Add numbers together */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = ( base->op == '+' ) ?
            base->u.n.l->u.num + base->u.n.r->u.num :
            base->u.n.l->u.num - base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

    /* Remove zero on left */

    else if( eqnop(base) == '+' &&
	 base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	/* 0.0 + a  -> a */

	free_eqn_node( base->u.n.l );
	right = base->u.n.r;
	copy_node(base,right);
	free(right);
    }

   /* Remove zero on right */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	/* a +/- 0.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* x - (-2)  --> x + 2 ,  x + (-2) --> x - 2 */

    else if( eqnop(eqnr(base)) == NUMBER && eqnval(eqnr(base)) < 0.0 )
    {
	if( eqnop(base) == '+' ) eqnop(base) = '-';
	else			 eqnop(base) = '+';
	eqnval(eqnr(base)) = - eqnval(eqnr(base));
    }

    /***  2 +/- ( 3 +/- x )  -->  (2 +/- 3 ) +/- x ***/

    else if( base->u.n.l->op == NUMBER && 
	    (base->u.n.r->op == '+' || base->u.n.r->op == '-' )
	    && base->u.n.r->u.n.l->op == NUMBER )
    {
	node = base->u.n.r;
	if(eqnop(base) == '+')
		num = base->u.n.l->u.num + node->u.n.l->u.num ;
	else
		num = base->u.n.l->u.num - node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.l->u.num = num;
	if(eqnop(base) == eqnop(eqnr(base)) )
		eqnop(base) = '+';
	else
		eqnop(base) = '-';
	base->u.n.r = node->u.n.r;
	free(node);
	eval_ops(base);
    }

    /***  2 +/- ( x +/- 3 )  -->  (2+/-3) +/- x ***/

    else if( base->u.n.l->op == NUMBER && 
	    (base->u.n.r->op == '+' || base->u.n.r->op == '-' )
	  && base->u.n.r->u.n.r->op == NUMBER )
    {
	node = base->u.n.r;
	if(eqnop(base) == eqnop(eqnr(base)) )
		num = base->u.n.l->u.num + node->u.n.r->u.num ;
	else
		num = base->u.n.l->u.num - node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.l;
	free(node);
	eval_ops(base);
    }

    /***  ( 2 +/- x ) +/- 3  -->  (2+/-3) +/- x  ***/

    else if( base->u.n.r->op == NUMBER &&
	    (base->u.n.l->op == '+' || base->u.n.l->op == '-' )
	  && base->u.n.l->u.n.l->op == NUMBER )
    {
	node = base->u.n.l;
	if( eqnop(base) == '+' )
		num = node->u.n.l->u.num + base->u.n.r->u.num ;
	else
		num = node->u.n.l->u.num - base->u.n.r->u.num ;
      	free_eqn_tree( base->u.n.r );
	base->u.n.r = node->u.n.r;
	eqnop(base) = eqnop(eqnl(base));
      	free_eqn_tree( node->u.n.l );
	eqnop(eqnl(base)) = NUMBER;
	eqnval(eqnl(base)) = num;
	eval_ops(base);
    }

    /***  ( x +/- 2 ) +/- 3 --> x +/- ( 2 +/- 3)  ***/

    else if( base->u.n.r->op == NUMBER &&
	    (base->u.n.l->op == '+' || base->u.n.l->op == '-' )
	  && base->u.n.l->u.n.r->op == NUMBER )
    {
	node = base->u.n.l;
	if( eqnop(eqnl(base)) == eqnop(base) )
		num = node->u.n.r->u.num + base->u.n.r->u.num ;
	else
		num = node->u.n.r->u.num - base->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
	eqnop(base) = eqnop(eqnl(base));
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.l;
	free(node);
	eval_ops(base);
    }

    return( leftres && rightres );

  case '.':
    leftres = eval_ops( base->u.n.l );
    rightres = eval_ops( base->u.n.r );
    return( leftres && rightres );

  case '*':
    leftres = eval_ops( base->u.n.l );
    rightres = eval_ops( base->u.n.r );

    /* multiply numbers together */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = base->u.n.l->u.num * base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

    /* Remove identity on left */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 1.0 )
    {
	/* 1.0 * a  ->  a */

	free_eqn_node( base->u.n.l );
	right = base->u.n.r;
	copy_node(base,right);
	free(right);
    }

   /* Remove identity on right */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a * 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* Multiply by zero on left and right */

    else if( ( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    	  || ( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 ) )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

    /***  2 * ( 3 * x )  -->  6 * x ***/

    else if( base->u.n.l->op == NUMBER && base->u.n.r->op == '*'
	  && base->u.n.r->u.n.l->op == NUMBER )
    {
	node = base->u.n.r;
	num = base->u.n.l->u.num * node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.r;
	free(node);
    }

    /***  2 * ( x * 3 )  -->  6 * x ***/

    else if( base->u.n.l->op == NUMBER && base->u.n.r->op == '*'
	  && base->u.n.r->u.n.r->op == NUMBER )
    {
	node = base->u.n.r;
	num = base->u.n.l->u.num * node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.l->u.num = num;
	base->u.n.r = node->u.n.l;
	free(node);
    }

    /***  ( 2 * x ) * 3  -->  x * 6  ***/

    else if( base->u.n.r->op == NUMBER && base->u.n.l->op == '*'
	  && base->u.n.l->u.n.l->op == NUMBER )
    {
	node = base->u.n.l;
	num = base->u.n.r->u.num * node->u.n.l->u.num ;
      	free_eqn_tree( node->u.n.l );
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.r;
	free(node);
    }

    /***  ( x * 2 ) * 3 --> x * 6 ***/

    else if( base->u.n.r->op == NUMBER && base->u.n.l->op == '*'
	  && base->u.n.l->u.n.r->op == NUMBER )
    {
	node = base->u.n.l;
	num = base->u.n.r->u.num * node->u.n.r->u.num ;
      	free_eqn_tree( node->u.n.r );
      	base->u.n.r->u.num = num;
	base->u.n.l = node->u.n.l;
	free(node);
    }

    return( leftres && rightres );

  case '/':
    leftres = eval_ops( base->u.n.l );
    rightres = eval_ops( base->u.n.r );

    /* devide numbers */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = base->u.n.l->u.num / base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

   /* Remove division by one */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a / 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* Multiply by zero */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

#ifdef DEVIDE_ZERO
    /* division by zero  */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 1.0 / 0.0;
    }
#endif

    return( leftres && rightres );

  case '^':
    leftres = eval_ops( base->u.n.l );
    rightres = eval_ops( base->u.n.r );
#ifdef NOT_DEF
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount != rightcount)
    {
	eprintf("Different counts for '^' while evaluating functions %d %d\n",leftcount,rightcount);
	return(FALSE);
    }
    if( leftcount == 3 ) return(leftres && rightres );
#endif
    /* evaluate numbers */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = pow(base->u.n.l->u.num, base->u.n.r->u.num);
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

   /* Remove raising to one */
#ifdef NOT_DEF
    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
    {
	/* a ^ 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
    }

    /* raising zero */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 0.0;
    }

    /* raise to power zero  */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = 1.0;
    }
#endif
    return( leftres && rightres );

  case '=':
    leftres = eval_ops( base->u.n.l );
    rightres = eval_ops( base->u.n.r );
    return( leftres && rightres );

  default:
	eprintf("eval_ops: bad op ");
        eprint_op(base->op);
        eprintf("\n");
	break;
  }
  return( FALSE );
}

/*****************************************************************************/
/*                                                                           */
/*  Now some routines to expand an equation.                                 */
/*    nodes are multiplied out by 'multiply_out'                             */
/*    nodes raised to a power are expanded by 'raise_out'                    */
/*    some simplification also occurs e.g.  x^0 = 0                          */
/*    'expand' manages the calls to these routines, does some simplification */
/*    and returns TRUE if an expanded form was arrived at.                   */
/*    a return value of FALSE indicates that an expansion was not posible    */
/*    for example 'x^a' where a is a variable or 'x/y'.                       */
/*                                                                           */
/*****************************************************************************/

int expand( base )
eqnode *base;
{
  int leftres, rightres,leftcount,rightcount;
  double num;
  eqnode *left,*right,*temp;

  if(base == NULL )
  {
	eprintf("Tried to expand null equation\n");
	return(FALSE);
  }
#ifdef PRINT_EXPANSION
  printf("expand(");
  print_eqn(base);
  printf(")\n");
#endif
  switch( base->op )
  {
  case NAME:
  case NUMBER:
    return( TRUE );

/*
  case BRACKET:
    right = base->u.n.r;
    rightres = expand( right );
    copy_node(base,right);
    free(right);
    return( leftres );
*/

  case INTERVAL: case ',':
    leftres = expand( base->u.n.l );
    rightres = expand( base->u.n.r );
    return( leftres && rightres );

  case FUNCTION:
    if( base->u.f.f->type == CONSTANT_FUN ) return(TRUE);
    return(expand(base->u.f.a));

  case '+': case '-':
    leftres = expand( base->u.n.l );
    rightres = expand( base->u.n.r );
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount != rightcount)
    {
	eprintf("Different counts for '+' while expanding equation %d %d\n",leftcount,rightcount);
	return(FALSE);
    }
    if(leftcount > 1 )	/* Vector Addition */
    {
	for(;leftcount>0;--leftcount)
	{
		if(leftcount == rightcount ) /* first pass */
			temp =
			  join_dup_eqns(base->op,
					get_eqn_arg(base->u.n.l,leftcount),
					get_eqn_arg(base->u.n.r,leftcount));
		else	/* other passes */
			temp =
			 join_eqns(',',
			  join_dup_eqns(base->op,
					get_eqn_arg(base->u.n.l,leftcount),
					get_eqn_arg(base->u.n.r,leftcount)),
			  temp);
	}
	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	copy_node(base,temp);
	return(expand(base));
    }


    /* Add numbers together */

    if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
    {
      num = ( base->op == '+' ) ?
            base->u.n.l->u.num + base->u.n.r->u.num :
            base->u.n.l->u.num - base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }

    /* Remove zero on left */

    else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
    {
	/* 0.0 +/- a  -> +/- a */

	if( base->op == '+' )
	{
		free_eqn_node( base->u.n.l );
		right = base->u.n.r;
		copy_node(base,right);
		free(right);
	}
    }

   /* Remove zero on right */

    else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
    {
	/* a +/- 0.0 -> a */

		free_eqn_node( base->u.n.r );
		left = base->u.n.l;
		copy_node(base,left);
		free(left);
    }

    return( leftres && rightres );

  case '*':
    leftres = expand( base->u.n.l );
    rightres = expand( base->u.n.r );
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount  > 1 && rightcount == 1)	/* Scaler Product */
    {
	temp = NULL;
	for(;leftcount>0;--leftcount)
	{
		if(temp != NULL)
			temp =
			 join_eqns(',',
			  join_dup_eqns('*',get_eqn_arg(base->u.n.l,leftcount),
					    base->u.n.r),
			  temp);
		else
			temp =
			  join_dup_eqns('*',get_eqn_arg(base->u.n.l,leftcount),
					   base->u.n.r);
	}
	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	copy_node(base,temp);
	return(expand(base));
    }
    else if( leftcount == 1 && rightcount > 1)
    {
	temp = NULL;
	for(;rightcount>0;--rightcount)
	{
		if(temp != NULL)
			temp =
			 join_eqns(',',
			  join_dup_eqns('*',base->u.n.l,
					   get_eqn_arg(base->u.n.r,rightcount)),
			  temp);
		else
			temp =
			  join_dup_eqns('*',base->u.n.l,
					   get_eqn_arg(base->u.n.r,rightcount));
	}
	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	copy_node(base,temp);
	return(expand(base));
    }
    else
	return( multiply_out( base ) && leftres && rightres );

  case '/':
    leftres = expand( base->u.n.l );
    rightres = expand( base->u.n.r );
    return( devide_out( base ) && leftres && rightres );

  case '^':
    leftres = expand( base->u.n.l );
    rightres = expand( base->u.n.r );
    leftcount = count_eqn_args(base->u.n.l);
    rightcount = count_eqn_args(base->u.n.r);
    if(leftcount != rightcount)
    {
	eprintf("Different counts for '^' while expanding equation %d %d\n",leftcount,rightcount);
	return(FALSE);
    }
    if(leftcount  == 3 )	/* Cross Product */
    {
	temp =
	  join_eqns(',',
	    join_eqns('-',
	     join_dup_eqns('*',get_eqn_arg(base->u.n.l,2),get_eqn_arg(base->u.n.r,3)),
	     join_dup_eqns('*',get_eqn_arg(base->u.n.l,3),get_eqn_arg(base->u.n.r,2))),
	   join_eqns(',',
	    join_eqns('-',
	     join_dup_eqns('*',get_eqn_arg(base->u.n.l,3),get_eqn_arg(base->u.n.r,1)),
	     join_dup_eqns('*',get_eqn_arg(base->u.n.l,1),get_eqn_arg(base->u.n.r,3))),
	    join_eqns('-',
	     join_dup_eqns('*',get_eqn_arg(base->u.n.l,1),get_eqn_arg(base->u.n.r,2)),
	     join_dup_eqns('*',get_eqn_arg(base->u.n.l,2),get_eqn_arg(base->u.n.r,1)))));

	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	copy_node(base,temp);
	return(expand(base));
    }
    else if(leftcount == 2)
    {
	temp =
	    join_eqns('-',
	     join_dup_eqns('*',
		get_eqn_arg(base->u.n.l,1),
		get_eqn_arg(base->u.n.r,2)),
	     join_dup_eqns('*',
		get_eqn_arg(base->u.n.l,2),
		get_eqn_arg(base->u.n.r,1)));

	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	copy_node(base,temp);
	return(expand(base));
    }
    else if( leftcount == 1)
	return( raise_out( base )  && leftres && rightres );

    eprintf("Bad number of arguments for '^' while trying to expand the equation %d\n",leftcount);
    return(FALSE);

  case '=':
    leftres = expand( base->u.n.l );
    rightres = expand( base->u.n.r );
    return( leftres && rightres );

  case '.':
	leftres = expand( base->u.n.l );
	rightres = expand( base->u.n.r );
	if( !leftres || ! rightres )
	{
		eprintf("Failed to expand lhs/rhs while expanding '.' %d %d\n",leftres,rightres);
		return(FALSE);
	}
	leftcount = count_eqn_args(base->u.n.l);
	rightcount = count_eqn_args(base->u.n.r);
	if(leftcount != rightcount )
	{
		eprintf("Bad argument counts while expanding '.' %d %d\n",leftcount,rightcount);
		return(FALSE);
	}
	if(leftcount == 0)
	{
		eprintf("Trying to expand a . with zero args on each side!");
		return(FALSE);
	}

	for(;leftcount>0;--leftcount)
	{
		if(leftcount != rightcount )
			temp =
			 join_eqns('+',
			  join_dup_eqns('*',get_eqn_arg(base->u.n.l,leftcount),
					    get_eqn_arg(base->u.n.r,leftcount)),
			  temp);
		else
			temp =
			  join_dup_eqns('*',get_eqn_arg(base->u.n.l,leftcount),
					   get_eqn_arg(base->u.n.r,leftcount));
	}
	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	copy_node(base,temp);
	return(expand(base) && leftres && rightres);

  default:
	eprintf("Bad op ");
	eprint_op(base->op);
	eprintf(" while trying to expand equation\n");
  }
  return( FALSE );
}

/**** multiply out an equation of type 'x*y' ****/

int multiply_out( base )
eqnode *base;
{
  double num;
  eqnode *temp,*left,*right;
  int	lres,rres;

  if(base == NULL )
  {
	eprintf("Tried to multiply out a null equation\n");
	return(FALSE);
  }

#ifdef PRINT_EXPANSION
  printf("multiply_out(");
  print_eqn(base);
  printf(")\n");
#endif
  if( base->u.n.l->op == '+' || base->u.n.l->op == '-' )
  {                /* (x+y)*z  --> (x*z) + (y*z) */
      temp = grballoc( eqnode );
      temp->op =  '*';
      base->op = base->u.n.l->op;
      base->u.n.l->op = '*';
      temp->u.n.r = duplicate( base->u.n.r );
      temp->u.n.l = base->u.n.l->u.n.r;
      base->u.n.l->u.n.r = base->u.n.r;
      base->u.n.r = temp;
      lres = multiply_out(base->u.n.l);
      rres = multiply_out(base->u.n.r);
#ifdef SORT_ADD
      return( sort_add(base) );
#else
      return( lres && rres );
#endif
  }

  else if( base->u.n.r->op == '+' || base->u.n.r->op == '-' )
  {                 /*  x*(y+z) --> (x*y) + (x*z) */
      temp = grballoc( eqnode );
      temp->op =  '*';
      base->op = base->u.n.r->op;
      base->u.n.r->op = '*';
      temp->u.n.l = duplicate( base->u.n.l );
      temp->u.n.r = base->u.n.r->u.n.l;
      base->u.n.r->u.n.l = base->u.n.l;
      base->u.n.l = temp;
      lres = multiply_out(base->u.n.l);
      rres = multiply_out(base->u.n.r);
#ifdef SORT_ADD
      return( sort_add(base) );
#else
      return( lres && rres );
#endif
  }

  else if( base->u.n.l->op == NUMBER && base->u.n.r->op == NUMBER )
  {
	num = base->u.n.l->u.num * base->u.n.r->u.num;
	free_eqn_tree( base->u.n.l );
	free_eqn_tree( base->u.n.r );
	base->op = NUMBER;
	base->u.num = num;
	return(TRUE);
  }

  else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 0.0 )
  {
	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	base->op = NUMBER;
	base->u.num = 0.0;
	return(TRUE);
  }

  else if( base->u.n.l->op == NUMBER && base->u.n.l->u.num == 1.0 )
  {
	/* a * 1.0 -> a */

	free_eqn_node( base->u.n.l );
	right = base->u.n.r;
	copy_node(base,right);
	free(right);
	return(TRUE);
  }
  else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 0.0 )
  {
	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	base->op = NUMBER;
	base->u.num = 0.0;
	return(TRUE);
  }
  else if( base->u.n.r->op == NUMBER && base->u.n.r->u.num == 1.0 )
  {
	/* a * 1.0 -> a */

	free_eqn_node( base->u.n.r );
	left = base->u.n.l;
	copy_node(base,left);
	free(left);
	return(TRUE);
   }
   else if( base->u.n.l->op == ',' && base->u.n.r->op == ',' 
	 && count_eqn_args(base->u.n.l) == 2 
	 && count_eqn_args(base->u.n.r) == 2 )
   {
	/* complex multiplication (x,y)*(z,w) --> ((x*z-y*w),(x*w+y*z)) */
	eqnode *x,*y,*z,*w;
	x = base->u.n.l->u.n.l;
	y = base->u.n.l->u.n.r;
	z = base->u.n.r->u.n.l;
	w = base->u.n.r->u.n.r;
	
	temp = join_eqns(',',
		join_eqns('-',
		 join_dup_eqns('*',x,z),
		 join_dup_eqns('*',y,w)
		),
		join_eqns('+',
		 join_dup_eqns('*',x,w),
		 join_dup_eqns('*',y,z)
		)
	       );
	free_eqn_tree(base->u.n.l);
	free_eqn_tree(base->u.n.r);
	copy_node(base,temp);
	return(TRUE);
   }

   else if( (base->u.n.l->op != NAME && base->u.n.l->op != NUMBER
	  && base->u.n.l->op != '*')
          ||(base->u.n.r->op != NAME && base->u.n.r->op != NUMBER
	  && base->u.n.r->op != '*') )
   {
#ifdef TO_POLY_ERR
	eprintf("multiply_out: couldn't expand\n");
	eprint_eqn(base);
	eprintf("\n");
#endif
	return(FALSE);
   }

#ifdef SORT_ADD
  else return( sort_multiply(base) );
#endif
  return(TRUE);
}

/**** devide out an equation of type 'x/y' ****/

int devide_out( base )
eqnode *base;
{
  double num;

  if(base == NULL )
  {
	eprintf("Tried to devide out a null equation\n");
	return(FALSE);
  }

#ifdef PRINT_EXPANSION
  printf("devide_out(");
  print_eqn(base);
  printf(")\n");
#endif
  switch( base->u.n.r->op )
  {
  case NUMBER:
    if( base->u.n.l->op == NUMBER )
    {
      num = base->u.n.l->u.num / base->u.n.r->u.num ;
      free_eqn_tree( base->u.n.l );
      free_eqn_tree( base->u.n.r );
      base->op = NUMBER;
      base->u.num = num;
      return( TRUE );
    }
    base->op = '*';
    base->u.n.r->u.num = 1.0 / base->u.n.r->u.num ;
    return( multiply_out(base) );

  default:
#ifdef TO_POLY_ERR
    eprintf("devide_out: don't know how to devide by non number op ");
        eprint_op(base->u.n.r->op);
        eprintf("\n");
#endif
    return( FALSE );
  }
}

/**** expand an equation of type 'x^y' ****/

int raise_out( base )
eqnode *base;
{

  int leftres, n, i;
  double num;
  eqnode *temp;

  if(base == NULL )
  {
	eprintf("Tried to raise out a null equation\n");
	return(FALSE);
  }

#ifdef PRINT_EXPANSION
  printf("raise_out(");
  print_eqn(base);
  printf(")\n");
#endif
 
  if( base->u.n.r->op == '+' ) /* x^(a+b) -> x^a * x^b */
  {
	temp = grballoc( eqnode );
	temp->op = '^';
	temp->u.n.l = duplicate(base->u.n.l);
	temp->u.n.r = base->u.n.r->u.n.l;
	base->u.n.r->u.n.l = base->u.n.l;
	base->u.n.l = temp;
	base->u.n.r->op = '^';
	base->op = '*';
	return( expand(base) );
  }

  if( base->u.n.r->op == '-' ) /* x^(a+b) -> x^a * x^-1*b */
  {
        temp = grballoc( eqnode );
        temp->op = '^';
        temp->u.n.l = duplicate(base->u.n.l);
        temp->u.n.r = base->u.n.r->u.n.l;
        base->u.n.r->u.n.l = base->u.n.l;
        base->u.n.l = temp;
	temp = grballoc( eqnode );
	temp->op = '*';
	temp->u.n.r = base->u.n.r->u.n.r;
	temp->u.n.l = grballoc( eqnode );
	temp->u.n.l->op = NUMBER;
	temp->u.n.l->u.num = -1.0;
	base->u.n.r->u.n.r = temp;
        base->u.n.r->op = '^';
        base->op = '*';
        return( expand(base) );
  }



	 
  if( base->u.n.r->op != NUMBER )
  {
#ifdef TO_POLY_ERR
    eprintf("raise_out: can't raise to non numerical power op ");
        eprint_op(base->u.n.r->op);
        eprintf("\n");
#endif
    return( FALSE );
  }

  if( base->u.n.l->op == NUMBER )
  {
    num = pow( base->u.n.l->u.num , base->u.n.r->u.num );
    free_eqn_tree( base->u.n.l );
    free_eqn_tree( base->u.n.r );
    base->op = NUMBER;
    base->u.num = num;
    return( num == num );  /* FALSE if num is NaN */
  }
    
  n = floor( base->u.n.r->u.num );

  if( base->u.n.r->u.num - n > 1.0e-9 )
  {
#ifdef TO_POLY_ERR
    eprintf("raise_out: power is not an integer %f\n",base->u.n.r->u.num);
#endif
    return( FALSE );
  }
  
  if( n < 0 ) 
  {
#ifdef TO_POLY_ERR
    eprintf("raise_out: power is negative %d\n",n);
#endif
    return( FALSE );
  }
  
  if( n == 0 )
  {
#ifdef TO_POLY_ERR
    eprintf("raise_out: warning power is zero\n");  
#endif
    free_eqn_tree( base->u.n.l );
    free_eqn_tree( base->u.n.r );
    base->op = NUMBER;
    base->u.num = 1.0;
    return( TRUE );  /* FALSE if num is NaN */
  }
    
  if( n == 1 )            /*  x^1 -- x */
  {
    temp = base->u.n.l;
    free_eqn_tree( base->u.n.r );
    copy_node(base,temp);
    return( TRUE );
  }

  if( n == 2 )           /* x^2 --> x*x */
  {
    base->op = '*';
    free_eqn_tree( base->u.n.r );
    base->u.n.r = duplicate( base->u.n.l );
    leftres =  expand( base );
    return( leftres );
  }

  /*  n >= 3               x^n --> x * ( x * ( x *.... ) ) )  */

  base->op = '*';
  base->u.n.r->u.n.l = duplicate( base->u.n.l );
  base->u.n.r->u.n.r = duplicate( base->u.n.l );
  base->u.n.r->op = '*';

  for( i=4 ; i<=n ; ++i )   /*  x*x^{i-1} --> x*(x*x^{i-1}) */
  {
    temp = grballoc( eqnode );
    temp->op = '*';
    temp->u.n.r = base->u.n.r;
    temp->u.n.l = duplicate( base->u.n.l );
    base->u.n.r = temp;
  }

  return( expand( base ) );
}

/*
 * Find the common_denominator of eqn
 */

void common_denominator(eqn)
eqnode *eqn;
{
	eqnode *temp;

  if(eqn == NULL )
  {
	eprintf("Tried to find a common denominator of a null equation\n");
	return;
  }
	if( eqn->op == NAME || eqn->op == NUMBER ) return;
	if( eqn->op == FUNCTION )
	{
		common_denominator(eqn->u.f.a);
		return;
	}

	/* must be binary op */

	common_denominator(eqn->u.n.l);
	common_denominator(eqn->u.n.r);
		
	switch(eqn->op)
	{
	case '^':
		if(eqn->u.n.l->op == '/')
		{
			/* (a/c)^b -> (a^b)/(c^b) */

			temp = grballoc(eqnode);
			temp->op = '^';
			temp->u.n.l = eqn->u.n.l->u.n.r;
			temp->u.n.r = duplicate(eqn->u.n.r);
			eqn->u.n.l->u.n.r = eqn->u.n.r;
			eqn->u.n.r = temp;
			eqn->u.n.l->op = '^';
			eqn->u.n.r->op = '^';
			eqn->op = '/';
		}
		break;
	case '+': case '-':
		if(eqn->u.n.l->op == '/')
		{
		    if(eqn->u.n.r->op == '/')
		    {
			/* a/c + b/d -> (a d + b c)/(c d) */

			temp = grballoc(eqnode);
			temp->op = eqn->op;
			temp->u.n.l = eqn->u.n.l;
			temp->u.n.r = eqn->u.n.r; /* (a/c+b/d) */
			eqn->u.n.r = grballoc(eqnode);
			eqn->u.n.r->op = '*';
			eqn->u.n.r->u.n.l = temp->u.n.l->u.n.r;
			eqn->u.n.r->u.n.r = temp->u.n.r->u.n.r;
			temp->u.n.l->u.n.r = duplicate(eqn->u.n.r->u.n.r);
			temp->u.n.r->u.n.r = duplicate(eqn->u.n.r->u.n.l);
			temp->u.n.l->op = '*';
			temp->u.n.r->op = '*';
			eqn->u.n.l = temp;
			eqn->op = '/';
		    }
		    else
		    {
			/* a/c + b -> (a + b c)/c */

			temp = grballoc(eqnode);
			temp->op = eqn->op;
			temp->u.n.l = eqn->u.n.l->u.n.l; /* a */
			temp->u.n.r = eqn->u.n.l;  /* a/c */
			temp->u.n.r->u.n.l = eqn->u.n.r; /* b */
			temp->u.n.r->op = '*'; /* b * c */
			eqn->u.n.r = duplicate(temp->u.n.r->u.n.r);
			eqn->u.n.l = temp;
			eqn->op = '/';
		    }
		}
		else
		{
		    if(eqn->u.n.r->op == '/')
		    {
			/* a + b/c -> (a c + b)/c */

			temp = grballoc(eqnode);
			temp->op = eqn->op;
			temp->u.n.r = eqn->u.n.r->u.n.l; /* b */
			temp->u.n.l = eqn->u.n.r;  /* b/c */
			temp->u.n.l->u.n.l = eqn->u.n.l; /* a */
			temp->u.n.l->op = '*';
			eqn->u.n.r = duplicate(temp->u.n.l->u.n.r);
			eqn->u.n.l = temp;
			eqn->op = '/';
		    }
		    else
		    {
			/* a + b */
		    }
		}
		break;
	case '*':
		if(eqn->u.n.l->op == '/')
		{
		    if(eqn->u.n.r->op == '/')
		    {
			/* a/c * b/d -> (a b)/(c d) */

			temp = eqn->u.n.l->u.n.r;
			eqn->u.n.l->u.n.r = eqn->u.n.r->u.n.l;
			eqn->u.n.r->u.n.l = temp;
			eqn->u.n.l->op = '*';
			eqn->u.n.r->op = '*';
			eqn->op = '/';
		    }
		    else
		    {
			/* a/c * b -> (a b)/c */

			temp = eqn->u.n.r;
			eqn->u.n.r = eqn->u.n.l->u.n.r;
			eqn->u.n.l->u.n.r = temp;
			eqn->u.n.l->op = '*';
			eqn->op = '/';
		    }
		}
		else
		{
		    if(eqn->u.n.r->op == '/')
		    {
			/* a * b/c -> (a b)/c */

			temp = eqn->u.n.r;
			eqn->u.n.r = temp->u.n.r;
			temp->u.n.r = temp->u.n.l;
			temp->u.n.l = eqn->u.n.l;
			eqn->u.n.l = temp;
			eqn->u.n.l->op = '*';
			eqn->op = '/';
		    }
		    else
		    {
			/* a * b */
		    }
		}
		break;
	case '/':
		if(eqn->u.n.l->op == '/')
		{
		    if(eqn->u.n.r->op == '/')
		    {
			/* (a/b)/(c/d) -> (a d)/(b c) */

			temp = eqn->u.n.l->u.n.r;
			eqn->u.n.l->u.n.r = eqn->u.n.r->u.n.r;
			eqn->u.n.r->u.n.r = eqn->u.n.r->u.n.l;
			eqn->u.n.r->u.n.l = temp;
			eqn->u.n.l->op = '*';
			eqn->u.n.r->op = '*';
			eqn->op = '/';
		    }
		    else
		    {
			/* (a/b)/c -> a/(b c) */

			temp = eqn->u.n.r;
			eqn->u.n.r = eqn->u.n.l;
			eqn->u.n.l = eqn->u.n.r->u.n.l;
			eqn->u.n.r->u.n.l = eqn->u.n.r->u.n.r;
			eqn->u.n.r->u.n.r = temp;
			eqn->u.n.r->op = '*';
			eqn->op = '/';
		    }
		}
		else
		{
		    if(eqn->u.n.r->op == '/')
		    {
			/* a/(b/c) -> (a c)/b */

			temp = eqn->u.n.l;
			eqn->u.n.l = eqn->u.n.r;
			eqn->u.n.r = eqn->u.n.l->u.n.l;
			eqn->u.n.l->u.n.l = temp;
			eqn->u.n.l->op = '*';
			eqn->op = '/';
		    }
		    else
		    {
		    }
		}
		break;
	default:
		eprintf("Bad op "); eprint_op(eqn->op);
		eprintf(" in common_denominator\n");
	}
}

void common_denom2(l, r)
eqnode *l,  *r;
{
	eqnode *temp;

	common_denominator(l);
	common_denominator(r);

	if( l->op == '/' )
	{
		if( r->op == '/' )
		{
			/* a/b : c/d -> (a d)/(b d) : (c b)/(b d) */

			temp = grballoc(eqnode);
			temp->op = '*';
			temp->u.n.l = l->u.n.r;
			temp->u.n.r = r->u.n.r;
			l->u.n.r = temp; /* a /(b d) */ 
			r->u.n.r = duplicate(temp); /* c/(b d) */
			temp = grballoc(eqnode);
			temp->op = '*';
			temp->u.n.l = l->u.n.l;
			temp->u.n.r = l->u.n.r->u.n.r;
			l->u.n.l = temp;
			temp = grballoc(eqnode);
			temp->op = '*';
			temp->u.n.l = r->u.n.l;
			temp->u.n.r = r->u.n.r->u.n.l;
			r->u.n.l = temp;
		}
		else
		{
			/* a/b : c -> a/b : (c b)/b */

			temp = grballoc(eqnode);
			temp->op = '*';
			temp->u.n.l = grballoc(eqnode);
			copy_node(temp->u.n.l,r);
			temp->u.n.r = duplicate(l->u.n.r); /* temp = c*b */
			r->u.n.r = duplicate(l->u.n.r);
			r->u.n.l = temp;
			r->op = '/';
		}
	}
	else
	{
		if( r->op == '/' )
		{
			/* a : b/c -> (a c)/c : b/c */

			temp = grballoc(eqnode);
			temp->op = '*';
			temp->u.n.l = grballoc(eqnode);
			copy_node(temp->u.n.l,l);
			temp->u.n.r = duplicate(r->u.n.r);
			l->u.n.r = duplicate(r->u.n.r);
			l->u.n.l = temp;
			l->op = '/';
		}
		else
		{
			/* a : b */
		}
	}
}	

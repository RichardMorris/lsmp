/*
 * This file is concerned with comparing equations.
 * To simplify things we assume that the equation has been run
 * through expand (and possibly had common denominators).
 * The main routines are
 *  compare_sum(l,r) true if two sums are equal
 *  compare_product(l,r) true if two products are equal upto scaler mult
 *  compare_element(l,r) true if two elements are real powers of each other
 *           we use x^(n*y) = (x^y)^n to give us an element x^y and a power
 *		n, and also x^(a+b) = x^a x^b from expand.
 * the above three call each other recursivly to resolve sums of product
 * of powers of elements, we also use the routines
 * get_scalar(l) which returns any scaler terms in a product, and
 * get_power(l) which returns any scaler terms in exponants
 * get_scalar and compare_product can be used efectivly to resolve
 * such equalities as 3x+3x = 2x+4x, similarly for powers.
 * other routines needed are
 * get_num_summands
 * get_ith_summand
 * get_num_multiplicands
 * get_ith_multiplicand
 *
 * well enouth comments, on with the code.
 */

#include <stdio.h>
#include <stdlib.h>
#include "eqn.h"

int get_num_summands(eqnode *eqn)
{
	if(eqn->op == '+' || eqn->op == '-')
		return(get_num_summands(eqn->u.n.l) +
		       get_num_summands(eqn->u.n.r) );
	return(1);
}

eqnode *get_ith_summand(eqnode *eqn,int i,int *mult)
{
	int n;

	if(eqn->op  == '+' )
	{
		n = get_num_summands(eqn->u.n.l);
		if( n >= i)
			return(get_ith_summand(eqn->u.n.l,i,mult));
		return(get_ith_summand(eqn->u.n.r,i-n,mult));
	}
	if( eqn->op == '-' )
	{
		n = get_num_summands(eqn->u.n.l);
		if( n >= i)
			return(get_ith_summand(eqn->u.n.l,i,mult));
		*mult *= -1;
		return(get_ith_summand(eqn->u.n.r,i-n,mult));
	}
	/* Now just an product */
	if( i == 1 )
		 return(eqn);
	return(NULL);
}

int get_num_multiplicands(eqnode *eqn)
{
	if(eqn->op == '*' )
		return(get_num_multiplicands(eqn->u.n.l) +
		       get_num_multiplicands(eqn->u.n.r) );
	return(1);
}

eqnode *get_ith_multiplicand(eqnode *eqn,int i)
{
	int n;

	if(eqn->op  == '*')
	{
		n = get_num_multiplicands(eqn->u.n.l);
		if( n >= i)
			return(get_ith_multiplicand(eqn->u.n.l,i));
		return(get_ith_multiplicand(eqn->u.n.r,i-n));
	}
	/* Now just an element */
	if( i == 1 ) return(eqn);
	return(NULL);
}

double get_scalar(eqnode *eqn)
{
	switch(eqn->op)
	{
	case NAME:
	case FUNCTION:
	case '^':
		return(1.0);
	case NUMBER:
		return(eqn->u.num);
	case '*':
		return(get_scalar(eqn->u.n.l)*get_scalar(eqn->u.n.r));
	case '/':
		return(get_scalar(eqn->u.n.l)/get_scalar(eqn->u.n.r));
	default:
		eprintf("Bad op %c in get_scalar\n",eqn->op);
		return(1.0);
	}
}

double get_power(eqnode *eqn)
{
	switch(eqn->op)
	{
	case NAME:
	case FUNCTION:
	case NUMBER:
		return(1.0);
	case '^': return(get_scalar(eqn->u.n.r));
        default:
                eprintf("Bad op %c in get_power\n",eqn->op);
                return(1.0);
        }
}

/* True if just a number False for sin(4) 2^3 */

int just_number(eqnode *eqn)
{
	switch(eqn->op)
	{
	case NUMBER: return(TRUE);
	case NAME:
	case FUNCTION:
	case '^':
		return(FALSE);
	case '+': case '-': case '*': case '/':
		return( just_number(eqn->u.n.l) && just_number(eqn->u.n.r));
	default:
		eprintf("Bad op %c in just_number\n",eqn->op);
		return(FALSE);
	}
}

/* forward definitions */

int compare_sums();
int compare_products();
	
int compare_elements(eqnode *l,eqnode *r)
{
	switch(l->op)
	{
	case NUMBER:
		switch(r->op)
		{
		case NUMBER: return(TRUE);
		case NAME:
		case FUNCTION:
		case '^':
			return(FALSE);
		default:
			eprintf("Bad op %c in compare_elements\n",r->op);
		}
	case NAME:
		switch(r->op)
		{
		case NAME:   return(!strcmp(l->u.str,r->u.str));
		case '^':
			return(r->u.n.l->op==NAME && just_number(r->u.n.r)
				&& !strcmp(l->u.str,r->u.n.l->u.str) );
		case NUMBER:
		case FUNCTION:
			return(FALSE);
		default:
			eprintf("Bad op %c in compare_elements\n",r->op);
		}
	case FUNCTION:
		switch(r->op)
		{
		case FUNCTION:
			return( l->u.f.f == r->u.f.f
				&& compare_sums(l->u.f.a,r->u.f.a) );
		case NAME:
		case NUMBER:
		case '^':
			return(FALSE);
		default:
			eprintf("Bad op %c in compare_elements\n",r->op);
		}
	case '^':
		switch(r->op)
		{
		case NAME:
			return(l->u.n.l->op==NAME && just_number(l->u.n.r)
                                && !strcmp(r->u.str,l->u.n.l->u.str) );
		case '^':
			return(compare_elements(l->u.n.l,r->u.n.l)
			 && compare_products(l->u.n.r,r->u.n.r) );
		case NUMBER:
		case FUNCTION:
			return(FALSE);
		default:
			eprintf("Bad op %c in compare_elements\n",r->op);
		}
	default:
		eprintf("Bad op %c in compare elements\n",l->op);
	}
	return(FALSE);
}

int compare_products(eqnode *l,eqnode *r)
{
	int n,m,i,j;
	int *doneL,*doneR;
	double power;
	eqnode *eqn_i,*eqn_j;

	m = get_num_multiplicands(l);
        n = get_num_multiplicands(r);

	doneL = (int *) calloc(m,sizeof(int));
	doneR = (int *) calloc(n,sizeof(int));

	/* first look for multiple occurances of multiplicand in l */

	for( i=1; i<=m; ++i) *(doneL-1+i) = FALSE;
	for( i=1; i<=n; ++i) *(doneR-1+i) = FALSE;

	power = 0.0;

	for( i=1; i<=m;++i)
	{
		if(*(doneL-1+i)) continue;
		eqn_i = get_ith_multiplicand(l,i);
		if( just_number(eqn_i))
	        {
			*(doneL-1+i) = TRUE;
			continue;
		}
		power = get_power(eqn_i);
		
		for(j=i+1; j<=m; ++j)
		{
			if(*(doneL-1+j)) continue;
			eqn_j = get_ith_multiplicand(l,j);
			if( just_number(eqn_j))
                	{
                       		*(doneL-1+j) = TRUE;
                        	continue;
                	}

			if(compare_elements(eqn_i,eqn_j) )
			{
				power += get_power(eqn_j);
				*(doneL-1+j) = TRUE;
			}
		}

	        /* now check the multiplicands in r */

                for(j=1; j<=n; ++j)
                {
                        if(*(doneR-1+j)) continue;
                        eqn_j = get_ith_multiplicand(r,j);
			if( just_number(eqn_j))
                	{
                       		*(doneR-1+j) = TRUE;
                        	continue;
                	}
                        if(compare_elements(eqn_i,eqn_j) )
                        {
                                power -= get_power(eqn_j);
                                *(doneR-1+j) = TRUE;
                        }
                }
		if( power != 0.0 ) break;
        }
        if( power == 0.0 )
	{
		for(i=1;i<=n;++i)
		{
		    if( *(doneR-1+i) ) continue;
		    eqn_i = get_ith_multiplicand(r,i);
		    if( just_number(eqn_i))
               	    {
                     	*(doneR-1+i) = TRUE;
                        continue;
                    }
		    power = get_power(eqn_i);
		    for(j=i+1;j<=n;++j)
		    {
			if( *(doneR-1+j) ) continue;
                        eqn_j = get_ith_multiplicand(r,j);
			if( just_number(eqn_j))
                	{
                       		*(doneR-1+j) = TRUE;
                        	continue;
                	}
                        if(compare_elements(eqn_i,eqn_j) )
                        {
                                power += get_power(eqn_j);
                                *(doneR-1+j) = TRUE;
                        }
		    }
		    if( power != 0.0 ) break;
        	}
			
	}
 
	free(doneL);
	free(doneR);

	return( power == 0.0 );
}

int compare_sums(eqnode *l,eqnode *r)
{
	int n,m,i,j;
	int *doneL,*doneR,mult;
	double scalar;
	eqnode *eqn_i,*eqn_j;

	m = get_num_summands(l);
        n = get_num_summands(r);

	doneL = (int *) calloc(m,sizeof(int));
	doneR = (int *) calloc(n,sizeof(int));


	for( i=1; i<=m; ++i) *(doneL-1+i) = FALSE;
	for( i=1; i<=n; ++i) *(doneR-1+i) = FALSE;

	scalar = 0.0;

	for( i=1; i<=m;++i)
	{
		if(*(doneL-1+i)) continue;
		mult = 1;
		eqn_i = get_ith_summand(l,i,&mult);
		scalar = get_scalar(eqn_i) * mult;
		
		/* first look for multiple occurances of summand in l */

		for(j=i+1; j<=m; ++j)
		{
			if(*(doneL-1+j)) continue;
			mult = 1;
			eqn_j = get_ith_summand(l,j,&mult);
			if(compare_products(eqn_i,eqn_j) )
			{
				scalar += get_scalar(eqn_j) * mult;
				*(doneL-1+j) = TRUE;
			}
		}

	        /* now check the summands in r */

                for(j=1; j<=n; ++j)
                {
                        if(*(doneR-1+j)) continue;
			mult = 1;
                        eqn_j = get_ith_summand(r,j,&mult);
                        if(compare_products(eqn_i,eqn_j) )
                        {
                                scalar -= get_scalar(eqn_j) * mult;
                                *(doneR-1+j) = TRUE;
                        }
                }
		if( scalar != 0.0 ) break;
        }

	/* If scalar == 0.0 then we have matched every thing on left 
		check everything on right is equal to zero */

	if( scalar == 0.0 )
	{
		for(i=1;i<=n;++i)
		{
		    if( *(doneR-1+i) ) continue;
		    mult = 1;
		    eqn_i = get_ith_summand(r,i,&mult);
		    scalar = get_scalar(eqn_i) * mult;
		    for(j=i+1;j<=n;++j)
		    {
			if( *(doneR-1+j) ) continue;
			mult = 1;
                        eqn_j = get_ith_summand(r,j,&mult);
                        if(compare_products(eqn_i,eqn_j) )
                        {
                                scalar += get_scalar(eqn_j) * mult;
                                *(doneR-1+j) = TRUE;
                        }
		    }
		if( scalar != 0.0 ) break;
        	}
			
	}
	
	free(doneL);
	free(doneR);

	return( scalar == 0.0 );
}

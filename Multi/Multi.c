/*
 * Multi.c:	a set of routines to handle multiple
 *		equations, in a single file.
		
 * The general scheam is to first read in the equations
 * then go through and find any special options such
 * as  _dimension = 4; or _normals = 3 or _colour = -2
 * these can effect the way the equations should be
 * interpreted.
 * If necessary combine the first few equations to
 * turn them into vector form.
 * Next find out which equations are parameters of variables
 * After that we work backwards through the list of equations
 * deducing which dimensions each equation is and checking the
 * dimensions are ok, we need a list of dimensions for each
 * equation to achieve this.
 * Now comes the fun step:
 *   we go through the equations replacing any occurance of
 *   diff(f,x) with the marker f@x 
 *   diff(f+h,x) will be represented as #1@x where #1 is f+h
 *   there is also a list of requests for derivatives and
 *   if a particular derivative is required for each equation
 *   then we calculate that derivative, the existance of
 *   markers like f@x implies we need to differentiate f wrt h.
 * Once we have done that we have an extended list of equations
 * which we can evaluate from the bottom up, and the
 * final step is to construct the appropriate rpe's.
 *
 * To get an implementation which does not need to know about 
 * normals or colours we need to split the main routine into
 * a number of sub routines:
 *
 * fscanMulti - reads in a multi from a file.
 * MfindOpts -  finds the values of options defined in the
 * 		multi, 
 *
 * now we can set the number of top level equation
 * using
 * Mget_opt_val_by_name  - to find the names of options
 * MsetNtop	- sets the number of top level eqns
 * MsetTopDim	- sets the dimensions for the top level eqns
 * 
 * then we combine the top level equations into one using
 * McombineTop
 * and find the names of the variable and parameters
 * MfindNames
 *
 * once we know the names we can tell the multi about derivative
 * MsetTopDeriv(top_no,"var1@var2") for example
 *
 * then its a 
 * McheckDims  
 * 
 * and McalcDerivs to find the derivative
 * and finally
 * McalcRPEs - to make the rpe's
 *
 * Got that? now lets get into the code.
 * Copyright (c) 1995 R.J.Morris ask me before you want to nick it
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <malloc.h>
#include "eqn.h"
#include "Multi.h"

/******************** Macros *****************************************/

#define grballoc(node) (node *) malloc( sizeof(node) )
#define MAX(a,b) ((a)>(b)?(a):(b))

#define COPY_STRING(target,source) { target = strdup(source); }

#define SET_DEPEND(i,j) { mul->depend[i][(j)/8] |= 0x01 << ((j)%8); }
#define GET_DEPEND(i,j) ( (mul->depend[i][(j)/8] ) & ( 0x01 << ((j)%8) ) )

#define SET_DEPVAR(i,j) { mul->depVar[i][(j)/8] |= 0x01 << ((j)%8); }
#define GET_DEPVAR(i,j) ( (mul->depVar[i][(j)/8] ) & ( 0x01 << ((j)%8) ) )

#define SET_DEPSPEC(i,j) { mul->depSpec[i][(j)/8] |= 0x01 << ((j)%8); }
#define GET_DEPSPEC(i,j) ( (mul->depSpec[i][(j)/8] ) & ( 0x01 << ((j)%8) ) )

#define SET_DONE(i) { mul->done[(i)/8] |= ( 0x01 << ((i)%8) ); }
#define GET_DONE(i) ( mul->done[(i)/8] & ( 0x01 << ((i)%8) ) )

/******************** Utilities Private ************************************/

/*
 * Remove entries from list of equations from n to n+m
 */

int MrmEqns(Multi *mul,int n, int m)
{
	int i;

	if( n <= 0 )
	{
		mul->error = TRUE;
		eprintf("MrmEqns bad arguments %d %d\n",n,m);
		dump_multi(stderr,mul);
		return(FALSE);
	}
	for(i=n;i<mul->n_eqns-m;++i)
		mul->eqns[i] = mul->eqns[i+m];
	for(i=mul->n_eqns-m;i<mul->n_eqns;++i)
		mul->eqns[i] = NULL;
	mul->n_eqns -= m;

	return(TRUE);
}

/* Add an equation */

int MaddEqn(Multi *mul,int n,eqnode *eqn)
{
	int l;

	if( mul->n_eqns >= MAX_NUM_EQNS )
	{
		eprintf("Sorry too many equations, max %d\n",
				MAX_NUM_EQNS);
		mul->error = TRUE;
		return(FALSE);
	}
	for(l=mul->n_eqns-1;l>=n;--l)
			mul->eqns[l+1] = mul->eqns[l];
	mul->eqns[n] = eqn;
	++mul->n_eqns;
	return(TRUE);
}

int MaddName(Multi *mul,int n,char *name,int dim)
{
	int k;

	if(n>=MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS)
	{
		eprintf("Error: too many names : max %d\n",
			MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS);
		mul->error = TRUE;
		return(FALSE);
	}
	for(k=mul->n_big-1;k>=n;--k)
	{
		mul->bignames[k+1] = mul->bignames[k];
		mul->bigdim[k+1] = mul->bigdim[k];
		mul->dimsum[k+1] = mul->dimsum[k];
		mul->bigvals[k+1] = mul->bigvals[k];
	}
	++mul->n_big;
	COPY_STRING(mul->bignames[n],name);
	mul->bigdim[n] = dim;
	return(TRUE);
}

int MrmName(Multi *mul,int n)
{
	int i;

	free(mul->bignames[n]);
	for(i=n;i<mul->n_big;++i)
	{
		mul->bignames[i] = mul->bignames[i+1];
		mul->bigdim[i] = mul->bigdim[i+1];
		mul->dimsum[i] = mul->dimsum[i+1];
		mul->bigvals[i] = mul->bigvals[i+1];
	}
	mul->n_big -= 1;
	return(TRUE);
}

/*
 * Find what type a name is
 * Bignames is organised
 * 0 .. n_vars-1 		Variables
 * n_vars .. n_vars + n_param-1	Parameters
 * n_vars + n_param .. n_big -1 Substitution
 *
 * Special vbl's and parameters start with a ?
 */

int Mget_type(Multi *mul,char *name)
{
	int i;

	for(i=0;i<mul->n_big;++i)
	{
		if( !strcmp(name,mul->bignames[i] ) )
		{
			if( i >= EQN_TO_NAME_INDEX(mul->n_eqns) )
				return(M_FREE_VBL);
			if( i >= mul->n_vars + mul->n_param )
				return(M_SUB);
			else if( i >= mul->n_vars )
			{
				if( *name == '?' ) return(M_SPEC_PARAM);
				else 		   return(M_PARAM);
			}
			if( *name == '?' ) return(M_SPEC_VAR);
			else		   return(M_VAR);
		}
	}
	eprintf("Error\007: name not found in Mget_type %s\n",name);
	eprintf("Names are\n");
	for(i=0;i<mul->n_big;++i)
	{
		eprintf("%s\n",mul->bignames[i]);
	}

	return(FALSE);
}

int MgetNameIndex(Multi *mul,char *name)
{
	int i;

	for(i=0;i<mul->n_big;++i)
	{
		if( !strcmp(name,mul->bignames[i] ) )
		{
			return(i);
		}
	}
	eprintf("Error\007: name not found in MgetNameIndex %s\n",name);
	eprintf("Names are\n");
	for(i=0;i<mul->n_big;++i)
	{
		eprintf("%s\n",mul->bignames[i]);
	}

	return(-1);
}

/* Check dimensions;
 * this bit stolen from count_eqn_args
 */

int Mget_dim(Multi *mul,eqnode *base)
{
  int leftcount,rightcount,i;

  if(base == NULL)
  {
	eprintf("Null equation while trying to count number of arguments\n");
	return(0);
  }

  switch( base->op )
  {
  case NUMBER:
    return( 1 );

  case NAME:
    for(i=0;i<mul->n_big;++i)
    {
	if( !strcmp(base->u.str,mul->bignames[i] ))
		return(mul->bigdim[i]);
    }
    eprintf("Error: name not found in Mget_dim %s\n",base->u.str);
    return(0);

  case '+': case '-': case '=':
    leftcount = Mget_dim(mul,base->u.n.l);
    rightcount = Mget_dim(mul,base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != rightcount )
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(leftcount);

  case '*':
    leftcount = Mget_dim(mul,base->u.n.l);
    rightcount = Mget_dim(mul,base->u.n.r);
    if(leftcount == 2  && rightcount == 2) return(2);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != 1  && rightcount != 1)
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    if(leftcount == 1) return(rightcount);
    return(leftcount);

  case '.':
    leftcount = Mget_dim(mul,base->u.n.l);
    rightcount = Mget_dim(mul,base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != rightcount )
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(1);

  case '/':
    leftcount = Mget_dim(mul,base->u.n.l);
    rightcount = Mget_dim(mul,base->u.n.r);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount == 2  && rightcount == 2) return(2);
    if( rightcount != 1)
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(leftcount);

  case '^':
    leftcount = Mget_dim(mul,base->u.n.l);
    rightcount = Mget_dim(mul,base->u.n.r);
    if(leftcount == 2  && rightcount == 2) return(1);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != rightcount || ( leftcount != 1 && leftcount != 3) )
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(rightcount);

  case FUNCTION:
    if( base->u.f.f->type == EXTERN_MAP || base->u.f.f->type == INTERN_MAP )
	return(base->u.f.f->dim);
    if( base->u.f.f->type == OPERATOR )
    {
	if( !strcmp(base->u.f.f->name,"diff") )
		return(Mget_dim(mul,base->u.f.a->u.n.l));
	eprintf("Whoopse! unknown operator %s\n",base->u.f.f->name);
	return(0);
    }
    else
	return(1);

  case ',':
    return( Mget_dim(mul,base->u.n.l)
		 + Mget_dim(mul,base->u.n.r));

  default:
	eprintf("Mget_dim: bad op ");
	eprint_op(base->op);
	eprintf("\n");
	break;
  }
  return( 0 );
}

/*
* Function:	Mcount_eqn
* Action:	counts the number of nodes in a tree 
*		note a,b only counts as two nodes
*		and if v is of dim 3 then it counts as 3 nodes
*/

int Mcount_eqn( base, mul )
eqnode *base; Multi *mul;
{
  int i;

  if( base == NULL )
  {
	eprintf("tried to count the nodes in a null equation\n");
	return(0);
  }
  if( mul == NULL )
  {
	eprintf("tried to count the nodes in a null equation\n");
	return(0);
  }
  switch( base->op )
  {
  case NUMBER:
    return( 1 );

  case NAME:
    for(i=0;i<mul->n_big;++i)
    {
	if( !strcmp(base->u.str,mul->bignames[i] ))
		return(mul->bigdim[i]);
    }
    eprintf("Error: name not found Mcount_eqn %s\n",base->u.str);
    return(0);

  case '+': case '-': case '*': case '/': case '^': case '=': case '.':
    return( 1 + Mcount_eqn(base->u.n.l,mul) + Mcount_eqn(base->u.n.r,mul) );
  case ',':
    return( Mcount_eqn(base->u.n.l,mul) + Mcount_eqn(base->u.n.r,mul) );
  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN ) return( 1 );
    if(base->u.f.f->type == SUM_FUN )
	 return(1+count_eqn_tree(base->u.f.a->u.n.r->u.n.r));
    return( 1 + Mcount_eqn(base->u.f.a,mul) );
  case BRACKET:
    return(Mcount_eqn(base->u.n.r,mul));
  default:
	eprintf("Mcount_eqn: bad op ");
	eprint_op(base->op);
	eprintf("\n");
	break;
  }
  return( 0 );
}

/* For sum type equations make them into variables */

int MmakeSumVbls(Multi *mul,eqn_names *allnames,eqnode *base)
{
  if( base == NULL )
  {
	eprintf("MmakeSumVbls: null equation\n");
	return(0);
  }
  if( mul == NULL )
  {
	eprintf("MmakeSumVbls: null Multi\n");
	return(0);
  }
  switch( base->op )
  {
  case NUMBER:
  case NAME:
    return( 1 );

  case '+': case '-': case '*': case '/': case '^': case '=': case '.':
  case ',':
	return( MmakeSumVbls(mul,allnames,base->u.n.l) &&
		MmakeSumVbls(mul,allnames,base->u.n.r) );

  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN ) return( 1 );
    if(base->u.f.f->type == SUM_FUN )
    {
	 if( !MmakeSumVbls(mul,allnames,base->u.f.a->u.n.l) )
		return(0);
	 if(base->u.f.a->u.n.r->u.n.l->op != NAME )
	 {
		eprintf("MmakeSumVbls: second op of %s should be a name but its ",base->u.f.f->name);
		eprint_op(base->u.f.a->u.n.r->u.n.l->op);
		eprintf("\n");
		return(0);
	 }
	 make_variable(allnames,base->u.f.a->u.n.r->u.n.l->u.str);
	 return( MmakeSumVbls(mul,allnames,base->u.f.a->u.n.r->u.n.r) );
    }
    return( MmakeSumVbls(mul,allnames,base->u.f.a) );
  case BRACKET:
    return( MmakeSumVbls(mul,allnames,base->u.n.r) );
  default:
	eprintf("MmakeSumVbls: bad op ");
	eprint_op(base->op);
	eprintf("\n");
	break;
  }
  return( 0 );
}

/* For sum type equations make them into variables */

int MaddSumVbls(Multi *mul,eqnode *base)
{
  if( base == NULL )
  {
	eprintf("MaddSumVbls: null equation\n");
	return(0);
  }
  if( mul == NULL )
  {
	eprintf("MaddSumVbls: null Multi\n");
	return(0);
  }
  switch( base->op )
  {
  case NUMBER:
  case NAME:
    return( 1 );

  case '+': case '-': case '*': case '/': case '^': case '=': case '.':
  case ',':
	return( MaddSumVbls(mul,base->u.n.l) &&
		MaddSumVbls(mul,base->u.n.r) );

  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN ) return( 1 );
    if(base->u.f.f->type == SUM_FUN )
    {
	 if( !MaddSumVbls(mul,base->u.f.a->u.n.l) )
		return(0);
	 if(base->u.f.a->u.n.r->u.n.l->op != NAME )
	 {
		eprintf("MaddSumVbls: second op of %s should be a name but its ",base->u.f.f->name);
		eprint_op(base->u.f.a->u.n.r->u.n.l->op);
		eprintf("\n");
		return(0);
	 }
	mul->bignames[mul->n_big] = strdup(base->u.f.a->u.n.r->u.n.l->u.str);
	++mul->n_big; 
	 return( MaddSumVbls(mul,base->u.f.a->u.n.r->u.n.r) );
    }
    return( MaddSumVbls(mul,base->u.f.a) );
  case BRACKET:
    return( MaddSumVbls(mul,base->u.n.r) );
  default:
	eprintf("MaddSumVbls: bad op ");
	eprint_op(base->op);
	eprintf("\n");
	break;
  }
  return( 0 );
}

/* differentes  g@x(x)=?1@?2(?2)   toget find g@x@y */

int do_trans_diff(Multi *mul,eqnode *eqn,char *name)
{
	char	*ptr;
	eqnode	*tmp1,*tmp2,*tmp3,*tmp4,*tmp5;
	int	i;

	/* basically all we need to do is subs every occurence of 
		x with ?2 */
/*
fprintf(stderr,"do_tran %s ",name); fprint_eqn(stderr,eqn); fprintf(stderr,"\n");
*/
	ptr = strrchr(name,'@');
	if( ptr == NULL )
	{
		eprintf("Requested derivative should contain a '@' its %s\n",
			name);
		mul->error = TRUE;
		return(FALSE);
	}
	if( eqn->u.n.r->op == NUMBER ) /* its already = 0 */
	{
		free(eqn->u.n.l->u.n.l->u.str);
		eqn->u.n.l->u.n.l->u.str = strdup(name);
		return(TRUE);
	}

	i = 0;
	tmp1 = eqn->u.n.l->u.n.r;
	tmp2 = eqn->u.n.r->u.n.r;
	while(1)
	{
		if( tmp1->op == NAME )
		{
			if(!strcmp(ptr+1,tmp1->u.str) )
			{
				tmp3  = duplicate(eqn->u.n.r->u.n.l);
				Mdiff_wrt(mul,tmp3,tmp2->u.str);
				if(i==0)
				{
					tmp4 = tmp3;
				}
				else
				{
					tmp5 = tmp4;
					tmp4 = grballoc(eqnode);
					tmp4->op = '+';
					tmp4->u.n.l = tmp5;
					tmp4->u.n.r = tmp3;
				}
				++i;
					
			}
/*
			else fprintf(stderr,"didn't match %s %s\n",
				ptr+1,tmp1->u.str);
*/
			break;
		}
		else	/* got to be * or , */
		{
			if(!strcmp(ptr+1,tmp1->u.n.l->u.str) )
			{
				tmp3  = duplicate(eqn->u.n.r->u.n.l);
				Mdiff_wrt(mul,tmp3,tmp2->u.n.l->u.str);
				if(i==0)
				{
					tmp4 = tmp3;
				}
				else
				{
					tmp5 = tmp4;
					tmp4 = grballoc(eqnode);
					tmp4->op = '+';
					tmp4->u.n.l = tmp5;
					tmp4->u.n.r = tmp3;
				}
				++i;
					
			}
			tmp1 = tmp1->u.n.r;
			tmp2 = tmp2->u.n.r;
		}
	}
	if(i == 0 )
	{
		/* deriv name not found must be 0 */
/*
fprintf(stderr,"i==0\n");
*/
		free_eqn_tree(eqn->u.n.r->u.n.l);
		free_eqn_tree(eqn->u.n.r->u.n.r);
		eqn->u.n.r->op = NUMBER;
		eqn->u.n.r->u.num = 0.0;
		free(eqn->u.n.l->u.n.l->u.str);
		eqn->u.n.l->u.n.l->u.str = strdup(name);
		return(TRUE);
	}
	free(eqn->u.n.l->u.n.l->u.str);
	eqn->u.n.l->u.n.l->u.str = strdup(name);
	free_eqn_tree(eqn->u.n.r->u.n.l);
	eqn->u.n.r->u.n.l = tmp4;
/*
fprintf(stderr,"do_tran done %s ",name); fprint_eqn(stderr,eqn); fprintf(stderr,"\n");
*/
	return(TRUE);
}
		
	
/* add a request to find   the  derivative  f@x@y */

int MaddRequest(Multi *mul,char *name)
{
	int i,j,dim;
	char *ptr;
	eqnode *eqn;

	ptr = strrchr(name,'@');
	if(ptr == NULL)
	{
		eprintf("MaddRequest: bad name %s can only add names of form f@x\n",
			name);
		mul->error = TRUE;
		return(FALSE);
	}

	/* find exact matches */

	for(i=0;i<mul->n_big;++i)
		if( !strcmp(mul->bignames[i],name) )
			return(TRUE);

/*
eprintf("MaddRequest %s\n",name);
eprint_eqn(mul->eqns[0]);
eprintf("\n");
*/

	if(name[0] == '?')
	{
	    *ptr = '\0';
	    for(i=0;i<mul->n_vars;++i)
	    {
		if( !strcmp(mul->bignames[i],name) )
		{
			*ptr = '@';
			if( ! MaddVar(mul,name) )
			{
				mul->error = TRUE;
				return(FALSE);
			}
			return(TRUE);
		}
	    }
	    *ptr = '@';
/*
		eprintf("Adding spec %s\n",name);
		dump_multi(stderr,mul);
		return(TRUE);
*/
	}
	else
	{
		/*  Find a match for head of name  */

	    *ptr = '\0';
	    for(i=mul->n_vars+mul->n_param;i<mul->n_big;++i)
	    {
		if( !strcmp(mul->bignames[i],name) )
		{
			dim = mul->bigdim[i];
			*ptr = '@';
			j = NAME_TO_EQN_INDEX(i);
			eqn = duplicate(mul->eqns[j]);
			if( eqn->u.n.l->op == '*' )
			{
				if( ! do_trans_diff(mul,eqn,name) )
				{
					mul->error = TRUE;
					return(FALSE);
				}
			}
			else
			{
				Mdiff_wrt(mul,eqn->u.n.r,ptr+1);
				eval_funs(eqn);
				free(eqn->u.n.l->u.str);
				COPY_STRING(eqn->u.n.l->u.str,name);
			}

			/* We can't garantee that the list of names
			   have not changed so NEI macro is not valid  
			   however the eqn index will not change
			   as any new eqns add will come after
			*/

			if( !MaddEqn(mul,j+1,eqn) )
				{
					mul->error = TRUE;
					return(FALSE);
				}
			if( ! MaddName(mul,EQN_TO_NAME_INDEX(j+1),name,dim) )
				{
					mul->error = TRUE;
					return(FALSE);
				}
			return(TRUE);
		}
	    }
	    *ptr = '@';
	}

	eprintf("Error\007: name not found in MaddRequest %s\n",name);
	eprintf("Names are\n");
	for(i=0;i<mul->n_big;++i)
	{
		eprintf("%s\n",mul->bignames[i]);
	}
	mul->error = TRUE;
	return(FALSE);
}

/************** PUBLIC FUNCTIONS *****************/

/* initilise a multi */

int Minit(Multi *mul)
{
	int i;

	mul->n_eqns = mul->n_opts = mul->n_vars = mul->n_param = 0;
	mul->n_top = mul->n_derivs = mul->n_sums = 0;
	for(i=0;i<MAX_NUM_TOP;++i)
		mul->top_dims[i] = 0;
	for(i=0;i<MAX_NUM_TOP;++i)
		mul->n_top_derivs[i]=0;
	for(i=0;i<MAX_NUM_EQNS;++i)
	{
		mul->eqns[i] = NULL;
	}
	for(i=0;i<MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS;++i)
	{
		mul->bignames[i] = NULL;
	}
	mul->results = NULL;
	mul->error = TRUE;
	return(TRUE);
}

/* Clears out any parameters, but leaves options, and variables */

int Mclear(Multi *mul)
{
	int i;

	for(i=0;i<MAX_NUM_EQNS;++i)
	{
		if(mul->eqns[i] != NULL)
		  {
		    free_eqn_tree(mul->eqns[i]);
		  }
		mul->eqns[i] = NULL;
	}
	for(i=mul->n_vars;i<MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS;++i)
	{
		if(mul->bignames[i] != NULL )
			free(mul->bignames[i]);
		mul->bignames[i] = NULL;
	}
	mul->n_eqns = mul->n_param = mul->n_sums = 0;
	if(mul->results != NULL)
			free(mul->results);
	mul->results = NULL;
	mul->error = TRUE;
	return(TRUE);
}

/* read in the equations from a file, do no other processing */

int fscanMulti(FILE *fp,Multi *mul)
{
	eqnode *eqn;
	int	i;

	/* initilise multi */

	mul->error = TRUE;
	for(i=0;i<mul->n_eqns;++i)
		free_eqn_tree(mul->eqns[i]);
	mul->n_eqns = 0; mul->n_derivs = 0;

	/* read in equations */

/*CONSTCOND*/
	while(TRUE)
	{
	  eqn = fscan_eqn(fp);
	  if(eqn == NULL) break;
	  eval_funs(eqn);
	  if( ! MaddEqn(mul,mul->n_eqns,eqn) ) /* Add at end of list */
	  {
		mul->error = TRUE;
		return(FALSE);
	  }
	}
/*
	if(feof(fp))	mul->error = FALSE;
	else		mul->error = TRUE;
	return(!mul->error);
*/
	mul->error = FALSE;
	return(TRUE);
}

/* Reads in a multi but only looks for the 
   parameter values, nothing else is changed
*/

int fscanMultiParams(FILE *fp,Multi *mul)
{
	eqnode *eqn;
	int	i;

	/* read in equations */
/*CONSTCOND*/

	if(mul->error) return(FALSE);
	while(TRUE)
	{
	  eqn = fscan_eqn(fp);
	  if(eqn == NULL) break;
	  eval_funs(eqn);
	  if(eqn->op == '=' && eqn->u.n.l->op == NAME 
		&& eqn->u.n.r->op == NUMBER )
	  {
 		for(i=0;i<mul->n_param;++i)
		{
		if( !strcmp(eqn->u.n.l->u.str,
			mul->bignames[mul->n_vars+i]) )
		    {
			mul->bigvals[mul->n_vars+i] = eqn->u.n.r->u.num;
		    }
		}
	  }
	  free_eqn_tree(eqn);
	}
	return(TRUE);
}

int MfindOpts(Multi *mul)
{
	int	i,j;
	char	*ptr;
	eqnode	*eqn;

	/* Find and equations for dimensions, colours, normals */
	/* ones read from the equation override the previous ones */

    	if(mul->error) return(FALSE);
	for(i=0;i<mul->n_eqns;++i)
	{
		eqn = mul->eqns[i];
		if( eqn->op == '='   
		 && eqn->u.n.l->op == NAME
		 && eqn->u.n.r->op == NUMBER )
		{
		    ptr = strchr(eqn->u.n.l->u.str,'_');
		    if(ptr == NULL) continue;

		    for(j=0;j<mul->n_opts;++j)
		    {
			if( !strcmp(ptr+1,mul->opt_names[j]) )
			{
				mul->opt_vals[j] = eqn->u.n.r->u.num;
				free_eqn_tree(eqn);
				MrmEqns(mul,i,1);
				--i;
				break;
		    	}
		    }
		}
	}
	return(TRUE);
}

/* Now we need to combine the top level equations */
/* False on error */

int McombineTop(Multi *mul)
{
    eqnode *eqn;
    int i,j;

    if(mul->error) return(FALSE);
    mul->copy_top = 0;
    if(mul->n_eqns == 0)
    {
    	eprintf("Error\007: no equations specified\n");
    	mul->error = TRUE;
    	return(FALSE);
    }

    for(i=0;i<mul->n_top;++i)
    {
	if( mul->eqns[i]->op == '=' )
	{
		/* We need to combine the top  mul->top_dims[i] equations */

	    if(mul->n_eqns < abs(mul->top_dims[i]) + i)
	    {
		eprintf(
	"Error\007: not enougth equations specified need at least %d\n",
			abs(mul->top_dims[i]) + i);
		mul->error = TRUE;
		return(FALSE);
	    }
	    for(j=0;j<abs(mul->top_dims[i]);++j)
	    {
		if( mul->top_dims[i] > 0 && 
			(mul->eqns[i+j]->op != '=' 
			  || mul->eqns[i+j]->u.n.l->op != NAME ) )

	   	{
			eprintf("Error\007: Equation of form X = ....; required\n");
			mul->error = TRUE;
			return(FALSE);
		}
	    }

	/* Should really do something to cope with implicit eqns here */

	/* now join the equations together */

	    switch(mul->top_dims[i])
	    {
	    case -1:
		mul->eqns[i]->op = '-';
		break;
	    case 1:
		eqn = mul->eqns[i]->u.n.r;
		free_eqn_node(mul->eqns[i]->u.n.l);
		free_eqn_node(mul->eqns[i]);
		mul->eqns[i] = eqn;
		break;
	    case 2:
		eqn = join_eqns(',',mul->eqns[i]->u.n.r,
				     mul->eqns[i+1]->u.n.r);
		free_eqn_node(mul->eqns[i]->u.n.l);
		free_eqn_node(mul->eqns[i]);
		free_eqn_node(mul->eqns[i+1]->u.n.l);
		free_eqn_node(mul->eqns[i+1]);
		mul->eqns[i] = eqn;
		MrmEqns(mul,i+1,1);
		break;
	    case 3:
		eqn = join_eqns(',',mul->eqns[i]->u.n.r,
			join_eqns(',',mul->eqns[i+1]->u.n.r,
			  		mul->eqns[i+2]->u.n.r));
		free_eqn_node(mul->eqns[i]->u.n.l);
		free_eqn_node(mul->eqns[i]);
		free_eqn_node(mul->eqns[i+1]->u.n.l);
		free_eqn_node(mul->eqns[i+1]);
		free_eqn_node(mul->eqns[i+2]->u.n.l);
		free_eqn_node(mul->eqns[i+2]);
		mul->eqns[i] = eqn;
		MrmEqns(mul,i+1,2);
		break;
		  	
	    case 4:
		eqn = join_eqns(',',mul->eqns[i]->u.n.r,
			join_eqns(',',mul->eqns[i+1]->u.n.r,
			  join_eqns(',',mul->eqns[i+2]->u.n.r,
					  mul->eqns[i+3]->u.n.r)));
		free_eqn_node(mul->eqns[i]->u.n.l);
		free_eqn_node(mul->eqns[i]);
		free_eqn_node(mul->eqns[i+1]->u.n.l);
		free_eqn_node(mul->eqns[i+1]);
		free_eqn_node(mul->eqns[i+2]->u.n.l);
		free_eqn_node(mul->eqns[i+2]);
		free_eqn_node(mul->eqns[i+3]->u.n.l);
		free_eqn_node(mul->eqns[i+3]);
		mul->eqns[i] = eqn;
		MrmEqns(mul,i+1,3);
		break;
	    default:
		eprintf("Requested dimension %d to large\n",
			mul->top_dims[i]);
		mul->error = TRUE;
			return(FALSE);
	    }
	    mul->copy_top += abs(mul->top_dims[i]);
	}
	else
	{ 
	    mul->copy_top += 1;
	}
    }
    mul->error = FALSE;
    return(TRUE);
}

/* Now we can work out which are parameters and variables */

int MfindNames(Multi *mul)
{
	eqnode *eqn,*tempeqn1,*tempeqn2;
	char   *tempname;
	int	i,j;
	int	old_count=0;	/* num params def in equation */
	char	*old_names[MAX_NUM_PARAMS];
	double	old_vals[MAX_NUM_PARAMS];
	int	new_count=0;	/* num params def in equation */
	char	*new_names[MAX_NUM_PARAMS];
	double	new_vals[MAX_NUM_PARAMS];
	int	num_int_defs=0;	/* num of interval defs in eqn's */
	eqn_names *allnames;
	

	/* copy names and values of old parameters */

	if(mul->error) return(FALSE);
	for(i=0;i<mul->n_param;++i)
	{
		old_names[i] = mul->bignames[i+mul->n_vars];
		old_vals[i] = mul->bigvals[i+mul->n_vars];
		mul->bignames[i+mul->n_vars] = NULL;
		++old_count;
	}

	/* free substitution names */

	for(i=mul->n_vars+mul->n_param;i<mul->n_big;++i)
	{
		free(mul->bignames[i]);
		mul->bignames[i] = NULL;
	}

	mul->n_param = 0;
	mul->n_big = 0;

	/* go through second level eqn finding parameter and var names */

	for(i = mul->n_top; i < mul->n_eqns; ++i)
	{
		eqn = mul->eqns[i];

		if( eqn->op != '=' )
		{
			eprintf("Error: assignment equation required in eqn %d\n",i);
			eprint_eqn(eqn);
			eprintf("\n");
			mul->error = TRUE;
			return(FALSE);
		}
		if( eqn->u.n.l->op == '*'
		 && eqn->u.n.r->op == '*' 
		 && eqn->u.n.l->u.n.l->op == NAME
		 && eqn->u.n.r->u.n.l->op == NAME )
		{
			/* now check the syntax here should be
				g*(x,y) = ?1*(?2,?3)
			*/
			tempeqn1 = eqn->u.n.l->u.n.r;
			tempeqn2 = eqn->u.n.r->u.n.r;
			while(1)
			{
				if( tempeqn1->op == NAME
				 && tempeqn2->op == NAME )
				{
					if( tempeqn2->u.str[0] != '?' )
					{
eprintf("Error: bad form (1) for function def should be g(x,y) = ?1(?2,?3)\n");
eprint_eqn(eqn);
eprintf("\n");
mul->error = TRUE;
return(FALSE);
					}
					break;
				} else if( tempeqn1->op == ',' 
					&& tempeqn2->op == ',' )
				{
					if( tempeqn1->u.n.l->op != NAME
					 || tempeqn2->u.n.l->op != NAME
					 || tempeqn2->u.n.l->u.str[0] != '?' )
					{
eprintf("Error: bad form (2) for function def should be g(x,y) = ?1(?2,?3)\n");
eprint_eqn(eqn);
eprintf("\n");
mul->error = TRUE;
return(FALSE);
					}
					tempeqn1 = tempeqn1->u.n.r;
					tempeqn2 = tempeqn2->u.n.r;
				}
				else
				{
eprintf("Error: bad form (3) for function def should be g(x,y) = ?1(?2,?3)\n");
eprint_eqn(eqn);
eprintf("\n");
mul->error = TRUE;
return(FALSE);
				}
				
			}
			continue;
		}

		if( eqn->u.n.l->op != NAME )
		{
			eprintf("Error: assignment equation required\n");
			eprint_eqn(eqn);
			eprintf("\n");
			mul->error = TRUE;
			return(FALSE);
		}
		if( eqn->u.n.r->op == NUMBER )
		{
			COPY_STRING(new_names[new_count],eqn->u.n.l->u.str);
			new_vals[new_count] = eqn->u.n.r->u.num;
			++new_count;
			free_eqn_tree(eqn);
			MrmEqns(mul,i,1);
			--i;
		}
		else if( eqn->u.n.r->op == INTERVAL )
		{
			if( eqn->u.n.l->op != NAME
			 || eqn->u.n.r->u.n.l->op != NUMBER
			 || eqn->u.n.r->u.n.r->op != NUMBER )
			{
				eprintf("Error\007: Bad interval definition\n");
				eprint_eqn(eqn);
				eprintf("\nShould be of form 'x = [-1,1]'\n");
				mul->error = TRUE;
				return(FALSE);
			}
			if( num_int_defs >= mul->n_vars )
			{
				fprintf(stderr,"Too many variable definitions %d max %d\n",num_int_defs,mul->n_vars);
				mul->error = TRUE;
				return(FALSE);
			}
			if(mul->bignames[num_int_defs] == NULL)
			{
				COPY_STRING(mul->bignames[num_int_defs],
					eqn->u.n.l->u.str);
			}
			else if( strcmp(mul->bignames[num_int_defs],
					eqn->u.n.l->u.str) )
			{
				free(mul->bignames[num_int_defs]);
				COPY_STRING(mul->bignames[num_int_defs],
					eqn->u.n.l->u.str);
			}

			mul->var_min[num_int_defs] =
				eqn->u.n.r->u.n.l->u.num;
			mul->var_max[num_int_defs] =
				eqn->u.n.r->u.n.r->u.num;
			++num_int_defs;
			MrmEqns(mul,i,1);
			--i;
		}
		else if( eqn->u.n.r->op == NAME &&
			*(eqn->u.n.r->u.str) == '?' )
		{
			/* Special variables and parameters */

		    if( isalpha( *(eqn->u.n.r->u.str+1) ) )
		    {
				/*  ?x  this is a parameter */

			new_names[new_count] = strdup(eqn->u.n.r->u.str);
			new_vals[new_count] = 0.0;
			++new_count;
		    }
		    else if( *(eqn->u.n.r->u.str+1) == '\0' )
		    {
			    /* Eqn of form a = ? have spec param ?a */

			new_names[new_count] = (char *) 
				calloc( strlen(eqn->u.n.l->u.str)+2,
				sizeof(char));
			strcpy(new_names[new_count],"?");
			strcat(new_names[new_count],
				eqn->u.n.l->u.str);
			new_vals[new_count] = 0.0;

			/* also want to change rhs of eqn */

			free(eqn->u.n.r->u.str);
			eqn->u.n.r->u.str = strdup(new_names[new_count]);

			++new_count;
		    }
		    else if( isdigit( *(eqn->u.n.r->u.str+1) )  )
		    {
			/* eqn of form X = ?1 */
			/* Might want to disallow special vbl's */

			/* Now perform substitution in all other eqns */
			/* This allows us to have derivates by lhs */

			for( j = 0; j < mul->n_eqns ; ++j )
			{
				if( i == j ) continue;
				substitute(mul->eqns[j],eqn);
			} 

			MrmEqns(mul,i,1);
			--i;
		    }
		    else
		    {
			eprintf("Bad special name %s\n",eqn->u.n.r->u.str);
			mul->error = TRUE;
			return(FALSE);
		    }
		}
	}

	/* Lets now build up the array of names */
	
	allnames = add_eqn_names(NULL,mul->eqns[0]);
	for(i=1;i<mul->n_eqns;++i)
	{
		allnames = add_eqn_names(allnames,mul->eqns[i]);
	}
	for(i=0;i<mul->n_vars;++i)
	{
		make_variable(allnames,mul->bignames[i]);
		mul->n_big++; /* remember your a macro, (repeat) */
	}
	for(i=mul->n_top;i<mul->n_eqns;++i)
	{	/* remove lhs from allnames */
		if( mul->eqns[i]->u.n.l->op == '*' )
		{
			make_variable(allnames,
				mul->eqns[i]->u.n.l->u.n.l->u.str);
		}
		else
			make_variable(allnames,mul->eqns[i]->u.n.l->u.str);
	}

	/* Now search for free variable which are second variable
		in Sum type equations */

	for(i=0;i<mul->n_eqns;++i)
		MmakeSumVbls(mul,allnames,mul->eqns[i]);

	mul->n_param = num_parameters(allnames);

	for(i=0; i < mul->n_param;++i)
	{
		tempname = get_parameter(allnames,i+1);
		if(i>= MAX_NUM_PARAMS )
		{
			eprintf("Sorry too many parameters\007\n");
			break;
		}
		mul->bignames[mul->n_big]=strdup(tempname);
		mul->bigvals[mul->n_big] = 0.0;
		for(j=0;j<old_count;++j)
			if(!strcmp(tempname,old_names[j]) )
				mul->bigvals[mul->n_big] = old_vals[j];
		for(j=0;j<new_count;++j)
			if(!strcmp(tempname,new_names[j]) )
				mul->bigvals[mul->n_big] = new_vals[j];
		++mul->n_big;
	}

	/* add lhs of subs eqns */

	for(i=mul->n_top;i<mul->n_eqns;++i)
	{	
		if( mul->eqns[i]->u.n.l->op == '*' )
		{
			mul->bignames[mul->n_big] = 
				strdup(mul->eqns[i]->u.n.l->u.n.l->u.str);
		}
		else
			mul->bignames[mul->n_big] = 
				strdup(mul->eqns[i]->u.n.l->u.str);

		++mul->n_big; 
	}

	/* add free variables in sum eqns */

	for(i=0;i<mul->n_eqns;++i)
	{
		MaddSumVbls(mul,mul->eqns[i]);
	}

	return(TRUE);
}

int McheckDims(Multi *mul)
{
	int i,j,dim;

	/* Check dimensions */
	
	if(mul->error) return(FALSE);
	if(mul->n_derivs != 0) 
	{
		eprintf(
		"Error n_derivs should be 0 when checking dims  its %d\n",
		mul->n_derivs);
		mul->error = TRUE;
		return(FALSE);
	}

	for(i=0;i<mul->n_big;++i)
		mul->bigdim[i]=1;
	
	for(i=mul->n_eqns-1;i>=mul->n_top;--i)
	{
		if( mul->eqns[i]->u.n.l->op == '*' )
			dim = Mget_dim(mul,mul->eqns[i]->u.n.r->u.n.l);
		else
			dim = Mget_dim(mul,mul->eqns[i]->u.n.r);
		if(dim == 0)
		{
			eprintf("Error\007: mismatch in dimensions in\n");
			eprint_eqn(mul->eqns[i]);
			eprintf(";\nDimensions are:\n");
			for(j=0;j<mul->n_big;++j)
				eprintf("%s dim %d\n",mul->bignames[j],mul->bigdim[j]);
			mul->error = TRUE;
			return(FALSE);
		}
/*
		mul->bigdim[mul->n_vars + mul->n_param + i - mul->n_top] = dim;
*/
		mul->bigdim[EQN_TO_NAME_INDEX(i)] = dim;
	}

	for(i=0;i<mul->n_top;++i)
	{
		dim=Mget_dim(mul,mul->eqns[i]);
		if(dim != abs(mul->top_dims[i]))
		{
			eprintf(
		"Error\007: wrong dimensions %d top level equation # %d should be %d\n",
				dim,i,mul->top_dims[i]);
			eprint_eqn(mul->eqns[i]);
			eprintf(";\nDimensions are:\n");
			for(j=0;j<mul->n_big;++j)
				eprintf("%s dim %d\n",mul->bignames[j],mul->bigdim[j]);
			mul->error = TRUE;
			return(FALSE);
		}
	}

	return(TRUE);
}

/* Create derivative equations , calculate all required derivatives */
/* Also deals with Sum type operators 
	where we want to ensure that
		a = b + Sum(c,0,2,1,d);
		d = c^2;
	becomes
		a = b + @1;
		@1 = Sum(c,0,2,1,d);
		d = c^2;

	*/

int McalcDerivs(Multi *mul)
{
	int	i,j,k;
	eqnode *eqn;
	char	*ptr;

	/* Now we can have a play with diffs */
	/* calls to Mconvert_diff may call MaddRequest which
		may add equations to the list */

	if(mul->error) return(FALSE);
	mul->n_derivs = 0;

	/* first we deal with Sums */

	for(i=0;i<mul->n_top;++i)
	{
/*
		printf("ConvertSum ");
		print_eqn(mul->eqns[i]);
		printf("\n");
*/
		MconvertSum(mul,mul->eqns[i],mul->n_top);
	}
	for(i=mul->n_top;i<mul->n_eqns;++i)
	{
/*
		printf("ConvertSum %s := ",mul->eqns[i]->u.n.l->u.str);
		print_eqn(mul->eqns[i]->u.n.r);
		printf("\n");
*/
		if( strncmp(mul->eqns[i]->u.n.l->u.str,"!!",2) )
			MconvertSum(mul,mul->eqns[i]->u.n.r,i+1);
	}

	/* convert substitution eqns */
	for(i=mul->n_eqns-1;i>=mul->n_top+mul->n_derivs;--i)
	{
		if( mul->eqns[i]->u.n.l->op == NAME )
			Mconvert_diff(mul,mul->eqns[i]->u.n.r);
	}

	/* convert top eqns */
	for(i=mul->n_top+mul->n_derivs-1;i>=0;--i)
		Mconvert_diff(mul,mul->eqns[i]);

	/* now add required derivatives */

	for(i=0;i<mul->n_top;++i)
	{
	    mul->top_deriv_ref[i] = mul->n_derivs;
	    for(j=0;j<mul->n_top_derivs[i];++j)
	    {
		ptr = strrchr(mul->top_deriv_names[i][j],'@');
		if(ptr == NULL )
		{
			/* just a first derivative  */

			eqn = duplicate(mul->eqns[i]);
			Mdiff_wrt(mul,eqn,mul->top_deriv_names[i][j]);
			eval_funs(eqn);
			if( !MaddEqn(mul,mul->n_top+mul->n_derivs,eqn) )
			{
				mul->error = TRUE;
				return(FALSE);
			}
			++mul->n_derivs;
		}
		else	/* A lesser derivative should already exist */
		{
		    *ptr = '\0';
		    for(k=0;k<j;++k)
		    {
			if( !strcmp(mul->top_deriv_names[i][k],
				mul->top_deriv_names[i][j]) )
			{
				eqn = duplicate(
					mul->eqns[mul->n_top
					+ mul->n_derivs-j
						+k]);
				break;
			}
		    }
		    if(k==j)
		    {
	eprintf("Couldn't find a previous derivative with name  %s\n",
					mul->top_deriv_names[i][k]);
				*ptr = '@';
				mul->error = TRUE;
				return(FALSE);
		    }
		    *ptr = '@';
		    Mdiff_wrt(mul,eqn, (ptr+1));
			if( !MaddEqn(mul,mul->n_top+mul->n_derivs,eqn) )
			{
				mul->error = TRUE;
				return(FALSE);
			}
		    ++mul->n_derivs;
		}
	    }
	} /* end for i */
/*
dump_multi(stderr,mul);
fprintf(stderr,"Now rem triv's\n");
*/
/* Now we can remove any trivial equations of the 
	form  f = 0; f=1; f=x;  */
/* But don't want to do those which start with ! */

	for(i=mul->n_eqns-1;i>=mul->n_top+mul->n_derivs;--i)
	{
		if( mul->eqns[i]->u.n.l->op == '*' )
		{}
		else if( mul->eqns[i]->u.n.r->op == NUMBER ||
		    ( mul->eqns[i]->u.n.r->op == NAME  
		    && mul->eqns[i]->u.n.l->u.str[0] != '!' ) )
				 /* Hope this is ok */
		{
			eqn=mul->eqns[i];

			for(j=0;j<mul->n_top+mul->n_derivs;++j)
				substitute(mul->eqns[j],eqn);
			for(j=mul->n_top+mul->n_derivs;j<i;++j)
				substitute(mul->eqns[j]->u.n.r,eqn);

			MrmEqns(mul,i,1);
			MrmName(mul,EQN_TO_NAME_INDEX(i));
		}
		else
			eval_funs(mul->eqns[i]);
	}
/*
dump_multi(stderr,mul);
*/
	for(i=0;i<mul->n_top+mul->n_derivs;++i)
		eval_funs(mul->eqns[i]);
	for(i=mul->n_top+mul->n_derivs;i<mul->n_eqns;++i)
	{
		eval_funs(mul->eqns[i]->u.n.r);
	}
	
	return(TRUE);
}

int McalcSumRPE(Multi *mul,int i)
{
	eqnode *sum_eqn;

	/* a sum type eqn */

	if( mul->eqns[i]->u.n.r->op != FUNCTION ||
		mul->eqns[i]->u.n.r->u.f.f->type != SUM_FUN )
	{
		eprintf("Should be a sum function\n");
		mul->error = TRUE;
       		return(FALSE);
       	}
	sum_eqn = mul->eqns[i]->u.n.r;

	if( Mget_dim(mul,sum_eqn->u.f.a) != sum_eqn->u.f.f->nvars )
	{
		eprintf("Actual argument count different to profile for function %s\n",sum_eqn->u.f.f->name);
		mul->error = TRUE;
		return(FALSE);
	}
	if( Mget_dim(mul,mul->eqns[i+1]->u.n.r) != 1 )
	{
		eprintf("Vectors not allowed is sum type equations\n");
		mul->error = TRUE;
		return(FALSE);
	}
		
	if( sum_eqn->u.f.f->nvars > MAX_FUN_ARGS )
	{
		eprintf("Sorry function %s has too many arguments to convert to reverse polish string\n",sum_eqn->u.f.f->name);
		mul->error = TRUE;
		return(FALSE);
	}

	if( sum_eqn->u.f.a->u.n.r->u.n.l->op != NAME)
	{
		eprintf("second opperand of %s must be a name\n",
				sum_eqn->u.f.f->name);
		mul->error = TRUE;
		return(FALSE);
	}

	mul->sumref[i] = 
		atoi( mul->bignames[EQN_TO_NAME_INDEX(i)]+2);
	mul->tvarref[i] = -1;
	mul->sums[ mul->sumref[i] ].ref =
		MgetNameIndex(mul,sum_eqn->u.f.a->u.n.r->u.n.l->u.str);
	if( mul->sums[ mul->sumref[i] ].ref < 0 )
	{
		eprintf("Couldn't find %s\n",
			sum_eqn->u.f.a->u.n.r->u.n.l->u.str);
		mul->error = TRUE;
		return(FALSE);
	}

	/* check next eqn is !1 */

	if( mul->bignames[EQN_TO_NAME_INDEX(i+1)][0] != '!' 
	 || atoi( mul->bignames[EQN_TO_NAME_INDEX(i+1)]+ 1)
		!= mul->sumref[i]  )
	{
		eprintf("Next name after a sum eqn wrong %s %s\n",
			 mul->bignames[EQN_TO_NAME_INDEX(i)],
			 mul->bignames[EQN_TO_NAME_INDEX(i+1)]);
	 	eprintf("refs %d %d\n",
atoi( mul->bignames[EQN_TO_NAME_INDEX(i+1)]+ 1),
mul->sumref[i]  );
		mul->error = TRUE;
		return(FALSE);
	}

	mul->sums[mul->sumref[i]].fun = sum_eqn->u.f.f;
	mul->rpes[i] = Mmake_vrpe(mul,sum_eqn->u.f.a->u.n.r->u.n.r);

	/* now use hard coded eqnations */
	
	if( !strcmp(sum_eqn->u.f.f->name,"Sum") ) 
		 mul->sums[mul->sumref[i]].hard_code = 1;
	else if( !strcmp(sum_eqn->u.f.f->name,"Prod") ) 
		 mul->sums[mul->sumref[i]].hard_code = 2;
	else if( !strcmp(sum_eqn->u.f.f->name,"Min") ) 
		 mul->sums[mul->sumref[i]].hard_code = 3;
	else if( !strcmp(sum_eqn->u.f.f->name,"Max") ) 
		 mul->sums[mul->sumref[i]].hard_code = 4;
	else if( !strcmp(sum_eqn->u.f.f->name,"Mean") ) 
		 mul->sums[mul->sumref[i]].hard_code = 5;
	else 
		 mul->sums[mul->sumref[i]].hard_code = 0;
/*
print_eqn(sum_eqn->u.f.a->u.n.r->u.n.r); printf("\n");
print_Mvrpe(mul->rpes[i],mul);
*/
	return(TRUE);
}

int McalcTransRPE(Multi *mul,int i)
{
	eqnode *tmp1,*tmp2;
	int	j,lhs,rhs;

/*
fprintf(stderr,"McaltransRPE "); fprint_eqn(stderr,mul->eqns[i]); fprintf(stderr,"\n");
*/
	if( mul->eqns[i]->u.n.r->op == NUMBER )
	{
		mul->rpes[i] = Mmake_vrpe(mul,mul->eqns[i]->u.n.r);
		mul->tvarref[i] = -1;
		return(TRUE);
	}
		
	mul->tvarref[i] = mul->n_tvars;
	if(mul->n_tvars >= MAX_NUM_TRANS )
	{
		eprintf("Sorry: too many transition equations\n");
		mul->error = TRUE;
		return(FALSE);
	}
	mul->rpes[i] = Mmake_vrpe(mul,mul->eqns[i]->u.n.r->u.n.l);
			/* now we need to set up tables */

	
	mul->tvars[mul->n_tvars].eqnno = i;
	mul->tvars[mul->n_tvars].refs
		 = (short *) malloc(sizeof(short)*mul->n_vars);
	mul->tvars[mul->n_tvars].prov_req
		 = (short *) malloc(sizeof(short)*mul->n_vars);
	for(j=0;j<mul->n_vars;++j)
		mul->tvars[mul->n_tvars].refs[j] = -1;

	tmp1 = mul->eqns[i]->u.n.l->u.n.r;
	tmp2 = mul->eqns[i]->u.n.r->u.n.r;
	while(1)
	{
		if(tmp1->op == ',')
		{
			/* find which name matches lhs */

			lhs = MgetNameIndex(mul,tmp1->u.n.l->u.str);
			rhs = MgetNameIndex(mul,tmp2->u.n.l->u.str);
			if(rhs >= mul->n_vars || rhs < 0)
			{
				eprintf("formal parameters in ");
				eprint_eqn(mul->eqns[i]->u.n.r);
				eprintf(" should be a variables\n");
				mul->error = TRUE;
				return(FALSE);
			}
			if(lhs < 0 )
			{
				eprintf("Bad formal parameters in ");
				eprint_eqn(mul->eqns[i]->u.n.l);
				eprintf("\n");
				mul->error = TRUE;
				return(FALSE);
			}	
			mul->tvars[mul->n_tvars].refs[rhs]=lhs;
			tmp1 = tmp1->u.n.r;
			tmp2 = tmp2->u.n.r;
		}
		else 
		{
			/* find which name matches lhs */

			lhs = MgetNameIndex(mul,tmp1->u.str);
			rhs = MgetNameIndex(mul,tmp2->u.str);
			if(rhs >= mul->n_vars || rhs < 0)
			{
				eprintf("formal parameters in ");
				eprint_eqn(mul->eqns[i]->u.n.r);
				eprintf(" should be a variables\n");
				eprint_eqn(tmp2);
				eprintf("\n");
				mul->error = TRUE;
				return(FALSE);
			}
			if(lhs < 0 )
			{
				eprintf("Bad formal parameters in ");
				eprint_eqn(mul->eqns[i]->u.n.l);
				eprintf("\n");
				mul->error = TRUE;
				return(FALSE);
			}	
			mul->tvars[mul->n_tvars].refs[rhs]=lhs;
			break;
		}
	}
/*
	for(j=0;j<mul->n_vars;++j)
	{
		if(mul->tvars[mul->n_tvars].refs[j] >= 0 )
		fprintf(stderr,"provide for %s is %s\n",
			mul->bignames[j],
			mul->bignames[mul->tvars[mul->n_tvars].refs[j]]);
	}
*/
	/* finding requires is easier after dep eqn */

	++mul->n_tvars;
	return(TRUE);
}

int McalcRPEs(Multi *mul)
{
	int  i,j,k,dim;
	short size;

	/* calculate the culamative dims */

	if(mul->error) return(FALSE);
/*
dump_multi(stderr,mul);
*/
	mul->n_tvars = 0;
	dim = 0;
	for(i=0;i<mul->n_big;++i)
	{
		dim += mul->bigdim[i];
		if( dim >= MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS )
		{
			eprintf("Sorry not enough space to hold all values\n");
			mul->error = TRUE;
			return(FALSE);
		}
		mul->dimsum[i] = dim;
	}

	/* Now we can construct the rpes */

	for(i=0; i<mul->n_top+mul->n_derivs;++i)
	{
/*
eprint_eqn(mul->eqns[i]); eprintf("\tcalcRpe\n");
*/
		mul->rpes[i] = Mmake_vrpe(mul,mul->eqns[i]);
		mul->tvarref[i] = -1;
		mul->sumref[i] = -1;
		if(mul->rpes[i] == NULL)
		{
			mul->error = TRUE;
			return(FALSE);
		}
	}

	for(i=mul->n_top+mul->n_derivs;i<mul->n_eqns;++i)
	{
/*
eprint_eqn(mul->eqns[i]); eprintf("\tcalcRpe\n");
*/
		if( mul->bignames[EQN_TO_NAME_INDEX(i)][0] == '!' 
		 && mul->bignames[EQN_TO_NAME_INDEX(i)][1] == '!' )
		{
			if( !McalcSumRPE(mul,i))
			{
				mul->error = TRUE;
				return(FALSE);
			}
			mul->tvarref[i] = -1;
		}
		else if( mul->eqns[i]->u.n.l->op == '*' )
		{
			if( !McalcTransRPE(mul,i))
			{
				mul->error = TRUE;
				return(FALSE);
			}
			mul->sumref[i] = -1;
		}
		else
		{
			mul->sumref[i] = -1;
			mul->tvarref[i] = -1;
			mul->rpes[i] = Mmake_vrpe(mul,mul->eqns[i]->u.n.r);
		}

		if(mul->rpes[i] == NULL)
		{
			mul->error = TRUE;
			return(FALSE);
		}
/*
fprint_Mvrpe(stderr,mul->rpes[i],mul);
*/
	}

	/* Now create the dependancies list */

	for(i=0;i<MAX_NUM_EQNS;++i)
	{
		for(j=0;j<MAX_NUM_EQNS/8;++j)
			mul->depend[i][j] = 0x00;

		for(j=0;j<MAX_NUM_VARS/8;++j)
			mul->depVar[i][j] = 0x00;

		for(j=0;j<MAX_NUM_SUMS/8;++j)
			mul->depSpec[i][j] = 0x00;
	}

#ifdef NOT_DEF
	for(i=0;i<mul->n_eqns;++i)
	{
		names = add_eqn_names(NULL,mul->eqns[i]);
		for(j=mul->n_vars+mul->n_param;j<mul->n_big;++j)
		{
			if( get_name_index(names,mul->bignames[j]) > 0 )
			{
				SET_DEPEND(i,NAME_TO_EQN_INDEX(j));
			}
		}
		free_eqn_names(names);
	}
#endif
	for(i=0;i<mul->n_eqns;++i)
	{
		for(j=mul->n_vars+mul->n_param;j<mul->n_big;++j)
		{
			if( mul->bignames[j][0] == '!' &&
				mul->bignames[j][1] != '!' )
			{
/*
				if( name_in_eqn(mul->eqns[i],mul->bignames[j]) )
					SET_DEPEND(i-1,NAME_TO_EQN_INDEX(j));
*/
			}
			else if( name_in_eqn(mul->eqns[i],mul->bignames[j]) )
			{
				SET_DEPEND(i,NAME_TO_EQN_INDEX(j));
			}
		}

		for(j=0;j<mul->n_vars;++j)
		{
			if( mul->tvarref[i] >= 0 )
			{
/*
fprintf(stderr,"tvar[%d] = %d\n",i,mul->tvarref[i]);
*/
				if( name_in_eqn(
					mul->eqns[i]->u.n.r->u.n.l,
					mul->bignames[j]) )
				SET_DEPVAR(i,j);
			}
			else if( name_in_eqn(mul->eqns[i],mul->bignames[j]) )
			{
				SET_DEPVAR(i,j);
			}
		}
			
		for(j=0;j<mul->n_sums;++j)
		{
			if( name_in_eqn(mul->eqns[i],
				mul->bignames[mul->sums[j].ref]) )
			{
				SET_DEPSPEC(i,j);
			}
		}
			
	}
/*
fprintMdep(stdout,mul);
*/

	
	/* now create culilative dependancies 
	 * if a dep on b  and b dep on c then a dep on c
	 */

	for(j=mul->n_eqns-1;j>=0;--j)
	{
		for(i=0;i<j;++i)
		{
			if( GET_DEPEND(i,j) )
			{
			    for(k=0;k<=mul->n_eqns/8;++k)
				mul->depend[i][k] |= mul->depend[j][k];
			    for(k=0;k<=mul->n_vars/8;++k)
				mul->depVar[i][k] |= mul->depVar[j][k];
			    for(k=0;k<=mul->n_sums/8;++k)
				mul->depSpec[i][k] |= mul->depSpec[j][k];
			}
			else if( mul->sumref[i] >= 0 
			      && GET_DEPEND(i+1,j) )
			{
			    for(k=0;k<=mul->n_vars/8;++k)
				mul->depVar[i][k] |= mul->depVar[j][k];
			    for(k=0;k<=mul->n_sums/8;++k)
				mul->depSpec[i][k] |= mul->depSpec[j][k];
			}
		}
	}

	/* what do the trans require */

	for(j=0;j<mul->n_tvars;++j)
	{
/*
		eprintf("Trans "); eprint_eqn(mul->eqns[mul->tvars[j].eqnno]);
		eprintf("\n");
*/
		for(k=0;k<mul->n_vars;++k)
		{
			if( mul->tvars[j].refs[k] >= 0 )
			{
				mul->tvars[j].prov_req[k] = 1;
/*
				eprintf("provide %s\n",mul->bignames[k]);
*/
			}
			else if( GET_DEPVAR(mul->tvars[j].eqnno,k) )
			{
/*
				eprintf("requires %s\n",mul->bignames[k]);
*/
				mul->tvars[j].prov_req[k] = -1;
			}
			else
				mul->tvars[j].prov_req[k] = 0;
		}
	}
		
/*
fprintMdep(stderr,mul);
*/

	/* Finally work out space for results */

	size = 0;
	for(i=0;i<mul->n_top;++i)
	{
		mul->top_res_ref[i] = size;
		size += (short) abs(mul->top_dims[i])
			* ( 1 + mul->n_top_derivs[i] );
	}
	mul->results = (double *) calloc(MAX(4,size),sizeof(double));
	return(TRUE);
}

/*************************** end of pre processing now evaluation *******/

int MstartEval(Multi *mul)
{
	if(mul->error)
	{
		eprintf("Error: bad equations while trying to evaluate\n");
		return(FALSE);
	}
	memset(mul->done,0,MAX_NUM_EQNS/8 * sizeof(unsigned char));
	return(TRUE);
}

double *MevalSum(Multi *mul,int n)
{
	int i,j,k,l,var_ref;
	register double r,low,high,inc;
	double *ptr;
	double itt_vals[4];
	int	depend_eqns[MAX_NUM_EQNS];
	int	num_dep;
	Msums	sum;

	/* first evaluate the bounds arguments */

	ptr = Meval_vrpe(mul->rpes[n],mul->bigvals);
	SET_DONE(n);

	/* got those */

	low = *ptr; high = *(ptr+1); inc = *(ptr+2);

/*
fprintf(stderr,"Meval_Sum: Sum "); fprint_eqn(stderr,mul->eqns[n]); fprintf(stderr,"\n");
fprintf(stderr,"sum no %d\n",mul->sumref[n]);
fprintMdep(stderr,mul);
print_Mvrpe(mul->rpes[n],mul);
fprintMvals(stdout,mul);
printf("MevalSum l %f h %f i %f\n",low,high,inc);
*/

	/* do fisrt itteration */
	/* its the next vrpe in the list */

	num_dep = 0;
	sum = mul->sums[mul->sumref[n]];
	var_ref = mul->dimsum[sum.ref]-1;
	mul->bigvals[var_ref] = low;
	for(i=mul->n_eqns-1;i>n+1;--i)
	{
		if( ! GET_DEPEND(n+1,i) ) continue;
		if(  GET_DEPSPEC(i,mul->sumref[n]) )
		{
/*
fprintf(stderr,"Sum dep on "); fprint_eqn(stderr,mul->eqns[i]); fprintf(stderr,"\n");
*/
			depend_eqns[num_dep++] = i;
		}
		else if ( GET_DONE(i) ) continue;
		if( mul->sumref[i] >= 0 )
		{
/*
printf("Meval_Sum: Sum "); print_eqn(mul->eqns[i]); printf("\n");
*/
			ptr = MevalSum(mul,i);
		}
		else
		{
/*
printf("Meval_Sum: Eqn "); print_eqn(mul->eqns[i]); printf("\n");
*/
			ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
		}
		SET_DONE(i);
		j = EQN_TO_NAME_INDEX(i);
		for(k=0;k<mul->bigdim[j];++k)
		{
			mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
/*
printf("Set %s %d %f\n",mul->bignames[j],mul->dimsum[j]-k-1,*(ptr+k));
*/
		}
	}
	itt_vals[1] = *Meval_vrpe(mul->rpes[n+1],mul->bigvals);
	itt_vals[2] = 1.0;
	itt_vals[3] = low;
	if( sum.hard_code ) itt_vals[0] = itt_vals[1];
	else
		 itt_vals[0] =  eval_rpe(sum.fun->rpe2,itt_vals);
	itt_vals[2] = 2.0;

/*
printf("res %f val %f num %f var %f\n",itt_vals[0],itt_vals[1],
		itt_vals[2],itt_vals[3]);
fprintMvals(stdout,mul);
*/
	for(r=low+inc;r<high+TAD;r=r+inc)
	{
		/* now evaluate the summand */

		mul->bigvals[var_ref] = r;
		for(l=0;l<num_dep;++l)
		{
			i = depend_eqns[l];
			if( mul->sumref[i] >= 0 )
				ptr = MevalSum(mul,i);
			else
				ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
			j = EQN_TO_NAME_INDEX(i);
			for(k=0;k<mul->bigdim[j];++k)
			{
				mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
			}
		}
		itt_vals[3] = r;
		itt_vals[1] = *Meval_vrpe(mul->rpes[n+1],mul->bigvals);

		/* and eval itterative function */

		switch( sum.hard_code )
		{
		case 0: itt_vals[0] = eval_rpe(sum.fun->rpe,itt_vals);
			break;
		case 1: itt_vals[0] += itt_vals[1]; break;
		case 2: itt_vals[0] *= itt_vals[1]; break;
		case 3: if(itt_vals[1] < itt_vals[0] )
				 itt_vals[0] = itt_vals[1]; 
			break;
		case 4: if(itt_vals[1] > itt_vals[0] )
				 itt_vals[0] = itt_vals[1]; 
			break;
		case 5: itt_vals[0] += (itt_vals[1]-itt_vals[0])/itt_vals[2];
			break;
		}
		itt_vals[2] += 1.0;
/*
printf("res %f val %f num %f var %f\n",itt_vals[0],itt_vals[1],
		itt_vals[2],itt_vals[3]);
*/
	}

	j = EQN_TO_NAME_INDEX(n);
	mul->bigvals[mul->dimsum[j]-1] = itt_vals[0];
	return( mul->bigvals+mul->dimsum[j]-1);
}

/* New style uses get done for tops and big results list */

double *MevalTop2(Multi *mul,int n)
{
	int i,j,k,res_ref;
	double *ptr;

	if(mul->error) return(mul->results);

	if( GET_DONE(n) )
		return(mul->results+mul->top_res_ref[n]);
	for(i=mul->n_eqns-1;i>=mul->n_derivs+mul->n_top;--i)
	{
		if( GET_DONE(i) ) continue;
		if( ! GET_DEPEND(n,i) ) continue;

		if( mul->sumref[i] >= 0 )
			ptr = MevalSum(mul,i);
		else
		{
/*
printf("MevalTop: "); print_eqn(mul->eqns[i]); printf("\n");
*/
			ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
		}
		j = EQN_TO_NAME_INDEX(i);
		for(k=0;k<mul->bigdim[j];++k)
		{
			mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
		}
		SET_DONE(i);
	}
	ptr = Meval_vrpe(mul->rpes[n],mul->bigvals);
	SET_DONE(n);
	res_ref = mul->top_res_ref[n];
	for(i=0;i<abs(mul->top_dims[n]);++i)
		*(mul->results+i+res_ref) = *(ptr+i);
	return(mul->results+res_ref);
}

/* Evaluate a top derivative new style call for use with MTrans */

double *MevalTopDeriv(Multi *mul,int n,int d)
{
	int i,j,k,ref;
	double *ptr;

	if(mul->error)
	{
		return(mul->results);
	}
	ref = mul->top_deriv_ref[n] + d + mul->n_top;
	if( GET_DONE(ref) )
		return(mul->results+mul->top_res_ref[n]
			 + (d+1) * abs(mul->top_dims[n]) );

	for(i=mul->n_eqns-1;i>=mul->n_derivs+mul->n_top;--i)
	{
		if( GET_DONE(i) ) continue;
		if( ! GET_DEPEND(ref,i) ) continue;

		if( mul->sumref[i] >= 0 )
			ptr = MevalSum(mul,i);
		else
			ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
		j = EQN_TO_NAME_INDEX(i);
		for(k=0;k<mul->bigdim[j];++k)
		{
			mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
		}
		SET_DONE(i);
	}
	ptr = Meval_vrpe(mul->rpes[ref],mul->bigvals);
	SET_DONE(ref);
	ref = mul->top_res_ref[n]+ (d+1) * abs(mul->top_dims[n]);
	for(i=0;i<abs(mul->top_dims[n]);++i)
		*(mul->results+i+ref) = *(ptr+i);
	return(mul->results + ref );
}

/************ super fancy with call back *********/
/*** fun is void fun(int require[MAX_NUM_VARS],int provide[],double val[]) **/

double *MevalTransCB(Multi *mul,int n,void (*fun)())
{
	double vals[MAX_NUM_EQNS],*ptr;
	int i,j,tref;

	tref = mul->tvarref[n];
/*
fprintf(stderr,"TransCB "); fprint_eqn(stderr,mul->eqns[n]); fprintf(stderr,"\n");
*/
	for(i=mul->n_vars-1;i>=0;--i)
	{
		if(mul->tvars[tref].refs[i] >= 0)
		{
		vals[i] = mul->bigvals[
			mul->dimsum[
				mul->tvars[tref].refs[i]
			  	   ] -1 ];
		}
	}
	fun(mul->tvars[tref].prov_req, vals);

	for(i=mul->n_vars-1;i>=0;--i)
	{
		if(mul->tvars[tref].prov_req[i] < 0)
		{
		mul->bigvals[ mul->dimsum[i] - 1 ] = vals[i];
		}
	}
	/* finally evaluate the rpe */

	ptr = Meval_vrpe(mul->rpes[n],mul->bigvals);
	SET_DONE(n);

	/* got those */

	j = EQN_TO_NAME_INDEX(n);
	mul->bigvals[mul->dimsum[j]-1] = *ptr;
/*
fprintf(stderr,"Trans callback val %f\n",*ptr);
*/
	return( mul->bigvals+mul->dimsum[j]-1);
}

double *MevalSumCB(Multi *mul,int n,void (*fun)())
{
	int i,j,k,l,res_ref,var_ref;
	register double r,low,high,inc,res,val;
	double *ptr;
	double itt_vals[4];
	int	depend_eqns[MAX_NUM_EQNS];
	int	num_dep;
	int	eqn_num;
	Msums	sum;

	/* first evaluate the bounds arguments */

	ptr = Meval_vrpe(mul->rpes[n],mul->bigvals);
	SET_DONE(n);

	/* got those */

	low = *ptr; high = *(ptr+1); inc = *(ptr+2);

/*
fprintf(stderr,"Meval_Sum: Sum "); fprint_eqn(stderr,mul->eqns[n]); fprintf(stderr,"\n");
printf("MevalSum l %f h %f i %f\n",low,high,inc);
fprintf(stderr,"sum no %d\n",mul->sumref[n]);
fprintMdep(stderr,mul);
print_Mvrpe(mul->rpes[n],mul);
fprintMvals(stdout,mul);
*/

	/* do fisrt itteration */
	/* its the next vrpe in the list */

	num_dep = 0;
	sum = mul->sums[mul->sumref[n]];
	var_ref = mul->dimsum[sum.ref]-1;
	mul->bigvals[var_ref] = low;
	for(i=mul->n_eqns-1;i>n+1;--i)
	{
		if( ! GET_DEPEND(n+1,i) ) continue;
		if(  GET_DEPSPEC(i,mul->sumref[n]) )
		{
/*
fprintf(stderr,"Sum dep on "); fprint_eqn(stderr,mul->eqns[i]); fprintf(stderr,"\n");
*/
			depend_eqns[num_dep++] = i;
		}
		else
		{
/*
fprintf(stderr,"Sum don't dep on "); fprint_eqn(stderr,mul->eqns[i]); fprintf(stderr,"\n");
*/
			 if ( GET_DONE(i) ) continue;
		}
		if( mul->sumref[i] >= 0 )
		{
/*
printf("Meval_Sum: Sum "); print_eqn(mul->eqns[i]); printf("\n");
*/
			ptr = MevalSumCB(mul,i,fun);
		}
		else if( mul->tvarref[i] >= 0 )
		{
			ptr = MevalTransCB(mul,i,fun);
		}
		else
		{
/*
printf("Meval_Sum: Eqn "); print_eqn(mul->eqns[i]); printf("\n");
*/
			ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
		
			j = EQN_TO_NAME_INDEX(i);
			for(k=0;k<mul->bigdim[j];++k)
			{
				mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
/*
printf("Set %s %d %f\n",mul->bignames[j],mul->dimsum[j]-k-1,*(ptr+k));
*/
			}
		}
		SET_DONE(i);
	}
	itt_vals[1] = *Meval_vrpe(mul->rpes[n+1],mul->bigvals);
	itt_vals[2] = 1.0;
	itt_vals[3] = low;
	if( sum.hard_code ) itt_vals[0] = itt_vals[1];
	else
		 itt_vals[0] =  eval_rpe(sum.fun->rpe2,itt_vals);
	itt_vals[2] = 2.0;
/*
printf("res %f val %f num %f var %f\n",itt_vals[0],itt_vals[1],
		itt_vals[2],itt_vals[3]);
fprintMvals(stdout,mul);
*/
	for(r=low+inc;r<high+TAD;r=r+inc)
	{
		/* now evaluate the summand */

		mul->bigvals[var_ref] = r;
		for(l=0;l<num_dep;++l)
		{
			i = depend_eqns[l];
			if( mul->sumref[i] >= 0 )
				ptr = MevalSumCB(mul,i,fun);
			else if( mul->tvarref[i] >= 0 )
			{
				ptr = MevalTransCB(mul,i,fun);
			}
			else
			{
				ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);

				j = EQN_TO_NAME_INDEX(i);
				for(k=0;k<mul->bigdim[j];++k)
				{
					mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
				}
			}
		}
		itt_vals[3] = r;
		itt_vals[1] = *Meval_vrpe(mul->rpes[n+1],mul->bigvals);

		/* and eval itterative function */

		switch( sum.hard_code )
		{
		case 0: itt_vals[0] = eval_rpe(sum.fun->rpe,itt_vals);
			break;
		case 1: itt_vals[0] += itt_vals[1]; break;
		case 2: itt_vals[0] *= itt_vals[1]; break;
		case 3: if(itt_vals[1] < itt_vals[0] )
				 itt_vals[0] = itt_vals[1]; 
			break;
		case 4: if(itt_vals[1] > itt_vals[0] )
				 itt_vals[0] = itt_vals[1]; 
			break;
		case 5: itt_vals[0] += (itt_vals[1]-itt_vals[0])/itt_vals[2];
			break;
		}
		itt_vals[2] += 1.0;
/*
printf("res %f\n",itt_vars[0]);
printf("res %f val %f num %f var %f\n",itt_vals[0],itt_vals[1],
		itt_vals[2],itt_vals[3]);
*/
	}

	j = EQN_TO_NAME_INDEX(n);
	mul->bigvals[mul->dimsum[j]-1] = itt_vals[0];
	return( mul->bigvals+mul->dimsum[j]-1);
}

/* New style uses get done for tops and big results list */

double *MevalTopCB(Multi *mul,int n,void (*fun)())
{
	int i,j,k,res_ref;
	double *ptr;

	if(mul->error) return(mul->results);

	if( GET_DONE(n) )
		return(mul->results+mul->top_res_ref[n]);
	for(i=mul->n_eqns-1;i>=mul->n_derivs+mul->n_top;--i)
	{
		if( GET_DONE(i) ) continue;
		if( ! GET_DEPEND(n,i) ) continue;

/*
fprintf(stderr,"MevalTop: "); fprint_eqn(stderr,mul->eqns[i]); fprintf(stderr,"\n");
fprint_Mvrpe(stderr,mul->rpes[i],mul);
*/
		if( mul->sumref[i] >= 0 )
			ptr = MevalSumCB(mul,i,fun);
		else if( mul->tvarref[i] >= 0 )
			ptr = MevalTransCB(mul,i,fun);
		else
		{
			ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
			j = EQN_TO_NAME_INDEX(i);
			for(k=0;k<mul->bigdim[j];++k)
			{
				mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
			}
		}
		SET_DONE(i);
	}
	ptr = Meval_vrpe(mul->rpes[n],mul->bigvals);
	SET_DONE(n);
	res_ref = mul->top_res_ref[n];
	for(i=0;i<abs(mul->top_dims[n]);++i)
		*(mul->results+i+res_ref) = *(ptr+i);
	return(mul->results+res_ref);
}

/* Evaluate a top derivative new style call for use with MTrans */

double *MevalTopDerivCB(Multi *mul,int n,int d,void (*fun)())
{
	int i,j,k,ref;
	double *ptr;

	if(mul->error)
	{
		return(mul->results);
	}
	ref = mul->top_deriv_ref[n] + d + mul->n_top;
	if( GET_DONE(ref) )
		return(mul->results+mul->top_res_ref[n]
			 + (d+1) * abs(mul->top_dims[n]) );

	for(i=mul->n_eqns-1;i>=mul->n_derivs+mul->n_top;--i)
	{
		if( GET_DONE(i) ) continue;
		if( ! GET_DEPEND(ref,i) ) continue;

		if( mul->sumref[i] >= 0 )
			ptr = MevalSumCB(mul,i,fun);
		else if( mul->tvarref[i] >= 0 )
			ptr = MevalTransCB(mul,i,fun);
		else
		{
			ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
			j = EQN_TO_NAME_INDEX(i);
			for(k=0;k<mul->bigdim[j];++k)
			{
				mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
			}
		}
		SET_DONE(i);
	}
	ptr = Meval_vrpe(mul->rpes[ref],mul->bigvals);
	SET_DONE(ref);
	ref = mul->top_res_ref[n]+ (d+1) * abs(mul->top_dims[n]);
	for(i=0;i<abs(mul->top_dims[n]);++i)
		*(mul->results+i+ref) = *(ptr+i);
	return(mul->results + ref );
}

/***********************************************************************/

/* Old style doesn't use GET_DONE for tops */

double *MevalTop(Multi *mul,int n)
{
	int i,j,k;
	double *ptr;

	if(mul->error) return(mul->results);

	for(i=mul->n_eqns-1;i>=mul->n_derivs+mul->n_top;--i)
	{
		if( GET_DONE(i) ) continue;
		if( ! GET_DEPEND(n,i) ) continue;

		ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
		j = EQN_TO_NAME_INDEX(i);
		for(k=0;k<mul->bigdim[j];++k)
		{
			mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
		}
		SET_DONE(i);
	}
	ptr = Meval_vrpe(mul->rpes[n],mul->bigvals);
	mul->results[0] = *(ptr);
	mul->results[1] = *(ptr+1);
	mul->results[2] = *(ptr+2);
	mul->results[3] = *(ptr+3);
	return(mul->results);
}

/* Evaluate a derivative old style call 
	can not be used with MTrans as corupts the result order */

double *MevalDeriv(Multi *mul,int n)
{
	int i,j,k;
	double *ptr;

	if(mul->error)
	{
		return(mul->results);
	}
	for(i=mul->n_eqns-1;i>=mul->n_derivs+mul->n_top;--i)
	{
		if( GET_DONE(i) ) continue;
		if( ! GET_DEPEND(mul->n_top + n,i) ) continue;

		ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
		j = EQN_TO_NAME_INDEX(i);
		for(k=0;k<mul->bigdim[j];++k)
		{
			mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
		}
		SET_DONE(i);
	}
	ptr = Meval_vrpe(mul->rpes[mul->n_top+n],mul->bigvals);
	mul->results[0] = *(ptr);
	mul->results[1] = *(ptr+1);
	mul->results[2] = *(ptr+2);
	mul->results[3] = *(ptr+3);
	return(mul->results);
}

/******* print outs ************************************/

void fprint_multi(FILE *fp,Multi *mul)
{
	int i;

	if( mul->error )
		return;
	for(i=0;i<mul->n_eqns;++i)
	{
		fprint_eqn(fp,mul->eqns[i]);
		fprintf(fp,";\n");
	}
	for(i=0;i<mul->n_vars;++i)
	{
		fprintf(fp,"%s = [",mul->bignames[i]);
		fprint_num(fp,mul->var_min[i]);
		fprintf(fp,",");
		fprint_num(fp,mul->var_max[i]);
		fprintf(fp,"];\n");
	}
	for(i=0;i<mul->n_param;++i)
	{
		fprintf(fp,"%s = ", mul->bignames[mul->n_vars+i]);
		fprint_num(fp,mul->bigvals[mul->n_vars+i]);
		fprintf(fp,";\n");
	}
	for(i=0;i<mul->n_opts;++i)
	{
		fprintf(fp,"_%s = ",mul->opt_names[i]);
		fprint_num(fp,mul->opt_vals[i]);
		fprintf(fp,";\n");
/*
		if( mul->opt_vals[i] == rint(mul->opt_vals[i]) )
			fprintf(fp,"_%s = %.0lf;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
		else
			fprintf(fp,"_%s = %f;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
*/
	}
}

void dump_multi(FILE *fp,Multi *mul)
{
	int i,j,k;

	if( mul->error )
		fprintf(fp,"Error in Multi:\n");
	fprintf(fp,"TOP\n");
	for(i=0;i<mul->n_top;++i)
	{
		fprintf(fp,"Top num %d res pos %d\n",
			i,mul->top_res_ref[i]);
		fprint_eqn(fp,mul->eqns[i]);
		fprintf(fp,";\n");
	}
	fprintf(fp,"DERIVS\n");
	k = mul->n_top;
	for(i=0;i<mul->n_top;++i)
	{
	    fprintf(fp,"Top No %d deriv_ref %d \n",
		i,mul->top_deriv_ref[i]);
	    for(j=0;j< mul->n_top_derivs[i];++j)
	    {
		fprintf(fp,"#%d@%s = ",i,
			mul->top_deriv_names[i][j]);
		fprint_eqn(fp,mul->eqns[k]);
		fprintf(fp,";\n");
		++k;
	    }
	}
	fprintf(fp,"SUBSTITUTIONS\n");	
	for(i=mul->n_top+mul->n_derivs;i<mul->n_eqns;++i)
	{
		fprintf(fp,"%s = ",mul->bignames[EQN_TO_NAME_INDEX(i)]);
		fprint_eqn(fp,mul->eqns[i]->u.n.r);
		fprintf(fp,";\n");
	}
	fprintf(fp,"FREE VARIABLES\n");
	for(i=EQN_TO_NAME_INDEX(mul->n_eqns);i<mul->n_big;++i)
	{
		fprintf(fp,"%s\n",mul->bignames[i]);
	}
	fprintf(fp,"VARIABLES\n");
	for(i=0;i<mul->n_vars;++i)
	{
		fprintf(fp,"%s = [",mul->bignames[i]);
		fprint_num(fp,mul->var_min[i]);
		fprintf(fp,",");
		fprint_num(fp,mul->var_max[i]);
		fprintf(fp,"];\n");
	}
	fprintf(fp,"PARAMETERS\n");
	for(i=0;i<mul->n_param;++i)
	{
		fprintf(fp,"%s = ", mul->bignames[mul->n_vars+i]);
		fprint_num(fp,mul->bigvals[mul->n_vars+i]);
		fprintf(fp,";\n");
	}
	fprintf(fp,"OPTIONS\n");
	for(i=0;i<mul->n_opts;++i)
	{
		fprintf(fp,"_%s = ",mul->opt_names[i]);
		fprint_num(fp,mul->opt_vals[i]);
		fprintf(fp,";\n");
	}
}

void fprintMvals(FILE *fp,Multi *mul)
{
	int i,j;

	for(i=0;i<mul->n_big;++i)
	{
		fprintf(fp,"%s = (",mul->bignames[i]);
		fprint_num(fp,mul->bigvals[mul->dimsum[i]-1]);
		for(j=1;j<mul->bigdim[i];++j)
		{
			fprintf(fp,", ");
			fprint_num(fp,mul->bigvals[mul->dimsum[i]-1-j]);
		}
		fprintf(fp,")\n");
	}
}

/* prints out the ops parameters and variables
 * misses out vbls of form ?1@?4
 */

void fprint_Mopts(FILE *fp,Multi *mul)
{
	int i;

	if( mul->error )
		return;
	for(i=0;i<mul->n_vars;++i)
	{
		if( strchr(mul->bignames[i],'@') != NULL ) continue;
		fprintf(fp,"%s = [",mul->bignames[i]);
		fprint_num(fp,mul->var_min[i]);
		fprintf(fp,",");
		fprint_num(fp,mul->var_max[i]);
		fprintf(fp,"];\n");
	}
	for(i=0;i<mul->n_param;++i)
	{
		fprintf(fp,"%s = ", mul->bignames[mul->n_vars+i]);
		fprint_num(fp,mul->bigvals[mul->n_vars+i]);
		fprintf(fp,";\n");
	}
	for(i=0;i<mul->n_opts;++i)
	{
		fprintf(fp,"_%s = ",mul->opt_names[i]);
		fprint_num(fp,mul->opt_vals[i]);
		fprintf(fp,";\n");
	}
}

void fprintMdep(FILE *fp,Multi *mul)
{
	int i,j;

	for(i=0;i<mul->n_eqns;++i)
	{
		for(j=0;j<mul->n_vars;++j)
			if( GET_DEPVAR(i,j) ) fprintf(fp,"V");
			else		      fprintf(fp," ");
		fprintf(fp,"|");
		for(j=0;j<mul->n_eqns;++j)
			if( GET_DEPEND(i,j) ) fprintf(fp,"T");
			else if(i==j)	      fprintf(fp,"\\");
			else		      fprintf(fp," ");
		fprintf(fp,"|");
		for(j=0;j<mul->n_sums;j++)
			if( GET_DEPSPEC(i,j) ) fprintf(fp,"S");
			else		      fprintf(fp," ");
		fprintf(fp,"| ");
		fprint_eqn(fp,mul->eqns[i]);
		fprintf(fp,"\n");
	}
}

void fprintMres(FILE *fp,Multi *mul)
{
	int i,j,k,pos;

	pos = 0;
	for(i=0;i<mul->n_top;++i)
	{
		fprintf(fp,"Top num %d res pos %d\n",
			i,mul->top_res_ref[i]);
		for(j=0;j<abs(mul->top_dims[i]);++j)
		{
			fprintf(fp,"%f ",mul->results[pos]);
			++pos;
		}
		fprintf(fp,"\n");
		for(k=0;k<mul->n_top_derivs[i];++k)
		{
			fprintf(fp,"Deriv no %d %s ",
				k,mul->top_deriv_names[i][k]);
			for(j=0;j<abs(mul->top_dims[i]);++j)
			{
				fprintf(fp,"%f ",mul->results[pos]);
				++pos;
			}
			fprintf(fp,"\n");
		}
	}
}

/*************** get no top equations, and dimensions *********************/

int MsetNtop(Multi *mul,int n)
{
	if(n >= MAX_NUM_TOP)
	{
		eprintf("Too many top equations %d max %d\n",
			n,MAX_NUM_TOP);
		mul->error = TRUE;
		return(FALSE);
	}
	mul->n_top = (short) n;
	return(TRUE);
}

int MsetTopDim(Multi *mul,int n,int dim)
{
	mul->top_dims[n] = (short) dim;
	return(TRUE);
}

/* set top derivatives */

int MsetNtopDerivs(Multi *mul, int n,int nDeriv)
{
	if(nDeriv > MAX_NUM_DERIVS)
	{
		eprintf("Too many derivatives\n %d max %d\n",
			nDeriv,MAX_NUM_DERIVS);
		mul->error = TRUE;
		return(FALSE);
	}
	if(n >= mul->n_top)
	{
		eprintf("Bad top equation number  %d\n",n);
		mul->error = TRUE;
		return(FALSE);
	}
	
	mul->n_top_derivs[n] = (short) nDeriv;
	return(TRUE);
}

int MsetDerivName(Multi *mul,int n,int deriv,char *name)
{
	if(deriv > MAX_NUM_DERIVS)
	{
		eprintf("Too many derivatives\n %d max %d\n",
			deriv,MAX_NUM_DERIVS);
		mul->error = TRUE;
		return(FALSE);
	}
	if(n >= mul->n_top)
	{
		eprintf("Bad top equation number  %d\n",n);
		mul->error = TRUE;
		return(FALSE);
	}
	if( name == NULL )
	{
		eprintf("NULL derivative name \n");
		mul->error = TRUE;
		return(FALSE);
	}
	if(mul->top_deriv_names[n][deriv] != NULL)
		free(mul->top_deriv_names[n][deriv] );
	COPY_STRING(mul->top_deriv_names[n][deriv],name);
	return(TRUE);
}

/* returns the number */

int MaddDerivName(Multi *mul,int n,char *name)
{
	int deriv,i;

	deriv = mul->n_top_derivs[n];
	for(i=0;i<deriv;++i)
		if( !strcmp(name, mul->top_deriv_names[n][i])) return(i);
	if( !MsetNtopDerivs(mul,n,deriv+1)) return(-1);
	if( !MsetDerivName(mul,n,deriv,name)) return(-1);
	return(deriv);
}

/****** get/set parameters *********/

int Mget_n_params(Multi *mul)
{
	return(mul->n_param);
}
char *Mget_param_name(Multi *mul,int i)
{
	return(mul->bignames[mul->n_vars+i]);
}
double Mget_param_val(Multi *mul,int i)
{
	return(mul->bigvals[mul->n_vars+i]);
}
int Mset_param_val(Multi *mul,int i,double val)
{
	mul->bigvals[mul->n_vars+i] = val;
	return(TRUE);
}

int Mset_param_val_by_name(Multi *mul,char *name,double val)
{
	int i;

	for(i=0;i<mul->n_param;++i)
	{
		if( !strcmp(name,mul->bignames[mul->n_vars+i]) )
		{
			mul->bigvals[mul->n_vars+i] = val;
			return(TRUE);
		}
	}
	eprintf("Parameter not found %s\n",name);
	mul->error = TRUE;
	return(FALSE);
}

int Mwhich_param(Multi *mul,char *name)
{
	int i;

	for(i=0;i<mul->n_param;++i)
	{
		if( !strcmp(name,mul->bignames[mul->n_vars+i]) )
		{
			return(i);
		}
	}
	return(-1);
}

/************** get/set variables */

int MgetNvars(Multi *mul)
{
	return(mul->n_vars);
}

int MsetNvars(Multi *mul,int n)
{
	int i,dif,max;

	if( mul->n_vars < n )	/* need to insert  into bignames */ 
	{
		dif = n - mul->n_vars;
		max = MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS-dif;
		if(mul->n_big >= max || n >= MAX_NUM_VARS )
		{
			eprintf("MsetNvars too many variables\n");
			mul->error = TRUE;
			return(FALSE);
		}
		for(i=mul->n_big-1;i>=mul->n_vars;--i)
		{
			mul->bignames[i+dif] = mul->bignames[i];
			mul->bigvals[i+dif]  = mul->bigvals[i];
			mul->bigdim[i+dif]  =  mul->bigdim[i];
		}
		for(i=mul->n_vars;i<n;++i)
		{
			mul->bignames[i] = NULL;
			mul->bigdim[i] = 1;
		}
		mul->n_big += dif;
	}
	else if( mul->n_vars > n ) /* remove some of the variables */
	{
		dif = mul->n_vars - n;
		for(i=n;i<mul->n_vars;++i)
		{
			if( mul->bignames[i] != NULL )
				free(mul->bignames[i]);
		}
		for(i=mul->n_vars;i<mul->n_big;++i)
		{
			mul->bignames[i-dif] = mul->bignames[i];
			mul->bignames[i] = NULL;
			mul->bigvals[i-dif]  =  mul->bigvals[i];
			mul->bigdim[i-dif]  =  mul->bigdim[i];
		}
		mul->n_big -= dif;
	}

	mul->n_vars = (short) n;
	return(TRUE);
}

int Mset_var_name(Multi *mul,int i,char *name)
{
	if(mul->bignames[i] == NULL)
		COPY_STRING(mul->bignames[i],name)
	else if( strcmp(mul->bignames[i],name ) )
	{
		free(mul->bignames[i]);
		COPY_STRING(mul->bignames[i],name);
	}
	return(TRUE);
}

char *Mget_var_name(Multi *mul,int i)
{
	return(mul->bignames[i]);
}
double Mget_var_min(Multi *mul,int i)
{
	return(mul->var_min[i]);
}
double Mget_var_max(Multi *mul,int i)
{
	return(mul->var_max[i]);
}

/* Adds a variable at end of vbl list */

int MaddVar(Multi *mul,char *name)
{
	int n;
	n = mul->n_vars;
	MsetNvars(mul,n+1);
	Mset_var_name(mul,n,name);
	Mset_var_minmax(mul,n,0.0,0.0);
	return(TRUE);
}

	
int Mset_var_minmax(Multi *mul,int i,double min,double max)
{
	mul->var_min[i]=min;
	mul->var_max[i]=max;
	return(TRUE);
}

int Mset_var_val(Multi *mul,int i,double val)
{
	mul->bigvals[i]=val;
	return(TRUE);
}

double Mget_var_val(Multi *mul,int i)
{
	return(mul->bigvals[i]);
}

int Mwhich_var(Multi *mul,char *name)
{
	int i;

	for(i=0;i<mul->n_vars;++i)
	{
		if( !strcmp(name,mul->bignames[i]) )
		{
			return(i);
		}
	}
	return(-1);
}

/* get/set options */

int Madd_opt(Multi *mul,char *name,double val)
{
	if( mul->n_opts >= MAX_NUM_OPTS )
	{
		eprintf("Sorry too many options max %d\n",MAX_NUM_OPTS);
		mul->error = TRUE;
		return(FALSE);
	}
	COPY_STRING(mul->opt_names[mul->n_opts],name);
	mul->opt_vals[mul->n_opts]=val;
	++mul->n_opts;
	return(TRUE);
}

int Mset_opt_val_by_name(Multi *mul,char *name,double val)
{
	int i;
	for(i=0;i<mul->n_opts;++i)
	{
		if( ! strcmp(name,mul->opt_names[i]) )
		{
			mul->opt_vals[i]=val;
			return(TRUE);
		}
	}
	eprintf("Could not find option %s\n",name);
	mul->error = TRUE;
	return(FALSE);
}

double Mget_opt_val_by_name(Multi *mul,char *name)
{
	int i;
	for(i=0;i<mul->n_opts;++i)
	{
		if( ! strcmp(name,mul->opt_names[i]) )
		{
			return(mul->opt_vals[i]);
		}
	}
	eprintf("Could not find option name %s\n",name);
	mul->error = TRUE;
	return(0.0);
}

/*
 * goes through variables, parameter, options to see if name matches
 * returns TRUE on match
 */

int Mmatch_opt(Multi *mul,char *name)
{
	int i;
	char *ptr;

	for(i=0;i<mul->n_vars+mul->n_param;++i)
	{
		if( !strcmp(name,mul->bignames[i]) )
			return(TRUE);
	}
	ptr = strchr(name,'_');
/*
	eprintf("Matching %s ptr %s\n",name,ptr);
*/
	if( ptr == NULL ) return(FALSE);

	for(i=0;i<mul->n_opts;++i)
		if( !strcmp(ptr+1,mul->opt_names[i]) )
			return(TRUE);

	return(FALSE);
}

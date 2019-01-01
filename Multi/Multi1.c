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
 * MbuildRPEs - to make the rpe's
 *
 * Got that? now lets get into the code.
 * Copyright (c) 1995 R.J.Morris ask me before you want to nick it
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "eqn.h"
#include "Multi.h"

#define grballoc(node) (node *) malloc( sizeof(node) )
#define COPY_STRING(target,source) {\
        target = (char *) calloc( strlen(source)+1,sizeof(char));\
        strcpy(target,source);}


/*
 * Remove entries from list of equations from n to n+m
 */

Mrm_eqns(Multi *mul,int n, int m)
{
	int i;

	for(i=n;i<mul->n_eqns-m;++i)
		mul->eqns[i] = mul->eqns[i+m];
	mul->n_eqns -= m;
}

/*
 * Find what type a name is
 * Bignames is organised
 * 0 .. n_vars-1 		Variables
 * n_vars .. n_vars + n_param-1	Parameters
 * vars + n_param .. n_big -1   Substitution
 */

int Mget_type(char *name,Multi *mul)
{
	int i;

	for(i=0;i<mul->n_big;++i)
	{
		if( !strcmp(name,mul->bignames[i] ) )
		{
			if( i >= mul->n_vars + mul->n_param )
				return(M_SUB);
			else if( i >= mul->n_vars )
				return(M_PARAM);
			return(M_VAR);
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

/*
 * Add a substitution name of the form f@x
 * this goes after f in the bignames
 */

int Madd_sub(char *name,Multi *mul)
{
	int i,j,k;
	char *ptr;

	ptr = strchr(name,'@');
	if(ptr == NULL)
	{
		eprintf("Error\007: can only add names of form f@x\n");
		return(FALSE);
	}
	for(i=0;i<mul->n_big;++i)
        {
                if( !strncmp(name,mul->bignames[i],ptr-name ) 
		 && ( mul->bignames[i][ptr-name] == '\0' 
		   || mul->bignames[i][ptr-name] == '@') )
                {
			/* got a match */

			/* check we ain't already done this before */

			for(j=i+1;j<mul->n_big;++j)
			{
				if( !strcmp(name,mul->bignames[j]) )
					return(TRUE);
			}
			if( mul->n_big >=
				MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS)
			{
			eprintf("Error\007: too many names in biglist\n");
				return(FALSE);
			}

			/* Now go through alphabetically on sufix */

			for(j=i+1;j<mul->n_big;++j)
			{
				if( strcmp(name,mul->bignames[j]) < 0)
					break;
				if( strncmp(name,mul->bignames[j],ptr-name ) 
		 		|| ( mul->bignames[j][ptr-name] != '\0' 
		   		&& mul->bignames[j][ptr-name] != '@') )
					break;
			}
				
			for(k=mul->n_big-1;k>=j;--k)
			{
				mul->bignames[k+1] = mul->bignames[k];
				mul->bigdim[k+1] = mul->bigdim[k];
				mul->bigvals[k+1] = mul->bigvals[k];
			}
			++mul->n_big;
			COPY_STRING(mul->bignames[j],name);
			mul->bigdim[j] = mul->bigdim[i];
			return(TRUE);
		}
	}
	eprintf("Error\007: name not found in Madd_sub %s\n",name);
	eprintf("Names are\n");
	for(i=0;i<mul->n_big;++i)
        {
		eprintf("%s\n",mul->bignames[i]);
	}
        return(FALSE);
}


/* Check dimensions;
 * this bit stolen from count_eqn_args
 */

int Mget_dim(eqnode *base,Multi *mul)
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
    leftcount = Mget_dim(base->u.n.l,mul);
    rightcount = Mget_dim(base->u.n.r,mul);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != rightcount )
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(leftcount);

  case '*':
    leftcount = Mget_dim(base->u.n.l,mul);
    rightcount = Mget_dim(base->u.n.r,mul);
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
    leftcount = Mget_dim(base->u.n.l,mul);
    rightcount = Mget_dim(base->u.n.r,mul);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if(leftcount != rightcount )
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(1);

  case '/':
    leftcount = Mget_dim(base->u.n.l,mul);
    rightcount = Mget_dim(base->u.n.r,mul);
    if(leftcount == 0 || rightcount == 0 ) return(0);
    if( rightcount != 1)
    {
	eprintf("Dimension mismatch for '%c' %d %d\n",
		base->op,leftcount,rightcount);
	return(0);
    }
    return(leftcount);

  case '^':
    leftcount = Mget_dim(base->u.n.l,mul);
    rightcount = Mget_dim(base->u.n.r,mul);
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
		return(Mget_dim(base->u.f.a->u.n.l,mul));
	eprintf("Whoopse! unknown operator %s\n",base->u.f.f->name);
	return(0);
    }
    else
	return(1);

  case ',':
    return( Mget_dim(base->u.n.l,mul)
		 + Mget_dim(base->u.n.r,mul));

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

/************** PUBLIC FUNCTIONS *****************/

/* initilise a multi */

int Minit(Multi *mul)
{
	int i;

	mul->n_eqns = mul->n_opts = mul->n_vars = mul->n_param = 0;
	mul->n_top = mul->n_derivs = 0;
	for(i=0;i<MAX_NUM_TOP;++i)
		mul->top_dims[i] = 0;
	for(i=0;i<MAX_NUM_TOP;++i)
		mul->n_top_derivs[i]=0;
	for(i=0;i<MAX_NUM_EQNS;++i)
		mul->eqns[i] = NULL;
	for(i=0;i<MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS;++i)
		mul->bignames[i] = NULL;
	mul->error = TRUE;
}

/* Clears out any parameters, but leaves options, and variables */
int Mclear(Multi *mul)
{
	int i;

	mul->n_eqns = mul->n_param = 0;
	for(i=0;i<MAX_NUM_EQNS;++i)
		mul->eqns[i] = NULL;
	for(i=mul->n_vars;i<MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS;++i)
		mul->bignames[i] = NULL;
	mul->error = TRUE;
}

/* read in the equations from a file, do no other processing */

int fscanMulti(FILE *fp,Multi *mul)
{
	eqnode *eqn;
	int	i;

	/* initilise multi */

	mul->error = TRUE;
	mul->n_eqns = 0;
	for(i=0;i<mul->n_eqns;++i)
		free_eqn_tree(mul->eqns[i]);

	/* read in equations */

	while(TRUE)
	{
	  eqn = fscan_eqn(fp);
	  if(eqn == NULL) break;
/*
	  if(eqn->op == '=' && !strcmp(eqn->u.n.l->u.str,"stop") )
		break;
*/
	  eval_funs(eqn);
	  mul->eqns[mul->n_eqns] = eqn;
	  mul->n_eqns++;
	}
}

int MfindOpts(Multi *mul)
{
	int	i,j;
	char	*ptr;
	eqnode	*eqn;

	/* Find and equations for dimensions, colours, normals */
	/* ones read from the equation override the previous ones */

	for(i=0;i<mul->n_eqns;++i)
	{
		eqn = mul->eqns[i];
		if( eqn->op == '='   
		 && eqn->u.n.l->op == NAME
                 && eqn->u.n.r->op == NUMBER )
                {
		    ptr = strrchr(eqn->u.n.l->u.str,'_');
		    if(ptr == NULL) continue;

		    for(j=0;j<mul->n_opts;++j)
		    {
			if( !strcmp(ptr+1,mul->opt_names[j]) )
			{
				mul->opt_vals[j] = eqn->u.n.r->u.num;
				free_eqn_tree(eqn);
				Mrm_eqns(mul,i,1);
				--i;
				break;
		    	}
                    }
		}
	}
#ifdef NOT_DEF
        mul->norm_eqns = ( mul->normals == EQN_NORM
		        || mul->normals == INOUT_NORM);
        mul->fourD_eqns = ( mul->dimension == FOUR_D );
	mul->fourD_inputs = ( mul->indim == FOUR_D  );
        mul->col_eqns = ( mul->colours == EQN_COL || mul->colours == INOUT_COL);

	/* Workout how many variables there are */

	mul->n_vars = mul->indim;
	if( mul->normals == INOUT_NORM ) mul->n_vars += 3;
	if( mul->colours == INOUT_COL ) mul->n_vars += 4;

	/* Now we need to combine the top level equations */
	/* This bits really boring and repatative so we will
		put it in a separate procedure */

	if( !McombineTop(mul) )
	{
		mul->error = TRUE;
		return(FALSE);
	}
#endif
}

/* Now we need to combine the top level equations */
/* False on error */

int McombineTop(Multi *mul)
{
    eqnode *eqn;
    int i,j;

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

	    if(mul->n_eqns < mul->top_dims[i] + i)
	    {
		eprintf("Error\007: not enougth equations specified\n");
		mul->error = TRUE;
		return(FALSE);
	    }
	    for(j=0;j<mul->top_dims[i];++j)
	    {
		if( mul->eqns[i+j]->op != '=' ||
		    mul->eqns[i+j]->u.n.l->op != NAME )

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
		Mrm_eqns(mul,i+1,1);
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
		Mrm_eqns(mul,i+1,2);
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
		Mrm_eqns(mul,i+1,3);
		break;
	    default:
		eprintf("Requested dimension %d to large\n",
			mul->top_dims[i]);
		mul->error = TRUE;
			return(FALSE);
	    }
	    mul->copy_top += mul->top_dims[i];
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
	eqnode *eqn;
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

		if( eqn->op != '=' || eqn->u.n.l->op != NAME )
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
			Mrm_eqns(mul,i,1);
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
                                fprintf(stderr,"Too many variable definitions\n");
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
			Mrm_eqns(mul,i,1);
			--i;
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
		make_variable(allnames,mul->eqns[i]->u.n.l->u.str);
	}

	mul->n_param = num_parameters(allnames);

        for(i=0; i < mul->n_param;++i)
        {
                tempname = get_parameter(allnames,i+1);
                if(i>= MAX_NUM_PARAMS )
                {
                        eprintf("Sorry too many parameters\007\n");
                        break;
                }
		COPY_STRING(mul->bignames[mul->n_big],tempname);
                mul->bigvals[mul->n_big] = 0.0;
                for(j=0;j<old_count;++j)
                        if(!strcmp(tempname,old_names[j]) )
                                mul->bigvals[mul->n_big] = old_vals[j];
                for(j=0;j<new_count;++j)
                        if(!strcmp(tempname,new_names[j]) )
                                mul->bigvals[mul->n_big] = new_vals[j];
		++mul->n_big; /* remember your a macro, */
        }

	/* add lhs of subs eqns */

	for(i=mul->n_top;i<mul->n_eqns;++i)
	{	
		COPY_STRING(mul->bignames[mul->n_big],mul->eqns[i]->u.n.l->u.str);
		++mul->n_big; /* the macros of Wimballdon common are free */
	}

}

McheckDims(Multi *mul)
{
	int i,j,dim;

	/* Check dimensions */
	
	for(i=0;i<mul->n_big;++i)
		mul->bigdim[i]=1;
	
	for(i=mul->n_eqns-1;i>=mul->n_top;--i)
	{
		dim = Mget_dim(mul->eqns[i]->u.n.r,mul);
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
		mul->bigdim[mul->n_vars + mul->n_param + i - mul->n_top] = dim;
	}

	for(i=0;i<mul->n_top;++i)
	{
		dim=Mget_dim(mul->eqns[mul->n_top-1],mul);
		if(dim != mul->top_dims[i])
                {
                        eprintf(
		"Error\007: wrong dimensions %d top level equation # %d\n",
				dim,i);
                        eprint_eqn(mul->eqns[i]);
                        eprintf(";\nDimensions are:\n");
                        for(j=0;j<mul->n_big;++j)
                                eprintf("%s dim %d\n",mul->bignames[j],mul->bigdim[j]);
                        mul->error = TRUE;
                        return(FALSE);
                }
	}


}

McalcDerivs(Multi *mul)
{
	int	i,j,k,l,flag,dim,len,eqnpos;
	char	*ptr;
	eqnode *eqn;

	/* Now we can have a play with diffs */

	mul->n_derivs = 0;
	for(i=0;i<mul->n_top;++i)
		Mconvert_diff(mul->eqns[i],mul);

	for(i=0;i<mul->n_top;++i)
	{
	    for(j=0;j<mul->n_top_derivs[i];++j)
	    {
		ptr = strrchr(mul->top_deriv_names[i][j],'@');
		if(ptr == NULL )
		{
			eqn = duplicate(mul->eqns[0]);
			Mdiff_wrt(eqn,mul->top_deriv_names[i][j],mul);
			eval_funs(eqn);
			if( mul->n_eqns >= MAX_NUM_EQNS )
			{
				eprintf("Sorry too many equations, max %d\n",
					MAX_NUM_EQNS);
				mul->error = TRUE;
				return(FALSE);
			}
			for(l=mul->n_eqns-1;l>=mul->n_top+mul->n_derivs;--l)
				mul->eqns[l+1] = mul->eqns[l];
			mul->eqns[mul->n_top+mul->n_derivs] = eqn;
			++mul->n_eqns;
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
	eprintf("Could find a previous derivative with name  %s\n",
					mul->top_deriv_names[i][k]);
				*ptr = '@';
				mul->error = TRUE;
				return(FALSE);
		    }
		    if( mul->n_eqns >= MAX_NUM_EQNS )
		    {
			eprintf("Sorry too many equations, max %d\n",
				MAX_NUM_EQNS);
			*ptr = '@';
			mul->error = TRUE;
			return(FALSE);
		    }
		    Mdiff_wrt(eqn, (ptr+1),mul);
		    *ptr = '@';
		    eval_funs(eqn);
		    for(l=mul->n_eqns-1;l>=mul->n_top+mul->n_derivs;--l)
			mul->eqns[l+1] = mul->eqns[l];
		    mul->eqns[mul->n_top+mul->n_derivs] = eqn;
		    ++mul->n_eqns;
		    ++mul->n_derivs;
		}
	    }
	} /* end for i */
	
	for(i=mul->n_top+mul->n_derivs;i<mul->n_eqns;++i)
	{
		eqnpos = i;

		Mconvert_diff(mul->eqns[i]->u.n.r,mul);
		
		/* Next do all the requests */
		/* we search through big list for this */

		/* find the ref */

		for(j=mul->n_vars+mul->n_param;j<mul->n_big;++j)
		{
			if( !strcmp(mul->eqns[i]->u.n.l->u.str,
				mul->bignames[j] ) )
				break;
		}
		/* j is now pos in biglist, i= eqnpos is pos in eqnlist */

		len = strlen(mul->eqns[eqnpos]->u.n.l->u.str );

		/* loop through bignames exiting when no longer match base */

		for(++j;j<mul->n_big;++j)
		{
			if( strncmp(mul->eqns[eqnpos]->u.n.l->u.str,
				mul->bignames[j],len ) 
			 || ( mul->bignames[j][len] != '\0' 
			   && mul->bignames[j][len] != '@' ) )
				break;

			/* This should be a request */

			/* now differentiate */
			/* have something like f@a@b@c */
			/* now f@a@b already exist and is higher up the 
			   list so use that */

			ptr = strrchr(mul->bignames[j],'@');
			*ptr = '\0';
			for(k=mul->n_top+mul->n_derivs;k<mul->n_eqns;++k)
				if(!strcmp(mul->bignames[j],
					mul->eqns[k]->u.n.l->u.str))
					break;
			*ptr= '@';
			eqn = duplicate(mul->eqns[k]);
			free(eqn->u.n.l->u.str);
			COPY_STRING(eqn->u.n.l->u.str,mul->bignames[j]);

			Mdiff_wrt(eqn->u.n.r,ptr+1,mul);
			eval_funs(eqn);

			/* and add this to the list of equations */

			if( mul->n_eqns >= MAX_NUM_EQNS )
			{
				eprintf("Error\007: too many equations\n");
				mul->error = TRUE;
				return(FALSE);
			}
			for(l=mul->n_eqns-1;l>i;--l)
				mul->eqns[l+1] = mul->eqns[l];
			mul->eqns[i+1] = eqn;
			++mul->n_eqns;
			++i;
		}
			
	}
}

McalcRPEs(Multi *mul)
{
	int  i,dim;

	/* calculate the culamative dims */

	dim = 0;
	for(i=0;i<mul->n_big;++i)
	{
		dim += mul->bigdim[i];
		mul->dimsum[i] = dim;
	}

	/* Now we can construct the rpes */

	for(i=0; i<mul->n_top+mul->n_derivs;++i)
	{
		mul->rpes[i] = make_Mvrpe(mul->eqns[i],mul);
		if(mul->rpes[i] == NULL)
			return(FALSE);
	}

	for(i=mul->n_top+mul->n_derivs;i<mul->n_eqns;++i)
	{
		mul->rpes[i] = make_Mvrpe(mul->eqns[i]->u.n.r,mul);
		if(mul->rpes[i] == NULL)
			return(FALSE);
	}
	mul->error = FALSE;
	return(TRUE);
}

fprint_multi(FILE *fp,Multi *mul)
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
		fprintf(fp,"%s = [%f,%f];\n",
			mul->bignames[i],
			mul->var_min[i],
			mul->var_max[i]);
	}
	for(i=0;i<mul->n_param;++i)
		fprintf(fp,"%s = %f;\n",
			mul->bignames[mul->n_vars+i],
			mul->bigvals[mul->n_vars+i]);
	for(i=0;i<mul->n_opts;++i)
	{
		if( mul->opt_vals[i] = rint(mul->opt_vals[i]) )
			fprintf(fp,"_%s = %.0lf;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
		else
			fprintf(fp,"_%s = %f;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
	}
}

dump_multi(FILE *fp,Multi *mul)
{
	int i,j,k;

	if( mul->error )
		fprintf(fp,"Error in Multi:\n");
	fprintf(fp,"TOP\n");
	for(i=0;i<mul->n_top;++i)
	{
		fprint_eqn(fp,mul->eqns[i]);
		fprintf(fp,";\n");
	}
	fprintf(fp,"DERIVS\n");
	k = mul->n_top;
	for(i=0;i<mul->n_top;++i)
	    for(j=0;j< mul->n_top_derivs[i];++j)
	    {
		fprintf(fp,"#%d@%s = ",i,
			mul->top_deriv_names[i][j]);
		fprint_eqn(fp,mul->eqns[k]);
		fprintf(fp,";\n");
		++k;
	    }
	fprintf(fp,"SUBSTITUTIONS\n");	
	for(i=mul->n_top+mul->n_derivs;i<mul->n_eqns;++i)
	{
		j = mul->n_vars+mul->n_param + 
			(i - mul->n_top - mul->n_derivs );
		fprint_eqn(fp,mul->eqns[i]);
		fprintf(fp,";\n");
	}
	fprintf(fp,"VARIABLES\n");
	for(i=0;i<mul->n_vars;++i)
	{
		fprintf(fp,"%s = [%f,%f];\n",
			mul->bignames[i],
			mul->var_min[i],
			mul->var_max[i]);
	}
	fprintf(fp,"PARAMETERS\n");
	for(i=0;i<mul->n_param;++i)
		fprintf(fp,"%s = %f;\n",
			mul->bignames[mul->n_vars+i],
			mul->bigvals[mul->n_vars+i]);
	fprintf(fp,"OPTIONS\n");
	for(i=0;i<mul->n_opts;++i)
	{
		if( mul->opt_vals[i] = rint(mul->opt_vals[i]) )
			fprintf(fp,"_%s = %.0lf;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
		else
			fprintf(fp,"_%s = %f;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
	}
}
fprint_Mopts(FILE *fp,Multi *mul)
{
	int i;

	if( mul->error )
		return;
	for(i=0;i<mul->n_vars;++i)
	{
		fprintf(fp,"%s = [%f,%f];\n",
			mul->bignames[i],
			mul->var_min[i],
			mul->var_max[i]);
	}
	for(i=0;i<mul->n_param;++i)
		fprintf(fp,"%s = %f;\n",
			mul->bignames[mul->n_vars+i],
			mul->bigvals[mul->n_vars+i]);
	for(i=0;i<mul->n_opts;++i)
	{
		if( mul->opt_vals[i] = rint(mul->opt_vals[i]) )
			fprintf(fp,"_%s = %.0lf;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
		else
			fprintf(fp,"_%s = %f;\n",
				mul->opt_names[i],
				mul->opt_vals[i]);
	}
}

Meval(Multi *mul)
{
	int i,j,k;
	double *ptr;

	if(mul->error)
	{
		eprintf("Error\007: bad equations while trying to evaluate\n");
		return;
	}
	for(i=mul->n_eqns-1;i>=mul->n_top;--i)
	{
		ptr = Meval_vrpe(mul->rpes[i],mul->bigvals);
		j = i-mul->n_top + mul->n_vars+mul->n_param;
		for(k=0;k<mul->bigdim[j];++k)
		{
			mul->bigvals[mul->dimsum[j]-k-1] = *(ptr+k);
		}
	}
#ifdef NOT_DEF
	if(mul->deriv)
	{
		for(i=0;i<mul->indim;++i)
		{
			ptr = Meval_vrpe(mul->rpes[i+1],mul->bigvals);
			mul->diff[i][0] = *(ptr);
			mul->diff[i][1] = *(ptr+1);
			mul->diff[i][2] = *(ptr+2);
			if(mul->fourD_eqns)
				mul->diff[i][3] = *(ptr+3);
		}
	}
	ptr = Meval_vrpe(mul->rpes[0],mul->bigvals);
	mul->main[0] = *(ptr);
	mul->main[1] = *(ptr+1);
	mul->main[2] = *(ptr+2);
	if(mul->fourD_eqns)
		mul->main[3] = *(ptr+3);
#endif
}

/* get no top equations, and dimensions */

int MsetNtop(Multi *mul,int n)
{
	if(n >= MAX_NUM_TOP)
	{
		eprintf("Too many top equations %d max %d\n",
			n,MAX_NUM_TOP);
		mul->error = TRUE;
		return(FALSE);
	}
	mul->n_top = n;
	return(TRUE);
}

int MsetTopDim(Multi *mul,int n,int dim)
{
	mul->top_dims[n]=dim;
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
	
	mul->n_top_derivs[n] =  nDeriv;
	return(TRUE);
}

int MsetDerivName(Multi *mul,int n,int deriv,char *name)
{
	char *ptr;

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
	
/* get/set parameters */

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
}

int Mset_param_val_by_name(Multi *mul,char *name,double val)
{
	int i;

	for(i=0;i<mul->n_param;++i)
	{
		if( !strcmp(name,mul->bignames[mul->n_vars+i]) )
		{
			mul->bigvals[mul->n_vars+i] = val;
			return;
		}
	}
	eprintf("Parameter not found %s\n",name);
}

/* get/set variables */

int MsetNvars(Multi *mul,int n)
{
	int i,dif,max;

	if( mul->n_vars < n )	/* need to insert  into bignames */ 
	{
		dif = n - mul->n_vars;
		max = MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS-dif;
		if(mul->n_big >= max) mul->n_big = max-1;
		for(i=mul->n_big-1;i>=mul->n_vars;--i)
		{
			mul->bignames[i+dif] = mul->bignames[i];
			mul->bigvals[i+dif]  = mul->bigvals[i];
		}
		for(i=mul->n_vars;i<n;++i)
			mul->bignames[i] = NULL;
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
		}
		mul->n_big -= dif;
	}

	mul->n_vars = n;
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
int Mset_var_minmax(Multi *mul,int i,double min,double max)
{
	mul->var_min[i]=min;
	mul->var_max[i]=max;
}
int Mset_var_val(Multi *mul,int i,double val)
{
	mul->bigvals[i]=val;
}

/* get/set options */

int Madd_opt(Multi *mul,char *name,double val)
{
	if( mul->n_opts >= MAX_NUM_OPTS )
	{
		eprintf("Sorry too many options max %d\n",MAX_NUM_OPTS);
		return;
	}
	COPY_STRING(mul->opt_names[mul->n_opts],name);
	mul->opt_vals[mul->n_opts]=val;
	++mul->n_opts;
}
int Mset_opt_val_by_name(Multi *mul,char *name,double val)
{
	int i;
	for(i=0;i<mul->n_opts;++i)
	{
		if( ! strcmp(name,mul->opt_names[i]) )
		{
			mul->opt_vals[i]=val;
			return;
		}
	}
	eprintf("Could not find option %s\n",name);
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
	ptr = strrchr(name,'_');
/*
	eprintf("Matching %s ptr %s\n",name,ptr);
*/
	if( ptr == NULL ) return(FALSE);
	if( !strcmp(ptr+1,"normals") 
	 || !strcmp(ptr+1,"colours")
	 || !strcmp(ptr+1,"dimension")
	 || !strcmp(ptr+1,"indim"))
	 	return(TRUE);

	for(i=0;i<mul->n_opts;++i)
		if( !strcmp(ptr+1,mul->opt_names[i]) )
			return(TRUE);

	return(FALSE);
}

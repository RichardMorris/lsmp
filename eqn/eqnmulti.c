
/*
 * Test program to input some equations and calc derivatives
 */

#include <stdio.h>
#include "eqn.h"

#define SIZE 10

main()
{
	eqn_node *eqns[SIZE];
	eqn_node *diffs[SIZE][SIZE];
	char	 *names[SIZE];
	double	 vals[SIZE];
	eqn_names *name_list;
	int	 *rpes[SIZE];
	int	 *diff_rpes[SIZE][SIZE];
	double	 diff_vals[SIZE][SIZE];
	double	 sum;
	int	 i,j,num_eqns,num_params;
	
	/* Read in number of eqns and eqns  */

	scanf("%d",&num_eqns);

	for(i=0;i<num_eqns;++i)
	{
		eqns[i] = scan_eqn();
	}

	/* Now construct name list  first get names on left and name list */

	for(i=0; i < num_eqns; ++i)
	{
		names[i] = eqns[i]->u.n.l->u.str;
		printf("names[%d] = %s\n",i,names[i]);
		name_list = add_eqn_names(name_list,eqns[i]);
		make_variable(name_list,names[i]);
	}

	/* Use x as the variable */

	names[num_eqns] = "x";
	make_variable(name_list,names[num_eqns]);

	/* Now find the other variables */

	num_params = num_parameters(name_list);
	for(i=0; i < num_params ; ++i )
	{
		names[i+num_eqns+1] = get_parameter(name_list,i+1);
	}

	/* can now construct diff's and rpe's */

	for(i=0;i<num_eqns;++i)
	{
		rpes[i] = make_rpe(eqns[i]->u.n.r,num_eqns+num_params+1,names);
		for(j=i+1;j<num_eqns+1;++j)
		{
			diffs[i][j] = duplicate(eqns[i]->u.n.r);
			diff_wrt(diffs[i][j],names[j]);
			diff_rpes[i][j] = make_rpe(diffs[i][j],
				num_eqns+num_params+1, names);
		}
	}

	

	/*
	 * So we have all the info we need.
	 * Time for evaluation.
	 */

	while( 1 )
	{
		printf("\nx ");
		for(i=0;i<num_params;++i)
			printf("%s ",names[num_eqns+i+1]);
		printf(" = ?\n");

		for(i=0;i<=num_params;++i)
			scanf("%lf",&vals[i+num_eqns]);

		
	
	/* First we evaluate the actual equation */

	for(i=num_eqns -1; i>=0; --i)
	{
		vals[i] = eval_rpe(rpes[i],vals);
		printf("%s = %f\n",names[i],vals[i]);
	}

	/* Now calc deriv's */

	for(i=0;i<num_eqns;++i)
	{
		for(j=i+1;j<num_eqns+1;++j)
		{
			diff_vals[i][j] = eval_rpe(diff_rpes[i][j],vals);
			printf("d%s/d%s = %lf\n",names[i],names[j],diff_vals[i][j]);
		}
	}

	/* Here comes the fun bit: multiplying out the deriv mat */

	for(i=num_eqns-1;i>=0;--i)
	{
		sum = 0;
		for(j=i+1;j<=num_eqns;++j)
			sum += diff_vals[i][j];
		printf("sum %f\n",sum);

		for(j=0;j<i;++j)
		{
			diff_vals[j][i] *= sum;
			printf("d%s/d%s = %lf\n",
				names[j],names[i],diff_vals[j][i]);
		}
	}

	} /* End while */
}

/*
 * File:	converge.c
 * Function:	convergence routine
 * Author;	Richard Morris
 * Date		22/5/93
 */

#include <stdio.h>
#include "../CVcommon.h"
#ifndef CVcommon_h
#include <hpoint3.h>
#endif
#include <math.h>

/*
#define PRINT_VALS
#define PRINT_ERRS
*/

extern double	tolerance;
extern int	max_itt;

/*
 * Function:	converge
 * Action:	given two points pt1,pt2 with values val1,val2
 *		find a point solpt with |value|<tolerance with
 *		lambda_sol * pt1 + (1-lambda_sol ) * pt2
 */

converge(pt1,val1,pt2,val2,solpt,lambda_sol,fun)
HPoint3 *pt1,*pt2,*solpt;
double  val1,val2,*lambda_sol;
double	(*fun)();
{
	HPoint3	block1,block2,block3,block4;
	HPoint3 *low,*mid,*high,*temp,*sol;
	double	lowlam,midlam,highlam,sollam;
	double	lowval,midval,highval,solval;
	double	lambda;
	int	count;

	low = &block1; high = &block2; mid = &block3;
	sol = &block4;

	/* So what happens if either val1 or val2 are NaN */

	if(val1<0.0 && val2 > 0.0 )
	{
		lowval = val1;
		highval = val2;
		lowlam = 1.0;
		highlam = 0.0;
		HPt3Copy(pt2,high);
		HPt3Copy(pt1,low);
	}
	else
	{
		lowval = val2;
		highval = val1;
		lowlam = 0.0;
		highlam = 1.0;
		HPt3Copy(pt1,high);
		HPt3Copy(pt2,low);
	}

#ifdef PRINT_VALS
	fprintf(stderr,"lowval %f highval %f\n",lowval,highval);
#endif

	count = 0;

	/*	For the convergence routine we use a 
		highbrid method finding where the cord
		crosses 0 and the mid point. This is
		expensive as there are two function
		evaluations, but ensures that the interval
		is always reduced.
	*/

	while(1)
	{
		lambda = (float) highval /(highval - lowval);

		HPt3LinSum(lambda,low,(1.0-lambda),high,sol);
		solval = fun(sol->x,sol->y,sol->z);
		sollam = lambda*lowlam + (1.0-lambda)*highlam;

		HPt3LinSum(0.5,low,0.5,high,mid);
		midval = fun(mid->x,mid->y,mid->z);
		midlam = 0.5 * (lowlam + highlam);

		if(solval == solval && fabs(solval)< tolerance ) break;

		if(midval == midval && fabs(midval) < tolerance )
		{
			solval = midval;
			sollam = midlam;
			temp = sol; sol = mid; mid = temp;
			break;
		}

		if( ++count > max_itt ) break;

		if( midval < 0.0 )
		{
			if( lambda < 0.5 )
			{
			if( solval > 0.0 )
			{
				highval = solval; highlam = sollam;
				lowval = midval; lowlam = midlam;
				temp = high; high = sol; sol = temp;
				temp = low; low = mid; mid =temp;
			}
			else
			{
				lowval = solval; lowlam = sollam;
				temp = low; low = sol; sol =temp;
			}
			}
			else
			{
			if( solval > 0.0 )
			{
#ifdef PRINT_ERRS
		fprintf(stderr,"More than one root on edge\n");
		fprintf(stderr,"pt1 %f %f %f val %f\n",
			pt1->x,pt1->y,pt1->z,
			fun(pt1->x,pt1->y,pt1->z) );
		fprintf(stderr,"pt2 %f %f %f val %f\n",
			pt2->x,pt2->y,pt2->z,
			fun(pt2->x,pt2->y,pt2->z) );
		fprintf(stderr,"high %f %f %f val %f\n",
			high->x,high->y,high->z, highval);
		fprintf(stderr,"low %f %f %f val %f\n",
			low->x,low->y,low->z, lowval);
		fprintf(stderr,"sol %f %f %f val %f\n",
			sol->x,sol->y,sol->z, solval);
		fprintf(stderr,"mid %f %f %f val %f\n",
			mid->x,mid->y,mid->z, midval);
#endif

				highval = solval; highlam = sollam;
				temp = high; high = sol; sol = temp;					}
			else
			{
				lowval = midval; lowlam = midlam;
				temp = low; low = mid; mid =temp;
			}
			}
		}
		else
		{
			if( lambda < 0.5 )
			{
			if( solval > 0.0 )
			{
				highval = midval; highlam = midlam;
				temp = high; high = mid; mid = temp;
			}
			else
			{
#ifdef PRINT_ERRS
		fprintf(stderr,"More than one root on edge!\n");
		fprintf(stderr,"pt1 %f %f %f val %f\n",
			pt1->x,pt1->y,pt1->z,
			fun(pt1->x,pt1->y,pt1->z) );
		fprintf(stderr,"pt2 %f %f %f val %f\n",
			pt2->x,pt2->y,pt2->z,
			fun(pt2->x,pt2->y,pt2->z) );
		fprintf(stderr,"high %f %f %f val %f\n",
			high->x,high->y,high->z, highval);
		fprintf(stderr,"low %f %f %f val %f\n",
			low->x,low->y,low->z, lowval);
		fprintf(stderr,"sol %f %f %f val %f\n",
			sol->x,sol->y,sol->z, solval);
		fprintf(stderr,"mid %f %f %f val %f\n",
			mid->x,mid->y,mid->z, midval);
#endif

				lowval = solval; lowlam = sollam;
				temp = low; low = sol; sol =temp;
			}
			}
			else
			{
			if( solval > 0.0 )
			{
				highval = solval; highlam = sollam;
				temp = high; high = sol; sol = temp;					}
			else
			{
				highval = midval; highlam = midlam;
				temp = high; high = mid; mid = temp;
				lowval = solval; lowlam = sollam;
				temp = low; low = sol; sol =temp;
			}
			}
		}
#ifdef NOT_DEF
		if(solval<0.0)
		{	temp = low; low = sol; sol = temp;
			lowval = solval;
			lowlam = sollam;
		}
		else
		{	temp = high; high = sol; sol = temp;
			highval = solval;
			highlam = sollam;
		}
#endif
#ifdef PRINT_VALS
		fprintf(stderr,"sol %f %f %f val %f\n",
			sol->x,sol->y,sol->z, solval);
#endif
	}
#ifdef PRINT_ERRS
	if(count > max_itt )
	{
		fprintf(stderr,"Error: couldn't converge\n");
		fprintf(stderr,"pt1 %f %f %f val %f\n",
			pt1->x,pt1->y,pt1->z,
			fun(pt1->x,pt1->y,pt1->z) );
		fprintf(stderr,"pt2 %f %f %f val %f\n",
			pt2->x,pt2->y,pt2->z,
			fun(pt2->x,pt2->y,pt2->z) );
		fprintf(stderr,"sol %f %f %f val %13.11lf\n",
			sol->x,sol->y,sol->z, solval);
	}
#endif
#ifdef PRINT_VALS
	fprintf(stderr,"\n");
#endif

	HPt3Copy(sol,solpt);
	*lambda_sol = sollam;
}

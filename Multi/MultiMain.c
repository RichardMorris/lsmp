/*
 * Test program for the multi routine
 *
 * read in file specified on arg line 
 * and evaluate
 *
 * Got that? now lets get into the code.
 * Copyright (c) 1995 R.J.Morris ask me before you want to nick it
 */

#include <stdio.h>
#include <string.h>
#include "eqn.h"
#include "Multi.h"

#define grballoc(node) (node *) malloc( sizeof(node) )
#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}


eqn_funs *funlist = NULL;       /* list of functions */

void init_funs()
{
	funlist = add_standard_functions(NULL);
	set_input_functions(funlist);
}

/* Sets the default config for the mul object */

void init_mul(Multi *mul)
{
	Minit(mul);
	MsetNvars(mul,3);
/*
	Mset_var_name(mul,0,"x");
	Mset_var_minmax(mul,0,-2.0,2.0);
	Mset_var_name(mul,0,"y");
	Mset_var_minmax(mul,0,-2.0,2.0);
*/
	Mset_var_name(mul,0,"?1");
	Mset_var_name(mul,1,"?2");
	Mset_var_name(mul,2,"?3");
/*
	Mset_var_name(mul,3,"?4");
	Mset_var_name(mul,4,"?5");
	Madd_opt(mul,"precision",1);
	Madd_opt(mul,"steps1",15);
	Madd_opt(mul,"steps2",15);
	Madd_opt(mul,"clipmax",1000);
*/
}

void eval(Multi *mul)
{
	int     i,j;
	int     xdim,ydim;
	float   xsize,ysize,dx,dy;
	double  x,y;
	register double xmin,xmax,ymin,ymax;

	xmin = Mget_var_min(mul,0);
	xmax = Mget_var_max(mul,0);
	xdim = Mget_opt_val_by_name(mul,"steps1");
	ymin = Mget_var_min(mul,1);
	ymax = Mget_var_max(mul,1);
	ydim = Mget_opt_val_by_name(mul,"steps2");
	xsize = xmax-xmin;
	ysize = ymax-ymin;
	dx = xsize/(xdim-1);
	dy = ysize/(ydim-1);
printf("%f %f %d %f %f %d\n",xmin,xmax,xdim,ymin,ymax,ydim);
	for (j=0, y = ymin; j<ydim; ++j, y += dy)
	{
	 for (i=0, x = xmin; i<xdim; ++i, x += dx)
	 {
		/* Set the variables */

		Mset_var_val(mul,0,x);
		Mset_var_val(mul,1,y);

		/* Evaluate equations */

		MstartEval(mul);
		MevalTop(mul,0);
/*
		if( mul->results[0] != mul->results[0] )
			fprintMvals(stderr,mul);
*/
/*
printf("res %f at (%f,%f)\n",mul->results[0],x,y);
*/
	  }
	}
}

void callback(short  *prov_req,double *vals)
{
	int i;

printf("Callback\n");
	for(i=0;i<5;++i)
	{
		if(prov_req[i]<0) printf("req %d\n",i);
		else if(prov_req[i]>0) printf("prov  %d val %f\n",i,vals[i]);
	}
	if( prov_req[0] < 0 )
		vals[0] = vals[1]*vals[1];
	else if( prov_req[3] < 0 )
		vals[3] = vals[1]*2.0;
	else if(prov_req[4] < 0 )
		vals[4] = 2.0;
	else
	{
		printf("Bad req\n");
		exit(1);
	}
}

main(int argc,char **argv)
{
	int i;
	Multi mul;
	FILE *fp;

	funlist = add_standard_functions(NULL);
	set_input_functions(funlist);

	init_mul(&mul);	/* sets default vars etc Nvars = 5 */
/*
	Mset_opt_val_by_name(&mul,"steps1",atoi(argv[2]));
	Mset_opt_val_by_name(&mul,"steps2",atoi(argv[3]));
*/
	fp = fopen(argv[1],"r");
	fscanMulti(fp,&mul);

	fclose(fp);
	MfindOpts(&mul);
	MsetNtop(&mul,1);
/*
	MsetNtop(&mul,2);
	MsetTopDim(&mul,1,4);
*/
	MsetTopDim(&mul,0,1);
	McombineTop(&mul);
	MfindNames(&mul);
	MsetNtopDerivs(&mul,0,0);
/*
	MsetNtopDerivs(&mul,1,0);
	MsetDerivName(&mul,0,0,Mget_var_name(&mul,3));
	MsetDerivName(&mul,0,1,Mget_var_name(&mul,4));
*/
/*
	MsetNtopDerivs(&mul,0,0);
	if( atoi(argv[3]) == 1 )
	{
		MsetNtopDerivs(&mul,0,2);
	}
*/
	McheckDims(&mul);
	McalcDerivs(&mul);
	McalcRPEs(&mul);
	dump_multi(stdout,&mul);
	fprintMdep(stdout,&mul);
	if(mul.error) exit(0);
	Mset_var_val(&mul,0,100.0);
/*
	Mset_var_val(&mul,1,2000.0);
*/
	/* Evaluate equations */
for(i=0;i<1;++i)
{
	MstartEval(&mul);
	MevalTopCB(&mul,0,callback);
}
/*
	eval(&mul);
*/
	fprintMvals(stdout,&mul);
	fprintMres(stdout,&mul);
return;
/*
	fCprintMulti(stdout,&mul);
*/
}

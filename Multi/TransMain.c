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
#include "MTrans.h"

#define grballoc(node) (node *) malloc( sizeof(node) )
#define COPY_STRING(target,source) {\
        target = (char *) calloc( strlen(source)+1,sizeof(char));\
        strcpy(target,source);}


eqn_funs *funlist = NULL;       /* list of functions */

init_funs()
{
	funlist = add_standard_functions(NULL);
        set_input_functions(funlist);
}


eval(Multi *mul)
{
        int     i,j;
        int     xdim,ydim;
        float   val,xsize,ysize,dx,dy,clipmax;
        double  x,y,X,Y,Z,W,L,M,N,R,G,B,A,a,b,c,d,e,f,len;
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

MTrans *global_trans;

void callback(short  *prov_req,double *vals)
{
	int i;
/*
printf("Callback\n");
*/
	MTstdCallback(global_trans,prov_req,vals);
}

main(int argc,char **argv)
{
	int i;
	double x,y;
	Multi op,ingr;
	MTrans	trans;
	FILE *fp;

	funlist = add_standard_functions(NULL);
        set_input_functions(funlist);

	global_trans = &trans;
	Minit(&op);
	fp = fopen(argv[1],"r");
	fscanMulti(fp,&op);
	fclose(fp);
	MfindOpts(&op);
	MsetNtop(&op,1);
	MsetTopDim(&op,0,1);
        McombineTop(&op);

	MsetNvars(&op,3);	/* i.e. ?1,?2,?3 : ?4,?5 */
	Mset_var_name(&op,0,"?1");
	Mset_var_name(&op,1,"?2");
	Mset_var_name(&op,2,"?3");
/*
	Mset_var_name(&op,3,"?4");
	Mset_var_name(&op,4,"?5");
*/
	MfindNames(&op);
	MsetNtopDerivs(&op,0,0);
/*
	MsetNtopDerivs(&op,0,2);
	MsetDerivName(&op,0,0,"?4");
	MsetDerivName(&op,0,1,"?5");
*/
	McheckDims(&op);
	McalcDerivs(&op);
	McalcRPEs(&op);
	fprintf(stdout,"\nThe Op\n");
	dump_multi(stdout,&op);
	fprintMdep(stdout,&op);

	Minit(&ingr);
	fp = fopen(argv[2],"r");
	fscanMulti(fp,&ingr);
	MfindOpts(&ingr);
	MsetNtop(&ingr,1);
	MsetTopDim(&ingr,0,1);
	McombineTop(&ingr);

	MsetNvars(&ingr,2);
	Mset_var_name(&ingr,0,"x");
	Mset_var_name(&ingr,1,"y");
/*
*/
	MfindNames(&ingr);
	McheckDims(&ingr);
/*
	fprintf(stdout,"\nThe Target Before\n");
	dump_multi(stdout,&ingr);
*/

	/* We now need to do the translations to work out which derivs */

	MTdefine(&trans,&op,&ingr);
/*
	MTsetVarTop(&trans,0,0,0);
	MTsetVarTop(&trans,1,0,1);
	MTsetVarTop(&trans,2,0,2);
	MTsetVarVar(&trans,3,0);
	MTsetVarVar(&trans,4,1);
*/
	MTsetVarTop(&trans,0,0,0);
	MTsetVarVar(&trans,1,0);
	MTsetVarVar(&trans,2,1);

	MTcalcVarTrans(&trans);
	McalcDerivs(&ingr);
	McalcRPEs(&ingr);
	fprintf(stdout,"\nThe Target After\n");
	dump_multi(stdout,&ingr);
	dumpMTrans(stdout,&trans);
	fprintMvals(stdout,global_trans->op);

	/* Now the evaluation step */
/*
	x = 1.0; y = 1.0;
	    for( y = -1.0; y <= 1.0; y += 1.0 )
*/
	for( x = -1.0; x <= 1.0; x += 1.0 )
	    {
		MstartEval(&ingr);
		MstartEval(&op);
		printf("\nx %f y %f\n",x,y);
		Mset_var_val(&ingr,1,x);
		Mset_var_val(&op,1,x);
/*
		Mset_var_val(&ingr,1,y);
		Mset_var_val(&op,4,y);
		fprintMvals(stdout,&ingr);
*/
		MTevalForTop(&trans,0);
/*
		MTevalForTopDeriv(&trans,0,0);
		MTevalForTopDeriv(&trans,0,1);
*/
/*
		fprintMres(stdout,&ingr);
		fprintf(stdout,"now copy\n");
*/
		MTcopyVars(&trans);
/*
		fprintMvals(stdout,&op);
*/
		MevalTopCB(&op,0,callback);
/*
		MevalTopDeriv(&op,0,0);
		MevalTopDeriv(&op,0,1);
*/
		fprintMres(stdout,&op);
	    }

}

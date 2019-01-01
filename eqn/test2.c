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
#include "eqn.h"

/*
#define TEST_DIFF
#define TEST_POW_SPEED
#define TEST_SUB
#define TEST_RPE
*/
#define PRINT_FUNS
#define USE_FUNCT
#define TEST_EVAL_FUNS

double x,y;
char *names[3] = {"x","y","z"};

main()
{
	eqnode *main_eqn, *sub_eqn;
	double vals[3],x,y,z;
	int i,num;
	char *str;
	int *rpe;

	eqn_funs *funlist = NULL;

	printf("Input equation\n");
	main_eqn = scan_eqn();
	display_eqn(main_eqn,0);

#ifdef USE_FUNCT
	funlist = add_standard_functions(funlist);
#ifdef PRINT_FUNS
	fprint_funs(stdout,funlist);
#endif
	use_functions(main_eqn,funlist);
#endif
#ifdef TEST_SUB
	sub_eqn = scan_eqn();
	print_eqn(sub_eqn);
	substitute(main_eqn,sub_eqn);
	display_eqn(main_eqn,0);
	print_eqn(main_eqn);
	printf("\n");
#endif
#ifdef TEST_RPE
	rpe = make_rpe(main_eqn,3,names);
	print_rpe(rpe,names);
	printf("\n");
#ifdef INPUT_VARS
	do
	{
		printf("Input x,y,z\n");
		if( scanf("%lf %lf %lf",&x,&y,&z) == EOF ) break;
		vals[0] = x;
		vals[1] = y;
		vals[2] = z;
		printf(" = %lf\n", eval_rpe(rpe,vals));
	} while (1);
#else
        for(x=0.0;x<=1.0;x+=0.01)
	{
		vals[0] = x;
		eval_rpe(rpe,vals);
	}
#endif

#endif

#ifdef TEST_DIFF
	diff_wrt(main_eqn,"x");
	clean_eqn(main_eqn);
	display_eqn(main_eqn,0);
	printf("\n");

	diff_wrt(main_eqn,"x");
	clean_eqn(main_eqn);
	display_eqn(main_eqn,0);
	printf("\n");

	diff_wrt(main_eqn,"x");
	clean_eqn(main_eqn);
	display_eqn(main_eqn,0);
	printf("\n");

	diff_wrt(main_eqn,"x");
	clean_eqn(main_eqn);
	display_eqn(main_eqn,0);
	printf("\n");

#endif
#ifdef TEST_EVAL_FUNS
	print_eqn(main_eqn);
	eval_funs(main_eqn);
	print_eqn(main_eqn);
#endif
#ifdef TEST_POW_SPEED
	prod1();
	prod2();
	prod3();
	prod4();
	prod5();
	pow1();
	pow2();
	pow3();
	pow4();
	pow5();
#endif
}
#ifdef TEST_POW_SPEED

pow1(){ for(x=0.0;x<=1.0;x+=0.01) y = pow(x,1.0); }
pow2(){ for(x=0.0;x<=1.0;x+=0.01) y = pow(x,2.0); }
pow3(){ for(x=0.0;x<=1.0;x+=0.01) y = pow(x,3.0); }
pow4(){ for(x=0.0;x<=1.0;x+=0.01) y = pow(x,4.0); }
pow5(){ for(x=0.0;x<=1.0;x+=0.01) y = pow(x,10.0); }

prod1(){ for(x=0.0;x<=1.0;x+=0.01) y = x; }
prod2(){ for(x=0.0;x<=1.0;x+=0.01) y = x * x; }
prod3(){ for(x=0.0;x<=1.0;x+=0.01) y = x * x * x; }
prod4(){ for(x=0.0;x<=1.0;x+=0.01) y = x * x * x * x; }
prod5(){ for(x=0.0;x<=1.0;x+=0.01) y = x * x * x * x * x * x * x * x * x * x; }
#endif


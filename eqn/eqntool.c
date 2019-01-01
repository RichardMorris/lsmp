/*
 * File:	eqntool.c
 * Action:	interactive equation handling
 * Date:	9/4/93
 * Author:	Richard Morris
 */
/*
#define SUS_SYS
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eqn.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define MAX_EQNS 50

/*
*/
#define USE_VRPE

eqnode *eqns[MAX_EQNS];		/* An array of equations */
eqnode *subeqns[MAX_EQNS];	/* the substitution equations */
int	num_eqns;		/* The number of equations */
eqn_funs *fun_list;		/* A list of function definitions */

int	expand_flag = FALSE;	/* Perform expansion? */
int	diff_flag = FALSE;	/* Perform differentiation */
int	clean_flag = TRUE;	/* Clean up all equations */
int	funct_flag = TRUE;	/* Use the function definitions */
int	sub_flag = FALSE;	/* Try substitutions */

/*
 * Function:	process_eqn
 * Action:	for each equation read in we want to do certain actions
 *		which are caried out here.
 */

void process_eqn(eqn)
eqnode *eqn;
{
	int i,n;
	eqn_names *namelist = NULL;

	if(clean_flag)  eval_funs(eqn); /* remove instances like cos(Pi) */
	if(expand_flag) expand(eqn);	/* expand the equation */
	if(clean_flag)  eval_funs(eqn);

			/* Now see if its a sub */
	if( sub_flag && eqnop(eqn) == '=' && eqnop(eqnl(eqn)) == NAME 
		&& eqnop(eqnr(eqn)) != INTERVAL )
	{
		/* an asignment see if name in other eqns */

	        for(i=0;i<num_eqns;++i)
               		namelist = add_eqn_names(namelist,eqns[i]);

		n = num_parameters(namelist);
		for(i=0;i<n;++i)
		{
			if(!strcmp(get_parameter(namelist,i+1),
				eqnname(eqnl(eqn)) ) )
			{
				/* matches now do substitution */

				for(i=0;i<num_eqns;++i)
				{
					substitute(eqns[i],eqn);
				}
				return;
			}
		}
	}
	else if( sub_flag && eqnop(eqn) == '=' && eqnop(eqnl(eqn)) == '*' 
		&& eqnop(eqnl(eqnl(eqn))) == NAME
		&& eqnop(eqnr(eqn)) != INTERVAL )
	{
		/* an asignment of type f(x) see if name in other eqns */

	        for(i=0;i<num_eqns;++i)
               		namelist = add_eqn_names(namelist,eqns[i]);

		n = num_parameters(namelist);
		for(i=0;i<n;++i)
		{
			if(!strcmp(get_parameter(namelist,i+1),
				eqnname(eqnl(eqnl(eqn)))) )
			{
				/* matches now do substitution */

				for(i=0;i<num_eqns;++i)
				{
					substitute(eqns[i],eqn);
				}
				return;
			}
		}
	}
			
	eqns[num_eqns++] = eqn;		/* add this to list of equations */
}

/*
 * Function:	ask
 * action:	called with the -A flag, repeatedly asks for all the
 *		values and then prints out the values of the rpe's
 */

void ask()
{
	eqn_names *namelist = NULL;
	int	*rpes[MAX_EQNS];
	int	num_args[MAX_EQNS];
	char	*namearray[MAX_EQNS];
	double	vals[MAX_EQNS],*ptr;
	int	i,j,n,failed;

/* get the names from all the equations */

	for(i=0;i<num_eqns;++i)
		namelist = add_eqn_names(namelist,
				eqns[i]);

/* find the number of names */

	n = num_parameters(namelist);

/* construct an array of the names */

	for(i=0;i<n;++i)
	{
		namearray[i] = get_parameter(namelist,i+1);
	}
	
/* Build the reverse polish equations(rpe) */

	for(i=0;i<num_eqns;++i)
	{
		if(clean_flag) eval_ops(eqns[i]);
#ifdef USE_VRPE
		num_args[i] = count_eqn_args(eqns[i]);
		rpes[i] = make_vrpe(eqns[i],n,namearray);
#else
		rpes[i] = make_rpe(eqns[i],n,namearray);
#endif
	}

/* Loop till EOF */

	while(1)
	{
	/* Print out the names */
		for(i=0;i<n;++i)
		{
			printf("%s ",namearray[i]);
		}
		printf("?\n");
		failed = FALSE;
	/* Input the values */
		for(i=0;i<n;++i)
		{
			if(scanf("%lf",&vals[i]) == EOF)
			{
				failed = TRUE;
				break;
			}
		}
		if(failed) break;

	/* evaluate the rpe with these values */
		for(i=0;i<num_eqns;++i)
		{
#ifdef USE_VRPE
			ptr = eval_vrpe(rpes[i],vals);
			for(j=0;j<num_args[i];++j)
			{
				printf("%f ",*(ptr+j));
			}
#else
			printf("%f ",
			eval_rpe(rpes[i],vals));
#endif
		}
		printf("\n");
	}
	free_eqn_names(namelist);
}

/*
 * Function:	conv_poly
 * Action:	convert each equation into a polynomial and print it out
 */

void conv_poly()
{
	eqn_names *namelist = NULL;
	char	*namearray[MAX_EQNS];
	int	i,n;
	double	poly1[MAXORDER];
	double	poly2[MAXORDER][MAXORDER];
#ifndef BOR_THINK
	double	poly3[MAXORDER][MAXORDER][MAXORDER];
#endif

/* get the names from all the equations */
	for(i=0;i<num_eqns;++i)
		namelist = add_eqn_names(namelist,
				eqns[i]);
/* find the number of names */
	n = num_parameters(namelist);

	if( n < 1 || n > 3 )
	{
		eprintf("Bad number of names %d for conversion into polynomial\n");
		eprintf("should be one two or three\n",n);
		return;
	}

	for(i=0;i<n;++i)
	{
		namearray[i] = get_parameter(namelist,i+1);
	}
	if( n == 1 )
	{
		for(i=0;i<num_eqns;++i)
		{
			if(clean_flag) eval_ops(eqns[i]);
			expand(eqns[i]); /* equations must be expanded
						before converting to polys */
			init_poly1(poly1);
			add_to_poly1(eqns[i],poly1,namearray[0]);
			print_poly1(poly1);
		}
	}
	else if( n == 2 )
	{
		for(i=0;i<num_eqns;++i)
		{
			if(clean_flag) eval_ops(eqns[i]);
			expand(eqns[i]); /* equations must be expanded
						before converting to polys */
			init_poly2(poly2);
			add_to_poly2(eqns[i],poly2,namearray[0],namearray[1]);
			print_poly2(poly2);
		}
	}
#ifndef BOR_THINK
	else if( n == 3)
	{
		for(i=0;i<num_eqns;++i)
		{
			if(clean_flag) eval_ops(eqns[i]);
			expand(eqns[i]); /* equations must be expanded
						before converting to polys */
			init_poly3(poly3);
			add_to_poly3(eqns[i],poly3,
				namearray[0],namearray[1],namearray[2]);
			print_poly3(poly3);
		}
	}
#endif
	free_eqn_names(namelist);
}

void print_help()
{
	printf("Syntax:\n");
	printf("\teqntool options\n");
	printf("Options:\n");
	printf("\t-\tRead from standard input\n");
	printf("\tfilename\tRead equation from file\n");
	printf("\tequation\tRead command line string\n");
	printf("\t-c\tClean the equation, tries to simplify the equation\n");
	printf("\t-d x\tDifferentiate with respect to x\n");
	printf("\t-e\tExpand the equation, trys express the equation as a sum of terms like x^2 y^4\n");
	printf("\t-f\tUse the standard set of functions\n");
	printf("\t-i\tRead one equation from standard input\n");
	printf("\t-s\tAllows substitutions of one equation into the others\n");
	printf("\t-u\tFind the common denominator of each equation\n");
	printf("\t-A\tInteractivly ask for values of each parameter and output the results of the equations\n");
	printf("\t-C\tCompare first two equations\n");
	printf("\t-D\tDisplay the equation as a tree\n");
	printf("\t-F\tPrint the definitions of functions\n");
	printf("\t-P\tConvert to polynomials\n");
	printf("\t-T\tPrint equations\n");
	printf("\t-U\tFind common denominator of first two equations\n");
	printf("\t-h\tPrint this help message\n");
}

/*
 * Function:	main
 * Action:	the main routine, loop through arguments and do whats necessary
 */

#ifdef BOR_THINK
main()
#else
main(argc,argv)
int argc; char **argv;
#endif
{
	int i,c,is_eqn;
	FILE *fp;
	eqnode *eqn;
	char   *str_ptr;
	int	output = FALSE;
#ifdef BOR_THINK
	char	inputline[256];
	char	**argv;
	char	*argp[10];	/* Ten pointers to strings */
	int		argc,flag;
#endif
		/* create a list of functions */
	fun_list = add_standard_functions(NULL); 
	set_input_functions(fun_list);

#ifdef BOR_THINK
while(TRUE)
{
	printf("Input Command sequ\n");
	gets(inputline);
	if( !strcmp(inputline,"exit") ) exit;
	
	/* Now set argv to  the arguments of inputstring */
	
	argc = 1; flag = TRUE;
	argp[0] = &inputline[0];
	argv = &argp[0]; i = 0;
	while(inputline[i] != '\0')
	{
		c = inputline[i];
		if( flag ) /* In an argument */
		{
			if( c == ' ' )
			{
				inputline[i] = '\0';
				flag = FALSE;
			}
		}
		else	/* In white space */
		{
			if( c != ' ' )
			{
				argp[argc] = &inputline[i];
				flag = TRUE;
				++argc;
			}
		}
		++i;
	}
	--argv; ++argc;
#endif
 	while(--argc)	/* Loop through arguments */
	{
		++argv;

		if(argv[0][0] == '-' && argv[0][1] == '\0')
		{
			/* arg is - read from stdin untill EOF */

			while( (eqn = scan_eqn()) != NULL)
				process_eqn(eqn);
		}
		else if( argv[0][0] == '-' && argv[0][2] == '\0')
		{
			switch(argv[0][1])
			{
			case 'h': case '?':
				print_help();
				break;
			case 'c': /* clean */
				clean_flag = !clean_flag;
			/* if clean flag is set clean all equations */
				if(clean_flag)
				    for(i=0;i<num_eqns;++i)
					eval_funs(eqns[i]);
				break;

			case 'd': /* Differentiate */
				if( argc == 1 || argv[1][0] == '-')
				{
					eprintf("must specify a name for differentiation\n");
					break;
				}
			/* For each equation differentiate wrt argv[1]
				and the optionally clean the equation */

				for(i=0;i<num_eqns;++i)
				{
					if(clean_flag) eval_ops(eqns[i]);
					diff_wrt(eqns[i],argv[1]);
					if(clean_flag) eval_funs(eqns[i]);
				}
				++argv; --argc;
				break;

			case 'e': /* Expand */
				expand_flag = !expand_flag;
			/* if expand flag is set expand all equations */
				if(expand_flag)
				    for(i=0;i<num_eqns;++i)
				    {
					if(clean_flag) eval_ops(eqns[i]);
					expand(eqns[i]);
				    }
				break;

			case 'f': /* Use functions */
				funct_flag = !funct_flag;
			/* if function flag is use the standard functions in
				all the equations */
				if(funct_flag)
					set_input_functions(fun_list);
				else
					set_input_functions(NULL);
				break;

			case 'i': /* Input one equation from stdin */
				if( (eqn = scan_eqn()) != NULL)
					process_eqn(eqn);
				break;

			case 's': /* flip substitution flag */
				sub_flag = !sub_flag;
				break;

			case 'u': /* find the common denominators of each eqn */
				for(i=0;i<num_eqns;++i)
				{
					common_denominator(eqns[i]);
				}
				break;
				
			case 'A': /* Convert to rpe's and ask for values */
				output = TRUE;
				ask();
				break;

#ifndef SUN_SYS
			case 'C': /* Compare first two equations */
				if(eqns[0]->op == '/' || eqns[1]->op == '/')
				{
					common_denom2(eqns[0],eqns[1]);
					expand(eqns[0]->u.n.l);
					expand(eqns[1]->u.n.l);
					i = compare_sums(
						eqns[0]->u.n.l,
						eqns[1]->u.n.l);
				}
				else	i = compare_sums(eqns[0],eqns[1]);
				if( i ) printf("equations match\n");
				else	printf("equations don't match\n");
				break;
#endif
			case 'D': /* Display equations */
				output = TRUE;
				for(i=0;i<num_eqns;++i)
				{
					if(clean_flag) eval_ops(eqns[i]);
					display_eqn(eqns[i],0);
	 				printf("\n");
				}
				break;

			case 'F': /* Print function definitions */
				output = TRUE;
				fprint_funs(stdout,fun_list);
				break;

			case 'P': /* Convert to Polynomials */
				output = TRUE;
				conv_poly();
				break;

			case 'T': /* type(print) equations */
				output = TRUE;
				for(i=0;i<num_eqns;++i)
				{
					if(clean_flag) eval_ops(eqns[i]);
					print_eqn(eqns[i]);
	 				printf(";\n");
				}
				break;

			case 'U': /* com denom of first two  equations */
				common_denom2(eqns[0],eqns[1]);
				break;

			default:
				eprintf("Bad option %s\n",argv[0]);
			}
		}

		else	/* Not an option hence a filename or equation */
		{
			i = 0;
			is_eqn = FALSE;
			while( ( c = argv[0][i++] ) != '\0' )
			{
				/* if argument contains space or tab must
					be a equation */
				if( c == ' ' || c == '\t' )
					is_eqn = TRUE;
			}
			if( is_eqn || ( fp = fopen(argv[0],"r") ) == NULL )
			{
			/* If we failed to open argument as a file we
				asume that it's an equation */

				str_ptr = argv[0];
				while( (eqn = sscan_eqn(str_ptr)) != NULL)
				{
					process_eqn(eqn);
					/* Next equation (if it exists)
						will start after semi-colon */
					str_ptr = strchr(str_ptr,';');
					if( str_ptr == NULL ) break;
					++str_ptr;
				}
			}
			else
			{
			/* read in equations from file till EOF */
				while( (eqn = fscan_eqn(fp)) != NULL)
					process_eqn(eqn);
				fclose(fp);
			}
		}
	}
	if( !output )
	{
		/* If no equations so far then read from standard input */
		if(num_eqns == 0)
			while( (eqn = scan_eqn()) != NULL)
					process_eqn(eqn);
	
		/* And then print them out */
		for(i=0;i<num_eqns;++i)
		{
			if(clean_flag) eval_ops(eqns[i]);
			print_eqn(eqns[i]);
		 	printf(";\n");
		}
	}
#ifdef BOR_THINK
}	/* endwhile */
#endif

}

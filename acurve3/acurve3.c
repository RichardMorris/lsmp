/*
 *      file:   main.c:   
 *      author: Rich Morris
 *      date:   Jan 5 1995
 *      
 *	main file for acurve3 program.
 */

#define GCC_COMP
/*
#define COMMAND_LINE
#include <color.h>
#include "normlist.h"
#include "geomsimp.h"
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#ifndef GCC_COMP
#include <getopt.h>
#endif
#include <eqn.h>
#ifndef COMMAND_LINE
#include <tcl.h>
#include <tk.h>
#include <geom.h>
#endif

#ifndef MAXORDER
#  define MAXORDER 25
#endif

#define MAX_NUM_PARAMS 50
#define MAX_NUM_VARS 3
#define ABORT 1
#define FLUSH 2

#define grballoc(node) (node *) malloc( sizeof(node) )

/* Three parameters which effect the execution */

extern unsigned int RESOLUTION,LINK_FACE_LEVEL,LINK_SING_LEVEL,SUPER_FINE;


char	prog_name[10];	/* Name prog was started with */
int	n_vars = 3;	/* The number of variables */
int	n_eqns = 0;	/* The number of equations */
int	n_params = 0;	/* The number of parameters */

eqnode *main_eqn1 = NULL;	/* The main eqn */
eqnode *main_eqn2 = NULL;	/* The main eqn */

double rpe_vals[MAX_NUM_PARAMS + MAX_NUM_VARS];	/* values of varibles */
char   *rpe_names[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* names of variables */
char	laststring[40];		/* Interupt message */
eqn_funs *funlist = NULL;	/* list of functions */
int	int_status = FALSE;	/* Status of interupt buttons */
#ifndef COMMAND_LINE
Tcl_Interp *interp;		/* The interprater for tcl */
#endif

/* default values */

int	precision = 1;			/* Number of decimal places shown */
int	command = FALSE;		/* Write geomview comands */
int	quiet = TRUE;			/* quite mode */
float	clipmax = 10.0;			/* Clip values greater than this */
int	colour = -1;			/* Default colour */
int     edit_time = 0;                  /* time of last editing */

#define  CLIP(X) (X > clipmax ? clipmax :( X < -clipmax ? -clipmax : ( X != X ? clipmax : X) ))
#define  COL_CLIP(X) (X > 1.0 ? 1.0 :( X < 0 ? 0 : ( X != X ? 1 : X) ))

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

#ifdef GCC_COMP
#undef HUGE
#define HUGE 123456789.0
#endif

#ifdef X
#undef X
#undef Y
#undef Z
#undef W
#endif

#ifndef COMMAND_LINE
Geom *GeomFLoad(FILE *fp,char *str) {} /* A dummy function as we
                                                don't link -lgeom */
#endif

/* default values */

double  xmin = -1.14, xmax = 1.03, /* Size of bounding box */
        ymin = -1.13, ymax = 1.04,
        zmin = -1.12, zmax = 1.05;

/*********** Argument handeling ***********************/

float arg_clipmax = 0.0;
int   arg_precision = -1;
double arg_xmin = HUGE, arg_xmax = HUGE, arg_ymin = HUGE, arg_ymax = HUGE,
	arg_zmin = HUGE, arg_zmax = HUGE;
int	arg_colour = -5;
double arg_vals[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* vals from arguments */
char   *arg_names[MAX_NUM_PARAMS + MAX_NUM_VARS];
int     arg_count=0;	/* number of params in arguments */
char	*arg_filename = NULL;	/* filename from arguments */
int	temp_flag = FALSE;	/* TRUE if equation def on command line */
char	temp_file_name[L_tmpnam];	/* temp file for equation */

unsigned int arg_RESOLUTION=0,arg_LINK_FACE_LEVEL=0,
             arg_LINK_SING_LEVEL=0,arg_SUPER_FINE=0;

print_usage(char *name)
{
	fprintf(stderr,"Usage: %s [-v xl xh yl yh zl zh] [-c coarse] [-f fine] [-F faces] [-E edges]\n",name);
	fprintf(stderr,"\t\t[-p precision] [-h]\n");
	fprintf(stderr,"\t\t[-C col] [-D name val] {-G|-I|-e equation|filename}\n");
}


psurf3_args(argc,argv)
int argc; char **argv;
{
	int i;
	extern char *optarg;
	extern  int  optind;
	FILE	*temp_file;
	char	*slash;

	/** strip the first argument **/

        ++argv; --argc;

	/** first find the name prog was called by **/
        slash = strrchr(argv[0],'/');
        if(slash == NULL) strcpy(prog_name,argv[0]);
        else              strcpy(prog_name,slash+1);

	if( !strcmp(prog_name,"acurve3") )
	{
		n_eqns = 1;  n_vars = 3;
	}
	else
	{
		fprintf(stderr,"bad program name: %s\n",prog_name);
		exit(-1);
	}

	/* Now we can look at the arguments */

    while((i=getopt(argc,argv,"hGIe:c:F:E:f:p:D:C:v:")) != -1)
    {
                switch(i)
                {
                case 'c': arg_RESOLUTION = atoi(optarg); break;
                case 'F': arg_LINK_FACE_LEVEL =atoi(optarg); break;
                case 'E': arg_SUPER_FINE =atoi(optarg); break;
                case 'f': arg_LINK_SING_LEVEL =atoi(optarg); break;
		case 'C': arg_colour = atoi(optarg); break;
		case 'v':
			arg_xmin = atof(argv[optind++ -1]);
                        arg_xmax = atof(argv[optind++ -1]);
			arg_ymin = atof(argv[optind++ -1]);
                        arg_ymax = atof(argv[optind++ -1]);
			arg_zmin = atof(argv[optind++ -1]);
                        arg_zmax = atof(argv[optind++ -1]);
			--optind;
			break;
                        
		case 'p':
			arg_precision = atoi(optarg);
			break;
		case 'D':
		    while(optind < argc )
		    {
			if(argv[optind-1][0] == '-') break;

			COPY_STRING(arg_names[arg_count],
				argv[optind-1]);
			arg_vals[arg_count] =
				(double) atof(argv[optind]);
			optind += 2;
			++arg_count;
		    }
		    --optind;
		    break;
                case 'G': quiet = FALSE; command = TRUE; break;
                case 'I': quiet = FALSE; break;
                case 'e':
                        arg_filename = tmpnam(temp_file_name);
                        temp_file = fopen(arg_filename,"w");
                        fprintf(temp_file,"%s\n",optarg);
                        fclose(temp_file);
                        temp_flag = TRUE;
                        break;

                case 'h':
		default:
		    print_usage(prog_name);
		    exit(-1);
		}
	}
	for(;optind<argc;optind++)
	{
		if( arg_filename != NULL )
		{
			fprintf(stderr,"Only one file name allowed\007\n");
			print_usage(prog_name);
		    exit(-1);
		}
		else
		{
			arg_filename = argv[optind];
			command = FALSE;
		}
	}

	if(quiet && arg_filename == NULL )
	{
		fprintf(stderr,"A file name must be given\n");
		print_usage(prog_name);
		exit(-1);
	}
}

/*
	This fun is called after file has been read to arguments
	are applied after file values
*/

use_arg_vals()
{
  int i,j;


  if(arg_RESOLUTION != 0 ) RESOLUTION = arg_RESOLUTION;
  if(arg_LINK_FACE_LEVEL != 0 ) LINK_FACE_LEVEL = arg_LINK_FACE_LEVEL;
  if(arg_SUPER_FINE != 0 ) SUPER_FINE = arg_SUPER_FINE;
  if(arg_LINK_SING_LEVEL != 0 ) LINK_SING_LEVEL = arg_LINK_SING_LEVEL;
  if(arg_precision != -1 ) precision = arg_precision;
  if(arg_colour != -5 ) colour = arg_colour;
  if(arg_xmin != HUGE) xmin = arg_xmin;
  if(arg_xmax != HUGE) xmax = arg_xmax;
  if(arg_ymin != HUGE) ymin = arg_ymin;
  if(arg_ymax != HUGE) ymax = arg_ymax;
  if(arg_zmin != HUGE) zmin = arg_zmin;
  if(arg_zmax != HUGE) zmax = arg_zmax;

  for(i=0;i<arg_count;++i)
  {
	for(j=0; j < n_params;++j)
	{
		if(!strcmp(arg_names[i],rpe_names[n_vars+j]))
		{
			rpe_vals[j+n_vars] = arg_vals[i];
			break;
		}
	}
  }
}

/************ general initilisation ************************/

/*
 * Function:	init_funs
 * Action;	perform initilisation on the equation front
 */

init_funs()
{
	int offset = 0;

	funlist = add_standard_functions(NULL);
        set_input_functions(funlist);
	rpe_names[offset++] = "x";
	rpe_names[offset++] = "y";
	rpe_names[offset++] = "z";
}

/************* The guts actually does the work ***********************/

/*
 * Function:	acurve3
 * Action;	calculates the surface
 */

acurve3()
{
        double   a[MAXORDER][MAXORDER][MAXORDER];       /* polynomial rep */
        double   b[MAXORDER][MAXORDER][MAXORDER];       /* polynomial rep */
        double   c[MAXORDER][MAXORDER][MAXORDER];       /* polynomial rep */
        double   d[MAXORDER][MAXORDER][MAXORDER];       /* polynomial rep */
        double   e[MAXORDER][MAXORDER][MAXORDER];       /* polynomial rep */
        eqnode  *sub_eqn,*temp_eqn1,*temp_eqn2;
        eqnode  *dXdx,*dXdy,*dXdz,*dYdx,*dYdy,*dYdz,*prod_eqn;

        int     i = 0;
        char    *name;
        int xord,yord,zord;

        if(main_eqn1 == NULL || main_eqn2 == NULL) return;
        if(main_eqn1->op == '=') main_eqn1->op = '-';
        if(main_eqn2->op == '=') main_eqn2->op = '-';
        temp_eqn1 = duplicate(main_eqn1);
        temp_eqn2 = duplicate(main_eqn2);

        /**** First some coments ****/

#ifndef COMMAND_LINE
        start_geom();
#endif
	printf("LIST\n");
	printf("COMMENT acurve3 LSMP_DEF {\n");
        printf("# Algebraic curve in 3D defined by\n");
        printf(" "); print_eqn(main_eqn1); printf(";\n");
        printf(" "); print_eqn(main_eqn2); printf(";\n");
        for(i=0;i < n_params;++i)
                printf(" %s = %f;\n",rpe_names[i+n_vars],rpe_vals[i+n_vars]);
        printf(" %s = [%f,%f];\n%s = [%f,%f];\n%s = [%f,%f];\n",
                rpe_names[0],xmin,xmax,rpe_names[1],ymin,ymax,
                rpe_names[2],zmin,zmax);
        printf(" _coarse = %d;\n",RESOLUTION);
        printf(" _fine = %d;\n",LINK_SING_LEVEL);
        printf(" _faces = %d;\n",LINK_FACE_LEVEL);
        printf(" _edges = %d;\n",SUPER_FINE);
        printf(" _precision = %d;\n",precision);
        printf(" _colour = %d;\n",colour);
	printf("}\n");
	printf("COMMENT asurf LSMP_EDIT_TIMESTAMP { %d }\n",edit_time);

	eval_ops(temp_eqn1);
	eval_ops(temp_eqn2);
	for(i=0;i<n_params;++i)
	{
                sub_eqn = assign( rpe_names[i+n_vars] , rpe_vals[i+n_vars] );
                substitute( temp_eqn1, sub_eqn );
                substitute( temp_eqn2, sub_eqn );
                free_eqn_tree( sub_eqn );
        }
	eval_funs(temp_eqn1);
	eval_funs(temp_eqn2);
	expand(temp_eqn1);
	expand(temp_eqn2);
        eval_ops(temp_eqn1);
        eval_ops(temp_eqn2);
        init_poly3(a);
        /* construct (dXdx,dXdy,dXdz)^(dYdx,dYdy,dYdz) */

        dXdx = duplicate(temp_eqn1); diff_wrt(dXdx,rpe_names[0]);
        dXdy = duplicate(temp_eqn1); diff_wrt(dXdy,rpe_names[1]);
        dXdz = duplicate(temp_eqn1); diff_wrt(dXdz,rpe_names[2]);
        dYdx = duplicate(temp_eqn2); diff_wrt(dYdx,rpe_names[0]);
        dYdy = duplicate(temp_eqn2); diff_wrt(dYdy,rpe_names[1]);
        dYdz = duplicate(temp_eqn2); diff_wrt(dYdz,rpe_names[2]);

        prod_eqn =
            join_eqns('^',
                join_eqns(',',dXdx,
                   join_eqns(',',dXdy,
                                 dXdz)),
                join_eqns(',',dYdx,
                   join_eqns(',',dYdy,
                                 dYdz)));
        clean_eqn(prod_eqn);

        expand(temp_eqn1);
        expand(temp_eqn2);
        expand(prod_eqn);

        init_poly3(a);
        init_poly3(b);
        init_poly3(c);
        init_poly3(d);
        init_poly3(e);

        if( add_to_poly3( temp_eqn1,a,rpe_names[0],rpe_names[1],rpe_names[2])
         && add_to_poly3( temp_eqn2,b,rpe_names[0],rpe_names[1],rpe_names[2])
         && add_to_poly3( get_eqn_arg(prod_eqn,1),
                c,rpe_names[0],rpe_names[1],rpe_names[2])
         && add_to_poly3( get_eqn_arg(prod_eqn,2),
                d,rpe_names[0],rpe_names[1],rpe_names[2])
         && add_to_poly3( get_eqn_arg(prod_eqn,3),
                e,rpe_names[0],rpe_names[1],rpe_names[2]) )
        {
                /**** Now the data ****/

                initoogl(colour);
                set_acurve3_sigs();
                marmain(a,b,c,d,e,xmin,xmax,ymin,ymax,zmin,zmax);
                reset_acurve3_sigs();
                finioogl();
        }
        else
	{
                fprintf(stderr,"Could not convert equation to polynomial form!\n");
                fprintf(stderr,"Equation is:\n");
                fprintf(stderr,"Equation are:\n");
                fprint_eqn(stderr,temp_eqn1); fprintf(stderr,";\n");
                fprint_eqn(stderr,temp_eqn2); fprintf(stderr,";\n");
/*
                fprint_eqn(stderr,get_eqn_arg(prod_eqn,1)); fprintf(stderr,";\n");
                fprint_eqn(stderr,get_eqn_arg(prod_eqn,2)); fprintf(stderr,";\n");
                fprint_eqn(stderr,get_eqn_arg(prod_eqn,3)); fprintf(stderr,";\n");
*/
                fprintf(stderr,";\n");
        }
#ifndef COMMAND_LINE
        fini_geom();
#endif
        free_eqn_tree( temp_eqn1 );
        free_eqn_tree( temp_eqn2 );
        free_eqn_tree( prod_eqn );
}

/********* file handeling ***********************/

/*
 * Function:	read_def
 * Action:	read in a new equation from 'filename',
 *		create a new geometry, set the default values etc..
 */

read_def(filename)
char *filename;
{
	char	*tempname;
	int	i=0,j;
	eqnode	*X_eqn,*Y_eqn,*Z_eqn,*W_eqn,*temp,*sub;
	eqn_names *allnames;
	char	*old_names[MAX_NUM_PARAMS];
	double	old_vals[MAX_NUM_PARAMS];
	int	old_num_params;
        char    *new_names[MAX_NUM_PARAMS];
        double  new_vals[MAX_NUM_PARAMS];
        int     new_count = 0,num_int_defs;
	FILE	*fp;
 
	edit_time = 0;
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not open file %s\007\n",filename);
		return;
	}

	eqn_errs_to_stderr = FALSE;

	/* save old names */

	for(i=1;i<=n_params;++i)
	{
		old_vals[i] = rpe_vals[i-1+n_vars];
		old_names[i] = rpe_names[i-1+n_vars];
	}
	old_num_params = n_params;

	/* clear out any previous equations */

	if( main_eqn1 != NULL ) free_eqn_tree(main_eqn1);
	if( main_eqn2 != NULL ) free_eqn_tree(main_eqn2);
	clear_rpe_const();
	/*free_eqn_names(allnames);*/

	/* input the equations */

	main_eqn1 = fscan_eqn(fp);
	main_eqn2 = fscan_eqn(fp);
	if( main_eqn1 == NULL)
	{
		fprintf(stderr,"Bad equation:");
		fprint_eqn(stderr,main_eqn1);
		fprintf(stderr,"\n");
		goto fini_read;
	}
	if( main_eqn2 == NULL)
	{
		fprintf(stderr,"Bad equation:");
		fprint_eqn(stderr,main_eqn2);
		fprintf(stderr,"\n");
		goto fini_read;
	}
 
	/* Now read in posible names and ranges for variables */
	/* and any macro def's */

	num_int_defs = 0;	/* The number of interval definitions */

	while( (temp = fscan_eqn(fp)) != NULL )
	{
		eval_funs(temp);

		if( temp->op != '='
                 || !( temp->u.n.l->op == NAME
                          || ( temp->u.n.l->op == '*'
                                 && temp->u.n.l->u.n.l->op == NAME )) )
                {
                        fprintf(stderr,"Bad equation:");
                        fprint_eqn(stderr,temp);
                        fprintf(stderr,"\nShould be an asignment\n");
                        continue;
                }

                else if( temp->u.n.r->op == NUMBER )
                {
                        if(!strcmp(temp->u.n.l->u.str,"acurve3_coarse"))
                                RESOLUTION = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"acurve3_fine"))
                                LINK_SING_LEVEL = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"acurve3_faces"))
                                LINK_FACE_LEVEL = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"acurve3_edges"))
                                SUPER_FINE = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"acurve3_colour"))
				colour = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"acurve3_precision"))
                        {
                                precision = (int) temp->u.n.r->u.num;
                        }
                    else
                    {
                        COPY_STRING(new_names[new_count],temp->u.n.l->u.str);
                        new_vals[new_count] = temp->u.n.r->u.num;
                        ++new_count;
                    }
                }

		else if( temp->u.n.r->op == INTERVAL )
		{
			if( temp->u.n.l->op != NAME
                         || temp->u.n.r->u.n.l->op != NUMBER
                         || temp->u.n.r->u.n.r->op != NUMBER )
                        {
                                fprintf(stderr,"Bad interval definition\n");
                                fprint_eqn(stderr,temp);
                                fprintf(stderr,"\nShould be of form 'x = [-1,1]'\n");
                        }

			if( num_int_defs >= n_vars )
			{
				fprintf(stderr,"Too many variable definitions\n");
				continue;
			}
			COPY_STRING(rpe_names[num_int_defs],temp->u.n.l->u.str);
			if( num_int_defs == 0 )
			{
				xmin = temp->u.n.r->u.n.l->u.num;
				xmax = temp->u.n.r->u.n.r->u.num;
			}
			else if( num_int_defs == 1 )
			{
                                ymin = temp->u.n.r->u.n.l->u.num;
                                ymax = temp->u.n.r->u.n.r->u.num;
                        }
			else if( num_int_defs == 2 )
			{
                                zmin = temp->u.n.r->u.n.l->u.num;
                                zmax = temp->u.n.r->u.n.r->u.num;
                        }
			++num_int_defs;
		}

		else    /* a macro definition */
                {
                        if( temp->u.n.l->op == NAME 
                         || ( temp->u.n.l->op == '*'
                           && temp->u.n.l->u.n.l->op == NAME ) )
                        {
                                substitute(main_eqn1,temp);
                                substitute(main_eqn2,temp);
                        }
                        else
                        {
                                fprintf(stderr,"Bad macro definition\n");
                                fprint_eqn(stderr,temp);
                                fprintf(stderr,"\nShould be of form 'x = ..' or 'f(x,y,..) = ...'\n");
                        }
                }
		free_eqn_tree(temp);

	} /* end while */

	allnames = add_eqn_names(NULL,main_eqn1);
	allnames = add_eqn_names(allnames,main_eqn2);
	for(i=0;i<n_vars;++i)
		make_variable(allnames,rpe_names[i]);

	/* Find out all the parameter names */

	n_params = num_parameters(allnames);
	for(i=1; i <= n_params;++i)
	{
		tempname = get_parameter(allnames,i);
		if(i>= MAX_NUM_PARAMS )
		{
			fprintf(stderr,"Sorry too many parameters\n");
			break;
		}
		COPY_STRING(rpe_names[i+n_vars-1],tempname);
		rpe_vals[i+n_vars-1] = 0.0;
		for(j=1;j<=old_num_params;++j)
			if(!strcmp(tempname,old_names[j]) )
				rpe_vals[i+n_vars-1] = old_vals[j];

		for(j=0;j<new_count;++j)
			if(!strcmp(tempname,new_names[j]) )
				rpe_vals[i+n_vars-1] = new_vals[j];
	}


	/*
	 * Now play about with the equations
	 * for the X_eqn we set temp2 = temp = X_eqn with X=0
	 * construct the rpe and then differentiate wrt x and y
	 */

	eval_ops(main_eqn1);
	eval_ops(main_eqn2);
		
fini_read:
	for(i=1;i<=old_num_params;++i)
	{
		free(old_names[i]);
	}

	fclose(fp);
	eqn_errs_to_stderr = FALSE;
	edit_time = time(NULL);
}

/*
 * When we read form a file we want to reset the variable names
 */

read_file(char *filename)
{
	int offset = 0;

	/* First reset default variable names */
	rpe_names[offset++] = "x";
	rpe_names[offset++] = "y";
	rpe_names[offset++] = "z";
	read_def(filename);
}

/****** Now some macros for the copy program *****/

#define write_to_end_of_line {\
	while((cc = getc(in)) != EOF && cc != '\n' )\
		putc(cc,out);\
	putc('\n',out);}

#define write_till_semi_colon {\
	while((cc = getc(in)) != EOF && cc != ';' ) {\
		putc(cc,out);\
		if(cc=='#') write_to_end_of_line;}\
	putc(';',out);}

#define skip_till_semi_colon {\
	while((cc = getc(in)) != EOF && cc != ';' )\
                if(cc=='#') {\
			putc(cc,out);\
			write_to_end_of_line;}}

/*
 * Function:	copy_def
 * Action:	copy the definitions from the input to the output
 *		missing off the range and parameter definitions.
 */

copy_def(in,out)
FILE *in,*out;
{
	int i,c,cc,spaces=0,name_ptr,dont_copy;
	char name[80];

	write_till_semi_colon;

	/* for the remaining eqns we want to get any macros 
		and coments but miss off varibles and parameters */

	while((c = getc(in))!=EOF)
	{
		if( c == '#' )
		{
			for(i=0;i<spaces;++i) putc(' ',out);
			spaces = 0;
			putc(c,out);
			write_to_end_of_line;
		}
		else if( c == '\n' )
		{
			spaces = 0;
			putc(c,out);
		}
		else if( c == ' ' )
			++spaces;
		else if( c == '\t' )
			spaces = 8 * ( spaces/8 + 1);
		else if( isalpha(c) || c == '_' || c == '?' )
		{
			name[0] = c;
			name_ptr = 1;
			while( (c=getc(in))!=EOF)
			{
				if(isalnum(c) || c == '_' )
					name[name_ptr++] = c;
				else break;
			}
			ungetc(c,in);
			name[name_ptr] = '\0';

			/* Now find if name matches a parameter */

			dont_copy = FALSE;
			for(i=0;i< n_vars + n_params;++i)
			{
				if(!strcmp(name,rpe_names[i]))
					dont_copy = TRUE;
			}
                        if( !strcmp(name,"acurve3_coarse")) dont_copy = TRUE;
                        if( !strcmp(name,"acurve3_fine")) dont_copy = TRUE;
                        if( !strcmp(name,"acurve3_faces")) dont_copy = TRUE;
                        if( !strcmp(name,"acurve3_edges")) dont_copy = TRUE;
                        if( !strcmp(name,"acurve3_precision")) dont_copy = TRUE;
                        if( !strcmp(name,"acurve3_colour")) dont_copy = TRUE;
			if( dont_copy )
			{
				skip_till_semi_colon;
				c = getc(in);
				if( c != '\n' ) ungetc(c,in);
				spaces = 0;
			}
			else
			{
				for(i=0;i<spaces;++i) putc(' ',out);
				fprintf(out,"%s",name);
				write_till_semi_colon;
				spaces = 0;
			}
		}
	} /* end while */
}

/************* Interupt handling **********************************/

/*
   We wish to trap SIGTERM (kill(1) command) SIGINT (^C from keyboard)
   SIGHUP (hang-up) and SIGTSTP (software interupt)
   when detected we call finioogl which writes out what we have already got,
   and flush the output before exiting.

   At regular intervals check_interupt will be called from further down
   in the depths of the program. If the 'stop' button has been pressed
   TRUE is returned which causes everything to be finished off nicely.

   If Quit is selected from the window-manager we again end gracefully.
*/
  
/*
 * Function:    acurve3_sig
 * Action:      handles signals
 */

void acurve3_sig(int sig,int code)
{
        pid_t pid;

        if(sig == SIGTSTP )
        {
                flushoogl();
#ifndef COMMAND_LINE
                fini_geom();
#endif
                pid = getpid();
                fprintf(stderr,"%s\n",laststring);
                signal(SIGCONT,acurve3_sig);
                kill(pid,SIGTSTP);
        }
        else if(sig == SIGCONT )
        {
#ifndef COMMAND_LINE
                start_geom();
#endif
                signal(SIGTSTP,acurve3_sig);
                fprintf(stderr,"%s\n",laststring);
                fprintf(stderr,"continuing\n");
                pid = getpid();
                kill(pid,SIGCONT);
        }
        else
        {
                signal(sig,SIG_IGN);
                finioogl();
#ifndef COMMAND_LINE
                fini_geom();
#endif
                exit(-1);
        }
}

set_acurve3_sigs()
{
        signal(SIGHUP,acurve3_sig);
        signal(SIGINT,acurve3_sig);
        signal(SIGTERM,acurve3_sig);
        signal(SIGTSTP,acurve3_sig);
}

reset_acurve3_sigs()
{
        signal(SIGHUP,SIG_DFL);
        signal(SIGINT,SIG_DFL);
        signal(SIGTERM,SIG_DFL);
        signal(SIGTSTP,SIG_DFL);
}

/*
 * Function:    check_interupt
 * Action:      checks to see if interupt has been pressed
 *              return TRUE if so.
 */

int check_interupt(char *string)
{
        long dev;
        short val;
	char	tcl_command[128];

        if( string != NULL ) strcpy(laststring,string);
        if( quiet ) return(FALSE);
#ifndef COMMAND_LINE
        if( string != NULL )
	{
		sprintf(tcl_command,"add_status \"%s\"\n",string);
		Tcl_GlobalEval(interp,tcl_command);
	}
	int_status = FALSE;
	Tk_DoOneEvent(TK_DONT_WAIT);
/*
        if( ret == FL_EVENT )
        {
                dev = fl_qread(&val);
                if(dev == WINQUIT )
                {
                        finioogl();
                        fini_geom();
                        exit(-1);
                }
        }
*/
        if( int_status == ABORT )
                 return(TRUE);
        else if( int_status == FLUSH )
        {
                flushoogl();
                fini_geom();
                start_geom();
        }
        return( FALSE );
#endif
}

#ifndef COMMAND_LINE
int abort_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int_status = ABORT;
	return TCL_OK;
}

int flush_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int_status = FLUSH;
	return TCL_OK;
}

/************* Now call back functions ****************************/

/*
 * Load the file
 * given the filename and the name of a tempory file for the editor
 */

int load_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp,*fq;
	int offset = 0;

        if(argc != 3)
        {
                interp->result = "Wrong number of arguments";
                return TCL_ERROR;
        }

	read_file(argv[1]);
	fp = fopen(argv[1],"r");
	if( fp == NULL )
	{
		interp->result = "Could not open file";
		return TCL_ERROR;
	}
	fq = fopen(argv[2],"w");
	copy_def(fp,fq);
	fclose(fp);
	fclose(fq);
	set_geom_name(argv[1]);
	return TCL_OK;
}

/*
 * Save the file
 * given file to save in and tempory file with editor text
 */

int save_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp,*fq;
	int  i;

        if(argc != 3)
        {
                interp->result = "Wrong number of arguments";
                return TCL_ERROR;
        }
	fp = fopen(argv[1],"w");
	if( fp == NULL )
	{
		interp->result = "Could not save file";
		return TCL_ERROR;
	}
	fq = fopen(argv[2],"r");
	copy_def(fq,fp);

	fprintf(fp," %s = [%f,%f];\n",rpe_names[0],xmin,xmax);
	fprintf(fp," %s = [%f,%f];\n",rpe_names[1],ymin,ymax);
	fprintf(fp," %s = [%f,%f];\n",rpe_names[2],zmin,zmax);

	for(i=0;i<n_params;++i)
		fprintf(fp," %s = %f;\n",
			rpe_names[i+n_vars],rpe_vals[i+n_vars]);
        fprintf(fp," acurve3_coarse = %d;\n",RESOLUTION);
        fprintf(fp," acurve3_fine = %d;\n",LINK_SING_LEVEL);
        fprintf(fp," acurve3_faces = %d;\n",LINK_FACE_LEVEL);
        fprintf(fp," acurve3_edges = %d;\n",SUPER_FINE);
        fprintf(fp," acurve3_precision = %d;\n",precision);
        fprintf(fp," acurve3_colour = %d;\n",colour);
	fclose(fp);
	fclose(fq);
	set_geom_name(argv[1]);
	return TCL_OK;
}

/*
 * Update the equations, called when editor is changed
 *	given the name of editor file
 */

int update_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp,*fq;
	int offset = 0;

        if(argc != 2)
        {
                interp->result = "Wrong number of arguments";
		return TCL_ERROR;
	}
	read_def(argv[1]);
	return TCL_OK;
}

/*
 * Run the program
 */

int run_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	acurve3();
	return TCL_OK;
}

int get_progname(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        sprintf(interp->result,"%s %d %d %d",prog_name,n_vars,n_eqns);
        return TCL_OK;
}

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        sprintf(interp->result,"%d %d %d %d %d %d\n",
                precision,RESOLUTION,LINK_SING_LEVEL,LINK_FACE_LEVEL,
			SUPER_FINE,colour);
        return TCL_OK;
}

int set_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        int i;

        if(argc != 7)
        {
                interp->result = "set_options: Wrong number of arguments";
                return TCL_ERROR;
        }
        precision = atoi(argv[1]);
	RESOLUTION = atoi(argv[2]);
	LINK_SING_LEVEL = atoi(argv[3]);
	LINK_FACE_LEVEL = atoi(argv[4]);
	SUPER_FINE = atoi(argv[5]);
        colour = atoi(argv[6]);

        return TCL_OK;
}

int get_num_params(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        sprintf(interp->result,"%d",n_params);
        return TCL_OK;
}

/*
 * Set the ith parameter value
 * set_param {i val}
 */

int set_param(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        if( argc != 3)
        {
                interp->result = "set_options: Wrong number of arguments";
                return TCL_ERROR;
        }
	rpe_vals[atoi(argv[1]) + n_vars] = atof(argv[2]);
	return TCL_OK;
}

/*
 * Returns the name and value of the ith parameter
 */

int get_param(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        int i;

        if( argc != 2)
        {
                interp->result = "set_options: Wrong number of arguments";
                return TCL_ERROR;
        }
        i = atoi(argv[1]);
        sprintf(interp->result,"%s %f",
                rpe_names[n_vars+i],
                rpe_vals[n_vars+i]);
        return TCL_OK;
}

int set_variables(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        int i;

	if( argc != 7)
        {
                interp->result = "set_variables: Wrong number of arguments";
                return TCL_ERROR;
        }
	xmin = atof(argv[1]);
	xmax = atof(argv[2]);
	ymin = atof(argv[3]);
	ymax = atof(argv[4]);
	zmin = atof(argv[5]);
	zmax = atof(argv[6]);
        return TCL_OK;
}

int get_variables(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        	sprintf(interp->result,"%s %f %f %s %f %f %s %f %f\n",
			rpe_names[0],xmin,xmax,
			rpe_names[1],ymin,ymax,
			rpe_names[2],zmin,zmax);
        return TCL_OK;
}

/*
 * Called from the object menu
 */

int object_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
	        interp->result = "object_cb: Wrong number of arguments";
                return TCL_ERROR;
        }
	set_object_mode(argv[1]);
	return TCL_OK;
}

/*
int target_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
	        interp->result = "object_cb: Wrong number of arguments";
                return TCL_ERROR;
        }
	set_target_name(argv[1]);
	return TCL_OK;
}
*/

int get_env(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        char    *tcl_dir, *examples_dir;
        tcl_dir = getenv("LSMP_TCL");
	examples_dir = getenv("LSMP_EXAMPLES");
        sprintf(interp->result,"%s %s",tcl_dir,examples_dir);
        return(TCL_OK);
}
#endif

/******* The main routine *********************************************/

main(argc, argv)        
int argc; char **argv;
{
	char	string[5],tcl_file[128],*tcl_dir;
	long	dev;
	short	val;
	int	i;
        int code;
#ifndef COMMAND_LINE
        Tk_Window wind;
#endif

	/* First check permision */

	tcl_dir = getenv("LSMP_TCL");
	if(tcl_dir == NULL)
	{
		fprintf(stderr,"Can not find LSMP_TCL environment variable\n");
		exit(-1);
	}
	strcpy(tcl_file,tcl_dir);
	strcat(tcl_file,"/../bin/livlock");
	i = system(tcl_file);
	if(WEXITSTATUS(i) != 5 ) exit(-1);


	/* read in arguments */

	psurf3_args(argc,argv);

	init_funs();

	/* If quite run then exit */

	if( quiet )
	{
                read_file(arg_filename);
		use_arg_vals();
		acurve3();
		if( temp_flag ) unlink(temp_file_name);
		exit(0);
	}

#ifndef COMMAND_LINE
	/* If we don't foreground then the process forks and dies
	   as soon as we do graphics. This is bad.
	 */


        if( arg_filename != NULL)
        {
                read_file(arg_filename);
		use_arg_vals();
                set_geom_name(arg_filename);
        }
	else
                set_geom_name(prog_name);

        interp = Tcl_CreateInterp();
/*
        wind = Tk_CreateMainWindow(interp,NULL,prog_name, "acurve3");
	if(wind == NULL)
	{
		fprintf(stderr,"%s\n",interp->result);
		exit(1);
	}
*/

        if (Tcl_Init(interp) == TCL_ERROR)
	{
		fprintf(stderr,"Tcl_Init failed: %s\n",interp->result);
		exit(-1); 
	}
        if (Tk_Init(interp) == TCL_ERROR)
	{
		fprintf(stderr,"Tk_Init failed: %s\n",interp->result);
		exit(-1); 
	}


	/* initilise tcl commands */

        Tcl_CreateCommand(interp,"get_progname",get_progname,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"load_cb",load_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"save_cb",save_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"update_cb",update_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"run_cb",run_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"get_variables",get_variables,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"set_variables",set_variables,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"get_options",get_options,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"set_options",set_options,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"get_num_params",get_num_params,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"get_param",get_param,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"set_param",set_param,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"object_cb",object_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"get_env",get_env,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"abort_cb",abort_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp,"flush_cb",flush_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	use_arg_vals();

        strcpy(tcl_file,tcl_dir);
        strcat(tcl_file,"/acurve3.tcl");

	code = Tcl_EvalFile(interp,tcl_file);
        if (code != TCL_OK) {
		fprintf(stderr,"%s\n",interp->result);
                exit(1);
        }

        Tk_MainLoop();

	if( temp_flag ) unlink(temp_file_name);
#endif
}

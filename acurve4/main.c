/*
 * Copyright I guess there should be some copywrite for this package,
 * 
 * 			Copyright (c) 1994
 * 
 * 	Liverpool University Department of Pure Mathematics,
 * 	Liverpool, L69 3BX, England.
 * 
 * 	Author Dr R. J. Morris.
 * 
 * 	e-mail rmorris@uk.ac.liv.uxb
 *
 */
#define NO_FORMS
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>
#ifndef NO_FORMS
#include <device.h>
#endif
#include <getopt.h>
#ifndef NO_FORMS
#include <forms.h>
#include "fd_panels.h"
#ifndef FL_FRACT
#  include "fract.h"
#endif
#ifndef FL_EDIT
#include "edit.h"
#endif
#endif
#include <eqn.h>
#include <sys/time.h>
#include <sys/resource.h>

/*
*/
#define PRI_MALL

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

#ifndef MAXORDER
#  define MAXORDER 25
#endif
#define MAX_NUM_PARAMS 50
#define NUM_VARS 4
#define MAX(a,b) ((a)>(b)?(a):(b))
#define REPLACE 1
#define ADD 2
/*
#define EXTRAOPTIONS
*/

int	quiet = TRUE;
#ifdef EXTRAOPTIONS
int	colours = FALSE, normals = FALSE; /* Not used yet */
#endif
int	precision = 1;
float	step = 0.1;
int	command = FALSE;
int	write_mode = REPLACE;
char	geomname[128] = "ac4";	/* Geomview target */
int	geomnumber = 0;			/* Geomview target number */

#ifndef NO_FORMS
FL_FORM	*param_form = NULL;
#endif

char handle[20];

				/* The main equation read from file */
eqnode	*main_eqn1,*main_eqn2,*main_eqn3;
				/* The vec product of derivatives */
eqnode	*prod_eqn1,*prod_eqn2,*prod_eqn3,*prod_eqn4;
eqn_names *allnames;	/* A list of names in the equation */
eqn_funs  *funlist = NULL; /* The list of functions */

int	jiggle;				/* Jiggle the box a bit */
char	*variables[4] = {"x","y","z","w"};	/* Names of four variables */
#ifndef NO_FORMS
FL_OBJECT	*param_obj[MAX_NUM_PARAMS];
#endif
double 		param_vals[MAX_NUM_PARAMS];
int  num_params=0;	/* Number of parameters */
char interupt_string[80];	/* String used for Interupts */

double arg_vals[MAX_NUM_PARAMS + NUM_VARS];
char   *arg_names[MAX_NUM_PARAMS + NUM_VARS];
int     arg_count=0;
char	*arg_filename = NULL;
char	temp_file_name[80];
int	temp_flag = FALSE;


/* Three parameters which effect the execution */

extern unsigned int RESOLUTION,LINK_BOX_LEVEL,LINK_SING_LEVEL,SUPER_FINE;
unsigned int arg_RESOLUTION=0,arg_LINK_BOX_LEVEL=0,arg_LINK_SING_LEVEL=0,arg_SUPER_FINE=0;
int arg_precision = -1;
float arg_step = -1.0;

/* default values */

double  xmin = -1.14, xmax = 1.03, /* Size of bounding box */
        ymin = -1.13, ymax = 1.04,
        zmin = -1.12, zmax = 1.05,
        wmin = -1.11, wmax = 1.06;

/************ ROUTINES **************************************/

/*
 * Function:	ac4_args
 * action:	process arguments
 */

ac4_args(argc,argv)
int argc; char **argv;
{
    int  i;
    extern char *optarg;
    extern int   optind;
    FILE	*temp_file;

    while((i=getopt(argc,argv,"GIhe:c:B:f:p:D:")) != -1)
    {
                switch(i)
                {
		case 'c': arg_RESOLUTION = atoi(optarg); break;
		case 'B': arg_LINK_BOX_LEVEL =atoi(optarg); break;
		case 'f': arg_LINK_SING_LEVEL =atoi(optarg); break;
#ifdef EXTRAOPTIONS
		case 'E': arg_SUPER_FINE =atoi(optarg); break;
		case 'C': colours = TRUE; break;
		case 'N': normals = TRUE; break;
#endif
		case 'p':
			arg_precision = atoi(optarg);
			arg_step = 1.0;
			for( i = 0; i< arg_precision; ++i )
				 arg_step = arg_step/10;
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
                case 'e':
                        arg_filename = tmpnam(temp_file_name);
                        temp_file = fopen(arg_filename,"w");
                        fprintf(temp_file,"%s\n",optarg);
                        fclose(temp_file);
                        temp_flag = TRUE;
                        break;
                case 'G': quiet = FALSE; command = TRUE; break;
                case 'I': quiet = FALSE; break;

                case 'h':
  
		default:
			print_usage();
			exit(-1);
                }
	}

	for(;optind < argc; optind++)
	{
		if( arg_filename != NULL )
		{
			fprintf(stderr,"Only one file name allowed\n");
			print_usage();
			exit(-1);
		}
		else
		{
                	arg_filename  = argv[optind];
		}
	}

    if( quiet && arg_filename == NULL )
    {
		fprintf(stderr,"A file name must be specified\n");
		print_usage();
		exit(-1);
    }
}

/*
 * Function:	print_usage
 * Action;	prints out the corect usage string
 */

print_usage()
{
	fprintf(stderr,"Usage:	intersect [-c coarse] [-f fine] [-B box] [-p precision]\n");
	fprintf(stderr,"\t\t[-D name val] {-G|-I|-e equation|filename}\n");
}

use_arg_vals()
{
  int i,j;

  if(arg_RESOLUTION != 0 ) RESOLUTION = arg_RESOLUTION;
  if(arg_LINK_BOX_LEVEL != 0 ) LINK_BOX_LEVEL = arg_LINK_BOX_LEVEL;
  if(arg_SUPER_FINE != 0 ) SUPER_FINE = arg_SUPER_FINE;
  if(arg_LINK_SING_LEVEL != 0 ) LINK_SING_LEVEL = arg_LINK_SING_LEVEL;
  if(arg_precision != -1 ) precision = arg_precision;
  if(arg_step != -1.0 ) step = arg_step;

  for(i=0;i<arg_count;++i)
  {
	for(j=1; j <= num_params;++j)
	{
		if(!strcmp(arg_names[i], get_parameter(allnames,j)))
		{
			param_vals[j] = arg_vals[i];
			break;
		}
	}
  }
}


#ifdef PRI_MALL
print_mallinfo()
{
/*
        struct mallinfo info;
        struct rusage   usage;
*/
        FILE    *fp;
        int     c;

        system("ps -l | grep acurve4 > zap");
        fp = fopen("zap","r");
        while( ( c = fgetc(fp) ) != EOF ) fputc(c,stderr);
        fclose(fp);
/*
        info = mallinfo();
        fprintf(stderr,"arena %d ordblks %d smblks %d hblkhd %d hblks %d\n",
                info.arena,info.ordblks,info.smblks,info.hblkhd,info.hblks);
        fprintf(stderr,"usmblks %d fsmblks %d uordblks %d fordblks %d keepcost %d\n",
                info.usmblks,info.fsmblks,info.uordblks,info.fordblks,info.keepcost);

        getrusage(RUSAGE_SELF,&usage);
        fprintf(stderr,"utime %d.%d stime %d.%d maxrss %d ru_nswap %d\n",
                usage.ru_utime.tv_sec,usage.ru_utime.tv_usec,
                usage.ru_stime.tv_sec,usage.ru_stime.tv_usec,
                usage.ru_maxrss,usage.ru_nswap);
*/
}
#endif


/***************** begin calculations **********************/

/*
 * Function:	run_algorithm
 * Action:	processes all the parameters and then stats the algorithm
 */

run_algorithm()
{
  	double   aa[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
  	double   bb[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
  	double   cc[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
  	double   dd[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
  	double   ee[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
  	double   ff[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
  	double   gg[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
	eqnode	*sub_eqn,*temp_eqn1,*temp_eqn2,*temp_eqn3;
	eqnode	*a,*b,*c,*d,*e,*f,*g,*h,*i,*j,*k,*l;
	eqnode  *tan_eqn1,*tan_eqn2,*tan_eqn3,*tan_eqn4;
	char	*name;
	int xord,yord,zord,word,count;

#ifdef PRI_MALL
	fprintf(stderr,"run_algorithm\n");
	print_mallinfo();
#endif

	if(main_eqn1 == NULL || main_eqn2 == NULL || main_eqn3 == NULL)
	{
		fprintf(stderr,"Can not calculate intersection null equations");
		return;
	}
	if(main_eqn1->op == '=') main_eqn1->op = '-';
	if(main_eqn2->op == '=') main_eqn2->op = '-';
	if(main_eqn3->op == '=') main_eqn3->op = '-';
	temp_eqn1 = duplicate(main_eqn1);
	temp_eqn2 = duplicate(main_eqn2);
	temp_eqn3 = duplicate(main_eqn3);
        
	/**** First some coments ****/

	start_geom();
	printf("# Intersection of surfaces defined by\n");
	printf("# "); print_eqn(main_eqn1); printf(";\n");
	printf("# "); print_eqn(main_eqn2); printf(";\n");
	printf("# "); print_eqn(main_eqn3); printf(";\n");

	eval_ops(temp_eqn1);
	eval_ops(temp_eqn2);
	eval_ops(temp_eqn3);
	for(count=1;count<=num_params;++count)
	{
		name = get_parameter(allnames,count);
	        sub_eqn = assign( name , param_vals[count] );
	        substitute( temp_eqn1, sub_eqn );
	        substitute( temp_eqn2, sub_eqn );
	        substitute( temp_eqn3, sub_eqn );

		printf("# ");
		print_eqn(sub_eqn);
		printf(";\n");
	        free_eqn_tree( sub_eqn );
	}
	eval_funs(temp_eqn1);
	eval_funs(temp_eqn2);
	eval_funs(temp_eqn3);

	/* The tangent for the curve is given by 
		| a b c d |
		| e f g h | where X Y Z W are cartesian coordinate
		| i j k l | vectors.
		| X Y Z W | 
		  |b c d|    |a c d|    |a b d|    |a b c|
	    =   - |f g h|X + |e g h|Y + |e f h|Z + |e f g|W
		  |j k l|    |i k l|    |i j l|    |i j k|
	    = (-b * |g h| + c |f h| - d |f g| ) X + ...
	 	    |k l|     |j l|     |j k| 
	*/

	a = duplicate(temp_eqn1); diff_wrt(a,variables[0]);
	b = duplicate(temp_eqn1); diff_wrt(b,variables[1]);
	c = duplicate(temp_eqn1); diff_wrt(c,variables[2]);
	d = duplicate(temp_eqn1); diff_wrt(d,variables[3]);
	e = duplicate(temp_eqn2); diff_wrt(e,variables[0]);
	f = duplicate(temp_eqn2); diff_wrt(f,variables[1]);
	g = duplicate(temp_eqn2); diff_wrt(g,variables[2]);
	h = duplicate(temp_eqn2); diff_wrt(h,variables[3]);
	i = duplicate(temp_eqn3); diff_wrt(i,variables[0]);
	j = duplicate(temp_eqn3); diff_wrt(j,variables[1]);
	k = duplicate(temp_eqn3); diff_wrt(k,variables[2]);
	l = duplicate(temp_eqn3); diff_wrt(l,variables[3]);
	
#define twodet(A,B,C,D) (\
	join_eqns('-',join_dup_eqns('*',A,D),join_dup_eqns('*',B,C)))

#define threedet(A,B,C,D,E,F,G,H,I) (\
	join_eqns('+',join_eqns('-', \
	  join_eqns('*',duplicate(A),twodet(E,F,H,I)), \
	  join_eqns('*',duplicate(B),twodet(D,F,G,I))), \
	  join_eqns('*',duplicate(C),twodet(D,E,G,H))) )

	tan_eqn1 = threedet(b,c,d, f,g,h, j,k,l);
	tan_eqn2 = threedet(a,c,d, e,g,h, i,k,l);
	tan_eqn3 = threedet(a,b,d, e,f,h, i,j,l);
	tan_eqn4 = threedet(a,b,c, e,f,g, i,j,k);

	expand(temp_eqn1);
	expand(temp_eqn2);
	expand(temp_eqn3);
	expand(tan_eqn1);
	expand(tan_eqn2);
	expand(tan_eqn3);
	expand(tan_eqn4);

	init_poly4(aa);
	init_poly4(bb);
	init_poly4(cc);
	init_poly4(dd);
	init_poly4(ee);
	init_poly4(ff);
	init_poly4(gg);

	if( add_to_poly4( temp_eqn1,aa,
		variables[0],variables[1],variables[2],variables[3])
	 && add_to_poly4( temp_eqn2,bb,
		variables[0],variables[1],variables[2],variables[3])
	 && add_to_poly4( temp_eqn3,cc,
		variables[0],variables[1],variables[2],variables[3])
	 && sub_from_poly4( tan_eqn1,dd,
		variables[0],variables[1],variables[2],variables[3])
	 && add_to_poly4( tan_eqn2,ee,
		variables[0],variables[1],variables[2],variables[3])
	 && sub_from_poly4( tan_eqn3,ff,
		variables[0],variables[1],variables[2],variables[3])
	 && add_to_poly4( tan_eqn4,gg,
		variables[0],variables[1],variables[2],variables[3]) )
	{
		/* Now work out bounding box */

		printf("#The bounding box is [%lf,%lf][%lf,%lf],[%lf,%lf],[%lf,%lf]\n",
			xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);

		printf("#corse %d fine %d boxes %d\n",
			RESOLUTION,LINK_SING_LEVEL,LINK_BOX_LEVEL);

		/**** Now the data ****/

		initoogl();
		set_ac4_sigs();
#ifdef PRI_MALL
	fprintf(stderr,"marmain ...\n");
	print_mallinfo();
#endif

		marmain(aa,bb,cc,dd,ee,ff,gg,
			xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
		reset_ac4_sigs();
		finioogl();
	}
	else
	{
		fprintf(stderr,"Could not convert equations to polynomial form!\n");
		fprintf(stderr,"Equation are:\n");
		fprint_eqn(stderr,temp_eqn1); fprintf(stderr,";\n");
		fprint_eqn(stderr,temp_eqn2); fprintf(stderr,";\n");
		fprint_eqn(stderr,temp_eqn3); fprintf(stderr,";\n");
		fprint_eqn(stderr,tan_eqn1); fprintf(stderr,";\n");
		fprint_eqn(stderr,tan_eqn2); fprintf(stderr,";\n");
		fprint_eqn(stderr,tan_eqn3); fprintf(stderr,";\n");
		fprint_eqn(stderr,tan_eqn4); fprintf(stderr,";\n");
	}
	fini_geom();
	free_eqn_tree( temp_eqn1 );
	free_eqn_tree( temp_eqn2 );
	free_eqn_tree( temp_eqn3 );
	free_eqn_tree( tan_eqn1 );
	free_eqn_tree( tan_eqn2 );
	free_eqn_tree( tan_eqn3 );
	free_eqn_tree( tan_eqn4 );
}

/************** reading/writing equations and parameters  ****************/

/*
 * Function:	init_funs
 * Action:	initilise stuff relating to definitions
 */

init_funs()
{
	funlist = add_standard_functions(NULL);
	set_input_functions(funlist);
	variables[0] = "x";
	variables[1] = "y";
	variables[2] = "z";
	variables[3] = "w";
}
	
/*
 * Function:	read_file
 * action:	read all the information in from the file
 */

read_def(filename)
char *filename;
{
	char	*tempname;
	int	i=0,j,num_vars,old_num_params;
	eqnode	*temp;
	FILE	*fp;
	char	old_names[MAX_NUM_PARAMS][64];
	double	old_vals[MAX_NUM_PARAMS];
        char    *new_names[MAX_NUM_PARAMS];
        double  new_vals[MAX_NUM_PARAMS];
        int     new_count = 0;

	fp = fopen(filename,"r");

	if(fp == NULL )
	{
		fprintf(stderr,"Could not open file %s\n",filename);
		return;
	}

	/* save the old names */

	for(i=1;i<=num_params;++i)
	{
		old_vals[i] = param_vals[i];
		strcpy(old_names[i],get_parameter(allnames,i));
	}
	old_num_params = num_params;

	/* clear out any previous equations */

	if(main_eqn1 != NULL ) free_eqn_tree(main_eqn1);
	if(main_eqn2 != NULL ) free_eqn_tree(main_eqn2);
	if(main_eqn3 != NULL ) free_eqn_tree(main_eqn3);
	clear_rpe_const();
	free_eqn_names(allnames);

	/* input the equations */

	main_eqn1 = fscan_eqn(fp);
	main_eqn2 = fscan_eqn(fp);
	main_eqn3 = fscan_eqn(fp);

	/* Now read in posible ranges for variables */

	num_vars = 0;	/* got 0 variable variables to date */

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
                        fprintf(stderr,"\nShould be an asignment\n");                        		continue;
                }

		else if ( temp->u.n.r->op == NUMBER )
		{
			if(!strcmp(temp->u.n.l->u.str,"ac4_coarse"))
				RESOLUTION = (int) temp->u.n.r->u.num;
			else if(!strcmp(temp->u.n.l->u.str,"ac4_fine"))
				LINK_SING_LEVEL = (int) temp->u.n.r->u.num;
			else if(!strcmp(temp->u.n.l->u.str,"ac4_boxes"))
				LINK_BOX_LEVEL = (int) temp->u.n.r->u.num;
			else if(!strcmp(temp->u.n.l->u.str,"ac4_edges"))
				SUPER_FINE = (int) temp->u.n.r->u.num;
			else if(!strcmp(temp->u.n.l->u.str,"ac4_precision"))
               		{
                        	precision = (int) temp->u.n.r->u.num;
                        	step = 1;
                        	for( i = 0; i< precision; ++i ) step = step/10;
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
			if( ++num_vars > 4 )
			{
				fprintf(stderr,"Too many variables\n");
				break;
			}
			COPY_STRING(variables[num_vars-1], temp->u.n.l->u.str);
			if( num_vars == 1 )
			{
				xmin = temp->u.n.r->u.n.l->u.num;
				xmax = temp->u.n.r->u.n.r->u.num;
			}
			else if( num_vars == 2 )
			{
				ymin = temp->u.n.r->u.n.l->u.num;
				ymax = temp->u.n.r->u.n.r->u.num;
			}
			else if( num_vars == 3 )
			{
				zmin = temp->u.n.r->u.n.l->u.num;
				zmax = temp->u.n.r->u.n.r->u.num;
			}
			else if( num_vars == 4 )
			{
				wmin = temp->u.n.r->u.n.l->u.num;
				wmax = temp->u.n.r->u.n.r->u.num;
			}
		}
                else    /* a macro definition */
                {
                        if( temp->u.n.l->op == NAME 
                         || ( temp->u.n.l->op == '*'
                           && temp->u.n.l->u.n.l->op == NAME ) )
                        {
                                substitute(main_eqn1,temp);
                                substitute(main_eqn2,temp);
                                substitute(main_eqn3,temp);
                        }
                        else
                        {
                                fprintf(stderr,"Bad macro definition\n");
                                fprint_eqn(stderr,temp);
                                fprintf(stderr,"\nShould be of form 'x = ..' or 'f(x,y,..) = ...'\n");
                        }
                }
                free_eqn_tree(temp);
	}

  /* Now set "x", "y" "z" to be variables */

	allnames = add_eqn_names(NULL,main_eqn1);
	allnames = add_eqn_names(allnames,main_eqn2);
	allnames = add_eqn_names(allnames,main_eqn3);
	make_variable(allnames,variables[0]);
	make_variable(allnames,variables[1]);
	make_variable(allnames,variables[2]);
	make_variable(allnames,variables[3]);

	num_vars = 4;

	/* Set parameter values */

	num_params = num_parameters(allnames);

	for(i=1;i<=num_params;++i)
	{
		tempname = get_parameter(allnames,i);
		if(i>= MAX_NUM_PARAMS ) 
		{
			fprintf(stderr,"Sorry too many parameters\n");
			break;
		}
		param_vals[i] = 0.0;
		for(j=1;j<=old_num_params;++j)
			if(!strcmp(tempname,old_names[j]) )
				param_vals[i] = old_vals[j];

		for(j=0;j<new_count;++j)
			if(!strcmp(tempname,new_names[j]) )
				param_vals[i] = new_vals[j];
	}

	fclose(fp);
}

/*
 * Function:	read_def
 * Action:	sets default names for varibles and reads the definitions
 */

read_file(char *filename)
{
	variables[0] = "x";
	variables[1] = "y";
	variables[2] = "z";
	variables[3] = "w";
	read_def(filename);
}

/*
 * Function:	read_input_eqn
 * Action:	read in the equation from the input.
 * Note:	different handeling of defaults to above
 */

read_input_eqn()
{
	char	temp_file_name[L_tmpnam];

	tmpnam(temp_file_name);
#ifndef NO_FORMS
	fl_save_edit(eqn_edit,temp_file_name);
#endif
	read_def(temp_file_name);
	unlink(temp_file_name);
}

/*
 * Function:	write_file
 * action:	write the definitions and parameters into filename
 */

write_file(filename)
char *filename;
{
	FILE *fp,*temp;
	int i;
	char tname[L_tmpnam];

	tmpnam(tname);
#ifndef NO_FORMS
	fl_save_edit(eqn_edit,tname);
#endif
	temp = fopen(tname,"r");
	fp = fopen(filename,"w");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not open file %s\n",filename);
		return;
	}
	copy_def(temp,fp);

	fprintf(fp," %s = [ %f , %f ];\n",variables[0],xmin,xmax);
	fprintf(fp," %s = [ %f , %f ];\n",variables[1],ymin,ymax);
	fprintf(fp," %s = [ %f , %f ];\n",variables[2],zmin,zmax);
	fprintf(fp," %s = [ %f , %f ];\n",variables[3],wmin,wmax);
	for(i=1;i<=num_params;++i)
	{
		
		fprintf(fp," %s = %f;\n",
			get_parameter(allnames,i),param_vals[i]);
	}
	fprintf(fp," ac4_coarse = %d;\n",RESOLUTION);
	fprintf(fp," ac4_fine = %d;\n",LINK_SING_LEVEL);
	fprintf(fp," ac4_boxes = %d;\n",LINK_BOX_LEVEL);
	fprintf(fp," ac4_edges = %d;\n",SUPER_FINE);
	fprintf(fp," ac4_precision = %d;\n",precision);
	fclose(fp);
	fclose(temp);
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
 * Function:    copy_def
 * Action:      copy the definitions from the input to the output
 *              missing off the range and parameter definitions.
 */

copy_def(in,out)
FILE *in,*out;
{
        int i,c,cc,spaces=0,name_ptr,dont_copy;
        char name[80];

        write_till_semi_colon;
        write_till_semi_colon;
        write_till_semi_colon;

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
                else if( isalpha(c) )
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
			if( !strcmp(name,variables[0]) ) dont_copy = TRUE;
			if( !strcmp(name,variables[1]) ) dont_copy = TRUE;
			if( !strcmp(name,variables[2]) ) dont_copy = TRUE;
			if( !strcmp(name,variables[3]) ) dont_copy = TRUE;
			for(i=1;i<=num_params;++i)
			{
                        	if(!strcmp(name,get_parameter(allnames,i)))
                                        dont_copy = TRUE;
                        }
			if( !strcmp(name,"ac4_coarse")) dont_copy = TRUE;
			if( !strcmp(name,"ac4_fine")) dont_copy = TRUE;
			if( !strcmp(name,"ac4_boxes")) dont_copy = TRUE;
			if( !strcmp(name,"ac4_edges")) dont_copy = TRUE;
			if( !strcmp(name,"ac4_precision")) dont_copy = TRUE;

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

/*************** Forms updating **************************************/

#ifndef NO_FORMS
void param_callback(FL_OBJECT *, long);	/* A forward definition */

/*
 * Function:	update_forms
 * Action:	update the forms,
 *		if 'filename' is specified load the edit from that file
 */

update_forms(char *filename)
{
	FILE *fp,*temp;
	int  i;
	char tname[L_tmpnam],string[20];

	fl_freeze_form(main_panel);

	if(filename != NULL )
	{
	    fp=fopen(filename,"r");
	    fl_set_input(deffile_input,filename);
	    if(fp != NULL)
            {
                tmpnam(tname);
                temp = fopen(tname,"w");
                copy_def(fp,temp);
                fclose(temp);
                fl_load_edit(eqn_edit,tname);
                fclose(fp);
                unlink(tname);
             }
	}

	fl_set_fract_value(xmin_counter,xmin);
	fl_set_fract_value(xmax_counter,xmax);
	fl_set_object_label(xmin_counter,variables[0]);
	fl_set_fract_value(ymin_counter,ymin);
	fl_set_fract_value(ymax_counter,ymax);
	fl_set_object_label(ymin_counter,variables[1]);
	fl_set_fract_value(zmin_counter,zmin);
	fl_set_fract_value(zmax_counter,zmax);
	fl_set_object_label(zmin_counter,variables[2]);
	fl_set_fract_value(wmin_counter,wmin);
	fl_set_fract_value(wmax_counter,wmax);
	fl_set_object_label(wmin_counter,variables[3]);

	fl_set_fract_precision(xmin_counter,precision);
	fl_set_fract_precision(xmax_counter,precision);
	fl_set_fract_precision(ymin_counter,precision);
	fl_set_fract_precision(ymax_counter,precision);
	fl_set_fract_precision(zmin_counter,precision);
	fl_set_fract_precision(zmax_counter,precision);
	fl_set_fract_precision(wmin_counter,precision);
	fl_set_fract_precision(wmax_counter,precision);

	fl_set_fract_step(xmin_counter,step,step*10);
	fl_set_fract_step(xmax_counter,step,step*10);
	fl_set_fract_step(ymax_counter,step,step*10);
	fl_set_fract_step(ymin_counter,step,step*10);
	fl_set_fract_step(zmin_counter,step,step*10);
	fl_set_fract_step(zmax_counter,step,step*10);
	fl_set_fract_step(wmin_counter,step,step*10);
	fl_set_fract_step(wmax_counter,step,step*10);

	/* Options defaults */

	sprintf(string,"%d",precision);
	fl_set_input(dp_input,string);
	sprintf(string,"%d",RESOLUTION);
	fl_set_input(coarse_input,string);
	sprintf(string,"%d",LINK_SING_LEVEL);
	fl_set_input(fine_input,string);
	sprintf(string,"%d",LINK_BOX_LEVEL);
	fl_set_input(boxes_input,string);
	sprintf(string,"%d",SUPER_FINE);
	fl_set_input(edges_input,string);

	fl_unfreeze_form(main_panel);
  
	create_param_form();
}

/*
 * Function:	create_param_form
 * Action:	creates a form with the various parameters which
 *		can be changed.
 */

create_param_form()
{
	int	i=0;
	static	int oldnumparams = 0;
        char    string[128];

        if(geomnumber == 0 )
                sprintf(string,"%s\0",geomname);
        else
                sprintf(string,"%s<%d>\0",geomname,geomnumber);

	if( oldnumparams != num_params )
	{
		if( param_form != NULL )
		{
			fl_hide_form(param_form);
			fl_free_form(param_form);
		}
		if( num_params == 0 )
		{
			param_form = NULL;
			oldnumparams = num_params;
			return;
		}
		param_form = fl_bgn_form(FL_UP_BOX,
				290.0, (num_params + 1.5) * 40.0 );
		param_obj[0] = fl_add_box(FL_FRAME_BOX,
				40.0, num_params * 40.0 + 20.0,
				200.0,30.0,string);

		for(i=1;i<=num_params;++i)
		{
			param_obj[i] = fl_add_fract(FL_NORMAL_FRACT,
				40.0, (num_params - i ) * 40.0 + 20.0,
				200.0, 30.0, get_parameter(allnames,i));
			fl_set_call_back(param_obj[i], param_callback, i);
			fl_set_object_align(param_obj[i],FL_ALIGN_LEFT);
			fl_set_fract_precision(param_obj[i],precision);
			fl_set_fract_step(param_obj[i],step,step*10);
			fl_set_fract_value(param_obj[i],param_vals[i]);
		}
		fl_end_form();
		fl_show_form(param_form, FL_PLACE_SIZE, TRUE, "Parameters");
	}
	else if( param_form != NULL )
	{
		fl_freeze_form(param_form);
		fl_set_object_label(param_obj[0],string);
		for(i=1;i<=num_params;++i)
		{
			fl_set_object_label(param_obj[i],
					get_parameter(allnames,i));
			fl_set_fract_value(param_obj[i],param_vals[i]);
			fl_set_fract_precision(param_obj[i],precision);
			fl_set_fract_step(param_obj[i],step,step*10);
		}
		fl_unfreeze_form(param_form);
	}
	oldnumparams = num_params;
}
#endif

/********** Geomview communication **********************/

/*
 * Function:	geom_name_exist
 * Action:	returns true if there exists a geometry name<number>
 *		in geomview
 */

int geom_name_exists(name,number)
char *name;
int number;
{
	int c,quote_count=0;

	if( !command ) return(FALSE);

	if( number == 0 )
		printf("(echo (real-id %s))\n",name);
	else
		printf("(echo (real-id %s<%d>))\n",name,number);
	fflush(stdout);

	while( ( c = getchar() ) != EOF )
        {
                if( c == '\n' || c == '\t' ) continue;
                if( c == '"' )
                {
                        ++quote_count;
                        if(quote_count == 1 ) continue;
                        else break;
                }
                if( quote_count == 0 && c == 'n' )
                {
                        /* Check for the name 'nil' */

                        if( getchar() == 'i' && getchar() == 'l' )
                        {
                                return(FALSE);
                        }
                }
        }
	return(TRUE);
}

/*
 * Function:	create_new_geom
 * Action:	creates a new geom with geomname and geomnumber
 */

create_new_geom()
{
	printf("# File %s\n",geomname);
	sprintf(handle,"ps%d",time(NULL));
	if( command )
	{
		if( geomnumber == 0 )
			printf( "(new-geometry %s { : %s })\n",
				geomname,handle);
		else
			printf("(new-geometry %s<%d> { : %s })\n",
				geomname,geomnumber,handle);
		fflush(stdout);
	}
}

/*
 * Function:	new_geom_name
 * Action:	sets the name of the geom object
 *		if filename is NULL use last name with a different number
 *		tests to see if name exists in geomview and calculates
 *		an apropriate number
 *		uses geomname, and geomnumber
 */

new_geom_name(name)
char *name;
{
	char *slash;

        if( name != NULL && name[0] == '\0' )
        {
                fprintf(stderr,
                        "Warning no name specified, using previous name\n");
                return;
        }

	if(name != NULL && strcmp(geomname,name) )
	{
		slash = strrchr(name,'/');
		if( slash != NULL )  name = slash +1;

		strcpy(geomname,name);
		geomnumber = 0;
	}

	if( name != NULL && name[0] == '\0' )
	{
		fprintf(stderr,
			"Warning no name specified, using previous name\n");
		return;
	}

	while( geom_name_exists(geomname,geomnumber) )
		++geomnumber;
#ifndef NO_FORMS
	if( !quiet ) update_forms(NULL);
#endif
}

/*
 * Function:	check_geom_name
 * Action:	checks to see if the geom name exist in geomview
 *		if not create a new geometry
 * Uses:	geomname,geomnumber,handle
 */

check_geom_name()
{
	if( geom_name_exists(geomname,geomnumber) ) return;

	create_new_geom();
}

/*
 * Function:	change_geom_name
 * Action:	change the name of the current geometry
 * Uses:	geomname,geomnumber,handle
 */

change_geom_name(name)
char *name;
{
	int number = 0;
	char *slash;

	if(name == NULL)
	{
		fprintf(stderr,"Tried to change an object to have a null name\n");
		return;
	}

	if( name[0] == '\0' )
	{
		fprintf(stderr,
			"Warning no name specified, using previous name\n");
		return;
	}

	if(name != NULL && strcmp(geomname,name) )
	slash = strrchr(name,'/');
	if( slash != NULL )  name = slash +1;

	if( geom_name_exists(geomname,geomnumber) )
	{
		while( geom_name_exists(name,number) ) ++number;

		if( geomnumber == 0 && number == 0 )
			printf("(name-object %s %s)\n",geomname,name);
		else if( number == 0 )
			printf("(name-object %s<%d> %s)\n",
				geomname,geomnumber,name );
		else if( geomnumber == 0 )
			printf("(name-object %s %s<%d>)\n",
				geomname,name,number);
		else
			printf("(name-object %s<d> %s<%d>)\n",
				geomname,geomnumber,name,number);
		fflush(stdout);

		strcpy(geomname,name);
		geomnumber = number;
	}
	else
	{
		/* Create a new object */

		while(geom_name_exists(name,number) ) ++number;
		
		strcpy(geomname,name);
		geomnumber = number;
	}
#ifndef NO_FORMS
	if( !quiet ) update_forms(NULL);
#endif
}

start_geom()
{
    if( write_mode == ADD ) new_geom_name(NULL);
    check_geom_name();
    if( command )
    {
	printf("(read geometry { define %s\n",handle);
    }
}

fini_geom()
{
	if( command )
	{
		printf("})\n");
		fflush(stdout);
	}
}

/*************** Interupt handeling	*****************/
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
 * Function:	ac4_sig
 * Action:	handles signals
 */

ac4_sig(int sig,int code)
{
	pid_t pid;

	if(sig == SIGTSTP )
	{
		flushoogl();
		fini_geom();
		pid = getpid();
		fprintf(stderr,"%s\n",interupt_string);
		signal(SIGCONT,ac4_sig);
		kill(pid,SIGTSTP);
	}
	else if(sig == SIGCONT )
	{
		start_geom();
		signal(SIGTSTP,ac4_sig);
		fprintf(stderr,"continuing\n");
		fprintf(stderr,"%s\n",interupt_string);
		pid = getpid();
		kill(pid,SIGCONT);
	}
	else
	{
		signal(sig,SIG_IGN);
		finioogl();
		fini_geom();
		exit(-1);
	}
}

set_ac4_sigs()
{
	signal(SIGHUP,ac4_sig);
	signal(SIGINT,ac4_sig);
	signal(SIGTERM,ac4_sig);
	signal(SIGTSTP,ac4_sig);
}

reset_ac4_sigs()
{
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	signal(SIGTERM,SIG_DFL);
	signal(SIGTSTP,SIG_DFL);
}

/*
 * Function:	check_interupt
 * Action:	checks to see if interupt has been pressed
 *		return TRUE if so.
 */

int check_interupt(char *string)
{
#ifndef NO_FORMS
	FL_OBJECT *ret;
#endif
	long dev;
	short val;

	if( string != NULL ) strcpy(interupt_string,string);
	if( quiet ) return(FALSE);
#ifndef NO_FORMS
	if( string != NULL ) fl_addto_browser(report_brow,string);
	ret = fl_check_forms();
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
	else if( ret == abort_but )
		 return(TRUE);
	else if( ret == flush_but )
	{
		flushoogl();
		fini_geom();
		start_geom();
	}
#endif
	return( FALSE );
}


/*************	Call back routines ******************************/

#ifndef NO_FORMS
void param_callback( FL_OBJECT *obj, long val)
{
	param_vals[val] = (double) fl_get_fract_value(obj);
}

void bound_cb(FL_OBJECT *obj, long val)
{
	switch( val )
	{
	case 1:	xmin = fl_get_fract_value(obj); break;
	case 2:	xmax = fl_get_fract_value(obj); break;
	case 3:	ymin = fl_get_fract_value(obj); break;
	case 4:	ymax = fl_get_fract_value(obj); break;
	case 5:	zmin = fl_get_fract_value(obj); break;
	case 6:	zmax = fl_get_fract_value(obj); break;
	case 7:	wmin = fl_get_fract_value(obj); break;
	case 8:	wmax = fl_get_fract_value(obj); break;
	}
}

void filename_cb(FL_OBJECT *obj, long val)
{
	char *filename,*head,*tail,root[128];

	if( val == 1 )	/* Filename input */
	{
		filename = fl_get_input(obj);
		if(filename == NULL ) return;
		read_file(filename);
		new_geom_name(filename);
		update_forms(filename);
	}
	else if ( val == 2 ) /* Load button */
	{
		filename = fl_show_file_selector("Definition File","","","");
		if(filename == NULL ) return;
		read_file(filename);
		new_geom_name(filename);
		update_forms(filename);
	}
	else if ( val == 3 )	/* Save button */
	{
		strcpy(root,fl_get_input(deffile_input));
		tail = strrchr(root,'/');
		if( tail == NULL )
		{
			tail = root;
			head = "";
		}
		else
		{
			head = root;
			*tail = '\0';
			++tail;
		}
		filename = fl_show_file_selector("Definition File",
			head,"",tail);
		if(filename == NULL ) return;
		write_file(filename);
		change_geom_name(filename);
		update_forms(filename);
	}
	else if ( val == 4 )	/* Input from equ_edit */
	{
		read_input_eqn();
		update_forms(NULL);
	}
}

void help_cb(FL_OBJECT *obj, long val)
{
	if( val == 2 )
	{
		fl_hide_form(ac4_help);
	}
	else	fl_show_form(ac4_help, FL_PLACE_SIZE, FALSE, "Intersect(1)");
}

void opt_cb(FL_OBJECT *obj, long val)
{
	int i; long oldwin;

	switch( val )
	{
	case -1:
		fl_hide_form(option_form);
		break;
	case 0: fl_show_form(option_form,FL_PLACE_SIZE,TRUE,"Intersect Options");
		oldwin = winget();
		winset(option_form->window);
		winpop();
		winset(oldwin);
		break;
	case 1: precision = atoi(fl_get_input(obj));
		step = 1;
		for( i = 0; i< precision; ++i ) step = step/10;

		fl_set_fract_precision(xmin_counter,precision);
		fl_set_fract_precision(xmax_counter,precision);
		fl_set_fract_precision(ymin_counter,precision);
		fl_set_fract_precision(ymax_counter,precision);
		fl_set_fract_precision(zmin_counter,precision);
		fl_set_fract_precision(zmax_counter,precision);
		fl_set_fract_precision(wmin_counter,precision);
		fl_set_fract_precision(wmax_counter,precision);

		fl_set_fract_step(xmin_counter,step,step*10);
		fl_set_fract_step(xmax_counter,step,step*10);
		fl_set_fract_step(ymin_counter,step,step*10);
		fl_set_fract_step(ymax_counter,step,step*10);
		fl_set_fract_step(zmin_counter,step,step*10);
		fl_set_fract_step(zmax_counter,step,step*10);
		fl_set_fract_step(wmin_counter,step,step*10);
		fl_set_fract_step(wmax_counter,step,step*10);

		for(i=1;i<=num_params;++i)
		{
			fl_set_fract_precision(param_obj[i],precision);
			fl_set_fract_step(param_obj[i],step,step*10);
		}
		break;
        case 2: write_mode = ADD;
                break;
        case 3: write_mode = REPLACE;
                break;
 
#ifdef EXTRAOPTIONS
	case 2: colours = fl_get_button(obj); 
		break;
	case 3: normals = fl_get_button(obj); 
		break;
	case 7: jiggle = fl_get_button(obj);
		break;
#endif
	case 4: RESOLUTION = atoi(fl_get_input(obj));
		break;
	case 5: LINK_SING_LEVEL = atoi(fl_get_input(obj));
		break;
	case 6: LINK_BOX_LEVEL = atoi(fl_get_input(obj));
		break;
	case 7: SUPER_FINE = atoi(fl_get_input(obj));
		break;
	case 10: /* New button */
		new_geom_name(NULL);
		break;
	}
}
	
void exit_cb(FL_OBJECT *obj, long val)
{
	exit();
}

void run_cb(FL_OBJECT *obj, long val)
{
	static long old_x = -1,old_y = -1; /* This will force middle of
						screen on first run */

	fl_deactivate_form(main_panel);
	fl_deactivate_form(ac4_help);
	fl_deactivate_form(option_form);
	fl_clear_browser(report_brow);
	fl_set_form_position(run_form,old_x,old_y);
	if( old_x == -1 && old_y == -1 )
		fl_show_form(run_form,FL_PLACE_CENTER,TRUE,"Running");
	else
		fl_show_form(run_form,FL_PLACE_POSITION,TRUE,"Running");
	run_algorithm();
	old_x = run_form->x; old_y = run_form->y;
	fl_activate_form(main_panel);
	fl_activate_form(ac4_help);
	fl_activate_form(option_form);
	fl_hide_form(run_form);
}
#endif
	

/******* The main routine *********************************************/

main(argc, argv)        
int argc; char **argv;
{
	char	string[5];
#ifndef NO_FORMS
	FL_OBJECT *ret;
#endif
	long dev;
	short val;
	int	i;

	/* First check permision */

#ifdef PRI_MALL
	print_mallinfo();
#endif

#ifndef NO_FORMS
	i = system("livlock");
	if(WEXITSTATUS(i) != 5 ) exit(-1);
#endif


	/* read in the arguments */

	ac4_args(argc,argv);

	init_funs();

#ifdef PRI_MALL
	print_mallinfo();
#endif

	/* If quiet mode run then exit */

	if( quiet )
	{
		read_file(arg_filename);
		use_arg_vals();
		new_geom_name(arg_filename);
#ifdef PRI_MALL
	print_mallinfo();
#endif

		run_algorithm();
		if( temp_flag ) unlink(temp_file_name);
		exit(0);
	}

	/* If we don't foreground then the process forks and dies
	   as soon as we do graphics. This is bad.
	 */

#ifndef NO_FORMS
	foreground();

	/* This routine is defined in the code generated by 
	   the forms designer.
	 */

        create_the_forms();

	/* Bounding box defaults */

	fl_set_fract_value(xmin_counter,xmin);
	fl_set_fract_value(xmax_counter,xmax);
	fl_set_fract_value(ymin_counter,ymin);
	fl_set_fract_value(ymax_counter,ymax);
	fl_set_fract_value(zmin_counter,zmin);
	fl_set_fract_value(zmax_counter,zmax);
	fl_set_fract_value(wmin_counter,zmin);
	fl_set_fract_value(wmax_counter,zmax);

	fl_set_fract_precision(xmin_counter,precision);
	fl_set_fract_precision(xmax_counter,precision);
	fl_set_fract_precision(ymin_counter,precision);
	fl_set_fract_precision(ymax_counter,precision);
	fl_set_fract_precision(zmin_counter,precision);
	fl_set_fract_precision(zmax_counter,precision);
	fl_set_fract_precision(wmin_counter,precision);
	fl_set_fract_precision(wmax_counter,precision);

	fl_set_fract_step(xmin_counter,step,step*10);
	fl_set_fract_step(xmax_counter,step,step*10);
	fl_set_fract_step(ymax_counter,step,step*10);
	fl_set_fract_step(ymin_counter,step,step*10);
	fl_set_fract_step(zmin_counter,step,step*10);
	fl_set_fract_step(zmax_counter,step,step*10);
	fl_set_fract_step(wmin_counter,step,step*10);
	fl_set_fract_step(wmax_counter,step,step*10);

	/* Options defaults */

	sprintf(string,"%d",precision);
	fl_set_input(dp_input,string);
	sprintf(string,"%d",RESOLUTION);
	fl_set_input(coarse_input,string);
	sprintf(string,"%d",LINK_SING_LEVEL);
	fl_set_input(fine_input,string);
	sprintf(string,"%d",LINK_BOX_LEVEL);
	fl_set_input(boxes_input,string);
	sprintf(string,"%d",SUPER_FINE);
	fl_set_input(edges_input,string);
#ifdef EXTRAOPTIONS
	fl_set_button(colour_but,colours);
	fl_set_button(normal_but,normals);
	fl_set_button(jiggle_but,jiggle);
#endif
	fl_set_button(replace_but,1);

	/* Help window */

	load_help(help_browser);

	/* Draw the main form */

	fl_show_form(main_panel, FL_PLACE_SIZE, TRUE, "Intersect");

	/* If appropriate read in the file */

	if( arg_filename != NULL)
	{
		read_file(arg_filename);
		use_arg_vals();
		new_geom_name(arg_filename);
		update_forms(arg_filename);
	}
	else	new_geom_name(NULL);

	/* Now endlessly check for button presses etc */

	use_arg_vals();
	while(TRUE)
	{
		ret = fl_do_forms();
		if( ret == FL_EVENT )
		{
			dev = fl_qread(&val);
			if(dev == WINQUIT ) break;
		}
	}
	if( temp_flag ) unlink(temp_file_name);
#endif
}

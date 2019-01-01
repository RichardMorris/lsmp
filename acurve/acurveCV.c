/*
 *      file:   main.c:   
 *      author: Rich Morris
 *      date:   5 Jan 1995
 *      
 *	main prog for acurve program
 * 	of three equations
 */

/*
#define GCC_COMP
#define NO_GEOM
#define SUN_CC
#include <color.h>
#include "normlist.h"
#include "geomsimp.h"
*/
#define CGIVRML
#define COMMAND_LINE
#define NO_GEOM

#ifndef NO_GEOM
#include <geom.h>
#endif

#ifndef COMMAND_LINE
#include <tcl.h>
#include <tk.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <time.h>

#ifndef CGIVRML
#include <sys/wait.h>
#endif

#include <eqn.h>
#include "../CVcommon.h"

#define MAX_NUM_PARAMS 50
#define MAX_NUM_VARS 6
typedef double VEC3D[3];
/*
#define DO_TANGENTS
#define PRINT_NORMALS
*/
#define VECTOR_EQNS

#define grballoc(node) (node *) malloc( sizeof(node) )

extern unsigned int RESOLUTION,LINK_FACE_LEVEL;

/* What function the program performs, 4 = 4D coords, N = transform normals,
	C = alter colours  */

enum {Acurve, Discrim} prog_type;
char	prog_name[10];
int	n_vars = 2;	/* The number of variables */
int	n_eqns = 0;	/* The number of equations */
int	n_params = 0;	/* The number of parameters */
int	map_eqns = FALSE;	/* An equation for mapping */
int	do_map = FALSE;		/* Perform the mapping (else crit set ) */
int     vrml_version = 1; /* the version of VRML produced */

double xmin = -2.0, xmax = 2.0, ymin= -2.0, ymax=2.0;

eqnode *main_eqn = NULL;	/* The main eqn */
eqnode *map_eqn = NULL;	/* Equation of the mapping */

int	*map_rpe;	/* mapping rpe */

double rpe_vals[MAX_NUM_PARAMS + MAX_NUM_VARS];	/* values of varibles */
char   *rpe_names[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* names of variables */

eqn_funs *funlist = NULL;	/* list of functions */
char	my_geom_name[255];

/* default values */

int	precision = 1;			/* Number of decimal places shown */
int	command = FALSE;		/* Write geomview comands */
int	quiet = TRUE;			/* quite mode */
float	clipmax = 10.0;			/* Clip values greater than this */
int	colour = -1;			/* Default colour */
int	dcolour = -1;			/* Default colour for discrim */
int	map_vectors = FALSE;		/* Vectors used in map */
int     edit_time = 0;                  /* time of last editing */

#define  CLIP(X) (X > clipmax ? clipmax :( X < -clipmax ? -clipmax : ( X != X ? clipmax : X) ))
#define  COL_CLIP(X) (X > 1.0 ? 1.0 :( X < 0 ? 0 : ( X != X ? 1 : X) ))

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

#ifndef NO_GEOM
Geom *GeomFLoad(FILE *fp,char *str) {} /* A dummy function as we
                                                don't link -lgeom */
#endif 

#ifdef X
#undef X
#undef Y
#undef Z
#undef W
#endif

#undef HUGE
#ifndef HUGE
#define HUGE 1e32
#endif


/*********** Argument handeling ***********************/

int	arg_do_crit = FALSE; /* Draw the critical set */
float arg_clipmax = 0.0;
int   arg_precision = -1;
double arg_xmin = HUGE, arg_xmax = HUGE, arg_ymin = HUGE, arg_ymax = HUGE;
int	arg_colour = -5;
int	arg_course = 0, arg_fine = 0;
double arg_vals[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* vals from arguments */
char   *arg_names[MAX_NUM_PARAMS + MAX_NUM_VARS];
int     arg_count=0;	/* number of params in arguments */
char	*arg_filename = NULL;	/* filename from arguments */
int	temp_flag = FALSE;	/* TRUE if equation def on command line */
char	temp_file_name[L_tmpnam];	/* temp file for equation */

print_usage(char *name)
{
if(prog_type == Acurve)
fprintf(stderr,"Usage: acurve [-v xl xh yl yh] [-r course] [-f fine] [-C col]\n");
else
fprintf(stderr,"Usage: discrim2 [-S] [-v xl xh yl yh] [-r course] [-f fine] [-c clip] [-C col]\n");
fprintf(stderr,"\t\t[-h] [-p precision] [-D name val] {-G|-I|-e equation|filename}\n");
}


psurf3_args(argc,argv)
int argc; char **argv;
{
#ifndef CGIVRML
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

	if( !strcmp(prog_name,"acurve") )
	{	prog_type = Acurve;
		map_eqns = FALSE;
		n_eqns = 1;  n_vars = 2;
	}
	else if( !strcmp(prog_name,"discrim2") )
	{	prog_type = Discrim;
		map_eqns = TRUE;
		n_eqns = 2; n_vars = 2;
	}
	else
	{
		fprintf(stderr,"bad program name: %s\n",prog_name);
		exit(-1);
	}

	/* Now we can look at the arguments */

	while((i=getopt(argc,argv,"GSIhe:c:p:D:r:f:v:C:")) != -1 )
	{
		switch(i)
		{
		case 'S': arg_do_crit = TRUE; break;
		case 'r': arg_course = atoi(optarg); break;
		case 'f': arg_fine = atoi(optarg); break;
		case 'v': arg_xmin = atof(argv[optind++ -1]);
                          arg_xmax = atof(argv[optind++ -1]);
                          arg_ymin = atof(argv[optind++ -1]);
                          arg_ymax = atof(argv[optind++ -1]);
                        --optind;
                        break;
		case 'c': arg_clipmax=atof(optarg); break;
		case 'C': arg_colour = atoi(optarg); break;
		case 'p': arg_precision = atoi(optarg); break;
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
#endif
}

/*
	This fun is called after file has been read to arguments
	are applied after file values
*/

use_arg_vals()
{
  int i,j;

  if(arg_precision != -1 ) precision = arg_precision;
  if(arg_clipmax != 0.0 ) clipmax = arg_clipmax;
  if(arg_course != 0) RESOLUTION = arg_course;
  if(arg_fine != 0) LINK_FACE_LEVEL = arg_fine;
  if(arg_colour != -5) colour = dcolour = arg_colour;
  if(arg_xmin != HUGE) xmin = arg_xmin;
  if(arg_xmax != HUGE) xmax = arg_xmax;
  if(arg_ymin != HUGE) ymin = arg_ymin;
  if(arg_ymax != HUGE) ymax = arg_ymax;

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

/******** read_cgi *****************/
/******** creates a tempory file arg_filename with all
        the stuff from the cgi request *****/


read_cgi()
{
        char *env_query;
        int cl,c;
        ncsa_entry entries[MAX_ENTRIES];
        register int x,m=0;
        char *def_ent,*coarse_ent,*fine_ent,*face_ent,*edge_ent,*timeout_ent;
        char *xmin_ent,*xmax_ent,*ymin_ent,*ymax_ent,*zmin_ent,*zmax_ent;
        char *version_ent;
        FILE    *temp_file;
        FILE *fp,*fl;
                char *tmp,*tstr; time_t tim;

        def_ent=coarse_ent=fine_ent=face_ent=edge_ent=timeout_ent=NULL;
        xmin_ent=xmax_ent=ymin_ent=ymax_ent=zmin_ent=zmax_ent=NULL;
        env_query = getenv("REQUEST_METHOD");
        if(env_query == NULL || env_query[0] == '\0' )
        {
                report_error(NO_REP_HEAD_ERROR,"NULL REQUEST METHOD",1);
                exit(1);
        }
        if(strcmp(env_query,"POST"))
        {
                report_error(HEAD_ERROR,"Request method should be POST",2);
                exit(1);
        }


        env_query = getenv("CONTENT_LENGTH");
        if(env_query == NULL || env_query[0] == '\0' )
        {
                report_error(HEAD_ERROR,"NULL CONTENT_LENGTH",1);
                exit(1);
        }
   cl = atoi(env_query);
#ifdef GDB
        fp = fopen("cgitest","r");
#else
        fp = stdin;
#endif
        fl = fopen("asurf.log","a");

                tmp = getenv("REMOTE_HOST");
                if(tmp != NULL ) fprintf(fl,"%s\t",tmp);
                else
                {
                        tmp = getenv("REMOTE_ADDR");
                        if(tmp != NULL ) fprintf(fl,"%s\t",tmp);
                        else fprintf(fl,"unknown\t");
                }
                tmp = getenv("HTTP_USER_AGENT");
                if(tmp != NULL ) fprintf(fl,"%s\t",tmp);
                else fprintf(fl,"unknown agent\t");

                time(&tim);
                tstr = ctime(&tim);
                tstr[19] = '\0';
                fprintf(fl,"%s\t",tstr);

    for(x=0;cl && (!feof(fp));x++) {
        m=x;
        entries[x].val = fmakeword(fp,'&',&cl);
        if(x>0) { fprintf(stderr,"&"); fprintf(fl,"&"); }
        fprintf(stderr,"%s",entries[x].val);
        fprintf(fl,"%s",entries[x].val);
        plustospace(entries[x].val);
        unescape_url(entries[x].val);

        entries[x].name = makeword(entries[x].val,'=');


        if(!strcmp(entries[x].name,"DEF"))          def_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"XMIN"))    xmin_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"XMAX"))    xmax_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"YMIN"))    ymin_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"YMAX"))    ymax_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"ZMIN"))    zmin_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"ZMAX"))    zmax_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"COARSE"))  coarse_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"FINE"))    fine_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"FACE"))    face_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"EDGE"))    edge_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"TIMEOUT")) timeout_ent = entries[x].val
;
        else if(!strcmp(entries[x].name,"VERSION")) version_ent = entries[x].val
;
        else 
        {    
                report_error2(HEAD_ERROR,"Bad field name %s",entries[x].name,3);
                exit(1);
        }    
    }   
        fprintf(stderr,"\n");
        fprintf(fl,"\n");
        fclose(fl);


        if(def_ent == NULL)
        {
                report_error(HEAD_ERROR,"A definition must be specified",12);
                exit(1);
        }
       arg_filename = tmpnam(temp_file_name);
       temp_file = fopen(arg_filename,"w");

        if(def_ent != NULL ) fprintf(temp_file,"%s\n",def_ent);
        if(xmin_ent != NULL && xmax_ent != NULL )
                fprintf(temp_file,"x = [%s,%s];\n",xmin_ent,xmax_ent);
        if(ymin_ent != NULL && ymax_ent != NULL )
                fprintf(temp_file,"y = [%s,%s];\n",ymin_ent,ymax_ent);

        if(coarse_ent != NULL )
                fprintf(temp_file,"_coarse = %s;\n",coarse_ent);
        if(fine_ent != NULL )
                fprintf(temp_file,"_fine = %s;\n",fine_ent);


        vrml_version = 1;
        if(version_ent != NULL)
                vrml_version = atoi(version_ent);

#ifdef TIMEOUT_LIMIT
        if(timeout <=0 || timeout > TIMEOUT_LIMIT )
        {
                report_error2(HEAD_ERROR,"Sorry your value for the timeout (%s) exceeds the allowable maximum.",timeout_ent,13);
                exit(1);
        }
#endif

       fclose(temp_file);
       temp_flag = TRUE;
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
}

/************* The guts actually does the work ***********************/

/* two functions called from the other c files */

int check_interupt(char *string)
{ return(FALSE);}

map(x,y,z)
float *x,*y,*z;
{
	double val,*ptr;

	if(do_map)
	{
		rpe_vals[0] = (double) *x;
		rpe_vals[1] = (double) *y;
		ptr = eval_vrpe(map_rpe,rpe_vals);
		val = (float) *ptr; *x = CLIP(val); 
		val = (float) *(ptr+1); *y = CLIP(val); 
		val = (float) *(ptr+2); *z = 0.0; 
	}
	else
		*z = 0.0;
}

/*
 * Function:	acurve
 * Action;	calculates the curve
 */

acurve()
{
	int	i,j,flag;
	float	val,xsize,ysize,dx,dy;
	int     offset;
	eqnode  *temp_eqn,*sub_eqn;
        double   a[MAXORDER][MAXORDER]; /* polynomial rep */
	int	xord,yord;

	double *ptr;

	if( main_eqn == NULL ||
	 ( map_eqns && map_rpe == NULL ) )
	{
		report_error(HEAD_ERROR,"Can't calculate curve  - bad equations\n",14);
		return;
	}
 
#ifndef NO_GEOM
	start_geom();
#endif

	temp_eqn = duplicate(main_eqn);
	eval_ops(temp_eqn);
	for(i=0;i<n_params;++i)
	{
                sub_eqn = assign( rpe_names[i+n_vars] , rpe_vals[i+n_vars] );
                substitute( temp_eqn, sub_eqn );
                free_eqn_tree( sub_eqn );
        }
	eval_funs(temp_eqn);
	expand(temp_eqn);
	init_poly2(a);
        if( !add_to_poly2( temp_eqn, a,rpe_names[0],rpe_names[1]) )
        {
                report_error(HEAD_ERROR,"Could not convert equation to polynomial form!\n",15);
                fprintf(stderr,"Could not convert equation to polynomial form!\n");
                fprintf(stderr,"Equation is:\n");
                fprint_eqn(stderr,temp_eqn);
                fprintf(stderr,";\n");
        }
        else
        {
                /**** Now the data ****/

		if(do_map)
                	initoogl(dcolour);
                else
			initoogl(colour);
                flag = marmain(a,xmin,xmax,ymin,ymax);

	if(vrml_version == 0) /* OFF */
	{
	printf("LIST\n");
	printf("COMMENT ");
	if( map_eqns )
	{
		if( do_map )
			 printf("descrim2");
		else
			printf("crit2");
	}
	else printf("acurve");

	printf(" LSMP_DEF {\n");
	if( map_eqns )
	{
		if( do_map )
			printf("# Discriminant of\n");
		else
			printf("# Critical Set of\n");
		printf(" "); print_eqn(map_eqn); printf(";\n");
	}
	else
	{
		printf("# Algebraic curve defined by\n");
        	printf(" "); print_eqn(main_eqn); printf(";\n");
	}
		
        for(i=0;i < n_params;++i)
                printf(" %s = %f;\n",rpe_names[i+n_vars],rpe_vals[i+n_vars]);
        printf(" %s = [%f,%f];\n",rpe_names[0],xmin,xmax);
        printf(" %s = [%f,%f];\n",rpe_names[1],ymin,ymax);
	if( map_eqns )
	{
	printf(" _clipping = %7.3f;\n",clipmax);
	printf(" _precision = %d;\n",precision);
	printf(" _course = %d;\n",RESOLUTION);
	printf(" _fine = %d;\n",LINK_FACE_LEVEL);
	printf(" _colour = %d;\n",colour);
	printf(" _dcolour = %d;\n",dcolour);
	} else
	{
	printf(" _precision = %d;\n",precision);
	printf(" _course = %d;\n",RESOLUTION);
	printf(" _fine = %d;\n",LINK_FACE_LEVEL);
	printf(" _colour = %d;\n",colour);
	}
	printf("}\n");
	printf("COMMENT asurf LSMP_EDIT_TIMESTAMP { %d }\n",edit_time);
	}
               else if(vrml_version == 1)
                {
                        printf("Content-type: x-world/x-vrml\n\n");

        printf("#VRML V1.0 ascii\n");
        printf("Info { string \"");
        printf("Model produce by the Liverpool Surface Modelling Package\n");
        printf("http://www.amsta.leeds.ac.uk/~rjm/lsmp/index.html\n");
        printf("Written by Richard Morrris rjm@amsta.leeds.ac.uk\n");
        printf("# Algebraic curve defined by\n");
        printf(" "); print_eqn(main_eqn); printf(";\n");
        for(i=0;i < n_params;++i)
                printf(" %s = %f;\n",rpe_names[i+n_vars],rpe_vals[i+n_vars]);
        printf(" %s = [%f,%f];\n%s = [%f,%f];\n",
                rpe_names[0],xmin,xmax,rpe_names[1],ymin,ymax);
        printf(" _coarse = %d;\n",RESOLUTION);
        printf(" _fine = %d;\n",LINK_FACE_LEVEL);
        printf("\" }\n");

                }
               else if(vrml_version == 2)
                {
                        printf("Content-type: model/vrml\n\n");

        printf("#VRML V2.0 utf8\n");
        printf("WorldInfo { title \"Algebraic Curve\"\n info \"");
        printf("Model produce by the Liverpool Surface Modelling Package\n");
        printf("http://www.amsta.leeds.ac.uk/~rjm/lsmp/index.html\n");
        printf("Written by Richard Morrris rjm@amsta.leeds.ac.uk\n");
        printf("# Algebraic surface defined by\n");
        printf(" "); print_eqn(main_eqn); printf(";\n");
        for(i=0;i < n_params;++i)
                printf(" %s = %f;\n",rpe_names[i+n_vars],rpe_vals[i+n_vars]);
        printf(" %s = [%f,%f];\n%s = [%f,%f];\n",
                rpe_names[0],xmin,xmax,rpe_names[1],ymin,ymax);
        printf(" _coarse = %d;\n",RESOLUTION);
        printf(" _fine = %d;\n",LINK_FACE_LEVEL);
        printf("\" }\n");
                }
               else if(vrml_version == 3)
                {
                        if(flag)
                        {
                                printf("Content-type: text/plain\n\n");
                                printf("OK Surface sucessfully calculated\n");
                        }

	print_jvx_header("acurve","Automatically generated algebraic curve");
        print_eqn(main_eqn); printf(";\n");
        for(i=0;i < n_params;++i)
                printf(" %s = %f;\n",rpe_names[i+n_vars],rpe_vals[i+n_vars]);
        printf(" %s = [%f,%f];\n%s = [%f,%f];\n",
                rpe_names[0],xmin,xmax,rpe_names[1],ymin,ymax);
        printf(" _coarse = %d;\n",RESOLUTION);
        printf(" _fine = %d;\n",LINK_FACE_LEVEL);
	print_jvx_header2("algebraic curve");

	printf(" <geometries>\n");
                }




                finioogl();

                if(vrml_version == 3)
                {
printf(" </geometries>\n");
printf("</jvx-model>\n");
                }
        }

        free_eqn_tree( temp_eqn );
#ifndef NO_GEOM
	fini_geom();
#endif
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
	eqnode	*X_eqn,*Y_eqn,*Z_eqn,*W_eqn,*temp,*temp2,*sub;
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

	if( main_eqn != NULL ) free_eqn_tree(main_eqn);
	if( map_eqns && map_eqn != NULL ) free_eqn_tree(map_eqn);
	clear_rpe_const();

	/* input the equations */

	if( ! map_eqns )
	{
		main_eqn = fscan_eqn(fp);
		if( main_eqn == NULL)
		{
			fprintf(stderr,"Bad equation:");
			fprint_eqn(stderr,main_eqn);
			fprintf(stderr,"\n");
			goto fini_read;
		}
	}
 
	if( map_eqns )	/* Now read eqns for mapping if required  */
	{
        X_eqn = fscan_eqn(fp);
        if( X_eqn == NULL )
        {
                fprintf(stderr,"Bad equation:");
                fprint_eqn(stderr,X_eqn);
                fprintf(stderr,"\nEquation of form R = ....; or (a,b,c) required.\n");
                X_eqn = NULL;
                goto fini_read;
        }
	if( X_eqn->op == '=')
	{
                Y_eqn = fscan_eqn(fp);
                if( Y_eqn == NULL || Y_eqn->op != '='
                 || Y_eqn->u.n.l->op != NAME )
                {
                        fprintf(stderr,"Bad equation:");
                        fprint_eqn(stderr,Y_eqn);
                        fprintf(stderr,"\nEquation of form Y = ....; required.\n");
                        X_eqn = NULL;
                        goto fini_read;
                }

		/* Can substitute these into main */

/*
		substitute(main_eqn,X_eqn);
		substitute(main_eqn,Y_eqn);
		substitute(main_eqn,Z_eqn);
	        map_eqn = join_eqns(',',X_eqn->u.n.r,
	                     join_eqns(',',Y_eqn->u.n.r,Z_eqn->u.n.r));
		free_eqn_node(Z_eqn->u.n.l); free_eqn_node(Z_eqn);
*/
	        map_eqn = join_eqns(',',X_eqn->u.n.r,Y_eqn->u.n.r);
		free_eqn_node(X_eqn->u.n.l); free_eqn_node(X_eqn);
		free_eqn_node(Y_eqn->u.n.l); free_eqn_node(Y_eqn);
		map_vectors = FALSE;
	}
	else
	{
		map_eqn = X_eqn;
		map_vectors = TRUE;
	}
	} /* finished reading mapping equations */

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
		if( map_eqns )
		{
		    if( !strcmp(temp->u.n.l->u.str, "discrim2_precision") )
                        precision = (int) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "discrim2_clipping"))
                        clipmax = (float) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "discrim2_course"))
                        RESOLUTION = (int) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "discrim2_fine"))
                        LINK_FACE_LEVEL = (int) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "discrim2_colour"))
                        colour = (int) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "discrim2_dcolour"))
                        dcolour = (int) temp->u.n.r->u.num;
		    else
		    {
			COPY_STRING(new_names[new_count],temp->u.n.l->u.str);
			new_vals[new_count] = temp->u.n.r->u.num;
			++new_count;
		    }
		}
		else
		{
		    if( !strcmp(temp->u.n.l->u.str, "acurve_precision") )
                        precision = (int) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "acurve_course"))
                        RESOLUTION = (int) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "acurve_fine"))
                        LINK_FACE_LEVEL = (int) temp->u.n.r->u.num;
                    else if( !strcmp(temp->u.n.l->u.str, "acurve_colour"))
                        colour = (int) temp->u.n.r->u.num;
		    else
		    {
			COPY_STRING(new_names[new_count],temp->u.n.l->u.str);
			new_vals[new_count] = temp->u.n.r->u.num;
			++new_count;
		    }
		} /* end if */
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
			++num_int_defs;
		}

		else    /* a macro definition */
                {
                        if( temp->u.n.l->op == NAME 
                         || ( temp->u.n.l->op == '*'
                           && temp->u.n.l->u.n.l->op == NAME ) )
                        {
				if( map_eqns )
					substitute(map_eqn,temp);
                                else
					substitute(main_eqn,temp);
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

	if( map_eqns )
		allnames = add_eqn_names(NULL,map_eqn);
	else
		allnames = add_eqn_names(NULL,main_eqn);

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

		
	if( map_eqns )
	{
		eval_ops(map_eqn);
		temp = duplicate(map_eqn);
		eval_funs(temp);
		temp2 = duplicate(temp);
		map_rpe = make_vrpe( temp, n_params + n_vars, rpe_names );
		diff_wrt(temp,rpe_names[0]);
		diff_wrt(temp2,rpe_names[1]);
		main_eqn = join_eqns('^',temp,temp2);
	}
	eval_ops(main_eqn);

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
	read_def(filename);
}

/****** Now some macros for the copy program *****/

#define write_to_end_of_line {\
	while((cc = getc(in)) != EOF && cc != '\n' )\
		if(cc == '<') fprintf(out,"&lt;"); \
		else if(cc == '>') fprintf(out,"&gt;"); \
		else putc(cc,out);\
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

	if( map_eqns )
	{
		write_till_semi_colon;
		if( !map_vectors )
		{
			write_till_semi_colon;
/*
			write_till_semi_colon;
*/
		}
	}
	else
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
			if( !strcmp(name,"acurve_precision")) dont_copy = TRUE;
			if( !strcmp(name,"acurve_fine")) dont_copy = TRUE;
			if( !strcmp(name,"acurve_course")) dont_copy = TRUE;
			if( !strcmp(name,"acurve_colour")) dont_copy = TRUE;
			if( !strcmp(name,"discrim2_precision")) dont_copy = TRUE;
			if( !strcmp(name,"discrim2_clipping")) dont_copy = TRUE;
			if( !strcmp(name,"discrim2_fine")) dont_copy = TRUE;
			if( !strcmp(name,"discrim2_course")) dont_copy = TRUE;
			if( !strcmp(name,"discrim2_colour")) dont_copy = TRUE;
			if( !strcmp(name,"discrim2_dcolour")) dont_copy = TRUE;

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

/************* Now call back functions ****************************/

#ifndef COMMAND_LINE
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
#ifndef NO_GEOM
	set_geom_name(argv[1]);
#endif
	strcpy(my_geom_name,argv[1]);
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

	for(i=0;i<n_params;++i)
		fprintf(fp," %s = %f;\n",
			rpe_names[i+n_vars],rpe_vals[i+n_vars]);

	if( map_eqns )
	{
	fprintf(fp," discrim2_clipping = %7.3f;\n",clipmax);
	fprintf(fp," discrim2_precision = %d;\n",precision);
	fprintf(fp," discrim2_course = %d;\n",RESOLUTION);
	fprintf(fp," discrim2_fine = %d;\n",LINK_FACE_LEVEL);
	fprintf(fp," discrim2_colour = %d;\n",colour);
	fprintf(fp," discrim2_dcolour = %d;\n",dcolour);
	} else {
	fprintf(fp," acurve_precision = %d;\n",precision);
	fprintf(fp," acurve_course = %d;\n",RESOLUTION);
	fprintf(fp," acurve_fine = %d;\n",LINK_FACE_LEVEL);
	fprintf(fp," acurve_colour = %d;\n",colour);
	}
	fclose(fp);
	fclose(fq);
#ifndef NO_GEOM
	set_geom_name(argv[1]);
#endif
	strcpy(my_geom_name,argv[1]);
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
	do_map = FALSE;
	acurve();
	return TCL_OK;
}

int crit_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	char string[255];

	strcpy(string,"crit{");
	strcat(string,my_geom_name);
	strcat(string,"}");
#ifndef NO_GEOM
	set_geom_name(string);
#endif
	do_map = FALSE;
	acurve();
	return TCL_OK;
}

int discrim_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	char string[255];

	strcpy(string,"disc{");
	strcat(string,my_geom_name);
	strcat(string,"}");
#ifndef NO_GEOM
	set_geom_name(string);
#endif
	do_map = TRUE;
	acurve();
	return TCL_OK;
}

int get_progname(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        sprintf(interp->result,"%s %d %d %d",prog_name,n_vars,n_eqns,map_eqn);
        return TCL_OK;
}

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        sprintf(interp->result,"%d %.2f %d %d %d %d\n", /*  %d  %d %d %d %d\n", */
                precision,clipmax,colour,dcolour,RESOLUTION,LINK_FACE_LEVEL);
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
        clipmax = atof(argv[2]);
        colour = atoi(argv[3]);
        dcolour = atoi(argv[4]);
	RESOLUTION = atoi(argv[5]);
	LINK_FACE_LEVEL = atoi(argv[6]);

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

	if( argc != 5)
        {
                interp->result = "set_variables: Wrong number of arguments";
                return TCL_ERROR;
        }
	xmin = atof(argv[1]);
	xmax = atof(argv[2]);
	ymin = atof(argv[3]);
	ymax = atof(argv[4]);
        return TCL_OK;
}

int get_variables(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        	sprintf(interp->result,"%s %f %f %s %f %f \n",
			rpe_names[0],xmin,xmax,
			rpe_names[1],ymin,ymax);
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
#ifndef NO_GEOM
	set_object_mode(argv[1]);
#endif
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
/* endif COMMAND_LINE */

/******* The main routine *********************************************/

main(argc, argv)        
int argc; char **argv;
{
	char	string[5],tcl_file[128],*tcl_dir;
	long	dev;
	short	val;
	int	i;
#ifndef COMMAND_LINE
        Tcl_Interp *interp;
        int code;
        Tk_Window wind;
#endif

#ifndef GDB
        freopen("asurf.error","a",stderr);
#endif


	/* First check permision */

#ifndef CGIVRML
	tcl_dir = getenv("LSMP_TCL");
	if( tcl_dir == NULL )
	{
		fprintf(stderr,
			"Could not find LSMP_TCL environment variable\n");
		exit(-1);
	}
	strcpy(tcl_file,tcl_dir);
	strcat(tcl_file,"/../bin/livlock");
	i = system(tcl_file);
	if(WEXITSTATUS(i) != 5 ) exit(-1);

	/* read in arguments */

	psurf3_args(argc,argv);
#endif
#ifdef CGIVRML
	read_cgi();
#endif

	init_funs();

	if( quiet )
	{
		read_file(arg_filename);
#ifndef CGIVRML
		use_arg_vals();
#endif
#ifndef NO_GEOM
                set_geom_name(arg_filename);
#endif
		if( prog_type != Acurve && ! arg_do_crit )
			do_map = TRUE;
		else
			do_map = FALSE;
		acurve();
		if( temp_flag ) unlink(temp_file_name);
		exit(0);
	}

#ifndef COMMAND_LINE
	/* If we don't foreground then the process forks and dies
	   as soon as we do graphics. This is bad.

		Even worse is to do this when trying to conect to
		an x terminal and running tk/tcl
	 */


        if( arg_filename != NULL)
        {
                read_file(arg_filename);
		use_arg_vals();
#ifndef NO_GEOM
                set_geom_name(arg_filename);
#endif
		strcpy(my_geom_name,arg_filename);
        }
	else
	{
#ifndef NO_GEOM
                set_geom_name(prog_name);
#endif
		strcpy(my_geom_name,prog_name);
	}

        interp = Tcl_CreateInterp();
/*
        wind = Tk_CreateMainWindow(interp,NULL,prog_name, "acurve");
	if(wind == NULL)
	{
		fprintf(stderr,"%s\n",interp->result);
		exit(-1);
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
        Tcl_CreateCommand(interp,"crit_cb",crit_cb,
                (ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
        Tcl_CreateCommand(interp,"discrim_cb",discrim_cb,
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

	use_arg_vals();

	tcl_dir = getenv("LSMP_TCL");
        strcpy(tcl_file,tcl_dir);
        strcat(tcl_file,"/acurve.tcl");

	code = Tcl_EvalFile(interp,tcl_file);
        if (code != TCL_OK) {
		fprintf(stderr,"%s\n",interp->result);
                exit(1);
        }

        Tk_MainLoop();

	if( temp_flag ) unlink(temp_file_name);
#endif
}

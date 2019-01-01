/*
 *      file:   main.c:   
 *      author: Rich Morris
 *      date:   Jan 5 1995
 *      
 *	main file for asurf program.
 */

/*
#define GCC_COMP
#include <color.h>
#include "normlist.h"
#include "geomsimp.h"
#define GDB
#define NO_TIMEOUT
*/
#define TIMEOUT_LIMIT 100000

#define NO_GEOMVIEW
#define CGIVRML
#define COMMAND_LINE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#ifndef CGIVRML
#include <sys/wait.h>
#endif
#include <time.h>
#include <eqn.h>
#include "../CVcommon.h"
#include <Multi.h>

#ifndef COMMAND_LINE
#include <tcl.h>
#include <tk.h>
#ifdef NO_GEOMVIEW
typedef void Geom;
extern Geom *GeomFLoad(FILE *fp,char *str); /* actually define in psurf.c */
#else
#include <geom.h>
#endif

#endif

/*
#define DISCRIM_CODE
*/

/* Names for options */

#define COL_NAME "colour"
#define PREC_NAME "precision"
#define COARSE_NAME "coarse"
#define FINE_NAME "fine"
#define EDGE_NAME "edges"
#define FACE_NAME "faces"

#define IMPSDEF_NAME "LSMP_DEF"
#define IMPSETIME_NAME "LSMP_EDIT_TIMESTAMP"

/* Default values */

#define XMIN_DEFAULT -1.14
#define XMAX_DEFAULT 1.03
#define YMIN_DEFAULT -1.13
#define YMAX_DEFAULT 1.04
#define ZMIN_DEFAULT -1.12
#define ZMAX_DEFAULT 1.05

#define COARSE_DEFAULT 8
#define FINE_DEFAULT 16
#define FACE_DEFAULT 64
#define EDGE_DEFAULT 128
#define PREC_DEFAULT 1

/* Colours for curve/surface avoid clash with 0-7 for black-white */
#define NO_COL -1
#define EQN_COL -2
#define STD_COL 0

#define ABORT 1
#define FLUSH 2

#define grballoc(node) (node *) malloc( sizeof(node) )

/* Three parameters which effect the execution */

extern unsigned int RESOLUTION,LINK_FACE_LEVEL,LINK_SING_LEVEL,SUPER_FINE;


char	prog_name[10];	/* Name prog was started with */
enum	{Impsurf } prog_type; /* do discrim or asurf? */

Multi	*main_mul;
#ifdef DISCRIM_CODE
eqnode *map_eqn = NULL;	/* The main eqn */
#endif

#ifdef DISCRIM_CODE
int	*map_rpe;		/* The rpe string for mapping */
#endif
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
int	colour = -1;			/* Default colour */
#ifdef DISCRIM_CODE
float	clipmax = 10.0;			/* Clip values greater than this */
int	map_vectors = FALSE;		/* mapping eqs defined with vectors */
#endif
int     edit_time = 0;                  /* time of last editing */

#define  CLIP(X) (X > clipmax ? clipmax :( X < -clipmax ? -clipmax : ( X != X ? clipmax : X) ))
#define  COL_CLIP(X) (X > 1.0 ? 1.0 :( X < 0 ? 0 : ( X != X ? 1 : X) ))

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

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

#ifdef GCC_COMP
#ifdef HUGE
#undef HUGE
#define HUGE 12345678.0
#endif
#endif
#ifndef HUGE
#define HUGE 1e32
#endif

/*********** Argument handeling ***********************/

float arg_clipmax = 0.0;
int   arg_precision = -1;
double arg_xmin = HUGE, arg_xmax = HUGE, arg_ymin = HUGE, arg_ymax = HUGE,
	arg_zmin = HUGE, arg_zmax = HUGE;
double arg_vals[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* vals from arguments */
char   *arg_names[MAX_NUM_PARAMS + MAX_NUM_VARS];
int     arg_count=0;	/* number of params in arguments */
char	*arg_filename = NULL;	/* filename from arguments */
int	temp_flag = FALSE;	/* TRUE if equation def on command line */
char	temp_file_name[L_tmpnam];	/* temp file for equation */

int	timeout = 1000;	/* The number of milliseconds which can pass
				before the program times out */
int	vrml_version = 1; /* the version of VRML produced */

unsigned int arg_RESOLUTION=0,arg_LINK_FACE_LEVEL=0,
	     arg_LINK_SING_LEVEL=0,arg_SUPER_FINE=0;

print_usage(char *name)
{
	fprintf(stderr,"Usage: %s [-v xl xh yl yh zl zh] [-c coarse] [-f fine] [-F faces] [-E edges]\n",name);
	fprintf(stderr,"\t\t[-p precision] [-h] [-D name val] {-G|-I|-e equation|filename}\n");
}

#ifndef CGIVRML
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
	else	      strcpy(prog_name,slash+1);

	if( !strcmp(prog_name,"impsurf") )
	{
	}
	else
	{
		fprintf(stderr,"bad program name: %s\n",prog_name);
		exit(-1);
	}

	/* Now we can look at the arguments */

    while((i=getopt(argc,argv,"hGIe:c:F:E:f:p:D:v:")) != -1)
    {
		switch(i)
		{
		case 'c': arg_RESOLUTION = atoi(optarg); break;
		case 'F': arg_LINK_FACE_LEVEL =atoi(optarg); break;
		case 'E': arg_SUPER_FINE =atoi(optarg); break;
		case 'f': arg_LINK_SING_LEVEL =atoi(optarg); break;
		case 'p': arg_precision = atoi(optarg); break;
		case 'v':
			arg_xmin = atof(argv[optind++ -1]);
			arg_xmax = atof(argv[optind++ -1]);
			arg_ymin = atof(argv[optind++ -1]);
			arg_ymax = atof(argv[optind++ -1]);
			arg_zmin = atof(argv[optind++ -1]);
			arg_zmax = atof(argv[optind++ -1]);
			--optind;
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
#endif


/*
	This fun is called after file has been read to arguments
	are applied after file values
*/

use_arg_vals()
{
  int i,j;


  if(arg_RESOLUTION != 0 )
	Mset_opt_val_by_name(main_mul,COARSE_NAME,(double) arg_RESOLUTION);
  if(arg_LINK_SING_LEVEL != 0 )
	Mset_opt_val_by_name(main_mul,FINE_NAME,(double) arg_LINK_SING_LEVEL);
  if(arg_LINK_FACE_LEVEL != 0 )
	Mset_opt_val_by_name(main_mul,FACE_NAME,(double) arg_LINK_FACE_LEVEL);
  if(arg_SUPER_FINE != 0 )
	Mset_opt_val_by_name(main_mul,EDGE_NAME,(double) arg_SUPER_FINE);
  if(arg_precision != -1 )
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) arg_precision);
  if(arg_xmin != HUGE) Mset_var_minmax(main_mul,0,arg_xmin,arg_xmax);
  if(arg_ymin != HUGE) Mset_var_minmax(main_mul,1,arg_ymin,arg_ymax);
  if(arg_zmin != HUGE) Mset_var_minmax(main_mul,2,arg_zmin,arg_zmax);

  for(i=0;i<arg_count;++i)
  {
	Mset_param_val_by_name(main_mul,arg_names[i],arg_vals[i]);
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
	FILE	*temp_file;
	FILE *fp,*fl = NULL;	
		char *tmp,*tstr; time_t tim;

	def_ent=coarse_ent=fine_ent=face_ent=edge_ent=timeout_ent=NULL;
	version_ent=xmin_ent=xmax_ent=ymin_ent=ymax_ent=zmin_ent=zmax_ent=NULL;
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
		if(tmp != NULL ) { if(fl!=NULL ) fprintf(fl,"%s\t",tmp); }
		else
		{
			tmp = getenv("REMOTE_ADDR");
			if(tmp != NULL ) fprintf(fl,"%s\t",tmp);
			else { if(fl!=NULL ) fprintf(fl,"unknown\t"); }
		}
		tmp = getenv("HTTP_USER_AGENT");
		if(tmp != NULL ) fprintf(fl,"%s\t",tmp);
		else { if(fl!=NULL ) fprintf(fl,"unknown agent\t"); }

		time(&tim);
		tstr = ctime(&tim);
		tstr[19] = '\0';
		{ if(fl!=NULL ) fprintf(fl,"%s\t",tstr); }

    for(x=0;cl && (!feof(fp));x++) {
        m=x;
        entries[x].val = fmakeword(fp,'&',&cl);
	if(x>0) { fprintf(stderr,"&"); fprintf(fl,"&"); }
	fprintf(stderr,"%s",entries[x].val);
	{ if(fl!=NULL ) fprintf(fl,"%s",entries[x].val); }
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
        else if(!strcmp(entries[x].name,"DRAWLINES")) { } 
        else if(!strcmp(entries[x].name,"TIMEOUT")) timeout_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"VERSION")) version_ent = entries[x].val;
        else 
        {    
                report_error2(HEAD_ERROR,"Bad field name %s",entries[x].name,3);
                exit(1);
        }    
    }   
	fprintf(stderr,"\n");
	{ if(fl!=NULL ) fprintf(fl,"\n"); }
	{ if(fl!=NULL ) fclose(fl); }

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
	if(zmin_ent != NULL && zmax_ent != NULL )
		fprintf(temp_file,"z = [%s,%s];\n",zmin_ent,zmax_ent);

	if(coarse_ent != NULL )
		fprintf(temp_file,"asurf_coarse = %s;\n",coarse_ent);
	if(fine_ent != NULL )
		fprintf(temp_file,"asurf_fine = %s;\n",fine_ent);
	if(face_ent != NULL )
		fprintf(temp_file,"asurf_face = %s;\n",face_ent);
	if(edge_ent != NULL )
		fprintf(temp_file,"asurf_edge = %s;\n",edge_ent);

	timeout = 1000;
	if(timeout_ent != NULL)
		timeout = atoi(timeout_ent);

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
	MsetNvars(main_mul,3);
	Mset_var_name(main_mul,0,"x");
	Mset_var_name(main_mul,1,"y");
	Mset_var_name(main_mul,2,"z");
	Mset_var_minmax(main_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
	Mset_var_minmax(main_mul,1,YMIN_DEFAULT,YMAX_DEFAULT);
	Mset_var_minmax(main_mul,2,ZMIN_DEFAULT,ZMAX_DEFAULT);
	Madd_opt(main_mul,PREC_NAME,(double) PREC_DEFAULT);
	Madd_opt(main_mul,COARSE_NAME,(double) COARSE_DEFAULT);
	Madd_opt(main_mul,FINE_NAME,(double) FINE_DEFAULT);
	Madd_opt(main_mul,FACE_NAME,(double) FACE_DEFAULT);
	Madd_opt(main_mul,EDGE_NAME,(double) EDGE_DEFAULT);
}

/***** The function and derivatives *****/

double imp_fun(double x, double y, double z)
{
	double *ptr,val;

	Mset_var_val(main_mul,0,x);
	Mset_var_val(main_mul,1,y);
	Mset_var_val(main_mul,2,z);
	MstartEval(main_mul);
	ptr = MevalTop(main_mul,0);
	val = (double) *ptr;
	return(val);
}

double imp_df_dx(double x, double y, double z)
{
	double *ptr,val;

	Mset_var_val(main_mul,0,x);
	Mset_var_val(main_mul,1,y);
	Mset_var_val(main_mul,2,z);
	MstartEval(main_mul);
	ptr = MevalTopDeriv(main_mul,0,0);
	val = (double) *ptr;
	return(val);
}

double imp_df_dy(double x, double y, double z)
{
	double *ptr,val;

	Mset_var_val(main_mul,0,x);
	Mset_var_val(main_mul,1,y);
	Mset_var_val(main_mul,2,z);
	MstartEval(main_mul);
	ptr = MevalTopDeriv(main_mul,0,1);
	val = (double) *ptr;
	return(val);
}

double imp_df_dz(double x, double y, double z)
{
	double *ptr,val;

	Mset_var_val(main_mul,0,x);
	Mset_var_val(main_mul,1,y);
	Mset_var_val(main_mul,2,z);
	MstartEval(main_mul);
	ptr = MevalTopDeriv(main_mul,0,2);
	val = (double) *ptr;
	return(val);
}

/************* The guts actually does the work ***********************/

/*
 * Function:	asurf
 * Action;	calculates the surface
 */

asurf(char *filename)
{
	int     i = 0,flag;
	char    *name;
	double	xmin,xmax,ymin,ymax,zmin,zmax;
	FILE	*fp;

	if(main_mul->error )
	{
		fprintf(stderr,"Can't calculate surface - syntax error in equations\n");
		return;
	}
	xmin =  Mget_var_min(main_mul,0);
	xmax =  Mget_var_max(main_mul,0);
	ymin =  Mget_var_min(main_mul,1);
	ymax =  Mget_var_max(main_mul,1);
	zmin =  Mget_var_min(main_mul,2);
	zmax =  Mget_var_max(main_mul,2);
	
	RESOLUTION = (int) Mget_opt_val_by_name(main_mul,COARSE_NAME);
	LINK_SING_LEVEL = (int) Mget_opt_val_by_name(main_mul,FINE_NAME);
	LINK_FACE_LEVEL = (int) Mget_opt_val_by_name(main_mul,FACE_NAME);
	SUPER_FINE = (int) Mget_opt_val_by_name(main_mul,EDGE_NAME);

	/**** First some comments ****/

	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read definition file %s\n",filename);
		return;
	} 
	

#ifndef COMMAND_LINE
	start_geom();
#endif

	/**** Now the data ****/

	initoogl();
/*
	fprintf(stderr,"f(%f,%f,%f)=%f\n",
			xmin,ymin,zmin, imp_fun(xmin,ymin,zmin) );
	fprintf(stderr,"f(%f,%f,%f)=%f\n",
			xmax,ymax,zmax, imp_fun(xmax,ymax,zmax) );
*/
	set_asurf_sigs();
	flag = marmain(imp_fun,imp_df_dx,imp_df_dy,imp_df_dz,xmin,xmax,ymin,ymax,zmin,zmax);
	reset_asurf_sigs();

		if(vrml_version == 0) /* OFF */
		{
	printf("LIST\n");
	printf("COMMENT asurf LSMP_DEF {\n");
        printf("# Algebraic surface defined by\n");

	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);

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
        printf("# Algebraic surface defined by\n");

	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("\" }\n");

		}
		else if(vrml_version == 2)
		{
			printf("Content-type: model/vrml\n\n");

	printf("#VRML V2.0 utf8\n");
	printf("WorldInfo { title \"Algebraic Surface\"\n info \"");
	printf("Model produce by the Liverpool Surface Modelling Package\n");
	printf("http://www.amsta.leeds.ac.uk/~rjm/lsmp/index.html\n");
	printf("Written by Richard Morrris rjm@amsta.leeds.ac.uk\n");
        printf("# Algebraic surface defined by\n");

	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
		}
		else if(vrml_version == 3)
		{
			if(flag) 
			{
				printf("Content-type: text/plain\n\n");
				printf("OK Surface sucessfully calculated\n");
			}


	print_jvx_header("impsurf","Automatically generated implicit surface");
        copy_def(fp,stdout);
        fprint_Mopts(stdout,main_mul);
        fclose(fp);
	print_jvx_header2("Implicit surface");

	printf(" <geometries>\n");
		}

                finioogl();

		if(vrml_version == 3)
		{
printf(" </geometries>\n");
printf("</jvx-model>\n");
		}

#ifndef COMMAND_LINE
        fini_geom();
#endif
}


/********* Now functions to perform the mapping ****************/

#ifdef DISCRIM_CODE
/*
 * Transform an individual point
 */

PointMap(pl,nl,cl)
HPoint3 *pl; Point3 *nl; ColorA *cl;
{
	float	val;
	double	a,b,c,d,e,f,g,h,i,v0,v1,v2,len;
	int     offset;

	double *ptr;

	/* Set the variables */

	if( proj_type == ASURF ) return;

	/* must now be a discrim */

	offset = 0;
	rpe_vals[offset++] = (double) pl->x;
	rpe_vals[offset++] = (double) pl->y;
	rpe_vals[offset++] = (double) pl->z;
#if 0 
	if( fourD_inputs ) 
		rpe_vals[offset++] = (double) pl->w;
	if( col_eqns )
	{
		rpe_vals[offset++] = (double) cl->r;
		rpe_vals[offset++] = (double) cl->g;
		rpe_vals[offset++] = (double) cl->b;
		rpe_vals[offset++] = (double) cl->a;
	}
	if( norm_eqns )
	{
		rpe_vals[offset++] = (double) nl->x;
		rpe_vals[offset++] = (double) nl->y;
		rpe_vals[offset++] = (double) nl->z;
	}
#endif

	/* Evaluate equations */

	ptr = eval_vrpe(main_rpe,rpe_vals);
	val = (float) *ptr; pl->x = CLIP(val); 
	val = (float) *(ptr+1); pl->y = CLIP(val); 
	val = (float) *(ptr+2); pl->z = CLIP(val); 
#if 0
	if( fourD_eqns) 
		val = (float) *(ptr+3); pl->w = CLIP(val); 

	if( col_eqns )
	{
		ptr = eval_vrpe(col_rpe,rpe_vals);
		val = (float) *(ptr+0); cl->r = COL_CLIP(val); 
		val = (float) *(ptr+1); cl->g = COL_CLIP(val); 
		val = (float) *(ptr+2); cl->b = COL_CLIP(val); 
		val = (float) *(ptr+3); cl->a = COL_CLIP(val); 
	}
	if( norm_eqns )
	{
		ptr = eval_vrpe(norm_rpe,rpe_vals);
		val = (float) *(ptr+0); nl->x = CLIP(val); 
		len = val*val;
		val = (float) *(ptr+1); nl->y = CLIP(val); 
		len += val*val;
		val = (float) *(ptr+2); nl->z = CLIP(val); 
		len += val*val;
		len = sqrt(len);
		if(len == 0 || len != len)
		{
			nl->x = (float) 0.0;
			nl->y = (float) 0.0;
			nl->z = (float) 0.0;
		}
		else if( len == len ) /* Propper numbers */
		{
			nl->x /= len;
			nl->y /= len;
			nl->z /= len;
		}
	}
	else if( normals ) /* calc normals from main_eqn */
	{
#endif
		/* Now transform the normals */
	
		ptr = eval_vrpe(dx_rpe,rpe_vals);
		a = *ptr; d = *(ptr+1); g = *(ptr+2);
		ptr = eval_vrpe(dy_rpe,rpe_vals);
		b = *ptr; e = *(ptr+1); h = *(ptr+2);
		ptr = eval_vrpe(dz_rpe,rpe_vals);
		c = *ptr; f = *(ptr+1); i = *(ptr+2);
	
	/* The map of tangent spaces sends 
	*	I --> a I + d J + g K
	*	J --> b I + e J + h K
	*	K --> c I + f J + i K
	*
	*   So for normals we have
	*	I = J^K --> (b I + e J + h K)^(c I + f J + i K)
	*		=  (e i - f h)I - (b i - c h)J + (b f - c e)K
	*	J     --> -(d i - f g)I + (a i - c g)J - (a f - c d)K
	*	K     -->  (d h - e g)I - (a h - b g)J + (a e - b d)K
	*
	*   and likewise for linear combs of I,J,K 
	*
	*/
	
		v0 =  (e*i-f*h)*nl->x-(d*i-f*g)*nl->y+(d*h-e*g)*nl->z;
		v1 = -(b*i-c*h)*nl->x+(a*i-c*g)*nl->y-(a*h-b*g)*nl->z;
		v2 =  (b*f-c*e)*nl->x-(a*f-c*d)*nl->y+(a*e-b*d)*nl->z;
		
		len = sqrt(v0*v0 + v1*v1 + v2*v2);

		if( len < 0.001 )
		{
/*
			fprintf(stderr,"len %f\n",len);
			fprintf(stderr,"point %f %f %f\n",
				rpe_vals[0],rpe_vals[1],rpe_vals[2]);
		}
		if(len == 0)
		{
*/
			nl->x = (float) 0.0;
			nl->y = (float) 0.0;
			nl->z = (float) 0.0;
		}
		else if( len == len ) /* Propper numbers */
		{
			nl->x = (float) v0/len;
			nl->y = (float) v1/len;
			nl->z = (float) v2/len;
		}	/* For NaN do nothing */
#if 0
	}
#endif
}

/*
 * Function;	limit_normal
 * Action:	calculates the normal when the surface is singular
 */

LimitMap(pt,norm)
HPoint3 *pt;
Point3 *norm;
{
	double	a,b,c,d,e,f,g,h,i,len1,len2;
	double  vec1[3],vec2[3],vec3[3],vec4[3],vec5[3],vec6[3];
	double  *ptr;
	int	offset;

	/* First we must calculate the two tangent vectors
		try I^norm,j^norm,k^norm
	*/

	if(fabs(norm->x) > fabs(norm->y) )
	{
		if(fabs(norm->x) > fabs(norm->z) ) /* x > y, x > z */
		{
			vec2[0] = norm->z;
			vec2[1] = 0.0;
			vec2[2] = -norm->x;
			vec2[0] = -norm->y;
			vec2[1] = norm->x;
			vec2[2] = 0.0;
		}
		else				/* z > x > y */
		{
			vec1[0] = 0.0;
			vec1[1] = -norm->z;
			vec1[2] = norm->y;
			vec2[0] = norm->z;
			vec2[1] = 0.0;
			vec2[2] = -norm->x;
		}
	}
	else
	{
		if(fabs(norm->y) > fabs(norm->z) ) /* y > z; y > x */
		{
			vec1[0] = 0.0;
			vec1[1] = -norm->z;
			vec1[2] = norm->y;
			vec2[0] = -norm->y;
			vec2[1] = norm->x;
			vec2[2] = 0.0;
		}
		else				/* z > y > x */
		{
			vec1[0] = 0.0;
			vec1[1] = -norm->z;
			vec1[2] = norm->y;
			vec2[0] = norm->z;
			vec2[1] = 0.0;
			vec2[2] = -norm->x;
		}
	}
#ifdef PRINT_LIMIT
fprintf(stderr,"point %f %f %f %f norm %f %f %f\nvec1 %f %f %f vec2 %f %f %f\n",
		pt->x,pt->y,pt->z,pt->w,
		norm->x,norm->y,norm->z,
		vec1[0],vec1[1],vec1[2],
		vec2[0],vec2[1],vec2[2]);
#endif
	/* Now we calculate the mapped vectors */

	offset = 0;
	rpe_vals[offset++] = (double) pt->x;
	rpe_vals[offset++] = (double) pt->y;
	rpe_vals[offset++] = (double) pt->z;

	ptr = eval_vrpe(dx_rpe,rpe_vals);
	a = *ptr; d = *(ptr+1); g = *(ptr+2);
	ptr = eval_vrpe(dy_rpe,rpe_vals);
	b = *ptr; e = *(ptr+1); h = *(ptr+2);
	ptr = eval_vrpe(dz_rpe,rpe_vals);
	c = *ptr; f = *(ptr+1); i = *(ptr+2);

	vec3[0] = a * vec1[0] + b * vec1[1] + c * vec1[2];
	vec3[1] = d * vec1[0] + e * vec1[1] + f * vec1[2];
	vec3[2] = g * vec1[0] + h * vec1[1] + i * vec1[2];

	vec4[0] = a * vec2[0] + b * vec2[1] + c * vec2[2];
	vec4[1] = d * vec2[0] + e * vec2[1] + f * vec2[2];
	vec4[2] = g * vec2[0] + h * vec2[1] + i * vec2[2];

	len1 = sqrt(vec3[0]*vec3[0]+vec3[1]*vec3[1]+vec3[2]*vec3[2]);
	len2 = sqrt(vec4[0]*vec4[0]+vec4[1]*vec4[1]+vec4[2]*vec4[2]);

#ifdef PRINT_LIMIT
fprintf(stderr,"len1 %f len2 %f\nvec3 %f %f %f vec4 %f %f %f\n",
		len1,len2,
		vec3[0],vec3[1],vec3[2],
		vec4[0],vec4[1],vec4[2]);
#endif

	/* We now have the two different methods for calc the
	    limit, in both vec6 is the requre tangent vector */

	vec6[0] = a * norm->x + b * norm->y + c * norm->z;
	vec6[1] = d * norm->x + e * norm->y + f * norm->z;
	vec6[2] = g * norm->x + h * norm->y + i * norm->z;

#ifdef PRINT_LIMIT
fprintf(stderr,"vec5 %f %f %f vec6 %f %f %f\n",
		vec5[0],vec5[1],vec5[2],
		vec6[0],vec6[1],vec6[2]);
#endif

	/* And finally the normal */

	if( fabs(len1) > fabs(len2) )
	{
		norm->x = vec3[1]*vec6[2] - vec3[2]*vec6[1];
		norm->y = vec3[2]*vec6[0] - vec3[0]*vec6[2];
		norm->z = vec3[0]*vec6[1] - vec3[1]*vec6[0];
	}
	else
	{
		norm->x = vec4[1]*vec6[2] - vec4[2]*vec6[1];
		norm->y = vec4[2]*vec6[0] - vec4[0]*vec6[2];
		norm->z = vec4[0]*vec6[1] - vec4[1]*vec6[0];
	}
	
	len1 = sqrt(norm->x*norm->x + norm->y*norm->y + norm->z*norm->z);
	if( len1 == len1 && len1 != 0.0)
	{
		norm->x /= len1;
		norm->y /= len1;
		norm->z /= len1;
	}
	else
	{
		norm->x = norm->y = norm->z = 0.0;
	}
}	
#endif

/********* file handeling ***********************/

int read_def(FILE *fp)
{
	edit_time = 0;
	if( !fscanMulti(fp,main_mul) ) return(FALSE);
	if( !MfindOpts(main_mul) ) return(FALSE);
	MsetNtop(main_mul,1);
	MsetTopDim(main_mul,0,-1);
	if( !McombineTop(main_mul) ) return(FALSE);
	if( !MfindNames(main_mul) ) return(FALSE);

	MsetNtopDerivs(main_mul,0,3);
	MsetDerivName(main_mul,0,0,Mget_var_name(main_mul,0));
	MsetDerivName(main_mul,0,1,Mget_var_name(main_mul,1));
	MsetDerivName(main_mul,0,2,Mget_var_name(main_mul,2));

	if( !McheckDims(main_mul) ) return(FALSE);
	if( !McalcDerivs(main_mul) ) return(FALSE);
	if( !McalcRPEs(main_mul) ) return(FALSE);

	edit_time = time(NULL);
	return(TRUE);
}

/*
 * When we read form a file we want to reset the variable names
 */

read_file(FILE *fp)
{
	int i;
	Mclear(main_mul);
	MsetNvars(main_mul,3);
	Mset_var_name(main_mul,0,"x");
	Mset_var_name(main_mul,1,"y");
	Mset_var_name(main_mul,2,"z");
	Mset_var_minmax(main_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
	Mset_var_minmax(main_mul,1,YMIN_DEFAULT,YMAX_DEFAULT);
	Mset_var_minmax(main_mul,2,ZMIN_DEFAULT,ZMAX_DEFAULT);
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) PREC_DEFAULT);
	Mset_opt_val_by_name(main_mul,COARSE_NAME,(double) COARSE_DEFAULT);
	Mset_opt_val_by_name(main_mul,FINE_NAME,(double) FINE_DEFAULT);
	Mset_opt_val_by_name(main_mul,FACE_NAME,(double) FACE_DEFAULT);
	Mset_opt_val_by_name(main_mul,EDGE_NAME,(double) EDGE_DEFAULT);
        i = read_def(fp);
        return(i);
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

			dont_copy = Mmatch_opt(main_mul,name);
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
 * Function:    asurf_sig
 * Action:      handles signals
 */

void asurf_sig(int sig,int code)
{
        pid_t pid;

#ifndef CGIVRML
        if(sig == SIGTSTP )
        {
                flushoogl();
#ifndef COMMAND_LINE
                fini_geom();
#endif
                pid = getpid();
                fprintf(stderr,"%s\n",laststring);
                signal(SIGCONT,asurf_sig);
                kill(pid,SIGTSTP);
        }
        else if(sig == SIGCONT )
        {
#ifndef COMMAND_LINE
                start_geom();
#endif
                signal(SIGTSTP,asurf_sig);
                fprintf(stderr,"%s\n",laststring);
                fprintf(stderr,"continuing\n");
                pid = getpid();
                kill(pid,SIGCONT);
        }
        else
#endif
        {
                signal(sig,SIG_IGN);
                finioogl();
#ifndef COMMAND_LINE
                fini_geom();
#endif
                exit(-1);
        }
}

/* called before the main calculation */

set_asurf_sigs()
{
#ifndef CGIVRML
        signal(SIGHUP,asurf_sig);
        signal(SIGINT,asurf_sig);
        signal(SIGTERM,asurf_sig);
        signal(SIGTSTP,asurf_sig);
#endif
}

/* called after the main calculation */

reset_asurf_sigs()
{
#ifndef CGIVRML
        signal(SIGHUP,SIG_DFL);
        signal(SIGINT,SIG_DFL);
        signal(SIGTERM,SIG_DFL);
        signal(SIGTSTP,SIG_DFL);
#endif
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
	clock_t ellapsed;
	static  clock_t initial_clock=0;
        if( string != NULL ) strcpy(laststring,string);
#ifdef CGIVRML
	if(initial_clock == 0) initial_clock = clock();
	ellapsed = ( (clock()-initial_clock) * 1000 ) / CLOCKS_PER_SEC ;
/*
fprintf(stderr,"clock %d initial %d ellasped %d max %d\n",clock(),initial_clock,ellapsed,timeout);
*/
#ifndef NO_TIMEOUT
	if( ellapsed > timeout )
	{
		fprintf(stderr,"timeout %d max time %d clock %d %d\n",ellapsed,timeout,clock(),initial_clock);
		report_error(MIDDLE_ERROR,"Process took longer than maximium allowable time.",15);
		return(TRUE);
	}
#endif
/*
*/
#endif
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
#ifndef COMMAND_LINE
                        fini_geom();
#endif
                        exit(-1);
                }
        }
*/
        if( int_status == ABORT )
                 return(TRUE);
        else if( int_status == FLUSH )
        {
                flushoogl();
#ifndef COMMAND_LINE
                fini_geom();
                start_geom();
#endif
        }
#endif
        return( FALSE );
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

#ifndef COMMAND_LINE

int load_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp,*fq;
	int offset = 0;

	if(argc != 3)
	{
		interp->result = "Wrong number of arguments";
		return TCL_ERROR;
	}

	fp = fopen(argv[1],"r");
	if( fp == NULL )
	{
		interp->result = "Could not open file";
		return TCL_ERROR;
	}
	read_file(fp);
	rewind(fp);
	fq = fopen(argv[2],"w");
	copy_def(fp,fq);
	fclose(fp);
	fclose(fq);
#ifndef COMMAND_LINE
	set_geom_name(argv[1]);
#endif
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
	fprint_Mopts(fp,main_mul);
	fclose(fp);
	fclose(fq);
#ifndef COMMAND_LINE
	set_geom_name(argv[1]);
#endif
	return TCL_OK;
}

/*
 * Update the equations, called when editor is changed
 *	given the name of editor file
 */

int update_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp,*fq;
	int i = 0;

	if(argc != 2)
	{
		interp->result = "Wrong number of arguments";
		return TCL_ERROR;
	}
        fp = fopen(argv[1],"r");
        if( fp == NULL )
        {
                interp->result = "Could not open file";
                return TCL_ERROR;
        }
        i =read_def(fp);
        fclose(fp);
	return TCL_OK;
}

/*
 * Run the program
 */

int run_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        if(argc != 2)
        {
                interp->result = "run_cb: Wrong number of arguments";
                return TCL_ERROR;
        }

	asurf(argv[1]);
	return TCL_OK;
}

int get_progname(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	sprintf(interp->result,"%s %d %d %d",prog_name,3,1);
	return TCL_OK;
}

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int precision,course,fine,face,edge;

	precision = (int) Mget_opt_val_by_name(main_mul,PREC_NAME);
	course = (int) Mget_opt_val_by_name(main_mul,COARSE_NAME);
	fine = (int) Mget_opt_val_by_name(main_mul,FINE_NAME);
	face = (int) Mget_opt_val_by_name(main_mul,FACE_NAME);
	edge = (int) Mget_opt_val_by_name(main_mul,EDGE_NAME);
	
	sprintf(interp->result,"%d %d %d %d %d\n",
		precision,course,fine,face,edge);
	return TCL_OK;
}

int set_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int i;

	if(argc != 6)
	{
		interp->result = "set_options: Wrong number of arguments";
		return TCL_ERROR;
	}
	Mset_opt_val_by_name(main_mul,PREC_NAME,atof(argv[1]));
	Mset_opt_val_by_name(main_mul,COARSE_NAME,atof(argv[2]));
	Mset_opt_val_by_name(main_mul,FINE_NAME,atof(argv[3]));
	Mset_opt_val_by_name(main_mul,FACE_NAME,atof(argv[4]));
	Mset_opt_val_by_name(main_mul,EDGE_NAME,atof(argv[5]));

	return TCL_OK;
}

int get_num_params(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	sprintf(interp->result,"%d",Mget_n_params(main_mul));
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
	Mset_param_val(main_mul,atoi(argv[1]),atof(argv[2]));
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
                Mget_param_name(main_mul,i),
                Mget_param_val(main_mul,i));
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
	Mset_var_minmax(main_mul,0,atof(argv[1]), atof(argv[2]));
	Mset_var_minmax(main_mul,1,atof(argv[3]), atof(argv[4]));
	Mset_var_minmax(main_mul,2,atof(argv[5]), atof(argv[6]));
	return TCL_OK;
}

int get_variables(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
		sprintf(interp->result,"%s %f %f %s %f %f %s %f %f\n",
                        Mget_var_name(main_mul,0),
                        Mget_var_min(main_mul,0),
                        Mget_var_max(main_mul,0),
                        Mget_var_name(main_mul,1),
                        Mget_var_min(main_mul,1),
                        Mget_var_max(main_mul,1),
                        Mget_var_name(main_mul,2),
                        Mget_var_min(main_mul,1),
                        Mget_var_max(main_mul,2));

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
	FILE	*fp;
#ifndef COMMAND_LINE
	Tk_Window wind;
#endif

	/* First check permision */

#ifndef GDB
	freopen("asurf.error","a",stderr);
#endif

#ifndef CGIVRML
	tcl_dir = getenv("LSMP_TCL");
	if(tcl_dir == NULL)
	{
		fprintf(stderr,"Could not find LSMP_TCL environment variable\n");
		exit(-1);
	}
	strcpy(tcl_file,tcl_dir);
	strcat(tcl_file,"/../bin/livlock");
	i = system(tcl_file);
	if(WEXITSTATUS(i) != 5 ) exit(-1);
#endif


	/* read in arguments */

	tmpnam(temp_file_name);
	main_mul = grballoc(Multi);
	Minit(main_mul);

#ifdef CGIVRML
	read_cgi();
#else
	psurf3_args(argc,argv);
#endif

	init_funs();

	/* If quite run then exit */

	if( quiet )
	{
                fp = fopen(arg_filename,"r");
                if(fp == NULL)
                {
                        fprintf(stderr,"Could not open %s\n",arg_filename);
                        exit(-1);
                }
                read_def(fp);
                fclose(fp);
#ifndef CGIVRML
		use_arg_vals();
#endif		
#ifndef COMMAND_LINE
		set_geom_name(arg_filename);
#endif
		asurf(arg_filename);
		if( temp_flag ) unlink(temp_file_name);
		exit(0);
	}

	/* If we don't foreground then the process forks and dies
	   as soon as we do graphics. This is bad.
	 */

#ifndef COMMAND_LINE

	if( arg_filename != NULL)
	{
                fp = fopen(arg_filename,"r");
                if(fp == NULL)
                {
                        fprintf(stderr,"Could not open %s\n",arg_filename);
                        exit(-1);
                }
                read_file(fp);
                fclose(fp);

#ifndef COMMAND_LINE
		set_geom_name(arg_filename);
#endif
	}
	else
	{
#ifndef COMMAND_LINE
		set_geom_name(prog_name);
#endif
	}

	interp = Tcl_CreateInterp();
/*
	wind = Tk_CreateMainWindow(interp,NULL,prog_name, "impsurf");
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

	tcl_dir = getenv("LSMP_TCL");
	strcpy(tcl_file,tcl_dir);
	strcat(tcl_file,"/impsurf.tcl");

	code = Tcl_EvalFile(interp,tcl_file);
	if (code != TCL_OK) {
		fprintf(stderr,"%s\n",interp->result);
		exit(1);
	}

	Tk_MainLoop();

	if( temp_flag ) unlink(temp_file_name);
#endif
}

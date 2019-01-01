/*
 *      file:   main.c:   
 *      author: Rich Morris
 *      date:   Jan 5 1995
 *      
 *	main file for asurf program.
 * hacked about to work with cgi post queries and produce
 * VRML data
 */

/*
#define GCC_COMP
#include <color.h>
#include "normlist.h"
#include "geomsimp.h"
#define TIMEOUT_LIMIT 1000000
#define GDB
*/

#define CGIVRML
#define COMMAND_LINE

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
#include "../lsmp.h"

#ifdef USE_KNOWN_SINGS
#include "../jvx/jvx.h"
#include "../jvx/jvxCore.h"
#endif

#ifndef COMMAND_LINE
/*
#include <geom.h>
*/

typedef void Geom;
#endif
#ifndef HUGE
#define HUGE 1e32
#endif
/*
#define DISCRIM_CODE
*/

#ifndef MAXORDER
#  define MAXORDER 25
#endif

#define MAX_NUM_PARAMS 50
#define MAX_NUM_VARS 3
#define ABORT 1
#define FLUSH 2

#define ASURF 1
#define DISCRIM3 2

#define grballoc(node) (node *) malloc( sizeof(node) )

extern int marmain(double aa[MAXORDER][MAXORDER][MAXORDER],
		int xord,int yord,int zord,double xmin,double xmax,
		double ymin,double ymax,double zmin,double zmax,
		HPoint3 *pl,int num_know_sings);

extern void initoogl();
extern void finioogl();


/* Three parameters which effect the execution */

extern unsigned int RESOLUTION,LINK_FACE_LEVEL,LINK_SING_LEVEL,SUPER_FINE;


char	prog_name[10];	/* Name prog was started with */
int	prog_type = ASURF; /* do discrim or asurf? */
int	n_vars = 3;	/* The number of variables */
int	n_eqns = 0;	/* The number of equations */
int	n_params = 0;	/* The number of parameters */

int	timeout = 1000;	/* The number of milliseconds which can pass
				before the program times out */
int	vrml_version = 1; /* the version of VRML produced */

#ifdef USE_KNOWN_SINGS
xml_tree *jvx;
#endif

eqnode *main_eqn = NULL;
#ifdef DISCRIM_CODE
eqnode *map_eqn = NULL;	/* The main eqn */
#endif

double rpe_vals[MAX_NUM_PARAMS + MAX_NUM_VARS];	/* values of varibles */
char   *rpe_names[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* names of variables */
int	*main_rpe;		/* The rpe string for main */
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
float	clipmax = 10.0;			/* Clip values greater than this */
int	colour = -1;			/* Default colour */
#ifdef DISCRIM_CODE
int	map_vectors = FALSE;		/* mapping eqs defined with vectors */
#endif
int     edit_time = 0;                  /* time of last editing */
int	draw_lines = 0;			/* whether to draw degenerate lines */
char	*auxGeomDef;		/* The def for the auxillary geom */

int global_mode = MODE_ASURF;		/* mode of program */
int global_selx = -1;			/* if >=0 then olny select boxes with this coord */
int global_sely = -1;
int global_selz = -1;
int global_denom = -1;
int global_degen = -1;
int global_do_refine = 1;	/* whether to refine facets */
int global_facet_count = 1000;

int global_lf = 0;			/* if set draw little facets */
char *global_geomname="asurf";

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

/* default values */

double  xmin = -1.14, xmax = 1.03, /* Size of bounding box */
        ymin = -1.13, ymax = 1.04,
        zmin = -1.12, zmax = 1.05;

void reset_asurf_sigs();
void set_asurf_sigs();

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

unsigned int arg_RESOLUTION=0,arg_LINK_FACE_LEVEL=0,
             arg_LINK_SING_LEVEL=0,arg_SUPER_FINE=0;

void print_usage(char *name)
{
	fprintf(stderr,"Usage: %s [-v xl xh yl yh zl zh] [-c coarse] [-f fine] [-F faces] [-E edges]\n",name);
	fprintf(stderr,"\t\t[-p precision] [-h] [-D name val] {-G|-I|-e equation|filename}\n");
}


void psurf3_args(argc,argv)
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

	if( !strcmp(prog_name,"asurf") )
	{
		n_eqns = 1;  n_vars = 3;
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
#endif
}

/*
	This fun is called after file has been read to arguments
	are applied after file values
*/

void use_arg_vals()
{
  int i,j;


  if(arg_RESOLUTION != 0 ) RESOLUTION = arg_RESOLUTION;
  if(arg_LINK_FACE_LEVEL != 0 ) LINK_FACE_LEVEL = arg_LINK_FACE_LEVEL;
  if(arg_SUPER_FINE != 0 ) SUPER_FINE = arg_SUPER_FINE;
  if(arg_LINK_SING_LEVEL != 0 ) LINK_SING_LEVEL = arg_LINK_SING_LEVEL;
  if(arg_precision != -1 ) precision = arg_precision;
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

/******** read_cgi *****************/
/******** creates a tempory file arg_filename with all
	the stuff from the cgi request *****/

void read_cgi()
{
	char *env_query;
	int cl;
	ncsa_entry entries[MAX_ENTRIES];
	register int x,m=0;
	char *def_ent,*coarse_ent,*fine_ent,*face_ent,*edge_ent,*timeout_ent;
	char *xmin_ent,*xmax_ent,*ymin_ent,*ymax_ent,*zmin_ent,*zmax_ent;
	char *version_ent,*lines_ent;
	FILE	*temp_file;
	FILE *fp,*fl;	
		char *tmp,*tstr; time_t tim;

	def_ent=coarse_ent=fine_ent=face_ent=edge_ent=timeout_ent=NULL;
	xmin_ent=xmax_ent=ymin_ent=ymax_ent=zmin_ent=zmax_ent=NULL;
	version_ent=lines_ent=NULL;
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
        else if(!strcmp(entries[x].name,"TIMEOUT")) timeout_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"VERSION")) version_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"DRAWLINES")) lines_ent = entries[x].val;
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
	if(strrchr(def_ent,';') == NULL ) fprintf(temp_file,";\n");
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
		fprintf(temp_file,"asurf_faces = %s;\n",face_ent);
	if(edge_ent != NULL )
		fprintf(temp_file,"asurf_edges = %s;\n",edge_ent);

	timeout = 1000;
	if(timeout_ent != NULL)
		timeout = atoi(timeout_ent);

	draw_lines = 0;
	if(lines_ent != NULL)
	{
		if( !strcmp(lines_ent,"true") ) draw_lines = 1;
	}

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

void read_lsmp_xml()
{
	LsmpInputSpec *spec;
	LsmpDef *def;
        LsmpOption *version_ent;
	LsmpOption *coarse_ent,*fine_ent,*face_ent,*edge_ent,*timeout_ent,*lines_ent;
        FILE    *temp_file,*fl;
	int cl,i;
	char *env_query;
#ifdef GDB
	FILE *fp;
#endif

	fl = fopen("asurf.log","w");

        env_query = getenv("CONTENT_LENGTH");
        if(env_query == NULL || env_query[0] == '\0' )
        {
		cl = -1;
        }
	else cl = atoi(env_query);
	
#ifdef GDB
        fp = fopen("xmltest","r");
	spec = readInputSpec(fp,&cl,fl);
#else
	spec = readInputSpec(stdin,&cl,fl);
#endif
fprintf(stderr,"Parse OK\n");

        if(spec == NULL )
        {
                report_error(HEAD_ERROR,"A specification must be specified",12);
                exit(1);
        }
        if(spec->Def == NULL)
        {
                report_error(HEAD_ERROR,"A definition must be specified",12);
                exit(1);
        }
	def = spec->Def;

	global_geomname = def->name;

       arg_filename = tmpnam(temp_file_name);
       temp_file = fopen(arg_filename,"w");

        fprintf(temp_file,"%s\n",spec->Def->data);
	fprintf(stderr,"[%s]\n",spec->Def->data);
	if(strrchr(spec->Def->data,';') == NULL ) fprintf(temp_file,";\n");

	coarse_ent = getLsmpOption(spec->Def,"coarse");
        if(coarse_ent != NULL )
                fprintf(temp_file,"asurf_coarse = %s;\n",coarse_ent->value);
	fine_ent = getLsmpOption(spec->Def,"fine");
        if(fine_ent != NULL )
                fprintf(temp_file,"asurf_fine = %s;\n",fine_ent->value);
	face_ent = getLsmpOption(spec->Def,"face");
        if(face_ent != NULL )
                fprintf(temp_file,"asurf_faces = %s;\n",face_ent->value);
	edge_ent = getLsmpOption(spec->Def,"edge");
        if(edge_ent != NULL )
                fprintf(temp_file,"asurf_edges = %s;\n",edge_ent->value);

	timeout = 1000;
	timeout_ent = getLsmpOption(spec->Def,"timeout");
	if(timeout_ent != NULL)
		timeout = atoi(timeout_ent->value);

/*
	if( (timeout_ent = getLsmpOption(spec->Def,"selx") ) != NULL)
        {  global_selx = timeout_ent->i_value; }
	if( (timeout_ent = getLsmpOption(spec->Def,"sely") ) != NULL)
        {  global_sely = timeout_ent->i_value; }
	if( (timeout_ent = getLsmpOption(spec->Def,"selz") ) != NULL)
        {  global_selz = timeout_ent->i_value; }
	if( (timeout_ent = getLsmpOption(spec->Def,"little_facets") ) != NULL)
        {  global_lf = timeout_ent->i_value; }
*/

	draw_lines = 0;
	lines_ent = getLsmpOption(spec->Def,"drawlines");
	if(lines_ent != NULL)
	{
		if( !strcmp(lines_ent->value,"true") ) draw_lines = 1;
	}

	for(i=0;i<spec->Def->n_variables;++i)
	{
		LsmpVariable *var;
		var = spec->Def->variables[i];
		if(var!=NULL)
			fprintf(temp_file,"%s = [%f,%f];\n",var->name,var->min,var->max);
	}

	if(spec->Def->opType != NULL)
	{
		if(!strcmp(spec->Def->opType,"None"))
		{
		}
		else if(!strcmp(spec->Def->opType,"surfvis"))
		{
			global_mode = MODE_KNOWN_SING;
		}
		else if(!strcmp(spec->Def->opType,"knownsings"))
		{
			global_mode = MODE_KNOWN_SING;
		}
		else
		{
			report_error2(HEAD_ERROR,"Bad type for operator %s",spec->Def->opType,13);
			exit(1);
		}
	}

        vrml_version = 3;
	version_ent = getLsmpOption(spec->Def,"vrml version");
        if(version_ent != NULL)
                vrml_version = atoi(version_ent->value);
/*
	fprintf(stderr,"vrml_version (%s) %d\n",version_ent->value,vrml_version);
*/
	if(spec->auxDef != NULL)
	{
		char *c1,*c2;
		c1 = strstr(spec->auxDef->data,"<p>");	
		c2 = strstr(spec->auxDef->data,"</p>");
		if(c1!=NULL) spec->auxDef->data = c1+3;
		if(c2!=NULL) *c2 = '\0';
		auxGeomDef = strdup(spec->auxDef->data);
	}
	else
		auxGeomDef = NULL;

#ifdef TIMEOUT_LIMIT
        if(timeout <=0 || timeout > TIMEOUT_LIMIT )
        {
                report_error2(HEAD_ERROR,"Sorry your value for the timeout (%s) exceeds the allowable maximum.",timeout_ent->value,13);
                exit(1);
        }
#endif

       fclose(temp_file);
       temp_flag = TRUE;

#ifdef USE_KNOWN_SINGS
#ifdef GDB
	if(global_mode == MODE_KNOWN_SING)
		jvx = fparse_jvx(fp,cl,fl);
#else
	if(global_mode == MODE_KNOWN_SING)
		jvx = fparse_jvx(stdin,cl,fl);
#endif
#endif

#ifdef GDB
	fclose(fp);
#endif
	fclose(fl);
}

/************ general initilisation ************************/

/*
 * Function:	init_funs
 * Action;	perform initilisation on the equation front
 */

void init_funs()
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
 * Function:	asurf
 * Action;	calculates the surface
 */

void asurf()
{
        double   a[MAXORDER][MAXORDER][MAXORDER];       /* polynomial rep */
        eqnode  *sub_eqn,*temp_eqn;
        int     i = 0,flag;
        int xord,yord,zord;
	HPoint3 *pl;
	int num_known_sings,known_sing_dim;

        if(main_eqn == NULL)
	{
		report_error(HEAD_ERROR,"syntax error in equation.",14);
	}

        /**** First some comments ****/

#ifndef COMMAND_LINE
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
/*
        printf("after eval_funs: "); print_eqn(temp_eqn); printf(";\n");
*/
	expand(temp_eqn);
/*        printf("after expand: "); print_eqn(temp_eqn); printf(";\n"); */
        eval_ops(temp_eqn);
/*
        printf("after eval_ops: "); print_eqn(temp_eqn); printf(";\n");
*/
        init_poly3(a);
        if( !add_to_poly3( temp_eqn, a,rpe_names[0],rpe_names[1],rpe_names[2]) )
	{
		report_error(HEAD_ERROR,"Could not convert equation to polynomial form. Only polynomial equations are allowed.",15);

                fprintf(stderr,"Could not convert equation to polynomial form!\n");
                fprintf(stderr,"Equation is:\n");
                fprint_eqn(stderr,temp_eqn);
                fprintf(stderr,";\n");
        }
        else
        {
                order_poly3(a,&xord,&yord,&zord);
#ifdef USE_KNOWN_SINGS
		if(global_mode == MODE_KNOWN_SING)
			pl = get_jvx_points(jvx,&num_known_sings,&known_sing_dim);
#endif

                /**** Now the data ****/
		initoogl();
                set_asurf_sigs();
                flag = marmain(a,xord,yord,zord,xmin,xmax,ymin,ymax,zmin,zmax,pl,num_known_sings);
		fprintf(stderr,"marmain finished flag %d\n",flag);
                reset_asurf_sigs();

		if(vrml_version == 0) /* OFF */
		{
	printf("LIST\n");
	printf("COMMENT asurf LSMP_DEF {\n");
        printf("# Algebraic surface defined by\n");
        printf(" "); print_eqn(main_eqn); printf(";\n");
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
        printf(" "); print_eqn(main_eqn); printf(";\n");
        for(i=0;i < n_params;++i)
                printf(" %s = %f;\n",rpe_names[i+n_vars],rpe_vals[i+n_vars]);
	printf(" %s = [%f,%f];\n%s = [%f,%f];\n%s = [%f,%f];\n",
		rpe_names[0],xmin,xmax,rpe_names[1],ymin,ymax,
		rpe_names[2],zmin,zmax);
        printf(" _coarse = %d;\n",RESOLUTION);
        printf(" _fine = %d;\n",LINK_SING_LEVEL);
        printf(" _faces = %d;\n",LINK_FACE_LEVEL);
        printf(" _edges = %d;\n",SUPER_FINE);
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
        printf(" "); print_eqn(main_eqn); printf(";\n");
        for(i=0;i < n_params;++i)
                printf(" %s = %f;\n",rpe_names[i+n_vars],rpe_vals[i+n_vars]);
	printf(" %s = [%f,%f];\n%s = [%f,%f];\n%s = [%f,%f];\n",
		rpe_names[0],xmin,xmax,rpe_names[1],ymin,ymax,
		rpe_names[2],zmin,zmax);
        printf(" _coarse = %d;\n",RESOLUTION);
        printf(" _fine = %d;\n",LINK_SING_LEVEL);
        printf(" _faces = %d;\n",LINK_FACE_LEVEL);
        printf(" _edges = %d;\n",SUPER_FINE);
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
	printf(" %s = [%f,%f];\n%s = [%f,%f];\n%s = [%f,%f];\n",
		rpe_names[0],xmin,xmax,rpe_names[1],ymin,ymax,
		rpe_names[2],zmin,zmax);
        printf(" _coarse = %d;\n",RESOLUTION);
        printf(" _fine = %d;\n",LINK_SING_LEVEL);
        printf(" _faces = %d;\n",LINK_FACE_LEVEL);
        printf(" _edges = %d;\n",SUPER_FINE);
	print_jvx_header2("algebraic curve");
	printf(" <geometries>\n");
		}

                finioogl();
		fprintf(stderr,"finioogl compleated\n");

		if(vrml_version == 3)
		{
	printf(" </geometries>\n");
	printf("</jvx-model>\n");
		}
        }

#ifndef COMMAND_LINE
        fini_geom();
#endif
        free_eqn_tree( temp_eqn );
}

/********* Now functions to perform the mapping ****************/

#ifdef DISCRIM_CODE
/*
 * Transform an individual point
 */

void PointMap(pl,nl,cl)
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

void LimitMap(pt,norm)
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

eqnode *read_sequ(int n,int *vectors,FILE *fp)
{
	eqnode *eqn[4],*res;
	int i,j;

        eqn[0] = fscan_eqn(fp);
        if( eqn[0] == NULL )
        {
                eprintf("Bad equation:");
                eprint_eqn(eqn[0]);
                eprintf("\nEquation of form X = ....; or (a,b,c); required.\n");
                return(NULL);
        }
        if( eqn[0]->op == '=' && eqn[0]->u.n.l->op == NAME) /* Three equtions */
        {
		for(i=1;i<n;++i)
		{
			eqn[i] = fscan_eqn(fp);
			if( eqn[i] == NULL )
			{
                		eprintf("Bad equation:");
                		eprint_eqn(eqn[i]);
                        eprintf("\nEquation of form Y = ....; required.\n");
				for(j=0;j<i;++j)
					free_eqn_tree(eqn[j]);
				return(NULL);
			}
			if( eqn[i]->op != '=' || eqn[i]->u.n.l->op != NAME )
			{
                        eprintf("\nEquation of form Y = ....; required.\n");
				for(j=0;j<i;++j)
					free_eqn_tree(eqn[j]);
				return(NULL);
			}
		}
		
		if( n == 4  )
		{
	                res = join_eqns(',',eqn[0]->u.n.r,
	                          join_eqns(',',eqn[1]->u.n.r,
	                          join_eqns(',',eqn[2]->u.n.r,eqn[3]->u.n.r)));
			free_eqn_node(eqn[0]->u.n.l); free_eqn_node(eqn[0]);
			free_eqn_node(eqn[1]->u.n.l); free_eqn_node(eqn[1]);
			free_eqn_node(eqn[2]->u.n.l); free_eqn_node(eqn[2]);
			free_eqn_node(eqn[3]->u.n.l); free_eqn_node(eqn[3]);
		}
		else	/* Just three coords */
		{
	                res = join_eqns(',',eqn[0]->u.n.r,
	                          join_eqns(',',eqn[1]->u.n.r,eqn[2]->u.n.r));
			free_eqn_node(eqn[0]->u.n.l); free_eqn_node(eqn[0]);
			free_eqn_node(eqn[1]->u.n.l); free_eqn_node(eqn[1]);
			free_eqn_node(eqn[2]->u.n.l); free_eqn_node(eqn[2]);
		}
		*vectors = FALSE;
	}
        else	/* defined as a vector */
        {
#ifdef NOT_DEF
		if( count_eqn_args(eqn[0]) != n )
		{
                eprintf("Bad equation:");
                eprint_eqn(eqn[0]);
                eprintf("\nEquation of form X = ....; or (a,b,c); required.\n");
		free_eqn_tree(eqn[0]);
                return(NULL);
		}
#endif
		res = eqn[0];
		*vectors = TRUE;
        }
	return(res);
}
/*
 * Function:	read_def
 * Action:	read in a new equation from 'filename',
 *		create a new geometry, set the default values etc..
 */

void read_def(filename)
char *filename;
{
	char	*tempname;
	int	i=0,j;
	eqnode	*temp;
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
#ifdef DISCRIM_CODE
	if( map_eqn != NULL ) free_eqn_tree(map_eqn);
#endif
	clear_rpe_const();

	/* input the equations */

	if( prog_type == ASURF )
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
	else
	{
#ifdef DISCRIM_CODE
		map_eqn = read_sequ(3,&map_vectors,fp);
		if( map_eqn == NULL ) goto fini_read;
#endif
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
                        if(!strcmp(temp->u.n.l->u.str,"asurf_coarse"))
                                RESOLUTION = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_fine"))
                                LINK_SING_LEVEL = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_faces"))
                                LINK_FACE_LEVEL = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_edges"))
                                SUPER_FINE = (int) temp->u.n.r->u.num;
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_precision"))
                        {
                                precision = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_selx"))
                        {
                                global_selx = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_sely"))
                        {
                                global_sely = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_selz"))
                        {
                                global_selz = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_denom"))
                        {
                                global_denom = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_degen"))
                        {
                                global_degen = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_lf"))
                        {
                                global_lf = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_refine"))
                        {
                                global_do_refine = (int) temp->u.n.r->u.num;
                        }
                        else if(!strcmp(temp->u.n.l->u.str,"asurf_facet_count"))
                        {
                                global_facet_count = (int) temp->u.n.r->u.num;
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
				if( prog_type == ASURF )
                                	substitute(main_eqn,temp);
#ifdef DISCRIM_CODE
				else
                                	substitute(map_eqn,temp);
#endif
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

#ifdef DISCRIM_CODE
	if( prog_type == ASURF )
		allnames = add_eqn_names(NULL,main_eqn);
	else
		allnames = add_eqn_names(NULL,map_eqn);
#else
	allnames = add_eqn_names(NULL,main_eqn);
#endif

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

#ifdef DISCRIM_CODE
	if( prog_type == DISCRIM3 )
	{
		eqnode *temp2,*temp3;

		eval_ops(map_eqn);
 	        temp = duplicate(map_eqn);
        	eval_funs(temp);
        	main_rpe = make_vrpe( temp, n_params + n_vars, rpe_names );
		temp2 = duplicate(temp);
		temp3 = duplicate(temp2);
		diff_wrt(temp,rpe_names[0]);
		dx_rpe = make_vrpe( temp, n_params + n_vars, rpe_names );
		diff_wrt(temp2,rpe_names[1]);
		dy_rpe = make_vrpe( temp2, n_params + n_vars, rpe_names );
		diff_wrt(temp3,rpe_names[2]);
		dz_rpe = make_vrpe( temp3, n_params + n_vars, rpe_names );

		/* To find the critical set we require 
			dx . ( dy ^ dz ) = 0
		*/

		main_eqn = join_eqns(
				'.',temp1,
				join_dup_eqns('^',temp2,temp3));
	}
#endif

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

void read_file(char *filename)
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

void copy_def(in,out)
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
                        if( !strcmp(name,"asurf_coarse")) dont_copy = TRUE;
                        if( !strcmp(name,"asurf_fine")) dont_copy = TRUE;
                        if( !strcmp(name,"asurf_faces")) dont_copy = TRUE;
                        if( !strcmp(name,"asurf_edges")) dont_copy = TRUE;
                        if( !strcmp(name,"asurf_precision")) dont_copy = TRUE;
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
#ifndef CGIVRML
        pid_t pid;

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
        {
                signal(sig,SIG_IGN);
                finioogl();
#ifndef COMMAND_LINE
                fini_geom();
#endif
                exit(-1);
        }
#endif
}

/* called before the main calculation */

void set_asurf_sigs()
{
#ifndef CGIVRML
        signal(SIGHUP,asurf_sig);
        signal(SIGINT,asurf_sig);
        signal(SIGTERM,asurf_sig);
        signal(SIGTSTP,asurf_sig);
#endif
}

/* called after the main calculation */

void reset_asurf_sigs()
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
#ifndef CGIVRML
        long dev;
        short val;
	char	tcl_command[128];
#endif
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
		fprintf(stderr,"timeout %ld max time %d clock %ld %ld\n",ellapsed,timeout,clock(),initial_clock);
		report_error(HEAD_WARNING,"Process took longer than maximium allowable time.",15);
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

	fprintf(fp," %s = [%f,%f];\n",rpe_names[0],xmin,xmax);
	fprintf(fp," %s = [%f,%f];\n",rpe_names[1],ymin,ymax);
	fprintf(fp," %s = [%f,%f];\n",rpe_names[2],zmin,zmax);

	for(i=0;i<n_params;++i)
		fprintf(fp," %s = %f;\n",
			rpe_names[i+n_vars],rpe_vals[i+n_vars]);
        fprintf(fp," asurf_coarse = %d;\n",RESOLUTION);
        fprintf(fp," asurf_fine = %d;\n",LINK_SING_LEVEL);
        fprintf(fp," asurf_faces = %d;\n",LINK_FACE_LEVEL);
        fprintf(fp," asurf_edges = %d;\n",SUPER_FINE);
        fprintf(fp," asurf_precision = %d;\n",precision);
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
	asurf();
	return TCL_OK;
}

int get_progname(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        sprintf(interp->result,"%s %d %d %d",prog_name,n_vars,n_eqns);
        return TCL_OK;
}

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        sprintf(interp->result,"%d %d %d %d %d\n",
                precision,RESOLUTION,LINK_SING_LEVEL,LINK_FACE_LEVEL,
			SUPER_FINE);
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
        precision = atoi(argv[1]);
	RESOLUTION = atoi(argv[2]);
	LINK_SING_LEVEL = atoi(argv[3]);
	LINK_FACE_LEVEL = atoi(argv[4]);
	SUPER_FINE = atoi(argv[5]);

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
#endif

/******* The main routine *********************************************/

int main(int argc,char **argv)
{
#ifndef COMMAND_LINE
	char	string[5],tcl_file[128],*tcl_dir;
	long	dev;
	short	val;
	int	i;
        int code;
        Tk_Window wind;
#endif

#ifndef GDB
	freopen("asurf.error","w",stderr);
#endif

	/* First check permision */

/*
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
*/

	/* read in arguments */
#ifndef CGIVRML
	psurf3_args(argc,argv);
#endif

	/* read in the cgi info */

/* need new parser for OL version
	read_cgi();
*/
	read_lsmp_xml();

	init_funs();

	/* If quite run then exit */

	if( quiet )
	{
                read_file(arg_filename);
#ifndef CGIVRML
		use_arg_vals();
#endif
		asurf();
		if( temp_flag ) unlink(temp_file_name);
fprintf(stderr,"OK lets exit\n");
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
        wind = Tk_CreateMainWindow(interp,NULL,prog_name, "asurf");
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
        strcat(tcl_file,"/asurf.tcl");

	code = Tcl_EvalFile(interp,tcl_file);
        if (code != TCL_OK) {
		fprintf(stderr,"%s\n",interp->result);
                exit(1);
        }

        Tk_MainLoop();

	if( temp_flag ) unlink(temp_file_name);
#endif
	return 0;
}

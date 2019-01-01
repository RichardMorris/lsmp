/*
 *      file:   icurve.c:   
 *      author: Rich Morris
 *      date:   Jan 6th 1995
 *      
 *      calculates intergral curves and wavefronts,
 *      taking a geomview object as the starting points.
 
 */

/*
#define GDB
*/
#define CGIVRML
#define NO_GEOM
#define COMMAND_LINE

#ifndef NO_GEOM
#include <geom.h>
#include "normlist.h"
#include "geomsimp.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef CGIVRML
#include <sys/wait.h>
#include <tcl.h>
#include <tk.h>
#include <color.h>
#include "../extractcomment.h"
#endif
#include <time.h>

#include <string.h>
#include <eqn.h>
#include <Multi.h>
#include <MTrans.h>
#include "../lsmp.h"
#include "../CVcommon.h"
#include "../jvx/jvx.h"
#include "../jvx/jvxCore.h"


/* preprocess switches
#define PRINT_ITT
#define SECOND_DERIV
*/
#define READ_ERRORS

#define LSMP_HEADER
#ifndef LSMP_HEADER
/* Colours for curve/surface avoid clash with 0-7 for black-white */
#define NO_COL -1
#define EQN_COL -2
#define STD_COL 0

#define FOUR_D_EQN 4
#define THREE_D_EQN 3

/* Names for options */

#define DIM_NAME "dimension"
#define COL_NAME "colour"
#define PREC_NAME "precision"
#define CLIP_NAME "clipping"
#define NUM_STEPS_NAME "num_steps"
#define STEP_LEN_NAME "step_len"
#define ORIENT_NAME "oriented"

/* Default values */

#define CLIP_DEFAULT 1000.0
#define PREC_DEFAULT 1
#endif

#define STEP_LEN_DEFAULT 0.1
#define NUM_STEPS_DEFAULT 10
#define ORIENT_DEFAULT 1

#define grballoc(node) (node *) malloc( sizeof(node) )
#define CoSet(col,R,G,B) { col.r = R; col.g = G; col.b = B; col.a = 1.0; }

/* What function the program performs, 4 = 4D coords, N = transform normals,
	C = alter colours  */

enum {Icurve} prog_type;
char	prog_name[10];
Multi	*main_mul;	/* Main mutiple eqn block */
eqn_funs *funlist = NULL;	/* list of functions */
Multi	*psurf_mul;
MTrans	*psurf_trans;

#define TGT_NM_SZ 1024
char     tgt_names[TGT_NM_SZ];   /* array to hold list of target names */

#ifndef NO_GEOM
Geom    *wave_geom;		     /* Geometry containing wave front */
int     self_prim = POINTLIST_SELF;     /* Transform self or primatives */
#endif
HPoint3 *wave_pl;		       /* Points in front */
int     wave_num_pts;		   /* # Point in front */

/* default values */

int	world = FALSE;			/* Transform relative to world */
int	command = FALSE;		/* Write geomview comands */
int	quiet = TRUE;			/* quite mode */
double  itt_vec[4];		     /* The last vector used */
int     do_vect_field = FALSE;	  /* Draw vector field */
int     do_wavefront = FALSE;	   /* Draw wave front */
int	global_dim,global_oriented;	/* Only used when evaluating */
float	clipmax;			/* Only used when eval */
int     global_mode = 0;	/* 0 = simple 1 = operator on a psurf */
int     psurf_edit_time = 0;    /* time psurf was last edited */
int     edit_time = 0;	  /* time this equation was last edited */
char    psurf_tgt_name[64];     /* The target name for the psurf ingr */
xml_tree *jvx;
int     vrml_version = 1; /* the version of VRML produced */
char	*auxGeomDef;		/* The def for the auxillary geom */

#define  CLIP(X) (X > clipmax ? clipmax :( X < -clipmax ? -clipmax : ( X != X ? clipmax : X) ))
#define  COL_CLIP(X) (X > 1.0 ? 1.0 :( X < 0 ? 0 : ( X != X ? 1 : X) ))

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

#ifndef NO_GEOM
extern Geom *load_geom(); 
#endif

#ifdef X
#undef X
#undef Y
#undef Z
#undef W
#endif 

void copy_def(FILE *in,FILE *out);
int read_psurf_def(FILE *fp,Multi *psurf_mul);
int read_file(FILE *fp);

/*********** Argument handeling ***********************/

float arg_clipmax = 0.0;
int   arg_precision = -1,arg_orient = -1;
int   arg_num_itt = -1, arg_colour = -5;
double  arg_increment = 0.0;
int	arg_dimension = 0;

double arg_vals[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* vals from arguments */
char   *arg_names[MAX_NUM_PARAMS + MAX_NUM_VARS];
int     arg_count=0;	/* number of params in arguments */
char	*arg_filename = NULL;	/* filename from arguments */
int	temp_flag = FALSE;	/* TRUE if equation def on command line */
char	temp_file_name[L_tmpnam];	/* temp file for equation */

void print_usage(char *name)
{
	fprintf(stderr,"Usage: %s [-V|-W] [-o] ",name);
	fprintf(stderr,"[-i increment] [-n num_steps] [-C col]\n");
	fprintf(stderr,"\t\t[-c clip] [-p precision]\n");
	fprintf(stderr,"\t\t[-h] [-D name val] {-G|-I|-e equation|filename}\n");
}


void icurve_args(argc,argv)
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
	else	      strcpy(prog_name,slash+1);

	if( !strcmp(prog_name,"icurve") )
	{	prog_type = Icurve;
	}
	else
	{
		fprintf(stderr,"bad program name: %s\n",prog_name);
		exit(-1);
	}

	/* Now we can look at the arguments */

	while((i=getopt(argc,argv,"hVoeIGi:n:c:p:C:D:")) != -1 )
	{
	       switch(i)
	       {
			case 'c': arg_clipmax=atof(optarg); break;
			case 'p': arg_precision = atoi(optarg); break;
			case 'o': arg_orient = FALSE; break;
			case 'V': do_vect_field = TRUE; break;
			case 'W': do_wavefront = TRUE; break;
			case 'i': arg_increment = (double) atof(optarg); break;
			case 'n': arg_num_itt =  atoi(optarg); break;
			case 'C': arg_colour =  atoi(optarg); break;
 
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

void read_lsmp_xml()
{
	LsmpInputSpec *spec;
	LsmpDef *def;
	LsmpOption *colors_ent,*dimension_ent,*num_steps_ent,*oriented_ent,
		*step_len_ent,*clip_ent,*inttype_ent,*version_ent;
        FILE    *temp_file,*fl;
#ifdef GDB
	FILE	*fp;
#endif
	int cl;
	char *env_query;

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

       arg_filename = tmpnam(temp_file_name);
       temp_file = fopen(arg_filename,"w");

        fprintf(temp_file,"%s\n",spec->Def->data);

	inttype_ent = getLsmpOption(spec->Def,"icurvetype");
	if(inttype_ent != NULL)
	{
		if(!strcmp(inttype_ent->value,"VecField"))
		{	do_vect_field = TRUE; do_wavefront = FALSE; }
		else if(!strcmp(inttype_ent->value,"WaveFront"))
		{	do_vect_field = FALSE; do_wavefront = TRUE; }
	        else if(!strcmp(inttype_ent->value,"IntegralCurve"))
		{	do_vect_field = FALSE; do_wavefront = FALSE; }
		else
		{
			report_error2(HEAD_ERROR,"Bad type of integral curve (%s)",inttype_ent->value,101);
			exit(1);
		}
	}
	else
		{	do_vect_field = TRUE; do_wavefront = FALSE; }

	colors_ent = getLsmpOption(spec->Def,"color");
	if(colors_ent != NULL )
	{
		if(!strcmp(colors_ent->value,NO_COL_NAME))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,NO_COL);
		else if(!strcmp(colors_ent->value,"Black"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,0);
		else if(!strcmp(colors_ent->value,"Red"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,1);
		else if(!strcmp(colors_ent->value,"Green"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,2);
		else if(!strcmp(colors_ent->value,"Yellow"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,3);
		else if(!strcmp(colors_ent->value,"Blue"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,4);
		else if(!strcmp(colors_ent->value,"Magenta"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,5);
		else if(!strcmp(colors_ent->value,"Cyan"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,6);
		else if(!strcmp(colors_ent->value,"White"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,7);
		else
		{
			report_error2(HEAD_ERROR,"Bad type for colors %s",colors_ent->value,101);
			exit(1);
		}
	}

	dimension_ent = getLsmpOption(spec->Def,"dim");
	if(dimension_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %s;\n",DIM_NAME,dimension_ent->value);
	}
	num_steps_ent = getLsmpOption(spec->Def,"numsteps");
	if(num_steps_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %s;\n",NUM_STEPS_NAME,num_steps_ent->value);
	}
	oriented_ent = getLsmpOption(spec->Def,"orientation");
	if(oriented_ent!=NULL)
	{
		if(!strcmp(oriented_ent->value,"Unoriented"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_UN);
		else if(!strcmp(oriented_ent->value,"Oriented"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_ORIENT);
	        else if(!strcmp(oriented_ent->value,"Major Eigen Vector"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_MAJOR);
	        else if(!strcmp(oriented_ent->value,"Minor Eigen Vector"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_MINOR);
		else
		{
			report_error2(HEAD_ERROR,"Bad type of orientation %s",oriented_ent->value,101);
			exit(1);
		}
	}
	step_len_ent = getLsmpOption(spec->Def,"steplen");
	if(step_len_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %s;\n",STEP_LEN_NAME,step_len_ent->value);
	}

	if(spec->Def->opType != NULL)
	{
		if(!strcmp(spec->Def->opType,"None"))
		{
		}
		else if(!strcmp(spec->Def->opType,"psurf"))
		{
                	fprintf(temp_file,"_mode = 1;\n");
			global_mode = MODE_PSURF;
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

#ifdef GDB
	jvx = fparse_jvx(fp,cl,fl);
	fclose(fp);
#else
	jvx = fparse_jvx(stdin,cl,fl);
#endif
	fclose(fl);
}

void read_cgi()
{
        char *env_query;
        int cl;
        ncsa_entry entries[MAX_ENTRIES]; 
        register int x,m=0;
        char *def_ent;
	char *colors_ent,*dimension_ent,*num_steps_ent,*oriented_ent;
	char *step_len_ent,*clip_ent;
        char *inttype_ent,*inputgeom_ent,*auxGeom_ent,*opType_ent;
        char *version_ent;
        FILE    *temp_file;
        FILE *fp,*fl,*fc;
        char *tmp,*tstr; time_t tim;

        def_ent=inttype_ent=NULL;
	colors_ent=dimension_ent=num_steps_ent=oriented_ent=NULL;
	step_len_ent=clip_ent=NULL;
        inttype_ent=inputgeom_ent=auxGeom_ent=opType_ent=NULL;
	version_ent=NULL;

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
	fc = fopen("asurf.cgi","w");

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
        if(x>0) { fprintf(fc,"&"); fprintf(fl,"&"); }
/*
	if(strncmp(entries[x].val,"INPUTGEOM",9))
*/
	        fprintf(fc,"%s",entries[x].val);
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
/*
	if(strncmp(entries[x].val,"INPUTGEOM",9))
*/
	        fprintf(fl,"%s\n",entries[x].val);

        entries[x].name = makeword(entries[x].val,'=');

        if(!strcmp(entries[x].name,"DEF"))          def_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"COLORS"))  colors_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"DIM"))  dimension_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"NUMSTEPS"))  num_steps_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"ORIENTED"))  oriented_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"STEPLEN"))  step_len_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"CLIPPING"))  clip_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"COLORS"))  colors_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"VERSION")) version_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"INPUTGEOM")) inputgeom_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"AUXGEOM")) auxGeom_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"OPTYPE")) opType_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"ICURVETYPE")) inttype_ent = entries[x].val;
        else 
        {    
                report_error2(HEAD_ERROR,"Bad field name %s",entries[x].name,3);
                exit(1);
        }     
    }   
        fprintf(stderr,"\n");
        fprintf(fl,"\n");
        fclose(fl);
        fclose(fc);
        if(def_ent == NULL)
        {
                report_error(HEAD_ERROR,"A definition must be specified",12);
                exit(1);
        }
       arg_filename = tmpnam(temp_file_name);
       temp_file = fopen(arg_filename,"w");

        if(def_ent != NULL ) fprintf(temp_file,"%s\n",def_ent);


	if(inttype_ent != NULL)
	{
		if(!strcmp(inttype_ent,"VecField"))
		{	do_vect_field = TRUE; do_wavefront = FALSE; }
		else if(!strcmp(inttype_ent,"WaveFront"))
		{	do_vect_field = FALSE; do_wavefront = TRUE; }
	        else if(!strcmp(inttype_ent,"IntegralCurve"))
		{	do_vect_field = FALSE; do_wavefront = FALSE; }
		else
		{
			report_error2(HEAD_ERROR,"Bad type of integral curve (%s)",inttype_ent,101);
			exit(1);
		}
	}
	else
		{	do_vect_field = TRUE; do_wavefront = FALSE; }

	if(colors_ent != NULL )
	{
		if(!strcmp(colors_ent,NO_COL_NAME))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,NO_COL);
		else if(!strcmp(colors_ent,"Black"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,0);
		else if(!strcmp(colors_ent,"Red"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,1);
		else if(!strcmp(colors_ent,"Green"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,2);
		else if(!strcmp(colors_ent,"Yellow"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,3);
		else if(!strcmp(colors_ent,"Blue"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,4);
		else if(!strcmp(colors_ent,"Magenta"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,5);
		else if(!strcmp(colors_ent,"Cyan"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,6);
		else if(!strcmp(colors_ent,"White"))
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,7);
		else
		{
			report_error2(HEAD_ERROR,"Bad type for colors %s",colors_ent,101);
			exit(1);
		}
	}

	if(dimension_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %s;\n",DIM_NAME,dimension_ent);
	}
	if(num_steps_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %s;\n",NUM_STEPS_NAME,num_steps_ent);
	}
	if(oriented_ent!=NULL)
	{
		if(!strcmp(oriented_ent,"Unoriented"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_UN);
		else if(!strcmp(oriented_ent,"Oriented"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_ORIENT);
	        else if(!strcmp(oriented_ent,"Major Eigen Vector"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_MAJOR);
	        else if(!strcmp(oriented_ent,"Minor Eigen Vector"))
			fprintf(temp_file,"_%s = %d;\n",ORIENT_NAME,ORIENT_MINOR);
		else
		{
			report_error2(HEAD_ERROR,"Bad type of orientation %s",oriented_ent,101);
			exit(1);
		}
	}
	if(step_len_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %s;\n",STEP_LEN_NAME,step_len_ent);
	}

	if(auxGeom_ent != NULL)
	{
		char *c1,*c2;
		c1 = strstr(auxGeom_ent,"<p>");	
		c2 = strstr(auxGeom_ent,"</p>");
		if(c1!=NULL) auxGeom_ent = c1+3;
		if(c2!=NULL) *c2 = '\0';
		auxGeomDef = strdup(auxGeom_ent);
	}
	else
		auxGeomDef = NULL;

	if(opType_ent != NULL)
	{
		if(!strcmp(opType_ent,"None"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_SIMPLE);
			global_mode = MODE_SIMPLE;
		}
		else if(!strcmp(opType_ent,"psurf"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_PSURF);
			global_mode = MODE_PSURF;
		}
		else
		{
			report_error2(HEAD_ERROR,"Bad type for operator %s",opType_ent,13);
			exit(1);
		}
	}
	else
		global_mode = MODE_SIMPLE;

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

        if(inputgeom_ent == NULL)
        {
                report_error(HEAD_ERROR,"An XML format input geometry must be specified",106);
                exit(1);
        }

	jvx = parse_jvx(inputgeom_ent);
}

/*
	This fun is called after file has been read to arguments
	are applied after file values
*/

void use_arg_vals()
{
  int i;

  if(arg_precision != -1 ) 
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) arg_precision);
  if(arg_clipmax != 0.0 )
	Mset_opt_val_by_name(main_mul,CLIP_NAME,arg_clipmax);
  if(arg_dimension != 0 )
	Mset_opt_val_by_name(main_mul,DIM_NAME,(double) arg_dimension);
  if(arg_colour != -5)
	Mset_opt_val_by_name(main_mul,COL_NAME,(double) arg_colour);
  if(arg_orient != -1)
	Mset_opt_val_by_name(main_mul,ORIENT_NAME,(double) arg_orient);
  if(arg_num_itt != -1)
	Mset_opt_val_by_name(main_mul,NUM_STEPS_NAME,(double) arg_num_itt);
  if(arg_increment != 0.0)
	Mset_opt_val_by_name(main_mul,STEP_LEN_NAME,(double) arg_increment);

  for(i=0;i<arg_count;++i)
	Mset_param_val_by_name(main_mul,arg_names[i],arg_vals[i]);
}

/************ general initilisation ************************/

/*
 * Function:	init_funs
 * Action;	perform initilisation on the equation front
 */

void init_funs()
{
	funlist = add_standard_functions(NULL);
	set_input_functions(funlist);
	MsetNvars(main_mul,3);
	Mset_var_name(main_mul,0,"x");
	Mset_var_name(main_mul,1,"y");
	Mset_var_name(main_mul,2,"z");
	Madd_opt(main_mul,PREC_NAME,(double) PREC_DEFAULT);
	Madd_opt(main_mul,CLIP_NAME,CLIP_DEFAULT);
	Madd_opt(main_mul,DIM_NAME,3.0);
	Madd_opt(main_mul,COL_NAME,(double) NO_COL);
	Madd_opt(main_mul,ORIENT_NAME,(double) ORIENT_DEFAULT);
	Madd_opt(main_mul,STEP_LEN_NAME,STEP_LEN_DEFAULT);
	Madd_opt(main_mul,NUM_STEPS_NAME,(double) NUM_STEPS_DEFAULT);
	Madd_opt(main_mul,MODE_NAME,(double) MODE_SIMPLE);

	MsetNvars(psurf_mul,2);
	Mset_var_name(psurf_mul,0,"x");
	Mset_var_name(psurf_mul,1,"y");
/*
	Mset_var_minmax(psurf_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
	Mset_var_minmax(psurf_mul,1,XMIN_DEFAULT,XMAX_DEFAULT);
*/
	Madd_opt(psurf_mul,PREC_NAME,(double) PREC_DEFAULT);
	Madd_opt(psurf_mul,CLIP_NAME,CLIP_DEFAULT);
/*
	Madd_opt(psurf_mul,STEPS1_NAME,(double) PS_STEPS);
	Madd_opt(psurf_mul,STEPS2_NAME,(double) PS_STEPS);
*/
	Madd_opt(psurf_mul,DIM_NAME,3.0);
	Madd_opt(psurf_mul,NORM_NAME,(double) STD_NORM);
	Madd_opt(psurf_mul,COL_NAME,(double) NO_COL);
}

/************* The guts actually does the work ***********************/

/*
 * Function:	itterate
 * Action:	given pt1 calculate pt2 
 * Returns:	FALSE if itteration failed
 */

void PsurfMap(HPoint3 *pl)
{
	float val;
	double *ptr;

        Mset_var_val(psurf_mul,0,pl->x);
        Mset_var_val(psurf_mul,1,pl->y);
	MstartEval(psurf_mul);
        ptr = MevalTop(psurf_mul,0);
        val = (float) *ptr; pl->x = CLIP(val); 
        val = (float) *(ptr+1); pl->y = CLIP(val); 
        val = (float) *(ptr+2); pl->z = CLIP(val); 
	pl->w = 1.0;
}

void PsurfMapTgt(HPoint3 *pl,HPoint3 *tgt)
{
	float val,u1,u2;
	double *ptr,a,b,c,d,e,f;

        Mset_var_val(psurf_mul,0,pl->x);
        Mset_var_val(psurf_mul,1,pl->y);
	MstartEval(psurf_mul);
        ptr = MevalTop(psurf_mul,0);
        val = (float) *ptr; pl->x = CLIP(val); 
        val = (float) *(ptr+1); pl->y = CLIP(val); 
        val = (float) *(ptr+2); pl->z = CLIP(val); 
	pl->w = 1.0;
	ptr = MevalDeriv(psurf_mul,0);
        a = *ptr; b = *(ptr+1); c = *(ptr+2);
        ptr = MevalDeriv(psurf_mul,1);
        d = *ptr; e = *(ptr+1); f = *(ptr+2);

	u1 = tgt->x; u2 = tgt->y;
	tgt->x = a * u1 + d * u2;
	tgt->y = b * u1 + e * u2;
	tgt->z = c * u1 + f * u2;
	tgt->w = 1.0;
}



int itterate_psurf(HPoint3 *pt1,HPoint3 *pt2,double dt);

int itterate(pt1,pt2,dt)
HPoint3 *pt1,*pt2;
double dt;
{
	double dx,dy,dz,dw;
#ifdef SECOND_DERIV
	double d2x,d2y,d2z;   
#endif
	double dot,len;
	double *ptr;
	int	offset = 0;

	/* Set the variables */

	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
	{
		return(itterate_psurf(pt1,pt2,dt));
	}


	Mset_var_val(main_mul,offset++,pt1->x);
	Mset_var_val(main_mul,offset++,pt1->y);
	Mset_var_val(main_mul,offset++,pt1->z);
	if( global_dim == FOUR_D_EQN )
		Mset_var_val(main_mul,offset++,pt1->w);
 
	MstartEval(main_mul);
	ptr = MevalTop(main_mul,0);
	dx = *ptr; 
	dy = *(ptr+1); 
	dz = *(ptr+2); 
	if( global_dim == FOUR_D_EQN ) 
		dw = *(ptr+3); 
	else
		dw = 0.0;

	if( global_oriented == ORIENT_MAJOR
			 || global_oriented == ORIENT_MINOR )
	{
		double det,tr,disc,lam1,lam2;
		double V1a_x,V1a_y,V1b_x,V1b_y,V2a_x,V2a_y,V2b_x,V2b_y;

		dw = *(ptr+3);
		det = dx * dw - dy * dz;
		tr  = dx + dw;
		disc = tr * tr - 4 * det;
		if( disc < 0.0 ) return(FALSE);
		lam1 = (tr + sqrt(disc) )/ 2.0;
		lam2 = tr - lam1;
		V1a_x = - dy;
		V1a_y = dx - lam1;
		V1b_x = dw - lam1;
		V1b_y = - dz;
		V2a_x = - dy;
		V2a_y = dx - lam2;
		V2b_x = dw - lam2;
		V2b_y = - dz;
		if( global_oriented == ORIENT_MAJOR )
		{
		    if( fabs(V1a_x)+fabs(V1a_y) > fabs(V1b_x) + fabs(V1b_y) )
		    {
			dx = V1a_x; dy = V1a_y;
			dz = dw = 0.0;
		    } else {
			dx = V1b_x; dy = V1b_y;
			dz = dw = 0.0;
		    }
		}
		else
		{
		    if( fabs(V2a_x)+fabs(V2a_y) > fabs(V2b_x) + fabs(V2b_y) )
		    {
			dx = V2a_x; dy = V2a_y;
			dz = dw = 0.0;
		    } else {
			dx = V2b_x; dy = V2b_y;
			dz = dw = 0.0;
		    }
		}
	}

	dot = itt_vec[0] * dx + itt_vec[1] * dy 
		+ itt_vec[2] * dz + itt_vec[3] * dw;
	len = sqrt(dx*dx + dy*dy + dz*dz + dw*dw);
	
	if( len == 0.0 || dx != dx || dy != dy || dz != dz || dw != dw)
	{
#ifdef PRINT_ITT
	fprintf(stderr,"NaN's X' %f Y' %f Z' %f W' %f x %f y %f z %f w %d\n",
		dx,dy,dz,dw,pt1->x,pt1->y,pt1->z,pt1->w);
#endif
		return(FALSE);
	}

	if( global_oriented == ORIENT_ORIENT)
	{
		pt2->x = pt1->x + dx * dt;
		pt2->y = pt1->y + dy * dt;
		pt2->z = pt1->z + dz * dt;
		pt2->w = pt1->w + dw * dt;
		itt_vec[0] = dx;
		itt_vec[1] = dy;
		itt_vec[2] = dz;
		itt_vec[3] = dw;
	}
	else if( dot >= 0.0 )
	{
		pt2->x = pt1->x + dx * dt/len;
		pt2->y = pt1->y + dy * dt/len;
		pt2->z = pt1->z + dz * dt/len;
		pt2->w = pt1->w + dw * dt/len;
		itt_vec[0] = dx;
		itt_vec[1] = dy;
		itt_vec[2] = dz;
		itt_vec[3] = dw;
	}
	else
	{
		pt2->x = pt1->x - dx * dt/len;
		pt2->y = pt1->y - dy * dt/len;
		pt2->z = pt1->z - dz * dt/len;
		pt2->w = pt1->w - dw * dt/len;
		itt_vec[0] = -dx;
		itt_vec[1] = -dy;
		itt_vec[2] = -dz;
		itt_vec[3] = -dw;
	}

	if( fabs(pt2->x) > clipmax || fabs(pt2->y) > clipmax
		|| fabs(pt2->z) > clipmax || fabs(pt2->w) > clipmax )
	{
#ifdef PRINT_ITT
	fprintf(stderr,"NaN's after X' %f Y' %f Z' %f x %f y %f z %f\n",
		dx,dy,dz,pt1->x,pt1->y,pt1->z);
#endif
		return(FALSE);
	}

#ifdef SECOND_DERIV
	d2x = -eval_rpe(dX_dx_rpe,rpe_vals) * dx
	      -eval_rpe(dX_dy_rpe,rpe_vals) * dy
	      -eval_rpe(dX_dz_rpe,rpe_vals) * dz;

	d2y = -eval_rpe(dY_dx_rpe,rpe_vals) * dx
	      -eval_rpe(dY_dy_rpe,rpe_vals) * dy
	      -eval_rpe(dY_dz_rpe,rpe_vals) * dz;

	d2z = -eval_rpe(dZ_dx_rpe,rpe_vals) * dx
	      -eval_rpe(dZ_dy_rpe,rpe_vals) * dy
	      -eval_rpe(dZ_dz_rpe,rpe_vals) * dz;

	if(d2x != d2x || fabs(d2x) > clipmax )
		pt2->x = pt1->x + dx * dt;
	else
		pt2->x = pt1->x + dx * dt + d2x * dt * dt / 2.0;

	if(d2y != d2y || fabs(d2y) > clipmax )
		pt2->y = pt1->y + dy * dt;
	else
		pt2->y = pt1->y + dy * dt + d2y * dt * dt / 2.0;

	if(d2z != d2z || fabs(d2z) > clipmax )
		pt2->z = pt1->z + dz * dt;
	else
		pt2->z = pt1->z + dz * dt + d2z * dt * dt / 2.0;
	pt2->x = CLIP(pt2->x);
	pt2->y = CLIP(pt2->y);
	pt2->z = CLIP(pt2->z);
#endif

	return(TRUE);
}

int itterate_psurf(HPoint3 *pt1,HPoint3 *pt2,double dt)
{
	double dx,dy,dz,dw;
#ifdef SECOND_DERIV
	double d2x,d2y,d2z;   
#endif
	double dot,len;
	double *ptr;

	/* Set the variables */


	MstartEval(main_mul);
	MstartEval(psurf_mul);
	Mset_var_val(main_mul,3,pt1->x);
	Mset_var_val(main_mul,4,pt1->y);
	Mset_var_val(psurf_mul,0,pt1->x);
	Mset_var_val(psurf_mul,1,pt1->y);

	MTevalForTop(psurf_trans,0);
	MTcopyVars(psurf_trans);
	ptr = MevalTop2(main_mul,0);
	dx = *ptr; 
	dy = *(ptr+1); 
	dz = *(ptr+2); 
	if( global_dim == FOUR_D_EQN ) 
		dw = *(ptr+3); 
	else
		dw = 0.0;

	if( global_oriented == ORIENT_MAJOR
			 || global_oriented == ORIENT_MINOR )
	{
		double det,tr,disc,lam1,lam2;
		double V1a_x,V1a_y,V1b_x,V1b_y,V2a_x,V2a_y,V2b_x,V2b_y;

		dw = *(ptr+3);
		det = dx * dw - dy * dz;
		tr  = dx + dw;
		disc = tr * tr - 4 * det;
		if( disc < 0.0 ) return(FALSE);
		lam1 = (tr + sqrt(disc) )/ 2.0;
		lam2 = tr - lam1;
		V1a_x = - dy;
		V1a_y = dx - lam1;
		V1b_x = dw - lam1;
		V1b_y = - dz;
		V2a_x = - dy;
		V2a_y = dx - lam2;
		V2b_x = dw - lam2;
		V2b_y = - dz;
		if( global_oriented == ORIENT_MAJOR )
		{
		    if( fabs(V1a_x)+fabs(V1a_y) > fabs(V1b_x) + fabs(V1b_y) )
		    {
			dx = V1a_x; dy = V1a_y;
			dz = dw = 0.0;
		    } else {
			dx = V1b_x; dy = V1b_y;
			dz = dw = 0.0;
		    }
		}
		else
		{
		    if( fabs(V2a_x)+fabs(V2a_y) > fabs(V2b_x) + fabs(V2b_y) )
		    {
			dx = V2a_x; dy = V2a_y;
			dz = dw = 0.0;
		    } else {
			dx = V2b_x; dy = V2b_y;
			dz = dw = 0.0;
		    }
		}
	}

	dot = itt_vec[0] * dx + itt_vec[1] * dy 
		+ itt_vec[2] * dz + itt_vec[3] * dw;
	len = sqrt(dx*dx + dy*dy + dz*dz + dw*dw);
	
	if( len == 0.0 || dx != dx || dy != dy || dz != dz || dw != dw)
	{
#ifdef PRINT_ITT
	fprintf(stderr,"NaN's X' %f Y' %f Z' %f W' %f x %f y %f z %f w %d\n",
		dx,dy,dz,dw,pt1->x,pt1->y,pt1->z,pt1->w);
#endif
		return(FALSE);
	}

	if( global_oriented == ORIENT_ORIENT )
	{
		pt2->x = pt1->x + dx * dt;
		pt2->y = pt1->y + dy * dt;
		pt2->z = pt1->z + dz * dt;
		pt2->w = pt1->w + dw * dt;
		itt_vec[0] = dx;
		itt_vec[1] = dy;
		itt_vec[2] = dz;
		itt_vec[3] = dw;
	}
	else if( dot >= 0.0 )
	{
		pt2->x = pt1->x + dx * dt/len;
		pt2->y = pt1->y + dy * dt/len;
		pt2->z = pt1->z + dz * dt/len;
		pt2->w = pt1->w + dw * dt/len;
		itt_vec[0] = dx;
		itt_vec[1] = dy;
		itt_vec[2] = dz;
		itt_vec[3] = dw;
	}
	else
	{
		pt2->x = pt1->x - dx * dt/len;
		pt2->y = pt1->y - dy * dt/len;
		pt2->z = pt1->z - dz * dt/len;
		pt2->w = pt1->w - dw * dt/len;
		itt_vec[0] = -dx;
		itt_vec[1] = -dy;
		itt_vec[2] = -dz;
		itt_vec[3] = -dw;
	}

	if( fabs(pt2->x) > clipmax || fabs(pt2->y) > clipmax
		|| fabs(pt2->z) > clipmax || fabs(pt2->w) > clipmax )
	{
#ifdef PRINT_ITT
	fprintf(stderr,"NaN's after X' %f Y' %f Z' %f x %f y %f z %f\n",
		dx,dy,dz,pt1->x,pt1->y,pt1->z);
#endif
		return(FALSE);
	}

#ifdef SECOND_DERIV
	d2x = -eval_rpe(dX_dx_rpe,rpe_vals) * dx
	      -eval_rpe(dX_dy_rpe,rpe_vals) * dy
	      -eval_rpe(dX_dz_rpe,rpe_vals) * dz;

	d2y = -eval_rpe(dY_dx_rpe,rpe_vals) * dx
	      -eval_rpe(dY_dy_rpe,rpe_vals) * dy
	      -eval_rpe(dY_dz_rpe,rpe_vals) * dz;

	d2z = -eval_rpe(dZ_dx_rpe,rpe_vals) * dx
	      -eval_rpe(dZ_dy_rpe,rpe_vals) * dy
	      -eval_rpe(dZ_dz_rpe,rpe_vals) * dz;

	if(d2x != d2x || fabs(d2x) > clipmax )
		pt2->x = pt1->x + dx * dt;
	else
		pt2->x = pt1->x + dx * dt + d2x * dt * dt / 2.0;

	if(d2y != d2y || fabs(d2y) > clipmax )
		pt2->y = pt1->y + dy * dt;
	else
		pt2->y = pt1->y + dy * dt + d2y * dt * dt / 2.0;

	if(d2z != d2z || fabs(d2z) > clipmax )
		pt2->z = pt1->z + dz * dt;
	else
		pt2->z = pt1->z + dz * dt + d2z * dt * dt / 2.0;
	pt2->x = CLIP(pt2->x);
	pt2->y = CLIP(pt2->y);
	pt2->z = CLIP(pt2->z);
#endif

	return(TRUE);
}



void print_icurve_jvx_header(char *filename)
{
	FILE *fp;

	print_jvx_header("Icurve","Integral curves");
		
	fp = fopen(filename,"r");
	copy_def(fp,stdout);		
	fprint_Mopts(stdout,main_mul);
	fclose(fp);

	print_jvx_header2("Icurve");
}

void print_lines_as_jvx(HPoint3 *points,int num_points,int *vert_per_line,int num_lines,int color)
{
	int cur_index,i,j;
	xml_tree *colours;

	printf("    <geometries>\n");
	printf("      <geometry name=\"icurve\">\n");
	printf("        <pointSet dim=\"3\" point=\"hide\">\n");
	printf("          <points num=\"%d\">\n",num_points);
	for(i=0;i<num_points;++i)
	printf("            <p>%f %f %f</p>\n",points[i].x,points[i].y,points[i].z);
	printf("          </points>\n");
	printf("        </pointSet>\n");
	printf("        <lineSet line=\"show\">\n");
/*
	printf("          <lines num=\"%d\">\n",num_lines);
*/
	printf("          <lines>\n");
	cur_index = 0;
	for(i=0;i<num_lines;++i)
	{
	    if(vert_per_line[i] >= 1)
	    {
	printf("            <l>");
		for(j=0;j<vert_per_line[i];++j)
		{
			printf("%d ",cur_index);
			if(j>0 && j%10==0 && j+5 < vert_per_line[i])
				printf("</l>\n<l>%d ",cur_index);
			++cur_index;
		}
		printf("</l>\n");		
	    }
	}
	colours = create_jvx_color_from_color_number(color);	
	print_brief_jvx_tree(stdout,colours,5,0);

	printf("          </lines>\n");
	printf("        </lineSet>\n");
	printf("      </geometry>\n");
	printf("    </geometries>\n");

	print_jvx_tail();
}

/*
 * Function:	int_curve
 * Action;	read in data from file construct curves and output it
 */

void int_curve_psurf(char *filename);

void int_curve(char *filename)
{
#ifndef NO_GEOM
	Transform Ident;
	Geom	*geom,*in_geom,*vect;
	int     new_edit_time;
	int	num_lines;
	ColorA	*colours;
	float	val;
#endif
	HPoint3 *pl,*points;
	int	*vert_per_vect,*coltab;
	int	num_pts,i,j,k,n_vect,num_itt,colindex;
	float	step_len;
	FILE    *fp;
#ifdef CGIVRML
	int dim_p;
#endif

	if(main_mul->error )
	{
		fprintf(stderr,"Can't calculate intergral curves - bad equations\n");
		return;
	}

	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
	{
		int_curve_psurf(filename);
		return;
	}

	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}

#ifndef NO_GEOM
	TmIdentity(Ident);
	in_geom = load_geom();

	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't calculate curves\n");
		fclose(fp);
	}
	start_geom();
#endif

	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	global_oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	num_itt = (int) Mget_opt_val_by_name(main_mul,NUM_STEPS_NAME);
	step_len = (float) Mget_opt_val_by_name(main_mul,STEP_LEN_NAME);
	colindex = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

#ifndef NO_GEOM
	printf("LIST\n");
	printf("COMMENT icurve %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT icurve %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);


	geom = GeomSimp(in_geom);
	GeomFree(in_geom);

	pl = (HPoint3 *) PointList_get(geom,Ident,self_prim);
	num_pts = PointList_length(geom);
#endif

#ifdef CGIVRML
	pl = get_jvx_points(jvx,&num_pts,&dim_p);
#endif

	if( global_oriented == ORIENT_ORIENT )
	{
		points = OOGLNewN(HPoint3,num_pts*num_itt);
		vert_per_vect = OOGLNewN(int,num_pts);
		coltab = OOGLNewN(int,num_pts);
		n_vect = num_pts;
	}
	else
	{
		points = OOGLNewN(HPoint3,num_pts*num_itt*2);
		vert_per_vect = OOGLNewN(int,num_pts*2);
		coltab = OOGLNewN(int,num_pts*2);
		n_vect = num_pts * 2;
	}

#ifndef NO_GEOM
	colours = OOGLNewN(ColorA,1);
	switch(colindex)
	{
		case 0: /* Black   */ CoSet(colours[0],0.0,0.0,0.0); break;
		case 1: /* Red     */ CoSet(colours[0],1.0,0.0,0.0); break;
		case 2: /* Green   */ CoSet(colours[0],0.0,1.0,0.0); break;
		case 3: /* Yellow  */ CoSet(colours[0],1.0,1.0,0.0); break;
		case 4: /* Blue    */ CoSet(colours[0],0.0,0.0,1.0); break;
		case 5: /* Magenta */ CoSet(colours[0],1.0,0.0,1.0); break;
		case 6: /* Cyan    */ CoSet(colours[0],0.0,1.0,1.0); break;
		case 7: /* White   */
		default:
				      CoSet(colours[0],1.0,1.0,1.0); break;
	}
#endif


	k = 0;
	for(j=0;j<num_pts;++j)
	{
	    if(global_oriented == ORIENT_ORIENT )
	    {
		coltab[j] = 0;
		HPt3Copy(&pl[j],&points[k]);
		vert_per_vect[j] = 1;
		for(i=0;i<num_itt-1;++i)
		{
		    if( !itterate(&points[k],&points[k+1],step_len))
		    {
			if(i!=0){ --k; --vert_per_vect[j]; }
			break;
		    }
		    ++k;
		    ++vert_per_vect[j];
		}
		++k;
	    }
	    else	/* Do it twice once in each dir */
	    {
		coltab[j*2] = 0;
		coltab[j*2+1] = 0;
		HPt3Copy(&pl[j],&points[k]);
		vert_per_vect[j*2] = 1;
		itt_vec[0] = 1.0; itt_vec[1] = 1.0;
		itt_vec[2] = 1.0; itt_vec[3] = 1.0;
		for(i=0;i<num_itt-1;++i)
		{
		    if( !itterate(&points[k],&points[k+1],step_len))
		    {
			if(i!=0){ --k; --vert_per_vect[j*2]; }
			break;
		    }
		    ++k;
		    ++vert_per_vect[j*2];
		}
		++k;

		vert_per_vect[j*2+1] = 1;
		HPt3Copy(&pl[j],&points[k]);
		itt_vec[0] = -1.0; itt_vec[1] = -1.0;
		itt_vec[2] = -1.0; itt_vec[3] = -1.0;
		for(i=0;i<num_itt-1;++i)
		{
		    if( !itterate(&points[k],&points[k+1],step_len))
		    {
			if(i!=0){ --k; --vert_per_vect[j*2+1]; }
			break;
		    }
		    ++k;
		    ++vert_per_vect[j*2+1];
		}
		++k;
	    }
	}

#ifndef NO_GEOM
	coltab[0] = 1;
	vect = GeomCreate("vect",
		CR_4D,global_dim==4 ? 1 : 0,	/* 4d results */
		CR_NVERT,k,			/* Number of vertices */
		CR_NVECT,n_vect,		/* Number of vectors */
		CR_POINT4,points,		/* The points */
		CR_VECTC,vert_per_vect,		/* verticies per vect */
		CR_NCOLR,1,			/* Number of colours */
		CR_COLOR,colours,		/* The colours */
		CR_COLRC,coltab,		/* # cols per vect */
		CR_END);
	GeomFSave(vect,stdout,NULL);
	fini_geom();
#endif

#ifdef CGIVRML
	printf("Content-type: text/plain\n\n");

	printf("OK Surface sucessfully calculated\n");

	print_icurve_jvx_header(filename);
	print_lines_as_jvx(points,k,vert_per_vect,n_vect,colindex);
#endif
}

void int_curve_psurf(char *filename)
{
#ifndef NO_GEOM
	Geom	*load_geom2();

	Transform Ident;
	Geom	*geom,*in_geom,*vect;
	CommentList     *defCom,*timeCom;
	Geom    *psurf_geom;
	int     new_edit_time;
	float	val;
	ColorA	*colours;
#endif
	char    comment_file_name[L_tmpnam];
	HPoint3 *pl,*points;
	int	*vert_per_vect,*coltab;
	int	num_pts,i,j,k,n_vect,num_itt,colindex,dim_p;
	float	step_len;
	FILE    *fp;
	int     res;



	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	global_oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	num_itt = (int) Mget_opt_val_by_name(main_mul,NUM_STEPS_NAME);
	step_len = (float) Mget_opt_val_by_name(main_mul,STEP_LEN_NAME);
	colindex = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

	/* check options are allowable */

	if(global_dim != THREE_D_EQN )
	{
		fprintf(stderr,
			"Equation must be 3D -> 3D when using operators\n");
		return;
	}

	/* Now read in the opperator */

#ifndef NO_GEOM
	TmIdentity(Ident);
	psurf_geom = load_geom2(psurf_tgt_name);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	defCom = GeomCommentGet(psurf_geom,LSMP_DEF_NAME);

	if( defCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_DEF_NAME);		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	else if( defCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	else if( defCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}

	timeCom = GeomCommentGet(psurf_geom,LSMP_EDIT_TIME_NAME);

	if( timeCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	else if( timeCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	 else if( timeCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}


	new_edit_time = atoi(timeCom->com->data);
	if( new_edit_time == 0 )
	{
		fprintf(stderr,"Bad timestamp data <%s>\n",timeCom->com->data);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",defCom->com->data);
	fclose(fp);
	fp = fopen(comment_file_name,"r");

	if( new_edit_time != psurf_edit_time )
	{
		psurf_edit_time = new_edit_time; 
		res = read_psurf_def(fp,psurf_mul);
	} else
	{
		res = fscanMultiParams(fp,psurf_mul);
	}
	fclose(fp);
	unlink(comment_file_name);

	CommentListFree(timeCom);
	CommentListFree(defCom);
	GeomFree(psurf_geom);
	if( !res )
	{
		psurf_edit_time = 0;
		return;
	}
#endif

	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}

#ifndef NO_GEOM
	in_geom = load_geom();

	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't calculate curves\n");
		fclose(fp);
	}
	start_geom();

	printf("LIST\n");
	printf("COMMENT icurve %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT icurve %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);


	geom = GeomSimp(in_geom);
	GeomFree(in_geom);

	pl = (HPoint3 *) PointList_get(geom,Ident,self_prim);
	num_pts = PointList_length(geom);
#endif

#ifdef CGIVRML
	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",auxGeomDef);
	fclose(fp);
	fp = fopen(comment_file_name,"r");
	res = read_psurf_def(fp,psurf_mul);
	if( !res ) 
	{
               	report_error(HEAD_ERROR,"Error reading psurf definition",3);
		exit(1);
	}

	pl = get_jvx_points(jvx,&num_pts,&dim_p);
#endif

	if( global_oriented == ORIENT_ORIENT )
	{
		points = OOGLNewN(HPoint3,num_pts*num_itt);
		vert_per_vect = OOGLNewN(int,num_pts);
		coltab = OOGLNewN(int,num_pts);
		n_vect = num_pts;
	}
	else
	{
		points = OOGLNewN(HPoint3,num_pts*num_itt*2);
		vert_per_vect = OOGLNewN(int,num_pts*2);
		coltab = OOGLNewN(int,num_pts*2);
		n_vect = num_pts * 2;
	}
#ifndef NO_GEOM
	colours = OOGLNewN(ColorA,1);
	switch(colindex)
	{
		case 0: /* Black   */ CoSet(colours[0],0.0,0.0,0.0); break;
		case 1: /* Red     */ CoSet(colours[0],1.0,0.0,0.0); break;
		case 2: /* Green   */ CoSet(colours[0],0.0,1.0,0.0); break;
		case 3: /* Yellow  */ CoSet(colours[0],1.0,1.0,0.0); break;
		case 4: /* Blue    */ CoSet(colours[0],0.0,0.0,1.0); break;
		case 5: /* Magenta */ CoSet(colours[0],1.0,0.0,1.0); break;
		case 6: /* Cyan    */ CoSet(colours[0],0.0,1.0,1.0); break;
		case 7: /* White   */
		default:
				      CoSet(colours[0],1.0,1.0,1.0); break;
	}
#endif

	k = 0;
	for(j=0;j<num_pts;++j)
	{
	    if( global_oriented == ORIENT_ORIENT )
	    {
		coltab[j] = 0;
		HPt3Copy(&pl[j],&points[k]);
		vert_per_vect[j] = 1;
		for(i=0;i<num_itt-1;++i)
		{
		    if( !itterate(&points[k],&points[k+1],step_len))
		    {
			if(i!=0){ --k; --vert_per_vect[j]; }
			break;
		    }
		    ++k;
		    ++vert_per_vect[j];
		}
		++k;
	    }
	    else	/* Do it twice once in each dir */
	    {
		coltab[j*2] = 0;
		coltab[j*2+1] = 0;
		HPt3Copy(&pl[j],&points[k]);
		vert_per_vect[j*2] = 1;
		itt_vec[0] = 1.0; itt_vec[1] = 1.0;
		itt_vec[2] = 1.0; itt_vec[3] = 1.0;
		for(i=0;i<num_itt-1;++i)
		{
		    if( !itterate(&points[k],&points[k+1],step_len))
		    {
			if(i!=0){ --k; --vert_per_vect[j*2]; }
			break;
		    }
		    ++k;
		    ++vert_per_vect[j*2];
		}
		++k;

		vert_per_vect[j*2+1] = 1;
		HPt3Copy(&pl[j],&points[k]);
		itt_vec[0] = -1.0; itt_vec[1] = -1.0;
		itt_vec[2] = -1.0; itt_vec[3] = -1.0;
		for(i=0;i<num_itt-1;++i)
		{
		    if( !itterate(&points[k],&points[k+1],step_len))
		    {
			if(i!=0){ --k; --vert_per_vect[j*2+1]; }
			break;
		    }
		    ++k;
		    ++vert_per_vect[j*2+1];
		}
		++k;
	    }
	}
	if( global_mode == MODE_PSURF_PROJ )
	{
		for(i=0;i<k;++i)
		{
			PsurfMap(&(points[i]));
		}
	}
#ifndef NO_GEOM
	coltab[0] = 1;
	vect = GeomCreate("vect",
		CR_4D,global_dim==4 ? 1 : 0,	/* 4d results */
		CR_NVERT,k,			/* Number of vertices */
		CR_NVECT,n_vect,		/* Number of vectors */
		CR_POINT4,points,		/* The points */
		CR_VECTC,vert_per_vect,		/* verticies per vect */
		CR_NCOLR,1,			/* Number of colours */
		CR_COLOR,colours,		/* The colours */
		CR_COLRC,coltab,		/* # cols per vect */
		CR_END);
	GeomFSave(vect,stdout,NULL);
	fini_geom();
#endif

#ifdef CGIVRML
	printf("Content-type: text/plain\n\n");

	printf("OK Surface sucessfully calculated\n");

	print_icurve_jvx_header(filename);
	print_lines_as_jvx(points,k,vert_per_vect,n_vect,colindex);
#endif
}

/*
 * Function:	vec_fields
 * Action:	for each point in the target draw the vectors
 */

#ifdef NOT_DEF
void vec_field(char *filename)
{
	Transform Ident;
	Geom	*geom,*in_geom,*vect;
	HPoint3 *pl,*points;
	short	*vert_per_vect,*coltab;
	ColorA	*colours;
	int	num_pts,i,j,k,l,colindex;
	float	val,step_len;
	double  dx,dy,dz,dw;
	int	res;
	int     new_edit_time;
	FILE    *fp;

	if( main_mul->error )
	{
		fprintf(stderr,"Can't calculate intergral curves - bad equations\n");
		return;
	}
	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
	{
		return(vec_field_psurf(filename));
	}
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}

	TmIdentity(Ident);
	in_geom = load_geom();

	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't calculate curves\n");
		fclose(fp);
	}
	start_geom();
	printf("LIST\n");
	printf("COMMENT mapping %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT mapping %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	geom = GeomSimp(in_geom);
	GeomFree(in_geom);
	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	global_oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	step_len = (float) Mget_opt_val_by_name(main_mul,STEP_LEN_NAME);
	colindex = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

	pl = (HPoint3 *) PointList_get(geom,Ident,self_prim);
	num_pts = PointList_length(geom);

	points = OOGLNewN(HPoint3,num_pts*2);
	vert_per_vect = OOGLNewN(short,num_pts);
	coltab = OOGLNewN(short,num_pts);
	colours = OOGLNewN(ColorA,1);
	switch(colindex)
	{
		case 0: /* Black   */ CoSet(colours[0],0.0,0.0,0.0); break;
		case 1: /* Red     */ CoSet(colours[0],1.0,0.0,0.0); break;
		case 2: /* Green   */ CoSet(colours[0],0.0,1.0,0.0); break;
		case 3: /* Yellow  */ CoSet(colours[0],1.0,1.0,0.0); break;
		case 4: /* Blue    */ CoSet(colours[0],0.0,0.0,1.0); break;
		case 5: /* Magenta */ CoSet(colours[0],1.0,0.0,1.0); break;
		case 6: /* Cyan    */ CoSet(colours[0],0.0,1.0,1.0); break;
		case 7: /* White   */ 
		default:	      CoSet(colours[0],1.0,1.0,1.0); break;
	}

	k = 0; l = 0;
	for(j=0;j<num_pts;++j)
	{
	    coltab[j] = 0;
	    HPt3Copy(&pl[j],&points[k],1);
	    res = itterate(&points[k],&points[k+1],step_len);
	    if( res )
	    {
	    	if( global_oriented != ORIENT_ORIENT )
		{
			dx = points[k+1].x - points[k].x;
			dy = points[k+1].y - points[k].y;
			dz = points[k+1].z - points[k].z;
			dw = points[k+1].w - points[k].w;
			points[k].x -= dx/2.0; points[k+1].x -= dx/2.0;
			points[k].y -= dy/2.0; points[k+1].y -= dy/2.0;
			points[k].z -= dz/2.0; points[k+1].z -= dz/2.0;
			points[k].w -= dw/2.0; points[k+1].w -= dw/2.0;
		}
		vert_per_vect[l] = 2;
		++l;
		++k;
	    	++k;
	    }
	}
	coltab[0] = 1;

	vect = GeomCreate("vect",
		CR_4D,global_dim==4 ? 1 : 0,	/* 4d results */
		CR_NVERT,k,			/* Number of vertices */
		CR_NVECT,l,		/* Number of vectors */
		CR_POINT4,points,		/* The points */
		CR_VECTC,vert_per_vect,		/* verticies per vect */
		CR_NCOLR,1,			/* Number of colours */
		CR_COLOR,colours,		/* The colours */
		CR_COLRC,coltab,		/* # cols per vect */
		CR_END);
	GeomFSave(vect,stdout,NULL);
	fini_geom();
}

void vec_field_psurf(char *filename)
{
	Geom	*load_geom2();
	Transform Ident;
	Geom	*geom,*in_geom,*vect,*psurf_geom;
	HPoint3 *pl,*points;
	short	*vert_per_vect,*coltab;
	ColorA	*colours;
	int	num_pts,i,j,k,l,colindex;
	float	val,step_len;
	double  dx,dy,dz,dw,len;
	int	res;
	int     new_edit_time;
	FILE    *fp;
	CommentList     *defCom,*timeCom;
	char    comment_file_name[L_tmpnam];


	if( main_mul->error )
	{
		fprintf(stderr,"Can't calculate intergral curves - bad equations\n");
		return;
	}

	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	global_oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	step_len = (float) Mget_opt_val_by_name(main_mul,STEP_LEN_NAME);
	colindex = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

	if(global_dim != THREE_D_EQN )
	{
		fprintf(stderr,
			"Equation must be 3D -> 3D when using operators\n");
		return;
	}
	/* Now read in the opperator */

	psurf_geom = load_geom2(psurf_tgt_name);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/


	defCom = GeomCommentGet(psurf_geom,LSMP_DEF_NAME);

	if( defCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_DEF_NAME);		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	else if( defCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	else if( defCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}


	timeCom = GeomCommentGet(psurf_geom,LSMP_EDIT_TIME_NAME);

	if( timeCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	else if( timeCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
	else if( timeCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}


	new_edit_time = atoi(timeCom->com->data);
	if( new_edit_time == 0 )
	{
		fprintf(stderr,"Bad timestamp data <%s>\n",timeCom->com->data);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(psurf_geom);
		return;
	}
/*
fprintf(stderr,"psurf Equation is\n%s\n",defCom->com->data);
*/

	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",defCom->com->data);
	fclose(fp);
	fp = fopen(comment_file_name,"r");

	if( new_edit_time != psurf_edit_time )
	{
		psurf_edit_time = new_edit_time;
		res = read_psurf_def(fp,psurf_mul);
	} else
	{
		res = fscanMultiParams(fp,psurf_mul);
	}
	fclose(fp);
	unlink(comment_file_name);


	CommentListFree(timeCom);
	CommentListFree(defCom);
	GeomFree(psurf_geom);
	if( !res )
	{
		psurf_edit_time = 0;
		return;
	}

	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}


	TmIdentity(Ident);
	in_geom = load_geom();

	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't calculate curves\n");
		fclose(fp);
	}
	start_geom();
	printf("LIST\n");
	printf("COMMENT mapping %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT mapping %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	geom = GeomSimp(in_geom);
	GeomFree(in_geom);


	pl = (HPoint3 *) PointList_get(geom,Ident,self_prim);
	num_pts = PointList_length(geom);

	points = OOGLNewN(HPoint3,num_pts*2);
	vert_per_vect = OOGLNewN(short,num_pts);
	coltab = OOGLNewN(short,num_pts);
	colours = OOGLNewN(ColorA,1);
	switch(colindex)
	{
		case 0: /* Black   */ CoSet(colours[0],0.0,0.0,0.0); break;
		case 1: /* Red     */ CoSet(colours[0],1.0,0.0,0.0); break;
		case 2: /* Green   */ CoSet(colours[0],0.0,1.0,0.0); break;
		case 3: /* Yellow  */ CoSet(colours[0],1.0,1.0,0.0); break;
		case 4: /* Blue    */ CoSet(colours[0],0.0,0.0,1.0); break;
		case 5: /* Magenta */ CoSet(colours[0],1.0,0.0,1.0); break;
		case 6: /* Cyan    */ CoSet(colours[0],0.0,1.0,1.0); break;
		case 7: /* White   */ 
		default:	      CoSet(colours[0],1.0,1.0,1.0); break;
	}

	k = 0; l = 0;
	for(j=0;j<num_pts;++j)
	{
	    coltab[j] = 0;
	    HPt3Copy(&pl[j],&points[k],1);
	    res = itterate(&points[k],&points[k+1],step_len);
	    if( res )
	    {
		if( global_mode == MODE_PSURF_PROJ )
		{
			dx = points[k+1].x - points[k].x;
			dy = points[k+1].y - points[k].y;
			points[k+1].x = dx;
			points[k+1].y = dy;
			points[k+1].z = 0.0;
			points[k+1].w = 1.0;
			PsurfMapTgt(&(points[k]),&(points[k+1]));

			len = sqrt( points[k+1].x * points[k+1].x
				+ points[k+1].y * points[k+1].y 
				+ points[k+1].z * points[k+1].z );
			if( len != len || len == 0.0 )
			{
				points[k+1].x = points[k].x;
				points[k+1].y = points[k].y;
				points[k+1].z = points[k].z;
				points[k+1].w = 1.0;
			}
			else if( global_oriented == ORIENT_ORIENT )
			{
				points[k+1].x += points[k].x;
				points[k+1].y += points[k].y;
				points[k+1].z += points[k].z;
				points[k+1].w += 1.0;
			}
			else /* all the other types */
{
	points[k].x += step_len * points[k+1].x/(2*len);
	points[k+1].x = points[k].x - step_len * points[k+1].x / len;
	points[k].y += step_len * points[k+1].y/(2*len);
	points[k+1].y = points[k].y - step_len * points[k+1].y / len;
	points[k].z += step_len * points[k+1].z/(2*len);
	points[k+1].z = points[k].z - step_len * points[k+1].z / len;
	points[k].w = points[k+1].w = 1.0;
}
		}
		else if( global_oriented != ORIENT_ORIENT )
		{
			dx = points[k+1].x - points[k].x;
			dy = points[k+1].y - points[k].y;
			dz = points[k+1].z - points[k].z;
			dw = points[k+1].w - points[k].w;
			points[k].x -= dx/2.0; points[k+1].x -= dx/2.0;
			points[k].y -= dy/2.0; points[k+1].y -= dy/2.0;
			points[k].z -= dz/2.0; points[k+1].z -= dz/2.0;
			points[k].w -= dw/2.0; points[k+1].w -= dw/2.0;
		}
/*
fprintf(stderr,"A pt1 (%f,%f,%f,%f) pt2 (%f,%f,%f,%f)\n",
	points[k].x, points[k].y, points[k].z, points[k].w,
	points[k+1].x, points[k+1].y, points[k+1].z, points[k+1].w);
*/
		vert_per_vect[l] = 2;
		++l;
		++k;
	    	++k;
	    } /* reuse current if itterate failed */
	}
	coltab[0] = 1;

	vect = GeomCreate("vect",
		CR_4D,global_dim==4 ? 1 : 0,	/* 4d results */
		CR_NVERT,k,			/* Number of vertices */
		CR_NVECT,l,			/* Number of vectors */
		CR_POINT4,points,		/* The points */
		CR_VECTC,vert_per_vect,		/* verticies per vect */
		CR_NCOLR,1,			/* Number of colours */
		CR_COLOR,colours,		/* The colours */
		CR_COLRC,coltab,		/* # cols per vect */
		CR_END);
	GeomFSave(vect,stdout,NULL);
	fini_geom();
}
/*
 * Function:	wave_front
 * Action:	calculates a wave front
 */

void wave_front(char *filename)
{
	Geom *in_geom;
	Transform Ident;

	TmIdentity(Ident);

	if( main_mul->error )
	{
		fprintf(stderr,"Can't calculate intergral curves - bad equations\n");
		return;
	}

	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	global_oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);

	if( global_oriented != ORIENT_ORIENT )
	{
		fprintf(stderr,"Can only calculate wave fronts for oriented vector fields\n");
		return;
	}

	in_geom = load_geom();

	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't calculate wave front\n");
		return;
	}
	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);
	if( wave_geom != NULL ) GeomFree(wave_geom);
	wave_geom = GeomSimp(in_geom);
	GeomFree(in_geom);
	 /* pl = (HPoint3 *) PointList_get(geom,Ident,self_prim); */
	wave_pl = (HPoint3 *) PointList_get(wave_geom,Ident,self_prim);
	wave_num_pts = PointList_length(wave_geom);
	next_wave_front(filename);
}

/*
 * Function:	next_wave_front
 * Action:	increment the geom
 */

void next_wave_front(char *filename)
{
	HPoint3 point1,point2;
	int	j;
	float	step_len;
	FILE    *fp;


	if( wave_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't calculate wave front\n");
		return;
	}
	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
	{
		fprintf(stderr,"Sorry can't use operators and wavefronts\n");
		return;
	}
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}

	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	global_oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	step_len = (float) Mget_opt_val_by_name(main_mul,STEP_LEN_NAME);


	for(j=0;j<wave_num_pts;++j)
	{
	    if( global_oriented == ORIENT_ORIENT )
	    {
		HPt3Copy(&wave_pl[j],&point1,1);
		itterate(&point1,&point2,step_len);
/*
		Pt3ToPt4(&point2,&wave_pl[j],1);
*/
		wave_pl[j].x = CLIP(point2.x);
		wave_pl[j].y = CLIP(point2.y);
		wave_pl[j].z = CLIP(point2.z);
		wave_pl[j].w = CLIP(point2.w);
	    }
	}
	PointList_set(wave_geom,self_prim,wave_pl);
	start_geom();
	printf("LIST\n");
	printf("COMMENT icurve %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT icurve %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	GeomFSave(wave_geom,stdout,NULL);
	fini_geom();
}
#endif

/********* file handeling ***********************/

/*
 * Function:	read_def
 * Action:	read in a new equation from 'filename',
 *		create a new geometry, set the default values etc..
 */

int read_def(FILE *fp)
{
	int     dimension,colours,local_oriented;

	edit_time = 0;
	Mset_opt_val_by_name(main_mul,MODE_NAME,(float) global_mode);
	if( !fscanMulti(fp,main_mul) ) return(FALSE);
	if( !MfindOpts(main_mul) ) return(FALSE);

	dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	local_oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	global_mode = (int) Mget_opt_val_by_name(main_mul,MODE_NAME);

	MsetNtop(main_mul,1);

	if( dimension == FOUR_D_EQN 
		|| local_oriented == ORIENT_MAJOR 
		|| local_oriented == ORIENT_MINOR )
		MsetTopDim(main_mul,0,4);
	else
		MsetTopDim(main_mul,0,3);

	if( !McombineTop(main_mul) ) return(FALSE);

	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
	{
		MsetNvars(main_mul,5);       /* i.e. ?1,?2,?3 : ?4,?5 */
		Mset_var_name(main_mul,0,"?1");
		Mset_var_name(main_mul,1,"?2");
		Mset_var_name(main_mul,2,"?3");
		Mset_var_name(main_mul,3,"?4");
		Mset_var_name(main_mul,4,"?5");
	}

	if( !MfindNames(main_mul) ) return(FALSE);
	
	MsetNtopDerivs(main_mul,0,0);

	if( !McheckDims(main_mul) ) return(FALSE);
	if( !McalcDerivs(main_mul) ) return(FALSE);
	if( !McalcRPEs(main_mul) ) return(FALSE);

	edit_time = time(NULL);
	return(TRUE);
}

/*
 * When we read form a file we want to reset the variable names
 */

int read_file(FILE *fp)
{
	int i;
	MsetNvars(main_mul,3);
	Mset_var_name(main_mul,0,"x");
	Mset_var_name(main_mul,1,"y");
	Mset_var_name(main_mul,1,"z");
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) PREC_DEFAULT);
	Mset_opt_val_by_name(main_mul,CLIP_NAME,CLIP_DEFAULT);
	Mset_opt_val_by_name(main_mul,DIM_NAME,(double) THREE_D_EQN);
	Mset_opt_val_by_name(main_mul,COL_NAME,(double) NO_COL);
	Mset_opt_val_by_name(main_mul,STEP_LEN_NAME,STEP_LEN_DEFAULT);
	Mset_opt_val_by_name(main_mul,NUM_STEPS_NAME,NUM_STEPS_DEFAULT);
	Mset_opt_val_by_name(main_mul,ORIENT_NAME,ORIENT_DEFAULT);
	Mset_opt_val_by_name(main_mul,MODE_NAME,(double) MODE_SIMPLE);
	global_mode = MODE_SIMPLE;
	i=read_def(fp);
	return(i);
}

int read_psurf_def(FILE *fp,Multi *psurf_mul)
{
	int     dimension,normals,colours;

	Mclear(psurf_mul);
	if( !fscanMulti(fp,psurf_mul) ) return(FALSE);
	if( !MfindOpts(psurf_mul) ) return(FALSE);

	dimension = (int) Mget_opt_val_by_name(psurf_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(psurf_mul,COL_NAME);
	normals = (int) Mget_opt_val_by_name(psurf_mul,NORM_NAME);

	if(colours == EQN_COL )
	{
		fprintf(stderr,"Can't have operators and equations for colours\n");
		return(FALSE);
	}
	if(normals == EQN_NORM )
	{
		fprintf(stderr,"Can't have operators and equations for normals\n");
		return(FALSE);
	}
	if(dimension != THREE_D_EQN )
	{
		fprintf(stderr,"Psurf must be 3D when using operators\n");
		return(FALSE);
	}
	MsetNtop(psurf_mul,1);
	MsetTopDim(psurf_mul,0,3);

	if( !McombineTop(psurf_mul) ) return(FALSE);

	MsetNvars(psurf_mul,2);
	Mset_var_name(psurf_mul,0,"x");
	Mset_var_name(psurf_mul,0,"y");
/*
fprint_multi(stderr,psurf_mul);
*/
	if( !MfindNames(psurf_mul) ) return(FALSE);
        MsetNtopDerivs(psurf_mul,0,2);
        MsetDerivName(psurf_mul,0,0,Mget_var_name(psurf_mul,0));
        MsetDerivName(psurf_mul,0,1,Mget_var_name(psurf_mul,1));


	if( !McheckDims(psurf_mul) ) return(FALSE);

	MTdefine(psurf_trans,main_mul,psurf_mul);
	MTsetVarTop(psurf_trans,0,0,0);
	MTsetVarTop(psurf_trans,1,0,1);
	MTsetVarTop(psurf_trans,2,0,2);
	MTsetVarVar(psurf_trans,3,0);
	MTsetVarVar(psurf_trans,4,1);
	MTcalcVarTrans(psurf_trans);
	if( !MTcheck(psurf_trans) ) return(FALSE);

	if( !McalcDerivs(psurf_mul) ) return(FALSE);
	if( !McalcRPEs(psurf_mul) ) return(FALSE);

	return(TRUE);
}


/*
 * When we change the number of variables say by adding colour
 * we need to re-do the variable names etc
 */

void change_n_vars(int new_dim)
{
	int offset = 0,n_vars;
	int old_dim;
	char *Xname,*Yname,*Zname,*Wname;

	/* first gothrough existing names in the multi */

	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
		return;
	old_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	Xname = strdup(Mget_var_name(main_mul,offset++));
	Yname = strdup(Mget_var_name(main_mul,offset++));
	Zname = strdup(Mget_var_name(main_mul,offset++));
	if( old_dim == FOUR_D_EQN )
		Wname = strdup(Mget_var_name(main_mul,offset++));
	else
		Wname = "w";


#ifdef NOT_DEF
	fprintf(stderr,"change_n_vars\n");
#endif

	n_vars = 3;
	if( new_dim == FOUR_D_EQN )
		++n_vars;

	MsetNvars(main_mul,n_vars);
	offset = 0;
	Mset_var_name(main_mul,offset++,Xname);
	Mset_var_name(main_mul,offset++,Yname);
	Mset_var_name(main_mul,offset++,Zname);
	if(new_dim == FOUR_D_EQN )
		Mset_var_name(main_mul,offset++,Wname);
		
	Mset_opt_val_by_name(main_mul,DIM_NAME,(double) new_dim);
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

void copy_def(FILE *in,FILE *out)
{
	int i,c,cc,spaces=0,name_ptr,dont_copy;
	char name[80];

	for(i=0;i<main_mul->copy_top;++i)
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

#ifndef NO_GEOM
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

	fp = fopen(argv[1],"r");
	fq = fopen(argv[2],"w");
	if( fp == NULL || fq == NULL )
	{
		interp->result = "Could not open file";
		return TCL_ERROR;
	}
	read_file(fp);
	rewind(fp);
	copy_def(fp,fq);
	fclose(fp);
	fclose(fq);
	set_geom_base_name(argv[1]);
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
	fq = fopen(argv[2],"r");
	if( fp == NULL || fq == NULL)
	{
		interp->result = "Could not save file";
		return TCL_ERROR;
	}
	copy_def(fq,fp);
	fprint_Mopts(fp,main_mul);
	fclose(fp);
	fclose(fq);
	set_geom_base_name(argv[1]);
	return TCL_OK;
}

/*
 * Update the equations, called when editor is changed
 *	given the name of editor file
 */

int update_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp;
	int offset = 0;

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
	read_def(fp);
	fclose(fp);

	return TCL_OK;
}

/*
 * Run the program vector/intergral/wavefront/itterate
 */

int vector_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "run_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	vec_field(argv[1]);
	return TCL_OK;
}

int integral_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "run_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	int_curve(argv[1]);
	return TCL_OK;
}

int wavefront_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "run_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	wave_front(argv[1]);
	return TCL_OK;
}

int itterate_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "run_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	next_wave_front(argv[1]);
	return TCL_OK;
}

int get_progname(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	sprintf(interp->result,"%s",prog_name);
	return TCL_OK;
}

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int precision,colour,dimension,num_steps,oriented;
	float step_len,clip;
	colour = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	precision = (int) Mget_opt_val_by_name(main_mul,PREC_NAME);
	dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	oriented = (int) Mget_opt_val_by_name(main_mul,ORIENT_NAME);
	num_steps = (int) Mget_opt_val_by_name(main_mul,NUM_STEPS_NAME);
	clip = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	step_len = (float) Mget_opt_val_by_name(main_mul,STEP_LEN_NAME);

	sprintf(interp->result,"%d %.2f %d %d %d %f %d\n",
		precision,clip,colour,oriented,num_steps,step_len,dimension);
	return TCL_OK;
}

int set_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 8)
	{
		interp->result = "set_options: Wrong number of arguments";
		return TCL_ERROR;
	}
	Mset_opt_val_by_name(main_mul,PREC_NAME, atof(argv[1]) );
	Mset_opt_val_by_name(main_mul,CLIP_NAME, atof(argv[2]) );
	Mset_opt_val_by_name(main_mul,COL_NAME, atof(argv[3]) );
	Mset_opt_val_by_name(main_mul,ORIENT_NAME, atof(argv[4]) );
	Mset_opt_val_by_name(main_mul,NUM_STEPS_NAME, atof(argv[5]) );
	Mset_opt_val_by_name(main_mul,STEP_LEN_NAME, atof(argv[6]) );
	change_n_vars( atoi(argv[7]) );
	return TCL_OK;
}

int set_mode(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "set_mode: Wrong number of arguments";
		return TCL_ERROR;
	}
	global_mode = atoi(argv[1]);
	Mset_opt_val_by_name(main_mul,MODE_NAME,(float) global_mode);
	return TCL_OK;
}

int get_mode(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	sprintf(interp->result,"%d\n",global_mode);
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
		interp->result = "set_param: Wrong number of arguments";
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
		interp->result = "get_param: Wrong number of arguments";
		return TCL_ERROR;
	}
	i = atoi(argv[1]);
	sprintf(interp->result,"%s %f",
		Mget_param_name(main_mul,i),
		Mget_param_val(main_mul,i));
	return TCL_OK;
}

/*
 * Just return variable names
 */

int get_variables(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int dim;
	dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	if( dim == 3 )
	sprintf(interp->result,"%s %s %s\n",
		Mget_var_name(main_mul,0),
		Mget_var_name(main_mul,1),
		Mget_var_name(main_mul,2));
	else
	sprintf(interp->result,"%s %s %s %s\n",
		Mget_var_name(main_mul,0),
		Mget_var_name(main_mul,1),
		Mget_var_name(main_mul,2),
		Mget_var_name(main_mul,3));
	return TCL_OK;
}

int get_env(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	char    *tcl_dir, *examples_dir;
	tcl_dir = getenv("LSMP_TCL");
	examples_dir = getenv("LSMP_EXAMPLES");
	sprintf(interp->result,"%s %s",tcl_dir,examples_dir);
	return(TCL_OK);
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

int psurf_target_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "psurf_target_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	strcpy(psurf_tgt_name,argv[1]);
	return TCL_OK;
}

/*
 * Gets the list of posible targets
 */

int get_targets_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	get_target_names(tgt_names,TGT_NM_SZ);
	interp->result = tgt_names;
	return TCL_OK;
}

#endif

/******* The main routine *********************************************/

int main(int argc, char **argv)
{
	FILE	*fp;
	int	i;
#ifndef NO_GEOM
	char	string[5],tcl_file[128],*tcl_dir;
	long	dev;
	short	val;

	Tcl_Interp *interp;
	int code;
	Tk_Window wind;

	/* First check permision */

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
#endif
#ifndef GDB
        freopen("asurf.error","a",stderr);
#endif


	/* read in arguments */

	main_mul = grballoc(Multi);
	psurf_mul = grballoc(Multi);
	Minit(main_mul);
	Minit(psurf_mul);
	psurf_trans = grballoc(MTrans);

#ifdef CGIVRML
	fprintf(stderr,"icurveCV A\n");
	read_lsmp_xml();
	print_time_message("icurveCV: done read_cgi");
#else
	icurve_args(argc,argv);
#endif
	init_funs();
#ifndef NO_GEOM
	pointlist_init();
	Simp_init();
	CommentExtract_init();
#endif
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
		use_arg_vals();
#ifndef NO_GEOM
		set_geom_name(arg_filename);
#endif
#ifdef NOT_DEF
		if( do_vect_field ) vec_field(arg_filename);
		else if( do_wavefront ) wave_front(arg_filename);
		else
#endif
			int_curve(arg_filename);
		if( temp_flag ) unlink(temp_file_name);
		exit(0);
	}

#ifndef NO_GEOM
	if( arg_filename != NULL)
	{
		fp = fopen(arg_filename,"r");
		if(fp == NULL)
		{
			fprintf(stderr,"Could not open %s\n",arg_filename);
			exit(-1);
		}
		read_def(fp);
		use_arg_vals();
		set_geom_base_name(arg_filename);
	}
	else
		set_geom_base_name(prog_name);

	interp = Tcl_CreateInterp();
/*
	wind = Tk_CreateMainWindow(interp,NULL,prog_name, "icurve");
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
	Tcl_CreateCommand(interp,"vector_cb",vector_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"wavefront_cb",wavefront_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"integral_cb",integral_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"itterate_cb",itterate_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_variables",get_variables,
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

	Tcl_CreateCommand(interp,"target_cb",target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_targets_cb",get_targets_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"object_cb",object_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_env",get_env,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_mode",get_mode,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"set_mode",set_mode,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"psurf_target_cb",psurf_target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	use_arg_vals();

	tcl_dir = getenv("LSMP_TCL");
	strcpy(tcl_file,tcl_dir);
	strcat(tcl_file,"/icurve.tcl");
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

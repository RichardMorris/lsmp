/*		
 *      file:   main.c:   
 *      author: Rich Morris
 *      date:   Jan 3rd 1995
 *      
 *	performs mapping on geomview objects defined by a set
 * 	of three equations
 */

/*
#define GDB
*/

#define CGIVRML
#define NO_GEOM
#define COMMAND_LINE
#ifndef NO_GEOM
#include <geom.h>
#include <color.h>
#include "normlist.h"
#include "geomsimp.h"
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef CGIVRML
#include <sys/wait.h>
#endif
#include <time.h>
#include <string.h>
/*
#include <getopt.h>
*/
#include <eqn.h>
#include <Multi.h>
#include <MTrans.h>
#include "../CVcommon.h"
#include "../lsmp.h"
#include "../jvx/jvx.h"
#include "../jvx/jvxCore.h"
extern void JvxMap(xml_tree *root);

#ifndef CGIVRML
#include <tcl.h>
#include <tk.h>
#endif

#ifndef NO_GEOM
#include "extractcomment.h"
#endif

#define OLD_EQN_COL 1
#define OLD_NO_COL 0

/*
#define DO_TANGENTS
#define PRINT_NORMALS
#define DISCRIM_LIM
#define PRINT_LIMIT
*/
#define CLEVER_OPS

#define grballoc(node) (node *) malloc( sizeof(node) )

/* What function the program performs, 4 = 4D coords, N = transform normals,
	C = alter colours  */

enum {Map} prog_type;
char	prog_name[10];
Multi   *main_mul,*psurf_mul,*pcurve_mul;
MTrans	*psurf_trans,*pcurve_trans;

eqn_funs *funlist = NULL;	/* list of functions */

/* Name of a target */

#define TGT_NM_SZ 1024

char	tgt_names[TGT_NM_SZ];	/* array to hold list of target names */

/* default values */

int	command = FALSE;		/* Write geomview comands */
int	quiet = TRUE;			/* quite mode */
double	clipmax;			/* clipping */
int	global_dim,global_cols,global_normals; /* Only used while evaluating */
double	tolerance = 0.0000001;
int	max_itt = 2;

/* stuff for operators */

int	global_mode = 0;	/* 0 = simple 1 = operator on a psurf */
int	edit_time = 0;		/* time this equation was last edited */
int	psurf_edit_time = 0;	/* time psurf was last edited */
char	psurf_tgt_name[64];	/* The target name for the psurf ingr */
int	pcurve_edit_time = 0;	/* time pcurve was last edited */
char	pcurve_tgt_name[64];	/* The target name for the pcurve ingr */
char	*auxGeomDef;		/* The def for the auxillary geom */

#define  CLIP(X) (clipmax < 0 ? X : (X > clipmax ? clipmax :( X < -clipmax ? -clipmax : ( X != X ? clipmax : X) )))
#define  COL_CLIP(X) (X > 1.0 ? 1.0 :( X < 0 ? 0 : ( X != X ? 1 : X) ))

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

int read_psurf_def(FILE *fp,Multi *psurf);
int read_pcurve_def(FILE *fp,Multi *pcurve);
void copy_def(FILE *in,FILE *out);
void change_n_vars(int new_norm,int new_col,int new_dim);

/*********** Argument handeling ***********************/

double arg_clipmax = 0.0;
int   arg_precision = -1;
int arg_dimension = 0;
int	arg_colours = -5;
int	arg_normals = -5;
double arg_vals[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* vals from arguments */
char   *arg_names[MAX_NUM_PARAMS + MAX_NUM_VARS];
int     arg_count=0;	/* number of params in arguments */
char	*arg_filename = NULL;	/* filename from arguments */
int	temp_flag = FALSE;	/* TRUE if equation def on command line */
char	temp_file_name[L_tmpnam];	/* temp file for equation */
xml_tree *jvx;
int     vrml_version = 1; /* the version of VRML produced */

void print_usage(char *name)
{
	fprintf(stderr,"Usage: %s [-3 dim|-4 dim] [-C] [-N norm] [-c clip] [-p precision] [-h]\n",name);
	fprintf(stderr,"\t\t[-D name val] {-G|-I|-e equation|filename}\n");
}


void map3_args(argc,argv)
int argc; char **argv;
{
#ifndef CGIVRML
	int i;
	extern char *optarg;
	extern  int  optind;
	FILE	*temp_file;
	char	*slash;

	/** we strip the first argument off **/

	--argc; ++argv;

	/** first find the name prog was called by **/

	slash = strrchr(argv[0],'/');
	if(slash == NULL) strcpy(prog_name,argv[0]);
	else		  strcpy(prog_name,slash+1);
	if( !strcmp(prog_name,"mapping") )
	{	prog_type = Map;
	}
	else
	{
		fprintf(stderr,"bad program name: %s\n",prog_name);
		exit(-1);
	}

	/* Now we can look at the arguments */

	while((i=getopt(argc,argv,"GIhe:c:p:D:4:3:CN:")) != -1 )
	{
		switch(i)
		{
		case '3': 
			if(atoi(optarg) == 4)
				arg_dimension = THREE_FOUR_D_EQN;
			else	arg_dimension = THREE_D_EQN;
			break;
		case '4': 
			if( atoi(optarg) == 4)
				arg_dimension = FOUR_D_EQN;
			else	arg_dimension = FOUR_THREE_D_EQN;
			break;
		case 'C': arg_colours = EQN_COL; break;
		case 'N': arg_normals = atoi(optarg); break;
		case 'c': arg_clipmax = atof(optarg); break;
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
#endif
}

void read_cgi()
{
        char *env_query;
        int cl;
        ncsa_entry entries[MAX_ENTRIES]; 
        register int x,m=0;
        char *def_ent,*clip_ent;
        char *indim_ent,*outdim_ent,*normals_ent,*coleqns_ent,*inputgeom_ent,*auxGeom_ent,*opType_ent;
        char *version_ent;
        FILE    *temp_file;
        FILE *fp,*fl;
                char *tmp,*tstr; time_t tim;

        def_ent=clip_ent=NULL;
        indim_ent=outdim_ent=normals_ent=coleqns_ent=inputgeom_ent=auxGeom_ent=opType_ent=NULL;
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
/*
*/
	if(strncmp(entries[x].val,"INPUTGEOM",9))
	        fprintf(stderr,"%s",entries[x].val);
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
	if(strncmp(entries[x].val,"INPUTGEOM",9))
	        fprintf(fl,"%s\n",entries[x].val);

        entries[x].name = makeword(entries[x].val,'=');


        if(!strcmp(entries[x].name,"DEF"))          def_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"INDIM"))    indim_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"OUTDIM"))   outdim_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"NORMALS"))  normals_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"COLEQNS"))  coleqns_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"CLIP"))  clip_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"VERSION")) version_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"INPUTGEOM")) inputgeom_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"AUXSURF")) auxGeom_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"OPTYPE")) opType_ent = entries[x].val;
        else 
        {    
/*
                report_error2(HEAD_ERROR,"Bad field name %s",entries[x].name,3);
                exit(1);
*/
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

        if(clip_ent != NULL )
                fprintf(temp_file,"_%s = %s;\n",CLIP_NAME,clip_ent);
	if(normals_ent != NULL )
	{
		if(!strcmp(normals_ent,NO_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,NO_NORM);
		else if(!strcmp(normals_ent,STD_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,STD_NORM);
		else if(!strcmp(normals_ent,DISC_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,DISC_NORM);
		else if(!strcmp(normals_ent,EQN_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,EQN_NORM);
		else
		{
			report_error2(HEAD_ERROR,"Bad type for normals %s",normals_ent,101);
			exit(1);
		}
	}
	if(indim_ent != NULL && outdim_ent != NULL)
	{
		if(!strcmp(indim_ent,"3"))
		{
			if(!strcmp(outdim_ent,"3"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,THREE_D_EQN);
			else if(!strcmp(outdim_ent,"4"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,THREE_FOUR_D_EQN);
			else
			{
				report_error2(HEAD_ERROR,"Bad output dimension %s",outdim_ent,102);
				exit(1);
			}
		}				
		else if(!strcmp(indim_ent,"4"))
		{
			if(!strcmp(outdim_ent,"3"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,FOUR_THREE_D_EQN);
			else if(!strcmp(outdim_ent,"4"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,FOUR_D_EQN);
			else
			{
				report_error2(HEAD_ERROR,"Bad output dimension %s",outdim_ent,103);
				exit(1);
			}
		}
		else				
		{
			report_error2(HEAD_ERROR,"Bad input dimension %s",indim_ent,104);
			exit(1);
		}
	}
	if(coleqns_ent!=NULL)
	{
		if(!strcmp(coleqns_ent,"true"))
				fprintf(temp_file,"_%s = %d;\n",COL_NAME,EQN_COL);
		else if(!strcmp(coleqns_ent,"false"))
				fprintf(temp_file,"_%s = %d;\n",COL_NAME,STD_COL);
		else				
		{
			report_error2(HEAD_ERROR,"Bad colour eqn type %s",coleqns_ent,105);
			exit(1);
		}
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
		if(!strcmp(opType_ent,"psurf"))
		{
                	fprintf(temp_file,"_mode = 1;\n");
			global_mode = MODE_PSURF;
		}
		else
		{
			report_error2(HEAD_ERROR,"Bad type for operator %s",opType_ent,13);
			exit(1);
		}
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

        if(inputgeom_ent == NULL)
        {
                report_error(HEAD_ERROR,"An XML format input geometry must be specified",106);
                exit(1);
        }
	jvx = parse_jvx(inputgeom_ent);
}

void read_lsmp_xml()
{
	LsmpInputSpec *spec;
	LsmpDef *def;
        LsmpOption *normals_ent,*coleqns_ent,*indim_ent,*outdim_ent,*version_ent,*clip_ent;
        FILE    *temp_file,*fl;
	int cl;
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

       arg_filename = tmpnam(temp_file_name);
       temp_file = fopen(arg_filename,"w");

        fprintf(temp_file,"%s\n",spec->Def->data);

	clip_ent = getLsmpOption(spec->Def,"clipping");
        if(clip_ent != NULL )
                fprintf(temp_file,"_%s = %s;\n",CLIP_NAME,clip_ent->value);

	normals_ent = getLsmpOption(spec->Def,"normals");
	if(normals_ent != NULL )
	{
		if(!strcmp(normals_ent->value,NO_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,NO_NORM);
		else if(!strcmp(normals_ent->value,STD_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,STD_NORM);
		else if(!strcmp(normals_ent->value,DISC_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,DISC_NORM);
		else if(!strcmp(normals_ent->value,EQN_NORM_NAME))
			fprintf(temp_file,"_%s = %d;\n",NORM_NAME,EQN_NORM);
		else
		{
			report_error2(HEAD_ERROR,"Bad type for normals %s",normals_ent->value,101);
			exit(1);
		}
	}

	indim_ent = getLsmpOption(spec->Def,"inDim");
	outdim_ent = getLsmpOption(spec->Def,"outDim");
	if(indim_ent != NULL && outdim_ent != NULL)
	{
		if(!strcmp(indim_ent->value,"3"))
		{
			if(!strcmp(outdim_ent->value,"3"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,THREE_D_EQN);
			else if(!strcmp(outdim_ent->value,"4"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,THREE_FOUR_D_EQN);
			else
			{
				report_error2(HEAD_ERROR,"Bad output dimension %s",outdim_ent->value,102);
				exit(1);
			}
		}			
		else if(!strcmp(indim_ent->value,"4"))
		{
			if(!strcmp(outdim_ent->value,"3"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,FOUR_THREE_D_EQN);
			else if(!strcmp(outdim_ent->value,"4"))
				fprintf(temp_file,"_%s = %d;\n",DIM_NAME,FOUR_D_EQN);
			else
			{
				report_error2(HEAD_ERROR,"Bad output dimension %s",outdim_ent->value,103);
				exit(1);
			}
		}
		else				
		{
			report_error2(HEAD_ERROR,"Bad input dimension %s",indim_ent->value,104);
			exit(1);
		}
	}
	coleqns_ent = getLsmpOption(spec->Def,"coleqns");
	if(coleqns_ent!=NULL)
	{
		if(!strcmp(coleqns_ent->value,"true"))
		{
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,EQN_COL);
			global_cols = EQN_COL;
		}
		else if(!strcmp(coleqns_ent->value,"false"))
		{
			fprintf(temp_file,"_%s = %d;\n",COL_NAME,STD_COL);
			global_cols = STD_COL;
		}
		else				
		{
			report_error2(HEAD_ERROR,"Bad colour eqn type %s",coleqns_ent->value,105);
			exit(1);
		}
	}

	if(spec->Def->opType != NULL)
	{
		if(!strcmp(spec->Def->opType,"None"))
		{
		}
		else if(!strcmp(spec->Def->opType,"psurf"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_PSURF);
			global_mode = MODE_PSURF;
		}
		else if(!strcmp(spec->Def->opType,"pcurve"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_PCURVE);
			global_mode = MODE_PCURVE;
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

/*
	This fun is called after file has been read to arguments
	are applied after file values
*/

void use_arg_vals()
{
  int i;

  if(arg_normals != -5)
	Mset_opt_val_by_name(main_mul,NORM_NAME,(double) arg_normals);
  if(arg_precision != -1 ) 
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) arg_precision);
  if(arg_clipmax != 0.0 )
	Mset_opt_val_by_name(main_mul,CLIP_NAME,arg_clipmax);
  if(arg_dimension != 0 )
	Mset_opt_val_by_name(main_mul,DIM_NAME,(double) arg_dimension);
  if(arg_colours != -5)
	Mset_opt_val_by_name(main_mul,COL_NAME,(double) arg_colours);


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
	Madd_opt(main_mul,NORM_NAME,(double) STD_NORM);
	Madd_opt(main_mul,COL_NAME,(double) NO_COL);
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

	MsetNvars(pcurve_mul,1);
	Mset_var_name(pcurve_mul,0,"x");
/*
	Mset_var_minmax(pcurve_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
	Mset_var_minmax(pcurve_mul,1,XMIN_DEFAULT,XMAX_DEFAULT);
*/
	Madd_opt(pcurve_mul,PREC_NAME,(double) PREC_DEFAULT);
	Madd_opt(pcurve_mul,CLIP_NAME,CLIP_DEFAULT);
/*
	Madd_opt(pcurve_mul,STEPS1_NAME,(double) PS_STEPS);
	Madd_opt(pcurve_mul,STEPS2_NAME,(double) PS_STEPS);
*/
	Madd_opt(pcurve_mul,DIM_NAME,3.0);
	Madd_opt(pcurve_mul,COL_NAME,(double) NO_COL);
}

/************* The guts actually does the work ***********************/

/*
 * Transform an individual point
 */

/* Clipping function */

double clip_fun(double x, double y, double z)
{
	if( clipmax < 0.0 ) 
	{
		if( x == x && y == y && z == z ) return(1.0);
		else
		{
/*			fprintf(stderr,"failed %f %f %f\n",x,y,z);
*/
			return(0.0/0.0);
		}
	}

	if( x != x || x >= clipmax || x <= -clipmax 
	 || y != y || y >= clipmax || y <= -clipmax 
	 || z != z || z >= clipmax || z <= -clipmax )
		return( 0.0 / 0.0 );
	return(1.0);
}
	
void PointMap_psurf(HPoint3 *pl, Point3 *nl, ColorA *cl);
void PointMap_pcurve(HPoint3 *pl, Point3 *nl, ColorA *cl);

void PointMap(HPoint3 *pl, Point3 *nl, ColorA *cl)
{
	double	val;
	double	a,b,c,d,e,f,g,h,i,v0,v1,v2,len;
	int	offset=0,norm_ref = 1;

	double *ptr;

	if( global_mode == MODE_PSURF )
	{
		PointMap_psurf(pl,nl,cl);
		return;
	}
	if( global_mode == MODE_PCURVE )
	{
		PointMap_pcurve(pl,nl,cl);
		return;
	}
/*
printf("POIntMAp dim %d col %d norm %d\n",global_dim,global_cols,global_normals);
*/
	/* Set the variables */
	Mset_var_val(main_mul,offset++,pl->x);
	Mset_var_val(main_mul,offset++,pl->y);
	Mset_var_val(main_mul,offset++,pl->z);
	if( global_dim == FOUR_D_EQN || global_dim == FOUR_THREE_D_EQN ) 
		Mset_var_val(main_mul,offset++,pl->w);
	if( global_cols == EQN_COL )
	{
		Mset_var_val(main_mul,offset++,cl->r);
		Mset_var_val(main_mul,offset++,cl->g);
		Mset_var_val(main_mul,offset++,cl->b);
		Mset_var_val(main_mul,offset++,cl->a);
		++norm_ref;
	}
	if( global_normals == EQN_NORM )
	{
		Mset_var_val(main_mul,offset++,nl->x);
		Mset_var_val(main_mul,offset++,nl->y);
		Mset_var_val(main_mul,offset++,nl->z);
	}

	/* Evaluate equations */

	MstartEval(main_mul);
	ptr = MevalTop2(main_mul,0);
	val = (double) *ptr; pl->x = CLIP(val); 
	val = (double) *(ptr+1); pl->y = CLIP(val); 
	val = (double) *(ptr+2); pl->z = CLIP(val); 
	if( global_dim == FOUR_D_EQN || global_dim == THREE_FOUR_D_EQN ) 
		val = (double) *(ptr+3); pl->w = CLIP(val); 

	if( global_cols == EQN_COL )
	{
		ptr = MevalTop2(main_mul,1);
		val = (double) *(ptr+0); cl->r = COL_CLIP(val); 
		val = (double) *(ptr+1); cl->g = COL_CLIP(val); 
		val = (double) *(ptr+2); cl->b = COL_CLIP(val); 
		val = (double) *(ptr+3); cl->a = COL_CLIP(val); 
	}
	if( global_normals == EQN_NORM )
	{
		ptr = MevalTop2(main_mul,norm_ref);
		val = (double) *(ptr+0); nl->x = CLIP(val); 
		len = val*val;
		val = (double) *(ptr+1); nl->y = CLIP(val); 
		len += val*val;
		val = (double) *(ptr+2); nl->z = CLIP(val); 
		len += val*val;
		len = sqrt(len);
		if(len == 0 || len != len)
		{
			nl->x = (double) 0.0;
			nl->y = (double) 0.0;
			nl->z = (double) 0.0;
		}
		else if( len == len ) /* Propper numbers */
		{
			nl->x /= len;
			nl->y /= len;
			nl->z /= len;
		}
	}

	else if( global_normals == STD_NORM || global_normals == DISC_NORM ) 
		/* calc normals from main_eqn */
	{
		/* Now transform the normals */
	
		ptr = MevalTopDeriv(main_mul,0,0);
		a = *ptr; d = *(ptr+1); g = *(ptr+2);
		ptr = MevalTopDeriv(main_mul,0,1);
		b = *ptr; e = *(ptr+1); h = *(ptr+2);
		ptr = MevalTopDeriv(main_mul,0,2);
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

		if( len < 0.001 || len != len )
		{
			nl->x = (double) 0.0;
			nl->y = (double) 0.0;
			nl->z = (double) 0.0;
		}
		else /* Propper numbers */
		{
			nl->x = (double) v0/len;
			nl->y = (double) v1/len;
			nl->z = (double) v2/len;
		} /* For NaN do nothing */
	}
}

/* Do the same this time for operators on psurf's */

void PointMap_psurf(HPoint3 *pl, Point3 *nl, ColorA *cl)
{
	double	val;
	double	a,b,c,d,e,f,v0,v1,v2,len;
	int	offset=0,norm_ref = 1;

	double *ptr;

	/* Set the variables */

	MstartEval(main_mul);
	MstartEval(psurf_mul);

	Mset_var_val(main_mul,3,pl->x);
	Mset_var_val(main_mul,4,pl->y);
	Mset_var_val(psurf_mul,0,pl->x);
	Mset_var_val(psurf_mul,1,pl->y);
#ifdef CLEVER_OPS
	offset = 5;
	if( global_dim == FOUR_D_EQN || global_dim == FOUR_THREE_D_EQN ) 
		Mset_var_val(main_mul,offset++,pl->w);
	if( global_cols == EQN_COL )
	{
		Mset_var_val(main_mul,offset++,cl->r);
		Mset_var_val(main_mul,offset++,cl->g);
		Mset_var_val(main_mul,offset++,cl->b);
		Mset_var_val(main_mul,offset++,cl->a);
		++norm_ref;
	}
	if( global_normals == EQN_NORM )
	{
		Mset_var_val(main_mul,offset++,nl->x);
		Mset_var_val(main_mul,offset++,nl->y);
		Mset_var_val(main_mul,offset++,nl->z);
	}
#endif

	/* Evaluate equations */

	MTevalForTop(psurf_trans,0);
	MTcopyVars(psurf_trans);
	ptr = MevalTop2(main_mul,0);
	val = (double) *ptr; pl->x = CLIP(val); 
	val = (double) *(ptr+1); pl->y = CLIP(val); 
	val = (double) *(ptr+2); pl->z = CLIP(val); 

#ifdef CLEVER_OPS
	if( global_dim == FOUR_D_EQN || global_dim == THREE_FOUR_D_EQN ) 
		val = (double) *(ptr+3); pl->w = CLIP(val); 

	if( global_cols == EQN_COL )
	{
		MTevalForTop(psurf_trans,1);
		MTcopyVars(psurf_trans);
		ptr = MevalTop(main_mul,1);
		val = (double) *(ptr+0); cl->r = COL_CLIP(val); 
		val = (double) *(ptr+1); cl->g = COL_CLIP(val); 
		val = (double) *(ptr+2); cl->b = COL_CLIP(val); 
		val = (double) *(ptr+3); cl->a = COL_CLIP(val); 
	}
	if( global_normals == EQN_NORM )
	{
		ptr = MevalTop(main_mul,norm_ref);
		val = (double) *(ptr+0); nl->x = CLIP(val); 
		len = val*val;
		val = (double) *(ptr+1); nl->y = CLIP(val); 
		len += val*val;
		val = (double) *(ptr+2); nl->z = CLIP(val); 
		len += val*val;
		len = sqrt(len);
		if(len == 0 || len != len)
		{
			nl->x = (double) 0.0;
			nl->y = (double) 0.0;
			nl->z = (double) 0.0;
		}
		else if( len == len ) /* Propper numbers */
		{
			nl->x /= len;
			nl->y /= len;
			nl->z /= len;
		}
	}
	else 
#endif
	if( global_normals == STD_NORM || global_normals == DISC_NORM ) 
		/* calc normals from main_eqn */
	{
		/* Now transform the normals */
	
		MTevalForTopDeriv(psurf_trans,0,0);
		MTevalForTopDeriv(psurf_trans,0,1);
		MTcopyVars(psurf_trans);
		ptr = MevalTopDeriv(main_mul,0,0);
		a = *ptr; b = *(ptr+1); c = *(ptr+2);
		ptr = MevalTopDeriv(main_mul,0,1);
		d = *ptr; e = *(ptr+1); f = *(ptr+2);
	
	/* Just have two tangent vectors (a,b,c) (d,e,f)
		take cross  product to get normal */
	
		v0 =  b*f-c*e;
		v1 =  c*d-a*f;
		v2 =  a*e-b*d;
		
		len = sqrt(v0*v0 + v1*v1 + v2*v2);

		if( len < 0.001 || len != len )
		{
/*
fprintf(stderr,"Zero normal at %f %f coeffs (%f %f %f)  (%f %f %f)\n",
		pl->x,pl->y,a,b,c,d,e,f);
*/
			nl->x = (double) 0.0;
			nl->y = (double) 0.0;
			nl->z = (double) 0.0;
		}
		else /* Propper numbers */
		{
			nl->x = (double) v0/len;
			nl->y = (double) v1/len;
			nl->z = (double) v2/len;
		} /* For NaN do nothing */
	}
}

void pcurve_cb(short prov_req[],double vals[])
{
        MTstdCallback(pcurve_trans,prov_req,vals);
}

/* Do the same this time for operators on psurf's */

void PointMap_pcurve(HPoint3 *pl, Point3 *nl, ColorA *cl)
{
	double	val;
	double	a,b,c,d,e,f,v0,v1,v2,len;

	double *ptr;

	/* Set the variables */

	MstartEval(main_mul);
	MstartEval(pcurve_mul);

	Mset_var_val(main_mul,3,pl->x);
	Mset_var_val(main_mul,4,pl->y);
	Mset_var_val(pcurve_mul,0,pl->x);

	/* Evaluate equations */

	MTevalForTop(pcurve_trans,0);
	MTcopyVars(pcurve_trans);
	ptr = MevalTopCB(main_mul,0,pcurve_cb);
	val = (double) *ptr; pl->x = CLIP(val); 
	val = (double) *(ptr+1); pl->y = CLIP(val); 
	val = (double) *(ptr+2); pl->z = CLIP(val); 

	if( global_normals == STD_NORM || global_normals == DISC_NORM ) 
		/* calc normals from main_eqn */
	{
		/* Now transform the normals */
	
		MTevalForTopDeriv(pcurve_trans,0,0);
		MTevalForTopDeriv(pcurve_trans,0,1);
		MTcopyVars(pcurve_trans);
		ptr = MevalTopDerivCB(main_mul,0,0,pcurve_cb);
		a = *ptr; b = *(ptr+1); c = *(ptr+2);
		ptr = MevalTopDerivCB(main_mul,0,1,pcurve_cb);
		d = *ptr; e = *(ptr+1); f = *(ptr+2);
	
	/* Just have two tangent vectors (a,b,c) (d,e,f)
		take cross  product to get normal */
	
		v0 =  b*f-c*e;
		v1 =  c*d-a*f;
		v2 =  a*e-b*d;
		
		len = sqrt(v0*v0 + v1*v1 + v2*v2);

		if( len < 0.001 || len != len )
		{
/*
fprintf(stderr,"Zero normal at %f %f coeffs (%f %f %f)  (%f %f %f)\n",
		pl->x,pl->y,a,b,c,d,e,f);
*/
			nl->x = (double) 0.0;
			nl->y = (double) 0.0;
			nl->z = (double) 0.0;
		}
		else /* Propper numbers */
		{
			nl->x = (double) v0/len;
			nl->y = (double) v1/len;
			nl->z = (double) v2/len;
		} /* For NaN do nothing */
	}
}

/*
 * Function;	limit_normal
 * Action:	calculates the normal when the surface is singular
 */

void LimitMap_psurf(HPoint3 *pt,Point3 *norm);
void LimitMap_pcurve(HPoint3 *pt,Point3 *norm);

void LimitMap(HPoint3 *pt,Point3 *norm)
{
	double	a,b,c,d,e,f,g,h,i,len1,len2;
	double  vec1[3],vec2[3],vec3[3],vec4[3],vec5[3],vec6[3];
	double  *ptr;

	/* First we must calculate the two tangent vectors
		try I^norm,j^norm,k^norm
	*/

	if( global_mode == MODE_PSURF )
	{
		LimitMap_psurf(pt,norm);
		return;
	}
	if( global_mode == MODE_PCURVE )
	{
		LimitMap_pcurve(pt,norm);
		return;
	}

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

	ptr = MevalTopDeriv(main_mul,0,0);
	a = *ptr; d = *(ptr+1); g = *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,1);
	b = *ptr; e = *(ptr+1); h = *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,2);
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

	if( global_normals == DISC_NORM )
	{
	vec6[0] = a * norm->x + b * norm->y + c * norm->z;
	vec6[1] = d * norm->x + e * norm->y + f * norm->z;
	vec6[2] = g * norm->x + h * norm->y + i * norm->z;
	}
	else if( global_normals == STD_NORM )
	{
	/* tangent vector which maps to 0 */

	vec5[0] = len2 * vec1[0] - len1 * vec2[0];
	vec5[1] = len2 * vec1[1] - len1 * vec2[1];
	vec5[2] = len2 * vec1[2] - len1 * vec2[2];

	/* Now calculate d^2<v5,v5> */

	ptr = MevalTopDeriv(main_mul,0,3);
	vec6[0] = vec5[0] * vec5[0] * *ptr;
	vec6[1] = vec5[0] * vec5[0] * *(ptr+1);
	vec6[2] = vec5[0] * vec5[0] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,4);
	vec6[0] += 2 * vec5[0] * vec5[1] * *ptr;
	vec6[1] += 2 * vec5[0] * vec5[1] * *(ptr+1);
	vec6[2] += 2 * vec5[0] * vec5[1] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,5);
	vec6[0] += 2 * vec5[0] * vec5[2] * *ptr;
	vec6[1] += 2 * vec5[0] * vec5[2] * *(ptr+1);
	vec6[2] += 2 * vec5[0] * vec5[2] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,6);
	vec6[0] += vec5[1] * vec5[1] * *ptr;
	vec6[1] += vec5[1] * vec5[1] * *(ptr+1);
	vec6[2] += vec5[1] * vec5[1] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,7);
	vec6[0] += 2 * vec5[1] * vec5[2] * *ptr;
	vec6[1] += 2 * vec5[1] * vec5[2] * *(ptr+1);
	vec6[2] += 2 * vec5[1] * vec5[2] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,8);
	vec6[0] += vec5[2] * vec5[2] * *ptr;
	vec6[1] += vec5[2] * vec5[2] * *(ptr+1);
	vec6[2] += vec5[2] * vec5[2] * *(ptr+2);
	}

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
		if( norm->x != norm->x ) norm->x = 0.0;
		if( norm->y != norm->y ) norm->y = 0.0;
		if( norm->z != norm->z ) norm->z = 0.0;
	}
	else
	{
		norm->x = norm->y = norm->z = 0.0;
	}
}	

/* as above this time with operators */

void LimitMap_psurf(HPoint3 *pt,Point3 *norm)
{
	double	a,b,c,d,e,f,g,h,i,len1,len2;
	double  vec1[3],vec2[3],vec3[3],vec4[3],vec5[3],vec6[3];
	double  *ptr;

	/* First we must calculate the two tangent vectors
		try I^norm,j^norm,k^norm
	*/
	norm->x = norm->y = norm->z = 0;
	return;

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

	MTevalForTopDeriv(psurf_trans,0,0);
	MTevalForTopDeriv(psurf_trans,0,1);
	MTevalForTopDeriv(psurf_trans,0,2);
	MTcopyVars(psurf_trans);
	ptr = MevalTopDeriv(main_mul,0,0);
	a = *ptr; d = *(ptr+1); g = *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,1);
	b = *ptr; e = *(ptr+1); h = *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,2);
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

	if( global_normals == DISC_NORM )
	{
	vec6[0] = a * norm->x + b * norm->y + c * norm->z;
	vec6[1] = d * norm->x + e * norm->y + f * norm->z;
	vec6[2] = g * norm->x + h * norm->y + i * norm->z;
	}
	else if( global_normals == STD_NORM )
	{
	/* tangent vector which maps to 0 */

	vec5[0] = len2 * vec1[0] - len1 * vec2[0];
	vec5[1] = len2 * vec1[1] - len1 * vec2[1];
	vec5[2] = len2 * vec1[2] - len1 * vec2[2];

	/* Now calculate d^2<v5,v5> */

	MTevalForTopDeriv(psurf_trans,0,3);
	MTevalForTopDeriv(psurf_trans,0,4);
	MTevalForTopDeriv(psurf_trans,0,5);
	MTevalForTopDeriv(psurf_trans,0,6);
	MTevalForTopDeriv(psurf_trans,0,7);
	MTevalForTopDeriv(psurf_trans,0,8);
	MTcopyVars(psurf_trans);
	ptr = MevalTopDeriv(main_mul,0,3);
	vec6[0] = vec5[0] * vec5[0] * *ptr;
	vec6[1] = vec5[0] * vec5[0] * *(ptr+1);
	vec6[2] = vec5[0] * vec5[0] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,4);
	vec6[0] += 2 * vec5[0] * vec5[1] * *ptr;
	vec6[1] += 2 * vec5[0] * vec5[1] * *(ptr+1);
	vec6[2] += 2 * vec5[0] * vec5[1] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,5);
	vec6[0] += 2 * vec5[0] * vec5[2] * *ptr;
	vec6[1] += 2 * vec5[0] * vec5[2] * *(ptr+1);
	vec6[2] += 2 * vec5[0] * vec5[2] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,6);
	vec6[0] += vec5[1] * vec5[1] * *ptr;
	vec6[1] += vec5[1] * vec5[1] * *(ptr+1);
	vec6[2] += vec5[1] * vec5[1] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,7);
	vec6[0] += 2 * vec5[1] * vec5[2] * *ptr;
	vec6[1] += 2 * vec5[1] * vec5[2] * *(ptr+1);
	vec6[2] += 2 * vec5[1] * vec5[2] * *(ptr+2);
	ptr = MevalTopDeriv(main_mul,0,8);
	vec6[0] += vec5[2] * vec5[2] * *ptr;
	vec6[1] += vec5[2] * vec5[2] * *(ptr+1);
	vec6[2] += vec5[2] * vec5[2] * *(ptr+2);
	}

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
		if( norm->x != norm->x ) norm->x = 0.0;
		if( norm->y != norm->y ) norm->y = 0.0;
		if( norm->z != norm->z ) norm->z = 0.0;
	}
	else
	{
		norm->x = norm->y = norm->z = 0.0;
	}
}

/* as above this time with operators */

void LimitMap_pcurve(HPoint3 *pt,Point3 *norm)
{
	double	a,b,c,d,e,f,g,h,i,len1,len2;
	double  vec1[3],vec2[3],vec3[3],vec4[3],vec5[3],vec6[3];
	double  *ptr;

	/* First we must calculate the two tangent vectors
		try I^norm,j^norm,k^norm
	*/
	norm->x = norm->y = norm->z = 0;
	return;

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

	MTevalForTopDeriv(pcurve_trans,0,0);
	MTevalForTopDeriv(pcurve_trans,0,1);
	MTevalForTopDeriv(pcurve_trans,0,2);
	MTcopyVars(pcurve_trans);
	ptr = MevalTopDerivCB(main_mul,0,0,pcurve_cb);
	a = *ptr; d = *(ptr+1); g = *(ptr+2);
	ptr = MevalTopDerivCB(main_mul,0,1,pcurve_cb);
	b = *ptr; e = *(ptr+1); h = *(ptr+2);
	ptr = MevalTopDerivCB(main_mul,0,2,pcurve_cb);
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

	if( global_normals == DISC_NORM )
	{
	vec6[0] = a * norm->x + b * norm->y + c * norm->z;
	vec6[1] = d * norm->x + e * norm->y + f * norm->z;
	vec6[2] = g * norm->x + h * norm->y + i * norm->z;
	}
	else if( global_normals == STD_NORM )
	{
	/* tangent vector which maps to 0 */

	vec5[0] = len2 * vec1[0] - len1 * vec2[0];
	vec5[1] = len2 * vec1[1] - len1 * vec2[1];
	vec5[2] = len2 * vec1[2] - len1 * vec2[2];

	/* Now calculate d^2<v5,v5> */

	MTevalForTopDeriv(pcurve_trans,0,3);
	MTevalForTopDeriv(pcurve_trans,0,4);
	MTevalForTopDeriv(pcurve_trans,0,5);
	MTevalForTopDeriv(pcurve_trans,0,6);
	MTevalForTopDeriv(pcurve_trans,0,7);
	MTevalForTopDeriv(pcurve_trans,0,8);
	MTcopyVars(pcurve_trans);
	ptr = MevalTopDerivCB(main_mul,0,3,pcurve_cb);
	vec6[0] = vec5[0] * vec5[0] * *ptr;
	vec6[1] = vec5[0] * vec5[0] * *(ptr+1);
	vec6[2] = vec5[0] * vec5[0] * *(ptr+2);
	ptr = MevalTopDerivCB(main_mul,0,4,pcurve_cb);
	vec6[0] += 2 * vec5[0] * vec5[1] * *ptr;
	vec6[1] += 2 * vec5[0] * vec5[1] * *(ptr+1);
	vec6[2] += 2 * vec5[0] * vec5[1] * *(ptr+2);
	ptr = MevalTopDerivCB(main_mul,0,5,pcurve_cb);
	vec6[0] += 2 * vec5[0] * vec5[2] * *ptr;
	vec6[1] += 2 * vec5[0] * vec5[2] * *(ptr+1);
	vec6[2] += 2 * vec5[0] * vec5[2] * *(ptr+2);
	ptr = MevalTopDerivCB(main_mul,0,6,pcurve_cb);
	vec6[0] += vec5[1] * vec5[1] * *ptr;
	vec6[1] += vec5[1] * vec5[1] * *(ptr+1);
	vec6[2] += vec5[1] * vec5[1] * *(ptr+2);
	ptr = MevalTopDerivCB(main_mul,0,7,pcurve_cb);
	vec6[0] += 2 * vec5[1] * vec5[2] * *ptr;
	vec6[1] += 2 * vec5[1] * vec5[2] * *(ptr+1);
	vec6[2] += 2 * vec5[1] * vec5[2] * *(ptr+2);
	ptr = MevalTopDerivCB(main_mul,0,8,pcurve_cb);
	vec6[0] += vec5[2] * vec5[2] * *ptr;
	vec6[1] += vec5[2] * vec5[2] * *(ptr+1);
	vec6[2] += vec5[2] * vec5[2] * *(ptr+2);
	}

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
		if( norm->x != norm->x ) norm->x = 0.0;
		if( norm->y != norm->y ) norm->y = 0.0;
		if( norm->z != norm->z ) norm->z = 0.0;
	}
	else
	{
		norm->x = norm->y = norm->z = 0.0;
	}
}

/*
 * Function:	transform
 * Action;	read in data from file and output it
 */

void transform_psurf(char *filename);
void transform_pcurve(char *filename);

void transform(char *filename)
{
	FILE *fp;
	char	def_name[256];
#ifndef NO_GEOM
	FILE *fq;
	int 	i,c;
	char    comment_file_name[L_tmpnam];
	Transform Ident;
	Geom	*geom,*in_geom,*load_geom();
	CommentList	*defCom,*timeCom;
#endif

	def_name[0] = '\0';

	if( main_mul->error )
	{
		fprintf(stderr,"Can't perform mapping - bad equations\n");
		return;
	}
 
	if( global_mode == MODE_PSURF )
	{
		transform_psurf(filename);
		return;
	}
	if( global_mode == MODE_PCURVE )
	{
		transform_psurf(filename);
		return;
	}
#ifndef NO_GEOM
	TmIdentity(Ident);
#endif
	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	global_normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	clipmax = (double) Mget_opt_val_by_name(main_mul,CLIP_NAME);

	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}

#ifdef CGIVRML
	if(vrml_version==3)
	{
		if(test_xml_errors(jvx))
		{
                	report_error2(HEAD_ERROR,"Parse error in JVX: %s",
				get_first_xml_error(jvx),2);
			print_xml_errors(stdout,jvx);
	                exit(1);
		}
		printf("Content-type: text/plain\n\n");

		JvxMap(jvx);

		printf("OK Surface sucessfully calculated\n");

		print_jvx_header("Mapping","Result of applying a mapping");
		
		copy_def(fp,stdout);
		fprint_Mopts(stdout,main_mul);
		fclose(fp);

		print_jvx_header2("Mapping");

		print_jvx_subtree(stdout,jvx,"geometries",0);
		print_jvx_tail();
	}
#else

	in_geom = load_geom();

	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't perform mapping\n");
		fclose(fp);
		return;
	}

	start_geom();
	printf("LIST\n");
	printf("COMMENT mapping %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT mapping %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	/* Now the data */

	geom = GeomSimp3(in_geom);
	GeomFree(in_geom);
	GeomMap(geom);
	GeomClip(geom,clip_fun);
	GeomFSave(geom,stdout,NULL);
	GeomFree(geom);
	fini_geom();
#endif

}

/* As above this time for operators */

void transform_psurf(char *filename)
{
	FILE *fp;
	int res;
#ifndef NO_GEOM
	FILE *fq;
	Transform Ident;
	Geom	*geom,*in_geom,*psurf_geom,*load_geom(),*load_geom2();

	int	new_edit_time;
	CommentList	*defCom,*timeCom;
	int 	i,c,res;
	char	def_name[256];
#endif
	char    comment_file_name[L_tmpnam];

	if( main_mul->error )
	{
		fprintf(stderr,"Can't perform mapping - bad equations\n");
		return;
	}


	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	global_normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	clipmax = (double) Mget_opt_val_by_name(main_mul,CLIP_NAME);

	/* check options are allowable */

	if(global_normals == EQN_NORM )
	{
		fprintf(stderr,"Can't perform mapping with operators and equations for normals\n");
		return;
	}
	if(global_dim != THREE_D_EQN )
	{
		fprintf(stderr,"Equation must be 3D -> 3D when using operators\n");
		return;
	}

#ifndef NO_GEOM
	def_name[0] = '\0';
	/* Now read in the opperator */

	psurf_geom = load_geom2(psurf_tgt_name);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	defCom = GeomCommentGet(psurf_geom,LSMP_DEF_NAME);

	if( defCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
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
		psurf_edit_time = 0; /* force re reading of pcurve */
		return;
	}
#endif
#ifdef CGIVRML
	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",auxGeomDef);
	fclose(fp);
	fp = fopen(comment_file_name,"r");
	res = 0;
	if(global_mode == MODE_PSURF)
	{
		res = read_psurf_def(fp,psurf_mul);
	}
	else if(global_mode == MODE_PCURVE)
	{
		res = read_pcurve_def(fp,pcurve_mul);
	}
	if( !res ) 
	{
		return;
	}

	/* have sucessfully read in the comment */


	/* Now the data */

	if(vrml_version==3)
	{
		if(test_xml_errors(jvx))
		{
                	report_error2(HEAD_ERROR,"Parse error in JVX: %s",
				get_first_xml_error(jvx),2);
			print_xml_errors(stdout,jvx);
	                exit(1);
		}
		printf("Content-type: text/plain\n\n");

dump_multi(stderr,main_mul);
dump_multi(stderr,psurf_mul);
dumpMTrans(stderr,psurf_trans);

		JvxMap(jvx);
/*		JvxClip(jvx,clip_fun);		*/

		printf("OK Surface sucessfully calculated\n");

		print_jvx_header("Mapping","Result of applying a mapping");
		
		fp = fopen(filename,"r");
		copy_def(fp,stdout);		
		fprint_Mopts(stdout,main_mul);
		fclose(fp);

		print_jvx_header2("Mapping");

		print_jvx_subtree(stdout,jvx,"geometries",0);
		print_jvx_tail();
	}
#else
/*
fprintf(stderr,"Now read in the data\n");
*/
	in_geom = load_geom();
	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't perform mapping\n");
		return;
	}
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
/*
fprintf(stderr,"Now create the geom\n");
*/
	start_geom();
	printf("LIST\n");
	printf("COMMENT mapping %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	printf("}\n");
	fclose(fp);
	printf("COMMENT mapping %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* finally we can start computation */

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	geom = GeomSimp3(in_geom);
	GeomFree(in_geom);
	GeomMap(geom);
	GeomClip(geom,clip_fun);
	GeomFSave(geom,stdout,NULL);
	GeomFree(geom);
	fini_geom();
#endif
}

/* As above this time for operators */

void transform_pcurve(char *filename)
{
#ifndef NO_GEOM
	FILE *fp,*fq;
	Transform Ident;
	Geom	*geom,*in_geom,*pcurve_geom,*load_geom(),*load_geom2();
	int 	i,c,res;
	char    comment_file_name[L_tmpnam];
	char	def_name[256];
	int	new_edit_time;
	CommentList	*defCom,*timeCom;

/*
fprintf(stderr,"Transform pcurve %d\n",main_mul->error);
*/
	if( main_mul->error )
	{
		fprintf(stderr,"Can't perform mapping - bad equations\n");
		return;
	}

	def_name[0] = '\0';

	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	global_normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
	global_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	clipmax = (double) Mget_opt_val_by_name(main_mul,CLIP_NAME);

	/* check options are allowable */

	if(global_normals == EQN_NORM )
	{
		fprintf(stderr,"Can't perform mapping with operators and equations for normals\n");
		return;
	}
	if(global_dim != THREE_D_EQN )
	{
		fprintf(stderr,"Equation must be 3D -> 3D when using operators\n");
		return;
	}

	/* Now read in the opperator */

	pcurve_geom = load_geom2(pcurve_tgt_name);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	defCom = GeomCommentGet(pcurve_geom,LSMP_DEF_NAME);

	if( defCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(pcurve_geom);
		return;
	}
	else if( defCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(pcurve_geom);
		return;
	}
	else if( defCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(pcurve_geom);
		return;
	}

	timeCom = GeomCommentGet(pcurve_geom,LSMP_EDIT_TIME_NAME);

	if( timeCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(pcurve_geom);
		return;
	}
	else if( timeCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(pcurve_geom);
		return;
	}
	else if( timeCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(pcurve_geom);
		return;
	}

	new_edit_time = atoi(timeCom->com->data);
	if( new_edit_time == 0 )
	{
		fprintf(stderr,"Bad timestamp data <%s>\n",timeCom->com->data);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(pcurve_geom);
		return;
	}
/*
fprintf(stderr,"pcurve Equation is\n%s\n",defCom->com->data);
*/
	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",defCom->com->data);
	fclose(fp);
	fp = fopen(comment_file_name,"r");

	if( new_edit_time != pcurve_edit_time )
	{
		pcurve_edit_time = new_edit_time; 
		res = read_pcurve_def(fp,pcurve_mul);
	} else
	{
		res = fscanMultiParams(fp,pcurve_mul);
	}
	fclose(fp);
	unlink(comment_file_name);

	CommentListFree(timeCom);
	CommentListFree(defCom);
	GeomFree(pcurve_geom);

	if( !res ) 
	{
		pcurve_edit_time = 0; /* force re reading of pcurve */
		return;
	}

	/* have sucessfully read in the comment */


	/* Now the data */

/*
fprintf(stderr,"Now read in the data\n");
*/
	in_geom = load_geom();
	if( in_geom == NULL )
	{
		fprintf(stderr,"Bad or empty geometry, can't perform mapping\n");
		return;
	}
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
/*
fprintf(stderr,"Now create the geom\n");
*/
	start_geom();
	printf("LIST\n");
	printf("COMMENT mapping %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	printf("}\n");
	fclose(fp);
	printf("COMMENT mapping %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* finally we can start computation */

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	geom = GeomSimp3(in_geom);
	GeomFree(in_geom);
	GeomMap(geom);
	GeomClip(geom,clip_fun);
	GeomFSave(geom,stdout,NULL);
	GeomFree(geom);
	fini_geom();
#endif
}

/********* file handeling ***********************/

/*
 * Function:	read_def
 * Action:	read in a new equation from 'filename',
 *		create a new geometry, set the default values etc..
 */

int read_def(FILE *fp)
{
	int     dimension,normals,colours,norm_eqns,col_eqns;
	char	str[100];

	edit_time = 0;
/*
fprintf(stderr,"Global mode is %d\n", global_mode);
*/
	Mset_opt_val_by_name(main_mul,MODE_NAME,(double) global_mode);
	if( !fscanMulti(fp,main_mul) ) return(FALSE);
	if( !MfindOpts(main_mul) ) return(FALSE);

	dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	if(colours == OLD_EQN_COL ) 
	{
		colours = EQN_COL;
		Mset_opt_val_by_name(main_mul,COL_NAME,(double) colours);
	}
	if( colours == OLD_NO_COL )
	{
		colours = NO_COL;
		Mset_opt_val_by_name(main_mul,COL_NAME,(double) colours);
	}
		
	col_eqns = ( colours == EQN_COL );
	normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
	global_mode = (int) Mget_opt_val_by_name(main_mul,MODE_NAME);
	norm_eqns = ( normals == EQN_NORM );
	if( norm_eqns && col_eqns )
	{
		MsetNtop(main_mul,3);
		MsetTopDim(main_mul,1,4); /* cols before norm */
		MsetTopDim(main_mul,2,3);
		MsetNtopDerivs(main_mul,1,0);
		MsetNtopDerivs(main_mul,2,0);
	}
	else if( norm_eqns )
	{
		MsetNtop(main_mul,2);
		MsetTopDim(main_mul,1,3);
		MsetNtopDerivs(main_mul,1,0);
	}
	else if( col_eqns )
	{
		MsetNtop(main_mul,2);
		MsetTopDim(main_mul,1,4);
		MsetNtopDerivs(main_mul,1,0);
		}
	else
		MsetNtop(main_mul,1);

	if( dimension == FOUR_D_EQN || dimension == THREE_FOUR_D_EQN )
		MsetTopDim(main_mul,0,4);
	else
		MsetTopDim(main_mul,0,3);

	if( !McombineTop(main_mul) ) return(FALSE);

	if( global_mode == MODE_PSURF || global_mode == MODE_PCURVE)
	{
		if(col_eqns)
			MsetNvars(main_mul,9);       /* i.e. ?1,?2,?3 : ?4,?5 : ?6,?7,?8,?9 */
		else
			MsetNvars(main_mul,5);       /* i.e. ?1,?2,?3 : ?4,?5 */

       		Mset_var_name(main_mul,0,"?1");
		Mset_var_name(main_mul,1,"?2");
		Mset_var_name(main_mul,2,"?3");
		Mset_var_name(main_mul,3,"?4");
		Mset_var_name(main_mul,4,"?5");

		if(col_eqns)
		{
       			Mset_var_name(main_mul,5,"?6");
			Mset_var_name(main_mul,6,"?7");
			Mset_var_name(main_mul,7,"?8");
			Mset_var_name(main_mul,8,"?9");
		}
	}
	else
	{
		int offset=0,n_vars=3;

		if( dimension == FOUR_D_EQN || dimension == FOUR_THREE_D_EQN )
			n_vars = 4;
		if( col_eqns )
			n_vars += 4;
		if( norm_eqns )
			n_vars += 3;
		MsetNvars(main_mul,n_vars);
       		Mset_var_name(main_mul,offset++,"x");
		Mset_var_name(main_mul,offset++,"y");
		Mset_var_name(main_mul,offset++,"z");
		if( dimension == FOUR_D_EQN || dimension == FOUR_THREE_D_EQN )
			Mset_var_name(main_mul,offset++,"w");
		if( col_eqns )
		{
			Mset_var_name(main_mul,offset++,"r");
			Mset_var_name(main_mul,offset++,"g");
			Mset_var_name(main_mul,offset++,"b");
			Mset_var_name(main_mul,offset++,"a");
		}
		if( norm_eqns )
		{
			Mset_var_name(main_mul,offset++,"l");
			Mset_var_name(main_mul,offset++,"m");
			Mset_var_name(main_mul,offset++,"n");
		}
	}

	if( !MfindNames(main_mul) ) return(FALSE);
	
	if( global_mode == MODE_PSURF  || global_mode == MODE_PCURVE )
	{
		/* Derivative compleatly different for psurf mode */
		/* we want to differentiate the top wrt to ?4 and ?5 */
		/* not sure what to do about limits yet though */

		if( normals == STD_NORM || normals == DISC_NORM )
		{
			MsetNtopDerivs(main_mul,0,2);
			MsetDerivName(main_mul,0,0,Mget_var_name(main_mul,3));
			MsetDerivName(main_mul,0,1,Mget_var_name(main_mul,4));
		}
		else MsetNtopDerivs(main_mul,0,0);
	}
	else
	{
		
	switch( normals )
	{
	case STD_NORM: /* we'll need derivatives */
		MsetNtopDerivs(main_mul,0,9);
		MsetDerivName(main_mul,0,0,Mget_var_name(main_mul,0));
		MsetDerivName(main_mul,0,1,Mget_var_name(main_mul,1));
		MsetDerivName(main_mul,0,2,Mget_var_name(main_mul,2));
		/* df^2/dxdx */
		strcpy(str,Mget_var_name(main_mul,0));
		strcat(str,"@");
		strcat(str,Mget_var_name(main_mul,0));
		MsetDerivName(main_mul,0,3,str);
		/* df^2/dxdy */
		strcpy(str,Mget_var_name(main_mul,0));
		strcat(str,"@");
		strcat(str,Mget_var_name(main_mul,1));
		MsetDerivName(main_mul,0,4,str);
		/* df^2/dxdz */
		strcpy(str,Mget_var_name(main_mul,0));
		strcat(str,"@");
		strcat(str,Mget_var_name(main_mul,2));
		MsetDerivName(main_mul,0,5,str);
		/* df^2/dydy */
		strcpy(str,Mget_var_name(main_mul,1));
		strcat(str,"@");
		strcat(str,Mget_var_name(main_mul,1));
		MsetDerivName(main_mul,0,6,str);
		/* df^2/dydz */
		strcpy(str,Mget_var_name(main_mul,1));
		strcat(str,"@");
		strcat(str,Mget_var_name(main_mul,2));
		MsetDerivName(main_mul,0,7,str);
		/* df^2/dzdz */
		strcpy(str,Mget_var_name(main_mul,2));
		strcat(str,"@");
		strcat(str,Mget_var_name(main_mul,2));
		MsetDerivName(main_mul,0,8,str);
		break;

	case DISC_NORM: /* we'll need derivatives */
		MsetNtopDerivs(main_mul,0,3);
		MsetDerivName(main_mul,0,0,Mget_var_name(main_mul,0));
		MsetDerivName(main_mul,0,1,Mget_var_name(main_mul,1));
		MsetDerivName(main_mul,0,2,Mget_var_name(main_mul,1));
		break;
	default:
		MsetNtopDerivs(main_mul,0,0);
	}
	} /* endif global_mode */

	if( !McheckDims(main_mul) ) return(FALSE);
	if( !McalcDerivs(main_mul) ) return(FALSE);
	if( !McalcRPEs(main_mul) ) return(FALSE);
/*
	dump_multi(stderr,main_mul);
*/
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
	Mset_opt_val_by_name(main_mul,NORM_NAME,(double) STD_NORM);
	Mset_opt_val_by_name(main_mul,MODE_NAME,(double) MODE_SIMPLE);
	global_mode = MODE_SIMPLE;
	i=read_def(fp);
	return(i);
}

int read_psurf_def(FILE *fp,Multi *psurf_mul)
{
	int     dimension,normals,colours,norm_eqns,col_eqns;

	Mclear(psurf_mul);
	if( !fscanMulti(fp,psurf_mul) ) return(FALSE);
	if( !MfindOpts(psurf_mul) ) return(FALSE);

	dimension = (int) Mget_opt_val_by_name(psurf_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(psurf_mul,COL_NAME);
	col_eqns = ( colours == EQN_COL );
	normals = (int) Mget_opt_val_by_name(psurf_mul,NORM_NAME);
	norm_eqns = ( normals == EQN_NORM );

	if(colours == EQN_COL )
	{
		fprintf(stderr,"read_psurf_def: Can't perform mapping with operators and equations for colours in ingredient\n");
		return(FALSE);
	}
	if(normals == EQN_NORM )
	{
		fprintf(stderr,"Can't perform mapping with operators and equations for normals\n");
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

	if( !McheckDims(psurf_mul) ) return(FALSE);

	MTdefine(psurf_trans,main_mul,psurf_mul);
	MTsetVarTop(psurf_trans,0,0,0);
	MTsetVarTop(psurf_trans,1,0,1);
	MTsetVarTop(psurf_trans,2,0,2);
	MTsetVarVar(psurf_trans,3,0);
	MTsetVarVar(psurf_trans,4,1);
	if(global_cols == EQN_COL)
	{
		MTsetVarInput(psurf_trans,5);
		MTsetVarInput(psurf_trans,6);
		MTsetVarInput(psurf_trans,7);
		MTsetVarInput(psurf_trans,8);
	}
	MTcalcVarTrans(psurf_trans);
	if( !MTcheck(psurf_trans) ) return(FALSE);

	if( !McalcDerivs(psurf_mul) ) return(FALSE);
	if( !McalcRPEs(psurf_mul) ) return(FALSE);

	return(TRUE);
}

int read_pcurve_def(FILE *fp,Multi *pcurve_mul)
{
	int     dimension,normals,colours,norm_eqns,col_eqns;

	Mclear(pcurve_mul);
	if( !fscanMulti(fp,pcurve_mul) ) return(FALSE);
	if( !MfindOpts(pcurve_mul) ) return(FALSE);

	dimension = (int) Mget_opt_val_by_name(pcurve_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(pcurve_mul,COL_NAME);
	col_eqns = ( colours == EQN_COL );
	norm_eqns = ( normals == EQN_NORM );

	if(colours == EQN_COL )
	{
		fprintf(stderr,"read_pcurve_def: Can't perform mapping with operators and equations for colours\n");
		return(FALSE);
	}
	if(dimension != THREE_D_EQN )
	{
		fprintf(stderr,"Pcurve must be 3D when using operators\n");
		return(FALSE);
	}
	MsetNtop(pcurve_mul,1);
	MsetTopDim(pcurve_mul,0,3);

	if( !McombineTop(pcurve_mul) ) return(FALSE);

	MsetNvars(pcurve_mul,1);
	Mset_var_name(pcurve_mul,0,"x");
/*
dump_multi(stderr,main_mul);
dump_multi(stderr,pcurve_mul);
*/
	if( !MfindNames(pcurve_mul) ) return(FALSE);

	if( !McheckDims(pcurve_mul) ) return(FALSE);

	MTdefine(pcurve_trans,main_mul,pcurve_mul);
	MTsetVarTop(pcurve_trans,0,0,0);
	MTsetVarTop(pcurve_trans,1,0,1);
	MTsetVarTop(pcurve_trans,2,0,2);

	MTsetVarVar(pcurve_trans,3,0);
	MTsetVarIgnore(pcurve_trans,4);

	if(global_cols == EQN_COL)
	{
		MTsetVarInput(psurf_trans,5);
		MTsetVarInput(psurf_trans,6);
		MTsetVarInput(psurf_trans,7);
		MTsetVarInput(psurf_trans,8);
	}
	MTcalcVarTrans(pcurve_trans);
	if( !MTcheck(pcurve_trans) ) return(FALSE);
/*
dumpMTrans(stderr,pcurve_trans);
*/
	if( !McalcDerivs(pcurve_mul) ) return(FALSE);
	if( !McalcRPEs(pcurve_mul) ) return(FALSE);

	return(TRUE);
}

/*
 * When we change the number of variables say by adding colour
 * we need to re-do the variable names etc
 */

void change_n_vars(int new_norm,int new_col,int new_dim)
{
	int offset = 0,n_vars;
	int old_col,old_norm,old_dim;
	char *Xname,*Yname,*Zname,*Wname, *Lname,*Mname,*Nname;
	char *Rname,*Gname,*Bname,*Aname;

	/* Don't want to mess about for operators */

	if( global_mode == MODE_PSURF ) return;
	if( global_mode == MODE_PCURVE ) return;

	/* first gothrough existing names in the multi */

	old_col = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	old_norm = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
	old_dim = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	Xname = strdup(Mget_var_name(main_mul,offset++));
	Yname = strdup(Mget_var_name(main_mul,offset++));
	Zname = strdup(Mget_var_name(main_mul,offset++));
	if( old_dim == FOUR_D_EQN || old_dim == FOUR_THREE_D_EQN )
		Wname = strdup(Mget_var_name(main_mul,offset++));
	else
		Wname = "w";

	if( old_col == EQN_COL )
	{
		Rname = strdup(Mget_var_name(main_mul,offset++));
		Gname = strdup(Mget_var_name(main_mul,offset++));
		Bname = strdup(Mget_var_name(main_mul,offset++));
		Aname = strdup(Mget_var_name(main_mul,offset++));
	}
	else
	{
		Rname = "r"; Gname = "g"; Bname = "b"; Aname = "a";
	}

	if(old_norm == EQN_NORM )
	{
		Lname = strdup(Mget_var_name(main_mul,offset++));
		Mname = strdup(Mget_var_name(main_mul,offset++));
		Nname = strdup(Mget_var_name(main_mul,offset++));
	}
	else
	{
		Lname = "l"; Mname = "m"; Nname = "n";
	}


#ifdef NOT_DEF
	fprintf(stderr,"change_n_vars\n");
#endif

	n_vars = 3;
	if( new_dim == FOUR_D_EQN || new_dim == FOUR_THREE_D_EQN )
		++n_vars;
	if( new_col == EQN_COL )
		n_vars += 4;
	if( new_norm == EQN_NORM )
		n_vars += 3;

	MsetNvars(main_mul,n_vars);
	offset = 0;
	Mset_var_name(main_mul,offset++,Xname);
	Mset_var_name(main_mul,offset++,Yname);
	Mset_var_name(main_mul,offset++,Zname);
	if(new_dim == FOUR_D_EQN || new_dim == FOUR_THREE_D_EQN )
		Mset_var_name(main_mul,offset++,Wname);
	if(new_col == EQN_COL )
	{
		Mset_var_name(main_mul,offset++,Rname);
		Mset_var_name(main_mul,offset++,Gname);
		Mset_var_name(main_mul,offset++,Bname);
		Mset_var_name(main_mul,offset++,Aname);
	}
	if(new_col == EQN_NORM )
	{
		Mset_var_name(main_mul,offset++,Lname);
		Mset_var_name(main_mul,offset++,Mname);
		Mset_var_name(main_mul,offset++,Nname);
	}
		
		
	Mset_opt_val_by_name(main_mul,COL_NAME,(double) new_col);
	Mset_opt_val_by_name(main_mul,NORM_NAME,(double) new_norm);
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

/************* Now call back functions from tcl *************************/

#ifndef COMMAND_LINE
/*
 * Load the file
 * given the filename and the name of a tempory file for the editor
 */

int load_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp,*fq;

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
	fclose(fp);
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
 * Run the program
 */

int run_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "run_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	switch( global_mode )
	{
	case MODE_SIMPLE:
		transform(argv[1]);
		break;
	case MODE_PSURF:
		transform_psurf(argv[1]);
		break;
	case MODE_PCURVE:
		transform_pcurve(argv[1]);
		break;
	default:
		fprintf(stderr,"Bad mode %d\n",global_mode);
	}
	return TCL_OK;
}

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int  precision,normals,colour,dimension;
	double clipmax;
	colour = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	precision = (int) Mget_opt_val_by_name(main_mul,PREC_NAME);
	dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	clipmax = (double) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
 
	sprintf(interp->result,"%d %.2f %d %d %d\n",
		precision,clipmax,normals,colour,dimension);
	return TCL_OK;
}

int set_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 6)
	{
		interp->result = "set_options: Wrong number of arguments";
		return TCL_ERROR;
	}
	Mset_opt_val_by_name(main_mul,PREC_NAME, atof(argv[1]) );
	Mset_opt_val_by_name(main_mul,CLIP_NAME, atof(argv[2]) );
	change_n_vars(atof(argv[3]),atof(argv[4]),atof(argv[5]));

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
	Mset_opt_val_by_name(main_mul,MODE_NAME,(double) global_mode);
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

int get_env(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	char    *tcl_dir, *examples_dir;
	tcl_dir = getenv("LSMP_TCL");
	examples_dir = getenv("LSMP_EXAMPLES");
	sprintf(interp->result,"%s %s",tcl_dir,examples_dir);
	return(TCL_OK);
}

int get_progname(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	sprintf(interp->result,"%s",prog_name);
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
 * Sets the target name
 */

int target_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "target_cb: Wrong number of arguments";
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

int pcurve_target_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "pcurve_target_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	strcpy(pcurve_tgt_name,argv[1]);
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

#ifndef COMMAND_LINE
	char	tcl_file[128],*tcl_dir;
	int	i;
        FILE *fo;
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
        freopen("asurf.error","w",stderr);
#endif
	/* read in arguments */

	read_lsmp_xml();
	print_time_message("MappingCV:");

	main_mul = grballoc(Multi);
	Minit(main_mul);
	psurf_mul = grballoc(Multi);
	Minit(psurf_mul);
	psurf_trans = grballoc(MTrans);
	pcurve_mul = grballoc(Multi);
	Minit(pcurve_mul);
	pcurve_trans = grballoc(MTrans);

#ifdef CGIVRML
	print_time_message("MappingCV: done read_cgi");
#else
	map3_args(argc,argv);
#endif
	init_funs();

	/* Initilise the point list program */

#ifdef NOT_DEF
	pointlist_init();
	Simp_init();
	Map_init();
	Clip_init();
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
		fclose(fp);
		use_arg_vals();
	print_time_message("MappingCV: start transform");
		transform(arg_filename);
		if( temp_flag ) unlink(temp_file_name);
	print_time_message("MappingCV: done");
		exit(0);
	}
#ifndef CGIVRML
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
	wind = Tk_CreateMainWindow(interp,NULL,prog_name, "mapping");
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

	Tcl_CreateCommand(interp,"get_options",get_options,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"set_options",set_options,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_mode",get_mode,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"set_mode",set_mode,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_env",get_env,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_num_params",get_num_params,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"get_param",get_param,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"set_param",set_param,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"object_cb",object_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"target_cb",target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"psurf_target_cb",psurf_target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"pcurve_target_cb",pcurve_target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_targets_cb",get_targets_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	use_arg_vals();

	strcpy(tcl_file,tcl_dir);
	strcat(tcl_file,"/mapping.tcl");
	code = Tcl_EvalFile(interp,tcl_file);
	if (code != TCL_OK) {
		fprintf(stderr,"%s\n",interp->result);
		exit(-1);
	}

	Tk_MainLoop();

	if( temp_flag ) unlink(temp_file_name);
#endif
	return 0;
}

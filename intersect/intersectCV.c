/*
 *      file:   main.c:   
 *      author: Rich Morris
 *      date:   Jan 3rd 1995
 *      
 *	performs clip on geomview objects defined by a set
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
#include "normlist.h"
#include "geomsimp.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifndef CGIVRML
#include <sys/wait.h>
#include <point.h>
#endif

#ifndef CGIVRML
#include <tcl.h>
#include <tk.h>
#include <unistd.h>
#endif

#ifndef NO_GEOM
#include "extractcomment.h"
#endif

#include <eqn.h>
#include <Multi.h>
#include <MTrans.h>
#include "../CVcommon.h"
#include "../lsmp.h"
#include "../jvx/jvx.h"
#include "../jvx/jvxCore.h"
#include "intersect.h"

extern void JvxMap(xml_tree *root);

/*
#define PRINT_CALC
*/
#define NON_EIGEN

/*
#define USE_SPECIAL 1
*/

/* Default values */

#define TOL_DEFAULT 1.0e-7
#define ITT_DEFAULT 20 

#define TGT_NM_SZ 1024

#define BOX 1
#define SPHERE 2
#define PLANE 3

#define grballoc(node) (node *) malloc( sizeof(node) )

enum	{Clip, Bad} prog_type;
Multi   *main_mul,*psurf_mul,*pcurve_mul,*icurve_mul,*impsurf_mul;
MTrans  *psurf_trans,*pcurve_trans,*icurve_psurf_trans,*impsurf_trans;

char	*arg_filename = NULL;
double arg_vals[MAX_NUM_PARAMS + MAX_NUM_VARS];
char   *arg_names[MAX_NUM_PARAMS + MAX_NUM_VARS];
int     arg_count=0;
char	temp_file_name[80];
char	temp_flag = FALSE;
char	prog_name[10];	/* The name used to call the prog by */
char    tgt_names[TGT_NM_SZ];   /* array to hold list of target names */

eqn_funs *funlist = NULL;

/* default values */

int	world = FALSE;			/* Transform relative to world */
int	command = FALSE;		/* Write geomview comands */
int	quiet = TRUE;			/* quite mode */
int	intersect_flag = TRUE;		/* Perform intersection instead */
int	max_itt = 20;			/* Max number of itterations */
double	tolerance;			/* Tolerance for cliping */
int	special_obj = 0;		/* Which special object */
int	box_num = 0;			/* Number for box */
int	sphere_num = 0;			/* Number for sphere */
int	plane_num = 0;			/* Number for plane */
char	boxhandle[20],spherehandle[20],planehandle[20];
double	intersect_sign = 1.0;		/* +ve or -ve for clipping */

/* stuff for operators */

int     global_mode = 0;	/* 0 = simple 1 = operator on a psurf */
int     global_orient = ORIENT_ORIENT;	/* 0 = Orient 1 = Unorient 
					2,3 major/minor eigen */
int	global_cols = 0;	/* Used with mapping */
int	global_normals = 0;	/* Used with mapping */
int	global_dim = 3;		/* Used with mapping */
float   clipmax;			/* clipping */
int     edit_time = 0;	  /* time this equation was last edited */
int     psurf_edit_time = 0;    /* time psurf was last edited */
char    psurf_tgt_name[64];     /* The target name for the psurf ingr */
char    pcurve_tgt_name[64];     /* The target name for the psurf ingr */
int     pcurve_edit_time = 0;    /* time psurf was last edited */
int     impsurf_edit_time = 0;    /* time impsurf was last edited */
char    impsurf_tgt_name[64];     /* The target name for the impsurf ingr */
int     icurve_edit_time = 0;    /* time icurve was last edited */
char    icurve_tgt_name[64];     /* The target name for the icurve ingr */
xml_tree *jvx;
int     vrml_version = 1; /* the version of VRML produced */
char	*auxGeomDef;		/* The def for the auxillary geom */
char	*auxGeomDef2;		/* The def for the 2nd auxillary geom */

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

#define  CLIP(X) (X > clipmax ? clipmax :( X < -clipmax ? -clipmax : ( X != X ? clipmax : X) ))

/**** agrument handeling *********************************************/

read_lsmp_xml()
{
	LsmpInputSpec *spec;
	LsmpDef *def;
        LsmpOption *tolerance_ent,*iterations_ent,*colors_ent,*inttype_ent,*opType_ent,*version_ent;
        FILE    *temp_file,*fp,*fl;
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

	inttype_ent = getLsmpOption(spec->Def,"inttype");
	if(inttype_ent != NULL)
	{
		if(!strcmp(inttype_ent->value,"0"))
		{	intersect_flag = TRUE; intersect_sign = 1.0; }
		else if(!strcmp(inttype_ent->value,"1")
	        ||	!strcmp(inttype_ent->value,"+1"))
		{	intersect_flag = FALSE; intersect_sign = 1.0; }
		else if(!strcmp(inttype_ent->value,"-1"))
		{	intersect_flag = FALSE; intersect_sign = -1.0; }
		else
		{
			report_error2(HEAD_ERROR,"Bad type of intersection %s",inttype_ent->value,101);
			exit(1);
		}
	}
	else
		{	intersect_flag = TRUE; intersect_sign = 1.0; }

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

	tolerance_ent = getLsmpOption(spec->Def,"tolerance");
	if(tolerance_ent!=NULL)
	{
		fprintf(stderr,"_%s = %s;\n",TOL_NAME,tolerance_ent->value);
		fprintf(temp_file,"_%s = %s;\n",TOL_NAME,tolerance_ent->value);
	}
	iterations_ent = getLsmpOption(spec->Def,"iterations");
	if(iterations_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %s;\n",ITT_NAME,iterations_ent->value);
	}
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
	if(spec->auxDef2 != NULL)
	{
		char *c1,*c2;
		c1 = strstr(spec->auxDef2->data,"<p>");	
		c2 = strstr(spec->auxDef2->data,"</p>");
		if(c1!=NULL) spec->auxDef2->data = c1+3;
		if(c2!=NULL) *c2 = '\0';
		auxGeomDef2 = strdup(spec->auxDef2->data);
	}
	else
		auxGeomDef2 = NULL;

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
		else if(!strcmp(spec->Def->opType,"impsurf"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_IMPSURF);
			global_mode = MODE_IMPSURF;
		}
		else if(!strcmp(spec->Def->opType,"psurf icurve"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_ICV_PSURF);
			global_mode = MODE_ICV_PSURF;
		}
		else if(!strcmp(spec->Def->opType,"psurf project"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_PSURF_PROJ);
			global_mode = MODE_PSURF_PROJ;
		}
		else if(!strcmp(spec->Def->opType,"psurf icurve proj"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_ICV_PSURF_PROJ);
			global_mode = MODE_ICV_PSURF_PROJ;
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


int     arg_precision = -1;
double	arg_tolerance = 0.0;
int	arg_itterations = 0;
int	arg_colour = -5;

map3_args(argc,argv)
int argc; char **argv;
{
#ifndef CGIVRML
	int i;
	extern char *optarg;
	extern  int  optind;
	FILE	*temp_file;
	char    *slash;

	/** we strip the first argument off **/

	--argc; ++argv;

	/** first find the name prog was called by **/

	slash = strrchr(argv[0],'/');
	if(slash == NULL) strcpy(prog_name,argv[0]);
	else	      strcpy(prog_name,slash+1);
	if( !strcmp(prog_name,"intersect") )
	{	
		prog_type = Clip;
	}
	else
	{
		fprintf(stderr,"bad program name: %s\n",prog_name);
		exit(-1);
	}
/*
printf("argc %d\n",argc);
for(i=0;i<argc;++i) printf("argv[i] %s\n",argv[i]);
*/
	while((i=getopt(argc,argv,"hGISPBCNe:i:t:c:p:D:")) != -1 )
	{
		switch(i)
		{
		case 'C': intersect_flag = FALSE; intersect_sign = -1.0; break;
		case 'N': intersect_flag = FALSE; intersect_sign = 1.0; break;
		case 't': arg_tolerance = atof(optarg); break;
		case 'i': arg_itterations = atof(optarg); break;
		case 'p': arg_precision = atoi(optarg); break;
		case 'c': arg_colour = atoi(optarg); break;
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

		case 'B':	/* Box */
			set_special(BOX);
			arg_filename = tmpnam(temp_file_name);
			temp_file = fopen(arg_filename,"w");
			fprintf(temp_file,"if( R - reject, 0/0, val);");
			fprintf(temp_file,"R = x^2 + y^2 + z^2;");
			fprintf(temp_file,"val = min(min(min(xh-x,x-xl),min(yh-y,y-yl)),min(zh-z,z-zl));\n");
			fprintf(temp_file,"xh = 1.0; xl = -1.0;\n");
			fprintf(temp_file,"yh = 1.0; yl = -1.0;\n");
			fprintf(temp_file,"zh = 1.0; zl = -1.0;\n");
			fprintf(temp_file,"reject=100;\n");
			fclose(temp_file);
			temp_flag = TRUE;
			break;
		case 'S':	/* Sphere */
			set_special(SPHERE);
			arg_filename = tmpnam(temp_file_name);
			temp_file = fopen(arg_filename,"w");
			fprintf(temp_file,
				"if( R - reject^2, 0/0, -R);\n");
			fprintf(temp_file,
				"R =  (x-x0)^2 + (y-y0)^2 + (z-z0)^2 - r^2;");
			fprintf(temp_file,"r=1;\n");
			fprintf(temp_file,"reject=100;\n");
			fclose(temp_file);
			temp_flag = TRUE;
			break;
		case 'P':	/* Plane */
			set_special(SPHERE);
			arg_filename = tmpnam(temp_file_name);
			temp_file = fopen(arg_filename,"w");
			fprintf(temp_file,"a x + b y + c z = d;\n");
			fprintf(temp_file,"a=0; b=1; c=0; d=0;\n");
			fclose(temp_file);
			temp_flag = TRUE;
			break;

	  	case 'h':
		default:
				printf("Bad option (%c) in %s \n",
					argv[optind-1]);
				print_usage();
				exit(-1);
		}
	}

	for(;optind<argc;optind++)
	{
		if( arg_filename != NULL )
		{
			fprintf(stderr,"Only one file name allowed\007\n");
			print_usage();
			exit(-1);
		}
		else
		{
			arg_filename = argv[optind];
		}
	}

	if(quiet && arg_filename == NULL )
	{
		fprintf(stderr,"A file name must be given in quiet mode\n");
		print_usage();
		exit(-1);
	}
#endif
}

print_usage()
{
	fprintf(stderr,"Usage: intersect [-C|-N] [-t tolerance] [-i itterations] [-p precision] \n");
	fprintf(stderr,"\t\t[-c colour] [-D name val] {-G|-I|-e equation|filename|-S|-P|-B}\n");
}

read_cgi()
{
        char *env_query;
        int cl,c;
        ncsa_entry entries[MAX_ENTRIES]; 
        register int x,m=0;
        char *def_ent;
        char *tolerance_ent,*iterations_ent,*colors_ent,*inttype_ent,
		*inputgeom_ent,*auxGeom_ent,*auxIcurve_ent,*opType_ent;
        char *version_ent;
        FILE    *temp_file;
        FILE *fp,*fl;
                char *tmp,*tstr; time_t tim;

        def_ent=inttype_ent=NULL;
        tolerance_ent=iterations_ent=colors_ent=inputgeom_ent=auxGeom_ent=auxIcurve_ent=opType_ent=NULL;
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
	if(strncmp(entries[x].val,"INPUTGEOM",9))
*/
	        fprintf(stderr,"%s",entries[x].val);
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
/*
	if(strncmp(entries[x].val,"INPUTGEOM",9))
*/
	        fprintf(fl,"%s\n",entries[x].val);

        entries[x].name = makeword(entries[x].val,'=');


        if(!strcmp(entries[x].name,"DEF"))          def_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"TOLERANCE"))    tolerance_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"ITERATIONS"))   iterations_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"COLORS"))  colors_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"VERSION")) version_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"INPUTGEOM")) inputgeom_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"AUXGEOM")) auxGeom_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"AUXICURVE")) auxIcurve_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"OPTYPE")) opType_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"INTTYPE")) inttype_ent = entries[x].val;
        else 
        {    
                report_error2(HEAD_ERROR,"Bad field name (%s)",entries[x].name,3);
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

	if(inttype_ent != NULL)
	{
		if(!strcmp(inttype_ent,"0"))
		{	intersect_flag = TRUE; intersect_sign = 1.0; }
		else if(!strcmp(inttype_ent,"1")
	        ||	!strcmp(inttype_ent,"+1"))
		{	intersect_flag = FALSE; intersect_sign = 1.0; }
		else if(!strcmp(inttype_ent,"-1"))
		{	intersect_flag = FALSE; intersect_sign = -1.0; }
		else
		{
			report_error2(HEAD_ERROR,"Bad type of intersection %s",inttype_ent,101);
			exit(1);
		}
	}
	else
		{	intersect_flag = TRUE; intersect_sign = 1.0; }

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
	if(tolerance_ent!=NULL)
	{
		fprintf(stderr,"_%s = %.13lf;\n",TOL_NAME,atof(tolerance_ent));
		fprintf(temp_file,"_%s = %13.9lf;\n",TOL_NAME,atof(tolerance_ent));
	}
	if(iterations_ent!=NULL)
	{
		fprintf(temp_file,"_%s = %d;\n",ITT_NAME,atoi(iterations_ent));
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
	if(auxIcurve_ent != NULL)
	{
		char *c1,*c2;
		c1 = strstr(auxIcurve_ent,"<p>");	
		c2 = strstr(auxIcurve_ent,"</p>");
		if(c1!=NULL) auxIcurve_ent = c1+3;
		if(c2!=NULL) *c2 = '\0';
		auxGeomDef2 = strdup(auxIcurve_ent);
	}
	else
		auxGeomDef2 = NULL;

	if(opType_ent != NULL)
	{
		if(!strcmp(opType_ent,"psurf"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_PSURF);
			global_mode = MODE_PSURF;
		}
		else if(!strcmp(opType_ent,"pcurve"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_PCURVE);
			global_mode = MODE_PCURVE;
		}
		else if(!strcmp(opType_ent,"impsurf"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_IMPSURF);
			global_mode = MODE_IMPSURF;
		}
		else if(!strcmp(opType_ent,"psurf icurve"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_ICV_PSURF);
			global_mode = MODE_ICV_PSURF;
		}
		else if(!strcmp(opType_ent,"psurf project"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_PSURF_PROJ);
			global_mode = MODE_PSURF_PROJ;
		}
		else if(!strcmp(opType_ent,"psurf icurve proj"))
		{
                	fprintf(temp_file,"_mode = %d;\n",MODE_ICV_PSURF_PROJ);
			global_mode = MODE_ICV_PSURF_PROJ;
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


use_arg_vals()
{
  int i,j;

  if(arg_precision != -1 ) 
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) arg_precision);
  if(arg_tolerance != 0.0 ) 
	Mset_opt_val_by_name(main_mul,TOL_NAME,(double) arg_tolerance);
  if(arg_itterations != 0 ) 
	Mset_opt_val_by_name(main_mul,ITT_NAME,(double) arg_itterations);
  if(arg_colour != -5 ) 
	Mset_opt_val_by_name(main_mul,COL_NAME,(double) arg_colour);
  for(i=0;i<arg_count;++i)
  {
	Mset_param_val_by_name(main_mul,arg_names[i],arg_vals[i]);
  }
}


/************************* Initilisation *******************************/

/*
 * Function:	init_funs
 * Action;	perform initilisation on the equation front
 */

init_funs()
{
	funlist = add_standard_functions(NULL);
	set_input_functions(funlist);
	MsetNvars(main_mul,3);
	Mset_var_name(main_mul,0,"x");
	Mset_var_name(main_mul,1,"y");
	Mset_var_name(main_mul,2,"z");
	Madd_opt(main_mul,PREC_NAME,(double) PREC_DEFAULT);
	Madd_opt(main_mul,TOL_NAME,(double) TOL_DEFAULT);
	Madd_opt(main_mul,ITT_NAME,(double) ITT_DEFAULT);
	Madd_opt(main_mul,COL_NAME,(double) STD_COL);

	Madd_opt(main_mul,MODE_NAME,(double) MODE_SIMPLE);

	MsetNvars(psurf_mul,2);
	Mset_var_name(psurf_mul,0,"x");
	Mset_var_name(psurf_mul,1,"y");
	Madd_opt(psurf_mul,PREC_NAME,(double) PREC_DEFAULT);
	Madd_opt(psurf_mul,CLIP_NAME,CLIP_DEFAULT);
	Madd_opt(psurf_mul,DIM_NAME,3.0);
	Madd_opt(psurf_mul,NORM_NAME,(double) STD_NORM);
	Madd_opt(psurf_mul,COL_NAME,(double) NO_COL);

	MsetNvars(pcurve_mul,1);
	Mset_var_name(pcurve_mul,0,"x");
	Madd_opt(pcurve_mul,PREC_NAME,(double) PREC_DEFAULT);
	Madd_opt(pcurve_mul,CLIP_NAME,CLIP_DEFAULT);
	Madd_opt(pcurve_mul,DIM_NAME,3.0);
	Madd_opt(pcurve_mul,COL_NAME,(double) NO_COL);

	MsetNvars(impsurf_mul,3);
	Mset_var_name(impsurf_mul,0,"x");
	Mset_var_name(impsurf_mul,1,"y");
	Mset_var_name(impsurf_mul,2,"z");
	Madd_opt(impsurf_mul,PREC_NAME,(double) PREC_DEFAULT);
/*
	Madd_opt(main_mul,COARSE_NAME,(double) COARSE_DEFAULT);
	Madd_opt(main_mul,FINE_NAME,(double) FINE_DEFAULT);
	Madd_opt(main_mul,FACE_NAME,(double) FACE_DEFAULT);
	Madd_opt(main_mul,EDGE_NAME,(double) EDGE_DEFAULT);
*/
	MsetNvars(icurve_mul,5);
	Mset_var_name(icurve_mul,0,"?1");
	Mset_var_name(icurve_mul,1,"?2");
	Mset_var_name(icurve_mul,2,"?3");
	Mset_var_name(icurve_mul,3,"?4");
	Mset_var_name(icurve_mul,4,"?5");
	Madd_opt(icurve_mul,DIM_NAME,3.0);
	Madd_opt(icurve_mul,ORIENT_NAME,2.0);
	Madd_opt(icurve_mul,MODE_NAME,MODE_SIMPLE);
	Madd_opt(icurve_mul,COL_NAME,NO_COL);
}

/******************** the guts of prog *****************************/

/*
 * Function:	rpe_fun
 * returns the value of the rpe
 */

double rpe_fun(x,y,z)
double x,y,z;
{
	double *ptr;

	Mset_var_val(main_mul,0,x);
	Mset_var_val(main_mul,1,y);
	Mset_var_val(main_mul,2,z);
	MstartEval(main_mul);
	ptr = MevalTop2(main_mul,0);
/*
printf("f(%f,%f,%f) = %f\n",x,y,z,*ptr);
*/
	return( intersect_sign * *ptr );
}

double rpe_fun_psurf(x,y,z)
double x,y,z;
{
	double *ptr;

	MstartEval(main_mul);
	MstartEval(psurf_mul);
	Mset_var_val(main_mul,3,x);
	Mset_var_val(main_mul,4,y);
	Mset_var_val(psurf_mul,0,x);
	Mset_var_val(psurf_mul,1,y);

	MTevalForTop(psurf_trans,0);
	MTcopyVars(psurf_trans);
	ptr = MevalTop2(main_mul,0);
/*
fprintMres(stderr,main_mul);
fprintf(stderr,"psurf\n");
fprintMres(stderr,psurf_mul);
fprintf(stderr,"x %f y %f val %f sign %f\n",x,y,*ptr,intersect_sign);
*/
	return( intersect_sign * *ptr );
}

void pcurve_cb(short prov_req[],double vals[])
{
	MTstdCallback(pcurve_trans,prov_req,vals);
}

double rpe_fun_pcurve(double x,double y,double z)
{
	double *ptr;

	MstartEval(main_mul);
	MstartEval(pcurve_mul);
	Mset_var_val(main_mul,3,x);
	Mset_var_val(main_mul,4,y);
	Mset_var_val(pcurve_mul,0,x);

	MTevalForTop(pcurve_trans,0);
	MTcopyVars(pcurve_trans);
	ptr = MevalTopCB(main_mul,0,pcurve_cb);
/*
dump_multi(stderr,main_mul);
dump_multi(stderr,pcurve_mul);
dumpMTrans(stderr,pcurve_trans);
fprintf(stderr,"pcurve\n");
fprintMres(stderr,pcurve_mul);
fprintMvals(stderr,main_mul);
fprintf(stderr,"x %f y %f val %f sign %f\n",x,y,*ptr,intersect_sign);
*/
	return( intersect_sign * *ptr );
}

double rpe_fun_impsurf(x,y,z)
double x,y,z;
{
	double *ptr;

	MstartEval(main_mul);
	MstartEval(impsurf_mul);
	Mset_var_val(main_mul,1,x);
	Mset_var_val(main_mul,2,y);
	Mset_var_val(main_mul,3,z);
	Mset_var_val(impsurf_mul,0,x);
	Mset_var_val(impsurf_mul,1,y);
	Mset_var_val(impsurf_mul,2,z);

	MTevalForTop(impsurf_trans,0);
	MTcopyVars(impsurf_trans);
	ptr = MevalTop2(main_mul,0);
	return( intersect_sign * *ptr );
}

/*
 *	calc_given_pq(x,y,p,q)
	calc_nearest_pq(x,y,p,q)
 *	calc from eigen vec equations, either the whole lot or just
	the results
 */

double calc_given_pq(double x, double y,VEC2D p,VEC2D q)
{
	double *ptr;

	MstartEval(main_mul);
	MstartEval(psurf_mul);
	Mset_var_val(main_mul,7,x);
	Mset_var_val(main_mul,8,y);
	Mset_var_val(main_mul,3,p.x);
	Mset_var_val(main_mul,4,p.y);
	Mset_var_val(main_mul,5,q.x);
	Mset_var_val(main_mul,6,q.y);
	Mset_var_val(psurf_mul,0,x);
	Mset_var_val(psurf_mul,1,y);
	MTevalForTop(psurf_trans,0);
	MTcopyVars(psurf_trans);
	ptr = MevalTop2(main_mul,0);
if( *ptr != *ptr )
fprintf(stderr,"calc_given (%f,%f) p (%f,%f) q (%f,%f) val %f\n",
		x,y,p.x,p.y,q.x,q.y,*ptr);
#ifdef PRINT_CALC
#endif
	return(*ptr);
}

double calc_nearest_pq(double x, double y,VEC2D *p,VEC2D *q)
{
	double *ptr;
	double A,B,C,D;
	double a,b,c,d;
	double det,tr,disc,lam1,lam2,len;
	double V1a_x,V1a_y,V1b_x,V1b_y,V2a_x,V2a_y,V2b_x,V2b_y;

#ifdef PRINT_CALC
if(y == 0.0 )
{
fprintf(stderr,"calc_near (%f,%f) p (%f,%f) q (%f,%f) \n",
		x,y,p->x,p->y,q->x,q->y);
}
#endif
	MstartEval(main_mul);
	MstartEval(psurf_mul);
	MstartEval(icurve_mul);
	Mset_var_val(main_mul,7,x);
	Mset_var_val(main_mul,8,y);
	Mset_var_val(psurf_mul,0,x);
	Mset_var_val(psurf_mul,1,y);
	Mset_var_val(icurve_mul,3,x);
	Mset_var_val(icurve_mul,4,y);
	MTevalForTop(icurve_psurf_trans,0);
	MTcopyVars(icurve_psurf_trans);
	ptr = MevalTop2(icurve_mul,0);
	A = *ptr; B = *(ptr+1); C = *(ptr+2); D = *(ptr+3);
	det = A * D - B * C;
	tr = A + D;
	disc = tr * tr - 4 * det;
#ifdef PRINT_CALC
fprintf(stderr,"mat (%f %f %f %f) det %f tr %f disc %f\n",
	A,B,C,D,det,tr,disc);
#endif
	if(disc < 0.0 || disc != disc) 
	{
		p->x = p->y = q->x = q->y = 0.0;
		return(0.0);
	}
	lam1 = (tr + sqrt(disc) )/ 2.0;
	lam2 = tr - lam1;
	V1a_x = - B;
	V1a_y = A - lam1;
	V1b_x = D - lam1;
	V1b_y = - C;
	V2a_x = - B;
	V2a_y = A - lam2;
	V2b_x = D - lam2;
	V2b_y = - C;
#ifdef PRINT_CALC
if( V1a_x != V1a_x || V1a_y != V1a_y || V1b_x != V1b_x || V1b_y != V1b_y
 || V2a_x != V2a_x || V2a_y != V2a_y || V2b_x != V2b_x || V2b_y != V2b_y )
{
fprintf(stderr,"calc_near (%f,%f) p (%f,%f) q (%f,%f) \n",
		x,y,p->x,p->y,q->x,q->y);
fprintf(stderr,"mat (%f %f %f %f) det %f tr %f disc %f lam1 %f lam2 %f\n",
	A,B,C,D,det,tr,disc,lam1,lam2);
}
#endif
	a = A; b = B; c = C; d = D;

	if( fabs(V1a_x)+fabs(V1a_y) > fabs(V1b_x) + fabs(V1b_y) )
	{
		A = V1a_x; B = V1a_y;
	} else {
		A = V1b_x; B = V1b_y;
	}
	if( fabs(V2a_x)+fabs(V2a_y) > fabs(V2b_x) + fabs(V2b_y) )
	{
		C = V2a_x; D = V2a_y;
	} else {
		C = V2b_x; D = V2b_y;
	}

	len = sqrt(A*A+ B*B);
#ifdef PRINT_CALC
if( len == 0.0 )
{
fprintf(stderr,"calc_near (%f,%f) p (%f,%f) q (%f,%f) \n",
		x,y,p->x,p->y,q->x,q->y);
fprintf(stderr,"mat2 (%f,%f,%f,%f) det %f tr %f disc %f lam1 %f lam2 %f\n",
	a,b,c,d,det,tr,disc,lam1,lam2);
fprintf(stderr,"ABCD %f  %f %f %f\n",A,B,C,D);
fprintf(stderr,"V1a (%f,%f) V1b (%f,%f)\n",
	V1a_x,V1a_y,V1b_x,V1b_y);
}
#endif
	A /= len; B /= len;
	len = sqrt(C*C+ D*D);
#ifdef PRINT_CALC
if( len == 0.0 )
{
fprintf(stderr,"calc_near (%f,%f) p (%f,%f) q (%f,%f) \n",
		x,y,p->x,p->y,q->x,q->y);
fprintf(stderr,"mat det %f tr %f disc %f lam1 %f lam2 %f\n",
	det,tr,disc,lam1,lam2);
fprintf(stderr,"ABCD %f  %f %f %f\n",A,B,C,D);
}
#endif
	C /= len; D /= len;

	if( p->x * A + p->y * B < 0.0 )
	{	p->x = -A; p->y = -B; }
	else
	{	p->x = A; p->y = B; }
	if( q->x * C + q->y * D < 0.0 )
	{	q->x = -C; q->y = -D;	}
	else
	{	q->x = C; q->y = D;	}

	Mset_var_val(main_mul,3,p->x);
	Mset_var_val(main_mul,4,p->y);
	Mset_var_val(main_mul,5,q->x);
	Mset_var_val(main_mul,6,q->y);
	MTevalForTop(psurf_trans,0);
	MTcopyVars(psurf_trans);
	ptr = MevalTop2(main_mul,0);
#ifdef PRINT_CALC
if( *ptr != *ptr )
{
fprintf(stderr,"calc_near (%f,%f) p (%f,%f) q (%f,%f) val %f\n",
		x,y,p->x,p->y,q->x,q->y,*ptr);
fprintMvals(stderr,main_mul);
fprintMres(stderr,main_mul);
}
#endif
	return(*ptr);
}

double calc_given_u(double x, double y,VEC3D p)
{
	double *ptr;

	MstartEval(main_mul);
	MstartEval(psurf_mul);
	Mset_var_val(main_mul,6,x);
	Mset_var_val(main_mul,7,y);
	Mset_var_val(main_mul,3,p.x);
	Mset_var_val(main_mul,4,p.y);
	Mset_var_val(main_mul,5,p.z);
	Mset_var_val(psurf_mul,0,x);
	Mset_var_val(psurf_mul,1,y);
	MTevalForTop(psurf_trans,0);
	MTcopyVars(psurf_trans);
	ptr = MevalTop2(main_mul,0);
#ifdef PRINT_CALC
fprintf(stderr,"calc_given (%f,%f) u (%f,%f,%f) val %f\n",
		x,y,p.x,p.y,p.z,*ptr);
#endif
	return(*ptr);
}

double calc_nearest_u(double x, double y,VEC3D *p)
{
	double *ptr;
	double A,B,C,D;
	double det,tr,disc,lam1,lam2,len;
	double V1a_x,V1a_y,V1b_x,V1b_y,V2a_x,V2a_y,V2b_x,V2b_y;

#ifdef PRINT_CALC
fprintf(stderr,"calc_near (%f,%f) p (%f,%f) q (%f,%f) \n",
		x,y,p->x,p->y,q->x,q->y);
#endif
	MstartEval(main_mul);
	MstartEval(psurf_mul);
	MstartEval(icurve_mul);
	Mset_var_val(main_mul,6,x);
	Mset_var_val(main_mul,7,y);
	Mset_var_val(psurf_mul,0,x);
	Mset_var_val(psurf_mul,1,y);
	Mset_var_val(icurve_mul,3,x);
	Mset_var_val(icurve_mul,4,y);
	MTevalForTop(icurve_psurf_trans,0);
	MTcopyVars(icurve_psurf_trans);
	ptr = MevalTop2(icurve_mul,0);
	A = *ptr; B = *(ptr+1); C = *(ptr+2); 
#ifdef PRINT_CALC
fprintf(stderr,"mat (%f %f %f %f) det %f tr %f disc %f\n",
	A,B,C,D,det,tr,disc);
#endif
	if( A != A || B != B || C != C) 
	{
		p->x = p->y = p->z = 0.0;
		return(0.0);
	}

	if( global_orient == ORIENT_UN
	 && ( p->x * A + p->y * B + p->z * C < 0.0 ) )
	{	p->x = -A; p->y = -B; p->z = -C; }
	else
	{	p->x = A; p->y = B; p->z = C; }

	Mset_var_val(main_mul,3,p->x);
	Mset_var_val(main_mul,4,p->y);
	Mset_var_val(main_mul,5,p->z);
	MTevalForTop(psurf_trans,0);
	MTcopyVars(psurf_trans);
	ptr = MevalTop2(main_mul,0);
#ifdef PRINT_CALC
fprintf(stderr,"calc_near (%f,%f) u (%f,%f,%f) val %f\n",
		x,y,p->x,p->y,p->z,*ptr);
#endif
	return(*ptr);
}

/*
 * Transform an individual point, used in the _PROJ methods
 */

PointMap(pl,nl,cl)
HPoint3 *pl; Point3 *nl; ColorA *cl;
{
	float	val;
	double	a,b,c,d,e,f,v0,v1,v2,len;
	int	offset=0,norm_ref = 1;

	double *ptr;

	/* Set the variables */

	MstartEval(psurf_mul);
	Mset_var_val(psurf_mul,0,pl->x);
	Mset_var_val(psurf_mul,1,pl->y);
	ptr = MevalTop2(psurf_mul,0);
	val = (float) *ptr; pl->x = CLIP(val); 
	val = (float) *(ptr+1); pl->y = CLIP(val); 
	val = (float) *(ptr+2); pl->z = CLIP(val); 
	ptr = MevalTopDeriv(psurf_mul,0,0);
	a = *ptr; b = *(ptr+1); c = *(ptr+2);
	ptr = MevalTopDeriv(psurf_mul,0,1);
	d = *ptr; e = *(ptr+1); f = *(ptr+2);

	/* normal is dot prod */

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
		nl->x = (float) 0.0;
		nl->y = (float) 0.0;
		nl->z = (float) 0.0;
	}
	else /* Propper numbers */
	{
		nl->x = (float) v0/len;
		nl->y = (float) v1/len;
		nl->z = (float) v2/len;
	} /* For NaN do nothing */

}


/*
 * Function;	limit_normal
 * Action:	calculates the normal when the surface is singular
 */

LimitMap(pt,norm)
HPoint3 *pt;
Point3 *norm;
{}

/*
 * Function:	intersect
 * Action;	read in data from file and output clipped version
 */

intersect(int type,char *filename)
{
#ifndef NO_GEOM
	Geom	*geom, *in_geom, *load_geom(),
		*GeomIntersect(Geom *,double (*fun)());
	int     new_edit_time;
#endif
	int	i;
	FILE    *fp;


	print_time_message("intersect");
	if(main_mul->error  )
	{
		fprintf(stderr,"Can't perform opperation - bad equations\n");
		return;
	}
	tolerance = (double) Mget_opt_val_by_name(main_mul,TOL_NAME);
	max_itt = (int) Mget_opt_val_by_name(main_mul,ITT_NAME);
	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
	{
		return(intersect_psurf(type,filename));
	}
	if( global_mode == MODE_PCURVE )
	{
		return(intersect_pcurve(type,filename));
	}
	if( global_mode == MODE_IMPSURF )
	{
		return(intersect_impsurf(type,filename));
	}
	if( global_mode == MODE_ICV_PSURF 
	 || global_mode == MODE_ICV_PSURF_PROJ )
	{
		if( type )
		{
			fprintf(stderr,"Can only calculate zero set for Psurf/Icurve operators");
			return;
		}
		return(intersect_icurve_psurf(filename));
	}

#ifndef NO_GEOM
	in_geom = load_geom();
	if( in_geom == NULL )
	{
		fprintf(stderr,
			"Bad or empty geometry, can't perform operation\n");
		fclose(fp);
		return;
	}

	start_geom();
	printf("LIST\n");
	if( !type )
		printf("COMMENT intersect %s {\n",LSMP_DEF_NAME);
	else if( intersect_sign > 0.0 )
		printf("COMMENT clip+ %s {\n",LSMP_DEF_NAME);
	else
		printf("COMMENT clip- %s {\n",LSMP_DEF_NAME);
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT intersect %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	/* Now the data */

	if( type )
	{
		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		GeomClip(geom,rpe_fun); 
	 }
	else
	{
		Geom *new_geom;

		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		new_geom =  GeomIntersect(geom,rpe_fun);
		GeomFree(geom);
		geom = new_geom;
	}
	GeomFSave(geom,stdout,NULL);
	GeomFree(geom);
	fini_geom();
#endif

#ifdef CGIVRML
	if(vrml_version!=3)
	{
		report_error(HEAD_ERROR,"Vrml version must be 3",101);
		return;
	}
	if(test_xml_errors(jvx))
	{
               	report_error2(HEAD_ERROR,"Parse error in JVX: %s",
			get_first_xml_error(jvx),2);
		print_xml_errors(stdout,jvx);
                exit(1);
	}
	printf("Content-type: text/plain\n\n");

	if( type )
		JvxClip(jvx,rpe_fun); 
	else
		JvxIntersect(jvx,rpe_fun);

	printf("OK Surface sucessfully calculated\n");

	print_jvx_header("Intersect","Result of intersecting a surface");
		
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);

	print_jvx_header2("Intersect");

	print_jvx_subtree(stdout,jvx,"geometries",0);
	print_jvx_tail();
#endif
}

/* type is 0 for intersect +/-1 for clip */

intersect_psurf(int type,char *filename)
{
#ifndef NO_GEOM
	Geom	*geom, *in_geom, *load_geom(), *load_geom2(),
		*GeomIntersect(Geom *,double (*fun)());
	int	i;
	CommentList     *defCom,*timeCom;
	Geom    *psurf_geom;
#endif
	char    comment_file_name[L_tmpnam];
	int     new_edit_time;
	int     res;
	FILE	*fp;

	tolerance = (double) Mget_opt_val_by_name(main_mul,TOL_NAME);
	max_itt = (int) Mget_opt_val_by_name(main_mul,ITT_NAME);
	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

#ifndef NO_GEOM
	/* Now read in the opperator */
	if(psurf_tgt_name == NULL )
	{
		fprintf(stderr,"NULL psurf ingredient name\n");
		return;
	}
	psurf_geom = load_geom2(psurf_tgt_name);

	if( psurf_geom == NULL )
	{
		fprintf(stderr,"NULL psurf geom name <%s>\n",psurf_tgt_name);
		return;
	}

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
#endif

	/* have sucessfully read in the comment */

 
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
		fprintf(stderr,
			"Bad or empty geometry, can't perform operation\n");
		fclose(fp);
		return;
	}

	start_geom();
	printf("LIST\n");
	if( !type )
		printf("COMMENT intersect %s {\n",LSMP_DEF_NAME);
	else if( intersect_sign > 0.0 )
		printf("COMMENT clip+ %s {\n",LSMP_DEF_NAME);
	else
		printf("COMMENT clip- %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT intersect %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	/* Now the data */

	if( type )
	{
		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		GeomClip(geom,rpe_fun); 
	 }
	else
	{
		Geom *new_geom;

		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		new_geom =  GeomIntersect(geom,rpe_fun_psurf);
		GeomFree(geom);
		geom = new_geom;
	}
	if( global_mode == MODE_PSURF_PROJ ) GeomMap(geom);
	GeomFSave(geom,stdout,NULL);
	GeomFree(geom);
	fini_geom();
#endif

#ifdef CGIVRML
	if(vrml_version!=3)
	{
		report_error(HEAD_ERROR,"Vrml version must be 3",101);
		return;
	}
	if(test_xml_errors(jvx))
	{
               	report_error2(HEAD_ERROR,"Parse error in JVX: %s",
			get_first_xml_error(jvx),2);
		print_xml_errors(stdout,jvx);
                exit(1);
	}
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
	fclose(fp);

	printf("Content-type: text/plain\n\n");

	if( type )
		JvxClip(jvx,rpe_fun_psurf); 
	else
		JvxIntersect(jvx,rpe_fun_psurf);

	printf("OK Surface sucessfully calculated\n");

	print_jvx_header("Intersect","Result of intersecting a surface");
		
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);

	print_jvx_header2("Intersect");

	print_jvx_subtree(stdout,jvx,"geometries",0);
	print_jvx_tail();
#endif
}

intersect_pcurve(int type,char *filename)
{
#ifndef NO_GEOM
	Geom	*geom, *in_geom, *load_geom(), *load_geom2(),
		*GeomIntersect(Geom *,double (*fun)());
	int	i;
	CommentList     *defCom,*timeCom;
	Geom    *pcurve_geom;
#endif
	char    comment_file_name[L_tmpnam];
	int     new_edit_time;
	int     res;
	FILE	*fp;

	tolerance = (double) Mget_opt_val_by_name(main_mul,TOL_NAME);
	max_itt = (int) Mget_opt_val_by_name(main_mul,ITT_NAME);
	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

#ifndef NO_GEOM
	/* Now read in the opperator */
	if(pcurve_tgt_name == NULL )
	{
		fprintf(stderr,"NULL pcurve ingredient name\n");
		return;
	}
	pcurve_geom = load_geom2(pcurve_tgt_name);

	if( pcurve_geom == NULL )
	{
		fprintf(stderr,"NULL pcurve geom name <%s>\n",pcurve_tgt_name);
		return;
	}

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	defCom = GeomCommentGet(pcurve_geom,LSMP_DEF_NAME);

	if( defCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_DEF_NAME);		CommentListFree(defCom);
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
		pcurve_edit_time = 0;
		return;
	}
#endif

	/* have sucessfully read in the comment */

 
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
		fprintf(stderr,
			"Bad or empty geometry, can't perform operation\n");
		fclose(fp);
		return;
	}

	start_geom();
	printf("LIST\n");
	if( !type )
		printf("COMMENT intersect %s {\n",LSMP_DEF_NAME);
	else if( intersect_sign > 0.0 )
		printf("COMMENT clip+ %s {\n",LSMP_DEF_NAME);
	else
		printf("COMMENT clip- %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT intersect %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	/* Now the data */

	if( type )
	{
		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		GeomClip(geom,rpe_fun_pcurve); 
	 }
	else
	{
		Geom *new_geom;

		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		new_geom =  GeomIntersect(geom,rpe_fun_pcurve);
		GeomFree(geom);
		geom = new_geom;
	}
	GeomFSave(geom,stdout,NULL);
	GeomFree(geom);
	fini_geom();
#endif


#ifdef CGIVRML
	if(vrml_version!=3)
	{
		report_error(HEAD_ERROR,"Vrml version must be 3",101);
		return;
	}
	if(test_xml_errors(jvx))
	{
               	report_error2(HEAD_ERROR,"Parse error in JVX: %s",
			get_first_xml_error(jvx),2);
		print_xml_errors(stdout,jvx);
                exit(1);
	}
	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",auxGeomDef);
	fclose(fp);
	fp = fopen(comment_file_name,"r");
	res = read_pcurve_def(fp,pcurve_mul);
	fclose(fp);
	if( !res ) 
	{
               	report_error(HEAD_ERROR,"Error reading pcurve definition",3);
		exit(1);
	}
	
	printf("Content-type: text/plain\n\n");

	if( type )
		JvxClip(jvx,rpe_fun_pcurve); 
	else
		JvxIntersect(jvx,rpe_fun_pcurve);

	printf("OK Surface sucessfully calculated\n");

	print_jvx_header("Intersect","Result of intersecting a surface");
		
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);

	print_jvx_header2("Intersect");

	print_jvx_subtree(stdout,jvx,"geometries",0);
	print_jvx_tail();
#endif
}

/* type is 0 for intersect +/-1 for clip */

intersect_impsurf(int type,char *filename)
{
#ifndef NO_GEOM
	Geom	*geom, *in_geom, *load_geom(), *load_geom2(),
		*GeomIntersect(Geom *,double (*fun)());
	int	i;
	CommentList     *defCom,*timeCom;
	Geom    *impsurf_geom;
#endif
	char    comment_file_name[L_tmpnam];
	int     new_edit_time;
	int     res;
	FILE	*fp;

	tolerance = (double) Mget_opt_val_by_name(main_mul,TOL_NAME);
	max_itt = (int) Mget_opt_val_by_name(main_mul,ITT_NAME);
	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

	/* Now read in the opperator */

#ifndef NO_GEOM
	if(impsurf_tgt_name == NULL )
	{
		fprintf(stderr,"NULL impsurf ingredient name\n");
		return;
	}
	impsurf_geom = load_geom2(impsurf_tgt_name);

	if( impsurf_geom == NULL )
	{
		fprintf(stderr,"NULL impsurf geom name <%s>\n",impsurf_tgt_name);
		return;
	}

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	defCom = GeomCommentGet(impsurf_geom,LSMP_DEF_NAME);

	if( defCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_DEF_NAME);		CommentListFree(defCom);
		GeomFree(impsurf_geom);
		return;
	}
	else if( defCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(impsurf_geom);
		return;
	}
	else if( defCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(impsurf_geom);
		return;
	}

	timeCom = GeomCommentGet(impsurf_geom,LSMP_EDIT_TIME_NAME);

	if( timeCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(impsurf_geom);
		return;
	}
	else if( timeCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(impsurf_geom);
		return;
	}
	else if( timeCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of type %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(impsurf_geom);
		return;
	}


	new_edit_time = atoi(timeCom->com->data);
	if( new_edit_time == 0 )
	{
		fprintf(stderr,"Bad timestamp data <%s>\n",timeCom->com->data);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(impsurf_geom);
		return;
	}
fprintf(stderr,"impsurf Equation is\n%s\n",defCom->com->data);
/*
*/
	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",defCom->com->data);
	fclose(fp);
	fp = fopen(comment_file_name,"r");

	if( new_edit_time != impsurf_edit_time )
	{
		impsurf_edit_time = new_edit_time;
		res = read_impsurf_def(fp,impsurf_mul);
	} else
	{
		res = fscanMultiParams(fp,impsurf_mul);
	}
	fclose(fp);
	unlink(comment_file_name);


	CommentListFree(timeCom);
	CommentListFree(defCom);
	GeomFree(impsurf_geom);
	if( !res )
	{
		impsurf_edit_time = 0;
		return;
	}
#endif

	/* have sucessfully read in the comment */

 
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
		fprintf(stderr,
			"Bad or empty geometry, can't perform operation\n");
		fclose(fp);
		return;
	}

	start_geom();
	printf("LIST\n");
	if( !type )
		printf("COMMENT intersect %s {\n",LSMP_DEF_NAME);
	else if( intersect_sign > 0.0 )
		printf("COMMENT clip+ %s {\n",LSMP_DEF_NAME);
	else
		printf("COMMENT clip- %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT intersect %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	/* Now the data */

	if( type )
	{
		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		GeomClip(geom,rpe_fun_impsurf); 
	}
	else
	{
		Geom *new_geom;

		geom = GeomSimp3(in_geom);
		GeomFree(in_geom);
		new_geom =  GeomIntersect(geom,rpe_fun_impsurf);
		GeomFree(geom);
		geom = new_geom;
	}
	GeomFSave(geom,stdout,NULL);
	GeomFree(geom);
	fini_geom();
#endif

#ifdef CGIVRML
	if(vrml_version!=3)
	{
		report_error(HEAD_ERROR,"Vrml version must be 3",101);
		return;
	}
	if(test_xml_errors(jvx))
	{
               	report_error2(HEAD_ERROR,"Parse error in JVX: %s",
			get_first_xml_error(jvx),2);
		print_xml_errors(stdout,jvx);
                exit(1);
	}

	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",auxGeomDef);
	fclose(fp);
	fp = fopen(comment_file_name,"r");
	res = read_impsurf_def(fp,impsurf_mul);
	fclose(fp);
	if( !res ) 
	{
               	report_error(HEAD_ERROR,"Error reading impsurf definition",3);
		exit(1);
	}

	printf("Content-type: text/plain\n\n");

	if( type )
		JvxClip(jvx,rpe_fun_impsurf); 
	else
		JvxIntersect(jvx,rpe_fun_impsurf);

	printf("OK Surface sucessfully calculated\n");

	print_jvx_header("Intersect","Result of intersecting a surface");
		
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);

	print_jvx_header2("Intersect");

	print_jvx_subtree(stdout,jvx,"geometries",0);
	print_jvx_tail();
#endif
}

intersect_icurve_psurf(char *filename)
{
#ifndef NO_GEOM
	Geom	*geom, *in_geom, *new_geom,*load_geom(), *load_geom2(),
		*GeomRIntersect(Geom *),*GeomUIntersect(Geom *);
	int	i;
	CommentList     *defCom,*timeCom;
	Geom    *psurf_geom,*icurve_geom;
	int     new_ps_edit_time,new_ic_edit_time;
#endif
	char    comment_file_name[L_tmpnam];
	int     res;
	FILE	*fp;

	tolerance = (double) Mget_opt_val_by_name(main_mul,TOL_NAME);
	max_itt = (int) Mget_opt_val_by_name(main_mul,ITT_NAME);
	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);


	/***************** Now the icurve ********************/
#ifndef NO_GEOM

	icurve_geom = load_geom2(icurve_tgt_name);

	if( icurve_geom == NULL )
	{
		fprintf(stderr,"NULL icurve geom\n");
		return;
	}

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	defCom = GeomCommentGet(icurve_geom,LSMP_DEF_NAME);

	if( defCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_DEF_NAME);		CommentListFree(defCom);
		GeomFree(icurve_geom);
		return;
	}
	else if( defCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s while reading icurve\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(icurve_geom);
		return;
	}
	else if( defCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_DEF_NAME);
		CommentListFree(defCom);
		GeomFree(icurve_geom);
		return;
	}

	timeCom = GeomCommentGet(icurve_geom,LSMP_EDIT_TIME_NAME);

	if( timeCom == NULL )
	{
		fprintf(stderr,"No Comment of type %s in data\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(icurve_geom);
		return;
	}
	else if( timeCom->next != NULL )
	{
		fprintf(stderr,"More than one comment of type %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(icurve_geom);
		return;
	}
	else if( timeCom->com->length != 0 )
	{
		fprintf(stderr,"Binary Comment Data of tpye %s\n",LSMP_EDIT_TIME_NAME);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(icurve_geom);
		return;
	}


	new_ic_edit_time = atoi(timeCom->com->data);
	if( new_ic_edit_time == 0 )
	{
		fprintf(stderr,"Bad timestamp data <%s>\n",timeCom->com->data);
		CommentListFree(timeCom);
		CommentListFree(defCom);
		GeomFree(icurve_geom);
		return;
	}
/*
fprintf(stderr,"icurve Equation is\n%s\n",defCom->com->data);
*/
	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",defCom->com->data);
	fclose(fp);
	fp = fopen(comment_file_name,"r");

	if( new_ic_edit_time != icurve_edit_time )
	{
		icurve_edit_time = new_ic_edit_time;
		res = read_icurve_def(fp,icurve_mul);
	} else
	{
		res = fscanMultiParams(fp,icurve_mul);
	}
	fclose(fp);
	unlink(comment_file_name);


	CommentListFree(timeCom);
	CommentListFree(defCom);
	GeomFree(icurve_geom);
	if( !res )
	{
		icurve_edit_time = 0;
		return;
	}


	/********** Now read in the psurf *************/

	psurf_geom = load_geom2(psurf_tgt_name);

	if( psurf_geom == NULL )
	{
		fprintf(stderr,"NULL psurf geom\n");
		return;
	}

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
		fprintf(stderr,"Binary Comment Data of type %s\n",LSMP_DEF_NAME);
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


	new_ps_edit_time = atoi(timeCom->com->data);
	if( new_ps_edit_time == 0 )
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

	if( new_ps_edit_time != psurf_edit_time 
	 || new_ic_edit_time != icurve_edit_time )
	{
		psurf_edit_time = new_ps_edit_time;
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
/*
fprintf(stderr,"main\n\n");
dump_multi(stderr,main_mul);
fprintf(stderr,"psurf\n\n");
dump_multi(stderr,psurf_mul);
fprintf(stderr,"icurve\n\n");
dump_multi(stderr,icurve_mul);
fprintf(stderr,"ps_tr\n\n");
dumpMTrans(stderr,psurf_trans);
fprintf(stderr,"ic_ps_tr\n\n");
dumpMTrans(stderr,icurve_psurf_trans);
*/

	/* have sucessfully read in the comment */
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
		fprintf(stderr,
			"Bad or empty geometry, can't perform operation\n");
		fclose(fp);
		return;
	}

	start_geom();
	printf("LIST\n");
	printf("COMMENT intersect %s {\n",LSMP_DEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT intersect %s { %d }\n",LSMP_EDIT_TIME_NAME,edit_time);

	/* Now process the comments, we want to use two types
		LSMP_DEF the definition and
		LSMP_EDIT_TIME_NAME the time the def was last changed.
	*/

	in_geom = GeomCommentRemove(in_geom,LSMP_DEF_NAME);
	in_geom = GeomCommentRemove(in_geom,LSMP_EDIT_TIME_NAME);

	/* Now the data */

	geom = GeomSimp3(in_geom);
	GeomFree(in_geom);
	if( global_orient == ORIENT_MAJOR || global_orient == ORIENT_MINOR )
	{
		new_geom = GeomRIntersect(geom);
	}
	else
	{
		new_geom = GeomUIntersect(geom);
	}
	GeomFree(geom);
	if( global_mode == MODE_ICV_PSURF_PROJ ) 
	{
		GeomMap(new_geom);
	}
	GeomFSave(new_geom,stdout,NULL);
	GeomFree(new_geom);
	fini_geom();
#endif

#ifdef CGIVRML
	if(vrml_version!=3)
	{
		report_error(HEAD_ERROR,"Vrml version must be 3",101);
		return;
	}
	if(test_xml_errors(jvx))
	{
               	report_error2(HEAD_ERROR,"Parse error in JVX: %s",
			get_first_xml_error(jvx),2);
		print_xml_errors(stdout,jvx);
                exit(1);
	}

	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",auxGeomDef2);
	fclose(fp);
	fp = fopen(comment_file_name,"r");
	res = read_icurve_def(fp,icurve_mul);
	if( !res ) 
	{
               	report_error(HEAD_ERROR,"Error reading icurve definition",3);
		exit(1);
	}
	fclose(fp);

	tmpnam(comment_file_name);
	fp = fopen(comment_file_name,"w");
	fprintf(fp,"%s\n",auxGeomDef);
	fclose(fp);
	fp = fopen(comment_file_name,"r");
	res = read_psurf_def(fp,psurf_mul);
	fclose(fp);
	if( !res ) 
	{
               	report_error(HEAD_ERROR,"Error reading psurf definition for (psurf/icurve op)",3);
		exit(1);
	}


	printf("Content-type: text/plain\n\n");

	if( global_orient == ORIENT_MAJOR || global_orient == ORIENT_MINOR )
	{
		JvxRIntersect(jvx);
	}
	else
	{
		JvxUIntersect(jvx);
	}

	printf("OK Surface sucessfully calculated\n");

	print_jvx_header("Intersect","Result of intersecting a surface");
		
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read file %s\n",filename);
		return;
	}
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);

	print_jvx_header2("Intersect");

	print_jvx_subtree(stdout,jvx,"geometries",0);
	print_jvx_tail();
#endif
}

/********** Special Object Drawing Routines *************/

set_special(val)
{
	special_obj = val;
}

#define unit(v) {len = sqrt(v.x*v.x+v.y*v.y+v.z*v.z); \
		v.x /= len; v.y /= len; v.z /= len; }

draw_special()
{
#ifdef USE_SPECIAL
	double a,b,c,d,l,th,phi,len;
	Point3 v1,v2;
	int  i,j;

	if(special_obj == 0 ) return;

	if( !command ) return;
	switch(special_obj)
	{
	case BOX: 
		set_geom_name("Box");
		start_geom();
		printf("OFF\n");
		printf("8 6 12\n");
		printf("%f %f %f\n",
			Mget_param_val(main_mul,1),
			Mget_param_val(main_mul,3),
			Mget_param_val(main_mul,5));
		printf("%f %f %f\n",
			Mget_param_val(main_mul,1),
			Mget_param_val(main_mul,4),
			Mget_param_val(main_mul,5));
		printf("%f %f %f\n",
			Mget_param_val(main_mul,2),
			Mget_param_val(main_mul,4),
			Mget_param_val(main_mul,5));
		printf("%f %f %f\n",
			Mget_param_val(main_mul,2),
			Mget_param_val(main_mul,3),
			Mget_param_val(main_mul,5));
		printf("%f %f %f\n",
			Mget_param_val(main_mul,1),
			Mget_param_val(main_mul,3),
			Mget_param_val(main_mul,6));
		printf("%f %f %f\n",
			Mget_param_val(main_mul,1),
			Mget_param_val(main_mul,4),
			Mget_param_val(main_mul,6));
		printf("%f %f %f\n",
			Mget_param_val(main_mul,2),
			Mget_param_val(main_mul,4),
			Mget_param_val(main_mul,6));
		printf("%f %f %f\n",
			Mget_param_val(main_mul,2),
			Mget_param_val(main_mul,3),
			Mget_param_val(main_mul,6));
		printf("4 0 1 2 3\n");
		printf("4 7 4 0 3\n");
		printf("4 4 5 1 0\n");
		printf("4 5 6 2 1\n");
		printf("4 3 2 6 7\n");
		printf("4 6 5 4 7\n"); 
		fini_geom();
		break;
	case PLANE:
		a = Mget_param_val(main_mul,0);
		b = Mget_param_val(main_mul,1);
		c = Mget_param_val(main_mul,2);
		d = Mget_param_val(main_mul,3);

		if( a == 0.0 && b == 0.0 && c == 0.0 ) break;
		l = d/(a*a+b*b+c*c);

		if(fabs(c) >= fabs(a) && fabs(c) >= fabs(b) )
		{
			v1.x = 0.0; v1.y = -c;  v1.z = b;
		}
		else if(fabs(b) >= fabs(a))
		{
			v1.x = -b;   v1.y = a;  v1.z = 0.0;
		}
		else
		{
			v1.x = c;   v1.y = 0.0; v1.z = -a;
		}
		unit(v1);
		v2.x = v1.y * c - v1.z * b;
		v2.y = v1.z * a - v1.x * c;
		v2.z = v1.x * b - v1.y * a;
		unit(v2);

		set_geom_name("Plane");
		start_geom();
		printf("QUAD\n");
		printf("%f %f %f\n",l*a+v1.x+v2.x,l*b+v1.y+v2.y,l*c+v1.z+v2.z);
		printf("%f %f %f\n",l*a+v1.x-v2.x,l*b+v1.y-v2.y,l*c+v1.z-v2.z);
		printf("%f %f %f\n",l*a-v1.x-v2.x,l*b-v1.y-v2.y,l*c-v1.z-v2.z);
		printf("%f %f %f\n",l*a-v1.x+v2.x,l*b-v1.y+v2.y,l*c-v1.z+v2.z);
		fini_geom();
		break;

	case SPHERE:
		d = Mget_param_val(main_mul,0); /* d is radius */
		a = Mget_param_val(main_mul,2);
		b = Mget_param_val(main_mul,3);
		c = Mget_param_val(main_mul,4);

		set_geom_name("Sphere");
		start_geom();
		printf("MESH\n");
		printf("11 11\n");
		for(i=0;i<=10;++i)
		{
		    th = M_PI * i/10;
		    for(j=0;j<=10;++j)
		    {
			phi = M_PI * j/5;
			printf("%f %f %f\n",a + d * sin(th) * cos(phi),
					    b + d * sin(th) * sin(phi),
					    c + d * cos(th));
		    }
		}
		fini_geom();
		break;
	}
#endif
}

#ifdef USE_SPECIAL
remove_special()
{
	extern char *geomname;
	extern int geomnumber;
	char oldname[128];
	int  oldnum;
	char oldhandle[20];
	double a,b,c,d,l,th,phi;
	Point3 v1,v2;
	int  i,j;

	if(special_obj == 0 ) return;

	if( !command ) return;
	strcpy(oldname,geomname);
	oldnum = geomnumber;
	switch(special_obj)
	{
	case BOX: 
		set_geom_name("Box");
		break;

	case PLANE:
		set_geom_name("Plane");
		break;

	case SPHERE:
		set_geom_name("Sphere");
		break;
		
	}
	if(geomnumber == 0 )
		printf("(if (real-id %s) (delete %s))",geomname,geomname);
	else
		printf("(if (real-id %s<%d>) (delete %s<%d>))",
			geomname,geomnumber,geomname,geomnumber);
	fflush(stdout);
	strcpy(geomname,oldname);
	geomnumber = oldnum;
}
#endif

/********** File and definition handleing ***************/
/*
 * Function:	read_def
 * Action:	read in a new equation from 'filename',
 *		create a new geometry, set the default values etc..
 */

int read_def(FILE *fp)
{
	int i;

	edit_time = 0;
	Mset_opt_val_by_name(main_mul,MODE_NAME,(float) global_mode);
	if( !fscanMulti(fp,main_mul) ) return(FALSE);
	if( !MfindOpts(main_mul) ) return(FALSE);

	global_mode = (int) Mget_opt_val_by_name(main_mul,MODE_NAME);
	global_cols = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	MsetNtop(main_mul,1);
	MsetTopDim(main_mul,0,-1);

	if( !McombineTop(main_mul) ) return(FALSE);

	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ 
	 || global_mode == MODE_PCURVE )
	{
		MsetNvars(main_mul,5);	/* i.e. ?1,?2,?3 : ?4,?5 */
		Mset_var_name(main_mul,0,"?1");
		Mset_var_name(main_mul,1,"?2");
		Mset_var_name(main_mul,2,"?3");
		Mset_var_name(main_mul,3,"?4");
		Mset_var_name(main_mul,4,"?5");
	}
	else if( global_mode == MODE_IMPSURF )
	{
		MsetNvars(main_mul,4);	/* i.e. ?1,?2,?3 : ?4,?5 */
		Mset_var_name(main_mul,0,"?1");
		Mset_var_name(main_mul,1,"?2");
		Mset_var_name(main_mul,2,"?3");
		Mset_var_name(main_mul,3,"?4");
	}
	else if( global_mode == MODE_ICV_PSURF || 
		 global_mode == MODE_ICV_PSURF_PROJ )
	{
		MsetNvars(main_mul,9);	/* i.e. ?1,?2,?3 : ?4,?5 */
		Mset_var_name(main_mul,0,"?1");
		Mset_var_name(main_mul,1,"?2");
		Mset_var_name(main_mul,2,"?3");
		Mset_var_name(main_mul,3,"?4");
		Mset_var_name(main_mul,4,"?5");
		Mset_var_name(main_mul,5,"?6");
		Mset_var_name(main_mul,6,"?7");
		Mset_var_name(main_mul,7,"?8");
		Mset_var_name(main_mul,8,"?9");
	}
	if( !MfindNames(main_mul) ) return(FALSE);
	MsetNtopDerivs(main_mul,0,0);

	if( !McheckDims(main_mul) ) return(FALSE);
	if( !McalcDerivs(main_mul) ) return(FALSE);
	if( !McalcRPEs(main_mul) ) return(FALSE);
	edit_time = time(NULL);

	/* want to force reinterpretation of ingredients */

	impsurf_edit_time = psurf_edit_time = icurve_edit_time = 0;
	pcurve_edit_time = 0;
	return(TRUE);
}

/*
 * When we read form a file we want to reset the variable names
 */

read_file(FILE *fp)
{
	MsetNvars(main_mul,3);
	Mset_var_name(main_mul,0,"x");
	Mset_var_name(main_mul,1,"y");
	Mset_var_name(main_mul,2,"z");
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) PREC_DEFAULT);
	Mset_opt_val_by_name(main_mul,TOL_NAME,(double) TOL_DEFAULT);
	Mset_opt_val_by_name(main_mul,ITT_NAME,(double) ITT_DEFAULT);
	Mset_opt_val_by_name(main_mul,COL_NAME,(double) STD_COL);
	return(read_def(fp));
}

read_psurf_def(FILE *fp)
{
	int     dimension,normals,colours;
	char    str[100];

	Mclear(psurf_mul);
	if( !fscanMulti(fp,psurf_mul) ) return(FALSE);
	if( !MfindOpts(psurf_mul) ) return(FALSE);

	dimension = (int) Mget_opt_val_by_name(psurf_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(psurf_mul,COL_NAME);
	normals = (int) Mget_opt_val_by_name(psurf_mul,NORM_NAME);
	clipmax = (float) Mget_opt_val_by_name(psurf_mul,CLIP_NAME);

	if(colours == EQN_COL )
	{
		fprintf(stderr,"Can't perform intersect with operators\n");
		fprintf(stderr," and equations for colours in psurf\n");
		return(FALSE);
	}
	if(normals == EQN_NORM )
	{
		fprintf(stderr,"Can't perform intersect with operators\n");
		fprintf(stderr," and equations for normals in psurf\n");
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
	Mset_var_name(psurf_mul,1,"y");
	if( !MfindNames(psurf_mul) ) return(FALSE);
	MsetNtopDerivs(psurf_mul,0,2);
	MsetDerivName(psurf_mul,0,0,Mget_var_name(psurf_mul,0));
	MsetDerivName(psurf_mul,0,1,Mget_var_name(psurf_mul,1));
	if( !McheckDims(psurf_mul) ) return(FALSE);
    

	if( global_mode == MODE_PSURF || global_mode == MODE_PSURF_PROJ )
	{
		MTdefine(psurf_trans,main_mul,psurf_mul);
		MTsetVarTop(psurf_trans,0,0,0);
		MTsetVarTop(psurf_trans,1,0,1);
		MTsetVarTop(psurf_trans,2,0,2);
		MTsetVarVar(psurf_trans,3,0);
		MTsetVarVar(psurf_trans,4,1);
		MTcalcVarTrans(psurf_trans);
		if( !MTcheck(psurf_trans) ) return(FALSE);
	}
	else if( global_mode == MODE_ICV_PSURF || 
		 global_mode == MODE_ICV_PSURF_PROJ ) 
	{
		MTdefine(psurf_trans,main_mul,psurf_mul);
		MTsetVarTop(psurf_trans,0,0,0);
		MTsetVarTop(psurf_trans,1,0,1);
		MTsetVarTop(psurf_trans,2,0,2);
		MTsetVarIgnore(psurf_trans,3);
		MTsetVarIgnore(psurf_trans,4);
		MTsetVarIgnore(psurf_trans,5);
#ifdef NON_EIGEN
		if( global_orient == ORIENT_MAJOR
			 || global_orient == ORIENT_MINOR )
		{
#endif
			MTsetVarIgnore(psurf_trans,6);
			MTsetVarVar(psurf_trans,7,0);
			MTsetVarVar(psurf_trans,8,1);
#ifdef NON_EIGEN
		}
		else
		{
			MTsetVarVar(psurf_trans,6,0);
			MTsetVarVar(psurf_trans,7,1);
			MTsetVarIgnore(psurf_trans,8);
		}
#endif
		MTcalcVarTrans(psurf_trans);
		if( !MTcheck(psurf_trans) ) return(FALSE);

		MTdefine(icurve_psurf_trans,icurve_mul,psurf_mul);
		MTsetVarTop(icurve_psurf_trans,0,0,0);
		MTsetVarTop(icurve_psurf_trans,1,0,1);
		MTsetVarTop(icurve_psurf_trans,2,0,2);
		MTsetVarVar(icurve_psurf_trans,3,0);
		MTsetVarVar(icurve_psurf_trans,4,1);
		MTcalcVarTrans(icurve_psurf_trans);
		if( !MTcheck(icurve_psurf_trans) ) return(FALSE);

	}

	if( !McalcDerivs(psurf_mul) ) return(FALSE);
	if( !McalcRPEs(psurf_mul) ) return(FALSE);

	return(TRUE);
}

read_pcurve_def(FILE *fp)
{
	int     dimension,normals,colours;
	char    str[100];

	Mclear(pcurve_mul);
	if( !fscanMulti(fp,pcurve_mul) ) return(FALSE);
	if( !MfindOpts(pcurve_mul) ) return(FALSE);
	dimension = (int) Mget_opt_val_by_name(pcurve_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(pcurve_mul,COL_NAME);
	clipmax = (float) Mget_opt_val_by_name(pcurve_mul,CLIP_NAME);

	if(colours == EQN_COL )
	{
		fprintf(stderr,"Can't perform intersect with operators\n");
		fprintf(stderr," and equations for colours in pcurve\n");
		return(FALSE);
	}
	if(dimension != THREE_D_EQN )
	{
		fprintf(stderr,"Psurf must be 3D when using operators\n");
		return(FALSE);
	}

	MsetNtop(pcurve_mul,1);
	MsetTopDim(pcurve_mul,0,3);

	if( !McombineTop(pcurve_mul) ) return(FALSE);

	MsetNvars(pcurve_mul,1);
	Mset_var_name(pcurve_mul,0,"x");
	if( !MfindNames(pcurve_mul) ) return(FALSE);
	MsetNtopDerivs(pcurve_mul,0,1);
	MsetDerivName(pcurve_mul,0,0,Mget_var_name(pcurve_mul,0));
	if( !McheckDims(pcurve_mul) ) return(FALSE);
    

	if( global_mode == MODE_PCURVE )
	{
		MTdefine(pcurve_trans,main_mul,pcurve_mul);
		MTsetVarTop(pcurve_trans,0,0,0);
		MTsetVarTop(pcurve_trans,1,0,1);
		MTsetVarTop(pcurve_trans,2,0,2);
		MTsetVarVar(pcurve_trans,3,0);
		MTsetVarIgnore(pcurve_trans,4);
		MTcalcVarTrans(pcurve_trans);
		if( !MTcheck(pcurve_trans) ) return(FALSE);
	}

	if( !McalcDerivs(pcurve_mul) ) return(FALSE);
	if( !McalcRPEs(pcurve_mul) ) return(FALSE);

	return(TRUE);
}

read_impsurf_def(FILE *fp)
{
	int     dimension,normals,colours;
	char    str[100];

	Mclear(impsurf_mul);
	if( !fscanMulti(fp,impsurf_mul) ) return(FALSE);
	if( !MfindOpts(impsurf_mul) ) return(FALSE);

	MsetNtop(impsurf_mul,1);
	MsetTopDim(impsurf_mul,0,-1);

	if( !McombineTop(impsurf_mul) ) return(FALSE);

	MsetNvars(impsurf_mul,3);
	Mset_var_name(impsurf_mul,0,"x");
	Mset_var_name(impsurf_mul,0,"y");
	Mset_var_name(impsurf_mul,0,"z");
	if( !MfindNames(impsurf_mul) ) return(FALSE);
	if( !McheckDims(impsurf_mul) ) return(FALSE);

	MTdefine(impsurf_trans,main_mul,impsurf_mul);
	MTsetVarTop(impsurf_trans,0,0,0);
	MTsetVarVar(impsurf_trans,1,0);
	MTsetVarVar(impsurf_trans,2,1);
	MTsetVarVar(impsurf_trans,3,2);
	MTcalcVarTrans(impsurf_trans);
	if( !MTcheck(impsurf_trans) ) return(FALSE);
	if( !McalcDerivs(impsurf_mul) ) return(FALSE);
	if( !McalcRPEs(impsurf_mul) ) return(FALSE);

	return(TRUE);
}

read_icurve_def(FILE *fp)
{
	int     dimension,colours,local_mode,local_orientation;
	char    str[100];

	Mclear(icurve_mul);
	if( !fscanMulti(fp,icurve_mul) ) return(FALSE);
	if( !MfindOpts(icurve_mul) ) return(FALSE);

	dimension = (int) Mget_opt_val_by_name(icurve_mul,DIM_NAME);
	colours = (int) Mget_opt_val_by_name(icurve_mul,COL_NAME);
	local_mode = (int) Mget_opt_val_by_name(icurve_mul,MODE_NAME);
	local_orientation = (int) Mget_opt_val_by_name(icurve_mul,ORIENT_NAME);
	global_orient = local_orientation;

	if(colours == EQN_COL )
	{
		fprintf(stderr,"Can't perform intersect with operators\n");
		fprintf(stderr," and equations for colours in icurve\n");
		return(FALSE);
	}
	if(dimension != THREE_D_EQN )
	{
		fprintf(stderr,"Psurf must be 3D when using operators\n");
		return(FALSE);
	}
	if( local_mode != MODE_PSURF && local_mode != MODE_PSURF_PROJ )
	{
		fprintf(stderr,"Mode of icurve must be %d\n",MODE_PSURF);
		return(FALSE);
	}

	if( local_orientation == ORIENT_MAJOR
	  || local_orientation == ORIENT_MINOR )
	{
		MsetNtop(icurve_mul,1);
		MsetTopDim(icurve_mul,0,4);
	}
#ifdef NON_EIGEN
	else if( local_orientation == ORIENT_UN
		 || local_orientation == ORIENT_ORIENT )
	{
		MsetNtop(icurve_mul,1);
		MsetTopDim(icurve_mul,0,3);
	}
	else
	{
	fprintf(stderr,"Orientation for icurve must be Oriented, Unoriented or and Eigen type. Its %d\n",
		local_orientation);
		return(FALSE);
	}
#else
	else
	{
	fprintf(stderr,"Orientation for icurve an Eigen type. Its %d\n",
		local_orientation);
		return(FALSE);
	}
#endif

	if( !McombineTop(icurve_mul) ) return(FALSE);
	MsetNvars(icurve_mul,5);
	Mset_var_name(icurve_mul,0,"?1");
	Mset_var_name(icurve_mul,1,"?2");
/*
	Mset_var_name(icurve_mul,2,"?3");
	Mset_var_name(icurve_mul,3,"?4");
	Mset_var_name(icurve_mul,4,"?5");
*/
	if( !MfindNames(icurve_mul) ) return(FALSE);
	if( !McheckDims(icurve_mul) ) return(FALSE);
	if( !McalcDerivs(icurve_mul) ) return(FALSE);
	if( !McalcRPEs(icurve_mul) ) return(FALSE);

	return(TRUE);
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

	/* We know we only have one top eqn so no need to use copy_top */
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

/*************	Call back routines from tcl ******************************/

#ifndef CGIVRML
/*
 * Load the file
 * given the filename and the name of a tempory file for the editor
 */

int load_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp,*fq;
	int i;

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
	i = read_file(fp);
	rewind(fp);
	fq = fopen(argv[2],"w");
	copy_def(fp,fq);
	fclose(fp);
	fclose(fq);
	set_geom_base_name(argv[1]);
	set_special(0);
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
	set_special(0);
	return TCL_OK;
}


/*
 * Update the equations, called when editor is changed
 *      given the name of editor file
 */

int update_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp;
	int i;
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
	set_special(0);
	return TCL_OK;
}

/*
 *	Called when the box option is selected
 *	Given the name of a temp file to put def for editor in
 */

int load_box_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp;

	if(argc != 2)
	{
		interp->result = "Wrong number of arguments";
		return TCL_ERROR;
	}

	fp = fopen(argv[1],"w");
	fprintf(fp,"# The interior of the box [xl,xh]X[yl,yh]X[zl,zh]\n");
	fprintf(fp,"if( R - reject^2, 0/0, val);\n");
	fprintf(fp,"R = x^2 + y^2 + z^2;\n");
	fprintf(fp,"val = min(min(min(xh-x,x-xl),min(yh-y,y-yl)),min(zh-z,z-zl));\n");
	fclose(fp);
#ifdef USE_SPECIAL
	fp = fopen(argv[1],"r");
	read_file(fp);
	fclose(fp);
	Mset_param_val(main_mul,0,100.0);
	Mset_param_val(main_mul,1,1.0);
	Mset_param_val(main_mul,2,-1.0);
	Mset_param_val(main_mul,3,1.0);
	Mset_param_val(main_mul,4,-1.0);
	Mset_param_val(main_mul,5,1.0);
	Mset_param_val(main_mul,6,-1.0);
	set_geom_base_name("Box");
	set_special(BOX);
	draw_special();
/*
	box_num = 0;
	while( geom_name_exists("Box",box_num) ) ++box_num;
*/
#endif
	return TCL_OK;
}

int load_sphere_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp;

	if(argc != 2)
	{
		interp->result = "Wrong number of arguments";
		return TCL_ERROR;
	}
	set_special(SPHERE);
	fp = fopen(argv[1],"w");
	fprintf(fp,"# The interior of a sphere center x0 y0 z0, radius r\n");
	fprintf(fp,"if( R - reject^2, 0/0, -R);\n");
	fprintf(fp,"R = (x-x0)^2 + (y-y0)^2 + (z-z0)^2 - r^2;\n");
	fclose(fp);
#ifdef USE_SPECIAL
	fp = fopen(argv[1],"r");
	read_file(fp);
	fclose(fp);
	Mset_param_val(main_mul,0,1.0);
	Mset_param_val(main_mul,1,100.0);
	Mset_param_val(main_mul,2,0.0);
	Mset_param_val(main_mul,3,0.0);
	Mset_param_val(main_mul,4,0.0);
	set_geom_base_name("Sphere");
	draw_special();
#endif
	return TCL_OK;
}

int load_plane_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	FILE *fp;

	if(argc != 2)
	{
		interp->result = "Wrong number of arguments";
		return TCL_ERROR;
	}
	fp = fopen(argv[1],"w");
	fprintf(fp,"# One side of a plane\n");
	fprintf(fp,"a x + b y + c z = d;\n");
	fclose(fp);
#ifdef USE_SPECIAL
	fp = fopen(argv[1],"r");
	read_file(fp);
	fclose(fp);
	Mset_param_val(main_mul,0,0.0);
	Mset_param_val(main_mul,1,1.0);
	Mset_param_val(main_mul,2,0.0);
	Mset_param_val(main_mul,3,0.0);
	set_special(PLANE);
	set_geom_base_name("Plane");
	draw_special();
#endif
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
	intersect_sign = 1.0;
	intersect(0,argv[1]);
	return TCL_OK;
}

int run_plus_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "run_cb: Wrong number of arguments";
		return TCL_ERROR;
	}

	intersect_sign = 1.0;
	intersect(1,argv[1]);
	return TCL_OK;
}

int run_minus_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "run_cb: Wrong number of arguments";
		return TCL_ERROR;
	}

	intersect_sign = -1.0;
	intersect(1,argv[1]);
	return TCL_OK;
}

int draw_special_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	draw_special();
	return TCL_OK;
}

/*
 * Passing info back and forth to tcl 
 */

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int precision,itt,col;
	double	tol;
	precision = (int) Mget_opt_val_by_name(main_mul,PREC_NAME);
	itt = (int) Mget_opt_val_by_name(main_mul,ITT_NAME);
	tol = (double) Mget_opt_val_by_name(main_mul,TOL_NAME);
	col = (int) Mget_opt_val_by_name(main_mul,COL_NAME);

	sprintf(interp->result,"%d %.9f %d %d\n",
		precision,tol,itt,col);
	return TCL_OK;
}

int set_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
/*
fprintf(stderr,"set_opt prec %s tol %s itt %s col %s\n",
	argv[1],argv[2],argv[3]i,argv[4]);
*/
	if(argc != 5)
	{
		interp->result = "set_options: Wrong number of arguments";
		return TCL_ERROR;
	}
	Mset_opt_val_by_name(main_mul,PREC_NAME, atof(argv[1]) );
	Mset_opt_val_by_name(main_mul,TOL_NAME, atof(argv[2]) );
	Mset_opt_val_by_name(main_mul,ITT_NAME, atof(argv[3]) );
	Mset_opt_val_by_name(main_mul,COL_NAME, atof(argv[4]) );
/*
fprintf(stderr,"and the value is now %.9f\n",
 Mget_opt_val_by_name(main_mul,TOL_NAME));
*/
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
		interp->result = "psurf_target_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	strcpy(pcurve_tgt_name,argv[1]);
	return TCL_OK;

}

int impsurf_target_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "impsurf_target_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	strcpy(impsurf_tgt_name,argv[1]);
	return TCL_OK;
}

int icurve_target_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if(argc != 2)
	{
		interp->result = "icurve_target_cb: Wrong number of arguments";
		return TCL_ERROR;
	}
	strcpy(icurve_tgt_name,argv[1]);
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

/*
 * Gets the name of the program
 */

int get_prog_name_cb(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	strcpy(interp->result,prog_name);
	return TCL_OK;
}
#endif

/******* The main routine *********************************************/

main(argc, argv)	
int argc; char **argv;
{
	int     i;
	FILE	*fp;
#ifndef COMMAND_LINE
	char    tcl_file[128],*tcl_dir;
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
	fprintf(stderr,"intersectCV\n");

	/* read in arguments */

	main_mul = grballoc(Multi);
	Minit(main_mul);
	psurf_mul = grballoc(Multi);
	Minit(psurf_mul);
	pcurve_mul = grballoc(Multi);
	Minit(pcurve_mul);
	impsurf_mul = grballoc(Multi);
	Minit(impsurf_mul);
	icurve_mul = grballoc(Multi);
	Minit(icurve_mul);
	psurf_trans = grballoc(MTrans);
	pcurve_trans = grballoc(MTrans);
	impsurf_trans = grballoc(MTrans);
	icurve_psurf_trans = grballoc(MTrans);

#ifdef CGIVRML
	fprintf(stderr,"intersectCV A\n");
/*	read_cgi();*/
	read_lsmp_xml();

	print_time_message("IntersectCV: done read_cgi");
#else
	map3_args(argc,argv);
#endif
	init_funs();

/* 
fprintf(stderr,"just after init_fun itt is %f\n",
	Mget_opt_val_by_name(main_mul,ITT_NAME));
*/

	/* Initilise the point list program */

#ifndef CGIVRML
	Simp_init();
	Clip_init();
	Intersect_init();
	RIntersect_init();
	UIntersect_init();
	CommentExtract_init();
	Map_init();
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
#ifndef NO_GEOM
		set_geom_base_name(arg_filename);
#endif
		if( intersect_flag ) intersect(0,arg_filename);
		else		     intersect(1,arg_filename);
		if( temp_flag ) unlink(temp_file_name);
		exit(0);
	}

#ifndef CGIVRML
	/* If we don't foreground then the process forks and dies
	   as soon as we do graphics. This is bad.
	 */

	if( arg_filename != NULL)
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
		set_geom_base_name(arg_filename);
	}
	else
		set_geom_base_name(prog_name);


	interp = Tcl_CreateInterp();
/*
	wind = Tk_CreateMainWindow(interp,NULL,prog_name, "intersect");
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

	Tcl_CreateCommand(interp,"load_cb",load_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"save_cb",save_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"update_cb",update_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	
	Tcl_CreateCommand(interp,"draw_special_cb",draw_special_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"load_sphere_cb",load_sphere_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"load_plane_cb",load_plane_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"load_box_cb",load_box_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"run_cb",run_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"run_plus_cb",run_plus_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"run_minus_cb",run_minus_cb,
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

	Tcl_CreateCommand(interp,"pcurve_target_cb",pcurve_target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"psurf_target_cb",psurf_target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"impsurf_target_cb",impsurf_target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp,"icurve_target_cb",icurve_target_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_targets_cb",get_targets_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand(interp,"get_prog_name_cb",get_prog_name_cb,
		(ClientData *) NULL, (Tcl_CmdDeleteProc *) NULL);
/*
fprintf(stderr,"just before use_arg_vals int.tcl  itt is %f\n",
	Mget_opt_val_by_name(main_mul,ITT_NAME));
fprintf(stderr,"just before evaluating int.tcl  itt is %f\n",
	Mget_opt_val_by_name(main_mul,ITT_NAME));
*/
	use_arg_vals();

	tcl_dir = getenv("LSMP_TCL");
	strcpy(tcl_file,tcl_dir);
	strcat(tcl_file,"/intersect.tcl");
	code = Tcl_EvalFile(interp,tcl_file);
	if (code != TCL_OK) {
		fprintf(stderr,"%s\n",interp->result);
		exit(1);
	}

	Tk_MainLoop();

	if( temp_flag ) unlink(temp_file_name);
#endif
}

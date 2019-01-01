/*
 *      file:   psurf.c:   
 *      author: Rich Morris
 *      date:   Jan 6th 1995
 *      
 *	performs psurf on geomview objects defined by a set
 * 	of three equations
 */

#define CGIVRML
#define NO_GEOM
#define COMMAND_LINE
/*
#include <color.h>
#include "normlist.h"
#include "geomsimp.h"
#include <geom.h>
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef CGIVRML
#include <sys/wait.h>
#endif
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <eqn.h>
#include <Multi.h>
#include "../CVcommon.h"
#include "../lsmp.h"

/* Default values */

#define PS_STEPS 15
#define PC_STEPS 52
#define CLIP_DEFAULT 1000.0
#define XMIN_DEFAULT -2.0
#define XMAX_DEFAULT 2.0
#define PREC_DEFAULT 1


/*
#include <device.h>
*/
#ifndef COMMAND_LINE
#include <tcl.h>
#include <tk.h>
#endif

/*
#define DO_TANGENTS
#define PRINT_NORMALS
*/
#define READ_ERRORS
#define OPT_IN_MULTI

#define grballoc(node) (node *) malloc( sizeof(node) )

/* What function the program performs, 4 = 4D coords, N = transform normals,
	C = alter colours  */

enum {Psurf, Pcurve } prog_type;
char	prog_name[10];

Multi	*main_mul;	/* Main mutiple eqn block */
eqn_funs *funlist = NULL;	/* list of functions */

/* default values */

int	world = FALSE;			/* Transform relative to world */
int	command = FALSE;		/* Write geomview comands */
int	quiet = TRUE;			/* quite mode */
int	edit_time = 0;			/* time of last editing */

#define  CLIP(X) (X > clipmax ? clipmax :( X < -clipmax ? -clipmax : ( X != X ? clipmax : X) ))
#define  COL_CLIP(X) (X > 1.0 ? 1.0 :( X < 0.0 ? 0.0 : ( X != X ? 1 : X) ))

#define COPY_STRING(target,source) {\
	target = (char *) calloc( strlen(source)+1,sizeof(char));\
	strcpy(target,source);}

typedef void Geom;
Geom *GeomFLoad(FILE *fp,char *str) {} /* A dummy function as we
						don't link -lgeom */
#ifdef X
#undef X
#undef Y
#undef Z
#undef W
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
int   arg_dimension = 3;
int   arg_colour = -5;
int   arg_normals = -5;
int	arg_xdim = -1, arg_ydim = -1;
double  arg_xmin = HUGE, arg_xmax = HUGE, arg_ymin = HUGE, arg_ymax = HUGE;
double arg_vals[MAX_NUM_PARAMS + MAX_NUM_VARS]; /* vals from arguments */
char   *arg_names[MAX_NUM_PARAMS + MAX_NUM_VARS];
int     arg_count=0;	/* number of params in arguments */
char	*arg_filename = NULL;	/* filename from arguments */
int	temp_flag = FALSE;	/* TRUE if equation def on command line */
char	temp_file_name[L_tmpnam];	/* temp file for equation */
int     vrml_version = 1; /* the version of VRML produced */

print_usage(char *name)
{
	if( !strcmp(prog_name,"pcurve") )
fprintf(stderr,"Usage: pcurve [-4] [-C col] [-v xl xh] [-s xs] [-c clip]\n");
	else
fprintf(stderr,"Usage: psurf [-4] [-C col] [-N norm] [-v xl xh yl yh] [-s xs ys] [-c clip]\n"); 
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

	if( !strcmp(prog_name,"pcurve") )
	{	prog_type = Pcurve;
	}
	else if( !strcmp(prog_name,"psurf") )
	{	prog_type = Psurf;
	}
	else
	{
		fprintf(stderr,"bad program name: %s\n",prog_name);
		exit(-1);
	}

	/* Now we can look at the arguments */
/*
	while((i=getopt(argc,argv,"GIhe:c:p:D:4CNin")) != -1 )
*/
	while((i=getopt(argc,argv,"GIhe:c:p:D:4C:N:v:s:")) != -1 )
	{
		switch(i)
		{
                case '4': arg_dimension = 4; break;
                case 'C':  arg_colour = atoi(optarg); break;
                case 'N':  arg_normals = atoi(optarg); break;
		case 'c': arg_clipmax=atof(optarg); break;
		case 'p': arg_precision = atoi(optarg); break;
		case 'v': 
			arg_xmin = atof(argv[optind++ -1]);
			arg_xmax = atof(argv[optind++ -1]);
			if(prog_type == Psurf)
			{
				arg_ymin = atof(argv[optind++ -1]);
				arg_ymax = atof(argv[optind++ -1]);
			}
			--optind;
			break;
		case 's':
			arg_xdim = atoi(argv[optind++ -1]);
			if(prog_type == Psurf)
				arg_ydim = atoi(argv[optind++ -1]);
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
                        arg_filename = temp_file_name;
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

  if(arg_precision != -1 ) 
	Mset_opt_val_by_name(main_mul,PREC_NAME,(double) arg_precision);
  if(arg_clipmax != 0.0 )
	Mset_opt_val_by_name(main_mul,CLIP_NAME,arg_clipmax);
  if(arg_dimension != 3 )
	Mset_opt_val_by_name(main_mul,DIM_NAME,(double) arg_dimension);
  if(arg_colour != -5)
	Mset_opt_val_by_name(main_mul,COL_NAME,(double) arg_colour);

  if( prog_type == Psurf )
  {
    if(arg_normals != -5)
	Mset_opt_val_by_name(main_mul,NORM_NAME,(double) arg_normals);
    if(arg_xmin != HUGE ) Mset_var_minmax(main_mul,0,arg_xmin,arg_xmax);
    if(arg_ymin != HUGE ) Mset_var_minmax(main_mul,1,arg_ymin,arg_ymax);
    if(arg_xdim != -1) 
		Mset_opt_val_by_name( main_mul,STEPS1_NAME,(double) arg_xdim);
    if(arg_ydim != -1) 
		Mset_opt_val_by_name(main_mul,STEPS2_NAME,(double ) arg_ydim);
  }
  else
  {
    if(arg_xmin != HUGE ) Mset_var_minmax(main_mul,0,arg_xmin,arg_xmax);
    if(arg_xdim != -1) 
	Mset_opt_val_by_name(main_mul,STEPS_NAME,(double) arg_xdim);
  }
	

  for(i=0;i<arg_count;++i)
  {
        Mset_param_val_by_name(main_mul,arg_names[i],arg_vals[i]);
  }
}

read_cgi()
{
        char *env_query;
        int cl,c;
        ncsa_entry entries[MAX_ENTRIES];
        register int x,m=0;
        char *def_ent,*clip_ent,*timeout_ent;
        char *xmin_ent,*xmax_ent,*ymin_ent,*ymax_ent,*xstep_ent,*ystep_ent;
        char *version_ent;
        FILE    *temp_file;
        FILE *fp,*fl;
                char *tmp,*tstr; time_t tim;

        def_ent=clip_ent=timeout_ent=NULL;
        xmin_ent=xmax_ent=ymin_ent=ymax_ent=xstep_ent=ystep_ent=NULL;
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
        else if(!strcmp(entries[x].name,"XSTEP"))   xstep_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"YSTEP"))   ystep_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"CLIPPING"))  clip_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"TIMEOUT")) timeout_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"VERSION")) version_ent = entries[x].val;
        else if(!strcmp(entries[x].name,"PCURVE"))  prog_type = Pcurve;
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

if(prog_type == Psurf)
{
        if(xmin_ent != NULL && xmax_ent != NULL )
                fprintf(temp_file,"x = [%s,%s];\n",xmin_ent,xmax_ent);
        if(ymin_ent != NULL && ymax_ent != NULL )
                fprintf(temp_file,"y = [%s,%s];\n",ymin_ent,ymax_ent);
        if(xstep_ent != NULL )
                fprintf(temp_file,"_%s = %s;\n",STEPS1_NAME,xstep_ent);
        if(ystep_ent != NULL )
                fprintf(temp_file,"_%s = %s;\n",STEPS2_NAME,ystep_ent);
}
else if(prog_type == Pcurve)
{
        if(xmin_ent != NULL && xmax_ent != NULL )
                fprintf(temp_file,"x = [%s,%s];\n",xmin_ent,xmax_ent);
        if(xstep_ent != NULL )
                fprintf(temp_file,"_%s = %s;\n",STEPS_NAME,xstep_ent);
}
        if(clip_ent != NULL )
                fprintf(temp_file,"_%s = %s;\n",CLIP_NAME,clip_ent);


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
 *		Called after psurf3_args
 */

init_funs()
{
	int offset = 0;

	funlist = add_standard_functions(NULL);
        set_input_functions(funlist);
	switch(prog_type)
	{
	case Pcurve:
		MsetNvars(main_mul,1);
		Mset_var_name(main_mul,0,"x");
		Mset_var_minmax(main_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
		Madd_opt(main_mul,PREC_NAME,(double) PREC_DEFAULT);
		Madd_opt(main_mul,STEPS_NAME,(double) PC_STEPS);
		Madd_opt(main_mul,CLIP_NAME,CLIP_DEFAULT);
		Madd_opt(main_mul,DIM_NAME,3.0);
		Madd_opt(main_mul,COL_NAME,(double) NO_COL);
		break;
	case Psurf:
		MsetNvars(main_mul,2);
		Mset_var_name(main_mul,0,"x");
		Mset_var_name(main_mul,1,"y");
		Mset_var_minmax(main_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
		Mset_var_minmax(main_mul,1,XMIN_DEFAULT,XMAX_DEFAULT);
		Madd_opt(main_mul,PREC_NAME,(double) PREC_DEFAULT);
		Madd_opt(main_mul,CLIP_NAME,CLIP_DEFAULT);
		Madd_opt(main_mul,STEPS1_NAME,(double) PS_STEPS);
		Madd_opt(main_mul,STEPS2_NAME,(double) PS_STEPS);
		Madd_opt(main_mul,DIM_NAME,3.0);
		Madd_opt(main_mul,NORM_NAME,(double) STD_NORM);
		Madd_opt(main_mul,COL_NAME,(double) NO_COL);
		break;
	}
}

/************* The guts actually does the work ***********************/


/*
 * Function:	psurf
 * Action;	read in data from file and output it
 */

psurf(char *filename)
{
	FILE	*fp;
	int	i,j,xdim,ydim;
	float	val,clipmax;
	double	xmin,xmax,ymin,ymax,xsize,ysize,dx,dy;
	double	x,y,X,Y,Z,W,L,M,N,R,G,B,A,a,b,c,d,e,f,len;
	int     dimension,normals,colours,fourD_eqns,norm_eqns,col_eqns;
	int	norm_ref;

	double *ptr;

	if( main_mul->error )
	{
		report_error(HEAD_ERROR,"Can't calculate surface - syntax error in equations\n",16);
		eprintf("eprintf: %s\n","whats going wrong");
		fprintf(stderr,"Can't calculate surface - syntax error in equations\n");
		return;
	}

	xmin =  Mget_var_min(main_mul,0);
	xmax =  Mget_var_max(main_mul,0);
	ymin =  Mget_var_min(main_mul,1);
	ymax =  Mget_var_max(main_mul,1);
	xdim = (int) Mget_opt_val_by_name(main_mul,STEPS1_NAME);
	ydim = (int) Mget_opt_val_by_name(main_mul,STEPS2_NAME);
	colours = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
	dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	fourD_eqns = (dimension == 4);
	norm_eqns = (normals == EQN_NORM );
	col_eqns = (colours == EQN_COL );
	if( col_eqns )
		norm_ref = 2; 
	else
		norm_ref = 1;
        xsize = xmax-xmin;
        ysize = ymax-ymin;
        dx = xsize/(xdim-1);
        dy = ysize/(ydim-1);

	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read definition file %s\n",filename);
		return;
	} 
#ifndef NO_GEOM
	start_geom();
#endif
	if(vrml_version == 0)
	{
		printf("LIST\n");
		printf("COMMENT psurf %s {\n",PSDEF_NAME);
		copy_def(fp,stdout);
		fprint_Mopts(stdout,main_mul);
		fclose(fp);
		printf("}\n");
		printf("COMMENT psurf %s { %d }\n",PSETIME_NAME,edit_time);

	/* Now the data */

	        if( col_eqns || colours == STD_COL ) printf("C");
	        if( normals == STD_NORM || norm_eqns ) printf("N");
		if( fourD_eqns ) printf("4");
	        printf("MESH\n");
	        printf("%d %d\n",xdim,ydim);
	}
	else if(vrml_version == 3)
	{
		printf("Content-type: text/plain\n\n");
		printf("OK Surface sucessfully calculated\n");

		print_jvx_header("Parametarised Surface","Parametarised Surface");
		
		copy_def(fp,stdout);
		fprint_Mopts(stdout,main_mul);
		fclose(fp);

		print_jvx_header2("Parametarised Surface");

		printf(" <geometries>\n");
		printf("  <geometry name=\"psurf\">\n");
		printf("   <pointSet dim=\"3\" point=\"hide\">\n");
		printf("    <points>\n");
	}

	/* Now the data */

	if(vrml_version == 0)
	{
	        if( col_eqns || colours == STD_COL ) printf("C");
	        if( normals == STD_NORM || norm_eqns ) printf("N");
		if( fourD_eqns ) printf("4");
	        printf("MESH\n");
	        printf("%d %d\n",xdim,ydim);


	for (j=0, y = ymin; j<ydim; ++j, y += dy)
        {
         for (i=0, x = xmin; i<xdim; ++i, x += dx)
	 {
		/* Set the variables */

		Mset_var_val(main_mul,0,x);
		Mset_var_val(main_mul,1,y);
		MstartEval(main_mul);

		/* Evaluate equations */

		ptr = MevalTop2(main_mul,0);
		val = (float) *ptr; X = CLIP(val); 
		val = (float) *(ptr+1); Y = CLIP(val); 
		val = (float) *(ptr+2); Z = CLIP(val); 
		printf("%f %f %f ",X,Y,Z);
		if( fourD_eqns) 
		{
			val = (float) *(ptr+3); W = CLIP(val); 
			printf("%f ",CLIP(W));
		}

		if( norm_eqns )
		{
			ptr = MevalTop2(main_mul,norm_ref);
			val = (float) *(ptr+0); L = CLIP(val); 
			len = val*val;
			val = (float) *(ptr+1); M = CLIP(val); 
			len += val*val;
			val = (float) *(ptr+2); N = CLIP(val); 
			len += val*val;
			len = sqrt(len);
			if(len == 0 || len != len)
			{
				L = (float) 0.0;
				M = (float) 0.0;
				N = (float) 0.0;
			}
			else if( len == len ) /* Propper numbers */
			{
				L /= len;
				M /= len;
				N /= len;
			}
			printf("%f %f %f ",L,M,N);
		}
		else if( normals ) /* calc normals from main_eqn */
		{
			/* Now calc the normals */
	
			ptr = MevalTopDeriv(main_mul,0,0);
			a = *ptr; b = *(ptr+1); c = *(ptr+2);
			ptr = MevalTopDeriv(main_mul,0,1);
			d = *ptr; e = *(ptr+1); f = *(ptr+2);
	
			L = b * f - c * e;
			M = c * d - a * f;
			N = a * e - b * d;
			len = sqrt(L*L+M*M+N*N);
			
			if(len == 0 || len != len)
			{
				L = (float) 0.0;
				M = (float) 0.0;
				N = (float) 0.0;
			}
			else if( len == len ) /* Propper numbers */
			{
				L /= len;
				M /= len;
				N /= len;
			}
			printf("%f %f %f ",L,M,N);
		}

		if( col_eqns )
		{
			ptr = MevalTop2(main_mul,1);
			val = (float) *(ptr+0); R = COL_CLIP(val); 
			val = (float) *(ptr+1); G = COL_CLIP(val); 
			val = (float) *(ptr+2); B = COL_CLIP(val); 
			val = (float) *(ptr+3); A = COL_CLIP(val); 
			printf("%f %f %f %f",R,G,B,A);
		}
		else if( colours == STD_COL )
		{
			printf(" %f %f %f 1",
				0.0 + 1.0*(x-xmin)/xsize + 0.0*(y-ymin)/ysize,
				0.8 - 0.4*(x-xmin)/xsize - 0.4*(y-ymin)/ysize,
				0.0 + 0.0*(x-xmin)/xsize + 1.0*(y-ymin)/ysize);
                }

		printf("\n");
	 }
	 printf("\n");
	}
}
else if( vrml_version == 3)
{
	for (j=0, y = ymin; j<ydim; ++j, y += dy)
        {
         for (i=0, x = xmin; i<xdim; ++i, x += dx)
	 {
		/* Set the variables */

		Mset_var_val(main_mul,0,x);
		Mset_var_val(main_mul,1,y);
		MstartEval(main_mul);

		/* Evaluate equations */

		ptr = MevalTop2(main_mul,0);
		val = (float) *ptr; X = CLIP(val); 
		val = (float) *(ptr+1); Y = CLIP(val); 
		val = (float) *(ptr+2); Z = CLIP(val); 
		printf("<p>%f %f %f</p>\n",X,Y,Z);
	 }
	}
printf("    </points>\n");

	if( norm_eqns || normals )
	{
printf("    <normals>\n");

	for (j=0, y = ymin; j<ydim; ++j, y += dy)
        {
         for (i=0, x = xmin; i<xdim; ++i, x += dx)
	 {
		/* Set the variables */

		Mset_var_val(main_mul,0,x);
		Mset_var_val(main_mul,1,y);
		MstartEval(main_mul);

		/* Evaluate equations */

		ptr = MevalTop2(main_mul,0);
		if( norm_eqns )
		{
			ptr = MevalTop2(main_mul,norm_ref);
			val = (float) *(ptr+0); L = CLIP(val); 
			len = val*val;
			val = (float) *(ptr+1); M = CLIP(val); 
			len += val*val;
			val = (float) *(ptr+2); N = CLIP(val); 
			len += val*val;
			len = sqrt(len);
			if(len == 0 || len != len)
			{
				L = (float) 0.0;
				M = (float) 0.0;
				N = (float) 0.0;
			}
			else if( len == len ) /* Propper numbers */
			{
				L /= len;
				M /= len;
				N /= len;
			}
			printf("<n>%f %f %f</n>\n",L,M,N);
		}
		else if( normals ) /* calc normals from main_eqn */
		{
			/* Now calc the normals */
	
			ptr = MevalTopDeriv(main_mul,0,0);
			a = *ptr; b = *(ptr+1); c = *(ptr+2);
			ptr = MevalTopDeriv(main_mul,0,1);
			d = *ptr; e = *(ptr+1); f = *(ptr+2);
	
			L = b * f - c * e;
			M = c * d - a * f;
			N = a * e - b * d;
			len = sqrt(L*L+M*M+N*N);
			
			if(len == 0 || len != len)
			{
				L = (float) 0.0;
				M = (float) 0.0;
				N = (float) 0.0;
			}
			else if( len == len ) /* Propper numbers */
			{
				L /= len;
				M /= len;
				N /= len;
			}
			printf("<n>%f %f %f</n>\n",L,M,N);
		}
	 }
	} /* end for loop */
printf("    </normals>\n");
	} /* end if normal of some sort */

#ifdef NOTDEF
		if( col_eqns )
		{
			ptr = MevalTop2(main_mul,1);
			val = (float) *(ptr+0); R = COL_CLIP(val); 
			val = (float) *(ptr+1); G = COL_CLIP(val); 
			val = (float) *(ptr+2); B = COL_CLIP(val); 
			val = (float) *(ptr+3); A = COL_CLIP(val); 
			printf("%f %f %f %f",R,G,B,A);
		}
		else if( colours == STD_COL )
		{
			printf(" %f %f %f 1",
				0.0 + 1.0*(x-xmin)/xsize + 0.0*(y-ymin)/ysize,
				0.8 - 0.4*(x-xmin)/xsize - 0.4*(y-ymin)/ysize,
				0.0 + 0.0*(x-xmin)/xsize + 1.0*(y-ymin)/ysize);
                }

#endif
#define VERT_IND(i,j) ((i) + (j) * xdim)
printf("   </pointSet>\n");
printf("   <faceSet face=\"show\" edge=\"hide\">\n");
printf("    <faces>\n");
	for (j=0; j<ydim-1; ++j)
        {
         for (i=0; i<xdim-1; ++i)
	 {
printf("<f>%d %d %d %d</f>\n",
	VERT_IND(i,j),
	VERT_IND(i,j+1),
	VERT_IND(i+1,j+1),
	VERT_IND(i+1,j));
	 }
	}
printf("    </faces>\n");
printf("   </faceSet>\n");
printf("  </geometry>\n");
printf(" </geometries>\n");
printf("</jvx-model>\n");

	 } /* end vrml_ver == 3 */
#ifndef NO_GEOM
	fini_geom();
#endif
}

pcurve(char *filename)
{
	FILE	*fp;
	int	i,n_cols,xdim;
	float	val,xsize,dx,xmin,xmax,clipmax;
	double	x,y,X,Y,Z,W,L,M,N,R,G,B,A,a,b,c,d,e,f,len;
	int     dimension,colours,fourD_eqns,col_eqns;

	double *ptr;

	if( main_mul->error )
	{
		report_error(HEAD_ERROR,"Can't calculate curves - syntax error in equations\n",16);
		fprintf(stderr,"Can't calculate curves - syntax error in equations\n");
		return;
	}
 

        xmin = (float) Mget_var_min(main_mul,0);
        xmax = (float) Mget_var_max(main_mul,0);
        xdim = (int) Mget_opt_val_by_name(main_mul,STEPS_NAME);
 	colours = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
	dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
	clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	fourD_eqns = (dimension == 4);
	col_eqns = (colours == EQN_COL );

        xsize = xmax-xmin;
        dx = xsize/(xdim-1);

#ifdef NOT_DEF
	printf("LIST\n");
	printf("COMMENT pcurve");
	if(fourD_eqns) printf("4");
	if(col_eqns) printf("C");
	printf(" LSMP {\n");
	printf("# Parametrized curve \n");
        if(col_eqns ) printf("# with equations for colours\n");
        printf("define by:\n");
        fprint_multi(stdout,main_mul);
        printf("}\n");
#endif
	fp = fopen(filename,"r");
	if(fp == NULL )
	{
		fprintf(stderr,"Could not read definition file %s\n",filename);
		return;
	} 
	
#ifndef NO_GEOM
	start_geom();
#endif

	if(vrml_version == 0)
	{
	printf("LIST\n");
	printf("COMMENT pcurve %s {\n",PSDEF_NAME);
	copy_def(fp,stdout);
	fprint_Mopts(stdout,main_mul);
	fclose(fp);
	printf("}\n");
	printf("COMMENT pcurve %s { %d }\n",PSETIME_NAME,edit_time);

	/* Now the data */

	if( col_eqns ) n_cols = xdim;
	else if( colours != NO_COL) n_cols = 1;
	else n_cols = 0;

	if( fourD_eqns ) printf("4");
        printf("VECT\n");
        printf("1 %d %d\n",xdim,n_cols);

	printf("%d\n",xdim);	/* number of verticies in first line */
	printf("%d\n",n_cols);	/* number of colours in first line */
	}
	else if(vrml_version == 3)
	{
		                                printf("Content-type: text/plain\n\n");
                                printf("OK Surface sucessfully calculated\n");

	print_jvx_header("psurf","Automatically generated parameterised surface");
        copy_def(fp,stdout);
        fprint_Mopts(stdout,main_mul);
        fclose(fp);
	print_jvx_header2("Parametrised surface");
	
printf(" <geometries>\n");
printf("  <geometry name=\"pcurve\">\n");
printf("   <pointSet dim=\"3\" point=\"hide\">\n");
printf("    <points>\n");
	}


        for (i=0, x = xmin; i<xdim; ++i, x += dx)
	{
		/* Set the variables */

                Mset_var_val(main_mul,0,x);
                MstartEval(main_mul);

		/* Evaluate equations */

                ptr = MevalTop2(main_mul,0);
		val = (float) *ptr; X = CLIP(val); 
		val = (float) *(ptr+1); Y = CLIP(val); 
		val = (float) *(ptr+2); Z = CLIP(val); 
		printf("<p>%f %f %f",X,Y,Z);
		if( fourD_eqns) 
		{
			val = (float) *(ptr+3); W = CLIP(val); 
			printf(" %f",CLIP(W));
		}
		printf("</p>\n");
	}
	printf("\n");
printf("    </points>\n");
printf("   </pointSet>\n");

#ifdef NOT_DEF
	if( col_eqns )
	{
        	for (i=0, x = xmin; i<xdim; ++i, x += dx)
		{
                Mset_var_val(main_mul,0,x);
                MstartEval(main_mul);

                /* Evaluate equations */

               	ptr = MevalTop2(main_mul,1);
		val = (float) *(ptr+0); R = COL_CLIP(val); 
		val = (float) *(ptr+1); G = COL_CLIP(val); 
		val = (float) *(ptr+2); B = COL_CLIP(val); 
		val = (float) *(ptr+3); A = COL_CLIP(val); 
		printf("%f %f %f %f\n",R,G,B,A);
		}
		printf("\n");
	 }
	else if( colours  != NO_COL)
	{
	        switch(colours)
        {
                case 0: /* Black   */ printf("0 0 0 1\n"); break;
                case 1: /* Red     */ printf("1 0 0 1\n"); break;
                case 2: /* Green   */ printf("0 1 0 1\n"); break;
                case 3: /* Yellow  */ printf("1 1 0 1\n"); break;
                case 4: /* Blue    */ printf("0 0 1 1\n"); break;
                case 5: /* Magenta */ printf("1 0 1 1\n"); break;
                case 6: /* Cyan    */ printf("0 1 1 1\n"); break;
                case 7: /* White   */ printf("1 1 1 1\n"); break;
        }
    
	 printf("\n");
	}
#endif
	if(vrml_version == 3)
	{
		
printf("   <lineSet>\n");
printf("    <lines>\n");
printf("     <l>");
for(i=0;i<xdim;++i) printf("%d ",i);
printf("</l>\n");
printf("    </lines>\n");
printf("   </lineSet>\n");
printf("  </geometry>\n");
printf(" </geometries>\n");
printf("</jvx-model>\n");

	}

#ifndef NO_GEOM
	fini_geom();
#endif
}

/********* file handeling ***********************/

/* Read in the equ block from a file,
 * either from the editor or via the Load menu
 */

int read_def(FILE *fp)
{
        int     dimension,normals,colours,fourD_eqns,norm_eqns,col_eqns;

	edit_time = 0;
	if( !fscanMulti(fp,main_mul) ) 
	{
		report_error(HEAD_ERROR,"Parsing equations failed",401);
		return(FALSE);
	}
	if( !MfindOpts(main_mul) )
	{
		report_error(HEAD_ERROR,"Finding options in equation failed",402);
		return(FALSE);
	}
	eprintf("read_def: parse OK\n");

	switch(prog_type)
	{
	case Pcurve:
		dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
		colours = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
		col_eqns = ( colours == EQN_COL );
		if( col_eqns )
		{
			 MsetNtop(main_mul,2);
			 MsetTopDim(main_mul,1,4);
		}
		else     MsetNtop(main_mul,1);
		if( dimension == 4 )
			MsetTopDim(main_mul,0,4);
		else
			MsetTopDim(main_mul,0,3);
		break;
	case Psurf:
		dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
		colours = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
		normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
		norm_eqns = ( normals == EQN_NORM );
		col_eqns = ( colours == EQN_COL );
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
		if( dimension == 4 )
			MsetTopDim(main_mul,0,4);
		else
			MsetTopDim(main_mul,0,3);
		break;
	}

	if( !McombineTop(main_mul) ) return(FALSE);
	if( !MfindNames(main_mul) ) return(FALSE);
	
	switch(prog_type)
	{
	case Pcurve:
			MsetNtopDerivs(main_mul,0,0);
		break;
	case Psurf:
		if( normals == STD_NORM ) /* we'll need derivatives */
		{
			MsetNtopDerivs(main_mul,0,2);
			MsetDerivName(main_mul,0,0,Mget_var_name(main_mul,0));
			MsetDerivName(main_mul,0,1,Mget_var_name(main_mul,1));
		}
		else
			MsetNtopDerivs(main_mul,0,0);
		break;
	}

	if( !McheckDims(main_mul) ) return(FALSE);
	if( !McalcDerivs(main_mul) ) return(FALSE);
/*
dump_multi(stderr,main_mul);
*/
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
	Mclear(main_mul);
	switch(prog_type)
	{
	case Pcurve:
		MsetNvars(main_mul,1);
		Mset_var_name(main_mul,0,"x");
		Mset_var_minmax(main_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
		Mset_opt_val_by_name(main_mul,PREC_NAME,(double) PREC_DEFAULT);
		Mset_opt_val_by_name(main_mul,STEPS_NAME,(double) PC_STEPS);
		Mset_opt_val_by_name(main_mul,CLIP_NAME,CLIP_DEFAULT);
		Mset_opt_val_by_name(main_mul,DIM_NAME,3.0);
		Mset_opt_val_by_name(main_mul,COL_NAME,(double) NO_COL);
		break;
	case Psurf:
		MsetNvars(main_mul,2);
		Mset_var_name(main_mul,0,"x");
		Mset_var_name(main_mul,1,"y");
		Mset_var_minmax(main_mul,0,XMIN_DEFAULT,XMAX_DEFAULT);
		Mset_var_minmax(main_mul,1,XMIN_DEFAULT,XMAX_DEFAULT);
		Mset_opt_val_by_name(main_mul,PREC_NAME,(double) PREC_DEFAULT);
		Mset_opt_val_by_name(main_mul,STEPS1_NAME,(double) PS_STEPS);
		Mset_opt_val_by_name(main_mul,STEPS2_NAME,(double) PS_STEPS);
		Mset_opt_val_by_name(main_mul,CLIP_NAME,CLIP_DEFAULT);
		Mset_opt_val_by_name(main_mul,DIM_NAME,3.0);
		Mset_opt_val_by_name(main_mul,COL_NAME,(double) NO_COL);
		Mset_opt_val_by_name(main_mul,NORM_NAME,(double) STD_NORM);
		break;
	}
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
	fprint_Mopts(fp,main_mul);
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
	if(prog_type == Psurf) psurf(argv[1]);
	else		pcurve(argv[1]);
	return TCL_OK;
}

int get_progname(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if( prog_type == Psurf )
        	sprintf(interp->result,"%s %d %s",
			prog_name,2,temp_file_name);
	else
        	sprintf(interp->result,"%s %d %s",
			prog_name,1,temp_file_name);
        return TCL_OK;
}

int get_options(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
        int  precision,normals,colour,dimension;
	float clipmax;
        colour = (int) Mget_opt_val_by_name(main_mul,COL_NAME);
        precision = (int) Mget_opt_val_by_name(main_mul,PREC_NAME);
        dimension = (int) Mget_opt_val_by_name(main_mul,DIM_NAME);
        clipmax = (float) Mget_opt_val_by_name(main_mul,CLIP_NAME);
	if( prog_type == Psurf )
        	normals = (int) Mget_opt_val_by_name(main_mul,NORM_NAME);
	else
		normals = 0;
 
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
	if( prog_type == Psurf )
		Mset_opt_val_by_name(main_mul,NORM_NAME, atof(argv[3]) );
	Mset_opt_val_by_name(main_mul,COL_NAME,atof(argv[4]) );
	Mset_opt_val_by_name(main_mul,DIM_NAME,atof(argv[5]) );
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


        if( prog_type == Pcurve  )
        {
                if( argc != 4)
                {
                interp->result = "set_variables: Wrong number of arguments";
                return TCL_ERROR;
                }
                Mset_var_minmax(main_mul,0,atof(argv[1]), atof(argv[2]));
                Mset_opt_val_by_name(main_mul,STEPS_NAME,atof(argv[3]));
        }
        else
        {
                if( argc != 7)
                {
                interp->result = "set_variables: Wrong number of arguments";
                return TCL_ERROR;
                }
                Mset_var_minmax(main_mul,0,atof(argv[1]), atof(argv[2]));
                Mset_opt_val_by_name(main_mul,STEPS1_NAME,atof(argv[3]));
                Mset_var_minmax(main_mul,1,atof(argv[4]), atof(argv[5]));
                Mset_opt_val_by_name(main_mul,STEPS2_NAME,atof(argv[6]));
        }
   
        return TCL_OK;
}

int get_variables(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	if( prog_type == Pcurve )
        {
                sprintf(interp->result,"%s %f %f %d\n",
                        Mget_var_name(main_mul,0),
                        Mget_var_min(main_mul,0),
                        Mget_var_max(main_mul,0),
                        (int) Mget_opt_val_by_name(main_mul,STEPS_NAME));
        }
        else
        {
                sprintf(interp->result,"%s %f %f %d %s %f %f %d\n",
                        Mget_var_name(main_mul,0),
                        Mget_var_min(main_mul,0),
                        Mget_var_max(main_mul,0),
                        (int) Mget_opt_val_by_name(main_mul,STEPS1_NAME),
                        Mget_var_name(main_mul,1),
                        Mget_var_min(main_mul,1),
                        Mget_var_max(main_mul,1),
                        (int) Mget_opt_val_by_name(main_mul,STEPS2_NAME));
        }
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
#endif

/******* The main routine *********************************************/

main(argc, argv)        
int argc; char **argv;
{
	char	string[5],tcl_file[128],*tcl_dir;
	long	dev;
	short	val;
	int	i;
	FILE	*fp;
	char *slash;
        int code;
        FILE *fo;
#ifndef COMMAND_LINE
        Tcl_Interp *interp;
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
        tmpnam(temp_file_name);
fprintf(stderr,"argv0 %s\n",argv[0]);
	main_mul = grballoc(Multi);
	Minit(main_mul);
#ifdef CGIVRML
	if( strstr(argv[0],"pcurveCV") != NULL ||
		( argv[1] != NULL && !strcmp(argv[1],"pcurveCV") ) )
        {       prog_type = Pcurve;
fprintf(stderr,"pcurve hello2\n");
fflush(stderr);
        }
        else
        {       prog_type = Psurf;
        }
 
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
#ifndef NO_GEOM
                set_geom_name(arg_filename);
#endif
		if(prog_type == Psurf) psurf(arg_filename);
		else		pcurve(arg_filename);

		if( temp_flag ) unlink(temp_file_name);
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
                read_file(fp);
		fclose(fp);
		use_arg_vals();
                set_geom_name(arg_filename);
        }
	else
		set_geom_name(prog_name);

        interp = Tcl_CreateInterp();
/*
        wind = Tk_CreateMainWindow(interp,NULL,prog_name, "psurf");
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
        strcat(tcl_file,"/psurf.tcl");
        code = Tcl_EvalFile(interp,tcl_file);
 
        if (code != TCL_OK) {
		fprintf(stderr,"%s\n",interp->result);
                exit(1);
        }

        Tk_MainLoop();

	unlink(temp_file_name);
#endif
}

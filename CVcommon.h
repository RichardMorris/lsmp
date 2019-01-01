/* defs for nsca support routines */

#ifndef CVcommon_h
#define CVcommon_h

#define MAX_ENTRIES 100

typedef struct {
    char *name;
    char *val;
} ncsa_entry;

#define HEAD_ERROR 1
#define MIDDLE_ERROR 2
#define NON_FATAL_ERROR 3
#define NO_REP_HEAD_ERROR 4
#define NO_REP_MID_ERROR 5
#define HEAD_WARNING 6

void getword(char *word, char *line, char stop);
char *makeword(char *line, char stop);
char *fmakeword(FILE *f, char stop, int *len);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);

extern void report_error(int code,char *str,int errcode);
extern void report_error2(int code,char *format,char *str,int errcode);
extern void print_time_message(char *str);

/** typedefs for elements used in callback functions. **/

typedef struct { double x; double y; double z; double w; } HPoint3;
typedef struct { double x; double y; double z; } Point3;
typedef struct { double r; double g; double b; double a; } ColorA;
typedef struct Vertex
{
	HPoint3	pt;
	ColorA	vcol;
	Point3	vn;
	float st[2];
}  Vertex;

typedef struct Poly
{
	int	n_vertices;
	Vertex	**v;
	ColorA  pcol;
	Point3	pn;
}  Poly;

typedef struct PolyList
{
	int	n_polys;
	int	n_verts;
	Poly	*p;
	Vertex	*vl;
	int	flags;
	int	seq;		/* for 4D->3D tforms */
#  define	  PL_HASVN	0x1	/* Per-vertex normals (vn) valid */
#  define	  PL_HASPN	0x2	/* Per-polygon normals (pn) valid */
#  define	  PL_HASVCOL	0x4	/* Per-vertex colors (vcol) valid */
#  define	  PL_HASPCOL	0x8	/* Per-polygon colors (pcol) valid */
#  define	  PL_EVNORM	0x10	/* Normals are everted */
#  define	  PL_HASST	0x20	/* Has s,t texture coords */
			/* For 4-D points, see geomflags & VERT_4D */
} PolyList;

typedef struct LsmpOption
{
	char *name;
	char *type;
	char *value;
	int  i_value;
	double d_value;
} LsmpOption;

typedef struct
{
	char *name;
	char *type;
	double min,max;
	double value;
} LsmpVariable;

typedef struct
{
	char *name;
	char *type;
	double value;
} LsmpParameter;

#include "jvx/jvx.h"
#include "jvx/jvxCore.h"

#define MAX_LSMP_OPTS 20
#define MAX_LSMP_VARS 20
#define MAX_LSMP_PARAMS 100
#define LSMP_ALLOC_CHUNK 1024
typedef struct
{
	char *def;
	char *name;
	char *type;
	char *opType;
	int n_options;
	LsmpOption *options[MAX_LSMP_OPTS];
	int n_variables;
	LsmpVariable *variables[MAX_LSMP_VARS];
	int n_parameters;
	LsmpParameter *parameters[MAX_LSMP_PARAMS];
	int alloc_size;
	char *data;
}  LsmpDef;

typedef struct
{
	LsmpDef *Def;
	LsmpDef *auxDef;
	LsmpDef *auxDef2;
	xml_tree *jvx;
	int alloc_data;
	char *data;
} LsmpInputSpec;

#define OOGLNewNE(type,num,msg) (type *) malloc(sizeof(type)*num)
#define OOGLNewN(type,num) (type *) malloc(sizeof(type)*num)
#define OOGLNew(type) (type *) malloc(sizeof(type))
#define OOGLFree(obj) free(obj)

extern void print_jvx_header(char *title,char *abstract);
extern void print_jvx_header2(char *keyword);
extern void print_jvx_tail();

extern void HPt3Copy(HPoint3 *sol,HPoint3 *res);
extern void HPt3LinSum(double lambda,HPoint3 *low,double mu,HPoint3 *high,HPoint3 *sol);
extern void Pt3Comb(double lambda,Point3 *low,double mu,Point3 *high,Point3 *sol);

extern LsmpInputSpec *readInputSpec(FILE *fp,int *content_length,FILE *fl);
extern LsmpOption *getLsmpOption(LsmpDef *def,char *name);

#endif




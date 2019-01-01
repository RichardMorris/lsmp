#include "../CVcommon.h"

#define ALLOC_INC 64
#define MAX_NUM_RSOLS 20
#define MAX_NUM_REDGES 20

extern double	tolerance;
extern	int	max_itt;
extern	int	global_cols;

#define DOT3D(A,B) ((A).x * (B).x + (A).y * (B).y +(A).z * (B).z )
#define COPY3D(A,B) {(B).x = (A).x; (B).y = (A).y; (B).z = (A).z; }
#define NEGATE3D(A) {(A).x = - (A).x; (A).y = - (A).y; (A).z = - (A).z; }
#define grballoc(node) (node *) malloc( sizeof(node) )
#define CoSet(col,R,G,B) { col.r = R; col.g = G; col.b = B; col.a = 1.0; }

/* A point: x,y its pd's etc */

typedef struct Ipoint
{
	double x,y,z,w;
	double val;
	struct Ipoint *next;
	short 	done;
} Ipoint;

/* an edge: HPoint3 for location */

typedef struct Iedgestr
{
	Ipoint *pt1,*pt2,*pt3;
	struct Iedgestr *next;
} Iedge;

/* We have an ever growing list of verticies */

extern HPoint3 *Iverts;
extern int	Ivert_cursize,Ivert_allocsize;
extern Iedge	*Iedgebase;

extern Ipoint *Iconverge(Ipoint *pt1, Ipoint *pt2,double (*fun)());
extern Ipoint *Iconv(Ipoint *pt1, Ipoint *pt2,int itts,double (*fun)());
extern Iedge *addIedge(Ipoint *pt1, Ipoint *pt2, Ipoint *pt3);
extern Iedge *getIedge(Ipoint *A,Ipoint *B);
extern addIpoint(Ipoint *A);
extern addIline(Ipoint *A, Ipoint *B);

extern PolyList *jvx2PolyList(xml_tree *root);










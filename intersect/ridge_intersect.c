/*
 * File:	ridge_intersect.c
 * Function:	intersects a geometry using ridge type criteria
 * Author;	Richard Morris
 * Date		22/4/93
 */

#define NO_GEOM
#include <eqn.h>
#include <stdio.h>
#ifdef NO_GEOM
#include "../CVcommon.h"
#else
#include <geom.h>
#include <ooglutil.h>
#include <point3.h>
#include <hpoint3.h>
#include <polylistP.h>
#include <vectP.h>
#include <stdarg.h>
#include <geomclass.h>
#include <listP.h>
#include <list.h>
#endif
#include <math.h>
#include "../lsmp.h"
#include "../jvx/jvx.h"
#include "../jvx/jvxCore.h"
#include "intersect.h"

/*
#define PRI_INFIN
#define PRINT_VALS
#define PRINT_ERRS
*/
#define ALLOC_INC 64
#define MAX_NUM_RSOLS 20
#define MAX_NUM_REDGES 20

extern double	tolerance;
extern	int	max_itt;
extern	int	global_cols;
extern	double calc_given_pq(),calc_nearest_pq();

#define DOT2D(A,B) ((A).x * (B).x + (A).y * (B).y )
#define COPY2D(A,B) {(B).x = (A).x; (B).y = (A).y; }
#define NEGATE2D(A) {(A).x = - (A).x; (A).y = - (A).y; }
#define grballoc(node) (node *) malloc( sizeof(node) )
#define CoSet(col,R,G,B) { col.r = R; col.g = G; col.b = B; col.a = 1.0; }

/* A point: x,y its pd's etc */

typedef struct Rpoint
{
	double x,y,z;
	VEC2D p,q;
	double val;
	struct Rpoint *next;
	short 	done;
} Rpoint;

/* an edge: HPoint3 for location */

typedef struct Redgestr
{
	Rpoint *pt1,*pt2,*pt3;
	struct Redgestr *next;
} Redge;

/* We have an ever growing list of verticies */

HPoint3 *Rverts;
int	Rvert_cursize,Rvert_allocsize;
Redge	*Redgebase;

addRline(Rpoint *A, Rpoint *B)
{
	HPoint3 *pt;

	if(Rvert_cursize == Rvert_allocsize)
	{
		Rvert_allocsize += ALLOC_INC;
		Rverts = (HPoint3 *) realloc((void *) Rverts,
				sizeof(HPoint3) * Rvert_allocsize );
	}
	if(Rverts == NULL)
	{
		fprintf(stderr,"Out of space for verticies\n");
		return;
	}
	pt = Rverts + Rvert_cursize;
	pt->x = A->x; pt->y = A->y; pt->z = A->z; pt->w = 1.0;
	++Rvert_cursize;
	pt = Rverts + Rvert_cursize;
	pt->x = B->x; pt->y = B->y; pt->z = B->z; pt->w = 1.0;
	++Rvert_cursize;
}

/* now find an edge */

Redge *getRedge(Rpoint *A,Rpoint *B)
{
	Redge *edge;

	edge = Redgebase;
	while( edge != NULL )
	{
		if( (edge->pt1 == A && edge->pt2 == B )
		 || (edge->pt1 == B && edge->pt2 == A ) )
			return(edge);
		edge = edge->next;
	}
	return(NULL);
}

Redge *addRedge(Rpoint *pt1, Rpoint *pt2, Rpoint *pt3)
{
	Redge *edge;

	edge = grballoc(Redge);
	edge->pt1 = pt1;
	edge->pt2 = pt2;
	edge->pt3 = pt3;
	edge->next = Redgebase;
	Redgebase = edge;
	return(Redgebase);
}

/*********** Now the convergence routines *******************/

Rpoint *Rconv(Rpoint *pt1, Rpoint *pt2,int itts)
{
	double Ax,Ay,Az,Aval,Bx,By,Bz,Bval,Cx,Cy,Cz,Cval;
	Rpoint *pt3,*pt4,*pt5;
	VEC2D	Ap,Aq,Bp,Bq,Cp,Cq;

	/* first check coherent orientations */

	if( pt1->done && pt2->done )
	{
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) 
		 || (pt1->q.x == 0.0 && pt1->q.y == 0.0) 
		 || (pt2->p.x == 0.0 && pt2->p.y == 0.0) 
		 || (pt2->q.x == 0.0 && pt2->q.y == 0.0) 
		 || pt1->p.x != pt1->p.x || pt1->p.y != pt1->p.y
		 || pt1->q.x != pt1->q.x || pt1->q.y != pt1->q.y 
		 || pt2->p.x != pt2->p.x || pt2->p.y != pt2->p.y
		 || pt2->q.x != pt2->q.x || pt2->q.y != pt2->q.y )
			return(NULL);

		if( DOT2D(pt1->p,pt2->p) > 0.0 && DOT2D(pt1->q,pt2->q) > 0.0 )
		{
		}
		else if( DOT2D(pt1->p,pt2->p) > 0.0 ) /* i.e q.q < 0 */
		{
			NEGATE2D(pt2->q);
			pt2->val = calc_given_pq(pt2->x,pt2->y,
				pt2->p,pt2->q);
		}
		else if( DOT2D(pt1->p,pt2->p) > 0.0 ) /* i.e q.q < 0 */
		{
			NEGATE2D(pt2->p);
			pt2->val = calc_given_pq(pt2->x,pt2->y,
				pt2->p,pt2->q);
		}
		else
		{
			NEGATE2D(pt2->p);
			NEGATE2D(pt2->q);
			pt2->val = calc_given_pq(pt2->x,pt2->y,
				pt2->p,pt2->q);
		}
	}
	else if( pt2->done )
	{
		if( (pt2->p.x == 0.0 && pt2->p.y == 0.0) 
		 || (pt2->q.x == 0.0 && pt2->q.y == 0.0) 
		 || pt2->p.x != pt2->p.x || pt2->p.y != pt2->p.y
		 || pt2->q.x != pt2->q.x || pt2->q.y != pt2->q.y )
			return(NULL);
		COPY2D(pt2->p,pt1->p);
		COPY2D(pt2->q,pt1->q);
		pt1->val = calc_nearest_pq(pt1->x,pt1->y,&(pt1->p),&(pt1->q));
		pt1->done = TRUE;
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) 
		 || (pt1->q.x == 0.0 && pt1->q.y == 0.0) 
		 || pt1->p.x != pt1->p.x || pt1->p.y != pt1->p.y
		 || pt1->q.x != pt1->q.x || pt1->q.y != pt1->q.y )
			return(NULL);
	}
	else if( pt1->done )
	{
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) 
		 || (pt1->q.x == 0.0 && pt1->q.y == 0.0) 
		 || pt1->p.x != pt1->p.x || pt1->p.y != pt1->p.y
		 || pt1->q.x != pt1->q.x || pt1->q.y != pt1->q.y )
			return(NULL);
		COPY2D(pt1->p,pt2->p);
		COPY2D(pt1->q,pt2->q);
		pt2->val = calc_nearest_pq(pt2->x,pt2->y,&(pt2->p),&(pt2->q));
		pt2->done = TRUE;
		if( (pt2->p.x == 0.0 && pt2->p.y == 0.0) 
		 || (pt2->q.x == 0.0 && pt2->q.y == 0.0) 
		 || pt2->p.x != pt2->p.x || pt2->p.y != pt2->p.y
		 || pt2->q.x != pt2->q.x || pt2->q.y != pt2->q.y )
			return(NULL);
	}
	else	/* neither is done */
	{
		pt1->p.x = 1.0; pt1->p.y = 0.0;
		pt1->q.x = 0.0; pt1->q.y = 1.0;
		pt1->val = calc_nearest_pq(pt1->x,pt1->y,&(pt1->p),&(pt1->q));
		pt1->done = TRUE;
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) 
		 || (pt1->q.x == 0.0 && pt1->q.y == 0.0) 
		 || pt1->p.x != pt1->p.x || pt1->p.y != pt1->p.y
		 || pt1->q.x != pt1->q.x || pt1->q.y != pt1->q.y )
			return(NULL);
		COPY2D(pt1->p,pt2->p);
		COPY2D(pt1->q,pt2->q);
		pt2->val = calc_nearest_pq(pt2->x,pt2->y,&(pt2->p),&(pt2->q));
		pt2->done = TRUE;
		if( (pt2->p.x == 0.0 && pt2->p.y == 0.0) 
		 || (pt2->q.x == 0.0 && pt2->q.y == 0.0) 
		 || pt2->p.x != pt2->p.x || pt2->p.y != pt2->p.y
		 || pt2->q.x != pt2->q.x || pt2->q.y != pt2->q.y )
			return(NULL);
	}
/*
fprintf(stderr,"\nRconv\n");
fprintf(stderr,"pt1 (%f,%f) p (%f,%f) q (%f,%f) val %f done %d\n",
		pt1->x,pt1->y,pt1->p.x,pt1->p.y,pt1->q.x,pt1->q.y,
		pt1->val,pt1->done);
fprintf(stderr,"pt2 (%f,%f) p (%f,%f) q (%f,%f) val %f done %d\n",
		pt2->x,pt2->y,pt2->p.x,pt2->p.y,pt2->q.x,pt2->q.y,
		pt2->val,pt2->done);
*/

	/* Now have coherent orientations */

	if( pt1->val == 0.0 )
	{
			pt3 = grballoc(Rpoint);
			pt3->x = pt1->x;
			pt3->y = pt1->y;
			pt3->z = pt1->z;
			COPY2D(pt1->p,pt3->p);
			COPY2D(pt1->q,pt3->q);
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
	}
	if( pt2->val == 0.0 )
	{
			pt3 = grballoc(Rpoint);
			pt3->x = pt2->x;
			pt3->y = pt2->y;
			pt3->z = pt2->z;
			COPY2D(pt2->p,pt3->p);
			COPY2D(pt2->q,pt3->q);
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
	}
	
	if( (pt1->val > 0.0 && pt2->val > 0.0 ) 
	 || (pt1->val < 0.0 && pt2->val < 0.0 ) )
		return(NULL);

	/* now converge */

	Ax = pt1->x; Ay = pt1->y; Az = pt1->z; 
	Bx = pt2->x; By = pt2->y; Bz = pt2->z;
	COPY2D(pt1->p,Ap); COPY2D(pt1->q,Aq);
	COPY2D(pt2->p,Bp); COPY2D(pt2->q,Bq);
	Aval = pt1->val; Bval = pt2->val;

	while(1)
	{
		Cx = (Ax + Bx)/2; Cy = (Ay + By)/2; Cz = (Az + Bz)/2;
		COPY2D(Ap,Cp);
		COPY2D(Aq,Cq);
		Cval = calc_nearest_pq(Cx,Cy,&Cp,&Cq);
		if( (Cp.x == 0.0 && Cp.y == 0.0) || Cp.x!=Cp.x || Cp.y!=Cp.y 
		 || (Cq.x == 0.0 && Cq.y == 0.0) || Cq.x!=Cq.x || Cq.y!=Cq.y )
			return(NULL);
/*
fprintf(stderr,"Bisect (%f,%f) p (%f,%f) q (%f,%f) val %f\n",
		Cx,Cy,Cp.x,Cp.y,Cq.x,Cq.y,Cval);		
*/
		if( ++itts > max_itt )
		{
			pt3 = grballoc(Rpoint);
			pt3->x = Cx;
			pt3->y = Cy;
			pt3->z = Cz;
			COPY2D(Cp,pt3->p);
			COPY2D(Cq,pt3->q);
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
		}

		/* Now check things are sensible */

		if( DOT2D(Cp,Ap) <= 0.0 || DOT2D(Cq,Aq) <= 0.0 ||
		    DOT2D(Cp,Bp) <= 0.0 || DOT2D(Cq,Bq) <= 0.0 )
		{
/*
fprintf(stderr,"wierdness A (%f,%f) B (%f,%f) C (%f,%f) itts %d\n",
	Ax,Ay,Bx,By,Cx,Cy,itts);
fprintf(stderr,"Ap (%f,%f) Aq (%f,%f), Bp (%f,%f) Bq (%f,%f) Cp (%f,%f) Cq (%f,%f)\n",
	Ap.x,Ap.y,Aq.x,Aq.y,Bp.x,Bp.y,Bq.x,Bq.y,Cp.x,Cp.y,Cq.x,Cq.y);
*/
			pt3 = grballoc(Rpoint);
			pt3->x = Cx;
			pt3->y = Cy;
			pt3->z = Cz;
			COPY2D(Cp,pt3->p);
			COPY2D(Cq,pt3->q);
			pt3->done = TRUE;
			pt3->next = NULL;

			pt4 = Rconv(pt1,pt3,itts);
			pt5 = Rconv(pt2,pt3,itts);
			free(pt3);
			if(pt4 == NULL && pt5 == NULL) return(NULL);
			if(pt4 == NULL ) return(pt5);
			if(pt5 == NULL ) return(pt4);
			pt3 = pt4;
			while(pt3->next != NULL) pt3 = pt3->next;
			pt3->next = pt5;
			return(pt4);
		}

		if( ( Aval > 0.0 && Cval > 0.0 && Bval < 0.0 )
		 || ( Aval < 0.0 && Cval < 0.0 && Bval > 0.0 ) )
		{
			Ax = Cx; Ay = Cy; Az = Cz; Aval = Cval;
			COPY2D(Cp,Ap); COPY2D(Cq,Aq);
		}
		else if( ( Aval > 0.0 && Cval < 0.0 && Bval < 0.0 )
		      || ( Aval < 0.0 && Cval > 0.0 && Bval > 0.0 ) )
		{
			Bx = Cx; By = Cy; Bz = Bz; Bval = Cval;
			COPY2D(Cp,Bp); COPY2D(Cq,Bq);
		}
		else if( Cval == 0.0 )
		{
			pt3 = grballoc(Rpoint);
			pt3->x = Cx;
			pt3->y = Cy;
			pt3->z = Cz;
			COPY2D(Cp,pt3->p);
			COPY2D(Cq,pt3->q);
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
		}
		else
		{
#ifdef PRINT_ERRS
			fprintf(stderr,"Whats gone wrong!\n");
			fprintf(stderr,"A (%f,%f) p (%f,%f) q (%f,%f) val %f\n",
				Ax,Ay,Ap.x,Ap.y,Aq.x,Aq.y,Aval);
			fprintf(stderr,"B (%f,%f) p (%f,%f) q (%f,%f) val %f\n",
				Bx,By,Bp.x,Bp.y,Bq.x,Bq.y,Bval);
			fprintf(stderr,"C (%f,%f) p (%f,%f) q (%f,%f) val %f\n",
				Cx,Cy,Cp.x,Cp.y,Cq.x,Cq.y,Cval);
#endif
			return(NULL);
		}
	}
}

Rpoint *Rconverge(Rpoint *pt1, Rpoint *pt2)
{
	return( Rconv(pt1,pt2,0) );
}

/*********** the main methods called from outside ***********/

#ifndef NO_GEOM
void *DefaultRIntersect(int sel, Geom *geom);
void *ListRIntersect(int sel, Geom *geom);
void *PolylistRIntersect(int sel, Geom *geom);
void *CommentRIntersect(int sel, Geom *geom);

void RIntersect_init()
{
  GeomNewMethod("RIntersect", DefaultRIntersect);
  GeomSpecifyMethod(GeomMethodSel("RIntersect"),
		 GeomClassLookup("list"), ListRIntersect);
  GeomSpecifyMethod(GeomMethodSel("RIntersect"),
		 GeomClassLookup("polylist"), PolylistRIntersect);
  GeomSpecifyMethod(GeomMethodSel("RIntersect"),
		 GeomClassLookup("comment"), CommentRIntersect);
}

/******* The Generic Function for intersectping objects cordinates ******/

Geom *GeomRIntersect(Geom *geom)
{
  int sel;
  Geom *res;

  sel = GeomMethodSel("RIntersect");
/*
  fprintf(stderr,"GeomRInt %s\n",GeomName(geom));
*/
  res =  (Geom *) GeomCall(sel, geom);
/*
  fprintf(stderr,"GeomRInt\n");
  GeomFSave(res,stderr,NULL);
*/
  return( (Geom *) res );
}

/*********** Now the methods for each class *******************/

void *DefaultRIntersect(int sel, Geom *geom)
{
        if(geom != NULL)
        {
                fprintf(stderr,"Can't intersect a geometry of type %s\n",
                        GeomName(geom));
        }
	return(NULL);
}

void *CommentRIntersect(int sel, Geom *geom)
{
	return((void *) geom);
}

void *ListRIntersect(int sel, Geom *geom)
{
  List *l = (List *)geom;
  List *L = NULL;
  Geom	*A,*B;

  A = GeomRIntersect((Geom *) l->car);
  B = GeomRIntersect((Geom *) l->cdr);
/*
fprintf(stderr,"List res %s %s\n",GeomName(A),GeomName(B));
*/
  if( A != NULL && B != NULL )
  {
 	L = (List *) ListAppend((Geom *) L,A);
	L = (List *) ListAppend((Geom *) L,B);
	L->ap = l->ap;
	return((void *) L);
  }
  if( A != NULL ) return((void *) A);
  return((void *) B);
}

/*
 * Function:	PolylistRIntersect RIntersect
 * Action:	Returns a RIntersectped version of the Polylist
 */

void *PolylistRIntersect(int sel, Geom *geom)
{
	PolyList *pl = (PolyList *) geom;
	PolylistRIntersectCore(pl);

	/* Done all faces: now construct the vect structure */

	vert_per_vect = OOGLNewN(short,Rvert_cursize/2);
	coltab = OOGLNewN(short,Rvert_cursize/2);
	colours = OOGLNewN(ColorA,1);
        switch(global_cols)
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

	for(i=0;i<Rvert_cursize/2;++i)
	{
		vert_per_vect[i] = 2;
		coltab[i] = 0;
	}
	coltab[0] = 1;
	if( Rvert_cursize )  
	vect = (Vect *) GeomCreate("vect",
                CR_4D,0,    			/* 4d results */
                CR_NVERT,Rvert_cursize,         /* Number of vertices */
                CR_NVECT,Rvert_cursize/2,       /* Number of vectors */
                CR_POINT4,Rverts,               /* The points */
                CR_VECTC,vert_per_vect,         /* verticies per vect */
                CR_NCOLR,1,                     /* Number of colours */
                CR_COLOR,colours,               /* The colours */
                CR_COLRC,coltab,                /* # cols per vect */
                CR_END);
	else vect = (Vect *) NULL;

	edge[0] = Redgebase;
	while(edge[0] != NULL)
	{
		edge[1] = edge[0]->next;
		sols[0] = edge[0]->pt3;
		while(sols[0] != NULL)
		{
			sols[1] = sols[0]->next;
			free(sols[0]);
			sols[0] = sols[1];
		}
		free(edge[0]);
		edge[0]=edge[1];
	}
	Redgebase = NULL;
	free(corners);
	return((void *) vect);
}
#endif

void *PolyListRIntersectCore(PolyList *pl)
{
	Poly	*p;
	Vertex	*v;
	int	i,j,k,count;
	Redge	*edge[MAX_NUM_REDGES];
	Rpoint	*sols[MAX_NUM_RSOLS];
	Rpoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	Redgebase = NULL;
	
	corners = (Rpoint *) calloc(pl->n_verts,sizeof(Rpoint));
	for(i=0;i<pl->n_verts;++i)
	{
		A = corners+i;
		v = pl->vl + i;
		A->x = v->pt.x;
		A->y = v->pt.y;
		A->z = v->pt.z;
		A->next = NULL;
		A->done = FALSE;
	}

	Rvert_cursize = 0;
	Rvert_allocsize += ALLOC_INC;
	Rverts = (HPoint3 *) calloc(Rvert_allocsize,sizeof(HPoint3) );

	for(i=0;i<pl->n_polys;++i)
	{
		p = pl->p + i;

		if(p->n_vertices > MAX_NUM_REDGES )
		{
	fprintf(stderr,"Too many edges on face %d max I can cope with is %d\n",
				p->n_vertices, MAX_NUM_REDGES);
			continue;
		}

		/* first calc sols on edges */

		for(j=0;j<p->n_vertices;++j)
		{
			k = (j+1) % p->n_vertices;
			jcorn = corners + (p->v[j] - pl->vl);
			kcorn = corners + (p->v[k] - pl->vl);
			edge[j] = getRedge(jcorn,kcorn);
			if(edge[j] == NULL )
			{
				A = Rconverge(jcorn,kcorn);
/*
if(A != NULL)
fprintf(stderr,"Sol found at (%f,%f) on line (%f,%f)..(%f,%f)\n",
	A->x,A->y,jcorn->x,jcorn->y,kcorn->x,kcorn->y);
*/

				edge[j] = addRedge(jcorn,kcorn,A);
			}
		}

		/* next count up how many sols */

		count = 0;
		for(j=0;j<p->n_vertices;++j)
		{
			A = edge[j]->pt3;
			while( A != NULL )
			{
				if( count >= MAX_NUM_RSOLS )
				{
		fprintf(stderr,"Too many sols on face %d max %d\n",
			count, MAX_NUM_RSOLS );
					break;
				}
				sols[count] = A;
				++count;
				A = A->next;
			}
		}
		if(count == 2)
		{
			addRline(sols[0],sols[1]);
		}
		else if( count > 2)
		{
			A = grballoc(Rpoint);
			A->x = 0.0; A->y = 0.0; A->z = 0.0;
			for(j=0;j<count;++j)
			{
				A->x += sols[j]->x;
				A->y += sols[j]->y;
				A->z += sols[j]->z;
			}
			A->x /= (double) count;
			A->y /= (double) count;
			A->z /= (double) count;
			for(j=0;j<count;++j)
			{
				addRline(sols[j],A);
			}
			free(A);
		}
	} 

}


void JvxRIntersect(xml_tree *root)
{
	struct geometry *geom;
	PolyList *pl;
	jvx_pointSet *ps;
	jvx_faceSet *fs;
	jvx_lineSet *ls;
	jvx_f	*p;
	double	*v;
	int	dim_p;
	int	i,j,k,count;
	Redge	*edge[MAX_NUM_REDGES];
	Rpoint	*sols[MAX_NUM_RSOLS];
	Rpoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	if(root->type!=LSMP_GEOMETRY)
	{
		int i;
		for(i=0;i<root->n_child;++i)
			JvxRIntersect(root->children[i]);
		return;
	}
fprintf(stderr,"JvxClip\n");
	geom = root->u.geometry;
	ps = root->u.geometry->pointSet->app_info;
	fs = root->u.geometry->faceSet->app_info;
	if(ps == NULL || fs == NULL)
	{
		report_error(MIDDLE_ERROR,"Could not find a pointset and a face set",601);
		exit(1);
	}
	dim_p = ps->point_dim;

	pl = jvx2PolyList(root);
	PolyListRIntersectCore(pl);

	delete_child_from_jvx_tree(root,"pointSet");
	delete_child_from_jvx_tree(root,"faceSet");
	delete_child_from_jvx_tree(root,"primitive");
	delete_child_from_jvx_tree(root,"vectorField");
	delete_child_from_jvx_tree(root,"bndbox");
	delete_child_from_jvx_tree(root,"center");
	geom->pointSet = NULL;
	geom->faceSet = NULL;
	geom->primitive = NULL;
	geom->vectorField = NULL;
	geom->bndbox = NULL;
	geom->center = NULL;
	{
		double* my_points;
		int* my_lines;
		xml_tree *xml_points,*xml_lines,*xml_color;
		my_points = (double *) malloc(sizeof(double)*Rvert_cursize*dim_p);
		my_lines = (int *) malloc(sizeof(int)*Rvert_cursize);
		for(i=0;i<Rvert_cursize;++i)
		{
			*(my_points + i*dim_p) = (Rverts + i)->x;
			*(my_points + i*dim_p+1) = (Rverts + i)->y;
			*(my_points + i*dim_p+2) = (Rverts + i)->z;
			if(dim_p == 4)
				*(my_points + i*dim_p+3) = (Iverts + i)->w;
			*(my_lines + i) = i;
		}
		xml_points = create_pointSet_from_data(dim_p,Rvert_cursize,my_points);
		set_jvx_attribute(xml_points,"point","hide");

		xml_lines = create_lineSet_from_data(Rvert_cursize/2,my_lines);
		set_jvx_attribute(xml_lines,"line","show");

		if(global_cols >= 0)
		{
			xml_color = create_jvx_color_from_color_number(global_cols);
			add_sub_child_to_lineSet(xml_lines,xml_color);
		}
		add_child_to_geometry(root,xml_points);
		add_child_to_geometry(root,xml_lines);
	}
	return;
	edge[0] = Redgebase;
	while(edge[0] != NULL)
	{
		edge[1] = edge[0]->next;
		sols[0] = edge[0]->pt3;
		while(sols[0] != NULL)
		{
			sols[1] = sols[0]->next;
			free(sols[0]);
			sols[0] = sols[1];
		}
		free(edge[0]);
		edge[0]=edge[1];
	}
	Redgebase = NULL;
}

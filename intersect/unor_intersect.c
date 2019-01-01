/*
 * File:	ridge_intersect.c
 * Function:	intersects a geometry using (un)oriented vec fields
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
*/
#define PRINT_VALS
#define PRINT_ERRS

#define ALLOC_INC 64
#define MAX_NUM_USOLS 20
#define MAX_NUM_UEDGES 20

extern double	tolerance;
extern	int	max_itt;
extern 	int	global_orient;
extern 	int	global_cols;
extern	double calc_given_u(),calc_nearest_u();

#define DOT3D(A,B) ((A).x * (B).x + (A).y * (B).y +(A).z * (B).z )
#define COPY3D(A,B) {(B).x = (A).x; (B).y = (A).y; (B).z = (A).z; }
#define NEGATE3D(A) {(A).x = - (A).x; (A).y = - (A).y; (A).z = - (A).z; }
#define grballoc(node) (node *) malloc( sizeof(node) )
#define CoSet(col,R,G,B) { col.r = R; col.g = G; col.b = B; col.a = 1.0; }

/* A point: x,y its pd's etc */

typedef struct Upoint
{
	double x,y,z;
	VEC3D p;
	double val;
	struct Upoint *next;
	short 	done;
} Upoint;

/* an edge: HPoint3 for location */

typedef struct Uedgestr
{
	Upoint *pt1,*pt2,*pt3;
	struct Uedgestr *next;
} Uedge;

/* We have an ever growing list of verticies */

HPoint3 *Uverts;
int	Uvert_cursize,Uvert_allocsize;
Uedge	*Uedgebase;

addUline(Upoint *A, Upoint *B)
{
	HPoint3 *pt;

	if(Uvert_cursize == Uvert_allocsize)
	{
		Uvert_allocsize += ALLOC_INC;
		Uverts = (HPoint3 *) realloc((void *) Uverts,
				sizeof(HPoint3) * Uvert_allocsize );
	}
	if(Uverts == NULL)
	{
		fprintf(stderr,"Out of space for verticies\n");
		return;
	}
	pt = Uverts + Uvert_cursize;
	pt->x = A->x; pt->y = A->y; pt->z = A->z; pt->w = 1.0;
	++Uvert_cursize;
	pt = Uverts + Uvert_cursize;
	pt->x = B->x; pt->y = B->y; pt->z = B->z; pt->w = 1.0;
	++Uvert_cursize;
}

/* now find an edge */

Uedge *getUedge(Upoint *A,Upoint *B)
{
	Uedge *edge;

	edge = Uedgebase;
	while( edge != NULL )
	{
		if( (edge->pt1 == A && edge->pt2 == B )
		 || (edge->pt1 == B && edge->pt2 == A ) )
			return(edge);
		edge = edge->next;
	}
	return(NULL);
}

Uedge *addUedge(Upoint *pt1, Upoint *pt2, Upoint *pt3)
{
	Uedge *edge;

	edge = grballoc(Uedge);
	edge->pt1 = pt1;
	edge->pt2 = pt2;
	edge->pt3 = pt3;
	edge->next = Uedgebase;
	Uedgebase = edge;
	return(Uedgebase);
}

/*********** Now the convergence routines *******************/

Upoint *Uconv(Upoint *pt1, Upoint *pt2,int itts)
{
	double Ax,Ay,Az,Aval,Bx,By,Bz,Bval,Cx,Cy,Cz,Cval;
	Upoint *pt3,*pt4,*pt5;
	VEC3D	Ap,Bp,Cp;

	/* first check coherent orientations */

	if( pt1->done && pt2->done )
	{
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) 
		 || (pt2->p.x == 0.0 && pt2->p.y == 0.0) )
			return(NULL);

		if( DOT3D(pt1->p,pt2->p) > 0.0 )
		{
		}
		else if( global_orient != ORIENT_ORIENT )
		{
			NEGATE3D(pt2->p);
			pt2->val = calc_given_u(pt2->x,pt2->y,
				pt2->p);
		}
	}
	else if( pt2->done )
	{
		if( (pt2->p.x == 0.0 && pt2->p.y == 0.0) )
			return(NULL);
		COPY3D(pt2->p,pt1->p);
		pt1->val = calc_nearest_u(pt1->x,pt1->y,&(pt1->p));
		pt1->done = TRUE;
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) )
			return(NULL);
	}
	else if( pt1->done )
	{
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) )
			return(NULL);
		COPY3D(pt1->p,pt2->p);
		pt2->val = calc_nearest_u(pt2->x,pt2->y,&(pt2->p));
		pt2->done = TRUE;
		if( (pt2->p.x == 0.0 && pt2->p.y == 0.0) )
			return(NULL);
	}
	else	/* neither is done */
	{
		pt1->p.x = 1.0; pt1->p.y = 0.0; pt1->p.z = 0.0;
		pt1->val = calc_nearest_u(pt1->x,pt1->y,&(pt1->p));
		pt1->done = TRUE;
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) )
			return(NULL);
		COPY3D(pt1->p,pt2->p);
		pt2->val = calc_nearest_u(pt2->x,pt2->y,&(pt2->p));
		pt2->done = TRUE;
		if( (pt1->p.x == 0.0 && pt1->p.y == 0.0) )
			return(NULL);
	}
/*
fprintf(stderr,"\nUconv\n");
fprintf(stderr,"pt1 (%f,%f) u (%f,%f,%f) val %f done %d\n",
		pt1->x,pt1->y,pt1->p.x,pt1->p.z,pt1->p.z,
		pt1->val,pt1->done);
fprintf(stderr,"pt2 (%f,%f) p (%f,%f,%f) val %f done %d\n",
		pt2->x,pt2->y,pt2->p.x,pt2->p.z,pt2->p.z,
		pt2->val,pt2->done);
*/

	/* Now have coherent orientations */

	if( pt1->val == 0.0 )
	{
			pt3 = grballoc(Upoint);
			pt3->x = pt1->x;
			pt3->y = pt1->y;
			pt3->z = pt1->z;
			COPY3D(pt1->p,pt3->p);
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
	}
	if( pt2->val == 0.0 )
	{
			pt3 = grballoc(Upoint);
			pt3->x = pt2->x;
			pt3->y = pt2->y;
			pt3->z = pt2->z;
			COPY3D(pt2->p,pt3->p);
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
	}
	
	if( (pt1->val > 0.0 && pt2->val > 0.0 ) 
	 || (pt1->val < 0.0 && pt2->val < 0.0 ) )
		return(NULL);

	/* now converge */

	Ax = pt1->x; Ay = pt1->y; Az = pt1->z; COPY3D(pt1->p,Ap);
	Bx = pt2->x; By = pt2->y; Bz = pt2->z; COPY3D(pt2->p,Bp); 
	Aval = pt1->val; Bval = pt2->val;

	while(1)
	{
		Cx = (Ax + Bx)/2; Cy = (Ay + By)/2;
		Cz = (Az + Bz)/2; 
		COPY3D(Ap,Cp);
		Cval = calc_nearest_u(Cx,Cy,&Cp);
		if( (Cp.x == 0.0 && Cp.y == 0.0) )
			return(NULL);
/*
fprintf(stderr,"Bisect (%f,%f) p (%f,%f,%f) val %f\n",
		Cx,Cy,Cp.x,Cp.y,Cp.z,Cval);		
*/
		if( ++itts > max_itt || Cval == 0.0 )
		{
			pt3 = grballoc(Upoint);
			pt3->x = Cx;
			pt3->y = Cy;
			pt3->z = Cz;
			COPY3D(Cp,pt3->p);
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
		}

		/* Now check things are sensible */

		if( DOT3D(Cp,Bp) <= 0.0 )
		{
			pt3 = grballoc(Upoint);
			pt3->x = Cx;
			pt3->y = Cy;
			pt3->z = Cz;
			COPY3D(Cp,pt3->p);
			pt3->done = TRUE;
			pt3->next = NULL;

			pt4 = Uconv(pt1,pt3,itts);
			pt5 = Uconv(pt2,pt3,itts);
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
			COPY3D(Cp,Ap); 
		}
		else if( ( Aval > 0.0 && Cval < 0.0 && Bval < 0.0 )
		      || ( Aval < 0.0 && Cval > 0.0 && Bval > 0.0 ) )
		{
			Bx = Cx; By = Cy; Bz = Cz; Bval = Cval;
			COPY3D(Cp,Bp); 
		}
		else
		{
#ifdef PRINT_ERRS
			fprintf(stderr,"Whats gone wrong!\n");
			fprintf(stderr,"A (%f,%f) p (%f,%f,%f) val %f\n",
				Ax,Ay,Ap.x,Ap.y,Ap.z,Aval);
			fprintf(stderr,"B (%f,%f) p (%f,%f,%f) val %f\n",
				Bx,By,Bp.x,Bp.y,Bp.z,Bval);
			fprintf(stderr,"C (%f,%f) p (%f,%f,%f) val %f\n",
				Cx,Cy,Cp.x,Cp.y,Cp.z,Cval);
#endif
			return(NULL);
		}
	}
}

Upoint *Uconverge(Upoint *pt1, Upoint *pt2)
{
	return( Uconv(pt1,pt2,0) );
}

/*********** the main methods called from outside ***********/

#ifndef NO_GEOM
void *DefaultUIntersect(int sel, Geom *geom);
void *ListUIntersect(int sel, Geom *geom);
void *PolylistUIntersect(int sel, Geom *geom);
void *CommentUIntersect(int sel, Geom *geom);

void UIntersect_init()
{
  GeomNewMethod("UIntersect", DefaultUIntersect);
  GeomSpecifyMethod(GeomMethodSel("UIntersect"),
		 GeomClassLookup("list"), ListUIntersect);
  GeomSpecifyMethod(GeomMethodSel("UIntersect"),
		 GeomClassLookup("polylist"), PolylistUIntersect);
  GeomSpecifyMethod(GeomMethodSel("UIntersect"),
		 GeomClassLookup("comment"), CommentUIntersect);
}

/******* The Generic Function for intersectping objects cordinates ******/

Geom *GeomUIntersect(Geom *geom)
{
  int sel;
  Geom *res;

  sel = GeomMethodSel("UIntersect");
/*
  fprintf(stderr,"GeomUInt %s\n",GeomName(geom));
*/
  res =  (Geom *) GeomCall(sel, geom);
/*
  fprintf(stderr,"GeomUInt\n");
  GeomFSave(res,stderr,NULL);
*/
  return( (Geom *) res );
}

/*********** Now the methods for each class *******************/

void *DefaultUIntersect(int sel, Geom *geom)
{
        if(geom != NULL)
        {
                fprintf(stderr,"Can't intersect a geometry of type %s\n",
                        GeomName(geom));
        }
	return(NULL);
}

void *CommentUIntersect(int sel, Geom *geom)
{
	return((void *) geom);
}

void *ListUIntersect(int sel, Geom *geom)
{
  List *l = (List *)geom;
  List *L = NULL;
  Geom	*A,*B;

  A = GeomUIntersect((Geom *) l->car);
  B = GeomUIntersect((Geom *) l->cdr);
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

void *PolylistUIntersect(int sel, Geom *geom)
{
	PolyList *pl = (PolyList *) geom;
	PolylistUIntersectCore(pl);

	/* Done all faces: now construct the vect structure */

	vert_per_vect = OOGLNewN(short,Uvert_cursize/2);
	coltab = OOGLNewN(short,Uvert_cursize/2);
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

	for(i=0;i<Uvert_cursize/2;++i)
	{
		vert_per_vect[i] = 2;
		coltab[i] = 0;
	}
	coltab[0] = 1;
	if( Uvert_cursize )
	vect = (Vect *) GeomCreate("vect",
                CR_4D,0,    			/* 4d results */
                CR_NVERT,Uvert_cursize,         /* Number of vertices */
                CR_NVECT,Uvert_cursize/2,       /* Number of vectors */
                CR_POINT4,Uverts,               /* The points */
                CR_VECTC,vert_per_vect,         /* verticies per vect */
                CR_NCOLR,1,                     /* Number of colours */
                CR_COLOR,colours,               /* The colours */
                CR_COLRC,coltab,                /* # cols per vect */
                CR_END);
	else vect = (Vect *) NULL;

	edge[0] = Uedgebase;
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
	Uedgebase = NULL;
	free(corners);
	return((void *) vect);
}
#endif

/*
 * Function:	PolylistUIntersect UIntersect
 * Action:	Returns a UIntersectped version of the Polylist
 */

void *PolyListUIntersectCore(PolyList *pl)
{
	Poly	*p;
	Vertex	*v;
	int	i,j,k,count;
	Uedge	*edge[MAX_NUM_REDGES];
	Upoint	*sols[MAX_NUM_RSOLS];
	Upoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	Uedgebase = NULL;
	
/*
	fprintf(stderr,"PUI vert %d poly %d\n",pl->n_verts,pl->n_polys);
*/
	corners = (Upoint *) calloc(pl->n_verts,sizeof(Upoint));
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

	Uvert_cursize = 0;
	Uvert_allocsize += ALLOC_INC;
	Uverts = (HPoint3 *) calloc(Uvert_allocsize,sizeof(HPoint3) );

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
			edge[j] = getUedge(jcorn,kcorn);
			if(edge[j] == NULL )
			{
				A = Uconverge(jcorn,kcorn);
/*
if(A != NULL)
fprintf(stderr,"Sol found at (%f,%f) on line (%f,%f)..(%f,%f)\n",
	A->x,A->y,jcorn->x,jcorn->y,kcorn->x,kcorn->y);
else
fprintf(stderr,"No Sol found on line (%f,%f)..(%f,%f)\n",
	jcorn->x,jcorn->y,kcorn->x,kcorn->y);
*/
				edge[j] = addUedge(jcorn,kcorn,A);
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
			addUline(sols[0],sols[1]);
		}
		else if( count > 2)
		{
			A = grballoc(Upoint);
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
				addUline(sols[j],A);
			}
			free(A);
		}
	} 

}

void JvxUIntersect(xml_tree *root)
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
	Uedge	*edge[MAX_NUM_UEDGES];
	Upoint	*sols[MAX_NUM_USOLS];
	Upoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	if(root->type!=LSMP_GEOMETRY)
	{
		int i;
		for(i=0;i<root->n_child;++i)
			JvxUIntersect(root->children[i]);
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
	PolyListUIntersectCore(pl);

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
		my_points = (double *) malloc(sizeof(double)*Uvert_cursize*dim_p);
		my_lines = (int *) malloc(sizeof(int)*Uvert_cursize);
		for(i=0;i<Uvert_cursize;++i)
		{
			*(my_points + i*dim_p) = (Uverts + i)->x;
			*(my_points + i*dim_p+1) = (Uverts + i)->y;
			*(my_points + i*dim_p+2) = (Uverts + i)->z;
			if(dim_p == 4)
				*(my_points + i*dim_p+3) = (Iverts + i)->w;
			*(my_lines + i) = i;
		}
		xml_points = create_pointSet_from_data(dim_p,Uvert_cursize,my_points);
		set_jvx_attribute(xml_points,"point","hide");

		xml_lines = create_lineSet_from_data(Uvert_cursize/2,my_lines);
		set_jvx_attribute(xml_lines,"line","show");

		if(global_cols >= 0)
		{
			xml_color = create_jvx_color_from_color_number(global_cols);
			add_sub_child_to_lineSet(xml_lines,xml_color);
		}
		add_child_to_geometry(root,xml_points);
		add_child_to_geometry(root,xml_lines);
	}

	edge[0] = Uedgebase;
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
	Uedgebase = NULL;
}

/*
 * File:	clip.c
 * Function:	clips a geometry by an equation
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
#include "geomclass.h"
#include "listP.h"
#include "list.h"
#endif
#include <math.h>
#define MEMSET
#ifdef MEMSET
#define bzero(a,b) memset((a),0,(b))
#define bcopy(in,out,size) memmove(out,in,size)
#endif

/*
#define PRINT_VALS
*/
extern double tolerance;

typedef struct clipedge
{
	Vertex *low,*high,*sol;
	struct clipedge *next;
} clipedge;

int	normals;		/* Do normals? */
int	colours;		/* Do Colours */
int	next_new_vert;		/* number of next new vertex */
Vertex	*extraverts,		/* A new list of vertices */
	*oldverts;		/* The old list of vertices */
int     extraverts_alloc;	/* number of allocated entries */
Vertex  **oldtonew;		/* Array giving new pointers */
clipedge *edgebase;
int clipCore_i;

#ifndef NO_GEOM
void *DefaultClip(int sel, Geom *geom,double (*fun)());
void *ListClip(int sel, Geom *geom,double (*fun)());
void *PolylistClip(int sel, Geom *geom, double (*fun)());
void *CommentClip(int sel, Geom *geom, double (*fun)());
void *VectClip(int sel, Geom *geom, double (*fun)());

void Clip_init()
{
  GeomNewMethod("Clip", DefaultClip);
  GeomSpecifyMethod(GeomMethodSel("Clip"),
		 GeomClassLookup("list"), ListClip);
  GeomSpecifyMethod(GeomMethodSel("Clip"),
		 GeomClassLookup("polylist"), PolylistClip);
  GeomSpecifyMethod(GeomMethodSel("Clip"),
		 GeomClassLookup("comment"), CommentClip);
  GeomSpecifyMethod(GeomMethodSel("Clip"),
		 GeomClassLookup("vect"), VectClip);
}

/******* The Generic Function for clipping objects cordinates ******/

Geom *GeomClip(Geom *geom,double (*fun)())
{
  int sel;

  sel = GeomMethodSel("Clip");
  return( (Geom *) GeomCallV(sel, geom,fun));
}

/*********** Now the methods for each class *******************/

void *DefaultClip(int sel, Geom *geom,double (*fun)())
{
        if(geom != NULL)
        {
                fprintf(stderr,"Can't clip a geometry of type %s\n",
                        GeomName(geom));
        }
}

void *CommentClip(int sel, Geom *geom,double (*fun)())
{
	return(geom);
}

void *ListClip(int sel, Geom *geom,double (*fun)())
{
  List *l = (List *)geom;

  GeomClip((Geom *) l->car,fun);
  GeomClip((Geom *) l->cdr,fun);
}

void *PolylistClip(int sel, Geom *geom,double (*fun)())
{
	PolyList *pl = (PolyList *) geom;

	PolyListClipCore(pl,fun);
}
#endif
/*
 * Function:	PolylistClip Clip
 * Action:	Returns a Clipped version of the Polylist
 */

void *PolyListClipCore(PolyList *pl,double (*fun)())
{
	int i,j;
	clipedge *edge;

	edgebase = NULL;
	normals = pl->flags & PL_HASVN;
	colours = pl->flags & PL_HASVCOL;

	/* Create a new double sized list of verticies */

	oldverts = pl->vl;
	extraverts = OOGLNewNE(Vertex,pl->n_verts*3,"extra vertices");
	extraverts_alloc = pl->n_verts*3;
	oldtonew = OOGLNewNE(Vertex *,pl->n_verts*2," old to new vertex");
	next_new_vert = 0;
	bzero(oldtonew,2*pl->n_verts*sizeof(Vertex *));

	/* Clip all the faces */

	for(i=0;i<pl->n_polys;++i)
	{
		clipCore_i = i;
		FaceClip(pl,&(pl->p[i]),fun);
	}

	/* Use the new list of verticies */

	OOGLFree(pl->vl);
	pl->vl=extraverts;
	pl->n_verts = next_new_vert;

	/* now remove empty faces */

	j = 0;
	for(i=0;i<pl->n_polys;++i)
	{
		if(pl->p[i].n_vertices != 0)
		{
			if(i!=j) bcopy(&pl->p[i],&pl->p[j],sizeof(Poly));
			++j;
		}
	}
	pl->n_polys = j;

	/* Free the edges */

	edge = edgebase;
	while(edge != NULL )
	{
		edge = edge->next;
		free(edgebase);
		edgebase = edge;
	}
	edgebase = NULL;
}

/*
 * Function:	newposn
 * Action:	given a pointer to old array return pointer to new array
 */

Vertex *newfromold(Vertex *oldposn) 
{
	int index;

	index = oldposn-oldverts;
	if(oldtonew[index] == NULL )
	{
		if(next_new_vert > extraverts_alloc - 2)
		{
			extraverts_alloc *= 2;
			extraverts = (Vertex *) realloc(extraverts,sizeof(Vertex)* extraverts_alloc);
		}
		oldtonew[index] = extraverts + next_new_vert;
		bcopy(oldposn,oldtonew[index],sizeof(Vertex));
		++next_new_vert;
	}
	return(oldtonew[index]);
}

/*
 * Function:	FaceClip
 * Action:	Clip one face
 */

FaceClip(pl,p,fun)
PolyList *pl;
Poly *p;
double (*fun)();
{
	int i,j,added = 0,removed = 0,nexti;
	double lambda;
	double val,oldval;
	Vertex **newverts;
	clipedge *edge,*tmp;
	int	errflag = FALSE;
	double *val_array;

	/* First count up how many verticies are added and removed */

	added = removed = 0;

	val_array = (double *) malloc(sizeof(double)*p->n_vertices);

	for(i=0;i<p->n_vertices;++i)
	{
		val_array[i] = fun(p->v[i]->pt.x,p->v[i]->pt.y,p->v[i]->pt.z);
	}

	for(i=0;i<p->n_vertices;++i)
	{
		val = val_array[i];
		if( val != val || val < -tolerance ) ++removed;
#ifdef PRINT_VALS
		if( val != val ) errflag = TRUE;
#endif
		oldval = val;
		nexti = (i+1)%p->n_vertices;
		val = val_array[nexti];
		if( val != val ) errflag = TRUE;
		if( ( oldval < -tolerance && val > tolerance)
		 || (oldval > tolerance && val < -tolerance))
			++added;
	}

	if( !added && !removed )
	{
		for(i=0;i<p->n_vertices;++i)
		{
			p->v[i] = newfromold(p->v[i]);
#ifdef PRINT_VALS
	if( errflag )
	fprintf(stderr,"funval %f\n",
		fun(p->v[i]->pt.x,p->v[i]->pt.y,p->v[i]->pt.z));
#endif
		}
		return; /* No change so finish */
	}

	/* Now we construct the new list of verticies */

#ifdef PRINT_VALS
	fprintf(stderr,"Funny face num %d add %d rem %d\n",
		p->n_vertices,added,removed);
	errflag = TRUE;
#endif
	newverts = OOGLNewN(Vertex *,p->n_vertices+added-removed);
	j = 0;

	for(i=0;i<p->n_vertices;++i)
	{
		val = val_array[i];
		if( val >= -tolerance )	/* Keep this vertex */
		{
#ifdef PRINT_VALS
	if( errflag )
		fprintf(stderr,"Keep : val %f\n",val);
#endif
			newverts[j++] = newfromold(p->v[i]);
		}

		oldval = val;
		nexti = (i+1)%p->n_vertices;
/*
		val=fun(p->v[nexti]->pt.x,p->v[nexti]->pt.y,p->v[nexti]->pt.z);
		if( ((oldval != oldval || oldval < -tolerance)
		     && val > tolerance)
		 || (oldval > tolerance && 
			( val != val || val < -tolerance)))
*/

		val = val_array[nexti];
		if( ( oldval < -tolerance && val > tolerance)
		 || (oldval > tolerance && val < -tolerance))
		{
		    edge = edgebase;

		    while( edge != NULL )
		    {
			if((edge->low == p->v[i] &&
				edge->high == p->v[nexti] )
			 ||( edge->low == p->v[nexti] &&
				edge->high == p->v[i] ) )
			{
				/* Found an edge which matches */

				newverts[j] = edge->sol;


#ifdef PRINT_VALS
	if( errflag )
				fprintf(stderr,"matched edge: oldval %f val %f edgeval %f\n",
					oldval,val,
					fun(edge->sol->pt.x,edge->sol->pt.y,
						edge->sol->pt.z));
#endif
				++j;
				break;
			}
			edge = edge->next;
		    }

		    if( edge == NULL )
		    {
			/* Edge not in list construct new vertex */

			if(next_new_vert > extraverts_alloc - 2)
			{
				extraverts_alloc *= 2;
				extraverts = (Vertex *) realloc(extraverts,sizeof(Vertex)* extraverts_alloc);
			}
			newverts[j] = extraverts + next_new_vert;
			++next_new_vert;

			/* converge to solution */

                        converge(&p->v[i]->pt,oldval,
                                 &p->v[nexti]->pt,val,
                                 &newverts[j]->pt,&lambda,fun);

			if( normals )
			Pt3Comb(lambda,&p->v[i]->vn,
				  (1.0-lambda),&p->v[nexti]->vn,
				&newverts[j]->vn);

			if( colours )
			{
			newverts[j]->vcol.r =
				lambda * p->v[i]->vcol.r +
				(1.0-lambda) * p->v[nexti]->vcol.r;
			newverts[j]->vcol.g =
				lambda * p->v[i]->vcol.g +
				(1.0-lambda) * p->v[nexti]->vcol.g;
			newverts[j]->vcol.b =
				lambda * p->v[i]->vcol.b +
				(1.0-lambda) * p->v[nexti]->vcol.b;
			newverts[j]->vcol.a =
				lambda * p->v[i]->vcol.a +
				(1.0-lambda) * p->v[nexti]->vcol.a;
			}

		if(clipCore_i>278) { tmp = edgebase; while(tmp!=NULL) tmp = tmp->next; }
			edge = OOGLNew(clipedge);
			edge->next = edgebase;
			edge->sol = newverts[j];
		if(clipCore_i>278) { tmp = edgebase; while(tmp!=NULL) tmp = tmp->next; }
#ifdef PRINT_VALS
	if( errflag )
		fprintf(stderr,"new edge: oldval %f val %f edgeval %f\n",
					oldval,val,
					fun(edge->sol->pt.x,edge->sol->pt.y,
						edge->sol->pt.z));
#endif
			if(oldval < 0.0 )
			{
				edge->low = p->v[i];
				edge->high = p->v[nexti];
			}
			else
			{
				edge->low = p->v[nexti];
				edge->high = p->v[i];
			}
			edgebase = edge;
			++j;
		if(clipCore_i>278) { tmp = edgebase; while(tmp!=NULL) tmp = tmp->next; }
		    }
		}
	} /* End for-loop */
#ifdef PRINT_VALS
	if( errflag )
	fprintf(stderr,"\n");
#endif

	free( p->v );
	p->v = newverts;
	if( j != p->n_vertices + added - removed )
		fprintf(stderr,"Error in GeomClip: bad number of verticies, new  %d old %d added %d removed %d\n",
			j,p->n_vertices,added,removed);
		
	p->n_vertices = j;
}

#define PLUS_VAL(x) ( x == x &&  x >= -tolerance )
#define NEG_VAL(x) ( x == x && x < -tolerance )
#define BAD_VAL(x) ( x != x )
#define COL_INTERP(A,B,C,lam) do { \
	A.r = lam * B.r + (1-lam) * C.r; \
	A.g = lam * B.g + (1-lam) * C.g; \
	A.b = lam * B.b + (1-lam) * C.b; \
	A.a = lam * B.a + (1-lam) * C.a; \
	} while(0); 

/*
 * Function:	VectClip Clip
 * Action:	Returns a Clipped version of the vect
 */

#ifndef NO_GEOM
void *VectClip(int sel, Geom *geom,double (*fun)())
{
	Vect *v = (Vect *) geom;
	int i,j,k,n,col_index;
	clipedge *edge;
	double *val;
	short	n_vec,n_vert,n_col,left,right,this;
	HPoint3	*verts;
	short	*vnvert;
	short	*vncol;
	ColorA	*colours;
	double	lambda;

	val = (double *) malloc(sizeof(double)*v->nvert);

	for(i=0;i<v->nvert;++i)
	{
		val[i]=fun(v->p[i].x,v->p[i].y,v->p[i].z);
/*
fprintf(stderr,"val[%d] = %f %d %d %d\n",i,val[i],
	PLUS_VAL(val[i]),NEG_VAL(val[i]),BAD_VAL(val[i]));
*/
	}

	/* now count up to see how much space we need */

	j = 0; /* which vertex */
	n_vec = 0;
	n_vert = 0;
	n_col = 0;
	col_index = 0;


	for(i=0;i<v->nvec;++i)
	{
		n = v->vnvert[i];

		if( n == 0 ) {}
		else if( n == 1 || n == -1 )
		{
			if( PLUS_VAL(val[j]) )
			{
				++n_vec;
				++n_vert;
				n_col += v->vncolor[i];
			}
			++j;
		}
		else if( n == 2 || n == -2 )
		{
/*
fprintf(stderr,"f(%f,%f) = %f, f(%f,%f) = %f ",
		v->p[j].x,v->p[j].y,val[j],
		v->p[j+1].x,v->p[j+1].y,val[j+1]);
*/
			if( PLUS_VAL(val[j]) && PLUS_VAL(val[j+1]) )
			{
/* fprintf(stderr,"++\n"); */
				n_vert += 2;
				++n_vec;
				n_col += v->vncolor[i];
			}
			else if( PLUS_VAL(val[j]) && NEG_VAL(val[j+1] ) )
			{	
/* fprintf(stderr,"+-\n"); */
				n_vert += 2;
				++n_vec;
				n_col += v->vncolor[i];
			}
			else if( NEG_VAL(val[j]) && PLUS_VAL(val[j+1] ) )
			{	
/* fprintf(stderr,"-+\n"); */
				n_vert += 2;
				++n_vec;
				n_col += v->vncolor[i];
			}
			else if( PLUS_VAL(val[j]) || PLUS_VAL(val[j+1]) )
			{
/* fprintf(stderr,"+||+\n"); */
				n_vert += 1;
				++n_vec;
				n_col += v->vncolor[i];
			}
/* else fprintf(stderr,"??\n"); */
	
		
			j += 2;
		}

		else if( n < 0 )	/* closed */
		{
			n = -n;
			this = left = right = 0;
/*
fprintf(stderr,"closed n %d vals\n",n);
*/
			for(k=0;k<n;++k)
			{
/*
fprintf(stderr,"%f ",val[j+k]);
*/
				if( PLUS_VAL(val[j+k]) ) {
					++this; }
			}
/*
fprintf(stderr,"\n");
*/
			for(k=0;k<n-1;++k)
			{
				if( PLUS_VAL(val[j+k]) && PLUS_VAL(val[j+k+1]))
				{}
				else if( PLUS_VAL(val[j+k]) && NEG_VAL(val[j+k+1]))
				{	++this; ++right;
				}
				else if(NEG_VAL(val[j+k])&&PLUS_VAL(val[j+k+1]))
				{	++this; ++left;
				}
				else if(PLUS_VAL(val[j+k]) )
				{	++right;
				} else if(PLUS_VAL(val[j+k+1]))
				{
					++left;
				}
			}

			/* now the joining seg */
#ifdef NOT_DEF
			if( left != right )
			{
				fprintf(stderr,"Left %d != right %d vals\n",
					left,right);
				for(k=0;k<n;++k)
				{
					fprintf(stderr,"%f ",val[j+k]);
				}
				fprintf(stderr,"\n");
			}
#endif
/*
fprintf(stderr,"this %d l %d r %d\n",this,left,right);
*/
			if( PLUS_VAL(val[j+k]) && PLUS_VAL(val[j]))
			{
				if( left == 0 && right == 0 && this > 0 )
				{
					n_vert += this;
					n_vec += 1;
					n_col += v->vncolor[i];
				}
				else if( this == 0 || left == 0 )
				{
				}
				else
				{
					++this; ++left;
					n_vert += this;
					n_vec += left; 
					if(v->vncolor[i] > 1 )
						n_col += this;
					else if(v->vncolor[i] == 1 )
						n_col += left;
				}
			}
			else if( PLUS_VAL(val[j+k]) && NEG_VAL(val[j]))
			{	++this; ++right;
				n_vert += this;
				n_vec += left; 
				if(v->vncolor[i] > 1 )
					n_col += this;
				else if(v->vncolor[i] == 1 )
					n_col += left;
			}
			else if(NEG_VAL(val[j+k])&&PLUS_VAL(val[j]))
			{	++this; ++left; ++left; ++this;
				n_vert += this;
				n_vec += left; 
				if(v->vncolor[i] > 1 )
					n_col += this;
				else if(v->vncolor[i] == 1 )
					n_col += left;
			}
			else if(PLUS_VAL(val[j]) )
			{	++left;
				n_vert += this;
				n_vec += left; 
				if(v->vncolor[i] > 1 )
					n_col += this;
				else if(v->vncolor[i] == 1 )
					n_col += left;
			} else if(PLUS_VAL(val[j+k]))
			{
				++right;
				n_vert += this;
				n_vec += left; 
				if(v->vncolor[i] > 1 )
					n_col += this;
				else if(v->vncolor[i] == 1 )
					n_col += left;
			}
			else
			{
				n_vert += this;
				n_vec += left; 
				if(v->vncolor[i] > 1 )
					n_col += this;
				else if(v->vncolor[i] == 1 )
					n_col += left;
			}
/*
fprintf(stderr,"this %d l %d r %d\n",this,left,right);
*/
			j += n;
		}
		else
		{
			this = left = right = 0;
/*
fprintf(stderr,"open n %d vals\n",n);
*/
			for(k=0;k<n;++k)
			{
/*
fprintf(stderr,"%f ",val[j+k]);
*/
				if( PLUS_VAL(val[j+k]) ) {
					++this; }
			}
/*
fprintf(stderr,"\n");
*/
			for(k=0;k<n-1;++k)
			{
				if( PLUS_VAL(val[j+k]) && PLUS_VAL(val[j+k+1]))
				{}
				else if( PLUS_VAL(val[j+k]) && NEG_VAL(val[j+k+1]))
				{	++this; ++right;
				}
				else if(NEG_VAL(val[j+k])&&PLUS_VAL(val[j+k+1]))
				{	++this; ++left;
				}
				else if(PLUS_VAL(val[j+k]) )
				{	++right;
				} else if(PLUS_VAL(val[j+k+1]))
				{
					++left;
				}
			}
			if( PLUS_VAL(val[j]) ) ++left;
			if( PLUS_VAL(val[j+k]) ) ++right;
/*
fprintf(stderr,"open this %d l %d r %d\n",this,left,right);
*/
			if( left != right )
			{
				fprintf(stderr,"Left != right\n");
				for(k=0;k<n-1;++k)
				{
					fprintf(stderr,"%f ",val[j+k]);
				}
				fprintf(stderr,"\n");
			}
			else if( left == 1 && this > 0 )
			{
				n_vert += this;
				n_vec += 1;
				n_col += v->vncolor[i];
			}
			else if( left == 1 || this == 0 )
			{}
			else 
			{
				n_vert += this;
				n_vec += left;
				if(v->vncolor[i] > 1 )
					n_col += this;
				else if(v->vncolor[i] == 1 )
					n_col += left;
			}
			j += n;
		}
	}

	/* now got numbers allocate space */
/*
fprintf(stderr,"Initial %d %d %d\n",n_vec,n_vert,n_col);
*/
	if( n_vert == 0 || n_vec == 0 )
	{
		OOGLFree(v->vnvert);
		OOGLFree(v->p);
		OOGLFree(v->c);
		v->vnvert = OOGLNewN(short,2);
		v->vncolor = v->vnvert + 1;
		v->p = OOGLNewN(HPoint3,1);
		v->c = OOGLNewN(ColorA,1);
		v->nvec = 0;
		v->nvert = 0;
		v->ncolor = 0;

		if( !vSane(v) ) 
		{
			fprintf(stderr,"insane vect\n");
		}
		return;
	}

	vnvert = OOGLNewN(short,n_vec*2);
        vncol = vnvert + n_vec;
        colours = OOGLNewN(ColorA,n_col);
	verts = OOGLNewN(HPoint3,n_vert);

	j = 0; /* which vertex */
	n_vec = 0;
	n_vert = 0;
	n_col = 0;
	col_index = 0;

	for(i=0;i<v->nvec;++i)
	{
		n = v->vnvert[i];

		if( n == 0 ) {}
		else if( n == 1 || n == -1 )
		{
			if( n < 0 ) n = -n;
			if( PLUS_VAL(val[j]) )
			{
				verts[n_vert] = v->p[j];
				vnvert[n_vec] = 1;
				if(v->vncolor[i] >= 1 )
				{
					vncol[n_vec] = 1;
					colours[n_col] = v->c[col_index];
					++n_col;
				}
				else
					vncol[n_vec] = 0;
				++n_vec;
				++n_vert;
			}
		}

		else if( n == 2 || n == -2 )
		{
			if( n < 0 ) n = -n;
			if( PLUS_VAL(val[j]) && PLUS_VAL(val[j+1]) )
			{
				verts[n_vert] = v->p[j];
				verts[n_vert+1] = v->p[j+1];
				n_vert += 2;
				if(v->vncolor[i] >= 2 )
				{
					vncol[n_vec] = 2;
					colours[n_col] = v->c[col_index];
					colours[n_col+1] = v->c[col_index+1];
					n_col += 2;
				}
				else if(v->vncolor[i] == 1 )
				{
					vncol[n_vec] = 1;
					colours[n_col] = v->c[col_index];
					n_col += 1;
				}
				else	vncol[n_vec] = 0;

				vnvert[n_vec] = 2;
				++n_vec;
			}
			else if( PLUS_VAL(val[j]) && NEG_VAL(val[j+1] ) )
			{
				verts[n_vert] = v->p[j];
				++n_vert;
                        	converge(&v->p[j],val[j],
                                	 &v->p[j+1],val[j+1],
                                	 &verts[n_vert],&lambda,fun);
				++n_vert;
				if(v->vncolor[i] >= 2 )
				{
					vncol[n_vec] = 2;
					colours[n_col] = v->c[col_index];
					COL_INTERP(colours[n_col+1],
						v->c[col_index],
						v->c[col_index+1],lambda);
					n_col += 2;
				}
				else if(v->vncolor[i] == 1 )
				{
					vncol[n_vec] = 1;
					colours[n_col] = v->c[col_index];
					n_col += 1;
				}
				else	vncol[n_vec] = 0;
				vnvert[n_vec] = 2;
				++n_vec;
			}
			else if( NEG_VAL(val[j]) && PLUS_VAL(val[j+1] ) )
			{
                        	converge(&v->p[j],val[j],
                                	 &v->p[j+1],val[j+1],
                                	 &verts[n_vert],&lambda,fun);
				++n_vert;
				verts[n_vert] = v->p[j+1];
				++n_vert;
				if(v->vncolor[i] >= 2 )
				{
					vncol[n_vec] = 2;
					COL_INTERP(colours[n_col],
						v->c[col_index],
						v->c[col_index+1],lambda);
					colours[n_col+1] = v->c[col_index];
					n_col += 2;
				}
				else if(v->vncolor[i] == 1 )
				{
					vncol[n_vec] = 1;
					colours[n_col] = v->c[col_index];
					n_col += 1;
				}
				else	vncol[n_vec] = 0;
				vnvert[n_vec] = 2;
				++n_vec;
			}
			else if( PLUS_VAL(val[j]) )
			{
				verts[n_vert] = v->p[j];
				n_vert += 1;
				if(v->vncolor[i] >= 1 )
				{
					vncol[n_vec] = 1;
					colours[n_col] = v->c[col_index];
					n_col += 1;
				}
				else	vncol[n_vec] = 0;
				vnvert[n_vec] = 1;
				++n_vec;
			}
			else if( PLUS_VAL(val[j+1]) )
			{
				verts[n_vert] = v->p[j+1];
				n_vert += 1;
				if(v->vncolor[i] >= 2 )
				{
					vncol[n_vec] = 1;
					colours[n_col] = v->c[col_index+1];
					n_col += 1;
				}
				else if(v->vncolor[i] >= 1 )
				{
					vncol[n_vec] = 1;
					colours[n_col] = v->c[col_index];
					n_col += 1;
				}
				else	vncol[n_vec] = 0;
				vnvert[n_vec] = 1;
				++n_vec;
			}
		}

		else	/* closed or open */
		{
/*
fprintf(stderr,"n %d vals\n",n);
*/
			if( n < 0 ) n = -n;
/*
			for(k=0;k<n;++k)
			{
fprintf(stderr,"%f ",val[j+k]);
			}
fprintf(stderr,"\n");
*/

			this = left = right = 0;

			for(k=0;k<n-1;++k)
			{
				if( PLUS_VAL(val[j+k]) && PLUS_VAL(val[j+k+1]))
				{
					verts[n_vert] = v->p[j+k];
					n_vert += 1;
					if(v->vncolor[i] >= 2 )
					{
						colours[n_col] =									 v->c[col_index+k];
						n_col += 1;
					}
					++this;
/*
fprintf(stderr,"++ this %d n_vert %d n_vec %d\n",this,n_vert,n_vec);
*/
				}
				else if( PLUS_VAL(val[j+k]) && NEG_VAL(val[j+k+1]))
				{

/* This is the end of a segment */

	++right;
	verts[n_vert] = v->p[j+k];
	++n_vert;
       	converge(&v->p[j+k],val[j+k], &v->p[j+k+1],val[j+k+1],
                                	 &verts[n_vert],&lambda,fun);
	++n_vert;
	this += 2;
	if(v->vncolor[i] >= 2 )
	{
		vncol[n_vec] = this;
		colours[n_col] = v->c[col_index+k];
		COL_INTERP(colours[n_col+1],
			v->c[col_index+k], v->c[col_index+k+1],lambda);
		n_col += 2;
	}
	else if(v->vncolor[i] == 1 )
	{
		vncol[n_vec] = 1;
		colours[n_col] = v->c[col_index];
		n_col += 1;
	}
	else	vncol[n_vec] = 0;
	vnvert[n_vec] = this;
	++n_vec;
/*
fprintf(stderr,"+- this %d n_vert %d n_vec %d\n",this,n_vert,n_vec);
*/
	this = 0;
				}
				else if(NEG_VAL(val[j+k])&&PLUS_VAL(val[j+k+1]))
				{

/* This is the start of a segment */

	++left;
       	converge(&v->p[j+k],val[j+k], &v->p[j+k+1],val[j+k+1],
                                	 &verts[n_vert],&lambda,fun);
	++n_vert;
	this = 1;
	if(v->vncolor[i] >= 2 )
	{
		COL_INTERP(colours[n_col],
			v->c[col_index+k], v->c[col_index+k+1],lambda);
		n_col += 1;
	}
/*
fprintf(stderr,"-+ this %d n_vert %d n_vec %d\n",this,n_vert,n_vec);
*/
				}
				else if(PLUS_VAL(val[j+k]) )
				{

/* This is the end of a segment */

	++right;
	verts[n_vert] = v->p[j+k];
	++n_vert;
	++this;
	if(v->vncolor[i] >= 2 )
	{
		vncol[n_vec] = this;
		colours[n_col] = v->c[col_index+k];
		n_col += 1;
	}
	else if(v->vncolor[i] == 1 )
	{
		vncol[n_vec] = 1;
		colours[n_col] = v->c[col_index];
		n_col += 1;
	}
	else	vncol[n_vec] = 0;
	vnvert[n_vec] = this;
	this = 0;
	++n_vec;
				}
				else if(PLUS_VAL(val[j+k+1]))
				{

/* This is the start of a segment */

	++left;
	this = 0;
	if(v->vncolor[i] >= 2 )
	{
	}
				}
			
			} /* end k loop */

/*********** now deal with final vertex ************/

			if( v->vnvert[i] < 0 ) /* closed */
			{
				if( PLUS_VAL(val[j+k]) && PLUS_VAL(val[j]))
				{
	verts[n_vert] = v->p[j+k];
	n_vert += 1;
	++this;
	if( left == 0 && right == 0 && this > 0 ) /* closed */
	{
		vnvert[n_vec] = v->vnvert[i];
		if(v->vncolor[i] >= 2 )
		{
			vncol[n_vec] = this;
			colours[n_col] =									 v->c[col_index+k];
			n_col += 1;
		}
		else if(v->vncolor[i] >= 1 )
		{
			vncol[n_vec] = 1;
			colours[n_col] = v->c[col_index];
			++n_col;
		}
		else	vncol[n_vec] = 0;

		++n_vec;
	}
	else /* got to be a strageling segment need to add p[j] */
	{
		verts[n_vert] = v->p[j];
		++n_vert; ++this;
		vnvert[n_vec] = this;
		if(v->vncolor[i] >= 2 )
		{
			vncol[n_vec] = this;
			colours[n_col] = v->c[col_index+k];
			colours[n_col+1] = v->c[col_index];
			n_col += 2;
		}
		else if(v->vncolor[i] >= 1 )
		{
			vncol[n_vec] = 1;
			colours[n_col] = v->c[col_index];
			++n_col;
		}
		else	vncol[n_vec] = 0;

		++n_vec;
	}
		
				}
				else if( PLUS_VAL(val[j+k]) && NEG_VAL(val[j]))
				{

/* This is the end of a segment */

	++right;
	verts[n_vert] = v->p[j+k];
	++n_vert;
       	converge(&v->p[j+k],val[j+k], &v->p[j],val[j],
                                	 &verts[n_vert],&lambda,fun);
	++n_vert;
	this += 2;
	if(v->vncolor[i] >= 2 )
	{
		vncol[n_vec] = this;
		colours[n_col] = v->c[col_index+k];
		COL_INTERP(colours[n_col+1],
			v->c[col_index+k], v->c[col_index],lambda);
		n_col += 2;
	}
	else if(v->vncolor[i] == 1 )
	{
		vncol[n_vec] = 1;
		colours[n_col] = v->c[col_index];
		n_col += 1;
	}
	else	vncol[n_vec] = 0;
	vnvert[n_vec] = this;
	this = 0;
	++n_vec;
				}
				else if(NEG_VAL(val[j+k])&&PLUS_VAL(val[j]))
				{

/* This is the start of a (very short) segment */

	++left;
       	converge(&v->p[j+k],val[j+k], &v->p[j],val[j],
                                	 &verts[n_vert],&lambda,fun);
	++n_vert;
	verts[n_vert] = v->p[j];
	++n_vert;
	this = 2;
	if(v->vncolor[i] >= 2 )
	{
		vncol[n_vec] = this;
		COL_INTERP(colours[n_col],
			v->c[col_index+k], v->c[col_index],lambda);
		colours[n_col+1] = v->c[col_index];
		n_col += 2;
	}
	else if(v->vncolor[i] == 1 )
	{
		vncol[n_vec] = 1;
		colours[n_col] = v->c[col_index];
		n_col += 1;
	}
	else	vncol[n_vec] = 0;
	vnvert[n_vec] = this;
	this = 0;
	++n_vec;
				}

				else if(PLUS_VAL(val[j+k]) )
				{

/* This is the end of a segment */

	++right;
	verts[n_vert] = v->p[j+k];
	++n_vert;
	++this;
	if(v->vncolor[i] >= 2 )
	{
		vncol[n_vec] = this;
		colours[n_col] = v->c[col_index+k];
		n_col += 1;
	}
	else if(v->vncolor[i] == 1 )
	{
		vncol[n_vec] = 1;
		colours[n_col] = v->c[col_index];
		n_col += 1;
	}
	else	vncol[n_vec] = 0;
	vnvert[n_vec] = this;
	this = 0;
	++n_vec;
				}
			
			}
			else	/* open */
			{
				if( PLUS_VAL(val[j+k]))
				{
	verts[n_vert] = v->p[j+k];
	n_vert += 1;
	++this;
	if(v->vncolor[i] >= 2 )
	{
		vncol[n_vec] = this;
		colours[n_col] =									 v->c[col_index+k];
		n_col += 1;
	}
	else if(v->vncolor[i] >= 1 )
	{
		vncol[n_vec] = 1;
		colours[n_col] = v->c[col_index];
		++n_col;
	}
	else	vncol[n_vec] = 0;

	/* got to be a strageling segment */
	
	vnvert[n_vec] = this;
	++n_vec;

		
				}
			
			}
		} /* end if close or open */
/*
fprintf(stderr,"end seg n_vert %d n_vec %d n_col %d colindex %d\n",
		n_vert,n_vec,n_col,col_index);
*/
			j += n;
			col_index += v->vncolor[i];
	}

	/* now have new values */
/*
fprintf(stderr,"Final %d %d %d\n",n_vec,n_vert,n_col);
*/

	OOGLFree(v->vnvert);
	OOGLFree(v->p);
	OOGLFree(v->c);
	v->vnvert = vnvert;
	v->vncolor = vncol;
	v->p = verts;
	v->c = colours;
	v->nvec = n_vec;
	v->nvert = n_vert;
	v->ncolor = n_col;

	if( !vSane(v) ) 
	{
		fprintf(stderr,"insane vect\n");
	}
}
#endif

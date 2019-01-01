/*
 * File:	ridge_intersect.c
 * Function:	intersects a geometry using (un)oriented vec fields
 * Author;	Richard Morris
 * Date		22/4/93
 */

#include <eqn.h>
#include <stdio.h>
#include "../lsmp.h"
#include "intersect.h"

/*
#define PRI_INFIN
*/
#define PRINT_VALS
#define PRINT_ERRS

/* We have an ever growing list of verticies */

HPoint3 *Iverts;
int	Ivert_cursize,Ivert_allocsize;
Iedge	*Iedgebase;

addIline(Ipoint *A, Ipoint *B)
{
	HPoint3 *pt;

	if(Ivert_cursize + 2 > Ivert_allocsize )
	{
		Ivert_allocsize += ALLOC_INC;
		Iverts = (HPoint3 *) realloc((void *) Iverts,
				sizeof(HPoint3) * Ivert_allocsize );
	}
	if(Iverts == NULL)
	{
		fprintf(stderr,"Out of space for verticies\n");
		return;
	}
	pt = Iverts + Ivert_cursize;
	pt->x = A->x; pt->y = A->y; pt->z = A->z; pt->w = A->w;
	++Ivert_cursize;
	pt = Iverts + Ivert_cursize;
	pt->x = B->x; pt->y = B->y; pt->z = B->z; pt->w = B->w;
	++Ivert_cursize;
}

addIpoint(Ipoint *A)
{
	HPoint3 *pt;

	if(Ivert_cursize == Ivert_allocsize)
	{
		Ivert_allocsize += ALLOC_INC;
		Iverts = (HPoint3 *) realloc((void *) Iverts,
				sizeof(HPoint3) * Ivert_allocsize );
	}
	if(Iverts == NULL)
	{
		fprintf(stderr,"Out of space for verticies\n");
		return;
	}
	pt = Iverts + Ivert_cursize;
	pt->x = A->x; pt->y = A->y; pt->z = A->z; pt->w = A->w;
	++Ivert_cursize;
}

/* now find an edge */

Iedge *getIedge(Ipoint *A,Ipoint *B)
{
	Iedge *edge;

	edge = Iedgebase;
	while( edge != NULL )
	{
		if( (edge->pt1 == A && edge->pt2 == B )
		 || (edge->pt1 == B && edge->pt2 == A ) )
			return(edge);
		edge = edge->next;
	}
	return(NULL);
}

Iedge *addIedge(Ipoint *pt1, Ipoint *pt2, Ipoint *pt3)
{
	Iedge *edge;

	edge = grballoc(Iedge);
	edge->pt1 = pt1;
	edge->pt2 = pt2;
	edge->pt3 = pt3;
	edge->next = Iedgebase;
	Iedgebase = edge;
	return(Iedgebase);
}

/*********** Now the convergence routines *******************/

Ipoint *Iconv(Ipoint *pt1, Ipoint *pt2,int itts,double (*fun)())
{
	float Ax,Ay,Az,Aw,Aval,Bx,By,Bz,Bw,Bval,Cx,Cy,Cz,Cw,Cval;
	Ipoint *pt3,*pt4,*pt5;

	/* first check coherent orientations */

	if( pt1->done && pt2->done )
	{
	}
	else if( pt2->done )
	{
		pt1->val = fun(pt1->x,pt1->y,pt1->z,pt1->w);
		pt1->done = TRUE;
	}
	else if( pt1->done )
	{
		pt2->val = fun(pt2->x,pt2->y,pt2->z,pt2->w);
		pt2->done = TRUE;
	}
	else	/* neither is done */
	{
		pt1->val = fun(pt1->x,pt1->y,pt1->z,pt1->w);
		pt1->done = TRUE;
		pt2->val = fun(pt2->x,pt2->y,pt2->z,pt2->w);
		pt2->done = TRUE;
	}
/*
fprintf(stderr,"\nIconv\n");
fprintf(stderr,"pt1 (%f,%f,%f) val %f done %d\n",
		pt1->x,pt1->y,pt1->z,
		pt1->val,pt1->done);
fprintf(stderr,"pt2 (%f,%f,%f) val %f done %d\n",
		pt2->x,pt2->y,pt2->z,
		pt2->val,pt2->done);
*/

	/* Now have coherent orientations */

	if( pt1->val == 0.0 )
	{
			pt3 = grballoc(Ipoint);
			pt3->x = pt1->x;
			pt3->y = pt1->y;
			pt3->z = pt1->z;
			pt3->w = pt1->w;
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
	}
	if( pt2->val == 0.0 )
	{
			pt3 = grballoc(Ipoint);
			pt3->x = pt2->x;
			pt3->y = pt2->y;
			pt3->z = pt1->z;
			pt3->w = pt1->w;
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
	}
	if( pt1->val != pt1->val || pt2->val != pt2->val ) 
		return(NULL);
	if( (pt1->val > 0.0 && pt2->val > 0.0 ) 
	 || (pt1->val < 0.0 && pt2->val < 0.0 ) )
		return(NULL);

	/* now converge */

	Ax = pt1->x; Ay = pt1->y; 
	Az = pt1->z; Aw = pt1->w; 
	Bx = pt2->x; By = pt2->y;
	Bz = pt2->z; Bw = pt2->w;
	Aval = pt1->val; Bval = pt2->val;

	while(1)
	{
		Cx = (Ax + Bx)/2; Cy = (Ay + By)/2;
		Cz = (Az + Bz)/2; Cw = (Aw + Bw)/2;
		Cval = fun(Cx,Cy,Cz,Cw);
		if( Cval != Cval )
			return(NULL);
/*
fprintf(stderr,"Bisect (%f,%f) p (%f,%f,%f) val %f\n",
		Cx,Cy,Cp.x,Cp.y,Cp.z,Cval);		
*/
		if( ++itts > max_itt || Cval == 0.0 )
		{
			pt3 = grballoc(Ipoint);
			pt3->x = Cx;
			pt3->y = Cy;
			pt3->z = Cz;
			pt3->w = Cw;
			pt3->done = TRUE;
			pt3->next = NULL;
			return(pt3);
		}

		/* Now check things are sensible */

		if( ( Aval > 0.0 && Cval > 0.0 && Bval < 0.0 )
		 || ( Aval < 0.0 && Cval < 0.0 && Bval > 0.0 ) )
		{
			Ax = Cx; Ay = Cy; Az = Cz; Aw = Cw; Aval = Cval;
		}
		else if( ( Aval > 0.0 && Cval < 0.0 && Bval < 0.0 )
		      || ( Aval < 0.0 && Cval > 0.0 && Bval > 0.0 ) )
		{
			Bx = Cx; By = Cy; Bz = Cz; Bw = Cw; Bval = Cval;
		}
		else
		{
#ifdef PRINT_ERRS
			fprintf(stderr,"Whats gone wrong!\n");
			fprintf(stderr,"A (%f,%f,%f,%f) val %f\n",
				Ax,Ay,Az,Aw,Aval);
			fprintf(stderr,"B (%f,%f,%f,%f) val %f\n",
				Bx,By,Bz,Bw,Bval);
			fprintf(stderr,"C (%f,%f,%f,%f) val %f\n",
				Cx,Cy,Cz,Cw,Cval);
#endif
			return(NULL);
		}
	}
}

Ipoint *Iconverge(Ipoint *pt1, Ipoint *pt2,double (*fun)())
{
	return( Iconv(pt1,pt2,0,fun) );
}

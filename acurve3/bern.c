/*
 * Copyright I guess there should be some copywrite for this package,
 * 
 * 			Copyright (c) 1992
 * 
 * 	Liverpool University Department of Pure Mathematics,
 * 	Liverpool, L69 3BX, England.
 * 
 * 	Author Dr R. J. Morris.
 * 
 * 	e-mail rmorris@uk.ac.liv.uxb
 *
 */
/************************************************************************/
/*									*/
/*	Some functions for using bernstein polynomials.			*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <math.h>
#define I_AM_BERN
#include "bern.h"
#include "cells.h"

/*
#define QUICK_MAT
*/

/*** All positive and all negative bernstein's ***/

bern3D pb3, nb3;
bern3D *posbern3D = &pb3;
bern3D *negbern3D = &nb3;
bern2D pb2, nb2;
bern2D *posbern2D = &pb2;
bern2D *negbern2D = &nb2;
bern1D pb1, nb1;
bern1D *posbern1D = &pb1;
bern1D *negbern1D = &nb1;

double *pyr3Dbase;

/*** Macros for getting elements from arrays ***/

#define ele3D(bb,i,j,k) (*(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k))
#define ele2D(bb,i,j)	(*(bb->array+(i*(bb->yord+1)+j)))
#define ele1D(bb,i)	(*(bb->array+i))

#define MAX(a,b) ((a)>(b)?(a):(b))

/*** calculate nCm ***/

int comb(n,m)   /* rewritten in double so can get up to 13! */
int n,m;
{
   register int j;
   register double a;

   a = 1.0;
   for(j=m+1;j<=n;j++) a *= (double) j;
   for(j=1;j<=n-m;j++) a /= (double) j;
   j = (int) (a+0.001);
   if( fabs( a - j ) > 1.0e-3 ) fprintf(stderr,"comb: error n %d m %d %.12f %d\n",n,m,a,j);
   return(j);
/* the old way of doing it
   register int i=1,j;

   for(j=m+1;j<=n;j++) i *= j;
   for(j=1;j<=n-m;j++) i /= j;
   return(i);
*/
}

/*** print x as either -0,0,+0 or x ***/

printzero(x)
double x;
{
  if( x >= 0.0000005 || x <= -0.0000005 )  fprintf(stderr,"%9.6f ",x);
  else if( x == 0.0 )                      fprintf(stderr," 0        ");
  else if( x  > 0.0 )                      fprintf(stderr,"+0        ");
  else if( x  < 0.0 )                      fprintf(stderr,"-0        ");
  else					   fprintf(stderr,"! %f ",x);
}

/*** print out the bernstein polynomials ***/

printbern1D(bb)
bern1D *bb;
{
	int i;

	if(bb == posbern1D ) fprintf(stderr,"posative 1D bern\n");
	else if( bb == negbern1D )  fprintf(stderr,"negative 1D bern\n");
	else for(i=0;i<=bb->ord;++i) printzero( *(bb->array+i) );
}

printbern2D(bb)
bern2D *bb;
{
	int i,j;

	if(bb == posbern2D ) fprintf(stderr,"posative 2D bern\n");
	else if( bb == negbern2D )  fprintf(stderr,"negative 2D bern\n");
	else for(j=bb->yord;j>=0;--j)
	{
		for(i=0;i<=bb->xord;++i) printzero( *(bb->array+i*(bb->yord+1)+j) );
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"\n");
}

printbern3D(bb)
bern3D *bb;
{
	int i,j,k;

	if(bb == posbern3D ) fprintf(stderr,"posative 3D bern\n");
	else if( bb == negbern3D )  fprintf(stderr,"negative 3D bern\n");
	else for(k=bb->zord;k>=0;--k)
	{
	   fprintf(stderr,"z^%d\n",k);
	   for(j=bb->yord;j>=0;--j)
	   {
		for(i=0;i<=bb->xord;++i)
			printzero( *(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k) );
		fprintf(stderr,"\t\ty^%d\n",j);
	   }
	}
}

/****************** Memory allocation **************************************/

struct mat3Dlist { struct mat3Dlist *next; };
int	mat3Dords[5][3] = {{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1}};
struct mat3Dlist *list3Dmats[5] = {NULL,NULL,NULL,NULL,NULL};

struct mat2Dlist { struct mat2Dlist *next; };
int	mat2Dords[15][2] = {{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
	{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
struct mat2Dlist *list2Dmats[15] = {NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

struct mat2Dlist *list2Dberns = NULL;

struct mat1Dlist { struct mat1Dlist *next; };
int	mat1Dords[15] = {-1,-1,-1,-1,-1, -1,-1,-1,-1,-1, -1,-1,-1,-1,-1};
struct mat1Dlist *list1Dmats[15] = {NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

/*
 * Function:	init_bern
 * Action:	create the lists of free bernsteins.
 */

init_berns(a,b,c,d,e)
bern3D *a,*b,*c,*d,*e;
{
	int i,pyr3Dsize;
	struct mat3Dlist *temp,*temp2;
	struct mat2Dlist *temp3,*temp4;
	struct mat1Dlist *temp5,*temp6;

	pyr3Dsize = (a->xord*2+1)*(a->yord*2+1)*(a->zord*2+1);
	pyr3Dsize = MAX(pyr3Dsize,
			(b->xord*2+1)*(b->yord*2+1)*(b->zord*2+1));
	pyr3Dsize = MAX(pyr3Dsize,
			(c->xord*2+1)*(c->yord*2+1)*(c->zord*2+1));
	pyr3Dsize = MAX(pyr3Dsize,
			(d->xord*2+1)*(d->yord*2+1)*(d->zord*2+1));
	pyr3Dsize = MAX(pyr3Dsize,
			(e->xord*2+1)*(e->yord*2+1)*(e->zord*2+1));

	pyr3Dbase = (double *) calloc(pyr3Dsize,sizeof(double));

	/* Now go through the existing list and freeing those not used */

	for(i=0;i<5;++i)
	{
		if( ( mat3Dords[i][0] != a->xord || mat3Dords[i][1] != a->yord
		    || mat3Dords[i][2] != a->zord)
		 && ( mat3Dords[i][0] != b->xord || mat3Dords[i][1] != b->yord
		    || mat3Dords[i][2] != b->zord)
		 && ( mat3Dords[i][0] != c->xord || mat3Dords[i][1] != c->yord
		    || mat3Dords[i][2] != c->zord)
		 && ( mat3Dords[i][0] != d->xord || mat3Dords[i][1] != d->yord
		    || mat3Dords[i][2] != d->zord) 
		 && ( mat3Dords[i][0] != e->xord || mat3Dords[i][1] != e->yord
		    || mat3Dords[i][2] != e->zord) )
		{
			temp = list3Dmats[i];
			temp2 = temp;
			while(temp != NULL)
			{
				temp2 = temp->next;
				free(temp);
				temp = temp2;
			}
			list3Dmats[i] = NULL;
			mat3Dords[i][0]= mat3Dords[i][1] = mat3Dords[i][2] = -1;
		}
	}

	for(i=0;i<15;++i)
	{
		if( ( mat2Dords[i][0] != a->xord||mat2Dords[i][1] != a->yord )
		 && ( mat2Dords[i][0] != a->xord||mat2Dords[i][1] != a->zord )
		 && ( mat2Dords[i][0] != a->yord||mat2Dords[i][1] != a->zord )
		 && ( mat2Dords[i][0] != b->xord||mat2Dords[i][1] != b->yord )
		 && ( mat2Dords[i][0] != b->xord||mat2Dords[i][1] != b->zord )
		 && ( mat2Dords[i][0] != b->yord||mat2Dords[i][1] != b->zord )
		 && ( mat2Dords[i][0] != c->xord||mat2Dords[i][1] != c->yord )
		 && ( mat2Dords[i][0] != c->xord||mat2Dords[i][1] != c->zord )
		 && ( mat2Dords[i][0] != c->yord||mat2Dords[i][1] != c->zord )
		 && ( mat2Dords[i][0] != d->xord||mat2Dords[i][1] != d->yord )
		 && ( mat2Dords[i][0] != d->xord||mat2Dords[i][1] != d->zord )
		 && ( mat2Dords[i][0] != d->yord||mat2Dords[i][1] != d->zord )
		 && ( mat2Dords[i][0] != e->xord||mat2Dords[i][1] != e->yord )
		 && ( mat2Dords[i][0] != e->xord||mat2Dords[i][1] != e->zord )
		 && ( mat2Dords[i][0] != e->yord||mat2Dords[i][1] != e->zord ))
		{
			temp3 = list2Dmats[i];
			while(temp3 != NULL)
			{
				temp4 = temp3->next;
				free(temp3);
				temp3 = temp4;
			}
			list2Dmats[i] = NULL;
			mat2Dords[i][0]= mat2Dords[i][1] = -1;
		}
	}

	for(i=0;i<15;++i)
	{
		if( mat1Dords[i] != a->xord
		 && mat1Dords[i] != a->yord
		 && mat1Dords[i] != a->zord
		 && mat1Dords[i] != b->xord
		 && mat1Dords[i] != b->yord
		 && mat1Dords[i] != b->zord
		 && mat1Dords[i] != c->xord
		 && mat1Dords[i] != c->yord
		 && mat1Dords[i] != c->zord
		 && mat1Dords[i] != d->xord
		 && mat1Dords[i] != d->yord
		 && mat1Dords[i] != d->zord
		 && mat1Dords[i] != e->xord
		 && mat1Dords[i] != e->yord
		 && mat1Dords[i] != e->zord )
		{
			temp5 = list1Dmats[i];
			while(temp5 != NULL)
			{
				temp6 = temp5->next;
				free(temp5);
				temp5 = temp6;
			}
			list1Dmats[i] = NULL;
			mat1Dords[i] = -1;
		}
	}
}
			
/*
 * Function:	alloc_bern
 * Action:	allocates space for 3D bern size xord,yord,zord
 */

bern3D *alloc_bern3D(xord,yord,zord)
int xord,yord,zord;
{
	bern3D *temp;
	int	i;

	temp = (bern3D *) malloc(sizeof(bern3D));
	temp->xord = xord;
	temp->yord = yord;
	temp->zord = zord;

	/* First find list */

	for(i=0;i<5;++i)
	{
		if( mat3Dords[i][0] == xord
		 && mat3Dords[i][1] == yord
		 && mat3Dords[i][2] == zord )
		{
		    if(list3Dmats[i] != NULL )
		    {
			temp->xord = xord;
			temp->yord = yord;
			temp->zord = zord;
			temp->array = (double *) list3Dmats[i];
			list3Dmats[i] = list3Dmats[i]->next;
			return(temp);
		    }
		    else
		    {
			temp->xord = xord;
			temp->yord = yord;
			temp->zord = zord;
			temp->array = (double *)
			    calloc((size_t) (xord+1)*(yord+1)*(zord+1),sizeof(double));
			return(temp);
		    }
		}
	}
	temp->array = (double *)
		calloc((size_t) (xord+1)*(yord+1)*(zord+1),sizeof(double));
#ifdef PRI_ALLOC_BERN
	if((mat3Dords[0][0]== -1&&mat3Dords[0][1] == -1&&mat3Dords[0][2] == -1)
	 ||(mat3Dords[1][0]== -1&&mat3Dords[1][1] == -1&&mat3Dords[1][2] == -1)
	 ||(mat3Dords[2][0]== -1&&mat3Dords[2][1] == -1&&mat3Dords[2][2] == -1)
	 ||(mat3Dords[3][0]== -1&&mat3Dords[3][1] == -1&&mat3Dords[3][2] == -1)
	 ||(mat3Dords[4][0]== -1&&mat3Dords[4][1] == -1&&mat3Dords[4][2] == -1))
		return(temp);
	fprintf(stderr,"alloc_bern3D: error couldn't find match %d %d %d\n",
		xord,yord,zord);
	for(i=0;i<5;++i)
		fprintf(stderr,"ords[%d] %d %d %d\n",i,
			mat3Dords[i][0],mat3Dords[i][1],mat3Dords[i][2]);
#endif
	return(temp);
}


/*
 * Function:	free_bern3D
 * Action:	frees the space used by bern poly
 */

free_bern3D(bb)
bern3D *bb;
{
	struct mat3Dlist *list;
	int i;

	if(bb == posbern3D || bb == negbern3D ) return;
	for(i=0;i<5;++i)
	{
		if( mat3Dords[i][0] == bb->xord
		 && mat3Dords[i][1] == bb->yord
		 && mat3Dords[i][2] == bb->zord )
		{
			list = (struct mat3Dlist *) bb->array;
			list->next = list3Dmats[i];
			list3Dmats[i] = list;
			free(bb);
			return;
		}
	}

	/* Failed to find match  fill in empty mats */

	for(i=0;i<5;++i)
	{
		if( mat3Dords[i][0] == -1
		 && mat3Dords[i][1] == -1
		 && mat3Dords[i][2] == -1)
		{
			mat3Dords[i][0] = bb->xord;
			mat3Dords[i][1] = bb->yord;
			mat3Dords[i][2] = bb->zord;
			list = (struct mat3Dlist *) bb->array;
			list->next = list3Dmats[i];
			list3Dmats[i] = list;
			free(bb);
			return;
		}
	}
	fprintf(stderr,"free_bern3D_mat: error couldn't find match\n");

/*
	fprintf(stderr,"free_bern3D_mat: count %d\n",--matcount);
*/
	free(bb->array);
	free(bb);
}

bern2D *alloc_bern2D(xord,yord)
int xord,yord;
{
	bern2D *temp;
	int	i;

	if( list2Dberns != NULL )
	{
		temp = (bern2D *) list2Dberns;
		list2Dberns = list2Dberns->next;
	}
	else
		temp = (bern2D *) malloc(sizeof(bern2D));

	temp->xord = xord;
	temp->yord = yord;

	/* First find list */

	for(i=0;i<15;++i)
	{
		if( mat2Dords[i][0] == xord
		 && mat2Dords[i][1] == yord )
		{
		    if(list2Dmats[i] != NULL )
		    {
			temp->xord = xord;
			temp->yord = yord;
			temp->array = (double *) list2Dmats[i];
			list2Dmats[i] = list2Dmats[i]->next;
			return(temp);
		    }
		    else
		    {
			temp->xord = xord;
			temp->yord = yord;
#ifdef CALLOC_TEST
fprintf(stderr,"calloc(%d,%d)\n",(xord+1)*(yord+1),sizeof(double));
#endif
			temp->array = (double *)
			    calloc((size_t) (xord+1)*(yord+1),sizeof(double));
			return(temp);
		    }
		}
	}
	temp->xord = xord;
	temp->yord = yord;
	temp->array = (double *)
		calloc((size_t) (xord+1)*(yord+1),sizeof(double));
	if(mat2Dords[14][0]== -1 && mat2Dords[14][1] == -1)
		return(temp);
/*
	fprintf(stderr,"alloc_bern2D_mat: error couldn't find match %d %d\n",
		xord,yord);
	for(i=0;i<15;++i)
		fprintf(stderr,"ords[%d] %d %d\n",i,
			mat2Dords[i][0],mat2Dords[i][1]);
*/
	return(temp);
}

free_bern2D(bb)
bern2D *bb;
{
	struct mat2Dlist *list;
	int i;

	if(bb == posbern2D || bb == negbern2D ) return;
	for(i=0;i<15;++i)
	{
		if( mat2Dords[i][0] == bb->xord
		 && mat2Dords[i][1] == bb->yord )
		{
			list = (struct mat2Dlist *) bb->array;
			list->next = list2Dmats[i];
			list2Dmats[i] = list;
/*
			free(bb);
*/
			list = (struct mat2Dlist *) bb;
			list->next = list2Dberns;
			list2Dberns = list;
			return;
		}
	}

	/* Failed to find match  fill in empty mats */

	for(i=0;i<15;++i)
	{
		if( mat2Dords[i][0] == -1
		 && mat2Dords[i][1] == -1 )
		{
			mat2Dords[i][0] = bb->xord;
			mat2Dords[i][1] = bb->yord;
			list = (struct mat2Dlist *) bb->array;
			list->next = list2Dmats[i];
			list2Dmats[i] = list;
/*
			free(bb);
*/
			list = (struct mat2Dlist *) bb;
			list->next = list2Dberns;
			list2Dberns = list;
			return;
		}
	}
	fprintf(stderr,"free_bern2D_mat: error couldn't find match %d %d\n",
		bb->xord,bb->yord);
	for(i=0;i<15;++i)
		fprintf(stderr,"ords[%d] %d %d\n",i,
			mat2Dords[i][0],mat2Dords[i][1]);

/*
	fprintf(stderr,"free_bern2D_mat: count %d\n",--matcount);
	free(bb->array);
	free(bb);
*/
			list = (struct mat2Dlist *) bb;
			list->next = list2Dberns;
			list2Dberns = list;
}

bern1D *alloc_bern1D(xord)
int xord;
{
	bern1D *temp;
	int	i;

	temp = (bern1D *) malloc(sizeof(bern1D));
	temp->ord = xord;

	/* First find list */

	for(i=0;i<15;++i)
	{
		if( mat1Dords[i] == xord )
		{
		    if(list1Dmats[i] != NULL )
		    {
			temp->array = (double *) list1Dmats[i];
			list1Dmats[i] = list1Dmats[i]->next;
			return(temp);
		    }
		    else
		    {
			temp->array = (double *)
			    calloc((size_t) (xord+1),sizeof(double));
			return(temp);
		    }
		}
	}
	temp->array = (double *)
		calloc((size_t) (xord+1),sizeof(double));
	if(mat1Dords[14]== -1 ) return(temp);
	fprintf(stderr,"alloc_bern1D: error couldn't find match %d\n",
		xord);
	for(i=0;i<15;++i)
		fprintf(stderr,"ords[%d] %d\n",i,mat1Dords[i]);
	return(temp);
}


/*
 * Function:	free_bern1D
 * Action:	frees the space used by bern poly
 */

free_bern1D(bb)
bern1D *bb;
{
	struct mat1Dlist *list;
	int i;

	if(bb == posbern1D || bb == negbern1D ) return;
	for(i=0;i<15;++i)
	{
		if( mat1Dords[i] == bb->ord )
		{
			list = (struct mat1Dlist *) bb->array;
			list->next = list1Dmats[i];
			list1Dmats[i] = list;
			free(bb);
			return;
		}
	}

	/* Failed to find match  fill in empty mats */

	for(i=0;i<15;++i)
	{
		if( mat1Dords[i] == -1 )
		{
			mat1Dords[i] = bb->ord;
			list = (struct mat1Dlist *) bb->array;
			list->next = list1Dmats[i];
			list1Dmats[i] = list;
			free(bb);
			return;
		}
	}
	fprintf(stderr,"free_bern1D_mat: error couldn't find match\n");

/*
	fprintf(stderr,"free_bern1D_mat: count %d\n",--matcount);
*/
	free(bb->array);
	free(bb);
}

/*
 * Function:	free_octbern3D
 * Action:	frees the elements of an octbern3D
 */

free_octbern3D(bb)
octbern3D bb;
{
	if(bb.lfd == posbern3D || bb.lfd == negbern3D ) return;
	free_bern3D(bb.lfd);
	free_bern3D(bb.rfd);
	free_bern3D(bb.lbd);
	free_bern3D(bb.rbd);
	free_bern3D(bb.lfu);
	free_bern3D(bb.rfu);
	free_bern3D(bb.lbu);
	free_bern3D(bb.rbu);
}

free_quadbern2D(bb)
quadbern2D bb;
{
	if(bb.lt == posbern2D || bb.lt == negbern2D ) return;
	free_bern2D(bb.lt);
	free_bern2D(bb.rt);
	free_bern2D(bb.lb);
	free_bern2D(bb.rb);
}

free_binbern1D(bb)
binbern1D bb;
{
	if(bb.l == posbern1D || bb.l == negbern1D ) return;
	free_bern1D(bb.l);
	free_bern1D(bb.r);
}

/* checks if every element of a 3d array has strictly the same sign */

int allonesign3D(bb)
bern3D *bb;
{
   register int i,j,k,max;
   register double *ele;

   if( bb == posbern3D ) return(1);
   if( bb == negbern3D ) return(-1);
   max = (bb->xord+1)*(bb->yord+1)*(bb->zord+1)-1;
   ele = bb->array+1;
   if( *bb->array < 0)
   {
	for(;max>0;--max,++ele)
		if( *ele >= 0.0 ) return(0);
/*
   for(ele=1;ele<max;++ele)
	     if( *(bb->array+ ele ) >= 0.0 ) return(0); 
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
            if( *(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k) >= 0.0)
		return(0);
*/
   return(-1);
   }
   else
   {
	for(;max>0;--max,++ele)
		if( *ele <= 0.0 ) return(0);
/*
   for(ele=1;ele<max;++ele)
	     if( *(bb->array+ ele ) <= 0.0 ) return(0); 
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
            if( *(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k) <= 0.0)
		return(0);
*/
   return(1);
   }
}

/* checks if every element of a 2d array has strictly the same sign */

int allonesign2D(bb)
bern2D *bb;
{
   register int i,j,max;
   register double *ele;

   if( bb == posbern2D ) return(1);
   if( bb == negbern2D ) return(-1);
   max = (bb->xord+1)*(bb->yord+1)-1;
   ele = bb->array+1;
   if( *bb->array < 0)
   {
	for(;max>0;--max,++ele)
		if( *ele >= 0.0 ) return(0);
/*
   for(ele=1;ele<max;++ele)
	     if( *(bb->array+ ele ) >= 0.0 ) return(0); 
   for(i=0;i<=bb->xord;i++)
      for(j=0 ;j<=bb->yord;j++)
            if( *(bb->array+i*(bb->yord+1)+j) >= 0.0) return(0);
*/
   return(-1);
   }
   else
   {
	for(;max>0;--max,++ele)
		if( *ele <= 0.0 ) return(0);
/*
   for(ele=1;ele<max;++ele)
	    if( *(bb->array+ ele ) <= 0.0) return(0);
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
            if( *(bb->array+i*(bb->yord+1)+j) <= 0.0) return(0);
*/
   return(1);
   }
}

/* checks if every element of a 1d array has strictly the same sign */

int allonesign1D(bb)
bern1D *bb;
{
	register int i, max;

   if( bb == posbern1D ) return(1);
   if( bb == negbern1D ) return(-1);
   max = bb->ord;
	if( *(bb->array) < 0)
	{
	for( i=1; i<= max; ++i)
		if( *(bb->array+i) >= 0.0 ) return(0);
	return(-1);
	}
	else
	{
	for( i=0; i<= max; ++i)
		if( *(bb->array+i) <= 0.0 ) return(0);
	return(1);
	}
}

/* check the derivatives have strictly the same sign */

int allonesignderiv1D(bb)
bern1D *bb;
{
	int i;
	if(bb->ord == 0)
	{
		fprintf(stderr,"bb->ord == 0\n");
		return(0);
	}
	if(*(bb->array+1) - *bb->array < 0)
	{
	for( i=1; i<=bb->ord - 1; ++i)
	if( *(bb->array+i+1) - *(bb->array+i) >= 0.0 ) return(0);
	return(-1);
	}
	else
	{
	for( i=0; i<=bb->ord - 1; ++i)
	if( *(bb->array+i+1) - *(bb->array+i) <= 0.0 ) return(0);
	return(1);
	}
}

/**************************************************************/
/*                                                            */
/*     input 'aa'   an array such that aa(i,j,k) is coeff of  */
/*                  x^i y^j z^k                               */
/*                                                            */
/**************************************************************/

formbernstein1D(aa,bb,ord,min,max)
double aa[MAXORDER];
bern1D *bb;
int ord;
double min,max;
{
	double d[MAXORDER][MAXORDERP2];
	int i,j;

	/* We think of the polynomial a as 				*/
	/* a0 + ( a1 + ( a2 + a3 x ) x ) x 				*/
	/* And write d01 = a3						*/
	/* first we convert ( a2 + a3 x ) to bernstein form to give 	*/
	/*	a2 (1-x) + (a2+a3) x = 	d11 (1-x) + d12 x		*/
	/* Now we look at ( a1 + ( d11 (1-x) + d12 x ) x) )		*/
	/*  = a1 (1-x)^2 + (d11 + 2 a1)(1-x)x + (d12 - a1) x^2		*/
	/*  = d21 (1-x)^2 + d22 (1-x)x + d23 x^2			*/
	/*	and so on						*/
	/*								*/


      bb->ord = ord;

      d[0][1] = aa[ord];
      for(i = 1; i <= ord; i++)
      {
         d[i-1][0] = 0;
         d[i-1][i+1] = 0;

         for( j = 0 ;j <= i;j++)
            d[i][j+1] = max*d[i-1][j] + min*d[i-1][j+1] + comb(i,j)*aa[ord-i];
      };
      for( i = 0 ; i<= ord; i++)
	 *(bb->array+i) = d[ord][i+1] / comb(ord,i);
}

/****************************************************************/
/*								*/
/*	formbernstein creates a bernstein polynomial bb 	*/
/*	from a polynomial aa for the region [xmin,xmax] X ...   */
/*								*/
/****************************************************************/

bern3D *formbernstein3D(aa,xmin,xmax,ymin,ymax,zmin,zmax)
double aa[MAXORDER][MAXORDER][MAXORDER];
double xmin,xmax,ymin,ymax,zmin,zmax;
{
   bern3D *bb;
   double c[MAXORDER];
   bern1D *d;
   int row,col,stack;
   int xord,yord,zord;

	/*** first convert polynomials in z ***/

   order_poly3(aa,&xord,&yord,&zord);
   bb = alloc_bern3D(xord,yord,zord);
   d = alloc_bern1D(MAXORDER);

   for(row = 0; row <= xord; row++) 
   for(col = 0; col <= yord; col++)
   {
      for( stack = 0; stack <= zord; ++stack) c[stack] = aa[row][col][stack];

      formbernstein1D(c,d,zord,zmin,zmax);

      for( stack = 0; stack <= zord; ++stack)
		*(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack)
			 = *(d->array+stack);
   }

	/*** next polynomials in y ***/

   for(row = 0; row <= xord; row++)
   for(stack = 0; stack <= zord; ++stack)
   {
      for(col = 0; col <= yord; col++)
		c[col] = *(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack);
      formbernstein1D(c,d,yord,ymin,ymax);
      for(col = 0; col <= yord; col++)
		*(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack)
			 = *(d->array+col);
   }
	/*** Finally polynomial in x ***/

   for(col = 0; col <= yord; col++)
   for(stack = 0; stack <= zord; ++stack)
   {
      for(row = 0; row <= xord; row++)
		c[row] = *(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack);
      formbernstein1D(c,d,xord,xmin,xmax);
      for(row = 0; row <= xord; row++)
		*(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack)
			 = *(d->array+row);
   }
   free_bern1D(d);
   return(bb);
}

/************************************************************************/
/*									*/
/*	Three routines to differentiate bernstein polynomials.		*/
/*	uses the property that the difference of two coefficients	*/
/*	of the bernstein is the coefficient of the derivative.		*/
/*									*/
/*	We also need to multiply each term by the degree of the poly eg.*/
/*	if	f = a x^2 + 2 b x(1-x) + c (1-x)^2			*/
/*	then	f' = 2(a-b)x + 2(b-c)(1-x)				*/
/************************************************************************/

bern3D *diffx3D(bb)
bern3D *bb;
{
   int row,col,stack;
   bern3D *xderiv;

   if( bb == posbern3D || bb == negbern3D )
   {
	fprintf(stderr,"diffx3D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->xord == 0 )
   {
	xderiv = alloc_bern3D(0,0,0);
	*(xderiv->array) = 0.0;
	return(xderiv);
   }
   xderiv = alloc_bern3D(bb->xord-1,bb->yord,bb->zord);
   for(row=0; row<= bb->xord-1; row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0;stack<=bb->zord;stack++)
	*(xderiv->array+(row*(xderiv->yord+1)+col)*(xderiv->zord+1)+stack) =
		bb->xord *
		( *(bb->array+((row+1)*(bb->yord+1)+col)*(bb->zord+1)+stack)
		 - *(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack));
   return(xderiv);
}

bern3D *diffy3D(bb)
bern3D *bb;
{
   int row,col,stack;
   bern3D *yderiv;

   if( bb == posbern3D || bb == negbern3D )
   {
	fprintf(stderr,"diffy3D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->yord == 0)
   {
	yderiv = alloc_bern3D(0,0,0);
	*(yderiv->array) = 0.0;
	return(yderiv);
   }
   yderiv = alloc_bern3D(bb->xord,bb->yord-1,bb->zord);
   for(row=0; row<= bb->xord; row++)
   for(col=0;col<=bb->yord-1;col++)
   for(stack=0;stack<=bb->zord;stack++)
	*(yderiv->array+(row*(yderiv->yord+1)+col)*(yderiv->zord+1)+stack) =
		bb->yord *
		( *(bb->array+(row*(bb->yord+1)+col+1)*(bb->zord+1)+stack)
		 - *(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack));
   return(yderiv);
}

bern3D *diffz3D(bb)
bern3D *bb;
{
   int row,col,stack;
   bern3D *zderiv;

   if( bb == posbern3D || bb == negbern3D )
   {
	fprintf(stderr,"diffz3D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->zord == 0)
   {
	zderiv = alloc_bern3D(0,0,0);
	*(zderiv->array) = 0.0;
	return(zderiv);
   }
   zderiv = alloc_bern3D(bb->xord,bb->yord,bb->zord-1);

   for(row=0; row<= bb->xord; row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0;stack<=bb->zord-1;stack++)
	*(zderiv->array+(row*(zderiv->yord+1)+col)*(zderiv->zord+1)+stack) =
		bb->zord *
		( *(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack+1)
		 - *(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack));
   return(zderiv);
}

bern2D *diffx2D(bb)
bern2D *bb;
{
   int row,col;
   bern2D *xderiv;

   if( bb == posbern2D || bb == negbern2D )
   {
	fprintf(stderr,"diffx2D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->xord == 0)
   {
	xderiv = alloc_bern2D(0,0);
	*(xderiv->array) = 0.0;
	return(xderiv);
   }
   xderiv = alloc_bern2D(bb->xord-1,bb->yord);

   for(row=0; row<= bb->xord-1; row++)
   for(col=0;col<=bb->yord;col++)
	*(xderiv->array+row*(xderiv->yord+1)+col) = bb->xord *
		( *(bb->array+(row+1)*(bb->yord+1)+col)
		 - *(bb->array+row*(bb->yord+1)+col));
   return(xderiv);
}

bern2D *diffy2D(bb)
bern2D *bb;
{
   int row,col;
   bern2D *yderiv;

   if( bb == posbern2D || bb == negbern2D )
   {
	fprintf(stderr,"diffy2D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->yord == 0)
   {
	yderiv = alloc_bern2D(0,0);
	*(yderiv->array) = 0.0;
	return(yderiv);
   }
   yderiv = alloc_bern2D(bb->xord,bb->yord-1);

   for(row=0; row<= bb->xord-1; row++)
   for(col=0;col<=bb->yord;col++)
	*(yderiv->array+row*(yderiv->yord+1)+col) = bb->yord *
		( *(bb->array+row*(bb->yord+1)+col+1)
		- *(bb->array+row*(bb->yord+1)+col));
   return(yderiv);
}

bern1D *diff1D(bb)
bern1D *bb;
{
	int i;
	bern1D *xderiv;

   if( bb == posbern1D || bb == negbern1D )
   {
	fprintf(stderr,"diff1D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->ord == 0)
   {
	xderiv = alloc_bern1D(0);
	*(xderiv->array) = 0.0;
	return(xderiv);
   }
	xderiv = alloc_bern1D(bb->ord-1);
	for(i=0; i<=bb->ord-1; ++i)
		*(xderiv->array+i) =
		  bb->ord *( *(bb->array+i+1) - *(bb->array+i));
	return(xderiv);
}

/************************************************************************/
/*									*/
/*	reduce3D(bb,b1,...,b8)	creates eight new bernstein polys	*/
/*	each one defining the surface in one eigth of the unit cube.	*/
/*									*/
/************************************************************************/

#ifndef QUICK_MAT
/*
*/
double pyramid[MAXORDERT2][MAXORDERT2][MAXORDERT2]; 

#define pyr3Dele(R,C,S) (\
	pyramid[R][C][S] )
/*
#define pyr3Dele(R,C,S) *(\
	pyr3Dbase + ( (R) * (2*bb->yord+1) + C)*(2*bb->zord+1) + S)
*/

octbern3D reduce3D(bb)
bern3D *bb;
{
   register int row,col,stack,level;
   register double *ele,*ele2,*pyr;
   octbern3D temp;

   if( bb == posbern3D )
   {
	temp.lfd = temp.rfd = temp.lbd = temp.rbd =
	temp.lfu = temp.rfu = temp.lbu = temp.rbu = posbern3D;
	return(temp);
   }
   if( bb == negbern3D )
   {
	temp.lfd = temp.rfd = temp.lbd = temp.rbd =
	temp.lfu = temp.rfu = temp.lbu = temp.rbu = negbern3D;
	return(temp);
   }

   temp.lfd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.lfu = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.lbd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.lbu = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rfd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rfu = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rbd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rbu = alloc_bern3D(bb->xord,bb->yord,bb->zord);

/*
*/
   ele = bb->array;
   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0; stack<=bb->zord; stack++)
   {
         pyr3Dele(2*row,2*col,2*stack) = 
		*ele;
	ele++;
/*
		*(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack);
*/
   }

#ifdef NOT_DEF
   ele = bb->array;
   pyr = pyr3Dbase;
   for(row=0;row<=bb->xord;row++)
   {
   	for(col=0;col<=bb->yord;col++)
	{
   		for(stack=0; stack<=bb->zord; stack++)
   		{
			*pyr = *(ele++);
/*
			pyr3Dele(row*2,col*2,stack*2) = *(ele++);
*/
			pyr += 2;
		}
		/* now at the 2nd ele of next stack */
		pyr += bb->zord*2; /* Skip a col: zord*2+1 eles */
	}
	/* now at 2nd col of next row */
	pyr += (bb->zord*2+1)*bb->yord*2; /* Skip a col */
   }
#endif

   for(level=1;level<=bb->xord;level++)
      for(row=level;row<= 2*bb->xord -level;row+=2)
/*
	ele = bb->array + (row-1)*(bb->yord+1)*(bb->zord+1);
	ele2 = bb->array + (row+1)*(bb->yord+1)*(bb->zord+1);
*/
      for(col=0;col<=2*bb->yord;col+=2)
      for(stack=0; stack<=2*bb->zord; stack+=2)
            pyr3Dele(row,col,stack) =
		0.5*(pyr3Dele(row-1,col,stack) + pyr3Dele(row+1,col,stack));
/*
		0.5*( *ele + *ele2 );
*/


#ifdef NOT_DEF
   for(level=1;level<=bb->xord;level++)
   {
	pyr = pyr3Dbase + level*(bb->yord*2+1)*(bb->zord*2+1);
	ele = pyr3Dbase + (level-1)*(bb->yord*2+1)*(bb->zord*2+1);
	ele2 = pyr3Dbase + (level+1)*(bb->yord*2+1)*(bb->zord*2+1);
	for(row=level;row<= 2*bb->xord -level;row+=2)
	{
		for(col=0;col<=2*bb->yord;col+=2)
		{
			for(stack=0; stack<=2*bb->zord; stack+=2)
			{
				*(pyr) = 0.5 * ( *ele + *ele2 );
/*
				pyr3Dele(row,col,stack) = 0.5 * (
					pyr3Dele(row-1,col,stack) +
					pyr3Dele(row+1,col,stack) );
*/
				pyr += 2; ele += 2; ele2 += 2;
			}
			pyr += bb->zord*2; /* skip a col */
			ele += bb->zord*2;
			ele2 += bb->zord*2;
		}
		pyr += (bb->zord*2+1)*bb->yord*2; /* skip a row */
		ele += (bb->zord*2+1)*bb->yord*2;
		ele2 += (bb->zord*2+1)*bb->yord*2;
	}
   }
#endif

   for(level=1;level<=bb->yord;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=level;col<=2*bb->yord-level;col+=2)
      for(stack=0; stack<=2*bb->zord; stack+=2)
            pyr3Dele(row,col,stack) =
		0.5*(pyr3Dele(row,col-1,stack) + pyr3Dele(row,col+1,stack));

   for(level=1;level<=bb->zord;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=0;col<=2*bb->yord;++col)
      for(stack=level; stack<=2*bb->zord-level; stack+=2)
            pyr3Dele(row,col,stack) =
		0.5*(pyr3Dele(row,col,stack-1) + pyr3Dele(row,col,stack+1));

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0; stack<=bb->zord; stack++)
   {
	ele3D(temp.lfd,row,col,stack) =
		 pyr3Dele(row,col,stack);
	ele3D(temp.rfd,row,col,stack) =
		 pyr3Dele(row+bb->xord,col,stack);
	ele3D(temp.lbd,row,col,stack) =
		 pyr3Dele(row,col+bb->yord,stack);
	ele3D(temp.rbd,row,col,stack) =
		 pyr3Dele(row+bb->xord,col+bb->yord,stack);
	ele3D(temp.lfu,row,col,stack) =
		 pyr3Dele(row,col,stack+bb->zord);
	ele3D(temp.rfu,row,col,stack) =
		 pyr3Dele(row+bb->xord,col,stack+bb->zord);
	ele3D(temp.lbu,row,col,stack) =
		 pyr3Dele(row,col+bb->yord,stack+bb->zord);
	ele3D(temp.rbu,row,col,stack) =
		 pyr3Dele(row+bb->xord,col+bb->yord,stack+bb->zord);
   }

#ifdef PRINT_PYRIMID
   fprintf(stderr,"pyrimid\n");
   for(stack=0; stack <= bb->zord*2; ++stack )
   {
     for(row=0; row <= bb->xord*2; ++row )
    {
     for(col=0; col <= bb->yord*2; ++col ) 
       fprintf(stderr,"%f\t",pyr3Dele(row,col,stack));
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
   }
#endif
   return(temp);
}
#else
octbern3D reduce3D(bb)
bern3D *bb;
{
   register int row,col,stack,level;
   register double *ele,*ele2,*pyr;
   octbern3D temp;

#define pyr3Dele(R,C,S) *(\
	pyr3Dbase + ( (R) * (2*bb->yord+1) + C)*(2*bb->zord+1) + S)

   if( bb == posbern3D )
   {
	temp.lfd = temp.lfu = temp.lbd = temp.rbd =
	temp.rfd = temp.rfu = temp.rbd = temp.rbd = posbern3D;
	return(temp);
   }
   if( bb == negbern3D )
   {
	temp.lfd = temp.lfu = temp.lbd = temp.rbd =
	temp.rfd = temp.rfu = temp.rbd = temp.rbd = negbern3D;
	return(temp);
   }

   temp.lfd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.lfu = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.lbd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.lbu = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rfd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rfu = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rbd = alloc_bern3D(bb->xord,bb->yord,bb->zord);
   temp.rbu = alloc_bern3D(bb->xord,bb->yord,bb->zord);

   ele = bb->array;
   pyr = pyr3Dbase;
   for(row=0;row<=bb->xord;row++)
   {
   	for(col=0;col<=bb->yord;col++)
	{
   		for(stack=0; stack<=bb->zord; stack++)
   		{
/*
			*pyr = *(ele++);
*/
			pyr3Dele(row*2,col*2,stack*2) = *(ele++);
			pyr += 2;
		}
		/* now at the 2nd ele of next stack */
		pyr += bb->zord*2; /* Skip a col: zord*2+1 eles */
	}
	/* now at 2nd col of next row */
	pyr += (bb->zord*2+1)*bb->yord*2; /* Skip a col */
   }

   for(level=1;level<=bb->xord;level++)
   {
	pyr = pyr3Dbase + level*(bb->yord*2+1)*(bb->zord*2+1);
	ele = pyr3Dbase + (level-1)*(bb->yord*2+1)*(bb->zord*2+1);
	ele2 = pyr3Dbase + (level+1)*(bb->yord*2+1)*(bb->zord*2+1);
	for(row=level;row<= 2*bb->xord -level;row+=2)
	{
		for(col=0;col<=2*bb->yord;col+=2)
		{
			for(stack=0; stack<=2*bb->zord; stack+=2)
			{
/*
				*(pyr) = 0.5 * ( *ele + *ele2 );
*/
				pyr3Dele(row,col,stack) = 0.5 * (
					pyr3Dele(row-1,col,stack) +
					pyr3Dele(row+1,col,stack) );
				pyr += 2; ele += 2; ele2 += 2;
			}
			pyr += bb->zord*2; /* skip a col */
			ele += bb->zord*2;
			ele2 += bb->zord*2;
		}
		pyr += (bb->zord*2+1)*bb->yord*2; /* skip a row */
		ele += (bb->zord*2+1)*bb->yord*2;
		ele2 += (bb->zord*2+1)*bb->yord*2;
	}
   }

   for(level=1;level<=bb->yord;level++)
   {
	pyr = pyr3Dbase + level * (2*bb->zord+1);
	ele = pyr3Dbase + (level-1) * (2*bb->zord+1);
	ele2 = pyr3Dbase + (level+1) * (2*bb->zord+1);

	for(row=0;row<=2*bb->xord;++row)
	{
		for(col=level;col<=2*bb->yord-level;col+=2)
		{
			for(stack=0; stack<=2*bb->zord; stack+=2)
			{
/*
				*(pyr) = 0.5 * ( *ele + *ele2 );
*/
				pyr3Dele(row,col,stack) = 0.5 * (
					pyr3Dele(row,col-1,stack) +
					pyr3Dele(row,col+1,stack) );
				pyr += 2; ele += 2; ele2 += 2;
			}
			pyr += bb->zord*2; /* skip a col */
			ele += bb->zord*2;
			ele2 += bb->zord*2;
		}
		/* Skip level * 2 + 1 cols and 1 row = yord*2 +1 cols */

		pyr += (bb->zord*2+1)*((level+bb->yord)*2+2);
		ele += (bb->zord*2+1)*((level+bb->yord)*2+2);
		ele2 += (bb->zord*2+1)*((level+bb->yord)*2+2);
	}
   }

   for(level=1;level<=bb->zord;level++)
   {
	pyr = pyr3Dbase + level;
	ele = pyr3Dbase + (level-1);
	ele2 = pyr3Dbase + (level+1);

	for(row=0;row<=2*bb->xord;++row)
	{
		for(col=0;col<=2*bb->yord;++col)
		{
			for(stack=level; stack<=2*bb->zord-level; stack+=2)
			{
/*
				*(pyr) = 0.5 * ( *ele + *ele2 );
*/
				pyr3Dele(row,col,stack) = 0.5 * (
					pyr3Dele(row,col,stack-1) +
					pyr3Dele(row,col,stack+1) );
				pyr += 2; ele += 2; ele2 += 2;
			}
			/* skip level*2+1 eles and 1 col */
			pyr += (level+bb->zord)*2+2;
			ele += (level+bb->zord)*2+2;
			ele2 += (level+bb->zord)*2+2;
		}
		/* Skip a row */

		pyr += (bb->zord*2+1)*bb->yord*2; /* skip a row */
		ele += (bb->zord*2+1)*bb->yord*2;
		ele2 += (bb->zord*2+1)*bb->yord*2;
	}
   }

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0; stack<=bb->zord; stack++)
   {
	ele3D(temp.lfd,row,col,stack) =
		pyr3Dele(row,col,stack);
	ele3D(temp.rfd,row,col,stack) =
		pyr3Dele(row+bb->xord,col,stack);
	ele3D(temp.lbd,row,col,stack) =
		pyr3Dele(row,col+bb->yord,stack);
	ele3D(temp.rbd,row,col,stack) =
		pyr3Dele(row+bb->xord,col+bb->yord,stack);
	ele3D(temp.lfu,row,col,stack) =
		pyr3Dele(row,col,stack+bb->zord);
	ele3D(temp.rfu,row,col,stack) =
		pyr3Dele(row+bb->xord,col,stack+bb->zord);
	ele3D(temp.lbu,row,col,stack) =
		pyr3Dele(row,col+bb->yord,stack+bb->zord);
	ele3D(temp.rbu,row,col,stack) =
		pyr3Dele(row+bb->xord,col+bb->yord,stack+bb->zord);
   }

#ifdef PRINT_PYRIMID
   fprintf(stderr,"pyrimid\n");
   for(stack=0; stack <= bb->zord*2; ++stack )
   {
     for(row=0; row <= bb->xord*2; ++row )
    {
     for(col=0; col <= bb->yord*2; ++col ) 
       fprintf(stderr,"%f\t",pyramid[row][col][stack]);
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
   }
#endif
   return(temp);
}
#endif

/*
 * Function:	reduce2D
 * Action:	devides bb into 4 bernsteins for polys.
 * Order:	m^2 n + n^2 m
 */

double pyr2D[MAXORDERT2][MAXORDERT2];	/* Global storage */

quadbern2D reduce2D(bb)
bern2D *bb;
{
   register int row,col,level;
   quadbern2D temp;


   if( bb == posbern2D )
   {
	temp.lb = temp.rb = temp.lt = temp.rt = posbern2D;
	return(temp);
   }
   if( bb == negbern2D )
   {
	temp.lb = temp.rb = temp.lt = temp.rt = negbern2D;
	return(temp);
   }

   temp.lb = alloc_bern2D(bb->xord,bb->yord);
   temp.rb = alloc_bern2D(bb->xord,bb->yord);
   temp.lt = alloc_bern2D(bb->xord,bb->yord);
   temp.rt = alloc_bern2D(bb->xord,bb->yord);

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
         pyr2D[2*row][2*col] = *(bb->array+row*(bb->yord+1)+col);

   for(level=1;level<=bb->xord;level++)
      for(row=level;row<= 2*bb->xord -level;row+=2)
      for(col=0;col<=2*bb->yord;col+=2)
            pyr2D[row][col] =
		0.5*(pyr2D[row-1][col] + pyr2D[row+1][col]);

   for(level=1;level<=bb->yord;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=level;col<=2*bb->yord-level;col+=2)
            pyr2D[row][col] =
		0.5*(pyr2D[row][col-1] + pyr2D[row][col+1]);

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   {
	ele2D(temp.lb,row,col) =
        	pyr2D[row][col];
	ele2D(temp.rb,row,col) =
        	pyr2D[row+bb->xord][col];
	ele2D(temp.lt,row,col) =
        	pyr2D[row][col+bb->yord];
	ele2D(temp.rt,row,col) =
        	pyr2D[row+bb->xord][col+bb->yord];
   }
#ifdef PRINT_PYRIMID
   fprintf(stderr,"pyr2D\n");
     for(row=0; row <= bb->xord*2; ++row )
    {
     for(col=0; col <= bb->yord*2; ++col ) 
       fprintf(stderr,"%f\t",pyr2D[row][col]);
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
   }
#endif
   return(temp);
}

/*
 * Function:	reduce1D
 * Action:	split bb into b1 b2 which are berns for half intervals.
 * Order:	n^2   =  n (n-1)/2 mults
 */

double pyr1D[MAXORDERT2];	/* Global storage */

binbern1D reduce1D(bb)
bern1D *bb;
{
   int row;
   int level;
   binbern1D temp;

   if( bb == posbern1D )
   {
	temp.l = temp.r = posbern1D;
	return(temp);
   }
   if( bb == negbern1D )
   {
	temp.l = temp.r = negbern1D;
	return(temp);
   }

   temp.l = alloc_bern1D(bb->ord);
   temp.r = alloc_bern1D(bb->ord);


   for(row=0;row<=bb->ord;row++)
         pyr1D[2*row] = *(bb->array+row);

   for(level=1;level<=bb->ord;level++)
      for(row=level;row<= 2*bb->ord -level;row+=2)
            pyr1D[row] = 0.5*(pyr1D[row-1] + pyr1D[row+1]);

#ifdef PRINT_PYRIMID
   fprintf(stderr,"pyrimid\n");
   for(row=0; row <= ord*2; ++row )
       fprintf(stderr,"%f\t",pyr1D[row]);
   fprintf(stderr,"\n");
#endif

   for(row=0;row<=bb->ord;row++)
   {
	ele1D(temp.l,row) = pyr1D[row];
        ele1D(temp.r,row) = pyr1D[row+bb->ord];
   }
   return(temp);
}

/************************************************************************/
/*									*/
/*	caluclates the value of a 1 dimensional berstein polynomial	*/
/*	bb of oder 'order' at 'root' in [0.0,1.0].			*/
/*	Order n^2  n(n-1) mults						*/
/*									*/
/************************************************************************/

double evalbern1D(bb,root)
bern1D *bb;
double root;
{
   double work[MAXORDERT2];
   double oneminusroot=1.0-root;
   register int element,level;


   if(bb == posbern1D )
   {
	fprintf(stderr,"evalbern1D: tryied to evaluate posbern1D\n");
	return(1.0);
   }
   if(bb == negbern1D )
   {
	fprintf(stderr,"evalbern1D: tryied to evaluate negbern1D\n");
	return(-1.0);
   }
   for(element=0;element<=bb->ord;element++)
      work[2*element] = *(bb->array+element);

   for(level=1;level<=bb->ord;level++)
      for(element=level;element<=2*bb->ord-level;element+=2)
         work[element] = oneminusroot * work[element-1] +
             root * work[element+1];

   return(work[bb->ord]);
}

double evalbern2D(bb,vec)
bern2D *bb;
double vec[2];
{
	int i,j;
	bern1D *bernA,*bernB;
	double res;

   if(bb == posbern2D )
   {
	fprintf(stderr,"evalbern2D: tryied to evaluate posbern2D\n");
	return(1.0);
   }
   if(bb == negbern2D )
   {
	fprintf(stderr,"evalbern2D: tryied to evaluate negbern2D\n");
	return(-1.0);
   }
	bernA = alloc_bern1D(bb->xord);
	bernB = alloc_bern1D(bb->yord);

	for(i=0;i<=bb->xord;++i)
	{
		for(j=0;j<=bb->yord;++j)
			*(bernB->array+j) = *(bb->array+i*(bb->yord+1)+j);

		*(bernA->array+i) = evalbern1D(bernB,vec[1]);
	}
	res = evalbern1D(bernA,vec[0]);
	free_bern1D(bernA);
	free_bern1D(bernB);
	return(res);
}

double evalbern3D(bb,vec)
bern3D *bb;
double vec[3];
{
	register int element,level;
	double workA[MAXORDERT2],workB[MAXORDERT2],workC[MAXORDERT2];
	double oneminusroot,root;
	int i,j;

   if(bb == posbern3D )
   {
	fprintf(stderr,"evalbern3D: tryied to evaluate posbern3D\n");
	return(1.0);
   }
   if(bb == negbern3D )
   {
	fprintf(stderr,"evalbern3D: tryied to evaluate negbern3D\n");
	return(-1.0);
   }
	for(i=0;i<=bb->xord;++i)
	{
		root = vec[2]; oneminusroot = 1.0 - root;

		for(j=0;j<=bb->yord;++j)
		{
			for(element=0;element<=bb->zord;element++)
			   workC[2*element] =
				*(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+element);

			for(level=1;level<=bb->zord;level++)
			   for(element=level;element<=2*bb->zord-level;
					element+=2)
			      workC[element] = oneminusroot * workC[element-1] +
			          root * workC[element+1];

			workB[j*2] = workC[bb->zord];
		}

		root = vec[1]; oneminusroot = 1.0 - root;

		for(level=1;level<=bb->yord;level++)
		    for(element=level;element<=2*bb->yord-level;element+=2)
		       workB[element] = oneminusroot * workB[element-1] +
		           root * workB[element+1];

		workA[i*2] = workB[bb->yord];
	}
	root = vec[0]; oneminusroot = 1.0 - root;

	for(level=1;level<=bb->xord;level++)
	     for(element=level;element<=2*bb->xord-level;element+=2)
	        workA[element] = oneminusroot * workA[element-1] +
	            root * workA[element+1];

	return(workA[bb->xord]);
}

/*
 * Function:	make_bern2D_of_box(bb,type,aa)
 * action:	create a 2D bernstein poly for face type of 3D bern-poly bb
 */

bern2D *make_bern2D_of_box(bb,type)
bern3D *bb;
soltype type;
{
	register int i,j;
	bern2D *aa;

	if( bb == posbern3D ) return(posbern2D);
	if( bb == negbern3D ) return(negbern2D);

	switch(type)
	{
	case FACE_LL:
		aa = alloc_bern2D(bb->yord,bb->zord);
		for(i=0;i<=bb->yord;++i)
		    for(j=0;j<=bb->zord;++j)
			ele2D(aa,i,j) = ele3D(bb,0,i,j);
	break;
	case FACE_RR:
		aa = alloc_bern2D(bb->yord,bb->zord);
		for(i=0;i<=bb->yord;++i)
		    for(j=0;j<=bb->zord;++j)
			ele2D(aa,i,j) = ele3D(bb,bb->xord,i,j);
	break;
	case FACE_FF:
		aa = alloc_bern2D(bb->xord,bb->zord);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->zord;++j)
			ele2D(aa,i,j) = ele3D(bb,i,0,j);
	break;
	case FACE_BB:
		aa = alloc_bern2D(bb->xord,bb->zord);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->zord;++j)
			ele2D(aa,i,j) = ele3D(bb,i,bb->yord,j);
	break;
	case FACE_DD:
		aa = alloc_bern2D(bb->xord,bb->yord);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->yord;++j)
			ele2D(aa,i,j) = ele3D(bb,i,j,0);
	break;
	case FACE_UU:
		aa = alloc_bern2D(bb->xord,bb->yord);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->yord;++j)
			ele2D(aa,i,j) = ele3D(bb,i,j,bb->zord);
	break;
	}
	return(aa);
}

/*
 * Function:	make_bern1D_of_face(bb,type,aa)
 * action:	create a 2D bernstein poly for face type of 2D bern-poly bb
 */

bern1D *make_bern1D_of_face(bb,type)
bern2D *bb;
int type;
{
	int i;
	bern1D *aa;

	if( bb == posbern2D ) return(posbern1D);
	if( bb == negbern2D ) return(negbern1D);

	switch(type)
	{
	case X_LOW:
		aa = alloc_bern1D(bb->yord);
		for(i=0;i<=aa->ord;++i)
			ele1D(aa,i) = ele2D(bb,0,i);
		break;
	case X_HIGH:
		aa = alloc_bern1D(bb->yord);
		for(i=0;i<=aa->ord;++i)
			ele1D(aa,i) = ele2D(bb,bb->xord,i);
		break;
	case Y_LOW:
		aa = alloc_bern1D(bb->xord);
		for(i=0;i<=aa->ord;++i)
			ele1D(aa,i) = ele2D(bb,i,0);
		break;
	case Y_HIGH:
		aa = alloc_bern1D(bb->xord);
		for(i=0;i<=aa->ord;++i)
			ele1D(aa,i) = ele2D(bb,i,bb->yord);
		break;
	}
	return(aa);
}

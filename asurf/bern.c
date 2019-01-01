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
 * This software is copyrighted as noted above.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is
 * preserved on all copies.
 *
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the authors, who may or may not act on them as they desire.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 *
 * All this software is public domain, as long as it is not used by any military
 * establishment. Please note if you are a military establishment then a mutating
 * virus has now escaped into you computer and is presently turning all your
 * programs into socially useful, peaceful ones.
 * 
 */
/************************************************************************/
/*									*/
/*	Some functions for using bernstein polynomials.			*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define I_AM_BERN
#include "bern.h"
#include "cells.h"

/*
#define TEST_ALLOC
#define DEBUG_ALLOC
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

int	bern3Dcount,bern3Dnew, bern2Dcount,bern2Dnew, bern1Dcount,bern1Dnew;
int	bern3Dmax,bern2Dmax,bern1Dmax;
int	bern3Dsize,bern2Dsize,bern1Dsize;

/*** Macros for getting elements from arrays ***/

#define ele3D(bb,i,j,k) (*(bb->array+((i)*(bb->yord+1)+j)*(bb->zord+1)+k))
#define ele2D(bb,i,j)	(*(bb->array+((i)*(bb->yord+1)+j)))
#define ele1D(bb,i)	(*(bb->array+i))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define BERN_NO_SIGN -2

/*** calculate nCm ***/

int combMaxSize=-1;

int *combVals;

int comb(n,m)   /* rewritten in double so can get up to 13! */
int n,m;
{
#ifdef NOT_DEF
   register int i=1,j;

   for(j=m+1;j<=n;j++) i *= j;
   for(j=1;j<=n-m;j++) i /= j;
   return(i);
#endif
   register int j;
   register double a;

   if(n<=combMaxSize) 
	return *(combVals + n + (combMaxSize+1) * m);

   a = 1.0;
   for(j=m+1;j<=n;j++) a *= (double) j;
   for(j=1;j<=n-m;j++) a /= (double) j;
   j = (int) (a+0.001); 
   if( fabs( a - j ) > 1.0e-3 )
	fprintf(stderr,"comb: error n %d m %d %.12f %d\n",
		n,m,a,j);
   return(j);
}

void build_combVals(int ord)
{
	int i,j;

	combMaxSize = -1;
	combVals = (int *) malloc(sizeof(int)*(ord+1)*(ord+1));
	for(i=0;i<=ord;++i)
	    for(j=0;j<=i;++j)
		*(combVals+i+j*(ord+1)) = comb(i,j);
	combMaxSize = ord;
}
		

/*** print x as either -0,0,+0 or x ***/

void printzero(x)
double x;
{
  fprintf(stderr,"%9.6g ",x);
  return;
  if( x >= 0.0000005 || x <= -0.0000005 )  fprintf(stderr,"%9.6f ",x);
  else if( x == 0.0 )                      fprintf(stderr," 0        ");
  else if( x  > 0.0 )                      fprintf(stderr,"+0        ");
  else if( x  < 0.0 )                      fprintf(stderr,"-0        ");
  else					   fprintf(stderr,"! %f ",x);
}

/*** print out the bernstein polynomials ***/

void printbern1D(bb)
bern1D *bb;
{
	int i;

	if(bb == posbern1D ) fprintf(stderr,"posative 1D bern\n");
	else if( bb == negbern1D )  fprintf(stderr,"negative 1D bern\n");
	else if( bb == NULL )  fprintf(stderr,"null 1D bern\n");
	else for(i=0;i<=bb->ord;++i) printzero( *(bb->array+i) );
}

void printbern1D_normal(bern1D *bb)
{
	int i,j,n;
	double coef[MAXORDER];

	if(bb == posbern1D ) fprintf(stderr,"posative 1D bern\n");
	else if( bb == negbern1D )  fprintf(stderr,"negative 1D bern\n");
	else if( bb == NULL )  fprintf(stderr,"null 1D bern\n");
	else
	{
		n = bb->ord;
		for(i=0;i<=bb->ord;++i)
			coef[i]=0.0;
	
		for(i=0;i<=n;++i)
		{
			for(j=0;j<=i;++j)
			{
			    if(j%2==1)
				coef[n+j-i] -= comb(n,i) * comb(i,j) * *(bb->array+i);
			    else
				coef[n+j-i] += comb(n,i) * comb(i,j) * *(bb->array+i);
			}
		}
		fprintf(stderr,"%d ",n);
		for(i=bb->ord;i>=0;--i)
			printzero(coef[i]);
	}
}

void printbern2D(bb)
bern2D *bb;
{
	int i,j;

	if(bb == posbern2D ) fprintf(stderr,"posative 2D bern\n");
	else if( bb == negbern2D )  fprintf(stderr,"negative 2D bern\n");
	else if( bb == NULL )  fprintf(stderr,"null 2D bern\n");
	else for(j=bb->yord;j>=0;--j)
	{
		for(i=0;i<=bb->xord;++i) printzero( *(bb->array+i*(bb->yord+1)+j) );
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"\n");
}

void printbern3D(bb)
bern3D *bb;
{
	int i,j,k;

	if(bb == posbern3D ) fprintf(stderr,"posative 3D bern\n");
	else if( bb == negbern3D )  fprintf(stderr,"negative 3D bern\n");
	else if( bb == NULL )  fprintf(stderr,"null 3D bern\n");
	for(k=bb->zord;k>=0;--k)
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

struct mat1Dlist { struct mat1Dlist *next; };
int	mat1Dords[15] = {-1,-1,-1,-1,-1, -1,-1,-1,-1,-1, -1,-1,-1,-1,-1};
struct mat1Dlist *list1Dmats[15] = {NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

/*
 * Function:	init_bern
 * Action:	create the lists of free bernsteins.
 */

void init_berns(bern3D *a,bern3D *b,bern3D *c,bern3D *d)
{
	int i;
	struct mat3Dlist *temp,*temp2;
	struct mat2Dlist *temp3,*temp4;
	struct mat1Dlist *temp5,*temp6;

	build_combVals(MAX(a->xord,MAX(a->yord,a->zord)));

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
		    || mat3Dords[i][2] != d->zord) )
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
		 && ( mat2Dords[i][0] != d->yord||mat2Dords[i][1] != d->zord ))
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
		 && mat1Dords[i] != d->zord )
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
#ifdef TEST_ALLOC
	bern3Dcount = bern3Dnew = bern3Dmax = bern3Dsize =
	bern2Dcount = bern2Dnew = bern2Dmax = bern2Dsize =
	bern1Dcount = bern1Dnew = bern1Dmax = bern1Dsize = 0;
#endif
}

void fini_berns()
{
#ifdef TEST_ALLOC
	fprintf(stderr,"3D\t%d\t%d\t%d\t%d\n2D\t%d\t%d\t%d\t%d\n1D\t%d\t%d\t%d\t%d\n",
		bern3Dcount,bern3Dnew,bern3Dmax,bern3Dsize,
		bern2Dcount,bern2Dnew,bern2Dmax,bern2Dsize,
		bern1Dcount,bern1Dnew,bern1Dmax,bern1Dsize);
#endif
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

#ifdef TEST_ALLOC
	++bern3Dcount;
	++bern3Dmax;
#endif
	temp = (bern3D *) malloc(sizeof(bern3D));
	temp->xord = xord;
	temp->yord = yord;
	temp->zord = zord;
	temp->sign = BERN_NO_SIGN;

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
#ifdef TEST_ALLOC
			++bern3Dnew;
			bern3Dsize += (xord+1)*(yord+1)*(zord+1)*sizeof(double);
#endif
			temp->xord = xord;
			temp->yord = yord;
			temp->zord = zord;
			temp->array = (double *)
			    calloc((size_t) (xord+1)*(yord+1)*(zord+1),sizeof(double));
			return(temp);
		    }
		}
	}
#ifdef TEST_ALLOC
	++bern3Dnew;
#endif
	temp->array = (double *)
		calloc((size_t) (xord+1)*(yord+1)*(zord+1),sizeof(double));
	if(mat3Dords[4][0]== -1 && mat3Dords[4][1] == -1 && mat3Dords[4][2] == -1 )
		return(temp);

#ifdef DEBUG_ALLOC
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

void free_bern3D(bb)
bern3D *bb;
{
	struct mat3Dlist *list;
	int i;

	if(bb == posbern3D || bb == negbern3D ) return;
#ifdef TEST_ALLOC
	--bern3Dcount;
#endif
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
#ifdef DEBUG_ALLOC
	fprintf(stderr,"free_bern3D_mat: error couldn't find match\n");
#endif
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

#ifdef TEST_ALLOC
	++bern2Dcount;
	++bern2Dmax;
#endif
	temp = (bern2D *) malloc(sizeof(bern2D));
	temp->xord = xord;
	temp->yord = yord;
	temp->sign = BERN_NO_SIGN;

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
#ifdef TEST_ALLOC
			++bern2Dnew;
			bern2Dsize += (xord+1)*(yord+1)*sizeof(double);
#endif
			temp->xord = xord;
			temp->yord = yord;
			temp->array = (double *)
			    calloc((size_t) (xord+1)*(yord+1),sizeof(double));
			return(temp);
		    }
		}
	}
#ifdef TEST_ALLOC
	++bern2Dnew;
#endif
	temp->xord = xord;
	temp->yord = yord;
	temp->array = (double *)
		calloc((size_t) (xord+1)*(yord+1),sizeof(double));
	if(mat2Dords[14][0]== -1 && mat2Dords[14][1] == -1)
		return(temp);
#ifdef DEBUG_ALLOC
	fprintf(stderr,"alloc_bern2D_mat: error couldn't find match %d %d\n",
		xord,yord);
	for(i=0;i<15;++i)
		fprintf(stderr,"ords[%d] %d %d\n",i,
			mat2Dords[i][0],mat2Dords[i][1]);
#endif
	return(temp);
}

void free_bern2D(bb)
bern2D *bb;
{
	struct mat2Dlist *list;
	int i;

	if(bb == NULL || bb == posbern2D || bb == negbern2D ) return;
#ifdef TEST_ALLOC
	--bern2Dcount;
#endif
	for(i=0;i<15;++i)
	{
		if( mat2Dords[i][0] == bb->xord
		 && mat2Dords[i][1] == bb->yord )
		{
			list = (struct mat2Dlist *) bb->array;
			list->next = list2Dmats[i];
			list2Dmats[i] = list;
			free(bb);
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
			free(bb);
			return;
		}
	}
#ifdef DEBUG_ALLOC
	fprintf(stderr,"free_bern2D_mat: error couldn't find match %d %d\n",
		bb->xord,bb->yord);
	for(i=0;i<15;++i)
		fprintf(stderr,"ords[%d] %d %d\n",i,
			mat2Dords[i][0],mat2Dords[i][1]);
#endif
/*
	fprintf(stderr,"free_bern2D_mat: count %d\n",--matcount);
*/
	free(bb->array);
	free(bb);
}

bern1D *alloc_bern1D(xord)
int xord;
{
	bern1D *temp;
	int	i;

#ifdef TEST_ALLOC
	++bern1Dcount;
	++bern1Dmax;
#endif
	temp = (bern1D *) malloc(sizeof(bern1D));
	temp->ord = xord;
	temp->sign = BERN_NO_SIGN;

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
#ifdef TEST_ALLOC
			++bern1Dnew;
			bern1Dsize += (xord+1)*sizeof(double);
#endif
			temp->array = (double *)
			    calloc((size_t) (xord+1),sizeof(double));
			return(temp);
		    }
		}
	}
#ifdef TEST_ALLOC
	++bern1Dnew;
#endif
	temp->array = (double *)
		calloc((size_t) (xord+1),sizeof(double));
	if(mat1Dords[14]== -1 ) return(temp);
#ifdef DEBUG_ALLOC
	fprintf(stderr,"alloc_bern1D: error couldn't find match %d\n",
		xord);
	for(i=0;i<15;++i)
		fprintf(stderr,"ords[%d] %d\n",i,mat1Dords[i]);
#endif
	return(temp);
}


/*
 * Function:	free_bern1D
 * Action:	frees the space used by bern poly
 */

void free_bern1D(bb)
bern1D *bb;
{
	struct mat1Dlist *list;
	int i;

	if(bb == posbern1D || bb == negbern1D ) return;
#ifdef TEST_ALLOC
	--bern1Dcount;
#endif
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
#ifdef DEBUG_ALLOC
	fprintf(stderr,"free_bern1D_mat: error couldn't find match\n");
#endif
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

void free_octbern3D(bb)
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

void free_quadbern2D(bb)
quadbern2D bb;
{
	if(bb.lt == posbern2D || bb.lt == negbern2D ) return;
	free_bern2D(bb.lt);
	free_bern2D(bb.rt);
	free_bern2D(bb.lb);
	free_bern2D(bb.rb);
}

void free_binbern1D(bb)
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
   int i,j,k;

   if( bb == NULL ) return(0);
   if( bb == posbern3D ) return(1);
   if( bb == negbern3D ) return(-1);
   if(bb->sign != BERN_NO_SIGN ) return bb->sign;
   if( *bb->array < 0)
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
            if( *(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k) >= 0.0)
		{ bb->sign = 0; return(0); }
   bb->sign = -1;
   return(-1);
   }
   else
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
            if( *(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k) <= 0.0)
		{ bb->sign = 0; return(0); }
   bb->sign = 1;
   return(1);
   }
}

/* checks if every element of a 2d array has strictly the same sign */

int allonesign2D(bb)
bern2D *bb;
{
   int i,j;

   if( bb == NULL ) return(0);
   if( bb == posbern2D ) return(1);
   if( bb == negbern2D ) return(-1);
   if(bb->sign != BERN_NO_SIGN ) return bb->sign;
   if( *bb->array < 0)
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
            if( *(bb->array+i*(bb->yord+1)+j) >= 0.0)
		{ bb->sign = 0; return(0); }
   bb->sign = -1;
   return(-1);
   }
   else
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
            if( *(bb->array+i*(bb->yord+1)+j) <= 0.0)
		{ bb->sign = 0; return(0); }
   bb->sign = 1;
   return(1);
   }
}

/* checks if every element of a 1d array has strictly the same sign */

int allonesign1D(bb)
bern1D *bb;
{
	int i;

   if( bb == NULL ) return(0);
   if( bb == posbern1D ) return(1);
   if( bb == negbern1D ) return(-1);
   if(bb->sign != BERN_NO_SIGN ) return bb->sign;
	if( *(bb->array) < 0)
	{
	for( i=1; i<=bb->ord; ++i)
		if( *(bb->array+i) >= 0.0 )
		{ bb->sign = 0; return(0); }
	bb->sign = -1;
	return(-1);
	}
	else
	{
	for( i=0; i<=bb->ord; ++i)
		if( *(bb->array+i) <= 0.0 )
		{ bb->sign = 0; return(0); }
	bb->sign = 1;
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

void formbernstein1D(aa,bb,ord,min,max)
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

   for(row=0; row<= bb->xord; row++)
   for(col=0;col<=bb->yord-1;col++)
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

double pyramid[MAXORDERT2][MAXORDERT2][MAXORDERT2]; /* Global def saves space */

octbern3D reduce3D(bb)
bern3D *bb;
{
   int row,col,stack;
   int level;
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

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0; stack<=bb->zord; stack++)
         pyramid[2*row][2*col][2*stack] = 
		*(bb->array+(row*(bb->yord+1)+col)*(bb->zord+1)+stack);

   for(level=1;level<=bb->xord;level++)
      for(row=level;row<= 2*bb->xord -level;row+=2)
      for(col=0;col<=2*bb->yord;col+=2)
      for(stack=0; stack<=2*bb->zord; stack+=2)
            pyramid[row][col][stack] =
		0.5*(pyramid[row-1][col][stack] + pyramid[row+1][col][stack]);

   for(level=1;level<=bb->yord;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=level;col<=2*bb->yord-level;col+=2)
      for(stack=0; stack<=2*bb->zord; stack+=2)
            pyramid[row][col][stack] =
		0.5*(pyramid[row][col-1][stack] + pyramid[row][col+1][stack]);

   for(level=1;level<=bb->zord;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=0;col<=2*bb->yord;++col)
      for(stack=level; stack<=2*bb->zord-level; stack+=2)
            pyramid[row][col][stack] =
		0.5*(pyramid[row][col][stack-1] + pyramid[row][col][stack+1]);

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0; stack<=bb->zord; stack++)
   {
	ele3D(temp.lfd,row,col,stack) =
		 pyramid[row][col][stack];
	ele3D(temp.rfd,row,col,stack) =
		 pyramid[row+bb->xord][col][stack];
	ele3D(temp.lbd,row,col,stack) =
		 pyramid[row][col+bb->yord][stack];
	ele3D(temp.rbd,row,col,stack) =
		 pyramid[row+bb->xord][col+bb->yord][stack];
	ele3D(temp.lfu,row,col,stack) =
		 pyramid[row][col][stack+bb->zord];
	ele3D(temp.rfu,row,col,stack) =
		 pyramid[row+bb->xord][col][stack+bb->zord];
	ele3D(temp.lbu,row,col,stack) =
		 pyramid[row][col+bb->yord][stack+bb->zord];
	ele3D(temp.rbu,row,col,stack) =
		 pyramid[row+bb->xord][col+bb->yord][stack+bb->zord];
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

   if( bb == NULL)
   {
	temp.lb = temp.rb = temp.lt = temp.rt = NULL;
	return(temp);
   }
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

quadbern2D quadDiff2Dx(quadbern2D bb)
{
   quadbern2D temp;

   temp.lb = diffx2D(bb.lb);
   temp.rb = diffx2D(bb.rb);
   temp.lt = diffx2D(bb.lt);
   temp.rt = diffx2D(bb.rt);
   return temp;
}

quadbern2D quadDiff2Dy(quadbern2D bb)
{
   quadbern2D temp;

   temp.lb = diffy2D(bb.lb);
   temp.rb = diffy2D(bb.rb);
   temp.lt = diffy2D(bb.lt);
   temp.rt = diffy2D(bb.rt);
   return temp;
}

binbern1D binDiff1D(binbern1D bb)
{
	binbern1D temp;
	temp.l = diff1D(bb.l);
	temp.r = diff1D(bb.r);
	return temp;
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
	register int element,level;
	int i,j;
	double workA[MAXORDERT2],workB[MAXORDERT2];
	double oneminusroot,root;

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

	for(i=0;i<=bb->xord;++i)
	{
		for(j=0;j<=bb->yord;++j)
		      workB[2*j] = *(bb->array+i*(bb->yord+1)+j);

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
			   for(element=level;element<=2*bb->zord-level;element+=2)
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

bern2D *make_bern2D_of_box(bern3D *bb,soltype type)
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
	default:
		fprintf(stderr,"bad type %d in make_bern2d_of_box\n",type);
		exit(1);
	}
	return(aa);
}

/*
 * Function:	make_bern1D_of_face(bb,type,aa)
 * action:	create a 2D bernstein poly for face type of 2D bern-poly bb
 */

bern1D *make_bern1D_of_face(bern2D *bb,int type)
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
	default:
		aa = NULL;
		fprintf(stderr,"make_bern1D_of_face: Whoopse bad type %d\n",type);
	}
	return(aa);
}

bern2D *multiplyBern2D(bern2D *M,bern2D *N)
{
	int i,j,k,l;
	bern2D *aa;
	double val;

	if( M== NULL || M == posbern2D || M == negbern2D ) return(NULL);
	if( N== NULL || N == posbern2D || N == negbern2D ) return(NULL);

	aa = alloc_bern2D(M->xord+N->xord,M->yord+N->yord);

	for(i=0;i<=M->xord+N->xord;++i)
		for(j=0;j<=M->yord+N->yord;++j)
				ele2D(aa,i,j) = 0.0;
	for(i=0;i<=M->xord;++i)
	    for(j=0;j<=N->xord;++j)
		for(k=0;k<=M->yord;++k)
		    for(l=0;l<=N->yord;++l)
			{
				val = 	comb(M->xord,i) * comb(N->yord,j) * 
					comb(M->yord,k) * comb(N->yord,l) *
				ele2D(M,i,k) * ele2D(N,j,l);
				ele2D(aa,(i+j),(k+l)) += val;
/*
				fprintf(stderr,"(%d+%d,%d+%d) += %f =%f %f\n",i,j,k,l,
					val,ele2D(M,i,k),ele2D(N,j,l));
*/
			}
	for(i=0;i<=M->xord+N->xord;++i)
		for(j=0;j<=M->yord+N->yord;++j)
				ele2D(aa,i,j) /= comb(M->xord+N->xord,i)
						* comb(M->yord+N->yord,j);
	return(aa);
}

bern2D *identityBern2D(int m,int n)
{
	bern2D *aa;
	int i,j;

	aa = alloc_bern2D(m,n);

	for(i=0;i<=m;++i)
	    for(j=0;j<=n;++j)
		ele2D(aa,i,j) = 1.0;
	return(aa);
}

bern2D *addBern2D(bern2D *M,bern2D *N)
{
	int i,j,xord,yord;
	bern2D *aa,*bb,*cc,*ll,*rr;
	
	if( M== NULL || M == posbern2D || M == negbern2D ) return(NULL);
	if( N== NULL || N == posbern2D || N == negbern2D ) return(NULL);

	xord = MAX(M->xord,N->xord);
	yord = MAX(M->yord,N->yord);

	aa = alloc_bern2D(xord,yord);

	if(M->xord < N->xord)
	{
		if(M->yord < N->yord)
		{
			bb = identityBern2D(N->xord-M->xord,N->yord-M->yord);
			ll = multiplyBern2D(M,bb);
			rr = N;
		}
		else if(M->yord > N->yord)
		{
			bb = identityBern2D(N->xord-M->xord,0);
			cc = identityBern2D(0,M->yord-N->yord);
			ll = multiplyBern2D(M,bb);
			rr = multiplyBern2D(N,cc);
		}
		else
		{
			bb = identityBern2D(N->xord-M->xord,0);
			ll = multiplyBern2D(M,bb);
			rr = N;
		}
	}
	else if(M->xord > N->xord)
	{
		if(M->yord < N->yord)
		{
			bb = identityBern2D(0,N->yord-M->yord);
			cc = identityBern2D(M->xord-N->xord,0);
			ll = multiplyBern2D(M,bb);
			rr = multiplyBern2D(M,bb);
		}
		else if(M->yord > N->yord)
		{
			cc = identityBern2D(M->xord-N->xord,M->yord-N->yord);
			ll = M;
			rr = multiplyBern2D(N,cc);
		}
		else
		{
			cc = identityBern2D(M->xord-N->xord,0);
			ll = M;
			rr = multiplyBern2D(N,cc);
		}
	}
	else
	{
		if(M->yord < N->yord)
		{
			bb = identityBern2D(0,N->yord-M->yord);
			ll = multiplyBern2D(M,bb);
			rr = N;
		}
		else if(M->yord > N->yord)
		{
			cc = identityBern2D(0,M->yord-N->yord);
			ll = M;
			rr = multiplyBern2D(N,cc);
		}
		else
		{
			ll = M;
			rr = N;
		}
	}

	for(i=0;i<=xord;++i)
	    for(j=0;j<=yord;++j)
		ele2D(aa,i,j) = ele2D(ll,i,j) + ele2D(rr,i,j);
	return(aa);
}

bern2D *subtractBern2D(bern2D *M,bern2D *N)
{
	int i,j,xord,yord;
	bern2D *aa,*bb,*cc,*ll,*rr;
	
	if( M== NULL || M == posbern2D || M == negbern2D ) return(NULL);
	if( N== NULL || N == posbern2D || N == negbern2D ) return(NULL);

	xord = MAX(M->xord,N->xord);
	yord = MAX(M->yord,N->yord);

	aa = alloc_bern2D(xord,yord);
	aa->sign = BERN_NO_SIGN;
	if(M->xord < N->xord)
	{
		if(M->yord < N->yord)
		{
			bb = identityBern2D(N->xord-M->xord,N->yord-M->yord);
			ll = multiplyBern2D(M,bb);
			rr = N;
		}
		else if(M->yord > N->yord)
		{
			bb = identityBern2D(N->xord-M->xord,0);
			cc = identityBern2D(0,M->yord-N->yord);
			ll = multiplyBern2D(M,bb);
			rr = multiplyBern2D(N,cc);
		}
		else
		{
			bb = identityBern2D(N->xord-M->xord,0);
			ll = multiplyBern2D(M,bb);
			rr = N;
		}
	}
	else if(M->xord > N->xord)
	{
		if(M->yord < N->yord)
		{
			bb = identityBern2D(0,N->yord-M->yord);
			cc = identityBern2D(M->xord-N->xord,0);
			ll = multiplyBern2D(M,bb);
			rr = multiplyBern2D(M,bb);
		}
		else if(M->yord > N->yord)
		{
			cc = identityBern2D(M->xord-N->xord,M->yord-N->yord);
			ll = M;
			rr = multiplyBern2D(N,cc);
		}
		else
		{
			cc = identityBern2D(M->xord-N->xord,0);
			ll = M;
			rr = multiplyBern2D(N,cc);
		}
	}
	else
	{
		if(M->yord < N->yord)
		{
			bb = identityBern2D(0,N->yord-M->yord);
			ll = multiplyBern2D(M,bb);
			rr = N;
		}
		else if(M->yord > N->yord)
		{
			cc = identityBern2D(0,M->yord-N->yord);
			ll = M;
			rr = multiplyBern2D(N,cc);
		}
		else
		{
			ll = M;
			rr = N;
		}
	}

	for(i=0;i<=xord;++i)
	    for(j=0;j<=yord;++j)
		ele2D(aa,i,j) = ele2D(ll,i,j) - ele2D(rr,i,j);
	return(aa);
}

bern2D *symetricDet2D(bern2D *a,bern2D *b,bern2D *c)
{
#ifdef NOT_DEF
	bern2D *ac,*bb,*sub;
	
	ac = multiplyBern2D(a,c);
	bb = multiplyBern2D(b,b);
	sub = 	subtractBern2D(ac,bb);
fprintf(stderr,"ac\n");
	printbern2D(ac);
	printbern2D(bb);
	printbern2D(sub);
#endif	
	return
		subtractBern2D(
			multiplyBern2D(a,c),
			multiplyBern2D(b,b));
}
	

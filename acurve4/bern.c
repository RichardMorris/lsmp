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
#define I_AM_BERN
#include "bern.h"
#include "cells.h"

/*** All positive and all negative bernstein's ***/

bern4D pb4, nb4;
bern4D *posbern4D = &pb4;
bern4D *negbern4D = &nb4;
bern3D pb3, nb3;
bern3D *posbern3D = &pb3;
bern3D *negbern3D = &nb3;
bern2D pb2, nb2;
bern2D *posbern2D = &pb2;
bern2D *negbern2D = &nb2;
bern1D pb1, nb1;
bern1D *posbern1D = &pb1;
bern1D *negbern1D = &nb1;

/*** Macros for getting elements from arrays ***/

#define ele4D(bb,i,j,k,l) (\
 *(bb->array+(((i)*(bb->yord+1)+(j))*(bb->zord+1)+(k))*(bb->word+1)+(l)))
#define ele3D(bb,i,j,k) (*(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k))
#define ele2D(bb,i,j)	(*(bb->array+(i*(bb->yord+1)+j)))
#define ele1D(bb,i)	(*(bb->array+i))

/*** calculate nCm ***/

comb(n,m)
int n,m;
{
   register int i=1,j;

   for(j=m+1;j<=n;j++) i *= j;
   for(j=1;j<=n-m;j++) i /= j;
   return(i);
}

/*** print x as either -0,0,+0 or x ***/

printzero(x)
double x;
{
  if( x >= 0.0000005 || x <= -0.0000005 )  fprintf(stderr,"%9.6f ",x);
  else if( x == 0.0 )                      fprintf(stderr," 0        ");
  else if( x  > 0.0 )                      fprintf(stderr,"+0        ");
  else if( x  < 0.0 )                      fprintf(stderr,"-0        ");
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

printbern4D(bb)
bern4D *bb;
{
	int i,j,k,l;

	if(bb == posbern4D ) fprintf(stderr,"posative 4D bern\n");
	else if( bb == negbern4D )  fprintf(stderr,"negative 4D bern\n");
	for(l=bb->word;l>=0;--l)
	for(k=bb->zord;k>=0;--k)
	{
	   fprintf(stderr,"z^%d w^%d\n",k,l);
	   for(j=bb->yord;j>=0;--j)
	   {
		for(i=0;i<=bb->xord;++i)
			printzero( ele4D(bb,i,j,k,l) );
		fprintf(stderr,"\t\ty^%d\n",j);
	   }
	}
}

/****************** Memory allocation **************************************/

typedef struct mat4Dlist { struct mat4Dlist *next; };
int	mat4Dords[7][4] = {{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},
		{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}};
struct mat4Dlist *list4Dmats[7] = {NULL,NULL,NULL,NULL,NULL};

#define MAX_NUM_3D_MATS 20
typedef struct mat3Dlist { struct mat3Dlist *next; };
int	mat3Dords[MAX_NUM_3D_MATS][3] = {
	{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},
	{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},
	{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},
	{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1},{-1,-1,-1}};
struct mat3Dlist *list3Dmats[MAX_NUM_3D_MATS] = 
	{NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL,
	 NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL};

typedef struct mat2Dlist { struct mat2Dlist *next; };
int	mat2Dords[15][2] = {{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
	{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
struct mat2Dlist *list2Dmats[15] = {NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

typedef struct mat1Dlist { struct mat1Dlist *next; };
int	mat1Dords[15] = {-1,-1,-1,-1,-1, -1,-1,-1,-1,-1, -1,-1,-1,-1,-1};
struct mat1Dlist *list1Dmats[15] = {NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

/*
 * Function:	init_bern
 * Action:	create the lists of free bernsteins.
 */

init_berns(a,b,c,d,e,f,g)
bern4D *a,*b,*c,*d,*e,*f,*g;
{
	int i,res;
	struct mat4Dlist *temp7,*temp8;
	struct mat3Dlist *temp,*temp2;
	struct mat2Dlist *temp3,*temp4;
	struct mat1Dlist *temp5,*temp6;

	/* Now go through the existing list and freeing those not used */

	for(i=0;i<7;++i)
	{
		if( ( mat4Dords[i][0] != a->xord || mat4Dords[i][1] != a->yord
		   || mat4Dords[i][2] != a->zord || mat4Dords[i][3] != a->word )
		 && ( mat4Dords[i][0] != b->xord || mat4Dords[i][1] != b->yord
		   || mat4Dords[i][2] != b->zord || mat4Dords[i][3] != b->word )
		 && ( mat4Dords[i][0] != c->xord || mat4Dords[i][1] != c->yord
		   || mat4Dords[i][2] != c->zord || mat4Dords[i][3] != c->word )
		 && ( mat4Dords[i][0] != d->xord || mat4Dords[i][1] != d->yord
		   || mat4Dords[i][2] != d->zord || mat4Dords[i][3] != d->word )
		 && ( mat4Dords[i][0] != e->xord || mat4Dords[i][1] != e->yord
		   || mat4Dords[i][2] != e->zord || mat4Dords[i][3] != e->word )
		 && ( mat4Dords[i][0] != f->xord || mat4Dords[i][1] != f->yord
		   || mat4Dords[i][2] != f->zord || mat4Dords[i][3] != f->word )
		 && ( mat4Dords[i][0] != g->xord || mat4Dords[i][1] != g->yord
		   || mat4Dords[i][2] != g->zord || mat4Dords[i][3] != g->word))
		{
			temp7 = list4Dmats[i];
			temp8 = temp7;
			while(temp7 != NULL)
			{
				temp8 = temp7->next;
				free(temp7);
				temp7 = temp8;
			}
			list4Dmats[i] = NULL;
			mat4Dords[i][0]= mat4Dords[i][1] = 
				mat4Dords[i][2] = mat4Dords[i][3] = -1;
		}
	}

	for(i=0;i<5;++i)
	{
		res =( mat3Dords[i][0] != a->xord || mat3Dords[i][1] != a->yord
		    || mat3Dords[i][2] != a->zord)
		 && ( mat3Dords[i][0] != a->xord || mat3Dords[i][1] != a->yord
		    || mat3Dords[i][2] != a->word)
		 && ( mat3Dords[i][0] != a->xord || mat3Dords[i][1] != a->zord
		    || mat3Dords[i][2] != a->word)
		 && ( mat3Dords[i][0] != a->yord || mat3Dords[i][1] != a->zord
		    || mat3Dords[i][2] != a->word);

		res = res &&
		    ( mat3Dords[i][0] != b->xord || mat3Dords[i][1] != b->yord
		    || mat3Dords[i][2] != b->zord)
		 && ( mat3Dords[i][0] != b->xord || mat3Dords[i][1] != b->yord
		    || mat3Dords[i][2] != b->word)
		 && ( mat3Dords[i][0] != b->xord || mat3Dords[i][1] != b->zord
		    || mat3Dords[i][2] != b->word)
		 && ( mat3Dords[i][0] != b->yord || mat3Dords[i][1] != b->zord
		    || mat3Dords[i][2] != b->word);

		res = res &&
		    ( mat3Dords[i][0] != c->xord || mat3Dords[i][1] != c->yord
		    || mat3Dords[i][2] != c->zord)
		 && ( mat3Dords[i][0] != c->xord || mat3Dords[i][1] != c->yord
		    || mat3Dords[i][2] != c->word)
		 && ( mat3Dords[i][0] != c->xord || mat3Dords[i][1] != c->zord
		    || mat3Dords[i][2] != c->word)
		 && ( mat3Dords[i][0] != c->yord || mat3Dords[i][1] != c->zord
		    || mat3Dords[i][2] != c->word);

		res = res &&
		    ( mat3Dords[i][0] != d->xord || mat3Dords[i][1] != d->yord
		    || mat3Dords[i][2] != d->zord)
		 && ( mat3Dords[i][0] != d->xord || mat3Dords[i][1] != d->yord
		    || mat3Dords[i][2] != d->word)
		 && ( mat3Dords[i][0] != d->xord || mat3Dords[i][1] != d->zord
		    || mat3Dords[i][2] != d->word)
		 && ( mat3Dords[i][0] != d->yord || mat3Dords[i][1] != d->zord
		    || mat3Dords[i][2] != d->word);

		res = res &&
		    ( mat3Dords[i][0] != e->xord || mat3Dords[i][1] != e->yord
		    || mat3Dords[i][2] != e->zord)
		 && ( mat3Dords[i][0] != e->xord || mat3Dords[i][1] != e->yord
		    || mat3Dords[i][2] != e->word)
		 && ( mat3Dords[i][0] != e->xord || mat3Dords[i][1] != e->zord
		    || mat3Dords[i][2] != e->word)
		 && ( mat3Dords[i][0] != e->yord || mat3Dords[i][1] != e->zord
		    || mat3Dords[i][2] != e->word);

		res = res &&
		    ( mat3Dords[i][0] != f->xord || mat3Dords[i][1] != f->yord
		    || mat3Dords[i][2] != f->zord)
		 && ( mat3Dords[i][0] != f->xord || mat3Dords[i][1] != f->yord
		    || mat3Dords[i][2] != f->word)
		 && ( mat3Dords[i][0] != f->xord || mat3Dords[i][1] != f->zord
		    || mat3Dords[i][2] != f->word)
		 && ( mat3Dords[i][0] != f->yord || mat3Dords[i][1] != f->zord
		    || mat3Dords[i][2] != f->word);

		res = res &&
		    ( mat3Dords[i][0] != g->xord || mat3Dords[i][1] != g->yord
		    || mat3Dords[i][2] != g->zord)
		 && ( mat3Dords[i][0] != g->xord || mat3Dords[i][1] != g->yord
		    || mat3Dords[i][2] != g->word)
		 && ( mat3Dords[i][0] != g->xord || mat3Dords[i][1] != g->zord
		    || mat3Dords[i][2] != g->word)
		 && ( mat3Dords[i][0] != g->yord || mat3Dords[i][1] != g->zord
		    || mat3Dords[i][2] != g->word);

		if( res )
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
		res = 
		    ( mat2Dords[i][0] != a->xord||mat2Dords[i][1] != a->yord )
		 && ( mat2Dords[i][0] != a->xord||mat2Dords[i][1] != a->zord )
		 && ( mat2Dords[i][0] != a->xord||mat2Dords[i][1] != a->word )
		 && ( mat2Dords[i][0] != a->yord||mat2Dords[i][1] != a->zord )
		 && ( mat2Dords[i][0] != a->yord||mat2Dords[i][1] != a->word )
		 && ( mat2Dords[i][0] != a->zord||mat2Dords[i][1] != a->word );

		res = res &&
		    ( mat2Dords[i][0] != b->xord||mat2Dords[i][1] != b->yord )
		 && ( mat2Dords[i][0] != b->xord||mat2Dords[i][1] != b->zord )
		 && ( mat2Dords[i][0] != b->xord||mat2Dords[i][1] != b->word )
		 && ( mat2Dords[i][0] != b->yord||mat2Dords[i][1] != b->zord )
		 && ( mat2Dords[i][0] != b->yord||mat2Dords[i][1] != b->word )
		 && ( mat2Dords[i][0] != b->zord||mat2Dords[i][1] != b->word );

		res = res &&
		    ( mat2Dords[i][0] != c->xord||mat2Dords[i][1] != c->yord )
		 && ( mat2Dords[i][0] != c->xord||mat2Dords[i][1] != c->zord )
		 && ( mat2Dords[i][0] != c->xord||mat2Dords[i][1] != c->word )
		 && ( mat2Dords[i][0] != c->yord||mat2Dords[i][1] != c->zord )
		 && ( mat2Dords[i][0] != c->yord||mat2Dords[i][1] != c->word )
		 && ( mat2Dords[i][0] != c->zord||mat2Dords[i][1] != c->word );

		res = res &&
		    ( mat2Dords[i][0] != d->xord||mat2Dords[i][1] != d->yord )
		 && ( mat2Dords[i][0] != d->xord||mat2Dords[i][1] != d->zord )
		 && ( mat2Dords[i][0] != d->xord||mat2Dords[i][1] != d->word )
		 && ( mat2Dords[i][0] != d->yord||mat2Dords[i][1] != d->zord )
		 && ( mat2Dords[i][0] != d->yord||mat2Dords[i][1] != d->word )
		 && ( mat2Dords[i][0] != d->zord||mat2Dords[i][1] != d->word );

		res = res &&
		    ( mat2Dords[i][0] != e->xord||mat2Dords[i][1] != e->yord )
		 && ( mat2Dords[i][0] != e->xord||mat2Dords[i][1] != e->zord )
		 && ( mat2Dords[i][0] != e->xord||mat2Dords[i][1] != e->word )
		 && ( mat2Dords[i][0] != e->yord||mat2Dords[i][1] != e->zord )
		 && ( mat2Dords[i][0] != e->yord||mat2Dords[i][1] != e->word )
		 && ( mat2Dords[i][0] != e->zord||mat2Dords[i][1] != e->word );

		res = res &&
		    ( mat2Dords[i][0] != f->xord||mat2Dords[i][1] != f->yord )
		 && ( mat2Dords[i][0] != f->xord||mat2Dords[i][1] != f->zord )
		 && ( mat2Dords[i][0] != f->xord||mat2Dords[i][1] != f->word )
		 && ( mat2Dords[i][0] != f->yord||mat2Dords[i][1] != f->zord )
		 && ( mat2Dords[i][0] != f->yord||mat2Dords[i][1] != f->word )
		 && ( mat2Dords[i][0] != f->zord||mat2Dords[i][1] != f->word );

		res = res &&
		    ( mat2Dords[i][0] != g->xord||mat2Dords[i][1] != g->yord )
		 && ( mat2Dords[i][0] != g->xord||mat2Dords[i][1] != g->zord )
		 && ( mat2Dords[i][0] != g->xord||mat2Dords[i][1] != g->word )
		 && ( mat2Dords[i][0] != g->yord||mat2Dords[i][1] != g->zord )
		 && ( mat2Dords[i][0] != g->yord||mat2Dords[i][1] != g->word )
		 && ( mat2Dords[i][0] != g->zord||mat2Dords[i][1] != g->word );

		if( res )
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
		 && mat1Dords[i] != a->word
		 && mat1Dords[i] != b->xord
		 && mat1Dords[i] != b->yord
		 && mat1Dords[i] != b->zord
		 && mat1Dords[i] != b->word
		 && mat1Dords[i] != c->xord
		 && mat1Dords[i] != c->yord
		 && mat1Dords[i] != c->zord
		 && mat1Dords[i] != c->word
		 && mat1Dords[i] != d->xord
		 && mat1Dords[i] != d->yord
		 && mat1Dords[i] != d->zord
		 && mat1Dords[i] != d->word
		 && mat1Dords[i] != e->xord
		 && mat1Dords[i] != e->yord
		 && mat1Dords[i] != e->zord
		 && mat1Dords[i] != e->word
		 && mat1Dords[i] != f->xord
		 && mat1Dords[i] != f->yord
		 && mat1Dords[i] != f->zord
		 && mat1Dords[i] != f->word
		 && mat1Dords[i] != g->xord
		 && mat1Dords[i] != g->yord
		 && mat1Dords[i] != g->zord
		 && mat1Dords[i] != g->word )
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


/*********************** Allocation **************************************/

/*
 * Function:	alloc_bern
 * Action:	allocates space for 4D bern size xord,yord,zord
 */

bern4D *alloc_bern4D(xord,yord,zord,word)
int xord,yord,zord,word;
{
	bern4D *temp;
	int	i;

	temp = (bern4D *) malloc(sizeof(bern4D));
	temp->xord = xord;
	temp->yord = yord;
	temp->zord = zord;
	temp->word = word;

	/* First find list */

	for(i=0;i<7;++i)
	{
		if( mat4Dords[i][0] == xord
		 && mat4Dords[i][1] == yord
		 && mat4Dords[i][2] == zord
		 && mat4Dords[i][3] == word )
		{
		    if(list4Dmats[i] != NULL )
		    {
			temp->xord = xord;
			temp->yord = yord;
			temp->zord = zord;
			temp->word = word;
			temp->array = (double *) list4Dmats[i];
			list4Dmats[i] = list4Dmats[i]->next;
			return(temp);
		    }
		    else
		    {
			temp->xord = xord;
			temp->yord = yord;
			temp->zord = zord;
			temp->word = word;
			temp->array = (double *)
			    calloc((size_t) (xord+1)*(yord+1)*(zord+1)*(word+1),
				sizeof(double));
			return(temp);
		    }
		}
	}
	temp->array = (double *)
		calloc((size_t) (xord+1)*(yord+1)*(zord+1)*(word+1),
			sizeof(double));
#ifdef PRI_ALLOC_BERN
	if((  mat4Dords[0][0] == -1 && mat4Dords[0][1] == -1
	   && mat4Dords[0][2] == -1 && mat4Dords[0][3] == -1)
	 ||(  mat4Dords[1][0] == -1 && mat4Dords[1][1] == -1
	   && mat4Dords[1][2] == -1 && mat4Dords[1][3] == -1)
	 ||(  mat4Dords[2][0] == -1 && mat4Dords[2][1] == -1
	   && mat4Dords[2][2] == -1 && mat4Dords[2][3] == -1)
	 ||(  mat4Dords[3][0] == -1 && mat4Dords[3][1] == -1
	   && mat4Dords[3][2] == -1 && mat4Dords[3][3] == -1)
	 ||(  mat4Dords[4][0] == -1 && mat4Dords[4][1] == -1
	   && mat4Dords[4][2] == -1 && mat4Dords[4][3] == -1)
	 ||(  mat4Dords[5][0] == -1 && mat4Dords[5][1] == -1
	   && mat4Dords[5][2] == -1 && mat4Dords[5][3] == -1)
	 ||(  mat4Dords[6][0] == -1 && mat4Dords[6][1] == -1
	   && mat4Dords[6][2] == -1 && mat4Dords[6][3] == -1) )
		return(temp);
	fprintf(stderr,"alloc_bern4D: error couldn't find match %d %d %d %d\n",
		xord,yord,zord,word);
	for(i=0;i<7;++i)
		fprintf(stderr,"ords[%d] %d %d %d %d\n",i,
			mat4Dords[i][0],mat4Dords[i][1],mat4Dords[i][2],
			mat4Dords[i][3]);
#endif
	return(temp);
}

/*
 * Function:	free_bern4D
 * Action:	frees the space used by bern poly
 */

free_bern4D(bb)
bern4D *bb;
{
	struct mat4Dlist *list;
	int i;

	if(bb == posbern4D || bb == negbern4D ) return;
	for(i=0;i<7;++i)
	{
		if( mat4Dords[i][0] == bb->xord
		 && mat4Dords[i][1] == bb->yord
		 && mat4Dords[i][2] == bb->zord 
		 && mat4Dords[i][3] == bb->word )
		{
			list = (struct mat4Dlist *) bb->array;
			list->next = list4Dmats[i];
			list4Dmats[i] = list;
			free(bb);
			return;
		}
	}

	/* Failed to find match  fill in empty mats */

	for(i=0;i<7;++i)
	{
		if( mat4Dords[i][0] == -1
		 && mat4Dords[i][1] == -1
		 && mat4Dords[i][2] == -1
		 && mat4Dords[i][3] == -1)
		{
			mat4Dords[i][0] = bb->xord;
			mat4Dords[i][1] = bb->yord;
			mat4Dords[i][2] = bb->zord;
			mat4Dords[i][3] = bb->word;
			list = (struct mat4Dlist *) bb->array;
			list->next = list4Dmats[i];
			list4Dmats[i] = list;
			free(bb);
			return;
		}
	}
	fprintf(stderr,"free_bern4D_mat: error couldn't find match\n");

/*
	fprintf(stderr,"free_bern4D_mat: count %d\n",--matcount);
*/
	free(bb->array);
	free(bb);
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

	for(i=0;i<MAX_NUM_3D_MATS;++i)
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
	
	for(i=0;i<MAX_NUM_3D_MATS;++i)
	if(mat3Dords[i][0]== -1&&mat3Dords[i][1] == -1&&mat3Dords[i][2] == -1)
		return(temp);
	fprintf(stderr,"alloc_bern3D: error couldn't find match %d %d %d\n",
		xord,yord,zord);
	for(i=0;i<MAX_NUM_3D_MATS;++i)
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
	for(i=0;i< MAX_NUM_3D_MATS;++i)
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

	for(i=0;i< MAX_NUM_3D_MATS;++i)
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
	fprintf(stderr,"free_bern2D_mat: error couldn't find match %d %d\n",
		bb->xord,bb->yord);
	for(i=0;i<15;++i)
		fprintf(stderr,"ords[%d] %d %d\n",i,
			mat2Dords[i][0],mat2Dords[i][1]);

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
 * Function:	free_hexbern4D
 * Action:	frees the elements of an hexbern4D
 */

free_hexbern4D(bb)
hexbern4D bb;
{
	if(bb.lfdi == posbern4D || bb.lfdi == negbern4D ) return;

	free_bern4D(bb.lfdi);
	free_bern4D(bb.rfdi);
	free_bern4D(bb.lbdi);
	free_bern4D(bb.rbdi);
	free_bern4D(bb.lfui);
	free_bern4D(bb.rfui);
	free_bern4D(bb.lbui);
	free_bern4D(bb.rbui);
	free_bern4D(bb.lfdo);
	free_bern4D(bb.rfdo);
	free_bern4D(bb.lbdo);
	free_bern4D(bb.rbdo);
	free_bern4D(bb.lfuo);
	free_bern4D(bb.rfuo);
	free_bern4D(bb.lbuo);
	free_bern4D(bb.rbuo);
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

/************** AllOneSign procedures *******************************/

/* checks if every element of a 4d array has strictly the same sign */

int aos4D(bb)
bern4D *bb;
{
   int i,j,k,l;

   if( bb == posbern4D ) return(1);
   if( bb == negbern4D ) return(-1);
   if( *bb->array < 0)
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
	    for(l=0;l<=bb->word;l++)
	       if( ele4D(bb,i,j,k,l) >= 0.0 )
		return(0);
   return(-1);
   }
   else
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
	    for(l=0;l<=bb->word;l++)
	       if( ele4D(bb,i,j,k,l) <= 0.0 )
		return(0);
   return(1);
   }
}

/* checks if every element of a 3d array has strictly the same sign */

int aos3D(bb)
bern3D *bb;
{
   int i,j,k;

   if( bb == posbern3D ) return(1);
   if( bb == negbern3D ) return(-1);
   if( *bb->array < 0)
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
            if( *(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k) >= 0.0)
		return(0);
   return(-1);
   }
   else
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
	 for(k=0;k<=bb->zord;k++)
            if( *(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k) <= 0.0)
		return(0);
   return(1);
   }
}

/* checks if every element of a 2d array has strictly the same sign */

int aos2D(bb)
bern2D *bb;
{
   int i,j;

   if( bb == posbern2D ) return(1);
   if( bb == negbern2D ) return(-1);
   if( *bb->array < 0)
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
            if( *(bb->array+i*(bb->yord+1)+j) >= 0.0) return(0);
   return(-1);
   }
   else
   {
   for(i=0;i<=bb->xord;i++)
      for(j=0;j<=bb->yord;j++)
            if( *(bb->array+i*(bb->yord+1)+j) <= 0.0) return(0);
   return(1);
   }
}

/* checks if every element of a 1d array has strictly the same sign */

int aos1D(bb)
bern1D *bb;
{
	int i;

   if( bb == posbern1D ) return(1);
   if( bb == negbern1D ) return(-1);
	if( *(bb->array) < 0)
	{
	for( i=1; i<=bb->ord; ++i)
		if( *(bb->array+i) >= 0.0 ) return(0);
	return(-1);
	}
	else
	{
	for( i=0; i<=bb->ord; ++i)
		if( *(bb->array+i) <= 0.0 ) return(0);
	return(1);
	}
}

/* check the derivatives have strictly the same sign */

int aosderiv1D(bb)
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

bern4D *formbernstein4D(aa,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax)
double aa[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];
double xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax;
{
   bern4D *bb;
   double c[MAXORDER];
   bern1D *d;
   int row,col,stack,slice;
   int xord,yord,zord,word;

   order_poly4(aa,&xord,&yord,&zord,&word);
   bb = alloc_bern4D(xord,yord,zord,word);
   d = alloc_bern1D(MAXORDER);

	/*** first convert polynomials in w ***/

   for(row = 0; row <= xord; row++) 
   for(col = 0; col <= yord; col++)
   for(stack = 0; stack <= zord; stack++)
   {
      for( slice = 0; slice <= word; ++slice)
		 c[slice] = aa[row][col][stack][slice];

      formbernstein1D(c,d,word,wmin,wmax);

      for( slice = 0; slice <= word; ++slice)
		 ele4D(bb,row,col,stack,slice) = *(d->array+slice);
   }

	/*** next polynomials in y ***/

   for(row = 0; row <= xord; row++) 
   for(col = 0; col <= yord; col++)
   for(slice = 0; slice <= word; ++slice)
   {
      for( stack = 0; stack <= zord; ++stack)
		c[stack] = ele4D(bb,row,col,stack,slice);

      formbernstein1D(c,d,zord,zmin,zmax);

      for( stack = 0; stack <= zord; ++stack)
		ele4D(bb,row,col,stack,slice) = *(d->array+stack);
   }

	/*** next polynomials in y ***/

   for(row = 0; row <= xord; row++)
   for(stack = 0; stack <= zord; ++stack)
   for(slice = 0; slice <= word; ++slice)
   {
      for(col = 0; col <= yord; col++)
		c[col] = ele4D(bb,row,col,stack,slice);
      formbernstein1D(c,d,yord,ymin,ymax);
      for(col = 0; col <= yord; col++)
		ele4D(bb,row,col,stack,slice) = *(d->array+col);
   }
	/*** Finally polynomial in x ***/

   for(col = 0; col <= yord; col++)
   for(stack = 0; stack <= zord; ++stack)
   for(slice = 0; slice <= word; ++slice)
   {
      for(row = 0; row <= xord; row++)
		c[row] = ele4D(bb,row,col,stack,slice);
      formbernstein1D(c,d,xord,xmin,xmax);
      for(row = 0; row <= xord; row++)
		ele4D(bb,row,col,stack,slice) = *(d->array+row);
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

bern4D *diffx4D(bb)
bern4D *bb;
{
   int row,col,stack,slice;
   bern4D *xderiv;

   if( bb == posbern4D || bb == negbern4D )
   {
	fprintf(stderr,"diffx4D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->xord == 0 )
   {
	xderiv = alloc_bern4D(0,0,0,0);
	return(xderiv);
   }
   xderiv = alloc_bern4D(bb->xord-1,bb->yord,bb->zord,bb->word);
   for(row=0; row<= bb->xord-1; row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0;stack<=bb->zord;stack++)
   for(slice=0;slice<=bb->word;slice++)
	ele4D(xderiv,row,col,stack,slice) =
		bb->xord * ( ele4D(bb,row+1,col,stack,slice) 
		           - ele4D(bb,row,col,stack,slice) );
   return(xderiv);
}

bern4D *diffy4D(bb)
bern4D *bb;
{
   int row,col,stack,slice;
   bern4D *yderiv;

   if( bb == posbern4D || bb == negbern4D )
   {
	fprintf(stderr,"diffy4D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->yord == 0 )
   {
	yderiv = alloc_bern4D(0,0,0,0);
	return(yderiv);
   }
   yderiv = alloc_bern4D(bb->xord,bb->yord-1,bb->zord,bb->word);
   for(row=0; row<= bb->xord; row++)
   for(col=0;col<=bb->yord-1;col++)
   for(stack=0;stack<=bb->zord;stack++)
   for(slice=0;slice<=bb->word;slice++)
	ele4D(yderiv,row,col,stack,slice) =
		bb->yord * ( ele4D(bb,row,col+1,stack,slice) 
		           - ele4D(bb,row,col,stack,slice) );
   return(yderiv);
}

bern4D *diffz4D(bb)
bern4D *bb;
{
   int row,col,stack,slice;
   bern4D *zderiv;

   if( bb == posbern4D || bb == negbern4D )
   {
	fprintf(stderr,"diffz4D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->zord == 0 )
   {
	zderiv = alloc_bern4D(0,0,0,0);
	return(zderiv);
   }
   zderiv = alloc_bern4D(bb->xord,bb->yord,bb->zord-1,bb->word);
   for(row=0; row<= bb->xord-1; row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0;stack<=bb->zord;stack++)
   for(slice=0;slice<=bb->word;slice++)
	ele4D(zderiv,row,col,stack,slice) =
		bb->zord * ( ele4D(bb,row,col,stack+1,slice) 
		           - ele4D(bb,row,col,stack,slice) );
   return(zderiv);
}

bern4D *diffw4D(bb)
bern4D *bb;
{
   int row,col,stack,slice;
   bern4D *wderiv;

   if( bb == posbern4D || bb == negbern4D )
   {
	fprintf(stderr,"diffw4D: tried to differentiate pos/negbern\n");
	return(bb);
   }
   if(bb->word == 0 )
   {
	wderiv = alloc_bern4D(0,0,0,0);
	return(wderiv);
   }
   wderiv = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word-1);
   for(row=0; row<= bb->xord-1; row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0;stack<=bb->zord;stack++)
   for(slice=0;slice<=bb->word;slice++)
	ele4D(wderiv,row,col,stack,slice) =
		bb->word * ( ele4D(bb,row,col,stack,slice+1) 
		           - ele4D(bb,row,col,stack,slice) );
   return(wderiv);
}

/*** Now the 3D version ***/

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

/*
double pyramid4D[6][6][6][6];
*/
double pyramid4D[SMALLORDERT2][SMALLORDERT2][SMALLORDERT2][SMALLORDERT2];
	 /* Global def saves heep space */

hexbern4D reduce4D(bb)
bern4D *bb;
{
   int row,col,stack,slice;
   int level;
   hexbern4D temp;

   if( bb == posbern4D )
   {
	temp.lfdi = temp.lfui = temp.lbdi = temp.rbdi =
	temp.rfdi = temp.rfui = temp.rbdi = temp.rbdi =
	temp.lfdi = temp.lfui = temp.lbdi = temp.rbdi =
	temp.rfdi = temp.rfui = temp.rbdi = temp.rbdi = posbern4D;
	temp.lfdo = temp.lfuo = temp.lbdo = temp.rbdo =
	temp.rfdo = temp.rfuo = temp.rbdo = temp.rbdo =
	temp.lfdo = temp.lfuo = temp.lbdo = temp.rbdo =
	temp.rfdo = temp.rfuo = temp.rbdo = temp.rbdo = posbern4D;
	return(temp);
   }
   if( bb == negbern4D )
   {
	temp.lfdi = temp.lfui = temp.lbdi = temp.rbdi =
	temp.rfdi = temp.rfui = temp.rbdi = temp.rbdi =
	temp.lfdi = temp.lfui = temp.lbdi = temp.rbdi =
	temp.rfdi = temp.rfui = temp.rbdi = temp.rbdi = negbern4D;
	temp.lfdo = temp.lfuo = temp.lbdo = temp.rbdo =
	temp.rfdo = temp.rfuo = temp.rbdo = temp.rbdo =
	temp.lfdo = temp.lfuo = temp.lbdo = temp.rbdo =
	temp.rfdo = temp.rfuo = temp.rbdo = temp.rbdo = negbern4D;
	return(temp);
   }

   temp.lfdi = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.lfui = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.lbdi = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.lbui = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rfdi = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rfui = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rbdi = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rbui = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);

   temp.lfdo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.lfuo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.lbdo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.lbuo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rfdo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rfuo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rbdo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);
   temp.rbuo = alloc_bern4D(bb->xord,bb->yord,bb->zord,bb->word);

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0; stack<=bb->zord; stack++)
   for(slice=0; slice<=bb->word; slice++)
         pyramid4D[2*row][2*col][2*stack][2*slice] = 
		ele4D(bb,row,col,stack,slice);

   for(level=1;level<=bb->xord;level++)
      for(row=level;row<= 2*bb->xord -level;row+=2)
      for(col=0;col<=2*bb->yord;col+=2)
      for(stack=0; stack<=2*bb->zord; stack+=2)
      for(slice=0; slice<=2*bb->word; slice+=2)
            pyramid4D[row][col][stack][slice] =
		0.5*(  pyramid4D[row-1][col][stack][slice]
		     + pyramid4D[row+1][col][stack][slice]);

   for(level=1;level<=bb->yord;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=level;col<=2*bb->yord-level;col+=2)
      for(stack=0; stack<=2*bb->zord; stack+=2)
      for(slice=0; slice<=2*bb->word; slice+=2)
            pyramid4D[row][col][stack][slice] =
		0.5*(  pyramid4D[row][col-1][stack][slice]
		     + pyramid4D[row][col+1][stack][slice]);

   for(level=1;level<=bb->zord;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=0;col<=2*bb->yord;++col)
      for(stack=level; stack<=2*bb->zord-level; stack+=2)
      for(slice=0; slice<=2*bb->word; slice+=2)
            pyramid4D[row][col][stack][slice] =
		0.5*(  pyramid4D[row][col][stack-1][slice]
		     + pyramid4D[row][col][stack+1][slice]);

   for(level=1;level<=bb->word;level++)
      for(row=0;row<=2*bb->xord;++row)
      for(col=0;col<=2*bb->yord;++col)
      for(stack=0; stack<=2*bb->zord; ++stack)
      for(slice=level; slice<=2*bb->word-level; slice+=2)
            pyramid4D[row][col][stack][slice] =
		0.5*(  pyramid4D[row][col][stack][slice-1]
		     + pyramid4D[row][col][stack][slice+1]);

   for(row=0;row<=bb->xord;row++)
   for(col=0;col<=bb->yord;col++)
   for(stack=0; stack<=bb->zord; stack++)
   for(slice=0; slice<=bb->word; slice++)
   {
	ele4D(temp.lfdi,row,col,stack,slice) =
		 pyramid4D[row][col][stack][slice];
	ele4D(temp.rfdi,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col][stack][slice];
	ele4D(temp.lbdi,row,col,stack,slice) =
		 pyramid4D[row][col+bb->yord][stack][slice];
	ele4D(temp.rbdi,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col+bb->yord][stack][slice];
	ele4D(temp.lfui,row,col,stack,slice) =
		 pyramid4D[row][col][stack+bb->zord][slice];
	ele4D(temp.rfui,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col][stack+bb->zord][slice];
	ele4D(temp.lbui,row,col,stack,slice) =
		 pyramid4D[row][col+bb->yord][stack+bb->zord][slice];
	ele4D(temp.rbui,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col+bb->yord][stack+bb->zord][slice];

	ele4D(temp.lfdo,row,col,stack,slice) =
		 pyramid4D[row][col][stack][slice+bb->word];
	ele4D(temp.rfdo,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col][stack][slice+bb->word];
	ele4D(temp.lbdo,row,col,stack,slice) =
		 pyramid4D[row][col+bb->yord][stack][slice+bb->word];
	ele4D(temp.rbdo,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col+bb->yord][stack][slice+bb->word];
	ele4D(temp.lfuo,row,col,stack,slice) =
		 pyramid4D[row][col][stack+bb->zord][slice+bb->word];
	ele4D(temp.rfuo,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col][stack+bb->zord][slice+bb->word];
	ele4D(temp.lbuo,row,col,stack,slice) =
		 pyramid4D[row][col+bb->yord][stack+bb->zord][slice+bb->word];
	ele4D(temp.rbuo,row,col,stack,slice) =
		 pyramid4D[row+bb->xord][col+bb->yord][stack+bb->zord][slice+bb->word];
   }

#ifdef PRINT_PYRIMID
   fprintf(stderr,"pyrimid4D\n");
   for(slice=0; slice <= bb->word*2; ++slice )
   {
   for(stack=0; stack <= bb->zord*2; ++stack )
   {
     for(row=0; row <= bb->xord*2; ++row )
    {
     for(col=0; col <= bb->yord*2; ++col ) 
       fprintf(stderr,"%f\t",pyramid4D[row][col][stack]);
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
   }
   fprintf(stderr,"\n");
   }
#endif
   return(temp);
}

double pyramid[MAXORDERT2][MAXORDERT2][MAXORDERT2];
	 /* Global def saves heep space */

octbern3D reduce3D(bb)
bern3D *bb;
{
   int row,col,stack;
   int level;
   octbern3D temp;

   if( bb == posbern3D )
   {
	temp.lfd = temp.rfd = temp.lbd = temp.rbd =
	temp.lfu = temp.rfu = temp.rbu = temp.rbu = posbern3D;
	return(temp);
   }
   if( bb == negbern3D )
   {
	temp.lfd = temp.rfd = temp.lbd = temp.rbd =
	temp.lfu = temp.rfu = temp.rbu = temp.rbu = negbern3D;
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

double evalbern4D(bb,vec)
bern4D *bb;
double vec[4];
{
	register int element,level;
	double workA[MAXORDERT2],workB[MAXORDERT2],workC[MAXORDERT2];
	double workD[MAXORDERT2];
	double oneminusroot,root;
	int i,j,k;

   if(bb == posbern4D )
   {
	fprintf(stderr,"evalbern4D: tryied to evaluate posbern4D\n");
	return(1.0);
   }
   if(bb == negbern4D )
   {
	fprintf(stderr,"evalbern4D: tryied to evaluate negbern4D\n");
	return(-1.0);
   }

   for(i=0;i<=bb->xord;++i)
   {
    	root = vec[3]; oneminusroot = 1.0 - root;

    	for(j=0;j<=bb->yord;++j)
    	{
	    for(k=0;k<=bb->zord;++k)
	    {
    		for(element=0;element<=bb->word;element++)
    		   workD[2*element] = ele4D(bb,i,j,k,element);

    		for(level=1;level<=bb->word;level++)
    		   for(element=level;element<=2*bb->word-level;
    				element+=2)
    		      workD[element] = oneminusroot * workD[element-1] +
    		          root * workD[element+1];

    		workC[k*2] = workD[bb->word];
	    }
    	    root = vec[2]; oneminusroot = 1.0 - root;

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

/*** Now routines which given find (n-1)D berns from nD berns ****/

/*
 * Function:	make_bern3D_of_hyper(bb,type,aa)
 * action:	create a 3D bernstein poly for face type of 3D bern-poly bb
 */

bern3D *make_bern3D_of_hyper(bb,type)
bern4D *bb;
soltype type;
{
	register int i,j,k;
	bern3D *aa;

	if( bb == posbern4D ) return(posbern3D);
	if( bb == negbern4D ) return(negbern3D);

	switch(type)
	{
	case BOX_LL:
		aa = alloc_bern3D(bb->yord,bb->zord,bb->word);
		for(i=0;i<=bb->yord;++i)
		    for(j=0;j<=bb->zord;++j)
		    for(k=0;k<=bb->word;++k)
			ele3D(aa,i,j,k) = ele4D(bb,0,i,j,k);
	break;
	case BOX_RR:
		aa = alloc_bern3D(bb->yord,bb->zord,bb->word);
		for(i=0;i<=bb->yord;++i)
		    for(j=0;j<=bb->zord;++j)
		    for(k=0;k<=bb->word;++k)
			ele3D(aa,i,j,k) = ele4D(bb,bb->xord,i,j,k);
	break;
	case BOX_FF:
		aa = alloc_bern3D(bb->xord,bb->zord,bb->word);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->zord;++j)
		    for(k=0;k<=bb->word;++k)
			ele3D(aa,i,j,k) = ele4D(bb,i,0,j,k);
	break;
	case BOX_BB:
		aa = alloc_bern3D(bb->xord,bb->zord,bb->word);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->zord;++j)
		    for(k=0;k<=bb->word;++k)
			ele3D(aa,i,j,k) = ele4D(bb,i,bb->yord,j,k);
	break;
	case BOX_DD:
		aa = alloc_bern3D(bb->xord,bb->yord,bb->word);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->yord;++j)
		    for(k=0;k<=bb->word;++k)
			ele3D(aa,i,j,k) = ele4D(bb,i,j,0,k);
	break;
	case BOX_UU:
		aa = alloc_bern3D(bb->xord,bb->yord,bb->word);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->yord;++j)
		    for(k=0;k<=bb->word;++k)
			ele3D(aa,i,j,k) = ele4D(bb,i,j,bb->zord,k);
	break;
	case BOX_II:
		aa = alloc_bern3D(bb->xord,bb->yord,bb->zord);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->yord;++j)
		    for(k=0;k<=bb->zord;++k)
			ele3D(aa,i,j,k) = ele4D(bb,i,j,k,0);
	break;
	case BOX_OO:
		aa = alloc_bern3D(bb->xord,bb->yord,bb->zord);
		for(i=0;i<=bb->xord;++i)
		    for(j=0;j<=bb->yord;++j)
		    for(k=0;k<=bb->zord;++k)
			ele3D(aa,i,j,k) = ele4D(bb,i,j,k,bb->word);
	break;
	}
	return(aa);
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

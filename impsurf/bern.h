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
/*	Header file for using bernstein polynomials.			*/
/*									*/
/************************************************************************/

#include <eqn.h>
#ifndef MAXORDER
#  define MAXORDER 25
#endif
#define MAXORDERT2 2*MAXORDER
#define MAXORDERP2 MAXORDER+2
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !(FALSE)
#endif
/*
*/
#define NEW_BERN

typedef struct
{
	int xord,yord,zord;
#ifdef NEW_BERN
	double *array;
#else
	double array[MAXORDER][MAXORDER][MAXORDER];
#endif
} bern3D;

typedef struct
{
	int xord,yord;
#ifdef NEW_BERN
	double *array;
#else
	double array[MAXORDER][MAXORDER];
#endif
} bern2D;

typedef struct
{
	int ord;
#ifdef NEW_BERN
	double *array;
#else
	double array[MAXORDER];
#endif
} bern1D;

/* Structures returned by the reduce routines */

typedef struct
{
	bern3D *lfd,*lfu,*lbd,*lbu,*rfd,*rfu,*rbd,*rbu;
} octbern3D;

typedef struct
{
	bern2D *lb,*rb,*lt,*rt;
} quadbern2D;

typedef struct
{
	bern1D *l,*r;
} binbern1D;

/* A bit of a memory saving cheet here, the following are pre-defined
	bernstein polys which represent all positive or all negative coeffs */

extern bern3D *posbern3D,*negbern3D;
extern bern2D *posbern2D,*negbern2D;
extern bern1D *posbern1D,*negbern1D;

/* Subroutines */

#ifndef I_AM_BERN
extern comb(int, int);
extern printzero(double);
extern printbern1D(bern1D *);
extern printbern2D(bern2D *);
extern printbern3D(bern3D *);
extern int allonesign3D(bern3D *);
extern int allonesign2D(bern2D *);
extern int allonesign1D(bern1D *);
extern int allonesignderiv1D(bern1D *);
extern double evalbern1D(bern1D *,double);
extern double evalbern2D(bern2D *,double[2]);
extern double evalbern3D(bern3D *,double[3]);
extern formbernstein1D( double[MAXORDER],bern1D *,int,double,double);
/*
extern reduce3D(bern3D *,
    bern3D *,bern3D *,bern3D *,bern3D *,bern3D *,bern3D *,bern3D *,bern3D *);
extern reduce2D(bern2D *,bern2D *,bern2D *,bern2D *,bern2D *);
extern reduce1D(bern1D *,bern1D *,bern1D *);
*/
extern octbern3D reduce3D(bern3D *);
extern quadbern2D reduce2D(bern2D *);
extern binbern1D reduce1D(bern1D *);
extern freeoctbern3D(octbern3D *);
extern freequadbern2D(quadbern2D *);
extern freebinbern1D(binbern1D *);
#ifdef NEW_BERN
extern bern3D *formbernstein3D();
extern bern3D *diffx3D(bern3D *);
extern bern3D *diffy3D(bern3D *);
extern bern3D *diffz3D(bern3D *);
extern bern2D *diffx2D(bern2D *);
extern bern2D *diffy2D(bern2D *);
extern bern1D *diff1D(bern1D *);
extern bern3D *alloc_bern3D(int,int,int);
extern bern2D *alloc_bern2D(int,int);
extern bern1D *alloc_bern1D(int);
extern alloc_bern3D_mat(bern3D *,int,int,int);
extern alloc_bern2D_mat(bern2D *,int,int);
extern alloc_bern1D_mat(bern1D *,int);
extern free_bern3D(bern3D *);
extern free_bern2D(bern2D *);
extern free_bern1D(bern1D *);
extern free_bern3D_mat(bern3D *);
extern free_bern2D_mat(bern2D *);
extern free_bern1D_mat(bern1D *);
extern bern2D *make_bern2D_of_box(bern3D *,int);
extern bern1D *make_bern1D_of_face(bern2D *,int);
#else
/*
extern formbernstein3D( double[MAXORDER][MAXORDER][MAXORDER],
 		 bern3D *,int,int,int,double,double,double,double,double,double);
*/
extern diffx3D(bern3D *,bern3D *);
extern diffy3D(bern3D *,bern3D *);
extern diffz3D(bern3D *,bern3D *);
extern diffx2D(bern2D *,bern2D *);
extern diffy2D(bern2D *,bern2D *);
extern diff1D(bern1D *,bern1D *);
#endif
#endif

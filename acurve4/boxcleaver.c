/*
 * Copyright I guess there should be some copywrite for this package,
 * 
 * 			Copyright (c) 1994
 * 
 * 	Liverpool University Department of Pure Mathematics,
 * 	Liverpool, L69 3BX, England.
 * 
 * 	Author Dr R. J. Morris.
 * 
 * 	e-mail rmorris@uk.ac.liv.uxb
 *
 */

#include <stdio.h>
#include "bern.h"
#include "cells.h"
#include <math.h>
#include <sys/types.h>
/*
#include <malloc.h>
*/
#include <sys/time.h>
#include <sys/resource.h>

/******************* RMAR GLOBALS ***********************************/

#define  numberofbitsinmantissaofrealnumber 27 /* at least for the */
						/* purposes of drawing */
/*
#define PRINT_PYRIMID
#define PRINT_SOLVEEDGE
#define PLOT_AS_LINES
#define PRINT_REDUCE_EDGE
#define PRINT_FOLLOW
#define PRINT_DRAW_HYPER
#define PRINT_SUSLINK
#define PRINT_PRE_COLLECT
#define TWO_PASS
#define PRINT_LINK_ALL_BOXS
#define CHECK_SECOND
#define PRINT_SECOND
#define PRINT_ODD_HYPER
#define PRINT_FIND_EDGE_SOLS
#define PRINT_SOL_VAL_ERR
#define PRINT_LINK_BOX_ALL
#define PRINT_LINK_NODES_ALL
#define OLD_CROSS
#define DO_FREE
#define PRINT_LINK_NODES_ALL
#define PRINT_LINK_NODES
#define PRINT_LINK_BOX_ALL
#define PRINT_LINK_BOX
#define PRINT_LINK_NODES_ALL
#define PRINT_LINK_BOX
#define PRINT_CHECK_BOX
#define PRINT_GEN_HYPERES
#define VERBOUSE
*/
#define CHECK_BOX
#define PRI_MALL
#define NON_GENERIC_NODES
#define LINK_SING
#define LINK_SOLS
#define LINK_BOX
#define DO_PLOT

#define MAX_EDGE_LEVEL 1048576 /* 32768 */
/*
unsigned int LINK_BOX_LEVEL = 64, LINK_SING_LEVEL = 16, RESOLUTION = 4;
unsigned int LINK_BOX_LEVEL = 4, LINK_SING_LEVEL = 2, RESOLUTION = 1;
unsigned int LINK_BOX_LEVEL = 32, LINK_SING_LEVEL = 8, RESOLUTION = 4;
unsigned int LINK_BOX_LEVEL = 256, LINK_SING_LEVEL = 64, RESOLUTION = 16;
*/
unsigned int LINK_BOX_LEVEL = 128, LINK_SING_LEVEL = 32, RESOLUTION = 8;
unsigned int SUPER_FINE = 8192;


#define EMPTY 0
#define FOUND_EVERYTHING 2
#define FOUND_BOXS 3

/*** A value returned by follow when there a sol not on an edge is found. ***/

#define NEW_NODE 2

#define grballoc(node) ( node * ) malloc( sizeof(node) )

extern sol_info *get_nth_sol_on_box();
extern node_info *get_nth_node_on_hyper();

hyper_info whole_hyper;
bern4D *AA,*BB,*CC;	/* The three main bernstein polys */
bern4D *DD,*EE,*FF,*GG;	/* Componants of product poly */

region_info region;

/*********************** Start of Sub-routines **************************/


/*
 * The main routine	*******************************************
 */

marmain(aa,bb,cc,dd,ee,ff,gg,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax)
double aa[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];  /* the input polynomial */
double bb[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];  /* the input polynomial */
double cc[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];  /* the input polynomial */
double dd[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];  /* the input polynomial */
double ee[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];  /* the input polynomial */
double ff[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];  /* the input polynomial */
double gg[SMALLORDER][SMALLORDER][SMALLORDER][SMALLORDER];  /* the input polynomial */
double xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax;
{
  region.xmin = xmin;
  region.ymin = ymin;
  region.zmin = zmin;
  region.wmin = wmin;
  region.xmax = xmax;
  region.ymax = ymax;
  region.zmax = zmax;
  region.wmax = wmax;

#ifdef PRI_MALL
  fprintf(stderr,"marmain\n");
  print_mallinfo();
#endif
  AA = formbernstein4D(aa,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
  BB = formbernstein4D(bb,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
  CC = formbernstein4D(cc,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
  DD = formbernstein4D(dd,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
  EE = formbernstein4D(ee,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
  FF = formbernstein4D(ff,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
  GG = formbernstein4D(gg,xmin,xmax,ymin,ymax,zmin,zmax,wmin,wmax);
#ifdef PRI_MALL
  fprintf(stderr,"marmain\n");
  print_mallinfo();
#endif

  init_berns(AA,BB,CC,DD,EE,FF,GG);
/*
  init_cells();
*/

#ifdef PRI_MALL
  fprintf(stderr,"...init\n");
  print_mallinfo();
#endif
#ifdef VERBOUSE
  fprintf(stderr,"range %f %f %f %f %f %f\n",
  	xmin,xmax,ymin,ymax,zmin,zmax);
  fprintf(stderr,"Bernstein polynomials are:\n");
  printbern4D(AA);
  fprintf(stderr,"\n");
  printbern4D(BB);
  fprintf(stderr,"\n");
  printbern4D(CC);
  fprintf(stderr,"\n");
/*
  printbern4D(DD);
  fprintf(stderr,"\n");
  printbern4D(EE);
  fprintf(stderr,"\n");
  printbern4D(FF);
  fprintf(stderr,"\n");
  printbern4D(GG);
*/
#endif
  make_hyper(&whole_hyper,0,0,0,0,1);
  generate_hypers(&whole_hyper,AA,BB,CC,DD,EE,FF,GG);
#ifdef PRI_MALL
  print_mallinfo();
#endif
  free_bern4D(AA);
  free_bern4D(BB);
  free_bern4D(CC);
  free_bern4D(DD);
  free_bern4D(EE);
  free_bern4D(FF);
  free_bern4D(GG);
  check_interupt("Writing Data");
}

/************************************************************************/
/*									*/
/*	The main routine for the first pass of the algorithim.		*/
/*	This recursivly creates a tree of hyperes where each hyper contains */
/*	eight smaller hyperes, only those hyperes where there might be a	*/
/*	solution are considered (i.e. !aos ).			*/
/*	The recursion ends when the none of the derivatives have	*/
/*	solutions, and a set depth has been reach or when 		*/
/*	a greater depth has been reached.				*/
/*									*/
/************************************************************************/

int generate_hypers(hyper,aa,bb,cc,dd,ee,ff,gg)
hyper_info *hyper;
bern4D *aa,*bb,*cc,*dd,*ee,*ff,*gg;
{
	int xl,yl,zl,wl,denom;
	double percent = 0.0;
	int flag;
	hexbern4D a1,b1,c1,d1,e1,f1,g1;
	char	string[40];

	xl = hyper->xl; yl = hyper->yl; zl = hyper->zl; wl = hyper->wl;
	for( denom = hyper->denom;denom>1;denom /= 2)
	{
		percent += (float)(xl%2);
		percent /= 2.0;
		xl /= 2;
		percent += (float)(yl%2);
		percent /= 2.0;
		yl /= 2;
		percent += (float)(zl%2);
		percent /= 2.0;
		zl /= 2;
		percent += (float)(wl%2);
		percent /= 2.0;
		wl /= 2;
	}
   sprintf(string,"Done %6.2lf percent.",percent*100.0);
   if( check_interupt( string ) ) return(FALSE);

   if( aos4D(aa) || aos4D(bb) || aos4D(cc) ) /* no componant in hyper */
   {
#ifdef PRINT_GEN_HYPERES
   	fprintf(stderr,"generate_hypers(%d,%d,%d,%d)/%d no conponant\n",
		hyper->xl,hyper->yl,hyper->zl,hyper->wl,hyper->denom);
#endif
	hyper->status = EMPTY;
	return(TRUE);
   }

      /*** If all derivitives non zero and the region is sufficiently	***/
      /***  small then draw the surbox.				***/

   if( hyper->denom >= RESOLUTION )
   {
#ifdef PRINT_GEN_HYPERES
   	fprintf(stderr,"generate_hypers(%d,%d,%d,%d)/%d LEAF\n",
		hyper->xl,hyper->yl,hyper->zl,hyper->wl,hyper->denom);
#endif

	return(find_hyper(hyper,aa,bb,cc,dd,ee,ff,gg));
   }
   else
   {		/**** Sub-devide the region into 16 sub hyperes.  ****/

#ifdef PRINT_GEN_HYPERES
   	fprintf(stderr,"generate_hypers(%d,%d,%d)/%d NODE\n",
		hyper->xl,hyper->yl,hyper->zl,hyper->denom);
	printbern4D(bb);
#endif

	a1 = reduce4D(aa);
	b1 = reduce4D(bb);
	c1 = reduce4D(cc);
	d1 = reduce4D(dd);
	e1 = reduce4D(ee);
	f1 = reduce4D(ff);
	g1 = reduce4D(gg);
	sub_devide_hyper(hyper);

	flag=
	   generate_hypers(hyper->lfdi,
		a1.lfdi,b1.lfdi,c1.lfdi,d1.lfdi,e1.lfdi,f1.lfdi,g1.lfdi) &&
	   generate_hypers(hyper->rfdi,
		a1.rfdi,b1.rfdi,c1.rfdi,d1.rfdi,e1.rfdi,f1.rfdi,g1.rfdi) &&
	   generate_hypers(hyper->lbdi,
		a1.lbdi,b1.lbdi,c1.lbdi,d1.lbdi,e1.lbdi,f1.lbdi,g1.lbdi) &&
	   generate_hypers(hyper->rbdi,
		a1.rbdi,b1.rbdi,c1.rbdi,d1.rbdi,e1.rbdi,f1.rbdi,g1.rbdi) &&
	   generate_hypers(hyper->lfui,
		a1.lfui,b1.lfui,c1.lfui,d1.lfui,e1.lfui,f1.lfui,g1.lfui) &&
	   generate_hypers(hyper->rfui,
		a1.rfui,b1.rfui,c1.rfui,d1.rfui,e1.rfui,f1.rfui,g1.rfui) &&
	   generate_hypers(hyper->lbui,
		a1.lbui,b1.lbui,c1.lbui,d1.lbui,e1.lbui,f1.lbui,g1.lbui) &&
	   generate_hypers(hyper->rbui,
		a1.rbui,b1.rbui,c1.rbui,d1.rbui,e1.rbui,f1.rbui,g1.rbui);

	flag = flag &&
	   generate_hypers(hyper->lfdo,
		a1.lfdo,b1.lfdo,c1.lfdo,d1.lfdo,e1.lfdo,f1.lfdo,g1.lfdo) &&
	   generate_hypers(hyper->rfdo,
		a1.rfdo,b1.rfdo,c1.rfdo,d1.rfdo,e1.rfdo,f1.rfdo,g1.rfdo) &&
	   generate_hypers(hyper->lbdo,
		a1.lbdo,b1.lbdo,c1.lbdo,d1.lbdo,e1.lbdo,f1.lbdo,g1.lbdo) &&
	   generate_hypers(hyper->rbdo,
		a1.rbdo,b1.rbdo,c1.rbdo,d1.rbdo,e1.rbdo,f1.rbdo,g1.rbdo) &&
	   generate_hypers(hyper->lfuo,
		a1.lfuo,b1.lfuo,c1.lfuo,d1.lfuo,e1.lfuo,f1.lfuo,g1.lfuo) &&
	   generate_hypers(hyper->rfuo,
		a1.rfuo,b1.rfuo,c1.rfuo,d1.rfuo,e1.rfuo,f1.rfuo,g1.rfuo) &&
	   generate_hypers(hyper->lbuo,
		a1.lbuo,b1.lbuo,c1.lbuo,d1.lbuo,e1.lbuo,f1.lbuo,g1.lbuo) &&
	   generate_hypers(hyper->rbuo,
		a1.rbuo,b1.rbuo,c1.rbuo,d1.rbuo,e1.rbuo,f1.rbuo,g1.rbuo);

	free_hexbern4D(a1);
	free_hexbern4D(b1);
	free_hexbern4D(c1);
	free_hexbern4D(d1);
	free_hexbern4D(e1);
	free_hexbern4D(f1);
	free_hexbern4D(g1);

	return(flag);
   }
}

/*
 * Function:	find_hyper
 * action:	finds all solutions, nodes and singularities for a hyper
 *		together with the topoligical linkage information.
 */

int find_hyper(hyper,aa,bb,cc,dd,ee,ff,gg)
hyper_info *hyper;
bern4D *aa,*bb,*cc,*dd,*ee,*ff,*gg;
{
	find_all_boxes(hyper,aa,bb,cc,dd,ee,ff,gg);
	if( !link_nodes(hyper,hyper,aa,bb,cc,dd,ee,ff,gg) ) return(FALSE);
	hyper->status = FOUND_EVERYTHING ;
	draw_hyper(hyper);
#ifdef DO_FREE
	free_bits_of_hyper(hyper);
#endif
	return(TRUE);
}

/*
 * Function:	find_all_boxes
 * action:	for all the boxes of the hyper find the information
 *		about the solutions and nodes.
 *		takes information already found about boxes of adjacient
 *		hypers.
 */

find_all_boxes(hyper,aa,bb,cc,dd,ee,ff,gg)
hyper_info *hyper;
bern4D *aa,*bb,*cc,*dd,*ee,*ff,*gg;
{
	get_existing_boxes(hyper);
	create_new_boxes(hyper);

	/* none of the boxes are internal */

#ifdef VERBOUSE
  fprintf(stderr,"find_all_boxes: Bernstein polynomials are:\n");
  printhyper(hyper);
  printbern4D(bb);
  fprintf(stderr,"\n");
/*
  printbern4D(aa);
  fprintf(stderr,"\n");
  printbern4D(cc);
  fprintf(stderr,"\n");
*/
#endif
	find_box(hyper->ll,aa,bb,cc,dd,ee,ff,gg,BOX_LL,FALSE);
	find_box(hyper->rr,aa,bb,cc,dd,ee,ff,gg,BOX_RR,FALSE);
	find_box(hyper->ff,aa,bb,cc,dd,ee,ff,gg,BOX_FF,FALSE);
	find_box(hyper->bb,aa,bb,cc,dd,ee,ff,gg,BOX_BB,FALSE);
	find_box(hyper->dd,aa,bb,cc,dd,ee,ff,gg,BOX_DD,FALSE);
	find_box(hyper->uu,aa,bb,cc,dd,ee,ff,gg,BOX_UU,FALSE);
	find_box(hyper->ii,aa,bb,cc,dd,ee,ff,gg,BOX_II,FALSE);
	find_box(hyper->oo,aa,bb,cc,dd,ee,ff,gg,BOX_OO,FALSE);
}

/*
 * Function:	find_box
 * action:	find all the information about solutions and nodes on box.
 */

find_box(box,aa,bb,cc,dd,ee,ff,gg,code,internal)
box_info *box;
bern4D *aa,*bb,*cc,*dd,*ee,*ff,*gg;
int code,internal;
{
	bern3D *a,*b,*c,*d,*e,*f,*g;

	if(box->status == FOUND_EVERYTHING ) return;
	a = make_bern3D_of_hyper(aa,code);
	b = make_bern3D_of_hyper(bb,code);
	c = make_bern3D_of_hyper(cc,code);
	if( aos3D(a) || aos3D(b) || aos3D(c) )
	{
		box->status = FOUND_EVERYTHING;
		free_bern3D(a);
		free_bern3D(b);
		free_bern3D(c);
		return;
	}
	d = make_bern3D_of_hyper(dd,code);
	e = make_bern3D_of_hyper(ee,code);
	f = make_bern3D_of_hyper(ff,code);
	g = make_bern3D_of_hyper(gg,code);
	link_box(box,box,a,b,c,d,e,f,g,internal);
	box->status = FOUND_EVERYTHING;
	free_bern3D(a);
	free_bern3D(b);
	free_bern3D(c);
	free_bern3D(d);
	free_bern3D(e);
	free_bern3D(f);
	free_bern3D(g);
}

/*
 * Function:	check_square
 * Action:	given the values of two functions at the
 *		cornors of a square, estimate whether the functions
 *		cross in the middle.
 */

check_square(a1,b1,c1,d1,a2,b2,c2,d2,x,y)
double a1,b1,c1,d1,a2,b2,c2,d2,*x,*y;
{
	double A,B,C,D;

	A = (a1-c1)*(b2-d2) + (d1-b1)*(a2-c2);
	B = 2*a2*b1-2*a1*b2 + a1*d2 - a2*d1 + b2*c1-b1*c2;
	C = a1*b2 - a2*b1;

	if( fabs(A) < 0.000001 )
	{
		*y = -C/B;
	B = 2*a2*c1-2*a1*c2 + a1*d2 - a2*d1 + c2*b1-c1*b2;
	C = a1*c2 - a2*c1;
		*x = -C/B;
		if( *x == *x && *x>=0.0 && *x <=1.0 
		 && *y == *y && *y>=0.0 && *y <=1.0 ) return(TRUE);
		return(FALSE);
	}

	D = B*B - 4*A*C;
	if(D > 0.0)
	{
		*y = (-B+sqrt(D))/(2*A);
		if( *y != *y )
		{
			fprintf(stderr,"check_face: y %f A %f B %f C %f D %f\n",
				*y,A,B,C,D);
		}
		if( *y == *y && *y>=0.0 && *y <=1.0 )
		{
			*x = (a1* *y-a1-c1* *y)/(a1* *y-a1+b1* *y-b1-c1* *y+d1* *y);
			if( *x == *x && *x >= 0.0 && *x <= 1.0 )
				return(TRUE);
		}

		*y = (-B-sqrt(D))/(2*A);
		if( *y == *y && *y>=0.0 && *y <=1.0 )
                {
                        *x = (a1* *y-a1-c1* *y)/(a1* *y-a1+b1* *y-b1-c1* *y+d1* *y);
			if( *x == *x && *x >= 0.0 && *x <= 1.0 )
				return(TRUE);
                }
	}
	return(FALSE);
}

/*
 * Function:	check_box
 * Action:	given a box where all three surfaces pass through,
 *		check to see whether they have a point in common.
 *		This can be done by finding the solutions on the
 *		edges for a and b, then finding an aprox for where
 *		they cross, should be at least 2, and checking
 *		wether c changes sign at these crossing points.
 *		Returns x,y,z a guess as to the intersection point.
 *
 *		Easier: assume that we have a deg 1 in x,y, &z
 *		so we just use the corner points. For a face
 *		a1 (1-x)(1-y)+b1 x(1-y)+c1 (1-x)y+d1 xy = 0
 *		a2 (1-x)(1-y)+b2 x(1-y)+c2 (1-x)y+d2 xy = 0
 *
 *		x =  [ a1(y-1) - c1 y]/[(a1-b1)(y-1)-(c1-d1)y]
 *		y = -B +/- sqrt(B^2-4AC) / 2A;
 *		A = b2 a1 -b1 a2 -c2 d1 -d2 a1 +a2 d1 +c1 d2 +b1 c2 -b2 c1
 *		B = (2 b1 a2 - a2 d1 - 2 b2 a1 + d2 a1 - b1 c2 + b2 c1
 *		C =  - b1 a2 + b2 a1
 *
 *		So we can get good aprox's to aa,bb crossing points
 *		on each face.
 */

#define ele3D(bb,i,j,k) (*(bb->array+(i*(bb->yord+1)+j)*(bb->zord+1)+k))

int check_box(box,aa,bb,cc,pos_x,pos_y,pos_z)
box_info *box;
bern3D *aa,*bb,*cc;
double *pos_x,*pos_y,*pos_z;
{
	double a1,b1,c1,d1,a2,b2,c2,d2;
	double A,B,C,D,lambda,res1,res2,x,y;
	double XYsols[12][3];
	int    i=0,j;

	/* first try the x = 0  face */

#ifdef PRINT_CHECK_BOX
	fprintf(stderr,"check_box:\n");
	printbox(box);
	printbern3D(aa);
	printbern3D(bb);
	printbern3D(cc);
#endif

	a1 = ele3D(aa,0,0,0);
	b1 = ele3D(aa,0,aa->yord,0);
	c1 = ele3D(aa,0,0,aa->zord);
	d1 = ele3D(aa,0,aa->yord,aa->zord);

	a2 = ele3D(bb,0,0,0);
	b2 = ele3D(bb,0,bb->yord,0);
	c2 = ele3D(bb,0,0,bb->zord);
	d2 = ele3D(bb,0,bb->yord,bb->zord);

	if(check_square(a1,b1,c1,d1,a2,b2,c2,d2,&x,&y))
	{
		XYsols[i][0] = 0.0;
		XYsols[i][1] = x;
		XYsols[i][2] = y;
		++i;
	}

	/* Now for x = 1 face */

	a1 = ele3D(aa,aa->xord,0,0);
	b1 = ele3D(aa,aa->xord,aa->yord,0);
	c1 = ele3D(aa,aa->xord,0,aa->zord);
	d1 = ele3D(aa,aa->xord,aa->yord,aa->zord);

	a2 = ele3D(bb,bb->xord,0,0);
	b2 = ele3D(bb,bb->xord,bb->yord,0);
	c2 = ele3D(bb,bb->xord,0,bb->zord);
	d2 = ele3D(bb,bb->xord,bb->yord,bb->zord);

	if(check_square(a1,b1,c1,d1,a2,b2,c2,d2,&x,&y))
	{
		XYsols[i][0] = 1.0;
		XYsols[i][1] = x;
		XYsols[i][2] = y;
		++i;
	}

	/* the y = 0  face */

	a1 = ele3D(aa,0,0,0);
	b1 = ele3D(aa,aa->xord,0,0);
	c1 = ele3D(aa,0,0,aa->zord);
	d1 = ele3D(aa,aa->xord,0,aa->zord);

	a2 = ele3D(bb,0,0,0);
	b2 = ele3D(bb,bb->xord,0,0);
	c2 = ele3D(bb,0,0,bb->zord);
	d2 = ele3D(bb,bb->xord,0,bb->zord);

	if(check_square(a1,b1,c1,d1,a2,b2,c2,d2,&x,&y))
	{
		XYsols[i][0] = x;
		XYsols[i][1] = 0.0;
		XYsols[i][2] = y;
		++i;
	}

	/* Now for y = 1 face */

	a1 = ele3D(aa,0,aa->yord,0);
	b1 = ele3D(aa,aa->xord,aa->yord,0);
	c1 = ele3D(aa,0,aa->yord,aa->zord);
	d1 = ele3D(aa,aa->xord,aa->yord,aa->zord);

	a2 = ele3D(bb,0,bb->yord,0);
	b2 = ele3D(bb,bb->xord,bb->yord,0);
	c2 = ele3D(bb,0,bb->yord,bb->zord);
	d2 = ele3D(bb,bb->xord,bb->yord,bb->zord);

	if(check_square(a1,b1,c1,d1,a2,b2,c2,d2,&x,&y))
	{
		XYsols[i][0] = x;
		XYsols[i][1] = 1.0;
		XYsols[i][2] = y;
		++i;
	}

	/* the z = 0  face */

	a1 = ele3D(aa,0,0,0);
	b1 = ele3D(aa,aa->xord,0,0);
	c1 = ele3D(aa,0,aa->yord,0);
	d1 = ele3D(aa,aa->xord,aa->yord,0);

	a2 = ele3D(bb,0,0,0);
	b2 = ele3D(bb,bb->xord,0,0);
	c2 = ele3D(bb,0,bb->yord,0);
	d2 = ele3D(bb,bb->xord,bb->yord,0);

	if(check_square(a1,b1,c1,d1,a2,b2,c2,d2,&x,&y))
	{
		XYsols[i][0] = x;
		XYsols[i][1] = y;
		XYsols[i][2] = 0.0;
		++i;
	}

	/* Now for z = 1 face */

	a1 = ele3D(aa,0,0,aa->zord);
	b1 = ele3D(aa,aa->xord,0,aa->zord);
	c1 = ele3D(aa,0,aa->yord,aa->zord);
	d1 = ele3D(aa,aa->xord,aa->yord,aa->zord);

	a2 = ele3D(bb,0,0,bb->zord);
	b2 = ele3D(bb,bb->xord,0,bb->zord);
	c2 = ele3D(bb,0,bb->yord,bb->zord);
	d2 = ele3D(bb,bb->xord,bb->yord,bb->zord);

	if(check_square(a1,b1,c1,d1,a2,b2,c2,d2,&x,&y))
	{
		XYsols[i][0] = x;
		XYsols[i][1] = y;
		XYsols[i][2] = 1.0;
		++i;
	}

	if(i>=1)
	{
		res1 = evalbern3D(cc,XYsols[0]);
#ifdef PRINT_CHECK_BOX
		fprintf(stderr,"sol[%d] %f %f %f = %f\n",0,
			XYsols[0][0],XYsols[0][1],XYsols[0][2],res1);
#endif
		for(j=1;j<i;++j)
		{
			res2 = evalbern3D(cc,XYsols[j]);
#ifdef PRINT_CHECK_BOX
		fprintf(stderr,"sol[%d] %f %f %f = %f\n",j,
			XYsols[j][0],XYsols[j][1],XYsols[j][2],res2);
#endif
			if( res1 * res2 < 0.0 )
			{
			lambda = res1 /(res1 - res2);
			if(lambda != lambda)
			{
				fprintf(stderr,"check_box: lambda %f res1 %f res2 %f\n",lambda,res1,res2);
				continue;
			}
			*pos_x=(1.0-lambda)*XYsols[0][0]+lambda*XYsols[j][0];
			*pos_y=(1.0-lambda)*XYsols[0][1]+lambda*XYsols[j][1];
			*pos_z=(1.0-lambda)*XYsols[0][2]+lambda*XYsols[j][2];
#ifdef PRINT_CHECK_BOX
	fprintf(stderr,"crossing found %f %f %f\n",*pos_x,*pos_y,*pos_z);
#endif
			return(TRUE);
			}
		}
	}
#ifdef PRINT_CHECK_BOX
	fprintf(stderr,"No crossing found\n");
#endif
	return(FALSE);
}

/*
 * Function:	link_box
 * action:	finds any nodes in the box, we don't actually
 *		do any think with links as they are not used in
 *		codim 1 case.
 *		There are two modes of operation:
 *		if internal is TRUE it indicates that the box is internal
 *		to a hyper, in which case we are just interested in
 *		the nodes and not the links.
 */

#define  no_second_derivatives_vanish TRUE

link_box(big_box,box,aa,bb,cc,dd,ee,ff,gg,internal)
box_info *big_box,*box;
bern3D *aa,*bb,*cc,*dd,*ee,*ff,*gg;
int internal;
{
	int f1,f2,f3,f4,res1,res2,res3;
	sol_info *sols;
	double pos_x,pos_y,pos_z;
	double posx1,posy1,posz1;
	double posx2,posy2,posz2;
	double posx3,posy3,posz3;
	octbern3D a,b,c,d,e,f,g;
	double v0[4];

	if( aos3D(aa) || aos3D(bb) || aos3D(cc) ) return;

	f1 = aos3D(dd);
	f2 = aos3D(ee);	
	f3 = aos3D(ff);	
	f4 = aos3D(gg);

#ifdef PRINT_LINK_BOX_ALL
		fprintf(stderr,"link_box: f1 %d f2 %d f3 %d f4 %d ",f1,f2,f3,f4);
		printsoltype(box->type);
		fprintf(stderr," (%d,%d,%d,%d)/%d\n",
			box->xl,box->yl,box->zl,box->wl,box->denom);
#ifdef VERBOUSE
  fprintf(stderr,"find_box: Bernstein polynomials are:\n");
  printbern3D(aa);
  fprintf(stderr,"\n");
  printbern3D(bb);
  fprintf(stderr,"\n");
  printbern3D(cc);
  fprintf(stderr,"\n");
#endif
#endif
	if( box->denom >= LINK_BOX_LEVEL )
	{
		/*** Posibly a node must first find if solutions cross ***/
		/*** Code deleted see intersect prog ***/

		pos_x = pos_y = pos_z = 0.5;
#ifdef CHECK_BOX
/*
		if( f1 && f2 && f3 && f4 )
*/
		{
			res1 = check_box(box,aa,bb,cc,&posx1,&posy1,&posz1);
			res2 = check_box(box,bb,cc,aa,&posx2,&posy2,&posz2);
			res3 = check_box(box,cc,aa,bb,&posx3,&posy3,&posz3);
/*
			if( res1 != res2 || res1 != res3 )
			{
				fprintf(stderr,"res1 %d %f %f %f\n",
					res1,posx1,posy1,posz1);
				fprintf(stderr,"res2 %d %f %f %f\n",
					res2,posx2,posy2,posz2);
				fprintf(stderr,"res3 %d %f %f %f\n",
					res3,posx3,posy3,posz3);
				printbox(box);
  printbern3D(aa);
  fprintf(stderr,"\n");
  printbern3D(bb);
  fprintf(stderr,"\n");
  printbern3D(cc);
  fprintf(stderr,"\n");
			}
			if( !res1 || !res2 || !res3 ) return;
			if( ( !res1 && !res2 ) || ( !res1 && !res3 ) 
			 || ( !res2 && !res3 ) ) return;
*/
			if( !res1 && !res2 && !res3 ) return;
			if( res1 ) { pos_x = posx1; pos_y=posy1; pos_z = posz1;}
			if( res2 ) { pos_x = posx2; pos_y=posy2; pos_z = posz2;}
			if( res3 ) { pos_x = posx3; pos_y=posy3; pos_z = posz3;}
		}
#endif
								
		sols = make_sol3(box->type,
				box->xl,box->yl,box->zl,box->wl,
				box->denom, pos_x,pos_y,pos_z );
		sols->dx = f1;
		sols->dy = f2;
		sols->dz = f3;
		sols->dw = f4;
#ifdef PRINT_LINK_BOX
		fprintf(stderr,"Sol ");
		print_sol(sols);
		calc_pos(sols,v0);
		fprintf(stderr,"Pos %f %f %f %f Vals %f %f %f\n",
			v0[0],v0[1],v0[2],v0[3],
			evalbern4D(AA,v0),evalbern4D(BB,v0),evalbern4D(CC,v0));
#endif
		add_node(big_box,sols);

		return;
	}
	else
	{
		box->lfd = alloc_box();
		box->rfd = alloc_box();
		box->lbd = alloc_box();
		box->rbd = alloc_box();
		box->lfu = alloc_box();
		box->rfu = alloc_box();
		box->lbu = alloc_box();
		box->rbu = alloc_box();

		a = reduce3D(aa);
		b = reduce3D(bb);
		c = reduce3D(cc);
		d = reduce3D(dd);
		e = reduce3D(ee);
		f = reduce3D(ff);
		g = reduce3D(gg);

		make_sub_boxes(box,
			box->lfd,box->rfd,box->lbd,box->rbd,
			box->lfu,box->rfu,box->lbu,box->rbu);

		link_box(big_box,box->lfd,
			a.lfd,b.lfd,c.lfd,d.lfd,e.lfd,f.lfd,g.lfd, internal);
		link_box(big_box,box->rfd,
			a.rfd,b.rfd,c.rfd,d.rfd,e.rfd,f.rfd,g.rfd, internal);
		link_box(big_box,box->lbd,
			a.lbd,b.lbd,c.lbd,d.lbd,e.lbd,f.lbd,g.lbd,internal);
		link_box(big_box,box->rbd,
			a.rbd,b.rbd,c.rbd,d.rbd,e.rbd,f.rbd,g.rbd, internal);

		link_box(big_box,box->lfu,
			a.lfu,b.lfu,c.lfu,d.lfu,e.lfu,f.lfu,g.lfu, internal);
		link_box(big_box,box->rfu,
			a.rfu,b.rfu,c.rfu,d.rfu,e.rfu,f.rfu,g.rfu, internal);
		link_box(big_box,box->lbu,
			a.lbu,b.lbu,c.lbu,d.lbu,e.lbu,f.lbu,g.lbu, internal);
		link_box(big_box,box->rbu,
			a.rbu,b.rbu,c.rbu,d.rbu,e.rbu,f.rbu,g.rbu, internal);

		free_octbern3D(a);
		free_octbern3D(b);
		free_octbern3D(c);
		free_octbern3D(d);
		free_octbern3D(e);
		free_octbern3D(f);
		free_octbern3D(g);

	}
}

/*
 * Function:	link_nodes(hyper,big_hyper)
 * action:	links together the nodes surronding a hyper.
 *		adds the links to the list in big_hyper.
 *		Returns FALSE if abort caught
 */

int link_nodes(big_hyper,hyper,aa,bb,cc,dd,ee,ff,gg)
hyper_info *big_hyper,*hyper;
bern4D *aa,*bb,*cc,*dd,*ee,*ff,*gg;
{
	int f1,f2,f3,f4,count,i;
	double vec0[4],vec1[4];
	node_info *nodes[4];
	hexbern4D a,b,c,d,e,f,g;
	int flag;

   	if( check_interupt( NULL ) ) return(FALSE);
	if( aos4D(aa) || aos4D(bb) || aos4D(cc) ) return(TRUE);

	f1 = aos4D(dd);
	f2 = aos4D(ee);	
	f3 = aos4D(ff);
	f4 = aos4D(gg);

	count = get_nodes_on_hyper_boxes(hyper,nodes);

#ifdef PRINT_LINK_NODES_ALL
		fprintf(stderr,
"link_nodes: count = %d f1 %d f2 %d f3 %d f4 %d HYPER (%d,%d,%d,%d)/%d\n",
count,f1,f2,f3,f4,hyper->xl,hyper->yl,hyper->zl,hyper->wl,hyper->denom);
		printhyper(hyper);
#endif

	if( count == 0 )
	{
		/* test for isolated zeros: require !f1 !f2 !f3 and */

		if( f1 || f2 || f3 || f4 ) return(TRUE);
	}
	else if( count == 2 )
	{

		if( nodes[0]->sol->dx == nodes[1]->sol->dx
		 && nodes[0]->sol->dy == nodes[1]->sol->dy
		 && nodes[0]->sol->dz == nodes[1]->sol->dz 
		 && nodes[0]->sol->dw == nodes[1]->sol->dw 
		 && nodes[0]->sol->dx == f1
		 && nodes[0]->sol->dy == f2
		 && nodes[0]->sol->dz == f3
		 && nodes[0]->sol->dw == f4 )
		 {

#ifdef PRINT_LINK_NODES_ALL
		fprintf(stderr,
"link_nodes: count = %d f1 %d f2 %d f3 %d f4 %d HYPER (%d,%d,%d,%d)/%d\n",
count,f1,f2,f3,f4,hyper->xl,hyper->yl,hyper->zl,hyper->wl,hyper->denom);

		for( i=1; i<=count;++i)
			print_node(get_nth_node_on_hyper(hyper,i));
#endif
		add_node_link(big_hyper,nodes[0],nodes[1]);
		return(TRUE);
		}
	}

	else if( count == 4 )
	{
		/* Pair wise test */
		/* Suceeds if nodes A,B match C,D match and
			A,C dont match */
		/* Must also check other derivative non-zero */

		nodes[2] = get_nth_node_on_hyper(hyper,3);
		nodes[3] = get_nth_node_on_hyper(hyper,4);

		if( nodes[0]->sol->dx == nodes[1]->sol->dx
		 && nodes[0]->sol->dy == nodes[1]->sol->dy
		 && nodes[0]->sol->dz == nodes[1]->sol->dz
		 && nodes[0]->sol->dw == nodes[1]->sol->dw )
		{
		  if( nodes[2]->sol->dx == nodes[3]->sol->dx
		   && nodes[2]->sol->dy == nodes[3]->sol->dy
		   && nodes[2]->sol->dz == nodes[3]->sol->dz
		   && nodes[2]->sol->dw == nodes[3]->sol->dw )
		  {
			/* Match 0,1  2,3 */

		    if( nodes[0]->sol->dx == nodes[3]->sol->dx
		     && nodes[0]->sol->dy == nodes[3]->sol->dy
		     && nodes[0]->sol->dz == nodes[3]->sol->dz
		     && nodes[0]->sol->dw == nodes[3]->sol->dw )
		    {
			/* match 0,3 */
		    }
		    else
		    {
			/* No match 0,3 */

			add_node_link(big_hyper,nodes[0],nodes[1]);
			add_node_link(big_hyper,nodes[2],nodes[3]);
			return(TRUE);
		    }
		  }
		}
		else if( nodes[0]->sol->dx == nodes[2]->sol->dx
		   && nodes[0]->sol->dy == nodes[2]->sol->dy
		   && nodes[0]->sol->dz == nodes[2]->sol->dz
		   && nodes[0]->sol->dw == nodes[2]->sol->dw )
		{
		  if( nodes[1]->sol->dx == nodes[3]->sol->dx
		   && nodes[1]->sol->dy == nodes[3]->sol->dy
		   && nodes[1]->sol->dz == nodes[3]->sol->dz
		   && nodes[1]->sol->dw == nodes[3]->sol->dw )
		  {
			/* Match 0,2  1,3 */

			add_node_link(big_hyper,nodes[0],nodes[2]);
			add_node_link(big_hyper,nodes[1],nodes[3]);
			return(TRUE);
		  }
		}
		else if( nodes[0]->sol->dx == nodes[3]->sol->dx
		   && nodes[0]->sol->dy == nodes[3]->sol->dy
		   && nodes[0]->sol->dz == nodes[3]->sol->dz
		   && nodes[0]->sol->dw == nodes[3]->sol->dw )
		{
		  if( nodes[1]->sol->dx == nodes[2]->sol->dx
		   && nodes[1]->sol->dy == nodes[2]->sol->dy
		   && nodes[1]->sol->dz == nodes[2]->sol->dz
		   && nodes[1]->sol->dw == nodes[2]->sol->dw )
		  {
			/* Match 0,3  1,2 */

			add_node_link(big_hyper,nodes[0],nodes[3]);
			add_node_link(big_hyper,nodes[1],nodes[2]);
			return(TRUE);
		  }
		}
	}

	/*** Too dificult to handle, either sub-devide or create a node ***/

	if( hyper->denom >= LINK_SING_LEVEL )
	{
		sol_info *sol;

		/* Get average of all soln's */

		vec0[0] = vec0[1] = vec0[2] =  vec0[3] = 0.0;

		for(i=1; i<=count; ++i)
		{
			nodes[1] = get_nth_node_on_hyper(hyper,i);
			calc_pos_in_hyper(hyper,nodes[1]->sol,vec1);
			vec0[0] += vec1[0];
			vec0[1] += vec1[1];
			vec0[2] += vec1[2];
			vec0[3] += vec1[3];
		}
		if( count == 0 )
			vec0[0] = vec0[1] = vec0[2] = vec0[3] = 0.5;
		else
		{
			vec0[0] /= count;
			vec0[1] /= count;
			vec0[2] /= count;
			vec0[3] /= count;
		}
#ifdef PRINT_LINK_NODES
		fprintf(stderr,
"link_nodes: count = %d f1 %d f2 %d f3 %d f4 %d HYPER (%d,%d,%d,%d)/%d\n",
count,f1,f2,f3,f4,hyper->xl,hyper->yl,hyper->zl,hyper->wl,hyper->denom);

		for( i=1; i<=count;++i)
			print_node(get_nth_node_on_hyper(hyper,i));
#endif

		sol = make_sol4(HYPER,hyper->xl,hyper->yl,hyper->zl,hyper->wl,
			hyper->denom, vec0[0],vec0[1],vec0[2],vec0[3] );
		sol->dx = f1;
		sol->dy = f2;
		sol->dz = f3;
		sol->dw = f4;
		add_sing(big_hyper,sol);
		if( count == 0 ) return(TRUE);
		nodes[0] = alloc_node();
		nodes[0]->next = NULL;
		nodes[0]->sol = sol;
		nodes[0]->status = NODE;
		for( i=1; i<=count; ++i)
		{
			nodes[1] = get_nth_node_on_hyper(hyper,i);
			add_node_link(big_hyper,nodes[0],nodes[1]);
		}
		return(TRUE);
	}
	else
	{
                a = reduce4D(aa);
                b = reduce4D(bb);
                c = reduce4D(cc);
                if( f1 > 0 )      d = reduce4D(posbern4D);
		else if( f1 < 0 ) d = reduce4D(negbern4D);
		else		  d = reduce4D(dd);
                if( f2 > 0 )      e = reduce4D(posbern4D);
		else if( f2 < 0 ) e = reduce4D(negbern4D);
		else		  e = reduce4D(ee);
                if( f3 > 0 )      f = reduce4D(posbern4D);
		else if( f3 < 0 ) f = reduce4D(negbern4D);
		else		  f = reduce4D(ff);
                if( f4 > 0 )      g = reduce4D(posbern4D);
		else if( f4 < 0 ) g = reduce4D(negbern4D);
		else		  g = reduce4D(gg);
                sub_devide_hyper(hyper);
		split_hyper(hyper,
			hyper->lfdi,hyper->rfdi,hyper->lbdi,hyper->rbdi,
                        hyper->lfui,hyper->rfui,hyper->lbui,hyper->rbui,
			hyper->lfdo,hyper->rfdo,hyper->lbdo,hyper->rbdo,
                        hyper->lfuo,hyper->rfuo,hyper->lbuo,hyper->rbuo);

		/* Now we find the internal boxes in the hyper cube,
		   there are 16 sub hypercubes, each with 8 boxes,
		   128 in all, 64 are internal, we find the boxes on
		   lfdi,rbdi,lbui,rfui,rfdo,lbdo,lfuo,rbuo */

		find_box(hyper->lfdi->rr,
			a.lfdi,b.lfdi,c.lfdi,d.lfdi,e.lfdi,f.lfdi,g.lfdi,
			BOX_RR,TRUE);
		find_box(hyper->lfdi->bb,
			a.lfdi,b.lfdi,c.lfdi,d.lfdi,e.lfdi,f.lfdi,g.lfdi,
			BOX_BB,TRUE);
		find_box(hyper->lfdi->uu,
			a.lfdi,b.lfdi,c.lfdi,d.lfdi,e.lfdi,f.lfdi,g.lfdi,
			BOX_UU,TRUE);
		find_box(hyper->lfdi->oo,
			a.lfdi,b.lfdi,c.lfdi,d.lfdi,e.lfdi,f.lfdi,g.lfdi,
			BOX_OO,TRUE);
		hyper->lfdi->status = FOUND_BOXS;

		find_box(hyper->rbdi->ll,
			a.rbdi,b.rbdi,c.rbdi,d.rbdi,e.rbdi,f.rbdi,g.rbdi,
			BOX_LL,TRUE);
		find_box(hyper->rbdi->ff,
			a.rbdi,b.rbdi,c.rbdi,d.rbdi,e.rbdi,f.rbdi,g.rbdi,
			BOX_FF,TRUE);
		find_box(hyper->rbdi->uu,
			a.rbdi,b.rbdi,c.rbdi,d.rbdi,e.rbdi,f.rbdi,g.rbdi,
			BOX_UU,TRUE);
		find_box(hyper->rbdi->oo,
			a.rbdi,b.rbdi,c.rbdi,d.rbdi,e.rbdi,f.rbdi,g.rbdi,
			BOX_OO,TRUE);
		hyper->rbdi->status = FOUND_BOXS;

		find_box(hyper->rfui->ll,
			a.rfui,b.rfui,c.rfui,d.rfui,e.rfui,f.rfui,g.rfui,
			BOX_LL,TRUE);
		find_box(hyper->rfui->bb,
			a.rfui,b.rfui,c.rfui,d.rfui,e.rfui,f.rfui,g.rfui,
			BOX_BB,TRUE);
		find_box(hyper->rfui->dd,
			a.rfui,b.rfui,c.rfui,d.rfui,e.rfui,f.rfui,g.rfui,
			BOX_DD,TRUE);
		find_box(hyper->rfui->oo,
			a.rfui,b.rfui,c.rfui,d.rfui,e.rfui,f.rfui,g.rfui,
			BOX_OO,TRUE);
		hyper->rfui->status = FOUND_BOXS;

		find_box(hyper->lbui->rr,
			a.lbui,b.lbui,c.lbui,d.lbui,e.lbui,f.lbui,g.lbui,
			BOX_RR,TRUE);
		find_box(hyper->lbui->ff,
			a.lbui,b.lbui,c.lbui,d.lbui,e.lbui,f.lbui,g.lbui,
			BOX_FF,TRUE);
		find_box(hyper->lbui->dd,
			a.lbui,b.lbui,c.lbui,d.lbui,e.lbui,f.lbui,g.lbui,
			BOX_DD,TRUE);
		find_box(hyper->lbui->oo,
			a.lbui,b.lbui,c.lbui,d.lbui,e.lbui,f.lbui,g.lbui,
			BOX_OO,TRUE);
		hyper->lbui->status = FOUND_BOXS;

		/* Now rfdo */

		find_box(hyper->rfdo->ll,
			a.rfdo,b.rfdo,c.rfdo,d.rfdo,e.rfdo,f.rfdo,g.rfdo,
			BOX_LL,TRUE);
		find_box(hyper->rfdo->bb,
			a.rfdo,b.rfdo,c.rfdo,d.rfdo,e.rfdo,f.rfdo,g.rfdo,
			BOX_BB,TRUE);
		find_box(hyper->rfdo->uu,
			a.rfdo,b.rfdo,c.rfdo,d.rfdo,e.rfdo,f.rfdo,g.rfdo,
			BOX_UU,TRUE);
		find_box(hyper->rfdo->ii,
			a.rfdo,b.rfdo,c.rfdo,d.rfdo,e.rfdo,f.rfdo,g.rfdo,
			BOX_II,TRUE);
		hyper->rfdo->status = FOUND_BOXS;
		hyper->rfdi->status = FOUND_BOXS;

		find_box(hyper->lbdo->rr,
			a.lbdo,b.lbdo,c.lbdo,d.lbdo,e.lbdo,f.lbdo,g.lbdo,
			BOX_RR,TRUE);
		find_box(hyper->lbdo->ff,
			a.lbdo,b.lbdo,c.lbdo,d.lbdo,e.lbdo,f.lbdo,g.lbdo,
			BOX_FF,TRUE);
		find_box(hyper->lbdo->uu,
			a.lbdo,b.lbdo,c.lbdo,d.lbdo,e.lbdo,f.lbdo,g.lbdo,
			BOX_UU,TRUE);
		find_box(hyper->lbdo->ii,
			a.lbdo,b.lbdo,c.lbdo,d.lbdo,e.lbdo,f.lbdo,g.lbdo,
			BOX_II,TRUE);
		hyper->lbdo->status = FOUND_BOXS;
		hyper->lbdi->status = FOUND_BOXS;

		find_box(hyper->lfuo->rr,
			a.lfuo,b.lfuo,c.lfuo,d.lfuo,e.lfuo,f.lfuo,g.lfuo,
			BOX_RR,TRUE);
		find_box(hyper->lfuo->bb,
			a.lfuo,b.lfuo,c.lfuo,d.lfuo,e.lfuo,f.lfuo,g.lfuo,
			BOX_BB,TRUE);
		find_box(hyper->lfuo->dd,
			a.lfuo,b.lfuo,c.lfuo,d.lfuo,e.lfuo,f.lfuo,g.lfuo,
			BOX_DD,TRUE);
		find_box(hyper->lfuo->ii,
			a.lfuo,b.lfuo,c.lfuo,d.lfuo,e.lfuo,f.lfuo,g.lfuo,
			BOX_II,TRUE);
		hyper->lfuo->status = FOUND_BOXS;
		hyper->lfui->status = FOUND_BOXS;
		hyper->lfdo->status = FOUND_BOXS;

		find_box(hyper->rbuo->ll,
			a.rbuo,b.rbuo,c.rbuo,d.rbuo,e.rbuo,f.rbuo,g.rbuo,
			BOX_LL,TRUE);
		find_box(hyper->rbuo->ff,
			a.rbuo,b.rbuo,c.rbuo,d.rbuo,e.rbuo,f.rbuo,g.rbuo,
			BOX_FF,TRUE);
		find_box(hyper->rbuo->dd,
			a.rbuo,b.rbuo,c.rbuo,d.rbuo,e.rbuo,f.rbuo,g.rbuo,
			BOX_DD,TRUE);
		find_box(hyper->rbuo->ii,
			a.rbuo,b.rbuo,c.rbuo,d.rbuo,e.rbuo,f.rbuo,g.rbuo,
			BOX_II,TRUE);
		hyper->rbuo->status = FOUND_BOXS;
		hyper->rbui->status = FOUND_BOXS;
		hyper->rbdo->status = FOUND_BOXS;
		hyper->rfuo->status = FOUND_BOXS;
		hyper->lbuo->status = FOUND_BOXS;

		flag = 
                   link_nodes(big_hyper,hyper->lfdi,
			a.lfdi,b.lfdi,c.lfdi,d.lfdi,e.lfdi,f.lfdi,g.lfdi)
                && link_nodes(big_hyper,hyper->rfdi,
			a.rfdi,b.rfdi,c.rfdi,d.rfdi,e.rfdi,f.rfdi,g.rfdi)
                && link_nodes(big_hyper,hyper->lbdi,
			a.lbdi,b.lbdi,c.lbdi,d.lbdi,e.lbdi,f.lbdi,g.lbdi)
                && link_nodes(big_hyper,hyper->rbdi,
			a.rbdi,b.rbdi,c.rbdi,d.rbdi,e.rbdi,f.rbdi,g.rbdi)
                && link_nodes(big_hyper,hyper->lfui,
			a.lfui,b.lfui,c.lfui,d.lfui,e.lfui,f.lfui,g.lfui)
                && link_nodes(big_hyper,hyper->rfui,
			a.rfui,b.rfui,c.rfui,d.rfui,e.rfui,f.rfui,g.rfui)
                && link_nodes(big_hyper,hyper->lbui,
			a.lbui,b.lbui,c.lbui,d.lbui,e.lbui,f.lbui,g.lbui)
                && link_nodes(big_hyper,hyper->rbui,
			a.rbui,b.rbui,c.rbui,d.rbui,e.rbui,f.rbui,g.rbui);

		flag = flag 
                && link_nodes(big_hyper,hyper->lfdo,
			a.lfdo,b.lfdo,c.lfdo,d.lfdo,e.lfdo,f.lfdo,g.lfdo)
                && link_nodes(big_hyper,hyper->rfdo,
			a.rfdo,b.rfdo,c.rfdo,d.rfdo,e.rfdo,f.rfdo,g.rfdo)
                && link_nodes(big_hyper,hyper->lbdo,
			a.lbdo,b.lbdo,c.lbdo,d.lbdo,e.lbdo,f.lbdo,g.lbdo)
                && link_nodes(big_hyper,hyper->rbdo,
			a.rbdo,b.rbdo,c.rbdo,d.rbdo,e.rbdo,f.rbdo,g.rbdo)
                && link_nodes(big_hyper,hyper->lfuo,
			a.lfuo,b.lfuo,c.lfuo,d.lfuo,e.lfuo,f.lfuo,g.lfuo)
                && link_nodes(big_hyper,hyper->rfuo,
			a.rfuo,b.rfuo,c.rfuo,d.rfuo,e.rfuo,f.rfuo,g.rfuo)
                && link_nodes(big_hyper,hyper->lbuo,
			a.lbuo,b.lbuo,c.lbuo,d.lbuo,e.lbuo,f.lbuo,g.lbuo)
                && link_nodes(big_hyper,hyper->rbuo,
			a.rbuo,b.rbuo,c.rbuo,d.rbuo,e.rbuo,f.rbuo,g.rbuo);

		free_hexbern4D(a);
		free_hexbern4D(b);
		free_hexbern4D(c);
		free_hexbern4D(d);
		free_hexbern4D(e);
		free_hexbern4D(f);
		free_hexbern4D(g);
		return(flag);
        }
}

calc_pos_norm(sol,vec,norm)
sol_info *sol;
double vec[4],norm[4];
{
	calc_pos(sol,vec);
	
	if(sol->dx == 0) norm[0] = 0.0;
	else		 norm[0] = evalbern4D(DD,vec);
	if(sol->dy == 0) norm[1] = 0.0;
	else		 norm[1] = evalbern4D(EE,vec);
	if(sol->dz == 0) norm[2] = 0.0;
	else		 norm[2] = evalbern4D(FF,vec);
	if(sol->dw == 0) norm[3] = 0.0;
	else		 norm[3] = evalbern4D(GG,vec);
}

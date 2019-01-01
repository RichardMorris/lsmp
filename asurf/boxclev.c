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
/*	sub-program to find the solution of a polynomial equ in 3D	*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../lsmp.h"
#include "bern.h"
#include "cells.h"
#include "topology.h"
#include "../CVcommon.h"

/******************* RMAR GLOBALS ***********************************/

#define  numberofbitsinmantissaofrealnumber 27 /* at least for the */
						/* purposes of drawing */
/*
#define PRINT_PYRIMID
#define PRINT_SOLVEEDGE
#define PLOT_AS_LINES
#define PRINT_REDUCE_EDGE
#define PRINT_FOLLOW
#define PRINT_DRAW_BOX
#define PRINT_SUSLINK
#define PRINT_PRE_COLLECT
#define TWO_PASS
#define CHECK_SECOND
#define PRINT_SECOND
#define PRINT_SOL_VAL_ERR
#define PRINT_LINK_FACE_ALL
#define PRINT_FIND_EDGE_SOL
#define PRINT_ODD_BOX
#define PRINT_GEN_BOXES
#define VERBOUSE
#define TEST_ALLOC
#define PRINT_LINK_FACE
#define PRINT_LINK_FACES_ALL
#define LITTLE_FACETS
#define PRINT_CONVERGE
#define USE_2ND_DERIV
#define PRINT_LINK_FACE_ALL
#define PRINT_LINKFACE04
#define PRINT_LINK_SING
#define USE_STURM
*/

#define TESTNODES
#define NON_GENERIC_NODES
#define NON_GENERIC_EDGE
#define DO_FREE
#define LINK_SING
#define LINK_SOLS
#define LINK_FACE
#define DO_PLOT
#define FACETS

#define MAX_EDGE_LEVEL 1048576 /* 32768 */
unsigned int LINK_FACE_LEVEL = 512, LINK_SING_LEVEL = 128, RESOLUTION = 8;
unsigned int SUPER_FINE = 8192;

#define EMPTY 0
#define FOUND_EVERYTHING 2
#define FOUND_FACES 3

/*** A value returned by follow when there a sol not on an edge is found. ***/

#define NEW_NODE 2

#define grballoc(node) ( node * ) malloc( sizeof(node) )

extern sol_info *get_nth_sol_on_face();
extern node_info *get_nth_node_on_box();
extern int global_mode;

box_info *whole_box;
bern3D *BB;		/* The main bernstein poly */
bern3D *CC,*DD,*EE;	/* The three derivative polys */
bern3D *Dxx,*Dxy,*Dxz,*Dyy,*Dyz,*Dzz;
region_info region;

sol_info **known_sings;	/* The singularities known from external data */
int num_known_sings;	/* number of such */

/*********************** Start of Sub-routines **************************/
/*									*/
/* draws an arbitrary polynomial curve in a specified region of space   */
/*									*/
/*********************** Start of Sub-routines **************************/

/** Forward defs **/

int find_box(box_info *box,bern3D *bb);

int generate_boxes(box_info *box,bern3D *bb);
void find_all_faces( box_info *box, bern3D *bb);
void find_face( box_info *box, bern3D *bb, face_info *face,int code,int internal);
void find_all_edges(box_info *box, face_info *face,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,int code);
void find_edge(edge_info *edge,bern2D *bb,bern2D *dx,
	bern2D *dy,bern2D *dz,int code);
void find_sols_on_edge(edge_info *edge,bern1D *bb,bern1D *dx,
	bern1D *dy,bern1D * dz);
void ReduceFace(face_info *big_face,face_info *face,bern2D *bb,bern2D *dx,
	bern2D *dy,bern2D *dz,bern2D *d2,int internal,int f1,int f2,int f3);
void link_face(face_info *big_face,face_info *face,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,int internal);
int link_nodes(box_info *box,bern3D *bb);
void calc_pos_norm(sol_info *sol,double vec[3],double norm[3]);
int converge_sing(box_info *box,sol_info *sol,int signDx,int signDy,int signDz);
int converge_sing2(box_info *box,sol_info *sol,bern3D *bb,bern3D *A,bern3D *B,bern3D *C);
int converge_node(sol_info *sol,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,
	int signDx,int signDy,int signDz);
void calc_pos_actual(sol_info *sol,double vec[3]);

void calc_known_sings(HPoint3 *pl,int num_known_sings);
int find_known_sing(sol_info *sol);

extern int check_interupt(char *string);
extern void make_facets(box_info *box);
extern void draw_box(box_info *box);
extern void clean_facets(box_info *box);

extern int	edgecount, edgemax, edgenew ,
	facecount, facemax, facenew ,
	linkcount, linkmax, linknew ,
	nodecount, nodemax, nodenew ,
	boxcount, boxmax, boxnew,
	solcount,  solmax, solnew,
	vertcount, vertmax, vertnew,
	node_linkcount, node_linkmax, node_linknew,
	singcount, singmax, singnew,
	chaincount, chainmax, chainnew,
	facet_solcount, facet_solmax, facet_solnew,
	facetcount, facetmax, facetnew;

extern int global_selx;			/* if >=0 then olny select boxes with this coord */
extern int global_sely;
extern int global_selz;
extern int global_denom;
extern int global_lf;			/* if set draw little facets */

/** Entry point **/

int marmain(
	double aa[MAXORDER][MAXORDER][MAXORDER],
	int xord, int yord, int zord,
	double xmin,double xmax,double ymin,double ymax,double zmin,double zmax,
	HPoint3 *pl,int num_pts)
{
  int flag;

  region.xmin = xmin;
  region.ymin = ymin;
  region.zmin = zmin;
  region.xmax = xmax;
  region.ymax = ymax;
  region.zmax = zmax;

  BB = formbernstein3D(aa,xmin,xmax,ymin,ymax,zmin,zmax);
  CC = diffx3D(BB);
  DD = diffy3D(BB);
  EE = diffz3D(BB);
  init_berns(BB,CC,DD,EE);
  Dxx = diffx3D(CC);
  Dxy = diffy3D(CC);
  Dxz = diffz3D(CC);

  Dyy = diffy3D(DD);
  Dyz = diffz3D(DD);

  Dzz = diffz3D(EE);

  init_cells();
  if(global_mode == MODE_KNOWN_SING)
	  calc_known_sings(pl,num_pts);
#ifdef VERBOUSE
  fprintf(stderr,"Initial polynomial\n");
  fprint_poly3(stderr,aa);
  fprintf(stderr,"range %f %f %f %f %f %f\n",
  	xmin,xmax,ymin,ymax,zmin,zmax);
  fprintf(stderr,"Bernstein polynomial is:\n");
  printbern3D(BB);
  fprintf(stderr,"double %d double * %d bern1D %d\n",
	sizeof(double),sizeof(double *),sizeof(bern1D));
#endif
 whole_box = (box_info *) malloc(sizeof(box_info));
  make_box(whole_box,0,0,0,1);
  flag = generate_boxes(whole_box,BB);
  free_box(whole_box);
  free_bern3D(BB);
  free_bern3D(CC);
  free_bern3D(DD);
  free_bern3D(EE);
  flag = flag && !check_interupt("Writing Data");
  fini_berns();
  fini_cells();
  return(flag);
}

/************************************************************************/
/*									*/
/*	The main routine for the first pass of the algorithim.		*/
/*	This recursivly creates a tree of boxes where each box contains */
/*	eight smaller boxes, only those boxes where there might be a	*/
/*	solution are considered (i.e. !allonesign ).			*/
/*	The recursion ends when the none of the derivatives have	*/
/*	solutions, and a set depth has been reach or when 		*/
/*	a greater depth has been reached.				*/
/*									*/
/************************************************************************/

int generate_boxes(box,bb)
box_info *box;
bern3D *bb;
{
	int xl,yl,zl,denom;
	double percent = 0.0;
	octbern3D temp;
	int flag;
	char string[40];

	xl = box->xl; yl = box->yl; zl = box->zl;
	for( denom = box->denom;denom>1;denom /= 2)
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
	}
	sprintf(string,"Done %6.2f percent.",percent*100.0);
	if( check_interupt( string ) ) return(FALSE);

	if(global_denom == box->denom)
	{
	if(global_selx != -1) if(box->xl != global_selx) return TRUE;
	if(global_sely != -1) if(box->yl != global_sely) return TRUE;
	if(global_selz != -1) if(box->zl != global_selz) return TRUE;
	}

   if( allonesign3D(bb) ) /* no componant in box */
   {
#ifdef PRINT_GEN_BOXES
   	fprintf(stderr,"generate_boxes: box (%d,%d,%d)/%d no conponant\n",
		box->xl,box->yl,box->zl,box->denom);
#endif
	box->status = EMPTY;
	return(TRUE);
   }

      /*** If all derivitives non zero and the region is sufficiently	***/
      /***  small then draw the surface.				***/

   if( box->denom >= RESOLUTION )
   {
#ifdef PRINT_GEN_BOXES
   	fprintf(stderr,"generate_boxes: box (%d,%d,%d)/%d LEAF\n",
		box->xl,box->yl,box->zl,box->denom);
#endif

	return(find_box(box,bb));
   }
   else
   {		/**** Sub-devide the region into 8 sub boxes.  ****/
#ifdef PRINT_GEN_BOXES
   	fprintf(stderr,"generate_boxes: box (%d,%d,%d)/%d NODE\n",
		box->xl,box->yl,box->zl,box->denom);
#endif

	temp = reduce3D(bb);
	sub_devide_box(box);
	flag = (generate_boxes(box->lfd,temp.lfd) &&
		generate_boxes(box->rfd,temp.rfd) &&
		generate_boxes(box->lbd,temp.lbd) &&
		generate_boxes(box->rbd,temp.rbd) &&
		generate_boxes(box->lfu,temp.lfu) &&
		generate_boxes(box->rfu,temp.rfu) &&
		generate_boxes(box->lbu,temp.lbu) &&
		generate_boxes(box->rbu,temp.rbu) );

	free_octbern3D(temp);
	return(flag);
   }
}

/*
 * Function:	find_box
 * action:	finds all solutions, nodes and singularities for a box
 *		together with the topoligical linkage information.
 */

int find_box(box,bb)
box_info *box;
bern3D *bb;
{
/*
printf("find_box (%d,%d,%d)/%d\n",box->xl,box->yl,box->zl,box->denom);
*/
	find_all_faces(box,bb);
	if( !link_nodes(box,bb) ) return(FALSE);
	box->status = FOUND_EVERYTHING ;

	if(!global_lf)
	{
		make_facets(box);
		draw_box(box);
	}
#ifdef DO_FREE
	free_bits_of_box(box);
#endif
	return(TRUE);
}

/*
 * Function:	find_all_faces
 * action:	for all the faces of the box find the information
 *		about the solutions and nodes.
 *		takes information already found about faces of adjacient
 *		boxes.
 */

void find_all_faces( box_info *box, bern3D *bb)
{
	get_existing_faces(box);
	create_new_faces(box);

	/* none of the faces are internal */

	find_face(box,bb,box->ll,FACE_LL,FALSE);
	find_face(box,bb,box->rr,FACE_RR,FALSE);
	find_face(box,bb,box->ff,FACE_FF,FALSE);
	find_face(box,bb,box->bb,FACE_BB,FALSE);
	find_face(box,bb,box->dd,FACE_DD,FALSE);
	find_face(box,bb,box->uu,FACE_UU,FALSE);
}

/*
 * Function:	find_face
 * action:	find all the information about solutions and nodes on face.
 */

void find_face( box_info *box, bern3D *bb, face_info *face,int code,int internal)
{
	bern2D *aa,*dx,*dy,*dz,*d2=NULL;
	bern3D *temp,*temp2;

	if(face->status == FOUND_EVERYTHING ) return;
	aa = make_bern2D_of_box(bb,code);
	if( allonesign2D(aa) )
	{
		face->status = FOUND_EVERYTHING;
		free_bern2D(aa);
		return;
	}
	if(face->type == FACE_LL || face->type == FACE_RR)
	{
		temp = diffx3D(bb);
		dx = make_bern2D_of_box(temp,code);
#ifdef USE_2ND_DERIV
		if(!allonesign2D(dx))
		{
			temp2 = diffx3D(temp);
			d2  = make_bern2D_of_box(temp2,code);
			free_bern3D(temp2);
		}
#endif
		free_bern3D(temp);
	}
	else
		dx = diffx2D(aa);

	if(face->type == FACE_FF || face->type == FACE_BB)
	{
		temp = diffy3D(bb);
		dy = make_bern2D_of_box(temp,code);
#ifdef USE_2ND_DERIV
		if(!allonesign2D(dx))
		{
			temp2 = diffy3D(temp);
			d2  = make_bern2D_of_box(temp2,code);
			free_bern3D(temp2);
		}
#endif
		free_bern3D(temp);
	}
	else if(face->type == FACE_LL || face->type == FACE_RR)
		dy = diffx2D(aa);
	else
		dy = diffy2D(aa);

	if(face->type == FACE_UU || face->type == FACE_DD)
	{
		temp = diffz3D(bb);
		dz = make_bern2D_of_box(temp,code);
#ifdef USE_2ND_DERIV
		if(!allonesign2D(dz))
		{
			temp2 = diffz3D(temp);
			d2  = make_bern2D_of_box(temp2,code);
			free_bern3D(temp2);
		}
#endif
		free_bern3D(temp);
	}
	else
		dz = diffy2D(aa);

	find_all_edges(box,face,aa,dx,dy,dz,code);

	link_face(face,face,aa,dx,dy,dz,d2,internal);
#ifdef FACETS
	colect_nodes(box,face);
#else
	if( !internal ) colect_nodes(box,face);
#endif
	face->status = FOUND_EVERYTHING;
	free_bern2D(dx);
	free_bern2D(dy);
	free_bern2D(dz);
	free_bern2D(aa);
}

/*
 * Function:	find_all_edges
 * action:	finds all the solutions on the edges of a face.
 *		uses the information already found from adjacient faces.
 */

void find_all_edges(box_info *box, face_info *face,bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,int code)
{
	get_existing_edges(box,face,code);
	create_new_edges(face);
	find_edge(face->x_low,bb,dx,dy,dz,X_LOW);
	find_edge(face->x_high,bb,dx,dy,dz,X_HIGH);
	find_edge(face->y_low,bb,dx,dy,dz,Y_LOW);
	find_edge(face->y_high,bb,dx,dy,dz,Y_HIGH);
}

/*
 * Function:	find_edge
 * action:	finds all the solutions on an edge.
 */

void find_edge(edge_info *edge,bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,int code)
{
	bern1D *aa,*dx1,*dy1,*dz1;

	if( edge->status == FOUND_EVERYTHING ) return;
	aa = make_bern1D_of_face(bb,code);
	dx1 = make_bern1D_of_face(dx,code);
	dy1 = make_bern1D_of_face(dy,code);
	dz1 = make_bern1D_of_face(dz,code);
	find_sols_on_edge(edge,aa,dx1,dy1,dz1);
	edge->status = FOUND_EVERYTHING;
	free_bern1D(aa);
	free_bern1D(dx1);
	free_bern1D(dy1);
	free_bern1D(dz1);
}

/*
 * Function:	find_sols_on_edge
 * action:	finds all the solutions on the edge
 */

void find_sols_on_edge(edge_info *edge,bern1D *bb,bern1D *dx,bern1D *dy,bern1D *dz)
{
	double vall,valm;
        double rootl,rooth,rootm,res;
        long level;
	int  f1,f2,f3;
	double vec[3];

	edge->status = FOUND_EVERYTHING;
        if( allonesign1D(bb) ) return;

	f1 = allonesign1D(dx);
	f2 = allonesign1D(dy);
	f3 = allonesign1D(dz);

        if( ( !f1 || !f2 || !f3 ) && edge->denom < SUPER_FINE )
        {
		binbern1D aa,dx1,dy1,dz1;

		aa = reduce1D(bb);
		if( f1 > 0 )      dx1 = reduce1D(posbern1D);
		else if( f1 < 0 ) dx1 = reduce1D(negbern1D);
		else if(edge->type == X_AXIS)
				  dx1 = binDiff1D(aa);
		else		  dx1 = reduce1D(dx);

		if( f2 > 0 )      dy1 = reduce1D(posbern1D);
		else if( f2 < 0 ) dy1 = reduce1D(negbern1D);
		else if(edge->type == Y_AXIS)
				  dy1 = binDiff1D(aa);
		else		  dy1 = reduce1D(dy);

		if( f3 > 0 )      dz1 = reduce1D(posbern1D);
		else if( f3 < 0 ) dz1 = reduce1D(negbern1D);
		else if(edge->type == Z_AXIS)
				  dz1 = binDiff1D(aa);
		else		  dz1 = reduce1D(dz);

		edge->left = grballoc(edge_info);
		edge->right = grballoc(edge_info);
#ifdef TEST_ALLOC
		++edgecount; ++edgemax; ++edgenew;
		++edgecount; ++edgemax; ++edgenew;
#endif
		subdevideedge(edge,edge->left,edge->right);

 		find_sols_on_edge(edge->left,aa.l,dx1.l,dy1.l,dz1.l);
		find_sols_on_edge(edge->right,aa.r,dx1.r,dy1.r,dz1.r);

		if( *(aa.r->array) == 0.0 )
		{
			edge->sol = make_sol(edge->type,edge->xl,
				edge->yl,edge->zl,edge->denom, 0.5 );
			edge->sol->dx = f1;
			edge->sol->dy = f2;
			edge->sol->dz = f3;
		}
		free_binbern1D(aa);
		free_binbern1D(dx1);
		free_binbern1D(dy1);
		free_binbern1D(dz1);
        	return;
	}

	/*** Either a simple interval or at bottom of tree ***/

	if( ( f1 && f2 && f3 ) || allonesignderiv1D(bb) )
	{
		/*** A simple interval ***/
        	vall = *(bb->array);
        	rootl = 0.0;
		rootm = 0.5;
        	rooth = 1.0;
		level = (long) edge->denom;
		while( level <= MAX_EDGE_LEVEL )
        	{
			level *= 2;
                	rootm = (rootl +rooth) * 0.5;
                	valm = evalbern1D(bb, rootm);
                	if((vall<0) != (valm<0)) rooth = rootm;
                	else
                	{
                	        vall = valm; 
                        	rootl = rootm;
                	}
        	}
	}
	else
	{
#ifdef USE_STURM
		extern int calc_sterm_root(bern1D *bb,double roots[MAXORDER]);

		double roots[MAXORDER];
		int num,i,calcnum;

		fprintf(stderr,"BAD EDGE: ");
		print_edge(edge);
		printbern1D_normal(bb);
		fprintf(stderr,"\n");

		num = calc_sterm_root(bb,roots);
		calcnum = 0;
		if(num==0) return;
		fprintf(stderr,"ROOTS: ");
		for(i=0;i<num;++i)
		{
			fprintf(stderr,"%g ",roots[i]);
			fprintf(stderr,"\n");
			if(roots[i]>=0.0 && roots[i]<=1.0) ++calcnum;
		}		
		
		if(calcnum>2)
		{
			fprintf(stderr,"More than 2 sturm sols on the edge %d\n",num);
			return;
		}
		if(calcnum==2)
		{
			edge->left = grballoc(edge_info);
			edge->right = grballoc(edge_info);
			make_edge(edge->left,edge->type,edge->xl,edge->yl,edge->zl,edge->denom);
			make_edge(edge->right,edge->type,edge->xl,edge->yl,edge->zl,edge->denom);
		}
	
		for(i=0;i<num;++i)
		{
			sol_info *sol;
			if(roots[i]<0.0 || roots[i]>1.0) continue;

			sol = make_sol(edge->type,edge->xl,edge->yl,edge->zl,
				edge->denom, roots[i] );
			sol->dx = f1;
			sol->dy = f2;
			sol->dz = f3;
			if( !f1 )
			{
				res = evalbern3D(CC,vec);
				if( res < 0 ) sol->dx = -1;
				if( res > 0 ) sol->dx = 1;
			}
			if( !f2 )
			{
				res = evalbern3D(DD,vec);
				if( res < 0 ) sol->dy = -1;
				if( res > 0 ) sol->dy = 1;
			}
			if( !f3 )
			{
				res = evalbern3D(EE,vec);
				if( res < 0 ) sol->dz = -1;
				if( res > 0 ) sol->dz = 1;
			}
			if(calcnum==1) edge->sol = sol;
			else if(i==0) edge->left->sol = sol;
			else if(i==1) edge->right->sol = sol;
		}
		return;
#endif
#ifdef NON_GENERIC_EDGE
		if( *(bb->array) * *(bb->array+bb->ord) > 0 ) return;
#endif
		rootm = BAD_EDGE;
	}


	edge->sol = make_sol(edge->type,edge->xl,edge->yl,edge->zl,
		edge->denom, rootm );
	edge->sol->dx = f1;
	edge->sol->dy = f2;
	edge->sol->dz = f3;

	if( !f1 || !f2 || !f3 )
	{
		/*** Can't work out derivatives easily ***/
		/*** use actual values ***/

		calc_pos(edge->sol,vec);
		if( !f1 )
		{
			res = evalbern3D(CC,vec);
			if( res < 0 ) edge->sol->dx = -1;
			if( res > 0 ) edge->sol->dx = 1;
		}
		if( !f2 )
		{
			res = evalbern3D(DD,vec);
			if( res < 0 ) edge->sol->dy = -1;
			if( res > 0 ) edge->sol->dy = 1;
		}
		if( !f3 )
		{
			res = evalbern3D(EE,vec);
			if( res < 0 ) edge->sol->dz = -1;
			if( res > 0 ) edge->sol->dz = 1;
		}

#ifdef PRINT_FIND_EDGE_SOL
		fprintf(stderr,"find_sol_on_edge: f1 %d f2 %d f3 %d\n",f1,f2,f3);
		print_sol(edge->sol);
#endif
	}
        return;
}

/*
 * Function:	link_face
 * action:	links together the solutions which lie around a face
 *		and add to list of links, also add nodes (where one
 *		derivitive vanishes) to the list.
 *		There are two modes of operation:
 *		if internal is TRUE it indicates that the face is internal
 *		to a box, in which case we are just interested in
 *		the nodes and not the links.
 */

/* The signs of the derivatives of the nodes must exactly match the string */

int TestSignsPerform(char *actualStr, char *testStr,int len,int width)
{
	return(!strcmp(actualStr,testStr));
}

int TestSignNum = 0;

int TestSignsReorder1(char *actualStr,char *testStr,int len,int width,int order[])
{
	char reorderStr[80];
	int i,j;

	strcpy(reorderStr,testStr);
	for(i=0;i<len;++i)
		for(j=0;j<width;++j)
			reorderStr[i*(width+1)+j] = actualStr[order[i]*(width+1)+j];
	return(!strcmp(reorderStr,testStr));
}

int TestSignsReorder2(char *actualStr,char *testStr,int len,int width,int order[])
{
	char reorderStr[80];
	int i,j;

	strcpy(reorderStr,testStr);
	for(i=0;i<len;++i)
		for(j=0;j<width;++j)
			reorderStr[i*(width+1)+j] = actualStr[order[i]*(width+1)+j];
	return(!strcmp(reorderStr,testStr));
}

int TestSignsReorder3(char *actualStr,char *testStr,int len,int width,int order[])
{
	char reorderStr[80];
	int i,j;

	strcpy(reorderStr,testStr);
	for(i=0;i<len;++i)
		for(j=0;j<width;++j)
			reorderStr[i*(width+1)+j] = actualStr[order[i]*(width+1)+j];
	return(!strcmp(reorderStr,testStr));
}

int TestSignsReorder4(char *actualStr,char *testStr,int len,int width,int order[])
{
	char reorderStr[80];
	int i,j;

	strcpy(reorderStr,testStr);
	for(i=0;i<len;++i)
		for(j=0;j<width;++j)
			reorderStr[i*(width+1)+j] = actualStr[order[i]*(width+1)+j];
	return(!strcmp(reorderStr,testStr));
}

int TestSignsReorder5(char *actualStr,char *testStr,int len,int width,int order[])
{
	char reorderStr[80];
	int i,j;

	strcpy(reorderStr,testStr);
	for(i=0;i<len;++i)
		for(j=0;j<width;++j)
			reorderStr[i*(width+1)+j] = actualStr[order[i]*(width+1)+j];
	return(!strcmp(reorderStr,testStr));
}

int TestSignsReorder6(char *actualStr,char *testStr,int len,int width,int order[])
{
	char reorderStr[80];
	int i,j;

	strcpy(reorderStr,testStr);
	for(i=0;i<len;++i)
		for(j=0;j<width;++j)
			reorderStr[i*(width+1)+j] = actualStr[order[i]*(width+1)+j];
	return(!strcmp(reorderStr,testStr));
}

int TestSignsReorder7(char *actualStr,char *testStr,int len,int width,int order[])
{
	char reorderStr[80];
	int i,j;

	strcpy(reorderStr,testStr);
	for(i=0;i<len;++i)
		for(j=0;j<width;++j)
			reorderStr[i*(width+1)+j] = actualStr[order[i]*(width+1)+j];
	return(!strcmp(reorderStr,testStr));
}

int TestSignsReorder(char *actualStr,char *testStr,int len,int width,int order[])
{
	switch(TestSignNum)
	{
	case 1: return TestSignsReorder1(actualStr,testStr,len,width,order);
	case 2: return TestSignsReorder2(actualStr,testStr,len,width,order);
	case 3: return TestSignsReorder3(actualStr,testStr,len,width,order);
	case 4: return TestSignsReorder4(actualStr,testStr,len,width,order);
	case 5: return TestSignsReorder5(actualStr,testStr,len,width,order);
	case 6: return TestSignsReorder6(actualStr,testStr,len,width,order);
	case 7: return TestSignsReorder7(actualStr,testStr,len,width,order);
	default: return TestSignsReorder7(actualStr,testStr,len,width,order);
	}
	return 0;
}

/* Any the nodes in any order must match */
/* Last parameter is array with the order in */

int TestSignsCycle(char *actualStr,int count,char *testStr,int len,int width,int order[])
{
	int i1,i2,i3,i4,i5,i6,i7,i8;
	
	switch(len)
	{
	case 1:
		for(i1=0;i1<count;++i1)
		{
			order[0] = i1;
			if(TestSignsReorder(actualStr,testStr,len,width,order))
				return TRUE;
		}
		return FALSE;
	case 2:
		for(i1=0;i1<count;++i1)
		for(i2=0;i2<count;++i2)
		{
			if(i1 == i2) continue;
			order[0] = i1;
			order[1] = i2;
			if(TestSignsReorder(actualStr,testStr,len,width,order))
				return TRUE;
		}
		return FALSE;
	case 3:
		for(i1=0;i1<count;++i1)
		for(i2=0;i2<count;++i2)
		{ if(i1 == i2) continue;
		  for(i3=0;i3<count;++i3)
		  {
			if(i3 == i1 || i3 == i2) continue;
			order[0] = i1;
			order[1] = i2;
			order[2] = i3;
			if(TestSignsReorder(actualStr,testStr,len,width,order))
				return TRUE;
		  }
		}
		return FALSE;
	case 4:
		for(i1=0;i1<count;++i1)
		for(i2=0;i2<count;++i2)
		{ if(i1 == i2) continue;
		  for(i3=0;i3<count;++i3)
		  { if(i3 == i1 || i3 == i2) continue;
		    for(i4=0;i4<count;++i4)
		    { if(i4 == i1 || i4 == i2 || i4 == i3) continue;
			order[0] = i1;
			order[1] = i2;
			order[2] = i3;
			order[3] = i4;
			if(TestSignsReorder(actualStr,testStr,len,width,order))
				return TRUE;
		    }
		  }
		}
		return FALSE;
	case 5:
		for(i1=0;i1<count;++i1)
		for(i2=0;i2<count;++i2)
		{ if(i1 == i2) continue;
		  for(i3=0;i3<count;++i3)
		  { if(i3 == i1 || i3 == i2) continue;
		    for(i4=0;i4<count;++i4)
		    { if(i4 == i1 || i4 == i2 || i4 == i3) continue;
		      for(i5=0;i5<count;++i5)
		      { if(i5 == i1 || i5 == i2 || i5 == i3 || i5 == i4) continue;
			order[0] = i1;
			order[1] = i2;
			order[2] = i3;
			order[3] = i4;
			order[4] = i5;
			if(TestSignsReorder(actualStr,testStr,len,width,order))
				return TRUE;
		      }
		    }
		  }
		}
		return FALSE;
	case 6:
		for(i1=0;i1<count;++i1)
		for(i2=0;i2<count;++i2)
		{ if(i1 == i2) continue;
		  for(i3=0;i3<count;++i3)
		  { if(i3 == i1 || i3 == i2) continue;
		    for(i4=0;i4<count;++i4)
		    { if(i4 == i1 || i4 == i2 || i4 == i3) continue;
		      for(i5=0;i5<count;++i5)
		      { if(i5 == i1 || i5 == i2 || i5 == i3 || i5 == i4) continue;
		        for(i6=0;i6<count;++i6)
		        { if(i6 == i1 || i6 == i2 || i6 == i3 || i6 == i4 || i6 == i5) continue;
				order[0] = i1;
				order[1] = i2;
				order[2] = i3;
				order[3] = i4;
				order[4] = i5;
				order[5] = i6;
				if(TestSignsReorder(actualStr,testStr,len,width,order))
					return TRUE;
			}
		      }
		    }
		  }
		}
		return FALSE;
	case 7:
		for(i1=0;i1<count;++i1)
		for(i2=0;i2<count;++i2)
		{ if(i1 == i2) continue;
		  for(i3=0;i3<count;++i3)
		  { if(i3 == i1 || i3 == i2) continue;
		    for(i4=0;i4<count;++i4)
		    { if(i4 == i1 || i4 == i2 || i4 == i3) continue;
		      for(i5=0;i5<count;++i5)
		      { if(i5 == i1 || i5 == i2 || i5 == i3 || i5 == i4) continue;
		        for(i6=0;i6<count;++i6)
		        { if(i6 == i1 || i6 == i2 || i6 == i3 || i6 == i4 || i6 == i5) continue;
		          for(i7=0;i7<count;++i7)
		          { if(i7 == i1 || i7 == i2 || i7 == i3 || i7 == i4 
				|| i7 == i5 || i7 == i6) continue;
				order[0] = i1;
				order[1] = i2;
				order[2] = i3;
				order[3] = i4;
				order[4] = i5;
				order[5] = i6;
				order[6] = i7;
				if(TestSignsReorder(actualStr,testStr,len,width,order))
					return TRUE;
			  }
			}
		      }
		    }
		  }
		}
		return FALSE;
	case 8:
		for(i1=0;i1<count;++i1)
		for(i2=0;i2<count;++i2)
		{ if(i1 == i2) continue;
		  for(i3=0;i3<count;++i3)
		  { if(i3 == i1 || i3 == i2) continue;
		    for(i4=0;i4<count;++i4)
		    { if(i4 == i1 || i4 == i2 || i4 == i3) continue;
		      for(i5=0;i5<count;++i5)
		      { if(i5 == i1 || i5 == i2 || i5 == i3 || i5 == i4) continue;
		        for(i6=0;i6<count;++i6)
		        { if(i6 == i1 || i6 == i2 || i6 == i3 || i6 == i4 || i6 == i5) continue;
		          for(i7=0;i7<count;++i7)
		          { if(i7 == i1 || i7 == i2 || i7 == i3 || i7 == i4 
				|| i7 == i5 || i7 == i6) continue;
		            for(i8=0;i8<count;++i8)
		            { if(i8 == i1 || i8 == i2 || i8 == i3 || i8 == i4 
				|| i8 == i5 || i8 == i6 || i8 == i7) continue;
				order[0] = i1;
				order[1] = i2;
				order[2] = i3;
				order[3] = i4;
				order[4] = i5;
				order[5] = i6;
				order[6] = i7;
				order[7] = i8;
				if(TestSignsReorder(actualStr,testStr,len,width,order))
					return TRUE;
			    }
			  }
			}
		      }
		    }
		  }
		}
		return FALSE;
	default:
		fprintf(stderr,"TestNodesCycle: Bad length %d\n",len);
	}
	return FALSE;
}

/* Given a string "++0|+-0|+0+|+0-" specifying signs of derivatives 
   and a string   "+++|-++"	    specifying how sign should be changed
   and a string   "abc|bca|cab"	    specifying how coords can be rotated

   Test all combinations to see if any match. Return true if so.
*/
	
int TestSigns(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	int i,j,k;
	int len,signLen,rotLen;
	char signTest[80], rotTest[80];
	len = (strlen(testStr)+1)/(width+1);
	signLen = (strlen(signStr)+1)/(width+1);
	rotLen  = (strlen(rotStr)+1)/(width+1);
	if(len>8)
	{
		fprintf(stderr,"TestNodes: len to big %d (%s)\n",len,testStr);
		return FALSE;
	}
	if(count < len) return FALSE;

	strcpy(rotTest,testStr); /* just to get the right size */

	for(i=0;i<signLen;++i)
	{
		strcpy(signTest,testStr);
		for(j=0;j<width;++j)
			if(signStr[i*(width+1)+j] == '-')
				for(k=0;k<len;++k)
				{
					if(     testStr[k*(width+1)+j] == '+')
						signTest[k*(width+1)+j] = '-';
					else if(testStr[k*(width+1)+j] == '-')
						signTest[k*(width+1)+j] = '+';
				}

		/* fixed signs, now fix rotation */

		for(j=0;j<rotLen;++j)
		{
			int offset;
			for(k=0;k<width;++k)
			{
				int l;
				offset = rotStr[j*(width+1)+k] - 'a';
				for(l=0;l<len;++l)
					rotTest[l*(width+1)+k] = signTest[l*(width+1)+offset];
			}

			if(TestSignsCycle(actualStr,count,rotTest,len,width,order))
				return TRUE;
		}
	}
	return FALSE;
}

int TestSigns1(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	TestSignNum = 1;
	return TestSigns(actualStr,count,width,testStr,signStr,rotStr,order);
}
int TestSigns2(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	TestSignNum = 2;
	return TestSigns(actualStr,count,width,testStr,signStr,rotStr,order);
}
int TestSigns3(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	TestSignNum = 3;
	return TestSigns(actualStr,count,width,testStr,signStr,rotStr,order);
}
int TestSigns4(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	TestSignNum = 4;
	return TestSigns(actualStr,count,width,testStr,signStr,rotStr,order);
}
int TestSigns5(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	TestSignNum = 5;
	return TestSigns(actualStr,count,width,testStr,signStr,rotStr,order);
}
int TestSigns6(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	TestSignNum = 6;
	return TestSigns(actualStr,count,width,testStr,signStr,rotStr,order);
}
int TestSigns7(char *actualStr,int count,int width,char *testStr,char *signStr,char *rotStr,int order[])
{
	TestSignNum = 7;
	return TestSigns(actualStr,count,width,testStr,signStr,rotStr,order);
}

int Test4nodesLike011(node_info *nodes[],int count,int order[])
{
	int i,j,num_match;

	for(i=0;i<count;++i)
	{
		if( (nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 )
		 || (nodes[i]->sol->dx == 0 && nodes[i]->sol->dz == 0 )
		 || (nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0 ) )
			continue;
		if( nodes[i]->sol->dx != 0 && nodes[i]->sol->dy != 0 && nodes[i]->sol->dz != 0)
			continue;
		num_match = 1;
		order[0] = i;
		for(j=i+1;j<count;++j)
		{
			if( nodes[j]->sol->dx == nodes[i]->sol->dx
			 && nodes[j]->sol->dy == nodes[i]->sol->dy
			 && nodes[j]->sol->dz == nodes[i]->sol->dz )
			{
				order[num_match++] = j;
				if(num_match == 4) return 1;
			}
		}
	}
	return 0;
}

void BuildNodeSigns(node_info *bn[],int count,char *testStr)
{
	int i;

	if(count>20)
	{
		fprintf(stderr,"BuildNodeSigns: Error count too high %d max 20\n",count);
		exit(0);
	}
	for(i=0;i<count;++i)
	{
		if(     bn[i]->sol->dx >  0) testStr[i*4+0] = '+';
		else if(bn[i]->sol->dx == 0) testStr[i*4+0] = '0';
		else if(bn[i]->sol->dx <  0) testStr[i*4+0] = '-';

		if(     bn[i]->sol->dy >  0) testStr[i*4+1] = '+';
		else if(bn[i]->sol->dy == 0) testStr[i*4+1] = '0';
		else if(bn[i]->sol->dy <  0) testStr[i*4+1] = '-';

		if(     bn[i]->sol->dz >  0) testStr[i*4+2] = '+';
		else if(bn[i]->sol->dz == 0) testStr[i*4+2] = '0';
		else if(bn[i]->sol->dz <  0) testStr[i*4+2] = '-';
		testStr[i*4+3] = '|';
	}
	testStr[count*4] = '\0';
}

char *BuildNodeSigns2(node_info *bn[],int count)
{
	int i;
	char *testStr;

	testStr = (char *) malloc(sizeof(char)*count*4);
	for(i=0;i<count;++i)
	{
		if(     bn[i]->sol->dx >  0) testStr[i*4+0] = '+';
		else if(bn[i]->sol->dx == 0) testStr[i*4+0] = '0';
		else if(bn[i]->sol->dx <  0) testStr[i*4+0] = '-';

		if(     bn[i]->sol->dy >  0) testStr[i*4+1] = '+';
		else if(bn[i]->sol->dy == 0) testStr[i*4+1] = '0';
		else if(bn[i]->sol->dy <  0) testStr[i*4+1] = '-';

		if(     bn[i]->sol->dz >  0) testStr[i*4+2] = '+';
		else if(bn[i]->sol->dz == 0) testStr[i*4+2] = '0';
		else if(bn[i]->sol->dz <  0) testStr[i*4+2] = '-';
		testStr[i*4+3] = '|';
	}
	testStr[count*4-1] = '\0';
	return testStr;
}

void BuildSolSigns(sol_info *bn[],int count,char *testStr)
{
	int i;

	if(count>20)
	{
		fprintf(stderr,"BuildNodeSigns: Error count too high %d max 20\n",count);
		exit(0);
	}

	for(i=0;i<count;++i)
	{
		if(     bn[i]->dx >  0) testStr[i*4+0] = '+';
		else if(bn[i]->dx == 0) testStr[i*4+0] = '0';
		else if(bn[i]->dx <  0) testStr[i*4+0] = '-';

		if(     bn[i]->dy >  0) testStr[i*4+1] = '+';
		else if(bn[i]->dy == 0) testStr[i*4+1] = '0';
		else if(bn[i]->dy <  0) testStr[i*4+1] = '-';

		if(     bn[i]->dz >  0) testStr[i*4+2] = '+';
		else if(bn[i]->dz == 0) testStr[i*4+2] = '0';
		else if(bn[i]->dz <  0) testStr[i*4+2] = '-';
		testStr[i*4+3] = '|';
	}
	testStr[count*4] = '\0';
}

/****		First some macros	****/

#define PairTest(a,b) (\
	   sols[a]->dx == sols[b]->dx \
	&& sols[a]->dy == sols[b]->dy \
	&& sols[a]->dz == sols[b]->dz )

#define MatchDeriv(a) (\
	   sols[a]->dx == f1 \
	&& sols[a]->dy == f2 \
	&& sols[a]->dz == f3 )

/* Dont want these
	   || ( sols[a]->dx == 0 && sols[b]->dx == 0 ) \
	   || ( sols[a]->dy == 0 && sols[b]->dy == 0 ) \
	   || ( sols[a]->dz == 0 && sols[b]->dz == 0 ) \
*/
#define StraddleDeriv(a,b) (\
	   (  f1 \
	   || ( sols[a]->dx == 1 && sols[b]->dx == -1 ) \
	   || ( sols[a]->dx == -1 && sols[b]->dx == 1 ) ) \
	&& (  f2 \
	   || ( sols[a]->dy == 1 && sols[b]->dy == -1 ) \
	   || ( sols[a]->dy == -1 && sols[b]->dy == 1 ) ) \
	&& (  f3 \
	   || ( sols[a]->dz == 1 && sols[b]->dz == -1 ) \
	   || ( sols[a]->dz == -1 && sols[b]->dz == 1 ) ) )

#define SusFace(q_face,q_count,q_sols,dq) { \
    if(q_face == NULL) \
    { \
	q_sols[0] = NULL; \
	q_sols[1] = NULL; \
	q_face = alloc_face(); \
	make_face(q_face,face->type,face->xl,face->yl,face->zl, \
	  	face->denom); \
	create_new_edges(q_face); \
	find_edge(q_face->x_low,dq,dx,dy,dz,X_LOW); \
	find_edge(q_face->x_high,dq,dx,dy,dz,X_HIGH); \
	find_edge(q_face->y_low,dq,dx,dy,dz,Y_LOW);  \
	find_edge(q_face->y_high,dq,dx,dy,dz,Y_HIGH); \
	q_count = get_sols_on_face(q_face,q_sols); \
    } }

#define CalcCross(a_sols,b_sols) {\
	calc_pos_on_face(face,a_sols[0],vec0); \
	calc_pos_on_face(face,a_sols[1],vec1); \
	calc_pos_on_face(face,b_sols[0],vec2); \
	calc_pos_on_face(face,b_sols[1],vec3); \
	 \
	lam = -( (vec3[1]-vec2[1])*(vec0[0]-vec2[0]) \
	-(vec3[0]-vec2[0])*(vec0[1]-vec2[1]) ) \
	/((vec3[1]-vec2[1]) * (vec1[0] - vec0[0]) \
	-(vec3[0]-vec2[0]) * (vec1[1]-vec0[1])); \
 \
	if( lam != lam ) \
	{ \
	} \
	else if( lam >= 0.0 && lam <= 1.0 ) \
	{ \
		pos_x = lam * vec1[0] + (1.0-lam)*vec0[0]; \
		pos_y = lam * vec1[1] + (1.0-lam)*vec0[1]; \
		DerivFlag = FALSE; \
	} }
/*

*/

#define DerivTest(a_face,a_count,a_sols,da,fa,b_face,b_count,b_sols,db,fb) {\
	if( DerivFlag && !fa && !fb \
	   && ( da->xord != 0 || da->yord != 0 ) \
	   && ( db->xord != 0 || db->yord != 0 ) ) \
	{ \
		SusFace(a_face,a_count,a_sols,da); \
		if( a_count == 2 ) \
		{ \
			SusFace(b_face,b_count,b_sols,db); \
			if( b_count == 2 ) \
		  	{	CalcCross(a_sols,b_sols); } \
			else if( b_count != 0 ) DerivFlag = FALSE; \
		} \
		else if( a_count != 0 ) DerivFlag = FALSE; \
	} }

#define AddLink(a,b) {\
		include_link(face,sols[a],sols[b]); }

		bern2D *dxx,*dxy,*dxz, *dyy,*dyz,*dzz;

void calc_2nd_derivs(sol_info *sol,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2)
{
	bern2D *dxx,*dxy,*dxz,*dyy,*dyz,*dzz;

fprintf(stderr,"Calc 2nd derivs\n");
	if(sol->type == FACE_LL || sol->type == FACE_RR)
	{ /* s=y, t = z */
		dxx = d2;
		dxy = diffx2D(dx); /* dyx */
		dxz = diffy2D(dx); /* dzx */
		dyy = diffx2D(dy);
		dyz = diffy2D(dy); /* dzy */
		dzz = diffy2D(dz);
	}
	else if(sol->type == FACE_FF || sol->type == FACE_BB)
	{ /* s=x, t = z */
		dxx = diffx2D(dx);
		dxy = diffx2D(dy); 
		dxz = diffx2D(dz); /* dz dx */
		dyy = d2;
		dyz = diffy2D(dy); /* dz dy */
		dzz = diffy2D(dz);
	}
	else if(sol->type == FACE_UU || sol->type == FACE_DD)
	{
		dxx = diffx2D(dx);
		dxy = diffx2D(dy); /* dydx */
		dxz = diffx2D(dz); /* dzdx */
		dyy = diffy2D(dy);
		dyz = diffy2D(dz); /* dzdy */
		dzz = d2;
	}
	sol->dxx = allonesign2D(dxx);
	sol->dxy = allonesign2D(dxy);
	sol->dxz = allonesign2D(dxz);
	sol->dyy = allonesign2D(dyy);
	sol->dyz = allonesign2D(dyz);
	sol->dzz = allonesign2D(dzz);

	free_bern2D(dxy);
	free_bern2D(dxz);
	free_bern2D(dyz);
	if(sol->type != FACE_LL && sol->type != FACE_RR)
		free_bern2D(dxx);
	if(sol->type != FACE_FF && sol->type != FACE_BB)
		free_bern2D(dyy);
	if(sol->type != FACE_DD && sol->type != FACE_UU)
		free_bern2D(dzz);
}

sol_info *MakeNode(face_info *face,double pos_x,double pos_y,int f1,int f2,int f3,
	bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2)
{
	sol_info *temp;

	temp = make_sol2(face->type,face->xl,face->yl,face->zl,face->denom,pos_x,pos_y);
	temp->dx = f1;
	temp->dy = f2;
	temp->dz = f3; 
#ifdef USE_2ND_DERIV
		calc_2nd_derivs(temp,dx,dy,dz,d2);
#endif
	return(temp);
}

#define GetMid { \
	pos_x = pos_y = 0.0; \
	for( i=1; i <= count; ++i ) \
	{ \
		sols[4] = get_nth_sol_on_face(face,i); \
		calc_pos_on_face(face,sols[4],vec); \
		pos_x += vec[0]; \
		pos_y += vec[1]; \
	} \
	if( count == 0 ) \
	{	pos_x = pos_y = 0.5; } \
	else \
	{	pos_x /= count; \
		pos_y /= count; \
	} }

void combine_links(face_info *face)
{
	link_info *l1;
	face->links = NULL;

	for(l1 = face->lb->links;l1!=NULL;l1=l1->next)
		include_link(face,l1->A,l1->B);
	for(l1 = face->lt->links;l1!=NULL;l1=l1->next)
		include_link(face,l1->A,l1->B);
	for(l1 = face->rb->links;l1!=NULL;l1=l1->next)
		include_link(face,l1->A,l1->B);
	for(l1 = face->rt->links;l1!=NULL;l1=l1->next)
		include_link(face,l1->A,l1->B);

/*
	if( 512 * face->yl == 264 * face->denom)
	{
		fprintf(stderr,"combine_links: ");
		print_face(face);
	}
*/
}

void ReduceFace(face_info *big_face,face_info *face,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,
	int internal,int f1,int f2,int f3)
{
	quadbern2D b1;
	quadbern2D dx1;
	quadbern2D dy1;
	quadbern2D dz1;
	quadbern2D dd2;

	face->lb = alloc_face();
	face->rb = alloc_face();
	face->lt = alloc_face();
	face->rt = alloc_face();

	b1 = reduce2D(bb);
#ifdef USE_2ND_DERIV
	dd2 = reduce2D(d2);
#endif
	if( f1 > 0 )      dx1 = reduce2D(posbern2D);
	else if( f1 < 0 ) dx1 = reduce2D(negbern2D);
	else if(face->type == FACE_LL || face->type == FACE_RR)
			  dx1 = reduce2D(dx);
	else		  dx1 = quadDiff2Dx(b1);

	if( f2 > 0 )      dy1 = reduce2D(posbern2D);
	else if( f2 < 0 ) dy1 = reduce2D(negbern2D);
	else if(face->type == FACE_FF || face->type == FACE_BB)
			  dy1 = reduce2D(dy);
	else if(face->type == FACE_LL || face->type == FACE_RR)
			  dy1 = quadDiff2Dx(b1);
	else		  dy1 = quadDiff2Dy(b1);

	if( f3 > 0 )      dz1 = reduce2D(posbern2D);
	else if( f3 < 0 ) dz1 = reduce2D(negbern2D);
	else if(face->type == FACE_UU || face->type == FACE_DD)
			  dz1 = reduce2D(dz);
	else		  dz1 = quadDiff2Dy(b1);

	make_sub_faces(face,face->lb,face->rb,face->lt,face->rt);
	split_face(face,face->lb,face->rb,face->lt,face->rt);

	find_edge(face->lb->x_high,b1.lb,dx1.lb,dy1.lb,dz1.lb,X_HIGH);
	find_edge(face->lb->y_high,b1.lb,dx1.lb,dy1.lb,dz1.lb,Y_HIGH);
	find_edge(face->rt->x_low,b1.rt,dx1.rt,dy1.rt,dz1.rt,X_LOW);
	find_edge(face->rt->y_low,b1.rt,dx1.rt,dy1.rt,dz1.rt,Y_LOW);

	link_face(big_face,face->lb,b1.lb,dx1.lb,dy1.lb,dz1.lb,dd2.lb,internal);
	face->lb->status = FOUND_EVERYTHING;
	link_face(big_face,face->rb,b1.rb,dx1.rb,dy1.rb,dz1.rb,dd2.rb,internal);
	face->rb->status = FOUND_EVERYTHING;
	link_face(big_face,face->lt,b1.lt,dx1.lt,dy1.lt,dz1.lt,dd2.lt,internal);
	face->lt->status = FOUND_EVERYTHING;
	link_face(big_face,face->rt,b1.rt,dx1.rt,dy1.rt,dz1.rt,dd2.rt,internal);
	face->rt->status = FOUND_EVERYTHING;

#ifdef FACETS
	/* Now need to combine links from sub face to big face */
	combine_links(face);
#endif
	free_quadbern2D(b1);
	free_quadbern2D(dx1);
	free_quadbern2D(dy1);
	free_quadbern2D(dz1);
}

void link_face0sols(face_info *face,sol_info **sols,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,
	int f1,int f2,int f3)
{
	face_info *x_face=NULL,*y_face=NULL,*z_face=NULL;
	sol_info *x_sols[2],*y_sols[2],*z_sols[2];
	int DerivFlag, x_count=0, y_count=0, z_count=0;
	double vec0[2],vec1[2],vec2[2],vec3[2],pos_x=0.0,pos_y=0.0,lam;
	int flag,sign;
	bern2D *det,*dxx,*dxy,*dyy;

#ifdef PRINT_LINKFACE04
fprintf(stderr,"link0: %d %d %d\n",f1,f2,f3);
print_face(face);
#endif
	sols[3] = sols[4] = NULL;

	switch(face->type)
	{
	case FACE_LL: case FACE_RR:
		if(f2 || f3) return;
		dxx = diffx2D(dy);
		dxy = diffy2D(dy);
		dyy = diffy2D(dz);
		break;
	case FACE_FF: case FACE_BB:
		if(f1 || f3) return;
		dxx = diffx2D(dx);
		dxy = diffy2D(dx);
		dyy = diffy2D(dz);
		break;
	case FACE_UU: case FACE_DD:
		if(f1 || f2) return;
		dxx = diffx2D(dx);
		dxy = diffy2D(dx);
		dyy = diffy2D(dy);
		break;
	default:
		return;
	}
	det = symetricDet2D(dxx,dxy,dyy);
	if(det == NULL)
	{
		fprintf(stderr,"Null det\n");
		fprintf(stderr,"link_face0sols: %d %d %d\n",f1,f2,f3);
		print_face(face);
		printbern2D(dx);
		printbern2D(dy);
		printbern2D(dz);
		printbern2D(dxx);
		printbern2D(dxy);
		printbern2D(dyy);
		sign = 0;
	}
	else
		sign = allonesign2D(det);
	if(sign<0) return;

	DerivFlag = TRUE;
	DerivTest(x_face,x_count,x_sols,dx,f1, y_face,y_count,y_sols,dy,f2);
	DerivTest(x_face,x_count,x_sols,dx,f1, z_face,z_count,z_sols,dz,f3);
	DerivTest(y_face,y_count,y_sols,dy,f2, z_face,z_count,z_sols,dz,f3);

	if( DerivFlag ) 
	{
#ifdef PRINT_LINKFACE04
		fprintf(stderr,"DerivFlag %d\n",DerivFlag);
#endif
		return;
	}
/*
	if( pos_x != pos_x || pos_y != pos_y )
		fprintf(stderr,"pos_x %f pos_y %f\n",pos_x,pos_y);
*/
	if(pos_x == 0.0 || pos_x == 1.0 || pos_y == 0.0 || pos_y == 1.0)
	{
#ifdef PRINT_LINKFACE04
fprintf(stderr,"Pos on boundary %f %f\n",pos_x,pos_y);
#endif
		return;
	}
	sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
	flag = converge_node(sols[4],bb,dx,dy,dz,1,1,1);
	if(!flag)
	{
#ifdef PRINT_LINKFACE04
fprintf(stderr,"conv_failed\n");
#endif
	}
	else
		add_node(face,sols[4]);

#ifdef PRINT_LINKFACE04
	fprintf(stderr,"link_face: count %d f1 %d f2 %d f3 %d\n",
		0,f1,f2,f3);
	print_sol(sols[4]);
/*
	print_face(x_face);
	print_face(y_face);
	print_face(z_face);
*/
#endif
}

void link_face2sols(face_info *face,sol_info **sols,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,
	int f1,int f2,int f3)
{
	double pos_x,pos_y,vec[2];
	int i,count=2;
	int f1a,f2a,f3a;

#ifdef PRINT_LINKFACE04
	fprintf(stderr,"link_face: count %d f1 %d f2 %d f3 %d\n",
		2,f1,f2,f3);
print_face(face);
#endif
	sols[3] = sols[4] = NULL;
	if( PairTest(0,1) ) { AddLink(0,1); }
	else
	{
		GetMid;

		f1a = f1; f2a = f2; f3a = f3;
		if(sols[0]->dx == sols[1]->dx) f1a = sols[0]->dx;
		if(sols[0]->dy == sols[1]->dy) f2a = sols[0]->dy;
		if(sols[0]->dz == sols[1]->dz) f3a = sols[0]->dz;
/*
		f1 = f1a; f2 = f2a; f3 = f3a;
*/
		/* do we want a duplicate node */
		if( ( f1a == 0 && f2a == 0 && f3a == 0 )
		 || sols[0]->dx == 0 || sols[0]->dy == 0 || sols[0]->dz == 0
		 || sols[1]->dx == 0 || sols[1]->dy == 0 || sols[1]->dz == 0 )
		{
			int res2;

			sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
			res2 = converge_node(sols[4],bb,dx,dy,dz,f1,f2,f3);
			add_node(face,sols[4]);
			AddLink(0,4);
			AddLink(1,4);
#ifdef PRINT_LINKFACE04
			fprintf(stderr,"link_face2sols: All three zero conv %d\n",res2);
			print_face_brief(face);
#endif
			return;
		}
		else if( ( f1a == 0 && (f2a == 0 || f3a == 0 ) ) || ( f2a == 0 && f3a == 0 ) )
		{
			int res1=0,res2=0;
			double vec1[2],vec2[2],dist1,dist2,dist3,dist4;

			sols[3] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
			sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);

			calc_pos_on_face(face,sols[0],vec1);
			calc_pos_on_face(face,sols[1],vec2);
			if(f1a == 0 && f2a == 0)
			{
				res1 = converge_node(sols[3],bb,dx,dy,dz,0,1,1);
				res2 = converge_node(sols[4],bb,dx,dy,dz,1,0,1);
			}
			if(f1a == 0 && f3a == 0)
			{
				res1 = converge_node(sols[3],bb,dx,dy,dz,0,1,1);
				res2 = converge_node(sols[4],bb,dx,dy,dz,1,1,0);
			}
			if(f2a == 0 && f3a == 0)
			{
				res1 = converge_node(sols[3],bb,dx,dy,dz,1,0,1);
				res2 = converge_node(sols[4],bb,dx,dy,dz,1,1,0);
			}
			if(!res1 || ! res2)
			{
				fprintf(stderr,"link_face2sols: converge failed! %d %d %d %d %d\n",f1,f2,f3,res1,res2);
				GetMid;
				sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
				add_node(face,sols[4]);
				AddLink(0,4);
				AddLink(1,4);
#ifdef PRINT_LINKFACE04
#endif
				print_face_brief(face);
				return;
			}
			dist1 = sqrt((vec1[0]-sols[3]->root )*(vec1[0]-sols[3]->root)
				 +   (vec1[1]-sols[3]->root2)*(vec1[1]-sols[3]->root2) );
			dist2 = sqrt((vec1[0]-sols[4]->root )*(vec1[0]-sols[4]->root)
				 +   (vec1[1]-sols[4]->root2)*(vec1[1]-sols[4]->root2) );
			dist3 = sqrt((vec2[0]-sols[3]->root )*(vec2[0]-sols[3]->root)
				 +   (vec2[1]-sols[3]->root2)*(vec2[1]-sols[3]->root2) );
			dist4 = sqrt((vec2[0]-sols[4]->root )*(vec2[0]-sols[4]->root)
				 +   (vec2[1]-sols[4]->root2)*(vec2[1]-sols[4]->root2) );
			if(dist1 < dist2 && dist4 < dist3 )
			{
				if(f1a == 0 && f2a == 0)
				{
					sols[3]->dx = 0; sols[4]->dx = sols[1]->dx;
					sols[3]->dy = sols[0]->dy; sols[4]->dy = 0;
					sols[3]->dz = sols[4]->dz = f3a;
				}
				if(f1a == 0 && f3a == 0)
				{
					sols[3]->dx = 0; sols[4]->dx = sols[1]->dx;
					sols[3]->dy = sols[4]->dy = f2a;
					sols[3]->dz = sols[0]->dz; sols[4]->dz = 0;
				}
				if(f2a == 0 && f3a == 0)
				{
					sols[3]->dx = sols[4]->dx = f1a;
					sols[3]->dy = 0; sols[4]->dy = sols[1]->dy;
					sols[3]->dz = sols[0]->dz; sols[4]->dz = 0;
				}
				add_node(face,sols[3]);
				add_node(face,sols[4]);
				AddLink(0,3);			
				AddLink(3,4);			
				AddLink(4,1);			
			}
			else if(dist1 > dist2 && dist4 > dist3 )
			{
				if(f1a == 0 && f2a == 0)
				{
					sols[3]->dx = 0; sols[4]->dx = sols[0]->dx;
					sols[3]->dy = sols[1]->dy; sols[4]->dy = 0;
					sols[3]->dz = sols[4]->dz = f3a;
				}
				if(f1a == 0 && f3a == 0)
				{
					sols[3]->dx = 0; sols[4]->dx = sols[0]->dx;
					sols[3]->dy = sols[4]->dy = f2a;
					sols[3]->dz = sols[1]->dz; sols[4]->dz = 0;
				}
				if(f2a == 0 && f3a == 0)
				{
					sols[3]->dx = sols[4]->dx = f1a;
					sols[3]->dy = 0; sols[4]->dy = sols[0]->dy;
					sols[3]->dz = sols[1]->dz; sols[4]->dz = 0;
				}
				add_node(face,sols[3]);
				add_node(face,sols[4]);
				AddLink(1,3);			
				AddLink(3,4);			
				AddLink(4,0);			
			}
			else
			{
				fprintf(stderr,"link_face2sols: Wierd distances %f %f %f %f\n",dist1,dist2,dist3,dist4);
#ifdef PRINT_LINKFACE04
				print_sol(sols[0]);
				print_sol(sols[1]);
				print_sol(sols[3]);
				print_sol(sols[4]);
#endif
				sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
				add_node(face,sols[4]);
				AddLink(0,4);
				AddLink(1,4);
			}
#ifdef PRINT_LINKFACE04
				fprintf(stderr,"link_face2sols: added two nodes %d %d %d %d %d\n",f1,f2,f3,res1,res2);
				print_face_brief(face);
#endif

		}
		else
		{
			if(f1!=f1a || f2!= f2a || f3 != f3a)
			{
#ifdef PRINT_LINKFACE04
			fprintf(stderr,"link_face2: default ");
				/* change this line to fix crash f1 = f1a; f2 = f2a; f3 = f3a; */
				fprintf(stderr,"f1 %d %d f2 %d %d f3 %d %d\n",
					f1,f1a,f2,f2a,f3,f3a);
#endif
				sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
				add_node(face,sols[4]);
				AddLink(0,4);
				AddLink(1,4);
#ifdef PRINT_LINKFACE04
				print_face_brief(face);
#endif
			}
			else
			{
				sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
				add_node(face,sols[4]);
				AddLink(0,4);
				AddLink(1,4);
			}
		}
	}
}

void link_face3sols(face_info *face,sol_info **sols,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,
	int f1,int f2,int f3)
{
	double pos_x,pos_y,vec[2];
	int i,count=3;

	sols[2] = get_nth_sol_on_face(face,3);
	sols[3] = sols[4] = NULL;
	GetMid;
/*
	if( pos_x != pos_x || pos_y != pos_y )
		fprintf(stderr,"pos_x %f pos_y %f\n",pos_x,pos_y);
*/
	sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
	add_node(face,sols[4]);
	AddLink(0,4);
	AddLink(1,4);
	AddLink(2,4);
#ifdef PRINT_LINK_FACE
	fprintf(stderr,"link_face3sols: count %d f1 %d f2 %d f3 %d\n",
		count,f1,f2,f3);
	print_sol(sols[4]);
	print_sol(sols[0]);
	print_sol(sols[1]);
	print_sol(sols[2]);
#endif

}

int sameEdge(sol_info *s1,sol_info *s2)
{
	if(s1->type != s2->type) return 0;
	switch(s1->type)
	{
	case X_AXIS:
		if( s1->yl * s2->denom == s2->yl * s1->denom
		 && s1->zl * s2->denom == s2->zl * s1->denom ) return 1;
		else return 0;
	case Y_AXIS:
		if( s1->xl * s2->denom == s2->xl * s1->denom
		 && s1->zl * s2->denom == s2->zl * s1->denom ) return 1;
		else return 0;
	case Z_AXIS:
		if( s1->xl * s2->denom == s2->xl * s1->denom
		 && s1->yl * s2->denom == s2->yl * s1->denom ) return 1;
		else return 0;
	default:
		return 0;
	}
}

int SameFace(sol_info *s1,sol_info *s2)
{
	switch(s1->type)
	{
	case FACE_LL: case FACE_RR:
		if(s2->type != FACE_LL && s2->type != FACE_RR) return 0;
		if( s1->xl * s2->denom == s2->xl * s1->denom ) return 1;
		else return 0;
	case FACE_FF: case FACE_BB:
		if(s2->type != FACE_FF && s2->type != FACE_BB) return 0;
		if( s1->yl * s2->denom == s2->yl * s1->denom ) return 1;
		else return 0;
	case FACE_UU: case FACE_DD:
		if(s2->type != FACE_UU && s2->type != FACE_DD) return 0;
		if( s1->zl * s2->denom == s2->zl * s1->denom ) return 1;
		else return 0;
	default:
		return 0;
	}
}

void link_face4solsPos(face_info *face,sol_info **sols,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,
	int f1,int f2,int f3)
{
	double vec[2],pos_x=0.0,pos_y=0.0;
	int i,count=4;
	int Aind=-1,Bind=-1,Cind=-1,Dind=-1;

	sols[4] = NULL;
	if( PairTest(0,1) ) { Aind = 0; Bind = 1; Cind = 2; Dind = 3;	}
	if( PairTest(0,2) ) { Aind = 0; Bind = 2; Cind = 1; Dind = 3;	}
	if( PairTest(0,3) ) { Aind = 0; Bind = 3; Cind = 1; Dind = 2;	}
	if( PairTest(1,2) ) { Aind = 1; Bind = 2; Cind = 0; Dind = 3;	}
	if( PairTest(1,3) ) { Aind = 1; Bind = 3; Cind = 0; Dind = 2;	}
	if( PairTest(2,3) ) { Aind = 2; Bind = 3; Cind = 0; Dind = 2;	}
	if(Aind != -1)
	{
		AddLink(Aind,Bind);
		if( PairTest(Cind,Dind) )
		{
			AddLink(Cind,Dind);
			return;
		}
		pos_x = pos_y = 0.0;
		calc_pos_on_face(face,sols[Cind],vec);
		pos_x += vec[0];
		pos_y += vec[1];
		calc_pos_on_face(face,sols[Dind],vec);
		pos_x += vec[0];
		pos_y += vec[1];
		sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
		add_node(face,sols[4]);
		AddLink(2,4);
		AddLink(3,4);
		return;
	}

	/* None of the point match */
	/* Test we have (1,1) (1,-1) (-1,1) (-1,-1) */

	Aind = Bind = Cind = Dind = -1;
	for(i=0;i<4;++i)
	    switch(face->type)
	    {
		case FACE_LL: case FACE_RR:
			if(sols[i]->dy ==  1 && sols[i]->dz ==  1) Aind = i;
			if(sols[i]->dy ==  1 && sols[i]->dz == -1) Bind = i;
			if(sols[i]->dy == -1 && sols[i]->dz ==  1) Cind = i;
			if(sols[i]->dy == -1 && sols[i]->dz == -1) Dind = i;
		break;
		case FACE_FF: case FACE_BB:
			if(sols[i]->dx ==  1 && sols[i]->dz ==  1) Aind = i;
			if(sols[i]->dx ==  1 && sols[i]->dz == -1) Bind = i;
			if(sols[i]->dx == -1 && sols[i]->dz ==  1) Cind = i;
			if(sols[i]->dx == -1 && sols[i]->dz == -1) Dind = i;
		break;
		case FACE_DD: case FACE_UU:
			if(sols[i]->dx ==  1 && sols[i]->dy ==  1) Aind = i;
			if(sols[i]->dx ==  1 && sols[i]->dy == -1) Bind = i;
			if(sols[i]->dx == -1 && sols[i]->dy ==  1) Cind = i;
			if(sols[i]->dx == -1 && sols[i]->dy == -1) Dind = i;
		break;
		default:
	    }
	if(Aind != -1 && Bind != -1 && Cind != -1 && Dind != -1 )
	{
		/* Now a nicly behaved example */
		/* I think all sols should be on two oposite edges */
		if(sameEdge(sols[Aind],sols[Bind]) && sameEdge(sols[Cind],sols[Dind]))
		{
#ifdef PRINT_LINKFACE04
	fprintf(stderr,"link4+ AB CD: %d %d %d %d\n",Aind,Bind,Cind,Dind);
#endif
			pos_x = pos_y = 0.0;
			calc_pos_on_face(face,sols[Aind],vec);
			pos_x += vec[0];
			pos_y += vec[1];
			calc_pos_on_face(face,sols[Cind],vec);
			pos_x += vec[0];
			pos_y += vec[1];
			sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
			if(sols[Aind]->dx != 0 && sols[Aind]->dx == sols[Cind]->dx)
				sols[4]->dx = sols[Aind]->dx;
			if(sols[Aind]->dy != 0 && sols[Aind]->dy == sols[Cind]->dy)
				sols[4]->dy = sols[Aind]->dy;
			if(sols[Aind]->dz != 0 && sols[Aind]->dz == sols[Cind]->dz)
				sols[4]->dz = sols[Aind]->dz;

			add_node(face,sols[4]);
			AddLink(Aind,4);
			AddLink(Cind,4);
			pos_x = pos_y = 0.0;
			calc_pos_on_face(face,sols[Bind],vec);
			pos_x += vec[0];
			pos_y += vec[1];
			calc_pos_on_face(face,sols[Dind],vec);
			pos_x += vec[0];
			pos_y += vec[1];

			sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);

			if(sols[Bind]->dx != 0 && sols[Bind]->dx == sols[Dind]->dx)
				sols[4]->dx = sols[Bind]->dx;
			if(sols[Bind]->dy != 0 && sols[Bind]->dy == sols[Dind]->dy)
				sols[4]->dy = sols[Bind]->dy;
			if(sols[Bind]->dz != 0 && sols[Bind]->dz == sols[Dind]->dz)
				sols[4]->dz = sols[Bind]->dz;

			add_node(face,sols[4]);
			AddLink(Bind,4);
			AddLink(Dind,4);
#ifdef PRINT_LINKFACE04
print_face(face);
#endif
			return;
		}
		else if(sameEdge(sols[Aind],sols[Cind]) && sameEdge(sols[Bind],sols[Dind]))
		{
#ifdef PRINT_LINKFACE04
	fprintf(stderr,"link4+ AC BD: %d %d %d %d\n",Aind,Bind,Cind,Dind);
#endif
			pos_x = pos_y = 0.0;
			calc_pos_on_face(face,sols[Aind],vec);
			pos_x += vec[0];
			pos_y += vec[1];
			calc_pos_on_face(face,sols[Bind],vec);
			pos_x += vec[0];
			pos_y += vec[1];
			sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);

			if(sols[Aind]->dx != 0 && sols[Aind]->dx == sols[Bind]->dx)
				sols[4]->dx = sols[Aind]->dx;
			if(sols[Aind]->dy != 0 && sols[Aind]->dy == sols[Bind]->dy)
				sols[4]->dy = sols[Aind]->dy;
			if(sols[Aind]->dz != 0 && sols[Aind]->dz == sols[Bind]->dz)
				sols[4]->dz = sols[Aind]->dz;

			add_node(face,sols[4]);
			AddLink(Aind,4);
			AddLink(Bind,4);
			pos_x = pos_y = 0.0;
			calc_pos_on_face(face,sols[Cind],vec);
			pos_x += vec[0];
			pos_y += vec[1];
			calc_pos_on_face(face,sols[Dind],vec);
			pos_x += vec[0];
			pos_y += vec[1];
			sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);

			if(sols[Cind]->dx != 0 && sols[Cind]->dx == sols[Dind]->dx)
				sols[4]->dx = sols[Cind]->dx;
			if(sols[Cind]->dy != 0 && sols[Cind]->dy == sols[Dind]->dy)
				sols[4]->dy = sols[Cind]->dy;
			if(sols[Cind]->dz != 0 && sols[Cind]->dz == sols[Dind]->dz)
				sols[4]->dz = sols[Cind]->dz;

			add_node(face,sols[4]);
			AddLink(Cind,4);
			AddLink(Dind,4);
			return;
		}
	}
#ifdef PRINT_LINKFACE04
	fprintf(stderr,"linkFace4Pos: odd sols not in expected posn\n");
	print_face(face);
#endif
	GetMid;
	sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
	add_node(face,sols[4]);
	AddLink(0,4);
	AddLink(1,4);
	AddLink(2,4);
	AddLink(3,4);
}			
					
		 

void link_face4sols(face_info *face,sol_info **sols,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,
	int f1,int f2,int f3)
{
	face_info *x_face=NULL,*y_face=NULL,*z_face=NULL;
	sol_info *x_sols[2],*y_sols[2],*z_sols[2];
	int DerivFlag, x_count=0, y_count=0, z_count=0;
	double vec0[2],vec1[2],vec2[2],vec3[2],vec[2],pos_x=0.0,pos_y=0.0,lam;
	bern2D *dxx=NULL,*dxy=NULL,*dyy=NULL,*det=NULL;
	int i,count=4,sign,order[4],res1;
	char signStr[80];

	sols[2] = get_nth_sol_on_face(face,3);
	sols[3] = get_nth_sol_on_face(face,4);
	sols[4] = NULL;

	BuildSolSigns(sols,4,signStr);
	if( TestSigns1(signStr,4,3,"+++|+++|++-|+--","+++|++-|+-+|+--|-++|-+-|--+|---","abc|bca|cab",order)
	 || TestSigns2(signStr,4,3,"+++|+++|+-+|+--","+++|++-|+-+|+--|-++|-+-|--+|---","abc|bca|cab",order) )
	{
		fprintf(stderr,"Node and Link\n");
		AddLink(order[0],order[1]);
		calc_pos_on_face(face,sols[order[2]],vec);
		pos_x = vec[0];
		pos_y = vec[1];
		calc_pos_on_face(face,sols[order[3]],vec);
		pos_x += vec[0];
		pos_y += vec[1];
		sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
		add_node(face,sols[4]);
		AddLink(order[2],4);
		AddLink(order[3],4);
#ifdef PRINT_LINKFACE04
		print_face(face);
#endif
		return;
	}
	else if( TestSigns3(signStr,4,3,"+++|+++|++-|+-+","+++|++-|+-+|+--|-++|-+-|--+|---","abc|bca|cab",order) )
	{
		fprintf(stderr,"2 Nodes and a Link\n");
		AddLink(order[0],order[1]);
		calc_pos_on_face(face,sols[order[2]],vec);
		pos_x = vec[0];
		pos_y = vec[1];
		calc_pos_on_face(face,sols[order[3]],vec);
		pos_x += vec[0];
		pos_y += vec[1];
		sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
		add_node(face,sols[4]);
		AddLink(order[2],4);
		AddLink(order[3],4);
#ifdef PRINT_LINKFACE04
		print_face(face);
#endif
		return;
	}

	switch(face->type)
	{
	case FACE_LL: case FACE_RR:
		if(f2 || f3) break;
		dxx = diffx2D(dy);
		dxy = diffy2D(dy);
		dyy = diffy2D(dz);
		break;
	case FACE_FF: case FACE_BB:
		if(f1 || f3) break;
		dxx = diffx2D(dx);
		dxy = diffy2D(dx);
		dyy = diffy2D(dz);
		break;
	case FACE_UU: case FACE_DD:
		if(f1 || f2) break;
		dxx = diffx2D(dx);
		dxy = diffy2D(dx);
		dyy = diffy2D(dy);
		break;
	default:
	}
	if(dxx!=NULL)
		det = symetricDet2D(dxx,dxy,dyy);
	if(det == NULL)
	{
#ifdef PRINT_LINKFACE04
		fprintf(stderr,"Null det\n");
#endif
		sign = 0;
	}
	else	sign = allonesign2D(det);

#ifdef PRINT_LINKFACE04
fprintf(stderr,"link4: %d %d %d %d\n",f1,f2,f3,sign);
print_face(face);
#endif
	if(sign>0)
	{
		link_face4solsPos(face,sols,bb,dx,dy,dz,d2,f1,f2,f3);
		return;
	}
	if(sign==0)
	{
#ifdef PRINT_LINKFACE04
		fprintf(stderr,"Zero det\n");
		print_face(face);
		printbern2D(bb);
		printbern2D(dx);
		printbern2D(dy);
		printbern2D(dz);
		printbern2D(dxx);
		printbern2D(dxy);
		printbern2D(dyy);
		printbern2D(det);
#endif
	}

	GetMid;
	if( PairTest(0,1) && PairTest(2,3) && !PairTest(0,2) )
	{
		DerivFlag = TRUE;
		DerivTest(x_face,x_count,x_sols,dx,f1, y_face,y_count,y_sols,dy,f2);
		DerivTest(x_face,x_count,x_sols,dx,f1, z_face,z_count,z_sols,dz,f3);
		DerivTest(y_face,y_count,y_sols,dy,f2, z_face,z_count,z_sols,dz,f3);
		if( DerivFlag)
		{
			AddLink(0,1);
			AddLink(2,3);
			goto fini_link_face;
		}
		sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
		res1 = converge_node(sols[4],bb,dx,dy,dz,f1,f2,f3);
		if(res1 == 0)
		{
			AddLink(0,1);
			AddLink(2,3);
			goto fini_link_face;
		}
	}
	else if( PairTest(0,2) && PairTest(1,3) && !PairTest(0,1) )
	{
		DerivFlag = TRUE;
		DerivTest(x_face,x_count,x_sols,dx,f1, y_face,y_count,y_sols,dy,f2);
		DerivTest(x_face,x_count,x_sols,dx,f1, z_face,z_count,z_sols,dz,f3);
		DerivTest(y_face,y_count,y_sols,dy,f2, z_face,z_count,z_sols,dz,f3);
		if( DerivFlag)
		{
			AddLink(0,2);
			AddLink(1,3);
			goto fini_link_face;
		}
		sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
		res1 = converge_node(sols[4],bb,dx,dy,dz,f1,f2,f3);
		if(res1 == 0)
		{
			AddLink(0,2);
			AddLink(1,3);
			goto fini_link_face;
		}
	}
	else if( PairTest(0,3) && PairTest(1,2) && !PairTest(0,1) )
	{
		DerivFlag = TRUE;
		DerivTest(x_face,x_count,x_sols,dx,f1, y_face,y_count,y_sols,dy,f2);
		DerivTest(x_face,x_count,x_sols,dx,f1, z_face,z_count,z_sols,dz,f3);
		DerivTest(y_face,y_count,y_sols,dy,f2, z_face,z_count,z_sols,dz,f3);
		if( DerivFlag)
		{
			AddLink(0,3);
			AddLink(1,2);
			goto fini_link_face;
		}
		sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
		res1 = converge_node(sols[4],bb,dx,dy,dz,f1,f2,f3);
		if(res1 == 0)
		{
			AddLink(0,3);
			AddLink(1,2);
			goto fini_link_face;
		}
	}
	else
	{
		sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
		res1 = converge_node(sols[4],bb,dx,dy,dz,f1,f2,f3);
	}

/*
	if( pos_x != pos_x || pos_y != pos_y )
		fprintf(stderr,"pos_x %f pos_y %f\n",pos_x,pos_y);
*/
	add_node(face,sols[4]);
	AddLink(0,4);
	AddLink(1,4);
	AddLink(2,4);
	AddLink(3,4);

	fini_link_face:

#ifdef PRINT_LINKFACE04
		fprintf(stderr,"link_face4: finished DerivFalg %d res1 %d\n",DerivFlag,res1);
		print_face_brief(face);
#endif
	if( x_face != NULL ) free_face(x_face);
	if( y_face != NULL ) free_face(y_face);
	if( z_face != NULL ) free_face(z_face);
}

void link_facemanysols(face_info *face,sol_info **sols,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,
	int count,int f1,int f2,int f3)
{
	double pos_x,pos_y,vec[2];
	int i;

	sols[4] = NULL;
	GetMid;
	sols[4] = MakeNode(face,pos_x,pos_y,f1,f2,f3,dx,dy,dz,d2);
	add_node(face,sols[4]);
#ifdef PRINT_LINKFACE04
	fprintf(stderr,"link_face many sols: ");
	print_soltype(face->type);
	fprintf(stderr," (%d,%d,%d)/%d count %d f1 %d f2 %d f3 %d\n",
		face->xl,face->yl,face->zl,face->denom,
		count,f1,f2,f3);
#endif
	for(i=1;i<=count;++i)
	{
		sols[0] = get_nth_sol_on_face(face,i);
		AddLink(0,4);
	}
}

void link_face(face_info *big_face,face_info *face,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,bern2D *d2,int internal)
{
	int f1,f2,f3,count;
	sol_info *sols[5];


	if( allonesign2D(bb) ) return;
	f1 = allonesign2D(dx);
	f2 = allonesign2D(dy);
	f3 = allonesign2D(dz);	

	sols[0] = sols[1] = sols[2] = sols[3] = sols[4] = NULL;
	count = get_sols_on_face(face,sols);

#ifdef PRINT_LINK_FACE_ALL
	fprintf(stderr,"link_face: ");
	print_soltype(face->type);
	fprintf(stderr," (%d,%d,%d)/%d count %d f1 %d f2 %d f3 %d\n",
		face->xl,face->yl,face->zl,face->denom,
		count,f1,f2,f3);
#endif
	if( count == 0 )
	{
		if( f1 && f2 && f3 ) goto fini_link_face;
		goto reduce_face;
	}
	else if( count == 1 )
		goto fini_link_face;

	else if( count == 2 )
	{
		if( !f1 && !f2 && !f3 ) goto reduce_face;

		if( PairTest(0,1) && MatchDeriv(0) )
		{
			AddLink(0,1);
			goto fini_link_face;
		}
		goto reduce_face;
	}

	else if( count == 3 )
		goto reduce_face;

	else if( count == 4 )
	{
		sols[2] = get_nth_sol_on_face(face,3);
		sols[3] = get_nth_sol_on_face(face,4);

		if( f1 && f2 && f3 ) goto reduce_face;
		if( PairTest(0,1) && PairTest(2,3) )
		{
			if( PairTest(0,2) ) goto reduce_face;
			if( StraddleDeriv(0,2) )
			{
				AddLink(0,1); AddLink(2,3);
				goto fini_link_face;
			}
			else	goto reduce_face;
		}
		if( PairTest(0,2) && PairTest(1,3) )
		{
			if( StraddleDeriv(0,1) )
			{
				AddLink(0,2); AddLink(1,3);
				goto fini_link_face;
			}
			else	goto reduce_face;
		}
		if( PairTest(0,3) && PairTest(1,2) )
		{
			if( StraddleDeriv(0,1) )
			{
				AddLink(0,3); AddLink(1,2);
				goto fini_link_face;
			}
			else	goto reduce_face;
		}
	}

	reduce_face:

	if( face->denom < LINK_FACE_LEVEL )
	{
		ReduceFace(big_face,face,bb,dx,dy,dz,d2,internal,f1,f2,f3);
		goto fini_link_face;
	}

	if( count == 0 )
		link_face0sols(face,sols,bb,dx,dy,dz,d2,f1,f2,f3);
	else if( count == 2 )
		link_face2sols(face,sols,bb,dx,dy,dz,d2,f1,f2,f3);
	else if( count == 3 )
		link_face3sols(face,sols,bb,dx,dy,dz,d2,f1,f2,f3);
	else if( count == 4 )
		link_face4sols(face,sols,bb,dx,dy,dz,d2,f1,f2,f3);
	else
		link_facemanysols(face,sols,bb,dx,dy,dz,d2,count,f1,f2,f3);

	fini_link_face:

	return;
}

#define MatchNodes(a,b) (\
	   nodes[a]->sol->dx == nodes[b]->sol->dx \
	&& nodes[a]->sol->dy == nodes[b]->sol->dy \
	&& nodes[a]->sol->dz == nodes[b]->sol->dz )

int link_node_three_planes(box_info *box,bern3D *bb,bern3D *dx,bern3D *dy,bern3D *dz,
	int f1,int f2,int f3,int count,node_info **nodes)
{
	int num_all_zero=0,i,flag;
	double pos_x,pos_y,pos_z;
	double vec[3];
	sol_info *sol;
	node_info *midnode;

	if(count<6) return FALSE;
	for(i=0;i<count;++i)
	{
		if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0 )
			++num_all_zero;
	}
	if(num_all_zero < 6) return FALSE;

#ifdef PRINT_SING
	fprintf(stderr,"link_three_planes sucess\n");
	print_box_brief(box);
#endif

	pos_x = pos_y = pos_z = 0.0;
	for( i=0; i<count; ++i)
	{
		calc_pos_in_box(box,nodes[i]->sol,vec);
		pos_x +=  vec[0];
		pos_y +=  vec[1];
		pos_z +=  vec[2];
	}
	pos_x /= count; pos_y /= count; pos_z /= count;

	sol = make_sol3(BOX,box->xl,box->yl,box->zl,box->denom,
		pos_x,pos_y,pos_z );
	sol->dx = 0;
	sol->dy = 0;
	sol->dz = 0;

	flag = find_known_sing(sol);

	add_sing(box,sol);
	midnode = grballoc(node_info);
#ifdef TEST_ALLOC
	++nodecount; ++nodemax; ++nodenew;
#endif
	midnode->next = NULL;
	midnode->sol = sol;
	midnode->status = NODE;
	for( i=0; i<count; ++i)
	{
		add_node_link(box,midnode,nodes[i]);
	}
	return TRUE;
}
	
int link_node_crosscap(box_info *box, bern3D *bb,bern3D *dx,bern3D *dy,bern3D *dz,
	int f1,int f2,int f3, int count,node_info *nodes[])
{
	bern3D *dxx,*dxy,*dxz,*dyy,*dyz,*dzz,*mat1,*mat2,*mat3;
	node_info *midnode;
	double vec0[3],val;
	int fxx,fxy,fxz,fyy,fyz,fzz;
	int i,flag;
	int negxx, negxy, negxz, negyy, negyz, negzz;
	int posxx, posxy, posxz, posyy, posyz, poszz;
	sol_info *sol;

	fprintf(stderr,"link_crosscap\n");
	print_box_brief(box);
	dxx = diffx3D(dx); dxy = diffy3D(dx); dxz = diffz3D(dx);
			   dyy = diffy3D(dy); dyz = diffz3D(dy);
					      dzz = diffz3D(dz);
	fxx = allonesign3D(dxx); fxy = allonesign3D(dxy); fxz = allonesign3D(dxz);
	fyy = allonesign3D(dxx); fyz = allonesign3D(dyz); fzz = allonesign3D(dzz);

	negxx = negxy = negxz = negyy = negyz = negzz = 0;
	posxx = posxy = posxz = posyy = posyz = poszz = 0;
	for(i=0;i<count;++i)
	{
		if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0 )
		{
			calc_pos_in_box(box,nodes[i]->sol,vec0);
			if(!fxx) {
				val = evalbern3D(dxx,vec0);
				if(val < 0.0) negxx = 1;
				if(val > 0.0) posxx = 1;
			}
			if(!fxy) {
				val = evalbern3D(dxy,vec0);
				if(val < 0.0) negxy = 1;
				if(val > 0.0) posxy = 1;
			}
			if(!fxz) {
				val = evalbern3D(dxz,vec0);
				if(val < 0.0) negxz = 1;
				if(val > 0.0) posxz = 1;
			}
			if(!fyy) {
				val = evalbern3D(dyy,vec0);
				if(val < 0.0) negyy = 1;
				if(val > 0.0) posyy = 1;
			}
			if(!fyz) {
				val = evalbern3D(dyz,vec0);
				if(val < 0.0) negyz = 1;
				if(val > 0.0) posyz = 1;
			}
			if(!fzz) {
				val = evalbern3D(dzz,vec0);
				if(val < 0.0) negzz = 1;
				if(val > 0.0) poszz = 1;
			}
		}
	}
	mat1 = mat2 = mat3 = NULL;
	if( ( negxx && posxx ) ) mat1 = dxx;
	if( ( negxy && posxy ) ) { if(mat1!=NULL) mat1 = dxy; else mat2 = dxy; }
	if( ( negxz && posxz ) ) { if(mat1!=NULL) mat1 = dxz; else if(mat2!=NULL) mat2 = dxz; else mat3 = dxz; }
	if( ( negyy && posyy ) ) { if(mat1!=NULL) mat1 = dyy; else if(mat2!=NULL) mat2 = dyy; else mat3 = dyy; }
	if( ( negyz && posyz ) ) { if(mat1!=NULL) mat1 = dyz; else if(mat2!=NULL) mat2 = dyz; else mat3 = dyz; }
	if( ( negzz && poszz ) ) { if(mat1!=NULL) mat1 = dzz; else if(mat2!=NULL) mat2 = dzz; else mat3 = dzz; }
	if(mat1 == NULL)
	{
		fprintf(stderr,"link_cross_cap; mat1 == NULL count %d\n",count);
		if(count!=2) return FALSE;
		add_node_link_simple(box,nodes[0],nodes[1]);
		return TRUE;
	}
	sol = make_sol3(BOX,box->xl,box->yl,box->zl,box->denom,
		0.5,0.5,0.5 );
	sol->dx = f1;
	sol->dy = f2;
	sol->dz = f3;
	flag = converge_sing2(box,sol,bb,mat1,mat2,mat3);

	if(sol->root < 0.0 || sol->root > 1.0 || sol->root2 < 0.0 || sol->root2 > 1.0 || sol->root3 < 0.0 || sol->root3 > 1.0 )
	{
		fprintf(stderr,"link_crosscap: odd posn D %f %f %f\n",sol->root,sol->root2,sol->root3);
		print_box_brief(box);
	}
	add_sing(box,sol);

	midnode = grballoc(node_info);
#ifdef TEST_ALLOC
	++nodecount; ++nodemax; ++nodenew;
#endif
	midnode->next = NULL;
	midnode->sol = sol;
	midnode->status = NODE;
	for( i=0; i<count; ++i)
	{
		add_node_link(box,midnode,nodes[i]);
	}
	return TRUE;
}

int link_sing_many_zeros(box_info *box, bern3D *bb,bern3D *dx,bern3D *dy,bern3D *dz,
	int f1,int f2,int f3, int count,node_info *nodes[])
{
	bern3D *dxx,*dxy,*dxz,*dyy,*dyz,*dzz,*mat1,*mat2,*mat3;
	node_info *midnode;
	double vec0[3],val,*val_array;
	int fxx,fxy,fxz,fyy,fyz,fzz;
	int i,j,flag,*sign_array,*done,*matches,unmatched;
	int negxx, negxy, negxz, negyy, negyz, negzz;
	int posxx, posxy, posxz, posyy, posyz, poszz;
	sol_info *sol;

	fprintf(stderr,"link_crosscap (%d,%d,%d)/%d\n",box->xl,box->yl,box->zl,box->denom);

	dxx = diffx3D(dx); dxy = diffy3D(dx); dxz = diffz3D(dx);
			   dyy = diffy3D(dy); dyz = diffz3D(dy);
					      dzz = diffz3D(dz);
	fxx = allonesign3D(dxx); fxy = allonesign3D(dxy); fxz = allonesign3D(dxz);
	fyy = allonesign3D(dxx); fyz = allonesign3D(dyz); fzz = allonesign3D(dzz);
	sign_array = (int *) malloc(sizeof(int) * count * 6);
	val_array = (double *) malloc(sizeof(double) * count * 6);

	negxx = negxy = negxz = negyy = negyz = negzz = 0;
	posxx = posxy = posxz = posyy = posyz = poszz = 0;
	for(i=0;i<count;++i)
	{
		print_sol(nodes[i]->sol);
/*		if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0 )
		{
*/
			calc_pos_in_box(box,nodes[i]->sol,vec0);
			if(!fxx) {
				val = evalbern3D(dxx,vec0);
				if(val < 0.0) { negxx = 1; sign_array[i*6 + 0] = -1; }
				else if(val > 0.0) { posxx = 1; sign_array[i*6 + 0] = 1; }
				else sign_array[i*6 + 0] = 0;
				val_array[i*6 + 0] = val;
			}
			else sign_array[i*6 + 0] = fxx;

			if(!fxy) {
				val = evalbern3D(dxy,vec0);
				if(val < 0.0) { negxy = 1; sign_array[i*6 + 1] = -1; }
				else if(val > 0.0) { posxy = 1; sign_array[i*6 + 1] = 1; }
				else sign_array[i*6 + 1] = 0;
				val_array[i*6 + 1] = val;
			}
			else sign_array[i*6 + 1] = fxy;

			if(!fxz) {
				val = evalbern3D(dxz,vec0);
				if(val < 0.0) { negxz = 1; sign_array[i*6 + 2] = -1; }
				else if(val > 0.0) { posxz = 1; sign_array[i*6 + 2] = 1; }
				else sign_array[i*6 + 2] = 0;
				val_array[i*6 + 2] = val;
			}
			else sign_array[i*6 + 2] = fxz;

			if(!fyy) {
				val = evalbern3D(dyy,vec0);
				if(val < 0.0) { negyy = 1; sign_array[i*6 + 3] = -1; }
				else if(val > 0.0) { posyy = 1; sign_array[i*6 + 3] = 1; }
				else sign_array[i*6 + 3] = 0;
				val_array[i*6 + 3] = val;
			}
			else sign_array[i*6 + 3] = fyy;

			if(!fyz) {
				val = evalbern3D(dyz,vec0);
				if(val < 0.0) { negyz = 1; sign_array[i*6 + 4] = -1; }
				else if(val > 0.0) { posyz = 1; sign_array[i*6 + 4] = 1; }
				else sign_array[i*6 + 4] = 0;
				val_array[i*6 + 4] = val;
			}
			else sign_array[i*6 + 4] = fyz;

			if(!fzz) {
				val = evalbern3D(dzz,vec0);
				if(val < 0.0) { negzz = 1; sign_array[i*6 + 5] = -1; }
				else if(val > 0.0) { poszz = 1; sign_array[i*6 + 5] = 1; }
				else sign_array[i*6 + 5] = 0;
				val_array[i*6 + 5] = val;
			}
			else sign_array[i*6 + 5] = fzz;

#ifdef USE_2ND_DERIV
			if(nodes[i]->sol->dxx > 0) posxx = 1;
			if(nodes[i]->sol->dxx < 0) negxx = 1;
			if(nodes[i]->sol->dxy > 0) posxy = 1;
			if(nodes[i]->sol->dxy < 0) negxy = 1;
			if(nodes[i]->sol->dxz > 0) posxz = 1;
			if(nodes[i]->sol->dxz < 0) negxz = 1;

			if(nodes[i]->sol->dyy > 0) posyy = 1;
			if(nodes[i]->sol->dyy < 0) negyy = 1;
			if(nodes[i]->sol->dyz > 0) posyz = 1;
			if(nodes[i]->sol->dyz < 0) negyz = 1;
			if(nodes[i]->sol->dzz > 0) poszz = 1;
			if(nodes[i]->sol->dzz < 0) negzz = 1;
		fprintf(stderr,"signs %d %d %d yy %d %d %d\t",
			nodes[i]->sol->dxx,nodes[i]->sol->dxy,nodes[i]->sol->dxz,
			nodes[i]->sol->dyy,nodes[i]->sol->dyz,nodes[i]->sol->dzz);
		fprintf(stderr,"signs %d %d %d yy %d %d %d\n",
			sign_array[i*6+0],sign_array[i*6+1],sign_array[i*6+2],
			sign_array[i*6+3],sign_array[i*6+4],sign_array[i*6+5]);
		fprintf(stderr,"fxx %d %d %d yy %d %d %d\t",
			fxx,fxy,fxz, fyy,fyz,fzz);
		fprintf(stderr,"vals %f %f %f yy %f %f %f\n",
			val_array[i*6+0],val_array[i*6+1],val_array[i*6+2],
			val_array[i*6+3],val_array[i*6+4],val_array[i*6+5]);
#endif
/*
		}
*/
	}
#ifdef NOT_DEF
	done  = (int *) malloc(sizeof(int) * count);
	matches  = (int *) malloc(sizeof(int) * count);

	for( i=0; i<count; ++i)
	{
		done[i] = 0;
		matches[i]=-1;
	}
	for(i=0;i<count;++i)
		for(j=i+1;j<count;++j)
		{
			if(done[i] || done[j] ) continue;

#ifdef NOT_DEF
			if(nodes[i]->sol->dx || nodes[i]->sol->dy || nodes[i]->sol->dz)
			{
				if( nodes[i]->sol->dx == nodes[j]->sol->dx
				 && nodes[i]->sol->dy == nodes[j]->sol->dy
				 && nodes[i]->sol->dz == nodes[j]->sol->dz )
				{
					matches[i] = j;
					done[i] = done[j] = 1;
				}
			}
			else if(nodes[j]->sol->dx && !nodes[j]->sol->dy && !nodes[j]->sol->dz)
#endif
			{
				/* both are all zero */
				int has_match=0,has_unmatch=0;

				if( nodes[i]->sol->dx != 0 || nodes[j]->sol->dx != 0)
				{
				   if(nodes[i]->sol->dx == nodes[j]->sol->dx) 
					has_match=1; 
				   else
					has_unmatch=1;
				}

				if( nodes[i]->sol->dy != 0 || nodes[j]->sol->dy != 0)
				{
				   if(nodes[i]->sol->dy == nodes[j]->sol->dy) 
					has_match=1; 
				   else
					has_unmatch=1;
				}

				if( nodes[i]->sol->dz != 0 || nodes[j]->sol->dz != 0)
				{
				   if(nodes[i]->sol->dz == nodes[j]->sol->dz) 
					has_match=1; 
				   else
					has_unmatch=1;
				}

				if( nodes[i]->sol->dxx != 0 || nodes[j]->sol->dxx != 0 )
				{
				   if(nodes[i]->sol->dxx == nodes[j]->sol->dxx) 
					has_match=1; 
				   else
					has_unmatch=1;
				}
				if( nodes[i]->sol->dxy != 0 || nodes[j]->sol->dxy != 0 )
				{
				   if(nodes[i]->sol->dxy == nodes[j]->sol->dxy) 
					has_match=1; 
				   else
					has_unmatch=1;
				}
				if( nodes[i]->sol->dxz != 0 || nodes[j]->sol->dxz != 0 )
				{
				   if(nodes[i]->sol->dxz == nodes[j]->sol->dxz) 
					has_match=1; 
				   else
					has_unmatch=1;
				}
				if( nodes[i]->sol->dyy != 0 || nodes[j]->sol->dyy != 0 )
				{
				   if(nodes[i]->sol->dyy == nodes[j]->sol->dyy) 
					has_match=1; 
				   else
					has_unmatch=1;
				}
				if( nodes[i]->sol->dyz != 0 || nodes[j]->sol->dyz != 0 )
				{
				   if(nodes[i]->sol->dyz == nodes[j]->sol->dyz) 
					has_match=1; 
				   else
					has_unmatch=1;
				}

				if( nodes[i]->sol->dzz != 0 || nodes[j]->sol->dzz != 0 )
				{
				   if(nodes[i]->sol->dzz == nodes[j]->sol->dzz) 
					has_match=1; 
				   else
					has_unmatch=1;

				}
				if(has_match && !has_unmatch)
				{
					matches[i] = j;
					done[i] = done[j] = 1;
				}
			}
		}
			
	unmatched = 0;
	for( i=0; i<count; ++i)
	{
		if(!done[i]) ++unmatched;
	}
fprintf(stderr,"unmatched %d\n",unmatched);

	if(unmatched == 0)
	{
		for( i=0; i<count; ++i)
		{
			if(matches[i]!=-1)
				add_node_link_simple(box,nodes[i],nodes[matches[i]]);
		}
		return TRUE;
	}
/*		
	if(unmatched==2)
	{
		j = -1;
		for( i=0; i<count; ++i)
			if(!done[i]) { if(j==-1) j=i; break; }
		add_node_link_simple(box,nodes[i],nodes[j]);
	}
	if(unmatched<=2) return TRUE;
*/
#endif

	mat1 = mat2 = mat3 = NULL;
	if( ( negxx && posxx ) ) mat1 = dxx;
	if( ( negxy && posxy ) ) { if(mat1!=NULL) mat1 = dxy; else mat2 = dxy; }
	if( ( negxz && posxz ) ) { if(mat1!=NULL) mat1 = dxz; else if(mat2!=NULL) mat2 = dxz; else mat3 = dxz; }
	if( ( negyy && posyy ) ) { if(mat1!=NULL) mat1 = dyy; else if(mat2!=NULL) mat2 = dyy; else mat3 = dyy; }
	if( ( negyz && posyz ) ) { if(mat1!=NULL) mat1 = dyz; else if(mat2!=NULL) mat2 = dyz; else mat3 = dyz; }
	if( ( negzz && poszz ) ) { if(mat1!=NULL) mat1 = dzz; else if(mat2!=NULL) mat2 = dzz; else mat3 = dzz; }
	if(mat1 != NULL)
	{
		sol = make_sol3(BOX,box->xl,box->yl,box->zl,box->denom,
			0.5,0.5,0.5 );
		sol->dx = f1;
		sol->dy = f2;
		sol->dz = f3;
		flag = converge_sing2(box,sol,bb,mat1,mat2,mat3);
		fprintf(stderr,"link_sing_many_zeros conv %d\n",flag);
		if(!flag) return FALSE;
		print_sol(sol);
		if(sol->root < 0.0 || sol->root > 1.0 || sol->root2 < 0.0 || sol->root2 > 1.0 || sol->root3 < 0.0 || sol->root3 > 1.0 )
			fprintf(stderr,"link_crosscap: odd posn D %f %f %f\n",sol->root,sol->root2,sol->root3);
		
		add_sing(box,sol);

		midnode = grballoc(node_info);
#ifdef TEST_ALLOC
	++nodecount; ++nodemax; ++nodenew;
#endif
		midnode->next = NULL;
		midnode->sol = sol;
		midnode->status = NODE;
		for( i=0; i<count; ++i)
		{
				add_node_link_simple(box,midnode,nodes[i]);
		}
#ifdef PRINT_LINK_SING
		print_box_brief(box);
#endif
		return TRUE;
	}
	unmatched=count;
	fprintf(stderr,"link_sing_many_zeros; mat1 == NULL count %d\n",count);
	if(unmatched==2)
	{
		add_node_link_simple(box,nodes[0],nodes[1]);
		return TRUE;
	}
	if(unmatched==3)
	{
		add_node_link_simple(box,nodes[0],nodes[1]);
		add_node_link_simple(box,nodes[0],nodes[2]);
		add_node_link_simple(box,nodes[1],nodes[2]);
		return TRUE;
	}
	if(unmatched==4)
	{
		int matchAB=1,matchAC=1,matchAD=1,matchBC=1,matchBD=1,matchCD=1;
		int j;

		for(j=0;j<6;++j)
		{
			if(sign_array[0*6+j]!=sign_array[1*6+j]) matchAB=0;
			if(sign_array[0*6+j]!=sign_array[2*6+j]) matchAC=0;
			if(sign_array[0*6+j]!=sign_array[3*6+j]) matchAD=0;
			if(sign_array[1*6+j]!=sign_array[2*6+j]) matchBC=0;
			if(sign_array[1*6+j]!=sign_array[3*6+j]) matchBD=0;
			if(sign_array[2*6+j]!=sign_array[3*6+j]) matchCD=0;
		}
		if(matchAB && matchCD && !matchAC)
		{
			add_node_link_simple(box,nodes[0],nodes[1]);
			add_node_link_simple(box,nodes[2],nodes[3]);
			return TRUE;
		}
		if(matchAC && matchBD && !matchAB)
		{
			add_node_link_simple(box,nodes[0],nodes[2]);
			add_node_link_simple(box,nodes[1],nodes[3]);
			return TRUE;
		}
		if(matchAD && matchBC && !matchAB)
		{
			add_node_link_simple(box,nodes[0],nodes[3]);
			add_node_link_simple(box,nodes[1],nodes[2]);
			return TRUE;
		}
		fprintf(stderr,"link 4 with zeros failed\n");
		return FALSE;
	}
	return FALSE;
}

int link_node_sing(box_info *box,bern3D *bb,bern3D *dx,bern3D *dy,bern3D *dz,
	int f1,int f2,int f3,int count)
{
	double pos_x,pos_y,pos_z,vec0[3];
	sol_info *sol;
	int i,j,unmatched,flag,*done,*undone,order[8],all_zero_count;
	node_info *node,*midnode,**nodes,**resNodes;
	char *signStr;

#ifdef PRINT_LINK_SING
	print_box_brief(box);
#endif

	flag = 0;


	nodes = (node_info **) malloc(sizeof(node_info *) * count);
	resNodes = (node_info **) malloc(sizeof(node_info *) * count);
	done  = (int *) malloc(sizeof(int) * count);
	undone  = (int *) malloc(sizeof(int) * count);
	all_zero_count = 0;
	pos_x = pos_y = pos_z = 0.0;
	for( i=0; i<count; ++i)
	{
		node = get_nth_node_on_box(box,i+1);
		if(node->sol->dx == 0 && node->sol->dy == 0 && node->sol->dz == 0 ) 
			++all_zero_count;
		calc_pos_in_box(box,node->sol,vec0);
		pos_x +=  vec0[0];
		pos_y +=  vec0[1];
		pos_z +=  vec0[2];

		nodes[i] = node;
		done[i] = 0;
		undone[i] = -1;
	}
	if( count == 0 ) pos_x = pos_y = pos_z = 0.5;
	else
	{
		pos_x /= count;
		pos_y /= count;
		pos_z /= count;
	}
	if(pos_x < 0 || pos_x > 1.0 || pos_y < 0.0 || pos_y > 1.0 || pos_z < 0.0 || pos_z > 1.0 )
	{
		fprintf(stderr,"link_sing: odd posn A %f %f %f\n",pos_x,pos_y,pos_z);
		print_box_brief(box);
	}

	if(all_zero_count==2 && global_mode != MODE_KNOWN_SING)
	{
		flag = link_sing_many_zeros(box,bb,dx,dy,dz,f1,f2,f3,count,nodes);
#ifdef PRINT_LINK_SING
		fprintf(stderr,"Crosscap test %d\n",flag);
#endif
		if(flag) return TRUE;
	}
	if(all_zero_count==6 && global_mode != MODE_KNOWN_SING)
	{
		flag = link_node_three_planes(box,bb,dx,dy,dz,f1,f2,f3,count,nodes);
		if(flag) return TRUE;
	}


	signStr = BuildNodeSigns2(nodes,count);
	sol = make_sol3(BOX,box->xl,box->yl,box->zl,box->denom,
		pos_x,pos_y,pos_z );
	sol->dx = f1;
	sol->dy = f2;
	sol->dz = f3;
	flag = converge_sing(box,sol,f1,f2,f3);
#ifdef PRINT_LINK_SING
	fprintf(stderr,"converge test %d\n",flag);
#endif

	if(global_mode == MODE_KNOWN_SING && flag)
	{
fprintf(stderr,"Sing with known sings\n");
		add_sing(box,sol);

		if( count == 0 ) return TRUE;
		midnode = grballoc(node_info);
#ifdef TEST_ALLOC
		++nodecount; ++nodemax; ++nodenew;
#endif
		midnode->next = NULL;
		midnode->sol = sol;
		midnode->status = NODE;
		for( i=0; i<count; ++i)
		{
			add_node_link(box,midnode,nodes[i]);
		}
		return TRUE;
	}

	/* the singularity converged to outside the box */

	if(count == 0)
	{
	   if(flag)
	   {
		if(sol->root < 0.0 || sol->root > 1.0 || sol->root2 < 0.0 || sol->root2 > 1.0 || sol->root3 < 0.0 || sol->root3 > 1.0 )
		{
			fprintf(stderr,"link_sing: odd posn B %f %f %f\n",sol->root,sol->root2,sol->root3);
			print_box_brief(box);
		}
		add_sing(box,sol);

		if( count == 0 ) return TRUE;
		midnode = grballoc(node_info);
#ifdef TEST_ALLOC
		++nodecount; ++nodemax; ++nodenew;
#endif
		midnode->next = NULL;
		midnode->sol = sol;
		midnode->status = NODE;
		for( i=0; i<count; ++i)
		{
			add_node_link(box,midnode,nodes[i]);
		}
	   }
	   return TRUE;
	}


	if(count==4)
	{
		if( TestSigns4(signStr,4,3,"++0|+-0|+0+|+0-","+++|-++","abc|bca|cab",order)
		 || TestSigns5(signStr,4,3,"--0|+-0|0-0|0-0","+++|+-+","abc|bca|cab",order)
		 || TestSigns6(signStr,4,3,"--0|+-0|0+0|0+0","+++|+-+","abc|bca|cab",order) )
		{
fprintf(stderr,"Forcing a singularity\n");
			unmatched =4;
			goto force_sing;
		}
	}

	for(i=0;i<count;++i)
	{
		if(done[i]) continue;
		for(j=i+1;j<count;++j)
		{
			if(done[i] || done[j]) continue;
			if(MatchNodes(i,j))
				done[i] = done[j] = 1;
		}
	}
	unmatched = 0;
	for(i=0;i<count;++i)
	{
		if(!done[i]) { undone[unmatched] = i; ++unmatched; }
		done[i] = 0;
	}
		
	if(unmatched == 3)
	{	/* if there are three unmatched then one might be degenerate */
		/* typically its (1,0,1)--(1,0,0)--(1,0,-1) */
		/* or            (0,1,-1) -- (0,1,0) -- (-1,1,0) */

		int k,matchedX=0,matchedY=0,matchedZ=0;

		i = undone[0];
		j = undone[1];
		k = undone[2];
/*
fprintf(stderr,"Linking 3 unmatched\n");
print_sol(nodes[i]->sol);
print_sol(nodes[j]->sol);
print_sol(nodes[k]->sol);
*/
		if( nodes[i]->sol->dx == nodes[j]->sol->dx
		 && nodes[i]->sol->dx == nodes[k]->sol->dx ) matchedX = 1;
		if( nodes[i]->sol->dy == nodes[j]->sol->dy
		 && nodes[i]->sol->dy == nodes[k]->sol->dy ) matchedY = 1;
		if( nodes[i]->sol->dz == nodes[j]->sol->dz
		 && nodes[i]->sol->dz == nodes[k]->sol->dz ) matchedZ = 1;

		if(matchedX && matchedY)
		{
			if(nodes[i]->sol->dz == 0)
			{		
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}
			else if(nodes[j]->sol->dz == 0)
			{		
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}
			else
			{		
				add_node_link_simple(box,nodes[i],nodes[k]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}
			done[i] = done[j] = done[k];
			unmatched = 0;
		}
		else if(matchedX && matchedZ)
		{
			if(nodes[i]->sol->dy == 0)
			{		
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}
			else if(nodes[j]->sol->dy == 0)
			{		
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}
			else
			{		
				add_node_link_simple(box,nodes[i],nodes[k]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}
			done[i] = done[j] = done[k];
			unmatched = 0;
		}
		else if(matchedY && matchedZ)
		{
			if(nodes[i]->sol->dx == 0)
			{		
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}
			else if(nodes[j]->sol->dx == 0)
			{		
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}
			else
			{		
				add_node_link_simple(box,nodes[i],nodes[k]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}
			done[i] = done[j] = done[k];
			unmatched = 0;
		}
		else if(matchedX)
		{
			/* or            (0,1,-1) -- (0,1,0) -- (-1,1,0) */
			
			if(nodes[i]->sol->dx != 0 && nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0)
			{
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}				
			else if(nodes[j]->sol->dx != 0 && nodes[j]->sol->dy == 0 && nodes[j]->sol->dz == 0)
			{
				add_node_link_simple(box,nodes[j],nodes[i]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}				
			else if(nodes[k]->sol->dx != 0 && nodes[k]->sol->dy == 0 && nodes[k]->sol->dz == 0)
			{
				add_node_link_simple(box,nodes[k],nodes[i]);
				add_node_link_simple(box,nodes[k],nodes[j]);
			}
			else
			{
				fprintf(stderr,"link_sing: wierdness\n");
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}
			done[i] = done[j] = done[k];
			unmatched = 0;
		}
		else if(matchedY)
		{
			/* or            (0,1,-1) -- (0,1,0) -- (-1,1,0) */
			
			if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dz == 0 && nodes[i]->sol->dy != 0)
			{
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}				
			else if(nodes[j]->sol->dx == 0 && nodes[j]->sol->dz == 0 && nodes[j]->sol->dy != 0)
			{
				add_node_link_simple(box,nodes[j],nodes[i]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}				
			else if(nodes[k]->sol->dx == 0 && nodes[k]->sol->dz == 0 && nodes[k]->sol->dy != 0)
			{
				add_node_link_simple(box,nodes[k],nodes[i]);
				add_node_link_simple(box,nodes[k],nodes[j]);
			}
			else
			{
				fprintf(stderr,"link_sing: wierdness\n");
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}								
			done[i] = done[j] = done[k];
			unmatched = 0;
		}
		else if(matchedZ)
		{
			/* or            (0,1,-1) -- (0,1,0) -- (-1,1,0) */
			
			if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 && nodes[i]->sol->dz != 0)
			{
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}				
			else if(nodes[j]->sol->dx == 0 && nodes[j]->sol->dy == 0 && nodes[j]->sol->dz != 0)
			{
				add_node_link_simple(box,nodes[j],nodes[i]);
				add_node_link_simple(box,nodes[j],nodes[k]);
			}				
			else if(nodes[k]->sol->dx == 0 && nodes[k]->sol->dy== 0 && nodes[k]->sol->dz != 0)
			{
				add_node_link_simple(box,nodes[k],nodes[i]);
				add_node_link_simple(box,nodes[k],nodes[j]);
			}
			else
			{
				fprintf(stderr,"link_sing: wierdness\n");
				add_node_link_simple(box,nodes[i],nodes[j]);
				add_node_link_simple(box,nodes[i],nodes[k]);
			}								
			done[i] = done[j] = done[k];
			unmatched = 0;
		}
		else if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy== 0 && nodes[i]->sol->dz == 0)
		{
			fprintf(stderr,"link_sing: matching 000\n");
			add_node_link_simple(box,nodes[i],nodes[j]);
			add_node_link_simple(box,nodes[i],nodes[k]);
			done[i] = done[j] = done[k];
			unmatched = 0;
		}			
		else if(nodes[j]->sol->dx == 0 && nodes[j]->sol->dy== 0 && nodes[j]->sol->dz == 0)
		{
			fprintf(stderr,"link_sing: matching 000\n");
			add_node_link_simple(box,nodes[j],nodes[i]);
			add_node_link_simple(box,nodes[j],nodes[k]);
			done[i] = done[j] = done[k];
			unmatched = 0;
		}			
		else if(nodes[k]->sol->dx == 0 && nodes[k]->sol->dy== 0 && nodes[k]->sol->dz == 0)
		{
			fprintf(stderr,"link_sing: matching 000\n");
			add_node_link_simple(box,nodes[k],nodes[i]);
			add_node_link_simple(box,nodes[k],nodes[j]);
			done[i] = done[j] = done[k];
			unmatched = 0;
		}			
		else
		{
			fprintf(stderr,"No two unmatched\n");
		}
/*
		print_box(box);
*/
	}
	if(unmatched == 0 || unmatched == 2)
	{
		if( count >= 4 && Test4nodesLike011(nodes,count,order)	)
		{
			double vecs[4][3];
			int A,B,C,D;
			bern3D *mat=NULL,*ddx,*ddy,*ddz;
			double distAB,distAC,distAD,distBC,distBD,distCD;
			double dist1x,dist1y,dist1z, dist2x,dist2y,dist2z;
			double dist3x,dist3y,dist3z, dist4x,dist4y,dist4z;
			double dist5x,dist5y,dist5z, dist6x,dist6y,dist6z;

			if(nodes[order[0]]->sol->dx == 0)
				mat = dx;
			else if(nodes[order[0]]->sol->dy == 0)
				mat = dy;
			else if(nodes[order[0]]->sol->dz == 0)
				mat = dz;

			ddx = diffx3D(mat);
			ddy = diffy3D(mat);
			ddz = diffz3D(mat);
			if( !allonesign3D(ddx) && !allonesign3D(ddy) && !allonesign3D(ddz) )
			{
				free_bern3D(ddx); free_bern3D(ddy); free_bern3D(ddz);
				fprintf(stderr,"all derivs zero\n");
				pos_x = pos_y = pos_z = 0.0;
				for( i=0; i<4; ++i)
				{
					calc_pos_in_box(box,nodes[order[i]]->sol,vec0);
					pos_x +=  vec0[0];
					pos_y +=  vec0[1];
					pos_z +=  vec0[2];
				}
				pos_x /= 4;
				pos_y /= 4;
				pos_z /= 4;
				fprintf(stderr,"Calculated posn %f %f %f\n",pos_x,pos_y,pos_z);
				sol->root = pos_x;
				sol->root2 = pos_y;
				sol->root3 = pos_z;
				if(sol->root < 0.0 || sol->root > 1.0 || sol->root2 < 0.0 || sol->root2 > 1.0 || sol->root3 < 0.0 || sol->root3 > 1.0 )
				{
					fprintf(stderr,"link_sing: odd posn C %f %f %f\n",sol->root,sol->root2,sol->root3);
					print_box_brief(box);
				}
				add_sing(box,sol);

				midnode = grballoc(node_info);
#ifdef TEST_ALLOC
				++nodecount; ++nodemax; ++nodenew;
#endif
				midnode->next = NULL;
				midnode->sol = sol;
				midnode->status = NODE;
				for( i=0; i<4; ++i)
				{
					add_node_link(box,midnode,nodes[order[i]]);
					done[order[i]] = 1;
				}
				goto force_deriv_cross_sing;
			}
			free_bern3D(ddx); free_bern3D(ddy); free_bern3D(ddz);

			if(SameFace(nodes[order[0]]->sol,nodes[order[1]]->sol))
			{	A = order[0]; B = order[1]; C = order[2]; D = order[3];	}
			else if(SameFace(nodes[order[0]]->sol,nodes[order[2]]->sol))
			{	A = order[0]; B = order[2]; C = order[1]; D = order[3];	}
			else if(SameFace(nodes[order[0]]->sol,nodes[order[3]]->sol))
			{	A = order[0]; B = order[3]; C = order[2]; D = order[1];	}
			else if(SameFace(nodes[order[1]]->sol,nodes[order[2]]->sol))
			{	A = order[1]; B = order[2]; C = order[0]; D = order[3];	}
			else if(SameFace(nodes[order[1]]->sol,nodes[order[3]]->sol))
			{	A = order[1]; B = order[3]; C = order[0]; D = order[2];	}
			else if(SameFace(nodes[order[2]]->sol,nodes[order[3]]->sol))
			{	A = order[2]; B = order[3]; C = order[0]; D = order[1];	}
			else
			{
				fprintf(stderr,"link_sing: 4 id nodes but non on same face\n");
				A = order[0]; B = order[1]; C = order[2]; D = order[3];
			}
			calc_pos_actual(nodes[A]->sol,vecs[0]);
			calc_pos_actual(nodes[B]->sol,vecs[1]);
			calc_pos_actual(nodes[C]->sol,vecs[2]);
			calc_pos_actual(nodes[D]->sol,vecs[3]);
			dist1x = vecs[0][0] - vecs[1][0]; dist1y = vecs[0][1] - vecs[1][1]; dist1z = vecs[0][2] - vecs[1][2];
			dist2x = vecs[0][0] - vecs[2][0]; dist2y = vecs[0][1] - vecs[2][1]; dist2z = vecs[0][2] - vecs[2][2];
			dist3x = vecs[0][0] - vecs[3][0]; dist3y = vecs[0][1] - vecs[3][1]; dist3z = vecs[0][2] - vecs[3][2];
			dist4x = vecs[1][0] - vecs[2][0]; dist4y = vecs[1][1] - vecs[2][1]; dist4z = vecs[1][2] - vecs[2][2];
			dist5x = vecs[1][0] - vecs[3][0]; dist5y = vecs[1][1] - vecs[3][1]; dist5z = vecs[1][2] - vecs[3][2];
			dist6x = vecs[2][0] - vecs[3][0]; dist6y = vecs[2][1] - vecs[3][1]; dist6z = vecs[2][2] - vecs[3][2];

			distAB = sqrt(dist1x*dist1x+dist1y*dist1y+dist1z*dist1z);
			distAC = sqrt(dist2x*dist2x+dist2y*dist2y+dist2z*dist2z);
			distAD = sqrt(dist3x*dist3x+dist3y*dist3y+dist3z*dist3z);
			distBC = sqrt(dist4x*dist4x+dist4y*dist4y+dist4z*dist4z);
			distBD = sqrt(dist5x*dist5x+dist5y*dist5y+dist5z*dist5z);
			distCD = sqrt(dist6x*dist6x+dist6y*dist6y+dist6z*dist6z);
#ifdef PRINT_LINK_SING
			fprintf(stderr,"4 indentical nodes, distances %f %f %f %f %f %f\n",distAB,distAC,distAD,distBC,distBD,distCD);
#endif
			/* found 4 nodes with identical sign pattern */

			if(distAC < distBC && distBD < distAD )
			{
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes %d, %d and  %d, %d\n",A,C,B,D);
#endif
				add_node_link_simple(box,nodes[A],nodes[C]);
				add_node_link_simple(box,nodes[B],nodes[D]);
				done[A] = done[B] = done[C] = done[D] = 1;
			}
			else if(distAC > distBC && distBD > distAD )
			{
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes %d, %d and  %d, %d\n",A,D,B,C);
#endif
				add_node_link_simple(box,nodes[A],nodes[D]);
				add_node_link_simple(box,nodes[B],nodes[C]);
				done[A] = done[B] = done[C] = done[D] = 1;
			}
			else
			{
				fprintf(stderr,"link_sing: wierd distances\n");
				add_node_link_simple(box,nodes[A],nodes[C]);
				add_node_link_simple(box,nodes[B],nodes[D]);
				done[A] = done[B] = done[C] = done[D] = 1;
			}
				
		}
		force_deriv_cross_sing:
fprintf(stderr,"At fdcs: unmatched %d\n",unmatched);
#ifdef NOT_DEF
		for(i=0;i<count;++i)
		{
			if(done[i]) continue;
			for(j=i+1;j<count;++j)
			{
				if(done[i] || done[j]) continue;
				if(MatchNodes(i,j))
				{
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes fdcs: %d and %d done %d %d\n",i,j,done[i],done[j]);
#endif
					add_node_link_simple(box,nodes[i],nodes[j]);
					done[i] = done[j] = 1;
				}
			}
		}
#endif
	}

	unmatched=0;
	for(i=0;i<count;++i)
		if(!done[i]) ++unmatched;
	if(unmatched == 0 ) return TRUE;

	fprintf(stderr,"unmatched %d\n",unmatched);
#ifdef PRINT_LINK_SING
	print_box_brief(box);
#endif

	for(i=0;i<count;++i)
	{
		if(done[i]) continue;
		for(j=i+1;j<count;++j)
		{
			if(done[i] || done[j]) continue;
			if( MatchNodes(i,j) 
			 && (nodes[i]->sol->dx != 0 || nodes[i]->sol->dy != 0 || nodes[i]->sol->dz != 0 ) )
			{
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes fdcs: %d and %d done %d %d\n",i,j,done[i],done[j]);
#endif
				add_node_link_simple(box,nodes[i],nodes[j]);
				done[i] = done[j] = 1;
			}
		}
	}
	unmatched=0;
	for(i=0;i<count;++i)
		if(!done[i]) ++unmatched;
	fprintf(stderr,"unmatched %d all zero %d\n",unmatched,all_zero_count);
	if(unmatched == 0 ) return TRUE;

	if(all_zero_count == 1)
	{
		int zero_index;
		double vec[3];

		for(i=0;i<count;++i)
			if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0 )
				zero_index = i;
		calc_pos_in_box(box,nodes[zero_index]->sol,vec);
		sol->root = vec[0]; sol->root2 = vec[1]; sol->root3 = vec[2];
		midnode = grballoc(node_info);
#ifdef TEST_ALLOC
		++nodecount; ++nodemax; ++nodenew;
#endif
		add_sing(box,sol);
		midnode->next = NULL;
		midnode->sol = sol;
		midnode->status = NODE;
		for( i=0; i<count; ++i)
		{
			if(!done[i])
			{
				add_node_link_simple(box,nodes[i],midnode);
			}
		}
#ifdef PRINT_LINK_SING
fprintf(stderr,"link_sing: one all zero\n");
print_box_brief(box);
#endif
		return TRUE;
	}
				
	/* now if converged to sing add that. */

	   if(flag)
	   {
		if(sol->root < 0.0 || sol->root > 1.0 || sol->root2 < 0.0 || sol->root2 > 1.0 || sol->root3 < 0.0 || sol->root3 > 1.0 )
		{
			fprintf(stderr,"link_sing: odd posn B %f %f %f\n",sol->root,sol->root2,sol->root3);
			print_box_brief(box);
		}
		add_sing(box,sol);

		if( count == 0 ) return TRUE;
		midnode = grballoc(node_info);
#ifdef TEST_ALLOC
		++nodecount; ++nodemax; ++nodenew;
#endif
		midnode->next = NULL;
		midnode->sol = sol;
		midnode->status = NODE;
		for( i=0; i<count; ++i)
		{
			add_node_link_simple(box,midnode,nodes[i]);
		}
	        return TRUE;
	   }

	/* Now lets get really hacky if there is a node (1,0,0) link it
		to all nodes (1,+/-1,0) and (1,0,+/-0) 
	   then if there is a node (0,0,0) link to all undone nodes
		and all nodes like (1,0,0) 
	*/

	for(i=0;i<count;++i)
	{
		if(done[i]) continue;
		for(j=i+1;j<count;++j)
		{
			if(done[i] || done[j]) continue;
			if(MatchNodes(i,j))
			{
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes %d and %d done %d %d\n",i,j,done[i],done[j]);
				add_node_link_simple(box,nodes[i],nodes[j]);
				done[i] = done[j] = 1;
#endif
			}
		}
	}

	/* Link 	(1,1,0) to (1,0,0) or (0,1,0) */

	for(i=0;i<count;++i)
	{
		if(done[i]) continue;
		if( (nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 )
		 || (nodes[i]->sol->dx == 0 && nodes[i]->sol->dz == 0 )
		 || (nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0 ) ) continue;
		for(j=0;j<count;++j)
		{
			if(j==i) continue;
			if(nodes[j]->sol->dx == 0 && nodes[j]->sol->dy == 0 && nodes[j]->sol->dz == 0)
				continue;
			if(SameFace(nodes[i]->sol,nodes[j]->sol) ) continue;
			if(nodes[i]->sol->dx == 0)
			{
				if(nodes[j]->sol->dx != 0) continue;
				if(nodes[j]->sol->dy != 0 && nodes[j]->sol->dy != nodes[i]->sol->dy )
					continue;
				if(nodes[j]->sol->dz != 0 && nodes[j]->sol->dz != nodes[i]->sol->dz )
					continue;
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes2a %d and %d done %d %d\n",i,j,done[i],done[j]);
#endif
				add_node_link_simple(box,nodes[i],nodes[j]);
				done[i] = done[j] = 1;
				break;
			}
			if(nodes[i]->sol->dy == 0)
			{
				if(nodes[j]->sol->dy != 0) continue;
				if(nodes[j]->sol->dx != 0 && nodes[j]->sol->dx != nodes[i]->sol->dx )
					continue;
				if(nodes[j]->sol->dz != 0 && nodes[j]->sol->dz != nodes[i]->sol->dz )
					continue;
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes2b %d and %d done %d %d\n",i,j,done[i],done[j]);
#endif
				add_node_link_simple(box,nodes[i],nodes[j]);
				done[i] = done[j] = 1;
				break;
			}
			if(nodes[i]->sol->dz == 0)
			{
				if(nodes[j]->sol->dz != 0) continue;
				if(nodes[j]->sol->dx != 0 && nodes[j]->sol->dx != nodes[i]->sol->dx )
					continue;
				if(nodes[j]->sol->dy != 0 && nodes[j]->sol->dy != nodes[i]->sol->dy )
					continue;
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes2c %d and %d done %d %d\n",i,j,done[i],done[j]);
#endif
				add_node_link_simple(box,nodes[i],nodes[j]);
				done[i] = done[j] = 1;
				break;
			}
		}
	}
				
			
	for(i=0;i<count;++i)
	{
		if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 && nodes[i]->sol->dz == 0 )
		{
			int j;

			for(j=0;j<count;++j)
			{
				if(j==i) continue;
				if( !done[j] && ! SameFace(nodes[i]->sol,nodes[j]->sol) )
/*
				 || (nodes[j]->sol->dx != 0 && nodes[j]->sol->dy == 0 && nodes[j]->sol->dz == 0 )
				 || (nodes[j]->sol->dx == 0 && nodes[j]->sol->dy != 0 && nodes[j]->sol->dz == 0 )
				 || (nodes[j]->sol->dx == 0 && nodes[j]->sol->dy == 0 && nodes[j]->sol->dz != 0 ) )
*/
				{
#ifdef PRINT_LINK_SING
	fprintf(stderr,"Linking nodes3 %d and %d done %d %d\n",i,j,done[i],done[j]);
#endif
					add_node_link_simple(box,nodes[i],nodes[j]);
					done[i] = done[j] = 1;
				}
			}
#ifdef FAKE_SINGS
			if(done[i])
			{
				add_sing(box,nodes[i]->sol);
			}
#endif
		}
	}

	unmatched = 0;
	for(i=0;i<count;++i)
		if(!done[i])
		{
#ifdef PRINT_LINK_SING
			print_node(nodes[i]);
#endif
			++unmatched;
		}
	if(unmatched==0 || unmatched == 1) return TRUE;
	fprintf(stderr,"unmatched %d\n",unmatched);
	if(global_mode == MODE_KNOWN_SING) return TRUE;

	force_sing:

	fprintf(stderr,"force_sing: (%d,%d,%d)/%d\n",box->xl,box->yl,box->zl,box->denom);

	if(global_mode == MODE_KNOWN_SING)
		return TRUE;

	pos_x = pos_y = pos_z = 0.0;
	for( i=0; i<count; ++i)
	{
		if(!done[i])
		{
		calc_pos_in_box(box,nodes[i]->sol,vec0);
		pos_x +=  vec0[0];
		pos_y +=  vec0[1];
		pos_z +=  vec0[2];
		}
	}
	pos_x /= unmatched;
	pos_y /= unmatched;
	pos_z /= unmatched;
	sol->root = pos_x;
	sol->root2 = pos_y;
	sol->root3 = pos_z;
	if(sol->root < 0.0 || sol->root > 1.0 || sol->root2 < 0.0 || sol->root2 > 1.0 || sol->root3 < 0.0 || sol->root3 > 1.0 )
	{
		fprintf(stderr,"link_sing: odd posn C %f %f %f\n",sol->root,sol->root2,sol->root3);
#ifdef PRINT_LINK_SING
		print_box_brief(box);
#endif
	}
	add_sing(box,sol);

	midnode = grballoc(node_info);
#ifdef TEST_ALLOC
		++nodecount; ++nodemax; ++nodenew;
#endif
	midnode->next = NULL;
	midnode->sol = sol;
	midnode->status = NODE;
	for( i=0; i<count; ++i)
	{
		if(!done[i])
			add_node_link_simple(box,midnode,nodes[i]);
	}
	return TRUE;
}

int link_nodes_reduce(box_info *box,bern3D *bb,bern3D *dx,bern3D *dy,bern3D *dz)
{
	octbern3D aa;
	int flag;

	aa = reduce3D(bb);
	sub_devide_box(box);
	split_box(box,box->lfd,box->rfd,box->lbd,box->rbd,
		 box->lfu,box->rfu,box->lbu,box->rbu);

	find_all_faces(box->lfd,aa.lfd);
	find_all_faces(box->lfu,aa.lfu);
	find_all_faces(box->lbd,aa.lbd);
	find_all_faces(box->lbu,aa.lbu);
	find_all_faces(box->rfd,aa.rfd);
	find_all_faces(box->rfu,aa.rfu);
	find_all_faces(box->rbd,aa.rbd);
	find_all_faces(box->rbu,aa.rbu);

	box->lfd->status = FOUND_FACES;
	box->rbd->status = FOUND_FACES;
	box->rfu->status = FOUND_FACES;
	box->rfd->status = FOUND_FACES;
	box->lbu->status = FOUND_FACES;
	box->lfu->status = FOUND_FACES;
	box->lbd->status = FOUND_FACES;
	box->rbu->status = FOUND_FACES;

	flag = link_nodes(box->lfd,aa.lfd)
	&& link_nodes(box->rfd,aa.rfd)
	&& link_nodes(box->lbd,aa.lbd)
	&& link_nodes(box->rbd,aa.rbd)
	&& link_nodes(box->lfu,aa.lfu)
	&& link_nodes(box->rfu,aa.rfu)
	&& link_nodes(box->lbu,aa.lbu)
	&& link_nodes(box->rbu,aa.rbu);
/*
#ifdef FACETS
	combine_facets(box);
#endif
*/
	free_octbern3D(aa);
	return(flag);
}

/*
 * Function:    link_nodes(box,big_box)
 * action:      links together the nodes surronding a box.
 *              adds the links to the list in big_box.
 *		Returns FALSE on interupt
 */


int link_nodes(box_info *box,bern3D *bb)
{
	int f1,f2,f3,count;
	node_info *nodes[8];
	bern3D *dx,*dy,*dz;
	int flag = TRUE;

	if( check_interupt( NULL ) ) return(FALSE);
	if( allonesign3D(bb) ) return(TRUE);

	dx = diffx3D(bb);
	dy = diffy3D(bb);
	dz = diffz3D(bb);
	f1 = allonesign3D(dx);
	f2 = allonesign3D(dy);
	f3 = allonesign3D(dz);	

	if( f1 && f2 && f3 ) goto fini_nodes;  /* no chance of links */

	count = get_nodes_on_box_faces(box,nodes);

#ifdef PRINT_LINK_NODES
fprintf(stderr,"link_nodes (%d,%d,%d)/%d: dx %d %d %d count %d\n",
	box->xl,box->yl,box->zl,box->denom,f1,f2,f3,count);
print_box_brief(box);
#endif
	if( count == 0 )
	{
		sol_info *sols[2];

		/* test for isolated zeros: require !f1 !f2 !f3 and */
		/* no solutions on faces.			    */

		if( f1 || f2 || f3 ) goto fini_nodes;

		if( get_sols_on_face(box->ll,sols)
		 || get_sols_on_face(box->rr,sols)
		 || get_sols_on_face(box->ff,sols)
		 || get_sols_on_face(box->bb,sols)
		 || get_sols_on_face(box->dd,sols)
		 || get_sols_on_face(box->uu,sols) )
		{
			/* non zero count return */

			goto fini_nodes;
		}

		/* no solutions, posible isolated zero */
	}
	else if( count == 2 )
	{
		/* Only add links whose derivs match */

		if( MatchNodes(0,1)
		 && ( nodes[0]->sol->dx != 0 || nodes[0]->sol->dy != 0 
				|| nodes[0]->sol->dz != 0 ) )
		{
			add_node_link(box,nodes[0],nodes[1]);
			goto fini_nodes;
		}
	}
	else if( count == 4 )
	{
		int i;
		/*** posible for two different node_links across box ***/

		nodes[2] = get_nth_node_on_box(box,3);
		nodes[3] = get_nth_node_on_box(box,4);

		for(i=0;i<4;++i)
			if(nodes[i]->sol->dx == 0 && nodes[i]->sol->dy == 0 
				&& nodes[i]->sol->dz == 0 )
				goto reduce_nodes;

		if(MatchNodes(0,1) && MatchNodes(2,3) )
		{
			if(MatchNodes(0,2)) goto reduce_nodes;

			add_node_link(box,nodes[0],nodes[1]);
			add_node_link(box,nodes[2],nodes[3]);
			goto fini_nodes;
		}
		if(MatchNodes(0,2) && MatchNodes(1,3) )
		{
			if(MatchNodes(0,1)) goto reduce_nodes;

			add_node_link(box,nodes[0],nodes[2]);
			add_node_link(box,nodes[1],nodes[3]);
			goto fini_nodes;
		}
		if(MatchNodes(0,3) && MatchNodes(1,2) )
		{
			if(MatchNodes(0,2)) goto reduce_nodes;

			add_node_link(box,nodes[0],nodes[3]);
			add_node_link(box,nodes[1],nodes[2]);
			goto fini_nodes;
		}
	}

	reduce_nodes:

	/*** Too dificult to handle, either sub-devide or create a node ***/

	if( box->denom >= LINK_SING_LEVEL )
		flag = link_node_sing(box,bb,dx,dy,dz,f1,f2,f3,count);
	else
	{
		flag = link_nodes_reduce(box,bb,dx,dy,dz);
		free_bern3D(dx);
		free_bern3D(dy);
		free_bern3D(dz);
		return(flag);
	}

	fini_nodes:
#ifdef PRINT_LINK_SING
fprintf(stderr,"link_nodes: done %d %d %d count %d\n",f1,f2,f3,count);
print_box_brief(box);
#endif

	if(global_lf)
	{
		make_facets(box);
		draw_box(box);
	}

	free_bern3D(dx);
	free_bern3D(dy);
	free_bern3D(dz);
	return(flag);
}

/* All cals in resaled function */

void calc_pos_norm(sol_info *sol,double vec[3],double norm[3])
{
        calc_pos(sol,vec);
        
        if(sol->dx == 0) norm[0] = 0.0;
        else             norm[0] = evalbern3D(CC,vec);
        if(sol->dy == 0) norm[1] = 0.0;
        else             norm[1] = evalbern3D(DD,vec);
        if(sol->dz == 0) norm[2] = 0.0;
        else             norm[2] = evalbern3D(EE,vec);
}

/** finds the pos in original domain **/

void calc_pos_actual(sol_info *sol,double vec[3])
{
        calc_pos(sol,vec);
        
	/* The above is of corse incorrect as its been incorectly scaled */

	vec[0] = region.xmin + vec[0] * (region.xmax-region.xmin);
	vec[1] = region.ymin + vec[1] * (region.ymax-region.ymin);
	vec[2] = region.zmin + vec[2] * (region.zmax-region.zmin);
}

/** finds the pos and norm in original domain **/

void calc_pos_norm_actual(sol_info *sol,double vec[3],double norm[3])
{
        calc_pos_norm(sol,vec,norm);
        
	/* The above is of corse incorrect as its been incorectly scaled */

	vec[0] = region.xmin + vec[0] * (region.xmax-region.xmin);
	vec[1] = region.ymin + vec[1] * (region.ymax-region.ymin);
	vec[2] = region.zmin + vec[2] * (region.zmax-region.zmin);

	norm[0] = norm[0] / (region.xmax-region.xmin);
	norm[1] = norm[1] / (region.ymax-region.ymin);
	norm[2] = norm[2] / (region.zmax-region.zmin);
}

/** Finds the sols corespondig to the know singularities **/

extern void calc_known_sings(HPoint3 *pl,int num_know_sings);

void calc_known_sings(HPoint3 *pl,int nks)
{
	int i;

	num_known_sings = nks;
	known_sings = (sol_info **) malloc(sizeof(sol_info *) * num_known_sings);
	for(i=0;i<num_known_sings;++i)
	{
		if( pl[i].x < region.xmin
		 || pl[i].x > region.xmax
		 || pl[i].y < region.ymin
		 || pl[i].y > region.ymax
		 || pl[i].z < region.zmin
		 || pl[i].z > region.zmax )
		{
			fprintf(stderr,"Sing outside box %f %f %f\n",pl[i].x,pl[i].y,pl[i].z);
			known_sings[i] = make_sol3(BOX,-1,-1,-1,LINK_SING_LEVEL,0.0,0.0,0.0);
		}
		else
		{
			double xbox,ybox,zbox;
			int xl,yl,zl;

			xbox = LINK_SING_LEVEL * (pl[i].x - region.xmin) / (region.xmax - region.xmin);
			ybox = LINK_SING_LEVEL * (pl[i].y - region.ymin) / (region.ymax - region.ymin);
			zbox = LINK_SING_LEVEL * (pl[i].z - region.zmin) / (region.zmax - region.zmin);
			xl = (int) floor(xbox);
			yl = (int) floor(ybox);
			zl = (int) floor(zbox);
			known_sings[i] = make_sol3(BOX,xl,yl,zl,LINK_SING_LEVEL,
				xbox - xl,ybox - yl,zbox-zl);
			known_sings[i]->dx = 0;
			known_sings[i]->dy = 0;
			known_sings[i]->dz = 0;

/*
*/
			fprintf(stderr,"Known sing ");
			print_sol(known_sings[i]);
		}
	}
}

/**

To converge on to a surface use:

- (f / ( df . df ) ) df;

to converge to a singularity point could use:

-(row1.df,row2.df,row3.df) / det;
row1= ( d*ff-e^2, -(b*ff-c*e),  (b*e-c*d));
row2 = (-(b*ff-c*e),  (a*ff-c^2), -(a*e-b*c));
row3 = ( (b*e-c*d), -(a*e-b*c),  (a*d-b^2));
det = (a*d*ff-a*e^2-b^2*ff+2*b*c*e-c^2*d);
a = diff(fx,x); b = diff(fx,y); c = diff(fx,z);
                      d = diff(fy,y); e = diff(fy,z);
		            ff = diff(fz,z);
df = (fx,fy,fz);
fx = diff(f,x); fy = diff(f,y); fz = diff(f,z);
f = x^2 y - y^3 - z^2;

i.e.

M^(-1) df where
M = d2f.

Formula calculated from 
f(x+v) = f(x) + v . df = 0
v = gamma df
hence 
f(x) + gamma df . df = 0;
so
	v = -f(x)/(df.df) df

and
dfx(x+v) = dfx(x) + v . d2fx 
dfy(x+v) = dfy(x) + v . d2fy
dfz(x+v) = dfz(x) + v . d2fz
i.e.
df(x+v) = df(x) + d2f<**> v
        = df(x) + M v
so
v = -M^(-1) df

*/

/** converge_sing
 *  converges to a singularity point inside a box
 * returns false if converged to a point outside the box.
 **/

#define PRINT_CS_VEC() {	\
fprintf(stderr,"\t%6.3f %6.3f %6.3f\t",vec[0],vec[1],vec[2]); \
fprintf(stderr,"%6.3f %6.3f %6.3f\t",	\
	region.xmin + vec[0] * (region.xmax-region.xmin), 	\
	region.ymin + vec[1] * (region.ymax-region.ymin),	\
	region.zmin + vec[2] * (region.zmax-region.zmin));	\
fprintf(stderr,"%6.3f %6.3f %6.3f\n",	\
	vec[0] * sol->denom - sol->xl,	\
	vec[1] * sol->denom - sol->yl,	\
	vec[2] * sol->denom - sol->zl);	}
 
int converge_sing_old(box_info *box,sol_info *sol,int signDx,int signDy,int signDz);

int find_known_sing(sol_info *sol)
{
	int i;
	if(global_mode != MODE_KNOWN_SING)
		return FALSE;

	for(i=0;i<num_known_sings;++i)
	{
		if( sol->xl == known_sings[i]->xl
		 && sol->yl == known_sings[i]->yl
		 && sol->zl == known_sings[i]->zl )
		{
#ifdef PRINT_CONVERGE
#endif
fprintf(stderr,"converge_sing: matched ");
print_sol(known_sings[i]);
			sol->root = known_sings[i]->root;
			sol->root2 = known_sings[i]->root2;
			sol->root3 = known_sings[i]->root3;
			return TRUE;
		}
	}
	return FALSE;
}

int converge_sing(box_info *box,sol_info *sol,int signDx,int signDy,int signDz)
{
	if(global_mode == MODE_KNOWN_SING)
		return find_known_sing(sol);
	else
		return converge_sing_old(box,sol,signDx,signDy,signDz);
}
	
int converge_sing_old(box_info *box,sol_info *sol,int signDx,int signDy,int signDz)
{
	double vec[3],oldvec[3];
	double val,dx,dy,dz,dxx=0.0,dxy=0.0,dxz=0.0,dyy=0.0,dyz=0.0,dzz=0.0;
	int i;
	double sumsq;

	calc_pos(sol,vec);

#ifdef PRINT_CONVERGE
fprintf(stderr,"converge_sing: ");
print_sol(sol);
fprintf(stderr,"init"); PRINT_CS_VEC();
#endif	

	for(i=0;i<10;++i)
	{
		oldvec[0] = vec[0];
		oldvec[1] = vec[1];
		oldvec[2] = vec[2];

		/* first converge onto surface */

		val = evalbern3D(BB,vec);
		dx  = evalbern3D(CC,vec);
		dy  = evalbern3D(DD,vec);
		dz  = evalbern3D(EE,vec);
		sumsq = dx * dx + dy * dy + dz * dz;
		vec[0] -= val * dx / sumsq;
		vec[1] -= val * dy / sumsq;
		vec[2] -= val * dz / sumsq;
	
#ifdef PRINT_CONVERGE
fprintf(stderr,"%d",i); PRINT_CS_VEC();
#endif	

		/* then converge onto dx */

		if(!signDx)
		{
			dx  = evalbern3D(CC,vec);
			dxx = evalbern3D(Dxx,vec);
			dxy = evalbern3D(Dxy,vec);
			dxz = evalbern3D(Dxz,vec);
			sumsq = dxx * dxx + dxy * dxy + dxz * dxz;
			vec[0] -= dx * dxx / sumsq;
			vec[1] -= dx * dxy / sumsq;
			vec[2] -= dx * dxz / sumsq;

#ifdef PRINT_CONVERGE
fprintf(stderr,"dx"); PRINT_CS_VEC();
#endif	
		}


		/* then converge onto dy */

		if(!signDy)
		{
			dy  = evalbern3D(DD,vec);
			dxy = evalbern3D(Dxy,vec);
			dyy = evalbern3D(Dyy,vec);
			dyz = evalbern3D(Dyz,vec);
			sumsq = dxy * dxy + dyy * dyy + dyz * dyz;
			vec[0] -= dy * dxy / sumsq;
			vec[1] -= dy * dyy / sumsq;
			vec[2] -= dy * dyz / sumsq;

#ifdef PRINT_CONVERGE
fprintf(stderr,"dy"); PRINT_CS_VEC();
#endif	
		}

		/* then converge onto dz */

		if(!signDz)
		{
			dy  = evalbern3D(DD,vec);
			dxz = evalbern3D(Dxz,vec);
			dyz = evalbern3D(Dyz,vec);
			dzz = evalbern3D(Dzz,vec);
			sumsq = dxz * dxz + dyz * dyz + dzz * dzz;
			vec[0] -= dz * dxz / sumsq;
			vec[1] -= dz * dyz / sumsq;
			vec[2] -= dz * dzz / sumsq;

#ifdef PRINT_CONVERGE
fprintf(stderr,"dz"); PRINT_CS_VEC();
#endif	
		}

		if( vec[0] != vec[0] )
		{
			fprintf(stderr,"NaN in converge_sing\n");
			fprintf(stderr,"%f %f, %f %f %f, %f %f %f %f %f %f\n",
				sumsq,val, dx,dy,dz, dxx,dxy,dxz,dyy,dyz,dzz);
			PRINT_CS_VEC();
			print_sol(sol);
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			vec[2] = oldvec[2];
			calc_relative_pos(sol,vec);
			return TRUE;	/* we've found a zero! */
		}

		if( 
			vec[0] * sol->denom - sol->xl < 0.0 
		 ||	vec[0] * sol->denom - sol->xl > 1.0 
		 ||	vec[1] * sol->denom - sol->yl < 0.0 	
		 ||	vec[1] * sol->denom - sol->yl > 1.0 	
	 	 ||	vec[2] * sol->denom - sol->zl < 0.0
		 ||	vec[2] * sol->denom - sol->zl > 1.0 )
		{
#ifdef PRINT_CONVERGE
			fprintf(stderr,"converge_sing failed\n%f %f, %f %f %f, %f %f %f %f %f %f\n",
				sumsq,val,dx,dy,dz,dxx,dxy,dxz,dyy,dyz,dzz);
#endif	
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			vec[2] = oldvec[2];
			calc_relative_pos(sol,vec);
			return FALSE;			
		}
		
	}
	calc_relative_pos(sol,vec);
#ifdef PRINT_CONVERGE
fprintf(stderr,"converge_sing done: ");
print_sol(sol);
#endif	

	if( sol->root  < 0.0 || sol->root  > 1.0
	 || sol->root2 < 0.0 || sol->root2 > 1.0
	 || sol->root3 < 0.0 || sol->root3 > 1.0 )
		return FALSE;
	else
		return TRUE;
}

int converge_sing2(box_info *box,sol_info *sol,bern3D *bb,bern3D *A,bern3D *B,bern3D *C)
{
	double vec[3],oldvec[3];
	double val,dx,dy,dz;
	int i;
	double sumsq;
	bern3D *bbx=NULL,*bby=NULL,*bbz=NULL,*Ax=NULL,*Ay=NULL,*Az=NULL,
		*Bx=NULL,*By=NULL,*Bz=NULL,*Cx=NULL,*Cy=NULL,*Cz=NULL;

	vec[0] = sol->root;
	vec[1] = sol->root2;
	vec[2] = sol->root3;
	if(bb!=NULL) { bbx = diffx3D(bb); bby = diffy3D(bb); bbz = diffz3D(bb); }
	if(A!=NULL) { Ax = diffx3D(A); Ay = diffy3D(A); Az = diffz3D(A); }
	if(B!=NULL) { Bx = diffx3D(B); By = diffy3D(B); Bz = diffz3D(B); }
	if(C!=NULL) { Cx = diffx3D(C); Cy = diffy3D(C); Cz = diffz3D(C); }
#ifdef PRINT_CONVERGE
fprintf(stderr,"converge_sing2: ");
print_sol(sol);
fprintf(stderr,"init"); PRINT_CS_VEC();
#endif	

	for(i=0;i<10;++i)
	{
		oldvec[0] = vec[0];
		oldvec[1] = vec[1];
		oldvec[2] = vec[2];

		/* first converge onto surface */

		val = evalbern3D(bb,vec);
		dx  = evalbern3D(bbx,vec);
		dy  = evalbern3D(bby,vec);
		dz  = evalbern3D(bbz,vec);
		sumsq = dx * dx + dy * dy + dz * dz;
		vec[0] -= val * dx / sumsq;
		vec[1] -= val * dy / sumsq;
		vec[2] -= val * dz / sumsq;
	
#ifdef PRINT_CONVERGE
fprintf(stderr,"%d",i); PRINT_CS_VEC();
#endif	

		/* then converge onto dx */

		if(A!=NULL)
		{
			val = evalbern3D(A,vec);
			dx  = evalbern3D(Ax,vec);
			dy  = evalbern3D(Ay,vec);
			dz  = evalbern3D(Az,vec);
			sumsq = dx * dx + dy * dy + dz * dz;
			vec[0] -= val * dx / sumsq;
			vec[1] -= val * dy / sumsq;
			vec[2] -= val * dz / sumsq;
#ifdef PRINT_CONVERGE
fprintf(stderr,"dx"); PRINT_CS_VEC();
#endif	
		}

		if(B!=NULL)
		{
			val = evalbern3D(B,vec);
			dx  = evalbern3D(Bx,vec);
			dy  = evalbern3D(By,vec);
			dz  = evalbern3D(Bz,vec);
			sumsq = dx * dx + dy * dy + dz * dz;
			vec[0] -= val * dx / sumsq;
			vec[1] -= val * dy / sumsq;
			vec[2] -= val * dz / sumsq;
#ifdef PRINT_CONVERGE
fprintf(stderr,"dx"); PRINT_CS_VEC();
#endif	
		}

		if(C!=NULL)
		{
			val = evalbern3D(C,vec);
			dx  = evalbern3D(Cx,vec);
			dy  = evalbern3D(Cy,vec);
			dz  = evalbern3D(Cz,vec);
			sumsq = dx * dx + dy * dy + dz * dz;
			vec[0] -= val * dx / sumsq;
			vec[1] -= val * dy / sumsq;
			vec[2] -= val * dz / sumsq;
#ifdef PRINT_CONVERGE
fprintf(stderr,"dx"); PRINT_CS_VEC();
#endif	
		}

		if( vec[0] != vec[0] )
		{
			fprintf(stderr,"NaN in converge_sing2\n");
			fprintf(stderr,"%f %f %f %f %f\n",val,dx,dy,dz,sumsq);
			print_sol(sol);
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			vec[2] = oldvec[2];
			sol->root = vec[0];
			sol->root2 = vec[1];
			sol->root3 = vec[2];
			return FALSE;			
		}

		if( 
			vec[0] < 0.0 
		 ||	vec[0] > 1.0 
		 ||	vec[1] < 0.0 	
		 ||	vec[1] > 1.0 	
	 	 ||	vec[2] < 0.0
		 ||	vec[2] > 1.0 )
		{
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			vec[2] = oldvec[2];
			sol->root = vec[0];
			sol->root2 = vec[1];
			sol->root3 = vec[2];
			return FALSE;			
		}
		
	}
#ifdef PRINT_CONVERGE
fprintf(stderr,"converge_sing2 done: ");
print_sol(sol);
#endif	
	sol->root = vec[0];
	sol->root2 = vec[1];
	sol->root3 = vec[2];

	if( sol->root  < 0.0 || sol->root  > 1.0
	 || sol->root2 < 0.0 || sol->root2 > 1.0
	 || sol->root3 < 0.0 || sol->root3 > 1.0 )
		return FALSE;
	return TRUE;
}

#define PRINT_CN_VEC() {double v=0.0,vx=0.0,vy=0.0,vz=0.0;	\
v = evalbern2D(bb,vec);	\
if(!signDx) vx = evalbern2D(dx,vec);	\
if(!signDy) vy = evalbern2D(dy,vec);	\
if(!signDz) vz = evalbern2D(dz,vec);	\
fprintf(stderr,"\t%6.3f %6.3f\t%g %g %g %g\n\t",vec[0],vec[1],v,vx,vy,vz); \
if(sol->type == FACE_RR || sol->type == FACE_LL )	\
fprintf(stderr,"%f %f %f\n",	\
	region.xmin + ( ((double) sol->xl)/sol->denom ) * (region.xmax-region.xmin), 	\
	region.ymin + ( (sol->yl + vec[0])/sol->denom ) * (region.ymax-region.ymin),	\
	region.zmin + ( (sol->zl + vec[1])/sol->denom ) * (region.zmax-region.zmin));	\
else if(sol->type == FACE_BB || sol->type == FACE_FF)	\
fprintf(stderr,"%f %f %f\n",	\
	region.xmin + ( (sol->xl + vec[0])/sol->denom ) * (region.xmax-region.xmin), 	\
	region.ymin + ( ((double) sol->yl)/sol->denom ) * (region.ymax-region.ymin),	\
	region.zmin + ( (sol->zl + vec[1])/sol->denom ) * (region.zmax-region.zmin));	\
else if(sol->type == FACE_UU || sol->type == FACE_DD)	\
fprintf(stderr,"%f %f %f\n",	\
	region.xmin + ( (sol->xl + vec[0])/sol->denom ) * (region.xmax-region.xmin), 	\
	region.ymin + ( (sol->yl + vec[1])/sol->denom ) * (region.ymax-region.ymin),	\
	region.zmin + ( ((double) sol->zl)/sol->denom ) * (region.zmax-region.zmin));	\
}

int converge_node(sol_info *sol,
	bern2D *bb,bern2D *dx,bern2D *dy,bern2D *dz,
	int signDx,int signDy,int signDz)
{
	double vec[2],oldvec[2];
	bern2D *bb_x,*bb_y;
	bern2D *dx_x=NULL,*dx_y=NULL, *dy_x=NULL,*dy_y=NULL, *dz_x=NULL, *dz_y=NULL;
	int i,flag=TRUE;
	double sumsq,a,b,val;

	bb_x = diffx2D(bb); bb_y = diffy2D(bb);
	if(!signDx) { dx_x = diffx2D(dx); dx_y = diffy2D(dx); }
	if(!signDy) { dy_x = diffx2D(dy); dy_y = diffy2D(dy); }
	if(!signDz) { dz_x = diffx2D(dz); dz_y = diffy2D(dz); }

#ifdef PRINT_CONVERGE
fprintf(stderr,"converge_node: ");
print_sol(sol);
/* printbern2D(bb); */
fprintf(stderr,"init"); PRINT_CN_VEC();
#endif	
	vec[0] = sol->root;
	vec[1] = sol->root2;

	for(i=0;i<10;++i)
	{
		oldvec[0] = vec[0];
		oldvec[1] = vec[1];

		/* first converge onto surface */

		val = evalbern2D(bb,vec);
		a   = evalbern2D(bb_x,vec);

/*		vec[0] -= val / a;		
#ifdef PRINT_CONVERGE
fprintf(stderr,"a %d",i); PRINT_CN_VEC();
#endif	
*/

		val = evalbern2D(bb,vec);
		b   = evalbern2D(bb_y,vec);
/*		vec[1] -= val / b;
*/
		sumsq = a * a + b * b;
		vec[0] -= val * a / sumsq;
		vec[1] -= val * b / sumsq;

#ifdef PRINT_CONVERGE
fprintf(stderr,"b %d",i); PRINT_CN_VEC();
#endif	

		if( vec[0] != vec[0] )
		{
			fprintf(stderr,"NaN in converge_node\n");
			PRINT_CN_VEC();
			print_sol(sol);
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			flag = FALSE;
			break;		
		}

		if( 
			vec[0] < 0.0 
		 ||	vec[0] > 1.0 
		 ||	vec[1] < 0.0 	
		 ||	vec[1] > 1.0 )
		{
#ifdef PRINT_CONVERGE
fprintf(stderr,"failed"); PRINT_CN_VEC();
#endif	
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			flag = FALSE;
			break;		
		}

#ifdef PRINT_CONVERGE
fprintf(stderr,"%d",i); PRINT_CN_VEC();
#endif	

		/* then converge onto dx */

		if(!signDx)
		{
			val = evalbern2D(dx,vec);
			a   = evalbern2D(dx_x,vec);
			b   = evalbern2D(dx_y,vec);
			sumsq = a * a + b * b;
			vec[0] -= val * a / sumsq;
			vec[1] -= val * b / sumsq;

#ifdef PRINT_CONVERGE
fprintf(stderr,"dx"); PRINT_CN_VEC();
#endif	
		}

		if(!signDy)
		{
			val = evalbern2D(dy,vec);
			a   = evalbern2D(dy_x,vec);
			b   = evalbern2D(dy_y,vec);
			sumsq = a * a + b * b;
			vec[0] -= val * a / sumsq;
			vec[1] -= val * b / sumsq;

#ifdef PRINT_CONVERGE
fprintf(stderr,"dy"); PRINT_CN_VEC();
#endif	
		}

		if(!signDz)
		{
			val = evalbern2D(dz,vec);
			a   = evalbern2D(dz_x,vec);
			b   = evalbern2D(dz_y,vec);
			sumsq = a * a + b * b;
			vec[0] -= val * a / sumsq;
			vec[1] -= val * b / sumsq;
#ifdef PRINT_CONVERGE
fprintf(stderr,"dz"); PRINT_CN_VEC();
#endif	
		}

		if( vec[0] != vec[0] )
		{
			fprintf(stderr,"NaN in converge_node\n");
			PRINT_CN_VEC();
			print_sol(sol);
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			flag = FALSE;
			break;		
		}
			
		if( 
			vec[0] < 0.0 
		 ||	vec[0] > 1.0 
		 ||	vec[1] < 0.0 	
		 ||	vec[1] > 1.0 )
		{
#ifdef PRINT_CONVERGE
fprintf(stderr,"failed"); PRINT_CN_VEC();
#endif	
			vec[0] = oldvec[0];
			vec[1] = oldvec[1];
			flag = FALSE;
			break;		
		}
		
	}
#ifdef PRINT_CONVERGE
	if(flag) fprintf(stderr,"sucess"); PRINT_CN_VEC();
#endif
	sol->root = vec[0];
	sol->root2 = vec[1];

	free_bern2D(bb_x); free_bern2D(bb_y);
	if(!signDx) { free_bern2D(dx_x); free_bern2D(dx_y); }
	if(!signDy) { free_bern2D(dy_x); free_bern2D(dy_y); }
	if(!signDz) { free_bern2D(dz_x); free_bern2D(dz_y); }
	return flag;
}


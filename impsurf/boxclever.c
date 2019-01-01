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
#include "cells.h"
#include <math.h>

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
#define PRINT_LINK_ALL_FACES
#define CHECK_SECOND
#define PRINT_SECOND
#define PRINT_ODD_BOX
#define PRINT_SOL_VAL_ERR
#define PRINT_GEN_BOXES
#define PRINT_ODD_BOX
#define PRINT_LINK_FACE_ALL
#define PRINT_FIND_EDGE_SOL
#define VERBOUSE
#define PRINT_LINK_FACE
#define PRINT_LINK_NODES
*/

#define NON_GENERIC_NODES
#define NON_GENERIC_EDGE
#define DO_FREE
#define LINK_SING
#define LINK_SOLS
#define LINK_FACE
#define DO_PLOT

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

extern double EvalEdge(edge_info *edge,double lam);
extern double EvalEdgeDerivX(edge_info *edge,double lam);
extern double EvalEdgeDerivY(edge_info *edge,double lam);
extern double EvalEdgeDerivZ(edge_info *edge,double lam);
extern double EvalFace(face_info *face,double lam,double lam2);
extern double EvalFaceDerivX(face_info *face,double lam,double lam2);
extern double EvalFaceDerivY(face_info *face,double lam,double lam2);
extern double EvalFaceDerivZ(face_info *face,double lam,double lam2);
extern double EvalBox(box_info *box,double lam,double lam2,double lam3);
extern double EvalBoxDerivX(box_info *box,double lam,double lam2,
		double lam3);
extern double EvalBoxDerivY(box_info *box,double lam,double lam2,
		double lam3);
extern double EvalBoxDerivZ(box_info *box,double lam,double lam2,
		double lam3);

/**** Forward defs ****/

void find_all_faces();
void find_face();
void find_all_edges();
void find_edge();
void find_sols_on_edge();
void ReduceFace();
void link_face();
void calc_pos_norm();


box_info *whole_box;
region_info region;
double (*global_fun)();
double (*global_df_dx)();
double (*global_df_dy)();
double (*global_df_dz)();

/*********************** Start of Sub-routines **************************/
/*									*/
/* draws an arbitrary polynomial curve in a specified region of space   */
/*									*/
/*********************** Start of Sub-routines **************************/

int marmain(fun,df_dx,df_dy,df_dz,xmin,xmax,ymin,ymax,zmin,zmax)
double (*fun)();
double (*df_dx)(), (*df_dy)(), (*df_dz)();
double xmin,xmax,ymin,ymax,zmin,zmax;
{
  int flag;

  region.xmin = xmin;
  region.ymin = ymin;
  region.zmin = zmin;
  region.xmax = xmax;
  region.ymax = ymax;
  region.zmax = zmax;
  global_fun = fun;
  global_df_dx = df_dx;
  global_df_dy = df_dy;
  global_df_dz = df_dz;

  init_cells();

#ifdef VERBOUSE
  fprintf(stderr,"range %f %f %f %f %f %f\n",
  	xmin,xmax,ymin,ymax,zmin,zmax);
#endif
 whole_box = allocbox();
  make_box(whole_box,0,0,0,1);
  flag = generate_boxes(whole_box);
  flag = flag && !check_interupt("Writing Data");
  fini_cells();
  free_box(whole_box);
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

int generate_boxes(box)
box_info *box;
{
	int xl,yl,zl,denom;
	double percent = 0.0;
	int flag;
	char string[80];

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
	sprintf(string,"Done %6.2lf percent.",percent*100.0);
	if( check_interupt( string ) ) return(FALSE);

      /*** If all derivitives non zero and the region is sufficiently	***/
      /***  small then draw the surface.				***/

   if( box->denom >= RESOLUTION )
   {
#ifdef PRINT_GEN_BOXES
   	fprintf(stderr,"generate_boxes: box (%d,%d,%d)/%d LEAF\n",
		box->xl,box->yl,box->zl,box->denom);
#endif

	return(find_box(box));
   }
   else
   {		/**** Sub-devide the region into 8 sub boxes.  ****/
#ifdef PRINT_GEN_BOXES
   	fprintf(stderr,"generate_boxes: box (%d,%d,%d)/%d NODE\n",
		box->xl,box->yl,box->zl,box->denom);
#endif

	sub_devide_box(box);
	flag = (generate_boxes(box->lfd) &&
		generate_boxes(box->rfd) &&
		generate_boxes(box->lbd) &&
		generate_boxes(box->rbd) &&
		generate_boxes(box->lfu) &&
		generate_boxes(box->rfu) &&
		generate_boxes(box->lbu) &&
		generate_boxes(box->rbu) );

	return(flag);
   }
}

/*
 * Function:	find_box
 * action:	finds all solutions, nodes and singularities for a box
 *		together with the topoligical linkage information.
 */

int find_box(box)
box_info *box;
{
	find_all_faces(box);
	if( !link_nodes(box,box) ) return(FALSE);
	collect_sings(box);
	box->status = FOUND_EVERYTHING ;
	draw_box(box);
#ifdef PRINT_ODD_BOX
	if(box->sings != NULL && box->sings->next != NULL)
		printbox(box);
#endif
#ifdef DO_FREE
/*
	free_bits_of_box(box);
*/
	free_box(box);
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

void find_all_faces(box)
box_info *box;
{
	get_existing_faces(box);
	create_new_faces(box);

	/* none of the faces are internal */

	find_face(box,box->ll,FACE_LL,FALSE);
	find_face(box,box->rr,FACE_RR,FALSE);
	find_face(box,box->ff,FACE_FF,FALSE);
	find_face(box,box->bb,FACE_BB,FALSE);
	find_face(box,box->dd,FACE_DD,FALSE);
	find_face(box,box->uu,FACE_UU,FALSE);
}

/*
 * Function:	find_face
 * action:	find all the information about solutions and nodes on face.
 */

void find_face(box,face,code,internal)
box_info *box;
face_info *face;
int code,internal;
{
	if(face->status == FOUND_EVERYTHING ) return;
/*
	fprintf(stderr,"find_face: (%d,%d,%d)/%d\n",
		face->xl,face->yl,face->zl,face->denom);
*/
	find_all_edges(box,face,code);

	link_face(face,face,internal);
	if( !internal ) colect_nodes(box,face);
	face->status = FOUND_EVERYTHING;
}

/*
 * Function:	find_all_edges
 * action:	finds all the solutions on the edges of a face.
 *		uses the information already found from adjacient faces.
 */

void find_all_edges(box,face,code)
box_info *box;
face_info *face;
int code;
{
	get_existing_edges(box,face,code);
	create_new_edges(face);
/*
fprintf(stderr,"find_all_edges\n");
printedge(face->x_low);
fprintf(stderr,"low %p high %p\n",face->x_low->low,face->x_low->high);
printedge(face->x_high);
fprintf(stderr,"low %p high %p\n",face->x_high->low,face->x_high->high);
printedge(face->y_low);
printedge(face->x_high);
*/
	find_edge(face->x_low,X_LOW);
	find_edge(face->x_high,X_HIGH);
	find_edge(face->y_low,Y_LOW);
	find_edge(face->y_high,Y_HIGH);
}

/*
 * Function:	find_edge
 * action:	finds all the solutions on an edge.
 */

void find_edge(edge,code)
edge_info *edge;
int code;
{
	if( edge->status == FOUND_EVERYTHING ) return;
	find_sols_on_edge(edge);
	edge->status = FOUND_EVERYTHING;
}

/*
 * Function:	find_sols_on_edge
 * action:	finds all the solutions on the edge
 */

#define SameSign(A,B) ( A > 0.0 ? ( B > 0.0 ? 1 : 0 ) : ( B < 0.0 && A < 0.0 ? 1 : 0 ) )

void find_sols_on_edge(edge)
edge_info *edge;
{
	double vall,valm,valh;
	double rootl,rooth,rootm;
	long level;
	double	xl,xh,yl,yh,zl,zh;
	int     count;

	edge->status = FOUND_EVERYTHING;

if( edge->xl * edge->low->denom != edge->low->xl * edge->denom
 || edge->yl * edge->low->denom != edge->low->yl * edge->denom
 || edge->zl * edge->low->denom != edge->low->zl * edge->denom )
{
	fprintf(stderr,"Vert does not match edge\n");
	printedge(edge);
	fprintf(stderr,"Vert: "); printvert(edge->low);
}

	xl = EvalEdgeDerivX(edge,0.0);
	xh = EvalEdgeDerivX(edge,1.0);
	yl = EvalEdgeDerivY(edge,0.0);
	yh = EvalEdgeDerivY(edge,1.0);
	zl = EvalEdgeDerivZ(edge,0.0);
	zh = EvalEdgeDerivZ(edge,1.0);

	if( edge->denom < SUPER_FINE && 
	    ( ! SameSign(xl,xh) || ! SameSign(yl,yh) || ! SameSign(zl,zh) ) )
	{
		/* Derivatives change sign recurse*/

#ifdef NOT_DEF
		edge->left = allocedge();
		edge->right = allocedge();
		subdevideedge(edge,edge->left,edge->right);
#endif
		split_edge(edge);

		find_sols_on_edge(edge->left);
		find_sols_on_edge(edge->right);
#ifdef NOT_DEF
		if( *(aa.r->array) == 0.0 )
		{
			edge->sol = make_sol(edge->type,edge->xl,
				edge->yl,edge->zl,edge->denom, 0.5 );
			edge->sol->dx = f1;
			edge->sol->dy = f2;
			edge->sol->dz = f3;
		}
#endif 
		return;
	}

	vall = EvalEdge(edge,0.0);
	valh = EvalEdge(edge,1.0);

	if( SameSign(vall,valh) ) return;

	count = 0;

	/*      For the convergence routine we use a 
		highbrid method finding where the cord
		crosses 0 and the mid point. This is
		expensive as there are two function
		evaluations, but ensures that the interval
		is always reduced.
	*/

	/*** A simple interval ***/

	rootl = 0.0;
	rooth = 1.0;
	level = (long) edge->denom;
	while( level <= MAX_EDGE_LEVEL )
	{
		level *= 2;
		rootm = (rootl +rooth) * 0.5;
		valm = EvalEdge(edge, rootm);
		if((vall<0) != (valm<0)) rooth = rootm;
		else
		{
			vall = valm; 
			rootl = rootm;
		}
	}

	edge->sol = make_sol(edge->type,edge->xl,edge->yl,edge->zl,
		edge->denom, rootm );
	edge->sol->dx = EvalEdgeDerivX(edge,rootm);
	edge->sol->dy = EvalEdgeDerivY(edge,rootm);
	edge->sol->dz = EvalEdgeDerivZ(edge,rootm);

#ifdef PRINT_FIND_EDGE_SOL
	fprintf(stderr,"find_sol_on_edge: f1 %f f2 %f f3 %f\n",f1,f2,f3);
	print_sol(edge->sol);
#endif
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

/****		First some macros	****/

#define PairTest(a,b) (\
	   sols[a]->dx == sols[b]->dx \
	&& sols[a]->dy == sols[b]->dy \
	&& sols[a]->dz == sols[b]->dz )

#define MatchDeriv(a) (\
	   sols[a]->dx == f1 \
	&& sols[a]->dy == f2 \
	&& sols[a]->dz == f3 )

#define AddLink(a,b) {\
	if( !internal ) \
		include_link(big_face,sols[a],sols[b]); }

#define AddNode { \
		sols[4] = make_sol2(face->type, \
			face->xl,face->yl,face->zl,face->denom,pos_x,pos_y); \
		sols[4]->dx = EvalFaceDerivX(face,pos_x,pos_y); \
		sols[4]->dy = EvalFaceDerivY(face,pos_x,pos_y); \
		sols[4]->dz = EvalFaceDerivZ(face,pos_x,pos_y); \
		add_node(big_face,sols[4]); }

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
	} } \

void ReduceFace(big_face,face,internal)
face_info *big_face,*face;
{
	face->lb = allocface();
	face->rb = allocface();
	face->lt = allocface();
	face->rt = allocface();

	make_sub_faces(face,face->lb,face->rb,face->lt,face->rt);
	split_face(face,face->lb,face->rb,face->lt,face->rt);

	find_edge(face->lb->x_high,X_HIGH);
	find_edge(face->lb->y_high,Y_HIGH);
	find_edge(face->rt->x_low,X_LOW);
	find_edge(face->rt->y_low,Y_LOW);

	link_face(big_face,face->lb,internal);
	link_face(big_face,face->rb,internal);
	link_face(big_face,face->lt,internal);
	link_face(big_face,face->rt,internal);
}

void link_face(big_face,face,internal)
face_info *big_face,*face;
int internal;
{
	int count,i;
	sol_info *sols[5];
	double vec[2],pos_x=0.0,pos_y=0.0;
	double	xll,xlh,xhl,xhh;
	double	yll,ylh,yhl,yhh;
	double	zll,zlh,zhl,zhh;

	count = get_sols_on_face(face,sols);

#ifdef PRINT_LINK_FACE_ALL
	fprintf(stderr,"link_face: ");
	printsoltype(face->type);
	fprintf(stderr," (%d,%d,%d)/%d count %d f1 %d f2 %d f3 %d\n",
		face->xl,face->yl,face->zl,face->denom,
		count,f1,f2,f3);
#endif

	/* calc derivs at each corner */

	xll = EvalEdgeDerivX(face->x_low,0.0);
	xlh = EvalEdgeDerivX(face->x_low,1.0);
	xhl = EvalEdgeDerivX(face->x_high,0.0);
	xhh = EvalEdgeDerivX(face->x_high,1.0);
	yll = EvalEdgeDerivY(face->x_low,0.0);
	ylh = EvalEdgeDerivY(face->x_low,1.0);
	yhl = EvalEdgeDerivY(face->x_high,0.0);
	yhh = EvalEdgeDerivY(face->x_high,1.0);
	zll = EvalEdgeDerivZ(face->x_low,0.0);
	zlh = EvalEdgeDerivZ(face->x_low,1.0);
	zhl = EvalEdgeDerivZ(face->x_high,0.0);
	zhh = EvalEdgeDerivZ(face->x_high,1.0);

	if( count == 0 )
	{
		if( ( ( xll > 0.0 && xlh > 0.0 && xhl > 0.0 && xhh > 0.0 )
		    ||( xll < 0.0 && xlh < 0.0 && xhl < 0.0 && xhh < 0.0 ) )
		 && ( ( yll > 0.0 && ylh > 0.0 && yhl > 0.0 && yhh > 0.0 )
		    ||( yll < 0.0 && ylh < 0.0 && yhl < 0.0 && yhh < 0.0 ) )
		 && ( ( zll > 0.0 && zlh > 0.0 && zhl > 0.0 && zhh > 0.0 )
		    ||( zll < 0.0 && zlh < 0.0 && zhl < 0.0 && zhh < 0.0 ) ) )
			goto fini_link_face;

	}
	else if( count == 2 )
	{
		if( ( ( xll > 0.0 && xlh > 0.0 && xhl > 0.0 && xhh > 0.0 )
		    ||( xll < 0.0 && xlh < 0.0 && xhl < 0.0 && xhh < 0.0 ) )
		 && ( ( yll > 0.0 && ylh > 0.0 && yhl > 0.0 && yhh > 0.0 )
		    ||( yll < 0.0 && ylh < 0.0 && yhl < 0.0 && yhh < 0.0 ) )
		 && ( ( zll > 0.0 && zlh > 0.0 && zhl > 0.0 && zhh > 0.0 )
		    ||( zll < 0.0 && zlh < 0.0 && zhl < 0.0 && zhh < 0.0 ) ) )
		{
			/* now check signs of derivatives */

			if( ( ( xll > 0.0 && sols[0]->dx > 0.0
				&& sols[1]->dx > 0.0 )
			    ||( xll > 0.0 && sols[0]->dx > 0.0
				&& sols[1]->dx > 0.0 ) )
			  &&( ( yll > 0.0 && sols[0]->dy > 0.0
				&& sols[1]->dy > 0.0 )
			    ||( yll > 0.0 && sols[0]->dy > 0.0
				&& sols[1]->dy > 0.0 ) )
			  &&( ( zll > 0.0 && sols[0]->dz > 0.0
				&& sols[1]->dz > 0.0 )
			    ||( zll > 0.0 && sols[0]->dz > 0.0
				&& sols[1]->dz > 0.0 ) ) )
			{
				AddLink(0,1);
	                        goto fini_link_face;
			}		     
		}
	}

	if( face->denom < LINK_FACE_LEVEL )
	{
		ReduceFace(big_face,face,internal);
		goto fini_link_face;
	}

	if( count == 0 )
	{
		/* Assume no sol in centre */

	}
	else if( count == 2 )
	{

		/* Not got the propper tests yet
			just test to see if change in signs
			of derivatives of solutions	*/

		if( ( ( sols[0]->dx > 0.0 && sols[1]->dx > 0.0 )
		    ||( sols[0]->dx < 0.0 && sols[1]->dx < 0.0 ) )
		  &&( ( sols[0]->dy > 0.0 && sols[1]->dy > 0.0 )
		    ||( sols[0]->dy < 0.0 && sols[1]->dy < 0.0 ) )
		  &&( ( sols[0]->dz > 0.0 && sols[1]->dz > 0.0 )
		    ||( sols[0]->dz < 0.0 && sols[1]->dz < 0.0 ) ) )
		{
			AddLink(0,1);
		}
		else
		{
			GetMid;
			AddNode;
			AddLink(0,4);
			AddLink(1,4);
#ifdef PRINT_LINK_FACE
	{
	fprintf(stderr,"link_face: count %d\n", count);
	print_sol(sols[4]);
	print_sol(sols[0]);
	print_sol(sols[1]);
	}
#endif
		}
	}

	else if( count == 3 )
	{
		sols[2] = get_nth_sol_on_face(face,3);
		GetMid;
/*
		if( pos_x != pos_x || pos_y != pos_y )
			fprintf(stderr,"pos_x %f pos_y %f\n",pos_x,pos_y);
*/
		AddNode;
		AddLink(0,4);
		AddLink(1,4);
		AddLink(2,4);
#ifdef PRINT_LINK_FACE
	fprintf(stderr,"link_face: count %d \n", count);
	print_sol(sols[4]);
	print_sol(sols[0]);
	print_sol(sols[1]);
	print_sol(sols[2]);
#endif

	}

	else if( count == 4 )
	{
		sols[2] = get_nth_sol_on_face(face,3);
		sols[3] = get_nth_sol_on_face(face,4);
		GetMid;
		AddNode;
		AddLink(0,4);
		AddLink(1,4);
		AddLink(2,4);
		AddLink(3,4);
#ifdef PRINT_LINK_FACE
	fprintf(stderr,"link_face: count %d\n", count);
	print_sol(sols[4]);
	print_sol(sols[0]);
	print_sol(sols[1]);
	print_sol(sols[2]);
	print_sol(sols[3]);
#endif
	}

	else
	{
		GetMid;
/*
		if( pos_x != pos_x || pos_y != pos_y )
			fprintf(stderr,"pos_x %f pos_y %f\n",pos_x,pos_y);
*/
		AddNode;
#ifdef PRINT_LINK_FACE
	fprintf(stderr,"link_face: ");
	printsoltype(face->type);
	fprintf(stderr," (%d,%d,%d)/%d count %d \n",
		face->xl,face->yl,face->zl,face->denom, count);
#endif
		for(i=1;i<=count;++i)
		{
			sols[0] = get_nth_sol_on_face(face,i);
			AddLink(0,4);
#ifdef PRINT_LINK_FACE
/*
			print_sol(sols[0]);
*/
#endif
		}
	}

	fini_link_face:

	return;
}


/*
 * Function:    link_nodes(box,big_box)
 * action:      links together the nodes surronding a box.
 *	      adds the links to the list in big_box.
 *		Returns FALSE on interupt
 */

#define MatchNodes(a,b) (\
	   nodes[a]->sol->dx == nodes[b]->sol->dx \
	|| nodes[a]->sol->dy == nodes[b]->sol->dy \
	|| nodes[a]->sol->dz == nodes[b]->sol->dz )

int link_nodes(big_box,box)
box_info *big_box,*box;
{
	int count,i,num_use_nodes = 0;
	double pos_x,pos_y,pos_z,vec0[3];
	node_info *nodes[4];
	int flag;

	/* calc derivs at each corner */

	if( check_interupt( NULL ) ) return(FALSE);

	count = get_nodes_on_box_faces(box,nodes);

        if( count == 0 )
        {
                sol_info *sols[2];

                /* test for isolated zeros: require !f1 !f2 !f3 and */
                /* no solutions on faces.                           */

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

	reduce_nodes:

	/*** Too dificult to handle, either sub-devide or create a node ***/

	if( box->denom >= LINK_SING_LEVEL )
	{
		sol_info *sol;

		/* Assume count = 0  implies no soln */

		if( count == 0 ) goto fini_nodes; 

		if( count == 2 )
		{
			/* For count == 2 just assume a simple link */

			add_node_link(big_box,nodes[0],nodes[1]);
			goto fini_nodes;
		}

#ifdef PRINT_LINK_NODES
		fprintf(stderr,"link_nodes: count = %d BOX (%d,%d,%d)/%d\n",
			count,box->xl,box->yl,box->zl,box->denom);
		printbox(box);
#endif
		pos_x = pos_y = pos_z = 0.0;
		for( i=1; i<=count; ++i)
		{
			nodes[1] = get_nth_node_on_box(box,i);
#ifdef PRINT_LINK_NODES
			print_node(nodes[1]);
#endif
			calc_pos_in_box(box,nodes[1]->sol,vec0);
			pos_x +=  vec0[0];
			pos_y +=  vec0[1];
			pos_z +=  vec0[2];
		}
		if( count == 0 ) pos_x = pos_y = pos_z = 0.5;
		else
		{
			pos_x /= count;
			pos_y /= count;
			pos_z /= count;
		}

		sol = make_sol3(BOX,box->xl,box->yl,box->zl,box->denom,
			pos_x,pos_y,pos_z );
		sol->dx = EvalBoxDerivX(box,pos_x,pos_y,pos_z);
		sol->dy = EvalBoxDerivY(box,pos_x,pos_y,pos_z);
		sol->dz = EvalBoxDerivZ(box,pos_x,pos_y,pos_z);

		add_sing(big_box,sol);
		if( count == 0 ) goto fini_nodes;
		nodes[0] = allocnode();
		nodes[0]->next = NULL;
		nodes[0]->sol = sol;
		nodes[0]->status = NODE;
		for( i=1; i<=count; ++i)
		{
			nodes[1] = get_nth_node_on_box(box,i);
			add_node_link(big_box,nodes[0],nodes[1]);
		}
		goto fini_nodes;
	}
	else
	{
		sub_devide_box(box);
		split_box(box,box->lfd,box->rfd,box->lbd,box->rbd,
				 box->lfu,box->rfu,box->lbu,box->rbu);

		find_face(box->lfd,box->lfd->rr,FACE_RR,TRUE);
		find_face(box->lfd,box->lfd->bb,FACE_BB,TRUE);
		find_face(box->lfd,box->lfd->uu,FACE_UU,TRUE);
		box->lfd->status = FOUND_FACES;

		find_face(box->rbd,box->rbd->ll,FACE_LL,TRUE);
		find_face(box->rbd,box->rbd->ff,FACE_FF,TRUE);
		find_face(box->rbd,box->rbd->uu,FACE_UU,TRUE);
		box->rbd->status = FOUND_FACES;

		find_face(box->rfu,box->rfu->ll,FACE_LL,TRUE);
		find_face(box->rfu,box->rfu->bb,FACE_BB,TRUE);
		find_face(box->rfu,box->rfu->dd,FACE_DD,TRUE);
		box->rfu->status = FOUND_FACES;
		box->rfd->status = FOUND_FACES;

		find_face(box->lbu,box->lbu->rr,FACE_RR,TRUE);
		find_face(box->lbu,box->lbu->ff,FACE_FF,TRUE);
		find_face(box->lbu,box->lbu->dd,FACE_DD,TRUE);
		box->lbu->status = FOUND_FACES;
		box->lfu->status = FOUND_FACES;
		box->lbd->status = FOUND_FACES;
		box->rbu->status = FOUND_FACES;

		flag = 
		   link_nodes(big_box,box->lfd)
		&& link_nodes(big_box,box->rfd)
		&& link_nodes(big_box,box->lbd)
		&& link_nodes(big_box,box->rbd)
		&& link_nodes(big_box,box->lfu)
		&& link_nodes(big_box,box->rfu)
		&& link_nodes(big_box,box->lbu)
		&& link_nodes(big_box,box->rbu);
#ifdef FACETS
		combine_facets(box);
#endif

		return(flag);
	}

	fini_nodes:
#ifdef FACETS
		make_facets(box);
#endif
	return(TRUE);
}

void calc_pos_norm(sol,vec,norm)
sol_info *sol;
double vec[3],norm[3];
{
	calc_pos(sol,vec);
	norm[0] = sol->dx;
	norm[1] = sol->dy;
	norm[2] = sol->dz;
}

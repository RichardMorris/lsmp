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
#include "bern.h"
#include "cells.h"
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
/*
#include <sys/resource.h>
*/

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
#define PRINT_FIND_EDGE_SOLS
#define PRINT_SOL_VAL_ERR
#define PRINT_LINK_FACE_ALL
#define VERBOUSE
#define OLD_CROSS
#define PRINT_LINK_FACE
#define PRINT_LINK_NODES
#define PRI_MALL
#define PRINT_LINK_NODES_ALL
#define PRINT_GEN_BOXES
*/
#define NON_GENERIC_NODES
#define DO_FREE
#define LINK_SING
#define LINK_SOLS
#define LINK_FACE
#define DO_PLOT

#define MAX_EDGE_LEVEL 1048576 /* 32768 */
unsigned int LINK_FACE_LEVEL = 1024, LINK_SING_LEVEL = 512, RESOLUTION = 16;
unsigned int SUPER_FINE = 8192;

#define EMPTY 0
#define FOUND_EVERYTHING 2
#define FOUND_FACES 3

/*** A value returned by follow when there a sol not on an edge is found. ***/

#define NEW_NODE 2

#define grballoc(node) ( node * ) malloc( sizeof(node) )

extern sol_info *get_nth_sol_on_face();
extern node_info *get_nth_node_on_box();

box_info whole_box;
bern3D *AA,*BB;		/* The two main bernstein polys */
bern3D *CC,*DD,*EE;	/* Componants of product poly */
/*
bern3D d2b_dx2,d2b_dxdy,d2b_dxdz,d2b_dy2,d2b_dydz,d2b_dz2;
*/
region_info region;

/*********************** Start of Sub-routines **************************/
/*									*/
/* draws an arbitrary polynomial curve in a specified region of space   */
/*									*/
/*********************** Start of Sub-routines **************************/

#ifdef PRI_MALL
print_mallinfo()
{
	struct mallinfo info;
	struct rusage   usage;
	FILE	*fp;
	int	c;

	system("ps -l | grep int > zap");
	fp = fopen("zap","r");
	while( ( c = fgetc(fp) ) != EOF ) fputc(c,stderr);
	fclose(fp);
	info = mallinfo();
	fprintf(stderr,"arena %d ordblks %d smblks %d hblkhd %d hblks %d\n",
		info.arena,info.ordblks,info.smblks,info.hblkhd,info.hblks);
	fprintf(stderr,"usmblks %d fsmblks %d uordblks %d fordblks %d keepcost %d\n",
		info.usmblks,info.fsmblks,info.uordblks,info.fordblks,info.keepcost);

	getrusage(RUSAGE_SELF,&usage);
	fprintf(stderr,"utime %d.%d stime %d.%d maxrss %d ru_nswap %d\n",
		usage.ru_utime.tv_sec,usage.ru_utime.tv_usec,
		usage.ru_stime.tv_sec,usage.ru_stime.tv_usec,
		usage.ru_maxrss,usage.ru_nswap);
}
#endif

/*
 * The main routine	*******************************************
 */

int marmain(aa,bb,cc,dd,ee,xmin,xmax,ymin,ymax,zmin,zmax)
double aa[MAXORDER][MAXORDER][MAXORDER];  /* the input polynomial */
double bb[MAXORDER][MAXORDER][MAXORDER];  /* the input polynomial */
double cc[MAXORDER][MAXORDER][MAXORDER];  /* the input polynomial */
double dd[MAXORDER][MAXORDER][MAXORDER];  /* the input polynomial */
double ee[MAXORDER][MAXORDER][MAXORDER];  /* the input polynomial */
double xmin,xmax,ymin,ymax,zmin,zmax;
{
  int flag;

  region.xmin = xmin;
  region.ymin = ymin;
  region.zmin = zmin;
  region.xmax = xmax;
  region.ymax = ymax;
  region.zmax = zmax;

#ifdef PRI_MALL
  print_mallinfo();
#endif
  AA = formbernstein3D(aa,xmin,xmax,ymin,ymax,zmin,zmax);
  BB = formbernstein3D(bb,xmin,xmax,ymin,ymax,zmin,zmax);
  CC = formbernstein3D(cc,xmin,xmax,ymin,ymax,zmin,zmax);
  DD = formbernstein3D(dd,xmin,xmax,ymin,ymax,zmin,zmax);
  EE = formbernstein3D(ee,xmin,xmax,ymin,ymax,zmin,zmax);

  init_berns(AA,BB,CC,DD,EE);
/*
  init_cells();
*/

#ifdef PRI_MALL
  print_mallinfo();
#endif
#ifdef VERBOUSE
  fprintf(stderr,"range %f %f %f %f %f %f\n",
  	xmin,xmax,ymin,ymax,zmin,zmax);
  fprintf(stderr,"Bernstein polynomials are:\n");
  printbern3D(AA);
  printbern3D(BB);
  printbern3D(CC);
  printbern3D(DD);
  printbern3D(EE);
#endif
  make_box(&whole_box,0,0,0,1);
  generate_boxes(&whole_box,AA,BB,CC,DD,EE);
#ifdef PRI_MALL
  print_mallinfo();
#endif
  free_bern3D(AA);
  free_bern3D(BB);
  free_bern3D(CC);
  free_bern3D(DD);
  free_bern3D(EE);
  flag = check_interupt("Writing Data");
  return(!flag);
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

int generate_boxes(box,aa,bb,cc,dd,ee)
box_info *box;
bern3D *aa,*bb,*cc,*dd,*ee;
{
	int xl,yl,zl,denom;
	double percent = 0.0;
	int flag;
	octbern3D a1,b1,c1,d1,e1;
	char	string[40];

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

   if( allonesign3D(aa) || allonesign3D(bb) ) /* no componant in box */
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

	return(find_box(box,aa,bb,cc,dd,ee));
   }
   else
   {		/**** Sub-devide the region into 8 sub boxes.  ****/

#ifdef PRINT_GEN_BOXES
   	fprintf(stderr,"generate_boxes: box (%d,%d,%d)/%d NODE\n",
		box->xl,box->yl,box->zl,box->denom);
#endif

	a1 = reduce3D(aa);
	b1 = reduce3D(bb);
	c1 = reduce3D(cc);
	d1 = reduce3D(dd);
	e1 = reduce3D(ee);
	sub_devide_box(box);

	flag =
		generate_boxes(box->lfd,a1.lfd,b1.lfd,c1.lfd,d1.lfd,e1.lfd) &&
		generate_boxes(box->rfd,a1.rfd,b1.rfd,c1.rfd,d1.rfd,e1.rfd) &&
		generate_boxes(box->lbd,a1.lbd,b1.lbd,c1.lbd,d1.lbd,e1.lbd) &&
		generate_boxes(box->rbd,a1.rbd,b1.rbd,c1.rbd,d1.rbd,e1.rbd) &&
		generate_boxes(box->lfu,a1.lfu,b1.lfu,c1.lfu,d1.lfu,e1.lfu) &&
		generate_boxes(box->rfu,a1.rfu,b1.rfu,c1.rfu,d1.rfu,e1.rfu) &&
		generate_boxes(box->lbu,a1.lbu,b1.lbu,c1.lbu,d1.lbu,e1.lbu) &&
		generate_boxes(box->rbu,a1.rbu,b1.rbu,c1.rbu,d1.rbu,e1.rbu);

	free_octbern3D(a1);
	free_octbern3D(b1);
	free_octbern3D(c1);
	free_octbern3D(d1);
	free_octbern3D(e1);

	return(flag);
   }
}

/*
 * Function:	find_box
 * action:	finds all solutions, nodes and singularities for a box
 *		together with the topoligical linkage information.
 */

int find_box(box,aa,bb,cc,dd,ee)
box_info *box;
bern3D *aa,*bb,*cc,*dd,*ee;
{

	find_all_faces(box,aa,bb,cc,dd,ee);
	if( !link_nodes(box,box,aa,bb,cc,dd,ee) ) return(FALSE);
	collect_sings(box);
	box->status = FOUND_EVERYTHING ;
	draw_box(box);
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

find_all_faces(box,aa,bb,cc,dd,ee)
box_info *box;
bern3D *aa,*bb,*cc,*dd,*ee;
{
	get_existing_faces(box);
	create_new_faces(box);

	/* none of the faces are internal */

	find_face(box,aa,bb,cc,dd,ee,box->ll,FACE_LL,FALSE);
	find_face(box,aa,bb,cc,dd,ee,box->rr,FACE_RR,FALSE);
	find_face(box,aa,bb,cc,dd,ee,box->ff,FACE_FF,FALSE);
	find_face(box,aa,bb,cc,dd,ee,box->bb,FACE_BB,FALSE);
	find_face(box,aa,bb,cc,dd,ee,box->dd,FACE_DD,FALSE);
	find_face(box,aa,bb,cc,dd,ee,box->uu,FACE_UU,FALSE);
}

/*
 * Function:	find_face
 * action:	find all the information about solutions and nodes on face.
 */

find_face(box,aa,bb,cc,dd,ee,face,code,internal)
box_info *box;
bern3D *aa,*bb,*cc,*dd,*ee;
face_info *face;
int code,internal;
{
	bern2D *a,*b,*c,*d,*e;
	bern3D temp;

	if(face->status == FOUND_EVERYTHING ) return;
	a = make_bern2D_of_box(aa,code);
	b = make_bern2D_of_box(bb,code);
	if( allonesign2D(a) || allonesign2D(b))
	{
		face->status = FOUND_EVERYTHING;
		free_bern2D(a);
		free_bern2D(b);
		return;
	}
	c = make_bern2D_of_box(cc,code);
	d = make_bern2D_of_box(dd,code);
	e = make_bern2D_of_box(ee,code);
	link_face(face,face,a,b,c,d,e,internal);
	if( !internal ) colect_nodes(box,face);
	face->status = FOUND_EVERYTHING;
	free_bern2D(a);
	free_bern2D(b);
	free_bern2D(c);
	free_bern2D(d);
	free_bern2D(e);
}

/*
 * Function:	find_all_edges
 * action:	finds all the solutions on the edges of a face.
 *		uses the information already found from adjacient faces.
 */

find_all_edges(box,face,bb,code)
box_info *box;
face_info *face;
bern2D *bb;
int code;
{
	get_existing_edges(box,face,code);
	create_new_edges(face);
	find_edge(face->x_low,bb,X_LOW);
	find_edge(face->x_high,bb,X_HIGH);
	find_edge(face->y_low,bb,Y_LOW);
	find_edge(face->y_high,bb,Y_HIGH);
}

/*
 * Function:	find_edge
 * action:	finds all the solutions on an edge.
 */

find_edge(edge,bb,code)
edge_info *edge;
bern2D *bb;
int code;
{
	bern1D *aa;

	if( edge->status == FOUND_EVERYTHING ) return;
	aa = make_bern1D_of_face(bb,code);
	find_sols_on_edge(edge,aa);
	edge->status = FOUND_EVERYTHING;
	free_bern1D(aa);
}

/*
 * Function:	find_sols_on_edge
 * action:	finds all the solutions on the edge
 */

find_sols_on_edge(edge,bb)
edge_info *edge;
bern1D *bb;
{
	double vall,valm;
        double rootl,rooth,rootm;
        bern1D *db_dt;
        long level;
	binbern1D b1;


	edge->status = FOUND_EVERYTHING;
        if( allonesign1D(bb) ) return;

        db_dt = diff1D(bb);
        if( !allonesign1D(db_dt) )
        {
            if( edge->denom >= SUPER_FINE )
            {
		if( *(bb->array) * *(bb->array+bb->ord) > 0 ) return;
		edge->sol = make_sol(edge->type,edge->xl,
				edge->yl,edge->zl,edge->denom,BAD_EDGE ); 
#ifdef PRINT_FIND_EDGE_SOLS
                fprintf(stderr,"Error: find_sols_on_edge: could not find solution on edge\n"
);
                printedge(edge);
                for(level=0;level<=bb->ord;++level)
			fprintf(stderr,"%11.5g ",*(bb->array+level));
                fprintf(stderr,"\n");
#endif
		free_bern1D(db_dt);
                return;
            }
	    else
	    {
		b1 = reduce1D(bb);
		edge->left = alloc_edge();
		edge->right = alloc_edge();
		subdevideedge(edge,edge->left,edge->right);

 		find_sols_on_edge(edge->left,b1.l);
		find_sols_on_edge(edge->right,b1.l);

		if( *(b1.l->array) == 0.0 )
			edge->sol = make_sol(edge->type,edge->xl,
				edge->yl,edge->zl,edge->denom, 0.5 );
		free_bern1D(db_dt);
		free_binbern1D(b1);
        	return;
	    }
	}

        /*** Now have an interval with only one solution, find it ***/

        vall = *(bb->array);
        rootl = 0.0;
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

	edge->sol = make_sol(edge->type,edge->xl,edge->yl,edge->zl,
		edge->denom, rootm );
	free_bern1D(db_dt);
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

#define  no_second_derivatives_vanish TRUE

link_face(big_face,face,aa,bb,cc,dd,ee,internal)
face_info *big_face,*face;
bern2D *aa,*bb,*cc,*dd,*ee;
int internal;
{
	int f1,f2,f3,count,i,j,num_use_sols = 0,use_sols[4];
	int a_count, b_count;
	sol_info *sols[4], *a_sols[4], *b_sols[4];
	double pos_x,pos_y,lam;
	double vec0[2],vec1[2],vec2[2],vec3[2];
	double v0[3], v1[3], res;
	face_info *a_face, *b_face;
	quadbern2D a1,b1,c1,d1,e1;

	if( allonesign2D(aa) || allonesign2D(bb) ) return;

	f1 = allonesign2D(cc);
	f2 = allonesign2D(dd);
	f3 = allonesign2D(ee);	

#ifdef PRINT_LINK_FACE_ALL
		fprintf(stderr,"link_face: f1 %d f2 %d f3 %d ",f1,f2,f3);
		printsoltype(face->type);
		fprintf(stderr,"(%d,%d,%d)/%d\n",face->xl,face->yl,face->zl,face->denom);
#endif
	if( face->denom >= LINK_FACE_LEVEL )
	{
		/*** Posibly a node must first find if solutions cross ***/

		a_face = alloc_face();
		b_face = alloc_face();

		make_face(a_face,face->type,
			face->xl,face->yl,face->zl,face->denom);
		make_face(b_face,face->type,
			face->xl,face->yl,face->zl,face->denom);
		create_new_edges(a_face);
		create_new_edges(b_face);

		find_edge(a_face->x_low,aa,X_LOW);
		find_edge(a_face->x_high,aa,X_HIGH);
		find_edge(a_face->y_low,aa,Y_LOW);
		find_edge(a_face->y_high,aa,Y_HIGH);
		find_edge(b_face->x_low,bb,X_LOW);
		find_edge(b_face->x_high,bb,X_HIGH);
		find_edge(b_face->y_low,bb,Y_LOW);
		find_edge(b_face->y_high,bb,Y_HIGH);

		a_count = get_sols_on_face(a_face,a_sols);
		b_count = get_sols_on_face(b_face,b_sols);

		pos_x = pos_y = 0.5;
		if(a_count==2 && b_count == 2)
		{
			/* Now try and find where lines cross */

			calc_pos_on_face(face,a_sols[0],vec0);
			calc_pos_on_face(face,a_sols[1],vec1);
			calc_pos_on_face(face,b_sols[0],vec2);
			calc_pos_on_face(face,b_sols[1],vec3);

			lam = -( (vec3[1]-vec2[1])*(vec0[0]-vec2[0])
			       -(vec3[0]-vec2[0])*(vec0[1]-vec2[1]) )
			     /((vec3[1]-vec2[1]) * (vec1[0] - vec0[0])
			       -(vec3[0]-vec2[0]) * (vec1[1]-vec0[1]));

#ifdef OLD_CROSS
			calc_pos(a_sols[0],v0);
			calc_pos(a_sols[1],v1);
			res = evalbern3D(BB,v0) * evalbern3D(BB,v1);

			if( (res < 0.0 &&(lam != lam || lam < 0.0 || lam > 1.0))
			 || ( res >= 0.0 && lam >= 0.0 && lam <= 1.0) )
			{
				fprintf(stderr,"res %f lam %f f1 %d f2 %d f3 %d\n",
					res,lam,f1,f2,f3);
				fprintf(stderr,"A %f %f   %f\n",
					vec0[0],vec0[1],evalbern3D(BB,v0));
				fprintf(stderr,"B %f %f   %f\n",
					vec1[0],vec1[1],evalbern3D(BB,v1));
			calc_pos(b_sols[0],v0);
			calc_pos(b_sols[1],v1);
				fprintf(stderr,"C %f %f   %f\n",
					vec2[0],vec2[1],evalbern3D(AA,v0));
				fprintf(stderr,"D %f %f   %f\n",
					vec3[0],vec3[1],evalbern3D(AA,v0));
			}
#endif
			
		/* if curves don't cross and at least one
			of derivatives is non-zero then no intersection */

			if( (lam != lam || lam < 0.0 || lam > 1.0 )
			 && (f1 || f2 || f3 ) ) 
			{
				free_face(a_face);
				free_face(b_face);
				return;
			}

			if( lam >= 0.0 && lam <= 1.0 )
			{
				pos_x = lam * vec1[0] + (1.0-lam)*vec0[0];
				pos_y = lam * vec1[1] + (1.0-lam)*vec0[1];
			}
		}

		/*** Must be a node ***/
		/*** Happens if if number of solutions for A or B
			differ from two,
			or if two solution curves cross,
			or if all derivatives are zero ***/

#ifdef PRINT_LINK_FACE
		fprintf(stderr,"link_face f1 %d f2 %d f3 %d a_count %d b_count %d posx %f posy %f\n",
			f1,f2,f3,a_count,b_count,pos_x,pos_y);
/*
		printface(a_face);
		printface(b_face);
*/
#endif

		sols[0] = make_sol2(face->type,
				face->xl,face->yl,face->zl,
				face->denom, pos_x,pos_y );
		sols[0]->dx = f1;
		sols[0]->dy = f2;
		sols[0]->dz = f3;
#ifdef PRINT_LINK_FACE
		fprintf(stderr,"Sol ");
		print_sol(sols[0]);
		calc_pos(sols[0],v0);
		fprintf(stderr,"Vals %f %f\n",
			evalbern3D(AA,v0),evalbern3D(BB,v0));
/*
		printbern2D(aa);
		printbern2D(bb);
*/
#endif
		add_node(big_face,sols[0]);
		if(pos_x == 0.0 || pos_x == 1.0 || pos_y == 0.0 || pos_y== 1.0 )
		{
#ifdef PRINT_LINK_FACE
		fprintf(stderr,"Face ");
		print_sol(sols[0]);
		fprintf(stderr,"link_face f1 %d f2 %d f3 %d a_count %d b_count %d\n",
			f1,f2,f3,a_count,b_count);
/*
		printface(a_face);
		printface(b_face);
*/
#endif
		}
			
		free_face(a_face);
		free_face(b_face);
		return;
	}
	else
	{
		face->lb = alloc_face();
		face->rb = alloc_face();
		face->lt = alloc_face();
		face->rt = alloc_face();

		a1 = reduce2D(aa);
		b1 = reduce2D(bb);
		c1 = reduce2D(cc);
		d1 = reduce2D(dd);
		e1 = reduce2D(ee);
		make_sub_faces(face,face->lb,face->rb,face->lt,face->rt);
		split_face(face,face->lb,face->rb,face->lt,face->rt);

		link_face(big_face,face->lb,a1.lb,b1.lb,c1.lb,d1.lb,e1.lb,
			internal);
		link_face(big_face,face->rb,a1.rb,b1.rb,c1.rb,d1.rb,e1.rb,
			internal);
		link_face(big_face,face->lt,a1.lt,b1.lt,c1.lt,d1.lt,e1.lt,
			internal);
		link_face(big_face,face->rt,a1.rt,b1.rt,c1.rt,d1.rt,e1.rt,
			internal);

		free_quadbern2D(a1);
		free_quadbern2D(b1);
		free_quadbern2D(c1);
		free_quadbern2D(d1);
		free_quadbern2D(e1);

	}
}

/*
 * Function:	link_nodes(box,big_box)
 * action:	links together the nodes surronding a box.
 *		adds the links to the list in big_box.
 *		Returns FALSE if abort caught
 */

int link_nodes(big_box,box,aa,bb,cc,dd,ee)
box_info *big_box,*box;
bern3D *aa,*bb,*cc,*dd,*ee;
{
	int f1,f2,f3,count,i,j,num_use_nodes = 0,use_nodes[4];
	double res1,res2,res3;
	double vec0[3],vec1[3];
	node_info *nodes[4];
	octbern3D a1,b1,c1,d1,e1;
	int flag;

   	if( check_interupt( NULL ) ) return(FALSE);
	if( allonesign3D(aa) || allonesign3D(bb) ) return(TRUE);

	f1 = allonesign3D(cc);
	f2 = allonesign3D(dd);
	f3 = allonesign3D(ee);	

	count = get_nodes_on_box_faces(box,nodes);

#ifdef PRINT_LINK_NODES_ALL
		fprintf(stderr,"link_nodes: count = %d f1 %d f2 %d f3 %d BOX (%d,%d,%d)/%d\n",
			count,f1,f2,f3,box->xl,box->yl,box->zl,box->denom);
		printbox(box);
#endif

	if( count == 0 )
	{
		sol_info *sols[2];

		/* test for isolated zeros: require !f1 !f2 !f3 and */

		if( f1 || f2 || f3 ) return(TRUE);
	}
	else if( count == 2 )
	{

		if( nodes[0]->sol->dx == nodes[1]->sol->dx
		 && nodes[0]->sol->dy == nodes[1]->sol->dy
		 && nodes[0]->sol->dz == nodes[1]->sol->dz 
		 && nodes[0]->sol->dx == f1
		 && nodes[0]->sol->dy == f2
		 && nodes[0]->sol->dz == f3 )
		 {

#ifdef PRINT_LINK_NODES_ALL
		fprintf(stderr,"link_nodes: count = %d f1 %d f2 %d f3 %d BOX (%d,%d,%d)/%d\n",
			count,f1,f2,f3,box->xl,box->yl,box->zl,box->denom);

		for( i=1; i<=count;++i)
			print_node(get_nth_node_on_box(box,i));
#endif
		add_node_link(big_box,nodes[0],nodes[1]);
		return(TRUE);
		}
	}

	else if( count == 4 )
	{
		/* Pair wise test */
		/* Suceeds if nodes A,B match C,D match and
			A,C dont match */
		/* Must also check other derivative non-zero */

		nodes[2] = get_nth_node_on_box(box,3);
		nodes[3] = get_nth_node_on_box(box,4);

		if( nodes[0]->sol->dx == nodes[1]->sol->dx
		 && nodes[0]->sol->dy == nodes[1]->sol->dy
		 && nodes[0]->sol->dz == nodes[1]->sol->dz )
		{
		  if( nodes[2]->sol->dx == nodes[3]->sol->dx
		   && nodes[2]->sol->dy == nodes[3]->sol->dy
		   && nodes[2]->sol->dz == nodes[3]->sol->dz )
		  {
			/* Match 0,1  2,3 */

		    if( nodes[0]->sol->dx == nodes[3]->sol->dx
		     && nodes[0]->sol->dy == nodes[3]->sol->dy
		     && nodes[0]->sol->dz == nodes[3]->sol->dz )
		    {
			/* match 0,3 */
		    }
		    else
		    {
			/* No match 0,3 */

			add_node_link(big_box,nodes[0],nodes[1]);
			add_node_link(big_box,nodes[2],nodes[3]);
			return(TRUE);
		    }
		  }
		}
		else if( nodes[0]->sol->dx == nodes[2]->sol->dx
		   && nodes[0]->sol->dy == nodes[2]->sol->dy
		   && nodes[0]->sol->dz == nodes[2]->sol->dz )
		{
		  if( nodes[1]->sol->dx == nodes[3]->sol->dx
		   && nodes[1]->sol->dy == nodes[3]->sol->dy
		   && nodes[1]->sol->dz == nodes[3]->sol->dz )
		  {
			/* Match 0,2  1,3 */

			add_node_link(big_box,nodes[0],nodes[2]);
			add_node_link(big_box,nodes[1],nodes[3]);
			return(TRUE);
		  }
		}
		else if( nodes[0]->sol->dx == nodes[3]->sol->dx
		   && nodes[0]->sol->dy == nodes[3]->sol->dy
		   && nodes[0]->sol->dz == nodes[3]->sol->dz )
		{
		  if( nodes[1]->sol->dx == nodes[2]->sol->dx
		   && nodes[1]->sol->dy == nodes[2]->sol->dy
		   && nodes[1]->sol->dz == nodes[2]->sol->dz )
		  {
			/* Match 0,3  1,2 */

			add_node_link(big_box,nodes[0],nodes[3]);
			add_node_link(big_box,nodes[1],nodes[2]);
			return(TRUE);
		  }
		}
	}

	/*** Too dificult to handle, either sub-devide or create a node ***/

	if( box->denom >= LINK_SING_LEVEL )
	{
		sol_info *sol;

		/* Get average of all soln's */

		vec0[0] = vec0[1] = vec0[2] = 0.0;

		for(i=1; i<=count; ++i)
		{
			nodes[1] = get_nth_node_on_box(box,i);
			calc_pos_in_box(box,nodes[1]->sol,vec1);
			vec0[0] += vec1[0];
			vec0[1] += vec1[1];
			vec0[2] += vec1[2];
		}
		if( count == 0 )
			vec0[0] = vec0[1] = vec0[2] = 0.5;
		else
		{
			vec0[0] /= count;
			vec0[1] /= count;
			vec0[2] /= count;
		}
#ifdef PRINT_LINK_NODES
		fprintf(stderr,"link_nodes: count = %d f1 %d f2 %d f3 %d BOX (%d,%d,%d)/%d\n",
			count,f1,f2,f3,box->xl,box->yl,box->zl,box->denom);

		for( i=1; i<=count;++i)
			print_node(get_nth_node_on_box(box,i));
/*
		if( !f1 ) printbern3D(cc);
		if( !f2 ) printbern3D(dd);
		if( !f3 ) printbern3D(ee);
		printbern3D(aa);
		printbern3D(bb);
		printbox(box);
*/
#endif

		sol = make_sol3(BOX,box->xl,box->yl,box->zl,box->denom,
			vec0[0],vec0[1],vec0[2] );
		sol->dx = f1;
		sol->dy = f2;
		sol->dz = f3;
		add_sing(big_box,sol);
		if( count == 0 ) return(TRUE);
		nodes[0] = alloc_node();
		nodes[0]->next = NULL;
		nodes[0]->sol = sol;
		nodes[0]->status = NODE;
		for( i=1; i<=count; ++i)
		{
			nodes[1] = get_nth_node_on_box(box,i);
			add_node_link(big_box,nodes[0],nodes[1]);
		}
		return(TRUE);
	}
	else
	{
                a1 = reduce3D(aa);
                b1 = reduce3D(bb);
                if( f1 > 0 )      c1 = reduce3D(posbern3D);
		else if( f1 < 0 ) c1 = reduce3D(negbern3D);
		else		  c1 = reduce3D(cc);
                if( f2 > 0 )      d1 = reduce3D(posbern3D);
		else if( f2 < 0 ) d1 = reduce3D(negbern3D);
		else		  d1 = reduce3D(dd);
                if( f3 > 0 )      e1 = reduce3D(posbern3D);
		else if( f3 < 0 ) e1 = reduce3D(negbern3D);
		else		  e1 = reduce3D(ee);

                sub_devide_box(box);
		split_box(box,box->lfd,box->rfd,box->lbd,box->rbd,
                                 box->lfu,box->rfu,box->lbu,box->rbu);

		find_face(box->lfd,a1.lfd,b1.lfd,c1.lfd,d1.lfd,e1.lfd,
			box->lfd->rr,FACE_RR,TRUE);
		find_face(box->lfd,a1.lfd,b1.lfd,c1.lfd,d1.lfd,e1.lfd,
			box->lfd->bb,FACE_BB,TRUE);
		find_face(box->lfd,a1.lfd,b1.lfd,c1.lfd,d1.lfd,e1.lfd,
			box->lfd->uu,FACE_UU,TRUE);
		box->lfd->status = FOUND_FACES;

		find_face(box->rbd,a1.rbd,b1.rbd,c1.rbd,d1.rbd,e1.rbd,
			box->rbd->ll,FACE_LL,TRUE);
		find_face(box->rbd,a1.rbd,b1.rbd,c1.rbd,d1.rbd,e1.rbd,
			box->rbd->ff,FACE_FF,TRUE);
		find_face(box->rbd,a1.rbd,b1.rbd,c1.rbd,d1.rbd,e1.rbd,
			box->rbd->uu,FACE_UU,TRUE);
		box->rbd->status = FOUND_FACES;

		find_face(box->rfu,a1.rfu,b1.rfu,c1.rfu,d1.rfu,e1.rfu,
			box->rfu->ll,FACE_LL,TRUE);
		find_face(box->rfu,a1.rfu,b1.rfu,c1.rfu,d1.rfu,e1.rfu,
			box->rfu->bb,FACE_BB,TRUE);
		find_face(box->rfu,a1.rfu,b1.rfu,c1.rfu,d1.rfu,e1.rfu,
			box->rfu->dd,FACE_DD,TRUE);
		box->rfu->status = FOUND_FACES;
		box->rfd->status = FOUND_FACES;

		find_face(box->lbu,a1.lbu,b1.lbu,c1.lbu,d1.lbu,e1.lbu,
			box->lbu->rr,FACE_RR,TRUE);
		find_face(box->lbu,a1.lbu,b1.lbu,c1.lbu,d1.lbu,e1.lbu,
			box->lbu->ff,FACE_FF,TRUE);
		find_face(box->lbu,a1.lbu,b1.lbu,c1.lbu,d1.lbu,e1.lbu,
			box->lbu->dd,FACE_DD,TRUE);
		box->lbu->status = FOUND_FACES;
		box->lfu->status = FOUND_FACES;
		box->lbd->status = FOUND_FACES;
		box->rbu->status = FOUND_FACES;

		flag = 
                   link_nodes(big_box,box->lfd,
			a1.lfd,b1.lfd,c1.lfd,d1.lfd,e1.lfd)
                && link_nodes(big_box,box->rfd,
			a1.rfd,b1.rfd,c1.rfd,d1.rfd,e1.rfd)
                && link_nodes(big_box,box->lbd,
			a1.lbd,b1.lbd,c1.lbd,d1.lbd,e1.lbd)
                && link_nodes(big_box,box->rbd,
			a1.rbd,b1.rbd,c1.rbd,d1.rbd,e1.rbd)
                && link_nodes(big_box,box->lfu,
			a1.lfu,b1.lfu,c1.lfu,d1.lfu,e1.lfu)
                && link_nodes(big_box,box->rfu,
			a1.rfu,b1.rfu,c1.rfu,d1.rfu,e1.rfu)
                && link_nodes(big_box,box->lbu,
			a1.lbu,b1.lbu,c1.lbu,d1.lbu,e1.lbu)
                && link_nodes(big_box,box->rbu,
			a1.rbu,b1.rbu,c1.rbu,d1.rbu,e1.rbu);

		free_octbern3D(a1);
		free_octbern3D(b1);
		free_octbern3D(c1);
		free_octbern3D(d1);
		free_octbern3D(e1);
		return(flag);
        }
}

calc_pos_norm(sol,vec,norm)
sol_info *sol;
double vec[3],norm[3];
{
	calc_pos(sol,vec);
	
	if(sol->dx == 0) norm[0] = 0.0;
	else		 norm[0] = evalbern3D(CC,vec);
	if(sol->dy == 0) norm[1] = 0.0;
	else		 norm[1] = evalbern3D(DD,vec);
	if(sol->dz == 0) norm[2] = 0.0;
	else		 norm[2] = evalbern3D(EE,vec);
}

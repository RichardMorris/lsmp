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
/* 									*/
/*	Sub routines dealing with the way boxes, faces and edges link	*/
/*	together.							*/
/* 									*/
/************************************************************************/

#include <stdio.h>
#include "cells.h"
#define grballoc(node) ( node * ) malloc( sizeof(node) )
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

extern hyper_info whole_hyper;

/*
 * Function:	get_hyper
 * action:	returns a pointer to the hyper specified by xl,yl,zl,denom
 *		returns NULL if hyper does not exist.
 */

hyper_info *get_hyper(x,y,z,w,denom)
int x,y,z,w,denom;
{
	hyper_info *temp;

	if( x < 0 || x >= denom || y < 0 || y >= denom || z < 0 || z >= denom 
	 || w < 0 || w >= denom )
		return( NULL );
	temp = &whole_hyper;

	while( temp->denom < denom )
	{
	    if( 2*temp->denom*x < ( 2*temp->xl+1 )*denom )
	    {
		if( 2*temp->denom*y < ( 2*temp->yl+1 )*denom )
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->lfdi;
			else
				temp = temp->lfdo;
		    }
		    else
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->lfui;
			else
				temp = temp->lfuo;
		    }
		}
		else
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->lbdi;
			else
				temp = temp->lbdo;
		    }
		    else
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->lbui;
			else
				temp = temp->lbuo;
		    }
		}
	    }
	    else
	    {
		if( 2*temp->denom*y < ( 2*temp->yl+1 )*denom )
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->rfdi;
			else
				temp = temp->rfdo;
		    }
		    else
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->rfui;
			else
				temp = temp->rfuo;
		    }
		}
		else
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->rbdi;
			else
				temp = temp->rbdo;
		    }
		    else
		    {
			if( 2*temp->denom*w < ( 2*temp->wl + 1)*denom )
				temp = temp->rbui;
			else
				temp = temp->rbuo;
		    }
		}
	    }

	    if( temp == NULL ) return( NULL );
	}
	if( ! temp->status ) return(NULL);
	return(temp);
}

/*
 * Function	get_existing_boxes
 * action:	if any of the boxes on hyper have been found on any
 *		other hyper use that information.
 */

get_existing_boxes(hyper)
hyper_info *hyper;
{
	hyper_info *adjacient_hyper;

	adjacient_hyper = get_hyper(
		hyper->xl-1,hyper->yl,hyper->zl,hyper->wl,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->ll = adjacient_hyper->rr;

	adjacient_hyper = get_hyper(
		hyper->xl+1,hyper->yl,hyper->zl,hyper->wl,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->rr = adjacient_hyper->ll;

	adjacient_hyper = get_hyper(
		hyper->xl,hyper->yl-1,hyper->zl,hyper->wl,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->ff = adjacient_hyper->bb;

	adjacient_hyper = get_hyper(
		hyper->xl,hyper->yl+1,hyper->zl,hyper->wl,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->bb = adjacient_hyper->ff;

	adjacient_hyper = get_hyper(
		hyper->xl,hyper->yl,hyper->zl-1,hyper->wl,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->dd = adjacient_hyper->uu;

	adjacient_hyper = get_hyper(
		hyper->xl,hyper->yl,hyper->zl+1,hyper->wl,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->uu = adjacient_hyper->dd;

	adjacient_hyper = get_hyper(
		hyper->xl,hyper->yl,hyper->zl,hyper->wl-1,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->ii = adjacient_hyper->oo;

	adjacient_hyper = get_hyper(
		hyper->xl,hyper->yl,hyper->zl,hyper->wl+1,hyper->denom);
	if( adjacient_hyper != NULL )
		hyper->oo = adjacient_hyper->ii;
}

/*
 * Function:	create_new_boxes
 * action:	if any of the boxes have not already been found
 *		create the apropriate information.
 */

create_new_boxes(hyper)
hyper_info *hyper;
{
	if( hyper->ll == NULL )
	{
		hyper->ll = alloc_box();
		make_hyper_box(hyper,BOX_LL,hyper->ll);
	}
	if( hyper->rr == NULL )
	{
		hyper->rr = alloc_box();
		make_hyper_box(hyper,BOX_RR,hyper->rr);
	}
	if( hyper->ff == NULL )
	{
		hyper->ff = alloc_box();
		make_hyper_box(hyper,BOX_FF,hyper->ff);
	}
	if( hyper->bb == NULL )
	{
		hyper->bb = alloc_box();
		make_hyper_box(hyper,BOX_BB,hyper->bb);
	}
	if( hyper->dd == NULL )
	{
		hyper->dd = alloc_box();
		make_hyper_box(hyper,BOX_DD,hyper->dd);
	}
	if( hyper->uu == NULL )
	{
		hyper->uu = alloc_box();
		make_hyper_box(hyper,BOX_UU,hyper->uu);
	}
	if( hyper->ii == NULL )
	{
		hyper->ii = alloc_box();
		make_hyper_box(hyper,BOX_II,hyper->ii);
	}
	if( hyper->oo == NULL )
	{
		hyper->oo = alloc_box();
		make_hyper_box(hyper,BOX_OO,hyper->oo);
	}
}

/*
 * Function:	get_existing_edges
 * action:	if any of the edges lie on existing faces of existing
 *		boxes link the information.
 */

#ifdef NOT_DEF
get_existing_edges(box,face,code)
box_info *box;
face_info *face;
int code;
{
	box_info *adjacient_box;

	switch( code )
	{
	case FACE_LL:

		/*** Do edge x_low of face LL ***/

		if( face->x_low != NULL )
			fprintf(stderr,"get_existing_edge: ll->x_low != NULL\n");

		if(box->ff->x_low != NULL ) face->x_low = box->ff->x_low;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl-1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->rr != NULL )
				face->x_low = adjacient_box->rr->x_high;
		}

		/*** Do edge x_high of face LL ***/

		if( face->x_high != NULL )
			fprintf(stderr,"get_existing_edge: ll->x_high != NULL\n");

		if(box->bb->x_low != NULL ) face->x_high = box->bb->x_low;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl+1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->rr != NULL )
				face->x_high = adjacient_box->rr->x_low;
		}

		/*** Do edge y_low of face LL ***/

		if( face->y_low != NULL )
			fprintf(stderr,"get_existing_edge: ll->y_low != NULL\n");

		if(box->dd->x_low != NULL ) face->y_low = box->dd->x_low;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->rr != NULL )
				face->y_low = adjacient_box->rr->y_high;
		}

		/*** Do edge y_high of face LL ***/

		if( face->y_high != NULL )
			fprintf(stderr,"get_existing_edge: ll->y_high != NULL\n");

		if(box->uu->x_low != NULL ) face->y_high = box->uu->x_low;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->rr != NULL )
				face->y_high = adjacient_box->rr->y_low;
		}
		break;

	case FACE_RR:

		/*** Do edge x_low of face RR ***/

		if( face->x_low != NULL )
			fprintf(stderr,"get_existing_edge: rr->x_low != NULL\n");

		if(box->ff->x_high != NULL ) face->x_low = box->ff->x_high;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl-1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->ll != NULL )
				face->x_low = adjacient_box->ll->x_high;
		}

		/*** Do edge x_high of face RR ***/

		if( face->x_high != NULL )
			fprintf(stderr,"get_existing_edge: rr->x_high != NULL\n");

		if(box->bb->x_high != NULL ) face->x_high = box->bb->x_high;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl+1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->ll != NULL )
				face->x_high = adjacient_box->ll->x_low;
		}

		/*** Do edge y_low of face LL ***/

		if( face->y_low != NULL )
			fprintf(stderr,"get_existing_edge: rr->y_low != NULL\n");

		if(box->dd->x_high != NULL ) face->y_low = box->dd->x_high;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->ll != NULL )
				face->y_low = adjacient_box->ll->y_high;
		}

		/*** Do edge y_high of face LL ***/

		if( face->y_high != NULL )
			fprintf(stderr,"get_existing_edge: rr->y_high != NULL\n");

		if(box->uu->x_high != NULL ) face->y_high = box->uu->x_high;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->ll != NULL )
				face->y_high = adjacient_box->ll->y_low;
		}
		break;

	case FACE_FF:

		/*** Do edge x_low of face FF ***/

		if( face->x_low != NULL )
			fprintf(stderr,"get_existing_edge: ff->x_low != NULL\n");

		if(box->ll->x_low != NULL ) face->x_low = box->ll->x_low;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl-1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->bb != NULL )
				face->x_low = adjacient_box->bb->x_high;
		}

		/*** Do edge x_high of face FF ***/

		if( face->x_high != NULL )
			fprintf(stderr,"get_existing_edge: ff->x_high != NULL\n");

		if(box->rr->x_low != NULL ) face->x_high = box->rr->x_low;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl-1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->bb != NULL )
				face->x_high = adjacient_box->bb->x_low;
		}

		/*** Do edge y_low of face FF ***/

		if( face->y_low != NULL )
			fprintf(stderr,"get_existing_edge: ff->y_low != NULL\n");

		if(box->dd->y_low != NULL ) face->y_low = box->dd->y_low;
		else
		{
			adjacient_box = get_box(box->xl,box->yl-1,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->bb != NULL )
				face->y_low = adjacient_box->bb->y_high;
		}

		/*** Do edge y_high of face FF ***/

		if( face->y_high != NULL )
			fprintf(stderr,"get_existing_edge: ff->y_high != NULL\n");

		if(box->uu->y_low != NULL ) face->y_high = box->uu->y_low;
		else
		{
			adjacient_box = get_box(box->xl,box->yl-1,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->bb != NULL )
				face->y_high = adjacient_box->bb->y_low;
		}
		break;

	case FACE_BB:

		/*** Do edge x_low of face BB ***/

		if( face->x_low != NULL )
			fprintf(stderr,"get_existing_edge: bb->x_low != NULL\n");

		if(box->ll->x_high != NULL ) face->x_low = box->ll->x_high;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl+1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->ff != NULL )
				face->x_low = adjacient_box->ff->x_high;
		}

		/*** Do edge x_high of face BB ***/

		if( face->x_high != NULL )
			fprintf(stderr,"get_existing_edge: bb->x_high != NULL\n");

		if(box->rr->x_high != NULL ) face->x_high = box->rr->x_high;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl+1,
				box->zl,box->denom);
			if( adjacient_box != NULL && adjacient_box->ff != NULL )
				face->x_high = adjacient_box->ff->x_low;
		}

		/*** Do edge y_low of face BB ***/

		if( face->y_low != NULL )
			fprintf(stderr,"get_existing_edge: bb->y_low != NULL\n");

		if(box->dd->y_high != NULL ) face->y_low = box->dd->y_high;
		else
		{
			adjacient_box = get_box(box->xl,box->yl+1,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->ff != NULL )
				face->y_low = adjacient_box->ff->y_high;
		}

		/*** Do edge y_high of face BB ***/

		if( face->y_high != NULL )
			fprintf(stderr,"get_existing_edge: bb->y_high != NULL\n");

		if(box->uu->y_high != NULL ) face->y_high = box->uu->y_high;
		else
		{
			adjacient_box = get_box(box->xl,box->yl+1,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->ff != NULL )
				face->y_high = adjacient_box->ff->y_low;
		}
		break;

	case FACE_DD:

		/*** Do edge x_low of face DD ***/

		if( face->x_low != NULL )
			fprintf(stderr,"get_existing_edge: dd->x_low != NULL\n");

		if(box->ll->y_low != NULL ) face->x_low = box->ll->y_low;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->uu != NULL )
				face->x_low = adjacient_box->uu->x_high;
		}

		/*** Do edge x_high of face DD ***/

		if( face->x_high != NULL )
			fprintf(stderr,"get_existing_edge: dd->x_high != NULL\n");

		if(box->rr->y_low != NULL ) face->x_high = box->rr->y_low;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->uu != NULL )
				face->x_high = adjacient_box->uu->x_low;
		}

		/*** Do edge y_low of face DD ***/

		if( face->y_low != NULL )
			fprintf(stderr,"get_existing_edge: dd->y_low != NULL\n");

		if(box->ff->y_low != NULL ) face->y_low = box->ff->y_low;
		else
		{
			adjacient_box = get_box(box->xl,box->yl-1,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->uu != NULL )
				face->y_low = adjacient_box->uu->y_high;
		}

		/*** Do edge y_high of face DD ***/

		if( face->y_high != NULL )
			fprintf(stderr,"get_existing_edge: dd->y_high != NULL\n");

		if(box->bb->y_low != NULL ) face->y_high = box->bb->y_low;
		else
		{
			adjacient_box = get_box(box->xl,box->yl+1,
				box->zl-1,box->denom);
			if( adjacient_box != NULL && adjacient_box->uu != NULL )
				face->y_high = adjacient_box->uu->y_low;
		}
		break;

	case FACE_UU:

		/*** Do edge x_low of face UU ***/

		if( face->x_low != NULL )
			fprintf(stderr,"get_existing_edge: uu->x_low != NULL\n");

		if(box->ll->y_high != NULL ) face->x_low = box->ll->y_high;
		else
		{
			adjacient_box = get_box(box->xl-1,box->yl,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->dd != NULL )
				face->x_low = adjacient_box->dd->x_high;
		}

		/*** Do edge x_high of face UU ***/

		if( face->x_high != NULL )
			fprintf(stderr,"get_existing_edge: uu->x_high != NULL\n");

		if(box->rr->y_high != NULL ) face->x_high = box->rr->y_high;
		else
		{
			adjacient_box = get_box(box->xl+1,box->yl,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->dd != NULL )
				face->x_high = adjacient_box->dd->x_low;
		}

		/*** Do edge y_low of face UU ***/

		if( face->y_low != NULL )
			fprintf(stderr,"get_existing_edge: uu->y_low != NULL\n");

		if(box->ff->y_high != NULL ) face->y_low = box->ff->y_high;
		else
		{
			adjacient_box = get_box(box->xl,box->yl-1,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->dd != NULL )
				face->y_low = adjacient_box->dd->y_high;
		}

		/*** Do edge y_high of face UU ***/

		if( face->y_high != NULL )
			fprintf(stderr,"get_existing_edge: uu->y_high != NULL\n");

		if(box->bb->y_high != NULL ) face->y_high = box->bb->y_high;
		else
		{
			adjacient_box = get_box(box->xl,box->yl+1,
				box->zl+1,box->denom);
			if( adjacient_box != NULL && adjacient_box->dd != NULL )
				face->y_high = adjacient_box->dd->y_low;
		}

		break;
	} /* end switch */
}
#endif

/*
 * Function:	create_new_face
 * action:	if any of the edge have not already been found
 *		create the apropriate information.
 */

#ifdef NOT_DEF
create_new_edges(face)
face_info *face;
{
	if( face->x_low == NULL )
	{
		face->x_low = alloc_edge();
		make_face_edge(face,X_LOW,face->x_low);
	}
	if( face->x_high == NULL )
	{
		face->x_high = alloc_edge();
		make_face_edge(face,X_HIGH,face->x_high);
	}
	if( face->y_low == NULL )
	{
		face->y_low = alloc_edge();
		make_face_edge(face,Y_LOW,face->y_low);
	}
	if( face->y_high == NULL )
	{
		face->y_high = alloc_edge();
		make_face_edge(face,Y_HIGH,face->y_high);
	}
}
#endif

/*
 * Function:	get_sols_on_edge
 * action:	returns the sum of the number of sols on an edge and count,
 *		if count < 2 pute the solutions in the array.
 */

int get_sols_on_edge(edge,sols,count)
edge_info *edge;
sol_info *sols[2];
int count;
{
	if( edge == NULL ) return(count);
	if( edge->sol != NULL )
	{
		if( count < 2 ) sols[count] = edge->sol;
		++count;
	}
	if( edge->left != NULL )
		count = get_sols_on_edge(edge->left,sols,count);
	if( edge->right != NULL )
		count = get_sols_on_edge(edge->right,sols,count);
	return(count);
}

/*
 * Function:	count_sols_on_edge
 * action:	returns the sum of the number of sols on an edge.
 */

int count_sols_on_edge(edge)
edge_info *edge;
{
	int count = 0;

	if( edge == NULL ) return(0);
	if( edge->sol != NULL )
	{
		++count;
	}
	if( edge->left != NULL )
		count += count_sols_on_edge(edge->left);
	if( edge->right != NULL )
		count += count_sols_on_edge(edge->right);
	return(count);
}

/*
 * Function:	get_nth_sol_on_edge
 * action:	returns the n-th sol on the edge NULL if don't exist.
 */

sol_info *get_nth_sol_on_edge(edge,n)
edge_info *edge;
{
	sol_info *temp;

	if( edge == NULL ) return(NULL);
	if( edge->sol != NULL )
	{
		--n;
		if( n == 0 ) return(edge->sol);
	}
	if( edge->left != NULL )
	{
		temp = get_nth_sol_on_edge(edge->left,n);
		if(temp != NULL ) return(temp);
		n -= count_sols_on_edge(edge->left);
	}
	if( edge->right != NULL )
	{
		temp = get_nth_sol_on_edge(edge->right,n);
		if(temp != NULL ) return(temp);
		n -= count_sols_on_edge(edge->right);
	}
	return(NULL);
}

/*
 * Function:	get_edge_sols_of_face
 * action:	returns the number of solutions round a face,
 *		the first two of these solutions are put in the array sols.
 */

int get_sols_on_face(face,sols)
face_info *face;
sol_info *sols[2];
{
	int count;

	count = 0;
	count = get_sols_on_edge(face->x_low,sols,count);
	count = get_sols_on_edge(face->x_high,sols,count);
	count = get_sols_on_edge(face->y_low,sols,count);
	count = get_sols_on_edge(face->y_high,sols,count);
	return(count);
}

/*
 * Function:	count_edge_sols_of_face
 * action:	returns the number of solutions round a face,
 */

int count_sols_on_face(face)
face_info *face;
{
	int count;

	count = 0;
	count += count_sols_on_edge(face->x_low);
	count += count_sols_on_edge(face->x_high);
	count += count_sols_on_edge(face->y_low);
	count += count_sols_on_edge(face->y_high);
	return(count);
}

/*
 * Function:	get_nth_sol_on_face
 * action:	returns the n-th edge sol round a face, or NULL.
 */

sol_info *get_nth_sol_on_face(face,n)
face_info *face;
int n;
{
	sol_info *temp;

	temp = get_nth_sol_on_edge(face->x_low,n);
	if( temp != NULL ) return(temp);
	n -= count_sols_on_edge(face->x_low);

	temp = get_nth_sol_on_edge(face->x_high,n);
	if( temp != NULL ) return(temp);
	n -= count_sols_on_edge(face->x_high);

	temp = get_nth_sol_on_edge(face->y_low,n);
	if( temp != NULL ) return(temp);
	n -= count_sols_on_edge(face->y_low);

	temp = get_nth_sol_on_edge(face->y_high,n);
	if( temp != NULL ) return(temp);
	n -= count_sols_on_edge(face->y_high);

	return(NULL);
}

/*
 * Function:	get_nodes_on_box
 * action:	returns the number of nodes on a box,
 *		if count < 2 pute the solutions in the array.
 */

int get_nodes_on_box(box,nodes,count)
box_info *box;
node_info *nodes[2];
int count;
{
	node_info *temp;

	if( box == NULL ) return(count);
	temp = box->nodes;
	while( temp != NULL )
	{
		if( count < 2 ) nodes[count] = temp;
		++count;
		temp = temp->next;
	}
	if( box->lfd != NULL )
	{
		count = get_nodes_on_box(box->lfd,nodes,count);
		count = get_nodes_on_box(box->rfd,nodes,count);
		count = get_nodes_on_box(box->lbd,nodes,count);
		count = get_nodes_on_box(box->rbd,nodes,count);
		count = get_nodes_on_box(box->lfu,nodes,count);
		count = get_nodes_on_box(box->rfu,nodes,count);
		count = get_nodes_on_box(box->lbu,nodes,count);
		count = get_nodes_on_box(box->rbu,nodes,count);
	}
	return(count);
}

int count_nodes_on_box(box)
box_info *box;
{
	int count=0;
	node_info *temp;

	if( box == NULL ) return(0);
	temp = box->nodes;
	while( temp != NULL )
	{
		++count;
		temp = temp->next;
	}
	if( box->lfd != NULL )
	{
		count += count_nodes_on_box(box->lfd)
		 + count_nodes_on_box(box->rfd)
		 + count_nodes_on_box(box->lbd)
		 + count_nodes_on_box(box->rbd)
		 + count_nodes_on_box(box->lfu)
		 + count_nodes_on_box(box->rfu)
		 + count_nodes_on_box(box->lbu)
		 + count_nodes_on_box(box->rbu);
	}
	return(count);
}

/*
 * Function:	get_nth_node_on_box
 * action:	finds the nth node on a box and returns a pointer to it.
 *		If there is no nth node return (nil).
 */

node_info *get_nth_node_on_box(box,n)
box_info *box;
int n;
{
	node_info *temp;

	if( box == NULL ) return(NULL);
	temp = box->nodes;
	while( temp != NULL )
	{
		--n;
		if( n == 0 ) return(temp);
		temp = temp->next;
	}

	/* Now try sub boxes */

	if( box->lfd != NULL )
	{
		temp = get_nth_node_on_box(box->lfd,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->lfd);
	}
	if( box->rfd != NULL )
	{
		temp = get_nth_node_on_box(box->rfd,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->rfd);
	}
	if( box->lbd != NULL )
	{
		temp = get_nth_node_on_box(box->lbd,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->lbd);
	}
	if( box->rbd != NULL )
	{
		temp = get_nth_node_on_box(box->rbd,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->rbd);
	}
	if( box->lfu != NULL )
	{
		temp = get_nth_node_on_box(box->lfu,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->lfu);
	}
	if( box->rfu != NULL )
	{
		temp = get_nth_node_on_box(box->rfu,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->rfu);
	}
	if( box->lbu != NULL )
	{
		temp = get_nth_node_on_box(box->lbu,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->lbu);
	}
	if( box->rbu != NULL )
	{
		temp = get_nth_node_on_box(box->rbu,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_box(box->rbu);
	}

	/* couldn't find an nth node */

	return(NULL);
}

/*
 * Function:	get_nodes_on_hyper
 * action:	returns the number of nodes round a hyper,
 *		the first two of these solutions are put in the array nodes.
 */

int get_nodes_on_hyper_boxes(hyper,nodes)
hyper_info *hyper;
node_info *nodes[2];
{
	int count;

	count = 0;
	count = get_nodes_on_box(hyper->ll,nodes,count);
	count = get_nodes_on_box(hyper->rr,nodes,count);
	count = get_nodes_on_box(hyper->ff,nodes,count);
	count = get_nodes_on_box(hyper->bb,nodes,count);
	count = get_nodes_on_box(hyper->dd,nodes,count);
	count = get_nodes_on_box(hyper->uu,nodes,count);
	count = get_nodes_on_box(hyper->ii,nodes,count);
	count = get_nodes_on_box(hyper->oo,nodes,count);
	return(count);
}

/*
 * Function:	get_nth_node_on_hyper
 * action:	returns the nth node  on the faces of the hyper.
 */

node_info *get_nth_node_on_hyper(hyper,n)
hyper_info *hyper;
int n;
{
	node_info *temp;

	temp = get_nth_node_on_box(hyper->ll,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->ll);

	temp = get_nth_node_on_box(hyper->rr,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->rr);

	temp = get_nth_node_on_box(hyper->ff,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->ff);

	temp = get_nth_node_on_box(hyper->bb,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->bb);

	temp = get_nth_node_on_box(hyper->dd,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->dd);

	temp = get_nth_node_on_box(hyper->uu,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->uu);

	temp = get_nth_node_on_box(hyper->ii,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->ii);

	temp = get_nth_node_on_box(hyper->oo,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_box(hyper->oo);
	return(NULL);
}

/*
 * Function:	split_face
 * action:	takes information about face and puts it in the
 *		sub faces. Does not find internal edges.
 */

#ifdef NOT_DEF
split_face(face,face1,face2,face3,face4)
face_info *face,*face1,*face2,*face3,*face4;
{
	/*** First put all the edges on the appropriate sub faces ***/

	if( face->x_low != NULL )
	{
		split_edge(face->x_low);
		face1->x_low = face->x_low->left;
		face3->x_low = face->x_low->right;
	}

	if( face->x_high != NULL )
	{
		split_edge(face->x_high);
		face2->x_high = face->x_high->left;
		face4->x_high = face->x_high->right;
	}

	if( face->y_low != NULL )
	{
		split_edge(face->y_low);
		face1->y_low = face->y_low->left;
		face2->y_low = face->y_low->right;
	}

	if( face->y_high != NULL )
	{
		split_edge(face->y_high);
		face3->y_high = face->y_high->left;
		face4->y_high = face->y_high->right;
	}

	/*** Now create the internal edges ***/

	face1->x_high = face2->x_low = alloc_edge();
	make_face_edge(face1,X_HIGH,face1->x_high);

	face3->x_high = face4->x_low = alloc_edge();
	make_face_edge(face3,X_HIGH,face3->x_high);

	face1->y_high = face3->y_low = alloc_edge();
	make_face_edge(face1,Y_HIGH,face1->y_high);

	face2->y_high = face4->y_low = alloc_edge();
	make_face_edge(face2,Y_HIGH,face2->y_high);
}
#endif

/*
 * Function:	distribute_nodes
 * action:	take all the nodes on box and put them on the corect subbox.
 *		needed if the second time a box is acessed a greater
 *		resolution is needed.
 */

distribute_nodes(box,box1,box2,box3,box4,box5,box6,box7,box8)
box_info *box,*box1,*box2,*box3,*box4,*box5,*box6,*box7,*box8;
{
	node_info *node;

	while( box->nodes != NULL )
	{
	    node = box->nodes;
	    switch(box->type)
	    {
	    case BOX_LL: case BOX_RR:
		if(node->sol->yl * box8->denom
			<  node->sol->denom * box8->yl )
		{
			if(node->sol->zl * box8->denom
				< node->sol->denom * box8->zl )
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box1 */

					box->nodes = node->next;
					node->next = box1->nodes;
					box1->nodes = node;
				}
				else
				{
					/* add to box5 */

					box->nodes = node->next;
					node->next = box5->nodes;
					box5->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box3 */

					box->nodes = node->next;
					node->next = box3->nodes;
					box3->nodes = node;
				}
				else
				{
					/* add to box7 */

					box->nodes = node->next;
					node->next = box7->nodes;
					box7->nodes = node;
				}
			}
		}
		else
		{
			if(node->sol->zl * box8->denom
				< node->sol->denom * box8->zl )
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box2 */

					box->nodes = node->next;
					node->next = box2->nodes;
					box2->nodes = node;
				}
				else
				{
					/* add to box6 */

					box->nodes = node->next;
					node->next = box6->nodes;
					box6->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box4 */

					box->nodes = node->next;
					node->next = box4->nodes;
					box4->nodes = node;
				}
				else
				{
					/* add to box8 */

					box->nodes = node->next;
					node->next = box8->nodes;
					box8->nodes = node;
				}
			}
		}
		break;

	    case BOX_FF: case BOX_BB:
		if(node->sol->xl * box8->denom
			<  node->sol->denom * box8->xl )
		{
			if(node->sol->zl * box8->denom
				< node->sol->denom * box8->zl )
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box1 */

					box->nodes = node->next;
					node->next = box1->nodes;
					box1->nodes = node;
				}
				else
				{
					/* add to box5 */

					box->nodes = node->next;
					node->next = box5->nodes;
					box5->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box3 */

					box->nodes = node->next;
					node->next = box3->nodes;
					box3->nodes = node;
				}
				else
				{
					/* add to box7 */

					box->nodes = node->next;
					node->next = box7->nodes;
					box7->nodes = node;
				}
			}
		}
		else
		{
			if(node->sol->zl * box8->denom
				< node->sol->denom * box8->zl )
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box2 */

					box->nodes = node->next;
					node->next = box2->nodes;
					box2->nodes = node;
				}
				else
				{
					/* add to box6 */

					box->nodes = node->next;
					node->next = box6->nodes;
					box6->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box4 */

					box->nodes = node->next;
					node->next = box4->nodes;
					box4->nodes = node;
				}
				else
				{
					/* add to box8 */

					box->nodes = node->next;
					node->next = box8->nodes;
					box8->nodes = node;
				}
			}
		}
		break;

	    case BOX_DD: case BOX_UU:
		if(node->sol->xl * box8->denom
			<  node->sol->denom * box8->xl )
		{
			if(node->sol->yl * box8->denom
				< node->sol->denom * box8->yl )
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box1 */

					box->nodes = node->next;
					node->next = box1->nodes;
					box1->nodes = node;
				}
				else
				{
					/* add to box5 */

					box->nodes = node->next;
					node->next = box5->nodes;
					box5->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box3 */

					box->nodes = node->next;
					node->next = box3->nodes;
					box3->nodes = node;
				}
				else
				{
					/* add to box7 */

					box->nodes = node->next;
					node->next = box7->nodes;
					box7->nodes = node;
				}
			}
		}
		else
		{
			if(node->sol->yl * box8->denom
				< node->sol->denom * box8->yl )
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box2 */

					box->nodes = node->next;
					node->next = box2->nodes;
					box2->nodes = node;
				}
				else
				{
					/* add to box6 */

					box->nodes = node->next;
					node->next = box6->nodes;
					box6->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->wl * box8->denom
					< node->sol->denom * box8->wl )
				{
					/* add to box4 */

					box->nodes = node->next;
					node->next = box4->nodes;
					box4->nodes = node;
				}
				else
				{
					/* add to box8 */

					box->nodes = node->next;
					node->next = box8->nodes;
					box8->nodes = node;
				}
			}
		}
		break;

	    case BOX_II: case BOX_OO:
		if(node->sol->xl * box8->denom
			<  node->sol->denom * box8->xl )
		{
			if(node->sol->yl * box8->denom
				< node->sol->denom * box8->yl )
			{
				if(node->sol->zl * box8->denom
					< node->sol->denom * box8->zl )
				{
					/* add to box1 */

					box->nodes = node->next;
					node->next = box1->nodes;
					box1->nodes = node;
				}
				else
				{
					/* add to box5 */

					box->nodes = node->next;
					node->next = box5->nodes;
					box5->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->zl * box8->denom
					< node->sol->denom * box8->zl )
				{
					/* add to box3 */

					box->nodes = node->next;
					node->next = box3->nodes;
					box3->nodes = node;
				}
				else
				{
					/* add to box7 */

					box->nodes = node->next;
					node->next = box7->nodes;
					box7->nodes = node;
				}
			}
		}
		else
		{
			if(node->sol->yl * box8->denom
				< node->sol->denom * box8->yl )
			{
				if(node->sol->zl * box8->denom
					< node->sol->denom * box8->zl )
				{
					/* add to box2 */

					box->nodes = node->next;
					node->next = box2->nodes;
					box2->nodes = node;
				}
				else
				{
					/* add to box6 */

					box->nodes = node->next;
					node->next = box6->nodes;
					box6->nodes = node;
				}
					
			}
			else
			{
				if(node->sol->zl * box8->denom
					< node->sol->denom * box8->zl )
				{
					/* add to box4 */

					box->nodes = node->next;
					node->next = box4->nodes;
					box4->nodes = node;
				}
				else
				{
					/* add to box8 */

					box->nodes = node->next;
					node->next = box8->nodes;
					box8->nodes = node;
				}
			}
		}
		break;

	    }  /* end switch */
	} /* end while */
}

/* Function:	split_hyper
 * action:	takes the information from hyper cube and puts it into
 *		the sub hyper cubes. does not creates internal boxes.
 */

split_hyper(hyper,lfdi,rfdi,lbdi,rbdi,lfui,rfui,lbui,rbui,
		  lfdo,rfdo,lbdo,rbdo,lfuo,rfuo,lbuo,rbuo)
hyper_info *hyper,*lfdi,*rfdi,*lbdi,*rbdi,*lfui,*rfui,*lbui,*rbui,
		  *lfdo,*rfdo,*lbdo,*rbdo,*lfuo,*rfuo,*lbuo,*rbuo;
{
	/* First Work with the LL boxes of the hypers */

        if( hyper->ll->lfd == NULL )
        {
                /*** create new boxes ***/

                lfdi->ll = alloc_box();
                lbdi->ll = alloc_box();
                lfui->ll = alloc_box();
                lbui->ll = alloc_box();
                lfdo->ll = alloc_box();
                lbdo->ll = alloc_box();
                lfuo->ll = alloc_box();
                lbuo->ll = alloc_box();
        
                make_sub_boxes(hyper->ll,lfdi->ll,lbdi->ll,lfui->ll,lbui->ll,
                	      		 lfdo->ll,lbdo->ll,lfuo->ll,lbuo->ll);
        }
        else
        {
                /*** box has already been split so use this info ***/

                lfdi->ll = hyper->ll->lfd;
                lbdi->ll = hyper->ll->rfd;
                lfui->ll = hyper->ll->lbd;
                lbui->ll = hyper->ll->rbd;
                lfdo->ll = hyper->ll->lfu;
                lbdo->ll = hyper->ll->rfu;
                lfuo->ll = hyper->ll->lbu;
                lbuo->ll = hyper->ll->rbu;
        }
        distribute_nodes(hyper->ll,lfdi->ll,lbdi->ll,lfui->ll,lbui->ll,
        			   lfdo->ll,lbdo->ll,lfuo->ll,lbuo->ll);

	/* Work with the RR boxes of the hypers */

        if( hyper->rr->lfd == NULL )
        {
                /*** create new boxes ***/

                rfdi->rr = alloc_box();
                rbdi->rr = alloc_box();
                rfui->rr = alloc_box();
                rbui->rr = alloc_box();
                rfdo->rr = alloc_box();
                rbdo->rr = alloc_box();
                rfuo->rr = alloc_box();
                rbuo->rr = alloc_box();

                make_sub_boxes(hyper->rr,rfdi->rr,rbdi->rr,rfui->rr,rbui->rr,
                	      	    rfdo->rr,rbdo->rr,rfuo->rr,rbuo->rr);
        }
        else
        {
                /*** box has already been split so use this info ***/

                rfdi->rr = hyper->rr->lfd;
                rbdi->rr = hyper->rr->rfd;
                rfui->rr = hyper->rr->lbd;
                rbui->rr = hyper->rr->rbd;
                rfdo->rr = hyper->rr->lfu;
                rbdo->rr = hyper->rr->rfu;
                rfuo->rr = hyper->rr->lbu;
                rbuo->rr = hyper->rr->rbu;
        }
        distribute_nodes(hyper->rr,rfdi->rr,rbdi->rr,rfui->rr,rbui->rr,
        			   rfdo->rr,rbdo->rr,rfuo->rr,rbuo->rr);

	/* Work with the FF boxes of the hypers */

        if( hyper->ff->lfd == NULL )
        {
                /*** create new boxes ***/

                lfdi->ff = alloc_box();
                rfdi->ff = alloc_box();
                lfui->ff = alloc_box();
                rfui->ff = alloc_box();
                lfdo->ff = alloc_box();
                rfdo->ff = alloc_box();
                lfuo->ff = alloc_box();
                rfuo->ff = alloc_box();

                make_sub_boxes(hyper->ff,lfdi->ff,rfdi->ff,lfui->ff,rfui->ff,
                	      	    lfdo->ff,rfdo->ff,lfuo->ff,rfuo->ff);
        }
        else
        {
                /*** box has already been split so use this info ***/

                lfdi->ff = hyper->ff->lfd;
                rfdi->ff = hyper->ff->rfd;
                lfui->ff = hyper->ff->lbd;
                rfui->ff = hyper->ff->rbd;
                lfdo->ff = hyper->ff->lfu;
                rfdo->ff = hyper->ff->rfu;
                lfuo->ff = hyper->ff->lbu;
                rfuo->ff = hyper->ff->rbu;
        }
        distribute_nodes(hyper->ff,lfdi->ff,rfdi->ff,lfui->ff,rfui->ff,
        			   lfdo->ff,rfdo->ff,lfuo->ff,rfuo->ff);

	/* Work with the BB boxes ob the hypers */

        if( hyper->bb->lfd == NULL )
        {
                /*** create new boxes ***/

                lbdi->bb = alloc_box();
                rbdi->bb = alloc_box();
                lbui->bb = alloc_box();
                rbui->bb = alloc_box();
                lbdo->bb = alloc_box();
                rbdo->bb = alloc_box();
                lbuo->bb = alloc_box();
                rbuo->bb = alloc_box();

                make_sub_boxes(hyper->bb,lbdi->bb,rbdi->bb,lbui->bb,rbui->bb,
                	      	    lbdo->bb,rbdo->bb,lbuo->bb,rbuo->bb);
        }
        else
        {
                /*** box has already been split so use this info ***/

                lbdi->bb = hyper->bb->lbd;
                rbdi->bb = hyper->bb->rbd;
                lbui->bb = hyper->bb->lfd;
                rbui->bb = hyper->bb->rfd;
                lbdo->bb = hyper->bb->lbu;
                rbdo->bb = hyper->bb->rbu;
                lbuo->bb = hyper->bb->lfu;
                rbuo->bb = hyper->bb->rfu;
        }
        distribute_nodes(hyper->bb,lbdi->bb,rbdi->bb,lbui->bb,rbui->bb,
        			   lbdo->bb,rbdo->bb,lbuo->bb,rbuo->bb);

	/* Work with the DD boxes ob the hypers */

        if( hyper->dd->lfd == NULL )
        {
                /*** create new boxes ***/

                lfdi->dd = alloc_box();
                rfdi->dd = alloc_box();
                lbdi->dd = alloc_box();
                rbdi->dd = alloc_box();
                lfdo->dd = alloc_box();
                rfdo->dd = alloc_box();
                lbdo->dd = alloc_box();
                rbdo->dd = alloc_box();

                make_sub_boxes(hyper->dd,lfdi->dd,rfdi->dd,lbdi->dd,rbdi->dd,
                	      	    lfdo->dd,rfdo->dd,lbdo->dd,rbdo->dd);
        }
        else
        {
                /*** box has already been split so use this info ***/

                lfdi->dd = hyper->dd->lfd;
                rfdi->dd = hyper->dd->rfd;
                lbdi->dd = hyper->dd->lbd;
                rbdi->dd = hyper->dd->rbd;
                lfdo->dd = hyper->dd->lfd;
                rfdo->dd = hyper->dd->rfd;
                lbdo->dd = hyper->dd->lfu;
                rbdo->dd = hyper->dd->rfu;
        }
        distribute_nodes(hyper->dd,lfdi->dd,rfdi->dd,lbdi->dd,rbdi->dd,
        			   lfdo->dd,rfdo->dd,lbdo->dd,rbdo->dd);

	/* Work with the UU boxes of the hypers */

        if( hyper->uu->lfu == NULL )
        {
                /*** create new boxes ***/

                lfui->uu = alloc_box();
                rfui->uu = alloc_box();
                lbui->uu = alloc_box();
                rbui->uu = alloc_box();
                lfuo->uu = alloc_box();
                rfuo->uu = alloc_box();
                lbuo->uu = alloc_box();
                rbuo->uu = alloc_box();

                make_sub_boxes(hyper->uu,lfui->uu,rfui->uu,lbui->uu,rbui->uu,
                	      	    lfuo->uu,rfuo->uu,lbuo->uu,rbuo->uu);
        }
        else
        {
                /*** box has already been split so use this info ***/

                lfui->uu = hyper->uu->lfd;
                rfui->uu = hyper->uu->rfd;
                lbui->uu = hyper->uu->lbd;
                rbui->uu = hyper->uu->rbd;
                lfuo->uu = hyper->uu->lfu;
                rfuo->uu = hyper->uu->rfu;
                lbuo->uu = hyper->uu->lfu;
                rbuo->uu = hyper->uu->rfu;
        }
        distribute_nodes(hyper->uu,lfui->uu,rfui->uu,lbui->uu,rbui->uu,
        			   lfuo->uu,rfuo->uu,lbuo->uu,rbuo->uu);

	/* Work with the II boxes of the hypers */

        if( hyper->ii->lfu == NULL )
        {
                /*** create new boxes ***/

                lfdi->ii = alloc_box();
                rfdi->ii = alloc_box();
                lbdi->ii = alloc_box();
                rbdi->ii = alloc_box();
                lfui->ii = alloc_box();
                rfui->ii = alloc_box();
                lbui->ii = alloc_box();
                rbui->ii = alloc_box();

                make_sub_boxes(hyper->ii,lfdi->ii,rfdi->ii,lbdi->ii,rbdi->ii,
                	      	    lfui->ii,rfui->ii,lbui->ii,rbui->ii);
        }
        else
        {
                /*** box has already been split so use this info ***/

                lfdi->ii = hyper->ii->lfd;
                rfdi->ii = hyper->ii->rfd;
                lbdi->ii = hyper->ii->lbd;
                rbdi->ii = hyper->ii->rbd;
                lfui->ii = hyper->ii->lfu;
                rfui->ii = hyper->ii->rfu;
                lbui->ii = hyper->ii->lfu;
                rbui->ii = hyper->ii->rfu;
        }
        distribute_nodes(hyper->ii,lfdi->ii,rfdi->ii,lbdi->ii,rbdi->ii,
        			   lfui->ii,rfui->ii,lbui->ii,rbui->ii);

	/* Work with the OO boxes of the hypers */

        if( hyper->oo->lfu == NULL )
        {
                /*** create new boxes ***/

                lfdo->oo = alloc_box();
                rfdo->oo = alloc_box();
                lbdo->oo = alloc_box();
                rbdo->oo = alloc_box();
                lfuo->oo = alloc_box();
                rfuo->oo = alloc_box();
                lbuo->oo = alloc_box();
                rbuo->oo = alloc_box();

                make_sub_boxes(hyper->oo,lfdo->oo,rfdo->oo,lbdo->oo,rbdo->oo,
                	      	    lfuo->oo,rfuo->oo,lbuo->oo,rbuo->oo);
        }
        else
        {
                /*** box has already been splot so use thos onfo ***/

                lfdo->oo = hyper->oo->lfd;
                rfdo->oo = hyper->oo->rfd;
                lbdo->oo = hyper->oo->lbd;
                rbdo->oo = hyper->oo->rbd;
                lfuo->oo = hyper->oo->lfu;
                rfuo->oo = hyper->oo->rfu;
                lbuo->oo = hyper->oo->lfu;
                rbuo->oo = hyper->oo->rfu;
        }
        distribute_nodes(hyper->oo,lfdo->oo,rfdo->oo,lbdo->oo,rbdo->oo,
        			   lfuo->oo,rfuo->oo,lbuo->oo,rbuo->oo);

	/*** Now create internal boxes left-right first ***/

	lfdi->rr = rfdi->ll = alloc_box();
	make_box(lfdi->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2,hyper->zl*2,
			hyper->wl*2,hyper->denom*2);

	lbdi->rr = rbdi->ll = alloc_box();
	make_box(lbdi->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2,hyper->denom*2);

	lfui->rr = rfui->ll = alloc_box();
	make_box(lfui->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	lbui->rr = rbui->ll = alloc_box();
	make_box(lbui->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	lfdo->rr = rfdo->ll = alloc_box();
	make_box(lfdo->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	lbdo->rr = rbdo->ll = alloc_box();
	make_box(lbdo->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	lfuo->rr = rfuo->ll = alloc_box();
	make_box(lfuo->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	lbuo->rr = rbuo->ll = alloc_box();
	make_box(lbuo->rr,BOX_RR,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	/* Now internal front/back boxes */

	lfdi->bb = lbdi->ff = alloc_box();
	make_box(lfdi->bb,BOX_BB,hyper->xl*2,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2,hyper->denom*2);

	rfdi->bb = rbdi->ff = alloc_box();
	make_box(rfdi->bb,BOX_BB,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2,hyper->denom*2);

	lfui->bb = lbui->ff = alloc_box();
	make_box(lfui->bb,BOX_BB,hyper->xl*2,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	rfui->bb = rbui->ff = alloc_box();
	make_box(rfui->bb,BOX_BB,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	lfdo->bb = lbdo->ff = alloc_box();
	make_box(lfdo->bb,BOX_BB,hyper->xl*2,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	rfdo->bb = rbdo->ff = alloc_box();
	make_box(rfdo->bb,BOX_BB,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	lfuo->bb = lbuo->ff = alloc_box();
	make_box(lfuo->bb,BOX_BB,hyper->xl*2,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	rfuo->bb = rbuo->ff = alloc_box();
	make_box(rfuo->bb,BOX_BB,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	/* Now internal down/up boxes */

	lfdi->uu = lfui->dd = alloc_box();
	make_box(lfdi->uu,BOX_UU,hyper->xl*2,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	rfdi->uu = rfui->dd = alloc_box();
	make_box(rfdi->uu,BOX_UU,hyper->xl*2+1,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	lbdi->uu = lbui->dd = alloc_box();
	make_box(lbdi->uu,BOX_UU,hyper->xl*2,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	rbdi->uu = rbui->dd = alloc_box();
	make_box(rbdi->uu,BOX_UU,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2,hyper->denom*2);

	lfdo->uu = lfuo->dd = alloc_box();
	make_box(lfdo->uu,BOX_UU,hyper->xl*2,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	rfdo->uu = rfuo->dd = alloc_box();
	make_box(rfdo->uu,BOX_UU,hyper->xl*2+1,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	lbdo->uu = lbuo->dd = alloc_box();
	make_box(lbdo->uu,BOX_UU,hyper->xl*2,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	rbdo->uu = rbuo->dd = alloc_box();
	make_box(rbdo->uu,BOX_UU,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	/* Now internal in/out boxes */

	lfdi->oo = lfdo->ii = alloc_box();
	make_box(lfdi->oo,BOX_OO,hyper->xl*2,hyper->yl*2,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	rfdi->oo = rfdo->ii = alloc_box();
	make_box(rfdi->oo,BOX_OO,hyper->xl*2+1,hyper->yl*2,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	lbdi->oo = lbdo->ii = alloc_box();
	make_box(lbdi->oo,BOX_OO,hyper->xl*2,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	rbdi->oo = rbdo->ii = alloc_box();
	make_box(rbdi->oo,BOX_OO,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2,
			hyper->wl*2+1,hyper->denom*2);

	lfui->oo = lfuo->ii = alloc_box();
	make_box(lfui->oo,BOX_OO,hyper->xl*2,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	rfui->oo = rfuo->ii = alloc_box();
	make_box(rfui->oo,BOX_OO,hyper->xl*2+1,hyper->yl*2,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	lbui->oo = lbuo->ii = alloc_box();
	make_box(lbui->oo,BOX_OO,hyper->xl*2,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);

	rbui->oo = rbuo->ii = alloc_box();
	make_box(rbui->oo,BOX_OO,hyper->xl*2+1,hyper->yl*2+1,hyper->zl*2+1,
			hyper->wl*2+1,hyper->denom*2);
}

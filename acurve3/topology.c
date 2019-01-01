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
#include <math.h>
#include "cells.h"
#define grballoc(node) ( node * ) malloc( sizeof(node) )
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

extern box_info whole_box;

/*
 * Function:	get_box
 * action:	returns a pointer to the box specified by xl,yl,zl,denom
 *		returns NULL if box does not exist.
 */

box_info *get_box(x,y,z,denom)
int x,y,z,denom;
{
	box_info *temp;

	if( x < 0 || x >= denom || y < 0 || y >= denom || z < 0 || z >= denom )
		return( NULL );
	temp = &whole_box;

	while( temp->denom < denom )
	{
	    if( 2*temp->denom*x < ( 2*temp->xl+1 )*denom )
	    {
		if( 2*temp->denom*y < ( 2*temp->yl+1 )*denom )
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
			temp = temp->lfd;
		    else
			temp = temp->lfu;
		}
		else
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
			temp = temp->lbd;
		    else
			temp = temp->lbu;
		}
	    }
	    else
	    {
		if( 2*temp->denom*y < ( 2*temp->yl+1 )*denom )
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
			temp = temp->rfd;
		    else
			temp = temp->rfu;
		}
		else
		{
		    if( 2*temp->denom*z < ( 2*temp->zl+1 )*denom )
			temp = temp->rbd;
		    else
			temp = temp->rbu;
		}
	    }

	    if( temp == NULL ) return( NULL );
	}
	if( ! temp->status ) return(NULL);
	return(temp);
}

/*
 * Function	get_existing_faces
 * action:	if any of the faces on box have been found on any
 *		other box use that information.
 */

get_existing_faces(box)
box_info *box;
{
	box_info *adjacient_box;

	adjacient_box = get_box(box->xl-1,box->yl,box->zl,box->denom);
	if( adjacient_box != NULL )
		box->ll = adjacient_box->rr;

	adjacient_box = get_box(box->xl+1,box->yl,box->zl,box->denom);
	if( adjacient_box != NULL )
		box->rr = adjacient_box->ll;

	adjacient_box = get_box(box->xl,box->yl-1,box->zl,box->denom);
	if( adjacient_box != NULL )
		box->ff = adjacient_box->bb;

	adjacient_box = get_box(box->xl,box->yl+1,box->zl,box->denom);
	if( adjacient_box != NULL )
		box->bb = adjacient_box->ff;

	adjacient_box = get_box(box->xl,box->yl,box->zl-1,box->denom);
	if( adjacient_box != NULL )
		box->dd = adjacient_box->uu;

	adjacient_box = get_box(box->xl,box->yl,box->zl+1,box->denom);
	if( adjacient_box != NULL )
		box->uu = adjacient_box->dd;
}

/*
 * Function:	create_new_faces
 * action:	if any of the faces have not already been found
 *		create the apropriate information.
 */

create_new_faces(box)
box_info *box;
{
	if( box->ll == NULL )
	{
		box->ll = alloc_face();
		make_box_face(box,FACE_LL,box->ll);
	}
	if( box->rr == NULL )
	{
		box->rr = alloc_face();
		make_box_face(box,FACE_RR,box->rr);
	}
	if( box->ff == NULL )
	{
		box->ff = alloc_face();
		make_box_face(box,FACE_FF,box->ff);
	}
	if( box->bb == NULL )
	{
		box->bb = alloc_face();
		make_box_face(box,FACE_BB,box->bb);
	}
	if( box->dd == NULL )
	{
		box->dd = alloc_face();
		make_box_face(box,FACE_DD,box->dd);
	}
	if( box->uu == NULL )
	{
		box->uu = alloc_face();
		make_box_face(box,FACE_UU,box->uu);
	}
}

/*
 * Function:	get_existing_edges
 * action:	if any of the edges lie on existing faces of existing
 *		boxes link the information.
 */

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

/*
 * Function:	create_new_face
 * action:	if any of the edge have not already been found
 *		create the apropriate information.
 */

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
 * Function:	get_nodes_on_face
 * action:	returns the number of nodes on a face,
 *		if count < 2 pute the solutions in the array.
 */

int get_nodes_on_face(face,nodes,count)
face_info *face;
node_info *nodes[2];
int count;
{
	node_info *temp;

	if( face == NULL ) return(count);
	temp = face->nodes;
	while( temp != NULL )
	{
		if( count < 2 ) nodes[count] = temp;
		++count;
		temp = temp->next;
	}
	if( face->lb != NULL )
	{
		count = get_nodes_on_face(face->lb,nodes,count);
		count = get_nodes_on_face(face->rb,nodes,count);
		count = get_nodes_on_face(face->lt,nodes,count);
		count = get_nodes_on_face(face->rt,nodes,count);
	}
	return(count);
}

int count_nodes_on_face(face)
face_info *face;
{
	int count=0;
	node_info *temp;

	if( face == NULL ) return(0);
	temp = face->nodes;
	while( temp != NULL )
	{
		++count;
		temp = temp->next;
	}
	if( face->lb != NULL )
	{
		count += count_nodes_on_face(face->lb)
		 + count_nodes_on_face(face->rb)
		 + count_nodes_on_face(face->lt)
		 + count_nodes_on_face(face->rt);
	}
	return(count);
}

/*
 * Function:	get_nth_node_on_face
 * action:	finds the nth node on a face and returns a pointer to it.
 *		If there is no nth node return (nil).
 */

node_info *get_nth_node_on_face(face,n)
face_info *face;
int n;
{
	node_info *temp;

	if( face == NULL ) return(NULL);
	temp = face->nodes;
	while( temp != NULL )
	{
		--n;
		if( n == 0 ) return(temp);
		temp = temp->next;
	}

	/* Now try sub faces */

	if( face->lb != NULL )
	{
		temp = get_nth_node_on_face(face->lb,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_face(face->lb);
	}
	if( face->rb != NULL )
	{
		temp = get_nth_node_on_face(face->rb,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_face(face->rb);
	}
	if( face->lt != NULL )
	{
		temp = get_nth_node_on_face(face->lt,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_face(face->lt);
	}
	if( face->rt != NULL )
	{
		temp = get_nth_node_on_face(face->rt,n);
		if(temp != NULL ) return(temp);
		n -= count_nodes_on_face(face->rt);
	}

	/* couldn't find an nth node */

	return(NULL);
}

/*
 * Function:	get_nodes_on_box
 * action:	returns the number of nodes round a box,
 *		the first two of these solutions are put in the array nodes.
 */

int get_nodes_on_box_faces(box,nodes)
box_info *box;
node_info *nodes[2];
{
	int count;

	count = 0;
	count = get_nodes_on_face(box->ll,nodes,count);
	count = get_nodes_on_face(box->rr,nodes,count);
	count = get_nodes_on_face(box->ff,nodes,count);
	count = get_nodes_on_face(box->bb,nodes,count);
	count = get_nodes_on_face(box->dd,nodes,count);
	count = get_nodes_on_face(box->uu,nodes,count);
	return(count);
}

/*
 * Function:	get_nth_node_on_box
 * action:	returns the nth node  on the faces of the box.
 */

node_info *get_nth_node_on_box(box,n)
box_info *box;
int n;
{
	node_info *temp;

	temp = get_nth_node_on_face(box->ll,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_face(box->ll);

	temp = get_nth_node_on_face(box->rr,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_face(box->rr);

	temp = get_nth_node_on_face(box->ff,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_face(box->ff);

	temp = get_nth_node_on_face(box->bb,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_face(box->bb);

	temp = get_nth_node_on_face(box->dd,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_face(box->dd);

	temp = get_nth_node_on_face(box->uu,n);
	if( temp != NULL ) return(temp);
	n -= count_nodes_on_face(box->uu);
	return(NULL);
}

/*
 * Function:	split_face
 * action:	takes information about face and puts it in the
 *		sub faces. Does not find internal edges.
 */

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

/*
 * Function:	distribute_nodes
 * action:	take all the nodes on face and put them on the corect subface.
 */

distribute_nodes(face,face1,face2,face3,face4)
face_info *face,*face1,*face2,*face3,*face4;
{
	node_info *node;

	while( face->nodes != NULL )
	{
	    node = face->nodes;
	    switch(face->type)
	    {
	    case FACE_LL: case FACE_RR:
		if(node->sol->yl * face4->denom
			<  node->sol->denom * face4->yl )

			if(node->sol->zl * face4->denom
				< node->sol->denom * face4->zl )
			{
				/* add to face1 */

				face->nodes = node->next;
				node->next = face1->nodes;
				face1->nodes = node;
			}
			else
			{
				/* add to face 3 */
		
				face->nodes = node->next;
				node->next = face3->nodes;
				face3->nodes = node;
			}
		else
			if(node->sol->zl * face4->denom
				< node->sol->denom * face4->zl )
			{
				/* add to face2 */

				face->nodes = node->next;
				node->next = face2->nodes;
				face2->nodes = node;
			}
			else
			{
				/* add to face 4 */
		
				face->nodes = node->next;
				node->next = face4->nodes;
				face4->nodes = node;
			}
		break;

	    case FACE_FF: case FACE_BB:
		if(node->sol->xl * face4->denom
			<  node->sol->denom * face4->xl )

			if(node->sol->zl * face4->denom
				< node->sol->denom * face4->zl )
			{
				/* add to face1 */

				face->nodes = node->next;
				node->next = face1->nodes;
				face1->nodes = node;
			}
			else
			{
				/* add to face 3 */
		
				face->nodes = node->next;
				node->next = face3->nodes;
				face3->nodes = node;
			}
		else
			if(node->sol->zl * face4->denom
				< node->sol->denom * face4->zl )
			{
				/* add to face2 */

				face->nodes = node->next;
				node->next = face2->nodes;
				face2->nodes = node;
			}
			else
			{
				/* add to face 4 */
		
				face->nodes = node->next;
				node->next = face4->nodes;
				face4->nodes = node;
			}
		break;

	    case FACE_UU: case FACE_DD:
		if(node->sol->xl * face4->denom
			<  node->sol->denom * face4->xl )

			if(node->sol->yl * face4->denom
				< node->sol->denom * face4->yl )
			{
				/* add to face1 */

				face->nodes = node->next;
				node->next = face1->nodes;
				face1->nodes = node;
			}
			else
			{
				/* add to face 3 */
		
				face->nodes = node->next;
				node->next = face3->nodes;
				face3->nodes = node;
			}
		else
			if(node->sol->yl * face4->denom
				< node->sol->denom * face4->yl )
			{
				/* add to face2 */

				face->nodes = node->next;
				node->next = face2->nodes;
				face2->nodes = node;
			}
			else
			{
				/* add to face 4 */
		
				face->nodes = node->next;
				node->next = face4->nodes;
				face4->nodes = node;
			}
		break;
	    }  /* end switch */
	} /* end while */
}

/* Function:	split_box
 * action:	takes the information from box and puts it into
 *		the sub boxes. Creates internal faces but does not
 *		find the internal solutions.
 */

split_box(box,lfd,rfd,lbd,rbd,lfu,rfu,lbu,rbu)
box_info *box,*lfd,*rfd,*lbd,*rbd,*lfu,*rfu,*lbu,*rbu;
{
	if( box->ll->lb == NULL )
	{
		/*** create new faces ***/

		lfd->ll = alloc_face();
		lbd->ll = alloc_face();
		lfu->ll = alloc_face();
		lbu->ll = alloc_face();
	
		make_sub_faces(box->ll,lfd->ll,lbd->ll,lfu->ll,lbu->ll);
		split_face(box->ll,lfd->ll,lbd->ll,lfu->ll,lbu->ll);
	}
	else
	{
		/*** face has already been split so use this info ***/

		lfd->ll = box->ll->lb;
		lbd->ll = box->ll->rb;
		lfu->ll = box->ll->lt;
		lbu->ll = box->ll->rt;
	}
	distribute_nodes(box->ll,lfd->ll,lbd->ll,lfu->ll,lbu->ll);

	if( box->rr->lb == NULL )
	{
		/*** create new faces ***/

		rfd->rr = alloc_face();
		rbd->rr = alloc_face();
		rfu->rr = alloc_face();
		rbu->rr = alloc_face();
	
		make_sub_faces(box->rr,rfd->rr,rbd->rr,rfu->rr,rbu->rr);
		split_face(box->rr,rfd->rr,rbd->rr,rfu->rr,rbu->rr);
	}
	else
	{
		/*** face has already been split so use this info ***/

		rfd->rr = box->rr->lb;
		rbd->rr = box->rr->rb;
		rfu->rr = box->rr->lt;
		rbu->rr = box->rr->rt;
	}
	distribute_nodes(box->rr,rfd->rr,rbd->rr,rfu->rr,rbu->rr);

	if( box->ff->lb == NULL )
	{
		/*** create new faces ***/

		lfd->ff = alloc_face();
		rfd->ff = alloc_face();
		lfu->ff = alloc_face();
		rfu->ff = alloc_face();
	
		make_sub_faces(box->ff,lfd->ff,rfd->ff,lfu->ff,rfu->ff);
		split_face(box->ff,lfd->ff,rfd->ff,lfu->ff,rfu->ff);
	}
	else
	{
		/*** face has already been split so use this info ***/

		lfd->ff = box->ff->lb;
		rfd->ff = box->ff->rb;
		lfu->ff = box->ff->lt;
		rfu->ff = box->ff->rt;
	}
	distribute_nodes(box->ff,lfd->ff,rfd->ff,lfu->ff,rfu->ff);

	if( box->bb->lb == NULL )
	{
		/*** create new faces ***/

		lbd->bb = alloc_face();
		rbd->bb = alloc_face();
		lbu->bb = alloc_face();
		rbu->bb = alloc_face();
	
		make_sub_faces(box->bb,lbd->bb,rbd->bb,lbu->bb,rbu->bb);
		split_face(box->bb,lbd->bb,rbd->bb,lbu->bb,rbu->bb);
	}
	else
	{
		/*** face has already been split so use this info ***/

		lbd->bb = box->bb->lb;
		rbd->bb = box->bb->rb;
		lbu->bb = box->bb->lt;
		rbu->bb = box->bb->rt;
	}
	distribute_nodes(box->bb,lbd->bb,rbd->bb,lbu->bb,rbu->bb);

	if( box->dd->lb == NULL )
	{
		/*** create new faces ***/

		lfd->dd = alloc_face();
		rfd->dd = alloc_face();
		lbd->dd = alloc_face();
		rbd->dd = alloc_face();
	
		make_sub_faces(box->dd,lfd->dd,rfd->dd,lbd->dd,rbd->dd);
		split_face(box->dd,lfd->dd,rfd->dd,lbd->dd,rbd->dd);
	}
	else
	{
		/*** face has already been split so use this info ***/

		lfd->dd = box->dd->lb;
		rfd->dd = box->dd->rb;
		lbd->dd = box->dd->lt;
		rbd->dd = box->dd->rt;
	}
	distribute_nodes(box->dd,lfd->dd,rfd->dd,lbd->dd,rbd->dd);

	if( box->uu->lb == NULL )
	{
		/*** create new faces ***/

		lfu->uu = alloc_face();
		rfu->uu = alloc_face();
		lbu->uu = alloc_face();
		rbu->uu = alloc_face();
	
		make_sub_faces(box->uu,lfu->uu,rfu->uu,lbu->uu,rbu->uu);
		split_face(box->uu,lfu->uu,rfu->uu,lbu->uu,rbu->uu);
	}
	else
	{
		/*** face has already been split so use this info ***/

		lfu->uu = box->uu->lb;
		rfu->uu = box->uu->rb;
		lbu->uu = box->uu->lt;
		rbu->uu = box->uu->rt;
	}
	distribute_nodes(box->uu,lfu->uu,rfu->uu,lbu->uu,rbu->uu);

	/*** Now create internal faces ***/

	lfd->rr = rfd->ll = alloc_face();
	make_face(lfd->rr,FACE_RR,box->xl*2+1,box->yl*2,box->zl*2,
			box->denom*2);

	lbd->rr = rbd->ll = alloc_face();
	make_face(lbd->rr,FACE_RR,box->xl*2+1,box->yl*2+1,box->zl*2,
			box->denom*2);

	lfu->rr = rfu->ll = alloc_face();
	make_face(lfu->rr,FACE_RR,box->xl*2+1,box->yl*2,box->zl*2+1,
			box->denom*2);

	lbu->rr = rbu->ll = alloc_face();
	make_face(lbu->rr,FACE_RR,box->xl*2+1,box->yl*2+1,box->zl*2+1,
			box->denom*2);

	/* Now internal front/back faces */

	lfd->bb = lbd->ff = alloc_face();
	make_face(lfd->bb,FACE_BB,box->xl*2,box->yl*2+1,box->zl*2,
			box->denom*2);

	rfd->bb = rbd->ff = alloc_face();
	make_face(rfd->bb,FACE_BB,box->xl*2+1,box->yl*2+1,box->zl*2,
			box->denom*2);

	lfu->bb = lbu->ff = alloc_face();
	make_face(lfu->bb,FACE_BB,box->xl*2,box->yl*2+1,box->zl*2+1,
			box->denom*2);

	rfu->bb = rbu->ff = alloc_face();
	make_face(rfu->bb,FACE_BB,box->xl*2+1,box->yl*2+1,box->zl*2+1,
			box->denom*2);

	/* Now internal down/up faces */

	lfd->uu = lfu->dd = alloc_face();
	make_face(lfd->uu,FACE_UU,box->xl*2,box->yl*2,box->zl*2+1,
			box->denom*2);

	rfd->uu = rfu->dd = alloc_face();
	make_face(rfd->uu,FACE_UU,box->xl*2+1,box->yl*2,box->zl*2+1,
			box->denom*2);

	lbd->uu = lbu->dd = alloc_face();
	make_face(lbd->uu,FACE_UU,box->xl*2,box->yl*2+1,box->zl*2+1,
			box->denom*2);

	rbd->uu = rbu->dd = alloc_face();
	make_face(rbd->uu,FACE_UU,box->xl*2+1,box->yl*2+1,box->zl*2+1,
			box->denom*2);
}

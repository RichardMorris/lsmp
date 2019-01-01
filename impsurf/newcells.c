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
/*	sub-routine to handle the topology of boxes, which have 	*/
/*	faces, edges and verticies surronding them.			*/
/*	Any cell (a box, a face, an edge, a vertex) is defined by 	*/
/*	four integers: xl,yl,zl and denom. The bottom, front, left 	*/
/*	cornor of the cell is given by (xl/denom,yl/denom,zl/denom).	*/
/*	This gives a very compact identifyer for the cell. Each cell	*/
/*	also has a type.						*/
/*									*/
/*	Boxes are defined in an oct-tree each box has eight pointers	*/
/*	to sub-boxes, it also has pointers to its sub faces, and to a	*/
/*	a list of solutions.						*/
/*									*/
/*	Faces have pointers to a list of solutions.			*/
/*									*/
/*	Edges have the same type as solutions, the edges can be	joined	*/
/*	in a number of linked lists, the pointer next is used as just	*/
/*	a general list of solutions, linkXY_FACE etc. are used for	*/
/*	lists of solutions just lying on the XY face, say.		*/
/*									*/
/************************************************************************/

#define I_AM_CELLS
#include <stdio.h>
#include <math.h>
#include "cells.h"
/*
#define PRINT_INCLUDE_LINK
#define JOIN_NODES_IN_COLLECT
#define MAKE_NODES
#define PRINT_INCLUDE_LINK_NODES
#define PRINT_FREE
*/
#define MEMSET

#define grballoc(node) ( node * ) malloc( sizeof(node) )
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
extern node_info *get_nth_node_on_face();
extern sol_info *get_nth_sol_on_face();
extern int count_nodes_on_face();

/************************************************************************/
/*									*/
/*	We start with procedures to print out the different cells	*/
/*									*/
/************************************************************************/

/*
 * Function:	printsoltype
 * action:	print on the standard output the type in hrf(human readable)
 */

printsoltype(type)
soltype type;
{
	switch(type)
	{
	case NONE: fprintf(stderr,"NONE"); break;
	case VERTEX: fprintf(stderr,"VERTEX"); break;
	case X_AXIS: fprintf(stderr,"X_AXIS"); break;
	case Y_AXIS: fprintf(stderr,"Y_AXIS"); break;
	case Z_AXIS: fprintf(stderr,"Z_AXIS"); break;
	case FACE_LL: fprintf(stderr,"FACE_LL"); break;
	case FACE_RR: fprintf(stderr,"FACE_RR"); break;
	case FACE_FF: fprintf(stderr,"FACE_FF"); break;
	case FACE_BB: fprintf(stderr,"FACE_BB"); break;
	case FACE_DD: fprintf(stderr,"FACE_DD"); break;
	case FACE_UU: fprintf(stderr,"FACE_UU"); break;
	case BOX: fprintf(stderr,"BOX"); break;
	default: fprintf(stderr,"Unknow type %d\n",type);
	}
}

/*
 * Function:	print_sol
 * action	print out a single solution.
 */

print_sol(temp)
sol_info *temp;
{
	if(temp == NULL ) 
	{
		fprintf(stderr,"\tNULL solution\n");
		return;
	}
	fprintf(stderr,"\tsol type ");
	printsoltype(temp->type);
	fprintf(stderr," (%hd,%hd,%hd)/%hd ",
		temp->xl,temp->yl,temp->zl,temp->denom);
	switch(temp->type)
	{
	case X_AXIS: case Y_AXIS: case Z_AXIS:
		fprintf(stderr,"root %lf ",temp->root);
		break;
	case FACE_LL: case FACE_RR: case FACE_FF: case FACE_BB:
	case FACE_DD: case FACE_UU:
		fprintf(stderr,"roots %lf %lf ",temp->root,temp->root2);
		break;
	case BOX:
		fprintf(stderr,"roots %lf %lf %lf ",
			temp->root,temp->root2,temp->root3);
		break;
	}
	fprintf(stderr,"deriv %f %f %f",
		temp->dx,temp->dy,temp->dz );
	fprintf(stderr,"\n");
}

/*
 * Function:	printsols_on_edge(edge)
 * action:	prints the solutions lying on the edge.
 */

printsols_on_edge(edge)
edge_info *edge;
{
	if( edge == NULL ) return;
	if(edge->sol != NULL ) print_sol(edge->sol);
	if(edge->left != NULL )
	{
		printsols_on_edge(edge->left);
	}
	if(edge->right != NULL )
	{
		printsols_on_edge(edge->right);
	}
}

printvert(vert_info *vert)
{
	if( vert == NULL )
	{
		fprintf(stderr,"NULL vert\n");
		return;
	}
	fprintf(stderr," (%hd,%hd,%hd)/%hd,status %x\n",
                vert->xl,vert->yl,vert->zl,vert->denom,vert->status);
	if( vert->status & FOUND_VAL )
		fprintf(stderr,"val %f ",vert->val);
	if( vert->status & FOUND_DX )
		fprintf(stderr,"dx %f ",vert->dx);
	if( vert->status & FOUND_DY )
		fprintf(stderr,"dy %f ",vert->dy);
	if( vert->status & FOUND_DZ )
		fprintf(stderr,"dz %f ",vert->dz);
	if(vert->status ) fprintf(stderr,"\n");
}

/*
 * Function:	printedge(edge)
 * action:	prints the edge and all which lies apon it.
 */

printedge(edge)
edge_info *edge;
{
	if( edge == NULL )
	{
		fprintf(stderr,"NULL edge\n");
		return;
	}
	fprintf(stderr,"EDGE: type ");
	printsoltype(edge->type);
	fprintf(stderr," (%hd,%hd,%hd)/%hd,status %hd\n",
		edge->xl,edge->yl,edge->zl,edge->denom,edge->status);
	printsols_on_edge(edge);
}

/*
 * Function:	printlinks
 * action:	print out a list of links.
 */

printlinks(link)
link_info *link;
{
	int count = 0;

	if(link == NULL )
		fprintf(stderr,"no links\n");

	while(link != NULL )
	{
		fprintf(stderr,"Link no %d, status",++count);
		switch(link->status)
		{
		case NODE:	fprintf(stderr," NODE\n"); break;
		case LINK:	fprintf(stderr," LINK\n"); break;
		default:	fprintf(stderr," unknow status %hd\n",link->status);
		}
		print_sol(link->A);	
		print_sol(link->B);
		link = link->next;
        }
	fprintf(stderr,"\n");
}

print_node(node)
node_info *node;
{
	if( node == NULL )
	{
		fprintf(stderr,"node == NULL\n");
	}
	else
	{
		fprintf(stderr,"Node: status %hd ",node->status);
		print_sol(node->sol);
	}
}

/*
 * Function:	printnodes
 * action:	print out a list of nodes.
 */

printnodes(node)
node_info *node;
{
	if(node == NULL )
		fprintf(stderr,"no nodes\n");

	while(node != NULL )
	{
		print_node(node);	
		node = node->next;
        }
}

/*
 * Function:	printnodes_on_face(face)
 * action:	prints the nodes lying on the face.
 */

printnodes_on_face(face)
face_info *face;
{
	if( face == NULL ) return;
	if(face->nodes != NULL ) printnodes(face->nodes);
	if(face->lb != NULL ) printnodes_on_face(face->lb);
	if(face->rb != NULL ) printnodes_on_face(face->rb);
	if(face->lt != NULL ) printnodes_on_face(face->lt);
	if(face->rt != NULL ) printnodes_on_face(face->rt);
}

/*
 * Function:	printface
 * action:	print out the face and its solutions.
 */

printface(face)
face_info *face;
{
	if( face == NULL )
	{
		fprintf(stderr,"NULL face\n");
		return;
	}
	fprintf(stderr,"FACE: type ");
	printsoltype(face->type);
	fprintf(stderr," (%hd,%hd,%hd)/%hd,status %hd\n",
		face->xl,face->yl,face->zl,face->denom,face->status);
	printedge(face->x_low);
	printedge(face->x_high);
	printedge(face->y_low);
	printedge(face->y_high);
	printnodes_on_face(face);
	printlinks(face->links);
}

printnode_links(node_link)
node_link_info *node_link;
{
	if( node_link == NULL )
		fprintf(stderr,"No node links\n");

	while(node_link != NULL )
	{
		fprintf(stderr,"NODE LINK: status %hd\n",node_link->status);
		fprintf(stderr,"\t");
		print_node(node_link->A);	
		fprintf(stderr,"\t");
		print_node(node_link->B);
		node_link = node_link->next;
        }
	fprintf(stderr,"\n");
}

printsing(sing)
sing_info *sing;
{
	if(sing == NULL ) fprintf(stderr,"no singularity\n");
	else
	{
		fprintf(stderr,"SING: status %hd",sing->status);
		print_sol(sing->sing);
	}
}

printsings(sings)
sing_info *sings;
{
	if(sings == NULL) fprintf(stderr,"No singularities\n");
	while(sings!=NULL)
	{
		printsing(sings);
		sings = sings->next;
	}
}

/*
 * Function:	printbox
 * action:	prints out information about a box.
 */

printbox(box)
box_info *box;
{
	if(box == NULL)
	{
		fprintf(stderr,"NULL box\n");
		return;
	}
	fprintf(stderr,"BOX: (%hd,%hd,%hd)/%hd status\n",
		box->xl,box->yl,box->zl,box->denom,box->status);
	fprintf(stderr,"face ll :"); printface(box->ll);
	fprintf(stderr,"face rr :"); printface(box->rr);
	fprintf(stderr,"face ff :"); printface(box->ff);
	fprintf(stderr,"face bb :"); printface(box->bb);
	fprintf(stderr,"face dd :"); printface(box->dd);
	fprintf(stderr,"face uu :"); printface(box->uu);
	printsings(box->sings);
	printnode_links(box->node_links);
}

/************************************************************************/
/*									*/
/*	Now procedures to allocate and free cells			*/
/*									*/
/*     Here we operate a lossy freeing scheme for each cell type	*/
/*	we have a list of currently freed items which starts with	*/
/*		cell_info *FreeCell;					*/
/*	and with a end marker 						*/
/*		cell_info last_cell;					*/
/*	when a cell is in use the pointer 				*/
/*		cell_info *nextfree;					*/
/*	is set to NULL. When the cell is not in use it is added to the  */
/*	free list and the pointer is set to the next cell in the list   */
/*	or &last_cell are no free cells. We can easily see if a cell 	*/
/*	has already been free'd if this pointer is not NULL.		*/
/*	hence we can implement a sloppy freeing scheme, with out 	*/
/*	causing problems with freeing the same cell twice.		*/
/*									*/
/************************************************************************/

sol_info last_sol;
sol_info *Free_sols = &last_sol;
vert_info last_vert;
vert_info *Free_verts = &last_vert;
edge_info last_edge;
edge_info *Free_edges = &last_edge;
link_info last_link;
link_info *Free_links = &last_link;
node_info last_node;
node_info *Free_nodes = &last_node;
face_info last_face;
face_info *Free_faces = &last_face;
node_link_info last_node_link;
node_link_info *Free_node_links = &last_node_link;
sing_info last_sing;
sing_info *Free_sings = &last_sing;
box_info last_box;
box_info *Free_boxs = &last_box;

/*** count is number of curently in-use objects, max is the
	total number ever referenced, new is number of mem allocations ***/

int	solcount=0, solmax=0, solnew=0,
	vertcount=0, vertmax=0, vertnew=0,
	edgecount=0, edgemax=0, edgenew=0,
	linkcount=0, linkmax=0, linknew=0,
	nodecount=0, nodemax=0, nodenew=0,
	facecount=0, facemax=0, facenew=0,
	node_linkcount=0, node_linkmax=0, node_linknew=0,
	singcount=0, singmax=0, singnew=0,
	boxcount=0, boxmax=0, boxnew=0;

init_cells()
{
#ifdef PRINT_FREE
	solcount = solmax = solnew =
	vertcount = vertmax = vertnew =
	edgecount = edgemax = edgenew =
	linkcount = linkmax = linknew =
	nodecount = nodemax = nodenew =
	facecount = facemax = facenew =
	node_linkcount = node_linkmax = node_linknew =
	singcount = singmax = singnew =
	boxcount = boxmax = boxnew=0;
#endif
}

fini_cells()
{
#ifdef PRINT_FREE
	fprintf(stderr,"sol (c,m,n) %d %d %d vert %d %d %d edge %d %d %d\n",
	solcount, solmax, solnew,
	vertcount, vertmax, vertnew,
	edgecount, edgemax, edgenew);
	fprintf(stderr,"link %d %d %d node %d %d %d face %d %d %d\n",
	linkcount, linkmax, linknew,
	nodecount, nodemax, nodenew,
	facecount, facemax, facenew);
	fprintf(stderr,"node_link %d %d %d sing %d %d %d box %d %d %d\n",
	node_linkcount, node_linkmax, node_linknew,
	singcount, singmax, singnew,
	boxcount, boxmax, boxnew);
#endif
}


#define ALLOCMACRO(gumbie) gumbie ## _info *alloc ## gumbie () \
{ \
	 gumbie ## _info * gumbie; \
 \
	++ gumbie ## count; \
	++ gumbie ## max; \
	if(Free_ ## gumbie ## s != &last_ ## gumbie ) \
	{ \
		gumbie = Free_ ## gumbie ## s; \
		Free_ ## gumbie ## s = Free_ ## gumbie ## s->nextfree; \
                memset((void *) gumbie,0,sizeof( gumbie ## _info)); \
		gumbie->nextfree = NULL;  \
 \
	} \
	else \
	{ \
		++ gumbie ## new; \
		 gumbie = ( gumbie ## _info *) malloc(sizeof(gumbie ## _info)); \
                memset((void *) gumbie,0,sizeof( gumbie ## _info)); \
		if( gumbie == NULL ) \
		{ \
			fprintf(stderr,"ERROR: alloc" #gumbie " NULL allocation, malloc failed, ( " #gumbie "new %d).\n",gumbie ## new); \
			exit(-1); \
		} \
		gumbie->nextfree = NULL;  \
	} \
	return( gumbie ); \
}


/*
 * Function:	free_vert
 * action:	frees an vert and all the sols on it.
 */

#define FREEMACRO(gumbie) free ## gumbie( gumbie ## _info *gumbie) \
{ \
        if(gumbie== NULL  ) return; \
	if(gumbie->nextfree != NULL) return; \
        -- gumbie ## count; \
        gumbie->nextfree = Free_ ## gumbie ## s; \
        Free_ ## gumbie ## s =  gumbie ; \
} 
 
ALLOCMACRO(sol)
FREEMACRO(sol)
ALLOCMACRO(vert)
FREEMACRO(vert)
ALLOCMACRO(edge)
FREEMACRO(edge)
ALLOCMACRO(link)
FREEMACRO(link)
ALLOCMACRO(node)
FREEMACRO(node)
ALLOCMACRO(face)
FREEMACRO(face)
ALLOCMACRO(node_link)
FREEMACRO(node_link)
ALLOCMACRO(sing)
FREEMACRO(sing)
ALLOCMACRO(box)
FREEMACRO(box)

/* These macros produce functions
	vert_info *allocvert() { ... }
	freevert (  vert_info * vert ) {}
   which only free the cell and not any of its dependants 
*/

/* Now functions which free an object and all its dependants */

free_sol(sol_info *sol) { freesol(sol); }

free_vert(vert_info *vert) { freevert(vert); }

free_edge(edge_info *edge)
{
	if(edge==NULL) return;
	freesol(edge->sol);
	freevert(edge->low);
	freevert(edge->high);
	free_edge(edge->left);
	free_edge(edge->right);
	edge->sol = NULL;
	edge->low = edge->high = NULL;
	edge->left = edge->right = NULL;
	freeedge(edge);
}

free_links(link_info *link)
{
	link_info *tmp;

	while(link != NULL)
	{
		tmp = link->next;
		link->next = NULL;
		link->A = link->B = NULL;
		freelink(link);
		link = tmp;
	}
}

free_node(node_info *node)
{
	if(node==NULL) return;
	node->sol = NULL;
	node->next = NULL;
	freenode(node);
}

free_face(face_info *face)
{
	if(face==NULL) return;
	free_edge(face->x_low);
	free_edge(face->y_low);
	free_edge(face->x_high);
	free_edge(face->y_high);
	free_links(face->links);
	free_node(face->nodes);
	free_face(face->lb);
	free_face(face->rb);
	free_face(face->lt);
	free_face(face->rt);
	face->x_low = face->y_low = face->x_high = face->y_high =  NULL;
	face->links = NULL; face->nodes =  NULL;
	face->lb = face->rb = face->lt = face->rt =  NULL;
	freeface(face);
}
	
free_node_links(node_link_info *node_link)
{
	node_link_info *tmp;

	while(node_link != NULL)
	{
		tmp = node_link->next;
		node_link->next = NULL;
		freenode_link(node_link);
		node_link = tmp;
	}
	freenode_link(node_link);
}

free_sing(sing_info *sing)
{
	if(sing==NULL) return;
	sing->sing = NULL;
	sing->next = NULL;
	freesing(sing);
}

free_box(box_info *box)
{
	if(box==NULL) return;
	free_face(box->ll);
	free_face(box->rr);
	free_face(box->ff);
	free_face(box->bb);
	free_face(box->dd);
	free_face(box->uu);
	free_box(box->lfd);
	free_box(box->lfu);
	free_box(box->lbd);
	free_box(box->lbu);
	free_box(box->rfd);
	free_box(box->rfu);
	free_box(box->rbd);
	free_box(box->rbu);
	free_node_links(box->node_links);
	free_sing(box->sings);

	box->ll = box->rr = box->ff = box->bb = box->dd = box->uu = NULL;
	box->lfd = box->lfu = box->lbd = box->lbu = 
	box->rfd = box->rfu = box->rbd = box->rbu = NULL;
	box->node_links = NULL; box->sings =  NULL;

	freebox(box);
}

/* Now functions to free the bottom lhs of an object */

free_bits_of_edge(edge_info *edge)
{
	if(edge == NULL) return;

	freevert(edge->low);
	freesol(edge->sol);
	free_edge(edge->left);
	free_bits_of_edge(edge->right);
	edge->sol = NULL;
	edge->low = edge->high = NULL;
	edge->left = edge->right = NULL;
	freeedge(edge);
}
	
/*
 * Function:	free_bits_of_face
 * action:	free those bits of face which won't be used again,
 *		only call for LL,FF and DD faces of a box.
 */

free_bits_of_face(face)
face_info *face;
{
	if(face==NULL) return;
	free_bits_of_edge(face->x_low);
	free_bits_of_edge(face->y_low);
	free_links(face->links);
	free_node(face->nodes);
	free_face(face->lb);
	free_bits_of_face(face->rb);
	free_bits_of_face(face->lt);
	free_bits_of_face(face->rt);
	face->x_low = face->y_low = face->x_high = face->y_high =  NULL;
	face->links = NULL; face->nodes =  NULL;
	face->lb = face->rb = face->lt = face->rt =  NULL;
	freeface(face);
}

/*
 *
 * Function:	free_bits_of_box
 * action:	free those bits of the box which won't be used again namely:
 *		box->sings,
 *		box->node_links,
 *		bits_of box->ll,
 *		bits_of box->ff,
 *		bits_of box->dd,
 */

free_bits_of_box(box)
box_info *box;
{
	if(box==NULL) return;
	free_bits_of_face(box->ll);
	free_bits_of_face(box->ff);
	free_bits_of_face(box->dd);
	free_box(box->lfd);
	free_bits_of_box(box->lfu);
	free_bits_of_box(box->lbd);
	free_bits_of_box(box->lbu);
	free_bits_of_box(box->rfd);
	free_bits_of_box(box->rfu);
	free_bits_of_box(box->rbd);
	free_bits_of_box(box->rbu);
	free_node_links(box->node_links);
	free_sing(box->sings);

	box->ll = box->rr = box->ff = box->bb = box->dd = box->uu = NULL;
	box->lfd = box->lfu = box->lbd = box->lbu = 
	box->rfd = box->rfu = box->rbd = box->rbu = NULL;
	box->node_links = NULL; box->sings =  NULL;

	freebox(box);
}

	
/************************************************************************/
/*									*/
/*	Now procedures to create other cells				*/
/*									*/
/************************************************************************/

/*
 * Function:	make_vert
 *		define an vert
 */

make_vert(vert,xl,yl,zl,denom)
vert_info *vert;
int xl,yl,zl,denom;
{
	vert->xl = xl;
	vert->yl = yl;
	vert->zl = zl;
	vert->denom = denom;
	vert->status = 0x00;
}

/*
/*
 * Function:	make_edge
 *		define an edge
 */

make_edge(edge,type,xl,yl,zl,denom)
edge_info *edge;
soltype type;
int xl,yl,zl,denom;
{
	edge->type = type;
	edge->xl = xl;
	edge->yl = yl;
	edge->zl = zl;
	edge->denom = denom;
	edge->status = FALSE;
	edge->sol =  NULL;
	edge->left = edge->right =  NULL;
	edge->low = edge->high = NULL;
}

/*
 * Function:	make_edge_on_face
 * action:	fill loctaion pointed to by sol with information
 *		about the edge on the face refered to by code.
 *
 *		Also finds vertex info
 */

make_face_edge(face,code,edge)
face_info *face;
int code;
edge_info *edge;
{
	switch(face->type)
	{
	case FACE_LL: case FACE_RR:
		switch(code)
		{
		case X_LOW:
			make_edge(edge,Z_AXIS,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case X_HIGH:
			make_edge(edge,Z_AXIS,face->xl,face->yl+1,
				face->zl,face->denom);
			break;
		case Y_LOW:
			make_edge(edge,Y_AXIS,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case Y_HIGH:
			make_edge(edge,Y_AXIS,face->xl,face->yl,
				face->zl+1,face->denom);
			break;
		case MID_FACE:
			make_edge(edge,face->type,face->xl,face->yl,
				face->zl,face->denom);
			break;
		}
		break;

	case FACE_FF: case FACE_BB:
		switch(code)
		{
		case X_LOW:
			make_edge(edge,Z_AXIS,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case X_HIGH:
			make_edge(edge,Z_AXIS,face->xl+1,face->yl,
				face->zl,face->denom);
			break;
		case Y_LOW:
			make_edge(edge,X_AXIS,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case Y_HIGH:
			make_edge(edge,X_AXIS,face->xl,face->yl,
				face->zl+1,face->denom);
			break;
		case MID_FACE:
			make_edge(edge,face->type,face->xl,face->yl,
				face->zl,face->denom);
			break;
		}
		break;

	case FACE_DD: case FACE_UU:
		switch(code)
		{
		case X_LOW:
			make_edge(edge,Y_AXIS,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case X_HIGH:
			make_edge(edge,Y_AXIS,face->xl+1,face->yl,
				face->zl,face->denom);
			break;
		case Y_LOW:
			make_edge(edge,X_AXIS,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case Y_HIGH:
			make_edge(edge,X_AXIS,face->xl,face->yl+1,
				face->zl,face->denom);
			break;
		case MID_FACE:
			make_edge(edge,face->type,face->xl,face->yl,
				face->zl,face->denom);
			break;
		}
		break;
	}
}

make_face_vert(face,code,vert)
face_info *face;
int code;
vert_info *vert;
{
	switch(face->type)
	{
	case FACE_LL: case FACE_RR:
		switch(code)
		{
		case X_LOW_Y_LOW:
			make_vert(vert,face->xl,face->yl,face->zl,face->denom);
			break;
		case X_HIGH_Y_LOW:
			make_vert(vert,face->xl,face->yl+1,
				face->zl,face->denom);
			break;
		case X_LOW_Y_HIGH:
			make_vert(vert,face->xl,face->yl,
				face->zl+1,face->denom);
			break;
		case X_HIGH_Y_HIGH:
			make_vert(vert,face->xl,face->yl+1,
				face->zl+1,face->denom);
			break;
		}
		break;

	case FACE_FF: case FACE_BB:
		switch(code)
		{
		case X_LOW_Y_LOW:
			make_vert(vert,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case X_HIGH_Y_LOW:
			make_vert(vert,face->xl+1,face->yl,
				face->zl,face->denom);
			break;
		case X_LOW_Y_HIGH:
			make_vert(vert,face->xl,face->yl,
				face->zl+1,face->denom);
			break;
		case X_HIGH_Y_HIGH:
			make_vert(vert,face->xl+1,face->yl,
				face->zl+1,face->denom);
			break;
		}
		break;

	case FACE_DD: case FACE_UU:
		switch(code)
		{
		case X_LOW_Y_LOW:
			make_vert(vert,face->xl,face->yl,
				face->zl,face->denom);
			break;
		case X_HIGH_Y_LOW:
			make_vert(vert,face->xl+1,face->yl,
				face->zl,face->denom);
			break;
		case X_LOW_Y_HIGH:
			make_vert(vert,face->xl,face->yl+1,
				face->zl,face->denom);
			break;
		case X_HIGH_Y_HIGH:
			make_vert(vert,face->xl+1,face->yl+1,
				face->zl,face->denom);
			break;
		}
		break;
	}
}

#ifdef NOT_DEF
subdevideedge(edge,edge1,edge2)
edge_info *edge,*edge1,*edge2;
{
	switch(edge->type)
	{
	case X_AXIS: 
		make_edge(edge1,edge->type,edge->xl*2,
			edge->yl*2,edge->zl*2,edge->denom*2);
		make_edge(edge2,edge->type,edge->xl*2 + 1,
			edge->yl*2,edge->zl*2,edge->denom*2);
		break;
	case Y_AXIS: 
		make_edge(edge1,edge->type,edge->xl*2,
			edge->yl*2,edge->zl*2,edge->denom*2);
		make_edge(edge2,edge->type,edge->xl*2,
			edge->yl*2 + 1,edge->zl*2,edge->denom*2);
		break;
	case Z_AXIS: 
		make_edge(edge1,edge->type,edge->xl*2,
			edge->yl*2,edge->zl*2,edge->denom*2);
		make_edge(edge2,edge->type,edge->xl*2,
			edge->yl*2,edge->zl*2 + 1,edge->denom*2);
		break;
	}
}
#endif

/*
 * Function:	split_edge
 * action:	ensures that edge comprises of two halves and that
 *		if a solution exists then it lies in one of the two
 *		halves.
 */

split_edge(edge)
edge_info *edge;
{
	if( edge == NULL ) return;
	if( edge->left == NULL )
	{
		edge->left = allocedge();
		make_edge(edge->left,edge->type,edge->xl*2,
			edge->yl*2,edge->zl*2,edge->denom*2);
		edge->left->status = edge->status;
		edge->left->low = edge->low;
	}

	if( edge->right == NULL )
	{
		edge->right = allocedge();
		switch( edge->type )
		{
		case X_AXIS: 
			make_edge(edge->right,edge->type,edge->xl*2 + 1,
				edge->yl*2,edge->zl*2,edge->denom*2);
			break;
		case Y_AXIS: 
			make_edge(edge->right,edge->type,edge->xl*2,
				edge->yl*2 + 1,edge->zl*2,edge->denom*2);
			break;
		case Z_AXIS: 
			make_edge(edge->right,edge->type,edge->xl*2,
				edge->yl*2,edge->zl*2 + 1,edge->denom*2);
			break;
		}
		edge->right->status = edge->status;
		edge->right->high = edge->high;
	}

	/* Now play with verts */
	if( edge->left->high == NULL )
	{
		if( edge->right->low == NULL )
		{
			edge->right->low = edge->left->high = allocvert();
			make_vert(edge->left->high,
				edge->right->xl,edge->right->yl,
				edge->right->zl,edge->right->denom);
		}
		else
		{
			fprintf(stderr,"Wierd right edge vert no left\n");
			printedge(edge);
			edge->left->high = edge->right->low;
		}
	}
	else
	{
		if( edge->right->low == NULL )
		{
			fprintf(stderr,"Wierd left edge vert no right\n");
			printedge(edge);
			edge->right->low = edge->left->high; 
		}
		else
		{
			if( edge->right->low != edge->left->high )
			{
				fprintf(stderr,"Wierd left and right differ\n");				printedge(edge);
			}
		}
	}

					

	if( edge->sol != NULL )
	{
		if( edge->sol->root > 0.0 && edge->sol->root < 0.5 )
		{
			if( edge->left->sol == NULL )
			{
				edge->left->sol = edge->sol;
				edge->sol = NULL;
				edge->left->sol->root *= 2.0;
				edge->left->sol->xl = edge->left->xl;
				edge->left->sol->yl = edge->left->yl;
				edge->left->sol->zl = edge->left->zl;
				edge->left->sol->denom = edge->left->denom;
			}
			else
			{
#ifdef PRI_SPLIT_EDGE
				fprintf(stderr,"split_edge: edge->left->sol != NULL\n");
				printedge(edge);
#endif
			}
		}
		else if( edge->sol->root > 0.5 && edge->sol->root < 1.0 )
		{
			if( edge->right->sol == NULL )
			{
				edge->right->sol = edge->sol;
				edge->sol = NULL;
				edge->right->sol->root *= 2.0;
				edge->right->sol->root -= 1.0;
				edge->right->sol->xl = edge->right->xl;
				edge->right->sol->yl = edge->right->yl;
				edge->right->sol->zl = edge->right->zl;
				edge->right->sol->denom = edge->right->denom;
			}
			else
			{
#ifdef PRI_SPLIT_EDGE
				fprintf(stderr,"split_edge: edge->right->sol != NULL\n");
				printedge(edge);
#endif
			}
		}
		else
		{
#ifdef PRI_SPLIT_EDGE
			fprintf(stderr,"split_edge: edge->sol->root = %f\n",
					edge->sol->root);
#endif
		}
	}
}

/*
 * Function:	make_face
 * action	fill the structure pointed to by face with info.
 */

make_face(face,type,xl,yl,zl,denom)
face_info *face;
soltype type;
int xl,yl,zl,denom;
{
	face->type = type;
	face->xl = xl;
	face->yl = yl;
	face->zl = zl;
	face->denom = denom;
	face->status = FALSE;
	face->x_low = face->x_high = face->y_low = face->y_high = NULL;
	face->lb = face->rb = face->lt = face->rt = NULL;
	face->links = NULL;
	face->nodes = NULL;
}

/*
 * Function:	make_box_face
 * action:	let face contain the info about the face 'type' of 'box'.
 */

make_box_face(box,type,face)
box_info *box;
soltype type;
face_info *face;
{
	switch(type)
	{
	case FACE_LL:
		make_face(face,FACE_LL,box->xl,box->yl,box->zl,box->denom);
		break;
	case FACE_RR:
		make_face(face,FACE_RR,box->xl+1,box->yl,box->zl,box->denom);
		break;
	case FACE_FF:
		make_face(face,FACE_FF,box->xl,box->yl,box->zl,box->denom);
		break;
	case FACE_BB:
		make_face(face,FACE_BB,box->xl,box->yl+1,box->zl,box->denom);
		break;
	case FACE_DD:
		make_face(face,FACE_DD,box->xl,box->yl,box->zl,box->denom);
		break;
	case FACE_UU:
		make_face(face,FACE_UU,box->xl,box->yl,box->zl+1,box->denom);
		break;
	default:
		fprintf(stderr,"make_box_face: bad type ");
		printsoltype(type);
		break;
	}
}

/*
 * Function:	make_sub_faces
 * action:	creates the four sub faces of a face
 */

make_sub_faces(face,face1,face2,face3,face4)
face_info *face,*face1,*face2,*face3,*face4;
{
	switch(face->type)
	{
	case FACE_LL: case FACE_RR:
		make_face(face1,face->type,
			face->xl*2,face->yl*2,face->zl*2,face->denom*2);
		make_face(face2,face->type,
			face->xl*2,face->yl*2+1,face->zl*2,face->denom*2);
		make_face(face3,face->type,
			face->xl*2,face->yl*2,face->zl*2+1,face->denom*2);
		make_face(face4,face->type,
			face->xl*2,face->yl*2+1,face->zl*2+1,face->denom*2);
		break;
	case FACE_FF: case FACE_BB:
		make_face(face1,face->type,
			face->xl*2,face->yl*2,face->zl*2,face->denom*2);
		make_face(face2,face->type,
			face->xl*2+1,face->yl*2,face->zl*2,face->denom*2);
		make_face(face3,face->type,
			face->xl*2,face->yl*2,face->zl*2+1,face->denom*2);
		make_face(face4,face->type,
			face->xl*2+1,face->yl*2,face->zl*2+1,face->denom*2);
		break;
	case FACE_DD: case FACE_UU:
		make_face(face1,face->type,
			face->xl*2,face->yl*2,face->zl*2,face->denom*2);
		make_face(face2,face->type,
			face->xl*2+1,face->yl*2,face->zl*2,face->denom*2);
		make_face(face3,face->type,
			face->xl*2,face->yl*2+1,face->zl*2,face->denom*2);
		make_face(face4,face->type,
			face->xl*2+1,face->yl*2+1,face->zl*2,face->denom*2);
		break;
	}
}

make_box(box,xl,yl,zl,denom)
box_info *box;
{
	box->xl = xl;
	box->yl = yl;
	box->zl = zl;
	box->denom = denom;
	box->status = FALSE;
	box->lfu = box->lfd = box->lbu = box->lbd = NULL;
	box->rfu = box->rfd = box->rbu = box->rbd = NULL;
	box->ll = box->rr = box->ff = box->bb = box->dd = box->uu = NULL;
	box->node_links = NULL;
	box->sings = NULL;
}

subdevidebox(box,box1,box2,box3,box4,box5,box6,box7,box8)
box_info *box,*box1,*box2,*box3,*box4,*box5,*box6,*box7,*box8;
{
	make_box(box1, 2*box->xl,  2*box->yl,  2*box->zl,  2*box->denom);
	make_box(box2, 2*box->xl+1,2*box->yl,  2*box->zl,  2*box->denom);
	make_box(box3, 2*box->xl,  2*box->yl+1,2*box->zl,  2*box->denom);
	make_box(box4, 2*box->xl+1,2*box->yl+1,2*box->zl,  2*box->denom);
	make_box(box5, 2*box->xl,  2*box->yl,  2*box->zl+1,2*box->denom);
	make_box(box6, 2*box->xl+1,2*box->yl,  2*box->zl+1,2*box->denom);
	make_box(box7, 2*box->xl,  2*box->yl+1,2*box->zl+1,2*box->denom);
	make_box(box8, 2*box->xl+1,2*box->yl+1,2*box->zl+1,2*box->denom);
}

/*
 * Function:	sub_devide_box
 * action:	create the apropriate information for all the sub boxes,
 *		does not play about with the faces.
 */

sub_devide_box(box)
box_info *box;
{
	box->lfd = allocbox();
	box->rfd = allocbox();
	box->lbd = allocbox();
	box->rbd = allocbox();
	box->lfu = allocbox();
	box->rfu = allocbox();
	box->lbu = allocbox();
	box->rbu = allocbox();

	subdevidebox(box,box->lfd,box->rfd,box->lbd,box->rbd,
			 box->lfu,box->rfu,box->lbu,box->rbu );
}

/*
 * Function:	calc_pos_on_face
 * action:	vec is the position of sol on the face
 */

calc_pos_on_face(face,sol,vec)
face_info *face;
sol_info *sol;
double vec[2];
{
	switch(face->type)
	{
	case FACE_LL: case FACE_RR:
		switch(sol->type)
		{
		case Y_AXIS:
			vec[0] = face->denom * (sol->yl + sol->root)/sol->denom
				- face->yl;
			vec[1] = face->denom * sol->zl / sol->denom - face->zl;
			break;
		case Z_AXIS:
			vec[0] = face->denom * sol->yl / sol->denom - face->yl;
			vec[1] = face->denom * (sol->zl + sol->root)/sol->denom
				- face->zl;
			break;
		default:
			fprintf(stderr,"calc_pos_on_face: bad types face ");
			printsoltype(face->type);
			fprintf(stderr," sol ");
			printsoltype(sol->type);
			fprintf(stderr,"\n");
			break;
		}
		break;
	case FACE_FF: case FACE_BB:
		switch(sol->type)
		{
		case X_AXIS:
			vec[0] = face->denom * (sol->xl + sol->root)/sol->denom
				- face->xl;
			vec[1] = face->denom * sol->zl / sol->denom - face->zl;
			break;
		case Z_AXIS:
			vec[0] = face->denom * sol->xl / sol->denom - face->xl;
			vec[1] = face->denom * (sol->zl + sol->root)/sol->denom
				- face->zl;
			break;
		default:
			fprintf(stderr,"calc_pos_on_face: bad types face ");
			printsoltype(face->type);
			fprintf(stderr," sol ");
			printsoltype(sol->type);
			fprintf(stderr,"\n");
			break;
		}
		break;
	case FACE_DD: case FACE_UU:
		switch(sol->type)
		{
		case X_AXIS:
			vec[0] = face->denom * (sol->xl + sol->root)/sol->denom
				- face->xl;
			vec[1] = face->denom * sol->yl / sol->denom - face->yl;
			break;
		case Y_AXIS:
			vec[0] = face->denom * sol->xl / sol->denom - face->xl;
			vec[1] = face->denom * (sol->yl + sol->root)/sol->denom
				- face->yl;
			break;
		default:
			fprintf(stderr,"calc_pos_on_face: bad types face ");
			printsoltype(face->type);
			fprintf(stderr," sol ");
			printsoltype(sol->type);
			fprintf(stderr,"\n");
			break;
		}
		break;
	}	/* end switch(face->type) */
}

/*
 * Function:	calc_pos_in_box
 * action:	vec is the position of sol on the box
 */

calc_pos_in_box(box,sol,vec)
box_info *box;
sol_info *sol;
double vec[3];
{
	switch(sol->type)
	{
	case FACE_LL: case FACE_RR:
		vec[0] = box->denom * (sol->xl)/sol->denom - box->xl;
		vec[1] = box->denom * (sol->yl+sol->root)/sol->denom - box->yl;
		vec[2] = box->denom * (sol->zl+sol->root2)/sol->denom - box->zl;
		break;
	case FACE_FF: case FACE_BB:
		vec[0] = box->denom * (sol->xl+sol->root)/sol->denom - box->xl;
		vec[1] = box->denom * (sol->yl)/sol->denom - box->yl;
		vec[2] = box->denom * (sol->zl+sol->root2)/sol->denom - box->zl;
		break;
	case FACE_DD: case FACE_UU:
		vec[0] = box->denom * (sol->xl+sol->root)/sol->denom - box->xl;
		vec[1] = box->denom * (sol->yl+sol->root2)/sol->denom - box->yl;
		vec[2] = box->denom * (sol->zl)/sol->denom - box->zl;
		break;
	default:
		fprintf(stderr,"calc_pos_in_box: bad types ");
		fprintf(stderr," sol ");
		printsoltype(sol->type);
		fprintf(stderr,"\n");
		break;
	}
}

/************************************************************************/
/*									*/
/*	Now play about with lists					*/
/*									*/
/************************************************************************/

/*
 * Function:	remove_link
 * action:	removes the link pointed to by link from the list
 *		of links belonging to face.
 *		return FALSE on error.
 */

int remove_link(face,link)
face_info *face;
link_info *link;
{
	link_info *temp;

	if( face->links == NULL )
	{
		fprintf(stderr,"Error: remove_link: empty list\n");
		return(FALSE);
	}
	if( face->links == link )
	{
		face->links = link->next;
		freelink(link);
		return(TRUE);
	}
	else
	{
		temp = face->links;
		while( temp != NULL )
		{
			if( temp->next == link )
			{
				temp->next = link->next;
				freelink(link);
				return(TRUE);
			}
			temp = temp->next;
		}
		fprintf(stderr,"Error: remove_link: couldn't find link on face\n");
		return(FALSE);
	}
}

/*
 * Function:	remove_node_link
 * action:	remove the link from those belonging to box.
 */

int remove_node_link(box,link)
box_info *box;
node_link_info *link;
{
	node_link_info *temp;

	if( box->node_links == NULL )
	{
		fprintf(stderr,"Error: remove_sing_link: empty list\n");
		return(FALSE);
	}
	if( box->node_links == link )
	{
		box->node_links = link->next;
		freenode_link(link);
		return(TRUE);
	}
	else
	{
		temp = box->node_links;
		while( temp != NULL )
		{
			if( temp->next == link )
			{
				temp->next = link->next;
				freenode_link(link);
				return(TRUE);
			}
			temp = temp->next;
		}
		fprintf(stderr,"Error: remove_sing_link: couldn't find link on box\n");
		return(FALSE);
	}
}

/*
 * Function:	include_link
 * action:	given a link between two solutions and a list of exsisting
 *		links on the face do the following:
 *		if neither sol in list add link to list;
 *		if one sol matches a sol in list extend the existing link;
 *		if link joins two exsisting links remove one and
 *		join the two together.
 *
 *		basically do the right thing to the list with the given link.
 */


include_link(face,sol1,sol2)
face_info *face;
sol_info *sol1,*sol2;
{
	link_info *link, *link1=NULL, *link2=NULL;
	int link1_keepA, link2_keepA;

	link = face->links;
	while( link != NULL )
	{
		if( sol1 == link->A && sol1->type < FACE_LL )
		{
			link1 = link;
			if( link->B == NULL )
			{
				link->B = sol2;
				link1_keepA = TRUE;
			}
			else
			{
				link->A = sol2;
				link1_keepA = FALSE;
			}
		}
		else if( sol1 == link->B && sol1->type < FACE_LL )
		{
			link1 = link;
			if( link->A == NULL )
			{
				link->A = sol2;
				link1_keepA = FALSE;
			}
			else
			{
				link->B = sol2;
				link1_keepA = TRUE;
			}
		}
		else if( sol2 == link->A && sol2->type < FACE_LL )
		{
			link2 = link;
			if( link->B == NULL )
			{
				link->B = sol1;
				link2_keepA = TRUE;
			}
			else
			{
				link->A = sol1;
				link2_keepA = FALSE;
			}
		}
		else if( sol2 == link->B && sol2->type < FACE_LL )
		{
			link2 = link;
			if( link->A == NULL )
			{
				link->A = sol1;
				link2_keepA = FALSE;
			}
			else
			{
				link->B = sol1;
				link2_keepA = TRUE;
			}
		}

		link = link->next;
	} /* end while */

	if( link1 == NULL && link2 == NULL )	/* Didn't find link add it */
	{
		link = alloclink();
		link->A = sol1;
		link->B = sol2;
		link->status = LINK;
		link->next = face->links;
		face->links = link;
	}
	else if( link1 != NULL ) /* join two links together */
	{
		if( link2 != NULL )
		{
			if( link1_keepA )
				if( link2_keepA )
					link1->B = link2->A;
				else
					link1->B = link2->B;
			else
				if( link2_keepA )
					link1->A = link2->A;
				else
					link1->A = link2->B;

			remove_link(face,link2);
		}

		/*** Do nothing if only one end of a simple link. ***/
	}
#ifdef PRINT_INCLUDE_LINK
	printface(face);
#endif
}

/*
 * Function:	add_node(face,sol)
 * action:	add the node to the list for the face.
 *		simple version where we don't try to join nodes together.
 */

add_node(face,sol)
face_info *face;
sol_info *sol;
{
	node_info *node;

	node = allocnode();
	node->next = face->nodes;
	node->sol = sol;
	node->status = NODE;
	face->nodes = node;
}

/*
 * Function:	colect_node
 * action:	collect all the MID_FACE solutions together to make
 *		a single node, and have all the solutions joined to it.
 * BUGS:	solution round edge adjicient to node?
 */

colect_nodes(box,face)
box_info *box;
face_info *face;
{
#ifdef COLLECT
	sol_info *sol;
	link_info *link;
	node_info *node;
	int num_nodes,num_sols,i,j;

	num_nodes = count_nodes_on_face(face);

	/*** First if a link is adjacient to a node change the end sol ***/
	/***	to point to the node.				       ***/

	for(i=1;i<=num_nodes;++i)
	{
		node = get_nth_node_on_face(face,i);

		link = face->links;
		while( link != NULL )
		{
			if( adjacient_to_node(node,link->A) )
			{
/*
				freesol(link->A);
*/
				link->A = node->sol;
			}
			if( adjacient_to_node(node,link->B) )
			{
/*
				freesol(link->B);
*/
				link->B = node->sol;
			}
			link = link->next;
	    	}
	}

	/*** Now if any edge sol is adjacient to a node create a link ***/
	/*** with the edge sol at one end and the node at the other.  ***/

	num_sols = count_sols_on_face(face);

	for(i=1;i<=num_nodes;++i)
	{
		node = get_nth_node_on_face(face,i);

		for(j=1;j<=num_sols;++j)
		{
			sol = get_nth_sol_on_face(face,j);
			if(adjacient_to_node(node,sol) )
			{
				include_link(face,node->sol,sol);
			}
		}
	}
#endif
}

/*
 * Function:	add_node_link
 * action:	given a link between two nodes and a list of exsisting
 *		links in the box do the following:
 *		if neither sol in list add link to list;
 *		if one sol matches a sol in list extend the existing link;
 *		if link joins two exsisting links remove one and
 *		join the two together.
 *
 *		basically do the right thing to the list with the given link.
 */

add_node_link(box,node1,node2)
box_info *box;
node_info *node1,*node2;
{
	node_link_info *link, *link1=NULL, *link2=NULL;
	int link1_keepA, link2_keepA;

	link = box->node_links;
	while( link != NULL )
	{
		if( node1 == link->A && node1->sol->type != BOX )
		{
			link1 = link;
			if( link->B == NULL )
			{
				link->B = node2;
				link1_keepA = TRUE;
			}
			else
			{
				link->A = node2;
				link1_keepA = FALSE;
			}
		}
		else if( node1 == link->B && node1->sol->type != BOX )
		{
			link1 = link;
			if( link->A == NULL )
			{
				link->A = node2;
				link1_keepA = FALSE;
			}
			else
			{
				link->B = node2;
				link1_keepA = TRUE;
			}
		}
		else if( node2 == link->A && node2->sol->type != BOX )
		{
			link2 = link;
			if( link->B == NULL )
			{
				link->B = node1;
				link2_keepA = TRUE;
			}
			else
			{
				link->A = node1;
				link2_keepA = FALSE;
			}
		}
		else if( node2 == link->B && node2->sol->type != BOX )
		{
			link2 = link;
			if( link->A == NULL )
			{
				link->A = node1;
				link2_keepA = FALSE;
			}
			else
			{
				link->B = node1;
				link2_keepA = TRUE;
			}
		}

		link = link->next;
	} /* end while */

	if( link1 == NULL && link2 == NULL )	/* Didn't find link add it */
	{
		link = allocnode_link();
		link->A = node1;
		link->B = node2;
		link->status = NODE;
		link->next = box->node_links;
		box->node_links = link;
	}
	else if( link1 != NULL ) /* join two links together */
	{
		if( link2 != NULL )
		{
			if( link1_keepA )
				if( link2_keepA )
					link1->B = link2->A;
				else
					link1->B = link2->B;
			else
				if( link2_keepA )
					link1->A = link2->A;
				else
					link1->A = link2->B;

			remove_node_link(box,link2);
		}

		/*** Do nothing if only one end of a simple link. ***/
	}
}

/*
 * Function:	add_sing(box,sol)
 * action:	add the sing to the list for the box.
 *		simple version where we don't try to join sings together.
 */

add_sing(box,sol)
box_info *box;
sol_info *sol;
{
	sing_info *sing;

	sing = allocsing();
	sing->next = box->sings;
	sing->sing = sol;
	sing->status = NODE;
	box->sings = sing;
}

/*
 * Function:	collect_sings
 * action:	simplyfy the list of node_links and sings
 *		for each sing check whats adjacient to it
 *		if there are only two adjaciencies eliminate the node
		if a node_link is adjacient to a sing then change the
		face to be a node.
 */

collect_sings(box)
box_info *box;
{
#ifdef COLLECT
	node_link_info *node_link;
	sing_info *sing;

	sing = box->sings;

	while(sing != NULL)
	{
		node_link = box->node_links;
		while( node_link != NULL )
		{
			if( adjacient_to_sing(sing,node_link->A->sol) )
			{
/*
*/
				freesol(node_link->A->sol);
				node_link->A->sol = sing->sing;
			}
			if( adjacient_to_sing(sing,node_link->B->sol) )
			{
/*
*/
				freesol(node_link->B->sol);
				node_link->B->sol = sing->sing;
			}
			node_link = node_link->next;
	    	}
		sing = sing->next;
	}

	/*** Now if any node is adjacient to a sing create a node_link ***/
	/*** with the node at one end and the sing at the other.  ***/

	num_sols = count_sols_on_face(face);

	for(i=1;i<=num_nodes;++i)
	{
		node = get_nth_node_on_face(face,i);

		for(j=1;j<=num_sols;++j)
		{
			sol = get_nth_sol_on_face(face,j);
			if(adjacient_to_node(node,sol) )
			{
				include_link(face,node->sol,sol);
			}
		}
	}
#endif
}

/************************************************************************/
/*									*/
/*	Routines to work out relationships between cells		*/
/*									*/
/************************************************************************/

/*
 * Function:	adjacient_to_node
 * action:	returns true if sol is adjacient to the node given
 *		by the link. Must be strictly adjacient, i.e. touch
 *		on whole edge.
 */

int adjacient_to_node(node,sol)
node_info *node;
sol_info *sol;
{
	if(sol == NULL ) return(FALSE);
	if(node == NULL ) return(FALSE);

	if( node->sol->type == FACE_LL || node->sol->type == FACE_RR )
	{
	    if( sol->xl * node->sol->denom != sol->denom * node->sol->xl )
		return(FALSE);

	    if( sol->type == Y_AXIS )
	    {
		return((sol->yl+1)*node->sol->denom > node->sol->yl * sol->denom &&
		       sol->yl * node->sol->denom< (node->sol->yl+1)*sol->denom &&
		       sol->zl * node->sol->denom >=   node->sol->zl * sol->denom &&
		       sol->zl * node->sol->denom <= (node->sol->zl+1)*sol->denom );
	    }
	    else if( sol->type == Z_AXIS )
	    {
		return(sol->yl * node->sol->denom >= node->sol->yl * sol->denom &&
		       sol->yl * node->sol->denom <= (node->sol->yl+1)*sol->denom &&
		       (sol->zl+1)*node->sol->denom > node->sol->zl * sol->denom &&
		       sol->zl * node->sol->denom< (node->sol->zl+1)*sol->denom );
	    }
	}
	if( node->sol->type == FACE_FF || node->sol->type == FACE_BB )
	{
	    if( sol->yl * node->sol->denom != sol->denom * node->sol->yl )
		return(FALSE);

	    if( sol->type == X_AXIS )
	    {
		return((sol->xl+1)*node->sol->denom > node->sol->xl * sol->denom &&
		       sol->xl * node->sol->denom< (node->sol->xl+1)*sol->denom &&
		       sol->zl * node->sol->denom >=   node->sol->zl * sol->denom &&
		       sol->zl * node->sol->denom <= (node->sol->zl+1)*sol->denom );
	    }
	    else if( sol->type == Z_AXIS )
	    {
		return(sol->xl * node->sol->denom >= node->sol->xl * sol->denom &&
		       sol->xl * node->sol->denom <= (node->sol->xl+1)*sol->denom &&
		       (sol->zl+1)*node->sol->denom > node->sol->zl * sol->denom &&
		       sol->zl * node->sol->denom< (node->sol->zl+1)*sol->denom );
	    }
	}
	if( node->sol->type == FACE_DD || node->sol->type == FACE_UU )
	{
	    if( sol->zl * node->sol->denom != sol->denom * node->sol->zl )
		return(FALSE);

	    if( sol->type == X_AXIS )
	    {
		return((sol->xl+1)*node->sol->denom > node->sol->xl * sol->denom &&
		       sol->xl * node->sol->denom< (node->sol->xl+1)*sol->denom &&
		       sol->yl * node->sol->denom >=   node->sol->yl * sol->denom &&
		       sol->yl * node->sol->denom <= (node->sol->yl+1)*sol->denom );
	    }
	    else if( sol->type == Y_AXIS )
	    {
		return(sol->xl * node->sol->denom >= node->sol->xl * sol->denom &&
		       sol->xl * node->sol->denom<=(node->sol->xl+1)*sol->denom &&
		       (sol->yl+1)*node->sol->denom > node->sol->yl * sol->denom &&
		       sol->yl * node->sol->denom< (node->sol->yl+1)*sol->denom );
	    }
	}
/*
	fprintf(stderr,"adjacient_sol: bad types node ");
	printsoltype(node->sol->type);
	fprintf(stderr," sol ");
	printsoltype(sol->type);
	fprintf(stderr,"\n");
*/
	return(FALSE);
}

/*
 * Function:	adjacient_to_sing
 * action:	returns true if sol is adjacient to the sing given
 *		by the link. Must be strictly adjacient, i.e. touch
 *		on whole edge.
 */

int adjacient_to_sing(sing,sol)
sing_info *sing;
sol_info *sol;
{
	if(sol == NULL ) return(FALSE);
	if(sing == NULL ) return(FALSE);

	if(sol->type == FACE_LL || sol->type == FACE_RR )
	{
	return(sol->xl * sing->sing->denom >= sing->sing->xl * sol->denom &&
	       sol->xl * sing->sing->denom<=(sing->sing->xl+1)*sol->denom &&
	       (sol->yl+1)*sing->sing->denom > sing->sing->yl * sol->denom &&
	       sol->yl * sing->sing->denom< (sing->sing->yl+1)*sol->denom &&
	       (sol->zl+1)*sing->sing->denom > sing->sing->zl * sol->denom &&
	       sol->zl * sing->sing->denom< (sing->sing->zl+1)*sol->denom );
	}

	else if(sol->type == FACE_FF || sol->type == FACE_BB )
	{
	return((sol->xl+1)*sing->sing->denom > sing->sing->xl * sol->denom &&
	       sol->xl * sing->sing->denom< (sing->sing->xl+1)*sol->denom &&
	       sol->yl * sing->sing->denom >= sing->sing->yl * sol->denom &&
	       sol->yl * sing->sing->denom<=(sing->sing->yl+1)*sol->denom &&
	       (sol->zl+1)*sing->sing->denom > sing->sing->zl * sol->denom &&
	       sol->zl * sing->sing->denom< (sing->sing->zl+1)*sol->denom );
	}

	else if(sol->type == FACE_DD || sol->type == FACE_UU )
	{
	return((sol->xl+1)*sing->sing->denom > sing->sing->xl * sol->denom &&
	       sol->xl * sing->sing->denom< (sing->sing->xl+1)*sol->denom &&
	       (sol->yl+1)*sing->sing->denom > sing->sing->yl * sol->denom &&
	       sol->yl * sing->sing->denom< (sing->sing->yl+1)*sol->denom &&
	       sol->zl * sing->sing->denom >= sing->sing->zl * sol->denom &&
	       sol->zl * sing->sing->denom<=(sing->sing->zl+1)*sol->denom );
	}
	else return(FALSE);
}

/*
 *      Function: make_sol,
 *      action:   make the solution given by xl,yl,zl,denom,root.
 */

sol_info *make_sol(type,xl,yl,zl,denom,root)
soltype type;
int xl,yl,zl,denom;
double root;
{
        sol_info *temp;

        temp = allocsol();
        temp->type = type;
        temp->xl = xl;
        temp->yl = yl;
        temp->zl = zl;
        temp->denom = denom;
        temp->root = root;
        temp->root2 = 0.0;
        temp->root3 = 0.0;
        temp->status = FALSE;
	temp->plotindex = -1;

	if(root != root )
	{
		fprintf(stderr,"Bad sol\n");
		print_sol(temp);
	}
	return(temp);
}

/*
 * Function:	make_sol2
 * Action:	creates a solution lying inside a face.
 */

sol_info *make_sol2(type,xl,yl,zl,denom,root,root2)
soltype type;
int xl,yl,zl,denom;
double root,root2;
{
        sol_info *temp;

        temp = allocsol();
        temp->type = type;
        temp->xl = xl;
        temp->yl = yl;
        temp->zl = zl;
        temp->denom = denom;
        temp->root = root;
        temp->root2 = root2;
        temp->root3 = 0.0;
        temp->status = FALSE;
	temp->plotindex = -1;
	if(root != root || root2 != root2)
	{
		fprintf(stderr,"Bad sol\n");
		print_sol(temp);
	}
	return(temp);
}

/*
 * Function:	make_sol3
 * Action:	creates a solution lying inside a face.
 */

sol_info *make_sol3(type,xl,yl,zl,denom,root,root2,root3)
soltype type;
int xl,yl,zl,denom;
double root,root2,root3;
{
        sol_info *temp;

        temp = allocsol();
        temp->type = type;
        temp->xl = xl;
        temp->yl = yl;
        temp->zl = zl;
        temp->denom = denom;
        temp->root = root;
        temp->root2 = root2;
        temp->root3 = root3;
        temp->status = FALSE;
	temp->plotindex = -1;
	if(root != root || root2 != root2 || root3 != root3)
	{
		fprintf(stderr,"Bad sol\n");
		print_sol(temp);
	}
	return(temp);
}

/*
 * Function:	calc_pos
 * Action;	calculates position of point relative to the big box
 */

calc_pos(sol,vec)
sol_info *sol;
double vec[3];
{
        switch(sol->type)
        {
        case X_AXIS:
                if( sol->root == BAD_EDGE )
                        vec[0] = (sol->xl + 0.5)/sol->denom;
                else
                        vec[0] = (sol->xl+sol->root)/sol->denom;
                vec[1] = ((double) sol->yl)/sol->denom;
                vec[2] = ((double) sol->zl)/sol->denom;
                break;
        case Y_AXIS:
                vec[0] = ((double) sol->xl)/sol->denom;
                if( sol->root == BAD_EDGE )
                        vec[1] = (sol->yl + 0.5)/sol->denom;
                else
                        vec[1] = (sol->yl+sol->root)/sol->denom;
                vec[2] = ((double) sol->zl)/sol->denom;
                break;
        case Z_AXIS:
                vec[0] = ((double) sol->xl)/sol->denom;
                vec[1] = ((double) sol->yl)/sol->denom;
                if( sol->root == BAD_EDGE )
                        vec[2] = (sol->zl + 0.5)/sol->denom;
                else
                        vec[2] = (sol->zl+sol->root)/sol->denom;
                break;
        case FACE_LL:
        case FACE_RR:
                vec[0] = ((double) sol->xl)/sol->denom;
                vec[1] = (sol->yl + sol->root)/sol->denom;
                vec[2] = (sol->zl + sol->root2)/sol->denom;
                break;
        case FACE_FF:
        case FACE_BB:
                vec[0] = (sol->xl + sol->root)/sol->denom;
                vec[1] = ((double) sol->yl)/sol->denom;
                vec[2] = (sol->zl + sol->root2)/sol->denom;
                break;
        case FACE_DD:
        case FACE_UU:
                vec[0] = (sol->xl + sol->root)/sol->denom;
                vec[1] = (sol->yl + sol->root2)/sol->denom;
                vec[2] = ((double) sol->zl)/sol->denom;
                break;
        case BOX:
                vec[0] = (sol->xl + sol->root)/sol->denom;
                vec[1] = (sol->yl + sol->root2)/sol->denom;
                vec[2] = (sol->zl + sol->root3)/sol->denom;
                break;
        default:
                vec[0] = ((double) sol->xl)/sol->denom;
                vec[1] = ((double) sol->yl)/sol->denom;
                vec[2] = ((double) sol->zl)/sol->denom;
                break;
        }
}

/*****	Facets Routines **************************************/

/*
 * Function:	combine_facets
 * Action:	combines all the facets in the sub boxes
 *		to form the facets of the main box.
 *		Removes the facets from the sub boxes.
 */

#ifdef FACETS
combine_facets(box)
box_info *box;
{
	join_facets(box->lfd,box->rfd);
	join_facets(box->lfu,box->rfu);
	join_facets(box->lbd,box->rbd);
	join_facets(box->lbu,box->rbu);

	join_facets(box->lfd,box->lbd);
	join_facets(box->lfu,box->lbu);

	join_facets(box->lfd,box->lfu);

	box->facets = box->lfd->facets;
	box->lfd->facets = NULL;
}

/*
 * Function:	join_facets
 * Action:	join all the facets of box1 and box2
 *		removes the facets from box2 and leaves the results in box1.
 */

join_facets(box1,box2)
box_info *box1,*box2;
{
	facet_info *facet1,*facet2,*next_facet,*new_facets;

	facet2 = box2->facets;
	while(facet2 != NULL)
	{
		next_facet = facet2->next;
		facet1 = box1->facets;
		while(facet1 != NULL)
		{
			if(match_facets(facet1,facet2))
			{
				new_facets =
					link_facets(new_facets,facet1,facet2);
				break;
			}
			facet1 = facet1->next;
		}
		if(facet1 == NULL)
		{
			/* Didn't match mess with lists */

			facet2->next = new_facets;
			new_facets = facet2;
		}
		facet2 = next_facet;
	}
}

/*
 * Function:	match_facets
 * Action:	if the two facets have an edge in common return TRUE
 */

int match_facets(facet1,facet2)
facet_info *facet1,*facet2;
{
	facet_sol_info *sol1,*sol2;
	int	count;

	sol1 = facet1->sols;
	while(sol1!=NULL)
	{
		sol2 = facet2->sols;
		while(sol2 != NULL)
		{
			if( sol1->sol == sol2->sol )
			{
				if( sol1->sol->type >= X_AXIS
			 	 && sol1->sol->type <= Z_AXIS )
					return(TRUE);
				if( sol1->sol->type >= FACE_LL
			 	 && sol1->sol->type <= FACE_RR
				 && ++count == 2 )
					return(TRUE);
			}
			sol2 = sol2->next;
		}
		sol1 = sol1->next;
	}
	return(FALSE);
}
#endif

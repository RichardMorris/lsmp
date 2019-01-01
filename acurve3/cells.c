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
*/
#define MEMSET
#ifdef MEMSET
#define bzero(a,b) memset((a),0,(b))
#endif

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
	fprintf(stderr," (%hd,%hd,%hd)/%hd roots %lf %lf %lf deriv %d %d %d",
		temp->xl,temp->yl,temp->zl,temp->denom,
		temp->root,temp->root2,temp->root3,
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
/************************************************************************/

struct edgelist { struct edgelist *next; };
struct edgelist *FreeEdges = NULL;

edge_info *alloc_edge()
{
	edge_info *edge;

	if(FreeEdges != NULL)
	{
		edge = (edge_info *) FreeEdges;
		FreeEdges = FreeEdges->next;
		bzero((void *)edge,sizeof(edge_info));
	}
	else
		edge = (edge_info *) malloc(sizeof(edge_info));
	return(edge);
}

/*
 * Function:	free_edge
 * action:	frees an edge and all the sols on it.
 */

free_edge(edge)
edge_info *edge;
{
	struct edgelist *list;

	if(edge == NULL ) return;
	if(edge->left != NULL ) free_edge(edge->left);
	if(edge->right != NULL ) free_edge(edge->right);
	if(edge->sol != NULL ) free(edge->sol);
	list = (struct edgelist *) edge;
	list->next = FreeEdges;
	FreeEdges = list;
}

link_info *FreeLinks = NULL;

link_info *alloc_link()
{
	link_info *link;

	if(FreeLinks != NULL)
	{
		link = FreeLinks;
		FreeLinks = FreeLinks->next;
		bzero((void *)link,sizeof(link_info));
	}
	else
		link = (link_info *) malloc(sizeof(link_info));
	return(link);
}

free_links(links)
link_info *links;
{
	link_info *temp;

	if(links == NULL)return;
	temp = links;
	
	while(temp->next!=NULL)
	{
		temp = temp->next;
	}
	temp->next = FreeLinks;
	FreeLinks = links;
}

node_info *FreeNodes = NULL;

node_info *alloc_node()
{
	node_info *node;

	if(FreeNodes != NULL)
	{
		node = FreeNodes;
		FreeNodes = node->next;
		bzero((void *) node,sizeof(node_info));
	}
	else
		node = (node_info *) malloc(sizeof(node_info));
	return(node);
}

free_nodes(nodes)
node_info *nodes;
{
	node_info *temp;

	if(nodes == NULL) return;
	temp = nodes;
	while(temp!=NULL)
	{
		free(temp->sol);
		if(temp->next == NULL) break;
		temp = temp->next;
	}
	temp->next = FreeNodes;
	FreeNodes = temp;
}

face_info *FreeFaces = NULL;

face_info *alloc_face()
{
	face_info *face;

	if(FreeFaces != NULL)
	{
		face = FreeFaces;
		FreeFaces = FreeFaces->lb;
		bzero((void *) face,sizeof(face_info));
	}
	else
		face = (face_info *) malloc(sizeof(face_info));
	return(face);
}

free_face(face)
face_info *face;
{
	if(face==NULL) return;
	if( face->links != NULL ) free_links(face->links);
	if( face->nodes != NULL ) free_nodes(face->nodes);

	if(face->lb != NULL )
	{
		free_edge(face->lb->x_high);
		free_edge(face->lb->y_high);
		free_edge(face->rt->x_low);
		free_edge(face->rt->y_low);
		free_face(face->lb);
		free_face(face->rb);
		free_face(face->lt);
		free_face(face->rt);
	}
	face->lb = FreeFaces;
	FreeFaces = face;
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
	free_nodes(face->nodes); face->nodes = NULL;
	free_links(face->links); face->links = NULL;
	if(face->lb != NULL )
	{
		free_edge(face->lb->x_high);
		free_edge(face->lb->y_high);
		free_edge(face->rt->x_low);
		free_edge(face->rt->y_low);
		free_face(face->lb);
		free_face(face->rb);
		free_face(face->lt);
		free_face(face->rt);
	}
	face->lb = face->rb = face->lt = face->rt = NULL;
}

free_node_links(node_links)
node_link_info *node_links;
{
	node_link_info *temp;

	temp = node_links;
	
	while(temp!=NULL)
	{
		temp = node_links->next;
		free(node_links);
		node_links = temp;
	}
}

free_sings(sings)
sing_info *sings;
{
	sing_info *temp;

	temp = sings;
	
	while(temp!=NULL)
	{
		temp = sings->next;
		free(sings->sing);
		free(sings);
		sings = temp;
	}
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
	free_sings(box->sings);
	free_node_links(box->node_links);
	free_bits_of_face(box->ll);
	free_bits_of_face(box->ff);
	free_bits_of_face(box->dd);

	if(box->ll != NULL ) free_edge(box->ll->x_low);
	else if( box->ff != NULL ) free_edge(box->ff->x_low);

	if(box->ll != NULL ) free_edge(box->ll->y_low);
	else if( box->dd != NULL ) free_edge(box->dd->x_low);

	if(box->ff != NULL ) free_edge(box->ff->y_low);
	else if( box->dd != NULL ) free_edge(box->dd->y_low);
}

/************************************************************************/
/*									*/
/*	Now procedures to create other cells				*/
/*									*/
/************************************************************************/

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
}

/*
 * Function:	make_edge_on_face
 * action:	fill loctaion pointed to by sol with information
 *		about the edge on the face refered to by code.
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
		edge->left = alloc_edge();
		make_edge(edge->left,edge->type,edge->xl*2,
			edge->yl*2,edge->zl*2,edge->denom*2);
		edge->left->status = edge->status;
	}

	if( edge->right == NULL )
	{
		edge->right = alloc_edge();
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
				fprintf(stderr,"split_edge: edge->left->sol != NULL\n");
				printedge(edge);
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
				fprintf(stderr,"split_edge: edge->right->sol != NULL\n");
				printedge(edge);
			}
		}
		else
		{
			fprintf(stderr,"split_edge: edge->sol->root = %f\n",
					edge->sol->root);
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
	box->lfd = grballoc(box_info);
	box->rfd = grballoc(box_info);
	box->lbd = grballoc(box_info);
	box->rbd = grballoc(box_info);
	box->lfu = grballoc(box_info);
	box->rfu = grballoc(box_info);
	box->lbu = grballoc(box_info);
	box->rbu = grballoc(box_info);

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
		free(link);
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
				free(link);
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
		free(link);
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
				free(link);
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
		link = alloc_link();
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

	node = alloc_node();
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
				free(link->A);
*/
				link->A = node->sol;
			}
			if( adjacient_to_node(node,link->B) )
			{
/*
				free(link->B);
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
		link = grballoc(node_link_info);
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

	sing = grballoc(sing_info);
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
				free(node_link->A->sol);
				node_link->A->sol = sing->sing;
			}
			if( adjacient_to_sing(sing,node_link->B->sol) )
			{
/*
*/
				free(node_link->B->sol);
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
        double vec[3],val;

        temp = grballoc(sol_info);
        temp->type = type;
        temp->xl = xl;
        temp->yl = yl;
        temp->zl = zl;
        temp->denom = denom;
        temp->root = root;
        temp->status = FALSE;
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
        double vec[3],val;

        temp = grballoc(sol_info);
        temp->type = type;
        temp->xl = xl;
        temp->yl = yl;
        temp->zl = zl;
        temp->denom = denom;
        temp->root = root;
        temp->root2 = root2;
        temp->status = FALSE;
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
        double vec[3],val;

        temp = grballoc(sol_info);
        temp->type = type;
        temp->xl = xl;
        temp->yl = yl;
        temp->zl = zl;
        temp->denom = denom;
        temp->root = root;
        temp->root2 = root2;
        temp->root3 = root3;
        temp->status = FALSE;
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

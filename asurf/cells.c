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
#include <stdlib.h>
#include <math.h>
#include "cells.h"
/*
#define JOIN_NODES_IN_COLLECT
#define MAKE_NODES
#define PRINT_FOLLOW_CHAIN
#define PRINT_INCLUDE_LINK
#define PRINT_INCLUDE_NODE_LINK
#define TEST_ALLOC
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
 * Function:	print_soltype
 * action:	print on the standard output the type in hrf(human readable)
 */

void print_soltype(soltype type)
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

void print_sol_brief(sol_info *temp)
{
	if(temp == NULL ) 
	{
		fprintf(stderr,"\tNULL solution\n");
		return;
	}
	fprintf(stderr,"\tsol ");
	print_soltype(temp->type);
	fprintf(stderr," (%hd,%hd,%hd)/%hd ",
		temp->xl,temp->yl,temp->zl,temp->denom);
	switch(temp->type)
	{
	case X_AXIS: case Y_AXIS: case Z_AXIS:
		fprintf(stderr,"root %6.3f ",temp->root);
		break;
	case FACE_LL: case FACE_RR: case FACE_FF: case FACE_BB:
	case FACE_DD: case FACE_UU:
		fprintf(stderr,"roots %6.3f %6.3f ",temp->root,temp->root2);
		break;
	case BOX:
		fprintf(stderr,"roots %6.3f %6.3f %6.3f ",
			temp->root,temp->root2,temp->root3);
		break;
	default:
		fprintf(stderr,"Bad type: %d\n",temp->type);
	}
	fprintf(stderr,"deriv %2d %2d %2d\n",
		temp->dx,temp->dy,temp->dz );
/*
	fprintf(stderr," %p\n",temp);
*/
}

extern region_info region;
void print_sol(sol_info *sol)
{
	print_sol_brief(sol);
	switch(sol->type)
	{
	case X_AXIS:
		fprintf(stderr,"%6.3f %6.3f %6.3f\n",
		region.xmin + ( (sol->xl + sol->root)/sol->denom ) * (region.xmax-region.xmin),
		region.ymin + ( ((double) sol->yl)/sol->denom ) * (region.ymax-region.ymin),
		region.zmin + ( ((double) sol->zl)/sol->denom ) * (region.zmax-region.zmin));
		break;
	case Y_AXIS:
		fprintf(stderr,"%6.3f %6.3f %6.3f\n",
		region.xmin + ( ((double) sol->xl)/sol->denom ) * (region.xmax-region.xmin),
		region.ymin + ( (sol->yl + sol->root)/sol->denom ) * (region.ymax-region.ymin),
		region.zmin + ( ((double) sol->zl)/sol->denom ) * (region.zmax-region.zmin));
		break;
	case Z_AXIS:
		fprintf(stderr,"%6.3f %6.3f %6.3f\n",
		region.xmin + ( ((double) sol->xl)/sol->denom ) * (region.xmax-region.xmin),
		region.ymin + ( ((double) sol->yl)/sol->denom ) * (region.ymax-region.ymin),
		region.zmin + ( (sol->zl + sol->root)/sol->denom ) * (region.zmax-region.zmin));
		break;

	case FACE_RR: case FACE_LL:
		fprintf(stderr,"%6.3f %6.3f %6.3f\n",
		region.xmin + ( ((double) sol->xl)/sol->denom ) * (region.xmax-region.xmin),
		region.ymin + ( (sol->yl + sol->root)/sol->denom ) * (region.ymax-region.ymin),
		region.zmin + ( (sol->zl + sol->root2)/sol->denom ) * (region.zmax-region.zmin));
		break;
	case FACE_FF: case FACE_BB:
		fprintf(stderr,"%6.3f %6.3f %6.3f\n",
		region.xmin + ( (sol->xl + sol->root)/sol->denom ) * (region.xmax-region.xmin),
		region.ymin + ( ((double) sol->yl)/sol->denom ) * (region.ymax-region.ymin),
		region.zmin + ( (sol->zl + sol->root2)/sol->denom ) * (region.zmax-region.zmin));
		break;
	case FACE_UU: case FACE_DD:
		fprintf(stderr,"%6.3f %6.3f %6.3f\n",
		region.xmin + ( (sol->xl + sol->root)/sol->denom ) * (region.xmax-region.xmin),
		region.ymin + ( (sol->yl + sol->root2)/sol->denom ) * (region.ymax-region.ymin),
		region.zmin + ( ((double) sol->zl)/sol->denom ) * (region.zmax-region.zmin));
		break;
	case BOX:
		fprintf(stderr,"%6.3f %6.3f %6.3f\n",
		region.xmin + ( (sol->xl + sol->root)/sol->denom ) * (region.xmax-region.xmin),
		region.ymin + ( (sol->yl + sol->root2)/sol->denom ) * (region.ymax-region.ymin),
		region.zmin + ( (sol->zl + sol->root3)/sol->denom ) * (region.zmax-region.zmin));
		break;
	default:
		break;
	}
}

/*
 * Function:	printsols_on_edge(edge)
 * action:	prints the solutions lying on the edge.
 */

void print_sols_on_edge(edge_info *edge)
{
	if( edge == NULL ) return;
	if(edge->sol != NULL ) print_sol(edge->sol);
	if(edge->left != NULL )
	{
		print_sols_on_edge(edge->left);
	}
	if(edge->right != NULL )
	{
		print_sols_on_edge(edge->right);
	}
}

/*
 * Function:	print_edge(edge)
 * action:	prints the edge and all which lies apon it.
 */

void print_edge(edge_info *edge)
{
	if( edge == NULL )
	{
		fprintf(stderr,"NULL edge\n");
		return;
	}
	fprintf(stderr,"EDGE: type ");
	print_soltype(edge->type);
	fprintf(stderr," (%hd,%hd,%hd)/%hd,status %hd\n",
		edge->xl,edge->yl,edge->zl,edge->denom,edge->status);
	print_sols_on_edge(edge);
}

/*
 * Function:	printlinks
 * action:	print out a list of links.
 */

void print_link(link_info *link)
{
	if(link == NULL )
	{
		fprintf(stderr,"NULL links\n");
		return;
	}

	switch(link->status)
	{
		case NODE:	fprintf(stderr," NODE plotstatus %d\n",link->plotted); break;
		case LINK:	fprintf(stderr," LINK %d\n",link->plotted); break;
		default:	fprintf(stderr," unknown %hd %d\n",link->status,link->plotted);
	}
	print_sol(link->A);	
	print_sol(link->B);
	fprintf(stderr,"\n");
}

void print_links(link_info *link)
{
	int count = 0;

	if(link == NULL )
		fprintf(stderr,"no links\n");

	while(link != NULL )
	{
		fprintf(stderr,"Link no %d ",++count);
		print_link(link);
		link = link->next;
        }
	fprintf(stderr,"\n");
}

void print_node(node_info *node)
{
	if( node == NULL )
	{
		fprintf(stderr,"node == NULL\n");
	}
	else
	{
		fprintf(stderr,"Node: status %hd ",node->status);
		print_sol(node->sol);
/*
		fprintf(stderr,"pts %p %p\n",node,node->sol);
*/
	}
}

/*
 * Function:	print_nodes
 * action:	print out a list of nodes.
 */

void print_nodes(node_info *node)
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

void print_nodes_on_face(face_info *face)
{
	if( face == NULL ) return;
	if(face->nodes != NULL ) print_nodes(face->nodes);
	if(face->lb != NULL ) print_nodes_on_face(face->lb);
	if(face->rb != NULL ) print_nodes_on_face(face->rb);
	if(face->lt != NULL ) print_nodes_on_face(face->lt);
	if(face->rt != NULL ) print_nodes_on_face(face->rt);
}

/*
 * Function:	print_face
 * action:	print out the face and its solutions.
 */

void print_face(face_info *face)
{
	if( face == NULL )
	{
		fprintf(stderr,"NULL face\n");
		return;
	}
	fprintf(stderr,"FACE: type ");
	print_soltype(face->type);
	fprintf(stderr," (%hd,%hd,%hd)/%hd,status %hd\n",
		face->xl,face->yl,face->zl,face->denom,face->status);
	print_edge(face->x_low);
	print_edge(face->x_high);
	print_edge(face->y_low);
	print_edge(face->y_high);
	print_nodes_on_face(face);
	print_links(face->links);
}

void print_face_brief(face_info *face)
{
	if( face == NULL )
	{
		fprintf(stderr,"NULL face\n");
		return;
	}
	fprintf(stderr,"FACE: type ");
	print_soltype(face->type);
	fprintf(stderr," (%hd,%hd,%hd)/%hd,status %hd\n",
		face->xl,face->yl,face->zl,face->denom,face->status);
	print_nodes_on_face(face);
	print_links(face->links);
}

void print_node_link(node_link_info *node_link)
{
	if( node_link == NULL )
		fprintf(stderr,"No node link\n");

		fprintf(stderr,"NODE LINK: status %hd\n",node_link->status);
		fprintf(stderr,"\t");
		print_node(node_link->A);	
		fprintf(stderr,"\t");
		print_node(node_link->B);
/*
fprintf(stderr,"\t");
print_sing(node_link->singA);
fprintf(stderr,"\t");
print_sing(node_link->singB);
*/
	fprintf(stderr,"\n");
}

void print_node_links(node_link_info *node_link)
{
	if( node_link == NULL )
		fprintf(stderr,"No node links\n");

	while(node_link != NULL )
	{
		print_node_link(node_link);
		node_link = node_link->next;
        }
	fprintf(stderr,"\n");
}

void print_sing(sing_info *sing)
{
	if(sing == NULL ) fprintf(stderr,"no singularity\n");
	else
	{
		fprintf(stderr,"SING: status %hd adjacent node links %d",sing->status,sing->numNLs);
		print_sol(sing->sing);
	}
}

void print_sings(sing_info *sings)
{
	if(sings == NULL) fprintf(stderr,"No singularities\n");
	while(sings!=NULL)
	{
		print_sing(sings);
		sings = sings->next;
	}
}

int countsings(sing_info *sings)
{
	int num = 0;
	while(sings!=NULL)
	{
		++num;
		sings = sings->next;
	}
	return num;
}

void print_chain(chain_info *chain)
{
	int i;

	if(chain == NULL) fprintf(stderr,"No chain\n");
	fprintf(stderr,"Chain: used %d\n",chain->used);
	for(i=0;i<chain->length;++i)	
		print_sol(chain->sols[i]);
}

void print_chains(chain_info *chains)
{
	if(chains == NULL) fprintf(stderr,"No chains\n");
	while(chains!=NULL)
	{
		print_chain(chains);
		chains = chains->next;
	}
}

void print_nodes_on_box(box_info *box)
{
	print_nodes_on_face(box->ll);
	print_nodes_on_face(box->rr);
	print_nodes_on_face(box->ff);
	print_nodes_on_face(box->bb);
	print_nodes_on_face(box->dd);
	print_nodes_on_face(box->uu);
}

void print_box_brief(box_info *box)
{
	if(box == NULL)
	{
		fprintf(stderr,"NULL box\n");
		return;
	}
	fprintf(stderr,"BOX: (%hd,%hd,%hd)/%hd status %d\n",
		box->xl,box->yl,box->zl,box->denom,box->status);
	print_nodes_on_box(box);
	print_sings(box->sings);
	print_node_links(box->node_links);
}

/*
 * Function:	print_box
 * action:	prints out information about a box.
 */

void print_box(box_info *box)
{
	if(box == NULL)
	{
		fprintf(stderr,"NULL box\n");
		return;
	}
	fprintf(stderr,"BOX: (%hd,%hd,%hd)/%hd status %d\n",
		box->xl,box->yl,box->zl,box->denom,box->status);
	fprintf(stderr,"face ll :"); print_face(box->ll);
	fprintf(stderr,"face rr :"); print_face(box->rr);
	fprintf(stderr,"face ff :"); print_face(box->ff);
	fprintf(stderr,"face bb :"); print_face(box->bb);
	fprintf(stderr,"face dd :"); print_face(box->dd);
	fprintf(stderr,"face uu :"); print_face(box->uu);
	print_sings(box->sings);
	print_node_links(box->node_links);
	print_chains(box->chains);
}

/************************************************************************/
/*									*/
/*	Now procedures to allocate and free cells			*/
/*									*/
/************************************************************************/

int	edgecount, edgemax, edgenew ,
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

void init_cells()
{
#ifdef TEST_ALLOC
	edgecount = edgemax = edgenew = 0;
	facecount = facemax = facenew = 0;
	linkcount = linkmax = linknew = 0;
	nodecount = nodemax = nodenew = 0;
	boxcount =   boxmax = boxnew = 0;
	solcount =  solmax = solnew = 0;
	vertcount = vertmax = vertnew = 0;
	node_linkcount = node_linkmax = node_linknew = 0;
	singcount = singmax = singnew = 0;
	chaincount = chainmax=chainnew = 0;
	facet_solcount = facet_solmax = facet_solnew = 0;
	facetcount = facetmax = facetnew = 0;
#endif
}

void fini_cells()
{
#ifdef TEST_ALLOC
	fprintf(stderr,"edge\t%d\t%d\t%d\t%d\t%d\n",edgecount,edgenew,edgemax,sizeof(edge_info),edgenew*sizeof(edge_info));
	fprintf(stderr,"face\t%d\t%d\t%d\t%d\t%d\n",facecount,facenew,facemax,sizeof(face_info),facenew*sizeof(face_info));
	fprintf(stderr,"link\t%d\t%d\t%d\t%d\t%d\n",linkcount,linknew,linkmax,sizeof(link_info),linknew*sizeof(link_info));
	fprintf(stderr,"node\t%d\t%d\t%d\t%d\t%d\n",nodecount,nodenew,nodemax,sizeof(node_info),nodenew*sizeof(node_info));
	fprintf(stderr,"box\t%d\t%d\t%d\t%d\t%d\n",boxcount,boxnew,boxmax,sizeof(box_info),boxnew*sizeof(box_info));
	fprintf(stderr,"sol\t%d\t%d\t%d\t%d\t%d\n",solcount,solnew,solmax,sizeof(sol_info),solnew*sizeof(sol_info));
	fprintf(stderr,"nl\t%d\t%d\t%d\t%d\t%d\n",node_linkcount,node_linknew,node_linkmax,sizeof(node_link_info),node_linknew*sizeof(node_link_info));
	fprintf(stderr,"sing\t%d\t%d\t%d\t%d\t%d\n",singcount,singnew,singmax,sizeof(sing_info),singnew*sizeof(sing_info));
	fprintf(stderr,"chain\t%d\t%d\t%d\t%d\t%d\n",chaincount,chainnew,chainmax,sizeof(chain_info),chainnew*sizeof(chain_info));
	fprintf(stderr,"fs\t%d\t%d\t%d\t%d\t%d\n",facet_solcount,facet_solnew,facet_solmax,sizeof(facet_sol),facet_solnew*sizeof(facet_sol));
	fprintf(stderr,"facet\t%d\t%d\t%d\t%d\t%d\n",facetcount,facetnew,facetmax,sizeof(facet_info),facetnew*sizeof(facet_info));
#endif
}

struct edgelist { struct edgelist *next; };
struct edgelist *FreeEdges = NULL;

edge_info *alloc_edge()
{
	edge_info *edge;

	++edgecount;
	++edgemax;
	if(FreeEdges != NULL)
	{
		edge = (edge_info *) FreeEdges;
		FreeEdges = FreeEdges->next;
		bzero((void *)edge,sizeof(edge_info));
	}
	else
	{
		++edgenew;
		edge = (edge_info *) malloc(sizeof(edge_info));
		if(edge == NULL )
		{
			fprintf(stderr,"ERROR: alloc_edge NULL allocation, malloc failed, (edgenew %d).\n",edgenew);
			exit(-1);
		}
	}

	return(edge);
}

/*
 * Function:	free_edge
 * action:	frees an edge and all the sols on it.
 */

void free_edge(edge_info *edge)
{
	struct edgelist *list;

	if(edge == NULL ) return;
	--edgecount;
	if(edge->left != NULL ) free_edge(edge->left);
	if(edge->right != NULL ) free_edge(edge->right);
	if(edge->sol != NULL ) free(edge->sol);
	edge->left = edge->right = NULL;
	edge->sol = NULL;
	list = (struct edgelist *) edge;
	list->next = FreeEdges;
	FreeEdges = list;
}

link_info *FreeLinks = NULL;

link_info *alloc_link()
{
	link_info *link;

	++linkcount;
	++linkmax;
	if(FreeLinks != NULL)
	{
		link = FreeLinks;
		FreeLinks = FreeLinks->next;
		bzero((void *)link,sizeof(link_info));
	}
	else
	{
		++linknew;
		link = (link_info *) malloc(sizeof(link_info));
	}
	return(link);
}

void free_links(link_info *links)
{
	link_info *temp;

	if(links == NULL)return;
	temp = links;
	
	while(temp->next!=NULL)
	{
		--linkcount;
		temp = temp->next;
	}
	temp->next = FreeLinks;
	FreeLinks = links;
}

node_info *FreeNodes = NULL;

node_info *alloc_node()
{
	node_info *node;

	++nodecount;
	++nodemax;
	if(FreeNodes != NULL)
	{
		node = FreeNodes;
		FreeNodes = node->next;
		bzero((void *) node,sizeof(node_info));
	}
	else
	{
		++nodenew;
		node = (node_info *) malloc(sizeof(node_info));
	}
	return(node);
}

void free_nodes(node_info *nodes)
{
	node_info *temp;

	if(nodes == NULL) return;
	temp = nodes;
	while(temp!=NULL)
	{
		free(temp->sol);
#ifdef PRINT_ZERO
		--solcount;
#endif
		if(temp->next == NULL) break;
		temp = temp->next;
		--nodecount;
	}
	temp->next = FreeNodes;
	FreeNodes = temp;
}

face_info *FreeFaces = NULL;

face_info *alloc_face()
{
	face_info *face;

	++facecount;
	++facemax;
	if(FreeFaces != NULL)
	{
		face = FreeFaces;
		FreeFaces = FreeFaces->lb;
		bzero((void *) face,sizeof(face_info));
	}
	else
	{
		++facenew;
		face = (face_info *) malloc(sizeof(face_info));
	}
	return(face);
}

void free_face(face_info *face)
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
	--facecount;
}

/*
 * Function:	free_bits_of_face
 * action:	free those bits of face which won't be used again,
 *		only call for LL,FF and DD faces of a box.
 */

void free_bits_of_face(face_info *face)
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

void free_node_links(node_link_info *node_links)
{
	node_link_info *temp;

	temp = node_links;
	
	while(temp!=NULL)
	{
		temp = node_links->next;
		free(node_links);
#ifdef PRINT_ZERO
		--node_linkcount;
#endif
		node_links = temp;
	}
}

void free_sings(sing_info *sings)
{
	sing_info *temp;

	temp = sings;
	
	while(temp!=NULL)
	{
		temp = sings->next;
		free(sings->sing);
		free(sings->adjacentNLs);
		free(sings);
#ifdef PRINT_ZERO
		--solcount;
		--singcount;
		--node_linkcount;
#endif
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

void free_bits_of_box(box_info *box)
{
	chain_info *chains,*chains2;

/*
	free_sings(box->sings);
*/
	chains = box->chains;
	while(chains!=NULL)
	{
		chains2 = chains->next;
		free(chains->sols);
		free(chains->metLens);
		free(chains);
#ifdef PRINT_ZERO
		--chaincount;
#endif
		chains = chains2;
	}
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

/*
 * Function:	free_box
 * Action:	frees all of the box
 */

void free_box(box_info *box)
{
	if( box == NULL ) return;
	free_bits_of_box(box);
/*
	free_face(box->ll);
	free_face(box->rr);
	free_face(box->ff);
	free_face(box->bb);
	free_face(box->uu);
	free_face(box->dd);
	free_box(box->lfd);
	free_box(box->rfd);
	free_box(box->lbd);
	free_box(box->rbd);
	free_box(box->lfu);
	free_box(box->rfu);
	free_box(box->lbu);
	free_box(box->rbu);
	free(box);
*/
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

void make_edge( edge_info *edge, soltype type, int xl,int yl,int zl,int denom)
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

void make_face_edge(face_info *face, int code, edge_info *edge)
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
		default:
			fprintf(stderr,"bad type %d in make_face_edge\n",code);
			exit(1);
		}
		break;
	default:
		fprintf(stderr,"bad type %d in make_face_edge\n",face->type);
		exit(1);
		
	}
}

void subdevideedge(edge_info *edge,edge_info *edge1,edge_info *edge2)
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
	default:
		fprintf(stderr,"bad type %d in subdevideedge\n",edge->type);
		exit(1);
	}
}

/*
 * Function:	split_edge
 * action:	ensures that edge comprises of two halves and that
 *		if a solution exists then it lies in one of the two
 *		halves.
 */

void split_edge(edge_info *edge)
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
		default:
			fprintf(stderr,"bad type %d in split_edge\n",edge->type);
			exit(1);
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
#ifdef PRI_SPLIT_EDGE
				fprintf(stderr,"split_edge: edge->left->sol != NULL\n");
				print_edge(edge);
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
				print_edge(edge);
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

void make_face(face_info *face, soltype type,int xl,int yl,int zl,int denom)
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

void make_box_face( box_info *box, soltype type, face_info *face)
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
		print_soltype(type);
		break;
	}
}

/*
 * Function:	make_sub_faces
 * action:	creates the four sub faces of a face
 */

void make_sub_faces(face_info *face,face_info *face1,face_info *face2,
	face_info *face3,face_info *face4)
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
	default:
		fprintf(stderr,"bad type %d in make_sub_face\n",face->type);
		exit(1);
	}
}

void make_box(box_info *box,int xl,int yl,int zl,int denom)
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
	box->chains = NULL;
	box->facets = NULL;
}

void subdevidebox(box_info *box,box_info *box1,box_info *box2,
	box_info *box3,box_info *box4,box_info *box5,
	box_info *box6,box_info *box7,box_info *box8)
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

void sub_devide_box(box_info *box)
{
	box->lfd = grballoc(box_info);
	box->rfd = grballoc(box_info);
	box->lbd = grballoc(box_info);
	box->rbd = grballoc(box_info);
	box->lfu = grballoc(box_info);
	box->rfu = grballoc(box_info);
	box->lbu = grballoc(box_info);
	box->rbu = grballoc(box_info);
	boxcount += 8;
	boxmax += 8;
	boxnew += 8;

	subdevidebox(box,box->lfd,box->rfd,box->lbd,box->rbd,
			 box->lfu,box->rfu,box->lbu,box->rbu );
}

/*
 * Function:	calc_pos_on_face
 * action:	vec is the position of sol on the face
 */

void calc_pos_on_face(face_info *face, sol_info *sol, double vec[2])
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
			print_soltype(face->type);
			fprintf(stderr," sol ");
			print_soltype(sol->type);
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
			print_soltype(face->type);
			fprintf(stderr," sol ");
			print_soltype(sol->type);
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
			print_soltype(face->type);
			fprintf(stderr," sol ");
			print_soltype(sol->type);
			fprintf(stderr,"\n");
			break;
		}
		break;
	default:
		fprintf(stderr,"bad type %d in calc_pos_on_face\n",face->type);
		exit(1);
	}	/* end switch(face->type) */
}

/*
 * Function:	calc_pos_in_box
 * action:	vec is the position of sol on the box
 */

void calc_pos_in_box( box_info *box, sol_info *sol, double vec[3])
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
		print_soltype(sol->type);
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

int remove_link( face_info *face, link_info *link)
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
#ifdef PRINT_ZERO
		--linkcount;
#endif
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
#ifdef PRINT_ZERO
		--linkcount;
#endif
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

int remove_node_link( box_info *box, node_link_info *link)
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
#ifdef PRINT_ZERO
		--node_linkcount;
#endif
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
#ifdef PRINT_ZERO
		--node_linkcount;
#endif
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
 * action:	given a link between two solutions and a list of existing
 *		links on the face do the following:
 *		if neither sol in list add link to list;
 *		if one sol matches a sol in list extend the existing link;
 *		if link joins two existing links remove one and
 *		join the two together.
 *
 *		basically do the right thing to the list with the given link.
 */


void include_link( face_info *face, sol_info *sol1, sol_info *sol2)
{
	link_info *link, *link1=NULL, *link2=NULL;
	int link1_keepA = FALSE, link2_keepA = FALSE;

#ifdef PRINT_INCLUDE_LINK
/*
	if( 512 * face->yl == 264 * face->denom)
*/
	{
	fprintf(stderr,"include_link\n");
	print_face(face);
	print_sol(sol1);
	print_sol(sol2);
	}
#endif
	link = face->links;
	while( link != NULL )
	{
		if( sol1 == link->A && sol1->type < FACE_LL )
		{
			if(link->B == sol2) return;

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
			if(link->A == sol2) return;

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
		link->plotted = 0;
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
	{
	fprintf(stderr,"include_link done\n");
	print_face(face);
	}
#endif
}

/*
 * Function:	add_node(face,sol)
 * action:	add the node to the list for the face.
 *		simple version where we don't try to join nodes together.
 */

void add_node( face_info *face, sol_info *sol)
{
	node_info *node;

/*	fprintf(stderr,"add_node: "); print_soltype(face->type); 
	fprintf(stderr," (%d,%d,%d)/%d\n",face->xl,face->yl,face->zl,face->denom);
*/
	if(sol->type < X_AXIS || sol->type > BOX )
	{
		fprintf(stderr,"add_node: bad soltype\n"); 
		print_sol(sol);
		print_face(face);
		exit(1);
	}
	node = alloc_node();
	node->next = face->nodes;
	node->sol = sol;
	node->status = NODE;
	face->nodes = node;

#ifdef PRINT_INCLUDE_NODE_LINK
	if( sol->xl == 317 && sol->yl == 112 && sol->zl == 345)
	{
	fprintf(stderr,"add_node\n");
	print_sol(node->sol);
	print_face(face);
	}
#endif

}

/*
 * Function:	colect_node
 * action:	collect all the MID_FACE solutions together to make
 *		a single node, and have all the solutions joined to it.
 * BUGS:	solution round edge adjicient to node?
 */

void colect_nodes( box_info *box, face_info *face)
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

void add_node_link_simple( box_info *box, node_info *node1, node_info *node2)
{
	node_link_info *link;
	link = grballoc(node_link_info);
	++linkcount; ++linkmax; ++linknew;
	link->A = node1;
	link->B = node2;
	link->singA = NULL;
	link->singB = NULL;
	link->status = NODE;
	link->next = box->node_links;
	box->node_links = link;
}

void add_node_link( box_info *box, node_info *node1, node_info *node2)
{
	node_link_info *link, *link1=NULL, *link2=NULL;
	int link1_keepA = FALSE, link2_keepA = FALSE;

#ifdef not_DEF
	fprintf(stderr,"include_node_link\n");
	print_sol(node1->sol);
	print_sol(node2->sol);
	print_box(box);
#endif

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
#ifdef TEST_ALLOC
		++node_linkcount; ++node_linkmax; ++node_linknew;
#endif
		link->A = node1;
		link->B = node2;
		link->singA = NULL;
		link->singB = NULL;
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

void add_sing( box_info *box, sol_info *sol)
{
	sing_info *sing;

	sing = grballoc(sing_info);
#ifdef TEST_ALLOC
	++singcount; ++ singmax; ++singnew;
#endif
	sing->sing = sol;
	sing->numNLs = 0;
	sing->adjacentNLs = NULL;
	sing->status = NODE;
	sing->next = box->sings;
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

void collect_sings( box_info *box)
{
	node_link_info *n1,*n2,*p1,*p2;
	sing_info *sing;

	/* remove any repeated node links */
/*
	print_node_links(box->node_links);
*/
	for(p1=NULL,n1=box->node_links;n1!=NULL;p1=p1,n1=n1->next)
	{
		n1->singA = n1->singB = NULL;
		for(p2=n1,n2=n1->next;n2!=NULL;p2=n2,n2=n2->next)
		{
			if( (  (n1->A->sol == n2->A->sol )
			    && (n1->B->sol == n2->B->sol ) )
			 || (  (n1->B->sol == n2->A->sol )
			    && (n1->A->sol == n2->B->sol ) ) )
			{
/*
fprintf(stderr,"rm nodel_link\n");
		print_node(n1->A);	
		fprintf(stderr,"\t");
		print_node(n1->B);
		fprintf(stderr,"\n");
		print_node(n2->A);	
		fprintf(stderr,"\t");
		print_node(n2->B);
		fprintf(stderr,"\n");
*/
				p2->next = n2->next;
				free(n2);
#ifdef TEST_ALLOC
		--node_linkcount;
#endif
				n2 = p2;
			}
		}
	}

	/* count up how many node_links adjacent to each sing */

	box->num_sings = 0;
	for(sing=box->sings;sing!=NULL;sing=sing->next)
	{
		++box->num_sings;
		sing->numNLs = 0;
		for(n1=box->node_links;n1!=NULL;n1=n1->next)
		{
/*
			if( n1->A->sol->type != BOX
			 && n1->B->sol->type != BOX ) continue;
*/
			if(n1->A->sol == sing->sing)
			{
				++sing->numNLs;
			}
			if(n1->B->sol == sing->sing)
			{
				++sing->numNLs;
			}
		}
		sing->adjacentNLs = (node_link_info **) malloc(sizeof(node_link_info *)*sing->numNLs);
#ifdef TEST_ALLOC
		++node_linkcount; ++node_linkmax; ++node_linknew;
#endif
		sing->numNLs = 0;
		for(n1=box->node_links;n1!=NULL;n1=n1->next)
		{
/*
			if( n1->A->sol->type != BOX
			 && n1->B->sol->type != BOX ) continue;
*/
			if(n1->A->sol == sing->sing)
			{
				sing->adjacentNLs[sing->numNLs++] = n1;
				n1->singA = sing;
			}
			if(n1->B->sol == sing->sing)
			{
				sing->adjacentNLs[sing->numNLs++] = n1;
				n1->singB = sing;
			}
		}


	}


			
#ifdef COLLECT

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
#ifdef TEST_ALLOC
				--solcount;
#endif
				node_link->A->sol = sing->sing;
			}
			if( adjacient_to_sing(sing,node_link->B->sol) )
			{
/*
*/
				free(node_link->B->sol);
#ifdef TEST_ALLOC
				--solcount;
#endif
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

int adjacient_to_node( node_info *node, sol_info *sol)
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
	print_soltype(node->sol->type);
	fprintf(stderr," sol ");
	print_soltype(sol->type);
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

int adjacient_to_sing( sing_info *sing, sol_info *sol)
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

sol_info *make_sol( soltype type,int xl,int yl,int zl,int denom,double root)
{
        sol_info *temp;

	if(type < X_AXIS || type > Z_AXIS )
	{
		fprintf(stderr,"make_sol: Bad type %d\n",type);
		exit(1);
	}
        temp = grballoc(sol_info);
#ifdef TEST_ALLOC
	++solcount;++solmax;++solnew;
#endif
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

sol_info *make_sol2( soltype type,int xl,int yl,int zl,int denom,
	double root,double root2)
{
        sol_info *temp;

	if(type < FACE_LL || type > FACE_UU )
	{
		fprintf(stderr,"make_sol2: Bad type %d\n",type);
		exit(1);
	}
        temp = grballoc(sol_info);
#ifdef TEST_ALLOC
	++solcount;++solmax;++solnew;
#endif
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

sol_info *make_sol3( soltype type,int xl,int yl,int zl,int denom,
	double root,double root2,double root3)
{
        sol_info *temp;

 	if(type != BOX )
	{
		fprintf(stderr,"make_sol2: Bad type %d\n",type);
		exit(1);
	}
       temp = grballoc(sol_info);
#ifdef TEST_ALLOC
	++solcount;++solmax;++solnew;
#endif
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

void calc_pos( sol_info *sol, double vec[3])
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

/* get the values for the roots, inverse of calc_pos */

void calc_relative_pos(sol_info *sol,double vec[3])
{
	sol->root  = vec[0] * sol->denom - sol->xl;		
	sol->root2 = vec[1] * sol->denom - sol->yl;		
	sol->root3 = vec[2] * sol->denom - sol->zl;
}


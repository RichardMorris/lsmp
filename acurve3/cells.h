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

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !(FALSE)
#endif

/************************************************************************/
/*									*/
/*	The type definition for a cell.					*/
/*									*/
/************************************************************************/

typedef enum
 {NONE, VERTEX, X_AXIS, Y_AXIS, Z_AXIS,
  FACE_LL, FACE_RR, FACE_FF, FACE_BB, FACE_DD, FACE_UU, BOX} soltype;

typedef struct
{
	short xl,yl,zl,denom;
	soltype type;
	short status;
} vert_info;

typedef struct
{
	short xl,yl,zl,denom;
	double root,root2,root3;
	int dx,dy,dz;
	soltype type;
	short status;
} sol_info;

typedef struct edge_node
{
	short xl,yl,zl,denom;
	soltype type;
	short status;
	sol_info *sol;
	struct edge_node *left,*right;
/*
	vert_info *low, *high;
*/
} edge_info;

typedef struct link_ele
{
	sol_info *A,*B;
	short	status;
	struct link_ele *next;
} link_info;

typedef struct node_ele
{
	sol_info *sol;
	short status;
	struct node_ele *next;
} node_info;

typedef struct face_node
{
	short xl,yl,zl,denom;
	soltype type;
	short status;
	edge_info *x_low, *y_low, *x_high, *y_high;
	link_info *links;
	node_info *nodes;
	struct face_node *lb,*rb,*lt,*rt;
	int count;
} face_info;

typedef struct node_link_ele
{
	node_info *A,*B;
	short status;
	struct node_link_ele *next;
} node_link_info;

typedef struct sing_ele
{
	sol_info *sing;
	short status;
	struct sing_ele *next;
} sing_info;

typedef struct box_node
{
	short xl,yl,zl,denom;
	soltype type;
	short status;
	struct box_node *lfd,*lfu,*lbd,*lbu,*rfd,*rfu,*rbd,*rbu;
	face_info *ll,*rr,*ff,*bb,*dd,*uu;
	node_link_info *node_links;
	sing_info *sings;
} box_info;

typedef struct
{
	double xmin,xmax,ymin,ymax,zmin,zmax;
	int xord,yord,zord;
} region_info;

/*** Some definitions to assist in making edges on faces ***/

#define X_LOW 1
#define X_HIGH 2
#define Y_LOW 3
#define Y_HIGH 4
#define MID_FACE 5
#define BAD_EDGE -0.5		/* code to signal find_edge failed */

/*** Definitions to indicate weather at the bottom of oct-tree or not ***/

#define NODE 0
#define LEAF 1

/*** Codes for link status ***/

#ifndef NODE
#define NODE 0
#endif
#define LINK 1

#ifndef I_AM_CELLS
extern printsoltype(soltype);
extern print_sol(sol_info *);
extern make_face(face_info *,soltype,int,int,int,int);
extern make_sub_faces(face_info *,
		face_info *,face_info *,face_info *,face_info *);
extern sol_info *make_sol(soltype,int,int,int,int,double);
extern sol_info *make_sol2(soltype,int,int,int,int,double,double);
extern sol_info *make_sol3(soltype,int,int,int,int,double,double,double);
extern box_info *get_box(int,int,int,int);
extern edge_info *alloc_edge();
extern face_info *alloc_face();
extern node_info *alloc_node();
extern link_info *alloc_link();
#endif

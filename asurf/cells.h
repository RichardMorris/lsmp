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

#define FOUND_VAL 0x01
#define FOUND_DX  0x02
#define FOUND_DY  0x04
#define FOUND_DZ  0x08

typedef struct Sol_info
{
	int xl,yl,zl,denom;
	double root,root2,root3;
	short dx,dy,dz; 
	short dxx,dxy,dxz,dyy,dyz,dzz;
	soltype type;
	short status; short is_sing;
	int plotindex;
	struct Sol_info *nextfree;
} sol_info;

typedef struct edge_node
{
	int xl,yl,zl,denom;
	soltype type;
	short status,refcount;
	sol_info *sol;
	struct edge_node *left,*right,*nextfree;
} edge_info;

typedef struct link_ele
{
	sol_info *A,*B;
	short	status; short plotted;
	struct link_ele *next,*nextfree;
} link_info;

typedef struct node_ele
{
	sol_info *sol;
	short status;
	struct node_ele *next,*nextfree;
} node_info;

typedef struct face_node
{
	int xl,yl,zl,denom;
	soltype type;
	short status;
	edge_info *x_low, *y_low, *x_high, *y_high;
	link_info *links;
	node_info *nodes;
	struct face_node *lb,*rb,*lt,*rt,*nextfree;
} face_info;

typedef struct node_link_ele
{
	node_info *A,*B;
	struct sing_ele *singA,*singB;
	short status;
	struct node_link_ele *next,*nextfree;
} node_link_info;

typedef struct sing_ele
{
	sol_info *sing;
	short status;
	short numNLs;
	node_link_info **adjacentNLs;
	struct sing_ele *next,*nextfree;
} sing_info;

typedef struct chain_node
{
	short length;		/* The number of sols in the chain */
	short used;		/* whether this chain has already been used to split a facet */
	sol_info **sols;	/* an array of sols */
	float metric_length;	/* the length of the chain */
	float *metLens;
	struct chain_node *next;	/* pointer to the next chain in the list */
} chain_info;

typedef struct facet_sol
{
	sol_info *sol;
	struct facet_sol *next;
} facet_sol;

typedef struct facet_info
{
	facet_sol *sols;
	struct facet_info *next;
	int	numsing;
	int flag;
} facet_info;

typedef struct box_node
{
	int xl,yl,zl,denom;
	soltype type;
	short status,num_sings;
	struct box_node *lfd,*lfu,*lbd,*lbu,*rfd,*rfu,*rbd,*rbu,*nextfree;
	face_info *ll,*rr,*ff,*bb,*dd,*uu;
	node_link_info *node_links;
	chain_info *chains;
	sing_info *sings;
	facet_info *facets;
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
#define X_LOW_Y_LOW 6
#define X_LOW_Y_HIGH 7
#define X_HIGH_Y_LOW 8
#define X_HIGH_Y_HIGH 9

#define BAD_EDGE -0.5		/* code to signal find_edge failed */

/*** Definitions to indicate weather at the bottom of oct-tree or not ***/

#define NODE 0
#define LEAF 1

/*** Codes for link status ***/

#ifndef NODE
#define NODE 0
#endif
#define LINK 1

extern void print_soltype(soltype type);
extern void print_sol(sol_info *temp);
extern void print_sols_on_edge(edge_info *edge);
extern void print_edge(edge_info *edge);
extern void print_link(link_info *link);
extern void print_links(link_info *link);
extern void print_node(node_info *node);
extern void print_nodes(node_info *node);
extern void print_nodes_on_face(face_info *face);
extern void print_face(face_info *face);
extern void print_face_brief(face_info *face);
extern void print_node_link(node_link_info *node_link);
extern void print_node_links(node_link_info *node_link);
extern void print_sing(sing_info *sing);
extern void print_sings(sing_info *sings);
extern void print_chain(chain_info *chain);
extern void print_chains(chain_info *chains);
extern void print_box(box_info *box);
extern void print_nodes_on_box(box_info *box);
extern void print_box_brief(box_info *box);

extern void make_box(box_info *box,int xl,int yl,int zl,int denom);
extern void make_face(face_info *,soltype,int,int,int,int);
extern void make_box_face( box_info *box, soltype type, face_info *face);
extern void make_sub_faces(face_info *,
 		face_info *,face_info *,face_info *,face_info *);
extern sol_info *make_sol(soltype,int,int,int,int,double);
extern sol_info *make_sol2(soltype,int,int,int,int,double,double);
extern sol_info *make_sol3(soltype,int,int,int,int,double,double,double);
extern box_info *get_box(int,int,int,int);

extern edge_info *alloc_edge();
extern face_info *alloc_face();
extern node_info *alloc_node();
extern link_info *alloc_link();

extern sol_info *allocsol();
/*extern vert_info *allocvert();*/
extern edge_info *allocedge();
extern node_info *allocnode();
extern link_info *alloclink();
extern face_info *allocface();
extern node_link_info *allocnode_link();
extern sing_info *allocsing();
extern box_info *allocbox();
extern void free_bits_of_box(box_info *box);

extern void free_box(box_info *box);
extern void free_bits_of_box(box_info *box);
extern void free_sings(sing_info *sings);
extern void free_node_links(node_link_info *node_links);
extern void free_bits_of_face(face_info *face);
extern void free_face(face_info *face);
extern void free_nodes(node_info *nodes);
extern void free_links(link_info *links);
extern void free_edge(edge_info *edge);

extern void init_cells();
extern void fini_cells();
extern void subdevidebox(box_info *box,box_info *box1,box_info *box2,
	box_info *box3,box_info *box4,box_info *box5,
	box_info *box6,box_info *box7,box_info *box8);
extern void sub_devide_box(box_info *box);
extern void colect_nodes( box_info *box, face_info *face);
extern void subdevideedge(edge_info *edge,edge_info *edge1,edge_info *edge2);
extern void calc_pos( sol_info *sol, double vec[3]);
extern void calc_relative_pos(sol_info *sol,double vec[3]);

extern void calc_pos_on_face(face_info *face, sol_info *sol, double vec[2]);
extern void calc_pos_in_box( box_info *box, sol_info *sol, double vec[3]);

extern void include_link( face_info *face, sol_info *sol1, sol_info *sol2);
extern void add_node( face_info *face, sol_info *sol);
extern void add_node_link( box_info *box, node_info *node1, node_info *node2);
extern void add_node_link_simple( box_info *box, node_info *node1, node_info *node2);
extern void add_sing( box_info *box, sol_info *sol);
extern void collect_sings( box_info *box);
extern void split_edge(edge_info *edge);
extern void make_face_edge(face_info *face, int code, edge_info *edge);








#ifndef JVXCORE_H
#define JVXCORE_H

#include "../CVcommon.h"

typedef struct
{
	int num_points,num_colors,num_normals,num_textures;
	int point_dim,color_dim,normal_dim,texture_dim;
	double *points;
	int *colors;
	double *normals;
	double *textures;
} jvx_pointSet;

/*
<!ELEMENT faceSet (faces,neighbours?,edges?,colors?,colorsBack?,normals?,textures?,boundaries?)>
*/

typedef struct
{
	int	num;
	int	*v;
} jvx_f;
	
typedef struct
{
	int num_faces;
/*		num_neighbours,num_edges,num_colors,num_colorsBack,
		num_normals,num_textures,_num_boundaries; */
	jvx_f	**faces;
} jvx_faceSet;

typedef struct
{
	int num_lines;
/*		num_neighbours,num_edges,num_colors,num_colorsBack,
		num_normals,num_textures,_num_boundaries; */
	int	*lines;
} jvx_lineSet;

/* returns true if there has been an error */
extern int test_xml_errors(xml_tree *root);
/* prints all errors ot fp */
extern void print_xml_errors(FILE *fp,xml_tree *root);
extern char *get_first_xml_error(xml_tree *root);

/** prints a short version of a jvx tree, 
 	only print 'limit' numbers of the <p>, <n>, ... elements. 4 is a good choice! 
    If limit is 0 print whole tree. */
extern void print_brief_jvx_tree(FILE *fp,xml_tree *root,int depth,int limit);

extern void print_brief_xml_tree(FILE *fp,xml_tree *root,int depth,int limit);
extern void print_brief_xml_node_tail(FILE *fp,xml_tree *root,int depth,int limit);
extern void print_brief_xml_node_children(FILE *fp,xml_tree *root,int depth,int limit);
extern void print_brief_xml_node_head(FILE *fp,xml_tree *root,int depth,int limit);

extern void print_jvx_subtree(FILE *fp,xml_tree *root,char *base,int depth);

/* deletes specified elements */

extern void delete_child_from_jvx_tree(xml_tree *root,char *delname);

/** Constructs a jvx tree from string. **/
extern xml_tree *parse_jvx(char *in);
extern xml_tree *fparse_jvx(FILE *fp,int cl,FILE *fl);

/** create a pointSet jvx_tree from an array of points **/
extern xml_tree *create_pointSet_from_data(int dim,int num_points,double *points);

/** create a lineSet jvx_tree from an array of lines **/
extern xml_tree *create_lineSet_from_data(int num_lines,int *lines);

/** create a color type from a number 0 = Black, ... **/
extern xml_tree *create_jvx_color_from_color_number(int num);
extern void add_sub_child_to_lineSet(xml_tree *root,xml_tree *child);

/** add a child to a jvx tree **/
extern void add_jvx_child(xml_tree *root,xml_tree *child);

extern void add_child_to_geometry(xml_tree *root,xml_tree *child);
extern void set_jvx_attribute(xml_tree *root,char *name,char *value);

/** Find first child matching name **/
extern xml_tree *find_child_in_jvx_tree(xml_tree *root,char *name);

/** Finds the points in a jvx **/
extern HPoint3 *get_jvx_points(xml_tree *jvx,int *num_points,int *dim);

#endif

/*****************************************************************
 * outline.c
 *
 * Copyright 1999, Clark Cooper
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the license contained in the
 * COPYING file that comes with the expat distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Read an XML document from standard input and print an element
 * outline on standard output.
 */


#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "../CVcommon.h"

#define BUFFSIZE        8192
#define ALLOC_CHUNK 16

char Buff[BUFFSIZE];

int Depth;

typedef struct xml_tree
{
	int n_attr,n_child,alloc_child;
	char *name;
	char **attr;
	char *data;
	struct xml_tree **children;
} xml_tree;

xml_tree *stack[32];

/* trims starting and ending spaces */

static char *my_trim(const char *s,int len)
{
	int i,j;
	char *str;
	for(i=0;i<len;++i)
	{
		if(! isspace(s[i]) ) break;
	}
	for(j=len-1;j>=0;--j)
	{
		if(! isspace(s[j]) ) break;
	}
/*
fprintf(stderr,"my trim %d %d %d",i,j,len);
*/
	if(i==len || i>j)
	{
		str = (char *) malloc(sizeof(char));
		str[0]='\0';
	}
	else
	{
		str = (char *) calloc(sizeof(char),j-i+2);
		strncpy(str,s+i,j-i+1);
	}
	return(str);
}

/** Create a node to hold info about an element. */

static xml_tree *create_node(const char *el, const char **attr)
{
	int i;
/*
fprintf(stderr,"creat_node %s\n",el);
*/
	xml_tree *tree;
	tree = (xml_tree *) malloc(sizeof(xml_tree));
	tree->name = (char *) strdup(el);
	for (i = 0; attr[i]; i += 2) {}
	tree->n_attr = i;
	tree->attr = calloc(sizeof(char *),i);
	for (i = 0; attr[i]; i += 2) 
	{
		tree->attr[i] = (char *) strdup(attr[i]);
	 	tree->attr[i+1] = (char *) strdup(attr[i+1]);
	}
	tree->n_child =0; tree->alloc_child = 0;
	tree->data = NULL;
}

/** Add a child node to the parent. */

static void add_child(xml_tree *parent,xml_tree *child)
{
/*
fprintf(stderr,"add_child %s %s\n",parent->name,child->name);
*/
	if(parent->alloc_child == 0)
	{
		parent->alloc_child += ALLOC_CHUNK;
		parent->children = (xml_tree **) malloc(sizeof(xml_tree *)*parent->alloc_child);
	}
	else if(parent->n_child+1 > parent->alloc_child)
	{
		parent->alloc_child += ALLOC_CHUNK;
		parent->children = (xml_tree **) 
			realloc(parent->children,sizeof(xml_tree *)*parent->alloc_child);
	}
	parent->children[parent->n_child++] = child;
}

/** add some character data to the element */

static void add_data(xml_tree *parent,char *data)
{
/*
fprintf(stderr,"add_data %s %s\n",parent->name,data);
*/
	if(parent->data == NULL)
	{
		parent->data = strdup(data);
	}
	else
	{
		int len1,len2;

		len1 = strlen(parent->data);
		len2 = strlen(data);
		parent->data = (char *) 
			realloc(parent->data,sizeof(char)*(len1+len2+1));
		strcat(parent->data,data);
	}
/*
	if(parent->alloc_data == 0)
	{
		parent->alloc_data += ALLOC_CHUNK;
		parent->data = (char **) malloc(sizeof(char *)*parent->alloc_data);
	}
	while(parent->n_data+len+1 > parent->alloc_data)
	{
		parent->alloc_data += ALLOC_CHUNK;
		parent->data = (char *) 
			realloc(parent->data,sizeof(char)*parent->alloc_data);
	}
	strcat(parent->data,data);
*/
}

/** Conversion routines to create a jvx_tree from an xml_tree */
/** Converts

       <points num=329>
          <p>
            -0.601252 -1.03 -0.84875
          </p>
	  ....

into an array of points.
Also
        <normals num=329>
          <n>
            0.36618346008406943 -0.7973368174960596 0.47975376290980154
          </n>

        <faces num=288>
          <f>
            0 1 2
          </f>

        <neighbours num=288>
          <nb>
            -1 -1 -1
          </nb>

        <colors num=288 type=rgb>
          <c>
            63 0 23
          </c>
*/

/** creates a jvx_tree node, from an xml_tree node with a given type */
static jvx_tree *create_jvx_node(xml_tree *root,jvx_type type)
{
	int i;
	jvx_tree *copy;

	copy = (jvx_tree *) malloc(sizeof(jvx_tree));
	copy->type = type;
	copy->name = strdup(root->name);

	copy->n_attr = root->n_attr;
	if(root->n_attr==0)
		copy->attr = NULL;
	else
		copy->attr = (char **) calloc(sizeof(char *),root->n_attr);
	for(i=0;i<root->n_attr;++i)
		copy->attr[i] = strdup(root->attr[i]);

	if(root->data == NULL)
		copy->data = NULL;
	else
		copy->data = strdup(root->data);

	return copy;
}

/** parse a string like "1.0 2.0 3.0" into an array of ints. */

static double *parse_double_coords(char *s)
{
	int i,count=0;
	char *s1;
	double *eles;

	s1 = strdup(s);
	if(strtok(s1," \n\t\r")==NULL)
	{
		free(s1);
		return(NULL);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;
	strcpy(s1,s);
	eles = (double *) calloc(sizeof(double),count+1);
	eles[0] = (double) count;
	eles[1] = atof(strtok(s1," \n\t\r"));
	for(i=2;i<count+1;++i)
	{
		eles[i] = atof(strtok(NULL," \n\t\r"));
	}
	free(s1);
	return(eles);
}

/**
 * Parse exactly n coordinates in a string.
 * returns false on error.
 * results copies into eles
 */

int parse_n_double_coords(char *s,int n,double *eles)
{
	int i,count=0;
	char *s1;
	int res = 1;

	s1 = strdup(s);
	if(strtok(s1," \n\t\r")==NULL)
	{
		free(s1);
		return(NULL);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;

	if(count != n)
	{
		char str[80];
		sprintf(str,"Expecting %d coordinates but found %d in string (%%s)\n",n,count);
		report_error(HEAD_ERROR,str,s,301);
		res = 0;
	}
	if(n<count) count=n;
	strcpy(s1,s);
	eles[0] = atof(strtok(s1," \n\t\r"));
	for(i=1;i<count;++i)
	{
		eles[i] = atof(strtok(NULL," \n\t\r"));
	}
	free(s1);
	return(res);
}

/** parse exactly n int coordinates. */
static int parse_n_int_coords(char *s,int n,int *eles)
{
	int i,count=0,res=1;
	char *s1;

	s1 = strdup(s);
	if(strtok(s1," \n\t\r")==NULL)
	{
		free(s1);
		return(NULL);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;
	strcpy(s1,s);

	if(count != n)
	{
		char str[80];
		sprintf(str,"Expecting %d coordinates but found %d in string (%%s)\n",n,count);
		report_error(HEAD_ERROR,str,s,301);
		res = 0;
	}
	eles[0] = atoi(strtok(s1," \n\t\r"));
	for(i=1;i<count;++i)
	{
		eles[i] = atoi(strtok(NULL," \n\t\r"));
	}
	free(s1);
	return eles;
}

/** parse a string like "1 2 3" into an array of ints. */
static int *parse_int_coords(char *s)
{
	int i,count=0;
	char *s1;
	int *eles;

	s1 = strdup(s);
	if(strtok(s1," \n\t\r")==NULL)
	{
		free(s1);
		return(NULL);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;

	strcpy(s1,s);
	eles = (int *) calloc(sizeof(int),count+1);
	eles[0] = (int) count;
	eles[1] = atoi(strtok(s1," \n\t\r"));
	for(i=1;i<count+1;++i)
	{
		eles[i] = atoi(strtok(NULL," \n\t\r"));
	}
	free(s1);
	return eles;
}

static jvx_tree *create_jvx_tree(xml_tree *root);

/** copy the child elements of a xml_tree into a jv_tree.
	If the child elements are <p> <n> etc.
	compress the into any arrays double u.p[num_points][dim] ec.
**/

static void jvx_copy_children(jvx_tree *copy,xml_tree *root)
{
	int i,n_child=0,n_points=0;
	for(i=0;i<root->n_child;++i)
	{
		char *s,*data;
		s = root->children[i]->name;
		switch(copy->type)
		{
		case JVX_POINTS:
			if(!strcmp(s,"p") ) ++n_points; else ++n_child; break;	
		case JVX_COLORS:
		case JVX_COLORSBACK:
			if(!strcmp(s,"c") ) ++n_points; else ++n_child; break;	
		case JVX_NORMALS:
			if(!strcmp(s,"n") ) ++n_points; else ++n_child; break;	
		case JVX_LINES:
			if(!strcmp(s,"l") ) ++n_points; else ++n_child; break;	
		case JVX_FACES:
			if(!strcmp(s,"f") ) ++n_points; else ++n_child; break;	
		case JVX_EDGES:
			if(!strcmp(s,"e") ) ++n_points; else ++n_child; break;	
		case JVX_NEIGHBOURS:
			if(!strcmp(s,"nb") ) ++n_points; else ++n_child; break;	
		case JVX_VECTORS:
			if(!strcmp(s,"v") ) ++n_points; else ++n_child; break;	
		case JVX_TEXTURES:
			if(!strcmp(s,"t") ) ++n_points; else ++n_child; break;
		default:
			++n_child;
		}
	}

	/* Creat space for child data either child or array of points */

	if(n_child==0)
	{
		copy->n_child = 0;
		copy->children = NULL;
	}
	else
	{
		copy->n_child = n_child;
		copy->children = (jvx_tree **) calloc(sizeof(jvx_tree *),n_child);
	}
	copy->n_points = n_points;
	switch(copy->type)
	{
	case JVX_POINTS:
		copy->u.p = (double **) calloc(sizeof(double *),n_points); break;
	case JVX_COLORS:
	case JVX_COLORSBACK:
		copy->u.c = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_NORMALS:
		copy->u.n = (double **) calloc(sizeof(double *),n_points); break;
	case JVX_LINES:
		copy->u.l = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_FACES:
		copy->u.f = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_EDGES:
		copy->u.e = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_NEIGHBOURS:
		copy->u.nb = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_VECTORS:
		copy->u.v = (double **) calloc(sizeof(double *),n_points); break;
	case JVX_TEXTURES:
		copy->u.t = (double **) calloc(sizeof(double *),n_points); break;
	}
	n_points = 0; n_child = 0;
	for(i=0;i<root->n_child;++i)
	{
		char *s;
		s = root->children[i]->name;

		switch(copy->type)
		{
		case JVX_POINTS:
			if(!strcmp(s,"p") )
				copy->u.p[n_points++] = parse_double_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_COLORS:
		case JVX_COLORSBACK:
			if(!strcmp(s,"c") )
				copy->u.c[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_NORMALS:
			if(!strcmp(s,"n") )
				copy->u.n[n_points++] = parse_double_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_LINES:
			if(!strcmp(s,"l") )
				copy->u.l[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_FACES:
			if(!strcmp(s,"f") )
				copy->u.f[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_EDGES:
			if(!strcmp(s,"e") )
				copy->u.e[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_NEIGHBOURS:
			if(!strcmp(s,"nb") )
				copy->u.nb[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_VECTORS:
			if(!strcmp(s,"v") )
				copy->u.v[n_points++] = parse_double_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_TEXTURES:
			if(!strcmp(s,"t") )
				copy->u.t[n_points++] = parse_double_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		default:
			copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		}
	}
}

/** crate a jvx tree from an xml tree */	
static jvx_tree *create_jvx_tree(xml_tree *root)
{
	jvx_tree *node;
	jvx_type type;
	type = JVX_DEFAULT;
	if(!strcmp(root->name,"points")) type = JVX_POINTS;
	if(!strcmp(root->name,"colors")) type = JVX_COLORS;
	if(!strcmp(root->name,"colorsBack")) type = JVX_COLORSBACK;
	if(!strcmp(root->name,"normals")) type = JVX_NORMALS;
	if(!strcmp(root->name,"lines")) type = JVX_LINES;
	if(!strcmp(root->name,"faces")) type = JVX_FACES;
	if(!strcmp(root->name,"edges")) type = JVX_EDGES;
	if(!strcmp(root->name,"neighbours")) type = JVX_NEIGHBOURS;
	if(!strcmp(root->name,"vectors")) type = JVX_VECTORS;
	if(!strcmp(root->name,"textures")) type = JVX_TEXTURES;
	if(!strcmp(root->name,"geometry")) type = JVX_GEOMETRY;
	if(!strcmp(root->name,"pointSet")) type = JVX_POINTSET;
	if(!strcmp(root->name,"faceSet")) type = JVX_FACESET;
	if(!strcmp(root->name,"lineSet")) type = JVX_LINESET;
	if(!strcmp(root->name,"vectorField")) type = JVX_VECTORFIELD;

	node = create_jvx_node(root,type);
	jvx_copy_children(node,root);
	return(node);
}

/** cleans up pcdata in a jvx tree, i.e. trim all pcdata elements. **/
void clean_xml_tree(xml_tree *root)
{
	char *buf; int j;

	if(root->data!=NULL)
	{
		buf = my_trim(root->data,strlen(root->data));
		free(root->data);
		if(strlen(buf)==0)
		{
			root->data = NULL;
		}
		else
			root->data = buf;
	}
	for(j=0;j<root->n_child;++j)
		clean_xml_tree(root->children[j]);
}

/** frees the space allocated to jvx_tree and its decendants */

void free_jvx_tree(jvx_tree *root)
{
	int j;
/*
printf("free_jvx_tree: %s\n",root->name);
*/
	for(j=0;j<root->n_child;++j)
	{
		free_jvx_tree(root->children[j]);
	}
	if(root->attr != NULL)
	{
		for(j=0;j<root->n_attr;++j)
		{
			free(root->attr[j]);
		}
		free(root->attr);
	}
	if(root->data != NULL) free(root->data);
	switch(root->type)
	{
	case JVX_POINTS:
		free(root->u.p);
		break;
	case JVX_COLORS:
	case JVX_COLORSBACK:
		free(root->u.c);
		break;
	case JVX_NORMALS:
		free(root->u.n);
		break;
	case JVX_LINES:
		free(root->u.l);
		break;
	case JVX_FACES:
		free(root->u.f);
		break;
	case JVX_EDGES:
		free(root->u.e);
		break;
	case JVX_NEIGHBOURS:
		free(root->u.nb);
		break;
	case JVX_VECTORS:
		free(root->u.v);
		break;
	case JVX_TEXTURES:
		free(root->u.t);
		break;
	}	
	free(root);
/*
printf("free_jvx_tree done\n");
*/
}

/** deleate a specific child from jvx_tree */

void delete_child_from_jvx_tree(jvx_tree *root,char *delname)
{
	int i,j;
	int flag = 1;

/*
printf("delete_child_form_jvx_tree: %s %s\n",root->name,delname);
print_brief_jvx_tree(stdout,root,0,4);
*/
	for(j=0;j<root->n_child;++j)
	{
		delete_child_from_jvx_tree(root->children[j],delname);
	}

/*
printf("delete_child_form_jvx_tree: %s %s done dependants\n",root->name,delname);
*/
	while(flag)
	{
		flag = 0;
		for(j=0;j<root->n_child;++j)
		{
			if(!strcmp(root->children[j]->name,delname))
			{
				free_jvx_tree(root->children[j]);
				for(i=j+1;i<root->n_child;++i)
					root->children[i-1] = root->children[i];
				root->children[root->n_child-1] = NULL;
				--root->n_child;
				flag = 1;
/*
printf("AFTER DEL\n");
print_brief_jvx_tree(stdout,root,0,4);
*/
				break;
			}
		}
	}
/*
printf("delete_child_form_jvx_tree done\n");
*/
}
			
/************** printing routines. ***********/

void print_xml_tree(FILE *fp,xml_tree *root,int depth)
{
	int i,j;

	for(i=0;i<depth;++i) fprintf(fp,"  ");
	fprintf(fp,"<%s",root->name);
	for(j=0;j<root->n_attr;j+=2)
	{
		fprintf(fp," %s=%s",root->attr[j],root->attr[j+1]);
	}
	if(root->n_child==0 && root->data== NULL) fprintf(fp,"/");
	fprintf(fp,">\n");
	for(j=0;j<root->n_child;++j)
		print_xml_tree(fp,root->children[j],depth+1);

	if(root->data != NULL)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"  %s\n",root->data);
	}
	if(root->n_child!=0 || root->data != NULL)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"</%s>\n",root->name);
	}
	
}

void print_brief_xml_tree(FILE *fp,xml_tree *root,int depth)
{
	int i,j;
	static char lastname[80];
	static count=0;

	if(!strcmp(root->name,lastname) && strlen(root->name) == strlen(lastname) )
	{
		++count; return;
	}
	else if(count>1)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"  (repeated %d times)\n",count);
	}
	count=1;
	strcpy(lastname,root->name);

	for(i=0;i<depth;++i) fprintf(fp,"  ");
	fprintf(fp,"<%s",root->name);
	for(j=0;j<root->n_attr;j+=2)
	{
		fprintf(fp," %s=%s",root->attr[j],root->attr[j+1]);
	}
	if(root->n_child==0 && root->data==NULL) fprintf(fp,"/");
	fprintf(fp,">\n");
	for(j=0;j<root->n_child;++j)
		print_brief_xml_tree(fp,root->children[j],depth+1);
	if(count>1)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"  (repeated %d times)\n",count);
	}
	count=1;

	if(root->data != NULL)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"  %s\n",root->data);
	}
	if(root->n_child!=0 || root->data!=NULL)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"</%s>\n",root->name);
	}
	
}

/* limit is max_num_of points to print -1 prints all points */

void print_brief_jvx_tree(FILE *fp,jvx_tree *root,int depth,int limit)
{
	int i,j,k,n;

	for(i=0;i<depth;++i) fprintf(fp,"  ");
	fprintf(fp,"<%s ",root->name);
	for(j=0;j<root->n_attr;j+=2)
	{
		fprintf(fp," %s=\"%s\"",root->attr[j],root->attr[j+1]);
	}
	if(root->n_child==0 && root->data== NULL && root->n_points == 0) fprintf(fp,"/");
	fprintf(fp,">\n");

	for(j=0;j<root->n_points;++j)
	{
		if(limit>=0 && j>=limit) break;

		switch(root->type)
		{
		case JVX_POINTS:
			n = (int) root->u.p[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <p>");
			for(k=0;k<n;++k)
				fprintf(fp,"%f ",root->u.p[j][k+1]);
			fprintf(fp,"</p>\n");
			break;
		case JVX_COLORS:
			n = (int) root->u.c[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <c>");
			for(k=0;k<n;++k)
				fprintf(fp,"%d ",root->u.c[j][k+1]);
			fprintf(fp,"</c>\n");
			break;
		case JVX_NORMALS:
			n = (int) root->u.n[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <n>");
			for(k=0;k<n;++k)
				fprintf(fp,"%f ",root->u.n[j][k+1]);
			fprintf(fp,"</n>\n");
			break;
		case JVX_LINES:
			n = (int) root->u.l[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <l>");
			for(k=0;k<n;++k)
				fprintf(fp,"%d ",root->u.l[j][k+1]);
			fprintf(fp,"</l>\n");
			break;
		case JVX_FACES:
			n = (int) root->u.f[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <f>");
			for(k=0;k<n;++k)
				fprintf(fp,"%d ",root->u.f[j][k+1]);
			fprintf(fp,"</f>\n");
			break;
		case JVX_EDGES:
			n = (int) root->u.e[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <e>");
			for(k=0;k<n;++k)
				fprintf(fp,"%d ",root->u.e[j][k+1]);
			fprintf(fp,"</e>\n");
			break;
		case JVX_NEIGHBOURS:
			n = (int) root->u.nb[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <nb>");
			for(k=0;k<n;++k)
				fprintf(fp,"%d ",root->u.nb[j][k+1]);
			fprintf(fp,"</nb>\n");
			break;
		case JVX_VECTORS:
			n = (int) root->u.v[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <v>");
			for(k=0;k<n;++k)
				fprintf(fp,"%f ",root->u.v[j][k+1]);
			fprintf(fp,"</v>\n");
			break;
		case JVX_TEXTURES:
			n = (int) root->u.t[j][0];
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  <t>");
			for(k=0;k<n;++k)
				fprintf(fp,"%f ",root->u.t[j][k+1]);
			fprintf(fp,"</t>\n");
			break;
		}
	}
	if(limit>=0 && root->n_points!=0)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"  %d points total\n",root->n_points);	
	}

	for(j=0;j<root->n_child;++j)
		print_brief_jvx_tree(fp,root->children[j],depth+1,limit);

	if(root->data != NULL)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"  %s\n",root->data);
	}
	if(root->n_child!=0 || root->data != NULL || root->n_points!=0)
	{
		for(i=0;i<depth;++i) fprintf(fp,"  ");
		fprintf(fp,"</%s>\n",root->name);
	}
	
}

void print_jvx_tree(FILE *fp,jvx_tree *root,int depth)
{
	print_brief_jvx_tree(fp,root,depth,-1);
}

void print_jvx_subtree(FILE *fp,jvx_tree *root,char *subtreename)
{
	int j;

	if(!strcmp(root->name,subtreename) )
		print_jvx_tree(fp,root,0);
	else
		for(j=0;j<root->n_child;++j)
			print_jvx_subtree(fp,root->children[j],subtreename);
}

/***** Callback functions for xml parser *******/

static void start(void *data, const char *el, const char **attr)
{
  int i;
  xml_tree *node,*parent;

/*
  for (i = 0; i < Depth; i++)
    fprintf(fp,"  ");

  fprintf(fp,"%s", el);

  for (i = 0; attr[i]; i += 2) {
    fprintf(fp," %s='%s'", attr[i], attr[i + 1]);
  }
  fprintf(fp,"\n");
*/

  node = create_node(el,attr);
  if(Depth>0)
  {
	parent = stack[Depth-1];
	add_child(parent,node);
  }
  stack[Depth] = node;
  Depth++;
}

static void end(void *data, const char *el)
{
  Depth--;
}

static void cdh(void *userData,const XML_Char *s,int len)
{
	char *buffer,*buf2;
	xml_tree *parent;

	/* find first non space chatracter */

	buffer = (char *) calloc(sizeof(char),len+1);
	strncpy(buffer,s,len);
	parent = stack[Depth-1];
	add_data(parent,buffer);
/*
	buf2 = (char *) calloc(sizeof(char),len+1);
	sscanf(buffer,"%s",buf2);

	buf2 = my_trim(buffer,len);	
	if(strlen(buf2)>0)
	{
		int i;
		for (i = 0; i < Depth; i++)
		fprintf(fp,"  ");
		fprintf(fp,"CD(%s)(%s)\n",buffer,buf2);

		parent = stack[Depth-1];
		add_data(parent,buf2);
	}
*/
}

jvx_tree *parse_jvx(char *in)
{
  jvx_tree *jvx;

  XML_Parser p = XML_ParserCreate(NULL);
  if (! p) {
    report_error(HEAD_ERROR, "Couldn't allocate memory for parser\n",321);
    exit(-1);
  }

  XML_SetElementHandler(p, start, end);
  XML_SetCharacterDataHandler(p, cdh);
/*
  XML_SetDefaultHandler(p, cdh);
*/

    if (XML_Parse(p, in, strlen(in), 1) == XML_STATUS_ERROR) 
    {
	char *message;
	message = (char *) calloc(sizeof(char),
		40 + strlen(XML_ErrorString(XML_GetErrorCode(p))));
	sprintf(message,"Parse error at line %d:\n%s\n",
              XML_GetCurrentLineNumber(p),
              XML_ErrorString(XML_GetErrorCode(p)));
	report_error(HEAD_ERROR,message,123);
        exit(-1);
    }

  clean_xml_tree(stack[0]);
/*
  print_brief_xml_tree(stack[0],0);
*/
  jvx = create_jvx_tree(stack[0]);
  print_brief_jvx_tree(stderr,jvx,0,4);
  return jvx;
}



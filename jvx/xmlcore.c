/*****************************************************************
 *
 * Heavily modified from 
 *
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
#include <ctype.h>
#include "../CVcommon.h"
#include "jvx.h"
#include "jvxCore.h"

#define BUFFSIZE        8192
#define ALLOC_CHUNK 16

char Buff[BUFFSIZE];

int Depth;

xml_tree *stack[32];

char print_brief_lastname[80];
int print_brief_count=0;

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
	return tree;
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

/** parse a string like "1.0 2.0 3.0" into an array of ints. */

double *parse_double_coords(char *s)
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

static int parse_n_double_coords(char *s,int n,double *eles)
{
	int i,count=0;
	char *s1;
	int res = 1;

	s1 = strdup(s);
	if(strtok(s1," \n\t\r")==NULL)
	{
		free(s1);
		return(0);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;

	if(count != n)
	{
		char str[80];
		sprintf(str,"Expecting %d coordinates but found %d in string (%%s)\n",n,count);
		report_error2(HEAD_ERROR,str,s,301);
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
		return(0);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;
	strcpy(s1,s);

	if(n == 2 && count == 1)
	{
		strcpy(s1,s);
		eles[0] = atof(strtok(s1," \n\t\r"));
		eles[1] = eles[0];
		free(s1);
		return(res);
	}
	if(count != n)
	{
		char str[80];
		sprintf(str,"Expecting %d coordinates but found %d in string (%%s)\n",n,count);
		report_error2(HEAD_ERROR,str,s,301);
		res = 0;
	}
	eles[0] = atoi(strtok(s1," \n\t\r"));
	for(i=1;i<count;++i)
	{
		eles[i] = atoi(strtok(NULL," \n\t\r"));
	}
	free(s1);
	return res;
}

/** parse a string like "1 2 3" into an array of ints. */

static jvx_f *parse_f(char *s)
{
	int i,count=0;
	char *s1;
	int *eles;
	jvx_f *f;

	s1 = strdup(s);
	if(strtok(s1," \n\t\r")==NULL)
	{
		free(s1);
		return(NULL);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;

	strcpy(s1,s);
	eles = (int *) calloc(sizeof(int),count);
	eles[0] = atoi(strtok(s1," \n\t\r"));
	for(i=1;i<count;++i)
	{
		eles[i] = atoi(strtok(NULL," \n\t\r"));
	}
	free(s1);
	f = (jvx_f *) malloc(sizeof(jvx_f));
	f->num = count;
	f->v = eles;
	return f;
}

/** parse a string like "1 2 3" into an array of ints. */
int *parse_int_coords(char *s)
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

/** cleans up pcdata in a jvx tree, i.e. trim all pcdata elements. **/
static void clean_xml_tree(xml_tree *root)
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

void print_brief_xml_node_head(FILE *fp,xml_tree *root,int depth,int limit)
{
	int i,j;

	for(i=0;i<depth;++i) fprintf(fp,"  ");
	fprintf(fp,"<%s",root->name);
	for(j=0;j<root->n_attr;j+=2)
	{
		fprintf(fp," %s=\"%s\"",root->attr[j],root->attr[j+1]);
	}
	if(root->n_child==0 && root->data==NULL) fprintf(fp,"/");
	fprintf(fp,">\n");
}

void print_brief_xml_node_children(FILE *fp,xml_tree *root,int depth,int limit)
{
	int j;

	for(j=0;j<root->n_child;++j)
		print_brief_xml_tree(fp,root->children[j],depth+1,limit);
}


void print_brief_xml_node_tail(FILE *fp,xml_tree *root,int depth,int limit)
{
	int i;

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

void print_brief_xml_tree(FILE *fp,xml_tree *root,int depth,int limit)
{
	int i;
	if(!strcmp(root->name,print_brief_lastname) )
	{
		if(++print_brief_count > limit && limit>0) return;
	}
	else
	{
		if(print_brief_count>limit && limit>0)
		{	/* end of a sequence of more than limit entries */
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  # repeated %d times\n",print_brief_count);
		}
		print_brief_count=1;
		strcpy(print_brief_lastname,root->name);
	}
	print_brief_xml_node_head(    fp,root,depth,limit);
	print_brief_xml_node_children(fp,root,depth,limit);
	print_brief_xml_node_tail(    fp,root,depth,limit);
}

void print_brief_jvx_tree(FILE *fp,xml_tree *root,int depth,int limit)
{
	int i,j;
	if(!strcmp(root->name,print_brief_lastname) )
	{
		if(++print_brief_count > limit && limit > 0) return;
	}
	else
	{
		if(print_brief_count>limit && limit > 0)
		{	/* end of a sequence of more than limit entries */
			for(i=0;i<depth;++i) fprintf(fp,"  ");
			fprintf(fp,"  # repeated %d times\n",print_brief_count);
		}
		print_brief_count=1;
		strcpy(print_brief_lastname,root->name);
	}
	if(root->type == LSMP_POINTSET)
	{
		jvx_pointSet *ps;
		ps = root->app_info;
		print_brief_xml_node_head(fp,root,depth,limit);

		/* now print points */
		for(i=0;i<depth+1;++i) fprintf(fp,"  ");
		fprintf(fp,"<points num=\"%d\">\n",ps->num_points);
		for(i=0;i<ps->num_points && (limit==0 || i<limit);++i)
		{
			for(j=0;j<depth+2;++j) fprintf(fp,"  ");
			fprintf(fp,"<p>");
			for(j=0;j<ps->point_dim;++j) 
				fprintf(fp,"%f ",ps->points[i*ps->point_dim+j]);
			fprintf(fp,"</p>\n");
		}

		if(root->u.pointSet->points != NULL)
		{
		if(root->u.pointSet->points->u.points->thickness != NULL)
			print_brief_jvx_tree(fp,root->u.pointSet->points->u.points->thickness,depth+2,limit);
		if(root->u.pointSet->points->u.points->colorTag != NULL)
			print_brief_jvx_tree(fp,root->u.pointSet->points->u.points->colorTag,depth+2,limit);
		if(root->u.pointSet->points->u.points->color != NULL)
			print_brief_jvx_tree(fp,root->u.pointSet->points->u.points->color,depth+2,limit);
		if(root->u.pointSet->points->u.points->labelAtt != NULL)
			print_brief_jvx_tree(fp,root->u.pointSet->points->u.points->labelAtt,depth+2,limit);
		}

		for(i=0;i<depth+1;++i) fprintf(fp,"  ");
		fprintf(fp,"</points>\n");
		
		if(ps->num_colors>0)
		{
			/* now print colors */
			for(i=0;i<depth+1;++i) fprintf(fp,"  ");
			fprintf(fp,"<colors num=\"%d\">\n",ps->num_colors);
			for(i=0;i<ps->num_colors && (limit==0 || i<limit);++i)
			{
				for(j=0;j<depth+2;++j) fprintf(fp,"  ");
				fprintf(fp,"<c>");
				for(j=0;j<ps->color_dim;++j) 
					fprintf(fp,"%d ",ps->colors[i*ps->color_dim+j]);
				fprintf(fp,"</c>\n");
			}
			for(i=0;i<depth+1;++i) fprintf(fp,"  ");
			fprintf(fp,"</colors>\n");
		}

		if(ps->num_normals>0)
		{
			/* now print normals */
			for(i=0;i<depth+1;++i) fprintf(fp,"  ");
			fprintf(fp,"<normals num=\"%d\">\n",ps->num_normals);
			for(i=0;i<ps->num_normals && (limit==0 || i<limit);++i)
			{
				for(j=0;j<depth+2;++j) fprintf(fp,"  ");
				fprintf(fp,"<n>");
				for(j=0;j<ps->normal_dim;++j) 
					fprintf(fp,"%f ",ps->normals[i*ps->normal_dim+j]);
				fprintf(fp,"</n>\n");
			}

			if(root->u.pointSet->normals != NULL)
			{
			if(root->u.pointSet->normals->u.normals->thickness != NULL)
				print_brief_jvx_tree(fp,root->u.pointSet->normals->u.normals->thickness,depth+2,limit);
			if(root->u.pointSet->normals->u.normals->length != NULL)
				print_brief_jvx_tree(fp,root->u.pointSet->normals->u.normals->length,depth+2,limit);
			if(root->u.pointSet->normals->u.normals->color != NULL)
				print_brief_jvx_tree(fp,root->u.pointSet->normals->u.normals->color,depth+2,limit);
			}
			for(i=0;i<depth+1;++i) fprintf(fp,"  ");
			fprintf(fp,"</normals>\n");
		}

		if(ps->num_textures>0)
		{
			/* now print textures */
			for(i=0;i<depth+1;++i) fprintf(fp,"  ");
			fprintf(fp,"<textures num=\"%d\">\n",ps->num_textures);
			for(i=0;i<ps->num_textures && (limit==0 || i<limit);++i)
			{
				for(j=0;j<depth+2;++j) fprintf(fp,"  ");
				fprintf(fp,"<t>");
				for(j=0;j<ps->texture_dim;++j) 
					fprintf(fp,"%f ",ps->textures[i*ps->texture_dim+j]);
				fprintf(fp,"</t>\n");
			}
			if(root->u.pointSet->textures != NULL && root->u.pointSet->textures->u.textures->image != NULL)
				print_brief_jvx_tree(fp,root->u.pointSet->textures->u.textures->image,depth+2,limit);
			for(i=0;i<depth+1;++i) fprintf(fp,"  ");
			fprintf(fp,"</textures>\n");
		}

		print_brief_xml_node_tail(    fp,root,depth,limit);
	}
	else if(root->type == LSMP_FACESET)
	{
		jvx_faceSet *fs;
		fs = root->app_info;
		print_brief_xml_node_head(fp,root,depth,limit);

		/* now print points */
		for(i=0;i<depth+1;++i) fprintf(fp,"  ");
		fprintf(fp,"<faces num=\"%d\">\n",fs->num_faces);
		for(i=0;i<fs->num_faces && (limit==0 || i<limit);++i)
		{
			for(j=0;j<depth+2;++j) fprintf(fp,"  ");
			fprintf(fp,"<f>");
			for(j=0;j<fs->faces[i]->num;++j) 
				fprintf(fp,"%d ",fs->faces[i]->v[j]);
			fprintf(fp,"</f>\n");
		}
		for(i=0;i<depth+1;++i) fprintf(fp,"  ");
		fprintf(fp,"</faces>\n");

		print_brief_xml_node_tail(    fp,root,depth,limit);
	}
	else if(root->type == LSMP_LINESET)
	{
		jvx_lineSet *ls;
		ls = root->app_info;
		print_brief_xml_node_head(fp,root,depth,limit);

		/* now print points */
		for(i=0;i<depth+1;++i) fprintf(fp,"  ");
		fprintf(fp,"<lines num=\"%d\">\n",ls->num_lines);
		for(i=0;i<ls->num_lines && (limit==0 || i<limit);++i)
		{
			for(j=0;j<depth+2;++j) fprintf(fp,"  ");
			fprintf(fp,"<l>%d %d</l>\n",ls->lines[i*2],ls->lines[i*2+1]);
		}

		if(root->u.lineSet->lines != NULL && root->u.lineSet->lines->u.lines->color != NULL)
			print_brief_jvx_tree(fp,root->u.lineSet->lines->u.lines->color,depth+2,limit);

		for(i=0;i<depth+1;++i) fprintf(fp,"  ");
		fprintf(fp,"</lines>\n");

		print_brief_xml_node_tail(    fp,root,depth,limit);
	}
	else
	{
		print_brief_xml_node_head(    fp,root,depth,limit);
		for(j=0;j<root->n_child;++j)
			print_brief_jvx_tree(fp,root->children[j],depth+1,limit);
		print_brief_xml_node_tail(    fp,root,depth,limit);
	}
}

void print_jvx_subtree(FILE *fp,xml_tree *root,char *base,int depth)
{
	int i;
	if(!strcmp(root->name,base) ) print_brief_jvx_tree(fp,root,depth,0);
	else
	{
		for(i=0;i<root->n_child;++i)
		{
			print_jvx_subtree(fp,root->children[i],base,depth+1);
		}
	}
}

void print_xml_errors(FILE *fp,xml_tree *root)
{
	int i;
	if(root->error_message != NULL)
		fprintf(fp,"%s: %s %d\n",root->name,root->error_message,root->error);
	else if(root->error != 0)
		fprintf(fp,"%s: %d\n",root->name,root->error);

	for(i=0;i<root->n_child;++i)
		print_xml_errors(fp,root->children[i]);
}

int test_xml_errors(xml_tree *root)
{
	int i,res,ret;

	res=root->error;
	if(root->error_message != NULL) res = 1;

	if(root->error_message != NULL)
		fprintf(stderr,"%s: %s %d\n",root->name,root->error_message,root->error);
	else if(root->error != 0)
		fprintf(stderr,"%s: %d\n",root->name,root->error);

	for(i=0;i<root->n_child;++i)
	{
		ret = test_xml_errors(root->children[i]);
		if( ret ) res = ret;
	}
	if(res != 0) fprintf(stderr,"%s %d\n",root->name,res);
	return res;
}

char *get_first_xml_error(xml_tree *root)
{
	int i;
	if(root->error_message != NULL)
		return root->error_message;
	for(i=0;i<root->n_child;++i)
	{
		char *msg;
		msg = get_first_xml_error(root->children[i]);
		if(msg!=NULL) return msg;
	}
	return NULL;
}

void free_jvx_tree(xml_tree *root) {}

xml_tree *find_child_in_jvx_tree(xml_tree *root,char *name)
{
	int j;
	xml_tree *node;

	if(!strcmp(root->name,name)) return(root);
	for(j=0;j<root->n_child;++j)
	{
		if( (node = find_child_in_jvx_tree(root->children[j],name) ) != NULL)
			return node;
	}
	return NULL;
}

void delete_child_from_jvx_tree(xml_tree *root,char *delname)
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


/***** Callback functions for xml parser *******/

static void start(void *data, const char *el, const char **attr)
{
  xml_tree *node,*parent;

/*
  int i;

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
	char *buffer;
	xml_tree *parent;

	/* find first non space chatracter */

	buffer = (char *) calloc(sizeof(char),len+2);
	strncpy(buffer,s,len);
	parent = stack[Depth-1];
/*
fprintf(stderr,"cdh %p (%s)\n",parent,buffer);
*/
	add_data(parent,buffer);
}

/************ end of callback functions *****************/

void create_pointSet_specific_info(xml_tree *node)
{
	jvx_pointSet *ps;
	struct points *points;
	struct colors *colors;
	struct normals *normals;
	struct textures *textures;
	int i;

	node->error = 0;
	ps = (jvx_pointSet *) malloc(sizeof(jvx_pointSet));
	node->app_info = ps;
	if(node->u.pointSet->points == NULL)
	{
		node->error = 101;
		node->error_message = strdup("No points in node");
		return;
	}
	points = node->u.pointSet->points->u.points;

	ps->num_points = points->n_p;
	if( points->num != NULL && ps->num_points != atoi(points->num) )
	{
		char buf[256];
		sprintf(buf,"Miss match in number of points actual %d from xml %s",
			points->n_p,points->num);
		node->error_message = strdup(buf);
		node->error = 102;
		return;
	}
	ps->point_dim = atoi(node->u.pointSet->dim);
	if(ps->point_dim < 2 || ps->point_dim > 10)
	{
		char buf[256];
		node->error = 103;
		sprintf(buf,"Bad dimension: %s",node->u.pointSet->dim);
		node->error_message = strdup(buf);
		return;
	}
/*
fprintf(stderr,"Num points %d dim %d\n",ps->num_points,ps->point_dim);
*/
	ps->points = (double *) calloc(sizeof(double),ps->num_points*ps->point_dim);

	for(i=0;i<ps->num_points;++i)
	{
		if( !parse_n_double_coords(points->p[i]->data,ps->point_dim,ps->points + i*ps->point_dim) )
		{
			node->error_message = (char *) calloc(sizeof(char),
				strlen("Could not parse point data %s")+strlen(points->p[i]->data));
			sprintf(node->error_message,"Could not parse point data %s",points->p[i]->data);
			node->error = 103;
			return;
		}
	}
/*
	for(i=0;i<3;++i)
	{
		fprintf(stderr,"%f %f %f\n",ps->points[i*ps->point_dim],ps->points[i*ps->point_dim+1],ps->points[i*ps->point_dim+2]);
	}
*/
	/* now do colors */

	ps->num_colors = 0; ps->color_dim = 0;
	if(node->u.pointSet->colors != NULL)
	{
		colors = node->u.pointSet->colors->u.colors;
		if(colors->type == NULL) ps->color_dim = 3;
		if(!strcmp(colors->type,"rgb")) ps->color_dim = 3;
		else if(!strcmp(colors->type,"rgba")) ps->color_dim = 4;
		else if(!strcmp(colors->type,"grey")) ps->color_dim = 3;
		else
		{
			node->error_message = (char *) calloc(sizeof(char),
				strlen("Bad colour type %s")+strlen(colors->type));
				
			sprintf(node->error_message,"Bad color type %s",colors->type);
			node->error = 104;
			return;
		}
		if( colors->num != NULL && ps->num_points != atoi(colors->num) )
		{
			char buf[256];
			sprintf(buf,"Miss match between number of points %d and specified number of colors %s",
				points->n_p,colors->num);
			node->error_message = strdup(buf);
			node->error = 105;
			return;
		}
		if(colors->n_c != ps->num_points)
		{
			char buf[256];
			sprintf(buf,"Miss match between number of points %d and actual number of colors %d",
				points->n_p,colors->n_c);
			node->error_message = strdup(buf);
			node->error = 106;
			return;
		}
		ps->num_colors = colors->n_c;

		ps->colors = (int *) calloc(sizeof(int),ps->num_colors*ps->color_dim);

		for(i=0;i<ps->num_colors;++i)
		{
			if( !parse_n_int_coords(colors->c[i]->data,ps->color_dim,ps->colors + i*ps->color_dim) )
			{
				report_error2(HEAD_ERROR,"Could not parse color data %s",colors->c[i]->data,103);
				node->error = 107;
				return;
			}
		}
		
	}

	/* now do normals */

	ps->num_normals = 0; ps->normal_dim = 0;
	if(node->u.pointSet->normals != NULL)
	{
		normals = node->u.pointSet->normals->u.normals;
		ps->normal_dim = 3;
		if( normals->num != NULL && ps->num_points != atoi(normals->num) )
		{
			char buf[256];
			sprintf(buf,"Miss match between number of points %d and specified number of normals %s",
				points->n_p,normals->num);
			node->error_message = strdup(buf);
			node->error = 108;
			return;
		}
		if(normals->n_n != ps->num_points)
		{
			char buf[256];
			sprintf(buf,"Miss match between number of points %d and actual number of normals %d",
				points->n_p,normals->n_n);
			node->error_message = strdup(buf);
			node->error = 109;
			return;
		}
		ps->num_normals = normals->n_n;

		ps->normals = (double *) calloc(sizeof(double),ps->num_normals*ps->normal_dim);

		for(i=0;i<ps->num_normals;++i)
		{
			if( !parse_n_double_coords(normals->n[i]->data,ps->normal_dim,ps->normals + i*ps->normal_dim) )
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("Could not parse point data %s")+strlen(normals->n[i]->data));
				sprintf(node->error_message,"Could not parse normal data %s",normals->n[i]->data);
				node->error = 110;
				return;
			}
		}
	}

	/* now do textures */

	ps->num_textures = 0; ps->texture_dim = 0;
	if(node->u.pointSet->textures != NULL)
	{
		textures = node->u.pointSet->textures->u.textures;
		ps->texture_dim = 2;
		if( textures->num != NULL && ps->num_points != atoi(textures->num) )
		{
			char buf[256];
			sprintf(buf,"Miss match between number of points %d and specified number of textures %s",
				points->n_p,textures->num);
			node->error_message = strdup(buf);
			node->error = 111;
			return;
		}
		if(textures->n_t != ps->num_points)
		{
			char buf[256];
			sprintf(buf,"Miss match between number of points %d and actual number of textures %d",
				points->n_p,textures->n_t);
			node->error_message = strdup(buf);
			node->error = 112;
			return;
		}
		ps->num_textures = textures->n_t;

		ps->textures = (double *) calloc(sizeof(double),ps->num_textures*ps->texture_dim);

		for(i=0;i<ps->num_textures;++i)
		{
			if( !parse_n_double_coords(textures->t[i]->data,ps->texture_dim,ps->textures + i*ps->texture_dim) )
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("Could not parse texture data %s")+strlen(textures->t[i]->data));
				sprintf(node->error_message,"Could not parse texture data %s",textures->t[i]->data);
				node->error = 113;
				return;
			}
		}
	}
}

void create_faceSet_specific_info(xml_tree *node)
{
	jvx_faceSet *fs;
	struct faces *faces;
	int i;

	node->error = 0;
	fs = (jvx_faceSet *) malloc(sizeof(jvx_faceSet));
	node->app_info = fs;
	if(node->u.faceSet->faces == NULL)
	{
		node->error = 201;
		node->error_message = strdup("No faces in node");
		return;
	}
	faces = node->u.faceSet->faces->u.faces;

	fs->num_faces = faces->n_f;
	if( faces->num != NULL && fs->num_faces != atoi(faces->num) )
	{
		char buf[256];
		sprintf(buf,"Miss match in number of faces actual %d from xml %s",
			faces->n_f,faces->num);
		node->error_message = strdup(buf);
		node->error = 202;
		return;
	}
/*
fprintf(stderr,"Num points %d dim %d\n",ps->num_points,ps->point_dim);
*/
	fs->faces = (jvx_f **) calloc(sizeof(jvx_f *),fs->num_faces);

	for(i=0;i<fs->num_faces;++i)
	{
		if( (fs->faces[i] = parse_f(faces->f[i]->data) ) == NULL )
		{
			node->error_message = (char *) calloc(sizeof(char),
				strlen("Could not parse point data %s")+strlen(faces->f[i]->data));
			sprintf(node->error_message,"Could not parse faces data %s",faces->f[i]->data);
			node->error = 303;
			return;
		}
	}
/*
	for(i=0;i<3;++i)
	{
		fprintf(stderr,"%f %f %f\n",ps->points[i*ps->point_dim],ps->points[i*ps->point_dim+1],ps->points[i*ps->point_dim+2]);
	}
*/
}

void create_lineSet_specific_info(xml_tree *node)
{
	jvx_lineSet *ls;
	struct lines *lines;
	int i;

	node->error = 0;
	ls = (jvx_lineSet *) malloc(sizeof(jvx_lineSet));
	node->app_info = ls;
	if(node->u.lineSet->lines == NULL)
	{
		node->error = 301;
		node->error_message = strdup("No lines in node");
		return;
	}
	lines = node->u.lineSet->lines->u.lines;

	ls->num_lines = lines->n_l;
	if( lines->num != NULL && ls->num_lines != atoi(lines->num) )
	{
		char buf[256];
		sprintf(buf,"Miss match in number of lines actual %d from xml %s",
			lines->n_l,lines->num);
		node->error_message = strdup(buf);
		node->error = 302;
		return;
	}
/*
fprintf(stderr,"Num points %d dim %d\n",ps->num_points,ps->point_dim);
*/
	ls->lines = (int *) malloc(sizeof(int) * ls->num_lines*2);

	for(i=0;i<ls->num_lines;++i)
	{
		if( !parse_n_int_coords(lines->l[i]->data, 2, ls->lines + i*2) )
		{
			node->error_message = (char *) calloc(sizeof(char),
				strlen("Could not parse line data %s")+strlen(lines->l[i]->data));
			sprintf(node->error_message,"Could not parse line data %s",lines->l[i]->data);
			node->error = 310;
			return;
		}
	}
/*
	for(i=0;i<3;++i)
	{
		fprintf(stderr,"%f %f %f\n",ps->points[i*ps->point_dim],ps->points[i*ps->point_dim+1],ps->points[i*ps->point_dim+2]);
	}
*/
}

void create_jvx_specific_info(xml_tree *node)
{
	int i;
	for(i=0;i<node->n_child;++i)
	{
		create_jvx_specific_info(node->children[i]);
	}
	switch(node->type)
	{
	case LSMP_POINTSET:
		create_pointSet_specific_info(node);
		break;
	case LSMP_FACESET:
		create_faceSet_specific_info(node);
		break;
	case LSMP_LINESET:
		create_lineSet_specific_info(node);
		break;
	case LSMP_GEOMETRY:
		node->error = 0; node->error_message = NULL;
		if(node->u.geometry->primitive != NULL)
		{
			if(node->u.geometry->pointSet != NULL)
			{
				node->error_message = strdup("primitive and pointSet in Geometry");
				node->error = 101;
				return;
			}
		}
		else if(node->u.geometry->pointSet == NULL)
		{
			node->error_message = strdup("Null pointSet in Geometry");
			node->error = 102;
			return;
		}
		else if(node->u.geometry->faceSet != NULL)
		{
			if(node->u.geometry->lineSet != NULL)
			{
				node->error_message = strdup("faceSet and lineSet in Geometry");
				node->error = 103;
				return;
			}
		}
		break;
	case LSMP_BNDBOX:
		if(node->n_child != 2
		 || node->children[0]->type != LSMP_P
		 || node->children[1]->type != LSMP_P )
		{
			node->error_message = strdup("Must have two child points");
			node->error = 104;
			return;
		}
		node->error = 0;
		node->error_message = NULL;
		break;
	default:
	}
}		

/*** create nodes from data ****/

void set_jvx_attribute(xml_tree *root,char *name,char *value)
{
	int i;

	for(i=0;i<root->n_attr;i+=2)
	{
		if(!strcmp(root->attr[i],name))
		{
			root->attr[i+1] = strdup(value);
			return;
		}
	}
	if(root->n_attr==0)
	{
		root->n_attr = 2;
		root->attr = (char **) malloc(sizeof(char *)*2);
		root->attr[0] = strdup(name);
		root->attr[1] = strdup(value);
	}
	else
	{
		root->n_attr += 2;
		root->attr = (char **) realloc(root->attr,sizeof(char *)*root->n_attr);
		root->attr[root->n_attr-2] = strdup(name);
		root->attr[root->n_attr-1] = strdup(value);
	}
}

xml_tree *create_lineSet_from_data(int num_lines,int *lines)
{
	xml_tree *node;
	jvx_lineSet *ls;

	node = (xml_tree *) malloc(sizeof(xml_tree));
	node->error = 0;
	node->error_message = NULL;
	node->name = "lineSet";
	node->type = LSMP_LINESET;
	node->n_child = num_lines;
	node->children = NULL;
	node->n_attr = 0;
	node->attr = NULL;
	node->data = NULL;
	node->u.lineSet = (struct lineSet *) malloc(sizeof(struct lineSet));
	node->u.lineSet->colors =NULL;
	node->u.lineSet->normals =NULL;
	node->u.lineSet->lines =NULL;
	
	node->u.lineSet->line = "show";
	node->u.lineSet->startArrow = "hide";
	node->u.lineSet->arrow = "hide";
	node->u.lineSet->normalArrow = "hide";
	node->u.lineSet->color = "hide";

	ls = (jvx_lineSet *) malloc(sizeof(jvx_lineSet));
	node->app_info = ls;
	ls->num_lines = num_lines;
	ls->lines = lines;
	return node;
}

void add_sub_child_to_lineSet(xml_tree *root,xml_tree *child)
{
	if(root->type != LSMP_LINESET)
	{
		report_error2(MIDDLE_ERROR,"Whoopse! add_color_to_lineSet called with a type of %s",root->name,401);
		exit(1);
	}
	if(root->u.lineSet->lines == NULL)
	{
		xml_tree *node;
		node = (xml_tree *) malloc(sizeof(xml_tree));
		node->error = 0;
		node->error_message = NULL;
		node->name = "lines";
		node->type = LSMP_LINES;
		node->n_child = 0;
		node->children = NULL;
		node->n_attr = 0;
		node->attr = NULL;
		node->data = NULL;
		node->u.lines = (struct lines *) malloc(sizeof(struct lines));
		node->u.lines->color=NULL;
		node->u.lines->thickness =NULL;
		node->u.lines->colorTag =NULL;
		node->u.lines->labelAtt = NULL;
		node->u.lines->num = "0";
		root->u.lineSet->lines = node;
	}
	if(child->type == LSMP_COLOR)
	{
		root->u.lineSet->lines->u.lines->color = child;
	}
	else
	{
		report_error2(MIDDLE_ERROR,"Whoopse! add_color_to_lineSet called with a type of %s",root->name,401);
		exit(1);
	}
}

xml_tree *create_jvx_color_from_color_number(int num)
{
	xml_tree *node;
	node = (xml_tree *) malloc(sizeof(xml_tree));
	node->error = 0;
	node->error_message = NULL;
	node->name = "color";
	node->type = LSMP_COLOR;
	node->n_child = 0;
	node->children = NULL;
	node->n_attr = 0;
	node->attr = NULL;
	set_jvx_attribute(node,"type","rgb");
	switch(num)
	{
	case 0:	node->data = strdup("0 0 0"); break; /* Black */
	case 1:	node->data = strdup("255 0 0"); break; /* Red */
	case 2:	node->data = strdup("0 255 0"); break; /* Green */
	case 3:	node->data = strdup("255 255 0"); break; /* Yellow */
	case 4:	node->data = strdup("0 0 255"); break; /* Blue */
	case 5:	node->data = strdup("255 0 255"); break; /* Magenta */
	case 6:	node->data = strdup("0 255 255"); break; /* Cyan */
	case 7:	node->data = strdup("255 255 255"); break; /* White */
	default:node->data = strdup("128 128 128"); break; /* Grey */
	}
	node->u.color = (struct color *) malloc(sizeof(struct color));
	node->u.color->type = "rgb";
	node->app_info = NULL;
	return node;
}

xml_tree *create_pointSet_from_data(int dim,int num_points,double *points)
{
	xml_tree *node;
	jvx_pointSet *ps;
	char	attval[4];

	node = (xml_tree *) malloc(sizeof(xml_tree));
	node->error = 0;
	node->error_message = NULL;
	node->name = "pointSet";
	node->type = LSMP_POINTSET;
	node->n_child = num_points;
	node->children = NULL;
	node->n_attr = 0;
	node->attr = NULL;
		
	sprintf(attval,"%d",dim);
	set_jvx_attribute(node,"dim",attval);
	node->data = NULL;
	node->u.pointSet = (struct pointSet *) malloc(sizeof(struct pointSet));
	node->u.pointSet->points =NULL;
	node->u.pointSet->colors =NULL;
	node->u.pointSet->normals =NULL;
	node->u.pointSet->textures =NULL;
	
	node->u.pointSet->dim = (char *) malloc(sizeof(char)*20);
	sprintf(node->u.pointSet->dim,"%d",dim);
	node->u.pointSet->normal = "hide";
	node->u.pointSet->point = "show";
	node->u.pointSet->normalArrow = "hide";
	node->u.pointSet->color = "hide";

	ps = (jvx_pointSet *) malloc(sizeof(jvx_pointSet));
	node->app_info = ps;
	ps->num_points = num_points;
	ps->point_dim = dim; 
	ps->points = points;
	ps->num_colors = 0; ps->num_normals = 0; ps->num_textures = 0;
	ps->color_dim = 0; ps->normal_dim = 0; ps->texture_dim = 0;
	ps->colors = NULL;
	ps->normals = NULL;
	ps->textures = NULL;
	return node;
}

void add_jvx_child(xml_tree *root,xml_tree *child)
{
	++root->n_child;
	if(root->children ==NULL)
		root->children = (xml_tree **) malloc(sizeof(xml_tree *) * root->n_child );
	else
		root->children = (xml_tree **) realloc((void *) root->children,
					sizeof(xml_tree *) * root->n_child );
	root->children[root->n_child-1] = child;
}

void add_child_to_geometry(xml_tree *root,xml_tree *child)
{
	add_jvx_child(root,child);
	if(root->type != LSMP_GEOMETRY)
	{
		report_error2(MIDDLE_ERROR,"Whoopse! add_child_to_geometry called with a type of %s",root->name,401);
		exit(1);
	}
	if(child->type == LSMP_POINTSET)
	{
		root->u.geometry->pointSet = child;
	}
	else if(child->type == LSMP_LINESET)
	{
		root->u.geometry->lineSet = child;
	}
	else
	{
		report_error2(MIDDLE_ERROR,"Whoopse! add_child_to_geometry called with a child of type of %s",child->name,402);
		exit(1);
	}
}

HPoint3 *get_jvx_points(xml_tree *jvx,int *num_points,int *dim)
{
	xml_tree *geometry;
	jvx_pointSet *ps;
	int dim_p,i;
	HPoint3 *pl;

	geometry = find_child_in_jvx_tree(jvx,"geometry");
	ps = geometry->u.geometry->pointSet->app_info;
	if(ps == NULL)
	{
		report_error(HEAD_ERROR,"Could not find a pointset",601);
		exit(1);
	}
	dim_p = ps->point_dim;

	pl = (HPoint3 *) malloc(sizeof(HPoint3) * ps->num_points);
	for(i=0;i<ps->num_points;++i)
	{
		pl[i].x = *(ps->points + i * dim_p);
		pl[i].y = *(ps->points + i * dim_p+1);
		pl[i].z = *(ps->points + i * dim_p+2);
		if(dim_p == 4)
			pl[i].w = *(ps->points + i * dim_p+3);
		else    pl[i].w = 1.0;
	}
	*num_points = ps->num_points;
	*dim = dim_p;
	return pl;
}

xml_tree *parse_jvx_core()
{
  xml_tree *jvx;
  jvx = stack[0];
  clean_xml_tree(jvx);
/*
  print_brief_xml_tree(stack[0],0);
  jvx = create_jvx_tree(stack[0]);
  print_brief_jvx_tree(stderr,jvx,0,4);
*/
  create_node_specific_info(jvx);
  create_jvx_specific_info(jvx);
  return jvx;
}

xml_tree *fparse_jvx(FILE *fp,int cl,FILE *fl)
{
  char *bufptr;

  XML_Parser p = XML_ParserCreate(NULL);
  if (! p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }

  XML_SetElementHandler(p, start, end);
  XML_SetCharacterDataHandler(p, cdh);

  for (;;) {
    int done;
    int len;

/*
    len = fread(Buff, 1, BUFFSIZE, fp);
*/
    bufptr = fgets(Buff, BUFFSIZE, fp);
    if(bufptr == NULL) break;
	len = strlen(Buff);
    if (ferror(fp)) {
	report_error(HEAD_ERROR,"Error reading input",776);
      exit(-1);
    }
    done = feof(fp);
    cl -= len;
    if(cl==0) done = 1;

/*
fprintf(fl,"%d %d %s",cl,len,Buff);
*/
fprintf(fl,"%s",Buff);

    if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) 
    {
      fprintf(stderr, "Parse error at line %d:\n%s\n",
              XML_GetCurrentLineNumber(p),
              XML_ErrorString(XML_GetErrorCode(p)));
	report_error(HEAD_ERROR,"Error with jvx format",777);
      exit(-1);
    }

    if (done)
      break;
  }
  return parse_jvx_core();
}

xml_tree *parse_jvx(char *in)
{
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
    return parse_jvx_core();
}


int JvxMain(int argc,char **argv)
{
  xml_tree *jvx;
  XML_Parser p = XML_ParserCreate(NULL);
  if (! p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }

  XML_SetElementHandler(p, start, end);
  XML_SetCharacterDataHandler(p, cdh);

  for (;;) {
    int done;
    int len;

    len = fread(Buff, 1, BUFFSIZE, stdin);
    if (ferror(stdin)) {
      fprintf(stderr, "Read error\n");
      exit(-1);
    }
    done = feof(stdin);

    if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
      fprintf(stderr, "Parse error at line %d:\n%s\n",
              XML_GetCurrentLineNumber(p),
              XML_ErrorString(XML_GetErrorCode(p)));
      exit(-1);
    }

    if (done)
      break;
  }
  jvx = stack[0];
  clean_xml_tree(jvx);
/*
  print_brief_xml_tree(stack[0],0);
  jvx = create_jvx_tree(stack[0]);
  print_brief_jvx_tree(stderr,jvx,0,4);
  fprintf(stderr,"create_node_specific_info\n");
*/
  create_node_specific_info(jvx);
  create_jvx_specific_info(jvx);
  print_xml_errors(stderr,jvx);
  print_brief_jvx_tree(stderr,jvx,0,4);

  return(0);
}


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

typedef enum { JVX_DEFAULT, JVX_POINTS, JVX_COLORS, JVX_BACKCOLORS, JVX_NORMALS,
	JVX_LINES, JVX_FACES, JVX_EDGES, JVX_NEIGHBOURS, JVX_VECTORS, JVX_TEXTURES } jvx_type;

typedef union {
	float **p;
	int   **c;
	float **n;
	int   **l;
	int   **f;
	int   **e;
	int   **nb;
	float **v;
	float **t;
} point_data;

typedef struct jvx_tree {
	jvx_type type;
	char *name;

	int num; /* only used for points, normals, ... */
	char color_type[10]; /* only used for color backColor */
	point_data u;	/* only for points, ... */
	int n_points;

	int n_attr,n_child;
	char **attr;
	char *data;
	struct jvx_tree **children;
} jvx_tree;


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
printf("my trim %d %d %d",i,j,len);
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
printf("creat_node %s\n",el);
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
printf("add_child %s %s\n",parent->name,child->name);
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
printf("add_data %s %s\n",parent->name,data);
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

static float *parse_float_coords(char *s)
{
	int i,count=0;
	char *s1;
	float *eles;

	s1 = strdup(s);
	if(strtok(s1," \n\t\r")==NULL)
	{
		free(s1);
		return(NULL);
	}
	++count;
	while(strtok(NULL," \n\t\r")!=NULL) ++count;
	strcpy(s1,s);
	eles = (float *) calloc(sizeof(float),count+1);
	eles[0] = (float) count;
	eles[1] = atof(strtok(s1," \n\t\r"));
	for(i=2;i<count+1;++i)
	{
		eles[i] = atof(strtok(NULL," \n\t\r"));
	}
	free(s1);
	return(eles);
}

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
	for(i=2;i<count+1;++i)
	{
		eles[i] = atoi(strtok(NULL," \n\t\r"));
	}
	free(s1);
	return eles;
}

static jvx_tree *create_jvx_tree(xml_tree *root);

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
			if(!strncmp(s,"p",1) ) ++n_points; else ++n_child; break;	
		case JVX_COLORS:
			if(!strncmp(s,"c",1) ) ++n_points; else ++n_child; break;	
		case JVX_NORMALS:
			if(!strncmp(s,"n",1) ) ++n_points; else ++n_child; break;	
		case JVX_LINES:
			if(!strncmp(s,"l",1) ) ++n_points; else ++n_child; break;	
		case JVX_FACES:
			if(!strncmp(s,"f",1) ) ++n_points; else ++n_child; break;	
		case JVX_EDGES:
			if(!strncmp(s,"e",1) ) ++n_points; else ++n_child; break;	
		case JVX_NEIGHBOURS:
			if(!strncmp(s,"nb",2) ) ++n_points; else ++n_child; break;	
		case JVX_VECTORS:
			if(!strncmp(s,"v",1) ) ++n_points; else ++n_child; break;	
		case JVX_TEXTURES:
			if(!strncmp(s,"t",1) ) ++n_points; else ++n_child; break;
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
		copy->u.p = (float **) calloc(sizeof(float *),n_points); break;
	case JVX_COLORS:
		copy->u.c = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_NORMALS:
		copy->u.n = (float **) calloc(sizeof(float *),n_points); break;
	case JVX_LINES:
		copy->u.l = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_FACES:
		copy->u.f = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_EDGES:
		copy->u.e = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_NEIGHBOURS:
		copy->u.nb = (int **) calloc(sizeof(int *),n_points); break;
	case JVX_VECTORS:
		copy->u.v = (float **) calloc(sizeof(float *),n_points); break;
	case JVX_TEXTURES:
		copy->u.t = (float **) calloc(sizeof(float *),n_points); break;
	}
	n_points = 0; n_child = 0;
	for(i=0;i<root->n_child;++i)
	{
		char *s;
		s = root->children[i]->name;

		switch(copy->type)
		{
		case JVX_POINTS:
			if(!strncmp(s,"p",1) )
				copy->u.p[n_points++] = parse_float_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_COLORS:
			if(!strncmp(s,"c",1) )
				copy->u.c[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_NORMALS:
			if(!strncmp(s,"n",1) )
				copy->u.n[n_points++] = parse_float_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_LINES:
			if(!strncmp(s,"l",1) )
				copy->u.l[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_FACES:
			if(!strncmp(s,"f",1) )
				copy->u.f[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_EDGES:
			if(!strncmp(s,"e",1) )
				copy->u.e[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_NEIGHBOURS:
			if(!strncmp(s,"nb",2) )
				copy->u.nb[n_points++] = parse_int_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_VECTORS:
			if(!strncmp(s,"v",1) )
				copy->u.v[n_points++] = parse_float_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		case JVX_TEXTURES:
			if(!strncmp(s,"t",1) )
				copy->u.t[n_points++] = parse_float_coords(root->children[i]->data);
			else
				copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		default:
			copy->children[n_child++] = create_jvx_tree(root->children[i]);
			break;
		}
	}
}
	
static jvx_tree *create_jvx_tree(xml_tree *root)
{
	jvx_tree *node;
	jvx_type type;
	type = JVX_DEFAULT;
	if(!strcmp(root->name,"points")) type = JVX_POINTS;
	if(!strcmp(root->name,"colors")) type = JVX_COLORS;
	if(!strcmp(root->name,"normals")) type = JVX_NORMALS;
	if(!strcmp(root->name,"lines")) type = JVX_LINES;
	if(!strcmp(root->name,"faces")) type = JVX_FACES;
	if(!strcmp(root->name,"edges")) type = JVX_EDGES;
	if(!strcmp(root->name,"neighbours")) type = JVX_NEIGHBOURS;
	if(!strcmp(root->name,"vectors")) type = JVX_VECTORS;
	if(!strcmp(root->name,"textures")) type = JVX_TEXTURES;

	node = create_jvx_node(root,type);
	jvx_copy_children(node,root);
	return(node);
}

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

static void print_xml_tree(xml_tree *root,int depth)
{
	int i,j;

	for(i=0;i<depth;++i) printf("  ");
	printf("<%s",root->name);
	for(j=0;j<root->n_attr;j+=2)
	{
		printf(" %s=%s",root->attr[j],root->attr[j+1]);
	}
	if(root->n_child==0 && root->data== NULL) printf("/");
	printf(">\n");
	for(j=0;j<root->n_child;++j)
		print_xml_tree(root->children[j],depth+1);

	if(root->data != NULL)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("  %s\n",root->data);
	}
	if(root->n_child!=0 || root->data != NULL)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("</%s>\n",root->name);
	}
	
}

static void print_brief_xml_tree(xml_tree *root,int depth)
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
		for(i=0;i<depth;++i) printf("  ");
		printf("  (repeated %d times)\n",count);
	}
	count=1;
	strcpy(lastname,root->name);

	for(i=0;i<depth;++i) printf("  ");
	printf("<%s",root->name);
	for(j=0;j<root->n_attr;j+=2)
	{
		printf(" %s=%s",root->attr[j],root->attr[j+1]);
	}
	if(root->n_child==0 && root->data==NULL) printf("/");
	printf(">\n");
	for(j=0;j<root->n_child;++j)
		print_brief_xml_tree(root->children[j],depth+1);
	if(count>1)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("  (repeated %d times)\n",count);
	}
	count=1;

	if(root->data != NULL)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("  %s\n",root->data);
	}
	if(root->n_child!=0 || root->data!=NULL)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("</%s>\n",root->name);
	}
	
}

/* limit is max_num_of points to print -1 prints all points */

static void print_brief_jvx_tree(jvx_tree *root,int depth,int limit)
{
	int i,j,k,n;

	for(i=0;i<depth;++i) printf("  ");
	printf("<%s",root->name);
	for(j=0;j<root->n_attr;j+=2)
	{
		printf(" %s=\"%s\"",root->attr[j],root->attr[j+1]);
	}
	if(root->n_child==0 && root->data== NULL && root->n_points == 0) printf("/");
	printf(">\n");

	for(j=0;j<root->n_points;++j)
	{
		if(limit>=0 && j>limit) break;

		switch(root->type)
		{
		case JVX_POINTS:
			n = (int) root->u.p[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <p>");
			for(k=0;k<n;++k)
				printf("%f ",root->u.p[j][k+1]);
			printf("</p>\n");
			break;
		case JVX_COLORS:
			n = (int) root->u.c[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <c>");
			for(k=0;k<n;++k)
				printf("%d ",root->u.c[j][k+1]);
			printf("</c>\n");
			break;
		case JVX_NORMALS:
			n = (int) root->u.n[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <n>");
			for(k=0;k<n;++k)
				printf("%f ",root->u.n[j][k+1]);
			printf("</n>\n");
			break;
		case JVX_LINES:
			n = (int) root->u.l[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <l>");
			for(k=0;k<n;++k)
				printf("%d ",root->u.l[j][k+1]);
			printf("</l>\n");
			break;
		case JVX_FACES:
			n = (int) root->u.f[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <f>");
			for(k=0;k<n;++k)
				printf("%d ",root->u.f[j][k+1]);
			printf("</f>\n");
			break;
		case JVX_EDGES:
			n = (int) root->u.e[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <e>");
			for(k=0;k<n;++k)
				printf("%d ",root->u.e[j][k+1]);
			printf("</e>\n");
			break;
		case JVX_NEIGHBOURS:
			n = (int) root->u.nb[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <nb>");
			for(k=0;k<n;++k)
				printf("%d ",root->u.nb[j][k+1]);
			printf("</nb>\n");
			break;
		case JVX_VECTORS:
			n = (int) root->u.v[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <v>");
			for(k=0;k<n;++k)
				printf("%f ",root->u.v[j][k+1]);
			printf("</v>\n");
			break;
		case JVX_TEXTURES:
			n = (int) root->u.t[j][0];
			for(i=0;i<depth;++i) printf("  ");
			printf("  <t>");
			for(k=0;k<n;++k)
				printf("%f ",root->u.t[j][k+1]);
			printf("</t>\n");
			break;
		}
	}
	if(limit>=0 && root->n_points!=0)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("  %d points total\n",root->n_points);	
	}

	for(j=0;j<root->n_child;++j)
		print_brief_jvx_tree(root->children[j],depth+1,limit);

	if(root->data != NULL)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("  %s\n",root->data);
	}
	if(root->n_child!=0 || root->data != NULL || root->n_points!=0)
	{
		for(i=0;i<depth;++i) printf("  ");
		printf("</%s>\n",root->name);
	}
	
}

static void print_jvx_tree(jvx_tree *root,int depth)
{
	print_brief_jvx_tree(root,depth,-1);
}

/***** Callback functions for xml parser *******/

static void start(void *data, const char *el, const char **attr)
{
  int i;
  xml_tree *node,*parent;

/*
  for (i = 0; i < Depth; i++)
    printf("  ");

  printf("%s", el);

  for (i = 0; attr[i]; i += 2) {
    printf(" %s='%s'", attr[i], attr[i + 1]);
  }
  printf("\n");
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
		printf("  ");
		printf("CD(%s)(%s)\n",buffer,buf2);

		parent = stack[Depth-1];
		add_data(parent,buf2);
	}
*/
}
	
int
main(int argc, char *argv[])
{
  jvx_tree *jvx;

  XML_Parser p = XML_ParserCreate(NULL);
  if (! p) {
    fprintf(stderr, "Couldn't allocate memory for parser\n");
    exit(-1);
  }

  XML_SetElementHandler(p, start, end);
  XML_SetCharacterDataHandler(p, cdh);
/*
  XML_SetDefaultHandler(p, cdh);
*/

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
  clean_xml_tree(stack[0]);
/*
  print_brief_xml_tree(stack[0],0);
*/
  jvx = create_jvx_tree(stack[0]);
  print_brief_jvx_tree(jvx,0,4);
  return 0;
}

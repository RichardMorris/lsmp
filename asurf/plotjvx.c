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
/*	some sub programs to plot the boxes.				*/
/*	The general process is as follows:				*/
/*	for each box {							*/
/*	{   for each solution						*/
/*		if solution already used continue;			*/
/*		plot first face adjacient to solution, update solution  */
/*									*/
/************************************************************************/

extern int vrml_version;
extern int draw_lines;
extern char *global_geomname;

#include <stdio.h>
#include "bern.h"
#include "cells.h"
#include <math.h>
#include "../lsmp.h"

/*
#define ECHO_OUTPUT
*/

void plot_all_facets(box_info *box);
void plot_all_sings(box_info *box,int flag);
void draw_box(box_info *box);
void initoogl();
void rewindoogl();
void finiflush();
void flushoogl();
void finioogl();
void write_ifs_coords(float *pos,float *norm);
void write_ifs_indicies(int indicies[3]);
void write_ifs_index(int index);
void unit3f(float vec[3]);
void bgnfacet();
void endfacet();
void plot_sol(sol_info *sol);
void plot_point(sol_info *sol);
void plot_line(sol_info *sol1,sol_info *sol2);

/* from boxclev.c */
extern void calc_pos_norm_actual(sol_info *sol,double vec[3],double norm[3]);
extern void calc_pos_actual(sol_info *sol,double vec[3]);

extern int global_degen;
extern int global_mode;
extern sol_info **known_sings;	/* The singularities known from external data */
extern int num_known_sings;	/* number of such */

/******************* RMAR GLOBALS ***********************************/

/*
#define PRINT_SOLVEEDGE
#define PLOT_AS_LINES
#define OLD_PLOT
#define BINARY
#define VERBOUSE
#define PRINT_FACET_ERR
#define NORM_ERR
#define PRINT_JOIN_FACET
#define PRINT_FACET
#define SHOW_VERTICES
#define DEBUG_FACET
#define PLOT_POINTS
#define PLOT_CHAINS
#define PRINT_DRAW_BOX
#define PRINT_REFINE
#define PLOT_LINES
*/
#define PLOT_NODE_LINKS
#define PLOT_SINGS

#define VRML
#undef  GRAPHICS
#define FORWARDS 1
#define BACKWARDS 2
#define FOUND_EVERYTHING 2

#define PLOTTEDBIT 8

#define grballoc(node) ( node * ) malloc( sizeof(node) )
#define fabsf(real) (float) fabs((double) real)

extern region_info region;
int facet_vertex_count;		/*** The number of verticies on a facet ***/
int total_face_sol_count;		/*** The number of solutions on faces ***/

/*
char *ifs_coords_file_name = "coordsfile", *vect_file_name = "vectfile";
char *ifs_index_file_name = "indexfile", *ifs_norms_file_name = "normfile";
*/
char vect_file_name[L_tmpnam],ifs_coords_file_name[L_tmpnam],isolated_file_name[L_tmpnam],
	ifs_norms_file_name[L_tmpnam],ifs_index_file_name[L_tmpnam];
FILE *vect_file,*ooglfile,*ifs_coords_file,*ifs_index_file,*ifs_norms_file,
	*isolated_file;

int tri_index[3];	/* holds the indices of the current facet */
int tri_count,total_tri_count;
int vect_point_count = 0;	/* number of points on degenerate lines */
int vect_count = 0;		/* number of line segments */
int isolated_count = 0;		/* number of isolated points */

/************************************************************************/
/*									*/
/*	draws a box.							*/
/*									*/
/************************************************************************/

void plot_all_facets(box_info *box)
{
	facet_info *f1;
	facet_sol *s1;

	for(f1 = box->facets;f1 != NULL;f1=f1->next)
	{
		s1 = f1->sols;
		if(s1 == NULL) continue;
		/* check the facet has at least 3 sols */
		if(s1->next == NULL) continue;
		if(s1->next->next == NULL) continue;

		bgnfacet();
		while(s1 != NULL)
		{
			plot_sol(s1->sol);
			s1 = s1->next;
		}
		endfacet();
	}
}

void plot_all_sings(box_info *box,int flag)
{
	sing_info *sing;

	sing = box->sings;
	while( sing != NULL)
	{
		sol_info *sol;

		sol = sing->sing;
		if( flag == 2 && global_mode == MODE_KNOWN_SING)
		{
			int i;
			for(i=0;i<num_known_sings;++i)
			{
				if( sol->xl == known_sings[i]->xl
				 && sol->yl == known_sings[i]->yl
				 && sol->zl == known_sings[i]->zl
				 && sol->root == known_sings[i]->root
				 && sol->root2 == known_sings[i]->root2
				 && sol->root3 == known_sings[i]->root3 )
					plot_point(sing->sing);
			}
		}
		else if( flag == 0
		 || (sol->dx == 0 && sol->dy == 0 && sol->dz == 0) )
			plot_point(sing->sing);
		sing=sing->next;
	}
	if(box->lfd != NULL)
	{
		plot_all_sings(box->lfd,flag);
		plot_all_sings(box->lfu,flag);
		plot_all_sings(box->lbd,flag);
		plot_all_sings(box->lbu,flag);
		plot_all_sings(box->rfd,flag);
		plot_all_sings(box->rfu,flag);
		plot_all_sings(box->rbd,flag);
		plot_all_sings(box->rbu,flag);
	}
}

/* flag = 0 plot everything, = 1 polt only join sings, = 2 only draw sings with all derivs zero */

void plot_all_node_links(box_info *box,int flag)
{
	node_link_info *node_link;

	node_link = box->node_links;
	while(node_link != NULL)
	{
		if( flag == 0 
		 || ( flag == 1 && 
			node_link->A->sol->type == BOX && node_link->B->sol->type == BOX) 
		 || ( flag == 2 
		      && node_link->A->sol->dx == 0
		      && node_link->A->sol->dy == 0
		      && node_link->A->sol->dz == 0
		      && node_link->B->sol->dx == 0
		      && node_link->B->sol->dy == 0
		      && node_link->B->sol->dz == 0 ) )
			plot_line(node_link->A->sol,node_link->B->sol);

		node_link = node_link->next;
	}

	if(box->lfd != NULL)
	{
		plot_all_node_links(box->lfd,flag);
		plot_all_node_links(box->lfu,flag);
		plot_all_node_links(box->lbd,flag);
		plot_all_node_links(box->lbu,flag);
		plot_all_node_links(box->rfd,flag);
		plot_all_node_links(box->rfu,flag);
		plot_all_node_links(box->rbd,flag);
		plot_all_node_links(box->rbu,flag);
	}
}

/********** Main entry point for routines *****************/

void draw_box(box_info *box)
{
	plot_all_facets(box);

	/* Now draw the node_links */

#ifdef PLOT_NODE_LINKS
	if(draw_lines)
	{
		if(global_degen != -1)
			plot_all_node_links(box,global_degen);
		else
			plot_all_node_links(box,2);
	}
#endif

#ifdef PLOT_SINGS
	if(draw_lines)
	{
		if(global_degen != -1)
			plot_all_sings(box,global_degen);
		else
			plot_all_sings(box,2);
	}
#endif

}

/************************************************************************/
/*									*/
/*	Now some routines for drawing the facets			*/
/*									*/
/************************************************************************/

void initoogl()
{
/*
	FILE mystdout;
	freopen("mystdout.txt","w",stderr);
*/
	tmpnam(vect_file_name);
	tmpnam(isolated_file_name);
	tmpnam(ifs_coords_file_name);
	tmpnam(ifs_norms_file_name);
	tmpnam(ifs_index_file_name);
	if(draw_lines)
	{
	vect_file = fopen(vect_file_name,"wb");
	isolated_file = fopen(isolated_file_name,"wb");
	}
	ifs_coords_file = fopen(ifs_coords_file_name,"wb");
	ifs_norms_file = fopen(ifs_norms_file_name,"wb");
	ifs_index_file = fopen(ifs_index_file_name,"wb");
	ooglfile = stdout;
	total_tri_count = 0;
	total_face_sol_count = 0;
	vect_point_count = 0;
	vect_count = 0;
	isolated_count = 0;
}

/*
 * Function:	rewindoogl
 * Action:	When output goes to standard output and more than
 *		model is produced it is imposible to rewind the standard
 *		output. One way round is to delay the creation of the
 *		ooglfile untill the program has ended. This comand
 *		is called intermediatly between models to clear out
 *		the vect and quad tempory vector files.
 */

void rewindoogl()
{
	fclose(ifs_coords_file);
	fclose(ifs_norms_file);
	fclose(ifs_index_file);
	if(draw_lines)
	{
	fclose(vect_file);
	fclose(isolated_file);
	vect_file = fopen(vect_file_name,"wb");
	isolated_file = fopen(isolated_file_name,"wb");
	}
	ifs_coords_file = fopen(ifs_coords_file_name,"wb");
	ifs_norms_file = fopen(ifs_norms_file_name,"wb");
	ifs_index_file = fopen(ifs_index_file_name,"wb");
	total_tri_count = 0;
	total_face_sol_count = 0;
	vect_point_count = 0;
	vect_count = 0;
	isolated_count = 0;
}

/*
 * Function:	finiflush
 * Action:	convert all the data into oogl and write it to the
		ooglfile. Called when the model is complete 'finioogl'
		or when a flush is required 'flushoogl'
 */


void finiflush()
{
	float x,y,z,vert[3];
	double dx,dy,dz,len;
	int i,num,indicies[3];

	if( total_tri_count > 0 )
	{
if(vrml_version == 1)
{
fprintf(ooglfile,"DEF BackgroundColor Info { string \"0.7 0.7 0.2\" }\n");
fprintf(ooglfile,"Separator {\n");
fprintf(ooglfile,"    ShapeHints {\n");
fprintf(ooglfile,"        creaseAngle 1.5\n");
fprintf(ooglfile,"        vertexOrdering  COUNTERCLOCKWISE\n");
fprintf(ooglfile,"        faceType        CONVEX\n");
fprintf(ooglfile,"    }\n");
fprintf(ooglfile,"    Separator {\n");
fprintf(ooglfile,"        # renderCulling OFF\n");
fprintf(ooglfile,"        Material {\n");
fprintf(ooglfile,"            diffuseColor        1 0 1\n");
fprintf(ooglfile,"        }\n");
fprintf(ooglfile,"        Separator {\n");
fprintf(ooglfile,"            # renderCulling     OFF\n");
fprintf(ooglfile,"            Coordinate3 { point   [\n");
} 
else if(vrml_version == 2)
{
fprintf(ooglfile,"DEF BKG Background\n");
fprintf(ooglfile,"{\n");
fprintf(ooglfile,"    skyColor [ 0.490 0.690 0.000 ]\n");
fprintf(ooglfile,"}\n");
fprintf(ooglfile,"\n");
fprintf(ooglfile,"\n");
fprintf(ooglfile,"Viewpoint {\n");
fprintf(ooglfile,"   position 0 0 8\n");
fprintf(ooglfile,"   orientation 0 1 0 0\n");
fprintf(ooglfile,"   fieldOfView 0.4\n");
fprintf(ooglfile,"}\n");
fprintf(ooglfile,"Viewpoint {\n");
fprintf(ooglfile,"   position 0 0 28\n");
fprintf(ooglfile,"   orientation 0 1 0 0\n");
fprintf(ooglfile,"   fieldOfView 0.4\n");
fprintf(ooglfile,"}\n");
fprintf(ooglfile,"Viewpoint {\n");
fprintf(ooglfile,"   position 0 0 1\n");
fprintf(ooglfile,"   orientation 0 1 0 0\n");
fprintf(ooglfile,"   fieldOfView 0.4\n");
fprintf(ooglfile,"}\n");
fprintf(ooglfile,"\n");
fprintf(ooglfile,"NavigationInfo {\n");
fprintf(ooglfile,"   type \"EXAMINE\"\n");
fprintf(ooglfile,"   speed 5\n");
fprintf(ooglfile,"}\n");
fprintf(ooglfile,"\n");
fprintf(ooglfile,"Transform {\n");
fprintf(ooglfile,"   children [\n");
fprintf(ooglfile,"      DEF CUBE_TRANSFORM Transform {\n");
fprintf(ooglfile,"	 children [\n");
fprintf(ooglfile,"	    Shape {\n");
fprintf(ooglfile,"	       appearance Appearance {\n");
fprintf(ooglfile,"		  material DEF CUBE_COLOR Material {\n");
fprintf(ooglfile,"		     diffuseColor 1.0 0.0 0.0\n");
fprintf(ooglfile,"		  }\n");
fprintf(ooglfile,"	       }\n");
fprintf(ooglfile,"		\n");


fprintf(ooglfile,"    geometry IndexedFaceSet {\n");
fprintf(ooglfile,"	solid FALSE\n");
fprintf(ooglfile,"	normalPerVertex   TRUE\n");
fprintf(ooglfile,"        coord Coordinate { point   [\n");
}
else if(vrml_version == 3)
{
fprintf(ooglfile,"  <geometry name=\"%s\">\n",global_geomname);
#ifdef SHOW_VERTICES
fprintf(ooglfile,"   <pointSet dim=\"3\" point=\"show\">\n");
#else
fprintf(ooglfile,"   <pointSet dim=\"3\" point=\"hide\">\n");
#endif
fprintf(ooglfile,"    <points>\n");
}


		/* First read all the quads in and write them out again */

		fclose(ifs_coords_file);
		ifs_coords_file = fopen(ifs_coords_file_name,"rb");

#ifdef VERBOUSE
		fprintf(stderr,"total number of Quads %d\n",total_quad_count);
#endif

		for(num=0;num<total_face_sol_count;++num)
		{
			fread((char *) vert,sizeof(float),3,ifs_coords_file);
			if(vrml_version == 1 || vrml_version == 2 )
			{
				fprintf(ooglfile,"%f %f %f,\n",
					vert[0],vert[1],vert[2]
					);
			}
			else if(vrml_version == 3)
			{
				fprintf(ooglfile,"<p>%f %f %f</p>\n",
					vert[0],vert[1],vert[2]
					);
#ifdef ECHO_OUTPUT
				fprintf(stderr,"<p>%f %f %f</p>\n",
					vert[0],vert[1],vert[2]
					);
#endif
			}
		}
	fclose(ifs_coords_file);

if(vrml_version==1)
{
fprintf(ooglfile,"]\n");
fprintf(ooglfile,"		}\n");
}
else if(vrml_version==2)
{
fprintf(ooglfile,"]\n");
fprintf(ooglfile,"		}\n");
}
else if(vrml_version==3)
{
fprintf(ooglfile,"   </points>\n");
}

/************* Now the normal information ********/

if(vrml_version==1)
{
fprintf(ooglfile,"        Normal { vector   [\n");
}
else if(vrml_version==2)
{
fprintf(ooglfile,"        normal Normal { vector   [\n");
}
else if(vrml_version==3)
{
fprintf(ooglfile,"   <normals>\n");
}

		fclose(ifs_norms_file);
		ifs_norms_file = fopen(ifs_norms_file_name,"rb");

#ifdef VERBOUSE
		fprintf(stderr,"total number of Quads %d\n",total_quad_count);
#endif

		for(num=0;num<total_face_sol_count;++num)
		{
			fread((char *) vert,sizeof(float),3,ifs_norms_file);
			if(vrml_version == 1 || vrml_version == 2 )
			{
				fprintf(ooglfile,"%f %f %f,\n",
					vert[0],vert[1],vert[2]
					);
			}
			else if(vrml_version == 3)
			{
				dx = (double) vert[0];
				dy = (double) vert[1];
				dz = (double) vert[2];
				len = sqrt(dx*dx+dy*dy+dz*dz);
				if( len != 0.0 )
				{
				dx /= len;
				dy /= len;
				dz /= len;
				}
				fprintf(ooglfile,"<n>%.17f %.17f %.17f</n>\n",dx,dy,dz);
/*
				fprintf(ooglfile,"<n>%f %f %f</n>\n",dx,dy,dz);
*/
			}
		}
	fclose(ifs_norms_file);

if(vrml_version==1)
{
fprintf(ooglfile,"]\n");
fprintf(ooglfile,"		}\n");
}
else if(vrml_version==2)
{
fprintf(ooglfile,"]\n");
fprintf(ooglfile,"		}\n");
}
else if(vrml_version==3)
{
fprintf(ooglfile,"   </normals>\n");
fprintf(ooglfile,"  </pointSet>\n");
}

/******* Now the info for the faces  ************/

if(vrml_version==1)
{
fprintf(ooglfile,"           IndexedFaceSet {\n");
fprintf(ooglfile,"                coordIndex      [\n");
}
else if(vrml_version==2)
{
fprintf(ooglfile,"	coordIndex      [\n");
}
else if(vrml_version==3)
{
#ifdef SHOW_VERTICES
fprintf(ooglfile,"  <faceSet face=\"show\" edge=\"show\">\n");
#else
fprintf(ooglfile,"  <faceSet face=\"show\" edge=\"hide\">\n");
#endif

fprintf(ooglfile,"   <faces>\n");
}


		fclose(ifs_index_file);
		ifs_index_file = fopen(ifs_index_file_name,"rb");
		for(num=0;num<total_tri_count;++num)
		{
			fprintf(ooglfile,"<f>");
			while(1)
			{
				fread((char *) indicies,sizeof(int),1,ifs_index_file);
				if(indicies[0] < 0 ) break;
				fprintf(ooglfile,"%d ",
					indicies[0]);
			}
			fprintf(ooglfile,"</f>\n");
		}
		fclose(ifs_index_file);

if(vrml_version==1)
{
fprintf(ooglfile,"]\n");
fprintf(ooglfile,"           }\n");
fprintf(ooglfile,"        }\n");
fprintf(ooglfile,"    }\n");
fprintf(ooglfile,"}\n");
}
else if(vrml_version==2)
{
fprintf(ooglfile,"]\n");
fprintf(ooglfile,"     ]\n");
fprintf(ooglfile,"    }\n");
fprintf(ooglfile,"      }\n");
fprintf(ooglfile,"     ]\n");
fprintf(ooglfile,"}\n");
fprintf(ooglfile,"]\n");
fprintf(ooglfile,"}\n");
}
else if(vrml_version==3)
{
fprintf(ooglfile,"    </faces>\n");
fprintf(ooglfile,"   </faceSet>\n");
fprintf(ooglfile,"  </geometry>\n");
}

	}
	else
		fprintf(stderr,"No points to output\n");

	if( draw_lines && vect_count > 0 && vrml_version == 3)
	{
fprintf(ooglfile,"  <geometry name=\"%s lines\">\n",global_geomname);
fprintf(ooglfile,"   <pointSet dim=\"3\" point=\"hide\">\n");
fprintf(ooglfile,"    <points>\n");

	/* First we have to read through the temp file to find the
		number of lines */

	fclose(vect_file);
	vect_file = fopen(vect_file_name,"r");

	/* find number of lines and total num of verticies */

	for(i=0;i<vect_point_count;++i)
	{
			fscanf(vect_file,"%f %f %f",&x,&y,&z);
			fprintf(ooglfile,"<p>%f %f %f</p>\n",x,y,z);
	}
fprintf(ooglfile,"    </points>\n");
fprintf(ooglfile,"   </pointSet>\n");
fprintf(ooglfile,"   <lineSet line=\"show\">\n");
fprintf(ooglfile,"    <lines>\n");
	for(i=0;i<vect_count;++i)
	{
		fprintf(ooglfile,"<l>%d %d</l>\n",2*i,2*i+1);
	}
fprintf(ooglfile,"    </lines>\n");
fprintf(ooglfile,"   </lineSet>\n");
fprintf(ooglfile,"  </geometry>\n");
	fclose(vect_file);
	}

	if( draw_lines && isolated_count > 0 && vrml_version == 3)
	{
fprintf(ooglfile,"  <geometry name=\"%s points\">\n",global_geomname);
fprintf(ooglfile,"   <pointSet dim=\"3\" point=\"show\">\n");
fprintf(ooglfile,"    <points>\n");

	/* First we have to read through the temp file to find the
		number of lines */

	fclose(isolated_file);
	isolated_file = fopen(isolated_file_name,"r");

	/* find number of lines and total num of verticies */

	for(i=0;i<isolated_count;++i)
	{
			fscanf(isolated_file,"%f %f %f",&x,&y,&z);
			fprintf(ooglfile,"<p>%f %f %f</p>\n",x,y,z);
	}
fprintf(ooglfile,"    </points>\n");
fprintf(ooglfile,"   </pointSet>\n");
fprintf(ooglfile,"  </geometry>\n");
	fclose(isolated_file);
	}
	fflush(ooglfile);
}


void flushoogl()
{
	finiflush();
	fopen(vect_file_name,"ab");
	unlink(ifs_coords_file_name);
	unlink(ifs_norms_file_name);
	unlink(ifs_index_file_name);
	unlink(vect_file_name);
	unlink(isolated_file_name);
}

void finioogl()
{
	finiflush();
	unlink(vect_file_name);
	unlink(isolated_file_name);
	unlink(ifs_coords_file_name);
	unlink(ifs_norms_file_name);
	unlink(ifs_index_file_name);
}

/*
 * Function:	write_ifs_coord
 * action:	writes out a coordinates
 */

void write_ifs_coords(float *pos,float *norm)
{
		fwrite((void *) pos, sizeof(float),3,ifs_coords_file);
		fwrite((void *) norm, sizeof(float),3,ifs_norms_file);
}

void write_ifs_indicies(int indicies[3])
{
		fwrite((void *) indicies, sizeof(int),3,ifs_index_file);
}

void write_ifs_index(int index)
{
		fwrite((void *) &index, sizeof(int),1,ifs_index_file);
}

/*
 * Function:	quadwrite
 * action:	writes out a quadrilateral, includes averaging normals.
 *		n is number of verticies to write.
 */

void unit3f(float vec[3])
{
	float len;
	len = (float) sqrt((double) vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
	if( len == 0.0 ) return;
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

short unit3frobust(float vec[3])
{
	float len;
	len = (float) sqrt((double) vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
	if( len == 0.0 || len != len ) return 0;
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
	return 1;
}

void bgnfacet()
{
#ifdef PRINT_FACET
	fprintf(stderr,"bgnfacet:\n");
#endif
	facet_vertex_count = 0;
	tri_count = 0;
}

void endfacet()
{
	int indicies[1];
	indicies[0] = -1;
	tri_count = 0;
#ifdef PRINT_FACET
	fprintf(stderr,"endfacet: %d\n",total_tri_count);
#endif
	write_ifs_index(-1);
	++total_tri_count;
}

/********* test using much bigger polygons ********/

void plot_sol(sol_info *sol)
{
	double vec[3],norm[3];
	float  fvec[3],fnorm[3];
	short  col[3];
	static short goodnorm[3];

/*
if( total_tri_count > 10 ) return;
*/
	if(sol == NULL)
	{
		fprintf(stderr,"Error: plot_sol: sol == NULL\n");
		return;
	}

	/* First calculate the position */

	if(sol->plotindex == -2) return;
	
	if(sol->plotindex == -1 )
	{
		calc_pos_norm_actual(sol,vec,norm);
		if(vec[0] != vec[0])
		{
			fprintf(stderr,"NaN in plot_sol\n");
			print_sol(sol);
			sol->plotindex = -2;
			return;
		}
		col[0] = 128;
		col[1] = (short) (256 * vec[2]);
		col[2] = (short) (256 - col[1]);
		fvec[0] = (float) vec[0];
		fvec[1] = (float) vec[1];
		fvec[2] = (float) vec[2];
		fnorm[0] = (float) norm[0];
		fnorm[1] = (float) norm[1];
		fnorm[2] = (float) norm[2];
		goodnorm[tri_count] = unit3frobust(fnorm);

		sol->plotindex = total_face_sol_count++;
		write_ifs_coords(fvec,fnorm);
	}
#ifdef PRINT_FACET
fprintf(stderr,"No %d ",sol->plotindex);
		print_sol(sol);
/*
		fprintf(stderr,"%5.2f %5.2f %5.2f # %5.2f %5.2f %5.2f # %5.2f %5.2f %5.2f\n",
			fvec[0],fvec[1],fvec[2],norm[0],norm[1],norm[2],fnorm[0],fnorm[1],fnorm[2]);
*/
#endif

	write_ifs_index(sol->plotindex);	
	return;
}

void plot_point(sol_info *sol)
{
	double vec[3];

	if(sol == NULL)
	{
		fprintf(stderr,"Error: plot_sol: sol == NULL\n");
		return;
	}

	/* First calculate the position */

	calc_pos_actual(sol,vec);
#ifdef PRINT_FACET
	fprintf(stderr,"point:\n");
	print_sol(sol);
#endif

	fprintf(isolated_file,"%f %f %f\n",vec[0],vec[1],vec[2]);
	++isolated_count;
#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		bgnpoint();
		v3d(vec);
		endpoint();
	}
#endif
}

void plot_line(sol_info *sol1,sol_info *sol2)
{
	double vec[3];

#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		bgnline();
	}
#endif
	if(sol1 == NULL)
	{
		fprintf(stderr,"Error: plot_sol1: sol1 == NULL\n");
		return;
	}

	/* First calculate the position */

	calc_pos_actual(sol1,vec);
	if(vec[0] != vec[0] || vec[1] != vec[1] || vec[2] != vec[2] )
	{
		fprintf(stderr,"bad posn vec %f %f %f\n",
			vec[0],vec[1],vec[2] );
		print_sol(sol1);
	}

	fprintf(vect_file,"%f %f %f ",vec[0],vec[1],vec[2]);
#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		v3d(vec);
	}
#endif

	calc_pos_actual(sol2,vec);
#ifdef PRINT_FACET
	fprintf(stderr,"line: \n");
	print_sol(sol1);
	print_sol(sol2);
#endif

	if(vec[0] != vec[0] || vec[1] != vec[1] || vec[2] != vec[2] )
	{
		fprintf(stderr,"bad posn vec %f %f %f\n",
			vec[0],vec[1],vec[2] );
		print_sol(sol2);
	}

	fprintf(vect_file,"%f %f %f\n",vec[0],vec[1],vec[2]);
	++vect_count;
	vect_point_count += 2;
#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		v3d(vec);
		endline();
	}
#endif
}



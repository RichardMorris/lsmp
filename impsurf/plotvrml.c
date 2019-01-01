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

#include <stdio.h>
#include "bern.h"
#include "cells.h"
#include <math.h>

/******************* RMAR GLOBALS ***********************************/

/*
#define PRINT_SOLVEEDGE
#define PLOT_AS_LINES
#define OLD_PLOT
#define PRINT_DRAW_BOX
#define BINARY
#define VERBOUSE
#define PRINT_FACET_ERR
#define NORM_ERR
#define PLOT_POINTS
#define PLOT_LINES
#define PRINT_FACET
*/
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
char vect_file_name[L_tmpnam],ifs_coords_file_name[L_tmpnam],
	ifs_norms_file_name[L_tmpnam],ifs_index_file_name[L_tmpnam];
FILE *vect_file,*ooglfile,*ifs_coords_file,*ifs_index_file,*ifs_norms_file;

int tri_index[3];	/* holds the indices of the current facet */
int tri_count,total_tri_count;

/************************************************************************/
/*									*/
/*	draws a box.							*/
/*									*/
/************************************************************************/

draw_box(box)
box_info *box;
{
	node_link_info *node_link;
	link_info *link;
	sing_info *sing;
	soltype cycle;
	int finished = FALSE;

#ifdef PRINT_DRAW_BOX
	fprintf(stderr,"\ndraw_box: box (%d,%d,%d)/%d\n",
		box->xl,box->yl,box->zl,box->denom);
#endif

	/*** First find a link to start from. ***/

	link = box->ll->links;
	cycle = FACE_LL;
	do
	{
		if( link == NULL )
		{
			switch( cycle )
			{
			case FACE_LL: cycle = FACE_RR;
				link = box->rr->links; break;
			case FACE_RR: cycle = FACE_FF;
				link = box->ff->links; break;
			case FACE_FF: cycle = FACE_BB;
				link = box->bb->links; break;
			case FACE_BB: cycle = FACE_DD;
				link = box->dd->links; break;
			case FACE_DD: cycle = FACE_UU;
				link = box->uu->links; break;
			case FACE_UU: finished = TRUE; break;
			}
			continue;
		}
		else if( !(link->status & PLOTTEDBIT) )
/*		else if( link->status == LINK )   */
			plot_facet(box,link,cycle);
		
		link = link->next;
	} while( !finished );

	/* Now draw the node_links */

#ifdef PLOT_LINES
	node_link = box->node_links;
	while(node_link != NULL)
	{
		if( node_link->status & PLOTTEDBIT )
			node_link->status &= ~PLOTTEDBIT;
		else
		{
			plot_line(node_link->A->sol,node_link->B->sol);
			node_link->A->sol->status |= PLOTTEDBIT;
			node_link->B->sol->status |= PLOTTEDBIT;
		}
		node_link = node_link->next;
	}
#endif

	/* Now draw the isolated points */

#ifdef PLOT_POINTS
	sing = box->sings;
	while( sing != NULL)
	{
		if(sing->sing->status & PLOTTEDBIT )
			sing->sing->status &= ~PLOTTEDBIT;
		else
		{
			plot_point(sing->sing);
		}
		sing = sing->next;
	}
#endif

	/* Now restore the statuses */

	link = box->ll->links;
	while( link != NULL )
	{	link->status &= ~PLOTTEDBIT; link = link->next; }
	link = box->rr->links;
	while( link != NULL )
	{	link->status &= ~PLOTTEDBIT; link = link->next; }
	link = box->ff->links;
	while( link != NULL )
	{	link->status &= ~PLOTTEDBIT; link = link->next; }
	link = box->bb->links;
	while( link != NULL )
	{	link->status &= ~PLOTTEDBIT; link = link->next; }
	link = box->dd->links;
	while( link != NULL )
	{	link->status &= ~PLOTTEDBIT; link = link->next; }
	link = box->uu->links;
	while( link != NULL )
	{	link->status &= ~PLOTTEDBIT; link = link->next; }
}

/*
 * Function:	plot_facet
 * action:	starting from a link, loop all the way round until
 *		you get back to the begining.
 */

plot_facet(box,startlink,startcycle)
box_info *box;
link_info *startlink;
soltype startcycle;
{
	link_info *link;
	soltype cycle;
	sol_info *startingsol,*presentsol;
	int direction;

	/* Now have a link which has not been plotted */

	link = startlink;
	cycle = startcycle;
	direction = FORWARDS;
	bgnfacet();
	plot_sol(link->A);
	plot_sol(link->B);
	link->status |= PLOTTEDBIT;
	startingsol = link->A;
	presentsol = link->B;

	while( presentsol != startingsol )
	{
		get_next_link(box,&presentsol,&link,&cycle);
		if(presentsol == startingsol ) break;

		plot_sol(presentsol);

		if( link == NULL )	/* just followed a node_link */
		{
			if( direction == BACKWARDS ) break;
			direction = BACKWARDS;

			presentsol->status |= PLOTTEDBIT;
			startingsol = presentsol;
			link = startlink;
			cycle = startcycle;
			endfacet();
			bgnfacet();
			plot_sol(presentsol);
			plot_sol(link->A);
			presentsol = link->A;
		}

	} /* end while loop */
	endfacet();
}

get_next_link(box,presentsol,currentlink,cycle)
box_info *box;
sol_info **presentsol;
link_info **currentlink;
soltype *cycle;
{
	link_info *link;
	node_link_info *node_link;

	if((*presentsol)->type >= FACE_LL && (*presentsol)->type <= FACE_UU )
	{
		/*** A node so now follow node links ***/

		node_link = box->node_links;
		while(node_link != NULL)
		{
			if(node_link->A->sol == *presentsol )
			{
				*presentsol = node_link->B->sol;
				node_link->status |= PLOTTEDBIT;
				break;
			}
			if(node_link->B->sol == *presentsol )
			{
				*presentsol = node_link->A->sol;
				node_link->status |= PLOTTEDBIT;
				break;
			}
			node_link = node_link->next;
		}
		*currentlink = NULL;
	}
	else
	{
		/* A link so loop through all the links starting from... */

		link = (*currentlink)->next;

		do
		{
			if(link == NULL)
			{
				switch( *cycle )
				{
				case FACE_LL: *cycle = FACE_RR;
					link = box->rr->links; break;
				case FACE_RR: *cycle = FACE_FF;
					link = box->ff->links; break;
				case FACE_FF: *cycle = FACE_BB;
					link = box->bb->links; break;
				case FACE_BB: *cycle = FACE_DD;
					link = box->dd->links; break;
				case FACE_DD: *cycle = FACE_UU;
					link = box->uu->links; break;
				case FACE_UU: *cycle = FACE_LL;
					link = box->ll->links; break;
				}
			}

			else if( link == *currentlink )
			{
#ifdef PRINT_FACET_ERR
				fprintf(stderr,"gone all the way round\n");
				printbox(box);
#endif
				*currentlink = NULL;
				break;
			}
				
			else if( link->status & PLOTTEDBIT )
			{
				link = link->next;
			}
			else if(link->A == *presentsol)
			{
				link->status |= PLOTTEDBIT;
				*presentsol = link->B;
				*currentlink = link;
				break;
			}
			else if( link->B == *presentsol )
			{
				link->status |= PLOTTEDBIT;
				*presentsol = link->A;
				*currentlink = link;
				break;
			}
			else
			{
				link = link->next;
			}

		} while(TRUE);  /* end do loop */

	}
}

/************************************************************************/
/*									*/
/*	Now some routines for drawing the facets			*/
/*									*/
/************************************************************************/

initoogl()
{
	tmpnam(vect_file_name);
	tmpnam(ifs_coords_file_name);
	tmpnam(ifs_norms_file_name);
	tmpnam(ifs_index_file_name);
	vect_file = fopen(vect_file_name,"wb");
	ifs_coords_file = fopen(ifs_coords_file_name,"wb");
	ifs_norms_file = fopen(ifs_norms_file_name,"wb");
	ifs_index_file = fopen(ifs_index_file_name,"wb");
	ooglfile = stdout;
	total_tri_count = 0;
	total_face_sol_count = 0;
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

rewindoogl()
{
	fclose(vect_file);
	fclose(ifs_coords_file);
	fclose(ifs_norms_file);
	fclose(ifs_index_file);
	ifs_coords_file = fopen(ifs_coords_file_name,"wb");
	ifs_norms_file = fopen(ifs_norms_file_name,"wb");
	ifs_index_file = fopen(ifs_index_file_name,"wb");
	total_tri_count = 0;
	total_face_sol_count = 0;
}

/*
 * Function:	finiflush
 * Action:	convert all the data into oogl and write it to the
		ooglfile. Called when the model is complete 'finioogl'
		or when a flush is required 'flushoogl'
 */


finiflush()
{
	float x,y,z,vert[3],norm[3];
	double dx,dy,dz,len;
	int nlines,nverticies,num,indicies[3];

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
fprintf(ooglfile,"  <geometry name=\"asurf\">\n");
fprintf(ooglfile,"   <pointSet dim=\"3\" point=\"hide\">\n");
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
				dx /= len;
				dy /= len;
				dz /= len;
				fprintf(ooglfile,"<n>%.17f %.17f %.17f</n>\n",dx,dy,dz
					);
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
fprintf(ooglfile,"  <faceSet face=\"show\" edge=\"hide\">\n");
fprintf(ooglfile,"   <faces>\n");
}


		fclose(ifs_index_file);
		ifs_index_file = fopen(ifs_index_file_name,"rb");
		for(num=0;num<total_tri_count;++num)
		{
			fread((char *) indicies,sizeof(int),3,ifs_index_file);
			if(vrml_version == 1 || vrml_version == 2)
			{
				fprintf(ooglfile,"%d, %d, %d, -1,\n",
					indicies[0],indicies[1],indicies[2]
					);
			}
			else if(vrml_version == 3)
			{
				fprintf(ooglfile,"<f>%d %d %d</f>\n",
					indicies[0],indicies[1],indicies[2]
					);
			}
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

#ifdef VRML_WRITE_POLYLINE
	/* First we have to read through the temp file to find the
		number of lines */

	fclose(vect_file);
	vect_file = fopen(vect_file_name,"rb");

	/* find number of lines and total num of verticies */

	nlines = nverticies = 0;
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		++nlines;
		nverticies += num;
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f %f",&x,&y,&z);
			--num;
		}
	}
	
	if( nverticies != 0 )
	{
	fprintf(ooglfile,"{ = VECT\n");
	fprintf(ooglfile,"%d %d %d\n",nlines,nverticies,0);

	/* find number of verticies in each line */

	rewind(vect_file);
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		fprintf(ooglfile,"%d\n",num);
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f %f",&x,&y,&z);
			--num;
		}
	}

	/* printf the colours for each line */

	for(num=1;num <= nlines; ++num) fprintf(ooglfile,"0 ");
	fprintf(ooglfile,"\n");

	/* print the verticies */
	
	rewind(vect_file);
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f %f",&x,&y,&z);
			fprintf(ooglfile,"%f %f %f\n",x,y,z);
			--num;
		}
	}

	/* now print the colours */

/*
	for(num=1; num <= nlines; ++num) fprintf(ooglfile,"1 1 1 1\n");
*/
	
	fprintf(ooglfile,"}\n");
	}
	fclose(vect_file);
#endif
	fflush(ooglfile);
}


flushoogl()
{
	finiflush();
	fopen(vect_file_name,"ab");
	unlink(ifs_coords_file_name);
	unlink(ifs_norms_file_name);
}

finioogl()
{
	finiflush();
	unlink(vect_file_name);
	unlink(ifs_coords_file_name);
	unlink(ifs_norms_file_name);
	unlink(ifs_index_file_name);
}

/*
 * Function:	write_ifs_coord
 * action:	writes out a coordinates
 */

write_ifs_coords(float *pos,float *norm)
{
		fwrite((void *) pos, sizeof(float),3,ifs_coords_file);
		fwrite((void *) norm, sizeof(float),3,ifs_norms_file);
}

write_ifs_indicies(int indicies[3])
{
		fwrite((void *) indicies, sizeof(int),3,ifs_index_file);
}

/*
 * Function:	quadwrite
 * action:	writes out a quadrilateral, includes averaging normals.
 *		n is number of verticies to write.
 */

unit3f(vec)
float vec[3];
{
	float len;
	len = (float) sqrt((double) vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
	if( len == 0.0 ) return;
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

short unit3frobust(vec)
float vec[3];
{
	float len;
	len = (float) sqrt((double) vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
	if( len == 0.0 ) return 0;
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
	return 1;
}

bgnfacet()
{
#ifdef PRINT_FACET
	fprintf(stderr,"bgnfacet:\n");
#endif
	facet_vertex_count = 0;
	tri_count = 0;
#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		bgntmesh();
	}
#endif
}

endfacet()
{
	tri_count = 0;
#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		endtmesh();
	}
#endif
#ifdef PRINT_FACET
	fprintf(stderr,"endfacet:\n");
#endif
}

plot_sol(sol)
sol_info *sol;
{
	double vec[3],norm[3];
	float  fvec[3],fnorm[3],facetnorm[3],dot0,dot1,dot2;
	short  col[3];
	static short goodnorm[3];
	static float tri_vec[3][3],tri_norm[3][3];
	float  x1,y1,z1,x2,y2,z2,det;
	int    tmp_ind;
/*
if( total_tri_count > 10 ) return;
*/
	if(sol == NULL)
	{
		fprintf(stderr,"Error: plot_sol: sol == NULL\n");
		return;
	}

	/* First calculate the position */


	calc_pos_norm(sol,vec,norm);

	col[0] = 128;
	col[1] = (short) (256 * vec[2]);
	col[2] = (short) (256 - col[1]);
	fvec[0] = (float) (region.xmin + (region.xmax-region.xmin) * vec[0]);
	fvec[1] = (float) (region.ymin + (region.ymax-region.ymin) * vec[1]);
	fvec[2] = (float) (region.zmin + (region.zmax-region.zmin) * vec[2]);
	fnorm[0] = (float) norm[0];
	fnorm[1] = (float) norm[1];
	fnorm[2] = (float) norm[2];
	goodnorm[tri_count] = unit3frobust(fnorm);

#ifdef PRINT_FACET
		print_sol(sol);
		fprintf(stderr,"%5.2f %5.2f %5.2f # %5.2f %5.2f %5.2f # %5.2f %5.2f %5.2f\n",
			fvec[0],fvec[1],fvec[2],norm[0],norm[1],norm[2],fnorm[0],fnorm[1],fnorm[2]);
/*
*/
#endif

	if(sol->plotindex == -1 && goodnorm[tri_count])
	{
		sol->plotindex = total_face_sol_count++;
#ifdef GRAPHICS
		if(global_graphics_mode)
		{
			winset(global_graphics_wind);
			if( ++facet_vertex_count > 2 )
				swaptmesh();
			c3s(col);
			v3d(vec);
		}
#endif
		write_ifs_coords(fvec,fnorm);
	}


	tri_vec[tri_count][0] = fvec[0];
	tri_vec[tri_count][1] = fvec[1];
	tri_vec[tri_count][2] = fvec[2];
	tri_norm[tri_count][0] = fnorm[0];
	tri_norm[tri_count][1] = fnorm[1];
	tri_norm[tri_count][2] = fnorm[2];
	tri_index[tri_count] = sol->plotindex;
	++tri_count;
	if(tri_count < 3) return;


	if(goodnorm[0] && goodnorm[1] && goodnorm[2])
	{
		/* work out if we are going positively or negatively round the normal */

#ifdef NOT_DEF
	dot0 =    tri_norm[0][0] * tri_norm[1][0]
		+ tri_norm[0][1] * tri_norm[1][1]
		+ tri_norm[0][2] * tri_norm[1][2] ;
	dot1 =    tri_norm[1][0] * tri_norm[2][0]
		+ tri_norm[1][1] * tri_norm[2][1]
		+ tri_norm[1][2] * tri_norm[2][2] ;
	dot2 =    tri_norm[2][0] * tri_norm[0][0]
		+ tri_norm[2][1] * tri_norm[0][1]
		+ tri_norm[2][2] * tri_norm[0][2] ;
		
	if( ! (
		 ( dot0 <= 0.0 && dot1 <= 0.0 && dot2 <= 0.0 ) 
	      || ( dot0 >= 0.0 && dot1 >= 0.0 && dot2 >= 0.0 ) ) )
	{
		fprintf(stderr,"Error in good facet dots %5.2f %5.2f %5.2f\n",
			dot0,dot1,dot2);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[0][0],tri_vec[0][1],tri_vec[0][2],tri_norm[0][0],tri_norm[0][1],tri_norm[0][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[1][0],tri_vec[1][1],tri_vec[0][2],tri_norm[1][0],tri_norm[1][1],tri_norm[1][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[2][0],tri_vec[2][1],tri_vec[2][2],tri_norm[2][0],tri_norm[2][1],tri_norm[2][2]);
	}
		x1 = tri_vec[1][0] - tri_vec[0][0];
		y1 = tri_vec[1][1] - tri_vec[0][1];
		z1 = tri_vec[1][2] - tri_vec[0][2];
		x2 = tri_vec[2][0] - tri_vec[0][0];
		y2 = tri_vec[2][1] - tri_vec[0][1];
		z2 = tri_vec[2][2] - tri_vec[0][2];
		facetnorm[0] = y1 * z2 - y2 * z1;
		facetnorm[1] = z1 * x2 - z2 * x1;
		facetnorm[2] = x1 * y2 - x2 * y1;
		unit3f(facetnorm);
	dot0 =    tri_norm[0][0] * facetnorm[0]
		+ tri_norm[0][1] * facetnorm[1]
		+ tri_norm[0][2] * facetnorm[2] ;
	dot1 =    tri_norm[1][0] * facetnorm[0]
		+ tri_norm[1][1] * facetnorm[1]
		+ tri_norm[1][2] * facetnorm[2] ;
	dot2 =    tri_norm[2][0] * facetnorm[0]
		+ tri_norm[2][1] * facetnorm[1]
		+ tri_norm[2][2] * facetnorm[2] ;

	if( ! (
		 ( dot0 < 0.0 && dot1 < 0.0 && dot2 < 0.0 ) 
	      || ( dot0 > 0.0 && dot1 > 0.0 && dot2 > 0.0 ) ) )
	{
		fprintf(stderr,"Error in good facet dots %5.2f %5.2f %5.2f\n",
			dot0,dot1,dot2);
		fprintf(stderr,"facetnorm %6.3f %6.3f %6.3f\n",
			facetnorm[0],facetnorm[1],facetnorm[2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[0][0],tri_vec[0][1],tri_vec[0][2],tri_norm[0][0],tri_norm[0][1],tri_norm[0][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[1][0],tri_vec[1][1],tri_vec[0][2],tri_norm[1][0],tri_norm[1][1],tri_norm[1][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[2][0],tri_vec[2][1],tri_vec[2][2],tri_norm[2][0],tri_norm[2][1],tri_norm[2][2]);
	}
#endif

	      	x1 = tri_vec[1][0] - tri_vec[0][0];
		y1 = tri_vec[1][1] - tri_vec[0][1];
		z1 = tri_vec[1][2] - tri_vec[0][2];
		x2 = tri_vec[2][0] - tri_vec[0][0];
		y2 = tri_vec[2][1] - tri_vec[0][1];
		z2 = tri_vec[2][2] - tri_vec[0][2];
		
		det = x1 * y2 * fnorm[2] + x2 * fnorm[1] * z1 + fnorm[0] * y1 * z2
			- fnorm[0] * y2 * z1 - x2 * y1 * fnorm[2] - x1 * fnorm[1] * z2;
		if( det < 0.0 )
		{ 
			tmp_ind = tri_index[1];
			tri_index[1] = tri_index[2];
			tri_index[2] = tmp_ind;
			write_ifs_indicies(tri_index);
			tri_count = 2;
		}
		else
		{ 
			write_ifs_indicies(tri_index);
			tri_count = 2;
			tri_index[1] = tri_index[2];
		}
		++total_tri_count;
		goodnorm[1] = goodnorm[2];
		tri_vec[1][0] = tri_vec[2][0];
		tri_vec[1][1] = tri_vec[2][1];
		tri_vec[1][2] = tri_vec[2][2];
		tri_norm[1][0] = tri_norm[2][0];
		tri_norm[1][1] = tri_norm[2][1];
		tri_norm[1][2] = tri_norm[2][2];
		return;
	}

#ifdef NOTDEF
fprintf(stderr,"Bad tri\n");
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[0][0],tri_vec[0][1],tri_vec[0][2],tri_norm[0][0],tri_norm[0][1],tri_norm[0][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[1][0],tri_vec[1][1],tri_vec[0][2],tri_norm[1][0],tri_norm[1][1],tri_norm[1][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[2][0],tri_vec[2][1],tri_vec[2][2],tri_norm[2][0],tri_norm[2][1],tri_norm[2][2]);
#endif

	if(!goodnorm[0])
	{
		/* work out normal from edges */
		
		x1 = tri_vec[1][0] - tri_vec[0][0];
		y1 = tri_vec[1][1] - tri_vec[0][1];
		z1 = tri_vec[1][2] - tri_vec[0][2];
		x2 = tri_vec[2][0] - tri_vec[0][0];
		y2 = tri_vec[2][1] - tri_vec[0][1];
		z2 = tri_vec[2][2] - tri_vec[0][2];
		tri_norm[0][0] = y1 * z2 - y2 * z1;
		tri_norm[0][1] = z1 * x2 - z2 * x1;
		tri_norm[0][2] = x1 * y2 - x2 * y1;
		unit3f(tri_norm[0]);
	}
	if(!goodnorm[1])
	{
		/* work out normal from edges */
		
		x1 = tri_vec[2][0] - tri_vec[1][0];
		y1 = tri_vec[2][1] - tri_vec[1][1];
		z1 = tri_vec[2][2] - tri_vec[1][2];
		x2 = tri_vec[0][0] - tri_vec[1][0];
		y2 = tri_vec[0][1] - tri_vec[1][1];
		z2 = tri_vec[0][2] - tri_vec[1][2];
		tri_norm[1][0] = y1 * z2 - y2 * z1;
		tri_norm[1][1] = z1 * x2 - z2 * x1;
		tri_norm[1][2] = x1 * y2 - x2 * y1;
		unit3f(tri_norm[1]);
	}
	if(!goodnorm[2])
	{
		/* work out normal from edges */
		
		x1 = tri_vec[0][0] - tri_vec[2][0];
		y1 = tri_vec[0][1] - tri_vec[2][1];
		z1 = tri_vec[0][2] - tri_vec[2][2];
		x2 = tri_vec[1][0] - tri_vec[2][0];
		y2 = tri_vec[1][1] - tri_vec[2][1];
		z2 = tri_vec[1][2] - tri_vec[2][2];
		tri_norm[2][0] = y1 * z2 - y2 * z1;
		tri_norm[2][1] = z1 * x2 - z2 * x1;
		tri_norm[2][2] = x1 * y2 - x2 * y1;
		unit3f(tri_norm[2]);
	}

	/* find the standard normal for the facet. If there is a goodnorm use that */

	if(goodnorm[0])
	{
		facetnorm[0] = tri_norm[0][0];
		facetnorm[1] = tri_norm[0][1];
		facetnorm[2] = tri_norm[0][2];
	}
	else if(goodnorm[1])
	{
		facetnorm[0] = tri_norm[1][0];
		facetnorm[1] = tri_norm[1][1];
		facetnorm[2] = tri_norm[1][2];
	}
	else
	{
		facetnorm[0] = tri_norm[2][0];
		facetnorm[1] = tri_norm[2][1];
		facetnorm[2] = tri_norm[2][2];
	}


	/* now correct normals so in same dir as facetnorm */

	dot0 =    tri_norm[0][0] * facetnorm[0]
		+ tri_norm[0][1] * facetnorm[1]
		+ tri_norm[0][2] * facetnorm[2] ;
	dot1 =    tri_norm[1][0] * facetnorm[0]
		+ tri_norm[1][1] * facetnorm[1]
		+ tri_norm[1][2] * facetnorm[2] ;
	dot2 =    tri_norm[2][0] * facetnorm[0]
		+ tri_norm[2][1] * facetnorm[1]
		+ tri_norm[2][2] * facetnorm[2] ;

	if( dot0 < 0.0 )
	{
		if(goodnorm[0]) 
		{
			fprintf(stderr,"Error with normals on facet");
		}
		else
		{
			tri_norm[0][0] = - tri_norm[0][0];
			tri_norm[0][1] = - tri_norm[0][1];
			tri_norm[0][2] = - tri_norm[0][2];
		}
	}
	if( dot1 < 0.0 )
	{
		if(goodnorm[1]) 
		{
			fprintf(stderr,"Error with normals on facet");
		}
		else
		{
			tri_norm[1][0] = - tri_norm[1][0];
			tri_norm[1][1] = - tri_norm[1][1];
			tri_norm[1][2] = - tri_norm[1][2];
		}
	}
	if( dot2 < 0.0 )
	{
		if(goodnorm[2]) 
		{
			fprintf(stderr,"Error with normals on facet");
		}
		else
		{
			tri_norm[2][0] = - tri_norm[2][0];
			tri_norm[2][1] = - tri_norm[2][1];
			tri_norm[2][2] = - tri_norm[2][2];
		}
	}


#ifdef NOTDEF
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[0][0],tri_vec[0][1],tri_vec[0][2],tri_norm[0][0],tri_norm[0][1],tri_norm[0][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[1][0],tri_vec[1][1],tri_vec[0][2],tri_norm[1][0],tri_norm[1][1],tri_norm[1][2]);
fprintf(stderr,"%6.3f %6.3f %6.3f -- %6.3f %6.3f %6.3f\n",
	tri_vec[2][0],tri_vec[2][1],tri_vec[2][2],tri_norm[2][0],tri_norm[2][1],tri_norm[2][2]);
#endif

	/* normals now point in right directions, write new points */

	if(!goodnorm[0])
	{
		write_ifs_coords(tri_vec[0],tri_norm[0]);
		tri_index[0] = total_face_sol_count++;
	}
	if(!goodnorm[1])
	{
		write_ifs_coords(tri_vec[1],tri_norm[1]);
		tri_index[1] = total_face_sol_count++;
	}
	if(!goodnorm[2])
	{
		write_ifs_coords(tri_vec[2],tri_norm[2]);
		tri_index[2] = total_face_sol_count++;
	}

	/* now work out orientation */						

	x1 = tri_vec[1][0] - tri_vec[0][0];
	y1 = tri_vec[1][1] - tri_vec[0][1];
	z1 = tri_vec[1][2] - tri_vec[0][2];
	x2 = tri_vec[2][0] - tri_vec[0][0];
	y2 = tri_vec[2][1] - tri_vec[0][1];
	z2 = tri_vec[2][2] - tri_vec[0][2];

	det = x1 * y2 * facetnorm[2] + x2 * facetnorm[1] * z1 + facetnorm[0] * y1 * z2
		- facetnorm[0] * y2 * z1 - x2 * y1 * facetnorm[2] - x1 * facetnorm[1] * z2;
	if( det < 0.0 )
	{ 
		tmp_ind = tri_index[1];
		tri_index[1] = tri_index[2];
		tri_index[2] = tmp_ind;
		write_ifs_indicies(tri_index);
	}
	else
	{ 
		write_ifs_indicies(tri_index);
		tri_index[1] = tri_index[2];
	}
	++total_tri_count;
	tri_count = 2;
	goodnorm[1] = goodnorm[2];
	tri_vec[1][0] = tri_vec[2][0];
	tri_vec[1][1] = tri_vec[2][1];
	tri_vec[1][2] = tri_vec[2][2];
	tri_norm[1][0] = tri_norm[2][0];
	tri_norm[1][1] = tri_norm[2][1];
	tri_norm[1][2] = tri_norm[2][2];
	return;
}

plot_point(sol)
sol_info *sol;
{
	double vec[3];

	if(sol == NULL)
	{
		fprintf(stderr,"Error: plot_sol: sol == NULL\n");
		return;
	}

	/* First calculate the position */

	calc_pos(sol,vec);
#ifdef PRINT_FACET
	fprintf(stderr,"point:\n");
	print_sol(sol);
#endif

	vec[0] = region.xmin + (region.xmax-region.xmin) * vec[0];
	vec[1] = region.ymin + (region.ymax-region.ymin) * vec[1];
	vec[2] = region.zmin + (region.zmax-region.zmin) * vec[2];

	fprintf(vect_file,"1 %f %f %f\n",vec[0],vec[1],vec[2]);
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

plot_line(sol1,sol2)
sol_info *sol1,*sol2;
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

	calc_pos(sol1,vec);
	if(vec[0] != vec[0] || vec[1] != vec[1] || vec[2] != vec[2] )
	{
		fprintf(stderr,"bad posn vec %f %f %f\n",
			vec[0],vec[1],vec[2] );
		print_sol(sol1);
	}
	vec[0] = region.xmin + (region.xmax-region.xmin) * vec[0];
	vec[1] = region.ymin + (region.ymax-region.ymin) * vec[1];
	vec[2] = region.zmin + (region.zmax-region.zmin) * vec[2];

	fprintf(vect_file,"2 %f %f %f ",vec[0],vec[1],vec[2]);
#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		v3d(vec);
	}
#endif

	calc_pos(sol2,vec);
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
	vec[0] = region.xmin + (region.xmax-region.xmin) * vec[0];
	vec[1] = region.ymin + (region.ymax-region.ymin) * vec[1];
	vec[2] = region.zmin + (region.zmax-region.zmin) * vec[2];

	fprintf(vect_file,"%f %f %f\n",vec[0],vec[1],vec[2]);
#ifdef GRAPHICS
	if(global_graphics_mode)
	{
		winset(global_graphics_wind);
		v3d(vec);
		endline();
	}
#endif
}



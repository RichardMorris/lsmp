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

#include <stdio.h>
#include "bern.h"
#include "cells.h"
#include <math.h>
extern int vrml_version;

/******************* RMAR GLOBALS ***********************************/

/*
#define PRINT_SOLVEEDGE
#define PLOT_AS_LINES
#define OLD_PLOT
#define PRINT_DRAW_BOX
#define PRINT_FACET
#define BINARY
#define VERBOUSE
#define NORM_ERR
*/
#define NO_COLS
#define PLOT_POINTS
#define PLOT_LINES
#undef  GRAPHICS
#define FORWARDS 1
#define BACKWARDS 2
#define FOUND_EVERYTHING 2

#define PLOTTEDBIT 8

#define grballoc(node) ( node * ) malloc( sizeof(node) )
#define fabsf(real) (float) fabs((double) real)

extern region_info region;
int facet_vertex_count;		/*** The number of verticies on a facet ***/
int	global_colour;

/*
char *quad_file_name = "quadfile", *vect_file_name = "vectfile";
*/
char quad_file_name[L_tmpnam],vect_file_name[L_tmpnam];
FILE *vect_file,*ooglfile,*quad_file;
float quadnorm[4][3];
float quadvect[4][3];
int quad_count,total_quad_count;

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
#ifdef PRINT_FACET
				fprintf(stderr,"gone all the way round\n");
#endif
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

initoogl(int colour)
{
	tmpnam(vect_file_name);
	tmpnam(quad_file_name);
	vect_file = fopen(vect_file_name,"w");
	quad_file = fopen(quad_file_name,"w");
	ooglfile = stdout;
	total_quad_count = 0;
	global_colour = colour;
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
	fclose(quad_file);
	vect_file = fopen(vect_file_name,"w");
	quad_file = fopen(quad_file_name,"w");
	total_quad_count = 0;
}

/*
 * Function:	finiflush
 * Action:	convert all the data into oogl and write it to the
		ooglfile. Called when the model is complete 'finioogl'
		or when a flush is required 'flushoogl'
 */

finiflushoogl()
{
	float x,y,z,vert[3],norm[3];
	int nlines,nverticies,ncols,num;

	if( total_quad_count > 0 )
	{
#ifdef BINARY
		fprintf(ooglfile,"LIST\n{ = NQUAD BINARY\n");
#else
		fprintf(ooglfile,"LIST\n{ = NQUAD\n");
#endif
		/* First read all the quads in and write them out again */

		fclose(quad_file);
		quad_file = fopen(quad_file_name,"r");

#ifdef VERBOUSE
		fprintf(stderr,"total number of Quads %d\n",total_quad_count);
#endif

#ifdef BINARY
		fwrite((char *) &total_quad_count,sizeof(int),1,ooglfile);
#endif
		for(num=0;num<total_quad_count*4;++num)
		{
			fread((char *) vert,sizeof(float),3,quad_file);
			fread((char *) norm,sizeof(float),3,quad_file);
#ifdef BINARY
			fwrite((char *) vert,sizeof(float),3,ooglfile);
			fwrite((char *) norm,sizeof(float),3,ooglfile);
#else
			fprintf(ooglfile,"%f %f %f %f %f %f\n",
				vert[0],vert[1],vert[2],
				norm[0],norm[1],norm[2]);
			if(num %4 == 3 ) fprintf(ooglfile,"\n");
#endif
		}
		fprintf(ooglfile,"}\n");
	}


	/* First we have to read through the temp file to find the
		number of lines */

	fclose(vect_file);
	vect_file = fopen(vect_file_name,"r");

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
	
        if( global_colour == -1 ) ncols = 0;
        else                      ncols = nlines;

	if( nverticies != 0 )
	{
	fprintf(ooglfile,"{ = VECT\n");
	fprintf(ooglfile,"%d %d %d\n",nlines,nverticies,ncols);

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

	/* printf the number of colours for each line */

        if( ncols > 0 )
                for(num=0;num < nlines; ++num) fprintf(ooglfile,"1\n");
        else
                for(num=0;num < nlines; ++num) fprintf(ooglfile,"0\n");
	fprintf(ooglfile,"\n# The verticies\n");

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

        for(num=1; num <= ncols; ++num)
                switch(global_colour)
                {
                case 0: /* Black   */ fprintf(ooglfile,"0 0 0 1\n"); break;
                case 1: /* Red     */ fprintf(ooglfile,"1 0 0 1\n"); break;
                case 2: /* Green   */ fprintf(ooglfile,"0 1 0 1\n"); break;
                case 3: /* Yellow  */ fprintf(ooglfile,"1 1 0 1\n"); break;
                case 4: /* Blue    */ fprintf(ooglfile,"0 0 1 1\n"); break;
                case 5: /* Magenta */ fprintf(ooglfile,"1 0 1 1\n"); break;
                case 6: /* Cyan    */ fprintf(ooglfile,"0 1 1 1\n"); break;
                case 7: /* White   */ fprintf(ooglfile,"1 1 1 1\n"); break;
                }
	
	fprintf(ooglfile,"}\n");
	}
	fflush(ooglfile);
	fclose(vect_file);
	fclose(quad_file);
}

finiflushjvx()
{
	float x,y,z,vert[3],norm[3];
	int nlines,nverticies,ncols,num;
	int i;
	char str[80];

	fprintf(ooglfile,"  <geometry name=\"acurve3\">\n");
	fprintf(ooglfile,"   <pointSet dim=\"3\" point=\"hide\">\n");
	fprintf(ooglfile,"    <points>\n");

	
	/* First we have to read through the temp file to find the
		number of lines */

	fclose(vect_file);
	vect_file = fopen(vect_file_name,"r");

	/* find number of lines and total num of verticies */

	nlines = nverticies = 0;
	while( fscanf(vect_file,"%d ",&num) != EOF )
	{
		if(num == 2) 
		{
			++nlines;
			fscanf(vect_file,"%f %f %f ",&x,&y,&z);
				fprintf(ooglfile,"<p>%f %f %f</p>\n",
					x,y,z
					); 
			fscanf(vect_file,"%f %f %f ",&x,&y,&z);
				fprintf(ooglfile,"<p>%f %f %f</p>\n",
					x,y,z
					);
		}
		else if (num == 1)
		{
			 nverticies += num;
	 		 fscanf(vect_file,"%f %f %f ",&x,&y,&z);
		}
	}
	fprintf(ooglfile,"   </points>\n");
	fprintf(ooglfile,"  </pointsSet>\n");
	fprintf(ooglfile,"  <lineSet lines=\"show\">\n");
	fprintf(ooglfile,"   <lines>\n");
	for(i=0;i<nlines;++i)
	fprintf(ooglfile,"<l>%d %d</l>\n",i*2,i*2+1);
	fprintf(ooglfile,"   </lines>\n");
	fprintf(ooglfile,"  </lineSet>\n");
	fprintf(ooglfile,"  </geometry>\n");
	
	rewind(vect_file);


	if( nverticies != 0 )
	{
	fprintf(ooglfile,"  <geometry name=\"acurve3 points\">\n");
	fprintf(ooglfile,"   <pointSet dim=\"3\" point=\"show\">\n");
	fprintf(ooglfile,"    <points>\n");

	/* find number of verticies in each line */

	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		if( num == 1 )
		{
			fscanf(vect_file,"%f %f %f",&x,&y,&z);
				fprintf(ooglfile,"<p>%f %f %f</p>\n",
					x,y,z
					);
		}
		else if(num == 2)
		{
			fscanf(vect_file,"%f %f %f",&x,&y,&z);
			fscanf(vect_file,"%f %f %f",&x,&y,&z);
		}
	}
	fprintf(ooglfile,"    </points>\n");
	fprintf(ooglfile,"   </pointSet>\n");
	fprintf(ooglfile,"  </geometry>\n");
	}

	/* print the verticies */
	
	fflush(ooglfile);
	fclose(vect_file);
	fclose(quad_file);
}

finiflush()
{
	if(vrml_version == 0)
		finiflushoogl();
	else if (vrml_version == 3)
		finiflushjvx();
}

flushoogl()
{
	finiflush();
	fopen(vect_file_name,"a");
	fopen(quad_file_name,"a");
}

finioogl()
{
	finiflush();
	unlink(vect_file_name);
	unlink(quad_file_name);
}

/*
 * Function:	quadwrite
 * action:	writes out a quadrilateral, includes averaging normals.
 *		n is number of verticies to write.
 */

quadwrite(n)
int n;
{
	float goodnorm[3];
	int i;

	if(n != 4 && n != 3 ) return;
	for(i=0;i<n;++i)
	{
		if(quadnorm[i][0] != 0.0 && quadnorm[i][1] != 0.0 &&
			quadnorm[i][2] != 0.0 )
		{
			goodnorm[0] = quadnorm[i][0];
			goodnorm[1] = quadnorm[i][1];
			goodnorm[2] = quadnorm[i][2];
			break;
		}
	}
	if(i==n)
        {
#ifdef NORM_ERR
		fprintf(stderr,"quadwrite: couldn't find a good normal\n");
#endif
		goodnorm[0] = (quadvect[1][1]-quadvect[0][1]) 
				* (quadvect[2][2] - quadvect[0][2] )
			     -(quadvect[1][2]-quadvect[0][2]) 
				* (quadvect[2][1] - quadvect[0][1] );
		goodnorm[1] = (quadvect[1][2]-quadvect[0][2]) 
				* (quadvect[2][0] - quadvect[0][0] )
			     -(quadvect[1][0]-quadvect[0][0]) 
				* (quadvect[2][2] - quadvect[0][2] );
		goodnorm[2] = (quadvect[1][0]-quadvect[0][0]) 
				* (quadvect[2][1] - quadvect[0][1] )
			     -(quadvect[1][1]-quadvect[0][1]) 
				* (quadvect[2][0] - quadvect[0][0] );
	}

	unit3f(goodnorm);

	for(i=0;i<n;++i)
	{
#ifdef PRINT_FACET
		fprintf(stderr,"%f %f %f  ",quadvect[i][0],quadvect[i][1],quadvect[i][2]);
#endif
		fwrite((void *) quadvect[i], sizeof(float),3,quad_file);
		if( (quadnorm[i][0] == 0.0 && quadnorm[i][1] == 0.0 &&
			quadnorm[i][2] == 0.0 ) ||
		    (quadnorm[i][0] == 0.0 && quadnorm[i][1] == 0.0 &&
			fabsf(quadnorm[i][2]) <= 0.01 ) ||
		    (quadnorm[i][0] == 0.0 && fabsf(quadnorm[i][1]) <= 0.01 &&
			quadnorm[i][2] == 0.0 ) ||
		    (fabsf(quadnorm[i][0]) <= 0.01 && quadnorm[i][1] == 0.0 &&
			quadnorm[i][2] == 0.0 ) )
		{
#ifdef NORM_ERR
			fprintf(stderr,"%f %f %f\n",goodnorm[0],goodnorm[1],goodnorm[2]);
#endif
			fwrite((void *) goodnorm,sizeof(float),3,quad_file);
		}
		else
		{
			unit3f(quadnorm[i]);
#ifdef PRINT_FACET
			fprintf(stderr,"%f %f %f\n",quadnorm[i][0],quadnorm[i][1],quadnorm[i][2]);
#endif
			fwrite((void *) quadnorm[i],sizeof(float),3,quad_file);
		}
	}
	if(n == 3)
	{
#ifdef PRINT_FACET
		fprintf(stderr,"%f %f %f  ",quadvect[2][0],quadvect[2][1],quadvect[2][2]);
#endif
		fwrite((void *) quadvect[2], sizeof(float),3,quad_file);
		if( (quadnorm[2][0] == 0.0 && quadnorm[2][1] == 0.0 &&
			quadnorm[2][2] == 0.0 ) ||
		    (quadnorm[2][0] == 0.0 && quadnorm[2][1] == 0.0 &&
			fabsf(quadnorm[2][2]) <= 0.01 ) ||
		    (quadnorm[2][0] == 0.0 && fabsf(quadnorm[2][1]) <= 0.01 &&
			quadnorm[2][2] == 0.0 ) ||
		    (fabsf(quadnorm[2][0]) <= 0.01 && quadnorm[2][1] == 0.0 &&
			quadnorm[2][2] == 0.0 ) )
		{
#ifdef PRINT_FACET
			fprintf(stderr,"%f %f %f\n",goodnorm[0],goodnorm[1],goodnorm[2]);
#endif
			fwrite((void *) goodnorm,sizeof(float),3,quad_file);
		}
		else
		{
#ifdef PRINT_FACET
			fprintf(stderr,"%f %f %f\n",quadnorm[2][0],quadnorm[2][1],quadnorm[2][2]);
#endif
			fwrite((void *) quadnorm[2],sizeof(float),3,quad_file);
		}
	}
#ifdef PRINT_FACET
	fprintf(stderr,"\n");
#endif
}

bgnfacet()
{
#ifdef PRINT_FACET
	fprintf(stderr,"bgnfacet:\n");
#endif
	facet_vertex_count = 0;
	quad_count = 0;
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
	if(quad_count == 3 || quad_count == 4)
	{
		quadwrite(quad_count);
		++total_quad_count;
	}
	quad_count = 0;
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
	short  col[3];

	if(sol == NULL)
	{
		fprintf(stderr,"Error: plot_sol: sol == NULL\n");
		return;
	}

	/* First calculate the position */

	calc_pos_norm(sol,vec,norm);
#ifdef PRINT_FACET
	print_sol(sol);
#endif

	col[0] = 128;
	col[1] = (short) (256 * vec[2]);
	col[2] = (short) (256 - col[1]);
	vec[0] = region.xmin + (region.xmax-region.xmin) * vec[0];
	vec[1] = region.ymin + (region.ymax-region.ymin) * vec[1];
	vec[2] = region.zmin + (region.zmax-region.zmin) * vec[2];

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

	/* now write the quad file */

	quadvect[quad_count][0] = (float) vec[0];
	quadvect[quad_count][1] = (float) vec[1];
	quadvect[quad_count][2] = (float) vec[2];
	quadnorm[quad_count][0] = (float) norm[0];
	quadnorm[quad_count][1] = (float) norm[1];
	quadnorm[quad_count][2] = (float) norm[2];
	++quad_count;
	if(quad_count == 4)
	{
		quadwrite(4);

		/* 1st vertex unchanged, 2nd vertex = 4th vertex */

		quadvect[1][0] = quadvect[3][0];
		quadvect[1][1] = quadvect[3][1];
		quadvect[1][2] = quadvect[3][2];
		quadnorm[1][0] = quadnorm[3][0];
		quadnorm[1][1] = quadnorm[3][1];
		quadnorm[1][2] = quadnorm[3][2];
		quad_count = 2;
		++total_quad_count;
	}
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

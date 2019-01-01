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

/******************* RMAR GLOBALS ***********************************/

/*
#define PRINT_SOLVEEDGE
#define PLOT_AS_LINES
#define OLD_PLOT
#define PRINT_DRAW_BOX
#define BINARY
#define VERBOUSE
#define NORM_ERR
#define PRINT_FACET
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

char vect_file_name[L_tmpnam];
FILE *vect_file,*ooglfile;

/************************************************************************/
/*									*/
/*	draws a box.							*/
/*									*/
/************************************************************************/

draw_hyper(hyper)
hyper_info *hyper;
{
	node_link_info *node_link;
	sing_info *sing;

#ifdef PRINT_DRAW_BOX
	fprintf(stderr,"\ndraw_hyper: hyper (%d,%d,%d,%d)/%d\n",
		hyper->xl,hyper->yl,hyper->zl,hyper->wl,hyper->denom);
#endif

	/* Now draw the node_links */

#ifdef PLOT_LINES
	node_link = hyper->node_links;
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
	sing = hyper->sings;
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

}

/************************************************************************/
/*									*/
/*	Now some routines for drawing the facets			*/
/*									*/
/************************************************************************/

initoogl()
{
	tmpnam(vect_file_name);
	vect_file = fopen(vect_file_name,"w");
	ooglfile = stdout;
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
	vect_file = fopen(vect_file_name,"w");
}

/*
 * Function:	finiflush
 * Action:	convert all the data into oogl and write it to the
		ooglfile. Called when the model is complete 'finioogl'
		or when a flush is required 'flushoogl'
 */

finiflush()
{
	float x,y,z,w;
	int nlines,nverticies,num;

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
			fscanf(vect_file,"%f %f %f %f",&x,&y,&z,&w);
			--num;
		}
	}
	
	if( nverticies != 0 )
	{
	fprintf(ooglfile,"{ = 4VECT\n");
#ifdef NO_COLS
	fprintf(ooglfile,"%d %d %d\n",nlines,nverticies,0);
#else
	fprintf(ooglfile,"%d %d %d\n",nlines,nverticies,nlines);
#endif

	/* find number of verticies in each line */

	rewind(vect_file);
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		fprintf(ooglfile,"%d\n",num);
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f %f %f",&x,&y,&z,&w);
			--num;
		}
	}

	/* printf the number of colours for each line */

#ifdef NO_COLS
	for(num=1;num <= nlines; ++num) fprintf(ooglfile,"0 ");
#else
	for(num=1;num <= nlines; ++num) fprintf(ooglfile,"1 ");
#endif
	fprintf(ooglfile,"\n");

	/* print the verticies */
	
	rewind(vect_file);
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f %f %f",&x,&y,&z,&w);
			fprintf(ooglfile,"%f %f %f %f\n",x,y,z,w);
			--num;
		}
	}

	/* now print the colours */

#ifndef NO_COLS
	for(num=1; num <= nlines; ++num) fprintf(ooglfile,"1 1 1 1\n");
#endif
	
	fprintf(ooglfile,"}\n");
	}
	fflush(ooglfile);
	fclose(vect_file);
}

flushoogl()
{
	finiflush();
	fopen(vect_file_name,"a");
}

finioogl()
{
	finiflush();
	unlink(vect_file_name);
}

plot_point(sol)
sol_info *sol;
{
	double vec[4];

	if(sol == NULL)
	{
		fprintf(stderr,"Error: plot_sol: sol == NULL\n");
		return;
	}

	/* First calculate the position */

	calc_pos(sol,vec);
#ifdef PRINT_FACET
	fprintf(stderr,"1 %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3]);
#endif

	vec[0] = region.xmin + (region.xmax-region.xmin) * vec[0];
	vec[1] = region.ymin + (region.ymax-region.ymin) * vec[1];
	vec[2] = region.zmin + (region.zmax-region.zmin) * vec[2];
	vec[3] = region.wmin + (region.wmax-region.wmin) * vec[3];

	fprintf(vect_file,"1 %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3]);
}

plot_line(sol1,sol2)
sol_info *sol1,*sol2;
{
	double vec[4];

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
	vec[3] = region.wmin + (region.wmax-region.wmin) * vec[3];

	fprintf(vect_file,"2 %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3]);

	calc_pos(sol2,vec);
#ifdef PRINT_FACET
	fprintf(stderr,"2 %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3]);
#endif

	vec[0] = region.xmin + (region.xmax-region.xmin) * vec[0];
	vec[1] = region.ymin + (region.ymax-region.ymin) * vec[1];
	vec[2] = region.zmin + (region.zmax-region.zmin) * vec[2];
	vec[3] = region.wmin + (region.wmax-region.wmin) * vec[3];

	fprintf(vect_file,"%f %f %f %f\n",vec[0],vec[1],vec[2],vec[3]);
#ifdef PRINT_FACET
	fprintf(stderr,"  %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3]);
	print_sol(sol1);
	print_sol(sol2);
#endif
}

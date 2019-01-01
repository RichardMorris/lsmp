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
#define BINARY
#define VERBOUSE
#define PRINT_FACET_ERR
#define NORM_ERR
#define SHOW_VERTICES
#define PRINT_REFINE
#define PLOT_NODE_LINKS
#define PRINT_DRAW_BOX
#define DEBUG_FACET
#define PRINT_JOIN_FACET
#define PRINT_FACET
*/

#define VRML
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

typedef struct facet_sol
{
	sol_info *sol;
	struct facet_sol *next;
} facet_sol;

typedef struct facet
{
	facet_sol *sols;
	struct facet *next;
	int	numsing;
} facet;

facet *all_facets = NULL;

add_sol_to_facet(facet *f,sol_info *s)
{
	facet_sol *fs;

	fs = (facet_sol *) malloc(sizeof(facet_sol));
	fs->sol = s;
	fs->next = f->sols;
	f->sols = fs;
}

add_sol_to_facet_backwards(facet *f,sol_info *s)
{
	facet_sol *fs,*fs1;

	fs = (facet_sol *) malloc(sizeof(facet_sol));
	fs->sol = s;
	fs->next = NULL;
	if(f->sols == NULL)
	{
		f->sols = fs;
		return;
	}
	/* find end of list */
	for(fs1=f->sols;fs1->next!=NULL;fs1=fs1->next) {}
	fs1->next = fs;
}

facet *add_facet()
{
	facet *ele;
	ele = (facet *) malloc(sizeof(facet));
	ele->next = all_facets;
	ele->sols = NULL;
	all_facets = ele;
	return(ele);
}

remove_facet(facet *f1)
{
	facet *f2;
	facet_sol *fs1,*fs2;

	f2=all_facets;
	while(f2!=NULL)
	{
		if(f2->next == f1)
		{
			f2->next = f1->next;
			fs1=f1->sols;
			while(fs1!=NULL)
			{
				fs2 = fs1->next;
				free(fs1);
				fs1 = fs2;
			}
			free(f1);
			f1 = f2;
			break;
		}
		f2 = f2->next;
	}
}

free_facets()
{
	facet *f1,*f2;
	facet_sol *s1,*s2;

	f1 = all_facets;
	while(f1 != NULL)
	{
		f2 = f1->next;
		s1 = f1->sols;
		while(s1 != NULL)
		{
			s2 = s1->next;
			free(s1);
			s1 = s2;
		}
		free(f1);
		f1 = f2;
	}
}

plot_all_facets()
{
	facet *f1;
	facet_sol *s1;

	for(f1 = all_facets;f1 != NULL;f1=f1->next)
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

print_facet(facet *f1)
{
	facet_sol *s1;

		s1 = f1->sols;
		if(s1 == NULL)
		{
			fprintf(stderr,"Empty facet\n");
			return;
		}
		fprintf(stderr,"bgnfacet\n");
		while(s1 != NULL)
		{
			print_sol(s1->sol);
			s1 = s1->next;
		}
		fprintf(stderr,"endfacet\n");
}

print_all_facets()
{
	facet *f1;

	f1 = all_facets;
	while(f1 != NULL)
	{
		print_facet(f1);
		f1 = f1->next;
	}
}

facet_sol *sol_on_facet(facet *f,sol_info *s)
{
	facet_sol *s1;

	s1 = f->sols;
	while(s1 != NULL)
	{
		if(s1->sol == s) return s1;
		s1 = s1->next;
	}
	return NULL;
}

int first_sol_on_facet(facet *f,facet_sol *fs1,facet_sol *fs2)
{
	facet_sol *s1;

	s1 = f->sols;
	while(s1 != NULL)
	{
		if(s1 == fs1) return 1;
		if(s1 == fs2) return 0;
		s1 = s1->next;
	}
	fprintf(stderr,"Error: facet_sol not found on facet");
	return 0;
}

split_facet_on_chains(box_info *box,facet *f1)
{
	facet_sol *fs1,*fs2,*fs3,*fs4;
	facet	*f2,*f3;
	sol_info *singsol,*facesol,*chainsol1,*chainsol2;
	sing_info *sing;
	chain_info *chain,*chain2;
	float chain_length;
	int i,flag;

#ifdef PRINT_REFINE
fprintf(stderr,"split_facet_on_chain:\n");
print_facet(f1);
#endif

	/* First split on repeated vertices */

	for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
	{
		for(fs2=fs1->next,fs3=fs1;fs2!=NULL;fs3=fs2,fs2=fs2->next)
		{
			if(fs1->sol == fs2->sol)
			{
				f2 = add_facet();
				fs4 = fs1->next;
				fs1->next = fs2->next;
				if(fs2 != fs4)
					fs2->next = fs4;
				if(fs3!=fs1)
					fs3->next = NULL;
				f2->sols = fs2;	
#ifdef PRINT_REFINE
fprintf(stderr,"duplicate sols in facet\n");
print_sol(fs1->sol);
#endif
				split_facet_on_chains(box,f1);
				split_facet_on_chains(box,f2);
				return;
			}
		}
	}

	for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
	{
		if(fs1->sol->type < FACE_LL || fs1->sol->type > FACE_UU ) continue;

		for(fs2=fs1->next;fs2!=NULL;fs2=fs2->next)
		{
			if(fs2->sol->type < FACE_LL || fs2->sol->type > FACE_UU ) continue;

			chain_length = 100.0;
			chain2 = NULL;
			for(chain=box->chains;chain!=NULL;chain=chain->next)
			{
				chainsol1 = chain->sols[0];
				chainsol2 = chain->sols[chain->length-1];
				if( fs1->sol != chainsol1 && fs1->sol != chainsol2 ) continue;
				if( fs2->sol != chainsol1 && fs2->sol != chainsol2 ) continue;
			
				if(chain->metric_length < chain_length)
				{
					chain_length = chain->metric_length;
					chain2 = chain;
				}
				if(chain->metric_length == chain_length && chain2->used)
				{
					chain2 = chain;
				}
			}
			if(chain2 == NULL) continue;

			/* need to check that this chain is not already included as 
				segments round the facet */

			flag = 1;
			for(fs3=fs1->next,i=1;fs3!=fs2;fs3=fs3->next,++i)
			{
				if(fs1->sol == chain2->sols[0] )
				{
					if(fs3->sol != chain2->sols[i] )
					{
						flag = 0;
						break;
					}
				}
				else
				{
					if(fs3->sol != chain2->sols[chain2->length-i-1] )
					{
						flag = 0;
						break;
					}
				}
			}
			if(flag) continue;
			/* could possible be the the other way round */

			flag = 1;
			for(fs3=fs2->next,i=1;fs3!=NULL;fs3=fs3->next,++i)
			{
				if(fs2->sol == chain2->sols[0] )
				{
					if(fs3->sol != chain2->sols[i] )
					{
						flag = 0;
						break;
					}
				}
				else
				{
					if(fs3->sol != chain2->sols[chain2->length-i-1] )
					{
						flag = 0;
						break;
					}
				}
			}
			for(fs3=f1->sols;fs3!=fs1;fs3=fs3->next,++i)
			{
				if(fs2->sol == chain2->sols[0] )
				{
					if(fs3->sol != chain2->sols[i] )
					{
						flag = 0;
						break;
					}
				}
				else
				{
					if(fs3->sol != chain2->sols[chain2->length-i-1] )
					{
						flag = 0;
						break;
					}
				}
			}
			if(flag) continue;
/*
			if(chain2->used) continue;
*/
			chain2->used =1;

			chainsol1 = chain2->sols[0];
			chainsol2 = chain2->sols[chain2->length-1];
#ifdef PRINT_REFINE
fprintf(stderr,"Split on chain\n");
printchain(chain2);
#endif
			f2 = add_facet();
			f3 = add_facet();
		
			for(fs3=fs1;fs3!=fs2;fs3=fs3->next)
			{
				add_sol_to_facet(f2,fs3->sol);
			}
			add_sol_to_facet(f2,fs2->sol);

			for(fs3=f1->sols;fs3!=fs1;fs3=fs3->next)
			{
				add_sol_to_facet(f3,fs3->sol);
			}
			add_sol_to_facet(f3,fs1->sol);

			if(chainsol1 == fs2->sol && chainsol2 == fs1->sol)
			{
				for(i=1;i<chain2->length-1;++i)
				{
					add_sol_to_facet(f2,chain2->sols[i]);
					add_sol_to_facet(f3,chain2->sols[chain2->length-1-i]);
				}
			}
			else if(chainsol1 == fs1->sol && chainsol2 == fs2->sol)
			{
				for(i=chain2->length-2;i>0;--i)
				{
					add_sol_to_facet(f2,chain2->sols[i]);
					add_sol_to_facet(f3,chain2->sols[chain2->length-1-i]);
				}
			}
			else
			{
fprintf(stderr,"Funny stuff happening with the chain sols\n");
			}

			for(fs3=fs2;fs3!=NULL;fs3=fs3->next)
			{
				add_sol_to_facet(f3,fs3->sol);
			}

			split_facet_on_chains(box,f2);
			split_facet_on_chains(box,f3);
			/* now have to remove f1 for list of facets */

			remove_facet(f1);
			return;
		}
	}						
}

join_facets_by_chains(box_info *box,facet *f1,facet *f2)
{
	facet_sol *fs1,*fs2,*fs3=NULL,*fs4=NULL,*fs5;
	chain_info *chain,*chain2,*chain3;
	int flag = 0,count1,count2,count3,i;
	facet *f3,*f4;
	float chain_length;
	sol_info *chainsol1,*chainsol2;

#ifdef PRINT_REFINE
fprintf(stderr,"Join facets\n");
print_facet(f1);
print_facet(f2);
#endif
	chain3 = NULL;
	for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
	{
		for(fs2=f2->sols;fs2!=NULL;fs2=fs2->next)
		{
			if(fs1->sol == fs2->sol) return; /* don't join touching facets */
chain_length = 100.0;
			chain2 = NULL;
			for(chain=box->chains;chain!=NULL;chain=chain->next)
			{
				chainsol1 = chain->sols[0];
				chainsol2 = chain->sols[chain->length-1];
				if( fs1->sol != chainsol1 && fs1->sol != chainsol2 ) continue;
				if( fs2->sol != chainsol1 && fs2->sol != chainsol2 ) continue;
			
				if(chain->metric_length < chain_length)
				{
					chain_length = chain->metric_length;
					chain2 = chain;
				}
			}
			if(chain2 == NULL) continue; /* didn't find a linking chain */
			if(chain2->used) continue;
			if(fs1 == fs3 || fs2 == fs4) continue;	/* ensure that start and end sols not the same */

			if(chain3!=NULL)
			{
				int flag2,j;

				/* have two chains ensure they have no vertices in common */

				flag2 = 0;
				for(i=0;i<chain2->length;++i)
					for(j=0;j<chain3->length;++j)
					{
						if(chain2->sols[i] == chain3->sols[j]) flag2 = 1;
					}
				if(flag2) continue;
			}

			if(flag++) break;

			/* now got the first chain */

			fs3 = fs1; fs4 = fs2;
			chain3 = chain2;
		}
		if(flag>=2) break;
	}
	if(flag!=2) return;

#ifdef PRINT_REFINE
fprintf(stderr,"Found two linking chains\n");
printchain(chain2);
fprintf(stderr,"and\n");
printchain(chain3);
#endif
	if(fs1 == fs3 || fs2 == fs4)
	{
		fprintf(stderr,"two of the linking facet sols arethe same\n");
		return;
	}
	chain2->used = 1;
	chain3->used = 1;

	f3 = add_facet();
	f4 = add_facet();
	if(chain3->sols[0] == fs3->sol)
	{
		for(i=chain3->length-1;i>=0;--i)
		{
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 1 f3"); print_sol(chain3->sols[i]);
#endif
			add_sol_to_facet(f3,chain3->sols[i]);
		}
	}
	else
	{
		for(i=0;i<chain3->length;++i)
		{
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 2 f3"); print_sol(chain3->sols[i]);
#endif
			add_sol_to_facet(f3,chain3->sols[i]);
		}
	}
	for(fs5 = fs3->next;fs5!=NULL;fs5=fs5->next)
	{
		if(fs5==fs1) break;
		add_sol_to_facet(f3,fs5->sol);
#ifdef PRINT_JOIN_FACET
		fprintf(stderr,"add_sol pt 3 f3"); print_sol(fs5->sol);
#endif
	}
	if(chain2->sols[0] == fs1->sol)
	{
		for(i=0;i<chain2->length;++i)
		{
			add_sol_to_facet(f3,chain2->sols[i]);
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 4 f3"); print_sol(chain2->sols[i]);
#endif
		}
		for(i=chain2->length-1;i>=0;--i)
		{
			add_sol_to_facet(f4,chain2->sols[i]);
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 5 f4"); print_sol(chain2->sols[i]);
#endif
		}
	}
	else
	{
		for(i=0;i<chain2->length;++i)
		{
			add_sol_to_facet(f4,chain2->sols[i]);
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 6 f4"); print_sol(chain2->sols[i]);
#endif
		}
		for(i=chain2->length-1;i>=0;--i)
		{
			add_sol_to_facet(f3,chain2->sols[i]);
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 7 f3"); print_sol(chain2->sols[i]);
#endif
		}
	}
	for(fs5=fs1->next;fs5!=NULL;fs5=fs5->next)
	{
		add_sol_to_facet(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
		fprintf(stderr,"add_sol pt 8 f4"); print_sol(fs5->sol);
#endif
	}
	for(fs5=f1->sols;fs5!=fs3;fs5=fs5->next)
	{
		add_sol_to_facet(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
		fprintf(stderr,"add_sol pt 9 f4"); print_sol(fs5->sol);
#endif
	}
	if(chain3->sols[0] == fs3->sol)
	{
		for(i=0;i<chain3->length;++i)
		{
			add_sol_to_facet(f4,chain3->sols[i]);
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 10 f4"); print_sol(chain3->sols[i]);
#endif
		}
	}
	else
	{
		for(i=chain3->length-1;i>=0;--i)
		{
			add_sol_to_facet(f4,chain3->sols[i]);
#ifdef PRINT_JOIN_FACET
			fprintf(stderr,"add_sol pt 11 f4"); print_sol(chain3->sols[i]);
#endif
		}
	}

	/* now count round the second facet to see which is the shortest linking path */

	count1 = 0; count2 = 0; count3 = 0;
	for(fs5=f2->sols;fs5!=NULL;fs5=fs5->next)
	{
		++count3;
		if(fs5 == fs2) count1 = count3;
		if(fs5 == fs4) count2 = count3;
	}
#ifdef PRINT_JOIN_FACET
fprintf(stderr,"count1 %d %d %d\n",count1,count2,count3);
#endif
	if(count1 < count2 )
	{
		if( (count2 - count1) < count3 / 2)
		{
			/* add to f3 in the direction of the list */

			for(fs5=fs2->next;fs5!=fs4;fs5=fs5->next)
			{
				add_sol_to_facet(f3,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 12 f3"); print_sol(fs5->sol);
#endif
			}

			/* add backwards */
			for(fs5=fs4->next;fs5!=NULL;fs5=fs5->next)
			{
				add_sol_to_facet(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 13 f4b"); print_sol(fs5->sol);
#endif
			}

			for(fs5=f2->sols;fs5!=fs2;fs5=fs5->next)
			{
				add_sol_to_facet(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 14 f4b"); print_sol(fs5->sol);
#endif
			}

		}
		else
		{
			for(fs5=fs4->next;fs5!=NULL;fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f3,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 15 f3b"); print_sol(fs5->sol);
#endif
			}

			for(fs5=f2->sols;fs5!=fs2;fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f3,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 16 f3b"); print_sol(fs5->sol);
#endif
			}

			for(fs5=fs2->next;fs5!=fs4;fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 17 f4b"); print_sol(fs5->sol);
#endif
			}
		}
	}
	else
	{
		if( (count2 - count1) < count3 / 2)
		{
			for(fs5=fs4->next;fs5!=fs2;fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f3,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 18 f3b"); print_sol(fs5->sol);
#endif
			}

			for(fs5=fs2->next;fs5!=NULL;fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 19 f4"); print_sol(fs5->sol);
#endif
			}

			for(fs5=f2->sols;fs5!=fs4;fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 20 f4"); print_sol(fs5->sol);
#endif
			}
		}
		else
		{
			for(fs5=fs2->next;fs5!=NULL;fs5=fs5->next)
			{
				add_sol_to_facet(f3,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 21 f3"); print_sol(fs5->sol);
#endif
			}

			for(fs5=f2->sols;fs5!=fs4;fs5=fs5->next)
			{
				add_sol_to_facet(f3,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 22 f3"); print_sol(fs5->sol);
#endif
			}

			for(fs5=fs4->next;fs5!=fs2;fs5=fs5->next)
			{
				add_sol_to_facet(f4,fs5->sol);
#ifdef PRINT_JOIN_FACET
				fprintf(stderr,"add_sol pt 23 f4b"); print_sol(fs5->sol);
#endif
			}
		}
	}

	remove_facet(f1);
	remove_facet(f2);
}

refine_facets(box_info *box)
{
	facet_sol *fs1,*fs2,*fs3,*fs4;
	facet	*f1,*f2,*f3,*f4;
	chain_info *chain,*chain2;
	sol_info *chainsol1,*chainsol2;

	/* May have a facet with repeated vertices
		if so split the facet */

#ifdef PRINT_REFINE
		print_all_facets();
#endif

	/* Now join together facets linked by chains */

	for(f1=all_facets;f1!=NULL;f1=f3)
	{
		f3 = f1->next;
		for(f2=f1->next;f2!=NULL;f2=f4)
		{
			f4 = f2->next;
			join_facets_by_chains(box,f1,f2);
			break;
		}
		if(f3==f2 && f3 != NULL) f3 = f4;
	}
		
#ifdef PRINT_REFINE
		fprintf(stderr,"After joining\n");
		print_all_facets();
#endif

	for(f1=all_facets;f1!=NULL;f1=f1->next)
	{
		for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
		{
			for(fs2=fs1->next,fs3=fs1;fs2!=NULL;fs3=fs2,fs2=fs2->next)
			{
				if(fs1->sol == fs2->sol)
				{
fprintf(stderr,"Split on sol\n");
print_sol(fs1->sol);
					f2 = add_facet();
					fs4 = fs1->next;
					fs1->next = fs2->next;
					if(fs2 != fs4)
						fs2->next = fs4;
					if(fs3!=fs1)
						fs3->next = NULL;
					f2->sols = fs2;	

					/* Now any chain which happens to start at fs1
						should be duplicated */
/*
					for(chain=box->chains;chain!=NULL;chain=chain->next)
					{
						chainsol1 = chain->sols[0];
						chainsol2 = chain->sols[chain->length-1];
						if( fs1->sol == chainsol1 || fs1->sol == chainsol2 )
						{
							chain2 = (chain_info *) malloc(sizeof(chain_info));
							chain2->next = chain->next;
							chain->next = chain2;
							chain2->length = chain->length;
							chain2->metric_length = chain->metric_length;
							chain2->used = 0;
							chain2->sols = (sol_info **) malloc(sizeof(sol_info *)*chain->length);
							bcopy(chain->sols,chain2->sols,sizeof(sol_info *)*chain->length);
							chain = chain2;
						}
					}								
*/
				}
			}
		}
	}


#ifdef PRINT_REFINE
		fprintf(stderr,"After split\n");
		print_all_facets();
/*
		printchains(box->chains);
*/
#endif


	/* Now split facets where two of the nodes are linked by a chain */

	for(f1=all_facets;f1!=NULL;f1=f2)
	{
		f2 = f1->next; /* cant guarentee that f1 will still exist after splitting */

		split_facet_on_chains(box,f1);
	} /* end loop through facets */

#ifdef PRINT_REFINE
		fprintf(stderr,"Final facets\n");
		print_all_facets();
#endif
}						

/********** Main entry point for routines *****************/

draw_box(box)
box_info *box;
{
	node_link_info *node_link;
	link_info *link;
	sing_info *sing;
	soltype cycle;
	int finished = FALSE;
	all_facets = NULL;

#ifdef DEBUG_FACET
/* cross-cap 4 3 4
   roman 4 7 4 and 6 3 4

   sixty5 5 4 3 , 1,6,6
if(box->xl!=4 || box->yl!=3 || box->zl!=4) return;
*/
if(box->xl!=4 || box->yl!=3 || box->zl!=4) return;
	printbox(box);
#endif
/*
*/

#ifdef PRINT_DRAW_BOX
	fprintf(stderr,"\ndraw_box: box (%d,%d,%d)/%d\n",
		box->xl,box->yl,box->zl,box->denom);
	printbox(box);
/*
	printsings(box->sings);
	printnode_links(box->node_links);
*/
	if(box->sings!=NULL) printbox(box);
#endif
	
	/*** First find a link to start from. ***/

	link = box->ll->links;
	cycle = FACE_LL;
	do
	{
		/* if no more links then search next face */

		if( link == NULL )
		{
			switch( cycle )
			{
			case FACE_LL:
				cycle = FACE_RR;
				link = box->rr->links; break;
			case FACE_RR:
				cycle = FACE_FF;
				link = box->ff->links; break;
			case FACE_FF:
				cycle = FACE_BB;
				link = box->bb->links; break;
			case FACE_BB:
				cycle = FACE_DD;
				link = box->dd->links; break;
			case FACE_DD:
				cycle = FACE_UU;
				link = box->uu->links; break;
			case FACE_UU:
				finished = TRUE; break;
			}
			continue;
		}	/* Plot if not set */
		else if( !(link->status & PLOTTEDBIT) )
			plot_facet(box,link,cycle);
		
		link = link->next;
	} while( !finished );

	/* Now divide up the facets */

/*
	plot_all_facets();
*/

	refine_facets(box);
	plot_all_facets();
	free_facets();
	all_facets = NULL;

	/* Now draw the node_links */

#ifdef PLOT_NODE_LINKS
	node_link = box->node_links;
	while(node_link != NULL)
	{
		bgnfacet();
		plot_sol(node_link->A->sol);
		plot_sol(node_link->B->sol);
		endfacet();
		node_link = node_link->next;
	}
#endif

/*
	Don't want to do this as plot_sol writes to the list of facet indicies
	sing = box->sings;
	while( sing != NULL)
	{
		plot_sol(sing->sing);
		sing=sing->next;
	}
*/
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
	facet *f;

	/* Now have a link which has not been plotted */

	link = startlink;
	cycle = startcycle;
	direction = FORWARDS;
	f = add_facet();
	add_sol_to_facet(f,link->A);
	add_sol_to_facet(f,link->B);

	link->status |= PLOTTEDBIT;
	startingsol = link->A;
	presentsol = link->B;

	while( presentsol != startingsol )
	{
		get_next_link(box,&presentsol,&link,&cycle);
		if(presentsol == startingsol ) break;

		add_sol_to_facet(f,presentsol);

	} /* end while loop */
}

get_next_link(box,presentsol,currentlink,cycle)
box_info *box;
sol_info **presentsol;
link_info **currentlink;
soltype *cycle;
{
	link_info *link;
	node_link_info *node_link;

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

/************************************************************************/
/*									*/
/*	Now some routines for drawing the facets			*/
/*									*/
/************************************************************************/

initoogl()
{
	FILE mystdout;
/*
	freopen("mystdout.txt","w",stderr);
*/
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

write_ifs_index(int index)
{
		fwrite((void *) &index, sizeof(int),1,ifs_index_file);
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
}

endfacet()
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

plot_sol(sol)
sol_info *sol;
{
	double vec[3],norm[3];
	float  fvec[3],fnorm[3],facetnorm[3],dot0,dot1,dot2;
	short  col[3];
	static short goodnorm[3];
	static float tri_vec[10][3],tri_norm[10][3];
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


	if(sol->plotindex == -1 )
	{
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



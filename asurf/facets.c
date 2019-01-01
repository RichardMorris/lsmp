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
extern int global_do_refine;
extern int global_facet_count;

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bern.h"
#include "cells.h"

/*
#define PRINT_SOLVEEDGE
#define PLOT_AS_LINES
#define OLD_PLOT
#define BINARY
#define VERBOUSE
#define NORM_ERR
#define PRINT_FACET
#define SHOW_VERTICES
#define DEBUG_FACET
#define PLOT_POINTS
#define PLOT_CHAINS
#define TEST_ALLOC
#define PRINT_FACET_ERR
#define PRINT_COMBINE_FACETS
#define PRINT_LINK_FACETS
#define PRINT_JOIN_CHAIN_POINT
#define PRINT_DRAW_BOX
#define PRINT_REFINE
#define PRINT_JOIN_FACETS
*/

#define PLOT_LINES
#define PLOT_NODE_LINKS
#define PLOT_SINGS

#define VRML
#undef  GRAPHICS
#define FORWARDS 1
#define BACKWARDS 2
#define FOUND_EVERYTHING 2

/*
#define PLOTTEDBIT 8
*/

#define grballoc(node) ( node * ) malloc( sizeof(node) )
#define fabsf(real) (float) fabs((double) real)

void cut_facet_on_sub_chain(facet_info *f1,facet_info *f2,facet_info *f3,
	facet_sol *fs1,facet_sol *fs2,
	chain_info *chain,int first,int last);
void add_sol_to_facet(facet_info *f,sol_info *s);
void add_sol_to_facet_backwards(facet_info *f,sol_info *s);
void remove_facet(facet_info *f1);
void free_facets();
void print_facet(facet_info *f1);
void print_all_facets();
void print_facets(facet_info *f1);
void split_facet_on_chains(box_info *box,facet_info *f1);
void fix_hanging_nodes(box_info *box,facet_info *f1);
void split_facet_by_sub_chains(box_info *box,facet_info *f1);
void split_facet_by_sub_chains(box_info *box,facet_info *f1);
int  join_facets_by_chains(box_info *box,facet_info *f1,facet_info *f2);
void refine_facets(box_info *box);
void follow_chain(box_info *box,chain_info *chain,sing_info *sing,node_link_info *nl);
void make_chains(box_info *box);
void follow_chain(box_info *box,chain_info *chain,sing_info *sing,node_link_info *nl);
void combine_facets(box_info *box);
int get_next_link(box_info *box,sol_info **presentsol,link_info **currentlink,soltype *cycle);
void create_facet(box_info *box,link_info *startlink,soltype startcycle);
void make_facets(box_info *box);

/* from boxclev.c */
extern void calc_pos_norm(sol_info *sol,double vec[3],double norm[3]);

extern int	edgecount, edgemax, edgenew ,
	facecount, facemax, facenew ,
	linkcount, linkmax, linknew ,
	nodecount, nodemax, nodenew ,
	boxcount, boxmax, boxnew,
	solcount,  solmax, solnew,
	vertcount, vertmax, vertnew,
	node_linkcount, node_linkmax, node_linknew,
	singcount, singmax, singnew,
	chaincount, chainmax, chainnew,
	facet_solcount, facet_solmax, facet_solnew,
	facetcount, facetmax, facetnew;

/******************* RMAR GLOBALS ***********************************/


/************************************************************************/
/*									*/
/*	draws a box.							*/
/*									*/
/************************************************************************/


/**
It all works as follows:

A facet consits of an ordered set of solutions (facet_sols).
Sols can be added to a facet at (front add_sol_to_facet)
or at the end (add_sol_to_facet_backwards)
A global list of facets (all_facets) is maintained.
Facets can be added to this with (add_facet) and removed with
(remove_facet) and thw whole list is freed up with (free_facets).
(plot_all_facets) plots the entire set of facets.
(print_facets) prints details of them on stderr.
(sol_on_facet) finds whether a sol lies on a facet and returns
the coresponding facet_sol.
(first_sol_on_facet) takes a pair of facet_sols and returns 
1 if the first facet_sol occurs first.

The first real routine is (split_facet_on_chains)
A chain is an ordered list of sols which connects nodes on the
faces of the box to to singularities in the interior.
This first finds if a facet contains repeated sols, if so
the facet is split into two and the fun is recuresed on each 
facet.
If then finds pairs of sols on the facet which are on the faces
of the box and connected by a chain and finds the shortest
such chain. 
If then checks that the chain does not form part of the boundary
of the facet. 
Finally if all the above is satisfied then the facet is split into
two which share a common edge which is the chain.
	     * ---- *
	    /	     \
	   /   chain  \
	  *---*----*---*
	   \	      /
	    \	     /
	     * ---- *

The next major routine is (join_facets_by_chain) this takes
a pair of facets and finds a pair of chains which link the two facets
these chains must not contain any points in common.
When two such chains have been found two new facets are constructed
which asumes that the facets form the oposite ends of a cylinder.
There is a bit of a logical problem here we do not know which way
the two facets should be conected
either
	  a ---------- e
	 / \          / \
	d   b        h   f
	 \ /          \ /
	  c ---------- g
or
	  a ---------- e
	 / \          / \
	d   b        f   h
	 \ /          \ /
	  c ---------- g
it might be posible to play a bit with the normals, but we cheet
by finding the path where the two dist a-c. This is wrong!

(refine_facets) manages the spliting up of facets
it starts with facets which are just bounded by edges lying on the
facets of the box.
First it calls (join_facets) for each pair of facets
then if finds facets which contain the same solution twice
and it splits them. It also duplicates any chains which start at the
linked facet (any chain can only be used to split a facet
once).
Finally it calls (split_facet_on_chains) for each facet.

The main entry point for the drawing routine is (draw_box)
this loops through all the the links joining solutions on the
edges and faces of the box. When it finds such a link it 
it calls (create_facet) this repeatadly calls (get_net_link)
which finds a link adjacent to the current one until it gets back to the
start. There is potential bug posibilities here where more than
one link is adjacet to a a node, hopefully cured by (refine_facets).

The main routine then calls (refine_facets) and then (plot_all_facets).
If drawing of degenerate lines is switched on then it will
find those chains which have not been used and print them
as well as sings which have not been used.
*/


facet_info *all_facets = NULL;

void add_sol_to_facet(facet_info *f,sol_info *s)
{
	facet_sol *fs;

	fs = (facet_sol *) malloc(sizeof(facet_sol));
#ifdef TEST_ALLOC
	++facet_solcount; ++facet_solmax; ++facet_solnew;
#endif
	fs->sol = s;
	fs->next = f->sols;
	f->sols = fs;
}

void add_sol_to_facet_backwards(facet_info *f,sol_info *s)
{
	facet_sol *fs,*fs1;

	fs = (facet_sol *) malloc(sizeof(facet_sol));
#ifdef TEST_ALLOC
	++facet_solcount; ++facet_solmax; ++facet_solnew;
#endif
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

void remove_sol_from_facet(facet_info *facet1,facet_sol *fs1)
{
	facet_sol *cur,*prev,*next;

	prev = NULL;
	for(cur = facet1->sols;cur!=NULL;cur=cur->next)
	{
		next = cur->next;

		if(cur == fs1 )
		{
			if(cur == facet1->sols) { facet1->sols = next; }
			else prev->next = next;
			free(cur);
			return;
		}
		prev = cur;
	}
}

facet_info *make_facet()
{
	facet_info *ele;
	ele = (facet_info *) malloc(sizeof(facet_info));
#ifdef TEST_ALLOC
	++facetcount; ++facetmax; ++facetnew;
#endif

	ele->next = all_facets;
	ele->sols = NULL;
	return(ele);
}

facet_info *add_facet()
{
	facet_info *ele;
	ele = make_facet();
	all_facets = ele;
	return(ele);
}

void free_facet(facet_info *f1)
{
	facet_sol *fs1,*fs2;

	fs1=f1->sols;
	while(fs1!=NULL)
	{
		fs2 = fs1->next;
		free(fs1);
		fs1 = fs2;
	}
	free(f1);
}

void remove_facet(facet_info *f1)
{
	facet_info *f2;

	f2=all_facets;
	if(f2==f1)
	{
		all_facets = f1->next;
		free(f1);
		return;
	}
	while(f2!=NULL)
	{
		if(f2->next == f1)
		{
			f2->next = f1->next;
			free_facet(f1);
			f1 = f2;
			break;
		}
		f2 = f2->next;
	}
}

facet_info *remove_facet_from_list(facet_info *existing,facet_info *facet2)
{
	facet_info *facet3,*prev=NULL;
	for(facet3=existing;facet3!=NULL;facet3=facet3->next)
	{
		if(facet3==facet2)
		{
			if(prev==NULL) return facet3->next;
			else
			{
				prev->next = facet3->next;
				return existing;
			}
		}
		prev = facet3;
	}
	return existing;
}

void free_facet_list(facet_info *f1)
{
	facet_info *f2;

	while(f1 != NULL)
	{
		f2 = f1->next;
		free_facet(f1);
		f1 = f2;
	}
}

void free_facets()
{
	free_facet_list(all_facets);
	all_facets = NULL;
}

void print_facet(facet_info *f1)
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

void print_all_facets()
{
	facet_info *f1;

	f1 = all_facets;
	while(f1 != NULL)
	{
		print_facet(f1);
		f1 = f1->next;
	}
}

void print_facets(facet_info *f1)
{
	if(f1 == NULL) fprintf(stderr,"No facets\n");
	while(f1 != NULL)
	{
		print_facet(f1);
		f1 = f1->next;
	}
}

facet_sol *sol_on_facet(facet_info *f,sol_info *s)
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

int first_sol_on_facet(facet_info *f,facet_sol *fs1,facet_sol *fs2)
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

/***** Modindfying facets by chains *****************************************/

/* actually split facet on a sub-chain
	results in f2,f3
*/

void cut_facet_on_sub_chain(facet_info *f1,facet_info *f2,facet_info *f3,
	facet_sol *fs1,facet_sol *fs2,
	chain_info *chain,int first,int last)
{
	facet_sol *fs3;
	int i,j;

	/* first copy sols on facet to new facets */

	for(fs3=fs1;fs3!=fs2 && fs3!=NULL;fs3=fs3->next)
	{
		add_sol_to_facet(f2,fs3->sol);
	}
	if(fs3==NULL)
		for(fs3=f1->sols;fs3!=fs2;fs3=fs3->next)
		{
			add_sol_to_facet(f2,fs3->sol);
		}
	add_sol_to_facet(f2,fs2->sol);
		
	for(fs3=fs2;fs3!=fs1 && fs3!= NULL;fs3=fs3->next)
	{
		add_sol_to_facet(f3,fs3->sol);
	}
	if(fs3==NULL)
		for(fs3=f1->sols;fs3!=fs1;fs3=fs3->next)
		{
			add_sol_to_facet(f3,fs3->sol);
		}
	add_sol_to_facet(f3,fs1->sol);

	/* now add the sols on chain */

	if(fs1->sol == chain->sols[first])
	{
		if(first < last)
			for(i=first+1,j=last-1;i<last;++i,--j)
			{
				add_sol_to_facet(f3,chain->sols[i]);
				add_sol_to_facet(f2,chain->sols[j]);
			}
		else
			for(i=first-1,j=last+1;i>last;--i,++j)
			{
				add_sol_to_facet(f3,chain->sols[i]);
				add_sol_to_facet(f2,chain->sols[j]);
			}
	}
	else if(fs1->sol == chain->sols[last])
	{
		if(first < last)
			for(i=first+1,j=last-1;i<last;++i,--j)
			{
				add_sol_to_facet(f3,chain->sols[j]);
				add_sol_to_facet(f2,chain->sols[i]);
			}
		else
			for(i=first-1,j=last+1;i>last;--i,++j)
			{
				add_sol_to_facet(f3,chain->sols[j]);
				add_sol_to_facet(f2,chain->sols[i]);
			}

	}
	else
	{
fprintf(stderr,"Funny stuff happening with the chain sols\n");
	}
}

void split_facet_on_chains(box_info *box,facet_info *f1)
{
	facet_sol *fs1,*fs2,*fs3,*fs4;
	facet_info *f2,*f3;
	sol_info *chainsol1,*chainsol2;
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
print_chain(chain2);
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

/** fix hanging nodes it may happen that 
	some nodes have not been joined along their
	chains **/

void fix_hanging_nodes(box_info *box,facet_info *f1)
{
	facet_sol *fs1=NULL,*fs2=NULL,*fs3=NULL,*fs4=NULL,*fs5=NULL;
	facet_info *f2=NULL,*f3=NULL;
	sol_info *chainsol1,*chainsol2;
	chain_info *chain,*chain2;
	float chain_length;
	int i;

#ifdef PRINT_REFINE
fprintf(stderr,"fix_hanging_nodes:\n");
print_facet(f1);
#endif

	/* get last facet */

 	for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
	{
		fs2 = fs1; /* previous sol */
	}
	fs3 = NULL; /* next sol */

	for(fs1=f1->sols;fs1!=NULL;fs2=fs1,fs1=fs1->next)
	{
		int last_index=-1;

		if(fs1->sol->type < FACE_LL || fs1->sol->type > FACE_UU ) continue;

		fs3 = fs1->next;
		if(fs3 == NULL) { fs3=f1->sols; }

		chain_length = 100.0;
		chain2 = NULL;
		for(chain=box->chains;chain!=NULL;chain=chain->next)
		{
			double curLen = 0.0;

			chainsol1 = chain->sols[0];
			chainsol2 = chain->sols[chain->length-1];
			if( fs1->sol == chainsol1 )
			{
				int flag3=0;
				
				for(i=1;i<chain->length;++i)
				{
					float dx,dy,dz;

			dx = ((float) chain->sols[i-1]->xl) / chain->sols[i-1]->denom
				- ((float) chain->sols[i]->xl) / chain->sols[i]->denom;
			dy = ((float) chain->sols[i-1]->yl) / chain->sols[i-1]->denom
				- ((float) chain->sols[i]->yl) / chain->sols[i]->denom;
			dz = ((float) chain->sols[i-1]->zl) / chain->sols[i-1]->denom
				- ((float) chain->sols[i]->zl) / chain->sols[i]->denom;

					curLen += sqrt( dx * dx + dy * dy + dz * dz);

					for(fs4=f1->sols;fs4!=NULL;fs4=fs4->next)
					{
						if(fs4->sol==chain->sols[i])
						{
							flag3 = 1;
							break;
						}
					}
					if(flag3) break;
				}
				if(flag3 && curLen < chain_length )
				{
					chain2 = chain;
					chain_length = curLen;
					last_index = i;
					fs5 = fs4;
				}
			}		
			else if( fs1->sol == chainsol2 )
			{
				int flag3=0;
				
				for(i=chain->length-2;i>=0;--i)
				{
					float dx,dy,dz;

			dx = ((float) chain->sols[i+1]->xl) / chain->sols[i+1]->denom
				- ((float) chain->sols[i]->xl) / chain->sols[i]->denom;
			dy = ((float) chain->sols[i+1]->yl) / chain->sols[i+1]->denom
				- ((float) chain->sols[i]->yl) / chain->sols[i]->denom;
			dz = ((float) chain->sols[i+1]->zl) / chain->sols[i+1]->denom
				- ((float) chain->sols[i]->zl) / chain->sols[i]->denom;

					curLen += sqrt( dx * dx + dy * dy + dz * dz);

					for(fs4=f1->sols;fs4!=NULL;fs4=fs4->next)
					{
						if(fs4->sol==chain->sols[i])
						{
							flag3 = 1;
							break;
						}
					}
					if(flag3) break;
				}
				if(flag3 && curLen < chain_length )
				{
					chain2 = chain;
					chain_length = curLen;
					last_index = i;
					fs5 = fs4;
				}
			}
			else
				continue;
		} /* end loop through chains */

		if(chain2 == NULL) continue;
		chainsol1 = chain2->sols[0];
		chainsol2 = chain2->sols[chain2->length-1];

		/* need to check that this chain is not already included as 
				segments round the facet */

		if( fs1->sol == chainsol1 )
		{
			if(chain2->sols[1] == fs2->sol || chain2->sols[1] == fs3->sol) continue;
		}
		else if( fs1->sol == chainsol2 )
		{
			if(chain2->sols[chain2->length-2] == fs2->sol || chain2->sols[chain2->length-2] == fs3->sol) continue;
		}
/*
			if(chain2->used) continue;
*/
			chain2->used =1;

#ifdef PRINT_REFINE
fprintf(stderr,"Split on chain\n");
print_chain(chain2);
#endif

		/* now found a chain to split on */

		f2 = add_facet();
		f3 = add_facet();
		if(fs1->sol == chainsol1)
			cut_facet_on_sub_chain(f1,f2,f3,fs1,fs5,
				chain2,0,last_index);
		else if(fs1->sol == chainsol2)
			cut_facet_on_sub_chain(f1,f2,f3,fs1,fs5,
				chain2,chain2->length-1,last_index);
		else
		{
fprintf(stderr,"Funny stuff happening with the chain sols\n");
		}


#ifdef PRINT_REFINE
fprintf(stderr,"fixed_hanging_nodes:\n");
print_facet(f2);
print_facet(f3);
#endif
		fix_hanging_nodes(box,f2);
		fix_hanging_nodes(box,f3);

			/* now have to remove f1 for list of facets */

		remove_facet(f1);
		return;
	}						
}

/** fix hanging nodes it may happen that 
	some nodes have not been join along their
	chains **/

void split_facet_by_sub_chains(box_info *box,facet_info *f1)
{
	facet_sol *fs1,*fs2,*fs3;
	facet_info *f2,*f3;
	chain_info *chain,*chain2;
	float chain_length;
	int i,flag;
	int last_index=-1,first_index=-1;
	int found_first,found_second;

	if(f1->sols == NULL || f1->sols->next == NULL || f1->sols->next->next == NULL) return;

#ifdef PRINT_REFINE
fprintf(stderr,"split_facet_by_sub_chains:\n");
print_facet(f1);
if(box->xl==19 && box->yl==7 && box->zl==21)
{
fprintf(stderr,"split_facet_by_sub_chains:\n");
print_facet(f1);print_chains(box->chains);
}
#endif
	for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
	{
	    if(fs1->sol->type < FACE_LL) continue;
	    for(fs2=f1->sols;fs2!=NULL;fs2=fs2->next)
	    {
		if(fs2 == fs1) continue;
		if(fs2->sol->type < FACE_LL) continue;

		/* Now got a reasonable pair of facet sols lets look for chains */


		chain_length = 100.0;
		chain2 = NULL;
		for(chain=box->chains;chain!=NULL;chain=chain->next)
		{
			double curLen = 0.0;
			found_first = found_second = -1;

			for(i=0;i<chain->length;++i)
			{
/*
fprintf(stderr,"%p %p %p\t",fs1->sol,fs2->sol,chain->sols[i]);
print_sol(chain->sols[i]);
*/
				if(fs1->sol == chain->sols[i]) found_first = i;
				if(fs2->sol == chain->sols[i]) found_second = i;
			}
/*
fprintf(stderr,"found_first %d %d\n",found_first,found_second);
*/
			if(found_first == -1 || found_second == -1) continue;
			if(found_first < found_second)
			{
				for(i=found_first;i<found_second;++i)
					curLen += chain->metLens[i];
			}
			else
			{
				for(i=found_second;i<found_first;++i)
					curLen += chain->metLens[i];
			}
				
			if(curLen < chain_length )
			{
				chain2 = chain;
				chain_length = curLen;
				if(found_first < found_second)
				{
					first_index = found_first;
					last_index = found_second;
				}
				else
				{
					first_index = found_second;
					last_index = found_first;
				}
			}
		} /* end loop through chains */

		if(chain2 == NULL) continue;

		/* need to check that this chain is not already included as 
				segments round the facet */

		flag = 0;
		for(i=first_index+1;i<last_index;++i)
		{
			for(fs3=f1->sols;fs3!=NULL;fs3=fs3->next)
				if(fs3->sol == chain2->sols[i]) { flag = 1; break; }
			if(flag) break;
		}
		if(flag) continue;
		if(chain2->length==2 &&
			(fs2 == fs1->next || fs1 == fs2->next
			|| ( fs1 == f1->sols && fs2->next == NULL )
			|| ( fs2 == f1->sols && fs1->next == NULL ) ) )
			continue;
/*
		if(chain2->used) continue;
*/
		if( last_index == first_index ) continue;
		if( last_index-first_index == 1 
		    && ( fs2 == fs1->next 
		       || fs1 == fs2->next 
		       || ( fs1 == f1->sols && fs2->next == NULL )
		       || ( fs2 == f1->sols && fs1->next == NULL ) ) ) 
			continue;
		chain2->used =1;

#ifdef PRINT_REFINE
fprintf(stderr,"Split on chain %d %d\n",first_index,last_index);
print_chain(chain2);
#endif
		/* now found a chain to split on */

		f2 = add_facet();
		f3 = add_facet();
		cut_facet_on_sub_chain(f1,f2,f3,fs1,fs2,
				chain2,first_index,last_index);


#ifdef PRINT_REFINE
fprintf(stderr,"splited facet_by_sub_chains:\n");
print_facet(f2);
print_facet(f3);
#endif
		split_facet_by_sub_chains(box,f2);
		split_facet_by_sub_chains(box,f3);

			/* now have to remove f1 for list of facets */

		remove_facet(f1);
		return;
	    } /* end fs2 loop */
	}  /* end fs1 loop */			
}

double calc_fs_dist(facet_sol *fs1,facet_sol *fs2)
{
	double vec1[3],vec2[3];
	double dx,dy,dz;

	calc_pos_actual(fs1->sol,vec1);
	calc_pos_actual(fs2->sol,vec2);
	dx = vec1[0]-vec2[0];
	dy = vec1[1]-vec2[1];
	dz = vec1[2]-vec2[2];
	return sqrt(dx*dx+dy*dy+dz*dz);
}
	

int calc_orint_of_joined_facets(facet_info *f1,facet_info *f2,
	facet_sol *fs1,facet_sol *fs2,facet_sol *fs3,facet_sol *fs4)
{
	facet_sol *fs5,*fs6,*fs7,*fs8;
	int test1=FALSE,test2=FALSE,test3=FALSE,test4=FALSE,test5=FALSE,test6=FALSE;
	int orient_error = 0; /* whether an error found in calculation */

	fs5 = fs1->next;
	if( fs5 == NULL ) fs5 = f1->sols;
	fs6 = fs2->next;
	if( fs6 == NULL ) fs6 = f2->sols;
	fs7 = fs3->next;
	if( fs7 == NULL ) fs7 = f1->sols;
	fs8 = fs4->next;
	if( fs8 == NULL ) fs8 = f2->sols;

	if( fs1->sol->dx == fs2->sol->dx 
	 && fs1->sol->dy == fs2->sol->dy
	 && fs1->sol->dz == fs2->sol->dz )
	{
		if( fs1->sol->dx == 0 && fs1->sol->dy != 0 && fs1->sol->dz != 0 )
		{
			test1 = fs5->sol->dx;
			test2 = fs6->sol->dx;
		}
		else if( fs1->sol->dx != 0 && fs1->sol->dy == 0 && fs1->sol->dz != 0 )
		{
			test1 = fs5->sol->dy;
			test2 = fs6->sol->dy;
		}
		else if( fs1->sol->dx != 0 && fs1->sol->dy != 0 && fs1->sol->dz == 0 )
		{
			test1 = fs5->sol->dz;
			test2 = fs6->sol->dz;
		}
		else
		{
			orient_error = 1;
		}
	}
	else
	{
		orient_error = 2;
	}

	if( fs3->sol->dx == fs4->sol->dx 
	 && fs3->sol->dy == fs4->sol->dy
	 && fs3->sol->dz == fs4->sol->dz )
	{
		if( fs3->sol->dx == 0 && fs3->sol->dy != 0 && fs3->sol->dz != 0 )
		{
			test3 = fs7->sol->dx;
			test4 = fs8->sol->dx;
		}
		else if( fs3->sol->dx != 0 && fs3->sol->dy == 0 && fs3->sol->dz != 0 )
		{
			test3 = fs7->sol->dy;
			test4 = fs8->sol->dy;
		}
		else if( fs3->sol->dx != 0 && fs3->sol->dy != 0 && fs3->sol->dz == 0 )
		{
			test3 = fs7->sol->dz;
			test4 = fs8->sol->dz;
		}
		else
		{
			orient_error = 3;
		}
	}
	else
	{
		orient_error = 4;
	}
	
	if( orient_error ) {}
	else if(test1 == 0 || test2 == 0 )
		orient_error = 5;
	else if( test1 == test2 )
		test5 = 1;
	else
		test5 = -1;

	if( orient_error ) {}
	else if(test1 == 0 || test2 == 0 )
		orient_error = 6;
	else if( test3 == test4 )
		test6 = 1;
	else
		test6 = -1;

	if( orient_error ) {}
	else if( test5 != test6 )
	{
		orient_error = 7; /* This is serious as get different info form the links */
	}
	else if( test5 != 0 ) return test5;

	/* well that failed */

	fprintf(stderr,"Error calculation orientation %d\n",orient_error);
#ifdef PRINT_FACET_ERR
	print_facet(f1);
	print_facet(f2);
	print_chain(chain1);
	print_chain(chain2);
#endif
	/* try calculation normals at each point */

	{
	double vec1[3],vec2[3],vec3[3],vec4[4],vec5[3],vec6[3],vec7[3],vec8[3];
	double norm1[3],norm2[3],norm3[3],norm4[4],norm5[3],norm6[3],norm7[3],norm8[3];
 	double vec15[3],vec12[3],vec26[3], vec34[3],vec37[3],vec48[3];
	double vnorm1[3],vnorm2[3],vnorm3[3],vnorm4[3];
	double dot1,dot2,dist,dist1,dist2,dist3,dist4,dist5;
	int count1 = 0, count2 = 0, count3 = 0,res1,res2;
	

	dist = 0.0;
	for(fs5=f1->sols,fs6=NULL;fs5!=NULL;fs6=fs5,fs5=fs5->next)
	{
		if(fs6!=NULL) 
			dist += calc_fs_dist(fs5,fs6);
		if(fs5 == fs1) dist1 = dist;
		if(fs5 == fs3) dist3 = dist;
	}
	dist += calc_fs_dist(f1->sols,fs6);
	if(dist1<dist3) { res1 = ( ( 2.0 * ( dist3 - dist1 ) ) < dist ); }
	else		{ res1 = ( ( 2.0 * ( dist1 - dist3 ) ) > dist ); }
	dist5 = dist;
	dist = 0.0;
	for(fs5=f2->sols,fs6=NULL;fs5!=NULL;fs6=fs5,fs5=fs5->next)
	{
		if(fs6!=NULL) dist+= calc_fs_dist(fs5,fs6);
		if(fs5 == fs2) dist2 = dist;
		if(fs5 == fs4) dist4 = dist;
	}
	dist += calc_fs_dist(f2->sols,fs6);
	if(dist2<dist4) { res2 = ( ( 2.0 * ( dist4 - dist2 ) ) < dist ); }
	else		{ res2 = ( ( 2.0 * ( dist2 - dist4 ) ) > dist ); }

/*
fprintf(stderr,"dist %f %f %f  %f %f %f res %d %d\n",dist5,dist1,dist3,dist,dist2,dist4,res1,res2);
*/

	if(  ( res1 && res2 ) || ( !res1 && !res2 ) ) return 1;
	else return -1;

	for(fs5=f2->sols;fs5!=NULL;fs5=fs5->next)
	{
		++count3;
		if(fs5 == fs2) count1 = count3;
		if(fs5 == fs4) count2 = count3;
	}

	calc_pos_norm(fs1->sol,vec1,norm1);
	calc_pos_norm(fs2->sol,vec2,norm2);
	calc_pos_norm(fs3->sol,vec3,norm3);
	calc_pos_norm(fs4->sol,vec4,norm4);
	calc_pos_norm(fs5->sol,vec5,norm5);
	calc_pos_norm(fs6->sol,vec6,norm6);
	calc_pos_norm(fs7->sol,vec7,norm7);
	calc_pos_norm(fs8->sol,vec8,norm8);

	#define vec_sub(v1,v2,v3) { v1[0] = v2[0]-v3[0]; v1[1] = v2[1]-v3[1]; v1[2] = v2[2]-v3[2]; }
	vec_sub(vec15,vec5,vec1)
	vec_sub(vec12,vec2,vec1)
	vec_sub(vec26,vec6,vec2)
	vec_sub(vec34,vec4,vec3)
	vec_sub(vec37,vec7,vec3)
	vec_sub(vec48,vec8,vec4)

	#define vec_cross(v1,v2,v3) {\
		v1[0] = v2[1] * v3[2] - v2[2] * v3[1]; \
		v1[1] = v2[2] * v3[0] - v2[0] * v3[2]; \
		v1[2] = v2[0] * v3[1] - v2[1] * v3[0]; }
	vec_cross(vnorm1,vec15,vec12)
	vec_cross(vnorm2,vec12,vec26)
	vec_cross(vnorm3,vec37,vec34)
	vec_cross(vnorm4,vec34,vec48)

	#define vec_dot(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
	dot1 = vec_dot(vnorm1,vnorm2);
	dot2 = vec_dot(vnorm3,vnorm4);

#ifdef PRINT_FACET_ERR
#define print_vec(s1,v1) { fprintf(stderr,"%s %f %f %f\n",s1,v1[0],v1[1],v1[2]); }
	print_vec("v1",vec1);
	print_vec("v2",vec2);
	print_vec("v3",vec3);
	print_vec("v4",vec4);
	print_vec("v5",vec5);
	print_vec("v6",vec6);
	print_vec("v7",vec7);
	print_vec("v8",vec8);

	print_vec("v15",vec15);
	print_vec("v12",vec12);
	print_vec("v26",vec26);
	print_vec("v34",vec34);
	print_vec("v37",vec37);
	print_vec("v48",vec48);

	print_vec("vn1",vnorm1);
	print_vec("vn2",vnorm2);
	print_vec("vn3",vnorm3);
	print_vec("vn4",vnorm4);

	fprintf(stderr,"dot1 %g dot2 %g\n",dot1,dot2);
#endif

	if(dot1 > 0.0 && dot2 > 0.0 ) return 1;
	if(dot1 < 0.0 && dot2 < 0.0 ) return -1;

	fprintf(stderr,"calc_orient: Normals different\n");
	
	return 1;
	/* now count round the second facet to see which is the shortest linking path 
		this is on course rubbish as we don't know that we have shortets path
		on the other facet */

	
		for(fs5=f2->sols;fs5!=NULL;fs5=fs5->next)
		{
			++count3;
			if(fs5 == fs2) count1 = count3;
			if(fs5 == fs4) count2 = count3;
		}
		if(count1 < count2 )
		{
			if(count2-count1<count3-count2+count1) return 1;
			else return -1;
		}
		else
		{
			if(count1-count2<count3-count1+count2) return 1;
			else return -1;
		}
	}
	return 1;
	
}

int join_on_chain_and_point(box_info *box,facet_info *f1,facet_info *f2,
	chain_info *chain,facet_sol *dp1,facet_sol *dp2,facet_sol *ch1,facet_sol *ch2)
{
	int res,i;
	facet_info *f3=NULL,*f4=NULL;
	facet_sol *fs5;

fprintf(stderr,"join on chain and point\n");
	if(dp1 == ch1 || dp2 == ch2 ) return 0;

	res = calc_orint_of_joined_facets(f1,f2,dp1,dp2,ch1,ch2);
#ifdef PRINT_JOIN_CHAIN_POINT
fprintf(stderr,"orient %d\n",res);
print_sol(dp1->sol);
print_chain(chain);
print_facet(f1);
print_facet(f2);
#endif
	f3 = add_facet();
	f4 = add_facet();

	add_sol_to_facet(f3,dp1->sol);

	for(fs5=dp1->next;fs5!=ch1 && fs5!=NULL; fs5=fs5->next)
	{
		add_sol_to_facet(f3,fs5->sol);
	}
	if(fs5==NULL)
		for(fs5=f1->sols;fs5!=ch1; fs5=fs5->next)
		{
			add_sol_to_facet(f3,fs5->sol);
		}
	if(chain->sols[0] == ch1->sol)
	{
		for(i=0;i<chain->length;++i)
			add_sol_to_facet(f3,chain->sols[i]);
	}
	else
	{
		for(i=chain->length-1;i>=0;--i)
			add_sol_to_facet(f3,chain->sols[i]);
	}


	if(chain->sols[0] == ch1->sol)
	{
		for(i=chain->length-1;i>=0;--i)
			add_sol_to_facet(f4,chain->sols[i]);
	}
	else
	{
		for(i=0;i<chain->length;++i)
			add_sol_to_facet(f4,chain->sols[i]);
	}
	for(fs5=ch1->next;fs5!=dp1 && fs5!=NULL; fs5=fs5->next)
	{
		add_sol_to_facet(f4,fs5->sol);
	}
	if(fs5==NULL)
		for(fs5=f1->sols;fs5!=dp1; fs5=fs5->next)
		{
			add_sol_to_facet(f4,fs5->sol);
		}
	add_sol_to_facet(f4,dp2->sol);

	if(res>0)
	{
		for(fs5=dp2->next;fs5!=ch2 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet_backwards(f3,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=ch2; fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f3,fs5->sol);
			}

		for(fs5=ch2->next;fs5!=dp2 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet_backwards(f4,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=dp2; fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f4,fs5->sol);
			}
	}
	else
	{
		for(fs5=ch2->next;fs5!=dp2 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet(f3,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=dp2; fs5=fs5->next)
			{
				add_sol_to_facet(f3,fs5->sol);
			}
		for(fs5=dp2->next;fs5!=ch2 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet(f4,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=ch2; fs5=fs5->next)
			{
				add_sol_to_facet(f4,fs5->sol);
			}
	}

#ifdef PRINT_JOIN_CHAIN_POINT
fprintf(stderr,"after join on chain and point\n");
print_facet(f3);
print_facet(f4);
#endif
	remove_facet(f1);
	remove_facet(f2);
	return(1);
}

int join_facets_by_chains(box_info *box,facet_info *f1,facet_info *f2)
{
	facet_sol *fs1=NULL,*fs2=NULL,*fs3=NULL,*fs4=NULL,*fs5=NULL;
	chain_info *chain=NULL,*chain2=NULL,*chain3=NULL;
	int count = 0,i;
	facet_info *f3=NULL,*f4=NULL;
	float chain_length;
	sol_info *chainsol1=NULL,*chainsol2=NULL;
	int res;
	facet_sol *double_pointA=NULL,*double_pointB=NULL;

#ifdef PRINT_REFINE
fprintf(stderr,"Join facets\n");
print_facet(f1);
print_facet(f2);
#endif
	
	for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
	{
		for(fs2=f2->sols;fs2!=NULL;fs2=fs2->next)
		{
			if(fs1->sol == fs2->sol)
			{
				double_pointA = fs1;
				double_pointB = fs2;
			}
		}
	}

	chain3 = NULL;
	for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
	{
		for(fs2=f2->sols;fs2!=NULL;fs2=fs2->next)
		{
			int flag5;

			if(fs1->sol == fs2->sol) continue;
			chain_length = 100.0;
			chain2 = NULL;
			for(chain=box->chains;chain!=NULL;chain=chain->next)
			{
				int f1_index=-1,f2_index=-1;

				chainsol1 = chain->sols[0];
				chainsol2 = chain->sols[chain->length-1];
				if( chainsol1 == fs1->sol ) f1_index = 0;
				if( chainsol2 == fs1->sol ) f1_index = chain->length-1;

				if( chainsol1 == fs2->sol ) f2_index = 0;
				if( chainsol2 == fs2->sol ) f2_index = chain->length-1;

				if(f1_index == -1 || f2_index == -1) continue;

				if( double_pointA != NULL &&
					(  chainsol1 == double_pointA->sol
					|| chainsol2 == double_pointA->sol ) ) continue;


				/* check that none of sols on chain are on facet */
			
				flag5 = 0;

				for(fs5=f1->sols;fs5!=NULL;fs5=fs5->next)
				{
					for(i=1;i<chain->length-1;++i)
					{
						if(chain->sols[i]==fs5->sol)
						{
							flag5 = 1;
							break;
						}
					}
					if(chain->sols[f2_index] == fs5->sol)
							flag5 = 1;

					if(flag5) break;
				}

				for(fs5=f2->sols;fs5!=NULL;fs5=fs5->next)
				{
					for(i=1;i<chain->length-1;++i)
					{
						if(chain->sols[i]==fs5->sol)
						{
							flag5 = 1;
							break;
						}
					}
					if(chain->sols[f1_index] == fs5->sol)
							flag5 = 1;
					if(flag5) break;
				}

				if(flag5) continue;
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
			/* found a chain */
			++count;
			if(count == 2) break; /* have two OK chains */ 

			/* now got the first chain */

			fs3 = fs1; fs4 = fs2;
			chain3 = chain2;
		} /* end fs2 loop */
		if(count>=2) break;
	} /* end fs1 loop */

	if(count == 0) return 0; /* no linking chains */
	if(double_pointA!=NULL)
	{
		return join_on_chain_and_point(box,f1,f2,chain3,
			double_pointA,double_pointB,fs3,fs4);
	}
	if(count == 1) return 0;

#ifdef PRINT_REFINE
fprintf(stderr,"Found two linking chains\n");
print_chain(chain2);
fprintf(stderr,"and\n");
print_chain(chain3);
#endif
	if(fs1 == fs3 || fs2 == fs4)
	{
		fprintf(stderr,"two of the linking facet sols arethe same\n");
		return 0;
	}
	chain2->used = 1;
	chain3->used = 1;

/*
	res = calc_orint_of_joined_facets(f1,f2,chain2,chain3,
		fs1,fs2,fs3,fs4);
*/
	res = calc_orint_of_joined_facets(f1,f2,
		fs1,fs2,fs3,fs4);
	f3 = add_facet();
	f4 = add_facet();

	if(chain2->sols[0] == fs1->sol)
	{
		for(i=chain2->length-1;i>=0;--i)
			add_sol_to_facet(f3,chain2->sols[i]);
	}
	else
	{
		for(i=0;i<chain2->length;++i)
			add_sol_to_facet(f3,chain2->sols[i]);
	}
	for(fs5=fs1->next;fs5!=fs3 && fs5!=NULL; fs5=fs5->next)
	{
		add_sol_to_facet(f3,fs5->sol);
	}
	if(fs5==NULL)
		for(fs5=f1->sols;fs5!=fs3; fs5=fs5->next)
		{
			add_sol_to_facet(f3,fs5->sol);
		}
	if(chain3->sols[0] == fs3->sol)
	{
		for(i=0;i<chain3->length;++i)
			add_sol_to_facet(f3,chain3->sols[i]);
	}
	else
	{
		for(i=chain3->length-1;i>=0;--i)
			add_sol_to_facet(f3,chain3->sols[i]);
	}


	if(chain3->sols[0] == fs3->sol)
	{
		for(i=chain3->length-1;i>=0;--i)
			add_sol_to_facet(f4,chain3->sols[i]);
	}
	else
	{
		for(i=0;i<chain3->length;++i)
			add_sol_to_facet(f4,chain3->sols[i]);
	}
	for(fs5=fs3->next;fs5!=fs1 && fs5!=NULL; fs5=fs5->next)
	{
		add_sol_to_facet(f4,fs5->sol);
	}
	if(fs5==NULL)
		for(fs5=f1->sols;fs5!=fs1; fs5=fs5->next)
		{
			add_sol_to_facet(f4,fs5->sol);
		}
	if(chain2->sols[0] == fs1->sol)
	{
		for(i=0;i<chain2->length;++i)
			add_sol_to_facet(f4,chain2->sols[i]);
	}
	else
	{
		for(i=chain2->length-1;i>=0;--i)
			add_sol_to_facet(f4,chain2->sols[i]);
	}


	if(res>0)
	{
		for(fs5=fs2->next;fs5!=fs4 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet_backwards(f3,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=fs4; fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f3,fs5->sol);
			}
		for(fs5=fs4->next;fs5!=fs2 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet_backwards(f4,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=fs2; fs5=fs5->next)
			{
				add_sol_to_facet_backwards(f4,fs5->sol);
			}
	}
	else
	{
		for(fs5=fs4->next;fs5!=fs2 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet(f3,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=fs2; fs5=fs5->next)
			{
				add_sol_to_facet(f3,fs5->sol);
			}
		for(fs5=fs2->next;fs5!=fs4 && fs5!=NULL; fs5=fs5->next)
		{
			add_sol_to_facet(f4,fs5->sol);
		}
		if(fs5==NULL)
			for(fs5=f2->sols;fs5!=fs4; fs5=fs5->next)
			{
				add_sol_to_facet(f4,fs5->sol);
			}
	}

	

	remove_facet(f1);
	remove_facet(f2);
	return(1);
}

void refine_facets(box_info *box)
{
	facet_sol *fs1,*fs2,*fs3,*fs4;
	facet_info *f1,*f2,*f3,*f4;
	int flag;

	/* May have a facet with repeated vertices
		if so split the facet */

#ifdef PRINT_REFINE
		print_all_facets();
#endif


	flag = 1;
	while(flag)
	{
	    flag = 0;
	    for(f1=all_facets;f1!=NULL;f1=f1->next)
	    {
		for(fs1=f1->sols;fs1!=NULL;fs1=fs1->next)
		{
			for(fs2=fs1->next,fs3=fs1;fs2!=NULL;fs3=fs2,fs2=fs2->next)
			{
				if(fs1->sol == fs2->sol)
				{
#ifdef PRINT_REFINE
fprintf(stderr,"Split on sol\n");
print_sol(fs1->sol);
print_facet(f1);
#endif
					f2 = add_facet();
					fs4 = fs1->next;
					fs1->next = fs2->next;
					if(fs2 != fs4)
						fs2->next = fs4;
					if(fs3!=fs1)
						fs3->next = NULL;
					f2->sols = fs2;	
					flag = 1;
#ifdef PRINT_REFINE
fprintf(stderr,"after Split on sol\n");
print_facet(f1);
print_facet(f2);
#endif
					break;
				}
			if(flag) break;
			} /* end fs2 loop */
		if(flag) break;
		} /* end fs1 loop */
	    if(flag) break;
	    } /* end f1 loop */
	}

	/* Now join together facets linked by chains */

	flag = 1;
	while(flag)
	{
		flag = 0;
		for(f1=all_facets;f1!=NULL;f1=f3)
		{
			f3 = f1->next;
			for(f2=f1->next;f2!=NULL;f2=f4)
			{
				f4 = f2->next;
				flag = join_facets_by_chains(box,f1,f2);
				if(flag) break;
			}
			if(flag) break;
		}
	}	
#ifdef PRINT_REFINE
		fprintf(stderr,"After joining\n");
		print_all_facets();
#endif

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
#ifdef TEST_ALLOC
	++chaincount; ++chainmax; ++chainnew;
#endif
			chain2->next = chain->next;
			chain->next = chain2;
			chain2->length = chain->length;
			chain2->metric_length = chain->metric_length;
			chain2->used = 0;
			chain2->sols = (sol_info **) malloc(sizeof(sol_info *)*chain->length);
#ifdef TEST_ALLOC
	++chaincount; ++chainmax; ++chainnew;
#endif
			bcopy(chain->sols,chain2->sols,sizeof(sol_info *)*chain->length);
			chain = chain2;
		}
	}								
*/

#ifdef PRINT_REFINE
	fprintf(stderr,"After split on sols\n");
	print_all_facets();
/*
	print_chains(box->chains);
*/
#endif

	/* Now split facets where two of the nodes are linked by a chain */

	for(f1=all_facets;f1!=NULL;f1=f2)
	{
		f2 = f1->next; /* cant guarentee that f1 will still exist after splitting */
/*
		fix_hanging_nodes(box,f1);
*/
		split_facet_by_sub_chains(box,f1);
	} /* end loop through facets */

	for(f1=all_facets;f1!=NULL;f1=f2)
	{
		f2 = f1->next; /* cant guarentee that f1 will still exist after splitting */
		fix_hanging_nodes(box,f1);
	} /* end loop through facets */

#ifdef PRINT_REFINE
		fprintf(stderr,"Final facets\n");
		print_all_facets();
#endif
}						



/************************************************************************/
/*									*/
/*	Working out chains of singularities through a box		*/
/*	make_chains starts from each node_link				*/
/*	if the node link has two faces at its end then we have a simple */
/* 	two element chain.						*/
/*	if it has two sings then we ignore it				*/
/*	if it has only one sing then use the follow_chain procedure	*/
/*	to follow the chain until it reaches another face node		*/
/*	or if it joins back on itself					*/
/*	If an intermediate sing has more than two adjacent node_links 	*/
/*	create a new chain and recursivly call follow_chain on than	*/
/*									*/
/************************************************************************/

void make_chains(box_info *box)
{
	chain_info *chain,*chain1,*chain2,*chain3;
	node_link_info *n1;
	float dx,dy,dz;
	int flag,i;

	box->chains = NULL;
	for(n1=box->node_links;n1!=NULL;n1=n1->next)
	{
		if(n1->singA == NULL && n1->singB == NULL)
		{
			chain = (chain_info *) malloc(sizeof(chain_info));
#ifdef TEST_ALLOC
	++chaincount; ++chainmax; ++chainnew;
#endif
			chain->length =2;
			chain->used = 0;
			chain->sols = (sol_info **) malloc(sizeof(sol_info *) * 2);
			chain->sols[0] = n1->A->sol;
			chain->sols[1] = n1->B->sol;

			dx = ((float) chain->sols[0]->xl) / chain->sols[0]->denom
				- ((float) chain->sols[1]->xl) / chain->sols[1]->denom;
			dy = ((float) chain->sols[0]->yl) / chain->sols[0]->denom
				- ((float) chain->sols[1]->yl) / chain->sols[1]->denom;
			dz = ((float) chain->sols[0]->zl) / chain->sols[0]->denom
				- ((float) chain->sols[1]->zl) / chain->sols[1]->denom;
			chain->metric_length = sqrt( dx * dx + dy * dy + dz * dz);

			chain->metLens = (float *) malloc(sizeof(float) * 1);
			chain->metLens[0] = chain->metric_length;
			chain->next = box->chains;
			box->chains = chain;
		}
		else if(n1->singA != NULL && n1->singB != NULL)
		{
		}
		else
		{
			chain = (chain_info *) malloc(sizeof(chain_info));
#ifdef TEST_ALLOC
	++chaincount; ++chainmax; ++chainnew;
#endif
			chain->length =2;
			chain->used = 0;
			chain->sols = (sol_info **) malloc(sizeof(sol_info *) * (box->num_sings+2));
			chain->metLens = (float *) malloc(sizeof(float) * (box->num_sings+1));
			if(n1->singA == NULL)
			{
				chain->sols[0] = n1->A->sol;
				chain->sols[1] = n1->B->sol;
			}
			else
			{
				chain->sols[0] = n1->B->sol;
				chain->sols[1] = n1->A->sol;
			}

			dx = ((float) chain->sols[0]->xl) / chain->sols[0]->denom
				- ((float) chain->sols[1]->xl) / chain->sols[1]->denom;
			dy = ((float) chain->sols[0]->yl) / chain->sols[0]->denom
				- ((float) chain->sols[1]->yl) / chain->sols[1]->denom;
			dz = ((float) chain->sols[0]->zl) / chain->sols[0]->denom
				- ((float) chain->sols[1]->zl) / chain->sols[1]->denom;
			chain->metric_length = sqrt( dx * dx + dy * dy + dz * dz);
			chain->metLens[0] = chain->metric_length;

			chain->next = box->chains;
			box->chains = chain;
			
			/* Now recurse along the chain */

			if(n1->singA != NULL)
			{
				follow_chain(box,chain,n1->singA,n1);
			}
			else
			{
				follow_chain(box,chain,n1->singB,n1);
			}
		}
	}

	/* Some chains may not end on a face - remove them */

	chain3 = NULL;
	for(chain1=box->chains;chain1!=NULL;chain1=chain2)
	{
		chain2 = chain1->next;
		if( chain1->sols[0]->type < FACE_LL || chain1->sols[0]->type > FACE_UU 
		 || chain1->sols[chain1->length-1]->type < FACE_LL 
		 || chain1->sols[chain1->length-1]->type > FACE_UU )
		{
			if(chain3 != NULL)
				chain3->next = chain2;
			else
				box->chains = chain2;	
			free(chain1->sols);
			free(chain1->metLens);
			free(chain1);
		}
		else
			chain3 = chain1;
	}
			


	/* Now want to remove duplicate chains */

	for(chain1=box->chains;chain1!=NULL;chain1=chain1->next)
	{
		for(chain2=chain1->next,chain3=chain1;chain2!=NULL;chain3=chain2,chain2=chain2->next)
		{
			if(chain1->length != chain2->length) continue;
			flag = 1;
			if( chain1->sols[0] == chain2->sols[0] 
			 && chain1->sols[chain1->length-1] == chain2->sols[chain2->length-1] )
			{
				for(i=1;i<chain1->length-1;++i)
				{
					if(chain1->sols[i] != chain2->sols[i]) flag = 0;
				}
			}
			else if( chain1->sols[0] == chain2->sols[chain2->length-1] 
			 && chain1->sols[chain1->length-1] == chain2->sols[0] )
			{
				for(i=1;i<chain1->length-1;++i)
				{
					if(chain1->sols[i] != chain2->sols[chain2->length-1-i]) flag = 0;
				}
			}
			else flag = 0;

			if(flag)
			{
				chain3->next = chain2->next;
				free(chain2->sols);
				free(chain2->metLens);
				free(chain2);
				break;
			}
		}
	}			
}

void follow_chain(box_info *box,chain_info *chain,sing_info *sing,node_link_info *nl)
{
	sol_info *next_sol=NULL;
	sing_info *next_sing=NULL;
	node_link_info *next_nl=NULL;
	chain_info *chain2=NULL;
	int i,j,k;
	float dx,dy,dz;

#ifdef PRINT_FOLLOW_CHAIN
fprintf(stderr,"follow_chain:\n");
printsing(sing);
printnode_link(nl);
#endif
	while(1)
	{
		if(sing->numNLs == 0)
		{
			fprintf(stderr,"Sing has zero adjNLs\n");
			break;
		}
		else if(sing->numNLs == 1)
		{
			fprintf(stderr,"Sing has only one adjNLs\n");
			break;
		}
		else if(sing->numNLs == 2)
		{
#ifdef PRINT_FOLLOW_CHAIN
fprintf(stderr,"Simple add\n");
#endif
			if(sing->adjacentNLs[0] == nl)
			{
				next_nl = sing->adjacentNLs[1];
			}
			else if(sing->adjacentNLs[1] == nl)
			{
				next_nl = sing->adjacentNLs[0];
			}
			else
			{
				fprintf(stderr,"node_link not adjacet to sing\n");
			}

			if(next_nl->singA == sing)
			{
				next_sing = next_nl->singB;
				next_sol = next_nl->B->sol;
			}
			else if(next_nl->singB == sing)
			{
				next_sing = next_nl->singA;
				next_sol = next_nl->A->sol;
			}
			else
			{
				fprintf(stderr,"Sing not adjacent to NL\n");
				break;
			}
						
			/* now check that the next sol is not already in the chain */

			for(i=0;i<chain->length;++i)
			{
				if(next_sol == chain->sols[i])
				{
					break;
				}
			}
			if(i!=chain->length) break;

			/* everything OK add sol to the end of the chain */

			chain->sols[chain->length++] = next_sol;

			dx = ((float) chain->sols[chain->length-2]->xl) / chain->sols[chain->length-2]->denom
				- ((float) chain->sols[chain->length-1]->xl) / chain->sols[chain->length-1]->denom;
			dy = ((float) chain->sols[chain->length-2]->yl) / chain->sols[chain->length-2]->denom
				- ((float) chain->sols[chain->length-1]->yl) / chain->sols[chain->length-1]->denom;
			dz = ((float) chain->sols[chain->length-2]->zl) / chain->sols[chain->length-2]->denom
				- ((float) chain->sols[chain->length-1]->zl) / chain->sols[chain->length-1]->denom;
			chain->metLens[chain->length-2] = sqrt( dx * dx + dy * dy + dz * dz);
			chain->metric_length += chain->metLens[chain->length-2];
 			nl = next_nl;
			sing = next_sing;

			if(sing==NULL) break;	/* reached the end of the chain */
			continue;
		}

		/* now have more than two sings in the chain */
#ifdef PRINT_FOLLOW_CHAIN
fprintf(stderr,"SIng with %d node links\n",sing->numNLs);
printsing(sing);
#endif
		j = 0;
		for(i=0;i<sing->numNLs;++i)
		{
			if(sing->adjacentNLs[i] == nl) continue;
#ifdef PRINT_FOLLOW_CHAIN
fprintf(stderr,"Trying link no %d\n",i);
#endif
			
			next_nl = sing->adjacentNLs[i];
			if(next_nl->singA == sing)
			{
				next_sing = next_nl->singB;
				next_sol = next_nl->B->sol;
			}
			else if(next_nl->singB == sing)
			{
				next_sing = next_nl->singA;
				next_sol = next_nl->A->sol;
			}
			else
			{
				fprintf(stderr,"Sing not adjacent to NL\n");
				break;
			}
#ifdef PRINT_FOLLOW_CHAIN
printnode_link(next_nl);
printsing(next_sing);
print_sol(next_sol);
#endif
			/* now check that the next sol is not already in the chain */


			for(k=0;k<chain->length;++k)
			{
				if(next_sol == chain->sols[k])
				{
					break;
				}
			} 

			++j; /* always increment this so can pick up last adjNL to do */
			if(j < sing->numNLs -1 )
			{
				if(k!=chain->length) continue;

				/* need to make a new chain */
#ifdef PRINT_FOLLOW_CHAIN
fprintf(stderr,"Making a new chain\n");
#endif
				chain2 = (chain_info *) malloc(sizeof(chain_info));
#ifdef TEST_ALLOC
	++chaincount; ++chainmax; ++chainnew;
#endif
				chain2->length = chain->length;
				chain2->metric_length = chain->metric_length;
				chain2->used = 0;
				chain2->sols = (sol_info **) malloc(sizeof(sol_info *)*(box->num_sings+2));
				chain2->metLens = (float *) malloc(sizeof(float)*(box->num_sings+1));
				memcpy(chain2->sols,chain->sols,sizeof(sol_info *)*chain->length);
				memcpy(chain2->metLens,chain->metLens,sizeof(float)*(chain->length-1));
				chain2->sols[chain2->length++] = next_sol;
				
				dx = ((float) chain2->sols[chain2->length-2]->xl) / chain2->sols[chain2->length-2]->denom
					- ((float) chain2->sols[chain2->length-1]->xl) / chain2->sols[chain2->length-1]->denom;
				dy = ((float) chain2->sols[chain2->length-2]->yl) / chain2->sols[chain2->length-2]->denom
					- ((float) chain2->sols[chain2->length-1]->yl) / chain2->sols[chain2->length-1]->denom;
				dz = ((float) chain2->sols[chain2->length-2]->zl) / chain2->sols[chain2->length-2]->denom
					- ((float) chain2->sols[chain2->length-1]->zl) / chain2->sols[chain2->length-1]->denom;
				chain2->metLens[chain2->length-2] = sqrt( dx * dx + dy * dy + dz * dz);
				chain2->metric_length += chain2->metLens[chain2->length-2];

				chain2->next = box->chains;
				box->chains = chain2;
				if(next_sing != NULL) 
				{
					follow_chain(box,chain2,next_sing,next_nl);
				}
			}
			else
			{
				if(k!=chain->length)
				{
					sing = NULL;
					break;
				}
#ifdef PRINT_FOLLOW_CHAIN
fprintf(stderr,"Adding to existing chain\n");
#endif
				/* Just add to this chain */

				chain->sols[chain->length++] = next_sol;

				dx = ((float) chain->sols[chain->length-2]->xl) / chain->sols[chain->length-2]->denom
					- ((float) chain->sols[chain->length-1]->xl) / chain->sols[chain->length-1]->denom;
				dy = ((float) chain->sols[chain->length-2]->yl) / chain->sols[chain->length-2]->denom
					- ((float) chain->sols[chain->length-1]->yl) / chain->sols[chain->length-1]->denom;
				dz = ((float) chain->sols[chain->length-2]->zl) / chain->sols[chain->length-2]->denom
					- ((float) chain->sols[chain->length-1]->zl) / chain->sols[chain->length-1]->denom;
				chain->metLens[chain->length-2] = sqrt( dx * dx + dy * dy + dz * dz);
				chain->metric_length += chain->metLens[chain->length-2];

				nl = next_nl;
				sing = next_sing;

				if(sing==NULL) break;	/* reached the end of the chain */
				continue;
			}
		} /* end for i */

		if(sing==NULL) break;	/* reached the end of the chain */
	} /* end while */
}

/*****	Combining Facets Routines **************************************/

int sol_on_box_boundary_or_halfplane(box_info *box,sol_info *sol,soltype plane)
{
	int testX,testY,testZ;

	testX = (   sol->xl * box->denom == box->xl * sol->denom 
		 || sol->xl * box->denom == (box->xl+1) * sol->denom );
	testY = (   sol->yl * box->denom == box->yl * sol->denom 
		 || sol->yl * box->denom == (box->yl+1) * sol->denom );
	testZ = (   sol->zl * box->denom == box->zl * sol->denom 
		 || sol->zl * box->denom == (box->zl+1) * sol->denom );

	if(plane == X_AXIS)
		testY = testY
			 || 2 * sol->yl * box->denom == (2*box->yl+1) * sol->denom;
	if(plane == FACE_DD || plane == X_AXIS)
		testZ = testZ 
			 || 2 * sol->zl * box->denom == (2*box->zl+1) * sol->denom;

	switch(sol->type)
	{
	case X_AXIS:
		return testY && testZ;
	case Y_AXIS:
		return testX && testZ;
	case Z_AXIS:
		return testX && testY;
	default:
		fprintf(stderr,"sol_on_bny_hlafplane bad soltype %d %d\n",sol->type,plane);
		return 1;
	}
	return 0;
}

int sol_on_box_boundary(box_info *box,sol_info *sol)
{
	return sol_on_box_boundary_or_halfplane(box,sol,NONE);
}

void remove_sols_not_on_boundary(facet_info *facet1,box_info *box,soltype type)
{
	facet_sol *cur;

	for(cur = facet1->sols;cur!=NULL;cur=cur->next)
	{
		if(cur->sol->type <= Z_AXIS
		 && !sol_on_box_boundary_or_halfplane(box,cur->sol,type) )
		{
/*
fprintf(stderr,"removing_sol plane %d (%d,%d,%d)/%d\n",type,box->xl,box->yl,box->zl,box->denom);
print_sol(cur->sol);
*/
			remove_sol_from_facet(facet1,cur);
			remove_sols_not_on_boundary(facet1,box,type);
			return;
		}
	}
}
			
void remove_repeated_and_hanging_points(facet_info *facet1)
{
	facet_sol *prev, *cur, *next;
	int count=0;

	for(cur = facet1->sols;cur!=NULL;cur=cur->next)
	{	
		if(cur->next == NULL) prev = cur;
		++count; 
	}

	if(count<2) return;

	prev = NULL;
	for(cur = facet1->sols;cur!=NULL;cur=cur->next)
	{
		next = cur->next;
		if(next==NULL) next = facet1->sols;

		if(cur->sol == next->sol )
		{
			remove_sol_from_facet(facet1,cur);
			remove_repeated_and_hanging_points(facet1);
			return;
		}
		prev=cur;
	}
	if(count<3) return;
	for(cur = facet1->sols;cur!=NULL;prev=cur,cur=cur->next)
	{
		next = cur->next;
		if(next==NULL) next = facet1->sols;

		if(prev->sol == next->sol )
		{
			remove_sol_from_facet(facet1,cur);
			remove_repeated_and_hanging_points(facet1);
			return;
		}
	}
}

void fix_facets(facet_info *facets,box_info *box,soltype type)
{
	facet_info *f2;

	for(f2=facets;f2!=NULL;f2=f2->next)
	{
		remove_repeated_and_hanging_points(f2);
		remove_sols_not_on_boundary(f2,box,type);
	}
}

int is_node_link(sol_info *fs1,sol_info *fs2)
{
	switch(fs1->type)
	{
	case X_AXIS: case Y_AXIS: case Z_AXIS:
		return 0;
	default:
	}
	switch(fs2->type)
	{
	case X_AXIS: case Y_AXIS: case Z_AXIS:
		return 0;
	default:
	}
	return 1;
}

facet_sol *link_on_facet(facet_info * facet2,facet_sol *f1a,facet_sol *f1b)
{
	facet_sol *f2a,*f2b;

	for(f2a = facet2->sols;f2a!=NULL;f2a=f2a->next)
	{
		f2b = f2a->next; if(f2b==NULL) f2b=facet2->sols;
			
		if( f1a->sol == f2a->sol && f1b->sol == f2b->sol )
			return(f2a);
		if( f1a->sol == f2b->sol && f1b->sol == f2a->sol )
			return(f2a);
	}
	return NULL;
}
	
/** if facet1 and facet2 extend facet2 and return true. **/

facet_info *link_facet(facet_info *facet2,facet_info *facet1)
{
	facet_sol *f1a,*f1b,*f2a,*f2b,*fs1,*fs2;
	facet_info *facet3;
	int orientation = 0,include_next_point,missed_prev_point;

#ifdef PRINT_JOIN_FACETS
fprintf(stderr,"link_facet:\n");
#endif
	f2a = NULL;

	for(f1a = facet1->sols;f1a!=NULL;f1a=f1a->next)
	{
		f1b = f1a->next; if(f1b==NULL) f1b=facet1->sols;

		if(is_node_link(f1a->sol,f1b->sol))
			continue;
		f2a = link_on_facet(facet2,f1a,f1b);
		if(f2a!=NULL) break;
	}
	if(f2a==NULL) return 0;

#ifdef PRINT_JOIN_FACETS
fprintf(stderr,"linking_facet:\n");
print_facet(facet1);
print_facet(facet2);
#endif
	f2b = f2a->next; if(f2b==NULL) f2b=facet2->sols;
			
	if( f1a->sol == f2a->sol && f1b->sol == f2b->sol )
		orientation = -1;
	if( f1a->sol == f2b->sol && f1b->sol == f2a->sol )
		orientation = 1;

	/* now add all soln from facet2 add all sols from facet1
		if a sol is on a linking edge and a non linking edge
		add for facet2 but not for facet1 */

	facet3 = make_facet();
	fs1 = f2b;
	missed_prev_point = 1;
	while(TRUE)
	{
		fs2 = fs1->next; if(fs2==NULL) fs2=facet2->sols;
		if( /* is_node_link(fs1->sol,fs2->sol)
		 || */ link_on_facet(facet1,fs1,fs2) == NULL)
		{
			if(missed_prev_point)
				add_sol_to_facet(facet3,fs1->sol);
			add_sol_to_facet(facet3,fs2->sol);
			missed_prev_point = 0;
		}
		else
			missed_prev_point = 1;
		fs1 = fs2;
		if(fs1==f2a) break;
	}
	include_next_point = 0;
	fs1 = f1b;
	while(TRUE)
	{
		fs2 = fs1->next; if(fs2==NULL) fs2=facet1->sols;
		if( /* is_node_link(fs1->sol,fs2->sol)
		 || */ link_on_facet(facet2,fs1,fs2) == NULL)
		{
			if(include_next_point)
			{
				if(orientation==1)
					add_sol_to_facet(facet3,fs1->sol);
				else
					add_sol_to_facet_backwards(facet3,fs1->sol);
			}
			include_next_point = 1;
		}
		else
			include_next_point = 0;
		fs1 = fs2;
		if(fs1==f1a) break;
	}
#ifdef PRINT_JOIN_FACETS
fprintf(stderr,"linking_facet: done\n");
print_facet(facet3);
#endif
	return(facet3);
}

int inc_count;

facet_info *include_facet(facet_info *existing,facet_info *facet1)
{
	facet_info *facet2,*facet3;
	facet_sol *fs1;

#ifdef PRINT_JOIN_FACETS
fprintf(stderr,"inc_facet %d\n",inc_count);
print_facet(facet1);
#endif
	for(facet2=existing;facet2!=NULL;facet2=facet2->next)
	{
		if( (facet3 = link_facet(facet2,facet1) ) != NULL )
		{
#ifdef PRINT_JOIN_FACETS
fprintf(stderr,"found_link\n");
#endif
			existing = remove_facet_from_list(existing,facet2);
			return include_facet(existing,facet3);
		}
	}
#ifdef PRINT_JOIN_FACETS
fprintf(stderr,"not found_link\n");
#endif
	/* not found just add it */
	facet2 = make_facet();
	for(fs1=facet1->sols;fs1!=NULL;fs1=fs1->next)
		add_sol_to_facet(facet2,fs1->sol);
	facet2->next = existing;
	return facet2;
}

facet_info *include_facets(facet_info *facetlist,facet_info *boxfacets)
{
	facet_info *facet1;

	for(facet1=boxfacets;facet1!=NULL;facet1=facet1->next)
	{
#ifdef PRINT_JOIN_FACETS
		++inc_count;
		if(global_facet_count != -1 && inc_count == global_facet_count )
		{
			facet_sol *fs1;
			facet_info *facet2;
			print_facet(facet1);
			facet2 = make_facet();
			for(fs1=facet1->sols;fs1!=NULL;fs1=fs1->next)
			add_sol_to_facet(facet2,fs1->sol);
			facet2->next = facetlist;
			return facet2;
		}
		if(global_facet_count != -1 && inc_count > global_facet_count )
			return facetlist;
#endif
		facetlist = include_facet(facetlist,facet1);
	}
	return facetlist;
}

/*
 * Function:	combine_facets
 * Action:	combines all the facets in the sub boxes
 *		to form the facets of the main box.
 *		Removes the facets from the sub boxes.
 */

void combine_facets(box)
box_info *box;
{

#ifdef PRINT_COMBINE_FACETS
fprintf(stderr,"Combine facets (%d,%d,%d)/%d\n",box->xl,box->yl,box->zl,box->denom);
fprintf(stderr,"lfd "); print_facets(box->lfd->facets);
fprintf(stderr,"lfu "); print_facets(box->lfu->facets);
fprintf(stderr,"lbd "); print_facets(box->lbd->facets);
fprintf(stderr,"lbu "); print_facets(box->lbu->facets);
fprintf(stderr,"rfd "); print_facets(box->rfd->facets);
fprintf(stderr,"rfu "); print_facets(box->rfu->facets);
fprintf(stderr,"rbd "); print_facets(box->rbd->facets);
fprintf(stderr,"rbu "); print_facets(box->rbu->facets);
#endif
	inc_count=0;
	box->facets = NULL;
	box->facets = include_facets(box->facets,box->lbd->facets);
	box->facets = include_facets(box->facets,box->lbu->facets);
	box->facets = include_facets(box->facets,box->lfd->facets);
	box->facets = include_facets(box->facets,box->lfu->facets);
	box->facets = include_facets(box->facets,box->rbd->facets);
	box->facets = include_facets(box->facets,box->rbu->facets);
	box->facets = include_facets(box->facets,box->rfd->facets);
	box->facets = include_facets(box->facets,box->rfu->facets);

	fix_facets(box->facets,box,NONE);

	free_facet_list(box->lbd->facets);
	free_facet_list(box->lbu->facets);
	free_facet_list(box->lfd->facets);
	free_facet_list(box->lfu->facets);
	free_facet_list(box->rbd->facets);
	free_facet_list(box->rbu->facets);
	free_facet_list(box->rfd->facets);
	free_facet_list(box->rfu->facets);

#ifdef PRINT_COMBINE_FACETS
fprintf(stderr,"Combine facets done\n");
print_facets(box->facets);
#endif

}

/********** Construct cycles round the boundary of box **************/

int get_next_link(box_info *box,sol_info **presentsol,link_info **currentlink,soltype *cycle)
{
	link_info *link;
	link_info *alllinks,*matchinglinks[4];
	face_info *face;
	int count;

	/* A link so loop through all the links starting from... */

	if(*currentlink == NULL) return 0;

	link = (*currentlink)->next;

	do
	{
/*
print_link(link);
*/
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
			default:
				fprintf(stderr,"get_next_link: bad type %d\n",*cycle);
				return 0;
			}
		}
		else if( link == *currentlink )
		{
			fprintf(stderr,"gone all the way round\n");
#ifdef PRINT_FACET_ERR
			printbox(box);
#endif
			*currentlink = NULL;
			return 0;
		}
		else if( link->plotted )
		{
			link = link->next;
		}
		else if(link->A == *presentsol)
		{
			break;
		}
		else if( link->B == *presentsol )
		{
			break;
		}
		else
		{
			link = link->next;
		}

	} while(TRUE);  /* end do loop */

	/* May have the case 
		<>< i.e. a pair of links which link two nodes together 

		find all the matching links
	*/

	switch( *cycle )
	{
	case FACE_LL: face = box->ll; break;
	case FACE_RR: face = box->rr; break;
	case FACE_FF: face = box->ff; break;
	case FACE_BB: face = box->bb; break;
	case FACE_DD: face = box->dd; break;
	case FACE_UU: face = box->uu; break;
	default:
	}
	alllinks = face->links;
	count = 0;
	for(;alllinks != NULL;alllinks=alllinks->next)
	{
		if(alllinks->plotted) continue;
		if(alllinks->A == *presentsol || alllinks->B == *presentsol)
		{
			if(count<4)
			matchinglinks[count++] = alllinks;
		}
	}
	if(count >1 )
	{
#ifdef PRINT_FACET_ERR
		int i;
		fprintf(stderr,"get_next_link: Potential double link %d\n",count);
		print_link(*currentlink);
		for(i=0;i<count;++i)
			print_link(matchinglinks[i]);
#else
		fprintf(stderr,"get_next_link: Potential double link %d\n",count);
#endif
	}
	if(count==3)
	{
		if( matchinglinks[0]->A->type >= FACE_LL
		 && matchinglinks[0]->B->type >= FACE_LL )
		{
			link = matchinglinks[0];
		}
		else if( matchinglinks[1]->A->type >= FACE_LL
		      && matchinglinks[1]->B->type >= FACE_LL )
		{
			link = matchinglinks[1];
		}
		else if( matchinglinks[2]->A->type >= FACE_LL
		      && matchinglinks[2]->B->type >= FACE_LL )
		{
			link = matchinglinks[2];
		}
#ifdef PRINT_FACET_ERR
		fprintf(stderr,"Using link\n");
		print_link(link);
#endif
	}

	
	if(link->A == *presentsol)
	{
		link->plotted = 1;
		*presentsol = link->B;
		*currentlink = link;
	}
	else if( link->B == *presentsol )
	{
		link->plotted = 1;
		*presentsol = link->A;
		*currentlink = link;
	}
	else
	{
		fprintf(stderr,"get_next_link: wierdness\n");
		return 0;
	}
	return 1;
}

/*
 * Function:	plot_facet
 * action:	starting from a link, loop all the way round until
 *		you get back to the begining.
 */

void create_facet(box_info *box,link_info *startlink,soltype startcycle)
{
	link_info *link;
	soltype cycle;
	sol_info *startingsol,*presentsol;
	int direction;
	facet_info *f;

	/* Now have a link which has not been plotted */

	link = startlink;
	cycle = startcycle;
	direction = FORWARDS;
	f = add_facet();
	add_sol_to_facet(f,link->A);
	add_sol_to_facet(f,link->B);

	link->plotted = 1;
	startingsol = link->A;
	presentsol = link->B;

	while( presentsol != startingsol )
	{
/*
fprintf(stderr,"create_facet: "); printsoltype(cycle);
print_link(link);
print_sol(presentsol);
*/
		if(!get_next_link(box,&presentsol,&link,&cycle)) break;
		if(presentsol == startingsol ) break;

		add_sol_to_facet(f,presentsol);

	} /* end while loop */
}

void create_3node_link_facets(box_info *box)
{
	node_link_info *nl;
	int i;

	for(nl=box->node_links;nl!=NULL;nl=nl->next) ++i;
	if(i<3) return;
}
	
/********** Main entry point for routines *****************/

void fix_crossing_gaps(box_info *box);
void clean_facets(box_info *box);

void make_facets(box_info *box)
{
	link_info *link;
	soltype cycle;
	int finished = FALSE;

	all_facets = NULL;

	if(box->lfd != NULL)
	{
		make_facets(box->lfd);
		make_facets(box->lfu);
		make_facets(box->lbd);
		make_facets(box->lbu);
		make_facets(box->rfd);
		make_facets(box->rfu);
		make_facets(box->rbd);
		make_facets(box->rbu);
		combine_facets(box);
		clean_facets(box);
		return;
	}

	collect_sings(box);

	make_chains(box);

#ifdef PRINT_DRAW_BOX
	fprintf(stderr,"\nmake_facets: box (%d,%d,%d)/%d\n",
		box->xl,box->yl,box->zl,box->denom);
	print_box_brief(box);
	print_chains(box->chains);
#endif

	for(link=box->ll->links;link!=NULL;link=link->next)
		link->plotted = 0;
	for(link=box->rr->links;link!=NULL;link=link->next)
		link->plotted = 0;
	for(link=box->ff->links;link!=NULL;link=link->next)
		link->plotted = 0;
	for(link=box->bb->links;link!=NULL;link=link->next)
		link->plotted = 0;
	for(link=box->dd->links;link!=NULL;link=link->next)
		link->plotted = 0;
	for(link=box->uu->links;link!=NULL;link=link->next)
		link->plotted = 0;

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
			default:
				fprintf(stderr,"make_facets: bad type %d\n",cycle);
				break;

			}
			continue;
		}	/* Plot if not set */
		else if( !(link->plotted) )
			create_facet(box,link,cycle);
		
		link = link->next;
	} while( !finished );

#ifdef PRINT_DRAW_BOX
	fprintf(stderr,"Initial facets\n");
	print_facets(all_facets);
#endif
	/* Now divide up the facets */


	create_3node_link_facets(box);

	if(global_do_refine)
		refine_facets(box);

#ifdef PRINT_DRAW_BOX
	fprintf(stderr,"Final facets\n");
	print_facets(all_facets);
#endif

	box->facets = all_facets;
	all_facets = NULL;

	clean_facets(box);
}

/**
  * dose a final cleanup of facets
  * removes thin identicle facets
  * removes any sols not on boundary
  */

void clean_facets(box_info *box)
{
	int more_to_do;

	facet_info *facet1,*facet2;
	facet_sol *fs1,*fs2,*next,*prev;

	for(facet1=box->facets;facet1!=NULL;facet1=facet1->next)
	{
		prev=NULL;
		for(fs1=facet1->sols;fs1!=NULL;fs1=next)
		{
			next=fs1->next;
			if(fs1->sol->type <= Z_AXIS
			 && !sol_on_box_boundary(box,fs1->sol) )
			{
				if(prev==NULL) facet1->sols = next;
				else		prev->next = next;
			}
			else prev=fs1;
		}
	}

	more_to_do =1;
	while(more_to_do)
	{
	more_to_do = 0;
	for(facet1=box->facets;facet1!=NULL;facet1=facet1->next)
	{
		for(facet2=facet1->next;facet2!=NULL;facet2=facet2->next)
		{
			int matched_all_sols = 1;

			for(fs1=facet1->sols;fs1!=NULL;fs1=fs1->next)
			{
				int matched_sol=0;
				for(fs2=facet2->sols;fs2!=NULL;fs2=fs2->next)
					if(fs1->sol == fs2->sol)
					{	
						matched_sol = 1; 
						break;
					}
				if(!matched_sol)
				{
					matched_all_sols = 0;
					break;
				}
			}

			for(fs1=facet2->sols;fs1!=NULL;fs1=fs1->next)
			{
				int matched_sol=0;
				for(fs2=facet1->sols;fs2!=NULL;fs2=fs2->next)
					if(fs1->sol == fs2->sol)
					{	
						matched_sol = 1; 
						break;
					}
				if(!matched_sol)
				{
					matched_all_sols = 0;
					break;
				}
			}

			if(matched_all_sols)
			{
				/* now remove facet1 and facet2 from list */
				facet_info *facet3,*nextfacet,*prevfacet;
				prevfacet = NULL;
				for(facet3=box->facets;facet3!=NULL;facet3=nextfacet)
				{
					nextfacet=facet3->next;

					if(facet3==facet1 || facet3==facet2)
					{
						if(prevfacet==NULL) 
							box->facets = nextfacet;
						else
							prevfacet->next = nextfacet;
					}
					else
						prevfacet=facet3;
				}
				more_to_do = 1;
			}
			if(more_to_do) break;
		} /* end facet2 loop */
		if(more_to_do) break;
	} /* end facet1 loop */
	}
return;
	fix_crossing_gaps(box);
}

void fix_crossing_gaps(box_info *box)
{
	facet_info *facet1;
	facet_sol *fs1;
	int sol_count,i,j;
	sol_info **sol_array,**sol_replacements;

	/** Now want to close up a few gaps which can occur along self intersections.
		it may happen that there are two nodes which are adjacent
		on the same face (may be internal). We want to consolodate
		these nodes into a single one. Always pick the one with
		smallest x (or y (or z)) **/

	sol_count = 0;
	for(facet1=box->facets;facet1!=NULL;facet1=facet1->next)
		for(fs1=facet1->sols;fs1!=NULL;fs1=fs1->next)
			if(fs1->sol->type>=FACE_LL && fs1->sol->type <= FACE_UU)
				++sol_count;

	if(sol_count ==0) return;
	sol_array = (sol_info **) calloc(sol_count,sizeof(sol_info *));
	sol_replacements = (sol_info **) calloc(sol_count,sizeof(sol_info *));

	sol_count = 0;
	for(facet1=box->facets;facet1!=NULL;facet1=facet1->next)
		for(fs1=facet1->sols;fs1!=NULL;fs1=fs1->next)
			if(fs1->sol->type>=FACE_LL && fs1->sol->type <= FACE_UU)
			{
				int matched_sol=0;

				for(i=0;i<sol_count;++i)
				{
					if(sol_array[i] == fs1->sol)
						matched_sol = 1;
				}
				if(!matched_sol)
					sol_array[sol_count++] = fs1->sol;
			}

	for(i=0;i<sol_count;++i)
		for(j=i+1;j<sol_count;++j)
		{
			if( sol_array[i]->type == sol_array[j]->type
			 &&  sol_array[i]->denom == sol_array[j]->denom 
			 &&  abs(sol_array[i]->xl - sol_array[j]->xl) <=1
			 &&  abs(sol_array[i]->yl - sol_array[j]->yl) <=1
			 &&  abs(sol_array[i]->zl - sol_array[j]->zl) <=1 )
			{
				if(sol_array[i]->xl < sol_array[j]->xl )
					sol_replacements[j] = sol_array[i];
				else if(sol_array[j]->xl < sol_array[i]->xl )
					sol_replacements[i] = sol_array[j];
				else if(sol_array[i]->yl < sol_array[j]->yl )
					sol_replacements[j] = sol_array[i];
				else if(sol_array[j]->yl < sol_array[i]->yl )
					sol_replacements[i] = sol_array[j];
				else if(sol_array[i]->zl < sol_array[j]->zl )
					sol_replacements[j] = sol_array[i];
				else if(sol_array[j]->zl < sol_array[i]->zl )
					sol_replacements[i] = sol_array[j];
				else if(sol_array[i] < sol_array[j])	/* comparing pointers */
					sol_replacements[j] = sol_array[i];
				else
					sol_replacements[i] = sol_array[j];
			}
		}

	for(facet1=box->facets;facet1!=NULL;facet1=facet1->next)
		for(fs1=facet1->sols;fs1!=NULL;fs1=fs1->next)
			if(fs1->sol->type>=FACE_LL && fs1->sol->type <= FACE_UU)
				for(i=0;i<sol_count;++i)
					if(sol_array[i] == fs1->sol)
					{
						if(sol_replacements[i]!=NULL)
							fs1->sol = sol_replacements[i];
					}
	free(sol_array);
	free(sol_replacements);
}								
						
			
			
	
	

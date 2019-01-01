/*
 * File:	ridge_intersect.c
 * Function:	intersects a geometry using (un)oriented vec fields
 * Author;	Richard Morris
 * Date		22/4/93
 */

#include <eqn.h>
#include <stdio.h>
#include <math.h>
#include "../lsmp.h"
#include "../CVcommon.h"
#include "../jvx/jvx.h"
#include "../jvx/jvxCore.h"
#include "intersect.h"

/*
#define PRI_INFIN
*/
#define PRINT_VALS
#define PRINT_ERRS


/*
 * Function:	PolylistIntersect Intersect
 * Action:	Returns a Intersectped version of the Polylist
 */

void JvxIntersect(xml_tree *root,double (*fun)())
{
	struct geometry *geom;
	jvx_pointSet *ps;
	jvx_faceSet *fs;
	jvx_lineSet *ls;
	jvx_f	*p;
	double	*v;
	int	dim_p;
	int	i,j,k,count;
	Iedge	*edge[MAX_NUM_REDGES];
	Ipoint	*sols[MAX_NUM_RSOLS];
	Ipoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	if(root->type!=LSMP_GEOMETRY)
	{
		int i;
		for(i=0;i<root->n_child;++i)
			JvxIntersect(root->children[i],fun);
		return;
	}
fprintf(stderr,"JVXintersect\n");
	geom = root->u.geometry;
	Iedgebase = NULL;
	ps = root->u.geometry->pointSet->app_info;
	fs = root->u.geometry->faceSet->app_info;
	if(ps == NULL || fs == NULL)
	{
		report_error(MIDDLE_ERROR,"Could not find a pointset and a face set",601);
		exit(1);
	}
	dim_p = ps->point_dim;
/*
	fprintf(stderr,"PUI vert %d poly %d\n",pl->n_verts,pl->n_polys);
*/
	corners = (Ipoint *) calloc(ps->num_points,sizeof(Ipoint));
	for(i=0;i<ps->num_points;++i)
	{
		A = corners+i;
		v = ps->points + i * dim_p;
		A->x = *v;
		A->y = *(v+1);
		A->z = *(v+2);
		if(dim_p == 4)
			A->w = *(v+3);
		else
			A->w = 1;
		A->next = NULL;
		A->done = FALSE;
	}

	Ivert_cursize = 0;
	Ivert_allocsize += ALLOC_INC;
	Iverts = (HPoint3 *) calloc(Ivert_allocsize,sizeof(HPoint3) );

	for(i=0;i<fs->num_faces;++i)
	{
		p = fs->faces[i];

		if(p->num > MAX_NUM_REDGES )
		{
	fprintf(stderr,"Too many edges on face %d max I can cope with is %d\n",
				p->num, MAX_NUM_REDGES);
			continue;
		}

		/* first calc sols on edges */

		for(j=0;j<p->num;++j)
		{
			k = (j+1) % p->num;
			jcorn = corners + (p->v[j]);
			kcorn = corners + (p->v[k]);
			edge[j] = getIedge(jcorn,kcorn);
			if(edge[j] == NULL )
			{
				A = Iconverge(jcorn,kcorn,fun);
/*
if(A != NULL)
fprintf(stderr,"Sol found at (%f,%f) on line (%f,%f)..(%f,%f)\n",
	A->x,A->y,jcorn->x,jcorn->y,kcorn->x,kcorn->y);
else
fprintf(stderr,"No Sol found on line (%f,%f)..(%f,%f)\n",
	jcorn->x,jcorn->y,kcorn->x,kcorn->y);
*/
				edge[j] = addIedge(jcorn,kcorn,A);
			}
		}

		/* next count up how many sols on this face */

		count = 0;
		for(j=0;j<p->num;++j)
		{
			A = edge[j]->pt3;
			while( A != NULL )
			{
				if( count >= MAX_NUM_RSOLS )
				{
		fprintf(stderr,"Too many sols on face %d max %d\n",
			count, MAX_NUM_RSOLS );
					break;
				}
				sols[count] = A;
				++count;
				A = A->next;
			}
		}
		if(count == 2)
		{
			addIline(sols[0],sols[1]);
		}
		else if( count > 2)
		{
			A = (Ipoint *) malloc(sizeof(Ipoint));
			A->x = 0.0; A->y = 0.0;
			A->z = 0.0; A->w = 0.0;
			for(j=0;j<count;++j)
			{
				A->x += sols[j]->x;
				A->y += sols[j]->y;
			}
			A->x /= (double) count;
			A->y /= (double) count;
			A->z /= (double) count;
			A->w /= (double) count;
			for(j=0;j<count;++j)
			{
				addIline(sols[j],A);
			}
			free(A);
		}
	} 

	/* Done all faces: now construct the vect structure */

fprintf(stderr,"done sums\n");
	delete_child_from_jvx_tree(root,"pointSet");
	delete_child_from_jvx_tree(root,"faceSet");
	delete_child_from_jvx_tree(root,"primitive");
	delete_child_from_jvx_tree(root,"vectorField");
	delete_child_from_jvx_tree(root,"bndbox");
	delete_child_from_jvx_tree(root,"center");
	geom->pointSet = NULL;
	geom->faceSet = NULL;
	geom->primitive = NULL;
	geom->vectorField = NULL;
	geom->bndbox = NULL;
	geom->center = NULL;
	{
		double* my_points;
		int* my_lines;
		xml_tree *xml_points,*xml_lines,*xml_color;
		my_points = (double *) malloc(sizeof(double)*Ivert_cursize*dim_p);
		my_lines = (int *) malloc(sizeof(int)*Ivert_cursize);
		for(i=0;i<Ivert_cursize;++i)
		{
			*(my_points + i*dim_p) = (Iverts + i)->x;
			*(my_points + i*dim_p+1) = (Iverts + i)->y;
			*(my_points + i*dim_p+2) = (Iverts + i)->z;
			if(dim_p == 4)
				*(my_points + i*dim_p+3) = (Iverts + i)->w;
			*(my_lines + i) = i;
		}
		xml_points = create_pointSet_from_data(dim_p,Ivert_cursize,my_points);
		set_jvx_attribute(xml_points,"point","hide");

		xml_lines = create_lineSet_from_data(Ivert_cursize/2,my_lines);
		set_jvx_attribute(xml_lines,"line","show");

		if(global_cols >= 0)
		{
			xml_color = create_jvx_color_from_color_number(global_cols);
			add_sub_child_to_lineSet(xml_lines,xml_color);
		}
		add_child_to_geometry(root,xml_points);
		add_child_to_geometry(root,xml_lines);
	}

	edge[0] = Iedgebase;
	while(edge[0] != NULL)
	{
		edge[1] = edge[0]->next;
		sols[0] = edge[0]->pt3;
		while(sols[0] != NULL)
		{
			sols[1] = sols[0]->next;
			free(sols[0]);
			sols[0] = sols[1];
		}
		free(edge[0]);
		edge[0]=edge[1];
	}
	Iedgebase = NULL;
	free(corners);
}

PolyList *jvx2PolyList(xml_tree *root)
{
	PolyList *pl;
	struct geometry *geom;
	jvx_pointSet *ps;
	jvx_faceSet *fs;
	jvx_lineSet *ls;
	jvx_f	*p;
	double	*v;
	int	dim_p,dim_n,dim_c;
	int	i,j,k,count;
	Iedge	*edge[MAX_NUM_REDGES];
	Ipoint	*sols[MAX_NUM_RSOLS];
	Ipoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	if(root->type!=LSMP_GEOMETRY)
	{
		report_error(MIDDLE_ERROR,"jvx2Polylist: node must be of type geometry",601);
		exit(1);
	}

	geom = root->u.geometry;
	ps = root->u.geometry->pointSet->app_info;
	fs = root->u.geometry->faceSet->app_info;
	if(ps == NULL || fs == NULL)
	{
		report_error(MIDDLE_ERROR,"Could not find a pointset and a face set",601);
		exit(1);
	}

	dim_p = ps->point_dim;
	dim_c = ps->color_dim;
	dim_n = ps->normal_dim;
	if(dim_p<3 || dim_p >4)
	{
			report_error(MIDDLE_ERROR,"Bad dimension for point.",205);
			fprintf(stderr,"dim is %d\n",dim_p);
			return;
	}
	if(dim_n != 0 && dim_n != 3)
	{
		report_error(MIDDLE_ERROR,"Bad dimension for normals.",206);
		fprintf(stderr,"dim is %d\n",dim_n);
		return;
	}
	if(dim_c != 0 && dim_c != 1 && dim_c != 3 && dim_c != 4)
	{
		report_error(MIDDLE_ERROR,"Bad dimension for colors.",206);
		fprintf(stderr,"dim is %d\n",dim_c);
		return;
	}

/*
	fprintf(stderr,"PUI vert %d poly %d\n",pl->n_verts,pl->n_polys);
*/
	pl = (PolyList *) malloc(sizeof(PolyList));
	pl->n_verts = ps->num_points;
	pl->n_polys = fs->num_faces;
	pl->flags = 0;
	pl->seq = 0;

	pl->vl = (Vertex *) malloc(sizeof(Vertex) * ps->num_points);
	for(i=0;i<ps->num_points;++i)
	{
		pl->vl[i].pt.x = *(ps->points + i * dim_p);
		pl->vl[i].pt.y = *(ps->points + i * dim_p+1);
		pl->vl[i].pt.z = *(ps->points + i * dim_p+2);
		if(dim_p == 4)
			pl->vl[i].pt.w = *(ps->points + i * dim_p+3);
		else
			pl->vl[i].pt.w = 1.0;

		if(dim_n == 3)
		{
			pl->vl[i].vn.x = ps->normals[i * dim_n];
			pl->vl[i].vn.y = ps->normals[i * dim_n + 1];
			pl->vl[i].vn.z = ps->normals[i * dim_n + 2];
		}
		else pl->vl[i].vn.x = pl->vl[i].vn.y = pl->vl[i].vn.z = 0.0;

		if(dim_c == 1)
		{
			pl->vl[i].vcol.r = ps->colors[i * dim_c];
			pl->vl[i].vcol.g = ps->colors[i * dim_c];
			pl->vl[i].vcol.b = ps->colors[i * dim_c];
			pl->vl[i].vcol.a = 0.0;
		}
		if(dim_c == 3)
		{
			pl->vl[i].vcol.r = ps->colors[i * dim_c];
			pl->vl[i].vcol.g = ps->colors[i * dim_c +1];
			pl->vl[i].vcol.b = ps->colors[i * dim_c +2];
			pl->vl[i].vcol.a = 0.0;
		}
		if(dim_c == 4)
		{
			pl->vl[i].vcol.r = ps->colors[i * dim_c];
			pl->vl[i].vcol.g = ps->colors[i * dim_c +1];
			pl->vl[i].vcol.b = ps->colors[i * dim_c +2];
			pl->vl[i].vcol.b = ps->colors[i * dim_c +3];
		}
		else if(dim_c == 0)
			pl->vl[i].vcol.r = pl->vl[i].vcol.g = pl->vl[i].vcol.b = pl->vl[i].vcol.a = 0.0;
	}

	pl->p = (Poly *) malloc(sizeof(Poly) * fs->num_faces);
	for(i=0;i<fs->num_faces;++i)
	{
		pl->p[i].n_vertices = fs->faces[i]->num;
		pl->p[i].v = (Vertex **) malloc(sizeof(Vertex *) * fs->faces[i]->num);
		for(j=0;j<fs->faces[i]->num;++j)
		{
			pl->p[i].v[j] = pl->vl + fs->faces[i]->v[j];
		}
		pl->p[i].pn.x = 0.0;
		pl->p[i].pn.y = 0.0;
		pl->p[i].pn.z = 0.0;
		pl->p[i].pcol.r = 0.0;
		pl->p[i].pcol.g = 0.0;
		pl->p[i].pcol.b = 0.0;
		pl->p[i].pcol.a = 0.0;
	}
	return pl;
}

void FillJvxWithPolyList(xml_tree *root,PolyList *pl)
{
	struct geometry *geom;
	jvx_pointSet *ps;
	jvx_faceSet *fs;
	jvx_lineSet *ls;
	jvx_f	*p;
	double	*v;
	int	dim_p,dim_n,dim_c;
	int	i,j,k,count;
	Iedge	*edge[MAX_NUM_REDGES];
	Ipoint	*sols[MAX_NUM_RSOLS];
	Ipoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	if(root->type!=LSMP_GEOMETRY)
	{
		report_error(MIDDLE_ERROR,"jvx2Polylist: node must be of type geometry",601);
		exit(1);
	}

	geom = root->u.geometry;
	ps = root->u.geometry->pointSet->app_info;
	fs = root->u.geometry->faceSet->app_info;
	if(ps == NULL || fs == NULL)
	{
		report_error(MIDDLE_ERROR,"Could not find a pointset and a face set",601);
		exit(1);
	}

	dim_p = ps->point_dim;
	dim_c = ps->color_dim;
	dim_n = ps->normal_dim;
	if(dim_p<3 || dim_p >4)
	{
			report_error(MIDDLE_ERROR,"Bad dimension for point.",205);
			fprintf(stderr,"dim is %d\n",dim_p);
			return;
	}
	if(dim_n != 0 && dim_n != 3)
	{
		report_error(MIDDLE_ERROR,"Bad dimension for normals.",206);
		fprintf(stderr,"dim is %d\n",dim_n);
		return;
	}
	if(dim_c != 0 && dim_c != 1 && dim_c != 3 && dim_c != 4)
	{
		report_error(MIDDLE_ERROR,"Bad dimension for colors.",206);
		fprintf(stderr,"dim is %d\n",dim_c);
		return;
	}

/*
	fprintf(stderr,"PUI vert %d poly %d\n",pl->n_verts,pl->n_polys);
	free(ps->points);
	if(ps->colors!= NULL) free(ps->colors);
	if(ps->normals!= NULL) free(ps->normals);
	if(ps->textures!= NULL) free(ps->textures);
*/

	ps->num_points = pl->n_verts;
	ps->points = (double *) malloc(sizeof(double) * ps->num_points * dim_p);

	fs->num_faces = pl->n_polys;
	fs->faces = (jvx_f **) malloc(sizeof(jvx_f *) * fs->num_faces);

	if(dim_c == 1 || dim_c == 3 || dim_c == 4)
	{
		ps->num_colors = pl->n_verts;
		ps->colors = (int *) malloc(sizeof(int) * ps->num_colors * dim_c);
	}

	if(dim_n == 3)
	{
		ps->num_normals = pl->n_verts;
		ps->normals = (double *) malloc(sizeof(double) * ps->num_normals * dim_n);
	}

	for(i=0;i<ps->num_points;++i)
	{
		*(ps->points + i * dim_p) = pl->vl[i].pt.x;
		*(ps->points + i * dim_p+1) = pl->vl[i].pt.y;
		*(ps->points + i * dim_p+2) = pl->vl[i].pt.z;
		if(dim_p == 4)
			*(ps->points + i * dim_p+3) = pl->vl[i].pt.w;

		if(dim_n == 3)
		{
			ps->normals[i * dim_n] = pl->vl[i].vn.x;
			ps->normals[i * dim_n + 1] = pl->vl[i].vn.y;
			ps->normals[i * dim_n + 2] = pl->vl[i].vn.z;
		}

		if(dim_c == 1)
		{
			ps->colors[i * dim_c] = (pl->vl[i].vcol.r + pl->vl[i].vcol.g + pl->vl[i].vcol.b )/3;
		}
		if(dim_c == 3)
		{
			ps->colors[i * dim_c] = pl->vl[i].vcol.r;
			ps->colors[i * dim_c +1] = pl->vl[i].vcol.r;
			ps->colors[i * dim_c +2] = pl->vl[i].vcol.b;
		}
		if(dim_c == 4)
		{
			ps->colors[i * dim_c] = pl->vl[i].vcol.r;
			ps->colors[i * dim_c +1] = pl->vl[i].vcol.r;
			ps->colors[i * dim_c +2] = pl->vl[i].vcol.b;
			ps->colors[i * dim_c +3] = pl->vl[i].vcol.a;
		}
	}

	for(i=0;i<fs->num_faces;++i)
	{
		fs->faces[i] = (jvx_f *) malloc(sizeof(jvx_f));
		fs->faces[i]->num = pl->p[i].n_vertices;
		fs->faces[i]->v = (int *) malloc(sizeof(int) * fs->faces[i]->num);
		for(j=0;j<fs->faces[i]->num;++j)
		{
			fs->faces[i]->v[j] = pl->p[i].v[j] - pl->vl;
		}
	}
}


void JvxClip(xml_tree *root,double (*fun)())
{
	struct geometry *geom;
	PolyList *pl;
	jvx_pointSet *ps;
	jvx_faceSet *fs;
	jvx_lineSet *ls;
	jvx_f	*p;
	double	*v;
	int	dim_p;
	int	i,j,k,count;
	Iedge	*edge[MAX_NUM_REDGES];
	Ipoint	*sols[MAX_NUM_RSOLS];
	Ipoint	*corners,*A,*jcorn,*kcorn;
	short   *vert_per_vect,*coltab;
        ColorA  *colours;

	if(root->type!=LSMP_GEOMETRY)
	{
		int i;
		for(i=0;i<root->n_child;++i)
			JvxClip(root->children[i],fun);
		return;
	}
fprintf(stderr,"JvxClip\n");
	geom = root->u.geometry;
	ps = root->u.geometry->pointSet->app_info;
	fs = root->u.geometry->faceSet->app_info;
	if(ps == NULL || fs == NULL)
	{
		report_error(MIDDLE_ERROR,"Could not find a pointset and a face set",601);
		exit(1);
	}

	pl = jvx2PolyList(root);
	PolyListClipCore(pl,fun);
	FillJvxWithPolyList(root,pl);

	/* Done all faces: now construct the vect structure */

fprintf(stderr,"done sums\n");
	delete_child_from_jvx_tree(root,"primitive");
	delete_child_from_jvx_tree(root,"vectorField");
	delete_child_from_jvx_tree(root,"bndbox");
	delete_child_from_jvx_tree(root,"center");
}

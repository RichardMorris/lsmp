/*
 * File:	JvxMap.c
 * Function:	Applies a mapping to a JvXTree
 * Author;	Richard Morris
 * Date		22/4/93
 */

#include <stdio.h>
#include <math.h>
#include "../lsmp.h"
#include "../CVcommon.h"
#include "../jvx/jvx.h"
#include "../jvx/jvxCore.h"

extern double tolerance;
extern int global_cols;
extern int global_normals;
extern int global_dim;

void JvxMap(xml_tree *root)
{
	jvx_pointSet *ps;
	HPoint3 p;
	Point3 n;
	ColorA c;
	int dim_p,dim_n=0,dim_c=0,i;
	int *new_cols;

	if(root->type == LSMP_GEOMETRY)
	{
		if(root->u.geometry->faceSet != NULL)
		{
			delete_child_from_jvx_tree(root->u.geometry->faceSet,"normals");
			delete_child_from_jvx_tree(root->u.geometry->faceSet,"colors");
		}
		delete_child_from_jvx_tree(root,"primitive");
		delete_child_from_jvx_tree(root,"vectorField");
		delete_child_from_jvx_tree(root,"bndbox");
		delete_child_from_jvx_tree(root,"center");
	}

	if(root->type!=LSMP_POINTSET)
	{
		int i;
		for(i=0;i<root->n_child;++i)
			JvxMap(root->children[i]);
		return;
	}
	ps = root->app_info;
	dim_p = ps->point_dim;
	dim_n = ps->normal_dim;
	dim_c = ps->color_dim;
	if(global_cols == EQN_COL)
	{
		new_cols = (int *) malloc(sizeof(int) * 4 * ps->num_points);
	}

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

	/** Actually apply the mapping **/

	for(i=0;i<ps->num_points;++i)
	{
		p.x = ps->points[i * dim_p];
		p.y = ps->points[i * dim_p + 1];
		p.z = ps->points[i * dim_p + 2];
		if(dim_p==4)
			p.x = ps->points[i * dim_p +3];
		else
			p.w = 1.0;

		if(dim_n == 3)
		{
			n.x = ps->normals[i * dim_n];
			n.y = ps->normals[i * dim_n + 1];
			n.z = ps->normals[i * dim_n + 2];
		}
		else n.x = n.y = n.z = 0.0;

		if(global_cols == EQN_COL )
		{
		    if(dim_c == 1)
		    {
			c.r = (ps->colors[i * dim_c])/255.0;
			c.g = (ps->colors[i * dim_c])/255.0;
			c.b = (ps->colors[i * dim_c])/255.0;
			c.a = 0.0;
		    }
		    else if(dim_c == 3)
		    {
			c.r = (ps->colors[i * dim_c])/255.0;
			c.g = (ps->colors[i * dim_c +1])/255.0;
			c.b = (ps->colors[i * dim_c +2])/255.0;
			c.a = 0.0;
		    }
		    else if(dim_c == 4)
		    {
			c.r = (ps->colors[i * dim_c])/255.0;
			c.g = (ps->colors[i * dim_c +1])/255.0;
			c.b = (ps->colors[i * dim_c +2])/255.0;
			c.a = (ps->colors[i * dim_c +3])/255.0;
		    }
		    else
			c.r = c.g = c.b = c.a = 0.0;
		}
		else
			c.r = c.g = c.b = c.a = 0.0;

		/** Now apply the mapping **/
fprintf(stderr,"Before: %d (%f %f %f %f) %d (%f %f %f) %d (%f %f %f %f)\n",dim_p,p.x,p.y,p.z,p.w,
		dim_n,n.x,n.y,n.z,dim_c,c.r,c.g,c.b,c.a);
/*
*/
		PointMap(&p,&n,&c);
fprintf(stderr,"After: %d (%f %f %f %f) %d (%f %f %f) %d (%f %f %f %f)\n",dim_p,p.x,p.y,p.z,p.w,
		dim_n,n.x,n.y,n.z,dim_c,c.r,c.g,c.b,c.a);
/*
*/				

		ps->points[i * dim_p] = p.x;
		ps->points[i * dim_p + 1] = p.y;
		ps->points[i * dim_p + 2] = p.z;
		if(dim_p==4)
			ps->points[i * dim_p +3] = p.x;

		if(dim_n == 3)
		{
			ps->normals[i * dim_n] = n.x;
			ps->normals[i * dim_n + 1] = n.y;
			ps->normals[i * dim_n + 2] = n.z;
		}


		if(global_cols == EQN_COL )
		{
			new_cols[i * 4]     = (int) (c.r * 255);
			new_cols[i * 4 + 1] = (int) (c.g * 255);
			new_cols[i * 4 + 2] = (int) (c.b * 255);
			new_cols[i * 4 + 3] = (int) (c.a * 255);
		}

	}

	if(global_cols == EQN_COL)
	{
		ps->colors = new_cols;
		ps->color_dim = 4;
		ps->num_colors = ps->num_points;
		delete_child_from_jvx_tree(root,"colorTag");
		delete_child_from_jvx_tree(root,"color");

		root->u.pointSet->points->u.points->colorTag = NULL;
		root->u.pointSet->points->u.points->color = NULL;
		set_jvx_attribute(root,"color","show");
	}
}

#include <stdio.h>
#include "cells.h"

extern region_info region;
extern double (*global_fun)();
extern double (*global_df_dx)();
extern double (*global_df_dy)();
extern double (*global_df_dz)();


/******** Some evaluation subs *****************/

double EvalVertex(vert_info *vert)
{
	double x,y,z;
	double	val;

	if(vert->status & FOUND_VAL ) return(vert->val);
	x = region.xmin + (region.xmax-region.xmin) *
		((double)vert->xl) / vert->denom;
	y = region.ymin + (region.ymax-region.ymin) *
		((double)vert->yl) / vert->denom;
	z = region.zmin + (region.zmax-region.zmin) *
		((double)vert->zl) / vert->denom;
	vert->val = global_fun(x,y,z);
	vert->status |= FOUND_VAL;
	return(vert->val);
}

double EvalVertexDerivX(vert_info *vert)
{
	double x,y,z;
	double	val;

	if(vert->status & FOUND_DX ) return(vert->dx);
	x = region.xmin + (region.xmax-region.xmin) *
		((double)vert->xl) / vert->denom;
	y = region.ymin + (region.ymax-region.ymin) *
		((double)vert->yl) / vert->denom;
	z = region.zmin + (region.zmax-region.zmin) *
		((double)vert->zl) / vert->denom;
	vert->dx = global_df_dx(x,y,z);
	vert->status |= FOUND_DX;
	return(vert->dx);
}

double EvalVertexDerivY(vert_info *vert)
{
	double x,y,z;
	double	val;

	if(vert->status & FOUND_DY ) return(vert->dy);
	x = region.xmin + (region.xmax-region.xmin) *
		((double)vert->xl) / vert->denom;
	y = region.ymin + (region.ymax-region.ymin) *
		((double)vert->yl) / vert->denom;
	z = region.zmin + (region.zmax-region.zmin) *
		((double)vert->zl) / vert->denom;
	vert->dy = global_df_dy(x,y,z);
	vert->status |= FOUND_DY;
	return(vert->dy);
}

double EvalVertexDerivZ(vert_info *vert)
{
	double x,y,z;
	double	val;

	if(vert->status & FOUND_DZ ) return(vert->dz);
	x = region.xmin + (region.xmax-region.xmin) *
		((double)vert->xl) / vert->denom;
	y = region.ymin + (region.ymax-region.ymin) *
		((double)vert->yl) / vert->denom;
	z = region.zmin + (region.zmax-region.zmin) *
		((double)vert->zl) / vert->denom;
	vert->dz = global_df_dz(x,y,z);
	vert->status |= FOUND_DZ;
	return(vert->dz);
}

double EvalEdge(edge_info *edge,double lam)
{
	double x,y,z;
	double	val;

	if( lam == 0.0 ) return(EvalVertex(edge->low));
	if( lam == 1.0 ) return(EvalVertex(edge->high));

	switch( edge->type )
	{
	case X_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			(edge->xl + lam) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Y_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl + lam) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Z_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl+lam) / edge->denom;
		break;
	}
	val = global_fun(x,y,z);
	if( val != val )
		fprintf(stderr,"f(%f,%f,%f) = NaN \n",x,y,z);
	return(val);
}

double EvalEdgeDerivX(edge_info *edge,double lam)
{
	double x,y,z;
	double	val;

	if( lam == 0.0 ) return(EvalVertexDerivX(edge->low));
	if( lam == 1.0 ) return(EvalVertexDerivX(edge->high));
	switch( edge->type )
	{
	case X_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			(edge->xl + lam) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Y_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl + lam) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Z_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl+lam) / edge->denom;
		break;
	}
	val = global_df_dx(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fx(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalEdgeDerivY(edge_info *edge,double lam)
{
	double x,y,z;
	double	val;

	if( lam == 0.0 ) return(EvalVertexDerivY(edge->low));
	if( lam == 1.0 ) return(EvalVertexDerivY(edge->high));
	switch( edge->type )
	{
	case X_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			(edge->xl + lam) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Y_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl + lam) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Z_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl+lam) / edge->denom;
		break;
	}
	val = global_df_dy(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fy(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalEdgeDerivZ(edge_info *edge,double lam)
{
	double x,y,z;
	double	val;

	if( lam == 0.0 ) return(EvalVertexDerivZ(edge->low));
	if( lam == 1.0 ) return(EvalVertexDerivZ(edge->high));
	switch( edge->type )
	{
	case X_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			(edge->xl + lam) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Y_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl + lam) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl) / edge->denom;
		break;
	case Z_AXIS:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)edge->xl) / edge->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)edge->yl) / edge->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)edge->zl+lam) / edge->denom;
		break;
	}
	val = global_df_dz(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fz(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalFace(face_info *face,double lam, double lam2)
{
	double x,y,z;
	double	val;

	switch( face->type )
	{
	case FACE_LL: case FACE_RR:
		x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam2) / face->denom;
		break;
	case FACE_FF: case FACE_BB:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl + lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl ) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl + lam2) / face->denom;
		break;
	case FACE_DD: case FACE_UU:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl +lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl + lam2) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam) / face->denom;
		break;
	}
	val = global_fun(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"f(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalFaceDerivX(face_info *face,double lam, double lam2)
{
	double x,y,z;
	double	val;

	switch( face->type )
	{
	case FACE_LL: case FACE_RR:
		x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam2) / face->denom;
		break;
	case FACE_FF: case FACE_BB:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl + lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl ) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl + lam2) / face->denom;
		break;
	case FACE_DD: case FACE_UU:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl +lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl + lam2) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam) / face->denom;
		break;
	}
	val = global_df_dx(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fx(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalFaceDerivY(face_info *face,double lam, double lam2)
{
	double x,y,z;
	double	val;

	switch( face->type )
	{
	case FACE_LL: case FACE_RR:
		x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam2) / face->denom;
		break;
	case FACE_FF: case FACE_BB:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl + lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl ) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl + lam2) / face->denom;
		break;
	case FACE_DD: case FACE_UU:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl +lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl + lam2) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam) / face->denom;
		break;
	}
	val = global_df_dy(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fy(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalFaceDerivZ(face_info *face,double lam, double lam2)
{
	double x,y,z;
	double	val;

	switch( face->type )
	{
	case FACE_LL: case FACE_RR:
		x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam2) / face->denom;
		break;
	case FACE_FF: case FACE_BB:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl + lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl ) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl + lam2) / face->denom;
		break;
	case FACE_DD: case FACE_UU:
		x = region.xmin + (region.xmax-region.xmin) *
			((double)face->xl +lam) / face->denom;
		y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl + lam2) / face->denom;
		z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam) / face->denom;
		break;
	}
	val = global_df_dz(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fz(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalBox(face_info *face,double lam, double lam2,double lam3)
{
	double x,y,z;
	double	val;

	x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl+lam) / face->denom;
	y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam2) / face->denom;
	z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam3) / face->denom;
	val = global_fun(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"f(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalBoxDerivX(face_info *face,double lam, double lam2,double lam3)
{
	double x,y,z;
	double	val;

	x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl+lam) / face->denom;
	y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam2) / face->denom;
	z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam3) / face->denom;
	val = global_df_dx(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fx(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalBoxDerivY(face_info *face,double lam, double lam2,double lam3)
{
	double x,y,z;
	double	val;

	x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl+lam) / face->denom;
	y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam2) / face->denom;
	z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam3) / face->denom;
	val = global_df_dy(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fy(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

double EvalBoxDerivZ(face_info *face,double lam, double lam2,double lam3)
{
	double x,y,z;
	double	val;

	x = region.xmin + (region.xmax-region.xmin) *
			((double) face->xl+lam) / face->denom;
	y = region.ymin + (region.ymax-region.ymin) *
			((double)face->yl+lam2) / face->denom;
	z = region.zmin + (region.zmax-region.zmin) *
			((double)face->zl+lam3) / face->denom;
	val = global_df_dz(x,y,z);
	if( val != val )
	{
		fprintf(stderr,"fz(%f,%f,%f) = NaN \n",x,y,z);
		val = 0.0;
	}
	return(val);
}

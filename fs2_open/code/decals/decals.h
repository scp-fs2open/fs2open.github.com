//header file for makeing decals-Bobboau
 
#ifndef _DECAL_H
#define _DECAL_H

#define MAX_DECAL_POLY 20
#define MAX_DECAL_POLY_POINT 10
#define MAX_DECAL_DEFPOINTS 1000
#define MAX_DECAL_POINT 10
#define MAX_SHIP_DECALS 25
#include "globalincs/pstypes.h"
#include "object/object.h"

#define DP_EOF 			0
#define DP_DEFPOINTS 	1
#define DP_FLATPOLY		2
#define DP_TMAPPOLY		3
#define DP_SORTNORM		4
#define DP_BOUNDBOX		5

typedef struct decal_model{	//this will store a triangulated version of the ships polygons
	struct{
		int point[3];		//an index to the vert list-Bobboau
		vector center;		//the center of the poly
		float radius;		//the max radius of the poly
	}poly[MAX_DECAL_POLY];
	vector vert[MAX_DECAL_DEFPOINTS];	//the defpoints-Bobboau
	int n_polys;
}decal_model;

//extern decal_model d_model;

typedef struct decal{
	struct{
		int backfaced;		//flag that this poly uses the backfaced texture
		uv_pair uv[MAX_DECAL_POLY_POINT];		//uv coordanants
		int point[MAX_DECAL_POLY_POINT];		//an index to the vert list-Bobboau
		int n_poly;
		vector norm;		//the normal used for lighting
	}poly[MAX_DECAL_POLY];
	vector position;
	float radius;
	int submodel_parent;	//the submodel this is part of
	vector vert[1000];		//the defpoints-Bobboau
	int frames;				//number of frames in an animation
	int fps;				//frames per second
	int starting_keyframe;	//the frame that indicates the end of the inital nonlooped portion and the start of the middle looped part
	int ending_keyframe;	//the frame that indicates the end of the loopable portion and the start of the end part
	int texture;			//the texture to be used, first in animation
	int backfaced_texture;	//the texture to be used by backfaced polys, first in animation
	int is_valid;			//this means it is a good decal that should be rendered-Bobboau
	int n_poly;				//the number of polys
	int timestamp;			//this is when the decal was made-Bobboau
	int lifetime;			//this is how long the decal should be alive for-Bobboau
	int importance;			//this is for big things large beam marks and bomb craters should last longer than the odd laser impacts-Bobboau
	int state;				//1 starting, 2 looping(mostly for beams), 3 ending, 4 static
}decal;

typedef struct decal_point{
vector pnt;
matrix orient;
float radius;
}decal_point;

int decal_create(object * obj, decal_point *point, int subobject, int texture, int backfaced_texture = -1);//makes the decal

#endif

int decal_create_simple(object * obj, decal_point *point, int texture);//makes a simple non-clipped decal

void decal_render_all(object * obj);	//renders all decals

int decal_create(object * obj, decal_point *point, int subobject, int texture, int backfaced_texture);//makes the decal

int decal_create_sub(void *model_ptr);

/*

int decal_is_in_face(int n, vertex vertlist[n]);//sees if a poly is inside a decal: 0 is yes, 1 is sort of, -1 is no
*/
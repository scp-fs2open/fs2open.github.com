//header file for making decals-Bobboau
 
#include "PreProcDefines.h"

#ifndef _DECAL_H
#define _DECAL_H


#include "object/object.h"

//#define DECALS_ENABLED

#define MAX_DECAL_POLY 40
#define MAX_DECAL_POLY_POINT 10
#define MAX_DECAL_DEFPOINTS 10000
#define MAX_DECAL_POINT 10
#define MAX_SHIP_DECALS 25
#define MAX_GLOBAL_DECAL_POLYS 11000
#define QUICKFIND_SUBDIVISION 100
#define QUICKFIND_SEGMENT_LENGTH (MAX_GLOBAL_DECAL_POLYS/QUICKFIND_SUBDIVISION)
#define MAX_GLOBAL_DECALS 5000

#include "globalincs/pstypes.h"
//#include "ship/ship.h"

#define DP_EOF 			0
#define DP_DEFPOINTS 	1
#define DP_FLATPOLY		2
#define DP_TMAPPOLY		3
#define DP_SORTNORM		4
#define DP_BOUNDBOX		5

struct decal;
struct decal_list;
struct decal_list_controle;

struct decal_poly{
	vector norm[3];
	vertex point[3];
	decal_poly *next;
	decal* parent;
	int idx;
};

struct decal{
	int n_poly;
	int timestamp;	//when this decal was created
	int burn_time;	//how long it takes for the glow texture to fade
	int vertex_buffer_start;	//were is the start of this decal's polys in the vertex buffer
	decal_poly* poly;
	decal_poly* add_poly();
	decal_poly* get_last_poly();
	void remove_last_poly();
	decal_list_controle *parent;
};

struct decal_list{
	decal dec;
	decal_list *next;
};

struct decal_item{
	decal_list dec;
	bool used;
};

struct decal_list_controle{
	int n_decals;
	int buffer;
	int texture;
	int n_poly;
	int burn_texture;
	int glow_texture;
	bool modifyed;
	decal_list *d_list;
	decal_list *end;
	decal_list *add_new();
	void trim();
	void clear();
	~decal_list_controle();
	decal_list_controle();
};

struct decal_system{
	decal_list_controle *decals;
	int n_decal_textures;
	bool decals_modified;
	int max_decals;
	decal_system():decals(NULL),n_decal_textures(0),decals_modified(false){};
};


//this is used to give all the data to the generation data to the functions that make them

struct decal_point{
	vector pnt;
	matrix orient;
	float radius;
};

int decal_create(object * obj, decal_point *point, int subobject, int texture, int backfaced_texture = -1, int glow_texture = -1, int burn_texture = -1, int burn_time = 1000);//makes the decal

int decal_create_simple(object * obj, decal_point *point, int texture);//makes a simple non-clipped decal

void decal_render_all(object * obj);	//renders all decals

int decal_create_sub(void *model_ptr);

void clear_decals(decal_system	*system);

#endif

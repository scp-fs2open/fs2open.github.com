/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include <map>
#include "globalincs/pstypes.h"
#include "graphics/grbatch.h"
#include "graphics/2d.h"
#include "cmdline/cmdline.h"
#include "render/3d.h"



typedef struct {
	int start, length;
	int flags;
	int bitmap;
} BatchNode;

const int BATCH_MAX_VERTEX = 500;
const int BATCH_MAX = 50;

vertex *Batch_vertex_array	= NULL;
BatchNode *Batch_array		= NULL;
bool Batch_in_process = false;
int  Batch_current = 0;
int  Batch_vertex_current = 0;

bool batch_init()
{
	if(!Cmdline_batch_3dunlit) return true;

	Batch_vertex_array	= (vertex *)	malloc(sizeof(vertex) * BATCH_MAX_VERTEX);
	Batch_array			= (BatchNode *) malloc(sizeof(vertex) * BATCH_MAX);

	return (Batch_array != 0); 
}

void batch_deinit()
{
	if(!Cmdline_batch_3dunlit) return;

	if (Batch_array != NULL) {
		free(Batch_array);
		Batch_array = NULL;
	}

	if (Batch_vertex_array != NULL) {
		free(Batch_vertex_array);
		Batch_vertex_array = NULL;
	}
}

void batch_start()
{
	Batch_in_process = true;
	Batch_current = 0;
	Batch_vertex_current = 0;
}

void batch_end()
{
	Batch_in_process = false;
}

void batch_render()
{
	if(!Cmdline_batch_3dunlit) return;
	if(Batch_current == 0) return;

	// Sort
	// Batch up again
	// Send to renderer
	for(int i = 0; i < Batch_current; i++)
	{
		vertex *vlist = &Batch_vertex_array[Batch_array[i].start];

		gr_set_bitmap(Batch_array[i].bitmap);

		gr_tmapper_batch_3d_unlit(Batch_array[i].length, vlist, Batch_array[i].flags);  
	}

	Batch_current = 0;
	Batch_vertex_current = 0;		  
}

vertex *batch_get_block(int num_verts, int flags)
{
	if(!Cmdline_batch_3dunlit) return NULL;

	if(Batch_vertex_array == NULL || Batch_array == NULL) {
		Assert(0);
		return NULL;
	}

	//int vcurrent = 0;

	// We've run out of vertex slots!
	if(num_verts >= (BATCH_MAX_VERTEX - Batch_vertex_current))
	{
		batch_render();
	}

	// Not the first batch
	if(Batch_current > 0)
	{
		int last_batch = Batch_current - 1;

		// Can add to current batch
		if(	Batch_array[last_batch].flags  == flags && 
			Batch_array[last_batch].bitmap == gr_screen.current_bitmap)
		{
			int pos = Batch_array[last_batch].start + Batch_array[last_batch].length;
			vertex *block = &Batch_vertex_array[pos];
			Batch_array[last_batch].length += num_verts;
			Batch_vertex_current += num_verts;
			return block; 
		}
	}	

	// We've run out of batch slots!
	if(Batch_current >= BATCH_MAX)
	{
		batch_render();
	}

	// Make a new batch
	Batch_array[Batch_current].flags  = flags;
	Batch_array[Batch_current].start  = Batch_vertex_current;
	Batch_array[Batch_current].length = num_verts;
	Batch_array[Batch_current].bitmap = gr_screen.current_bitmap;
	vertex *block = &Batch_vertex_array[Batch_array[Batch_current].start];
	Batch_current++;

	Batch_vertex_current += num_verts;
	return block;
}


//I noticed this file after implementing this, and figured it would be better to put my batcher in here -Bobboau


/*
class geometry_batcher{
	int n_to_render;
	int n_allocated;
	vertex* vert;
	vertex** vert_list;//V's stupid rendering functions need this
}
*/

void geometry_batcher::allocate_internal(int n_verts){
	//this is called to start a batch, you make sure you have enough memory 
	//to store all the geometry, 
	//then you clear out the memory and set the number of primitives to 0
	if(n_verts>n_allocated){
		if(vert)free(vert);
		if(vert_list)free(vert_list);
		vert = (vertex*)malloc(sizeof(vertex)*n_verts);
		vert_list = (vertex**)malloc(sizeof(vertex*)*n_verts);
		for(int i = 0; i<n_verts; i++)vert_list[i] = &vert[i];
		memset(vert,0,sizeof(vertex)*n_verts);
		n_allocated = n_verts;
	}
	n_to_render = 0;
}

void geometry_batcher::allocate(int quad, int n_tri){
	int to_aloc = 0;
	to_aloc += quad*6;
	//quads have two triangles, therefore six verts
	to_aloc += n_tri*3;
	//a single triangle has a mere 3 verts

	allocate_internal(to_aloc);

}

void geometry_batcher::add_alocate(int quad, int n_tri){
	int to_aloc = n_to_render*3;
	to_aloc += quad*6;
	//quads have two triangles, therefore six verts
	to_aloc += n_tri*3;
	//a single triangle has a mere 3 verts

	vertex* old_vert = vert;

	if(to_aloc>n_allocated){
		if(vert_list)free(vert_list);

		vert = (vertex*)malloc(sizeof(vertex)*to_aloc);
		vert_list = (vertex**)malloc(sizeof(vertex*)*to_aloc);

		for(int i = 0; i<to_aloc; i++)vert_list[i] = &vert[i];
		memset(vert,0,sizeof(vertex)*to_aloc);

		if(old_vert){
			memcpy(vert,old_vert,sizeof(vertex)*n_to_render*3);
			free(old_vert);
		}
		n_allocated = to_aloc;
	}
}

/*
0----1
|\   |
|  \ |
3----2
*/
void geometry_batcher::draw_bitmap(vertex *pnt, float rad, float depth){
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

	vector PNT;
	vm_vert2vec(pnt, &PNT);
	vector p[4];
	vertex *P = &vert[n_to_render*3];
	vector fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize(&fvec);
	//get the direction from the point to the eye

	uvec = View_matrix.vec.uvec;

//get an up vector in the general direction of what we want

	vm_vec_crossprod(&rvec, &fvec, &uvec);
	vm_vec_normalize(&rvec);
//make a right vector from the f and up vector, this r vec is exactly what we want, so...
	vm_vec_crossprod(&uvec, &fvec, &rvec);
//fix the u vec with it

	vm_vec_scale_add(&PNT, &PNT, &fvec, depth);
//move the center of the sprite based on the depth parameter
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);
	//move one of the verts to the left
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);
	//and one to the right

	//now move all oof the verts to were they need to be
	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);


	//move all the data from the vecs into the verts
	//tri 1
	g3_transfer_vertex(&P[5], &p[3]);
	g3_transfer_vertex(&P[4], &p[2]);
	g3_transfer_vertex(&P[3], &p[1]);

	//tri 2
	g3_transfer_vertex(&P[2], &p[3]);
	g3_transfer_vertex(&P[1], &p[1]);
	g3_transfer_vertex(&P[0], &p[0]);

	//set up the UV coords
	//tri 1
	P[5].u = 0.0f;	P[5].v = 0.0f;
	P[4].u = 1.0f;	P[4].v = 0.0f;
	P[3].u = 1.0f;	P[3].v = 1.0f;

	//tri 2
	P[2].u = 0.0f;	P[2].v = 0.0f;
	P[1].u = 1.0f;	P[1].v = 1.0f;
	P[0].u = 0.0f;	P[0].v = 1.0f;

	for(int i = 0; i<6 ; i++){
		P[i].r = pnt->r;
		P[i].g = pnt->g;
		P[i].b = pnt->b;
		P[i].a = pnt->a;
	}

	n_to_render += 2;
}

void geometry_batcher::draw_bitmap(vertex *pnt, float rad, float angle, float depth){
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

	extern float Physics_viewer_bank;
	angle+=Physics_viewer_bank;
	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;

	vector PNT;
	vm_vert2vec(pnt, &PNT);
	vector p[4];
	vertex *P = &vert[n_to_render*3];
	vector fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize(&fvec);

	vm_rot_point_around_line(&uvec, &View_matrix.vec.uvec, angle, &vmd_zero_vector, &fvec);
//	uvec = View_matrix.vec.uvec;

	vm_vec_crossprod(&rvec, &fvec, &uvec);
	vm_vec_normalize(&rvec);
	vm_vec_crossprod(&uvec, &fvec, &rvec);

	vm_vec_scale_add(&PNT, &PNT, &fvec, depth);
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);


	//move all the data from the vecs into the verts
	//tri 1
	g3_transfer_vertex(&P[5], &p[3]);
	g3_transfer_vertex(&P[4], &p[2]);
	g3_transfer_vertex(&P[3], &p[1]);

	//tri 2
	g3_transfer_vertex(&P[2], &p[3]);
	g3_transfer_vertex(&P[1], &p[1]);
	g3_transfer_vertex(&P[0], &p[0]);

	//set up the UV coords
	//tri 1
	P[5].u = 0.0f;	P[5].v = 0.0f;
	P[4].u = 1.0f;	P[4].v = 0.0f;
	P[3].u = 1.0f;	P[3].v = 1.0f;

	//tri 2
	P[2].u = 0.0f;	P[2].v = 0.0f;
	P[1].u = 1.0f;	P[1].v = 1.0f;
	P[0].u = 0.0f;	P[0].v = 1.0f;

	for(int i = 0; i<6 ; i++){
		P[i].r = pnt->r;
		P[i].g = pnt->g;
		P[i].b = pnt->b;
		P[i].a = pnt->a;
	}

	n_to_render += 2;
}

void geometry_batcher::draw_tri(vertex* verts){

	vertex *P = &vert[n_to_render*3];

	for(int i = 0; i<3; i++)
		P[i] = verts[i];

	n_to_render += 1;
}

void geometry_batcher::draw_quad(vertex* verts){

	vertex *P = &vert[n_to_render*3];

	P[0] = verts[0];
	P[1] = verts[1];
	P[2] = verts[2];

	P[0] = verts[0];
	P[2] = verts[2];
	P[3] = verts[3];

	n_to_render += 2;
}


void geometry_batcher::draw_beam(vector*start,vector*end, float width, float intinsity){
	vector p[4];
	vertex *P = &vert[n_to_render*3];

	vector fvec, uvecs, uvece, evec;

	vm_vec_sub(&fvec, end, start);
	vm_vec_normalize(&fvec);

	vm_vec_sub(&evec, &View_position, start);
	vm_vec_normalize(&evec);

	vm_vec_crossprod(&uvecs, &evec, &fvec);

	vm_vec_sub(&evec, &View_position, end);
	vm_vec_normalize(&evec);

	vm_vec_crossprod(&uvece, &evec, &fvec);


	vm_vec_scale_add(&p[0], start, &uvecs, width);
	vm_vec_scale_add(&p[1], end, &uvece, width);
	vm_vec_scale_add(&p[2], end, &uvece, -width);
	vm_vec_scale_add(&p[3], start, &uvecs, -width);


	//move all the data from the vecs into the verts
	//tri 1
	g3_transfer_vertex(&P[0], &p[3]);
	g3_transfer_vertex(&P[1], &p[2]);
	g3_transfer_vertex(&P[2], &p[1]);

	//tri 2
	g3_transfer_vertex(&P[3], &p[3]);
	g3_transfer_vertex(&P[4], &p[1]);
	g3_transfer_vertex(&P[5], &p[0]);

	//set up the UV coords
	//tri 1
	P[0].u = 0.0f;	P[0].v = 0.0f;
	P[1].u = 1.0f;	P[1].v = 0.0f;
	P[2].u = 1.0f;	P[2].v = 1.0f;

	//tri 2
	P[3].u = 0.0f;	P[3].v = 0.0f;
	P[4].u = 1.0f;	P[4].v = 1.0f;
	P[5].u = 0.0f;	P[5].v = 1.0f;

	for(int i = 0; i<6 ; i++){
		P[i].r = 255 * intinsity;
		P[i].g = 255 * intinsity;
		P[i].b = 255 * intinsity;
		P[i].a = 255 * intinsity;
	}

	n_to_render += 2;
}

float geometry_batcher::draw_laser(vector *p0,float width1,vector *p1,float width2, int r, int g, int b){

	width1 *= 0.5f;
	width2 *= 0.5f;

	vector uvec, fvec, rvec, center, reye;

	vm_vec_sub( &fvec, p0, p1 );
	vm_vec_normalize_safe( &fvec );

	vm_vec_avg( &center, p0, p1 ); //needed for the return value only
	vm_vec_sub(&reye, &Eye_position, &center);
	vm_vec_normalize(&reye);

	vm_vec_crossprod(&uvec,&fvec,&reye);
	vm_vec_normalize(&uvec);
	vm_vec_crossprod(&fvec,&uvec,&reye);
	vm_vec_normalize(&fvec);
	
	 
	//now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&rvec,&uvec,&fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	int i;

	vector start, end;

	vm_vec_scale_add(&start, p0, &fvec, -width1);
	vm_vec_scale_add(&end, p1, &fvec, width2);

	vector vecs[4];

	vertex *pts = &vert[n_to_render*3];

	vm_vec_scale_add( &vecs[0], &end, &uvec, width2 );
	vm_vec_scale_add( &vecs[1], &start, &uvec, width1 );
	vm_vec_scale_add( &vecs[2], &start, &uvec, -width1 );
	vm_vec_scale_add( &vecs[3], &end, &uvec, -width2 );

	g3_transfer_vertex( &pts[0], &vecs[0] );
	g3_transfer_vertex( &pts[1], &vecs[1] );
	g3_transfer_vertex( &pts[2], &vecs[2] );

	g3_transfer_vertex( &pts[3], &vecs[0] );
	g3_transfer_vertex( &pts[4], &vecs[2] );
	g3_transfer_vertex( &pts[5], &vecs[3] );

	pts[0].u = 0.0f;
	pts[0].v = 0.0f;
	pts[1].u = 1.0f;
	pts[1].v = 0.0f;
	pts[2].u = 1.0f;
	pts[2].v = 1.0f;

	pts[3].u = 0.0f;
	pts[3].v = 0.0f;
	pts[4].u = 1.0f;
	pts[4].v = 1.0f;
	pts[5].u = 0.0f;
	pts[5].v = 1.0f;

	pts[0].r = (ubyte)r;
	pts[0].g = (ubyte)g;
	pts[0].b = (ubyte)b;
	pts[0].a = 255;
	pts[1].r = (ubyte)r;
	pts[1].g = (ubyte)g;
	pts[1].b = (ubyte)b;
	pts[1].a = 255;
	pts[2].r = (ubyte)r;
	pts[2].g = (ubyte)g;
	pts[2].b = (ubyte)b;
	pts[2].a = 255;
	pts[3].r = (ubyte)r;
	pts[3].g = (ubyte)g;
	pts[3].b = (ubyte)b;
	pts[3].a = 255;
	pts[4].r = (ubyte)r;
	pts[4].g = (ubyte)g;
	pts[4].b = (ubyte)b;
	pts[4].a = 255;
	pts[5].r = (ubyte)r;
	pts[5].g = (ubyte)g;
	pts[5].b = (ubyte)b;
	pts[5].a = 255;


	n_to_render += 2;

	return center.xyz.z;
}

void geometry_batcher::render(int flags){
//	gr_set_cull(0);
	if(n_to_render)
	g3_draw_poly( n_to_render*3, vert_list, flags | TMAP_FLAG_TRILIST);
	n_to_render = 0;
}



//laser batcher

#define MAX_LASER_TYPES 256
batch_item	geometry_map[MAX_LASER_TYPES];

int find_good_batch_item(int texture){
	//I realy doubt there will be more than 12 or so of these
	//this could probly be sped up by reseting it every mission
	for(int i = 0; i<MAX_LASER_TYPES; i++){
		if(geometry_map[i].texture == -1){
			geometry_map[i].texture = texture;
			return i;
		}
		if(geometry_map[i].texture == texture)return i;
	}
	Error(LOCATION, "too many laser types, this is probly imposable, but aparently it isn't\n");
	return -1;
}

float add_laser(int texture, vector *p0,float width1,vector *p1,float width2, int r, int g, int b){
	geometry_batcher* item = &geometry_map[find_good_batch_item(texture)].batch;

	item->add_alocate(1);
	return item->draw_laser(p0, width1, p1, width2, r,g,b);	

	return 0;
}

void batch_render_lasers(){

	for (int i = 0;		geometry_map[i].texture != -1;		i++){
		 gr_set_bitmap(geometry_map[i].texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.99999f);
		 geometry_map[i].batch.render(TMAP_FLAG_TEXTURED | TMAP_FLAG_XPARENT | TMAP_HTL_3D_UNLIT | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_CORRECT);
	}

}
/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include "globalincs/pstypes.h"
#include "graphics/grbatch.h"
#include "graphics/2d.h"
#include "cmdline/cmdline.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"

geometry_batcher::~geometry_batcher()
{
	if (vert != NULL) {
		vm_free(vert);
		vert = NULL;
	}
}

void geometry_batcher::allocate_internal(int n_verts)
{
	// this is called to start a batch, you make sure you have enough memory 
	// to store all the geometry, then you clear out the memory and set the
	// number of primitives to 0
	if (n_verts > n_allocated) {
		if (vert != NULL) {
			vm_free(vert);
			vert = NULL;
		}

		vert = (vertex *) vm_malloc( sizeof(vertex) * n_verts );

		Verify( (vert != NULL) );

		memset( vert, 0, sizeof(vertex) * n_verts );
		n_allocated = n_verts;
	}

	n_to_render = 0;
}

void geometry_batcher::allocate(int quad, int n_tri)
{
	int to_alloc = 0;

	// quads have two triangles, therefore six verts
	if (quad > 0 ) {
		to_alloc += (quad * 6);
	}

	// a single triangle has a mere 3 verts
	if ( n_tri > 0 ) {
		to_alloc += (n_tri * 3);
	}

	allocate_internal(to_alloc);
}

void geometry_batcher::add_allocate(int quad, int n_tri)
{
	int to_alloc = (n_to_render * 3);

	// quads have two triangles, therefore six verts
	if ( quad > 0 ) {
		to_alloc += (quad * 6);
	}

	// a single triangle has a mere 3 verts
	if (n_tri > 0) {
		to_alloc += (n_tri * 3);
	}

	vertex *old_vert = vert;

	if (to_alloc > n_allocated) {
		vert = (vertex *) vm_malloc( sizeof(vertex) * to_alloc );

		Verify( (vert != NULL) );

		memset( vert, 0, sizeof(vertex) * to_alloc );

		if (old_vert != NULL) {
			memcpy( vert, old_vert, sizeof(vertex) * n_to_render * 3 );
			vm_free(old_vert);
		}

		n_allocated = to_alloc;
	}
}

void geometry_batcher::clone(const geometry_batcher &geo)
{
	n_to_render = geo.n_to_render;
	n_allocated = geo.n_allocated;

	if (n_allocated > 0) {
		vert = (vertex *) vm_malloc( sizeof(vertex) * n_allocated );

		memcpy( vert, geo.vert, sizeof(vertex) * n_allocated );
	} else {
		vert = NULL;
	}
}

const geometry_batcher &geometry_batcher::operator=(const geometry_batcher &geo)
{
	if (this != &geo) {
		clone(geo);
	}

	return *this;
}

/*
0----1
|\   |
|  \ |
3----2
*/
void geometry_batcher::draw_bitmap(vertex *pnt, int orient, float rad, float depth)
{
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	vec3d PNT, p[4];
	vec3d fvec, rvec, uvec;
	vertex *P = &vert[n_to_render * 3];

	vm_vert2vec(pnt, &PNT);


	// get the direction from the point to the eye
	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	// get an up vector in the general direction of what we want
	uvec = View_matrix.vec.uvec;

	// make a right vector from the f and up vector, this r vec is exactly what we want, so...
	vm_vec_crossprod(&rvec, &fvec, &uvec);
	vm_vec_normalize_safe(&rvec);

	// fix the u vec with it
	vm_vec_crossprod(&uvec, &fvec, &rvec);

	// move the center of the sprite based on the depth parameter
	if ( depth != 0.0f )
		vm_vec_scale_add(&PNT, &PNT, &fvec, depth);

	// move one of the verts to the left
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);

	// and one to the right
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	// now move all oof the verts to were they need to be
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

	// set up the UV coords
	if ( orient & 1 ) {
		// tri 1
		P[5].u = 1.0f;
		P[4].u = 0.0f;
		P[3].u = 0.0f;
		// tri 2
		P[2].u = 1.0f;
		P[1].u = 0.0f;
		P[0].u = 1.0f;
	} else {
		// tri 1
		P[5].u = 0.0f;
		P[4].u = 1.0f;
		P[3].u = 1.0f;
		// tri 2
		P[2].u = 0.0f;
		P[1].u = 1.0f;
		P[0].u = 0.0f;
	}

	if ( orient & 2 ) {
		// tri 1
		P[5].v = 1.0f;
		P[4].v = 1.0f;
		P[3].v = 0.0f;
		// tri 2
		P[2].v = 1.0f;
		P[1].v = 0.0f;
		P[0].v = 0.0f;
	} else {
		// tri 1
		P[5].v = 0.0f;
		P[4].v = 0.0f;
		P[3].v = 1.0f;
		// tri 2
		P[2].v = 0.0f;
		P[1].v = 1.0f;
		P[0].v = 1.0f;
	}

	for (int i = 0; i < 6 ; i++) {
		P[i].r = pnt->r;
		P[i].g = pnt->g;
		P[i].b = pnt->b;
		P[i].a = pnt->a;
	}

	n_to_render += 2;
}

void geometry_batcher::draw_bitmap(vertex *pnt, int orient, float rad, float angle, float depth)
{
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	extern float Physics_viewer_bank;
	angle += Physics_viewer_bank;

	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;

	vec3d PNT, p[4];
	vec3d fvec, rvec, uvec;
	vertex *P = &vert[n_to_render * 3];

	vm_vert2vec(pnt, &PNT);

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	vm_rot_point_around_line(&uvec, &View_matrix.vec.uvec, angle, &vmd_zero_vector, &View_matrix.vec.fvec);
//	uvec = View_matrix.vec.uvec;

	vm_vec_crossprod(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize_safe(&rvec);
	vm_vec_crossprod(&uvec, &View_matrix.vec.fvec, &rvec);

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

	// set up the UV coords
	if ( orient & 1 ) {
		// tri 1
		P[5].u = 1.0f;
		P[4].u = 0.0f;
		P[3].u = 0.0f;
		// tri 2
		P[2].u = 1.0f;
		P[1].u = 0.0f;
		P[0].u = 1.0f;
	} else {
		// tri 1
		P[5].u = 0.0f;
		P[4].u = 1.0f;
		P[3].u = 1.0f;
		// tri 2
		P[2].u = 0.0f;
		P[1].u = 1.0f;
		P[0].u = 0.0f;
	}

	if ( orient & 2 ) {
		// tri 1
		P[5].v = 1.0f;
		P[4].v = 1.0f;
		P[3].v = 0.0f;
		// tri 2
		P[2].v = 1.0f;
		P[1].v = 0.0f;
		P[0].v = 0.0f;
	} else {
		// tri 1
		P[5].v = 0.0f;
		P[4].v = 0.0f;
		P[3].v = 1.0f;
		// tri 2
		P[2].v = 0.0f;
		P[1].v = 1.0f;
		P[0].v = 1.0f;
	}

/*	//tri 1
	P[5].u = 0.0f;	P[5].v = 0.0f;
	P[4].u = 1.0f;	P[4].v = 0.0f;
	P[3].u = 1.0f;	P[3].v = 1.0f;

	//tri 2
	P[2].u = 0.0f;	P[2].v = 0.0f;
	P[1].u = 1.0f;	P[1].v = 1.0f;
	P[0].u = 0.0f;	P[0].v = 1.0f;*/

	for (int i = 0; i < 6 ; i++) {
		P[i].r = pnt->r;
		P[i].g = pnt->g;
		P[i].b = pnt->b;
		P[i].a = pnt->a;
	}

	n_to_render += 2;
}

void geometry_batcher::draw_tri(vertex* verts)
{
	vertex *P = &vert[n_to_render *3 ];

	for (int i = 0; i < 3; i++)
		P[i] = verts[i];

	n_to_render += 1;
}

void geometry_batcher::draw_quad(vertex* verts)
{
	vertex *P = &vert[n_to_render * 3];

	P[0] = verts[0];
	P[1] = verts[1];
	P[2] = verts[2];

	P[0] = verts[0];
	P[2] = verts[2];
	P[3] = verts[3];

	n_to_render += 2;
}


void geometry_batcher::draw_beam(vec3d *start, vec3d *end, float width, float intensity)
{
	vec3d p[4];
	vertex *P = &vert[n_to_render * 3];

	vec3d fvec, uvecs, uvece, evec;

	vm_vec_sub(&fvec, start, end);
	vm_vec_normalize_safe(&fvec);

	vm_vec_sub(&evec, &View_position, start);
	vm_vec_normalize_safe(&evec);

	vm_vec_crossprod(&uvecs, &fvec, &evec);
	vm_vec_normalize_safe(&uvecs);

	vm_vec_sub(&evec, &View_position, end);
	vm_vec_normalize_safe(&evec);

	vm_vec_crossprod(&uvece, &fvec, &evec);
	vm_vec_normalize_safe(&uvece);


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

	ubyte _color = (ubyte)(255.0f * intensity);

	for(int i = 0; i < 6; i++){
		P[i].r = P[i].g = P[i].b = P[i].a = _color;
	}

	n_to_render += 2;
}

float geometry_batcher::draw_laser(vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b)
{
	width1 *= 0.5f;
	width2 *= 0.5f;

	vec3d uvec, fvec, rvec, center, reye;

	vm_vec_sub( &fvec, p0, p1 );
	vm_vec_normalize_safe( &fvec );

	vm_vec_avg( &center, p0, p1 ); // needed for the return value only
	vm_vec_sub(&reye, &Eye_position, &center);
	vm_vec_normalize(&reye);

	// compute the up vector
	vm_vec_crossprod(&uvec, &fvec, &reye);
	vm_vec_normalize_safe(&uvec);
	// ... the forward vector
	vm_vec_crossprod(&fvec, &uvec, &reye);
	vm_vec_normalize_safe(&fvec);
	// now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&rvec, &uvec, &fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	vec3d start, end;

	vm_vec_scale_add(&start, p0, &fvec, -width1);
	vm_vec_scale_add(&end, p1, &fvec, width2);

	vec3d vecs[4];

	vertex *pts = &vert[n_to_render * 3];

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

	pts[0].u = 1.0f;
	pts[0].v = 0.0f;
	pts[1].u = 0.0f;
	pts[1].v = 0.0f;
	pts[2].u = 0.0f;
	pts[2].v = 1.0f;

	pts[3].u = 1.0f;
	pts[3].v = 0.0f;
	pts[4].u = 0.0f;
	pts[4].v = 1.0f;
	pts[5].u = 1.0f;
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

void geometry_batcher::render(int flags)
{
	if (n_to_render) {
		gr_render(n_to_render * 3, vert, flags | TMAP_FLAG_TRILIST);
		n_to_render = 0;
	}
}



// laser batcher

struct batch_item {
	batch_item(): texture(-1), tmap_flags(0), alpha(1.0f), laser(false) {};

	geometry_batcher batch;

	int texture;
	int tmap_flags;
	float alpha;

	bool laser;
};

static SCP_vector<batch_item> geometry_map;

static int find_good_batch_item(int texture)
{
	uint max_size = geometry_map.size();

	for (uint i = 0; i < max_size; i++) {
		if (geometry_map[i].texture == texture)
			return (int)i;
	}

	// don't have an existing match so add a new entry
	batch_item new_item;

	new_item.texture = texture;

	geometry_map.push_back(new_item);

	return (int)(geometry_map.size() - 1);
}

float batch_add_laser(int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b)
{
	if (texture < 0) {
		Int3();
		return 0.0f;
	}

	geometry_batcher *item = NULL;
	int index = find_good_batch_item(texture);
	Assert( index >= 0 );

	geometry_map[index].laser = true;
	item = &geometry_map[index].batch;

	item->add_allocate(1);

	return item->draw_laser(p0, width1, p1, width2, r, g, b);
}

int batch_add_bitmap(int texture, int tmap_flags, vertex *pnt, int orient, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	geometry_batcher *item = NULL;
	int index = find_good_batch_item(texture);
	Assert( index >= 0 );

	Assertion( (geometry_map[index].laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	geometry_map[index].tmap_flags = tmap_flags;
	geometry_map[index].alpha = alpha;

	item = &geometry_map[index].batch;

	item->add_allocate(1);

	item->draw_bitmap(pnt, orient, rad, depth);

	return 0;
}

void batch_render_lasers()
{
	uint map_size = geometry_map.size();
	batch_item *bi;

	for (uint i = 0; i < map_size; i++) {
		bi = &geometry_map[i];

		if ( !bi->laser )
			continue;

		if ( !bi->batch.need_to_render() )
			continue;

		Assert( bi->texture >= 0 );
		gr_set_bitmap(bi->texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.99999f);
		bi->batch.render(TMAP_FLAG_TEXTURED | TMAP_FLAG_XPARENT | TMAP_HTL_3D_UNLIT | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_CORRECT);
	}
}

void batch_render_bitmaps()
{
	uint map_size = geometry_map.size();
	batch_item *bi;

	for (uint i = 0; i < map_size; i++) {
		bi = &geometry_map[i];

		if ( bi->laser )
			continue;

		if ( !bi->batch.need_to_render() )
			continue;

		Assert( bi->texture >= 0 );
		gr_set_bitmap(bi->texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, bi->alpha);
		bi->batch.render( bi->tmap_flags );
	}
}

void batch_render_all()
{
	batch_render_lasers();
	batch_render_bitmaps();
}

void batch_reset()
{
	geometry_map.clear();
}

/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/grbatch.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglstate.h"
#include "render/3d.h"

geometry_batcher::~geometry_batcher()
{
	if (vert != NULL) {
		vm_free(vert);
		vert = NULL;
	}

	if (radius_list != NULL) {
		vm_free(radius_list);
		radius_list = NULL;
	}
}

/**
 * Called to start a batch, you make sure you have enough memory 
 * to store all the geometry, then you clear out the memory and set the
 * number of primitives to 0
 */
void geometry_batcher::allocate_internal(int n_verts)
{

	if (n_verts > n_allocated) {
		if (vert != NULL) {
			vm_free(vert);
			vert = NULL;
		}

		if (radius_list != NULL) {
			vm_free(radius_list);
			radius_list = NULL;
		}

		vert = (vertex *) vm_malloc( sizeof(vertex) * n_verts );
		radius_list = (float *) vm_malloc( sizeof(float) * n_verts );

		Verify( (vert != NULL) );
		Verify( (radius_list != NULL) );

		memset( vert, 0, sizeof(vertex) * n_verts );
		memset( radius_list, 0, sizeof(float) * n_verts );

		n_allocated = n_verts;
	}

	n_to_render = 0;
	use_radius = true;
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
	float *old_radius_list = radius_list;

	if (to_alloc > n_allocated) {
		vert = (vertex *) vm_malloc( sizeof(vertex) * to_alloc );
		radius_list = (float *) vm_malloc( sizeof(float) * to_alloc );

		Verify( (vert != NULL) );
		Verify( (radius_list != NULL) );

		memset( vert, 0, sizeof(vertex) * to_alloc );
		memset( radius_list, 0, sizeof(float) * to_alloc );

		if (old_vert != NULL) {
			memcpy( vert, old_vert, sizeof(vertex) * n_to_render * 3 );
			vm_free(old_vert);
		}

		if (old_radius_list != NULL) {
			memcpy( radius_list, old_radius_list, sizeof(float) * n_to_render * 3 );
			vm_free(old_radius_list);
		}

		n_allocated = to_alloc;
	}
}

void geometry_batcher::clone(const geometry_batcher &geo)
{
	n_to_render = geo.n_to_render;
	n_allocated = geo.n_allocated;
	use_radius = geo.use_radius;

	if (n_allocated > 0) {
		vert = (vertex *) vm_malloc( sizeof(vertex) * n_allocated );
		radius_list = (float *) vm_malloc( sizeof(float) * n_allocated );

		memcpy( vert, geo.vert, sizeof(vertex) * n_allocated );
		memcpy( radius_list, geo.radius_list, sizeof(float) * n_allocated);
	} else {
		vert = NULL;
		radius_list = NULL;
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
	float radius = rad;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	vec3d PNT(pnt->world);
	vec3d p[4];
	vec3d fvec, rvec, uvec;
	vertex *P = &vert[n_to_render * 3];
	float *R = &radius_list[n_to_render * 3];

	// get the direction from the point to the eye
	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	// get an up vector in the general direction of what we want
	uvec = View_matrix.vec.uvec;

	// make a right vector from the f and up vector, this r vec is exactly what we want, so...
	vm_vec_cross(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize_safe(&rvec);

	// fix the u vec with it
	vm_vec_cross(&uvec, &View_matrix.vec.fvec, &rvec);

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
		P[5].texture_position.u = 1.0f;
		P[4].texture_position.u = 0.0f;
		P[3].texture_position.u = 0.0f;
		// tri 2
		P[2].texture_position.u = 1.0f;
		P[1].texture_position.u = 0.0f;
		P[0].texture_position.u = 1.0f;
	} else {
		// tri 1
		P[5].texture_position.u = 0.0f;
		P[4].texture_position.u = 1.0f;
		P[3].texture_position.u = 1.0f;
		// tri 2
		P[2].texture_position.u = 0.0f;
		P[1].texture_position.u = 1.0f;
		P[0].texture_position.u = 0.0f;
	}

	if ( orient & 2 ) {
		// tri 1
		P[5].texture_position.v = 1.0f;
		P[4].texture_position.v = 1.0f;
		P[3].texture_position.v = 0.0f;
		// tri 2
		P[2].texture_position.v = 1.0f;
		P[1].texture_position.v = 0.0f;
		P[0].texture_position.v = 0.0f;
	} else {
		// tri 1
		P[5].texture_position.v = 0.0f;
		P[4].texture_position.v = 0.0f;
		P[3].texture_position.v = 1.0f;
		// tri 2
		P[2].texture_position.v = 0.0f;
		P[1].texture_position.v = 1.0f;
		P[0].texture_position.v = 1.0f;
	}

	for (int i = 0; i < 6 ; i++) {
		P[i].r = pnt->r;
		P[i].g = pnt->g;
		P[i].b = pnt->b;
		P[i].a = pnt->a;

		R[i] = radius;
	}

	n_to_render += 2;
}

void geometry_batcher::draw_bitmap(vertex *pnt, float rad, float angle, float depth)
{
	float radius = rad;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	extern float Physics_viewer_bank;
	angle -= Physics_viewer_bank;

	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;

	vec3d PNT(pnt->world);
	vec3d p[4];
	vec3d fvec, rvec, uvec;
	vertex *P = &vert[n_to_render * 3];
	float *R = &radius_list[n_to_render * 3];

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	vm_rot_point_around_line(&uvec, &View_matrix.vec.uvec, angle, &vmd_zero_vector, &View_matrix.vec.fvec);

	vm_vec_cross(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize_safe(&rvec);
	vm_vec_cross(&uvec, &View_matrix.vec.fvec, &rvec);

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

	//tri 1
	P[5].texture_position.u = 0.0f;	P[5].texture_position.v = 0.0f;
	P[4].texture_position.u = 1.0f;	P[4].texture_position.v = 0.0f;
	P[3].texture_position.u = 1.0f;	P[3].texture_position.v = 1.0f;

	//tri 2
	P[2].texture_position.u = 0.0f;	P[2].texture_position.v = 0.0f;
	P[1].texture_position.u = 1.0f;	P[1].texture_position.v = 1.0f;
	P[0].texture_position.u = 0.0f;	P[0].texture_position.v = 1.0f;

	for (int i = 0; i < 6 ; i++) {
		P[i].r = pnt->r;
		P[i].g = pnt->g;
		P[i].b = pnt->b;
		P[i].a = pnt->a;

		R[i] = radius;
	}

	n_to_render += 2;
}

void geometry_batcher::draw_tri(vertex* verts)
{
	vertex *P = &vert[n_to_render *3 ];

	for (int i = 0; i < 3; i++)
		P[i] = verts[i];

	n_to_render += 1;
	use_radius = false;
}

void geometry_batcher::draw_quad(vertex* verts)
{
	vertex *P = &vert[n_to_render * 3];

	P[0] = verts[0];
	P[1] = verts[1];
	P[2] = verts[2];

	P[3] = verts[0];
	P[4] = verts[2];
	P[5] = verts[3];

	n_to_render += 2;
	use_radius = false;
}


void geometry_batcher::draw_beam(vec3d *start, vec3d *end, float width, float intensity, float offset)
{
	vec3d p[4];
	vertex *P = &vert[n_to_render * 3];
	float *R = &radius_list[n_to_render * 3];

	vec3d fvec, uvecs, uvece, evec;

	vm_vec_sub(&fvec, start, end);
	vm_vec_normalize_safe(&fvec);

	vm_vec_sub(&evec, &View_position, start);
	vm_vec_normalize_safe(&evec);

	vm_vec_cross(&uvecs, &fvec, &evec);
	vm_vec_normalize_safe(&uvecs);

	vm_vec_sub(&evec, &View_position, end);
	vm_vec_normalize_safe(&evec);

	vm_vec_cross(&uvece, &fvec, &evec);
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
	P[0].texture_position.u = 0.0f;	P[0].texture_position.v = 0.0f;
	P[1].texture_position.u = 1.0f;	P[1].texture_position.v = 0.0f;
	P[2].texture_position.u = 1.0f;	P[2].texture_position.v = 1.0f;

	//tri 2
	P[3].texture_position.u = 0.0f;	P[3].texture_position.v = 0.0f;
	P[4].texture_position.u = 1.0f;	P[4].texture_position.v = 1.0f;
	P[5].texture_position.u = 0.0f;	P[5].texture_position.v = 1.0f;

	ubyte _color = (ubyte)(255.0f * intensity);

	for(int i = 0; i < 6; i++){
		P[i].r = P[i].g = P[i].b = P[i].a = _color;
		if(offset > 0.0f) {
			R[i] = offset;
		} else {
			R[i] = width;
		}
	}

	n_to_render += 2;
	use_radius = true;
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
	vm_vec_cross(&uvec, &fvec, &reye);
	vm_vec_normalize_safe(&uvec);
	// ... the forward vector
	vm_vec_cross(&fvec, &uvec, &reye);
	vm_vec_normalize_safe(&fvec);
	// now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_cross(&rvec, &uvec, &fvec);

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

	pts[0].texture_position.u = 1.0f;
	pts[0].texture_position.v = 0.0f;
	pts[1].texture_position.u = 0.0f;
	pts[1].texture_position.v = 0.0f;
	pts[2].texture_position.u = 0.0f;
	pts[2].texture_position.v = 1.0f;

	pts[3].texture_position.u = 1.0f;
	pts[3].texture_position.v = 0.0f;
	pts[4].texture_position.u = 0.0f;
	pts[4].texture_position.v = 1.0f;
	pts[5].texture_position.u = 1.0f;
	pts[5].texture_position.v = 1.0f;

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
	use_radius = false;

	return center.xyz.z;
}

void geometry_batcher::render(int flags, float radius)
{
	if (n_to_render) {
		if ( Use_Shaders_for_effect_rendering && (flags & TMAP_FLAG_SOFT_QUAD || flags & TMAP_FLAG_DISTORTION || flags & TMAP_FLAG_DISTORTION_THRUSTER) && use_radius ) {
			gr_render_effect(n_to_render * 3, vert, radius_list, flags | TMAP_FLAG_TRILIST);
		} else {
			gr_render(n_to_render * 3, vert, flags | TMAP_FLAG_TRILIST);
		}

		use_radius = true;
		n_to_render = 0;
	}
}

void geometry_batcher::load_buffer(effect_vertex* buffer, int *n_verts)
{
	int verts_to_render = n_to_render * 3;
	int i;

	buffer_offset = *n_verts;

	for ( i = 0; i < verts_to_render; ++i) {
		buffer[buffer_offset+i].position = vert[i].world;
		buffer[buffer_offset+i].tex_coord = vert[i].texture_position;

		if ( use_radius && radius_list != NULL ) {
			buffer[buffer_offset+i].radius = radius_list[i];
		} else {
			buffer[buffer_offset+i].radius = 0.0f;
		}

		buffer[buffer_offset+i].r = vert[i].r;
		buffer[buffer_offset+i].g = vert[i].g;
		buffer[buffer_offset+i].b = vert[i].b;
		buffer[buffer_offset+i].a = vert[i].a;
	}

	*n_verts = *n_verts + verts_to_render;
}

void geometry_batcher::render_buffer(int buffer_handle, int flags)
{
	if ( buffer_offset < 0 ) {
		return;
	}

	if ( !n_to_render ) {
		return;
	}

	if ( buffer_handle < 0 ) {
		return;
	}
	
	gr_render_stream_buffer(buffer_handle, buffer_offset, n_to_render * 3, flags | TMAP_FLAG_TRILIST);
	
	use_radius = true;
	n_to_render = 0;
	buffer_offset = -1;
}

void geometry_shader_batcher::render_buffer(int buffer_handle, int flags)
{
	if ( buffer_offset < 0 ) {
		return;
	}

	if ( !vertices.size() ) {
		return;
	}

	if ( buffer_handle < 0 ) {
		return;
	}

	gr_render_stream_buffer(buffer_handle, buffer_offset, vertices.size(), flags | TMAP_FLAG_POINTLIST);

	vertices.clear();
	buffer_offset = -1;
}

void geometry_shader_batcher::load_buffer(particle_pnt* buffer, int *n_verts)
{
	int verts_to_render = vertices.size();
	int i;

	buffer_offset = *n_verts;

	for ( i = 0; i < verts_to_render; ++i) {
		buffer[buffer_offset+i] = vertices[i];
	}

	*n_verts = *n_verts + verts_to_render;
}

void geometry_shader_batcher::draw_bitmap(vertex *position, int orient, float rad, float depth)
{
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	vec3d PNT(position->world);
	vec3d fvec;

	// get the direction from the point to the eye
	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	// move the center of the sprite based on the depth parameter
	if ( depth != 0.0f )
		vm_vec_scale_add(&PNT, &PNT, &fvec, depth);

	particle_pnt new_particle;
	vec3d up = {{{0.0f, 1.0f, 0.0f}}};

	new_particle.position = position->world;
	new_particle.size = rad;

	int direction = orient % 4;

	if ( direction == 1 ) {
		up.xyz.x = 0.0f;
		up.xyz.y = -1.0f;
		up.xyz.z = 0.0f;
	} else if ( direction == 2 ) {
		up.xyz.x = -1.0f;
		up.xyz.y = 0.0f;
		up.xyz.z = 0.0f;
	} else if ( direction == 3 ) {
		up.xyz.x = 1.0f;
		up.xyz.y = 0.0f;
		up.xyz.z = 0.0f;
	}

	new_particle.up = up;

	vertices.push_back(new_particle);
}

/**
 * Laser batcher
 */
struct batch_item {
	batch_item(): texture(-1), tmap_flags(0), alpha(1.0f), laser(false) {}

	geometry_batcher batch;

	int texture;
	int tmap_flags;
	float alpha;

	bool laser;
};

struct g_sdr_batch_item {
	g_sdr_batch_item(): texture(-1), tmap_flags(0), alpha(1.0f), laser(false) {};

	geometry_shader_batcher batch;

	int texture;
	int tmap_flags;
	float alpha;

	bool laser;
};

static SCP_map<int, g_sdr_batch_item> geometry_shader_map;
static SCP_map<int, batch_item> geometry_map;
static SCP_map<int, batch_item> distortion_map;

// Used for sending verts to the vertex buffer
void *Batch_buffer = NULL;
size_t Batch_buffer_size = 0;

void *Batch_geometry_buffer = NULL;
size_t Batch_geometry_buffer_size = 0;

float batch_add_laser(int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	batch_item *item = NULL;
	SCP_map<int, batch_item>::iterator it = geometry_map.find(texture);

	if ( !geometry_map.empty() && it != geometry_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_map[texture];
		item->texture = texture;
	}

	item->laser = true;

	item->batch.add_allocate(1);

	float num = item->batch.draw_laser(p0, width1, p1, width2, r, g, b);

	return num;
}

int batch_add_bitmap(int texture, int tmap_flags, vertex *pnt, int orient, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	if ( tmap_flags & TMAP_FLAG_SOFT_QUAD && ( !Cmdline_softparticles || Use_GLSL <= 2 ) ) {
		// don't render this as a soft particle if we don't support soft particles
		tmap_flags &= ~(TMAP_FLAG_SOFT_QUAD);
	}

	if ( Use_GLSL > 2 && Cmdline_softparticles && !Cmdline_no_geo_sdr_effects && Is_Extension_Enabled(OGL_EXT_GEOMETRY_SHADER4) && (tmap_flags & TMAP_FLAG_VERTEX_GEN) ) {
		geometry_batch_add_bitmap(texture, tmap_flags, pnt, orient, rad, alpha, depth);
		return 0;
	} else if ( tmap_flags & TMAP_FLAG_VERTEX_GEN ) {
		tmap_flags &= ~(TMAP_FLAG_VERTEX_GEN);
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = geometry_map.find(texture);

	if ( !geometry_map.empty() && it != geometry_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = alpha;

	item->batch.add_allocate(1);

	item->batch.draw_bitmap(pnt, orient, rad, depth);

	return 0;
}

int geometry_batch_add_bitmap(int texture, int tmap_flags, vertex *pnt, int orient, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	g_sdr_batch_item *item = NULL;
	SCP_map<int, g_sdr_batch_item>::iterator it = geometry_shader_map.find(texture);

	if ( !geometry_shader_map.empty() && it != geometry_shader_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_shader_map[texture];
		item->texture = texture;
	}
	
	Assertion( (item->laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = alpha;

	item->batch.draw_bitmap(pnt, orient, rad, depth);

	return 0;
}

int batch_add_bitmap_rotated(int texture, int tmap_flags, vertex *pnt, float angle, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	if ( tmap_flags & TMAP_FLAG_SOFT_QUAD && ( !Cmdline_softparticles || Use_GLSL <= 2 ) ) {
		// don't render this as a soft particle if we don't support soft particles
		tmap_flags &= ~(TMAP_FLAG_SOFT_QUAD);
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = geometry_map.find(texture);

	if ( !geometry_map.empty() && it != geometry_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = alpha;

	item->batch.add_allocate(1);

	item->batch.draw_bitmap(pnt, rad, angle, depth);

	return 0;
}

int batch_add_tri(int texture, int tmap_flags, vertex *verts, float alpha)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = geometry_map.find(texture);

	if ( !geometry_map.empty() && it != geometry_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = alpha;

	item->batch.add_allocate(0, 1);	// just allocating for one triangle

	item->batch.draw_tri(verts);

	return 0;
}

int batch_add_quad(int texture, int tmap_flags, vertex *verts, float alpha)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = geometry_map.find(texture);

	if ( !geometry_map.empty() && it != geometry_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = alpha;

	item->batch.add_allocate(1);

	item->batch.draw_quad(verts);

	return 0;
}

int batch_add_polygon(int texture, int tmap_flags, vec3d *pos, matrix *orient, float width, float height, float alpha)
{
	//idiot-proof
	if(width == 0 || height == 0)
		return 0;

	Assert(pos != NULL);
	Assert(orient != NULL);

	//Let's begin.

	const int NUM_VERTICES = 4;
	vec3d p[NUM_VERTICES] = { ZERO_VECTOR };
	vertex v[NUM_VERTICES] = { vertex() };

	p[0].xyz.x = width;
	p[0].xyz.y = height;

	p[1].xyz.x = -width;
	p[1].xyz.y = height;

	p[2].xyz.x = -width;
	p[2].xyz.y = -height;

	p[3].xyz.x = width;
	p[3].xyz.y = -height;

	for(int i = 0; i < NUM_VERTICES; i++)
	{
		vec3d tmp = vmd_zero_vector;

		//Rotate correctly
		vm_vec_unrotate(&tmp, &p[i], orient);
		//Move to point in space
		vm_vec_add2(&tmp, pos);

		//Convert to vertex
		g3_transfer_vertex(&v[i], &tmp);
	}

	v[0].texture_position.u = 1.0f;
	v[0].texture_position.v = 0.0f;

	v[1].texture_position.u = 0.0f;
	v[1].texture_position.v = 0.0f;

	v[2].texture_position.u = 0.0f;
	v[2].texture_position.v = 1.0f;

	v[3].texture_position.u = 1.0f;
	v[3].texture_position.v = 1.0f;

	if (texture < 0) {
		Int3();
		return 1;
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = geometry_map.find(texture);

	if ( !geometry_map.empty() && it != geometry_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = alpha;

	item->batch.add_allocate(1);

	item->batch.draw_quad(v);

	return 0;
}

int batch_add_beam(int texture, int tmap_flags, vec3d *start, vec3d *end, float width, float intensity)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = geometry_map.find(texture);

	if ( !geometry_map.empty() && it != geometry_map.end() ) {
		item = &it->second;
	} else {
		item = &geometry_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = intensity;

	item->batch.add_allocate(1);

	item->batch.draw_beam(start, end, width, intensity);

	return 0;
}

void batch_render_lasers(int buffer_handle)
{
	for (SCP_map<int, batch_item>::iterator bi = geometry_map.begin(); bi != geometry_map.end(); ++bi) {

		if ( !bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		gr_set_bitmap(bi->second.texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.99999f);
		if ( buffer_handle >= 0 ) {
			bi->second.batch.render_buffer(buffer_handle, TMAP_FLAG_TEXTURED | TMAP_FLAG_XPARENT | TMAP_HTL_3D_UNLIT | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_CORRECT);
		} else {
			bi->second.batch.render(TMAP_FLAG_TEXTURED | TMAP_FLAG_XPARENT | TMAP_HTL_3D_UNLIT | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_CORRECT);
		}
	}
}

void batch_load_buffer_lasers(effect_vertex* buffer, int *n_verts)
{
	for (SCP_map<int, batch_item>::iterator bi = geometry_map.begin(); bi != geometry_map.end(); ++bi) {

		if ( !bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		bi->second.batch.load_buffer(buffer, n_verts);
	}
}

void batch_render_geometry_map_bitmaps(int buffer_handle)
{
	for (SCP_map<int, batch_item>::iterator bi = geometry_map.begin(); bi != geometry_map.end(); ++bi) {

		if ( bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		gr_set_bitmap(bi->second.texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, bi->second.alpha);
		if ( buffer_handle >= 0 ) {
			bi->second.batch.render_buffer(buffer_handle, bi->second.tmap_flags);
		} else {
			bi->second.batch.render( bi->second.tmap_flags);
		}
	}
}

void batch_render_geometry_shader_map_bitmaps(int buffer_handle)
{
	for (SCP_map<int, g_sdr_batch_item>::iterator bi = geometry_shader_map.begin(); bi != geometry_shader_map.end(); ++bi) {

		if ( bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		gr_set_bitmap(bi->second.texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, bi->second.alpha);
		bi->second.batch.render_buffer(buffer_handle, bi->second.tmap_flags);
	}
}

void batch_load_buffer_geometry_map_bitmaps(effect_vertex* buffer, int *n_verts)
{
	for (SCP_map<int, batch_item>::iterator bi = geometry_map.begin(); bi != geometry_map.end(); ++bi) {

		if ( bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		bi->second.batch.load_buffer(buffer, n_verts);
	}
}

void batch_load_buffer_geometry_shader_map_bitmaps(particle_pnt* buffer, int *n_verts)
{
	for (SCP_map<int, g_sdr_batch_item>::iterator bi = geometry_shader_map.begin(); bi != geometry_shader_map.end(); ++bi) {

		if ( bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		bi->second.batch.load_buffer(buffer, n_verts);
	}
}

void geometry_batch_render(int stream_buffer)
{
	if ( stream_buffer < 0 ) {
		return;
	}

	int n_to_render = geometry_batch_get_size();
	int n_verts = 0;

	if ( Batch_geometry_buffer_size < (n_to_render * sizeof(particle_pnt)) ) {
		if ( Batch_geometry_buffer != NULL ) {
			vm_free(Batch_geometry_buffer);
		}

		Batch_geometry_buffer_size = n_to_render * sizeof(particle_pnt);
		Batch_geometry_buffer = vm_malloc(Batch_geometry_buffer_size);
	}

	batch_load_buffer_geometry_shader_map_bitmaps((particle_pnt*)Batch_geometry_buffer, &n_verts);

	gr_update_buffer_object(stream_buffer, Batch_geometry_buffer_size, Batch_geometry_buffer);

	batch_render_geometry_shader_map_bitmaps(stream_buffer);
}

void batch_render_all(int stream_buffer)
{
	if ( stream_buffer >= 0 ) {
		// need to get vertex size
		int n_to_render = batch_get_size();
		int n_verts = 0;

		if ( ( Batch_buffer_size < (n_to_render * sizeof(effect_vertex)) ) ) {
			if ( Batch_buffer != NULL ) {
				vm_free(Batch_buffer);
			}

			Batch_buffer_size = n_to_render * sizeof(effect_vertex);
			Batch_buffer = vm_malloc(Batch_buffer_size);
		}
		
		batch_load_buffer_lasers((effect_vertex*)Batch_buffer, &n_verts);
		batch_load_buffer_geometry_map_bitmaps((effect_vertex*)Batch_buffer, &n_verts);
		batch_load_buffer_distortion_map_bitmaps((effect_vertex*)Batch_buffer, &n_verts);
		gr_update_buffer_object(stream_buffer, Batch_buffer_size, Batch_buffer);

		Assert(n_verts <= n_to_render);

		batch_render_lasers(stream_buffer);
		batch_render_geometry_map_bitmaps(stream_buffer);
		//batch_render_distortion_map_bitmaps(true);
	} else {
		batch_render_lasers();
		batch_render_geometry_map_bitmaps();
		//batch_render_distortion_map_bitmaps();
	}

	gr_clear_states();
}

void batch_reset()
{
	geometry_map.clear();
	distortion_map.clear();
}

void batch_render_close()
{
	if ( Batch_buffer != NULL ) {
		vm_free(Batch_buffer);
		Batch_buffer = NULL;
	}

	Batch_buffer_size = 0;
}

int distortion_add_bitmap_rotated(int texture, int tmap_flags, vertex *pnt, float angle, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	if ( Use_GLSL < 2 || !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) ) {
		// don't render distortions if we can't support them.
		return 0;
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = distortion_map.find(texture);

	if ( !distortion_map.empty() && it != distortion_map.end() ) {
		item = &it->second;
	} else {
		item = &distortion_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Distortion particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = alpha;
	
	item->batch.add_allocate(1);

	item->batch.draw_bitmap(pnt, rad, angle, depth);

	return 0;
}

int distortion_add_beam(int texture, int tmap_flags, vec3d *start, vec3d *end, float width, float intensity, float offset)
{
	if (texture < 0) {
		Int3();
		return 1;
	}

	if ( Use_GLSL < 2 || !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) ) {
		// don't render distortions if we can't support them.
		return 0;
	}

	batch_item *item = NULL;

	SCP_map<int, batch_item>::iterator it = distortion_map.find(texture);

	if ( !distortion_map.empty() && it != distortion_map.end() ) {
		item = &it->second;
	} else {
		item = &distortion_map[texture];
		item->texture = texture;
	}

	Assertion( (item->laser == false), "Distortion particle effect %s used as laser glow or laser bitmap\n", bm_get_filename(texture) );

	item->tmap_flags = tmap_flags;
	item->alpha = intensity;

	item->batch.add_allocate(1);

	item->batch.draw_beam(start,end,width,intensity,offset);

	return 0;
}

void batch_render_distortion_map_bitmaps(int buffer_handle)
{
	for (SCP_map<int,batch_item>::iterator bi = distortion_map.begin(); bi != distortion_map.end(); ++bi) {

		if ( bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		gr_set_bitmap(bi->second.texture, GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL, bi->second.alpha);

		if ( buffer_handle >= 0 ) {
			bi->second.batch.render_buffer(buffer_handle, bi->second.tmap_flags);
		} else {
			bi->second.batch.render( bi->second.tmap_flags);
		}
	}
}

void batch_load_buffer_distortion_map_bitmaps(effect_vertex* buffer, int *n_verts)
{
	for (SCP_map<int, batch_item>::iterator bi = distortion_map.begin(); bi != distortion_map.end(); ++bi) {

		if ( bi->second.laser )
			continue;

		if ( !bi->second.batch.need_to_render() )
			continue;

		Assert( bi->second.texture >= 0 );
		bi->second.batch.load_buffer(buffer, n_verts);
	}
}

int batch_get_size()
{
	int n_to_render = 0;
	SCP_map<int, batch_item>::iterator bi;

	for (bi = geometry_map.begin(); bi != geometry_map.end(); ++bi) {
		n_to_render += bi->second.batch.need_to_render();
	}

	for (bi = distortion_map.begin(); bi != distortion_map.end(); ++bi) {
		if ( bi->second.laser )
			continue;

		n_to_render += bi->second.batch.need_to_render();
	}

	return n_to_render * 3;
}

int geometry_batch_get_size()
{
	int n_to_render = 0;
	SCP_map<int, g_sdr_batch_item>::iterator bi;

	for (bi = geometry_shader_map.begin(); bi != geometry_shader_map.end(); ++bi) {
		n_to_render += bi->second.batch.need_to_render();
	}

	return n_to_render;
}

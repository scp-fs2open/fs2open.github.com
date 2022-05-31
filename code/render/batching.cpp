/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "render/batching.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "graphics/material.h"
#include "tracing/tracing.h"

static SCP_map<batch_info, primitive_batch> Batching_primitives;
static SCP_map<batch_buffer_key, primitive_batch_buffer> Batching_buffers;
static int lineTexture = -1;

void primitive_batch::add_triangle(batch_vertex* v0, batch_vertex* v1, batch_vertex *v2)
{
	Vertices.push_back(*v0);
	Vertices.push_back(*v1);
	Vertices.push_back(*v2);
}

void primitive_batch::add_point_sprite(batch_vertex *p)
{
	Vertices.push_back(*p);
}

size_t primitive_batch::load_buffer(batch_vertex* buffer, size_t n_verts)
{
	size_t verts_to_render = Vertices.size();

	for ( size_t i = 0; i < verts_to_render; ++i) {
		buffer[n_verts+i] = Vertices[i];
	}

	return verts_to_render;
}

void primitive_batch::clear()
{
	Vertices.clear();
}

void batching_setup_vertex_layout(vertex_layout *layout, uint vert_mask)
{
	int stride = sizeof(batch_vertex);

	if ( vert_mask & vertex_format_data::mask(vertex_format_data::POSITION3) ) {
		layout->add_vertex_component(vertex_format_data::POSITION3, stride, (int)offsetof(batch_vertex, position));
	}

	if ( vert_mask & vertex_format_data::mask(vertex_format_data::TEX_COORD3) ) {
		layout->add_vertex_component(vertex_format_data::TEX_COORD3, stride, (int)offsetof(batch_vertex, tex_coord));
	}

	if ( vert_mask & vertex_format_data::mask(vertex_format_data::COLOR4) ) {
		layout->add_vertex_component(vertex_format_data::COLOR4, stride, (int)offsetof(batch_vertex, r));
	}

	if ( vert_mask & vertex_format_data::mask(vertex_format_data::RADIUS) ) {
		layout->add_vertex_component(vertex_format_data::RADIUS, stride, (int)offsetof(batch_vertex, radius));
	}

	if ( vert_mask & vertex_format_data::mask(vertex_format_data::UVEC) ) {
		layout->add_vertex_component(vertex_format_data::UVEC, stride, (int)offsetof(batch_vertex, uvec));
	}
}

void batching_init_buffer(primitive_batch_buffer *buffer, primitive_type prim_type, uint vertex_mask)
{
	batching_setup_vertex_layout(&buffer->layout, vertex_mask);

	buffer->buffer_num = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Streaming);
	buffer->buffer_ptr = NULL;
	buffer->buffer_size = 0;
	buffer->desired_buffer_size = 0;
	buffer->prim_type = prim_type;
}

void batching_determine_blend_color(color *clr, int texture, float alpha)
{
	gr_alpha_blend blend_mode = material_determine_blend_mode(texture, true);

	if ( blend_mode == ALPHA_BLEND_ADDITIVE ) {
		gr_init_alphacolor(clr, fl2i(255.0f*alpha), fl2i(255.0f*alpha), fl2i(255.0f*alpha), 255);
	} else {
		gr_init_alphacolor(clr, 255, 255, 255, fl2i(255.0f*alpha));
	}
}

primitive_batch_buffer* batching_find_buffer(uint vertex_mask, primitive_type prim_type)
{
	batch_buffer_key query(vertex_mask, prim_type);

	SCP_map<batch_buffer_key, primitive_batch_buffer>::iterator iter = Batching_buffers.find(query);

	if ( iter == Batching_buffers.end() ) {
		primitive_batch_buffer *buffer = &Batching_buffers[query];

		batching_init_buffer(buffer, prim_type, vertex_mask);

		return buffer;
	} else {
		return &iter->second;
	}
}

primitive_batch* batching_find_batch(int texture, batch_info::material_type material_id, primitive_type prim_type, bool thruster)
{
	// Use the base texture for finding the batch item since all items can reuse the same texture array
	auto base_tex = bm_get_base_frame(texture);

	batch_info query(material_id, base_tex, prim_type, thruster);

	SCP_map<batch_info, primitive_batch>::iterator iter = Batching_primitives.find(query);

	if ( iter == Batching_primitives.end() ) {
		primitive_batch* batch = &Batching_primitives[query];

		*batch = primitive_batch(query);

		return batch;
	} else {
		return &iter->second;
	}
}

uint batching_determine_vertex_layout(batch_info *info)
{
	if ( info->prim_type == PRIM_TYPE_POINTS ) {
		return vertex_format_data::mask(vertex_format_data::POSITION3) 
			| vertex_format_data::mask(vertex_format_data::RADIUS) 
			| vertex_format_data::mask(vertex_format_data::UVEC)
			| vertex_format_data::mask(vertex_format_data::TEX_COORD3);
	}

	if ( info->mat_type == batch_info::VOLUME_EMISSIVE || info->mat_type == batch_info::DISTORTION ) {
		return vertex_format_data::mask(vertex_format_data::POSITION3) 
			| vertex_format_data::mask(vertex_format_data::COLOR4) 
			| vertex_format_data::mask(vertex_format_data::TEX_COORD3)
			| vertex_format_data::mask(vertex_format_data::RADIUS);
	} else {
		return vertex_format_data::mask(vertex_format_data::POSITION3) 
			| vertex_format_data::mask(vertex_format_data::COLOR4) 
			| vertex_format_data::mask(vertex_format_data::TEX_COORD3);
	}
}

void batching_add_bitmap_internal(primitive_batch *batch, int texture, vertex *pnt, int orient, float rad, color *clr, float depth)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

	float radius = rad;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	vec3d PNT(pnt->world);
	vec3d p[4];
	vec3d fvec, rvec, uvec;
	batch_vertex verts[6];

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
	verts[5].position = p[3];
	verts[4].position = p[2];
	verts[3].position = p[1];

	//tri 2
	verts[2].position = p[3];
	verts[1].position = p[1];
	verts[0].position = p[0];

	// set up the UV coords
	if ( orient & 1 ) {
		// tri 1
		verts[5].tex_coord.xyz.x = 1.0f;
		verts[4].tex_coord.xyz.x = 0.0f;
		verts[3].tex_coord.xyz.x = 0.0f;

		// tri 2
		verts[2].tex_coord.xyz.x = 1.0f;
		verts[1].tex_coord.xyz.x = 0.0f;
		verts[0].tex_coord.xyz.x = 1.0f;
	} else {
		// tri 1
		verts[5].tex_coord.xyz.x = 0.0f;
		verts[4].tex_coord.xyz.x = 1.0f;
		verts[3].tex_coord.xyz.x = 1.0f;

		// tri 2
		verts[2].tex_coord.xyz.x = 0.0f;
		verts[1].tex_coord.xyz.x = 1.0f;
		verts[0].tex_coord.xyz.x = 0.0f;
	}

	if ( orient & 2 ) {
		// tri 1
		verts[5].tex_coord.xyz.y = 1.0f;
		verts[4].tex_coord.xyz.y = 1.0f;
		verts[3].tex_coord.xyz.y = 0.0f;

		// tri 2
		verts[2].tex_coord.xyz.y = 1.0f;
		verts[1].tex_coord.xyz.y = 0.0f;
		verts[0].tex_coord.xyz.y = 0.0f;
	} else {
		// tri 1
		verts[5].tex_coord.xyz.y = 0.0f;
		verts[4].tex_coord.xyz.y = 0.0f;
		verts[3].tex_coord.xyz.y = 1.0f;

		// tri 2
		verts[2].tex_coord.xyz.y = 0.0f;
		verts[1].tex_coord.xyz.y = 1.0f;
		verts[0].tex_coord.xyz.y = 1.0f;
	}

	auto array_index = texture - batch->get_render_info().texture;
	for (int i = 0; i < 6 ; i++) {
		verts[i].r = clr->red;
		verts[i].g = clr->green;
		verts[i].b = clr->blue;
		verts[i].a = clr->alpha;

		verts[i].radius = radius;

		// Set array index
		verts[i].tex_coord.xyz.z = (float) array_index;
	}

	batch->add_triangle(&verts[5], &verts[4], &verts[3]);
	batch->add_triangle(&verts[2], &verts[1], &verts[0]);
}

void batching_add_bitmap_rotated_internal(primitive_batch *batch, int texture, vertex *pnt, float angle, float rad, color *clr, float depth)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

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
	batch_vertex verts[6];

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
	verts[5].position = p[3];
	verts[4].position = p[2];
	verts[3].position = p[1];

	//tri 2
	verts[2].position = p[3];
	verts[1].position = p[1];
	verts[0].position = p[0];

	//tri 1
	verts[5].tex_coord.xyz.x = 0.0f;	verts[5].tex_coord.xyz.y = 0.0f;
	verts[4].tex_coord.xyz.x = 1.0f;	verts[4].tex_coord.xyz.y = 0.0f;
	verts[3].tex_coord.xyz.x = 1.0f;	verts[3].tex_coord.xyz.y = 1.0f;

	//tri 2
	verts[2].tex_coord.xyz.x = 0.0f;	verts[2].tex_coord.xyz.y = 0.0f;
	verts[1].tex_coord.xyz.x = 1.0f;	verts[1].tex_coord.xyz.y = 1.0f;
	verts[0].tex_coord.xyz.x = 0.0f;	verts[0].tex_coord.xyz.y = 1.0f;

	auto array_index = texture - batch->get_render_info().texture;
	for (int i = 0; i < 6 ; i++) {
		verts[i].r = clr->red;
		verts[i].g = clr->green;
		verts[i].b = clr->blue;
		verts[i].a = clr->alpha;

		verts[i].radius = radius;
		verts[i].tex_coord.xyz.z = (float)array_index;
	}

	batch->add_triangle(&verts[0], &verts[1], &verts[2]);
	batch->add_triangle(&verts[3], &verts[4], &verts[5]);
}

void batching_add_polygon_internal(primitive_batch *batch, int texture, vec3d *pos, matrix *orient, float width, float height, color *clr)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

	//idiot-proof
	if(width == 0 || height == 0)
		return;

	Assert(pos != NULL);
	Assert(orient != NULL);

	//Let's begin.

	const int NUM_VERTICES = 4;
	vec3d p[NUM_VERTICES] = { ZERO_VECTOR };
	batch_vertex v[NUM_VERTICES];

	p[0].xyz.x = width;
	p[0].xyz.y = height;

	p[1].xyz.x = -width;
	p[1].xyz.y = height;

	p[2].xyz.x = -width;
	p[2].xyz.y = -height;

	p[3].xyz.x = width;
	p[3].xyz.y = -height;

	auto array_index = texture - batch->get_render_info().texture;

	for(int i = 0; i < NUM_VERTICES; i++)
	{
		vec3d tmp = vmd_zero_vector;

		//Rotate correctly
		vm_vec_unrotate(&tmp, &p[i], orient);
		//Move to point in space
		vm_vec_add2(&tmp, pos);

		v[i].position = tmp;

		v[i].r = clr->red;
		v[i].g = clr->green;
		v[i].b = clr->blue;
		v[i].a = clr->alpha;
		v[i].tex_coord.xyz.z = (float)array_index;
	}

	v[0].tex_coord.xyz.x = 1.0f;
	v[0].tex_coord.xyz.y = 0.0f;

	v[1].tex_coord.xyz.x = 0.0f;
	v[1].tex_coord.xyz.y = 0.0f;

	v[2].tex_coord.xyz.x = 0.0f;
	v[2].tex_coord.xyz.y = 1.0f;

	v[3].tex_coord.xyz.x = 1.0f;
	v[3].tex_coord.xyz.y = 1.0f;

	batch->add_triangle(&v[0], &v[1], &v[2]);
	batch->add_triangle(&v[0], &v[2], &v[3]);
}

void batching_add_quad_internal(primitive_batch *batch, int texture, vertex *verts)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

	const int NUM_VERTICES = 4;
	batch_vertex v[NUM_VERTICES];

	auto array_index = texture - batch->get_render_info().texture;
	for ( int i = 0; i < NUM_VERTICES; i++ ) {
		v[i].position = verts[i].world;
		
		v[i].r = verts[i].r;
		v[i].g = verts[i].g;
		v[i].b = verts[i].b;
		v[i].a = verts[i].a;

		v[i].tex_coord.xyz.x = verts[i].texture_position.u;
		v[i].tex_coord.xyz.y = verts[i].texture_position.v;
		v[i].tex_coord.xyz.z = (float)array_index;
	}

	batch->add_triangle(&v[0], &v[1], &v[2]);
	batch->add_triangle(&v[0], &v[2], &v[3]);
}

void batching_add_tri_internal(primitive_batch *batch, int texture, vertex *verts)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

	const int NUM_VERTICES = 3;
	batch_vertex v[NUM_VERTICES];

	auto array_index = texture - batch->get_render_info().texture;
	for ( int i = 0; i < NUM_VERTICES; i++ ) {
		v[i].position = verts[i].world;

		v[i].r = verts[i].r;
		v[i].g = verts[i].g;
		v[i].b = verts[i].b;
		v[i].a = verts[i].a;

		v[i].tex_coord.xyz.x = verts[i].texture_position.u;
		v[i].tex_coord.xyz.y = verts[i].texture_position.v;
		v[i].tex_coord.xyz.z = (float)array_index;
	}

	batch->add_triangle(&v[0], &v[1], &v[2]);
}

void batching_add_beam_internal(primitive_batch *batch, int texture, vec3d *start, vec3d *end, float width, color *clr, float offset)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

	vec3d p[4];
	batch_vertex verts[6];

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
	verts[0].position = p[3];
	verts[1].position = p[2];
	verts[2].position = p[1];

	//tri 2
	verts[3].position = p[3];
	verts[4].position = p[1];
	verts[5].position = p[0];

	//set up the UV coords
	//tri 1
	verts[0].tex_coord.xyz.x = 0.0f; verts[0].tex_coord.xyz.y = 0.0f;
	verts[1].tex_coord.xyz.x = 1.0f; verts[1].tex_coord.xyz.y = 0.0f;
	verts[2].tex_coord.xyz.x = 1.0f; verts[2].tex_coord.xyz.y = 1.0f;

	//tri 2
	verts[3].tex_coord.xyz.x = 0.0f; verts[3].tex_coord.xyz.y = 0.0f;
	verts[4].tex_coord.xyz.x = 1.0f; verts[4].tex_coord.xyz.y = 1.0f;
	verts[5].tex_coord.xyz.x = 0.0f; verts[5].tex_coord.xyz.y = 1.0f;

	auto array_index = texture - batch->get_render_info().texture;
	for(int i = 0; i < 6; i++){
		verts[i].r = clr->red;
		verts[i].g = clr->green;
		verts[i].b = clr->blue;
		verts[i].a = clr->alpha;

		if(offset > 0.0f) {
			verts[i].radius = offset;
		} else {
			verts[i].radius = width;
		}
		verts[i].tex_coord.xyz.z = (float)array_index;
	}

	batch->add_triangle(&verts[0], &verts[1], &verts[2]);
	batch->add_triangle(&verts[3], &verts[4], &verts[5]);
}

void batching_add_line_internal(primitive_batch *batch, int texture, vec3d *start, vec3d *end, float width1, float width2, color *clr)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

	vec3d p[4];
	batch_vertex verts[6];

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

	vm_vec_scale_add(&p[0], start, &uvecs, width1);
	vm_vec_scale_add(&p[1], end, &uvece, width2);
	vm_vec_scale_add(&p[2], end, &uvece, -width2);
	vm_vec_scale_add(&p[3], start, &uvecs, -width1);

	//move all the data from the vecs into the verts
	//tri 1
	verts[0].position = p[3];
	verts[1].position = p[2];
	verts[2].position = p[1];

	//tri 2
	verts[3].position = p[3];
	verts[4].position = p[1];
	verts[5].position = p[0];

	//set up the UV coords
	//tri 1
	verts[0].tex_coord.xyz.x = 0.0f; verts[0].tex_coord.xyz.y = 0.0f;
	verts[1].tex_coord.xyz.x = 1.0f; verts[1].tex_coord.xyz.y = 0.0f;
	verts[2].tex_coord.xyz.x = 1.0f; verts[2].tex_coord.xyz.y = 1.0f;

	//tri 2
	verts[3].tex_coord.xyz.x = 0.0f; verts[3].tex_coord.xyz.y = 0.0f;
	verts[4].tex_coord.xyz.x = 1.0f; verts[4].tex_coord.xyz.y = 1.0f;
	verts[5].tex_coord.xyz.x = 0.0f; verts[5].tex_coord.xyz.y = 1.0f;

	auto array_index = texture - batch->get_render_info().texture;
	for(int i = 0; i < 6; i++){
		verts[i].r = clr->red;
		verts[i].g = clr->green;
		verts[i].b = clr->blue;
		verts[i].a = clr->alpha;

		verts[i].radius = width1;
		
		verts[i].tex_coord.xyz.z = (float)array_index;
	}

	batch->add_triangle(&verts[0], &verts[1], &verts[2]);
	batch->add_triangle(&verts[3], &verts[4], &verts[5]);
}

void batching_add_laser_internal(primitive_batch *batch, int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b)
{
	Assert(batch->get_render_info().prim_type == PRIM_TYPE_TRIS);

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
	batch_vertex verts[6];

	vm_vec_scale_add( &vecs[0], &end, &uvec, width2 );
	vm_vec_scale_add( &vecs[1], &start, &uvec, width1 );
	vm_vec_scale_add( &vecs[2], &start, &uvec, -width1 );
	vm_vec_scale_add( &vecs[3], &end, &uvec, -width2 );

	verts[0].position = vecs[0];
	verts[1].position = vecs[1];
	verts[2].position = vecs[2];

	verts[3].position = vecs[0];
	verts[4].position = vecs[2];
	verts[5].position = vecs[3];

	verts[0].tex_coord.xyz.x = 1.0f;
	verts[0].tex_coord.xyz.y = 0.0f;
	verts[1].tex_coord.xyz.x = 0.0f;
	verts[1].tex_coord.xyz.y = 0.0f;
	verts[2].tex_coord.xyz.x = 0.0f;
	verts[2].tex_coord.xyz.y = 1.0f;

	verts[3].tex_coord.xyz.x = 1.0f;
	verts[3].tex_coord.xyz.y = 0.0f;
	verts[4].tex_coord.xyz.x = 0.0f;
	verts[4].tex_coord.xyz.y = 1.0f;
	verts[5].tex_coord.xyz.x = 1.0f;
	verts[5].tex_coord.xyz.y = 1.0f;

	auto array_index = texture - batch->get_render_info().texture;
	for (auto& vert : verts) {
		vert.r = (ubyte)r;
		vert.g = (ubyte)g;
		vert.b = (ubyte)b;
		vert.a = 255;

		vert.tex_coord.xyz.z = (float)array_index;
	}

	batch->add_triangle(&verts[0], &verts[1], &verts[2]);
	batch->add_triangle(&verts[3], &verts[4], &verts[5]);
}

void batching_add_bitmap(int texture, vertex *pnt, int orient, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	color clr;
	batching_determine_blend_color(&clr, texture, alpha);

	batching_add_bitmap_internal(batch, texture, pnt, orient, rad, &clr, depth);
}

void batching_add_volume_bitmap(int texture, vertex *pnt, int orient, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch;
	
	if ( gr_is_capable(CAPABILITY_SOFT_PARTICLES) ) {
		batch = batching_find_batch(texture, batch_info::VOLUME_EMISSIVE);
	} else {
		batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);
	}

	color clr;
	batching_determine_blend_color(&clr, texture, alpha);

	batching_add_bitmap_internal(batch, texture, pnt, orient, rad, &clr, depth);
}

void batching_add_volume_bitmap_rotated(int texture, vertex *pnt, float angle, float rad, float alpha, float depth)
{
	if ( texture < 0 ) {
		Int3();
		return;
	}

	primitive_batch *batch;

	if ( gr_is_capable(CAPABILITY_SOFT_PARTICLES) ) {
		batch = batching_find_batch(texture, batch_info::VOLUME_EMISSIVE);
	} else {
		batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);
	}

	color clr;
	batching_determine_blend_color(&clr, texture, alpha);

	batching_add_bitmap_rotated_internal(batch, texture, pnt, angle, rad, &clr, depth);
}

void batching_add_distortion_bitmap_rotated(int texture, vertex *pnt, float angle, float rad, float alpha, float depth)
{
	if (texture < 0) {
		Int3();
		return;
	}

	if ( !gr_is_capable(CAPABILITY_DISTORTION) ) {
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::DISTORTION);

	color clr;
	batching_determine_blend_color(&clr, texture, alpha);

	batching_add_bitmap_rotated_internal(batch, texture, pnt, angle, rad, &clr, depth);
}

void batching_add_distortion_beam(int texture, vec3d *start, vec3d *end, float width, float intensity, float offset)
{
	if (texture < 0) {
		Int3();
		return;
	}

	if ( !gr_is_capable(CAPABILITY_DISTORTION) ) {
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::DISTORTION, PRIM_TYPE_TRIS, true);

	color clr;
	batching_determine_blend_color(&clr, texture, intensity);

	batching_add_beam_internal(batch, texture, start, end, width, &clr, offset);
}

void batching_add_beam(int texture, vec3d *start, vec3d *end, float width, float intensity)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	color clr;
	batching_determine_blend_color(&clr, texture, intensity);


	batching_add_beam_internal(batch, texture, start, end, width, &clr, 0.0f);
}

void batching_add_line(vec3d *start, vec3d *end, float widthStart, float widthEnd, color custom_color, bool translucent)
{
	color clr = custom_color;
	primitive_batch *batch;

	if (lineTexture < 0)
	{
		//We only need a single pixel sized texture to render as many lines as we want. 
		//If it doesn't exist yet, then we make one!
		auto previous_target = gr_screen.rendering_to_texture;

		//The texture needs to be colored white, otherwise the line will render
		//as black (or invisible if in translucent mode) no matter which color the user picks		
		lineTexture = bm_make_render_target(1, 1, BMP_FLAG_RENDER_TARGET_STATIC);
		bm_set_render_target(lineTexture);			
		color temp = gr_screen.current_color; 			//Store our working color
		color c;
		gr_init_color(&c, 255, 255, 255);
		gr_set_color_fast(&c); 							//set white as the working color
		gr_pixel(0, 0, GR_RESIZE_NONE);					//paint the pixel to the texture
		gr_set_color_fast(&temp);						//Reset our working color
		bm_set_render_target(previous_target);			//Reset our render target
	}

	if (translucent){
		batch = batching_find_batch(lineTexture, batch_info::FLAT_EMISSIVE);
	}
	else {
		batch = batching_find_batch(lineTexture, batch_info::FLAT_OPAQUE);
	}
	
	batching_add_line_internal(batch, lineTexture, start, end, widthStart, widthEnd, &clr);
}

void batching_add_laser(int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b)
{
	if (texture < 0) {
		Int3();
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	batching_add_laser_internal(batch, texture, p0, width1, p1, width2, r, g, b);
}

void batching_add_polygon(int texture, vec3d *pos, matrix *orient, float width, float height, float alpha)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	color clr;
	batching_determine_blend_color(&clr, texture, alpha);

	batching_add_polygon_internal(batch, texture, pos, orient, width, height, &clr);
}

void batching_add_quad(int texture, vertex *verts)
{
	if ( texture < 0 ) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	batching_add_quad_internal(batch, texture, verts);
}

void batching_add_tri(int texture, vertex *verts)
{
	if ( texture < 0 ) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	batching_add_tri_internal(batch, texture, verts);
}

void batching_render_batch_item(primitive_batch_item* item,
	vertex_layout* layout,
	primitive_type prim_type,
	gr_buffer_handle buffer_num)
{
	GR_DEBUG_SCOPE("Batching render item");
	TRACE_SCOPE(tracing::RenderBatchItem);

	if ( item->batch_item_info.mat_type == batch_info::VOLUME_EMISSIVE ) { // Cmdline_softparticles
		particle_material material_def;

		material_set_unlit_volume(&material_def, item->batch_item_info.texture, prim_type == PRIM_TYPE_POINTS);
		gr_render_primitives_particle(&material_def, prim_type, layout, (int)item->offset, (int)item->n_verts, buffer_num);
	} else if ( item->batch_item_info.mat_type == batch_info::DISTORTION ) {
		distortion_material material_def;

		material_set_distortion(&material_def, item->batch_item_info.texture, item->batch_item_info.thruster);
		gr_render_primitives_distortion(&material_def, PRIM_TYPE_TRIS, layout, (int)item->offset, (int)item->n_verts, buffer_num);
	} else if ( item->batch_item_info.mat_type == batch_info::FLAT_OPAQUE) {
		batched_bitmap_material material_def;

		material_set_batched_opaque_bitmap(&material_def, item->batch_item_info.texture, 2.0f);
		gr_render_primitives_batched(&material_def, PRIM_TYPE_TRIS, layout, (int)item->offset, (int)item->n_verts, buffer_num);
	} else {
		batched_bitmap_material material_def;

		material_set_batched_bitmap(&material_def, item->batch_item_info.texture, 1.0f, 2.0f);
		gr_render_primitives_batched(&material_def, PRIM_TYPE_TRIS, layout, (int)item->offset, (int)item->n_verts, buffer_num);
	}
}

void batching_allocate_and_load_buffer(primitive_batch_buffer *draw_queue)
{
	Assert(draw_queue != NULL);

	if ( draw_queue->buffer_size < draw_queue->desired_buffer_size ) {
		if ( draw_queue->buffer_ptr != NULL ) {
			vm_free(draw_queue->buffer_ptr);
		}

		draw_queue->buffer_size = draw_queue->desired_buffer_size;
		draw_queue->buffer_ptr = vm_malloc(draw_queue->desired_buffer_size);
	}

	draw_queue->desired_buffer_size = 0;
	
	size_t offset = 0;
	size_t num_items = draw_queue->items.size();

	for ( size_t i = 0; i < num_items; ++i ) {
		primitive_batch_item *item = &draw_queue->items[i];

		item->offset = offset;
		item->n_verts = item->batch->load_buffer((batch_vertex*)draw_queue->buffer_ptr, offset);
		item->batch->clear();
		
		offset += item->n_verts;
	}

	if (draw_queue->buffer_num.isValid()) {
		gr_update_buffer_data(draw_queue->buffer_num, draw_queue->buffer_size, draw_queue->buffer_ptr);
	}
}

void batching_load_buffers(bool distortion)
{
	GR_DEBUG_SCOPE("Batching load buffers");
	TRACE_SCOPE(tracing::LoadBatchingBuffers);

	for (auto &buffer_iter : Batching_buffers) {
		// zero out the buffers
		buffer_iter.second.desired_buffer_size = 0;
	}

	// assign primitive batch items
	for (auto &bi : Batching_primitives) {
		if ( bi.first.mat_type == batch_info::DISTORTION ) {
			if ( !distortion ) {
				continue;
			}
		} else {
			if ( distortion ) {
				continue;
			}
		}

		size_t num_verts = bi.second.num_verts();

		if ( num_verts > 0 ) {
			batch_info render_info = bi.second.get_render_info();
			uint vertex_mask = batching_determine_vertex_layout(&render_info);

			primitive_batch_buffer *buffer = batching_find_buffer(vertex_mask, render_info.prim_type);
			primitive_batch_item draw_item;

			draw_item.batch_item_info = render_info;
			draw_item.offset = 0;
			draw_item.n_verts = num_verts;
			draw_item.batch = &bi.second;

			buffer->desired_buffer_size += num_verts * sizeof(batch_vertex);
			buffer->items.push_back(draw_item);
		}
	}

	for (auto &buffer_iter : Batching_buffers) {
		batching_allocate_and_load_buffer(&buffer_iter.second);
	}
}

void batching_render_buffer(primitive_batch_buffer *buffer)
{
	GR_DEBUG_SCOPE("Batching render buffer");
	TRACE_SCOPE(tracing::RenderBatchBuffer);

	size_t num_batches = buffer->items.size();

	for ( int j = 0; j < batch_info::NUM_RENDER_TYPES; ++j ) {
		for ( size_t i = 0; i < num_batches; ++i ) {
			if ( buffer->items[i].batch_item_info.mat_type != j ) {
				continue;
			}

			batching_render_batch_item(&buffer->items[i], &buffer->layout, buffer->prim_type, buffer->buffer_num);
		}
	}

	buffer->items.clear();
}

void batching_render_all(bool render_distortions)
{
	GR_DEBUG_SCOPE("Batching render all");
	TRACE_SCOPE(tracing::DrawEffects);

	batching_load_buffers(render_distortions);

	SCP_map<batch_buffer_key, primitive_batch_buffer>::iterator bi;

	for ( bi = Batching_buffers.begin(); bi != Batching_buffers.end(); ++bi ) {
		batching_render_buffer(&bi->second);
	}
	
	gr_clear_states();
}

void batching_shutdown()
{
	for ( auto buffer_iter = Batching_buffers.begin(); buffer_iter != Batching_buffers.end(); ++buffer_iter ) {
		primitive_batch_buffer *batch_buffer = &buffer_iter->second;

		if ( batch_buffer->buffer_ptr != NULL ) {
			vm_free(batch_buffer->buffer_ptr);
			batch_buffer->buffer_ptr = nullptr;
		}
	}
}

/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/material.h"

struct batch_vertex {
	vec3d position;
	vec3d tex_coord; // 3D coordinate since we also include the array index
	ubyte r, g, b, a;
	float radius;
	vec3d uvec;
};

struct batch_info {
	enum material_type {
		FLAT_EMISSIVE,
		VOLUME_EMISSIVE,
		DISTORTION,
		FLAT_OPAQUE,
		NUM_RENDER_TYPES
	};

	material_type mat_type;
	int texture;
	primitive_type prim_type;
	bool thruster;	// only used by distortion

	batch_info(): mat_type(FLAT_EMISSIVE), texture(-1), prim_type(PRIM_TYPE_TRIS), thruster(false) {}
	batch_info(material_type mat, int tex, primitive_type prim, bool thrust): mat_type(mat), texture(tex), prim_type(prim), thruster(thrust) {}

	bool operator < (const batch_info& batch) const {
		if ( mat_type != batch.mat_type ) {
			return mat_type < batch.mat_type;
		}

		if ( texture != batch.texture ) {
			return texture < batch.texture;
		}

		if ( prim_type != batch.prim_type ) {
			return prim_type < batch.prim_type;
		}

		return thruster != batch.thruster;
	}
};

struct batch_buffer_key {
	uint Vertex_mask;
	primitive_type Prim_type;

	batch_buffer_key(): Vertex_mask(0), Prim_type(PRIM_TYPE_TRIS) {}
	batch_buffer_key(uint vertex_mask, primitive_type prim_type): Vertex_mask(vertex_mask), Prim_type(prim_type) {}

	bool operator < (const batch_buffer_key& key) const {
		if ( Vertex_mask != key.Vertex_mask ) {
			return Vertex_mask < key.Vertex_mask;
		}

		return Prim_type < key.Prim_type;
	}
};

class primitive_batch
{
	batch_info render_info;
	SCP_vector<batch_vertex> Vertices;

public:
	primitive_batch() : render_info() {}
	primitive_batch(batch_info info): render_info(info) {}

	batch_info &get_render_info() { return render_info; }

	void add_triangle(batch_vertex* v0, batch_vertex* v1, batch_vertex* v2);
	void add_point_sprite(batch_vertex *p);

	size_t load_buffer(batch_vertex* buffer, size_t n_verts);

	size_t num_verts() { return Vertices.size();  }

	void clear();
};

struct primitive_batch_item {
	batch_info batch_item_info;
	size_t offset;
	size_t n_verts;

	primitive_batch *batch;
};

struct primitive_batch_buffer {
	vertex_layout layout;
	gr_buffer_handle buffer_num;

	void* buffer_ptr;
	size_t buffer_size;

	size_t desired_buffer_size;

	primitive_type prim_type;

	SCP_vector<primitive_batch_item> items;
};

primitive_batch* batching_find_batch(int texture, batch_info::material_type material_id, primitive_type prim_type = PRIM_TYPE_TRIS, bool thruster = false);

void batching_add_bitmap(int texture, vertex *pnt, int orient, float rad, float alpha = 1.0f, float depth = 0.0f);
void batching_add_volume_bitmap(int texture, vertex *pnt, int orient, float rad, float alpha = 1.0f, float depth = 0.0f);
void batching_add_volume_bitmap_rotated(int texture, vertex *pnt, float angle, float rad, float alpha = 1.0f, float depth = 0.0f);
void batching_add_distortion_bitmap_rotated(int texture, vertex *pnt, float angle, float rad, float alpha = 1.0f, float depth = 0.0f);
void batching_add_distortion_beam(int texture, vec3d *start, vec3d *end, float width, float intensity, float offset);
void batching_add_beam(int texture, vec3d *start, vec3d *end, float width, float intensity);
void batching_add_beam(int texture, vec3d *start, vec3d *end, float width, color custom_color, bool translucent = true);
void batching_add_polygon(int texture, vec3d *pos, matrix *orient, float width, float height, float alpha = 1.0f);
void batching_add_laser(int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r = 255, int g = 255, int b = 255);
void batching_add_quad(int texture, vertex *verts);
void batching_add_tri(int texture, vertex *verts);

void batching_render_all(bool render_distortions = false);

void batching_shutdown();

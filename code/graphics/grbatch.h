/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#ifndef	__GRBATCH_H__
#define	__GRBATCH_H__


class geometry_batcher
{
private:
	int n_to_render;		// the number of primitives to render
	int n_allocated;		// the number of verts allocated
	vertex *vert;

	bool use_radius;
	float *radius_list;		// radiuses associated with the vertices in vert

	int buffer_offset;

	// makes sure we have enough space in the memory buffer for the geometry we are about to put into it
	// you need to figure out how many verts are going to be required
	void allocate_internal(int n_verts);

	void clone(const geometry_batcher &geo);

public:
	geometry_batcher(): n_to_render(0), n_allocated(0), vert(NULL), use_radius(true), radius_list(NULL), buffer_offset(-1) {}
	~geometry_batcher();

    geometry_batcher(const geometry_batcher &geo) { clone(geo); }
    const geometry_batcher &operator=(const geometry_batcher &geo);

	// initial memory space needed
	// NOTE: This MUST be called BEFORE calling any of the draw_*() functions!!
	void allocate(int quad, int n_tri = 0);

	// allocate an additional block of memory to what we already have (this is a resize function)
	void add_allocate(int quad, int n_tri = 0);

	// draw a bitmap into the geometry batcher
	void draw_bitmap(vertex *position, int orient, float rad, float depth = 0);

	// draw a rotated bitmap
	void draw_bitmap(vertex *position, float rad, float angle, float depth);

	// draw a simple 3 vert polygon
	void draw_tri(vertex *verts);

	// draw a simple 4 vert polygon
	void draw_quad(vertex *verts);

	// draw a beam
	void draw_beam(vec3d *start, vec3d *end, float width, float intensity = 1.0f, float offset = 0.0f);

	//draw a laser
	float draw_laser(vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b);

	// draw all of the batched geometry to the back buffer and flushes the cache
	// accepts tmap flags so you can use anything you want really
	void render(int flags, float radius = 0.0f);

	void load_buffer(effect_vertex* buffer, int *n_verts);

	void render_buffer(int buffer_handle, int flags);

	// determine if we even need to try and render this (helpful for particle system)
	int need_to_render() { return n_to_render; }

	void operator =(int){}
};

class geometry_shader_batcher
{
	SCP_vector<particle_pnt> vertices;

	int buffer_offset;
public:
	// draw a bitmap into the geometry batcher
	void draw_bitmap(vertex *position, int orient, float rad, float depth = 0);

	// draw a rotated bitmap
//	void draw_bitmap(vertex *position, float rad, float angle, float depth);

	void load_buffer(particle_pnt* buffer, int *n_verts);

	void render_buffer(int buffer_handle, int flags);

	int need_to_render() { return vertices.size(); };
};


float batch_add_laser(int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r = 255, int g = 255, int b = 255);
int batch_add_bitmap(int texture, int tmap_flags, vertex *pnt, int orient, float rad, float alpha = 1.0f, float depth = 0.0f);
int batch_add_bitmap_rotated(int texture, int tmap_flags, vertex *pnt, float angle, float rad, float alpha = 1.0f, float depth = 0.0f);
int batch_add_beam(int texture, int tmap_flags, vec3d *start, vec3d *end, float width, float intensity = 1.0f);
int batch_add_polygon(int texture, int tmap_flags, vec3d *pos, matrix *orient, float width, float height, float alpha = 1.0f);
int batch_add_tri(int texture, int tmap_flags, vertex *verts, float alpha = 1.0f);
int batch_add_quad(int texture, int tmap_flags, vertex *verts, float alpha = 1.0f);
int distortion_add_bitmap_rotated(int texture, int tmap_flags, vertex *pnt, float angle, float rad, float alpha = 1.0f, float depth = 0.0f);
int distortion_add_beam(int texture, int tmap_flags, vec3d *start, vec3d *end, float width, float intensity = 1.0f, float offset = 0.0f);
void batch_render_all(int stream_buffer = -1);
void batch_render_geometry_map_bitmaps(int buffer_handle = -1);
void batch_load_buffer_geometry_map_bitmaps(effect_vertex* buffer, int *n_verts);
void batch_render_lasers(int buffer_handle = -1);
void batch_load_buffer_lasers(effect_vertex* buffer, int *n_verts);
void batch_reset();
void batch_render_distortion_map_bitmaps(int buffer_handle = -1);
void batch_load_buffer_distortion_map_bitmaps(effect_vertex* buffer, int *n_verts);

int batch_get_size();
void batch_render_close();

int geometry_batch_add_bitmap(int texture, int tmap_flags, vertex *pnt, int orient, float rad, float alpha, float depth);
void batch_load_buffer_geometry_shader_map_bitmaps(particle_pnt* buffer, int *n_verts);
void batch_render_geometry_shader_map_bitmaps();
void geometry_batch_render(int stream_buffer);
int geometry_batch_get_size();

#endif

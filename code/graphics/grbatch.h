/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#ifndef	__GRBATCH_H__
#define	__GRBATCH_H__

// Batch structure to be filled by process requesting batch
typedef struct {
	void (*state_set_func)();
	// For gr_set_bitmap
	int texture_id;
	// For gr_tcache_set
	int bitmap_type;

	// For gr_d3d_set_state
	int filter_type;
	int alpha_blend_type;
	int zbuffer_type;

	int is_set;

} BatchInfo;

bool batch_init();
void batch_deinit();

void batch_start();
void batch_end();
void batch_render();
vertex *batch_get_block(int num_verts, int flags);

extern bool Batch_in_process;

#endif

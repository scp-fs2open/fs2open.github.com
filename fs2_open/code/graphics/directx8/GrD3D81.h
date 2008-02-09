/*
	Main header file for the DX8.1 graphics routines.
	See the .cpp for more info.
	This is going to essentially resemble the original [V] one.
*/

#ifndef _GRD3D81_H
#define _GRD3D81_H

void gr_d3d8_init();
void gr_d3d8_cleanup();

// call this to safely fill in the texture shift and scale values for the specified texture type (Gr_t_*)
void gr_d3d8_get_tex_format(int alpha);

// bitmap functions
void gr_d3d8_bitmap(int x, int y);
void gr_d3d8_bitmap_ex(int x, int y, int w, int h, int sx, int sy);

// create all rendering objects (surfaces, d3d device, viewport, etc)
int gr_d3d8_create_rendering_objects(int clear);
void gr_d3d8_release_rendering_objects();


void gr_d3d8_set_initial_render_state();

#endif // _GRD3D81_H	
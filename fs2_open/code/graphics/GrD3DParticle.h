#ifndef _GRD3DPARTICLE_H_
#define _GRD3DPARTICLE_H_

// #include <d3d8.h>

#include "graphics/GrD3DInternal.h"
#include "graphics/GrD3DParticle.h"

bool gr_d3d_particle_set(vertex *pos, int bitmap_id, float size);
void gr_d3d_particle_reset_list();
bool gr_d3d_particle_render_list();

#endif
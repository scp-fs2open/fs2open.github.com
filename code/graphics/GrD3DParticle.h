#ifndef _GRD3DPARTICLE_H_
#define _GRD3DPARTICLE_H_

const float rt_pointsize   = 3.0f;
const float rt_pointsize_A = 1.0f;
const float rt_pointsize_B = 0.9f;
const float rt_pointsize_C = 0.8f;  

// #include <d3d8.h>

#include "graphics/GrD3DInternal.h"
#include "graphics/GrD3DParticle.h"

bool gr_d3d_particle_set(vertex *pos, int bitmap_id, float size);
void gr_d3d_particle_reset_list();
bool gr_d3d_particle_render_list();

#endif
/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include <d3d8.h>

#include "graphics/GrD3DInternal.h"
#include "graphics/GrD3DLight.h"
#include "particle/particle.h"
#include "cmdline/cmdline.h"

#include "GrD3DParticle.h"

D3DPOINTVERTEX Particle_list[MAX_PARTICLES];
int Current_index = 0;

bool d3d_particle_init()
{
	return true;
}

void d3d_particle_deinit()
{
}

// If this function returns false the particle will not be rendered and steps should be
// taken to render it by another method
bool gr_d3d_particle_set(vertex *pos, int bitmap_id, float size)
{
	if(Cmdline_d3d_particle == 0) return false;
	if(pos == NULL) return false;
	if(Current_index >= MAX_PARTICLES) return false;

	// Better cap the particle size just incase
	if(size >= GlobalD3DVars::d3d_caps.MaxPointSize) {
		return false;
	}

	Particle_list[Current_index].x		= pos->x;
	Particle_list[Current_index].y		= pos->y;
	Particle_list[Current_index].z		= pos->z;
	Particle_list[Current_index].size   = size;

	Particle_list[Current_index].custom = bitmap_id;

	Current_index++;
	return true;
}

void gr_d3d_particle_reset_list()
{
	Current_index = 0;
}

int qsort_texture_compare_func(const void *elem1, const void *elem2 ) 
{
	D3DPOINTVERTEX *p1 = (D3DPOINTVERTEX *) elem1;
	D3DPOINTVERTEX *p2 = (D3DPOINTVERTEX *) elem2;

	if(p1->custom > p2->custom)
		return 1;
	else
		return 0;
}

inline DWORD FtoDW( float f ) { return *((DWORD*)&f); }

bool gr_d3d_particle_render_list()
{
	if(Current_index == 0) return true;

	// Set the render states for using point sprites
    d3d_SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
    d3d_SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );
    d3d_SetRenderState( D3DRS_POINTSIZE_MIN, FtoDW(0.0f) );
	d3d_SetRenderState( D3DRS_POINTSIZE, FtoDW(rt_pointsize) );
    d3d_SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(rt_pointsize_A) );
    d3d_SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(rt_pointsize_B) );
    d3d_SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(rt_pointsize_C) );

	// Sort by texture (only if its worth doing)
	if(Current_index > 10)
		qsort( Particle_list, Current_index, sizeof(D3DPOINTVERTEX), qsort_texture_compare_func);

	// Set engine requirements

	// Render particles in batches
	int render_from = 0;
	int i = 0;
	DWORD current_texture = Particle_list[0].custom;

	HRESULT hr = 0;

	while(i < Current_index) {

		if(current_texture != Particle_list[i].custom)
		{
			// Render that batch
			gr_set_bitmap(current_texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);
			float u, v;
			gr_tcache_set(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u, &v);
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );

			hr += d3d_DrawPrimitive(
				D3DVT_PVERTEX,
				D3DPT_POINTLIST, 
				&Particle_list[render_from],
				i - render_from);

		  //	if((i - render_from) > 5)
		  //		mprintf(("Particle render: %d\n",i - render_from));

			// Get the next texture	and mark the start of the next batch
			current_texture = Particle_list[i].custom;
			render_from		= i;
		}

		i++;
	}

	d3d_SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
    d3d_SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );
		     
	return (hr == 0);
}


/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _GRD3D_H
#define _GRD3D_H

#ifndef NO_DIRECT3D

struct poly_list;
struct line_list;
struct colored_vector;

#include "globalincs/systemvars.h"
#include <d3dx8math.h>

void gr_d3d_exb_flush(int end_of_frame);

extern DWORD 
initial_state_block, 
defuse_state_block, 
glow_mapped_defuse_state_block, 
nonmapped_specular_state_block, 
glow_mapped_nonmapped_specular_state_block, 
mapped_specular_state_block,
cell_state_block, 
glow_mapped_cell_state_block, 
additive_glow_mapping_state_block, 
//single_pass_specmapping_state_block, 
//single_pass_glow_spec_mapping_state_block, 
background_fog_state_block, 
env_state_block, 
cloak_state_block;

void d3d_start_frame();
void d3d_stop_frame();
void d3d_set_initial_render_state(bool set = false)	;
void set_stage_for_defuse(bool set = false);
void set_stage_for_glow_mapped_defuse(bool set = false);
void set_stage_for_defuse_and_non_mapped_spec(bool set = false);
void set_stage_for_glow_mapped_defuse_and_non_mapped_spec(bool set = false);
bool set_stage_for_spec_mapped(bool set = false);
void set_stage_for_cell_shaded(bool set = false);
void set_stage_for_cell_glowmapped_shaded(bool set = false);
void set_stage_for_additive_glowmapped(bool set = false);
void set_stage_for_background_fog(bool set = false);
bool set_stage_for_env_mapped(bool set = false);
//void set_stage_for_single_pass_specmapping(int SAME, bool set = true);
//void set_stage_for_single_pass_glow_specmapping(int SAME, bool set = true);

void gr_d3d_flip();
void gr_d3d_flip_cleanup();
void gr_d3d_fade_in(int instantaneous);
void gr_d3d_fade_out(int instantaneous);
int gr_d3d_save_screen();
void gr_d3d_restore_screen(int id);
void gr_d3d_free_screen(int id);
void gr_d3d_dump_frame_start(int first_frame, int frames_between_dumps);
void gr_d3d_flush_frame_dump();
void gr_d3d_dump_frame_stop();
void gr_d3d_dump_frame();
uint gr_d3d_lock();
void gr_d3d_unlock();
void gr_d3d_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far);
void gr_d3d_set_gamma(float gamma);
void gr_d3d_set_cull(int cull);
void gr_d3d_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);
void gr_d3d_filter_set(int filter);
void gr_d3d_set_clear_color(int r, int g, int b);
void gr_d3d_get_region(int front, int w, int h, ubyte *data);

int gr_d3d_make_buffer(poly_list *list, uint flags);
void gr_d3d_destroy_buffer(int idx);
void gr_d3d_render_buffer(int start, int n_prim, ushort* index_list, uint*, int flags);
void gr_d3d_set_buffer(int idx);
int gr_d3d_make_flat_buffer(poly_list *list);
int gr_d3d_make_line_buffer(line_list *list);

void gr_d3d_set_proj_matrix(float fov, float ratio, float n, float f);
void gr_d3d_end_proj_matrix();
void gr_d3d_set_view_matrix(vec3d* offset, matrix *orient);
void gr_d3d_end_view_matrix();
void gr_d3d_set_scale_matrix(vec3d* scale);
void gr_d3d_end_scale_matrix();
void gr_d3d_start_instance_matrix(vec3d*, matrix*);
void gr_d3d_start_angles_instance_matrix(vec3d* offset, angles *orient);
void gr_d3d_end_instance_matrix();

void d3d_set_texture_panning(float u, float v, bool enable);

void gr_d3d_start_clip();
void gr_d3d_end_clip();

extern ID3DXMatrixStack *world_matrix_stack;
extern ID3DXMatrixStack *view_matrix_stack;
extern ID3DXMatrixStack *proj_matrix_stack;

void gr_d3d_set_texture_addressing(int);

void gr_d3d_setup_background_fog(bool);

void gr_d3d_draw_line_list(colored_vector*lines, int num);
//void d3d_render_to_env(int FACE);

#endif // !NO_DIRECT3D

#endif

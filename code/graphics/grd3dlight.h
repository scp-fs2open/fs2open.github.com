
#ifndef _GRD3DLIGHT_H_
#define _GRD3DLIGHT_H_

#ifndef NO_DIRECT3D

struct light;

int	gr_d3d_make_light(light *light, int idx, int priority);
void gr_d3d_modify_light(light *light, int idx, int priority);
void gr_d3d_destroy_light(int idx);
void gr_d3d_set_light(light *light);
void gr_d3d_reset_lighting();
void gr_d3d_lighting(bool set, bool state);
void gr_d3d_center_alpha(int);
void gr_d3d_set_ambient_light(int r, int g, int b);

#endif // !NO_DIRECT3D

#endif

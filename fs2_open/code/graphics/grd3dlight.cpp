#include "grd3dlight.h"

#include <d3d8.h>

#include "2d.h"
#include "graphics/grd3dinternal.h"

// Constants
const int MAX_LIGHTS     = 256;
const int MAX_D3D_LIGHTS = 8;		// Maximum number of lights that DX8 supports in one pass

enum
{
	LT_DIRECTIONAL,		// A light like a sun
	LT_POINT,			// A point light, like an explosion
	LT_TUBE,			// A tube light, like a fluorescent light
};

// Structures
struct d3d_light{
	// RT, not keen on this, is it a class or a structure? A bit of both is dangerous!
	d3d_light():occupied(false), priority(1){};
	D3DLIGHT8 light;
	bool occupied;
	int priority;
};

// Variables

d3d_light d3d_lights[MAX_LIGHTS];
int hardware_slot[MAX_D3D_LIGHTS];
bool active_list[MAX_LIGHTS];
int currently_enabled[MAX_D3D_LIGHTS] = {-1};

int n_active_lights = 0;
bool lighting_enabled = true;
int HWLightSlot = 0;
int total_lights = 0;
extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;

void FSLight2DXLight(D3DLIGHT8 *DXLight,light_data *FSLight) {

	//Copy the vars into a dx compatible struct
	DXLight->Diffuse.r = FSLight->r * FSLight->intensity;
	DXLight->Diffuse.g = FSLight->g * FSLight->intensity;
	DXLight->Diffuse.b = FSLight->b * FSLight->intensity;
	DXLight->Specular.r = FSLight->spec_r * FSLight->intensity;
	DXLight->Specular.g = FSLight->spec_g * FSLight->intensity;
	DXLight->Specular.b = FSLight->spec_b * FSLight->intensity;
	DXLight->Diffuse.a = 1.0f;
	DXLight->Specular.a = 1.0f;
	DXLight->Ambient.r = 0.0f;
	DXLight->Ambient.g = 0.0f;
	DXLight->Ambient.b = 0.0f;
	DXLight->Ambient.a = 1.0f;


	//If the light is a directional light
	if(FSLight->type == LT_DIRECTIONAL) {
		DXLight->Type = D3DLIGHT_DIRECTIONAL;
		DXLight->Position.x = 0.0f;
		DXLight->Position.y = 0.0f;
		DXLight->Position.z = 0.0f;

		DXLight->Direction.x = FSLight->vec.xyz.x;
		DXLight->Direction.y = FSLight->vec.xyz.y;
		DXLight->Direction.z = FSLight->vec.xyz.z;

		DXLight->Specular.r *= static_light_factor;
		DXLight->Specular.g *= static_light_factor;
		DXLight->Specular.b *= static_light_factor;
	}

	//If the light is a point or tube type
	if((FSLight->type == LT_POINT) || (FSLight->type == LT_TUBE)) {

		if(FSLight->type == LT_POINT){
			DXLight->Specular.r *= static_point_factor;
			DXLight->Specular.g *= static_point_factor;
			DXLight->Specular.b *= static_point_factor;
		}else{
			DXLight->Specular.r *= static_tube_factor;
			DXLight->Specular.g *= static_tube_factor;
			DXLight->Specular.b *= static_tube_factor;
		}

		DXLight->Type = D3DLIGHT_POINT;
		DXLight->Position.x = FSLight->vec.xyz.x;
		DXLight->Position.y = FSLight->vec.xyz.y;
		DXLight->Position.z = FSLight->vec.xyz.z;
		
		//Increase the brightness of point and beam lights, as they seem to be too dark
		DXLight->Diffuse.r *= 2;
		DXLight->Diffuse.g *= 2;
		DXLight->Diffuse.b *= 2;
		DXLight->Specular.r *= 2;
		DXLight->Specular.g *= 2;
		DXLight->Specular.b *= 2;

		//They also have almost no radius...
		DXLight->Range = FSLight->radb +FSLight->rada;
		DXLight->Attenuation0 = 0.0f;
		DXLight->Attenuation1 = 1.0f;
		DXLight->Attenuation2 = 0.0f;
	}

}

//finds the first unocupyed light
int find_first_empty_light()
{
	for(int i = 0; i<MAX_LIGHTS; i++) {
		if(!d3d_lights[i].occupied) {
			return i;
		}
	}
	return -1;
}

void pre_render_lights_init(){
//	if(lighting_enabled)	GlobalD3DVars::lpD3DDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,16,16,16));
//	else 	GlobalD3DVars::lpD3DDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,255,255,255));
	for(int i = 0; i<8; i++){
		if(currently_enabled[i] > -1)GlobalD3DVars::lpD3DDevice->LightEnable(currently_enabled[i],false);
		currently_enabled[i] = -1;
	}
}


void shift_active_lights(int pos){
	int k = 0;
	int l = 0;
	if(!lighting_enabled)return;
	bool move = false;
	for(unsigned int i = 0; (i < GlobalD3DVars::d3d_caps.MaxActiveLights) && ((pos * GlobalD3DVars::d3d_caps.MaxActiveLights)+i < (unsigned int)n_active_lights); i++){
		if(currently_enabled[i] > -1)GlobalD3DVars::lpD3DDevice->LightEnable(currently_enabled[i],false);
		move = false;
		for(k; k<MAX_LIGHTS && !move; k++){
			int slot = (pos * GlobalD3DVars::d3d_caps.MaxActiveLights)+l;
			if(active_list[slot]){
				if(d3d_lights[slot].occupied){
					GlobalD3DVars::lpD3DDevice->SetLight(slot,&d3d_lights[slot].light);
					GlobalD3DVars::lpD3DDevice->LightEnable(slot,true);
					currently_enabled[i] = slot;
					move = true;
					l++;
				}
			}
		}
	}
}

int	gr_d3d_make_light(light_data* light, int idx, int priority)
{
//Stub
	return idx;
}

void gr_d3d_modify_light(light_data* light, int idx, int priority)
{
//Stub
}

void gr_d3d_destroy_light(int idx)
{
//Stub
}

void gr_d3d_set_light(light_data *light)
{
	//Init the light
	FSLight2DXLight(&d3d_lights[n_active_lights].light,light);
	d3d_lights[n_active_lights].occupied = true;
	active_list[n_active_lights++] = true;
}

void gr_d3d_reset_lighting()
{
	for(int i = 0; i<MAX_LIGHTS; i++){
		d3d_lights[i].occupied = false;
	}
	for(i=0; i<8; i++){
		GlobalD3DVars::lpD3DDevice->LightEnable(i,false);
		active_list[i] = false;
	}
	n_active_lights =0;
}

extern D3DMATERIAL8 material;
extern bool the_lights_are_on;
void		d3d_set_initial_render_state();

void gr_d3d_lighting(bool set, bool state)
{
	lighting_enabled = set;
//	if(!the_lights_are_on && state)


	if((gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER) && !set){
/*		float a = gr_screen.current_alpha;
		D3DCOLORVALUE col;
		col.r = a;
		col.g = a;
		col.b = a;
		col.a = a;
		material.Ambient = col;
		material.Diffuse = col;
		material.Specular = col;
		material.Emissive = col;*/
		int l = int(255.0f*gr_screen.current_alpha);
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(l,l,l,l));
		for(int i = 0; i<8; i++){
			if(currently_enabled[i] > -1)GlobalD3DVars::lpD3DDevice->LightEnable(currently_enabled[i],false);
			currently_enabled[i] = -1;
		}
		d3d_set_initial_render_state();
	}else{
		D3DCOLORVALUE col;
		col.r = 1.0;col.g = 1.0;col.b = 1.0;col.a = 1.0;
		material.Ambient = col;
		material.Diffuse = col;
		material.Specular = col;
		col.r = 0.0;col.g = 0.0;col.b = 0.0;col.a = 0.0;
		material.Emissive = col;
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,16,16,16));
	}
	if(!state)d3d_set_initial_render_state();

	GlobalD3DVars::lpD3DDevice->SetMaterial(&material);
	d3d_SetRenderState(D3DRS_LIGHTING , state);

}

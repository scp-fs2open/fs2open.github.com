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

bool lighting_enabled = true;
int HWLightSlot = 0;
int total_lights = 0;

void d3d_fs2light_2_dxlight(D3DLIGHT8 *DXLight,light_data *FSLight) 
{
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
	}

	//If the light is a point or tube type
	if((FSLight->type == LT_POINT) || (FSLight->type == LT_TUBE)) {
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
		DXLight->Range = FSLight->rada * 64;
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

void pre_render_lights_init()
{
	if(lighting_enabled) {
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,16,16,16));
	} else {
		d3d_SetRenderState(D3DRS_AMBIENT, 
			D3DCOLOR_ARGB(
				byte(255.0f * gr_screen.current_alpha),
				byte(255.0f * gr_screen.current_alpha),
				byte(255.0f * gr_screen.current_alpha),
				byte(255.0f * gr_screen.current_alpha)));
	}
	
	for(int i = 0; i<MAX_D3D_LIGHTS; i++){
		if(currently_enabled[i] > -1) {
			GlobalD3DVars::lpD3DDevice->LightEnable(currently_enabled[i],false);
		}
		currently_enabled[i] = -1;
	}
}

void shift_active_lights(int pos)
{
//Stub
}

int find_first_empty_hardware_slot()
{
	int light_in_use;
	for(unsigned int i = 0; i < GlobalD3DVars::d3d_caps.MaxActiveLights; i++){
		GlobalD3DVars::lpD3DDevice->GetLightEnable(i,&light_in_use);
		if(!light_in_use) {
			return i;
		}
	}
	return -1;
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
	D3DLIGHT8 DXLight;
	d3d_fs2light_2_dxlight(&DXLight,light);

	//Increment the hw light and set it up in d3d
	HWLightSlot++;
	if(HWLightSlot <= GlobalD3DVars::d3d_caps.MaxActiveLights) {

		GlobalD3DVars::lpD3DDevice->SetLight(HWLightSlot,&DXLight);
		GlobalD3DVars::lpD3DDevice->LightEnable(HWLightSlot,TRUE);
	
	}
}

void gr_d3d_reset_lighting()
{
	//Reset the light counter
	HWLightSlot = 0;

	//Disable all the HW lights
	for(unsigned int i = 0; i< GlobalD3DVars::d3d_caps.MaxActiveLights; i++){
		GlobalD3DVars::lpD3DDevice->LightEnable(i,FALSE);
	}
}

void gr_d3d_lighting(bool set)
{
	lighting_enabled = set;
//	d3d_SetRenderState(D3DRS_LIGHTING , set);
}

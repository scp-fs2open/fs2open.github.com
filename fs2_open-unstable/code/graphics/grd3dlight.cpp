
#ifndef NO_DIRECT3D

#include "grd3dlight.h"

#include <d3d8.h>

#include "2d.h"
#include "math/vecmat.h"
#include "graphics/grd3dinternal.h"
#include "cmdline/cmdline.h"
#include "lighting/lighting.h"



// Constants
const int MAX_D3D_LIGHTS = 8;		// Maximum number of lights that DX8 supports in one pass


// Structures
struct d3d_light{
	// RT, not keen on this, is it a class or a structure? A bit of both is dangerous!
	d3d_light():occupied(false), priority(1){};
	D3DLIGHT8 dxlight;
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

// Resonable defaults, can change using -ambient_light
// Also someone should put an a Fred slider or something 

const int Ambient_default = 120;

int Ambient_red = Ambient_default;
int Ambient_green = Ambient_default;
int Ambient_blue = Ambient_default;

D3DCOLOR ambient_light;

// Initialise the module
bool d3d_init_light()
{
	int factor = (Cmdline_ambient_factor * 2) - 255;

	int r = Ambient_red + factor;
	int g = Ambient_green + factor;
	int b = Ambient_blue + factor;

	if(r < 0) r = 0;
	else if( r > 255) r = 255;
	if(g < 0) g = 0;
	else if( g > 255) g = 255;
	if(b < 0) b = 0;
	else if( b > 255) b = 255;

	// This method gives the user overall control 
	ambient_light = D3DCOLOR_ARGB(255, r, g, b);
	return true;
}

void FSLight2DXLight(D3DLIGHT8 *DXLight, light *FSLight) 
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

		DXLight->Specular.r *= static_light_factor;
		DXLight->Specular.g *= static_light_factor;
		DXLight->Specular.b *= static_light_factor;
	}

	//If the light is a point or tube type
	if((FSLight->type == LT_POINT) || (FSLight->type == LT_TUBE)) {

		float r = 2.0f;
		if(FSLight->type == LT_POINT){
			DXLight->Specular.r *= static_point_factor;
			DXLight->Specular.g *= static_point_factor;
			DXLight->Specular.b *= static_point_factor;
		}else{
			DXLight->Specular.r *= static_tube_factor;
			DXLight->Specular.g *= static_tube_factor;
			DXLight->Specular.b *= static_tube_factor;
			r = 10.0f;
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
		DXLight->Range = FSLight->rada + FSLight->radb * r;
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
//	if(lighting_enabled)	d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,16,16,16));
//	else 	d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,255,255,255));
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
					GlobalD3DVars::lpD3DDevice->SetLight(slot,&d3d_lights[slot].dxlight);
					GlobalD3DVars::lpD3DDevice->LightEnable(slot,true);
					currently_enabled[i] = slot;
					move = true;
					l++;
				}
			}
		}
	}
}

int	gr_d3d_make_light(light *light, int idx, int priority)
{
//Stub
	return idx;
}

void gr_d3d_modify_light(light *light, int idx, int priority)
{
//Stub
}

void gr_d3d_destroy_light(int idx)
{
//Stub
}

void gr_d3d_set_light(light *light)
{
	//Init the light
	FSLight2DXLight(&d3d_lights[n_active_lights].dxlight, light);
#ifndef NDEBUG
	Assert(d3d_lights[n_active_lights].dxlight.Range >= 0);
	Assert(d3d_lights[n_active_lights].dxlight.Range != 0 || d3d_lights[n_active_lights].dxlight.Type == D3DLIGHT_DIRECTIONAL);
#endif
	d3d_lights[n_active_lights].occupied = true;
	active_list[n_active_lights++] = true;
}

void gr_d3d_reset_lighting()
{
	int i;

	for(i = 0; i<MAX_LIGHTS; i++){
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
void		d3d_set_initial_render_state(bool set = true);

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
		d3d_SetRenderState(D3DRS_AMBIENT, ambient_light);
	}
	if(!state)d3d_set_initial_render_state();

	GlobalD3DVars::lpD3DDevice->SetMaterial(&material);
	d3d_SetRenderState(D3DRS_LIGHTING , state);

}



//this sets up a light to be pointinf from the eye to the object, 
//the point being to make the object ether center or edge alphaed with THL
//this effect is used mostly on shockwave models
//-1 == edge
//1 == center
//0 == none
//should be called after lighting has been set up, 
//currently not designed for use with lit models
extern vec3d Eye_position, Object_position;
int GR_center_alpha = false;
/**** !!!>>>---yes this fixes a bug---<<<!!! ****/
void gr_d3d_center_alpha(int type){
	GR_center_alpha = type;
}
void gr_d3d_center_alpha_int(int type){
	if(!type){
	//	GlobalD3DVars::lpD3DDevice->LightEnable(0,false);
	//	GlobalD3DVars::lpD3DDevice->LightEnable(1,false);
		return;
	}
//	pre_render_lights_init();
//	type*=-1;
	vec3d dir;
	vm_vec_sub(&dir, &Eye_position, &Object_position);
	vm_vec_normalize(&dir);

//	if(type == -1)d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,255,255,255));
//	if(type == 1)d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(0,0,0,0));
	d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(0,0,0,0));

	D3DLIGHT8 XLight;
	D3DLIGHT8 *DXLight = &XLight;

	if(type == 1){
	DXLight->Diffuse.r = -1;
	DXLight->Diffuse.g = -1;
	DXLight->Diffuse.b = -1;
	}else{
	DXLight->Diffuse.r = gr_screen.current_alpha;
	DXLight->Diffuse.g = gr_screen.current_alpha;
	DXLight->Diffuse.b = gr_screen.current_alpha;
	}
	DXLight->Specular.r = 0;
	DXLight->Specular.g = 0;
	DXLight->Specular.b = 0;
	DXLight->Diffuse.a = 1;
	DXLight->Specular.a = 0.0f;
	if(type == 1){
	DXLight->Ambient.r = gr_screen.current_alpha;
	DXLight->Ambient.g = gr_screen.current_alpha;
	DXLight->Ambient.b = gr_screen.current_alpha;
	}else{
	DXLight->Ambient.r = 0.0f;
	DXLight->Ambient.g = 0.0f;
	DXLight->Ambient.b = 0.0f;
	}
	DXLight->Ambient.a = 1.0f;
	DXLight->Range = 100; //not even used for directional lights

		DXLight->Type = D3DLIGHT_DIRECTIONAL;

		DXLight->Direction.x = dir.xyz.x;
		DXLight->Direction.y = dir.xyz.y;
		DXLight->Direction.z = dir.xyz.z;

			GlobalD3DVars::lpD3DDevice->SetLight(0,DXLight);
			GlobalD3DVars::lpD3DDevice->LightEnable(0,true);

		DXLight->Direction.x = -dir.xyz.x;
		DXLight->Direction.y = -dir.xyz.y;
		DXLight->Direction.z = -dir.xyz.z;

			GlobalD3DVars::lpD3DDevice->SetLight(1,DXLight);
			GlobalD3DVars::lpD3DDevice->LightEnable(1,true);

/*		D3DCOLORVALUE col;
		col.r = gr_screen.current_alpha*type;col.g = gr_screen.current_alpha*type;col.b = gr_screen.current_alpha*type;col.a = gr_screen.current_alpha*type;
		material.Diffuse = col;
		col.r = 0.0;col.g = 0.0;col.b = 0.0;col.a = 0.0;
		material.Specular = col;
		material.Emissive = col;
		col.r = 1.0;col.g = 1.0;col.b = 1.0;col.a = 10.0;
		material.Ambient = col;
		GlobalD3DVars::lpD3DDevice->SetMaterial(&material);*/
		GR_center_alpha = 0;
		d3d_SetRenderState(D3DRS_LIGHTING , TRUE);
}

void gr_d3d_set_ambient_light(int r, int g, int b)
{
	Ambient_red=r;
	Ambient_green=g;
	Ambient_blue=b;

	d3d_init_light();
}

#endif // !NO_DIRECT3D

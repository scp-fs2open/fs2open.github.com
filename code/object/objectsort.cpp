/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <algorithm>
#include <list>
#include <vector>

#include "asteroid/asteroid.h"
#include "cmdline/cmdline.h"
#include "debris/debris.h"
#include "graphics/light.h"
#include "jumpnode/jumpnode.h"
#include "mission/missionparse.h"
#include "model/modelrender.h"
#include "nebula/neb.h"
#include "object/object.h"
#include "scripting/scripting.h"
#include "render/3d.h"
#include "render/batching.h"
#include "ship/ship.h"
#include "tracing/tracing.h"
#include "weapon/weapon.h"
#include "decals/decals.h"

class sorted_obj
{
public:
	object			*obj;					// a pointer to the original object
	float			z, min_z, max_z;		// The object's z values relative to viewer

	sorted_obj() :
		obj(NULL), z(0.0f), min_z(0.0f), max_z(1.0f)
	{
	}

	bool operator < (const sorted_obj &other) const;
};

inline bool sorted_obj::operator < (const sorted_obj &other) const
{
	int model_num_a = -1;
	int model_num_b = -1;

	if ( obj->type == OBJ_SHIP ) {
		ship_info *sip = &Ship_info[Ships[obj->instance].ship_info_index];

		model_num_a = sip->model_num;
	} else if ( obj->type == OBJ_WEAPON ){
		weapon_info *wip;

		wip = &Weapon_info[Weapons[obj->instance].weapon_info_index];

		if ( wip->render_type == WRT_POF ) {
			model_num_a = wip->model_num;
		}
	} else if ( obj->type == OBJ_DEBRIS ) {
		debris		*db;

		db = &Debris[obj->instance];
		model_num_a = db->model_num;
	} else if ( obj->type == OBJ_ASTEROID ) {
		asteroid		*asp;

		asp = &Asteroids[obj->instance];
		model_num_a = Asteroid_info[asp->asteroid_type].model_num[asp->asteroid_subtype];
	}

	if ( other.obj->type == OBJ_SHIP ) {
		ship_info *sip = &Ship_info[Ships[other.obj->instance].ship_info_index];

		model_num_b = sip->model_num;
	} else if ( other.obj->type == OBJ_WEAPON ){
		weapon_info *wip;

		wip = &Weapon_info[Weapons[other.obj->instance].weapon_info_index];

		if ( wip->render_type == WRT_POF ) {
			model_num_b = wip->model_num;
		}
	} else if ( other.obj->type == OBJ_DEBRIS ) {
		debris		*db;

		db = &Debris[other.obj->instance];
		model_num_b = db->model_num;
	} else if ( other.obj->type == OBJ_ASTEROID ) {
		asteroid		*asp;

		asp = &Asteroids[other.obj->instance];
		model_num_b = Asteroid_info[asp->asteroid_type].model_num[asp->asteroid_subtype];
	}

	if ( model_num_a == model_num_b ) {
		return (max_z > other.max_z);
	}

	return model_num_a < model_num_b;
}


SCP_vector<sorted_obj> Sorted_objects;
SCP_vector<object*> effect_ships; 
SCP_vector<object*> transparent_objects;
bool object_had_transparency = false;
// Used to (fairly) quicky find the 8 extreme
// points around an object.
vec3d check_offsets[8] = { 
  { { { -1.0f, -1.0f, -1.0f } } },
  { { { -1.0f, -1.0f,  1.0f } } },
  { { { -1.0f,  1.0f, -1.0f } } },
  { { { -1.0f,  1.0f,  1.0f } } },
  { { {  1.0f, -1.0f, -1.0f } } },
  { { {  1.0f, -1.0f,  1.0f } } },
  { { {  1.0f,  1.0f, -1.0f } } },
  { { {  1.0f,  1.0f,  1.0f } } }
};

// See if an object is in the view cone.
// Returns:
// 0 if object isn't in the view cone
// 1 if object is in cone 
// This routine could possibly be optimized.  Right now, for an
// offscreen object, it has to rotate 8 points to determine it's
// offscreen.  Not the best considering we're looking at a sphere.
int obj_in_view_cone( object * objp )
{
	int i;
	vec3d tmp,pt;
	ubyte codes;

	// Center isn't in... are other points?
	ubyte and_codes = 0xff;

	for (i=0; i<8; i++ ) {
		vm_vec_scale_add( &pt, &objp->pos, &check_offsets[i], objp->radius );
		codes=g3_rotate_vector(&tmp,&pt);
		if ( !codes ) {
			//mprintf(( "A point is inside, so render it.\n" ));
			return 1;		// this point is in, so return 1
		}
		and_codes &= codes;
	}

	if (and_codes) {
		//mprintf(( "All points offscreen, so don't render it.\n" ));
		return 0;	//all points off screen
	}

	//mprintf(( "All points inside, so render it, but doesn't need clipping.\n" ));
	return 1;
}

inline bool obj_render_is_model(object *obj)
{
	return obj->type == OBJ_SHIP 
		|| (obj->type == OBJ_WEAPON 
			&& Weapon_info[Weapons[obj->instance].weapon_info_index].render_type == WRT_POF) 
		|| obj->type == OBJ_ASTEROID 
		|| obj->type == OBJ_DEBRIS
		|| obj->type == OBJ_JUMP_NODE;
}

// Are there reasons to hide objects base on distance?
bool is_full_nebula()
{
	return (The_mission.flags[Mission::Mission_Flags::Fullneb]) && (Neb2_render_mode != NEB2_RENDER_NONE) && !Fred_running;
}

// Sorts all the objects by Z and renders them
void obj_render_all(const std::function<void(object*)>& render_function, bool *draw_viewer_last )
{
	object *objp;
	int i;
	float fog_near, fog_far, fog_density;

	objp = Objects;

	for (i=0;i<=Highest_object_index;i++,objp++) {
		if ( (objp->type != OBJ_NONE) && (objp->flags[Object::Object_Flags::Renders]) )	{
            objp->flags.remove(Object::Object_Flags::Was_rendered);

			if ( obj_in_view_cone(objp) )	{
				sorted_obj osp;

				osp.obj = objp;

				vec3d to_obj;
				vm_vec_sub( &to_obj, &objp->pos, &Eye_position );
				osp.z = vm_vec_dot( &Eye_matrix.vec.fvec, &to_obj );
/*
				if ( objp->type == OBJ_SHOCKWAVE )
					osp.z -= 2*objp->radius;
*/
				// Make warp in effect draw after any ship in it
				if ( objp->type == OBJ_FIREBALL )	{
					//if ( fireball_is_warp(objp) )	{
					osp.z -= 2*objp->radius;
					//}
				}
					
				osp.min_z = osp.z - objp->radius;
				osp.max_z = osp.z + objp->radius;

				Sorted_objects.push_back(osp);
			}
		}	
	}

	if ( Sorted_objects.empty() )
		return;

	std::sort(Sorted_objects.begin(), Sorted_objects.end());

	gr_zbuffer_set( GR_ZBUFF_FULL );	

	bool full_neb = is_full_nebula();
	bool c_viewer = (!Viewer_mode || (Viewer_mode & VM_PADLOCK_ANY) || (Viewer_mode & VM_OTHER_SHIP) || (Viewer_mode & VM_TRACK));

	// now draw them
	// only render models in this loop in order to minimize state changes
	SCP_vector<sorted_obj>::iterator os;
	for (os = Sorted_objects.begin(); os != Sorted_objects.end(); ++os) {
		object *obj = os->obj;

		obj->flags.set(Object::Object_Flags::Was_rendered);

		//This is for ship cockpits. Bobb, feel free to optimize this any way you see fit
		if ( (obj == Viewer_obj)
			&& (obj->type == OBJ_SHIP)
			&& c_viewer
			&& (Ship_info[Ships[obj->instance].ship_info_index].flags[Ship::Info_Flags::Show_ship_model]) )
		{
			(*draw_viewer_last) = true;
			continue;
		}

		// if we're fullneb, fire up the fog - this also generates a fog table
		if (full_neb) {
			// get the fog values
			neb2_get_adjusted_fog_values(&fog_near, &fog_far, &fog_density, obj);

			// maybe skip rendering an object because its obscured by the nebula
			if(neb2_skip_render(obj, os->z)){
				continue;
			}
		}

		if ( obj_render_is_model(obj) ) {
			if( ((obj->type == OBJ_SHIP) && Ships[obj->instance].shader_effect_active) || (obj->type == OBJ_FIREBALL) )
				effect_ships.push_back(obj);
			else 
				render_function(obj);
		}
		if(object_had_transparency)
		{
			object_had_transparency = false;
			transparent_objects.push_back(obj);
		}
	}
	gr_deferred_lighting_end();

	// we're done rendering models so flush render states
	gr_clear_states();

	// render everything else that isn't a model
	for (os = Sorted_objects.begin(); os != Sorted_objects.end(); ++os) {
		object *obj = os->obj;

		obj->flags.set(Object::Object_Flags::Was_rendered);

		if ( obj_render_is_model(obj) )
			continue;

		// maybe skip rendering an object because its obscured by the nebula
		if(full_neb && neb2_skip_render(obj, os->z)){
			continue;
		}

		render_function(obj);
	}

	Sorted_objects.clear();
	
	batching_render_all();
	batching_render_all(true);
}

void obj_render_queue_all()
{
	GR_DEBUG_SCOPE("Render all objects");
	TRACE_SCOPE(tracing::RenderScene);

	object *objp;
	int i;
	model_draw_list scene;

	objp = Objects;

	gr_deferred_lighting_begin();

	scene.init();

	bool full_neb = is_full_nebula();

	for ( i = 0; i <= Highest_object_index; i++,objp++ ) {
		if ( (objp->type != OBJ_NONE) && ( objp->flags [Object::Object_Flags::Renders] ) )	{
            objp->flags.remove(Object::Object_Flags::Was_rendered);

			if ( !obj_in_view_cone(objp) ) {
				continue;
			}

			if ( full_neb ) {
				vec3d to_obj;
				vm_vec_sub( &to_obj, &objp->pos, &Eye_position );
				float z = vm_vec_dot( &Eye_matrix.vec.fvec, &to_obj );

				if ( neb2_skip_render(objp, z) ){
					continue;
				}
			}


			if ( (objp->type == OBJ_SHIP) && Ships[objp->instance].shader_effect_active ) {
				effect_ships.push_back(objp);
				continue;
			}

            objp->flags.set(Object::Object_Flags::Was_rendered);
			obj_queue_render(objp, &scene);
		}
	}

	scene.init_render();

	scene.render_all(ZBUFFER_TYPE_FULL);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_zbias(0);
	gr_set_cull(0);

	gr_clear_states();
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	decals::renderAll();

 	gr_deferred_lighting_end();
	gr_deferred_lighting_finish();

	gr_zbuffer_set(ZBUFFER_TYPE_READ);

	gr_reset_lighting();
	gr_set_lighting(false, false);

	// now render transparent meshes
	scene.render_all(ZBUFFER_TYPE_READ);
	scene.render_all(ZBUFFER_TYPE_NONE);

	// render electricity effects and insignias
	scene.render_outlines();
	scene.render_insignias();
	scene.render_arcs();

	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_zbias(0);
	gr_set_cull(0);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_clear_states();

	gr_reset_lighting();
	gr_set_lighting(false, false);

	batching_render_all();

	gr_zbias(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_cull(0);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_clear_states();

	gr_reset_lighting();
	gr_set_lighting(false, false);
}

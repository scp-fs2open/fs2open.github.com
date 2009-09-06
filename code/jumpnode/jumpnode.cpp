/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



int Num_jump_nodes = 0;

#include "object/object.h"
#include "jumpnode/jumpnode.h"
#include "model/model.h"
#include "hud/hud.h"
#include "globalincs/linklist.h"

linked_list<jump_node> Jump_nodes;

void jumpnode_level_close()
{
	jump_node *jnp;
	jump_node *next_node;

	for ( jnp = Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = next_node ) {	
		next_node = jnp->get_next();
		delete jnp;
	}

	//This could mean a memory leak
	if(Num_jump_nodes!=0)Warning(LOCATION, "Num_jump_nodes should be 0, but is actually %d.", Num_jump_nodes);
}

void jump_node::render(vec3d *pos, vec3d *view_pos)
{
	if(m_flags & JN_HIDE)
		return;

	if(m_modelnum < 0)
		return;

	matrix				node_orient = IDENTITY_MATRIX;

	int mr_flags = MR_NO_LIGHTING | MR_LOCK_DETAIL;
	if(!(m_flags & JN_SHOW_POLYS)) {
		mr_flags |= MR_NO_CULL | MR_NO_POLYS | MR_SHOW_OUTLINE_PRESET;
	}

	if ( Fred_running ) {
		gr_set_color_fast(&m_display_color);		
		model_render(m_modelnum, &node_orient, pos, mr_flags );
	} else {
		if (m_flags & JN_USE_DISPLAY_COLOR) {
			gr_set_color_fast(&m_display_color);
		}
		else if ( view_pos != NULL) {
			int alpha_index = HUD_color_alpha;

			// generate alpha index based on distance to jump this
			float dist;

			dist = vm_vec_dist_quick(view_pos, pos);

			// linearly interpolate alpha.  At 1000m or less, full intensity.  At 10000m or more 1/2 intensity.
			if ( dist < 1000 ) {
				alpha_index = HUD_COLOR_ALPHA_USER_MAX - 2;
			} else if ( dist > 10000 ) {
				alpha_index = HUD_COLOR_ALPHA_USER_MIN;
			} else {
				alpha_index = fl2i( HUD_COLOR_ALPHA_USER_MAX - 2 + (dist-1000) * (HUD_COLOR_ALPHA_USER_MIN-HUD_COLOR_ALPHA_USER_MAX-2) / (9000) + 0.5f);
				if ( alpha_index < HUD_COLOR_ALPHA_USER_MIN ) {
					alpha_index = HUD_COLOR_ALPHA_USER_MIN;
				}
			}

	//		nprintf(("Alan","alpha index is: %d\n", alpha_index));
			gr_set_color_fast(&HUD_color_defaults[alpha_index]);
//			model_set_outline_color(HUD_color_red, HUD_color_green, HUD_color_blue);

		} else {
			gr_set_color(HUD_color_red, HUD_color_green, HUD_color_blue);
		}
		model_render(m_modelnum, &node_orient, pos, mr_flags );
	}

}

void jump_node::set_model(char *model_name, bool show_polys)
{
	//Try to load the new model; if we can't, then we can't set it
	//WMC - TODO, change ferror back to -1
	int new_model = model_load(model_name, 0, NULL, 0);

	if(new_model == -1)
	{
		Warning(LOCATION, "Couldn't load model file %s for jump node %s", model_name, m_name);
		return;
	}
	
	//If there's an old model, unload it
	if(m_modelnum != -1)
		model_unload(m_modelnum);

	//Now actually set stuff
	m_modelnum = new_model;
	m_flags |= JN_SPECIAL_MODEL;

	//Do we want to change poly showing?
	if(show_polys)
		m_flags |= JN_SHOW_POLYS;
	else
		m_flags &= ~JN_SHOW_POLYS;
}

void jump_node::set_alphacolor(int r, int g, int b, int alpha)
{
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	if (alpha < 0) alpha = 0;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (alpha > 255) alpha = 255;

	m_flags |= JN_USE_DISPLAY_COLOR;
	gr_init_alphacolor(&m_display_color, r, g, b, alpha);
}

// create a jump node object and return pointer to it.
jump_node::jump_node(vec3d *pos)
{
	int obj;
	float radius;

	//Set name
	sprintf(m_name, XSTR( "Jump Node %d", 632), Num_jump_nodes);

	//Set model
	//WMC - TODO, change ferror back to -1
	m_modelnum = model_load(NOX("subspacenode.pof"), 0, NULL, 0);
	if ( m_modelnum < 0 ) {
		Warning(LOCATION, "Could not load default model for %s", m_name);
		radius = 0.0f;
	} else {
		radius = model_get_radius(m_modelnum);
	}

	//Set default color
	gr_init_alphacolor(&m_display_color, 0, 255, 0, 255);

	//Set flags
	m_flags = 0;

	//Create the object
	obj = obj_create(OBJ_JUMP_NODE, -1, -1, NULL, pos, radius, OF_RENDERS);

	if (obj >= 0)
	{
		Objects[obj].jnp = this;
		m_objnum = obj;
	}

	//Add it
	Jump_nodes.append(this);
	Num_jump_nodes++;
}

jump_node::~jump_node()
{
	// fred does something special with jumpnodes so
	// don't delete object in that case
	if (!Fred_running)
		obj_delete(m_objnum);

	//We now return you to your scheduled deletion
	Jump_nodes.remove(this);
	Num_jump_nodes--;
}

jump_node *jumpnode_get_by_name(char* name)
{
	jump_node *jnp;

	for ( jnp = Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = jnp->get_next() ) {	
		if(!stricmp(jnp->get_name_ptr(), name)) return jnp;
	}

	return NULL;
}

// only called by FRED
void jumpnode_render_all()
{
	jump_node *jnp;

	for ( jnp = Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = jnp->get_next() ) {	
		jnp->render(&jnp->get_obj()->pos);
	}
}

jump_node *jumpnode_get_which_in(object *objp)
{
	jump_node *jnp;
	float radius, dist;

	for ( jnp = Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = jnp->get_next() )
	{	
		//WMC - if a jump node has no model, who cares?
		if(jnp->get_modelnum() < 0)
			continue;

		radius = model_get_radius( jnp->get_modelnum() );
		dist = vm_vec_dist( &objp->pos, &jnp->get_obj()->pos );
		if ( dist <= radius ) {
			return jnp;
		}
	}

	return NULL;
}

bool jumpnode_check_for_duplicates()
{
	jump_node *reference, *other;
	for (reference = Jump_nodes.get_first(); !Jump_nodes.is_end(reference); reference = reference->get_next())
	{
		for (other =  reference->get_next(); !Jump_nodes.is_end(other); other = other->get_next())
		{
			if (!stricmp(reference->get_name_ptr(), other->get_name_ptr()))
				return true;
		}
	}

	return false;
}

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

SCP_list<jump_node> Jump_nodes;

/**
 * Constructor for jump_node object
 */
jump_node::jump_node(vec3d *pos)
{	
	Assert(pos != NULL);
	
	this->radius = 0.0f;
	this->m_modelnum = -1;
	this->m_objnum = -1;
	this->m_flags = 0;
	gr_init_alphacolor(&this->m_display_color, 0, 255, 0, 255);
	
	// Set name
	sprintf(this->m_name, XSTR( "Jump Node %d", 632), Jump_nodes.size());
	
	// Set model
	this->m_modelnum = model_load(NOX("subspacenode.pof"), 0, NULL, 0);
	if (this->m_modelnum < 0)
		Warning(LOCATION, "Could not load default model for %s", this->m_name);
	else
		this->radius = model_get_radius(this->m_modelnum);
	
	// Create the object
	this->m_objnum = obj_create(OBJ_JUMP_NODE, -1, -1, NULL, pos, this->radius, OF_RENDERS);
	if (this->m_objnum >= 0)
		Objects[this->m_objnum].jnp = this;
}

jump_node::~jump_node()
{
	model_unload(m_modelnum);
	obj_delete(m_objnum);
}

color jump_node::get_color()
{
	return m_display_color;
}

int jump_node::get_modelnum()
{
	return m_modelnum;
}

int jump_node::get_objnum()
{
	return m_objnum;
}

object *jump_node::get_obj()
{
	return &Objects[m_objnum];
}

char *jump_node::get_name_ptr()
{
	return m_name;
}

bool jump_node::is_hidden()
{
	if(m_flags & JN_HIDE)
		return true;
	else
		return false;
}

bool jump_node::is_colored()
{
	return ((m_flags & JN_USE_DISPLAY_COLOR) != 0);
}

bool jump_node::is_special_model()
{
	return ((m_flags & JN_SPECIAL_MODEL) != 0);
}

void jump_node::set_alphacolor(int r, int g, int b, int alpha)
{
	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);
	CLAMP(alpha, 0, 255);
	
	m_flags |= JN_USE_DISPLAY_COLOR;
	gr_init_alphacolor(&m_display_color, r, g, b, alpha);
}

void jump_node::set_model(char *model_name, bool show_polys)
{
	Assert(model_name != NULL);
	
	//Try to load the new model; if we can't, then we can't set it
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

/**
 * Set jump node name
 *
 * @param new_name New name to set
 */
void jump_node::set_name(char *new_name)
{
	Assert(new_name != NULL);
	
	strcpy_s(this->m_name, new_name);
}

/**
 * Set appearance, hidden or not
 */
void jump_node::show(bool enabled)
{
	if(enabled)
		m_flags&=~JN_HIDE;
	else
		m_flags|=JN_HIDE;
}

/**
 * Render jump node
 *
 * @param pos		World position
 * @param view_pos	Viewer's world position
 */
void jump_node::render(vec3d *pos, vec3d *view_pos)
{
	Assert(pos != NULL);
	
	if(m_flags & JN_HIDE)
		return;
	
	if(m_modelnum < 0)
		return;
	
	matrix node_orient = IDENTITY_MATRIX;
	
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
			
			gr_set_color_fast(&HUD_color_defaults[alpha_index]);
			
		} else {
			gr_set_color(HUD_color_red, HUD_color_green, HUD_color_blue);
		}
		model_render(m_modelnum, &node_orient, pos, mr_flags );
	}
	
}

/**
 * Get jump node by given name
 *
 * @param name Name of jump node
 * @return Jump node object
 */
jump_node *jumpnode_get_by_name(char* name)
{
	Assert(name != NULL);
	SCP_list<jump_node>::iterator jnp;

	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {	
		if(!stricmp(jnp->get_name_ptr(), name)) 
			return &(*jnp);
	}

	return NULL;
}

/**
 * Given an object, returns which jump node it's in (if any)
 *
 * @param objp Object
 * @return Jump node object or NULL if not in one
 */
jump_node *jumpnode_get_which_in(object *objp)
{
	Assert(objp != NULL);
	SCP_list<jump_node>::iterator jnp;
	float radius, dist;

	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
		//WMC - if a jump node has no model, who cares?
		if(jnp->get_modelnum() < 0)
			continue;

		radius = model_get_radius( jnp->get_modelnum() );
		dist = vm_vec_dist( &objp->pos, &jnp->get_obj()->pos );
		if ( dist <= radius ) {
			return &(*jnp);
		}
	}

	return NULL;
}

// only called by FRED
void jumpnode_render_all()
{
	SCP_list<jump_node>::iterator jnp;
	
	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {	
		jnp->render(&jnp->get_obj()->pos);
	}
}

bool jumpnode_check_for_duplicates()
{
	SCP_list<jump_node>::iterator reference, other;
	
	for (reference = Jump_nodes.begin(); reference != Jump_nodes.end(); ++reference)
	{
		for (other = Jump_nodes.begin(); other != Jump_nodes.end(); ++other)
		{
			if (!stricmp(reference->get_name_ptr(), other->get_name_ptr()))
				return true;
		}
	}

	return false;
}
	
void jumpnode_level_close()
{
	Jump_nodes.clear();
}

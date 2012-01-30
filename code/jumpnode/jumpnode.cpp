/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "object/object.h"
#include "jumpnode/jumpnode.h"
#include "model/model.h"
#include "hud/hud.h"
#include "globalincs/linklist.h"

SCP_list<jump_node> Jump_nodes;

/**
 * Constructor for jump_node object, default
 */
jump_node::jump_node() : m_radius(0.0f), m_modelnum(-1), m_objnum(-1), m_flags(0)
{	
    gr_init_alphacolor(&m_display_color, 0, 255, 0, 255);

	m_name[0] = '\0';
	
    pos.xyz.x = 0.0f;
	pos.xyz.y = 0.0f;
	pos.xyz.z = 0.0f;
}

/**
 * Constructor for jump_node object, with world position argument
 */
jump_node::jump_node(vec3d *position) : m_radius(0.0f), m_modelnum(-1), m_objnum(-1), m_flags(0)
{	
	Assert(position != NULL);
	
	gr_init_alphacolor(&m_display_color, 0, 255, 0, 255);
	
	// Set m_name
	sprintf(m_name, XSTR( "Jump Node %d", 632), Jump_nodes.size());
	
	// Set m_modelnum and m_radius
	m_modelnum = model_load(NOX("subspacenode.pof"), 0, NULL, 0);
	if (m_modelnum == -1)
		Warning(LOCATION, "Could not load default model for %s", m_name);
	else
		m_radius = model_get_radius(m_modelnum);
	
    pos.xyz.x = position->xyz.x;
	pos.xyz.y = position->xyz.y;
	pos.xyz.z = position->xyz.z;
    
	// Create the object
	m_objnum = obj_create(OBJ_JUMP_NODE, -1, -1, NULL, &pos, m_radius, OF_RENDERS);
}

/**
 * Destructor for jump_node object
 */
jump_node::~jump_node()
{
	model_unload(m_modelnum);

	if (Objects[m_objnum].type != OBJ_NONE)
		obj_delete(m_objnum);
}

// Accessor functions for private variables

char *jump_node::get_name_ptr()
{
	return m_name;
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
	Assert(m_objnum != -1);
    return &Objects[m_objnum];
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

color jump_node::get_color()
{
	return m_display_color;
}

vec3d *jump_node::get_pos()
{
	return &pos;
}

// Settor functions for private variables

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
	m_radius = model_get_radius(m_modelnum);

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
	#ifndef NDEBUG
	jump_node* check = jumpnode_get_by_name(new_name);
	#endif
	Assert((check == this || !check));
	strcpy_s(m_name, new_name);
}

/**
 * Set appearance, hidden or not
 */
void jump_node::show(bool enabled)
{
	if(enabled)
	{
		m_flags&=~JN_HIDE;
	}
	else
	{
		// Untarget this node if it is already targeted
		if ( Player_ai->target_objnum == m_objnum )
			Player_ai->target_objnum = -1;
		m_flags|=JN_HIDE;
	}
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
    // Assert(view_pos != NULL);
	
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
	
void jumpnode_level_close()
{
	Jump_nodes.clear();
}

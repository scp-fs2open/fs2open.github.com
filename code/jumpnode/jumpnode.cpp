/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include "jumpnode/jumpnode.h"
#include "model/model.h"
#include "hud/hud.h"

SCP_list<CJumpNode> Jump_nodes;

/**
 * Constructor for CJumpNode class, default
 */
CJumpNode::CJumpNode() : m_radius(0.0f), m_modelnum(-1), m_objnum(-1), m_flags(0)
{	
    gr_init_alphacolor(&m_display_color, 0, 255, 0, 255);

	m_name[0] = '\0';
	
    m_pos.xyz.x = 0.0f;
    m_pos.xyz.y = 0.0f;
    m_pos.xyz.z = 0.0f;
}

/**
 * Constructor for CJumpNode class, with world position argument
 */
CJumpNode::CJumpNode(vec3d *position) : m_radius(0.0f), m_modelnum(-1), m_objnum(-1), m_flags(0)
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
	
    m_pos.xyz.x = position->xyz.x;
    m_pos.xyz.y = position->xyz.y;
    m_pos.xyz.z = position->xyz.z;
    
	// Create the object
    m_objnum = obj_create(OBJ_JUMP_NODE, -1, -1, NULL, &m_pos, m_radius, OF_RENDERS);
}

/**
 * Destructor for CJumpNode class
 */
CJumpNode::~CJumpNode()
{
	model_unload(m_modelnum);

	if (Objects[m_objnum].type != OBJ_NONE)
		obj_delete(m_objnum);
}

// Accessor functions for private variables

/**
 * @return Name of jump node
 */
char *CJumpNode::GetName()
{
	return m_name;
}

/**
 * @return Handle to model
 */
int CJumpNode::GetModelNumber()
{
	return m_modelnum;
}

/**
 * @return Index into Objects[]
 */
int CJumpNode::GetSCPObjectNumber()
{
	return m_objnum;
}

/**
 * @return Object
 */
object *CJumpNode::GetSCPObject()
{
	Assert(m_objnum != -1);
    return &Objects[m_objnum];
}

/**
 * @return Color of jump node when rendered
 */
color CJumpNode::GetColor()
{
	return m_display_color;
}

/**
 * @return World position of jump node
 */
vec3d *CJumpNode::GetPosition()
{
	return &m_pos;
}

// Settor functions for private variables

/**
 * Set jump node alpha and color
 *
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @param alpha Alpha component
 */
void CJumpNode::SetAlphaColor(int r, int g, int b, int alpha)
{
	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);
	CLAMP(alpha, 0, 255);
	
	m_flags |= JN_USE_DISPLAY_COLOR;
	gr_init_alphacolor(&m_display_color, r, g, b, alpha);
}

/**
 * Set jump node model to render
 *
 * @param model_name Name of model file to load
 * @param show_polys Whether to render wireframe or not
 */
void CJumpNode::SetModel(char *model_name, bool show_polys)
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
void CJumpNode::SetName(const char *new_name)
{
	Assert(new_name != NULL);
    
	#ifndef NDEBUG
	CJumpNode* check = jumpnode_get_by_name(new_name);
	Assert((check == this || !check));
	#endif
    
	strcpy_s(m_name, new_name);
}

/**
 * Set appearance, hidden or not
 *
 * @param enabled Visibility to set
 */
void CJumpNode::SetVisibility(bool enabled)
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

// Query functions

/**
 * @return Is the jump node hidden when rendering?
 */
bool CJumpNode::IsHidden()
{
	if(m_flags & JN_HIDE)
		return true;
	else
		return false;
}

/**
 * @return Is the jump node colored any other color than default white?
 */
bool CJumpNode::IsColored()
{
	return ((m_flags & JN_USE_DISPLAY_COLOR) != 0);
}

/**
 * @return Is the jump node model set differently from the default one?
 */
bool CJumpNode::IsSpecialModel()
{
	return ((m_flags & JN_SPECIAL_MODEL) != 0);
}

/**
 * Render jump node
 *
 * @param pos		World position
 * @param view_pos	Viewer's world position, can be NULL
 */
void CJumpNode::Render(vec3d *pos, vec3d *view_pos)
{
	Assert(pos != NULL);
    // Assert(view_pos != NULL); - view_pos can be NULL
	
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
CJumpNode *jumpnode_get_by_name(const char* name)
{
	Assert(name != NULL);
	SCP_list<CJumpNode>::iterator jnp;

	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {	
		if(!stricmp(jnp->GetName(), name)) 
			return &(*jnp);
	}

	return NULL;
}

/**
 * Given an object, returns which jump node it's inside (if any)
 *
 * @param objp Object
 * @return Jump node object or NULL if not in one
 */
CJumpNode *jumpnode_get_which_in(object *objp)
{
	Assert(objp != NULL);
	SCP_list<CJumpNode>::iterator jnp;
	float radius, dist;

	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
		if(jnp->GetModelNumber() < 0)
			continue;

		radius = model_get_radius( jnp->GetModelNumber() );
		dist = vm_vec_dist( &objp->pos, &jnp->GetSCPObject()->pos );
		if ( dist <= radius ) {
			return &(*jnp);
		}
	}

	return NULL;
}

/**
 * Render all function
 *
 * @note Only called by FRED
 */
void jumpnode_render_all()
{
	SCP_list<CJumpNode>::iterator jnp;
	
	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {	
		jnp->Render(&jnp->GetSCPObject()->pos);
	}
}

/**
 * Level cleanup
 */
void jumpnode_level_close()
{
	Jump_nodes.clear();
}

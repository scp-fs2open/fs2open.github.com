/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include "hud/hud.h"
#include "jumpnode/jumpnode.h"
#include "model/model.h"
#include "model/modelrender.h"

SCP_list<CJumpNode> Jump_nodes;

/**
 * Constructor for CJumpNode class, default
 */
CJumpNode::CJumpNode()
{	
    gr_init_alphacolor(&m_display_color, 0, 255, 0, 255);

	m_name[0] = '\0';
	m_display[0] = '\0';
	
    m_pos.xyz.x = 0.0f;
    m_pos.xyz.y = 0.0f;
    m_pos.xyz.z = 0.0f;
}

/**
 * Constructor for CJumpNode class, with world position argument
 */
CJumpNode::CJumpNode(const vec3d* position)
{	
	Assert(position != NULL);
	
	gr_init_alphacolor(&m_display_color, 0, 255, 0, 255);
	
	// Set m_name and m_display
	sprintf(m_name, XSTR( "Jump Node %d", 632), Jump_nodes.size());
	m_display[0] = '\0';
	
	// Set m_modelnum and m_radius
	m_modelnum = model_load(NOX(JN_DEFAULT_MODEL), nullptr, ErrorType::WARNING);
	if (m_modelnum == -1) {
		Warning(LOCATION, "Could not load default model for %s", m_name);
	} else {
		m_radius = model_get_radius(m_modelnum);
	}
	
    m_pos.xyz.x = position->xyz.x;
    m_pos.xyz.y = position->xyz.y;
    m_pos.xyz.z = position->xyz.z;
    
	// Create the object
    flagset<Object::Object_Flags> default_flags;
    default_flags.set(Object::Object_Flags::Renders);
    m_objnum = obj_create(OBJ_JUMP_NODE, -1, -1, NULL, &m_pos, m_radius, default_flags);

	if (m_modelnum >= 0) {
		// set up animation in case of instrinsic_rotate
		polymodel* pm = model_get(m_modelnum);

		if (pm->flags & PM_FLAG_HAS_INTRINSIC_MOTION) {
			m_polymodel_instance_num = model_create_instance(m_objnum, m_modelnum);
		}
	}
}

CJumpNode::CJumpNode(CJumpNode&& other) noexcept
	: m_radius(other.m_radius), m_modelnum(other.m_modelnum), m_objnum(other.m_objnum), m_polymodel_instance_num(other.m_polymodel_instance_num), m_flags(other.m_flags)
{
	other.m_radius = 0.0f;
	other.m_modelnum = -1;
	other.m_objnum = -1;
	other.m_polymodel_instance_num = -1;
	other.m_flags = 0;

	m_display_color = other.m_display_color;
	m_pos = other.m_pos;

	strcpy_s(m_name, other.m_name);
	strcpy_s(m_display, other.m_display);
}

CJumpNode& CJumpNode::operator=(CJumpNode&& other) noexcept
{
	if (this != &other)
	{
		m_radius = other.m_radius;
		m_modelnum = other.m_modelnum;
		m_objnum = other.m_objnum;
		m_flags = other.m_flags;
		m_polymodel_instance_num = other.m_polymodel_instance_num;

		other.m_radius = 0.0f;
		other.m_modelnum = -1;
		other.m_objnum = -1;
		other.m_flags = 0;
		other.m_polymodel_instance_num = -1;

		m_display_color = other.m_display_color;
		m_pos = other.m_pos;

		strcpy_s(m_name, other.m_name);
		strcpy_s(m_display, other.m_display);
	}

	return *this;
}

/**
 * Destructor for CJumpNode class
 */
CJumpNode::~CJumpNode()
{
	if (m_modelnum >= 0)
	{
		model_unload(m_modelnum);
	}

	if (m_objnum >= 0 && Objects[m_objnum].type != OBJ_NONE)
	{
		obj_delete(m_objnum);
	}
}

// Accessor functions for private variables

/**
 * @return Name of jump node
 */
const char *CJumpNode::GetName() const
{
	return m_name;
}

/**
 * @return Display Name of jump node
 */
const char* CJumpNode::GetDisplayName() const
{
	if (HasDisplayName())
		return m_display;
	else
		return m_name;
}

/**
 * @return Handle to model
 */
int CJumpNode::GetModelNumber() const
{
	return m_modelnum;
}

/**
 * @return Index into Objects[]
 */
int CJumpNode::GetSCPObjectNumber() const
{
	return m_objnum;
}

/**
 * @return Object
 */
const object *CJumpNode::GetSCPObject() const
{
	Assert(m_objnum != -1);
    return &Objects[m_objnum];
}

/**
 * @return Color of jump node when rendered
 */
const color &CJumpNode::GetColor() const
{
	return m_display_color;
}

/**
 * @return World position of jump node
 */
const vec3d *CJumpNode::GetPosition() const
{
	return &m_pos;
}

/*
* @return Polymodel Instance Index
*/
int CJumpNode::GetPolymodelInstanceNum() const
{
	return m_polymodel_instance_num;
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
	
	// see whether this is actually the default color
	// (which actually means to use the HUD color rather than this exact color;
	// it might be useful to change this design in the future, but beware of
	// FRED calling this function in the background)
	if (r == 0 && g == 255 && b == 0 && alpha == 255)
		m_flags &= ~JN_USE_DISPLAY_COLOR;
	else
		m_flags |= JN_USE_DISPLAY_COLOR;

	gr_init_alphacolor(&m_display_color, r, g, b, alpha);
}

/**
 * Set jump node model to render
 *
 * @param model_name Name of model file to load
 * @param show_polys Whether to render wireframe or not
 */
void CJumpNode::SetModel(const char *model_name, bool show_polys)
{
	Assert(model_name != NULL);
	
	//Try to load the new model; if we can't, then we can't set it
	int new_model = model_load(model_name, nullptr, ErrorType::WARNING);
	
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
	auto check = jumpnode_get_by_name(new_name);
	Assertion((check == this || !check), "Jumpnode %s is being renamed to %s, but a jump node with that name already exists in the mission!\n", m_name, new_name);
	#endif

	strcpy_s(m_name, new_name);

	// if this name has a hash, create a default display name
	if (get_pointer_to_first_hash_symbol(new_name))
	{
		strcpy_s(m_display, new_name);
		end_string_at_first_hash_symbol(m_display);
		m_flags |= JN_HAS_DISPLAY_NAME;
	}
}

/**
 * Set jump node display name
 *
 * @param new_display_name New name to set
 */
void CJumpNode::SetDisplayName(const char *new_display_name)
{
	Assert(new_display_name != NULL);

	// if display name is blank or matches the actual name, clear it
	if (*new_display_name == '\0' || !stricmp(new_display_name, m_name))
	{
		*m_display = '\0';
		m_flags &= ~JN_HAS_DISPLAY_NAME;
	}
	else
	{
		strcpy_s(m_display, new_display_name);
		m_flags |= JN_HAS_DISPLAY_NAME;
	}
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
		if ((Game_mode & GM_IN_MISSION) && Player_ai->target_objnum == m_objnum) {
			Player_ai->target_objnum = -1;
		}
		m_flags|=JN_HIDE;
	}
}

// Query functions

/**
 * @return Is the jump node hidden when rendering?
 */
bool CJumpNode::IsHidden() const
{
	if(m_flags & JN_HIDE)
		return true;
	else
		return false;
}

/**
 * @return Is the jump node colored any other color than default white?
 */
bool CJumpNode::IsColored() const
{
	return ((m_flags & JN_USE_DISPLAY_COLOR) != 0);
}

/**
 * @return Is the jump node model set differently from the default one?
 */
bool CJumpNode::IsSpecialModel() const
{
	return ((m_flags & JN_SPECIAL_MODEL) != 0);
}

/**
 * @return Does the jump node have a display name?
 */
bool CJumpNode::HasDisplayName() const
{
	return ((m_flags & JN_HAS_DISPLAY_NAME) != 0);
}

/**
* Render jump node. Creates its own draw list to render
*
* @param pos		World position
* @param view_pos	Viewer's world position, can be NULL
*/
void CJumpNode::Render(const vec3d *pos, const vec3d *view_pos) const
{
	model_draw_list scene;

	Render(&scene, pos, view_pos);

	scene.init_render();
	scene.render_all();
	scene.render_outlines();

	gr_set_fill_mode(GR_FILL_MODE_SOLID);
	gr_clear_states();
}

/**
* Render jump node
*
* @param scene		A scene's draw list
* @param pos		World position
* @param view_pos	Viewer's world position, can be NULL
*/
void CJumpNode::Render(model_draw_list *scene, const vec3d *pos, const vec3d *view_pos) const
{
	Assert(pos != NULL);
	// Assert(view_pos != NULL); - view_pos can be NULL

	if(m_flags & JN_HIDE)
		return;

	if(m_modelnum < 0)
		return;

	matrix node_orient = IDENTITY_MATRIX;

	uint64_t mr_flags = MR_NO_LIGHTING | MR_NO_BATCH;
	if(!(m_flags & JN_SHOW_POLYS)) {
		mr_flags |= MR_NO_CULL | MR_NO_POLYS | MR_SHOW_OUTLINE | MR_SHOW_OUTLINE_HTL | MR_NO_TEXTURING;
	}

	model_render_params render_info;

	render_info.set_object_number(m_objnum);
	render_info.set_detail_level_lock(0);
	render_info.set_flags(mr_flags);

	if ( Fred_running ) {
		render_info.set_color(m_display_color);

		model_render_queue(&render_info, scene, m_modelnum, &node_orient, pos);
	} else {
		if (m_flags & JN_USE_DISPLAY_COLOR) {
			//gr_set_color_fast(&m_display_color);
			render_info.set_color(m_display_color);
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
				alpha_index = (int)std::lround( HUD_COLOR_ALPHA_USER_MAX - 2 + (dist-1000) * (HUD_COLOR_ALPHA_USER_MIN-HUD_COLOR_ALPHA_USER_MAX-2) / (9000));
				if ( alpha_index < HUD_COLOR_ALPHA_USER_MIN ) {
					alpha_index = HUD_COLOR_ALPHA_USER_MIN;
				}
			}

			render_info.set_color(HUD_color_defaults[alpha_index]);
		} else {
			render_info.set_color(HUD_color_red, HUD_color_green, HUD_color_blue);
		}

		model_render_queue(&render_info, scene, m_modelnum, &node_orient, pos);
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
 * Get jump node object by the object number
 *
 * @param objnum to search for
 * @return Jump node object pointer
 */
CJumpNode *jumpnode_get_by_objnum(int objnum)
{
	Assert(objnum > -1);

	for (CJumpNode &jnp : Jump_nodes) {
		if (jnp.GetSCPObjectNumber() == objnum)
			return &(jnp);
	}

	return nullptr;
}

/**
 * Get jump node object by the object pointer
 *
 * @param objp to search for
 * @return Jump node object pointer
 */
CJumpNode *jumpnode_get_by_objp(const object *objp)
{
	Assert(objp != nullptr);

	for (CJumpNode &jnp : Jump_nodes) {
		if (jnp.GetSCPObject() == objp)
			return &(jnp);
	}

	return nullptr;
}

/**
 * Given an object, returns which jump node it's inside (if any)
 *
 * @param objp Object
 * @return Jump node object or NULL if not in one
 */
CJumpNode *jumpnode_get_which_in(const object *objp)
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

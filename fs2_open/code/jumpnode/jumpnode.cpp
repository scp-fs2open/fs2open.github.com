/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/JumpNode/JumpNode.cpp $
 * $Revision: 2.7 $
 * $Date: 2005-03-03 06:05:28 $
 * $Author: wmcoolmon $
 *
 * Module for everything to do with jump nodes
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2005/01/31 23:27:53  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.5  2004/07/26 20:47:35  Kazan
 * remove MCD complete
 *
 * Revision 2.4  2004/07/12 16:32:52  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.3  2004/07/01 01:12:32  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.2  2003/11/17 04:25:56  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 9     6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 8     5/12/98 2:03p Adam
 * make jumpnode less bright
 * 
 * 7     4/01/98 8:38p Lawrance
 * Add support for jump node icons in the briefings.
 * 
 * 6     3/24/98 4:27p Lawrance
 * Use new method for dimming lines
 * 
 * 5     3/24/98 12:05p Lawrance
 * Don't set alpha color for jumpnode
 * 
 * 4     3/23/98 11:05a Lawrance
 * Dim jump node as it get farther away
 * 
 * 3     3/21/98 7:43p Lawrance
 * Disable jump node dimming until bug with alpha colors is fixed
 * 
 * 2     3/21/98 7:36p Lawrance
 * Move jump nodes to own lib.
 * 
 * 1     3/21/98 3:53p Lawrance
 *
 * $NoKeywords: $
 */

int Num_jump_nodes = 0;

#include "object/object.h"
#include "jumpnode/jumpnode.h"
#include "model/model.h"
#include "hud/hud.h"
#include "globalincs/linklist.h"

linked_list Jump_nodes;

void jumpnode_level_close()
{
	jump_node *jnp;
	jump_node *next_node;

	for ( jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = next_node ) {	
		next_node = (jump_node *)jnp->get_next();
		delete jnp;
		Num_jump_nodes--;
	}

	//This could mean a memory leak
	if(Num_jump_nodes!=0)Warning(LOCATION, "Num_jump_nodes should be 0, but is actually %d.", Num_jump_nodes);
}

void jump_node::render(vector *pos, vector *view_pos)
{
	if(m_flags & JN_HIDE)
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
	int new_model = model_load(model_name, 0, NULL);

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

	//Do we want to change poly showing?
	if(show_polys)
		m_flags &= ~JN_SHOW_POLYS;
	else
		m_flags |= JN_SHOW_POLYS;
}

void jump_node::set_alphacolor(int r, int g, int b, int alpha)
{
	m_flags |= JN_USE_DISPLAY_COLOR;
	gr_init_alphacolor(&m_display_color, r, g, b, alpha);
}

// create a jump node object and return pointer to it.
jump_node::jump_node(vector *pos)
{
	int obj;

	//Set name
	sprintf(m_name, XSTR( "Jump Node %d", 632), Num_jump_nodes);

	//Set model
	m_modelnum = model_load(NOX("subspacenode.pof"), 0, NULL);
	if ( m_modelnum < 0 ) {
		Warning(LOCATION, "Could not load default model for %s", m_name);
	}

	//Set default color
	gr_init_alphacolor(&m_display_color, 0, 255, 0, 255);

	//Set flags
	m_flags = 0;

	//Create the object
	obj = obj_create(OBJ_JUMP_NODE, -1, -1, NULL, pos, model_get_radius(m_modelnum), OF_RENDERS);

	if (obj >= 0)
	{
		Objects[obj].jnp = this;
		m_objnum = obj;

		//Add it
		Jump_nodes.append(this);
		Num_jump_nodes++;
	}
}

jump_node::~jump_node()
{
	obj_delete(m_objnum);

	//We now return you to your scheduled deletion
	Jump_nodes.remove(this);
	Num_jump_nodes--;
}

jump_node *jumpnode_get_by_name(char* name)
{
	jump_node *jnp;

	for ( jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() ) {	
		if(!stricmp(jnp->get_name_ptr(), name)) return jnp;
	}

	return NULL;
}

// only called by FRED
void jumpnode_render_all()
{
	jump_node *jnp;

	for ( jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() ) {	
		jnp->render(&jnp->get_obj()->pos);
	}
}

jump_node *jumpnode_get_which_in(object *objp)
{
	jump_node *jnp;
	float radius, dist;

	for ( jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() )
	{	
		radius = model_get_radius( jnp->get_modelnum() );
		dist = vm_vec_dist( &objp->pos, &jnp->get_obj()->pos );
		if ( dist <= radius ) {
			return jnp;
		}
	}

	return NULL;
}
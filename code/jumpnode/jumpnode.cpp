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
 * $Revision: 2.4 $
 * $Date: 2004-07-12 16:32:52 $
 * $Author: Kazan $
 *
 * Module for everything to do with jump nodes
 *
 * $Log: not supported by cvs2svn $
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

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

jump_node_struct Jump_nodes[MAX_JUMP_NODES];

void jumpnode_render(object *jumpnode_objp, vector *pos, vector *view_pos)
{
	jump_node_struct	*node;
	matrix				node_orient = IDENTITY_MATRIX;

	node = &Jump_nodes[jumpnode_objp->instance];

	if ( Fred_running ) {
		model_set_outline_color(0, 255, 0);		
		model_render(node->modelnum, &node_orient, pos, MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_NO_POLYS | MR_SHOW_OUTLINE );
	} else {
		if ( view_pos ) {
			int alpha_index = HUD_color_alpha;

			// generate alpha index based on distance to jump node
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
		model_render(node->modelnum, &node_orient, pos, MR_NO_CULL | MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_NO_POLYS | MR_SHOW_OUTLINE_PRESET );
	}

}

// create a jump node object and return index to it.
int jumpnode_create(vector *pos, char* file_name)
{
	int obj;

	Assert(Num_jump_nodes < MAX_JUMP_NODES);

	Jump_nodes[Num_jump_nodes].modelnum = model_load(NOX("subspacenode.pof"), NULL, NULL);
	if ( Jump_nodes[Num_jump_nodes].modelnum < 0 ) {
		Int3();
		return -1;
	}

	obj = obj_create(OBJ_JUMP_NODE, -1, Num_jump_nodes, NULL, pos, model_get_radius(Jump_nodes[Num_jump_nodes].modelnum), OF_RENDERS);
	sprintf(Jump_nodes[Num_jump_nodes].name, XSTR( "Jump Node %d", 632), Num_jump_nodes);
	if (obj >= 0) {
		Jump_nodes[Num_jump_nodes].objnum = obj;
		Num_jump_nodes++;
	}
	return obj;
}

// only called by FRED
void jumpnode_render_all()
{
	int		i;
	object	*jumpnode_objp;

	for ( i = 0; i < Num_jump_nodes; i++ ) {	
		jumpnode_objp = &Objects[Jump_nodes[i].objnum];
		jumpnode_render(jumpnode_objp, &jumpnode_objp->pos);
	}
}

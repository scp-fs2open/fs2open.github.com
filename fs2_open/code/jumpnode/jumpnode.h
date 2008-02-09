/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/JumpNode/JumpNode.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:24 $
 * $Author: penguin $
 *
 * Header for everything to do with jump nodes
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 2     3/21/98 7:36p Lawrance
 * Move jump nodes to own lib.
 * 
 * 1     3/21/98 3:53p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __JUMPNODE_H__
#define __JUMPNODE_H__

#include "parselo.h"

#define MAX_JUMP_NODES	3

typedef struct {
	int	modelnum;
	int	objnum;						// objnum of this jump node
	char	name[NAME_LENGTH];
} jump_node_struct;

extern int Num_jump_nodes;
extern jump_node_struct Jump_nodes[MAX_JUMP_NODES];

int	jumpnode_create(vector *pos);
void	jumpnode_render(object *jumpnode_objp, vector *pos, vector *view_pos = NULL);
void	jumpnode_render_all();	// called by FRED

#endif

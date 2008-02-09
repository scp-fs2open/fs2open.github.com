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
 * $Revision: 2.13 $
 * $Date: 2005-07-13 03:15:53 $
 * $Author: Goober5000 $
 *
 * Header for everything to do with jump nodes
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.12  2005/04/05 05:53:18  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.11  2005/03/29 06:28:52  wmcoolmon
 * Fixed extern to compile under Linux
 *
 * Revision 2.10  2005/03/29 03:42:07  phreak
 * global variable "Jump_nodes" needs external linkage for FRED
 *
 * Revision 2.9  2005/03/25 06:57:34  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.8  2005/03/20 21:07:10  phreak
 * Add a function to check whether any duplicate jumpnode names exist
 *
 * Revision 2.7  2005/03/11 01:28:23  wmcoolmon
 * Oops, gotta remember to commit main codebase stuff when fixing FRED
 *
 * Revision 2.6  2005/03/03 06:05:28  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.5  2004/08/11 05:06:26  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.4  2004/07/01 01:12:32  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.3  2004/03/05 09:02:04  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2002/12/17 01:56:36  DTP
 * Bumped MAX JUMP-nodes to 50. if anybody wants a Jump-node forest
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
 * 2     3/21/98 7:36p Lawrance
 * Move jump nodes to own lib.
 * 
 * 1     3/21/98 3:53p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __JUMPNODE_H__
#define __JUMPNODE_H__

#include <stdlib.h>

#include "globalincs/globals.h"
#include "globalincs/linklist.h"
#include "graphics/2d.h"

struct vec3d;
struct object;

//#define MAX_JUMP_NODES	50
//No max anymore. Why you'd want more than 50 is anybody's guess though.

//Jump node flags
#define JN_USE_DISPLAY_COLOR		(1<<0)		//Use display_color instead of HUD color
#define JN_SHOW_POLYS				(1<<1)		//Display model normally, rather than as wireframe
#define JN_HIDE						(1<<2)		//Hides a jump node

class jump_node : public linked_list<jump_node>
{
	char m_name[NAME_LENGTH];

	int	m_modelnum;
	int	m_objnum;						// objnum of this jump node

	int m_flags;
	color m_display_color;			// Color node will be shown in (Default:0/255/0/255)
public:
	//Construction
	jump_node(vec3d *pos);
	~jump_node();
	
	//Getting
	int get_modelnum(){return m_modelnum;}
	int get_objnum(){return m_objnum;}
	object *get_obj(){return &Objects[m_objnum];}
	char *get_name_ptr(){return m_name;}
	bool is_hidden(){if(m_flags & JN_HIDE){return true;}else{return false;}}

	//Setting
	void set_alphacolor(int r, int g, int b, int alpha);
	void set_model(char *model_name, bool show_polys=false);
	void show(bool show){if(show){m_flags&=~JN_HIDE;}else{m_flags|=JN_HIDE;}}

	//Rendering
	void render(vec3d *pos, vec3d *view_pos = NULL);
};

//-----Globals------
//extern int Num_jump_nodes;
//extern linked_list Jump_nodes;
extern linked_list<jump_node> Jump_nodes;

//-----Functions-----
//Given a name, returns pointer to the jump node object
jump_node *jumpnode_get_by_name(char *name);

//Given an object, returns which jump node it's in
jump_node *jumpnode_get_which_in(object *objp);

void jumpnode_level_init();
void jumpnode_render_all();	// called by FRED
void jumpnode_level_close();

bool jumpnode_check_for_duplicates();

#endif

/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
#define JN_SPECIAL_MODEL			(1<<3)		//If non-default model

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
	color get_color(){return m_display_color;}
	int get_modelnum(){return m_modelnum;}
	int get_objnum(){return m_objnum;}
	object *get_obj(){return &Objects[m_objnum];}
	char *get_name_ptr(){return m_name;}
	bool is_hidden(){if(m_flags & JN_HIDE){return true;}else{return false;}}
	bool is_colored(){return ((m_flags & JN_USE_DISPLAY_COLOR) != 0);}
	bool is_special_model(){return ((m_flags & JN_SPECIAL_MODEL) != 0);}

	//Setting
	void set_alphacolor(int r, int g, int b, int alpha);
	void set_model(char *model_name, bool show_polys=false);
	void show(bool enabled){if(enabled){m_flags&=~JN_HIDE;}else{m_flags|=JN_HIDE;}}

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

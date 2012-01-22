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
#include "graphics/2d.h"

struct vec3d;
struct object;

//Jump node flags
#define JN_USE_DISPLAY_COLOR		(1<<0)		//Use display_color instead of HUD color
#define JN_SHOW_POLYS				(1<<1)		//Display model normally, rather than as wireframe
#define JN_HIDE						(1<<2)		//Hides a jump node
#define JN_SPECIAL_MODEL			(1<<3)		//If non-default model

class jump_node
{
private:
	char m_name[NAME_LENGTH];
	float m_radius;

	int	m_modelnum;
	int	m_objnum;						// objnum of this jump node

	int m_flags;
	color m_display_color;			// Color node will be shown in (Default:0/255/0/255)
    vec3d pos;
public:
	//Constructors
    jump_node();
	jump_node(vec3d *position);
    
    //Destructor
	~jump_node();
	
	//Getting
    char *get_name_ptr();
	int get_modelnum();
	int get_objnum();
	object *get_obj();
	bool is_hidden();
	bool is_colored();
	bool is_special_model();
    color get_color();
    vec3d *get_pos();

	//Setting
	void set_alphacolor(int r, int g, int b, int alpha);
	void set_model(char *model_name, bool show_polys=false);
	void set_name(char *new_name);
	void show(bool enabled);

	//Rendering
	void render(vec3d *pos, vec3d *view_pos = NULL);
};

//-----Globals------
extern SCP_list<jump_node> Jump_nodes;

//-----Functions-----
jump_node *jumpnode_get_by_name(char *name);
jump_node *jumpnode_get_which_in(object *objp);

void jumpnode_render_all();
void jumpnode_level_close();

#endif

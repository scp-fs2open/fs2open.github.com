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

class CJumpNode
{
private:
    char m_name[NAME_LENGTH];
    float m_radius;

    int	m_modelnum;
    int	m_objnum;						// objnum of this jump node

    int m_flags;
    color m_display_color;			// Color node will be shown in (Default:0/255/0/255)
    vec3d m_pos;
public:
    //Constructors
    CJumpNode();
    CJumpNode(vec3d *position);
    
    //Destructor
    ~CJumpNode();
	
	//Getting
    char *GetName();
    int GetModelNumber();
    int GetSCPObjectNumber();
    object *GetSCPObject();
    color GetColor();
    vec3d *GetPosition();

    //Setting
    void SetAlphaColor(int r, int g, int b, int alpha);
    void SetModel(char *model_name, bool show_polys=false);
    void SetName(const char *new_name);
    void SetVisibility(bool enabled);
    
    //Query
    bool IsHidden();
    bool IsColored();
    bool IsSpecialModel();

    //Rendering
    void Render(vec3d *pos, vec3d *view_pos = NULL);
};

//-----Globals------
extern SCP_list<CJumpNode> Jump_nodes;

//-----Functions-----
CJumpNode *jumpnode_get_by_name(const char *name);
CJumpNode *jumpnode_get_which_in(object *objp);

void jumpnode_render_all();
void jumpnode_level_close();

#endif

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

#include <cstdlib>

#include "globalincs/globals.h"
#include "graphics/2d.h"

struct vec3d;
class object;

class model_draw_list;

//Jump node flags
#define JN_USE_DISPLAY_COLOR		(1<<0)		//Use display_color instead of HUD color
#define JN_SHOW_POLYS				(1<<1)		//Display model normally, rather than as wireframe
#define JN_HIDE						(1<<2)		//Hides a jump node
#define JN_SPECIAL_MODEL			(1<<3)		//If non-default model
#define JN_HAS_DISPLAY_NAME			(1<<4)		//If it has a display name

#define JN_DEFAULT_MODEL            "subspacenode.pof"

class CJumpNode
{
private:
	char m_name[NAME_LENGTH];
	char m_display[NAME_LENGTH];
	float m_radius {0.0f};

	int	m_modelnum {-1};
	int m_objnum {-1};                 // objnum of this jump node
	int m_polymodel_instance_num {-1}; // polymodel instance number, used for rotations 

	int m_flags {0};
	color m_display_color;			// Color node will be shown in (Default:0/255/0/255)
	vec3d m_pos;

	CJumpNode(const CJumpNode&);
	CJumpNode& operator=(const CJumpNode&) = delete;
public:
	//Constructors
	CJumpNode();
	CJumpNode(const vec3d *position);
	CJumpNode(CJumpNode&& other) noexcept;

	CJumpNode& operator=(CJumpNode&&) noexcept;

	//Destructor
	~CJumpNode();

	//Getting
	const char *GetName() const;
	const char *GetDisplayName() const;
	int GetModelNumber() const;
	int GetSCPObjectNumber() const;
	int GetPolymodelInstanceNum() const;
	const object *GetSCPObject() const;
	const color &GetColor() const;
	const vec3d *GetPosition() const;

	//Setting
	void SetAlphaColor(int r, int g, int b, int alpha);
	void SetModel(const char *model_name, bool show_polys = false);
	void SetName(const char *new_name);
	void SetDisplayName(const char* new_name);
	void SetVisibility(bool enabled);

	//Query
	bool IsHidden() const;
	bool IsColored() const;
	bool IsSpecialModel() const;
	bool HasDisplayName() const;

	//Rendering
	void Render(const vec3d *pos, const vec3d *view_pos = nullptr) const;
	void Render(model_draw_list *scene, const vec3d *pos, const vec3d *view_pos = nullptr) const;
};

//-----Globals------
extern SCP_list<CJumpNode> Jump_nodes;

//-----Functions-----
CJumpNode *jumpnode_get_by_name(const char *name);
CJumpNode *jumpnode_get_by_objnum(int objnum);
CJumpNode *jumpnode_get_by_objp(const object *objp);
CJumpNode *jumpnode_get_which_in(const object *objp);

void jumpnode_render_all();
void jumpnode_level_close();

#endif

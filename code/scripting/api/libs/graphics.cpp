//
//

#include "graphics.h"

#include "scripting/api/objs/camera.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/font.h"
#include "scripting/api/objs/model.h"
#include "scripting/api/objs/movie_player.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/particle.h"
#include "scripting/api/objs/streaminganim.h"
#include "scripting/api/objs/subsystem.h"
#include "scripting/api/objs/texture.h"
#include "scripting/api/objs/vecmath.h"

#include <asteroid/asteroid.h>
#include <camera/camera.h>
#include <debris/debris.h>
#include <freespace.h>
#include <globalincs/systemvars.h>
#include <graphics/2d.h>
#include <graphics/material.h>
#include <graphics/matrix.h>
#include <hud/hudbrackets.h>
#include <hud/hudtarget.h>
#include <jumpnode/jumpnode.h>
#include <model/modelrender.h>
#include <parse/parselo.h>
#include <render/3d.h>
#include <render/3dinternal.h>
#include <render/batching.h>
#include <scripting/scripting.h>
#include <ship/ship.h>
#include <weapon/weapon.h>
#include <cmath>

namespace {

static const int NextDrawStringPosInitial[] = {0, 0};
static int NextDrawStringPos[] = {NextDrawStringPosInitial[0], NextDrawStringPosInitial[1]};
static fix PreviousFrametimeOverall = 0;

static bool WarnedBadThicknessLine = false;

}

namespace scripting {
namespace api {

model_draw_list *Current_scene = nullptr;

//**********LIBRARY: Graphics
ADE_LIB(l_Graphics, "Graphics", "gr", "Graphics Library");

static float lua_Opacity = 1.0f;
static int lua_Opacity_type = GR_ALPHABLEND_FILTER;
static int lua_ResizeMode = GR_RESIZE_NONE;

//****SUBLIBRARY: Graphics/Cameras
ADE_LIB_DERIV(l_Graphics_Cameras, "Cameras", NULL, "Cameras", l_Graphics);

ADE_INDEXER(l_Graphics_Cameras, "number/string IndexOrName", "Gets camera", "camera", "Ship handle, or invalid ship handle if index was invalid")
{
	const char* s = nullptr;
	if(!ade_get_args(L, "*s", &s))
		return ade_set_error(L, "o", l_Camera.Set(camid()));

	camid cid = cam_lookup(s);
	if(!cid.isValid())
	{
		int cn = atoi(s);
		if(cn > 0)
		{
			//Lua-->FS2
			cn--;
			cid = cam_get_camera(cn);
		}
	}

	return ade_set_args(L, "o", l_Camera.Set(cid));
}

ADE_FUNC(__len, l_Graphics_Cameras, NULL, "Gets number of cameras", "number", "Number of cameras")
{
	return ade_set_args(L, "i", (int)cam_get_num());
}

//****SUBLIBRARY: Graphics/Fonts
ADE_LIB_DERIV(l_Graphics_Fonts, "Fonts", nullptr, "Font library", l_Graphics);

ADE_FUNC(__len, l_Graphics_Fonts, NULL, "Number of loaded fonts", "number", "Number of loaded fonts")
{
	return ade_set_args(L, "i", font::FontManager::numberOfFonts());
}

ADE_INDEXER(l_Graphics_Fonts, "number/string IndexOrFilename", "Array of loaded fonts", "font", "Font handle, or invalid font handle if index is invalid")
{
	if (lua_isnumber(L, 2))
	{
		int index = -1;

		if (!ade_get_args(L, "*i", &index))
			return ade_set_error(L, "o", l_Font.Set(font_h()));

		auto realIdx = index - 1;

		if (realIdx < 0 || realIdx >= font::FontManager::numberOfFonts())
		{
			return ade_set_error(L, "o", l_Font.Set(font_h()));
		}

		return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getFont(index - 1))));
	}
	else
	{
		const char* s = nullptr;

		if(!ade_get_args(L, "*s", &s))
			return ade_set_error(L, "o", l_Font.Set(font_h()));

		return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getFont(s))));
	}
}

ADE_VIRTVAR(CurrentFont, l_Graphics, "font", "Current font", "font", NULL)
{
	font_h *newFh = NULL;

	if(!ade_get_args(L, "*|o", l_Font.GetPtr(&newFh)))
		return ade_set_error(L, "o", l_Font.Set(font_h()));

	if(ADE_SETTING_VAR && newFh->isValid()) {
		font::FontManager::setCurrentFont(newFh->Get());
	}

	return ade_set_args(L, "o", l_Font.Set(font_h(font::FontManager::getCurrentFont())));
}

//****SUBLIBRARY: Graphics/PostEffects
ADE_LIB_DERIV(l_Graphics_Posteffects, "PostEffects", nullptr, "Post-processing effects", l_Graphics);

ADE_INDEXER(l_Graphics_Posteffects, "number index", "Gets the name of the specified post-processing index", "string", "post-processing name or empty string on error")
{
	int index = -1;
	if(!ade_get_args(L, "*i", &index))
		return ade_set_error(L, "s", "");

	index--; // Lua -> C/C++

	if (index < 0)
		return ade_set_error(L, "s", "");

	SCP_vector<SCP_string> names;
	gr_get_post_process_effect_names(names);
	names.push_back(SCP_string("lightshafts"));

	if (index >= (int) names.size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", const_cast<char*>(names[index].c_str()));
}

ADE_FUNC(__len, l_Graphics_Posteffects, nullptr, "Gets the number of available post-processing effects", "number", "number of post-processing effects or 0 on error")
{
	SCP_vector<SCP_string> names;
	gr_get_post_process_effect_names(names);

	// Add one for lightshafts
	return ade_set_args(L, "i", ((int) names.size()) + 1);
}

ADE_FUNC(setPostEffect, l_Graphics, "string name, [number value=0, number red=0.0, number green=0.0, number blue=0.0]", "Sets the intensity of the specified post-processing effect. Optionally sets RGB values for effects that use them (valid values are 0.0 to 1.0)", "boolean", "true when successful, false otherwise")
{
	const char* name     = nullptr;
	int intensity = 0;
	vec3d rgb; rgb.xyz.x = 0.0f; rgb.xyz.y = 0.0f; rgb.xyz.z = 0.0f; // clang you are a PITA

	if (!ade_get_args(L, "s|ifff", &name, &intensity, &rgb.xyz.x, &rgb.xyz.y, &rgb.xyz.z))
		return ADE_RETURN_FALSE;

	if (name == nullptr || intensity < 0)
		return ADE_RETURN_FALSE;

	CAP(rgb.xyz.x, 0.0f, 1.0f);
	CAP(rgb.xyz.y, 0.0f, 1.0f);
	CAP(rgb.xyz.z, 0.0f, 1.0f);

	gr_post_process_set_effect(name, intensity, &rgb);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetPostEffects, l_Graphics, nullptr, "Resets all post-processing effects to their default values", "boolean", "true if successful, false otherwise")
{
	gr_post_process_set_defaults();

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(CurrentOpacityType, l_Graphics, "enumeration", "Current alpha blending type; uses ALPHABLEND_* enumerations", "enumeration", NULL)
{
	enum_h *alphatype = NULL;

	if(!ade_get_args(L, "*|o", l_Enum.GetPtr(&alphatype)))
		return ADE_RETURN_NIL;

	if(ADE_SETTING_VAR)
	{
		if(alphatype != NULL && alphatype->index == LE_ALPHABLEND_FILTER)
			lua_Opacity_type = GR_ALPHABLEND_FILTER;
		else
			lua_Opacity_type = GR_ALPHABLEND_NONE;
	}

	lua_enum rtn;
	switch(lua_Opacity_type)
	{
		case GR_ALPHABLEND_FILTER:
			rtn = LE_ALPHABLEND_FILTER;
			break;
		default:
			rtn = LE_ALPHABLEND_NONE;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(rtn)));
}

ADE_VIRTVAR(CurrentRenderTarget, l_Graphics, "texture", "Current rendering target", "texture", "Current rendering target, or invalid texture handle if screen is render target")
{
	texture_h* newtx = nullptr;

	if(ADE_SETTING_VAR && lua_isnil(L, 2))
	{
		bm_set_render_target(-1);
		return ade_set_args(L, "o", l_Texture.Set(texture_h(gr_screen.rendering_to_texture)));
	}
	else
	{
		if (!ade_get_args(L, "*|o", l_Texture.GetPtr(&newtx)))
			return ade_set_error(L, "o", l_Texture.Set(texture_h()));

		if(ADE_SETTING_VAR) {
			if (newtx != nullptr && newtx->isValid())
				bm_set_render_target(newtx->handle, 0);
			else
				bm_set_render_target(-1);
		}

		return ade_set_args(L, "o", l_Texture.Set(texture_h(gr_screen.rendering_to_texture)));
	}
}

ADE_VIRTVAR(CurrentResizeMode, l_Graphics, "enumeration ResizeMode", "Current resize mode; uses GR_RESIZE_* enumerations.  This resize mode will be used by the gr.* drawing methods.", "enumeration", nullptr)
{
	enum_h resize_arg;

	if (!ade_get_args(L, "*|o", l_Enum.Get(&resize_arg)))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		if (!resize_arg.IsValid() || resize_arg.index < LE_GR_RESIZE_NONE || resize_arg.index > LE_GR_RESIZE_MENU_NO_OFFSET)
		{
			Warning(LOCATION, "Invalid resize mode index %d", resize_arg.index);
			return ADE_RETURN_NIL;
		}

		lua_ResizeMode = resize_arg.index - LE_GR_RESIZE_NONE;
	}

	return ade_set_args(L, "o", l_Enum.Set(enum_h(static_cast<lua_enum>(LE_GR_RESIZE_NONE + lua_ResizeMode))));
}

ADE_FUNC(clear, l_Graphics, nullptr, "Calls gr_clear(), which fills the entire screen with the currently active color.  Useful if you want to have a fresh start for drawing things.  (Call this between setClip and resetClip if you only want to clear part of the screen.)", nullptr, nullptr)
{
	gr_clear();

	SCP_UNUSED(L);	// avoid unused parameter warning

	return ADE_RETURN_NIL;
}

ADE_FUNC(clearScreen, l_Graphics, "[number red, number green, number blue, number alpha]",
         "Clears the screen to black, or the color specified.", nullptr, nullptr)
{
	int r,g,b,a;
	r=g=b=0;
	a=255;
	ade_get_args(L, "|iiii", &r, &g, &b, &a);

	//WMC - Set to valid values
	if(r != 0 || g != 0 || b != 0 || a!= 255)
	{
		CAP(r,0,255);
		CAP(g,0,255);
		CAP(b,0,255);
		CAP(a,0,255);
		gr_set_clear_color(r,g,b);
		gr_screen.current_clear_color.alpha = (ubyte)a;
		gr_clear();
		gr_set_clear_color(0,0,0);

		return ADE_RETURN_NIL;
	}

	gr_clear();
	return ADE_RETURN_NIL;
}

ADE_FUNC(createCamera, l_Graphics,
		 "string Name, [vector Position, orientation Orientation]",
		 "Creates a new camera using the specified position and orientation (World)",
		 "camera",
		 "Camera handle, or invalid camera handle if camera couldn't be created")
{
	const char* s = nullptr;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "s|oo", &s, l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh)))
		return ADE_RETURN_NIL;

	matrix *mtx = &vmd_identity_matrix;
	if(mh != NULL)
		mtx = mh->GetMatrix();
	camid cid = cam_create(s, v, mtx);

	//Set position
	return ade_set_args(L, "o", l_Camera.Set(cid));
}

ADE_FUNC(isMenuStretched, l_Graphics, NULL, "Returns whether the standard interface is stretched", "boolean", "True if stretched, false if aspect ratio is maintained")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", Cmdline_stretch_menu != 0);
}

ADE_FUNC(getScreenWidth, l_Graphics, NULL, "Gets screen width", "number", "Width in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.max_w);
}

ADE_FUNC(getScreenHeight, l_Graphics, NULL, "Gets screen height", "number", "Height in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.max_h);
}

ADE_FUNC(getCenterWidth, l_Graphics, NULL, "Gets width of center monitor (should be used in conjuction with getCenterOffsetX)", "number", "Width of center monitor in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_w);
}

ADE_FUNC(getCenterHeight, l_Graphics, NULL, "Gets height of center monitor (should be used in conjuction with getCenterOffsetY)", "number", "Height of center monitor in pixels, or 0 if graphics are not initialized yet")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_h);
}

ADE_FUNC(getCenterOffsetX, l_Graphics, NULL, "Gets X offset of center monitor", "number", "X offset of center monitor in pixels")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_offset_x);
}

ADE_FUNC(getCenterOffsetY, l_Graphics, NULL, "Gets Y offset of center monitor", "number", "Y offset of center monitor in pixels")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", gr_screen.center_offset_y);
}

ADE_FUNC(getCurrentCamera, l_Graphics, "[boolean]", "Gets the current camera handle, if argument is <i>true</i> then it will also return the main camera when no custom camera is in use", "camera", "camera handle or invalid handle on error")
{
	camid current;

	bool rtnMain = false;

	ade_get_args(L, "|b", &rtnMain);

	if (!rtnMain || Viewer_mode & VM_FREECAMERA)
		current = cam_get_current();
	else
		current = Main_camera;

	return ade_set_args(L, "o", l_Camera.Set(current));
}

ADE_FUNC(getVectorFromCoords, l_Graphics,
		 "[number X=center, number Y=center, number Depth, boolean normalize = false]",
		 "Returns a vector through screen coordinates x and y. "
			 "If depth is specified, vector is extended to Depth units into space"
			 "If normalize is true, vector will be normalized.",
		 "vector",
		 "Vector, or zero vector on failure")
{
	int x = gr_screen.max_w/2;
	int y = gr_screen.max_h/2;
	float depth = 0.0f;
	bool normalize = false;
	ade_get_args(L, "|iifb", &x, &y, &depth, &normalize);

	vec3d pos = vmd_zero_vector;

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame) {
		g3_start_frame(0);

		vec3d cam_pos;
		matrix cam_orient;

		camid cid;
		if (Viewer_mode & VM_FREECAMERA)
			cid = cam_get_current();
		else
			cid = Main_camera;

		camera *cam = cid.getCamera();

		if (cam != NULL) {
			cam->get_info(&cam_pos, &cam_orient);
			g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
		} else {
			g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
		}

		gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	g3_point_to_vec(&pos, x, y);

	if(!in_frame) {
		gr_end_view_matrix();
		gr_end_proj_matrix();
		g3_end_frame();
	}

	if(depth)
		vm_vec_scale(&pos, depth);

	if (normalize)
		vm_vec_normalize_quick(&pos);

	vm_vec_add2(&pos, &View_position);

	return ade_set_args(L, "o", l_Vector.Set(pos));
}

ADE_FUNC(setTarget, l_Graphics, "[texture Texture]",
		 "If texture is specified, sets current rendering surface to a texture."
			 "Otherwise, sets rendering surface back to screen.",
		 "boolean",
		 "True if successful, false otherwise")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	texture_h* idx = nullptr;
	ade_get_args(L, "|o", l_Texture.GetPtr(&idx));

	if (idx == nullptr) {
		return ade_set_args(L, "b", bm_set_render_target(-1, 0));
	}
	else {
		return ade_set_args(L, "b", bm_set_render_target(idx->isValid() ? idx->handle : -1, 0));
	}
}

ADE_FUNC(setCamera, l_Graphics, "[camera Camera]", "Sets current camera, or resets camera if none specified", "boolean", "true if successful, false or nil otherwise")
{
	camid cid;
	if(!ade_get_args(L, "|o", l_Camera.Get(&cid)))
	{
		cam_reset_camera();
		return ADE_RETURN_NIL;
	}

	if(!cid.isValid())
		return ADE_RETURN_NIL;

	cam_set_camera(cid);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setColor, l_Graphics, "number Red, number Green, number Blue, [number Alpha]",
         "Sets 2D drawing color; each color number should be from 0 (darkest) to 255 (brightest)", nullptr, nullptr)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int r,g,b,a=255;

	if(!ade_get_args(L, "iii|i", &r, &g, &b, &a))
		return ADE_RETURN_NIL;

	color ac;
	gr_init_alphacolor(&ac,r,g,b,a);
	gr_set_color_fast(&ac);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getColor, l_Graphics, nullptr, "Gets the active 2D drawing color", "number, number, number, number" , "rgba color which is currently in use for 2D drawing")
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	color cur = gr_screen.current_color;
	return ade_set_args(L, "iiii", (int)cur.red, (int)cur.green, (int)cur.blue, (int)cur.alpha);
}

ADE_FUNC(setLineWidth, l_Graphics, "[number width=1.0]", "Sets the line width for lines. This call might fail if the specified width is not supported by the graphics implementation. Then the width will be the nearest supported value.", "boolean", "true if succeeded, false otherwise")
{
	if(!Gr_inited)
		return ADE_RETURN_FALSE;

	float width = 1.0f;

	ade_get_args(L, "|f", &width);

	if (width <= 0.0f)
	{
		return ADE_RETURN_FALSE;
	}

	gr_set_line_width(width);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawCircle, l_Graphics, "number Radius, number X, number Y, [boolean Filled=true]", "Draws a circle", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y,ra;
	bool fill = true;

	if(!ade_get_args(L, "iii|b", &ra,&x,&y,&fill))
		return ADE_RETURN_NIL;

	if (fill) {
		//WMC - Circle takes...diameter.
		gr_circle(x,y, ra*2, lua_ResizeMode);
	} else {
		gr_unfilled_circle(x,y, ra*2, lua_ResizeMode);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawArc, l_Graphics, "number Radius, number X, number Y, number StartAngle, number EndAngle, [boolean Filled=true]", "Draws an arc", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y;
	float ra,angle_start,angle_end;
	bool fill = true;

	if(!ade_get_args(L, "fiiff|b", &ra,&x,&y,&angle_start,&angle_end,&fill)) {
		return ADE_RETURN_NIL;
	}

	gr_arc(x,y, ra, angle_start, angle_end, fill, lua_ResizeMode);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawCurve, l_Graphics, "number X, number Y, number Radius", "Draws a curve", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y,ra,dir = 0;

	if(!ade_get_args(L, "iii|i", &x,&y,&ra, &dir))
		return ADE_RETURN_NIL;

	//WMC - direction should be settable at a certain point via enumerations.
	//Not gonna deal with it now.
	gr_curve(x, y, ra, dir, lua_ResizeMode);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawGradientLine, l_Graphics, "number X1, number Y1, number X2, number Y2", "Draws a line from (x1,y1) to (x2,y2) with the CurrentColor that steadily fades out", NULL, NULL)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;

	if(!ade_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return ADE_RETURN_NIL;

	gr_gradient(x1,y1,x2,y2,lua_ResizeMode);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawLine, l_Graphics, "number X1, number Y1, number X2, number Y2", "Draws a line from (x1,y1) to (x2,y2) with CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x1,y1,x2,y2;

	if(!ade_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return ADE_RETURN_NIL;

	gr_line(x1,y1,x2,y2,lua_ResizeMode);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawPixel, l_Graphics, "number X, number Y", "Sets pixel to CurrentColor", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y;

	if(!ade_get_args(L, "ii", &x, &y))
		return ADE_RETURN_NIL;

	gr_pixel(x,y,lua_ResizeMode);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawPolygon,
	l_Graphics,
	"texture Texture, [vector Position /* Default is null vector */, orientation Orientation=nil, number Width=1.0, "
	"number Height=1.0]",
	"Draws a polygon. May not work properly in hooks other than On Object Render.",
	nullptr,
	nullptr)
{
	texture_h* tdx = nullptr;
	vec3d pos = vmd_zero_vector;
	matrix_h *mh = NULL;
	float width = 1.0f;
	float height = 1.0f;
	if (!ade_get_args(L, "o|ooff", l_Texture.GetPtr(&tdx), l_Vector.Get(&pos), l_Matrix.GetPtr(&mh), &width, &height))
		return ADE_RETURN_NIL;

	if (!tdx->isValid())
		return ADE_RETURN_FALSE;

	matrix *orip = &vmd_identity_matrix;
	if(mh != NULL)
		orip = mh->GetMatrix();

	//Do 3D stuff
	bool in_frame = g3_in_frame() > 0;
	if(!in_frame)
		g3_start_frame(0);

	//gr_set_bitmap(tdx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL, lua_Opacity);
	//g3_draw_polygon(&pos, orip, width, height, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
	material mat_params;
	material_set_unlit(&mat_params, tdx->handle, lua_Opacity, lua_Opacity_type == GR_ALPHABLEND_FILTER ? true : false,
	                   false);
	g3_render_rect_oriented(&mat_params, &pos, orip, width, height);

	if(!in_frame)
		g3_end_frame();

	return ADE_RETURN_TRUE;
}

void drawRectInternal(int x1, int x2, int y1, int y2, bool f = true, float a = 0.f) 
{
	if(f)
	{
		gr_set_bitmap(0);  // gr_rect will use the last bitmaps info, so set to zero to flush any previous alpha state
		gr_rect(x1, y1, x2-x1, y2-y1, lua_ResizeMode, a);
	}
	else
	{
		if (a != 0) {
			float centerX = (x1 + x2) / 2.0f;
			float centerY = (y1 + y2) / 2.0f;

			//We need to calculate each point individually due to the rotation, as they won't always align horizontally and vertically. 
			
			float AX = cosf(a) * (x1 - centerX) - sinf(a) * (y1 - centerY) + centerX;
			float AY = sinf(a) * (x1 - centerX) + cosf(a) * (y1 - centerY) + centerY;
			
			float BX = cosf(a) * (x2 - centerX) - sinf(a) * (y1 - centerY) + centerX;
			float BY = sinf(a) * (x2 - centerX) + cosf(a) * (y1 - centerY) + centerY;

			float CX = cosf(a) * (x2 - centerX) - sinf(a) * (y2 - centerY) + centerX;
			float CY = sinf(a) * (x2 - centerX) + cosf(a) * (y2 - centerY) + centerY;
			
			float DX = cosf(a) * (x1 - centerX) - sinf(a) * (y2 - centerY) + centerX;
			float DY = sinf(a) * (x1 - centerX) + cosf(a) * (y2 - centerY) + centerY;


			gr_line(fl2i(AX), fl2i(AY), fl2i(BX), fl2i(BY), lua_ResizeMode);
			gr_line(fl2i(BX), fl2i(BY), fl2i(CX), fl2i(CY), lua_ResizeMode);
			gr_line(fl2i(CX), fl2i(CY), fl2i(DX), fl2i(DY), lua_ResizeMode);
			gr_line(fl2i(DX), fl2i(DY), fl2i(AX), fl2i(AY), lua_ResizeMode);
		}
		else {

			gr_line(x1, y1, x2, y1, lua_ResizeMode);	//Top
			gr_line(x1, y2, x2, y2, lua_ResizeMode);	//Bottom
			gr_line(x1, y1, x1, y2, lua_ResizeMode);	//Left
			gr_line(x2, y1, x2, y2, lua_ResizeMode);	//Right
		}
	}

}

ADE_FUNC(drawRectangle, l_Graphics, "number X1, number Y1, number X2, number Y2, [boolean Filled=true, number angle=0.0]", "Draws a rectangle with CurrentColor. May be rotated by passing the angle parameter in radians.", nullptr, nullptr)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x1,y1,x2,y2;
	bool f=true;
	float a = 0;

	if(!ade_get_args(L, "iiii|bf", &x1, &y1, &x2, &y2, &f, &a))
		return ADE_RETURN_NIL;
	
	drawRectInternal(x1, x2, y1, y2, f, a);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawRectangleCentered, l_Graphics, 
	"number X, number Y, number Width, number Height, [boolean Filled=true, number angle=0.0]", 
	"Draws a rectangle centered at X,Y with CurrentColor. May be rotated by passing the angle parameter in radians.", nullptr, nullptr) 
{
		if(!Gr_inited)
		return ADE_RETURN_NIL;

	int x,y,w,h;
	bool f=true;
	float a = 0;

	if(!ade_get_args(L, "iiii|bf", &x, &y, &w, &h, &f, &a))
		return ADE_RETURN_NIL;
	
	int x1 = x - (w / 2);
	int x2 = x + (w / 2);
	int y1 = y - (h / 2);
	int y2 = y + (h / 2);

	drawRectInternal(x1, x2, y1, y2, f, a);

	return ADE_RETURN_NIL;
}

ADE_FUNC(drawSphere, l_Graphics, "[number Radius = 1.0, vector Position]", "Draws a sphere with radius Radius at world vector Position. May not work properly in hooks other than On Object Render.", "boolean", "True if successful, false or nil otherwise")
{
	float rad = 1.0f;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|fo", &rad, l_Vector.Get(&pos));

	bool in_frame = g3_in_frame() > 0;
	if(!in_frame) {
		g3_start_frame(0);

		vec3d cam_pos;
		matrix cam_orient;

		camid cid;

		if (Viewer_mode & VM_FREECAMERA)
			cid = cam_get_current();
		else
			cid = Main_camera;

		camera *cam = cid.getCamera();

		if (cam != NULL) {
			cam->get_info(&cam_pos, &cam_orient);
			g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
		} else {
			g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
		}

		gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	vertex vtx;
	vtx.world = pos;
	g3_rotate_vertex(&vtx, &pos);
	g3_draw_sphere(&vtx, rad);

	if(!in_frame) {
		gr_end_view_matrix();
		gr_end_proj_matrix();
		g3_end_frame();
	}
	return ADE_RETURN_TRUE;
}

ADE_FUNC(draw3dLine, l_Graphics, "vector origin, vector destination, [boolean translucent=true, number thickness=1.0, number thicknessEnd=thickness]", "Draws a line from origin to destination. "
"The line may be translucent or solid. Translucent lines will NOT use the alpha value, instead being more transparent the darker the color is. "
"The thickness at the start can be different from the thickness at the end, to draw a line that tapers or expands.", nullptr, nullptr)
{
	vec3d *v1 = &vmd_zero_vector;
	vec3d *v2 = &vmd_zero_vector;
	bool translucent = true;
	float thickness = 1.0f;
	float thicknessEnd = -1.0f;

	if (!ade_get_args(L, "oo|bff", l_Vector.GetPtr(&v1), l_Vector.GetPtr(&v2), &translucent, &thickness, &thicknessEnd))
		return ADE_RETURN_NIL;

	if (thickness < 0) thickness = 0;
	if (thicknessEnd < 0) thicknessEnd = thickness;
	
	if (thickness <= 0 && thicknessEnd <= 0)
	{
		if (!WarnedBadThicknessLine)
		{
			LuaError(L, "At least one end of a 3D line must be greater than zero! The line won't be drawn!\nThis warning won't be shown again this play session.");
			WarnedBadThicknessLine = true;
		}

		//While the batching_add_line function would also do this check, I feel that its good to return early here.
		return ADE_RETURN_NIL;
	}

	color &clr = gr_screen.current_color;

	batching_add_line(v1, v2, thickness, thicknessEnd, clr, translucent);

	return ADE_RETURN_NIL;
}

// Aardwolf's test code to render a model, supposed to emulate WMC's gr.drawModel function
ADE_FUNC(drawModel, l_Graphics, "model model, vector position, orientation orientation",
         "Draws the given model with the specified position and orientation.  "
	     "Note: this method does NOT use CurrentResizeMode.",
         "number", "Zero if successful, otherwise an integer error code")
{
	model_h *mdl = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "ooo", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh)))
		return ade_set_args(L, "i", 1);

	if(mdl == NULL)
		return ade_set_args(L, "i", 2);

	int model_num = mdl->GetID();
	if(model_num < 0)
		return ade_set_args(L, "i", 3);

	// Make sure we have a scene to use
	// Note that this is only relevant, and thus only expected to be set, in OBJECTRENDER hooks
	if (scripting::hooks::OnObjectRender->isActive() && !Current_scene)
		return ade_set_args(L, "i", 4);

	//Handle angles
	matrix *orient = mh->GetMatrix();

	//Clip
	gr_set_clip(0, 0, gr_screen.max_w, gr_screen.max_h, GR_RESIZE_NONE);	// don't use lua_ResizeMode here since this function handles its own resizing

	//Handle 3D init stuff
	g3_start_frame(1);

	vec3d cam_pos;
	matrix cam_orient;

	camid cid;
	if (Viewer_mode & VM_FREECAMERA)
		cid = cam_get_current();
	else
		cid = Main_camera;

	camera *cam = cid.getCamera();

	if (cam != NULL) {
		cam->get_info(&cam_pos, &cam_orient);
		g3_set_view_matrix(&cam_pos, &cam_orient, View_zoom);
	} else {
		g3_set_view_matrix(&Eye_position, &Eye_matrix, View_zoom);
	}

	gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	//Draw the ship!!
	model_clear_instance(model_num);
	model_set_detail_level(0);
	model_render_params render_info;

	render_info.set_detail_level_lock(0);

	// If this function executes in OBJECTRENDER, and if the Current_scene is set, we can take advantage
	// of some optimizations by adding this model to the global render queue.
	// In all other circumstances, we need to render the model in immediate mode, which may be slow but
	// is guaranteed to work.
	if (scripting::hooks::OnObjectRender->isActive())
		model_render_queue(&render_info, Current_scene, model_num, orient, v);
	else
		model_render_immediate(&render_info, model_num, orient, v);

	//OK we're done
	gr_end_view_matrix();
	gr_end_proj_matrix();

	//Bye!!
	g3_end_frame();
	gr_reset_clip();

	return ade_set_args(L, "i", 0);
}

// Wanderer
ADE_FUNC(drawModelOOR, l_Graphics, "model Model, vector Position, orientation Orientation, [number Flags]",
         "Draws the given model with the specified position and orientation",
         "number", "Zero if successful, otherwise an integer error code")
{
	model_h *mdl = NULL;
	vec3d *v = &vmd_zero_vector;
	matrix_h *mh = NULL;
	int flags = MR_NORMAL;
	if(!ade_get_args(L, "ooo|i", l_Model.GetPtr(&mdl), l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh), &flags))
		return ade_set_args(L, "i", 1);

	if(mdl == NULL)
		return ade_set_args(L, "i", 2);

	polymodel *pm = mdl->Get();

	if (pm == NULL)
		return ade_set_args(L, "i", 3);

	int model_num = pm->id;

	if(model_num < 0)
		return ade_set_args(L, "i", 3);

	// Make sure we have a scene to use
	// Note that this is only relevant, and thus only expected to be set, in OBJECTRENDER hooks
	if (scripting::hooks::OnObjectRender->isActive() && !Current_scene)
		return ade_set_args(L, "i", 4);

	//Handle angles
	matrix *orient = mh->GetMatrix();

	//Draw the ship!!
	model_clear_instance(model_num);

	model_render_params render_info;
	render_info.set_flags(flags);

	// If this function executes in OBJECTRENDER, and if the Current_scene is set, we can take advantage
	// of some optimizations by adding this model to the global render queue.
	// In all other circumstances, we need to render the model in immediate mode, which may be slow but
	// is guaranteed to work.
	if (scripting::hooks::OnObjectRender->isActive())
		model_render_queue(&render_info, Current_scene, model_num, orient, v);
	else
		model_render_immediate(&render_info, model_num, orient, v);
	return ade_set_args(L, "i", 0);
}

// Aardwolf's targeting brackets function
ADE_FUNC(drawTargetingBrackets, l_Graphics, "object Object, [boolean draw=true, number padding=5]",
         "Gets the edge positions of targeting brackets for the specified object. The brackets will only be drawn if "
         "draw is true or the default value of draw is used. Brackets are drawn with the current color. The brackets "
         "will have a padding (distance from the actual bounding box); the default value (used elsewhere in FS2) is 5.  "
         "Note: this method does NOT use CurrentResizeMode.",
         "number, number, number, number",
         "Left, top, right, and bottom positions of the brackets, or nil if invalid")
{
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}

	object_h *objh = NULL;
	bool draw_box = true;
	int padding = 5;

	if( !ade_get_args(L, "o|bi", l_Object.GetPtr(&objh), &draw_box, &padding) ) {
		return ADE_RETURN_NIL;
	}

	// The following code is mostly copied from
	// void hud_show_brackets(object *targetp, vertex *projected_v)
	// in hudtarget.cpp

	if( !objh->IsValid()) {
		return ADE_RETURN_NIL;
	}

	object *targetp = objh->objp;

	int x1,x2,y1,y2;
	int bound_rc, pof;
	int modelnum;
	bool entered_frame = false;
	SCP_list<CJumpNode>::iterator jnp;

	if ( !(g3_in_frame( ) > 0 ) )
	{
		g3_start_frame( 0 );
		entered_frame = true;
	}


	switch ( targetp->type ) {
		case OBJ_SHIP:
			modelnum = Ship_info[Ships[targetp->instance].ship_info_index].model_num;
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			if ( bound_rc != 0 ) {
				if ( entered_frame )
					g3_end_frame( );
				return ADE_RETURN_NIL;
			}
			break;
		case OBJ_DEBRIS:
			modelnum = Debris[targetp->instance].model_num;
			bound_rc = submodel_find_2d_bound_min( modelnum, Debris[targetp->instance].submodel_num, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			if ( bound_rc != 0 ) {
				if ( entered_frame )
					g3_end_frame( );
				return ADE_RETURN_NIL;
			}
			break;
		case OBJ_WEAPON:
			modelnum = Weapon_info[Weapons[targetp->instance].weapon_info_index].model_num;
			if (modelnum != -1)
				bound_rc = model_find_2d_bound_min(modelnum, &targetp->orient, &targetp->pos, &x1, &y1, &x2, &y2);
			else {
				vertex vtx;
				g3_rotate_vertex(&vtx, &targetp->pos);
				g3_project_vertex(&vtx);
				x1 = x2 = (int)vtx.screen.xyw.x;
				y1 = y2 = (int)vtx.screen.xyw.y;
			}
			break;
		case OBJ_ASTEROID:
			pof = Asteroids[targetp->instance].asteroid_subtype;
			modelnum = Asteroid_info[Asteroids[targetp->instance].asteroid_type].model_num[pof];
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		case OBJ_JUMP_NODE:
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if(jnp->GetSCPObject() == targetp)
					break;
			}

			modelnum = jnp->GetModelNumber();
			bound_rc = model_find_2d_bound_min( modelnum, &targetp->orient, &targetp->pos,&x1,&y1,&x2,&y2 );
			break;
		default: //Someone passed an invalid pointer.
			if ( entered_frame )
				g3_end_frame( );
			return ADE_RETURN_NIL;
	}

	x1 -= padding;
	x2 += padding;
	y1 -= padding;
	y2 += padding;
	if ( draw_box ) {
		graphics::line_draw_list line_draw_list;
		draw_brackets_square(&line_draw_list, x1, y1, x2, y2, GR_RESIZE_NONE);	// don't use lua_ResizeMode here since this function handles its own resizing
		line_draw_list.flush();
	}

	if ( entered_frame )
		g3_end_frame( );

	return ade_set_args(L, "iiii", x1, y1, x2, y2);
}

ADE_FUNC(
    drawSubsystemTargetingBrackets, l_Graphics, "subsystem subsys, [boolean draw=true, boolean setColor=false]",
    "Gets the edge position of the targeting brackets drawn for a subsystem as if they were drawn on the HUD. Only "
    "actually draws the brackets if <i>draw</i> is true, optionally sets the color the as if it was drawn on the HUD",
    "number, number, number, number",
    "Left, top, right, and bottom positions of the brackets, or nil if invalid or off-screen")
{
	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}

	ship_subsys_h *sshp = NULL;
	bool draw = true;
	bool set_color = false;

	if( !ade_get_args(L, "o|bb", l_Subsystem.GetPtr(&sshp), &draw, &set_color) ) {
		return ADE_RETURN_NIL;
	}

	if (!sshp->isSubsystemValid())
	{
		return ADE_RETURN_NIL;
	}

	bool entered_frame = false;

	if ( !(g3_in_frame( ) > 0 ) )
	{
		g3_start_frame( 0 );
		entered_frame = true;
	}

	int coords[4];

	graphics::line_draw_list line_draw_list;
	int in_sight = draw_subsys_brackets(&line_draw_list, sshp->ss, 24, 24, draw, set_color, coords);
	line_draw_list.flush();

	if ( entered_frame )
		g3_end_frame( );

	if (in_sight > 0)
	{
		return ade_set_args(L, "iiii", coords[0], coords[1], coords[2], coords[3]);
	}
	else
	{
		return ADE_RETURN_NIL;
	}
}

ADE_FUNC(drawOffscreenIndicator, l_Graphics, "object Object, [boolean draw=true, boolean setColor=false]",
         "Draws an off-screen indicator for the given object. The indicator will not be drawn if draw=false, but the "
         "coordinates will be returned in either case. The indicator will be drawn using the current color if "
         "setColor=true and using the IFF color of the object if setColor=false.",
         "number, number",
         "Coordinates of the indicator (at the very edge of the screen), or nil if object is on-screen")
{
	object_h *objh = NULL;
	bool draw = false;
	bool setcolor = false;
	vec2d outpoint = { -1.0f, -1.0f };

	if(!Gr_inited) {
		return ADE_RETURN_NIL;
	}

	if( !ade_get_args(L, "o|bb", l_Object.GetPtr(&objh), &draw, &setcolor) ) {
		return ADE_RETURN_NIL;
	}

	if( !objh->IsValid()) {
		return ADE_RETURN_NIL;
	}

	object *targetp = objh->objp;
	bool in_frame = g3_in_frame() > 0;

	if (!in_frame)
		g3_start_frame(0);

	vertex target_point;
	g3_rotate_vertex(&target_point, &targetp->pos);
	g3_project_vertex(&target_point);

	if (!in_frame)
		g3_end_frame();

	if(target_point.codes == 0)
		return ADE_RETURN_NIL;

	hud_target_clear_display_list();
	hud_target_add_display_list(targetp, &target_point, &targetp->pos, 5, NULL, NULL, TARGET_DISPLAY_DIST);

	size_t j, num_gauges;
	num_gauges = default_hud_gauges.size();

	for(j = 0; j < num_gauges; j++) {
		if (default_hud_gauges[j]->getObjectType() == HUD_OBJECT_OFFSCREEN) {
			HudGaugeOffscreen *offscreengauge = static_cast<HudGaugeOffscreen*>(default_hud_gauges[j].get());

			int dir;
			float tri_separation;

			offscreengauge->calculatePosition(&target_point, &targetp->pos, &outpoint, &dir, &tri_separation);

			if (draw) {
				// needs to be turned on so it can pass the canRender() condition
				// but first make sure to record the original status to use to restore later
				bool original_status = offscreengauge->canRender();
				offscreengauge->updateActive(true);

				// only draw if it can be rendered
				if ( offscreengauge->canRender() ) {

					offscreengauge->preprocess();
					offscreengauge->onFrame(flFrametime);

					offscreengauge->resetClip();
					offscreengauge->setFont();

					float distance = hud_find_target_distance(targetp, Player_obj);

					if (!setcolor)
						hud_set_iff_color(targetp, 1);

					offscreengauge->renderOffscreenIndicator(&outpoint, dir, distance, tri_separation, true);
				}

				// now that the gauge is rendered restore the original status
				offscreengauge->updateActive(original_status);
			}

			offscreengauge->resize(&outpoint.x, &outpoint.y);

			break;
		}
	}

	if (outpoint.x >= 0 && outpoint.y >=0)
		return ade_set_args(L, "ii", (int)outpoint.x, (int)outpoint.y);
	else
		return ADE_RETURN_NIL;
}

#define MAX_TEXT_LINES		256
static const char *BooleanValues[] = {"False", "True"};

static int drawString_sub(lua_State *L, bool use_resize_arg)
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	int resize_mode = lua_ResizeMode;

	// if the frame has changed since the last drawString call, reset the string position
	if (PreviousFrametimeOverall != game_get_overall_frametime())
	{
		PreviousFrametimeOverall = game_get_overall_frametime();
		memcpy(NextDrawStringPos, NextDrawStringPosInitial, sizeof(NextDrawStringPos));
	}

	int x = NextDrawStringPos[0];
	int y = NextDrawStringPos[1];

	const char *s = "(null)";
	int x2=-1,y2=-1;
	int num_lines = 0;

	if (use_resize_arg)
	{
		enum_h resize_arg;
		if (!ade_get_args(L, "o", l_Enum.Get(&resize_arg)))
			return ade_set_error(L, "i", 0);

		if (!resize_arg.IsValid() || resize_arg.index < LE_GR_RESIZE_NONE || resize_arg.index > LE_GR_RESIZE_MENU_NO_OFFSET)
		{
			Warning(LOCATION, "Invalid resize mode index %d", resize_arg.index);
			return ade_set_error(L, "i", 0);
		}

		resize_mode = resize_arg.index - LE_GR_RESIZE_NONE;

		// so that ade_get_args below will read the correct positions
		internal::Ade_get_args_skip++;
	}

	if (lua_isboolean(L, 1 + internal::Ade_get_args_skip))
	{
		bool b = false;
		if(!ade_get_args(L, "b|iiii", &b, &x, &y, &x2, &y2))
			return ade_set_error(L, "i", 0);

		if(b)
			s = BooleanValues[1];
		else
			s = BooleanValues[0];
	}
	else if (lua_isstring(L, 1 + internal::Ade_get_args_skip))
	{
		if(!ade_get_args(L, "s|iiii", &s, &x, &y, &x2, &y2))
			return ade_set_error(L, "i", 0);
	}
	else
	{
		ade_get_args(L, "|*iiii", &x, &y, &x2, &y2);
	}

	NextDrawStringPos[0] = x;
	if(x2 < 0)
	{
		num_lines = 1;
		gr_string(x,y,s,resize_mode);

		int height = 0;
		gr_get_string_size(NULL, &height, s);
		NextDrawStringPos[1] = y+height;
	}
	else
	{
		SCP_vector<int> linelengths;
		SCP_vector<const char*> linestarts;

		// This would pass a <=0 value to split_str
		if (x2 <= x)
		{
			static bool Warned_drawString_x_x2 = false;
			if (!Warned_drawString_x_x2)
			{
				Warning(LOCATION, "Illegal values passed to gr.drawString: x2 (%d) must be greater than x (%d).", x2, x);
				Warned_drawString_x_x2 = true;
			}

			std::swap(x, x2);
		}
		// Invalid y2 value
		if (y2 >= 0 && y2 <= y)
		{
			static bool Warned_drawString_y_y2 = false;
			if (!Warned_drawString_y_y2)
			{
				Warning(LOCATION, "Illegal y2 value passed to gr.drawString: y2 (%d) must be greater than y (%d).", y2, y);
				Warned_drawString_y_y2 = true;
			}

			std::swap(y, y2);
		}

		num_lines = split_str(s, x2-x, linelengths, linestarts, INT_MAX, (unicode::codepoint_t)-1, false);

		int line_ht = gr_get_font_height();
		if (y2 < 0)
			y2 = y + line_ht;

		//Make sure we don't go over size
		num_lines = MIN(num_lines, (y2 - y) / line_ht);

		int curr_y = y;
		for(int i = 0; i < num_lines; i++)
		{
			//Contrary to WMC's previous comment, let's make a new string each line
			int len = linelengths[i];
			char *buf = new char[len+1];
			strncpy(buf, linestarts[i], len);
			buf[len] = '\0';

			//Draw the string
			gr_string(x,curr_y,buf,resize_mode);

			//Free the string we made
			delete[] buf;

			//Increment line height
			curr_y += line_ht;
		}

		if (num_lines <= 0)
		{
			// If no line was drawn then we need to add one so the next line is
			// aligned right
			curr_y += line_ht;
		}

		NextDrawStringPos[1] = curr_y;
	}
	return ade_set_error(L, "i", num_lines);
}

ADE_FUNC(drawString, l_Graphics, "string|boolean Message, [number X1, number Y1, number X2, number Y2]",
	"Draws a string. Use x1/y1 to control position, x2/y2 to limit textbox size."
	"Text will automatically move onto new lines, if x2/y2 is specified."
	"Additionally, calling drawString with only a string argument will automatically"
	"draw that string below the previously drawn string (or 0,0 if no strings"
	"have been drawn yet",
	"number",
	"Number of lines drawn, or 0 on failure")
{
	return drawString_sub(L, false);
}

ADE_FUNC(drawStringResized, l_Graphics, "enumeration ResizeMode, string|boolean Message, [number X1, number Y1, number X2, number Y2]",
	"Draws a string, scaled according to the GR_RESIZE_* parameter. Use x1/y1 to control position, x2/y2 to limit textbox size."
	"Text will automatically move onto new lines, if x2/y2 is specified, however the line spacing will probably not be correct."
	"Additionally, calling drawString with only a string argument will automatically"
	"draw that string below the previously drawn string (or 0,0 if no strings"
	"have been drawn yet",
	"number",
	"Number of lines drawn, or 0 on failure")
{
	return drawString_sub(L, true);
}

ADE_FUNC(setScreenScale, l_Graphics, "number width, number height, [number zoom_x, number zoom_y, number max_x, number max_y, number center_x, number center_y, boolean force_stretch]",
	"Calls gr_set_screen_scale with the specified parameters.  This is useful for adjusting the drawing of graphics or text to be the same apparent size regardless of resolution.",
	nullptr, nullptr)
{
	if (!Gr_inited)
		return ADE_RETURN_NIL;

	int x, y;
	int zoom_x = -1, zoom_y = -1;
	int max_x = gr_screen.max_w, max_y = gr_screen.max_h;
	int center_x = gr_screen.center_w, center_y = gr_screen.center_h;
	bool force_stretch = false;

	if (!ade_get_args(L, "ii|iiiiiib", &x, &y, &zoom_x, &zoom_y, &max_x, &max_y, &center_x, &center_y, &force_stretch))
		return ADE_RETURN_NIL;

	gr_set_screen_scale(x, y, zoom_x, zoom_y, max_x, max_y, center_x, center_y, force_stretch);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetScreenScale, l_Graphics, nullptr, "Rolls back the most recent call to setScreenScale.", nullptr, nullptr)
{
	if (!Gr_inited)
		return ADE_RETURN_NIL;

	gr_reset_screen_scale();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getStringWidth, l_Graphics, "string String", "Gets string width", "number", "String width, or 0 on failure")
{
	if(!Gr_inited)
		return ade_set_error(L, "i", 0);

	const char* s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int w;

	gr_get_string_size(&w, NULL, s);

	return ade_set_args(L, "i", w);
}

ADE_FUNC(loadStreamingAnim, l_Graphics, "string Filename, [boolean loop, boolean reverse, boolean pause, boolean cache, boolean grayscale]",
		 "Plays a streaming animation, returning its handle. The optional booleans (except cache and grayscale) can also be set via the handle's virtvars<br>"
			 "cache is best set to false when loading animations that are only intended to play once, e.g. headz<br>"
			 "Remember to call the unload() function when you're finished using the animation to free up memory.",
		 "streaminganim",
		 "Streaming animation handle, or invalid handle if animation couldn't be loaded")
{
	const char* s;
	int rc = -1;
	bool loop = false, reverse = false, pause = false, cache = true, grayscale = false;

	if(!ade_get_args(L, "s|bbbbb", &s, &loop, &reverse, &pause, &cache, &grayscale))
		return ADE_RETURN_NIL;

	streaminganim_h sah(s);
	if (!loop) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_NOLOOP;
	}
	if (reverse) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_BACKWARDS;
	}
	if (pause) {
		sah.ga.direction |= GENERIC_ANIM_DIRECTION_PAUSED;
	}
	if (grayscale) {
		sah.ga.use_hud_color = true;
	}
	rc = generic_anim_stream(&sah.ga, cache);

	if(rc < 0)
		return ade_set_args(
		    L, "o",
		    l_streaminganim.Set(std::move(sah))); // this object should be "invalid", matches loadTexture behaviour

	return ade_set_args(L, "o", l_streaminganim.Set(std::move(sah)));
}

ADE_FUNC(createTexture, l_Graphics, "[number Width=512, number Height=512, enumeration Type=TEXTURE_DYNAMIC]",
		 "Creates a texture for rendering to."
			 "Types are TEXTURE_STATIC - for infrequent rendering - and TEXTURE_DYNAMIC - for frequent rendering.",
		 "texture",
		 "New texture handle, or invalid texture handle if texture couldn't be created")
{
	int w=512;
	int h=512;
	enum_h *e = NULL;

	//GET ARGS
	ade_get_args(L, "|iio", &w, &h, l_Enum.GetPtr(&e));

	int t = BMP_FLAG_RENDER_TARGET_DYNAMIC;
	if(e != NULL)
	{
		if(e->index == LE_TEXTURE_STATIC)
			t = BMP_FLAG_RENDER_TARGET_STATIC;
		else if(e->index == LE_TEXTURE_DYNAMIC)
			t = BMP_FLAG_RENDER_TARGET_DYNAMIC;
	}

	int idx = bm_make_render_target(w, h, t);

	if(idx < 0)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	//Since creating a render target does not increase load count, when creating the scripting texture object we need to pass true to the constructor so that the constructor will increase the load count for us.
	return ade_set_args(L, "o", l_Texture.Set(texture_h(idx, true)));
}

ADE_FUNC(loadTexture, l_Graphics, "string Filename, [boolean LoadIfAnimation, boolean NoDropFrames]",
		 "Gets a handle to a texture. If second argument is set to true, animations will also be loaded."
			 "If third argument is set to true, every other animation frame will not be loaded if system has less than 48 MB memory."
			 "<br><strong>IMPORTANT:</strong> Textures will not be unload themselves unless you explicitly tell them to do so."
			 "When you are done with a texture, call the unload() function to free up memory.",
		 "texture",
		 "Texture handle, or invalid texture handle if texture couldn't be loaded")
{
	const char* s;
	int idx=-1;
	bool b=false;
	bool d=false;

	if(!ade_get_args(L, "s|bb", &s, &b, &d))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if(b) {
		idx = bm_load_animation(s, nullptr, nullptr, nullptr, nullptr, d);
	}
	if(idx < 0) {
		idx = bm_load(s);
	}

	if(idx < 0)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));
	
	//Loading increases load count, so we must pass false to the texture scripting object, so that the constructor doesn't increase the load count by itself again.
	return ade_set_args(L, "o", l_Texture.Set(texture_h(idx, false)));
}

ADE_FUNC(drawImage,
	l_Graphics,
	"string|texture fileNameOrTexture, [number X1=0, number Y1=0, number X2, number Y2, number UVX1 = 0.0, number UVY1 "
	"= 0.0, number UVX2=1.0, number UVY2=1.0, number alpha=1.0, boolean aaImage = false, number angle = 0.0]",
	"Draws an image file or texture. Any image extension passed will be ignored."
	"The UV variables specify the UV value for each corner of the image. "
	"In UV coordinates, (0,0) is the top left of the image; (1,1) is the lower right. If aaImage is true, image needs "
	"to be monochrome and will be drawn tinted with the currently active color."
	"The angle variable can be used to rotate the image in radians.",
	"boolean",
	"Whether image was drawn")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx;
	int x1 = 0;
	int y1 = 0;
	int x2=INT_MAX;
	int y2=INT_MAX;
	float uv_x1=0.0f;
	float uv_y1=0.0f;
	float uv_x2=1.0f;
	float uv_y2=1.0f;
	float alpha=1.0f;
	bool aabitmap = false;
	float angle = 0.f;

	if(lua_isstring(L, 1))
	{
		const char* s = nullptr;
		if (!ade_get_args(L, "s|iiiifffffbf", &s, &x1, &y1, &x2, &y2, &uv_x1, &uv_y1, &uv_x2, &uv_y2, &alpha, &aabitmap, &angle))
			return ade_set_error(L, "b", false);

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return ADE_RETURN_FALSE;
	}
	else
	{
		texture_h* th = nullptr;
		if (!ade_get_args(L,
				"o|iiiifffffbf",
				l_Texture.GetPtr(&th),
				&x1,
				&y1,
				&x2,
				&y2,
				&uv_x1,
				&uv_y1,
				&uv_x2,
				&uv_y2,
				&alpha,
				&aabitmap,
				&angle))
			return ade_set_error(L, "b", false);

		if (!th->isValid()) {
			return ade_set_error(L, "b", false);
		}

		idx = th->handle;
	}

	if(!bm_is_valid(idx))
		return ade_set_error(L, "b", false);

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return ADE_RETURN_FALSE;

	if(x2!=INT_MAX)
		w = x2-x1;

	if(y2!=INT_MAX)
		h = y2-y1;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL, alpha);

	bitmap_rect_list brl = bitmap_rect_list(x1, y1, w, h, uv_x1, uv_y1, uv_x2, uv_y2);

	if (aabitmap) {
		gr_aabitmap_list(&brl, 1, lua_ResizeMode, angle);
	} else {
		gr_bitmap_list(&brl, 1, lua_ResizeMode, angle);
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(drawImageCentered,
	l_Graphics,
	"string|texture fileNameOrTexture, [number X=0, number Y=0, number W, number H, number UVX1 = 0.0, number UVY1 "
	"= 0.0, number UVX2=1.0, number UVY2=1.0, number alpha=1.0, boolean aaImage = false, number angle = 0.0]",
	"Draws an image file or texture centered on the X,Y position. Any image extension passed will be ignored."
	"The UV variables specify the UV value for each corner of the image. "
	"In UV coordinates, (0,0) is the top left of the image; (1,1) is the lower right. If aaImage is true, image needs "
	"to be monochrome and will be drawn tinted with the currently active color."
	"The angle variable can be used to rotate the image in radians.",
	"boolean",
	"Whether image was drawn") {
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx;
	int x = 0;
	int y = 0;
	int w=INT_MAX;
	int h=INT_MAX;
	float uv_x1=0.0f;
	float uv_y1=0.0f;
	float uv_x2=1.0f;
	float uv_y2=1.0f;
	float alpha=1.0f;
	bool aabitmap = false;
	float angle = 0.f;

	if(lua_isstring(L, 1))
	{
		const char* s = nullptr;
		if (!ade_get_args(L, "s|iiiifffffbf", &s, &x, &y, &w, &h, &uv_x1, &uv_y1, &uv_x2, &uv_y2, &alpha, &aabitmap, &angle))
			return ade_set_error(L, "b", false);

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return ADE_RETURN_FALSE;
	}
	else
	{
		texture_h* th = nullptr;
		if (!ade_get_args(L,
				"o|iiiifffffbf",
				l_Texture.GetPtr(&th),
				&x,
				&y,
				&w,
				&h,
				&uv_x1,
				&uv_y1,
				&uv_x2,
				&uv_y2,
				&alpha,
				&aabitmap,
				&angle))
			return ade_set_error(L, "b", false);

		if (!th->isValid()) {
			return ade_set_error(L, "b", false);
		}

		idx = th->handle;
	}

	if(!bm_is_valid(idx))
		return ade_set_error(L, "b", false);

	int original_w, original_h;
	if(bm_get_info(idx, &original_w, &original_h) < 0)
		return ADE_RETURN_FALSE;

	if(w==INT_MAX)
		w = original_w;

	if(h==INT_MAX)
		h = original_h;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL, alpha);

	bitmap_rect_list brl = bitmap_rect_list(x-(w/2), y-(h/2), w, h, uv_x1, uv_y1, uv_x2, uv_y2);

	if (aabitmap) {
		gr_aabitmap_list(&brl, 1, lua_ResizeMode, angle);
	} else {
		gr_bitmap_list(&brl, 1, lua_ResizeMode, angle);
	}

	return ADE_RETURN_TRUE;

}



ADE_FUNC_DEPRECATED(drawMonochromeImage,
	l_Graphics,
	"string|texture fileNameOrTexture, number X1, number Y1, [number X2, number Y2, number alpha=1.0]",
	"Draws a monochrome image from a texture or file using the current color",
	"boolean",
	"Whether image was drawn",
	gameversion::version(21, 0),
	"gr.drawImage now has support for drawing monochrome images with full UV scaling support")
{
	if(!Gr_inited)
		return ade_set_error(L, "b", false);

	int idx;
	int x,y;
	int x2=INT_MAX;
	int y2=INT_MAX;
	int sx=0;
	int sy=0;
	bool m = false;
	float alpha=1.0;

	if(lua_isstring(L, 1))
	{
		const char* s = nullptr;
		if(!ade_get_args(L, "sii|iif", &s,&x,&y,&x2,&y2,&alpha))
			return ade_set_error(L, "b", false);

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return ADE_RETURN_FALSE;
	}
	else
	{
		texture_h* th = nullptr;
		if(!ade_get_args(L, "oii|iif", l_Texture.GetPtr(&th),&x,&y,&x2,&y2,&alpha))
			return ade_set_error(L, "b", false);

		if (!th->isValid()) {
			return ade_set_error(L, "b", false);
		}

		idx = th->handle;
	}

	if(!bm_is_valid(idx))
		return ade_set_error(L, "b", false);

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return ADE_RETURN_FALSE;

	if(sx < 0)
		sx = w + sx;

	if(sy < 0)
		sy = h + sy;

	if(x2!=INT_MAX)
		w = x2-x;

	if(y2!=INT_MAX)
		h = y2-y;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL,alpha);
	gr_aabitmap_ex(x, y, w, h, sx, sy, lua_ResizeMode, m);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getImageWidth, l_Graphics, "string Filename", "Gets image width", "number", "Image width, or 0 if filename is invalid")
{
	const char* s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int w;

	int idx = bm_load(s);

	if(idx < 0)
		return ade_set_error(L, "i", 0);

	bm_get_info(idx, &w);
	return ade_set_args(L, "i", w);
}

ADE_FUNC(getImageHeight, l_Graphics, "string name", "Gets image height", "number", "Image height, or 0 if filename is invalid")
{
	const char* s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "i", 0);

	int h;

	int idx = bm_load(s);

	if(idx < 0)
		return ade_set_error(L, "i", 0);

	bm_get_info(idx, NULL, &h);
	return ade_set_args(L, "i", h);
}

ADE_FUNC(flashScreen, l_Graphics, "number Red, number Green, number Blue", "Flashes the screen", NULL, NULL)
{
	if(!Gr_inited)
		return ADE_RETURN_NIL;

	int r,g,b;

	if(!ade_get_args(L, "iii", &r, &g, &b))
		return ADE_RETURN_NIL;

	gr_flash(r,g,b);

	return ADE_RETURN_NIL;
}

ADE_FUNC(loadModel, l_Graphics, "string Filename", "Loads the model - will not setup subsystem data, DO NOT USE FOR LOADING SHIP MODELS", "model", "Handle to a model")
{
	const char* s;
	int model_num = -1;

	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	if (s[0] == '\0')
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	model_num = model_load(s, 0, NULL);

	return ade_set_args(L, "o", l_Model.Set(model_h(model_num)));
}

ADE_FUNC(hasViewmode, l_Graphics, "enumeration", "Specifies if the current viemode has the specified flag, see VM_* enumeration", "boolean", "true if flag is present, false otherwise")
{
	enum_h *type = NULL;

	if (!ade_get_args(L, "o", l_Enum.GetPtr(&type)))
		return ade_set_error(L, "b", false);

	if (type == NULL || !type->IsValid())
		return ade_set_error(L, "b", false);

	int bit = 0;
	static bool VM_EXTERNAL_CAMERA_LOCKED_WARNED = false;

	switch(type->index)
	{
		case LE_VM_INTERNAL:
			return ade_set_args(L, "b", (Viewer_mode & ~(VM_CAMERA_LOCKED | VM_CENTERING)) == 0);	// z64: Ignore camera lock state and centering state
			break;

		case LE_VM_EXTERNAL:
			bit = VM_EXTERNAL;
			break;

		case LE_VM_OTHER_SHIP:
			bit = VM_OTHER_SHIP;
			break;

		case LE_VM_CHASE:
			bit = VM_CHASE;
			break;

		case LE_VM_DEAD_VIEW:
			bit = VM_DEAD_VIEW;
			break;

		case LE_VM_EXTERNAL_CAMERA_LOCKED:
		    if (!VM_EXTERNAL_CAMERA_LOCKED_WARNED) {
			    Warning(LOCATION, "The enumeration VM_EXTERNAL_CAMERA_LOCKED has been deprecated for lua function "
			                      "hasViewmode()! To ensure future compatibility, please check for either "
			                      "VM_CAMERA_LOCKED, VM_EXTERNAL, or both, instead.");		    
				VM_EXTERNAL_CAMERA_LOCKED_WARNED = true;
			}

			return ade_set_args(L, "b", ((Viewer_mode & VM_CAMERA_LOCKED) && (Viewer_mode & VM_EXTERNAL)) != 0);
			break;

		case LE_VM_CAMERA_LOCKED:
			bit = VM_CAMERA_LOCKED;
			break;

		case LE_VM_FREECAMERA:
			bit = VM_FREECAMERA;
			break;

		case LE_VM_PADLOCK_LEFT:
			bit = VM_PADLOCK_LEFT;
			break;

		case LE_VM_PADLOCK_REAR:
			bit = VM_PADLOCK_REAR;
			break;

		case LE_VM_PADLOCK_RIGHT:
			bit = VM_PADLOCK_RIGHT;
			break;

		case LE_VM_PADLOCK_UP:
			bit = VM_PADLOCK_UP;
			break;

		case LE_VM_TOPDOWN:
			bit = VM_TOPDOWN;
			break;

		case LE_VM_TRACK:
			bit = VM_TRACK;
			break;

		case LE_VM_WARP_CHASE:
			bit = VM_WARP_CHASE;
			break;

		case LE_VM_WARPIN_ANCHOR:
			bit = VM_WARPIN_ANCHOR;
			break;

		case LE_VM_CENTERING:
			bit = VM_CENTERING;
			break;

		default:
			LuaError(L, "Attempted to use hasViewmode with an invalid enumeration! Only VM_* enumerations are allowed!");
			return ade_set_error(L, "b", false);
			break;
	}

	return ade_set_args(L, "b", (Viewer_mode & bit) != 0);
}

ADE_FUNC(setClip, l_Graphics, "number x, number y, number width, number height", "Sets the clipping region to the specified rectangle. Most drawing functions are able to handle the offset.", "boolean", "true if successful, false otherwise")
{
	int x, y, width, height;

	if (!ade_get_args(L, "iiii", &x, &y, &width, &height))
		return ADE_RETURN_FALSE;

	gr_set_clip(x, y, width, height, lua_ResizeMode);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(resetClip, l_Graphics, NULL, "Resets the clipping region that might have been set", "boolean", "true if successful, false otherwise")
{
	gr_reset_clip();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(openMovie, l_Graphics, "string name, boolean looping = false",
         "Opens the movie with the specified name. If the name has an extension it will be removed. This function will "
         "try all movie formats supported by the engine and use the first that is found.",
         "movie_player", "The cutscene player handle or invalid handle if cutscene could not be opened.")
{
	const char* name = nullptr;
	bool looping = false;
	if (!ade_get_args(L, "s|b", &name, &looping)) {
		return ade_set_error(L, "o", l_MoviePlayer.Set(movie_player_h()));
	}

	// Audio is disabled for scripted movies at the moment
	cutscene::PlaybackProperties props;
	props.with_audio = false;
	props.looping = looping;

	auto player = cutscene::Player::newPlayer(name, props);

	if (!player) {
		return ade_set_args(L, "o", l_MoviePlayer.Set(movie_player_h()));
	}

	return ade_set_args(L, "o", l_MoviePlayer.Set(movie_player_h(std::move(player))));
}

ADE_FUNC(createPersistentParticle,
	l_Graphics,
	"vector Position, vector Velocity, number Lifetime, number Radius, [enumeration Type=PARTICLE_DEBUG, number TracerLength=-1, "
	"boolean Reverse=false, texture Texture=Nil, object AttachedObject=Nil]",
	"Creates a persistent particle. Persistent variables are handled specially by the engine so that this "
	"function can return a handle to the caller. Only use this if you absolutely need it. Use createParticle if "
	"the returned handle is not required. Use PARTICLE_* enumerations for type."
	"Reverse reverse animation, if one is specified"
	"Attached object specifies object that Position will be (and always be) relative to.",
	"particle",
	"Handle to the created particle")
{
	particle::particle_info pi;
	pi.type            = particle::PARTICLE_DEBUG;
	pi.optional_data   = -1;
	pi.attached_objnum = -1;
	pi.attached_sig    = -1;
	pi.reverse         = false;

	// Need to consume tracer_length parameter but it isn't used anymore
	float temp;

	enum_h* type       = nullptr;
	bool rev           = false;
	object_h* objh     = nullptr;
	texture_h* texture = nullptr;
	if (!ade_get_args(L, "ooff|ofboo", l_Vector.Get(&pi.pos), l_Vector.Get(&pi.vel), &pi.lifetime, &pi.rad,
	                  l_Enum.GetPtr(&type), &temp, &rev, l_Texture.GetPtr(&texture), l_Object.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if (type != nullptr) {
		switch (type->index) {
		case LE_PARTICLE_DEBUG:
			pi.type = particle::PARTICLE_DEBUG;
			break;
		case LE_PARTICLE_FIRE:
			pi.type = particle::PARTICLE_FIRE;
			break;
		case LE_PARTICLE_SMOKE:
			pi.type = particle::PARTICLE_SMOKE;
			break;
		case LE_PARTICLE_SMOKE2:
			pi.type = particle::PARTICLE_SMOKE2;
			break;
		case LE_PARTICLE_BITMAP:
			if (texture == nullptr || !texture->isValid()) {
				LuaError(L, "Invalid texture specified for createParticle()!");
				return ADE_RETURN_NIL;
			} else {
				pi.optional_data = texture->handle;
				pi.type          = particle::PARTICLE_BITMAP;
			}
			break;
		default:
			LuaError(L, "Invalid particle enum for createParticle(). Can only support PARTICLE_* enums!");
			return ADE_RETURN_NIL;
		}
	}

	if (rev)
		pi.reverse = false;

	if (objh != nullptr && objh->IsValid()) {
		pi.attached_objnum = OBJ_INDEX(objh->objp);
		pi.attached_sig    = objh->objp->signature;
	}

	particle::WeakParticlePtr p = particle::createPersistent(&pi);

	if (!p.expired())
		return ade_set_args(L, "o", l_Particle.Set(particle_h(p)));
	else
		return ADE_RETURN_NIL;
}

ADE_FUNC(createParticle,
	l_Graphics,
	"vector Position, vector Velocity, number Lifetime, number Radius, [enumeration Type=PARTICLE_DEBUG, number TracerLength=-1, "
	"boolean Reverse=false, texture Texture=Nil, object AttachedObject=Nil]",
	"Creates a non-persistent particle. Use PARTICLE_* enumerations for type."
	"Reverse reverse animation, if one is specified"
	"Attached object specifies object that Position will be (and always be) relative to.",
	"boolean",
	"true if particle was created, false otherwise")
{
	particle::particle_info pi;
	pi.type            = particle::PARTICLE_DEBUG;
	pi.optional_data   = -1;
	pi.attached_objnum = -1;
	pi.attached_sig    = -1;
	pi.reverse         = false;

	// Need to consume tracer_length parameter but it isn't used anymore
	float temp;

	enum_h* type       = nullptr;
	bool rev           = false;
	object_h* objh     = nullptr;
	texture_h* texture = nullptr;
	if (!ade_get_args(L, "ooff|ofboo", l_Vector.Get(&pi.pos), l_Vector.Get(&pi.vel), &pi.lifetime, &pi.rad,
	                  l_Enum.GetPtr(&type), &temp, &rev, l_Texture.GetPtr(&texture), l_Object.GetPtr(&objh)))
		return ADE_RETURN_FALSE;

	if (type != nullptr) {
		switch (type->index) {
		case LE_PARTICLE_DEBUG:
			pi.type = particle::PARTICLE_DEBUG;
			break;
		case LE_PARTICLE_FIRE:
			pi.type = particle::PARTICLE_FIRE;
			break;
		case LE_PARTICLE_SMOKE:
			pi.type = particle::PARTICLE_SMOKE;
			break;
		case LE_PARTICLE_SMOKE2:
			pi.type = particle::PARTICLE_SMOKE2;
			break;
		case LE_PARTICLE_BITMAP:
			if (texture == nullptr || !texture->isValid()) {
				LuaError(L, "Invalid texture specified for createParticle()!");
				return ADE_RETURN_NIL;
			} else {
				pi.optional_data = texture->handle;
				pi.type          = particle::PARTICLE_BITMAP;
			}
			break;
		default:
			LuaError(L, "Invalid particle enum for createParticle(). Can only support PARTICLE_* enums!");
			return ADE_RETURN_NIL;
		}
	}

	if (rev)
		pi.reverse = false;

	if (objh != nullptr && objh->IsValid()) {
		pi.attached_objnum = OBJ_INDEX(objh->objp);
		pi.attached_sig    = objh->objp->signature;
	}

	particle::create(&pi);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(screenToBlob, l_Graphics, nullptr, "Captures the current render target and encodes it into a blob-PNG", "string", "The png blob string")
{
	if (!Gr_inited)
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", gr_blob_screen().c_str());
}

} // namespace api
} // namespace scripting

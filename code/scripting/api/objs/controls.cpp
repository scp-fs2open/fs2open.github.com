//
//

#include "controls.h"
#include "enums.h"

#include "io/mouse.h"
#include "io/cursor.h"
#include "graphics/2d.h"
#include "globalincs/systemvars.h"
#include "gamesequence/gamesequence.h"
#include "controlconfig/controlsconfig.h"
#include "headtracking/headtracking.h"

extern int mouse_inited;

namespace scripting {
namespace api {

//**********LIBRARY: Controls library
ADE_LIB(l_Mouse, "Controls", "io", "Controls library");

ADE_FUNC(getMouseX, l_Mouse, NULL, "Gets Mouse X pos", "number", "Mouse x position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int x;

	mouse_get_pos(&x, NULL);

	return ade_set_args(L, "i", x);
}

ADE_FUNC(getMouseY, l_Mouse, NULL, "Gets Mouse Y pos", "number", "Mouse y position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int y;

	mouse_get_pos(NULL, &y);

	return ade_set_args(L, "i", y);
}

ADE_FUNC(isMouseButtonDown,
	l_Mouse,
	"enumeration buttonCheck1 /* MOUSE_*_BUTTON */, [ enumeration buttonCheck2 /* MOUSE_*_BUTTON */, enumeration "
	"buttonCheck3 /* MOUSE_*_BUTTON */ ]",
	"Returns whether the specified mouse buttons are up or down",
	"boolean",
	"Whether specified mouse buttons are down, or false if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "b", false);

	enum_h *e[3] = {NULL, NULL, NULL};
	ade_get_args(L, "o|oo", l_Enum.GetPtr(&e[0]), l_Enum.GetPtr(&e[1]), l_Enum.GetPtr(&e[2]));	//Like a snake!

	bool rtn = false;
	int check_flags = 0;

	for(int i = 0; i < 3; i++)
	{
		if(e[i] == NULL)
			break;

		if(e[i]->index == LE_MOUSE_LEFT_BUTTON)
			check_flags |= MOUSE_LEFT_BUTTON;
		if(e[i]->index == LE_MOUSE_MIDDLE_BUTTON)
			check_flags |= MOUSE_MIDDLE_BUTTON;
		if(e[i]->index == LE_MOUSE_RIGHT_BUTTON)
			check_flags |= MOUSE_RIGHT_BUTTON;
	}

	if(mouse_down(check_flags))
		rtn = true;

	return ade_set_args(L, "b", rtn);
}

static int AxisInverted_sub(Joy_axis_index joy_axis, lua_State* L)
{
	Assertion(joy_axis == JOY_X_AXIS || joy_axis == JOY_Y_AXIS || joy_axis == JOY_Z_AXIS, "Only the X, Y, and Z axes are supported for now.");

	bool b;
	if (!ade_get_args(L, "*|b", &b))
		return ade_set_error(L, "b", false);

	// Find the binding of the action that has the given joystick axis bound
	CC_bind *A = nullptr;
	for (int i = JOY_HEADING_AXIS; i <= JOY_REL_THROTTLE_AXIS; ++i)
	{
		CC_bind B(CID_JOY0, joy_axis, CCF_AXIS);
		if (Control_config[i].first == B)
		{
			A = &Control_config[i].first;
			break;
		}
		else if (Control_config[i].second == B)
		{
			A = &Control_config[i].second;
			break;
		}
	}

	if (A == nullptr)
	{
		LuaError(L, "Control axis %d not found!", static_cast<int>(joy_axis));
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR)
		b ? (A->flags |= CCF_INVERTED) : (A->flags &= ~CCF_INVERTED);

	if (A->flags & CCF_INVERTED)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR_DEPRECATED(XAxisInverted, l_Mouse, "boolean inverted", "Gets or sets whether the Joy0 X-axis is inverted", "boolean", "True/false", gameversion::version(20, 1), "Deprecated in favor of per-action inversion")
{
	return AxisInverted_sub(JOY_X_AXIS, L);
}

ADE_VIRTVAR_DEPRECATED(YAxisInverted, l_Mouse, "boolean inverted", "Gets or sets whether the Joy0 Y-axis is inverted", "boolean", "True/false", gameversion::version(20, 1), "Deprecated in favor of per-action inversion")
{
	return AxisInverted_sub(JOY_Y_AXIS, L);
}

ADE_VIRTVAR_DEPRECATED(ZAxisInverted, l_Mouse, "boolean inverted", "Gets or sets whether the Joy0 Z-axis is inverted", "boolean", "True/false", gameversion::version(20, 1), "Deprecated in favor of per-action inversion")
{
	return AxisInverted_sub(JOY_Z_AXIS, L);
}

ADE_FUNC(AxisActionInverted, l_Mouse, "int IoActionId, boolean inverted", "Gets or sets whether the given axis action is inverted", "boolean", "True/false")
{
	// z64: If so desired, it may be possible to have two inversion states, one set by the player in the config menu,
	//   and one set by scripting.  Essentially, the CCI will have its own inversion flag in addition to the inversion
	//   flags to both bindings.  Thus, its possible for the player to invert one binding while keeping the other
	//   binding normal instead of having the script try to set each bind's inversion state individually.
	int AxisAction = CCFG_MAX;
	bool inverted;

	int n = ade_get_args(L, "i|b", &AxisAction, &inverted);

	if (n==0)
		return ade_set_error(L, "b", false);	// no arguments passed

	if ((AxisAction < JOY_HEADING_AXIS) || (JOY_REL_THROTTLE_AXIS <= AxisAction))
		return ade_set_error(L, "b", false);	// invalid IoActionId

	auto& item = Control_config[AxisAction];

	if (n > 1)
	{
		item.invert(inverted);
	}

	if (item.is_inverted())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(AxisInverted, l_Mouse, "int cid, int axis, boolean inverted", "Gets or sets the given Joystick or Mouse axis inversion state", "boolean", "True/false")
{
	int joy = CID_NONE;
	int axis = -1;
	bool inverted = false;
	int n = ade_get_args(L, "ii|b", &joy, &axis, &inverted);

	if (n == 0)
		return ade_set_error(L, "b", false);	// no arguments passed

	if ((joy < CID_JOY0) || (CID_JOY_MAX <= joy) || (joy != CID_MOUSE))
		return ade_set_error(L, "b", false);	// Invalid cid

	if ((axis < 0) || (axis > JOY_NUM_AXES))
		return ade_set_error(L, "b", false);	// invalid axis

	// Find the binding that has the given joy.
	// This is a bit obtuse since inversion is on the config side instead of on the joystick side
	CC_bind *A = nullptr;
	for (int i = JOY_HEADING_AXIS; i <= JOY_REL_THROTTLE_AXIS; ++i)
	{
		CC_bind B(static_cast<CID>(joy), axis, CCF_AXIS);
		A = Control_config[i].find(B);

		if (A != nullptr)
			break;
	}

	// TODO Should this be an error or silent false?
	if (A == nullptr)
		return ade_set_error(L, "b", false);	// Axis not bound

	if (n > 2)
	{
		A->invert(inverted);
	}

	if (A->is_inverted())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(setCursorImage, l_Mouse, "string filename", "Sets mouse cursor image, and allows you to lock/unlock the image. (A locked cursor may only be changed with the unlock parameter)", "boolean", "true if successful, false otherwise")
{
	using namespace io::mouse;

	if(!mouse_inited || !Gr_inited)
		return ade_set_error(L, "b", false);

	if (Is_standalone)
		return ade_set_error(L, "b", false);

	const char* s = nullptr;
	enum_h *u = NULL; // This isn't used anymore

	if(!ade_get_args(L, "s|o", &s, l_Enum.GetPtr(&u)))
		return ade_set_error(L, "b", false);

	Cursor* cursor = CursorManager::get()->loadCursor(s);

	if (cursor == NULL)
	{
		return ade_set_error(L, "b", false);
	}

	CursorManager::get()->setCurrentCursor(cursor);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(setCursorHidden, l_Mouse, "boolean hide, [boolean grab]", "Hides the cursor when <i>hide</i> is true, otherwise shows it. <i>grab</i> determines if "
	"the mouse will be restricted to the window. Set this to true when hiding the cursor while in game. By default grab will be true when we are in the game play state, false otherwise.", NULL, NULL)
{
	if(!mouse_inited)
		return ADE_RETURN_NIL;

	bool b = false;
	bool grab = gameseq_get_state() == GS_STATE_GAME_PLAY;
	ade_get_args(L, "b|b", &b, &grab);

	io::mouse::CursorManager::get()->showCursor(!b, grab);

	return ADE_RETURN_NIL;
}

ADE_FUNC(forceMousePosition, l_Mouse, "number x, number y", "function to force mouse position", "boolean", "if the operation succeeded or not")
{
	if(!mouse_inited)
		return ADE_RETURN_FALSE;

	if(!Gr_inited)
		return ADE_RETURN_FALSE;

	int x, y;
	if (!(ade_get_args(L, "ii", &x, &y)))
		return ADE_RETURN_FALSE;

	if (!((x >= 0) && (x <= gr_screen.max_w)))
		return ADE_RETURN_FALSE;

	if (!((y >= 0) && (y <= gr_screen.max_h)))
		return ADE_RETURN_FALSE;

	mouse_set_pos(x, y);

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(MouseControlStatus, l_Mouse, "boolean", "Gets and sets the retail mouse control status", "boolean", "if the retail mouse is on or off")
{
	if(!mouse_inited)
		return ADE_RETURN_NIL;

	bool newVal = false;
	if (!ade_get_args(L, "*|b", &newVal))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		if (newVal)
		{
			Use_mouse_to_fly = true;
		}
		else
		{
			Use_mouse_to_fly = false;
		}
	}

	if (Use_mouse_to_fly)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(getMouseSensitivity, l_Mouse, NULL, "Gets mouse sensitivity setting", "number", "Mouse sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Mouse_sensitivity);
}

ADE_FUNC(getJoySensitivity, l_Mouse, NULL, "Gets joystick sensitivity setting", "number", "Joystick sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Joy_sensitivity);
}

ADE_FUNC(getJoyDeadzone, l_Mouse, NULL, "Gets joystick deadzone setting", "number", "Joystick deadzone in range of 0-9")
{
	return ade_set_args(L, "i", Joy_dead_zone_size / 5);
}

//trackir funcs
ADE_FUNC(updateTrackIR, l_Mouse, NULL, "Updates Tracking Data. Call before using get functions", "boolean", "Checks if trackir is available and updates variables, returns true if successful, otherwise false")
{
	if( !headtracking::isEnabled() )
		return ADE_RETURN_FALSE;

	if (!headtracking::query())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getTrackIRPitch, l_Mouse, NULL, "Gets pitch axis from last update", "number", "Pitch value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args( L, "f", headtracking::getStatus()->pitch);
}

ADE_FUNC(getTrackIRYaw, l_Mouse, NULL, "Gets yaw axis from last update", "number", "Yaw value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->yaw);
}

ADE_FUNC(getTrackIRRoll, l_Mouse, NULL, "Gets roll axis from last update", "number", "Roll value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->roll);
}

ADE_FUNC(getTrackIRX, l_Mouse, NULL, "Gets x position from last update", "number", "X value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->x);
}

ADE_FUNC(getTrackIRY, l_Mouse, NULL, "Gets y position from last update", "number", "Y value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->y);
}

ADE_FUNC(getTrackIRZ, l_Mouse, NULL, "Gets z position from last update", "number", "Z value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled() )
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->z);
}

}
}

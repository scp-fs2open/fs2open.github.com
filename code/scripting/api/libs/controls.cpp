//
//

#include "controls.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/control_binding.h"

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

//****SUBLIBRARY: Controls/Keybindings
ADE_LIB_DERIV(l_Control_Keybindings, "Keybinding", nullptr, nullptr, l_Mouse);

ADE_INDEXER(l_Control_Keybindings, "string Name", "Gets handle to a keybinding", "keybinding", "Key binding handle, or invalid key binding handle if name is invalid")
{
	int idx = -1;

	const char* name = nullptr;

	if (!ade_get_args(L, "*s", &name))
		return ade_set_args(L, "o", l_ControlBinding.Set(cci_h()));
	
	if (name == nullptr)
		return ade_set_args(L, "o", l_ControlBinding.Set(cci_h()));

	idx = ActionToVal(name);
	
	if (idx < 0 || idx >= IoActionId::CCFG_MAX)
		return ade_set_args(L, "o", l_ControlBinding.Set(cci_h()));
	else
		return ade_set_args(L, "o", l_ControlBinding.Set(idx));
}
ADE_FUNC(__len, l_Control_Keybindings, nullptr, "Number of keybindings.", "number", "Number of keybindings in mission")
{
	return ade_set_args(L, "i", (int) IoActionId::CCFG_MAX);
}

ADE_FUNC(getMouseX, l_Mouse, nullptr, "Gets Mouse X pos", "number", "Mouse x position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int x;

	mouse_get_pos(&x, nullptr);

	return ade_set_args(L, "i", x);
}

ADE_FUNC(getMouseY, l_Mouse, nullptr, "Gets Mouse Y pos", "number", "Mouse y position, or 0 if mouse is not initialized yet")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", 0);

	int y;

	mouse_get_pos(nullptr, &y);

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

	enum_h *e[3] = {nullptr, nullptr, nullptr};
	ade_get_args(L, "o|oo", l_Enum.GetPtr(&e[0]), l_Enum.GetPtr(&e[1]), l_Enum.GetPtr(&e[2]));	//Like a snake!

	bool rtn = false;
	int check_flags = 0;

	for(int i = 0; i < 3; i++)
	{
		if(e[i] == nullptr)
			break;

		if(e[i]->index == LE_MOUSE_LEFT_BUTTON)
			check_flags |= MOUSE_LEFT_BUTTON;
		if(e[i]->index == LE_MOUSE_MIDDLE_BUTTON)
			check_flags |= MOUSE_MIDDLE_BUTTON;
		if(e[i]->index == LE_MOUSE_RIGHT_BUTTON)
			check_flags |= MOUSE_RIGHT_BUTTON;
		if (e[i]->index == LE_MOUSE_X1_BUTTON)
			check_flags |= MOUSE_X1_BUTTON;
		if (e[i]->index == LE_MOUSE_X2_BUTTON)
			check_flags |= MOUSE_X2_BUTTON;
	}

	if(mouse_down(check_flags))
		rtn = true;

	return ade_set_args(L, "b", rtn);
}

ADE_FUNC(mouseButtonDownCount,
	l_Mouse,
	"enumeration buttonCheck /* any one of MOUSE_LEFT_BUTTON, MOUSE_RIGHT_BUTTON, MOUSE_MIDDLE_BUTTON, MOUSE_X1_BUTTON, MOUSE_X2_BUTTON */, [ boolean reset_count ]",
	"Returns the pressed count of the specified button.  The count is then reset, unless reset_count (which defaults to true) is false.",
	"number",
	"The number of frames this button has been pressed, or -1 if the mouse has not been initialized")
{
	if(!mouse_inited)
		return ade_set_error(L, "i", -1);

	enum_h buttonCheck;
	int check_btn = 0;
	bool reset_count = true;

	if (!ade_get_args(L, "o|b", l_Enum.Get(&buttonCheck), &reset_count))
		return ade_set_error(L, "i", -1);

	if (!buttonCheck.IsValid())
		return ade_set_error(L, "i", -1);

	switch (buttonCheck.index)
	{
		case LE_MOUSE_LEFT_BUTTON:
			check_btn = MOUSE_LEFT_BUTTON;
			break;
		case LE_MOUSE_RIGHT_BUTTON:
			check_btn = MOUSE_RIGHT_BUTTON;
			break;
		case LE_MOUSE_MIDDLE_BUTTON:
			check_btn = MOUSE_MIDDLE_BUTTON;
			break;
		case LE_MOUSE_X1_BUTTON:
			check_btn = MOUSE_X1_BUTTON;
			break;
		case LE_MOUSE_X2_BUTTON:
			check_btn = MOUSE_X2_BUTTON;
			break;
		default:
			return ade_set_error(L, "i", -1);
	}

	int count = mouse_down_count(check_btn, reset_count ? 1 : 0);

	return ade_set_args(L, "i", count);
}

ADE_FUNC(flushMouse,
	l_Mouse,
	nullptr,
	"Clears mouse input data, including button press count, button flags, wheel scroll value, and position delta.",
	nullptr,
	"Returns nothing")
{
	SCP_UNUSED(L);

	mouse_flush();

	return ADE_RETURN_NIL;
}

static int AxisActionInverted_sub(int AxisAction, int ordinal, lua_State* L)
{
	bool b;
	if (!ade_get_args(L, "*|b", &b))
		return ade_set_error(L, "b", false);

	if ((AxisAction < JOY_HEADING_AXIS) || (AxisAction > JOY_REL_THROTTLE_AXIS))
		return ade_set_error(L, "b", false);	// invalid IoActionId

	CC_bind *bind;
	if (ordinal == 0)
		bind = &Control_config[AxisAction].first;
	else if (ordinal == 1)
		bind = &Control_config[AxisAction].second;
	else
	{
		UNREACHABLE("Currently only primary and secondary bindings are supported!");
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR)
		bind->invert(b);

	if (bind->is_inverted())
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR_DEPRECATED(XAxisInverted, l_Mouse, "boolean inverted", "Gets or sets whether the heading axis action's primary binding is inverted", "boolean", "True/false", gameversion::version(21, 6), "Deprecated in favor of HeadingAxisInverted")
{
	return AxisActionInverted_sub(JOY_HEADING_AXIS, 0, L);
}

ADE_VIRTVAR_DEPRECATED(YAxisInverted, l_Mouse, "boolean inverted", "Gets or sets whether the pitch axis action's primary binding is inverted", "boolean", "True/false", gameversion::version(21, 6), "Deprecated in favor of PitchAxisInverted")
{
	return AxisActionInverted_sub(JOY_PITCH_AXIS, 0, L);
}

ADE_VIRTVAR_DEPRECATED(ZAxisInverted, l_Mouse, "boolean inverted", "Gets or sets whether the bank axis action's primary binding is inverted", "boolean", "True/false", gameversion::version(21, 6), "Deprecated in favor of BankAxisInverted")
{
	return AxisActionInverted_sub(JOY_BANK_AXIS, 0, L);
}

ADE_VIRTVAR(HeadingAxisPrimaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the heading axis action's primary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_HEADING_AXIS, 0, L);
}

ADE_VIRTVAR(HeadingAxisSecondaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the heading axis action's secondary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_HEADING_AXIS, 1, L);
}

ADE_VIRTVAR(PitchAxisPrimaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the pitch axis action's primary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_PITCH_AXIS, 0, L);
}

ADE_VIRTVAR(PitchAxisSecondaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the pitch axis action's secondary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_PITCH_AXIS, 1, L);
}

ADE_VIRTVAR(BankAxisPrimaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the bank axis action's primary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_BANK_AXIS, 0, L);
}

ADE_VIRTVAR(BankAxisSecondaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the bank axis action's secondary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_BANK_AXIS, 1, L);
}

ADE_VIRTVAR(AbsoluteThrottleAxisPrimaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the absolute throttle axis action's primary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_ABS_THROTTLE_AXIS, 0, L);
}

ADE_VIRTVAR(AbsoluteThrottleAxisSecondaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the absolute throttle axis action's secondary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_ABS_THROTTLE_AXIS, 1, L);
}

ADE_VIRTVAR(RelativeThrottleAxisPrimaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the relative throttle axis action's primary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_REL_THROTTLE_AXIS, 0, L);
}

ADE_VIRTVAR(RelativeThrottleAxisSecondaryInverted, l_Mouse, "boolean inverted", "Gets or sets whether the relative throttle axis action's secondary binding is inverted", "boolean", "True/false")
{
	return AxisActionInverted_sub(JOY_REL_THROTTLE_AXIS, 1, L);
}

ADE_FUNC(AxisInverted, l_Mouse, "number cid, number axis, boolean inverted", "Gets or sets the given Joystick or Mouse axis inversion state.  Mouse cid = -1, Joystick cid = [0, 3]", "boolean", "True/false")
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
		CC_bind B(static_cast<CID>(joy), static_cast<short>(axis), CCF_AXIS);
		A = Control_config[i].find(B);
		if (A != nullptr)
			break;

		// the binding we're looking for could be inverted
		CC_bind C(static_cast<CID>(joy), static_cast<short>(axis), CCF_AXIS | CCF_INVERTED);
		A = Control_config[i].find(C);
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
	enum_h *u = nullptr; // This isn't used anymore

	if(!ade_get_args(L, "s|o", &s, l_Enum.GetPtr(&u)))
		return ade_set_error(L, "b", false);

	Cursor* cursor = CursorManager::get()->loadCursor(s);

	if (cursor == nullptr)
	{
		return ade_set_error(L, "b", false);
	}

	CursorManager::get()->setCurrentCursor(cursor);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(setCursorHidden, l_Mouse, "boolean hide, [boolean grab]", "Hides the cursor when <i>hide</i> is true, otherwise shows it. <i>grab</i> determines if "
	"the mouse will be restricted to the window. Set this to true when hiding the cursor while in game. By default grab will be true when we are in the game play state, false otherwise.", nullptr, nullptr)
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

		mouse_flush();
	}

	if (Use_mouse_to_fly)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(getMouseSensitivity, l_Mouse, nullptr, "Gets mouse sensitivity setting", "number", "Mouse sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Mouse_sensitivity);
}

ADE_FUNC(getJoySensitivity, l_Mouse, nullptr, "Gets joystick sensitivity setting", "number", "Joystick sensitivity in range of 0-9")
{
	return ade_set_args(L, "i", Joy_sensitivity);
}

ADE_FUNC(getJoyDeadzone, l_Mouse, nullptr, "Gets joystick deadzone setting", "number", "Joystick deadzone in range of 0-9")
{
	return ade_set_args(L, "i", Joy_dead_zone_size / 5);
}

//trackir funcs
ADE_FUNC(updateTrackIR, l_Mouse, nullptr, "Updates Tracking Data. Call before using get functions", "boolean", "Checks if trackir is available and updates variables, returns true if successful, otherwise false")
{
	if( !headtracking::isEnabled() )
		return ADE_RETURN_FALSE;

	if (!headtracking::query())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getTrackIRPitch, l_Mouse, nullptr, "Gets pitch axis from last update", "number", "Pitch value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args( L, "f", headtracking::getStatus()->pitch);
}

ADE_FUNC(getTrackIRYaw, l_Mouse, nullptr, "Gets yaw axis from last update", "number", "Yaw value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->yaw);
}

ADE_FUNC(getTrackIRRoll, l_Mouse, nullptr, "Gets roll axis from last update", "number", "Roll value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->roll);
}

ADE_FUNC(getTrackIRX, l_Mouse, nullptr, "Gets x position from last update", "number", "X value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->x);
}

ADE_FUNC(getTrackIRY, l_Mouse, nullptr, "Gets y position from last update", "number", "Y value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->y);
}

ADE_FUNC(getTrackIRZ, l_Mouse, nullptr, "Gets z position from last update", "number", "Z value -1 to 1, or 0 on failure")
{
	if (!headtracking::isEnabled() )
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", headtracking::getStatus()->z);
}

}
}

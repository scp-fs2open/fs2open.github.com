/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "globalincs/pstypes.h"
#include "io/gamepad.h"
#include "io/mouse.h"
#include "io/cursor.h"
#include "io/key.h"
#include "osapi/osapi.h"
#include "gamesequence/gamesequence.h"
#include "options/Option.h"
#include "scpui/rocket_ui.h"

#include "imgui.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

PUSH_SUPPRESS_WARNINGS
#include <Rocket/Controls.h>
#include <Rocket/Core.h>
POP_SUPPRESS_WARNINGS

#pragma pop_macro("Assert")



using namespace io::gamepad;

namespace {

bool initialized = false;

typedef std::unique_ptr<Gamepad> GamepadPtr;

SCP_vector<GamepadPtr> gamepads;

constexpr uint32_t KEY_CHECK_INTERVAL_MS = 150;
constexpr uint32_t CURSOR_UPDATE_INTERVAL_MS = 20;	// 50 Hz

// config options ----
bool NavEnabled = true;
bool SwapActionCancel = false;
int CursorSpeed = 6;
// -------------------

// NOTE: never return true from here since we want these events to cascade to
//       other places
bool event_handler(const SDL_Event &evt)
{
	if ( !os::events::isWindowEvent(evt, os::getSDLMainWindow()) ) {
		return false;
	}

	const auto id = evt.gdevice.which;

	auto gamepad = std::find_if(gamepads.begin(), gamepads.end(),
								[id](GamepadPtr &p) { return p->getId() == id; });

	if (evt.type == SDL_EVENT_GAMEPAD_ADDED) {
		// this event can fire more than once for the same device so we need to
		// check for duplicates
		if (gamepad == gamepads.end()) {
			gamepads.push_back(GamepadPtr(new Gamepad(id)));
		}

		return false;
	} else if (evt.type == SDL_EVENT_GAMEPAD_REMOVED) {
		if (gamepad != gamepads.end()) {
			std::swap(*gamepad, gamepads.back());
			gamepads.pop_back();
		}

		return false;
	}

	if (gamepad == gamepads.end()) {
		return false;
	}

	// skip these events if control config is in bind or search mode
	if (control_config_special_mode()) {
		return false;
	}

	switch (evt.type) {
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			(*gamepad)->mark_button(evt.gbutton.button, evt.gbutton.down);
			break;

		case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
			bool down = (evt.gaxis.value > 10000);

			if (evt.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) {
				(*gamepad)->mark_button(GAMEPAD_BUTTON_LEFT_TRIGGER, down);
			} else if (evt.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
				(*gamepad)->mark_button(GAMEPAD_BUTTON_RIGHT_TRIGGER, down);
			}

			break;
		}
	}

	return false;
}

bool change_gamepad_nav_func(float new_val, bool initial)
{
	NavEnabled = new_val;

	if ( !initial ) {
		if (new_val) {
			io::gamepad::init();
		} else {
			io::gamepad::shutdown();
		}
	}

	return true;
}

void parse_gamepad_nav_func()
{
	bool value;
	stuff_boolean(&value);

	NavEnabled = value;
}

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
auto GamepadNavOption __UNUSED = options::OptionBuilder<bool>("Input.GamepadNav",
			std::pair<const char*, int>{"Gamepad Navigation", 1933},
			std::pair<const char*, int>{"Enable or disable gamepad UI control", 1934})
			.category(std::make_pair("Input", 1827))
			.level(options::ExpertLevel::Beginner)
			.importance(2)
			.default_func([]() { return NavEnabled; })
			.change_listener(change_gamepad_nav_func)
			.parser(parse_gamepad_nav_func)
			.finish();

void parse_gamepad_swap_func()
{
	bool value;
	stuff_boolean(&value);

	SwapActionCancel = value;
}

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
auto GamepadSwapOption __UNUSED = options::OptionBuilder<bool>("Input.GamepadSwapActionCancel",
			std::pair<const char*, int>{"Swap Action/Cancel Buttons", 1935},
			std::pair<const char*, int>{"Swap gamepad buttons used for action and cancel", 1936})
			.category(std::make_pair("Input", 1827))
			.level(options::ExpertLevel::Beginner)
			.importance(1)
			.default_func([]() { return SwapActionCancel; })
			.bind_to(&SwapActionCancel)
			.parser(parse_gamepad_swap_func)
			.finish();

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
auto GamepadCursorSpeed __UNUSED = options::OptionBuilder<int>("Input.GamepadCursorSpeed",
			std::pair<const char*, int>{"Gamepad Cursor Speed", 1937},
			std::pair<const char*, int>{"Movement speed of cursor from gamepad inputs", 1938})
			.category(std::make_pair("Input", 1827))
			.level(options::ExpertLevel::Beginner)
			.importance(0)
			.range(1, 10)
			.default_func([]() { return CursorSpeed; })
			.flags({options::OptionFlags::RangeTypeInteger})
			.bind_to(&CursorSpeed)
			.finish();

}	// namespace


namespace io::gamepad {

Gamepad::Gamepad(SDL_JoystickID _id) :
	m_id(_id)
{
	m_gamepad = SDL_OpenGamepad(m_id);
	m_button_state.fill(false);
}

Gamepad::~Gamepad()
{
	if (m_gamepad) {
		SDL_CloseGamepad(m_gamepad);
	}
}

bool Gamepad::action()
{
	auto button = SwapActionCancel ? SDL_GAMEPAD_BUTTON_EAST : SDL_GAMEPAD_BUTTON_SOUTH;

	auto state = m_button_state[button];
	m_button_state[button] = false;	// we only want this to be true once

	return state;
}

bool Gamepad::cancel()
{
	auto button = SwapActionCancel ? SDL_GAMEPAD_BUTTON_SOUTH : SDL_GAMEPAD_BUTTON_EAST;

	auto state = m_button_state[button];
	m_button_state[button] = false;	// we only want this to be true once

	return state;
}

int Gamepad::get_key()
{
	int k = 0;

	if (cancel()) {
		k = KEY_ESC;
		// resets automatically
	} else if (m_button_state[SDL_GAMEPAD_BUTTON_DPAD_DOWN]) {
		k = KEY_DOWN;
		// key repeats
	} else if (m_button_state[SDL_GAMEPAD_BUTTON_DPAD_UP]) {
		k = KEY_UP;
		// key repeats
	} else if (m_button_state[SDL_GAMEPAD_BUTTON_DPAD_LEFT]) {
		k = KEY_LEFT;
		// key repeats
	} else if (m_button_state[SDL_GAMEPAD_BUTTON_DPAD_RIGHT]) {
		k = KEY_RIGHT;
		// key repeats
	} else if (m_button_state[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER]) {
		k = KEY_SHIFTED | KEY_TAB;
		// reset after use
		m_button_state[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER] = false;
	} else if (m_button_state[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER]) {
		k = KEY_TAB;
		// reset after use
		m_button_state[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] = false;
	} else if (m_button_state[GAMEPAD_BUTTON_LEFT_TRIGGER]) {
		k = SDLK_PAGEDOWN;
		// reset after use
		m_button_state[GAMEPAD_BUTTON_LEFT_TRIGGER] = false;
	} else if (m_button_state[GAMEPAD_BUTTON_RIGHT_TRIGGER]) {
		k = SDLK_PAGEUP;
		// reset after use
		m_button_state[GAMEPAD_BUTTON_RIGHT_TRIGGER] = false;
	}

	return k;
}

bool Gamepad::update_mouse_pos()
{
	if ( !m_gamepad ) {
		return false;
	}

	// we poll directly here in order to get smooth movement
	int gx = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
	int gy = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTY);

	// ignore possible stick drift
	if (abs(gx) < DEAD_ZONE) gx = 0;
	if (abs(gy) < DEAD_ZONE) gy = 0;

	if ( !gx && !gy ) {
		return false;
	}

	// scales delta to a range between -4..4 (slow, accurate) and -32..32 (fast, inaccurate)
	const float sensitivity = 8000.f - (CursorSpeed - 1.f) * (7000.f / 9.f);

	float dx = gx / sensitivity;
	float dy = gy / sensitivity;

	int x = 0;
	int y = 0;

	mouse_get_real_pos(&x, &y);

	// update pos and deltas
	mouse_update_pos_scaled(static_cast<int>(x+dx), static_cast<int>(y+dy), dx, dy);

	// now change position
	SDL_HideCursor();		// prevents cursor getting stuck as non-game one
	SDL_WarpMouseInWindow(os::getSDLMainWindow(), x+dx, y+dy);
	SDL_ShowCursor();

	return true;
}

void Gamepad::mark_mouse_button(Uint8 button)
{
	const int left_button = SwapActionCancel ? SDL_GAMEPAD_BUTTON_EAST : SDL_GAMEPAD_BUTTON_SOUTH;
	uint m_button = 0;

	// skip this if we're doing scpui events
	if (scpui::getContext()) {
		return;
	}

	if (button == left_button) {
		// "A" or "B" (if swapped)
		m_button = MOUSE_LEFT_BUTTON;
	} else if (button == SDL_GAMEPAD_BUTTON_WEST) {
		// "X"
		m_button = MOUSE_RIGHT_BUTTON;
	}

	if ( !m_button ) {
		return;
	}

	mouse_mark_button(m_button, m_button_state[button] ? 1 : 0);
}

void Gamepad::scpui_translate_button(Uint8 button)
{
	// skip this if we aren't doing scpui inputs
	auto input_context = scpui::getContext();

	if ( !input_context ) {
		return;
	}

	using namespace Rocket::Core;

	// mouse button events
	const int left_button = SwapActionCancel ? SDL_GAMEPAD_BUTTON_EAST : SDL_GAMEPAD_BUTTON_SOUTH;
	int mouse_button = -1;

	if (button == left_button) {
		mouse_button = 0;
	} else if (button == SDL_GAMEPAD_BUTTON_WEST) {
		mouse_button = 1;
	}

	if (mouse_button >= 0) {
		if (m_button_state[button]) {
			input_context->ProcessMouseButtonDown(mouse_button, 0);
		} else {
			input_context->ProcessMouseButtonUp(mouse_button, 0);
		}

		// reset down state to avoid repeats
		m_button_state[button] = false;

		return;
	}

	// keyboard events
	Input::KeyIdentifier key = Input::KI_UNKNOWN;
	int mod = 0;
	bool reset_state = true;

	if (cancel()) {
		key = Input::KI_ESCAPE;
	} else if (button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
		key = Input::KI_UP;
		reset_state = false;
	} else if (button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
		key = Input::KI_DOWN;
		reset_state = false;
	} else if (button == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
		key = Input::KI_LEFT;
		reset_state = false;
	} else if (button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
		key = Input::KI_RIGHT;
		reset_state = false;
	} else if (button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
		key = Input::KI_TAB;
		mod = Input::KM_SHIFT;
	} else if (button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
		key = Input::KI_TAB;
	} else if (button == GAMEPAD_BUTTON_LEFT_TRIGGER) {
		key = Input::KI_NEXT;
	} else if (button == GAMEPAD_BUTTON_RIGHT_TRIGGER) {
		key = Input::KI_PRIOR;
	}

	if (key != Input::KI_UNKNOWN) {
		if (m_button_state[button]) {
			input_context->ProcessKeyDown(key, mod);

			// reset down state to avoid repeats
			if (reset_state) {
				m_button_state[button] = false;
			}
		} else {
			input_context->ProcessKeyUp(key, mod);
		}
	}
}

void Gamepad::mark_button(Uint8 button, bool down)
{
	if (button >= m_button_state.size()) {
		return;
	}

	m_button_state[button] = down;

	mark_mouse_button(button);
	scpui_translate_button(button);
}

bool Gamepad::get_camera_vals(int *dx, int *dy, int *dz, bool *lmb_down, bool *rmb_down, uint *key_flags)
{
	if ( !m_gamepad ) {
		return false;
	}

	// we poll directly here in order to get smooth movement
	int gx = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
	int gy = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTY);

	// ignore possible stick drift
	if (abs(gx) < DEAD_ZONE) gx = 0;
	if (abs(gy) < DEAD_ZONE) gy = 0;

	if ( !gx && !gy ) {
		return false;
	}

	const bool LeftTrigger = m_button_state[GAMEPAD_BUTTON_LEFT_TRIGGER];
	const bool RightTrigger = m_button_state[GAMEPAD_BUTTON_RIGHT_TRIGGER];

	if (LeftTrigger) {
		if (rmb_down) *rmb_down = true;

		if (RightTrigger) {
			if (key_flags) *key_flags |= KEY_SHIFTED;
		}
	} else if ( !RightTrigger ) {
		if (lmb_down) *lmb_down = true;
	}

	// scales delta to -3..3 (slow, accurate)
	// NOTE: using purposefully slow scale due to dz speed issues
	const float sensitivity = 10000.f;

	auto mdx = fl2i(gx / sensitivity);
	auto mdy = fl2i(gy / sensitivity);

	// we should only set dz or dx/dy, not both, and don't set dz if "shifted"
	if (RightTrigger && !LeftTrigger) {
		if (mdy && dz) {
			*dz = (mdy > 0) ? -1 : 1;
		}
	} else {
		if (dx) *dx = fl2i(mdx);
		if (dy) *dy = fl2i(mdy);
	}

	return true;
}


// Initialize gamepad leanback
// NOTE: This should work independently of io::joystick!
void init()
{
	if (initialized) {
		return;
	}

	if (Is_standalone) {
		return;
	}

	if ( !Using_in_game_options ) {
		NavEnabled = os_config_read_uint("Input", "GamepadNav", 1) == 1;
		SwapActionCancel = os_config_read_uint("Input", "GamepadSwapActionCancel", 0) == 1;
		CursorSpeed = os_config_read_uint("Input", "GamepadCursorSpeed", 6);
		CLAMP(CursorSpeed, 1, 10);
	}

	if ( !NavEnabled ) {
		return;
	}

	if ( !SDL_InitSubSystem(SDL_INIT_GAMEPAD) ) {
		return;
	}

	// TODO: SDL3 => It might be nice to eventually add touchpad support here
	//               for more precise cursor control.

	initialized = true;

	auto pads = SDL_GetGamepads(nullptr);

	if (pads) {
		for (int i = 0; pads[i]; ++i) {
			gamepads.push_back(GamepadPtr(new Gamepad(pads[i])));
		}

		SDL_free(pads);
		pads = nullptr;
	}

	if (ImGui::GetCurrentContext()) {
		auto &io = ImGui::GetIO();

		// enable gamepad nav for imgui
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		io.ConfigNavSwapGamepadButtons = SwapActionCancel;
		io.ConfigNavMoveSetMousePos = true;
	}

	// These events should *not* be consumed, so that the joystick code can also
	// make use of them
	using namespace os::events;
	addEventListener(SDL_EVENT_GAMEPAD_ADDED, DEFAULT_LISTENER_WEIGHT - 5, event_handler);
	addEventListener(SDL_EVENT_GAMEPAD_REMOVED, DEFAULT_LISTENER_WEIGHT - 5, event_handler);
	addEventListener(SDL_EVENT_GAMEPAD_AXIS_MOTION, DEFAULT_LISTENER_WEIGHT - 5, event_handler);
	addEventListener(SDL_EVENT_GAMEPAD_BUTTON_DOWN, DEFAULT_LISTENER_WEIGHT - 5, event_handler);
	addEventListener(SDL_EVENT_GAMEPAD_BUTTON_UP, DEFAULT_LISTENER_WEIGHT - 5, event_handler);
}

void shutdown()
{
	if ( !initialized ) {
		return;
	}

	if (ImGui::GetCurrentContext()) {
		auto &io = ImGui::GetIO();

		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

		io.ConfigNavSwapGamepadButtons = false;
		io.ConfigNavMoveSetMousePos = false;
	}

	gamepads.clear();
	gamepads.shrink_to_fit();

	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);

	initialized = false;
}

bool action()
{
	if ( !navActive() ) {
		return false;
	}

	// this should always work (cursor on/off, imgui, etc.)

	for (auto &p : gamepads) {
		if (p->action()) {
			return true;
		}
	}

	return false;
}

bool cancel()
{
	if ( !navActive() ) {
		return false;
	}

	// this should always work (cursor on/off, imgui, etc.)

	for (auto &p : gamepads) {
		if (p->cancel()) {
			return true;
		}
	}

	return false;
}

bool action_or_cancel()
{
	if ( !navActive() ) {
		return false;
	}

	// this should always work (cursor on/off, imgui, etc.)

	for (auto &p : gamepads) {
		if (p->action_or_cancel()) {
			return true;
		}
	}

	return false;
}

int get_key()
{
	static Uint64 key_check_time = 0;

	if ( !navActive() ) {
		return 0;
	}

	// skip if we aren't doing ui stuff
	if ( !io::mouse::CursorManager::get()->isCursorShown() ) {
		return 0;
	}

	// if imgui should be controlling the gamepad nav then let it
	const auto game_state = gameseq_get_state();

	if ((game_state == GS_STATE_LAB) || (game_state == GS_STATE_INGAME_OPTIONS)) {
		return 0;
	}

	// also skip this if we're doing scpui events
	if (scpui::getContext()) {
		return 0;
	}

	if (SDL_GetTicks() < key_check_time) {
		return 0;
	}

	key_check_time = SDL_GetTicks() + KEY_CHECK_INTERVAL_MS;

	for (auto &p : gamepads) {
		auto k = p->get_key();

		if (k) {
			return k;
		}
	}

	return 0;
}

void do_frame()
{
	static Uint64 next_update = 0;

	if ( !navActive() ) {
		return;
	}

	// skip if we aren't doing ui stuff
	if ( !io::mouse::CursorManager::get()->isCursorShown() ) {
		return;
	}

	// if imgui should be controlling the gamepad nav then let it
	const auto game_state = gameseq_get_state();

	if ((game_state == GS_STATE_LAB) || (game_state == GS_STATE_INGAME_OPTIONS)) {
		return;
	}

	// limit updates so that we aren't zooming all over the place at higher fps
	if (next_update > SDL_GetTicks()) {
		return;
	}

	next_update = SDL_GetTicks() + CURSOR_UPDATE_INTERVAL_MS;

	for (auto &p : gamepads) {
		if (p->update_mouse_pos()) {
			return;
		}
	}
}

bool get_camera_vals(int *dx, int *dy, int *dz, bool *lmb_down, bool *rmb_down, uint *key_flags)
{
	if ( !navActive() ) {
		return false;
	}

	if (dx) *dx = 0;
	if (dy) *dy = 0;
	if (dz) *dz = 0;
	if (lmb_down) *lmb_down = false;
	if (rmb_down) *rmb_down = false;
	if (key_flags) *key_flags = 0;

	for (auto &p : gamepads) {
		if (p->get_camera_vals(dx, dy, dz, lmb_down, rmb_down, key_flags)) {
			return true;
		}
	}

	return false;
}

bool navActive()
{
	return (initialized && NavEnabled && !gamepads.empty());
}

}	// namespace io::gamepad
